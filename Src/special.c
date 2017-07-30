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
*    ROM 2.4 is copyright 1993-1996 Russ Taylor               *
*    ROM has been brought to you by the ROM consortium           *
*        Russ Taylor (rtaylor@efn.org)                   *
*        Gabrielle Taylor                           *
*        Brian Moore (zump@rom.org)                       *
*    By using this code, you have agreed to follow the terms of the       *
*    ROM license, in the file Rom24/doc/rom.license               *
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
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "mob_cmds.h"
#include "special.h"
#include "tables.h"
#include "interp.h"

/* the function table */
const   struct  spec_type    spec_table[] =
{
  {"spec_breath_any",       spec_breath_any },
  {"spec_breath_acid",      spec_breath_acid },
  {"spec_breath_fire",      spec_breath_fire },
  {"spec_breath_frost",     spec_breath_frost },
  {"spec_breath_gas",       spec_breath_gas },
  {"spec_breath_lightning", spec_breath_lightning },
  {"spec_cast_adept",       spec_cast_adept },
  {"spec_cast_cleric",      spec_cast_cleric },
  {"spec_cast_judge",       spec_cast_judge },
  {"spec_cast_mage",        spec_cast_mage },
  {"spec_cast_battlemage",  spec_cast_battlemage },
  {"spec_cast_undead",      spec_cast_undead },
  {"spec_executioner",      spec_executioner },
  {"spec_fido",             spec_fido },
  {"spec_guard",            spec_guard },
  {"spec_shadowguard",      spec_shadowguard },
  {"spec_janitor",          spec_janitor },
  {"spec_mayor",            spec_mayor },
  {"spec_poison",           spec_poison },
  {"spec_thief",            spec_thief },
  {"spec_nasty",            spec_nasty },
  {"spec_troll_member",     spec_troll_member },
  {"spec_ogre_member",      spec_ogre_member },
  {"spec_questmaster",      spec_questmaster },
  {"spec_patrolman",        spec_patrolman },
  {"spec_tracker",          spec_tracker },
  {"spec_assassin",         spec_assassin },
  {"spec_taxidermist",      spec_taxidermist },
  {"spec_snake_charmer",    spec_snake_charmer },
  {"spec_dreddguard",       spec_dreddguard },
  {"spec_terrorist",        spec_terrorist },
  {"cast_druid",            spec_cast_druid },
  {NULL,                    NULL }
};

/*
 * Given a name, return the appropriate spec fun.
 */
SPEC_FUN *spec_lookup( const char *name )
{
  int i;
 
  for ( i = 0; spec_table[i].name; i++ )
  {
    if ( LOWER(name[0] ) == LOWER( spec_table[i].name[0] )
    &&  !str_prefix( name,spec_table[i].name ) )
      return spec_table[i].function;
  }
  return 0;
}

char *spec_name( SPEC_FUN *function )
{
    int i;

    for ( i = 0; spec_table[i].function; i++ )
    {
      if ( function == spec_table[i].function )
        return spec_table[i].name;
    }
    return NULL;
}

bool spec_troll_member( CHAR_DATA *ch)
{
  CHAR_DATA *vch, *victim = NULL;
  int count = 0;
  char *message;

  if (!IS_AWAKE(ch) || IS_AFFECTED(ch,AFF_CALM) || ch->in_room == NULL 
      ||  IS_AFFECTED(ch,AFF_CHARM) || ch->fighting != NULL)
    return FALSE;

  /* find an ogre to beat up */
  for ( vch = ch->in_room->people;  vch;  vch = vch->next_in_room )
    {
      if ( !IS_NPC( vch ) || ch == vch )
        continue;

      if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
        return FALSE;

      if (vch->pIndexData->group == GROUP_VNUM_OGRES
      &&  ch->level > vch->level - 2 && !is_safe(ch,vch, TRUE))
    {
      if (number_range(0,count) == 0)
        victim = vch;

      count++;
    }
    }

  if (victim == NULL)
    return FALSE;

  /* say something, then raise hell */
  switch (number_range(0,6))
    {
    case 0:    message = "$n yells 'I've been looking for you, punk!'";
      break;
    case 1: message = "With a scream of rage, $n attacks $N.";
      break;
    case 2: message = 
          "$n says 'What's slimy Ogre trash like you doing around here?'";
    break;
    case 3: message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
      break;
    case 4: message = "$n says 'There's no cops to save you this time!'";
      break;    
    case 5: message = "$n says 'Time to join your brother, spud.'";
      break;
    case 6: message = "$n says 'Let's rock.'";
      break;
    default:  message = NULL;     break;
    }

  if ( message )
    act( message, ch, NULL, victim, TO_ALL );
  multi_hit( ch, victim, TYPE_UNDEFINED );
  return TRUE;
}

bool spec_ogre_member( CHAR_DATA *ch )
{
  CHAR_DATA *vch, *victim = NULL;
  int count = 0;
  char *message;
 
  if (!IS_AWAKE(ch) || IS_AFFECTED(ch,AFF_CALM) || ch->in_room == NULL
      ||  IS_AFFECTED(ch,AFF_CHARM) || ch->fighting != NULL)
    return FALSE;

  /* find an troll to beat up */
  for (vch = ch->in_room->people;  vch != NULL;  vch = vch->next_in_room)
    {
      if (!IS_NPC(vch) || ch == vch)
    continue;
 
      if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
    return FALSE;
 
      if (vch->pIndexData->group == GROUP_VNUM_TROLLS
      &&  ch->level > vch->level - 2 && !is_safe(ch,vch, TRUE))
        {
      if (number_range(0,count) == 0)
        victim = vch;
 
      count++;
        }
    }
 
  if (victim == NULL)
    return FALSE;
 
  /* say something, then raise hell */
  switch (number_range(0,6))
    {
    default: message = NULL;    break;
    case 0: message = "$n yells 'I've been looking for you, punk!'";
      break;
    case 1: message = "With a scream of rage, $n attacks $N.'";
      break;
    case 2: message =
          "$n says 'What's Troll filth like you doing around here?'";
    break;
    case 3: message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
      break;
    case 4: message = "$n says 'There's no cops to save you this time!'";
      break;
    case 5: message = "$n says 'Time to join your brother, spud.'";
      break;
    case 6: message = "$n says 'Let's rock.'";
      break;
    }
 
  if (message != NULL)
    act(message,ch,NULL,victim,TO_ALL);
  multi_hit( ch, victim, TYPE_UNDEFINED );
  return TRUE;
}

