#include <stdint.h>
#include "ssd1306.h"
#include "ssd1306_cmds.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

/// The init function sets up the pins and puts the SSD1306 into reset.
/// Initialization should happen before any other function is called.
void SSD1306::init() {
    gpio_init(res);
    gpio_set_dir(res, GPIO_OUT);
    gpio_put(res, 1);

    gpio_init(dc);
    gpio_set_dir(dc, GPIO_OUT);
    gpio_put(dc, 0);

    gpio_init(cs);
    gpio_set_dir(cs, GPIO_OUT);
    gpio_put(cs, 1);

    spi_init(spi0,100000);
    gpio_set_function(18, GPIO_FUNC_SPI);
    gpio_set_function(19, GPIO_FUNC_SPI);

    sleep_ms(1);
    reset();
    
    const static uint8_t config_buf[] = {
        CMD_TURN_OFF,
        CMD_SET_ADDR_MODE, CMD_ADDR_MODE_HORIZ,
        CMD_SET_COM_SCAN_DIR(1),
        CMD_SET_COM_CONFIG, CMD_SET_COM_CONFIG_PARAM(false,true),
        CMD_SET_PRECHARGE_PERIODS, 0xF1, 
        CMD_SET_VCOMH_DESELECT, 0x30,
        CMD_CONTRAST, 0xff,
        CMD_CHARGE_PUMP, 0x14,
        CMD_TURN_ON,
    };
    send_cmds(config_buf, sizeof(config_buf));
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
    send_spi(buf,len);
}

/// Send a data buffer
void SSD1306::send_data(const uint8_t* buf, size_t len) {
    uint8_t cmds[] = { CMD_SET_ADDR_COL_RANGE, 0, 127, CMD_SET_ADDR_PAGE_RANGE, 0, 7 };
    send_cmds(cmds, sizeof(cmds));
    gpio_put(cs, 1);
    gpio_put(dc, 1);
    send_spi(buf, len);
}
    
/// Reset the SSD1306 by toggling the reset pin.
void SSD1306::reset() {
    gpio_put(res, 0);
    sleep_us(3); // As per spec
    gpio_put(res, 1); // come out of reset.
    sleep_us(1); // As per spec
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

    