/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
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
size_t sysEx_data_length = 0;
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
  if (e256_current_mode == PENDING_MODE && millis() - bootTime > PENDING_MODE_TIMEOUT){
    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
      Serial.printf("\nPENDING_MODE_TIME_OUT");
    #endif
    if(load_flash_config()){
      #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
        Serial.printf("\nFLASH_CONFIG_LOAD_DONE: ");
        print_bytes(flash_config_ptr, flash_config_size);
      #endif
      if (apply_config(flash_config_ptr, flash_config_size)) {
        #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
          Serial.printf("\nAPPLY_MODE_DONE");
        #endif
      }
      else {
        #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
          Serial.printf("\nCONFIG_APPLY_FAILED");
        #endif
        //set_mode(ERROR_MODE);
      }
      matrix_calibrate();
      blink(10);
      set_mode(STANDALONE_MODE);
    }
    else {
      bootTime = millis();
      set_mode(PENDING_MODE);
      #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
        Serial.printf("\nNO_CONFIG_FILE_LOADED");
      #endif
    };
  };
};

void usb_midi_transmit() {
  static uint32_t usbTransmitTimeStamp = 0;
  uint8_t blob_values[8] = {0};
  switch (e256_current_mode) {
    case PENDING_MODE:
      // Nothing to do
      break;
    case SYNC_MODE:
      // Nothing to do
      break;
    case MATRIX_MODE_RAW:
      if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
        usbTransmitTimeStamp = millis();
        usbMIDI.sendSysEx(RAW_FRAME, raw_frame.data_ptr, false);
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
        usbMIDI.sendSysEx(NEW_FRAME, interp_frame.data_ptr, false, 0);
        usbMIDI.send_now();
      };
      while (usbMIDI.read()); // Read and discard any incoming MIDI messages
      break;
    */
    case EDIT_MODE:
      // Send all blobs values over USB using MIDI format
      for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_blobs); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
        blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(node_ptr);
        if (blob_ptr->status == NEW) {
          usbMIDI.sendNoteOn(blob_ptr->UID, 1, BS); // sendNoteOn(note, velocity, channel);
          usbMIDI.send_now();
        }
        else if (blob_ptr->status == PRESENT) {
          //if (millis() - blob_ptr->transmit_time_stamp > MIDI_TRANSMIT_INTERVAL) {
          //blob_ptr->transmit_time_stamp = millis();
          blob_values[0] = blob_ptr->UID;

          uint8_t whole_part = (uint8_t)blob_ptr->centroid.x;
          blob_values[1] = whole_part;
          blob_values[2] = (uint8_t)((blob_ptr->centroid.x - whole_part) * 100); // Fractional part

          whole_part = (uint8_t)blob_ptr->centroid.y;
          blob_values[3] = whole_part;
          blob_values[4] = (uint8_t)((blob_ptr->centroid.y - whole_part) * 100); // Fractional part

          blob_values[5] = blob_ptr->box.w;
          blob_values[6] = blob_ptr->box.h;
          blob_values[7] = blob_ptr->centroid.z;
          
          usbMIDI.sendSysEx(8, blob_values, false);
          //};
        }
        else if (blob_ptr->status == MISSING && blob_ptr->last_status == PRESENT) { ///////////////////////// TO_TEST!
          usbMIDI.sendNoteOff(blob_ptr->UID, 0, BS); // sendNoteOff(note, velocity, channel);
          usbMIDI.send_now();
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
  while (usbMIDI.read());
};

void usb_read_noteOn(uint8_t channel, uint8_t note, uint8_t velocity){
  midi_node_t* node_ptr = (midi_node_t*)llist_pop_front(&midi_nodes_pool);
  node_ptr->midi.type = midi::NoteOn;
  node_ptr->midi.data1 = note;
  node_ptr->midi.data2 = velocity;
  node_ptr->midi.channel = channel;
  switch (e256_current_mode) {
    case EDIT_MODE:
      llist_push_front(&midi_in, node_ptr);  // Add the node to the midi_in linked list
      break;
    case PLAY_MODE:
      llist_push_front(&midi_out, node_ptr); // Add the node to the midi_out linked list
      break;
    default:
      break;
  }
};