bool spec_patrolman(CHAR_DATA *ch)
{
  CHAR_DATA *vch,*victim = NULL;
  OBJ_DATA *obj;
  char *message;
  int count = 0;

  if (!IS_AWAKE(ch) || IS_AFFECTED(ch,AFF_CALM) || ch->in_room == NULL
      ||  IS_AFFECTED(ch,AFF_CHARM) || ch->fighting != NULL)
    return FALSE;

  /* look for a fight in the room */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (vch == ch)
    continue;

      if (vch->fighting != NULL)  /* break it up! */
    {
      if (number_range(0,count) == 0)
        victim = (vch->level > vch->fighting->level) 
          ? vch : vch->fighting;
      count++;
    }
    }

  if (victim == NULL || (IS_NPC(victim) && victim->spec_fun == ch->spec_fun))
    return FALSE;

  if (((obj = get_eq_char(ch,WEAR_NECK_1)) != NULL 
       &&   obj->pIndexData->vnum == OBJ_VNUM_WHISTLE)
      ||  ((obj = get_eq_char(ch,WEAR_NECK_2)) != NULL
       &&   obj->pIndexData->vnum == OBJ_VNUM_WHISTLE))
    {
      act("You blow down hard on $p.",ch,obj,NULL,TO_CHAR);
      act("$n blows on $p, ***WHEEEEEEEEEEEET***",ch,obj,NULL,TO_ROOM);

      for ( vch = char_list; vch != NULL; vch = vch->next )
        {
      if ( vch->in_room == NULL )
        continue;

      if (vch->in_room != ch->in_room 
          &&  vch->in_room->area == ch->in_room->area)
        send_to_char( "You hear a shrill whistling sound.\n\r", vch );
        }
    }

  switch (number_range(0,6))
    {
    default:    message = NULL;        break;
    case 0:    message = "$n yells 'All roit! All roit! break it up!'";
      break;
    case 1: message = 
          "$n says 'Society's to blame, but what's a bloke to do?'";
    break;
    case 2: message = 
          "$n mumbles 'bloody kids will be the death of us all.'";
    break;
    case 3: message = "$n shouts 'Stop that! Stop that!' and attacks.";
      break;
    case 4: message = "$n pulls out his billy and goes to work.";
      break;
    case 5: message = 
          "$n sighs in resignation and proceeds to break up the fight.";
    break;
    case 6: message = "$n says 'Settle down, you hooligans!'";
      break;
    }

  if (message != NULL)
    act(message,ch,NULL,NULL,TO_ALL);

  multi_hit(ch,victim,TYPE_UNDEFINED);

  return TRUE;
}
    

bool spec_nasty( CHAR_DATA *ch )
{
  CHAR_DATA *victim, *v_next;
  long gold;
 
  if (!IS_AWAKE(ch)) {
    return FALSE;
  }
 
  if (ch->position != POS_FIGHTING) {
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
        v_next = victim->next_in_room;

        if (!IS_NPC(victim)
        && (victim->level > ch->level)
        && (victim->level < ch->level + 10))
      {
        do_function(ch, &do_backstab,victim->name);

          if (ch->position != POS_FIGHTING
        &&  can_see(ch, victim) )
            do_function(ch, &do_murder,victim->name);

          // should steal some coins right away? :)
        return TRUE;
      }
    }

    return FALSE;    /*  No one to attack */
  }
 
  /* okay, we must be fighting.... steal some coins and flee */
  if ( (victim = ch->fighting) == NULL)
    return FALSE;   /* let's be paranoid.... */
 
  switch ( number_bits(2) )
    {
    case 0:
      if ((victim->gold > 9) || (victim->silver > 900))
      {
        act( "$n rips apart your coin purse, spilling your {yg{Yo{yld{x!",
          ch, NULL, victim, TO_VICT);
        act( "You slash apart $N's coin purse and gather his {yg{Yo{yld{x.",
          ch, NULL, victim, TO_CHAR);
        act( "$N's coin purse is ripped apart!",
          ch, NULL, victim, TO_NOTVICT);
        gold = victim->gold / 10;  /* steal 10% of his gold */
        victim->gold -= gold;
        ch->gold     += gold;

        gold = victim->silver / 10;  /* steal 5% of his silver */
        victim->silver -= gold;
        ch->silver     += gold;
      }
      return TRUE;
 
    case 1:  
      if (!is_affected(ch, skill_lookup("confine") ) && !IS_SET(ch->act, ACT_SENTINEL) )
    {
          do_function(ch, &do_flee, "");
            return TRUE;
    }
      return FALSE;
 
    default: return FALSE;
    }
}

/*
 * Core procedure for dragons.
 */
bool dragon( CHAR_DATA *ch, char *spell_name )
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  int sn;

  if ((sn = skill_lookup(spell_name)) < 0)
    return FALSE;

  if (ch->level < skill_table[sn].skill_level[class_lookup("warlock")])
    return FALSE;

  if ( ch->position != POS_FIGHTING )
    return FALSE;

  for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
      v_next = victim->next_in_room;
      if ( victim->fighting == ch && number_bits( 3 ) == 0 )
    break;
    }

  if ( victim == NULL )
    return FALSE;

  if ( ( sn = skill_lookup( spell_name ) ) < 0 )
    return FALSE;
  (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, TARGET_CHAR);
  return TRUE;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any( CHAR_DATA *ch )
{
  if ( ch->position != POS_FIGHTING )
    return FALSE;

  switch ( number_bits( 3 ) )
    {
    case 0: return spec_breath_fire        ( ch );
    case 1:
    case 2: return spec_breath_lightning    ( ch );
    case 3: return spec_breath_gas        ( ch );
    case 4: return spec_breath_acid        ( ch );
    case 5:
    case 6:
    case 7: return spec_breath_frost        ( ch );
    }

  return FALSE;
}



bool spec_breath_acid( CHAR_DATA *ch )
{
  if ( is_affected( ch, gsn_nerve ) )
    return FALSE;

  return dragon( ch, "acid breath" );
}



bool spec_breath_fire( CHAR_DATA *ch )
{
  if ( is_affected( ch, gsn_nerve ) )
    return FALSE;

  return dragon( ch, "fire breath" );
}



bool spec_breath_frost( CHAR_DATA *ch )
{
  if ( is_affected( ch, gsn_nerve ) )
    return FALSE;  

  return dragon( ch, "frost breath" );
}



