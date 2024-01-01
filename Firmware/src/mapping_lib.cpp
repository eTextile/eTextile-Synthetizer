/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping_lib.h"

///////////////////////////////////////////////
              /*__TRIGGERS__*/
///////////////////////////////////////////////
uint8_t mapp_trigs = 0;
keysroke_t *mapp_trigsParams = NULL;
static keysroke_t mapp_trigsParams_privStore[MAX_TRIGGERS];

void mapping_triggers_alloc(uint8_t triggers_cnt) {
  mapp_trigs = min(triggers_cnt, MAX_TRIGGERS);
  mapp_trigsParams = mapp_trigsParams_privStore;
};

inline void mapping_triggers_setup(void) {
  for (uint8_t i = 0; i < mapp_trigs; i++) {
    mapp_trigsParams[i].midiMsg.status = midi::ControlChange;
  };
};

void mapping_triggers_update(blob_t* blob_ptr) {
  for (uint8_t i = 0; i < mapp_trigs; i++) {
    // Test if the blob is within the key limits
    if (blob_ptr->centroid.x > mapp_trigsParams[i].rect.from.x &&
        blob_ptr->centroid.x < mapp_trigsParams[i].rect.to.x &&
        blob_ptr->centroid.y > mapp_trigsParams[i].rect.from.y &&
        blob_ptr->centroid.y < mapp_trigsParams[i].rect.to.y) {
      if (!blob_ptr->lastState) {
        #if defined(USB_MIDI_SERIAL) & defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_TRIGGERS\tID:%d\tNOTE_ON:%d", i, mapp_trigParams[i].midiMsg.data1);
        #else
          mapp_trigsParams[i].midiMsg.status = midi::NoteOn;
          midi_sendOut(mapp_trigsParams[i].midiMsg);
        #endif
      }
      else if (!blob_ptr->state) {
        #if defined(USB_MIDI_SERIAL) & defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_TRIGGERS\tID;%d\tNOTE_OFF:%d", i, mapp_trigParams[i].midiMsg.data1);
        #else
          mapp_trigsParams[i].midiMsg.status = midi::NoteOff;
          midi_sendOut(mapp_trigsParams[i].midiMsg);
        #endif
      };
    };
  };
};

///////////////////////////////////////////////
              /*__SWITCHS__*/
///////////////////////////////////////////////
uint8_t mapp_switchs = 0;
keysroke_t *mapp_switchParams = NULL;
static keysroke_t mapp_switchParams_privStore[MAX_SWITCHS];

void mapping_switchs_alloc(uint8_t switchs_cnt) {
  mapp_switchs = min(switchs_cnt, MAX_SWITCHS);
  mapp_switchParams = mapp_switchParams_privStore;
};

void mapping_switchs_setup(void) {
};

void mapping_switchs_update(blob_t* blob_ptr) {
  for (uint8_t i = 0; i < mapp_switchs; i++) {
    // Test if the blob is within the key limits
    if (blob_ptr->centroid.x > mapp_switchParams[i].rect.from.x &&
        blob_ptr->centroid.x < mapp_switchParams[i].rect.to.x &&
        blob_ptr->centroid.y > mapp_switchParams[i].rect.from.y &&
        blob_ptr->centroid.y < mapp_switchParams[i].rect.to.y) {
      if (!blob_ptr->lastState) {
        #if defined(USB_MIDI_SERIAL) & defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_SWITCHS\tID:%d\tNOTE_ON:%d", i, toggleParam[i].midiMsg.data1);
        #else
          mapp_switchParams[i].midiMsg.status = midi::NoteOn;
          midi_sendOut(mapp_switchParams[i].midiMsg);
        #endif
      };
      if (!blob_ptr->state) {
        #if defined(USB_MIDI_SERIAL) & defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_SWITCHS\tID:%d\tNOTE_OFF:%d", i, toggleParam[i].midiMsg.data1);
        #else
          mapp_switchParams[i].midiMsg.status = midi::NoteOff;
          midi_sendOut(mapp_switchParams[i].midiMsg);
        #endif
      };
    };
  };
};

