/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MAPPING_LIB_H__
#define __MAPPING_LIB_H__

#include "config.h"
#include "blob.h"
#include "llist.h"
#include "notes.h"
#include "midi_bus.h"

typedef struct center center_t;
struct center {
  float x;
  float y;
};

typedef struct knob knob_t;
struct knob {
  midiMsg_t midiMsg_radius;
  midiMsg_t midiMsg_theta;
  center_t center;
  float radius;
  float offset;
  //float lastRadius;
  //float lasttheta;
};

typedef struct point point_t;
struct point {
  float x;
  float y;
};

typedef struct rect rect_t;
struct rect {
  point_t from;
  point_t to;
};

typedef struct key keysroke_t;
struct key {
  midiMsg_t midiMsg;
  rect_t rect;
  bool state;
  bool lastState;
};

typedef struct slider slider_t;
struct slider {
  midiMsg_t midiMsg;
  rect_t rect;
  uint8_t dir; // H_SLIDER || V_SLIDER
  uint8_t min;
  uint8_t max;
  uint8_t last_val;
};

typedef struct cTrack cTrack_t;
struct cTrack {
  uint8_t sliders;
  uint8_t index;
  float offset;
};

typedef struct cSlider cSlider_t;
struct cSlider {
  midiMsg_t midiMsg;
  uint8_t id;
  float thetaMin;
  float thetaMax;
  float lastVal;
};

typedef struct polygon polygon_t;
struct polygon {
  midiMsg_t midiMsg;
  uint8_t point_cnt;
  point_t point[MAX_POLYGON_POINTS];
  float m[MAX_POLYGON_POINTS]; // 
  float c[MAX_POLYGON_POINTS]; //
  bool is_inside; 
};

typedef struct e256_touch e256_touch_t;
struct e256_touch {
  midiMsg_t midiMsg_x;
  midiMsg_t midiMsg_y;
  midiMsg_t midiMsg_z;
};

typedef struct touchpad touchpad_t;
struct touchpad {
  rect_t rect;
  uint8_t touchs;
  e256_touch_t touch[MAX_TOUCH_POINTS];
  uint8_t x_min; // TOUCHPAD MIDI X_MIN_VALUE
  uint8_t x_max; // TOUCHPAD MIDI X_MAX_VALUE
  uint8_t y_min; // TOUCHPAD MIDI Y_MIN_VALUE
  uint8_t y_max; // TOUCHPAD MIDI Y_MAX_VALUE
  uint8_t z_min; // TOUCHPAD MIDI Z_MIN_VALUE
  uint8_t z_max; // TOUCHPAD MIDI Z_MAX_VALUE
};

typedef struct grid grid_t;
struct grid {
  midiMsg_t midiLayout[MAX_GRID_KEYS];
  midiMsg_t *lastKeyPress[MAX_BLOBS];
  rect_t rect;
  uint8_t cols;
  uint8_t rows;
  uint8_t keys;
  //uint8_t gap;  // Move it as global constant!
  uint8_t mode;   // MIDI aftertouch -> Move to midiMsg!
  float scale_factor_x;
  float scale_factor_y;
};

extern uint8_t mapp_trigs;
extern keysroke_t *mapp_trigsParams;
void mapping_triggers_alloc(uint8_t trigs_cnt);

extern uint8_t mapp_switchs;
extern keysroke_t *mapp_switchParams;
void mapping_switchs_alloc(uint8_t switchs_cnt);

extern uint8_t mapp_sliders;
extern slider_t *mapp_slidersParams;
void mapping_sliders_alloc(uint8_t sliders_cnt);

extern uint8_t mapp_knobs;
extern knob_t *mapp_knobsParams;
void mapping_knobs_alloc(uint8_t knobs_cnt);

extern uint8_t mapp_polygons;
extern polygon_t *mapp_polygonsParams;
void mapping_polygons_alloc(uint8_t polygons_cnt);

extern uint8_t mapp_touchpads;
extern touchpad_t *mapp_touchpadsParams;
void mapping_touchpads_alloc(uint8_t touchpads_cnt);

// DEV
extern uint8_t mapp_grids;
extern grid_t *mapp_gridsParams;
void mapping_grids_alloc(uint8_t grids_cnt);

// DEV
extern uint8_t mapp_csliders;
extern slider_t *mapp_cslidersParams;
void mapping_sliders_alloc(uint8_t csliders_cnt);

void mapping_lib_setup(void);
void mapping_lib_update(void);

#endif /*__MAPPING_LIB_H__*/
