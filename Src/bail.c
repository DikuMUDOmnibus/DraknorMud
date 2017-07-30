/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "tables.h"
#include "magic.h"

void do_bail(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *mob;
  OBJ_DATA *obj, *obj_next;
  char arg[MAX_INPUT_LENGTH];

  /* check for healer */
  for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
      if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_BAILER) )
	break;
    }
 
  if ( mob == NULL )
    {
      send_to_char( "You can't do that here.\n\r", ch );
      return;
    }

  if (IS_GHOST(ch)) {
    send_to_char("You cannot bail yourself out, you are {rDEAD{x.\n\r",ch);
    return;
  }

  if (!IS_SET(ch->act, PLR_KILLER)) {
    send_to_char("You have to be a {rkiller{x!\n\r",ch);
    return;
  }
  if (IS_SET(ch->act, PLR_VIOLENT) || IS_SAFFECTED(ch, SAFF_ADRENALIZE)) {
    send_to_char("You cannot get bail while you are violent or adrenalized.\n\r", ch);
    return;
  }
  if (ch->bailed) {
    send_to_char("You are already bailed\n\r",ch);
    return;
  }

  one_argument(argument,arg);

  if (arg[0] == '\0')
    {
      /* display price list */

      if (ch->num_bailed > 5) {
	send_to_char("Sorry, you have exceeded your number of bail times.\n\r",ch);
      }
      else
	printf_to_char(ch, "Your bail will cost %d diamonds.\n\r", ch->level);
      return;
    }

  if (!str_prefix(arg,"pay"))
    {
      int count=0;
      for (obj = ch->carrying; obj; obj = obj->next_content)
	if (IS_DIAMOND(obj)) {
	  count++;
	}
      printf_to_char(ch, "You are carrying %d diamond(s). The Cost is %d\n\r",count, ch->level);
      if (count >= ch->level)
	{
	  count = ch->level;
	  send_to_char("Paying bail now.\n\r",ch);
	  for (obj=ch->carrying;  obj; obj = obj_next) {

	    obj_next = obj->next_content;
	    if (count <= 0)
	      break;
	    if (IS_DIAMOND(obj)) {
	      clan_table[9].free_dia += 1;
	      clan_table[9].total_dia += 1;
	      obj_from_char(obj);
	      count--;
	      extract_obj( obj );
	    }
	  }
	  /* ASSUME diamonds extracted.  now update dredds diamonds */
	  save_clan_info(CLAN_FILE);
	  ch->bailed = current_time;
	  ch->num_bailed += 1;
	  return;
	} else {
	  send_to_char("You cannot afford bail.\n\r",ch);
	  return;
	}
    }
  do_function(ch, &do_bail, "");
}
