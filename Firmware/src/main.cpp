/*
  **eTextile-Synthesizer**
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "scan.h"
#include "interp.h"
#include "blob.h"
#include "median.h"
#include "midi_bus.h"
#include "mapping_lib.h"
#include "usb_midi_io.h"
#include "hardware_midi_io.h"
#include "sound_card.h"

uint32_t fpsTimeStamp = 0;
uint16_t fps = 0;

void setup() {
  hardware_setup();
  scan_setup();
  interp_setup();
  blob_setup();
  midi_bus_setup();
  mapping_lib_setup();
  usb_midi_io_setup();
  hardware_midi_io_setup();
  //while (!Serial);
  //Serial.println("START");

set_mode(PENDING_MODE);
bootTime = millis();
};

void loop() {
  matrix_scan();
  matrix_interp();
  matrix_find_blobs();
  update_controls();

  switch (e256_currentMode) {
    case PENDING_MODE:
      usb_midi_recive();
      usb_midi_pending_mode_timeout();
      break;
    case SYNC_MODE:
      usb_midi_recive();
      break;
    case STANDALONE_MODE:
      #if defined(GRID_LAYOUT_MIDI_IN)
        hardware_midi_read_input();
      #endif
      mapping_lib_update();
      //update_levels(); // NOT_USED in this branche!
      hardware_midi_transmit();
      break;
    case MATRIX_MODE_RAW:
      usb_midi_recive();
      usb_midi_transmit();
      break;
    case EDIT_MODE:
      usb_midi_recive();
      usb_midi_transmit();
      break;
    case PLAY_MODE:
      usb_midi_recive();
      mapping_lib_update();
      usb_midi_transmit();
      break;
  };

  #if defined(USB_MIDI_SERIAL) & defined(DEBUG_FPS)
  if (millis() - fpsTimeStamp >= 1000) {
    fpsTimeStamp = millis();
    Serial.printf("\nFPS:%d", fps);
    // Serial.printf("\nFPS:%d\tCPU:%f\tMEM:%f", fps, AudioProcessorUsageMax(), AudioMemoryUsageMax());
    // AudioProcessorUsageMaxReset();
    // AudioMemoryUsageMaxReset();
    fps = 0;
  };
  fps++;
  #endif
};
