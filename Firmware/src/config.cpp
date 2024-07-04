/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
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
#include "mappings.h"

// The modes below can be selected using E256 built-in switches
Bounce BUTTON_L = Bounce();
Bounce BUTTON_R = Bounce();

mode_codes_t mode_code;
verbosity_codes_t verbosity_code;
error_codes_t error_code;

const char* get_mode_name(mode_codes_t code) {
  const char* char_code;
  switch (code) {
    case PENDING_MODE: char_code = "PENDING_MODE";
    case SYNC_MODE: char_code = "SYNC_MODE";
    case CALIBRATE_MODE: char_code = "CALIBRATE_MODE";
    case MATRIX_MODE_RAW: char_code = "MATRIX_MODE_RAW"; 
    case EDIT_MODE: char_code = "EDIT_MODE";
    case PLAY_MODE: char_code = "PLAY_MODE";
    case ALLOCATE_MODE: char_code = "ALLOCATE_MODE";
    case UPLOAD_MODE: char_code = "UPLOAD_MODE";
    case APPLY_MODE: char_code = "APPLY_MODE";
    case WRITE_MODE: char_code = "WRITE_MODE";
    case LOAD_MODE: char_code = "LOAD_MODE";
    case FETCH_MODE: char_code = "FETCH_MODE";
    case STANDALONE_MODE: char_code = "STANDALONE_MODE";
    case ERROR_MODE: char_code = "ERROR_MODE";
    default:
      char_code = "null";
      break;
  }
  return char_code;
};

const char* get_verbosity_name(verbosity_codes_t code) {
  const char* char_code;
  switch (code) {
    case PENDING_MODE_DONE: char_code = "PENDING_MODE_DONE";
    case SYNC_MODE_DONE: char_code = "SYNC_MODE_DONE";
    case CALIBRATE_MODE_DONE: char_code = "CALIBRATE_MODE_DONE";
    case MATRIX_MODE_RAW_DONE: char_code = "MATRIX_MODE_RAW_DONE";
    case EDIT_MODE_DONE: char_code = "EDIT_MODE_DONE";
    case PLAY_MODE_DONE: char_code = "PLAY_MODE_DONE";
    case ALLOCATE_MODE_DONE: char_code = "ALLOCATE_MODE_DONE";
    case UPLOAD_MODE_DONE: char_code = "UPLOAD_MODE_DONE";
    case APPLY_MODE_DONE: char_code = "APPLY_MODE_DONE";
    case WRITE_MODE_DONE: char_code = "WRITE_MODE_DONE";
    case LOAD_MODE_DONE: char_code = "LOAD_MODE_DONE";
    case FETCH_MODE_DONE: char_code = "FETCH_MODE_DONE";
    case STANDALONE_MODE_DONE: char_code = "STANDALONE_MODE_DONE";
    case DONE_ACTION: char_code = "DONE_ACTION";
    default:
      char_code = "null";
      break;
  }
  return char_code;
};

const char* get_error_name(error_codes_t code) {
  const char* char_code;
  switch (code) {
    case WAITING_FOR_CONFIG: char_code = "WAITING_FOR_CONFIG";
    case CONNECTING_FLASH: char_code = "CONNECTING_FLASH";
    case FLASH_FULL: char_code = "FLASH_FULL";
    case FILE_TO_BIG: char_code = "FILE_TO_BIG";
    case NO_CONFIG_FILE: char_code = "NO_CONFIG_FILE";
    case WHILE_OPEN_FLASH_FILE: char_code = "WHILE_OPEN_FLASH_FILE";
    case USBMIDI_CONFIG_LOAD_FAILED: char_code = "USBMIDI_CONFIG_LOAD_FAILED";
    case FLASH_CONFIG_LOAD_FAILED: char_code = "FLASH_CONFIG_LOAD_FAILED";
    case FLASH_CONFIG_WRITE_FAILED: char_code = "FLASH_CONFIG_WRITE_FAILED";
    case CONFIG_APPLY_FAILED: char_code = "CONFIG_APPLY_FAILED";
    case UNKNOWN_SYSEX: char_code = "UNKNOWN_SYSEX";
    case TOO_MANY_BLOBS: char_code = "TOO_MANY_BLOBS";
    default:
      char_code = "null";
      break;
  }
  return char_code;
};

