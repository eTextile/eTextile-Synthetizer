/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "usb_midi_io.h"

#include "config.h"
#include "scan.h"
#include "interp.h"
#include "llist.h"
#include "blob.h"
//#include "midi_bus.h"
#include "allocate.h"

uint32_t bootTime = 0;
uint16_t sysEx_data_length = 0;
uint8_t* sysEx_data_ptr = NULL;

// Used by USB_MIDI && HARDWARE_MIDI
void e256_noteOn(byte channel, byte note, byte velocity){
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);
  node_ptr->midiMsg.channel = channel;
  node_ptr->midiMsg.status = midi::NoteOn;
  node_ptr->midiMsg.data1 = note;
  node_ptr->midiMsg.data2 = velocity;
  llist_push_front(&midiIn, node_ptr);
};

// Used by USB_MIDI & HARDWARE_MIDI
void e256_noteOff(byte channel, byte note, byte velocity){
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);
  node_ptr->midiMsg.channel = channel;
  node_ptr->midiMsg.status = midi::NoteOff;
  node_ptr->midiMsg.data1 = note;
  node_ptr->midiMsg.data2 = velocity;
  llist_push_front(&midiIn, node_ptr);
};

// Used by USB_MIDI && HARDWARE_MIDI
void e256_controlChange(byte channel, byte number, byte value){
    switch (channel){
      case MIDI_LEVELS_CHANNEL:
        set_level(number, value);
        break;
      default:
        // NA
        break;
    }
};

