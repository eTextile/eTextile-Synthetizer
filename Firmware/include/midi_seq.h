/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MIDI_SEQ_H__
#define __MIDI_SEQ_H__

#include "config.h"
#include "llist.h"
#include "blob.h"
#include "midi_bus.h"

typedef struct seq seq_t;
struct seq {
  uint16_t intervalTime[];
  uint8_t* seqframe;
};

void tapTempo(void);      // TODO
void stepSequencer(void); // TODO
void arpeggiator(void);   // TODO

#endif /*__MIDI_SEQ_H__*/
