/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include <ArduinoJson.h>
#include <SerialFlash.h>
#include <Bounce2.h>

#include "config.h"
#include "scan.h"
#include "interp.h"
#include "midi_bus.h"
#include "usb_midi_io.h"
#include "allocate.h"

#include "mapping.h"
#include "mapp_switch.h"
#include "mapp_slider.h"
#include "mapp_touchpad.h"
#include "mapp_knob.h"
#include "mapp_grid.h"
#include "mapp_polygon.h"

typedef union {
  switch_t _switch;
  slider_t _slider;
  touchpad_t _touchpad;
  knob_t knob;
  grid_t grid;
  polygon_t polygon;
}mapping_u;

Bounce BUTTON_L = Bounce();
Bounce BUTTON_R = Bounce();

const char* get_mode_name(mode_codes_t code) {
  const char* char_code = NULL;
  switch (code) {
    case PENDING_MODE: char_code = "PENDING_MODE"; break;
    case SYNC_MODE: char_code = "SYNC_MODE"; break;
    case CALIBRATE_MODE: char_code = "CALIBRATE_MODE"; break;
    case MATRIX_MODE_RAW: char_code = "MATRIX_MODE_RAW"; break;
    case MAPPING_MODE: char_code = "MAPPING_MODE"; break;
    case EDIT_MODE: char_code = "EDIT_MODE"; break;
    case PLAY_MODE: char_code = "PLAY_MODE"; break;
    case ALLOCATE_MODE: char_code = "ALLOCATE_MODE"; break;
    case UPLOAD_MODE: char_code = "UPLOAD_MODE"; break;
    case APPLY_MODE: char_code = "APPLY_MODE"; break;
    case WRITE_MODE: char_code = "WRITE_MODE"; break;
    case LOAD_MODE: char_code = "LOAD_MODE"; break;
    case FETCH_MODE: char_code = "FETCH_MODE"; break;
    case STANDALONE_MODE: char_code = "STANDALONE_MODE"; break;
    case ERROR_MODE: char_code = "ERROR_MODE"; break;
  }
  return char_code;
};

const char* get_verbosity_name(verbosity_codes_t code) {
  const char* char_code = NULL;
  switch (code) {
    case PENDING_MODE_DONE: char_code = "PENDING_MODE_DONE"; break;
    case SYNC_MODE_DONE: char_code = "SYNC_MODE_DONE"; break;
    case CALIBRATE_MODE_DONE: char_code = "CALIBRATE_MODE_DONE"; break;
    case MATRIX_MODE_RAW_DONE: char_code = "MATRIX_MODE_RAW_DONE"; break;
    case MAPPING_MODE_DONE: char_code = "MAPPING_MODE_DONE"; break;
    case EDIT_MODE_DONE: char_code = "EDIT_MODE_DONE"; break;
    case PLAY_MODE_DONE: char_code = "PLAY_MODE_DONE"; break;
    case ALLOCATE_MODE_DONE: char_code = "ALLOCATE_MODE_DONE"; break;
    case ALLOCATE_DONE: char_code = "ALLOCATE_DONE"; break;
    case UPLOAD_MODE_DONE: char_code = "UPLOAD_MODE_DONE"; break;
    case UPLOAD_DONE: char_code = "UPLOAD_MODE_DONE"; break;
    case APPLY_MODE_DONE: char_code = "APPLY_MODE_DONE"; break;
    case WRITE_MODE_DONE: char_code = "WRITE_MODE_DONE"; break;
    case LOAD_MODE_DONE: char_code = "LOAD_MODE_DONE"; break;
    case FETCH_MODE_DONE: char_code = "FETCH_MODE_DONE"; break;
    case STANDALONE_MODE_DONE: char_code = "STANDALONE_MODE_DONE"; break;
    case DONE_ACTION: char_code = "DONE_ACTION"; break;
  }
  return char_code;
};

const char* get_level_name(level_codes_t code) {
  const char* char_code = NULL;
  switch (code) {
    case THRESHOLD: char_code = "THRESHOLD"; break;
    case SIG_IN: char_code = "SIG_IN"; break;
    case SIG_OUT: char_code = "SIG_OUT"; break;
    case LINE_OUT: char_code = "LINE_OUT"; break;
  }
  return char_code;
};

