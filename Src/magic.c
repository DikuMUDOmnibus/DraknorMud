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
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "magic.h"
#include "recycle.h"
#include "interp.h"
#include "special.h"
/* command procedures needed */

/*
 * Local functions.
 */
void say_spell    args( ( CHAR_DATA *ch, int sn ) );
void cast_a_spell(int sn, int lvl, CHAR_DATA *ch, void *vo, int target, int object);
int  check_proficiency_level(CHAR_DATA *ch, int sn);

/* imported functions */
bool remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void wear_obj    args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );



/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
  int sn;

  for ( sn = 0; sn < MAX_SKILL; sn++ )
  {
    if ( skill_table[sn].name == NULL )
        break;

    if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
    &&   !str_prefix( name, skill_table[sn].name ) )
        return sn;
  }

  return -1;
}

/*
 * Lookup a skill by an exact name.
 */
int skill_lookup_exact( const char *name )
{
  int sn;

  for ( sn = 0; sn < MAX_SKILL; sn++ )
  {
    if ( skill_table[sn].name == NULL )
        break;

    if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
    &&   !str_cmp( name, skill_table[sn].name ) )
        return sn;
  }

  return -1;
}


/*
 * Lookup a skill by name.
 */
int skill_lookup_err( const char *name )
{
  int sn;

  for ( sn = 0; sn < MAX_SKILL; sn++ )
  {
    if ( skill_table[sn].name == NULL )
        break;
    if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
    &&   !str_prefix( name, skill_table[sn].name ) )
        return sn;
  }

  bugf("Skill: %s is not found.\n\r",name);
  return -1;
}

int find_spell( CHAR_DATA *ch, const char *name )
{
  /* finds a spell the character can cast if possible */
  int sn, found = -1;

  if (IS_NPC(ch))
    return skill_lookup(name);

  for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
      if (skill_table[sn].name == NULL)
    break;
      if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
      &&  !str_prefix(name,skill_table[sn].name))
    {
      if ( found == -1)
        found = sn;
      if (ch->level >= skill_table[sn].skill_level[ch->gameclass]
          &&  ch->pcdata->learned[sn] > 0)
        return sn;
    }
    }
  return found;
}



/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
  extern bool fBootDb;
  int sn;

  if ( slot <= 0 )
    return -1;

  for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
      if ( slot == skill_table[sn].slot )
    return sn;
    }

  if ( fBootDb )
    {
      bug( "Slot_lookup: bad slot %d.", slot );
      abort( );
    }

  return -1;
}



/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
  char buf  [MAX_STRING_LENGTH];
  char buf2 [MAX_STRING_LENGTH];
  CHAR_DATA *rch;
  char *pName;
  int iSyl;
  int length;

  struct syl_type
  {
    char *    old;
    char *    gamenew;
  };

  static const struct syl_type syl_table[] =
  {
    { " ",     " "      },
    { "ar",    "abra"   },
    { "au",    "kada"   },
    { "bless", "fido"   },
    { "blind", "nose"   },
    { "bur",   "mosa"   },
    { "cu",    "judi"   },
    { "de",    "oculo"  },
    { "en",    "unso"   },
    { "light", "dies"   },
    { "lo",    "hi"     },
    { "mor",   "zak"    },
    { "move",  "sido"   },
    { "ness",  "lacri"  },
    { "ning",  "illa"   },
    { "per",   "duda"   },
    { "ra",    "gru"    },
    { "fresh", "ima"    },
    { "re",    "candus" },
    { "son",   "sabru"  },
    { "tect",  "infra"  },
    { "tri",   "cula"   },
    { "ven",   "nofo"   },
    { "a", "o" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
    { "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
    { "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
    { "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
    { "q", "d" }, { "r", "s" }, { "s", "g" }, { "t", "h" },
    { "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
    { "y", "l" }, { "z", "k" },
    { "", "" }
  };
/*
    { "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
    { "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
    { "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
    { "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
    { "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
    { "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
    { "y", "l" }, { "z", "k" },
*/
  buf[0]    = '\0';
  for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
    {
      for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
    {
      if ( !str_prefix( syl_table[iSyl].old, pName ) )
        {
          strcat( buf, syl_table[iSyl].gamenew );
          break;
        }
    }

      if ( length == 0 )
    length = 1;
    }

  mprintf(sizeof(buf2), buf2, "$n utters the words, '%s'.", buf );
  mprintf(sizeof(buf), buf,  "$n utters the words, '%s'.", skill_table[sn].name );

  for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
      if ( rch != ch )
    act((!IS_NPC(rch) && ch->gameclass==rch->gameclass) ? buf : buf2,
        ch, NULL, rch, TO_VICT );
    }

  return;
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA *victim, int dam_type )
{
  int save;

  save = 50 + ( victim->level - level) * 5 - victim->saving_throw * 2;

  if (IS_AFFECTED(victim,AFF_BERSERK))
    save += victim->level/2;

  switch(check_immune(victim,dam_type))
  {
    case IS_IMMUNE:         return TRUE;
    case IS_RESISTANT:      save += 2;    break;
    case IS_VULNERABLE:     save -= 2;    break;
  }

  if (!IS_NPC(victim) && class_table[victim->gameclass].fMana)
    save = 9 * save / 10;
  save = URANGE( 5, save, 95 );
  return (number_percent( ) < save);
}

/* RT save for dispels */
bool saves_dispel( int dis_level, int spell_level, int duration)
{
  int save;
    
  if (duration == -1)
    spell_level += 5;  
  /* very hard to dispel permanent effects */

  save = 50 + (spell_level - dis_level) * 5;
  save = URANGE( 5, save, 95 );
  return number_percent( ) < save;
}

/* co-routine for dispel magic and cancellation */
bool check_dispel( int dis_level, CHAR_DATA *victim, int sn)
{
  AFFECT_DATA *af;

  if (is_affected(victim, sn))
  {
      for ( af = victim->affected; af != NULL; af = af->next )
      {
        if ( af->type == sn )
        {
          if (!saves_dispel(dis_level,af->level,af->duration))
          {
            affect_strip(victim,sn);
            if ( skill_table[sn].msg_off )
            {
              send_to_char( skill_table[sn].msg_off, victim );
              send_to_char( "\n\r", victim );
            }
          return TRUE;
          }
          else
            af->level--;
       }
     }
  }
  return FALSE;
}

/* for finding mana costs -- temporary version */
int mana_cost (CHAR_DATA *ch, int min_mana, int level)
{
  if (ch->level + 2 == level)
    return 1000;
  return UMAX(min_mana,(100/(2 + ch->level - level)));
}



/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_cast( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  void *vo;
  int mana;
  int sn;
  int target;
  int level;

  /*
   * Switched NPC's can cast spells, but others can't.
   */
  if ( IS_NPC(ch) && ch->desc == NULL)
    return;
  
  if ( IS_CLASS_ALCHEMIST(ch)
  &&   !IS_IMMORTAL(ch) )
  {
    send_to_char("You cannot cast spells.\n\r",ch);
    return;
  }

  target_name = one_argument( argument, arg1 );
  strip_string(target_name);
  one_argument( target_name, arg2 );

  if ( arg1[0] == '\0' )
  {
    send_to_char( "Cast which what where?\n\r", ch );
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("You cannot cast spells...you are a ghost.\n\r",ch);
    return;
  }
    
  if (((sn = find_spell(ch,arg1)) < 1)
      ||  (skill_table[sn].spell_fun == spell_null)
      || (!IS_NPC(ch) 
      && (ch->level < skill_table[sn].skill_level[ch->gameclass]
          || ch->pcdata->learned[sn] == 0)))
    {
      if (!is_racial_skill(ch,sn) || skill_table[sn].spell_fun == spell_null){
    send_to_char( "You don't know any spells of that name.\n\r", ch );
    return;
      }
    }

  if ( ch->position < skill_table[sn].minimum_position )
  {
      send_to_char( "You can't concentrate enough.\n\r", ch );
      return;
  }

  if ( is_affected( ch, skill_lookup( "nerve pinch" ) ) )
  {
    send_to_char( "It's hard enough catching your breath.\n\r", ch );
    return;
  }


  if ( IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC) )
    {
      send_to_char( "Your magic doesn't seem to work.\n\r", ch);
      return;
    }
/*
  racial spells are using min_mana instead of cost because the level is so high


*/
  if (is_racial_skill( ch,sn ) )
    mana = UMAX( skill_table[sn].min_mana, 100 / ( 2 + ch->level ) ); 
  else
  {
    if ( ( skill_table[sn].skill_level[ch->gameclass] == (ch->level + 2) )
    &&   ( skill_table[sn].min_mana < 50 ) )
      mana = 50;
    else
      mana = UMAX(skill_table[sn].min_mana,
          100 / ( 2 + ch->level - skill_table[sn].skill_level[ch->gameclass] ) );
  } // !racial

  if (ch->gameclass == cDruid)
  {
    if (ch->in_room->sector_type == SECT_FOREST)
      mana /= 2;
    else if (IS_OUTSIDE(ch))
      mana /= 1.5;
  }

/* if class is mastered, reduce casting cost?
  bool ismaster = TRUE;
  int tsn;
  for ( tsn = 0; tsn < MAX_SKILL; tsn++ )
  {
    if ( skill_table[tsn].name == NULL )
      continue;
    if ( ( skill_table[tsn].skill_level[ch->gameclass] < 0 )
    &&   !is_racial_skill( ch,tsn ) )
      continue;
    if (ch->pcdata->learned[tsn] != 100) 
      ismaster = FALSE;
  }
  if (ismaster)
    mana /= 2;
*/

/* Vampires suck...
  if (ch->race == race_lookup("vampire") && (ch->clan == 7))
  if (IS_VAMPIRE(ch) && IS_KINDRED(ch))
    mana = mana - (mana * .1);
*/

  /*
   * Locate targets.
   */
  victim    = NULL;
  obj       = NULL;
  vo        = NULL;
  target    = TARGET_NONE;
      
  switch ( skill_table[sn].target )
    {
    default:
      bug( "Do_cast: bad target for sn %d.", sn );
      return;

    case TAR_IGNORE:
      break;

    case TAR_CHAR_OFFENSIVE:
      if ( arg2[0] == '\0' )
      {
        if ( ( victim = ch->fighting ) == NULL )
        {
          send_to_char( "Cast the spell on whom?\n\r", ch );
          return;
        }
      }
      else
      {
        if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
        {
          send_to_char( "They aren't here.\n\r", ch );
          return;
        }
      }


      if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
      {
        send_to_char( "You can't do that to your own follower.\n\r", ch );
        return;
      }

      if (victim == ch)
      {
        if (IS_CHARMED(ch))
        {
          send_to_char("Pop: You cannot damage yourself, your master would not like that.\n\r",ch);
          return;
        }
      }

      if ( !is_in_pk_range(ch,victim) && (ch != victim) && (!IS_NPC(victim)) )
      {
        send_to_char( "You cannot attack them.\n\r", ch );
        return;
      }

      if ( !IS_NPC(ch) )
      {
        if (is_safe(ch,victim, TRUE) && victim != ch)
        {
          send_to_char("Not on that target.\n\r",ch);
          return; 
        }

        check_killer(ch,victim);
      }

      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_CHAR_DEFENSIVE:
    if ( arg2[0] == '\0' )
    {
      victim = ch;
    }
    else
    {
        if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
        {
          send_to_char( "They aren't here.\n\r", ch );
          return;
        }
        
        if ( ( ch->level <= IMMORTAL && IS_IMMORTAL( ch ) )
        && !IS_IMMORTAL( victim ) )
        {
            send_to_char( "You are not high enough level to cast spells on mortals.\n\r", ch );
            return;
        }
    }

      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_CHAR_SELF:
      if ( arg2[0] != '\0' && !is_name( target_name, ch->name ) )
    {
      send_to_char( "You cannot cast this spell on another.\n\r", ch );
      return;
    }

      vo = (void *) ch;
      target = TARGET_CHAR;
      break;

    case TAR_OBJ_INV:
      if ( arg2[0] == '\0' )
    {
      send_to_char( "What should the spell be cast upon?\n\r", ch );
      return;
    }

      if ( ( obj = get_obj_carry( ch, target_name, ch ) ) == NULL )
    {
      send_to_char( "You are not carrying that.\n\r", ch );
      return;
    }

      vo = (void *) obj;
      target = TARGET_OBJ;
      break;

    case TAR_OBJ_CHAR_OFF:
    if (arg2[0] == '\0')
    {
      if ((victim = ch->fighting) == NULL)
        {
          send_to_char("Cast the spell on whom or what?\n\r",ch);
          return;
        }
    
      target = TARGET_CHAR;
    }
    else if ((victim = get_char_room(ch,target_name)) != NULL)
    {
      target = TARGET_CHAR;
    }

    if (target == TARGET_CHAR) /* check the sanity of the attack */
    {
      if(is_safe_spell(ch,victim,FALSE, TRUE) && victim != ch)
      {
        send_to_char("Not on that target.\n\r",ch);
        return;
      }

      if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
      {
        send_to_char( "You can't do that to your own follower.\n\r", ch );
        return;
      }

      if (!IS_NPC(ch))
        check_killer(ch,victim);

      vo = (void *) victim;
    }
    else if ((obj = get_obj_here(ch,target_name)) != NULL)
    {
      vo = (void *) obj;
      target = TARGET_OBJ;
    }
    else
    {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
  break; 

    case TAR_OBJ_CHAR_DEF:
      if (arg2[0] == '\0')
      {
        vo = (void *) ch;
        target = TARGET_CHAR;                                                 
      }
      else if ((victim = get_char_room(ch,target_name)) != NULL)
      {
        vo = (void *) victim;
        target = TARGET_CHAR;
      }
      else if ((obj = get_obj_carry(ch,target_name,ch)) != NULL)
      {
        vo = (void *) obj;
        target = TARGET_OBJ;
      }
      else
      {
        send_to_char("You don't see that here.\n\r",ch);
        return;
      }
    break;
  }
        
  if ( !IS_NPC(ch) && ch->mana < mana )
  {
      send_to_char( "You don't have enough mana.\n\r", ch );
      return;
  }
      
  if ( str_cmp( skill_table[sn].name, "ventriloquate" ) )
    say_spell( ch, sn );
      
  WAIT_STATE( ch, ( skill_table[sn].beats /
    ( is_affected( ch, skill_lookup( "clear head" ) ) ? 1.5 : 1 ) ) );
      
  if ( number_percent( ) > get_skill(ch,sn) && !IS_IMMORTAL(ch))
  {
    send_to_char( "You lost your concentration.\n\r", ch );
    check_improve(ch,sn,FALSE,1);
    ch->mana -= mana / 2;
  }
  else
  {
    ch->mana -= mana;

   /* check for group proficiency bonuses */
    level = check_proficiency_level(ch,sn);

    cast_a_spell(sn,level,ch,vo,target, FALSE);
    check_improve(ch,sn,TRUE,1);
  }

  if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
  ||  (skill_table[sn].target == TAR_OBJ_CHAR_OFF
  && target == TARGET_CHAR))
  &&   victim != ch
  &&   victim->master != ch)
  {
    if (victim)
      if (!victim->fighting && (ch->in_room == victim->in_room))
      {
        check_killer(victim, ch);
        multi_hit( victim, ch, TYPE_UNDEFINED );
      }
    }

  return;
}

/*
 * returns the char's level, modified by the spell's group's class' proficiency
 */
int check_proficiency_level( CHAR_DATA *ch, int gsn )
{
  int level, gn, sn, fn;

  if (ch == NULL) // should never happen, but being careful
    return 100;

  if ( IS_NPC(ch) )
    return ch->level;

  level = ch->level;

  for (gn = 0; gn < MAX_GROUP; gn++)
  {
    if (group_table[gn].name == NULL)
      break; // end of list

    if (group_table[gn].rating[ch->gameclass] < 0)
      continue; // player doesn't have this class, keep looking
    
    for (sn = 0; sn < MAX_IN_GROUP; sn++)
    {
      if ( group_table[gn].spells[sn] == NULL )
        break; // end of list

      fn = skill_lookup_exact( group_table[gn].spells[sn] );

      if (skill_table[fn].skill_level[ch->gameclass] <= 0)
        continue; // player doesn't have this spell in this class

      if (gsn == fn )
      { // we found our spell
        level = ( (level * group_table[gn].proficiency[ch->gameclass]) / 100 );
        break;
      }

    } // loop spells in group
  } // loop groups

  return level;
}

/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA
             *victim, OBJ_DATA *obj , int bypass)
{
  void *vo;
  int target = TARGET_NONE;

  if ( sn <= 0 ) {
    return;
  }

  if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
      bug( "Obj_cast_spell: bad sn %d.", sn );
      return;
    }

  if ( IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC) )
    {
      send_to_char( "Your magic doesn't seem to work.\n\r", ch);
      return;
    }

  switch ( skill_table[sn].target )
    {
    default:
      bug( "Obj_cast_spell: bad target for sn %d.", sn );
      return;

    case TAR_IGNORE:
      vo = NULL;
      break;

    case TAR_CHAR_OFFENSIVE:
      if ( victim == NULL )
        victim = ch->fighting;

      if ( victim == NULL )
      {
        send_to_char( "You can't do that.\n\r", ch );
        return;
      }

      if (is_safe(ch,victim, TRUE) && ch != victim)
      {
        send_to_char("Something isn't right...\n\r",ch);
        return;
      }

      if (!IS_NPC(ch))
        check_killer(ch,victim);

      vo = (void *) victim;
      target = TARGET_CHAR;
      break; // TAR_CHAR_OFFENSIVE

    case TAR_CHAR_DEFENSIVE:
      if ( victim == NULL )
        victim = ch;
      vo = (void *) victim;
      target = TARGET_CHAR;
      break;
      
    case TAR_CHAR_SELF: // deter...
      if ( victim == NULL )
        victim = ch;

      if (victim != ch)
      {
        send_to_char( "You can't cast this on others.\n\r", ch );
        return;
      }

      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_OBJ_INV:
      if ( obj == NULL )
      {
        send_to_char( "You can't do that.\n\r", ch );
        return;
      }
      vo = (void *) obj;
      target = TARGET_OBJ;
      break;

    case TAR_OBJ_CHAR_OFF:
      if ( victim == NULL && obj == NULL)
      {
        if (ch->fighting != NULL)
          victim = ch->fighting;
        else
        {
          send_to_char("You can't do that.\n\r",ch);
          return;
        }
      }
      if (victim != NULL)
      {
        if (is_safe_spell(ch,victim,FALSE, TRUE) && ch != victim)
        {
            send_to_char("Something isn't right...\n\r",ch);
            return;
        }

        vo = (void *) victim;
        target = TARGET_CHAR;
      }
      else
      {
        vo = (void *) obj;
        target = TARGET_OBJ;
      }
      break; // TAR_OBJ_CHAR_OFF


    case TAR_OBJ_CHAR_DEF:
      if (victim == NULL && obj == NULL)
      {
        vo = (void *) ch;
        target = TARGET_CHAR;
      }
      else if (victim != NULL)
      {
        vo = (void *) victim;
        target = TARGET_CHAR;
      }
      else
      {
        vo = (void *) obj;
        target = TARGET_OBJ;
      }
      break;
    } // TAR_OBJ_CHAR_DEF

  if (!bypass)
    target_name = "";
  cast_a_spell( sn, level, ch, vo,target, TRUE);

  if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
       &&   victim != ch
       &&   victim->master != ch )
    {
      if (victim)
    if (!victim->fighting && (ch->in_room == victim->in_room))
      {
        check_killer(victim, ch);
        multi_hit( victim, ch, TYPE_UNDEFINED );
      }
    }
  return;
}



