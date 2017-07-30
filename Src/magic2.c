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
*  ROM 2.4 is copyright 1993-1996 Russ Taylor         *
*  ROM has been brought to you by the ROM consortium       *
*      Russ Taylor (rtaylor@efn.org)           *
*      Gabrielle Taylor               *
*      Brian Moore (zump@rom.org)             *
*  By using this code, you have agreed to follow the terms of the     *
*  ROM license, in the file Rom24/doc/rom.license         *
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
#include "magic.h"
#include "tables.h"
#include "interp.h"


extern char *target_name;
bool check_gate(CHAR_DATA *ch,char *target,int spell_level, int sn);
CHAR_DATA *get_char_world_persistent( CHAR_DATA *ch, char *argument );

void spell_farsight( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  AFFECT_DATA af;
  if (IS_AFFECTED(ch,AFF_BLIND))
  {
    send_to_char("Maybe it would help if you could see?\n\r",ch);
    return;
  }
  if (IS_SET(ch->spell_aff, SAFF_FARSIGHT))
  {
    send_to_char("You are already in farsight.\n\r",ch);
    return;
  }
  af.where     = TO_SPELL_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = SAFF_FARSIGHT;
  affect_to_char( ch, &af );
  send_to_char("You have become one with the farsight.\n\r",ch);
  /*
    do_function(ch, do_scan, target_name);
  */
}


/* RT ROM-style gate */

