/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapp_switch.h"

struct mapp_switch_s;
typedef struct mapp_switch_s mapp_switch_t;
struct mapp_switch_s {
  common_t common;
  switch_t params;
};

static mapp_switch_t mapp_switches[MAX_SWITCHS];

llist_t llist_switch_pool;

void mapping_switchs_alloc(uint8_t switchs_cnt) {
  llist_builder(&llist_switch_pool, &mapp_switches[0], switchs_cnt, sizeof(mapp_switches[0]));
};

void mapping_switch_play(blob_t*);

// Test if the blob is within the key limits
bool mapping_switch_interact(blob_t* blob_ptr, common_t* common_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)common_ptr;
  //for (uint8_t j = 0; j < switch_ptr->params.touchs; j++) {
    if (blob_ptr->centroid.x > switch_ptr->params.rect.from.x &&
        blob_ptr->centroid.x < switch_ptr->params.rect.to.x &&
        blob_ptr->centroid.y > switch_ptr->params.rect.from.y &&
        blob_ptr->centroid.y < switch_ptr->params.rect.to.y) {
      blob_ptr->action.mapping_ptr = switch_ptr;
      blob_ptr->action.func_ptr = &mapping_switch_play;
      blob_ptr->action.mapping_ptr = switch_ptr;
      //blob_ptr->action.data_ptr = &switch_ptr->params.touch[j];
      return true;
    }
  //}
  return false;
};

void mapping_switch_play(blob_t* blob_ptr) {
  mapp_switch_t* mapp_switch = (mapp_switch_t*)blob_ptr->action.mapping_ptr;
    switch (mapp_switch->params.msg.midi.type) {
      case midi::NoteOff:
        break;
      case midi::NoteOn:
        if (!blob_ptr->lastState) {
          mapp_switch->params.msg.midi.type = midi::NoteOn;
          //mapp_switch->params.msg.midi.data2 = ... // TODO: add the velocity to the blob values!
          midi_sendOut(mapp_switch->params.msg.midi);
          #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_SWITCHS)
            Serial.printf("\nDEBUG_MAPPINGS_SWITCHS\tID:%d\tNOTE_ON:%d", i, mapp_switch->params.msg.midi.data1);
          #endif
        }
        /*
        else if (!blob_ptr->state) {
          mapp_switch->params.msg.midi.type = midi::NoteOff;
          midi_sendOut(mapp_switch->params.msg.midi);
          #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS)
            Serial.printf("\nDEBUG_MAPPINGS_SWITCHS\tID:%d\tNOTE_OFF:%d", i, mapp_switch->params.msg.midi.data1);
          #endif
        }
        */
        break;

      case midi::AfterTouchPoly:
        if (!blob_ptr->lastState) {
          mapp_switch->params.msg.midi.type = midi::NoteOn;
          midi_sendOut(mapp_switch->params.msg.midi);
          #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_SWITCHS)
            Serial.printf("\nDEBUG_MAPPINGS_SWITCHS\tID:%d\tNOTE_ON:%d", i, mapp_switch->params.msg.midi.data1);
          #endif
        }
        else if (!blob_ptr->lastState) {
          mapp_switch->params.msg.midi.type = midi::AfterTouchPoly;
          midi_sendOut(mapp_switch->params.msg.midi);
          #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_SWITCHS)
            Serial.printf("\nDEBUG_MAPPINGS_SWITCHS\tID:%d\tC_CHANGE:%d", i, mapp_switch->params.msg.midi.data2);
          #endif
        }
        break;

      case midi::ControlChange:
        mapp_switch->params.msg.midi.data2 = blob_ptr->centroid.z;
        midi_sendOut(mapp_switch->params.msg.midi);
        #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_SWITCHS)
          Serial.printf("\nDEBUG_MAPPINGS_SWITCHS\tID:%d\tC_CHANGE:%d", i, mapp_switch->params.msg.midi.data2);
        #endif
        break;

      case midi::ProgramChange:
        break;
      case midi::AfterTouchChannel:
        break;
      case midi::PitchBend:
        break;
      case midi::SystemExclusive:
        break;
      default:
        // Not handled in switch
        break;
    };
};

void mapping_switch_create(const JsonObject &config) {
  mapp_switch_t* mapp_switch = (mapp_switch_t*)llist_pop_front(&llist_switch_pool);
  midi_status_t status;
  //mapp_switch->common.blob_ptr = NULL;
  //mapp_switch->common.play_ptr = &mapping_switch_play;
  //mapp_switch->common.test_ptr = &mapping_switch_test;
  mapp_switch->params.rect.from.x = config["from"][0].as<float>();
  mapp_switch->params.rect.from.y = config["from"][1].as<float>();
  mapp_switch->params.rect.to.x = config["to"][0].as<float>();
  mapp_switch->params.rect.to.y = config["to"][1].as<float>();
  midi_msg_status_unpack(config["msg"][0]["press"]["midi"]["status"].as<uint8_t>(), &status);
  mapp_switch->params.msg.midi.type = status.type;
  mapp_switch->params.msg.midi.data1 = config["msg"][0]["press"]["midi"]["data1"].as<uint8_t>();
  mapp_switch->params.msg.midi.data2 = config["msg"][0]["press"]["midi"]["data2"].as<uint8_t>();
  mapp_switch->params.msg.midi.channel = status.channel;
  if (mapp_switch->params.msg.midi.type == midi::ControlChange ||
    mapp_switch->params.msg.midi.type == midi::AfterTouchPoly) {
    mapp_switch->params.msg.limit.min = config["msg"][0]["press"]["limit"]["min"].as<uint8_t>();
    mapp_switch->params.msg.limit.max = config["msg"][0]["press"]["limit"]["max"].as<uint8_t>(); 
  }
  llist_push_back(&llist_controls, mapp_switch);
};