bool spec_breath_gas( CHAR_DATA *ch )
{
/* Taeloch: Why not this?  it works and gas_breath is killer and way more powerful than any of the others...
  return dragon( ch, "gas breath" );
*/
  int sn;

  if ( is_affected( ch, gsn_nerve ) )
    return FALSE;

  if ( ch->position != POS_FIGHTING )
    return FALSE;

  if ( ( sn = skill_lookup( "gas breath" ) ) < 0 )
    return FALSE;
  (*skill_table[sn].spell_fun) ( sn, ch->level, ch, NULL,TARGET_CHAR);
  return TRUE;

}



bool spec_breath_lightning( CHAR_DATA *ch )
{
  if ( is_affected( ch, gsn_nerve ) )
    return FALSE;

  return dragon( ch, "lightning breath" );
}



bool spec_cast_adept( CHAR_DATA *ch )
{
  CHAR_DATA *victim;

  if ( !IS_AWAKE(ch) )
    return FALSE;

  if ( is_affected( ch, skill_lookup( "nerve pinch" ) ) )
    return FALSE;

  for ( victim = ch->in_room->people; victim; victim =  victim->next_in_room)

    {
      if ( victim != ch && can_see( ch, victim ) && number_bits( 1 ) == 0 
       && !IS_NPC(victim) && victim->level < 11)
    break;
    }

  if ( victim == NULL )
    return FALSE;
  if ( ch->position < POS_SLEEPING )
    return FALSE;
  if ( IS_GHOST( ch ) )
    return FALSE;

  switch ( number_bits( 4 ) )
    {
    case 0:

      if (!is_affected(victim,skill_lookup("armor"))){ 
    act( "$n utters the word 'abrazak'.", ch, NULL, NULL, TO_ROOM );
          spell_armor( skill_lookup( "armor" ), ch->level,ch,victim,TARGET_CHAR);
          return TRUE;
      }
      return FALSE; 
    case 1:
      if (!is_affected(victim,skill_lookup("bless"))) {
        act( "$n utters the word 'fido'.", ch, NULL, NULL, TO_ROOM );
        spell_bless( skill_lookup( "bless" ), ch->level,ch,victim,TARGET_CHAR);
        return TRUE;
      }
      return FALSE;

    case 2:
      if (IS_AFFECTED(victim,AFF_BLIND)) {
        act("$n utters the words 'judicandus noselacri'.",ch,NULL,NULL,TO_ROOM);
        spell_cure_blindness( skill_lookup( "cure blindness" ),
                ch->level, ch, victim,TARGET_CHAR);
        return TRUE;
      }

      return FALSE;

    case 3:
      if (victim->hit<victim->max_hit) {
        act("$n utters the words 'judicandus dies'.", ch,NULL, NULL, TO_ROOM );
        spell_cure_light( skill_lookup( "cure light" ),
            ch->level, ch, victim,TARGET_CHAR);
        return TRUE;
      }

      return FALSE;

    case 4:
      if (IS_AFFECTED(victim,AFF_POISON)) {
        act( "$n utters the words 'judicandus sausabru'.",ch,NULL,NULL,TO_ROOM);
        spell_cure_poison( skill_lookup( "cure poison" ),
             ch->level, ch, victim,TARGET_CHAR);
        return TRUE;
      }

      return FALSE;

    case 5:
      if (victim->move < victim->max_move){
        act("$n utters the word 'candusima'.", ch, NULL, NULL, TO_ROOM );
        spell_refresh( skill_lookup("refresh"),ch->level,ch,victim,TARGET_CHAR);
        return TRUE;
      }

      return FALSE;

    case 6:
      if (IS_AFFECTED(victim,AFF_PLAGUE)) {
    act("$n utters the words 'judicandus eugzagz'.",ch,NULL,NULL,TO_ROOM);
        spell_cure_disease(skill_lookup("cure disease"),
             ch->level,ch,victim,TARGET_CHAR);
    return TRUE;
      }

      return FALSE;

    }

  return FALSE;
}



bool spec_cast_cleric( CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  char *spell;
  int min_level;
  int sn;

  if ( ch->position != POS_FIGHTING )
    return FALSE;

  if ( is_affected( ch, skill_lookup( "nerve pinch" ) ) )
    return FALSE;

  for ( victim = ch->in_room->people; victim; victim =  victim->next_in_room)
    {
      if ( victim->fighting == ch && number_bits( 2 ) == 0 )
    break;
    }

  if ( victim == NULL )
    return FALSE;


  switch ( number_bits( 4 ) )
    {    
    case  0: min_level =  0; spell = "blindness";      break;
    case  1: min_level =  3; spell = "cause serious";  break;
    case  2: min_level =  7; spell = "earthquake";     break;
    case  3: min_level =  9; spell = "cause critical"; break;
    case  4: min_level = 10; spell = "dispel evil";    break;
    case  5: min_level = 12; spell = "curse";          break;
    case  6: min_level = 12; spell = "change sex";     break;
    case  7: min_level = 13; spell = "flamestrike";    break;
    case  8:
    case  9:
    case 10: min_level = 15; spell = "harm";           break;
    case 11: min_level = 15; spell = "plague";       break;
    default: min_level = 16; spell = "dispel magic";   break;
    }

  if ( ( sn = skill_lookup( spell ) ) < 0 )
    return FALSE;

  min_level = skill_table[sn].skill_level[class_lookup("priest")];
  
  if (min_level < 0)
    return FALSE;

  if (ch->level < min_level)
    return FALSE;

  char buf[MSL];
  sprintf( buf, "'%s' %s", spell, IS_NPC( victim ) ? victim->short_descr : victim->name );
  do_function( ch, &do_cast, buf );

//  (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
  return TRUE;
}