const char* get_error_name(error_codes_t code) {
  const char* char_code = NULL;
  switch (code) {
    case WAITING_FOR_CONFIG: char_code = "WAITING_FOR_CONFIG"; break;
    case CONNECTING_FLASH: char_code = "CONNECTING_FLASH"; break;
    case FLASH_FULL: char_code = "FLASH_FULL"; break;
    case FILE_TO_BIG: char_code = "FILE_TO_BIG"; break;
    case NO_CONFIG_FILE: char_code = "NO_CONFIG_FILE"; break;
    case WHILE_OPEN_FLASH_FILE: char_code = "WHILE_OPEN_FLASH_FILE"; break;
    case USBMIDI_CONFIG_LOAD_FAILED: char_code = "USBMIDI_CONFIG_LOAD_FAILED"; break;
    case FLASH_CONFIG_LOAD_FAILED: char_code = "FLASH_CONFIG_LOAD_FAILED"; break;
    case FLASH_CONFIG_WRITE_FAILED: char_code = "FLASH_CONFIG_WRITE_FAILED"; break;
    case CONFIG_APPLY_FAILED: char_code = "CONFIG_APPLY_FAILED"; break;
    case UNKNOWN_SYSEX: char_code = "UNKNOWN_SYSEX"; break;
    case TOO_MANY_BLOBS: char_code = "TOO_MANY_BLOBS"; break;
  }
  return char_code;
};

e256_mode_t e256_m[15] = {
  {{HIGH, LOW, false}, 50, 50, true},     // [0] PENDING_MODE
  {{HIGH, LOW, false}, 500, 500, true},   // [1] SYNC_MODE
  {{HIGH, LOW, false}, 15, 15, true},     // [2] CALIBRATE_MODE
  {{HIGH, HIGH, false}, 200, 200, true},  // [3] MATRIX_MODE_RAW
  {{HIGH, HIGH, false}, 400, 400, true},  // [4] MAPPING_MODE
  {{HIGH, LOW, false}, 1000, 50, true},   // [5] EDIT_MODE
  {{HIGH, LOW, false}, 50, 1000, true},   // [6] PLAY_MODE
  {{HIGH, LOW, false}, 1000, 1000, true}, // [7] ALLOCATE_MODE
  {{HIGH, LOW, false}, 1000, 1000, true}, // [8] UPLOAD_MODE
  {{HIGH, LOW, false}, 1000, 1000, true}, // [9] APPLY_MODE
  {{HIGH, LOW, false}, 1000, 1000, true}, // [10] WRITE_MODE
  {{HIGH, LOW, false}, 1000, 1000, true}, // [11] LOAD_MODE
  {{HIGH, LOW, false}, 1000, 1000, true}, // [12] FETCH_MODE
  {{HIGH, LOW, false}, 2500, 2500, true}, // [13] STANDALONE_MODE
  {{HIGH, HIGH, false}, 10, 10, true}     // [14] ERROR_MODE
};

// The E256 built-in encoder is used to adjust levels
Encoder e256_e(ENCODER_PIN_A, ENCODER_PIN_B);

// The levels below can be selected using E256 built-in right switche
e256_level_t e256_l[4] = {
  {{HIGH, HIGH, false}, 2, 50, 10, false}, // [0] THRESHOLD
  {{HIGH, LOW, false}, 1, 31, 17, false},  // [1] SIG_IN
  {{LOW, HIGH, false}, 13, 31, 29, false}, // [2] SIG_OUT
  {{LOW, LOW, false}, 2, 60, 3, false}     // [3] LINE_OUT
};

e256_control_t e256_ctr = {
  &e256_e,    // encoder_ptr
  &e256_m[0], // modes_ptr
  &e256_l[0]  // levels_ptr
};

mode_codes_t e256_current_mode = PENDING_MODE;
//mode_codes_t e256_lastMode = PENDING_MODE;
level_codes_t e256_current_level = THRESHOLD;
verbosity_codes_t e256_verbosity_code;
error_codes_t e256_error_code;

uint8_t* flash_config_ptr = NULL;
size_t flash_config_size = 0;

