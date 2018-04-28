/**
\mainpage Arduino TicTacToe game decision engine library

The motivation for this library is to explore the separation between the 
algorithm for managing a game and the user interface for the game. Discovering 
a TicTacToe algorithm simple enough to implement on the Arduino allows an 
exploration of this concept with simple user interface requirements.

The integration between game library and user interface is largely implemented 
through a callback function that allows the user program to update each game 
cell as the game progresses, without needing detailed knowledge of the game 
board and current positions. The example programs supplied provide a framework 
that can be changed for alternative user interfaces.

The code implements an elegant algorithm that is briefly described in the blog
referenced with some additional features to make decisions easier to implement in code. 

The only call required while running the game is to the library function 
doMove().

Topics
------
- \subpage pageBackground

- \subpage pageLibrary

References
----------
Xeda112358, ‘Tic-Tac-Toe algorithm’, blog on 26 March, 2012, 17:43:57, 
accessed at http://www.omnimaga.org/math-and-science/tic-tac-toe-algorithm/

Revision History
----------------
April 2018 - version 1.0.1
- Minor documentation uypdates

March 2013 - version 1.0
- Initial implementation

Copyright
---------
Copyright (C) 2013 Marco Colli. All rights reserved.

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
  
\page pageBackground Rules of the Game

In case there is anyone who has never played this game, TicTacToe
(also called Noughts and Crosses, Xs and Os) is a pencil-and-paper 
game for two players, X and O, who take turns marking the spaces in 
a 3×3 grid. The player who succeeds in placing three respective marks 
in a horizontal, vertical, or diagonal row wins the game.

The following example game is won by the first player, X:
![TicTacToe Game](media\photo\TicTacToe-game.png)

Players soon discover that best play from both parties leads to a draw
(often referred to as cat or cat's game). Hence, Tic-tac-toe is most often 
played by young children.

Strategy
--------

A player can play perfect TicTacToe (win or draw) given they choose the 
first possible move from the following list.
- **Win**: If the player has two in a row, they can place a third to get three in a row.

- **Block**: If the [opponent] has two in a row, the player must play the third 
themself to block the opponent.

- **Fork**: Creation of an opportunity where the player has two threats to win 
(two non-blocked lines of 2). 

- **Blocking an opponent's fork**:

  + **Option 1**: The player should create two in a row to force the opponent into 
defending, as long as it doesn't result in them creating a fork. For example, 
if "X" has a corner, "O" has the center, and "X" has the opposite corner as well, 
"O" must not play a corner in order to win. (Playing a corner in this scenario 
creates a fork for "X" to win.)
 
  + **Option 2**: If there is a configuration where the opponent can fork, the player 
should block that fork.
   
- **Center**: A player marks the center. (If it is the first move of the game, 
playing on a corner gives "O" more opportunities to make a mistake and may 
therefore be the better choice; however, it makes no difference between perfect 
players.)

- **Opposite corner**: If the opponent is in the corner, the player plays the opposite 
corner.

- **Empty corner**: The player plays in a corner square.

- **Empty side**: The player plays in a middle square on any of the 4 sides.

\page pageLibrary Computer Algorithm for TicTacToe

Note: To allow manually exploring the concepts outlined here, the algorithm described 
is implemented in the Microsoft Excel spreadsheet accompanying the library release.

Playing Board
-------------
In TicTacToe, there are 9 positions and 8 wins. Each position we label as _a_ to _i_ 
like this:

    a  b  c
    d  e  f
    g  h  i

Winning Moves
-------------
We can assign a matrix of win contributions to each position as a row of 8 elements.

    [D1, H1, H2, H3, V1, V2, V3, D2]
    
where _D1_ is the main diagonal (left to right), _H1_, _H2_, _H3_ are horizontal wins, 
_V1_, _V2_, _V3_ are vertical wins, and _D2_ is the other diagonal.

For each win element, if a board position occurs in a win, give it a 1, like so:

    [a] = [1, 1, 0, 0, 1, 0, 0, 0]
    [b] = [0, 1, 0, 0, 0, 1, 0, 0]
    [c] = [0, 1, 0, 0, 0, 0, 1, 1]
    [d] = [0, 0, 1, 0, 1, 0, 0, 0]
    [e] = [1, 0, 1, 0, 0, 1, 0, 1]
    [f] = [0, 0, 1, 0, 0, 0, 1, 0]
    [g] = [0, 0, 0, 1, 1, 0, 0, 1]
    [h] = [0, 0, 0, 1, 0, 1, 0, 0]
    [i] = [1, 0, 0, 1, 0, 0, 1, 0]

Now, reform this into a giant 9x8 matrix. This matrix remains constant for all games.

|  |D1|H1|H2|H3|V1|V2|V3|D2|
|-:|-:|-:|-:|-:|-:|-:|-:|-:|
| a| 1| 1|  |  | 1|  |  |  |
| b|  | 1|  |  |  | 1|  |  |
| c|  | 1|  |  |  |  | 1| 1|
| d|  |  | 1|  | 1|  |  |  |
| e| 1|  | 1|  |  | 1|  | 1|
| f|  |  | 1|  |  |  | 1|  |
| g|  |  |  | 1| 1|  |  | 1|
| h|  |  |  | 1|  | 1|  |  |
| i| 1|  |  | 1|  |  | 1|  |

The Algorithm
-------------
The game win matrix [M] starts clean. Here we assume

    [M] = [0, 0, 0, 0, 0, 0, 0, 0]
    
Player 1 makes the first move, say position [a]. The line for positions 
[a] is added to the game matrix [M] to give an game position: 

    [M] = [1, 1, 0, 0, 1, 0, 0, 0]

Player 2’s move will subtract from the game matrix. If they take position 
[e], the resulting game matrix will be 

    [M] = [1, 1, 0, 0, 1, 0, 0, 0] - [1, 0, 1, 0, 0, 1, 0, 1] = [0, 1,-1, 0, 1,-1, 0,-1]

And so-on for subsequent moves, alternately adding and subtracting from [M].

We therefore have a way of tracking how the game is progressing and, more 
importantly, to test a move against the current board to determine which 
is the best move at any time.

The best move, in order of importance, will be one that

+ Makes a 3 or -3 (depending on the player), because this means we have made 
3 in a row, column or diagonal.

+ Makes as many 2’s as possible provided there are no 2 or 1 of opposite sign, 
as it means other player would win the match. Leaving 2 2’s will mean that we 
are guaranteed to win on the next turn. Leaving a 2 of the opposite sign means 
that the other player will win next turn.

+ Make as many 2’s into 1’s as a last resort. A 2 now will turn into a 3 the next 
move, and 3 means someone wins!

+ Make the highest number of 1 provided there are no 2 of opposite sign, as 
it means other player would win the match.

If there is more than one ‘best’ move than it is possible to select randomly 
between choices that fit the highest criterion. This makes the algorithm slightly 
unpredictable and can result in moves that allow the other player to win the 
game, thus relieving the boredom of always losing against the computer!

The game is over when there are any 3’s or -3’s and the column in which this 
number appears will also tell exactly where to strike through for wins.
*/
#ifndef _MD_TTT_H
#define _MD_TTT_H

