#include <stdio.h>
#include <pico/stdlib.h>
#include <stdlib.h>
#include <hardware/spi.h>
#include "avrprog.h"
#include "embededFirm.h"
#include "main.h"

#define startButtonPin 0

uint8_t program_code[2000] = {};
char input[10] = {0};
bool verify = false;
bool embededFirm = true;

int main() {

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_init(startButtonPin);
    gpio_set_dir(startButtonPin, GPIO_IN);
    stdio_init_all();
    sleep_ms(1000);

    while(true){
        gpio_put(PICO_DEFAULT_LED_PIN,0);
        sleep_ms(500);
        enterStandby();
        int code_c = 0;
        if(!embededFirm){
            printf("Using Firmware From external Program");
            code_c = getFirmFromProg();
            printf("Program length: %d / first & last byte: %hhu %hhu\n", code_c, program_code[0], program_code[code_c - 1]);
        }else{
            printf("Using Embeded Firmware To Flash");
            printf("Program length: %d / first & last byte: %hhu %hhu\n", eByteAmt, eFirm[0], eFirm[eByteAmt - 1]);
        }
        
        if ((code_c % 2) != 0) {
            printf("program bytes are not a multiple of 2!\n");
            return 1;
        }

        avr_spi_init();
        avr_reset();

        if (!avr_enter_programming_mode()) {
            printf("failed to enter programming mode\n");
            return 1;
        }

        printf("Entered programming mode\n");

        avr_erase_memory();
        printf("Erased program memory successfully for programming\n");

        if(embededFirm){
            flashProgAndVerify(eFirm,eByteAmt,verify);
        }else{
            flashProgAndVerify(program_code,code_c,verify);
        }


        printf("Going back to standby mode for future flashes..\n");
        printf("FINISH");
    }
    

    return 0;
}

void enterStandby(){
    gpio_put(PICO_DEFAULT_LED_PIN,1);
    bool buttonState = true;
        
    gpio_pull_up(startButtonPin);
    while( buttonState == true){
        buttonState = gpio_get(startButtonPin);
        sleep_ms(10);
    }
    gpio_put(PICO_DEFAULT_LED_PIN,0);
    while( buttonState == false){
        buttonState = gpio_get(startButtonPin);
        sleep_ms(10);
    }
    gpio_put(PICO_DEFAULT_LED_PIN,1);
    bool flashFinished = false;
}

int getFirmFromProg(){
    int len = 0;
    int input_c = 0;
    while (true) { // read rom
        char c = getchar();

        // Tells the automatic programmer the programmer is ready.
        if (c == '?') {
            printf("READY");
        }

        if (!(c == 13 || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || c == ' ')) {
            continue;
        }

        if (c == 13) {
            input[input_c] = 0;
            break;
        }

        input[input_c] = c;

        if (input[input_c] == ' ') {
            input[input_c] = 0;

            // Readback the byte.
            printf("%s",input);

            // Convert from hex to byte.
            program_code[len] = strtol(input, NULL, 16);

            input_c = 0;
            len++;
        }

        else input_c++;
    }
    return len;
}
int flashDelay = 0;
bool flashProgAndVerify(uint8_t prog[2000],int len, bool doVerify){
    //full page
    for (int i = 0; i < len / 64; i++) {
        printf("Flashing page %d..\n", i);

        int page_offset = i * 64;

        for (int j = 0; j < 32; j++) {
            sleep_ms(flashDelay/2);
            gpio_put(PICO_DEFAULT_LED_PIN,0);
            int word_index = (j * 2) + page_offset;
            
            uint8_t high_byte = prog[word_index + 1];
            uint8_t low_byte = prog[word_index];

            avr_write_temporary_buffer(j, low_byte, high_byte);
            sleep_ms(flashDelay/2);
            gpio_put(PICO_DEFAULT_LED_PIN,1);
        }

        avr_flash_program_memory(i * 32);

        if(doVerify){
            printf("Verifying flash..\n");
            if (!avr_verify_program_memory_page(i * 32, (uint16_t *)prog+page_offset, 32)) {
                printf("Verification failed! Page has not been flashed correctly..\n");
                sleep_ms(100);
                return 0;
            } else printf("Page flash successfully verified!\n");
        }
    }
    int remaining_bytes = len % 64;
    int bytes_offset = len - remaining_bytes;

    // partial page
    printf("Flashing partial page..\n");

    for (int j = 0; j < remaining_bytes / 2; j++) {
        sleep_ms(flashDelay/2);
        gpio_put(PICO_DEFAULT_LED_PIN,0);
        int word_index = (j * 2) + bytes_offset;
        
        uint8_t high_byte = prog[word_index + 1];
        uint8_t low_byte = prog[word_index];

        avr_write_temporary_buffer(j, low_byte, high_byte);
        sleep_ms(flashDelay/2);
        gpio_put(PICO_DEFAULT_LED_PIN,1);
    }
    avr_flash_program_memory(bytes_offset / 2);
    
    
    
    if(doVerify){
        printf("Verifying flash..\n");
        if (!avr_verify_program_memory_page(bytes_offset / 2, (uint16_t *) prog + bytes_offset, remaining_bytes / 2)) {
            printf("Verification failed! Page has not been flashed correctly..\n");
            sleep_ms(100);
            return 0;
        } else printf("Page flash successfully verified!\n");

        printf("Verifying overall page flashes..\n");

        if (!avr_verify_program_memory_page(0, (uint16_t *) prog, len / 2)) {
            printf("Overall page verification failed! Pages have not been flashed correctly..\n");
            sleep_ms(100);
            return 0;
        } else printf("Overall page flash verification successful! Firmware has been flashed correctly\n");
        
    }
    return 1;
}