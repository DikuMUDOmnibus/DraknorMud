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
*  ROM 2.4 is copyright 1993-1996 Russ Taylor                              *
*  ROM has been brought to you by the ROM consortium                       *
*      Russ Taylor (rtaylor@efn.org)                                       *
*      Gabrielle Taylor                                                    *
*      Brian Moore (zump@rom.org)                                          *
*  By using this code, you have agreed to follow the terms of the          *
*  ROM license, in the file Rom24/doc/rom.license                          *
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
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include <math.h>
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "interp.h"

void look_sky args( ( CHAR_DATA *ch ) );

void look_checkers( CHAR_DATA *ch, OBJ_DATA *obj );
char *wholine( CHAR_DATA *ch, CHAR_DATA *wch );
void donate_obj( CHAR_DATA *ch, OBJ_DATA *obj, int frominv );
bool has_affect( CHAR_DATA *ch, sh_int sn ); // to clean up spellups
int toxin_type( CHAR_DATA *ch );
//char * continent_name (AREA_DATA *pArea); Moving to merc.h

char *  const  where_name  [] =
{
  "<used as light>      ",
  "<worn on finger>     ",
  "<worn on finger>     ",
  "<worn around neck>   ",
  "<worn around neck>   ",
  "<worn on body>       ",
  "<worn on head>       ",
  "<worn on legs>       ",
  "<worn on feet>       ",
  "<worn on hands>      ",
  "<worn on arms>       ",
  "<worn as shield>     ",
  "<worn about torso>   ",
  "<worn about waist>   ",
  "<worn around wrist>  ",
  "<worn around wrist>  ",
  "<wielded>            ",
  "<held>               ",
  "<floating nearby>    ",
  "<secondary weapon>   ",  /* ADD THIS */
  "<hooked on belt>     ",  /* ADD THIS */
  "<worn on back>       ",
  "<worn on lapel>      ",
  "<worn on left ear>   ",
  "<worn on right ear>  ",
  "<worn on left eye>   ",
  "<worn on right foot> ",
  "<worn on left foot>  ",
  "<worn as crest>      ",

};

/* for do_count */
static int max_on = 0;

char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort,
                          bool fShowLevel )
{
  static char buf[MAX_STRING_LENGTH];

  buf[0] = '\0';

  if ( ( fShort && ( obj->short_descr == NULL || obj->short_descr[0] == '\0' ) )
       ||               ( obj->description == NULL || obj->description[0] == '\0' ) )
    return buf;

  if ( fShowLevel )
    if ( obj->level <= ch->level )
      strcat( buf, "{C*{x" );

  if ( IS_OBJ_STAT( obj, ITEM_INVIS ) ) strcat( buf, "{c({DIn{wvis{c){x " );

  if ( IS_AFFECTED( ch,  AFF_DETECT_EVIL )
       &&   IS_OBJ_STAT( obj, ITEM_EVIL ) )  strcat( buf, "{w({rEv{Di{rl{w){x " );

  if ( IS_AFFECTED( ch,  AFF_DETECT_GOOD )
       &&   IS_OBJ_STAT( obj, ITEM_BLESS ) ) strcat( buf, "{w({yG{Wo{yod{w){x " );

  if ( IS_AFFECTED( ch,  AFF_DETECT_MAGIC )
       &&   IS_OBJ_STAT( obj, ITEM_MAGIC ) ) strcat( buf, "{w({mMagic{w){x " );

  if ( IS_OBJ_STAT( obj, ITEM_GLOW ) )  strcat( buf, "{w({cGlow{w){x " );

  if ( IS_OBJ_STAT( obj, ITEM_HUM ) )   strcat( buf, "{w({gHumm{w){x " );
  {
    if ( !IS_NPC( ch ) )
      if ( ( ch->questdata->obj_vnum == obj->pIndexData->vnum )
           &&     is_exact_name( ch->name, obj->owner ) )
        strcat( buf, "{W[{RTARGET{W]{x" );
  }

  if ( fShort )
  {
    if ( obj->short_descr )
      strcat( buf, obj->short_descr );
  }
  else
  {
    if ( obj->description )
      strcat( buf, obj->description );
  }

  return buf;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort,
                        bool fShowNothing, bool fShowLevel, bool fDonationPit )
{
  char buf[MAX_STRING_LENGTH];
  BUFFER *output;
  char **prgpstrShow;
  int *prgnShow;
  char *pstrShow;
  OBJ_DATA *obj;
  int nShow;
  int iShow;
  int count;
  bool fCombine;
  int z = 0;

  if ( ch->desc == NULL )
    return;

  /*
   * Alloc space for output lines.
   */
  count = 0;
  for ( obj = list; obj; obj = obj->next_content )
  {
    if ( obj->next_content == obj )
    {
      bugf( "Infinite loop detected. %s", interp_cmd );
      obj->next_content = NULL;
    }
    if ( count < 0 )
    {
      crash_fix();
      exit(1);
    }
    count++;
  }

  /* fix in case there is a memory problem */
  if ( !count )
    return;

  output        = new_buf();
  prgpstrShow    = alloc_mem( count * sizeof(char *) );
  prgnShow      = alloc_mem( count * sizeof(int)    );
  nShow  = 0;

  /*
   * Format the list of objects.
   */
  if ( fDonationPit )
  {
    for ( z = 1 ; z < LEVEL_IMMORTAL ; z++ )
    {
      for ( obj = list; obj; obj = obj->next_content )
      {
        if ( obj->level == z )
        {
          if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ))
          {
            pstrShow = format_obj_to_char( obj, ch, fShort, fShowLevel );
            fCombine = FALSE;

            if ( IS_NPC( ch ) || IS_SET( ch->comm_flags, COMM_COMBINE ) )
            {
              /*
               * Look for duplicates, case sensitive.
               * Matches tend to be near end so run loop backwords.
               */
              for ( iShow = nShow - 1; iShow >= 0; iShow-- )
              {
                if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
                {
                  prgnShow[iShow]++;
                  fCombine = TRUE;
                  break;
                }
              }
            }

            /*
             * Couldn't combine, or didn't want to.
             */
            if ( !fCombine )
            {
              prgpstrShow [nShow] = str_dup( pstrShow , "");
              prgnShow    [nShow] = 1;
              nShow++;
            }
          }
        }
      }
    }
  }
  else
  {
    for ( obj = list; obj; obj = obj->next_content )
    {
      if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ))
      {
        pstrShow = format_obj_to_char( obj, ch, fShort, fShowLevel );
        fCombine = FALSE;

        if ( IS_NPC( ch ) || IS_SET( ch->comm_flags, COMM_COMBINE ) )
        {
          /*
           * Look for duplicates, case sensitive.
           * Matches tend to be near end so run loop backwords.
           */
          for ( iShow = nShow - 1; iShow >= 0; iShow-- )
          {
            if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
            {
              prgnShow[iShow]++;
              fCombine = TRUE;
              break;
            }
          }
        }

        /*
         * Couldn't combine, or didn't want to.
         */
        if ( !fCombine )
        {
          prgpstrShow [nShow] = str_dup( pstrShow , "");
          prgnShow    [nShow] = 1;
          nShow++;
        }
      }
    }
  }
  /*
   * Output the formatted list.
   * First for Donation Pits, then for all other containers
   */
  for ( iShow = 0; iShow < nShow; iShow++ )
  {
    if ( prgpstrShow[iShow][0] == '\0' )
    {
      free_string( prgpstrShow[iShow] );
      continue;
    }

    if ( IS_NPC( ch ) || IS_SET( ch->comm_flags, COMM_COMBINE) )
    {
      if ( prgnShow[iShow] != 1 )
      {
        mprintf( sizeof(buf), buf, "(%2d) ", prgnShow[iShow] );
        add_buf(output,buf);
      }
      else
        add_buf(output,"     ");
    }

    add_buf(output,prgpstrShow[iShow] );
    add_buf(output,"\n\r" );
    free_string( prgpstrShow[iShow] );
  }

  if ( fShowNothing && nShow == 0 )
  {
    if ( IS_NPC(ch) || IS_SET( ch->comm_flags, COMM_COMBINE ) )
      send_to_char( "     ", ch );

    send_to_char( "Nothing.\n\r", ch );
  }

  page_to_char( buf_string( output ), ch );

  /*
   * Clean up.
   */
#if OLD_MEM
  free_mem( prgpstrShow, count * sizeof(char *) );
  free_mem( prgnShow,    count * sizeof(int)    );
#else
  free_mem( prgpstrShow );
  free_mem( prgnShow );
#endif
  free_buf( output );

  return;
}



void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH], message[MAX_STRING_LENGTH];

  buf[0] = '\0';

  if ( IS_PLOADED( victim ) )
    strcat( buf, "{R[{rPLoad{R]{x "  );
  if ( IS_AFK( victim ) )
    strcat( buf, "{w[{gA{GF{gK{w]{x " );
  if ( IS_AFFECTED( victim, AFF_INVISIBLE ) )
    strcat( buf, "{c({DIn{wvis{c){x " );
  if ( victim->invis_level >= LEVEL_HERO )
    strcat( buf, "{w[{cW{w]{x "  );
  if ( IS_AFFECTED( victim, AFF_HIDE ) )
    strcat( buf, "{w({DHide{w){x " );
  if ( IS_AFFECTED( victim, AFF_CHARM ) )
    strcat( buf, "{m({rCharm{m){x " );
  if ( IS_AFFECTED( victim, AFF_PASS_DOOR ) )
    strcat( buf, "{W({wTrans{W){x ");
  if ( IS_AFFECTED( victim, AFF_FAERIE_FIRE ) )
    strcat( buf, "{w({mPink{w){x "  );
  if ( IS_LINKDEAD( victim ) )
    strcat( buf, "{W[{RLinkdead{W]{x "  );
  if ( IS_EVIL( victim ) && IS_AFFECTED( ch, AFF_DETECT_EVIL ) )
    strcat( buf, "{w({rEv{Di{rl{w){x "   );
  if ( victim->desc && victim->desc->editor )
    strcat( buf, "{B[{cBLD{B]{x " );
  if ( IS_GOOD( victim ) && IS_AFFECTED( ch, AFF_DETECT_GOOD ) )
    strcat( buf, "{w({yG{Wo{yod{w){x " );
  if ( IS_AFFECTED( victim, AFF_SANCTUARY ) )
    strcat( buf, "{w({WWhite{w){x " );
  if ( IS_AFFECTED( victim, AFF_AQUA_ALBEDO ) )
    strcat( buf, "{B({cAlbedo{B){x " );
  if ( IS_AFFECTED2( victim, AFF2_INVUN ) )
    strcat( buf, "{b({mGlobe{b){x " );
  if ( IS_SAFFECTED( victim, SAFF_MAGESHIELD ) )
    strcat( buf, "{B({bMage{B){x " );
  if ( IS_SAFFECTED( victim, SAFF_WARRIORSHIELD ) )
    strcat( buf, "{Y({yWarrior{Y){x " );
  if ( IS_AFFECTED2( victim, AFF2_NIRVANA ) )
    strcat( buf, "{w({gNirv{cana{w){x " );
  if ( IS_GHOST( victim ) )
    strcat( buf, "{D({WG{wh{Do{ws{Wt{D){x " );
  if ( IS_SAFFECTED( victim, SAFF_IRONWILL ) )
    strcat( buf, "{w({DIron{ywill{w){x " );
  if ( IS_SAFFECTED( victim, SAFF_WARCRY_GUARDING ) )
    strcat( buf, "{r({yGuard{r){x " );
  if ( IS_AFFECTED2( victim, AFF2_FADE_OUT ) )
    strcat( buf, "{w({WF{wa{Dde{w){x " );
  if ( IS_AFFECTED2( victim, AFF2_RADIANT ) )
    strcat( buf, "{c({WRadiant{c){x " );
  if ( IS_AFFECTED2( victim, AFF2_SHROUD )     )
    strcat( buf, "{r({DShroud{r){x " );
  if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_KILLER ) )
    strcat( buf, "{w({rK{w){x "     );
  if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_THIEF  ) )
    strcat( buf, "{w({DT{w){x " );
  if ( !IS_NPC( victim ) && victim->bailed
       && ( IS_IMMORTAL( ch ) || ch == victim ) )
    //||  ch->clan == clan_lookup("dredd")))
    strcat( buf, "{B({bB{B){x " );
  if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_TWIT ) )
    strcat( buf, "{R({YT{BW{MI{GT{R){x " );
  if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_VIOLENT ) )
    strcat( buf, "{w({yV{w){x " );
  if ( IS_NPC( victim ) && !IS_NPC( ch ) )
    if ( ch->questdata->mob_vnum == victim->pIndexData->vnum )
      strcat(buf,"{W[{RTARGET{W]{x");
  if ( victim->position == victim->start_pos && victim->long_descr[0] )
  {
    strcat( buf, victim->long_descr );
    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_FISHING ) )
      strcat( buf, "{w[{yFishing{w]{x " );
    send_to_char( buf, ch );
    return;
  }

  strcat( buf, PERS( victim, ch ) );
  if ( !IS_NPC( victim ) && !IS_SET( ch->comm_flags, COMM_BRIEF )
       &&    victim->position == POS_STANDING && ch->on == NULL )
    strcat( buf, victim->pcdata->title );

  switch ( victim->position )
  {
    case POS_DEAD:
      strcat( buf, " is a rotting corpse."     );
      break;
    case POS_MORTAL:
      strcat( buf, " is mortally wounded."     );
      break;
    case POS_INCAP:
      strcat( buf, " is incapacitated."        );
      break;
    case POS_STUNNED:
      strcat( buf, " is lying here stunned."   );
      break;
    case POS_SLEEPING:
      if ( victim->on )
      {
        if ( IS_SET( victim->on->value[2], SLEEP_AT ) )
        {
          mprintf(sizeof(message), message, " is sleeping at %s.",
                  victim->on->short_descr );
          strcat( buf, message );
        }
        else if ( IS_SET( victim->on->value[2], SLEEP_ON ) )
        {
          mprintf(sizeof(message), message, " is sleeping on %s.",
                  victim->on->short_descr );
          strcat( buf, message );
        }
        else if ( IS_SET( victim->on->value[2], SLEEP_UNDER ) )
        {
          mprintf(sizeof(message), message, " is sleeping under %s.",
                  victim->on->short_descr );
          strcat( buf, message );
        }
        else
        {
          mprintf(sizeof(message), message, " is sleeping in %s.",
                  victim->on->short_descr );
          strcat( buf, message );
        }
      }
      else
        strcat(buf," is sleeping here.");
      break;
    case POS_RESTING:
      if ( victim->on )
      {
        if (IS_SET(victim->on->value[2],REST_AT))
        {
          mprintf(sizeof(message), message," is resting at %s.",
                  victim->on->short_descr);
          strcat(buf,message);
        }
        else if (IS_SET(victim->on->value[2],REST_ON))
        {
          mprintf(sizeof(message), message," is resting on %s.",
                  victim->on->short_descr);
          strcat(buf,message);
        }
        else if (IS_SET(victim->on->value[2],REST_UNDER))
        {
          mprintf(sizeof(message), message," is resting under %s.",
                  victim->on->short_descr);
          strcat(buf,message);
        }
        else
        {
          mprintf(sizeof(message), message, " is resting in %s.",
                  victim->on->short_descr);
          strcat(buf,message);
        }
      }
      else
        strcat( buf, " is resting here." );
      break;
    case POS_SITTING:
      if ( victim->on )
      {
        if (IS_SET(victim->on->value[2],SIT_AT))
        {
          mprintf(sizeof(message), message," is sitting at %s.",
                  victim->on->short_descr);
          strcat(buf,message);
        }
        else if (IS_SET(victim->on->value[2],SIT_ON))
        {
          mprintf(sizeof(message), message," is sitting on %s.",
                  victim->on->short_descr);
          strcat(buf,message);
        }
        else if (IS_SET(victim->on->value[2],SIT_UNDER))
        {
          mprintf(sizeof(message), message," is sitting under %s.",
                  victim->on->short_descr);
          strcat(buf,message);
        }
        else
        {
          mprintf(sizeof(message), message, " is sitting in %s.",
                  victim->on->short_descr);
          strcat(buf,message);
        }
      }
      else
        strcat(buf, " is sitting here.");
      break;
    case POS_STANDING:
      if ( victim->on )
      {
        if ( IS_SET( victim->on->value[2], STAND_AT ) )
        {
          mprintf(sizeof(message), message," is standing at %s.",
                  victim->on->short_descr);
          strcat(buf,message);
        }
        else if (IS_SET(victim->on->value[2],STAND_ON))
        {
          mprintf(sizeof(message), message," is standing on %s.",
                  victim->on->short_descr);
          strcat(buf,message);
        }
        else
        {
          mprintf(sizeof(message), message," is standing in %s.",
                  victim->on->short_descr);
          strcat(buf,message);
        }
      }
      else
        strcat( buf, " is here." );
      break;
    case POS_FIGHTING:
      strcat( buf, " is here, fighting " );
      if ( victim->fighting == NULL )
        strcat( buf, "thin air??" );
      else if ( victim->fighting == ch )
        strcat( buf, "{RYOU!{x" );
      else if ( victim->in_room == victim->fighting->in_room )
      {
        strcat( buf, PERS( victim->fighting, ch ) );
        strcat( buf, "." );
      }
      else
        strcat( buf, "someone who left??" );
      break;
  }

  if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_FISHING ) )
    strcat( buf, " {w[{yFishing{w]{x " );

  strcat( buf, "\n\r" );
  buf[0] = UPPER( buf[0] );
  send_to_char( buf, ch );
  return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  int percent;
  int number;

  number=number_percent();

  if ( can_see( victim, ch )
       &&  ( !IS_IMMORTAL( ch ) || ch->trust <= victim->trust ) )
  {
    if ( ch == victim )
      act( "$n looks at $mself.", ch, NULL, NULL, TO_ROOM );
    else
    {
      if ( !IS_CLASS_HIGHWAYMAN( ch ) )
      {
        act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
        act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
      }

      if ( IS_CLASS_HIGHWAYMAN( ch ) && ch != victim
           &&   number > get_skill( ch, gsn_investigate ) )
      {
        act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
        act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
        check_improve( ch, gsn_investigate, TRUE, 8 );
      }
    }
  }

  if ( victim->description[0] )
    send_to_char( victim->description, ch );
  else
    act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );

  if ( victim->max_hit > 0 )
    percent = ( 100 * victim->hit ) / GET_HP( victim );
  else
    percent = -1;

  strcpy( buf, PERS( victim, ch ) );

  if (percent >= 100)
    strcat( buf, " is in excellent condition.\n\r" );
  else if (percent >= 95 )
    strcat( buf, " is in good condition.\n\r" );
  else if (percent >= 87 )
    strcat( buf, " has a few scratches.\n\r" );
  else if (percent >= 80 )
    strcat( buf, " has a few small wounds.\n\r" );
  else if (percent >= 70 )
    strcat( buf," has some small wounds and bruises.\n\r" );
  else if (percent >= 60 )
    strcat( buf," has some wounds and bruises.\n\r" );
  else if (percent >=  50 )
    strcat( buf, " has quite a few wounds.\n\r" );
  else if (percent >= 40 )
    strcat( buf, " has some big wounds and scratches.\n\r" );
  else if (percent >= 30 )
    strcat( buf, " has some big nasty wounds and scratches.\n\r" );
  else if (percent >= 15 )
    strcat ( buf, " looks pretty hurt.\n\r" );
  else if (percent >= 7 )
    strcat ( buf, " looks very hurt.\n\r" );
  else if (percent >= 0 )
    strcat ( buf, " is in awful condition.\n\r" );
  else
    strcat(buf, " is bleeding to death.\n\r" );

  if ( IS_NPC( victim ) && !IS_NPC( ch ) ) // Check if mob is quest target
    if ( ch->questdata->mob_vnum == victim->pIndexData->vnum )
      strcat(buf,"{&This is your quest target!{x");

  buf[0] = UPPER(buf[0]);
  send_to_char( buf, ch );

  show_char_eq_to_char( ch, victim );

  if ( victim != ch
       &&  !IS_NPC( ch )
       &&  !IS_IMMORTAL( victim )
       &&   number_percent() < get_skill( ch, gsn_peek ) )
  {
    send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
    check_improve( ch, gsn_peek, TRUE, 4 );
    show_list_to_char( victim->carrying, ch, TRUE, TRUE, FALSE, FALSE );
  }
  else
  {
    if ( IS_IMMORTAL( ch )
         && ( victim != ch )
         && ( ch->trust >= victim->trust ) )
    {
      send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
      show_list_to_char( victim->carrying, ch, TRUE, TRUE, FALSE, FALSE );
    }
  }
  /* Why dupe?
    if ( victim != ch
    &&  !IS_NPC( ch )
    &&  !IS_IMMORTAL( victim )
    &&   number_percent() < get_skill( ch, gsn_peek ) )
    {
      send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
      check_improve( ch, gsn_peek, TRUE, 4 );
      show_list_to_char( victim->carrying, ch, TRUE, TRUE, FALSE, FALSE );
    }
  */
  return;
}

void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
  CHAR_DATA *rch;

  for ( rch = list; rch; rch = rch->next_in_room )
  {
    if ( rch == ch )
      continue;

    if ( get_trust( ch ) < rch->invis_level )
      continue;

    if ( can_see( ch, rch ) )
    {
      if ( strcmp( rch->short_descr, "NOTHING" ) )
        show_char_to_char_0( rch, ch );
    }

    else if ( room_is_dark( ch->in_room )
              &&        IS_AFFECTED( rch, AFF_INFRARED ) )
      send_to_char( "You see {cglowing {rred{x eyes watching you.\n\r", ch );
  }

// hallucinating (poison)
  if ( IS_AFFECTED(ch, AFF_POISON)  /* 25% chance to see nonexistent mob */
       &&   number_percent() < 25 )
  {
    if (toxin_type(ch) == TOX_HALLUCINOGENIC)
    {
      // pick a random mob from the world
      if (!IS_NPC(ch)
          &&   ch->hate
          &&   ch->hate->who
          && ( ch->hate->who->in_room != ch->in_room )
          && ( number_percent() < 50 ) ) // 50% chance (after 25) that char will see last person fighting
        show_char_to_char_0( ch->hate->who, ch );

      CHAR_DATA *vsearch=NULL;
      CHAR_DATA *mob_array[250];
      int nmobs = 0;

      for ( vsearch = char_list; vsearch; vsearch = vsearch->next )
      {
        if ( vsearch->in_room == NULL)
          continue; // Don't want mobs not in a room or area

        if (vsearch->in_room->area->low_range == -1
            || vsearch->in_room->area->high_range == -1)
          continue; // No unlinked areas

        if (IS_SET(vsearch->in_room->area->flags, AREA_DRAFT))
          continue; // No idea what this is, but might be important

        if (number_percent() > 5) // consider only 5% of all mobs
          continue;

        mob_array[nmobs++] = vsearch;
        if (nmobs >= 250)
          break;

      }
      if (nmobs >=1)
        show_char_to_char_0( mob_array[number_range(0,nmobs)], ch );
    }
  }
  return;
}

char *mprog_type_to_name( int type );

void show_proglist( CHAR_DATA *ch, MPROG_LIST *proglist, int vnum, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  MPROG_LIST *list;
  int cnt;

  printf_to_char( ch,
                  "\n\r{cPrograms for {w[{y%5d{w ]{c:{x %s\n\r\n\r",
                  vnum, argument );

  send_to_char( " {wNumber  Vnumber  Name         Trigger Phrase\n\r", ch );
  send_to_char( " {c------ --------- ------------ ------- ------{x\n\r", ch );

  for ( cnt = 0, list = proglist; list; list = list->next, cnt++ )
  {
    strncpy_color(buf, ((list->code == NULL) ? "{r?" : ((list->code->name == NULL) ? "{Y?" : list->code->name)) , 12, ' ', TRUE );
    printf_to_char( ch, "{w [{y%3d {w] [{y%6d{w ]{x %12s %-7s %-6s\n\r",
                    cnt, list->vnum, buf, mprog_type_to_name( list->trig_type ), list->trig_phrase);
  }
}

bool check_blind( CHAR_DATA *ch )
{

  if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) )
    return TRUE;

  if ( IS_AFFECTED( ch, AFF_BLIND ) )
  {
    send_to_char( "You can't see a thing!\n\r", ch );
    return FALSE;
  }

  return TRUE;
}

/* changes your scroll */
void do_scroll( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  int lines;

  one_argument( argument, arg );

  if (arg[0] == '\0')
  {
    if ( ch->lines == 0 )
      send_to_char( "You do not page long messages.\n\r", ch );
    else
      printf_to_char( ch, "You currently display %d lines per page.\n\r",
                      ch->lines + 2 );
    return;
  }

  if ( !is_number( arg ) )
  {
    send_to_char("You must provide a number.\n\r",ch);
    return;
  }

  lines = atoi( arg );

  if ( !lines )
  {
    send_to_char( "Paging disabled.\n\r", ch );
    ch->lines = 0;
    return;
  }

  if ( lines < 10 || lines > 100 )
  {
    send_to_char( "You must provide a reasonable number.\n\r", ch );
    return;
  }

  printf_to_char( ch, "Scroll set to %d lines.\n\r", lines );
  ch->lines = lines - 2;
}

/* RT does socials */
void do_socials( CHAR_DATA *ch, char *argument )
{
  int iSocial;
  int col;
  BUFFER *buffer;
  char buf[MSL];
  char arg[MIL];
  col = 0;

  argument = one_argument( argument, arg );

  buffer = new_buf();
  for ( iSocial = 0; social_table[iSocial].name[0]; iSocial++ )
  {
    if ( arg[0] && !strstr( social_table[iSocial].name, arg ) )
      continue;

    mprintf(sizeof(buf), buf, "%-12s", social_table[iSocial].name );
    add_buf( buffer,buf );
    if (++col % 6 == 0)
      add_buf( buffer, "\n\r" );
  }

  if ( col % 6 )
    add_buf( buffer, "\n\r" );
  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );
  return;
}



/* RT Commands to replace news, motd, imotd, etc from ROM */

void do_motd( CHAR_DATA *ch, char *argument )
{
  do_function( ch, &do_help, "motd" );
}

void do_imotd( CHAR_DATA *ch, char *argument )
{
  do_function( ch, &do_help, "imotd" );
}

void do_story(CHAR_DATA *ch, char *argument)
{
  do_function(ch,&do_help,"story");
}

void do_wizlist(CHAR_DATA *ch, char *argument)
{
  do_function(ch,&do_help,"wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist(CHAR_DATA *ch, char *argument)
{
  /* lists most player flags */
  if ( IS_NPC( ch ) )
    return;

  send_to_char( "{cAuto Options   Status   Nospam Options  Status (Help Nospam for Syntax){x\n\r", ch );
  send_to_char( "=====================   ===============================================\n\r", ch );

  printf_to_char( ch, "Autoassist     %-3s{x       Self Misses     %-3s    Other Misses      %-3s\n\r",
                  IS_SET( ch->act, PLR_AUTOASSIST ) ? "{gON{x " : "{rOFF{x",
                  IS_SET( ch->nospam, NOSPAM_SMISS ) ? "{gON{x " : "{rOFF{x",
                  IS_SET( ch->nospam, NOSPAM_OMISS ) ? "{gON{x " : "{rOFF{x" );

  printf_to_char( ch, "Autoexit       %-3s{x       Self Hits       %-3s    Other Hits        %-3s\n\r",
                  IS_SET( ch->act, PLR_AUTOEXIT )   ? "{gON{x " : "{rOFF{x",
                  IS_SET( ch->nospam, NOSPAM_SHIT ) ? "{gON{x " : "{rOFF{x",
                  IS_SET( ch->nospam, NOSPAM_OHIT ) ? "{gON{x " : "{rOFF{x" );

  printf_to_char( ch, "Autogold       %-3s{x       Self Effects    %-3s    Other Effects     %-3s\n\r",
                  IS_SET( ch->act, PLR_AUTOGOLD )   ? "{gON{x " : "{rOFF{x",
                  IS_SET( ch->nospam, NOSPAM_SEFFECTS ) ? "{gON{x " : "{rOFF{x",
                  IS_SET( ch->nospam, NOSPAM_OEFFECTS ) ? "{gON{x " : "{rOFF{x" );

  printf_to_char( ch, "Autoloot       %-3s{x       Self Parry      %-3s    Other Parry       %-3s\n\r",
                  IS_SET( ch->act, PLR_AUTOLOOT )   ? "{gON{x " : "{rOFF{x",
                  IS_SET( ch->nospam, NOSPAM_SPARRY ) ? "{gON{x " : "{rOFF{x",
                  IS_SET( ch->nospam, NOSPAM_OPARRY ) ? "{gON{x " : "{rOFF{x" );

  printf_to_char( ch, "Autosplit      %-3s{x       Self Dodge      %-3s    Other Dodge       %-3s\n\r",
                  IS_SET( ch->act, PLR_AUTOSPLIT )  ? "{gON{x " : "{rOFF{x",
                  IS_SET( ch->nospam, NOSPAM_SDODGE ) ? "{gON{x " : "{rOFF{x",
                  IS_SET( ch->nospam, NOSPAM_ODODGE ) ? "{gON{x " : "{rOFF{x" );


  printf_to_char( ch, "Autosac        %-3s{x\n\r",
                  IS_SET( ch->act, PLR_AUTOSAC )    ? "{gON{x " : "{rOFF{x" );

  printf_to_char( ch, "Autodonate     %-3s{x\n\r",
                  IS_SET( ch->act, PLR_AUTODONATE ) ? "{gON{x " : "{rOFF{x" );

  printf_to_char( ch, "Autohunt       %-3s{x\n\r\n\r",
                  IS_SET( ch->act, PLR_AUTOHUNT )   ? "{gON{x " : "{rOFF{x" );

  send_to_char( "{cCommand        Status   Score Options   Status{x\n\r", ch );
  send_to_char( "=====================   =======================\n\r", ch );

  printf_to_char( ch, "ansi           %-3s{x       Show Affects    %-3s{x\n\r",
                  IS_SET( ch->act, PLR_COLOUR ) ? "{gON{x " : "{rOFF{x",
                  IS_SET(ch->comm_flags,COMM_SHOW_AFFECTS) ? "{gON{x " : "{rOFF{x" );

  printf_to_char( ch, "compact        %-3s{x       Show Stats      %-3s{x\n\r",
                  IS_SET( ch->comm_flags, COMM_COMPACT ) ? "{gON{x " : "{rOFF{x",
                  IS_SET(ch->comm_flags,COMM_STATS_SHOW) ? "{gON{x " : "{rOFF{x" );

  printf_to_char( ch, "brief          %-3s{x\n\r",
                  IS_SET( ch->comm_flags, COMM_BRIEF ) ? "{gON{x " : "{rOFF{x" );

  printf_to_char( ch, "newsfaerie     %-3s{x\n\r",
                  IS_SET( ch->comm_flags, COMM_NEWSFAERIE)? "{gON{x " : "{rOFF{x" );

  printf_to_char( ch, "prompt         %-3s %s{x\n\r\n\r",
                  IS_SET( ch->comm_flags, COMM_PROMPT ) ? "{gON{x " : "{rOFF{x",
                  IS_SET( ch->comm_flags, COMM_PROMPT ) ? ch->prompt : "" );

  send_to_char( "{cCommand        Status{x\n\r", ch );
  send_to_char( "==============================================================\n\r", ch );

  if ( ch->lines == 0 )
    send_to_char("scroll         You don't use paging.\n\r", ch );
  else
    printf_to_char(ch, "scroll         Your scroll is set to %d lines.\n\r",
                   ch->lines + 2 );

  printf_to_char(ch, "autoafk        You will go AFK after %d ticks idle.\n\r",
                 ch->pcdata->afktime );

  if ( IS_SET( ch->act, PLR_CANLOOT ) )
    send_to_char("noloot         Your corpse may be looted.{x\n\r",ch);
  else if (IS_SET(ch->act,PLR_MORGUE))
    send_to_char("noloot         Your corpse will be returned to the morgue.{x\n\r", ch);
  else
    send_to_char("noloot         Your corpse is safe from thieves.{x\n\r",ch);

  if (IS_SET(ch->act,PLR_NOSUMMON))
    send_to_char("nosummon       You cannot be summoned.\n\r",ch);
  else
    send_to_char("nosummon       You can be summoned.\n\r",ch);

  if (IS_SET(ch->act,PLR_NOGATE))
    send_to_char("nogate         You cannot be gated to.\n\r",ch);
  else
    send_to_char("nogate         You can be gated to.\n\r",ch);

  if (IS_IMMORTAL(ch))
  {
    if (IS_SET(ch->act,PLR_NODIGMOVE))
      send_to_char("automove       You cannot move when digging.\n\r",ch);
    else
      send_to_char("automove       You can automove when digging.\n\r",ch);
  }

  if (IS_SET(ch->act,PLR_NOFOLLOW))
    send_to_char("nofollow       You do not welcome followers.{x\n\r",ch);
  else
    send_to_char("nofollow       You accept followers.{x\n\r",ch);

  if (IS_SET(ch->act,PLR_NOCANCEL))
    send_to_char("nocancel       You cannot be cancelled.{x\n\r",ch);
  else
    send_to_char("nocancel       You can be cancelled.{x\n\r",ch);

  if (IS_SET( ch->act, PLR_HIDEQUEST ))
    send_to_char("quest showinfo Others do not see your detailed quest info.{x\n\r",ch);
  else
    send_to_char("quest showinfo Others may see your detailed quest info.{x\n\r",ch);

  if (ch->wimpy > 0)
    printf_to_char(ch, "wimpy          You will attempt to flee under %d hit points.\n\r",
                   ch->wimpy );
  else
    send_to_char("wimpy          You will not automatically flee from combat.{x\n\r",ch);

  if (!IS_NPC(ch) && ( ch->level > 79 ))
  {
    if ( IS_SET( ch->plr2, PLR2_NO_FRAGXP ) )
      send_to_char("cleanse fragxp You will not receive experience from fragment kills.\n\r",ch);
    else
      send_to_char("cleanse fragxp You will receive experience from fragment kills.\n\r", ch);
  }

  if (IS_SET(ch->comm_flags,COMM_NO_SLEEP_TELLS))
    send_to_char("sleeptells     You cannot receive tells when asleep.\n\r",ch);
  else
    send_to_char("sleeptells     You will receive tells when asleep.\n\r", ch);
}

void do_autoassist( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC( ch ) )
    return;

  if ( IS_SET( ch->act, PLR_AUTOASSIST ) )
  {
    send_to_char( "Autoassist removed.\n\r", ch );
    REMOVE_BIT( ch->act, PLR_AUTOASSIST );
  }
  else
  {
    send_to_char( "You will now assist when needed.\n\r", ch );
    SET_BIT( ch->act, PLR_AUTOASSIST );
  }
}

void do_autoexit(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->act,PLR_AUTOEXIT))
  {
    send_to_char("Exits will no longer be displayed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTOEXIT);
  }
  else
  {
    send_to_char("Exits will now be displayed.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOEXIT);
  }
}

void do_autogold(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->act,PLR_AUTOGOLD))
  {
    send_to_char("Autogold removed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTOGOLD);
  }
  else
  {
    send_to_char("Automatic gold looting set.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOGOLD);
  }
}

void do_autoloot(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->act,PLR_AUTOLOOT))
  {
    send_to_char("Autolooting removed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTOLOOT);
  }
  else
  {
    send_to_char("Automatic corpse looting set.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOLOOT);
  }
}

void do_autosac(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->act,PLR_AUTOSAC))
  {
    send_to_char("Autosacrificing removed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTOSAC);
  }
  else
  {
    send_to_char("Automatic corpse sacrificing set.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOSAC);

    //Can't have both donate and sac.
    if ( IS_SET( ch->act, PLR_AUTODONATE ) )
    {
      send_to_char("Autodonation removed.\n\r", ch );
      REMOVE_BIT( ch->act, PLR_AUTODONATE );
    }
  }
}

void do_autodonate(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->act,PLR_AUTODONATE))
  {
    send_to_char("Autodonation removed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTODONATE);
  }
  else
  {
    send_to_char("Automatic corpse donation set.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTODONATE);

    //Can't have both donate and sac
    if ( IS_SET( ch->act, PLR_AUTOSAC ) )
    {
      send_to_char("Autosacrificing removed.\n\r", ch );
      REMOVE_BIT( ch->act, PLR_AUTOSAC );
    }

  }
}

void do_autohunt(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->act,PLR_AUTOHUNT))
  {
    send_to_char("Autohunt removed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTOHUNT);
  }
  else
  {
    send_to_char("Autohunt set.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOHUNT);
  }
}


void do_autostats(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->plr2,PLR2_STATS))
  {
    send_to_char("Direct Statistics removed.\n\r",ch);
    REMOVE_BIT(ch->plr2,PLR2_STATS);
  }
  else
  {
    send_to_char("Direct Statistics Set.\n\r",ch);
    SET_BIT(ch->plr2,PLR2_STATS);
  }
}

