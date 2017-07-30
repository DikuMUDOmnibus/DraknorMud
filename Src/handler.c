/**************************************************************************r
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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include <stdarg.h>
#include "magic.h"
#include "recycle.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN( do_return );

/*
 * Local functions.
 */
void affect_modify    args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
void show_file_to_char_bw(  CHAR_DATA *ch, char *filename );
bool rm_line_from_file(     CHAR_DATA *ch, char *filename, char *argument );
void search_line_from_file( CHAR_DATA *ch, char *filename, char *argument );


/* friend stuff -- for NPC's mostly */
bool is_friend( CHAR_DATA *ch, CHAR_DATA *victim )
{
  if ( is_same_group( ch, victim ) )
    return TRUE;

  if ( !IS_NPC( ch ) )
    return FALSE;

  /*
    if ( !IS_NPC( victim ) )
      return ( IS_SET( ch->off_flags, ASSIST_PLAYERS ) );

  Rewritten because that should always return false:
  */
  if ( !IS_NPC( victim ) )
    return FALSE;

  if ( IS_AFFECTED( ch, AFF_CHARM ) )
    return FALSE;

  if ( IS_SET( ch->off_flags, ASSIST_ALL ) )
    return TRUE;

  if ( ch->group && ch->group == victim->group )
    return TRUE;

  if ( IS_SET( ch->off_flags, ASSIST_VNUM )
       &&   ch->pIndexData == victim->pIndexData )
    return TRUE;

  if ( IS_SET( ch->off_flags, ASSIST_RACE )
       &&   ch->race == victim->race )
    return TRUE;

  if ( IS_SET( ch->off_flags, ASSIST_ALIGN )
       &&  !IS_SET( ch->act, ACT_NOALIGN ) && !IS_SET( victim->act, ACT_NOALIGN )
       &&  ( ( IS_GOOD( ch )     && IS_GOOD( victim ) )
             ||    ( IS_EVIL( ch )     && IS_EVIL( victim ) )
             ||    ( IS_NEUTRAL( ch )  && IS_NEUTRAL( victim ) ) ) )
    return TRUE;

  return FALSE;
}

/* returns number of people on an object */
int count_users( OBJ_DATA *obj )
{
  CHAR_DATA *fch;
  int count = 0;

  if ( obj->in_room == NULL )
    return 0;

  for ( fch = obj->in_room->people; fch; fch = fch->next_in_room )
    if ( fch->on == obj )
      count++;

  return count;
}

/* returns material number */
/*int material_lookup (const char *name)
{
    return 0;
}
*/
int weapon_lookup ( const char *name )
{
  int type;

  for ( type = 0; weapon_table[type].name; type++ )
  {
    if ( LOWER(name[0]) == LOWER( weapon_table[type].name[0] )
         &&  !str_prefix( name,weapon_table[type].name ) )
      return type;
  }
  return -1;
}

int weapon_type( const char *name )
{
  int type;

  for ( type = 0; weapon_table[type].name; type++)
  {
    if ( LOWER( name[0] ) == LOWER( weapon_table[type].name[0] )
         &&  !str_prefix( name, weapon_table[type].name ) )
      return weapon_table[type].type;
  }
  return WEAPON_EXOTIC;
}


char *item_name( int item_type )
{
  int type;

  for ( type = 0; item_table[type].name; type++ )
    if ( item_type == item_table[type].type )
      return item_table[type].name;
  return "none";
}

char *weapon_name( int weapon_type )
{
  int type;

  for ( type = 0; weapon_table[type].name; type++ )
    if ( weapon_type == weapon_table[type].type )
      return weapon_table[type].name;
  return "exotic";
}

int attack_lookup( const char *name )
{
  int att;

  for ( att = 0; attack_table[att].name; att++ )
  {
    if ( LOWER( name[0] ) == LOWER( attack_table[att].name[0] )
         &&  !str_prefix( name, attack_table[att].name ) )
      return att;
  }
  return 0;
}

/* return attack name */
char *get_attack_name( int dam_msg )
{
  if ( dam_msg < 0 || dam_msg >= MAX_DAMAGE_MESSAGE )
  {
    bugf( "Get_attack_name: bad dam_type %d", dam_msg );
    return "unknown";
  }
  return attack_table[dam_msg].name;
}

/* return attack noun */
char *get_attack_noun( int dam_msg )
{
  if ( dam_msg < 0 || dam_msg >= MAX_DAMAGE_MESSAGE )
  {
    bugf( "Get_attack_noun: bad dam_msg %d", dam_msg );
    return "unknown";
  }
  return attack_table[dam_msg].noun;
}

/* returns a flag for wiznet */
long wiznet_lookup ( const char *name )
{
  int flag;

  for ( flag = 0; wiznet_table[flag].name; flag++ )
  {
    if ( LOWER( name[0] ) == LOWER( wiznet_table[flag].name[0] )
         && !str_prefix( name,wiznet_table[flag].name ) )
      return flag;
  }
  return -1;
}

/* returns class number */
int class_lookup ( const char *name )
{
  int gameclass;

  for ( gameclass = 0; gameclass < MAX_CLASS; gameclass++ )
  {
    if ( LOWER( name[0]) == LOWER( class_table[gameclass].name[0] )
         &&  !str_prefix( name,class_table[gameclass].name ) )
      return gameclass;
  }
  return -1;
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */
int check_immune(CHAR_DATA *ch, int dam_type)
{
  int immune, def;
  int bit;

  immune = -1;
  def = IS_NORMAL;

  if (dam_type == DAM_NONE)
    return immune;

  if (dam_type <= 3)
  {
    if (IS_SET(ch->imm_flags,IMM_WEAPON))
      def = IS_IMMUNE;
    else if (IS_SET(ch->res_flags,RES_WEAPON))
      def = IS_RESISTANT;
    else if (IS_SET(ch->vuln_flags,VULN_WEAPON))
      def = IS_VULNERABLE;
  }
  else /* magical attack */
  {
    if (IS_SET(ch->imm_flags,IMM_MAGIC))
      def = IS_IMMUNE;
    else if (IS_SET(ch->res_flags,RES_MAGIC))
      def = IS_RESISTANT;
    else if (IS_SET(ch->vuln_flags,VULN_MAGIC))
      def = IS_VULNERABLE;
  }

  /* set bits to check -- VULN etc. must ALL be the same or this will fail */
  switch (dam_type)
  {
    case(DAM_BASH):        bit = IMM_BASH;        break;
    case(DAM_PIERCE):    bit = IMM_PIERCE;    break;
    case(DAM_SLASH):    bit = IMM_SLASH;    break;
    case(DAM_FIRE):        bit = IMM_FIRE;        break;
    case(DAM_COLD):        bit = IMM_COLD;        break;
    case(DAM_LIGHTNING):    bit = IMM_LIGHTNING;    break;
    case(DAM_ACID):        bit = IMM_ACID;        break;
    case(DAM_POISON):    bit = IMM_POISON;    break;
    case(DAM_NEGATIVE):    bit = IMM_NEGATIVE;    break;
    case(DAM_HOLY):        bit = IMM_HOLY;        break;
    case(DAM_ENERGY):    bit = IMM_ENERGY;    break;
    case(DAM_MENTAL):    bit = IMM_MENTAL;    break;
    case(DAM_DISEASE):    bit = IMM_DISEASE;    break;
    case(DAM_DROWNING):    bit = IMM_DROWNING;    break;
    case(DAM_LIGHT):    bit = IMM_LIGHT;    break;
    case(DAM_CHARM):    bit = IMM_CHARM;    break;
    case(DAM_SOUND):    bit = IMM_SOUND;    break;
    case(DAM_WIND):        bit = IMM_WIND;        break;
    case(DAM_SHOCK):    bit = IMM_SHOCK;    break;
    default:        return def;
  }

  if (IS_SET(ch->imm_flags,bit))
    immune = IS_IMMUNE;
  else if (IS_SET(ch->res_flags,bit) && immune != IS_IMMUNE)
    immune = IS_RESISTANT;
  else if (IS_SET(ch->vuln_flags,bit))
  {
    if (immune == IS_IMMUNE)
      immune = IS_RESISTANT;
    else if (immune == IS_RESISTANT)
      immune = IS_NORMAL;
    else
      immune = IS_VULNERABLE;
  }

  if (immune == -1)
    return def;
  else
    return immune;
}
/*
bool is_clan(CHAR_DATA *ch)
{
    return ch->clan;
}
*/
/*
bool is_same_clan(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (clan_table[ch->clan].independent)
    return FALSE;
    else
    return (ch->clan == victim->clan);
}
*/
/* checks mob format */
bool is_old_mob( CHAR_DATA *ch )
{
  if ( ch->pIndexData == NULL )
    return FALSE;
  else if ( ch->pIndexData->new_format )
    return FALSE;
  return TRUE;
}

/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
{
  int skill;

  if (sn == -1) /* shorthand for level based skills */
  {
    skill = ch->level * 5 / 2;
  }

  else if (sn < -1 || sn > MAX_SKILL)
  {
    bug("Bad sn %d in get_skill.",sn);
    skill = 0;
  }

  else if (!IS_NPC(ch))
  {
    if ( (ch->level < skill_table[sn].skill_level[ch->gameclass])&& !is_racial_skill(ch, sn) )
      skill = 0;
    else
      skill = ch->pcdata->learned[sn];
  }

  else /* mobiles */
  {


    if (skill_table[sn].spell_fun != spell_null)
      skill = 40 + 2 * ch->level;

    else if (sn == gsn_sneak || sn == gsn_hide || sn == gsn_chameleon)
      skill = ch->level * 2 + 20;

    else if ((sn == gsn_dodge && IS_SET(ch->off_flags,OFF_DODGE))
             ||       (sn == gsn_parry && IS_SET(ch->off_flags,OFF_PARRY)))
      skill = ch->level * 2;

    else if (sn == gsn_shield_block)
      skill = 10 + 2 * ch->level;

    else if (sn == gsn_second_attack
             && (IS_SET(ch->act,ACT_WARRIOR) || IS_SET(ch->act,ACT_THIEF)))
      skill = 10 + 3 * ch->level;

    else if (sn == gsn_third_attack && IS_SET(ch->act,ACT_WARRIOR))
      skill = 4 * ch->level - 40;

    else if (sn == gsn_fourth_attack && IS_SET(ch->act,ACT_WARRIOR))
      skill = 3 * ch->level - 40;

    else if (sn == gsn_hand_to_hand)
      skill = 40 + 2 * ch->level;

    else if (sn == gsn_karate)
      skill = 45 + 2 * ch->level;

    else if (sn == gsn_trip && IS_SET(ch->off_flags,OFF_TRIP))
      skill = 10 + 3 * ch->level;

    else if (sn == gsn_bash && IS_SET(ch->off_flags,OFF_BASH))
      skill = 10 + 3 * ch->level;

    else if ((sn == gsn_disarm || sn== gsn_shatter)
             &&  (IS_SET(ch->off_flags,OFF_DISARM)
                  ||   IS_SET(ch->act,ACT_WARRIOR)
                  ||      IS_SET(ch->act,ACT_THIEF)))
      skill = 20 + 3 * ch->level;

    else if (sn == gsn_berserk && IS_SET(ch->off_flags,OFF_BERSERK))
      skill = 3 * ch->level;

    else if (sn == gsn_kick)
      skill = 10 + 3 * ch->level;

    else if (sn == gsn_backstab && IS_SET(ch->act,ACT_THIEF))
      skill = 20 + 2 * ch->level;

    else if (sn == gsn_flying)
      skill = 15 + 3 * ch->level;

    else if (sn == gsn_rescue)
      skill = 40 + ch->level;

    else if (sn == gsn_recall)
      skill = 40 + ch->level;

    else if (sn == gsn_sword
             ||  sn == gsn_dagger
             ||  sn == gsn_spear
             ||  sn == gsn_mace
             ||  sn == gsn_axe
             ||  sn == gsn_flail
             ||  sn == gsn_whip
             ||  sn == gsn_polearm
						 ||  sn == gsn_crossbow)
      skill = 40 + 5 * ch->level / 2;

    else
      skill = 0;
  }

  if (ch->daze > 0)
  {
    if (skill_table[sn].spell_fun != spell_null)
      /*skill /= 2;*/
      skill /= 4;    /* Modified by Sartan */
    else
      /*skill = 2 * skill / 3;*/
      skill /= 3;    /* Modified by Sartan */
  }

  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
    skill = 9 * skill / 10;

  return URANGE(0,skill,100);
}

/* for returning weapon information */
int get_weapon_sn( CHAR_DATA *ch )
{
  OBJ_DATA *wield;
  int sn;

  wield = get_eq_char( ch, WEAR_WIELD );
  if ( wield == NULL || wield->item_type != ITEM_WEAPON )
  {
    if ( get_skill( ch, gsn_karate ) > 1 )
      sn = gsn_karate;
    else
      sn = gsn_hand_to_hand;
  }
  else switch (wield->value[0])
    {
      default :               sn = -1;                break;
      case(WEAPON_EXOTIC):    sn = gsn_exotic;        break;
      case(WEAPON_SWORD):     sn = gsn_sword;         break;
      case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
      case(WEAPON_SPEAR):     sn = gsn_spear;         break;
      case(WEAPON_MACE):      sn = gsn_mace;          break;
      case(WEAPON_AXE):       sn = gsn_axe;           break;
      case(WEAPON_FLAIL):     sn = gsn_flail;         break;
      case(WEAPON_WHIP):      sn = gsn_whip;          break;
      case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
      case(WEAPON_CROSSBOW):  sn = gsn_crossbow;      break;
    }
  return sn;
}
/* for returning weapon information */
int get_weapon_sn_2( CHAR_DATA *ch )
{
  OBJ_DATA *wield;
  int sn;

  wield = get_eq_char( ch, WEAR_SECONDARY );
  if ( wield == NULL || wield->item_type != ITEM_WEAPON )
  {
    if ( get_skill( ch, gsn_karate ) > 1 )
      sn = gsn_karate;
    else
      sn = gsn_hand_to_hand;
  }
  else
  {
    switch ( wield->value[0] )
    {
      default :                 sn = -1;                break;
      case( WEAPON_EXOTIC ):    sn = gsn_exotic;        break;
      case( WEAPON_SWORD ):     sn = gsn_sword;         break;
      case( WEAPON_DAGGER ):    sn = gsn_dagger;        break;
      case( WEAPON_SPEAR ):     sn = gsn_spear;         break;
      case( WEAPON_MACE ):      sn = gsn_mace;          break;
      case( WEAPON_AXE ):       sn = gsn_axe;           break;
      case( WEAPON_FLAIL ):     sn = gsn_flail;         break;
      case( WEAPON_WHIP ):      sn = gsn_whip;          break;
      case( WEAPON_POLEARM ):   sn = gsn_polearm;       break;
			case( WEAPON_CROSSBOW ):  sn = gsn_crossbow;      break;
    }
  }
  return sn;
}

int get_weapon_skill( CHAR_DATA *ch, int sn )
{
  int skill;

  /* -1 is exotic */
  if ( IS_NPC( ch ) )
  {
    if ( sn == -1 )
      skill = 3 * ch->level;
    else if ( sn == gsn_hand_to_hand )
      skill = 40 + 2 * ch->level;
    else if ( sn == gsn_karate )
      skill = 45 + 2 * ch->level;
    else
      skill = 40 + 5 * ch->level / 2;
  }
  else
  {
    if ( sn == -1 )
      skill = 3 * ch->level;
    else
      skill = ch->pcdata->learned[sn];
  }
  return URANGE( 0, skill, 100 );
}


/* used to de-screw characters */
void reset_char( CHAR_DATA *ch )
{
  int loc, mod, stat;
  OBJ_DATA      *obj;
  AFFECT_DATA   *af;
  int i;

  if ( IS_NPC( ch ) )
    return;

  if ( ch->pcdata->perm_hit == 0
       ||   ch->pcdata->perm_mana == 0
       ||   ch->pcdata->perm_move == 0
       ||   ch->pcdata->last_level == 0)
  {
    /* do a FULL reset */
    for ( loc = 0; loc < MAX_WEAR; loc++ )
    {
      obj = get_eq_char( ch, loc );
      if ( obj == NULL )
        continue;
      if ( !obj->enchanted )
        for ( af = obj->pIndexData->affected; af; af = af->next )
        {
          mod = af->modifier;
          switch (af->location)
          {
            case APPLY_SEX:    ch->sex                                -= mod;
              if ( ch->sex < 0 || ch->sex > 2 )
                ch->sex = IS_NPC( ch ) ? 0 : ch->pcdata->true_sex;
              break;
            case APPLY_MANA:    ch->max_mana                    -= mod;    break;
            case APPLY_HIT:        ch->max_hit                        -= mod;    break;
            case APPLY_MOVE:    ch->max_move                    -= mod;    break;
          }
        }

      for ( af = obj->affected; af; af = af->next )
      {
        mod = af->modifier;
        switch ( af->location )
        {
          case APPLY_SEX:       ch->sex                         -= mod; break;
          case APPLY_MANA:      ch->max_mana                    -= mod; break;
          case APPLY_HIT:       ch->max_hit                     -= mod; break;
          case APPLY_MOVE:      ch->max_move                    -= mod; break;
        }
      }
    }
    /* now reset the permanent stats */
    ch->pcdata->perm_hit     = ch->max_hit;
    ch->pcdata->perm_mana     = ch->max_mana;
    ch->pcdata->perm_move    = ch->max_move;
    ch->pcdata->last_level    = ch->played / 3600;
    if ( ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2 )
    {
      if ( ch->sex > 0 && ch->sex < 3 )
        ch->pcdata->true_sex    = ch->sex;
      else
        ch->pcdata->true_sex     = 0;
    }
  }

  /* now restore the character to his/her true condition */
  for ( stat = 0; stat < MAX_STATS; stat++ )
    ch->mod_stat[stat] = 0;

  if ( ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2 )
    ch->pcdata->true_sex = 0;

  ch->sex        = ch->pcdata->true_sex;
  ch->max_hit     = ch->pcdata->perm_hit;
  ch->max_mana    = ch->pcdata->perm_mana;
  ch->max_move    = ch->pcdata->perm_move;

  for ( i = 0; i < 4; i++ )
    ch->armor[i]    = 100;

  ch->hitroll        = 0;
  ch->damroll        = 0;
  ch->saving_throw    = 0;

  /* now start adding back the effects */
  for ( loc = 0; loc < MAX_WEAR; loc++ )
  {
    obj = get_eq_char( ch, loc );
    if ( obj == NULL )
      continue;
    for ( i = 0; i < 4; i++ )
      ch->armor[i] -= apply_ac( obj, loc, i );

    if ( !obj->enchanted )
      for ( af = obj->pIndexData->affected; af; af = af->next )
      {
        mod = af->modifier;
        switch ( af->location )
        {
          case APPLY_STR:            ch->mod_stat[STAT_STR]        += mod;    break;
          case APPLY_DEX:            ch->mod_stat[STAT_DEX]        += mod; break;
          case APPLY_INT:            ch->mod_stat[STAT_INT]        += mod; break;
          case APPLY_WIS:            ch->mod_stat[STAT_WIS]        += mod; break;
          case APPLY_CON:            ch->mod_stat[STAT_CON]        += mod; break;
          case APPLY_SEX:            ch->sex                        += mod; break;
          case APPLY_MANA:            ch->max_mana                += mod; break;
          case APPLY_HIT:            ch->max_hit                    += mod; break;
          case APPLY_MOVE:            ch->max_move                += mod; break;
          case APPLY_AC:          for (i = 0; i < 4; i ++)
              ch->armor[i]                += mod; break;
          case APPLY_HITROLL:        ch->hitroll                    += mod; break;
          case APPLY_DAMROLL:        ch->damroll                    += mod; break;
          case APPLY_SAVES:            ch->saving_throw            += mod; break;
          case APPLY_SAVING_ROD:     ch->saving_throw            += mod; break;
          case APPLY_SAVING_PETRI:    ch->saving_throw            += mod; break;
          case APPLY_SAVING_BREATH: ch->saving_throw            += mod; break;
          case APPLY_SAVING_SPELL:    ch->saving_throw            += mod; break;
        }
      }

    for ( af = obj->affected; af; af = af->next )
    {
      mod = af->modifier;
      switch (af->location)
      {
        case APPLY_STR:             ch->mod_stat[STAT_STR]      += mod; break;
        case APPLY_DEX:             ch->mod_stat[STAT_DEX]      += mod; break;
        case APPLY_INT:             ch->mod_stat[STAT_INT]      += mod; break;
        case APPLY_WIS:             ch->mod_stat[STAT_WIS]      += mod; break;
        case APPLY_CON:             ch->mod_stat[STAT_CON]      += mod; break;
        case APPLY_SEX:             ch->sex                     += mod; break;
        case APPLY_MANA:            ch->max_mana                += mod; break;
        case APPLY_HIT:             ch->max_hit                 += mod; break;
        case APPLY_MOVE:            ch->max_move                += mod; break;
        case APPLY_AC:            for (i = 0; i < 4; i ++)
            ch->armor[i]                += mod; break;
        case APPLY_HITROLL:         ch->hitroll                 += mod; break;
        case APPLY_DAMROLL:         ch->damroll                 += mod; break;
        case APPLY_SAVES:           ch->saving_throw            += mod; break;
        case APPLY_SAVING_ROD:      ch->saving_throw            += mod; break;
        case APPLY_SAVING_PETRI:    ch->saving_throw            += mod; break;
        case APPLY_SAVING_BREATH:   ch->saving_throw            += mod; break;
        case APPLY_SAVING_SPELL:    ch->saving_throw            += mod; break;
      }
    }
  }

  /* now add back spell effects */
  for ( af = ch->affected; af; af = af->next )
  {
    mod = af->modifier;
    switch (af->location)
    {
      case APPLY_STR:           ch->mod_stat[STAT_STR]      += mod; break;
      case APPLY_DEX:           ch->mod_stat[STAT_DEX]      += mod; break;
      case APPLY_INT:           ch->mod_stat[STAT_INT]      += mod; break;
      case APPLY_WIS:           ch->mod_stat[STAT_WIS]      += mod; break;
      case APPLY_CON:           ch->mod_stat[STAT_CON]      += mod; break;
      case APPLY_SEX:           ch->sex                     += mod; break;
      case APPLY_MANA:          ch->max_mana                += mod; break;
      case APPLY_HIT:           ch->max_hit                 += mod; break;
      case APPLY_MOVE:          ch->max_move                += mod; break;
      case APPLY_AC:         for ( i = 0; i < 4; i ++ )
          ch->armor[i]                += mod; break;
      case APPLY_HITROLL:       ch->hitroll                 += mod; break;
      case APPLY_DAMROLL:       ch->damroll                 += mod; break;
      case APPLY_SAVES:         ch->saving_throw            += mod; break;
      case APPLY_SAVING_ROD:    ch->saving_throw            += mod; break;
      case APPLY_SAVING_PETRI:  ch->saving_throw            += mod; break;
      case APPLY_SAVING_BREATH: ch->saving_throw            += mod; break;
      case APPLY_SAVING_SPELL:  ch->saving_throw            += mod; break;
    }
  }

  /* make sure sex is RIGHT!!!! */ /* Sex is always right :D -- Merak */
  if ( ch->sex < 0 || ch->sex > 2 )
    ch->sex = ch->pcdata->true_sex;
}


/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch )
{
  CHAR_DATA *och = ch;

  if ( ch == NULL )
  {
    return ( 0 );
  }

  if ( ch->desc && ch->desc->original )
    och = ch->desc->original;

  if ( och->trust )
    return och->trust;

  if ( IS_NPC( och ) && och->level >= LEVEL_HERO )
    return LEVEL_HERO - 1;
  else
    return och->level;
}


