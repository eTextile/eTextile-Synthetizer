/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping.h"

#if MIDI_HARDWARE
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI);

void SETUP_MIDI_HARDWARE() {
  MIDI.begin(MIDI_CHANNEL_OMNI);
}
#endif

void getVelocity(llist_t* blobs_ptr, velocity_t* storage_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {

    storage_ptr[blob_ptr->UID].velocityX = blob_ptr->centroid.X - storage_ptr[blob_ptr->UID].lastValX;
    storage_ptr[blob_ptr->UID].velocityY = blob_ptr->centroid.Y - storage_ptr[blob_ptr->UID].lastValY;
    storage_ptr[blob_ptr->UID].velocityD = blob_ptr->box.D - storage_ptr[blob_ptr->UID].lastValD;

    storage_ptr[blob_ptr->UID].lastValX = blob_ptr->centroid.X;
    storage_ptr[blob_ptr->UID].lastValY = blob_ptr->centroid.Y;
    storage_ptr[blob_ptr->UID].lastValD = blob_ptr->box.D;

#if DEBUG_MAPPING
    Serial.printf("\nDEBUG_VELOCITY : X:%f\tY:%f\tZ:%f", storage_ptr[blob_ptr->UID].velocityX, storage_ptr[blob_ptr->UID].velocityY, storage_ptr[blob_ptr->UID].velocityD);
#endif
  }
}

void gridLayoutSet() {
  //TODO
}

// 
void ControlChangeMapping(llist_t* blobs_ptr, ccPesets_t* pesets_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    // Test if we are within the blob limit

    if (blob_ptr->UID < MAX_BLOBS && blob_ptr->UID == pesets_ptr->blobID) {
      // Test if the blob is alive
      if (blob_ptr->alive) {
        if (millis() - pesets_ptr->timer > DEBOUNCE_TIME_MIDI) {
          pesets_ptr->timer = millis();
#if MIDI_HARDWARE
          MIDI.sendControlChange(1, constrain(pesets_ptr->blobVal, 0, 127), midiChannel);
#endif
#if MIDI_USB
          usbMIDI.sendControlChange(1, constrain(pesets_ptr->blobVal, 0, 127), midiChannel);
#endif
#if DEBUG_MAPPING
          Serial.printf("\nCC:\tBLOB:%d\t\tCC_D:%d", blob_ptr->UID, constrain(pesets_ptr->blobVal, 0, 127));
#endif
        }
      }
      else {
#if MIDI_HARDWARE
        MIDI.sendControlChange(blob_ptr->UID, 0, pesets_ptr->midiChannel);
#endif
#if MIDI_USB
        usbMIDI.sendControlChange(blob_ptr->UID, 0, pesets_ptr->midiChannel);
#endif
#if DEBUG_MAPPING
        Serial.printf("\nCC:\tBLOB:%d\t\tCC_D:%d", blob_ptr->UID, 0);
#endif
      }
    }
  }
}

