#include <stdio.h>
#include "pico/stdlib.h"
#include "pin_config.h"
#include "ssd1306.h"

int main()
{
    stdio_init_all();
    
    SSD1306 oled(OLED_RES, OLED_DC, OLED_CS);
    oled.init();

    uint8_t buf[128];
    for (int i = 0; i < 128; i++) {
        buf[i] = 0x55;
    }
    oled.send_data(buf,128);
    while(1) {
        sleep_ms(100);
        uint8_t buf[128];
        for (int i = 0; i < 128; i++) {
            buf[i] = 0xAA;
        }
        oled.send_data(buf,128);
        oled.clear(0,4,50,7);
        oled.blit(buf,30,5,46,6);
        sleep_ms(1200);
        puts("Running");
    }
    return 0;
}
