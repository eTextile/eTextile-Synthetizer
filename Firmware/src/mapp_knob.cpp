/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapp_knob.h"

typedef struct mapp_knob_s mapp_knob_t;
struct mapp_knob_s {
  common_t common;
  knob_t params;
  uint8_t active_blob_count;
  uint8_t touch_index;
};

static mapp_knob_t mapp_knobs[MAX_KNOBS];

llist_t llist_knobs_pool;

void mapping_knobs_alloc(uint8_t knobs_cnt) {
  llist_builder(&llist_knobs_pool, &mapp_knobs[0], knobs_cnt, sizeof(mapp_knobs[0]));
};

void mapping_knob_play(blob_t*);

bool mapping_knob_is_blob_inside(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_knob_t* knob_ptr = (mapp_knob_t*)mapping_ptr;
  if (blob_ptr->centroid.x > knob_ptr->params.rect.from.x &&
      blob_ptr->centroid.x < knob_ptr->params.rect.to.x &&
      blob_ptr->centroid.y > knob_ptr->params.rect.from.y &&
      blob_ptr->centroid.y < knob_ptr->params.rect.to.y) {
    return true;
  };
  return false;
};

// blob == valeurs physiqyes captées
// touch == données du nieme blob
bool mapping_knob_assign_blob(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_knob_t* knob_ptr = (mapp_knob_t*)mapping_ptr;
  if (knob_ptr->touch_index < knob_ptr->params.touchs) {
    blob_ptr->action.mapping_ptr = knob_ptr;
    blob_ptr->action.mapping_data_ptr = &knob_ptr->params.touch[knob_ptr->touch_index++];
    knob_ptr->active_blob_count++;
    return true;
  }
  return false;
};

void mapping_knob_dispose_blob(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_knob_t* knob_ptr = (mapp_knob_t*)mapping_ptr;
  blob_ptr->action.mapping_ptr = NULL;
  blob_ptr->action.mapping_data_ptr = NULL;
  if (--knob_ptr->active_blob_count == 0) {
    knob_ptr->touch_index = 0;
  };
};

void mapping_knob_play(blob_t* blob_ptr) {
  mapp_knob_t* knob_ptr = (mapp_knob_t*)blob_ptr->action.mapping_ptr;
  knob_touch_t* touch_ptr = (knob_touch_t*)blob_ptr->action.mapping_data_ptr;
  
  float x = blob_ptr->centroid.x - knob_ptr->params.center.x;
  float y = blob_ptr->centroid.y - knob_ptr->params.center.y;
  float radius = sqrt(x * x + y * y);

  if (radius < knob_ptr->params.radius) {
    // Rotation of Axes through an angle without shifting Origin
    float posX = x * cos(knob_ptr->params.offset) + y * sin(knob_ptr->params.offset);
    float posY = -x * sin(knob_ptr->params.offset) + y * cos(knob_ptr->params.offset);
    if (posX == 0 && 0 < posY) {
      touch_ptr->theta.midi.data2 = PiII;
    } else if (posX == 0 && posY < 0) {
      touch_ptr->theta.midi.data2 = IIIPiII;
    } else if (posX < 0) {
      touch_ptr->theta.midi.data2 = atanf(posY / posX) + PI;
    } else if (posY < 0) {
      touch_ptr->theta.midi.data2 = atanf(posY / posX) + IIPi;
    } else {
      touch_ptr->theta.midi.data2 = atanf(posY / posX);
    }
    midi_send_out(touch_ptr->radius.midi);
    midi_send_out(touch_ptr->theta.midi);
    midi_send_out(touch_ptr->pressure.midi);
    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_KNOBS)
      Serial.printf("\nDEBUG_MAPPINGS_KNOBS:\tKnobsID:\t%d\tradius:\t%fTheta:\t%f", i, knob_ptr->params.touch[j].radius.midi.data2, knob_ptr->params.touch[j].theta.midi.data2);
    #endif
  };
};