/*
 * Spell functions.
 */
void spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = magic_dam(sn,level);
  if ( saves_spell( level, victim, DAM_ACID ) )
    dam /= 2;
  damage( ch, victim, dam, sn,DAM_ACID,TRUE, TRUE);
  return;
}



void spell_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
    {
      if (victim == ch)
    send_to_char("You are already armored.\n\r",ch);
      else
    act("$N is already armored.",ch,NULL,victim,TO_CHAR);
      return;
    }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 24;
  af.modifier  = -20;
  af.location  = APPLY_AC;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  act( "You feel $G protecting you.", victim, NULL, ch, TO_CHAR );
  if ( ch != victim )
    act("$N is protected by your magic.",ch,NULL,victim,TO_CHAR);
  return;
}



void spell_bless( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;

  /* dea with the object case first */
  if (target == TARGET_OBJ)
  {
    obj = (OBJ_DATA *) vo;
    if (IS_OBJ_STAT(obj,ITEM_BLESS))
    {
      act("$p is already blessed.",ch,obj,NULL,TO_CHAR);
      return;
    }

    if (IS_OBJ_STAT(obj,ITEM_EVIL))
    {
      AFFECT_DATA *paf;

      paf = affect_find(obj->affected,gsn_curse);
      if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
      {
        if (paf != NULL)
          affect_remove_obj(obj,paf);
        act("$p glows a pale {cblue{x.",ch,obj,NULL,TO_ALL);
        REMOVE_BIT(obj->extra_flags,ITEM_EVIL);
        return;
      }
      else
      {
        act("The evil of $p is too powerful for you to overcome.", ch,obj,NULL,TO_CHAR);
        return;
      }
    }
    
    af.where    = TO_OBJECT;
    af.type        = sn;
    af.level    = level;
    af.duration    = 6 + level;
    af.location    = APPLY_SAVES;
    af.modifier    = -1;
    af.bitvector    = ITEM_BLESS;
    affect_to_obj(obj,&af);

    act("$p glows with a {yh{Wo{yly{x aura.",ch,obj,NULL,TO_ALL);

    if (obj->wear_loc != WEAR_NONE)
      ch->saving_throw -= 1;
    return;
  }

  /* character target */
  victim = (CHAR_DATA *) vo;

  if ( victim->position == POS_FIGHTING
  &&   !IS_IMMORTAL( ch ) ) //Imms can bless fighting fools.
  {
    if (victim == ch)
      send_to_char("You cannot be blessed while fighting.\n\r",ch);
    else
      act("$N cannot be blessed while they are doing such violence.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if ( is_affected( victim, sn )
  || is_affected(victim, skill_lookup("mass bless")))
  {
    if (victim == ch)
      send_to_char("You are already blessed.\n\r",ch);
    else
      act("$N already has divine favor.",ch,NULL,victim,TO_CHAR);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 6+level;
  af.location  = APPLY_HITROLL;
  af.modifier  = level / 8;
  af.bitvector = 0;
  affect_to_char( victim, &af );

  af.location  = APPLY_SAVING_SPELL;
  af.modifier  = 0 - level / 8;
  affect_to_char( victim, &af );

  send_to_char( "You feel righteous.\n\r", victim );

  if ( ch->clan )
  {
    if ( victim != ch )
    {
      if ( IS_IMMORTAL( ch ) )
      {
        act( "You grant your blessing to $N.", ch, NULL, victim, TO_CHAR );
        act( "$n grants $N his blessing.", ch, NULL, victim, TO_NOTVICT );
      }
      else
      {
        act( "You grant $N the blessing of $g.", ch, NULL, victim, TO_CHAR );
        act( "$n grants $N the blessing of $g.", ch, NULL, victim, TO_NOTVICT );
      }
    }
    else
      act( "$n grants $mself the blessing of $g.", ch, NULL, victim, TO_NOTVICT );
  }
  else
  {
    if ( victim != ch )
    {
      if ( IS_IMMORTAL( ch ) )
      {
        act( "You grant your blessing to $N.", ch, NULL, victim, TO_CHAR );
        act( "$n grants $N his blessing.", ch, NULL, victim, TO_NOTVICT );
      }
      else
      {
        act( "You grant $N the blessing of {CM{cir{Ml{mya{x.", ch, NULL, victim, TO_CHAR );
        act( "$n grants $N the blessing of {CM{cir{Ml{mya{x.", ch, NULL, victim, TO_NOTVICT );
      }
    }
    else
      act( "$n grants $mself the blessing of {CM{cir{Ml{mya{x.", ch, NULL, victim, TO_NOTVICT );
  }

  return;
}



void spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;


  if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
  {
      send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
      return;
  }

  if ( IS_AFFECTED( victim, AFF_BLIND ) )
  {
    send_to_char( "They are already blind.\n\r", ch );
    return;
  }

  if ( saves_spell( level, victim, DAM_OTHER ) )
  {
      send_to_char("Blindness failed.\n\r",ch);

      if (!is_affected(victim, skill_lookup("sleep")))
        damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
      return;
  }


  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.location  = APPLY_HITROLL;
  af.modifier  = -4;
  af.duration  = 1+level;
  af.bitvector = AFF_BLIND;
  affect_to_char( victim, &af );
  send_to_char( "You are blinded!\n\r", victim );
  if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
  act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
  return;
}



void spell_burning_hands(int sn,int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  level    = level;
  level    = UMAX(0, level);
  dam   = magic_dam(sn,level);
  if ( saves_spell( level, victim,DAM_FIRE) )
    dam /= 2;
  damage( ch, victim, dam, sn, DAM_FIRE,TRUE, TRUE);
  return;
}



void spell_call_lightning( int sn, int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  char buf[MIL];
  int dam;

  if ( !IS_OUTSIDE(ch) )
    {
      send_to_char( "You must be out of doors.\n\r", ch );
      return;
    }

/*  if ( weather_info.sky < SKY_RAINING )
    {
      send_to_char( "You need bad weather.\n\r", ch );
      return;
    }
*/
  dam = magic_dam(sn,level);
  if ( IS_IN_CLAN( ch ) )
  {
      printf_to_char(ch, "%s's {yl{Yi{yg{Wh{Yt{yn{wi{Wn{yg{x strikes your foes!\n\r",
        ch->clan->clan_immortal );  //clan_table[ch->clan].patron);
      mprintf(sizeof(buf), buf,"$n calls %s's {yl{Yi{yg{Wh{Yt{yn{wi{Wn{yg{x to strike $s foes!",
        ch->clan->clan_immortal );  //clan_table[ch->clan].patron);
      act(buf,ch, NULL, NULL, TO_ROOM );
  }
  else
  {
      send_to_char( "{CM{cir{Ml{mya{x's {yl{Yi{yg{Wh{Yt{yn{wi{Wn{yg{x strikes your foes!\n\r", ch );
      act( "$n calls {CM{cir{Ml{mya{x's {yl{Yi{yg{Wh{Yt{yn{wi{Wn{yg{x to strike $s foes!",
       ch, NULL, NULL, TO_ROOM );
  }

  for ( vch = char_list; vch != NULL; vch = vch_next )
  {
    vch_next    = vch->next;
    if ( vch->in_room == NULL )
      continue;
    if ( vch->in_room == ch->in_room )
    {
      if ( vch != ch
      &&   vch->master != ch
      &&   ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
        damage( ch, vch, saves_spell( level,vch,DAM_LIGHTNING) 
          ? dam / 2 : dam, sn,DAM_LIGHTNING,TRUE, TRUE);
      continue;
    }

    if ( vch->in_room->area == ch->in_room->area
    &&   IS_OUTSIDE(vch)
    &&   IS_AWAKE(vch) )
      send_to_char( "{yL{Yi{yg{Wh{Yt{yn{wi{Wn{yg{x flashes in the sky.\n\r", vch );
  }

  return;
}

/* RT calm spell stops all fighting in the room */

void spell_calm( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *vch;
  int mlevel = 0;
  int count = 0;
  int high_level = 0;    
  int chance;
  AFFECT_DATA af;

  /* get sum of all mobile levels in the room */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (vch->position == POS_FIGHTING)
    {
      count++;
      if (IS_NPC(vch))
        mlevel += vch->level;
      else
        mlevel += vch->level/2;
      high_level = UMAX(high_level,vch->level);
    }
    }

  /* compute chance of stopping combat */
  chance = 4 * level - high_level + 2 * count;

  if (IS_IMMORTAL(ch))    /* always works */
    mlevel = 0;

  if (number_range(0, chance) >= mlevel) /* hard to stop large fights */
  {
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (IS_NPC(vch)
      && (IS_SET(vch->imm_flags,IMM_MAGIC)
        || IS_SET(vch->act,ACT_UNDEAD)))
      {
        send_to_char("You fail.\n\r", ch);
        return;
      }

      if (IS_AFFECTED(vch,AFF_CALM)
      || IS_AFFECTED(vch,AFF_BERSERK)
      ||  is_affected(vch,skill_lookup("frenzy"))
      ||   ( vch->position != POS_FIGHTING && IS_WIZINVIS( vch, ch ) ))
      {
        send_to_char("You fail.\n\r", ch);
        return;
      }
        
      send_to_char("A wave of calm passes over you.\n\r",vch);

      if (vch->fighting || vch->position == POS_FIGHTING)
        stop_fighting(vch,FALSE);

      af.where = TO_AFFECTS;
      af.type = sn;
      af.level = level;
      af.duration = level/4;
      af.location = APPLY_HITROLL;
      if (!IS_NPC(vch))
        af.modifier = -5;
      else
        af.modifier = -2;
      af.bitvector = AFF_CALM;
      affect_to_char(vch,&af);

      af.location = APPLY_DAMROLL;
      affect_to_char(vch,&af);
    }
  }
  else
    send_to_char("You fail.\n\r", ch);
}

void spell_cancellation( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool found = FALSE;
 
  level += 2;

  if ( ch->fighting )
  {
    send_to_char( "You can't do that while fighting!\n\r", ch );
    return;
  }

  if (IS_NPC(ch) && !IS_SET(ch->act, ACT_IS_HEALER))
  {
    magic_dispel(level, victim);
    return;
  }

  if ( ( // +p1
         !IS_NPC(ch)
         && ( IS_NPC(victim) && victim->master != ch )
         && !(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
       ) // -p1
     ||
       ( // +p2
         !IS_NPC(victim)
         && IS_SET(victim->act,PLR_NOCANCEL)
         && ( ch != victim )
       ) // -p2
     || IS_AFFECTED(ch, AFF_CHARM) )
    {
      send_to_char("You failed, try dispel magic.\n\r",ch);
      return;
    }

  /* unlike dispel magic, the victim gets NO save */
 
  /* begin running through the spells */

  found = magic_dispel(level, victim);
 
  if (found)
    send_to_char("Ok.\n\r",ch);
  else
    send_to_char("Spell failed.\n\r",ch);
}

void spell_cause_light( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
    {
      send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
      return;
    }

  damage( ch, (CHAR_DATA *) vo, magic_dam(sn,level), sn,DAM_HARM,TRUE, TRUE);
  return;
}



void spell_cause_critical(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  damage( ch, (CHAR_DATA *) vo, magic_dam(sn,level), sn,DAM_HARM,TRUE, TRUE);
  return;
}



void spell_cause_serious(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  damage( ch, (CHAR_DATA *) vo, magic_dam(sn,level), sn,DAM_HARM,TRUE, TRUE);
  return;
}

void spell_chain_lightning(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *tmp_vict,*last_vict,*next_vict;
  bool found;
  int dam;
#define DEC_LEVEL 8
  /* first strike */

  act("A {yl{Wi{yg{Yh{Wt{yn{wi{Yn{yg {Wbo{yl{wt{x flies from $n's hand and arcs around the room.",
      ch,NULL,victim,TO_ROOM);
  dam = magic_dam(sn,level);
  if (saves_spell(level,victim,DAM_LIGHTNING))
    dam /= 3;
  damage(ch,victim,dam,sn,DAM_LIGHTNING,TRUE, TRUE);
  last_vict = victim;
  level -= DEC_LEVEL;            /* decrement damage */

  /* new targets */
  while (level > 0)
    {
      found = FALSE;
      for (tmp_vict = ch->in_room->people; 
       tmp_vict != NULL; 
       tmp_vict = next_vict) 
    {
      next_vict = tmp_vict->next_in_room;
      if (!is_safe_spell(ch,tmp_vict,FALSE,FALSE) && 
          tmp_vict != last_vict && 
          !IS_WIZINVIS(tmp_vict,ch)
          && (tmp_vict->position != POS_DEAD))
        {
          found = TRUE;
          last_vict = tmp_vict;
          dam = magic_dam(sn,level);
          if (saves_spell(level,tmp_vict,DAM_LIGHTNING))
        dam /= 3;
          damage(ch,tmp_vict,dam,sn,DAM_LIGHTNING,TRUE, TRUE);
          level -= DEC_LEVEL;    /* decrement damage */
        }
    } /* end target searching loop */
    
      if (!found)        /* no target found, hit the caster */
    {
      if (ch == NULL)
        return;

      if (last_vict == ch) /* no double hits */
        {
          act("The {yl{Wi{yg{Yh{Wt{yn{wi{Yn{yg {Wbo{yl{wt{x seems to have fizzled out.",ch,NULL,NULL,TO_ROOM);
          act("The {yl{Wi{yg{Yh{Wt{yn{wi{Yn{yg {Wbo{yl{wt{x grounds out through your body.",
          ch,NULL,NULL,TO_CHAR);
          return;
        }
    
      last_vict = ch;
      if (last_vict->position != POS_DEAD) {
        act("The {yl{Wi{yg{Yh{Wt{yn{wi{Yn{yg {Wbo{yl{wt{x arcs to $n...whoops!",ch,NULL,NULL,TO_ROOM);
        send_to_char("You are struck by your own {yl{Wi{yg{Yh{Wt{yn{wi{Yn{yg {Wbo{yl{wt{x!\n\r",ch);
        dam = magic_dam(sn,level);
        if (saves_spell(level,ch,DAM_LIGHTNING))
          dam /= 3;
        damage(ch,ch,dam,sn,DAM_LIGHTNING,TRUE, TRUE);
        level -= DEC_LEVEL;        /* decrement damage */
      }
    }
      /* now go back and find more targets */
    }
}
      

void spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ))
    {
      if (victim == ch)
    send_to_char("You've already been changed.\n\r",ch);
      else
    act("$N has already had $s(?) sex changed.",ch,NULL,victim,TO_CHAR);
      return;
    }
  if (saves_spell(level , victim,DAM_OTHER)) {
    send_to_char(
      "Your spell failed to penetrate your victim's defensive aura.\n\r",ch);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 2 * level;
  af.location  = APPLY_SEX;
  do
    {
      af.modifier  = number_range( 0, 2 ) - victim->sex;
    }
  while ( af.modifier == 0 );
  af.bitvector = 0;
  send_to_char( "You feel different.\n\r", victim );
  act("$n doesn't look like $mself anymore...",victim,NULL,NULL,TO_ROOM);
  affect_to_char( victim, &af );
  copy_roster_clannie(victim);
  return;
}



void spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_safe_spell(ch,victim, FALSE, FALSE)) return;

  if (IS_SET(victim->in_room->room_flags,ROOM_ARENA)) {
    send_to_char("Charm failed, Immortals prevent this here.\n\r",ch);
    return;
  }

  if ( victim->position == POS_SLEEPING )
  {
      send_to_char( "You can not get your victim's attention.\n\r", ch );
      send_to_char( "Your slumbers are briefly troubled.\n\r", victim );
      return;
  }

  if ( victim->position == POS_FIGHTING )
  {
      send_to_char( "You can not get your victim's attention.\n\r", ch );
      send_to_char( "You hesitate a bit but continue on.\n\r", victim );
      return;
  }

  if ( victim == ch )
  {
      send_to_char( "You like yourself even better!\n\r", ch );
      return;
  }

  if ( IS_AFFECTED(victim, AFF_CHARM)
  ||   IS_AFFECTED(ch, AFF_CHARM)
  ||   (IS_SET(victim->act, ACT_AGGRESSIVE) && IS_NPC(victim))
  ||   saves_spell( level, victim,DAM_CHARM)
  ||   IS_SET(victim->imm_flags,IMM_CHARM)
  ||   level < victim->level)
  {
      send_to_char("You failed.\n\r",ch);
      return;
    }

  if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
    {
      send_to_char(
           "The Gods do not allow charming in the city limits.\n\r",ch);
      return;
    }
  
  if ( victim->master )
    stop_follower( victim );
  add_follower( victim, ch );

  if (ch->leader)
    victim->leader = ch->leader;
  else
    victim->leader = ch;

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = number_fuzzy( (level /2) + (level /4) );
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char( victim, &af );
  act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
  if ( ch != victim )
    act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
  return;
}



void spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int dam;

  level    = UMAX(0, level);
  dam        = magic_dam(sn,level);
  if ( !saves_spell( level, victim,DAM_COLD ) )
    {
      act("$n turns {Bbl{cue{x and {csh{Bi{cve{Br{cs{x.",victim,NULL,NULL,TO_ROOM);
      af.where     = TO_AFFECTS;
      af.type      = sn;
      af.level     = level;
      af.duration  = 6;
      af.location  = APPLY_STR;
      af.modifier  = -1;
      af.bitvector = 0;
      affect_join( victim, &af );
    }
  else
    {
      dam /= 2;
    }

  damage( ch, victim, dam, sn, DAM_COLD,TRUE, TRUE );
  return;
}



void spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  level    = UMAX(0, level);
  dam    = magic_dam(sn,level);   ;

  if ( saves_spell( level, victim,DAM_LIGHT) )
    dam /= 2;
  else 
    spell_blindness(skill_lookup("blindness"),
            level/2,ch,(void *) victim,TARGET_CHAR);

  damage( ch, victim, dam, sn, DAM_LIGHT,TRUE, TRUE );
  return;
}



void spell_continual_light(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  OBJ_DATA *light;
  char buf[MAX_STRING_LENGTH];

  if (target_name[0] != '\0')    /* do a glow on some object */
    {
      light = get_obj_carry(ch,target_name,ch);
    
    if (light == NULL)
    {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }

    if (IS_OBJ_STAT(light,ITEM_GLOW))
    {
      act("$p is already glowing.",ch,light,NULL,TO_CHAR);
      return;
    }

    SET_BIT(light->extra_flags,ITEM_GLOW);

    mprintf(sizeof(buf), buf, "%s glows with a bright {cl{Ci{cght{x.", capitalize(light->short_descr) );
    act(buf,ch,light,NULL,TO_ALL);
    return;
  }

  light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
  obj_to_room( light, ch->in_room );
  act( "$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM );
  act( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
  return;
}



void spell_control_weather(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{

  //Need to be outdoors to change the weather.
  if ( !IS_OUTSIDE(ch) )
  {
      send_to_char( "You must be out of doors.\n\r", ch );
      return;
  }

  if ( !str_cmp( target_name, "better" ) ) 
  {
    weather_info.change += dice( level / 3, 4 );
    printf_to_char(ch,"You have improved the weather.\n\r");
  }
  else if ( !str_cmp( target_name, "worse" ) )
  {
    weather_info.change -= dice( level / 3, 4 );
    printf_to_char(ch,"You have worsened the weather.\n\r");
  }
  else
    send_to_char ("Do you want it to get better or worse?\n\r", ch );

  return;
}



void spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  OBJ_DATA *mushroom;

  mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
  mushroom->value[1] = interpolateNew( level, 1, MAX_LEVEL, 4, 16 ); /* Full */
  mushroom->value[0] = interpolateNew( level, 1, MAX_LEVEL, 8, 32 ); /* Food */

  if ( !IS_NULLSTR(target_name) )
  {
    if ( !str_prefix( target_name, "lichen" ) )
    {
      free_string(mushroom->name);
      mushroom->name = str_dup("lichen", mushroom->name);
      free_string(mushroom->short_descr);
      mushroom->short_descr = str_dup( "a delicious lichen", mushroom->short_descr);
      free_string(mushroom->description);
      mushroom->description = str_dup( "A magically crafted brown lichen.", mushroom->description);
    }

    if ( !str_prefix( target_name, "apple" ) )
    {
      free_string(mushroom->name);
      mushroom->name = str_dup("apple", mushroom->name);
      free_string(mushroom->short_descr);
      mushroom->short_descr = str_dup( "a juicy apple", mushroom->short_descr);
      free_string(mushroom->description);
      mushroom->description = str_dup( "A magically crafted red apple.", mushroom->description);
    }

    if ( !str_prefix( target_name, "carrot" ) )
    {
      free_string(mushroom->name);
      mushroom->name = str_dup("carrot", mushroom->name);
      free_string(mushroom->short_descr);
      mushroom->short_descr = str_dup( "a crisp carrot", mushroom->short_descr);
      free_string(mushroom->description);
      mushroom->description = str_dup( "A magically crafted orange carrot.", mushroom->description);
    }

    if ( !str_prefix( target_name, "venison" ) )
    {
      free_string(mushroom->name);
      mushroom->name = str_dup("cut venison", mushroom->name);
      free_string(mushroom->short_descr);
      mushroom->short_descr = str_dup( "a succulent cut of venison", mushroom->short_descr);
      free_string(mushroom->description);
      mushroom->description = str_dup( "A magically crafted cut of venison.", mushroom->description);
    }

    if ( !str_prefix( target_name, "pecans" ) )
    {
      free_string(mushroom->name);
      mushroom->name = str_dup("handful pecans", mushroom->name);
      free_string(mushroom->short_descr);
      mushroom->short_descr = str_dup( "a handful of pecans", mushroom->short_descr);
      free_string(mushroom->description);
      mushroom->description = str_dup( "A magically crafted handful of brown pecans.", mushroom->description);
    }

    if ( !str_prefix( target_name, "meat" ) )
    {
      free_string(mushroom->name);
      mushroom->name = str_dup("dried meat", mushroom->name);
      free_string(mushroom->short_descr);
      mushroom->short_descr = str_dup( "some dried meat", mushroom->short_descr);
      free_string(mushroom->description);
      mushroom->description = str_dup( "Magically crafted dried meat.", mushroom->description);
    }

  }

  obj_to_room( mushroom, ch->in_room );
  act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
  act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
  return;
}
/* Thanks to AR :) */
void spell_create_rose( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  OBJ_DATA *rose;
  EXTRA_DESCR_DATA *ed;
  char     buf[MAX_STRING_LENGTH];
  char     rose_color[MAX_INPUT_LENGTH];

  if ( target_name[0] == '\0' )
    strcpy( rose_color, "{rr{Re{rd{x" );
  else
    strcpy( rose_color, target_name );
  strcat(rose_color,"{x");
  /*    smash_tilde( rose_color );*/

  /*    strcat( rose_color, tag_norm_color( rose_color ) );*/
  if (ch->carry_number +1 > can_carry_n(ch))
  {
      send_to_char("You can't carry any more items.\n\r",ch);
      return;
  }
  rose = create_object( get_obj_index( OBJ_VNUM_ROSE ), 0);
  mprintf(sizeof(buf), buf, "$n has created the beautiful %s rose.", rose_color );
  act_new( buf, ch, rose, NULL, TO_ROOM, POS_RESTING );
  printf_to_char(ch, "You create the beautiful %s rose.\n\r", rose_color );

  /*    mprintf(sizeof(buf), buf, rose->short_descr, rose_color );
    free_string( rose->short_descr );
    rose->short_descr   = str_dup( buf );
      
    mprintf(sizeof(buf), buf, rose->description, rose_color );
    free_string( rose->description );
    rose->description   = str_dup( buf );
  */
  mprintf(sizeof(buf), buf,"%s rose",rose_color);
  free_string(rose->short_descr);
  rose->short_descr = str_dup( buf, rose->short_descr);
  free_string(rose->description);
  rose->description = str_dup( buf, rose->description);
  ed = new_extra_descr();
  ed->keyword         = str_dup( "rose" , ed->keyword);

  mprintf(sizeof(buf), buf,
      "The magic of %s has created the %s rose.\n\rIt is beautiful, bearing a soft and romantic fragrance.\n\r",
      IS_NPC( ch ) ? ch->short_descr: ch->name, rose_color );

  ed->description     = str_dup( buf , ed->description);
  ed->next            = rose->extra_descr;
  rose->extra_descr   = ed;

  obj_to_char( rose, ch );
}

void spell_create_spring(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  OBJ_DATA *spring;

  spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
  spring->timer = level;
  obj_to_room( spring, ch->in_room );
  act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
  act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );
  return;
}

void spell_create_fountain(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  OBJ_DATA *spring;
  int liquid=0;
  char buf[MSL];

  if ( ( liquid = liq_lookup(target_name) )  < 0 )
    {
      send_to_char("What type of liquid is THAT?\n\r",ch);
      return;
    }
  spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
  spring->timer = level;
  /*    spring->type = ITEM_FOUNTAIN;*/
  mprintf(sizeof(buf), buf,"magical fountain %s",liq_table[liquid].liq_name);
  free_string(spring->name);
  spring->name = str_dup(buf, spring->name);
  mprintf(sizeof(buf), buf,"a magical fountain of %s",liq_table[liquid].liq_name);
  free_string(spring->short_descr);
  spring->short_descr = str_dup(buf, spring->short_descr);
  mprintf(sizeof(buf), buf,"A magical fountain of %s sits here.",liq_table[liquid].liq_name);
  free_string(spring->description);
  spring->description = str_dup(buf, spring->description);
  spring->value[2] = liquid;
  obj_to_room( spring, ch->in_room );
  act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
  act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );
  return;
}



void spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int water;

  if ( obj->item_type != ITEM_DRINK_CON )
    {
      send_to_char( "It is unable to hold water.\n\r", ch );
      return;
    }

  if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
      send_to_char( "It contains some other liquid.\n\r", ch );
      return;
    }

  water = UMIN(
           level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
           obj->value[0] - obj->value[1]
           );
  
  if ( water > 0 )
    {
      obj->value[2] = LIQ_WATER;
      obj->value[1] += water;
      if ( !is_name( "water", obj->name ) )
    {
      char buf[MAX_STRING_LENGTH];

      mprintf(sizeof(buf), buf, "%s water", obj->name );
      free_string( obj->name );
      obj->name = str_dup( buf , obj->name);
    }

    act( "You add water to $p.", ch, obj, NULL, TO_CHAR );
    }
   else
    act( "You can't add any more water to $p.", ch, obj, NULL, TO_CHAR );

  return;
}



void spell_cure_blindness(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool found = FALSE;
  int fb=skill_lookup("fire breath");
  
  if (!is_affected(victim, gsn_blindness )  &&
      !is_affected(victim, fb)              &&
      !is_affected(victim, gsn_dirt))
    {
      if (victim == ch)
    send_to_char("You aren't blind.\n\r",ch);
      else
    act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
      return;
    }

#ifdef INQUISITOR
  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
    {
      send_to_char("You are not righteous enough to call on your holy powers.\n\r", ch);
      return;
    }


  if (is_affected(victim, gsn_blindness)) {
    if (check_dispel(level,victim,gsn_blindness))
      {
    send_to_char( "Your vision returns!\n\r", victim );
    act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
      
      }
  }
#endif

  if (is_affected(victim, gsn_dirt)) {
    if (check_dispel(level, victim, gsn_dirt))
      {
    send_to_char( "Your vision returns!\n\r", victim );
    act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
      }
  }
  if (is_affected(victim, fb)) {
    if (check_dispel(level,victim,fb))
      {
    send_to_char( "The {Ds{wm{Doke{x clears and your vision returns!\n\r", victim );
    act("$n is no longer fire blinded.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
      }
  }
  if (!found)
    send_to_char("Spell failed.\n\r",ch);
}



void spell_cure_critical( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal;

  heal = dice(3, 8) + level - 6;
  victim->hit = UMIN( victim->hit + heal, GET_HP(victim) );
  update_pos( victim );
  send_to_char( "You feel better!\n\r", victim );
  if (victim->hit >= GET_HP(victim))
    send_to_char("Fully healed.\n\r",ch);
  else {
    if ( ch != victim )
      send_to_char( "You have healed wounds.\n\r", ch );
  }
  return;
}

/* RT added to cure plague */
void spell_cure_disease( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( !is_affected( victim, gsn_plague ) && (!is_affected( victim, gsn_yawn)))
    {
      if (victim == ch)
    send_to_char("You aren't ill.\n\r",ch);
      else
    act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
      return;
    }

  if (is_affected(victim, gsn_plague)) {
    if (check_dispel(level,victim,gsn_plague))
      {
    act("$n looks relieved as $s {rs{Do{gres{x vanish.",victim,NULL,NULL,TO_ROOM);
      }
    else
      send_to_char("Spell failed.\n\r",ch);
  }  else {
    if (check_dispel(level,victim,gsn_yawn))
      {
    act("$n looks relieved as $e starts to feel more awake.",victim,NULL,NULL,TO_ROOM);
      }
    else
      send_to_char("Spell failed.\n\r",ch);
  }
}



void spell_cure_light( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal;

  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
    {
      send_to_char("You are not righteous enough to be granted your holy powers.\n\r", ch);
      return;
    }

  heal = dice(1, 8) + level / 3;
  victim->hit = UMIN( victim->hit + heal, GET_HP(victim) );
  update_pos( victim );
  send_to_char( "You feel better!\n\r", victim );
  if (victim->hit >= GET_HP(victim))
    send_to_char("Fully healed.\n\r",ch);
  else {
    if ( ch != victim )
      send_to_char( "You have healed wounds.\n\r", ch );
  }
  return;
}



void spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
 
  if ( !is_affected( victim, gsn_poison ) )
    {
      if (victim == ch)
    send_to_char("You aren't poisoned.\n\r",ch);
      else
    act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
      return;
    }
 
  if (check_dispel(level,victim,gsn_poison))
    {
      send_to_char("A warm feeling runs through your body.\n\r",victim);
      act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
    }
  else
    send_to_char("Spell failed.\n\r",ch);
}

void spell_cure_serious( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal;

  heal = dice(2, 8) + level /2 ;
  victim->hit = UMIN( victim->hit + heal, GET_HP(victim) );
  update_pos( victim );
  send_to_char( "You feel better!\n\r", victim );
  if (victim->hit >= GET_HP(victim))
    send_to_char("Fully healed.\n\r",ch);
  else {
    if ( ch != victim )
      send_to_char( "You have healed wounds.\n\r", ch );
  }
  return;
}



void spell_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;

  /* deal with the object case first */
  if (target == TARGET_OBJ)
    {
      obj = (OBJ_DATA *) vo;
      if (IS_OBJ_STAT(obj,ITEM_EVIL))
        {
      act("$p is already filled with evil.",ch,obj,NULL,TO_CHAR);
      return;
        }

      if (IS_OBJ_STAT(obj,ITEM_BLESS))
        {
      AFFECT_DATA *paf;

      paf = affect_find(obj->affected,skill_lookup("bless"));
      if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
            {
          if (paf != NULL)
        affect_remove_obj(obj,paf);
          act("$p glows with a {rvi{Dl{re{x aura.",ch,obj,NULL,TO_ALL);
          REMOVE_BIT(obj->extra_flags,ITEM_BLESS);
          return;
            }
      else
            {
          act("The holy aura of $p is too powerful for you to overcome.",
          ch,obj,NULL,TO_CHAR);
          return;
            }
        }

      af.where        = TO_OBJECT;
      af.type         = sn;
      af.level        = level;
      af.duration     = 2 * level;
      af.location     = APPLY_SAVES;
      af.modifier     = +1;
      af.bitvector    = ITEM_EVIL;
      affect_to_obj(obj,&af);

      act("$p glows with a {rvi{Dl{re{x aura.",ch,obj,NULL,TO_ALL);

      if (obj->wear_loc != WEAR_NONE)
    ch->saving_throw += 1;
      return;
    }

  /* character curses */
  victim = (CHAR_DATA *) vo;

  if ( IS_AFFECTED(victim,AFF_CURSE) )
  {
    send_to_char( "They are already cursed.\n\r", ch );
    return;
  }

  if ( saves_spell(level,victim,DAM_NEGATIVE))
  {
    send_to_char("Curse failed.\n\r",ch);

    if (!is_affected(victim, skill_lookup("sleep")))
      damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);

    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 2*level;
  af.location  = APPLY_HITROLL;
  af.modifier  = -1 * (level / 8);
  af.bitvector = AFF_CURSE;
  affect_to_char( victim, &af );

  af.location  = APPLY_SAVING_SPELL;
  af.modifier  = level / 8;
  affect_to_char( victim, &af );

  if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);

  send_to_char( "You feel unclean.\n\r", victim );
  if ( ch != victim )
    act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
  return;
}

/* RT replacement demonfire spell */

void spell_demonfire(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ( !IS_NPC(ch) && !IS_EVIL(ch) )
    {
      victim = ch;
      send_to_char("The demons turn upon you!\n\r",ch);
    }

  
  if (victim != ch)
    {
      act("$n calls forth the fire of {rde{Dmo{rn{Ds{x upon $N!",
      ch,NULL,victim,TO_ROOM);
      act("$n has assailed you with the fire of {rde{Dmo{rn{Ds{x.",
      ch,NULL,victim,TO_VICT);
      send_to_char("You conjure forth the fire of {rde{Dmo{rn{Ds{x.\n\r",ch);
    }

  dam = magic_dam(sn,level);
  if ( saves_spell( level, victim,DAM_NEGATIVE) )
    dam /= 2;
  spell_curse(gsn_curse, 3 * level / 4, ch, (void *) victim,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, TRUE);
}

