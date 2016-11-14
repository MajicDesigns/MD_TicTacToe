// Tic Tac Toe using Graphics LCD
//
// Output using an GLCD, input using software buttons on the touchscreen.
//
// Game play using the MD_TTT library and non-blocking user input 
// so we can do other stuff!
//
// GLCD libraries used in this example are UTFT libraries found at 
// http://henningkarlsen.com/electronics/library.php?id=51
//
// GUI library can be found at
// http://code.google.com/p/arduino-code-repository/
//
#include <UTFT.h>
#include <UTouch.h>
#include <MD_GUI.h>
#include <MD_TTT.h>
#include "MD_TTT_GLCD.h"

// Miscellaneous defines
#define	GRIDWIDTH		6
#define	ORIENTATION		LANDSCAPE
#define	GRID_COL		VGA_LIME
#define	AUTO_SEL_COL	VGA_AQUA
#define	TOKEN_NORMAL_COL	VGA_WHITE
#define	TOKEN_STRIKE_COL	VGA_RED
#define	BLANK_MESG		"           "

// Declare which fonts we will be using
extern uint8_t SmallFont[];

// Main objects used defined here
// Remember to change the model parameter to suit your display module!
UTFT	myGLCD(INFINIT32, A1, A2, A0, A3, A5);
UTouch	myTouch(13, 10, 11, 12, A4);

MD_TTT	TTT(tttCallback);
boardCoord	board[TTT_BOARD_SIZE];
MD_Button	*autoPlay[2];

int8_t	curPlayer = TTT_P2;
bool	inGamePlay = false;

char	token[] = {'O', ' ', 'X'};

// Messages area
uint16_t	mesgColumn;
uint16_t	mesgRow[2];

void setup() 
{
	MD_GUI	GUI;

	GUI.begin(&myGLCD, &myTouch);

	// initialise GLCD display
	myGLCD.InitLCD(ORIENTATION);
	myGLCD.setFont(SmallFont);
	myGLCD.clrScr();

	myTouch.InitTouch(ORIENTATION);
	myTouch.setPrecision(PREC_MEDIUM);

	// initialise the board positions based on the size of the board
	uint16_t  width = min(myGLCD.getDisplayYSize(),myGLCD.getDisplayXSize());

	width /= 3;	// cells across & down

	for (uint8_t i=0; i<3; i++)
		for (uint8_t j=0; j<3; j++)       
		{
			board[(i*3)+j].x1 = j*width;
			board[(i*3)+j].y1 = i*width; 
			board[(i*3)+j].x2 = (j*width)+width-GRIDWIDTH;
			board[(i*3)+j].y2 = (i*width)+width-GRIDWIDTH;
		}

	drawGrid();

	// Message area
#if (ORIENTATION == LANDSCAPE)
	mesgColumn = board[2].x2 + GRIDWIDTH;
	for (uint8_t i=0; i<ARRAY_SIZE(mesgRow); i++)
		mesgRow[i] = board[2].y1 + (myGLCD.getFontYsize()*i) + 3;
#else
	mesgColumn = board[6].x1 + GRIDWIDTH;
	for (uint8_t i=0; i<ARRAY_SIZE(mesgRow); i++)
		mesgRow[i] = board[6].y2 + (myGLCD.getFontYsize()*i) + 3;
#endif

	// Autoplayer Buttons
#if (ORIENTATION == LANDSCAPE)
	uint16_t	x1 = board[5].x2 + (width/3);
	uint16_t	y1 = board[5].y1 + (width/3);
#else
	uint16_t	x1 = board[7].x1 + width/3;
	uint16_t	y1 = board[7].y2 + (width/4);
#endif
	uint16_t	x2 = x1 + (width/2);
	uint16_t	y2 = y1 + (width/2);

	autoPlay[0] = new MD_Button(TTT_P2, x1, y1, x2, y2, "O");
	autoPlay[0]->widBorder = 3;
	if (autoPlay[0]->id == curPlayer)
		autoPlay[0]->colFace = AUTO_SEL_COL;
	autoPlay[0]->refresh();

#if (ORIENTATION == LANDSCAPE)
	autoPlay[1] = new MD_Button(TTT_P1, x1, y1+(width/2), x2, y2+(width/2), "X");
#else
	autoPlay[1] = new MD_Button(TTT_P1, x1+(width/2), y1, x2+(width/2), y2, "X");
#endif
	autoPlay[1]->widBorder = 3;
	if (autoPlay[1]->id == curPlayer)
		autoPlay[1]->colFace = AUTO_SEL_COL;
	autoPlay[1]->refresh();

	myGLCD.setColor(TOKEN_NORMAL_COL);
	myGLCD.print("Auto", x1, y1-3-myGLCD.getFontYsize());
    
	TTT.setAutoPlayer(curPlayer);
}

void userMessage(uint8_t row, char *msg)
{
	myGLCD.setColor(VGA_WHITE);
	myGLCD.print(msg, mesgColumn, mesgRow[row]);
}

