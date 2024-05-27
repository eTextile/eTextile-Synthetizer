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

e256_mode_t e256_m[8] = {
  {{HIGH, LOW, false}, 50, 50, true},      // [0] PENDING_MODE
  {{HIGH, LOW, false}, 500, 500, true},    // [1] SYNC_MODE
  {{HIGH, LOW, false}, 2500, 2500, true},  // [2] STANDALONE_MODE
  {{HIGH, HIGH, false}, 200, 200, true},   // [3] MATRIX_MODE_RAW
  {{HIGH, HIGH, false}, 200, 200, true},   // [4] MATRIX_MODE_INTERP
  {{HIGH, LOW, false}, 1000, 50, true},    // [5] EDIT_MODE
  {{HIGH, LOW, false}, 50, 1000, true},    // [6] PLAY_MODE
  {{HIGH, LOW, false}, 10, 10, true}       // [7] ERROR_MODE
};

e256_state_t e256_s[2] = {
  {{LOW, LOW, false}, 50, 50, 8},          // [0] CALIBRATE_REQUEST
  {{HIGH, LOW, false}, 15, 50, 200}        // [1] CONFIG_FILE_REQUEST
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
  &e256_s[0], // state_ptr
  &e256_l[0]  // levels_ptr
};

mapping_mode_t e256_currentMode = PENDING_MODE;
mapping_mode_t e256_lastMode = PENDING_MODE;

uint8_t e256_level = THRESHOLD;

uint8_t* flash_config_ptr = NULL;
uint16_t flash_configSize = 0;

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
  leds_t* leds = (leds_t*)ptr;
  pinMode(LED_PIN_D1, OUTPUT);
  pinMode(LED_PIN_D2, OUTPUT);
  digitalWrite(LED_PIN_D1, leds->D1);
  digitalWrite(LED_PIN_D2, leds->D2);
};

void set_mode(uint8_t mode) {
  e256_ctr.modes[(uint8_t)e256_currentMode].leds.update = false;
  e256_ctr.levels[e256_level].leds.update = false;
  setup_leds(&e256_ctr.modes[mode]);
  e256_ctr.modes[mode].leds.update = true;
  e256_lastMode = e256_currentMode;
  e256_currentMode = (mapping_mode_t)mode; // IS IT OK!?
  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MODES)
    Serial.printf("\nSET_MODE:%d", mode);
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

void set_state(uint8_t state) {
  setup_leds(&e256_ctr.states[state]);
  for (int i = 0; i<e256_ctr.states[state].iter; i++){
    digitalWrite(LED_PIN_D1, e256_ctr.states[state].leds.D1);
    digitalWrite(LED_PIN_D2, e256_ctr.states[state].leds.D2);
    delay(e256_ctr.states[state].timeOn);
    digitalWrite(LED_PIN_D1, !e256_ctr.states[state].leds.D1);
    digitalWrite(LED_PIN_D2, !e256_ctr.states[state].leds.D2);
    delay(e256_ctr.states[state].timeOff);
  };
  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_STATES)
    Serial.printf("\nSET_STATE:%d", state);
  #endif
};

bool flash_file(const char *fileName, uint8_t* data_ptr, uint16_t size) {
  if (sysEx_data_length != 0) {
    if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
      usb_midi_send_info(CONNECTING_FLASH, MIDI_ERROR_CHANNEL);
      return false;
    };
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
      //usb_midi_send_info(FILE_WRITE_DONE, MIDI_ERROR_CHANNEL);
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
        usb_midi_send_info(FLASH_CONFIG_WRITE_DONE, MIDI_VERBOSITY_CHANNEL);
      }
      else {
        #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
          Serial.printf("\nSYSEX_CONFIG_WRITE: "); // FIXME FILE CORRUPTED!
          printBytes(sysEx_data_ptr, sysEx_data_length);
        #else
          usb_midi_send_info(FLASH_CONFIG_WRITE_FAILED, MIDI_ERROR_CHANNEL);
          set_mode(ERROR_MODE);
        #endif
      }
    };
  };
  // ACTION: BUTTON_R long press
  // FONCTION: PENDING_MODE (waiting for mode)
  // LEDs: blink slowly (500ms) alternately
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() > LONG_HOLD) {
    //set_mode(PENDING_MODE);
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
    set_mode(e256_lastMode);
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

inline bool config_load_mappings_switchs(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  }

  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
    // DEBUG...
  #endif
  
  mapping_switchs_alloc(config.size()); // TEST_IT!!!!!!!!!!

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

bool config_load_mappings(const JsonObject &config) {
  if (config.isNull()) {
    usb_midi_send_info(CONFIG_APPLY_FAILED, MIDI_ERROR_CHANNEL);
    set_mode(ERROR_MODE);
    return false;
  };
  if (!config_load_mappings_switchs(config["switchs"])) {
    usb_midi_send_info(CONFIG_APPLY_FAILED, MIDI_ERROR_CHANNEL);
    set_mode(ERROR_MODE);
    return false;
  };
  if (!config_load_mappings_sliders(config["sliders"])) {
    usb_midi_send_info(CONFIG_APPLY_FAILED, MIDI_ERROR_CHANNEL);
    set_mode(ERROR_MODE);
    return false;
  };
  if (!config_load_mappings_knobs(config["knobs"])) {
    usb_midi_send_info(CONFIG_APPLY_FAILED, MIDI_ERROR_CHANNEL);
    set_mode(ERROR_MODE);
    return false;
  };
  if (!config_load_mappings_touchpads(config["touchpads"])) {
    usb_midi_send_info(CONFIG_APPLY_FAILED, MIDI_ERROR_CHANNEL);
    set_mode(ERROR_MODE);
    return false;
  };
  if (!config_load_mappings_polygons(config["polygons"])) {
    usb_midi_send_info(CONFIG_APPLY_FAILED, MIDI_ERROR_CHANNEL);
    set_mode(ERROR_MODE);
    return false;
  };
  if (!config_load_mappings_grids(config["grids"])) {
    usb_midi_send_info(CONFIG_APPLY_FAILED, MIDI_ERROR_CHANNEL);
    set_mode(ERROR_MODE);
    return false;
  };
  return true;
};

bool apply_config(uint8_t* conf_ptr, uint16_t conf_size) {
  //DynamicJsonDocument config(conf_size);
  StaticJsonDocument<4095> config;
  DeserializationError error = deserializeJson(config, conf_ptr);
  if (error) {
    return false;
  };
  if (config_load_mappings(config["mappings"])) {
    return true;
  } else {
    return false;
  };
};

bool load_flash_config() {
  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    usb_midi_send_info(CONNECTING_FLASH, MIDI_ERROR_CHANNEL);
    return false;
  }
  else {
    SerialFlashFile configFile = SerialFlash.open("config.json");
    if (configFile) {
      flash_configSize = (uint16_t)configFile.size();
      flash_config_ptr = (uint8_t *)allocate(flash_config_ptr, flash_configSize);
      configFile.read(flash_config_ptr, flash_configSize);
      return true;
    }
    else {
      return false;
    };
  };
};

void hardware_setup(){
  setup_buttons();
  setup_encoder();
};

void update_controls(){
  update_buttons();
  update_encoder();
  update_leds();
};
