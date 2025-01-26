#ifndef __MAPPING_SWITCH_H__
#define __MAPPING_SWITCH_H__

#include "mapping.h"

typedef struct switch_s switch_t;
struct switch_s {
  rect_t rect;
  msg_t msg;
  //bool state; // Do we nead it?
  //bool lastState; // Do we nead it?
};

void mapping_switchs_alloc(uint8_t switchs_cnt);
void mapping_switch_create(const JsonObject &config);
//bool mapping_switch_interact(blob_t* blob_ptr, common_t* common_ptr);
//void mapping_switch_play(blob_t* blob_ptr);

#endif /*__MAPPING_SWITCH_H__*/