bool spec_cast_judge( CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  char *spell;
  int sn;
 
  if ( is_affected( ch, skill_lookup( "nerve pinch" ) ) )
    return FALSE;

  if ( ch->position != POS_FIGHTING )
    return FALSE;
 
  for ( victim = ch->in_room->people; victim != NULL; victim =
      victim->next_in_room)
    {
      if ( victim->fighting == ch && number_bits( 2 ) == 0 )
    break;
    }
 
  if ( victim == NULL )
    return FALSE;
 
  spell = "judgement";
  if ( ( sn = skill_lookup( spell ) ) < 0 )
    return FALSE;
  (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
  return TRUE;
}



bool spec_cast_mage( CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  char *spell;
  int min_level;
  int sn;
  int spell_range;

  if ( is_affected( ch, skill_lookup( "nerve pinch" ) ) )
    return FALSE;

  if ( ch->position != POS_FIGHTING )
    return FALSE;

  for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
  {
    if ( ( victim->fighting == ch )
    &&   ( number_bits( 2 ) < 2 ) )
      break;
  }

  if ( victim == NULL )
    return FALSE;

/* This will ensure low-level spec_mages cast at a reasonable rate */
  if (ch->level < 10)
    spell_range = 3;
  else if (ch->level < 15)
    spell_range = 7;
  else if (ch->level < 20)
    spell_range = 11;
  else if (ch->level < 25)
    spell_range = 13;
  else if (ch->level < 30)
    spell_range = 15;
  else if (ch->level < 40)
    spell_range = 16;
  else
    spell_range = 20;
// The last # is 20 so that incinerate is favored for those that can cast it

  switch ( number_range( 0, spell_range ) )
  {
    case  0: spell = "magic missile";  min_level =  1; break;
    case  1: spell = "shocking grasp"; min_level =  3; break;
    case  2: spell = "chill touch";    min_level =  4; break;
    case  3: spell = "burning hands";  min_level =  7; break;
    case  4: spell = "lightning bolt"; min_level = 11; break;
    case  5: spell = "weaken";         min_level = 11; break;
    case  6: spell = "blindness";      min_level = 12; break;
    case  7: spell = "teleport";       min_level = 13; break;
    case  8: spell = "colour spray";   min_level = 16; break;
    case  9: spell = "curse";          min_level = 18; break;
    case 10: spell = "change sex";     min_level = 19; break;
    case 11: spell = "cone of cold";   min_level = 19; break;
    case 12: spell = "fireball";       min_level = 22; break;
    case 13: spell = "slow";           min_level = 23; break;
    case 14: spell = "sonic blast";    min_level = 25; break;
    case 15: spell = "icicle";         min_level = 28; break;
    case 16: spell = "acid blast";     min_level = 30; break;
    default: spell = "incinerate";     min_level = 40; break;
  }

  if ( ( sn = skill_lookup( spell ) ) < 0 )
    return FALSE;

  min_level = skill_table[sn].skill_level[class_lookup("conjurer")];

  if (min_level < 0)
    return FALSE;

  if (ch->level < min_level)
    return FALSE;

  (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
// why did we ever go to do_cast?  It works worse.
//  do_function( ch, &do_cast, buf );

  return TRUE;
}



bool spec_cast_undead( CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  char *spell;
  int min_level;
  int sn;

  if ( is_affected( ch, skill_lookup( "nerve pinch" ) ) )
    return FALSE;

  if ( ch->position != POS_FIGHTING )
    return FALSE;

  for ( victim = ch->in_room->people; victim != NULL; victim =
      victim->next_in_room)
    {
      if ( victim->fighting == ch && number_bits( 2 ) == 0 )
    break;
    }

  if ( victim == NULL )
    return FALSE;
            

  switch ( number_bits( 4 ) )
    {
    case  0: min_level =  0; spell = "curse";          break;
    case  1: min_level =  3; spell = "weaken";         break;
    case  2: min_level =  6; spell = "chill touch";    break;
    case  3: min_level =  9; spell = "blindness";      break;
    case  4: min_level = 12; spell = "poison";         break;
    case  5: min_level = 15; spell = "energy drain";   break;
    case  6: min_level = 18; spell = "harm";           break;
    case  7: min_level = 21; spell = "teleport";       break;
    case  8: min_level = 20; spell = "plague";       break;
    default: min_level = 18; spell = "harm";           break;
    }

  if ( ( sn = skill_lookup( spell ) ) < 0 )
    return FALSE;

  min_level = skill_table[sn].skill_level[class_lookup("mage")];
  if (skill_table[sn].skill_level[class_lookup("priest")] > min_level)
    min_level = skill_table[sn].skill_level[class_lookup("priest")];

  if (min_level < 0)
    return FALSE;

  if (ch->level < min_level)
    return FALSE;

  char buf[MSL];
  sprintf( buf, "'%s' %s", spell, IS_NPC( victim ) ? victim->short_descr : victim->name );
  do_function( ch, &do_cast, buf );

//  (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR);
  return TRUE;
}


bool spec_executioner( CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *crime;

  if ( !IS_AWAKE(ch) || ch->fighting != NULL )
    return FALSE;

  if (IS_SET(ch->in_room->room_flags,ROOM_SAFE))
    return FALSE;

  crime = "";
  for ( victim = ch->in_room->people; victim; victim = v_next )
    {
      v_next = victim->next_in_room;

      if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER) 
       &&   can_see(ch,victim))
    { crime = "a KILLER"; break; }

      if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF) 
       &&   can_see(ch,victim))
    { crime = "a THIEF"; break; }

      if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_TWIT) 
       &&   can_see(ch,victim))
    { crime = "a TWIT"; break; }

      if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_VIOLENT) 
       &&   can_see(ch,victim))
    { crime = "VIOLENT"; break; }
    }

  if ( victim == NULL )
    return FALSE;
    
  if (IS_IMMORTAL(victim))
    return FALSE;

  if (IS_GHOST(victim))
    return FALSE;

  if (IS_AFK(victim))
    return FALSE;

  if (IS_LINKDEAD(victim))
    return FALSE;

  if (!can_see(ch,victim))
    return FALSE;

  mprintf( sizeof(buf), buf, "%s is %s!  PROTECT THE INNOCENT!  REVENGE!!!!",
       victim->name, crime );
  REMOVE_BIT(ch->chan_flags,CHANNEL_NOSHOUT);
  do_function(ch, &do_yell, buf );
  multi_hit( ch, victim, TYPE_UNDEFINED );
  return TRUE;
}

            