///////////////////////////////////////////////
              /*__SLIDERS__*/
///////////////////////////////////////////////
uint8_t mapp_sliders = 0;
slider_t* mapp_slidersParams = NULL;
static slider_t mapp_slidersParams_privStore[MAX_SLIDERS];

void mapping_sliders_alloc(uint8_t sliders_cnt) {
  mapp_sliders = min(sliders_cnt, MAX_SLIDERS);
  mapp_slidersParams = mapp_slidersParams_privStore;
};

void mapping_sliders_setup(void) {
  for (uint8_t i = 0; i < mapp_sliders; i++) {
    mapp_slidersParams[i].midiMsg.status = midi::ControlChange;
    uint8_t size_x = mapp_slidersParams[i].rect.to.x - mapp_slidersParams[i].rect.from.x;
    uint8_t size_y = mapp_slidersParams[i].rect.to.y - mapp_slidersParams[i].rect.from.y;
    if (size_x > size_y){
      mapp_slidersParams[i].dir = H_SLIDER;
    } else {
      mapp_slidersParams[i].dir = V_SLIDER;
    };
  };
};

void mapping_sliders_update(blob_t* blob_ptr) {
  for (uint8_t i = 0; i < mapp_sliders; i++) {
    if (blob_ptr->centroid.x > mapp_slidersParams[i].rect.from.x &&
        blob_ptr->centroid.x < mapp_slidersParams[i].rect.to.x &&
        blob_ptr->centroid.y > mapp_slidersParams[i].rect.from.y &&
        blob_ptr->centroid.y < mapp_slidersParams[i].rect.to.y) {
      switch (mapp_slidersParams[i].dir){
        case V_SLIDER:
          mapp_slidersParams[i].midiMsg.data2 = round(map(
            blob_ptr->centroid.y,
            mapp_slidersParams[i].rect.from.y,
            mapp_slidersParams[i].rect.to.y,
            mapp_slidersParams[i].min,
            mapp_slidersParams[i].max)
          ); // [0:127]
        break;
        case H_SLIDER:
          mapp_slidersParams[i].midiMsg.data2 = round(map(
            blob_ptr->centroid.x,
            mapp_slidersParams[i].rect.from.x,
            mapp_slidersParams[i].rect.to.x,
            mapp_slidersParams[i].min,
            mapp_slidersParams[i].max)
            ); // [0:127]
        break;
      }
      if (mapp_slidersParams[i].midiMsg.data2 != mapp_slidersParams[i].last_val) {
        mapp_slidersParams[i].last_val = mapp_slidersParams[i].midiMsg.data2;
        #if defined(USB_MIDI_SERIAL) & defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_SLIDER\tID:%d\tVal:%d", i, mapp_slidersParams[i].midiMsg.data2);
        #else
          midi_sendOut(mapp_slidersParams[i].midiMsg);
        #endif
      };
      break;
    };
  };
};

///////////////////////////////////////////////
              /*__KNOBS__*/
///////////////////////////////////////////////
uint8_t mapp_knobs = 0;
knob_t *mapp_knobsParams = NULL;
static knob_t mapp_knobsParams_privStore[MAX_KNOBS];

void mapping_knobs_alloc(uint8_t knobs_cnt) {
  mapp_knobs = min(knobs_cnt, MAX_KNOBS);
  mapp_knobsParams = mapp_knobsParams_privStore;
};

void mapping_knobs_setup(void){
  for (int i = 0; i < mapp_knobs; i++) {
    mapp_knobsParams[i].midiMsg_radius.status = midi::ControlChange;
    mapp_knobsParams[i].midiMsg_theta.status = midi::ControlChange;
  };
};

