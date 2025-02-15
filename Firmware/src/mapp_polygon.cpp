/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// Algorithm: http://alienryderflex.com/polygon

#include "mapp_polygon.h"

struct mapp_polygon_s;
typedef struct mapp_polygon_s mapp_polygon_t;
struct mapp_polygon_s {
  common_t common;
  polygon_t params;
  uint8_t active_blob_count;
  uint8_t touch_index;
};

static mapp_polygon_t mapp_polygons[MAX_POLYGONS];

llist_t llist_polygons_pool;

void mapping_polygons_alloc(uint8_t polygons_cnt) {
  llist_builder(&llist_polygons_pool, &mapp_polygons[0], polygons_cnt, sizeof(mapp_polygons[0]));
};

void mapping_polygon_play(blob_t*);

// Test if the blob is within the polygon
bool mapping_polygon_is_blob_inside(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)mapping_ptr;
  int i, j = (polygon_ptr->params.point_cnt - 1);
  polygon_ptr->params.is_inside = false;
  for (i = 0; i < polygon_ptr->params.point_cnt; i++) {
    //float X1 = polygon_ptr->params.point[i].x;
    float Y1 = polygon_ptr->params.point[i].y;
    //float X2 = polygon_ptr->params.point[j].x;
    float Y2 = polygon_ptr->params.point[j].y;
    if ((Y1 < blob_ptr->centroid.y && Y2 >= blob_ptr->centroid.y) || (Y2 < blob_ptr->centroid.y && Y1 >= blob_ptr->centroid.y)) {
      // x ^= y; // equivalent to x = x ^ y;
      polygon_ptr->params.is_inside ^= ((blob_ptr->centroid.y * polygon_ptr->params.m[i] + polygon_ptr->params.c[i]) < blob_ptr->centroid.x);
    };
    j = i;
  };
  if (polygon_ptr->params.is_inside) {
    return true;
  }
  return false;
};

// blob == valeurs physiqyes captées
// touch == données du nieme blob
bool mapping_polygon_assign_blob(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)mapping_ptr;
  if (polygon_ptr->touch_index < polygon_ptr->params.touchs) {
    blob_ptr->action.mapping_ptr = polygon_ptr;
    blob_ptr->action.mapping_data_ptr = &polygon_ptr->params.touch[polygon_ptr->touch_index++];
    polygon_ptr->active_blob_count++;
    return true;
  }
  return false;
};

void mapping_polygon_dispose_blob(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)mapping_ptr;
  blob_ptr->action.mapping_ptr = NULL;
  blob_ptr->action.mapping_data_ptr = NULL;
  if (--polygon_ptr->active_blob_count == 0){
    polygon_ptr->touch_index = 0;
  };
};

// Use to detect if a blob is inside a polygon
// We can draw polygons to define zones et/ou zones overlaps playing MIDI_NOTES
void mapping_polygon_play(blob_t* blob_ptr) {
  //mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)blob_ptr->action.mapping_ptr;
  //touch_3d_t* touch_ptr = (touch_3d_t*)blob_ptr->action.touch_ptr;
  // TODO: get the max width & max height and scale it to [0-1]
  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_POLYGONS)
    Serial.printf("\nDEBUG_MAPPINGS_POLYGONS\tPoint %f %f is inside polygon %d\n", blob_ptr->centroid.x, blob_ptr->centroid.y, p);
  #endif
};

void mapping_polygon_create(const JsonObject &config) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)llist_pop_front(&llist_polygons_pool);
  
  polygon_ptr->params.point_cnt = config["cnt"].as<uint8_t>();
  for (uint8_t j = 0; j < polygon_ptr->params.point_cnt; j++) {
    polygon_ptr->params.point[j].x = config["point"][j]["X"].as<float>();
    polygon_ptr->params.point[j].y = config["point"][j]["Y"].as<float>();
  };
  // For line equation y = mx + c, we pre-compute m and c for all edges of a given polygon
  float x1, x2, y1, y2;
  uint8_t v1, v2 = (polygon_ptr->params.point_cnt - 1);
  polygon_ptr->params.is_inside = false;
  for (v1 = 0; v1 < polygon_ptr->params.point_cnt; v1++) {
    x1 = polygon_ptr->params.point[v1].x;
    y1 = polygon_ptr->params.point[v1].y;
    x2 = polygon_ptr->params.point[v2].x;
    y2 = polygon_ptr->params.point[v2].y;
    if (y2 == y1) {
      polygon_ptr->params.c[v1] = x1;
      polygon_ptr->params.m[v1] = 0;
    }
    else {
      polygon_ptr->params.c[v1] = x1 - (y1 * x2) / (y2 - y1) + (y1 * x1) / (y2 - y1);
      polygon_ptr->params.m[v1] = (x2 - x1) / (y2 - y1);
    };
    v2 = v1;
  }
  
  polygon_ptr->common.blob_assign_func_ptr = &mapping_polygon_assign_blob;
  polygon_ptr->common.blob_dispose_func_ptr = &mapping_polygon_dispose_blob;
  polygon_ptr->common.is_blob_inside_func_ptr = &mapping_polygon_is_blob_inside;
  polygon_ptr->common.play_func_ptr = &mapping_polygon_play;

  llist_push_back(&llist_mappings, polygon_ptr);
};