e256_mode_t e256_m[15] = {
  {{HIGH, LOW, false}, 50, 50, true},     // [0] PENDING_MODE
  {{HIGH, LOW, false}, 500, 500, true},   // [1] SYNC_MODE
  {{HIGH, LOW, false}, 15, 15, true},     // [2] CALIBRATE_MODE
  {{HIGH, HIGH, false}, 250, 250, true},  // [3] MATRIX_MODE_RAW
  {{HIGH, HIGH, false}, 100, 100, true},  // [4] MAPPING_MODE
  {{HIGH, LOW, false}, 1000, 50, true},   // [5] EDIT_MODE
  {{HIGH, LOW, false}, 50, 1000, true},   // [6] PLAY_MODE
  {{HIGH, LOW, false}, 10, 10, true},     // [7] ALLOCATE_MODE
  {{HIGH, LOW, false}, 10, 10, true},     // [8] UPLOAD_MODE
  {{HIGH, LOW, false}, 10, 10, true},     // [9] APPLY_MODE
  {{HIGH, LOW, false}, 10, 10, true},     // [10] WRITE_MODE
  {{HIGH, LOW, false}, 10, 10, true},     // [11] LOAD_MODE
  {{HIGH, LOW, false}, 30, 10, true},     // [12] FETCH_MODE
  {{HIGH, LOW, false}, 2500, 2500, true}, // [13] STANDALONE_MODE
  {{HIGH, LOW, false}, 10, 10, true}      // [14] ERROR_MODE
};

// The levels below can be adjusted using E256 built-in encoder
Encoder e256_e(ENCODER_PIN_A, ENCODER_PIN_B);

e256_level_t e256_l[4] = {
  {{HIGH, HIGH, false}, 2, 50, 10, false}, // [0]  THRESHOLD
  {{HIGH, LOW, false}, 1, 31, 17, false},  // [1]  SIG_IN
  {{LOW, HIGH, false}, 13, 31, 29, false}, // [2]  SIG_OUT
  {{LOW, LOW, false}, 2, 60, 3, false}     // [3]  LINE_OUT
};

e256_control_t e256_ctr = {
  &e256_e,    // encoder_ptr
  &e256_m[0], // modes_ptr
  //&e256_s[0], // state_ptr
  &e256_l[0]  // levels_ptr
};

mode_codes_t e256_currentMode = PENDING_MODE;
mode_codes_t e256_lastMode = PENDING_MODE;

uint8_t e256_level = THRESHOLD;

uint8_t* flash_config_ptr = NULL;
uint32_t flash_config_size = 0;

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
  for (int i = 0; i<iter; i++){
    digitalWrite(LED_PIN_D1, HIGH);
    digitalWrite(LED_PIN_D2, HIGH);
    delay(50);
    digitalWrite(LED_PIN_D1, LOW);
    digitalWrite(LED_PIN_D2, LOW);
    delay(50);
  };
};

void set_mode(uint8_t mode) {
  e256_ctr.modes[(uint8_t)e256_currentMode].leds.update = false;
  e256_ctr.levels[e256_level].leds.update = false;
  setup_leds(&e256_ctr.modes[mode]);
  e256_ctr.modes[mode].leds.update = true;
  e256_lastMode = e256_currentMode;
  e256_currentMode = (mode_codes_t)mode;
  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MODES)
    Serial.printf("\nSET_MODE:\t%s", get_mode_name((mode_codes_t)mode));
  #endif
};

void set_level(uint8_t level, uint8_t value) {
  e256_ctr.modes[(uint8_t)e256_currentMode].leds.update = false;
  e256_ctr.encoder->write(value << 2);
  setup_leds(&e256_ctr.levels[level]);
  e256_ctr.levels[level].update = true;
  e256_level = level;
  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_LEVELS)
    Serial.printf("\nSET_LEVEL:%d_%d", level, value);
  #endif
};

