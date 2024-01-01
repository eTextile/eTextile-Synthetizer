/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MIDI_TRANSMIT_H__
#define __MIDI_TRANSMIT_H__

#include "config.h"
#include "llist.h"
#include "blob.h"


#include <MIDI.h>  // https://github.com/FortySevenEffects/arduino_midi_library > https://github.com/PaulStoffregen/MIDI
using namespace midi;

extern llist_t midi_node_stack;  // Exposed local declaration see midi_transmit.cpp
extern llist_t midiIn;           // Exposed local declaration see midi_transmit.cpp
extern llist_t midiOut;          // Exposed local declaration see midi_transmit.cpp
extern llist_t midiChord;        // Exposed local declaration see midi_transmit.cpp

typedef struct midiMsg midiMsg_t;
struct midiMsg {
  uint8_t status;   // For MIDI status bytes see: https://github.com/PaulStoffregen/MIDI/blob/master/src/midi_Defs.h
  uint8_t data1;    // First value  (0-127), controller number or note number
  uint8_t data2;    // Second value (0-127), controller value or velocity
  uint8_t channel;  // MIDI channel (0-15)
};

typedef struct midiNode midiNode_t;
struct midiNode {
  lnode_t node;
  midiMsg_t midiMsg;
};

void llist_midi_init(llist_t* llist_ptr, midiNode_t* nodeArray_ptr, const int nodes);

void MIDI_TRANSMIT_SETUP(void);
void read_midi_input(void);
void handle_midi_input(const midi::Message<128u> &midiMsg);
void midi_transmit(void);

#endif /*__MIDI_TRANSMIT_H__*/
