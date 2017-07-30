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
 *    ROM 2.4 is copyright 1993-1996 Russ Taylor                           *
 *    ROM has been brought to you by the ROM consortium                    *
 *        Russ Taylor (rtaylor@efn.org)                                    *
 *        Gabrielle Taylor                                                 *
 *        Brian Moore (zump@rom.org)                                       *
 *    By using this code, you have agreed to follow the terms of the       *
 *    ROM license, in the file Rom24/doc/rom.license                       *
 ***************************************************************************/

/***************************************************************************
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by              *
 *      Chris Litchfield and Mark Archambault                              *
 *      Sacred has been created with much time and effort from many        *
 *      different people's input and ideas.                                *
 *      By using this code, you have agreed to follow the terms of the     *
 *      Sacred license, in the file doc/sacred.license                     *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "music.h"
#include "tables.h"
#include "interp.h"
#include "special.h" // To lessen the load on merc.h -- Merak
#include "olc.h"  //To update areas

/*
 * Local functions.
 */
int     hit_gain        args( ( CHAR_DATA *ch ) );
int     mana_gain        args( ( CHAR_DATA *ch ) );
int     move_gain       args( ( CHAR_DATA *ch ) );
void    mobile_update    args( ( void ) );
void    room_update     args( ( void ) );
void    weather_update    args( ( void ) );
void    bank_update        args( ( void ) );
void    ban_update        args( ( void ) );
void    char_update     args( ( void ) );
void    obj_update        args( ( void ) );
void    aggr_update     args( ( void ) );
void    count_update    args( ( void ) );
void    newbie_update         ( void );
void    charm_update          ();
void    motd_timer_update     ();
void    send_channel          (char *argument, int chan_number);
void    player_char_stat_update( CHAR_DATA *ch );
int     player_char_sickness_update( CHAR_DATA *ch );
void    player_char_affect_update( CHAR_DATA *ch );
void    player_char_eq_update ( CHAR_DATA *ch );
void    player_char_update    ( CHAR_DATA *ch );
static  int pulse_auction;
void    violent_flag_update   ( void );
void    bail_flag_update      ( void );
void    char_act_update       ( void );
int     toxin_type( CHAR_DATA *ch );

/* used for saving */
int    save_number = 0;

void advance_claneq( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( IS_SET( obj->extra_flags, ITEM_CLANEQ ) )                   //If Clan Eq
        {
            //if ( !IS_DELETED( obj ) )              //I thought this checked for object existence..
            //{
              if (  !IS_NULLSTR( obj->owner )
              && !str_cmp( obj->owner, ch->name  ) )     //Only if the character is the owner
                set_claneq( obj, ch );
            //}
        }
    }
}

/*
 * Advancement stuff.
 */
void advance_level( CHAR_DATA *ch, bool hide )
{
  /*  char buf[MAX_STRING_LENGTH];*/
  int add_hp;
  int add_mana;
  int add_move;
  int add_prac;

  ch->pcdata->last_level = TOTAL_PLAY_TIME(ch) / 3600;

  /*  if (!ch->pcdata->user_set_title) {
      mprintf(sizeof(buf),buf, "the %s",
      title_table [ch->class] [ch->level] [ch->sex == SEX_FEMALE ?
      1 : 0] );
    mprintf(sizeof(buf),buf,".");
    set_title( ch, buf );
    }*/
  add_hp     = con_app[get_curr_stat( ch, STAT_CON )].hitp
               + number_range( class_table[ch->gameclass].hp_min, class_table[ch->gameclass].hp_max );

  add_mana   = int_app[get_curr_stat( ch, STAT_INT )].manap
               + number_range( 7, ( 2 * get_curr_stat( ch, STAT_INT )
               + get_curr_stat( ch, STAT_WIS ) ) / 5 );

  if (!class_table[ch->gameclass].fMana)
    add_mana /= 2;

  add_move   = dex_app[get_curr_stat( ch, STAT_DEX )].movep
               + number_range( 1, (get_curr_stat(ch,STAT_CON)
               + get_curr_stat(ch,STAT_DEX))/6 );

  add_prac        = wis_app[get_curr_stat(ch,STAT_WIS)].practice;
  add_hp          = add_hp * 9/10;
  add_mana        = add_mana * 9/10;
  add_move        = add_move * 9/10;
  add_hp          = UMAX(  2, add_hp   );
  add_mana        = UMAX(  2, add_mana );
  add_move        = UMAX(  6, add_move );
  ch->max_hit     += add_hp;
  ch->max_mana    += add_mana;
  ch->max_move    += add_move;
  ch->practice    += add_prac;
  ch->train       += 1;

  ch->pcdata->perm_hit            += add_hp;
  ch->pcdata->perm_mana           += add_mana;
  ch->pcdata->perm_move           += add_move;
  ch->questdata->quest_number     = 0;
  ch->questdata->current_streak   = 0;
  ch->hit                         = GET_HP(ch);
  ch->mana                        = GET_MANA(ch);
  ch->move                        = ch->max_move;
  update_pos(ch);

  char buf[MAX_STRING_LENGTH];
  char buf1[MSL];
  
  if (!hide)
  {
    printf_to_char(ch,"You gain {R%d{x hit point%s, {G%d{x mana, {B%d{x move, and {W%d{x practice%s.\n\r",
      add_hp, add_hp == 1 ? "" : "s", add_mana, add_move,
      add_prac, add_prac == 1 ? "" : "s");
     
    mprintf(sizeof(buf),buf,"{D%s {wattained {glevel %d {wwith {R%d Hp, {G%d Mana, {B%d Move, {W%d pracs{x!",
      capitalize( ch->name ),
      ch->level,
      add_hp,
      add_mana,
      add_move,
      add_prac );
    wiznet(buf,ch,NULL,WIZ_LEVELS,0,0);

    mprintf(sizeof(buf1),buf1,
      "{D%s {wattained {glevel %d {wwith {R%d Hp, {G%d Mana, {B%d Move, {W%d pracs{x!",
        ch->name,
        ch->level,
        add_hp,
        add_mana,
        add_move,
        add_prac );
    log_string( buf1 );       
  }
  return;
}   



void gain_exp( CHAR_DATA *ch, int gain )
{
  char buf[MAX_STRING_LENGTH];
  char str_buf[MAX_STRING_LENGTH];
  char note_subject[MAX_STRING_LENGTH], note_body[MAX_STRING_LENGTH];
  //long min_per_level    = 0;

  if ( IS_NPC(ch) || ch->level > LEVEL_HERO )
    return;

  if (IS_GHOST(ch))
    return;


  /*    if (gain >= 500) {
    send_to_char("{RSorry, NO experience AWARDED for this kill.\n\r",ch);
    send_to_char("Due to excessive cheating, a max experience is set\n\r",ch);
    send_to_char("If you see this message All this is noted and\n\r",ch);
    send_to_char("An IMM is alerted.  Please contact an IMM or note\n\r",ch);
    send_to_char("Immortals if you gained this without cheating.{x\n\r",ch);
    mprintf(sizeof(buf),buf,"CHAR %s gained %d experience in 1 kill",ch->name, gain);
    bug(buf,0);
    return;
    }
  */

  /* MOVED TO xp_compute (fight.c)
  if (gain >= 800) { // limit XP gain to 800 + a random percentage of the XP overflow
    gain = 800 + ( ( gain - 800 ) * number_percent() );
    mprintf(sizeof(buf),buf,"CHAR %s gained %d experience in 1 kill",ch->name, gain);
    bug(buf,0);
  }*/

  if ( ( gain > 0 ) && !IS_NPC( ch ) )
  {
    int qgain = gain / 25;

    //if ( ch->level < LEVEL_HERO  ) // Heroes don't get exp so more glory..
    qgain /= 4; //No reason Heroes should get better quest points...

    ch->questdata->curr_points += qgain;
    ch->questdata->glory += qgain;

    if ( qgain >= 1 )
      printf_to_char( ch, "{&You have gained %d glory.{x\n\r", qgain );
  }
  
  int XPL = exp_per_level( ch, ch->pcdata->points );
  if ( ch->exp < TNL( XPL, ch->level - 1 ) )
    ch->exp = TNL( XPL, ch->level -1 );
 // min_per_level = exp_per_level( ch, ch->pcdata->points ) * ( ch->level );
 //  ch->exp = UMAX( min_per_level, ch->exp + gain );
  ch->exp += gain;

  //printf_to_char( ch, "min_per_level: %d   ch->exp: %d\n\r",
  //  min_per_level, ch->exp );
  //printf_to_char( ch, "Next Level: %d\n\r",
   // exp_per_level( ch, ch->pcdata->points ) * ( ch->level + 1 ) );
  
  while ( ch->level < LEVEL_HERO
  &&      ch->exp >= TNL( exp_per_level(ch, ch->pcdata->points ), 
                          ch->level ) )
  {

      if (ch->level == (LEVEL_HERO-1)) // char has just heroed!
      {
        if (ch->alignment < -500)
          mprintf(sizeof(str_buf),str_buf,
            "{YThe Town Crier Announces: %s has levelled, achieving the status of Villain!{x\n\r",
            ch->name, ++ch->level );
        else
          mprintf(sizeof(str_buf),str_buf,
            "{YThe Town Crier Announces: %s has levelled, achieving the status of Hero!{x\n\r",
            ch->name, ++ch->level );
      }
      else
        mprintf(sizeof(str_buf),str_buf,
          "{YThe Town Crier Announces: %s has reached level %d{x\n\r",
          ch->name, ++ch->level );
      send_channel( str_buf, CHANNEL_NOGRATS );
      ch->last_level = TOTAL_PLAY_TIME( ch );
      send_to_char( "{YYou raise a level!!{x\n\r", ch );
      ch->pcdata->degrading = 0;

      if ( ch->level == ch->pcdata->clanowe_level )
      {
          ch->pcdata->clanowe_level = 0;

          if ( ch->pcdata->clanowe_dia == 0 )
            ch->pcdata->clanowe_clan = NULL;

          mprintf(sizeof(note_subject), note_subject, 
            "%s's debt to your clan.", ch->name );
          mprintf(sizeof(note_body), note_body, 
            "%s's level debt to your clan has been reached.\n\r", ch->name );
          note_line( "{YTown Crier{x", NOTE_NOTE, ch->pcdata->clanowe_clan,
                     note_subject, note_body );
          printf_to_char( ch, "You have repaid your level debt to %s.\n\r", 
                              ch->pcdata->clanowe_clan );
      }

      mprintf(sizeof(buf),buf,"{y%s gained level %d{x", ch->name, ch->level );

      log_string( buf );
      advance_level( ch, FALSE) ;
      advance_claneq( ch );

      save_char_obj( ch, FALSE );
    }
  return;
}



