## STM32F103 Midi USB Interface based on Midiklik

This is a detailed description of how to compile and program an STM32F103C8T6 MidiUSBConverter with the code from [**TheKikGen USBMidiKliK4x4**](https://github.com/TheKikGen/USBMidiKliK4x4) as the instructions there are aimed at current users of the device, rather than a new user. *Midiklik 4x is an unusual MidiUSB interface because of the its extensive number of options via sysex commands.*

<p align="left">
<img src="images/midi4x4a.png" height="240" /> 
</p>

1. Install [**Arduino 1.8.19**](https://www.arduino.cc/en/software).

2. Download the [**MidiUSB4x4 repository**](https://github.com/TheKikGen/USBMidiKliK4x4) as a zip file and unzip it underneath your My Documents/Arduino/ folder. Rename the folder as UsbMidiKliK4x4 and you can then delete the bin and doc folders, and the .gitignore and README.md files. Replace three original files with the ones I have edited (usb_midi_device.h, hardware_config.h, UsbMidiKliK4x4.ino) - or do your own editing on the original files. Double click on UsbMidiKliK4x4.ino to open the Arduino IDE.

3. Download the two repositories [**midiXparser**](https://github.com/TheKikGen/midiXparser) and [**Pulseout**](https://github.com/TheKikGen/PulseOut) as zip files, and extract then underneath your My Documents/Arduino/libraries/ folder as midiXparser and Pulseout folders.

4. Install [**STM32Duino**](https://github.com/stm32duino/Arduino_Core_STM32) as follows:  Open the Arduino File->Preferences and put
``` 
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json 
``` 
in the additional boards section at the bottom - click [OK].

<p align="left">
<img src="images/image1.png" height="240" /> 
</p>
 
5. Click Tool->Boards->Boards Manager, scroll until STM32 MCU based boards and click [Install] and then [Close]. You do not need to do all the other steps [**here**](https://github.com/stm32duino/Arduino_Core_STM32/wiki/Getting-Started).

<p align="left">
<img src="images/image2.png" height="240" /> 
</p>

6. Install the Arduino SAM boards (Cortex-M3) board as explained [**here**](https://github.com/TheKikGen/USBMidiKliK4x4/wiki/Build-UsbMidiKlik4x4-from-sources) and [**here**](https://github.com/rogerclarkmelbourne/Arduino_STM32/wiki/Installation). Click [Install] and [Close].

<p align="left">
<img src="images/image3.png" height="240" /> 
</p>
 
7. Download zip file containing STM32 files from [**here**](https://github.com/rogerclarkmelbourne/Arduino_STM32/archive/refs/heads/master.zip).Make a hardware folder underneath your My Documents/Arduino/ folder and extract the Arduino_STM32 there. This will add the libmaple (modified) libraries originally from [**Leaflab Maple**](https://github.com/leaflabs/libmaple), which is used by the MidiUSB application.

<p align="left">
<img src="images/image4.png" height="240" /> 
</p>
 
Select your board as Generic STM32F103C series as board type and STM32F103C8 (20k RAM.64k Flash) as variant - see below.
 
<p align="left">
<img src="images/image5.jpg" height="240" /> 
</p>

Also in the Tool menu select:<br>
* "Faster -O2" as optimize option
* "72 Mhz" as CPU speed
* "STLink" as upload method 

<p align="left">
<img src="images/image6.jpg" height="240" /> 
</p>

8. Connect the STM32F103 to the STLink adapter (4 wires), and connect only the STLink to the computer USB.

9. Click Compile on the Arduino IDE and then Upload. The application is now on the Blue Pill but it still needs a bootloader installed.

10. Download the [**repository**](https://github.com/TheKikGen/stm32-tkg-hid-bootloader/tree/master/tools/tkg_hid_btl_uploader) and extract it as folder tkg_hid_btl_uploader underneath your
 My Documents/Arduino/ folder. Doubleclick on the tkg_hid_btl_uploader.ino file and comment out // --- TARGETS --#define GENERIC_PC13.

<p align="left">
<img src="images/image7.png" height="240" /> 
</p>
 
Click Compile on the Arduino IDE and then Upload. This should then upload the HID bootloader to the blue Pill still connected via the STLink adapter.

11. Close all Arduino windows and remove the STLInk from the Blue Pill. Plug it into the PC USB Port and check that a new sound device named Midiklik 4x is present. If not, try to install the bootloader differently, as in the next section by using a serial UART.

<p align="left">
<img src="images/image8.png" height="240" /> 
</p>
 
12. Download and install the [**STM32 Flash loader**](https://www.st.com/en/development-tools/flasher-stm32.html).

Also download the bootloader bin tkg_hid_generic_pc13.bin from the [**releases page**](https://github.com/TheKikGen/USBMidiKliK4x4/releases/tag/v2.5.1). 

Connect your sTM32F103 to a USB-Serial converter using only the V+ (5v or 3v3 but do connect it then to the right corresponding pins on the Blue Pill as well), Ground and TX-RX and Rx-Tx wires. Then run the Flash loader and select the ComXX port for the USBSerial converter and then follow the setup as below - but select the tkg_hid_generic_pc13.bin file.

<p align="left">
<img src="images/image9.jpg" height="240" /> 
</p>

Plug your Blue Pill it into the PC USB Port and check that a new sound device named Midi is present. You may want to install [**MidiOx**](http://www.midiox.com/) to check that all four input and four output Midi ports are available.

<p align="left">
<img src="images/image10.png" height="240" /> 
</p>
 

