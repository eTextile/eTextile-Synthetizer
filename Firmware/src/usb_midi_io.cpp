/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "usb_midi_io.h"

#include "config.h"
#include "scan.h"
#include "interp.h"
#include "llist.h"
#include "blob.h"
#include "midi_bus.h"
#include "allocate.h"

uint32_t bootTime = 0;
uint16_t sysEx_data_length = 0;
uint8_t* sysEx_data_ptr = NULL;

// Setup the USB_MIDI communication port
void usb_midi_setup() {
  usbMIDI.begin();
  usbMIDI.setHandleProgramChange(usb_read_programChange);
  usbMIDI.setHandleNoteOn(usb_read_noteOn);
  usbMIDI.setHandleNoteOff(usb_read_noteOff);
  usbMIDI.setHandleControlChange(usb_read_controlChange);
  usbMIDI.setHandleSystemExclusive(usb_read_systemExclusive);
  usbMIDI.setHandleClock(usb_read_midi_clock);
};

void usb_midi_recive(void) {
  usbMIDI.read(); // Is there a MIDI incoming messages on any channel
};

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
    /*
    case MATRIX_MODE_INTERP:
      // NOT_WORKING > see https://forum.pjrc.com/threads/28282-How-big-is-the-MIDI-receive-buffer
      // Interpolation will be made on the web_app side!
      if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
        usbTransmitTimeStamp = millis();
        usbMIDI.sendSysEx(NEW_FRAME, interpFrame.pData, false, 0);
        usbMIDI.send_now();
      };
      while (usbMIDI.read()); // Read and discard any incoming MIDI messages
      break;
    */
    case EDIT_MODE:
      // Send all blobs values over USB using MIDI format
      for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
        if (blob_ptr->state) {
          if (!blob_ptr->lastState) {
            usbMIDI.sendNoteOn(blob_ptr->UID + 1, 1, BS); // sendNoteOn(note, velocity, channel);
            usbMIDI.send_now();
          }
          else {
            //if (millis() - blob_ptr->transmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
              //blob_ptr->transmitTimeStamp = millis();
              // This must be zerocopy!
              blobValues[0] = blob_ptr->UID + 1;
              blobValues[1] = (uint8_t)round(map(blob_ptr->centroid.x, 0, WIDTH, 0, 127));
              blobValues[2] = (uint8_t)round(map(blob_ptr->centroid.y, 0, HEIGHT, 0, 127));
              blobValues[3] = blob_ptr->centroid.z;
              blobValues[4] = blob_ptr->box.w;
              blobValues[5] = blob_ptr->box.h;
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
        // NA 
    break;
  default:
    break;
  };
};

void usb_midi_send_info(uint8_t msg, uint8_t channel){
  usbMIDI.sendProgramChange(msg, channel); // ProgramChange(program, channel);
  usbMIDI.send_now();
  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
    Serial.printf("\nSEND_MIDI_MSG:%d\tCHANNEL:%d", msg, channel);
  #endif
};

void usb_read_noteOn(byte channel, byte note, byte velocity){
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);
  node_ptr->midi.type = midi::NoteOn;
  node_ptr->midi.data1 = note;
  node_ptr->midi.data2 = velocity;
  node_ptr->midi.channel = channel;
  switch (e256_currentMode) {
  case EDIT_MODE:
      llist_push_front(&midiIn, node_ptr);  // Add the node to the midiIn linked liste
    break;
    case PLAY_MODE:
      llist_push_front(&midiOut, node_ptr); // Add the node to the midiOut linked liste
    break;
  default:
    break;
  }
};

void usb_read_noteOff(byte channel, byte note, byte velocity){
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);
  node_ptr->midi.type = midi::NoteOff;
  node_ptr->midi.data1 = note;
  node_ptr->midi.data2 = velocity;
  node_ptr->midi.channel = channel;
  switch (e256_currentMode) {
  case EDIT_MODE:
      llist_push_front(&midiIn, node_ptr);  // Add the node to the midiIn linked liste
    break;
    case PLAY_MODE:
      llist_push_front(&midiOut, node_ptr); // Add the node to the midiOut linked liste
    break;
  default:
    break;
  }
};

