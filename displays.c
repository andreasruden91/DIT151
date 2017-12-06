#include "displays.h"
#include "defines.h"
#include "timer.h"

static const unsigned char seg7[] =
{
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71
};

void seg7_init(void)
{
    *((volatile unsigned long*)(GPIO_D + GPIO_MODER)) &= 0xFFFF0000;
    *((volatile unsigned long*)(GPIO_D + GPIO_MODER)) |= 0x00005555;
}

void seg7_write(int out)
{
    *((volatile unsigned long*)(GPIO_D + GPIO_ODR)) &= 0xFFFFFF00;
    
    if (out < 0 || out > 0xF)
        return;
    
    *((volatile unsigned long*)(GPIO_D + GPIO_ODR)) |= seg7[out];
}

// Display status/command byte:
// bit 0: RS
// bit 1: R/W
// bit 2: SELECT (1 for ascii display, 0 for arbitrary display)
// bit 6: E
#define CTRL_RS (1)
#define CTRL_RW (2)
#define CTRL_SELECT (4)
#define CTRL_E (0x40)

static unsigned char ascii_read(int rs);

static void ascii_write_controller(unsigned char c)
{
    *portE_odr_lo |= (CTRL_SELECT | CTRL_E);
    *portE_odr_hi = c;
    *portE_odr_lo &= ~CTRL_E;
    delay_250ns();
}

static unsigned char ascii_read_controller(void)
{
    unsigned char c;
    *portE_odr_lo |= (CTRL_SELECT | CTRL_E);
    delay_250ns(); /* min 360ns */
    delay_250ns();
    c = *portE_odr_hi;
    *portE_odr_lo &= ~CTRL_E;
    return c;
}

static void ascii_write_cmd(unsigned char cmd)
{
    // Rs=0, RW=0
    *portE_odr_lo &= ~(CTRL_RS | CTRL_RW);
    
    ascii_write_controller(cmd);
}

static void ascii_write_data(unsigned char data)
{
    // Rs=1, RW=0
    *portE_odr_lo &= ~CTRL_RW;
    *portE_odr_lo |= CTRL_RS;
    
    ascii_write_controller(data);
}

static unsigned char ascii_read_status(void)
{
    unsigned char rv;
    
    // Temporarily mark GPIO E hi as input
    *portE_moder = 0x00005555;
    
    // RS=0, RW=1
    *portE_odr_lo &= ~CTRL_RS;
    *portE_odr_lo |= CTRL_RW;
    
    rv = *portE_odr_hi;
    
    // Restore GPIO E hi as output
    *portE_moder = 0x55555555;
    
    return rv;
}

static unsigned char ascii_read_data(void)
{
    unsigned char rv;
    
    // Temporarily mark GPIO E hi as input
    *portE_moder = 0x00005555;
    
    // RS=1, RW=1
    *portE_odr_lo |= (CTRL_RW | CTRL_RS);
    
    rv = *portE_odr_hi;
    
    // Restore GPIO E hi as output
    *portE_moder = 0x55555555;
    
    return rv;
}

static void ascii_command(unsigned char cmd, int usDelay)
{
    while (ascii_read_status() & 0x80) /*wait*/;
    delay_micro(8);
    ascii_write_cmd(cmd);
    delay_micro(usDelay);
}

void ascii_init(void)
{
    *portE_moder = 0x55555555;
    
    // Function Set: 2 rows, 5x11 point size
    ascii_command(0x20 | 0x10 | 0x8 | 0x4, 39);
    
    // Display Control: Light up display and show cursor
    ascii_command(0x8 | 0x4 | 0x2, 39);
    
    // Clear Display
    ascii_command(0x1, 2000);
    
    // Entry Mode Set: Increment mode (shift off), direction right
    ascii_command(0x4 | 0x2, 39);
}

void ascii_gotoxy(int x, int y)
{
    int address;
    
    address = y * 0x40 + x;
    if (address >= 0x80)
        return;
    
    ascii_command(0x80 | (unsigned char)address, 39);
}

void ascii_write_char(char c)
{
    // while (ascii_read_status() & 0x80) /*wait*/;
    delay_micro(8);
    ascii_write_data(c);
    delay_micro(43);
}

#define B_CS1 (8)
#define B_CS2 (0x10)
#define CTRL_RESET (0x20)
#define CTRL_CS_BOTH (B_CS1|B_CS2)
#define LCD_CMD_ON 0x3F
#define LCD_CMD_OFF 0x3E
#define LCD_CMD_SET_ADDR 0x40
#define LCD_CMD_SET_PAGE 0xB8
#define LCD_CMD_SET_START_ADDR 0xC0

static void graphic_ctrl_bit_set(unsigned char c)
{
	*portE_odr_lo |= c;
}

static void graphic_ctrl_bit_clear(unsigned char c)
{
	*portE_odr_lo &= ~c;
}

static void select_controller(unsigned char cs)
{
	switch (cs)
	{
		case 0:
			graphic_ctrl_bit_clear(B_CS1);
			graphic_ctrl_bit_clear(B_CS2);
			break;
		case B_CS1:
			graphic_ctrl_bit_set(B_CS1);
			graphic_ctrl_bit_clear(B_CS2);
			break;
		case B_CS2:
			graphic_ctrl_bit_clear(B_CS1);
			graphic_ctrl_bit_set(B_CS2);
			break;
		case (B_CS1|B_CS2):
			graphic_ctrl_bit_set(B_CS1);
			graphic_ctrl_bit_set(B_CS2);
			break;
	}
}