void mapping_knobs_update(blob_t* blob_ptr) {
  for (uint8_t i = 0; i < mapp_knobs; i++) {
    float x = blob_ptr->centroid.x - mapp_knobsParams[i].center.x;
    float y = blob_ptr->centroid.y - mapp_knobsParams[i].center.y;
    float radius = sqrt(x * x + y * y);
    if (radius < mapp_knobsParams[i].midiMsg_radius.data2) {
      // Rotation of Axes through an angle without shifting Origin
      float posX = x * cos(mapp_knobsParams[i].offset) + y * sin(mapp_knobsParams[i].offset);
      float posY = -x * sin(mapp_knobsParams[i].offset) + y * cos(mapp_knobsParams[i].offset);
      if (posX == 0 && 0 < posY) {
        mapp_knobsParams[i].midiMsg_theta.data2 = PiII;
      } else if (posX == 0 && posY < 0) {
        mapp_knobsParams[i].midiMsg_theta.data2 = IIIPiII;
      } else if (posX < 0) {
        mapp_knobsParams[i].midiMsg_theta.data2 = atanf(posY / posX) + PI;
      } else if (posY < 0) {
        mapp_knobsParams[i].midiMsg_theta.data2 = atanf(posY / posX) + IIPi;
      } else {
        mapp_knobsParams[i].midiMsg_theta.data2 = atanf(posY / posX);
      }
      #if defined(USB_MIDI_SERIAL) & defined(DEBUG_MAPPING)
        Serial.printf("\nDEBUG_MAPPING_KNOBSS:\tKnobsID:\t%d\tradius:\t%fTheta:\t%f", i, mapp_knobsParams[i].midiMsg_radius.data2, mapp_knobsParams[i].midiMsg_theta.data2);
      #else
        midi_sendOut(mapp_knobsParams[i].midiMsg_radius);
        midi_sendOut(mapp_knobsParams[i].midiMsg_theta);
      #endif
    };
  };
};

///////////////////////////////////////////////
              /*__TOUCHPAD__*/
///////////////////////////////////////////////
uint8_t mapp_touchpads = 0;
touchpad_t* mapp_touchpadsParams = NULL;
static touchpad_t mapp_touchpadsParams_privStore[MAX_TOUCHPADS];

void mapping_touchpads_alloc(uint8_t touchpads_cnt) {
  mapp_touchpads = min(touchpads_cnt, MAX_TOUCHPADS);
  mapp_touchpadsParams = mapp_touchpadsParams_privStore;
};

void mapping_touchpads_setup(void){
  for (uint8_t i = 0; i < mapp_touchpads; i++) {
    for (uint8_t j = 0; j < mapp_touchpadsParams[i].touchs; j++) {
      mapp_touchpadsParams[i].touch[j].midiMsg_x.status = midi::ControlChange;
      mapp_touchpadsParams[i].touch[j].midiMsg_y.status = midi::ControlChange;
      mapp_touchpadsParams[i].touch[j].midiMsg_z.status = midi::ControlChange;
    };
  };
};