bool flash_file(const char *fileName, uint8_t* data_ptr, uint16_t size) {
  if (sysEx_data_length != 0) {
    SerialFlash.wakeup();
    while (!SerialFlash.ready());
    if (SerialFlash.exists(fileName)) {
      SerialFlash.remove(fileName); // 
    };
    // Create a new file and open it for writing
    SerialFlashFile tmpFile;
    if (SerialFlash.create(fileName, size)) {
      tmpFile = SerialFlash.open(fileName);
      if (!tmpFile){
        usb_midi_send_info(WHILE_OPEN_FLASH_FILE, MIDI_ERROR_CHANNEL);
        return false;
      };
    }
    else {
      usb_midi_send_info(FLASH_FULL, MIDI_ERROR_CHANNEL);
      return false;
    };
    if (sysEx_data_length < FLASH_SIZE) {
      tmpFile.write(data_ptr, size);
      tmpFile.close();
      SerialFlash.sleep();
      return true;
    }
    else {
      usb_midi_send_info(FILE_TO_BIG, MIDI_ERROR_CHANNEL);
      return false;
    };
  };
  return false;
};

// Selec the current mode and perform the matrix sensor calibration 
inline void update_buttons() {
  BUTTON_L.update();
  BUTTON_R.update();
  // ACTION: BUTTON_L short press
  // FONCTION: CALIBRATE THE SENSOR MATRIX
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() < LONG_HOLD) {
    //matrix_calibrate(); // FIXME for erratic call when booting!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //set_state(CALIBRATE_REQUEST);
  };
  // ACTION: BUTTON_L long press
  // FONCTION: save the mapping config file to the permanent flash memory
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() > LONG_HOLD) {
    if (sysEx_data_length > 0){
      if (flash_file("config.json", sysEx_data_ptr, sysEx_data_length)){
        usb_midi_send_info(WRITE_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
      }
      else {
        usb_midi_send_info(FLASH_CONFIG_WRITE_FAILED, MIDI_ERROR_CHANNEL);
        #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
          Serial.printf("\nSYSEX_CONFIG_WRITE: ");
          printBytes(sysEx_data_ptr, sysEx_data_length);
        #endif
        set_mode((uint8_t)ERROR_MODE);
      };
    };
  };
  // ACTION: BUTTON_R long press
  // FONCTION: PENDING_MODE (waiting for mode)
  // LEDs: blink slowly (500ms) alternately
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() > LONG_HOLD) {
    //set_mode((uint8_t)PENDING_MODE);
  };
  // ACTION: BUTTON_R short press
  // FONCTION: SELECT_LEVEL
  // levels[0] = THRESHOLD
  // levels[1] = SIG_IN
  // levels[2] = SIG_OUT
  // levels[3] = LINE_OUT
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() < LONG_HOLD) {
    e256_level = (e256_level + 1) % 4; // Loop into level modes
    set_level(e256_level, e256_ctr.levels[e256_level].val);
  };
};

inline void setup_encoder(){
  e256_ctr.encoder->write(e256_ctr.levels[e256_level].val << 2);
}

// Levels values adjustment using rotary encoder
// May interrupt driven !?
inline bool read_encoder(uint8_t level) {
  uint8_t val = e256_ctr.encoder->read() >> 2;
  if (val != e256_ctr.levels[level].val) {
    if (val > e256_ctr.levels[level].maxVal) {
      e256_ctr.encoder->write(e256_ctr.levels[level].maxVal << 2);
      return false;
    }
    else if (val < e256_ctr.levels[level].minVal) {
      e256_ctr.encoder->write(e256_ctr.levels[level].minVal << 2);
      return false;
    }
    else {
      e256_ctr.levels[level].val = val;
      e256_ctr.levels[level].leds.update = true;
      return true;
    };
  }
  else {
    return false;
  };
};