// Compute the grid Index location acording to blob X-Y positions
// Return curent & last keys presed
// TODO : Adding dead zone (gap between each keys)
void gridLayoutMapping_A(llist_t* blobs_ptr, grid_t* grid_ptr) {

  int16_t keyIndex = 0;

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    // Test if we are within the blob limit
    if (blob_ptr->UID < MAX_BLOBS) {
      // Test if the blob is alive
      if (blob_ptr->alive) {

        // Compute X and Y key position // TODO add keyGap to separate each keys
        uint8_t keyPosX = (uint8_t)round((blob_ptr->centroid.X / X_MAX) * X_GRID_STEP);        // Compute X window position
        uint8_t keyPosY = (uint8_t)round((blob_ptr->centroid.Y / Y_MAX) * X_GRID_STEP);        // Compute Y window position
        keyIndex = (keyPosY * Y_GRID_STEP + keyPosX);                                          // Compute 1D key index position

        if (keyIndex != grid_ptr->lastKeyIndex[blob_ptr->UID] &&                               // Send only if new key is pressed
            millis() - grid_ptr->timer[blob_ptr->UID] > DEBOUNCE_TIME_SWITCH) {         // Use some debounce to avoid erratic key triggering
          grid_ptr->timer[blob_ptr->UID] = millis();                                    // Reset debounce time
#if MIDI_HARDWARE
          if (grid_ptr->lastKeyIndex[blob_ptr->UID] != -1) {
            MIDI.sendNoteOff(grid_ptr->lastKeyIndex[blob_ptr->UID], 0, blob_ptr->UID + 1);     // Send NoteOFF (ONE BLOB ID PER CHANNEL)
            //MIDI.sendNoteOff(grid_ptr->lastKeyIndex[blob_ptr->UID], 0, 1);                   // Send NoteOFF (ALL BLOBS ON CHANNEL_1)
          }
          MIDI.sendNoteOn(keyIndex, 127, blob_ptr->UID + 1);                                   // Send NoteON (ONE BLOB ID PER CHANNEL)
          //MIDI.sendNoteOn(keyIndex, 127, 1);                                                 // Send NoteON (ALL BLOBS ON CHANNEL_1)
#endif
#if MIDI_USB
          if (grid_ptr->lastKeyIndex[blob_ptr->UID] != -1) {
            usbMIDI.sendNoteOff(grid_ptr->lastKeyIndex[blob_ptr->UID], 0, blob_ptr->UID + 1);  // Send NoteOFF (ONE BLOB ID PER CHANNEL)
            //usbMIDI.sendNoteOff(grid_ptr->lastKeyIndex[blob_ptr->UID], 0, 1);                // Send NoteOFF (ALL BLOBS ON CHANNEL_1)
          }
          usbMIDI.sendNoteOn(keyIndex, 127, blob_ptr->UID + 1);                                // Send NoteON (ONE BLOB ID PER CHANNEL)
          //usbMIDI.sendNoteOn(keyIndex, 127, 1);                                              // Send NoteON (ALL BLOBS ON CHANNEL_1)
#endif
#if DEBUG_MAPPING
          if (grid_ptr->lastKeyIndex[blob_ptr->UID] != -1) {
            Serial.printf("\nGRID_A:BLOB:%d\tKEY_OFF:%d", blob_ptr->UID, grid_ptr->lastKeyIndex[blob_ptr->UID]);
          }
          Serial.printf("\nGRID_A:\tBLOB:%d\t\tKEY_ON:%d", blob_ptr->UID, keyIndex);
#endif
          grid_ptr->lastKeyIndex[blob_ptr->UID] = keyIndex;                                     // Save the current key position
        }
      }
      else {
#if MIDI_HARDWARE
        MIDI.sendNoteOff(grid_ptr->lastKeyIndex[blob_ptr->UID], 0, blob_ptr->UID + 1);          // Send NoteOFF (ONE BLOB ID PER CHANNEL)
        //MIDI.sendNoteOff(grid_ptr->lastKeyIndex[blob_ptr->UID], 0, 1);                        // Send NoteOFF (ALL BLOBS ON CHANNEL_1)
#endif
#if MIDI_USB
        usbMIDI.sendNoteOff(grid_ptr->lastKeyIndex[blob_ptr->UID], 0, blob_ptr->UID + 1);       // Send NoteOFF (ONE BLOB ID PER CHANNEL)
        //MIDI.sendNoteOff(grid_ptr->lastKeyIndex[blob_ptr->UID], 0, 1);                        // Send NoteOFF (ALL BLOBS ON CHANNEL_1)
#endif
#if DEBUG_MAPPING
        Serial.printf("\nGRID_A:BLOB:%d\tKEY_OFF:%d", blob_ptr->UID, grid_ptr->lastKeyIndex[blob_ptr->UID]);
#endif
        grid_ptr->lastKeyIndex[blob_ptr->UID] = -1;
      }
    }
  }
}

// Pre compute key X-Y centroid positions
void SETUP_KEYBOARD_LAYOUT(gridPoint_t* keyPosArray_ptr) {

  for (int row = 0; row < X_GRID_STEP; row++) {
    for (int col = 0; col < Y_GRID_STEP; col++) {
      int index = row * X_GRID_STEP + col;
      keyPosArray_ptr[index].X = row + 1 * X_GRID_STRIDE;
      keyPosArray_ptr[index].Y = col + 1 * Y_GRID_STRIDE;
    }
  }
}

