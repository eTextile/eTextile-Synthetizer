/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "midi_transmit.h"

#define MIDI_TRANSMIT_INTERVAL 15
unsigned long int usbTransmitTimeStamp = 0;

#if MIDI_HARDWARE
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI);
#endif

#define MIDI_NODES 64

midiNode_t midiNodeArray[MIDI_NODES] = {0}; // Memory allocation for all MIDI I/O messages

llist_t midi_node_stack;                    // Main MIDI node stack
llist_t midiIn;                             // Main MIDI Input linked list
llist_t midiOut;                            // Main MIDI Output linked list
llist_t midiChord;                          // Main MIDI chord linked list

void llist_midi_init(llist_t* llist_ptr, midiNode_t* nodeArray_ptr, const int nodes) {
  llist_raz(llist_ptr);
  for (int i = 0; i < nodes; i++) {
    llist_push_front(llist_ptr, &nodeArray_ptr[i]);
  };
};

void e256_controlChange(byte channel, byte control, byte value){
   midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
   node_ptr->midiMsg.status = midi::ControlChange;  // Set the MIDI status
   node_ptr->midiMsg.data1 = control;               // Set the MIDI note
   node_ptr->midiMsg.data2 = value;                 // Set the MIDI velocity
   //node_ptr->midiMsg.channel = midiMsg.channel;   // Set the MIDI channel
   llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste
};

void MIDI_TRANSMIT_SETUP(void) {
#if MIDI_TRANSMIT
  usbMIDI.begin();
  //usbMIDI.setHandleMessage(handle_midi_input);
  usbMIDI.setHandleControlChange(e256_controlChange);
#endif
#if MIDI_HARDWARE
  MIDI.begin(MIDI_INPUT_CHANNEL); // Launch MIDI and listen to channel 1
  MIDI.setHandleMessage(handle_midi_input);
#endif
  llist_midi_init(&midi_node_stack, &midiNodeArray[0], IDI_NODES); // Add X nodes to the midi_node_stack
  llist_raz(&midiIn);
  llist_raz(&midiOut);
  llist_raz(&midiChord);
};

void read_midi_input(void) {
#if MIDI_TRANSMIT
  usbMIDI.read(MIDI_INPUT_CHANNEL);         // Is there a MIDI incoming messages on channel One
  while (usbMIDI.read(MIDI_INPUT_CHANNEL)); // Read and discard any incoming MIDI messages
#endif
#if MIDI_HARDWARE
  MIDI.read(MIDI_INPUT_CHANNEL);            // Is there a MIDI incoming messages on channel One
  while (MIDI.read(MIDI_INPUT_CHANNEL));    // Read and discard any incoming MIDI messages
#endif
};

void handle_midi_input(const midi::Message<128u> &midiMsg) {
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  switch (midiMsg.type) {
    case midi::NoteOn:
      node_ptr->midiMsg.status = midi::NoteOn;         // Set the MIDI status
      node_ptr->midiMsg.data1 = midiMsg.data1;         // Set the MIDI note
      node_ptr->midiMsg.data2 = midiMsg.data2;         // Set the MIDI velocity
      //node_ptr->midiMsg.channel = midiMsg.channel;   // Set the MIDI channel
      llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste
      break;
    case midi::NoteOff:
      node_ptr->midiMsg.status = midi::NoteOff;        // Set the MIDI status
      node_ptr->midiMsg.data1 = midiMsg.data1;         // Set the MIDI note
      node_ptr->midiMsg.data2 = midiMsg.data2;         // Set the MIDI velocity
      //node_ptr->midiMsg.channel = midiMsg.channel;   // Set the MIDI channel
      llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste
      break;
    case midi::ControlChange:
      node_ptr->midiMsg.status = midi::ControlChange;  // Set the MIDI status
      node_ptr->midiMsg.data1 = midiMsg.data1;         // Set the MIDI note
      node_ptr->midiMsg.data2 = midiMsg.data2;         // Set the MIDI velocity
      //node_ptr->midiMsg.channel = midiMsg.channel;   // Set the MIDI channel
      llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste
      break;
    case midi::Clock:
      node_ptr->midiMsg.status = midi::Clock;          // Set the MIDI status
      //node_ptr->midiMsg.data1 = midiMsg.data1;       // Set the MIDI note
      //node_ptr->midiMsg.data2 = midiMsg.data2;       // Set the MIDI velocity
      //node_ptr->midiMsg.channel = midiMsg.channel;   // Set the MIDI channel
      llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste
      break;
    default:
      llist_push_front(&midi_node_stack, node_ptr);    // Add the node to the midi_node_stack linked liste
      break;
  };
};

void midi_transmit(void) {
  switch (currentMode) {
    case RAW_MATRIX:
      if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
        usbTransmitTimeStamp = millis();
#if MIDI_TRANSMIT
        usbMIDI.sendSysEx(RAW_FRAME, rawFrame.pData, false, 0);
        usbMIDI.send_now();
#endif
      };
      break;
    case INTERP_MATRIX:
      if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
        usbTransmitTimeStamp = millis();
#if MIDI_TRANSMIT
        // NOT_WORKING > You can use OSC insted of MIDI!
        // See https://forum.pjrc.com/threads/28282-How-big-is-the-MIDI-receive-buffer
        //usbMIDI.sendSysEx(NEW_FRAME, interpFrame.pData, false, 0);
        //usbMIDI.send_now();
#endif
      };
      break;
    case BLOBS_PLAY:
      // Send all blobs values over USB using MIDI format