void mapping_touchpads_update(blob_t* blob_ptr) {
  for (uint8_t i = 0; i < mapp_touchpads; i++) {
    if (blob_ptr->centroid.x > mapp_touchpadsParams[i].rect.from.x &&
        blob_ptr->centroid.x < mapp_touchpadsParams[i].rect.to.x &&
        blob_ptr->centroid.y > mapp_touchpadsParams[i].rect.from.y &&
        blob_ptr->centroid.y < mapp_touchpadsParams[i].rect.to.y) {
      if (mapp_touchpadsParams[i].touch[blob_ptr->UID].midiMsg_x.data2) {
        #if defined(USB_MIDI_SERIAL) & defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_TOUCHPAD\tMIDI_x_cc:%d\tVAL:%d", 
          mapp_touchpadsParams[i].midiMsg_x.data2,
          round(map(blob_ptr->centroid.x, mapp_touchpadsParams[i].rect.from.x, mapp_touchpadsParams[i].rect.to.x, 0, 127)));
        #else
          mapp_touchpadsParams[i].touch[blob_ptr->UID].midiMsg_x.data2 = 
          round(map(blob_ptr->centroid.x,
            mapp_touchpadsParams[i].rect.from.x,
            mapp_touchpadsParams[i].rect.to.x,
            mapp_touchpadsParams[i].x_min,
            mapp_touchpadsParams[i].x_max
            )
          );
          midi_sendOut(mapp_touchpadsParams[i].touch[blob_ptr->UID].midiMsg_x);
        #endif
      };
      if (mapp_touchpadsParams[i].touch[blob_ptr->UID].midiMsg_y.data2) {
        #if defined(USB_MIDI_SERIAL) & defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_TOUCHPAD\tMIDI_y_cc:%d\tVAL:%d", 
          mapp_touchpadsParams[i].touch[blob_ptr->UID].midiMsg_y.data2, 
          round(map(blob_ptr->centroid.y, mapp_touchpadsParams[i].rect.from.y, mapp_touchpadsParams[i].rect.to.y, 0, 127)));
        #else
          mapp_touchpadsParams[i].touch[blob_ptr->UID].midiMsg_y.data2 = 
          round(map(blob_ptr->centroid.y,
            mapp_touchpadsParams[i].rect.from.y,
            mapp_touchpadsParams[i].rect.to.y,
            mapp_touchpadsParams[i].y_min,
            mapp_touchpadsParams[i].y_max
            )
          );
          midi_sendOut(mapp_touchpadsParams[i].touch[blob_ptr->UID].midiMsg_y);
        #endif
      };
      if (mapp_touchpadsParams[i].touch[blob_ptr->UID].midiMsg_z.data2) {
        #if defined(USB_MIDI_SERIAL) & defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_TOUCHPAD\tMIDI_z_cc:%d\tVAL:%d",
                        mapp_touchpadsParams[i].touch[blob_ptr->UID].midiMsg_z.data2,
                        map(blob_ptr->centroid.z, 0, 255, 0, 127));
        #else
          mapp_touchpadsParams[i].touch[blob_ptr->UID].midiMsg_y.data2 = 
          round(map(blob_ptr->centroid.z,
            Z_MIN,
            Z_MAX,
            mapp_touchpadsParams[i].z_min,
            mapp_touchpadsParams[i].z_max
            )
          );
          midi_sendOut(mapp_touchpadsParams[i].touch[blob_ptr->UID].midiMsg_z);
        #endif
      };
      //...
    };
  };
};

///////////////////////////////////////////////
              /*__POLYGONS__*/ 
///////////////////////////////////////////////
// Algorithm: http://alienryderflex.com/polygon
uint8_t mapp_polygons = 0;
polygon_t* mapp_polygonsParams = NULL;
static polygon_t mapp_polygonsParams_privStore[MAX_POLYGONS];

void mapping_polygons_alloc(uint8_t polygons_cnt) {
  mapp_polygons = min(polygons_cnt, MAX_POLYGONS);
  mapp_polygonsParams = mapp_polygonsParams_privStore;
};

// For line equation y = mx + c, we pre-compute m and c for all edges of a given polygon
// Testing
void mapping_polygons_setup(void) {
  for (uint8_t p = 0; p < mapp_polygons; p++) {
    float x1, x2, y1, y2;
    uint8_t v1, v2 = (mapp_polygonsParams[p].point_cnt - 1);
    mapp_polygonsParams[p].is_inside = false;
    for (v1 = 0; v1 < mapp_polygonsParams[p].point_cnt; v1++) {
      x1 = mapp_polygonsParams[p].point[v1].x;
      y1 = mapp_polygonsParams[p].point[v1].y;
      x2 = mapp_polygonsParams[p].point[v2].x;
      y2 = mapp_polygonsParams[p].point[v2].y;
      if (y2 == y1) {
        mapp_polygonsParams[p].c[v1] = x1;
        mapp_polygonsParams[p].m[v1] = 0;
      }
      else {
        mapp_polygonsParams[p].c[v1] = x1 - (y1 * x2) / (y2 - y1) + (y1 * x1) / (y2 - y1);
        mapp_polygonsParams[p].m[v1] = (x2 - x1) / (y2 - y1);
      };
      v2 = v1;
    };
  };
};

