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

## AVR Code Compile Commands

This is the test firmware example code:
```C
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

- compiled by turned it into a `.bin` microcode binary file with the following bash sequence on Ubuntu (F_CPU is the clock speed of the controller in Hz required by the `delay.h` header):
  - `avr-gcc -DF_CPU=1000000 -mmcu=attiny84a -O2 test.c -o fw.elf`
  - `avr-objcopy -O binary fw.elf fw.bin`
  - To see and read the underlying microcode bytes:
    - `xxd -i fw.bin`

## How use code made using the Ardeunio IDE

1. Go to Arduino IDE sketch --> export as binary
2. Go to the sketch directory and Use `avr-objcopy -O binary program.elf fw.bin` to convert it to a binary file
3. Copy the .bin file from build to the client input folder
4. Now its ready to be used by the AVR pico Programing Client or to be embeded!

## Programming

### Using AVR pico Programing Client Method

if its your first time runing this, first use `npm install` in
terminal or cmd to install depencies

run by doing:
`node program.js`

- The tool is designed so that you can program firmware using a serial port client, (like the one in the client folder)
- The client has a couple of flags that you can take advantage of:
  - -i PATH/TO/FIRM       | set the input firmware
  - -setPort COMPORTPATH  | set the com path (default is 3)
  - -makeFirmEmbeded      | makes embededFirm.h for use with the Embeded Firmware option
  - -h                    | desplays the help menu
(HINT: use device Manager on windows or `ls /dev/ttyUSB* /dev/ttyACM*` on linux to find your COM ports)
if your on linux and your ports are in /dev/ you may need to run with `sudo`

- Alternatively, you could use a tool such as PuTTY and pasting in the hex of each individual byte of the firmware, each being followed by a space (" ")

### Using Embeded Program Method

Do you need to program a bunch of AVR micro controllers?? well look no further then the Embeded Program Method!
Compile your ARV code into the `.u2f` itself so you can flash while only needed to supply power to the pico!
to do this method simply replace the `embededFirm.h` in the pico folder with your own and recomile it.

```C
//embededFirm.h example
uint8_t eFirm[2000] = {
   0x23,0xc0,0x32,0xc0,0x31,0xc0, ...     
};
const int eByteAmt = 676;
```

`eFirm` is the program in hex bytes
`eByteAmt` is the amount of bytes in eFirm

You can also generate this file useing the `-makeFirmEmbeded` flag useing the [AVR pico Programing Client](#using-avr-pico-programing-client-method)

To recompile it I recomend using VScode's pico extention,

1. install Pico SDK extention --> [Pico sdk extention for VScode]()
2. install Cmake Tools extention --> [Cmake tools extention for VScode]()
3. click the pico icon on the right,
4. wait for it to load (sometimes it takes a bit)
5. hit import project
6. change directory to pico folder
7. check the "Enable CMake-Tools extension integration"
8. click compile project!

- if your on windows make sure you have pico-sdk and pico-extras
  - ${USERHOME}.pico-sdk/external/pico_sdk_import.cmake"
  - ${USERHOME}.pico-extras/external/pico_extras_import.cmake"

- and if your on linux make sure you have pico-sdk and pico-extras
  - /opt/pico-sdk/external/pico_sdk_import.cmake
  - /opt/pico-extras/external/pico_extras_import.cmake
or set the paths manully in the CMakeLists.txt

## Motivation

SpeedyCraftah's motivation for intialy making this project:

- A while back SpeedyCraftah got hold of an ATTiny84A but had no idea how to program it, later realising they needed an Arduino or a dedicated AVR programmer, hence decided to try to program it via a Raspberry Pi Pico.
- they started looking through the [ATTiny datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/ATtiny24A-44A-84A-DataSheet-DS40002269A.pdf) and attempted to program it according to the serial programming section.
- It served as a fun project as they was interested in the AVR architecture and wanted to challenge themself by writing a programmer for it from scratch.

My motivation for expanding it:

- I dont own an arduino of anykind and I need to make an auto ATtiny programmer as I soon will need to mass program like 500
- I bought picos as I had had experiance and figured that programing an attiny with one would be pretty easy, it wasnt.
- Then I found this and after some fanagling I got it working but it wasnt quite what I wanted yet so I chose to fork it and add the features I need myself

Thanks so much to the original creator SpeedyCraftah!

