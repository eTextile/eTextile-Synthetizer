/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.

  TODO?: https://github.com/KurtE/DmaSpi/tree/T4
  TODO?: https://github.com/pedvide/ADC/issues/76
*/

#include "scan.h"
#include "blob.h"

#include <SPI.h>                         // https://github.com/PaulStoffregen/SPI
#include <ADC.h>                         // https://github.com/pedvide/ADC

ADC* adc = new ADC();                    // ADC object 
ADC::Sync_result result;                 // Store ADC_0 & ADC_1

#if defined(__MK20DX256__)               // If using Teensy 3.1 & 3.2
#define SS_PIN            10             // Hardware SPI (SELECT : STCP)
#define SCK_PIN           13             // Hardware SPI (CLOCK - SHCP)
#define MOSI_PIN          11             // Hardware SPI (DATA - DS)
#define ADC0_PIN          A9             // Pin 23 is connected to the output of multiplexerA (SIG pin)
#define ADC1_PIN          A3             // Pin 17 is connected to the output of multiplexerB (SIG pin)
#endif

#if defined(__IMXRT1062__)               // If using Teensy 4.0 & 4.1
#define SS1_PIN           0              // Hardware SPI1 (SELECT : STCP)
#define SCK1_PIN          27             // Hardware SPI1 (CLOCK - SHCP)
#define MOSI1_PIN         26             // Hardware SPI1 (DATA - DS)
#define ADC0_PIN          A3             // Pin 17 is connected to the output of multiplexerA (SIG pin) 
#define ADC1_PIN          A2             // Pin 16 is connected to the output of multiplexerB (SIG pin)
#endif

#define DUAL_COLS         (RAW_COLS / 2)

#if defined(__MK20DX256__)               // If using Teensy 3.1 & 3.2
//#define SET_ORIGIN_X                   // X-axis origine positioning
//#define SET_ORIGIN_Y                   // Y-axis origine positioning
#endif

#if defined(__IMXRT1062__)               // If using Teensy 4.0 & 4.1
#define SET_ORIGIN_X                     // X-axis origine positioning
#define SET_ORIGIN_Y                     // Y-axis origine positioning
#endif

#define CALIBRATION_CYCLES  10           // 

uint8_t rawFrameArray[RAW_FRAME] = {0};  // 1D Array to store E256 ofseted analog input values
uint8_t offsetArray[RAW_FRAME] = {0};    // 1D Array to store E256 smallest values

image_t rawFrame;                        // Memory allocation for raw frame values
image_t offsetFrane;                     // Memory allocation for offset frame values

// Array to store all parameters used to configure the two 8:1 analog multiplexeurs
// Each byte |ENA|A|B|C|ENA|A|B|C|
uint8_t setDualCols[DUAL_COLS] = {
  #if defined(SET_ORIGIN_X)
    0x33, 0x00, 0x11, 0x22, 0x44, 0x66, 0x77, 0x55
  #else
    0x55, 0x77, 0x66, 0x44, 0x22, 0x11, 0x00, 0x33
#endif
};

#undef SPI_HAS_TRANSACTION
#define SPI_HAS_TRANSACTION 0
// will look at https://github.com/RobTillaart/FastShiftInOut

inline void setup_spi(void) {
  #if defined(__MK20DX256__)                                              // If using Teensy 3.2
    pinMode(SS_PIN, OUTPUT);                                              // Set the Slave Select Pin as OUTPUT
    SPI.begin();                                                          // Start the SPI module
    SPI.beginTransaction(SPISettings(30000000, MSBFIRST, SPI_MODE0));     // 74HC595BQ Shift out register frequency is 100 MHz = 100000000 Hz
    digitalWrite(SS_PIN, LOW);                                            // Set latchPin LOW
    digitalWrite(SS_PIN, HIGH);
  #endif
  #if defined(__IMXRT1062__)                                              // If using Teensy 4.0 & 4.1
    pinMode(SS1_PIN, OUTPUT);                                             // Set the Slave Select Pin as OUTPUT
    SPI1.begin();                                                         // Start the SPI module
    SPI1.beginTransaction(SPISettings(30000000, MSBFIRST, SPI_MODE0));    // 74HC595BQ Shift out register frequency is 100 MHz = 100000000 Hz
    digitalWrite(SS1_PIN, LOW);                                           // Set latchPin LOW
    digitalWrite(SS1_PIN, HIGH);                                          // Set latchPin HIGH
  #endif
};