// Use to detect if a blob is inside a polygon
// We can draw polygons to define zones et/ou zones overlaps playing MIDI_NOTES
void mapping_polygons_update(blob_t* blob_ptr) {
  for (uint8_t p = 0; p < mapp_polygons; p++) {
    int i, j = (mapp_polygonsParams[p].point_cnt - 1);
    mapp_polygonsParams[p].is_inside = false;
    for (i = 0; i < mapp_polygonsParams[p].point_cnt; i++) {
      //float X1 = mapp_polygonsParams[p].point[i].x;
      float Y1 = mapp_polygonsParams[p].point[i].y;
      //float X2 = mapp_polygonsParams[p].point[j].x;
      float Y2 = mapp_polygonsParams[p].point[j].y;
      if ((Y1 < blob_ptr->centroid.y && Y2 >= blob_ptr->centroid.y) || (Y2 < blob_ptr->centroid.y && Y1 >= blob_ptr->centroid.y)) {
        // x ^= y; // equivalent to x = x ^ y;
        mapp_polygonsParams[p].is_inside ^= ((blob_ptr->centroid.y * mapp_polygonsParams[p].m[i] + mapp_polygonsParams[p].c[i]) < blob_ptr->centroid.x);
      };
      j = i;
    };
    if (mapp_polygonsParams[p].is_inside) {
      // TODO: get the max width & max height and scale it to [0-1]
      #if defined(USB_MIDI_SERIAL) & defined(DEBUG_MAPPING)
        Serial.printf("\nDEBUG_MAPPING_POLYGONS\tPoint %f %f is inside polygon %d\n", blob_ptr->centroid.x, blob_ptr->centroid.y, p);
      #else
      //
      #endif
      break;
    }
    else {
      #if defined(USB_MIDI_SERIAL) & defined(DEBUG_MAPPING)
        printf("\nDEBUG_MAPPING_POLYGONS\tPoint %f %f does not lie within any polygon\n", blob_ptr->centroid.x, blob_ptr->centroid.y);
      #endif
    }
  };
};

///////////////////////////////////////////////
/*__GRIDS__*/
///////////////////////////////////////////////

uint8_t mapp_grids = 0;
grid_t *mapp_gridsParams = NULL;
static grid_t mapp_gridsParams_privStore[MAX_GRIDS];

void mapping_grids_alloc(uint8_t grids_cnt){
  mapp_grids = min(grids_cnt, MAX_GRIDS);
  mapp_gridsParams = mapp_gridsParams_privStore;
};

// Pre-compute key min & max coordinates using the pre-loaded config file
void mapping_grids_setup(void){

  //uint8_t gridLayout[GRID_KEYS] = {0};

  for (uint8_t i = 0; i < mapp_grids; i++){
    int grid_size_x = mapp_gridsParams[i].rect.to.x - mapp_gridsParams[i].rect.from.x;
    //float key_size_x = (grid_size_x / mapp_gridsParams[i].cols) - mapp_gridsParams[i].gap;
    int grid_size_y = mapp_gridsParams[i].rect.to.y - mapp_gridsParams[i].rect.from.y;
    //float key_size_y = (grid_size_y / mapp_gridsParams[i].rows) - mapp_gridsParams[i].gap;
    mapp_gridsParams[i].scale_factor_x = ((float)1 / grid_size_x) * mapp_gridsParams[i].cols;
    mapp_gridsParams[i].scale_factor_y = ((float)1 / grid_size_y) * mapp_gridsParams[i].rows;
  };
};

