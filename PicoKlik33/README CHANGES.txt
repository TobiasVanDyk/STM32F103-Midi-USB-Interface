
Earlier changes (as on first Github) search for *NEW*
Later changes search for #NEW#

Later changes corrected the no MidiUSB cable connected i.e. standalone mode but did not include the Dec2023 midi descriptor changes


MidiKlik source code changes:
(1) Addition of a four SysEx F0 77 77 78 E0 04,05,06,07 00 00 F7 to mod_intsysex.h
(2) Change all default routing to off (0) in  UsbMidiKliK4x4.ino except shift cables one jackout position
(3) Update Dec 2024 changes to correct no USB Midi cable detect in  UsbMidiKliK4x4.ino
(4) //#pragma message(__VAR_NAME_VALUE(HARDWARE_TYPE)) in hardware_config.h 
(5) #define DEFAULT_ITHRU_USB_IDLE_TIME_PERIOD 1 in usbmidiklik4x4.h  
(6) #define USB_MIDI_PRODUCT_STRING "MidiKlik 5x" in usb_midi_device.h
                       
Using library WireSlave at version 1.0 in folder: C:\Users\Tobias\Documents\Arduino\hardware\Arduino_STM32\STM32F1\libraries\WireSlave 
Using library midiXparser in folder: C:\Users\Tobias\Documents\Arduino\libraries\midiXparser (legacy)
"C:\\Users\\Tobias\\AppData\\Local\\Arduino15\\packages\\arduino\\tools\\arm-none-eabi-gcc\\4.8.3-2014q1/bin/arm-none-eabi-size" -A "I:\\Data\\Win10LTSC3\\Arduino/UsbMidiKliK4x4.ino.elf"
Sketch uses 54356 bytes (82%) of program storage space. Maximum is 65536 bytes.
Global variables use 6176 bytes (30%) of dynamic memory, leaving 14304 bytes for local variables. Maximum is 20480 bytes.


