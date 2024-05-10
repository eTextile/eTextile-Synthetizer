/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MIDI_BUS_H__
#define __MIDI_BUS_H__

#include "config.h"
#include "llist.h"

#include <MIDI.h>  // https://github.com/FortySevenEffects/arduino_midi_library > https://github.com/PaulStoffregen/MIDI
using namespace midi;

#define MIDI_NODES      128

#define NOTE_OFF      0x8 // NOTE_OFF
#define NOTE_ON       0x9 // NOTE_ON
#define P_AFTERTOUCH  0xA // POLYPHONIC_AFTERTOUCH
#define C_CHANGE      0xB // CONTROL_CHANGE
#define P_CHANGE      0xC // PROGRAM_CHANGE
#define C_AFTERTOUCH  0xD // CHANNEL_AFTERTOUCH
#define P_BEND        0xE // PITCH_BEND
#define SYS_EX        0xF // SYSTEM_EXCLUSIVE

/*
const MIDI_TYPES = {
  const 0x8 "NOTE_OFF";      // NOTE_OFF
  const 0x9: "NOTE_ON",      // NOTE_ON
  const 0xA: "P_AFTERTOUCH", // POLYPHONIC_AFTERTOUCH
  const 0xB: "C_CHANGE",     // CONTROL_CHANGE
  const 0xC: "P_CHANGE",     // PROGRAM_CHANGE
  const 0xD: "C_AFTERTOUCH", // CHANNEL_AFTERTOUCH
  const 0xE: "P_BEND",       // PITCH_BEND
  const 0xF: "SYS_EX"        // SYSTEM_EXCLUSIVE
};
*/

extern llist_t midi_node_stack;                    // Main MIDI node stack
extern llist_t midiIn;                             // Main MIDI Input linked list
extern llist_t midiOut;                            // Main MIDI Output linked list
extern llist_t midiChord;                          // Main MIDI chord linked list

// For MIDI status bytes see: https://github.com/PaulStoffregen/MIDI/blob/master/src/midi_Defs.h
typedef struct midiMsg midi_t;
struct midiMsg {
  uint8_t type;     // (Extract from status byte)
  uint8_t data1;    // [0-127] MIDI controller number or note number
  uint8_t data2;    // [0-127] MIDI controller value or velocity
  uint8_t channel;  // [1-15] MIDI channel (Extract from status byte)
};

typedef struct limit limit_t;
struct limit {
  uint8_t min; // [0-127]
  uint8_t max; // [0-127]
};

typedef struct msg msg_t;
struct msg {
  midi_t midi;
  limit_t limit;
  uint8_t last_val;
};

typedef struct midiNode midiNode_t;
struct midiNode {
  lnode_t node;
  midi_t midi;
};

typedef struct midi_status {
  uint8_t type;
  uint8_t channel;
} midi_status_t;

void midi_bus_setup(void);
uint8_t midi_msg_status_pack(uint8_t type, uint8_t channel);
void midi_msg_status_unpack(uint8_t in_status, midi_status_t* out_status);
void printBytes(uint8_t* data_ptr, uint16_t length);

inline void midi_sendOut(midi_t midiMsg){
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);
  node_ptr->midi = midiMsg;
  llist_push_front(&midiOut, node_ptr);
};

#endif /*__MIDI_BUS_H__*/