// This Grid Layaout fonction is disigned to be more robust than previous one
// By providing a better insolation (gap) betwin each keys
// points_t based on the MAX_X and MAX_Y surface
// Divide the grid ind sub window
// Localise the blob in sub windows and compute the nearest point of the grid
// Play it if it is not in the dead zone (gap between each keys)
void gridLayoutMapping_B(llist_t* blobs_ptr, grid_t* grid_ptr) {

  uint16_t keyIndex = 0;

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    // Test if we are within the blob limit
    if (blob_ptr->UID < MAX_BLOBS) {
      // Test if the blob is alive
      if (blob_ptr->alive) {

        // Compute X and Y window position
        uint8_t winPosX = (uint8_t)round((blob_ptr->centroid.X / X_MAX) * X_GRID_WINDOW);   // Compute X window position
        uint8_t winPosY = (uint8_t)round((blob_ptr->centroid.Y / Y_MAX) * Y_GRID_WINDOW);   // Compute Y window position
        uint16_t winPos = winPosY * Y_GRID_WINDOW + winPosX;                                // Compute 1D window position

        uint16_t indexArray[4] = {0};
        if (winPos != grid_ptr[blob_ptr->UID].lastWinPos) {
          // Compute 1D key index contained in the window
          indexArray[0] = winPosX * 2 + winPosY * Y_GRID_STEP;
          indexArray[1] = index[0] + 1;
          indexArray[2] = index[0] + X_GRID_STEP;
          indexArray[3] = index[2] + 1;
        }
        keyIndex = dist(blob_ptr, grid_ptr, &indexArray[0]);

        if (keyIndex != grid_ptr->lastKeyIndex[blob_ptr->UID] &&
            millis() - grid_ptr->timer[blob_ptr->UID] > DEBOUNCE_TIME_SWITCH) {
          grid_ptr->timer[blob_ptr->UID] = millis();
#if MIDI_HARDWARE
          if (grid_ptr->lastKeyIndex[blob_ptr->UID] != -1) {
            MIDI.sendNoteOff(grid_ptr->lastKeyIndex[blob_ptr->UID], 0, blob_ptr->UID + 1);      // Send NoteOFF
          }
          MIDI.sendNoteOn(keyIndex, 127, blob_ptr->UID + 1);                                    // Send NoteON
#endif
#if MIDI_USB
          if (grid_ptr->lastKeyIndex[blob_ptr->UID] != -1) {
            usbMIDI.sendNoteOff(grid_ptr->lastKeyIndex[blob_ptr->UID], 0, blob_ptr->UID + 1);   // Send NoteOFF
          }
          usbMIDI.sendNoteOn(keyIndex, 127, blob_ptr->UID + 1);                                 // Send NoteON
#endif
#if DEBUG_MAPPING
          if (grid_ptr->lastKeyIndex[blob_ptr->UID] != -1) {
            Serial.printf("\nGRID_B:BLOB:%d\tKEY_OFF:%d", blob_ptr->UID, grid_ptr->lastKeyIndex[blob_ptr->UID]);
          }
          Serial.printf("\nGRID_B:\tBLOB:%d\t\tKEY_ON:%d", blob_ptr->UID, keyIndex);
#endif
          grid_ptr->lastKeyIndex[blob_ptr->UID] = keyIndex; // Save the current position
        }
      }
      else {
        if (grid_ptr->lastKeyIndex[blob_ptr->UID] != -1) {
#if MIDI_HARDWARE
          MIDI.sendNoteOff(grid_ptr->lastKeyIndex[blob_ptr->UID], 0, blob_ptr->UID + 1);  // Send a Note (pitch 42, velo 127 on channel 1)
#endif
#if MIDI_USB
          usbMIDI.sendNoteOff(grid_ptr->lastKeyIndex[blob_ptr->UID], 0, blob_ptr->UID + 1);  // Send a Note (pitch 42, velo 127 on channel 1)
#endif
#if DEBUG_MAPPING
          Serial.printf("\nGRID_B:BLOB:%d\tKEY_OFF:%d", blob_ptr->UID, grid_ptr->lastKeyIndex[blob_ptr->UID]);
#endif
        }
        grid_ptr->lastKeyIndex[blob_ptr->UID] = -1;
      }
    }
  }
}

// keyIndexArray_ptr is pre computed to hold 2D index in 1D array
uint16_t dist(blob_t* blob_ptr, grid_t* grid_ptr, uint16_t* keyIndexArray_ptr) {

  uint16_t id = 0;
  float dist = 0.0f;
  float lastDist = 255.0f;

  for (int i = 0; i < 4; i++) {
    float sum = (blob_ptr->centroid.X - grid_ptr->keyPosArray[keyIndexArray_ptr[i]].X) * (blob_ptr->centroid.X - grid_ptr->keyPosArray[keyIndexArray_ptr[i]].X) +
                (blob_ptr->centroid.Y - grid_ptr->keyPosArray[keyIndexArray_ptr[i]].Y) * (blob_ptr->centroid.Y - grid_ptr->keyPosArray[keyIndexArray_ptr[i]].Y);
    float dist = sqrtf(sum);
    if (dist < lastDist) {
      lastDist = dist;
      id = keyIndexArray_ptr[i];
    }
  }
  if (lastDist < GRID_GAP) {
    return id;
  }
  else {
    return -1;
  }
}

void vSlider(llist_t* blobs_ptr, vSlider_t* slider_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.X > slider_ptr->posX - slider_ptr->width &&
        blob_ptr->centroid.X < slider_ptr->posX + slider_ptr->width) {
      if (blob_ptr->centroid.Y > slider_ptr->Ymin &&
          blob_ptr->centroid.Y < slider_ptr->Ymax) {

        slider_ptr->val = (uint8_t)round((blob_ptr->centroid.Y - (float)slider_ptr->Ymin) * ((slider_ptr->Ymax - slider_ptr->Ymin) / 127)); // [0:127]

#if DEBUG_MAPPING
        Serial.printf("\nDEBUG_V_SLIDER : %d", slider_ptr->val);
#endif
      }
    }
  }
}

