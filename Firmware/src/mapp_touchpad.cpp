/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapp_touchpad.h"

typedef struct mapp_touchpad_s mapp_touchpad_t;
struct mapp_touchpad_s {
  common_t common;
  touchpad_t params;
};

static mapp_touchpad_t mapp_touchpads[MAX_TOUCHPADS];

llist_t llist_touchpads_pool;

void mapping_touchpads_alloc(uint8_t touchpads_cnt) {
  llist_builder(&llist_touchpads_pool, &mapp_touchpads[0], touchpads_cnt, sizeof(mapp_touchpads[0]));
};

void mapping_touchpad_create(const JsonObject &config) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)llist_pop_front(&llist_touchpads_pool);
  touchpad_ptr->params.touchs = config["touchs"].as<uint8_t>();
  touchpad_ptr->params.rect.from.x = config["from"][0].as<float>();
  touchpad_ptr->params.rect.from.y = config["from"][1].as<float>();
  touchpad_ptr->params.rect.to.x = config["to"][0].as<float>();
  touchpad_ptr->params.rect.to.y = config["to"][1].as<float>();
  //touchpad_ptr->params.mode = config["mode_z"].as<uint8_t>();
  if (touchpad_ptr->params.touchs < MAX_TOUCHPAD_TOUCHS) {
    midi_status_t status;
    for (uint8_t j = 0; j<touchpad_ptr->params.touchs; j++){
      midi_msg_status_unpack(config["msg"][j]["pos_x"]["midi"]["status"].as<uint8_t>(), &status);
      touchpad_ptr->params.touch[j].pos_x.midi.type = status.type;
      touchpad_ptr->params.touch[j].pos_x.midi.data1 = config["msg"][j]["pos_x"]["midi"]["data1"].as<uint8_t>();
      touchpad_ptr->params.touch[j].pos_x.midi.data2 = config["msg"][j]["pos_x"]["midi"]["data2"].as<uint8_t>();
      touchpad_ptr->params.touch[j].pos_x.midi.channel = status.channel;
      if (touchpad_ptr->params.touch[j].pos_x.midi.type == midi::ControlChange || 
        touchpad_ptr->params.touch[j].pos_x.midi.type == midi::AfterTouchPoly) {
        touchpad_ptr->params.touch[j].pos_x.limit.min = config["msg"][j]["pos_x"]["limit"]["min"].as<uint8_t>();
        touchpad_ptr->params.touch[j].pos_x.limit.max = config["msg"][j]["pos_x"]["limit"]["max"].as<uint8_t>();
      }
      midi_msg_status_unpack(config["msg"][j]["pos_y"]["midi"]["status"].as<uint8_t>(), &status);
      touchpad_ptr->params.touch[j].pos_y.midi.type = status.type;
      touchpad_ptr->params.touch[j].pos_y.midi.data1 = config["msg"][j]["pos_y"]["midi"]["data1"].as<uint8_t>();
      touchpad_ptr->params.touch[j].pos_y.midi.data2 = config["msg"][j]["pos_y"]["midi"]["data2"].as<uint8_t>();
      touchpad_ptr->params.touch[j].pos_y.midi.channel = status.channel;
      if (touchpad_ptr->params.touch[j].pos_y.midi.type == midi::ControlChange ||
        touchpad_ptr->params.touch[j].pos_y.midi.type == midi::AfterTouchPoly) {
        touchpad_ptr->params.touch[j].pos_y.limit.min = config["msg"][j]["pos_y"]["limit"]["min"].as<uint8_t>();
        touchpad_ptr->params.touch[j].pos_y.limit.max = config["msg"][j]["pos_y"]["limit"]["max"].as<uint8_t>();
      }
      midi_msg_status_unpack(config["msg"][j]["press"]["midi"]["status"].as<uint8_t>(), &status);
      touchpad_ptr->params.touch[j].press.midi.type = status.type;
      touchpad_ptr->params.touch[j].press.midi.data1 = config["msg"][j]["press"]["midi"]["data1"].as<uint8_t>();
      touchpad_ptr->params.touch[j].press.midi.data2 = config["msg"][j]["press"]["midi"]["data2"].as<uint8_t>();
      touchpad_ptr->params.touch[j].press.midi.channel = status.channel;
      if (touchpad_ptr->params.touch[j].press.midi.type == midi::ControlChange || 
        touchpad_ptr->params.touch[j].press.midi.type == midi::AfterTouchPoly) {
        touchpad_ptr->params.touch[j].press.limit.min = config["msg"][j]["press"]["limit"]["min"].as<uint8_t>();
        touchpad_ptr->params.touch[j].press.limit.max = config["msg"][j]["press"]["limit"]["max"].as<uint8_t>();
      };
    };
  };
  llist_push_back(&llist_mappings, touchpad_ptr);
};

void mapping_touchpad_play(blob_t*);

bool mapping_touchpad_interact(blob_t* blob_ptr, common_t* mapping_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)mapping_ptr;
  if (blob_ptr->centroid.x > touchpad_ptr->params.rect.from.x &&
        blob_ptr->centroid.x < touchpad_ptr->params.rect.to.x &&
        blob_ptr->centroid.y > touchpad_ptr->params.rect.from.y &&
        blob_ptr->centroid.y < touchpad_ptr->params.rect.to.y) {

    blob_ptr->action.func_ptr = &mapping_touchpad_play;
    blob_ptr->action.mapping_ptr = touchpad_ptr;
    blob_ptr->action.touch_ptr = &touchpad_ptr->params.touch[0]; // FIXME: find a solution to mapp the touch index
    return true;
  }
  return false;
};

