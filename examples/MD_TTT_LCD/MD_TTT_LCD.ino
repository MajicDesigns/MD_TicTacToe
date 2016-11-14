// Tic Tac Toe using serial LCD display
//
// Output using an LCD with at least 4 lines and 20 characters 
// with the current configuration of messages.
// Input using switches - select and accept. The select switch 
// is used to progress from one empty board cell to another. 
// The accept switch will make the move into the currently selected 
// cell.
//
// Game play using the MD_TTT library and non-blocking user input 
// so we can do other stuff!
//
// LCD libraries used in this example are fm version of the 
// libraries found at 
// https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
//
#include <LCD.h>
#include <LiquidCrystal_SR.h>
#include <MD_TTT.h>
#include "MD_TTT_LCD.h"

// User switches for gameplay
#define	SWITCH_SELECT	3	// pin for the switch that moves from one selection to another
#define	SWITCH_ACCEPT	2	// pin for the switch that accepts the current selection	

// LCD display definitions
#define	LCD_CLK_PIN	7
#define	LCD_DTA_PIN	8
#define	LCD_ROWS	4
#define	LCD_COLS	20

// Define the custom chars used for the game
// first char is for the first 2 lines, last one for the last line
char	P0token[] = { 0, ' ' };
char	P1token[] = { 1, 4 }; 
char	P2token[] = { 2, 5 };
char	separator[] = { 7, 0x7c };

// Coordinate positions for the board on the LCD
boardCoord movePos[TTT_BOARD_SIZE] = 
{
	{ 0, 0 }, { 0, 2 }, { 0, 4 },
	{ 1, 0 }, { 1, 2 }, { 1, 4 },
	{ 2, 0 }, { 2, 2 }, { 2, 4 }
};

// Handling for switch states (User Input)
swState	swAccept = { SWITCH_ACCEPT, false, 0 };
swState	swSelect = { SWITCH_SELECT, false, 0 };

// Main objects used defined here
LiquidCrystal_SR lcd(LCD_DTA_PIN, LCD_CLK_PIN);
MD_TTT	TTT(tttCallback);

int8_t	curPlayer = TTT_P2;
bool	inGamePlay = false;

void setup() 
{
  // initialise switch pins for input
  pinMode(swAccept.pin, INPUT);
  pinMode(swSelect.pin, INPUT);

  // initialise LCD display
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.clear();
  lcd.noCursor();
  lcd.print(F("TTT LCD Example"));

  // Load characters to the LCD
  lcd.createChar(P0token[0], p0_0);
  lcd.createChar(P1token[0], p1_1);
  lcd.createChar(P1token[1], p1_4);
  lcd.createChar(P2token[0], p2_2);
  lcd.createChar(P2token[1], p2_5);
  lcd.createChar(separator[0], sep_7);

  TTT.setAutoPlayer(curPlayer);

  delay(1000);	//allow time to display
}

bool detectSwitch(swState *ss)
// detects the LOW to HIGH transition of a switch
// returns true if a transition has occurred
// only check if a period of time has expired to debounce
{
	boolean b = false;

	if ((millis() - ss->lastCheckTime) > 50)
	{
		bool curState = (digitalRead(ss->pin) == HIGH);

		ss->lastCheckTime = millis();
		b = (curState && !ss->lastState);
		ss->lastState = curState;
	}

	return(b);
}

uint8_t getMove(void)
// get the next move from the player
// there may not be a move there so we need to split the 
// function into a prompting and then checking phase
// return 0xff of no move entered
{
	static uiState	promptMode = uiStart;
	static uint8_t	curPosUI;

	uint8_t	m = 0xff;

	switch (promptMode)
	{
	case uiStart:	// print the message
		USER_MESSAGE(1, "Your move?");
		promptMode = uiHilight;
		break;

	case uiHilight:	// find the first empty cell and highlight it
		for (curPosUI=0; curPosUI<TTT_BOARD_SIZE; curPosUI++)
		{
			if (TTT.getBoardPosition(curPosUI) == TTT_P0)
			{
				lcd.setCursor(movePos[curPosUI].col, movePos[curPosUI].row);
				break;
			}
		}

		lcd.blink();
		promptMode = uiSelect;
		break;

	case uiSelect: // highlight the cell we are on and handle switches
		if (detectSwitch(&swSelect))
			promptMode = uiNextHilight;
		else if (detectSwitch(&swAccept))
			promptMode = uiAccept;
		break;

	case uiNextHilight: // user selected next cell, find it and highlight it
		for (curPosUI=curPosUI+1; curPosUI<TTT_BOARD_SIZE; curPosUI++)
		{
			if (TTT.getBoardPosition(curPosUI) == TTT_P0)
			{
				lcd.setCursor(movePos[curPosUI].col, movePos[curPosUI].row);
				break;
			}
		}

		if (curPosUI == TTT_BOARD_SIZE)	// oops - none there
			promptMode = uiHilight;	// start again
		else
			promptMode = uiSelect;	// wait for a switch again
		break;

	case uiAccept: // we have a selection, return the appropriate move
		m = curPosUI;
		lcd.noBlink();
		promptMode = uiStart;	// set up for next time
		break;

	default:	// catch all - reset
		promptMode = uiStart;
		break;
	} 

	return(m);
}