/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch )
{
  //return 17 + ( ch->played + (int) (current_time - ch->logon) ) / 72000;
  int had_birthday = 0;
  int year;

  year = 17 + ( time_info.year - ch->birthday.year );
  if ( ch->birthday.month < time_info.month )
    had_birthday =1;
  else if ( ch->birthday.month == time_info.month )
  {
    if ( ch->birthday.day  < time_info.day
         &&   ch->birthday.hour < time_info.hour )
      had_birthday = 1;
  }
  return year + had_birthday;
}

/* command for retrieving stats */
int get_curr_stat( CHAR_DATA *ch, int stat )
{
  int max;

  if ( IS_NPC( ch ) || ch->level > LEVEL_IMMORTAL )
    max = 28;
  else
  {
    if ( pc_race_table[ch->race].max_stats[stat] < ch->perm_stat[stat] )
      max = ch->perm_stat[stat] + 4;
    else
      max = pc_race_table[ch->race].max_stats[stat] + 4;

    //Humans get 1 extra
    if ( ch->race == race_lookup( "human" ) )
      max += 1;

    //+2 class bonus
    if ( class_table[ch->gameclass].attr_prime == stat
         &&   ( pc_race_table[ch->race].max_stats[stat] + 2 >= ch->perm_stat[stat] ) )
    {
      max += 2;
      //make sure the +2 doesn't go over what is allowed
      //over 25 will fix itself below.
      //max = UMIN( max, pc_race_table[ch->race].max_stats[stat] + 4 );
    }

    //Make sure nobody goes over what is allowed
    if ( ch->perm_stat[stat] <= get_max_train( ch, stat ) )
      max = UMIN( max, 25 );
    if ( ch->perm_stat[stat] == get_stage1_max_train( ch, stat ) )
      max = UMIN( max, 26 );
    if ( ch->perm_stat[stat] == get_stage2_max_train( ch, stat ) )
      max = UMIN( max, 27 );
    else
      max = UMIN( max, 28 );
  }

  return URANGE( 3, ch->perm_stat[stat] + ch->mod_stat[stat], max );
}


/* command for returning max training score */
int get_max_train( CHAR_DATA *ch, int stat )
{
  int max;

  if ( IS_NPC( ch ) || ch->level > LEVEL_IMMORTAL )
    return 28;

  max = pc_race_table[ch->race].max_stats[stat];
  if ( class_table[ch->gameclass].attr_prime == stat )
  {
    if ( ch->race == race_lookup( "human" ) )
      max += 3;
    else
      max += 2;
  }

  return UMIN( max, 25 );
}

/* command for returning max training score */
int get_stage1_max_train( CHAR_DATA *ch, int stat )
{
  int max;

  if ( IS_NPC( ch ) || ch->level > LEVEL_IMMORTAL )
    return 26;

  max = pc_race_table[ch->race].max_stats[stat] + 1;
  if ( class_table[ch->gameclass].attr_prime == stat )
  {
    if ( ch->race == race_lookup( "human" ) )
      max += 3;
    else
      max += 2;
  }

  return UMIN( max, 26 );
}

/* command for returning max training score */
int get_stage2_max_train( CHAR_DATA *ch, int stat )
{
  int max;

  if ( IS_NPC( ch ) || ch->level > LEVEL_IMMORTAL )
    return 27;

  max = pc_race_table[ch->race].max_stats[stat] + 2;
  if ( class_table[ch->gameclass].attr_prime == stat )
  {
    if ( ch->race == race_lookup( "human" ) )
      max += 3;
    else
      max += 2;
  }

  return UMIN( max, 27 );
}

/* command for returning max training score */
int get_stage3_max_train( CHAR_DATA *ch, int stat )
{
  int max;

  if ( IS_NPC( ch ) || ch->level > LEVEL_IMMORTAL )
    return 28;

  max = pc_race_table[ch->race].max_stats[stat] + 2;
  if ( class_table[ch->gameclass].attr_prime == stat )
  {
    if ( ch->race == race_lookup( "human" ) )
      max += 3;
    else
      max += 2;
  }

  return UMIN( max, 28 );
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
  if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
    return 1000;

  if ( IS_NPC( ch ) && IS_PET( ch ) )
    return 0;

  return MAX_WEAR + 2 * get_curr_stat( ch, STAT_DEX ) + ch->level + dex_app[get_curr_stat( ch, STAT_DEX )].item_bonus;
}


int get_carry_weight( CHAR_DATA *ch )
{
  if ( !has_money_pouch( ch ) )
    return ( ch->carry_weight + ch->silver / 10 + 2 * ch->gold / 5 );
  else
    return ( ch->carry_weight + ch->silver / ( 10 * MONEY_POUCH_WEIGHT_MULT ) +
             2 * ch->gold / ( 5 * MONEY_POUCH_WEIGHT_MULT ) );
}

int has_money_pouch( CHAR_DATA *ch )
{
  OBJ_DATA *obj;

  int i = 0;
  for ( obj = ch->carrying; obj; obj = obj->next_content )
  {
    if ( obj->item_type == ITEM_MONEY_POUCH )
      return ( 5 );
    i++;
    //if (i > 2500) {    Winston said to remove
    //  bugf("CONTINUING:Infinite loop in has_money_pouch.%s",interp_cmd);
    //  obj->next_content = NULL;
    //  return(0);
    //}
  }
  return ( 0 );
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
  if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
    return 10000000;

  if ( IS_NPC( ch ) && IS_PET( ch ) )
    return ( ch->level * 20 ) + 500 ;

  return ( str_app[get_curr_stat( ch, STAT_STR )].carry * 10 +
           ch->level * 25 );
}

/*
 * See if a string is one of the names of an object.
 */
bool is_name ( char *str, char *namelist )
{
  char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
  char *list, *string;

  /* fix crash on NULL namelist */
  if ( namelist == NULL || namelist[0] == '\0' )
    return FALSE;

  /* fixed to prevent is_name on "" returning TRUE */
  if ( str[0] == '\0' )
    return FALSE;

  string = str;
  /* we need ALL parts of string to match part of namelist */
  for ( ; ; )  /* start parsing string */
  {
    str = one_argument( str, part );

    if ( part[0] == '\0' )
      return TRUE;

    /* check to see if this is part of namelist */
    list = namelist;
    for ( ; ; )  /* start parsing namelist */
    {
      list = one_argument( list, name );
      if ( name[0] == '\0' )  /* this name was not found */
        return FALSE;

      if ( !str_prefix( string, name ) )
        return TRUE; /* full pattern match */

      if ( !str_prefix( part, name ) )
        break;
    }
  }
}

bool is_exact_name( char *str, char *namelist )
{
  char name[MAX_INPUT_LENGTH];

  if ( namelist == NULL )
    return FALSE;

  for ( ; ; )
  {
    namelist = one_argument( namelist, name );
    if ( name[0] == '\0' )
      return FALSE;
    if ( !str_cmp( str, name ) )
      return TRUE;
  }
}

AFFECT_DATA *affect_exist( CHAR_DATA *ch, int gsn )
{
  AFFECT_DATA *af;

  for ( af = ch->affected; af; af = af->next )
    if ( af->type == gsn )
      return( af );

  return NULL;
}


/* enchanted stuff for eq */
void affect_enchant(OBJ_DATA *obj)
{
  /* okay, move all the old flags into new vectors if we have to */
  if ( !obj->enchanted )
  {
    AFFECT_DATA *paf, *af_new;
    obj->enchanted = TRUE;

    for ( paf = obj->pIndexData->affected; paf; paf = paf->next)
    {
      af_new = new_affect();
      /* Validated at this point */
      af_new->next  = obj->affected;
      obj->affected = af_new;

      af_new->where        = paf->where;
      af_new->type      = UMAX( 0, paf->type );
      af_new->level     = paf->level;
      af_new->duration  = paf->duration;
      af_new->location  = paf->location;
      af_new->modifier  = paf->modifier;
      af_new->bitvector = paf->bitvector;
    }
  }
}

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
  OBJ_DATA *wield = NULL;
  int mod, i;

  mod = paf->modifier;

  if ( fAdd )
  {
    switch ( paf->where )
    {
      case TO_AFFECTS:
        SET_BIT( ch->affected_by, paf->bitvector );
        break;
      case TO_AFFECTS2:
        SET_BIT( ch->affected2_by, paf->bitvector );
        break;
      case TO_ACT:
        SET_BIT( ch->act, paf->bitvector );
        break;
      case TO_SPELL_AFFECTS:
        SET_BIT( ch->spell_aff, paf->bitvector );
        break;
      case TO_IMMUNE:
        SET_BIT( ch->imm_flags, paf->bitvector );
        break;
      case TO_RESIST:
        SET_BIT( ch->res_flags, paf->bitvector );
        break;
      case TO_VULN:
        SET_BIT( ch->vuln_flags, paf->bitvector );
        break;
      case TO_COMM:
        SET_BIT( ch->comm_flags, paf->bitvector );
        break;
      case TO_PENALTY:
        SET_BIT( ch->pen_flags, paf->bitvector );
        break;
      case TO_CHANNEL:
        SET_BIT( ch->chan_flags, paf->bitvector );
        break;
    }
  }
  else
  {
    switch ( paf->where )
    {
      case TO_AFFECTS:
        REMOVE_BIT( ch->affected_by, paf->bitvector );
        ch->affected_by = ch->affected_by | race_table[ch->race].aff;
        break;
      case TO_AFFECTS2:
        REMOVE_BIT( ch->affected2_by, paf->bitvector );
        ch->affected2_by = ch->affected2_by | race_table[ch->race].aff2;
        break;
      case TO_ACT:
        REMOVE_BIT( ch->act, paf->bitvector );
        break;
      case TO_SPELL_AFFECTS:
        REMOVE_BIT( ch->spell_aff, paf->bitvector );
        break;
      case TO_IMMUNE:
        REMOVE_BIT( ch->imm_flags, paf->bitvector );
        ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
        break;
      case TO_RESIST:
        REMOVE_BIT( ch->res_flags, paf->bitvector );
        ch->res_flags = ch->res_flags | race_table[ch->race].res;
        break;
      case TO_VULN:
        REMOVE_BIT( ch->vuln_flags, paf->bitvector );
        ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
        break;
      case TO_COMM:
        REMOVE_BIT( ch->comm_flags, paf->bitvector );
        break;
      case TO_PENALTY:
        REMOVE_BIT( ch->pen_flags, paf->bitvector );
        break;
      case TO_CHANNEL:
        REMOVE_BIT( ch->chan_flags,paf->bitvector );
        break;
    }
    mod = 0 - mod;
  }

  switch ( paf->location )
  {
    default:
      bugf("Affect_modify_location error:Affc '%s' %3d %3d %3d %3d %3d %10d\n",
           skill_table[paf->type].name,
           paf->where,
           paf->level,
           paf->duration,
           paf->modifier,
           paf->location,
           paf->bitvector );
      return;

    case APPLY_NONE:                                            break;
    case APPLY_STR:           ch->mod_stat[STAT_STR]    += mod;    break;
    case APPLY_DEX:           ch->mod_stat[STAT_DEX]    += mod;    break;
    case APPLY_INT:           ch->mod_stat[STAT_INT]    += mod;    break;
    case APPLY_WIS:           ch->mod_stat[STAT_WIS]    += mod;    break;
    case APPLY_CON:           ch->mod_stat[STAT_CON]    += mod;    break;
    case APPLY_SEX:           change_sex(ch,mod,fAdd);            break;
    case APPLY_CLASS:                                            break;
    case APPLY_LEVEL:                                            break;
    case APPLY_AGE:                                                break;
    case APPLY_HEIGHT:                                            break;
    case APPLY_WEIGHT:                                            break;
    case APPLY_MANA:          ch->max_mana                += mod;    break;
    case APPLY_HIT:           ch->max_hit                += mod;    break;
    case APPLY_MOVE:          ch->max_move                += mod;    break;
    case APPLY_GOLD:                                            break;
    case APPLY_EXP:                                                break;
    case APPLY_AC:            for ( i = 0; i < 4; i ++ )
        ch->armor[i]              += mod; break;
    case APPLY_HITROLL:       ch->hitroll                += mod;    break;
    case APPLY_DAMROLL:       ch->damroll                += mod;    break;
    case APPLY_SAVES:         ch->saving_throw            += mod;    break;
    case APPLY_SAVING_ROD:    ch->saving_throw            += mod;    break;
    case APPLY_SAVING_PETRI:  ch->saving_throw            += mod;    break;
    case APPLY_SAVING_BREATH: ch->saving_throw            += mod;    break;
    case APPLY_SAVING_SPELL:  ch->saving_throw            += mod;    break;
    case APPLY_SPELL_AFFECT:                                      break;
  }

  /*
   * Check for weapon wielding.
   * Guard against recursion (for weapons with affects).
   */
  if ( !IS_NPC( ch ) && ( wield = get_eq_char( ch, WEAR_WIELD ) )
       &&    get_obj_weight(wield) >
       ( str_app[get_curr_stat( ch, STAT_STR )].wield * 10 ) )
  {
    static int depth = 0;

    if ( depth == 0 )
    {
      depth++;
      act( "You drop $p.", ch, wield, NULL, TO_CHAR );
      act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
      if ( !melt_drop( ch, wield ) )
      {
        obj_from_char( wield );
        if ( IS_OBJ_STAT( wield, ITEM_NODROP ) )
          obj_to_char( wield, ch );
        else
          if ( ch->in_room )
            obj_to_room( wield, ch->in_room );
      }
      depth--;
    }
  }
#if MEMDEBUG
  memdebug_check( ch, "affect modify" );
#endif
  return;
}


/* find an effect in an affect list */
AFFECT_DATA  *affect_find( AFFECT_DATA *paf, int sn )
{
  AFFECT_DATA *paf_find;

  for ( paf_find = paf; paf_find; paf_find = paf_find->next )
    if ( paf_find->type == sn )
      return paf_find;

  return NULL;
}