void do_autosplit(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->act,PLR_AUTOSPLIT))
  {
    send_to_char("Autosplitting removed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTOSPLIT);
  }
  else
  {
    send_to_char("Automatic gold splitting set.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOSPLIT);
  }
}

/*void do_autohunt( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC( ch ) )
    return;

  if ( IS_SET( ch->act, PLR_AUTOHUNT ) )
  {
    send_to_char( "Autohunt removed.\n\r", ch );
    REMOVE_BIT( ch->act, PLR_AUTOHUNT );
  }
  else
  {
    send_to_char( "You will now hunt when needed.\n\r", ch );
    SET_BIT( ch->act, PLR_AUTOHUNT );
  }
}*/


void do_autoall(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  SET_BIT(ch->act,PLR_AUTOASSIST);
  SET_BIT(ch->act,PLR_AUTOEXIT);
  SET_BIT(ch->act,PLR_AUTOGOLD);
  SET_BIT(ch->act,PLR_AUTOLOOT);
//  SET_BIT(ch->act,PLR_AUTOSAC); // donate only
  SET_BIT(ch->act,PLR_AUTODONATE);
  SET_BIT(ch->act,PLR_AUTOSPLIT);
  SET_BIT(ch->comm_flags,COMM_NEWSFAERIE);

  send_to_char("All relevant autos turned on.\n\r",ch);
}

void do_brief(CHAR_DATA *ch, char *argument)
{
  if (IS_SET(ch->comm_flags,COMM_BRIEF))
  {
    send_to_char("Full descriptions activated.\n\r",ch);
    REMOVE_BIT(ch->comm_flags,COMM_BRIEF);
  }
  else
  {
    send_to_char("Short descriptions activated.\n\r",ch);
    SET_BIT(ch->comm_flags,COMM_BRIEF);
  }
}

void do_compact(CHAR_DATA *ch, char *argument)
{
  if (IS_SET(ch->comm_flags,COMM_COMPACT))
  {
    send_to_char("Compact mode removed.\n\r",ch);
    REMOVE_BIT(ch->comm_flags,COMM_COMPACT);
  }
  else
  {
    send_to_char("Compact mode set.\n\r",ch);
    SET_BIT(ch->comm_flags,COMM_COMPACT);
  }
}

void do_show(CHAR_DATA *ch, char *argument)
{
  char arg[MSL];
  argument = one_argument(argument, arg);

  if (arg[0] == '\0')
    send_to_char("Syntax: Show <Affects|Stats>\n\r",ch);
  else if ( !str_prefix(arg, "stats" ) )
  {
    if (IS_SET(ch->comm_flags,COMM_STATS_SHOW))
    {
      send_to_char("Statistics will no longer be shown in score.\n\r",ch);
      REMOVE_BIT(ch->comm_flags,COMM_STATS_SHOW);
    }
    else
    {
      send_to_char("Statistics will now be shown in score.\n\r",ch);
      SET_BIT(ch->comm_flags,COMM_STATS_SHOW);
    }
  }
  else if ( !str_prefix(arg, "affects" ) )
  {
    if (IS_SET(ch->comm_flags,COMM_SHOW_AFFECTS))
    {
      send_to_char("Affects will no longer be shown in score.\n\r",ch);
      REMOVE_BIT(ch->comm_flags,COMM_SHOW_AFFECTS);
    }
    else
    {
      send_to_char("Affects will now be shown in score.\n\r",ch);
      SET_BIT(ch->comm_flags,COMM_SHOW_AFFECTS);
    }
  }
  else
    send_to_char("Syntax: Show <Affects|Stats>\n\r",ch);
}

void do_statshow(CHAR_DATA *ch, char *argument)
{
  if (IS_SET(ch->comm_flags,COMM_STATS_SHOW))
  {
    send_to_char("Statistics will no longer be shown in score.\n\r",ch);
    REMOVE_BIT(ch->comm_flags,COMM_STATS_SHOW);
  }
  else
  {
    send_to_char("Statistics will now be shown in score.\n\r",ch);
    SET_BIT(ch->comm_flags,COMM_STATS_SHOW);
  }
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];

  if ( argument[0] == '\0' )
  {
    if (IS_SET(ch->comm_flags,COMM_PROMPT))
    {
      send_to_char("You will no longer see prompts.\n\r",ch);
      REMOVE_BIT(ch->comm_flags,COMM_PROMPT);
    }
    else
    {
      send_to_char("You will now see prompts.\n\r",ch);
      SET_BIT(ch->comm_flags,COMM_PROMPT);
    }
    return;
  }

  if (!strcmp(argument, "show" ) )
  {
    printf_to_char(ch, "Your prompt is %s\n\r", ch->prompt);
    return;
  }

  if ( IS_IMMORTAL(ch)
       && ( !strcmp(argument, "olc" )
            ||   !strcmp(argument, "OLC" ) ) )
  {
    if (IS_SET(ch->comm_flags, COMM_NO_OLC_PROMPT))
    {
      REMOVE_BIT(ch->comm_flags,COMM_NO_OLC_PROMPT);
      printf_to_char(ch, "You will now see the OLC prompt.\n\r");
    }
    else
    {
      SET_BIT(ch->comm_flags,COMM_NO_OLC_PROMPT);
      printf_to_char(ch, "You will no longer see the OLC prompt.\n\r");
    }
    return;
  }


  if ( !strcmp( argument, "all" ) )
    strcpy( buf, "{c<%hhp %mm %vmv>{x " );
  else if (!strcmp(argument, "new" ))
    mprintf(sizeof(buf), buf,"{W<{R%%h{W/{r%%HHP {C%%m{W/{c%%MMA {B%%v{W/{b%%VMV{W>{x");
  else
  {
    if ( strlen(argument) > 300 )
    {
      argument[300] = '\0';
      send_to_char("Are you stupid? Aren't 300 chars enough for you? Truncating.\n\r",ch);
    }


    strcpy( buf, argument );
    smash_tilde( buf );

    if (str_suffix("%c",buf))
      strcat(buf," ");

  }


  free_string( ch->prompt );
  strcat(buf,"{x");
  ch->prompt = str_dup( buf , ch->prompt);
#if MEMDEBUG
  free_string(ch->memdebug_prompt);
  ch->memdebug_prompt = str_dup (buf, ch->memdebug_prompt);
#endif
  printf_to_char(ch,"Prompt set to %s\n\r",ch->prompt );
  return;
}

void do_combine(CHAR_DATA *ch, char *argument)
{
  if (IS_SET(ch->comm_flags,COMM_COMBINE))
  {
    send_to_char("Long inventory selected.\n\r",ch);
    REMOVE_BIT(ch->comm_flags,COMM_COMBINE);
  }
  else
  {
    send_to_char("Combined inventory selected.\n\r",ch);
    SET_BIT(ch->comm_flags,COMM_COMBINE);
  }
}

void do_noloot(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  if ( IS_NULLSTR(argument) )
  {
    send_to_char( "Syntax: noloot <safe|lootable|morgue>\n\r", ch );
    return;
  }

  if ( !str_prefix(argument, "safe") )
  {
    send_to_char( "Your corpse is now safe from thieves.\n\r", ch );
    REMOVE_BIT(ch->act,PLR_MORGUE);
    REMOVE_BIT(ch->act,PLR_CANLOOT);
    return;
  }

  if ( !str_prefix(argument, "lootable" ) )
  {
    send_to_char( "Your corpse may now be looted.\n\r", ch );
    SET_BIT(ch->act,PLR_CANLOOT);
    REMOVE_BIT(ch->act,PLR_MORGUE);
    return;
  }

  if ( !str_prefix(argument, "morgue") )
  {
    send_to_char( "Your corpse will be delivered to the morgue.\n\r", ch );
    REMOVE_BIT(ch->act,PLR_CANLOOT);
    SET_BIT(ch->act,PLR_MORGUE);
    return;
  }

  send_to_char( "Syntax: noloot <safe|lootable|morgue>\n\r", ch );
}

void do_nofollow(CHAR_DATA *ch, char *argument)
{
  /*if ( IS_NPC(ch) || IS_AFFECTED( ch, AFF_CHARM ) )
    return;

  if (IS_SET(ch->act,PLR_NOFOLLOW))
    {
      send_to_char("You now accept followers.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    }
  else
    {
      send_to_char("You no longer accept followers.\n\r",ch);
      SET_BIT(ch->act,PLR_NOFOLLOW);
      die_follower( ch );
    }*/

  char        arg[MAX_INPUT_LENGTH];
  CHAR_DATA   *fch;

  one_argument( argument, arg );

  if ( IS_SET( ch->act, PLR_NOFOLLOW )
       &&   ( arg[0] == '\0' || !str_cmp( arg, "off" ) ) )
  {
    send_to_char( "You now accept followers.\n\r", ch );
    REMOVE_BIT( ch->act, PLR_NOFOLLOW );
  }
  else if ( !IS_SET( ch->act, PLR_NOFOLLOW )
            &&   ( arg[0] == '\0' || !str_cmp( arg, "on" ) ) )
  {
    send_to_char( "You no longer accept followers.\n\r", ch );
    SET_BIT( ch->act, PLR_NOFOLLOW );
    if ( !IS_AFFECTED( ch, AFF_CHARM ) )
      die_follower( ch );
  }
  else if ( ( fch = get_char_room( ch, arg ) ) && fch->master == ch )
  {
    stop_follower( fch );
  }
  else
    printf_to_char( ch, "Nofollow remains %s.\n\r",
                    IS_SET( ch->act, PLR_NOFOLLOW ) ? "on" : "off" );
}

void do_nosummon(CHAR_DATA *ch, char *argument)
{
  if ( IS_NPC( ch ) )
  {
    if (IS_SET(ch->imm_flags,IMM_SUMMON))
    {
      send_to_char("You are no longer immune to summon.\n\r",ch);
      REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
    }
    else
    {
      send_to_char("You are now immune to summoning.\n\r",ch);
      SET_BIT(ch->imm_flags,IMM_SUMMON);
    }
  }
  else
  {
    if (IS_SET(ch->act,PLR_NOSUMMON))
    {
      send_to_char("You are no longer immune to summon.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NOSUMMON);
    }
    else
    {
      send_to_char("You are now immune to summoning.\n\r",ch);
      SET_BIT(ch->act,PLR_NOSUMMON);
    }
  }
}

void do_nogate(CHAR_DATA *ch, char *argument)
{
  if (ch->clan != NULL)
  {
    send_to_char("Clanned players cannot turn off gating.\n\r",ch);
    return;
  }

  if (IS_SET(ch->act,PLR_NOGATE))
  {
    send_to_char("You are no longer immune to gate.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_NOGATE);
  }
  else
  {
    send_to_char("You are now immune to gating.\n\r",ch);
    SET_BIT(ch->act,PLR_NOGATE);
  }
}

void do_nocancel(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
  {
    send_to_char("You are a NPC and cannot be canceled anyways.\n\r",ch);
  }
  else
  {
    if (IS_SET(ch->act,PLR_NOCANCEL))
    {
      send_to_char("You are no longer immune to cancellation.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NOCANCEL);
    }
    else
    {
      send_to_char("You are now immune to cancellation.\n\r",ch);
      SET_BIT(ch->act,PLR_NOCANCEL);
    }
  }
}
void do_press( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];

  one_argument( argument, arg );

  if ( IS_SET( ch->in_room->mprog_flags, TRIG_PRESS )
       &&   get_extra_descr( arg, ch->in_room->extra_descr ) )
    mp_press_trigger( ch, arg );
  else
    act("Nothing seems to happen.", ch, NULL, NULL, TO_CHAR );
}

void do_turn( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];

  one_argument( argument, arg );

  if ( IS_SET( ch->in_room->mprog_flags, TRIG_TURN )
       &&   get_extra_descr( arg, ch->in_room->extra_descr ) )
    mp_turn_trigger( ch, arg );
  else
    act("Nothing seems to happen.", ch, NULL, NULL, TO_CHAR );
}

void do_pull( CHAR_DATA *ch, char *argument )
{
  char arg [MAX_INPUT_LENGTH];

  one_argument( argument, arg );

  if ( IS_SET( ch->in_room->mprog_flags, TRIG_PULL )
       &&   get_extra_descr( arg, ch->in_room->extra_descr ) )
    mp_pull_trigger( ch, arg );
  else
    act("Nothing seems to happen.", ch, NULL, NULL, TO_CHAR );
}

void do_dig( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];

  one_argument( argument, arg );

  if ( IS_SET( ch->in_room->mprog_flags, TRIG_DIG )
       &&   get_extra_descr( arg, ch->in_room->extra_descr ) )
    mp_dig_trigger( ch, arg );
  else
    act("Nothing seems to happen.", ch, NULL, NULL, TO_CHAR );
}

void do_touch( CHAR_DATA *ch, char *argument )
{
  char arg [MAX_INPUT_LENGTH];

  one_argument( argument, arg );

  if ( IS_SET( ch->in_room->mprog_flags, TRIG_TOUCH )
       &&   get_extra_descr( arg, ch->in_room->extra_descr ) )
    mp_touch_trigger( ch, arg );
  else
    act("Nothing seems to happen.", ch, NULL, NULL, TO_CHAR );
}

void do_look( CHAR_DATA *ch, char *argument )
{
  BUFFER    *buffer;
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  char buf  [MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *room;
  EXIT_DATA       *pexit;
  CHAR_DATA       *victim;
  OBJ_DATA        *obj;
  OBJ_DATA        *obj2;
  char            *pdesc;
  int door;
  int number,count=0,ocount=0;


  if ( ch->desc == NULL )
    return;

  if ( ch->position < POS_SLEEPING )
  {
    send_to_char( "You can't see anything at all.\n\r", ch );
    return;
  }

  if ( ch->position == POS_SLEEPING )
  {
    send_to_char( "You can't see anything, you're sleeping.\n\r", ch );
    return;
  }

  if ( !check_blind( ch ) )
    return;

  if ( !ch->in_room )
  {
    bugf("ERROR Char is NOT in room. %s",interp_cmd);
    move_to_room( ch, get_room_index( ROOM_VNUM_ALTAR ) );
    return;
  }

  if ( !IS_NPC( ch )
       &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
       &&    room_is_dark( ch->in_room )
       &&   !IS_AFFECTED( ch, AFF_DARK_VISION ) )
  {
    send_to_char( "It is pitch black... \n\r", ch );
    show_char_to_char( ch->in_room->people, ch );
    return;
  }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  /* Introduced by Merak 06/08/11 to handle idiots that type "look at " ;) */
  if ( !str_cmp( arg1, "at" ) && arg2[0] )
  {
    strcpy( arg1, arg2 );
    arg2[0] = '\0';
  }


  if ( !str_cmp( arg1, "sky" ) )
  {
    if ( !IS_OUTSIDE(ch) )
    {
      send_to_char( "You can't see the sky indoors.\n\r", ch );
      return;
    }
    else
    {
      look_sky( ch );
      return;
    }
  } // "sky"

  number = number_argument(arg1,arg3);
  count = 0;

  if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
  {
    sprintf( buf, "{W%s{x", ch->in_room->name );
    send_to_char( buf, ch );

    if ( ( IS_IMMORTAL( ch ) && ( IS_NPC( ch )
                                  ||     IS_SET( ch->act, PLR_HOLYLIGHT ) ) )
         ||     IS_BUILDER( ch, ch->in_room->area ) )
    {
      printf_to_char( ch, " {g[{wR-%d{g] [{wA- %s{g]{x",
                      ch->in_room->vnum,
                      ch->in_room->area->name );
    }
    send_to_char( "\n\r", ch );

    if ( arg1[0] == '\0'
         || ( !IS_NPC( ch ) && !IS_SET( ch->comm_flags, COMM_BRIEF ) ) )
    {
      send_to_char( "  ", ch );

// hallucinating (poison)
      if ( IS_AFFECTED(ch, AFF_POISON)  /* 10% chance to see wrong room */
           &&  (number_percent() < 10 )
           &&  (toxin_type(ch) == TOX_HALLUCINOGENIC) )
      {
        // pick a random room from the world
        CHAR_DATA *vsearch=NULL;
        CHAR_DATA *mob_array[250];
        int nmobs = 0;

        for ( vsearch = char_list; vsearch; vsearch = vsearch->next )
        {
          if ( vsearch->in_room == NULL)
            continue; // Don't want mobs not in a room or area

          if (vsearch->in_room->area->low_range == -1
              || vsearch->in_room->area->high_range == -1)
            continue; // No unlinked areas

          if (IS_SET(vsearch->in_room->area->flags, AREA_DRAFT))
            continue; // No idea what this is, but might be important

          if (number_percent() > 5) // consider only 5% of all mobs
            continue;

          mob_array[nmobs++] = vsearch;
          if (nmobs >= 250)
            break;
        }

        if (nmobs >=1)
          send_to_char( mob_array[number_range(0,nmobs)]->in_room->description, ch );
        else // not found
          send_to_char( ch->in_room->description, ch );
      } // if hallucinating
      else
        send_to_char( ch->in_room->description, ch );
    } // if "look", no arg, no COMM_BRIEF

    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOEXIT ) )
    {
      send_to_char( "\n\r", ch );
      do_function( ch, &do_exits, "auto" );
    }

    show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE, FALSE, FALSE);
    show_char_to_char( ch->in_room->people,   ch );

    /* rescue quest mob found....now follow quester */
    if ( !IS_NPC( ch )
         && ( ch->questdata->mob_vnum > -1 ) )
    {
      CHAR_DATA *rch;
      for ( rch = ch->in_room->people ; rch ; rch = rch->next_in_room)
      {
        if ( !IS_NPC( rch ) )
          continue;

        if ( strstr(rch->name,ch->name )
             && ( rch->pIndexData->vnum == MOB_VNUM_QUEST )
             && ( rch->master == NULL ) )
        {
          if (IS_SET(ch->act,PLR_NOFOLLOW))
            REMOVE_BIT(ch->act,PLR_NOFOLLOW);

          if (ch->alignment > -500)
            sprintf( buf, "{x%s{& is thrilled to be rescued!{x\n\r", rch->short_descr );
          else
            sprintf( buf, "{x%s{& cowers in fear from her kidnapper!{x\n\r", rch->short_descr );

          send_to_char( buf, ch );
          do_function(rch,&do_follow,ch->name);
          break;
        }
      }
    } // if rescue_quest

    return;
  } // if "look" or "look auto"

  if ( !str_cmp( arg1, "i" ) || !str_cmp(arg1, "in")  || !str_cmp(arg1,"on"))
  {
    if ( arg2[0] == '\0' )
    {
      send_to_char( "Look in what?\n\r", ch );
      return;
    }

    if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
    {
      send_to_char( "You do not see that here.\n\r", ch );
      return;
    }

    switch ( obj->item_type )
    {
      default:
        send_to_char( "That is not a container.\n\r", ch );
        break;

      case ITEM_PORTAL:
        if ( IS_SET( obj->value[1], EX_CLOSED ) )
        {
          send_to_char( "It is closed.\n\r", ch );
          return;
        }

        if ( IS_SET( obj->value[1], EX_SEETHROUGH ) )
        {
          room = ch->in_room;
          ch->in_room = get_room_index( obj->value[3] );
          printf_to_char( ch, "You peer through %s...\n\r", obj->short_descr );
          do_function( ch, &do_look, "" );
          ch->in_room = room;
          break;
        }
        else // shouldn't this look into the room on the other side?  Maybe an object flag to allow it - Taeloch
        {
          send_to_char( "The portal is too blurred to make out the other side.\n\r", ch );
          break;
        }
        break; // end ITEM_PORTAL

      case ITEM_DRINK_CON:
        if ( obj->value[1] <= 0 )
        {
          send_to_char( "It is empty.\n\r", ch );
          break;
        }

        printf_to_char(ch, "It's %sfilled with a %s liquid.\n\r",
                       (obj->value[1] < ( obj->value[0] / 4))
                       ? "less than half-" :
                       (obj->value[1] < (3 * obj->value[0] / 4))
                       ? "about half-" : "more than half-",
                       liq_table[obj->value[2]].liq_color
                      );

        break; // end ITEM_DRINK_CON

      case ITEM_CONTAINER:
        if ( IS_SET(obj->value[1], CONT_CLOSED) )
        {
          send_to_char( "It is closed.\n\r", ch );
          break;
        }

        ocount = 0;
        for (obj2 = obj->contains; obj2 != NULL; obj2 = obj2->next_content)
        {
          ocount++;
        }

        if (IS_SET( obj->extra_flags, ITEM_PLURAL ))
        {
          sprintf( buf, "$p hold (%d/%d):", ocount, obj->value[0] );
          act( buf, ch, obj, NULL, TO_CHAR );
        }
        else
        {
          sprintf( buf, "$p holds (%d/%d):", ocount, obj->value[0] );
          act( buf, ch, obj, NULL, TO_CHAR );
        }

        /* WTF? -- Taeloch
                if ( IS_SET( obj->extra_flags, ITEM_DONATION_PIT ) )
                  show_list_to_char( obj->contains, ch, TRUE, TRUE, TRUE, FALSE );
                else
                  show_list_to_char( obj->contains, ch, TRUE, TRUE, TRUE, FALSE );
        */
        show_list_to_char( obj->contains, ch, TRUE, TRUE, TRUE, FALSE );
        break; // end ITEM_CONTAINER

      case ITEM_SPELLBOOK:
        act( "$p holds:", ch, obj, NULL, TO_CHAR );
        show_list_to_char( obj->contains, ch, TRUE, TRUE, TRUE, FALSE );
        break;

      case ITEM_KEYRING:
        act( "$p has:", ch, obj, NULL, TO_CHAR );
        show_list_to_char( obj->contains, ch, TRUE, TRUE, TRUE, FALSE );
        break;

      case ITEM_LOCKER:
        if (IS_NPC(ch))
          return;

        if ( ch->clan )
        {
          if ( ch->clan->locker != obj->pIndexData->vnum )
          {
            send_to_char("That isn't your locker.\n\r", ch);
            return;
          }
        }
        else
        {
          if ( obj->value[0] )
          {
            send_to_char( "That isn't your locker.\n\r", ch );
            return;
          }
        }

        if (ch->pcdata->locker_max <= 0)
        {
          send_to_char("You haven't been assigned a locker.\n\r",ch);
          return;
        }

        act( "Your locker holds:", ch, obj, NULL, TO_CHAR );
        show_list_to_char( ch->pcdata->locker, ch, TRUE, TRUE, TRUE, FALSE );
        break; // end ITEM_LOCKER

      case ITEM_CORPSE_NPC:
      case ITEM_CORPSE_PC:
        if ( IS_SET(obj->value[1], CONT_CLOSED) )
        {
          send_to_char( "It is closed.\n\r", ch );
          break;
        }

        act( "$p holds:", ch, obj, NULL, TO_CHAR );
        show_list_to_char( obj->contains, ch, TRUE, TRUE, FALSE, FALSE );
        break; // end ITEM_CORPSE_[N]PC

      case ITEM_MONEY_POUCH:
        if ( IS_SET(obj->value[1], CONT_CLOSED) )
        {
          send_to_char( "It is closed.\n\r", ch );
          break;
        }
        printf_to_char(ch,
                       "{yG{Yo{yld{x: {W%-6d {wSi{Wl{Dv{wer{x: {W%d\n\r",
                       ch->gold,
                       ch->silver);
        break; // end ITEM_MONEY_POUCH
    } // end switch(item_type)

    return;
  } // end "look [in|on]"

  if ( ( victim = get_char_room( ch, arg1 ) ) )
  {
    show_char_to_char_1( victim, ch );
    return;
  }

  if ( (victim = get_race_room( ch, arg1 ) ) )
  {
    show_char_to_char_1(victim, ch );
    return;
  }

// Look for extra_descs in room
  pdesc = get_extra_descr( arg3, ch->in_room->extra_descr );
  if ( pdesc )
  {
    if ( ++count == number )
    {
      send_to_char( pdesc, ch );
      return;
    }
  } // room extra_descs

// Loop through items in inventory and look for Extra Descriptions
  for ( obj = ch->carrying; obj; obj = obj->next_content )
  {
    if ( can_see_obj( ch, obj ) )
    {
      /* player can see object */

      pdesc = get_extra_descr( arg3, obj->extra_descr );
      if ( pdesc )
      {
        if ( ++count == number )
        {
          buffer = new_buf();
          add_buf( buffer, pdesc );
          page_to_char( buf_string( buffer ), ch );
          free_buf( buffer );
          return;
        }
        else
          continue;
      } // if (pdesc) on obj->extra_descr

      pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
      if ( pdesc )
      {
        if ( ++count == number )
        {
          buffer = new_buf();
          add_buf( buffer, pdesc );
          page_to_char( buf_string( buffer ), ch );
          free_buf( buffer );
          return;
        }
        else
          continue;
      } // if (pdesc) on obj->pIndexData->extra_descr

      if ( is_name( arg3, obj->name ) )
        if ( ++count == number )
        {
          send_to_char( obj->description, ch );
          send_to_char( "\n\r",ch);
          return;
        }
    } // if can_see_obj
  } // for obj->carrying

// Loop through items in room and look for Extra Descriptions
  for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
  {
    if ( obj->item_type == ITEM_BOOK )
      continue;

    if ( can_see_obj( ch, obj ) )
    {
      if (obj->item_type == ITEM_CHECKERS)
      {
        look_checkers(ch, obj);
        return;
      }

      pdesc = get_extra_descr( arg3, obj->extra_descr );
      if ( pdesc )
        if (++count == number)
        {
          send_to_char( pdesc, ch );
          return;
        }

      pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
      if ( pdesc )
        if (++count == number)
        {
          send_to_char( pdesc, ch );
          return;
        }

      if ( is_name( arg3, obj->name ) )
        if (++count == number)
        {
          send_to_char( obj->description, ch );
          send_to_char("\n\r",ch);
          return;
        }
    } // if can_see_obj
  } // for obj->in_room

  if ( count > 0 && count != number )
  {
    if ( count == 1 )
      printf_to_char( ch, "You only see one %s here.\n\r", arg3 );
    else
      printf_to_char( ch, "You only see %d of those here.\n\r", count );
    return;
  }

  for ( door = 0; door <= 5; door++ )
  {
    if ( ( pexit = ch->in_room->exit[door] )
         &&   pexit->keyword
         &&   is_name( arg1, pexit->keyword ) )
      break;
  }

  if ( door == 6 )
    if ( ( door = arg_to_dirnum( arg1 ) ) == -1 )
    {
      send_to_char( "You do not see that here.\n\r", ch );
      return;
    }

  /* 'look direction' */
  if ( IS_SET( ch->in_room->room_flags, ROOM_SHIP ) )
  {
    if ( ch->in_room->state < 0 )
      pexit = NULL;
    else if ( ( pexit = ch->in_room->exit[ch->in_room->state] ) == NULL
              ||      str_prefix( arg1, ch->in_room->exit[ch->in_room->state]->keyword ) )
      pexit = NULL;
    else
      pexit = ch->in_room->exit[ch->in_room->state];
  }
  else pexit = ch->in_room->exit[door];

  if ( pexit == NULL )
  {
    send_to_char( "Nothing special there.\n\r", ch );
    return;
  }

  if ( pexit->description && pexit->description[0]
       && ( !IS_SET( pexit->exit_info, EX_HIDDEN )
            ||    IS_SET( ch->act, PLR_HOLYLIGHT )
            ||    IS_AFFECTED( ch, AFF_DETECT_HIDDEN ) ) )
    send_to_char( pexit->description, ch );

  if ( !IS_SET( pexit->exit_info, EX_CLOSED )
       &&    IS_SET( pexit->exit_info, EX_SEETHROUGH ) )
  {
    if ( !IS_SET( ch->act, PLR_HOLYLIGHT )
         &&   !IS_AFFECTED( ch, AFF_DARK_VISION )
         &&    room_is_dark( pexit->u1.to_room ) )
      printf_to_char( ch, "Too dark to tell" );
    else
      printf_to_char( ch, "  %s", pexit->u1.to_room->description );
  }
  else if ( !IS_SET( pexit->exit_info, EX_ISDOOR )
            &&    !( pexit->description && pexit->description[0] ) )
  {
    send_to_char( "Nothing special there.\n\r", ch );
    return;
  }
  else if ( IS_SET( pexit->exit_info, EX_HIDDEN )
            &&  !IS_SET( ch->act, PLR_HOLYLIGHT )
            &&  !IS_AFFECTED( ch, AFF_DETECT_HIDDEN ) )
  {
    send_to_char(
      "Nothing special there. You just can't tell these days.\n\r", ch );
    return;
  }

  if ( !IS_SET( pexit->exit_info, EX_ISDOOR ) )
    return;

  if (IS_AFFECTED(ch, AFF_BLIND))
  {
    strcpy( buf, "The exit" );
  }
  else
  {
    strcpy( buf, "The " );
    if ( pexit->keyword
         &&   pexit->keyword[0]
         &&   pexit->keyword[0] != ' ' )
      strcat( buf, pexit->keyword );
    else
      if ( IS_SET( pexit->exit_info, EX_MULTI ) )
        strcat( buf, "doors" );
      else
        strcat( buf, "door" );

    if ( IS_SET( pexit->exit_info, EX_MULTI ) )
      strcat( buf, " are" );
    else
      strcat( buf, " is" );
  }

  if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
    printf_to_char( ch, "%s closed.\n\r", buf );
  else
    printf_to_char( ch, "%s open.\n\r", buf );

  return;
}

/* RT added back for the hell of it */
void do_read ( CHAR_DATA *ch, char *argument )
{
  //In a library, try to read a book instead of "look"
  if ( IS_SET(ch->in_room->area->flags, AREA_LIBRARY) )
    do_function( ch, &do_research, argument );
  else
    do_function( ch, &do_look, argument );
}