// Compute the keyPresed position acording to the blobs XY (centroid) coordinates
// Add corresponding MIDI message to the MIDI out liked list
void mapping_grids_update(blob_t *blob_ptr){
  for (uint8_t i = 0; i < mapp_grids; i++){
  // Test if the blob is within the grid limits
    if (blob_ptr->centroid.x > mapp_gridsParams[i].rect.from.x &&
        blob_ptr->centroid.x < mapp_gridsParams[i].rect.to.x &&
        blob_ptr->centroid.y > mapp_gridsParams[i].rect.from.y &&
        blob_ptr->centroid.y < mapp_gridsParams[i].rect.to.y){

      uint8_t keyPressX = lround((blob_ptr->centroid.x - mapp_gridsParams[i].rect.from.x) * mapp_gridsParams[i].scale_factor_x); // Compute X grid position
      uint8_t keyPressY = lround((blob_ptr->centroid.y - mapp_gridsParams[i].rect.from.y) * mapp_gridsParams[i].scale_factor_y); // Compute Y grid position
      uint8_t keyPress = keyPressY * mapp_gridsParams[i].cols + keyPressX; // Compute 1D key index position
      // Serial.printf("\nGRID\tKEY:%d\tPOS_X:%d\tPOS_Y:%d", keyPress, keyPressX, keyPressY);
      // Serial.printf("\nGRID\tBLOB:%d\tBLOB_X:%f\tBLOB_Y:%f", blob_ptr->UID, blob_ptr->centroid.x, blob_ptr->centroid.x);
      
      if (blob_ptr->state){ // Test if the blob is alive
        if (&mapp_gridsParams[i].midiLayout[keyPress] != mapp_gridsParams[i].lastKeyPress[blob_ptr->UID]){ // Test if the blob is touching a new key
          if (mapp_gridsParams[i].lastKeyPress[blob_ptr->UID] != NULL){ // Test if the blob was touching another key
          #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPING)
            Serial.printf("\nDEBUG_MAPPING_GRID\tBLOB_ID:%d\tKEY_SLIDING_OFF:%d", blob_ptr->UID, lastKeyPress[blob_ptr->UID]);
          #else
            mapp_gridsParams[i].lastKeyPress[blob_ptr->UID]->status = midi::NoteOff;
            midi_sendOut((midiMsg_t)*mapp_gridsParams[i].lastKeyPress[blob_ptr->UID]);
          #endif
            mapp_gridsParams[i].lastKeyPress[blob_ptr->UID] = NULL; // RAZ last key pressed value
          };
          #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPING)
            Serial.printf("\nDEBUG_MAPPING_GRID\tBLOB_ID:%d\tKEY_PRESS:%d", blob_ptr->UID, mapp_gridsParams[i].midiLayout[keyPress]);
          #else
            mapp_gridsParams[i].midiLayout[keyPress].status = midi::NoteOn;
            midi_sendOut(mapp_gridsParams[i].midiLayout[keyPress]);
          #endif
            mapp_gridsParams[i].lastKeyPress[blob_ptr->UID] = &mapp_gridsParams[i].midiLayout[keyPress]; // Keep track of last key pressed to switch it OFF when sliding
        };
      }
      else { // if !blob_ptr->state
      #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPING)
        Serial.printf("\nDEBUG_MAPPING_GRID\tBLOB_ID:%d\tKEY_UP:%d", blob_ptr->UID, lastKeyPress[blob_ptr->UID]);
      #else
        mapp_gridsParams[i].lastKeyPress[blob_ptr->UID]->status = midi::NoteOff;
        midi_sendOut((midiMsg_t)*mapp_gridsParams[i].lastKeyPress[blob_ptr->UID]);
      #endif
        mapp_gridsParams[i].lastKeyPress[blob_ptr->UID] = NULL; // RAZ last key pressed ptr value
      };
        
    };
  };
};