/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
  int gain;
  int number;
  int tlev;

  if (ch->in_room == NULL)
    return 0;

  if ( IS_NPC(ch) )
  {
      gain =  5 + ch->level;

      if (ch->level > 50)
        gain += ch->level;

    switch(ch->position)
    {
        default :         gain /= 2;            break;
        case POS_SLEEPING:     gain = 3 * gain/2;        break;
        case POS_RESTING:                      break;
        case POS_FIGHTING:    gain /= 3;             break;
     }
  }
  else
  {
      gain = UMAX(3,get_curr_stat(ch,STAT_CON) - 3 + ch->level/2); 
      gain += class_table[ch->gameclass].hp_max - 10;

      if (ch->level > 50)
        gain += ch->level;

      number = number_percent();

      if (number < get_skill(ch,gsn_fast_healing))
      {
        gain += number * gain / 100;

        tlev = ch->level;
        if (ch->hit < GET_HP(ch))
            check_improve(ch,gsn_fast_healing,TRUE,8);

        if (ch->level > tlev)
          return GET_HP(ch);
      }

      switch ( ch->position )
      {
        default:           gain /= 4;            break;
        case POS_SLEEPING:                     break;
        case POS_RESTING:      gain /= 2;            break;
        case POS_FIGHTING:     gain /= 6;            break;
      }

      if (!IS_NPC(ch))
        if (ch->pcdata->degrading < 0)
            gain = 0;
  }

  if ( race_table[ch->race].native_sect == ch->in_room->sector_type )
    gain += gain*.10; // 10% bonus if in native sector

  if (IS_AFFECTED(ch,AFF_REGENERATION))
    gain *= 2;

  gain = gain * ch->in_room->heal_rate / 100;
    
  if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
    gain = gain * ch->on->value[3] / 100;

  if ( IS_AFFECTED(ch, AFF_POISON) )
    gain /= 4;

  if (IS_AFFECTED(ch, AFF_PLAGUE))
    gain /= 8;

  if ( is_affected( ch, skill_lookup( "nightmare" ) ) )
    gain /= 3;

  if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
    gain /=2 ;

  if (!IS_NPC(ch)) 
    if (ch->pcdata->degrading < 0)
      gain = (int)ch->pcdata->degrading/10;

  return UMIN(gain, GET_HP(ch) - ch->hit);
}



int mana_gain( CHAR_DATA *ch )
{
  int gain;
  int number;
  int tlev;

  if (ch->in_room == NULL)
    return 0;

  if ( IS_NPC(ch) )
    {
      gain = 5 + ch->level;
      if (ch->level > 50)
      gain += ch->level;
      switch (ch->position)
    {
    default:        gain /= 2;        break;
    case POS_SLEEPING:    gain = 3 * gain/2;    break;
    case POS_RESTING:                break;
    case POS_FIGHTING:    gain /= 3;        break;
        }
    }
  else
    {

      gain = (get_curr_stat(ch,STAT_WIS) 
          + get_curr_stat(ch,STAT_INT) + ch->level) / 2;
      if (ch->level > 50)
      gain += ch->level;
      number = number_percent();
      if (number < get_skill(ch,gsn_meditation))
    {
      gain += (number/2) * gain / 10;

      tlev = ch->level;

      if ( (ch->mana < GET_MANA(ch))
      && (ch->position != POS_FIGHTING) )
        check_improve(ch,gsn_meditation,TRUE,8);

      if (ch->level > tlev)
        return GET_MANA(ch);
    }
      if (!class_table[ch->gameclass].fMana)
    gain /= 2;

      switch ( ch->position )
    {
    default:        gain /= 4;            break;
    case POS_SLEEPING:                     break;
    case POS_RESTING:    gain /= 2;            break;
    case POS_FIGHTING:    gain /= 6;            break;
    }

      if (!IS_NPC(ch))
    if (ch->pcdata->degrading < 0)
      gain = 0;

    }

  if ( race_table[ch->race].native_sect == ch->in_room->sector_type )
    gain += gain*.10; // 10% bonus if in native sector

  gain = gain * ch->in_room->mana_rate / 100;

  if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
    gain = gain * ch->on->value[4] / 100;

  if ( IS_AFFECTED( ch, AFF_POISON ) )
    gain /= 4;

  if (IS_AFFECTED(ch, AFF_PLAGUE))
    gain /= 8;

  if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
    gain /=2 ;
  if (!IS_NPC(ch)) 
    if (ch->pcdata->degrading < 0)
      gain = (int)ch->pcdata->degrading;

  return UMIN(gain, GET_MANA(ch) - ch->mana);
}



int move_gain( CHAR_DATA *ch )
{
  int gain;

  if (ch->in_room == NULL)
    return 0;

  if ( IS_NPC(ch) )
    {
      gain = ch->level;
      if (ch->level > 50)
      gain += ch->level;
    }
  else
    {
      gain = UMAX( 15, ch->level );
      if (ch->level > 50)
      gain += ch->level;

      switch ( ch->position )
    {
    case POS_SLEEPING: gain += get_curr_stat(ch,STAT_DEX);        break;
    case POS_RESTING:  gain += get_curr_stat(ch,STAT_DEX) / 2;    break;
    }

      if (!IS_NPC(ch))
    if (ch->pcdata->degrading < 0)
      gain = 0;
    }

  gain = gain * ch->in_room->heal_rate/100;

  if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
    gain = gain * ch->on->value[3] / 100;

  if ( IS_AFFECTED(ch, AFF_POISON) )
    gain /= 4;

  if (IS_AFFECTED(ch, AFF_PLAGUE))
    gain /= 8;

  if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
    gain /=2 ;

  if (!IS_NPC(ch)) 
    if (ch->pcdata->degrading < 0)
      gain = ch->pcdata->degrading;

  return UMIN(gain, ch->max_move - ch->move);
}



void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
  int condition;
  int degraded;
  if ( value == 0 || IS_NPC(ch) || ch->level > LEVEL_HERO)
    return;

  condition                = ch->pcdata->condition[iCond];
  if (condition == -1)
    return;
  if (IS_GHOST(ch))
    return;
  ch->pcdata->condition[iCond]    = URANGE( 0, condition + value, 48 );
  if (ch->pcdata->condition[COND_HUNGER] != 0 &&
      ch->pcdata->condition[COND_THIRST] != 0)
    ch->pcdata->degrading =0;

  if ( ch->pcdata->condition[iCond] == 0 )
  {
    degraded = -1 * (int)ch->pcdata->degrading/10;

    switch ( iCond )
    {
    case COND_HUNGER:
      switch (degraded) {
      case 0:
        send_to_char( "You are hungry.\n\r",  ch );
        break;
      case 1:
        send_to_char( "You begin to feel weak from a lack of food.\n\r",ch);
        break;
      case 2:
        send_to_char( "Your muscles stop functioning from your lack of food.\n\r",ch);
        break;
      case 3:
        send_to_char( "You almost collapse from hunger.\n\r",ch);
        break;
      default:
        send_to_char( "You need to eat or you will die.\n\r",ch);
        break;
      }
      break;
    case COND_THIRST:
      switch (degraded) {
      case 0:
        send_to_char( "You are thirsty.\n\r", ch );
        break;
      case 1:
        send_to_char( "You begin to feel weak, you could really use a drink.\n\r",ch);
        break;
      case 2:
        send_to_char( "Your body grows weak from thirst.\n\r",ch);
        break;
      case 3:
        send_to_char( "You are feeling really weak, you have no tongue left.\n\r",ch);
        break;
      default:
        send_to_char( "You need to drink or you will die.\n\r",ch);
        break;
      }
      break;
    case COND_DRUNK:
      if ( condition )
        send_to_char( "You are sober.\n\r", ch );
      break;
    }
    }

  return;
}

