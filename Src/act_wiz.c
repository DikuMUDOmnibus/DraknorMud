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
*    ROM 2.4 is copyright 1993-1996 Russ Taylor                            *
*    ROM has been brought to you by the ROM consortium                     *
*        Russ Taylor (rtaylor@efn.org)                                     *
*        Gabrielle Taylor                                                  *
*        Brian Moore (zump@rom.org)                                        *
*    By using this code, you have agreed to follow the terms of the        *
*    ROM license, in the file Rom24/doc/rom.license                        *
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
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"
#include "magic.h"
#include "interp.h"

struct thread_pass_st pass;

/*
 * Local functions.
 */
/* opposite directions */
const sh_int opposite_dir [6] = { DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP };
void reset_denied_file(char *filename);
void logsearch_do();
void new_dump ();
void purge_room( ROOM_INDEX_DATA *pRoom, bool override );
char *do_mload( CHAR_DATA *ch, char *argument);
char *do_oload( CHAR_DATA *ch, char *argument);

enum penalty_action_e {pen_killer_e,pen_thief_e,pen_twit_e,pen_violent_e, pen_notedeny_e,
                       pen_bailed_e
                      };

void do_wiznet( CHAR_DATA *ch, char *argument )
{
  int flag;
  int col=0;
  int wlev=0;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  if ( argument[0] == '\0' )
  {
    for (wlev = 100; wlev <= get_trust(ch); wlev++)
    {
      for (flag = 0; wiznet_table[flag].name != NULL; flag++)
      {
        if ( wiznet_table[flag].level == wlev )
        {
          sprintf( buf, "{D(%3d) {c%-14s{x %-7s ",
                   wiznet_table[flag].level,
                   wiznet_table[flag].name,
                   IS_SET(ch->wiznet,wiznet_table[flag].flag) ? "{gON{x" : "{gOFF{x" );
          send_to_char(buf, ch);
          col++;

          if (col==3)
          {
            send_to_char("\n\r",ch);
            col=0;
          }
        } // if level/trust

        /* To avoid color bleeding */
        send_to_char("{x",ch);
      } // for flag
    } // for level
    send_to_char( "\n\r", ch );
    return;
  }

  if (!str_prefix(argument,"on"))
  {
    send_to_char("Welcome to Wiznet!\n\r",ch);
    SET_BIT(ch->wiznet,WIZ_ON);
    return;
  }

  if (!str_prefix(argument,"off"))
  {
    send_to_char("Signing off of Wiznet.\n\r",ch);
    REMOVE_BIT(ch->wiznet,WIZ_ON);
    return;
  }

  /* show wiznet status */
  if (!str_prefix(argument,"status"))
  {
    buf[0] = '\0';

    if (!IS_SET(ch->wiznet,WIZ_ON))
      strcat(buf,"off ");

    for (flag = 0; wiznet_table[flag].name != NULL; flag++)
      if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
      {
        strcat(buf,wiznet_table[flag].name);
        strcat(buf," ");
      }

    strcat(buf,"\n\r");

    send_to_char("Wiznet status:\n\r",ch);
    send_to_char(buf,ch);
    return;
  }

  if (!str_prefix(argument,"show"))
    /* list of all wiznet options */
  {
    buf[0] = '\0';
    col = 0;

    for (flag = 0; wiznet_table[flag].name != NULL; flag++)
    {
      if (wiznet_table[flag].level <= get_trust(ch))
      {
        if (col==4)
        {
          strcat(buf,"\n\r");
          col=0;
        }
        col++;

        sprintf( buf2, "{c%-14s{x  ",wiznet_table[flag].name);

        strcat(buf,buf2);
        strcat(buf," ");
      }
    }

    strcat(buf,"\n\r");

    send_to_char("Wiznet options available to you are:\n\r",ch);
    send_to_char(buf,ch);
    return;
  }

  flag = wiznet_lookup(argument);

  if (flag == -1 || get_trust(ch) < wiznet_table[flag].level)
  {
    send_to_char("No such option.\n\r",ch);
    return;
  }

  if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
  {
    printf_to_char(ch,"You will no longer see %s on wiznet.\n\r",
                   wiznet_table[flag].name);
    REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
    return;
  }
  else
  {
    printf_to_char(ch,"You will now see %s on wiznet.\n\r",
                   wiznet_table[flag].name);
    SET_BIT(ch->wiznet,wiznet_table[flag].flag);
    return;
  }

}

void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
            long flag, long flag_skip, int min_level)
{
  DESCRIPTOR_DATA *d;
  char buf[ MAX_STRING_LENGTH ];
  int spt = 0;
  int bpt = 0;

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if (d->connected == CON_PLAYING
        &&  IS_IMMORTAL(d->character)
        &&  IS_SET(d->character->wiznet,WIZ_ON)
        &&  (!flag || IS_SET(d->character->wiznet,flag))
        &&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
        &&  get_trust(d->character) >= min_level
        &&  d->character != ch)
    {
      if (IS_SET(d->character->wiznet,WIZ_PREFIX))
        send_to_char("--> ",d->character);

      if (!strstr(string,"$"))
        act_new(string,d->character,obj,ch,TO_CHAR,POS_DEAD);
      else
      {
        // damn you, act_new, and your dollar sign code!
        while (string[spt] != '\0')
        {
          if (string[spt] != '$')
          {
            buf[bpt] = string[spt];
            bpt++;
          }
          spt++;
        }
        buf[bpt] = '\0';
        act_new(buf,d->character,obj,ch,TO_CHAR,POS_DEAD);
      }
    }
  }

  return;
}

void wiznet_special(char *string, CHAR_DATA *ch, long flag, long flag_skip, int min_level)
{
  DESCRIPTOR_DATA *d;

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if (d->connected == CON_PLAYING
        &&  IS_IMMORTAL(d->character)
        &&  IS_SET(d->character->wiznet,WIZ_ON)
        &&  (!flag || IS_SET(d->character->wiznet,flag))
        &&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
        &&  get_trust(d->character) >= min_level
        &&  d->character != ch)
    {
      if (IS_SET(d->character->wiznet,WIZ_PREFIX))
        send_to_char("--> ",d->character);

      act_new("$t",d->character,string,ch,TO_CHAR,POS_DEAD);
    }
  }
  return;
}


void do_bank_update( CHAR_DATA *ch, char *argument )         /* by Maniac */
{
  char    arg[MAX_INPUT_LENGTH];

  /*        if ( !authorized( ch, "iscore" ) )
        return;
  */
  if ( argument[0] == '\0' )    /* No options ??? */
  {
    send_to_char( "Update, call some game functions\n\r\n\r", ch );
    send_to_char( "bank: Update the share_value.\n\r", ch );
    return;
  }

  argument = one_argument(argument, arg);

  if (!str_prefix(arg, "bank" ) )
  {
    bank_update ( );
    send_to_char ("Ok...bank updated.\n\r", ch);
    return;
  }

  return;
}

/*
 * Number of objects within another that match the index data.
 * This does include the container itself.
 */
int get_obj_contains_index( OBJ_DATA *obj, OBJ_INDEX_DATA *pObjIndex )
{
  OBJ_DATA    *iObj;
  int     number;

  if ( IS_DELETED( obj ) )
    return 0;

  if ( obj->pIndexData == pObjIndex )
    number = 1;
  else
    number = 0;

  for ( iObj = obj->contains; iObj != NULL; iObj = iObj->next_content )
    number += get_obj_contains_index( iObj, pObjIndex );

  return number;
}

/*
 * Number of objects carried that matches the index data.
 */
int get_obj_carry_index( CHAR_DATA *ch, OBJ_INDEX_DATA *pObjIndex )
{
  OBJ_DATA    *obj;
  int     number = 0;

  for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    number += get_obj_contains_index( obj, pObjIndex );

  return number;
}


/*
 * Equips a low level player.
 */
void do_outfit( CHAR_DATA *ch, char *argument )
{
  OBJ_INDEX_DATA    *pObjIndex;
  OBJ_DATA        *obj;
  bool        fEquip = FALSE;

  if ( ch->level > 5 || IS_NPC( ch ) )
  {
    send_to_char( "Find it yourself!\n\r", ch );
    return;
  }

  /* Give a light. */
  if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL
       &&     ch->carry_number < can_carry_n( ch ) )
  {
    if ( ( pObjIndex = get_obj_index( OBJ_VNUM_SCHOOL_BANNER ) ) != NULL
         &&   get_obj_carry_index( ch, pObjIndex ) == 0 )
    {
      obj = create_object( pObjIndex, 0 );
      obj->cost = 0;
      obj_to_char( obj, ch );
      equip_char( ch, obj, WEAR_LIGHT );
      fEquip = TRUE;
    }
  }

  /* Give a vest. */
  if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL
       &&     ch->carry_number < can_carry_n( ch ) )
  {
    if ( ( pObjIndex = get_obj_index( OBJ_VNUM_SCHOOL_VEST ) ) != NULL
         &&   get_obj_carry_index( ch, pObjIndex ) == 0 )
    {
      obj = create_object( pObjIndex, 0 );
      obj->cost = 0;
      obj_to_char( obj, ch );
      equip_char( ch, obj, WEAR_BODY );
      fEquip = TRUE;
    }
  }

  /* Give a weapon. */
  if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
       &&     ch->carry_number < can_carry_n( ch ) )
  {
    int    i;
    int    sn   = 0;
    int    vnum = OBJ_VNUM_SCHOOL_SWORD; /* Just in case! */

    /* Pick a weapon. */
    for ( i = 0; weapon_table[i].name != NULL; i++ )
    {
      if ( ch->pcdata->learned[sn] <
           ch->pcdata->learned[*weapon_table[i].gsn] )
      {
        sn   = *weapon_table[i].gsn;
        vnum = weapon_table[i].vnum;
      }
    }

    if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL
         &&   get_obj_carry_index( ch, pObjIndex ) == 0 )
    {
      obj = create_object( pObjIndex, 0 );
      obj->cost = 0;
      obj_to_char( obj, ch );
      equip_char( ch, obj, WEAR_WIELD );
      fEquip = TRUE;
    }
  }

  /* Give a shield. */
  if ( ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
         ||       !IS_WEAPON_STAT( obj, WEAPON_TWO_HANDS ) )
       &&     get_eq_char( ch, WEAR_SECONDARY ) == NULL
       &&     get_eq_char( ch, WEAR_SHIELD ) == NULL
       &&     ch->carry_number < can_carry_n( ch ) )
  {
    if ( ( pObjIndex = get_obj_index( OBJ_VNUM_SCHOOL_SHIELD ) ) != NULL
         &&   get_obj_carry_index( ch, pObjIndex ) == 0 )
    {
      obj = create_object( pObjIndex, 0 );
      obj->cost = 0;
      obj_to_char( obj, ch );
      equip_char( ch, obj, WEAR_SHIELD );
      fEquip = TRUE;
    }
  }

  /* Lets take care of giving a map on creation, otherwise they have to buy it*/
  /*if ( ch->carry_number < can_carry_n( ch ) )
    {
      if ( (pObjIndex = get_obj_index( OBJ_VNUM_MAP ) ) != NULL
         &&   get_obj_carry_index( ch, pObjIndex ) == 0 )
      {
        if (IS_EVIL(ch))
            pObjIndex = get_obj_index( OBJ_VNUM_CAL_MAP );
    
        obj = create_object( pObjIndex, 0 );
        obj->cost = 0;
        obj_to_char( obj, ch );
        fEquip = TRUE;
      }
    }*/

  if ( fEquip )
  {
    act( "You have been equipped by {gLa{Gur{ce{wl{cin{x.", ch, NULL, NULL, TO_CHAR );
    act( "$n has been been equipped by {gLa{Gur{ce{wl{cin{x.", ch, NULL, NULL, TO_ROOM );
  }
  else
  {
    if ( ch->carry_number >= can_carry_n( ch ) )
      send_to_char( "Perhaps you should drop something first?\n\r", ch );
    else
      act( "You should be a little less greedy.", ch, NULL, NULL, TO_CHAR );

  }
}

#if OLD
/* equips a character */
void do_outfit ( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  int i,sn,vnum;

  if (ch->level > LEVEL_NEWBIE || IS_NPC(ch))
  {
    send_to_char("Find it yourself!\n\r",ch);
    return;
  }

  if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL )
  {
    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0 );
    obj->cost = 0;
    if (ch->carry_number + 1 > can_carry_n(ch))
    {
      send_to_char("Sorry, you have reached your carry limit,\n\r",ch);
      free_obj(obj);
      return;
    }
    if (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch))
    {
      send_to_char("Sorry, you have reached your weight limit,\n\r",ch);
      free_obj(obj);
      return;
    }
    obj_to_char( obj, ch );
    equip_char( ch, obj, WEAR_LIGHT );
  }

  if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL )
  {
    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );
    obj->cost = 0;
    if (ch->carry_number + 1 > can_carry_n(ch))
    {
      send_to_char("Sorry, you have reached your carry limit,\n\r",ch);
      free_obj(obj);
      return;
    }
    if (get_carry_weight(ch) + get_obj_weight(obj) >
        can_carry_w(ch))
    {
      send_to_char("Sorry, you have reached your weight limit,\n\r",ch);
      free_obj(obj);
      return;
    }
    obj_to_char( obj, ch );
    equip_char( ch, obj, WEAR_BODY );
  }

  /* do the weapon thing */
  if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL)
  {
    sn = 0;
    vnum = OBJ_VNUM_SCHOOL_SWORD; /* just in case! */

    for (i = 0; weapon_table[i].name != NULL; i++)
    {
      if (ch->pcdata->learned[sn] <
          ch->pcdata->learned[*weapon_table[i].gsn])
      {
        sn = *weapon_table[i].gsn;
        vnum = weapon_table[i].vnum;
      }
    }

    obj = create_object(get_obj_index(vnum),0);
    if (ch->carry_number + 1 > can_carry_n(ch))
    {
      send_to_char("Sorry, you have reached your carry limit,\n\r",ch);
      free_obj(obj);
      return;
    }
    if (get_carry_weight(ch) + get_obj_weight(obj) >
        can_carry_w(ch))
    {
      send_to_char("Sorry, you have reached your weight limit,\n\r",ch);
      free_obj(obj);
      return;
    }
    obj_to_char(obj,ch);
    equip_char(ch,obj,WEAR_WIELD);
  }

  if (((obj = get_eq_char(ch,WEAR_WIELD)) == NULL
       ||   !IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS))
      &&  (obj = get_eq_char( ch, WEAR_SHIELD ) ) == NULL )
  {
    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
    obj->cost = 0;
    if (ch->carry_number + 1 > can_carry_n(ch))
    {
      send_to_char("Sorry, you have reached your carry limit,\n\r",ch);
      free_obj(obj);
      return;
    }
    if (get_carry_weight(ch) + get_obj_weight(obj) >
        can_carry_w(ch))
    {
      send_to_char("Sorry, you have reached your weight limit,\n\r",ch);
      free_obj(obj);
      return;
    }
    obj_to_char( obj, ch );
    equip_char( ch, obj, WEAR_SHIELD);
  }

  send_to_char("You have been equipped by {gLa{Gur{ce{wl{cin{x.\n\r",ch);
}

#endif
/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument )
{
  AFFECT_DATA af, *paf;
  int time;
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Nochannel whom?", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( ch != victim && get_trust( victim ) >= get_trust( ch ) )
  {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  if (is_number(argument))
  {
    time = atoi(argument);

    if (IS_NOCHANNELED(victim))
      if (is_affected(victim, gsn_nochannel))
      {
        for (paf = victim->affected; paf; paf= paf->next)
        {
          if (paf->where == TO_PENALTY && paf->bitvector == PEN_NOCHANNELS)
          {
            time += paf->duration;
          }
        }
      }
    remove_affect(victim, TO_PENALTY, PEN_NOCHANNELS);
    REMOVE_BIT(victim->pen_flags, PEN_NOCHANNELS);
    af.where     = TO_PENALTY;
    af.type     = gsn_nochannel;
    af.level     = victim->level;
    af.duration  = time;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = PEN_NOCHANNELS;
    affect_to_char( victim, &af );
    send_to_char("You have been nochanneled by the gods for horrible infractions.\n\r",victim);
    printf_to_char(ch, "Nochanneled for %d ticks.\n\r", time);
    return;
  }

  if ( IS_NOCHANNELED(victim) )
  {
    REMOVE_BIT(victim->pen_flags, PEN_NOCHANNELS);
    send_to_char( "The gods have restored your channel priviliges.\n\r",
                  victim );
    send_to_char( "NOCHANNELS removed.\n\r", ch );
    remove_affect(victim, TO_PENALTY, PEN_NOCHANNELS);
    mprintf(sizeof(buf), buf,"%s restores channels to %s",capitalize( ch->name ),victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }
  else
  {
    SET_BIT(victim->pen_flags, PEN_NOCHANNELS);
    send_to_char( "The gods have revoked your channel priviliges.\n\r",
                  victim );
    send_to_char( "NOCHANNELS set.\n\r", ch );
    mprintf(sizeof(buf), buf,"%s revokes %s's channels.",capitalize( ch->name ),victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }

  return;
}


void do_smote(CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *vch;
  char *letter,*name;
  char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
  int matches = 0;

  if ( !IS_NPC(ch) && IS_SET(ch->chan_flags, CHANNEL_NOEMOTE) )
  {
    send_to_char( "You can't show your emotions.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  {
    send_to_char( "Emote what?\n\r", ch );
    return;
  }

  if (strstr(argument,ch->name) == NULL)
  {
    send_to_char("You must include your name in an smote.\n\r",ch);
    return;
  }

  printf_to_char(ch,"%s{x\n\r",argument);

  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
  {
    if (vch->desc == NULL || vch == ch)
      continue;

    if ((letter = strstr(argument,vch->name)) == NULL)
    {
      printf_to_char(vch,"%s{x\n\r",argument);
      continue;
    }

    strcpy(temp,argument);
    temp[strlen(argument) - strlen(letter)] = '\0';
    last[0] = '\0';
    name = vch->name;

    for (; *letter != '\0'; letter++)
    {
      if (*letter == '\'' && matches == strlen(vch->name))
      {
        strcat(temp,"r");
        continue;
      }

      if (*letter == 's' && matches == strlen(vch->name))
      {
        matches = 0;
        continue;
      }

      if (matches == strlen(vch->name))
      {
        matches = 0;
      }

      if (*letter == *name)
      {
        matches++;
        name++;
        if (matches == strlen(vch->name))
        {
          strcat(temp,"you");
          last[0] = '\0';
          name = vch->name;
          continue;
        }
        strncat(last,letter,1);
        continue;
      }

      matches = 0;
      strcat(temp,last);
      strncat(temp,letter,1);
      last[0] = '\0';
      name = vch->name;
    }

    send_to_char(temp,vch);
    send_to_char("\n\r",vch);
  }

  return;
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{

  if ( !IS_NPC(ch) )
  {
    smash_tilde( argument );

    if (argument[0] == '\0')
    {
      printf_to_char(ch,"Your poofin is %s{x\n\r",ch->pcdata->bamfin);
      return;
    }

    if ( strstr(argument,ch->name) == NULL &&
         ch->level < DEMI )
    {
      send_to_char("You must include your name.\n\r",ch);
      return;
    }

    strcat(argument,"{x");
    free_string( ch->pcdata->bamfin );
    ch->pcdata->bamfin = str_dup( argument, ch->pcdata->bamfin );

    printf_to_char(ch,"Your poofin is now %s{x\n\r",ch->pcdata->bamfin);
  }
  return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{

  if ( !IS_NPC(ch) )
  {
    smash_tilde( argument );

    if (argument[0] == '\0')
    {
      printf_to_char(ch,"Your poofout is %s{x\n\r",ch->pcdata->bamfout);
      return;
    }

    if ( strstr(argument,ch->name) == NULL &&
         ch->level < DEMI )
    {
      send_to_char("You must include your name.\n\r",ch);
      return;
    }

    strcat(argument,"{x");
    free_string( ch->pcdata->bamfout );
    ch->pcdata->bamfout = str_dup( argument , ch->pcdata->bamfout);

    printf_to_char(ch,"Your poofout is now %s{x\n\r",ch->pcdata->bamfout);
  }
  return;
}



void do_deny( CHAR_DATA *ch, char *argument )
{
  char
  arg[MAX_INPUT_LENGTH],
  arg2[MAX_INPUT_LENGTH],
  buf[MAX_STRING_LENGTH];
  int denytime=0;
  CHAR_DATA *victim;

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );
  if ( arg[0] == '\0' )
  {
    send_to_char( "Deny whom?\n\r", ch );
    return;
  }

  if (arg2[0] != '\0')
  {
    if (!is_number(arg2))
    {
      send_to_char("Syntax: Deny Person <NUMDAYS>", ch);
      return;
    }
    else
      denytime = atoi(arg2);
  }
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) )
  {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }

  if ( get_trust( victim ) >= get_trust( ch ) )
  {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  if (denytime)
  {
    victim->pcdata->denytime = denytime;
    printf_to_char(ch,"They are denied for %d days.\n\r", denytime);
  }
  else
  {
    if (is_clan(victim))
    {
      mprintf(sizeof(buf), buf, "%s loner",victim->name);
      do_function(ch, &do_guild, buf);
    }
    add_to_denied_file(victim->name);
  }
  SET_BIT(victim->act, PLR_DENY);
  send_to_char( "You are denied access!\n\r", victim );
  victim->pcdata->denied_time = current_time;
  mprintf(sizeof(buf), buf,"%s denies access to %s",capitalize( ch->name ),victim->name);
  wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  send_to_char( "OK.\n\r", ch );
  affect_strip( victim, gsn_charm_person );
  stop_fighting(victim,TRUE);
  save_char_obj(victim, FALSE);
  do_function(victim, &do_quit, "confirm" );

  return;
}
void do_undeny( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA d;
  DENIED_DATA *dnd;
  DENIED_DATA *olddnd;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int found = FALSE;
  CHAR_DATA *victim;
  bool load_char = FALSE;

  if (argument[0] == '\0' ||
      argument[0] == '/'  ||
      argument[0] == '\\' ||
      argument[0] == '.')
  {
    send_to_char("Undeny who?\n\r", ch);
    return;
  }
  smash_tilde(argument);
  argument = one_argument(argument, arg);
  for (dnd = olddnd = denied_list; dnd; dnd = dnd->next)
  {
    if (is_exact_name(capitalize(arg),dnd->name))
    {
      free_string(dnd->name);
      found = TRUE;
      send_to_char("That person is FOUND.\n\r",ch);
      if (dnd == denied_list)
        denied_list = dnd->next;
      olddnd->next = dnd->next;
    }
    else
      olddnd = dnd;
  }

  if (((victim = get_char_world(ch, arg)) != NULL) && (!IS_NPC(victim)))
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
  if (load_char == FALSE)
  {
    printf_to_char(ch,"No such player.\n\r");
    return;
  }
  REMOVE_BIT(victim->act, PLR_DENY);
  victim->pcdata->denytime = 0;
  mprintf(sizeof(buf), buf,"%s UN-denies access to %s",capitalize( ch->name ),victim->name);
  wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  if (found)
    reset_denied_file(DENIED_FILE);
  send_to_char( "OK.\n\r", ch );
  save_char_obj(victim, FALSE);
  nuke_pets(d.character,FALSE);
  free_char(d.character);
  return;
}

//New Disconnect from Winston added Dec 24, 2003.
void do_disconnect( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d, *dn;
  CHAR_DATA *victim;
  int toDisco = -1;

  one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {
    send_to_char( "Disconnect whom?\n\r", ch );
    send_to_char( "Syntax: disconnect #, playing, ansi, noplay, getname\n\r", ch);
    return;
  }

  if (!str_cmp( arg, "playing"))
    toDisco = CON_PLAYING;

  if (!str_cmp( arg, "ansi"))
    toDisco = CON_ANSI;

  if (!str_cmp(arg,"getname"))
    toDisco = CON_GET_NAME;

  if (toDisco > 0)
  {
    for ( d = descriptor_list; d ; d = dn )
    {
      dn = d->next;
      if (d->connected == toDisco && d != ch->desc)
        close_socket(d);
    }
    return;
  }

  if (!str_cmp( arg, "noplay"))
  {
    for ( d= descriptor_list; d; d = dn)
    {
      dn = d->next;
      if (d->connected != CON_PLAYING && d != ch->desc)
        close_socket(d);
    }
    return;
  }

  if (is_number(arg))
  {
    int desc;

    desc = atoi(arg);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
      if ( d->descriptor == desc )
      {
        close_socket( d );
        send_to_char( "Ok.\n\r", ch );
        return;
      }
    }
  }
  else
  {

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    if ( victim->desc == NULL )
    {
      act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
      return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
      if ( d == victim->desc )
      {
        close_socket( d );
        send_to_char( "Ok.\n\r", ch );
        return;
      }
    }
  }
  //  bug( "Do_disconnect: desc not found.", 0 );
  send_to_char( "Descriptor not found!\n\r", ch );
  return;
}

void do_echo( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA *d;

  if ( argument[0] == '\0' )
  {
    send_to_char( "Global echo what?\n\r", ch );
    return;
  }

  for ( d = descriptor_list; d; d = d->next )
  {
    if ( d->connected == CON_PLAYING )
    {
      if (get_trust(d->character) >= get_trust(ch))
        send_to_char( "{W[{CGlobal{W]{x ",d->character);
      printf_to_char(d->character, "{W%s{x\n\r", argument);
    }
  }

  return;
}



void do_recho( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA *d;

  if ( argument[0] == '\0' )
  {
    send_to_char( "Local echo what?\n\r", ch );

    return;
  }

  for ( d = descriptor_list; d; d = d->next )
  {
    if ( d->connected == CON_PLAYING
         &&   d->character->in_room == ch->in_room )
    {
      if (get_trust(d->character) >= get_trust(ch))
        send_to_char( "local> ",d->character);
      printf_to_char(d->character,"{x%s{x\n\r", argument);
    }
  }

  return;
}

void do_zecho(CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA *d;

  if (argument[0] == '\0')
  {
    send_to_char("Zone echo what?\n\r",ch);
    return;
  }

  for (d = descriptor_list; d; d = d->next)
  {
    if (d->connected == CON_PLAYING
        &&  d->character->in_room != NULL && ch->in_room != NULL
        &&  d->character->in_room->area == ch->in_room->area)
    {
      if (get_trust(d->character) >= get_trust(ch))
        send_to_char("zone> ",d->character);
      printf_to_char(d->character,"{x%s{x\n\r", argument);
    }
  }
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument(argument, arg);

  if ( argument[0] == '\0' || arg[0] == '\0' )
  {
    send_to_char("Personal echo what?\n\r", ch);
    return;
  }

  if  ( (victim = get_char_world(ch, arg) ) == NULL )
  {
    send_to_char("Target not found.\n\r",ch);
    return;
  }

  if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
    send_to_char( "personal> ",victim);

  printf_to_char(victim,"{x%s\n\r",argument);
  printf_to_char(ch,"{x%s\n\r",argument);
}

ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  strip_string(arg);
  if ( is_number(arg) )
    return get_room_index( atoi( arg ) );

  if ( ( victim = get_char_world_ordered( ch, arg, "chars" ) ) != NULL )
    return victim->in_room;

  if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
    return obj->in_room;

  return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' )
  {
    send_to_char( "Transfer whom (and where)?\n\r", ch );
    return;
  }

  if ( !str_cmp( arg1, "all" ) )
  {
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
      if ( d->connected == CON_PLAYING
           &&   d->character != ch
           &&   d->character->in_room != NULL
           &&   can_see( ch, d->character ) )
      {
        char buf[MAX_STRING_LENGTH];
        mprintf( sizeof(buf), buf, "%s %s", d->character->name, arg2 );
        do_function(ch, &do_transfer, buf );
      }
    }
    return;
  }

  /*
   * Thanks to Grodyn for the optional location parameter.
   */
  if ( arg2[0] == '\0' )
  {
    location = ch->in_room;
  }
  else
  {
    if ( ( location = find_location( ch, arg2 ) ) == NULL )
    {
      send_to_char( "No such location.\n\r", ch );
      return;
    }

    if ( !is_room_owner(ch,location) && room_is_private( location )
         &&  get_trust(ch) < MAX_LEVEL)
    {
      send_to_char( "That room is private right now.\n\r", ch );
      return;
    }
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( victim->in_room == NULL )
  {
    send_to_char( "They are in limbo.\n\r", ch );
    return;
  }

  if ( victim->fighting != NULL )
    stop_fighting( victim, TRUE );
  act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_WIZ_ROOM );
  move_to_room( victim, location );
  act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_WIZ_ROOM );
  if ( ch != victim )
    act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
  do_function(victim, &do_look, "auto" );
  send_to_char( "Ok.\n\r", ch );
}



