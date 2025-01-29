#ifndef __MAPPING_KNOB_H__
#define __MAPPING_KNOB_H__

#include "mapping.h"

typedef struct knob_touch_s knob_touch_t;
struct knob_touch_s {
  msg_t radius;
  msg_t theta;
  msg_t pressure;
};

typedef struct knob_s knob_t;
struct knob_s {
  rect_t rect;
  point_t center;
  float radius; // IN PROGRESS
  float offset;
  uint8_t touchs;
  knob_touch_t touch[MAX_KNOB_TOUCHS];
};

void mapping_knobs_alloc(uint8_t knobs_cnt);
void mapping_knob_create(const JsonObject &config);

#endif /*__MAPPING_KNOB_H__*/