/* fix object affects when removing one */
void affect_check( CHAR_DATA *ch, int where, int vector )
{
  AFFECT_DATA *paf;
  OBJ_DATA *obj;

  if ( where == TO_OBJECT || where == TO_WEAPON || vector == 0 )
    return;

  for ( paf = ch->affected; paf; paf = paf->next )
    if ( paf->where == where && paf->bitvector == vector )
    {
      switch ( where )
      {
        case TO_AFFECTS:
          SET_BIT( ch->affected_by, vector );
          break;
        case TO_AFFECTS2:
          SET_BIT( ch->affected2_by, vector );
          break;
        case TO_IMMUNE:
          SET_BIT( ch->imm_flags, vector );
          break;
        case TO_RESIST:
          SET_BIT( ch->res_flags, vector );
          break;
        case TO_VULN:
          SET_BIT( ch->vuln_flags, vector );
          break;
      }
      return;
    }

  for ( obj = ch->carrying; obj; obj = obj->next_content )
  {
    if (obj->wear_loc == -1)
      continue;

    for ( paf = obj->affected; paf; paf = paf->next )
      if ( paf->where == where && paf->bitvector == vector )
      {
        switch ( where )
        {
          case TO_AFFECTS:
            SET_BIT( ch->affected_by, vector );
            break;
          case TO_AFFECTS2:
            SET_BIT( ch->affected2_by, vector );
            break;
          case TO_IMMUNE:
            SET_BIT( ch->imm_flags, vector );
            break;
          case TO_RESIST:
            SET_BIT( ch->res_flags, vector );
            break;
          case TO_VULN:
            SET_BIT( ch->vuln_flags, vector );
        }
        return;
      }

    if ( obj->enchanted )
      continue;

    for ( paf = obj->pIndexData->affected; paf; paf = paf->next )
      if ( paf->where == where && paf->bitvector == vector )
      {
        switch (where)
        {
          case TO_AFFECTS:
            SET_BIT( ch->affected_by, vector );
            break;
          case TO_AFFECTS2:
            SET_BIT( ch->affected2_by, vector );
          case TO_IMMUNE:
            SET_BIT( ch->imm_flags, vector );
            break;
          case TO_RESIST:
            SET_BIT( ch->res_flags, vector );
            break;
          case TO_VULN:
            SET_BIT( ch->vuln_flags, vector );
            break;
        }
        return;
      }
  }
}

/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
  AFFECT_DATA *paf_new;

  paf_new  =  new_affect();
  *paf_new    = *paf;

  VALIDATE ( paf_new );
  VALIDATE ( paf );
  paf_new->next    = ch->affected;
  ch->affected    = paf_new;

  affect_modify( ch, paf_new, TRUE );
  return;
}

/* give an affect to an object */
void affect_to_obj( OBJ_DATA *obj, AFFECT_DATA *paf )
{
  AFFECT_DATA *paf_new;

  paf_new  =  new_affect();
  *paf_new    = *paf;

  VALIDATE ( paf_new );
  VALIDATE ( paf );
  paf_new->next    = obj->affected;
  obj->affected    = paf_new;

  /* apply any affect vectors to the object's extra_flags */
  if ( paf->bitvector )
    switch ( paf->where )
    {
      case TO_OBJECT:
        SET_BIT( obj->extra_flags, paf->bitvector );
        break;
      case TO_WEAPON:
        if ( obj->item_type == ITEM_WEAPON )
          SET_BIT( obj->value[4], paf->bitvector );
        break;
    }

  return;
}

/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
  int where;
  int vector;

  if ( ch->affected == NULL )
  {
    bugf( "Affect_remove: no affect.%s", interp_cmd );
    return;
  }

  affect_modify( ch, paf, FALSE );
  where     = paf->where;
  vector    = paf->bitvector;

  if ( paf == ch->affected )
    ch->affected    = paf->next;
  else
  {
    AFFECT_DATA *prev;

    for ( prev = ch->affected; prev; prev = prev->next )
    {
      if ( prev->next == paf )
      {
        prev->next = paf->next;
        break;
      }
    }

    if ( prev == NULL )
    {
      bugf( "Affect_remove: cannot find paf.%s", interp_cmd );
      return;
    }
  }

  free_affect( paf );
  affect_check( ch, where, vector );
  return;
}

void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf )
{
  int where, vector;

  if ( obj->affected == NULL )
  {
    bugf( "Affect_remove_object: no affect. %s", interp_cmd );
    return;
  }

  if ( obj->carried_by && obj->wear_loc != -1 )
    affect_modify( obj->carried_by, paf, FALSE );

  where = paf->where;
  vector = paf->bitvector;

  /* remove flags from the object if needed */
  if ( paf->bitvector )
    switch ( paf->where )
    {
      case TO_OBJECT:
        REMOVE_BIT( obj->extra_flags, paf->bitvector );
        break;
      case TO_WEAPON:
        if ( obj->item_type == ITEM_WEAPON )
          REMOVE_BIT( obj->value[4], paf->bitvector );
        break;
    }

  if ( paf == obj->affected )
    obj->affected = paf->next;
  else
  {
    AFFECT_DATA *prev;

    for ( prev = obj->affected; prev; prev = prev->next )
    {
      if ( prev->next == paf )
      {
        prev->next = paf->next;
        break;
      }
    }

    if ( prev == NULL )
    {
      bugf( "Affect_remove_object: cannot find paf %s.", interp_cmd );
      return;
    }
  }

  free_affect( paf );

  if ( obj->carried_by && obj->wear_loc != -1 )
    affect_check( obj->carried_by, where, vector );
  return;
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;

  for ( paf = ch->affected; paf; paf = paf_next )
  {
    paf_next = paf->next;
    if ( paf->type == sn )
      affect_remove( ch, paf );
  }

  return;
}

/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
  AFFECT_DATA *paf;

  for ( paf = ch->affected; paf; paf = paf->next )
    if ( paf->type == sn )
      return TRUE;

  return FALSE;
}

/*
 * Add or enhance an affect.
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
  AFFECT_DATA *paf_old;

  if (paf == NULL)
    return;

  for ( paf_old = ch->affected; paf_old; paf_old = paf_old->next )
  {
    if ( paf_old->type == paf->type )
    {
      paf->level = (paf->level + paf_old->level) / 2;
      paf->duration += paf_old->duration;
      paf->modifier += paf_old->modifier;
      affect_remove( ch, paf_old );
      break;
    }
  }

  affect_to_char( ch, paf );
  return;
}

/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
  OBJ_DATA *obj;

  if ( ch->in_room == NULL )
  {
    bugf( "Char_from_room:[%s] NULL. %s",ch->name, interp_cmd );
    return;
  }

  if ( !IS_NPC( ch ) )
  {
    if ( --ch->in_room->area->nplayer == 0 )
      ch->in_room->area->empty = TRUE;
  }

  if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) )
       &&     obj->item_type == ITEM_LIGHT
       &&     obj->value[2]
       &&     ch->in_room->light > 0 )
    --ch->in_room->light;

  if ( ch == ch->in_room->people )
  {
    ch->in_room->people = ch->next_in_room;
  }
  else
  {
    CHAR_DATA *prev;

    for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
    {
      if ( prev->next_in_room == ch )
      {
        prev->next_in_room = ch->next_in_room;
        break;
      }
    }

    if ( prev == NULL )
      bugf( "Char_from_room: ch [%s] not found. %s", ch->name, interp_cmd );
  }

  ch->in_room       = NULL;
  ch->next_in_room  = NULL;
  ch->on             = NULL;  /* sanity check! */
  REMOVE_BIT( ch->active_flags, ACTIVE_CHAR_IN_ROOM );

  return;
}

/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
  OBJ_DATA  *obj;
  char      *tmp;

  if ( pRoomIndex == NULL )
  {
    ROOM_INDEX_DATA *room;

    bugf( "Char_to_room: %s NULL.%s", ch->name, interp_cmd );

    if ( ( room = get_room_index( ROOM_VNUM_TEMPLE ) ) )
      char_to_room( ch, room );
    return;
  }

//  if ( ch->pcdata->tag_area
//  && ( ch->pcdata->tag_area != pRoomIndex->area ) )
//  {
//    send_to_char( "You must recall or leave the tag game.\n\r", ch );
//    return;
//  }

  /* Quick check here to make sure char isn't already in a room! */
  if ( IS_SET( ch->active_flags, ACTIVE_CHAR_IN_ROOM ) )
  {
    bugf("Char_to_room: Calling it for a Character[%s] Already in a room. -%s--- VNUM: %d",
         ch->name, interp_cmd, pRoomIndex->vnum);
    return;
  }
  /*
    if (ch->in_room == pRoomIndex) {
    }
  */
  ch->in_room        = pRoomIndex;
  if ( ch == pRoomIndex->people )
  {
    bugf( "Char_to_room: Character %s already SET as first person in room. %s",
          ch->name, interp_cmd);
  }
  else
    ch->next_in_room    = pRoomIndex->people;
  pRoomIndex->people    = ch;

  REMOVE_BIT(ch->act, PLR_FISHING); // easiest way to stop fishing... maybe a message to char?

  SET_BIT( ch->active_flags, ACTIVE_CHAR_IN_ROOM );

  if ( !IS_NPC( ch ) )
  {
    /*if (ch->in_room->area->empty)
      {
        ch->in_room->area->empty = FALSE;
        ch->in_room->area->age = 0;
      }*/
    ++ch->in_room->area->nplayer;
  }
  else // It is an NPC, check for change in long desc....
  {
    tmp = ch->pIndexData->long2_descr;
    if ( ch->reset_room && IS_SET( ch->act, ACT_SENTINEL )
         &&   tmp && tmp[0] && tmp[0] != ' '
         &&   strcmp( "(null)", tmp ) )
      // we don't do anything, unless it has a resetroom and is sentinel
    {
      if ( ch->reset_room == ch->in_room )
      {
        free_string( ch->long_descr );
        ch->long_descr = str_dup( ch->pIndexData->long_descr, ch->long_descr );
      }
      else
      {
        free_string( ch->long_descr );
        ch->long_descr = str_dup( ch->pIndexData->long2_descr, ch->long_descr );
      }
    }
  }

  if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) )
       &&     obj->item_type == ITEM_LIGHT
       &&     obj->value[2] )
    ++ch->in_room->light;

  if ( IS_AFFECTED( ch, AFF_PLAGUE ) )
  {
    AFFECT_DATA *af, plague;
    CHAR_DATA *vch;

    for ( af = ch->affected; af; af = af->next )
    {
      if ( af->type == gsn_plague )
        break;
    }

    if ( af == NULL )
    {
      REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
      return;
    }

    if ( af->level == 1 )
      return;

    plague.where        = TO_AFFECTS;
    plague.type         = gsn_plague;
    plague.level         = af->level - 1;
    plague.duration     = number_range( 1 , 2 * plague.level );
    plague.location    = APPLY_STR;
    plague.modifier     = -5;
    plague.bitvector     = AFF_PLAGUE;

    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
      if ( !saves_spell( plague.level - 2, vch, DAM_DISEASE )
           &&   !IS_IMMORTAL( vch )
           &&   !IS_AFFECTED( vch, AFF_PLAGUE )
           &&    number_bits( 6 ) == 0
           &&   !IS_SET( vch->in_room->room_flags, ROOM_SAFE ) )
      {
        send_to_char( "You feel hot and feverish.\n\r", vch );
        act( "$n shivers and looks very ill.", vch, NULL, NULL, TO_ROOM );
        affect_join( vch, &plague );
      }
    }
  }
  return;
}

/*
 * Move a char into a room. QUICK function.
 */
void move_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
  if ( ch->in_room == NULL )
    bugf( "MoveToRoom Ch [%s] inroom NULL.  %s", ch->name, interp_cmd );
  if ( pRoomIndex == NULL )
    bugf( "MoveToRoom RoomIndex NULL.[%s],  %s", ch->name, interp_cmd );
  if ( ch->in_room == pRoomIndex )
  {
//    bugf( "MoveToRoom: Moving to same room. [%s],  %s", ch->name, interp_cmd );
//    return;
  }

  char_from_room( ch );
  char_to_room( ch, pRoomIndex );
}

/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
  obj->next_content    = ch->carrying;
  ch->carrying        = obj;
  obj->carried_by        = ch;
  obj->in_room        = NULL;
  obj->in_obj            = NULL;
  ch->carry_number   += get_obj_number( obj );
  ch->carry_weight   += get_obj_weight( obj );

  /* Object ownership. */
  if ( IS_NULLSTR( obj->owner ) && IS_SET( obj->extra_flags, ITEM_CLANEQ ) )
  {
    free_string( obj->owner ); //Make sure the memory is clear by default
    obj->owner = str_dup( ch->name, obj->owner ); //Then set the memory
    //printf_to_char( ch, "Setting clan owner: %s owner, %s name\n\r",
    //                obj->owner, ch->name );
    set_claneq( obj, ch ); //Call set_claneq from clan.c
  }
}

/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
  CHAR_DATA *ch;

  if ( ( ch = obj->carried_by ) == NULL )
  {
    bug( "Obj_from_char: null ch.", 0 );
    return;
  }

  if ( obj->wear_loc != WEAR_NONE )
    unequip_char( ch, obj );

  if ( ch->carrying == obj )
    ch->carrying = obj->next_content;
  else
  {
    OBJ_DATA *prev;

    for ( prev = ch->carrying; prev; prev = prev->next_content )
    {
      if ( prev->next_content == obj )
      {
        prev->next_content = obj->next_content;
        break;
      }
    }

    if ( prev == NULL )
      bugf( "Obj_from_char %s: obj %s not in list.", ch->name, obj->name );
  }

  obj->carried_by     = NULL;
  obj->next_content     = NULL;
  ch->carry_number    -= get_obj_number( obj );
  ch->carry_weight    -= get_obj_weight( obj );
  return;
}

/*** put an object in the char's locker
     Chris Jensen 11/4/2000
***/
void obj_to_locker( OBJ_DATA *obj, CHAR_DATA *ch )
{
  if ( IS_NPC( ch ) )
    return;

  obj->next_content              = ch->pcdata->locker;
  ch->pcdata->locker          = obj;
  ch->pcdata->locker_content += get_obj_number( obj );
  obj->carried_by              = ch;
  obj->in_room                  = NULL;
  obj->in_obj                  = NULL;
}

/*** take an object from the char's locker
     Chris Jensen 11/4/2000
***/
void obj_from_locker( OBJ_DATA *obj, CHAR_DATA *ch )
{
  if ( IS_NPC( ch ) )
    return;

  if ( ch->pcdata->locker == obj )
    ch->pcdata->locker = obj->next_content;
  else
  {
    OBJ_DATA *prev;

    for ( prev = ch->pcdata->locker; prev; prev = prev->next_content )
    {
      if ( prev->next_content == obj )
      {
        prev->next_content = obj->next_content;
        break;
      }
    }

    if ( prev == NULL )
      bug( "Obj_from_locker: obj not in list.", 0 );
  }

  obj->next_content     = NULL;
  obj->carried_by    = NULL;
  ch->pcdata->locker_content -= get_obj_number( obj );
  return;
}

/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear, int type )
{
  if ( obj->item_type != ITEM_ARMOR )
    return 0;

  switch ( iWear )
  {
    case WEAR_BODY:        return 3 * obj->value[type];
    case WEAR_HEAD:        return 2 * obj->value[type];
    case WEAR_LEGS:        return 2 * obj->value[type];
    case WEAR_FEET:        return     obj->value[type];
    case WEAR_HANDS:        return     obj->value[type];
    case WEAR_ARMS:        return     obj->value[type];
    case WEAR_SHIELD:        return 2 * obj->value[type];
    case WEAR_NECK_1:        return     obj->value[type];
    case WEAR_NECK_2:        return     obj->value[type];
    case WEAR_ABOUT:        return 2 * obj->value[type];
    case WEAR_WAIST:        return     obj->value[type];
    case WEAR_WRIST_L:    return     obj->value[type];
    case WEAR_WRIST_R:    return     obj->value[type];
    case WEAR_FINGER_L:    return     obj->value[type];
    case WEAR_FINGER_R:    return     obj->value[type];
    case WEAR_HOLD:        return     obj->value[type];
    case WEAR_FLOAT:        return     obj->value[type];
    case WEAR_CREST:      return     obj->value[type];
  }

  return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
  OBJ_DATA *obj;

  if ( ch == NULL )
    return NULL;

  for ( obj = ch->carrying; obj; obj = obj->next_content )
  {
    if (obj->next_content == obj)
    {
      bugf( "infinite obj loop. %s", interp_cmd );
      obj->next_content = NULL;
    }
    if ( obj->wear_loc == iWear )
      return obj;
  }

  return NULL;
}



/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
  AFFECT_DATA *paf;
  int i;

  if ( get_eq_char( ch, iWear ) )
  {
    bugf("Equip_char: %s is already equipped at spot (%d).",
         ch->name, iWear );
    return;
  }

  if ( ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL )     && IS_EVIL( ch ) )
       ||   ( IS_OBJ_STAT( obj, ITEM_ANTI_GOOD )     && IS_GOOD( ch ) )
       ||   ( IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL )  && IS_NEUTRAL( ch ) ) )
  {
    if (IS_SET(obj->extra_flags,ITEM_NODROP) )
    {
      act( "You are zapped by $p and remove it.", ch, obj, NULL, TO_CHAR );
      act( "$n is zapped by $p and removes it.",  ch, obj, NULL, TO_ROOM );
    }
    else
    {
      act( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
      act( "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
      if ( !melt_drop( ch, obj ) )
      {
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
      }
    }
    return;
  }

  for ( i = 0; i < 4; i++ )
    ch->armor[i] -= apply_ac( obj, iWear,i );
  obj->wear_loc      = iWear;

  if ( !obj->enchanted )
    for ( paf = obj->pIndexData->affected; paf; paf = paf->next )
      if ( paf->location != APPLY_SPELL_AFFECT )
        affect_modify( ch, paf, TRUE );

  for ( paf = obj->affected; paf; paf = paf->next )
    if ( paf->location == APPLY_SPELL_AFFECT )
      affect_to_char ( ch, paf );
    else
      affect_modify( ch, paf, TRUE );

  if ( obj->item_type == ITEM_LIGHT
       &&   obj->value[2]
       &&   ch->in_room )
    ++ch->in_room->light;

  return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
  AFFECT_DATA *pAf       = NULL;
  AFFECT_DATA *lpAf      = NULL;
  AFFECT_DATA *lpAf_next = NULL;
  OBJ_DATA *wield;
  int     i;
  int     old_loc;

  if ( !IS_WORN( obj ) )
  {
    bugf( "Unequip_char: %s %d (%s) already unequipped, R[%d]",
          IS_NPC( ch ) ? "npc" : "pc",
          IS_NPC( ch ) ? ch->pIndexData->vnum : 0,
          IS_NPC( ch ) ? ch->short_descr : ch->name,
          ch->in_room->vnum );
    return;
  }

  for ( i = 0; i < 4; i++ )
    ch->armor[i] += apply_ac( obj, obj->wear_loc, i );

  old_loc         = obj->wear_loc;
  obj->wear_loc     = WEAR_NONE;

  if ( ( old_loc == WEAR_WIELD )
       &&   ( wield = get_eq_char( ch, WEAR_SECONDARY ) ) )
  {
    wield->wear_loc = WEAR_WIELD;
    /* act("Your primary weapon is now $p.", ch, wield, NULL, TO_CHAR);*/
  }

  if ( !obj->enchanted )
    for ( pAf = obj->pIndexData->affected; pAf; pAf = pAf->next )
    {
      if ( pAf->location == APPLY_SPELL_AFFECT )
      {
        for ( lpAf = ch->affected; lpAf; lpAf = lpAf_next )
        {
          lpAf_next = lpAf->next;
          if ( lpAf
               &&   lpAf->type       == pAf->type
               &&   lpAf->level      == pAf->level
               &&   lpAf->location   == APPLY_SPELL_AFFECT )
          {
            affect_remove( ch, lpAf );
            lpAf->next = NULL;
          }
        }
      }
      else
      {
        affect_modify( ch, pAf, FALSE );
        affect_check( ch, pAf->where, pAf->bitvector );
      }
    }

  for ( pAf = obj->affected; pAf; pAf = pAf->next )
  {
    if ( pAf == NULL )
      continue;

    if ( pAf->location == APPLY_SPELL_AFFECT )
    {
      bugf( "Unequip_char: apply_spell_affect, obj %d (%s)",
            obj->pIndexData->vnum, obj->short_descr );
      for ( lpAf = ch->affected; lpAf; lpAf = lpAf_next )
      {
        lpAf_next = lpAf->next;
        if ( lpAf
             &&   lpAf->type       == pAf->type
             &&   lpAf->level      == pAf->level
             &&   lpAf->location   == APPLY_SPELL_AFFECT )
        {
          bugf( "Unequip_char: location %d, type %d",
                lpAf->location, lpAf->type );
          affect_remove( ch, lpAf );
          lpAf->next = NULL;
        }
      }
    }
    else
    {
      affect_modify( ch, pAf, FALSE );
      affect_check( ch, pAf->where, pAf->bitvector );
    }
  }

  if ( old_loc == WEAR_LIGHT
       &&   obj->item_type == ITEM_LIGHT
       &&   obj->value[2]
       &&   ch->in_room
       &&   ch->in_room->light > 0 )
    --ch->in_room->light;
  return;
}

/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
  OBJ_DATA *obj;
  int nMatch;

  nMatch = 0;

  for ( obj = list; obj; obj = obj->next_content )
    if ( obj->pIndexData == pObjIndex )
      nMatch++;

  return nMatch;
}