// Busily (spin) wait for LCD to finish its operation
static void graphic_wait_ready()
{
    unsigned char data;

    graphic_ctrl_bit_clear(CTRL_E);
    *portE_moder = 0x00005555; // Hi byte input, lo byte output
	graphic_ctrl_bit_clear(CTRL_RS);
    graphic_ctrl_bit_set(CTRL_RW);
    delay_500ns();
    
    while (1)
    {
        graphic_ctrl_bit_set(CTRL_E);
        delay_500ns();
        
        data = *portE_idr_hi;
            
        graphic_ctrl_bit_clear(CTRL_E);
        delay_500ns();
		
		data = 0;
		if ((data & 0x80) == 0)
            break;
    }
    
    graphic_ctrl_bit_set(CTRL_E);
	*portE_moder = 0x55555555;
}

static void _lcd_write(unsigned char dataByte, unsigned char cs, int rs)
{
    // Setup control register
	// graphic_write_data/command
	*portE_odr_lo &= ~CTRL_E; // E=0
	select_controller(cs);
	if (rs)
		*portE_odr_lo |= CTRL_RS;
	else
		*portE_odr_lo &= ~CTRL_RS;
	*portE_odr_lo &= ~CTRL_RW; // rw=0
	// graphic write
	*portE_odr_hi = dataByte;
	select_controller(cs);
    
    // Write data
    //*portE_odr_hi = dataByte;
    delay_500ns();
    *portE_odr_lo |= CTRL_E; // E=1
    delay_500ns();
    *portE_odr_lo &= ~CTRL_E; // E=0
    
    // Wait for write to complete
    if (cs & B_CS1)
        graphic_wait_ready(B_CS1);
    if (cs & B_CS2)
        graphic_wait_ready(B_CS2);
    
    *portE_odr_hi = 0;   // data = 0
    *portE_odr_lo |= CTRL_E; // E=1
    *portE_odr_lo &= ~CTRL_CS_BOTH; // CS1=0,CS2=0
}

void lcd_write(unsigned char dataByte, unsigned char cs)
{
    _lcd_write(dataByte, cs, 1);
}

void lcd_command(unsigned char cmd, unsigned char cs)
{
    _lcd_write(cmd, cs, 0);
}

void lcd_init(void)
{
    *portE_moder = 0x55555555;
    *portE_otyper = 0;
    *portE_ospeedr = 0x55555555; /* medium speed */
    *portE_pupdr = 0x55550000; /* inputs are pull up */
    
	*portE_odr_lo &= ~CTRL_SELECT;
	
    *portE_odr_lo |= CTRL_E; // E=1
    delay_micro(10);
    *portE_odr_lo &= ~(CTRL_CS_BOTH|CTRL_RESET|CTRL_E);
    delay_milli(30);
    *portE_odr_lo |= CTRL_RESET;
    
    lcd_command(LCD_CMD_OFF, CTRL_CS_BOTH);
    lcd_command(LCD_CMD_ON, CTRL_CS_BOTH);
    lcd_command(LCD_CMD_SET_START_ADDR | 0, CTRL_CS_BOTH);
    lcd_command(LCD_CMD_SET_ADDR | 0, CTRL_CS_BOTH);
    lcd_command(LCD_CMD_SET_PAGE | 0, CTRL_CS_BOTH);
	select_controller(0);
}

void lcd_clear(void)
{
    for (int y = 0; y < 8; ++y)
    {
        lcd_command(LCD_CMD_SET_ADDR | 0, CTRL_CS_BOTH);
        lcd_command(LCD_CMD_SET_PAGE | y, CTRL_CS_BOTH);
        
        for (int x = 0; x < 64; ++x)
        {
            lcd_write(0, CTRL_CS_BOTH); // NOTE: This write increases LCD's X by one
        }
    }
}

static unsigned char graphic_read(unsigned char cs)
{
    unsigned char data;
    
	graphic_ctrl_bit_clear(CTRL_E);
    *portE_moder = 0x00005555; // Hi byte input, lo byte output
    
	graphic_ctrl_bit_set(CTRL_RS);
	graphic_ctrl_bit_set(CTRL_RW);
	select_controller(cs);
    delay_500ns();

    graphic_ctrl_bit_set(CTRL_E);
    delay_500ns();
    
    data = *portE_idr_hi;
    
    graphic_ctrl_bit_clear(CTRL_E);
    *portE_moder = 0x55555555; // Restore all to output
    
    if (cs & B_CS1)
        graphic_wait_ready(B_CS1);
    if (cs & B_CS2)
        graphic_wait_ready(B_CS2);

    return data;
}

static unsigned char lcd_read(unsigned char cs)
{
    // By the specs of the LCD: you need to do two reads to get the actual value;
    // the first one being a dummy read
    graphic_read(cs);
    return graphic_read(cs);
}

static void _lcd_pixel(int x, int y, int draw)
{
    int cs;
    unsigned char byte, bi;
    
    if ((!(0 <= x && x < 128)) || (!(0 <= y && y < 64)))
        return;
   
    cs = (x < 64) ? B_CS1 : B_CS2;
        
    bi = 1 << (y % 8);
	
	lcd_command(LCD_CMD_SET_ADDR | (x%64), cs);
    lcd_command(LCD_CMD_SET_PAGE | (y/8), cs);
    byte = lcd_read(cs); // This updates page to what we want (but not addr)
	lcd_command(LCD_CMD_SET_ADDR | (x%64), cs);

	if (draw)
		byte |= bi; // add our pixel
	else
		byte &= ~bi; // remove our pixel
	
	lcd_write(byte, cs);
	lcd_command(LCD_CMD_ON, CTRL_CS_BOTH);
    lcd_command(LCD_CMD_SET_START_ADDR | 0, CTRL_CS_BOTH);
}

void lcd_draw(int x, int y)
{
    _lcd_pixel(x, y, 1);
}

void lcd_erase(int x, int y)
{
    _lcd_pixel(x, y, 0);
}