#include <Arduino.h>

// Miscellaneous defines
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#define CELL_ID(i)    ((char)(i+'a'))

#define TTT_BOARD_SIZE  9

// Player definitions
#define TTT_P1  1 ///< Player 1
#define TTT_P0  0 ///< No player
#define TTT_P2 -1 ///< Player 2

// Winning Lines definitions
#define TTT_WL_D1 0 ///< Win line diagonal left to right
#define TTT_WL_H1 1 ///< Win line 1st horizontal
#define TTT_WL_H2 2 ///< Win line 3nd horizontal
#define TTT_WL_H3 3 ///< Win line 3rd horizontal
#define TTT_WL_V1 4 ///< Win line 1st vertical
#define TTT_WL_V2 5 ///< Win line 2nd vertical
#define TTT_WL_V3 6 ///< Win line 3rd vertical
#define TTT_WL_D2 7 ///< Win line diagonal right to left

/**
 * Core object for the MD_TTT library.
 * This class contains all logic and status information for the game.
 */
class MD_TTT 
{
  public:
  //--------------------------------------------------------------
  /** \name Methods for Setup and Initialization.
   * @{
   */

  /** 
   * Class Constructor.
   *
   * Creates a newly initialized MD_TTT object. The parameter mh is the address 
   * of a user callback function with prototype
   * 
   * void tttCallback(uint8_t pos, int8_t player)
   * 
   * This callback function is called from the library at the completion of every 
   * move to allow the user code to animate or otherwise deal with the user 
   * interface component of the game. The callback is given the position board 
   * position of the last move (0-8) in pos and the player identifier for the 
   * player occupying that position (TTT_P0, TTT_P1 or TTT_P2) in player. 
   * TTT_P0 denotes an empty cell.
   *
   * The callback function may use any of the library status functions to 
   * determine the status of the game at that point. It is intended that all in-game user interface updates should occur only during the callback. 
   *
   * \param mh pointer to user callback function.
   */
  MD_TTT(void (*mh)(uint8_t pos, int8_t player));