inline void setup_adc(void) {
  pinMode(ADC0_PIN, INPUT);                                               // PIN A2 (Teensy 4.0 pin 16)
  pinMode(ADC1_PIN, INPUT);                                               // PIN A3 (Teensy 4.0 pin 17)
  adc->adc0->setAveraging(1);                                             // Set number of averages
  adc->adc0->setResolution(8);                                            // Set bits of resolution
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED);   // Change the conversion speed
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);       // Change the sampling speed
  //adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);      // Change the conversion speed
  //adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);          // Change the sampling speed
  adc->adc1->setAveraging(1);                                             // Set number of averages
  adc->adc1->setResolution(8);                                            // Set bits of resolution
  adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED);   // Change the conversion speed
  adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);       // Change the sampling speed
  //adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);      // Change the conversion speed/*
  //adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);          // Change the sampling speed
};

void scan_setup(void) {

  setup_spi(); 
  setup_adc();

  // image_t* rawFrame init config
  rawFrame.pData = &rawFrameArray[0];
  rawFrame.numCols = RAW_COLS;
  rawFrame.numRows = RAW_ROWS;

  // image_t* offsetFrane init config
  offsetFrane.pData = &offsetArray[0];
  offsetFrane.numCols = RAW_COLS;
  offsetFrane.numRows = RAW_ROWS;
};

// Columns are analog INPUT_PINS reded two by two
// Rows are digital OUTPUT_PINS supplyed one by one sequentially with 3.3V
void matrix_calibrate(void) {

  memset((uint8_t*)offsetArray, 0, RAW_FRAME);              // RAZ offsetArray (16x16)

  for (uint8_t i = 0; i < CALIBRATION_CYCLES; i++) {
    for (uint8_t cols = 0; cols < DUAL_COLS; cols++) {      // ANNALOG_PINS [0-7] with [8-15]
      #if defined(SET_ORIGIN_Y)
        uint16_t setRows = 0x1;                             // Reset to [0000 0000 0000 0001]
      #else
        uint16_t setRows = 0x8000;                            // Reset to [1000 0000 0000 0000]
      #endif
        for (uint8_t row = 0; row < RAW_ROWS; row++) {      // DIGITAL_PINS [0-15]
        #if defined(__MK20DX256__)                          // If using Teensy 3.2
          digitalWrite(SS_PIN, LOW);                        // Set the Slave Select Pin LOW
          //SPI.transfer16(setRows);                        // Set up the two OUTPUT shift registers (FIXME)
          SPI.transfer((uint8_t)(setRows & 0xFF));          // Shift out one byte to setup one OUTPUT shift register
          SPI.transfer((uint8_t)((setRows >> 8) & 0xFF));   // Shift out one byte to setup one OUTPUT shift register
          SPI.transfer(setDualCols[cols]);                  // Shift out one byte that setup the two INPUT 8:1 analog multiplexers
          digitalWrite(SS_PIN, HIGH);                       // Set the Slave Select Pin HIGH
        #endif
        #if defined(__IMXRT1062__)                          // If using Teensy 4.0 & 4.1
          digitalWrite(SS1_PIN, LOW);                       // Set the Slave Select Pin LOW
          //SPI1.transfer16(setRows);                       // Set up the two OUTPUT shift registers (FIXME)
          SPI1.transfer((uint8_t)(setRows & 0xFF));         // Shift out one byte to setup one OUTPUT shift register
          SPI1.transfer((uint8_t)((setRows >> 8) & 0xFF));  // Shift out one byte to setup one OUTPUT shift register
          SPI1.transfer(setDualCols[cols]);                 // Shift out one byte that setup the two INPUT 8:1 analog multiplexers
          digitalWrite(SS1_PIN, HIGH);                      // Set the Slave Select Pin HIGH
        #endif
        uint8_t indexA = row * RAW_COLS + cols;             // Compute 1D array indexA
        uint8_t indexB = indexA + DUAL_COLS;                // Compute 1D array indexB

        //delayMicroseconds(10);
        pinMode(ADC0_PIN, OUTPUT);
        pinMode(ADC1_PIN, OUTPUT);
        digitalWrite(ADC0_PIN, LOW);                        // Set the ADC0 Pin to GND to discharge
        digitalWrite(ADC1_PIN, LOW);                        // Set the ADC0 Pin to GND to discharge
        pinMode(ADC0_PIN, INPUT);
        pinMode(ADC1_PIN, INPUT);

        result = adc->analogSynchronizedRead(ADC0_PIN, ADC1_PIN);
        uint8_t ADC0_val = result.result_adc0;
        //if (ADC0_val > offsetArray[indexA]) offsetArray[indexA] = ADC0_val;
        offsetArray[indexA] = max(offsetArray[indexA], ADC0_val);
        uint8_t ADC1_val = result.result_adc1;
        //if (ADC1_val > offsetArray[indexB]) offsetArray[indexB] = ADC1_val;
        offsetArray[indexB] = max(offsetArray[indexB], ADC1_val);
        #if defined(SET_ORIGIN_Y)
          setRows = setRows << 1;
        #else
          setRows = setRows >> 1;
        #endif
      };
    };
  };
};

