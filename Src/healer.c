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
#include "magic.h"

void do_heal(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *mob;
  char arg[MAX_INPUT_LENGTH];
  int cost,sn;
  SPELL_FUN *spell;
  char *words;
  char arg2[MIL];
  CHAR_DATA *vch = ch;

  /* check for healer */
  for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
      if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER) )
	break;
    }
 
  if ( mob == NULL )
    {
      send_to_char( "You can't do that here.\n\r", ch );
      return;
    }

  if (IS_GHOST(ch)) {
    send_to_char("Healing yourself is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  argument = one_argument(argument,arg);
  one_argument( argument, arg2 );

      /* If there is an arg2, try to match it to someone in the room. */
      if ( arg2[0] != '\0' )
        {
                if ( ( vch = get_char_room( ch, arg2 ) ) == NULL )
                {
                   act( "{g$N tells you '{WI'm sorry, but they are not here.{g'{x", mob, NULL, ch, TO_VICT );
                   return;
                }
        }

  if ( (arg[0] == '\0')
  ||   !str_prefix(arg,"list") )
    {
      /* display price list */
      act("{g$N says '{WI offer the following spells:{g'{x",ch,NULL,mob,TO_CHAR);
      send_to_char("  refresh{c:{x restore movement       5 {ygold{x\n\r",ch);
      send_to_char("  mana{c:{x    restore mana          10 {ygold{x\n\r",ch);
      send_to_char("  light{c:{x   cure light wounds     10 {ygold{x\n\r",ch);
      send_to_char("  disease{c:{x cure disease          15 {ygold{x\n\r",ch);
      send_to_char("  serious{c:{x cure serious wounds   15 {ygold{x\n\r",ch);
      send_to_char("  blind{c:{x   cure blindness        20 {ygold{x\n\r",ch);
      send_to_char("  critic{c:{x  cure critical wounds  25 {ygold{x\n\r",ch);
      send_to_char("  poison{c:{x  cure poison           25 {ygold{x\n\r",ch);
      send_to_char("  dispel{c:{x  dispel magic          30 {ygold{x\n\r",ch);
      send_to_char("  uncurse{c:{x remove curse          40 {ygold{x\n\r",ch);
      send_to_char("  heal{c:{x    healing spell         50 {ygold{x\n\r",ch);
      send_to_char("  weaken{c:{x  cure weaken           50 {ygold{x\n\r",ch);
      send_to_char(" Type heal {c<{xtype{c>{x to be healed.\n\r",ch);
      return;
    }

  if (!str_prefix(arg,"light"))
    {
      spell = spell_cure_light;
      sn    = skill_lookup("cure light");
      words = "judicandus dies";
      cost  = 1000;
    }

  else if (!str_prefix(arg,"serious"))
    {
      spell = spell_cure_serious;
      sn    = skill_lookup("cure serious");
      words = "judicandus gzfuajg";
      cost  = 1600;
    }

  else if (!str_prefix(arg,"critical"))
    {
      spell = spell_cure_critical;
      sn    = skill_lookup("cure critical");
      words = "judicandus qfuhuqar";
      cost  = 2500;
    }

  else if (!str_prefix(arg,"heal"))
    {
      spell = spell_heal;
      sn = skill_lookup("heal");
      words = "pzar";
      cost  = 5000;
    }

  else if (!str_prefix(arg,"blindness"))
    {
      spell = spell_cure_blindness;
      sn    = skill_lookup("cure blindness");
      words = "judicandus noselacri";		
      cost  = 2000;
    }

  else if (!str_prefix(arg,"disease"))
    {
      spell = spell_cure_disease;
      sn    = skill_lookup("cure disease");
      words = "judicandus eugzagz";
      cost = 1500;
    }

  else if (!str_prefix(arg,"poison"))
    {
      spell = spell_cure_poison;
      sn    = skill_lookup("cure poison");
      words = "judicandus sausabru";
      cost  = 2500;
    }
	
  else if (!str_prefix(arg,"uncurse") || !str_prefix(arg,"curse"))
    {
      spell = spell_remove_curse; 
      sn    = skill_lookup("remove curse");
      words = "candussido judifgz";
      cost  = 4000;
    }

  else if (!str_prefix(arg,"mana") || !str_prefix(arg,"energize"))
    {
      spell = NULL;
      sn = -1;
      words = "energizer";
      cost = 1000;
    }

  else if (!str_prefix(arg,"dispel"))
    {
      if (ch != vch){
        act( "{g$N tells you '{WI'm sorry, but I won't dispel someone else.{g'{x", mob, NULL, ch, TO_VICT );
        return;
      }
      spell = spell_cancellation;
      sn = skill_lookup("cancellation");
      words = "armoaoaa";
      cost = 3000;
    }

	
  else if (!str_prefix(arg,"refresh") || !str_prefix(arg,"moves"))
    {
      spell =  spell_refresh;
      sn    = skill_lookup("refresh");
      words = "candusima"; 
      cost  = 500;
    }

  else if (!str_prefix(arg,"weaken"))
  {
      spell = spell_cure_weaken;
      sn    = skill_lookup("cure weaken");
      words = "judicandus xzatunso";
      cost  = 5000;
  }

  else 
    {
      act("{g$N says '{WType 'heal' for a list of spells.{g'{x",
	  ch,NULL,mob,TO_CHAR);
      return;
    }

  if (cost > (ch->gold * 100 + ch->silver))
    {
      act("{g$N says '{WYou do not have enough gold for my services.{g'{x",
	  ch,NULL,mob,TO_CHAR);
      return;
    }

  WAIT_STATE(ch,PULSE_VIOLENCE);

  deduct_cost(ch,cost);
  mob->gold += cost / 100;
  mob->silver += cost % 100;
  /*ch->gold     += cost/100;*/
  act("$n utters the words '$T'.",mob,NULL,words,TO_ROOM);
  
  if (spell == NULL)  /* restore mana trap...kinda hackish */
    {
      vch->mana += dice(2,8) + mob->level / 3;
      vch->mana = UMIN(vch->mana,GET_MANA(vch));
      send_to_char("A warm glow passes through you.\n\r",vch);
      return;
    }

  if (sn == -1)
    return;
    
  spell(sn, mob->level, mob, vch, TARGET_CHAR);
}
