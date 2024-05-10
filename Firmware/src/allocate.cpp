/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "allocate.h"

void* allocate(void* data_ptr, uint16_t size) {
  if (data_ptr == NULL){
    data_ptr = malloc(size * sizeof(uint8_t));
    return data_ptr;
    } else {
    data_ptr = realloc(data_ptr, size * sizeof(uint8_t));
    return data_ptr;
  };
};