// Used by USB_MIDI && HARDWARE_MIDI
void e256_programChange(byte channel, byte program){
  switch (channel){
    case MIDI_MODES_CHANNEL:
      switch (program){
        case PENDING_MODE:
          //set_mode(PENDING_MODE);
          midiInfo(PENDING_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
        case SYNC_MODE:
          set_mode(SYNC_MODE);
          midiInfo(SYNC_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
        case MATRIX_MODE_RAW:
          set_mode(MATRIX_MODE_RAW);
          midiInfo(MATRIX_MODE_RAW_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
        case MATRIX_MODE_INTERP:
          set_mode(MATRIX_MODE_INTERP);
          midiInfo(MATRIX_MODE_INTERP_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
        case EDIT_MODE:
          set_mode(EDIT_MODE);
          midiInfo(EDIT_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
        case PLAY_MODE:
          set_mode(PLAY_MODE);
          midiInfo(PLAY_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
      };
      break;
    case MIDI_STATES_CHANNEL:
      switch (program){
        case CALIBRATE_REQUEST:
          matrix_calibrate();
          set_state(CALIBRATE_REQUEST);
          midiInfo(CALIBRATE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
        case CONFIG_FILE_REQUEST:
          if(load_flash_config()){
            midiInfo(FLASH_CONFIG_LOAD_DONE, MIDI_VERBOSITY_CHANNEL);
            usbMIDI.sendSysEx(flash_configSize, flash_config_ptr, false);
            usbMIDI.send_now();
            while (usbMIDI.read());
            #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
              Serial.printf("\nSEND_FLASH_CONFIG: ");
              printBytes(flash_config_ptr, flash_configSize);
            #endif
          } else {
            midiInfo(FLASH_CONFIG_LOAD_FAILED, MIDI_ERROR_CHANNEL);
          };
          break;
      };
      break;
  };
  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MIDI_IO)
    Serial.printf("\nRECIVE_PGM_IN:%d\tCHANNEL:%d", program, channel);
  #endif
};

// Move to midi_management.cpp
void usb_midi_pending_mode_timeout(){
  if (e256_currentMode == PENDING_MODE && millis() - bootTime > PENDING_MODE_TIMEOUT){
    if(load_flash_config()){
      set_mode(STANDALONE_MODE);
      matrix_calibrate();
      #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
        Serial.printf("\nFLASH_CONFIG_LOAD_DONE: ");
        printBytes(flash_config_ptr, flash_configSize);
      #endif
    } else {
      set_mode(SYNC_MODE);
      #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
        Serial.printf("\nFLASH_CONFIG_LOAD_FAILED!");
      #endif
      //set_mode(ERROR_MODE);
    };
  };
};

void midiInfo(uint8_t msg, uint8_t channel){
if (e256_currentMode == STANDALONE_MODE){
  usbMIDI.sendProgramChange(msg, channel); // ProgramChange(program, channel);
  usbMIDI.send_now();
  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
    Serial.printf("\nSEND_MIDI_MSG:%d\tCHANNEL:%d", msg, channel);
  #endif
  }
};

// Send data via MIDI system exclusive message
// As MIDI buffer is limited to (USB_MIDI_SYSEX_MAX) we must register the data in chunks!
// Recive: [ SYSEX_BEGIN, SYSEX_DEVICE_ID, SYSEX_IDENTIFIER, SYSEX_SIZE_MSB, SYSEX_SIZE_LSB, SYSEX_END ] 
// Send: USBMIDI_CONFIG_ALLOC_DONE
// Recive: [ SYSEX_BEGIN, SYSEX_DEVICE_ID, SYSEX_DATA, SYSEX_END ]
// Send: USBMIDI_CONFIG_LOAD_DONE

void e256_systemExclusive(const uint8_t* data_ptr, uint16_t length, bool complete){
  static bool sysEx_alloc = true;
  static uint8_t* sysEx_chunk_ptr = NULL;
  static uint16_t sysEx_lastChunkSize = 0;
  static uint8_t sysEx_chunks = 0;
  static uint8_t sysEx_chunkCount = 0;
  static uint8_t sysEx_identifier = 0;
  static uint16_t sysEx_chunkSize = 0;
  
  if (sysEx_alloc){
    sysEx_alloc = false;
    sysEx_identifier = *(data_ptr + 2);
    sysEx_data_length = *(data_ptr + 3) << 7 | *(data_ptr + 4);
    sysEx_chunk_ptr = sysEx_data_ptr = (uint8_t*)allocate(sysEx_data_ptr, sysEx_data_length );
    sysEx_chunks = (uint8_t)((sysEx_data_length + 3) / USB_MIDI_SYSEX_MAX);
    sysEx_lastChunkSize = (sysEx_data_length + 3) % USB_MIDI_SYSEX_MAX;
    if (sysEx_lastChunkSize != 0) sysEx_chunks++;
    sysEx_chunkCount = 0;
    midiInfo(USBMIDI_CONFIG_ALLOC_DONE, MIDI_VERBOSITY_CHANNEL);
  }
  else {
    if (sysEx_chunks == 1) { // Only one chunk to load
      memcpy(sysEx_chunk_ptr, data_ptr + 2, sizeof(uint8_t) * sysEx_data_length);
    }
    else if (sysEx_chunks > 1 && sysEx_chunkCount == 0) { // First chunk
      sysEx_chunkSize = USB_MIDI_SYSEX_MAX - 2; // Removing header size
      memcpy(sysEx_chunk_ptr, data_ptr + 2, sizeof(uint8_t) * sysEx_chunkSize);
      sysEx_chunk_ptr += sysEx_chunkSize;
      sysEx_chunkCount++;
    }
    else if (sysEx_chunkCount < sysEx_chunks - 1){ // Middle chunks
      sysEx_chunkSize = USB_MIDI_SYSEX_MAX;
      memcpy(sysEx_chunk_ptr, data_ptr, sizeof(uint8_t) * sysEx_chunkSize);
      sysEx_chunk_ptr += sysEx_chunkSize;
      sysEx_chunkCount++;
    }
    else { // Last chunk
      sysEx_chunkSize = sysEx_lastChunkSize - 1; // Removing footer size
      memcpy(sysEx_chunk_ptr, data_ptr, sizeof(uint8_t) * sysEx_chunkSize);
      sysEx_alloc = true;
      if (sysEx_identifier == SYSEX_CONF) {
        midiInfo(USBMIDI_CONFIG_LOAD_DONE, MIDI_VERBOSITY_CHANNEL);
        if (apply_config(sysEx_data_ptr, sysEx_data_length)){
          #if defined(USB_MIDI_SERIAL) & defined(DEBUG_CONFIG)
            Serial.printf("\nSYSEX_CONFIG_RECIVED: ");
            printBytes(sysEx_data_ptr, sysEx_data_length);
          #else
            midiInfo(CONFIG_APPLY_DONE, MIDI_VERBOSITY_CHANNEL);
          #endif
        }
        else {
          midiInfo(CONFIG_APPLY_FAILED, MIDI_ERROR_CHANNEL);
        }
      }
      else if (sysEx_identifier == SYSEX_SOUND){ // TODO
        midiInfo(USBMIDI_SOUND_LOAD_DONE, MIDI_VERBOSITY_CHANNEL);
      }
      else {
        midiInfo(UNKNOWN_SYSEX, MIDI_ERROR_CHANNEL);
      };
    };
  };
};

// TODO
void e256_clock(){
#if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
  Serial.println("Clock");
#endif
};

// Setup the USB_MIDI communication port
void usb_midi_io_setup() {
  usbMIDI.begin();
  usbMIDI.setHandleProgramChange(e256_programChange);
  usbMIDI.setHandleNoteOn(e256_noteOn);
  usbMIDI.setHandleNoteOff(e256_noteOff);
  usbMIDI.setHandleControlChange(e256_controlChange);
  usbMIDI.setHandleSystemExclusive(e256_systemExclusive);
  usbMIDI.setHandleClock(e256_clock);
};

void usb_midi_recive(void) {
  usbMIDI.read(); // Is there a MIDI incoming messages on any channel
};

// NOT USED!
void usb_midi_handle_input(const midi::Message<128u> &midiMsg){
midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  // This can be refactored via C++ !?
  //
  switch (midiMsg.type) {
    case midi::NoteOn:
      node_ptr->midiMsg.status = midi::NoteOn;         // Set the MIDI status
      node_ptr->midiMsg.channel = midiMsg.channel;     // Set the MIDI channel
      node_ptr->midiMsg.data1 = midiMsg.data1;         // Set the MIDI note
      node_ptr->midiMsg.data2 = midiMsg.data2;         // Set the MIDI velocity
      #if defined(MIDI_THRU) 
        llist_push_front(&midiOut, node_ptr);          // Add the node to the midiOut linked liste
      #else
        llist_push_front(&midiIn, node_ptr);           // Add the node to the midiIn linked liste
      #endif
      break;
    case midi::NoteOff:
      node_ptr->midiMsg.status = midi::NoteOff;        // Set the MIDI status
      node_ptr->midiMsg.channel = midiMsg.channel;     // Set the MIDI channel
      node_ptr->midiMsg.data1 = midiMsg.data1;         // Set the MIDI note
      node_ptr->midiMsg.data2 = midiMsg.data2;         // Set the MIDI velocity
      #if defined(MIDI_THRU) 
        llist_push_front(&midiOut, node_ptr);          // Add the node to the midiOut linked liste
      #else
        llist_push_front(&midiIn, node_ptr);           // Add the node to the midiIn linked liste
      #endif
      break;
    case midi::ControlChange:
      node_ptr->midiMsg.status = midi::ControlChange;  // Set the MIDI status
      node_ptr->midiMsg.channel = midiMsg.channel;     // Set the MIDI channel
      node_ptr->midiMsg.data1 = midiMsg.data1;         // Set the MIDI control
      node_ptr->midiMsg.data2 = midiMsg.data2;         // Set the MIDI value
      #if defined(MIDI_THRU) 
        llist_push_front(&midiOut, node_ptr);          // Add the node to the midiOut linked liste
      #else
        llist_push_front(&midiIn, node_ptr);           // Add the node to the midiIn linked liste
      #endif
      break;
    case midi::Clock:
      node_ptr->midiMsg.status = midi::Clock;          // Set the MIDI status
      //node_ptr->midiMsg.channel = midiMsg.channel;   // Set the MIDI channel
      //node_ptr->midiMsg.data1 = midiMsg.data1;       // Set the MIDI note
      //node_ptr->midiMsg.data2 = midiMsg.data2;       // Set the MIDI velocity
      #if defined(MIDI_THRU) 
        llist_push_front(&midiOut, node_ptr);          // Add the node to the midiOut linked liste
      #else
        llist_push_front(&midiIn, node_ptr);           // Add the node to the midiIn linked liste
      #endif
      break;
    default:
      llist_push_front(&midi_node_stack, node_ptr);    // Add the node to the midi_node_stack linked liste
      break;
  };
};

void usb_midi_transmit() {
  static uint32_t usbTransmitTimeStamp = 0;
  uint8_t blobValues[6] = {0};
  switch (e256_currentMode) {
    case PENDING_MODE:
      // Nothing to do
    break;
    case SYNC_MODE:
      // Nothing to do
    break;
    case MATRIX_MODE_RAW:
      if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
        usbTransmitTimeStamp = millis();
        usbMIDI.sendSysEx(RAW_FRAME, rawFrame.pData, false);
        usbMIDI.send_now();
      };
      while (usbMIDI.read()); // Read and discard any incoming MIDI messages
      break;
    case MATRIX_MODE_INTERP:
      if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
        usbTransmitTimeStamp = millis();
        usbMIDI.sendSysEx(NEW_FRAME, interpFrame.pData, false);
        usbMIDI.send_now();
      };
      while (usbMIDI.read()); // Read and discard any incoming MIDI messages
      break;
    case EDIT_MODE:
      // Send all blobs values over USB using MIDI format
      for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
        if (blob_ptr->state) {
          if (!blob_ptr->lastState) {
            usbMIDI.sendNoteOn(blob_ptr->UID + 1, 1, BS); // sendNoteOn(note, velocity, channel);
            usbMIDI.send_now();
          } else {
            //if (millis() - blob_ptr->transmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
            //blob_ptr->transmitTimeStamp = millis();
            
            //if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
            //usbTransmitTimeStamp = millis();
            
            // This must be zerocopy!

            blobValues[0] = blob_ptr->UID + 1;
            blobValues[1] = (uint8_t)round(map(blob_ptr->centroid.x, 0, WIDTH, 0, 127));
            blobValues[2] = (uint8_t)round(map(blob_ptr->centroid.y, 0, HEIGHT, 0, 127));
            blobValues[3] = blob_ptr->centroid.z;
            blobValues[4] = blob_ptr->box.W;
            blobValues[5] = blob_ptr->box.H;

            usbMIDI.sendSysEx(6, blobValues, false); // Testing!
            
            //usbMIDI.sendSysEx(6, blob_ptr.pData, false); // TODO !?
            //};
          };
        } else {
          if (blob_ptr->lastState && blob_ptr->status != NOT_FOUND) {
            usbMIDI.sendNoteOff(blob_ptr->UID + 1, 0, BS); // sendNoteOff(note, velocity, channel);
            usbMIDI.send_now();
          };
        };
      };
      while (usbMIDI.read()); // Read and discard any incoming MIDI messages
      break;
    case PLAY_MODE:
    // The PLAY_MODE will play MIDI depending on the mapping you made using the web app
    // Using MIDI_THRU : USB_MIDI_IN-> HARDWARE_MIDI_OUT
      for (midiNode_t *node_ptr = (midiNode_t *)ITERATOR_START_FROM_HEAD(&midiOut); node_ptr != NULL; node_ptr = (midiNode_t *)ITERATOR_NEXT(node_ptr)){
        switch (node_ptr->midiMsg.status) {
          case midi::NoteOn:
            usbMIDI.sendNoteOn(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel);
            break;
          case midi::NoteOff:
            usbMIDI.sendNoteOff(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel);
            break;
          case midi::ControlChange:
            if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
              usbTransmitTimeStamp = millis();
              usbMIDI.sendControlChange(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel);
            };
            break;
          default:
            break;
        };
      };
      break;
    default:
      break;
  };
};