void usb_read_noteOff(uint8_t channel, uint8_t note, uint8_t velocity){
  midi_node_t* node_ptr = (midi_node_t*)llist_pop_front(&midi_nodes_pool);
  node_ptr->midi.type = midi::NoteOff;
  node_ptr->midi.data1 = note;
  node_ptr->midi.data2 = velocity;
  node_ptr->midi.channel = channel;
  switch (e256_current_mode) {
    case EDIT_MODE:
      llist_push_front(&midi_in, node_ptr);  // Add the node to the midi_in linked list
      break;
    case PLAY_MODE:
      llist_push_front(&midi_out, node_ptr); // Add the node to the midi_out linked list
      break;
    default:
      break;
  }
};

// Used by USB_MIDI
// If the CC comes from usb it is forwarded to midi_out acting as MIDI thru
void usb_read_controlChange(uint8_t channel, uint8_t control, uint8_t value){
  switch (channel){
    case MIDI_LEVELS_CHANNEL:
      set_level((level_codes_t)control, value);
      break;
    default:
      midi_node_t* node_ptr = (midi_node_t*)llist_pop_front(&midi_nodes_pool);  // Get a node from the MIDI nodes stack
      node_ptr->midi.type = midi::ControlChange; // Set the MIDI type
      node_ptr->midi.data1 = control;            // Set the MIDI note
      node_ptr->midi.data2 = value;              // Set the MIDI velocity
      node_ptr->midi.channel = channel;          // Set the MIDI channel
      switch (e256_current_mode) {
        case EDIT_MODE:
          llist_push_front(&midi_in, node_ptr);  // Add the node to the midi_in linked list
          break;
        case PLAY_MODE:
          llist_push_front(&midi_out, node_ptr); // Add the node to the midi_out linked list
          break;
        default:
          break;
      }
      break;
  }
};

