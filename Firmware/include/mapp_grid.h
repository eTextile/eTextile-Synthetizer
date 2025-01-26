#ifndef __MAPPING_GRID_H__
#define __MAPPING_GRID_H__

#include "mapping.h"
#include "mapp_switch.h"

typedef struct grid_s grid_t;
struct grid_s {
  rect_t rect;
  uint8_t cols;
  uint8_t rows;
  uint8_t keys_count;
  switch_t keys[MAX_GRID_KEYS];
  uint8_t touchs;
  switch_t* last_keys_ptr[MAX_BLOBS];
  //key_mode_t mode;
  float scale_factor_x;
  float scale_factor_y;
};

void mapping_grids_alloc(uint8_t grids_cnt);
void mapping_grid_create(const JsonObject &config);

#endif /*__MAPPING_GRID_H__*/
