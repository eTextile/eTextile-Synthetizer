/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapp_slider.h"

typedef struct mapp_slider_s mapp_slider_t;
struct mapp_slider_s {
  common_t common;
  slider_t params;
  uint8_t active_blob_count;
  uint8_t touch_index;
};

static mapp_slider_t mapp_sliders[MAX_SLIDERS];

llist_t llist_sliders_pool;

void mapping_sliders_alloc(uint8_t sliders_cnt) {
  llist_builder(&llist_sliders_pool, &mapp_sliders[0], sliders_cnt, sizeof(mapp_sliders[0]));
};

bool mapping_slider_is_blob_inside(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;
  if (blob_ptr->centroid.x > slider_ptr->params.rect.from.x &&
      blob_ptr->centroid.x < slider_ptr->params.rect.to.x &&
      blob_ptr->centroid.y > slider_ptr->params.rect.from.y &&
      blob_ptr->centroid.y < slider_ptr->params.rect.to.y) {
    return true;
  }
  return false;
};

// blob == valeurs physiqyes captées
// touch == données du nieme blob
bool mapping_slider_assign_blob(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;
  if (slider_ptr->touch_index < slider_ptr->params.touchs) {
    blob_ptr->action.mapping_ptr = slider_ptr;
    blob_ptr->action.mapping_data_ptr = &slider_ptr->params.touch[slider_ptr->touch_index++];
    slider_ptr->active_blob_count++;
    return true;
  }
  return false;
};

void mapping_slider_dispose_blob(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;
  blob_ptr->action.mapping_ptr = NULL;
  blob_ptr->action.mapping_data_ptr = NULL;
  if (--slider_ptr->active_blob_count == 0) {
    slider_ptr->touch_index = 0;
  };
};

void mapping_slider_play(blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)blob_ptr->action.mapping_ptr;
  touch_2d_t* touch_ptr = (touch_2d_t*)blob_ptr->action.mapping_data_ptr;
  //Serial.printf("\nDEBUG_MAPPINGS_SLIDERS\tTOUCHS:%d", slider_ptr->params.touchs);
    switch (slider_ptr->params.dir) {
      case HORIZONTAL:
        if (blob_ptr->centroid.x != blob_ptr->last_centroid.x) {
          touch_ptr->pos.midi.data2 = round(map(
            blob_ptr->centroid.x,
            slider_ptr->params.rect.from.x,
            slider_ptr->params.rect.to.x,
            touch_ptr->pos.limit.min,
            touch_ptr->pos.limit.max)
          );
          midi_send_out(touch_ptr->pos.midi);
          #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_SLIDERS)
            Serial.printf("\nDEBUG_MAPPINGS_SLIDERS\tID:%d\tVal:%d", i, touch_ptr->pos.midi.data2);
          #endif
        };
        break;
      case VERTICAL:
        if (blob_ptr->centroid.y != blob_ptr->last_centroid.y) {
          touch_ptr->pos.midi.data2 = round(map(
            blob_ptr->centroid.y,
            slider_ptr->params.rect.from.y,
            slider_ptr->params.rect.to.y,
            touch_ptr->pos.limit.min,
            touch_ptr->pos.limit.max)
          );
          midi_send_out(touch_ptr->pos.midi);
          #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_SLIDERS)
            Serial.printf("\nDEBUG_MAPPINGS_SLIDERS\tID:%d\tVal:%d", i, touch_ptr->pos.midi.data2);
          #endif   
        }
        break;
      };
      switch (touch_ptr->press.midi.type) {
        case midi::NoteOff:
          break;
        case midi::NoteOn:
          break;
        case midi::AfterTouchPoly:
          break;
        case midi::ControlChange:
          if (blob_ptr->centroid.z != blob_ptr->last_centroid.z) {
            /*
            round(map(
              blob_ptr->centroid.z,
              Z_MIN,
              Z_MAX,
              touch_ptr->press.limit.min,
              touch_ptr->press.limit.max)
            */
            touch_ptr->press.midi.data2 = blob_ptr->centroid.z;
            midi_send_out(touch_ptr->press.midi);
            #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPING_SLIDERS)
              Serial.printf("\nDEBUG_MAPPINGS_SLIDERS\tID:%d\tC_CHANGE:%d", i, mapp_grids->params.msg.midi.data2);
            #endif
          }
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
          // Not handled
          break;
    };
  //};
};

void mapping_slider_create(const JsonObject &config) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)llist_pop_front(&llist_sliders_pool);

  slider_ptr->params.rect.from.x = config["from"][0].as<float>();
  slider_ptr->params.rect.from.y = config["from"][1].as<float>();
  slider_ptr->params.rect.to.x = config["to"][0].as<float>();
  slider_ptr->params.rect.to.y = config["to"][1].as<float>();
  slider_ptr->params.touchs = config["touchs"].as<uint8_t>();

  midi_status_t status;
  for (uint8_t j = 0; j<slider_ptr->params.touchs; j++){
    midi_msg_status_unpack(config["msg"][j]["pos"]["midi"]["status"].as<uint8_t>(), &status);
    slider_ptr->params.touch[j].pos.midi.type = status.type;
    slider_ptr->params.touch[j].pos.midi.data1 = config["msg"][j]["pos"]["midi"]["data1"].as<uint8_t>();
    slider_ptr->params.touch[j].pos.midi.data2 = config["msg"][j]["pos"]["midi"]["data2"].as<uint8_t>();
    slider_ptr->params.touch[j].pos.midi.channel = status.channel;
    slider_ptr->params.touch[j].pos.limit.min = config["msg"][j]["pos"]["limit"]["min"].as<uint8_t>();
    slider_ptr->params.touch[j].pos.limit.max = config["msg"][j]["pos"]["limit"]["max"].as<uint8_t>();

    midi_msg_status_unpack(config["msg"][j]["press"]["midi"]["status"].as<uint8_t>(), &status);
    slider_ptr->params.touch[j].press.midi.type = status.type;
    slider_ptr->params.touch[j].press.midi.data1 = config["msg"][j]["press"]["midi"]["data1"].as<uint8_t>();
    slider_ptr->params.touch[j].press.midi.data2 = config["msg"][j]["press"]["midi"]["data2"].as<uint8_t>();
    slider_ptr->params.touch[j].press.midi.channel = status.channel;
    if (slider_ptr->params.touch[j].pos.midi.type == midi::ControlChange ||
      slider_ptr->params.touch[j].pos.midi.type == midi::AfterTouchPoly) {
      slider_ptr->params.touch[j].press.limit.min = config["msg"][j]["press"]["limit"]["min"].as<uint8_t>();
      slider_ptr->params.touch[j].press.limit.max = config["msg"][j]["press"]["limit"]["max"].as<uint8_t>();
    };  
    uint8_t size_x = slider_ptr->params.rect.to.x - slider_ptr->params.rect.from.x;
    uint8_t size_y = slider_ptr->params.rect.to.y - slider_ptr->params.rect.from.y;
    if (size_x < size_y) {
      slider_ptr->params.dir = VERTICAL; // TODO: pos -> dir
    } else {
      slider_ptr->params.dir = HORIZONTAL;
    };
  };

  slider_ptr->common.is_blob_inside_func_ptr = &mapping_slider_is_blob_inside;
  slider_ptr->common.blob_assign_func_ptr = &mapping_slider_assign_blob;
  slider_ptr->common.blob_dispose_func_ptr = &mapping_slider_dispose_blob;
  slider_ptr->common.play_func_ptr = &mapping_slider_play;

  llist_push_back(&llist_mappings, slider_ptr);
};