void room_update( void )
{

  ROOM_INDEX_DATA *room;
  int iHash;

  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
  {
    for( room = room_index_hash[iHash]; room; room = room->next )
    {
      if ( room->mprogs )
      { 
        if ( room->mprog_delay > 0 )
        {
          if ( --room->mprog_delay <= 0 )
          {
            if ( ap_percent_trigger(
              room, NULL, NULL, NULL, TRIG_DELAY, ROOM_PROG ) )
            continue;
          }
        }
        if ( IS_SET( room->mprog_flags, TRIG_RANDOM ) )
      // Check for trigger
        {
          if ( ap_percent_trigger( 
            room, NULL, NULL, NULL, TRIG_RANDOM, ROOM_PROG ) )
          continue;
        }
      }
    }
  }
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void )
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  EXIT_DATA *pexit      = NULL;
  int        door       = 0;

  /* Examine all mobs. */
  for ( ch = char_list; ch; ch = ch_next )
  {
      ch_next = ch->next;

      if ( !IS_NPC( ch )
      ||   ch->in_room == NULL 
      ||   IS_AFFECTED( ch, AFF_CHARM ) )
        continue;

      if ( ch->position <= POS_DEAD )
        continue;

        /* This check was, for some reason, omitted */
      if ( !( IS_SET( ch->act, ACT_UPDATE_ALWAYS )
      ||      ch->in_room->area->nplayer ) )
        continue;

      /* Recalls */
      if ( IS_SET( ch->act, ACT_RECALLS ) && !ch->fighting
      &&   ch->reset_room && ch->reset_room != ch->in_room
      &&   number_bits( 6 ) == 0 )
      {
          act( "$n fades away.", ch, NULL, NULL, TO_ROOM );
          move_to_room( ch, ch->reset_room );
          act( "$n fades into view.", ch, NULL, NULL, TO_ROOM );
          continue; // No more action after a recall!
      }

      /* Examine call for special procedure */
      if ( ch->spec_fun )  // Here is room for many enhancements... - Merak
          if ( ( *ch->spec_fun)( ch ) )
            continue;

      if ( ch->pIndexData == NULL ) 
      {
        bugf("BUG: Char is treated as NPC.");
        continue;
      }
      if ( ch->pIndexData->pShop ) /* give him some gold */
        if ( ( ch->gold * 100 + ch->silver ) < ch->pIndexData->wealth )
        {
          ch->gold   += ch->pIndexData->wealth * number_range( 1, 20 ) / 50000;
          ch->silver += ch->pIndexData->wealth * number_range( 1, 20 ) / 500;
        }
     
      /*
       * Check triggers only if mobile still in default position
       */
      /* if ( ch->position == ch->pIndexData->default_pos ) */
      /*         OH, really??  Merak :P                     */

      /* Delay */
      if ( ch->mprog_delay > 0 )
      {
          if ( --ch->mprog_delay <= 0 )
          {
            mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_DELAY );
            continue;
          }
      }
      /* Random trigger */
      if ( HAS_TRIGGER( ch, TRIG_RANDOM) )
      {
          if( mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_RANDOM ) )
            continue;
      }

      /* That's all for sleeping / busy monster, and empty zones */
      if ( ch->position != POS_STANDING )
        continue;

      /* Scavenger */
      if ( IS_SET( ch->act, ACT_SCAVENGER )
      &&   ch->in_room->contents
      &&   number_bits( 6 ) == 0 )
      {
        OBJ_DATA *obj;
        OBJ_DATA *obj_best;
        int max;

        max         = 1;
        obj_best    = 0;
        for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
        {
          if ( CAN_WEAR( obj, ITEM_TAKE )
          && !IS_SET(obj->extra_flags, ITEM_RESTRING)
          && can_loot( ch, obj )
          && obj->cost > max  && obj->cost > 0 )
          {
            obj_best    = obj;
            max         = obj->cost;
          }
        }

        if ( obj_best )
        {
          if (obj_best->item_type != ITEM_MONEY)
          {
            obj_from_room( obj_best );
            obj_to_char( obj_best, ch );
            act( "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
          }
          else
          {
            do_function(ch,&do_get,"all.gold");
            do_function(ch,&do_get,"all.silver");
          }
        }
      }

        CHAR_DATA *rch;

        for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
        {
          if ( IS_AFFECTED( ch,  AFF_SNEAK ) 
          &&  !IS_AFFECTED( rch, AFF_DETECT_HIDDEN ) )
            continue;

          if ( is_hating( ch, rch ) && can_see( ch, rch ) && (ch != rch ) )
          {
            if ( !IS_SET( ch->in_room->room_flags, ROOM_SAFE )
            && ( !ch->fighting )
            && !IS_AFFECTED( ch, AFF_CALM )
            && !IS_SET(ch->act, ACT_WIMPY) )
            { 
              do_function( ch, &do_say, "I remember the likes of you!" );
              ch->position = POS_FIGHTING;
              multi_hit( ch, rch, TYPE_UNDEFINED );
            }
            break;
          }
        }

      if ( ch->path && ch->path[0] )
      {
        if ( ch->path_ptr == NULL )
          ch->path_ptr = ch->path;

        spec_path( ch );
        continue;
      }

      /* Wander */
      if ( !IS_SET( ch->act, ACT_SENTINEL ) 
      &&   number_bits(3) == 0
      &&   ( door = number_bits( 5 ) ) <= 5
      &&   ( pexit = ch->in_room->exit[door] )
      &&   !IS_SET( pexit->exit_info, EX_FENCED )
      &&   pexit->u1.to_room
      &&   !IS_SET( pexit->exit_info, EX_CLOSED )
      &&   !IS_SET( pexit->u1.to_room->room_flags, ROOM_SHIP )
      &&   !IS_SET( pexit->u1.to_room->room_flags, ROOM_NO_MOB )
      &&   ( !IS_SET( ch->act, ACT_STAY_AREA )
      ||   pexit->u1.to_room->area == ch->in_room->area ) 
      &&   ( !IS_SET( ch->act, ACT_OUTDOORS )
      ||   !IS_SET( pexit->u1.to_room->room_flags, ROOM_INDOORS ) ) 
      &&   ( !IS_SET( ch->act, ACT_INDOORS )
      ||    IS_SET( pexit->u1.to_room->room_flags, ROOM_INDOORS ) ) )
        move_char( ch, door, FALSE );

      /* remove hating for fully-healed mobs */
      if ( ( ch->position != POS_FIGHTING )
      && ( ch->hate != NULL )
      && ( ch->mana >= GET_MANA( ch ) )
      && ( ch->hit  >= GET_HP( ch ) ) )
      {
        ch->hate  = NULL;
        ch->plevel = NULL;
      }

  }
  return;
}

/*
 * Update the weather.
 */
