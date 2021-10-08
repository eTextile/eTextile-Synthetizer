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

// MIDI status bytes
#define MIDI_NOTE_ON            0x90
#define MIDI_NOTE_OFF           0x80
#define MIDI_CONTROL_CHANGE     0xB0
#define MIDI_SYSEX              0xF0

/*
  typedef enum status {
  MIDI_NOTE_OFF,
  MIDI_NOTE_ON,
  MIDI_CONTROL_CHANGE,
  MIDI_PROGRAM_CHANGE,
  MIDI_SYSEX
  } status_t;
*/

extern llist_t midi_node_stack;  // Exposed local declaration see midi_transmit.cpp
extern llist_t midiIn;           // Exposed local declaration see midi_transmit.cpp
extern llist_t midiOut;          // Exposed local declaration see midi_transmit.cpp
extern llist_t midiChord;        // MIDI chord linked list

typedef struct midiMsg midiMsg_t;
struct midiMsg {
  uint8_t status;   // Status message MIDI_NOTE_OFF, MIDI_NOTE_OFF, MIDI_CONTROL_CHANGE
  uint8_t data1;    // First value (0-127), controller number or note number
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
void handle_midi_input_cc(byte channel, byte control, byte value);
void handle_midi_input_noteOn(byte channel, byte note, byte velocity);
void handle_midi_input_noteOff(byte channel, byte note, byte velocity);
void midi_transmit(void);

#endif /*__MIDI_TRANSMIT_H__*/
