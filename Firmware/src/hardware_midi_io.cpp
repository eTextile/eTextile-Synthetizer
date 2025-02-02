/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
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
  //midi_msg_t* midiMsg = (midi_msg_t*)dataPacket;
  midi_node_t* node_ptr = (midi_node_t*)llist_pop_front(&midi_nodes_pool);  // Get a node from the MIDI nodes stack
  node_ptr->midi.type = midiMsg.type;       // Set the MIDI type
  node_ptr->midi.data1 = midiMsg.data1;     // Set the MIDI note
  node_ptr->midi.data2 = midiMsg.data2;     // Set the MIDI velocity
  node_ptr->midi.channel = midiMsg.channel; // Set the MIDI channel
  #if defined(MIDI_THRU)
    llist_push_front(&midi_out, node_ptr);      // Add the node to the midi_out linked list
  #else
    llist_push_front(&midi_in, node_ptr);       // Add the node to the midi_in linked list
  #endif
};

void hardware_midi_transmit(void){
  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&midi_out); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)){
    midi_node_t* midi_node_ptr = (midi_node_t*)ITERATOR_DATA(node_ptr);
    MIDI.send(midi_node_ptr->midi.type, midi_node_ptr->midi.data1, midi_node_ptr->midi.data2, midi_node_ptr->midi.channel);
  };
  llist_concat_nodes(&midi_nodes_pool, &midi_out);
};