void drawGrid(void)
{
  myGLCD.setColor(GRID_COL);

  for (uint8_t i=0; i<GRIDWIDTH; i++)
  {
	  myGLCD.drawLine(board[0].x2+i, board[0].y1, board[6].x2+i, board[6].y2);	// vert line 1
	  myGLCD.drawLine(board[1].x2+i, board[1].y1, board[7].x2+i, board[7].y2);	// vert line 2
	  myGLCD.drawLine(board[0].x1, board[0].y2+i, board[2].x2, board[2].y2+i);	// horiz line 1
	  myGLCD.drawLine(board[3].x1, board[3].y2+i, board[5].x2, board[5].y2+i);	// horiz line 2
  }
}

void displayPosition(uint8_t pos, int8_t player, uint16_t col = TOKEN_NORMAL_COL)
// update the board position with the player token
{
	if (!inGamePlay)
		return;

    board[pos].button->enable = false;
	board[pos].button->colFace = VGA_BLACK;
	board[pos].button->widBorder = 0;
	board[pos].button->refresh();

	myGLCD.setColor(col);
	
	uint8_t	offset = (board[pos].x2 - board[pos].x1) / 10;

	switch (token[player+1])
	{
	case 'X': 
		{
			uint16_t	x1 = board[pos].x1+offset;
			uint16_t	y1 = board[pos].y1+offset;
			uint16_t	x2 = board[pos].x2-offset;
			uint16_t	y2 = board[pos].y2-offset;

			myGLCD.drawLine(x1, y1, x2, y2); 
			myGLCD.drawLine(x1+2, y1, x2, y2-2); 
			myGLCD.drawLine(x1, y1+2, x2-2, y2); 
			myGLCD.drawLine(x1, y2, x2, y1);
			myGLCD.drawLine(x1+2, y2, x2, y1+2);
			myGLCD.drawLine(x1, y2-2, x2-2, y1);
		}
		break;

	case 'O':
		{
			uint16_t	x = board[pos].x1 + (board[pos].x2-board[pos].x1)/2;
			uint16_t	y = board[pos].y1 + (board[pos].y2-board[pos].y1)/2;
			uint16_t	r = ((board[pos].y2-board[pos].y1)/2) - offset;

			myGLCD.drawCircle(x, y, r);
			myGLCD.drawCircle(x, y, r-1);
			myGLCD.drawCircle(x, y, r-2);
			myGLCD.drawCircle(x, y, r-3);
		}
		break;
	}
}

void strikeLine(uint8_t line, int8_t win)
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

	for (uint8_t i=0; i<3; i++)
		displayPosition(l[i], win, TOKEN_STRIKE_COL);

	delay(2000);
}

void createBoardButtons()
{
	for (uint8_t i=0; i<TTT_BOARD_SIZE; i++)
	{
		board[i].button = new MD_Button(i, board[i].x1, board[i].y1, board[i].x2, board[i].y2);
		board[i].button->refresh();
	}
}

void killBoardButtons()
{
	for (uint8_t i=0; i<TTT_BOARD_SIZE; i++)
	{
		delete board[i].button;
		board[i].button = NULL;
	}
}

void checkAutoPlay(void)
{
	uint8_t	to, from = 99;

	if (autoPlay[0]->isPressed() && (autoPlay[0]->id != TTT.getAutoPlayer()))
	{
		TTT.setAutoPlayer(autoPlay[0]->id);
		from = 1; 
		to = 0;
	}

	if (autoPlay[1]->isPressed() && (autoPlay[1]->id != TTT.getAutoPlayer()))
	{
		TTT.setAutoPlayer(autoPlay[1]->id);
		from = 0; 
		to = 1;
	}

	if (from != 99)
	{
		autoPlay[from]->colFace = autoPlay[to]->colFace;
		autoPlay[to]->colFace = AUTO_SEL_COL;
		autoPlay[0]->refresh();
		autoPlay[1]->refresh();
	}
}

uint8_t getMove(void)
// get the next move from the player
// return 0xff of no move entered
{
	int8_t	m = 0xff;

	for (uint8_t i=0; i<TTT_BOARD_SIZE; i++)
		if (board[i].button->isPressed())
		{
			m = board[i].button->id;
			break;
		}

	return(m);
}

void tttCallback(uint8_t position, int8_t player)
{
	displayPosition(position, player);
}

void TTT_FSM() 
{
	static gameState curState = gStart;	// current state

	checkAutoPlay();

	switch (curState)
	{
	case gStart:	// initialise for a new game
		for (uint8_t i=0; i<ARRAY_SIZE(mesgRow); i++)
			userMessage(i, BLANK_MESG);
		createBoardButtons();
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
			userMessage(0, "GAME OVER!!");
			if (TTT.getGameWinner() == TTT_P0)
			{
				userMessage(1, "We draw.");
			}
			else if (TTT.getGameWinner() == TTT.getAutoPlayer())
				userMessage(1, "I win!");
			else
				userMessage(1, "You win!");

			if (TTT.getGameWinner() != TTT_P0)	// not a draw
				strikeLine(TTT.getWinLine(), TTT.getGameWinner());

			inGamePlay = false;

			killBoardButtons();
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