// Update levels[level] of each mode using the rotary encoder
inline void update_encoder() {
  static uint32_t levelTimeStamp = 0;
  static bool levelToggle = false;
  if (read_encoder(e256_level)) {
    levelTimeStamp = millis();
    levelToggle = true;
    set_level(e256_level, e256_ctr.levels[e256_level].val);
  }
  if (millis() - levelTimeStamp > LEVEL_TIMEOUT && levelToggle){
    levelToggle = false;
    set_mode((uint8_t)e256_lastMode);
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

inline void fade_leds(uint8_t level) {
  if (e256_ctr.levels[level].leds.update) {
    e256_ctr.levels[level].leds.update = false;
    uint8_t ledVal = constrain(map(e256_ctr.levels[level].val, e256_ctr.levels[level].minVal, e256_ctr.levels[level].maxVal, 0, 255), 0, 255);
    analogWrite(LED_PIN_D1, abs(255 - ledVal));
    analogWrite(LED_PIN_D2, ledVal);
  };
};

// Update LEDs according to the mode and rotary encoder values
inline void update_leds() {
  blink_leds((uint8_t)e256_currentMode);
  fade_leds(e256_level);
};

// https://arduinojson.org/v7/how-to/upgrade-from-v6/
inline bool config_load_mappings_switchs(const JsonArray &config) {
  if (config.isNull()) {
    return false;
  }
  mapping_switchs_alloc(config.size());

  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
    Serial.printf("\nCONF_SIZE:\t%d", config.size());     // print 1 (ok)
    Serial.printf("\nFROM_X:\t%d", config[0]["from"][0]); // print 537296528 FIXME!
    Serial.printf("\nTO_X:\t%d", config[0]["to"][0]);     // print 537296528 FIXME!
    Serial.printf("\nMODE_Z:\t%d", config[0]["mode_z"]);  // print 537296528 FIXME!
  #endif
  
  uint8_t midi_status;
  midi_status_t status;
  for (uint8_t i = 0; i < mapp_switchs; i++) {
    mapp_switchParams[i].rect.from.x = config[i]["from"][0];
    mapp_switchParams[i].rect.from.y = config[i]["from"][1];
    mapp_switchParams[i].rect.to.x = config[i]["to"][0];
    mapp_switchParams[i].rect.to.y = config[i]["to"][1];
    
    midi_status = config[i]["msg"]["midi"]["status"];
    midi_msg_status_unpack(midi_status, &status); // Extract MIDI type and channel from MIDI status 
    mapp_switchParams[i].msg.midi.type = status.type;
    mapp_switchParams[i].msg.midi.data1 = config[i]["msg"]["midi"]["data1"];
    mapp_switchParams[i].msg.midi.data2 = config[i]["msg"]["midi"]["data2"];
    mapp_switchParams[i].msg.midi.channel = status.channel;
    if (status.type == C_CHANGE || status.type == P_AFTERTOUCH) {
      mapp_switchParams[i].msg.limit.min = config[i]["msg"]["limit"]["min"];
      mapp_switchParams[i].msg.limit.max = config[i]["msg"]["limit"]["max"]; 
    }
  }
  return true;
}

inline bool config_load_mappings_sliders(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  };
  
  mapping_sliders_alloc(config.size());
  
  for (uint8_t i = 0; i < mapp_sliders; i++) {
    mapp_slidersParams[i].rect.from.x = config[i]["from"][0];
    mapp_slidersParams[i].rect.from.y = config[i]["from"][1];
    mapp_slidersParams[i].rect.to.x = config[i]["to"][0];
    mapp_slidersParams[i].rect.to.y = config[i]["to"][1];
    
    uint8_t midi_status;
    midi_status_t status;
    for (uint8_t j = 0; j<config[i]["touchs"]; j++){
      midi_status = config[i][j]["msg"]["dir"]["midi"]["status"];
      midi_msg_status_unpack(midi_status, &status);
      mapp_slidersParams[i].touch[j].dir.midi.type = status.type;
      mapp_slidersParams[i].touch[j].dir.midi.data1 = config[i][j]["msg"]["dir"]["midi"]["data1"];
      mapp_slidersParams[i].touch[j].dir.midi.data2 = config[i][j]["msg"]["dir"]["midi"]["data2"];
      mapp_slidersParams[i].touch[j].dir.midi.channel = status.channel;
      if (status.type == C_CHANGE || status.type == P_AFTERTOUCH){
        mapp_slidersParams[i].touch[j].dir.limit.min = config[i][j]["msg"]["dir"]["limit"]["min"];
        mapp_slidersParams[i].touch[j].dir.limit.max = config[i][j]["msg"]["dir"]["limit"]["max"];
      }

      midi_status = config[i][j]["msg"]["pos_z"]["midi"]["status"];
      midi_msg_status_unpack(midi_status, &status);
      mapp_slidersParams[i].touch[j].pos_z.midi.type = status.type;
      mapp_slidersParams[i].touch[j].pos_z.midi.data1 = config[i][j]["msg"]["pos_z"]["midi"]["data1"];
      mapp_slidersParams[i].touch[j].pos_z.midi.data2 = config[i][j]["msg"]["pos_z"]["midi"]["data2"];
      mapp_slidersParams[i].touch[j].pos_z.midi.channel = status.channel;
      if (status.type == C_CHANGE || status.type == P_AFTERTOUCH){
        mapp_slidersParams[i].touch[j].pos_z.limit.min = config[i][j]["msg"]["pos_z"]["limit"]["min"];
        mapp_slidersParams[i].touch[j].pos_z.limit.max = config[i][j]["msg"]["pos_z"]["limit"]["max"];
      }
    }
  };
  return true;
};

inline bool config_load_mappings_knobs(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  }
  mapping_knobs_alloc(config.size());
  for (uint8_t i = 0; i < mapp_knobs; i++) {
    mapp_knobsParams[i].rect.from.x = config[i]["from"][0];
    mapp_knobsParams[i].rect.from.y = config[i]["from"][1];
    mapp_knobsParams[i].rect.to.x = config[i]["to"][0];
    mapp_knobsParams[i].rect.to.y = config[i]["to"][1];
    mapp_knobsParams[i].radius = config[i]["radius"];
    mapp_knobsParams[i].offset = config[i]["offset"];

    uint8_t midi_status;
    midi_status_t status;
    for (uint8_t j = 0; j<config[i]["touchs"]; j++){
      midi_status = config[i][j]["msg"]["radius"]["midi"]["status"];
      midi_msg_status_unpack(midi_status, &status);
      mapp_knobsParams[i].touch[j].radius.midi.type = status.type;
      mapp_knobsParams[i].touch[j].radius.midi.data1 = config[i][j]["msg"]["radius"]["midi"]["data1"];
      mapp_knobsParams[i].touch[j].radius.midi.data2 = config[i][j]["msg"]["radius"]["midi"]["data2"];
      mapp_knobsParams[i].touch[j].radius.midi.channel = status.channel;
      if (status.type == C_CHANGE || status.type == P_AFTERTOUCH){
        mapp_knobsParams[i].touch[j].radius.limit.min = config[i][j]["msg"]["radius"]["limit"]["min"];
        mapp_knobsParams[i].touch[j].radius.limit.max = config[i][j]["msg"]["radius"]["limit"]["max"];
      }

      midi_status = config[i][j]["msg"]["theta"]["midi"]["status"];
      midi_msg_status_unpack(midi_status, &status);
      mapp_knobsParams[i].touch[j].theta.midi.type = status.type;
      mapp_knobsParams[i].touch[j].theta.midi.data1 = config[i][j]["msg"]["theta"]["midi"]["data1"];
      mapp_knobsParams[i].touch[j].theta.midi.data2 = config[i][j]["msg"]["theta"]["midi"]["data2"];
      mapp_knobsParams[i].touch[j].theta.midi.channel = status.channel;
      if (status.type == C_CHANGE || status.type == P_AFTERTOUCH){
        mapp_knobsParams[i].touch[j].theta.limit.min = config[i][j]["msg"]["theta"]["limit"]["min"];
        mapp_knobsParams[i].touch[j].theta.limit.max = config[i][j]["msg"]["theta"]["limit"]["max"];
      }

      midi_status = config[i][j]["msg"]["pressure"]["midi"]["status"];
      midi_msg_status_unpack(midi_status, &status);
      mapp_knobsParams[i].touch[j].pressure.midi.type = status.type;
      mapp_knobsParams[i].touch[j].pressure.midi.data1 = config[i][j]["msg"]["pressure"]["midi"]["data1"];
      mapp_knobsParams[i].touch[j].pressure.midi.data2 = config[i][j]["msg"]["pressure"]["midi"]["data2"];
      mapp_knobsParams[i].touch[j].pressure.midi.channel = status.channel;
      if (status.type == C_CHANGE || status.type == P_AFTERTOUCH){
        mapp_knobsParams[i].touch[j].pressure.limit.min = config[i][j]["msg"]["pressure"]["limit"]["min"];
        mapp_knobsParams[i].touch[j].pressure.limit.max = config[i][j]["msg"]["pressure"]["limit"]["max"];
      }
    }
  };
  return true;
}