// Her it should not compile if you didn't install the library
// [Bounce2]: https://github.com/thomasfredericks/Bounce2
// https://www.pjrc.com/teensy/interrupts.html
// https://github.com/khoih-prog/Teensy_TimerInterrupt/blob/main/examples/SwitchDebounce/SwitchDebounce.ino
inline void setup_buttons() {
  BUTTON_L.attach(BUTTON_PIN_L, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  BUTTON_R.attach(BUTTON_PIN_R, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  BUTTON_L.interval(25);                        // Debounce interval of 25 millis
  BUTTON_R.interval(25);                        // Debounce interval of 25 millis
};

void setup_leds(void* ptr){
  leds_t* leds_ptr = (leds_t*)ptr;
  pinMode(LED_PIN_D1, OUTPUT);
  pinMode(LED_PIN_D2, OUTPUT);
  digitalWrite(LED_PIN_D1, leds_ptr->D1);
  digitalWrite(LED_PIN_D2, leds_ptr->D2);
};

void blink(uint8_t iter) {
  for (uint8_t i = 0; i<iter; i++){
    digitalWrite(LED_PIN_D1, HIGH);
    digitalWrite(LED_PIN_D2, HIGH);
    delay(50);
    digitalWrite(LED_PIN_D1, LOW);
    digitalWrite(LED_PIN_D2, LOW);
    delay(50);
  };
};

void set_mode(mode_codes_t mode) {
  e256_ctr.modes[(uint8_t)e256_current_mode].leds.update = false;
  e256_ctr.levels[(uint8_t)e256_current_level].leds.update = false;
  setup_leds(&e256_ctr.modes[(uint8_t)mode]);
  e256_ctr.modes[(uint8_t)mode].leds.update = true;
  //e256_lastMode = e256_current_mode;
  e256_current_mode = mode;
  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MODES)
    Serial.printf("\nMODE:\t%s", get_mode_name(mode));
  #endif
};

void set_level(level_codes_t level, uint8_t value) {
  e256_ctr.modes[(uint8_t)e256_current_mode].leds.update = false;
  e256_ctr.encoder->write(value << 2);
  setup_leds(&e256_ctr.levels[(uint8_t)level]);
  e256_ctr.levels[(uint8_t)level].update = true;
  e256_current_level = level;
  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_LEVELS)
    Serial.printf("\n%s:\t%d", get_level_name(level), value);
  #endif
};

bool flash_file(const char *fileName, uint8_t* data_ptr, uint16_t size) {
  if (sysEx_data_length != 0) {
    SerialFlash.wakeup();
    while (!SerialFlash.ready());
    if (SerialFlash.exists(fileName)) {
      SerialFlash.remove(fileName);
    };
    // Create a new file and open it for writing
    SerialFlashFile tmpFile;
    if (SerialFlash.create(fileName, size)) {
      tmpFile = SerialFlash.open(fileName);
      if (!tmpFile){
        usb_midi_send_info((uint8_t)WHILE_OPEN_FLASH_FILE, MIDI_ERROR_CHANNEL);
        return false;
      };
    }
    else {
      usb_midi_send_info((uint8_t)FLASH_FULL, MIDI_ERROR_CHANNEL);
      return false;
    };
    if (sysEx_data_length < FLASH_SIZE) {
      tmpFile.write(data_ptr, size);
      tmpFile.close();
      SerialFlash.sleep();
      return true;
    }
    else {
      usb_midi_send_info((uint8_t)FILE_TO_BIG, MIDI_ERROR_CHANNEL);
      return false;
    };
  };
  return false;
};

// Selec the current mode and perform the matrix sensor calibration 
inline void update_buttons() {
  BUTTON_L.update();
  BUTTON_R.update();
  // ACTION: BUTTON_L short pressure
  // FONCTION: CALIBRATE THE SENSOR MATRIX
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() < LONG_HOLD) {
    matrix_calibrate();
    blink(10);
  };
  // ACTION: BUTTON_L long pressure
  // FONCTION: save the mapping config file to the permanent flash memory
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() > LONG_HOLD) {
    if (sysEx_data_length > 0){
      if (flash_file("config.json", sysEx_data_ptr, sysEx_data_length)){
        usb_midi_send_info((uint8_t)WRITE_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
      }
      else {
        usb_midi_send_info((uint8_t)FLASH_CONFIG_WRITE_FAILED, MIDI_ERROR_CHANNEL);
        #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
          Serial.printf("\nSYSEX_CONFIG_WRITE: ");
          print_bytes(sysEx_data_ptr, sysEx_data_length);
        #endif
        set_mode(ERROR_MODE);
      };
    };
  };
  // ACTION: BUTTON_R long pressure
  // FONCTION: PENDING_MODE (waiting for mode)
  // LEDs: blink slowly (500ms) alternately
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() > LONG_HOLD) {
    //set_mode(PENDING_MODE);
  };
  // ACTION: BUTTON_R short pressure
  // FONCTION: SELECT_LEVEL
  // levels[0] = THRESHOLD
  // levels[1] = SIG_IN
  // levels[2] = SIG_OUT
  // levels[3] = LINE_OUT
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() < LONG_HOLD) {
    uint8_t tmp_level = (((uint8_t)e256_current_level) + 1) % 4; // Loop into level modes
    set_level((level_codes_t)tmp_level, e256_ctr.levels[tmp_level].val);
  };
};