// Used by USB_MIDI
// If the CC comes from usb it is forwarded to midiOut acting as MIDI thru
void usb_read_controlChange(byte channel, byte control, byte value){
  switch (channel){
      case MIDI_LEVELS_CHANNEL:
        set_level(control, value);
        break;
      default:
        midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
        node_ptr->midi.type = midi::ControlChange;  // Set the MIDI status
        node_ptr->midi.data1 = control;               // Set the MIDI note
        node_ptr->midi.data2 = value;                 // Set the MIDI velocity
        node_ptr->midi.channel = channel;             // Set the MIDI channel
        switch (e256_currentMode) {
          case EDIT_MODE:
            llist_push_front(&midiIn, node_ptr);  // Add the node to the midiIn linked liste
            break;
          case PLAY_MODE:
            llist_push_front(&midiOut, node_ptr); // Add the node to the midiOut linked liste
            break;
          default:
            break;
        }
      break;
    }
};

void usb_read_programChange(byte channel, byte program){
  switch (channel){
    case MIDI_MODES_CHANNEL:
      switch (program){
        case PENDING_MODE:
          //set_mode(PENDING_MODE);
          usb_midi_send_info(PENDING_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
        case SYNC_MODE:
          set_mode(SYNC_MODE);
          usb_midi_send_info(SYNC_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
        case MATRIX_MODE_RAW:
          set_mode(MATRIX_MODE_RAW);
          usb_midi_send_info(MATRIX_MODE_RAW_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
        case EDIT_MODE:
          set_mode(EDIT_MODE);
          usb_midi_send_info(EDIT_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
        case PLAY_MODE:
          set_mode(PLAY_MODE);
          usb_midi_send_info(PLAY_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
      };
      break;
    case MIDI_STATES_CHANNEL:
      switch (program){
        case CALIBRATE_REQUEST:
          matrix_calibrate();
          set_state(CALIBRATE_REQUEST);
          usb_midi_send_info(CALIBRATE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
        case CONFIG_FILE_REQUEST:
          if(load_flash_config()){
            usb_midi_send_info(FLASH_CONFIG_LOAD_DONE, MIDI_VERBOSITY_CHANNEL);
            usbMIDI.sendSysEx(flash_configSize, flash_config_ptr, false);
            usbMIDI.send_now();
            while (usbMIDI.read());
            #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
              Serial.printf("\nSEND_FLASH_CONFIG: ");
              printBytes(flash_config_ptr, flash_configSize);
            #endif
          } else {
            usb_midi_send_info(FLASH_CONFIG_LOAD_FAILED, MIDI_ERROR_CHANNEL);
          };
          break;
      };
      break;
  };
  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MIDI_IO)
    Serial.printf("\nRECIVE_PGM_IN:%d\tCHANNEL:%d", program, channel);
  #endif
};

// TODO
void usb_read_midi_clock(){
#if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
  Serial.println("Clock");
#endif
};

// Send data via MIDI system exclusive message
// As MIDI buffer is limited to (USB_MIDI_SYSEX_MAX) we must register the data in chunks!
// Recive: [ SYSEX_BEGIN, SYSEX_DEVICE_ID, SYSEX_IDENTIFIER, SYSEX_SIZE_MSB, SYSEX_SIZE_LSB, SYSEX_END ] 
// Send: USBMIDI_CONFIG_ALLOC_DONE
// Recive: [ SYSEX_BEGIN, SYSEX_DEVICE_ID, SYSEX_DATA, SYSEX_END ]
// Send: USBMIDI_CONFIG_LOAD_DONE

void usb_read_systemExclusive(const uint8_t* data_ptr, uint16_t length, bool complete){
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
    usb_midi_send_info(USBMIDI_CONFIG_ALLOC_DONE, MIDI_VERBOSITY_CHANNEL);
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
        usb_midi_send_info(USBMIDI_CONFIG_LOAD_DONE, MIDI_VERBOSITY_CHANNEL);
        if (apply_config(sysEx_data_ptr, sysEx_data_length)){
          #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
            Serial.printf("\nSYSEX_CONFIG_RECIVED: ");
            printBytes(sysEx_data_ptr, sysEx_data_length);
          #else
            usb_midi_send_info(CONFIG_APPLY_DONE, MIDI_VERBOSITY_CHANNEL);
          #endif
        }
        else {
          usb_midi_send_info(CONFIG_APPLY_FAILED, MIDI_ERROR_CHANNEL);
        }
      }
      else if (sysEx_identifier == SYSEX_SOUND){ // TODO
        usb_midi_send_info(USBMIDI_SOUND_LOAD_DONE, MIDI_VERBOSITY_CHANNEL);
      }
      else {
        usb_midi_send_info(UNKNOWN_SYSEX, MIDI_ERROR_CHANNEL);
      };
    };
  };
};