/*
  uint8_t type;     // For MIDI status bytes see: https://github.com/PaulStoffregen/MIDI/blob/master/src/midi_Defs.h
  uint8_t data1;    // First value  (0-127), controller number or note number
  uint8_t data2;    // Second value (0-127), controller value or velocity
  uint8_t channel;  // MIDI channel (0-15)
*/

inline bool config_load_mappings_touchpads(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  };
  mapping_touchpads_alloc(config.size());
  for (uint8_t i = 0; i < mapp_touchpads; i++) {
    mapp_touchpadsParams[i].rect.from.x = config[i]["from"][0];
    mapp_touchpadsParams[i].rect.from.y = config[i]["from"][1];
    mapp_touchpadsParams[i].rect.to.x = config[i]["to"][0];
    mapp_touchpadsParams[i].rect.to.y = config[i]["to"][1];
    if (config[i]["touchs"] < MAX_TOUCHPAD_TOUCHS) {
      mapp_touchpadsParams[i].touchs = config[i]["touchs"];
      
      uint8_t midi_status;
      midi_status_t status;
      for (uint8_t j = 0; j < config[i]["touchs"]; j++) {
        mapp_touchpadsParams[i].touch[j].pos_x.midi.type = status.type;

        midi_status = config[i][j]["pos_x"]["msg"]["midi"]["status"];
        midi_msg_status_unpack(midi_status, &status);
        mapp_touchpadsParams[i].touch[j].pos_x.midi.type = status.type;
        mapp_touchpadsParams[i].touch[j].pos_x.midi.data1 = config[i][j]["msg"]["pos_x"]["midi"]["data1"];
        mapp_touchpadsParams[i].touch[j].pos_x.midi.data2 = config[i][j]["msg"]["pos_x"]["midi"]["data2"];
        mapp_touchpadsParams[i].touch[j].pos_x.midi.channel = status.channel;
        if (status.type == C_CHANGE || status.type == P_AFTERTOUCH){
          mapp_touchpadsParams[i].touch[j].pos_x.limit.min = config[i][j]["msg"]["pos_x"]["limit"]["min"];
          mapp_touchpadsParams[i].touch[j].pos_x.limit.max = config[i][j]["msg"]["pos_x"]["limit"]["max"];
        }

        midi_status = config[i][j]["pos_y"]["msg"]["midi"]["status"];
        midi_msg_status_unpack(midi_status, &status);
        mapp_touchpadsParams[i].touch[j].pos_y.midi.type = status.type;
        mapp_touchpadsParams[i].touch[j].pos_y.midi.data1 = config[i][j]["msg"]["pos_y"]["midi"]["data1"];
        mapp_touchpadsParams[i].touch[j].pos_y.midi.data2 = config[i][j]["msg"]["pos_y"]["midi"]["data2"];
        mapp_touchpadsParams[i].touch[j].pos_y.midi.channel = status.channel;
        if (status.type == C_CHANGE || status.type == P_AFTERTOUCH){
        mapp_touchpadsParams[i].touch[j].pos_y.limit.min = config[i][j]["msg"]["pos_y"]["limit"]["min"];
        mapp_touchpadsParams[i].touch[j].pos_y.limit.max = config[i][j]["msg"]["pos_y"]["limit"]["max"];
        }

        midi_status = config[i][j]["pos_z"]["msg"]["midi"]["status"];
        midi_msg_status_unpack(midi_status, &status);
        mapp_touchpadsParams[i].touch[j].pos_z.midi.type = status.type;
        mapp_touchpadsParams[i].touch[j].pos_z.midi.data1 = config[i][j]["msg"]["pos_z"]["midi"]["data1"];
        mapp_touchpadsParams[i].touch[j].pos_z.midi.data2 = config[i][j]["msg"]["pos_z"]["midi"]["data2"];
        mapp_touchpadsParams[i].touch[j].pos_z.midi.channel = status.channel;
        if (status.type == C_CHANGE || status.type == P_AFTERTOUCH){
          mapp_touchpadsParams[i].touch[j].pos_z.limit.min = config[i][j]["msg"]["pos_z"]["limit"]["min"];
          mapp_touchpadsParams[i].touch[j].pos_z.limit.max = config[i][j]["msg"]["pos_z"]["limit"]["max"];
        }
      }
    }
    else {
      //usb_midi_send_info(TOO_MANY_TOUCHS, MIDI_ERROR_CHANNEL); // TODO!
    }
  };
  return true;
};

