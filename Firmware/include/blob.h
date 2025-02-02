/*
  FORKED FROM https://github.com/openmv/openmv/blob/master/src/omv/imlib/blob.c
    - This file is part of the OpenMV project.
    - Copyright (c) 2013- Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
    - This work is licensed under the MIT license, see the file LICENSE for details.

  Added custom blob détection algorithm to keep track of the blobs ID's
    - This patch is part of the eTextile-Synthesizer project - https://synth.eTextile.org
    - Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
    - This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __BLOB_H__
#define __BLOB_H__

#include "llist.h"

extern llist_t llist_blobs;                   // Intermediate blobs linked list

typedef enum blob_params_e {
  BI, // [0] Blob UID
  BS, // [1] Blob State
  BL, // [2] Blob Last State
  BX, // [3] Blob X centroid position
  BY, // [4] Blob Y centroid position
  BZ, // [5] Blob Depth
  BW, // [6] Blob width
  BH  // [7] Blob Height
} blob_params_t;

//extern llist_t llist_previous_blobs; // Exposed local declaration see blob.cpp

typedef struct image_s image_t;
struct image_s {
  uint8_t* data_ptr;
  uint8_t num_cols;
  uint8_t num_rows;
};

#define COMPUTE_IMAGE_ROW_PTR(image_ptr, y) \
  ({ \
    __typeof__ (image_ptr) _image_ptr = (image_ptr); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_image_ptr->data_ptr) + (_image_ptr->num_cols* _y); \
  })

#define IMAGE_GET_PIXEL_FAST(row_ptr, x) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x]; \
  })

#define IMAGE_SET_PIXEL_FAST(row_ptr, x, v) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    __typeof__ (v) _v = (v); \
    _row_ptr[_x] = _v; \
  })

#define PIXEL_THRESHOLD(pixel, threshold) \
  ({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (threshold) _threshold = (threshold); \
    _pixel > _threshold; \
  })

#define MIN(a, b)({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define MAX(a, b)({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

typedef struct xylr_s xylr_t;
struct xylr_s {
  uint8_t x;
  uint8_t y;
  uint8_t l;
  uint8_t r;
  uint8_t t_l;
  uint8_t b_l;
};

typedef struct vertrice_s vertrice_t;
struct vertrice_s {
  float x;
  float y;
  uint8_t z;
};

typedef struct box_s box_t;
struct box_s {
  uint8_t w; // TODO: make it as float // use from 
  uint8_t h; // TODO: make it as float // use to
};

typedef struct velocity_s velocity_t;
struct velocity_s {
  unsigned long timeStamp;
  float xy;
  float z;
};

/*
typedef enum blob_status_e{
  FREE,
  NOT_FOUND, // INACTIVE
  TO_REMOVE 
} blob_status_t;
*/

typedef enum blob_status_e {
  NEW,
  PRESENT,
  MISSING
} blob_status_t;

typedef struct blob_s blob_t;

typedef void blob_action_func_t(blob_t*);

typedef struct blob_action_s blob_action_t;
struct blob_action_s {
  blob_action_func_t* func_ptr;
  void* mapping_ptr;
  void* touch_ptr;
  //llist touchs; // TODO
};

struct blob_s {
  uint8_t UID;
  blob_status_t status;
  uint32_t debounce_time_stamp; // time to leav
  uint32_t transmit_time_stamp;
  box_t box;
  uint16_t pixels;
  bool state;
  bool last_state;
  vertrice_t centroid;
  vertrice_t last_centroid;
  velocity_t velocity;
  blob_action_t action;
};

void blob_setup(void);
void matrix_find_blobs(void);
bool is_blob_existing(blob_t* blob_ptr, blob_t* new_blob_ptr);

#endif /*__BLOB_H__*/