  /** 
   * Class Destructor.
   *
   * Release allocated memory and does the necessary to clean up once the object is
   * no longer required.
   */
  ~MD_TTT(void);

  /** @} */
  //--------------------------------------------------------------
  /** \name Methods for Game Management.
   * @{
   */

  /**
   * Reset the board for a new game.
   *
   * Resets the board for a new game. All the internal tracking parameters 
   * are reset and the game board is cleared. The user callback function 
   * is invoked for each location that is changed to its reset state.
   *
   * \return true if no errors occurred, false otherwise.
   */
  bool start();
  
  /**
   * Execute the next game move.
   *
   * Instruct the library to execute the next move. The location for the 
   * move is given in _pos_ and the identifier for the player who is 
   * making this move is passed through _player_. If _player_ corresponds 
   * to the auto player, then _pos_ is ignored and the libraries 
   * make a decision on the next move for this player. The user callback 
   * function is always invoked after the move is completed and all game 
   * status values have been settled.
   *
   * \param pos position on the board for the move [0..8].
   * \param player  player identifier TT_P1 or TT_P2.
   * \return true if no errors occurred, false otherwise.
   */
  bool doMove(uint8_t pos, int8_t player);

  /**
   * Set the computer player.
   * 
   * Sets player to be the library controlled player. By default, this is 
   * TT_P0 (ie, not one of the players). If the auto player is not set 
   * then the library will never invoke the move generation logic for any 
   * player in doMove().
   *
   * \param player  player identifier TT_P1 or TT_P2.
   * \return true if no errors occurred, false otherwise.
   */  
  bool setAutoPlayer(int8_t player);

  /**
   * Get the computer player id.
   *
   * Returns the player identifier of the designated auto player. 
   * This would have previously been set by a call to setAutoPlayer().
   * Note that the return value is an int8_t (ie, a signed value).
   *
   * \return the player identifier, one of TTT_P*.
   */
  int8_t getAutoPlayer(void) {return _autoPlayer;}

  /** @} */
  //--------------------------------------------------------------
  /** \name Methods for Board Management.
   * @{
   */

  /**
   * Return if the game is over
   *
   * Used to check if a game has been won.
   *
   * \return true if the game is over, false otherwise.
   */
   bool isGameOver(void) {return _gameOver;}

  /**
   * Return the player that won
   *
   * Used to determine who won a game. Note that the return 
   * value is an int8_t (ie, a signed value).
   *
   * \return winner player identifier, one of TTT_P*.
   */
  int8_t getGameWinner(void) {return _gameWinner;}

  /**
   * Return the winning line
   *
   * Returns the winning line for the game. The calling program should first 
   * check isGameOver() to ensure that the winning line returned is valid. 
   * The winning line will be one of the identifiers TTT_WL_* in MD_TTT.h.
   * The function should be used to easily determine which line to 
   * ‘strike out’ at the end of the game.
   *
   * \return the winning line id, one of TTT_WL_*.
   */
  uint8_t getWinLine(void) {return _winLine;}

  /**
   * Get the occupier of a board position
   *
   * Returns the player identifier (TTT_P*) for the player occupying the 
   * board position _pos_. Note that the return value is an int8_t 
   * (ie, a signed value).
   *
   * \param pos the position to check
   * \return the player identifier, one of TTT_P*.
   */
  int8_t getBoardPosition(uint8_t pos);

  /** @} */

  protected:
  int8_t  _board[TTT_BOARD_SIZE];  ///< the game board
  uint8_t _movesLeft;     ///< the number of moves left in the game
  bool    _gameOver;      ///< flag to know when the game is over
  int8_t  _gameWinner;    ///< id of player who won
  uint8_t _winLine;       ///< the winning line (TTT_WL_*) or 0xff
  int8_t  _autoPlayer;    ///< the computer player (TTT_P0 if neither)

  void (*_cbMoveHandler)(uint8_t pos, int8_t player); ///< callback into user code to process the move

  uint8_t doAutoMove(int8_t player);        ///< work out a move for the auto player

  void unpackByte(uint8_t *pb, uint8_t b);  ///< unpack the byte into the array
  bool randomChoice(void);                  ///< return true or false randomly
};

#endif