void weather_update( void )
{
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  int diff;

  buf[0] = '\0';

  switch ( ++time_info.hour )
    {
    case  5:
      weather_info.sunlight = SUN_LIGHT;
      strcat( buf, "The {cd{Wa{cy{x has begun.\n\r" );
      break;

    case  6:
      weather_info.sunlight = SUN_RISE;
      strcat( buf, "The {Ysu{yn{x rises in the east.\n\r" );
      break;

    case 19:
      weather_info.sunlight = SUN_SET;
      strcat( buf, "The {ys{Yu{bn{x slowly disappears in the west.\n\r" );
      break;

    case 20:
      weather_info.sunlight = SUN_DARK;
      strcat( buf, "The {bn{Bi{cg{Dh{bt{x has begun.\n\r" );
      break;

    case 24:
      time_info.hour = 0;
      time_info.day++;
      break;
    }

  if ( time_info.day   >= 35 )
    {
      time_info.day = 0;
      time_info.month++;
    }

  if ( time_info.month >= 17 )
    {
      time_info.month = 0;
      time_info.year++;
    }

  /*
   * Weather change.
   */
  if ( time_info.month >= 9 && time_info.month <= 16 )
    diff = weather_info.mmhg >  985 ? -2 : 2;
  else
    diff = weather_info.mmhg > 1015 ? -2 : 2;

  weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
  weather_info.change    = UMAX(weather_info.change, -12);
  weather_info.change    = UMIN(weather_info.change,  12);

  weather_info.mmhg += weather_info.change;
  weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
  weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

  switch ( weather_info.sky )
    {
    default: 
      bug( "Weather_update: bad sky %d.", weather_info.sky );
      weather_info.sky = SKY_CLOUDLESS;
      break;

    case SKY_CLOUDLESS:
      if ( weather_info.mmhg <  990
       || ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
    {
      strcat( buf, "The {cs{wk{Dy{x is getting cloudy.\n\r" );
      weather_info.sky = SKY_CLOUDY;
    }
      break;

    case SKY_CLOUDY:
      if ( weather_info.mmhg <  970
       || ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
    {
      strcat( buf, "It starts to {br{ca{Di{cn{x.\n\r" );
      weather_info.sky = SKY_RAINING;
    }

      if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
    {
      strcat( buf, "The {Dc{wl{Wou{wd{cs{x disappear.\n\r" );
      weather_info.sky = SKY_CLOUDLESS;
    }
      break;

    case SKY_RAINING:
      if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
    {
      strcat( buf, "{yL{Wi{yg{Yh{Wt{yn{Yi{wn{yg{x flashes in the sky.\n\r" );
      weather_info.sky = SKY_LIGHTNING;
    }

      if ( weather_info.mmhg > 1030
       || ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
    {
      strcat( buf, "The {br{ca{Wi{wn{x stopped.\n\r" );
      weather_info.sky = SKY_CLOUDY;
    }
      break;

    case SKY_LIGHTNING:
      if ( weather_info.mmhg > 1010
       || ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
    {
      strcat( buf, "The {yl{Yi{wg{yh{Wt{yn{Yi{wn{yg{x has stopped.\n\r" );
      weather_info.sky = SKY_RAINING;
      break;
    }
      break;
    }

  if ( buf[0] != '\0' )
    {
      for ( d = descriptor_list; d != NULL; d = d->next )
    {
      if ( d->connected == CON_PLAYING
           &&   IS_OUTSIDE(d->character)
           &&   IS_AWAKE(d->character)
		   &&   (d->character->in_room != NULL)
           &&   !IS_SET(d->character->in_room->room_flags,ROOM_NOWEATHER))
        send_to_char( buf, d->character );
    }
    }

  return;
}

/*
 * Update the bank system
 * (C) 1996 The Maniac from Mythran Mud
 *
 * This updates the shares value (I hope)
 */
void bank_update(void)
{
  int     value = 0;
  FILE    *fp;

  if ((time_info.hour < 9) || (time_info.hour > 17))
    return;         /* Bank is closed, no market... */

  value = number_range ( 0, 200);
  value -= 100;
  value /= 10;

  share_value += value;

  if ( !( fp = fopen ( BANK_FILE, "w" ) ) )
    {
      bug( "bank_update:  fopen of BANK_FILE failed", 0 );
      return;
    }
  fprintf (fp, "SHARE_VALUE %d\n\r", share_value);
  fclose(fp);
  nFilesOpen--;
}


/*
 * Update all chars, including mobs.
*/
void char_update( void )
{   
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  //char tmpstr[MSL];

  /* update save counter */
  save_number++;

  if ( save_number > 29 )
    save_number = 0;

  for ( ch = char_list; ch; ch = ch_next )
  {
      ch_next = ch->next;

      if ( !IS_VALID( ch ) ) // Has the char been free'd ?
        continue;
#if MEMDEBUG
      memdebug_check(ch,"char_update: Top");
#endif

      if ( ch->position >= POS_STUNNED )
      {
      /* check to see if we need to go home */
//        if ( IS_NPC( ch )
          /*&& ch->zone != NULL
          && ch->zone != ch->in_room->area*/
/*        && ch->in_room && ch->in_room->area
        && !IN_RANGE( ch->in_room->area->min_vnum, 
                      ch->pIndexData->vnum, 
                      ch->in_room->area->max_vnum )
        && ch->desc == NULL
        && ch->fighting == NULL 
        && !IS_AFFECTED( ch, AFF_CHARM )
        && number_percent() < 10 )
        {
          act( "$n wanders on home.", ch, NULL, NULL, TO_ROOM);
          extract_char( ch, TRUE );
          continue;
        }
*/
      player_char_stat_update( ch );
  }
  if ( !IS_VALID( ch ) ) // Has ch been deleted already?
    continue;

  if ( ch->position == POS_STUNNED )
    update_pos( ch );
  if ( !IS_NPC( ch ) )
    if ( ch->position == POS_DEAD ) 
    {
      ch->position = POS_RESTING;
      //if ( IS_IN_CLAN( ch ) )
      //  strcpy( tmpstr, ch->clan->clan_immortal ); //clan_table[ch->clan].patron);
      //else
      //  strcpy( tmpstr, "Aarchane" );
      if ( !IS_GHOST( ch ) ) 
      {
        if ( ch->clan )
        {
            act( "You are resurrected by the power of $g.",
                ch, NULL, NULL, TO_CHAR );
            act( "$n has been resurrected by the power of $g.",
                ch, NULL, NULL, TO_ROOM );
        }
        else
        {
            act( "You have been resurrected by the power of {CM{cir{Ml{mya{x.",
                ch, NULL, NULL, TO_CHAR );
            act( "$n has been resurrected by the power of {CM{cir{Ml{mya{x.",
                ch, NULL, NULL, TO_ROOM );
        }

      } 
      else 
      {
        printf_to_char( ch,
            "You feel woozy, and you start to be able to move about as a ghost.\n\r" );
      }
    }

/* I had switched the next two to avoid waking on hunger, "can't see"
from blindness/firebreath that then wears off and player sees room...
Problem is, affects wearing off would then show before room, which
would be more annoying than the problem it tries to fix -- Taeloch */
      player_char_update( ch );
      if ( !IS_VALID( ch ) )
        continue;

      player_char_affect_update( ch ); 
      if ( !IS_VALID( ch ) )
        continue;

      player_char_eq_update( ch );
      if ( !IS_VALID( ch ) )
        continue;
      player_char_sickness_update( ch );
#if MEMDEBUG
      memdebug_check(ch,"char_update: Middle");
#endif
    }

    for ( ch = player_list; ch; ch = ch_next )
    {
      ch_next = ch->next_player;
      /*
       * Autosave and autoquit.
       * Check that these chars still exist.
       */
      if ( ch->desc && ch->desc->descriptor % 15 == save_number)
        save_char_obj( ch, FALSE );

//      if ( ch->timer > 20
//      &&  !IS_IMMORTAL( ch )
//      &&  !IS_AFK( ch ) )
//        do_function(ch, &do_quit, "confirm" );

//  after 12 ticks, chars go AFK now, so this must be dealth with another way...
      if ( ( ch->timer > 10 )
      &&  IS_LINKDEAD( ch )
      &&  !IS_IMMORTAL( ch ) )
        do_function(ch, &do_quit, "confirm" );
// end AFK-timer fix

    }

// Taeloch's super-awesome mount code
// Need to account for leaving a mount behind
/*
  if ( ( ch->mount != NULL )
  &&   ( ch->in_room != ch->mount->in_room) )
    ch->mount = NULL;
*/

#if MEMDEBUG
  memdebug_check(ch,"char_update: Bottom");
#endif
  return;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  AFFECT_DATA *paf, *paf_next;

  for ( obj = object_list; obj; obj = obj_next )
  {
    CHAR_DATA *rch;
    char *message;

    obj_next = obj->next;

    if ( obj->pIndexData->mprogs
    &&   IS_SET( obj->pIndexData->mprog_flags, TRIG_RANDOM ) )
    // Check for trigger
    {
      if ( ap_percent_trigger(
            obj, NULL, NULL, NULL, TRIG_RANDOM, OBJ_PROG ) );
        continue; // If the trigger worked, we don't bother more with this obj
    }

    if (((obj->item_type == ITEM_CORPSE_NPC)
    ||   (obj->item_type == ITEM_CORPSE_NPC))
    &&  (obj->value[5] > 0))
      obj->value[5]--;

    /* go through affects and decrement */
    for ( paf = obj->affected; paf; paf = paf_next )
    {
      paf_next = paf->next;
      if ( paf->duration > 1 )
      {
        paf->duration--;
        if ( number_range( 0, 4 ) == 0 && paf->level > 0 )
          paf->level--;  /* spell strength fades with time */
      }
      else if ( paf->duration == 1 )
      {
        if ( paf_next == NULL
        ||   paf_next->type != paf->type
        ||   paf_next->duration > 1 )
        {
          if ( paf->type > 0 && skill_table[paf->type].msg_obj )
          {
            if ( obj->carried_by )
            {
              rch = obj->carried_by;
              act( skill_table[paf->type].msg_obj, rch, obj, NULL, TO_CHAR );
            }
            if ( obj->in_room && obj->in_room->people )
            {
              rch = obj->in_room->people;
              act( skill_table[paf->type].msg_obj, rch, obj, NULL, TO_ALL );
            }
          }
        }
        affect_remove_obj( obj, paf );
      }
    }

    if ( obj->timer <= 0 || --obj->timer > 0 )
    {
      if ( obj->in_room 
      &&   obj->in_room->sector_type == SECT_AIR
      && ( obj->wear_flags & ITEM_TAKE )
      &&   obj->in_room->exit[5]
      &&   obj->in_room->exit[5]->u1.to_room )
      {
        ROOM_INDEX_DATA *new_room = obj->in_room->exit[5]->u1.to_room;

        if ( ( rch = obj->in_room->people ) )
        {
          act( "$p falls away.", rch, obj, NULL, TO_ROOM );
          act( "$p falls away.", rch, obj, NULL, TO_CHAR );
        }

        obj_from_room( obj );
        obj_to_room( obj, new_room );

        if ( ( rch = obj->in_room->people ) )
        {
          act( "$p floats by.", rch, obj, NULL, TO_ROOM );
          act( "$p floats by.", rch, obj, NULL, TO_CHAR );
        }
      }
      continue;
    }
    switch ( obj->item_type )
    {
      default:              message = "$p crumbles into dust.";  break;
      case ITEM_FOUNTAIN:   message = "$p dries up.";         break;
      case ITEM_CORPSE_NPC: message = "$p decays into dust."; break;
      case ITEM_CORPSE_PC:  message = "$p decays into dust."; break;
      case ITEM_FOOD:       message = "$p decomposes.";    break;
      case ITEM_POTION:     message = "$p has evaporated from disuse.";    
        break;
      case ITEM_PORTAL:     message = "$p fades out of existence."; break;
      case ITEM_FURNITURE:
        message = "$p crumbles into dust.";
        if (obj->in_room)
        {
          for ( rch = obj->in_room->people; rch != NULL; rch = rch->next_in_room )
          {
            if ( rch->on == obj )
              do_function(rch, &do_wake, "");
          }
        }
        break;
      case ITEM_CONTAINER: 
        if ( CAN_WEAR( obj, ITEM_WEAR_FLOAT ) )
        {
          if ( obj->contains ) 
            message = 
              "$p flickers and vanishes, spilling its contents on the floor.";
          else
            message = "$p flickers and vanishes.";
        }
        else
          message = "$p crumbles into dust.";
        break;
    }

    if ( obj->carried_by )
    {
      if ( IS_NPC( obj->carried_by )
      &&   obj->carried_by->pIndexData->pShop )
        obj->carried_by->silver += obj->cost / 5;
      else
      {
        act( message, obj->carried_by, obj, NULL, TO_CHAR );

        if ( obj->wear_loc == WEAR_FLOAT )
          act( message, obj->carried_by, obj, NULL, TO_ROOM );
      }
    }
    else if ( obj->in_room && ( rch = obj->in_room->people ) )
    {
      if ( !( obj->in_obj && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
      &&   !CAN_WEAR( obj->in_obj, ITEM_TAKE ) ) )
      {
        act( message, rch, obj, NULL, TO_ROOM );
        act( message, rch, obj, NULL, TO_CHAR );
      }
    }
    
    if ( obj->contains
    && ( obj->item_type == ITEM_CORPSE_PC
      || obj->item_type == ITEM_CORPSE_NPC // NPC money (only) falls to the ground
      || obj->wear_loc == WEAR_FLOAT ) )
    {   /* save the contents */
      OBJ_DATA *t_obj, *next_obj;

      for ( t_obj = obj->contains; t_obj; t_obj = next_obj )
      {
        next_obj = t_obj->next_content;
        obj_from_obj( t_obj );

        /* Taeloch: NPC cash drops to ground */
        if ( (obj->item_type == ITEM_CORPSE_NPC)
        &&   (t_obj->item_type == ITEM_MONEY)
        &&   (obj->in_room) )
        {
          obj_to_room( t_obj, obj->in_room );
          continue;
        }

        if ( obj->in_obj ) /* in another object */
          obj_to_obj( t_obj, obj->in_obj );
        else if ( obj->carried_by )  /* carried */ 
        {
          if ( obj->wear_loc == WEAR_FLOAT )
          {
            if ( obj->carried_by->in_room == NULL )
              extract_obj( t_obj );
            else
              obj_to_room( t_obj, obj->carried_by->in_room );
          }
          else
          {
            if ( t_obj->item_type == ITEM_MONEY )
            {
              obj->carried_by->silver += t_obj->value[0];
              obj->carried_by->gold   += t_obj->value[1];
              extract_obj( t_obj );
            }
            else
              obj_to_char( t_obj, obj->carried_by );
          }
        }
        else if ( obj->in_room == NULL )  /* destroy it */
          extract_obj( t_obj );
        else /* to a room */
          obj_to_room( t_obj, obj->in_room );
      }
    }
    extract_obj( obj );
  }
  return;
}

/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update( void )
{
  CHAR_DATA *wch;
  CHAR_DATA *wch_next;
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  CHAR_DATA *victim;

  for ( wch = player_list; wch != NULL; wch = wch_next )
    {
      if (wch->next == NULL) 
    return;

      wch_next = wch->next_player;
      if ( IS_NPC(wch)
       ||   wch->level >= LEVEL_IMMORTAL
       ||   wch->in_room == NULL 
//       ||   wch->in_room->area->empty
       ||   (IS_SET(wch->spell_aff, SAFF_DETER) && !IS_SET(wch->act, PLR_TWIT))
       ||   IS_SET(wch->in_room->room_flags,ROOM_SAFE))
    continue;

      for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
    {
      int count;

      if (ch == ch->next_in_room) {
        ch->next_in_room = NULL;
        bugf("Aggro Memory Loop.  [%s]Loop same char in same room. %s",ch->name, interp_cmd);
      }
      ch_next    = ch->next_in_room;
       
      if ( !IS_NPC(ch)
           ||   !IS_SET(ch->act, ACT_AGGRESSIVE)
           ||   ch->fighting != NULL
           ||   ( ch->level < 80 && ch->level > ( wch->level + MAX_AGGRO_LEVELS ) )
           ||   IS_AFFECTED(ch,AFF_CALM)
           ||   IS_LINKDEAD(ch)
           ||   IS_AFK(ch)
           ||   IS_AFFECTED(ch, AFF_BLIND)
           ||   IS_AFFECTED(ch, AFF_CHARM)
           ||   !IS_AWAKE(ch)
           ||   (IS_SET(wch->spell_aff, SAFF_DETER) && !IS_SET(wch->act, PLR_TWIT))
           ||   (IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
           ||   !can_see( ch, wch ) 
           ||   number_bits(1) == 0 )
        continue;

      /*
       * Ok we have a 'wch' player character and a 'ch' npc aggressor.
       * Now make the aggressor fight a RANDOM pc victim in the room,
       *   giving each 'vch' an equal chance of selection.
       */
      count    = 0;
      victim    = NULL;
      for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
        {
          vch_next = vch->next_in_room;

          if ( !IS_NPC(vch)
           &&   vch->level < LEVEL_IMMORTAL
           &&   ch->level >= vch->level - MIN_AGGRO_LEVELS
           &&   ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
           &&   ( !IS_SET(vch->spell_aff, SAFF_DETER) || IS_SET(vch->act, PLR_TWIT))
           &&   can_see( ch, vch ) )
        {
          if ( number_range( 0, count ) == 0 )
            victim = vch;
          count++;
        }
        }

      if ( victim == NULL )
        continue;

      multi_hit( ch, victim, TYPE_UNDEFINED );
    }
    }    

  return;
}



/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler( void )
{
  static  int     pulse_area;
  static  int     pulse_mobile;
  static  int     pulse_violence;
  static  int     pulse_point;
  static  int      pulse_music;
  static  int      pulse_underwater;
  static  int      pulse_char_act;
  static int      pulse_hour;
  char            buf[MIL];
  static int      pulse_asave;
  AREA_DATA       *pArea;

  if ( pulse_asave > PULSE_ASAVE)
  {
    bugf("Pulse asave is %d, ",pulse_asave);
    pulse_asave = 0;
  }

  if ( --pulse_asave <= 0 )
  {
      pulse_asave  = PULSE_ASAVE;
      //We'll just save editted area, to save time.
      for( pArea = area_first; pArea; pArea = pArea->next )
      {
        if ( IS_SET(pArea->area_flags, AREA_CHANGED) )
        {
            save_area( pArea );
            REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
        }
      }

  }

  if (pulse_area > PULSE_AREA)
  {
    print_cmd_hist();
    bugf("Pulse area is %d, %s ",pulse_area, interp_cmd);
    pulse_area = 0;

  }

  if ( --pulse_area     <= 0 )
    {
      pulse_area    = PULSE_AREA;
      /* number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 ); */
      area_update ();
      count_update    ( );
      save_sys_data();
    }

  if (pulse_auction > PULSE_AUCTION) {
    bugf("Pulse auction is %d,%s ",pulse_auction, interp_cmd);
    pulse_auction = 0;
  }
  if ( --pulse_auction        <= 0 )
    {
      pulse_auction   = PULSE_AUCTION;
      auction_update( );
    }

  if (pulse_music > PULSE_MUSIC) {
    bugf("Pulse music is %d, ",pulse_music);
    pulse_music = 0;
  }

  if ( --pulse_music      <= 0 )
    {
      pulse_music    = PULSE_MUSIC;
      song_update();
    }

  if (pulse_mobile > PULSE_MOBILE) {
    bugf("Pulse mobile is %d, ",pulse_mobile);
    pulse_mobile = 0;
  }

  if ( --pulse_mobile   <= 0 )
    {
      pulse_mobile    = PULSE_MOBILE;
      mobile_update    ( );
      room_update   ( );
    }

  if (pulse_violence > PULSE_VIOLENCE) {
    bugf("Pulse violence is %d, ",pulse_violence);
    pulse_violence = 0;
  }
  if ( --pulse_violence <= 0 )
    {
      pulse_violence    = PULSE_VIOLENCE;
      violence_update    ( );
    }
  if (pulse_underwater > PULSE_UNDERWATER) {
    bugf("Pulse underwater is %d, ",pulse_underwater);
    pulse_underwater = 0;
  }

  if ( --pulse_underwater    <= 0 )
  {
      pulse_underwater     = PULSE_UNDERWATER;
      underwater_update    ( );
  }

  if (pulse_point > PULSE_TICK)
  {
    bugf("Pulse POint is %d, ",pulse_point);
    pulse_point = 0;
  }

  if ( --pulse_char_act <= 0 ) // initially created for fishing
    {
      pulse_char_act = PULSE_CHAR_ACT;
      char_act_update    ( );
    }


  if ( --pulse_point    <= 0 )
  {
      strftime( buf, MSL,"TICK TOCK:    %A, %B %d, %Y at %X %Z.",
        localtime( &current_time ) );
      wiznet(buf,NULL,NULL,WIZ_TICKS,0,0);
      pulse_point     = PULSE_TICK;
      /* number_range( PULSE_TICK / 2, 3 * PULSE_TICK / 2 ); */
      weather_update    ( );
      char_update    ( );
      obj_update    ( );
      /*motd_timer_update ( );*/
      quest_update    ( );
      /*violent_flag_update ( );*/
      newbie_update( );
      bail_flag_update();
      charm_update    ( );
  }

  if (pulse_hour > PULSE_HOUR) {
    bugf("Pulse Hour is %d, ",pulse_hour);
    pulse_hour = 0;
  }
  if ( --pulse_hour        <= 0 )
  {
      pulse_hour        = PULSE_HOUR;
/*      olc_autosave();*/
      ban_update();
    }    

  aggr_update( );
  tail_chain( );
  return;
}

void send_channel(char *argument, int chan_number)
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d; d = d->next )
    {
      if ( d->connected == CON_PLAYING )
    {
      if (!IS_SET(d->character->chan_flags,chan_number) &&
          !IS_SET(d->character->chan_flags,CHANNEL_QUIET))
        {
          printf_to_char(d->character, "%s\n\r",argument);
        }
    }
    }
}

void player_char_update(CHAR_DATA *ch)
{
  CHAR_DATA *vch;
  OBJ_DATA *obj;
  OBJ_DATA *onobj;
  //char buf[MSL];
  int afktime = 12;

  if (IS_NPC(ch))
    return;

  if (IS_IMMORTAL(ch))
  {
      ch->timer++;
      return;
  }

  /* Light check on characters, lights are perm on mobs and imms*/
  if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) )
  &&   obj->item_type == ITEM_LIGHT
  &&   obj->value[2] > 0 )
  {
      if ( --obj->value[2] == 0 && ch->in_room )
      {
        --ch->in_room->light;
        act( "$p goes out.", ch, obj, NULL, TO_ROOM );
        act( "$p {yflickers{x and goes out.", ch, obj, NULL, TO_CHAR );
        extract_obj( obj );
      }
      else if ( obj->value[2] <= 5 && ch->in_room )
        act("$p {Yflickers{x.",ch,obj,NULL,TO_CHAR);
  }

  if (!IS_NPC(ch))
    afktime = ch->pcdata->afktime;

  if ( IS_LINKDEAD( ch )
  ||   ch->timer >= afktime )
    SET_BIT(ch->comm_flags,COMM_AFK);

  if ( IS_AFK( ch ) )
  {
      if ((int)(current_time - ch->idle_snapshot) / 60 >= 30)
      {
        ch->timer++;
        return;
      }
  }

  if ( ++ch->timer >= afktime
  &&   !IS_AFK( ch ) )
  {
    if ( ch->was_in_room == NULL && ch->in_room )
    {
      ch->was_in_room = ch->in_room;

      if ( ch->fighting )
        stop_fighting( ch, TRUE );

// send player AFK instead of into the void
      for ( vch = player_list; vch != NULL; vch = vch->next_player )
      {
        if (IS_SET(vch->comm_flags,CHANNEL_QUIET))
          continue;
        if (IS_NPC(vch))
          continue;
        if (vch == ch)
          continue;
        if ( is_same_group( vch, ch ) )
        {
          do_function(ch, &do_gtell, "I am AFK (Automessage)");
          SET_BIT( ch->comm_flags, COMM_AFK );
          break;
        }
      }

      if (vch == NULL) //Have to SET_BIT here too... 2/28/09
      {
        SET_BIT( ch->comm_flags, COMM_AFK );
        send_to_char( "You are now AFK.\n\r", ch );
      }

      if (ch->idle_snapshot == 0)
        ch->idle_snapshot = current_time;

/*
      act( "$n disappears into the void.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You disappear into the void.\n\r", ch );

      if (ch->idle_snapshot == 0) 
        ch->idle_snapshot = current_time;

      for (vch = ch->in_room->people; vch; vch =vch->next_in_room) 
      {
        if (vch->master == ch && IS_PET(vch)) 
        {
          vch->was_in_room = vch->in_room;
          move_to_room(vch, get_room_index(ROOM_VNUM_LIMBO));
        }
      }

      save_char_obj( ch , FALSE);
      move_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
*/
    }
  }

  /* Do the update on a player */
  if ( !IS_LINKDEAD( ch )
  &&   !IS_AFK( ch )
  && ( ch->in_room->vnum != ROOM_VNUM_LIMBO )
  && (  ( ch->level > 1)
     || (  ( ch->level == 1 ) // if char level > 1 and 5 hours have passed, need food/drink
        && ( TOTAL_PLAY_TIME(ch) > (5 * 3600) ) ) ) )
  {
      if (ch->in_room == NULL)
      {
        bugf("Character %s in in a NULL room.%s",ch->name, interp_cmd);
        return;
      }
    
      gain_condition( ch, COND_DRUNK,  -1 );
      gain_condition( ch, COND_FULL, ch->size > SIZE_MEDIUM ? -4 : -2 );
      gain_condition( ch, COND_THIRST, -1 );
      gain_condition( ch, COND_HUNGER, ch->size > SIZE_MEDIUM ? -2 : -1);

      if ( ( ch->pcdata->condition[COND_HUNGER] <= 0 ) 
      || ( ch->pcdata->condition[COND_THIRST] <= 0 ) )
      {
        if (ch->position == POS_SLEEPING) 
        {
          if ( IS_AFFECTED(ch, AFF_SLEEP) )
          {
            send_to_char( "You can't wake up!\n\r", ch );
            return;
          }

          send_to_char( "You wake and stand up.\n\r", ch );
          act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
          ch->position = POS_STANDING;
          do_function(ch, &do_look,"auto");
          if ( ( ( onobj = ch->on ) != NULL ) // check if they are on quest bedroll
          &&  ( ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_MANA )
             || ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_HP )
             || ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_BOTH ) ) )
          {
            act("You pick up $p.",ch,onobj,NULL,TO_CHAR);
            act("$n picks up $p.",ch,onobj,NULL,TO_ROOM);
            obj_from_room( onobj );
            obj_to_char( onobj, ch );
          }

          ch->on = NULL;
      }

      ch->pcdata->degrading -= 5;

    }
    else
      ch->pcdata->degrading = 0;
  }
}