bool spec_fido( CHAR_DATA *ch )
{

  CHAR_DATA *vch;
  OBJ_DATA *corpse;
  OBJ_DATA *c_next;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;

  if ( !IS_AWAKE(ch) )
    return FALSE;

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
  {
      if ( !IS_NPC( vch ) )
      {
        if (number_range(1,25)==5)
        {
            act( "$n suddenly bites you.",ch,NULL,vch,TO_VICT );
            act( "$n suddenly bites $N.",ch,NULL,vch,TO_NOTVICT );
            act( "You angrily bites $N.",ch,NULL,vch,TO_CHAR );

            return TRUE; 
        }
      }
  }

  for ( corpse = ch->in_room->contents; corpse != NULL; corpse = c_next )
    {
      c_next = corpse->next_content;
      if ( corpse->item_type != ITEM_CORPSE_NPC )
    continue;

      act( "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );
      for ( obj = corpse->contains; obj; obj = obj_next )
    {
      obj_next = obj->next_content;
      obj_from_obj( obj );
      obj_to_room( obj, ch->in_room );
    }
      extract_obj( corpse );
      return TRUE;
    }

  return FALSE;
}

bool spec_guard( CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  CHAR_DATA *ech = NULL;;
  int max_align_diff = 0;
  char crime[MSL];
  strcpy(crime, "");

  if ( !IS_AWAKE(ch)
  ||   (ch->fighting != NULL) )
    return FALSE;

  if (IS_SET(ch->in_room->room_flags,ROOM_SAFE))
    return FALSE;
      
  for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
  {
    v_next = victim->next_in_room;

    if ( !IS_NPC(victim)
    &&    IS_SET(victim->act, PLR_KILLER) 
    &&    can_see(ch,victim) )
    {
      strcpy(crime,"a KILLER");
      break;
    }

    if ( !IS_NPC(victim)
    &&    IS_SET(victim->act, PLR_THIEF) 
    &&    can_see(ch,victim) )
    {
      strcpy(crime,"a THIEF");
      break;
    }

    if ( !IS_NPC(victim)
    &&    IS_SET(victim->act, PLR_TWIT) 
    &&    can_see(ch,victim) )
    {
      strcpy(crime,"a TWIT");
      break;
    }

    if ( !IS_NPC(victim)
    &&    IS_SET(victim->act, PLR_VIOLENT) 
    &&    can_see(ch,victim))
    {
      strcpy(crime,"VIOLENT");
      break;
    }

    if ( ( victim->fighting != NULL )
    &&   ( victim->fighting != ch )
    &&   ( max_align_diff < abs((ch->alignment + 1000) - (victim->alignment + 1000)) ) )
    {
      max_align_diff = abs((ch->alignment + 1000) - (victim->alignment + 1000));
      ech = victim;
    }
  }

  if ( victim != NULL ) // means there was a special-case break
  {
    if (IS_IMMORTAL(victim))
      return FALSE;
    if (IS_GHOST(victim))
      return FALSE;
    if (IS_AFK(victim))
      return FALSE;
    if (IS_LINKDEAD(victim))
      return FALSE;
    if (victim->in_room != ch->in_room)
      return FALSE;
    if (!can_see(ch,victim))
      return FALSE;

    mprintf( sizeof(buf), buf, "%s is %s!  PROTECT THE INNOCENT!!  BANZAI!!", victim->name, crime );
    REMOVE_BIT(ch->chan_flags,CHANNEL_NOSHOUT);
    do_function(ch, &do_yell, buf );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return TRUE;
  }

  if ( ech != NULL ) // found the most mis-aligned fighting person in the room
  {
    if (IS_AFK(ech))
      return FALSE;
    if (IS_LINKDEAD(ech))
      return FALSE;
    if (IS_GHOST(ech))
      return FALSE;
    if (ech->in_room != ch->in_room)
      return FALSE;
    if (!can_see(ch,ech))
      return FALSE;

    act( "$n screams 'PROTECT THE INNOCENT!!  BANZAI!!",
      ch, NULL, NULL, TO_ROOM );
    multi_hit( ch, ech, TYPE_UNDEFINED );
    return TRUE;
  }

  return FALSE;
}



bool spec_janitor( CHAR_DATA *ch )
{
  OBJ_DATA *trash;
  OBJ_DATA *trash_next;

  if ( !IS_AWAKE(ch) )
    return FALSE;

  for ( trash = ch->in_room->contents; trash != NULL; trash = trash_next )
  {
      trash_next = trash->next_content;

      if ( !IS_SET( trash->wear_flags, ITEM_TAKE )
      || !can_loot(ch,trash)
      || IS_SET(trash->extra_flags, ITEM_RESTRING)
      || trash->item_type == ITEM_FURNITURE )
//      || trash->item_type == ITEM_CORPSE_PC )
        continue;

      if ( ( trash->item_type == ITEM_DRINK_CON
      ||   trash->item_type == ITEM_TRASH
      ||   trash->cost < 10 )
      && ( trash->item_type != ITEM_MONEY ) )
      {
        act( "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
        obj_from_room( trash );
        obj_to_char( trash, ch );
        return TRUE;
      }
  }

  return FALSE;

}



bool spec_mayor( CHAR_DATA *ch )
{
  static const char open_path[] =
    "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static const char close_path[] =
    "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static const char *path;
  static int pos;
  static bool move;

  if ( !move )
    {
      if ( time_info.hour ==  6 )
    {
      path = open_path;
      move = TRUE;
      pos  = 0;
    }

      if ( time_info.hour == 20 )
    {
      path = close_path;
      move = TRUE;
      pos  = 0;
    }
    }

  if ( ch->fighting != NULL )
    return spec_cast_mage( ch );
  if ( !move || ch->position < POS_SLEEPING )
    return FALSE;

  switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
      move_char( ch, path[pos] - '0', FALSE );
      break;

    case 'W':
      ch->position = POS_STANDING;
      act( "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM );
      break;

    case 'S':
      ch->position = POS_SLEEPING;
      act( "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
      break;

    case 'a':
      act( "$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM );
      break;

    case 'b':
      act( "$n says 'What a view!  I must do something about that dump!'",
       ch, NULL, NULL, TO_ROOM );
      break;

    case 'c':
      act( "$n says 'Vandals!  Youngsters have no respect for anything!'",
       ch, NULL, NULL, TO_ROOM );
      break;

    case 'd':
      act( "$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM );
      break;

    case 'e':
      act( "$n says 'I hereby declare the city of Midgaard open!'",
       ch, NULL, NULL, TO_ROOM );
      break;

    case 'E':
      act( "$n says 'I hereby declare the city of Midgaard closed!'",
       ch, NULL, NULL, TO_ROOM );
      break;

    case 'O':
      do_function(ch, &do_open, "gate" );
      break;

    case 'C':
      do_function(ch , &do_close, "gate" );
      break;

    case '.' :
      move = FALSE;
      break;
    }

  pos++;
  return FALSE;
}



bool spec_poison( CHAR_DATA *ch )
{
  CHAR_DATA *victim=NULL;

  if ( ch->position != POS_FIGHTING
       || ( victim = ch->fighting ) == NULL
       ||   number_percent( ) > 2 * ch->level )
    return FALSE;

  act( "You bite $N!",  ch, NULL, victim, TO_CHAR    );
  act( "$n bites $N!",  ch, NULL, victim, TO_NOTVICT );
  act( "$n bites you!", ch, NULL, victim, TO_VICT    );
  spell_poison( gsn_poison, ch->level, ch, victim,TARGET_CHAR);
  return TRUE;
}



bool spec_thief( CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  long gold,silver;

  if ( ch->position != POS_STANDING )
    return FALSE;

  for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
      v_next = victim->next_in_room;

      if ( IS_NPC(victim)
       ||   victim->level >= LEVEL_IMMORTAL
       ||   number_bits( 5 ) != 0 
       ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
       ||   !can_see(ch,victim))
    continue;

      if ( IS_AWAKE(victim) && number_range( 0, ch->level ) == 0 )
    {
      act( "You discover $n's hands in your wallet!",
           ch, NULL, victim, TO_VICT );
      act( "$N discovers $n's hands in $S wallet!",
           ch, NULL, victim, TO_NOTVICT );
      return TRUE;
    }
      else
    {
      gold = victim->gold * UMIN(number_range(1,20),ch->level / 2) / 100;
      gold = UMIN(gold, ch->level * ch->level * 10 );
      ch->gold     += gold;
      victim->gold -= gold;
      silver = victim->silver * UMIN(number_range(1,20),ch->level/2)/100;
      silver = UMIN(silver,ch->level*ch->level * 25);
      ch->silver    += silver;
      victim->silver -= silver;
      return TRUE;
    }
    }

  return FALSE;
}

bool spec_questmaster( CHAR_DATA *ch )
{
  return TRUE;
}

bool spec_assassin( CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  int rnd_say;

  if ( ch->fighting != NULL )
    return FALSE;

  for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
      v_next = victim->next_in_room;
      /* this should kill mobs as well as players */
      if (victim->gameclass != 2)  /* thieves */
    break;
    }

  if ( victim == NULL || victim == ch || IS_IMMORTAL(victim) )
    return FALSE;
  if ( victim->level > ch->level + 7 || IS_NPC(victim))
    return FALSE;
  if (IS_SET(victim->spell_aff, SAFF_DETER));
    return FALSE;
  rnd_say = number_range (1, 10);
                
  if ( rnd_say <= 5)
    mprintf( sizeof(buf), buf, "Death to is the true end...");
  else if ( rnd_say <= 6)
    mprintf( sizeof(buf), buf, "Time to die....");
  else if ( rnd_say <= 7)
    mprintf( sizeof(buf), buf, "Cabrone...."); 
  else if ( rnd_say <= 8)
    mprintf( sizeof(buf), buf, "Welcome to your fate....");
  else if ( rnd_say <= 9)
    mprintf( sizeof(buf), buf, "Another Victim....");
  else if ( rnd_say <= 10)
    mprintf( sizeof(buf), buf, "Ever dance with the devil...."); 

  do_function(ch, &do_say, buf );
  multi_hit( ch, victim, gsn_backstab );
  return TRUE;
}


bool spec_taxidermist( CHAR_DATA *ch )
{
  OBJ_DATA *inv;
  int sn;

  if ( ch->position != POS_STANDING )
    return FALSE;

  if ( ch->pIndexData->pShop == 0 ||
       time_info.hour < ch->pIndexData->pShop->open_hour ||
       time_info.hour > ch->pIndexData->pShop->close_hour )
    return FALSE;

  for ( inv = ch->carrying; inv != NULL; inv = inv->next_content )
    {
      if (inv->item_type == ITEM_CORPSE_NPC)
    {
      if ( ( sn = skill_lookup( "make bag" ) ) >= 0 )
        (*skill_table[sn].spell_fun) ( sn, ch->level, ch, inv, 0 );
      return TRUE;
    }
      else if (inv->wear_loc == WEAR_NONE && number_percent() < 5)
    {
      act( "$n suggests you buy $p.", ch, inv, NULL, TO_ROOM );
      return TRUE;
    }
    }

  return FALSE;
}



/* Ok, here is a spec-fun example for a nasty mobile known as the snake
   charmer.  I rather like this guy, but I never could find a MUD to
   put this code in.

   Here is what happens.  Player attacks snake charmer mobile.  Snake
   charmer then casts CHARM spell on player, and forces player to give
   charmer weapons, sing, and he can even order the player to CHAT that
   the snake charmer is this really great mob to attack.

   I figure this should liven up any egypt or india zone.
*/

bool spec_snake_charmer( CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;

  if ( ch->position != POS_FIGHTING )
    {
      switch ( number_bits( 3 ) ) {
      case 0:
    do_function(ch, &do_order, "all music 'charmer is the man'" ); /* a chance to get free here */
    break;
      case 1:
    do_function(ch, &do_order,
            "all gossip 'This is pretty cool. I'm getting a lot of experience really fast!" );
    break;
      case 2:
    do_function(ch, &do_order,
            "all gossip 'YES!  I just got 327 exp for killing the the mountain king!");
    break;
      case 3:
    do_function(ch, &do_order, "all remove dagger" );
    do_function(ch, &do_order, "all give dagger charmer" );
    break;
      case 4:
    do_function(ch, &do_order, "all remove sword" );
    do_function(ch, &do_order, "all give sword charmer" );
    break;
      case 5:
    do_function(ch, &do_order, "all remove mace" );
    do_function(ch, &do_order, "all give mace charmer" );
    break;
      case 6:
    do_function(ch, &do_order, "all drop all" );
    break;
      case 7:
    do_function(ch, &do_order, "all cast 'cure light' charmer" );
    break;
      };

      return TRUE;
    }

  for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
      v_next = victim->next_in_room;
      if ( victim->fighting == ch && number_bits( 2 ) == 0 )
    break;
    }

  if ( victim == NULL )
    return FALSE;

  act( "$n begins playing a new, beautiful song.", ch, NULL, NULL, TO_ROOM );
  spell_charm_person(gsn_charm_person, ch->level, ch, victim , 0);
  if (IS_AFFECTED(victim, AFF_CHARM))
    stop_fighting( victim, TRUE );

  return TRUE;
}

bool spec_tracker (CHAR_DATA *ch)
{
  if ( ch->tracking!= NULL 
       && ( ( ch->wait <= 0 && ch->fighting == NULL )
        ||       get_char_room( ch, ch->tracking ) ) )
    {
      real_track( ch, ch->tracking );
    }
  ch->wait = UMAX( 0, ch->wait - 1 );
  return FALSE;
}

bool spec_dreddguard( CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *crime;

  if ( !IS_AWAKE(ch) || ch->fighting != NULL )
    return FALSE;

  crime    = "";

  if (IS_SET(ch->in_room->room_flags,ROOM_SAFE))
    return FALSE;

  for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
      v_next = victim->next_in_room;

      if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_TWIT) 
       &&   can_see(ch,victim))
    { crime = "TWIT"; break; }

      if ( !IS_NPC(victim) 
       && IS_SET(victim->act, PLR_VIOLENT)
       && IS_SET(victim->act, PLR_THIEF)
       &&   can_see(ch,victim))
    { crime = "VIOLENT THIEF"; break; }

      if ( !IS_NPC(victim) 
       && IS_SET(victim->act, PLR_VIOLENT)
       && IS_SET(victim->act, PLR_KILLER)
       &&   can_see(ch,victim))
    { crime = "VIOLENT KILLER"; break; }

    }

  if ( victim != NULL )
    {
      if (IS_IMMORTAL(victim))
    return FALSE;
      if (IS_AFK(victim))
    return FALSE;
      if (IS_GHOST(victim))
    return FALSE;
      if (IS_LINKDEAD(victim))
    return FALSE;
      mprintf( sizeof(buf), buf, "%s is to BE JUDGED!!! For your crime of being a %s you will DIE",
          victim->name, crime );
      REMOVE_BIT(ch->chan_flags,CHANNEL_NOSHOUT);
      do_function(ch, &do_yell, buf );
      multi_hit( ch, victim, TYPE_UNDEFINED );
      return TRUE;
    }
  return FALSE;
}