inline void setup_encoder(){
  e256_ctr.encoder->write(e256_ctr.levels[(uint8_t)e256_current_level].val << 2);
}

// Levels values adjustment using rotary encoder
inline bool read_encoder(level_codes_t level) {
  uint8_t val = e256_ctr.encoder->read() >> 2;
  if (val != e256_ctr.levels[(uint8_t)level].val) {
    if (val > e256_ctr.levels[(uint8_t)level].maxVal) {
      e256_ctr.encoder->write(e256_ctr.levels[(uint8_t)level].maxVal << 2);
    }
    else if (val < e256_ctr.levels[(uint8_t)level].minVal) {
      e256_ctr.encoder->write(e256_ctr.levels[(uint8_t)level].minVal << 2);
    }
    else {
      e256_ctr.levels[(uint8_t)level].val = val;
      e256_ctr.levels[(uint8_t)level].leds.update = true;
    };
    return true;
  }
  else {
    return false;
  };
};

// Update levels[level] of each mode using the rotary encoder
inline void update_encoder() {
  static uint32_t levelTimeStamp = 0;
  static bool levelToggle = false;
  if (read_encoder(e256_current_level)) {
    levelTimeStamp = millis();
    levelToggle = true;
    set_level(e256_current_level, e256_ctr.levels[e256_current_level].val);
  }
  if (millis() - levelTimeStamp > LEVEL_TIMEOUT && levelToggle){
    levelToggle = false;
    set_mode(e256_current_mode);
  };
};

inline void blink_leds(uint8_t mode) {
  static uint32_t ledsTimeStamp = 0;

  if (e256_ctr.modes[mode].leds.update) {
    if (millis() - ledsTimeStamp < e256_ctr.modes[mode].timeOn && e256_ctr.modes[mode].toggle == true ) {
      e256_ctr.modes[mode].toggle = false;
      digitalWrite(LED_PIN_D1, e256_ctr.modes[mode].leds.D1);
      digitalWrite(LED_PIN_D2, e256_ctr.modes[mode].leds.D2);
    }
    else if (millis() - ledsTimeStamp > e256_ctr.modes[mode].timeOn && e256_ctr.modes[mode].toggle == false) {
      e256_ctr.modes[mode].toggle = true;
      digitalWrite(LED_PIN_D1, !e256_ctr.modes[mode].leds.D1);
      digitalWrite(LED_PIN_D2, !e256_ctr.modes[mode].leds.D2);
    }
    else if (millis() - ledsTimeStamp > e256_ctr.modes[mode].timeOn + e256_ctr.modes[mode].timeOff) {
    ledsTimeStamp = millis();
    };
  };
};

inline void fade_leds(level_codes_t level) {
  if (e256_ctr.levels[(uint8_t)level].leds.update) {
    e256_ctr.levels[(uint8_t)level].leds.update = false;
    uint8_t ledVal = constrain(map(e256_ctr.levels[(uint8_t)level].val, e256_ctr.levels[(uint8_t)level].minVal, e256_ctr.levels[(uint8_t)level].maxVal, 0, 255), 0, 255);
    analogWrite(LED_PIN_D1, abs(255 - ledVal));
    analogWrite(LED_PIN_D2, ledVal);
  };
};

// Update LEDs according to the mode and rotary encoder values
inline void update_leds() {
  blink_leds((uint8_t)e256_current_mode);
  fade_leds(e256_current_level);
};

/////////////////////////////////////////////////////
// LOAD CONFIG PARAMS FROM JSON FILE
// https://arduinojson.org/v7/how-to/upgrade-from-v6/
/////////////////////////////////////////////////////