// Experimental: populate the MIDI grid layaout with the incomming MIDI notes / chord (from MIDI_IN HARD plug)
// Populate the MIDI grid layaout with the incomming MIDI notes / chord (from MIDI_IN HARD plug)
void mapping_grids_populate_dynamic(void) {
  bool newNote = false;
  while (1) {
    midiNode_t* nodeIn_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiIn);
    if (nodeIn_ptr != NULL) {
      switch (nodeIn_ptr->midiMsg.status) {
        case midi::NoteOn:
          // Move the input MIDI node to the midiChord linked list
          llist_push_front(&midiChord, llist_pop_front(&midiIn));
          newNote = true;
          break;
        case midi::NoteOff:
          // Remove and save the MIDI node from the midiChord linked list
          // Save the nodeIn_ptr NOTE_OFF MIDI node from the midiIn linked list
          midiNode_t* prevNode_ptr = NULL;
          for (midiNode_t* nodeOut_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiChord); nodeOut_ptr != NULL; nodeOut_ptr = (midiNode_t*)ITERATOR_NEXT(nodeOut_ptr)) {
            if (nodeIn_ptr->midiMsg.data2 == nodeOut_ptr->midiMsg.data2) {
              llist_push_front(&midi_node_stack, llist_pop_front(&midiIn));
              llist_extract_node(&midiChord, prevNode_ptr, nodeOut_ptr);
              llist_push_front(&midi_node_stack, nodeOut_ptr);
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
    while (key < mapp_gridsParams[0].keys) { // FIXME!
      for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiChord); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
        gridLayout[key] = node_ptr->midiMsg.data2;
        key++;
      };
    };
  };
};

///////////////////////////////////////////////
              /*__PATH__*/ 
///////////////////////////////////////////////
// TODO

///////////////////////////////////////////////
              /*__CROSFADE__*/ 
///////////////////////////////////////////////
// TODO

///////////////////////////////////////////////
              /*__C_SLIDERS__*/ 
