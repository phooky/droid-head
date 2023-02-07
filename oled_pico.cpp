#include <stdio.h>
#include "pico/stdlib.h"
#include "pin_config.h"
#include "ssd1306.h"

int main()
{
    stdio_init_all();
    sleep_ms(3000);
    puts("Hello, world!");

    SSD1306 oled(OLED_D0, OLED_D1, OLED_RES, OLED_DC, OLED_CS);
    oled.init();
    sleep_ms(10);
    oled.reset();
    oled.configure();
    //oled.display_on(true);
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
        
        sleep_ms(1200);
        puts("Running");
    }
    return 0;
}
