/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __PLAYER_GRANULAR_H__
#define __PLAYER_GRANULAR_H__

#include "config.h"
#include "llist.h"
#include "blob.h"
#include "midi_bus.h"

#include <Audio.h>         // https://github.com/PaulStoffregen/Audio
#include <Wire.h>          // https://github.com/PaulStoffregen/Wire
#include <SPI.h>           // https://github.com/PaulStoffregen/SPI
#include <SD.h>            // https://github.com/PaulStoffregen/SD
#include <SerialFlash.h>   // https://github.com/PaulStoffregen/SerialFlash

void PLAYER_GRANULAR_SETUP(void);
void player_granular(void);

#endif /*__PLAYER_GRANULAR_H__*/