/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>                 // https://github.com/PaulStoffregen/Encoder

#define PROJECT                      "ETEXTILE-SYNTHESIZER"
#define NAME                         "256"
//#define VERSION                      "1.0.12"
#define SENSOR_UID                   1 // Unique sensor ID

// E256 HARDWARE CONSTANTS
#define LED_PIN_D1                   5
#define LED_PIN_D2                   4
#define BUTTON_PIN_L                 2
#define BUTTON_PIN_R                 3
#define ENCODER_PIN_A                22
#define ENCODER_PIN_B                9
#define FLASH_CHIP_SELECT            6
#define FLASH_SIZE                   4096
#define BAUD_RATE                    230400
#define RAW_COLS                     16
#define RAW_ROWS                     16
#define RAW_FRAME                    (RAW_COLS * RAW_ROWS)

// E256 SOFTWARE CONSTANTS
#define SCALE_X                      4
#define SCALE_Y                      4
#define NEW_COLS                     (RAW_COLS * SCALE_X)
#define NEW_ROWS                     (RAW_ROWS * SCALE_Y)
#define NEW_FRAME                    (NEW_COLS * NEW_ROWS)
#define SIZEOF_FRAME                 (NEW_FRAME * sizeof(uint8_t))
#define BLOB_MIN_PIX                 6     // Set the minimum blob pixels
#define BLOB_MAX_PIX                 1024  // Set the minimum blob pixels
#define X_MIN                        1     // Blob centroid X min value
#define X_MAX                        58    // Blob centroid X max value
#define Y_MIN                        1     // Blob centroid Y min value
#define Y_MAX                        58    // Blob centroid Y max value
#define Z_MIN                        0     // Blob centroid Z min value
#define Z_MAX                        256   // Blob centroid Z max value
#define WIDTH                        (X_MAX - X_MIN)
#define HEIGHT                       (Y_MAX - Y_MIN)
#define IIPi                         (float)(2 * PI)
#define IIIPiII                      (float)(3 * PI) / 2
#define PiII                         (float)(PI / 2)
#define LONG_HOLD                    1500
#define BLINK_ITER                   10
#define MIDI_TRANSMIT_INTERVAL       50    // 20Hz
#define LEVEL_TIMEOUT                3000
#define PENDING_MODE_TIMEOUT         4000

// E256 MIDI I/O CHANNELS CONSTANTS [1:15]
#define MIDI_INPUT_CHANNEL           1
#define MIDI_OUTPUT_CHANNEL          2
#define MIDI_MODES_CHANNEL           3
#define MIDI_STATES_CHANNEL          4
#define MIDI_LEVELS_CHANNEL          5
#define MIDI_VERBOSITY_CHANNEL       6
#define MIDI_ERROR_CHANNEL           7

// E256 MODES CONSTANTS (MIDI_MODES_CHANNEL)
#define PENDING_MODE                 0 // Waiting for mode
#define SYNC_MODE                    1 // Hand chake mode
#define STANDALONE_MODE              2 // Send mappings values over MIDI hardware
#define MATRIX_MODE_RAW              3 // Send matrix analog sensor values (16x16) over USB using MIDI format
#define MATRIX_MODE_INTERP           4 // Send matrix analog sensor values (16x16) over USB using MIDI format
#define EDIT_MODE                    5 // Send all blobs values over USB_MIDI
#define PLAY_MODE                    6 // Send mappings values over USB_MIDI
#define ERROR_MODE                   7 // 

// E256 STATES CONSTANTS (MIDI_STATES_CHANNEL)
#define CALIBRATE_REQUEST            0
#define CONFIG_FILE_REQUEST          1                 

// E256 LEVELS CONSTANTS (MIDI_LEVELS_CHANNEL)
#define THRESHOLD                    0 // E256-LEDs: | 1 | 1 |
#define SIG_IN                       1 // E256-LEDs: | 1 | 0 |
#define SIG_OUT                      2 // E256-LEDs: | 0 | 1 |
#define LINE_OUT                     3 // E256-LEDs: | 0 | 0 |

// E256 MAPPING_LIB CONSTANTS
#define MAX_BLOBS                    16    // [0:7] How many blobs can be tracked at the same time
#define MAX_TRIGGERS                 16
#define MAX_SWITCHS                  16
#define MAX_SLIDERS                  8
#define MAX_CSLIDERS                 2
#define MAX_KNOBS                    9
#define MAX_TOUCHPADS                2
#define MAX_TOUCH_POINTS             10
#define MAX_POLYGON_POINTS           64
#define MAX_POLYGONS                 8
#define MAX_GRIDS                    2
#define MAX_GRID_KEYS                128
#define H_SLIDER                     0
#define V_SLIDER                     1

// E256 MIDI CONSTANTS
#define SYSEX_CONF                   0x7C // DEC: 124
#define SYSEX_SOUND                  0x6C // DEC: 108

// VERBOSITY CONSTANTS
#define PENDING_MODE_DONE            0
#define SYNC_MODE_DONE               1
#define MATRIX_MODE_RAW_DONE         2
#define MATRIX_MODE_INTERP_DONE      3
#define EDIT_MODE_DONE               4
#define PLAY_MODE_DONE               5
#define FLASH_CONFIG_ALLOC_DONE      6
#define FLASH_CONFIG_LOAD_DONE       7
#define FLASH_CONFIG_WRITE_DONE      8
#define USBMIDI_CONFIG_ALLOC_DONE    9
#define USBMIDI_CONFIG_LOAD_DONE     10
#define CONFIG_APPLY_DONE            11
#define USBMIDI_SOUND_LOAD_DONE      12
#define USBMIDI_SET_LEVEL_DONE       13
#define CALIBRATE_DONE               14
#define DONE_ACTION                  15 

// ERROR CODES CONSTANTS
#define WAITING_FOR_CONFIG             0
#define CONNECTING_FLASH               1
#define FLASH_FULL                     2
#define FILE_TO_BIG                    3
#define NO_CONFIG_FILE                 4
#define WHILE_OPEN_FLASH_FILE          5
#define USBMIDI_CONFIG_LOAD_FAILED     6
#define FLASH_CONFIG_LOAD_FAILED       7
#define FLASH_CONFIG_WRITE_FAILED      8
#define CONFIG_APPLY_FAILED            9
#define UNKNOWN_SYSEX                  10
#define TOO_MANY_BLOBS                 11

/*
#define M_CHAN                         "c"
#define M_VELO                         "v"
#define M_NOTE                         "n"
#define M_CC                           "cc"
#define M_VAL                          "v"
#define M_STATE                        "s"
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
  Encoder* encoder;
  e256_mode_t* modes;
  e256_state_t* states;
  e256_level_t* levels;
};

extern e256_control_t e256_ctr;
extern uint8_t e256_currentMode;
extern uint8_t e256_level;

extern uint8_t* flash_config_ptr;
extern uint16_t flash_configSize;

void set_mode(uint8_t mode);
void set_state(uint8_t state);
void set_level(uint8_t level, uint8_t value);

void hardware_setup(void);
void update_controls(void);
bool load_flash_config(void);
bool apply_config(uint8_t* conf_ptr, uint16_t conf_size);

#endif /*__CONFIG_H__*/
