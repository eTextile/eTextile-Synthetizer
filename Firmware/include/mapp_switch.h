#ifndef __MAPPING_SWITCH_H__
#define __MAPPING_SWITCH_H__

#include "mapping.h"

typedef struct switch_s switch_t;
struct switch_s {
  rect_t rect;
  msg_t msg;
};

void mapping_switchs_alloc(uint8_t switchs_cnt);
void mapping_switch_create(const JsonObject &config);

#endif /*__MAPPING_SWITCH_H__*/
