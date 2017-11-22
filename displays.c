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
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) |= (CTRL_SELECT | CTRL_E);
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR + 1)) = c;
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) &= ~CTRL_E;
    delay_250ns();
}

static unsigned char ascii_read_controller(void)
{
    unsigned char c;
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) |= (CTRL_SELECT | CTRL_E);
    delay_250ns(); /* min 360ns */
    delay_250ns();
    c = *((volatile unsigned char*)(GPIO_E + GPIO_ODR + 1));
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) &= ~CTRL_E;
    return c;
}

static void ascii_write_cmd(unsigned char cmd)
{
    // Rs=0, RW=0
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) &= ~(CTRL_RS | CTRL_RW);
    
    ascii_write_controller(cmd);
}

static void ascii_write_data(unsigned char data)
{
    // Rs=1, RW=0
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) &= ~CTRL_RW;
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) |= CTRL_RS;
    
    ascii_write_controller(data);
}

static unsigned char ascii_read_status(void)
{
    unsigned char rv;
    
    // Temporarily mark GPIO E hi as input
    *((volatile unsigned long*)(GPIO_E + GPIO_MODER)) = 0x00005555;
    
    // RS=0, RW=1
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) &= ~CTRL_RS;
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) |= CTRL_RW;
    
    rv = *((volatile unsigned char*)(GPIO_E + GPIO_ODR + 1));
    
    // Restore GPIO E hi as output
    *((volatile unsigned long*)(GPIO_E + GPIO_MODER)) = 0x55555555;
    
    return rv;
}

static unsigned char ascii_read_data(void)
{
    unsigned char rv;
    
    // Temporarily mark GPIO E hi as input
    *((volatile unsigned long*)(GPIO_E + GPIO_MODER)) = 0x00005555;
    
    // RS=1, RW=1
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) |= (CTRL_RW | CTRL_RS);
    
    rv = *((volatile unsigned char*)(GPIO_E + GPIO_ODR + 1));
    
    // Restore GPIO E hi as output
    *((volatile unsigned long*)(GPIO_E + GPIO_MODER)) = 0x55555555;
    
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
    *((volatile unsigned long*)(GPIO_E + GPIO_MODER)) = 0x55555555;
    
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

#define CTRL_CS1 (8)
#define CTRL_CS2 (0x10)
#define CTRL_RESET (0x20)
#define CTRL_CS_BOTH (CTRL_CS1|CTRL_CS2)
#define LCD_CMD_ON (0x20|0x10|8|4|2|1)
#define LCD_CMD_OFF (0x20|0x10|8|4|2)
#define LCD_CMD_SET_ADDR (0x40)
#define LCD_CMD_SET_PAGE (0x80|0x20|0x10|8)
#define LCD_CMD_SET_START_ADDR (0x80|0x40)

// Display is 128x64; a byte in the buffer represents 8 vertical pixels
// XXX: We should probably have some dynamic memory so that applications
// not making use of an LCD display need not resever 1 KiB of memory
static unsigned char lcd_buffer[128][8];
// Each bit signifies one byte-row in the Y-axis; with LSB being Y=0, and MSB Y=7
// If that group of 8 pixels need to be repainted it's 1, otherwise 0
static unsigned char lcd_updatemask[128];

// Busily (spin) wait for LCD to finish its operation
static void lcd_busy_wait(unsigned char cs)
{
    unsigned char data;
    
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) = 0; // Clear CTRL bit; E=0, SELECT=0 esp.
    
    *((volatile unsigned long*)(GPIO_E + GPIO_MODER)) = 0x00005555; // Hi byte input, lo byte output
    
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) = CTRL_RW | cs; // RW=1, ChipSelect
    delay_500ns();
    
    while (1)
    {
        *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) |= CTRL_E; // E=1
        delay_500ns();
        
        data = *((volatile unsigned char*)(GPIO_E + GPIO_IDR + 1));
        if ((data & 0x80) == 0)
            break;
            
        *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) &= ~CTRL_E; // E=0
        delay_500ns();
    }
    
    *((volatile unsigned long*)(GPIO_E + GPIO_MODER)) = 0x55555555;
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) |= CTRL_E; // E=1
}

