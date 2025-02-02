/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "midi_bus.h"
#include "blob.h"

#define MIDI_NODES 128

midi_node_t midi_nodes_array[MIDI_NODES] = {}; // Memory allocation for all MIDI I/O messages

llist_t midi_nodes_pool;                    // Main MIDI node stack
llist_t midi_in;                             // Main MIDI Input linked list
llist_t midi_out;                            // Main MIDI Output linked list
llist_t midi_chord;                          // Main MIDI chord linked list

// Extract MIDI type and channel from MIDI status msg
// https://www.midi.org/specifications-old/item/table-2-expanded-messages-list-status-bytes
void midi_msg_status_unpack(uint8_t in_status, midi_status_t* out_status) {
  out_status->type = (MidiType)((in_status >> 4) & 0xF); // Save the 4 MSB bits
  out_status->channel = (in_status & 0xF) + 1; // Save the 4 LSB bits [0000 === chan 1]
};

// Concatenate MIDI status msg from MIDI type and channel
uint8_t midi_msg_status_pack(MidiType type, uint8_t channel) {
  uint8_t status = (channel - 1) | (uint8_t)type;
  return status;
};

void midi_bus_setup(void) {
  llist_builder(&midi_nodes_pool, &midi_nodes_array[0], MIDI_NODES, sizeof(midi_nodes_array[0])); // Add X nodes to the midi_nodes_pool
  llist_raz(&midi_in);
  llist_raz(&midi_out);
  llist_raz(&midi_chord);
};

/*
void midi_handle_input(const midi::Message<128u> &midiMsg) {
  midi_node_t* node_ptr = (midi_node_t*)llist_pop_front(&midi_nodes_pool);  // Get a node from the MIDI nodes stack
  node_ptr->midiMsg.type = midiMsg.type;           // Set the MIDI type
  node_ptr->midiMsg.data1 = midiMsg.data1;         // Set the MIDI note
  node_ptr->midiMsg.data2 = midiMsg.data2;         // Set the MIDI velocity
  node_ptr->midiMsg.channel = midiMsg.channel;     // Set the MIDI channel
  llist_push_front(&midi_in, node_ptr);             // Add the node to the midi_in linked liste    
  //llist_push_front(&midi_nodes_pool, node_ptr);  // Save the node to the midi_nodes_pool linked liste
};
*/

void print_bytes(const uint8_t* data_ptr, size_t data_length) {
  Serial.printf("\nPRINT_BYTES / DATA_LENGTH: %d", data_length);
  Serial.printf("\nPRINT_BYTES / DATA: ");
  for (size_t i = 0; i < data_length; i++) {
    Serial.printf("%c", char(*data_ptr));
    data_ptr++;
  };
};
