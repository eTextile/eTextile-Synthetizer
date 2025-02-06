/*
  FORKED FROM https://github.com/openmv/openmv/blob/master/src/omv/imlib/blob.c
    - This file is part of the OpenMV project.
    - Copyright (c) 2013- Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
    - This work is licensed under the MIT license, see the file LICENSE for details.

  Added custom blob détection algorithm to keep track of the blobs ID's
    - This patch is part of the eTextile-Synthesizer project - https://synth.eTextile.org
    - Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
    - This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "interp.h"
#include "blob.h"
#include "median.h"
#include "mapping.h"

#define LIFO_NODES          512        // Set the maximum nodes number
#define X_STRIDE            4          // Speed up X scanning
#define Y_STRIDE            2          // Speed up Y scanning
#define DEBOUNCE_TIME       50         // Avioding undesired bouncing effect when tapping or sliding on the sensor

uint8_t bitmap_array[NEW_FRAME] = {0}; // Store (64*64) binary values
xylr_t lifo_array[LIFO_NODES] = {0};   // Store lifo nodes
blob_t blob_array[MAX_BLOBS] = {0};    // Store blobs

llist_t llist_context_pool;            // Free nodes pool
llist_t llist_context;                 // Used nodes
llist_t llist_blobs_pool;              // Free nodes pool

llist_t llist_blobs;                   // Intermediate blobs linked list
//llist_t llist_previous_blobs;        // Output blobs linked list

void blob_setup(void) {
  llist_builder(&llist_context_pool, &lifo_array[0], LIFO_NODES, sizeof(lifo_array[0])); // Add X nodes to the llist_context_pool
  llist_builder(&llist_blobs_pool, &blob_array[0], MAX_BLOBS, sizeof(blob_array[0])); // Add X nodes to the llist_blobs_pool
  llist_raz(&llist_context);
  llist_raz(&llist_blobs);
  //llist_raz(&llist_previous_blobs);

  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_blobs_pool); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(node_ptr);
    blob_ptr->UID = 255;
    blob_ptr->last_status = MISSING;
  };
};

inline uint8_t set_id(void) {
  static uint8_t UID = 0;
  return UID++;
};

/////////////////////////////// Scanline flood fill algorithm / SFF
/////////////////////////////// Connected-component labeling / CCL
void matrix_find_blobs(void) {

  // DEAD BLOBS REMOVER
  llist_t blobs_to_keep;
  blob_t* is_dead_blob_ptr = NULL;

  while ((is_dead_blob_ptr = (blob_t*)llist_pop_front(&llist_blobs)) != NULL) {
    if ((millis() - is_dead_blob_ptr->life_time_stamp) > TIME_TO_LEAVE) {
      llist_push_front(&llist_blobs_pool, is_dead_blob_ptr);
      #if defined(USB_MIDI_SERIAL) && defined(DEBUG_FIND_BLOBS)
        Serial.printf("\nDEBUG_FIND_BLOBS / Blob: %p removed", &is_dead_blob_ptr);
      #endif
    }
    else {
      is_dead_blob_ptr->status = MISSING;
      llist_push_front(&blobs_to_keep, is_dead_blob_ptr);
    };
  };
  llist_swap_llist(&llist_blobs, &blobs_to_keep);

  memset((uint8_t*)bitmap_array, 0, SIZEOF_FRAME);
  uint8_t blob_count = 0;
  
  for (uint8_t posY = 0; posY < NEW_ROWS; posY += Y_STRIDE) {

    uint8_t* row_ptr_a = COMPUTE_IMAGE_ROW_PTR(&interp_frame, posY);
    uint8_t* bmp_row_ptr_A = &bitmap_array[0] + posY * NEW_COLS;

    for (uint8_t posX = (posY % X_STRIDE); posX < NEW_COLS; posX += X_STRIDE) {
      if (!IMAGE_GET_PIXEL_FAST(bmp_row_ptr_A, posX) &&
          PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_a, posX), e256_ctr.levels[THRESHOLD].val)
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

          uint8_t* row_ptr_b = COMPUTE_IMAGE_ROW_PTR(&interp_frame, posY);
          uint8_t* bmp_row_ptr_b = &bitmap_array[0] + posY * NEW_COLS;

          while ((left > 0) &&
                 !IMAGE_GET_PIXEL_FAST(bmp_row_ptr_b, left - 1) &&
                 PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_b, left - 1), e256_ctr.levels[THRESHOLD].val)
                ) {
            left--;
          };
          while (right < (NEW_COLS - 1) &&
                 !IMAGE_GET_PIXEL_FAST(bmp_row_ptr_b, right + 1) &&
                 PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_b, right + 1), e256_ctr.levels[THRESHOLD].val)
                ) {
            right++;
          };

          blob_x1 = MIN(blob_x1, left);
          blob_x2 = MAX(blob_x2, right);

          for (int i = left; i <= right; i++) {
            IMAGE_SET_PIXEL_FAST(bmp_row_ptr_b, i, 1);
            blob_depth = MAX(blob_depth, IMAGE_GET_PIXEL_FAST(row_ptr_b, i));
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
              row_ptr_b = COMPUTE_IMAGE_ROW_PTR(&interp_frame, posY - 1);
              bmp_row_ptr_b = &bitmap_array[0] + (posY - 1) * NEW_COLS;

              bool recurse = true;
              for (uint8_t i = top_left; i <= right; i++) {

                if ((!IMAGE_GET_PIXEL_FAST(bmp_row_ptr_b, i))
                    && (PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_b, i), e256_ctr.levels[THRESHOLD].val))) {

                  xylr_t* context = (xylr_t*)llist_pop_front(&llist_context_pool);
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

            row_ptr_b = COMPUTE_IMAGE_ROW_PTR(&interp_frame, posY + 1);
            bmp_row_ptr_b = &bitmap_array[0] + (posY + 1) * NEW_COLS;

            bool recurse = true;
            for (uint8_t i = bot_left; i <= right; i++) {

              if (!IMAGE_GET_PIXEL_FAST(bmp_row_ptr_b, i) &&
                  PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_b, i), e256_ctr.levels[THRESHOLD].val)) {

                xylr_t* context = (xylr_t*)llist_pop_front(&llist_context_pool);
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
            llist_push_front(&llist_context_pool, context);
            blob_height++;
          }; // END while_B

          if (break_out) {
            break;
          };
        }; // END while_A

        if (blob_pixels > BLOB_MIN_PIX && blob_pixels < BLOB_MAX_PIX && blob_count < MAX_BLOBS) {
          blob_count++;
          blob_t* new_blob_ptr = (blob_t*)llist_pop_front(&llist_blobs_pool);
          new_blob_ptr->last_centroid = new_blob_ptr->centroid;
          new_blob_ptr->centroid.x = constrain(blob_cx / blob_pixels, X_MIN, X_MAX) - X_MIN ;
          new_blob_ptr->centroid.y = constrain(blob_cy / blob_pixels, Y_MIN, Y_MAX) - Y_MIN;
          // Subtract threshold and avoid negative numbers
          blob_depth > e256_ctr.levels[THRESHOLD].val ? new_blob_ptr->centroid.z = (blob_depth - e256_ctr.levels[THRESHOLD].val) : new_blob_ptr->centroid.z = 0;
          new_blob_ptr->box.w = (blob_x2 - blob_x1);
          new_blob_ptr->box.h = blob_height;

          blob_t* existing_blob_ptr = (blob_t*)llist_find(&llist_blobs, new_blob_ptr, (llist_compare_func_t*)&is_blob_existing);
          if (existing_blob_ptr != NULL) {
            existing_blob_ptr->last_status = existing_blob_ptr->status;
            existing_blob_ptr->status = PRESENT;
            existing_blob_ptr->life_time_stamp = millis();
            llist_push_back(&llist_blobs_pool, new_blob_ptr);
          } else {
            new_blob_ptr->status = NEW;
            new_blob_ptr->UID = set_id();
            new_blob_ptr->life_time_stamp = millis();
            llist_push_back(&llist_blobs, new_blob_ptr);
          }
        };
        posX = oldX;
        posY = oldY;
      };
    };
  };

  #if defined(RUNING_MEDIAN)
    runing_median();
  #endif

#if defined(VELOCITY)
for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_previous_blobs); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
  blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(node_ptr);
  if (blob_ptr->status == NEW) {
    blob_ptr->velocity.time_stamp = millis();
    blob_ptr->last_centroid = blob_ptr->centroid;
  }
  else if (blob_ptr->status == PRESENT) {
    if (millis() - blob_ptr->velocity.time_stamp > 5) {
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
  for (uint8_t row_pos = 0; row_pos < NEW_ROWS; row_pos++) {
    uint8_t* rowPos_ptr = &bitmap_array[0] + row_pos * NEW_ROWS;
    for (int col_pos = 0; col_pos < NEW_COLS; col_pos++) {
      Serial.printf("%d-", IMAGE_GET_PIXEL_FAST(rowPos_ptr, col_pos));
    };
    Serial.printf("\n");
  };
  Serial.printf("\n");
#endif
#if defined(USB_MIDI_SERIAL) && defined(DEBUG_BLOBS)
  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_previous_blobs); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(node_ptr);
    Serial.printf("\nDEBUG_BLOBS:%d\tS:%d\tX:%f\tY:%f\tZ:%d\tW:%d\tH:%d\tVXY:%f\tVZ:%f",
                  blob_ptr->UID,
                  blob_ptr->status,
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
    if ((lnode_t*)llist_blobs.head_ptr != NULL) {
      Serial.printf("\n___________DEBUG_FIND_BLOBS / END_OF_FRAME");
    }
  #endif
};

bool is_blob_existing(blob_t* blob_ptr, blob_t* new_blob_ptr) {
  if (blob_ptr->action.mapping_ptr != NULL) {
    float dist = sqrtf(pow(blob_ptr->centroid.x - new_blob_ptr->centroid.x, 2) + pow(blob_ptr->centroid.y - new_blob_ptr->centroid.y, 2));
    if (dist < 10) { // SET IT AS GLOBAL !!
      common_t* mapping_ptr = (common_t*)blob_ptr->action.mapping_ptr;
      mapping_ptr->interact_func_ptr(new_blob_ptr, mapping_ptr);
      return true;
    };
  }
  return false;
};

void blob_sort(llist_t* llist_ptr) {
  lnode_t* next_node_ptr = (lnode_t*)llist_ptr->head_ptr->next_ptr;

  uint16_t llist_size = 0;
  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_ptr); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    llist_size++;
  };

  blob_t* tmp_blob_ptr;
  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_ptr); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(node_ptr);
    for (int j=0; j<llist_size; j++) {
      blob_t* next_blob_ptr = (blob_t*)ITERATOR_DATA(next_blob_ptr);
      if (blob_ptr->UID > next_blob_ptr->UID) {
        tmp_blob_ptr = blob_ptr;
        blob_ptr = next_blob_ptr;
        next_node_ptr->data_ptr = tmp_blob_ptr;
      }
      next_node_ptr = ITERATOR_NEXT(next_node_ptr);
    };
  };
};