///////////////////////////////////////////////
// https://www.flickr.com/photos/maurin/50866229007/in/dateposted-public/
// CIRCULAR_SLIDERS_CONSTANTS
// This is not yet modified to work with loaded config file!!
/*
#define CS_TRACKS         4
#define CS_SLIDERS        7
#define CS_RADIUS         (float)(X_MAX - X_MIN) / 2
#define CS_MARGIN         (float)0.5
#define CS_RMIN           (float)3
#define CS_RMAX           (float)(CS_RADIUS - CS_MARGIN)
#define CS_TRACK_WIDTH    (float)((CS_RMAX - CS_RMIN) / CS_TRACKS)
#define CS_SCALE_FACTOR   (float)(1 / CS_TRACK_WIDTH)

cTrack_t cTrack[CS_TRACKS] = {
  {1, 0, 0  },      // PARAMS[sliders, index, track-offset-rad]
  {1, 1, 1  },      // PARAMS[sliders, index, track-offset-rad]
  {2, 2, 2.4},      // PARAMS[sliders, index, track-offset-rad]
  {3, 4, 3.0}       // PARAMS[sliders, index, track-offset-rad]
};

cSlider_t mapp_cSliders[CS_SLIDERS] = {
  {1, 0,    IIPi, 0},  // PARAMS[id, thetaMin, thetaMax, lastVal]
  {2, 0,    IIPi, 0},  // PARAMS[id, thetaMin, thetaMax, lastVal]
  {3, 0,    3.60, 0},  // PARAMS[id, thetaMin, thetaMax, lastVal]
  {4, 3.80, IIPi, 0},  // PARAMS[id, thetaMin, thetaMax, lastVal]
  {5, 0,    3.20, 0},  // PARAMS[id, thetaMin, thetaMax, lastVal]
  {6, 3.40, 5.20, 0},  // PARAMS[id, thetaMin, thetaMax, lastVal]
  {7, 5.30, 8, IIPi}   // PARAMS[id, thetaMin, thetaMax, lastVal]
};

uint8_t mapp_csliders = 0;
cSlider_t *mapp_cslidersParams = NULL;
static cSlider_t mapp_cslidersParams_privStore[MAX_CSLIDERS];
cSlider_t* mapp_cSliders_ptr[CS_SLIDERS] = {NULL};

void mapping_cslider_alloc(uint8_t csliders_cnt){
  mapp_csliders = min(csliders_cnt, MAX_CSLIDERS);
  mapp_cslidersParams = mapp_cslidersParams_privStore;
};

inline void mapping_csliders_setup() {
  // Nothing to pre-compute yet
}

void mapping_cSliders_update(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    float x = blob_ptr->centroid.x - CS_RADIUS;
    float y = blob_ptr->centroid.y - CS_RADIUS;
    float radius = sqrt(x * x + y * y);
    if (radius > CS_RMIN && radius < CS_RMAX) {
      uint8_t track = (uint8_t)round(radius * CS_SCALE_FACTOR) - 1; // Compute track position
      // Rotation of Axes through an angle without shifting Origin
      float posX = x * cos(cTrack[track].offset) + y * sin(cTrack[track].offset);
      float posY = -x * sin(cTrack[track].offset) + y * cos(cTrack[track].offset);
      if (posX == 0 && 0 < posY) {
        theta = PiII;
      } else if (posX == 0 && posY < 0) {
        theta = IIIPiII;
      } else if (posX < 0) {
        theta = atanf(posY / posX) + PI;
      } else if (posY < 0) {
        theta = atanf(posY / posX) + IIPi;
      } else {
        theta = atanf(posY / posX);
      }
      #if defined(USB_MIDI_SERIAL) & defined(DEBUG_MAPPING)
        Serial.printf("\nDEBUG_MAPPING_CSLIDER\tRADIUS:%f\tTHETA:%f", radius, theta);
      #endif
      if (blob_ptr->state) {
        if (!blob_ptr->lastState) {
          for (uint8_t id = cTrack[track].index; id < cTrack[track].index + cTrack[track].sliders; id++) {
            if (theta > mapp_cSliders[id].thetaMin && theta < mapp_cSliders[id].thetaMax) {
              mapp_cSliders_ptr[blob_ptr->UID] = &mapp_cSliders[id]; // Record pointer to slider
            };
          };
        }
        else {
          cSlider_t* cSlider_ptr = mapp_cSliders_ptr[blob_ptr->UID];
          if (cSlider_ptr != NULL) {
            if (theta > cSlider_ptr->thetaMin && theta < cSlider_ptr->thetaMax) {
              if (theta != cSlider_ptr->lastVal) {
                cSlider_ptr->lastVal = theta;
                mapp_cslidersParams[i].midiMsg.data2 = (uint8_t)map(theta, cSlider_ptr->thetaMin, cSlider_ptr->thetaMax, 0, 127);
                #if defined(USB_MIDI_SERIAL) & defined(DEBUG_MAPPING)
                  Serial.printf("\nDEBUG_MAPPING_CSLIDER\tSLIDER_ID:%d\tRADIUS:%f\tTHETA:%f\tVAL:%d", id, radius, theta, sliderVal);
                #else
                  midi_sendOut(mapp_csliderParams[i].midiMsg);
                #endif
              };
            };
          };
        };
      }
      else {
        mapp_cSliders_ptr[blob_ptr->UID] = NULL;
      };
    };
  };
};
*/

void mapping_lib_setup(void){
  mapping_triggers_setup();
  mapping_switchs_setup();
  mapping_sliders_setup();
  mapping_knobs_setup();
  mapping_touchpads_setup();
  mapping_polygons_setup();
  mapping_grids_setup();
};

void mapping_lib_update(void) {
  llist_save_nodes(&midi_node_stack, &midiOut); // Save/rescure all midiOut nodes
  for (blob_t *blob_ptr = (blob_t *)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t *)ITERATOR_NEXT(blob_ptr)){
    mapping_touchpads_update(blob_ptr);
    mapping_polygons_update(blob_ptr);
    mapping_knobs_update(blob_ptr);
    mapping_triggers_update(blob_ptr);
    mapping_switchs_update(blob_ptr);
    mapping_sliders_update(blob_ptr);
    mapping_grids_update(blob_ptr);
    //mapping_cSliders_update(&blob_ptr); // Experimental
  }
};