void do_at( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  ROOM_INDEX_DATA *original;
  OBJ_DATA *on;
  CHAR_DATA *wch;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' || argument[0] == '\0' )
  {
    send_to_char( "At where what?\n\r", ch );
    return;
  }

  if ( ( location = find_location( ch, arg ) ) == NULL )
  {
    send_to_char( "No such location.\n\r", ch );
    return;
  }

  if (!is_room_owner(ch,location) && room_is_private( location )
      &&  get_trust(ch) < MAX_LEVEL)
  {
    send_to_char( "That room is private right now.\n\r", ch );
    return;
  }

  original = ch->in_room;
  on = ch->on;
  move_to_room( ch, location );
  interpret( ch, argument );

  /*
   * See if 'ch' still exists before continuing!
   * Handles 'at XXXX quit' case.
   */
  for ( wch = char_list; wch; wch = wch->next )
  {
    if ( wch == ch )
    {
      OBJ_DATA *obj;

      move_to_room( ch, original );
      /* See if on still exists before setting ch->on back to it. */
      for ( obj = original->contents; obj; obj = obj->next_content )
      {
        if ( obj == on )
        {
          ch->on = on;
          break;
        }
      }
      break;
    }
  }
  return;
}



void do_goto( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *location;
  CHAR_DATA *rch;
  int count = 0;

  if ( argument[0] == '\0' )
  {
    send_to_char( "Goto where?\n\r", ch );
    return;
  }
  if ( ( location = find_location( ch, argument ) ) == NULL )
  {
    send_to_char( "No such location.\n\r", ch );
    return;
  }

  count = 0;

  for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
    count++;

  if ( ( !is_room_owner(ch,location) && room_is_private(location)
  &&     count > 1  )
  &&     get_trust(ch) < MAX_LEVEL)
  {
    send_to_char( "That room is private right now.\n\r", ch );
    return;
  }

  if ( ch->fighting != NULL )
    stop_fighting( ch, TRUE );

  if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
    act("$t",ch,ch->pcdata->bamfout,NULL,TO_WIZ_ROOM);
  else
    act("$n leaves in a swirling mist.",ch,NULL,NULL,TO_WIZ_ROOM);

  move_to_room( ch, location );

  if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
    act("$t",ch,ch->pcdata->bamfin,NULL,TO_WIZ_ROOM);
  else
    act("$n appears in a swirling mist.",ch,NULL,NULL,TO_WIZ_ROOM);

  do_function(ch, &do_look, "auto" );
  return;
}

void do_violate( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *location;

  if ( argument[0] == '\0' )
  {
    send_to_char( "Goto where?\n\r", ch );
    return;
  }

  if ( ( location = find_location( ch, argument ) ) == NULL )
  {
    send_to_char( "No such location.\n\r", ch );
    return;
  }

  if (!room_is_private( location ))
  {
    send_to_char( "That room isn't private, use goto.\n\r", ch );
    return;
  }

  if ( ch->fighting != NULL )
    stop_fighting( ch, TRUE );

  if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
    act("$t",ch,ch->pcdata->bamfout,NULL,TO_WIZ_ROOM);
  else
    act("$n leaves in a swirling mist.",ch,NULL,NULL,TO_WIZ_ROOM);

  move_to_room( ch, location );

  if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
    act("$t",ch,ch->pcdata->bamfin,NULL,TO_WIZ_ROOM);
  else
    act("$n appears in a swirling mist.",ch,NULL,NULL,TO_WIZ_ROOM);

  do_function(ch, &do_look, "auto" );
  return;
}


/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char *string;

  string = one_argument(argument,arg);

  if (arg[0] == '\0')
  {
    send_to_char("Syntax:\n\r",ch);
    send_to_char("  vnum obj <name>\n\r",ch);
    send_to_char("  vnum mob <name>\n\r",ch);
    send_to_char("  vnum skill <skill or spell>\n\r",ch);
    return;
  }

  if (!str_cmp(arg,"obj"))
  {
    do_function(ch, &do_ofind,string);
    return;
  }

  if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
  {
    do_function(ch, &do_mfind,string);
    return;
  }

  if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
  {
    do_function(ch, &do_slookup,string);
    return;
  }
  /* do both */
  do_function(ch, &do_mfind,argument);
  do_function(ch, &do_ofind,argument);
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
  extern int top_mob_index;
  char arg[MAX_INPUT_LENGTH];
  MOB_INDEX_DATA *pMobIndex;
  int vnum;
  BUFFER *buffer=0;
  char buf[MSL];
  int nMatch;
  bool fAll;
  bool found;

  one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {
    send_to_char( "Find whom?\n\r", ch );
    return;
  }

  fAll    = FALSE;    /* !str_cmp( arg, "all" ); */
  found    = FALSE;
  nMatch    = 0;

  /*
   * Yeah, so iterating over all vnum's takes 10,000 loops.
   * Get_mob_index is fast, and I don't feel like threading another link.
   * Do you?
   * -- Furey
   */
  for ( vnum = 0; nMatch < top_mob_index; vnum++ )
  {
    if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
    {
      nMatch++;
      if ( fAll || is_name( argument, pMobIndex->player_name ) )
      {
        if (!found)
        {
          buffer = new_buf();
        }
        found = TRUE;

        mprintf(sizeof(buf), buf, "[%5d] %s\n\r",
                pMobIndex->vnum, pMobIndex->short_descr
               );
        add_buf(buffer, buf);
      }
    }
  }

  if ( !found )
    send_to_char( "No mobiles by that name.\n\r", ch );
  else
  {
    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);
  }
  return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
  extern int top_obj_index;
  char arg[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  BUFFER *buffer=0;
  char buf[MSL];
  int vnum;
  int nMatch;
  bool fAll;
  bool found;

  one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {
    send_to_char( "Find what?\n\r", ch );
    return;
  }

  fAll    = FALSE;    /* !str_cmp( arg, "all" ); */
  found    = FALSE;
  nMatch    = 0;

  /*
   * Yeah, so iterating over all vnum's takes 10,000 loops.
   * Get_obj_index is fast, and I don't feel like threading another link.
   * Do you?
   * -- Furey
   */
  for ( vnum = 0; nMatch < top_obj_index; vnum++ )
  {
    if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
    {
      nMatch++;
      if ( fAll || is_name( argument, pObjIndex->name ) )
      {
        if (!found)
        {
          buffer = new_buf();
        }
        found = TRUE;
        mprintf(sizeof(buf), buf, "[%5d] %s\n\r",
                pObjIndex->vnum, pObjIndex->short_descr
               );
        add_buf(buffer,buf);
      }
    }
  }

  if ( !found )
    send_to_char( "No objects by that name.\n\r", ch );
  else
  {
    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);
  }

  return;
}


void do_owhere(CHAR_DATA *ch, char *argument )
{
  char buf[MAX_INPUT_LENGTH*3];
  char arg1[MIL], arg2[MIL];
  char isare[5];
  BUFFER *buffer;
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  char **pObjStrShow;
  int  *pObjNumShow;
  bool *pObjWhere;
  bool bObjWhere = FALSE, use_level = FALSE;;
  bool fCombine  = FALSE, found     = FALSE;
  int  nShow     = 0,     iShow     = 0;
  int  count     = 0;
  int  level     = 0,     vnum      = 0;
  int  number    = 0,     max_found = 100;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0')
  {
    send_to_char( "owhere [level]\n\r", ch );
    send_to_char( "owhere [keyword]\n\r", ch );
    send_to_char( "owhere vnum [vnum]\n\r", ch );
    return;
  }

  buffer = new_buf();

  if ( is_number( arg1 ) )
  {
    use_level = TRUE;
    level = atoi( arg1 );
  }

  if ( !IS_NULLSTR(arg2) && ( !str_prefix(arg1,"vnum") ) )
  {
    if ( is_number( arg2 ) )
      vnum = atoi( arg2 );
    else
    {
      send_to_char( "owhere vnum [vnum]\n\r", ch );
      return;
    }
  }

  pObjStrShow = alloc_mem( max_found * sizeof( char *) );
  pObjWhere   = alloc_mem( max_found * sizeof( bool  ) );
  pObjNumShow = alloc_mem( max_found * sizeof(  int  ) );

  for ( obj = object_list; obj; obj = obj->next )
  {
    count++;

    if ( ( use_level && ( obj->level != level ) )
         || ( vnum && ( obj->pIndexData->vnum != vnum ) )
         || ( !vnum && !use_level && !is_name( arg1, obj->name ) )
         || ( !can_see_obj( ch, obj ) ) )
      continue;

    found = TRUE;

    for ( in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj ); // look in containers

    if (IS_SET( obj->extra_flags, ITEM_PLURAL ))
    {
      strcpy(isare,"are");
    }
    else
    {
      strcpy(isare,"is");
    }

    if ( in_obj->carried_by
         && in_obj->carried_by
         && in_obj->carried_by->in_room )
    {
      if ( can_see( ch, in_obj->carried_by )
           ||   IS_LINKDEAD( in_obj->carried_by ) )
      {
        mprintf(sizeof(buf), buf, "[%5d] %s %s carried by %s [Room %d]\n\r",
                obj->pIndexData->vnum, isare,
                obj->short_descr, PERS( in_obj->carried_by, ch ),
                in_obj->carried_by->in_room->vnum );
      }
      else // I dunno - Taeloch
      {
        mprintf(sizeof(buf), buf, "[%5d] %s %s carried by someone [Room %d]\n\r",
                obj->pIndexData->vnum, isare,
                obj->short_descr, in_obj->carried_by->in_room->vnum );
      }
    }
    else if ( in_obj->in_room  && in_obj->in_room ) // Taeloch saw this line and said "WTF" out loud.  WTF???  Afraid to remove one tho...
    {
      mprintf(sizeof(buf), buf, "[%5d] %s %s in %s [Room %d]\n\r",
              obj->pIndexData->vnum, isare,
              obj->short_descr, in_obj->in_room->name,
              in_obj->in_room->vnum );
    }
    else
    {
      // How does this happen?  An item in a container in a pit in a dark room, no holylight?  no, then HOW?  Can't reproduce bug.
      mprintf(sizeof(buf), buf, "[%5d] %s %s somewhere\n\r",
              obj->pIndexData->vnum, obj->short_descr, isare );
    }

    /*
     * Look for duplicates, case sensitive.
     * Matches tend to be near end so run loop backwords.
     */
    fCombine = FALSE;
    for ( iShow = nShow - 1; iShow >= 0; iShow-- )
    {
      if ( pObjWhere[iShow] == bObjWhere
           && !strcmp( pObjStrShow[iShow], buf ) )
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
      number++;
    }

    if (number >= max_found)
      break;
  }

  for ( iShow = 0; iShow < nShow; iShow++ )
  {
    if ( pObjStrShow[iShow][0] == '\0' )
    {
      free_string( pObjStrShow[iShow] );
      continue;
    }

    bprintf(buffer, "(%3d) %s", pObjNumShow[iShow],pObjStrShow[iShow]);
    /* loop room */
    free_string( pObjStrShow[iShow] );
  }
  free_mem( pObjStrShow );
  free_mem( pObjWhere   );
  free_mem( pObjNumShow );

  if ( !found )
    if (vnum) send_to_char( "No objects found with that VNUM.\n\r", ch );
    else send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
  else
    page_to_char( buf_string( buffer ), ch );

  free_buf( buffer );
}

void do_mwhere( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  char arg1[MIL];
  char arg2[MIL];
  BUFFER *buffer;
  CHAR_DATA *victim;
  bool found;
  int count = 0;
  bool use_level= FALSE;
  int level =0;
  int vnum  =0;

  argument = one_argument(argument, arg1);

  if ( arg1[0] == '\0' )
  {
    DESCRIPTOR_DATA *d;

    /* show characters logged */

    buffer = new_buf();
    for (d = descriptor_list; d; d = d->next)
    {
      if ( d->character && d->connected == CON_PLAYING
           &&  d->character->in_room && can_see( ch, d->character ) )
      {
        victim = d->character;
        count++;
        if ( d->original  )
          mprintf(sizeof(buf), buf,
                  "%3d)[%5d] %s (in the body of %s) is in %s [%d]\n\r",
                  count,
                  IS_NPC( victim ) ? victim->pIndexData->vnum : 0,
                  d->original->name, victim->short_descr,
                  victim->in_room->name, victim->in_room->vnum );
        else
        {
          mprintf(sizeof(buf), buf,
                  "%3d)[%5d] %s is in %s [%d]\n\r",
                  count,
                  IS_NPC( victim ) ? victim->pIndexData->vnum : 0,
                  victim->name, victim->in_room->name,
                  victim->in_room->vnum );
        }
        add_buf(buffer,buf);
      }
    }

    page_to_char( buf_string( buffer ), ch );
    free_buf( buffer );
    return;
  }

  argument = one_argument(argument, arg2);

  if ( is_number( arg1 ) )
  {

    use_level = TRUE;
    level = atoi( arg1 );
  }
  else
  {
    if ( !IS_NULLSTR( arg1 ) && !str_prefix(arg1,"vnum") )
    {
      if ( is_number( arg2 ) )
      {
        vnum = atoi( arg2 );
      }
      else
      {
        send_to_char( "mwhere vnum [vnum]\n\r", ch );
        return;
      }
    }
  }

  found = FALSE;
  buffer = new_buf();
  for ( victim = char_list; victim; victim = victim->next )
  {
    if (IS_NPC( victim ) )
    {
      strncpy_color(buf1, victim->short_descr, 28, ' ', TRUE );
    }
    else
    {
      strncpy_color(buf1, victim->name, 28, ' ', TRUE );
    }

    if ( use_level )
    {
      if ( victim->in_room && victim->level == level)
      {
        found = TRUE;
        count++;
        mprintf(sizeof(buf), buf, "%3d)[%5d] %-28s [%5d] %s\n\r",
                count,
                IS_NPC( victim ) ? victim->pIndexData->vnum : 0,
                IS_NPC( victim ) ? buf1 : victim->name,
                victim->in_room->vnum,
                victim->in_room->name );
        add_buf( buffer, buf );
      }
    }
    else
    {
      if ( ( victim->in_room
             &&     is_name( arg1, victim->name ) )
           ||   ( ( IS_NPC( victim )
                    &&       victim->pIndexData->vnum == vnum ) ) )
      {
        found = TRUE;
        count++;
        mprintf(sizeof(buf), buf, "%3d)[%5d] %-28s [%5d] %s\n\r",
                count,
                IS_NPC( victim ) ? victim->pIndexData->vnum : 0,
                IS_NPC( victim ) ? buf1 : victim->name,
                victim->in_room->vnum,
                victim->in_room->name );
        add_buf( buffer, buf );

      }
      /*else
      {
        if ( IS_NPC( victim )
        && victim->in_room
        && ( victim->pIndexData->vnum == vnum ) )
        {
          found = TRUE;
          count++;
          mprintf(sizeof(buf), buf, "%3d)[%5d] %-28s [%5d] %s\n\r",
              count,
              victim->pIndexData->vnum,
              buf1,
              victim->in_room->vnum,
              victim->in_room->name );
          add_buf( buffer, buf );
        }*/

      //}
    }
  }

  if ( !found )
  {
    if (vnum) act( "You didn't find a mobile with VNUM $T.", ch, NULL, arg2, TO_CHAR );
    else act( "You didn't find any $T.", ch, NULL, arg1, TO_CHAR );
  }
  else
  {
    /*    if ( ch->lines == 0 )
      send_to_char( buffer, ch );
    else
    */
    page_to_char( buf_string( buffer ), ch );
  }
  free_buf( buffer );

  return;
}

void do_mhere( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  BUFFER *buffer;
  CHAR_DATA *victim;
  bool found;
  int count = 0;

  found = FALSE;
  buffer = new_buf();

  for ( victim = char_list; victim; victim = victim->next )
  {
    if ( IS_NPC( victim ) && victim->in_room
         && victim->in_room->area->vnum == ch->in_room->area->vnum )
    {
      if ( argument[0] == '\0' )
      {
        found = TRUE;
        count++;
        mprintf(sizeof(buf), buf, "%3d)[%5d] %-28s [%5d] %s\n\r",
                count,
                victim->pIndexData->vnum,
                victim->short_descr,
                victim->in_room->vnum,
                victim->in_room->name );
        add_buf( buffer, buf );
      }
      else
      {
        if ( is_name( argument, victim->name ) )
          found = TRUE;
        count++;
        mprintf(sizeof(buf), buf, "%3d)[%5d] %-28s [%5d] %s\n\r",
                count,
                victim->pIndexData->vnum,
                victim->short_descr,
                victim->in_room->vnum,
                victim->in_room->name );
        add_buf( buffer, buf );
      }
    }
  }

  if ( !found )
    act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );

  else
    page_to_char( buf_string( buffer ), ch );

  free_buf( buffer );
  return;
}

void do_reboo( CHAR_DATA *ch, char *argument )
{
  send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
  return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  extern bool merc_down;
  DESCRIPTOR_DATA *d,*d_next;
  CHAR_DATA *vch,*vch_next;

  if (ch->invis_level < LEVEL_HERO)
  {
    mprintf(sizeof(buf), buf, "Reboot by %s.", ch->name );
    do_function(ch, &do_echo, buf );
  }

  for ( vch = player_list; vch; vch = vch_next )
  {
    vch_next = vch->next_player;
    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    save_char_obj( ch, FALSE );
  }

  for ( d = descriptor_list; d; d = d_next )
  {
    d_next = d->next;
    vch = d->original ? d->original : d->character;
    save_char_obj( vch, FALSE );
    close_socket( d );
  }
  merc_down = TRUE;
  return;
}



void do_shutdow( CHAR_DATA *ch, char *argument )
{
  send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
  return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  extern bool merc_down;
  DESCRIPTOR_DATA *d,*d_next;
  CHAR_DATA *vch;

  if ( ch->invis_level < LEVEL_HERO )
    mprintf(sizeof(buf), buf, "Shutdown by %s.", ch->name );
  append_file( ch, SHUTDOWN_FILE, buf );
  strcat( buf, "\n\r" );
  if ( ch->invis_level < LEVEL_HERO )
    do_function( ch, &do_echo, buf );
  merc_down = TRUE;
  for ( d = descriptor_list; d; d = d_next)
  {
    d_next = d->next;
    vch = d->original ? d->original : d->character;
    save_char_obj( vch, FALSE );
    close_socket( d );
  }
  return;
}

void do_protect( CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;

  if ( argument[0] == '\0' )
  {
    send_to_char("Protect whom from snooping?\n\r",ch);
    return;
  }

  if ( ( victim = get_char_world( ch, argument ) ) == NULL )
  {
    send_to_char("You can't find them.\n\r",ch);
    return;
  }

  if ( IS_SET( victim->pen_flags, PEN_SNOOP_PROOF ) )
  {
    act_new( "$N is no longer snoop-proof.",
             ch, NULL, victim, TO_CHAR, POS_DEAD );
    send_to_char( "Your snoop-proofing was just removed.\n\r", victim );
    REMOVE_BIT( victim->pen_flags, PEN_SNOOP_PROOF );
  }
  else
  {
    act_new( "$N is now snoop-proof.",
             ch, NULL, victim, TO_CHAR, POS_DEAD );
    send_to_char( "You are now immune to snooping.\n\r", victim );
    SET_BIT( victim->pen_flags, PEN_SNOOP_PROOF );
  }
}



void do_snoop( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Snoop whom?\n\r", ch );
    return;
  }