void player_char_affect_update(CHAR_DATA *ch)
{
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;

  if (IS_AFK(ch) || IS_LINKDEAD(ch))
    return;

  for ( paf = ch->affected; paf; paf = paf_next )
    {
      paf_next    = paf->next;

      /* This is for perm affects */
      if (paf->duration < 0 )
    continue;

      paf->duration--;
      if (number_range(0,4) == 0 && paf->level > 0)
    paf->level--;  /* spell strength fades with time */

  if (paf->duration <= 0)
    {
      if (  paf_next == NULL ||
        paf_next->type != paf->type ||   
        paf_next->duration > 1 )
      {
          // This little ugliness prevents charmies from bailing
          // out in the middle of a fight.
        if ( paf->type == gsn_charm_person && ch->fighting )
            continue;

        if ( paf->type > 0 && skill_table[paf->type].msg_off )
          {
            send_to_char( skill_table[paf->type].msg_off, ch );
            send_to_char( "\n\r", ch );
          }
      }

      if ((paf->where == TO_AFFECTS) && IS_SET(paf->bitvector,AFF_CHARM)) 
        stop_follower(ch);
    else
    {
        affect_remove( ch, paf );
        //Make sure stats return to normal here to be safe.
        if ( ch->hit > GET_HP(ch) )
          ch->hit = GET_HP(ch);
  
        if ( ch->mana > GET_MANA(ch) )
          ch->mana = GET_MANA(ch);
    }
  }
 }
}

