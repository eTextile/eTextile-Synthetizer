#ifndef __MAPPING_TOUCHPAD_H__
#define __MAPPING_TOUCHPAD_H__

#include "mapping.h"

typedef struct touchpad_s touchpad_t;
struct touchpad_s {
  rect_t rect;
  uint8_t touchs;
  touch_3d_t touch[MAX_TOUCHPAD_TOUCHS];
};

void mapping_touchpads_alloc(uint8_t touchpads_cnt);
void mapping_touchpad_create(const JsonObject &config);

#endif /*__MAPPING_TOUCHPAD_H__*/