//  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  if ( !str_cmp( arg, "self" ) )
    victim = ch;
  else if ( ( victim = get_char_world_ordered( ch, arg, "chars" ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( victim->desc == NULL )
  {
    send_to_char( "No descriptor to snoop.\n\r", ch );
    return;
  }

  if ( victim == ch )
  {
    send_to_char( "Cancelling all snoops.\n\r", ch );

    mprintf( sizeof(buf), buf, "%s stops being such a snoop.",
             capitalize( ch->name ) );
    wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
      if ( d->snoop_by == ch->desc )
        d->snoop_by = NULL;
    }
    return;
  }

  if ( victim->desc->snoop_by != NULL )
  {
    send_to_char( "Busy already.\n\r", ch );
    return;
  }

  if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room
      &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
  {
    send_to_char("That character is in a private room.\n\r",ch);
    return;
  }

  if ( get_trust( victim ) >= get_trust( ch )
       ||   IS_SET(victim->pen_flags,PEN_SNOOP_PROOF))
  {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  if ( ch->desc != NULL )
  {
    for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
    {
      if ( d->character == victim || d->original == victim )
      {
        send_to_char( "No snoop loops.\n\r", ch );
        return;
      }
    }
  }

  victim->desc->snoop_by = ch->desc;

  mprintf(sizeof(buf), buf,"%s starts snooping on %s",
          capitalize( ch->name ),
          (IS_NPC(victim) ? victim->short_descr : victim->name));
  wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
  send_to_char( "Ok.\n\r", ch );
  return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *och;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Switch into whom?\n\r", ch );
    return;
  }

  if ( ch->desc == NULL )
    return;

  if ( ch->desc )
    if ( ch->desc->editor )
    {
      send_to_char( "You must leave OLC before switching.\n\r", ch );
      return;
    }

  if ( IS_SET(ch->active_flags, ACTIVE_HAS_SWITCHED))
  {
    send_to_char( "You are already switched.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( victim == ch )
  {
    send_to_char( "Ok.\n\r", ch );
    return;
  }

  if (!IS_NPC(victim))
  {
    send_to_char("You can only switch into mobiles.\n\r",ch);
    return;
  }

  if (!is_room_owner(ch,victim->in_room)
      && ch->in_room != victim->in_room
      && room_is_private(victim->in_room)
      && !IS_TRUSTED(ch,IMPLEMENTOR))
  {
    send_to_char("That character is in a private room.\n\r",ch);
    return;
  }

  if ( victim->desc != NULL )
  {
    send_to_char( "Character in use.\n\r", ch );
    return;
  }

  if (IS_AFK(ch))
  {
    send_to_char("I do not care if you are an IMM. Cannot switch while AFK!\n\r",ch);
    return;
  }
  if ( ch->desc->original )
  {
    och = ch->desc->original;
    if ( IS_NPC( ch ) && ch->prompt )
    {
      free_string( ch->prompt );
      ch->prompt = &str_empty[0];
    }
  }
  else
    och = ch;

  mprintf(sizeof(buf), buf,"%s switches into %s",capitalize( ch->name ),victim->short_descr);
  wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
  SET_BIT(och->active_flags, ACTIVE_HAS_SWITCHED);
  ch->desc->character = victim;
  ch->desc->original  = och;
  victim->desc        = ch->desc;
  ch->desc            = NULL;
  /* change communications to match */
  if (ch->prompt != NULL)
  {
#if MEMDEBUG
    free_string(victim->memdebug_prompt);
    victim->memdebug_prompt = str_dup(ch->prompt, victim->memdebug_prompt);
#endif
    free_string(victim->prompt);
    victim->prompt = str_dup(och->prompt, victim->prompt);
  }
  if (IS_SET(och->act, PLR_COLOUR))
  {
    SET_BIT(victim->act, PLR_COLOUR);
    send_to_char("Switched your Act to have color.  Is this true?\n\r",victim);
  }
  if ( IS_NPC( victim ) )
  {
    victim->comm_flags    = och->comm_flags;
    victim->chan_flags    = och->chan_flags;
    victim->pen_flags    = och->pen_flags;
    victim->lines    = och->lines;
  }
  send_to_char( "Ok.\n\r", victim );
  return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  if ( ch->desc == NULL )
    return;

  if ( ch->desc == NULL )
    return;

  if ( ch->desc->original == NULL )
  {
    send_to_char( "You aren't switched.\n\r", ch );
    return;
  }

  send_to_char(
    "You return to your original body. Type replay to see any missed tells.\n\r",
    ch );
  if (ch->prompt != NULL)
  {
#if MEMDEBUG
    free_string(ch->memdebug_prompt);
    ch->memdebug_prompt = NULL;
#endif
    free_string(ch->prompt);
    ch->prompt = NULL;
  }

  REMOVE_BIT(ch->desc->original->active_flags, ACTIVE_HAS_SWITCHED);
  mprintf(sizeof(buf), buf,"%s returns from %s.",capitalize( ch->name ),ch->short_descr);
  wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
  ch->desc->character       = ch->desc->original;
  ch->desc->original        = NULL;
  ch->desc->character->desc = ch->desc;
  ch->desc                  = NULL;
  return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
  if (IS_TRUSTED(ch,GOD)
      || (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
      || (IS_TRUSTED(ch,DEMI)        && obj->level <= 10 && obj->cost <= 500)
      || (IS_TRUSTED(ch,ANGEL)    && obj->level <=  5 && obj->cost <= 250)
      || (IS_TRUSTED(ch,AVATAR)   && obj->level ==  0 && obj->cost <= 100))
    return TRUE;
  else
    return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
  OBJ_DATA *c_obj, *t_obj;


  for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
  {
    if (obj_check(ch,c_obj))
    {
      t_obj = create_object(c_obj->pIndexData,0);
      clone_object(c_obj,t_obj);
      obj_to_obj(t_obj,clone);
      recursive_clone(ch,c_obj,t_obj);
    }
  }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *mob = NULL;
  OBJ_DATA  *obj = NULL;
  int num, i;


  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Syntax:\n\r", ch );
    send_to_char( "  clone [#*]object [object]\n\r", ch );
    send_to_char( "  clone [#*]mobile [mobile]\n\r", ch );
    return;
  }

  num = mult_argument( arg, arg2 );

  if ( num >= 50 )
  {
    send_to_char( "Max clone 50 at a time: setting to 50.\n\r", ch );
    num = 50;
  }

  if ( num < 1 )
    num = 1;

  if ( arg2[0] == '\0' )
  {
    send_to_char( "Syntax:\n\r", ch );
    send_to_char( "  clone [#*]object [object]\n\r", ch );
    send_to_char( "  clone [#*]mobile [mobile]\n\r", ch );
    return;
  }

  if (!str_prefix(arg,"object"))
  {
    obj = get_obj_here(ch,argument);
    if (obj == NULL)
    {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
  }
  else if (!str_prefix(arg,"mobile"))
  {
    mob = get_char_room(ch,argument);
    if (mob == NULL)
    {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
  }
  else /* find both */
  {
    mob = get_char_room(ch,argument);
    obj = get_obj_here(ch,argument);
    if ((mob == NULL) && (obj == NULL))
    {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
  }

  /* clone an object */
  if (obj != NULL)
  {
    OBJ_DATA *clone = NULL;

    if (!obj_check(ch,obj))
    {
      send_to_char("Your powers are not great enough for such a task.\n\r",ch);
      return;
    }

    for ( i = 0; i < num; i++ )
    {
      clone = create_object(obj->pIndexData,0);
      clone_object(obj,clone);
      if (obj->carried_by != NULL)
        obj_to_char(clone,ch);
      else
        obj_to_room(clone,ch->in_room);
      recursive_clone(ch,obj,clone);
    }

    if (num > 1)
    {
      mprintf( sizeof(buf), buf, "$n has created %d*$ps.", num );
      act(buf,ch,clone,NULL,TO_ROOM);
      mprintf( sizeof(buf), buf, "You clone %d*$ps.", num);
      act(buf,ch,clone,NULL,TO_CHAR);
      mprintf( sizeof(buf), buf, "%s clones %d*%ss.",capitalize( ch->name ), num, obj->short_descr );
      wiznet(buf,ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    }
    else
    {
      act("$n has created $p.",ch,clone,NULL,TO_ROOM);
      act("You clone $p.",ch,clone,NULL,TO_CHAR);
      mprintf( sizeof(buf), buf, "%s clones %s.", capitalize( ch->name ), obj->short_descr );
      wiznet(buf,ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    }

    return;
  }
  else if (mob != NULL)
  {
    CHAR_DATA *clone = NULL;
    OBJ_DATA *new_obj;
    char buf[MAX_STRING_LENGTH];

    if (!IS_NPC(mob))
    {
      send_to_char("You can only clone mobiles.\n\r",ch);
      return;
    }

    if ( ( mob->level > 20 && !IS_TRUSTED( ch, GOD      ) )
         ||   ( mob->level > 10 && !IS_TRUSTED( ch, IMMORTAL ) )
         ||   ( mob->level >  5 && !IS_TRUSTED( ch, DEMI     ) )
         ||   ( mob->level >  0 && !IS_TRUSTED( ch, ANGEL    ) )
         ||                        !IS_TRUSTED( ch, AVATAR   ) )
    {
      send_to_char("Your powers are not great enough for such a task.\n\r", ch );
      return;
    }

    for ( i = 0; i < num; i++ )
    {
      clone = create_mobile( mob->pIndexData );
      clone_mobile( mob, clone );

      for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
      {
        if (obj_check(ch,obj))
        {
          new_obj = create_object(obj->pIndexData,0);
          clone_object(obj,new_obj);
          recursive_clone(ch,obj,new_obj);
          obj_to_char(new_obj,clone);
          new_obj->wear_loc = obj->wear_loc;
        }
      }
      char_to_room(clone,ch->in_room);
    }
    if (num > 1)
    {
      mprintf( sizeof(buf), buf, "$n has created %d*$Ns.", num );
      act(buf,ch,NULL,clone,TO_ROOM);
      mprintf( sizeof(buf), buf, "You clone %d*$Ns.", num);
      act(buf,ch,NULL,clone,TO_CHAR);
      mprintf( sizeof(buf), buf, "%s clones %d*%ss.",capitalize( ch->name ), num, clone->short_descr );
      wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    }
    else
    {
      act("$n has created $N.",ch,NULL,clone,TO_ROOM);
      act("You clone $N.",ch,NULL,clone,TO_CHAR);
      mprintf(sizeof(buf), buf,"%s clones %s.",capitalize( ch->name ),clone->short_descr);
      wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    }

    return;
  } // mob
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char arg4[MAX_INPUT_LENGTH];
  char buf[MIL];
  char buf1[MIL];
  int i = 0, num = 1;
  int count = 0;
  char *name = NULL;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Syntax:\n\r", ch );
    send_to_char( "  load [#*]mob <vnum>\n\r", ch );
    send_to_char( "  load [#*]obj <vnum> <level>\n\r", ch );
    return;
  }

  num = mult_argument( arg, arg4 );

  if ( num >= 50 )
  {
    send_to_char( "Max load 50 at a time, setting num to 50.\n\r", ch );
    num = 50;
  }
  count = num;
  if ( arg4[0] == '\0' )
  {
    send_to_char( "Syntax:\n\r", ch );
    send_to_char( "  load [#*]mob <vnum>\n\r", ch );
    send_to_char( "  load [#*]obj <vnum> <level>\n\r", ch );
    return;
  }

  if ( !str_cmp( arg4, "mob" ) || !str_cmp( arg4, "char" ) )
  {
    for ( i = 1; i <= num; i++ )
      name = do_mload( ch, argument );
    if ( !name )
    {
      send_to_char( "Unable to find that Mob.\n\r", ch );
      send_to_char( "Syntax: load [#*]mob <vnum>\n\r", ch );
      return;
    }
    mprintf(sizeof(buf), buf, " created %s %s mob",
            numcpy( buf1, count ), name );
    if ( count > 1 ) strcat( buf, "s" );
    strcat( buf, "." );
    mprintf(sizeof(buf1), buf1, "$n has%s", buf );
    act( buf1, ch, NULL, NULL, TO_ROOM );
    mprintf(sizeof(buf1), buf1, "%s has%s", capitalize( ch->name ), buf );
    wiznet( buf1, ch, NULL, WIZ_LOAD, WIZ_SECURE, get_trust( ch ) );
    printf_to_char( ch, "You%s\n\r", buf );
    return;
  }

  if ( !str_cmp( arg4, "obj" ) )
  {
    for ( i = 1; i <= num; i++ )
      name = do_oload( ch, argument );
    if ( !name )
    {
      send_to_char( "Unable to find that Object.\n\r", ch );
      send_to_char( "Syntax: load [#*]obj <vnum> <level>\n\r", ch );
      return;
    }
    mprintf(sizeof(buf), buf, " created %s %s object",
            numcpy( buf1, count ), name );
    if ( count > 1 ) strcat( buf, "s" );
    strcat( buf, "." );
    mprintf(sizeof(buf1), buf1, "$n has%s", buf );
    act( buf1, ch, NULL, NULL, TO_ROOM );
    mprintf(sizeof(buf1), buf1, "%s has%s", capitalize( ch->name ), buf );
    wiznet( buf1, ch, NULL, WIZ_LOAD, WIZ_SECURE, get_trust( ch ) );
    printf_to_char( ch, "You%s\n\r", buf );
    return;
  }
  /* echo syntax */
  do_function( ch, &do_load, "" );
}


char *do_mload( CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' || !is_number(arg) )
  {
    return NULL;
  }

  if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
  {
    return NULL;
  }

  victim = create_mobile( pMobIndex );
  char_to_room( victim, ch->in_room );
  victim->reset_room = ch->in_room;

  return ( victim->short_descr );
}



char *do_oload( CHAR_DATA *ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;
  int level;

  argument = one_argument( argument, arg1 );
  one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || !is_number( arg1 ) )
  {
    return NULL;
  }

  level = get_trust( ch ); /* default */

  if ( arg2[0] != '\0' )  /* load with a level */
  {
    if ( !is_number( arg2 ) )
      return NULL;

    level = atoi(arg2);
    if (level < 0 || level > get_trust( ch ) )
      return NULL;
  }

  if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    return NULL;

  obj = create_object( pObjIndex, level );
  if ( CAN_WEAR(obj, ITEM_TAKE) )
    obj_to_char( obj, ch );
  else
    obj_to_room( obj, ch->in_room );
  return ( obj->short_descr );
}

void do_purge( CHAR_DATA *ch, char *argument )
{
  char arg[MIL];
  char arg2[MIL];
  char buf[MSL];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  DESCRIPTOR_DATA *d;
  bool override = FALSE;

  argument = one_argument( argument, arg );

  /* allow avatar purges? */
  if ( (ch->level < L4 )
       &&   !IS_BUILDER(ch,ch->in_room->area ) )
  {
    send_to_char( "You can only purge in your own areas at your level.\n\r", ch );
    return;
  }

  if (!strcmp(arg, "override"))
  {
    strcpy(arg,"");
    if (ch->level >= 108)
      override = TRUE;
  }

  if ( arg[0] == '\0' )
  {
    purge_room(ch->in_room, override);
    act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
    send_to_char( "Ok.\n\r", ch );
    return;
  }

  if (!str_prefix( arg, "object" ))
  {
    one_argument( argument, arg2 );
    obj = get_obj_here( ch, arg2 );

    if ( obj == NULL )
    {
      send_to_char( "That item is not here.\n\r", ch );
      return;
    }

    for ( victim = ch->in_room->people; victim; victim = victim->next_in_room )
    {
      if ( IS_IMMORTAL( victim )
           && ( ch != victim ) )
        printf_to_char(victim,"%s purges %s.\n\r",ch->name,obj->short_descr);
    }

    act( "You purge $p.", ch, obj, NULL, TO_CHAR);

    if ( obj->carried_by != NULL )
      obj_from_char( obj );
    extract_obj( obj );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( !IS_NPC(victim) )
  {
    if (ch == victim)
    {
      send_to_char("Ho ho ho.\n\r",ch);
      return;
    }

    if (get_trust(ch) <= get_trust(victim))
    {
      send_to_char("Maybe that wasn't a good idea...\n\r",ch);
      printf_to_char(victim,"%s tried to purge you!\n\r",ch->name);
      return;
    }

    if (ch->level < L4)
    {
      send_to_char("You can't do that at your level!!!\n\r",ch);
      return;
    }

    //act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);

    if (victim->level > LEVEL_CHANNEL)
      save_char_obj( victim, FALSE );
    d = victim->desc;
    extract_char( victim, TRUE );
    if ( d != NULL )
      close_socket( d );

    sprintf( buf, "%s disintegrates %s.",
             ch->name,
             victim->name );
    wiznet( buf, NULL, NULL, WIZ_SECURE, 0, get_trust(ch) );

    return;
  }

  //act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
  act( "You purge $N.", ch, NULL, victim, TO_CHAR);
  extract_char( victim, TRUE );
  return;
}



void do_advance( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int level;
  int iLevel;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
  {
    send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
  {
    send_to_char( "That player is not here.\n\r", ch);
    return;
  }

  if ( IS_NPC(victim) )
  {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }

  if ( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL + 1 )
  {
    printf_to_char(ch, "Level must be 1 to %d.\n\r",MAX_LEVEL );
    return;
  }

  if ( level > get_trust( ch ) )
  {
    send_to_char( "Limited to your trust level.\n\r", ch );
    return;
  }
  /* Winston has all power */
  if ( (victim->level >= get_trust(ch)) &&
       !is_name(ch->name, "WinstonHeeHeeHee"))
  {
    send_to_char("You cannot advance or lower someone higher or equal to yourself.\n\r",ch);
    return;
  }
  /*
   * Lower level:
   *   Reset to level 1.
   *   Then raise again.
   *   Currently, an imp can lower another imp.
   *   -- Swiftest
   */
  victim->pcdata->degrading =0;
  if ( level <= victim->level )
  {
    int temp_prac;

    /* Remove holylight bug */
    if (IS_SET(victim->act, PLR_HOLYLIGHT))
      REMOVE_BIT(victim->act,PLR_HOLYLIGHT);
    if (IS_SET(victim->wiznet, WIZ_ON))
      REMOVE_BIT(victim->wiznet,WIZ_ON);
    send_to_char( "Lowering a player's level!\n\r", ch );
    send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim );
    temp_prac = victim->practice;
    victim->level    = 1;
    victim->exp      = 0; // exp_per_level(victim,victim->pcdata->points);
    victim->max_hit  = 10;
    victim->max_mana = 100;
    victim->max_move = 100;
    victim->practice = 0;
    victim->hit      = victim->max_hit;
    victim->mana     = victim->max_mana;
    victim->move     = victim->max_move;
    advance_level( victim, TRUE );
    victim->practice = temp_prac;
  }
  else
  {
    send_to_char( "Raising a player's level!\n\r", ch );
    send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
  }

  for ( iLevel = victim->level ; iLevel < level; iLevel++ )
  {
    victim->level += 1;
    advance_level( victim,TRUE);
  }
  printf_to_char(victim,"You are now level %d.\n\r",victim->level);
  if (victim->pcdata->clanowe_level > 0 && victim->pcdata->clanowe_level <= victim->level)
  {
    victim->pcdata->clanowe_level = 0;
    if (victim->pcdata->clanowe_dia == 0)
      victim->pcdata->clanowe_clan = 0;
  }
  victim->exp   = TNL( exp_per_level( victim,victim->pcdata->points ),
                       UMAX( 0, victim->level - 1 ) );
  victim->trust = 0;
  save_char_obj( victim, FALSE );
  return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int level;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
  {
    send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
  {
    send_to_char( "That player is not here.\n\r", ch);
    return;
  }

  if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
  {
    printf_to_char(ch, "Level must be 0 (reset) or 1 to %d.\n\r",MAX_LEVEL );
    return;
  }

  if ( level > get_trust( ch ) )
  {
    send_to_char( "Limited to your trust.\n\r", ch );
    return;
  }

  victim->trust = level;
  return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  char buf1[MIL];
  CHAR_DATA *victim;
  CHAR_DATA *vch;
  DESCRIPTOR_DATA *d;
  int         count = 0;

  one_argument( argument, arg );
  if ( arg[0] == '\0' || !str_cmp( arg, "room" ) )
  {
    /* cure room */

    for (vch = ch->in_room->people; vch; vch = vch->next_in_room)
    {
      if ( IS_NPC( vch ) )
        continue;

      count++;
      restore_char( vch );
      printf_to_char( vch, "%s has restored you.\n\r",
                      GODNAME( ch, vch ) );
    }

    mprintf(sizeof(buf), buf,
            "{g%s{w restored room {R%d{w ({g%s{w character%s{w){x.",
            ch->name, ch->in_room->vnum, numcpy( buf1, count ),
            count == 1 ? "" : "s" );
    wiznet( buf, NULL, NULL, WIZ_RESTORE, 0, get_trust( ch ) );

    send_to_char( "Room restored.\n\r", ch );
    return;

  }

  if ( get_trust( ch ) >=  MAX_LEVEL - 1 && !str_cmp( arg, "all" ) )
  {
    /* cure all */
    for ( d = descriptor_list; d; d = d->next )
    {
      victim = d->character;

      if ( victim == NULL || IS_NPC( victim ) )
        continue;
      restore_char( victim );
      if ( victim->in_room )
        printf_to_char( victim, "%s has restored you.\n\r",
                        GODNAME( ch, victim ) );
    }
    send_to_char( "All active players restored.\n\r", ch );
    return;
  }

  if ( get_trust( ch ) >= MAX_LEVEL - 1 && !str_cmp( arg, "area" ) )
  {
    /* Restore area */

    for ( vch = player_list; vch; vch = vch->next_player )
    {
      if ( vch == NULL || IS_NPC( vch ) )
        continue;

      if ( vch->in_room == NULL )
        continue;

      if ( IS_SET( vch->in_room->room_flags, ROOM_NOWHERE ) )
        continue;

      if ( !is_room_owner( ch, vch->in_room )
           &&    room_is_private( vch->in_room ) )
        continue;

      if ( IS_LINKDEAD( vch ) )
        continue;

      if ( vch->in_room->area != ch->in_room->area )
        continue;

      restore_char( vch );
      printf_to_char( vch, "%s has restored you.\n\r", GODNAME( ch, vch ) );
    }
    send_to_char( "All active players in area restored.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  restore_char(victim);
  printf_to_char( victim, "%s has restored you.\n\r", GODNAME( ch, victim ) );

  mprintf(sizeof(buf), buf, "%s restored %s",
          capitalize( ch->name ),
          IS_NPC( ch ) ? ch->short_descr : ch->name );
  wiznet( buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, get_trust( ch ) );
  send_to_char( "Ok.\n\r", ch );
  return;
}


void do_freeze( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Freeze whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) )
  {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }

  if ( get_trust( victim ) >= get_trust( ch ) )
  {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  if ( IS_SET(victim->act, PLR_FREEZE) )
  {
    REMOVE_BIT(victim->act, PLR_FREEZE);
    send_to_char( "You can play again.\n\r", victim );
    send_to_char( "FREEZE removed.\n\r", ch );
    mprintf(sizeof(buf), buf,"%s thaws %s.",capitalize( ch->name ),victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }
  else
  {
    SET_BIT(victim->act, PLR_FREEZE);
    send_to_char( "You can't do ANYthing!\n\r", victim );
    send_to_char( "FREEZE set.\n\r", ch );
    mprintf(sizeof(buf), buf,"%s puts %s in the deep freeze.",capitalize( ch->name ),victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }

  save_char_obj( victim , FALSE);

  return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Log whom?\n\r", ch );
    return;
  }

  if ( !str_cmp( arg, "all" ) )
  {
    if ( fLogAll )
    {
      fLogAll = FALSE;
      send_to_char( "Log ALL off.\n\r", ch );
    }
    else
    {
      fLogAll = TRUE;
      send_to_char( "Log ALL on.\n\r", ch );
    }
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) )
  {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }

  /*
   * No level check, gods can log anyone.
   */
  if ( IS_SET(victim->act, PLR_LOG) )
  {
    REMOVE_BIT(victim->act, PLR_LOG);
    send_to_char( "LOG removed.\n\r", ch );
  }
  else
  {
    SET_BIT(victim->act, PLR_LOG);
    send_to_char( "LOG set.\n\r", ch );
  }

  return;
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Noemote whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }


  if ( get_trust( victim ) >= get_trust( ch ) )
  {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  if ( IS_SET(victim->chan_flags, CHANNEL_NOEMOTE) )
  {
    REMOVE_BIT(victim->chan_flags, CHANNEL_NOEMOTE);
    send_to_char( "You can emote again.\n\r", victim );
    send_to_char( "NOEMOTE removed.\n\r", ch );
    mprintf(sizeof(buf), buf,"%s restores emotes to %s.",capitalize( ch->name ),victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }
  else
  {
    SET_BIT(victim->chan_flags, CHANNEL_NOEMOTE);
    send_to_char( "You can't emote!\n\r", victim );
    send_to_char( "NOEMOTE set.\n\r", ch );
    mprintf(sizeof(buf), buf,"%s revokes %s's emotes.",capitalize( ch->name ),victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }

  return;
}



void do_noshout( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Noshout whom?\n\r",ch);
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) )
  {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }

  if ( get_trust( victim ) >= get_trust( ch ) )
  {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  if ( IS_SET(victim->chan_flags, CHANNEL_NOSHOUT) )
  {
    REMOVE_BIT(victim->chan_flags, CHANNEL_NOSHOUT);
    send_to_char( "You can shout again.\n\r", victim );
    send_to_char( "NOSHOUT removed.\n\r", ch );
    mprintf(sizeof(buf), buf,"%s restores shouts to %s.",capitalize( ch->name ),victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }
  else
  {
    SET_BIT(victim->chan_flags, CHANNEL_NOSHOUT);
    send_to_char( "You can't shout!\n\r", victim );
    send_to_char( "NOSHOUT set.\n\r", ch );
    mprintf(sizeof(buf), buf,"%s revokes %s's shouts.",capitalize( ch->name ),victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }

  return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Notell whom?", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( get_trust( victim ) >= get_trust( ch ) )
  {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  if ( IS_SET(victim->chan_flags, CHANNEL_NOTELL) )
  {
    REMOVE_BIT(victim->chan_flags, CHANNEL_NOTELL);
    send_to_char( "You can tell again.\n\r", victim );
    send_to_char( "NOTELL removed.\n\r", ch );
    mprintf(sizeof(buf), buf,"%s restores tells to %s.",capitalize( ch->name ),victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }
  else
  {
    SET_BIT(victim->chan_flags, CHANNEL_NOTELL);
    send_to_char( "You can't tell!\n\r", victim );
    send_to_char( "NOTELL set.\n\r", ch );
    mprintf(sizeof(buf), buf,"%s revokes %s's tells.",capitalize( ch->name ),victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }

  return;
}



void do_peace( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *rch;

  for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
  {
    if ( rch->fighting != NULL )
    {
      stop_fighting( rch, TRUE );
    }
    if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
      REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    stop_hating(rch);
    rch->group_fight = 0;
  }

  send_to_char( "Ok.\n\r", ch );
  return;
}

void do_wizlock( CHAR_DATA *ch, char *argument )
{
  extern bool wizlock;
  wizlock = !wizlock;
  char buf[MAX_INPUT_LENGTH];

  if ( wizlock )
  {
    mprintf( sizeof(buf), buf, "%s has wizlocked the game.",
             capitalize( ch->name ));
    wiznet(buf,ch,NULL,0,0,0);
    send_to_char( "Game wizlocked.\n\r", ch );
  }
  else
  {
    mprintf( sizeof(buf), buf, "%s removes wizlock.",
             capitalize( ch->name ));
    wiznet(buf,ch,NULL,0,0,0);
    send_to_char( "Game un-wizlocked.\n\r", ch );
  }

  return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
  extern bool newlock;
  newlock = !newlock;
  char buf[MAX_INPUT_LENGTH];

  if ( newlock )
  {
    mprintf( sizeof(buf), buf, "%s locks out new characters.",
             capitalize( ch->name ));
    wiznet(buf,ch,NULL,0,0,0);
    send_to_char( "New characters have been locked out.\n\r", ch );
  }
  else
  {
    mprintf( sizeof(buf), buf, "%s allows new characters back in.",
             capitalize( ch->name ));
    wiznet(buf,ch,NULL,0,0,0);
    send_to_char( "Newlock removed.\n\r", ch );
  }

  return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  int sn;

  one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {
    send_to_char( "Lookup which skill or spell?\n\r", ch );
    return;
  }

  if ( !str_cmp( arg, "all" ) )
  {
    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
      if ( skill_table[sn].name == NULL )
        break;
      printf_to_char(ch, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
                     sn, skill_table[sn].slot, skill_table[sn].name );
    }
  }
  else
  {
    if ( ( sn = skill_lookup( arg ) ) < 0 )
    {
      send_to_char( "No such skill or spell.\n\r", ch );
      return;
    }

    printf_to_char(ch, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
                   sn, skill_table[sn].slot, skill_table[sn].name );
  }

  return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument(argument,arg);

  if (arg[0] == '\0')
  {
    send_to_char("Syntax:\n\r",ch);
    send_to_char("  set mob       <name> <field> <value>\n\r",ch);
    send_to_char("  set obj       <name> <field> <value>\n\r",ch);
    send_to_char("  set attribute <name> <field> <value>\n\r",ch);
    send_to_char("  set room      <room> <field> <value>\n\r",ch);
    send_to_char("  set skill     <name> <spell or skill> <value>\n\r",ch);
    return;
  }

  if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
  {
    do_function(ch, &do_mset,argument);
    return;
  }

  if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
  {
    do_function(ch, &do_sset,argument);
    return;
  }

  if (!str_prefix(arg,"object"))
  {
    do_function(ch, &do_oset,argument);
    return;
  }

  if (!str_prefix(arg,"room"))
  {
    do_function(ch, &do_rset,argument);
    return;
  }

  if (!str_prefix(arg,"attribute"))
  {
    do_function(ch, &do_aset,argument);
    return;
  }

  /* echo syntax */
  do_function(ch, &do_set,"");
}

void do_sset( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int value;
  int sn;
  bool fAll;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
  {
    send_to_char( "Syntax:\n\r",ch);
    send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
    send_to_char( "  set skill <name> all <value>\n\r",ch);
    send_to_char("   (use the name of the skill, not the number)\n\r",ch);
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) )
  {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }

  fAll = !str_cmp( arg2, "all" );
  sn   = 0;
  if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
  {
    send_to_char( "No such skill or spell.\n\r", ch );
    return;
  }

  /*
   * Snarf the value.
   */
  if ( !is_number( arg3 ) )
  {
    send_to_char( "Value must be numeric.\n\r", ch );
    return;
  }

  value = atoi( arg3 );
  if ( value < 0 || value > 100 )
  {
    send_to_char( "Value range is 0 to 100.\n\r", ch );
    return;
  }

  if ( fAll )
  {
    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
      if ( skill_table[sn].name != NULL )
        victim->pcdata->learned[sn]    = value;
    }
  }
  else
  {
    victim->pcdata->learned[sn] = value;
  }

  return;
}



void do_mset( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int value;

  smash_tilde( argument );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  strcpy( arg3, argument );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
  {
    send_to_char("Syntax: set char <name> <field> <value>\n\r\n\r",ch);
    send_to_char( "  {gCharacter Attributes:{x\n\r", ch );
    send_to_char( "    str, int, wis, dex, con, hp, mana, move, level, align, hours, prac, train,\n\r",    ch );
    send_to_char( "    class, race, group, gold, silver, locker, sex, experience\n\r\n\r", ch );
    send_to_char( "  {gDegrading Attributes:{x\n\r", ch );
    send_to_char( "    thirst, hunger, drunk, full\n\r\n\r",ch );
    send_to_char( "  {gClan Attributes:{x\n\r", ch );
    send_to_char( "    numclans, bounty, bailnum, diamond\n\r\n\r", ch);
    send_to_char( "  {gFragment Attributes:{x\n\r", ch );
    send_to_char( "    ruby_frag, sap_frag, emer_frag, dia_frag, ruby_counter,\n\r", ch);
    send_to_char( "    sap_counter, emer_counter, dia_counter, first_frag_lvl,\n\r", ch);
    send_to_char( "    second_frag_lvl, third_frag_lvl, fourth_frag_lvl, fifth_frag_lvl\n\r\n\r", ch );
    send_to_char( "  {gQuest Attributes:{x\n\r", ch );
    send_to_char( "    questpoint, questwait, questtime, quest_earned, quest_att, quest_comp,\n\r", ch);
    send_to_char( "    quest_quit, quest_failed, quest_streak, streak_needed\n\r", ch);
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  /* clear zones for mobs */
  victim->zone = NULL;

  /*
   * Snarf the value (which need not be numeric).
   */
  value = is_number( arg3 ) ? atoi( arg3 ) : -1;

  /*
   * Set something.
   */
  if ( !str_cmp( arg2, "str" ) )
  {
    if ( value < 3 || value > get_max_train(victim,STAT_STR) )
    {
      printf_to_char(ch,
                     "Strength range is 3 to %d\n\r.",
                     get_max_train(victim,STAT_STR));
      return;
    }

    victim->perm_stat[STAT_STR] = value;
    return;
  }

  if ( !str_cmp( arg2, "quest_streak") )
  {
    if ( IS_NPC( victim))
    {
      send_to_char( "Not on mobs.\n\r", ch );
      return;
    }

    if ( ch->level != (MAX_LEVEL + 1) )
    {
      send_to_char( "Only the owners can set quest streaks.\n\r", ch );
      return;
    }

    if ( value < 0 )
    {
      send_to_char( "Streak must be positive.\n\r", ch );
      return;
    }

    victim->questdata->streak = value;
    return;
  }

  if ( !str_cmp( arg2, "streak_needed") )
  {
    if ( IS_NPC( victim))
    {
      send_to_char( "Not on mobs.\n\r", ch );
      return;
    }

    if ( ch->level != MAX_LEVEL + 1 )
    {
      send_to_char( "Only the owners can set quest streaks.\n\r", ch );
      return;
    }

    if ( value > 100 || value < 0 )
    {
      send_to_char( "Streak Needed can be between 0 and 100.\n\r", ch );
      return;
    }

    victim->questdata->current_streak = value;
    return;
  }

  if ( !str_cmp( arg2, "security" ) )    /* OLC */
  {
    if ( IS_NPC(ch) )
    {
      send_to_char( "Si, claro.\n\r", ch );
      return;
    }

    if ( IS_NPC( victim ) )
    {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }

    if ( value > ch->pcdata->security || value < 0 )
    {
      if ( ch->pcdata->security != 0 )
      {
        printf_to_char(ch, "Valid security is 0-%d.\n\r",
                       ch->pcdata->security );
      }
      else
      {
        send_to_char( "Valid security is 0 only.\n\r", ch );
      }
      return;
    }
    victim->pcdata->security = value;
    return;
  }

  if ( !str_cmp( arg2, "bounty"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Mobs dont get bounties.\n\r",ch);
      return;
    }

    if (value < 0 || value > 100000)
    {
      send_to_char("0 - 100000 range.\n\r",ch);
      return;
    }
    victim->pcdata->bounty = value;
    return;
  }

  /* Fragment setting values */
  if ( !str_cmp( arg2, "ruby_frag"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Mobs don't collect fragments...duh.\n\r",ch);
      return;
    }

    if (value < 0 || value > 250000)
    {
      send_to_char("0-250000 range.\n\r",ch);
      return;
    }
    victim->ruby_fragment = value;
    return;
  }

  if ( !str_cmp( arg2, "ruby_counter"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Mobs don't collect fragments...duh.\n\r",ch);
      return;
    }

    if (value < 0 || value > 100)
    {
      send_to_char("You cannot set more than 100 counter points.\n\r",ch);
      return;
    }

    victim->ruby_counter = value;
    return;
  }

  if ( !str_cmp( arg2, "sap_frag"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Mobs don't collect fragments..duh.\n\r",ch);
      return;
    }
    if (value < 0 || value > 200000)
    {
      send_to_char("0-200000 range.\n\r",ch);
      return;
    }
    victim->sapphire_fragment = value;
    return;
  }

  if ( !str_cmp( arg2, "sap_counter"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Mobs don't collect fragments...duh.\n\r",ch);
      return;
    }

    if (value < 0 || value > 100)
    {
      send_to_char("You cannot set more than 100 counter points.\n\r",ch);
      return;
    }

    victim->sapphire_counter = value;
    return;
  }


  if ( !str_cmp( arg2, "emer_frag"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Mobs don't collect fragments...duh.\n\r",ch);
      return;
    }
    if (value < 0 || value > 150000)
    {
      send_to_char("0-150000 range.\n\r",ch);
      return;
    }
    victim->emerald_fragment = value;
    return;
  }

  if ( !str_cmp( arg2, "emer_counter"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Mobs don't collect fragments...duh.\n\r",ch);
      return;
    }

    if (value < 0 || value > 100)
    {
      send_to_char("You cannot set more than 100 counter points.\n\r",ch);
      return;
    }

    victim->emerald_counter = value;
    return;
  }

  if ( !str_cmp( arg2, "dia_frag"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Mobs don't collect fragments...duh.\n\r",ch);
      return;
    }
    if (value < 0 || value > 100000)
    {
      send_to_char("0-100000 range.\n\r",ch);
      return;
    }
    victim->diamond_fragment = value;
    return;
  }

  if ( !str_cmp( arg2, "dia_counter"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Mobs don't collect fragments...duh.\n\r",ch);
      return;
    }

    if (value < 0 || value > 100)
    {
      send_to_char("You cannot set more than 100 counter points.\n\r",ch);
      return;
    }

    victim->diamond_counter = value;
    return;
  }

  if ( !str_cmp( arg2, "first_frag_lvl") )
  {
    if (IS_NPC(victim))
    {
      send_to_char("Mobs don't collect fragments...duh.\n\r",ch);
      return;
    }

    if (value < 0 || value > 5)
    {
      send_to_char("Choices are 0-5.\n\r",ch);
      return;
    }

    victim->first_frag_level = value;
    return;
  }

  if ( !str_cmp( arg2, "second_frag_lvl") )
  {
    if (IS_NPC(victim))
    {
      send_to_char("Mobs don't collect fragments...duh.\n\r",ch);
      return;
    }

    if (value < 0 || value > 5)
    {
      send_to_char("Choices are 0-5.\n\r",ch);
      return;
    }

    victim->second_frag_level = value;
    return;
  }

  if ( !str_cmp( arg2, "third_frag_lvl") )
  {
    if (IS_NPC(victim))
    {
      send_to_char("Mobs don't collect fragments...duh.\n\r",ch);
      return;
    }

    if (value < 0 || value > 3)
    {
      send_to_char("Choices are 0-3.\n\r",ch);
      return;
    }

    victim->third_frag_level = value;
    return;
  }

  if ( !str_cmp( arg2, "fourth_frag_lvl") )
  {
    if (IS_NPC(victim))
    {
      send_to_char("Mobs don't collect fragments...duh.\n\r",ch);
      return;
    }

    if (value < 0 || value > 2)
    {
      send_to_char("Choices are 0-2.\n\r",ch);
      return;
    }

    victim->fourth_frag_level = value;
    return;
  }

  if ( !str_cmp( arg2, "fifth_frag_lvl") )
  {
    if (IS_NPC(victim))
    {
      send_to_char("Mobs don't collect fragments...duh.\n\r",ch);
      return;
    }

    if (value < 0 || value > 1)
    {
      send_to_char("Choices are 0-1.\n\r",ch);
      return;
    }

    victim->fifth_frag_level = value;
    return;
  }

  if ( !str_cmp( arg2, "numclans"))
  {
    if (value < 0 || value > 10)
    {
      send_to_char("0 - 10 range.\n\r",ch);
      return;
    }
    victim->pcdata->numclans = value;
    return;
  }

  if ( !str_cmp( arg2, "hours"))
  {
    if (value < 0 || value > 100000)
    {
      send_to_char("0 - 100000 range.\n\r",ch);
      return;
    }
    victim->played  = value * 3600;
    return;
  }

  if ( !str_cmp( arg2, "experience" ) )
  {
    if ( get_trust( ch ) < 109 )
    {
      send_to_char( "You are not powerful enough for this.\n\r", ch );
      return;
    }

    if ( value < 0 )
    {
      send_to_char( "Can only set them to a positive value.\n\r", ch );
      return;
    }

    victim->exp = value;
    return;
  }

  if ( !str_cmp( arg2, "int" ) )
  {
    if ( value < 3 || value > get_max_train(victim,STAT_INT) )
    {
      printf_to_char(ch,
                     "Intelligence range is 3 to %d.\n\r",
                     get_max_train(victim,STAT_INT));
      return;
    }

    victim->perm_stat[STAT_INT] = value;
    return;
  }

  if ( !str_cmp( arg2, "wis" ) )
  {
    if ( value < 3 || value > get_max_train(victim,STAT_WIS) )
    {
      printf_to_char(ch,
                     "Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
      return;
    }

    victim->perm_stat[STAT_WIS] = value;
    return;
  }

  if ( !str_cmp( arg2, "dex" ) )
  {
    if ( value < 3 || value > get_max_train(victim,STAT_DEX) )
    {
      printf_to_char(ch,
                     "Dexterity ranges is 3 to %d.\n\r",
                     get_max_train(victim,STAT_DEX));
      return;
    }

    victim->perm_stat[STAT_DEX] = value;
    return;
  }

  if ( !str_cmp( arg2, "con" ) )
  {
    if ( value < 3 || value > get_max_train(victim,STAT_CON) )
    {
      printf_to_char(ch,
                     "Constitution range is 3 to %d.\n\r",
                     get_max_train(victim,STAT_CON));
      return;
    }

    victim->perm_stat[STAT_CON] = value;
    return;
  }

  if ( !str_prefix( arg2, "sex" ) )
  {
    if ( value < 0 || value > 2 )
    {
      send_to_char( "Sex range is 0 to 2.\n\r", ch );
      return;
    }
    victim->sex = value;
    if (!IS_NPC(victim))
      victim->pcdata->true_sex = value;
    copy_roster_clannie(victim);
    return;
  }
  if ( !str_prefix( arg2, "bailnum" ) )
  {
    if ( value < 0 || value > 5 )
    {
      send_to_char( "Bailnum range is 0 to 5.\n\r", ch );
      return;
    }
    victim->num_bailed = value;
    return;
  }

  if ( !str_prefix( arg2, "class" ) )
  {
    int gameclass;

    if (IS_NPC(victim))
    {
      send_to_char("Mobiles have no class.\n\r",ch);
      return;
    }

    gameclass = class_lookup(arg3);
    if ( gameclass == -1 )
    {
      char buf[MAX_STRING_LENGTH];

      strcpy( buf, "Possible classes are: " );
      for ( gameclass = 0; gameclass < MAX_CLASS; gameclass++ )
      {
        if ( gameclass > 0 )
          strcat( buf, " " );
        strcat( buf, class_table[gameclass].name );
      }
      strcat( buf, ".\n\r" );

      send_to_char(buf,ch);
      return;
    }

    victim->gameclass = gameclass;
    return;
  }

  if ( !str_prefix( arg2, "level" ) )
  {
    if ( !IS_NPC(victim) )
    {
      send_to_char( "Not on PC's.\n\r", ch );
      return;
    }

    if ( value < 0 || value > MAX_LEVEL )
    {
      printf_to_char(ch, "Level range is 0 to %d.\n\r",MAX_LEVEL );
      return;
    }
    victim->level = value;
    return;
  }

  if ( !str_prefix( arg2, "gold" ) )
  {
    victim->gold = value;
    return;
  }

  if ( !str_prefix(arg2, "silver" ) )
  {
    victim->silver = value;
    return;
  }

  if ( !str_prefix( arg2, "quest_earned" ) )
  {
    victim->questdata->accum_points = value;
    return;
  }

  if ( !str_prefix( arg2, "quest_att" ) )
  {
    victim->questdata->attempt_num = value;
    return;
  }

  if ( !str_prefix( arg2, "quest_comp" ) )
  {
    if ( IS_NPC(victim) ) return;

    victim->questdata->comp_num = value;
    return;
  }

  if ( !str_prefix( arg2, "quest_quit" ) )
  {
    if ( IS_NPC( victim ) ) return;

    victim->questdata->quit_num = value;
    return;
  }

  if ( !str_prefix( arg2, "quest_failed" ) )
  {
    if ( IS_NPC(victim) ) return;

    victim->questdata->failed = value;
    return;
  }

  if ( !str_prefix( arg2, "hp" ) )
  {
    if ( value < -10 || value > 30000 )
    {
      send_to_char( "Hp range is -10 to 30,000 hit points.\n\r", ch );
      return;
    }
    victim->max_hit = value;
    if (!IS_NPC(victim))
      victim->pcdata->perm_hit = value;
    return;
  }

  if ( !str_prefix( arg2, "mana" ) )
  {
    if ( value < 0 || value > 30000 )
    {
      send_to_char( "Mana range is 0 to 30,000 mana points.\n\r", ch );
      return;
    }
    victim->max_mana = value;
    if (!IS_NPC(victim))
      victim->pcdata->perm_mana = value;
    return;
  }

  if ( !str_prefix( arg2, "move" ) )
  {
    if ( value < 0 || value > 30000 )
    {
      send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
      return;
    }
    victim->max_move = value;
    if (!IS_NPC(victim))
      victim->pcdata->perm_move = value;
    return;
  }

  if ( !str_prefix( arg2, "questpoints" ) )
  {
    if ( value < 0 || value > 40000 )
    {
      send_to_char(" qustpoint range is 0 to 30,000 quest points.\n\r", ch );
      return;
    }

    if (!IS_NPC(victim))
    {
      victim->questdata->curr_points = value;
      victim->questdata->accum_points += value;
    }

    return;
  }

  if ( !str_prefix(arg2, "questfailed") )
  {
    if ( IS_NPC( victim ) ) return;

    if ( value < 0 )
    {
      send_to_char( "Must be a positive value.\n\r", ch );
      return;
    }

    printf_to_char( ch, "You have set %s's failed number to %d\n\r",
                    victim->name, value );
    victim->questdata->failed = value;
    return;

  }

  if (!str_prefix( arg2, "questwait") )
  {
    if (IS_SET(victim->act, PLR_QUESTING) )
    {
      send_to_char( "Character is currently questing.\n\r", ch);
      return;
    }
    if ( value < 0 || value > 30 )
    {
      send_to_char( "The range for time between quests is 0 to 30 minutes.\n\r", ch);
      return;
    }
    if (!IS_NPC(victim))
    {
      victim->questdata->nextquest = value;
      printf_to_char(ch, "%s must now wait %d minutes before questing again.\n\r",
                     victim->name, value);
    }
    return;
  }

  if (!str_prefix( arg2, "questtime") )
  {
    if (!IS_SET(victim->act, PLR_QUESTING) )
    {
      send_to_char("Character has no current quest.\n\r", ch);
      return;
    }

    if ( value < 0 || value > 30 )
    {
      send_to_char( "Time to complete quest must be between 0 and 30 minutes.\n\r", ch);
      return;
    }

    if (!IS_NPC(victim))
    {
      victim->questdata->countdown = value;
      printf_to_char(ch, "%S now has %d minutes to complete their quest.\n\r",
                     victim->name, value);
    }
    return;
  }

  if ( !str_prefix( arg2, "practice" ) )
  {
    if ( value < 0 || value > 250 )
    {
      send_to_char( "Practice range is 0 to 250 sessions.\n\r", ch );
      return;
    }
    victim->practice = value;
    return;
  }

  if ( !str_prefix( arg2, "train" ))
  {
    if (value < 0 || value > 50 )
    {
      send_to_char("Training session range is 0 to 50 sessions.\n\r",ch);
      return;
    }
    victim->train = value;
    return;
  }

  if ( !str_prefix( arg2, "align" ) )
  {
    if ( value < -1000 || value > 1000 )
    {
      send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
      return;
    }
    align_change(victim,value, FALSE, TRUE);
    return;
  }

  if ( !str_prefix( arg2, "thirst" ) )
  {
    if ( IS_NPC(victim) )
    {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }

    if ( value < -1 || value > 100 )
    {
      send_to_char( "Thirst range is -1 to 100.\n\r", ch );
      return;
    }

    victim->pcdata->condition[COND_THIRST] = value;
    return;
  }

  if ( !str_prefix( arg2, "drunk" ) )
  {
    if ( IS_NPC(victim) )
    {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }

    if ( value < -1 || value > 100 )
    {
      send_to_char( "Drunk range is -1 to 100.\n\r", ch );
      return;
    }

    victim->pcdata->condition[COND_DRUNK] = value;
    return;
  }

  if ( !str_prefix( arg2, "full" ) )
  {
    if ( IS_NPC(victim) )
    {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }

    if ( value < -1 || value > 100 )
    {
      send_to_char( "Full range is -1 to 100.\n\r", ch );
      return;
    }

    victim->pcdata->condition[COND_FULL] = value;
    return;
  }

  if ( !str_prefix( arg2, "hunger" ) )
  {
    if ( IS_NPC(victim) )
    {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }

    if ( value < -1 || value > 100 )
    {
      send_to_char( "Full range is -1 to 100.\n\r", ch );
      return;
    }

    victim->pcdata->condition[COND_HUNGER] = value;
    return;
  }

  if (!str_prefix( arg2, "race" ) )
  {
    int race;

    race = race_lookup(arg3);

    if ( race == 0)
    {
      send_to_char("That is not a valid race.\n\r",ch);
      return;
    }

    if (!IS_NPC(victim) && !race_table[race].pc_race)
    {
      send_to_char("That is not a valid player race.\n\r",ch);
      return;
    }

    victim->race = race;
    return;
  }

  if (!str_prefix(arg2,"group"))
  {
    if (!IS_NPC(victim))
    {
      send_to_char("Only on NPCs.\n\r",ch);
      return;
    }
    victim->group = value;
    return;
  }

  if (!str_prefix(arg2,"locker"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on MOBs.\n\r",ch);
      return;
    }

    if (value < 0 || value > 200)
    {
      send_to_char("Full range is 0 to 200\n\r",ch);
      return;
    }
    victim->pcdata->locker_max = value;
    send_to_char("Locker max content set.\n\r",ch);
    return;
  }

  if (!str_prefix(arg2, "diamonds"))
  {
    if (IS_NPC(victim))
    {
      send_to_char("Not on MOBs.\n\r", ch);
      return;
    }

    if (value < 0)
    {
      send_to_char("The number of diamonds must be greater than or equal to zero.\n\r", ch);
      return;
    }

    victim->pcdata->donated_dia = value;
    copy_roster_clannie(victim);
    save_clan_info(CLAN_FILE);
    send_to_char("Diamond count set.\n\r", ch);
    return;
  }

  /*
   * Generate usage message.
   */
  do_function(ch, &do_mset, "" );
  return;
}

void do_aset( CHAR_DATA *ch, char *argument )
{
  char arg [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  bool valid = FALSE;
  int num;

  argument = one_argument( argument, arg ); // victim name

  if ( arg[0] == '\0' )
  {
    send_to_char( "Syntax:\n\r",ch);
    send_to_char( "  set attribute <name> <attribute> <value>\n\r", ch);
    return;
  }

  if ( ( ( victim = get_char_world( ch, arg ) ) == NULL )
       ||  IS_NPC(victim) )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }


  argument = one_argument( argument, arg ); // attribute name

  if ( arg[0] == '\0' )
  {
    send_to_char( "Syntax:\n\r",ch);
    send_to_char( "  set attribute <name> <attribute> <value>\n\r\n\r", ch);
    send_to_char( "Valid attributes are: Hair, Eyes, Skin, Height, Weight\n\r", ch);
    return;
  }

  /* *********************************************** */
  if (!str_prefix(arg,"eyes"))
  {
    argument = one_argument( argument, arg ); // attribute value
    if (arg[0] != '\0')
    {
      if (!strcmp(arg,"custom")
          &&  (ch->level > 105)
          &&  (argument[0] != '\0') )
      {
        valid = TRUE;
        free_string(victim->pcdata->eye_color);
        victim->pcdata->eye_color = str_dup(capitalize(argument), victim->pcdata->eye_color);
        printf_to_char(ch, "Eye color set.");
        return;
      }

      for (num=0; ((num < MAX_APPR) && (valid == FALSE)); num++)
      {
        if ( (pc_race_table[victim->race].eye_color[num] != NULL)
             &&   !str_prefix(arg, pc_race_table[victim->race].eye_color[num]) )
        {
          valid = TRUE;
          free_string(victim->pcdata->eye_color);
          victim->pcdata->eye_color = str_dup(pc_race_table[victim->race].eye_color[num], victim->pcdata->eye_color);
          printf_to_char(ch, "Eye color set.");
          return;
        }
      }
    }

    if (!valid)
    {
      printf_to_char(ch, "Valid eye colors for their race are: ");
      for (num=0; num < MAX_APPR; num++)
      {
        if (pc_race_table[victim->race].eye_color[num] != NULL)
          printf_to_char(ch, "%s", pc_race_table[victim->race].eye_color[num]);
        if ( ((num+1) < MAX_APPR)
             &&   (pc_race_table[victim->race].eye_color[num+1] != NULL) )
          printf_to_char(ch, ", ");
      }
      printf_to_char(ch, "{x\n\r");
      return;
    } // not found: list valid colors
  } // end eye color

  /* *********************************************** */
  else if (!str_prefix(arg,"hair"))
  {
    argument = one_argument( argument, arg ); // attribute value
    if (arg[0] != '\0')
    {
      if (!strcmp(arg,"custom")
          &&  (ch->level > 105)
          &&  (argument[0] != '\0') )
      {
        valid = TRUE;
        free_string(victim->pcdata->hair_color);
        victim->pcdata->hair_color = str_dup(capitalize(argument), victim->pcdata->hair_color);
        printf_to_char(ch, "Hair color set.");
        return;
      }

      for (num=0; ((num < MAX_APPR) && (valid == FALSE)); num++)
      {
        if ( (pc_race_table[victim->race].hair_color[num] != NULL)
             &&   !str_prefix(arg, pc_race_table[victim->race].hair_color[num]) )
        {
          valid = TRUE;
          free_string(victim->pcdata->hair_color);
          victim->pcdata->hair_color = str_dup(pc_race_table[victim->race].hair_color[num], victim->pcdata->hair_color);
          printf_to_char(ch, "Hair color set.");
          return;
        }
      }
    }

    if (!valid)
    {
      printf_to_char(ch, "Valid hair colors for their race are: ");
      for (num=0; num < MAX_APPR; num++)
      {
        if (pc_race_table[victim->race].hair_color[num] != NULL)
          printf_to_char(ch, "%s", pc_race_table[victim->race].hair_color[num]);
        if ( ((num+1) < MAX_APPR)
             &&   (pc_race_table[victim->race].hair_color[num+1] != NULL) )
          printf_to_char(ch, ", ");
      }
      printf_to_char(ch, "{x\n\r");
      return;
    } // not found: list valid colors
  } // end hair color

  /* *********************************************** */
  else if (!str_prefix(arg,"skin"))
  {
    argument = one_argument( argument, arg ); // attribute value
    if (arg[0] != '\0')
    {
      if (!strcmp(arg,"custom")
          &&  (ch->level > 105)
          &&  (argument[0] != '\0') )
      {
        valid = TRUE;
        free_string(victim->pcdata->skin_color);
        victim->pcdata->skin_color = str_dup(capitalize(argument), victim->pcdata->skin_color);
        printf_to_char(ch, "Skin color set.");
        return;
      }

      for (num=0; ((num < MAX_APPR) && (valid == FALSE)); num++)
      {
        if ( (pc_race_table[victim->race].skin_color[num] != NULL)
             &&   !str_prefix(arg, pc_race_table[victim->race].skin_color[num]) )
        {
          valid = TRUE;
          free_string(victim->pcdata->skin_color);
          victim->pcdata->skin_color = str_dup(pc_race_table[victim->race].skin_color[num], victim->pcdata->skin_color);
          printf_to_char(ch, "Skin color set.");
          return;
        }
      }
    }

    if (!valid)
    {
      printf_to_char(ch, "Valid skin colors for their race are: ");
      for (num=0; num < MAX_APPR; num++)
      {
        if (pc_race_table[victim->race].skin_color[num] != NULL)
          printf_to_char(ch, "%s", pc_race_table[victim->race].skin_color[num]);
        if ( ((num+1) < MAX_APPR)
             &&   (pc_race_table[victim->race].skin_color[num+1] != NULL) )
          printf_to_char(ch, ", ");
      }
      printf_to_char(ch, "{x\n\r");
      return;
    } // not found: list valid colors
  } // end skin color

  /* *********************************************** */
  else if (!str_prefix(arg,"height"))
  {
    argument = one_argument( argument, arg ); // attribute value

    if (!strcmp(arg,"custom")
        &&  (ch->level > 105)
        &&  (argument[0] != '\0')
        &&   is_number(argument))
    {
      victim->pcdata->height = atoi(argument);
      printf_to_char(ch, "Height set.");
      return;
    }

    if (!is_number(arg))
      num = 0;
    else
      num = atoi(arg);

    if ( (num < pc_race_table[victim->race].min_height)
         ||   (num > pc_race_table[victim->race].max_height) )
    {
      printf_to_char(ch, "The valid height range for their race is from %d to %d.\n\r",
                     pc_race_table[victim->race].min_height, pc_race_table[victim->race].max_height);
      return;
    }
    victim->pcdata->height = num;
    printf_to_char(ch, "Height set.");
    return;
  } // end height

  /* *********************************************** */
  else if (!str_prefix(arg,"weight"))
  {
    argument = one_argument( argument, arg ); // attribute value

    if (!strcmp(arg,"custom")
        &&  (ch->level > 105)
        &&  (argument[0] != '\0')
        &&   is_number(argument))
    {
      victim->pcdata->weight = atoi(argument);
      printf_to_char(ch, "Weight set.");
      return;
    }

    if (!is_number(arg))
      num = 0;
    else
      num = atoi(arg);

    if ( (victim->sex == SEX_FEMALE)
         &&   (victim->size >= SIZE_MEDIUM) )
    {
      if ( (num < pc_race_table[victim->race].min_weight)
           ||   (num > (pc_race_table[victim->race].max_weight - 50)) )
      {
        printf_to_char(ch, "The valid weight range for their race is from %d to %d.\n\r",
                       pc_race_table[victim->race].min_weight, (pc_race_table[victim->race].max_weight - 50));
        return;
      }
    }
    else
    {
      if ( (num < pc_race_table[victim->race].min_weight)
           ||   (num > pc_race_table[victim->race].max_weight) )
      {
        printf_to_char(ch, "The valid weight range for their race is from %d to %d.\n\r",
                       pc_race_table[victim->race].min_weight, pc_race_table[victim->race].max_weight);
        return;
      }
    }
    victim->pcdata->weight = num;
    printf_to_char(ch, "Weight set.");
    return;
  } // end weight

  else
  {
    send_to_char( "Syntax:\n\r",ch);
    send_to_char( "  set attribute <name> <attribute> <value>\n\r", ch);
  }
  return;
}


void do_string( CHAR_DATA *ch, char *argument )
{
  char type [MAX_INPUT_LENGTH];
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj=0;
  AFFECT_DATA *oaf;

  smash_tilde( argument );
  argument = one_argument( argument, type );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  strcpy( arg3, argument );

  if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
  {
    send_to_char("Syntax:\n\r",ch);
    send_to_char("  string char <name> <field> <string>\n\r",ch);
    send_to_char("    fields: name short long desc title spec\n\r",ch);
    send_to_char("  string obj  <name> <field> <string>\n\r",ch);
    send_to_char("    fields: name short long extended\n\r",ch);
    return;
  }

  if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
  {
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    /* clear zone for mobs */
    victim->zone = NULL;


    if ( !str_prefix( arg2, "name" ) )
    {
      if ( !IS_NPC(victim) )
      {
        send_to_char( "Not on PC's.\n\r", ch );
        return;
      }
      free_string( victim->name );
      victim->name = str_dup( arg3 , victim->name);
#if MEMDEBUG
      victim->memdebug_name = str_dup (arg3 , victim->memdebug_name);
      memdebug_check(victim,"do_string: string name");
      memdebug_check(ch,"do_string: string name");
#endif
      return;
    }

    if ( !str_prefix( arg2, "description" ) )
    {
      free_string(victim->description);
      strcat(arg3,"{x\n\r");
      victim->description = str_dup(arg3, victim->description);
      return;
    }

    if ( !str_prefix( arg2, "short" ) )
    {
      free_string( victim->short_descr );
      strcat(arg3,"{x");
      victim->short_descr = str_dup( arg3 , victim->short_descr);
      return;
    }

    if ( !str_prefix( arg2, "long" ) )
    {
      free_string( victim->long_descr );
      strcat(arg3,"{x\n\r");
      victim->long_descr = str_dup( arg3 , victim->long_descr);
      return;
    }

    if ( !str_prefix( arg2, "title" ) )
    {
      if ( IS_NPC(victim) )
      {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
      }
      strcat(arg3,"{x");
      set_title( victim, arg3 );
      return;
    }

    if ( !str_prefix( arg2, "spec" ) )
    {
      if ( !IS_NPC(victim) )
      {
        send_to_char( "Not on PC's.\n\r", ch );
        return;
      }

      if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
      {
        send_to_char( "No such spec fun.\n\r", ch );
        return;
      }

      return;
    }
  }

  if (!str_prefix(type,"object"))
  {
    /* string an obj */

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
      send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
      return;
    }

    /* DJR added to help prevent loss of restrings */
    if (IS_SET(obj->extra_flags, ITEM_BURN_PROOF) )
    {
      for ( oaf = obj->affected; oaf != NULL; oaf = oaf->next )
      {
        if (oaf->type == skill_lookup("fireproof"))
          affect_remove_obj(obj, oaf);
      }
    }
    SET_BIT( obj->extra_flags, ITEM_NOPURGE|ITEM_BURN_PROOF|ITEM_RESTRING );
    SET_BIT( obj->wear_flags, ITEM_NO_SAC );
    REMOVE_BIT( obj->extra_flags, ITEM_MELT_DROP );
    obj->cost = 0;
    /* string something */
    if ( !str_prefix( arg2, "name" ) )
    {
      free_string( obj->name );
      obj->name = str_dup( arg3 , obj->name);
      return;
    }

    if ( !str_prefix( arg2, "short" ) )
    {
      free_string( obj->short_descr );
      strcat(arg3,"{x");
      obj->short_descr = str_dup( arg3, obj->short_descr );
      return;
    }

    if ( !str_prefix( arg2, "long" ) )
    {
      free_string( obj->description );
      strcat(arg3,"{x");
      obj->description = str_dup( arg3, obj->description );
      return;
    }

    if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    {
      EXTRA_DESCR_DATA *ed;

      argument = one_argument( argument, arg3 );
      if ( argument == NULL )
      {
        send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
                      ch );
        return;
      }

      strcat(argument,"\n\r");

      ed = new_extra_descr();

      ed->keyword        = str_dup( arg3, ed->keyword  );
      ed->description    = str_dup( argument, ed->description );
      ed->next        = obj->extra_descr;
      obj->extra_descr    = ed;
      string_append( ch, &ed->description, APPEND_AREA, obj->pIndexData->area );
      return;
    }
  }


  /* echo bad use message */
  do_function(ch, &do_string,"");
}



void do_oset( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int64 value;

  smash_tilde( argument );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  strcpy( arg3, argument );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
  {
    send_to_char("Syntax:\n\r",ch);
    send_to_char("  set obj <object> <field> <value>\n\r",ch);
    send_to_char("  Field being one of:\n\r",                ch );
    send_to_char("    value0 value1 value2 value3 value4 value5 value6 (v0-v6)\n\r",    ch );
    send_to_char("    extra wear level weight cost timer owner\n\r",        ch );
    return;
  }

  if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
  {
    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    return;
  }

  /*
   * Snarf the value (which need not be numeric).
   */
  value = atoi( arg3 );

  /*
   * Set something.
   */
  if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
  {
    obj->value[0] = UMIN(100,value);
    return;
  }

  if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
  {
    obj->value[1] = value;
    return;
  }

  if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
  {
    obj->value[2] = value;
    return;
  }

  if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
  {
    obj->value[3] = value;
    return;
  }
  if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
  {
    obj->value[4] = value;
    return;
  }
  if ( !str_cmp( arg2, "value5" ) || !str_cmp( arg2, "v5" ) )
  {
    obj->value[5] = value;
    return;
  }
  if ( !str_cmp( arg2, "value6" ) || !str_cmp( arg2, "v6" ) )
  {
    obj->value[6] = value;
    return;
  }

  if ( !str_prefix( arg2, "extra" ) )
  {
    if ( ( value = flag_value( extra_flags, arg3 ) ) != NO_FLAG)
    {
      TOGGLE_BIT(obj->extra_flags, value);
    }
    return;
  }
  if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
  {
    obj->value[4] = value;
    return;
  }

  if ( !str_prefix( arg2, "extra" ) )
  {
    obj->extra_flags = value;
    return;
  }

  if ( !str_prefix( arg2, "wear" ) )
  {
    obj->wear_flags = value;
    return;
  }

  if ( !str_prefix( arg2, "level" ) )
  {
    obj->level = value;
    return;
  }

  if ( !str_prefix( arg2, "weight" ) )
  {
    obj->weight = value;
    return;
  }

  if ( !str_prefix( arg2, "cost" ) )
  {
    obj->cost = value;
    return;
  }

  if ( !str_prefix( arg2, "timer" ) )
  {
    obj->timer = value;
    return;
  }

  if ( !str_prefix( arg2, "owner" ) )
  {
    free_string(obj->owner);
    obj->owner = NULL;

    if (strcmp(arg3,"none"))
      obj->owner = str_dup(capitalize(arg3), obj->owner);

    return;
  }

  /*
   * Generate usage message.
   */
  do_function(ch, &do_oset, "" );
  return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  int value;

  smash_tilde( argument );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  strcpy( arg3, argument );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
  {
    send_to_char( "Syntax:\n\r",ch);
    send_to_char( "  set room <location> <field> <value>\n\r",ch);
    send_to_char( "  Field being one of:\n\r",            ch );
    send_to_char( "    flags sector\n\r",                ch );
    return;
  }

  if ( ( location = find_location( ch, arg1 ) ) == NULL )
  {
    send_to_char( "No such location.\n\r", ch );
    return;
  }

  if (!is_room_owner(ch,location) && ch->in_room != location
      &&  room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR))
  {
    send_to_char("That room is private right now.\n\r",ch);
    return;
  }

  /*
   * Snarf the value.
   */
  if ( !is_number( arg3 ) )
  {
    send_to_char( "Value must be numeric.\n\r", ch );
    return;
  }
  value = atoi( arg3 );

  /*
   * Set something.
   */
  if ( !str_prefix( arg2, "flags" ) )
  {
    location->room_flags    = value;
    return;
  }

  if ( !str_prefix( arg2, "sector" ) )
  {
    location->sector_type    = value;
    return;
  }

  /*
   * Generate usage message.
   */
  do_function(ch, &do_rset, "" );
  return;
}



#ifdef ZEPH

void clear_multiplay( void )
{
  CHAR_DATA        *vch;
  DESCRIPTOR_DATA    *d;

  for ( d = descriptor_list; d; d = d->next )
    REMOVE_BIT( d->misc_flags, MISC_DESC_MULTIPLAY );

  for ( vch = player_list; vch; vch = vch->next_player )
    REMOVE_BIT( vch->pcdata->status, STATUS_MULTIPLAY );
}

void update_multiplay( CHAR_DATA *ch, bool fIPAddr )
{
  CHAR_DATA        *vch, *vch_chk;
  DESCRIPTOR_DATA    *d, *d_chk;
  char        host1[256], host2[256];
  bool        found;

  clear_multiplay();

  for ( d = descriptor_list; d; d = d->next )
  {
    if ( CH(d) == NULL || !can_see( ch, CH(d) ) )
      continue;

    for ( d_chk = d->next; d_chk; d_chk = d_chk->next )
    {
      if ( CH(d_chk) == NULL || !can_see( ch, CH(d_chk) ) )
        continue;

      strcpy( host1, get_host_str( d, NULL, fIPAddr, TRUE ) );
      strcpy( host2, get_host_str( d_chk, NULL, fIPAddr, TRUE ) );
      if ( host1[0] != '\0' && !str_cmp( host1, host2 ) )
      {
        SET_BIT( d->misc_flags, MISC_DESC_MULTIPLAY );
        SET_BIT( CH(d)->pcdata->status, STATUS_MULTIPLAY );
        SET_BIT( d_chk->misc_flags, MISC_DESC_MULTIPLAY );
        SET_BIT( CH(d_chk)->pcdata->status, STATUS_MULTIPLAY );
        break;
      }
    }

    for ( vch = player_list; vch; vch = vch->next_player )
    {
      if ( IS_DELETED( vch ) || !can_see( ch, vch ) )
        continue;

      found = FALSE;
      for ( d_chk = descriptor_list; d_chk; d_chk = d_chk->next )
        if ( CH(d_chk) && !str_cmp( CH(d_chk)->name, vch->name ) )
        {
          found = TRUE;
          break;
        }

      if ( found )
        continue;

      strcpy( host1, get_host_str( d, NULL, fIPAddr, TRUE ) );
      strcpy( host2, get_host_str( NULL, vch, fIPAddr, TRUE ) );
      if ( host1[0] != '\0' && !str_cmp( host1, host2 ) )
      {
        SET_BIT( d->misc_flags, MISC_DESC_MULTIPLAY );
        SET_BIT( CH(d)->pcdata->status, STATUS_MULTIPLAY );
        SET_BIT( vch->pcdata->status, STATUS_MULTIPLAY );
        break;
      }
    }
  }

  for ( vch = player_list; vch; vch = vch->next_player )
  {
    if ( IS_DELETED( vch ) || !can_see( ch, vch ) )
      continue;

    for ( vch_chk = vch->next_player;
          vch_chk;
          vch_chk = vch_chk->next_player )
    {
      if ( IS_DELETED( vch_chk ) || !can_see( ch, vch_chk ) )
        continue;

      strcpy( host1, get_host_str( NULL, vch, fIPAddr, TRUE ) );
      strcpy( host2, get_host_str( NULL, vch_chk, fIPAddr, TRUE ) );
      if ( host1[0] != '\0' && !str_cmp( host1, host2 ) )
      {
        SET_BIT( vch->pcdata->status, STATUS_MULTIPLAY );
        SET_BIT( vch_chk->pcdata->status, STATUS_MULTIPLAY );
        break;
      }
    }
  }
}

void do_zsockets( CHAR_DATA *ch, char *argument )
{
  int         (*cmp_fn)( const void *a, const void *b ) = NULL;
  SOCKET_INDEX_DATA    *sd;
  CHAR_DATA        *vch;
  DESCRIPTOR_DATA    *d;
  BUFFER        *buffer;
  char        arg[MAX_INPUT_LENGTH];
  char        name[MAX_INPUT_LENGTH];
  char        tmp[MAX_INPUT_LENGTH];
  int         count = 0;
  int            mccp_count = 0;
  int            ld_count = 0;
  int         nShow = 0;
  int         iShow;
  bool        found;
  bool        fIPAddr = FALSE;

  argument = one_argument( argument, arg );

  if ( arg[0] != '\0' && !str_cmp( arg, "ip" ) )
  {
    argument = one_argument( argument, arg );
    fIPAddr = TRUE;
  }

  if ( arg[0] != '\0' && !str_prefix( "sort=", arg ) )
  {
    cmp_fn = get_socket_sort( ch, &do_sockets, arg );

    if ( !cmp_fn )
    {
      send_to_char( "Invalid sort type.\n\r", ch );
      return;
    }

    one_argument( argument, arg );
  }

  for ( d = descriptor_list; d; d = d->next )
  {
    if ( CH(d) && can_see( ch, CH(d) ) )
    {
      count++;
      if ( d->out_compress )
        mccp_count++;
    }
    else if ( CH(d) == NULL )
    {
      count++;
      if ( d->out_compress )
        mccp_count++;
    }
  }

  for ( vch = player_list; vch; vch = vch->next_player )
  {
    if ( IS_DELETED( vch ) || !can_see( ch, vch ) )
      continue;

    /* Don't duplicate what's been listed by descriptor. */
    found = FALSE;
    for ( d = descriptor_list; d; d = d->next )
      if ( CH(d) && !str_cmp( CH(d)->name, vch->name ) )
      {
        found = TRUE;
        break;
      }

    /* Linkdead characters. */
    if ( !found && IS_LINKDEAD( vch ) )
      count++, ld_count++;
  }

  if ( count == 0 )
  {
    send_to_char( "No players visible.\n\r", ch );
    return;
  }

  update_multiplay( ch, fIPAddr );

  sd = alloc_mem( count * sizeof( *sd ) );

  for ( d = descriptor_list; d; d = d->next )
  {
    if ( CH(d) && can_see( ch, CH(d) )
         &&   ( arg[0] == '\0' || is_name( arg, CH(d)->name ) ) )
    {
      sd[nShow].ch = CH(d);
      sd[nShow].d  = d;
      nShow++;
    }
    else if ( d->character == NULL && arg[0] == '\0' )
    {
      sd[nShow].ch = NULL;
      sd[nShow].d  = d;
      nShow++;
    }
  }

  for ( vch = player_list; vch; vch = vch->next_player )
  {
    if ( IS_DELETED( vch ) || !can_see( ch, vch )
         ||   ( arg[0] != '\0' && !is_name( arg, vch->name ) ) )
      continue;

    /* Don't duplicate what's been listed by descriptor. */
    found = FALSE;
    for ( d = descriptor_list; d; d = d->next )
      if ( CH(d) && !str_cmp( CH(d)->name, vch->name ) )
      {
        found = TRUE;
        break;
      }

    /* Linkdead characters. */
    if ( !found && IS_LINKDEAD( vch ) )
    {
      sd[nShow].ch = vch;
      sd[nShow].d  = NULL;
      nShow++;
    }
  }

  if ( cmp_fn && nShow > 1 )
    qsort( sd, nShow, sizeof( *sd ), cmp_fn );

  buffer = new_buf();

  bprintf( buffer,
           "#_Desc Connected    Idle   Name          %s#:\n\r",
           fIPAddr ? "IP Address" : "Host Name" );

  for ( iShow = 0; iShow < nShow; iShow++ )
  {
    d    = sd[iShow].d;
    vch    = sd[iShow].ch;
    if ( vch && d )
    {
      mprintf( sizeof(name), name, "%s%s",
               CH(d) ? CH(d)->name : "(none)",
               d->original ? "#R*#n" : "" );

      strncpyft_color( tmp, name, 13, ' ', TRUE );

      bprintf( buffer, "%3d%s %-12.12s %5d%s %-13s %s%s%s\n\r",
               d->descriptor,
               d->out_compress ? "#GC#n" : " ",
               flag_string( connected_flags, d->connected ),
               CH(d) ? CH(d)->timer : -1,
               CH(d) ? IS_SET( CH(d)->comm_flags, COMM_AFK )
         ? "#PA#n" : " " : " ",
               tmp,
               IS_SET( d->misc_flags, MISC_DESC_MULTIPLAY )
               ? "#r[#R" : " ",
               get_host_str( d, NULL, fIPAddr, !fIPAddr ),
               IS_SET( d->misc_flags, MISC_DESC_MULTIPLAY )
               ? "#r]#n" : "" );
    }
    else if ( vch )
    {
      strncpyft_color( tmp, LINKDEAD_STRING, 12, ' ', TRUE );

      bprintf( buffer, "%3s  %s %5d%s %-13s %s%s%s\n\r",
               "-",
               tmp,
               vch->timer,
               IS_AFK( vch ) ? "#PA#n" : " ",
               vch->name,
               IS_SET( vch->pcdata->status, STATUS_MULTIPLAY )
               ? "#r[#R" : " ",
               get_host_str( NULL, vch, fIPAddr, !fIPAddr ),
               IS_SET( vch->pcdata->status, STATUS_MULTIPLAY )
               ? "#r]#n" : "" );
    }
    else if ( d )
    {
      bprintf( buffer, "#A%3d%s %-12.12s %5d%s %-13s  %s#n\n\r",
               d->descriptor,
               d->out_compress ? "#GC#n" : " ",
               flag_string( connected_flags, d->connected ),
               d->connected == CON_GET_NAME ? d->repeat : 0,
               " ",
               "(null)",
               get_host_str( d, NULL, fIPAddr, !fIPAddr ) );
    }
  }

  if ( iShow > 0 )
  {
    bprintf( buffer, "%d player%s listed", iShow, iShow == 1 ? "" : "s" );
    if ( mccp_count > 0 )
      bprintf( buffer, ", %d using MCCP", mccp_count );
    if ( ld_count > 0 )
      bprintf( buffer, ", %d linkdead", ld_count );
    bstrcat( buffer, ".\n\r" );
    page_to_char( buf_string( buffer ), ch );
  }
  else if ( arg[0] == '\0' )
    send_to_char( "No players visible.\n\r", ch );
  else
    send_to_char( "No one by that name is connected.\n\r", ch );

  free_buf( buffer );
  free_mem( sd, count * sizeof( *sd ) );

  clear_multiplay();
}
#endif
void do_sockets( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA       *vch;
  DESCRIPTOR_DATA *d;
  int             count,count2,count3, count4;
  char *          st;
  char            s[100];
  char            idle[10];
  BUFFER    *buffer;

  count       = count2 = 0;
  count3=count4=0;
  buffer = new_buf();

  add_buf( buffer, "\n\r{W[Num ConnectState  Login  Idl  Cmp] Player        Host\n\r" );
  add_buf( buffer,"--------------------------------------------------------------------------{x\n\r");
  for ( d = descriptor_list; d; d = d->next )
  {
    if ( d->character && can_see( ch, d->character ) )
    {
      switch ( d->connected ) /* NB: You may need to edit the CON_ values */
      {
        case CON_PLAYING:
          st = " PLAYING    ";
          break;
        case CON_GET_NAME:
          st = " Get Name   ";
          break;
        case CON_GET_OLD_PASSWORD:
          st = " Old Passwd ";
          break;
        case CON_CONFIRM_NEW_NAME:
          st = " Check Name ";
          break;
        case CON_GET_NEW_PASSWORD:
          st = " New Passwd ";
          break;
        case CON_CONFIRM_NEW_PASSWORD:
          st = " Check Pass ";
          break;
        case CON_GET_NEW_RACE:
          st = " New Race   ";
          break;
        case CON_GET_NEW_SEX:
          st = " New Sex    ";
          break;
        case CON_GET_NEW_CLASS:
          st = " New Class  ";
          break;
        case CON_GET_ALIGNMENT:
          st = " New Align  ";
          break;
        case CON_DEFAULT_CHOICE:
          st = " Choice     ";
          break;
        case CON_GEN_GROUPS:
          st = " Customize  ";
          break;
        case CON_PICK_WEAPON:
          st = " Weapon     ";
          break;
        case CON_READ_IMOTD:
          st = " IMOTD      ";
          break;
        case CON_BREAK_CONNECT:
          st = " Linkdead   ";
          break;
        case CON_READ_MOTD:
          st = " MOTD       ";
          break;
        case CON_COPYOVER_RECOVER:
          st = " Copyover   ";
          break;
        case CON_ANSI:
          st = " Pick Ansi  ";
          break;
        case CON_ROLL_STATS:
          st = " Roll stats ";
          break;
        case CON_GET_STORY:
          st = " Pick Story ";
          break;
        case CON_GET_STORY2:
          st = " Read Story ";
          break;
        case CON_GET_NEWBIE:
          st = " Pick Newbie";
          break;
        case CON_GET_DISCLAIMER:
          st = " Disclaimer ";
          break;
        case CON_GET_CONFIRMATION:
          st = " Confirm    ";
          break;
        case CON_GET_ANSI_COLOR:
          st = " Get Color  ";
          break;
        case CON_PROMPT_DISCLAIMER:
          st = " Prompt Disc";
          break;
        case CON_NEW_CREATION:
          st = " Creation   ";
          break;
        default:
          st = " !UNKNOWN!  ";
          break;
      } // switch

      /* Format "login" value... */
      vch = d->original ? d->original : d->character;

      strftime( s, 100, "%I:%M%p", localtime( &vch->logon ) );
      if ( vch->timer > 0 )
        mprintf( sizeof(idle), idle, "{R%-4d{x", vch->timer );
      else
        mprintf( sizeof(idle), idle, "    " );

      if (d->character)
      {
        if (d->character->level < ch->level || IS_IMPLEMENTOR(ch))
        {
          count++;
          if (d->original)
          {
            if (IS_SET(vch->active_flags, ACTIVE_HAS_SWITCHED))
            {
              st = " {WSWITCHED   {C";
              count3++;
            }
          }

          if (d->out_compress)
            count2++;

          bprintf(buffer, "{W[{C%3d %s %7s %4s %2s {W]{c  %-12s %-32.52s{x\n\r",
                  d->descriptor,
                  st,
                  s,
                  idle,
                  (d->out_compress) ? "{YY{x" : "{RN{x",
                  ( d->original ) ? d->original->name : ( d->character )  ? d->character->name : "(None!)",
                  d->host );
        } // if lev<lev || IMP
      } // if d->char
    } // if desc && can_see
  } // for descriptor loop

  for (vch = char_list; vch; vch = vch->next)
  {
    if (IS_NPC(vch) || !can_see(ch,vch))
      continue;

    if (!vch->desc && (!(IS_SET(vch->active_flags, ACTIVE_HAS_SWITCHED))))
    {
      strftime( s, 100, "%I:%M%p", localtime( &vch->logon ) );
      if ( vch->timer > 0 )
        mprintf( sizeof(idle), idle, "{R%-4d{x", vch->timer );
      else
        mprintf( sizeof(idle), idle, "    " );

      bprintf(buffer, "{W[{C%3d %s {C%7s %4s %2s {W]{c  %-12s %-32.52s{x\n\r",
              -1,
              " {RLINKDEAD   {x",
              s,
              idle,
              "{RX{x",
              vch->name,
              "Dropped" );

      count4++;
    }
    continue;
  } // for char_list

  bprintf(buffer,
          "\n\r{WUser%s [%d] Compressed [%d] Switched [%d] linkdead [%d]{x\n\r",
          count == 1 ? "" : "s", count, count2, count3, count4);

  page_to_char(buf_string( buffer), ch );
  free_buf(buffer);
  return;
}


/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' || argument[0] == '\0' )
  {
    send_to_char( "Force whom to do what?\n\r", ch );
    return;
  }

  one_argument(argument,arg2);

  if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob"))
  {
    send_to_char("That will NOT be done.\n\r",ch);
    return;
  }

  mprintf(sizeof(buf), buf, "$n forces you to '%s'.", argument );

  if ( !str_cmp( arg, "all" ) )
  {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if (get_trust(ch) < MAX_LEVEL - 3)
    {
      send_to_char("Not at your level!\n\r",ch);
      return;
    }

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next;

      if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
      {
        if (!IS_AFK(vch)
            || !str_cmp(arg2,"save")
            || !str_cmp(arg2,"quit"))
        {
          act( buf, ch, NULL, vch, TO_VICT );
          interpret( vch, argument );
        }
      }
    }
  }
  else if (!str_cmp(arg,"players"))
  {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if (get_trust(ch) < MAX_LEVEL - 2)
    {
      send_to_char("Not at your level!\n\r",ch);
      return;
    }

    for ( vch = char_list; vch; vch = vch_next )
    {
      vch_next = vch->next;

      if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
           &&     vch->level < LEVEL_HERO)
      {
        if (!IS_AFK(vch)
            || !str_cmp(arg2,"save")
            || !str_cmp(arg2,"quit"))
        {
          act( buf, ch, NULL, vch, TO_VICT );
          interpret( vch, argument );
        }
      }
    }
  }
  else if (!str_cmp(arg,"gods"))
  {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if (get_trust(ch) < MAX_LEVEL - 2)
    {
      send_to_char("Not at your level!\n\r",ch);
      return;
    }

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next;

      if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
           &&   vch->level >= LEVEL_HERO)
      {
        act( buf, ch, NULL, vch, TO_VICT );
        interpret( vch, argument );
      }
    }
  }
  else
  {
    CHAR_DATA *victim;

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    if ( victim == ch )
    {
      send_to_char( "Aye aye, dude!\n\r", ch );
      return;
    }

    if (!is_room_owner(ch,victim->in_room)
        &&  ch->in_room != victim->in_room
        &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
      send_to_char("That character is in a private room.\n\r",ch);
      return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "Do it yourself!\n\r", ch );
      return;
    }

    if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
    {
      send_to_char("Not at your level!\n\r",ch);
      return;
    }

    act( buf, ch, NULL, victim, TO_VICT );
    /*if (victim->level == 1 && IS_LINKDEAD(victim) && !str_cmp(arg2, "quit"))
    do_function(victim, &do_quit, "imm-overide");
    else*/
    interpret( victim, argument );
  }

  send_to_char( "Ok.\n\r", ch );
  return;
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_silent_force( CHAR_DATA *ch, char *argument )
{
//  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' || argument[0] == '\0' )
  {
    send_to_char( "Force whom to do what?\n\r", ch );
    return;
  }

  one_argument(argument,arg2);

  if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob"))
  {
    send_to_char("That will NOT be done.\n\r",ch);
    return;
  }

  if ( !str_cmp( arg, "all" ) )
  {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if (get_trust(ch) < MAX_LEVEL - 3)
    {
      send_to_char("Not at your level!\n\r",ch);
      return;
    }

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next;

      if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
      {
//          act( buf, ch, NULL, vch, TO_VICT );
        interpret( vch, argument );
      }
    }
  }
  else if (!str_cmp(arg,"players"))
  {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if (get_trust(ch) < MAX_LEVEL - 2)
    {
      send_to_char("Not at your level!\n\r",ch);
      return;
    }

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next;

      if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
           &&     vch->level < LEVEL_HERO)
      {
//          act( buf, ch, NULL, vch, TO_VICT );
        interpret( vch, argument );
      }
    }
  }
  else if (!str_cmp(arg,"gods"))
  {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if (get_trust(ch) < MAX_LEVEL - 2)
    {
      send_to_char("Not at your level!\n\r",ch);
      return;
    }

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
      vch_next = vch->next;

      if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
           &&   vch->level >= LEVEL_HERO)
      {
//          act( buf, ch, NULL, vch, TO_VICT );
        interpret( vch, argument );
      }
    }
  }
  else
  {
    CHAR_DATA *victim;

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    if ( victim == ch )
    {
      send_to_char( "Aye aye, right away!\n\r", ch );
      return;
    }

    if (!is_room_owner(ch,victim->in_room)
        &&  ch->in_room != victim->in_room
        &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
      send_to_char("That character is in a private room.\n\r",ch);
      return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "Do it yourself!\n\r", ch );
      return;
    }

    if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
    {
      send_to_char("Not at your level!\n\r",ch);
      return;
    }

//      act( buf, ch, NULL, victim, TO_VICT );
    interpret( victim, argument );
  }

  send_to_char( "Ok.\n\r", ch );
  return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
  int level;
  char arg[MAX_STRING_LENGTH];
  CHAR_DATA *fch;

  /* RT code for taking a level argument */
  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    /* take the default path */

    if ( ch->invis_level)
    {
      ch->invis_level = 0;
      act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly fade back into existence.\n\r", ch );
    }
    else
    {
#if NO_SECRET
      ch->invis_level = LEVEL_IMMORTAL;
#else
      ch->invis_level = get_trust(ch);
#endif
      act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly vanish into thin air.\n\r", ch );
      for ( fch = char_list; fch != NULL; fch = fch->next )
      {
        if ( ( fch->master == ch ) && ( fch->level < ch->invis_level ) )
          stop_follower( fch );
        if ( ( fch->leader == ch ) && ( fch->level < ch->invis_level ) )
          fch->leader = fch;
        if ( ( fch->reply == ch )  && ( fch->level < ch->invis_level ) )
          fch->reply = NULL;
      }
    }
  }
  else
    /* do the level thing */
  {
    level = atoi(arg);
    if (level < 2 || level > get_trust(ch))
    {
      send_to_char("Invis level must be between 2 and your level.\n\r",ch);
      return;
    }
    else
    {
      ch->reply = NULL;
      ch->invis_level = level;
      act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly vanish into thin air.\n\r", ch );
      for ( fch = char_list; fch != NULL; fch = fch->next )
      {
        if ( ( fch->master == ch ) && ( fch->level < ch->invis_level ) )
          stop_follower ( fch );
        if ( ( fch->leader == ch ) && ( fch->level < ch->invis_level ) )
          fch->leader = fch;
        if ( ( fch->reply == ch )  && ( fch->level < ch->invis_level ) )
          fch->reply = NULL;
      }
    }
  }


  return;
}


