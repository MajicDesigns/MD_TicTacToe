// Tic Tac Toe using the console for input/output
//
// Input and output using the Serial console.
// Game play using the MD_TTT library and
// non-blocking user input so we can do other stuff!
//
#include <MD_TTT.h>

MD_TTT	TTT(tttCallback);

char	player[] = { 'O', ' ', 'X' };
int8_t	curPlayer = TTT_P1;
bool	inGamePlay = false;

void setup() 
{
  Serial.begin(57600);
  Serial.println(F("\n[TTT Console Example]\n"));

  TTT.setAutoPlayer(curPlayer);
}

uint8_t getChar()
// non-blocking wait for an input character from the input stream
{
	if (Serial.available() == 0)
		return(0xff);

	return(toupper(Serial.read()));
}

void clearInput()
// clear all characters from the serial input
{
	while (Serial.available() > 0)
		Serial.read();
}

uint8_t getMove(void)
// get the next move from the player
// there may not be a move there so we need to split the 
// function into a prompting and then checking phase
// return 0xff of no move entered
{
	static bool	promptMode = true;

	uint8_t	m = 0xff;

	if (promptMode)
	{
		Serial.print(F("\nYour move? (a-i): "));
		clearInput();
		promptMode = false;
	}
	else
	{
		uint8_t	c = getChar();

		if (c != 0xff)
		{
			Serial.print((char)c);
			if (c>='A' && c<='I')
			{
				m = c - 'A';
				if (TTT.getBoardPosition(m) != TTT_P0)
					m = 0xff;
			}
			promptMode = true;
		}
	} 

	return(m);
}

void tttCallback(uint8_t position, int8_t player)
{
	if (player == TTT.getAutoPlayer())
	{
		Serial.print("\nComputer move: ");
		Serial.print((char)(position+'A'));
	}
	displayBoard();
}

void displayBoard(void)
{
#define SEPARATOR	Serial.print("\n---+---+---  ---+---+---");
#define	LINE(i,j,k)	{ Serial.print("\n ");\
					  Serial.print(player[TTT.getBoardPosition(i)+1]); Serial.print(" | "); \
					  Serial.print(player[TTT.getBoardPosition(j)+1]); Serial.print(" | "); \
					  Serial.print(player[TTT.getBoardPosition(k)+1]); Serial.print("    "); \
					  Serial.print((TTT.getBoardPosition(i) == TTT_P0) ? CELL_ID(i) : ' '); Serial.print(" | "); \
					  Serial.print((TTT.getBoardPosition(j) == TTT_P0) ? CELL_ID(j) : ' '); Serial.print(" | "); \
					  Serial.print((TTT.getBoardPosition(k) == TTT_P0) ? CELL_ID(k) : ' '); \
					}
	if (inGamePlay)
	{
		LINE(0,1,2);
		SEPARATOR;
		LINE(3,4,5);
		SEPARATOR;
		LINE(6,7,8);
		Serial.println();
	}
#undef	SEPARATOR
#undef	LINE
}

void TTT_FSM() 
{
	static uint8_t	curState = 0;	// current state

	switch (curState)
	{
	case 0:	// initialise for a new game
		inGamePlay = TTT.start();
		displayBoard();
		curState++;
		break;

	case 1:	// get and make player move - this section is non-blocking
		{
			uint8_t	m = 0;

			if (TTT.getAutoPlayer() != curPlayer)
				m = getMove();
			
			if (m != 0xff)
			{
				TTT.doMove(m, curPlayer);
				curState++;
			}
		}
		break;

	case 2:	// switch players and check if this is the end of the game
		if (TTT.isGameOver())
		{
			inGamePlay = false;

			Serial.print(F("\nGAME OVER!! "));
			if (TTT.getGameWinner() == TTT_P0)
				Serial.print(F("It's a draw."));
			else if (TTT.getGameWinner() == TTT.getAutoPlayer())
				Serial.print(F("I win!"));
			else
				Serial.print(F("You win. Congratulations!\n"));

			if (TTT.getGameWinner() != TTT_P0)
			{
				Serial.print(F("\nWinning line is "));
				switch (TTT.getWinLine())
				{
				case TTT_WL_D1:	Serial.print(F("D1"));	break;
				case TTT_WL_D2:	Serial.print(F("D2"));	break;
				case TTT_WL_H1:	Serial.print(F("H1"));	break;
				case TTT_WL_H2:	Serial.print(F("H2"));	break;
				case TTT_WL_H3:	Serial.print(F("H3"));	break;
				case TTT_WL_V1:	Serial.print(F("V1"));	break;
				case TTT_WL_V2:	Serial.print(F("V2"));	break;
				case TTT_WL_V3:	Serial.print(F("V3"));	break;
				}
			}
			Serial.print(F("\nLet's play again...\n"));

			curState = 0;
		}
		else
			curState = 1;

		// switch turns for players
		curPlayer = (curPlayer == TTT_P1 ? TTT_P2 : TTT_P1);
		break;

	default:
		curState = 0;
		break;
	}
}

void loop(void)
{
	TTT_FSM();
}
