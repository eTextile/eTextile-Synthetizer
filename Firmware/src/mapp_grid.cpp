/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapp_grid.h"

typedef struct mapp_grid_s mapp_grid_t;
struct mapp_grid_s {
  common_t common;
  grid_t params;
};

static mapp_grid_t mapp_grids[MAX_GRIDS];

llist_t llist_grid_pool;

void mapping_grids_alloc(uint8_t grids_cnt) {
  llist_builder(&llist_grid_pool, &mapp_grids[0], grids_cnt, sizeof(mapp_grids[0]));
};

void mapping_grid_play(blob_t*);

// Test if the blob is within the grid limits
bool mapping_grid_interact(blob_t* blob_ptr, common_t* mapping_ptr) {
  mapp_grid_t* grid_ptr = (mapp_grid_t*)mapping_ptr;
  if (blob_ptr->centroid.x > grid_ptr->params.rect.from.x &&
      blob_ptr->centroid.x < grid_ptr->params.rect.to.x &&
      blob_ptr->centroid.y > grid_ptr->params.rect.from.y &&
      blob_ptr->centroid.y < grid_ptr->params.rect.to.y) {
    
    blob_ptr->action.mapping_ptr = grid_ptr;
    
    //blob_ptr->action.touch_ptr = &grid_ptr->params.keys[j];
    
    blob_ptr->action.func_ptr = &mapping_grid_play;

    return true;
  };
  return false;
};

// Compute the keyPresed position acording to the blobs XY (centroid) coordinates
// Add corresponding MIDI message to the MIDI out liked list
void mapping_grid_play(blob_t *blob_ptr) {
  mapp_grid_t* grid_ptr = (mapp_grid_t*)blob_ptr->action.mapping_ptr;
  switch_t* touch_ptr = (switch_t*)blob_ptr->action.touch_ptr;
  
  //cast (uint8_t)!?
  uint8_t keyPressX = (uint8_t)lround((blob_ptr->centroid.x - grid_ptr->params.rect.from.x) * grid_ptr->params.scale_factor_x); // Compute X grid position
  uint8_t keyPressY = (uint8_t)lround((blob_ptr->centroid.y - grid_ptr->params.rect.from.y) * grid_ptr->params.scale_factor_y); // Compute Y grid position
  uint8_t keyPress = keyPressY * grid_ptr->params.cols + keyPressX; // Compute 1D key index position
  // Serial.printf("\nGRID\tKEY:%d\tPOS_X:%d\tPOS_Y:%d", keyPress, keyPressX, keyPressY);
  // Serial.printf("\nGRID\tBLOB:%d\tBLOB_X:%f\tBLOB_Y:%f", blob_ptr->UID, blob_ptr->centroid.x, blob_ptr->centroid.x);
  switch (touch_ptr->msg.midi.type) {
    case midi::NoteOff:
      break;
    case midi::NoteOn:
      if (blob_ptr->status == PRESENT) { // Test if the blob is alive
        if (&grid_ptr->params.keys[keyPress].msg.midi != &grid_ptr->params.last_keys_ptr[blob_ptr->UID]->msg.midi) { // Test if the blob is touching a new key
          if (grid_ptr->params.last_keys_ptr[blob_ptr->UID] != NULL) { // Test if the blob was touching another key
            grid_ptr->params.last_keys_ptr[blob_ptr->UID]->msg.midi.type = midi::NoteOff;
            midi_sendOut(grid_ptr->params.last_keys_ptr[blob_ptr->UID]->msg.midi);
            #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_GRIDS)
              Serial.printf("\nDEBUG_MAPPINGS_GRIDS\tBLOB_ID:%d\tKEY_SLIDING_OFF:%d", blob_ptr->UID, grid_ptr->params.lastKeyPress[blob_ptr->UID]);
            #endif
            grid_ptr->params.last_keys_ptr[blob_ptr->UID] = NULL; // RAZ last key pressed value
          };
          grid_ptr->params.keys[keyPress].msg.midi.type = midi::NoteOn;
          midi_sendOut(grid_ptr->params.keys[keyPress].msg.midi);
          #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_GRIDS)
            Serial.printf("\nDEBUG_MAPPINGS_GRIDS\tBLOB_ID:%d\tKEY_PRESS:%d", blob_ptr->UID, grid_ptr->params.keys[keyPress].msg.midi);
          #endif
          grid_ptr->params.last_keys_ptr[blob_ptr->UID] = &grid_ptr->params.keys[keyPress]; // Keep track of last key pressed to switch it OFF when sliding
        };
      }
      else { // if !blob_ptr->status == PRESENT
        grid_ptr->params.last_keys_ptr[blob_ptr->UID]->msg.midi.type = midi::NoteOff;
        midi_sendOut(grid_ptr->params.last_keys_ptr[blob_ptr->UID]->msg.midi);
        #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_GRIDS)
          Serial.printf("\nDEBUG_MAPPINGS_GRIDS\tBLOB_ID:%d\tKEY_UP:%d", blob_ptr->UID, grid_ptr->params.lastKeyPress[blob_ptr->UID]);
        #endif
        grid_ptr->params.last_keys_ptr[blob_ptr->UID] = NULL; // RAZ last key pressed ptr value
      };
      break;
    case midi::AfterTouchPoly:
      // TODO
      break;
    case midi::ControlChange:
      if (blob_ptr->centroid.z != blob_ptr->last_centroid.z) {
        grid_ptr->params.keys[keyPress].msg.midi.data2 = blob_ptr->centroid.z;
        midi_sendOut(grid_ptr->params.keys[keyPress].msg.midi);
        #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS_GRIDS)
          Serial.printf("\nDEBUG_MAPPINGS_GRIDS\tID:%d\tC_CHANGE:%d", i, grid_ptr->params.msg.midi.data2);
        #endif
      }
      break;
    case midi::ProgramChange:
      break;
    case midi::AfterTouchChannel:
      break;
    case midi::PitchBend:
      break;
    case midi::SystemExclusive:
      break;
    default:
      // Not handled in switch
      break;
  };
};