inline bool config_load_mappings_polygons(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  };
  mapping_polygons_alloc(config.size());
  for (uint8_t i = 0; i < mapp_polygons; i++) {
    mapp_polygonsParams[i].point_cnt = config[i]["cnt"];
    for (uint8_t j = 0; j < config[i]["cnt"]; j++) {
      mapp_polygonsParams[i].point[j].x = (float)config[i]["point"][j]["X"];
      mapp_polygonsParams[i].point[j].y = (float)config[i]["point"][j]["Y"];
    };
  };
  return true;
};

inline bool config_load_mappings_grids(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  }
  mapping_grids_alloc(config.size());
  for (uint8_t i = 0; i < mapp_grids; i++) {
    mapp_gridsParams[i].rect.from.x = config[i]["from"][0];
    mapp_gridsParams[i].rect.from.y = config[i]["from"][1];
    mapp_gridsParams[i].rect.to.x = config[i]["to"][0];
    mapp_gridsParams[i].rect.to.y = config[i]["to"][1];
    mapp_gridsParams[i].cols = config[i]["cols"];
    mapp_gridsParams[i].rows = config[i]["rows"];
    //mapp_gridsParams[i].mode = config[i]["mode"];
    mapp_gridsParams[i].keys_count = mapp_gridsParams[i].cols * mapp_gridsParams[i].rows;

    uint8_t midi_status;
    midi_status_t status;
    for (uint8_t j = 0; j < mapp_gridsParams[i].keys_count; j++) {
      midi_status = config[i][j]["msg"]["midi"]["status"];
      midi_msg_status_unpack(midi_status, &status);
      mapp_gridsParams[i].keys[j].msg.midi.type = status.type;
      mapp_gridsParams[i].keys[j].msg.midi.data1 = config[i][j]["msg"]["midi"]["data1"];
      mapp_gridsParams[i].keys[j].msg.midi.data2 = config[i][j]["msg"]["midi"]["data2"];
      mapp_gridsParams[i].keys[j].msg.midi.channel = status.channel;
      if (status.type == C_CHANGE || status.type == P_AFTERTOUCH){
        mapp_gridsParams[i].keys[j].msg.limit.min = config[i][j]["msg"]["limit"]["min"];
        mapp_gridsParams[i].keys[j].msg.limit.max = config[i][j]["msg"]["limit"]["max"];
      }
    }
  };
  return true;
};

