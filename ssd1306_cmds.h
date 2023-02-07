/// A brief description of the pins, from the SSD1306 datasheet:
///  - D0 is serial clock input (SCK)
///  - D1 is serial data input (SDI)
///  - RES is active-low reset. Hold high for regular operation.
///  - DC is Data/Command. High indicates data transfer, low indicates command transfer.
///  - CS is the chip select line (CS), active low.

/// The SCK, SDI, CS, and DC lines together form a "4-wire SPI" connection.
/// Data is shifted in, MSB first, to an 8-bit register on every rising edge of D0.
/// (It looks like the DC pin only needs to be valid when the final bit is shifted in.)

/// Power-up and down:
/// Start by pulling RES low for a minimum of 3uS.
/// Send ON command and wait 100mS.


/// *** Basic commands: We're just grabbing the ones that are going to be useful for now.

/// Turn on the display.
#define CMD_TURN_ON          0xAF
/// Turn off the display.
#define CMD_TURN_OFF         0xAE

/// Use normal display mode (set bits illuminate pixels) (default)
#define CMD_NORMAL_DISPLAY   0xA6
/// Use inverse display mode (clear bits illuminate pixels)
#define CMD_INVERSE_DISPLAY  0xA7

/// Illuminate all pixels based on RAM contents (default)
#define CMD_ILLUM_RAM        0xA4
/// Illuminate all pixels regardless of RAM contents
#define CMD_ILLUM_ALL        0xA5

/// Set the contrast. The following byte is an 8-bit contrast value.
#define CMD_CONTRAST         0x81

/// *** Scrolling commands:
/// I'm not going to support these yet but they look cool.


/// *** Adressing commands

/// Memory is arranged as 8-bit-deep "pages" of 128 columns.

/// ADDRESSING MODE
/// Set the addressing mode. Followed immediately by one of the modes specified below.
#define CMD_SET_ADDR_MODE    0x20
/// Horizontal addressing mode. Writes cycle column by column across each page,
/// and then to the next page after the last column. Writes loop back to the first page
/// after the end of the last page.
#define CMD_ADDR_MODE_HORIZ  0x00
/// Vertical addressing mode. Writes cycle from page to page and then wrap to the next
/// column.
#define CMD_ADDR_MODE_VERT   0x01
/// Page addressing mode (default). In page addressing mode, writes cycle endlessly
/// through a single page until the next page is explicitly set.
#define CMD_ADDR_MODE_PAGE   0x02

/// [PAGE ADDRESSING MODE ONLY]
/// Set the low nibble of the column address.
#define CMD_SET_ADDR_COL_LO(nibble) (0x00 | (nibble & 0x0F))
/// Set the high nibble of the column address.
#define CMD_SET_ADDR_COL_HI(nibble) (0x10 | (nibble & 0x0F))
/// Set the page address. The 3-bit page address is ORd with the command.
#define CMD_SET_ADDR_PAGE   0xB0

/// [HORIZONTAL AND VERTICAL ADDRESSING MODE ONLY]
/// Command for setting the start/end column addresses. This command is followed by
/// two further bytes, one specifying the start, one specifying the end.
#define CMD_SET_ADDR_COL_RANGE 0x21
/// Command for setting the start/end page addresses. This command is followed by
/// two further bytes, one specifying the start, one specifying the end.
#define CMD_SET_ADDR_PAGE_RANGE 0x22

/// *** Hardware configuration commands

/// Configure the RAM display start line address. Default is 0 on reset.
#define CMD_SET_DISPLAY_START_LINE(line) (0x40 | (line & 0x3F))
/// Configure the segment address remapping to map address 0 to SEG0 (default)
#define CMD_SET_SEGMENT_REMAP_0    0xA0
/// Configure the segment address remapping to map address 127 to SEG0
#define CMD_SET_SEGMENT_REMAP_127  0xA1
/// Configure the multiplex ratio. What's the multiplex ratio? Nobody seems to say exactly, but
/// it's clearly the display height. Followed by a single byte 6-bit value indicating the MUX - 1.
/// Defaults to 63 (a MUX/display height of 64).
#define CMD_SET_MULTIPLEX_RATIO 0xA8
/// Configure the common line scan direction. 0 is ascending, nonzero is descending. Defaults to 0 on reset.
#define CMD_SET_COM_SCAN_DIR(dir) ((dir == 0)?0xC0:0xC8)
/// Configure the vertical display offset. The value is given in a 6-bit value in the following byte.
/// Defaults to 0 on reset.
#define CMD_SET_DISPLAY_OFFSET 0xD3
/// Configure the COM pin layout. The second byte should be the COM pin parameter.
#define CMD_SET_COM_CONFIG 0xDA
/// Seq value indicates sequential or alternative com pin configuration,
/// flip indicates left/right remapping. Seq defaults to 1, flip to 0.
#define CMD_SET_COM_CONFIG_PARAM(seq,flip) (0x02 | (seq?0x20:0) | (flip?0x10:0))

/// *** Timing and driving commands

/// Set the display clock divider and oscillator frequency. The next byte is the argument; the high
/// 4 bits are the frequency adjustment, and the low four bits plus one set the divider value.
/// Default is 0x80 (frequency in middle of range; divider at 1).
#define CMD_SET_DIVIDE_AND_FREQ 0xD5
/// Set the pre-charge periods for phase 1 and 2. The next byte is the argument.
/// The low 4 bits of the argument are the DCLK precharge periods for
/// phase 1; the high 4 bits are the DCLK precharge periods for phase 2. 0 values are invalid.
/// The default value for both is 2.
#define CMD_SET_PRECHARGE_PERIODS 0xD9

/// Set the deselected voltage of the COM lines. I think? I'm hazy here.
/// Followed by a single byte that can be 0 (0.65*VCC), 0x20 (0.77*VCC), or 0x30 (0.83*VCC).
/// Defaults to 0x20 (0.77*VCC).
#define CMD_SET_VCOMH_DESELECT 0xDB

/// *** Charge pump enabling
/// Set the charge pump enabled value. Requires a one-byte parameter.
#define CMD_CHARGE_PUMP 0x8D
#define CMD_CHARGE_PUMP_PARAM(enabled) (0x10 | (enabled?0x04:0))
