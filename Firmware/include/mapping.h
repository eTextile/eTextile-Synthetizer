/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
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

typedef enum dir_e {
  VERTICAL,
  HORIZONTAL
} dir_t;

extern llist_t llist_mappings;

typedef struct common_s common_t;

typedef bool interact_t(blob_t*, common_t*); 
typedef void play_t(blob_t*);

struct common_s {
  interact_t* interact_func_ptr;
  play_t* play_func_ptr;
};

void mapping_lib_update(void);

#endif /*__MAPPING_LIB_H__*/