void tttCallback(uint8_t position, int8_t player)
{
	displayPosition(position, player);
}

void displayPosition(uint8_t pos, int8_t player)
// update the board position with the player token
{
	char	*p;

	// update the position on the grid
	lcd.setCursor(movePos[pos].col, movePos[pos].row);

	switch (player)
	{
	case TTT_P0: p = P0token;	break;
	case TTT_P1: p = P1token;	break;
	case TTT_P2: p = P2token;	break;
	}

	if (pos < 6)	// first 2 rows
		lcd.write(p[0]);
	else
		lcd.write(p[1]);
}

void displayGrid(void)
{
#define	SHOW_CHAR(r, c, ch)	{ lcd.setCursor(c, r); lcd.write((char)(ch));}
  lcd.clear();
  lcd.noDisplay();
  SHOW_CHAR(0, 1, separator[0]);   SHOW_CHAR(0, 3, separator[0]);
  SHOW_CHAR(1, 1, separator[0]);   SHOW_CHAR(1, 3, separator[0]);
  SHOW_CHAR(2, 1, separator[1]);   SHOW_CHAR(2, 3, separator[1]);
  lcd.display();
#undef SHOW_CHAR
}

void flashLine(uint8_t line)
// note this is blocking as it uses delay();
{
	uint8_t	l[3];

	// work out the cells for this line
	switch (line)
	{
	case TTT_WL_D1:	l[0]=0; l[1]=4; l[2]=8;	break;
	case TTT_WL_D2:	l[0]=2; l[1]=4; l[2]=6;	break;
	case TTT_WL_H1:	l[0]=0; l[1]=1; l[2]=2;	break;
	case TTT_WL_H2:	l[0]=3; l[1]=4; l[2]=5;	break;
	case TTT_WL_H3:	l[0]=6; l[1]=7; l[2]=8;	break;
	case TTT_WL_V1:	l[0]=0; l[1]=3; l[2]=6;	break;
	case TTT_WL_V2:	l[0]=1; l[1]=4; l[2]=7;	break;
	case TTT_WL_V3:	l[0]=2; l[1]=5; l[2]=8;	break;
	}

	// turn them off and on a number of times (flash!)
	for (uint8_t i=0; i<FLASH_REPEAT; i++)
	{
		lcd.noDisplay();
		for (uint8_t j=0; j<ARRAY_SIZE(l); j++)
			displayPosition(l[j], TTT_P0);
		lcd.display();
		delay(FLASH_DELAY);

		lcd.noDisplay();
		for (uint8_t j=0; j<ARRAY_SIZE(l); j++)
			displayPosition(l[j], TTT.getBoardPosition(l[j]));
		lcd.display();
		delay(FLASH_DELAY);
	}
}

void TTT_FSM() 
{
	static gameState curState = gStart;	// current state

	switch (curState)
	{
	case gStart:	// initialise for a new game
		displayGrid();
		inGamePlay = TTT.start();
		curState = gGetMove;
		break;

	case gGetMove:	// get and make player move - this section is non-blocking
		{
			uint8_t	m = 0;

			if (TTT.getAutoPlayer() != curPlayer)
				m = getMove();
			
			if (m != 0xff)
			{
				TTT.doMove(m, curPlayer);
				curState = gCheckEnd;
			}
		}
		break;

	case gCheckEnd:	// switch players and check if this is the end of the game
		if (TTT.isGameOver())
		{
			inGamePlay = false;

			USER_MESSAGE(0, "GAME OVER!!");
			if (TTT.getGameWinner() == TTT_P0)
			{
				USER_MESSAGE(1, "It's a draw.")
				delay(FLASH_REPEAT*2*FLASH_DELAY); // yes this blocks - so does flashLine()
			}
			else if (TTT.getGameWinner() == TTT.getAutoPlayer())
				USER_MESSAGE(1, "I win!    ")
			else
				USER_MESSAGE(1, "You win!  ")

			if (TTT.getGameWinner() != TTT_P0)	// not a draw
				flashLine(TTT.getWinLine());

			curState = gStart;	// restart
		}
		else
			curState = gGetMove;	// get or make next move

		// switch turns for players
		curPlayer = (curPlayer == TTT_P1 ? TTT_P2 : TTT_P1);
		break;

	default:
		curState = gStart;
		break;
	}
}

void loop(void)
{
	TTT_FSM();
}