/*
  uint8_t type;     // For MIDI status bytes see: https://github.com/PaulStoffregen/MIDI/blob/master/src/midi_Defs.h
  uint8_t data1;    // First value  (0-127), controller number or note number
  uint8_t data2;    // Second value (0-127), controller value or velocity
  uint8_t channel;  // MIDI channel (0-15)
*/

bool config_load_mappings_switchs(const JsonArray &config) {
  if (config.isNull()) {
    return false;
  };
  uint8_t n = config.size();
  mapping_switchs_alloc(n);
  for (uint8_t i = 0; i < n; i++) {
    mapping_switch_create(config[i]);
  };
  return true;
};

bool config_load_mappings_sliders(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  };
  uint8_t n = config.size();
  mapping_sliders_alloc(n);
  for (uint8_t i = 0; i < n; i++) {
    mapping_slider_create(config[i]);
  };
  return true;
};

bool config_load_mappings_knobs(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  };
  uint8_t n = config.size();
  mapping_knobs_alloc(n);
  for (uint8_t i = 0; i < n; i++) {
    mapping_knob_create(config[i]);
  };
  return true;
};

bool config_load_mappings_touchpads(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  };
  uint8_t n = config.size();
  mapping_touchpads_alloc(n);
  for (uint8_t i = 0; i < n; i++) {
    mapping_touchpad_create(config[i]);
  };
  return true;
};

bool config_load_mappings_grids(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  };
  uint8_t n = config.size();
  mapping_grids_alloc(n);
  for (uint8_t i = 0; i < n; i++) {
    mapping_grid_create(config[i]);
  };
  return true;
};

bool config_load_mappings_polygons(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  };
  uint8_t n = config.size();
  mapping_polygons_alloc(n);
  for (uint8_t i = 0; i < n; i++) {
    mapping_polygon_create(config[i]);
  };
  return true;
};

bool config_load_mappings(const JsonObject config) {
  if (config.isNull()) {
    //usb_midi_send_info((uint8_t)CONFIG_FILE_IS_NULL, MIDI_ERROR_CHANNEL);
    //Serial.println("CONFIG_ERROR");
    return false;
  };
  if (config_load_mappings_switchs(config["switch"])) {
    //Serial.println("CONFIG_LOAD_SWITCHS");
  };
  if (config_load_mappings_sliders(config["slider"])) {
    //Serial.println("CONFIG_LOAD_SLIDER");
  };
  if (config_load_mappings_knobs(config["knob"])) {
    //Serial.println("CONFIG_LOAD_KNOBS");
  };
  if (config_load_mappings_touchpads(config["touchpad"])) {
    //Serial.println("CONFIG_LOAD_TOUCHPAD");
  };
  if (config_load_mappings_grids(config["grid"])) {
    //Serial.println("CONFIG_LOAD_GRID");
  };
  if (config_load_mappings_polygons(config["polygon"])) {
    //Serial.println("CONFIG_LOAD_POLYGON");
  };
  return true;
};

bool apply_config(uint8_t* conf_ptr, size_t conf_size) {
  //DynamicJsonDocument e256_config(conf_size);
  //StaticJsonDocument<4095> e256_config;
  JsonDocument e256_config;
  DeserializationError error = deserializeJson(e256_config, conf_ptr, conf_size);
  if (error) {
    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
      Serial.printf("\nDESERIALIZATION_ERROR:\t%s", error.c_str());
    #endif
    return false;
  };
  if (config_load_mappings(e256_config["mappings"])) {
    return true;
  } else {
    return false;
  };
};

inline void setup_serial_flash(){
  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    usb_midi_send_info((uint8_t)CONNECTING_FLASH, MIDI_ERROR_CHANNEL);
  }
  else {
    SerialFlash.sleep();
  }
};

bool load_flash_config() {
  SerialFlash.wakeup();
  while (!SerialFlash.ready());
  if (SerialFlash.exists("config.json")) {
    SerialFlashFile configFile = SerialFlash.open("config.json");
    flash_config_size = configFile.size();
    flash_config_ptr = (uint8_t *)allocate(flash_config_ptr, flash_config_size);
    configFile.read(flash_config_ptr, flash_config_size);
    configFile.close();
    SerialFlash.sleep();
    return true;
  }
  else {
    return false;
  };
};

void hardware_setup(){
  setup_buttons();
  setup_encoder();
  setup_serial_flash();
};

void update_controls(){
  update_buttons();
  update_encoder();
  update_leds();
};


