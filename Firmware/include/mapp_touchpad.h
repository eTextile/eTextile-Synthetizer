#ifndef __MAPPING_TOUCHPAD_H__
#define __MAPPING_TOUCHPAD_H__

#include "mapping.h"

typedef struct e256_touchpad touchpad_t;
struct e256_touchpad {
  rect_t rect;
  uint8_t touchs;
  touch_3d_t touch[MAX_TOUCHPAD_TOUCHS];
  uint8_t touchs_count;
};

void mapping_touchpads_alloc(uint8_t touchpads_cnt);
void mapping_touchpad_create(const JsonObject &config);
//bool mapping_touchpad_interact(blob_t* blob_ptr, common_t* common_ptr);
//void mapping_touchpad_play(blob_t* blob_ptr);

#endif /*__MAPPING_TOUCHPAD_H__*/