void do_incognito( CHAR_DATA *ch, char *argument )
{
  int level;
  char arg[MAX_STRING_LENGTH];
  CHAR_DATA *fch;

  /* RT code for taking a level argument */
  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    /* take the default path */

    if ( ch->incog_level)
    {
      ch->incog_level = 0;
      act( "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You are no longer cloaked.\n\r", ch );
    }
    else
    {
#if NO_SECRET
      ch->incog_level = LEVEL_IMMORTAL;
#else
      ch->incog_level = get_trust(ch);
#endif
      act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You cloak your presence.\n\r", ch );
      for ( fch = char_list; fch != NULL; fch = fch->next )
      {
        if ( ( fch->reply == ch )  && ( fch->level < ch->incog_level ) )
          fch->reply = NULL;
      }
    }
  }
  else
    /* do the level thing */
  {
    level = atoi(arg);
    if (level < LEVEL_CHANNEL || level > get_trust(ch))
    {
      send_to_char("Incog level must be between 2 and your level.\n\r",ch);
      return;
    }
    else
    {
      ch->reply = NULL;
      ch->incog_level = level;
      act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You cloak your presence.\n\r", ch );
      for ( fch = char_list; fch != NULL; fch = fch->next )
      {
        if ( ( fch->reply == ch )  && ( fch->level < ch->incog_level ) )
          fch->reply = NULL;
      }

    }
  }


  return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) )
    return;

  if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
  {
    REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
    send_to_char( "Holy light mode off.\n\r", ch );
  }
  else
  {
    SET_BIT(ch->act, PLR_HOLYLIGHT);
    send_to_char( "Holy light mode on.\n\r", ch );
  }

  return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi (CHAR_DATA *ch, char *argument)
{
  send_to_char("You cannot abbreviate the prefix command.\r\n",ch);
  return;
}