void do_examine( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  bool partial, frozen;
  OBJ_DATA *obj;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Examine what?\n\r", ch );
    return;
  }


  if ( ( obj = get_obj_here( ch, arg ) ) )
  {
    switch ( obj->item_type )
    {
      default:
        do_function( ch,&do_look, arg );
        break;

      case ITEM_SLOT_MACHINE:
        do_function( ch,&do_look, arg );
        if (obj->value[3] == 1)
          partial = TRUE;
        else
          partial = FALSE;
        if (obj->value[4] == 1)
          frozen = TRUE;
        else
          frozen = FALSE;
        mprintf(sizeof(buf), buf,
                "This is a %d bar slot machine costing {Y%d {yg{Yo{yld{x with a jackpot of {Y%d {yg{Yo{yld{x coins.\n\rThe jackpot is %sfrozen with %spartial winnings.",
                obj->value[2],
                obj->value[0],
                obj->value[1],
                frozen ?  "" : "not ",
                partial ? "" : "no " );
        send_to_char( buf, ch );
        break;
      case ITEM_JUKEBOX:
        do_function( ch, &do_look, arg );
        do_function( ch, &do_play, "list" );
        break;

      case ITEM_MONEY:
        do_function( ch,&do_look, arg );
        if (obj->value[0] == 0)
        {
          if (obj->value[1] == 0)
            mprintf(sizeof(buf), buf,"Odd...there's no coins in the pile.\n\r");
          else if (obj->value[1] == 1)
            mprintf(sizeof(buf), buf,"Wow. One {yg{Yo{yld{x coin.\n\r");
          else
            mprintf(sizeof(buf), buf,"There are {Y%d {yg{Yo{yld {xcoins in the pile.\n\r",
                    obj->value[1]);
        }
        else if (obj->value[1] == 0)
        {
          if (obj->value[0] == 1)
            mprintf(sizeof(buf), buf,"Wow. One {wsi{Wl{Dv{wer{x coin.\n\r");
          else
            mprintf(sizeof(buf), buf,"There are {W%d {wsi{Wl{Dv{wer{x coins in the pile.\n\r",
                    obj->value[0]);
        }
        else
          mprintf(sizeof(buf), buf,
                  "There are {Y%d {yg{Yo{yld{x and {W%d {wsi{Wl{Dv{wer{x coins in the pile.\n\r",
                  obj->value[1],obj->value[0]);
        send_to_char(buf,ch);
        break;

      case ITEM_DRINK_CON:
      case ITEM_CONTAINER:
      case ITEM_SPELLBOOK:
      case ITEM_KEYRING:
      case ITEM_CORPSE_NPC:
      case ITEM_CORPSE_PC:
        mprintf(sizeof(buf), buf,"in %s",argument);
        do_function( ch, &do_look, buf );
    }
    return;
  }
  do_function( ch, &do_look, arg);
  return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
  extern char * const dir_name[];
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_INPUT_LENGTH];
  /*  char *color_closed_dir_name[] =
  {
    "{mn{Mo{Wr{Mt{mh{x", "{me{Mas{mt{x", "{ms{Mo{Wu{Mt{mh{x",  "{mw{Mes{mt{x",
    "{mu{Mp{x", "{md{Mow{mn{x"
  };
  char *nocolor_closed_dir_name[] =
  {    "NORTH", "EAST", "SOUTH", "WEST", "UP", "DOWN" };*/
  EXIT_DATA *pexit;
  bool found;
  bool fAuto;
  int door;
  bool    fSeeDark;
  bool    fHolyLight;

  if ( ch->in_room == NULL )
    return;

  if ( !check_blind( ch ) )
    return;

  fAuto  = !str_cmp( argument, "auto" );

  fHolyLight = !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT );
  fSeeDark = IS_AFFECTED( ch, AFF_DARK_VISION ) || fHolyLight;


  if ( fAuto )
    mprintf(sizeof(buf), buf,"{D{m[Exits:{x");
  else if ( IS_IMMORTAL( ch ) )
    if ( fHolyLight )
      mprintf(sizeof(buf), buf,"Exits from Room - %d:\n\r",ch->in_room->vnum);
    else
      mprintf(sizeof(buf), buf,"Obvious exits from Room - %d:\n\r",
              ch->in_room->vnum );
  else
    mprintf(sizeof(buf), buf, "Obvious exits:\n\r" );

  found = FALSE;
  if ( IS_SET( ch->in_room->room_flags, ROOM_SHIP ) )
  {
    if ( ch->in_room->state >= 0 )
    {
      if ( ( pexit = ch->in_room->exit[ch->in_room->state] ) )
      {
        if ( (  pexit->u1.to_room && can_see_room( ch,pexit->u1.to_room )
                &&   ( !IS_SET( pexit->exit_info, EX_HIDDEN )
                       ||      IS_AFFECTED(ch, AFF_DETECT_HIDDEN )
                       ||      IS_SET( ch->act, PLR_HOLYLIGHT ) ) ) )
        {
          found = TRUE;
          if ( pexit->keyword )
            strcat( strcpy( buf1, " " ), pexit->keyword );
          else
            strcpy( buf1, " Void" );

          if ( fAuto ) strcat( buf, buf1 );
          else
          {
            sprintf( buf + strlen(buf), "%-5s - %s",
                     capitalize( buf1 ),
                     ( room_is_dark( pexit->u1.to_room ) && !fSeeDark )
                     ? "Too dark to tell" : pexit->u1.to_room->name );
            if ( IS_IMMORTAL( ch ) ) sprintf( buf + strlen( buf ),
                                                " (room %d)\n\r",pexit->u1.to_room->vnum );
            else
              sprintf( buf + strlen( buf ), "\n\r");
          }
        }
      }
    }
  }

  else
  {
    for ( door = 0; door <= 5; door++ )
    {
      /*      CRASH POINT, ADDING CHECKS TO VERIFY WHERE */
      if ( ( ( pexit = ch->in_room->exit[door] )
             &&   !IS_SET( pexit->exit_info, EX_NOEXIT ) )
           &&    pexit->u1.to_room
           &&    can_see_room( ch, pexit->u1.to_room )
           &&   ( !IS_SET( pexit->exit_info, EX_HIDDEN )
                  ||      IS_AFFECTED( ch, AFF_DETECT_HIDDEN )
                  ||      IS_SET( ch->act, PLR_HOLYLIGHT ) ) )
      {
        found = TRUE;
        if ( fAuto )
        {
          strcat( buf, " " );
          if ( IS_SET( pexit->exit_info, EX_CLOSED )
               ||   IS_SET( pexit->exit_info, EX_LOCKED ) )
          {
            /* strcat(buf, "(closed)");*/
            strcat(buf,"{D(");
            strcat( buf, dir_name[door] );
            strcat(buf,"{D){x");
          }
          else
            strcat( buf, dir_name[door] );
        }
        else
        {
          sprintf(buf + strlen(buf), "%-5s - %s",
                  capitalize( dir_name[door] ),
                  IS_SET(pexit->exit_info, EX_CLOSED) && !fHolyLight
                  ? "Closed Exit"
                  : (room_is_dark( pexit->u1.to_room )
                     && !fSeeDark )
                  ?  "Too dark to tell"
                  : pexit->u1.to_room->name );
          if (IS_IMMORTAL(ch))
            sprintf(buf + strlen(buf),
                    " (room %d)\n\r",pexit->u1.to_room->vnum);
          else
            sprintf(buf + strlen(buf), "\n\r");
        }
      }
    }
  }

  if ( !found )
    strcat( buf, fAuto ? " none" : "None.\n\r" );

  if ( fAuto )
    strcat( buf, "{m]{x\n\r" );

  send_to_char( buf, ch );
  return;
}

void do_worth( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  OBJ_DATA *pObj;
  int count = 0;

  for (obj = ch->carrying; obj; obj = obj->next_content)
  {
    if (IS_DIAMOND(obj))
      count++;
    if (obj->item_type == ITEM_CONTAINER)
      count += get_diamond_container_number(obj);
  }

  if (IS_NPC(ch))
  {
    printf_to_char(ch,"You have %ld {yg{Yo{yld{x\n\r%ld {wsi{Wl{Dv{wer{x, and %d {cdi{Camo{cnd%s{x.\n\r",
                   ch->gold,ch->silver, count, (count == 1) ? "" : "s");
    return;
  }

  printf_to_char(ch,
                 "{yG{Yo{yld{w:{x     %ld\n\r{wSi{Wl{Dv{wer:{x   %ld\n\rExp:{x      %-8d  ",
                 ch->gold, ch->silver, ch->exp);

  if ( ch->level < LEVEL_HERO )
    printf_to_char(ch, "(%d exp to level)\n\r",
                   TNL( exp_per_level( ch, ch->pcdata->points ), ch->level) - ch->exp );
  else
    printf_to_char(ch, "\n\r" );

  if ( ch->clan && !IS_IMMORTAL( ch ) )
    if ( ( pObj = find_obj_vnum( ch->clan->donation_gem ) ) )
    {
      printf_to_char( ch,
                      "Donated:  %-8d (%d)  (%s){x.\n\r",
                      ch->pcdata->donated_dia,
                      ch->clan->donation_balance,
                      pObj->short_descr );
    }

  if ( ch->pcdata->balance > 0 )
    printf_to_char( ch, "Your current bank balance is %d{x.\n\r",
                    ch->pcdata->balance );

  if ( ch->pcdata->locker_max )
    printf_to_char( ch, "Locker{w:{x %d/%d\n\r",
                    ch->pcdata->locker_content, ch->pcdata->locker_max );

  return;
}

int get_diamond_container_number(OBJ_DATA *container)
{
  OBJ_DATA *obj;
  int count = 0;

  for (obj = container->contains; obj; obj = obj->next_content)
  {
    if (IS_DIAMOND(obj))
      count++;
    if (obj->item_type == ITEM_CONTAINER)
      count += get_diamond_container_number(obj);
  }

  return count;
}