void spell_detect_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) )
    {
      if (victim == ch)
    send_to_char("You can already sense evil.\n\r",ch);
      else
    act("$N can already detect evil.",ch,NULL,victim,TO_CHAR);
      return;
    }

  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
    {
      send_to_char("You are not righteous enough to be granted your holy powers.\n\r", ch);
      return;
    }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_EVIL;
  affect_to_char( victim, &af );
  send_to_char( "Your eyes detect evil in the world.\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}


void spell_detect_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if ( IS_AFFECTED(victim, AFF_DETECT_GOOD) )
    {
      if (victim == ch)
    send_to_char("You can already sense good.\n\r",ch);
      else
    act("$N can already detect good.",ch,NULL,victim,TO_CHAR);
      return;
    }

  if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
    {
      send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
      return;
    }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_GOOD;
  affect_to_char( victim, &af );
  send_to_char( "Your eyes detect good in the world.\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}



void spell_detect_hidden(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_DETECT_HIDDEN) )
    {
      if (victim == ch)
    send_to_char("You are already as alert as you can be. \n\r",ch);
      else
    act("$N can already sense hidden lifeforms.",ch,NULL,victim,TO_CHAR);
      return;
    }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_DETECT_HIDDEN;
  affect_to_char( victim, &af );
  send_to_char( "Your awareness improves.\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}



void spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
    {
      if (victim == ch)
    send_to_char("You can already see invisible.\n\r",ch);
      else
    act("$N can already see invisible things.",ch,NULL,victim,TO_CHAR);
      return;
    }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVIS;
  affect_to_char( victim, &af );
  send_to_char( "Your eyes can now detect the invisible.\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}



void spell_detect_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) )
    {
      if (victim == ch)
    send_to_char("You can already sense magical auras.\n\r",ch);
      else
    act("$N can already detect magic.",ch,NULL,victim,TO_CHAR);
      return;
    }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;
  affect_to_char( victim, &af );
  send_to_char( "Your eyes reveal magic in the world.\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}



void spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;

  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
    {
      send_to_char("You are not righteous enough to be granted your holy powers.\n\r", ch);
      return;
    }
  if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
    {
      send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
      return;
    }

  if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
      if ( obj->value[3] != 0 )
    send_to_char( "You smell poisonous {gfum{Ge{cs{x.\n\r", ch );
      else
    send_to_char( "It looks delicious.\n\r", ch );
    }
  else
    {
      send_to_char( "It doesn't look poisoned.\n\r", ch );
    }

  return;
}



void spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  //char buf[MSL];
  
  if ( !IS_NPC(ch) && IS_EVIL(ch) )
    victim = ch;

  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
    {
      send_to_char("You are not righteous enough to be granted your holy powers.\n\r", ch);
      return;
    }
  
  if ( IS_GOOD(victim) )
  {
      if ( IS_IN_CLAN( ch ) )
      {
        act( "The power of $g protects $N.", ch, NULL, victim, TO_CHAR );
        act( "The power of $g protects $N.", ch, NULL, victim, TO_ROOM );
      }
      else
      {
        act( "The power of {CM{cir{Ml{mya{x protects $N.", ch, NULL, victim, TO_CHAR );
        act( "The power of {CM{cir{Ml{mya{x protects $N.", ch, NULL, victim, TO_ROOM );
      }

      //if (!is_affected(victim, skill_lookup("sleep")))
      //  damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);
          
      return;
  }

  if ( IS_NEUTRAL(victim) )
    {
      act( "$N does not seem to be greatly affected.", ch, NULL, victim, TO_CHAR );
      if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);
      return;
    }

  if (victim->hit > (level * 4))
    dam = dice( level, 4 );
  else
    dam = UMAX(victim->hit, dice(level,4));
  if ( saves_spell( level, victim,DAM_HOLY) )
    dam /= 2;
  damage( ch, victim, dam, sn, DAM_HOLY ,TRUE, TRUE);
  return;
}


void spell_dispel_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
 
  if ( !IS_NPC(ch) && IS_GOOD(ch) )
    victim = ch;

  if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
    {
      send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
      return;
    }
 
  if ( IS_EVIL(victim) )
    {
      act( "$N is protected by $S {rev{Di{rl{x.", ch, NULL, victim, TO_CHAR );
      act( "$N is protected by $S {rev{Di{rl{x.", ch, NULL, victim, TO_ROOM );
      return;
    }
 
  if ( IS_NEUTRAL(victim) )
    {
      act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
      return;
    }
 
  if (victim->hit > (level * 4))
    dam = dice( level, 4 );
  else
    dam = UMAX(victim->hit, dice(level,4));
  if ( saves_spell( level, victim,DAM_NEGATIVE) )
    dam /= 2;
  damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, TRUE);
  return;
}


/* modified for enhanced use */

void spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool found = FALSE;

  if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)) {
    send_to_char("This is a SAFE room.\n\r",ch);
    return;
  }
    
  if (saves_spell(level, victim,DAM_OTHER))
    {
      send_to_char( "You feel a brief tingling sensation.\n\r",victim);
      send_to_char( "You failed.\n\r", ch);
      if (!is_affected(victim, skill_lookup("sleep")))
      damage( ch, victim, 1, sn,DAM_MENTAL,FALSE, TRUE);
      return;
    }
  found = magic_dispel(level,victim);
  /* begin running through the spells */ 

  if (found)
    send_to_char("Ok.\n\r",ch);
  else
    send_to_char("Spell failed.\n\r",ch);
  if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL,FALSE, TRUE);
  return;
}

void spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  send_to_char( "The {yea{wr{Dth{x trembles beneath your feet!\n\r", ch );
//  act( "$n makes the {yea{wr{Dth{x tremble and shiver.", ch, NULL, NULL, TO_ROOM );

  for ( vch = char_list; vch != NULL; vch = vch_next )
    {
      vch_next    = vch->next;
      if ( vch->in_room == NULL )
    continue;
      if (IS_IMMORTAL(vch))
      continue;
      if ( vch->in_room == ch->in_room )
    {
      if ( vch != ch && !is_safe_spell(ch,vch,TRUE,TRUE) &&
           !is_same_group(ch,vch)) {
        if (IS_AFFECTED(vch,AFF_FLYING))
          damage(ch,vch,0,sn,DAM_BASH,TRUE, TRUE);
        else
          damage( ch,vch,magic_dam(sn,level), sn, DAM_BASH,TRUE, TRUE);
      }
      continue;
    }

    if ( vch->in_room->area == ch->in_room->area )
    {
      if (vch->position != POS_SLEEPING )
        send_to_char( "The {yea{wr{Dth{x trembles and shivers.\n\r", vch );
      else
        send_to_char( "The {yea{wr{Dth{x trembles, momentarily disturbing your sleep.\n\r", vch );
    }
  }

  return;
}

void spell_enchant_armor( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA *paf; 
  int result, fail;
  int ac_bonus, added;
  bool ac_found = FALSE;

  if (obj->item_type != ITEM_ARMOR)
    {
      send_to_char("That isn't an armor.\n\r",ch);
      return;
    }

  if (obj->wear_loc != -1)
    {
      send_to_char("The item must be carried to be enchanted.\n\r",ch);
      return;
    }

  /* this means they have no bonus */
  ac_bonus = 0;
  fail = 25;    /* base 25% chance of failure */

  /* find the bonuses */

  if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
      {
    if ( paf->location == APPLY_AC )
      {
        ac_bonus = paf->modifier;
        ac_found = TRUE;
        fail += 5 * (ac_bonus * ac_bonus);
      }

    else  /* things get a little harder */
      fail += 20;
      }
 
  for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
      if ( paf->location == APPLY_AC )
      {
      ac_bonus = paf->modifier;
      ac_found = TRUE;
      fail += 5 * (ac_bonus * ac_bonus);
    }

      else /* things get a little harder */
    fail += 20;
    }

  /* apply other modifiers */
  fail -= level;

  if (IS_OBJ_STAT(obj,ITEM_BLESS))
    fail -= 15;
  if (IS_OBJ_STAT(obj,ITEM_GLOW))
    fail -= 5;

  fail = URANGE(5,fail,85);

  result = number_percent();
  if (IS_IMMORTAL(ch))
    result = 100;
  /* the moment of truth */
  if (result < (fail / 5))  /* item destroyed */
    {
      act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_CHAR);
      act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_ROOM);
      extract_obj(obj);
      return;
    }

  if (result < (fail / 3)) /* item disenchanted */
    {
      AFFECT_DATA *paf_next;

      act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
      act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
      obj->enchanted = TRUE;

      /* remove all affects */
      for (paf = obj->affected; paf != NULL; paf = paf_next)
    {
      paf_next = paf->next; 
      free_affect(paf);
    }
      obj->affected = NULL;

      /* clear all flags */
      obj->extra_flags = 0;
      return;
    }

  if ( result <= fail )  /* failed, no bad result */
    {
      send_to_char("Nothing seemed to happen.\n\r",ch);
      return;
    }

  /* okay, move all the old flags into new vectors if we have to */
  if (!obj->enchanted)
    {
      AFFECT_DATA *af_new;
      obj->enchanted = TRUE;

      for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
    {
      af_new = new_affect();
    
      af_new->next = obj->affected;
      obj->affected = af_new;

      af_new->where    = paf->where;
      af_new->type     = UMAX(0,paf->type);
      af_new->level    = paf->level;
      af_new->duration    = paf->duration;
      af_new->location    = paf->location;
      af_new->modifier    = paf->modifier;
      af_new->bitvector    = paf->bitvector;
    }
    }

  if (result <= (90 - level/5))  /* success! */
    {
      act("$p shimmers with a {yg{Yo{yld{x aura.",ch,obj,NULL,TO_CHAR);
      act("$p shimmers with a {yg{Yo{yld{x aura.",ch,obj,NULL,TO_ROOM);
      SET_BIT(obj->extra_flags, ITEM_MAGIC);
      added = -1;
    }
    
  else  /* exceptional enchant */
    {
      act("$p glows a brilliant {yg{Yo{yld{x!",ch,obj,NULL,TO_CHAR);
      act("$p glows a brilliant {yg{Yo{yld{x!",ch,obj,NULL,TO_ROOM);
      SET_BIT(obj->extra_flags,ITEM_MAGIC);
      SET_BIT(obj->extra_flags,ITEM_GLOW);
      added = -2;
    }
        
  /* now add the enchantments */ 

  if (obj->level < LEVEL_HERO)
    obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

  if (ac_found)
    {
      for ( paf = obj->affected; paf != NULL; paf = paf->next)
    {
      if ( paf->location == APPLY_AC)
        {
          paf->type = sn;
          paf->modifier += added;
          paf->level = UMAX(paf->level,level);
        }
    }
    }
  else /* add a new affect */
    {
      paf = new_affect();

      paf->where    = TO_OBJECT;
      paf->type    = sn;
      paf->level    = level;
      paf->duration    = -1;
      paf->location    = APPLY_AC;
      paf->modifier    =  added;
      paf->bitvector  = 0;
      paf->next    = obj->affected;
      obj->affected    = paf;
    }

}




void spell_enchant_weapon(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA *paf; 
  int result, fail;
  int hit_bonus, dam_bonus, added;
  bool hit_found = FALSE, dam_found = FALSE;

  if (obj->item_type != ITEM_WEAPON)
    {
      send_to_char("That isn't a weapon.\n\r",ch);
      return;
    }

  if (obj->wear_loc != -1)
    {
      send_to_char("The item must be carried to be enchanted.\n\r",ch);
      return;
    }

  /* this means they have no bonus */
  hit_bonus = 0;
  dam_bonus = 0;
  fail = 25;    /* base 25% chance of failure */

  /* find the bonuses */

  if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
      if ( paf->location == APPLY_HITROLL )
      {
        hit_bonus = paf->modifier;
        hit_found = TRUE;
        fail += 2 * (hit_bonus * hit_bonus);
      }

      else if (paf->location == APPLY_DAMROLL )
      {
        dam_bonus = paf->modifier;
        dam_found = TRUE;
        fail += 2 * (dam_bonus * dam_bonus);
      }
      else  /* things get a little harder */
        fail += 25;
    }
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
      if ( paf->location == APPLY_HITROLL )
      {
        hit_bonus = paf->modifier;
        hit_found = TRUE;
        fail += 2 * (hit_bonus * hit_bonus);
      }

      else if (paf->location == APPLY_DAMROLL )
      {
        dam_bonus = paf->modifier;
        dam_found = TRUE;
        fail += 2 * (dam_bonus * dam_bonus);
      }
      else /* things get a little harder */
        fail += 25;
    }

  /* apply other modifiers */
  fail -= 3 * level/2;

  if (IS_OBJ_STAT(obj,ITEM_BLESS))
    fail -= 15;
  if (IS_OBJ_STAT(obj,ITEM_GLOW))
    fail -= 5;

  fail = URANGE(5,fail,95);

  result = number_percent();

  if (IS_IMMORTAL(ch))
    result = 100;
  /* the moment of truth */
  if (result < (fail / 3))  /* item destroyed */
    {
      act("$p shivers violently and explodes!",ch,obj,NULL,TO_CHAR);
      act("$p shivers violently and explodes!",ch,obj,NULL,TO_ROOM);
      extract_obj(obj);
      return;
    }
#if goof
  if (result < (fail / 2)) /* item disenchanted */
    {
      AFFECT_DATA *paf_next;

      act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
      act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
      obj->enchanted = TRUE;

      /* remove all affects */
      for (paf = obj->affected; paf != NULL; paf = paf_next)
    {
      paf_next = paf->next; 
      free_affect(paf);
    }
      obj->affected = NULL;

      /* clear all flags */
      obj->extra_flags = 0;
      return;
    }
#endif

  if ( result <= fail )  /* failed, no bad result */
    {
      send_to_char("Nothing seemed to happen.\n\r",ch);
      return;
    }

  /* okay, move all the old flags into new vectors if we have to */
  if (!obj->enchanted)
    {
      AFFECT_DATA *af_new;
      obj->enchanted = TRUE;

      for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
    {
      af_new = new_affect();
    
      af_new->next = obj->affected;
      obj->affected = af_new;

      af_new->where    = paf->where;
      af_new->type     = UMAX(0,paf->type);
      af_new->level    = paf->level;
      af_new->duration    = paf->duration;
      af_new->location    = paf->location;
      af_new->modifier    = paf->modifier;
      af_new->bitvector    = paf->bitvector;
    }
    }

  if (result <= (100 - level/5))  /* success! */
    {
      act("$p glows {Bblu{ce{x.",ch,obj,NULL,TO_CHAR);
      act("$p glows {Bblu{ce{x.",ch,obj,NULL,TO_ROOM);
      SET_BIT(obj->extra_flags, ITEM_MAGIC);
      added = 1;
    }
    
  else  /* exceptional enchant */
    {
      act("$p glows a brilliant {Bblu{ce{x!",ch,obj,NULL,TO_CHAR);
      act("$p glows a brilliant {Bblu{ce{x!",ch,obj,NULL,TO_ROOM);
      SET_BIT(obj->extra_flags,ITEM_MAGIC);
      SET_BIT(obj->extra_flags,ITEM_GLOW);
      added = 2;
    }
        
  /* now add the enchantments */ 

  if (obj->level < LEVEL_HERO - 1)
    obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

  if (dam_found)
    {
      for ( paf = obj->affected; paf != NULL; paf = paf->next)
    {
      if ( paf->location == APPLY_DAMROLL)
        {
          paf->type = sn;
          paf->modifier += added;
          paf->level = UMAX(paf->level,level);
          if (paf->modifier > 4)
        SET_BIT(obj->extra_flags,ITEM_HUM);
        }
    }
    }
  else /* add a new affect */
    {
      paf = new_affect();

      paf->where    = TO_OBJECT;
      paf->type    = sn;
      paf->level    = level;
      paf->duration    = -1;
      paf->location    = APPLY_DAMROLL;
      paf->modifier    =  added;
      paf->bitvector  = 0;
      paf->next    = obj->affected;
      obj->affected    = paf;
    }

  if (hit_found)
    {
      for ( paf = obj->affected; paf != NULL; paf = paf->next)
    {
      if ( paf->location == APPLY_HITROLL)
            {
          paf->type = sn;
          paf->modifier += added;
          paf->level = UMAX(paf->level,level);
          if (paf->modifier > 4)
        SET_BIT(obj->extra_flags,ITEM_HUM);
            }
    }
    }
  else /* add a new affect */
    {
      paf = new_affect();
 
      paf->type       = sn;
      paf->level      = level;
      paf->duration   = -1;
      paf->location   = APPLY_HITROLL;
      paf->modifier   =  added;
      paf->bitvector  = 0;
      paf->next       = obj->affected;
      obj->affected   = paf;
    }

}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  //Removed per request of Laurelin 9-13-08-Aarchane
  /*
  if ( (level > 10) && (level - 10 > victim->level) )
  {
    send_to_char("They are too small to drain their energy.\n\r", ch);
    return;
  }*/

  if ( saves_spell( level, victim,DAM_NEGATIVE) )
    {
      send_to_char("You feel a momentary chill.\n\r",victim);      
      send_to_char("Your energy drain fails to lock on to their life force.\n\r",ch);      
      return;
    }


  if ( victim->level <= 2 )
    {
      dam         = ch->hit + 1;
    }
  else
    {
      gain_exp( victim, 0 - number_range( level/4, 3 * level / 4 ) );
      if (victim->mana/2 >= 2*level)
    victim->mana    -= 2*level;
      else
    victim->mana    /= 2;
      if (victim->move/2 >= 2*level)
    victim->move    -= 2*level;
      else
    victim->move /= 2;
      dam         = dice(1, level);
      ch->hit        += dam;
    }

    if (ch->hit >= GET_HP(ch))
      ch->hit = GET_HP(ch);
    if (ch->mana >= GET_MANA(ch))
      ch->mana = GET_MANA(ch);

  send_to_char("You feel your {rl{Ri{rfe{x slipping away!\n\r",victim);
  send_to_char("You {rd{Rra{rin{x your opponent's life force.\n\r",ch);
  damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, TRUE);

  return;
}



void spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  level    = UMAX(0, level);
  dam  = magic_dam(sn,level);
  if ( saves_spell( level, victim, DAM_FIRE) )
    dam /= 2;
  fire_effect((void *) victim, level/2,dam, TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_FIRE ,TRUE, TRUE);
    
  return;
}


void spell_fireproof(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA af;
 
  if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
    {
      act("$p is already protected from burning.",ch,obj,NULL,TO_CHAR);
      return;
    }
 
  af.where     = TO_OBJECT;
  af.type      = sn;
  af.level     = level;
  af.duration  = number_fuzzy(level / 4);
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = ITEM_BURN_PROOF;
 
  affect_to_obj(obj,&af);
 
  act("You protect $p from {Rf{ri{yre{x.",ch,obj,NULL,TO_CHAR);
  act("$p is surrounded by a protective aura.",ch,obj,NULL,TO_ROOM);
}



void spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = magic_dam(sn,level);
  if ( saves_spell( level, victim,DAM_FIRE) )
    dam /= 2;
  damage( ch, victim, dam, sn, DAM_FIRE ,TRUE, TRUE);
  return;
}



void spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) {
    if (!is_affected(victim, skill_lookup("sleep")))
      damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_AC;
  af.modifier  = 2 * level;
  af.bitvector = AFF_FAERIE_FIRE;
  affect_to_char( victim, &af );
  if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);
  send_to_char( "You are surrounded by a {mpink{x outline.\n\r", victim );
  act( "$n is surrounded by a {mpink{x outline.", victim, NULL, NULL, TO_ROOM );
  return;
}

void spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *ich;

  act( "$n conjures a cloud of {mpu{Mrp{mle{x {Dsm{wo{Dke{x.", ch, NULL, NULL, TO_ROOM );
  send_to_char( "You conjure a cloud of {mpu{Mrp{mle{D sm{wo{Dke{x.\n\r", ch );

  for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
      if (ich->invis_level > 0)
    continue;
      if (IS_WIZINVIS(ich,ch))
    continue;
      if ( ich == ch || saves_spell( level, ich,DAM_OTHER) )
    continue;


      affect_strip ( ich, gsn_inverted_light   );
      affect_strip ( ich, gsn_invis            );
      affect_strip ( ich, gsn_mass_invis       );
      affect_strip ( ich, gsn_sneak            );
      REMOVE_BIT   ( ich->affected_by, AFF_HIDE    );
      REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE    );
      REMOVE_BIT   ( ich->affected_by, AFF_SNEAK    );
      act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
      send_to_char( "You are revealed!\n\r", ich );
    }

  return;
}

void spell_floating_disc( int sn, int level,CHAR_DATA *ch,void *vo,int target )
{
  OBJ_DATA *disc, *floating;

  floating = get_eq_char(ch,WEAR_FLOAT);
  if (floating != NULL && IS_OBJ_STAT(floating,ITEM_NOREMOVE))
    {
      act("You can't remove $p.",ch,floating,NULL,TO_CHAR);
      return;
    }

  disc = create_object(get_obj_index(OBJ_VNUM_DISC), 0);
  disc->value[0]    = level * 10; /* 10 pounds per level capacity */
  disc->value[3]    = level * 5; /* 5 pounds per level max per item */
  disc->timer        = level * 2 - number_range(0,level / 2); 

  act("$n has created a floating disc.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You create a floating disc.\n\r",ch);
  obj_to_char(disc,ch);
  wear_obj(ch,disc,TRUE);
  return;
}


void spell_fly( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_FLYING) )
    {
      if (victim == ch)
    send_to_char("You are already airborne.\n\r",ch);
      else
    act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
      return;
    }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level + 3;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_FLYING;
  affect_to_char( victim, &af );
  send_to_char( "Your feet rise off the ground.\n\r", victim );
  act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
  return;
}

/* RT clerical berserking spell */

void spell_frenzy(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim,sn) || IS_AFFECTED(victim,AFF_BERSERK))
  {
      if (victim == ch)
        send_to_char("You are already in a frenzy.\n\r",ch);
      else
        act("$N is already in a frenzy.",ch,NULL,victim,TO_CHAR);
      return;
  }

  if (is_affected(victim,skill_lookup("calm")))
  {
      if (victim == ch)
        send_to_char("Why don't you just relax for a while?\n\r",ch);
      else
        act("$N doesn't look like $e wants to fight anymore.",
            ch,NULL,victim,TO_CHAR);
      return;
  }

  if ( ( IS_GOOD(ch) && !IS_GOOD( victim ) )
  ||   ( IS_NEUTRAL(ch) && !IS_NEUTRAL( victim ) )
  ||   ( IS_EVIL(ch) && !IS_EVIL( victim ) ) )
  {
      act("Your god doesn't seem to like $N.",ch,NULL,victim,TO_CHAR);
      return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration     = level / 3;
  af.modifier  = level / 6;
  af.bitvector = 0;

  af.location  = APPLY_HITROLL;
  affect_to_char(victim,&af);

  af.location  = APPLY_DAMROLL;
  affect_to_char(victim,&af);

  af.modifier  = 10 * (level / 12);
  af.location  = APPLY_AC;
  affect_to_char(victim,&af);

  if ( ch->alignment > 0 )
    send_to_char("You are filled with holy wrath!\n\r",victim);
  else if ( ch->alignment < 0 )
    send_to_char("You are filled with unholy wrath!\n\r", victim);
  else
    send_to_char("You are filled with wrath!\n\r", victim );

  act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}


void spell_giant_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
    {
      if (victim == ch)
    send_to_char("You are already as strong as you can get!\n\r",ch);
      else
    act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR);
      return;
    }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_STR;
  af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
  af.bitvector = 0;
  affect_to_char( victim, &af );
  send_to_char( "Your muscles surge with heightened power!\n\r", victim );
  act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
  return;
}



void spell_harm( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = UMAX(  20, victim->hit - dice(1,4) );
  if ( saves_spell( level, victim,DAM_HARM) )
    dam = UMIN( 50, dam / 2 );
  dam = UMIN( 100, dam );
  damage( ch, victim, dam, sn, DAM_HARM ,TRUE, TRUE);
  return;
}

/* RT haste spell */

void spell_haste( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE)
  ||   IS_SET(victim->off_flags,OFF_FAST))
  {
      if (victim == ch)
        send_to_char("You can't move any faster!\n\r",ch);
      else
        act("$N is already moving as fast as $E can.",

      ch,NULL,victim,TO_CHAR);
      return;
  }

  if (IS_AFFECTED(victim,AFF_SLOW))
  {
    if (!check_dispel(level,victim,skill_lookup("slow")))
    {
      if (victim != ch)
        send_to_char("Spell failed.\n\r",ch);

      send_to_char("You feel momentarily faster.\n\r",victim);
      return;
    }

    act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;

  if (victim == ch)
    af.duration  = level/2;
  else
    af.duration  = level/4;

  af.location  = APPLY_DEX;
  af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
  af.bitvector = AFF_HASTE;
  affect_to_char( victim, &af );

  send_to_char( "You feel yourself moving more quickly.\n\r", victim );
  act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);

  return;
}

void spell_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  victim->hit = UMIN( victim->hit + 100, GET_HP(victim) );
  update_pos( victim );
  send_to_char( "A warm feeling fills your body.\n\r", victim );
  if (victim->hit >= GET_HP(victim))
    send_to_char("Fully healed.\n\r",ch);
  else {
    if ( ch != victim )
      send_to_char( "You have healed wounds.\n\r", ch );
  }
  return;
}

void spell_heat_metal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  OBJ_DATA *obj_lose, *obj_next;
  int dam = 0;
  bool fail = TRUE;
 
  if (!saves_spell(level + 2,victim,DAM_FIRE) 
      &&  !IS_SET(victim->imm_flags,IMM_FIRE))
    {
      for ( obj_lose = victim->carrying;  obj_lose; obj_lose = obj_next)
        {
      obj_next = obj_lose->next_content;
      if ( number_range(1,2 * level) > obj_lose->level 
           &&   !saves_spell(level,victim,DAM_FIRE)
           &&   !IS_OBJ_STAT(obj_lose,ITEM_NONMETAL)
           &&   !IS_OBJ_STAT(obj_lose,ITEM_BURN_PROOF))
            {
          switch ( obj_lose->item_type ){
          case ITEM_ARMOR:
        if (obj_lose->wear_loc != -1) /* remove the item */
          {
            if (can_drop_obj(victim,obj_lose)  &&
            ((obj_lose->weight / 10) <  number_range(1,2 * get_curr_stat(victim,STAT_DEX)))
            &&  remove_obj( victim, obj_lose->wear_loc, TRUE ))
              {
            act("$n yelps and throws $p to the ground.",
                victim,obj_lose,NULL,TO_ROOM);
            act("You drop $p before it burns you.",
                victim,obj_lose,NULL,TO_CHAR);
            dam += (number_range(1,obj_lose->level) / 3);
            if (!melt_drop(victim, obj_lose)) {
              obj_from_char(obj_lose);
              obj_to_room(obj_lose, victim->in_room);
            }
            fail = FALSE;
              }
            else /* stuck on the body! ouch! */
              {
            act("Your skin is seared by $p!",
                victim,obj_lose,NULL,TO_CHAR);
            dam += (number_range(1,obj_lose->level));
            fail = FALSE;
              }

          }
        else /* drop it if we can */
          {
            if (can_drop_obj(victim,obj_lose))
              {
            act("$n yelps and throws $p to the ground.",
                victim,obj_lose,NULL,TO_ROOM);
            act("You drop $p before it burns you.",
                victim,obj_lose,NULL,TO_CHAR);
            dam += (number_range(1,obj_lose->level) / 6);
            if (!melt_drop(victim, obj_lose)) {
              obj_from_char(obj_lose);
              obj_to_room(obj_lose, victim->in_room);
            }
            /*              obj_to_room(obj_lose, victim->in_room);*/
            fail = FALSE;
              }
            else /* cannot drop */
              {
            act("Your skin is seared by $p!",
                victim,obj_lose,NULL,TO_CHAR);
            dam += (number_range(1,obj_lose->level) / 2);
            fail = FALSE;
              }
          }
        break;
          case ITEM_WEAPON:
        if (obj_lose->wear_loc != -1) /* try to drop it */
          {
            if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
              continue;

            if (can_drop_obj(victim,obj_lose) 
            &&  remove_obj(victim,obj_lose->wear_loc,TRUE))
              {
            act("$n is burned by $p, and tosses it to the ground.",
                victim,obj_lose,NULL,TO_ROOM);
            send_to_char(
                     "You throw your red-hot weapon to the ground.\n\r",
                     victim);
            dam += 1;
            if (!melt_drop(victim, obj_lose)) {
              obj_from_char(obj_lose);
              obj_to_room(obj_lose,victim->in_room);
            }
            /*              obj_to_room(obj_lose,victim->in_room);*/
            fail = FALSE;
              }
            else /* YOWCH! */
              {
            send_to_char("Your weapon sears your flesh!\n\r",
                     victim);
            dam += number_range(1,obj_lose->level);
            fail = FALSE;
              }
          }
        else /* drop it if we can */
          {
            if (can_drop_obj(victim,obj_lose))
              {
            act("$n yelps and throws $p to the ground.",
                victim,obj_lose,NULL,TO_ROOM);
            act("You remove $p and drop it before it burns you.",
                victim,obj_lose,NULL,TO_CHAR);
            dam += (number_range(1,obj_lose->level) / 6);
            if (!melt_drop(victim, obj_lose)) {
              obj_from_char(obj_lose);
              obj_to_room(obj_lose, victim->in_room);
            }
            /*obj_to_room(obj_lose, victim->in_room);*/
            fail = FALSE;
              }
            else /* cannot drop */
              {
            act("Your skin is seared by $p!",
                victim,obj_lose,NULL,TO_CHAR);
            dam += (number_range(1,obj_lose->level) / 2);
            fail = FALSE;
              }
          }
        break;
          }
        }
    }
        
    
    }
  if (fail)
    {
      send_to_char("Your spell had no effect.\n\r", ch);
      send_to_char("You feel momentarily warmer.\n\r",victim);
    }
  else /* damage! */
    {
      if (saves_spell(level,victim,DAM_FIRE))
    dam = 2 * dam / 3;
      damage(ch,victim,dam,sn,DAM_FIRE,TRUE, TRUE);
    }
}

/* RT really nasty high-level attack spell */
void spell_holy_word(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  int bless_num, curse_num, frenzy_num;
   
  bless_num = skill_lookup("bless");
  curse_num = skill_lookup("curse"); 
  frenzy_num = skill_lookup("frenzy");

  act("$n utters a word of {ydi{Wvi{yne {Wsa{enc{Yt{yion{x!",ch,NULL,NULL,TO_ROOM);
  send_to_char("You utter a word of {ydi{Wvi{yne {Wsa{enc{Yt{yion{x.\n\r",ch);
 
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next_in_room;

      if (IS_IMMORTAL(vch) && !can_see(ch, vch) )
    continue;

      if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
      (IS_EVIL(ch) && IS_EVIL(vch)) ||
      (IS_NEUTRAL(ch) && IS_NEUTRAL(vch)) )
    {
       send_to_char("You feel more powerful.\n\r",vch);
      spell_frenzy(frenzy_num,level,ch,(void *) vch,TARGET_CHAR); 
      spell_bless(bless_num,level,ch,(void *) vch,TARGET_CHAR);
    }

      else if ((IS_GOOD(ch) && IS_EVIL(vch)) ||
           (IS_EVIL(ch) && IS_GOOD(vch)) )
    {
      if (!is_safe_spell(ch,vch,FALSE,FALSE))
        {
          spell_curse(curse_num,level,ch,(void *) vch,TARGET_CHAR);
          send_to_char("You are struck down!\n\r",vch);
          dam = dice(level,6);
          damage(ch,vch,dam,sn,DAM_ENERGY,TRUE, TRUE);
        }
    }

      else if (IS_NEUTRAL(ch))
    {
      if (!is_safe_spell(ch,vch,FALSE, FALSE))
        {
          spell_curse(curse_num,level/2,ch,(void *) vch,TARGET_CHAR);
          send_to_char("You are struck down!\n\r",vch);
          dam = dice(level,4);
          damage(ch,vch,dam,sn,DAM_ENERGY,TRUE, TRUE);
        }
    }
    }  
    
  send_to_char("You feel drained.\n\r",ch);
  ch->move -= ch->move/2;
  ch->hit -= ch->level/2;
  if (ch->hit <=1)
    ch->hit =1;
}
 
void spell_identify( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  show_obj_stats(ch, obj);
  return;
}



void spell_infravision( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_INFRARED) )
    {
      if (victim == ch)
    send_to_char("You can already see in the dark.\n\r",ch);
      else
    act("$N already has infravision.\n\r",ch,NULL,victim,TO_CHAR);
      return;
    }
  act( "$n's eyes glow {rr{Re{rd{x.\n\r", victim, NULL, NULL, TO_ROOM );

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 2 * level;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_INFRARED;
  affect_to_char( victim, &af );
  send_to_char( "Your eyes glow {rr{Re{rd{x.\n\r", victim );
  return;
}



void spell_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;

  /* object invisibility */
  if (target == TARGET_OBJ)
    {
      obj = (OBJ_DATA *) vo;    

      if (IS_OBJ_STAT(obj,ITEM_INVIS))
    {
      act("$p is already invisible.",ch,obj,NULL,TO_CHAR);
      return;
    }
    
      af.where    = TO_OBJECT;
      af.type        = sn;
      af.level    = level;
      af.duration    = level + 12;
      af.location    = APPLY_NONE;
      af.modifier    = 0;
      af.bitvector    = ITEM_INVIS;
      affect_to_obj(obj,&af);

      act("$p fades out of sight.",ch,obj,NULL,TO_ALL);
      return;
    }

  /* character invisibility */
  victim = (CHAR_DATA *) vo;

  //Check for already invisible.
  if ( IS_AFFECTED(victim, AFF_INVISIBLE) )
  {
    if ( victim == ch )
        send_to_char( "You can't get more invisible than you already are.\n\r", victim );
    else
        send_to_char( "They are already invisible.\n\r", ch );
      
    return;
  }

  act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level + 12;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_INVISIBLE;
  affect_to_char( victim, &af );
  send_to_char( "You fade out of existence.\n\r", victim );
  return;
}



void spell_know_alignment(int sn,int level,CHAR_DATA *ch,void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  char *msg;
  int ap;

  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
    {
      send_to_char("You are not righteous enough to be granted your holy powers.\n\r", ch);
      return;
    }
  if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
    {
      send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
      return;
    }

  ap = victim->alignment;

  if ( ap >  700 ) msg = "$N has a pure and good aura.";
  else if ( ap >  350 ) msg = "$N is of excellent moral character.";
  else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
  else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
  else if ( ap > -350 ) msg = "$N lies to $S friends.";
  else if ( ap > -700 ) msg = "$N is a black-hearted murderer.";
  else msg = "$N is the embodiment of pure evil!";

  act( msg, ch, NULL, victim, TO_CHAR );
  return;
}



void spell_lightning_bolt(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  level    = UMAX(0, level);
  dam    = magic_dam(sn,level);
  if ( saves_spell( level, victim,DAM_LIGHTNING) )
    dam /= 2;
  damage( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE, TRUE);
  return;
}