// Columns are analog INPUT_PINS reded two by two
// Rows are digital OUTPUT_PINS supplyed one by one sequentially with 3.3V
void matrix_scan(void) {

  for (uint8_t cols = 0; cols < DUAL_COLS; cols++) {      // ANNALOG_PINS [0-7] with [8-15]

#if defined(SET_ORIGIN_Y)
    uint16_t setRows = 0x1;                               // state to [0000 0000 0000 0001]
#else
    uint16_t setRows = 0x8000;                            // Reset to [1000 0000 0000 0000]
#endif

    for (uint8_t row = 0; row < RAW_ROWS; row++) {        // DIGITAL_PINS [0-15]
#if defined(__MK20DX256__)                                // If using Teensy 3.2
      digitalWrite(SS_PIN, LOW);                          // Set the Slave Select Pin LOW
      //SPI.transfer16(setRows);                          // Set up the two OUTPUT shift registers (FIXME)
      SPI.transfer((uint8_t)(setRows & 0xFF));            // Shift out one byte to setup one OUTPUT shift register
      SPI.transfer((uint8_t)((setRows >> 8) & 0xFF));     // Shift out one byte to setup one OUTPUT shift register
      SPI.transfer(setDualCols[cols]);                    // Shift out one byte that setup the two INPUT 8:1 analog multiplexers
      digitalWrite(SS_PIN, HIGH);                         // Set the Slave Select Pin HIGH
#endif
#if defined(__IMXRT1062__)                                // If using Teensy 4.0 & 4.1
      digitalWrite(SS1_PIN, LOW);                         // Set the Slave Select Pin LOW
      //SPI1.transfer16(setRows);                         // Set up the two OUTPUT shift registers (FIXME)
      SPI1.transfer((uint8_t)(setRows & 0xFF));           // Shift out one byte to setup one OUTPUT shift register
      SPI1.transfer((uint8_t)((setRows >> 8) & 0xFF));    // Shift out one byte to setup one OUTPUT shift register
      SPI1.transfer(setDualCols[cols]);                   // Shift out one byte that setup the two INPUT 8:1 analog multiplexers
      digitalWrite(SS1_PIN, HIGH);                        // Set the Slave Select Pin HIGH
#endif
      uint8_t indexA = row * RAW_COLS + cols;             // Compute 1D array indexA
      uint8_t indexB = indexA + DUAL_COLS;                // Compute 1D array indexB

      delayMicroseconds(10);
      pinMode(ADC0_PIN, OUTPUT);
      pinMode(ADC1_PIN, OUTPUT);
      digitalWrite(ADC0_PIN, LOW);                        // Set the ADC0 Pin to GND to discharge
      digitalWrite(ADC1_PIN, LOW);                        // Set the ADC0 Pin to GND to discharge
      pinMode(ADC0_PIN, INPUT);
      pinMode(ADC1_PIN, INPUT);

      result = adc->analogSynchronizedRead(ADC0_PIN, ADC1_PIN);

      uint8_t valA = result.result_adc0;
      valA > offsetArray[indexA] ? rawFrameArray[indexA] = valA - offsetArray[indexA] : rawFrameArray[indexA] = 0;
      rawFrameArray[indexA] = min(valA, 127); // Add limit for MIDI message 0:127

      uint8_t valB = result.result_adc1;
      valB > offsetArray[indexB] ? rawFrameArray[indexB] = valB - offsetArray[indexB] : rawFrameArray[indexB] = 0;
      rawFrameArray[indexB] = min(valB, 127); // Add limit for MIDI message 0:127

#if defined(SET_ORIGIN_Y)
      setRows = setRows << 1;
#else
      setRows = setRows >> 1;
#endif
    };
  };

#if defined(USB_MIDI_SERIAL) && defined(DEBUG_ADC)
  for (uint8_t posY = 0; posY < RAW_ROWS; posY++) {
    uint8_t* row_ptr = COMPUTE_IMAGE_ROW_PTR(&rawFrame, posY);
    for (uint8_t posX = 0; posX < RAW_COLS; posX++) {
      Serial.printf("\t%d", IMAGE_GET_PIXEL_FAST(row_ptr, posX));
    };
    Serial.printf("\n");
  };
  Serial.printf("\n");
#endif
};