void player_char_eq_update(CHAR_DATA *ch)
{
  OBJ_DATA *obj, *obj_next;

  if (IS_AFK(ch) || IS_LINKDEAD(ch) || IS_GHOST(ch))
    return;
  if (ch->in_room == NULL)
    return;

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
            } else if (!melt_drop(ch,obj)) {
              obj_from_char( obj );
              if ( IS_OBJ_STAT(obj,ITEM_NODROP) )
                obj_to_char( obj, ch );
              else
                obj_to_room( obj, ch->in_room );
            }
        }
    }
}

int player_char_sickness_update(CHAR_DATA *ch)
{
  /*
   * Careful with the damages here,
   *   MUST NOT refer to ch after damage taken,
   *   as it may be lethal damage (on NPC).
   */

  if (IS_AFK(ch) || IS_LINKDEAD(ch) || IS_GHOST(ch))
    return -1;
  
  if (ch->in_room == NULL)
    return(-1);

  if (is_affected(ch, gsn_plague))
    {
      AFFECT_DATA *af, plague;
      CHAR_DATA *vch;
      int dam;

      act("$n writhes in agony as plague {rs{Do{gres{x erupt from $s skin.",
      ch,NULL,NULL,TO_ROOM);
      send_to_char("You writhe in agony from the {rp{Dl{gague{x.\n\r",ch);

      if ((af = affect_exist(ch, gsn_plague)) == NULL)
    {
      REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
      return(-1);
    }
        
      if (af->level == 1)
    return(-1);
        
      plague.where    = TO_AFFECTS;
      plague.type     = gsn_plague;
      plague.level     = af->level - 1; 
      plague.duration   = number_range(1,2 * plague.level);
      plague.location    = APPLY_STR;
      plague.modifier     = -5;
      plague.bitvector     = AFF_PLAGUE;
        
      for ( vch = ch->in_room->people; vch; vch = vch->next_in_room)
    {
      if (!saves_spell(plague.level - 2,vch,DAM_DISEASE) 
          && !IS_IMMORTAL(vch)
          && !IS_AFFECTED(vch,AFF_PLAGUE)
          && number_bits(4) == 0
          && (!IS_SET(vch->in_room->room_flags, ROOM_SAFE)))
        {
          send_to_char("You feel hot and feverish.\n\r",vch);
          act("$n shivers and looks very {Di{gll{x.",vch,NULL,NULL,TO_ROOM);
          affect_join(vch,&plague);
        }
    }

      dam = UMIN(ch->level,af->level/5+1);
      ch->mana -= dam;
      ch->move -= dam;
      damage( ch, ch, dam, gsn_plague,DAM_DISEASE,FALSE, FALSE);
    }
  else if (IS_SET(ch->spell_aff,SAFF_YAWN))
    {
      AFFECT_DATA *af, yawn;
      CHAR_DATA *vch;

      if (ch->position != POS_SLEEPING) 
    {
      act("$n yawns really wide.", ch,NULL,NULL,TO_ROOM);
      send_to_char("You yawn so hard it hurts.\n\r",ch);
    }

      if ((af = affect_exist(ch, gsn_yawn)) == NULL)
    {
      REMOVE_BIT(ch->spell_aff,SAFF_YAWN);
      return(-1);
    }

      if (af->level == 1)
    return(-1);
        
      yawn.where    = TO_SPELL_AFFECTS;
      yawn.type     = gsn_yawn;
      yawn.level     = af->level + 100; 
      yawn.duration     = number_range(3,5 * yawn.level);
      yawn.modifier     = 0;
      yawn.location    = APPLY_NONE;
      yawn.bitvector     = SAFF_YAWN;
        
      for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if  (!IS_IMMORTAL(vch)
          && !IS_SET(vch->spell_aff,SAFF_YAWN)
          && number_bits(4) == 0)
        {
          send_to_char("You feel tired.\n\r",vch);
          act("$n looks weak and tired.",vch,NULL,NULL,TO_ROOM);
          affect_join(vch,&yawn);
        }
    }
    }
  else if ( IS_AFFECTED(ch, AFF_POISON) && !IS_AFFECTED(ch,AFF_SLOW))
  {
    AFFECT_DATA *poison;
    poison = affect_find(ch->affected,gsn_poison);

    if ( poison )
    { // tox affects here
      switch (toxin_type(ch))
      {
//elemental  - fatigue, unsteadiness, sickness
        case TOX_ELEMENTAL:
          act( "$n stumbles and looks sickly.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You feel sick and lose your balance.\n\r", ch );
          ch->move = UMIN( ch->move - ch->level, ch->max_move );
          damage(ch,ch,poison->level/15 + 1,gsn_poison, DAM_POISON,FALSE, FALSE);
          break;
//fungal     - disorientation, decreased immunity, thirst
        case TOX_FUNGAL:
          act( "$n stumbles and winces in pain.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "An intense pain shoots through your arm.\n\r", ch );
          if (!IS_NPC(ch))
            gain_condition(ch,COND_THIRST,-2);
          damage(ch,ch,poison->level/10 + 1,gsn_poison, DAM_POISON,FALSE, FALSE);
          break;
//acidic     - extreme pain, feverish shaking, fatigue
        case TOX_ACIDIC:
          act( "$n screams in agony.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "An intense pain shoots through your body.\n\r", ch );
          ch->move = UMIN( ch->move - (ch->level/2), ch->max_move );
          damage(ch,ch,poison->level/5 + 1,gsn_poison, DAM_POISON,FALSE, FALSE);
          break;
//necrotic   - extreme pain, severe damage
        case TOX_NECROTIC:
          act( "$n stumbles and winces in pain.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "An intense pain shoots through your arm.\n\r", ch );
          damage(ch,ch,poison->level/4 + 1,gsn_poison, DAM_POISON,FALSE, FALSE);
          break;
//viral      - decreases strength, long lasting
        case TOX_VIRAL:
          act( "$n shivers violently and sweats profusely.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "A cold chill runs through your body.\n\r", ch );
          damage(ch,ch,poison->level/8 + 1,gsn_poison, DAM_POISON,FALSE, FALSE);
          break;
//neurotoxic - disorientation, loss of muscle control
        case TOX_NEUROTOXIC:
          act( "$n shivers and screams in pain.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "A wave of pain shoots through your body.\n\r", ch );
          damage(ch,ch,poison->level/10 + 1,gsn_poison, DAM_POISON,FALSE, FALSE);
          break;
//hallucinogenic - visual impairment (wrong exits/direction, MOBs --PK chars? -- that aren't present)
        case TOX_HALLUCINOGENIC:
          act( "$n dodges an unseen assailant.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "The world temporarily melts into a sea of color.\n\r", ch );
          break;
 // traditional venom
        default: // If a non-toxicology poison was administered, this will be a negative value
          act( "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You shiver and suffer.\n\r", ch );
          damage(ch,ch,poison->level/10 + 1,gsn_poison, DAM_POISON,FALSE, FALSE);
          break;
      }
    }
  }

  else if ( ch && IS_SET( ch->spell_aff, SAFF_HICCUP ) )
  {
      AFFECT_DATA *hiccup;

      hiccup = affect_find(ch->affected,gsn_hiccup);

      if ( hiccup )
    {
      act( "$n hiccups rather loudly.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You hiccup loudly.\n\r", ch );
    }
  }

  else if ( ch->position == POS_INCAP && number_range(0,1) == 0)
    {
      damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE, FALSE);
    }
  else if ( ch->position == POS_MORTAL )
    {
      damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE, FALSE);
    }

  return(0);
}

void player_char_stat_update(CHAR_DATA *ch)
{
  char buf[MSL];
  bool changed = FALSE;

  if ( IS_AFK( ch ) || IS_LINKDEAD( ch ) || IS_GHOST( ch ) )
      return;

  if ( !IS_NPC( ch ) ) 
  {
    if ( ch->in_room == NULL )
      return;

    if ( IS_IN_CLAN( ch ) )
     copy_roster_clannie( ch );

    if ( ( !IS_LINKDEAD( ch ) ) 
    && !IS_AFK( ch ) 
    && ( ch->in_room->vnum != ROOM_VNUM_LIMBO ) ) 
    {
      if ( ch->hit  < GET_HP( ch ) || ( ch->pcdata->degrading < 0 ) 
      || ( ch->in_room && ch->in_room->heal_rate < 0 ) ) 
      {
        ch->hit  += hit_gain( ch );
        changed = TRUE;
      }
      else if ( ch->hit > GET_HP( ch ) ) 
      {
        if (ch->hit - GET_HP(ch) > 100)
          ch->hit -= 20 * (ch->hit - GET_HP(ch)) / 100;
        else
          ch->hit -=20;
        if ( ch->hit < GET_HP( ch ) )
          ch->hit = GET_HP( ch );
        changed = TRUE;
      }
      else
        ch->hit = GET_HP( ch );

      if ( ch->mana < GET_MANA( ch ) || ( ch->pcdata->degrading < 0 ) 
      || ( ch->in_room && ch->in_room->mana_rate < 0 ) ) 
      {
        ch->mana += mana_gain( ch );
        changed = TRUE;
      }
      else if ( ch->mana > GET_MANA( ch ) ) 
      {
        if ( ch->mana - GET_MANA( ch ) > 100 )
          ch->mana -= 20 * ( ch->mana - GET_MANA( ch ) ) / 100;
        else
          ch->mana -=20;
        if ( ch->mana < GET_MANA( ch ) )
          ch->mana = GET_MANA( ch );
        changed = TRUE;
      }
      else
        ch->mana = GET_MANA( ch );

      if ( ch->move < ch->max_move || ( ch->pcdata->degrading < 0 )  
      || (ch->in_room && ch->in_room->heal_rate < 0 ) ) 
      {
        ch->move += move_gain(ch);
        changed = TRUE;
      }
      else if ( ch->move > ch->max_move )
      {
        if ( ch->move - ch->max_move > 100 )
          ch->move -= 20 * ( ch->move - ch->max_move ) / 100;
        else
          ch->move -=20;
        if ( ch->move < ch->max_move )
          ch->move = ch->max_move;
        changed = TRUE;
      }
      else
        ch->move = ch->max_move;

      if (ch->hit<= -1) 
      {
        send_to_char("You have {rdied{x due to not taking care of yourself.{x\n\r",ch);
        mprintf(sizeof(buf),buf,
                "%s has {rdied{x due to condition.\n\r", ch->name );
        wiznet( buf, NULL, NULL, WIZ_DEATHS, 0, 0 );
        ch->position = POS_FIGHTING; // keeps mobs from crashing the MUD! - Taeloch
        raw_kill( ch, ch );
        ch->pcdata->degrading =0;
      }
    }
  }
  else 
  {
    if ( ch->hit < GET_HP( ch ) )//ch->max_hit )//GET_HP(ch))
      ch->hit += hit_gain( ch );
    else
      ch->hit = GET_HP(ch); //ch->max_hit;
    if ( ch->mana < GET_MANA(ch) ) //ch->max_mana ) 
      ch->mana += mana_gain( ch );
    else
      ch->mana = GET_MANA(ch);

    if ( ch->move < ch->max_move )
      ch->move += move_gain( ch );
    else
      ch->move = ch->max_move;
  }

// make sure char doesn't go over max (rare circumstances)
  if ( ch->mana > GET_MANA( ch ) )
    ch->mana = GET_MANA( ch );
  if ( ch->hit > GET_HP( ch ) )
    ch->hit = GET_HP( ch );

  if ( changed )
    send_to_char( "\n\r", ch );
}

void charm_update()
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;

  // update save counter 
  for ( ch = char_list; ch; ch = ch_next )
  {
    ch_next = ch->next;

    if (!IS_AFFECTED(ch,AFF_CHARM))
    {
      //Make mobs "wander home" when they don't belong in area (abandoned pets, ex-charms)
      if ( IS_NPC(ch)
      && ( ch->in_room )
      && ( ch->pIndexData )
      && ( ch->in_room->area->vnum != ch->pIndexData->area->vnum )
      && ( ch->position != POS_FIGHTING )
      && !IS_SET(ch->act2,ACT2_NOWANDEROFF) )
      {
        if ( !ch->reset_room // pets have no reset room
        || ( ch->reset_room
          && ( ch->reset_room->area->vnum != ch->in_room->area->vnum) ) )
        {
          act("$n wanders off...",ch,NULL,NULL,TO_ROOM);
          extract_char( ch, TRUE );
        }
      }
      continue;
    }

/* end of wander-home change
    if (!IS_AFFECTED(ch,AFF_CHARM))
        continue;
*/

    if (ch->master == NULL)
        continue; /* dont remove, as person may have lost
             connection, can reconnect into this */
    if (IS_PET(ch))
        continue;

    if (IS_NPC(ch))
        ch->master->mana -= ch->level/2;
    else
        ch->master->mana -= ch->level*3;
    
    if (ch->master->mana <= 0)
      {
        ch->master->mana = 0;
        act( "You have broken free of $N's control!", ch, NULL,
          ch->master, TO_CHAR );
        send_to_char("You have lost control of a charmie.\n\r",ch->master);
        stop_follower(ch);
        remove_affect(ch, TO_AFFECTS, AFF_CHARM);
      } 
    else if ((ch->master->in_room != ch->in_room)
    &&       (ch->in_room->vnum != ROOM_VNUM_LIMBO)
    &&       (number_percent() > 50))
      {
        act( "You have broken free of $N's control!", ch, NULL,  ch->master, TO_CHAR );
        send_to_char("You have lost control of a charmie.\n\r",ch->master);
        remove_affect(ch, TO_AFFECTS, AFF_CHARM);
        stop_follower(ch);
      }
  }
}

void underwater_update( void )
{
  char buf[MSL];
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  for ( vch = player_list; vch; vch = vch_next )
    {
      vch_next = vch->next_player;

      if (IS_LINKDEAD(vch) || IS_AFK(vch))
    continue;

      if (IS_NPC(vch) || IS_IMMORTAL(vch))
    continue;
    
      if ( IS_SET(vch->in_room->room_flags, ROOM_UNDER_WATER) 
      &&  !IS_SET(vch->affected_by, AFF_SWIM)
      &&  !is_affected( vch, skill_lookup("aqua landhi") )
      &&  (vch->race != race_lookup("vampire"))
      &&  (vch->race != race_lookup("yzendri")) )
    {
      if ( vch->hit > 20)
        {
          vch->hit /= 2;
          send_to_char("You're drowning!!!\n\r", vch);
          act("$n is drowning!!!",vch, NULL, NULL, TO_ROOM);
        }
      else
        {
	      mprintf(sizeof(buf), buf,
            "{R%s{x has managed to drown %sself at {g%s {x[{g%d{x]",
						IS_NPC(vch) ? vch->short_descr : vch->name,
					  vch->sex == 2 ? "her" : "him",
            vch->in_room->name,
            vch->in_room->vnum );
	      wiznet(buf,vch,NULL,WIZ_DEATHS,0,0);

          vch->hit = 1;
          vch->position = POS_FIGHTING; // keeps mobs from crashing the MUD! - Taeloch
          raw_kill(vch,vch);
          send_to_char("{WYou are {rDEAD{x!!\n\r", vch );
          act("$n is dead and bloated from drowning!!!",
            vch, NULL, NULL, TO_ROOM);
        }
    }
    }
}

void violent_flag_update( void )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  for ( vch = player_list; vch; vch = vch_next )
    {
      vch_next = vch->next_player;

      if (IS_NPC(vch))
    continue;
      
      if (IS_LINKDEAD(vch) || IS_AFK(vch))
    continue;
      
      if (!IS_VIOLENT(vch))
    continue;

      if (IS_SET(vch->in_room->room_flags, ROOM_SAFE)
      || (vch->in_room->clan)
      || (IS_SET(vch->in_room->room_flags, ROOM_PRIVATE))
      || IS_SET(vch->in_room->room_flags, ROOM_ARENA)) 
    {
      send_to_char("COWARD, your god increases your violent time in this room.\n\r",vch);
      if (vch->vflag_timer < 90)
        vch->vflag_timer += 5;
      if (vch->vflag_timer >= 90)
        vch->vflag_timer = 90;
    } else {
      vch->vflag_timer--;
      if (vch->vflag_timer <= 0) {
        REMOVE_BIT(vch->act,PLR_VIOLENT);
        vch->vflag_timer = 0;
        send_to_char("You are no longer considered violent to society.\n\r",vch);
      }
    }
    }

}

void bail_flag_update( void )
{
  CHAR_DATA *vch;

  for ( vch = player_list; vch; vch = vch->next_player )
    {
      if (IS_NPC(vch))
    continue;
      
      if (!IS_KILLER(vch) && vch->bailed) 
    {
      vch->bailed = 0;
      send_to_char("Bail flag is removed.\n\r",vch);
    }

      if (vch->bailed) 
    if (vch->bailed + 604800 < current_time) /* one week of time */ {
      vch->bailed = 0;
      send_to_char("Your bail flag has expired.\n\r",vch);
    }
    }

}

void newbie_update( void )
{
  int numlines=0,hintno=1;
  char buf1[MSL],buf2[MSL];
  FILE *fp;

  nFilesOpen++;

  if ((fp = fopen(HINTS_FILE,"r")) == NULL)
    return;

  while ( (get_next_line(fp, buf1) != EOF) )
    numlines++;

  fclose(fp);

  if (numlines > 0) {
    hintno = number_range(1,numlines);
    numlines = 1;

    if ((fp = fopen(HINTS_FILE,"r")) != NULL) {
      while ( (get_next_line(fp, buf1) != EOF)
      && numlines < hintno )
        numlines++;

      mprintf(sizeof(buf2),buf2,"{W<{gNewbie {cHint{W> %s{x\n\r",buf1);
      send_channel(buf2,CHANNEL_NEWBIE);
    }

    fclose(fp);
  }

  nFilesOpen--;
}

/*
 * Update count array for people online in the last 24 hours.
 */
void count_update( void )
{
  DESCRIPTOR_DATA    *d;
  struct tm        *tod;
  sh_int        i;
  sh_int        dow;

  /* Count the people playing. */
  countCur = 0;
  for ( d = descriptor_list; d; d = d->next )
    if ( d->connected == CON_PLAYING )
      countCur++;

  /* Check for new hour. */
  if ( countHour != ( current_time / 3600 ) % 24 )
    {
      countHour = ( current_time / 3600 ) % 24;
      countArr[countHour] = 0;
    }

  /* Check max on the hour. */
  if ( countArr[countHour] < countCur )
    countArr[countHour] = countCur;

  /* Check max on the last 24 hours. */
  for ( countMaxDay = i = 0; i < 24; i++ )
    if ( countArr[i] > countMaxDay )
      countMaxDay = countArr[i];

  /* Check max on for Day-of-the-Week. */
  tod = localtime( &current_time );
  dow = URANGE( 0, tod->tm_wday, 6 );
  if ( countCur > countMaxDoW[dow] )
    countMaxDoW[dow] = countCur;

  /* Check for max on ever. */
  if ( countMaxDay > countMax )
    countMax = countMaxDay;
}

void motd_timer_update()
{

  DESCRIPTOR_DATA *d;

  for (d = descriptor_list; d ; d = d->next);
  {
      if (d->connected == CON_READ_MOTD)
      {
        d->character->timer++;

        if ( d->character->timer >= 5 ) 
            do_function(d->character, do_quit, "confirm");
      }
  }
}
      
void char_act_update( void )
{ // every 10 seconds
  CHAR_DATA *ch;
  OBJ_DATA  *obj=NULL;
  OBJ_DATA  *fishobj=NULL;
  int chance = 0;
  int cfish,fish[7];
  char buf[MSL];

// Check for players who are fishing
  for (ch = player_list; ch != NULL; ch = ch->next_player)
  {
    if ((IS_SET(ch->act, PLR_FISHING))
    &&  !IS_AFK( ch ) )
    {
      if ( ( !IS_SET( ch->in_room->room_flags, ROOM_FISHING ) ) // check sector
      &&   ( !IS_SET( ch->in_room->room_flags, ROOM_SHIP ) )
      &&   (  ch->in_room->sector_type != SECT_WATER_SWIM ) )
        REMOVE_BIT(ch->act, PLR_FISHING);

      if ( ch->fighting != NULL ) // no fighting
        REMOVE_BIT(ch->act, PLR_FISHING);

      if ( ( ( obj = get_eq_char(ch, WEAR_HOLD) ) == NULL )
      ||     ( obj->item_type != ITEM_FISHING_ROD ) ) // need a rod
        REMOVE_BIT(ch->act, PLR_FISHING);

      if (IS_SET(ch->act, PLR_FISHING))
      { // OK, he's really fishing then
        chance = number_percent();
        chance += obj->value[0]-1; // bonus for a longer rod
        if (get_curr_stat( ch, STAT_DEX ) > 21 ) chance += 1;
        if (get_curr_stat( ch, STAT_STR ) > 21 ) chance += 1;

        if (chance > 97)
        {
          if (number_percent() > obj->value[1]) // rod strength check
          {
            send_to_char( "Something breaks your rod, rendering it useless!\n\r", ch );
            act( "$n just broke $s fishing rod!", ch, NULL, NULL, TO_ROOM );
            ch->pcdata->fish_broken++;
            REMOVE_BIT(ch->act, PLR_FISHING);
            extract_obj( obj );
            continue;
          }

          // We've successfully caught something!
          fish[0] = OBJ_VNUM_DEF_FISH1;
          fish[1] = OBJ_VNUM_DEF_FISH2;
          fish[2] = OBJ_VNUM_DEF_FISH3;
          fish[3] = OBJ_VNUM_DEF_FISH4;
          fish[4] = OBJ_VNUM_DEF_FISH5;
          fish[5] = OBJ_VNUM_DEF_FISH6;
          fish[6] = OBJ_VNUM_DEF_FISH7;
          fish[7] = OBJ_VNUM_DEF_FISH8;
          for (cfish=0;cfish<=7;cfish++)
          {
            if (ch->in_room->fish[cfish] > 0)
              fish[cfish] = ch->in_room->fish[cfish];
          }

          cfish = 0;

          chance = number_percent();
          if ( chance <= FC1)
            cfish = fish[0];
          else if ( chance <= (FC1+FC2))
            cfish = fish[1];
          else if ( chance <= (FC1+FC2+FC3))
            cfish = fish[2];
          else if ( chance <= (FC1+FC2+FC3+FC4))
            cfish = fish[3];
          else if ( chance <= (FC1+FC2+FC3+FC4+FC5))
            cfish = fish[4];
          else if ( chance <= (FC1+FC2+FC3+FC4+FC5+FC6))
            cfish = fish[5];
          else if ( chance <= (FC1+FC2+FC3+FC4+FC5+FC6+FC7))
            cfish = fish[6];
          else // FC8 % chance
            cfish = fish[7];

          fishobj = create_object( get_obj_index( cfish ), 0 );
          if (fishobj)
          {
            obj_to_char( fishobj, ch );
            if (fishobj->item_type == ITEM_FOOD)
            {
              printf_to_char( ch, "{xYou caught %s!{x\n\r",fishobj->short_descr);
              sprintf( buf, "$n just caught %s.", fishobj->short_descr );
              act( buf, ch, NULL, NULL, TO_ROOM );
              ch->pcdata->fish_caught++;
            }
            else
            {
              printf_to_char( ch, "{xYour line gets caught and you bring up %s!{x\n\r",
                fishobj->short_descr);
              sprintf( buf, "$n was trying for a fish, but all $s came up with was %s.",
                fishobj->short_descr );
              act( buf, ch, NULL, NULL, TO_ROOM );
            }
          }
          else
            bugf("NULL fish [%d]!",cfish);

        }
      }
    }
  }
  return;
}
/*
*/