void do_prefix (CHAR_DATA *ch, char *argument)
{
  char buf[MSL];

  if (argument[0] == '\0')
  {
    if (ch->prefix[0] == '\0')
    {
      send_to_char("You have no prefix to clear.\r\n",ch);
      return;
    }

    send_to_char("Prefix removed.\r\n",ch);
    free_string(ch->prefix);
    ch->prefix = str_dup("", ch->prefix);
    return;
  }

  if (ch->prefix[0] != '\0')
  {
    mprintf(sizeof(buf), buf,"Prefix changed to %s.\r\n",argument);
    free_string(ch->prefix);
  }
  else
  {
    mprintf(sizeof(buf), buf,"Prefix set to %s.\r\n",argument);
  }

  ch->prefix = str_dup(argument, ch->prefix);
}

/** do_pload
    Loads a player object into the mud, bringing them (and their pet) to
    you for easy modification.  Player must not be connected.
    Note: be sure to send them back when your done with them.
 */

void do_pload( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA d;
  bool isChar = FALSE;
  char name[MAX_INPUT_LENGTH], log_buf[MSL];
  char buf[MAX_INPUT_LENGTH];

  if (argument[0] == '\0')
  {
    send_to_char("Load who?\n\r", ch);
    return;
  }

  argument[0] = UPPER(argument[0]);
  argument = one_argument(argument, name);

  /* Dont want to load a second copy of a player who's allready online! */
  if ( get_char_world( ch, name ) != NULL )
  {
    send_to_char( "That person is already connected.\n\r", ch );
    return;
  }
  d.original = NULL;
  isChar = load_char_obj(&d, name); /* char pfile exists? */

  if (!isChar)
  {
    send_to_char("Load who? Are you sure? I cant seem to find them.\n\r", ch);
    return;
  }

  d.character->desc     = NULL;
  d.character->next     = char_list;
  char_list             = d.character;
  d.character->next_player = player_list;
  player_list = d.character;
  d.connected           = CON_PLAYING;
  reset_char(d.character);
  SET_BIT(d.character->active_flags, ACTIVE_PLOAD);
  if (d.character->level > ch->level)
  {
    send_to_char("\n\r{RNO WAY, JOSE.  {wYou cannot pload a char of greater level than yourself.\n\r", ch);
    send_to_char("This action has been logged.\n\r", ch);
    mprintf(sizeof(log_buf), log_buf, "%s attempted to pload %s.\n\r", ch->name, d.character->name);
    wiznet(log_buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    do_function(d.character, &do_quit, "imm-overide");
    log_string(log_buf);
    return;
  }

  /* bring player to imm */
  if ( d.character->in_room != NULL )
  {
    d.character->old_room = d.character->in_room;
    char_to_room( d.character, ch->in_room); /* put in room imm is in */
  }

  mprintf( sizeof(buf), buf, "{R%s{x has been pulled from pattern.",
           capitalize( d.character->name ));
  wiznet ( buf, d.character, NULL, WIZ_SECURE, 0, get_trust( ch ) );
  act( "$n has pulled $N from the pattern!",
       ch, NULL, d.character, TO_ROOM );
  act( "You have pulled $n from the pattern!",
       d.character, NULL, ch, TO_VICT );

  if (d.character->pet != NULL)
  {
    char_to_room(d.character->pet,d.character->in_room);
    act("$n has entered the game.",d.character->pet,NULL,NULL,TO_ROOM);
  }

}

/** do_punload
    Returns a player, previously 'ploaded' back to the void from whence
    they came.  This does not work if the player is actually connected.

    Note: removing the check;
    if (victim->desc != NULL)
    {
    }

    will allow you to essentially boot players offline. However, I'm not
    really sure what kind of havoc this may wreak, although I have a few
    ideas.  Be sure you look in comm.c and find out the proper procedure
    for booting folks off before you do this... (namely, closing the
    socket)
 */

void do_punload( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char who[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];

  argument = one_argument(argument, who);

  if ( ( victim = get_char_world( ch, who ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  /** Person is legitametly logged on... was not ploaded.
   */
  if (victim->desc != NULL)
  {
    send_to_char("I dont think that would be a good idea...\n\r", ch);
    return;
  }
  /* return player and pet to orig room */
  if (victim->was_in_room != NULL)
  {
    char_to_room(victim, victim->was_in_room);

    if (victim->pet != NULL)
      char_to_room(victim->pet, victim->was_in_room);
  }

  if (victim->old_room != NULL) /* return player and pet to orig room */
  {
    if (victim->old_room != victim->in_room)
    {
      move_to_room(victim, victim->old_room);
    }

    if (victim->pet != NULL)
    {
      if (victim->pet->in_room != victim->old_room)
      {
        move_to_room(victim->pet, victim->old_room);
      }
    }
  }

  mprintf( sizeof(buf), buf, "{R%s{x has been released to the pattern.",
           capitalize( victim->name ));

  wiznet ( buf, victim, NULL, WIZ_SECURE, 0, get_trust( ch ) );
  printf_to_char( ch, "You have released %s back to the pattern.\n\r", victim->name );
  if (victim->pet && IS_NPC(victim->pet))
    printf_to_char( ch, "%s is released to the pattern.\n\r",
                    victim->pet->short_descr );

  do_function(victim, &do_quit,"imm-overide");
}


void do_plock( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char who[MAX_INPUT_LENGTH];

  argument = one_argument(argument, who);

  if ( ( victim = get_char_world( ch, who ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  /** Person is legitametly logged on... was not ploaded.
   */
  if (victim->desc != NULL)
  {
    send_to_char("I dont think that would be a good idea...\n\r", ch);
    return;
  }

  /* return player and pet to orig room */
  victim->was_in_room = NULL;
  victim->old_room = NULL;
  send_to_char("That character has been plocked to the room they leave from.\n\r",ch);
}



/* do_cloak - Cloak other users presence
 *
 *      Creator: The Mage
 *
 *    do_cloak( CHAR_DATA *ch, char *argument
 *    ch - Character who calls this function information
 *    argument - Parameters to function
 *
 * do_cloak will set another users incognito level the same as the
 *    person who called.  This will allow the caller to cloak
 *    anyones presence in the room with them.
 *    The first thing this does is convert the argument to 1
 *    argument field via one_argument().  Then it checks to see if
 *    an argument was inputed into the system.  Then it gets the
 *    victim information using get_char_room().
 *    Once it has who it is.. then you set the incognio level to the
 *    level of the caster. ie: level 60 caster will set a level
 *    60(trust level that the caster has ) incog flag.  If the flag
 *    already is enabled then remove any incog flags
 *
 */
void do_cloak( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int lev  = 0;

  argument = one_argument( argument, arg1 );

  /* take the default path */
  if ( arg1[0] == '\0' )
  {
    send_to_char("You cannot cloak yourself\n\r",ch);
    return;
  }

  victim = get_char_room(ch, arg1);
  if (victim == NULL)
  {
    send_to_char("Invalid victim.\n\r",ch);
    return;
  }

  one_argument( argument, arg2 );
  if (arg2[0] == '\0')
    lev = LEVEL_IMMORTAL;
  else
    lev = atoi(arg2);

  /* If victim already is incognito then set them Visible */
  if ( ( victim->invis_level )
       ||   ( lev == 0 ) )
  {
    victim->invis_level = 0;
    act( "$n is no longer cloaked.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are no longer cloaked.\n\r", victim );
  }
  else /* do the level thing */
  {
    victim->reply = NULL;
    victim->invis_level = lev;
    printf_to_char(ch,"You cloak %s's presence.\n\r",victim->name);
    send_to_char( "You are now cloaked.\n\r", victim );
  }
  return;
}

void do_secset(CHAR_DATA *ch, char *argument)      /* By sembiance - bert@ncinter.net */
{
  CHAR_DATA *victim;
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  int seclevel;

  argument = one_argument(argument, arg1);

  if (arg1[0] == '\0')
  {
    send_to_char("Set who's Security?\n\r", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg1)) == NULL)
  {
    send_to_char("They're not here.\n\r", ch);
    return;
  }

  if ( IS_NPC( victim ) )
  {
    send_to_char( "You may change an NPC's security.\n\r", ch );
    return;
  }

  if ( victim->level > ch->level )
  {
    send_to_char("You may not alter someone who is a higher level than you.\n\r", ch);
    return;
  }

  argument = one_argument(argument, arg2);

  if (arg2[0] == '\0')
  {
    send_to_char("Set their security to what level?\n\r", ch);
    return;
  }

  seclevel = atoi(arg2);

  if ( (seclevel <= 0) || (seclevel > 9) )
  {
    send_to_char("Security range is from 1 to 9.\n\r", ch);
    return;
  }

  victim->pcdata->security = seclevel;
  send_to_char("Changed players security level.\n\r", ch);
  printf_to_char(victim, "%s just raised your security to level %d", ch->name,
                 seclevel);

  return;
}


void do_saveskills( CHAR_DATA *ch, char *argument )
{
  save_spell_info(SPELLS_FILE);
  send_to_char("Skills info Saved\n\r",ch);
}

/* This code was written by Kadian of Distant Thunder if you have any problems
with this code feel free to e-mail at kadian@nurf.com ...no need to give
any credit...it's a tiny snippet :) */


void do_otype(CHAR_DATA *ch, char *argument)
{
  int64 type = 0,atype=0;
  int64 type2 = 0, type3 = 0, type4 = 0;
  int vnum=1, i=0;
  int llevel = FALSE;
  int tlevel = FALSE;
  char buf[MSL], buf2[MSL];
  char buffer[12 * MAX_STRING_LENGTH];
  char arg1[MIL];
  char arg2[MIL];
  char last_arg[MIL];
  char next_arg[MIL];
  OBJ_INDEX_DATA *obj;
  bool found;
  int olevel=0;
  int avg = 0;
  bool havelevel= FALSE;


  argument = one_argument(argument, arg1);
  argument = one_argument (argument, arg2);
  argument  = one_argument(argument, next_arg);

  found = FALSE;
  buffer [0] = '\0';


  if ( IS_NULLSTR( arg1 )
       ||   IS_NULLSTR( arg2 ) )
  {
    send_to_char("Syntax: otype <armor|weapon> <wear_loc> <low_level> <high_level>\n\r", ch );
    send_to_char("        otype <wear_loc> <low_level> <high_level>\n\r", ch );
    return;
  }

  if ( !str_cmp( arg1, "armor" ) )
  {
    type = flag_value( type_flags, arg1 );
    atype = flag_value( type_flags, "jewelry" );

    if ( ( type2 = flag_value( wear_flags, arg2 ) ) == NO_FLAG )
    {
      send_to_char( "No such armor type.\n\r", ch );
      return;
    }
  }
  else if (!str_cmp(arg1,"weapon"))
  {
    type = flag_value(type_flags,arg1);

    if ((type2 = flag_value(weapon_class,arg2)) == NO_FLAG)
    {
      send_to_char("No such weapon type.\n\r",ch);
      return;
    }
  }
  else if ( ( type3 = flag_value( wear_flags, arg1 ) ) != NO_FLAG )
  {
    if ( !is_number( arg2 )
         ||   !is_number( next_arg ) )
    {
      send_to_char( "Syntax: otype <wear_loc> <min_level> <max_level>\n\r", ch );
      return;
    }

    type = 0;
    llevel = atoi(next_arg);
    tlevel = TRUE;

    olevel = atoi(arg2);
    havelevel = TRUE;

  }
  else
  {
    if ( ( type4 = flag_value( type_flags, arg1 ) ) == NO_FLAG )
    {
      send_to_char( "Unknown Type.\n\r", ch );
      return;
    }

    if (!is_number(arg2))
    {
      send_to_char("Arg2 argument is bad...should be a number. See help otype.\n\r",ch);
      return;
    }
    else
    {
      olevel = atoi(arg2);
      havelevel = TRUE;
    }
  }

  if ( next_arg[0] != '\0'
       &&   !is_number( arg2 ) )
  {
    one_argument(argument,last_arg);

    if (!is_number(next_arg))
    {
      send_to_char("Invalid level.\n\r",ch);
      return;
    }

    olevel = atoi(next_arg);
    havelevel = TRUE;

    if (last_arg[0] != '\0')
    {
      if (!is_number(last_arg))
      {
        send_to_char("Arg4 argument is bad...should be a number. See help otype.\n\r",ch);
        return;
      }

      llevel = atoi(last_arg);
      tlevel = TRUE;
    }
  }


  for ( ;vnum <= top_vnum_obj ;vnum++)
  {
    if ( ( obj = get_obj_index(vnum) ) != NULL )
    {
      if ( !IS_NULLSTR( obj->clan_name ) )
        continue;

      if ( !str_cmp( arg1, "weapon" )
           &&   ( obj->item_type != type
                  ||     obj->value[0] != type2 ) )
        continue;

      if ( !str_cmp( arg1, "armor" )
           &&   ( ( obj->item_type != type
                    &&       obj->item_type != atype )
                  ||       !IS_SET( obj->wear_flags, type2 ) ) )
        continue;

      if ( type3 != 0
           &&   ( !IS_SET( obj->wear_flags, type3 )
                  ||     !is_number( arg2 )
                  ||     !is_number( next_arg ) ) )
        continue;

      if ( type4 != 0
           &&   ( obj->item_type != type
                  ||     type2 != 0 ) )
        continue;

      avg = 0;
      if (obj->item_type == ITEM_WEAPON)
        avg = DICE_AVG(obj->value[1], obj->value[2]);
      else if (obj->item_type == ITEM_ARMOR)
        avg = (obj->value[0] + obj->value[1] + obj->value[2] + obj->value[3]) / -4;
      

      if (havelevel)
      {
        if (tlevel)
        {
          if (obj->level >= olevel && obj->level <= llevel)
          {
            mprintf(sizeof(buf), buf,
                    "[{c%3d{x]%6d {g%3d {r%4d  {x%s{x\n\r",
                    obj->area->vnum, vnum, obj->level, avg, obj->short_descr);
            found = TRUE;
            i++;
            strcat(buffer,buf);
          }
        }
        else if (olevel == obj->level)
        {
          mprintf(sizeof(buf), buf,
                  "[{c%3d{x]%6d {g%3d {r%4d  {x%s{x\n\r",
                  obj->area->vnum, vnum, olevel, avg, obj->short_descr );
          found = TRUE;
          i++;
          strcat(buffer,buf);
        }
      }
      else
      {
        mprintf(sizeof(buf), buf,
                "[{c%3d{x]%6d {g%3d {r%4d  {x%s{x\n\r",
                obj->area->vnum, vnum, obj->level, avg, obj->short_descr );
        found = TRUE;
        i++;
        strcat(buffer,buf);
      }
    }
  }

  //Display counter (i)
  if ( i > 0 )
  {
    sprintf( buf2, "\n\rTotal Found: %d\n\r", i );
    strcat(buffer, buf2);
  }

  if (!found)
    send_to_char("No objects of that type exist.\n\r",ch);
  else
  {
    send_to_char( "Area  Vnum  Lvl  Avg  Short Desc\n\r", ch );
    send_to_char( "----- ----- --- ----- -----------------\n\r", ch );

    if (ch->lines)
      page_to_char(buffer,ch);
    else
      send_to_char(buffer,ch);

  }
}

void do_skillstat(CHAR_DATA *ch, char *argument)
{
  BUFFER *buffer;
  char arg[MSL];
  char skill_list[LEVEL_HERO + 1][MSL];
  int sn, level, min_lev = 1, max_lev = LEVEL_HERO;
  bool found = FALSE;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  char color[2];

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument,arg);

  if ( arg[0] == '\0' )
  {
    send_to_char( "List skills for whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char( "Use this for skills on players.\n\r", ch );
    return;
  }

  /* initialize data */
  for (level = 0; level <=LEVEL_HERO; level++)
  {
    skill_list[level][0] = '\0';
  }

  /* New skill display loop */
  for ( sn = 0; sn < MAX_SKILL; sn++ )
  {
    if ( IS_NULLSTR( skill_table[sn].name ) )
      continue;


    level = is_racial_skill( victim, sn ) ? 0 : skill_table[sn].skill_level[victim->gameclass];

    if ( ((level < min_lev)
          && !is_racial_skill( victim, sn ) )
         ||   (level > max_lev)
         ||   (skill_table[sn].spell_fun != spell_null)
         ||   (victim->pcdata->learned[sn] <= 0 ) )
      continue;

    found = TRUE;
    strncpy_color(buf2,skill_table[sn].name,18,' ',TRUE);

    // don't list the same spell twice in case it matches group AND spell name
    if (strstr(skill_list[level], skill_table[sn].name ) != NULL )
      continue;

    if ( victim->level < level )
      sprintf( buf, "%-18s n/a       ", buf2 );
    else
    {
      sprintf( color, "%c", color_scale(victim->pcdata->learned[sn], "rRYG"));
      sprintf( buf, "{w[{c%-20s{w]{g[{%s%3d%%{g]{x",
               buf2,
               victim->pcdata->learned[sn] == 100 ? "W" : color,
               victim->pcdata->learned[sn]);
    }

    if ( skill_list[level][0] == '\0' )
    {
      if ( is_racial_skill( victim, sn ) )//level == 0 )
        sprintf( skill_list[level], "\n\r{xRacial:   %s", buf );

      if ( level == 0 )
        sprintf( skill_list[level], "\n\r{xRacial:   %s", buf );
      else
        sprintf( skill_list[level], "\n\r{xLevel %2d: %s", level, buf );
    }
    else /* append */
    {
      strcat( skill_list[level], "\n\r          " );
      strcat( skill_list[level], buf );
    }
  } // for (sn = 0 to MAX_SKILL)

  if (!found)
  {
    send_to_char("No skills found.\n\r",ch);
    return;
  }

  buffer = new_buf();
  for (level = 0; level <=LEVEL_HERO ; level++)
    if (skill_list[level][0] != '\0')
      add_buf(buffer,skill_list[level]);
  add_buf(buffer,"\n\r");
  page_to_char(buf_string(buffer),ch);
  free_buf(buffer);
}

void do_spellstat(CHAR_DATA *ch, char *argument)
{
  BUFFER *buffer;
  char arg[MSL];
  char spell_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
  int sn, gn, col, level, min_lev = 1, max_lev = LEVEL_HERO, mana;
  bool found = FALSE;
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  char color[2];


  if (IS_NPC(ch))
    return;

  argument = one_argument(argument,arg);

  if ( arg[0] == '\0' )
  {
    send_to_char( "List spells for whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char( "Use this for skills on players.\n\r", ch );
    return;
  }

  /* groups */

  col = 0;

  for (gn = 0; gn < MAX_GROUP; gn++)
  {
    if (group_table[gn].name == NULL)
      break;
    if (victim->pcdata->group_known[gn])
    {
      printf_to_char(ch,"%-20s ",group_table[gn].name);
      if (++col % 3 == 0)
        send_to_char("\n\r",ch);
    }
  }
  if ( col % 3 != 0 )
  {
    send_to_char( "\n\r", ch );
    printf_to_char(ch,"Creation points: %d\n\r",victim->pcdata->points);

  }

  /* initialize data */
  for (level = 0; level <= LEVEL_HERO; level++)
  {
    spell_list[level][0] = '\0';
  }

  for (sn = 0; sn < MAX_SKILL; sn++)
  {
    if (skill_table[sn].name == NULL )
      break;

    if ((level = skill_table[sn].skill_level[victim->gameclass]) < LEVEL_HERO + 1
        &&  level >= min_lev && level <= max_lev
        &&  skill_table[sn].spell_fun != spell_null
        &&  victim->pcdata->learned[sn] > 0)
    {
      found = TRUE;
      level = skill_table[sn].skill_level[victim->gameclass];

      if (victim->level < level)
        /*mprintf(sizeof(buf), buf,"%-18s n/a      ", skill_table[sn].name);*/
        mprintf(sizeof(buf), buf, "{W[{C%2d{W]{D[{c%-18s{D]{x\n\r",level, skill_table[sn].name);
      else
      {
        mana = UMAX(skill_table[sn].min_mana,
                    100/(2 + victim->level - level));
        /*mprintf(sizeof(buf), buf,"%-18s  %3d mana  ",skill_table[sn].name,mana);*/
        mprintf(sizeof(color), color, "%c", color_scale(victim->pcdata->learned[sn], "rRYG"));
        mprintf(sizeof(buf), buf, "{W[{BLvl:{C%2d{W]{D[{c%-18s{D]{W[{BMana:{Y%3d{W]{x[{DP:{x{%s%4d%%]{x\n\r",level,
                skill_table[sn].name, mana, victim->pcdata->learned[sn] == 100 ? "W" : color,
                victim->pcdata->learned[sn]);
      }

      strcat(spell_list[level], buf);
    }
  }

  /* return results */

  if (!found)
  {
    send_to_char("No spells found.\n\r",ch);
    return;
  }

  buffer = new_buf();
  for (level = 0; level <= LEVEL_HERO; level++)
    if (spell_list[level][0] != '\0')
      add_buf(buffer,spell_list[level]);
  add_buf(buffer,"\n\r");
  page_to_char(buf_string(buffer),ch);
  free_buf(buffer);
}
/*
 * I don't think ROM muds have this particular function, if you do...
 * just don't include it :)
 */
CHAR_DATA *get_char( CHAR_DATA *ch )
{
  if ( !ch->pcdata )
    return ch->desc->original;
  else
    return ch;
}


/*
 * Copywrite 1996 by Amadeus of AnonyMUD, AVATAR, Horizon MUD, and Despair
 *            ( amadeus@myself.com )
 *
 * Public use authorized with this header intact.
 */
void do_wpeace(CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *rch;

  rch = get_char( ch );

  for ( rch = char_list; rch; rch = rch->next )
  {
    if ( ch->desc == NULL || ch->desc->connected != CON_PLAYING )
      continue;

    stop_hating(rch);
    if ( rch->fighting )
    {
      printf_to_char(rch, "%s has declared World Peace.\n\r", ch->name );
      stop_fighting( rch, TRUE );
    }
    rch->group_fight = 0;
  }

  send_to_char( "You have declared World Peace.\n\r", ch );
  do_function(ch, &do_info,"World Peace has been Declared");
  return;
}

void restore_char(CHAR_DATA *vch)
{
  int fb = skill_lookup("fire breath");
  int slow = skill_lookup("slow");
  int enta = skill_lookup("enta");

  affect_strip(vch,gsn_plague);
  affect_strip(vch,gsn_poison);
  affect_strip(vch,gsn_blindness);
  affect_strip(vch,gsn_sleep);
  affect_strip(vch,gsn_curse);
  affect_strip(vch,gsn_dirt);
  affect_strip(vch,gsn_weaken);
  affect_strip(vch,gsn_change_sex);
  affect_strip(vch,fb);
  affect_strip(vch,slow);
  affect_strip(vch,enta);

  vch->hit     = GET_HP( vch );
  vch->mana    = GET_MANA( vch);
  vch->move    = vch->max_move;
  if (vch->pcdata != NULL)
  {
    vch->pcdata->condition[COND_HUNGER] = 40;
    vch->pcdata->condition[COND_THIRST] = 40;
    vch->pcdata->condition[COND_FULL] = 0;
    vch->pcdata->condition[COND_DRUNK] = 0;
    vch->pcdata->degrading = 0;
  }
  if (vch->pet)
    restore_char(vch->pet);
  update_pos(vch);
}

void fullrestore_char(CHAR_DATA *vch, CHAR_DATA *ch)
{
  int fb = skill_lookup("fire breath");
  int slow = skill_lookup("slow");
  int enta = skill_lookup("enta");

  affect_strip(vch,gsn_plague);
  affect_strip(vch,gsn_poison);
  affect_strip(vch,gsn_blindness);
  affect_strip(vch,gsn_sleep);
  affect_strip(vch,gsn_curse);
  affect_strip(vch,gsn_dirt);
  affect_strip(vch,gsn_weaken);
  affect_strip(vch,gsn_change_sex);
  affect_strip(vch,fb);
  affect_strip(vch,slow);
  affect_strip(vch,enta);

  if ( !is_sanc_spelled( vch ) && !IS_IMMORTAL(vch) )
    spell_sanctuary(skill_lookup("sanctuary"), ch->level, ch, (void*)vch, TAR_CHAR_DEFENSIVE);

  if ( !IS_AFFECTED(vch, AFF_HASTE ) && !IS_IMMORTAL(vch) )
    spell_haste(skill_lookup("haste"), ch->level, ch, (void*)vch, TAR_CHAR_DEFENSIVE);

  if ( !is_affected( vch, skill_lookup("bless")) && !IS_IMMORTAL(vch) )
    spell_bless(skill_lookup("bless"), ch->level, ch, (void*)vch, TAR_CHAR_DEFENSIVE);

  if ( !is_affected( vch, skill_lookup("smoke screen")) && !IS_IMMORTAL(vch) )
    spell_smoke_screen(skill_lookup("smoke screen"), ch->level, ch, (void*)vch, TAR_CHAR_DEFENSIVE);

  vch->hit      = GET_HP(vch);
  vch->mana     = GET_MANA(vch);
  vch->move     = vch->max_move;
  if (vch->pcdata != NULL)
  {
    vch->pcdata->condition[COND_HUNGER] = 40;
    vch->pcdata->condition[COND_THIRST] = 40;
    vch->pcdata->condition[COND_FULL] = 0;
    vch->pcdata->condition[COND_DRUNK] = 0;
    vch->pcdata->degrading = 0;
  }
  if (vch->pet)
    restore_char(vch->pet);
  update_pos(vch);
}
/***************************************************************************
 * Ever wonder just what the stats on things were in the game? Statting    *
 * one object or mob at a time can be tedious and frequently you have to   *
 * stop and write things down, like hitpoints, armor classes, etc, if you  *
 * are trying to build an area and you want to preserve some continuity.   *
 * Granted, there should probably be tables and such availabe for builders'*
 * use (as there are on Broken Shadows), but you have to base those tables *
 * off something.                                                          *
 *                                                                         *
 * Well... this function is a cross between stat and dump. It loads each   *
 * mob and object briefly and writes its stats (or at least the vital ones)*
 * to a file. I removed a lot of the things from the stat part, mostly     *
 * empty lines where PC data was stored and also a lot of the things that  *
 * returned 0, such as carry weight.                                       *
 *                                                                         *
 * The files are place in the parent directory of the area directory by    *
 * default and are rather large (about 800k each for Shadows). With a      *
 * little work (I wrote a little C++ program to do this), they can be      *
 * converted into a character-delimeted file, so you can import it into    *
 * Access, Excel, or many other popular programs. I could have modified    *
 * This file to write it out in that format, but I was too lazy.           *
 *                                                                         *
 * Oh yeah. There's also a section for rooms. This is straight from rstat  *
 * It hasn't been tweaked at all. The first time I used it, it hit an      *
 * endless loop somewhere in there and I was too lazy to debug it. If you  *
 * want to uncomment it and debug it for me, feel free :)                  *
 *                                                                         *
 * One other thing work noting: Since it does load all the objects and     *
 * mobs in quick succession, CPU and memory usage climbs for about 10-15   *
 * seconds. This might cause a bit of lag for the players. I dunno. I      *
 * haven't used it when players were on.                                   *
 *                                                                         *
 * If you choose to use this code, please retain my name in this file and  *
 * send me an email (dwa1844@rit.edu) saying you are using it. Suggestions *
 * for improvement are welcome                                             *
 ***************************************************************************/

/*
 * new_dump written by Rahl (Daniel Anderson) of Broken Shadows
 */
void do_new_dump( CHAR_DATA *ch, char *argument )
{
  pthread_t thread;

  send_to_char("Sorry, this is disabled due to lag it causes.\n\r",ch);

  if (IS_SET(ch->active_flags, ACTIVE_THREAD))
  {
    send_to_char("Sorry, a Fixed buffer operation is currently underway for you.\n\r",ch);
    return;
  }

  SET_BIT(ch->active_flags, ACTIVE_THREAD);
  /* serious kludge here */
  pass.ch = ch;
  free_string(pass.args);
  pass.args = str_dup(argument, pass.args);
  send_to_char("Starting the NEW_DUMP process.\n\r",ch);
  pthread_create( &thread, NULL,(void *)new_dump, (void *)NULL);
  pthread_detach(thread);


}

void new_dump ()
{
  MOB_INDEX_DATA *pMobIndex;
  OBJ_INDEX_DATA *pObjIndex;
  CHAR_DATA *ch;
  /*    ROOM_INDEX_DATA *pRoomIndex; */
  FILE *fp;
  int vnum,nMatch = 0;
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  /*    CHAR_DATA *rch; */
  /*    int door; */
  AFFECT_DATA *paf;
  CHAR_DATA *mob;

  /* open file */
  fclose(fpReserve);
  nFilesOpen--;
  ch = pass.ch;
  /* start printing out mobile data */
  fp = fopen("../mob.txt","w");
  nFilesOpen++;
  fprintf(fp,"\nMobile Analysis\n");
  fprintf(fp,  "---------------\n");
  nMatch = 0;
  for (vnum = 0; nMatch < top_mob_index; vnum++)
    if ((pMobIndex = get_mob_index(vnum)) != NULL)
    {
      nMatch++;
      mob = create_mobile( pMobIndex );
      mprintf( sizeof(buf), buf, "Name: %s.\n",
               mob->name );
      fprintf( fp, "%s", buf );

      mprintf( sizeof(buf), buf, "Vnum: %d  Race: %s  Sex: %s  Room: %d  Count %d\n",
               IS_NPC(mob) ? mob->pIndexData->vnum : 0,
               race_table[mob->race].name,
               mob->sex == SEX_MALE    ? "male"   :
               mob->sex == SEX_FEMALE  ? "female" : "neutral",
               mob->in_room == NULL    ?        0 : mob->in_room->vnum,
               mob->pIndexData->count );
      fprintf( fp, "%s", buf );

      mprintf( sizeof(buf), buf,
               "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n",
               mob->perm_stat[STAT_STR],
               get_curr_stat(mob,STAT_STR),
               mob->perm_stat[STAT_INT],
               get_curr_stat(mob,STAT_INT),
               mob->perm_stat[STAT_WIS],
               get_curr_stat(mob,STAT_WIS),
               mob->perm_stat[STAT_DEX],
               get_curr_stat(mob,STAT_DEX),
               mob->perm_stat[STAT_CON],
               get_curr_stat(mob,STAT_CON) );
      fprintf( fp, "%s", buf );

      mprintf( sizeof(buf), buf, "Hp: %d  Mana: %d  Move: %d  Hit: %d  Dam: %d\n",
               mob->max_hit,
               mob->max_mana,
               mob->max_move,
               GET_HITROLL(mob), GET_DAMROLL(mob) );
      fprintf( fp, "%s", buf );

      mprintf( sizeof(buf), buf,
               "Lv: %d  Align: %d  Gold: %ld  Damage: %dd%d  Message: %s\n",
               mob->level,
               mob->alignment,
               mob->gold,
               mob->damage[DICE_NUMBER],mob->damage[DICE_TYPE],
               attack_table[mob->dam_type].name);
      fprintf( fp, "%s", buf );

      mprintf(sizeof(buf), buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n",
              GET_AC(mob,AC_PIERCE), GET_AC(mob,AC_BASH),
              GET_AC(mob,AC_SLASH),  GET_AC(mob,AC_EXOTIC));
      fprintf( fp, "%s", buf );

      mprintf(sizeof(buf), buf, "Act: %s\n",act_bit_name(mob->act));
      fprintf( fp, "%s", buf );

      if (IS_NPC(mob) && mob->off_flags)
      {
        mprintf(sizeof(buf), buf, "Offense: %s\n",off_bit_name(mob->off_flags));
        fprintf( fp, "%s", buf );
      }

      if (mob->imm_flags)
      {
        mprintf(sizeof(buf), buf, "Immune: %s\n",imm_bit_name(mob->imm_flags));
        fprintf( fp, "%s", buf );
      }

      if (mob->res_flags)
      {
        mprintf(sizeof(buf), buf, "Resist: %s\n", imm_bit_name(mob->res_flags));
        fprintf( fp, "%s", buf );
      }

      if (mob->vuln_flags)
      {
        mprintf(sizeof(buf), buf, "Vulnerable: %s\n", imm_bit_name(mob->vuln_flags));
        fprintf( fp, "%s", buf );
      }

      mprintf(sizeof(buf), buf, "Form: %s\nParts: %s\n",
              form_bit_name(mob->form), part_bit_name(mob->parts));
      fprintf( fp, "%s", buf );

      if (mob->affected_by)
      {
        mprintf(sizeof(buf), buf, "Affected by %s\n",
                affect_bit_name(mob->affected_by));
        fprintf( fp, "%s", buf );
      }

      mprintf( sizeof(buf), buf, "Short description: %s\nLong  description: %s",
               mob->short_descr,
               mob->long_descr[0] != '\0' ? mob->long_descr : "(none)\n" );
      fprintf( fp, "%s", buf );

      if ( IS_NPC(mob) && mob->spec_fun != 0 )
      {
        mprintf( sizeof(buf), buf, "Mobile has special procedure. - %s\n", spec_name( mob->spec_fun ) );
        fprintf( fp, "%s", buf );
      }

      for ( paf = mob->affected; paf != NULL; paf = paf->next )
      {
        mprintf( sizeof(buf), buf,
                 "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n",
                 skill_table[(int) paf->type].name,
                 affect_loc_name( paf->location ),
                 paf->modifier,
                 paf->duration,
                 affect_bit_name( paf->bitvector ),
                 paf->level
               );
        fprintf( fp, "%s", buf );
      }
      fprintf( fp, "\n" );
      extract_char( mob, FALSE );
    }
  fclose(fp);
  nFilesOpen--;

  /* start printing out object data */
  fp = fopen("../obj.txt","w");
  nFilesOpen++;

  fprintf(fp,"\nObject Analysis\n");
  fprintf(fp,  "---------------\n");
  nMatch = 0;

  for (vnum = 0; nMatch < top_obj_index; vnum++)
    if ((pObjIndex = get_obj_index(vnum)) != NULL)
    {
      nMatch++;

      obj = create_object( pObjIndex, 0 );

      mprintf( sizeof(buf), buf, "Name(s): %s\n",
               obj->name );
      fprintf( fp, "%s", buf );

      mprintf( sizeof(buf), buf, "Vnum: %d  Format: %s  Type: %s  Number: %d/%d  Weight: %d/%d\n",
               obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
               (obj->short_descr), 1, get_obj_number( obj ),
               obj->weight, get_obj_weight( obj ) );
      fprintf( fp, "%s", buf );

      mprintf( sizeof(buf), buf, "Short description: %s\nLong description: %s\n",
               obj->short_descr, obj->description );
      fprintf( fp, "%s", buf );

      mprintf( sizeof(buf), buf, "Wear bits: %s  Extra bits: %s\n",
               wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );
      fprintf( fp, "%s", buf );

      mprintf( sizeof(buf), buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n",
               obj->level, obj->cost, obj->condition, obj->timer );
      fprintf( fp, "%s", buf );

      mprintf( sizeof(buf), buf,
               "In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n",
               obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
               obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
               obj->carried_by == NULL    ? "(none)" : obj->carried_by->name,
               obj->wear_loc );
      fprintf( fp, "%s", buf );

      mprintf( sizeof(buf), buf, "Values: %d %d %d %d %d\n",
               obj->value[0], obj->value[1], obj->value[2], obj->value[3],
               obj->value[4] );
      fprintf( fp, "%s", buf );

      /* now give out vital statistics as per identify */

      switch ( obj->item_type )
      {
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
          mprintf( sizeof(buf), buf, "Level %d spells of:", obj->value[0] );
          fprintf( fp, "%s", buf );

          if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
          {
            fprintf( fp, " '" );
            fprintf( fp, "%s", skill_table[obj->value[1]].name );
            fprintf( fp, "'" );
          }

          if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
          {
            fprintf( fp, " '" );
            fprintf( fp, "%s", skill_table[obj->value[2]].name );
            fprintf( fp, "'" );
          }

          if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
          {
            fprintf( fp, " '" );
            fprintf( fp, "%s", skill_table[obj->value[3]].name );
            fprintf( fp, "'" );
          }

          fprintf( fp, ".\n" );
          break;

        case ITEM_WAND:
        case ITEM_STAFF:
          mprintf( sizeof(buf), buf, "Has %d(%d) charges of level %d",
                   obj->value[1], obj->value[2], obj->value[0] );
          fprintf( fp, "%s", buf );

          if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
          {
            fprintf( fp, " '" );
            fprintf( fp, "%s", skill_table[obj->value[3]].name );
            fprintf( fp, "'" );
          }

          fprintf( fp, ".\n" );
          break;

        case ITEM_WEAPON:
          fprintf( fp, "Weapon type is " );
          switch (obj->value[0])
          {
            case(WEAPON_EXOTIC)     : fprintf(fp, "exotic\n");
              break;
            case(WEAPON_SWORD)      : fprintf(fp, "sword\n");
              break;
            case(WEAPON_DAGGER)     : fprintf(fp, "dagger\n");
              break;
            case(WEAPON_SPEAR)  : fprintf(fp, "spear/staff\n");
              break;
            case(WEAPON_MACE)   : fprintf(fp, "mace/club\n");
              break;
            case(WEAPON_AXE)    : fprintf(fp, "axe\n");
              break;
            case(WEAPON_FLAIL)  : fprintf(fp, "flail\n");
              break;
            case(WEAPON_WHIP)   : fprintf(fp, "whip\n");
              break;
            case(WEAPON_POLEARM)    : fprintf(fp, "polearm\n");
              break;
						case(WEAPON_CROSSBOW)   : fprintf(fp, "crossbow\n");
							break;
            default         :
              fprintf(fp, "unknown\n");
              break;
          }
          if (obj->pIndexData->new_format)
            mprintf(sizeof(buf), buf,"Damage is %dd%d (average %d)\n",
                    obj->value[1],obj->value[2],
                    (1 + obj->value[2]) * obj->value[1] / 2);
          else
            mprintf( sizeof(buf), buf, "Damage is %d to %d (average %d)\n",
                     obj->value[1], obj->value[2],
                     ( obj->value[1] + obj->value[2] ) / 2 );
          fprintf( fp, "%s", buf );

          if (obj->value[4])  /* weapon flags */
          {
            mprintf(sizeof(buf), buf,"Weapons flags: %s\n",weapon_bit_name(obj->value[4]));
            fprintf(fp, "%s", buf);
          }
          break;

        case ITEM_ARMOR:
          mprintf( sizeof(buf),  buf,
                   "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n",
                   obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
          fprintf( fp, "%s", buf );
          break;
      }  /* switch */

      for ( paf = obj->affected; paf != NULL; paf = paf->next )
      {
        mprintf( sizeof(buf), buf, "Affects %s by %d, level %d",
                 affect_loc_name( paf->location ), paf->modifier,paf->level );
        fprintf( fp, "%s", buf );
        /* added by Rahl */
        if ( paf->duration > -1 )
          mprintf( sizeof(buf), buf, ", %d hours.\n", paf->duration );
        else
          mprintf( sizeof(buf), buf, ".\n" );
        fprintf( fp, "%s", buf );
        if ( paf->bitvector )
        {
          switch ( paf->where )
          {
            case TO_AFFECTS:
              mprintf( sizeof(buf), buf, "Adds %s affect.\n",
                       affect_bit_name( paf->bitvector ) );
              break;
            case TO_WEAPON:
              mprintf( sizeof(buf), buf, "Adds %s weapon flags.\n",
                       weapon_bit_name( paf->bitvector ) );
              break;
            case TO_OBJECT:
              mprintf( sizeof(buf), buf, "Adds %s object flag.\n",
                       extra_bit_name( paf->bitvector ) );
              break;
            case TO_IMMUNE:
              mprintf( sizeof(buf), buf, "Adds immunity to %s.\n",
                       imm_bit_name( paf->bitvector ) );
              break;
            case TO_RESIST:
              mprintf( sizeof(buf), buf, "Adds resistance to %s.\n",
                       imm_bit_name( paf->bitvector ) );
              break;
            case TO_VULN:
              mprintf( sizeof(buf), buf, "Adds vulnerability to %s.\n",
                       imm_bit_name( paf->bitvector ) );
              break;
            default:
              mprintf( sizeof(buf), buf, "Unknown bit %d %d\n",
                       paf->where, paf->bitvector );
              break;
          }
          fprintf( fp, "%s", buf );
        }  /* if */
      }  /* for */

      if (!obj->enchanted)
        for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
        {
          mprintf( sizeof(buf), buf, "Affects %s by %d, level %d.\n",
                   affect_loc_name( paf->location ), paf->modifier,paf->level );
          fprintf( fp, "%s", buf );
          if ( paf->bitvector )
          {
            switch ( paf->where )
            {
              case TO_AFFECTS:
                mprintf( sizeof(buf), buf, "Adds %s affect.\n",
                         affect_bit_name( paf->bitvector ) );
                break;
              case TO_WEAPON:
                mprintf( sizeof(buf), buf, "Adds %s weapon flags.\n",
                         weapon_bit_name( paf->bitvector ) );
                break;
              case TO_OBJECT:
                mprintf( sizeof(buf), buf, "Adds %s object flag.\n",
                         extra_bit_name( paf->bitvector ) );
                break;
              case TO_IMMUNE:
                mprintf( sizeof(buf), buf, "Adds immunity to %s.\n",
                         imm_bit_name( paf->bitvector ) );
                break;
              case TO_RESIST:
                mprintf( sizeof(buf), buf, "Adds resistance to %s.\n",
                         imm_bit_name( paf->bitvector ) );
                break;
              case TO_VULN:
                mprintf( sizeof(buf), buf, "Adds vulnerability to %s.\n",
                         imm_bit_name( paf->bitvector ) );
                break;
              default:
                mprintf( sizeof(buf), buf, "Unknown bit %d %d\n",
                         paf->where, paf->bitvector );
                break;
            }      /* switch */
            fprintf( fp, "%s", buf );
          }       /* if */
        }   /* for */
      fprintf( fp, "\n" );
      extract_obj( obj );

    }       /* if */
  /* close file */
  fclose(fp);
  nFilesOpen--;

  /* start printing out room data */
  /*   fp = fopen("../room.txt","w");

       fprintf(fp,"\nRoom Analysis\n");
       fprintf(fp,  "---------------\n");
       nMatch = 0;
       for (vnum = 0; nMatch < top_vnum_room; vnum++)
       if ((pRoomIndex = get_room_index(vnum)) != NULL)
       {
       nMatch++;
       mprintf( sizeof(buf), buf, "Name: '%s.'\nArea: '%s'.\n",
       pRoomIndex->name,
       pRoomIndex->area->name );
       fprintf( fp, "%s", buf );

       mprintf( sizeof(buf), buf,
       "Vnum: %d.  Sector: %d.  Light: %d.\n",
       pRoomIndex->vnum,
       pRoomIndex->sector_type,
       pRoomIndex->light );
       fprintf( fp, "%s", buf );

       mprintf( sizeof(buf), buf,
       "Room flags: %d.\nDescription:\n%s",
       pRoomIndex->room_flags,
       pRoomIndex->description );
       fprintf( fp, "%s", buf );

       if ( pRoomIndex->extra_descr != NULL )
       {
       EXTRA_DESCR_DATA *ed;

       fprintf( fp, "Extra description keywords: '" );
       for ( ed = pRoomIndex->extra_descr; ed; ed = ed->next )
       {
       fprintf( fp, ed->keyword );
       if ( ed->next != NULL )
       fprintf( fp, " " );
       }
       fprintf( fp, "'.\n" );
       }

       fprintf( fp, "Characters:" );
       for ( rch = pRoomIndex->people; rch; rch = rch->next_in_room )
       {
       fprintf( fp, " " );
       one_argument( rch->name, buf );
       fprintf( fp, "%s", buf );
       }
    
       fprintf( fp, ".\nObjects:   " );
       for ( obj = pRoomIndex->contents; obj; obj = obj->next_content )
       {
       fprintf( fp, " " );
       one_argument( obj->name, buf );
       fprintf( fp, "%s", buf );
       }
       fprintf( fp, ".\n" );

       for ( door = 0; door <= 5; door++ )
       {
       EXIT_DATA *pexit;

       if ( ( pexit = pRoomIndex->exit[door] ) != NULL )
       {
       mprintf( sizeof(buf), buf,
       "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\nKeyword: '%s'.  Description: %s",
       door,
       (pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
       pexit->key,
       pexit->exit_info,
       pexit->keyword,
       pexit->description[0] != '\0' ? pexit->description : "(none).\n" );
       fprintf( fp, "%s", buf );
       }
       }

       }
  */
  /* close file */
  /*    fclose(fp); */

  fpReserve = fopen( NULL_FILE, "r" );
  nFilesOpen++;
  send_to_char( "Done writing files...\n\r", ch );
  send_to_char("FINISHED with DUMP operation.\n\r",ch);
  REMOVE_BIT(ch->active_flags, ACTIVE_THREAD);
  pthread_exit(NULL);
}

/*From: The Anomaly <anomaly@voicenet.com>

I admit, this was something of a joke when it was suggested to me about
10 minutes ago, but then again, I'm sure there are some sadistic imps out
there who might like this.  If you do actually add this to your mud, I
request that you use it wisely and hopefully not frequently.

You may have to fiddle with this to get it to work. I wrote it in 5
minutes, and I'm sure there are some bugs. If you can't get it to work,
you probably shouldn't be doing anything except looking for a coder. It's
reasonably straightforward.

Now, without further ado, the wizskill addlag:

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
void do_addlag(CHAR_DATA *ch, char *argument)
{

  CHAR_DATA *victim;
  char arg1[MAX_STRING_LENGTH];
  int x;

  argument = one_argument(argument, arg1);

  if (arg1[0] == '\0')
  {
    send_to_char("addlag to who?", ch);
    return;
  }

  if ((victim = get_char_world(ch, arg1)) == NULL)
  {
    send_to_char("They're not here.", ch);
    return;
  }

  if ((x = atoi(argument)) <= 0)
  {
    send_to_char("That makes a LOT of sense.", ch);
    return;
  }

  if (x > 100)
  {
    send_to_char("There's a limit to cruel and unusual punishment", ch);
    return;
  }

  /*  send_to_char("Somebody REALLY didn't like you", victim);*/
  WAIT_STATE(victim, x);
  send_to_char("Adding lag now...", ch);
  return;
}

void do_nodigmove(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
  {
    send_to_char("You are a NPC and cannot be canceled anyways.\n\r",ch);
  }
  else
  {
    if (IS_SET(ch->act,PLR_NODIGMOVE))
    {
      send_to_char("You will now move when you DIG.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NODIGMOVE);
    }
    else
    {
      send_to_char("You will no longer move when you dig.\n\r",ch);
      SET_BIT(ch->act,PLR_NODIGMOVE);
    }
  }
}

/*
* Add this somewhere in act_wiz.c
*/
void do_objaff(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  AFFECT_DATA *paf;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  int affect_modify = 0, bit = 0, enchant_type, pos, i;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
  {
    send_to_char("Syntax for applies: objaff <object> <apply type> <value>\n\r",ch);
    send_to_char("Apply Types: hp str dex int wis con mana\n\r", ch);
    send_to_char("             ac move hitroll damroll saves\n\r\n\r", ch);
    send_to_char("Syntax for affects: objaff <object> affect <affect name>\n\r",ch);
    send_to_char("Affect Names: blind invisible detect_evil detect_invis detect_magic\n\r",ch);
    send_to_char("              detect_hidden detect_good sanctuary faerie_fire infrared\n\r",ch);
    send_to_char("              curse poison protect_evil protect_good sneak hide sleep charm\n\r", ch);
    send_to_char("              flying pass_door haste calm plague weaken dark_vision berserk\n\r", ch);
    send_to_char("              swim regeneration slow\n\r", ch);
    return;
  }

  if ((obj = get_obj_here(ch,arg1)) == NULL)
  {
    send_to_char("No such object exists!\n\r",ch);
    return;
  }

  if (!str_prefix(arg2,"hp"))
    enchant_type=APPLY_HIT;
  else if (!str_prefix(arg2,"str"))
    enchant_type=APPLY_STR;
  else if (!str_prefix(arg2,"dex"))
    enchant_type=APPLY_DEX;
  else if (!str_prefix(arg2,"int"))
    enchant_type=APPLY_INT;
  else if (!str_prefix(arg2,"wis"))
    enchant_type=APPLY_WIS;
  else if (!str_prefix(arg2,"con"))
    enchant_type=APPLY_CON;
  else if (!str_prefix(arg2,"mana"))
    enchant_type=APPLY_MANA;
  else if (!str_prefix(arg2,"move"))
    enchant_type=APPLY_MOVE;
  else if (!str_prefix(arg2,"ac"))
    enchant_type=APPLY_AC;
  else if (!str_prefix(arg2,"hitroll"))
    enchant_type=APPLY_HITROLL;
  else if (!str_prefix(arg2,"damroll"))
    enchant_type=APPLY_DAMROLL;
  else if (!str_prefix(arg2,"saves"))
    enchant_type=APPLY_SAVING_SPELL;
  else if (!str_prefix(arg2,"affect"))
    enchant_type=APPLY_SPELL_AFFECT;
  else
  {
    send_to_char("That apply is not possible!\n\r",ch);
    return;
  }

  if (enchant_type==APPLY_SPELL_AFFECT)
  {
    for (pos = 0; affect_flags[pos].name != NULL; pos++)
      if (!str_cmp(affect_flags[pos].name,arg3))
        bit = affect_flags[pos].bit;
  }
  else
  {
    if ( is_number(arg3) )
      affect_modify=atoi(arg3);
    else
    {
      send_to_char("Applies require a value.\n\r", ch);
      return;
    }
  }

  affect_enchant(obj);

  /* create the affect */
  paf = new_affect();
  paf->where     = TO_AFFECTS;
  paf->type      = 0;
  paf->level     = ch->level;
  paf->duration  = -1;
  paf->location  = enchant_type;
  paf->modifier  = affect_modify;
  paf->bitvector = bit;

  if ( enchant_type == APPLY_SPELL_AFFECT )
  {
    /* make table work with skill_lookup */
    for ( i=0 ; arg3[i] != '\0'; i++ )
    {
      if ( arg3[i] == '_' )
        arg3[i] = ' ';
    }
    paf->type = skill_lookup(arg3);
  }

  paf->next = obj->affected;
  obj->affected = paf;
  return;
}

void do_rwhere(CHAR_DATA *ch, char *argument )
{
  char buf[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  AREA_DATA *area;
  ROOM_INDEX_DATA *room;
  int number = 0, max_found = 500, vnum;

  buffer = new_buf();

  if (argument[0] == '\0')
  {
    send_to_char("Syntax: rwhere <room name>\n\r",ch);
    return;
  }

  for ( area = area_first; area; area = area->next )
  {
    for ( vnum = area->min_vnum; vnum <= area->max_vnum; vnum++ )
    {
      if ( !( room = get_room_index( vnum ) ) )
        continue;
      strip_color(buf,to_upper(room->name));

      if ( !strstr( buf,to_upper(argument) ) )
        continue;

      ++number; /*count it if we found a match*/

      mprintf( sizeof(buf), buf, "%3d) [%5d] %s (%s)\n\r",
               number, vnum, room->name, area->name );
      add_buf( buffer, buf );

      if ( number >= max_found )
        break;
    }
    if ( number >= max_found )
      break;
  }

  if ( !number )
    send_to_char( "No rooms matched your search.\n\r", ch );
  else
    page_to_char(buf_string(buffer),ch);

  free_buf(buffer);
}

void do_smite( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Smite whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( get_trust( victim ) >= get_trust( ch ) )
  {
    send_to_char( "You failed.\n\r", ch );
    return;
  }


  victim->hit = 1;
  victim->mana = 1;
  victim->move = 1;
  printf_to_char( victim,
                  "{wYou scream in agony as a {bbo{Wl{rt{w of {wli{Wght{yn{Yi{yng{x sent from %s smites you where you stand.",
                  ch->name );
  send_to_char("You smite them, sending the wrath of the gods.\n\r",ch);
  act("$n drops to $s knees, struck down by a {bbo{Wl{rt{w of {wli{Wght{yn{Yi{yng{x from an angry God.",victim, NULL, NULL, TO_ROOM);
  save_char_obj( victim, FALSE );
  return;
}

void do_godname( CHAR_DATA *ch, char *argument )
{

  if (!IS_IMMORTAL(ch))
  {
    send_to_char("Who do you think you are?  A God?\n\r",ch);
    return;
  }


  if ( argument[0] == '\0' )
  {
    send_to_char( "Syntax: GODNAME NAME \n\r", ch );
    return;
  }

  if (is_name(argument,"reset"))
  {
    free_string(ch->short_descr);
    ch->short_descr = str_dup("", ch->short_descr);
    send_to_char("Your Godname has been reset.\n\r",ch);
    return;
  }
  if (colorstrlen(argument) > 20)
  {
    send_to_char("Eeek, your name is too long.\n\r",ch);
    return;
  }
  free_string(ch->short_descr);
  ch->short_descr = str_dup(argument,ch->short_descr);
  printf_to_char(ch,"Now do a who. %s\n\r",ch->short_descr);
}

void do_gen_denylist(CHAR_DATA *ch, char *argument)
{

  struct dirent *dp;
  DIR *dirptr;
  int return_int, return_int2;

  send_to_char("Generating Deny List.... \n\r",ch);

  if ((dirptr = opendir(PLAYER_DIR)) == NULL)
  {
    send_to_char("Player dir unable to be opened\n\r",ch);
    return;
  }

  while ((dp=readdir(dirptr)) != NULL)
  {
    /* read in filename */
    return_int = memcmp(&dp->d_name[0],".",1);
    return_int2 = memcmp(&dp->d_name[0],"..",2);

    if ((return_int != 0 ) && (return_int2 != 0 ))
    {
      /*Get Start and Stop Time for DERG File here!!  */
      if (check_if_denied_file(dp->d_name))
      {
        add_to_denied_file(dp->d_name);
      }
    }
  }

  closedir(dirptr);

  if ((dirptr = opendir(DEAD_PLAYER_DIR)) == NULL)
  {
    send_to_char("OLD Player dir unable to be opened\n\r",ch);
    return;
  }

  while ((dp=readdir(dirptr)) != NULL)
  {
    /* read in filename */
    return_int = memcmp(&dp->d_name[0],".",1);
    return_int2 = memcmp(&dp->d_name[0],"..",2);

    if ((return_int != 0 ) && (return_int2 != 0 ))
    {
      /*Get Start and Stop Time for DERG File here!!  */
      if (check_if_denied_file(dp->d_name))
      {
        add_to_denied_file(dp->d_name);
      }
    }
  }

  closedir(dirptr);
  send_to_char("Done..\n\r",ch);

}

void reset_denied_file(char *filename)
{
  FILE *fp;
  DENIED_DATA *dnd;
  remove(filename);
  if ((fp = fopen(DENIED_FILE,"a")) != NULL)
  {
    for (dnd = denied_list; dnd; dnd = dnd->next)
    {
      fprintf(fp,"%s\n",dnd->name);
    }
  }
  else
    return;
  fclose(fp);
}

bool check_if_denied_file(char *filename)
{
  DESCRIPTOR_DATA d;

  d.original = NULL;
  if (!load_char_obj(&d, capitalize(filename))) /* char pfile exists? */
  {
    bugf("{RCheck If Denied:No such player: {W%s{x.\n\r", capitalize(filename));
    free_char(d.character);
    return(FALSE);
  }
  d.character->desc = NULL; /* safe than sorry */
  if (IS_SET(d.character->act, PLR_DENY) && !(d.character->pcdata->denytime))
  {
    nuke_pets(d.character,FALSE);
    free_char(d.character);
    return(TRUE);
  }
  nuke_pets(d.character,FALSE);
  free_char(d.character);
  return(FALSE);

}

int add_to_denied_file(char *name)
{
  FILE *fp;
  DENIED_DATA *denied;
  denied = new_denied();
  denied->next = denied_list;
  denied_list = denied;
  denied->name = str_dup(name,denied->name);
  if ((fp = fopen(DENIED_FILE,"a")) != NULL)
  {
    fprintf(fp,"%s\n",name);
    fclose(fp);
  }
  return TRUE;
}

int read_denied_file()
{
  FILE *fp;
  char buf[MSL];
  logit("Reading from Denied file.\n\r");
  if ((fp = fopen(DENIED_FILE,"r")) != NULL)
  {
    while (get_next_line(fp,buf) != EOF)
    {
      DENIED_DATA *denied;
      denied = new_denied();
      denied->next = denied_list;
      denied_list = denied;
      denied->name = str_dup(buf,denied->name);
    }
    fclose(fp);
  }
  return TRUE;
}

void do_show_denied_list(CHAR_DATA *ch, char *argument)
{
  BUFFER *buffer;
  DENIED_DATA *dnd;
  buffer = new_buf();
  for (dnd = denied_list; dnd; dnd = dnd->next)
    bprintf(buffer,"Denied: %s\n\r",dnd->name);
  page_to_char(buf_string(buffer),ch);
  free_buf(buffer);
}


void do_logsearch( CHAR_DATA *ch, char *argument )
{
  pthread_t thread;

  if (IS_SET(ch->active_flags, ACTIVE_THREAD))
  {
    send_to_char("Sorry, a Fixed buffer operation is currently underway for you.\n\r",ch);
    return;
  }
  SET_BIT(ch->active_flags, ACTIVE_THREAD);
  /* serious kludge here */
  pass.ch = ch;
  free_string(pass.args);
  pass.args = str_dup(argument, pass.args);
  pthread_create( &thread, NULL,(void *)logsearch_do, (void *)NULL);
  pthread_detach(thread);
}

void logsearch_do()
{
  DIR *logdir;
  CHAR_DATA *ch;
  struct dirent *logstream;
  char *logfile = NULL;
  char *logline;
  char buf[MSL];
  FILE *openlog;
  char logtoopen[MSL];
  int count=0;
  int max_count = 1000;

  ch = pass.ch;

  send_to_char("Starting Logsearch Fixed buffer Operation. \n\r",ch);
  clear_buf(ch->fixed_buffer);

  if ((logdir = opendir(LOG_DIR)) == NULL)
  {
    send_to_char("Can't open the log directory stream.\n\r", ch);
    bug("logsearch: can't open the logdir", 0);
    REMOVE_BIT(ch->active_flags, ACTIVE_THREAD);
    pthread_exit(NULL);
  }

  while ((logstream = readdir(logdir)) != NULL)
  {
    logfile = logstream->d_name;

    if (logfile == NULL)
    {
      send_to_char("There are no logs in the log directory.\n\r", ch);
      closedir(logdir);
      REMOVE_BIT(ch->active_flags, ACTIVE_THREAD);
      pthread_exit(NULL);
    }

    if (!strstr(logfile,"log"))
      continue;

    mprintf(sizeof(logtoopen), logtoopen,"%s/%s",LOG_DIR,logfile);

    if ((openlog = fopen(logtoopen,"r")) == NULL)
    {
      bug("logsearch: couldn't open latest logfile.", 0);
      send_to_char("logsearch: couldn't open latest logfile.",ch);
      closedir(logdir);
      REMOVE_BIT(ch->active_flags, ACTIVE_THREAD);
      pthread_exit(NULL);
    }


    while ((logline = fgets(buf, sizeof(buf), openlog)) != NULL)
    {
      if (strstr(logline,pass.args))
      {
        add_buf(ch->fixed_buffer,logline);
        add_buf(ch->fixed_buffer,"\r");
        if (count++ >= max_count)
        {
          REMOVE_BIT(ch->active_flags, ACTIVE_THREAD);
          send_to_char("Max count is reached for Logsearch.\n\r",ch);
          send_to_char("Complete the Logsearch by activing 'finish'.\n\r",ch);
          pthread_exit(NULL);
        }
      }
    }
    fclose(openlog);
  }

  send_to_char("Complete the Logsearch by activing 'finish'.\n\r",ch);
  REMOVE_BIT(ch->active_flags, ACTIVE_THREAD);
  closedir(logdir);
  pthread_exit(NULL);
}

void do_show_fixed_buffer(CHAR_DATA *ch, char *argument)
{
  if (IS_SET(ch->active_flags, ACTIVE_THREAD))
  {
    send_to_char("Current Thread operation Active.\n\r",ch);
    return;
  }
  else
  {
    send_to_char("Output of last Fixed buffer Operation.\n\r",ch);
    page_to_char(buf_string(ch->fixed_buffer),ch);
    send_to_char("Output complete.\n\r",ch);
  }
}


void show_penalty_syntax(CHAR_DATA *ch)
{
  send_to_char("Syntax:punish <victim> [add|rem] <flag> <timeoption>/\n\r",ch);
  send_to_char("Flags:\n\r",ch);
  send_to_char("  killer     -  Killer.\n\r",ch);
  send_to_char("  thief      -  Thief.\n\r",ch);
  send_to_char("  twit       -  Twit.\n\r",ch);
  send_to_char("  violent    -  Violent.\n\r",ch);
  send_to_char("  notedeny   -  Notedeny.\n\r",ch);
  send_to_char("  bailed     -  Bailed (Turn off only).\n\r",ch);
  send_to_char("timeoption \n\r",ch);
  send_to_char("  for twit it is the time of the twit.\n\r",ch);

}


void do_punish( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  AFFECT_DATA af;
  int time;
  CHAR_DATA *victim;
  enum penalty_action_e action;
  bool add;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
  {
    show_penalty_syntax(ch);
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) )
  {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }

  if (!str_cmp(arg2, "add")) add = TRUE;
  else
    if (!str_cmp(arg2, "rem")) add = FALSE;
    else
    {
      show_penalty_syntax(ch);
      return;
    }
  if (!str_cmp(arg3, "killer")) action = pen_killer_e;
  else
    if (!str_cmp(arg3, "thief")) action = pen_thief_e;
    else
      if (!str_cmp(arg3, "twit")) action = pen_twit_e;
      else
        if (!str_cmp(arg3, "violent")) action = pen_violent_e;
        else
          if (!str_cmp(arg3, "notedeny")) action = pen_notedeny_e;
          else
            if (!str_cmp(arg3, "bailed")) action = pen_bailed_e;
            else
            {
              show_penalty_syntax(ch);
              return;
            }


  switch (action)
  {
    case pen_killer_e:
      if (!add && IS_SET(victim->act, PLR_KILLER) )
      {
        REMOVE_BIT( victim->act, PLR_KILLER );
        send_to_char( "Killer flag removed.\n\r", ch );
        send_to_char( "{CREMOVED: You are no longer a KILLER.{x\n\r", victim );
      }
      else if (add && !IS_SET(victim->act, PLR_KILLER) )
      {
        SET_BIT( victim->act, PLR_KILLER );
        send_to_char( "Killer flag set.\n\r", ch );
        send_to_char( "{RPUNISHED: You are a now KILLER or an Accomplice.{x\n\r", victim );
      }
      else
        send_to_char("Flag already set.\n\r",ch);
      break;
    case pen_thief_e:
      if (!add && IS_SET(victim->act, PLR_THIEF) )
      {
        REMOVE_BIT( victim->act, PLR_THIEF );
        send_to_char( "Thief flag removed.\n\r", ch );
        send_to_char( "{CREMOVED: You are no longer a THIEF.{x\n\r", victim );
      }
      else if (add && !IS_SET( victim->act, PLR_THIEF ) )
      {
        SET_BIT( victim->act, PLR_THIEF );
        send_to_char( "Thief flag set.\n\r", ch );
        send_to_char( "{RPUNISHED: You are now a THIEF, or an Accomplice.{x\n\r", victim );
      }
      else
      {
        send_to_char( "Flag already set.\n\r", ch );
      }
      break;
    case pen_twit_e:
      if (!add && IS_SET(victim->act, PLR_TWIT) )
      {
        remove_affect(victim, TO_ACT, PLR_TWIT);
        REMOVE_BIT( victim->act, PLR_TWIT );
        send_to_char( "Twit flag removed.\n\r", ch );
        send_to_char( "{CREMOVED: You are no longer a TWIT.{x\n\r", victim );
      }
      else if (add &&  !IS_SET( victim->act, PLR_TWIT ) )
      {
        if (is_number(argument))
        {
          time = atoi(argument);

          af.where     = TO_ACT;
          af.type     = gsn_twit;
          af.level     = victim->level;
          af.duration  = time;
          af.location  = APPLY_NONE;
          af.modifier  = 0;
          af.bitvector = PLR_TWIT;
          affect_to_char( victim, &af );
          send_to_char("{RPUNISHED: You have been twitted by the gods for horrible infractions.{x\n\r",victim);
          printf_to_char(ch, "Twitted for %d ticks.\n\r", time);
        }
        else
        {
          SET_BIT( victim->act, PLR_TWIT );

          send_to_char( " Twit flag set.\n\r", ch );
          if ( ch != victim )
            send_to_char( "{RPUNISHED:You are now a TWIT you TWIT you.{x\n\r", victim  );
        }
      }
      else
      {
        send_to_char( "Flag already set.\n\r", ch );
      }
      break;
    case pen_bailed_e:
      if (add)
        send_to_char("Sorry this cannot be accomplished.\n\r",ch);
      else if (!add && victim->bailed )
      {
        victim->bailed = 0;
        send_to_char( "Bailed flag removed.\n\r", ch );
        send_to_char( "{CREMOVED: You are no longer a Bailed.{x\n\r", victim );
      }
      break;
    case pen_violent_e:
      if ( !add && IS_SET(victim->act, PLR_VIOLENT) )
      {
        REMOVE_BIT( victim->act, PLR_VIOLENT );
        send_to_char( "Violent flag removed.\n\r", ch );
        send_to_char( "{CREMOVED: You are no longer VIOLENT.{x\n\r", victim );
      }
      else if    (add &&  !IS_SET( victim->act, PLR_VIOLENT ) )
      {
        SET_BIT( victim->act, PLR_VIOLENT);
        send_to_char( "Violent flag set.\n\r", ch );
        send_to_char("{RPUNISHED: You are now a Violent!{x\n\r",victim);
        victim->vflag_timer = 10;
      }
      else
      {
        send_to_char( "Flag already set.\n\r", ch );
      }
      break;
    case pen_notedeny_e:
      if (!add && IS_SET(victim->pen_flags, PEN_NOTE) )
      {
        REMOVE_BIT( victim->pen_flags, PEN_NOTE );
        send_to_char( "Note Deny flag removed.\n\r", ch );
        send_to_char( "{CREMOVED: You are no longer denied note access. Do not abuse this function.{x\n\r", victim );
      }
      else if (add && !IS_SET( victim->pen_flags, PEN_NOTE ) )
      {
        SET_BIT( victim->pen_flags, PEN_NOTE);
        send_to_char( "Note DENY flag set.\n\r", ch );
        if ( ch != victim )
          send_to_char( "{RPUNISHED: You are now NOTE DENIED.{x\n\r", victim);
      }
      else
      {
        send_to_char( "They are already NOTE denied.\n\r", ch );
      }
      break;
    default:
      send_to_char("Error:\n\r",ch);
      break;
  }
}

int simple_comp_two_obj(OBJ_DATA *obj, OBJ_DATA *wobj)
{

  if ( obj->pIndexData    != wobj->pIndexData)
    return 1;

  if (
    strcmp(obj->name, wobj->name)
    ||   obj->pIndexData->vnum != wobj->pIndexData->vnum
    ||   strcmp(obj->short_descr,wobj->short_descr)
    ||   strcmp(obj->description,wobj->description))
    return 2;

  if (
    obj->material     != wobj->material
    ||   obj->extra_flags    != wobj->extra_flags
    ||   obj->wear_flags    != wobj->wear_flags
    ||   obj->item_type    != wobj->item_type
    ||   obj->weight    != wobj->weight
    ||   obj->condition    != wobj->condition
    ||   obj->level     != wobj->level
    ||   obj->timer     != wobj->timer
    ||   obj->cost       != wobj->cost )
    return 3;

  if (  obj->value[0]    != wobj->value[0]
        ||   obj->value[1]    != wobj->value[1]
        ||   obj->value[2]    != wobj->value[2]
        ||   obj->value[3]    != wobj->value[3]
        ||   obj->value[4]    != wobj->value[4]
        ||   obj->value[5]    != wobj->value[5]
        ||   obj->value[6]    != wobj->value[6] )
    return 4;

  if (   ( IS_NULLSTR( obj->owner ) && !IS_NULLSTR( wobj->owner ) )
         ||   ( !IS_NULLSTR( obj->owner ) && IS_NULLSTR( wobj->owner ) )
         ||   ( !IS_NULLSTR( obj->owner ) && !IS_NULLSTR( wobj->owner )
                && str_cmp( obj->owner, wobj->owner ) )
     )
    return 5;
  return 0;
}

void do_check_valid_eq(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *pch;
  OBJ_INDEX_DATA *pObjIndex;
  int tmp;
  BUFFER *buffer;
  OBJ_DATA *obj, *tobj;

  buffer = new_buf();
  for (pch=player_list; pch; pch=pch->next_player)
  {
    for (obj=pch->carrying; obj; obj=obj->next_content)
    {
      if (      obj->enchanted
                ||     obj->affected
                ||     obj->contains
                ||     obj->extra_descr
                ||     !obj->pIndexData->new_format )
        continue;

      if (IS_SET(obj->extra_flags, ITEM_RESTRING))
        continue;

      if ((pObjIndex = get_obj_index(obj->pIndexData->vnum)) != NULL)
      {
        tobj = create_object( pObjIndex, 0 );
        if ((tmp = simple_comp_two_obj(obj, tobj)) > 0)
          bprintf(buffer, "%s: Vnum %d. Name %s is different- Reason[%d].\n\r",
                  pch->name, obj->pIndexData->vnum, obj->short_descr, tmp);
        extract_obj(tobj);
      }
    }
  }
  /* Method Brute Force
     for (vnum = 0; nMatch < top_obj_index; vnum++)
     if ((pObjIndex = get_obj_index(vnum)) != NULL)
     {
     tobj = create_object( pObjIndex, 0 );
     nMatch++;
     for (pch=player_list; pch; pch=pch->next_player)
     {
     for (obj=pch->carrying; obj; obj=obj->next_content)
     {
     if (      obj->enchanted
     ||     obj->affected
     ||     obj->contains
     ||     obj->extra_descr
     ||     !obj->pIndexData->new_format )
     continue;
     if (simple_comp_two_obj(obj, tobj))
     bprintf(buffer, "%s: Vnum %d. Name %s is different.\n\r",
     pch->name, obj->pIndexData->vnum, obj->short_descr);
    
     }
     }
     extract_obj(tobj);
     }
  */
  page_to_char(buf_string(buffer),ch);
  free_buf(buffer);

}


void do_setpassword(CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA d;
  DENIED_DATA *dnd;
  char arg[MAX_STRING_LENGTH];
  char *pwdnew;
  char *p;
  CHAR_DATA *victim;
  bool load_char = FALSE;

  if (argument[0] == '\0' ||
      argument[0] == '/'  ||
      argument[0] == '\\' ||
      argument[0] == '.')
  {
    send_to_char("Set Password for who?\n\r", ch);
    return;
  }
  argument = one_argument(argument, arg);
  smash_tilde(arg);
  for (dnd = denied_list; dnd; dnd = dnd->next)
    if (is_exact_name(capitalize(arg),dnd->name))
    {
      send_to_char("That char is Permanently {RDENIED{x\n\r",ch);
      return;
    }

  if (((victim = get_char_world(ch, arg)) != NULL) && (!IS_NPC(victim)))
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

  if ( strlen(argument) < 5 )
  {
    send_to_char("New Password must be at least five characters long.\n\r",ch);
    return;
  }


  if (((victim = get_char_world(ch, arg)) != NULL) && (!IS_NPC(victim)))
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
  pwdnew = crypt( argument, victim->name );
  for ( p = pwdnew; *p != '\0'; p++ )
  {
    if ( *p == '~' )
    {
      send_to_char("New password not acceptable, try again.\n\r ",ch);
      if (load_char)
      {
        nuke_pets(d.character,FALSE);
        free_char(d.character);
      }
      return;
    }
  }
  free_string( victim->pcdata->pwd );
  victim->pcdata->pwd    = str_dup( pwdnew, victim->pcdata->pwd );
#if MEMDEBUG
  free_string (victim->pcdata->memdebug_pwd);
  victim->pcdata->memdebug_pwd = str_dup(pwdnew, victim->pcdata->memdebug_pwd);
#endif
  save_char_obj(d.character, FALSE);
  if (load_char)
  {
    nuke_pets(d.character,FALSE);
    free_char(d.character);
  }

  send_to_char("Password Changed!\n\r",ch);
  return;
}


int count_obj_loss()
{
  int count=0;
  OBJ_DATA *obj, *in_obj;

  for ( obj = object_list; obj != NULL; obj = obj->next )
  {
    for ( in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj );

    if ( in_obj->carried_by  && in_obj->carried_by &&
         in_obj->carried_by->in_room)
    {
      continue;
    }
    else if (in_obj->in_room  && in_obj->in_room)
    {
      continue;
    }
    else  if (auction_info.item == obj)
    {
      continue;
    }
    else
    {
      count++;
    }
  }
  return (count);
}


int count_mob_loss()
{
  CHAR_DATA *victim;
  int count=0;
  for ( victim = char_list; victim != NULL; victim = victim->next )
  {
    if (IS_DELETED(victim))
      continue;
    if ( victim->in_room != NULL)
      continue;
    count ++;
  }
  return count;
}

//New full spellup/restore for all players == level IMP and OWNER
void do_fullrestore( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *d;

  one_argument( argument, arg );

  if ( get_trust(ch) >=  MAX_LEVEL - 1 && !str_cmp(arg,"all"))
  {
    /* cure all */
    for (d = descriptor_list; d != NULL; d = d->next)
    {
      if ( d->connected != CON_PLAYING )
        continue;

      victim = d->character;

      if (victim == NULL || IS_NPC(victim))
        continue;

      if (victim->in_room != NULL)
      {
        if (can_see( victim, ch ) )
          printf_to_char( victim, "You hear %s chanting softly in the distance, suddenly you feel more powerful.\n\r",
                          ch->short_descr != '\0' ? ch->short_descr : ch->name);
        else
          send_to_char( "You hear an unknown entity chanting softly in the distance, suddenly you feel more powerful.\n\r", victim );
      }

      fullrestore_char(victim, ch);
    }

    send_to_char("All active players fullrestored.\n\r",ch);
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if (can_see( victim, ch ) )
    printf_to_char( victim, "You hear %s chanting softly in the distance, suddenly you feel more powerful.\n\r",
                    ch->short_descr != '\0' ? ch->short_descr : ch->name);
  else
    send_to_char( "You hear an unknown entity chanting softly in the distance, suddenly you feel more powerful.\n\r", victim );

  fullrestore_char(victim, ch);

  mprintf(sizeof(buf), buf,"%s just fullrestored %s",
          ch->short_descr != '\0' ? ch->short_descr : ch->name,
          IS_NPC(victim) ? victim->short_descr : victim->name);

  wiznet(buf,NULL,NULL,WIZ_RESTORE,0,get_trust(ch));
  send_to_char( "Ok.\n\r", ch );
  return;
}

void do_snooplist(CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  bool found = FALSE;
  CHAR_DATA *fch;

  printf_to_char(ch, "{D--- {gSNOOP LIST{D ---{x\n\r");
  for (d= descriptor_list; d; d=d->next)
  {
    if (d->snoop_by != NULL)
    {
      if (d->character)
      {
        fch = d->snoop_by->character;

        if (!fch)
          continue;

        if ( get_trust(ch) >= get_trust(fch) )
        {
          printf_to_char(ch, "Char [{W%s{x] snooped by {g%s{x.\n\r", d->character->name, fch->name );
          found = TRUE;
        }
      }
    }
  }
  if (!found)
    printf_to_char(ch,"No snoops found now.\n\r");
}

// Taeloch/Logos' olevel
void do_olevel(CHAR_DATA *ch, char *argument)
{
  extern int top_obj_index;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char oflags[MAX_STRING_LENGTH];
  char srchword[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char arg2[MIL];
  char arg3[MIL];
  BUFFER *buffer;
  OBJ_INDEX_DATA *pObjIndex;
  int vstart = 0;
  int vstop = 32600;//top_obj_index;
  int vnum, level = 0;
  int nMatch = 0, tMatch = 0;
  bool showunlinked = FALSE;
  bool showAll = FALSE;
  bool byKeyword = FALSE;

  argument=one_argument(argument, arg);
  strcpy(arg2,"");
  strcpy(arg3,"");

  if ( IS_NULLSTR(arg)
       ||  !str_prefix( arg, "area" ) )
  {
    vstart = ch->in_room->area->min_vnum;
    vstop = ch->in_room->area->max_vnum;
    showAll = TRUE;
  }
  else
  {
    if (!is_number(arg) )
    {
      byKeyword = TRUE;
      showAll = TRUE;
      strcpy(srchword,arg);
    }
    else
    {
      level = atoi(arg);
      argument = one_argument(argument, arg2);

      if ( !IS_NULLSTR( arg2 ) )
      {
        if ( !str_prefix( arg2, "unlinked" ) )
        {
          argument = one_argument(argument, arg3);
          showunlinked = TRUE;
        }
      }
    }
  } // if non-"area" arg was given

  buffer= new_buf();
  add_buf(buffer,"{c  VNUM {DLvl  {xObject Name                    {gType          {YValue   {WWear Locations\n\r");
  add_buf(buffer,"{D ===== === ============================== =========== ========= =================={x\n\r");

  for (vnum = vstart; vnum <= vstop; vnum++)
  {
    if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
    {
      nMatch++;

      if ( (showAll)
           || (level == pObjIndex->level))
      {
        if ( !showAll)
        {
          if ( !IS_NULLSTR(pObjIndex->clan_name) )
            continue;

          if ( !IS_NULLSTR(arg2)
               &&   !showunlinked
               &&   str_prefix(arg2, item_name(pObjIndex->item_type) ) )
            continue;

          if ( !IS_NULLSTR(arg3)
               &&   str_prefix(arg3, item_name(pObjIndex->item_type) ) )
            continue;

          if ( showunlinked
               &&   (pObjIndex->area->low_range != -1) )
            continue;

          if ( !showunlinked
               &&   (pObjIndex->area->low_range == -1) )
            continue;
        } // !showAll

        if ( byKeyword // if searching by keyword, compare names
             &&  !strstr(pObjIndex->name, srchword ) )
          continue;

// AC, weight, Resets, Extra effs, Damage, if a weapon
// weapons, damage, damtype, avg. Armor, ac. Furniture, position flags. Etc.
        tMatch++;
        strncpy_color(buf2, pObjIndex->short_descr, 30, ' ', TRUE );
        strcpy(oflags,wear_bit_name(pObjIndex->wear_flags));
        oflags[20] = '\0';

        sprintf(buf,
                "{c %5d {D%3d  {x%-28s {g%-12s{Y%7d   {W%-20s{x\n\r",
                pObjIndex->vnum,
                pObjIndex->level,
                buf2,
                item_name(pObjIndex->item_type),
                pObjIndex->cost,
                oflags );
        add_buf(buffer,buf);
      }
    }
  }

  if (!tMatch)
  {
    send_to_char("No objects of that level.\n\r",ch);
  }
  else
  {
    sprintf(buf, "\n\r%d matches found.\n\r",tMatch);
    add_buf(buffer,buf);

    page_to_char(buf_string(buffer),ch);
  }

  free_buf(buffer);
  return;
}

// Taeloch's mlevel -- blame him
void do_mlevel(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char srchword[MAX_STRING_LENGTH];
  BUFFER *buffer;
  MOB_INDEX_DATA *victim;
  int vstart = 0;
  int vstop = 32600;
  int vnum, tMatch = 0, minlevel = 0, maxlevel = 0;
  bool showunlinked = FALSE;
  bool showAll = FALSE;
  bool byKeyword = FALSE;
  argument=one_argument(argument, arg);

  if ( IS_NULLSTR(arg)
       ||  !str_prefix( arg, "area" ) )
  {
    vstart = ch->in_room->area->min_vnum;
    vstop = ch->in_room->area->max_vnum;
    showAll = TRUE;
  }
  else
  {
    if (!is_number(arg) )
    {
      byKeyword = TRUE;
      showAll = TRUE;
      strcpy(srchword,arg);
    }
    else
    {
      minlevel = maxlevel = atoi(arg);
      argument = one_argument(argument, arg);

      if (is_number(arg) )
      {
        maxlevel = atoi(arg);
        argument = one_argument(argument, arg);
      }

      if ( !IS_NULLSTR( arg )
           && ( !str_prefix( arg, "unlinked" ) ) )
        showunlinked = TRUE;
    }
  }

  buffer= new_buf();
  add_buf(buffer,"{cVNUM  {DLvl {xMobile Name          {BAvgHP {GAvgMa {YWealth {RHit {rDamDice  {MAvgAC {wAlign {ySx {WCt{x\n\r");
  add_buf(buffer,"{D----- --- -------------------- ----- ----- ------ --- -------- ----- ----- -- --{x\n\r");

  for ( vnum = vstart; vnum <= vstop; vnum++ )
  {
    if ( ( ( victim = get_mob_index( vnum ) ) != NULL )
         && ( (showAll)
              || ( ( ( victim->level >= minlevel ) && ( victim->level <= maxlevel ) )
                   && !IS_SET(victim->act, ACT_BANKER )
                   && !IS_SET(victim->act, ACT_TRAIN )
                   && !IS_SET(victim->act, ACT_LOCKER )
                   && !IS_SET(victim->act, ACT_IS_HEALER )
                   && !IS_SET(victim->act, ACT_GAIN )
                   && !IS_SET(victim->act, ACT_IS_CHANGER )
                   && !IS_SET(victim->act, ACT_PRACTICE )
                   && ( showunlinked || ( victim->area->low_range != -1) ) ) ) )
    {
      if ( byKeyword // if searching by keyword, compare names
           &&  !strstr(victim->player_name, srchword ) )
        continue;

      tMatch++;
      strncpy_color(buf2, victim->short_descr, 20, ' ', TRUE );

      mprintf(sizeof(buf), buf,
              "{c%5d {D%3d {x%17s {B%5d {G%5d {Y%6d {R%3d {r%2d{Wd{r%2d{W+{r%2d {M%5d {w%5d  {y%s {W%2d{x\n\r",
              victim->vnum,      // MOB VNUM
              victim->level,      // MOB Level
              buf2,              // Short desc
              ( ( ( victim->hit[0] * 1 ) + ( victim->hit[0] * victim->hit[1] ) ) / 2 ) + victim->hit[2],  // Average HP
              ( ( ( victim->mana[0]* 1 ) + ( victim->mana[0]* victim->mana[1]) ) / 2 ) + victim->mana[2], // Average Mana
              victim->wealth,    // Wealth
              victim->hitroll,   // Hitroll
              victim->damage[0], // DamDice
              victim->damage[1], // DamDice Size
              victim->damage[2], // DamDice Bonus
              ( (victim->ac[0] + victim->ac[1] + victim->ac[2] + victim->ac[3]) / 4), // Average Armor Class
              victim->alignment,  // alignment
              ( (victim->sex == SEX_MALE) ? "M" :
                (victim->sex == SEX_FEMALE) ? "F" :
                (victim->sex == 3 ? "R" : "N" ) ), // Sex
              victim->count );      // count
      add_buf( buffer, buf );
    }
  }



  if (!tMatch)
  {
    send_to_char("No NPCs of that level found.\n\r",ch);
  }
  else
  {
    sprintf(buf, "\n\r%d matches found.\n\r",tMatch);
    add_buf(buffer,buf);

    page_to_char(buf_string(buffer),ch);
  }

  free_buf(buffer);
  return;
}

// Taeloch's rsearch -- blame him... oh, and Aarchane, it was his idea
void do_rsearch(CHAR_DATA *ch, char *argument)
{
  ROOM_INDEX_DATA *room;
  BUFFER *buffer;
  char buf[MAX_STRING_LENGTH];
  int vnum, vstart, vstop, rMatch=0;

  if ( IS_NPC(ch) )
  {
    send_to_char("You crazy mob!\n\r",ch);
    return;
  }

  if ( IS_NULLSTR(argument) )
  {
    send_to_char("Search room descriptions for what text?\n\r",ch);
    return;
  }

  vstart = ch->in_room->area->min_vnum;
  vstop = ch->in_room->area->max_vnum;

  buffer= new_buf();
  add_buf(buffer,"{cVNUM   {WRoom name{x\n\r");
  add_buf(buffer,"{D----- -------------------------------{x\n\r");

  for ( vnum = vstart; vnum <= vstop; vnum++ )
  {
    room = get_room_index(vnum);
    if (room != NULL)
    {
      strip_color(buf,room->description);
      if (strstr(buf,argument))
      {
        mprintf(sizeof(buf), buf, "{c%5d  {W%s{x\n\r", vnum, room->name);
        add_buf( buffer, buf );
        rMatch++;
      }
    }
  }

  if (rMatch == 0)
    send_to_char("No rooms found with that text.\n\r",ch);
  else
  {
    sprintf(buf, "\n\r%d matches found.\n\r",rMatch);
    add_buf(buffer,buf);
    page_to_char(buf_string(buffer),ch);
  }

  free_buf(buffer);
  return;
}

// Taeloch's msearch -- blame him... oh, and Aarchane, it was his idea
void do_msearch(CHAR_DATA *ch, char *argument)
{
  MOB_INDEX_DATA *mob;
  BUFFER *buffer;
  char buf[MAX_STRING_LENGTH];
  int vnum, vstart, vstop, mMatch=0;

  if ( IS_NPC(ch) )
  {
    send_to_char("You crazy mob!\n\r",ch);
    return;
  }

  if ( IS_NULLSTR(argument) )
  {
    send_to_char("Search mob descriptions for what text?\n\r",ch);
    return;
  }

  vstart = ch->in_room->area->min_vnum;
  vstop = ch->in_room->area->max_vnum;

  buffer= new_buf();
  add_buf(buffer,"{cVNUM   {WMob name{x\n\r");
  add_buf(buffer,"{D----- -------------------------------{x\n\r");

  for ( vnum = vstart; vnum <= vstop; vnum++ )
  {
    mob = get_mob_index(vnum);
    if (mob != NULL)
    {
      strip_color(buf,mob->description);
      if (strstr(buf,argument))
      {
        mprintf(sizeof(buf), buf, "{c%5d  {W%s{x\n\r", vnum, mob->short_descr);
        add_buf( buffer, buf );
        mMatch++;
      }
    }
  }

  if (mMatch == 0)
    send_to_char("No mobs found with that text.\n\r",ch);
  else
  {
    sprintf(buf, "\n\r%d matches found.\n\r",mMatch);
    add_buf(buffer,buf);
    page_to_char(buf_string(buffer),ch);
  }

  free_buf(buffer);
  return;
}

// Taeloch's hsearch -- blame him... oh, and Aarchane, it was his idea
void do_hsearch(CHAR_DATA *ch, char *argument)
{
  HELP_DATA *pHelp;
  BUFFER *buffer;
  char buf[MAX_STRING_LENGTH];
  int hMatch=0;

  if ( IS_NPC(ch) )
  {
    send_to_char("You crazy mob!\n\r",ch);
    return;
  }

  if ( IS_NULLSTR(argument) )
  {
    send_to_char("Search help files for what text?\n\r",ch);
    return;
  }

  buffer= new_buf();
  add_buf(buffer,"{cHelp  {WHelp Name{x\n\r");
  add_buf(buffer,"{D---- ------------------------------------------------------------------------{x\n\r");

  for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
  {
    strip_color(buf,pHelp->text);
    if (strstr(buf,argument))
    {
      mprintf(sizeof(buf), buf, "{c%4d  {W%s{x\n\r", pHelp->vnum, capitalize(pHelp->keyword));
      add_buf( buffer, buf );
      hMatch++;
    }
  }

  if (hMatch == 0)
    send_to_char("No helps found with that text.\n\r",ch);
  else
  {
    sprintf(buf, "\n\r%d matches found.\n\r",hMatch);
    add_buf(buffer,buf);
    page_to_char(buf_string(buffer),ch);
  }

  free_buf(buffer);
  return;
}