/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
  ROOM_INDEX_DATA *in_room;
  CHAR_DATA *ch;

  if ( ( in_room = obj->in_room ) == NULL )
  {
    bug( "obj_from_room: NULL.", 0 );
    return;
  }

  for ( ch = in_room->people; ch; ch = ch->next_in_room )
    if ( ch->on == obj )
      ch->on = NULL;

  if ( obj == in_room->contents )
    in_room->contents = obj->next_content;
  else
  {
    OBJ_DATA *prev;

    for ( prev = in_room->contents; prev; prev = prev->next_content )
    {
      if ( prev->next_content == obj )
      {
        prev->next_content = obj->next_content;
        break;
      }
    }

    if ( prev == NULL )
    {
      bug( "Obj_from_room: obj not found.", 0 );
      return;
    }
  }

  obj->in_room      = NULL;
  obj->next_content = NULL;
  return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{
  obj->next_content        = pRoomIndex->contents;
  pRoomIndex->contents    = obj;
  obj->in_room            = pRoomIndex;
  obj->carried_by            = NULL;
  obj->in_obj                = NULL;
  return;
}

/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
  if ( obj == obj_to )
  {
    bugf( "Obj %s is being put in the same Obj %s\n\r",
          obj->name, obj_to->name );
    return;
  }

  obj->next_content        = obj_to->contains;
  obj_to->contains        = obj;
  obj->in_obj            = obj_to;
  obj->in_room            = NULL;
  obj->carried_by        = NULL;

  if ( obj_to->pIndexData->vnum == OBJ_VNUM_PIT )
    obj->cost = 0;

  for ( ; obj_to; obj_to = obj_to->in_obj )
  {
    if ( obj_to->carried_by )
    {
      obj_to->carried_by->carry_number += get_obj_number( obj );
      obj_to->carried_by->carry_weight += get_obj_weight( obj ) *
                                          WEIGHT_MULT( obj_to ) / 100;
    }
  }
  return;
}

/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
  OBJ_DATA *obj_from;

  if ( ( obj_from = obj->in_obj ) == NULL )
  {
    bug( "Obj_from_obj: null obj_from.", 0 );
    return;
  }

  if ( obj == obj_from->contains )
    obj_from->contains = obj->next_content;
  else
  {
    OBJ_DATA *prev;

    for ( prev = obj_from->contains; prev; prev = prev->next_content )
      if ( prev->next_content == obj )
      {
        prev->next_content = obj->next_content;
        break;
      }

    if ( prev == NULL )
    {
      bug( "Obj_from_obj: obj not found.", 0 );
      return;
    }
  }
  obj->next_content = NULL;
  obj->in_obj       = NULL;

  for ( ; obj_from; obj_from = obj_from->in_obj )
    if ( obj_from->carried_by )
    {
      obj_from->carried_by->carry_number -= get_obj_number( obj );
      obj_from->carried_by->carry_weight -= get_obj_weight( obj ) *
                                            WEIGHT_MULT(obj_from) / 100;
    }

  return;
}

/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj )
{
  OBJ_DATA *obj_content;
  OBJ_DATA *obj_next;

  if ( obj->in_room )
    obj_from_room( obj );
  else if ( obj->carried_by )
    obj_from_char( obj );
  else if ( obj->in_obj )
    obj_from_obj( obj );

  for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
  {
    obj_next = obj_content->next_content;
    extract_obj( obj_content );
  }

  if ( object_list == obj )
  {
    object_list = obj->next;
  }
  else
  {
    OBJ_DATA *prev;

    for ( prev = object_list; prev; prev = prev->next )
    {
      if ( prev->next == obj )
      {
        prev->next = obj->next;
        break;
      }
    }

    if ( prev == NULL )
    {
      bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
      return;
    }
  }

  --obj->pIndexData->count;
  free_obj( obj );
  return;
}



/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
  ROOM_INDEX_DATA   *room;
  CHAR_DATA         *wch;
  OBJ_DATA          *obj;
  OBJ_DATA          *obj_next;

  nuke_pets( ch, FALSE );
  ch->pet = NULL; /* just in case */

  if ( fPull )
    die_follower( ch );

  stop_fighting( ch, TRUE );

  for ( obj = ch->carrying; obj; obj = obj_next )
  {
    obj_next = obj->next_content;
    VALIDATE( obj );
    extract_obj( obj );
  }

  room = ch->in_room; //Added to pass to get_death_rom

  if ( ch->in_room )
    char_from_room( ch );

  /* Death room is set in the clan table now */
  if ( !fPull )
  {
    char_to_room( ch, get_room_index( get_death_room( ch, room ) ) );
    return;
  }

  if ( IS_NPC( ch ) )
    --ch->pIndexData->count;

  if ( ch->desc && ch->desc->original )
  {
    do_function(ch, &do_return, "" );
    ch->desc = NULL;
  }

  for ( wch = char_list; wch; wch = wch->next )
  {
    if ( wch->reply == ch )
    {
      REMOVE_BIT( wch->comm_flags, COMM_REPLY_LOCK );
      wch->reply = NULL;
    }
    if ( ch->mprog_target == wch )
      wch->mprog_target = NULL;
  }

  if ( ch == char_list )
    char_list = ch->next;
  else
  {
    CHAR_DATA *prev;

    for ( prev = char_list; prev; prev = prev->next )
    {
      if ( prev->next == ch )
      {
        prev->next = ch->next;
        break;
      }
    }

    if ( prev == NULL )
    {
      bugf( "Extract_char: char_list: char not found. %s", interp_cmd );
      /*      return;*/
    }
  }
  if ( !IS_NPC( ch ) )
  {
    if ( ch == player_list )
      player_list = ch->next_player;
    else
    {
      CHAR_DATA *prev;

      for ( prev = player_list; prev; prev = prev->next_player )
      {
        if ( prev->next_player == ch )
        {
          prev->next_player = ch->next_player;
          break;
        }
      }
      if ( prev == NULL )
      {
        bugf( "Extract_char: player_list: char not found. %s", interp_cmd );
        /*      return;*/
      }
    }
  }

  if ( ch->desc )
    ch->desc->character = NULL;
  free_char( ch );
  return;
}

/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *rch;
  int i=0;

  one_argument( argument, arg );

  if ( !str_cmp( argument, "self" )
       ||   !str_cmp( argument, ".self" ) )
    return ch;

  if ( arg[0] == '\0' )
    return NULL;

  if (arg[0] == '.')
  {
    for (i=0;arg[i] != '\0';i++)
      arg[i] = arg[i+1];

    CHAR_DATA *lch = NULL;
    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
      if ( !can_see( ch, rch ) || !is_name( arg, rch->name ) )
        continue;
      lch = rch;
    }
    return lch;
  }
  else
  {
    int number = number_argument( argument, arg );;
    int count = 0;

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
      if ( !can_see( ch, rch ) || !is_name( arg, rch->name ) )
        continue;
      if ( ++count == number )
        return rch;
    }
  }
  return NULL;
}

/*
 * Find a char in the room, starting with "mobs" or "chars".
 */
CHAR_DATA *get_char_room_ordered( CHAR_DATA *ch, char *argument, char *sortfirst )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *rch;
  int i=0;

  one_argument( argument, arg );

  if ( !str_cmp( argument, "self" )
       ||   !str_cmp( argument, ".self" ) )
    return ch;

  if (arg[0] == '.')
  {
    for (i=0;arg[i] != '\0';i++)
      arg[i] = arg[i+1];

    CHAR_DATA *lch = NULL;
    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
      if ( !str_cmp( sortfirst, "mobs" )  && !IS_NPC( rch ) )
        continue;
      if ( !str_cmp( sortfirst, "chars" ) &&  IS_NPC( rch ) )
        continue;
      if ( !can_see( ch, rch ) || !is_name( arg, rch->name ) )
        continue;
      lch = rch;
    }
    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
      if ( !str_cmp( sortfirst, "mobs" )  &&  IS_NPC( rch ) )
        continue;
      if ( !str_cmp( sortfirst, "chars" ) && !IS_NPC( rch ) )
        continue;
      if ( !can_see( ch, rch ) || !is_name( arg, rch->name ) )
        continue;
      lch = rch;
    }
    return lch;
  }
  else
  {
    int count = 0;
    int number = number_argument( argument, arg );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
      if ( !str_cmp( sortfirst, "mobs" )  && !IS_NPC( rch ) )
        continue;
      if ( !str_cmp( sortfirst, "chars" ) &&  IS_NPC( rch ) )
        continue;
      if ( !can_see( ch, rch ) || !is_name( arg, rch->name ) )
        continue;
      if ( ++count == number )
        return rch;
    }
    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
      if ( !str_cmp( sortfirst, "mobs" )  &&  IS_NPC( rch ) )
        continue;
      if ( !str_cmp( sortfirst, "chars" ) && !IS_NPC( rch ) )
        continue;
      if ( !can_see( ch, rch ) || !is_name( arg, rch->name ) )
        continue;
      if ( ++count == number )
        return rch;
    }
  }
  return NULL;
}

/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *wch;
  int number;
  int count;

  if ( ( wch = get_char_room( ch, argument ) ) )
    return wch;

  number = number_argument( argument, arg );
  count  = 0;

  for ( wch = char_list; wch; wch = wch->next )
  {
    if ( wch->in_room == NULL || !can_see( ch, wch )
         ||  !is_name( arg, wch->name ) )
      continue;
    if ( ++count == number )
      return wch;
  }
  return NULL;
}

/*
 * Find a char in the world, try harder for valid target
 */
CHAR_DATA *get_char_world_persistent( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *wch;
  int number;
  int count;

  if ( ( wch = get_char_room( ch, argument ) ) )
    return wch;

  number = number_argument( argument, arg );
  count  = 0;

  for ( wch = char_list; wch; wch = wch->next )
  {
    if ( wch->in_room == NULL
         || !can_see( ch, wch )
         || !is_name( arg, wch->name ) )
      continue;

    if ( IS_SET(wch->in_room->area->flags, AREA_DRAFT)
         ||   IS_SET(wch->in_room->area->flags, AREA_NOGATE)
         ||  ( ch->in_room->area->continent != wch->in_room->area->continent ) )
      continue;

    if ( ++count == number )
      return wch;
  }
  return NULL;
}

CHAR_DATA *get_char_world_ordered( CHAR_DATA *ch, char *argument, char *sortfirst )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *wch;
  int number;
  int count;

//  if ( ( wch = get_char_room_ordered( ch, argument, sortfirst ) ) )
//    return wch;

  number = number_argument( argument, arg );

  count  = 0;
  for ( wch = char_list; wch; wch = wch->next )
  {
    if ( !str_cmp( sortfirst, "mobs" )  && !IS_NPC( wch ) )
      continue;

    if ( !str_cmp( sortfirst, "chars" ) &&  IS_NPC( wch ) )
      continue;


    if ( wch->in_room == NULL || !can_see( ch, wch )
         ||  !is_name( arg, wch->name ) )
      continue;

    if ( ++count == number )
      return wch;
  }

  count  = 0;
  for ( wch = char_list; wch; wch = wch->next )
  {
    if ( !str_cmp( sortfirst, "mobs" )  &&  IS_NPC( wch ) )
      continue;

    if ( !str_cmp( sortfirst, "chars" ) && !IS_NPC( wch ) )
      continue;

    if ( wch->in_room == NULL || !can_see( ch, wch )
         ||  !is_name( arg, wch->name ) )
      continue;

    if ( ++count == number )
      return wch;
  }
  return NULL;
}

/*OBJ_DATA *get_donation_pit( OBJ_DATA *obj-> )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL ; obj = obj->nex )
    {
        if ( (obj->pIndexdata == num ) )
            return obj;
    }

    return NULL;
}*/

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
  OBJ_DATA *obj;

  for ( obj = object_list; obj; obj = obj->next )
  {
    if ( obj->pIndexData == pObjIndex )
      return obj;
  }
  return NULL;
}

/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MIL];
  OBJ_DATA *obj;
  int number;
  int count;

  strcpy( buf, argument );
  number = number_argument( buf, arg );
  count  = 0;
  for ( obj = list; obj; obj = obj->next_content )
  {
    if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
      if ( ++count == number )
        return obj;
  }
  return NULL;
}

/*
 * Find an obj in a donation pit.
 */
OBJ_DATA *get_obj_list_donation_pit( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MIL];
  OBJ_DATA *obj;
  int number;
  int count;
  int z = 0;

  strcpy( buf, argument );
  number = number_argument( buf, arg );
  count  = 0;
  for ( z = 0 ; z < LEVEL_IMMORTAL ; z++ )
  {
    for ( obj = list; obj; obj = obj->next_content )
    {
      if ( obj->level == z )
        if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
          if ( ++count == number )
            return obj;
    }
  }
  return NULL;
}


/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument, CHAR_DATA *viewer )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;

  number = number_argument( argument, arg );
  count  = 0;
  for ( obj = ch->carrying; obj; obj = obj->next_content )
  {
    if ( obj->wear_loc == WEAR_NONE
         && ( can_see_obj( viewer, obj ) )
         &&   is_name( arg, obj->name ) )
    {
      if ( ++count == number )
        return obj;
    }
  }
  return NULL;
}

/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;

  number = number_argument( argument, arg );
  count  = 0;
  for ( obj = ch->carrying; obj; obj = obj->next_content )
  {
    if ( obj->wear_loc != WEAR_NONE
         &&   can_see_obj( ch, obj )
         &&   is_name( arg, obj->name ) )
    {
      if ( ++count == number )
        return obj;
    }
  }
  return NULL;
}

/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;

  if ( ( obj = get_obj_list( ch, argument, ch->in_room->contents ) ) )
    return obj;

  if ( ( obj = get_obj_carry( ch, argument, ch ) ) )
    return obj;

  if ( ( obj = get_obj_wear( ch, argument ) ) )
    return obj;

  return NULL;
}

/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;

  if ( ( obj = get_obj_here( ch, argument ) ) )
    return obj;

  number = number_argument( argument, arg );
  count  = 0;
  for ( obj = object_list; obj; obj = obj->next )
  {
    if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
    {
      if ( ++count == number )
        return obj;
    }
  }
  return NULL;
}

/*
 * Find an obj in the world from its vnum
 */
OBJ_DATA *find_obj_vnum( int vnum )
{
  OBJ_DATA *obj;

  for ( obj = object_list; obj; obj = obj->next )
    if ( obj->pIndexData->vnum == vnum )
      break;

  return obj;
}

/* deduct cost from a character */
void new_deduct_cost( CHAR_DATA *ch, int cost, int *deducted_gold,
                      int *deducted_silver )
{
  int gold;
  int silver;

  if ( IS_NPC( ch ) && ch->pIndexData->pShop )
  {
    gold   = UMIN( ch->gold * 100, cost ) / 100;
    silver = cost - gold * 100;

    /* No negative ch->silver, convert some gold over. */
    if ( silver > ch->silver )
    {
      ch->gold   -= ( silver - ch->silver + 99 ) / 100;
      ch->silver += ( silver - ch->silver + 99 ) / 100 * 100;
    }
  }
  else
  {
    gold   = 0;
    silver = UMIN( ch->silver, cost );

    /* Not enough silver, use some gold. */
    if ( silver < cost )
    {
      gold   = ( cost - silver + 99 ) / 100;
      silver = cost - gold * 100;
    }
  }

  /* Negative silver, see ch->silver is enough to cover. */
  if ( silver < 0 && 100 + silver <= ch->silver )
  {
    gold   -= 1;
    silver += 100;
  }

  /* Deduct the money. */
  ch->gold   -= gold;
  ch->silver -= silver;

  if ( ch->gold < 0 )
  {
    bugf( "new_deduct_cost: ch gold %d", ch->gold );
    ch->gold = 0;
  }

  if ( ch->silver < 0 )
  {
    bugf( "new_deduct_cost: ch silver %d", ch->silver );
    ch->silver = 0;
  }

  /* Set deducted variables. */
  if ( deducted_gold )
    *deducted_gold = gold;
  if ( deducted_silver )
    *deducted_silver = silver;
}

/* deduct cost from a character */
void deduct_cost( CHAR_DATA *ch, int cost )
{
  int silver = 0, gold = 0;

  silver = UMIN( ch->silver, cost );

  if ( silver < cost )
  {
    gold = ( ( cost - silver + 99 ) / 100 );
    silver = cost - 100 * gold;
  }
  ch->gold   -= gold;
  ch->silver -= silver;

  if ( ch->gold < 0 )
  {
    bug("deduct costs: gold %d < 0",ch->gold);
    ch->gold = 0;
  }

  if (ch->silver < 0)
  {
    bug("deduct costs: silver %d < 0",ch->silver);
    ch->silver = 0;
  }
}

/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int gold, int silver )
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;

  if ( gold < 0 || silver < 0 || ( gold == 0 && silver == 0 ) )
  {
    bug( "Create_money: zero or negative money.", UMIN( gold, silver ) );
    gold   = UMAX( 1, gold );
    silver = UMAX( 1, silver );
  }

  if ( gold == 0 && silver == 1 )
    obj = create_object( get_obj_index( OBJ_VNUM_SILVER_ONE ), 0 );

  else if ( gold == 1 && silver == 0 )
    obj = create_object( get_obj_index( OBJ_VNUM_GOLD_ONE), 0 );

  else if ( silver == 0 )
  {
    obj = create_object( get_obj_index( OBJ_VNUM_GOLD_SOME ), 0 );
    mprintf(sizeof(buf), buf, obj->short_descr, gold );
    free_string( obj->short_descr );
    obj->short_descr    = str_dup( buf, obj->short_descr );
    obj->value[1]       = gold;
    obj->cost           = gold;
    obj->weight            = 2 * gold / 5;
  }
  else if ( gold == 0 )
  {
    obj = create_object( get_obj_index( OBJ_VNUM_SILVER_SOME ), 0 );
    mprintf(sizeof(buf), buf, obj->short_descr, silver );
    free_string( obj->short_descr );
    obj->short_descr    = str_dup( buf, obj->short_descr );
    obj->value[0]       = silver;
    obj->cost           = silver;
    obj->weight            = silver / 10;
  }
  else
  {
    obj = create_object( get_obj_index( OBJ_VNUM_COINS ), 0 );
    mprintf(sizeof(buf), buf, obj->short_descr, silver, gold );
    free_string( obj->short_descr );
    obj->short_descr    = str_dup( buf, obj->short_descr );
    obj->value[0]        = silver;
    obj->value[1]        = gold;
    obj->cost            = 100 * gold + silver;
    obj->weight            = gold / 5 + silver / 20;
  }
  return obj;
}