void mapping_touchpad_play(blob_t* blob_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)blob_ptr->action.mapping_ptr;
  touch_3d_t* touch_ptr = (touch_3d_t*)blob_ptr->action.touch_ptr;

  if (blob_ptr->status == NEW) {
    midi_send_out(touch_ptr->press.midi);
    midi_send_out(touch_ptr->pos_x.midi);
    midi_send_out(touch_ptr->pos_y.midi);
  }
  else if (blob_ptr->status == PRESENT) {
    //
  }  
  else if (blob_ptr->status == MISSING && blob_ptr->last_status == PRESENT) {
    //
  }

  if (blob_ptr->centroid.x != blob_ptr->last_centroid.x) { // This is float :-(
    touch_ptr->pos_x.midi.data2 = round(map(
        blob_ptr->centroid.x,
        touchpad_ptr->params.rect.from.x,
        touchpad_ptr->params.rect.to.x,
        touch_ptr->pos_x.limit.min,
        touch_ptr->pos_x.limit.max));
    // Change here!
    midi_send_out(touch_ptr->pos_x.midi);
    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_TOUCHPAD)
      Serial.printf("\nDEBUG_MAPPINGS_TOUCHPADS\tMIDI_X_CC:%d\tVAL:%d",
      touch_ptr->pos_x.midi.data2,
      round(map(blob_ptr->centroid.x, touchpad_ptr->params.rect.from.x, touchpad_ptr->params.rect.to.x, 0, 127)));
    #endif
  };

  if (blob_ptr->centroid.y != blob_ptr->last_centroid.y) { // This is float :-(
    touch_ptr->pos_y.last_val = touch_ptr->pos_y.midi.data2;
    touch_ptr->pos_y.midi.data2 = round(map(
        blob_ptr->centroid.y,
        touchpad_ptr->params.rect.from.y,
        touchpad_ptr->params.rect.to.y,
        touch_ptr->pos_y.limit.min,
        touch_ptr->pos_y.limit.max));
    // Change here!
    midi_send_out(touch_ptr->pos_y.midi);
    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_TOUCHPAD)
      Serial.printf("\nDEBUG_MAPPINGS_TOUCHPADS\tMIDI_Y_CC:%d\tVAL:%d",
      touch_ptr->pos_y.midi.data2,
      round(map(blob_ptr->centroid.y, touchpad_ptr->params.rect.from.y, touchpad_ptr->params.rect.to.y, 0, 127)));
    #endif
  };

  switch (touch_ptr->press.midi.type) {
    case midi::NoteOff:
      if (blob_ptr->status == NEW) {
        touch_ptr->press.midi.type = midi::NoteOn;
        //touch_ptr->press.midi.data2 = ... // TODO: add the velocity to the blob values!
        midi_send_out(touch_ptr->press.midi);
        #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_SWITCHS)
          Serial.printf("\nDEBUG_MAPPINGS_SWITCHS\tID:%d\tNOTE_ON:%d", i, touchpad_ptr->params.msg.midi.data1);
        #endif
      }
      else if (blob_ptr->status == MISSING && blob_ptr->last_status == PRESENT) {
        touch_ptr->press.midi.type = midi::NoteOff;
        midi_send_out(touch_ptr->press.midi);
        #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS)
          Serial.printf("\nDEBUG_MAPPINGS_SWITCHS\tID:%d\tNOTE_OFF:%d", i, touchpad_ptr->params.msg.midi.data1);
        #endif
      }
      break;
    case midi::NoteOn:
      if (blob_ptr->status == NEW) {
        touch_ptr->press.midi.type = midi::NoteOn;
        //mapp_switch->params.msg.midi.data2 = ... // TODO: add the velocity to the blob values!
        midi_send_out(touch_ptr->press.midi);
        #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_SWITCHS)
          Serial.printf("\nDEBUG_MAPPINGS_SWITCHS\tID:%d\tNOTE_ON:%d", i, touchpad_ptr->params.msg.midi.data1);
        #endif
      }
      break;
    case midi::AfterTouchPoly:
      break;
    case midi::ControlChange:
      touch_ptr->press.midi.data2 = 
        round(map(
          blob_ptr->centroid.z,
          Z_MIN,
          Z_MAX,
          touch_ptr->press.limit.min,
          touch_ptr->press.limit.max));
      midi_send_out(touch_ptr->press.midi);
      #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_SWITCHS)
        Serial.printf("\nDEBUG_MAPPINGS_SWITCHS\tID:%d\tC_CHANGE:%d", i, touch_ptr->press.midi.data2);
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

  if (blob_ptr->centroid.z != blob_ptr->last_centroid.z) {
    touch_ptr->press.midi.data2 = round(map(
        blob_ptr->centroid.z,
        Z_MIN,
        Z_MAX,
        touch_ptr->press.limit.min,
        touch_ptr->press.limit.max));
    midi_send_out(touch_ptr->press.midi);
    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_TOUCHPAD)
      Serial.printf("\nDEBUG_MAPPINGS_TOUCHPADS\tMIDI_Z_CC:%d\tVAL:%d",
      touch_ptr->press.midi.data2,
      map(blob_ptr->centroid.z, 0, 255, 0, 127));
    #endif
  };

};