bool config_load_mappings(const JsonObject config) {
  if (config.isNull()) {
    //usb_midi_send_info(CONFIG_FILE_IS_NULL, MIDI_ERROR_CHANNEL);
    return false;
  };
  if (!config_load_mappings_switchs(config["switch"])) {
    ///usb_midi_send_info(CONFIG_LOAD_SWITCHS_FAILED, MIDI_ERROR_CHANNEL);
    return false;
  };
  if (!config_load_mappings_sliders(config["slider"])) {
    //usb_midi_send_info(CONFIG_LOAD_SLIDERS_FAILED, MIDI_ERROR_CHANNEL);
    return false;
  };
  if (!config_load_mappings_knobs(config["knob"])) {
    //usb_midi_send_info(CONFIG_LOAD_KNOBS_FAILED, MIDI_ERROR_CHANNEL);
    return false;
  };
  if (!config_load_mappings_touchpads(config["touchpad"])) {
    //usb_midi_send_info(CONFIG_LOAD_TOUCHPADS_FAILED, MIDI_ERROR_CHANNEL);
    return false;
  };
  if (!config_load_mappings_polygons(config["polygon"])) {
    //usb_midi_send_info(CONFIG_LOAD_POLYGONS_FAILED, MIDI_ERROR_CHANNEL);
    return false;
  };
  if (!config_load_mappings_grids(config["grid"])) {
    //usb_midi_send_info(CONFIG_LOAD_GRIDS_FAILED, MIDI_ERROR_CHANNEL);
    return false;
  };
  return true;
};

bool apply_config(uint8_t* conf_ptr, uint32_t size) {
  //DynamicJsonDocument e256_config(conf_size);
  //StaticJsonDocument<4095> e256_config;
  JsonDocument e256_config;
  
  DeserializationError error = deserializeJson(e256_config, conf_ptr, size);
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

bool setup_serial_flash(){
  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    usb_midi_send_info(CONNECTING_FLASH, MIDI_ERROR_CHANNEL);
    return false;
  }
  else {
    SerialFlash.sleep();
    return true;
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


