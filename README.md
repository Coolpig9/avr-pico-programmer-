# avr-pico-programmer

A programmer for the AVR architecture microcontroller using the Raspberry Pi Pico.

- Turns the Raspberry Pi Pico into an AVR programmer via SPI programming mode.
- Instructions are streamed using the serial port to the Raspberry Pi Pico via USB which handles the rest.
- Verifies flashed pages after programming to confirm a successful flash.
- alows for flashing with an AVR program embeded into a pico firmware,

Chips Tested:

- ATtiny85

## How to use

### Flashing firmware to AVR deices

Either use the compiled `.uf2` and use the node js program to flash selected firmware from PC
or compile it yourself with an embeded firmware using [Raspberry Pi Pico SDK for C/C++](https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html).

### Physical setup

- You simply need to connect all the SPI and other relavent pins from the Raspberry Pi Pico to the logic converter, and from the logic converter to the ATTiny.
- I created a schematic which demonstrates this with a Raspberry Pi Pico and the ATTiny84A, if you're using another AVR controller you should be able to infer which pins to connect from the relavent AVR datasheet:
  ![image](https://github.com/SpeedyCraftah/avr-pico-programmer/assets/45142584/598428f8-867c-4d9e-a480-de1b2b60a3f2)

  In case the schematic is unclear: VBUS-HV&VCC | GND-GND&GND | 3V3_OUT-LV | GP19-LV3-HV3-PA6 | GP18-LV2-HV2-PA4 | GP17-LV1-HV1-PB3 | GP16-LV4-HV4-PA5

### Programming

#### Node js client method

- The tool is designed so that you can program firmware using a serial port client, (like the one in the client folder)
- The client has a couple of flags that you can take advantage of:
  - -i PATH/TO/FIRM       | set the input firmware
  - -setPort COMPORTPATH  | set the com path (default is 3) (HINT: use device Manager on windows or `ls /dev/ttyUSB* /dev/ttyACM*` on linux to find one)
  - -makeFirmEmbeded      | makes embededFirm.h for use with the Embeded Firmware option
  - -h                    | desplays the help menu
- Alternatively, you could use a tool such as PuTTY and pasting in the hex of each individual byte of the firmware, each being followed by a space (" ")

## Motivation

- A while back I got hold of an ATTiny84A but had no idea how to program it, later realising I needed an Arduino or a dedicated AVR programmer, hence decided to try to program it via a Raspberry Pi Pico.
- I started looking through the [ATTiny datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/ATtiny24A-44A-84A-DataSheet-DS40002269A.pdf) and attempted to program it according to the serial programming section.
- It served as a fun project as I was interested in the AVR architecture and wanted to challenge myself by writing a programmer for it from scratch.

## Writing code for AVR

- This is a topic on it's own and may include a guide on programming the ATTiny84A controller as well as others, but in general you will need to install the AVR GCC compiler, AVR assembly compiler, as well as the headers for AVR programming.
- Due to the nature of how the AVRs registers and control flow works, you will likely not have to write any assembly and can write the whole thing in C as the AVR GCC compiler already handles the interrupt vector table and layout of your program automatically - all you need to include is a main function written by yourself.
- A sample script in C which toggles the `PA0` pin on and off every second using busy wait (quick rundown - DDRA=IO bank A, _BV(PA0)=Bit shift register key PA0 | basically all that is happening here is that the bit for register PA0 in bank A is being toggled on and off):
  
  ```c
  #include <avr/io.h>
  #include <util/delay.h>
  
  int main() {
      DDRA = _BV(PA0);
      while (1) {
          // Toggle port A.
          PORTA ^= _BV(PA0);
  
          // Busy wait 1 second.
          _delay_ms(1000);
      }
  
      return 0;
  }
  ```

- I then compiled and turned it into a `.bin` microcode binary file with the following bash sequence on Ubuntu (F_CPU is the clock speed of the controller in Hz required by the `delay.h` header):
  - `avr-gcc -DF_CPU=1000000 -mmcu=attiny84a -O2 test.c -o fw.elf`
  - `avr-objcopy -O binary fw.elf fw.bin`
  - To see and read the underlying microcode bytes:
    - `xxd -i fw.bin`

## How to prepare and flash Arduino code

1. Use clone repo (or save as zip and extract)
2. Go to Arduino IDE sketch --> export as binary
3. Go to the sketch directory and copy the .elf file from build to the client folder in clone
4. Use `avr-objcopy -O binary file.elf fw.bin` to convert it to a binary file
5. Use `node program.js` to flash the newly converted bin
