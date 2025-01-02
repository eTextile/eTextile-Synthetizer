/*
  FORKED FROM https://github.com/openmv/openmv/blob/master/src/omv/imlib/blob.c
    - This file is part of the OpenMV project.
    - Copyright (c) 2013- Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
    - This work is licensed under the MIT license, see the file LICENSE for details.

  Added custom blob détection algorithm to keep track of the blobs ID's
    - This patch is part of the eTextile-Synthesizer project - http://synth.eTextile.org
    - Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
    - This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "interp.h"
#include "blob.h"
#include "median.h"

#define LIFO_NODES          512           // Set the maximum nodes number
#define X_STRIDE            4             // Speed up X scanning
#define Y_STRIDE            2             // Speed up Y scanning
#define DEBOUNCE_TIME       50            // Avioding undesired bouncing effect when tapping or sliding on the sensor

uint8_t bitmapArray[NEW_FRAME] = {0};     // Store (64*64) binary values
xylr_t lifoArray[LIFO_NODES] = {0};       // Store lifo nodes
blob_t blobArray[MAX_BLOBS] = {0};        // Store blobs

llist_t llist_context_stack;              // Free nodes stack
llist_t llist_context;                    // Used nodes
llist_t llist_blobs_stack;                // Free nodes stack
llist_t llist_blobs_temp;                 // Intermediate blobs linked list
llist_t llist_blobs;                      // Output blobs linked list

void blob_setup(void) {
  llist_builder(&llist_context_stack, &lifoArray[0], LIFO_NODES, sizeof(lifoArray[0])); // Add X nodes to the llist_context_stack
  llist_builder(&llist_blobs_stack, &blobArray[0], MAX_BLOBS, sizeof(blobArray[0])); // Add X nodes to the llist_blobs_stack
  llist_raz(&llist_context);
  llist_raz(&llist_blobs_temp);
  llist_raz(&llist_blobs);
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs_stack); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    blob_ptr->UID = 255;
    blob_ptr->lastState = false;
    blob_ptr->state = false;
  };
};

/////////////////////////////// Scanline flood fill algorithm / SFF
/////////////////////////////// Connected-component labeling / CCL
void matrix_find_blobs(void) {

  memset((uint8_t*)bitmapArray, 0, SIZEOF_FRAME);
  uint8_t blob_count = 0;
  
  for (uint8_t posY = 0; posY < NEW_ROWS; posY += Y_STRIDE) {

    uint8_t* row_ptr_A = COMPUTE_IMAGE_ROW_PTR(&interpFrame, posY);
    uint8_t* bmp_row_ptr_A = &bitmapArray[0] + posY * NEW_COLS;

    for (uint8_t posX = (posY % X_STRIDE); posX < NEW_COLS; posX += X_STRIDE) {
      if (!IMAGE_GET_PIXEL_FAST(bmp_row_ptr_A, posX) &&
          PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_A, posX), e256_ctr.levels[THRESHOLD].val)
         ) {

        uint8_t oldX = posX;
        uint8_t oldY = posY;

        uint8_t blob_x1 = posX;
        uint8_t blob_x2 = posX;

        uint8_t blob_height = 0;
        uint8_t blob_depth = 0;

        uint16_t blob_pixels = 0;
        float blob_cx = 0;
        float blob_cy = 0;

        while (1) { // while_A
          uint8_t left = posX;
          uint8_t right = posX;

          uint8_t* row_ptr_B = COMPUTE_IMAGE_ROW_PTR(&interpFrame, posY);
          uint8_t* bmp_row_ptr_B = &bitmapArray[0] + posY * NEW_COLS;

          while ((left > 0) &&
                 !IMAGE_GET_PIXEL_FAST(bmp_row_ptr_B, left - 1) &&
                 PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, left - 1), e256_ctr.levels[THRESHOLD].val)
                ) {
            left--;
          };
          while (right < (NEW_COLS - 1) &&
                 !IMAGE_GET_PIXEL_FAST(bmp_row_ptr_B, right + 1) &&
                 PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, right + 1), e256_ctr.levels[THRESHOLD].val)
                ) {
            right++;
          };

          blob_x1 = MIN(blob_x1, left);
          blob_x2 = MAX(blob_x2, right);

          for (int i = left; i <= right; i++) {
            IMAGE_SET_PIXEL_FAST(bmp_row_ptr_B, i, 1);
            blob_depth = MAX(blob_depth, IMAGE_GET_PIXEL_FAST(row_ptr_B, i));
          };

          float sum = ((right * (right + 1)) - (left * (left - 1))) / 2;

          uint8_t pixels = right - left + 1;
          blob_pixels += pixels;
          blob_cx += sum;
          blob_cy += posY * pixels;

          uint8_t top_left = left;
          uint8_t bot_left = left;

          bool break_out = false;

          while (1) { // while_B

            if (posY > 0) {
              row_ptr_B = COMPUTE_IMAGE_ROW_PTR(&interpFrame, posY - 1);
              bmp_row_ptr_B = &bitmapArray[0] + (posY - 1) * NEW_COLS;

              bool recurse = true;
              for (uint8_t i = top_left; i <= right; i++) {

                if ((!IMAGE_GET_PIXEL_FAST(bmp_row_ptr_B, i))
                    && (PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, i), e256_ctr.levels[THRESHOLD].val))) {

                  xylr_t* context = (xylr_t*)llist_pop_front(&llist_context_stack);
                  context->x = posX;
                  context->y = posY;
                  context->l = left;
                  context->r = right;
                  context->t_l = i++; // Don't test the same pixel again
                  context->b_l = bot_left;
                  llist_push_front(&llist_context, context);
                  posX = i;
                  posY--;
                  recurse = false;
                  break;
                };
              };
              if (!recurse) {
                break;
              };
            };

            row_ptr_B = COMPUTE_IMAGE_ROW_PTR(&interpFrame, posY + 1);
            bmp_row_ptr_B = &bitmapArray[0] + (posY + 1) * NEW_COLS;

            bool recurse = true;
            for (uint8_t i = bot_left; i <= right; i++) {

              if (!IMAGE_GET_PIXEL_FAST(bmp_row_ptr_B, i) &&
                  PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, i), e256_ctr.levels[THRESHOLD].val)) {

                xylr_t* context = (xylr_t*)llist_pop_front(&llist_context_stack);
                context->x = posX;
                context->y = posY;
                context->l = left;
                context->r = right;
                context->t_l = top_left;
                context->b_l = i++; // Don't test the same pixel again
                llist_push_front(&llist_context, context);
                posX = i;
                posY++;
                recurse = false;
                break;
              };
            };

            if (!recurse) {
              break;
            };
            if (llist_context.head_ptr == NULL) {
              break_out = true;
              break;
            };

            xylr_t* context = (xylr_t*)llist_pop_front(&llist_context);
            posX = context->x;
            posY = context->y;
            left = context->l;
            right = context->r;
            top_left = context->t_l;
            bot_left = context->b_l;
            llist_push_front(&llist_context_stack, context);
            blob_height++;
          }; // END while_B

          if (break_out) {
            break;
          };
        }; // END while_A

        if (blob_pixels > BLOB_MIN_PIX && blob_pixels < BLOB_MAX_PIX && blob_count < MAX_BLOBS) {
          blob_count++;
          blob_t* blob_ptr = (blob_t*)llist_pop_front(&llist_blobs_stack);
          blob_ptr->last_centroid = blob_ptr->centroid;
          blob_ptr->centroid.x = constrain(blob_cx / blob_pixels, X_MIN, X_MAX) - X_MIN ;
          blob_ptr->centroid.y = constrain(blob_cy / blob_pixels, Y_MIN, Y_MAX) - Y_MIN;
          
          // blob_ptr->centroid.z = blob_depth - e256_ctr.levels[THRESHOLD].val; // TODO add a limit to avoid negative numbers!
          blob_depth > e256_ctr.levels[THRESHOLD].val ? blob_ptr->centroid.z = (blob_depth - e256_ctr.levels[THRESHOLD].val) : blob_ptr->centroid.z = 0; // TESTING

          blob_ptr->box.w = (blob_x2 - blob_x1);
          blob_ptr->box.h = blob_height;
          llist_push_front(&llist_blobs_temp, blob_ptr);
        };
        posX = oldX;
        posY = oldY;
      };
    };
  };

  /////////////////////////////// PERSISTANT BLOB ID

  // DEAD BLOBS REMOVER
  blob_t* prevBlob_ptr = NULL;
  while (1) {
    bool deadFound = false;
    for (blob_t* blobOut_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blobOut_ptr != NULL; blobOut_ptr = (blob_t*)ITERATOR_NEXT(blobOut_ptr)) {
      if (blobOut_ptr->status == TO_REMOVE) {
        deadFound = true;
        blobOut_ptr->UID = 255;
        blobOut_ptr->status = FREE;
        //blobOut_ptr->state = false; // __TO_REMOVE_LATER__
        blobOut_ptr->lastState = false;
        llist_extract_node(&llist_blobs, prevBlob_ptr, blobOut_ptr);
        llist_push_front(&llist_blobs_stack, blobOut_ptr);
        #if defined(USB_MIDI_SERIAL) && defined(DEBUG_FIND_BLOBS)
          Serial.printf("\nDEBUG_FIND_BLOBS / Blob: %p removed from **llist_blobs**", (lnode_t*)blobOut_ptr);
        #endif
        break;
      };
      prevBlob_ptr = blobOut_ptr;
    };
    if (!deadFound) {
      break;
    };
  };

  // NEW BLOBS MANAGMENT
  // Look for corresponding blobs into the **llist_blobs_temp** and **llist_blobs**
  uint8_t minID = 0;
  blob_t* nearestBlob_ptr = NULL;
  for (blob_t* blobIn_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs_temp); blobIn_ptr != NULL; blobIn_ptr = (blob_t*)ITERATOR_NEXT(blobIn_ptr)) {
    float minDist = 255.0f;
    for (blob_t* blobOut_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blobOut_ptr != NULL; blobOut_ptr = (blob_t*)ITERATOR_NEXT(blobOut_ptr)) {
      float dist = sqrtf(pow(blobIn_ptr->centroid.x - blobOut_ptr->centroid.x, 2) + pow(blobIn_ptr->centroid.y - blobOut_ptr->centroid.y, 2));
      if (dist < minDist) {
        minDist = dist;
        nearestBlob_ptr = blobOut_ptr;
      };
    };
    // If the distance between curent blob and last blob position is less than minDist:
    // Give the nearestBlob UID to the input blob.
    if (minDist < 4) {
      #if defined(USB_MIDI_SERIAL) && defined(DEBUG_FIND_BLOBS)
        //Serial.printf("\nDEBUG_FIND_BLOBS / the minimum distance between blobs from llist_blobs_temp & llist_blobs is: %f ", minDist);
        Serial.printf("\nDEBUG_FIND_BLOBS / Found corresponding blob: %p in the **llist_blobs**", (lnode_t*)nearestBlob_ptr);
      #endif
      blobIn_ptr->debounceTimeStamp = millis();
      blobIn_ptr->UID = nearestBlob_ptr->UID;
      blobIn_ptr->state = true;
      blobIn_ptr->lastState = true;
    }
    else {
      // Found a new blob! We nead to give it a UID
      // Find the smallest missing UID in the outputBlobs linked list
      // BUG: when two blobs are arriving in the new frame they get the same ID (FIXED or NOT?)
      while (1) {
        bool isFree = true;
        for (blob_t* blobOut_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blobOut_ptr != NULL; blobOut_ptr = (blob_t*)ITERATOR_NEXT(blobOut_ptr)) {
          if (blobOut_ptr->UID == minID) {
            #if defined(USB_MIDI_SERIAL) && defined(DEBUG_FIND_BLOBS)
              Serial.printf("\nDEBUG_FIND_BLOBS / USED_ID: %d ", minID);
            #endif
            isFree = false;
            minID++;
            break;
          };
        };
        if (isFree) {
          #if defined(USB_MIDI_SERIAL) && defined(DEBUG_FIND_BLOBS)
            Serial.printf("\nDEBUG_FIND_BLOBS / Found new blob and select ID: %d", minID);
          #endif
          blobIn_ptr->UID = minID;
          blobIn_ptr->state = true;
          blobIn_ptr->lastState = false;
          blobIn_ptr->debounceTimeStamp = millis();
          minID++;
          break;
        };
      }; // while_end / The ID of the new blob is set with the smallest missing ID
    };
  };

  // DEAD BLOBS MANAGMENT
  // Look for dead blobs in the outputBlobs linked list
  // If found flag it NOT_FOUND or TO_REMOVE and add it to the temporary input blobs linked list
  while (1) {
    bool allDone = true;
    blob_t* prevBlob_ptr = NULL;
    for (blob_t* blobOut_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blobOut_ptr != NULL; blobOut_ptr = (blob_t*)ITERATOR_NEXT(blobOut_ptr)) {
      bool found = false;
      for (blob_t* blobIn_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs_temp); blobIn_ptr != NULL; blobIn_ptr = (blob_t*)ITERATOR_NEXT(blobIn_ptr)) {
        if (blobIn_ptr->UID == blobOut_ptr->UID) {
          found = true;
          break;
        };
      };
      if (!found) {
        allDone = false;
        if ((millis() - blobOut_ptr->debounceTimeStamp) < DEBOUNCE_TIME) {
          blobOut_ptr->status = NOT_FOUND;
          #if defined(USB_MIDI_SERIAL) && defined(DEBUG_FIND_BLOBS)
            Serial.printf("\nDEBUG_FIND_BLOBS / Blob: %p in the **llist_blobs** linked list is NOT_FOUND(%d)", (lnode_t*)blobOut_ptr, blobOut_ptr->UID);
          #endif
        } 
        else {
          blobOut_ptr->state = false;
          blobOut_ptr->status = TO_REMOVE;
          #if defined(USB_MIDI_SERIAL) && defined(DEBUG_FIND_BLOBS)
            Serial.printf("\nDEBUG_FIND_BLOBS / Blob: %p in the **llist_blobs** linked list taged TO_REMOVE(%d)", (lnode_t*)blobOut_ptr, blobOut_ptr->UID);
          #endif
        };
        llist_extract_node(&llist_blobs, prevBlob_ptr, blobOut_ptr);
        llist_push_front(&llist_blobs_temp, blobOut_ptr);
        break;
      };
      prevBlob_ptr = blobOut_ptr;
    };
    if (allDone) {
      break;
    };
  };

  llist_swap_llist(&llist_blobs, &llist_blobs_temp);       // Swap outputBlobs linked list nodes with inputBlobs linked list nodes
  llist_save_nodes(&llist_blobs_stack, &llist_blobs_temp); // Save/rescure all nodes from the temporay blobs Linked list

  #if defined(RUNING_MEDIAN)
    runing_median();
  #endif

#if defined(VELOCITY)
for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
  if (!blob_ptr->lastState) {
    blob_ptr->velocity.TimeStamp = millis();
    blob_ptr->last_centroid = blob_ptr->centroid;
  }
  else {
    if (millis() - blob_ptr->velocity.TimeStamp > 10) {
      blob_ptr->velocity.timeStamp = millis();
      float vx = fabs(blob_ptr->centroid.x - blob_ptr->last_centroid.x);
      float vy = fabs(blob_ptr->centroid.y - blob_ptr->last_centroid.y);
      blob_ptr->velocity.xy = sqrtf(vx * vx + vy * vy);
      blob_ptr->velocity.z = blob_ptr->centroid.z - blob_ptr->last_centroid.z;
      blob_ptr->last_centroid = blob_ptr->centroid;
    };
  };

};
#endif

#if defined(USB_MIDI_SERIAL) && defined(DEBUG_BITMAP)
  for (uint8_t rowPos = 0; rowPos < NEW_ROWS; rowPos++) {
    uint8_t* rowPos_ptr = &bitmapArray[0] + rowPos * NEW_ROWS;
    for (int colPos = 0; colPos < NEW_COLS; colPos++) {
      Serial.printf("%d-", IMAGE_GET_PIXEL_FAST(rowPos_ptr, colPos));
    };
    Serial.printf("\n");
  };
  Serial.printf("\n");
#endif
#if defined(USB_MIDI_SERIAL) && defined(DEBUG_BLOBS)
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    Serial.printf("\nDEBUG_BLOBS:%d\tLS:%d\tS:%d\tX:%f\tY:%f\tZ:%d\tW:%d\tH:%d\tVXY:%f\tVZ:%f",
                  blob_ptr->UID,
                  blob_ptr->lastState,
                  blob_ptr->state,
                  blob_ptr->centroid.x,
                  blob_ptr->centroid.y,
                  blob_ptr->centroid.z,
                  blob_ptr->box.w,
                  blob_ptr->box.h,
                  blob_ptr->velocity.xy,
                  blob_ptr->velocity.z
                 );
  };
  #endif
  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_FIND_BLOBS)
    if ((lnode_t*)llist_blobs_temp.head_ptr != NULL) {
      Serial.printf("\n___________DEBUG_FIND_BLOBS / END_OF_FRAME");
    }
  #endif
};
