/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "hardware_midi_io.h"

#include "config.h"
#include "llist.h"

MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI);

void hardware_midi_setup(void) {
  MIDI.begin(MIDI_INPUT_CHANNEL); // Launch MIDI hardware and listen to channel 1
  MIDI.setHandleMessage(hardware_midi_handle_input);
};

void hardware_midi_recive(void) {
  MIDI.read(MIDI_INPUT_CHANNEL);         // Is there any incoming MIDI messages on channel 1
  while (MIDI.read(MIDI_INPUT_CHANNEL)); // Read and discard any incoming MIDI messages
};

void hardware_midi_handle_input(const midi::Message<128u> &midiMsg) {
  // This can be refactored to avoide data copy !
  // I dont understand the midiMsg struct!
  // can it be casted or direct pushed to our I/O midi linked list?
  //midi_t* midiMsg = (midi_t*)dataPacket;
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  node_ptr->midi.type = midiMsg.type;       // Set the MIDI type
  node_ptr->midi.data1 = midiMsg.data1;     // Set the MIDI note
  node_ptr->midi.data2 = midiMsg.data2;     // Set the MIDI velocity
  node_ptr->midi.channel = midiMsg.channel; // Set the MIDI channel
  #if defined(MIDI_THRU)
    llist_push_front(&midiOut, node_ptr);      // Add the node to the midiOut linked liste
  #else
    llist_push_front(&midiIn, node_ptr);       // Add the node to the midiIn linked liste
  #endif
};

void hardware_midi_transmit(void){
  for (midiNode_t *node_ptr = (midiNode_t *)ITERATOR_START_FROM_HEAD(&midiOut); node_ptr != NULL; node_ptr = (midiNode_t *)ITERATOR_NEXT(node_ptr)){
    MIDI.send(node_ptr->midi.type, node_ptr->midi.data1, node_ptr->midi.data2, node_ptr->midi.channel);
  };
  llist_save_nodes(&midi_node_stack, &midiOut);
};
