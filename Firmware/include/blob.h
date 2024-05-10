/*
  FORKED FROM https://github.com/openmv/openmv/blob/master/src/omv/imlib/blob.c
    - This file is part of the OpenMV project.
    - Copyright (c) 2013- Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
    - This work is licensed under the MIT license, see the file LICENSE for details.

  Added custom blob détection algorithm to keep track of the blobs ID's
    - This patch is part of the eTextile-Synthesizer project - http://synth.eTextile.org
    - Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
    - This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __BLOB_H__
#define __BLOB_H__

#include "llist.h"

#define  BI   0  // [0] Blob UID
#define  BS   1  // [1] Blob State
#define  BL   2  // [2] Blob Last State
#define  BX   3  // [3] Blob X centroid position
#define  BY   4  // [4] Blob Y centroid position
#define  BZ   5  // [5] Blob Depth
#define  BW   6  // [6] Blob width
#define  BH   7  // [7] Blob Height

extern llist_t llist_blobs;            // Exposed local declaration see blob.cpp

typedef struct image image_t;
struct image {
  uint8_t* pData;
  uint8_t numCols;
  uint8_t numRows;
};

#define COMPUTE_IMAGE_ROW_PTR(pImage, y) \
  ({ \
    __typeof__ (pImage) _pImage = (pImage); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_pImage->pData) + (_pImage->numCols * _y); \
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

#define PIXEL_THRESHOLD(pixel, Threshold) \
  ({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (Threshold) _Threshold = (Threshold); \
    _pixel > _Threshold; \
  })

#define MIN(a, b)({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define MAX(a, b)({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

typedef struct xylr xylr_t;
struct xylr {
  lnode_t node;
  uint8_t x;
  uint8_t y;
  uint8_t l;
  uint8_t r;
  uint8_t t_l;
  uint8_t b_l;
};

typedef struct vertrice vertrice_t;
struct vertrice {
  float   x;
  float   y;
  uint8_t z;
};

typedef struct box box_t;
struct box {
  uint8_t W; // TODO Make it as float
  uint8_t H; // TODO Make it as float
};

typedef struct velocity velocity_t;
struct velocity {
  unsigned long timeStamp;
  float XY;
  float Z;
};

typedef enum status {
  FREE,
  NOT_FOUND,
  TO_REMOVE
} status_t;

typedef struct blob blob_t;
struct blob {
  lnode_t node;
  uint8_t UID;
  status_t status;
  uint32_t debounceTimeStamp;
  uint32_t transmitTimeStamp;
  uint16_t pixels;
  bool state;
  bool lastState;
  box_t box;
  vertrice_t centroid;
  velocity_t velocity;
};

void blob_setup(void);
void matrix_find_blobs(void);

#endif /*__BLOB_H__*/
