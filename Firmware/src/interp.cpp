/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// Bilinear interpolation
// TODO: https://github.com/Pharap/FixedPointsArduino

#include "interp.h"
#include "config.h"
#include "scan.h"

#define IRAW_ROWS       (RAW_ROWS - 1)
#define IRAW_COLS       (RAW_COLS - 1)
#define INTERP_STRIDE   (SCALE_X * SCALE_Y * RAW_COLS)

uint8_t interp_threshold = 5;               //
uint8_t interp_frame_array[NEW_FRAME] = {0}; // 1D Array to store E256 bilinear interpolated values

float coef_A[SCALE_X * SCALE_Y] = {0};
float coef_B[SCALE_X * SCALE_Y] = {0};
float coef_C[SCALE_X * SCALE_Y] = {0};
float coef_D[SCALE_X * SCALE_Y] = {0};

image_t interp_frame; // Interpolated frame values

// Pre-compute the four coefficient values for all interpolated output matrix positions
void interp_setup(void) {
  interp_frame.data_ptr = &interp_frame_array[0];
  interp_frame.num_cols = NEW_COLS;
  interp_frame.num_rows = NEW_ROWS;
  float scale_factor = SCALE_X * SCALE_Y;
  for (uint8_t row = 0; row < SCALE_Y; row++) {
    for (uint8_t col = 0; col < SCALE_X; col++) {
      int index = row * SCALE_Y + col;
      coef_A[index] = (SCALE_X - col) * (SCALE_Y - row) / scale_factor;
      coef_B[index] = col * (SCALE_Y - row) / scale_factor;
      coef_C[index] = (SCALE_X - col) * row / scale_factor;
      coef_D[index] = row * col / scale_factor;
    };
  };
};

inline void update_interpThreshold(level_t* levels_ptr){
  if (levels_ptr->leds.update){
    levels_ptr->leds.update = false;
    interp_threshold = constrain(levels_ptr->val - 4, 0, levels_ptr->max_val);
  };
};

// Bilinear interpolation
void matrix_interp(void) {
  update_interpThreshold(&e256_ctr.levels[THRESHOLD]);
  memset((uint8_t*)interp_frame_array, 0, SIZEOF_FRAME); // Clear interp_frame_array
  for (uint8_t row_pos = 0; row_pos < IRAW_ROWS; row_pos++) {
    uint16_t index_a = row_pos * INTERP_STRIDE;
    uint8_t* raw_row_A_ptr = COMPUTE_IMAGE_ROW_PTR(&raw_frame, row_pos);
    uint8_t* raw_row_B_ptr = raw_row_A_ptr + RAW_COLS;
    for (uint8_t col_pos = 0; col_pos < IRAW_COLS; col_pos++) {
      uint16_t index_b = col_pos * SCALE_X;
      if (IMAGE_GET_PIXEL_FAST(raw_row_A_ptr, col_pos) > interp_threshold) { // 'Windowing' interpolation
        uint8_t raw_pixel_a = IMAGE_GET_PIXEL_FAST(raw_row_A_ptr, col_pos);
        uint8_t raw_pixel_b = IMAGE_GET_PIXEL_FAST(raw_row_A_ptr, col_pos + 1);
        uint8_t raw_pixel_c = IMAGE_GET_PIXEL_FAST(raw_row_B_ptr, col_pos);
        uint8_t raw_pixel_d = IMAGE_GET_PIXEL_FAST(raw_row_B_ptr, col_pos + 1);
        for (uint8_t row = 0; row < SCALE_Y; row++) {
          uint16_t index_c = row * NEW_COLS;
          uint8_t row_index = row * SCALE_X;
          float* row_ptr_a = &coef_A[0] + row_index;
          float* row_ptr_b = &coef_B[0] + row_index;
          float* row_ptr_c = &coef_C[0] + row_index;
          float* row_ptr_d = &coef_D[0] + row_index;
          for (uint8_t col = 0; col < SCALE_X; col++) {
            uint16_t index = index_a + index_b + index_c + col;
            interp_frame_array[index] = (uint8_t)lround(
              raw_pixel_a * IMAGE_GET_PIXEL_FAST(row_ptr_a, col) +
              raw_pixel_b * IMAGE_GET_PIXEL_FAST(row_ptr_b, col) +
              raw_pixel_c * IMAGE_GET_PIXEL_FAST(row_ptr_c, col) +
              raw_pixel_d * IMAGE_GET_PIXEL_FAST(row_ptr_d, col)
            );
          };
        };
      };
    };
  };
#if defined(USB_MIDI_SERIAL) && defined(DEBUG_INTERP)
  for (uint8_t row_pos = 0; row_pos < NEW_ROWS; row_pos++) {
    uint8_t* rowPos_ptr = &interp_frame_array[0] + row_pos * NEW_ROWS;
    for (int col_pos = 0; col_pos < NEW_COLS; col_pos++) {
      Serial.printf("%d-", IMAGE_GET_PIXEL_FAST(rowPos_ptr, col_pos));
    };
    Serial.printf("\n");
  };
  Serial.printf("\n\n");
#endif
};