bool spec_shadowguard( CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  CHAR_DATA *ech;
  char *crime;
  int max_evil;

  if ( !IS_AWAKE(ch) || ch->fighting != NULL )
    return FALSE;

  max_evil = 300;
  ech      = NULL;
  crime    = "";

  if (IS_SET(ch->in_room->room_flags,ROOM_SAFE))
    return FALSE;
  for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
      v_next = victim->next_in_room;
      if (victim->level <= 20)
    continue;
      if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_TWIT) 
       &&   can_see(ch,victim))
    { crime = "TWIT"; break; }

      if ( !IS_NPC(victim) && (victim->alignment > 850) 
       &&   can_see(ch,victim))
    { crime = "Nice Person"; break; }

      if ( victim->fighting != NULL
       &&   victim->fighting != ch
       &&   victim->alignment > max_evil )
    {
      max_evil = victim->alignment;
      ech      = victim;
    }
    }

  if ( victim != NULL )
    {
      if (IS_IMMORTAL(victim))
    return FALSE;
      if (IS_AFK(victim))
    return FALSE;
      if (IS_GHOST(victim))
    return FALSE;
      if (IS_LINKDEAD(victim))
    return FALSE;
      mprintf( sizeof(buf), buf, "%s is a %s! Destroy the innocent! Get em!!",
           victim->name, crime );
      REMOVE_BIT(ch->chan_flags,CHANNEL_NOSHOUT);
      do_function(ch, &do_yell, buf );
      multi_hit( ch, victim, TYPE_UNDEFINED );
      return TRUE;
    }

  if ( ech != NULL )
    {
      if (IS_AFK(ech))
    return FALSE;
      if (IS_LINKDEAD(ech))
    return FALSE;
      act( "$n screams 'WHO THE HELL LET THIS NICE PERSON IN HERE!!",
       ch, NULL, NULL, TO_ROOM );
      multi_hit( ch, ech, TYPE_UNDEFINED );
      return TRUE;
    }

  return FALSE;
}


