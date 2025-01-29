/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __INTERP_H__
#define __INTERP_H__

#include "blob.h"

extern uint8_t interpThreshold; // Exposed local declaration see interp.cpp
extern image_t interpFrame;     // Exposed local declaration see interp.cpp

void interp_setup(void);
void matrix_interp(void);

#endif /*__INTERP_H__*/
