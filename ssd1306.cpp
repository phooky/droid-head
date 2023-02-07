#include <stdint.h>
#include "ssd1306.h"
#include "ssd1306_cmds.h"
#include "pico/stdlib.h"
#include <stdio.h>
/// The init function sets up the pins and puts the SSD1306 into reset.
/// Initialization should happen before any other function is called.
void SSD1306::init() {
    const static uint8_t pins[] = {res, cs, dc};
    for (uint8_t pin : pins) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        printf("Setting pin %d to out\n",pin);
        gpio_put(pin, 0);
    }
    gpio_put(cs, 1);
    spi_init(spi0,1000000);
}

void SSD1306::configure() {
    
    const static uint8_t config_buf[] = {
        CMD_TURN_OFF,
        CMD_SET_ADDR_MODE, CMD_ADDR_MODE_HORIZ,
        CMD_SET_DISPLAY_START_LINE(0), //default
        CMD_SET_SEGMENT_REMAP_127,
        CMD_SET_MULTIPLEX_RATIO,63, // default
        CMD_SET_COM_SCAN_DIR(1),
        CMD_SET_DISPLAY_OFFSET,0, //default
        CMD_SET_COM_CONFIG, CMD_SET_COM_CONFIG_PARAM(false,true),
        CMD_SET_DIVIDE_AND_FREQ, 0x80, // default
        CMD_SET_PRECHARGE_PERIODS, 0xF1, 
        CMD_SET_VCOMH_DESELECT, 0x30,
        CMD_CONTRAST, 0xff,
        CMD_ILLUM_RAM,
        CMD_NORMAL_DISPLAY,
        CMD_CHARGE_PUMP, 0x14,
        CMD_TURN_ON,
    };
    send_cmds(config_buf, sizeof(config_buf));
    printf("send cmd buf len %d\n",sizeof(config_buf));
    gpio_put(dc,1);
}

void SSD1306::send_spi(const uint8_t* buf, size_t len) {
    gpio_put(cs, 1);
    gpio_put(cs, 0);
    spi_write_blocking(spi0, buf, len);
    gpio_put(cs, 1);
}

/// Send a command buffer
void SSD1306::send_cmds(const uint8_t* buf, size_t len) {
    gpio_put(cs, 1);
    gpio_put(dc, 0);
    for (size_t i = 0; i < len; i++) {
        send_spi(buf+i,1);
    }
}

/// Send a data buffer
void SSD1306::send_data(const uint8_t* buf, size_t len) {
    uint8_t cmds[] = { CMD_SET_ADDR_COL_RANGE, 0, 127, CMD_SET_ADDR_PAGE_RANGE, 0, 7 };
    send_cmds(cmds, sizeof(cmds));
    gpio_put(cs, 1);
    gpio_put(dc, 1);
    uint8_t buffo[10] = { 1,2,3,4,5,6,7,8,9,10 };
    for (int i =0; i < 100; i++) {
        send_spi(buffo,10);
    }
    //send_spi(buf, len);
}
    
/// Reset the SSD1306 by toggling the reset pin.
void SSD1306::reset() {
    gpio_put(res, 0);
    sleep_ms(10); // As per spec
    gpio_put(res, 1); // come out of reset.
    sleep_ms(1);
}

/// Turn the display on or off.
void SSD1306::display_on(bool on) {
    uint8_t v = on?CMD_TURN_ON:CMD_TURN_OFF;
    send_cmds(&v, 1);
}

/// Invert the display.
void SSD1306::invert(bool invert) {
    uint8_t v = invert?CMD_INVERSE_DISPLAY:CMD_NORMAL_DISPLAY;
    send_cmds(&v,1);
}

/// Turn all the pixels on, or resume ordinary display mode.
void SSD1306::all_on(bool all_on) {
    uint8_t v = all_on?CMD_ILLUM_ALL:CMD_ILLUM_RAM;
    send_cmds(&v,1);
}

/// Set the addressing mode
void SSD1306::set_addr_mode(AddrMode mode) {
    uint8_t buf[] = { CMD_SET_ADDR_MODE, 0 };
    switch(mode) {
    HORIZONTAL: buf[1] = CMD_ADDR_MODE_HORIZ; break;
    VERTICAL: buf[1] = CMD_ADDR_MODE_VERT; break;
    PAGE: buf[1] = CMD_ADDR_MODE_PAGE; break;
    }
    send_cmds(buf,2);
}

    
