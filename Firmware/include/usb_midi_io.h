/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __USB_MIDI_IO_H__
#define __USB_MIDI_IO_H__

#include <Arduino.h>
#include "midi_bus.h"

// E256 MIDI CONSTANTS

/*
typedef enum midi_types_e {
  NOTE_OFF = 0x8,     // TRIGGER WITH VELOCITY
  NOTE_ON = 0x9,      // PRESSURE ONLY
  P_AFTERTOUCH = 0xA, // TRIGGER AND PRESSURE
  C_CHANGE = 0xB,
  P_CHANGE = 0xC,
  C_AFTERTOUCH = 0xD,
  P_BEND = 0xE,
  SYS_EX = 0xF
} midi_types_t;
*/

#define SYSEX_CONF 0x7C  // DEC: 124
#define SYSEX_SOUND 0x6C // DEC: 108

extern uint32_t bootTime;

extern size_t sysEx_data_length;
extern uint8_t* sysEx_data_ptr;

void usb_midi_setup(void);
void usb_midi_recive(void);
void usb_midi_pending_mode_timeout(void);
void usb_midi_transmit(void);

void usb_midi_send_info(uint8_t msg, uint8_t channel);

void usb_read_noteOn(uint8_t channel, uint8_t note, uint8_t velocity);
void usb_read_noteOff(uint8_t channel, uint8_t note, uint8_t velocity);
void usb_read_controlChange(uint8_t channel, uint8_t control, uint8_t value);
void usb_read_programChange(uint8_t channel, uint8_t program);
void usb_read_systemExclusive(const uint8_t* data_ptr, uint16_t length, bool complete);
void usb_read_midi_clock();

#endif /*__USB_MIDI_IO_H__*/
