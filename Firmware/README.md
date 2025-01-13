# eTextile-MIDI-interface
## Build your own eTextile-User-Interfaces to play music through textile

## Easy step-by-step User Guide
1. Poweirng the **eTextile-MIDI-interface**:
    - Use the included power cable with a **common 5V USB power supply**
    - Or use the same cable to plug the interface to your laptop USB port
2. For data communication between the eTextile-MIDI-interface and the web app or music software:
    - use both cables: the power cable and the Micro USB cable (Type B)
3. Load the provided web application (using Chome): https://mapping.etextile.org/
4. Build your custom Graphic-User-Interface using the web app features
5. The app contains GUI moduls like toggles, sliders, grids, etc.
6. Each one can be placed, resized and configured to target specific MIDI_PARAMETERS
    - Each modul is providing **Midi Note** or **Midi Control-Change**
7. The web app is made to generate and upload a **config file** to the eTextile-MIDI-interface
8. You can then save it to the flash memory to be **load on each startup**
    - Long Press on the LEFT_BUTTON (physical) will **save the config to the flash memory**
9. You can as well save it to your computer for arkive
10. Use MIDI cable (mini-jack-TRS-A) to connect the eTextile-MIDI-interface to external MIDI Synthesizers.

## BOOT SEQUENCE
### BOOTING WITH USB CABLE & POWER PLUG
1. When the eTextile-MIDI-interface is connected to the web app via Micro USB cable (type B) & power cable
2. The App will automatically detect the device an proceed with [SYNC_MODE_REQUEST]
3. The device answer [SYNC_MODE_DONE]
4. The App will proceed with [CONFIG_FILE_REQUEST]
5. The device can answer [FLASH_CONFIG_LOAD_DONE] or [FLASH_CONFIG_LOAD_FAILED]
    - if [FLASH_CONFIG_LOAD_DONE]
      - Right after the device will send it **current config file** using (MIDI_SYSTEM_EXCLUSIVE_MSG)
      - then the web app will load the config on the sceen
      - This config can be modified and re-uplod to the device, **erasing the previous config file** :-(
      - A config manager is would be cool :-)
    - if [FLASH_CONFIG_LOAD_FAILED]
      - The web app will prompt **NO CONFIG FILE IN THE DEVICE!**
      - you will then start from scratch

### BOOTING WITH POWER PLUG ONLY (NO USB CABLE!)
1. When powering the eTextile-MIDI-interface with the power plug (No USB)
2. The firmware boot in [PENDING_MODE]
3. If [PENDING_MODE_TIMEOUT] ocurre it switch to [STANDALONE_MODE]
4. The device proceed with **loaded_flash_config()**
    - If [FLASH_CONFIG_LOAD_DONE]: the **BUILTIN_LEDS** will blink **STANDALONE_MODE**
    - If [FLASH_CONFIG_LOAD_FAILED]: the **BUILTIN_LEDS** will blink **ERROR_CODE**
5. The eTextile-MIDI-interface with TUI mappings is now RUNING!
6. By using a mini-jack (TRS-A) on the MIDI-OUT socket your custom eTextile-MIDI-interface is now able to talk MIDI to your external MIDI SYNTH

## MAPPING LIB
### LIST OF AVAILABLE TUIs
- All feture are parametric
  - switchs()
  - sliders()
  - knobs()
  - polygons()
  - touchpads()
  - grids()

### Development Frameworks
- **eTextile-Synthesizer PCB** & **Teensy 4.0**
- **Arduino IDE**: Arduino 1.8.19 [DOWNLOAD](https://www.arduino.cc/en/Main/Software/)
- **Visual Studio Code**: [DOWNLOAD](https://visualstudio.microsoft.com/)
- **platform.io extension**: [DOWNLOAD](https://platform.io/)
- **platform.io additional board**: Teensy 4.0

### External Libraries
All Library dependencies are automatically installed using platformio.ini (lib_deps)
Path of the lib_deps installation : 
    /Synth/Firmware/.pio/libdeps/teensy40/...

### Firmware main file
    /Synth/Firmware/src/main.cpp

### Install Teensy platform in PlatformIO
1. Go to "PIO Home"
2. Click on "Platforms"
3. Click on "Advanced Installation"
4. Enter "https://github.com/nicolas-thill/platform-teensy-e256.git"
5. Click "Install"

### Powering the eTextile-Synthesizer
- The Teensy Micro USB Type B **is not powering** the eTextile-Synthesizer.
- You must use the included power cable with an **5Volts AC/DC power Plug** or your laptop USB port.

### Firmware Synopsis (Low level)
- **Force image acquisition** 16x16 using sinchronious dual analog read.
- **Bilinear interpolation** the sensor force image is interpolated to 64x64 using bilinear algorithm.
- **Touch biasing** adjustable threshold, depending on needed sensitivity.
- **Blob segmentation** the binary image is genarated using a flood fill algorithm.
- **Blob tracking** blob ID management (matching with previous frame)
- **Blob shape and movement characterisation**: blobs coordinates, size, pressure, velocity.
- **Tactile-User-Interface using Mapping-lib functionalitys**
    - MIDI over USB (USB cable type B)
    - MIDI over HARDWARE (mini-jack-TRS-A)

### Benchmark
  - ADC_INPUT : 2500 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION : ...
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING : 550 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING / AUDIO : ...

## Copyright
Except as otherwise noted, all files in the eTextile-Synthesizer project folder

    Copyright (c) 2014- Maurin Donneaud

For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see LICENSE.txt included in the eTextile-Synthesizer project folder.

## TODO
- **Gesture Recognizer** could be a nice feature that fit the eTextile-Synthesizer tactile surface.
- **MIDI MPE** : MIDI in general does not have continuous control of note parameters (pitch and velocity) after the note has been played. Some companies like Roli with MPE have tried to address these limitations, but support is not as wide as standard control changes.
- **Teensy Synthesizer** / Open source
    - [MicroDexed](https://www.parasitstudio.de/)
    - [Dexed - FM Plugin Synth](https://github.com/asb2m10/dexed)
