/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __SCAN_H__
#define __SCAN_H__

#include "blob.h"

typedef struct image_s image_t;   // Forward declaration located in blob.h
extern image_t rawFrame;        // Exposed local declaration see scan.cpp

void scan_setup(void);
void matrix_calibrate(void);
void matrix_scan(void);

#endif /*__SCAN_H__*/
