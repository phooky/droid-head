#include <stddef.h>
#include "hardware/spi.h"

class SSD1306 {
 private:
    uint8_t res, dc, cs;
    void send_spi(const uint8_t* buf, size_t len);
 public:
    /// Constructor
    SSD1306(uint8_t res_i, uint8_t dc_i, uint8_t cs_i) :
        res(res_i), dc(dc_i), cs(cs_i) {}

    /// The init function sets up the pins and puts the SSD1306 into reset.
    /// Initialization should happen before any other function is called.
    void init();

    /// Send a command buffer
    void send_cmds(const uint8_t* buf, size_t len);
    /// Send a data buffer
    void send_data(const uint8_t* buf, size_t len);
    
    enum AddrMode {
        PAGE,
        HORIZONTAL,
        VERTICAL,
    };
    /// Reset the SSD1306 by toggling the reset pin.
    void reset();
    /// Turn the display on or off.
    void display_on(bool on);
    /// Invert the display.
    void invert(bool invert);
    /// Turn all the pixels on, or resume ordinary display mode.
    void all_on(bool all_on);
    /// Set the addressing mode
    void set_addr_mode(AddrMode mode);
};