void usb_read_programChange(uint8_t channel, uint8_t program){
  
  switch (channel){
    case MIDI_MODES_CHANNEL:
      //Serial.printf("\nRECEIVED:\t%s", get_mode_name((enum_mode_codes_t)program));
      switch (program){

        case PENDING_MODE:
          set_mode(PENDING_MODE);
          usb_midi_send_info((uint8_t)PENDING_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case SYNC_MODE:
          set_mode(SYNC_MODE);
          usb_midi_send_info((uint8_t)SYNC_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case MATRIX_MODE_RAW:
          set_mode(MATRIX_MODE_RAW);
          usb_midi_send_info((uint8_t)MATRIX_MODE_RAW_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case MAPPING_MODE:
          set_mode(MAPPING_MODE);
          usb_midi_send_info((uint8_t)MAPPING_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case EDIT_MODE:
          set_mode(EDIT_MODE);
          usb_midi_send_info((uint8_t)EDIT_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case PLAY_MODE:
          set_mode(PLAY_MODE);
          usb_midi_send_info((uint8_t)PLAY_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case LOAD_MODE:
          if(load_flash_config()){
            #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
              Serial.printf("\nSEND_FLASH_CONFIG: ");
              print_bytes(flash_config_ptr, flash_config_size);
            #endif
            usb_midi_send_info((uint8_t)LOAD_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          }
          else {
            usb_midi_send_info((uint8_t)NO_CONFIG_FILE, MIDI_ERROR_CHANNEL);
          };
          break;

        case FETCH_MODE:
          usbMIDI.sendSysEx(flash_config_size, flash_config_ptr, false);
          //usbMIDI.send_now();
          usb_midi_send_info((uint8_t)FETCH_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
        
        case ALLOCATE_MODE:
          e256_current_mode = ALLOCATE_MODE;
          usb_midi_send_info((uint8_t)ALLOCATE_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;
        
        case UPLOAD_MODE:
          e256_current_mode = UPLOAD_MODE;
          usb_midi_send_info((uint8_t)UPLOAD_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case APPLY_MODE:
          if (apply_config(sysEx_data_ptr, sysEx_data_length)) {
            usb_midi_send_info((uint8_t)APPLY_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
            #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
              Serial.printf("\nnCONFIG_APPLY_DONE");
            #endif
          }
          else {
            usb_midi_send_info((uint8_t)CONFIG_APPLY_FAILED, MIDI_ERROR_CHANNEL);
            #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
              Serial.printf("\nCONFIG_APPLY_FAILED");
            #endif
            //set_mode(ERROR_MODE);
          }
          break;

        case CALIBRATE_MODE:
          matrix_calibrate();
          blink(10);
          usb_midi_send_info((uint8_t)CALIBRATE_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case ERROR_MODE:
          set_mode(ERROR_MODE);
          //usb_midi_send_info((uint8_t)ERROR_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        default:
          //Serial.println("OUT_OFF_RANGE!"); // DROP!
          break;
      };
      break;
  };
};

// TODO
void usb_read_midi_clock(){
#if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
  Serial.println("Clock");
#endif
};

// Send data via MIDI system exclusive message
// As MIDI buffer is limited to (USB_MIDI_SYSEX_MAX) we must register the data in chunks!
// Recive: [ SYSEX_BEGIN, SYSEX_DEVICE_ID, SYSEX_SIZE_MSB, SYSEX_SIZE_LSB, SYSEX_END ] 
// Send: USBMIDI_CONFIG_ALLOC_DONE
// Recive: [ SYSEX_BEGIN, SYSEX_DEVICE_ID, SYSEX_DATA, SYSEX_END ]
// Send: USBMIDI_CONFIG_LOAD_DONE

void usb_read_systemExclusive(const uint8_t *data_ptr, uint16_t sysEx_chunk_size, bool complete) {
  static uint8_t *sysEx_chunk_ptr = NULL;
  static uint16_t sysEx_last_chunk_size = 0;
  static uint8_t sysEx_chunks = 0;
  static uint8_t sysEx_chunk_count = 0;

  switch (e256_current_mode) {

  case ALLOCATE_MODE:
    uint8_t sysEx_data_length_MSB;
    uint8_t sysEx_data_length_LSB;

    sysEx_data_length_MSB = *(data_ptr + 2);
    sysEx_data_length_LSB = *(data_ptr + 3);
    sysEx_data_length = sysEx_data_length_MSB << 7 | sysEx_data_length_LSB;

    sysEx_chunk_ptr = sysEx_data_ptr = (uint8_t *)allocate(sysEx_data_ptr, sysEx_data_length + 1);
    sysEx_chunks = (uint8_t)(sysEx_data_length + 3 / USB_MIDI_SYSEX_MAX); // +3 adding heder & footer size
    sysEx_last_chunk_size = sysEx_data_length + 3 % USB_MIDI_SYSEX_MAX; // +3 adding heder & footer size
    
    if (sysEx_last_chunk_size != 0) sysEx_chunks++;

    usb_midi_send_info((uint8_t)ALLOCATE_DONE, MIDI_VERBOSITY_CHANNEL);
    break;

  case UPLOAD_MODE:
    if (sysEx_chunks == 1 && complete) { // Only one chunk to load
      memcpy(sysEx_chunk_ptr, data_ptr + 2, sizeof(uint8_t) * sysEx_chunk_size - 3); // -3 removing heder & footer size
      usb_midi_send_info((uint8_t)UPLOAD_DONE, MIDI_VERBOSITY_CHANNEL);
    }
    else if (sysEx_chunks > 1 && sysEx_chunk_count == 0) { // First chunk
      memcpy(sysEx_chunk_ptr, data_ptr + 2, sizeof(uint8_t) * sysEx_chunk_size - 2); // -2 removing heder size
      sysEx_chunk_ptr += (sysEx_chunk_size - 2);
      sysEx_chunk_count++;
    }
    else if (!complete) { // Middle chunks
      memcpy(sysEx_chunk_ptr, data_ptr, sizeof(uint8_t) * sysEx_chunk_size);
      sysEx_chunk_ptr += sysEx_chunk_size;
      sysEx_chunk_count++;
    }
    else { // Last chunk
      memcpy(sysEx_chunk_ptr, data_ptr, sizeof(uint8_t) * sysEx_chunk_size - 1); // -1 removing footer size
      sysEx_chunk_count = 0;
      usb_midi_send_info((uint8_t)UPLOAD_DONE, MIDI_VERBOSITY_CHANNEL);
    };
    break;

    default:
      usb_midi_send_info((uint8_t)UNKNOWN_SYSEX, MIDI_ERROR_CHANNEL);
      set_mode(ERROR_MODE);
      break;
  };
};