/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
  int number;

  if ( obj->item_type == ITEM_CONTAINER
       ||   obj->item_type == ITEM_MONEY
       ||   obj->item_type == ITEM_GEM
       ||   obj->item_type == ITEM_JEWELRY )
    number = 0;
  else
    number = 1;

  for ( obj = obj->contains; obj; obj = obj->next_content )
    number += get_obj_number( obj );

  return number;
}

/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
  int weight;
  OBJ_DATA *tobj;

  weight = obj->weight;
  for ( tobj = obj->contains; tobj; tobj = tobj->next_content )
    weight += get_obj_weight( tobj ) * WEIGHT_MULT( obj ) / 100;

  return weight;
}

int get_true_weight(OBJ_DATA *obj)
{
  int weight;

  weight = obj->weight;
  for ( obj = obj->contains; obj; obj = obj->next_content )
    weight += get_obj_weight( obj );

  return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
  OBJ_DATA *obj;

  if ( !pRoomIndex )
  {
    bugf( "Room Index is NULL. %s", interp_cmd );
    return FALSE;
  }

  if ( pRoomIndex->light > 0 )
    return FALSE;

  for ( obj = pRoomIndex->contents; obj; obj = obj->next_content )
  {
    if (obj->item_type == ITEM_LIGHT)
      return FALSE;
  }

  if ( IS_SET( pRoomIndex->room_flags, ROOM_DARK ) )
    return TRUE;

  if ( pRoomIndex->sector_type == SECT_INSIDE
       ||   pRoomIndex->sector_type == SECT_CITY )
    return FALSE;

  if ( weather_info.sunlight == SUN_SET
       ||   weather_info.sunlight == SUN_DARK )
    return TRUE;

  return FALSE;
}

bool is_room_owner( CHAR_DATA *ch, ROOM_INDEX_DATA *room )
{
  if ( room->owner == NULL || room->owner[0] == '\0' )
    return FALSE;

  return is_name( ch->name, room->owner );
}

/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
  CHAR_DATA *rch;
  int count;

  if ( pRoomIndex->owner && pRoomIndex->owner[0] )
    return TRUE;

  count = 0;
  for ( rch = pRoomIndex->people; rch; rch = rch->next_in_room )
    count++;

  if ( IS_SET( pRoomIndex->room_flags, ROOM_PRIVATE ) && count >= 2 )
    return TRUE;

  if ( IS_SET( pRoomIndex->room_flags, ROOM_SOLITARY ) && count >= 1 )
    return TRUE;

  if ( IS_SET( pRoomIndex->room_flags, ROOM_IMP_ONLY ) )
    return TRUE;

  return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
  int iGuild, iClass;

  if ( IS_SET( pRoomIndex->room_flags, ROOM_IMP_ONLY )
       &&   get_trust( ch ) < MAX_LEVEL )
    return FALSE;

  if ( IS_IMMORTAL( ch ) )
    return TRUE;

  if ( IS_SET( pRoomIndex->room_flags, ROOM_GODS_ONLY ) )
    return FALSE;

  if ( IS_SET( pRoomIndex->room_flags, ROOM_HEROES_ONLY )
       && ( ch->level < LEVEL_HERO ) )
    return FALSE;

  if ( IS_SET( pRoomIndex->room_flags, ROOM_NEWBIES_ONLY )
       &&   ch->level > 5
       && !( ch->clan && IS_SET( ch->clan->clan_flags, CLAN_GWYLIAD ) ) )
    return FALSE;

  for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
  {
    for ( iGuild = 0; iGuild < MAX_GUILD; iGuild ++ )
    {
      if ( iClass != ch->gameclass
           &&   pRoomIndex->vnum == class_table[iClass].guild[iGuild] )
        return FALSE;
    }
  }

  if ( pRoomIndex->clan_name
       &&   !IS_NULLSTR( pRoomIndex->clan_name ) )
  {
    if ( !ch->clan )
      return FALSE;

    if ( str_cmp( ch->clan->name, pRoomIndex->clan_name  ) )
      return FALSE;
  }
  return TRUE;
}

/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
  /* RT changed so that WIZ_INVIS has levels */
  if ( ch == victim )
    return TRUE;

  if ( ch->in_room == NULL )
    return FALSE;

  if ( get_trust( ch ) < victim->invis_level )
    return FALSE;

  if ( get_trust( ch ) < victim->incog_level
       &&   ch->in_room != victim->in_room )
    return FALSE;

  if ( ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) )
       ||   (  IS_NPC( ch ) && IS_IMMORTAL( ch ) ) )
    return TRUE;

  if ( IS_AFFECTED( ch, AFF_BLIND ) )
    return FALSE;

  if ( room_is_dark( ch->in_room )
       &&   !IS_AFFECTED( ch, AFF_INFRARED )
       &&   !IS_AFFECTED( ch, AFF_DARK_VISION ) )
    return FALSE;

  /*
   * INVISIBLE can be seen w/ detect_hidden or those in the same clan
   */
  if (  IS_AFFECTED( victim, AFF_INVISIBLE )
        && ( !IS_AFFECTED( ch, AFF_DETECT_INVIS ) && !is_same_clan( ch, victim ) ) )
    return FALSE;

  /*
   * SNEAK first checks detect_hidden or if they are the same clan,
   * then runs through the chances

   * Sneak is a boring skill, why let it matter in can_see. Can add another
   * more advanced sneak if we want to use this. So, I'll remove it for now.
   * RWL 1-21-07
   */
  /*if (  IS_AFFECTED( victim, AFF_SNEAK )
  && ( !IS_AFFECTED( ch, AFF_DETECT_HIDDEN ) && !is_same_clan( ch, victim ) )
  &&   victim->fighting == NULL )
  {
      int chance;

      chance = get_skill( victim, gsn_sneak );
      chance += get_curr_stat( victim, STAT_DEX ) * 3 / 2;
      chance -= get_curr_stat( ch, STAT_INT ) * 2;
      chance -= ch->level - victim->level * 3 / 2;

    if ( number_percent() < chance )
        return FALSE;
  }*/

  /*
   * HIDE is seen by those with detect hidden or people in the same clan.
   */
  if ( IS_AFFECTED( victim, AFF_HIDE )
       && !is_same_clan( ch, victim )
       &&  (victim->fighting == NULL ) )
  {
    if ( !IS_AFFECTED( ch, AFF_DETECT_HIDDEN )  )
      return FALSE;

    if (get_skill(victim, gsn_cloak_of_assassins) > number_percent())
      return FALSE;
  }
#if MEMDEBUG
  memdebug_check( ch, "can see" );
#endif

  return TRUE;
}

/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
  if ( obj->carried_by )
  {
    if ( IS_SHOPKEEPER(ch) // keeper can always see own items
         && (obj->carried_by == ch) )
      return TRUE;
    if ( IS_WIZINVIS( obj->carried_by, ch ) )
      return FALSE;
    if ( ch->in_room == NULL || obj->carried_by->in_room == NULL )
      return FALSE;
    if ( get_trust( ch ) < obj->carried_by->incog_level
         &&   ch->in_room != obj->carried_by->in_room )
      return FALSE;
  }

  if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) )
    return TRUE;

  if ( IS_SET( obj->extra_flags, ITEM_VIS_DEATH ) )
    return FALSE;

  if ( IS_AFFECTED( ch, AFF_BLIND ) && obj->item_type != ITEM_POTION )
    return FALSE;

  if ( obj->item_type == ITEM_LIGHT && obj->value[2] )
    return TRUE;

  if ( IS_SET( obj->extra_flags, ITEM_INVIS )
       &&  !IS_AFFECTED( ch, AFF_DETECT_INVIS ) )
    return FALSE;

  if ( IS_OBJ_STAT( obj, ITEM_GLOW ) )
    return TRUE;

  if ( room_is_dark( ch->in_room )
       && !IS_AFFECTED( ch, AFF_DARK_VISION ) )
//  ||   IS_SET(race_table[ch->race].aff, AFF_DARK_VISION) ) )
//  ||   ( race_table[ch->race].aff != AFF_DARK_VISION) ) )
    return FALSE;

  return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
  if ( !IS_SET( obj->extra_flags, ITEM_NODROP ) )
    return TRUE;

  if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
    return TRUE;

  return FALSE;
}


/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( int location )
{
  switch ( location )
  {
    case APPLY_NONE:        return "none";
    case APPLY_STR:        return "strength";
    case APPLY_DEX:        return "dexterity";
    case APPLY_INT:        return "intelligence";
    case APPLY_WIS:        return "wisdom";
    case APPLY_CON:        return "constitution";
    case APPLY_SEX:        return "sex";
    case APPLY_CLASS:        return "class";
    case APPLY_LEVEL:        return "level";
    case APPLY_AGE:        return "age";
    case APPLY_MANA:        return "mana";
    case APPLY_HIT:        return "hp";
    case APPLY_MOVE:        return "moves";
    case APPLY_GOLD:        return "gold";
    case APPLY_EXP:        return "experience";
    case APPLY_AC:        return "armor class";
    case APPLY_HITROLL:        return "hit roll";
    case APPLY_DAMROLL:        return "damage roll";
    case APPLY_SAVES:        return "saves";
    case APPLY_SAVING_ROD:    return "save vs rod";
    case APPLY_SAVING_PETRI:    return "save vs petrification";
    case APPLY_SAVING_BREATH:    return "save vs breath";
    case APPLY_SAVING_SPELL:    return "save vs spell";
    case APPLY_WEIGHT:        return "weight";
    case APPLY_HEIGHT:        return "height";
    case APPLY_SPELL_AFFECT:    return "none";
  }

  bugf("Affect_location_error: Location = %d %s\n",
       location, interp_cmd);
  return "(unknown)";
}



/*
 * Return ascii name of an affect bit vector.
 */