void mapping_knob_create(const JsonObject &config) {
  mapp_knob_t* knob_ptr = (mapp_knob_t*)llist_pop_front(&llist_knobs_pool);
  knob_ptr->params.rect.from.x = config["from"][0].as<float>();
  knob_ptr->params.rect.from.y = config["from"][1].as<float>();
  knob_ptr->params.rect.to.x = config["to"][0].as<float>();
  knob_ptr->params.rect.to.y = config["to"][1].as<float>();
  knob_ptr->params.radius = config["radius"].as<float>();
  knob_ptr->params.offset = config["offset"].as<uint8_t>();
  midi_status_t status;
  for (uint8_t j = 0; j<config["touchs"].as<uint8_t>(); j++){
    midi_msg_status_unpack(config["msg"][j]["radius"]["midi"]["status"].as<uint8_t>(), &status);
    knob_ptr->params.touch[j].radius.midi.type = status.type;
    knob_ptr->params.touch[j].radius.midi.data1 = config["msg"][j]["radius"]["midi"]["data1"].as<uint8_t>();
    knob_ptr->params.touch[j].radius.midi.data2 = config["msg"][j]["radius"]["midi"]["data2"].as<uint8_t>();
    knob_ptr->params.touch[j].radius.midi.channel = status.channel;
    if (knob_ptr->params.touch[j].radius.midi.type == midi::ControlChange ||
      knob_ptr->params.touch[j].radius.midi.type == midi::AfterTouchPoly) {
      knob_ptr->params.touch[j].radius.limit.min = config["msg"][j]["radius"]["limit"]["min"].as<uint8_t>();
      knob_ptr->params.touch[j].radius.limit.max = config["msg"][j]["radius"]["limit"]["max"].as<uint8_t>();
    }
    midi_msg_status_unpack(config["msg"][j]["theta"]["midi"]["status"].as<uint8_t>(), &status);
    knob_ptr->params.touch[j].theta.midi.type = status.type;
    knob_ptr->params.touch[j].theta.midi.data1 = config["msg"][j]["theta"]["midi"]["data1"].as<uint8_t>();
    knob_ptr->params.touch[j].theta.midi.data2 = config["msg"][j]["theta"]["midi"]["data2"].as<uint8_t>();
    knob_ptr->params.touch[j].theta.midi.channel = status.channel;
    if (knob_ptr->params.touch[j].theta.midi.type == midi::ControlChange ||
      knob_ptr->params.touch[j].theta.midi.type == midi::AfterTouchPoly) {
      knob_ptr->params.touch[j].theta.limit.min = config["msg"][j]["theta"]["limit"]["min"].as<uint8_t>();
      knob_ptr->params.touch[j].theta.limit.max = config["msg"][j]["theta"]["limit"]["max"].as<uint8_t>();
    }
    midi_msg_status_unpack(config["msg"][j]["pressure"]["midi"]["status"].as<uint8_t>(), &status);
    knob_ptr->params.touch[j].pressure.midi.type = status.type;
    knob_ptr->params.touch[j].pressure.midi.data1 = config["msg"][j]["press"]["midi"]["data1"].as<uint8_t>();
    knob_ptr->params.touch[j].pressure.midi.data2 = config["msg"][j]["press"]["midi"]["data2"].as<uint8_t>();
    knob_ptr->params.touch[j].pressure.midi.channel = status.channel;
    if (knob_ptr->params.touch[j].pressure.midi.type == midi::ControlChange ||
      knob_ptr->params.touch[j].pressure.midi.type == midi::AfterTouchPoly) {
      knob_ptr->params.touch[j].pressure.limit.min = config["msg"][j]["press"]["limit"]["min"].as<uint8_t>();
      knob_ptr->params.touch[j].pressure.limit.max = config["msg"][j]["press"]["limit"]["max"].as<uint8_t>();
    }
  }
  knob_ptr->params.radius = (knob_ptr->params.rect.to.x - knob_ptr->params.rect.from.x) / 2;
  knob_ptr->params.center.x = (knob_ptr->params.rect.from.x + knob_ptr->params.radius);
  knob_ptr->params.center.y = (knob_ptr->params.rect.from.y + knob_ptr->params.radius);
  
  knob_ptr->common.is_blob_inside_func_ptr = &mapping_knob_is_blob_inside;
  knob_ptr->common.blob_assign_func_ptr = &mapping_knob_assign_blob;
  knob_ptr->common.blob_dispose_func_ptr = &mapping_knob_dispose_blob;
  knob_ptr->common.play_func_ptr = &mapping_knob_play;

  llist_push_back(&llist_mappings, knob_ptr);
};