void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  char buf[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  char    **pObjStrShow;
  bool    *pObjWhere;
  int     *pObjNumShow;
  bool    bObjWhere;
  bool    fCombine;
  int     nShow;
  int     iShow;
  int number = 0, max_found;

  number = 0;
  max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;

  pObjStrShow = alloc_mem( max_found * sizeof(char *) );
  pObjWhere    = alloc_mem( max_found * sizeof(bool)      );
  pObjNumShow = alloc_mem( max_found * sizeof(int)    );
  nShow    = 0;
  buffer = new_buf();
 
  for ( obj = object_list; obj != NULL; obj = obj->next )
  {
      if ( !can_see_obj( ch, obj )
      ||   !is_name( target_name, obj->name ) 
      ||   IS_OBJ_STAT( obj, ITEM_NOLOCATE )
      ||   number_percent() > 2 * level
      ||   level < obj->level
      ||   IS_SET( obj->pIndexData->area->flags, AREA_DRAFT )
      ||   obj->pIndexData->area->continent != ch->in_room->area->continent )
        continue;

      number++;

      for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
    ;

      /* First case: Object is carried by someone and you can see
     them. Immortal can see anyways with can_see*/
      if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by))
    {
      mprintf(sizeof(buf), buf, "%s", PERS(in_obj->carried_by, ch) );
      bObjWhere = FALSE;
    }
      /* Second Case, if you immortal, and there is a room, then you
     see it no matter what */
      else if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
    {
      mprintf(sizeof(buf), buf, "%s [Room %d]",
          in_obj->in_room->name, in_obj->in_room->vnum);
      bObjWhere = TRUE;
    }
      /* Third case: Object is carried by someone and you cannot see
     them. if its in a room, show the room name, else not
      */
      else if (!IS_IMMORTAL(in_obj->carried_by))
    {
      mprintf(sizeof(buf), buf, "%s",
          in_obj->in_room == NULL
          ? "somewhere" : in_obj->in_room->name );
      bObjWhere = TRUE;
    } else
      continue;
    


      /*
       * Look for duplicates, case sensitive.
       * Matches tend to be near end so run loop backwords.
       */
      fCombine = FALSE;
      for ( iShow = nShow - 1; iShow >= 0; iShow-- )
    {
      if ( pObjWhere[iShow] == bObjWhere
           &&     !strcmp( pObjStrShow[iShow], buf ) )
        {
          pObjNumShow[iShow]++;
          fCombine = TRUE;
          break;
        }
    }

      /*
       * Couldn't combine.
       */
      if ( !fCombine )
    {
      pObjStrShow[nShow] = str_dup( buf, pObjStrShow[nShow] );
      pObjWhere  [nShow] = bObjWhere;
      pObjNumShow[nShow] = 1;
      nShow++;
    }

      if (number >= max_found)
    break;
    }

  /*
   * Output the formatted list.
   */
  for ( iShow = 0; iShow < nShow; iShow++ )
    {
      if ( pObjStrShow[iShow][0] == '\0' )
    {
      free_string( pObjStrShow[iShow] );
      continue;
    }

      bprintf( buffer, "{W[{D%3d{W]{w %s %s %s\n\r",
           pObjNumShow[iShow],
           pObjNumShow[iShow] == 1 ? "is" : "are",
           pObjWhere[iShow] ? "in" : "carried by",
           pObjStrShow[iShow] );

      free_string( pObjStrShow[iShow] );
    }

  if ( number == 0 )
    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
  else
    page_to_char( buf_string( buffer ), ch );

  free_buf(buffer);
  free_mem( pObjStrShow);
  free_mem( pObjWhere );
  free_mem( pObjNumShow);

  return;
}



void spell_magic_missile( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,i;
  ROOM_INDEX_DATA *ch_orig_room, *victim_orig_room;
  level    = UMAX(0, level);
  /* 1 Missile for every ten levels of the caster */

  if(level<10) i=1; else i=level/10+1;
    
  ch_orig_room = ch->in_room;
  victim_orig_room = victim->in_room;
  for(; i>=0; i--) {
    dam= magic_dam(sn,level);
    if ( saves_spell( level, victim,DAM_ENERGY) )
      dam /= 2;
    if (victim->in_room != ch->in_room || victim->position == POS_DEAD)
      return;
    if (damage( ch, victim, dam, sn, DAM_ENERGY ,TRUE, TRUE) == FALSE)
      return;
    if (ch->in_room != ch_orig_room || victim->in_room != victim_orig_room)
      i = -1;
  }
  return;
}

void spell_mass_healing(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *gch;
  int heal_num, refresh_num;
    
  heal_num = skill_lookup("heal");
  refresh_num = skill_lookup("refresh"); 

  for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
  {
    if ( IS_WIZINVIS(gch,ch) )
        continue;

    if ( ( IS_NPC(ch) && IS_NPC(gch) )
    ||   ( !IS_NPC(ch) && !IS_NPC(gch) )
    ||   ( !IS_NPC(ch) && IS_NPC(gch) && is_same_group(ch,gch) ) )
      {
        spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);
        spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR);  
      }
  }
}
        

void spell_mass_invis( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  CHAR_DATA *gch;

  for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
      if ( !is_same_group( gch, ch ) || IS_AFFECTED(gch, AFF_INVISIBLE) 
       || IS_WIZINVIS(gch,ch))
    continue;
      act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly fade out of existence.\n\r", gch );

      af.where     = TO_AFFECTS;
      af.type      = sn;
      af.level     = level/2;
      af.duration  = 24;
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = AFF_INVISIBLE;
      affect_to_char( gch, &af );
    }
  send_to_char( "Ok.\n\r", ch );

  return;
}



void spell_null( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  send_to_char( "That's not a spell!\n\r", ch );
  return;
}



void spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
      if (victim == ch)
    send_to_char("You are already out of phase.\n\r",ch);
      else
    act("$N is already shifted out of phase.",ch,NULL,victim,TO_CHAR);
      return;
    }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = number_fuzzy( level / 4 );
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_PASS_DOOR;
  affect_to_char( victim, &af );
  act( "$n turns {Wtranslucent{x.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You turn {Wtranslucent{x.\n\r", victim );
  return;
}

/* RT plague spell, very nasty */

void spell_plague( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn)) {
    send_to_char("Their body is riddled with {gd{Di{rs{gease{x.\n\r",ch);
      if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
    return;
  }
    
  if (saves_spell(level,victim,DAM_DISEASE) || 
      (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
    {
      if (ch == victim)
    send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
      else
    act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
      if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
      return;
    }

  af.where     = TO_AFFECTS;
  af.type       = sn;
  af.level      = level * 3/4;
  af.duration  = level;
  af.location  = APPLY_STR;
  af.modifier  = -5; 
  af.bitvector = AFF_PLAGUE;
  affect_join(victim,&af);
  if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
   
  send_to_char
    ("You scream in agony as plague {rs{Do{gres{x erupt from your skin.\n\r",victim);
  act("$n screams in agony as plague {rs{Do{gres{x erupt from $s skin.",
      victim,NULL,NULL,TO_ROOM);
}

void spell_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;


  if (target == TARGET_OBJ)
  {
    obj = (OBJ_DATA *) vo;

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {
      if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
      {
        act("Your spell fails to corrupt $p.",ch,obj,NULL,TO_CHAR);
        return;
      }
      obj->value[3] = 1;
      act("$p is infused with a poisonous {gg{Ca{gs{x.",ch,obj,NULL,TO_ALL);
      return;
    }

    if (obj->item_type == ITEM_WEAPON)
    {
      if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
      ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
      ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
      ||  IS_WEAPON_STAT(obj,WEAPON_MANA_DRAIN)
      ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
      ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
      ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
      ||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
      {
        act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
        return;
      }

      if (IS_WEAPON_STAT(obj,WEAPON_POISON))
      {
          act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
          return;
      }

      af.where     = TO_WEAPON;
      af.type     = sn;
      af.level     = level / 2;
      af.duration     = level/8;
      af.location     = APPLY_NONE;
      af.modifier     = 0;
      af.bitvector = WEAPON_POISON;
      affect_to_obj(obj,&af);

      act("$p is coated with deadly {gv{ce{Gn{gom{x.",ch,obj,NULL,TO_ALL);
      return;
    }

    act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
    return;
  }

  victim = (CHAR_DATA *) vo;
  if (is_affected(victim, sn))
    {
      send_to_char("They are already sickly looking.\n\r",ch);
      if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
      return;
    }
  if ( saves_spell( level, victim,DAM_POISON) )
    {
      act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
      send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
      if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
      return;
    }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_STR;
  af.modifier  = -2;
  af.bitvector = AFF_POISON;
  affect_join( victim, &af );
  if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
  send_to_char( "You feel very {cs{gick{x.\n\r", victim );
  act("$n looks very {ci{gll{x.",victim,NULL,NULL,TO_ROOM);
  return;
}



void spell_protection_evil(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
    {
      send_to_char("You are not righteous enough to be granted your holy powers.\n\r", ch);
      return;
    }
 
  if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL) 
       ||   IS_AFFECTED(victim, AFF_PROTECT_GOOD)
       ||   IS_SAFFECTED(victim, SAFF_PROTECT_NEUTRAL)
       ||   IS_SAFFECTED(victim, SAFF_PROTECT_HOLY)
       ||   IS_SAFFECTED(victim, SAFF_PROTECT_NEGATIVE))
    {
      if (victim == ch)
    send_to_char("You are already protected.\n\r",ch);
      else
    act("$N is already protected.",ch,NULL,victim,TO_CHAR);
      return;
    }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 24;
  af.location  = APPLY_SAVING_SPELL;
  af.modifier  = -1;
  af.bitvector = AFF_PROTECT_EVIL;
  affect_to_char( victim, &af );
  send_to_char( "You feel {yh{Wo{yly{x and pure.\n\r", victim );
  if ( ch != victim )
    act("$N is protected from {rev{Di{rl{x.",ch,NULL,victim,TO_CHAR);
  return;
}
 
void spell_protection_good(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
  {
      send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
      return;
  }
 
  if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) 
  ||   IS_AFFECTED(victim, AFF_PROTECT_EVIL)
  ||   IS_SAFFECTED(victim, SAFF_PROTECT_NEUTRAL)
  || ( IS_SAFFECTED(victim, SAFF_PROTECT_HOLY)
  &&   !IS_CLASS_OCCULTIST( victim ) )
  ||   IS_SAFFECTED(victim, SAFF_PROTECT_NEGATIVE))
  {
      if (victim == ch)
        send_to_char("You are already protected.\n\r",ch);
      else
        act("$N is already protected.",ch,NULL,victim,TO_CHAR);
      return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 24;
  af.location  = APPLY_SAVING_SPELL;
  af.modifier  = -1;
  af.bitvector = AFF_PROTECT_GOOD;
  affect_to_char( victim, &af );
  send_to_char( "You feel aligned with {rda{Dr{rk{Dn{ress{x.\n\r", victim );
  if ( ch != victim )
    act("$N is protected from {yg{Wo{yod{x.",ch,NULL,victim,TO_CHAR);
  return;
}

#ifdef INQUISITOR
/* Before this spell is compiled, changes need to be made to the other
 * protection spells and to the damage routine (where the other protection
 * spells are checked for.*/

void spell_protection_negative(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
    {
      send_to_char("You are not righteous enough to be granted your holy powers.\n\r", ch);
      return;
    }

  if ( IS_SAFFECTED(victim, SAFF_PROTECT_HOLY)
       || IS_SAFFECTED(victim, SAFF_PROTECT_NEGATIVE)
       || IS_AFFECTED(victim, AFF_PROTECT_GOOD)
       || IS_AFFECTED(victim, AFF_PROTECT_EVIL)
       || IS_SAFFECTED(victim, SAFF_PROTECT_NEUTRAL))
    {
      if (victim == ch)
    send_to_char("You are already protected.\n\r", ch);
      else
    act("$N is already protected.", ch, NULL, victim, TO_CHAR);
      return;
    }

  af.where     = TO_SPELL_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 24;
  af.location  = APPLY_SAVING_SPELL;
  af.modifier   = -1;
  af.bitvector = SAFF_PROTECT_NEGATIVE;
  affect_to_char( victim, &af );
  send_to_char( "You feel {yd{Wi{yv{Wi{yne{x energies flow through you.\n\r", victim);
  if (ch != victim)
    act("$N is protected from {rneg{Da{rt{Di{rve{x energies", ch, NULL, victim, TO_CHAR);
  return;
}

/* Before this spell is compiled, changes need to be made to the other
 * protection spells and to the damage routine (where the other protection
 * spells are checked for.*/
void spell_protection_holy(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
  {
      send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
      return;
  }

  if ( IS_SAFFECTED(victim, SAFF_PROTECT_HOLY)
  || IS_SAFFECTED(victim, SAFF_PROTECT_NEGATIVE)
  || ( IS_AFFECTED(victim, AFF_PROTECT_GOOD)
  &&   !IS_CLASS_OCCULTIST( victim ) )
  || IS_AFFECTED(victim, AFF_PROTECT_EVIL)
  || IS_SAFFECTED(victim, SAFF_PROTECT_NEUTRAL))
  {
      if (victim == ch)
        send_to_char("You are already protected.\n\r", ch);
      else
        act("$N is already protected.", ch, NULL, victim, TO_CHAR);
      return;
  }

  af.where     = TO_SPELL_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 24;
  af.location  = APPLY_SAVING_SPELL;
  af.modifier  = -1;
  af.bitvector = SAFF_PROTECT_HOLY;
  affect_to_char( victim, &af );
  send_to_char( "{rDem{Do{rn{Di{rc{x energies surge through you.\n\r", victim);
  if (ch != victim)
    act("$N is protected from {yd{Wi{yv{Wi{yne{x energies", ch, NULL, victim, TO_CHAR);
  return;
}
#endif


void spell_ray_of_truth (int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, align;
 
  if (IS_EVIL(ch) )
    {
      victim = ch;
      send_to_char("The energy explodes inside you!\n\r",ch);
    }
 
  if (victim != ch)
    {
      act("$n raises $s hand, and a blinding {Wray{x of {yl{Yig{Wh{wt{x shoots forth!",
      ch,NULL,NULL,TO_ROOM);
      send_to_char(
           "You raise your hand and a blinding {Wray{x of {yl{Yig{Wh{wt{x shoots forth!\n\r",
           ch);
    }

  if (IS_GOOD(victim))
    {
      act("$n seems unharmed by the light.",victim,NULL,victim,TO_ROOM);
      send_to_char("The light seems powerless to affect you.\n\r",victim);
      return;
    }

  dam = dice( level, 10 );
  if ( saves_spell( level, victim,DAM_HOLY) )
    dam /= 2;

  align = victim->alignment;
  align -= 350;

  if (align < -1000)
    align = -1000 + (align + 1000) / 3;

  dam = (dam * align * align) / 1000000;

  spell_blindness(gsn_blindness, 
          3 * level / 4, ch, (void *) victim,TARGET_CHAR);
  damage( ch, victim, dam, sn, DAM_HOLY ,TRUE, TRUE);
}


void spell_recharge( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int chance, percent;

  if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF)
    {
      send_to_char("That item does not carry charges.\n\r",ch);
      return;
    }

  if (obj->value[0] >= 3 * level / 2)
    {
      send_to_char("Your skills are not great enough for that.\n\r",ch);
      return;
    }

  if (obj->value[1] == 0)
    {
      send_to_char("That item has already been recharged once.\n\r",ch);
      return;
    }

  chance = 40 + 2 * level;

  chance -= obj->value[3]; /* harder to do high-level spells */
  chance -= (obj->value[1] - obj->value[2]) *
    (obj->value[1] - obj->value[2]);

  chance = UMAX(level/2,chance);

  percent = number_percent();

  if (percent < chance / 2)
    {
      act("$p glows softly.",ch,obj,NULL,TO_CHAR);
      act("$p glows softly.",ch,obj,NULL,TO_ROOM);
      obj->value[2] = UMAX(obj->value[1],obj->value[2]);
      obj->value[1] = 0;
      return;
    }

  else if (percent <= chance)
    {
      int chargeback,chargemax;

      act("$p glows softly.",ch,obj,NULL,TO_CHAR);
      act("$p glows softly.",ch,obj,NULL,TO_CHAR);

      chargemax = obj->value[1] - obj->value[2];
    
      if (chargemax > 0)
    chargeback = UMAX(1,chargemax * percent / 100);
      else
    chargeback = 0;

      obj->value[2] += chargeback;
      obj->value[1] = 0;
      return;
    }    

  else if (percent <= UMIN(95, 3 * chance / 2))
    {
      send_to_char("Nothing seems to happen.\n\r",ch);
      if (obj->value[1] > 1)
    obj->value[1]--;
      return;
    }

  else /* whoops! */
    {
      act("$p glows brightly and explodes!",ch,obj,NULL,TO_CHAR);
      act("$p glows brightly and explodes!",ch,obj,NULL,TO_ROOM);
      extract_obj(obj);
    }
}

void spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  victim->move = UMIN( victim->move + level, victim->max_move );
  if (victim->max_move == victim->move)
    {
      send_to_char("You feel fully refreshed!\n\r",victim);
      if (ch != victim)
    act("$N looks fully refreshed.", ch, NULL, victim, TO_CHAR);
    }
  else
    send_to_char( "You feel less tired.\n\r", victim );
  if ( ch != victim )
    act("$N looks refreshed.", ch, NULL, victim, TO_CHAR);
  return;
}

void spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  bool found = FALSE;

  /* do object cases first */
  if (target == TARGET_OBJ)
    {
      obj = (OBJ_DATA *) vo;

      if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
      if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE)
          &&  !saves_dispel(level + 2,obj->level,0))
        {
          REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
          REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
          act("$p glows blue.",ch,obj,NULL,TO_ALL);
          return;
        }

      act("The curse on $p is beyond your power.",ch,obj,NULL,TO_CHAR);
      return;
    }
      act("There doesn't seem to be a curse on $p.",ch,obj,NULL,TO_CHAR);
      return;
    }

  /* characters */
  victim = (CHAR_DATA *) vo;

  if (check_dispel(level,victim,gsn_curse))
    {
      found = TRUE;
      send_to_char("You feel better.\n\r",victim);
      act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
    }

  for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
    {
      if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
      &&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
        {   /* attempt to remove curse */
      if (!saves_dispel(level,obj->level,0))
            {
          found = TRUE;
          REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
          REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
          act("Your $p glows {cblue{x.",victim,obj,NULL,TO_CHAR);
          act("$n's $p glows {cblue{x.",victim,obj,NULL,TO_ROOM);
            }
    }
    }
  if (!found)
    send_to_char("Your attempt to remove the curse failed.\n\r",ch);
}

void spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_sanc_spelled(victim))
    {
      if (victim == ch)
    send_to_char("You are already in sanctuary.\n\r",ch);
      else
    act("$N is already in sanctuary.",ch,NULL,victim,TO_CHAR);
      return;
    }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 6;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_SANCTUARY;
  affect_to_char( victim, &af );
  act( "$n is surrounded by a {Wwhite{x aura.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You are surrounded by a {Wwhite{x aura.\n\r", victim );
  return;
}

void spell_fade_out( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_sanc_spelled(ch) )
    {
        send_to_char("You are already in some sort of sanctuary.\n\r",ch);
        return;
    }

    if ( victim != ch )
    {
        send_to_char( "You cannot cast this on someone else.\n\r", ch );
        return;
    }

    af.where     = TO_AFFECTS2;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF2_FADE_OUT;
    affect_to_char (victim, &af );
    act( "$n seems to begin to {Wf{wa{Dde{x slightly in front of you.",victim, NULL, NULL, TO_ROOM );
    send_to_char( "You begin to {Wf{wa{Dde{x into the background.\n\r", victim );
    return;
}



void spell_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
    {
      if (victim == ch)
    send_to_char("You are already shielded from harm.\n\r",ch);
      else
    act("$N is already protected by a shield.",ch,NULL,victim,TO_CHAR);
      return;
    }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 8 + level;
  af.location  = APPLY_AC;
  af.modifier  = -20;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  act( "$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You are surrounded by a force shield.\n\r", victim );
  return;
}



void spell_shocking_grasp(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  level    = UMAX(0, level);
  dam        = magic_dam(sn,level);
  if ( saves_spell( level, victim,DAM_LIGHTNING) )
    dam /= 2;
  damage( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE, TRUE);
  return;
}



void spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  
  if ( IS_AFFECTED(victim, AFF_SLEEP) ||
       is_affected(victim, sn))
    {
      send_to_char("Your victim is already asleep.\n\r",ch);
      return;
    }

  if (((IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD))
       ||   (level + 2) < victim->level
       ||   saves_spell( level-4, victim,DAM_CHARM) 
       ||   (IS_SET(victim->in_room->room_flags,ROOM_ARENA)))) {
    send_to_char("Sleep failed.\n\r",ch);
    damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/15;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_SLEEP;
  affect_join( victim, &af );

  if ( IS_AWAKE(victim) )
    {
      send_to_char( "You feel very sleepy...{bzz{cz{Dz{cz{bz{x.\n\r", victim );
      act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
      victim->position = POS_SLEEPING;
    } else {
      send_to_char("You feel your sleep deepen...\n\r",victim);
      act( "$n looks more peaceful while sleeping.", victim, NULL, NULL, TO_ROOM );
    }

  return;
}

void spell_slow( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
  {
      if (victim == ch)
    send_to_char("You can't move any slower!\n\r",ch);
      else
    act("$N can't get any slower than that.",
        ch,NULL,victim,TO_CHAR);
      if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
      return;
  }

  if ( ch == victim
  &&   is_affected( ch, skill_lookup( "haste" ) )
  &&   ch->pcdata->learned[sn] > 89 )
//  &&   level > 50 )
  {
      remove_affect( ch, TO_AFFECTS, AFF_HASTE );
      send_to_char( "You are no longer moving so quickly.\n\r", ch );
      return;
  }

  if ( saves_spell( level, victim, DAM_OTHER )
  ||   IS_SET( victim->imm_flags, IMM_MAGIC ) )
  {
      if (victim != ch)
        send_to_char("Nothing seemed to happen.\n\r",ch);

      send_to_char("You feel momentarily lethargic.\n\r",victim);

      if (!is_affected(victim, skill_lookup("sleep")))
        damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);

      return;
  }
 
  if (IS_AFFECTED(victim,AFF_HASTE))
  {
      if (!check_dispel(level,victim,skill_lookup("haste")))
      {
        if (victim != ch)
            send_to_char("Spell failed.\n\r",ch);

        send_to_char("You feel momentarily slower.\n\r",victim);

           if (!is_affected(victim, skill_lookup("sleep")))
            damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
        return;
      }

      act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);

      if (!is_affected(victim, skill_lookup("sleep")))
        damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);

      return;
    }
 

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/2;
  af.location  = APPLY_DEX;
  af.modifier  = level / 9;//-1 - (level >= 18) - (level >= 25) - (level >= 32);
  af.bitvector = AFF_SLOW;
  affect_to_char( victim, &af );
//  if (!is_affected(victim, skill_lookup("sleep")))
//    damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);
  send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
  act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
  return;
}




void spell_stone_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( ch, sn ) )
    {
      if (victim == ch)
    send_to_char("Your skin is already as hard as a rock.\n\r",ch); 
      else
    act("$N is already as hard as can be.",ch,NULL,victim,TO_CHAR);
      return;
    }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_AC;
  af.modifier  = -40;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  act( "$n's skin turns to {Ds{wt{Done{x.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "Your skin turns to {Ds{wt{Done{x.\n\r", victim );
  return;
}



void spell_summon( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim;

  if ( ( victim = get_char_world( ch, target_name ) ) == NULL
  ||   victim == ch
  ||   victim->in_room == NULL
  ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
  ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
  ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
  ||   IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
  ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
  ||   IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)
  ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
  ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
  ||   IS_SET(victim->in_room->room_flags, ROOM_ARENA)
  ||   IS_SET(ch->in_room->room_flags, ROOM_ARENA)
  ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
  ||   victim->in_room->area->continent != ch->in_room->area->continent
  ||   victim->level >= level + 3
  ||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)
  ||   victim->fighting != NULL
  ||   (victim->spec_fun == spec_assassin)
  ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
  ||   (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
  ||   (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON))
  ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER)) )
  {
      send_to_char( "You failed.\n\r", ch );
      return;
  }

  act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
  move_to_room( victim, ch->in_room );
  act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
  act( "$n has summoned you!", ch, NULL, victim,   TO_VICT );
  do_function(victim, &do_look, "auto" );
  if (IS_NPC(victim))
    SET_BIT(victim->act2,ACT2_NOWANDEROFF);
  return;
}



void spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
//  CHAR_DATA *vsearch=NULL;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  ROOM_INDEX_DATA *pRoomIndex;
  bool is_valid_room;  /*Newbie areas and no-recall rooms are not valid rooms*/
  int i = 0;

  if ( victim->in_room == NULL )
    return;

  if ( ( !IS_IMMORTAL( ch ) || !IS_NPC( ch ) )
  &&   ch != victim )
  {
    send_to_char( "You may not cast this upon another.\n\r", ch );
    return;
  }

  if ( IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
  || ( IS_SET( victim->in_room->area->flags, AREA_DRAFT ))
  || ( victim != ch && IS_SET(victim->imm_flags,IMM_SUMMON))
  || ( IS_SET(victim->in_room->room_flags,ROOM_ARENA))
  || ( IS_SET(victim->in_room->room_flags,ROOM_NO_TELEPORT))
  || ( !IS_NPC(ch) && victim->fighting != NULL )
  || ( IS_AFFECTED(victim, AFF_CHARM) && (victim == ch))
  || ( IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
  || ( IS_AFFECTED(victim, AFF_CURSE))
  || ( (IS_SAFFECTED(victim, SAFF_ADRENALIZE) ) && (victim == ch))
  || ( (victim != ch) && ( saves_spell( level - 3, victim,DAM_OTHER))))
  {
      send_to_char( "You failed.\n\r", ch );
      return;
  }

  /*if (number_percent() > 98)
    {
      send_to_char("AHHHHHHHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHHHHHHHHHHHHH\n\r",victim);
      damage(ch,victim,8000,0,DAM_NEGATIVE,FALSE, FALSE);
      return;
    }*/
  is_valid_room = FALSE;
  while (!is_valid_room)
  {
    pRoomIndex = get_random_room(victim);
    i++;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_NO_RECALL)
    || ( ( IS_SET(pRoomIndex->room_flags, ROOM_NEWBIES_ONLY) )
    && ( victim->level > LEVEL_NEWBIE) ) 
    || ( IS_SET(pRoomIndex->room_flags, ROOM_NO_TELEPORT) ) 
    || ( IS_SET(pRoomIndex->room_flags, ROOM_ARENA) ) 
    || ( IS_SET(pRoomIndex->area->flags, AREA_DRAFT ) ) 
    || ( IS_SET(pRoomIndex->room_flags, ROOM_NOMAGIC) )
    || ( pRoomIndex->clan )
    || ( victim->in_room->area->continent != pRoomIndex->area->continent ) )
    {
      is_valid_room = FALSE;
    }
    else
    {
      is_valid_room = TRUE;
    }

    if ( i == 50 )
    {
      send_to_char( "Spell Failed!\n\r", ch );
      return;
    }
  }

  if (victim != ch)
    send_to_char("You have been teleported!\n\r",victim);

  act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
  move_to_room( victim, pRoomIndex );
  act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
  do_function(victim, &do_look, "auto" );
  return;
}



void spell_ventriloquate( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char speaker[MAX_INPUT_LENGTH];
  CHAR_DATA *vch;

  target_name = one_argument( target_name, speaker );

  if ((vch =get_char_room(ch, speaker)) == NULL)
    {
      send_to_char("That person is not in the room.\n\r",ch);
      return;
    }
      
  mprintf(sizeof(buf1), buf1, "{g%s {gsays '{w%s{g'{x\n\r",              IS_NPC(vch) ? vch->short_descr : vch->name, target_name);
  mprintf(sizeof(buf2), buf2, "{gSomeone makes %s {gsay '{w%s{g'{x\n\r", IS_NPC(vch) ? vch->short_descr : vch->name, target_name);
  buf1[0] = UPPER(buf1[0]);

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
      if (!is_exact_name( speaker, vch->name) && IS_AWAKE(vch))
    send_to_char( saves_spell(level,vch,DAM_OTHER) ? buf2 : buf1, vch );
    }

  return;
}



void spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
  {
    send_to_char( "They are already weakened.\n\r", ch );
    return;
  }

  if ( saves_spell( level, victim,DAM_OTHER) )
  {
    send_to_char("Weaken failed.\n\r",ch);

    if (!is_affected(victim, skill_lookup("sleep")))
      damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);

    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 2;
  af.location  = APPLY_STR;
  af.modifier  = -1 * (level / 5);
  af.bitvector = AFF_WEAKEN;
  affect_to_char( victim, &af );
  if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
  send_to_char( "You feel your strength slip away.\n\r", victim );
  act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
  return;
}



/* RT recall spell is back */

void spell_word_of_recall( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  ROOM_INDEX_DATA *location;
  int room = -1;

  /*  if (is_in_pk_range(ch,victim)) {
    room = get_recall_room(ch);
  }
  else { */
    room = get_recall_room(victim);
    /*  }*/

  if ( (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim) ) )
  {
      send_to_char("You desire to stay with your adoring master.\n\r", ch);
      send_to_char("is too strong for the magic work.\n\r", ch);
      return;
  }

  location = get_room_index ( get_recall_room( ch ) );
//  if (ch->in_room == location )
//    send_to_char("You are already at recall...\n\r", ch);
  
  if (!recall(victim,FALSE,room))
    return;
  act( "$n disappears.", victim, NULL, NULL, TO_WIZ_ROOM );
  if (victim->fighting)
    stop_fighting(victim,TRUE);

  location = get_room_index(room);
  move_to_room(victim,location);
  act( "$n appears in the room.", victim, NULL, NULL, TO_WIZ_ROOM );
  do_function(victim, &do_look,"auto");
 
  if (victim->pet && (victim->pet->position > POS_RESTING) )
    do_function(victim->pet, &do_recall,"");

/* rescue quest can follow through recall */
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
        do_function(rch, &do_recall,"");
        break;
      }
    }
  }
*/

}

/*
 * NPC spells.
 */
void spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,hp_dam,dice_dam,hpch;

  act("$n spits a stream of {ca{gc{Gi{cd{x at $N.",ch,NULL,victim,TO_NOTVICT);
  act("$n spits a stream of corrosive {ca{gc{Gi{cd{x at you.",ch,NULL,victim,TO_VICT);
  act("You spit a stream of {ca{gc{Gi{cd{x at $N.",ch,NULL,victim,TO_CHAR);

  hpch = UMAX(12,ch->hit);
  hp_dam = number_range(hpch/11 + 1, hpch/6);
  dice_dam = dice(level,16);

  dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    
  if (saves_spell(level,victim,DAM_ACID))
  {
      acid_effect(victim,level/2,dam/4,TARGET_CHAR);
      damage(ch,victim,dam/2,sn,DAM_ACID,TRUE, TRUE);
  }
  else
  {
      acid_effect(victim,level,dam,TARGET_CHAR);
      damage(ch,victim,dam,sn,DAM_ACID,TRUE, TRUE);
  }
}



void spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *vch, *vch_next;
  int dam,hp_dam,dice_dam;
  int hpch;

  if (ch == victim) {
    send_to_char("Now that is REALLY not SMART.\n\r",ch);
    return;
  }
  act("$n breathes forth a cone of {yf{Ri{rre{x.",ch,NULL,victim,TO_NOTVICT);
  act("$n breathes a cone of hot {yf{Ri{rre{x over you!",ch,NULL,victim,TO_VICT);
  act("You breathe forth a cone of {yf{Ri{rre{x.",ch,NULL,NULL,TO_CHAR);

  hpch = UMAX( 10, ch->hit );
  hp_dam  = number_range( hpch/9+1, hpch/5 );
  dice_dam = dice(level,20);

  dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
  fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);

  for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
  {
    vch_next = vch->next_in_room;

    if (is_safe_spell(ch,vch,TRUE, TRUE) 
      ||  (IS_WIZINVIS(vch,ch))
      ||  (IS_NPC(vch) && IS_NPC(ch)
      &&   (ch->fighting != vch || vch->fighting != ch)))
        continue;

    if (vch == victim) /* full damage */
      {
        if (saves_spell(level,vch,DAM_FIRE))
        {
          fire_effect(vch,level/2,dam/4,TARGET_CHAR);
          damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE, TRUE);
        }
        else
        {
          fire_effect(vch,level,dam,TARGET_CHAR);
          damage(ch,vch,dam,sn,DAM_FIRE,TRUE, TRUE);
        }
      }
    else /* partial damage */
      {
        if (saves_spell(level - 2,vch,DAM_FIRE))
        {
          fire_effect(vch,level/4,dam/8,TARGET_CHAR);
          damage(ch,vch,dam/4,sn,DAM_FIRE,TRUE, TRUE);
        }
        else
        {
          fire_effect(vch,level/2,dam/4,TARGET_CHAR);
          damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE, TRUE);
        }
      }
  }
}

void spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *vch, *vch_next;
  int dam,hp_dam,dice_dam, hpch;

  act("$n breathes out a freezing cone of {Wf{Cr{Bo{bst{x!",ch,NULL,victim,TO_NOTVICT);
  act("$n breathes a freezing cone of {Wf{Cr{Bo{bst{x over you!",
      ch,NULL,victim,TO_VICT);
  act("You breathe out a cone of {Wf{Cr{Bo{bst{x.",ch,NULL,NULL,TO_CHAR);

  hpch = UMAX(12,ch->hit);
  hp_dam = number_range(hpch/11 + 1, hpch/6);
  dice_dam = dice(level,16);

  dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
  cold_effect(victim->in_room,level,dam/2,TARGET_ROOM); 

  for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;

      if (is_safe_spell(ch,vch,TRUE, TRUE)
      ||  (IS_WIZINVIS(vch,ch))
      ||  (IS_NPC(vch) && IS_NPC(ch) 
           &&   (ch->fighting != vch || vch->fighting != ch)))
    continue;

      if (vch == victim) /* full damage */
    {
      if (saves_spell(level,vch,DAM_COLD))
        {
          cold_effect(vch,level/2,dam/4,TARGET_CHAR);
          damage(ch,vch,dam/2,sn,DAM_COLD,TRUE, TRUE);
        }
      else
        {
          cold_effect(vch,level,dam,TARGET_CHAR);
          damage(ch,vch,dam,sn,DAM_COLD,TRUE, TRUE);
        }
    }
      else
    {
      if (saves_spell(level - 2,vch,DAM_COLD))
        {
          cold_effect(vch,level/4,dam/8,TARGET_CHAR);
          damage(ch,vch,dam/4,sn,DAM_COLD,TRUE, TRUE);
        }
      else
        {
          cold_effect(vch,level/2,dam/4,TARGET_CHAR);
          damage(ch,vch,dam/2,sn,DAM_COLD,TRUE, TRUE);
        }
    }
    }
}

    
void spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam,hp_dam,dice_dam,hpch;

  act("$n breathes out a cloud of poisonous {gg{Ca{gs{x!",ch,NULL,NULL,TO_ROOM);
  act("You breathe out a cloud of poisonous {gg{Ca{gs{x.",ch,NULL,NULL,TO_CHAR);

  hpch = UMAX(16,ch->hit);
  hp_dam = number_range(hpch/15+1,8);
  dice_dam = dice(level,18);

  dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
  poison_effect(ch->in_room,level,dam,TARGET_ROOM);

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
  {
    vch_next = vch->next_in_room;

    if ( is_safe_spell(ch,vch,TRUE, TRUE)
    ||   ( IS_WIZINVIS(vch,ch))
    ||   ( IS_NPC(ch) && IS_NPC(vch) 
    &&   ( ch->fighting == vch || vch->fighting == ch)))
      continue;

    if (saves_spell(level,vch,DAM_POISON))
    {
      poison_effect(vch,level/2,dam/4,TARGET_CHAR);
      damage(ch,vch,dam/2,sn,DAM_POISON,TRUE, TRUE);
    }
    else
    {
      poison_effect(vch,level,dam,TARGET_CHAR);
      damage(ch,vch,dam,sn,DAM_POISON,TRUE, TRUE);
    }
  }
}

