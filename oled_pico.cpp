#include <stdio.h>
#include "pico/stdlib.h"
#include "pin_config.h"
#include "ssd1306.h"

#include "font/oled_font.h"

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
    {
        uint8_t buf[128*8];
        for (int i = 0; i < 128*8; i++) {
            buf[i] = 0x0;
        }
        uint16_t idx = 0;
        const char* msg = "What a pleasant afternoon for UNSPANGLED FLANGES. Ampersand &, at @, ::colons:: and ;;semicolons;; interrogative? exclamation! _-+=$ ZEBULON VERGING very nice this goggle-eyed font, ajar, neue, lysergic, VYU, cannery, explosive";
        while (*msg != 0) {
            char c = *(msg++);
            uint8_t len = oled_font_f.get_length(c);
            const uint8_t* dat = oled_font_f.get_data(c);
            while (len-- > 0)
                if (idx < 128*8) 
                    buf[idx++] = *(dat++);
            idx++;
        }
        oled.clear();
        oled.send_data(buf,128*8);
        sleep_ms(1200);
        puts("Running");
    }
    while(1) {}
    return 0;
}