static void _lcd_write(unsigned char dataByte, unsigned char cs, int rs)
{
    // Setup control register
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) = cs | (rs ? CTRL_RS : 0); // RS=rs, ChipSelect, SELECT=0, RW=0, etc
    
    // Write data
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR + 1)) = dataByte;
    delay_500ns();
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) |= CTRL_E; // E=1
    delay_500ns();
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) &= ~CTRL_E; // E=0
    
    // Wait for write to complete
    if (cs & CTRL_CS1)
        lcd_busy_wait(CTRL_CS1);
    if (cs & CTRL_CS2)
        lcd_busy_wait(CTRL_CS2);
    
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR + 1)) = 0;   // data = 0
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) |= CTRL_E; // E=1
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) &= ~CTRL_CS_BOTH; // CS1=0,CS2=0
}

static void lcd_write(unsigned char dataByte, unsigned char cs)
{
    _lcd_write(dataByte, cs, 1);
}

static void lcd_command(unsigned char cmd, unsigned char cs)
{
    _lcd_write(cmd, cs, 0);
}

void lcd_init(void)
{
    *((volatile unsigned long*)(GPIO_E + GPIO_MODER)) = 0x55555555;
    *((volatile unsigned long*)(GPIO_E + GPIO_OTYPER)) = 0;
    *((volatile unsigned long*)(GPIO_E + GPIO_OSPEEDR)) = 0x55555555;
    *((volatile unsigned long*)(GPIO_E + GPIO_PUPDR)) = 0x55550000;
    
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) = CTRL_E; // E=1
    delay_micro(10);
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) = 0;
    delay_milli(30);
    *((volatile unsigned char*)(GPIO_E + GPIO_ODR)) = CTRL_RESET;
    delay_milli(100);
    
    lcd_command(LCD_CMD_OFF, CTRL_CS_BOTH);
    lcd_command(LCD_CMD_ON, CTRL_CS_BOTH);
    lcd_command(LCD_CMD_SET_START_ADDR | 0, CTRL_CS_BOTH);
    lcd_command(LCD_CMD_SET_ADDR | 0, CTRL_CS_BOTH);
    lcd_command(LCD_CMD_SET_PAGE | 0, CTRL_CS_BOTH);
}

void lcd_clear(void)
{
    for (int y = 0; y < 8; ++y)
    {
        lcd_command(LCD_CMD_SET_PAGE | y, CTRL_CS_BOTH);
        for (int x = 0; x < 64; ++x)
        {
            lcd_buffer[x][y] = 0;
            lcd_buffer[x+64][y] = 0;
            if (y == 0)
                lcd_updatemask[x] = 0;
                
            lcd_write(0, CTRL_CS_BOTH); // NOTE: This write increases LCD's X by one
        }
    }
}

void lcd_draw(int x, int y)
{
    if (!(0 <= x && x < 128) || !(0 <= y && y < 64))
        return;
        
    if (lcd_buffer[x][y/8] & (1 << (y%8)))
        return; // Pixel already turned on
    
    lcd_buffer[x][y/8] |= 1 << (y%8); // Mark pixel as turned on
    lcd_updatemask[x] |= 1 << (y/8); // Mark byte as needing to paint
}

void lcd_erase(int x, int y)
{
    if (!(0 <= x && x < 128) || !(0 <= y && y < 64))
        return;
        
    if (!(lcd_buffer[x][y/8] & (1 << (y%8))))
        return; // Pixel already turned off
    
    lcd_buffer[x][y/8] &= ~(1 << (y%8)); // Mark pixel as turned on
    lcd_updatemask[x] |= 1 << (y/8); // Mark byte as needing to paint
}

void lcd_present(void)
{
    int cs = 0;
    for (int y = 0; y < 8; ++y)
    {
        lcd_command(LCD_CMD_SET_PAGE | y, CTRL_CS_BOTH);
        for (int x = 0; x < 128; ++x)
        {
            unsigned char um = lcd_updatemask[x];
            if ((lcd_updatemask[x] & (1 << y)) == 0)
                continue; // Need not update this byte
            
            cs = (x < 64) ? CTRL_CS1 : CTRL_CS2;
            
            lcd_command(LCD_CMD_SET_ADDR | x, cs);
            lcd_command(LCD_CMD_SET_PAGE | y, cs);
            lcd_write(lcd_buffer[x][y], cs);
        }
    }
}