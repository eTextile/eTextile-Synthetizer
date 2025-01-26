#ifndef __MAPPING_SLIDER_H__
#define __MAPPING_SLIDER_H__

#include "mapping.h"

typedef struct slider_s slider_t;
struct slider_s {
  rect_t rect;
  dir_t pos;
  uint8_t touchs;
  touch_2d_t touch[MAX_SLIDER_TOUCHS];
};

void mapping_sliders_alloc(uint8_t sliders_cnt);
void mapping_slider_create(const JsonObject &config);
//bool mapping_slider_interact(blob_t* blob_ptr, common_t* common_ptr);
//void mapping_slider_play(blob_t* blob_ptr);

#endif /*__MAPPING_SLIDER_H__*/