char *affect_bit_name( int64 vector )
{
  static char buf[512];

  buf[0] = '\0';
  if ( vector & AFF_BLIND         ) strcat( buf, " blind"         );
  if ( vector & AFF_INVISIBLE     ) strcat( buf, " invisible"     );
  if ( vector & AFF_DETECT_EVIL   ) strcat( buf, " detect_evil"   );
  if ( vector & AFF_DETECT_GOOD   ) strcat( buf, " detect_good"   );
  if ( vector & AFF_DETECT_INVIS  ) strcat( buf, " detect_invis"  );
  if ( vector & AFF_DETECT_MAGIC  ) strcat( buf, " detect_magic"  );
  if ( vector & AFF_DETECT_HIDDEN ) strcat( buf, " detect_hidden" );
  if ( vector & AFF_SANCTUARY     ) strcat( buf, " sanctuary"     );
  if ( vector & AFF_AQUA_ALBEDO   ) strcat( buf, " aqua_albedo"   );
  if ( vector & AFF_AQUA_REGIA    ) strcat( buf, " aqua_regia"    );
  if ( vector & AFF_FAERIE_FIRE   ) strcat( buf, " faerie_fire"   );
  if ( vector & AFF_INFRARED      ) strcat( buf, " infrared"      );
  if ( vector & AFF_CURSE         ) strcat( buf, " curse"         );
  if ( vector & AFF_POISON        ) strcat( buf, " poison"        );
  if ( vector & AFF_PROTECT_EVIL  ) strcat( buf, " prot_evil"     );
  if ( vector & AFF_PROTECT_GOOD  ) strcat( buf, " prot_good"     );
  if ( vector & AFF_SLEEP         ) strcat( buf, " sleep"         );
  if ( vector & AFF_SNEAK         ) strcat( buf, " sneak"         );
  if ( vector & AFF_HIDE          ) strcat( buf, " hide"          );
  if ( vector & AFF_CHARM         ) strcat( buf, " charm"         );
  if ( vector & AFF_FLYING        ) strcat( buf, " flying"        );
  if ( vector & AFF_PASS_DOOR     ) strcat( buf, " pass_door"     );
  if ( vector & AFF_BERSERK          ) strcat( buf, " berserk"          );
  if ( vector & AFF_CALM            ) strcat( buf, " calm"            );
  if ( vector & AFF_HASTE            ) strcat( buf, " haste"            );
  if ( vector & AFF_SLOW          ) strcat( buf, " slow"          );
  if ( vector & AFF_PLAGUE          ) strcat( buf, " plague"           );
  if ( vector & AFF_DARK_VISION   ) strcat( buf, " dark_vision"   );
  if ( vector & AFF_ALACRITY      ) strcat( buf, " alacrity"       );
  //if ( vector & AFF_NIRVANA       ) strcat( buf, " nirvana"       );
  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *affect2_bit_name( int64 vector )
{
  static char buf[512];

  buf[0] = '\0';
  if ( vector & AFF2_NIRVANA     ) strcat( buf, " nirvana"        );
  if ( vector & AFF2_FADE_OUT    ) strcat( buf, " fade_out"       );
  if ( vector & AFF2_PROTECT_NEUTRAL ) strcat( buf, " protect_neutral"    );
  if ( vector & AFF2_INVUN       ) strcat( buf, " globe"          );

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/*
 * Return ascii name of an spell affect bit vector.
 */
char *spell_affect_bit_name( int64 vector )
{
  static char buf[512];

  buf[0] = '\0';
  if ( vector & SAFF_MARTYR            ) strcat( buf, " martyr"         );
  if ( vector & SAFF_WALK_ON_WATER     ) strcat( buf, " waterwalk" );
  if ( vector & SAFF_DETER             ) strcat( buf, " deter"   );
  if ( vector & SAFF_INVUN             ) strcat( buf, " invunerability");
  if ( vector & SAFF_FARSIGHT          ) strcat( buf, " farsight"  );
  if ( vector & SAFF_MAGESHIELD        ) strcat( buf, " mageshield"  );
  if ( vector & SAFF_REPLENISH         ) strcat( buf, " replenish" );
  if ( vector & SAFF_WARRIORSHIELD     ) strcat( buf, " warriorshield"     );
  if ( vector & SAFF_YAWN              ) strcat( buf, " yawn"   );
  if ( vector & SAFF_HICCUP            ) strcat( buf, " hiccup"      );
  if ( vector & SAFF_GHOST             ) strcat( buf, " ghost"         );
  //if ( vector & SAFF_NIRVANA           ) strcat( buf, " nirvana"         );
  if ( vector & SAFF_PROTECT_NEUTRAL   ) strcat( buf, " protect neutral"     );
  if ( vector & SAFF_ADRENALIZE        ) strcat( buf, " adrenalize"         );
  return ( buf[0] != '\0' ) ? buf+1 : "none";
}



/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name( int64 extra_flags )
{
  static char buf[512];

  buf[0] = '\0';
  if ( extra_flags & ITEM_GLOW          ) strcat( buf, " glow"          );
  if ( extra_flags & ITEM_HUM           ) strcat( buf, " hum"           );
  if ( extra_flags & ITEM_DARK          ) strcat( buf, " dark"          );
  if ( extra_flags & ITEM_LOCK          ) strcat( buf, " lock"          );
  if ( extra_flags & ITEM_EVIL          ) strcat( buf, " evil"          );
  if ( extra_flags & ITEM_INVIS         ) strcat( buf, " invis"         );
  if ( extra_flags & ITEM_MAGIC         ) strcat( buf, " magic"         );
  if ( extra_flags & ITEM_NODROP        ) strcat( buf, " nodrop"        );
  if ( extra_flags & ITEM_BLESS         ) strcat( buf, " bless"         );
  if ( extra_flags & ITEM_ANTI_GOOD     ) strcat( buf, " anti-good"     );
  if ( extra_flags & ITEM_ANTI_EVIL     ) strcat( buf, " anti-evil"     );
  if ( extra_flags & ITEM_ANTI_NEUTRAL  ) strcat( buf, " anti-neutral"  );
  if ( extra_flags & ITEM_NOREMOVE      ) strcat( buf, " noremove"      );
  if ( extra_flags & ITEM_INVENTORY     ) strcat( buf, " inventory"     );
  if ( extra_flags & ITEM_NOPURGE          ) strcat( buf, " nopurge"          );
  if ( extra_flags & ITEM_VIS_DEATH        ) strcat( buf, " vis_death"        );
  if ( extra_flags & ITEM_ROT_DEATH        ) strcat( buf, " rot_death"        );
  if ( extra_flags & ITEM_NOLOCATE        ) strcat( buf, " no_locate"        );
  if ( extra_flags & ITEM_SELL_EXTRACT  ) strcat( buf, " sell_extract"  );
  if ( extra_flags & ITEM_BURN_PROOF      ) strcat( buf, " burn_proof"      );
  if ( extra_flags & ITEM_NOUNCURSE        ) strcat( buf, " no_uncurse"      );
  if ( extra_flags & ITEM_MELT_DROP        ) strcat( buf, " meltdrop"        );
  if ( extra_flags & ITEM_RESTRING        ) strcat( buf, " restring"        );
  if ( extra_flags & ITEM_CLANEQ        ) strcat( buf, " claneq"        );
  if ( extra_flags & ITEM_DONATION_PIT  ) strcat( buf, " pit"           );
  if ( extra_flags & ITEM_PLAYER_HOUSE  ) strcat( buf, " player_house"  );
  if ( extra_flags & ITEM_BLESSED_SHIELD) strcat( buf, " blessed_shield");
  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/* return ascii name of an act vector */
char *act_bit_name( int64 act_flags )
{
  static char buf[512];

  buf[0] = '\0';

  if ( IS_SET( act_flags, ACT_IS_NPC ) )
  {
    strcat(buf," npc");
    if ( act_flags & ACT_SENTINEL         ) strcat( buf, " sentinel"      );
    if ( act_flags & ACT_SCAVENGER        ) strcat( buf, " scavenger"     );
    if ( act_flags & ACT_AGGRESSIVE        ) strcat( buf, " aggressive"    );
    if ( act_flags & ACT_BANKER            ) strcat( buf, " banker"        );
    if ( act_flags & ACT_STAY_AREA        ) strcat( buf, " stay_area"     );
    if ( act_flags & ACT_WIMPY            ) strcat( buf, " wimpy"         );
    if ( act_flags & ACT_PET                ) strcat( buf, " pet"           );
    if ( act_flags & ACT_TRAIN            ) strcat( buf, " train"         );
    if ( act_flags & ACT_PRACTICE            ) strcat( buf, " practice"      );
    if ( act_flags & ACT_DEALER            ) strcat( buf, " dealer"        );
    if ( act_flags & ACT_UNDEAD            ) strcat( buf, " undead"        );
    if ( act_flags & ACT_CLERIC           ) strcat( buf, " cleric"        );
    if ( act_flags & ACT_MAGE                ) strcat( buf, " mage"          );
    if ( act_flags & ACT_THIEF            ) strcat( buf, " thief"         );
    if ( act_flags & ACT_WARRIOR            ) strcat( buf, " warrior"       );
    if ( act_flags & ACT_NOALIGN          ) strcat( buf, " no_align"      );
    if ( act_flags & ACT_NOPURGE            ) strcat( buf, " no_purge"      );
    if ( act_flags & ACT_NOGHOST            ) strcat( buf, " no_ghost"      );
    /* Added by Sartan 12/18/00 */
    if ( act_flags & ACT_IS_HEALER        ) strcat( buf, " healer"        );
    if ( act_flags & ACT_IS_CHANGER       ) strcat( buf, " changer"       );
    if ( act_flags & ACT_GAIN                ) strcat( buf, " skill_train"   );
    if ( act_flags & ACT_UPDATE_ALWAYS    ) strcat( buf, " update_always" );
    if ( act_flags & ACT_FORGER           ) strcat( buf, " forger"        );
    /* RH 2/1/98 forger*/
    if ( act_flags & ACT_NOQUEST          ) strcat( buf, " no_quest"      );
    /*Added by Sartan 12/3/01*/
    if ( act_flags & ACT_RECALLS          ) strcat( buf, " recalls"       );
    /* Added by Merak 11/11/06 */
  }
  else
  {
    strcat(buf," player");
    if (act_flags & PLR_AUTOASSIST    ) strcat(buf, " autoassist");
    if (act_flags & PLR_AUTOEXIT    ) strcat(buf, " autoexit");
    if (act_flags & ACT_AGGRESSIVE    ) strcat(buf, " aggressive");
    if (act_flags & PLR_AUTOLOOT    ) strcat(buf, " autoloot");
    if (act_flags & PLR_AUTOSAC    ) strcat(buf, " autosac");
    if (act_flags & PLR_AUTOGOLD    ) strcat(buf, " autogold");
    if (act_flags & PLR_AUTOSPLIT    ) strcat(buf, " autosplit");
    if (act_flags & PLR_HOLYLIGHT    ) strcat(buf, " holy_light");
    if (act_flags & PLR_CANLOOT    ) strcat(buf, " loot_corpse");
    if (act_flags & PLR_MORGUE    ) strcat(buf, " morgue_corpse");
    if (act_flags & PLR_NOSUMMON    ) strcat(buf, " no_summon");
    if (act_flags & PLR_NOGATE    ) strcat(buf, " no_gate");
    if (act_flags & PLR_NOFOLLOW    ) strcat(buf, " no_follow");
    if (act_flags & PLR_FREEZE    ) strcat(buf, " frozen");
    if (act_flags & PLR_COLOUR    ) strcat(buf, " colour");
    if (act_flags & PLR_THIEF            ) strcat(buf, " thief");
    if (act_flags & PLR_KILLER    ) strcat(buf, " killer");
    if (act_flags & PLR_TWIT        ) strcat(buf, " twit");
    if (act_flags & PLR_VIOLENT        ) strcat(buf, " violent");
    if (act_flags & PLR_NOCANCEL    ) strcat(buf, " no_cancellation");
    if (act_flags & PLR_QUESTING    ) strcat(buf, " questing");
    if (act_flags & PLR_AUTODONATE ) strcat(buf, " autodonate");
    if (act_flags & PLR_AUTOHUNT ) strcat(buf, " autohunt");
    if (act_flags & PLR_FISHING ) strcat(buf, " fishing");
  }
  return ( buf[0] ) ? buf + 1 : "none";
}

/* return ascii name of an act2 vector  -- Oct, 2003 Robert Leonard */
char *act2_bit_name( int64 act2_flags )
{
  static char buf[512];

  buf[0] = '\0';
  if ( act2_flags & ACT2_RFRAG      ) strcat(buf, " ruby_fragment"    );
  if ( act2_flags & ACT2_SFRAG      ) strcat(buf, " sapphire_fragment");
  if ( act2_flags & ACT2_EFRAG      ) strcat(buf, " emerald_fragment" );
  if ( act2_flags & ACT2_DFRAG      ) strcat(buf, " diamond_fragment" );
  if ( act2_flags & ACT2_AFRAG      ) strcat(buf, " any_fragment"     );
  if ( act2_flags & ACT2_FRAG_SELL  ) strcat(buf, " frag_seller"      );
  if ( act2_flags & ACT2_IDENTIFIER ) strcat(buf, " identifier"       );
  if ( act2_flags & ACT2_SWITCHER   ) strcat(buf, " switcher"         );
  if ( act2_flags & ACT2_NOHAGGLE   ) strcat(buf, " nohaggle"         );
  if ( act2_flags & ACT2_MOUNTABLE  ) strcat(buf, " mountable"        );
  if ( act2_flags & ACT2_NOWANDEROFF) strcat(buf, " no_wander_off"    );
  if ( act2_flags & ACT2_BLOODLESS  ) strcat(buf, " bloodless"        );

  return ( buf[0] ) ? buf + 1 : "none";
}

char *comm_bit_name(int64 comm_flags)
{
  static char buf[512];

  buf[0] = '\0';

  if (comm_flags & COMM_COMPACT    ) strcat(buf, " compact");
  if (comm_flags & COMM_BRIEF        ) strcat(buf, " brief");
  if (comm_flags & COMM_PROMPT    ) strcat(buf, " prompt");
  if (comm_flags & COMM_COMBINE    ) strcat(buf, " combine");
  if (comm_flags & COMM_AFK        ) strcat(buf, " AFK");
  if (comm_flags & COMM_NEWSFAERIE    ) strcat(buf, " newsfaerie");
  if (comm_flags & COMM_REPLY_LOCK    ) strcat(buf, " replylock");

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}
char *chan_bit_name(int64 chan_flags)
{
  static char buf[512];

  buf[0] = '\0';

  if (chan_flags & CHANNEL_QUIET        ) strcat(buf, " quiet");
  if (chan_flags & CHANNEL_DEAF        ) strcat(buf, " deaf");
  if (chan_flags & CHANNEL_NOWIZ        ) strcat(buf, " no_wiz");
  if (chan_flags & CHANNEL_NOAUCTION    ) strcat(buf, " no_auction");
  if (chan_flags & CHANNEL_NOGOSSIP    ) strcat(buf, " no_gossip");
  if (chan_flags & CHANNEL_NOQUESTION    ) strcat(buf, " no_question");
  if (chan_flags & CHANNEL_NOMUSIC    ) strcat(buf, " no_music");
  if (chan_flags & CHANNEL_NOQUOTE    ) strcat(buf, " no_quote");
  if (chan_flags & CHANNEL_NOEMOTE    ) strcat(buf, " no_emote");
  if (chan_flags & CHANNEL_NOOOC        ) strcat(buf, " no_ooc");
  if (chan_flags & CHANNEL_NOINFO    ) strcat(buf, " no_emote");
  if (chan_flags & CHANNEL_NOSHOUT    ) strcat(buf, " no_shout");
  if (chan_flags & CHANNEL_NOTELL    ) strcat(buf, " no_tell");
  if (chan_flags & CHANNEL_NOWAR        ) strcat(buf, " no_wartalk");
  if (chan_flags & CHANNEL_NOPOLITIC    ) strcat(buf, " no_political");
  if (chan_flags & CHANNEL_ALL    ) strcat(buf, " ALL");
  if (chan_flags & CHANNEL_NEWBIE    ) strcat(buf, " nonewbie");


  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *pen_bit_name(int64 pen_flags)
{
  static char buf[512];

  buf[0] = '\0';

  if (pen_flags & PEN_NOCHANNELS    ) strcat(buf, " no_channels");
  if (pen_flags & PEN_SNOOP_PROOF    ) strcat(buf, " Snoop Proof");
  if (pen_flags & PEN_NOTE    ) strcat(buf, " Note denied");


  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *plr2_bit_name(int64 plr2_flags)
{
  static char buf[512];

  buf[0] = '\0';

  if (plr2_flags & PLR2_STATS        ) strcat(buf, " statistics");
  /*if (plr2_flags & PLR2_NOQUEST        ) strcat(buf, " noquest");*/
  if (plr2_flags & PLR2_TELNET_GA    ) strcat(buf, " telnet_ga");

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *imm_bit_name(int64 imm_flags)
{
  static char buf[512];

  buf[0] = '\0';

  if (imm_flags & IMM_SUMMON        ) strcat(buf, " summon");
  if (imm_flags & IMM_CHARM        ) strcat(buf, " charm");
  if (imm_flags & IMM_MAGIC        ) strcat(buf, " magic");
  if (imm_flags & IMM_WEAPON        ) strcat(buf, " weapon");
  if (imm_flags & IMM_BASH        ) strcat(buf, " blunt");
  if (imm_flags & IMM_PIERCE        ) strcat(buf, " piercing");
  if (imm_flags & IMM_SLASH        ) strcat(buf, " slashing");
  if (imm_flags & IMM_FIRE        ) strcat(buf, " fire");
  if (imm_flags & IMM_COLD        ) strcat(buf, " cold");
  if (imm_flags & IMM_LIGHTNING    ) strcat(buf, " lightning");
  if (imm_flags & IMM_ACID        ) strcat(buf, " acid");
  if (imm_flags & IMM_POISON        ) strcat(buf, " poison");
  if (imm_flags & IMM_NEGATIVE    ) strcat(buf, " negative");
  if (imm_flags & IMM_HOLY        ) strcat(buf, " holy");
  if (imm_flags & IMM_ENERGY        ) strcat(buf, " energy");
  if (imm_flags & IMM_MENTAL        ) strcat(buf, " mental");
  if (imm_flags & IMM_DISEASE    ) strcat(buf, " disease");
  if (imm_flags & IMM_DROWNING    ) strcat(buf, " drowning");
  if (imm_flags & IMM_LIGHT        ) strcat(buf, " light");
  if (imm_flags & IMM_IRON        ) strcat(buf, " iron");
  if (imm_flags & IMM_WOOD        ) strcat(buf, " wood");
  if (imm_flags & IMM_SILVER    )     strcat(buf, " silver");

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *wear_bit_name(int64 wear_flags)
{
  static char buf[512];

  buf [0] = '\0';
  if (wear_flags & ITEM_TAKE            ) strcat(buf, " take");
  if (wear_flags & ITEM_WEAR_FINGER        ) strcat(buf, " finger");
  if (wear_flags & ITEM_WEAR_NECK        ) strcat(buf, " neck");
  if (wear_flags & ITEM_WEAR_BODY        ) strcat(buf, " body");
  if (wear_flags & ITEM_WEAR_HEAD        ) strcat(buf, " head");
  if (wear_flags & ITEM_WEAR_LEGS        ) strcat(buf, " legs");
  if (wear_flags & ITEM_WEAR_FEET        ) strcat(buf, " feet");
  if (wear_flags & ITEM_WEAR_HANDS        ) strcat(buf, " hands");
  if (wear_flags & ITEM_WEAR_ARMS        ) strcat(buf, " arms");
  if (wear_flags & ITEM_WEAR_SHIELD        ) strcat(buf, " shield");
  if (wear_flags & ITEM_WEAR_ABOUT        ) strcat(buf, " about");
  if (wear_flags & ITEM_WEAR_WAIST        ) strcat(buf, " waist");
  if (wear_flags & ITEM_WEAR_WRIST        ) strcat(buf, " wrist");
  if (wear_flags & ITEM_WIELD            ) strcat(buf, " wield");
  if (wear_flags & ITEM_HOLD            ) strcat(buf, " hold");
  if (wear_flags & ITEM_WEAR_BAG        ) strcat(buf, " bag");
  if (wear_flags & ITEM_WEAR_BACK       ) strcat(buf, " back");
  if (wear_flags & ITEM_NO_SAC            ) strcat(buf, " nosac");
  if (wear_flags & ITEM_WEAR_FLOAT        ) strcat(buf, " float");
  if (wear_flags & ITEM_WEAR_EAR        ) strcat(buf, " ear");
  if (wear_flags & ITEM_WEAR_LAPEL      ) strcat(buf, " lapel");
  if (wear_flags & ITEM_WEAR_EYE        ) strcat(buf, " eye");
  if (wear_flags & ITEM_WEAR_RFOOT      ) strcat(buf, " rfoot");
  if (wear_flags & ITEM_WEAR_CREST      ) strcat(buf, " crest");

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *form_bit_name(int64 form_flags)
{
  static char buf[512];

  buf[0] = '\0';
  if (form_flags & FORM_POISON    ) strcat(buf, " poison");
  else if (form_flags & FORM_EDIBLE    ) strcat(buf, " edible");
  if (form_flags & FORM_MAGICAL    ) strcat(buf, " magical");
  if (form_flags & FORM_INSTANT_DECAY    ) strcat(buf, " instant_rot");
  if (form_flags & FORM_OTHER        ) strcat(buf, " other");
  if (form_flags & FORM_ANIMAL    ) strcat(buf, " animal");
  if (form_flags & FORM_SENTIENT    ) strcat(buf, " sentient");
  if (form_flags & FORM_UNDEAD    ) strcat(buf, " undead");
  if (form_flags & FORM_CONSTRUCT    ) strcat(buf, " construct");
  if (form_flags & FORM_MIST        ) strcat(buf, " mist");
  if (form_flags & FORM_INTANGIBLE    ) strcat(buf, " intangible");
  if (form_flags & FORM_BIPED        ) strcat(buf, " biped");
  if (form_flags & FORM_CENTAUR    ) strcat(buf, " centaur");
  if (form_flags & FORM_INSECT    ) strcat(buf, " insect");
  if (form_flags & FORM_SPIDER    ) strcat(buf, " spider");
  if (form_flags & FORM_CRUSTACEAN    ) strcat(buf, " crustacean");
  if (form_flags & FORM_WORM        ) strcat(buf, " worm");
  if (form_flags & FORM_BLOB        ) strcat(buf, " blob");
  if (form_flags & FORM_MAMMAL    ) strcat(buf, " mammal");
  if (form_flags & FORM_BIRD        ) strcat(buf, " bird");
  if (form_flags & FORM_REPTILE    ) strcat(buf, " reptile");
  if (form_flags & FORM_SNAKE        ) strcat(buf, " snake");
  if (form_flags & FORM_DRAGON    ) strcat(buf, " dragon");
  if (form_flags & FORM_AMPHIBIAN    ) strcat(buf, " amphibian");
  if (form_flags & FORM_FISH        ) strcat(buf, " fish");
  if (form_flags & FORM_COLD_BLOOD     ) strcat(buf, " cold_blooded");

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *part_bit_name(int64 part_flags)
{
  static char buf[512];

  buf[0] = '\0';
  if (part_flags & PART_HEAD        ) strcat(buf, " head");
  if (part_flags & PART_ARMS        ) strcat(buf, " arms");
  if (part_flags & PART_LEGS        ) strcat(buf, " legs");
  if (part_flags & PART_HEART        ) strcat(buf, " heart");
  if (part_flags & PART_BRAINS    ) strcat(buf, " brains");
  if (part_flags & PART_GUTS        ) strcat(buf, " guts");
  if (part_flags & PART_HANDS        ) strcat(buf, " hands");
  if (part_flags & PART_FEET        ) strcat(buf, " feet");
  if (part_flags & PART_FINGERS    ) strcat(buf, " fingers");
  if (part_flags & PART_EAR        ) strcat(buf, " ears");
  if (part_flags & PART_EYE        ) strcat(buf, " eyes");
  if (part_flags & PART_LONG_TONGUE    ) strcat(buf, " long_tongue");
  if (part_flags & PART_EYESTALKS    ) strcat(buf, " eyestalks");
  if (part_flags & PART_TENTACLES    ) strcat(buf, " tentacles");
  if (part_flags & PART_FINS        ) strcat(buf, " fins");
  if (part_flags & PART_WINGS        ) strcat(buf, " wings");
  if (part_flags & PART_TAIL        ) strcat(buf, " tail");
  if (part_flags & PART_CLAWS        ) strcat(buf, " claws");
  if (part_flags & PART_FANGS        ) strcat(buf, " fangs");
  if (part_flags & PART_HORNS        ) strcat(buf, " horns");
  if (part_flags & PART_SCALES    ) strcat(buf, " scales");
  if (part_flags & PART_TALONS  ) strcat(buf, " talons");

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *weapon_bit_name(int64 weapon_flags)
{
  static char buf[512];

  buf[0] = '\0';
  if (weapon_flags & WEAPON_FLAMING    ) strcat(buf, " flaming");
  if (weapon_flags & WEAPON_FROST    ) strcat(buf, " frost");
  if (weapon_flags & WEAPON_VAMPIRIC    ) strcat(buf, " vampiric");
  if (weapon_flags & WEAPON_SHARP    ) strcat(buf, " sharp");
  if (weapon_flags & WEAPON_VORPAL    ) strcat(buf, " vorpal");
  if (weapon_flags & WEAPON_TWO_HANDS ) strcat(buf, " two-handed");
  if (weapon_flags & WEAPON_SHOCKING     ) strcat(buf, " shocking");
  if (weapon_flags & WEAPON_POISON    ) strcat(buf, " poison");
  if (weapon_flags & WEAPON_CLAN    ) strcat(buf, " clan");
  if (weapon_flags & WEAPON_MANA_DRAIN    ) strcat(buf, " manadrain");
  if (weapon_flags & WEAPON_HOLY ) strcat( buf, " holy" );
  if (weapon_flags & WEAPON_UNHOLY ) strcat( buf, " unholy" );

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *cont_bit_name( int64 cont_flags)
{
  static char buf[512];

  buf[0] = '\0';

  if (cont_flags & CONT_CLOSEABLE    ) strcat(buf, " closable");
  if (cont_flags & CONT_PICKPROOF    ) strcat(buf, " pickproof");
  if (cont_flags & CONT_CLOSED    ) strcat(buf, " closed");
  if (cont_flags & CONT_LOCKED    ) strcat(buf, " locked");
  //if (cont_flags & CONT_FENCED  ) strcat(buf, " fenced");

  return (buf[0] != '\0' ) ? buf+1 : "none";
}


char *off_bit_name(int64 off_flags)
{
  static char buf[512];

  buf[0] = '\0';

  if (off_flags & OFF_AREA_ATTACK    ) strcat(buf, " area attack");
  if (off_flags & OFF_BACKSTAB    ) strcat(buf, " backstab");
  if (off_flags & OFF_BASH        ) strcat(buf, " bash");
  if (off_flags & OFF_BERSERK        ) strcat(buf, " berserk");
  if (off_flags & OFF_DISARM        ) strcat(buf, " disarm");
  if (off_flags & OFF_DODGE        ) strcat(buf, " dodge");
  if (off_flags & OFF_FADE        ) strcat(buf, " fade");
  if (off_flags & OFF_FAST        ) strcat(buf, " fast");
  if (off_flags & OFF_KICK        ) strcat(buf, " kick");
  if (off_flags & OFF_KICK_DIRT          ) strcat(buf, " kick_dirt");
  if (off_flags & OFF_PARRY        ) strcat(buf, " parry");
  if (off_flags & OFF_RESCUE        ) strcat(buf, " rescue");
  if (off_flags & OFF_TAIL        ) strcat(buf, " tail");
  if (off_flags & OFF_TRIP        ) strcat(buf, " trip");
  if (off_flags & OFF_CRUSH        ) strcat(buf, " crush");
  if (off_flags & ASSIST_ALL        ) strcat(buf, " assist_all");
  if (off_flags & ASSIST_ALIGN            ) strcat(buf, " assist_align");
  if (off_flags & ASSIST_RACE        ) strcat(buf, " assist_race");
  if (off_flags & ASSIST_PLAYERS    ) strcat(buf, " assist_players");
  if (off_flags & ASSIST_GUARD            ) strcat(buf, " assist_guard");
  if (off_flags & ASSIST_VNUM        ) strcat(buf, " assist_vnum");
  if (off_flags & OFF_FLYING            ) strcat(buf, " flying");
  if (off_flags & ASSIST_MOBILE         ) strcat(buf, " assist_mobile");
  if (off_flags & ASSIST_AREA           ) strcat(buf, " assist_area");

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

int roll_stat( CHAR_DATA *ch, int stat )
{
  int temp,low,high;

  high = pc_race_table[ch->race].max_stats[stat] - 2;
  low = pc_race_table[ch->race].stats[stat] + 1;
  temp = (number_range(low,high));
  /*
    if (class_table[ch->class].attr_prime == stat)
    if (ch->race == race_lookup("human"))
    temp += 3;
    else
    temp += 2;
  */
  return UMIN(temp,high);
}

bool melt_drop(CHAR_DATA *ch, OBJ_DATA *obj)
{
  if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
  {
    act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
    act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
    extract_obj(obj);
    return(TRUE);
  }
  else
    return(FALSE);
}

bool is_in_pk_range(CHAR_DATA *ch,CHAR_DATA *victim)
{
  CHAR_DATA *gch, *vch;

  /*  int temp_level=0;*/
  if (ch == NULL
      ||  victim == NULL
      ||  IS_GHOST(ch)
      ||  IS_GHOST(victim)
      ||  IS_NPC(ch) || IS_NPC(victim)
      ||  IS_IMMORTAL(ch)
      ||  IS_IMMORTAL(victim))
    return FALSE;

  if (IS_NPC(victim) )
    return TRUE;

  if ( !( ch->clan ) //is_clan(ch)
       || ch->pcdata->old_char
       || victim->pcdata->old_char
       || !( victim->clan ) ) //!is_clan(victim))
    return FALSE;

  if ( ( (victim->clan == ch->clan) && (ch!=victim) )
       &&   !IS_SET( ch->clan->clan_flags, CLAN_INDEPENDENT ) )
    return FALSE;

  if ( IS_SET( ch->clan->clan_flags, CLAN_PEACEFUL )
       ||   IS_SET( victim->clan->clan_flags, CLAN_PEACEFUL ) )
    return FALSE;

  /* Check charmie chain.  If victim or victim master is in same clan as attacker, no attack. */
  gch = ch;
  vch = victim;
  while ( (IS_PET(gch) || IS_AFFECTED(gch, AFF_CHARM)) && gch->master != NULL )
    gch = gch->master;
  while ( (IS_PET(vch) || IS_AFFECTED(vch, AFF_CHARM)) && vch->master != NULL )
    vch = vch->master;

  if ( (gch->clan == vch->clan
        ||    gch->clan == victim->clan )
       &&    !IS_SET( gch->clan->clan_flags, CLAN_INDEPENDENT ) )
    return FALSE;

  if ( IS_SET( ch->clan->clan_flags, CLAN_LAW )
       &&   ( IS_SET( victim->act, PLR_KILLER )
              ||     IS_SET( victim->act, PLR_THIEF )
              ||     IS_SET( victim->act, PLR_VIOLENT ) ) )
    return TRUE;

  if ( IS_SET( ch->clan->clan_flags, CLAN_LAW )
       &&   ( !IS_SET( victim->act, PLR_KILLER )
              &&     !IS_SET( victim->act, PLR_THIEF )
              &&     !IS_SET( victim->act, PLR_VIOLENT ) ) )
    return FALSE;

  if ( IS_SET( ch->clan->clan_flags, CLAN_GWYLIAD )
       ||   IS_SET( victim->clan->clan_flags, CLAN_GWYLIAD ) )
    return FALSE;

  if (IS_SAFFECTED(victim, SAFF_ADRENALIZE))
    return TRUE;

  /*  if (IS_SET(victim->act,PLR_KILLER) ||
      IS_SET(victim->act,PLR_THIEF) ||
      IS_SET(victim->act,PLR_TWIT)) {
      if (ch->level <= victim->level + LEVEL_KILLRANGE)
      return TRUE;
      }
  */
  if ((ch->level >= LEVEL_HERO) && (victim->level >=LEVEL_HERO))
    return TRUE;

  /*
   * A bit simplistic. Could scale similar to what AR did. Fewer level range the
   * lower level the fighters are. R. L.
   */
  if (ch->level < 51)
  {
    if (victim->level < (int)(ch->level - 6) )
      return FALSE;
  }
  else
  {
    if (victim->level < (ch->level - (int)(ch->level * 0.10)))
      return FALSE;
  }


  return TRUE;
}

void change_sex(CHAR_DATA *ch,int mod, bool fAdd)
{
  int i=0;
  if (mod < 0)
    mod *= -1;

  if (ch->sex < 0 || ch->sex > 2)
  {
    bugf("BUG: SEX outta bounds for Character.");
    ch->sex = 1;
    ch->sex_dir = 0;
  }
  for (i=1;i<=mod; i++)
  {
    if (ch->sex == 2)
    {
      ch->sex = 1;
      if (fAdd)
        ch->sex_dir = downward_e;
      else
        ch->sex_dir = upward_e;
    }
    else if (ch->sex == 0)
    {
      ch->sex = 1;
      if (fAdd)
        ch->sex_dir = upward_e;
      else
        ch->sex_dir = downward_e;
    }
    else if (ch->sex == 1)
    {
      if (ch->sex_dir == upward_e)
      {
        if (fAdd)
          ch->sex = 2;
        else
          ch->sex =0;
      }
      else
      {
        if (fAdd)
          ch->sex = 0;
        else
          ch->sex = 2;
      }
    }
  }
#if MEMDEBUG
  memdebug_check(ch,"change sex");
#endif
}

int num_followers(CHAR_DATA *ch)
{
  CHAR_DATA *ch_next, *fch;
  int count=0;
  for ( fch = char_list; fch != NULL; fch = ch_next )
  {
    ch_next    = fch->next;

    if ( IS_AFFECTED(fch, AFF_CHARM)
         &&   fch->master == ch)
      count ++;
  }
  return(count);
}
/*
 * Similar to strncpy, but works as per strlen_color for counting characters.
 * Uses the fill character to pad the string and if terminate is TRUE, will tag
 * a terminating character on the end.
 */
char *strncpyft_color( char *s1, const char *s2, size_t n, char fill,
                       bool terminate )
{
  char *s;

  for ( s = s1; n > 0 && *s2 != '\0'; )
  {
    if ( ( *s++ = *s2++ ) != '#' )
    {
      --n;
      continue;
    }

    switch ( *s2 )
    {
      case '\0':            break;
      case 'v':
      case '-':
      case 'x':  *s++ = *s2++; --n;    break;
      default:   *s++ = *s2++;    break;
    }
  }

  for ( ; n > 0; --n )
    *s++ = fill;

  if ( terminate )
    *s = '\0';

  return s1;
}

void remove_affect(CHAR_DATA *ch, int where, long affect)
{
  AFFECT_DATA *paf, *paf_next;

  for ( paf = ch->affected; paf != NULL; paf = paf_next )
  {
    paf_next = paf->next;
    if ( paf->where == where  &&
         paf->bitvector == affect)
      affect_remove( ch, paf );
  }


  switch (where)
  {
    case TO_AFFECTS:
      REMOVE_BIT(ch->affected_by, affect);
      break;
    case TO_ACT:
      REMOVE_BIT(ch->act, affect);
      break;
    case TO_SPELL_AFFECTS:
      REMOVE_BIT(ch->spell_aff, affect);
      break;
    case TO_IMMUNE:
      REMOVE_BIT(ch->imm_flags, affect);
      break;
    case TO_RESIST:
      REMOVE_BIT(ch->res_flags, affect);
      break;
    case TO_VULN:
      REMOVE_BIT(ch->vuln_flags, affect);
      break;
    case TO_COMM:
      REMOVE_BIT(ch->comm_flags, affect);
      break;
    case TO_PENALTY:
      REMOVE_BIT(ch->pen_flags, affect);
      break;
    case TO_CHANNEL:
      REMOVE_BIT(ch->chan_flags, affect);
      break;
  }
#if MEMDEBUG
  memdebug_check(ch,"remove affect");
#endif

}
#ifdef MEMDEBUG
void memdebug_check( CHAR_DATA *ch , char *calling_func)
{

  char buf[MSL];
  if (!ch) return;
  if (IS_NPC(ch))
    return;

  if (str_cmp(ch->memdebug_name, ch->name))
  {
    mprintf(sizeof(buf), buf,
            "Memcheck: MEMDEBUG(%s) : [%s]Char -%s- name has been changed to -%s-.\n\r",
            calling_func, ch->name, ch->memdebug_name, ch->name);
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    bugf(buf);
  }

  if (str_cmp(ch->memdebug_prompt, ch->prompt))
  {
    mprintf(sizeof(buf), buf,
            "Memcheck: MEMDEBUG(%s) : [%s]Char -%s- Prompt has been changed to -%s-.\n\r",
            calling_func, ch->name,ch->memdebug_prompt, ch->prompt);
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    bugf(buf);
  }

  if (str_cmp(ch->pcdata->memdebug_title, ch->pcdata->title))
  {
    mprintf(sizeof(buf), buf,
            "Memcheck: MEMDEBUG(%s) : [%s]Char -%s- Title has been changed to -%s-.\n\r",
            calling_func, ch->name,ch->pcdata->memdebug_title, ch->pcdata->title);
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    bugf(buf);
  }
  if (str_cmp(ch->pcdata->memdebug_permtitle, ch->pcdata->permtitle))
  {
    mprintf(sizeof(buf), buf,
            "Memcheck: MEMDEBUG(%s) : [%s]Char -%s- Perm Title has been changed to -%s-.\n\r",
            calling_func, ch->name, ch->pcdata->memdebug_permtitle, ch->pcdata->permtitle);
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    bugf(buf);
  }
  if (str_cmp(ch->pcdata->memdebug_pwd, ch->pcdata->pwd))
  {
    mprintf(sizeof(buf), buf,
            "Memcheck: MEMDEBUG(%s) : [%s]Char -%s- Password has been changed to -%s-.\n\r",
            calling_func, ch->name, ch->pcdata->memdebug_pwd, ch->pcdata->pwd);
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    bugf(buf);
  }
}
#endif

void crash_fix()
{
  CHAR_DATA *vch,*vch_next;

  for ( vch = char_list; vch != NULL; vch = vch_next )
  {
    vch_next = vch->next;

    if ( !IS_NPC(vch) )
    {
      send_to_char( "You are saving yourself from doom.\n\r", vch );
      interpret( vch, "save" );
      send_to_char("System Rebooting Automatically.\n\r",vch);
    }
  }

}

/*
 * Change a 'money' obj.
 */
void change_money( OBJ_DATA *obj, int gold, int silver )
{
  OBJ_INDEX_DATA    *pObjIndex;
  char        buf[MAX_STRING_LENGTH];

  if ( obj->item_type != ITEM_MONEY )
  {
    bugf( "Change_money: not money, obj %d is item_type %d",
          obj->pIndexData->vnum, obj->item_type );
    return;
  }

  if ( gold < 0 || silver < 0 || ( gold == 0 && silver == 0 ) )
  {
    bugf( "Change_money: gold %d, silver %d", gold, silver );
    gold   = UMAX( 0, gold     );
    silver = UMAX( 0, silver );
    if ( gold == 0 && silver == 0 )
      silver = 1;
  }

  if ( gold == 0 && silver == 1 )
  {
    pObjIndex = get_obj_index( OBJ_VNUM_SILVER_ONE );
    strcpy( buf, pObjIndex->short_descr );
  }
  else if ( gold == 1 && silver == 0 )
  {
    pObjIndex = get_obj_index( OBJ_VNUM_GOLD_ONE );
    strcpy( buf, pObjIndex->short_descr );
  }
  else if ( silver == 0 )
  {
    pObjIndex = get_obj_index( OBJ_VNUM_GOLD_SOME );
    mprintf( sizeof(buf), buf, pObjIndex->short_descr, gold );
  }
  else if ( gold == 0 )
  {
    pObjIndex = get_obj_index( OBJ_VNUM_SILVER_SOME );
    mprintf( sizeof(buf), buf, pObjIndex->short_descr, silver );
  }
  else
  {
    pObjIndex = get_obj_index( OBJ_VNUM_COINS );
    mprintf( sizeof(buf), buf, pObjIndex->short_descr,
             silver, gold );
  }

  free_string( obj->name );
  obj->name        = str_dup( pObjIndex->name, obj->name );

  free_string( obj->short_descr );
  obj->short_descr    = str_dup( buf, obj->short_descr );

  free_string( obj->description );
  obj->description    = str_dup( pObjIndex->description, obj->description );

  obj->value[0] = silver;
  obj->value[1] = gold;
  obj->cost      = 100 * gold + silver;
  obj->weight   = GOLD_WEIGHT( gold ) + SILVER_WEIGHT( silver );
}


void auto_toggle( CHAR_DATA *ch, char *argument, int64 *flag, long bitvector,
                  const char *onstr, const char *offstr, char *samestr )
{
  char arg[MAX_INPUT_LENGTH];

  one_argument( argument, arg );

  if ( IS_SET( *flag, bitvector )
       &&     ( arg[0] == '\0' || !str_cmp( arg, "off" ) ) )
  {
    send_to_char( offstr, ch );
    REMOVE_BIT( *flag, bitvector );
  }
  else
    if ( !IS_SET( *flag, bitvector )
         &&     ( arg[0] == '\0' || !str_cmp( arg, "on" ) ) )
    {
      send_to_char( onstr, ch );
      SET_BIT( *flag, bitvector );
    }
    else
      printf_to_char( ch, samestr,
                      IS_SET( *flag, bitvector ) ? "on" : "off" );
}

void do_worklist(CHAR_DATA *ch, char *argument)
{
  char arg1[MSL], arg[MSL];

  if (argument[0] == '\0')
  {
    send_to_char("Syntax: worklist [task] [item] [start/killline] [numlines]\n\r",ch);
    return;
  }

  argument = one_argument(argument,arg);
  argument = one_argument(argument,arg1);

  if (is_exact_name(arg,"delete"))
  {
    if (is_exact_name(arg1,"bug"))
    {
      if ( rm_line_from_file(ch, BUG_FILE, argument) )
        ch->pcdata->wlbugs++;
    }
    else if (is_exact_name(arg1,"client"))
    {
      if ( rm_line_from_file(ch, BUILDER_FILE, argument) )
        ch->pcdata->wlbuild++;
    }
    else if (is_exact_name(arg1,"help"))
    {
      if ( rm_line_from_file(ch, TODO_FILE, argument) )
        ch->pcdata->wlhelps++;
    }
    else if (is_exact_name(arg1,"duh"))
    {
      if ( rm_line_from_file(ch, DUH_FILE, argument) )
        ch->pcdata->wlduhs++;
    }
    else if (is_exact_name(arg1,"typo"))
    {
      if ( rm_line_from_file(ch, TYPO_FILE, argument) )
        ch->pcdata->wltypos++;
    }
    else if (is_exact_name(arg1,"typo"))
      rm_line_from_file(ch, TYPO_FILE, argument);
    else if (is_exact_name(arg1,"hints"))
      rm_line_from_file(ch, HINTS_FILE, argument);
    else
    {
      send_to_char("Possible Work Lists are:\n\r",ch);
      send_to_char("client, help, bug, duh, hints, typo\n\r",ch);
    }
  }
  else if (is_exact_name(arg,"show"))
  {

    if (is_exact_name(arg1,"bug"))
      show_file_to_char(ch, BUG_FILE, argument);
    else if (is_exact_name(arg1,"client"))
      show_file_to_char(ch, BUILDER_FILE, argument);
    else if (is_exact_name(arg1,"help"))
      show_file_to_char(ch, TODO_FILE, argument);
    else if (is_exact_name(arg1,"duh"))
      show_file_to_char(ch, DUH_FILE, argument);
    else if (is_exact_name(arg1,"typo"))
      show_file_to_char(ch, TYPO_FILE, argument);
    else if (is_exact_name(arg1,"hints"))
      show_file_to_char(ch, HINTS_FILE, argument);
    else
    {
      send_to_char("Possible Work Lists are:\n\r",ch);
      send_to_char("client, help, bug, duh, hints, typo\n\r",ch);
    }
  }
  else if (is_exact_name(arg,"search"))
  {

    if (is_exact_name(arg1,"bug"))
      search_line_from_file(ch, BUG_FILE, argument);
    else if (is_exact_name(arg1,"client"))
      search_line_from_file(ch, BUILDER_FILE, argument);
    else if (is_exact_name(arg1,"help"))
      search_line_from_file(ch, TODO_FILE, argument);
    else if (is_exact_name(arg1,"duh"))
      search_line_from_file(ch, DUH_FILE, argument);
    else if (is_exact_name(arg1,"typo"))
      search_line_from_file(ch, TYPO_FILE, argument);
    else if (is_exact_name(arg1,"hints"))
      search_line_from_file(ch, HINTS_FILE, argument);
    else
    {
      send_to_char("Possible Work Lists are:\n\r",ch);
      send_to_char("client, help, bug, duh, hints, typo\n\r",ch);
    }
  }
  else
  {
    send_to_char("Possible Work Tasks are: \n\r",ch);
    send_to_char("delete, show, search\n\r",ch);
  }
}

void show_file_to_char(CHAR_DATA *ch, char *filename, char *argument)
{
  int numlines=0, start = 0, stop = 100;
  FILE *fp;
  BUFFER *buffer;
  char buf[MSL];
  char arg1[MSL], arg2[MSL];


  if (argument[0] == '\0')
  {
    start = 0;
    stop = 100;
  }
  else
  {
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    if (!is_number(arg1))
    {
      send_to_char("Syntax: command STARTLINE NUMBER_OF_LINES_SHOWN\n\r",ch);
      return;
    }
    start = atoi(arg1);
    if (start <= 0)
    {
      send_to_char("Syntax: command STARTLINE NUMBER_OF_LINES_SHOWN\n\r",ch);
      return;
    }

    if (arg2[0] != '\0')
    {
      if (!is_number(arg2))
      {
        send_to_char("Syntax: command STARTLINE NUMBER_OF_LINES_SHOWN\n\r",ch);
        return;
      }
      stop = atoi(arg2);
      if (stop <= 0)
      {
        send_to_char("Syntax: command STARTLINE NUMBER_OF_LINES_SHOWN\n\r",ch);
        return;
      }
      if (stop > 1000)
      {
        send_to_char("Error: Too many lines to show.\n\r",ch);
        return;
      }
    }


  }


  if ((fp = fopen(filename,"r")) == NULL)
  {
    send_to_char("Failure to open file.\n\r",ch);
    return;
  }
  nFilesOpen++;
  buffer = new_buf();
  while ((get_next_line(fp, buf) != EOF)
         && (numlines < stop+start-1))
  {

    if (++numlines >= start)
    {
      if (!strcmp(filename,HINTS_FILE))
      {
        bprintf(buffer,"{W[%3d] ", numlines);
        add_buf(buffer,buf);
        add_buf(buffer,"{x\n\r");
      }
      else
      {
        bprintf(buffer,"{W[%3d]", numlines);
        add_buf(buffer,buf);
        add_buf(buffer,"{x\n\r");
      }
    }
  }
  fclose(fp);
  nFilesOpen--;
  page_to_char(buf_string(buffer),ch);
  free_buf(buffer);
}

void show_file_to_char_bw(CHAR_DATA *ch, char *filename)
{
  int numlines=0;
  FILE *fp;
  char buf[MSL];

  if ((fp = fopen(filename,"r")) == NULL)
  {
    send_to_char("Failure to open file.\n\r",ch);
    return;
  }
  nFilesOpen++;
  while ((get_next_line(fp, buf) != EOF)
         && (numlines < 1000))
  {
    send_to_char_bw(buf,ch);
    send_to_char("\n\r",ch);
    ++numlines;
  }
  fclose(fp);
  nFilesOpen--;
}


void align_change(CHAR_DATA *ch, int change, bool isgood, bool straight)
{
  OBJ_DATA *obj, *obj_next;
  if (straight)
    ch->alignment = change;
  else
  {
    if (isgood)
      ch->alignment += change;
    else
      ch->alignment -= change;
  }

  if (ch->alignment >= 1000)
    ch->alignment = 1000;
  if (ch->alignment <= -1000)
    ch->alignment = -1000;

  /* ZAP objects if they drop based on align change. */
  for ( obj = ch->carrying; obj; obj = obj_next )
  {
    obj_next = obj->next_content;
    if ( obj->wear_loc == WEAR_NONE )
      continue;

    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
         ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
         ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
    {
      act( "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
      act( "$n is zapped by $p.",   ch, obj, NULL, TO_ROOM );

      if ( IS_OBJ_STAT(obj, ITEM_NOREMOVE) )
      {
        act ( "It causes you great pain.", ch, obj, NULL, TO_CHAR );
        damage(ch, ch, 2 * obj->level, TYPE_UNDEFINED, DAM_SHOCK, FALSE, FALSE);
      }
      else if (!melt_drop(ch,obj))
      {
        obj_from_char( obj );
        if ( IS_OBJ_STAT(obj,ITEM_NODROP) )
          obj_to_char( obj, ch );
        else
          obj_to_room( obj, ch->in_room );
      }
    }
  }

}

bool check_obj_loop(OBJ_DATA *obj)
{
  OBJ_DATA *test_obj;
  int i;
  int test_case = 200000;
  /* this routine should be able to detect infinit loops */
  /* removed as it is found that 50k is now reached normally. test case upped to 200k */
  //  if (obj->next == obj) {
  // bugf("check_obj_loop: Obj EQUALS itself. %s",interp_cmd);
  // return FALSE;
  // }
  if (pObjNum >= test_case)
    return TRUE;
  i = 0;
  for (test_obj = object_list;test_obj;test_obj = test_obj->next, i++)
  {
    if (i > test_case)
    {
      bugf("check_obj_loop: Infinite loop detected. Test Case = %d Count = %d, Num Objs In Use = %d %s.",
           test_case, i, pObjNum, interp_cmd);
      return FALSE;
    }
  }
  return TRUE;
}

bool rm_line_from_file(CHAR_DATA *ch, char *filename, char *argument)
{
  int numlines=0, killline = 0, success = FALSE;
  FILE *fp, *fp1;
  char buf[MSL];
  char wbuf[MSL];
  char arg1[MSL];
  char worklist[MSL];

  argument = one_argument(argument, arg1);
  if (argument == NULL)
  {
    send_to_char("Error: No number picked for rm_line_from_file.\n\r",ch);
    return success;
  }

  if (!is_number(arg1))
  {
    send_to_char("Error: Please use a valid integer for the delete line\n\r",ch);
    return success;
  }

  killline = atoi(arg1);
  if (killline <= 0)
  {
    send_to_char("Error: Killline does not exist\n\r",ch);
    return success;
  }

  if ((fp = fopen(filename,"r")) == NULL)
  {
    send_to_char("Failure to open file.\n\r",ch);
    return success;
  }

  nFilesOpen++;

  if ((fp1 = fopen("rm_line_temp.txt","w")) == NULL)
  {
    send_to_char("Failed to open temp file.\n\r",ch);
    fclose(fp);
    nFilesOpen--;
    return success;
  }

  nFilesOpen++;

  if (!strcmp(filename,DUH_FILE))
    strcpy(worklist,"Duh");
  else if (!strcmp(filename,TYPO_FILE))
    strcpy(worklist,"Typo");
  else if (!strcmp(filename,BUG_FILE))
    strcpy(worklist,"Bug");
  else if (!strcmp(filename,TODO_FILE))
    strcpy(worklist,"Help");
  else if (!strcmp(filename,HINTS_FILE))
    strcpy(worklist,"Hint");
  else if (!strcmp(filename,BUILDER_FILE))
    strcpy(worklist,"Client");
  else
    strcpy(worklist,"(unknown)");


  while ((get_next_line(fp, buf) != EOF))
  {

    if (++numlines == killline)
    {
      if ( !IS_SET(ch->wiznet,WIZ_ON)
           ||   !IS_SET(ch->wiznet,WIZ_WORKLIST) )
        printf_to_char(ch, "Removing line: %s\n\r",buf);

      mprintf(sizeof(wbuf), wbuf,"%s removed \"%s\" entry:%s{x", ch->name, worklist, buf );
      wiznet( wbuf, NULL, NULL, WIZ_WORKLIST, 0, 0 );

      success = TRUE;
      continue;
    }
    fprintf(fp1,"%s\n\r",buf);
  }
  fclose(fp);
  fclose(fp1);
  rename("rm_line_temp.txt",filename);
  nFilesOpen--;
  nFilesOpen--;
  return success;
}

void search_line_from_file(CHAR_DATA *ch, char *filename, char *argument)
{
  int numlines=0, numfound=0;
  FILE *fp;
  BUFFER *buffer;
  char buf[MSL];


  if (argument[0] == '\0')
  {
    send_to_char("Syntax: command searcharg\n\r",ch);
    return;
  }

  if ((fp = fopen(filename,"r")) == NULL)
  {
    send_to_char("Failure to open file.\n\r",ch);
    return;
  }
  nFilesOpen++;
  buffer = new_buf();
  while (((get_next_line(fp, buf)) != EOF) && numfound <= 100)
  {

    ++numlines;
    if (strstr(buf, argument))
    {
      bprintf(buffer,"[%d]", numlines);
      add_buf(buffer,buf);
      add_buf(buffer,"\n\r");
      numfound++;
    }
  }
  if (numfound)
    bprintf(buffer,"Found %d Matches.",numfound);
  fclose(fp);
  nFilesOpen--;
  page_to_char(buf_string(buffer),ch);
  free_buf(buffer);
}

void mprintf(int size, char *str, char *fmt, ...)
{
  char buf [MAX_ALLOC_SIZE];
  va_list args;
  va_start (args, fmt);
  vsprintf (buf, fmt, args);
  va_end (args);

  if (size < strlen(buf)+1)
  {
    bugf("MPRINTF error: fmt %s.\n\r",fmt);
    bugf("ERROR: System Memory Corrupted by Overflow, through mprintf.\n\r");
  }
  else
    strcpy(str,buf);
}

/*
 * Find a race_char in the room by Mendanbar
 */
CHAR_DATA *get_race_room(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *rch;
  int number, count;

  number = number_argument(argument, arg);
  count = 0;
  for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
  {
    if (!can_see(ch, rch) ||
        !(rch->race == race_lookup(arg))
        || (rch == ch))
      continue;

    /* race_lookup will return 0 if there not a race match (as in the case
       of objects).  Race = 0 corresponds to "unique".  Thus, when one tries
       to look at an object, and a "unique" race is in the room, the char
       will actually examine the "unique" mob, not the object.  The following
       line fixes that.  Added by Sartan 05/09/01 */

    if ((rch->race == 0) && (str_prefix(arg, "unique"))) return NULL;

    if (++count == number)
      return rch;
  }
  return NULL;
}

void do_tasklist( CHAR_DATA *ch, char *argument )
{
  char arg1[MSL], arg2[MSL], buf[MSL];
  bool privledged = FALSE;

  if (  is_exact_name(ch->name, "Aarchane")
        || is_exact_name(ch->name, "Laurelin")
        || is_exact_name(ch->name, "Merak")
        || is_exact_name(ch->name, "Taeloch"))
    privledged = TRUE;

  if (argument[0] == '\0')
  {
    if (!privledged)
    {
      send_to_char("Syntax: TASKLIST show\n\r", ch);
      return;
    }
    else
    {
      send_to_char("Syntax: TASKLIST add <victim> <message>\n\r", ch);
      send_to_char("        TASKLIST delete <message number>\n\r", ch);
      send_to_char("        TASKLIST show <victim / all> <startline> <# of lines to show>\n\r", ch);
      send_to_char("        TASKLIST search <string>\n\r", ch);
      return;
    }
  }

  argument = one_argument(argument, arg1);

  if (is_exact_name(arg1, "show") )
  {
    argument = one_argument(argument, arg2);
    if ( arg2[0] == '\0' || (is_exact_name(arg2, "all") && !privledged))
      strcpy(arg2, ch->name);
    if (!privledged && str_cmp(capitalize(arg2), ch->name) )
    {
      send_to_char("You can only view your own tasklist.\n\r", ch);
      return;
    }
    if (is_exact_name(arg2, "all") )
    {
      show_file_to_char(ch, TASK_FILE, argument);
      return;
    }
    else
    {
      mprintf(sizeof(buf), buf, "--%s--", capitalize(arg2));
      search_line_from_file(ch, TASK_FILE, buf);
      return;
    }
  }

  if (!privledged)
  {
    send_to_char("You can only view your tasklist.  'TASKLIST show'\n\r", ch);
    return;
  }

  if (is_exact_name(arg1, "add") )
  {
    CHAR_DATA *victim;
    argument = one_argument(argument, arg2);
    if (arg2[0] == '\0')
    {
      send_to_char("Who do you want to assign a task to?\n\r", ch);
      return;
    }
    else if (argument[0] == '\0')
    {
      send_to_char("What task do you want to assign?\n\r", ch);
      return;
    }
    else
    {
      mprintf(sizeof(buf), buf,"{cTASK FOR {Y--{y%s{Y-- {W%s", capitalize(arg2), argument);
      append_file(ch, TASK_FILE, buf);
      if ( (victim = get_char_world(ch, arg2)) != NULL )
        printf_to_char(victim, "You've been assigned a new task by %s.  Type TASKLIST SHOW to see it.\n\r", ch->name);
      return;
    }
  }

  if (is_exact_name(arg1, "delete") )
  {
    if (argument[0] == '\0')
      send_to_char("What line number do you want to delete?\n\r", ch);
    else
      rm_line_from_file(ch, TASK_FILE, argument);
    return;
  }

  if (is_exact_name(arg1, "search") )
  {
    if (argument[0] == '\0')
      send_to_char("What string do you want to search for?\n\r", ch);
    else
      search_line_from_file(ch, TASK_FILE, argument);
    return;
  }

  send_to_char("Syntax: TASKLIST add <victim> <message>\n\r", ch);
  send_to_char("        TASKLIST delete <message number>\n\r", ch);
  send_to_char("        TASKLIST show <victim / all> <startline> <# of lines to show>\n\r", ch);
  send_to_char("        TASKLIST search <string>\n\r", ch);
  return;
}


void cleanup_restrings(CHAR_DATA *ch)
{
  AREA_DATA *area;
  ROOM_INDEX_DATA *room;
  OBJ_DATA *obj, *obj_next;
  CHAR_DATA *owner;
  int vnum;

  for ( area = area_first; area; area = area->next )
  {
    for ( vnum = area->min_vnum; vnum <= area->max_vnum; vnum++ )
    {
      if ( !( room = get_room_index( vnum ) ) )
        continue;

      if (room->contents)
      {
        for (obj=room->contents; obj; obj= obj_next)
        {
          obj_next = obj->next_content;

          if (IS_SET(obj->extra_flags, ITEM_RESTRING))
          {
            if ( (obj->owner != NULL)
                 && ( ( owner = get_char_world( ch, obj->owner ) ) != NULL ) )
            {
              printf_to_char(ch, "Obj: %s, returned to %s from room %d.\n\r", obj->short_descr, owner->name, room->vnum);
              obj_from_room(obj);
              obj_to_char(obj, owner);
            }
            else
            {
              printf_to_char(ch, "Obj: %s, retrieved from room %d.\n\r", obj->short_descr, room->vnum);
              obj_from_room(obj);
              obj_to_char(obj, ch);
            }
          }
        }
      }
    }
  }
  send_to_char("\n\r", ch);
}

void remove_diamonds_char(CHAR_DATA *ch, CHAR_DATA *recipient, int amount)
{
  OBJ_DATA *obj, *obj_next;

  for (obj = ch->carrying; obj != NULL; obj = obj_next)
  {
    obj_next = obj->next_content;
    if (amount <= 0)
      break;
    if (IS_DIAMOND(obj))
    {
      amount--;
      if (recipient != NULL)
      {
        obj_from_char(obj);
        obj_to_char(obj, recipient);
      }
      else
        extract_obj(obj);
    }
    if (obj->item_type == ITEM_CONTAINER)
      remove_diamonds_container(obj, recipient, &amount);
  }
}

void remove_diamonds_container(OBJ_DATA *container, CHAR_DATA *recipient, int *amount)
{
  OBJ_DATA *obj, *obj_next;

  for (obj = container->contains; obj != NULL; obj = obj_next)
  {
    obj_next = obj->next_content;
    if ( (*amount) <= 0)
      break;

    if (IS_DIAMOND(obj))
    {
      (*amount)--;
      if (recipient != NULL)
      {
        obj_from_obj(obj);
        obj_to_char(obj, recipient);
      }
      else
        extract_obj(obj);
    }

    if (obj->item_type == ITEM_CONTAINER)
      remove_diamonds_container(obj, recipient, amount);
  }
}



/* Thanks to Rahty from Hellsgate*/
char    *strncpy_color( char *s1, const char *s2, size_t n, char fill,
                        bool terminate )
{
  char * s;

  for ( s = s1; n > 0 && *s2 != '\0'; )
  {
    if ( ( *s++ = *s2++ ) != '{' )
    {
      --n;
      continue;
    }
    switch ( *s2 )
    {
      case '\0':
        break;
      case '{':
        *s++ = *s2++;
        --n;
        break;
      default:
        *s++ = *s2++;
        break;
    }
  }

  for ( ; n > 0; --n )
    *s++ = fill;

  if ( terminate )
  {
    *s++ = '{';
    *s++ = 'x';
    *s   ='\0';
  }

  return s1;
}

char *area_bit_name( int64 area_flags )
{
  static char buf[512];

  buf[0] = '\0';

  if ( area_flags & AREA_NO_QUEST   ) strcat(buf, " noquest"      );
  if ( area_flags & AREA_DRAFT      ) strcat(buf, " draft"     );
  if ( area_flags & AREA_CRYSTAL    ) strcat(buf, " crystal"   );
  if ( area_flags & AREA_NOGATE     ) strcat(buf, " nogate"    );
  if ( area_flags & AREA_NOSUMMON   ) strcat(buf, " nosummon"  );
  if ( area_flags & AREA_NORESCUE   ) strcat(buf, " norescue"  );
  if ( area_flags & AREA_LIBRARY    ) strcat(buf, " library"   );
  if ( area_flags & AREA_CLANHALL   ) strcat(buf, " clan_hall" );

  return ( buf[0] ) ? buf + 1 : "none";
}

/*
 *  * Find an area in the world. For use in imm commands.
 *   */
AREA_DATA *get_area_world( char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  AREA_DATA *sArea;
  int number;
  int count;

  number = number_argument( argument, arg );
  count  = 0;

  for ( sArea = area_first; sArea; sArea = sArea->next )
  {
    if ( !is_name( sArea->name, capitalize( argument ) ) )
      continue;

    if ( ++count == number )
      return sArea;
  }
  return NULL;
}

/*
 * Gives a pointer to an obj-storage, container, ch, room...
 */
void *string_to_container( CHAR_DATA *ch, char *argument, int *from_flag )
{
  void *container = NULL;

  if ( argument[0] == '\0' )
  {
    *from_flag |= FROM_NOT_FOUND;
    return container;
  }

  if ( !strcmp( argument, "room" ) )
  {
    *from_flag = FROM_ROOM;
    return ch->in_room;
  }

  // Search for container
  if ( ( container = get_obj_here( ch, argument ) ) )
  {
    *from_flag = FROM_OBJ;
    return container;
  }

// Taeloch: Why do you want our MUD to crash?  Getting objects from a CHAR_DATA is strictly verboten!
  // Search for char
//  if ( ( container = get_char_room( ch, argument ) ) )
//  {
//    *from_flag = FROM_CH;
//    return container;
//  }

  *from_flag |= FROM_NOT_FOUND;
  return container;
}

/*
 * Parse a string into an objlist and possibly source and target
 */
bool parse_objhandling( CHAR_DATA *ch, char *objlist,
                        void **source, int *from_flag,
                        void **target, int *to_flag,
                        char *argument )
{
  char arg[MIL];

  *from_flag |= FROM_DEFAULT;
  *to_flag   |= FROM_DEFAULT;

  /* ACHTUNG!! We must consider the case of a totally empty list! */
  strcpy( objlist, "" );

  one_argument( argument, arg );
  if ( arg[0] == '\0' ) return TRUE; // This means it has failed, actually

  for ( strcpy( arg, "and" ); !strcmp( arg, "and" );
        argument = one_argument( argument, arg ) )
  {
    argument = one_argument( argument, arg );

    if ( is_number( arg ) )
    {
      strcat( strcat( objlist, arg ), "*" );
      argument = one_argument( argument, arg );
    }
    strcat( strcat( objlist, arg ), " " );
  }

  /*
   *  Now we have set up an objlist, which is to be parsed by the caller.
   *  Here we are to find the first location.
   */
  if ( arg[0] == '\0' ) return FALSE;

  if ( !strcmp( arg, "from" ) )
  {
    argument = one_argument( argument, arg );
    // Use arg to find a source -> obj, ch or room
    if ( !( *source = string_to_container( ch, arg, from_flag ) ) )
      return TRUE; // TRUE means failure
    argument = one_argument( argument, arg );

  }
  if ( arg[0] == '\0' ) return FALSE;

  if ( !strcmp( arg, "to" ) )
  {
    argument = one_argument( argument, arg );
    // Use arg to find a target
    if ( !( *target = string_to_container( ch, arg, to_flag ) ) )
      return TRUE;  // This means failure
    argument = one_argument( argument, arg );

  }
  if ( arg[0] == '\0' ) return FALSE;

  if ( !strcmp( arg, "from" ) )
  {
    argument = one_argument( argument, arg );
    // Yes, I know it looks strange, but I know what I am doing :)
    if ( !( *source = string_to_container( ch, arg, from_flag ) ) )
      return TRUE; // Failure!
    argument = one_argument( argument, arg );

  }
  if ( arg[0] == '\0' ) return FALSE;

  if ( IS_SET( *from_flag, FROM_DEFAULT ) )
  {
    if ( !( *source = string_to_container( ch, arg, from_flag ) ) )
      return TRUE;
    argument = one_argument( argument, arg );

  }
  if ( arg[0] == '\0' ) return FALSE;

  if ( IS_SET( *to_flag, FROM_DEFAULT ) )
  {
    if ( !( *target = string_to_container( ch, arg, to_flag ) ) )
      return TRUE;
    argument = one_argument( argument, arg );

  }

  return FALSE; // This means it has succeeded, actually!
}
