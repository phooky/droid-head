

/// Pin numbers for the control lines

/// A brief description of the pins, from the SSD1306 datasheet:
///  - D0 is serial clock input (SCK)
///  - D1 is serial data input (SDI)
///  - RES is active-low reset. Hold high for regular operation.
///  - DC is Data/Command. High indicates data transfer, low indicates command transfer.
///  - CS is the chip select line (CS), active low.

/// The SCK, SDI, CS, and DC lines together form a "4-wire SPI" connection.
/// Data is shifted in, MSB first, to an 8-bit register on every rising edge of D0.
/// (It looks like the DC pin only needs to be valid when the final bit is shifted in.)


#define OLED_D0   11
#define OLED_D1   12
#define OLED_RES  13
#define OLED_DC   14
#define OLED_CS   15