void spell_lightning_breath(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,hp_dam,dice_dam,hpch;

  act("$n breathes a bolt of {yl{Yi{Wgh{yt{wn{Yi{Wn{yg{x at $N.",ch,NULL,victim,TO_NOTVICT);
  act("$n breathes a bolt of {yl{Yi{Wgh{yt{wn{Yi{Wn{yg{x at you!",ch,NULL,victim,TO_VICT);
  act("You breathe a bolt of {yl{Yi{Wgh{yt{wn{Yi{Wn{yg{x at $N.",ch,NULL,victim,TO_CHAR);

  hpch = UMAX(10,ch->hit);
  hp_dam = number_range(hpch/9+1,hpch/5);
  dice_dam = dice(level,20);

  dam = UMAX( hp_dam+dice_dam/10, dice_dam+hp_dam/10 );

  if (saves_spell(level,victim,DAM_LIGHTNING))
    {
      shock_effect(victim,level/2,dam/4,TARGET_CHAR);
      damage(ch,victim,dam/2,sn,DAM_LIGHTNING,TRUE, TRUE);
    }
  else
    {
      shock_effect(victim,level,dam,TARGET_CHAR);
      damage(ch,victim,dam,sn,DAM_LIGHTNING,TRUE, TRUE); 
    }
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
 
  dam = number_range( 25, 100 );
  if ( saves_spell( level, victim, DAM_PIERCE) )
    dam /= 2;
  damage( ch, victim, dam, sn, DAM_PIERCE ,TRUE, TRUE);
  return;
}

void spell_high_explosive(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
 
  dam = number_range( 30, 120 );
  if ( saves_spell( level, victim, DAM_PIERCE) )
    dam /= 2;
  damage( ch, victim, dam, sn, DAM_PIERCE ,TRUE, TRUE);
  return;
}

bool magic_dispel(int level, CHAR_DATA *victim)
{
  bool found = FALSE;

  if (check_dispel(level,victim,skill_lookup("armor"))) {
    act("$n is no longer armored.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
 
  if (check_dispel(level,victim,skill_lookup("bless"))) {
    found = TRUE;
    act("$n has lost the favor of $s god.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("mass bless"))) {
    found = TRUE;
    act("$n has lost the favor of $s god.",victim,NULL,NULL,TO_ROOM);
  } 

  if (check_dispel(level,victim,skill_lookup("blindness")))
    {
      found = TRUE;
      act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
  
  if (check_dispel(level,victim,skill_lookup("calm")))
    {
      found = TRUE;
      act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
  if (check_dispel(level,victim,skill_lookup("change sex")))
    {
      found = TRUE;
      act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level, victim, skill_lookup("fish breath")))
    {
      found = TRUE;
      act("$n can no longer breathe {Bunder{cwater{x.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel(level, victim, skill_lookup("clear head")))
    {
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("charm person")))
    {
      found = TRUE;
      if (victim->leader)
    /* victim->leader = NULL; */ /* Removes victim from any group (Sartan) */
        stop_follower(victim);
      act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
  if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
      found = TRUE;
      act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
  if (check_dispel(level,victim,skill_lookup("curse"))) {
    act("$n is no longer cursed.", victim, NULL, NULL, TO_ROOM);
    found = TRUE;
  }
  if (check_dispel(level,victim,skill_lookup("detect evil"))) 
    {
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("detect good")))
    {
      found = TRUE;
    }
 
  if (check_dispel(level,victim,skill_lookup("detect hidden")))
    {
      found = TRUE;
    }
 
  if (check_dispel(level,victim,skill_lookup("detect invis")))
    {
      found = TRUE;
    }
 
  if (check_dispel(level,victim,skill_lookup("detect magic")))
    {
      found = TRUE;
    }
 
  if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
      act("$n's {mpink{x outline fades.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }
 
  if (check_dispel(level,victim,skill_lookup("fly")))
    {
      act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
      act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
      found = TRUE;
    }
 
  if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
      act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("haste")))
    {
      act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }
 
  if (check_dispel(level,victim,skill_lookup("infravision")))
    {
      found = TRUE;
    }
 
  if (check_dispel(level,victim,skill_lookup("invis")))
    {
      act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }
 
  if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
      act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }
 
  if (check_dispel(level,victim,skill_lookup("pass door")))
    {
      found = TRUE;
      act("$n is no longer transparent.",victim,NULL,NULL,TO_ROOM);
    }

  if (check_dispel(level, victim, skill_lookup("poison aura")))
    {
      found = TRUE;
      act("$n's aura of health brightens and disappears.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel(level,victim,skill_lookup("protection evil")))
    {
      act("$n looks less protected from evil.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("protection good")))
    {
      act("$n looks less protected from good.",victim,NULL,NULL,TO_ROOM);
      found = TRUE; 
    }

  if (check_dispel(level, victim, skill_lookup("protection holy")))
    {
      act("$n loses his {rdem{Do{rn{Di{rc{x energy.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level, victim, skill_lookup("protection negative")))
    {
      act("$n loses his {yd{Wi{yv{Wi{yne{x energy.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("protection neutral")))
    {
      act("$n looks less protected from neutrality.",victim,NULL,NULL,TO_ROOM);
      found = TRUE; 
    }
 
  if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
      act("The {Wwhite{x aura around $n's body vanishes.", victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("aqua albedo")))
    {
      act("The {Wshimmering{x aura around $n's body vanishes.", victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("radiance")))
    {
      act("The {Wradian{cce{x around $n fades away.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("malevolent shroud")))
    {
      act("The wispy {rs{Dhroud{x around $n dissipates.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level, victim, skill_lookup("regeneration")))
    {
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("shield")))
    {
      act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }
 
  if (check_dispel(level,victim,skill_lookup("sleep")))
    {
      found = TRUE;
      act("$n awakes from a deep sleep.",victim,NULL,NULL,TO_ROOM);
    }

  if (check_dispel(level,victim,skill_lookup("slow")))
    {
      act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }
  if (check_dispel(level,victim,skill_lookup("entangle")))
    {
      act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }
 
  if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
      act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }


 if (check_dispel(level, victim, skill_lookup("vaccine")))
    {
      found = TRUE;
    }

  if (check_dispel(level, victim, skill_lookup("water walk")))
    {
      found = TRUE;
    }

 if (check_dispel(level,victim,skill_lookup("weaken")))
    {
      act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }
 
  if (check_dispel(level,victim,skill_lookup("moon armor")))
    {
      act("$n is no longer in {cm{Do{bo{cn{Wlight{x.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }
 
  if (check_dispel(level,victim,skill_lookup("globe of invulnerability")))
    {
      act("$n loses $s magical {Bgl{mobe{x.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("fade out") ))
  {
      act("$n has lost the ability to fade from attacks.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  }
  
  if (check_dispel(level,victim,skill_lookup("nirvana")))
  {
    act("$n is no longer in {gnirv{cana{x.",
        victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }

  if ( check_dispel( level, victim, skill_lookup( "noble truth" ) ) )
  {
    act( "$n's {Yno{yb{Wl{we{x spirit slips away",
        victim, NULL, NULL, TO_ROOM );
    found = TRUE;
  }

  if (check_dispel(level,victim,skill_lookup("dislocation")))
  {
      act("$n appears more in this world.",
        victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  }

  if (check_dispel(level,victim,skill_lookup("eagle spirit")))
  {
      act("$n loses $s {wea{Wg{Dle{x look.",
        victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  }

  if (check_dispel(level,victim,skill_lookup("bear spirit")))
  {
      act("$n loses $s {Dbe{wa{Dr{x look.",
        victim,NULL,NULL,TO_ROOM);
      found = TRUE;
  }
  if (check_dispel(level,victim,skill_lookup("dragon spirit")))
    {
      act("$n loses $s {gdrag{con{x look.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }
  if (check_dispel(level,victim,skill_lookup("tiger spirit")))
    {
      act("$n loses $s {yt{Di{yg{De{yr{x look.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }
  if (check_dispel(level,victim,skill_lookup("windtomb")))
    {
      act("The wind dies down around $n.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }
  if (check_dispel(level,victim,skill_lookup("farsight")))
    {
      found = TRUE;
    }
  if (check_dispel(level,victim,skill_lookup("jump")))
    {
      found = TRUE;
    }
  if (check_dispel(level,victim,skill_lookup("mind shield")))
    {
      found = TRUE;
    }
  if (check_dispel(level,victim,skill_lookup("aid")))
    {
      found = TRUE;
    }
  if (check_dispel(level,victim,skill_lookup("corrosive aura")))
    {
      found = TRUE;
      act("$n's {cco{gr{Cr{cos{Gi{cve{x aura vanishes.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("flame aura")))
    {
      found = TRUE;
      act("$n's aura of {rf{yl{Ra{rme{x blinks out of existence.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("frost aura")))
    {
      found = TRUE;
      act("$n's aura of {Wf{Cr{Bo{bst{x fades away.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("electric aura")))
    {
      found = TRUE;
      act("$n's aura of {ye{Wl{ye{Yc{Wt{yr{Wi{yc{Yi{Wt{yy{x dissipates.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("arcane aura")))
    {
      found = TRUE;
      act("$n's aura of the {Bma{mg{Bi{bc{x collapses.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("holy aura")))
    {
      found = TRUE;
      act("$n's aura of {yh{Wo{yl{Wi{yness{x darkens and disappears.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("dark aura")))
    {
      found = TRUE;
      act("$n's aura of {rda{Dr{rk{Dn{ress{x brightens and disappears.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("deter")))
    {
      found = TRUE;
      act("$n's aura of power flashes away into nothingness.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("smoke screen")))
    {
      found = TRUE;
      act("The {Dh{Wa{wz{De{x surrounding $n has dissipated.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("petrify")))
    {
      found = TRUE;
      act("$n seems a bit more confident.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("mageshield")))
    {
      found = TRUE;
      act("$n's {bm{Bag{bic{x shield cracks and shatters.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("warriorshield")))
    {
      found = TRUE;
      act("$n's {yba{Yttl{ye{x shield cracks and shatters.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("darksight")))
    {
      found = TRUE;
    }
  if (check_dispel(level,victim,skill_lookup("yawn")))
    {
      found = TRUE;
      act("$n's yawning has finally stopped.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("hiccup")))
    {
      found = TRUE;
      act("$n's hiccupping has finally stopped.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("air element")))
    {
      found = TRUE;
      act("The element of {Wai{cr{x stops flowing into $n.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("earth element")))
    {
      found = TRUE;
      act("The element of {Dear{yth{x stops flowing into $n.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("fire element")))
    {
      found = TRUE;
      act("The element of {Rfi{yre{x stops flowing into $n.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("water element")))
    {
      found = TRUE;
      act("The element of {Bwa{cter{x stops flowing into $n.",victim,NULL,NULL,TO_ROOM);
    }
  if (check_dispel(level,victim,skill_lookup("bark skin")))
    {
      act("$n loses $s {ybark{w-{glike{x appearance.",victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }
  if (check_dispel(level,victim,skill_lookup("clairvoyance")))
    {
      found = TRUE;
    }
  if (check_dispel(level,victim,skill_lookup("strengthen")))
  {
    act("$n lost a bit of strength.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
  if (check_dispel(level,victim,skill_lookup("nimbleness")))
  {
        act("$n doesn't look so agile.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
  }
  if ( check_dispel( level, victim, skill_lookup( "bone armor" ) ) )
  {
    act("$n loses their armor of bone.", victim, NULL, NULL, TO_ROOM );
    found = TRUE;
  }
  if ( check_dispel( level, victim, skill_lookup( "nightmare" ) ) )
  {
    act( "$n looks less afraid of his dreams.", victim, NULL, NULL, TO_ROOM );
    found = TRUE;
  }

  if (check_dispel(level,victim,skill_lookup("aqua regia")))
    {
      act("You lose your {cac{gi{cd{Ci{cc{x shroud.", victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("aqua fortis")))
    {
      act("Your liquid shield falls with a splash.", victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("aqua landhi")))
    {
      act("You can no longer breathe water.", victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("aqua citrinitas")))
    {
      act("Your thoughts are no longer so focused.", victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("aqua rubedo")))
    {
      act("You have lost your connection to the gods.", victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("warcry rage")))
    {
      act("You no longer feel your god's rage.", victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("warcry vigor")))
    {
      act("You no longer feel as agile.", victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("warcry guarding")))
    {
      act("You no longer feel guarded.", victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  if (check_dispel(level,victim,skill_lookup("warcry hardening")))
    {
      act("Your body loses its hard edge.", victim,NULL,NULL,TO_ROOM);
      found = TRUE;
    }

  return (found);
}

void cast_a_spell(int sn, int lvl, CHAR_DATA *ch, void *vo, int target, int object)
{
  int i=1;
  int x=0;
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *ch_orig_room=NULL, *victim_orig_room=NULL;

  if (ch->in_room == NULL) 
  {
    bugf("ERROR: cast_a_spell: character not in a room.\n\r");
    return;
  }
    
  if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE)
  ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF  ) )
  {
    if (get_skill(ch,gsn_second_cast) > number_percent()) 
    {
      if (number_percent() <= 40)
        i++;
      check_improve(ch,gsn_second_cast,TRUE,1);
    }
    if (get_skill(ch,gsn_third_cast) > number_percent()) 
    {
      if (number_percent() <= 20)
        i++;
      if (get_skill(ch,gsn_second_cast) > 89)
        check_improve(ch,gsn_third_cast,TRUE,3);
    }
  }
  if ( object )
    i = 1;
  for ( x = 0; x < i; x++ ) 
  {
    if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE)
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF  ) )
    {
      victim = (CHAR_DATA *)vo;
      if (x == 0)
      {
        ch_orig_room = ch->in_room;
        victim_orig_room = victim->in_room;
      }

      if (victim->in_room != ch->in_room || victim->position == POS_DEAD
      || ch->in_room != ch_orig_room || victim->in_room != victim_orig_room)
        return;
    }
    /* check to see if deaths are done between rounds */
    ( *skill_table[sn].spell_fun)( sn, lvl, ch, vo, target );
  }

}


struct spell_power_table_st spell_power_table[] =
{
  {"magic missile",             1,    7},
  {"chill touch",               2,    5},
  {"burning hands",             3,    4},
  {"shocking grasp",            4,    4},
  {"lightning bolt",            4,    6},
  {"colour spray",              5,    6},
  {"cone of cold",             -1,    7},
  {"fireball",                  7,    8},
  {"sonic blast",               8,    9},
  {"icicle",                    9,   12},
  {"acid blast",               -1,   12},
  {"incinerate",               25,   15},
  {"flash",                     2,    4},
  {"banshee scream",            7,    7},
  {"chain lightning",          -1,    6},
  {"ionwave",                  -1,   13},
  {"bigby bash",                8,    9},
  {"holy bolt",                 1,    7},
  {"cause light",               2,    7},
  {"cause serious",             3,    8},
  {"cause critical",            4,    8},
  {"insect swarm",              4,    5},
  {"call lightning",           -1,    5},
  {"flamestrike",              -1,    8},
  {"sunbeam",                  -1,    6},
  {"demonfire",                -1,   11},
  {"hammer of thor",            6,    6},
  {"earthquake",               -1,    5},
  {"thunder",                  -1,    5},
  {"tornado",                  10,   10},
  {"divine intervention",      -1,    3},
  {"demonic intervention",     -1,    3},
  {"force of faith",           -1,    5},
  {"darkflow",                 -1,    5},
  {"dark fire",                 1,    7},
  {"turmoil",                  -1,   10},
  {"skeletal spike",           -1,   15},
  {"poisonous dart",            4,    5},
  {"ghostly wail",             60,   20},
  {"acidic gas",                1,    7},
  {"sulfur blast",             -1,   13},
  {"hyracal pressure",          5,    6},
  {"thorn blast",               4,    9}, // no idea wtf this is -- Taeloch
  {"demonic screech",          -1,   15},
  {NULL,                        0,    0}
};

/* ================================ */
/* Centralize routine for magical damage */
int magic_dam(int sn,int level)
{
  /* REVERSE THIS LOOP TO SAVE TIME */

  int dam =0,i,num=0;

  for (i=0; spell_power_table[i].name != NULL; i++)
  {
    if (!str_cmp(skill_table[sn].name, spell_power_table[i].name))
    {
      if (spell_power_table[i].number == -1)
      {
        num = level;
        dam = dice(num, spell_power_table[i].size);

        return(dam);
      }
      else
      {
        num = spell_power_table[i].number;
        dam = dice(num, spell_power_table[i].size);

        return(dam*2);
      }
    }
  }
  
  bugf("Error..  magic_dam could not find spell to set damage to -%s-",skill_table[sn].name);
  return (0);
}    
