/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MEDIAN_H__
#define __MEDIAN_H__

#include "config.h"
#include "llist.h"
#include "blob.h"

#define MEDIAN_WINDOW    5     // Allowed filter window size [3-5-7]
#define M_WINDOW         (MEDIAN_WINDOW - 1)

typedef struct median median_t;
struct median {
  float X_rawVal[M_WINDOW];    // Blob X centroid input values
  uint8_t X_sort[M_WINDOW];    // Sorted index Array
  float Y_rawVal[M_WINDOW];    // Blob Y centroid input values
  uint8_t Y_sort[M_WINDOW];    // Sorted index Array
  uint8_t Z_rawVal[M_WINDOW];  // Blob Y centroid input values
  uint8_t Z_sort[M_WINDOW];    // Sorted index Array
  uint8_t count;
  uint8_t index;               // Circular buffer index
};

void running_median_setup(void);
void runing_median(void);

#endif /*__MEDIAN_H__*/