// FIXME
void hSlider(llist_t* blobs_ptr, hSlider_t* slider_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.Y > slider_ptr->posY - slider_ptr->height &&
        blob_ptr->centroid.Y < slider_ptr->posY + slider_ptr->height) {
      if (blob_ptr->centroid.X > slider_ptr->Xmin &&
          blob_ptr->centroid.X < slider_ptr->Xmax) {

        slider_ptr->val = (uint8_t)round((blob_ptr->centroid.Y - (float)slider_ptr->Xmin) * ((slider_ptr->Xmax - slider_ptr->Xmin) / 127)); // [0:127]

#if DEBUG_MAPPING
        Serial.printf("\nDEBUG_V_SLIDER : %d", slider_ptr->val);
#endif
      }
    }
  }
}

void cSlider(llist_t* blobs_ptr, polar_t* polar_ptr, cSlider_t* slider_ptr) {
  float phi = 0;

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    for (int i = 0; i < C_SLIDERS; i++) {
      if (polar_ptr[blob_ptr->UID].r > slider_ptr[i].r - slider_ptr[i].width &&
          polar_ptr[blob_ptr->UID].r < slider_ptr[i].r + slider_ptr[i].width) {
        if (polar_ptr[blob_ptr->UID].phi > slider_ptr[i].phiOffset) {
          phi = polar_ptr[blob_ptr->UID].phi - slider_ptr[i].phiOffset;
        }
        else {
          phi = polar_ptr[blob_ptr->UID].phi + (6.2839 - slider_ptr[i].phiOffset);
        }
#if DEBUG_MAPPING
        Serial.printf("\nDEBUG_C_SLIDER_%d phi : %f", i, map(constrain(phi, 0.2, 5.9), 0.2, 5.9, 0, 127));
#endif
      }
    }
  }
}

void getPolarCoordinates(llist_t* blobs_ptr, polar_t* polar_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    float posX = blob_ptr->centroid.X - POLAR_CX;
    float posY = blob_ptr->centroid.Y - POLAR_CY;
    if (posX == 0 && posY == 0 ) {
      polar_ptr[blob_ptr->UID].r = 0;
      polar_ptr[blob_ptr->UID].phi = 0;
    }
    else {
      polar_ptr[blob_ptr->UID].r = sqrt(posX * posX + posY * posY);
      if (posX == 0 && 0 < posY) {
        polar_ptr[blob_ptr->UID].phi = PI / 2;
      } else if (posX == 0 && posY < 0) {
        polar_ptr[blob_ptr->UID].phi = PI * 3 / 2;
      } else if (posX < 0) {
        polar_ptr[blob_ptr->UID].phi = atan(posY / posX) + PI;
      } else if (posY < 0) {
        polar_ptr[blob_ptr->UID].phi = atan(posY / posX) + 2 * PI;
      } else {
        polar_ptr[blob_ptr->UID].phi = atan(posY / posX);
      }
    }
#if DEBUG_MAPPING
    Serial.printf("\nDEBUG_POLAR : R:%f\tPHY:%f", polar_ptr[blob_ptr->UID].r, polar_ptr[blob_ptr->UID].phi);
#endif
  }
}

boolean toggle(llist_t* blobs_ptr, tSwitch_t* switch_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.X > switch_ptr->posX - switch_ptr->rSize &&
        blob_ptr->centroid.X < switch_ptr->posX + switch_ptr->rSize) {
      if (blob_ptr->centroid.Y > switch_ptr->posY - switch_ptr->rSize &&
          blob_ptr->centroid.Y < switch_ptr->posY + switch_ptr->rSize) {
        if (millis() - switch_ptr->timer > DEBOUNCE_TIME_SWITCH) {
          switch_ptr->timer = millis();
          switch_ptr->state = !switch_ptr->state;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_TOGGLE : STATE:%d", switch_ptr->state);
#endif
          return switch_ptr->state;
        }
        switch_ptr->timer = millis();
      }
    }
  }
}

boolean trigger(llist_t* blobs_ptr, tSwitch_t* switch_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.X > switch_ptr->posX - switch_ptr->rSize &&
        blob_ptr->centroid.X < switch_ptr->posX + switch_ptr->rSize) {
      if (blob_ptr->centroid.Y > switch_ptr->posY - switch_ptr->rSize &&
          blob_ptr->centroid.Y < switch_ptr->posY + switch_ptr->rSize) {
        if (millis() - switch_ptr->timer > DEBOUNCE_TIME_SWITCH) {
          switch_ptr->timer = millis();
          switch_ptr->state = true;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_TRIGGER : POSX:%f\tPOSY:%f", blob_ptr->centroid.X, blob_ptr->centroid.Y);
#endif
          return true;
        }
        switch_ptr->timer = millis();
      }
    }
    else {
      return false;
    }
  }
}