#if MIDI_TRANSMIT
      for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
        if (blob_ptr->state) {
          if (!blob_ptr->lastState) {
            usbMIDI.sendNoteOn(blob_ptr->UID + 1, 1, BS); // sendNoteOn(note, velocity, channel);
#if DEBUG_MIDI_TRANSMIT
            Serial.printf("\nDEBUG_MIDI_TRANSMIT\tNOTE_ON: %d", blob_ptr->UID + 1);
#endif
            usbMIDI.send_now();
            //while (usbMIDI.read()); // Read and discard any incoming MIDI messages
          } else {
            if (millis() - blob_ptr->transmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
              blob_ptr->transmitTimeStamp = millis();
              // usbMIDI.sendControlChange(control, value, channel);
              usbMIDI.sendControlChange(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BZ, constrain(blob_ptr->centroid.Z, 0, 127), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BW, blob_ptr->box.W, blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BH, blob_ptr->box.H, blob_ptr->UID + 1);
#if DEBUG_MIDI_TRANSMIT
              Serial.printf("\nDEBUG_MIDI_TRANSMIT\tCONTROL_CHANGE: %d", blob_ptr->UID);
#endif
            };
          };
        } else {
          if (blob_ptr->lastState && blob_ptr->status != NOT_FOUND) {
            usbMIDI.sendNoteOff(blob_ptr->UID + 1, 0, BS); // sendNoteOff(note, velocity, channel);
#if DEBUG_MIDI_TRANSMIT
            Serial.printf("\nDEBUG_MIDI_TRANSMIT\tNOTE_OFF: %d", blob_ptr->UID);
#endif
            usbMIDI.send_now();
            //while (usbMIDI.read()); // Read and discard any incoming MIDI messages
          };
        };
        //usbMIDI.send_now();
        while (usbMIDI.read()); // Read and discard any incoming MIDI messages
      };
#endif
      break;
    case BLOBS_LEARN:
      // Send separate blobs values using Control Change MIDI format
      // Send only the last blob that have been added to the sensor surface
      // Select blob's values according to the encoder position to allow the auto-mapping into Max4Live...
#if MIDI_TRANSMIT
      if ((blob_t*)llist_blobs.tail_ptr != NULL) {
        blob_t* blob_ptr = (blob_t*)llist_blobs.tail_ptr;
        switch (presets[BLOBS_LEARN].val) {
          case BS:
            if (blob_ptr->state) {
              if (!blob_ptr->lastState) {
                //usbMIDI.sendNoteOn(blob_ptr->UID + 1, 1, 0);
                usbMIDI.sendNoteOn(blob_ptr->UID + 1, 1, BS); // sendNoteOn(note, velocity, channel);
              };
            }
            else {
              //usbMIDI.sendNoteOff(blob_ptr->UID + 1, 0, 0);
              usbMIDI.sendNoteOn(blob_ptr->UID + 1, 0, BS); // sendNoteOn(note, velocity, channel);
            };
            break;
          case BX:
            usbMIDI.sendControlChange(blob_ptr->UID + 1, (uint8_t)round(map(blob_ptr->centroid.X, 0.0, X_MAX - X_MIN , 0, 127)), BX);
            break;
          case BY:
            usbMIDI.sendControlChange(blob_ptr->UID + 1, (uint8_t)round(map(blob_ptr->centroid.Y, 0.0, X_MAX - X_MIN, 0, 127)), BY);
            break;
          case BZ:
            usbMIDI.sendControlChange(blob_ptr->UID + 1, constrain(blob_ptr->centroid.Z, 0, 127), BZ);
            break;
          case BW:
            usbMIDI.sendControlChange(blob_ptr->UID + 1, blob_ptr->box.W, BW);
            break;
          case BH:
            usbMIDI.sendControlChange(blob_ptr->UID + 1, blob_ptr->box.H, BH);
            break;
          default:
            break;
        };
      };
#endif
      break;
    case BLOBS_MAPPING:
      for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiOut); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
        switch (node_ptr->midiMsg.status) {
          case midi::NoteOn:
#if DEBUG_MIDI_TRANSMIT
            Serial.printf("\nTRANSMIT\tNOTE_ON:%d\tVALUE:%d\tCHANNEL:%d", node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);
#endif
#if MIDI_TRANSMIT
            usbMIDI.sendNoteOn(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);  // USB send MIDI noteOn
#endif
#if MIDI_HARDWARE
            MIDI.sendNoteOn(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);     // Hardware send MIDI noteOn
#endif
            break;
          case midi::NoteOff:
#if DEBUG_MIDI_TRANSMIT
            Serial.printf("\nTRANSMIT\tNOTE_OFF:%d\tVALUE:%d\tCHANNEL:%d", node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);
#endif
#if MIDI_TRANSMIT
            usbMIDI.sendNoteOff(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);  // USB send MIDI noteOff
#endif
#if MIDI_HARDWARE
            MIDI.sendNoteOff(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);     // Hardware send MIDI noteOff
#endif
            break;
          case midi::ControlChange:
#if DEBUG_MIDI_TRANSMIT
            Serial.printf("\nTRANSMIT\tCC:%d\tVALUE:%d\tCHANNEL:%d", node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);
#endif
#if MIDI_TRANSMIT
            usbMIDI.sendControlChange(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL); // USB send MIDI control_change
#endif
#if MIDI_HARDWARE
            MIDI.sendControlChange(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);    // Hardware send MIDI control_change
#endif
            break;
          default:
            break;
        };
      };
      llist_save_nodes(&midi_node_stack, &midiOut); // Save/rescure all midiOut nodes
      break;
    default:
      break;
  };
};
