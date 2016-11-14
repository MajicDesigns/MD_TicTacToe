/*
  MD_TTT.cpp - Arduino library for TicTacToe game decision engine
  Copyright (C) 2013 Marco Colli
  All rights reserved.

  See MD_TTT.h for complete comments

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <MD_TTT.h>

#define  DEBUG_TTT	0

#if  DEBUG_TTT
#define  DEBUG(s, v)  { Serial.print(F(s)); Serial.print(v); }
#define  DEBUGS(s)    Serial.print(F(s))
#else
#define  DEBUG(s, v)
#define  DEBUGS(s)
#endif

// The game win weight matrix - see documentation for meaning of bits
uint8_t	wwm[TTT_BOARD_SIZE] = 
{
// D1 H1 H2 H3 V1 V2 V3 D2 in order
	0b11001000,	// a
	0b01000100,	// b
	0b01000011,	// c
	0b00101000,	// d
	0b10100101,	// e
	0b00100010,	// f
	0b00011001,	// g
	0b00010100,	// h
	0b10010010	// i
};

// Current state of the game
int8_t	currState[8];


MD_TTT::MD_TTT(void	(*_mh)(uint8_t pos, int8_t player))
{
	_cbMoveHandler = _mh;
	_autoPlayer = TTT_P0;
}

MD_TTT::~MD_TTT(void)
{
}

int8_t MD_TTT::getBoardPosition(uint8_t pos)
{
	if (pos < TTT_BOARD_SIZE)
		return(_board[pos]);

	DEBUG("\ngetBoardPosition out of bounds: ", pos);

	return(TTT_P0);
}

bool MD_TTT::setAutoPlayer(int8_t p)
// set the automatic player to the player id
{
	if ((p == TTT_P0) || (p == TTT_P1) || (p == TTT_P2))
	{
		DEBUG("\nsetAutoPlayer ", p);
		_autoPlayer = p;
		return(true);
	}

	return(false);
}

void MD_TTT::unpackByte(uint8_t *pb, uint8_t b)
// unpack the byte into the array. MSB is in pb[0]
{
	uint8_t	mask = 0x80;

	for (uint8_t i=0; i<8; i++, mask>>=1)
		*pb++ = ((b & mask) ? 1 : 0);
}

bool MD_TTT::randomChoice(void)
// if millis is odd, then return true, else false
{
	DEBUGS("R");
	return((micros() & 0x7) > 3);
}

bool MD_TTT::start(void)
{
	DEBUGS("\nStarting NEW GAME");
	// reset the player positions on the board
	for (uint8_t i=0; i<TTT_BOARD_SIZE; i++)
	{
		_board[i] = TTT_P0;
		// run the callback to sync the user board
		if (_cbMoveHandler != NULL)
			(_cbMoveHandler)(i, TTT_P0);
	}

	// reset the current game position to start
	for (uint8_t i=0; i<ARRAY_SIZE(currState); i++)
		currState[i] = 0;

	// game control variables
	_gameOver = false;
	_gameWinner = TTT_P0;
	_movesLeft = TTT_BOARD_SIZE;
	_winLine = 0xff;

	return(true);
}

bool MD_TTT::doMove(uint8_t pos, int8_t player)
{
	uint8_t	weight[ARRAY_SIZE(currState)];

	// first check if we are supposed to make a move
	if (player == _autoPlayer)
		pos = doAutoMove(player);

	DEBUG("\nMove ", CELL_ID(pos));
	DEBUG(" for player ", player);

	// now proceed with a normal move
	if (pos >= TTT_BOARD_SIZE) return(false);
	if (_board[pos] != TTT_P0) return(false);

	// execute the move ...
	_movesLeft--;
	_board[pos] = player;

	// ... work out what this means to the current game ...
	DEBUGS("\nCur GM");
	unpackByte(weight, wwm[pos]);
	for (uint8_t i=0; i<ARRAY_SIZE(weight); i++)
	{
		currState[i] += (player * weight[i]);
		DEBUG(" ", currState[i]);

		// check if we have a winner
		if ((currState[i] == 3) || (currState[i] == -3))
		{
			_gameOver = true;
			_gameWinner = player;
			_winLine = i;
		}
	}

	// ... if there are no moves left to play then the game is also over ...
	_gameOver |= (_movesLeft == 0);

	// ... and run the callback
	if (_cbMoveHandler != NULL)
		(_cbMoveHandler)(pos, player);

	return(true);
}

uint8_t MD_TTT::doAutoMove(int8_t player)
// Determine and select the best available move
{
	uint8_t		p = 0xff;	// best move position to be returned
	uint8_t		scores[TTT_BOARD_SIZE][7];	// indices should be [-3, -2, -1, 0, 1, 2, 3] - need to offset by 3 for zero based array
	uint8_t		tryMode;	// modes for move selection

	DEBUG("\nAutomove P", player);

	// reset the scores matrix
	for (uint8_t k=0; k<TTT_BOARD_SIZE; k++)
		for (uint8_t i=0; i<ARRAY_SIZE(scores[0]); i++)
			scores[k][i] = 0;
	
	// work out the scores array for each position
	for (uint8_t k=0; k<TTT_BOARD_SIZE; k++)
	{
		uint8_t		weight[ARRAY_SIZE(currState)];	// for unpacking the weighting

		if (_board[k] == TTT_P0)	// an empty cell so a potential move
		{
			unpackByte(weight, wwm[k]);

			DEBUG("\n", CELL_ID(k));
			for (uint8_t i=0; i<ARRAY_SIZE(weight); i++)
			{
				int8_t	s;

				s = currState[i] + (player * weight[i]);
				DEBUG(" ", s);
				scores[k][s+3]++;	// one more for this team (offset)
			}

			// debug print the scores for this line
			DEBUGS("\tS:");
			for (uint8_t i=0; i<ARRAY_SIZE(scores[0]); i++)
				DEBUG(" ", scores[k][i]);
		}
	}

	// player sign determines our preferred score being + or -, 
	// so to standardise how we check the score we swap the location 
	// of the scores based on which player this is.
	if (player < 0) // preferred scres are -ve
	{
		for (uint8_t k=0; k<TTT_BOARD_SIZE; k++)
		{
#define	SWAP(i, j)	{ int8_t t = scores[k][i]; \
					scores[k][i] = scores[k][j]; \
					scores[k][j] = t; }
			SWAP(0, 6);
			SWAP(1, 5);
			SWAP(2, 4);
#undef	SWAP
		}
		DEBUGS("\nSWAPPED score matrix");
	}

	// now do our checks on the scores array to get the best move
	// Scores array indices [0..7] are for scores [-3, -2, -1, 0, 1, 2, 3], so given any score,
	// its index position is offset by +3 eg, score -2 is position 1 in the array (-2 + 3 = 1)
	tryMode = 0;					// the criterion to use for checking optimal move (switch statement)
	while (p == 0xff)				// when we have a return value, stop looking
	{
		uint8_t	curBest = 0xff;		// save current best move for comparison to others
		
		DEBUG("\nTrying Mode ", tryMode);
		for (uint8_t i=0; i<TTT_BOARD_SIZE; i++)
		{
			if (_board[i] == TTT_P0)
			{
				DEBUG(": ", CELL_ID(i));
				switch (tryMode)
				{
				case 0:		
					// best move if we have any 3 scores as we have the winning move
					if (scores[i][6] != 0)
						curBest = i;
					break;

				case 1:
					// next best is highest number of 2 provided there are no 2 or 1 of opposite sign, 
					// as it means other player would win the match
					if ((scores[i][5] != 0) && (scores[i][1] == 0) && (scores[i][2] == 0))
					{
						if ((curBest == 0xff) || (scores[i][5] > scores[curBest][5]))
							curBest = i;
						else if (scores[i][5] == scores[curBest][5])
							curBest = (randomChoice() ? i : curBest);
					}
					break;

				case 2:
					// next best is highest number of 2 provided there are no 2 of opposite sign, 
					// as it means other player would win the match
					if ((scores[i][5] != 0) && (scores[i][1] == 0))
					{
						if ((curBest == 0xff) || (scores[i][5] > scores[curBest][5]))
							curBest = i;
						else if (scores[i][5] == scores[curBest][5])
						{
							if (scores[i][2] < scores[curBest][2])	// advantage of fewer 1's
								curBest = i;
							else if (scores[i][2] == scores[curBest][2]) // same number of 1's
								curBest = (randomChoice() ? i : curBest);
						}
					}
					break;

				case 3:
					// next best is highest number of 1 provided there are no 2 of opposite sign, 
					// as it means other player would win the match
					if ((scores[i][4] != 0) && (scores[i][1] == 0))
					{
						if ((curBest == 0xff) || (scores[i][4] > scores[curBest][4]))
							curBest  = i;
						else if (scores[i][4] == scores[curBest][4]) 
						{
							if (scores[i][2] < scores[curBest][2])	// lowest opponent 1's
								curBest = i;
							else if (scores[i][2] == scores[curBest][2]) // same number of 1's
								curBest = (randomChoice() ? i : curBest);
						}
					}
					break;

				default:
					// next best is highest number of 1 with no other consideration
					if (scores[i][4] != 0)
					{
						if ((curBest == 0xff) || (scores[i][4] > scores[curBest][4]))
							curBest = i;
						else if (scores[i][4] == scores[curBest][4])
						{
							if (scores[i][2] < scores[curBest][2])	// lowest opponent 1's
								curBest = i;
							else if (scores[i][2] == scores[curBest][2]) // same number of 1's
								curBest = (randomChoice() ? i : curBest);
						}
					}
					break;
				} // switch
				DEBUG("/", curBest);
			} // if
		} // for

		// have we got a current best move?
		if (curBest != 0xff)
		{
			p = curBest;
			DEBUG("\nBest move at cell '", CELL_ID(p));
			DEBUG("' on mode ", tryMode);
		}

		tryMode++;		// try the next best criteria if we go around again

	} // while

	return(p);
}