void spell_gate( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim;
  bool gate_pet;

  if ( IS_AFFECTED(ch,AFF_CURSE) )
  {
    send_to_char("You cannot create a gate while cursed.\n\r",ch);
    return;
  }

  if (!check_gate(ch,target_name, ch->level, sn) )
  {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  victim = get_char_world_persistent( ch, target_name );

  if ( !victim || !ch )
  {
    send_to_char("You failed to gate.\n\r",ch);
    return;
  }

  if ( !IS_NPC( ch ) && !IS_NPC( victim ) )
  {
    if ( (victim->clan == NULL)
         &&   IS_SET(victim->act, PLR_NOGATE) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }

    if ( victim->clan && ch->clan )
    {
      if ( !IS_SET( ch->clan->clan_flags, CLAN_GWYLIAD )
           &&   !IS_IMMORTAL( ch ) )
      {
        if ( !is_same_group( ch, victim ) )
          if (!is_same_clan( ch, victim )
              || ( IS_SET( ch->clan->clan_flags, CLAN_INDEPENDENT ) ) )
          {
            send_to_char( "You must be grouped to gate to a member of another clan.\n\r", ch );
            return;
          }
      }
    }
  }

  if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
    gate_pet = TRUE;
  else
    gate_pet = FALSE;

  act("$n steps through a gate and vanishes.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You step through a gate and vanish.\n\r",ch);
  if ((ch->level < MAX_LEVEL-9)&&(ch->incog_level))
  {
    send_to_char("{RRemoving {WCloaking{G: Leaving Area.{x\n\r",ch);
    ch->incog_level = 0;
  }

  stop_fighting(ch,TRUE);
  move_to_room(ch,victim->in_room);

  act("$n has arrived through a gate.",ch,NULL,NULL,TO_ROOM);
  do_function(ch, &do_look,"auto");

  if (gate_pet)
  {
    ch->pet->position = POS_STANDING;
    act("$n steps through a gate and vanishes.",ch->pet,NULL,NULL,TO_ROOM);
    send_to_char("You step through a gate and vanish.\n\r",ch->pet);
    stop_fighting(ch->pet,TRUE);
    move_to_room(ch->pet,victim->in_room);
    act("$n has arrived through a gate.",ch->pet,NULL,NULL,TO_ROOM);
    do_function(ch->pet, &do_look,"auto");
  }

  /* rescue quest can follow through a gate */
  /* Really?  Should they?
    if ( !IS_NPC( ch )
    && ( ch->questdata->mob_vnum > -1 ) )
    {
      CHAR_DATA *rch;
      for ( rch = ch->in_room->people ; rch ; rch = rch->next_in_room)
      {
        if ( !IS_NPC( rch ) )
          continue;

        if ( strstr(rch->name,ch->name )
        && ( rch->pIndexData->vnum == MOB_VNUM_QUEST ) )
        {
          rch->position = POS_STANDING;
          act("$n steps through a gate and vanishes.",rch,NULL,NULL,TO_ROOM);
          move_to_room(rch,victim->in_room);
          act("$n has arrived through a gate.",rch,NULL,NULL,TO_ROOM);
          break;
        }
      }
    }
  */
}



void spell_portal( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *portal, *stone;

  if ( !check_gate(ch,target_name, ch->level, sn))
  {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  victim = get_char_world( ch, target_name );
  if (!victim || !ch)
  {
    send_to_char("You failed.\n\r",ch);
    return;
  }

  if ( ch->in_room->area->continent != victim->in_room->area->continent )
  {
    send_to_char( "Your magic is too weak for such a distance.\n\r", ch );
    return;
  }

  if (!IS_NPC( ch ) && !IS_NPC( victim ) )
  {
    if (!is_same_clan( ch, victim )
        || ( IS_SET( ch->clan->clan_flags, CLAN_INDEPENDENT ) ))
    {
      send_to_char("Trying to jump another clan is cheap.\n\r",ch);
      return;
    }
  }

  stone = get_eq_char(ch,WEAR_HOLD);

  if (!IS_IMMORTAL(ch)
      &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
  {
    send_to_char("You lack the proper component for this spell.\n\r",ch);
    return;
  }

  if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
  {
    act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
    act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
    extract_obj(stone);
  }

  portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
  portal->timer = 2 + level / 25;
  SET_BIT( portal->value[1], EX_SEETHROUGH );
  portal->value[3] = victim->in_room->vnum;

  obj_to_room(portal,ch->in_room);

  act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
  act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
}

void spell_nexus( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *vch;
  OBJ_DATA *portal;
  OBJ_DATA *stone;

  if ( ( vch = get_char_world( ch, target_name ) ) == NULL
       ||   !check_gate( ch,target_name, ch->level, sn )
       ||   IS_SET( vch->in_room->room_flags, ROOM_SAFE )
       ||   IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
  {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  if ( ch->in_room->area->continent != vch->in_room->area->continent )
  {
    send_to_char( "Your magic is too weak for such a distance.\n\r", ch );
    return;
  }

  stone = get_eq_char( ch, WEAR_HOLD );
  if ( !IS_IMMORTAL( ch )
       &&   ( stone == NULL || stone->item_type != ITEM_WARP_STONE ) )
  {
    send_to_char( "You lack the proper component for this spell.\n\r", ch );
    return;
  }

  if ( stone && stone->item_type == ITEM_WARP_STONE )
  {
    act( "You draw upon the power of $p.", ch, stone, NULL, TO_CHAR );
    stone->value[1] -= number_percent() <= 25 ? 2 : 1;
    if ( stone->value[1] <= 0 )
    {
      act( "It flares brightly and vanishes!",
           ch, stone, NULL, TO_CHAR );
      extract_obj( stone );
    }
    else
    {
      act( "It flares brightly!", ch, stone, NULL, TO_CHAR );
    }
  }

  /* portal one */
  portal = create_object( get_obj_index( OBJ_VNUM_PORTAL ), level );
  portal->timer = 1 + level / 10;
  SET_BIT( portal->value[1], EX_SEETHROUGH );
  portal->value[3] = vch->in_room->vnum;

  obj_to_room( portal, ch->in_room );

  act( "$p rises up from the ground.", ch, portal, NULL, TO_ROOM );
  act( "$p rises up before you.", ch, portal, NULL, TO_CHAR );

  /* no second portal if rooms are the same */
  if ( vch->in_room == ch->in_room )
    return;

  /* portal two */
  portal = create_object( get_obj_index( OBJ_VNUM_PORTAL ), level );
  portal->timer = 1 + level/10;
  SET_BIT( portal->value[1], EX_SEETHROUGH );
  portal->value[3] = ch->in_room->vnum;

  obj_to_room( portal, vch->in_room );

  if ( vch->in_room->people != NULL )
  {
    act( "$p rises up from the ground.",
         vch->in_room->people, portal, NULL, TO_ROOM );
    act( "$p rises up from the ground.",
         vch->in_room->people, portal, NULL, TO_CHAR );
  }

}

bool check_gate(CHAR_DATA *ch,char *target, int spell_level, int sn)
{
  CHAR_DATA *victim;
  if (( victim = get_char_world( ch, target ) ) == NULL)
    return FALSE;

  if ( victim == ch
       ||   victim->in_room == NULL
       ||   !can_see_room(ch,victim->in_room)
       ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
       ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
       ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
       ||   IS_SET(victim->in_room->room_flags, ROOM_NOMAGIC)
       ||   IS_AFFECTED(ch,AFF_CURSE)
       ||   IS_SAFFECTED(ch,SAFF_ADRENALIZE)
       ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
       ||   IS_AFFECTED(ch,AFF_CHARM)
       ||   ((ch->gameclass == cDruid) && (ch->pcdata->learned[sn] < 90) && (!IS_OUTSIDE(victim)))
       ||   (!IS_NPC(victim) && victim->level > LEVEL_HERO)  /* NOT trust */
       ||   (IS_NPC(victim) && saves_spell( spell_level, victim,DAM_OTHER)))
    return FALSE;

  if (IS_SET(victim->in_room->room_flags, ROOM_NEWBIES_ONLY)
      && ( ch->level > LEVEL_NEWBIE ) // never know, some class could get L1 gate...
      && !( ch->clan && IS_SET( ch->clan->clan_flags, CLAN_GWYLIAD ) ) )
    return FALSE;

//  if ( is_same_group(ch, victim)
//  &&   victim->level <= spell_level + 3 )
//    return TRUE;

  if ((victim->level > spell_level + 3)
      ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE) )
    return FALSE;

  return TRUE;
}
void spell_shadow_walk( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA  *vch;
  AREA_DATA *area;
  bool shadow_pet, can_walk, found = FALSE;
  int count, number, vnum;
  char arg[MSL], buf[MSL];
  ROOM_INDEX_DATA *room=NULL;

  number = number_argument (target_name, arg);
  count = 0;
  printf_to_char(ch, "--%d--   --%s--\n\r", number, arg);

  if (!str_cmp(arg, ""))
  {
    send_to_char("Where do you want the shadows to take you?\n\r", ch);
    return;
  }

  for (area = area_first; area; area = area->next)
  {
    for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++)
    {
      if ( !(room = get_room_index(vnum) ) )
        continue;
      strip_color(buf, to_upper(room->name));
      if (str_prefix(to_upper(arg), buf) )
        continue;
      if ((++count) == number)
      {
        found = TRUE;
        break;
      }
    }
    if (found)
      break;
  }

  if (!found || room == NULL)
  {
    send_to_char("No such place.\n\r", ch);
    return;
  }

  if (room == ch->in_room)
  {
    send_to_char("There is a slight shimmer in the light pattern on the floor.\n\r", ch);
    return;
  }

  can_walk = TRUE;

  /*if (!IS_SET(ch->in_room->room_flags, ROOM_DARK)
      || (!IS_SET(ch->in_room->room_flags, ROOM_INDOORS) && !IS_NIGHT)
      || ch->in_room->light > 0)
    can_walk = FALSE;

  if (!IS_SET(room->room_flags, ROOM_DARK)
      || (!IS_SET(room->room_flags, ROOM_INDOORS) && !IS_NIGHT)
      || room->light > 0)
    can_walk = FALSE;*/

  if (ch->in_room->light > 0)
    can_walk = FALSE;
  if (ch->in_room->sector_type == SECT_INSIDE && !IS_SET(ch->in_room->room_flags, ROOM_DARK))
    can_walk = FALSE;
  if (ch->in_room->sector_type == SECT_UNDERGROUND && !IS_SET(ch->in_room->room_flags, ROOM_DARK))
    can_walk = FALSE;
  if (ch->in_room->sector_type == SECT_CITY
      && !IS_SET(ch->in_room->room_flags, ROOM_DARK)
      && !IS_NIGHT)
    can_walk = FALSE;

  if (room->light > 0)
    can_walk = FALSE;
  if (room->sector_type == SECT_INSIDE && !IS_SET(room->room_flags, ROOM_DARK))
    can_walk = FALSE;
  if (room->sector_type == SECT_UNDERGROUND && !IS_SET(room->room_flags, ROOM_DARK))
    can_walk = FALSE;
  if (ch->in_room->sector_type == SECT_CITY
      && !IS_SET(ch->in_room->room_flags, ROOM_DARK)
      && !IS_NIGHT)
    can_walk = FALSE;


  if (!can_walk)
  {
    send_to_char("There are not enough shadows to walk.\n\r", ch);
    return;
  }

  if (IS_SET(room->room_flags, ROOM_SAFE)
      || IS_SET(room->room_flags, ROOM_SOLITARY)
      || IS_SET(room->room_flags, ROOM_PRIVATE)
      || IS_SET(room->room_flags, ROOM_NO_RECALL)
      || IS_SET(room->room_flags, ROOM_NOMAGIC)
      || IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
      || IS_SET(room->room_flags, ROOM_IMP_ONLY)
      || IS_SET(room->room_flags, ROOM_GODS_ONLY)
      || (IS_SET(room->room_flags, ROOM_HEROES_ONLY) && ch->level < LEVEL_HERO)
      || (IS_SET(room->room_flags, ROOM_NEWBIES_ONLY) && ch->level > LEVEL_NEWBIE)
      || IS_AFFECTED(ch, AFF_CURSE)
      || IS_SAFFECTED(ch, SAFF_ADRENALIZE)
      || IS_AFFECTED(ch, AFF_CHARM)
      || ch->level < room->area->low_range)
  {
    send_to_char("The shadows do not move.  You failed.\n\r", ch);
    return;
  }

  for (vch = room->people; vch != NULL; vch = vch->next_in_room)
  {
    if (vch->clan)
    {
      send_to_char("Trying to jump another clan is cheap!\n\r", ch);
      return;
    }
  }

  if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
    shadow_pet = TRUE;
  else
    shadow_pet = FALSE;

  if ((ch->level < MAX_LEVEL - 9) && (ch->incog_level))
  {
    send_to_char("{RRemoving Cloaking{x\n\r", ch);
    ch->incog_level = 0;
  }
  send_to_char("The shadows gather around you and you vanish.\n\r\n\r", ch);
  act("The shadows shift around in the room.", ch, NULL, NULL, TO_ROOM);
  move_to_room(ch, room);
  act("The shadows shift around in the room and someone steps out of them.",
      ch, NULL, NULL, TO_ROOM);
  do_function(ch, &do_look, "auto");


  if (shadow_pet)
  {
    send_to_char("The shadows gather around you and you vanish.\n\r\n\r", ch->pet);
    ch->pet->position = POS_STANDING;
    move_to_room(ch->pet, room);
    do_function(ch->pet, &do_look, "auto");
  }
}
