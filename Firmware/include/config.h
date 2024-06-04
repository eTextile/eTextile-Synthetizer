/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h> // https://github.com/PaulStoffregen/Encoder

#define PROJECT "ETEXTILE-SYNTHESIZER"
#define NAME "256"
// #define VERSION "1.0.12"
#define SENSOR_UID 1 // Unique sensor ID
#define FLASH_BUFFER_SIZE 4096
// E256 HARDWARE CONSTANTS
#define LED_PIN_D1 5
#define LED_PIN_D2 4
#define BUTTON_PIN_L 2
#define BUTTON_PIN_R 3
#define ENCODER_PIN_A 22
#define ENCODER_PIN_B 9
#define FLASH_CHIP_SELECT 6
#define FLASH_SIZE 4096
#define BAUD_RATE 230400
#define RAW_COLS 16
#define RAW_ROWS 16
#define RAW_FRAME (RAW_COLS * RAW_ROWS)

// E256 SOFTWARE CONSTANTS
#define SCALE_X 4
#define SCALE_Y 4
#define NEW_COLS (RAW_COLS * SCALE_X)
#define NEW_ROWS (RAW_ROWS * SCALE_Y)
#define NEW_FRAME (NEW_COLS * NEW_ROWS)
#define SIZEOF_FRAME (NEW_FRAME * sizeof(uint8_t))
#define BLOB_MIN_PIX 6    // Set the minimum blob pixels
#define BLOB_MAX_PIX 1024 // Set the minimum blob pixels
#define X_MIN 1           // Blob centroid X min value
#define X_MAX 58          // Blob centroid X max value
#define Y_MIN 1           // Blob centroid Y min value
#define Y_MAX 58          // Blob centroid Y max value
#define Z_MIN 0           // Blob centroid Z min value
#define Z_MAX 256         // Blob centroid Z max value
#define WIDTH (X_MAX - X_MIN)
#define HEIGHT (Y_MAX - Y_MIN)
#define IIPi (float)(2 * PI)
#define IIIPiII (float)(3 * PI) / 2
#define PiII (float)(PI / 2)
#define LONG_HOLD 1500
#define LEVEL_TIMEOUT 3000
#define PENDING_MODE_TIMEOUT 5000

// E256 MIDI I/O CHANNELS CONSTANTS [1:15]
#define MIDI_INPUT_CHANNEL 1
//#define MIDI_OUTPUT_CHANNEL 2

#define MIDI_MODES_CHANNEL 3
#define MIDI_STATES_CHANNEL 4
#define MIDI_LEVELS_CHANNEL 5
#define MIDI_VERBOSITY_CHANNEL 6
#define MIDI_ERROR_CHANNEL 7

typedef enum {
  PENDING_MODE,    // Waiting for mode
  SYNC_MODE,       // Hand chake mode
  STANDALONE_MODE, // Send mappings values over MIDI hardware
  MATRIX_MODE_RAW, // Send matrix analog sensor values (16x16) over USB using MIDI format
  EDIT_MODE,       // Send all blobs values over USB_MIDI
  PLAY_MODE,       // Recive mappings values from USB_MIDI and forward them to USB_HARDWARE
  FETCH_MODE,      // Send mapping config file
  ERROR_MODE       // Unexpected behaviour
} mapping_mode_t;

// E256 STATES CONSTANTS (MIDI_STATES_CHANNEL)
#define CALIBRATE_REQUEST 0
#define CONFIG_FILE_REQUEST 1

// E256 LEVELS CONSTANTS (MIDI_LEVELS_CHANNEL)
#define THRESHOLD 0 // E256-LEDs: | 1 | 1 |
#define SIG_IN 1    // E256-LEDs: | 1 | 0 |
#define SIG_OUT 2   // E256-LEDs: | 0 | 1 |
#define LINE_OUT 3  // E256-LEDs: | 0 | 0 |

// E256 MAPPING_LIB CONSTANTS
#define MAX_BLOBS 16 // [0:7] How many blobs can be tracked at the same time

// #define MAX_TRIGGERS 16
#define MAX_SWITCHS 16

#define MAX_SLIDERS 6
#define MAX_SLIDER_TOUCHS 2

#define MAX_KNOBS 4
#define MAX_KNOB_TOUCHS 4

#define MAX_TOUCHPADS 2
#define MAX_TOUCHPAD_TOUCHS 5

#define MAX_POLYGONS 8
#define MAX_POLYGON_POINTS 64

#define MAX_GRIDS 2
#define MAX_GRID_KEYS 256

#define MAX_CSLIDERS 2

typedef enum {
  PENDING_MODE_DONE,
  SYNC_MODE_DONE,
  MATRIX_MODE_RAW_DONE,
  EDIT_MODE_DONE,
  PLAY_MODE_DONE,
  FLASH_CONFIG_ALLOC_DONE,
  FLASH_CONFIG_LOAD_DONE,
  FLASH_CONFIG_WRITE_DONE,
  USBMIDI_CONFIG_ALLOC_DONE,
  USBMIDI_CONFIG_LOAD_DONE,
  CONFIG_APPLY_DONE,
  USBMIDI_SOUND_LOAD_DONE,
  USBMIDI_SET_LEVEL_DONE,
  CALIBRATE_DONE,
  DONE_ACTION
} e256_verbosity_t;

typedef enum {
  WAITING_FOR_CONFIG,
  CONNECTING_FLASH,
  FLASH_FULL,
  FILE_TO_BIG,
  NO_CONFIG_FILE,
  WHILE_OPEN_FLASH_FILE,
  USBMIDI_CONFIG_LOAD_FAILED,
  FLASH_CONFIG_LOAD_FAILED,
  FLASH_CONFIG_WRITE_FAILED,
  CONFIG_APPLY_FAILED,
  UNKNOWN_SYSEX,
  TOO_MANY_BLOBS
} e256_errors_t;

#define MIDI_TRANSMIT_INTERVAL 50 // 20Hz

/*
#define M_CHAN                       "c"
#define M_VELO                       "v"
#define M_NOTE                       "n"
#define M_CC                         "cc"
#define M_VAL                        "v"
#define M_STATE                      "s"
*/

typedef struct leds leds_t;
struct leds {
  bool D1;
  bool D2;
  bool update;
};

typedef struct e256_mode e256_mode_t;
struct e256_mode {
  leds_t leds;
  uint16_t timeOn;
  uint16_t timeOff;
  bool toggle;
};

typedef struct e256_state e256_state_t;
struct e256_state {
  leds_t leds;
  uint16_t timeOn;
  uint16_t timeOff;
  uint8_t iter;
};

typedef struct e256_level e256_level_t;
struct e256_level {
  leds_t leds;
  uint8_t minVal;
  uint8_t maxVal;
  uint8_t val;
  bool update;
};

typedef struct e256_control e256_control_t;
struct e256_control {
  Encoder *encoder;
  e256_mode_t *modes;
  e256_state_t *states;
  e256_level_t *levels;
};

extern e256_control_t e256_ctr;
extern mapping_mode_t e256_currentMode;
extern uint8_t e256_level;

extern uint8_t *flash_config_ptr;
extern uint32_t flash_configSize;

void set_mode(uint8_t mode);
void set_state(uint8_t state);
void set_level(uint8_t level, uint8_t value);

void hardware_setup(void);
void update_controls(void);
bool load_flash_config(void);
bool apply_config(uint8_t *conf_ptr, uint16_t conf_size);

#endif /*__CONFIG_H__*/