void mapping_grid_create(const JsonObject &config) {
  mapp_grid_t* grid_ptr = (mapp_grid_t*)llist_pop_front(&llist_grid_pool);
  grid_ptr->params.rect.from.x = config["from"][0].as<float>();
  grid_ptr->params.rect.from.y = config["from"][1].as<float>();
  grid_ptr->params.rect.to.x = config["to"][0].as<float>();
  grid_ptr->params.rect.to.y = config["to"][1].as<float>();
  grid_ptr->params.cols = config["cols"].as<uint8_t>();
  grid_ptr->params.rows = config["rows"].as<uint8_t>();
  //grid_ptr->params.mode = config["mode_z"].as<uint8_t>();
  grid_ptr->params.keys_count = grid_ptr->params.cols * grid_ptr->params.rows;
  midi_status_t status;
  for (uint8_t j = 0; j < grid_ptr->params.keys_count; j++) {
    midi_msg_status_unpack(config["msg"][j]["midi"]["status"].as<uint8_t>(), &status);
    grid_ptr->params.keys[j].msg.midi.type = status.type;
    grid_ptr->params.keys[j].msg.midi.data1 = config["msg"][j]["midi"]["data1"].as<uint8_t>();
    grid_ptr->params.keys[j].msg.midi.data2 = config["msg"][j]["midi"]["data2"].as<uint8_t>();
    grid_ptr->params.keys[j].msg.midi.channel = status.channel;
    if (grid_ptr->params.keys[j].msg.midi.type == midi::ControlChange || 
        grid_ptr->params.keys[j].msg.midi.type == midi::AfterTouchPoly) {
      grid_ptr->params.keys[j].msg.limit.min = config["msg"][j]["limit"]["min"].as<uint8_t>();
      grid_ptr->params.keys[j].msg.limit.max = config["msg"][j]["limit"]["max"].as<uint8_t>();
    }
  }
  // Pre-compute key min & max coordinates using the pre-loaded config file
  int grid_size_x = grid_ptr->params.rect.to.x - grid_ptr->params.rect.from.x;
  //float key_size_x = (grid_size_x / grid_ptr_->params.cols) - grid_ptr_->params.gap;
  int grid_size_y = grid_ptr->params.rect.to.y - mapp_grids->params.rect.from.y;
  //float key_size_y = (grid_size_y / grid_ptr_->params.rows) - grid_ptr->params.gap;
  mapp_grids->params.scale_factor_x = ((float)1 / grid_size_x) * grid_ptr->params.cols;
  mapp_grids->params.scale_factor_y = ((float)1 / grid_size_y) * grid_ptr->params.rows;
  llist_push_back(&llist_mappings, grid_ptr);
};

/*
// This mode has been inspired by the Omnichord instrument: https://en.wikipedia.org/wiki/Omnichord
// It populates the MIDI grid layout with the incomming MIDI notes/chord coming from a keyboard plugged in the e256 HARDWARE_MIDI_INPUT
void mapping_grids_populate_dynamic(void) {
  bool newNote = false; // TODO: move it to grid_t struct
  for (uint8_t i = 0; i < mapp_grids; i++) {
    while (1) {
      midi_node_t* nodeIn_ptr = (midi_node_t*)ITERATOR_START_FROM_HEAD(&midi_in);
      if (nodeIn_ptr != NULL) {
        switch (nodeIn_ptr->midi.type) {
          case midi::NoteOn:
            // Move the input MIDI node to the midi_chord linked list
            llist_push_front(&midi_chord, llist_pop_front(&midi_in));
            newNote = true;
            break;
          case midi::NoteOff:
            // Remove and save the MIDI node from the midi_chord linked list
            // Save the nodeIn_ptr NOTE_OFF MIDI node from the midi_in linked list
            midi_node_t* prevNode_ptr = NULL;
            for (midi_node_t* nodeOut_ptr = (midi_node_t*)ITERATOR_START_FROM_HEAD(&midi_chord); nodeOut_ptr != NULL; nodeOut_ptr = (midi_node_t*)ITERATOR_NEXT(nodeOut_ptr)) {
              if (nodeIn_ptr->midi.data2 == nodeOut_ptr->midi.data2) {
                llist_push_front(&midi_nodes_pool, llist_pop_front(&midi_in));
                llist_extract_node(&midi_chord, prevNode_ptr, nodeOut_ptr);
                llist_push_front(&midi_nodes_pool, nodeOut_ptr);
                break;
              };
              prevNode_ptr = nodeOut_ptr;
            };
            break;
          //default:
          //break;
        };
      }
      else {
        break;
      };
    };
    if (newNote) {
      newNote = false;
      unsigned int key = 0;
      //while (key < GRID_KEYS) {
      while (key < mapp_grids_params[0].keys) { // FIXME: only works for one grid!
        for (midi_node_t* node_ptr = (midi_node_t*)ITERATOR_START_FROM_HEAD(&midi_chord); node_ptr != NULL; node_ptr = (midi_node_t*)ITERATOR_NEXT(node_ptr)) {
          mapp_grids_params[0].midiLayout[key] = node_ptr->midi;
          key++;
        };
      };
    };
  };
};
*/