bool spec_terrorist(CHAR_DATA *ch)
{
  static bool move;

  if ( ch->fighting != NULL )
    return spec_cast_battlemage( ch );
  if ( !move || ch->position < POS_SLEEPING ) {
    move = TRUE;
    return FALSE;
  }
  else
    {
      if (move_char(ch, number_range(0,5), FALSE)) {
      act("$n moves and acts just like everyone else.",ch, NULL, NULL,
      TO_ROOM );
      move = FALSE;
      }
      
    }
  if (number_range(0,1000) > 999)
    {
      REMOVE_BIT(ch->pen_flags, PEN_NOCHANNELS);
      martyr_char(ch, gsn_martyr);
      damage(ch, ch, 100000, 1, 1, TRUE, FALSE);
    }
  return TRUE;
}

bool spec_cast_battlemage( CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  int min_level;
  char *spell;
  int sn;

  if ( ch->position != POS_FIGHTING )
    return FALSE;

  for ( victim = ch->in_room->people; victim; victim = victim->next_in_room )
    {
      if ( victim->fighting == ch)
    break;
    }

  if ( victim == NULL )
    return FALSE;


  switch ( number_bits( 3 ) )
    {
    case  0: min_level = 11; spell = "teleport";   break;
    case  1: min_level = 11; spell = "icicle";   break;
    case  2: min_level = 11; spell = "blindness";   break;
    case  3: min_level = 11; spell = "slow";   break;
    case  4: min_level = 11; spell = "incinerate";   break;
    case  5: min_level = 50; spell = "immolation";     break;
    case  6: min_level = 40; spell = "lightning breath";   break;
    case  7: min_level = 22; spell = "cone of cold";       break;
    case  8: min_level = 10; spell = "fireball";   break;
    case  9: min_level = 30; spell = "avalanche";       break;
    case 10: min_level = 65; spell = "devour soul";       break;
    case 11: min_level = 80; spell = "star storm"; break;
    default: min_level = 0; spell = "lightning breath";     break;
    }

  if ( ch->level < min_level )
  {
      min_level = 1;
      spell = "lighning breath";
  }

  if ( ( sn = skill_lookup( spell ) ) < 0 )
    return FALSE;

  min_level = skill_table[sn].skill_level[class_lookup("mage")];
  if (skill_table[sn].skill_level[class_lookup("warlock")] > min_level)
    min_level = skill_table[sn].skill_level[class_lookup("warlock")];
  if (skill_table[sn].skill_level[class_lookup("druid")] > min_level)
    min_level = skill_table[sn].skill_level[class_lookup("druid")];

  
  if (min_level < 0)
    return FALSE;

  if (ch->level < min_level)
    return FALSE;

  char buf[MSL];
  sprintf( buf, "'%s' %s", spell, IS_NPC( victim ) ? victim->short_descr : victim->name );
  do_function( ch, &do_cast, buf );

//  cast_a_spell( sn, ch->level, ch, victim,TARGET_CHAR, FALSE);
  return TRUE;
}

/*                   Original Specs for Lands of Draknor                    */

/*
    spec_cast_druid - Contains druid spells and tactics for mobs.
*/

