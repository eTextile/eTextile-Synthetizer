/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MAPPING_LIB_H__
#define __MAPPING_LIB_H__

#include "config.h"
#include "blob.h"
#include "llist.h"
#include "midi_bus.h"

typedef struct e256_point point_t;
struct e256_point {
  float x;
  float y;
};

typedef struct e256_rect rect_t;
struct e256_rect {
  point_t from;
  point_t to;
};

typedef struct e256_2d_touch touch_2d_t;
struct e256_2d_touch {
  msg_t pos;
  msg_t press;
};

typedef struct e256_3d_touch touch_3d_t;
struct e256_3d_touch {
  msg_t pos_x;
  msg_t pos_y;
  msg_t press;
};

typedef struct e256_knob_touch touch_param_t;
struct e256_knob_touch {
  msg_t radius;
  msg_t theta;
  msg_t pressure;
};

typedef struct e256_knob knob_t;
struct e256_knob {
  rect_t rect;
  float offset;
  point_t center;
  float radius;
  uint8_t touchs;
  touch_param_t touch[MAX_KNOB_TOUCHS]; // TODO: replace with MAX_BLOBS
};

typedef struct e256_switch switch_t;
struct e256_switch {
  rect_t rect;
  blob_t* touch;
  msg_t msg;
  //bool state; // Do we nead it?
  //bool lastState; // Do we nead it?
};

typedef enum {
  VERTICAL,
  HORIZONTAL
} dir_t;

typedef struct e256_slider slider_t;
struct e256_slider {
  rect_t rect;
  dir_t pos;
  uint8_t touchs;
  touch_2d_t touch[MAX_SLIDER_TOUCHS]; // TODO: replace with MAX_BLOBS
};

typedef struct cTrack cTrack_t;
struct cTrack {
  uint8_t sliders;
  uint8_t index;
  float offset;
};

typedef struct cSlider cSlider_t;
struct cSlider {
  midi_t midiMsg;
  uint8_t id;
  float thetaMin;
  float thetaMax;
  float lastVal;
};

typedef struct e256_polygon polygon_t;
struct e256_polygon {
  msg_t msg;
  uint8_t point_cnt;
  point_t point[MAX_POLYGON_POINTS];
  float m[MAX_POLYGON_POINTS]; // 
  float c[MAX_POLYGON_POINTS]; //
  bool is_inside;
};

typedef struct e256_touchpad touchpad_t;
struct e256_touchpad {
  rect_t rect;
  uint8_t touchs;
  touch_3d_t touch[MAX_TOUCHPAD_TOUCHS];
  uint8_t touchs_count;
};

typedef struct e256_grid grid_t;
struct e256_grid {
  rect_t rect;
  uint8_t cols;
  uint8_t rows;
  uint8_t keys_count;
  switch_t keys[MAX_GRID_KEYS];
  switch_t* last_keys_ptr[MAX_BLOBS];
  //key_mode_t mode;
  float scale_factor_x;
  float scale_factor_y;
};

extern uint8_t mapp_switchs;
extern switch_t* mapp_switch_params;
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
//extern uint8_t mapp_csliders;
//extern slider_t *mapp_cslidersParams;
//void mapping_sliders_alloc(uint8_t csliders_cnt);

void mapping_lib_setup(void);
void mapping_lib_update(void);

#endif /*__MAPPING_LIB_H__*/
