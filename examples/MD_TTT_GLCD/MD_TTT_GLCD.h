// Header file for MD_TTT_GLCD.ino
//
#ifndef MD_TTT_GLCD_H
#define MD_TTT_GLCD_H

// Enumerated types
enum gameState	{ gStart, gGetMove, gCheckEnd };

// Coordinates for a board position
typedef struct 
{
  uint16_t	x1, y1;
  uint16_t	x2, y2;
  MD_Button	*button;
} boardCoord;

#endif