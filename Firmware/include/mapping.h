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

#include <ArduinoJson.h>

typedef struct point_s point_t;
struct point_s {
  float x;
  float y;
};

typedef struct rect_s rect_t;
struct rect_s {
  point_t from;
  point_t to;
};

typedef struct touch_2d_s touch_2d_t;
struct touch_2d_s {
  msg_t pos;
  msg_t press;
};

typedef struct touch_3d_s touch_3d_t;
struct touch_3d_s {
  msg_t pos_x;
  msg_t pos_y;
  msg_t press;
};


typedef enum {
  VERTICAL,
  HORIZONTAL
} dir_t;

/*
typedef struct cTrack_s cTrack_t;
struct cTrack_s {
  uint8_t sliders;
  uint8_t index;
  float offset;
};
*/

/*
typedef struct cSlider_s cSlider_t;
struct cSlider_s {
  midi_t midiMsg;
  uint8_t id;
  float thetaMin;
  float thetaMax;
  float lastVal;
};
*/

extern llist_t llist_controls;

typedef struct mapp_common_s common_t;
typedef bool mapping_test_t(blob_t*, common_t*); 
typedef void mapping_play_t(blob_t*, common_t*);

struct mapp_common_s {
  blob_t* blob_ptr;
  mapping_test_t* test_ptr;
  mapping_play_t* play_ptr;
};

void mapping_lib_update(void);

#endif /*__MAPPING_LIB_H__*/