void do_oldscore( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  int i;


  printf_to_char(ch,
                 "You are %s%s, level %d, %d years old (%d hours).\n\r",
                 ch->name,
                 IS_NPC(ch) ? "" : ch->pcdata->title,
                 ch->level, get_age(ch),
                 TOTAL_PLAY_TIME(ch) / 3600);

  if ( get_trust( ch ) != ch->level )
  {
    printf_to_char(ch, "You are trusted at level %d.\n\r",
                   get_trust( ch ) );
  }

  printf_to_char(ch, "Race: %s  Sex: %s  Class: %s\n\r",
                 race_table[ch->race].name,
                 ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
                 IS_NPC(ch) ? "mobile" : class_table[ch->gameclass].name);


  printf_to_char(ch,
                 "You have %d/%d hit, %d/%d mana, %d/%d movement.\n\r",
                 ch->hit,  GET_HP(ch),
                 ch->mana, GET_MANA(ch),
                 ch->move, ch->max_move);

  printf_to_char(ch,
                 "You have %d practices and %d training sessions.\n\r",
                 ch->practice, ch->train);

  printf_to_char(ch,
                 "You are carrying %d/%d items with weight %ld/%d pounds.\n\r",
                 ch->carry_number, can_carry_n(ch),
                 get_carry_weight(ch) / 10, can_carry_w(ch) /10 );

  printf_to_char(ch,
                 "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
                 ch->perm_stat[STAT_STR],
                 get_curr_stat(ch,STAT_STR),
                 ch->perm_stat[STAT_INT],
                 get_curr_stat(ch,STAT_INT),
                 ch->perm_stat[STAT_WIS],
                 get_curr_stat(ch,STAT_WIS),
                 ch->perm_stat[STAT_DEX],
                 get_curr_stat(ch,STAT_DEX),
                 ch->perm_stat[STAT_CON],
                 get_curr_stat(ch,STAT_CON) );

  /*
   * This section (exp, gold, balance and shares) was written by
   * The Maniac from Mythran Mud
   */
  if (IS_NPC(ch))             /* NPC's have no bank balance and shares */
  {
    /* and don't level !!       -- Maniac -- */
    printf_to_char(ch,
                   "You have scored {g%d {Dexp{x, and have {Y%ld {yg{Yo{yld{x coins.\n\r",
                   ch->exp,  ch->gold );
  }

  if (!IS_NPC(ch))            /* PC's do level and can have bank accounts */
  {
    /* HERO's don't level anymore               */
    printf_to_char(ch,
                   "You have scored %d exp, and need %d exp to level. You have %d Quest Points\n\r",
                   ch->exp,
                   IS_HERO(ch) ?  1 :
                   ( TNL(exp_per_level( ch, ch->pcdata->points ), ch->level) - ch->exp ),
                   ch->questdata->curr_points );
    /*((ch->level+1)*1000)-ch->exp);*/
    printf_to_char(ch,
                   "You have %ld gold in cash, %d gold in the bank, %d Silver in hand\n\r",
                   ch->gold, ch->pcdata->balance, ch->silver);
    if ( ch->pcdata->shares )
    {
      printf_to_char(ch,"You have %d gold invested in %d shares (%d each).\n\r",
                     (ch->pcdata->shares * share_value),
                     ch->pcdata->shares, share_value);
    }
  }

  /* OLD CODE BEFORE BANKING
     mprintf(sizeof(buf), buf,
     "You have scored %d exp, and have %ld gold and %ld silver coins.\n\r",
     ch->exp,  ch->gold, ch->silver );
     send_to_char( buf, ch );
  */
  /* RT shows exp to level */
  if ( !IS_NPC(ch) && ch->level < LEVEL_HERO )
  {
    printf_to_char(ch,
                   "You need %d exp to level.\n\r",
                   TNL( exp_per_level( ch, ch->pcdata->points ), ch->level ) -
                   ch->exp );
  }

  printf_to_char(ch, "Wimpy set to %d hit points.\n\r", ch->wimpy );

  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   >= 5 )
    send_to_char( "You are drunk.\n\r",   ch );
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
    send_to_char( "You are thirsty.\n\r", ch );
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_HUNGER]   ==  0 )
    send_to_char( "You are hungry.\n\r",  ch );

  switch ( ch->position )
  {
    case POS_DEAD:
      send_to_char( "You are {rDEAD{x!!\n\r",    ch );
      break;
    case POS_MORTAL:
      send_to_char( "You are mortally wounded.\n\r",  ch );
      break;
    case POS_INCAP:
      send_to_char( "You are incapacitated.\n\r",  ch );
      break;
    case POS_STUNNED:
      send_to_char( "You are stunned.\n\r",    ch );
      break;
    case POS_SLEEPING:
      send_to_char( "You are sleeping.\n\r",    ch );
      break;
    case POS_RESTING:
      send_to_char( "You are resting.\n\r",    ch );
      break;
    case POS_SITTING:
      send_to_char( "You are sitting.\n\r",    ch );
      break;
    case POS_STANDING:
      send_to_char( "You are standing.\n\r",    ch );
      break;
    case POS_FIGHTING:
      send_to_char( "You are fighting.\n\r",    ch );
      break;
  }


  /* print AC values */
  if ( ch->level >= LEVEL_REVEAL_AC )
  {
    printf_to_char(ch,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
                   GET_AC(ch,AC_PIERCE),
                   GET_AC(ch,AC_BASH),
                   GET_AC(ch,AC_SLASH),
                   GET_AC(ch,AC_EXOTIC));
  }

  for ( i = 0; i < 4; i++ )
  {
    char *temp;

    switch (i)
    {
      case( AC_PIERCE ):  temp = "piercing";
        break;
      case( AC_BASH ):      temp = "bashing";
        break;
      case( AC_SLASH ):      temp = "slashing";
        break;
      case( AC_EXOTIC ):  temp = "magic";
        break;
      default:
        temp = "error";
        break;
    }

    send_to_char( "You are ", ch );

    if ( GET_AC( ch, i ) >=  101 )
      mprintf(sizeof(buf), buf,"hopelessly vulnerable to %s.\n\r",temp);
    else if (GET_AC(ch,i) >= 80)
      mprintf(sizeof(buf), buf,"defenseless against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= 60)
      mprintf(sizeof(buf), buf,"barely protected from %s.\n\r",temp);
    else if (GET_AC(ch,i) >= 40)
      mprintf(sizeof(buf), buf,"slightly armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= 20)
      mprintf(sizeof(buf), buf,"somewhat armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= 0)
      mprintf(sizeof(buf), buf,"armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -20)
      mprintf(sizeof(buf), buf,"well-armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -40)
      mprintf(sizeof(buf), buf,"very well-armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -60)
      mprintf(sizeof(buf), buf,"heavily armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -80)
      mprintf(sizeof(buf), buf,"superbly armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -100)
      mprintf(sizeof(buf), buf,"almost invulnerable to %s.\n\r",temp);
    else
      mprintf(sizeof(buf), buf,"divinely armored against %s.\n\r",temp);

    send_to_char( buf, ch );
  }


  /* RT wizinvis and holy light */
  if ( IS_IMMORTAL(ch))
  {
    send_to_char( "Holy Light: ", ch );
    if (IS_SET(ch->act,PLR_HOLYLIGHT))
      send_to_char( "on", ch );
    else
      send_to_char( "off", ch );

    if (ch->invis_level)
    {
      printf_to_char(ch, "  Invisible: level %d",ch->invis_level);
    }

    if (ch->incog_level)
    {
      printf_to_char(ch,"  Incognito: level %d",ch->incog_level);
    }
    send_to_char("\n\r",ch);
  }

  if ( ch->level >= LEVEL_REVEAL_AC )
  {
    printf_to_char(ch, "Hitroll: %d  Damroll: %d.\n\r",
                   GET_HITROLL(ch), GET_DAMROLL(ch) );
  }

  if ( ch->level >= LEVEL_REVEAL_ALIGN )
  {
    printf_to_char(ch, "Alignment: %d.  ", ch->alignment );
  }

  send_to_char( "You are ", ch );
  if ( ch->alignment >  900 ) send_to_char( "angelic.\n\r", ch );
  else if ( ch->alignment >  700 ) send_to_char( "saintly.\n\r", ch );
  else if ( ch->alignment >  350 ) send_to_char( "good.\n\r",    ch );
  else if ( ch->alignment >  100 ) send_to_char( "kind.\n\r",    ch );
  else if ( ch->alignment > -100 ) send_to_char( "neutral.\n\r", ch );
  else if ( ch->alignment > -350 ) send_to_char( "mean.\n\r",    ch );
  else if ( ch->alignment > -700 ) send_to_char( "evil.\n\r",    ch );
  else if ( ch->alignment > -900 ) send_to_char( "demonic.\n\r", ch );
  else                             send_to_char( "satanic.\n\r", ch );

  if (IS_SET(ch->comm_flags,COMM_SHOW_AFFECTS))
    do_function(ch,&do_affects,"");
}

void do_affects(CHAR_DATA *ch, char *argument )
{
  AFFECT_DATA *paf;
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  bool found = FALSE;
  int iWear = 0;
  int flag = 0;

  if ( !IS_NULLSTR( argument ) )
  {
    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
      victim = ch;

    if ( ch != victim )
    {
      if ( !IS_IMMORTAL( ch )
           && ( !is_same_group( ch, victim )
                &&   victim->clan != ch->clan )
           && ( ch->clan && !IS_SET( ch->clan->clan_flags, CLAN_GWYLIAD )) )
      {
        send_to_char( "You are unable to detect their affects.\n\r", ch );
        return;
      }

      if ( !IS_AFFECTED( ch, AFF_DETECT_MAGIC )
           && !IS_IMMORTAL( ch ) )
      {
        send_to_char(
          "You are unable to detect magical affects on others.\n\r", ch );
        return;
      }

      if (!IS_IMMORTAL( ch )
          &&   IS_NPC( victim )
          &&  !is_same_group( ch, victim ) )
      {
        send_to_char(
          "You are unable to detect magical affects on others.\n\r", ch );
        return;
      }
    }
  }
  else
    victim = ch;

  printf_to_char( ch,
                  "%s are affected by the following spells:\n\r\n\r",
                  victim == ch ? "You" : "They"  );
  send_to_char(
    "{gLvl  Spells           Duration Mod  ApplyTo            Adds\n\r",ch);
  send_to_char(
    "{g*{D-----------------------------------------------------------------------{g*{x\n\r",ch);
// Show racial affects first (at any level)
  for ( flag = 0; affect_flags[flag].name; flag++)
  {
    if ( !is_stat( affect_flags ) && IS_SET(race_table[victim->race].aff, affect_flags[flag].bit) )
    {
      found = TRUE;
      printf_to_char(ch, "{D%3d {g[{DRacial          {g]{D  Permanent  {g[{Dnone            {g] {g[{W%-16s{g]{x\n\r",
                     victim->level,
                     affect_flags[flag].name );
    }
    else
    {
      if ( affect_flags[flag].bit == race_table[victim->race].aff )
      {
        found = TRUE;
        printf_to_char(ch, "{D%3d {g[{DRacial          {g]{D  Permanent  {g[{Dnone            {g] {g[{W%-16s{g]{x\n\r",
                       victim->level,
                       affect_flags[flag].name );
        break;
      }
    }
  }

  for ( flag = 0; affect2_flags[flag].name; flag++)
  {
    if ( !is_stat( affect2_flags ) && IS_SET(race_table[victim->race].aff2, affect2_flags[flag].bit) )
    {
      found = TRUE;
      printf_to_char(ch, "{D%3d {g[{DRacial          {g]{D  Permanent  {g[{Dnone            {g] {g[{W%-16s{g]{x\n\r",
                     victim->level,
                     affect2_flags[flag].name );
    }
    else
    {
      if ( affect2_flags[flag].bit == race_table[victim->race].aff2 )
      {
        found = TRUE;
        printf_to_char(ch, "{D%3d {g[{DRacial          {g]{D  Permanent  {g[{Dnone            {g] {g[{W%-16s{g]{x\n\r",
                       victim->level,
                       affect2_flags[flag].name );
        break;
      }
    }
  }

// Show object affects second
  for ( iWear = 0; iWear < (MAX_WEAR+1); iWear++ )
  {
    if (iWear == MAX_WEAR + 1 )
    {
      if ( ( obj = get_eq_char(victim, WEAR_WIELD)) == NULL )
        continue;
    }
    else
    {
      if ( ( obj = get_eq_char( victim, iWear ) ) == NULL )
        continue;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
      if (paf->bitvector)
      {
        switch (paf->where)
        {
          case TO_AFFECTS:
            strncpy_color(buf, obj->short_descr, 16, ' ', TRUE );
            if ( ch->level >= LEVEL_REVEAL_AC )
              printf_to_char(ch, "{D%3d {g[{D%-16s{g]{D Object Aff  {g[{D%-16s{g] {g[{W%-16s{g]{x{x\n\r",
                             paf->level,
                             buf,
                             affect_loc_name( paf->location ),
                             affect_bit_name( paf->bitvector ) );
            else
              printf_to_char(ch, "{D%3d {g[{DUnknown Item    {g]{D  Object Aff {g[{DUnk             {g] {g[{W%-16s{g]{x\n\r",
                             paf->level,
//                obj->name,
                             affect_bit_name( paf->bitvector ) );
            found = TRUE;
            break;

          case TO_AFFECTS2:
            strncpy_color(buf, obj->short_descr, 16, ' ', TRUE );
            if ( ch->level >= LEVEL_REVEAL_AC )
              printf_to_char(ch, "{D%3d {g[{D%-16s{g]{D Object Aff  {g[{D%-16s{g] {g[{W%-16s{g]{x{x\n\r",
                             paf->level,
                             buf,
                             affect_loc_name( paf->location ),
                             affect2_bit_name( paf->bitvector ) );
            else
              printf_to_char(ch, "{D%3d {g[{DUnknown Item    {g]{D  Object Aff {g[{DUnk             {g] {g[{W%-16s{g]{x\n\r",
                             paf->level,
//                obj->name,
                             affect_bit_name( paf->bitvector ) );
            found = TRUE;
            break;

          default:
            break;
        } // switch paf
      } // if bit vector
    } // Object Aff loop

    if (!obj->enchanted)
    {
      for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
      {
        if (paf->bitvector)
        {
          switch (paf->where)
          {
            case TO_AFFECTS:
              strncpy_color(buf, obj->short_descr, 16, ' ', TRUE );
              if ( ch->level >= LEVEL_REVEAL_AC )
                printf_to_char(ch, "{D%3d {g[{D%-16s{g]{D Object Aff  {g[{D%-16s{g] {g[{W%-16s{g]{x\n\r",
                               paf->level,
                               buf,
                               affect_loc_name( paf->location ),
                               affect_bit_name( paf->bitvector ) );
              else
                printf_to_char(ch, "{D%3d {g[{DUnknown Item    {g]{D  Object Aff {g[{DUnk             {g] {g[{W%-16s{g]{x\n\r",
                               paf->level,
//                obj->name,
                               affect_bit_name( paf->bitvector ) );
              found = TRUE;
              break;
            case TO_AFFECTS2:
              strncpy_color(buf, obj->short_descr, 16, ' ', TRUE );
              if ( ch->level >= LEVEL_REVEAL_AC )
                printf_to_char(ch, "{D%3d {g[{D%-16s{g]{D Object Aff  {g[{D%-16s{g] {g[{W%-16s{g]{x\n\r",
                               paf->level,
                               buf,
                               affect_loc_name( paf->location ),
                               affect2_bit_name( paf->bitvector ) );
              else
                printf_to_char(ch, "{D%3d {g[{DUnknown Item    {g]{D  Object Aff {g[{DUnk             {g] {g[{W%-16s{g]{x\n\r",
                               paf->level,
//                obj->name,
                               affect_bit_name( paf->bitvector ) );
              found = TRUE;
              break;

            default:
              break;
          } // switch paf (!enchanted)
        } // if bit vector (!enchanted)
      } // Object Aff loop (!enchanted)
    } // if not enchanted
  } // worn-objects loop

// Show spell affects third
  for ( paf = victim->affected; paf; paf = paf->next )
  {
    if (paf->where == TO_AFFECTS)
      mprintf( sizeof( buf ), buf, "{g[{W%-16s{g]{x",
               flag_string( affect_flags ,  paf->bitvector ) );
    else if ( paf->where == TO_SPELL_AFFECTS )
      mprintf( sizeof( buf ), buf, "{g[{W%-16s{g]{x",
               flag_string( spell_affect_flags ,  paf->bitvector ) );
    else
      mprintf( sizeof( buf ), buf, "{g[{W%-16s{g]{x","none" );


    if ( ch->level >= LEVEL_REVEAL_AC )
    {
      strcpy(buf2,skill_table[(int) paf->type].name);
      buf2[16] = '\0';
      printf_to_char( ch,
                      "{D%3d {g[{D%-16s{g]{D%5d %5d  {g[{D%-16s{g] %s{x\n\r",
                      paf->level,
                      buf2,
                      paf->duration,
                      paf->modifier,
                      affect_loc_name( paf->location ),
                      buf );
    }
    else
    {
      strncpy_color(buf2, skill_table[(int) paf->type].name, 16, ' ', TRUE );
      mprintf( sizeof( buf ), buf, "{g[{D%-16s{g]{x","Unk" );
      printf_to_char( ch,
                      "{W%3s {g[{D%-16s{g]{D %5s %5s {g[{D%-16s{g] %s{x\n\r",
                      "Unk",
                      buf2,
                      "Unk",
                      "Unk",
                      "Unk",
                      buf );
    }
    found = TRUE;

  }

  if (ch->daze > 0)
    printf_to_char(ch, "{DN/A {g[{Dstunned         {g]{D    -     0  {g[{Dnone            {g] {g[{Wnone            {g]{x\n\r");

  if (!found)
    printf_to_char(ch,"{WNo affects.\n\r");

  send_to_char(
    "{g*{D-----------------------------------------------------------------------{g*{x\n\r",ch);
  return;
}

char *  const  day_name  [] =
{
  "the {cM{Do{bo{cn{x", "the {DB{yu{wl{yl{x", "{DDecept{wion{x",
  "{BTh{Du{Bn{Dd{ber{x", "{cFree{Bd{Ro{Bm{x", "the {cG{great {mG{yods{x",
  "the {YSu{yn{x"
};

char *  const  month_name  [] =
{
  "{WW{ci{bnter{x", "the {WW{ci{bnter {wW{Wo{Dlf{x",
  "the {cFr{wost {gGi{Dant{x", "the {DOld {mF{Bo{mrces{x",
  "the {yGrand {rStrugg{wl{re{x", "the {mSpr{ci{wn{mg{x",
  "{gN{Ga{yt{gur{ce{x", "{rF{Du{rt{Di{rl{Di{rty{x",
  "the {gDrag{Go{Dn{x", "the {yS{Yu{yn{x",
  "the {rH{Ye{ra{yt{x", "the {rBa{Rtt{yl{re{x",
  "the {DDark {wSh{Wa{wdes{x", "the {DShado{ww{bs{x",
  "the {cLong {DShado{ww{bs{x", "the {yAnc{Di{we{ynt {DDark{yn{wess{x",
  "the {DGre{ra{wt {rEv{Di{rl{x"
};

void do_time( CHAR_DATA *ch, char *argument )
{
  extern time_t boot_time;
  //extern char str_boot_time[];
  char *suf;
  int day;
  int day2;
  char buf[MSL];
  int boot_minute, boot_second, boot_hour, boot_day;


  if ( argument[0] == '\0' )
  {

    if ( time_info.hour == 0 )
      sprintf( buf, ", midnight" );
    else if ( time_info.hour == 12 )
      sprintf( buf, ", noon" );
    else if ( time_info.hour > 0 && time_info.hour <= 10 )
      sprintf( buf, " in the morning" );
    else if ( time_info.hour > 10 && time_info.hour <= 16 )
      sprintf( buf, " in the afternoon" );
    else if ( time_info.hour > 16 && time_info.hour < 21 )
      sprintf( buf, " in the evening" );
    else
      sprintf( buf, " at night" );

    day     = time_info.day + 1;

    if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    send_to_char("\n\rTime for the {gLand{cs{x of {RD{rr{Da{wk{Dn{ro{Rr{x.\n\r",ch);

    printf_to_char(ch,
                   "\n\rThe current time is %d o'clock%s, on the day of %s.\n\r"
                   "It is the %d%s day of the month of %s.\n\r"
                   "The current year is %d.\n\r\n\r",
                   (time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
                   buf,
                   //time_info.hour >= 12 ? "after" : "before",
                   day_name[day % 7],
                   day, suf,
                   month_name[time_info.month],
                   time_info.year);

    send_to_char("System date and time information\n\r",ch);
    strftime( buf, MSL,"{gSystem started{B:{x  %A, %B %d, %Y at %X %Z.\n\r",
              localtime( &tml_boot_time ) );
    send_to_char( buf, ch );
    strftime( buf, MSL,"{gLast Copyover{B:{x   %A, %B %d, %Y at %X %Z.\n\r",
              localtime( &boot_time ) );
    send_to_char( buf, ch );
    strftime( buf, MSL,"{gCurrent Time{B:{x    %A, %B %d, %Y at %X %Z.\n\r",
              localtime( &current_time ) );
    send_to_char( buf, ch );


    //take the difference, put them into seconds.
    boot_second    = (long)difftime( current_time, tml_boot_time );
    //convert to minutes
    boot_minute  = boot_second / 60;
    boot_second %= 60;
    //convert to hours
    boot_hour    = boot_minute / 60;
    boot_minute %= 60;
    //convert to days
    boot_day     = boot_hour / 24;
    boot_hour   %= 24;

    printf_to_char( ch, "Time since last crash: %02ldd %02ldh %02ldm %02lds\n\r",
                    boot_day, boot_hour, boot_minute, boot_second );
    return;

  }

  if ( !str_prefix( argument, "birthday" ) )
  {
    day2 = ch->birthday.day + 1;
    if ( day2 > 4 && day2 <  20 ) suf = "th";
    else if ( day2 % 10 ==  1       ) suf = "st";
    else if ( day2 % 10 ==  2       ) suf = "nd";
    else if ( day2 % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    printf_to_char(ch,
                   "You were born at %d o'clock %s mid-day, on the day of %s.\n\r%d%s of the month of %s, in the year %d.\n\r\n\r",
                   (ch->birthday.hour % 12 == 0) ? 12 : ch->birthday.hour % 12,
                   ch->birthday.hour >= 12 ? "after" : "before",
                   day_name[day2 % 7],
                   day2, suf,
                   month_name[ch->birthday.month],
                   ch->birthday.year-17);
    return;
  }
}



void do_weather( CHAR_DATA *ch, char *argument )
{

  static char * const sky_look[4] =
  {
    "cloudless",
    "cloudy",
    "rainy",
    "lit by {Yflashes{x of {Wlightning{x"
  };

  if (ch->in_room->sector_type == SECT_WATER_SWIM)
  {
    send_to_char( "You can't see the weather underwater.\n\r", ch );
    return;
  }

  if ( !IS_OUTSIDE(ch) )
  {
    send_to_char( "You can't see the weather indoors.\n\r", ch );
    return;
  }

  printf_to_char(ch, "The sky is %s and %s.\n\r",
                 sky_look[weather_info.sky],
                 weather_info.change >= 0
                 ? "a warm southerly breeze blows"
                 : "a cold northern gust blows"
                );
  return;
}
void show_specific_help( CHAR_DATA *ch, char *argument );
void show_specific_help_vnum( CHAR_DATA *ch, char *argument );

void do_help( CHAR_DATA *ch, char *argument )
{
  HELP_DATA *pHelp;
  BUFFER *output;
  char buf[MAX_STRING_LENGTH];
  bool found = FALSE;
  bool help_found = FALSE;
  int count = 0;
  char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
  char temparg[MIL];
  int level;
  int iSocial;

  strcpy (temparg, argument);

  if (is_number(argument))
  {
    show_specific_help_vnum(ch,argument);
    return;
  }

  if ( argument[0] == '#' ) // If users try to search by help# starting with a '#'
  {
    argument = one_argument(argument,argone);

    if (argone[1] == '\0') // Is there a space between '#' and number?  Then get next arg
    {
      argument = one_argument(argument,argone);
      if (is_number(argone))
      {
        show_specific_help_vnum(ch,argone);
        return;
      }
    } // if all else fails it will sho "no help found" message
    else
    {
      for (count = 0; argone[count] != '\0'; count++)
        argone[count] = argone[count+1];
      if (is_number(argone))
      {
        show_specific_help_vnum(ch,argone);
        return;
      }
    }
    count = 0;
  }

  output = new_buf();
  if ( argument[0] == '\0' )
    argument = "summary";



  /* this parts handles help a b so that it returns help 'a b' */
  argall[0] = '\0';
  while (argument[0] != '\0' )
  {
    argument = one_argument(argument,argone);
    if (argall[0] != '\0')
      strcat(argall," ");
    strcat(argall,argone);
  }

  bprintf(output,"{CMatching Helps Found. For More Information <help #>{x\n\r");
  bprintf(output,"{W==================================================={x\n\r");
  for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
  {
    if (pHelp->delete_it == TRUE)
      continue;

    if ( is_name( argall, pHelp->keyword ) )
    {
      level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

      help_found = TRUE;

      if (level <= get_trust( ch ) )
      {
        if (!pHelp->synopsis)
          pHelp->synopsis = str_dup("(null)", pHelp->synopsis);

        if (strcmp(pHelp->synopsis,"(null)"))
          bprintf(output,"{W[{G%3d{W]%s{C%s{x\r",
                  pHelp->vnum,
                  (pHelp->level > 100) ? "{R*" : " ",
                  pHelp->synopsis);
        else
          bprintf(output,"{W[{G%3d{W]%s{C%s{x\n\r",
                  pHelp->vnum,
                  (pHelp->level > 100) ? "{R*" : " ",
                  pHelp->keyword);

        found = TRUE;
        count++;
      }
    }
  } // for loop

  if ( !help_found )
  {
    // search for matching socials
    for ( iSocial = 0; social_table[iSocial].name[0]; iSocial++ )
    {
      if ( strstr( social_table[iSocial].name, argall )
           &&  !strstr( social_table[iSocial].name, "socials" ) )
        help_found = TRUE;
    }

    if (help_found)
    {
      do_function(ch, &do_help, "socials" );
      return;
    }

    FILE *fp;
    if ( ( fp = fopen(TODO_FILE,"r") ) != NULL )
    {
      nFilesOpen++;
      while ( ( get_next_line(fp, buf) ) != EOF )
      {
        if ( strstr(buf, argall) )
          help_found = TRUE;
      }
      fclose(fp);
      nFilesOpen--;
    }

    if (!help_found) // might as well reuse the var
    {
      mprintf(sizeof(buf), buf, "%s{x", argall );
      append_file( ch, TODO_FILE, buf );
      ch->pcdata->fdhelps++;
    }

    send_to_char( "No help on that word(s).\n\r", ch );
    return;
  }

  if ( !found )
  {
    send_to_char( "No help on that word(s).\n\r", ch );
    free_buf( output );
    return;
  }
  else if ( count == 1 )
  {
    free_buf( output );
    show_specific_help( ch, temparg );
    return;
  }
  else
  {
    bprintf( output, "\n\rType 'help' and then # beside the help to select that individual help.\n\r" );
    page_to_char( buf_string( output ), ch );
    free_buf( output );
    return;
  }
}

void show_specific_help(CHAR_DATA *ch, char *argument)
{
  HELP_DATA *pHelp;
  BUFFER *output;
  bool found = FALSE;
  char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
  int level;

  output = new_buf();

  if ( argument[0] == '\0' )
    argument = "summary";

  /* this parts handles help a b so that it returns help 'a b' */
  argall[0] = '\0';
  while (argument[0] != '\0' )
  {
    argument = one_argument(argument,argone);
    if (argall[0] != '\0')
      strcat(argall," ");
    strcat(argall,argone);
  }

  for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
  {
    level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

    if (level > get_trust( ch ) )
      continue;

    if ( is_name( argall, pHelp->keyword ) )
    {
      /* add seperator if found */
      if (found)
        add_buf(output,
                "\n\r============================================================\n\r\n\r");
      if ( pHelp->level >= 0 && IS_IMMORTAL(ch))
      {
        bprintf(output,"LEVEL: %d\n\r",pHelp->level);
      }
      if (!pHelp->synopsis)
        pHelp->synopsis = str_dup("(null)", pHelp->synopsis);

      if (strcmp(pHelp->synopsis,"(null)"))
      {
        bprintf(output,"-----\n\r");
        bprintf(output,"{W%s{x\r",pHelp->synopsis);
        bprintf(output,"-----\n\r");
      }
      /*
       * Strip leading '.' to allow initial blanks.
       */
      if ( pHelp->text[0] == '.' )
        add_buf(output,pHelp->text+1);
      else
        add_buf(output,pHelp->text);
      found = TRUE;
      /* small hack :) */
      if ( ch->desc && ch->desc->connected != CON_PLAYING
           &&               ch->desc->connected != CON_GEN_GROUPS )
        break;
    }
  }

  if ( !found )
  {
    logit("HELP MISSING: %s",argall);
    send_to_char("No help on that word(s).\n\r", ch);
  }
  else
    page_to_char(buf_string(output),ch);
  free_buf(output);
}
void show_specific_help_vnum(CHAR_DATA *ch, char *argument)
{
  HELP_DATA *pHelp;
  BUFFER *output;
  int vnum=0;
  bool found = FALSE;
  char argall[MAX_INPUT_LENGTH];
  int level;

  if ( argument[0] == '\0' )
  {
    send_to_char("Error, number required.\n\r",ch);
    return;
  }
  if (!is_number(argument))
  {
    send_to_char("Error, number required.\n\r",ch);
    return;
  }
  vnum = atoi(argument);
  output = new_buf();

  for ( pHelp = help_first; pHelp; pHelp = pHelp->next )
  {
    level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

    if (level > get_trust( ch ) )
      continue;

    if ( pHelp->vnum == vnum)
    {
      /* add seperator if found */
      if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
      {
        add_buf(output,pHelp->keyword);
        add_buf(output,"\n\r");
      }

      if (strcmp(pHelp->synopsis,"(null)"))
      {
        bprintf(output,"-----\n\r");
        bprintf(output,"{W%s{x\r",pHelp->synopsis);
        bprintf(output,"-----\n\r");
      }

      /*
       * Strip leading '.' to allow initial blanks.
       */
      if ( pHelp->text[0] == '.' )
        add_buf(output,pHelp->text+1);
      else
        add_buf(output,pHelp->text);
      found = TRUE;
      /* small hack :) */
      if ( ch->desc && ch->desc->connected != CON_PLAYING
           &&               ch->desc->connected != CON_GEN_GROUPS )
        break;
    }
  }

  if ( !found )
    send_to_char( "No help for that number available to you.\n\r", ch );
  else
    page_to_char(buf_string(output),ch);
  free_buf(output);
}

/* whois command */
void do_whois ( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  BUFFER *output = NULL;
  DESCRIPTOR_DATA *d;
  bool found = FALSE;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "You must provide a name.\n\r", ch );
    return;
  }

  for ( d = descriptor_list; d; d = d->next )
  {
    CHAR_DATA *wch;

    if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
      continue;

    wch = ( d->original ) ? d->original : d->character;

    if ( !can_see( ch, wch ) )
      continue;
    if ( IS_NPC( wch ) )
      continue;
    if ( !str_prefix( arg, wch->name ) )
    {
      if ( !found )
        output = new_buf();

      found = TRUE;
      add_buf( output, wholine( ch, wch ) );
    }
  }

  if ( !found )
  {
    send_to_char("No one of that name is playing.\n\r",ch);
    return;
  }

  page_to_char(buf_string(output),ch);
  free_buf(output);
}


/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void do_who( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char buf1[MIL];
  BUFFER *output;
  DESCRIPTOR_DATA *d;
  int iClass;
  int iRace;
//  int iClan;
  int count=0;
  int iLevelLower;
  int iLevelUpper;
  int nNumber;
  int nMatch;
  bool found = FALSE;
  bool rgfClass[MAX_CLASS];
  bool rgfRace[MAX_PC_RACE];
//  bool rgfClan[MAX_CLAN];
  bool fClassRestrict = FALSE;
//  bool fClanRestrict = FALSE;
  bool fClan = FALSE;
  bool fKiller = FALSE;
  bool fViolent = FALSE;
  bool fRaceRestrict = FALSE;
  bool fImmortalOnly = FALSE;
  bool fPKill = FALSE;
  bool fGroup = FALSE;
  int j;

  /*
   * Set default arguments.
   */
  iLevelLower    = 0;
  iLevelUpper    = MAX_LEVEL+1;

  for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
    rgfClass[iClass] = FALSE;

  for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
    rgfRace[iRace] = FALSE;

  /*  for (iClan = 0; iClan <= actual_num_clans; iClan++)
      rgfClan[iClan] = FALSE;
  */
  /*
   * Parse arguments.
   */
  nNumber = 0;
  for ( ; ; )
  {
    char arg[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
      break;

    if ( is_number( arg ) )
    {
      switch ( ++nNumber )
      {
        case 1:
          iLevelLower = atoi( arg );
          break;
        case 2:
          iLevelUpper = atoi( arg );
          break;
        default:
          send_to_char( "Only two level numbers allowed.\n\r", ch );
          return;
      }
    }
    else
    {

      /*
       * Look for classes to turn on.
       */
      if ( !str_prefix(arg,"immortals" ) )
      {
        fImmortalOnly = TRUE;
        found = TRUE;
      }

      iClass = class_lookup(arg);

      if (iClass != -1)
      {
        fClassRestrict = TRUE;
        rgfClass[iClass] = TRUE;
        found = TRUE;
      }

      iRace = race_lookup(arg);
      if (iRace != 0 && iRace < MAX_PC_RACE)
      {
        fRaceRestrict = TRUE;
        rgfRace[iRace] = TRUE;
        found = TRUE;
      }

      if (!str_prefix(arg,"clan"))
      {
        fClan = TRUE;
        found = TRUE;
      }
      /*    iClan = clan_lookup(arg);
          if (iClan != -1)
            {
              fClanRestrict = TRUE;
              rgfClan[iClan] = TRUE;
              found = TRUE;
            }*/

      if ( !str_prefix( arg, "killer" ) )
      {
        fKiller = TRUE;
        found = TRUE;
      }

      if ( !str_prefix( arg, "violent" ) )
      {
        fViolent = TRUE;
        found = TRUE;
      }

      if ( !str_prefix( arg, "pk" ) )
      {
        fPKill = TRUE;
        found = TRUE;
      }

      if ( !str_prefix( arg, "group" ) )
      {
        fGroup = TRUE;
        found = TRUE;
      }

      if ( !found )
      {
        send_to_char("Syntax: who [option]\n\r",ch);
        send_to_char("Options: <race>    - Put a Specific RACE.\n\r",ch);
        send_to_char("         <class>   - Put a Specific CLASS.\n\r",ch);
        send_to_char("         <clan>    - Put a Specific CLAN.\n\r",ch);
        send_to_char("         pk        - Show players you can PK.\n\r",ch);
        send_to_char("         group     - Show players in your group.\n\r",ch);
        send_to_char("         violent   - Show all Violents.\n\r",ch);
        send_to_char("         killer    - Show all Killers.\n\r",ch);
        return;
      }
    }
  }

  /*
   * Now show matching chars.
   */
  nMatch = 0;
  buf[0] = '\0';
  output = new_buf();
  add_buf(output,"\n");

  for (j=MAX_LEVEL+1; j > 0; j--)
  {
    for ( d = descriptor_list; d; d = d->next )
    {
      CHAR_DATA *wch;

      /*
       * Check for match against restrictions.
       * Don't use trust as that exposes trusted mortals.
       */
      if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
        continue;

      wch   = ( d->original ) ? d->original : d->character;

      if (j != wch->level)
        continue;

      if (!can_see(ch,wch))
        continue;

      if (IS_NPC(wch))
        continue;

      if ( wch->level < iLevelLower
           ||   wch->level > iLevelUpper
           || ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
           || ( fClassRestrict && !rgfClass[wch->gameclass] )
           || ( fRaceRestrict && !rgfRace[wch->race])
           || ( fClan && !(wch->clan))
           || ( fKiller && !IS_SET(wch->act, PLR_KILLER) )
           || ( fViolent && !IS_SET(wch->act, PLR_VIOLENT) )
           || ( fGroup && !is_same_group( wch, ch ) )
           || ( fPKill
                && !(is_in_pk_range(ch,wch)
                     && wch->clan != NULL
                     && ch->clan != NULL
                     && wch != ch
                     && !IS_IMMORTAL( wch )
                     && !IS_SET( wch->clan->clan_flags, CLAN_PEACEFUL ) ) )
           /*      || ( fClanRestrict && !rgfClan[wch->clan]) */ )

        continue;

      nMatch++;
      add_buf(output,wholine(ch,wch));
    }
  }

  /*  mprintf(sizeof(buf2), buf2, "\n\rCurrent Players found: %d\n\r", nMatch );
      add_buf(output,buf2);*/
  for ( d = descriptor_list; d; d = d->next )
    if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
      count++;

  count_update();
  max_on = UMAX(count,countMaxDay);

  if (max_on == count)
    bprintf( output,
             "There %s {W%s{x character%s on, the most so far today.\n\r",
             ( count == 1 ? "is" : "are" ), numcpy( buf, count ),
             ( count == 1 ? ""   : "s" ) );
  else
    bprintf( output,
             "There %s {c%s{x character%s on, the most on today was {W%s{x.\n\r",
             ( count == 1 ? "is" : "are" ),  numcpy( buf,  count ),
             ( count == 1 ? ""   : "s" ),    numcpy( buf1, countMaxDay ) );

  page_to_char( buf_string(output), ch );
  free_buf(output);
  return;
}
void do_count ( CHAR_DATA *ch, char *argument )
{
  char buf[MIL];
  char buf1[MIL];
  int count;
  DESCRIPTOR_DATA *d;

  count = 0;
  count_update();
  for ( d = descriptor_list; d; d = d->next )
    if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
      count++;

  countMax = UMAX(count,countMax);

  if (countMax == count)
    printf_to_char( ch,
                    "There %s {W%s{x character%s on, the most so far today.\n\r",
                    ( count == 1 ? "is" : "are" ), numcpy( buf, count ),
                    ( count == 1 ? ""   : "s" ) );
  else
    printf_to_char( ch,
                    "There %s {W%s{x character%s on, the most on today was {C%s{x.\n\r",
                    ( count == 1 ? "is" : "are" ), numcpy( buf,  count ),
                    ( count == 1 ? ""   : "s" ),   numcpy( buf1, countMax ) );

}

void do_inventory( CHAR_DATA *ch, char *argument )
{
  printf_to_char( ch, "You are carrying (%d/%d):\n\r",
                  ch->carry_number,
                  can_carry_n(ch) );
  show_list_to_char( ch->carrying, ch, TRUE, TRUE, FALSE, FALSE);
  return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  int iWear;
  bool found;
  //char arg[MIL];

  if ( !IS_NULLSTR( argument ) )
  {
    if ( !strcmp( argument, "all" ) )
    {
      do_new_equipment( ch, argument );
      return;
    }
  }


  send_to_char( "You are using:\n\r", ch );
  found = FALSE;
  for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
  {
    if ( ( obj = get_eq_char( ch, where_table[iWear].where ) ) == NULL )
      continue;

    send_to_char( where_name[where_table[iWear].where], ch );
    if ( can_see_obj( ch, obj ) )
    {
      send_to_char( format_obj_to_char( obj, ch, TRUE, FALSE ), ch );
      send_to_char( "\n\r", ch );
    }
    else
      send_to_char( "something.\n\r", ch );

    found = TRUE;
  }

  if ( !found )
    send_to_char( "Nothing.\n\r", ch );

  return;
}

void do_compare( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj1;
  OBJ_DATA *obj2;
  int value1;
  int value2;
  char *msg;
  int skill = 0;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  if ( arg1[0] == '\0' )
  {
    send_to_char( "Compare what to what?\n\r", ch );
    return;
  }

  if ( ( obj1 = get_obj_carry( ch, arg1, ch ) ) == NULL )
  {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }

  if ( arg2[0] == '\0' )
  {
    for ( obj2 = ch->carrying; obj2; obj2 = obj2->next_content )
    {
      if ( obj2->wear_loc != WEAR_NONE
           &&   can_see_obj( ch, obj2 )
           &&   obj1->item_type == obj2->item_type
           && ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) )
        break;
    }

    if ( obj2 == NULL )
    {
      send_to_char("You aren't wearing anything comparable.\n\r",ch);
      return;
    }
  }

  else if ( ( obj2 = get_obj_carry( ch, arg2, ch ) ) == NULL )
  {
    send_to_char("You do not have that item.\n\r", ch );
    return;
  }

  msg    = NULL;
  value1  = 0;
  value2  = 0;

  if ( obj1 == obj2 )
    msg = "You compare $p to itself.  It looks about the same.";
  else if ( obj1->item_type != obj2->item_type )
    msg = "You can't compare $p and $P.";
  else
  {
    switch ( obj1->item_type )
    {
      default:
        msg = "You can't compare $p and $P.";
        break;

      case ITEM_ARMOR:
        value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
        value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
        break;

      case ITEM_WEAPON:
        if (obj1->item_type == ITEM_WEAPON)
        {
          switch (obj1->value[0])
          {
            case(WEAPON_EXOTIC):  skill = get_weapon_skill(ch,gsn_exotic);
              break;
            case(WEAPON_SWORD):   skill = get_weapon_skill(ch,gsn_sword);
              break;
            case(WEAPON_DAGGER):  skill = get_weapon_skill(ch,gsn_dagger);
              break;
            case(WEAPON_SPEAR):   skill = get_weapon_skill(ch,gsn_spear);
              break;
            case(WEAPON_MACE):    skill = get_weapon_skill(ch,gsn_mace);
              break;
            case(WEAPON_AXE):     skill = get_weapon_skill(ch,gsn_axe);
              break;
            case(WEAPON_FLAIL):   skill = get_weapon_skill(ch,gsn_flail);
              break;
            case(WEAPON_WHIP):    skill = get_weapon_skill(ch,gsn_whip);
              break;
            case(WEAPON_POLEARM): skill = get_weapon_skill(ch,gsn_polearm);
              break;
						case(WEAPON_CROSSBOW): skill = get_weapon_skill(ch,gsn_crossbow);
							break;
          }

          if (skill < 2)
          {
            act("You have no experience using $p.",ch,obj1,NULL,TO_CHAR);
            return;
          }
          else if (skill < 20)
          {
            act("You would probably hurt yourself using $p.",ch,obj1,NULL,TO_CHAR);
            return;
          }
          else if (skill < 40)
          {
            act("You are not very skilled with $p...",ch,obj1,NULL,TO_CHAR);
            return;
          }
        }

        if ( obj1->pIndexData->new_format )
          value1 = ( 1 + obj1->value[2] ) * obj1->value[1];
        else
          value1 = obj1->value[1] + obj1->value[2];

        if ( obj2->pIndexData->new_format )
          value2 = ( 1 + obj2->value[2] ) * obj2->value[1];
        else
          value2 = obj2->value[1] + obj2->value[2];
        break;
    }
  }

  if ( msg == NULL )
  {
    if      ( value1 == value2 ) msg = "$p and $P look about the same.";
    else if ( value1  > value2 ) msg = "$p looks better than $P.";
    else                         msg = "$p looks worse than $P.";
  }

  act( msg, ch, obj1, obj2, TO_CHAR );
  return;
}



void do_credits( CHAR_DATA *ch, char *argument )
{
  do_function( ch,&do_help, "diku" );
}



void do_where( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *vch;
  bool found;

  one_argument( argument, arg );
  if ( ch->in_room == NULL )
  {
    bugf( "Hey, we are in a NULL room?? %s", interp_cmd );
    return;
  }

  if ( arg[0] == '\0' )
  {
    if ( IS_IMMORTAL(ch) )
    {
      printf_to_char( ch, "{xYou are in: {g%s {w({c%d{w-{c%d{w){x\n\rContinent:  %s {x({c%d{x){x\n\r",
                      ch->in_room->area->name,
                      ch->in_room->area->low_range, ch->in_room->area->high_range,
                      continent_name(ch->in_room->area), ch->in_room->area->continent);
    }
    else
    {
      printf_to_char( ch, "{xYou are in: {g%s {w({c%d{w-{c%d{w){x\n\rContinent:  {y%s{x\n\r",
                      ch->in_room->area->name,
                      ch->in_room->area->low_range, ch->in_room->area->high_range,
                      continent_name(ch->in_room->area));
    }

    send_to_char( "{xPlayers near you:{x\n\r", ch );
    found = FALSE;
    for ( vch = player_list; vch; vch = vch->next_player )
    {
      if ( !IS_IMMORTAL( ch ) )
      {
        if ( vch->in_room == NULL || vch->in_room->area != ch->in_room->area )
          continue;

        if ( IS_SET( vch->in_room->room_flags, ROOM_NOWHERE ) )
          continue;

        if ( !is_room_owner( ch, vch->in_room )
             &&   room_is_private( vch->in_room ) )
          continue;

        if ( vch == ch )
          continue;

        if ( IS_LINKDEAD( vch ) )
          continue;

        if ( !can_see( ch, vch ) )
        {
          if ( ch->clan )
          {
            if ( ch->clan != vch->clan
                 ||   IS_SET( ch->clan->clan_flags, CLAN_INDEPENDENT ) )
              continue;

            if ( (ch->incog_level < vch->incog_level)
                 ||   (ch->invis_level < vch->invis_level) )
              continue;
          }
          else
            continue;
        } // !can_see

        if ( ch->in_room != vch->in_room )
        {
          if ( !IS_IMMORTAL( ch )
               &&   IS_IMMORTAL( vch ) )
            continue;

          if ( !IS_SAFFECTED( ch, SAFF_FARSIGHT )
               &&   !IS_SET( ch->act, PLR_HOLYLIGHT ) )
          {
            if ( ch->clan )
            {
              if ( ch->clan != vch->clan
                   ||   IS_SET( ch->clan->clan_flags, CLAN_INDEPENDENT ) )
                continue;
            }
            else
              continue;
          }
        }
      }

      found = TRUE;

      if ( IS_IMMORTAL(ch) )
        printf_to_char( ch, "%s {D%-28s {x%s\n\r",
                        vch->in_room->area == ch->in_room->area ? "(*)" : "   ",
                        vch->name,
                        vch->in_room->name );
      else
        printf_to_char( ch, "{D%-28s {x%s\n\r",
                        vch->name,
                        vch->in_room->name );

    }
    if ( !found )
      send_to_char( "None\n\r", ch );
  }
  else
  {
    found = FALSE;
    for ( vch = char_list; vch; vch = vch->next )
    {
      if ( vch->in_room == NULL )
        continue;

      if ( IS_SET( vch->in_room->room_flags, ROOM_NOWHERE ) )
        continue;

      if ( vch->in_room->area != ch->in_room->area )
        continue;

      if ( !can_see( ch, vch ) )
        continue;

      if ( !IS_SET( ch->spell_aff, SAFF_FARSIGHT )
           &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
           && (  IS_CLASS_HIGHWAYMAN( ch ) ) )
        continue;

      if ( IS_AFFECTED( vch, AFF_HIDE ) )
        continue;

      if ( IS_AFFECTED( vch, AFF_SNEAK ) )
        continue;

      if ( !is_name( arg, vch->name ) )
        continue;

      if ( vch->incog_level > get_trust(ch) )
        continue;


      found = TRUE;
      printf_to_char( ch, "%-28s %s\n\r",
                      PERS( vch, ch ), vch->in_room->name );
      break;
    }
    if ( !found )
      act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
  }
  return;
}




void do_consider( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  char *msg;
  int diff;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Consider killing whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
  {
    send_to_char( "They're not here.\n\r", ch );
    return;
  }

  if (is_safe(ch,victim, TRUE))
  {
    send_to_char("Don't even think about it.\n\r",ch);
    return;
  }

  diff = victim->level - ch->level;

  if ( diff <= -10 ) msg = "You can kill $N naked and weaponless.";
  else if ( diff <=  -5 ) msg = "$N is no match for you.";
  else if ( diff <=  -2 ) msg = "$N doesn't look very tough.";
  else if ( diff <=   1 ) msg = "$N could put up just a little fight.";
  else if ( diff <=   4 ) msg = "$N looks like a good match.";
  else if ( diff <=   9 ) msg = "$N asks, 'Do you feel lucky, punk'?";
  else if ( diff <=   15 ) msg = "$N laughs at you mercilessly.";
  else if ( diff <=   24 ) msg = "$N doesn't even notice you, FLEA.";
  else if ( diff <=   40 ) msg = "Don't even think about it.";
  else                    msg = "Death will thank you for your gift.";

  act( msg, ch, NULL, victim, TO_CHAR );
  return;
}



void set_title( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
// If you think other chars should not have a spece, add them here
  char *NOSPACECHAR = ".,?!':";

  if ( IS_NPC( ch ) )
  {
    bug( "Set_title: NPC.", 0 );
    return;
  }

  strip_string( argument );
  buf[0] = '\0';

  if ( !strchr( NOSPACECHAR, *argument ) )
    strcat( buf, " " );

  strcat( buf, argument );

#if MEMDEBUG
  free_string( ch->pcdata->memdebug_title );
  ch->pcdata->memdebug_title = str_dup( buf, ch->pcdata->memdebug_title );
#endif
  free_string( ch->pcdata->title );
  ch->pcdata->title = str_dup( buf , ch->pcdata->title );
  return;
}


void set_titleafk( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
// If you think other chars should not have a spece, add them here
  char *NOSPACECHAR = ".,?!':";

  if ( IS_NPC( ch ) )
  {
    bug( "Set_afktitle: NPC.", 0 );
    return;
  }

  strip_string( argument );
  buf[0] = '\0';

  if ( !strchr( NOSPACECHAR, *argument ) )
    strcat( buf, " " );

  strcat( buf, argument );

  free_string( ch->pcdata->afk_title );
  ch->pcdata->afk_title = str_dup( buf , ch->pcdata->afk_title );
  return;
}


void do_title( CHAR_DATA *ch, char *argument )
{
  char buf[MSL];
  char title[MSL];

  if ( IS_NPC( ch ) )
    return;

  if ( IS_GHOST( ch ) )
  {
    send_to_char( "Changing your title is pointless as you are still {rDEAD{x.\n\r", ch);
    return;
  }

  if ( argument[0] == '\0' )
  {
    send_to_char( "Change your title to what?\n\r", ch );
    return;
  }

  smash_tilde( argument );
  strcpy( title, argument );

  if ( !strcmp( "Reset", capitalize( argument ) ) )
  {
    send_to_char( "Resetting to use normal titles for your class.\n\r", ch );
    /*    mprintf(sizeof(buf), buf, "the %s",
      title_table [ch->class] [ch->level] [ch->sex == SEX_FEMALE
      ? 1 : 0] );
    */
    mprintf(sizeof(buf), buf, "." );
    set_title( ch, buf );
    ch->pcdata->user_set_title = FALSE;
  }
  else
  {
    if ( colorstrlen( title ) > 45 )
    {
      send_to_char("Title is too long.  Please shorten it.\n\r",ch);
      return;
    }
    else
    {
      strcat(title,"{x\0");
    }
    strip_color( buf, title );
    if ( strstr( buf, "AFK" )  )
    {
      send_to_char( "Illegal string AFK in title.\n\r", ch );
      return;
    }
    if ( title[0] == '\0' )
      return;
    set_title( ch, title );
    ch->pcdata->user_set_title = TRUE;
    printf_to_char( ch, "Title set to: %s%s\n\r", ch->name, ch->pcdata->title );
  }
#if MEMDEBUG
  memdebug_check( ch, "act_info: do_title" );
#endif
}



void do_description( CHAR_DATA *ch, char *argument )
{
  if (IS_NPC(ch))
  {
    send_to_char("This function not available for NPC's\n\r",ch);
    return;
  }
  if ( argument[0] == '+' )
  {
    string_append( ch, &ch->description, APPEND_CHAR, ch );
  }
  send_to_char( "Your description is:\n\r", ch );
  send_to_char( ch->description ? ch->description : "(None).\n\r", ch );
  return;
}

void do_report( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *gch;
  char buf[MAX_INPUT_LENGTH];
  char arg[MIL];
  int64 xp = 0;

  argument = one_argument( argument, arg );

  if (IS_NPC(ch))
    xp = 0;
  else
  {
    if (ch->level < LEVEL_HERO)
      xp = TNL( exp_per_level( ch, ch->pcdata->points ), ch->level ) - ch->exp;
    else
      xp = ch->exp;
  }

  if (arg[0] == '\0')
  {
    printf_to_char(ch,
                   "{gYou say '{DI have <{R%d{r(%d){rh{D> <{C%d{c(%d){cm{D> <{B%d{b(%d){bmv{D> <{g%d exp{D>{g'{x\n\r",
                   ch->hit,  GET_HP(ch),
                   ch->mana, GET_MANA(ch),
                   ch->move, ch->max_move ,xp
                  );

    mprintf(sizeof(buf), buf, "{g$n says '{DI have <{R%d{r(%d){rh{D> <{C%d{c(%d){cm{D> <{B%d{b(%d){bmv{D> <{g%d exp{D>{g'{x",
            ch->hit, GET_HP(ch),
            ch->mana, GET_MANA(ch),
            ch->move, ch->max_move ,xp
           );
    act( buf, ch, NULL, NULL, TO_ROOM );
  }
  else
  {
    // there is an argument
    int argfound = 0;

    if ( !str_prefix( arg, "group" ) )
    {
      argfound = 1;

      if ( IS_SET( ch->chan_flags, CHANNEL_NOTELL ) )
      {
        send_to_char( "Your message didn't get through!\n\r", ch );
        return;
      }

      if (IS_SET(ch->chan_flags,CHANNEL_QUIET))
      {
        send_to_char("You must turn off quiet mode first.\n\r",ch);
        return;
      }

      printf_to_char(ch,
                     "{GYou tell your group '{DI have <{R%d{r(%d){rh{D> <{C%d{c(%d){cm{D> <{B%d{b(%d){bmv{D> <{g%d exp{D>{G'{x\n\r",
                     ch->hit,  GET_HP(ch),
                     ch->mana, GET_MANA(ch),
                     ch->move, ch->max_move, xp
                    );

      for ( gch = player_list; gch != NULL; gch = gch->next_player )
      {
        if (IS_SET(gch->chan_flags,CHANNEL_QUIET)) continue;

        if ( ( is_same_group( gch, ch ) ) && ( gch != ch ) )
        {
          mprintf(sizeof(buf), buf,
                  "{G$n tells the group '{DI have <{R%d{r(%d){rh{D> <{C%d{c(%d){cm{D> <{B%d{b(%d){bmv{D> <{g%d exp{D>{G'{x\n\r",
                  ch->hit, GET_HP(ch),
                  ch->mana, GET_MANA(ch),
                  ch->move, ch->max_move, xp
                 );
          act_channels(buf,ch,argument,gch,TO_VICT,POS_SLEEPING, CHAN_GROUPTELL);
        } // if is in group
      } // victim 'for' loop
    } // if 'group' arg


    if ( !str_prefix( arg, "tell" ) ) //This will search all 3.
    {
      argfound = 1;

      if ( IS_SET( ch->chan_flags, CHANNEL_NOTELL ) )
      {
        send_to_char( "Your message didn't get through!\n\r", ch );
        return;
      }

      if (IS_SET(ch->chan_flags,CHANNEL_QUIET))
      {
        send_to_char("You must turn off quiet mode first.\n\r",ch);
        return;
      }

      one_argument( argument, arg );

      if ( ( gch = get_char_world( ch, arg ) ) == NULL )
      {
        printf_to_char(ch,"They cannot be found.{x\n\r");
        return;
      }
      else
      {
        if ( IS_NPC( gch ) )
        {
          send_to_char( "They cannot be found!\n\r", ch );
          return;
        }

        if ( ( IS_SET( gch->chan_flags, CHANNEL_NOTELL ) ) || ( IS_SET( gch->chan_flags,CHANNEL_QUIET ) ) )
        {
          send_to_char( "Your message didn't get through!\n\r", ch );
          return;
        }

        printf_to_char(ch,
                       "{gYou tell %s '{DI have <{R%d{r(%d){rh{D> <{C%d{c(%d){cm{D> <{B%d{b(%d){bmv{D> <{g%d exp{D>{g'{x\n\r",
                       gch->name,
                       ch->hit, GET_HP(ch),
                       ch->mana, GET_MANA(ch),
                       ch->move, ch->max_move, xp
                      );

        mprintf(sizeof(buf), buf,
                "{g$n tells you '{DI have <{R%d{r(%d){rh{D> <{C%d{c(%d){cm{D> <{B%d{b(%d){bmv{D> <{g%d exp{D>{g'{x\n\r",
                ch->hit, GET_HP(ch),
                ch->mana, GET_MANA(ch),
                ch->move, ch->max_move, xp
               );
        act_channels(buf,ch,argument,gch,TO_VICT,POS_SLEEPING, CHAN_GROUPTELL);
      } // victim found
    } // if 'tell' arg
    if ( !argfound )
    {
      printf_to_char(ch,"What???{x\n\r");
    }
  } // arg is not null

  return;
}



void do_practice( CHAR_DATA *ch, char *argument )
{
  BUFFER *buffer=NULL;
  char buf[MAX_STRING_LENGTH];
  int sn,i;
  bool race_skill_found;
  bool pra_found=FALSE;
  char color[2];

  if ( IS_NPC( ch ) )
    return;

  if ( IS_GHOST( ch ) )
  {
    send_to_char( "Practicing is useless as you are still {rDEAD{x.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  {
    int col = 0;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
      if ( skill_table[sn].name == NULL )
        break;
      if ( !is_racial_skill( ch,sn ) )
      {
        if ( ch->level < skill_table[sn].skill_level[ch->gameclass]
             ||   ch->pcdata->learned[sn] < 1 /* skill is not known */)
          continue;
      }

      if ( !pra_found )
        buffer = new_buf();
      pra_found = TRUE;

      mprintf(sizeof(color), color, "%c",
              color_scale(ch->pcdata->learned[sn], "rRYG"));

      strncpy_color(buf, skill_table[sn].name, 19, ' ', TRUE );

      bprintf(buffer, "%-18s {%s%3d%%{x  ",
              buf, ch->pcdata->learned[sn] == 100 ? "W" : color,
              ch->pcdata->learned[sn] );

      if ( ++col % 3 == 0 )
        add_buf( buffer, "\n\r" );
    }

    if ( col % 3 )
      send_to_char( "\n\r", ch );

    printf_to_char(ch, "You have %d practice sessions left.\n\r",
                   ch->practice );
  }
  else
  {
    CHAR_DATA *mob;
    int adept;

    if ( !IS_AWAKE( ch ) )
    {
      send_to_char( "In your dreams, or what?\n\r", ch );
      return;
    }

    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
      if ( IS_NPC( mob ) && IS_SET( mob->act, ACT_PRACTICE) )
        break;
    }

    if ( !mob )
    {
      send_to_char( "You can't do that here.\n\r", ch );
      return;
    }

    if ( ch->practice <= 0 )
    {
      send_to_char( "You have no practice sessions left.\n\r", ch );
      return;
    }

    if ( ( sn = find_spell( ch,argument ) ) < 0)
    {
      send_to_char( "You can't practice that.\n\r", ch );
      return;
    }

    race_skill_found = FALSE;
    for ( i=0; i < 5; i++ )
      if ( pc_race_table[ch->race].skills[i] )
        if ( sn == skill_lookup( pc_race_table[ch->race].skills[i] ) )
          race_skill_found = TRUE;
    if ( !race_skill_found)
    {
      if ( ( !IS_NPC( ch )
             &&   ( ch->level < skill_table[sn].skill_level[ch->gameclass]
                    ||     ch->pcdata->learned[sn] < 1 /* skill is not known */
                    ||     skill_table[sn].rating[ch->gameclass] == 0 ) ) )
      {
        send_to_char( "You can't practice that.\n\r", ch );
        return;
      }
    }
    adept = IS_NPC( ch ) ? 100 : class_table[ch->gameclass].skill_adept;

    if ( ch->pcdata->learned[sn] >= adept )
    {
      printf_to_char(ch, "You are already learned at %s.\n\r",
                     skill_table[sn].name );
    }
    else
    {
      ch->practice--;
      ch->pcdata->learned[sn] += int_app[get_curr_stat(ch,STAT_INT)].learn /
                                 ( is_racial_skill( ch, sn ) ? 1 : skill_table[sn].rating[ch->gameclass] );
      //ch->pcdata->learned[sn] += get_curr_stat(ch,STAT_INT);
      if ( ch->pcdata->learned[sn] < adept )
      {
        act( "You practice $T.", ch, NULL, skill_table[sn].name, TO_CHAR );
        act( "$n practices $T.", ch, NULL, skill_table[sn].name, TO_ROOM );

        mprintf(sizeof(color), color, "%c",
                color_scale(ch->pcdata->learned[sn], "rRYG"));
        printf_to_char( ch, "Your skill is now {%s%d%%{x.\n\r",
                        color, ch->pcdata->learned[sn] );
      }
      else
      {
        ch->pcdata->learned[sn] = adept;
        act( "You are now learned at {C$T{x.", ch, NULL,
             skill_table[sn].name, TO_CHAR );
        act( "$n is now learned at {C$T{x.", ch, NULL,
             skill_table[sn].name, TO_ROOM );
      }
    }
  }
  if ( pra_found )
  {
    add_buf( buffer, "\n\r" );
    page_to_char( buf_string( buffer ), ch );
    free_buf( buffer );
  }
  return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  int wimpy;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
    wimpy = ch->max_hit / 5;
  else
    wimpy = atoi( arg );

  if ( wimpy < 0 )
  {
    send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
    return;
  }

  if ( wimpy > ch->max_hit/2 )
  {
    send_to_char( "Such cowardice ill becomes you.\n\r", ch );
    return;
  }

  ch->wimpy  = wimpy;
  printf_to_char(ch, "Wimpy set to %d hit points.\n\r", wimpy );
  return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char *pArg;
  char *pwdnew;
  char *p;
  char cEnd;

  if ( IS_NPC(ch) )
    return;

  /*
   * Can't use one_argument here because it smashes case.
   * So we just steal all its code.  Bleagh.
   */
  pArg = arg1;
  while ( isspace(*argument) )
    argument++;

  cEnd = ' ';
  if ( *argument == '\'' || *argument == '"' )
    cEnd = *argument++;

  while ( *argument )
  {
    if ( *argument == cEnd )
    {
      argument++;
      break;
    }
    *pArg++ = *argument++;
  }
  *pArg = '\0';

  pArg = arg2;
  while ( isspace(*argument) )
    argument++;

  cEnd = ' ';
  if ( *argument == '\'' || *argument == '"' )
    cEnd = *argument++;

  while ( *argument != '\0' )
  {
    if ( *argument == cEnd )
    {
      argument++;
      break;
    }
    *pArg++ = *argument++;
  }
  *pArg = '\0';

  free_string(ch->cmd_hist[0].text);
  ch->cmd_hist[0].text = str_dup("password ****** ******", ch->cmd_hist[0].text);

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  {
    send_to_char( "Syntax: password <old> <new>.\n\r", ch );
    return;
  }

  if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
  {
    WAIT_STATE( ch, 40 );
    send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
    return;
  }

  if ( strlen(arg2) < 7 )
  {
    send_to_char(
      "New password must be at least seven characters long.\n\r", ch );
    return;
  }

  if (!strstr(arg2,"1") &&
      !strstr(arg2,"2") &&
      !strstr(arg2,"3") &&
      !strstr(arg2,"4") &&
      !strstr(arg2,"5") &&
      !strstr(arg2,"6") &&
      !strstr(arg2,"7") &&
      !strstr(arg2,"8") &&
      !strstr(arg2,"9") &&
      !strstr(arg2,"0"))
  {
    send_to_char("You must have a number in your password.\n\r",ch);
    return;
  }

  /*
   * No tilde allowed because of player file format.
   */
  pwdnew = crypt( arg2, ch->name );
  for ( p = pwdnew; *p; p++ )
  {
    if ( *p == '~' )
    {
      send_to_char(
        "New password not acceptable, try again.\n\r", ch );
      return;
    }
  }
  logit("%s has changed thier password.\n\r",ch->name);

  free_string( ch->pcdata->pwd );
  ch->pcdata->pwd = str_dup( pwdnew, ch->pcdata->pwd );
#if MEMDEBUG
  free_string(ch->pcdata->memdebug_pwd);
  ch->pcdata->memdebug_pwd = str_dup (pwdnew, ch->pcdata->memdebug_pwd);
#endif
  save_char_obj( ch, FALSE );
  send_to_char( "Ok.\n\r", ch );
  return;
}

void do_omni( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  BUFFER *output;
  DESCRIPTOR_DATA *d;
  int immmatch;
  int mortmatch;
  int hptemp;
  CHAR_DATA       *vch;
// char            s[100];
  char            idle[10];
  int manatemp=0;

  /*
   * Initalize Variables.
   */

  immmatch = 0;
  mortmatch = 0;
  buf[0] = '\0';
  output = new_buf();

  /*
   * Count and output the IMMs.
   */

  mprintf(sizeof(buf), buf, "          ----==={g   Immortals   {w===----{x\n\r");
  add_buf(output,buf);
  mprintf(sizeof(buf), buf, "{gName          {wLevel   Wiz   Incog     [Vnum]    {BIdle{x\n\r");
  add_buf(output,buf);

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    CHAR_DATA *wch;

    if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
      continue;

    wch   = ( d->original != NULL ) ? d->original : d->character;

    if (!can_see(ch,wch)  || wch->level <= LEVEL_HERO)
      continue;

    immmatch++;


    //Added in IDLE timer to omni for the IMMortal portion (idle)
    vch = d->original ? d->original : d->character;

    if ( vch->timer > 0 )
      mprintf( sizeof(idle), idle, "{R%-4d{x", vch->timer );
    else
      mprintf( sizeof(idle), idle, "    " );

    mprintf(sizeof(buf), buf, "%-14s %3d     %-3d     %-3d    [%5d]   %s {x\n\r",
            wch->name,
            wch->level,
            wch->invis_level,
            wch->incog_level,
            wch->in_room->vnum,
            idle);
    add_buf(output,buf);
  }


  /*
   * Count and output the Morts.
   */
  mprintf(sizeof(buf), buf, " \n\r         ----===   {gMortals   {w===----{x\n\r");
  add_buf(output,buf);
  mprintf(sizeof(buf), buf, "{gName           {wRace/{wClass   {wPosition        Lev  %%hps/{w%%mana   [Vnum]  {BIdle{x\n\r");
  add_buf(output,buf);
  hptemp = 0;

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    CHAR_DATA *wch;
    char const *gameclass;

    if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
      continue;

    wch   = ( d->original != NULL ) ? d->original : d->character;

    if (!can_see(ch,wch) || wch->level > ch->level || wch->level > LEVEL_HERO)
      continue;

    mortmatch++;
    if ((GET_HP(wch) != wch->hit) && (wch->hit > 0))
      hptemp = (wch->hit*100)/GET_HP(wch);
    else if (GET_HP(wch) == wch->hit)
      hptemp = 100;
    else if (wch->hit < 0)
      hptemp = 0;

    if ((GET_MANA(wch) != wch->mana) && (wch->mana > 0))
      manatemp = (wch->mana*100)/GET_MANA(wch);
    else if (GET_MANA(wch) == wch->mana)
      manatemp = 100;
    else if (wch->mana < 0)
      manatemp = 0;

    gameclass = class_table[wch->gameclass].who_name;

    //Added in IDLE timer to omni for the Mortal section (idle)
    vch = d->original ? d->original : d->character;

    if ( vch->timer > 0 )
      mprintf( sizeof(idle), idle, "{R%d{x", vch->timer );
    else
      mprintf( sizeof(idle), idle, "    " );



    mprintf(sizeof(buf), buf, "%-14s  {w%5s/%3s     %-15s %-3d  %3d%%/%3d%%   [%5d]  {R%s{x\n\r",
            wch->name,
            wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name : "     ",
            gameclass,
            capitalize( position_table[wch->position].name) ,
            wch->level,
            hptemp,manatemp,
            wch->in_room->vnum,
            idle);
    add_buf(output,buf);
  }

  /*
   * Tally the counts and send the whole list out.
   */
  mprintf(sizeof(buf2), buf2, "\n\rIMMs found: %d\n\r", immmatch );
  add_buf(output,buf2);
  mprintf(sizeof(buf2), buf2, "Morts found: %d\n\r", mortmatch );
  add_buf(output,buf2);
  page_to_char( buf_string(output), ch );
  free_buf(output);
  return;
}


void do_finger(CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA d;
  DENIED_DATA *dnd;
  char str[100];
  char arg[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MSL];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim,*qch;
  bool load_char = FALSE;
  int v1=0;

  if ( argument[0] == '\0' ||
       argument[0] == '/'  ||
       argument[0] == '\\' ||
       argument[0] == '.')
  {
    send_to_char("Finger who?\n\r", ch);
    return;
  }

  smash_tilde(argument);
  argument = one_argument(argument, arg);
  strcpy(arg2, argument);

  for (dnd = denied_list; dnd; dnd = dnd->next)
    if (is_exact_name(capitalize(arg),dnd->name))
    {
      send_to_char("That char is Permanently {RDENIED{x\n\r",ch);
      return;
    }

  if (((victim = get_char_world(ch, arg)) != NULL)
      && (!IS_NPC(victim))
      && !IS_LINKDEAD(victim))
  {
    load_char = FALSE;
  }
  else
  {
    d.original = NULL;

    if (!load_char_obj(&d, capitalize(arg))) /* char pfile exists? */
    {
      printf_to_char(ch, "{RNo such player: {W%s{x.\n\r", capitalize(arg));
      free_char(d.character);
      return;
    }

    d.character->desc = NULL; /* safe than sorry */
    victim = d.character;
    load_char = TRUE;
  }

  //I guess just to be safe. - 10/12/09
  if ( !victim )
  {
    send_to_char( "Error: Victim was not found.\n\r", ch );
    return;
  }

  send_to_char("-----------------------------------------------------------------------------{x\n\r",ch);
  if (load_char || !can_see(ch,victim))
    printf_to_char(ch, "{g%s{w is not currently playing.{x\n\r", victim->name);
  else if (victim->desc != NULL)
    printf_to_char(ch, "{g%s{w is currently playing.{x\n\r", victim->name);
  else
    printf_to_char(ch, "{g%s{w is currently Linkdead{x.\n\r", victim->name);
  send_to_char("\n\r\n\r",ch);

  if ( IS_IMMORTAL( victim ) )
  {
    if (victim->short_descr[0] != '\0')
      printf_to_char( ch, "Name:     %s\n\r", victim->short_descr );
    else
      printf_to_char( ch, "Name:     %s\n\r", victim->name );
  }
  else
  {
    printf_to_char(ch, "Name:     %s\n\r",
                   victim->name );
  }

  if ( strlen( victim->pcdata->title ) < 2 )
    printf_to_char( ch, "Title:   %s\n\r", ", the young" );
  else
  {
    strcpy(buf,victim->pcdata->title);
    if ( !strchr( ".,?!':", *buf ) )
    {
      printf_to_char( ch, "Title:   %s\n\r", victim->pcdata->title );
    }
    else
    {
      printf_to_char( ch, "Title:    %s\n\r", victim->pcdata->title );
    }
  }

  if ( IS_IMMORTAL(ch) || (ch == victim) )
  {
    if ( victim->pcdata->afk_title )
      printf_to_char( ch, "AFK:     %s\n\r", victim->pcdata->afk_title );
    else
      printf_to_char( ch, "AFK:     %s\n\r", " is {w[{gA{GF{gK{w]{x" );
  }

  if ( victim->pcdata->hair_color )
  {
    mprintf( sizeof(arg3), arg3, "Hair Color: %s",
             capitalize(victim->pcdata->hair_color) );
  }
  else
    sprintf( arg3, "%s", " " );

  printf_to_char( ch, "Sex:      %-15s                     %s\n\r",
                  sex_table[victim->sex], arg3 );

  if ( victim->pcdata->eye_color )
  {
    mprintf( sizeof(arg3), arg3, "Eye Color:  %s",
             capitalize(victim->pcdata->eye_color) );
  }
  else
    sprintf( arg3, "%s", " " );

  printf_to_char( ch, "Level     %-15d                     %s\n\r",
                  victim->level, arg3 );

  if ( victim->pcdata->skin_color )
  {
    mprintf( sizeof(arg3), arg3, "Skin Color: %s",
             capitalize(victim->pcdata->skin_color) );
  }
  else
    sprintf( arg3, "%s", " " );

  printf_to_char( ch, "Age:      %-15d                     %s\n\r",
                  get_age(victim), arg3 );

  if ( victim->pcdata->weight > 0 )
  {
    mprintf( sizeof(arg3), arg3, "Weight:     %d lbs.",
             victim->pcdata->weight );
  }
  else
    sprintf( arg3, "%s", " " );

  printf_to_char(ch, "Race:     %-15s                     %s\n\r",
                 race_table[victim->race].name, arg3 );

  if ( victim->pcdata->height > 0 )
  {
    int feet = 0, inches = 0;
    feet = victim->pcdata->height / 12;
    inches = victim->pcdata->height % 12;
    mprintf( sizeof(arg3), arg3, "Height:     %d in. (%d'%d\")",
             victim->pcdata->height, feet, inches );
  }
  else
    sprintf( arg3, "%s", " " );

  printf_to_char( ch, "Class     %-15s                     %s\n\r",
                  class_table[victim->gameclass].name, arg3 );

  if (victim->alignment < -500)
  {
    strcpy(buf,"{R");
  }
  else
  {
    if (victim->alignment > 500)
    {
      strcpy(buf,"{Y");
    }
    else
    {
      strcpy(buf,"{W");
    }
  }

  printf_to_char( ch, "Align:    %s%d{x\n\r", buf, victim->alignment);

  if (victim->pcdata->bounty)
    printf_to_char(ch, "Bounty:   %d{x\n\r",victim->pcdata->bounty);


  if ( IS_IN_CLAN( victim) ) //(victim->clan)
  {
    printf_to_char(ch, "Clan:     %s\n\r{wRank:     %s{x\n\r",
                   victim->clan->symbol,
                   IS_IN_CLAN( victim ) ? get_rank( victim ) : " " );
  }

  if ( IS_IMMORTAL(ch) || ( ch == victim ) )
  {
    v1 = ( victim->played + (int) (current_time - victim->logon) ) / 3600;

    printf_to_char( ch,
             "Played{x:   %d (%d Year%s, %d Day%s, %d Hour%s){x\n\r",
             v1,
             v1/24/365,
             ( v1/24/365 == 1 ) ? "" : "s",
             (v1/24) % 365,
             ( (v1/24) % 365 == 1 ) ? "" : "s",
             v1 % 24,
             ( (v1 % 24) == 1 ) ? "" : "s" );

    printf_to_char(ch, "Created:  {w%s{x\n\r",
                   ctime( &victim->id ));
  }

  if ( IS_IMMORTAL( ch )
       &&   IS_SET(victim->act, PLR_FREEZE) )
    printf_to_char(ch, "{CThis character has been FROZEN.{x\n\r");


  if (( !IS_IMMORTAL( ch ) &&  !IS_IMMORTAL( victim ) )
      ||   IS_IMMORTAL( ch ) )
  {

    if ( load_char || !can_see( ch,victim ) )
    {
      printf_to_char( ch, "\n\rLast Login Time at %s\n\r",
                      ctime( &victim->pcdata->laston ) );
      if ( IS_IMMORTAL( ch ) )
        printf_to_char( ch, "Logged out in room: %s [%d]\n\r",
                        (  victim->in_room == get_room_index( ROOM_VNUM_LIMBO )
                           && victim->was_in_room != NULL )
                        ? victim->was_in_room->name
                        : victim->in_room == NULL ? "{wA {gG{ca{gr{md{re{gn{x Sanctuary" : victim->in_room->name,
                        (  victim->in_room == get_room_index( ROOM_VNUM_LIMBO )
                           && victim->was_in_room != NULL )
                        ? victim->was_in_room->vnum
                        : victim->in_room == NULL ? ROOM_VNUM_ALTAR : victim->in_room->vnum );
    }
    else
    {
      strftime( str, 100, "%I:%M%p", localtime( &victim->logon ) );
      printf_to_char( ch, "\n\rLogged on at : {g%s{x\n\r",str );
      if ( IS_IMMORTAL( ch ) )
        printf_to_char( ch, "In Room: %s [%d]\n\r",
                        victim->in_room->name, victim->in_room->vnum );
    }

  }

  if ( victim->description[0] != '\0' )
    printf_to_char(ch,"\n\r{gDescription:\n\r{x%s\n",victim->description);
  else
    printf_to_char(ch,"{gDescription:\n\r{x%s\n","Player has not made a description yet.");
  send_to_char("----------------------------------------------------------------------------{x\n\r",ch);
  if ((IS_IMMORTAL(ch) && (get_trust(ch) >= IMPLEMENTOR)))
  {
    if (load_char || !can_see(ch,victim))
      printf_to_char(ch, "%s last logged on from -%s-\n\r", victim->name,
                     victim->pcdata->host);
    else if (victim->desc != NULL)
      printf_to_char(ch, "%s is logged on from -%s-\n\r", victim->name,
                     victim->desc->host);
    else
      printf_to_char(ch, "%s last logged on from -%s-\n\r", victim->name,
                     victim->pcdata->host);
    if ( check_ban(victim->pcdata->host,BAN_ALL))
      printf_to_char(ch, "{RThis host has been BANNED.{x\n\r");
    send_to_char("------------------------------------------------------------------------------{x\n\r",ch);
  }
  if (IS_IMMORTAL(ch) && IS_SET(victim->act, PLR_DENY))
    printf_to_char(ch,"State: {RDENIED{x\n\r");

  /*  char_to_room(d.character, get_room_index(ROOM_VNUM_LIMBO));*/
  if (load_char)
  {
    for ( qch = char_list ; qch ; qch = qch->next)
    {
      // Check for rescue-quest mob and remove if loaded
      if ( !IS_NPC( qch ) )
        continue;

      if ( strstr(qch->name,victim->name )
           && ( qch->pIndexData->vnum == MOB_VNUM_QUEST ) )
      {
        do_function(qch,&do_follow,"self");
        do_function(qch,&do_nofollow,"");
        extract_char( qch, TRUE );
      }
    }

    nuke_pets(d.character,FALSE);
    free_char(d.character);
  }
  return;
}

void do_donate( CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj, *obj_next;
  CLAN_DATA *clan = NULL;
  int found;
  char arg[MAX_INPUT_LENGTH];
  char arg4[MAX_INPUT_LENGTH];
  char buf[MIL];
  int count = 0,i = 0, n = 0, clan_owed=0, clan_owed_c=0;

  argument = one_argument(argument, arg);

  if (arg[0] == '\0' )
  {
    send_to_char("Donate what?\n\r",ch);
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("Donations are useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if (ch->position == POS_FIGHTING)
  {
    send_to_char(" You're {rfighting{x!{x\n\r",ch);
    return;
  }

  if (!IS_AWAKE(ch))
  {
    send_to_char("Awaken first.\n\r",ch);
    return;
  }

  found = FALSE;

  if ( !str_prefix( "all.", arg ) || !str_prefix("all",arg))
  {
    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
      obj_next = obj->next_content;

      if (obj->next_content == obj)
      {
        bugf("Error in donate: Infinite obj loop. %s",interp_cmd);
        obj_next = NULL;
      }

      /*if ( ( clan = get_clan( ch->clan_name, FALSE ) )
      && !IS_SET( clan->clan_flags, CLAN_INDEPENDENT ) )
      {
        if ( obj->pIndexData->vnum == clan->donation_gem )
        {
          obj_from_char( obj );
          extract_obj( obj );
          i++;
          found = FALSE;
        }
      }*/

      if ( (arg[3] == '\0' || is_name( &arg[4], obj->name ) )
           && can_drop_obj(ch, obj)
           && obj->timer <= 0
           //&& ( obj->pIndexData->vnum != clan->donation_gem )
           && obj->wear_loc == WEAR_NONE
           && obj->item_type != ITEM_MONEY
           && obj->item_type != ITEM_CORPSE_NPC
           && obj->item_type != ITEM_CORPSE_PC)
      {
        if ( !can_use_clan_obj( ch, obj ) )
        {
          send_to_char( "That belongs to another clan.\n\r", ch );
          continue;
        }

        if ( ch->pcdata->clanowe_clan
             &&   ( clan = get_clan(ch->pcdata->clanowe_clan, TRUE) )
             &&   i < 1 ) //Just in case
        {
          if (!clan) continue; //To be safe

          if ( IS_SET(clan->clan_flags, CLAN_INDEPENDENT))
            continue;

          if ( obj->pIndexData->vnum == clan->donation_gem )
          {
            if ( ch->pcdata->clanowe_dia > 0 )
            {
              found = TRUE;
              clan_owed++;
              ch->pcdata->clanowe_dia--;

              if ( ch->pcdata->clanowe_dia == 0 )
                send_to_char( "Congratulations, you have paid off your debt!\n\r", ch );

              do_donate_clan( ch, obj, 1, clan );
              continue;
            }
          }
        }

        if (IS_SET(obj->extra_flags, ITEM_RESTRING))
        {
          send_to_char("You cannot donate restrung items!\n\r", ch );
          continue;
        }

        if ( ( IS_IN_CLAN( ch )
               &&   obj->pIndexData->vnum != ch->clan->donation_gem )
             ||   !IS_IN_CLAN( ch ) )
        {
          found = TRUE;
          donate_obj(ch,obj,1);
        }

        if ( ch->clan && !IS_SET(ch->clan->clan_flags, CLAN_INDEPENDENT) )
        {
          if ( obj->pIndexData->vnum == ch->clan->donation_gem )
          {
            //obj_from_char( obj );
            //extract_obj( obj );
            i++;
            found = TRUE;
            do_donate_clan( ch, obj, 1, ch->clan );
          }
        }
      }
    }

    if ( clan_owed > 0 ) // This and i(below) should never both be above 0
    {
      char buf3[MSL];

      printf_to_char( ch, "You have donated %s gem%s to your debt.\n\r",
                      numcpy( buf, clan_owed ), clan_owed == 1 ? "" : "s" );
      mprintf(sizeof(buf3), buf3,
              "{g%s{w donated %s gem%s to %s as a debt payment (%d){x.",
              ch->name,
              numcpy( buf, clan_owed ),
              clan_owed == 1 ? "" : "s",
              clan->symbol,
              clan->donation_balance );
      wiznet( buf3, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );
    }

    if ( i > 0 )
    {
      char buf3[MSL];

      printf_to_char( ch, "You have donated %s gem%s toward your clan.\n\r",
                      numcpy( buf, i ), i == 1 ? "" : "s" );
      mprintf(sizeof(buf3), buf3,
              "{g%s{w donated %s gem%s to %s (%d){x.",
              ch->name,
              numcpy( buf, i ),
              i == 1 ? "" : "s",
              ch->clan->symbol,
              ch->clan->donation_balance );
      wiznet( buf3, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );
      do_worth( ch, "" );
      //clan->donation_total += i;
      //clan->donation_balance += i;
      //ch->pcdata->donated_dia += i;
      //copy_roster_clannie( ch );
      //do_short_diamonds( ch, "" );
      //save_clan( ch->clan );
      //return;
    }

    if (!found) send_to_char("Nothing to donate.\n\r",ch);
    return;
  }

  count = mult_argument(arg, arg4);

  if (count < 1)
  {
    send_to_char("Donate how many?\n\r",ch);
    return;
  }

  if (count > 50)
  {
    send_to_char("You cannot donate more than 50 items.\n\r",ch);
    return;
  }

// donate a corpse from ground before one in inventory (autodonate bugfix)
  if ( !strcmp(arg4,"corpse")
       &&   (obj = get_obj_here(ch, "corpse"))
       &&   (obj->carried_by == NULL) // because, strangely, get_obj_here also checks inventory
       &&   (obj->item_type == ITEM_CORPSE_NPC) )
  {
    if (obj->contains)
    {
      send_to_char("That must be empty to donate!\n\r",ch);
      return;
    }

    donate_obj(ch,obj,0);
    return;
  }

  for (i=0; i < count; i++)
  {
    if ( (obj = get_obj_carry(ch, arg4, ch) ) == NULL)
    {
      if ( (obj = get_obj_here (ch, arg4)) == NULL)
      {
        send_to_char("You do not have that!\n\r",ch);
        continue;
      }
      else // object was found on ground, and can now be donateed -- Taeloch
      {
        if ( ( obj->item_type == ITEM_CORPSE_NPC
               ||     obj->item_type == ITEM_CORPSE_PC )
             &&     obj->contains )
        {
          send_to_char("That must be empty to donate!\n\r",ch);
          continue;
        }

        if ( obj->timer > 0
             &&   ( obj->item_type != ITEM_CORPSE_NPC
                    &&     obj->item_type != ITEM_CORPSE_PC ) )
        {
          send_to_char("You cannot donate that.\n\r",ch);
          continue;
        }

        if ( !CAN_WEAR( obj, ITEM_TAKE ) )
        {
          send_to_char("You cannot donate that.\n\r",ch);
          continue;
        }

        if ( !can_use_clan_obj( ch, obj ) )
        {
          send_to_char( "That belongs to another clan.\n\r", ch );
          continue;
        }

        if (IS_SET(obj->extra_flags, ITEM_RESTRING))
        {
          send_to_char("You cannot donate restrung items!\n\r", ch );
          continue;
        }

        if ( ( clan = get_clan( ch->clan_name, FALSE ) )
             &&     !IS_SET( clan->clan_flags, CLAN_INDEPENDENT ) )
        {
          if ( obj->pIndexData->vnum == clan->donation_gem )
          {
            n++;
            do_donate_clan( ch, obj, 0, ch->clan );
            continue;
            //return;
          }
        }
        donate_obj(ch,obj,0);
        continue;
      } // end object on ground
    }
    else
    {
      if (!can_drop_obj(ch, obj) && ch->level <= LEVEL_HERO)
      {
        send_to_char("Its stuck to you.\n\r",ch);
        continue;
      }

      if ( ( obj->item_type == ITEM_CORPSE_NPC
             ||     obj->item_type == ITEM_CORPSE_PC )
           &&     obj->contains )
      {
        send_to_char("That must be empty to donate!\n\r",ch);
        continue;
      }

      if ( obj->timer > 0
           &&   ( obj->item_type != ITEM_CORPSE_NPC
                  &&     obj->item_type != ITEM_CORPSE_PC ) )
      {
        send_to_char("You cannot donate that.\n\r",ch);
        continue;
      }

      if (IS_SET(obj->extra_flags, ITEM_RESTRING))
      {
        send_to_char("You cannot donate restrung items!\n\r", ch );
        continue;
      }

      if ( !can_use_clan_obj( ch, obj ) )
      {
        send_to_char( "That belongs to another clan.\n\r", ch );
        continue;
      }

      if ( ch->pcdata->clanowe_clan
           &&   ( clan = get_clan(ch->pcdata->clanowe_clan, TRUE) ) )
      {
        if (!clan) continue; //To be safe

        if ( IS_SET(clan->clan_flags, CLAN_INDEPENDENT))
          continue;

        if ( obj->pIndexData->vnum == clan->donation_gem )
        {
          if ( ch->pcdata->clanowe_dia > 0 )
          {
            clan_owed_c++;
            ch->pcdata->clanowe_dia--;

            if ( ch->pcdata->clanowe_dia == 0 )
              send_to_char( "Congratulations, you have paid off your debt!\n\r", ch );

            do_donate_clan( ch, obj, 1, clan );
            continue;
          }
        }
      }
      else if ( ( clan = get_clan( ch->clan_name, FALSE ) )
                &&     !IS_SET( clan->clan_flags, CLAN_INDEPENDENT ) )
      {
        if ( obj->pIndexData->vnum == clan->donation_gem )
        {
          n++;
          do_donate_clan( ch, obj, 1, ch->clan );
          continue;
          //return;
        }
      }
      donate_obj(ch,obj,1);
    }
  }

  if ( clan_owed_c > 0 )
  {
    char buf3[MSL];

    if ( clan_owed_c == 1 ) // This and i(below) should never both be above 0
    {
      send_to_char( "You donate one gem to your debt.\n\r", ch );
      mprintf(sizeof(buf3), buf3,
              "{g%s{w donated one gem to %s as a debt payment (%d){x.",
              ch->name,
              clan->symbol,
              clan->donation_balance );
      wiznet( buf3, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );
    }
    else
    {
      printf_to_char( ch, "You have donated %s gem%s to your debt.\n\r",
                      numcpy( buf, clan_owed_c ), clan_owed_c == 1 ? "" : "s" );
      mprintf(sizeof(buf3), buf3,
              "{g%s{w donated %s gem%s to %s as a debt payment (%d){x.",
              ch->name,
              numcpy( buf, clan_owed_c ),
              clan_owed_c == 1 ? "" : "s",
              clan->symbol,
              clan->donation_balance );
      wiznet( buf3, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );
    }
  }

  if ( n > 0 )
  {
    char buf4[MSL];

    if ( n == 1 )
    {
      send_to_char( "You donate one gem to your clan.\n\r", ch );
      mprintf( sizeof(buf4), buf4, "{g%s{x donated one gem to %s (%d)",
               ch->name,
               ch->clan->symbol,
               ch->clan->donation_balance );
      wiznet( buf4, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );
    }
    else
    {
      printf_to_char( ch, "You donate %d gems to your clan.\n\r", n );
      mprintf( sizeof(buf4), buf4, "{g%s{x donated %d gems to %s (%d)",
               ch->name,
               n,
               ch->clan->symbol,
               ch->clan->donation_balance );
      wiznet( buf4, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );

    }

    do_worth( ch, "" );
  }

}

void donate_obj(CHAR_DATA *ch, OBJ_DATA *obj, int frominv)
{
  OBJ_DATA *pit;
  int amount, reward;
  int pit_vnum;

  reward = UMAX( 1, (obj->level * 3) / 4 );

  if (IS_WORN(obj))
  {
    send_to_char("You cannot donate something you are wearing!\n\r",ch);
    return;
  }

  if (obj->item_type == ITEM_MONEY)
  {
    send_to_char("You cannot donate coins.\n\r",ch);
    return;
  }

  if ( obj->item_type != ITEM_CORPSE_NPC
       &&   obj->item_type != ITEM_CORPSE_PC )
    reward = UMIN( reward, obj->cost / 3 );

  act("$n donates {W$p{x to $g.",ch,obj,NULL,TO_ROOM);
  act("You donate {W$p{x to $g.",ch,obj,NULL,TO_CHAR);

  if ( reward < 0 )
  {
    bugf( "do_donate: Invalid Reward: %d", reward );
    reward = 0;
  }

  switch ( ch->gameclass )
  {
    default:
      ch->silver += reward;
      break;
    case cConjurer:
    case cOccultist:
    case cWarlock:
      if ( GET_MANA(ch) > ch->mana )
        ch->mana += UMIN( reward, GET_MANA(ch) - ch->mana );
      if ( GET_MANA(ch) < ch->mana )
        ch->mana = GET_MANA(ch);
      break;
    case cInquisitor:
    case cDruid:
    case cPriest:
    case cAlchemist:
      if ( GET_MANA(ch) > ch->mana )
        ch->mana += UMIN( reward/2, GET_MANA(ch) - ch->mana );
      if ( GET_HP(ch) > ch->hit )
        ch->hit += UMIN( reward/2, GET_HP(ch) - ch->hit );
      if ( GET_MANA(ch) < ch->mana )
        ch->mana = GET_MANA(ch);
      if ( GET_HP(ch) < ch->hit )
        ch->hit = GET_HP(ch);
      break;
    case cBarbarian:
    case cKnight:
    case cHighwayman:
    case cMystic:
    case cWoodsman:
      if ( GET_HP(ch) > ch->hit )
        ch->hit += UMIN( reward, GET_HP(ch) - ch->hit );
      if ( GET_HP(ch) < ch->hit )
        ch->hit = GET_HP(ch);
      break;
  }

  if ((!IS_OBJ_STAT(obj ,ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
      (!IS_OBJ_STAT(obj ,ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
      IS_NEUTRAL(ch))
    if (obj->cost > 0 && obj->level > 0)
    {
      amount = UMAX(1, obj->cost/8);

      if (amount == 1)
      {
        printf_to_char(ch, "You receive {Wone {wsi{Wl{Dv{wer{x for your donation.\n\r");
      }
      else
      {
        printf_to_char(ch, "You receive {W%d {wsi{Wl{Dv{wer{x for your donation.\n\r",amount);
      }

      ch->silver += amount;
    }

  pit_vnum = 0;
  if ( IS_IN_CLAN( ch )
       && ( !IS_SET( ch->clan->clan_flags, CLAN_INDEPENDENT ) ) )
  {
    pit_vnum = ch->clan->donation_obj[ch->in_room->area->continent];
    if (pit_vnum == 0) // default to Kaishaan pit
      pit_vnum = ch->clan->donation_obj[0];
  }
  if (pit_vnum == 0)
  {
    if ( IS_EVIL( ch ) )
      pit_vnum = OBJ_VNUM_EVIL_DONATE;
    else if ( IS_GOOD( ch ) )
      pit_vnum = OBJ_VNUM_GOOD_DONATE;
    else
      pit_vnum = OBJ_VNUM_NEUT_DONATE;
  }

  /* Set the object to useless since */
  /* you donated it */
  if ( ( pit = find_obj_vnum( pit_vnum ) ) )
  {
    obj->cost = 0;
    if (frominv)
    {
      obj_from_char( obj );
    }
    else
    {
      obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_CORPSE_NPC
         ||   obj->item_type == ITEM_CORPSE_PC )
    {
      // prevent redonating corpses from pits
      extract_obj( obj );
    }
    else
    {
      obj_to_obj( obj, pit );
    }
  }
  else
  {
    send_to_char( "That pit is unavailible right now.\n", ch );
  }
}



void do_bounty( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );


  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  {
    send_to_char( "Place a bounty on who's head?\n\rSyntax:Bounty <victim> <amount>\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL)
  {
    send_to_char( "They are currently not logged in!", ch );
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char( "You cannot put a bounty on NPCs!", ch );
    return;
  }

  if (ch == victim && !IS_IMMORTAL(ch))
  {
    send_to_char( "You cannot place a bounty upon yourself.\n\r", ch);
    return;
  }

  if ( is_number( arg2 ) )
  {
    int amount;
    amount   = atoi(arg2);
    amount = abs(amount);
    if (ch->gold < amount)
    {
      send_to_char( "You don't have that much gold!\n\r", ch );
      return;
    }
    if (amount <=0)
    {
      send_to_char("What good is placing a bounty of ZERO?\n\r",ch);
      return;
    }
    ch->gold -= amount;
    victim->pcdata->bounty +=amount;
    printf_to_char(ch, "You have placed a {Y%d {yg{Yo{yld{x bounty on %s{g.\n\r%s now has a bounty of {Y%d {yg{Yo{yld{x.\n\r",
                   amount,victim->name,victim->name,victim->pcdata->bounty );
    /* specific stuff for The Mage's Lair public channel system */
    mprintf(sizeof(buf), buf, "emote has placed a {Y%d {yg{Yo{yld{x bounty on %s{g.",  amount,victim->name );
    do_function(ch,&do_info, buf);
    return;
  }
  else
  {
    send_to_char("That is not a number.\n\r",ch);
    return;
  }
}

char *wholine(CHAR_DATA *ch, CHAR_DATA *wch)
{
  char gameclass[25];
  char race[15];
  char title[MSL];
  char alevel[MSL];
  static char buf[MSL];
  bool pkable=FALSE;
  bool killable=FALSE;
  char name[MSL];
  bool wanted=FALSE;
  bool building=FALSE;
  bool bailed=FALSE;

  /*
   * Figure out what to print for class.
   */
  strcpy(gameclass,class_table[wch->gameclass].who_name);
  if (IS_GOOD(wch))
    mprintf(sizeof(alevel), alevel,"{W%-3d{x",wch->level);
  else if (IS_EVIL(wch))
    mprintf(sizeof(alevel), alevel,"{D%-3d{x",wch->level);
  else
    mprintf(sizeof(alevel), alevel,"{w%-3d{x",wch->level);
  switch ( wch->level )
  {
    default:
      break;
      {
      case MAX_LEVEL + 1 :
        strcpy(alevel,"{gOw{cn{x");
        break;
      case MAX_LEVEL - 0 :
        strcpy(alevel,"{yI{wm{yp{x");
        break;
      case MAX_LEVEL - 1 :
        strcpy(alevel,"{mC{cre{x");
        break;
      case MAX_LEVEL - 2 :
        strcpy(alevel,"{wS{Du{wp{x");
        break;
      case MAX_LEVEL - 3 :
        strcpy(alevel,"{rDe{gi{x");
        break;
      case MAX_LEVEL - 4 :
        strcpy(alevel,"{bG{mo{bd{x");
        break;
      case MAX_LEVEL - 5 :
        strcpy(alevel,"{yI{gmm{x");
        break;
      case MAX_LEVEL - 6 :
        strcpy(alevel,"{DDe{rm{x");
        break;
      case MAX_LEVEL - 7 :
        strcpy(alevel,"{wA{yng{x");
        break;
      case MAX_LEVEL - 8 :
        strcpy(alevel,"{wAva{x");
        break;
      case MAX_LEVEL - 9 :
        if (IS_GOOD(wch))
          strcpy(alevel,"{yH{cro{x");
        else if (IS_EVIL(wch))
          strcpy(alevel,"{DV{rln{x");
        else
          strcpy(alevel,"{xHro{x");
        break;
      }
  }

  if (is_in_pk_range(ch,wch)
      && ch->clan
      && !IS_SET( wch->clan->clan_flags, CLAN_PEACEFUL )
      && !IS_SET( ch->clan->clan_flags, CLAN_PEACEFUL ) )
    pkable = TRUE;
  else
    pkable = FALSE;

  if (is_in_pk_range(wch,ch)
      && ch->clan
      && !IS_SET( wch->clan->clan_flags, CLAN_PEACEFUL )
      && !IS_SET( ch->clan->clan_flags, CLAN_PEACEFUL ) )
    killable = TRUE;
  else
    killable = FALSE;

  if (wch->desc)
    if (wch->desc->editor)
      building = TRUE;

  if (wch->pcdata->bounty >= 10000)
    wanted = TRUE;
  else
    wanted = FALSE;

  if ( IS_IMMORTAL( ch )
       ||   ch == wch )
    if (wch->bailed)
      bailed = TRUE;
  /*
   * Format it up.
   */
  clrstrcenter(alevel,3);
  if (IS_GHOST(wch) )
    mprintf(sizeof(name), name,"{W[{D%s{W]{x",wch->name);
  else if (wch->level > LEVEL_HERO)
  {
    if (wch->short_descr[0] != '\0')
      strcpy( name, wch->short_descr);
    else
      strcpy(name, wch->name);
  }
  else if (wch->short_descr[0] != '\0')
    strcpy( name, wch->short_descr);
  else
    strcpy(name, wch->name);

  strcpy(race, wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name
         : "     ");

  if ( IS_AFK( wch ) )
  {
    if ( wch->pcdata->afk_title )
      mprintf( sizeof( title ), title, "%s", wch->pcdata->afk_title );
    else
      mprintf( sizeof( title ), title, "%s", " is {w[{gA{GF{gK{w]{x" );
  }
  else
  {
    if ( strlen( wch->pcdata->title ) < 2 )
      mprintf( sizeof( title ), title, "%s", ", the young" );
    else
      mprintf( sizeof( title ), title, "%s", wch->pcdata->title );
  }

  mprintf(sizeof(buf), buf, "{W[{w%-3s %s %s{W]{x%s{w%s{w%s{w%s{w%s {w%s%s%s%s%s%s%s%s%s%s%s%s{x\n\r",
          alevel,
          race,
          gameclass,
          (pkable) ? "{W*{w" : " ",
          IS_IN_CLAN( wch ) ? wch->clan->symbol : "         ",
          (killable) ? "{R*{w" : " ",
          /*    (pkable) ? "{W[*{RPK{W*]{w" : "",
            (killable) ? "{R[>{WPK{R<]{w" : "",*/
          name,
          //(strlen(wch->pcdata->title) < 6)? ", the young": wch->pcdata->title,
          title,
          IS_IN_CLAN( wch ) ? get_rank( wch ) : " ",
          //(wch->clan) ? (wch->clan == 6 ? "" : clan_rank_table[wch->pcdata->clan_rank].title_of_rank[wch->sex]) : "",
          wch->incog_level >= LEVEL_HERO ? "{DI{x" : "",
          wch->invis_level >= LEVEL_HERO ? "{cW{x" : "",
          building ? "{B[{cOLC{B]{x" : "",
          IS_SET(wch->act, PLR_KILLER) ? "{w({rK{w){x" : "",
          IS_SET(wch->act, PLR_THIEF)  ? "{w({DT{w){x"  : "",
          IS_SET(wch->act, PLR_VIOLENT)  ? "{w({yV{w){x"  : "",
          IS_SET(wch->chan_flags, CHANNEL_QUIET) ? "{c({yQ{c){x" : "",
          wanted ? "{w({RW{w){x" : "",
          bailed ? "{w({BB{w){x" : "",
          IS_SET(wch->act, PLR_TWIT)  ? "{R({YT{BW{MI{GT{R){x"  : "",
          ((wch->level <= LEVEL_HERO) && wch->desc->pString) ? " {Y[{yEDIT{Y]{x" : "" );
  return(buf);
}
/*+
 * strcenter - Center a String in the Number of Characters Given
 *
 * SYNOPSIS:
 *       strcenter(char *input_string, int size_of_string)
 *  input_string - String to Change
 *  size_of_string - Center Within this number.
 *
 * DESCRIPTION:
 *       {strcenter} will take a given string and center it into the
 *  number of characters given.  This basically right pads and
 *  left pads the string with spaces and allow the string to be
 *  centered.
 *
 *  SEE ALSO:
 *  strlpad()
 *  strrpad()
 *  div()
 *
 */

void clrstrcenter( char *input_string, int size_of_string )
{
  char tempstr[MSL];
  int temp_int,padlength;
  div_t x;

  strncpy( tempstr, input_string, MSL );
  strip_string( tempstr );
  temp_int = size_of_string - colorstrlen( tempstr );
  x = div( temp_int, 2 );
  padlength = x.quot;
  strrpad( tempstr, padlength+strlen( tempstr ) );
  strlpad( tempstr, padlength+strlen( tempstr ) );
  if ( colorstrlen( tempstr ) != size_of_string )
    strlpad( tempstr, 1 );
  strcpy( input_string, tempstr );
}




void strlpad( char *string, int num )
{
  char  temp[100];
  int    i;

  strcpy( temp, "" );
  for ( i = 0;  i < num-strlen( string ); i++ )
    strcat( temp, " " );
  strcat( temp, string );
  strcpy( string, temp );
}


void strrpad( char *string, int num )
{
  char  temp[100];
  int  i;

  strcpy( temp, string );
  for ( i = 0; i < num-strlen( string ); i++)
    strcat( temp, " " );
  strcpy( string, temp );
}

void do_history( CHAR_DATA *ch, char *argument )
{
  BUFFER *buffer;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument,arg);

  if (IS_NPC(ch))
  {
    send_to_char("Get Real.\n\r",ch);
    return;
  }
  if (IS_NULLSTR(arg))
  {
    buffer = new_buf();
    add_buf(buffer, "{DHistory for {w");
    add_buf(buffer, ch->name);
    add_buf(buffer, "{x\n\r");
    add_buf(buffer, "{g-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\n\r");
    add_buf(buffer,ch->pcdata->history);
    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);
    return;
  }
  /*
    if (!str_cmp(arg,"add")) {
      free_string(ch->pcdata->history);
      ch->pcdata->history = str_dup("", ch->pcdata->history);
      string_append(ch, &ch->pcdata->history);
      return;
    }
  */
  if (!str_cmp(arg,"mod")
      ||  !str_cmp(arg,"add") )
  {
    string_append(ch, &ch->pcdata->history, APPEND_CHAR, ch);
    return;
  }

  if (!str_cmp(arg,"delete"))
  {
    free_string(ch->pcdata->history);
    ch->pcdata->history = str_dup("", ch->pcdata->history);
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if (victim->invis_level > get_trust(ch))
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if (victim->incog_level > get_trust(ch))
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("Not on a mobile.\n\r",ch);
    return;
  }

  if (!can_see(ch,victim))
  {
    send_to_char("They aren't here.\n\r",  ch );
    return;
  }

  if (victim->pcdata)
  {
    if (victim->pcdata->history)
    {
      buffer = new_buf();
      add_buf(buffer, "{DHistory for {w");
      add_buf(buffer, victim->name);
      add_buf(buffer, "{x\n\r");
      add_buf(buffer, "{g-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-{x\n\r");
      add_buf(buffer,victim->pcdata->history);
      page_to_char(buf_string(buffer),ch);
      free_buf(buffer);
      return;
    }
    else
    {
      send_to_char("No history exists for that player.\n\r",ch);
      return;
    }
  }
}

#define NUM_DAYS 35
/* Match this to the number of days per month; this is the moon cycle */
#define NUM_MONTHS 17
/* Match this to the number of months defined in month_name[].  */
#define MAP_WIDTH 72
#define SHOW_WIDTH MAP_WIDTH/2
#define MAP_HEIGHT 9
/* Should be the string length and number of the constants below.*/
const char * star_map[] =
{
  "   W.N     ' .     :. M,N     :  y:.,N    `  ,       B,N      .      .  ",
  " W. :.N .      G,N  :M.: .N  :` y.N    .      :     B:   .N       :     ",
  "    W:N    G.N:       M:.,N:.:   y`N      ,    c.N           .:    `    ",
  "   W.`:N       '. G.N  `  : ::.      y.N      c'N      B.N R., ,N       ",
  " W:'  `:N .  G. N    `  :    .y.N:.          ,     B.N      :  R:   . .N",
  ":' '.   .    G:.N      .'   '   :::.  ,  c.N   :c.N    `        R`.N    ",
  "      :       `        `        :. ::. :     '  :        ,   , R.`:N    ",
  "  ,       G:.N              `y.N :. ::.c`N      c`.N   '        `      .",
  "     ..        G.:N :           .:   c.N:.    .              .          "
};

/***************************CONSTELLATIONS*******************************
  Lupus     Gigas      Pyx      Enigma   Centaurus    Terken    Raptus
   The       The       The       The       The         The       The
White Wolf  Giant     Pixie     Sphinx    Centaur      Drow     Raptor
*************************************************************************/
const char * sun_map[] =
{
  "\\'|'/",
  "- O -",
  "/.|.\\"
};
const char * moon_map[] =
{
  " @@@ ",
  "@@@@@",
  " @@@ "
};

void look_sky ( CHAR_DATA * ch )
{
  static char buf[MAX_STRING_LENGTH];
  static char buf2[4];
  int starpos, sunpos, moonpos, moonphase, i, linenum;
#ifdef DEBUG
  Debug ("look_sky");
#endif

  send_to_char("You gaze up towards the heavens and see:\n\r",ch);

  sunpos  = (MAP_WIDTH * (24 - time_info.hour) / 24);
  moonpos = (sunpos + time_info.day * MAP_WIDTH / NUM_DAYS) % MAP_WIDTH;
  if ((moonphase = ((((MAP_WIDTH + moonpos - sunpos ) % MAP_WIDTH ) +
                     (MAP_WIDTH/16)) * 8 ) / MAP_WIDTH)
      > 4) moonphase -= 8;
  starpos = (sunpos + MAP_WIDTH * time_info.month / NUM_MONTHS) % MAP_WIDTH;
  /* The left end of the star_map will be straight overhead at midnight during
     month 0 */

  for ( linenum = 0; linenum < MAP_HEIGHT; linenum++ )
  {
    if ((time_info.hour >= 6 && time_info.hour <= 18) &&
        (linenum < 3 || linenum >= 6))
      continue;
    mprintf(sizeof(buf), buf,"{W|{x");
    for ( i = MAP_WIDTH/4; i <= 3*MAP_WIDTH/4; i++)
    {
      /* plot moon on top of anything else...unless new moon & no eclipse */
      if ((time_info.hour >= 6 && time_info.hour <= 18)  /* daytime? */
          && (moonpos >= MAP_WIDTH/4 - 2) && (moonpos <= 3*MAP_WIDTH/4 + 2) /* in sky? */
          && ( i >= moonpos - 2 ) && (i <= moonpos + 2) /* is this pixel near moon? */
          && ((sunpos == moonpos && time_info.hour == 12) || moonphase != 0  ) /*no eclipse*/
          && (moon_map[linenum-3][i+2-moonpos] == '@'))
      {
        if ((moonphase < 0 && i - 2 - moonpos >= moonphase) ||
            (moonphase > 0 && i + 2 - moonpos <= moonphase))
          strcat(buf,"{W@");
        else
          strcat(buf," ");
      }
      else
        if ((linenum >= 3) && (linenum < 6) && /* nighttime */
            (moonpos >= MAP_WIDTH/4 - 2) && (moonpos <= 3*MAP_WIDTH/4 + 2) /* in sky? */
            && ( i >= moonpos - 2 ) && (i <= moonpos + 2) /* is this pixel near moon? */
            && (moon_map[linenum-3][i+2-moonpos] == '@'))
        {
          if ((moonphase < 0 && i - 2 - moonpos >= moonphase) ||
              (moonphase > 0 && i + 2 - moonpos <= moonphase))
            strcat(buf,"{W@");
          else
            strcat(buf," ");
        }
        else /* plot sun or stars */
        {
          if (time_info.hour>=6 && time_info.hour<=18) /* daytime */
          {
            if ( i >= sunpos - 2 && i <= sunpos + 2 )
            {
              mprintf(sizeof(buf2), buf2,"{y%c",sun_map[linenum-3][i+2-sunpos]);
              strcat(buf,buf2);
            }
            else
              strcat(buf," ");
          }
          else
          {
            switch (star_map[linenum][(MAP_WIDTH + i - starpos)%MAP_WIDTH])
            {
              default     :
                strcat(buf," ");
                break;
              case '.'    :
                strcat(buf,".");
                break;
              case ','    :
                strcat(buf,",");
                break;
              case ':'    :
                strcat(buf,":");
                break;
              case '`'    :
                strcat(buf,"`");
                break;
              case 'R'    :
                strcat(buf,"{R ");
                break;
              case 'G'    :
                strcat(buf,"{G ");
                break;
              case 'B'    :
                strcat(buf,"{B ");
                break;
              case 'W'    :
                strcat(buf,"{W ");
                break;
              case 'M'    :
                strcat(buf,"{M ");
                break;
              case 'N'    :
                strcat(buf,"{x ");
                break;
              case 'y'    :
                strcat(buf,"{y ");
                break;
              case 'c'    :
                strcat(buf,"{c ");
                break;
            }
          }
        }
    }
    strcat(buf,"{W|{x\n\r");
    send_to_char(buf,ch);
  }
}
void look_checkers( CHAR_DATA *ch, OBJ_DATA *obj )
{
  int i, j, k, a, b, c, d;
  char buf[MAX_STRING_LENGTH];
  strcpy(buf,"\0");
  send_to_char( "The checkers board is set up as follows: \n\r", ch );

  send_to_char( "    1   2   3   4   5   6   7   8\n\r", ch );
  send_to_char( "  ---------------------------------\n\r", ch );

  j = 1;
  k = 1;
  for ( i=1;i<=4;i++ )
  {
    a = ( IS_SET( obj->value[0], j ) ) ? 1 : 0;
    a += ( IS_SET( obj->value[1], j ) ) ? 2 : 0;
    a += ( IS_SET( obj->value[2], j ) ) ? 4 : 0;
    j *= 2;
    b = ( IS_SET( obj->value[0], j ) ) ? 1 : 0;
    b += ( IS_SET( obj->value[1], j ) ) ? 2 : 0;
    b += ( IS_SET( obj->value[2], j ) ) ? 4 : 0;
    j *= 2;
    c = ( IS_SET( obj->value[0], j ) ) ? 1 : 0;
    c += ( IS_SET( obj->value[1], j ) ) ? 2 : 0;
    c += ( IS_SET( obj->value[2], j ) ) ? 4 : 0;
    j *= 2;
    d = ( IS_SET( obj->value[0], j ) ) ? 1 : 0;
    d += ( IS_SET( obj->value[1], j ) ) ? 2 : 0;
    d += ( IS_SET( obj->value[2], j ) ) ? 4 : 0;
    j *= 2;

    mprintf( sizeof(buf), buf, "%d | %s |   | %s |   | %s |   | %s |   |\n\r",
             k,
             ( a >= 4 ) ? "{BB{w" :
             ( a >= 3 ) ? "{Bb{w" :
             ( a >= 2 ) ? "{WW{w" :
             ( a >= 1 ) ? "{Ww{w" :
             ( a >= 0 ) ? " " : "1",
             ( b >= 4 ) ? "{BB{w" :
             ( b >= 3 ) ? "{Bb{w" :
             ( b >= 2 ) ? "{WW{w" :
             ( b >= 1 ) ? "{Ww{w" :
             ( b >= 0 ) ? " " : "1",
             ( c >= 4 ) ? "{BB{w" :
             ( c >= 3 ) ? "{Bb{w" :
             ( c >= 2 ) ? "{WW{w" :
             ( c >= 1 ) ? "{Ww{w" :
             ( c >= 0 ) ? " " : "1",
             ( d >= 4 ) ? "{BB{w" :
             ( d >= 3 ) ? "{Bb{w" :
             ( d >= 2 ) ? "{WW{w" :
             ( d >= 1 ) ? "{Ww{w" :
             ( d >= 0 ) ? " " : "1" );

    send_to_char( buf, ch );
    send_to_char( "  ---------------------------------\n\r", ch );
    k++;

    a = ( IS_SET( obj->value[0], j ) ) ? 1 : 0;
    a += ( IS_SET( obj->value[1], j ) ) ? 2 : 0;
    a += ( IS_SET( obj->value[2], j ) ) ? 4 : 0;
    j *= 2;
    b = ( IS_SET( obj->value[0], j ) ) ? 1 : 0;
    b += ( IS_SET( obj->value[1], j ) ) ? 2 : 0;
    b += ( IS_SET( obj->value[2], j ) ) ? 4 : 0;
    j *= 2;
    c = ( IS_SET( obj->value[0], j ) ) ? 1 : 0;
    c += ( IS_SET( obj->value[1], j ) ) ? 2 : 0;
    c += ( IS_SET( obj->value[2], j ) ) ? 4 : 0;
    j *= 2;
    d = ( IS_SET( obj->value[0], j ) ) ? 1 : 0;
    d += ( IS_SET( obj->value[1], j ) ) ? 2 : 0;
    d += ( IS_SET( obj->value[2], j ) ) ? 4 : 0;
    j *= 2;

    mprintf( sizeof(buf), buf, "%d |   | %s |   | %s |   | %s |   | %s |\n\r",
             k,
             ( a >= 4 ) ? "{BB{w" :
             ( a >= 3 ) ? "{Bb{w" :
             ( a >= 2 ) ? "{WW{w" :
             ( a >= 1 ) ? "{Ww{w" :
             ( a >= 0 ) ? " " : "1",
             ( b >= 4 ) ? "{BB{w" :
             ( b >= 3 ) ? "{Bb{w" :
             ( b >= 2 ) ? "{WW{w" :
             ( b >= 1 ) ? "{Ww{w" :
             ( b >= 0 ) ? " " : "1",
             ( c >= 4 ) ? "{BB{w" :
             ( c >= 3 ) ? "{Bb{w" :
             ( c >= 2 ) ? "{WW{w" :
             ( c >= 1 ) ? "{Ww{w" :
             ( c >= 0 ) ? " " : "1",
             ( d >= 4 ) ? "{BB{w" :
             ( d >= 3 ) ? "{Bb{w" :
             ( d >= 2 ) ? "{WW{w" :
             ( d >= 1 ) ? "{Ww{w" :
             ( d >= 0 ) ? " " : "1" );

    send_to_char( buf, ch );
    if ( i < 4 )
      send_to_char( "  ---------------------------------\n\r", ch );
    else
      send_to_char( "  ---------------------------------\n\r", ch );
    k++;
  }
}


void do_newcount( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA  *d;
  char    arg[MAX_INPUT_LENGTH];
  int     count = 0;
  int      i;

  /* Update the count. */
  count_update();

  one_argument( argument, arg );

  if ( IS_IMMORTAL( ch ) && arg[0] != '\0'
       &&   !str_prefix( arg, "diagnostic" ) )
  {
    int hour = ( current_time / 3600 ) % 24;
    int j;

    printf_to_char( ch, "Max on: %d (last 24hrs), %d (ever)\n\r\n\r",
                    countMaxDay, countMax );

    send_to_char( "24hr player count array:\n\r", ch );
    for ( j = hour + 1, i = 0; i < 24; i++, j++ )
    {
      printf_to_char( ch, "%s%4d%s",
                      countArr[j % 24] == countMaxDay ? "{W" : "{x",
                      countArr[j % 24],
                      j % 24 == hour ? "{R*{x" : "{x " );
      if ( ( i + 1 ) % 6 == 0 )
        send_to_char( "\n\r", ch );
    }
    printf_to_char( ch,
                    "{R*{x denotes the current count hour (%d)\n\r",
                    countHour );

    send_to_char( "\n\rMost players on, by day of the week\n\r"
                  "\n\r-----------------------------------\n\r"
                  " Sun  Mon  Tue  Wed  Thu  Fri  Sat\n\r", ch );
    for ( i = 0; i < 7; i++ )
      printf_to_char( ch, "%s%4d ",
                      countMaxDoW[i] == countMax ? "{W" : "{x", countMaxDoW[i] );
    send_to_char( "{x\n\r", ch );
    return;
  }

  /* Count number of players the character can see. */
  for ( d = descriptor_list; d != NULL; d = d->next )
    if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
      count++;

  if ( countMax == count )
  {
    printf_to_char( ch,
                    "There %s %d player%s on, the most on ever.\n\r",
                    count == 1 ? "is" : "are", count, count == 1 ? "" : "s" );
  }
  else if ( countMaxDay == count )
  {
    printf_to_char( ch,
                    "There %s %d player%s on, the most in the past 24 hours.\n\r"
                    "The most on ever was %d.\n\r",
                    count == 1 ? "is" : "are", count, count == 1 ? "" : "s",
                    countMax );
  }
  else if ( countMaxDay == countMax )
  {
    printf_to_char( ch,
                    "There %s %d player%s on.\n\r"
                    "The most on ever and in the past 24 hours was %d.\n\r",
                    count == 1 ? "is" : "are", count, count == 1 ? "" : "s",
                    countMax );
  }
  else
  {
    printf_to_char( ch,
                    "There %s %d player%s on.  The most in the past 24 hours was %d.\n\r"
                    "The most on ever was %d.\n\r",
                    count == 1 ? "is" : "are", count, count == 1 ? "" : "s",
                    countMaxDay, countMax );
  }

  send_to_char( "\n\r{cMost players on, by day of the week\n\r"
                "{y-----------------------------------\n\r"
                " {cSun  Mon  Tue  Wed  Thu  Fri  Sat{x\n\r", ch );
  for ( i = 0; i < 7; i++ )
    printf_to_char( ch, "%s%4d ",
                    countMaxDoW[i] == countMax ? "{W" : "{x", countMaxDoW[i] );
  send_to_char( "{x\n\r", ch );
  save_sys_data();
}

void do_telnet_ga( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC( ch ) )
    return;

  auto_toggle( ch, argument, &ch->plr2, PLR2_TELNET_GA,
               "Telnet Go-Ahead protocol will be used.\n\r",
               "Telnet Go-Ahead protocol will no longer be used.\n\r",
               "Telnet Go-Ahead protocol remains %s.\n\r" );
}

void do_spell_maintain( CHAR_DATA *ch, char *argument )
{
  char arg[MIL];
  int sn,i;
  bool spellfound = FALSE;

  if ( IS_NPC( ch ) )
    return;

  if ( IS_GHOST( ch ) )
  {
    send_to_char("Casting spells or maintaining a spell list is useless as you are still {rDEAD{x.\n\r", ch);
    return;
  }

  argument = one_argument(argument,arg);

  if (arg[0] == '\0')
  {
    if ( ch->position < POS_STANDING
         &&   ch->position != POS_FIGHTING )
    {
      send_to_char("You must stand up to do that.{x\n\r",ch);
      return;
    }

    send_to_char("Starting Spellup...{x\n\r",ch);

    for (i = 0 ; i < MAX_MAINTAINED; i++)
    {
      if ( ( ch->pcdata->maintained[i] > 0 )
           &&    !has_affect( ch, ch->pcdata->maintained[i] ) )
      {
        if (skill_table[ch->pcdata->maintained[i]].spell_fun == spell_null)
        {
          printf_to_char(ch,"Using   {c%s{x.\n\r", skill_table[ch->pcdata->maintained[i]].name);
          strcat(ch->desc->inbuf, skill_table[ch->pcdata->maintained[i]].name);
          strcat(ch->desc->inbuf, "\n");
        }
        else
        {
          printf_to_char(ch,"Casting {c%s{x.\n\r",skill_table[ch->pcdata->maintained[i]].name);
          strcat(ch->desc->inbuf, "cast '");
          strcat(ch->desc->inbuf, skill_table[ch->pcdata->maintained[i]].name);
          strcat(ch->desc->inbuf, "'\n");
        }
      }
    }

    send_to_char("{xFinished Spellup.\n\r",ch);
    return;
  }

  if (!str_cmp(arg,"list"))
  {
    for (i=0; i < MAX_MAINTAINED; i++)
      if (ch->pcdata->maintained[i] > 0 )
      {
        printf_to_char(ch,"{[{c%-20s{w] {wis maintained.{x\n\r",
                       skill_table[ch->pcdata->maintained[i]].name);
        spellfound = TRUE;
      }

    if (spellfound == FALSE)
      send_to_char( "No spells in the list.\n\r", ch );

    return;
  }

  if (!str_cmp(arg,"empty"))
  {
    for (i=0; i < MAX_MAINTAINED; i++)
      ch->pcdata->maintained[i] = -1;
    send_to_char("{xSpellup list emptied.\n\r", ch);
    return;
  }

  if ((sn = skill_lookup(arg)) <= 0)
  {
    send_to_char("{xWhat spell?\n\r", ch);
    return;
  }

  for (i = 0; i < MAX_MAINTAINED; i++)
  {
    if (ch->pcdata->maintained[i] == sn)
    {
      printf_to_char(ch,"You remove {c%s{x from your list.{x\n\r",
                     skill_table[sn].name);
      ch->pcdata->maintained[i] = -1;
      return;
    }
  }

  if (get_skill(ch, sn) <= 0)
  {
    send_to_char("{xWhat spell?\n\r", ch);
    return;
  }

  for (i = 0; i < MAX_MAINTAINED; i++)
  {
    if (ch->pcdata->maintained[i] <= 0)
    {
      printf_to_char(ch,"You begin to maintain {c%s{x.\n\r",
                     skill_table[sn].name);
      ch->pcdata->maintained[i] = sn;
      return;
    }
  }

  send_to_char("Sorry, you may not memorize any more at this time.\n\r",ch);
  return;
}

void do_ospellup( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim = NULL;
  char arg[MIL],arg1[MIL];
  int sn,i;
  bool spellfound = FALSE;

  if ( IS_NPC( ch ) )
    return;

  if ( IS_GHOST( ch ) )
  {
    send_to_char("Casting spells or maintaining a spell list is useless as you are still {rDEAD{x.\n\r", ch);
    return;
  }

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg1 );

  if ( IS_NULLSTR( arg ) )
  {
    send_to_char( "Syntax: ospellup <name>\n\r", ch );
    send_to_char( "        ospellup edit <spell to add or remove>\n\r", ch );
    send_to_char( "        ospellup list\n\r", ch );
    return;
  }

  if (!str_cmp(arg,"list"))
  {
    for (i=0; i < MAX_MAINTAINED; i++)
    {
      if (ch->pcdata->maintained_other[i] > 0 )
      {
        printf_to_char(ch,"{[{c%-20s{w] {wis maintained.{x\n\r",
                       skill_table[ch->pcdata->maintained_other[i]].name);
        spellfound = TRUE;
      }
    }

    if (spellfound == FALSE)
      send_to_char( "No spells in the list.\n\r", ch );

    return;
  }

  if (!str_cmp(arg,"empty"))
  {
    for (i=0; i < MAX_MAINTAINED; i++)
      ch->pcdata->maintained_other[i] = -1;
    send_to_char("{xOSpellup list emptied.\n\r", ch);
    return;
  }

  if ( !str_cmp( arg, "edit" ) )
  {
    if ((sn = skill_lookup(arg1)) <= 0)
    {
      send_to_char("{xWhat spell?\n\r", ch);
      return;
    }

    for (i = 0; i < MAX_MAINTAINED; i++)
    {
      if (ch->pcdata->maintained_other[i] == sn)
      {
        printf_to_char(ch,"You remove {c%s{x from your list.{x\n\r",
                       skill_table[sn].name);
        ch->pcdata->maintained_other[i] = -1;
        return;
      }
    }

    if (get_skill(ch, sn) <= 0)
    {
      send_to_char("{xWhat spell?\n\r", ch);
      return;
    }

    if ( skill_table[sn].target == TAR_CHAR_SELF
         ||   skill_table[sn].target == TAR_CHAR_OFFENSIVE )
    {
      send_to_char( "You may not add this spell.\n\r", ch );
      return;
    }

    for (i = 0; i < MAX_MAINTAINED; i++)
    {
      if (ch->pcdata->maintained_other[i] <= 0)
      {
        printf_to_char(ch,"You begin to maintain {c%s{x.\n\r",
                       skill_table[sn].name);
        ch->pcdata->maintained_other[i] = sn;
        return;
      }
    }
    send_to_char("Sorry, you may not memorize any more at this time.\n\r",ch);
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
  {
    send_to_char( "You can't seem to find them here.\n\r", ch );
    return;
  }

  if ( victim )
  {
    if ( ch->position == POS_SLEEPING )
    {
      send_to_char("You cannot cast spells while asleep.{x\n\r",ch);
      return;
    }

    send_to_char("Starting Spellup...{x\n\r",ch);

    for (i = 0 ; i < MAX_MAINTAINED; i++)
    {
      if ( ( ch->pcdata->maintained_other[i] > 0 )
      &&    !has_affect( victim, ch->pcdata->maintained_other[i] ) )
      {
        if ( !str_cmp( skill_table[ch->pcdata->maintained_other[i]].name, "frenzy" ) )
				{
					if ( ( IS_GOOD(ch) && !IS_GOOD(victim) )
          ||   ( IS_EVIL(ch) && !IS_EVIL(victim) )
				  ||   ( IS_NEUTRAL(ch) && !IS_NEUTRAL(victim) ) )
            continue;
				}

        printf_to_char(ch,"Casting {c%s{x.\n\r",skill_table[ch->pcdata->maintained_other[i]].name);
        strcat(ch->desc->inbuf, "cast '" );
        strcat(ch->desc->inbuf, skill_table[ch->pcdata->maintained_other[i]].name );
        strcat(ch->desc->inbuf, "' " );
        strcat(ch->desc->inbuf, arg ); // arg used instead of victim->name in case of "ospellup 2.name"
        strcat(ch->desc->inbuf, "\n" );
      }
    }

    send_to_char("Finished Spellup.{x\n\r",ch);
    return;
  }

  return;
}

bool has_affect( CHAR_DATA *ch, sh_int sn )
{
  int flag,iWear;
  OBJ_DATA *obj;
  AFFECT_DATA *paf;

  if ( is_affected( ch, sn ) )
    return TRUE;

// Check for racial affects:
  for ( flag = 0; affect_flags[flag].name; flag++)
  {
    if ( !is_stat( affect_flags ) && IS_SET(race_table[ch->race].aff, affect_flags[flag].bit) )
    {
      if (!strcmp(skill_table[sn].name,affect_flags[flag].name))
        return TRUE;

      if ( !strcmp("flying",affect_flags[flag].name)
           &&   !strcmp("fly",skill_table[sn].name) )
        return TRUE;
    }
    else
    {
      if ( affect_flags[flag].bit == race_table[ch->race].aff )
      {
        if (!strcmp(skill_table[sn].name,affect_flags[flag].name))
          return TRUE;

        if ( !strcmp("dark_vision",affect_flags[flag].name)
             &&   !strcmp("darksight",skill_table[sn].name) )
          return TRUE;

        if ( !strcmp("infrared",affect_flags[flag].name)
             &&   !strcmp("infravision",skill_table[sn].name) )
          return TRUE;

        if ( !strcmp("flying",affect_flags[flag].name)
             &&   !strcmp("fly",skill_table[sn].name) )
          return TRUE;
      }
    }
  }

  for ( flag = 0; affect2_flags[flag].name; flag++)
  {
    if ( !is_stat( affect2_flags ) && IS_SET(race_table[ch->race].aff2, affect2_flags[flag].bit) )
    {
      if (!strcmp(skill_table[sn].name,affect2_flags[flag].name))
        return TRUE;

      if ( ( !strcmp("infrared",affect_flags[flag].name)
             ||     !strcmp("dark_vision",affect_flags[flag].name) )
           &&   ( !strcmp("infravision",skill_table[sn].name)
                  ||     !strcmp("darksight",skill_table[sn].name) ) )
        return TRUE;

      if ( !strcmp("flying",affect_flags[flag].name)
           &&   !strcmp("fly",skill_table[sn].name) )
        return TRUE;
    }
    else
    {
      if ( affect2_flags[flag].bit == race_table[ch->race].aff2 )
      {
        if (!strcmp(skill_table[sn].name,affect2_flags[flag].name))
          return TRUE;

        if ( ( !strcmp("infrared",affect_flags[flag].name)
               ||     !strcmp("dark_vision",affect_flags[flag].name) )
             &&   ( !strcmp("infravision",skill_table[sn].name)
                    ||     !strcmp("darksight",skill_table[sn].name) ) )
          return TRUE;

        if ( !strcmp("flying",affect_flags[flag].name)
             &&   !strcmp("fly",skill_table[sn].name) )
          return TRUE;
      }
    }
  }

// Check for object affects:
  for ( iWear = 0; (iWear <= MAX_WEAR) ; iWear++ )
  {
    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
      continue;

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
      if (paf->bitvector)
      {
        if ( ( paf->where == TO_AFFECTS )
             &&    !str_cmp( skill_table[sn].name, affect_bit_name( paf->bitvector ) ) )
          return TRUE;

        if ( ( paf->where == TO_AFFECTS2 )
             &&    !str_cmp( skill_table[sn].name, affect2_bit_name( paf->bitvector ) ) )
          return TRUE;

        // pass door goes by two different names
        if ( !str_cmp( "pass_door", affect_bit_name( paf->bitvector ) )
             &&   !str_cmp( "pass door", skill_table[sn].name )  )
          return TRUE;

      } // if bit vector
    } // Object Aff loop

    if (!obj->enchanted)
    {
      for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
      {
        if (paf->bitvector)
        {
          if ( ( paf->where == TO_AFFECTS )
               &&    !str_cmp( skill_table[sn].name, affect_bit_name( paf->bitvector ) ) )
            return TRUE;

          if ( ( paf->where == TO_AFFECTS2 )
               &&    !str_cmp( skill_table[sn].name, affect2_bit_name( paf->bitvector ) ) )
            return TRUE;

          // pass door goes by two different names
          if ( !str_cmp( "pass_door", affect_bit_name( paf->bitvector ) )
               &&   !str_cmp( "pass door", skill_table[sn].name )  )
            return TRUE;

        } // if bit vector (!enchanted)
      } // Object Aff loop (!enchanted)
    } // if not enchanted
  } // worn-objects loop

// Check for spell affects
  if ( !str_cmp( skill_table[sn].name, "warriorshield" ) )
  {
    if ( IS_SET(ch->spell_aff, SAFF_INVUN)
         ||   IS_SET(ch->spell_aff, SAFF_WARRIORSHIELD)
         ||   IS_SET(ch->spell_aff, SAFF_IRONWILL)
         ||   IS_SET(ch->spell_aff, SAFF_WARCRY_GUARDING )
         ||   IS_AFFECTED(ch, AFF_SANCTUARY)
         ||   IS_AFFECTED(ch, AFF_AQUA_ALBEDO)
         ||   IS_AFFECTED2(ch, AFF2_INVUN)
         ||   IS_AFFECTED2(ch, AFF2_FADE_OUT)
         ||   IS_AFFECTED2(ch, AFF2_SHROUD)
         ||   IS_AFFECTED2(ch, AFF2_RADIANT)
         ||   IS_AFFECTED2(ch, AFF2_NIRVANA) )
      return TRUE;
    else
      return FALSE; // we know the spell is "warriorshield", no more checking needed
  }
  else if ( !str_cmp( skill_table[sn].name, "mageshield" ) )
  {
    if ( IS_SET(ch->spell_aff, SAFF_INVUN)
         ||   IS_SET(ch->spell_aff, SAFF_MAGESHIELD)
         ||   IS_SET(ch->spell_aff, SAFF_IRONWILL)
         ||   IS_SET(ch->spell_aff, SAFF_WARCRY_GUARDING )
         ||   IS_AFFECTED(ch, AFF_SANCTUARY)
         ||   IS_AFFECTED(ch, AFF_AQUA_ALBEDO)
         ||   IS_AFFECTED2(ch, AFF2_INVUN)
         ||   IS_AFFECTED2(ch, AFF2_FADE_OUT)
         ||   IS_AFFECTED2(ch, AFF2_SHROUD)
         ||   IS_AFFECTED2(ch, AFF2_RADIANT)
         ||   IS_AFFECTED2(ch, AFF2_NIRVANA) )
      return TRUE;
    else
      return FALSE; // we know the spell is "mageshield", no more checking needed
  }
  else
  {
    if ( ( !str_cmp( skill_table[sn].name, "ironwill" )
           ||   !str_cmp( skill_table[sn].name, "radiance" )
           ||   !str_cmp( skill_table[sn].name, "sanctuary" )
           ||   !str_cmp( skill_table[sn].name, "aqua albedo" )
           ||   !str_cmp( skill_table[sn].name, "fade out" )
           ||   !str_cmp( skill_table[sn].name, "malevolent shroud" )
           ||   !str_cmp( skill_table[sn].name, "nirvana" )
           ||   !str_cmp( skill_table[sn].name, "warriorshield" )
           ||   !str_cmp( skill_table[sn].name, "warcry guarding" )
           ||   !str_cmp( skill_table[sn].name, "mageshield" )
           ||   !str_cmp( skill_table[sn].name, "globe of invulnerability" ) )
         &&   is_sanc_spelled( ch ) )
      return TRUE;
  }

// Can't frenzy while calmed: what about berserk?
  if ( !str_cmp( skill_table[sn].name, "frenzy" )
       &&   is_affected( ch, skill_lookup( "calm" ) ) )
    return TRUE;

  if ( !str_cmp( skill_table[sn].name, "haste" )
       &&   (  IS_AFFECTED( ch, AFF_HASTE )
               ||      IS_SET( ch->off_flags, OFF_FAST ) ) )
    return TRUE;

  if ( !str_cmp( skill_table[sn].name, "fly" )
       &&   IS_AFFECTED( ch, AFF_FLYING ) )
    return TRUE;

  if ( !str_cmp( skill_table[sn].name, "darksight" )
       &&   IS_AFFECTED( ch, AFF_DARK_VISION ) )
    return TRUE;

  if ( !str_cmp( skill_table[sn].name, "infravision" )
       &&   IS_AFFECTED( ch, AFF_INFRARED ) )
    return TRUE;

  if ( ( !str_cmp( skill_table[sn].name, "bless" )
         && ( is_affected(ch, skill_lookup("mass bless")) ) )
       || ( !str_cmp( skill_table[sn].name, "mass bless" )
            && ( is_affected(ch, skill_lookup("bless")) ) ) )
    return TRUE;

  if ( ( !str_cmp( skill_table[sn].name, "invisibility" )
         && ( is_affected(ch, skill_lookup("mass invis")) ) )
       || ( !str_cmp( skill_table[sn].name, "mass invis" )
            && ( is_affected(ch, skill_lookup("invisibility")) ) ) )
    return TRUE;

  if ( ( !str_cmp( skill_table[sn].name, "flame aura" )
         ||     !str_cmp( skill_table[sn].name, "frost aura" )
         ||     !str_cmp( skill_table[sn].name, "electric aura" )
         ||     !str_cmp( skill_table[sn].name, "corrosive aura" )
         ||     !str_cmp( skill_table[sn].name, "holy aura" )
         ||     !str_cmp( skill_table[sn].name, "dark aura" )
         ||     !str_cmp( skill_table[sn].name, "arcane aura" )
         ||     !str_cmp( skill_table[sn].name, "poison aura" ) )
       &&   ( is_affected( ch, skill_lookup("flame aura") )
              ||     is_affected( ch, skill_lookup("frost aura") )
              ||     is_affected( ch, skill_lookup("alectric aura") )
              ||     is_affected( ch, skill_lookup("corrosive aura") )
              ||     is_affected( ch, skill_lookup("holy aura") )
              ||     is_affected( ch, skill_lookup("dark aura") )
              ||     is_affected( ch, skill_lookup("arcane aura") )
              ||     is_affected( ch, skill_lookup("poison aura") ) ) )
    return TRUE;

  if ( ( !str_cmp( skill_table[sn].name, "bear spirit" )
         ||     !str_cmp( skill_table[sn].name, "eagle spirit" )
         ||     !str_cmp( skill_table[sn].name, "dragon spirit" )
         ||     !str_cmp( skill_table[sn].name, "tiger spirit" ) )
       &&   ( is_affected( ch, skill_lookup("bear spirit") )
              ||     is_affected( ch, skill_lookup("eagle spirit") )
              ||     is_affected( ch, skill_lookup("dragon spirit") )
              ||     is_affected( ch, skill_lookup("tiger spirit") ) ) )
    return TRUE;

  if ( ( !str_cmp( skill_table[sn].name, "fire element" )
         ||     !str_cmp( skill_table[sn].name, "water element" )
         ||     !str_cmp( skill_table[sn].name, "air element" )
         ||     !str_cmp( skill_table[sn].name, "earth element" ) )
       &&   ( is_affected( ch, skill_lookup("fire element") )
              ||     is_affected( ch, skill_lookup("water element") )
              ||     is_affected( ch, skill_lookup("air element") )
              ||     is_affected( ch, skill_lookup("earth element") ) ) )
    return TRUE;

  if (   is_affected( ch, skill_lookup("clairvoyance") )
         &&   ( !str_cmp( skill_table[sn].name, "farsight" )
                ||     !str_cmp( skill_table[sn].name, "detect invis" )
                ||     !str_cmp( skill_table[sn].name, "detect hidden" ) ) )
    return TRUE;

  if (   !str_cmp( skill_table[sn].name, "clairvoyance" )
         &&     is_affected( ch, skill_lookup("farsight") )
         &&     is_affected( ch, skill_lookup("detect invis") )
         &&     is_affected( ch, skill_lookup("detect hidden") ) )
    return TRUE;

  if ( IS_CLASS_OCCULTIST( ch ) ) // Occultists can use both holy and good protections
  {
    if ( ( !str_cmp( skill_table[sn].name, "protection evil" )
           ||     !str_cmp( skill_table[sn].name, "protection negative" )
           ||     !str_cmp( skill_table[sn].name, "protection neutral" ) )
         &&   ( (IS_AFFECTED(ch, AFF_PROTECT_GOOD))
                ||     (IS_AFFECTED(ch, AFF_PROTECT_EVIL))
                ||     (IS_SAFFECTED(ch, SAFF_PROTECT_HOLY))
                ||     (IS_SAFFECTED(ch, SAFF_PROTECT_NEGATIVE))
                ||     (IS_AFFECTED2(ch, AFF2_PROTECT_NEUTRAL)) ) )
      return TRUE;

    if ( ( !str_cmp( skill_table[sn].name, "protection good" ) )
         &&   ( (IS_AFFECTED(ch, AFF_PROTECT_GOOD))
                ||     (IS_AFFECTED(ch, AFF_PROTECT_EVIL))
                ||     (IS_SAFFECTED(ch, SAFF_PROTECT_NEGATIVE))
                ||     (IS_AFFECTED2(ch, AFF2_PROTECT_NEUTRAL)) ) )
      return TRUE;

    if ( ( !str_cmp( skill_table[sn].name, "protection holy" ) )
         &&   ( (IS_AFFECTED(ch, AFF_PROTECT_EVIL))
                ||     (IS_SAFFECTED(ch, SAFF_PROTECT_HOLY))
                ||     (IS_SAFFECTED(ch, SAFF_PROTECT_NEGATIVE))
                ||     (IS_AFFECTED2(ch, AFF2_PROTECT_NEUTRAL)) ) )
      return TRUE;
  }
  else
  {
    if ( ( !str_cmp( skill_table[sn].name, "protection good" )
           ||     !str_cmp( skill_table[sn].name, "protection evil" )
           ||     !str_cmp( skill_table[sn].name, "protection holy" )
           ||     !str_cmp( skill_table[sn].name, "protection negative" )
           ||     !str_cmp( skill_table[sn].name, "protection neutral" ) )
         &&   ( (IS_AFFECTED(ch, AFF_PROTECT_GOOD))
                ||     (IS_AFFECTED(ch, AFF_PROTECT_EVIL))
                ||     (IS_SAFFECTED(ch, SAFF_PROTECT_HOLY))
                ||     (IS_SAFFECTED(ch, SAFF_PROTECT_NEGATIVE))
                ||     (IS_AFFECTED2(ch, AFF2_PROTECT_NEUTRAL)) ) )
      return TRUE;
  }

  if (!str_cmp( skill_table[sn].name, "vaccine" )
      &&   IS_SET( ch->res_flags, RES_DISEASE ) )
    return TRUE;

  return FALSE; // nothing found!
}

void do_showhints(CHAR_DATA *ch, char *argument)
{
  send_to_char("{Y-----------------------------------------------------------------------------{x\n\r",ch);
  send_to_char("                               {RNEWBIE HINTS{x\n\r",ch);
  send_to_char("{Y-----------------------------------------------------------------------------{x\n\r",ch);
  show_file_to_char(ch,HINTS_FILE,"");
  send_to_char("{Y-----------------------------------------------------------------------------{x\n\r",ch);
}


void do_cmdhist(CHAR_DATA *ch, char *argument)
{
  int cmd_num;
  char buf[MAX_STRING_LENGTH];


  for (cmd_num = MAX_COMMAND_HISTORY - 1; cmd_num >= 0; cmd_num--)
  {
    if (ch->cmd_hist[cmd_num].text[0] != '\0')
    {
      mprintf(sizeof(buf), buf, "[%d]: %s", cmd_num, ch->cmd_hist[cmd_num].text);
      act(buf, ch, NULL, NULL, TO_CHAR);
    }
  }

}

void do_sacred(CHAR_DATA *ch, char *argument)
{
  do_function( ch, &do_help, "sacred" );
  return;
}

char *get_align_str( int align )
{
  if         ( align >  900 ) return "{Yangelic{x  ";
  else    if ( align >  700 ) return "{Ysaint{yly{x";
  else    if ( align >  350 ) return "{yg{Wo{yod{x     ";
  else    if ( align >  100 ) return "{ykind{x   ";
  else    if ( align > -100 ) return "{wneutral{x  ";
  else    if ( align > -350 ) return "{rmean{x     ";
  else    if ( align > -700 ) return "{rev{Di{rl{x   ";
  else    if ( align > -900 ) return "{Rdemon{ric{x";
  else                        return "{Rsatanic{x  ";
  return  NULL;
}

char *get_ac_str( int ac )
{
  if ( ac >   100 ) return "Hoplessly vulnerable";
  else if ( ac >=   80 ) return "Defenseless";
  else if ( ac >=   60 ) return "Almost defenseless";
  else if ( ac >=   40 ) return "Barely armored";
  else if ( ac >=   20 ) return "Slightly armored";
  else if ( ac >=    0 ) return "Somewhat armored";
  else if ( ac >=  -20 ) return "Armored";
  else if ( ac >=  -40 ) return "Fairly armored";
  else if ( ac >=  -60 ) return "Fairly Well-armored";
  else if ( ac >= -100 ) return "Well-armored";
  else if ( ac >= -150 ) return "Very well-armored";
  else if ( ac >= -200 ) return "Heavily armored";
  else if ( ac >= -250 ) return "Fairly heavily armored";
  else if ( ac >= -300 ) return "Very heavily armored";
  else if ( ac >= -350 ) return "Superbly armored";
  else if ( ac >= -400 ) return "Supremely armored";
  else if ( ac >= -500 ) return "Almost invulnerable";
  return "Divinely armored";
}

void do_statistics( CHAR_DATA *ch, char *argument )
{
  BUFFER *buffer;
  char buf1[MAX_STRING_LENGTH], buf[MSL];
  int frag_stat = 0;
  int v1;
  int v2;

  if ((argument[0] != '\0')
      && !str_prefix( argument, "clan" ) )
  {
    do_function( ch, &do_score_clan, argument );
    return;
  }

  buffer = new_buf();

  if (!IS_SET( ch->comm_flags, COMM_STATS_SHOW) )
  {
    bstrcat( buffer,
             "{g*{D-----------------------------------------------------------------------{g*{x\n\r");
  }

  bstrcat( buffer,
           "  {wStat    Curr    Bonus{x \n\r" );

  /*
   *    *Strength and Hunger
   */

  frag_stat = ch->perm_stat[STAT_STR] - get_max_train(ch, STAT_STR);
  mprintf(sizeof(buf),buf,"(+%d)", frag_stat );

  bprintf( buffer,
           "  {wStr{x:    %s%2d %4s %s%2d   {x",
           ch->perm_stat[STAT_STR] < get_max_train( ch, STAT_STR ) ? "{r" : "{R",
           ch->perm_stat[STAT_STR],
           ch->perm_stat[STAT_STR] > get_max_train(ch, STAT_STR) ? buf : "    ",
           ch->perm_stat[STAT_STR] <= get_curr_stat( ch, STAT_STR ) ? "{G" : "{B",
           get_curr_stat( ch, STAT_STR ) );

  if ( !IS_NPC( ch ) )
  {
    v1 = ch->pcdata->condition[COND_HUNGER];
    v2 = URANGE( 0, (v1+2) / 5, 10 );

    if ( v1 == -1 )
      strcpy( buf1, "{b[{c   none   {b]{x" );
    else if ( v1 >= 48 )
      strcpy( buf1, "{b[{c=========={b]{x" );
    else if ( v1 <= 0 )
      strcpy( buf1, "{b[{D----------{b]{x" );
    else
      sprintf( buf1, "{b[{c%.*s{C+{D%.*s{b]{x",
               v2, "=========", 9-v2, "---------" );


    bprintf( buffer, "                      {wHunger{x:{B %3d{x %s",
             v1,
             buf1 );
  }

  bstrcat( buffer, "{x\n\r" );

  /*
   *  *Dexterity and Thirst
   */
  frag_stat = ch->perm_stat[STAT_DEX] - get_max_train(ch, STAT_DEX);
  mprintf(sizeof(buf),buf,"(+%d)", frag_stat );

  bprintf( buffer,
           "  {wDex{x:    %s%2d %4s %s%2d   {x",
           ch->perm_stat[STAT_DEX] < get_max_train( ch, STAT_DEX ) ? "{r" : "{R",
           ch->perm_stat[STAT_DEX],
           ch->perm_stat[STAT_DEX] > get_max_train(ch, STAT_DEX) ? buf : "    ",
           ch->perm_stat[STAT_DEX] <= get_curr_stat( ch, STAT_DEX ) ? "{G" : "{B",
           get_curr_stat( ch, STAT_DEX ) );

  if ( !IS_NPC( ch ) )
  {
    v1 = ch->pcdata->condition[COND_THIRST];
    v2 = URANGE( 0, (v1 + 2) / 5, 10 );

    if ( v1 == -1 )
      strcpy( buf1, "{b[{c   none   {b]{x" );
    else if ( v1 >= 48 )
      strcpy( buf1, "{b[{c=========={b]{x" );
    else if ( v1 <= 0 )
      strcpy( buf1, "{b[{D----------{b]{x" );
    else
      sprintf( buf1, "{b[{c%.*s{C+{D%.*s{b]{x",
               v2, "=========", 9-v2, "---------" );

    bprintf( buffer, "                      {wThirst{x:{B %3d{x %s",
             v1,
             buf1 );
  }

  bstrcat( buffer, "{x\n\r" );

  /*
  *    *Intelligence and Full
  */
  frag_stat = ch->perm_stat[STAT_INT] - get_max_train(ch, STAT_INT);
  mprintf(sizeof(buf),buf,"(+%d)", frag_stat );

  bprintf( buffer,
           "  {wInt{x:    %s%2d %4s %s%2d   {x",
           ch->perm_stat[STAT_INT] < get_max_train( ch, STAT_INT ) ? "{r" : "{R",
           ch->perm_stat[STAT_INT],
           ch->perm_stat[STAT_INT] > get_max_train(ch, STAT_INT) ? buf : "    ",
           ch->perm_stat[STAT_INT] <= get_curr_stat( ch, STAT_INT ) ? "{G" : "{B",
           get_curr_stat( ch, STAT_INT ) );

  if ( !IS_NPC( ch ) )
  {
    v1 = ch->pcdata->condition[COND_FULL];
    v2 = URANGE( 0, v1/4, 10 );

    if ( v1 == -1 )
      strcpy( buf1, "{b[{c   none   {b]{x" );
    else if ( v1 >= 40 )
      strcpy( buf1, "{b[{c=========={b]{x" );
    else if ( v1 <= 0 )
      strcpy( buf1, "{b[{D----------{b]{x" );
    else
      sprintf( buf1, "{b[{c%.*s{C+{D%.*s{b]{x",
               v2, "=========", 9-v2, "---------" );

    bprintf( buffer, "                      {wFull{x: {B  %3d{x %s",
             v1,
             buf1 );
  }

  bstrcat( buffer, "{x\n\r" );

  /*
   *    *Wisdom and Drunk
   */
  frag_stat = ch->perm_stat[STAT_WIS] - get_max_train(ch, STAT_WIS);
  mprintf(sizeof(buf),buf,"(+%d)", frag_stat );

  bprintf( buffer,
           "  {wWis{x:    %s%2d %4s %s%2d   {x",
           ch->perm_stat[STAT_WIS] < get_max_train( ch, STAT_WIS ) ? "{r" : "{R",
           ch->perm_stat[STAT_WIS],
           ch->perm_stat[STAT_WIS] > get_max_train(ch, STAT_WIS) ? buf : "    ",
           ch->perm_stat[STAT_WIS] <= get_curr_stat( ch, STAT_WIS ) ? "{G" : "{B",
           get_curr_stat( ch, STAT_WIS ) );

  if ( !IS_NPC( ch ) )
  {
    v1 = ch->pcdata->condition[COND_DRUNK];
    v2 = URANGE( 0, v1-1, 10 );

    if ( v1 == -1 )
      strcpy( buf1, "{b[{c   none   {b]{x" );
    else if ( v1 >= 10 )
      strcpy( buf1, "{b[{c=========={b]{x" );
    else if ( v1 <= 0 )
      strcpy( buf1, "{b[{D----------{b]{x" );
    else
      sprintf( buf1, "{b[{c%.*s{C+{D%.*s{b]{x",
               v2, "=========", 9-v2, "---------" );

    bprintf( buffer, "                      {wDrunk{x:{B  %3d{x %s",
             v1,
             buf1 );
  }

  bstrcat( buffer, "{x\n\r" );

  /*
   *Constitution and Alignment
   */
  frag_stat = ch->perm_stat[STAT_CON] - get_max_train(ch, STAT_CON);
  mprintf(sizeof(buf),buf,"(+%d)", frag_stat );

  bprintf( buffer,
           "  {wCon{x:    %s%2d %4s %s%2d   {x",
           ch->perm_stat[STAT_CON] < get_max_train( ch, STAT_CON ) ? "{r" : "{R",
           ch->perm_stat[STAT_CON],
           ch->perm_stat[STAT_CON] > get_max_train(ch, STAT_CON) ? buf : "    ",
           ch->perm_stat[STAT_CON] <= get_curr_stat( ch, STAT_CON ) ? "{G" : "{B",
           get_curr_stat( ch, STAT_CON ) );

  if ( ch->alignment >  900 ) strcpy( buf1,     "{w({YAngelic{w){x" );
  else if ( ch->alignment >  700 ) strcpy( buf1, "{w({YSaint{yly{w){x" );
  else if ( ch->alignment >  350 ) strcpy( buf1, "{w({yG{Wo{yod{w){x" );
  else if ( ch->alignment >  100 ) strcpy( buf1, "{w({yKind{w){x" );
  else if ( ch->alignment > -100 ) strcpy( buf1, "{w(Neutral)" );
  else if ( ch->alignment > -350 ) strcpy( buf1, "{w({rMean{w){w" );
  else if ( ch->alignment > -700 ) strcpy( buf1, "{w({rEv{Di{rl{w){x" );
  else if ( ch->alignment > -900 ) strcpy( buf1, "{w({RDemon{ric{w){x" );
  else           strcpy( buf1, "{w({RSatanic{w){x" );

  if ( IS_NPC( ch ) || ch->level >= 5 )
    bprintf( buffer, "                      Align:   %s%+5d %s",
             IS_GOOD( ch ) ? "{Y" : IS_EVIL( ch ) ? "{R" : "{w",
             ch->alignment,
             buf1 );
  else
    bprintf( buffer, "                      Align:   %s%+5d %s      ",
             IS_GOOD( ch ) ? "{Y" : IS_EVIL( ch ) ? "{R" : "{w",
             ch->alignment,
             buf1 );

  bstrcat( buffer, "{x\n\r" );

  bstrcat( buffer,
           "{g*{D-----------------------------------------------------------------------{g*{x\n\r");

  v1 = GET_AC( ch, AC_PIERCE );

  if ( IS_NPC( ch ) || ch->level >= 25 )
    sprintf( buf1, "{C[{c%+5d{C]{x ", v1 );
  else
    buf1[0] = '\0';

  bprintf( buffer,
           "  {wPierce{x: %4s %-22s ",
           buf1,
           get_ac_str( v1 ) );

  v1 = GET_AC( ch, AC_BASH );

  if ( IS_NPC( ch ) || ch->level >= 25 )
    sprintf( buf1, "{C[{c%+5d{C]{x ", v1 );
  else
    buf1[0] = '\0';

  /*EXPERIENCE Total*/

  if ( !IS_NPC( ch ) )
  {
    bprintf( buffer, "        {wExperience{x: %d\n\r",
             ch->exp );
  }
  else
    bstrcat( buffer, "{x\n\r" );

  bprintf( buffer,
           "  {wBash{x  : %4s %-22s ",
           buf1,
           get_ac_str( v1 ) );

  v1 = GET_AC( ch, AC_SLASH );

  if ( IS_NPC( ch ) || ch->level >= 25 )
    sprintf( buf1, "{C[{c%+5d{C]{x ", v1 );
  else
    buf1[0] = '\0';

  /*EXPERIENCE TNL*/

  if ( !IS_NPC( ch ) && ch->level < LEVEL_HERO )
  {
    bprintf( buffer, "        {wNeeded{x: %d\n\r",
             TNL( exp_per_level( ch, ch->pcdata->points ), ch->level ) - ch->exp );
  }
  else
    bstrcat( buffer, "{x\n\r" );


  bprintf( buffer,
           "  {wSlash{x : %4s %-22s ",
           buf1,
           get_ac_str( v1 ) );

  bprintf( buffer, "        {wHitroll{w:{g %d{x\n\r",
           GET_HITROLL( ch ) );

  v1 = GET_AC( ch, AC_EXOTIC );

  if ( IS_NPC( ch ) || ch->level >= 25 )
    sprintf( buf1, "{C[{c%+5d{C]{x ", v1 );
  else
    buf1[0] = '\0';

  bprintf( buffer,
           "  {wMagic {x: %4s %-22s ",
           buf1,
           get_ac_str( v1 ) );

  bprintf( buffer, "        {wDamroll{w:{g %d{x\n\r",
           GET_DAMROLL( ch ) );

  bstrcat( buffer,
           "{g*{D-----------------------------------------------------------------------{g*{x\n\r");

  if (!IS_NPC(ch))
    bprintf(buffer,
            "  {wQuest Points:   {g%-9d                  {wAttempted:{g  %d{x\n\r",
            ch->questdata->curr_points, ch->questdata->attempt_num);

  if (!IS_NPC( ch ))
    bprintf(buffer,
            "  {wLifetime Total: {g%-9d{x                  {wComp/Quit:{g  %d/%d{x\n\r",
            ch->questdata->accum_points,
            ch->questdata->comp_num,
            ch->questdata->quit_num);

  bstrcat( buffer,
           "{g*{D-----------------------------------------------------------------------{g*{x\n\r");

  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );

}

void do_scorio( CHAR_DATA *ch, char *argument )
{
  BUFFER *buffer;
  char buf1[MSL];
  char buf2[MSL];
  char buf3[MSL], show_trust[MSL];
  int v1, v2;
  int ruby_percent;
  int sapphire_percent;
  int emerald_percent;
  int diamond_percent;

  buffer = new_buf();

  bstrcat( buffer,
           "\n\r{g*{D-----------------------------------------------------------------------{g*{x\n\r");

  if ( IS_NPC(ch) )
    sprintf( buf1, "%6s", "      ");
  else
  {
    strcpy( buf2,size_table[pc_race_table[ch->race].size].name );
    sprintf( buf1, "%-6s",
             capitalize( buf2 ) );
  }

  bprintf( buffer,
           "{D|{x Name:  %-15s   Race:   %-12s Size: %s            {D|{x\n\r",
           ch->name,
           race_table[ch->race].name,
           buf1);

  if ( get_trust(ch) != ch->level )
    sprintf( show_trust, "(%3d)", get_trust(ch) );
  else
    sprintf( show_trust, "%5s", "     " );

  if ( IS_NPC( ch ) )
    sprintf( buf1, "%8s", "mobile  ");
  else
    sprintf( buf1, "%-8s",
             class_table[ch->gameclass].name);

  if ( IS_NPC( ch ) )
    sprintf( buf2, "%9s", "Null     ");
  else if ( is_clan( ch ) )
    sprintf( buf2, "%-9s",
             ch->clan->symbol );
  else
    sprintf( buf2, "%9s", "None     ");

  if (is_clan( ch ))
  {
    bprintf( buffer, "{D|{c Level{w: %-3d %s         {cClass{x:  %-11s  {cClan{x: %-9s         {D|{x\n\r",
             ch->level, show_trust, buf1, buf2);
  }
  else
  {
    if (ch->alignment < -500)
      strcpy(buf2,"Evil");
    else if (ch->alignment > 500)
      strcpy(buf2,"Good");
    else
      strcpy(buf2,"Neutral");

    bprintf( buffer, "{D|{c Level{w: %-3d %s         {cClass{x:  %-11s  {cAlign{x: %-9s        {D|{x\n\r",
             ch->level, show_trust, buf1, buf2);
  }

  if ( IS_NPC( ch ) )
    sprintf( buf1, "%-7s", "N/A    ");
  else
    sprintf( buf1, "%-7s",
             ch->sex == 0 ? "Sexless" : ch->sex == 1 ? "Male" : "Female");

  if ( IS_NPC( ch )
       || !IS_IN_CLAN( ch ) )
    sprintf( buf2, "%-16s", "None            ");
  //else if ( ch->clan == 7 )
  //            sprintf( buf2, "%-20s", get_clan_rank_char(ch) );
  else
    sprintf( buf2, "%-16s", get_rank( ch ) );

  strncpy_color( buf3,
                 FIX_STR( buf2, "", "" ),
                 16, ' ', TRUE );

  if (ch->level >= LEVEL_CLAN)
  {
    bprintf( buffer,  "{D|{m Pos.{x:  %-16s  {mSex{x:    %-7s      {mRank{x: %s  {D|{x\n\r",
             capitalize(  position_table[ch->position].name ), buf1, buf3 );
  }
  else
  {
    bprintf( buffer,  "{D|{m Pos.{x:  %-16s  {mSex{x:    %-7s                              {D|{x\n\r",
             capitalize(  position_table[ch->position].name ), buf1);
  }


  if ( !IS_NPC( ch ) )
  {
    v1 = ( ch->played + (int) (current_time - ch->logon) ) / 3600;

    bprintf( buffer, "{D|{g Age{x:   %-4d              {gPlayed{x: %-5d (%2d Year%s, %3d Day%s, %2d Hour%s)%s%s%s {D|{x\n\r",
      get_age( ch ),
      v1,
      // Year(s)
      v1/24/365,     ( v1/24/365 == 1 ) ? "" : "s",
      // Day(s)
      (v1/24) % 365, ( (v1/24) % 365 == 1 ) ? "" : "s",
      // Hour(s)
      v1 % 24,       ( (v1 & 24) == 1 ) ? "" : "s",
      // add spaces to align right side
      ( v1/24/365 == 1 ) ? " " : "",
      ( v1/24  == 1 ) ? " " : "",
      ( v1 % 24 == 1 ) ? " " : "" );
  }

  bstrcat( buffer,
           "{g*{D------------------------------------{g*{D----------------------------------{g*{x\n\r");

  v1 = PERCENT( ch->hit, GET_HP(ch) );
  if ( v1 >= 100 ) strcpy( buf1, "{R[{r=========={R]{x" );
  else if ( v1 <=   0 ) strcpy( buf1, "{R[{D----------{R]{x" );
  else
    sprintf( buf1, "{R[{r%.*s{R+{D%.*s{R]{x",
             v1/10, "=========", 9-v1/10, "---------" );

  if ( ( v2 = GOLD_WEIGHT( ch->gold ) / 10 ) > 0 )
    sprintf( buf2, " {y({w%5d lb%s{y){x", v2, v2 == 1 ? " " : "s" );
  else
    sprintf( buf2, "%s", "            " );

  bprintf( buffer,
           "{D|{R H{rit{x:  %5d/%-5d %s     {D|{x {yG{Yo{yld{x:   %7d%s      {D|{x\n\r",
           ch->hit,
           GET_HP( ch ),
           buf1,
           ch->gold,
           buf2 );

  v1 = PERCENT( ch->mana, GET_MANA(ch) );
  if ( v1 >= 100 ) strcpy( buf1, "{G[{g=========={G]{x" );
  else if ( v1 <=   0 ) strcpy( buf1, "{G[{D----------{G]{x" );
  else
    sprintf( buf1, "{G[{g%.*s{G+{D%.*s{G]{x",
             v1/10, "=========", 9-v1/10, "---------" );

  if ( ( v2 = SILVER_WEIGHT( ch->silver ) / 10  ) > 0 )
    sprintf( buf2, " {D({w%5d lb%s{D){x", v2, v2 == 1 ? " " : "s" );
  else
    sprintf( buf2, "%s", "            " );


  bprintf( buffer,
           "{D|{G M{gana{x: %5d/%-5d %s     {D|{x {wSi{Wl{Dv{wer{x: %7d%s      {D|{x\n\r",
           ch->mana,
           GET_MANA( ch ),
           //ch->max_mana,
           buf1,
           ch->silver,
           buf2 );

  v1 = PERCENT( ch->move, ch->max_move );
  if ( v1 >= 100 ) strcpy( buf1, "{B[{b=========={B]{x" );
  else if ( v1 <=   0 ) strcpy( buf1, "{B[{D----------{B]{x" );
  else
    sprintf( buf1, "{B[{b%.*s{B+{D%.*s{B]{x",
             v1/10, "=========", 9-v1/10, "---------" );

  if ( !IS_NPC( ch ))
  {
    bprintf( buffer,
             "{D|{B M{bove{x: %5d/%-5d %s     {D|{x Items:  %5d/%-5d              {D|{x\n\r",
             ch->move,
             ch->max_move,
             buf1,
             ch->carry_number,
             can_carry_n( ch ) );

    bprintf( buffer,
             "{D|{y Saves{x:%5d                        {D|{x Weight: %5d/%-7d            {D|{x\n\r",
             ch->saving_throw,
             get_carry_weight( ch ) / 10,
             can_carry_w( ch ) / 10 );

		if ( IS_CLASS_OCCULTIST(ch) )
		{
			bprintf( buffer,
						"{D| {RB{rloodshards{x: %-6d                {D|                                  {D|{x\n\r",
							ch->pcdata->bloodshards );
		}

  }

  bstrcat( buffer,
           "{g*{D------------------------------------{g*{D----------------------------------{g*{x\n\r");

  if ( ch->level > 79 )
  {
    ruby_percent = ch->ruby_fragment * 100 / 250000;
    emerald_percent = ch->emerald_fragment * 100 / 150000;

    bprintf( buffer,
             "{D| {rR{Ru{rby{x:      %6d/250000 {r[{R%3d%{r]{w%2d"
             "    {gE{Gme{gr{Ga{gld{x:  %6d/150000 {g[{G%3d{g%]{w%2d{D |{x\n\r",
             ch->ruby_fragment, ruby_percent, ch->ruby_counter,
             ch->emerald_fragment, emerald_percent, ch->emerald_counter );

    sapphire_percent = ch->sapphire_fragment * 100 / 200000;
    diamond_percent = ch->diamond_fragment * 100 / 100000;

    bprintf( buffer,
             "{D| {cSap{Bp{bhi{cre{x:  %6d/200000 {b[{c%3d%{b]{w%2d"
             "    {CDi{cam{Wo{wnd{x:  %6d/100000 {c[{C%3d%{c]{w%2d{D |{x\n\r",
             ch->sapphire_fragment, sapphire_percent, ch->sapphire_counter,
             ch->diamond_fragment, diamond_percent, ch->diamond_counter );

    bstrcat( buffer,
             "{g*{D------------------------------------{g*{D----------------------------------{g*{x\n\r");
  }

  if (!IS_NPC(ch))
  {
    bprintf( buffer,
             "{D| Practices: {w%3d                                {DTrains: {w%3d             {D|{x\n\r",
             ch->practice,
             ch->train );
  }

  if ( IS_IMMORTAL( ch ) )
  {
    if ( ch->invis_level )
      bprintf( buffer, "{D| {DWizinvis{x: %4d", ch->invis_level);
    else
      bprintf( buffer, "{D| {DWizinvis{x: %4s", "off" );

    if ( ch->incog_level )
      bprintf( buffer, "            {cIncog{x: %4d", ch->incog_level );
    else
      bprintf( buffer, "            {cIncog{x: %4s", "off" );

    if ( !IS_NPC( ch ) )
      bprintf( buffer, "         {WHoly Light{x: %4s",
               IS_SET( ch->act, PLR_HOLYLIGHT) ? "on" : "off" );
    bstrcat( buffer, "        {D|{x\n\r" );
  }

  bstrcat( buffer,
           "{g*{D-----------------------------------------------------------------------{g*{x\n\r");

  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );

  if ( IS_SET( ch->comm_flags, COMM_SHOW_AFFECTS) )
  {
    do_function(ch, &do_affects, "");
  }

  if (IS_SET( ch->comm_flags, COMM_STATS_SHOW) )
  {
    do_function(ch, &do_statistics, "");
  }

}

void do_score_records( CHAR_DATA *ch, char *argument )
{
  BUFFER *buffer;

  if ( IS_NPC( ch ) )
  {
    send_to_char( "Mobs don't keep records!\n\r", ch );
    return;
  }

  buffer = new_buf();

  bstrcat( buffer, "{gKills and Deaths:{x\n\r" );

  bprintf( buffer, "Kills:{x   {W%-5d\n\r{wPKills:  {W%-5d{x\n\r",
           ch->pcdata->kills,
           ch->pcdata->pkills );

  bprintf( buffer, "Deaths:{x  {W%-5d\n\r{wPDeaths: {W%-5d{x\n\r",
           ch->pcdata->deaths,
           ch->pcdata->pdeaths );

  bstrcat( buffer, "\n\r{gQuest Info:{x\n\r" );

  if ( ch->questdata->attempt_num > 0 )
  {
    float srate = ch->questdata->comp_num * 100;
    srate /= ch->questdata->attempt_num;

    bprintf( buffer,
             "Best Streak:{x   {W%d{x\n\r",
             ch->questdata->best_streak);
    bprintf( buffer,
             "Success rate:{x  {W%.2f%%{x\n\r\n\r", srate );
  }
  else
    bstrcat( buffer, "You haven't attempted a quest yet.\n\r" );


  bstrcat( buffer, "\n\r{gFishing Records:{x\n\r" );

  bprintf( buffer, "Fish Caught:{x   %-5d\n\r", ch->pcdata->fish_caught );
  //Guess you can't lose a fish?
  //bprintf( buffer, "Fish Lost:{x     %-5d\n\r", ch->pcdata->fish_lost );
  bprintf( buffer, "Rods Broken:{x   %-5d\n\r", ch->pcdata->fish_broken );

  bstrcat( buffer, "\n\r{gClan Information:{x\n\r" );

  bprintf( buffer, "Clans Joined:  {W%d{x\n\r", ch->pcdata->numclans );

  if ( ch->pcdata->clanowe_clan )
    bprintf( buffer, "Clan Debted:   {W%s{x\n\r", ch->pcdata->clanowe_clan );

  if ( ch->pcdata->clanowe_dia > 0 )
    bprintf( buffer, "Gems Owed:     {W%d{x\n\r", ch->pcdata->clanowe_dia );

  if ( ch->pcdata->clanowe_level > 0 )
    bprintf( buffer, "Level Needed:  {W%d{x\n\r", ch->pcdata->clanowe_level );

  if ((ch->pcdata->fdbugs + ch->pcdata->fdtypos + ch->pcdata->fdhelps + ch->pcdata->fdduhs) > 0)
  {
    bprintf( buffer, "\n\r{gGame fixes:{x\n\r" );
    if (ch->pcdata->fdbugs > 0)
      bprintf( buffer, "Reported Bugs:  {W%d{x\n\r", ch->pcdata->fdbugs );
    if (ch->pcdata->fdtypos > 0)
      bprintf( buffer, "Reported Typos: {W%d{x\n\r", ch->pcdata->fdtypos );
    if (ch->pcdata->fdhelps > 0)
      bprintf( buffer, "Reported Helps: {W%d{x\n\r", ch->pcdata->fdhelps );
    if (ch->pcdata->fdduhs > 0)
      bprintf( buffer, "Reported Duhs:  {W%d{x\n\r", ch->pcdata->fdduhs );
  }

  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );
}

void do_score_stats( CHAR_DATA *ch, char *argument )
{
  BUFFER *buffer;
  double tmp;
  int max_hps = 0;

  if ( IS_NPC( ch ) )
  {
    send_to_char( "Mobs don't need base attributes.\n\r", ch );
    return;
  }

  buffer = new_buf();

  max_hps = 40 * ch->level;

  bstrcat( buffer, "{gBase Attributes{x\n\r\n\r" );

  bprintf( buffer, "{RHp{r:{x      %d\n\r", ch->pcdata->perm_hit );

  bprintf( buffer, "{GMana{g:{x    %d\n\r", ch->pcdata->perm_mana );

  bprintf( buffer, "{BMove{b:{x    %d\n\r", ch->pcdata->perm_move );

  bprintf( buffer, "Max Bases Available:   %d\n\r", max_hps);

  bstrcat( buffer, "\n\r{gStrength{x\n\r" );

  bprintf( buffer, "Modifier to hit roll:       {W%+5d{x\n\r",
           str_app[get_curr_stat( ch, STAT_STR )].tohit );

  bprintf( buffer, "Modifier to damage roll:    {W%+5d{x\n\r",
           str_app[get_curr_stat( ch, STAT_STR )].todam );

  tmp = (double)str_app[get_curr_stat( ch, STAT_STR )].carry;
  bprintf( buffer, "Carrying capacity bonus:    {W%7.1f lb%s{x\n\r",
           tmp, tmp == 1.0 ? "" : "s" );

  tmp = (double)str_app[get_curr_stat( ch, STAT_STR )].wield;
  bprintf( buffer, "Max primary weapon weight:  {W%7.1f lb%s{x\n\r",
           tmp, tmp == 1.0 ? "" : "s" );

  tmp = (double)str_app[get_curr_stat( ch, STAT_STR )].wield / 10.0;
  bprintf( buffer, "Max secondary weapon weight:{W%7.1f lb%s{x\n\r",
           tmp, tmp == 1.0 ? "" : "s" );

  bstrcat( buffer, "\n\r{gIntelligence{x\n\r" );

  bprintf( buffer, "Learning bonus percentage:  {W%5d%%{x\n\r",
           int_app[get_curr_stat( ch, STAT_INT )].learn );

  bprintf( buffer, "Modifier to mana point:     {W%+5d{x\n\r",
           int_app[get_curr_stat( ch, STAT_INT )].mana_add );

  bprintf( buffer, "Modifier to mana gain:      {W%+5d{x\n\r",
           int_app[get_curr_stat( ch, STAT_INT )].manap );

  bstrcat( buffer, "\n\r{gWisdom{x\n\r" );

  bprintf( buffer, "Practices gained per level: {W%5d{x\n\r",
           wis_app[get_curr_stat( ch, STAT_WIS )].practice );

  bprintf( buffer, "Fragment modifier:          {W%5d%%{x\n\r",
           wis_app[get_curr_stat( ch, STAT_WIS )].frag_percent );

  bstrcat( buffer, "\n\r{gDexterity{x\n\r" );

  bprintf( buffer, "Modifier to armor class:    {W%+5d{x\n\r",
           dex_app[get_curr_stat( ch, STAT_DEX )].defensive );

  bprintf( buffer, "Modifier to move gain:      {W%+5d{x\n\r",
           dex_app[get_curr_stat( ch, STAT_DEX )].movep );

  bprintf( buffer, "Bonus to item capacity:     {W%+5d{x\n\r",
           dex_app[get_curr_stat( ch, STAT_DEX )].item_bonus );

  bstrcat( buffer, "\n\r{gConstitution{x\n\r" );

  bprintf( buffer, "Modifier to hit point gain: {W%+5d{x\n\r",
           con_app[get_curr_stat( ch, STAT_CON )].hitp );

  bprintf( buffer, "Modifier to hit point:      {W%+5d{x\n\r",
           con_app[get_curr_stat( ch, STAT_CON )].hp_add );

  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );
}


void do_score_race( CHAR_DATA *ch, char *argument )
{
  BUFFER *buffer;
  char buf1[MSL], buf2[MSL];
  bool found1;
  int i, bclass, wclass;

  if ( IS_NPC( ch ) )
  {
    send_to_char( "None of your business, mob!\n\r", ch );
    return;
  }

  if ( ch->level < 5 )
  {
    send_to_char( "You must reach level five before seeing that information.\n\r", ch );
    return;
  }

  buffer = new_buf();

  mprintf( sizeof(buf1), buf1, "%s (%s) ", race_table[ch->race].name, pc_race_table[ch->race].who_name );
  bprintf( buffer, "       {cRace: {x%s\n\r", buf1);
  bprintf( buffer, "       {cSize: {x%s\n\r", capitalize(flag_string( size_flags, pc_race_table[ch->race].size )) );
  bprintf( buffer, "     {cSector: {x%s\n\r", capitalize(flag_string( sector_flags, race_table[ch->race].native_sect )) );
  bprintf( buffer, "\n\r");
  bprintf( buffer, "  {cResistant: {x%s\n\r", flag_string( imm_flags, race_table[ch->race].res ));
  bprintf( buffer, " {cVulnerable: {x%s\n\r", flag_string( imm_flags, race_table[ch->race].vuln ) );

  strcpy(buf1,"");
  found1 = FALSE;
  for ( i = 0; i < MAX_IN_RACE; i++ )
  {
    if ( IS_NULLSTR( pc_race_table[ch->race].skills[i] ) )
      break;

    if (found1)
      strcat(buf1,", ");

    strcat(buf1, pc_race_table[ch->race].skills[i]);
    found1 = TRUE;
  }
  if (strlen(buf1) < 1)
    strcpy(buf1,"none");
  bprintf( buffer, "  {cAbilities: {x%s\n\r", buf1);

  strcpy(buf1,flag_string( affect_flags, race_table[ch->race].aff ));
  strcpy(buf2,flag_string( affect2_flags, race_table[ch->race].aff2 ));
  if (!strcmp(buf1,"none"))
    strcpy(buf1,"");
  if (!strcmp(buf2,"none"))
    strcpy(buf2,"");
  if (strlen(buf1) > 1)
    strcat(buf1," ");
  if ((strlen(buf1) < 1)
      &&  (strlen(buf2) < 1))
    strcpy(buf1,"none");

  bprintf( buffer, "    {cAffects: {x%s%s\n\r", buf1, buf2);
  bprintf( buffer, "\n\r");
  bprintf( buffer, " {cBody Parts: {x%s\n\r", flag_string( part_flags, race_table[ch->race].parts ) );

  if (ch->race == race_lookup("human"))
  {
    strcpy(buf1,"Humans are equally good at all classes");
    strcpy(buf2,"Humans are equally good at all classes");
  }
  else if (ch->race == race_lookup("drow"))
  {
    strcpy(buf1,"Drow are equally good at all classes");
    strcpy(buf2,"Drow are equally good at all classes");
  }
  else
  {
    strcpy(buf1,""); // best
    strcpy(buf2,""); // worst
    found1 = FALSE;  // best
    bclass = 200; // as we find better, it will go down
    wclass = 0;   // as we find worse, it will go up

    for ( i = 0; i < MAX_CLASS; i++ )
    {
      if ( pc_race_table[ch->race].class_mult[i] < 1 )
        continue;

      if (pc_race_table[ch->race].class_mult[i] < bclass)
      {
        // we found a better one, set it
        strcpy(buf1, capitalize( class_table[i].name));
        bclass = pc_race_table[ch->race].class_mult[i];
        found1 = TRUE;
      }
      else if (pc_race_table[ch->race].class_mult[i] == bclass)
      {
        // we found as good of one, cat it
        strcat(buf1, ", ");
        strcat(buf1, capitalize( class_table[i].name));
      }

      if (pc_race_table[ch->race].class_mult[i] > wclass)
      {
        // we found a worse one, set it
        strcpy(buf2, capitalize( class_table[i].name));
        wclass = pc_race_table[ch->race].class_mult[i];
      }
      else if (pc_race_table[ch->race].class_mult[i] == wclass)
      {
        // we found as bad of one, cat it
        strcat(buf2, ", ");
        strcat(buf2, capitalize( class_table[i].name));
      }
    }
  }

  bprintf( buffer, " {cBest Class: {x%-20s {c\n\rWorst Class: {x%s\n\r", buf1, buf2 );

  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );
}

void do_score( CHAR_DATA *ch, char *argument )
{
  char arg[MSL];

  argument = one_argument( argument, arg );

  if ( IS_NULLSTR( arg ) )
  {
    do_function( ch, &do_scorio, argument );
    return;
  }

  if ( !str_prefix( arg, "old" ) )
  {
    do_function( ch, &do_oldscore, argument );
    return;
  }

  else if ( !str_prefix( arg, "base" ) || !str_prefix( arg, "bases" ) )
  {
    do_function( ch, &do_score_stats, argument );
    return;
  }

  else if ( !str_prefix( arg, "stats" ) )
  {
    do_function( ch, &do_statistics, argument );
    return;
  }
  else if ( !str_prefix( arg, "clan" ) )
  {
    do_function( ch, &do_score_clan, argument );
    return;
  }
  else if ( !str_prefix( arg, "records" ) )
  {
    do_function( ch, &do_score_records, argument );
    return;
  }

  else if ( !str_prefix( arg, "race" ) )
  {
    do_function( ch, &do_score_race, argument );
    return;
  }

  else if ( !str_prefix( arg, "player" )
            &&        IS_IMMORTAL( ch ) )
  {
    do_function( ch, &do_other_scorio, argument );
    return;
  }

  do_function( ch, &do_scorio, argument );
}

void do_other_scorio( CHAR_DATA *ch, char *argument )
{
  BUFFER *buffer;
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int v1, v2;
  CHAR_DATA *victim;
  char arg[MSL];
  char arg2[MSL];
  char buf3[MSL];
  bool load_char=FALSE;
  DESCRIPTOR_DATA d;
  DENIED_DATA *dnd;
  int ruby_percent;
  int sapphire_percent;
  int emerald_percent;
  int diamond_percent;

  buffer = new_buf();

  if ( !IS_IMMORTAL(ch) )
  {
    do_function( ch, &do_scorio, argument );
    return;
  }

  strcpy( arg2, argument );
  for (dnd = denied_list; dnd; dnd = dnd->next)
    if (is_exact_name(capitalize(arg),dnd->name))
    {
      send_to_char( "Sorry, this character is {RDenied{x.\n\r", ch );
      return;
    }

  if ( ( ( victim = get_char_world( ch, argument ) ) != NULL )
       &&     ( !IS_NPC(victim ) ) )
  {
    load_char = FALSE;
  }
  else
  {
    d.original = NULL;

    if ( !load_char_obj(&d, capitalize(argument))) /*Does the Pfile exist??*/
    {
      printf_to_char(ch, "%s is not loged on!\n\r", capitalize(argument));
      free_char(d.character);
      return;
    }
    d.character->desc = NULL; //For safety sake..
    victim = d.character;
    load_char = TRUE;
  }

  if ( get_trust( victim ) > get_trust( ch ) )
  {
    send_to_char( "Yeah...right!\n\r", ch );
    return;
  }

  bstrcat( buffer,
           "{g*{D-----------------------------------------------------------------------{g*{x\n\r");

  if ( IS_NPC(victim) )
    sprintf( buf1, "%6s", "      ");
  else
  {
    strcpy( buf2,size_table[pc_race_table[victim->race].size].name );
    sprintf( buf1, "%-6s",
             capitalize( buf2 ) );
  }

  bprintf( buffer,
           "{D|{x Name:  %-15s   Race:   %-10s   Size: %s            {D|{x\n\r",
           victim->name,
           race_table[victim->race].name,
           buf1);

  if ( IS_NPC( victim ) )
    sprintf( buf1, "%8s", "mobile  ");
  else
    sprintf( buf1, "%-8s",
             class_table[victim->gameclass].name);

  if ( IS_NPC( victim ) )
    sprintf( buf2, "%9s", "Null     ");
  else if ( is_clan( victim ) )
    sprintf( buf2, "%-9s",
             victim->clan->symbol );
  else
    sprintf( buf2, "%9s", "None     ");

  bprintf( buffer,
           "{D|{c Level{w: %-3d               {cClass{x:  %-11s  {cClan{x: %-9s         {D|{x\n\r",
           victim->level,
           buf1,
           buf2);

  if ( IS_NPC( victim ) )
    sprintf( buf1, "%-7s", "N/A    ");
  else
    sprintf( buf1, "%-7s",
             victim->sex == 0 ? "Sexless" : victim->sex == 1 ? "Male" : "Female");

  if ( IS_NPC( victim )
       || !IS_IN_CLAN( victim ) )
    sprintf( buf2, "%-16s", "None            ");
  else
    sprintf( buf2, "%-16s", get_rank( victim ) );

  strncpy_color( buf3,
                 FIX_STR( buf2, "", "" ),
                 16, ' ', TRUE );

  bprintf( buffer,
           "{D|{m Pos.{x:  %-16s  {mSex{x:    %-7s      {mRank{x: %s  {D|{x\n\r",
           capitalize( position_table[victim->position].name ),
           buf1,
           buf3 );

  if ( !IS_NPC( victim ) )
  {
    v1 = ( victim->played + (int) (current_time - victim->logon) ) / 3600;

    bprintf( buffer,
             "{D|{g Age{x:   %-4d              {gPlayed{x: %-5d (%3d Day%s, %2d Hour%s)%s%s         {D|{x\n\r",
             get_age( victim ),
             v1,
             v1 / 24,
             ( v1 / 24 == 1 ) ? "" : "s",
             v1 % 24,
             ( v1 % 24 == 1 ) ? "" : "s",
             ( v1 / 24 == 1 ) ? " " : "",
             ( v1 % 24 == 1 ) ? " " : "" );
  }

  bstrcat( buffer,
           "{g*{D------------------------------------{g*{D----------------------------------{g*{x\n\r");

  v1 = PERCENT( ch->hit, victim->max_hit );
  if ( v1 >= 100 ) strcpy( buf1, "{R[{r=========={R]{x" );
  else if ( v1 <    0 ) strcpy( buf1, "{R[{D----------{R]{x" );
  else
    sprintf( buf1, "{R[{r%.*s{R+{D%.*s{R]{x",
             v1/10, "=========", 9-v1/10, "---------" );

  if ( ( v2 = GOLD_WEIGHT( victim->gold ) / 10 ) > 0 )
    sprintf( buf2, " {y({w%5d lb%s{y){x", v2, v2 == 1 ? " " : "s" );
  else
    sprintf( buf2, "%s", "            " );

  bprintf( buffer,
           "{D|{R H{rit{x:  %5d/%-5d %s     {D|{x {yG{Yo{yld{x:   %7d%s      {D|{x\n\r",
           victim->hit,
           GET_HP( victim ),
           buf1,
           victim->gold,
           buf2 );

  v1 = PERCENT( victim->mana, victim->max_mana );
  if ( v1 >= 100 ) strcpy( buf1, "{G[{g=========={G]{x" );
  else if ( v1 <    0 ) strcpy( buf1, "{G[{D----------{G]{x" );
  else
    sprintf( buf1, "{G[{g%.*s{G+{D%.*s{G]{x",
             v1/10, "=========", 9-v1/10, "---------" );

  if ( ( v2 = SILVER_WEIGHT( victim->silver ) / 10  ) > 0 )
    sprintf( buf2, " {D({w%5d lb%s{D){x", v2, v2 == 1 ? " " : "s" );
  else
    sprintf( buf2, "%s", "            " );


  bprintf( buffer,
           "{D|{G M{gana{x: %5d/%-5d %s     {D|{x {wSi{Wl{Dv{wer{x: %7d%s      {D|{x\n\r",
           victim->mana,
           GET_MANA( victim ),
           //ch->max_mana,
           buf1,
           victim->silver,
           buf2 );

  v1 = PERCENT( victim->move, victim->max_move );
  if ( v1 >= 100 ) strcpy( buf1, "{B[{b=========={B]{x" );
  else if ( v1 <    0 ) strcpy( buf1, "{B[{D----------{B]{x" );
  else
    sprintf( buf1, "{B[{b%.*s{B+{D%.*s{B]{x",
             v1/10, "=========", 9-v1/10, "---------" );

  if ( !IS_NPC( victim ))
  {
    bprintf( buffer,
             "{D|{B M{bove{x: %5d/%-5d %s     {D|{x Items:  %5d/%-5d              {D|{x\n\r",
             victim->move,
             victim->max_move,
             buf1,
             victim->carry_number,
             can_carry_n( victim ) );

    if ( victim->wimpy > 0 )
      sprintf( buf1, "%5d  ", victim->wimpy);
    else
      sprintf( buf1, "%5s", "Off    ");

    bprintf( buffer,
             "{D|{y Wimpy{x: %5s                     {D|{x Weight: %5d/%-7d            {D|{x\n\r",
             buf1,
             get_carry_weight( victim ) / 10,
             can_carry_w( victim ) / 10 );

  }

  bstrcat( buffer,
           "{g*{D------------------------------------{g*{D----------------------------------{g*{x\n\r");

  if ( ch->level > 79 )
  {
    ruby_percent = victim->ruby_fragment * 100 / 250000;
    emerald_percent = victim->emerald_fragment * 100 / 150000;

    bprintf( buffer,
             "{D| {rR{Ru{rby{x:      %6d/250000 {r[{R%3d%{r]{w%2d"
             "    {gE{Gme{gr{Ga{gld{x:  %6d/150000 {g[{G%3d{g%]{w%2d{D |{x\n\r",
             victim->ruby_fragment, ruby_percent, victim->ruby_counter,
             victim->emerald_fragment, emerald_percent, victim->emerald_counter );

    sapphire_percent = victim->sapphire_fragment * 100 / 200000;
    diamond_percent = victim->diamond_fragment * 100 / 100000;

    bprintf( buffer,
             "{D| {cSap{Bp{bhi{cre{x:  %6d/200000 {b[{c%3d%{b]{w%2d"
             "    {CDi{cam{Wo{wnd{x:  %6d/100000 {c[{C%3d%{c]{w%2d{D |{x\n\r",
             victim->sapphire_fragment, sapphire_percent, victim->sapphire_counter,
             victim->diamond_fragment, diamond_percent, victim->diamond_counter );

    bstrcat( buffer,
             "{g*{D-------------------------------------{g*{D---------------------------------{g*{x\n\r");
  }

  if (!IS_NPC(victim))
  {
    bprintf( buffer,
             "{D| {wPractices: {w%3d                                {wTrains: {w%3d             {D|{x\n\r",
             victim->practice,
             victim->train );
  }

  if ( IS_IMMORTAL( victim ) )
  {
    if ( ch->invis_level )
      bprintf( buffer, "{D| {DWizinvis{x: %4d", victim->invis_level);
    else
      bprintf( buffer, "{D| {DWizinvis{x: %4s", "off" );

    if ( ch->incog_level )
      bprintf( buffer, "            {cIncog{x: %4d", victim->incog_level );
    else
      bprintf( buffer, "            {cIncog{x: %4s", "off" );

    if ( !IS_NPC( ch ) )
      bprintf( buffer, "         {WHoly Light{x: %4s",
               IS_SET( victim->act, PLR_HOLYLIGHT) ? "on" : "off" );
    bstrcat( buffer, "        {D|{x\n\r" );
  }

  bstrcat( buffer,
           "{g*{D-----------------------------------------------------------------------{g*{x\n\r");

  if ( IS_SET( victim->comm_flags, COMM_SHOW_AFFECTS) )
  {
    do_function(ch, &do_affects, "");
  }

  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );
  if (load_char)
  {
    nuke_pets(d.character,FALSE);
    free_char(d.character);
  }

}

