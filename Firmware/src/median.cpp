/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// https://github.com/RobTillaart/RunningMedian/blob/2a155ba97f1d393077590decc71f770f1962a994/RunningMedian.cpp

#include "median.h"

median_t filter[MAX_BLOBS];

// resets all internal counters
void running_median_setup(void) {
  for (uint8_t i = 0; i < MAX_BLOBS; i++) {
    for (uint8_t j = 0; j < M_WINDOW; j++) {
      filter[i].X_sort[j] = j;
      filter[i].Y_sort[j] = j;
      filter[i].Z_sort[j] = j;
    };
  };
};

// Adds a new value to the data-set
// or overwrites the oldest if buffer is full
void runing_median(void) {

  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {

    if (!blob_ptr->lastState) {
      filter[blob_ptr->UID].count = 1; // Circular buffer fill index
      filter[blob_ptr->UID].index = 1; // Circular buffer runing index
    };

    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MEDIAN)
      //Serial.printf("\nCOUNT_%d\tINDEX_%d", filter[0].count, filter[0].index);
    #endif
    // Add the new values to the input runing buffer
    filter[blob_ptr->UID].X_rawVal[filter[blob_ptr->UID].index] = blob_ptr->centroid.x;
    filter[blob_ptr->UID].Y_rawVal[filter[blob_ptr->UID].index] = blob_ptr->centroid.y;
    //filter[blob_ptr->UID].Z_rawVal[filter[blob_ptr->UID].index] = blob_ptr->centroid.z;

    // Sort the [X] values
    for (uint8_t i = 1; i <= filter[blob_ptr->UID].count; i++) {
      uint8_t j = i;
      uint8_t tempIndex = filter[blob_ptr->UID].X_sort[j];
      while ((j > 0) && (filter[blob_ptr->UID].X_rawVal[tempIndex] < filter[blob_ptr->UID].X_rawVal[filter[blob_ptr->UID].X_sort[j - 1]])) {
        filter[blob_ptr->UID].X_sort[j] = filter[blob_ptr->UID].X_sort[j - 1];
        j--;
      };
      filter[blob_ptr->UID].X_sort[j] = tempIndex;
    };


    // Sort the [Y] values
    for (uint8_t i = 1; i <= filter[blob_ptr->UID].count; i++) {
      uint8_t j = i;
      uint8_t tempIndex = filter[blob_ptr->UID].Y_sort[j];
      while ((j > 0) && (filter[blob_ptr->UID].Y_rawVal[tempIndex] < filter[blob_ptr->UID].Y_rawVal[filter[blob_ptr->UID].Y_sort[j - 1]])) {
        filter[blob_ptr->UID].Y_sort[j] = filter[blob_ptr->UID].Y_sort[j - 1];
        j--;
      };
      filter[blob_ptr->UID].X_sort[j] = tempIndex;
    };

    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MEDIAN)
      Serial.printf("\nDEBUG_MEDIAN : ");
      for (uint8_t i = 0; i <= filter[0].count; i++) {
       Serial.printf("\t%d_%d_%f", i, filter[0].X_sort[i], filter[0].X_rawVal[i]);
      }
    #endif

    // Get sorted element
    if (filter[blob_ptr->UID].count == 0) {
    }
    else if (filter[blob_ptr->UID].count == 1) {
      blob_ptr->centroid.x = filter[blob_ptr->UID].X_rawVal[1];
      blob_ptr->centroid.y = filter[blob_ptr->UID].Y_rawVal[1];
      //blob_ptr->centroid.z = filter[blob_ptr->UID].Z_rawVal[1];
    }
    else if (filter[blob_ptr->UID].count == 2) {
      blob_ptr->centroid.x = filter[blob_ptr->UID].X_rawVal[1];
      blob_ptr->centroid.y = filter[blob_ptr->UID].Y_rawVal[1];
      //blob_ptr->centroid.z = filter[blob_ptr->UID].Z_rawVal[1];
    }
    else if (filter[blob_ptr->UID].count == 3) {
      blob_ptr->centroid.x = filter[blob_ptr->UID].X_rawVal[1];
      blob_ptr->centroid.y = filter[blob_ptr->UID].Y_rawVal[1];
      //blob_ptr->centroid.z = filter[blob_ptr->UID].Z_rawVal[1];
    }
    else if (filter[blob_ptr->UID].count >= 4) {
      blob_ptr->centroid.x  = filter[blob_ptr->UID].X_rawVal[2];
      blob_ptr->centroid.y  = filter[blob_ptr->UID].Y_rawVal[2];
      //blob_ptr->centroid.z  = filter[blob_ptr->UID].Z_rawVal[2];
    };

    if (filter[blob_ptr->UID].index >= M_WINDOW) {
      filter[blob_ptr->UID].index = 0;
    } else {
      filter[blob_ptr->UID].index++;
    }
    if (filter[blob_ptr->UID].count < M_WINDOW) filter[blob_ptr->UID].count++;
  };
};