bool spec_cast_druid( CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  char *spell;
  int min_level;
  int sn;
  OBJ_DATA *obj;

  if ( ch->position != POS_FIGHTING )
    return FALSE;

  if ( is_affected( ch, skill_lookup( "nerve pinch" ) ) )
    return FALSE;

  for ( victim = ch->in_room->people; victim; victim = victim->next_in_room)
    {
      if ( victim->fighting == ch/* && number_bits( 2 ) == 0*/ )
        break;
    }

  if ( victim == NULL )
    return FALSE;

  switch ( number_bits( 3 ) )
    {
    case  0: min_level = 11; spell = "call lightning";  break;
    case  1: min_level = 11; spell = "sunbeam";         break;
    case  2: min_level = 11; spell = "entangle";        break;
    case  3: min_level = 11; spell = "dispel magic";    break;
    case  4: min_level = 11;
             if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
               spell = "create staff";
             else 
               spell = "dispel magic";
                                                        break;
    case  5: min_level = 20; spell = "thunder";         break;
    case  6: min_level = 30; spell = "nirvana";         break;
    case  7: min_level = 15; spell = "phoenix";         break;
    case  8: min_level = 16; spell = "tornado";         break;
    case  9: min_level = 18; spell = "avalanche";       break;
    case 10: min_level = 25; spell = "fatigue";         break;
    case 11: min_level = 40; spell = "cure critical";   break;
    default: min_level = 16; spell = "dispel magic";    break;
    }

  if ( ( sn = skill_lookup( spell ) ) < 0 )
    return FALSE;

  min_level = skill_table[sn].skill_level[class_lookup("druid")];

  if (min_level < 0)
    return FALSE;

  if (ch->level < min_level)
    return FALSE;

  if ( ( min_level = ch->level ) >= LEVEL_HERO )
    min_level = LEVEL_HERO - 1;

  char buf[MSL];
  OBJ_DATA *obj2;

  sprintf( buf, "'%s' %s", spell, IS_NPC( victim ) ? victim->short_descr : victim->name );
  do_function( ch, &do_cast, buf );
//  cast_a_spell( sn, min_level, ch, victim, TARGET_CHAR, FALSE );

  if ( ( ( obj = get_obj_carry( ch, "staff", ch ) ) != NULL )
  &&   ( ( obj2 = get_eq_char( ch, WEAR_WIELD ) ) == NULL ) )
  {
    obj->level = min_level;
    wear_obj( ch, obj, TRUE );
  }

  return TRUE;
}

/*****************************************************************************\
*                                                                             *
*  These are not exactly spec's, but I feel they fit best here -- Merak       *
*                                                                             *
\*****************************************************************************/

void spec_changer( CHAR_DATA *ch, CHAR_DATA *victim, int gold, int silver )
{
  int change;
  char buf[MSL];

  change = ( silver ? 95 * silver / 100 / 100
                    : 95 * gold );

  if ( gold && change > ch->silver )
    ch->silver += change;

  if ( silver && change > ch->gold )
    ch->gold += change;

  if ( change < 1 && can_see( ch, victim ) )
  {
    act( "$n tells you 'I'm sorry, you did not give me enough to change.'",
          ch, NULL, victim, TO_VICT );
    mprintf(sizeof(buf), buf, "%d %s %s",
            silver ? silver : gold, silver ? "silver" : "gold", victim->name );
    do_function( ch, &do_give, buf );
  }
  else if ( can_see( ch, victim ) )
  {
    mprintf(sizeof(buf), buf, "%d %s %s",
            change, silver ? "gold" : "silver", victim->name );
    do_function( ch, &do_give, buf );
    if ( silver )
    {
      mprintf(sizeof(buf), buf, "%d silver %s",
             ( 95 * silver / 100 - change * 100 ), victim->name );
      do_function( ch, &do_give, buf );
    }
    act( "$n tells you 'Thank you, come again.'",
          ch,  NULL, victim, TO_VICT );
  }
  return;
}

void spec_locker( CHAR_DATA *ch, CHAR_DATA *victim, int gold, int silver )
{
  if ( silver )
  {
    act( "$n tells you 'I'm sorry, I dont accept silver.'",
          ch, NULL, victim, TO_VICT );
    victim->silver += silver;
    return;
  }
  if ( victim->pcdata->locker_max > 200 )
  {
    act( "$n tells you 'I'm sorry, you have reached maximum capacity.'",
          ch, NULL, victim, TO_VICT );
    victim->gold += gold;
    return;
  }
  if ( gold < 500 )
  {
    act( "$n tells you 'I'm sorry, you cannot afford to increase your locker privileges.'",
          ch, NULL, victim, TO_VICT );
    victim->gold += gold;
    return;
  }

  if ( gold >= 1000 )
  {
    do_function( ch, &do_say, "One locker space coming right up." );
    victim->pcdata->locker_max += 1;
    spec_locker( ch, victim, gold - 500, 0 );
    return;
  }
  if ( gold > 500 )
  {
    do_function( ch, &do_say,
         "One locker space coming right up.  Thanks for the tip!" );
    victim->pcdata->locker_max += 1;
    return;
  }

  do_function( ch, &do_say, "One locker space coming right up." );
  victim->pcdata->locker_max += 1;
  return;
}

void spec_path( CHAR_DATA *ch )
{
  char  buf[MSL];
  char *tmp;
  int   value;

  if ( ch->path_ptr[0] == '\0' )
  {
    ch->path_ptr = NULL;
    return;
  }
  tmp = ch->path_ptr;
  switch ( *tmp )
  {
    case 'n': case 'e': case 's': case 'w': case 'u': case 'd':
    // walk the direction
    buf[0] = *tmp;
    buf[1] = '\0';
    for ( value = 0; value <= 5; value++ )
      if ( !str_prefix( buf, dir_name[value] ) ) break;
    if ( value == 6 ) break;
    move_char( ch, value, FALSE );
    break;

    case 'C': // Close a door
    tmp++;
    buf[0] = *tmp;
    buf[1] = '\0';
    do_mpclose( ch, buf ); // Think there maybe be a more verbouse function..
    break;

    case 'P':
    for ( value = 0; isdigit( tmp[1] ); )
    {
      tmp++;
      buf[value++] = *tmp;
    }
    MPROG_CODE *prg;
    buf[value] = '\0';
    value = atoi( buf );
    if ( ( prg = get_mprog_index( value ) ) )
      program_flow( prg->vnum, prg->code, ch, NULL, NULL, NULL );
    break;


    case 'O': // Open a door
    tmp++;
    buf[0] = *tmp;
    buf[1] = '\0';
    do_mpopen( ch, buf ); // Think there maybe be a more verbouse function..
    break;

    case 'R': do_mprecall( ch, buf );
    break;

    case 'T':
    for ( value = 0; isdigit( tmp[1] ); )
    {
      tmp++;
      buf[value++] = *tmp;
    }
    buf[value] = '\0';
    value = atoi( buf );
    if ( time_info.hour != value ) return;
    break;

    default: break;
  }
  ch->path_ptr = ++tmp;
}