void do_score_clan( CHAR_DATA *ch, char *argument )
{
  BUFFER      *buffer;
  OBJ_DATA    *obj;
  CLAN_DATA   *clan = NULL;
  RANK_DATA   *rank = NULL;
  char        buf[MSL];
  char        buf1[MSL];
  char        buf2[MSL];
  char        ranklev[4];
  char        rcolor[4];
  bool        showrank = TRUE;

  if ( IS_NPC( ch ) )
  {
    send_to_char( "Mobs don't have clans, iddddddiot!\n\r", ch );
    return;
  }

  buffer = new_buf( );

  if ( !IS_IMMORTAL( ch )
       &&   !ch->clan )
  {
    send_to_char( "Sorry, you are not in a clan.\n\r", ch );
    return;
  }

  if ( !IS_IMMORTAL( ch ) )
    clan = ch->clan;

  if ( IS_IMMORTAL( ch ) )
  {
    if ( IS_NULLSTR( argument ) )
    {
      if ( !ch->clan )
      {
        send_to_char( "Syntax: score clan <name>\n\r", ch );
        return;
      }
      else
        clan = ch->clan;
    }
    else
    {
      showrank = FALSE;
      if ( !( clan = get_clan( argument, TRUE ) ) )
      {
        send_to_char( "Syntax: score clan <valid clan name>\n\r", ch );
        return;
      }
    }

  }

  bprintf( buffer, "Clan Information for: %s\n\r", clan->symbol );

  bprintf( buffer, "Name:    %s\n\r", clan->name );

  if ( ( obj = find_obj_vnum( clan->donation_gem ) ) )
    bprintf( buffer, "Gem:     %s\n\r", obj->short_descr );

  if ( !IS_IMMORTAL( ch ) )
    bprintf( buffer, "Donated: %d\n\r", ch->pcdata->donated_dia );

  bprintf( buffer, "Balance: %d\n\r", clan->donation_balance );
  bprintf( buffer, "Spent:   %d\n\r", clan->donation_spent );
  bprintf( buffer, "Total:   %d\n\r", clan->donation_total );

  if ( clan->rank )
  {
    bstrcat( buffer,
             "{cLvl Male             Female            Neutral{x\n\r\n\r" );

    for ( rank = clan->rank ; rank ; rank = rank->next )
    {
      strncpy_color( buf,
                     FIX_STR( rank->male, "<Recruit", "<Recruit" ),
                     16, ' ', TRUE );

      strncpy_color( buf1,
                     FIX_STR( rank->female, "<Recruit>", "<Recruit>" ),
                     16, ' ', TRUE );

      strncpy_color( buf2,
                     FIX_STR( rank->neutral, "<Recruit>", "<Recruit>" ),
                     16, ' ', TRUE );

      strcpy(rcolor,"{w");
      strcpy(ranklev," ");

      if ( showrank )
      {
        if ( rank->level == ch->pcdata->clan_rank )
        {
          if ( ( rank->rank_flags & 65520 ) == 0 )
          {
            strcpy(rcolor,"{W");
            strcpy(ranklev,"*");
          }
          else
          {
            switch ( ch->gameclass )
            {

              case cConjurer :
                if ( IS_SET( rank->rank_flags, RANK_CONJURER ) )
                {
                  strcpy(ranklev,"*");
                  strcpy(rcolor,"{W");
                }
                break;
              case cPriest :
                if ( IS_SET( rank->rank_flags, RANK_PRIEST ) )
                {
                  strcpy(ranklev,"*");
                  strcpy(rcolor,"{W");
                }
                break;
              case cHighwayman :
                if ( IS_SET( rank->rank_flags, RANK_HIGHWAYMAN ) )
                {
                  strcpy(ranklev,"*");
                  strcpy(rcolor,"{W");
                }
                break;
              case cKnight :
                if ( IS_SET( rank->rank_flags, RANK_KNIGHT ) )
                {
                  strcpy(ranklev,"*");
                  strcpy(rcolor,"{W");
                }
                break;
              case cWarlock :
                if ( IS_SET( rank->rank_flags, RANK_WARLOCK ) )
                {
                  strcpy(ranklev,"*");
                  strcpy(rcolor,"{W");
                }
                break;
              case cBarbarian :
                if ( IS_SET( rank->rank_flags, RANK_BARBARIAN ) )
                {
                  strcpy(ranklev,"*");
                  strcpy(rcolor,"{W");
                }
                break;
              case cMystic :
                if ( IS_SET( rank->rank_flags, RANK_MYSTIC ) )
                {
                  strcpy(ranklev,"*");
                  strcpy(rcolor,"{W");
                }
                break;
              case cDruid :
                if ( IS_SET( rank->rank_flags, RANK_DRUID ) )
                {
                  strcpy(ranklev,"*");
                  strcpy(rcolor,"{W");
                }
                break;
              case cInquisitor :
                if ( IS_SET( rank->rank_flags, RANK_INQUISITOR ) )
                {
                  strcpy(ranklev,"*");
                  strcpy(rcolor,"{W");
                }
                break;
              case cOccultist :
                if ( IS_SET( rank->rank_flags, RANK_OCCULTIST ) )
                {
                  strcpy(ranklev,"*");
                  strcpy(rcolor,"{W");
                }
                break;
              case cAlchemist :
                if ( IS_SET( rank->rank_flags, RANK_ALCHEMIST ) )
                {
                  strcpy(ranklev,"*");
                  strcpy(rcolor,"{W");
                }
                break;
              case cWoodsman :
                if ( IS_SET( rank->rank_flags, RANK_WOODSMAN ) )
                {
                  strcpy(ranklev,"*");
                  strcpy(rcolor,"{W");
                }
                break;
            } // switch
          } // else
        } // if ranks match
      } // if showrank
      bprintf( buffer,
               "%s%3d%s{x%-18s %-18s  %-18s %s%s\n\r",
               rcolor,
               rank->level,
               ranklev,
               buf,
               buf1,
               buf2,
               rcolor,
               flag_string( rank_flags, rank->rank_flags ) );
    }
  }

  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );

}

int toxin_type( CHAR_DATA *ch )
{
  AFFECT_DATA *paf;
  int toxtype = TOX_VENOM;

  for ( paf = ch->affected; paf; paf = paf->next )
  {
    if ( (paf->bitvector == AFF_POISON)
         &&   (paf->location == APPLY_NONE) )
    {
      // find the type identifier affect
      toxtype = paf->modifier;
      break;
    }
  }

  return toxtype;
}
