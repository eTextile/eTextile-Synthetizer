#ifndef __MAPPING_SLIDER_H__
#define __MAPPING_SLIDER_H__

#include "mapping.h"

typedef struct slider_s slider_t;
struct slider_s {
  rect_t rect;
  dir_t dir;
  uint8_t touchs;
  // TODO: add llist of touchs
  //llist_t touch;
  touch_2d_t touch[MAX_SLIDER_TOUCHS];
};

void mapping_sliders_alloc(uint8_t sliders_cnt);
void mapping_slider_create(const JsonObject &config);

#endif /*__MAPPING_SLIDER_H__*/
