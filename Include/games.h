/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/
/*
01234567890123456789012345678901234567890123456789012345678901234567890123456789
Game Code v2 for ROM based muds. Robert Schultz, Sembiance  -  bert@ncinter.net
Snippets of mine can be found at http://www.ncinter.net/~bert/mud/
This file (games.h) is the header file for games.c
*/


#define      GAME_NONE       0
#define      GAME_SLOTS      1
#define      GAME_HIGH_DICE  2

void do_slots(CHAR_DATA *ch, char *argument );
void do_high_dice(CHAR_DATA *ch, char *argument);
void do_checkers(CHAR_DATA *ch, char *argument);
