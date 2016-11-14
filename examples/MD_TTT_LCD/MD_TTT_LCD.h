// Header file for MD_TTT_LCD.ino
//
#ifndef MD_TTT_LCD_H
#define MD_TTT_LCD_H

// Miscellaneous defines
#define	USER_MESSAGE(r, s)	{ lcd.setCursor(7,(r)); lcd.print(F(s)); }
#define	FLASH_DELAY		300		// milliseconds
#define	FLASH_REPEAT	3		// number of flashes

// Enumerated types
enum gameState	{ gStart, gGetMove, gCheckEnd };
enum uiState	{ uiStart, uiHilight, uiSelect, uiNextHilight, uiAccept };

// Coordinates for a board position
typedef struct 
{
  uint8_t	row;
  uint8_t	col;
} boardCoord;

// Switch status
typedef struct
{
	uint8_t		pin;
	bool 		lastState;
	uint32_t	lastCheckTime;
} swState;

// lcd user defined character definitions
uint8_t sep_7[8] =
{
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b11111
};

uint8_t p0_0[8] = 
{
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b11111
};

uint8_t p1_1[8] = 
{
    0b01110,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b01110,
    0b00000,
    0b11111
};

uint8_t p1_4[8] = 
{
    0b01110,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b01110,
    0b00000,
    0b00000
};

uint8_t p2_2[8] = 
{
    0b10001,
    0b01010,
    0b00100,
    0b00100,
    0b01010,
    0b10001,
    0b00000,
    0b11111
};

uint8_t p2_5[8] = 
{
    0b10001,
    0b01010,
    0b00100,
    0b00100,
    0b01010,
    0b10001,
    0b00000,
    0b00000
};


#endif