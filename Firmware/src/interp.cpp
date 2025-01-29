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

uint8_t interpThreshold = 5;               //
uint8_t interpFrameArray[NEW_FRAME] = {0}; // 1D Array to store E256 bilinear interpolated values

float coef_A[SCALE_X * SCALE_Y] = {0};
float coef_B[SCALE_X * SCALE_Y] = {0};
float coef_C[SCALE_X * SCALE_Y] = {0};
float coef_D[SCALE_X * SCALE_Y] = {0};

image_t interpFrame; // Interpolated frame values

// Pre-compute the four coefficient values for all interpolated output matrix positions
void interp_setup(void) {
  interpFrame.pData = &interpFrameArray[0];
  interpFrame.numCols = NEW_COLS;
  interpFrame.numRows = NEW_ROWS;
  float sFactor = SCALE_X * SCALE_Y;
  for (uint8_t row = 0; row < SCALE_Y; row++) {
    for (uint8_t col = 0; col < SCALE_X; col++) {
      int index = row * SCALE_Y + col;
      coef_A[index] = (SCALE_X - col) * (SCALE_Y - row) / sFactor;
      coef_B[index] = col * (SCALE_Y - row) / sFactor;
      coef_C[index] = (SCALE_X - col) * row / sFactor;
      coef_D[index] = row * col / sFactor;
    };
  };
};

inline void update_interpThreshold(e256_level_t* levels_ptr){
  if (levels_ptr->leds.update){
    levels_ptr->leds.update = false;
    interpThreshold = constrain(levels_ptr->val - 4, 0, levels_ptr->maxVal);
  };
};

// Bilinear interpolation
void matrix_interp(void) {
  update_interpThreshold(&e256_ctr.levels[THRESHOLD]);
  memset((uint8_t*)interpFrameArray, 0, SIZEOF_FRAME); // Clear interpFrameArray
  for (uint8_t rowPos = 0; rowPos < IRAW_ROWS; rowPos++) {
    uint16_t indexA = rowPos * INTERP_STRIDE;
    uint8_t* rawRowA_ptr = COMPUTE_IMAGE_ROW_PTR(&rawFrame, rowPos);
    uint8_t* rawRowB_ptr = rawRowA_ptr + RAW_COLS;
    for (uint8_t colPos = 0; colPos < IRAW_COLS; colPos++) {
      uint16_t indexB = colPos * SCALE_X;
      if (IMAGE_GET_PIXEL_FAST(rawRowA_ptr, colPos) > interpThreshold) { // 'Windowing' interpolation
        uint8_t rawPixA = IMAGE_GET_PIXEL_FAST(rawRowA_ptr, colPos);
        uint8_t rawPixB = IMAGE_GET_PIXEL_FAST(rawRowA_ptr, colPos + 1);
        uint8_t rawPixC = IMAGE_GET_PIXEL_FAST(rawRowB_ptr, colPos);
        uint8_t rawPixD = IMAGE_GET_PIXEL_FAST(rawRowB_ptr, colPos + 1);
        for (uint8_t row = 0; row < SCALE_Y; row++) {
          uint16_t indexC = row * NEW_COLS;
          uint8_t rowIndex = row * SCALE_X;
          float* row_ptr_A = &coef_A[0] + rowIndex;
          float* row_ptr_B = &coef_B[0] + rowIndex;
          float* row_ptr_C = &coef_C[0] + rowIndex;
          float* row_ptr_D = &coef_D[0] + rowIndex;
          for (uint8_t col = 0; col < SCALE_X; col++) {
            uint16_t index = indexA + indexB + indexC + col;
            interpFrameArray[index] = (uint8_t)lround(
              rawPixA * IMAGE_GET_PIXEL_FAST(row_ptr_A, col) +
              rawPixB * IMAGE_GET_PIXEL_FAST(row_ptr_B, col) +
              rawPixC * IMAGE_GET_PIXEL_FAST(row_ptr_C, col) +
              rawPixD * IMAGE_GET_PIXEL_FAST(row_ptr_D, col)
            );
          };
        };
      };
    };
  };
#if defined(USB_MIDI_SERIAL) && defined(DEBUG_INTERP)
  for (uint8_t rowPos = 0; rowPos < NEW_ROWS; rowPos++) {
    uint8_t* rowPos_ptr = &interpFrameArray[0] + rowPos * NEW_ROWS;
    for (int colPos = 0; colPos < NEW_COLS; colPos++) {
      Serial.printf("%d-", IMAGE_GET_PIXEL_FAST(rowPos_ptr, colPos));
    };
    Serial.printf("\n");
  };
  Serial.printf("\n\n");
#endif
};
