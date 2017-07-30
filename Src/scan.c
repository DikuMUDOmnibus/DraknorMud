/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@efn.org)				   *
*	    Gabrielle Taylor						   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"

char *const distance[4]=
{
"right here.", "just %s from you.", "not far %s.", "off in the distance %s."
};

void scan_list           args((ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch,
                               sh_int depth, sh_int door));
void scan_char           args((CHAR_DATA *victim, CHAR_DATA *ch,
                               sh_int depth, sh_int door));
void do_scan(CHAR_DATA *ch, char *argument)
{
  extern char *const dir_name[];
  char arg1[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *scan_room;
  EXIT_DATA *pExit;
  sh_int door, depth;

  argument = one_argument(argument, arg1);

  if (arg1[0] == '\0')
    {
      act("$n looks all around.", ch, NULL, NULL, TO_ROOM);
      send_to_char("Looking around you see:\n\r", ch);
      scan_list(ch->in_room, ch, 0, -1);

      for ( door = 0; door < 6; door++ )
	{
	  if ((pExit = ch ->in_room->exit[door]) != NULL)
            scan_list(pExit->u1.to_room, ch, 1, door);
	}
      return;
    }
  else 
    door = arg_to_dirnum( arg1 );
  if ( door == -1 )
      { send_to_char("Which way do you want to scan?\n\r", ch); return; }

  act("You peer intently $T.", ch, NULL, dir_name[door], TO_CHAR);
  act("$n peers intently $T.", ch, NULL, dir_name[door], TO_ROOM);
  mprintf(sizeof(buf),buf, "Looking %s you see:\n\r", dir_name[door]);
                                                                                  
  scan_room = ch->in_room;

  for (depth = 1; depth < 4; depth++)
    {
      if ((pExit = scan_room->exit[door]) != NULL)
	{
	  scan_room = pExit->u1.to_room;
	  scan_list(pExit->u1.to_room, ch, depth, door);
	}
    }
  return;
}

void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, sh_int depth,
               sh_int door)
{
  CHAR_DATA *rch;

  if (scan_room == NULL) return;
  /*
   * this used to cause a mysterious crash here, finally realized it was
   * 'door' being -1, and rev_dir seems to have a problem with that...
   * only acted up when it was done in a room with "extra" exits - Mull
   */
  if ( door != -1 && scan_room->exit[rev_dir[door]] != NULL
       && IS_SET(scan_room->exit[rev_dir[door]]->exit_info,EX_CLOSED) )
    return;

  for (rch=scan_room->people; rch != NULL; rch=rch->next_in_room)
    {
      if (rch == ch) continue;
      if (!IS_NPC(rch) && rch->invis_level > get_trust(ch)) continue;
      if (can_see(ch, rch)) scan_char(rch, ch, depth, door);
    }
  return;
}

void scan_char(CHAR_DATA *victim, CHAR_DATA *ch, sh_int depth, sh_int door)
{
  extern char *const dir_name[];
  extern char *const distance[];
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

  buf[0] = '\0';

  if ( IS_SAFFECTED( ch, SAFF_FARSIGHT )
  ||   ch->in_room == victim->in_room
  ||   IS_SET( ch->act, PLR_HOLYLIGHT ) )
  {
      sprintf( buf, "%s, ", capitalize_color( PERS( victim, ch ), FALSE ) );
  }
  else
  {
      sprintf( buf, "Something %s, ", size_table[victim->size].name );
  }

  //strcat(buf, PERS(victim, ch));
  //if (!strcmp(buf,"NOTHING"))
  //  return;
  //strcat(buf, ", ");
  if (depth != 0)
    mprintf(sizeof(buf2),buf2, distance[depth], dir_name[door]);
  else
    mprintf(sizeof(buf2),buf2, distance[depth]);
  strcat(buf, buf2);
  strcat(buf, "\n\r");
 
  send_to_char(buf, ch);
  return;
}
