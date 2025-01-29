/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
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
  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&midiOut); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    midiNode_t* midiNode_ptr = (midiNode_t*)ITERATOR_DATA(node_ptr);
    // TODO
  };
};

// TODO
void stepSequencer(void) {
};

// TODO
void arpeggiator(void) {
};
