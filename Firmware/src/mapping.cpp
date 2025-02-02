/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping.h"

llist_t llist_mappings;

void mapping_lib_update(void) {
  
  llist_concat_nodes(&midi_nodes_pool, &midi_out); // Save/rescure all midi_out nodes

  for (lnode_t* blob_node_ptr = ITERATOR_START_FROM_HEAD(&llist_blobs); blob_node_ptr != NULL; blob_node_ptr = ITERATOR_NEXT(blob_node_ptr)) {
    blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(blob_node_ptr);
    for (lnode_t* common_node_ptr = ITERATOR_START_FROM_HEAD(&llist_mappings); common_node_ptr != NULL; common_node_ptr = ITERATOR_NEXT(common_node_ptr)) {
      common_t* mapping_common_ptr = (common_t*)ITERATOR_DATA(common_node_ptr);
      if (mapping_common_ptr->interact_func_ptr(blob_ptr, mapping_common_ptr)) {
        mapping_common_ptr->play_func_ptr(blob_ptr);
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

/*
typedef struct cSlider_s cSlider_t;
struct cSlider_s {
  midi_msg_t midiMsg;
  uint8_t id;
  float thetaMin;
  float thetaMax;
  float lastVal;
};
*/

/*
typedef struct cTrack_s cTrack_t;
struct cTrack_s {
  uint8_t sliders;
  uint8_t index;
  float offset;
};
*/

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
cSlider_t *mapp_csliders_params = NULL;
static cSlider_t mapp_csliders_params_privStore[MAX_CSLIDERS];
cSlider_t* mapp_cSliders_ptr[CS_SLIDERS] = {NULL};

void mapping_cslider_alloc(uint8_t csliders_cnt) {
  mapp_csliders = min(csliders_cnt, MAX_CSLIDERS);
  mapp_csliders_params = mapp_csliders_params_privStore;
};

inline void mapping_csliders_setup() {
  // Nothing to pre-compute yet
}

void mapping_cSliders_update(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_previous_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
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
      #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS)
        Serial.printf("\nDEBUG_MAPPINGS_CSLIDER\tRADIUS:%f\tTHETA:%f", radius, theta);
      #endif
      if (blob_ptr->status == PRESENT) {
        if (blob_ptr->status == NEW) {
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
                mapp_csliders->params.msg.midi.data2 = (uint8_t)map(theta, cSlider_ptr->thetaMin, cSlider_ptr->thetaMax, 0, 127);
                #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MAPPINGS)
                  Serial.printf("\nDEBUG_MAPPINGS_CSLIDER\tSLIDER_ID:%d\tRADIUS:%f\tTHETA:%f\tVAL:%d", id, radius, theta, sliderVal);
                #else
                  midi_sendOut(mapp_csliderParams[i].msg.midi);
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
