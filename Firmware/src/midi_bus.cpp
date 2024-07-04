/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "midi_bus.h"
#include "blob.h"

midiNode_t midiNodeArray[MIDI_NODES] = {0}; // Memory allocation for all MIDI I/O messages

llist_t midi_node_stack;                    // Main MIDI node stack
llist_t midiIn;                             // Main MIDI Input linked list
llist_t midiOut;                            // Main MIDI Output linked list
llist_t midiChord;                          // Main MIDI chord linked list

void llist_midi_init(llist_t* llist_ptr, midiNode_t* nodesArray_ptr, const int nodes) {
  llist_raz(llist_ptr);
  for (int i = 0; i < nodes; i++) {
    llist_push_front(llist_ptr, &nodesArray_ptr[i]);
  };
};

// MIDI struct
// https://www.midi.org/specifications-old/item/table-2-expanded-messages-list-status-bytes
uint8_t midi_msg_status_pack(uint8_t type, uint8_t channel) {
  uint8_t status = (channel - 1) | (type << 4); 
  return status;
};

void midi_msg_status_unpack(uint8_t in_status, midi_status_t* out_status) {
  out_status->type = (in_status >> 4) & 0xF; // Save the 4 MSB bits
  out_status->channel = (in_status & 0xF) + 1; // Save the 4 LSB bits [0000 === chan 1]
};

void midi_bus_setup(void) {
  llist_midi_init(&midi_node_stack, &midiNodeArray[0], MIDI_NODES); // Add X nodes to the midi_node_stack
  llist_raz(&midiIn);
  llist_raz(&midiOut);
  llist_raz(&midiChord);
};

/*
void midi_handle_input(const midi::Message<128u> &midiMsg) {
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  node_ptr->midiMsg.type = midiMsg.type;           // Set the MIDI status
  node_ptr->midiMsg.data1 = midiMsg.data1;         // Set the MIDI note
  node_ptr->midiMsg.data2 = midiMsg.data2;         // Set the MIDI velocity
  node_ptr->midiMsg.channel = midiMsg.channel;     // Set the MIDI channel
  llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste    
  //llist_push_front(&midi_node_stack, node_ptr);  // Save the node to the midi_node_stack linked liste
};
*/

void printBytes(const uint8_t* data_ptr, uint32_t length) {
  Serial.printf("\nPRINT_BYTES / DATA_LENGTH: %d", length);
  Serial.printf("\nPRINT_BYTES / DATA: ");
  while (length > 0) {
    uint8_t b = *data_ptr++;
    Serial.printf("%c", char(b));
    length--;
  };
};
