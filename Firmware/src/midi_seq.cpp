/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "midi_seq.h"

typedef struct seq seq_t;
struct seq {
  //uint32_t intervalTime[32];
  uint8_t* seqframe;
};

void tapTempo(void) {
  for (midiNode_t* midiNode_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiOut); midiNode_ptr != NULL; midiNode_ptr = (midiNode_t*)ITERATOR_NEXT(midiNode_ptr)) {
    // TODO
  };
};

// TODO
void stepSequencer(void) {
};

// TODO
void arpeggiator(void) {
};
