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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "interp.h"
#include "olc.h"

char *  const  dir_name  []  =
{
  "north", "east", "south", "west", "up", "down"
};

char * const dir_from_name  []  =
{
  "the south", "the west", "the north", "the east", "below",  "above"
};

char *  const  dir_abbr  []  =
{
  "n", "e", "s", "w", "u", "d"
};

const  sh_int  rev_dir    []  =
{
  2, 3, 0, 1, 5, 4
};

const  sh_int  movement_loss  [SECT_MAX]  =
{
  1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6  // The cost for movement for sectors...
  // Why is it here??  Merak...
};



/*
 * Local functions.
 */
int  find_door  args( ( CHAR_DATA *ch, char *arg ) );
bool  has_key    args( ( CHAR_DATA *ch, int key ) );

int arg_to_dirnum( char *arg )
{
  int door;
  if ( arg )
  {
    for ( door = 0; door <= 5; door++ )
      if ( !str_prefix( arg, dir_name[door] ) ) return door;
  }
  return -1;
}

void set_state_room( ROOM_INDEX_DATA *room, int state )
{
  ROOM_INDEX_DATA *to_room = NULL;
  EXIT_DATA *pexit;
  EXIT_DATA *pexit_rev;
  int door;

  if ( !IS_SET( room->room_flags, ROOM_SHIP ) ) return;

  if ( state > 3 ) /* up and down are reserved */
  {
    bug( "set_state_room: Bad state %d.", state );
    return;
  }
  if ( state < -1 ) state = -1; /* we don't like bad states! */

  if ( room->state >= 0 )
  {
    for ( door = 0; door <= 5; door++ )
      /* this finds the direcition-num of the open door */
      if ( !strcmp( room->exit[room->state]->keyword,dir_name[door] ) ) break;

    if ( state < 0 )
    {
      pexit = room->exit[room->state];
      to_room = pexit->u1.to_room;
      if ( ( pexit_rev = to_room->exit[rev_dir[door]] ) == NULL ) return;
      if ( pexit_rev->u1.to_room == room )
      {
        free_exit( to_room->exit[rev_dir[door]] );
        to_room->exit[rev_dir[door]] = room->exit[4];
        room->exit[4] = NULL;
        if ( room->exit[5] )
        {
          to_room->exit[rev_dir[door]]->u1.to_room->exit[door] = room->exit[5];
          room->exit[5] = NULL;
        }
      }
      room->state = state;
    }
  }
  else
  {
    if ( state >= 0 )
    {
      if ( room->exit[state] == NULL )
      {
        bug( "set_state_room: Bad exit, state %d.\n\r", state );
        return;
      }
      for ( door = 0; door <= 5; door++ )
        if ( !strcmp( room->exit[state]->keyword, dir_name[door] ) ) break;
      if ( door == 6 )
      {
        bug("set_state_room: Bad exit, state %d.", state );
        return;
      }
      pexit = room->exit[state];
      to_room = pexit->u1.to_room;
      room->exit[4] = to_room->exit[rev_dir[door]];
      pexit_rev = new_exit();
      pexit_rev->u1.to_room = room;
      to_room->exit[rev_dir[door]] = pexit_rev;
      if ( room->exit[4]
           &&   room->exit[4]->u1.to_room->exit[door]->u1.to_room == to_room )
      {
        room->exit[5] = room->exit[4]->u1.to_room->exit[door];
        room->exit[4]->u1.to_room->exit[door] = NULL;
      }
      room->state = state;
    }
  }
}

int move_char( CHAR_DATA *ch, int door, bool follow )
{
  CHAR_DATA *fch;
  CHAR_DATA *fch_next;
  CHAR_DATA *vch;
  char buf[MSL];
  ROOM_INDEX_DATA *in_room;
  ROOM_INDEX_DATA *to_room = NULL;
  EXIT_DATA *pexit;

  if ( ch->pcdata )
  {

    if ( can_carry_w( ch ) < get_carry_weight( ch ) - 1 )
    {
      send_to_char("You are carrying more than you can take with you.\n\r",ch);
      return FALSE;
    }

    if ( ch->pcdata->condition[COND_DRUNK] >= 5 /* Is char. drunk? */
         &&   number_percent() < 50 ) /* 50-50 chance */
    {
      act( "$n stumbles around drunk.", ch, NULL, NULL, TO_ROOM );
      act( "You stumble around drunk.", ch, NULL, NULL, TO_CHAR );
      /* End of "Add this for ROT only" */
      door = number_range( 0, 5 ); /* Find new direction to
             leave in */
    }

// hallucinating (poison)
    if ( IS_AFFECTED(ch, AFF_POISON)
         &&   number_percent() < 33 ) /* 33% chance */
    {
      AFFECT_DATA *poison;
      if ( ( ( poison = affect_find(ch->affected,gsn_poison) ) != NULL )
           &&     ( poison->modifier == TOX_HALLUCINOGENIC) )
        door = number_range( 0, 5 );
    }

#if foo
    if ( IS_BLIND( ch ) && ( number_percent() < 50 ) )
    {
      if ( !IS_AFFECTED( ch, AFF_CHARM ) && !IS_IMMORTAL( ch ) )
      {
        act( "$n stumbles around blind.", ch, NULL, NULL, TO_ROOM );
        act( "You stumble around blind.", ch, NULL, NULL, TO_CHAR );
        /* End of "Add this for ROT only" */
        door = number_range( 0, 5 ); /* Find new direction to
               leave in */
      }
    }
#endif
  }
  if ( door < 0 || door > 5 )
  {
    bug( "Do_move: bad door %d.", door );
    return FALSE;
  }

  /*
   * Exit trigger, if activated, bail out. Only PCs are triggered.
   */
  if ( !IS_NPC( ch ) && mp_exit_trigger( ch, door ) )
    return FALSE;

  in_room = ch->in_room;

  if ( IS_SET( in_room->room_flags, ROOM_SHIP ) )
  {
    if ( in_room->state < 0 || in_room->state > 5 ) pexit = NULL;
    else
    {
      if ( ( pexit = in_room->exit[in_room->state] ) )
        if ( strcmp( dir_name[door], pexit->keyword ) ) pexit = NULL;
    }
  }
  else pexit = in_room->exit[door];

  if ( pexit == NULL
       ||   IS_SET( pexit->exit_info, EX_NOEXIT )
       || ( to_room = pexit->u1.to_room ) == NULL
       ||  !can_see_room( ch, pexit->u1.to_room ) )
  {
    send_to_char( "Alas, you cannot go that way.\n\r", ch );
    return FALSE;
  }

  if ( IS_SET( pexit->exit_info, EX_CLOSED )
       && ( !IS_AFFECTED( ch, AFF_PASS_DOOR )
            || ( IS_SET( pexit->exit_info, EX_NOPASS )
                 &&  !IS_GHOST( ch ) ) )
       &&  !IS_TRUSTED( ch, ANGEL ) )
  {
    if (IS_AFFECTED(ch, AFF_BLIND))
    {
      strcpy( buf, "The exit" );
    }
    else
    {
      strcpy( buf, "The " );
      if ( pexit->keyword
           &&   pexit->keyword[0] != '\0'
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

    printf_to_char( ch, "%s closed.\n\r", buf );
    return FALSE;
  }

  if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master
       &&   in_room == ch->master->in_room
       &&   ch->fighting == NULL )

  {
    send_to_char( "What?  And leave your beloved master?\n\r", ch );
    return FALSE;
  }

  /* We'll let order <pet> flee
   * If this becomes an issue with them fleeing on their own we'll find a new way.
   */
  /*if ( IS_AFFECTED( ch, AFF_CHARM )
  &&   ch->master != NULL )
    if ( ch->master->fighting != NULL )
    {
      send_to_char(
            "What? Your beloved master is fighting, you just cannot bear to flee and leave him.\n\r",
                ch);
      return FALSE;
    }*/

  if ( ( !is_room_owner( ch, to_room ) && room_is_private( to_room ) )
       && ( get_trust( ch ) < IMPLEMENTOR ) )
  {
    send_to_char( "That room is private right now.\n\r", ch );
    return FALSE;
  }

  /* Activate Drowning! */
  /*
    if (( IS_SET(in_room->room_flags,ROOM_UNDER_WATER)
    ||    IS_SET(to_room->room_flags,ROOM_UNDER_WATER ))
    &&   (!IS_AFFECTED(ch,AFF_SWIM) && !IS_IMMORTAL(ch) ))
    {
    send_to_char("You can't swim!!\n\r", ch);
    return FALSE;
    }*/
  if ( IS_SET( to_room->room_flags, ROOM_NO_GHOST ) )
  {
    if ( IS_GHOST( ch ) )
    {
      send_to_char( "Ghosts are not allowed in there.\n\r", ch );
      return FALSE;
    }
  }

  if ( !IS_NPC( ch ) )
  {
    int iClass, iGuild;
    int move;

    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
    {
      for ( iGuild = 0; iGuild < MAX_GUILD; iGuild ++)
      {
        if ( iClass != ch->gameclass
             &&   to_room->vnum == class_table[iClass].guild[iGuild]
             &&  !IS_IMMORTAL( ch ) )
        {
          send_to_char( "You aren't allowed in there.\n\r", ch );
          return FALSE;
        }
      }
    }
    if ( in_room->sector_type == SECT_AIR
         ||   to_room->sector_type == SECT_AIR )
    {
      if ( !IS_AFFECTED( ch, AFF_FLYING ) && !IS_IMMORTAL( ch ) )
      {
        send_to_char( "You can't fly.\n\r", ch );
        return FALSE;
      }
    }

    if ( ( in_room->sector_type == SECT_WATER_NOSWIM
           ||     to_room->sector_type == SECT_WATER_NOSWIM )
         &&    !IS_AFFECTED( ch, AFF_FLYING )
         &&  !IS_SET( ch->spell_aff, SAFF_WALK_ON_WATER ) )
    {
      OBJ_DATA *obj;
      bool found;

      /*
       * Look for a boat.
       */
      found = IS_IMMORTAL( ch );

      if ( !found )
      {
        for ( obj = ch->carrying; obj; obj = obj->next_content )
        {
          if ( obj->item_type == ITEM_BOAT )
          {
            found = TRUE;
            break;
          }
        }
      }
      if ( !found )
      {
        send_to_char( "You need a boat to go there.\n\r", ch );
        return FALSE;
      }
    }

    move = movement_loss[UMIN( SECT_MAX-1, in_room->sector_type )]
           + movement_loss[UMIN( SECT_MAX-1, to_room->sector_type )];
    move /= 2;    /* i.e. the average */

    /* conditional effects */
    if ( IS_AFFECTED( ch, AFF_FLYING ) || IS_AFFECTED( ch, AFF_HASTE ) )
      move /= 2;

    if ( IS_AFFECTED( ch, AFF_SLOW ) )
      move *= 2;

    if ( !IS_GHOST( ch ) )
      if ( ch->move < move )
      {
        send_to_char( "You are too exhausted.\n\r", ch );
        return FALSE;
      }

    WAIT_STATE( ch, 1 );
    if ( !IS_GHOST( ch ) )
      ch->move -= move;
  }

  if ( ( ch->level < MAX_LEVEL - 9 ) && ( ch->incog_level ) )
  {
    send_to_char( "{RRemoving {WCloaking{G: Leaving Area.{x\n\r", ch );
    ch->incog_level = 0;
  }

  /*
    //So...if not sneaking and invis level < hero...
    if ( !IS_AFFECTED( ch, AFF_SNEAK )
    &&   ch->invis_level < LEVEL_HERO)
      act( "$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM );
    else
    {
       for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
          //Or...not sneaking, can see (with imm checks)
        if ( !IS_AFFECTED( ch, AFF_SNEAK )
          && (  can_see(vch,ch)
          || (  IS_IMMORTAL( vch ) && IS_SET( vch->act, PLR_HOLYLIGHT )
          &&    get_trust( vch ) >= ch->invis_level  ) ) )
          {
            mprintf(sizeof(buf), buf, "$n leaves %s.", dir_name[door] );
            act( buf, ch, NULL, vch, TO_VICT);
          }
    }
  */

  //Instead, first we check for sneaking.
  //If you are hiding or an immortal, you see them sneak off.

  for ( vch = ch->in_room->people ; vch ; vch = vch->next_in_room )
  {
    if ( IS_AFFECTED( ch, AFF_SNEAK ) )
    {
      if (  IS_AFFECTED( vch, AFF_HIDE )
            || (  IS_IMMORTAL( vch ) && IS_SET( vch->act, PLR_HOLYLIGHT )
                  &&    get_trust( vch ) >= ch->invis_level ) )
      {
        mprintf( sizeof( buf ), buf, "$n sneaks %s.", dir_name[door] );
        act( buf, ch, NULL, vch, TO_VICT );
      }
    }
    else //I don't see any need for more than can_see here, since it checks imms.
    {
      if ( can_see( vch, ch ) )
      {
        mprintf( sizeof( buf ), buf, "$n leaves %s.", dir_name[ door ] );
        act( buf, ch, NULL, vch, TO_VICT );
      }
    }
  }

  move_to_room( ch, to_room );

  for ( vch = ch->in_room->people ; vch ; vch = vch->next_in_room )
  {
    if ( IS_AFFECTED( ch, AFF_SNEAK ) )
    {

      if ( IS_AFFECTED( vch, AFF_HIDE )
           || ( IS_IMMORTAL( vch ) && IS_SET( vch->act, PLR_HOLYLIGHT )
                &&   get_trust( vch ) >= ch->invis_level ) )
      {
        mprintf( sizeof( buf ), buf,
                 "$n sneaks in from %s.", dir_from_name[ door ] );
        act( buf, ch, NULL, vch, TO_VICT );
      }
    }
    else //I don't see any need for more than can_see here, since it checks imms.
    {
      if ( can_see( vch, ch ) )
      {
        mprintf( sizeof( buf ), buf,
                 "$n has arrived from %s.", dir_from_name[ door ] );
        act( buf, ch, NULL, vch, TO_VICT );
      }
    }
  }

  // do_look this had "auto" removed... not sure why
  do_function( ch, &do_look, "auto" );

  if ( in_room == to_room )  /* no circular follows */
    return TRUE;

  for ( fch = in_room->people; fch; fch = fch_next )
  {
    fch_next = fch->next_in_room;

    if ( fch->master == ch && IS_AFFECTED( fch, AFF_CHARM )
         &&   fch->position < POS_STANDING && !IS_SET( fch->act, ACT_PET ) )
      do_function( fch, &do_stand, "" );

    if ( fch->master == ch && fch->position == POS_STANDING
         &&   can_see_room( fch, to_room ) )
    {

      if ( IS_SET( ch->in_room->room_flags, ROOM_LAW )
           && ( IS_NPC( fch ) && IS_SET( fch->act, ACT_AGGRESSIVE ) ) )
      {
        act( "You can't bring $N into the city.", ch, NULL, fch, TO_CHAR );
        act( "You aren't allowed in the city.", fch, NULL, NULL, TO_CHAR );
        continue;
      }
      if ( IS_AFK( fch ) )
      {
        act( "$N is AFK and cannot move.", ch, NULL, fch, TO_CHAR );
        continue;
      }
      if ( IS_LINKDEAD( fch ) )
      {
        act("$N is Linkdead and cannot move.", ch, NULL, fch, TO_CHAR );
        continue;
      }
      if ( fch->in_room )
      {

        if ( IS_NPC( fch )
             && ( get_carry_weight( fch ) > can_carry_w( fch ) ) )
        {
          // pets don't have a max carry weight, so limit to 20*level + 500
          act( "$N is too weighed down to follow.", ch, NULL, fch, TO_CHAR );
          act( "You are carrying too much to follow $N.", fch, NULL, ch, TO_CHAR );
        }
        else
        {
          act( "You follow $N.", fch, NULL, ch, TO_CHAR );
          move_char( fch, door, TRUE );
        }

      }
      else
        bugf( "Move_char: Character %s has no room.\n\r", fch->name );
    }
  }

  /*
   * If someone is following the char, these triggers get activated
   * for the followers before the char, but it's safer this way...
   */
  if ( IS_NPC( ch ) )
  {
    if ( HAS_TRIGGER( ch, TRIG_ENTRY )
         &&   mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_ENTRY ) )
      return TRUE;
  }
  else
    mp_greet_trigger( ch, dir_name[rev_dir[door]] );

  if ( !follow ) // Only activated for leader
    ap_gronce_trigger( ch );

  if ( IS_SET( ch->act, PLR_AUTOHUNT )
       &&   !IS_NULLSTR( ch->tracking ) )
  {
    real_track( ch, ch->tracking );
  }

  /* Taeloch: aggro out-of-level derision */
  int  actpicker = 0;
  bool threatened = FALSE;
  char charbuf[MAX_INPUT_LENGTH];
  char roombuf[MAX_STRING_LENGTH];

  for ( fch = ch->in_room->people; (fch && !threatened); fch = fch_next )
  {
    fch_next = fch->next_in_room;

    if ( IS_NPC(fch)
         &&   IS_SET(fch->act, ACT_AGGRESSIVE)
         &&   ( ch->level < 80 && ( fch->level > ( ch->level + MAX_AGGRO_LEVELS ) ) ) // the key purpose
         &&   fch->fighting == NULL
         &&   can_see( fch, ch )
         &&  !IS_AFFECTED(fch, AFF_CALM)
         &&  !IS_AFFECTED(fch, AFF_BLIND)
         &&  !IS_AFFECTED(fch, AFF_CHARM)
         &&  !IS_SET(ch->spell_aff, SAFF_DETER)
         &&  !IS_LINKDEAD(ch)
         &&  !IS_AFK(ch)
         &&   IS_AWAKE(fch) )
    {
      threatened = TRUE;
      actpicker = number_range (1,5);
      switch (actpicker)
      {
        case 1:
          strcpy(charbuf,"$N prepares to attack");
          strcpy(roombuf,"$N prepares to attack $n");
          break;
        case 2:
          strcpy(charbuf,"$N looks you over");
          strcpy(roombuf,"$N looks at $n");
          break;
        case 3:
          strcpy(charbuf,"$N is startled at your presence");
          strcpy(roombuf,"$N is startled by $n");
          break;
        case 4:
          strcpy(charbuf,"$N jumps up and glares at you");
          strcpy(roombuf,"$N jumps up and glares at $n");
          break;
        default:
          strcpy(charbuf,"$N quickly glances at you");
          strcpy(roombuf,"$N quickly glances at $n");
          break;
      }

      actpicker = number_range (1,4);
      switch (actpicker)
      {
        case 1:
          strcat(charbuf,", then laughs loudly.");
          strcat(roombuf,", then laughs at $m.");
          break;
        case 2:
          strcat(charbuf," and decides you aren't a threat.");
          strcat(roombuf," and decides $e is not a threat.");
          break;
        case 3:
          strcat(charbuf,", then scoffs at your weakness.");
          strcat(roombuf,", then scoffs at $s weakness.");
          break;
        default:
          strcat(charbuf,", then laughs at your weakness.");
          strcat(roombuf,", then laughs at $s weakness.");
          break;
      }

      act(charbuf, ch, NULL, fch, TO_CHAR );
      act(roombuf, ch, NULL, fch, TO_ROOM );
    }
  }
  /* end aggro dealio */
  return TRUE;
}



void do_north( CHAR_DATA *ch, char *argument )
{
  move_char( ch, DIR_NORTH, FALSE );
  return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
  move_char( ch, DIR_EAST, FALSE );
  return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
  move_char( ch, DIR_SOUTH, FALSE );
  return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
  move_char( ch, DIR_WEST, FALSE );
  return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
  move_char( ch, DIR_UP, FALSE );
  return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
  move_char( ch, DIR_DOWN, FALSE );
  return;
}



int find_door( CHAR_DATA *ch, char *arg )
{
  EXIT_DATA *pexit;
  int door;

  if ( !str_prefix( arg, "north" ) )
  {
    return DIR_NORTH;
  }
  else if ( !str_prefix( arg, "east"  ) )
  {
    return DIR_EAST;
  }
  else if ( !str_prefix( arg, "south" ) )
  {
    return DIR_SOUTH;
  }
  else if ( !str_prefix( arg, "west"  ) )
  {
    return DIR_WEST;
  }
  else if ( !str_prefix( arg, "up"    ) )
  {
    return DIR_UP;
  }
  else if ( !str_prefix( arg, "down"  ) )
  {
    return DIR_DOWN;
  }

  for ( door = 0; door <= 5; door++ )
  {
    if ( ( pexit = ch->in_room->exit[door] )
         &&   IS_SET( pexit->exit_info, EX_ISDOOR )
         &&   pexit->keyword
         &&   is_name( arg, pexit->keyword ) )
      return door;

    if ( !str_prefix( arg, dir_name[door] ) )
      break;
  }

  if ( door == 6 ) return -1;

  if ( ( pexit = ch->in_room->exit[door] ) == NULL ) return ( -2-door );

  if ( !IS_SET( pexit->exit_info, EX_ISDOOR ) ) return ( -2-door );

  return door;
}



void do_open( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_INPUT_LENGTH];

  OBJ_DATA *obj;
  int door;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Open what?\n\r", ch );
    return;
  }

  if ( ( door = find_door( ch, arg ) ) >= 0 )
  {
    /* 'open door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev=NULL;

    pexit = ch->in_room->exit[door];

    if ( !pexit
         ||   !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
      send_to_char( "There is nothing to open in that direction.\n\r", ch );
      return;
    }

    if ( IS_SET( pexit->exit_info, EX_MULTI ) )
      strcpy( buf, "They are" );
    else
      strcpy( buf, "It is" );

    if ( !IS_SET( pexit->exit_info, EX_CLOSED ) )
    {
      strcat( buf, " already open.\n\r" );
      send_to_char( buf, ch );
      return;
    }

    if (  IS_SET(pexit->exit_info, EX_LOCKED) )
    {
      strcat( buf, " locked.\n\r" );
      send_to_char( buf, ch );
      return;
    }

    /* now we open the door */
    REMOVE_BIT(pexit->exit_info, EX_CLOSED);

    strcpy( buf, "the " );
    if ( pexit->keyword
         && pexit->keyword[0] != '\0'
         && pexit->keyword[0] != ' ' )
      strcat( buf, pexit->keyword );
    else
      if ( IS_SET( pexit->exit_info, EX_MULTI ) )
        strcat( buf, "doors" );
      else
        strcat( buf, "door" );

    strcat( buf, "." );

    strcat( strcpy( buf1, "$n opens " ), buf );
    act( buf1, ch, NULL, NULL, TO_ROOM );
    strcat( strcpy( buf1, "You open " ), buf );
    act( buf1, ch, NULL, NULL, TO_CHAR);

    /* open the other side */
    if ( ( to_room = pexit->u1.to_room )
         && ( pexit_rev = to_room->exit[rev_dir[door]] )
         &&   pexit_rev->u1.to_room == ch->in_room )
    {
      REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );

      if ( to_room->people )
      {
        strcpy( buf, "The " );
        if ( pexit_rev->keyword
             && pexit_rev->keyword[0]
             && pexit->keyword[0] != ' ' )
          strcat( buf, pexit_rev->keyword );
        else
          if ( IS_SET( pexit_rev->exit_info, EX_MULTI ) )
            strcat( buf, "doors" );
          else
            strcat( buf, "door" );

        if ( IS_SET( pexit_rev->exit_info, EX_MULTI ) )
          strcat( buf, " open." );
        else
          strcat( buf, " opens." );
        act( buf, to_room->people, NULL, NULL, TO_ROOM_ALL );
      }
    }
    return;
  }

  if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
  {
    /* open portal */
    if ( obj->item_type == ITEM_PORTAL )
    {
      if ( !IS_SET(obj->value[1], EX_ISDOOR ) )
      {
        send_to_char("You can't do that.\n\r",ch);
        return;
      }

      if (!IS_SET(obj->value[1], EX_CLOSED))
      {
        send_to_char("It's already open.\n\r",ch);
        return;
      }

      if (IS_SET(obj->value[1], EX_LOCKED))
      {
        send_to_char("It's locked.\n\r",ch);
        return;
      }

      REMOVE_BIT(obj->value[1], EX_CLOSED);
      act( "You open $p. ", ch, obj, NULL, TO_CHAR );
      act( "$n opens $p. ", ch, obj, NULL, TO_ROOM );
      return;
    }

    /* 'open object' */
    if ( obj->item_type != ITEM_CONTAINER )
    {
      send_to_char( "That's not a container.\n\r", ch );
      return;
    }
    if ( !IS_SET(obj->value[1], CONT_CLOSED) )
    {
      send_to_char( "It's already open.\n\r",      ch );
      return;
    }
    if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
    {
      send_to_char( "You can't do that.\n\r",      ch );
      return;
    }
    if ( IS_SET(obj->value[1], CONT_LOCKED) )
    {
      send_to_char( "It's locked.\n\r",            ch );
      return;
    }

    REMOVE_BIT(obj->value[1], CONT_CLOSED);
    act( "You open $p.", ch, obj, NULL, TO_CHAR);
    act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
    return;
  }

  /*if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
     */ /* 'open door' *//*
ROOM_INDEX_DATA *to_room;
EXIT_DATA *pexit;
EXIT_DATA *pexit_rev=NULL;

pexit = ch->in_room->exit[door];
if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
{ send_to_char( "It's already open.\n\r",      ch ); return; }
if (  IS_SET(pexit->exit_info, EX_LOCKED) )
{ send_to_char( "It's locked.\n\r",            ch ); return; }

REMOVE_BIT(pexit->exit_info, EX_CLOSED);
act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
act( "You open the $d.", ch, NULL, pexit->keyword, TO_CHAR);

*//* open the other side *//*
if ( ( to_room   = pexit->u1.to_room            ) != NULL
&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
&&   pexit_rev->u1.to_room == ch->in_room )
{
CHAR_DATA *rch;

REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
}
}*/
  if ( door < -1 )
    act( "You see no door $T here.", ch, NULL, dir_name[-door-2], TO_CHAR );
  else
    act("You see no $T here.", ch, NULL, arg, TO_CHAR);
  return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  int door;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Close what?\n\r", ch );
    return;
  }

  if ( ( door = find_door( ch, arg ) ) >= 0 )
  {
    /* 'close door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev = NULL;

    pexit = ch->in_room->exit[door];

    if ( !pexit
         ||   !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
      send_to_char( "There is nothing to close in that direction.\n\r", ch );
      return;
    }

    if ( IS_SET( pexit->exit_info, EX_MULTI ) )
      strcpy( buf, "They are" );
    else
      strcpy( buf, "It is" );

    if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
    {
      strcat( buf, " already closed.\n\r" );
      send_to_char( buf, ch );
      return;
    }

    SET_BIT( pexit->exit_info, EX_CLOSED );

    if ( pexit->keyword
         && pexit->keyword[0]
         && pexit->keyword[0] != ' ' )
      strcpy( buf, pexit->keyword );
    else
    {
      strcpy( buf, "door" );
      if ( IS_SET( pexit->exit_info, EX_MULTI ) )
        strcat( buf, "s" );
    }

    strcat( buf, "." );

    strcat( strcpy( buf1, "$n closes the " ), buf );
    act( buf1, ch, NULL, NULL, TO_ROOM );
    strcat( strcpy( buf1, "You close the " ), buf );
    act( buf1, ch, NULL, NULL, TO_CHAR);

    /* close the other side */
    if ( ( to_room   = pexit->u1.to_room            )
         &&   ( pexit_rev = to_room->exit[rev_dir[door]] )
         &&   pexit_rev->u1.to_room == ch->in_room )
    {
      SET_BIT( pexit_rev->exit_info, EX_CLOSED );

      if ( to_room->people )
      {
        strcpy( buf, "The " );
        if ( pexit_rev->keyword
             &&   pexit_rev->keyword[0] != '\0'
             &&   pexit_rev->keyword[0] != ' ' )
          strcat( buf, pexit_rev->keyword );
        else
          if ( IS_SET( pexit_rev->exit_info, EX_MULTI ) )
            strcat( buf, "doors" );
          else
            strcat( buf, "door" );

        if ( IS_SET( pexit_rev->exit_info, EX_MULTI ) )
          strcat( buf, " close." );
        else
          strcat( buf, " closes." );
        act( buf, to_room->people, NULL, NULL, TO_ROOM_ALL );
      }
    }
    return;
  }

  if ( ( obj = get_obj_here( ch, arg ) ) )
  {
    /* portal stuff */
    if (obj->item_type == ITEM_PORTAL)
    {

      if (!IS_SET(obj->value[1],EX_ISDOOR)
          ||   IS_SET(obj->value[1],EX_NOCLOSE))
      {
        send_to_char("You can't do that.\n\r",ch);
        return;
      }

      if (IS_SET(obj->value[1],EX_CLOSED))
      {
        send_to_char("It's already closed.\n\r",ch);
        return;
      }

      SET_BIT(obj->value[1],EX_CLOSED);
      act("You close $p.",ch,obj,NULL,TO_CHAR);
      act("$n closes $p.",ch,obj,NULL,TO_ROOM);
      return;
    }

    /* 'close object' */
    if ( obj->item_type != ITEM_CONTAINER )
    {
      send_to_char( "That's not a container.\n\r", ch );
      return;
    }
    if ( IS_SET(obj->value[1], CONT_CLOSED) )
    {
      send_to_char( "It's already closed.\n\r",    ch );
      return;
    }
    if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
    {
      send_to_char( "You can't do that.\n\r",      ch );
      return;
    }

    SET_BIT(obj->value[1], CONT_CLOSED);
    act("You close $p.",ch,obj,NULL,TO_CHAR);
    act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
    return;
  }
  if ( door < -1 )
    act( "You see no door $T here.", ch, NULL, dir_name[-door-2], TO_CHAR );
  else
    act("You see no $T here.", ch, NULL, arg, TO_CHAR);
  return;
}



bool has_key( CHAR_DATA *ch, int key )
{
  OBJ_DATA *obj;
  OBJ_DATA *keyring_key;

  for ( obj = ch->carrying; obj; obj = obj->next_content )
  {
    if ( obj->pIndexData->vnum == key )
      return TRUE;

    if ( obj->item_type == ITEM_KEYRING )
    {
      for ( keyring_key = obj->contains; keyring_key; keyring_key = keyring_key->next_content )
      {
        if ( keyring_key->pIndexData->vnum == key )
          return TRUE;
      } // keyring keys
    } // if keyring
  } // carrying

  return FALSE;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  int door;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Lock what?\n\r", ch );
    return;
  }

  if ( ( obj = get_obj_here( ch, arg ) ) )
  {
    /* portal stuff */
    if (obj->item_type == ITEM_PORTAL)
    {
      if (!IS_SET(obj->value[1],EX_ISDOOR)
          ||  IS_SET(obj->value[1],EX_NOCLOSE))
      {
        send_to_char("You can't do that.\n\r",ch);
        return;
      }
      if (!IS_SET(obj->value[1],EX_CLOSED))
      {
        send_to_char("It's not closed.\n\r",ch);
        return;
      }

      if (obj->value[4] < 0 || IS_SET(obj->value[1],EX_NOLOCK))
      {
        send_to_char("It can't be locked.\n\r",ch);
        return;
      }

      if (!has_key(ch,obj->value[4]))
      {
        send_to_char("You lack the key.\n\r",ch);
        return;
      }

      if (IS_SET(obj->value[1],EX_LOCKED))
      {
        send_to_char("It's already locked.\n\r",ch);
        return;
      }

      SET_BIT(obj->value[1],EX_LOCKED);
      act("You lock $p.",ch,obj,NULL,TO_CHAR);
      act("$n locks $p.",ch,obj,NULL,TO_ROOM);
      return;
    }

    /* 'lock object' */
    if ( obj->item_type != ITEM_CONTAINER )
    {
      send_to_char( "That's not a container.\n\r", ch );
      return;
    }
    if ( !IS_SET(obj->value[1], CONT_CLOSED) )
    {
      send_to_char( "It's not closed.\n\r",        ch );
      return;
    }
    if ( obj->value[2] < 0 )
    {
      send_to_char( "It can't be locked.\n\r",     ch );
      return;
    }
    if ( !has_key( ch, obj->value[2] ) )
    {
      send_to_char( "You lack the key.\n\r",       ch );
      return;
    }
    if ( IS_SET(obj->value[1], CONT_LOCKED) )
    {
      send_to_char( "It's already locked.\n\r",    ch );
      return;
    }

    SET_BIT(obj->value[1], CONT_LOCKED);
    act("You lock $p.",ch,obj,NULL,TO_CHAR);
    act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
    return;
  }

  if ( ( door = find_door( ch, arg ) ) >= 0 )
  {
    /* 'lock door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev = NULL;

    pexit  = ch->in_room->exit[door];

    if ( !pexit
         ||   !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
      send_to_char( "There is nothing to lock in that direction.\n\r", ch );
      return;
    }

    if ( IS_SET( pexit->exit_info, EX_MULTI ) )
    {
      strcpy( buf,  "They " );
      strcpy( buf1, "are" ); // We don't need are/is if it can't be locked..
    }
    else
    {
      strcpy( buf,  "It " );
      strcpy( buf1, "is" );
    }

    if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
    {
      strcat( strcat( buf, buf1 ), " not closed.\n\r" );
      send_to_char( buf, ch );
      return;
    }
    if ( pexit->key < 0 )
    {
      strcat( buf, "can't be locked.\n\r" );
      send_to_char( buf, ch );
      return;
    }
    if ( !has_key( ch, pexit->key ) )
    {
      send_to_char( "You lack the key.\n\r",ch );
      return;
    }
    if ( IS_SET(pexit->exit_info, EX_LOCKED) )
    {
      strcat( strcat( buf, buf1 ), " already locked.\n\r" );
      send_to_char( buf, ch );
      return;
    }
    /* Now lock the door */
    SET_BIT(pexit->exit_info, EX_LOCKED);

    if ( pexit->keyword
         && pexit->keyword[0]
         && pexit->keyword[0] != ' ' )
      strcpy( buf, pexit->keyword );
    else
      if ( IS_SET( pexit->exit_info, EX_MULTI ) )
        strcpy( buf, "doors" );
      else
        strcpy( buf, "door" );

    strcat( buf, "." );
    strcat( strcpy( buf1, "$n locks " ), buf );
    act( buf1, ch, NULL, NULL, TO_ROOM );
    send_to_char( "*Click*\n\r", ch );

    /* lock the other side */
    if ( ( to_room   = pexit->u1.to_room            )
         &&   ( pexit_rev = to_room->exit[rev_dir[door]] )
         &&   pexit_rev->u1.to_room == ch->in_room )
      SET_BIT( pexit_rev->exit_info, EX_LOCKED );

    return;
  }
  if ( door < -1 )
    act( "You see no door $T here.", ch, NULL, dir_name[-door-2], TO_CHAR);
  else
    act( "You see no $T here.", ch, NULL, arg, TO_CHAR);
  return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MSL];
  char buf1[MSL];
  OBJ_DATA *obj;
  int door;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Unlock what?\n\r", ch );
    return;
  }

  if ( ( door = find_door( ch, arg ) ) >= 0 )
  {
    /* 'unlock door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev = NULL;

    pexit = ch->in_room->exit[door];

    if ( !pexit
         ||   !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
      send_to_char( "There is nothing to unlock in that direction.\n\r", ch );
      return;
    }

    if ( IS_SET( pexit->exit_info, EX_MULTI ) )
    {
      strcpy( buf,  "They " );
      strcpy( buf1, "are" ); // We don't need are/is if it can't be locked..
    }
    else
    {
      strcpy( buf,  "It " );
      strcpy( buf1, "is" );
    }


    if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
    {
      strcat( strcat( buf, buf1 ), " not closed.\n\r" );
      send_to_char( buf, ch );
      return;
    }
    if ( pexit->key < 0 )
    {
      strcat( buf, "can't be unlocked.\n\r" );
      send_to_char( buf, ch );
      return;
    }

    if ( !has_key( ch, pexit->key) )
    {
      send_to_char( "You lack the key.\n\r",ch );
      return;
    }
    if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
    {
      strcat( strcat ( buf, buf1 ), " already unlocked.\n\r" );
      send_to_char( buf, ch );
      return;
    }

    /* Now unlock the door */
    REMOVE_BIT(pexit->exit_info, EX_LOCKED);

    if ( pexit->keyword
         &&   pexit->keyword[0]
         &&   pexit->keyword[0] != ' ' )
      strcpy( buf, pexit->keyword );
    else
      if ( IS_SET( pexit->exit_info, EX_MULTI ) )
        strcpy( buf, "doors" );
      else
        strcpy( buf, "door" );

    strcat( buf, "." );
    strcat( strcpy( buf1, "$n unlocks the " ), buf );
    act( buf1, ch, NULL, NULL, TO_ROOM );
    send_to_char( "*Click*\n\r", ch );

    /* unlock the other side */
    if ( ( to_room = pexit->u1.to_room )
         &&   ( pexit_rev = to_room->exit[rev_dir[door]] )
         &&     pexit_rev->u1.to_room == ch->in_room )
      REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );

    return;
  }

  if ( ( obj = get_obj_here( ch, arg ) ) )
  {
    /* portal stuff */
    if (obj->item_type == ITEM_PORTAL)
    {
      if (!IS_SET(obj->value[1],EX_ISDOOR))
      {
        send_to_char("You can't do that.\n\r",ch);
        return;
      }

      if (!IS_SET(obj->value[1],EX_CLOSED))
      {
        send_to_char("It's not closed.\n\r",ch);
        return;
      }

      if (obj->value[4] < 0)
      {
        send_to_char("It can't be unlocked.\n\r",ch);
        return;
      }

      if (!IS_SET(obj->value[1],EX_LOCKED))
      {
        send_to_char("It's already unlocked.\n\r",ch);
        return;
      }

      if (!has_key(ch,obj->value[4]))
      {
        send_to_char("You lack the key.\n\r",ch);
        return;
      }

      REMOVE_BIT(obj->value[1],EX_LOCKED);
      act("You unlock $p.",ch,obj,NULL,TO_CHAR);
      act("$n unlocks $p.",ch,obj,NULL,TO_ROOM);
      return;
    }

    /* 'unlock object' */
    if ( obj->item_type != ITEM_CONTAINER )
    {
      send_to_char( "That's not a container.\n\r", ch );
      return;
    }
    if ( !IS_SET(obj->value[1], CONT_CLOSED) )
    {
      send_to_char( "It's not closed.\n\r",        ch );
      return;
    }
    if ( obj->value[2] < 0 )
    {
      send_to_char( "It can't be unlocked.\n\r",   ch );
      return;
    }
    if ( !has_key( ch, obj->value[2] ) )
    {
      send_to_char( "You lack the key.\n\r",       ch );
      return;
    }
    if ( !IS_SET(obj->value[1], CONT_LOCKED) )
    {
      send_to_char( "It's already unlocked.\n\r",  ch );
      return;
    }

    REMOVE_BIT(obj->value[1], CONT_LOCKED);
    act("You unlock $p.",ch,obj,NULL,TO_CHAR);
    act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
    return;
  }
  if ( door < -1 )
    act( "You see no door $T here.", ch, NULL, dir_name[-door-2], TO_CHAR );
  else
    act( "You see no $T here.", ch, NULL, arg, TO_CHAR );
  return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MSL];
  char buf1[MSL];
  CHAR_DATA *gch;
  OBJ_DATA *obj;
  int door;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Pick what?\n\r", ch );
    return;
  }

  WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

  /* look for guards */
  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
  {
    if ( IS_NPC( gch ) && IS_AWAKE( gch ) && ch->level + 5 < gch->level )
    {
      if ( (!IS_AFFECTED( gch, AFF_DETECT_HIDDEN ) ) && ( IS_AFFECTED( ch, AFF_HIDE ) ) )
      {
        act( "You sneak past $N.", ch, NULL, gch, TO_CHAR );
      }
      else
      {
        act( "$N is standing too close to the lock.", ch, NULL, gch, TO_CHAR );
        return;
      }
    }
  }

  if ( !IS_NPC( ch ) && number_percent( ) > get_skill( ch, gsn_pick_lock ) )
  {
    send_to_char( "You failed.\n\r", ch );
    check_improve( ch, gsn_pick_lock, FALSE, 2 );
    return;
  }

  if ( ( obj = get_obj_here( ch, arg ) ) )
  {
    /* portal stuff */
    if ( obj->item_type == ITEM_PORTAL )
    {
      if (!IS_SET(obj->value[1],EX_ISDOOR))
      {
        send_to_char("You can't do that.\n\r",ch);
        return;
      }

      if (!IS_SET(obj->value[1],EX_CLOSED))
      {
        send_to_char("It's not closed.\n\r",ch);
        return;
      }

      if (obj->value[4] < 0)
      {
        send_to_char("It can't be unlocked.\n\r",ch);
        return;
      }

      if (IS_SET(obj->value[1],EX_PICKPROOF))
      {
        send_to_char("You failed.\n\r",ch);
        return;
      }

      REMOVE_BIT(obj->value[1],EX_LOCKED);
      act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR);
      act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM);
      check_improve(ch,gsn_pick_lock,TRUE,2);
      return;
    }

    /* 'pick object' */
    if ( obj->item_type != ITEM_CONTAINER )
    {
      send_to_char( "That's not a container.\n\r", ch );
      return;
    }
    if ( !IS_SET(obj->value[1], CONT_CLOSED) )
    {
      send_to_char( "It's not closed.\n\r",        ch );
      return;
    }
    if ( obj->value[2] < 0 )
    {
      send_to_char( "It can't be unlocked.\n\r",   ch );
      return;
    }
    if ( !IS_SET(obj->value[1], CONT_LOCKED) )
    {
      send_to_char( "It's already unlocked.\n\r",  ch );
      return;
    }
    if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
    {
      send_to_char( "You failed.\n\r",             ch );
      return;
    }

    REMOVE_BIT(obj->value[1], CONT_LOCKED);
    act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR);
    act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM);
    check_improve(ch,gsn_pick_lock,TRUE,2);
    return;
  }

  if ( ( door = find_door( ch, arg ) ) >= 0 )
  { /* 'pick door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev = NULL;

    pexit = ch->in_room->exit[door];

    if ( !pexit
    ||   !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
      send_to_char( "There is no door in that direction.\n\r", ch );
      return;
    }

    if ( IS_SET( pexit->exit_info, EX_MULTI ) )
    {
      strcpy( buf,  "They " );
      strcpy( buf1, "are" ); // We don't need are/is if it can't be locked..
    }
    else
    {
      strcpy( buf,  "It " );
      strcpy( buf1, "is" );
    }


    if ( !IS_SET( pexit->exit_info, EX_CLOSED ) && !IS_IMMORTAL( ch ) )
    {
      strcat( strcat( buf, buf1 ), " not closed.\n\r" );
      send_to_char( buf, ch );
      return;
    }
    if ( pexit->key < 0 && !IS_IMMORTAL( ch ) )
    {
      strcat( buf, "can't be picked.\n\r" );
      send_to_char( buf, ch );
      return;
    }
    if ( !IS_SET( pexit->exit_info, EX_LOCKED ) )
    {
      strcat( strcat( buf, buf1 ), " already unlocked.\n\r" );
      send_to_char( buf, ch );
      return;
    }
    if ( IS_SET( pexit->exit_info, EX_PICKPROOF ) && !IS_IMMORTAL( ch ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }

    /* now pick the door */
    REMOVE_BIT(pexit->exit_info, EX_LOCKED);
    if ( pexit->keyword
         &&   pexit->keyword[0]
         &&   pexit->keyword[0] != ' ' )
      strcpy( buf, pexit->keyword );
    else
    {
      strcat( buf, "door" );
      if ( IS_SET( pexit->exit_info, EX_MULTI ) )
        strcat( buf, "s" );
    }
    strcat( buf, "." );

    strcat( strcpy( buf1, "$n picks the ") , buf );
    act( buf1, ch, NULL, pexit->keyword, TO_ROOM );

    send_to_char( "*Click*\n\r", ch );
    check_improve(ch,gsn_pick_lock,TRUE,2);

    /* pick the other side */
    if ( ( to_room = pexit->u1.to_room )
         &&   ( pexit_rev = to_room->exit[rev_dir[door]] )
         &&     pexit_rev->u1.to_room == ch->in_room )
      REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );

    return;
  }
  if ( door <-1 )
    act( "You see no door $T here.", ch, NULL, dir_name[-door-2], TO_CHAR );
  else
    act( "You see no $T here.", ch, NULL, arg, TO_CHAR );
  return;
}




void do_stand( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj = NULL;
  OBJ_DATA *onobj = NULL;
  bool onquestbedroll = FALSE;

  if ( argument[0] )
  {
    if (ch->position == POS_FIGHTING)
    {
      send_to_char("Maybe you should finish fighting first?\n\r",ch);
      return;
    }
    obj = get_obj_list(ch,argument,ch->in_room->contents);
    if (obj == NULL)
    {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }

    if (obj->item_type != ITEM_FURNITURE
        ||  (!IS_SET(obj->value[2],STAND_AT)
             &&   !IS_SET(obj->value[2],STAND_ON)
             &&   !IS_SET(obj->value[2],STAND_IN)))
    {
      send_to_char("You can't seem to find a place to stand.\n\r",ch);
      return;
    }

    if ( ( ( onobj = ch->on ) != NULL ) // check if they are on quest bedroll
         &&   ( ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_MANA )
                ||     ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_HP )
                ||     ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_BOTH ) ) )
      onquestbedroll = TRUE;

    if (ch->on != obj && count_users(obj) >= obj->value[0])
    {
      act_new("There's no room to stand on $p.",
              ch,obj,NULL,TO_CHAR,POS_DEAD);
      return;
    }
    ch->on = obj;
  }
  else
  {
    if ( ( ( onobj = ch->on ) != NULL ) // check if they are on quest bedroll
         &&   ( ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_MANA )
                ||     ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_HP )
                ||     ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_BOTH ) ) )
      onquestbedroll = TRUE;
  }

  switch ( ch->position )
  {
    case POS_SLEEPING:
      if ( IS_AFFECTED(ch, AFF_SLEEP) )
      {
        send_to_char( "You can't wake up!\n\r", ch );
        return;
      }

      if (obj == NULL)
      {
        send_to_char( "You wake and stand up.\n\r", ch );
        act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
        ch->on = NULL;
      }
      else if (IS_SET(obj->value[2],STAND_AT))
      {
        act_new("You wake and stand at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
        act("$n wakes and stands at $p.",ch,obj,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],STAND_ON))
      {
        act_new("You wake and stand on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
        act("$n wakes and stands on $p.",ch,obj,NULL,TO_ROOM);
      }
      else
      {
        act_new("You wake and stand in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
        act("$n wakes and stands in $p.",ch,obj,NULL,TO_ROOM);
      }

      ch->position = POS_STANDING;
      do_function(ch, &do_look,"auto");
      break;

    case POS_RESTING:
    case POS_SITTING:
      if (obj == NULL)
      {
        send_to_char( "You stand up.\n\r", ch );
        act( "$n stands up.", ch, NULL, NULL, TO_ROOM );
        ch->on = NULL;
      }
      else if (IS_SET(obj->value[2],STAND_AT))
      {
        act("You stand at $p.",ch,obj,NULL,TO_CHAR);
        act("$n stands at $p.",ch,obj,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],STAND_ON))
      {
        act("You stand on $p.",ch,obj,NULL,TO_CHAR);
        act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
      }
      else
      {
        act("You stand in $p.",ch,obj,NULL,TO_CHAR);
        act("$n stands in $p.",ch,obj,NULL,TO_ROOM);
      }
      ch->position = POS_STANDING;
      break;

    case POS_STANDING:
      if (obj == NULL)
      {
        send_to_char( "You are already standing.\n\r", ch );
        ch->on = NULL;
      }
      else if (IS_SET(obj->value[2],STAND_AT))
      {
        act("You stand at $p.",ch,obj,NULL,TO_CHAR);
        act("$n stands at $p.",ch,obj,NULL,TO_ROOM);
        ch->on = obj;
      }
      else if (IS_SET(obj->value[2],STAND_ON))
      {
        act("You stand on $p.",ch,obj,NULL,TO_CHAR);
        act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
        ch->on = obj;
      }
      else if (IS_SET(obj->value[2],STAND_IN))
      {
        act("You stand in $p.",ch,obj,NULL,TO_CHAR);
        act("$n stands in $p.",ch,obj,NULL,TO_ROOM);
        ch->on = obj;
      }
      else
        send_to_char( "You are already standing.\n\r", ch );

      break;

    case POS_FIGHTING:
      send_to_char( "You are already fighting!\n\r", ch );
      break;
  }

  if ( (ch->on != onobj) && (onquestbedroll) && !IS_NPC(ch) ) // check if they are on quest bedroll (but not pets!)
  {
    act("You pick up $p.",ch,onobj,NULL,TO_CHAR);
    act("$n picks up $p.",ch,onobj,NULL,TO_ROOM);
    obj_from_room( onobj );
    obj_to_char( onobj, ch );
  }
  return;
}



void do_rest( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj = NULL;

  if ( IS_GHOST( ch ) )
  {
    send_to_char( "Resting is useless as you are still {rDEAD{x.\n\r", ch );
    return;
  }

  if ( ch->position == POS_FIGHTING )
  {
    send_to_char( "You can't rest while fighting!\n\r", ch );
    return;
  }

  /* okay, now that we know we can rest, find an object to rest on */
  if ( argument[0] )
  {
    obj = get_obj_list( ch, argument, ch->in_room->contents );

    if ( obj == NULL )
    {
      send_to_char( "You don't see that here.\n\r", ch );
      return;
    }
  }
  else obj = ch->on;

  if ( obj )
  {
    if (  obj->item_type != ITEM_FURNITURE
          || ( !IS_SET( obj->value[2], REST_ON )
               &&   !IS_SET( obj->value[2], REST_IN )
               &&   !IS_SET( obj->value[2], REST_UNDER )
               &&   !IS_SET( obj->value[2], REST_AT ) ) )
    {
      send_to_char( "You can't rest on that.\n\r", ch );
      return;
    }

    if ( obj && ch->on != obj && count_users( obj ) >= obj->value[0] )
    {
      act_new( "There's no more room on $p.",
               ch, obj, NULL, TO_CHAR, POS_DEAD );
      return;
    }

    if ( ch->position == POS_RESTING
         &&   obj == ch->on )
    {
      send_to_char( "You are already resting on that.\n\r", ch );
      return;
    }
    else if ( ch->position == POS_RESTING
              &&        obj != ch->on )
    {
      printf_to_char( ch, "You sit up and move to rest on %s.\n\r",
                      obj->short_descr );
      ch->on = obj;
      return;
    }

    ch->on = obj;
  }

  switch ( ch->position )
  {
    case POS_SLEEPING:
      if (IS_AFFECTED(ch,AFF_SLEEP))
      {
        send_to_char("You can't wake up!\n\r",ch);
        return;
      }

      if (obj == NULL)
      {
        send_to_char( "You wake up and start resting.\n\r", ch );
        act ("$n wakes up and starts resting.",ch,NULL,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],REST_AT))
      {
        act_new("You wake up and rest at $p.",
                ch,obj,NULL,TO_CHAR,POS_SLEEPING);
        act("$n wakes up and rests at $p.",ch,obj,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],REST_ON))
      {
        act_new("You wake up and rest on $p.",
                ch,obj,NULL,TO_CHAR,POS_SLEEPING);
        act("$n wakes up and rests on $p.",ch,obj,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],REST_UNDER))
      {
        act_new("You wake up and rest under $p.",
                ch,obj,NULL,TO_CHAR,POS_SLEEPING);
        act("$n wakes up and rests under $p.",ch,obj,NULL,TO_ROOM);
      }
      else
      {
        act_new("You wake up and rest in $p.",
                ch,obj,NULL,TO_CHAR,POS_SLEEPING);
        act("$n wakes up and rests in $p.",ch,obj,NULL,TO_ROOM);
      }
      ch->position = POS_RESTING;
      break;

    case POS_RESTING:
      send_to_char( "You are already resting.\n\r", ch );
      break;

    case POS_STANDING:
      if (obj == NULL)
      {
        for ( obj = ch->carrying; obj; obj = obj->next_content )
        {
          if ( can_see_obj( ch, obj )
               &&  ( ( obj->pIndexData->vnum == OBJ_VNUM_BEDROLL_MANA )
                     || ( obj->pIndexData->vnum == OBJ_VNUM_BEDROLL_HP )
                     || ( obj->pIndexData->vnum == OBJ_VNUM_BEDROLL_BOTH ) ) )
          {
            act("You rest on $p.",ch,obj,NULL,TO_CHAR);
            act("$n rests on $p.",ch,obj,NULL,TO_ROOM);
            obj_from_char( obj );
            obj_to_room( obj, ch->in_room );
            ch->position = POS_RESTING;
            ch->on = obj;
            return;
          }
        }

        send_to_char( "You rest.\n\r", ch );
        act( "$n sits down and rests.", ch, NULL, NULL, TO_ROOM );
      }
      else if (IS_SET(obj->value[2],REST_AT))
      {
        act("You sit down at $p and rest.",ch,obj,NULL,TO_CHAR);
        act("$n sits down at $p and rests.",ch,obj,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],REST_ON))
      {
        act("You sit on $p and rest.",ch,obj,NULL,TO_CHAR);
        act("$n sits on $p and rests.",ch,obj,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],REST_UNDER))
      {
        act("You sit under $p and rest.",ch,obj,NULL,TO_CHAR);
        act("$n sits under $p and rests.",ch,obj,NULL,TO_ROOM);
      }
      else
      {
        act("You rest in $p.",ch,obj,NULL,TO_CHAR);
        act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
      }
      ch->position = POS_RESTING;
      break;

    case POS_SITTING:
      if (obj == NULL)
      {
        send_to_char("You rest.\n\r",ch);
        act("$n rests.",ch,NULL,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],REST_AT))
      {
        act("You rest at $p.",ch,obj,NULL,TO_CHAR);
        act("$n rests at $p.",ch,obj,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],REST_ON))
      {
        act("You rest on $p.",ch,obj,NULL,TO_CHAR);
        act("$n rests on $p.",ch,obj,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],REST_UNDER))
      {
        act("You rest under $p.",ch,obj,NULL,TO_CHAR);
        act("$n rests under $p.",ch,obj,NULL,TO_ROOM);
      }
      else
      {
        act("You rest in $p.",ch,obj,NULL,TO_CHAR);
        act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
      }
      ch->position = POS_RESTING;
      break;
  }


  return;
}


void do_sit (CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj = NULL;

  if (ch->position == POS_FIGHTING)
  {
    send_to_char("Maybe you should finish this fight first?\n\r",ch);
    return;
  }

  /* okay, now that we know we can sit, find an object to sit on */
  if ( argument[0] )
  {
    obj = get_obj_list(ch,argument,ch->in_room->contents);
    if (obj == NULL)
    {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
  }
  else obj = ch->on;

  if ( obj )
  {
    if (  obj->item_type != ITEM_FURNITURE
          || ( !IS_SET( obj->value[2], SIT_ON )
               &&   !IS_SET( obj->value[2], SIT_IN )
               &&   !IS_SET( obj->value[2], SIT_UNDER )
               &&   !IS_SET( obj->value[2], SIT_AT ) ) )
    {
      send_to_char("You can't sit on that.\n\r",ch);
      return;
    }

    if ( obj && ch->on != obj && count_users(obj) >= obj->value[0] )
    {
      act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
      return;
    }

    ch->on = obj;
  }
  switch (ch->position)
  {
    case POS_SLEEPING:
      if (IS_AFFECTED(ch,AFF_SLEEP))
      {
        send_to_char("You can't wake up!\n\r",ch);
        return;
      }

      if (obj == NULL)
      {
        send_to_char( "You wake and sit up.\n\r", ch );
        act( "$n wakes and sits up.", ch, NULL, NULL, TO_ROOM );
      }
      else if (IS_SET(obj->value[2],SIT_AT))
      {
        act_new("You wake and sit at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
        act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],SIT_ON))
      {
        act_new("You wake and sit on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
        act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],SIT_UNDER))
      {
        act_new("You wake and sit under $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
        act("$n wakes and sits under $p.",ch,obj,NULL,TO_ROOM);
      }
      else
      {
        act_new("You wake and sit in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
        act("$n wakes and sits in $p.",ch,obj,NULL,TO_ROOM);
      }

      ch->position = POS_SITTING;
      break;
    case POS_RESTING:
      if (obj == NULL)
        send_to_char("You stop resting.\n\r",ch);
      else if (IS_SET(obj->value[2],SIT_AT))
      {
        act("You sit at $p.",ch,obj,NULL,TO_CHAR);
        act("$n sits at $p.",ch,obj,NULL,TO_ROOM);
      }

      else if (IS_SET(obj->value[2],SIT_ON))
      {
        act("You sit on $p.",ch,obj,NULL,TO_CHAR);
        act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],SIT_UNDER))
      {
        act("You sit under $p.",ch,obj,NULL,TO_CHAR);
        act("$n sits under $p.",ch,obj,NULL,TO_ROOM);
      }
      ch->position = POS_SITTING;
      break;
    case POS_SITTING:
      send_to_char("You are already sitting down.\n\r",ch);
      break;
    case POS_STANDING:
      if (obj == NULL)
      {
        send_to_char("You sit down.\n\r",ch);
        act("$n sits down on the ground.",ch,NULL,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],SIT_AT))
      {
        act("You sit down at $p.",ch,obj,NULL,TO_CHAR);
        act("$n sits down at $p.",ch,obj,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],SIT_ON))
      {
        act("You sit on $p.",ch,obj,NULL,TO_CHAR);
        act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],SIT_UNDER))
      {
        act("You sit under $p.",ch,obj,NULL,TO_CHAR);
        act("$n sits under $p.",ch,obj,NULL,TO_ROOM);
      }
      else
      {
        act("You sit down in $p.",ch,obj,NULL,TO_CHAR);
        act("$n sits down in $p.",ch,obj,NULL,TO_ROOM);
      }
      ch->position = POS_SITTING;
      break;
  }
  return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj = NULL;

  if (IS_GHOST(ch))
  {
    send_to_char("Sleeping is useless as you are still DEAD.\n\r",ch);
    return;
  }

  if ( is_affected( ch, skill_lookup( "nightmare" ) ) )
  {
    send_to_char( "You are too frightened to sleep.\n\r", ch );
    return;
  }

  switch ( ch->position )
  {
    case POS_SLEEPING:
      send_to_char( "You are already sleeping.\n\r", ch );
      break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
      if (argument[0] == '\0' && ch->on == NULL)
      {
        for ( obj = ch->carrying; obj; obj = obj->next_content )
        {
          if ( can_see_obj( ch, obj )
               &&  ( ( obj->pIndexData->vnum == OBJ_VNUM_BEDROLL_MANA )
                     || ( obj->pIndexData->vnum == OBJ_VNUM_BEDROLL_HP )
                     || ( obj->pIndexData->vnum == OBJ_VNUM_BEDROLL_BOTH ) ) )
          {


            if (IS_SET(obj->value[2],SLEEP_AT))
            {
              act("You go to sleep at $p.",ch,obj,NULL,TO_CHAR);
              act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM);
            }
            else if (IS_SET(obj->value[2],SLEEP_ON))
            {
              act("You go to sleep on $p.",ch,obj,NULL,TO_CHAR);
              act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM);
            }
            else if (IS_SET(obj->value[2],SLEEP_UNDER))
            {
              act("You go to sleep under $p.",ch,obj,NULL,TO_CHAR);
              act("$n goes to sleep under $p.",ch,obj,NULL,TO_ROOM);
            }
            else
            {
              act("You go to sleep in $p.",ch,obj,NULL,TO_CHAR);
              act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM);
            }

            obj_from_char( obj );
            obj_to_room( obj, ch->in_room );
            ch->position = POS_SLEEPING;
            ch->on = obj;
            return;
          }
        }

        send_to_char( "You go to sleep.\n\r", ch );
        act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SLEEPING;
      }
      else  /* find an object and sleep on it */
      {
        if ( argument[0] == '\0' )
          obj = ch->on;
        else
          obj = get_obj_list( ch, argument,  ch->in_room->contents );

        if ( obj == NULL )
        {
          send_to_char("You don't see that here.\n\r",ch);
          return;
        }
        if (  obj->item_type != ITEM_FURNITURE
              || ( !IS_SET( obj->value[2], SLEEP_ON )
                   &&   !IS_SET( obj->value[2], SLEEP_IN )
                   &&   !IS_SET( obj->value[2], SLEEP_UNDER )
                   &&   !IS_SET( obj->value[2], SLEEP_AT ) ) )
        {
          send_to_char("You can't sleep on that!\n\r",ch);
          return;
        }

        if ( ch->on != obj && count_users(obj) >= obj->value[0] )
        {
          if (obj->pIndexData->vnum == OBJ_VNUM_SHELTER)
            act_new( "There is no room in $p for you.",
                     ch, obj, NULL, TO_CHAR, POS_DEAD );
          else
            act_new( "There is no room on $p for you.",
                     ch, obj, NULL, TO_CHAR, POS_DEAD );

          return;
        }

        ch->on = obj;
        if (IS_SET(obj->value[2],SLEEP_AT))
        {
          act("You go to sleep at $p.",ch,obj,NULL,TO_CHAR);
          act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],SLEEP_ON))
        {
          act("You go to sleep on $p.",ch,obj,NULL,TO_CHAR);
          act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],SLEEP_UNDER))
        {
          act("You go to sleep under $p.",ch,obj,NULL,TO_CHAR);
          act("$n goes to sleep under $p.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
          act("You go to sleep in $p.",ch,obj,NULL,TO_CHAR);
          act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM);
        }
        ch->position = POS_SLEEPING;
      }
      break;

    case POS_FIGHTING:
      send_to_char( "You are fighting!\n\r", ch );
      break;
  }

  return;
}


void do_wake( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {
    do_function(ch, &do_stand, argument );
    return;
  }

  if ( !IS_AWAKE(ch) )
  {
    send_to_char( "You are asleep yourself!\n\r",       ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r",              ch );
    return;
  }

  if ( IS_AWAKE(victim) )
  {
    act( "$N is already awake.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( IS_AFFECTED(victim, AFF_SLEEP) )
  {
    act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );
    return;
  }

  act_new( "$n wakes you.", ch, NULL, victim, TO_VICT,POS_SLEEPING );
  do_function(victim, &do_stand,"");
  return;
}



void do_sneak( CHAR_DATA *ch, char *argument )
{
  AFFECT_DATA af;

  if (IS_GHOST(ch))
  {
    send_to_char("Sneaking is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  send_to_char( "You attempt to move silently.\n\r", ch );
  affect_strip( ch, gsn_sneak );

  if (IS_AFFECTED(ch,AFF_SNEAK))
    return;

  if ( number_percent( ) < get_skill(ch,gsn_sneak))
  {
    check_improve(ch,gsn_sneak,TRUE,3);
    af.where     = TO_AFFECTS;
    af.type      = gsn_sneak;
    af.level     = ch->level;
    af.duration  = ch->level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SNEAK;
    affect_to_char( ch, &af );
    send_to_char("{DYou have become one with the shadows.{x\n\r",ch);
    WAIT_STATE(ch, 8);
  }
  else
    check_improve(ch,gsn_sneak,FALSE,3);

  return;
}



void do_hide( CHAR_DATA *ch, char *argument )
{
  send_to_char( "You attempt to hide.\n\r", ch );

  if (IS_GHOST(ch))
  {
    send_to_char("Hiding is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( IS_AFFECTED(ch, AFF_HIDE) )
    REMOVE_BIT(ch->affected_by, AFF_HIDE);

  if ( number_percent( ) < get_skill(ch,gsn_hide))
  {
    SET_BIT(ch->affected_by, AFF_HIDE);
    send_to_char("{DYou quickly become one with the shadows and hide.{x\n\r",ch);
    check_improve(ch,gsn_hide,TRUE,3);
    WAIT_STATE(ch,8);
    if (get_skill(ch, gsn_cloak_of_assassins) > 1)
      check_improve(ch,gsn_cloak_of_assassins,TRUE,3);
  }
  else
    check_improve(ch,gsn_hide,FALSE,3);

  return;
}

/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
  affect_strip ( ch, gsn_invis      );
  affect_strip ( ch, gsn_inverted_light  );
  affect_strip ( ch, gsn_mass_invis      );
  affect_strip ( ch, gsn_sneak      );
  REMOVE_BIT   ( ch->affected_by, AFF_HIDE    );
  REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE  );
  REMOVE_BIT   ( ch->affected_by, AFF_SNEAK    );
  send_to_char( "Ok.\n\r", ch );
  return;
}

void do_train( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *mob;
  sh_int stat = - 1;
  char *pOutput = NULL;
  int cost;

  if ( IS_NPC(ch) )
    return;

  if (IS_GHOST(ch))
  {
    send_to_char("Training is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }
  /*
   * Check for trainer.
   */
  for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
  {
    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
      break;
  }

  if ( mob == NULL )
  {
    send_to_char( "You can't do that here.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  {
    printf_to_char(ch, "You have %d training sessions.\n\r", ch->train );
    argument = "foo";
  }

  cost = 1;

  if ( !str_cmp( argument, "str" ) )
  {
    if ( class_table[ch->gameclass].attr_prime == STAT_STR )
      cost    = 1;
    stat        = STAT_STR;
    pOutput     = "strength";
  }

  else if ( !str_cmp( argument, "int" ) )
  {
    if ( class_table[ch->gameclass].attr_prime == STAT_INT )
      cost    = 1;
    stat      = STAT_INT;
    pOutput     = "intelligence";
  }

  else if ( !str_cmp( argument, "wis" ) )
  {
    if ( class_table[ch->gameclass].attr_prime == STAT_WIS )
      cost    = 1;
    stat      = STAT_WIS;
    pOutput     = "wisdom";
  }

  else if ( !str_cmp( argument, "dex" ) )
  {
    if ( class_table[ch->gameclass].attr_prime == STAT_DEX )
      cost    = 1;
    stat        = STAT_DEX;
    pOutput     = "dexterity";
  }

  else if ( !str_cmp( argument, "con" ) )
  {
    if ( class_table[ch->gameclass].attr_prime == STAT_CON )
      cost    = 1;
    stat      = STAT_CON;
    pOutput     = "constitution";
  }

  else if ( !str_cmp(argument, "hp" ) )
    cost = 1;

  else if ( !str_cmp(argument, "mana" ) )
    cost = 1;

  else if ( !str_cmp(argument, "move" ) )
    cost = 1;

  else
  {
    strcpy( buf, "You can train:" );
    if ( ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR))
      strcat( buf, " str" );
    if ( ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT))
      strcat( buf, " int" );
    if ( ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS))
      strcat( buf, " wis" );
    if ( ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX))
      strcat( buf, " dex" );
    if ( ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON))
      strcat( buf, " con" );
    if ( ch->pcdata->perm_hit < 40 * ch->level )
      strcat( buf, " hp");
    if ( ch->pcdata->perm_mana < 40 * ch->level )
      strcat( buf, " mana");
    if ( ch->pcdata->perm_move < 40 * ch->level )
      strcat( buf, " move");

    if (strlen(buf) < 16)
      strcat( buf, " nothing.  All of your stats are maxed.");

    if ( buf[strlen(buf)-1] != ':' )
    {
      strcat( buf, "\n\r" );
      send_to_char( buf, ch );
    }
    else
    {
      /*
       * This message dedicated to Jordan ... you big stud!
       */
      act( "You have nothing left to train, you $T!",
           ch, NULL,
           ch->sex == SEX_MALE   ? "big stud" :
           ch->sex == SEX_FEMALE ? "hot babe" :
           "wild thing",
           TO_CHAR );
    }

    return;
  }

  /* Values for maxing hp/mana/move per level */
  int level;
  char log_buf[MSL];
  level = ch->level;

  if (!str_cmp("hp",argument))
  {
    if ( cost > ch->train )
    {
      send_to_char( "You don't have enough training sessions.\n\r", ch );
      return;
    }

    if ( ch->pcdata->perm_hit >= 40 * level )
    {
      send_to_char( "You have already maxed out your hp for this level.\n\r", ch );
      return;
    }

    ch->train -= cost;
    ch->pcdata->perm_hit += 10;
    ch->max_hit += 10;
    ch->hit +=10;
    mprintf(sizeof(log_buf), log_buf, "{g%s{w just used {D%d{w train raising {rhp{x to {R%d{w, leaving {g%d{x trains.",
            ch->name, cost, ch->pcdata->perm_hit, ch->train);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_SECURE,0,get_trust(ch));
    act( "Your durability increases!",ch,NULL,NULL,TO_CHAR);
    act( "$n's durability increases!",ch,NULL,NULL,TO_ROOM);
    return;
  }

  if (!str_cmp("mana",argument))
  {
    if ( cost > ch->train )
    {
      send_to_char( "You don't have enough training sessions.\n\r", ch );
      return;
    }

    if ( ch->pcdata->perm_mana >= 40 * level )
    {
      send_to_char( "You have already maxed out your mana for this level.\n\r",
                    ch );
      return;
    }

    ch->train -= cost;
    ch->pcdata->perm_mana += 10;
    ch->max_mana += 10;
    ch->mana += 10;
    mprintf(sizeof(log_buf), log_buf, "{g%s{w just used {D%d{w train raising {gmana{x to {G%d{w, leaving {g%d{x trains.",
            ch->name, cost, ch->pcdata->perm_mana, ch->train);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_SECURE,0,get_trust(ch));
    act( "Your power increases!",ch,NULL,NULL,TO_CHAR);
    act( "$n's power increases!",ch,NULL,NULL,TO_ROOM);
    return;
  }

  if (!str_cmp("move",argument))
  {
    if ( cost > ch->train )
    {
      send_to_char( "You don't have enough training sessions.\n\r", ch );
      return;
    }

    if ( ch->pcdata->perm_move >= 40 * level )
    {
      send_to_char( "You have already maxed out your move for this level.\n\r", ch );
      return;
    }

    ch->train -= cost;
    ch->pcdata->perm_move += 10;
    ch->max_move += 10;
    ch->move += 10;
    mprintf(sizeof(log_buf), log_buf, "{g%s{w just used {D%d{w train raising {bmoves{x to {B%d{w, leaving {g%d{x trains.",
            ch->name, cost, ch->pcdata->perm_move, ch->train);
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_SECURE,0,get_trust(ch));
    act( "Your movement increases!",ch,NULL,NULL,TO_CHAR);
    act( "$n's movement increases!",ch,NULL,NULL,TO_ROOM);
    return;
  }

  if ( ch->perm_stat[stat]  >= get_max_train(ch,stat) )
  {
    act( "Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR );
    return;
  }

  if ( cost > ch->train )
  {
    send_to_char( "You don't have enough training sessions.\n\r", ch );
    return;
  }

  ch->train    -= cost;
  ch->perm_stat[stat]           += 1;
  mprintf(sizeof(log_buf), log_buf, "{g%s{w just used {D%d{w train raising {W%s{w to {D%d{w, leaving {g%d{x trains.",
          ch->name, cost, pOutput, ch->perm_stat[stat], ch->train);
  log_string( log_buf );
  wiznet(log_buf,NULL,NULL,WIZ_SECURE,0,get_trust(ch));

  act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
  act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
  return;
}


void do_push_drag( CHAR_DATA *ch, char *argument, char *verb )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *in_room;
  ROOM_INDEX_DATA *to_room=NULL;
  CHAR_DATA *victim;
  EXIT_DATA *pexit;
  OBJ_DATA *obj;
  int door;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  victim = get_char_room(ch,arg1);
  obj = get_obj_list( ch, arg1, ch->in_room->contents );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  {
    printf_to_char(ch, "%s whom or what where?\n\r", capitalize(verb));
    return;
  }

  if ( (!victim || !can_see(ch,victim))
       && (!obj || !can_see_obj(ch,obj)) )
  {
    printf_to_char(ch,"%s whom or what where?\n\r", capitalize(verb));
    return;
  }
  for ( door = 0; door <= 5; door++ ) /* find an exit in the direction */
  {
    if ( !str_prefix( arg2, dir_name[door] ) )
      break;
  }
  if ( door == 6 )
  {
    printf_to_char(ch, "Alas, you cannot %s in that direction.\n\r", verb );
    return;
  }

  if ( obj )
  {
    in_room = obj->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
         ||   ( to_room = pexit->u1.to_room   ) == NULL
         ||     IS_SET( pexit->exit_info, EX_NOEXIT )
         ||  !can_see_room( ch, pexit->u1.to_room ) )
    {
      printf_to_char(ch, "Alas, you cannot %s in that direction.\n\r", verb );
      return;
    }

    if ( !IS_SET(obj->wear_flags, ITEM_TAKE) || !can_loot(ch,obj) )
    {
      send_to_char( "It won't budge.\n\r", ch );
      return;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
         || IS_SET(pexit->exit_info,EX_NOPASS) )
    {
      act( "You cannot $t it through the $d.", ch, verb, pexit->keyword, TO_CHAR );
      act( "$n decides to $t $P around!", ch, verb, obj, TO_ROOM );
      return;
    }

    act( "You attempt to $T $p out of the room.", ch, obj, verb, TO_CHAR );
    act( "$n is attempting to $T $p out of the room.", ch, obj, verb, TO_ROOM );

    if ( obj->weight >  (2 * can_carry_w (ch)) )
    {
      act( "$p is too heavy to $T.\n\r", ch, obj, verb, TO_CHAR);
      act( "$n attempts to $T $p, but it is too heavy.\n\r", ch, obj, verb, TO_ROOM);
      return;
    }
    if    ( !IS_IMMORTAL(ch)
            ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
            ||   IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
            ||   IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)
            ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
            ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
            ||    (number_percent() > 75) )
    {
      send_to_char( "It won't budge.\n\r", ch );
      return;
    }

    if ( ch->move > 10 )
    {
      ch->move -= 10;
      send_to_char( "You succeed!\n\r", ch );
      act( "$n succeeds!", ch, NULL, NULL, TO_ROOM );
      obj_from_room( obj );
      obj_to_room( obj, to_room );
      if (!str_cmp( verb, "drag" ))
        move_char( ch, door, FALSE );
    }
    else
    {
      printf_to_char(ch, "You are too tired to %s anything around!\n\r", verb );
    }
  }
  else
  {
    if ( ch == victim )
    {
      act( "You $t yourself about the room and look very silly.", ch, verb, NULL, TO_CHAR );
      act( "$n decides to be silly and $t $mself about the room.", ch, verb, NULL, TO_ROOM );
      return;
    }

    in_room = victim->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
         ||   ( to_room = pexit->u1.to_room   ) == NULL
         ||     IS_SET( pexit->exit_info, EX_NOEXIT )
         ||  !can_see_room( victim, pexit->u1.to_room ) )
    {
      printf_to_char(ch, "Alas, you cannot %s them that way.\n\r", verb );
      return;
    }
    // Here a check must be made for EX_NOEXIT, was too tired when I saw it
    //  Merak
    if (  IS_SET( pexit->exit_info, EX_CLOSED )
          && ( !IS_AFFECTED( victim, AFF_PASS_DOOR )
               ||    IS_SET( pexit->exit_info,EX_NOPASS ) ) )
    {
      act( "You try to $t them through the $d.", ch, verb, pexit->keyword, TO_CHAR );
      act( "$n decides to $t you around!", ch, verb, victim, TO_VICT );
      act( "$n decides to $t $N around!", ch, verb, victim, TO_NOTVICT );
      return;
    }

    act( "You attempt to $t $N out of the room.", ch, verb, victim, TO_CHAR );
    act( "$n is attempting to $t you out of the room!", ch, verb, victim, TO_VICT );
    act( "$n is attempting to $t $N out of the room.", ch, verb, victim, TO_NOTVICT );

    if    ( !IS_IMMORTAL(ch)
            ||   (IS_NPC(victim)
                  &&   (IS_SET(victim->act,ACT_TRAIN)
                        ||   IS_SET(victim->act,ACT_PRACTICE)
                        ||   IS_SET(victim->act,ACT_IS_HEALER)
                        ||   IS_SET(victim->act,ACT_IS_CHANGER)
                        ||   IS_SET(victim->imm_flags,IMM_SUMMON)
                        ||   victim->pIndexData->pShop ))
            ||   victim->in_room == NULL
            ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
            ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
            ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
            ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
            ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
            ||   (!str_cmp( verb, "drag" ) && victim->position >= POS_STANDING)
            ||   (!str_cmp( verb, "push" ) && victim->position != POS_STANDING)
            ||    is_safe(ch,victim, FALSE)
            ||    (number_percent() > 75)
            ||   victim->level >= ch->level + 5
            ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO) )
    {
      send_to_char( "They won't budge.\n\r", ch );
      return;
    }

    if ( ch->move > 10 )
    {
      ch->move -= 10;
      send_to_char( "You succeed!\n\r", ch );
      act( "$n succeeds!", ch, NULL, NULL, TO_ROOM );
      if (!str_cmp( verb, "drag" ))
        move_char( ch, door, FALSE );
      move_char( victim, door, FALSE );
    }
    else
    {
      printf_to_char(ch, "You are too tired to %s anybody around!\n\r", verb );
    }
  }

  return;
}

void do_push( CHAR_DATA *ch, char *argument )
{
  do_push_drag( ch, argument, "push" );
  return;
}

void do_drag( CHAR_DATA *ch, char *argument )
{
  do_push_drag( ch, argument, "drag" );
  return;
}

void do_mount( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *mount;
  CHAR_DATA *rch;
  char arg[MAX_INPUT_LENGTH];
  int mountcount = 0;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    printf_to_char(ch, "Mount what?\n\r");
    return;
  }

  mount = get_char_room(ch,arg);
  if ( !IS_SET( mount->act2, ACT2_MOUNTABLE ) )
  {
    printf_to_char(ch, "You attempt to climb onto %s, but they don't look happy about it.\n\r",
                   ( IS_NPC( mount ) ? mount->short_descr : mount->name ) );
    return;
  }

  for ( rch = char_list; rch; rch = rch->next )
    if (rch->mount == mount)
      mountcount++;

  if (mount->max_riders <= mountcount)
  {
    printf_to_char(ch, "%s can't carry any more passengers.\n\r",
                   ( IS_NPC( mount ) ? mount->short_descr : mount->name ) );
    return;
  }

  printf_to_char(ch, "You climb onto %s's back.\n\r",
                 ( IS_NPC( mount ) ? mount->short_descr : mount->name ) );
  ch->mount = mount;

  return;
}

void do_dismount( CHAR_DATA *ch, char *argument )
{
  if (ch->mount == NULL)
    printf_to_char(ch, "But you aren't riding anything.\n\r");
  else
    printf_to_char(ch, "You nimbly dismount %s.\n\r",
                   ( IS_NPC( ch->mount ) ? ch->mount->short_descr : ch->mount->name ) );

  ch->mount = NULL;
  return;
}

void do_leave( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *portal;
  ROOM_INDEX_DATA *in_room;
  ROOM_INDEX_DATA *to_room = NULL;
  EXIT_DATA *pexit;
  CHAR_DATA *vch;
  char buf[MSL];
  int door;

  if ( IS_NPC(ch) )
    return;

  in_room = ch->in_room;

  if ( !str_prefix( argument, "north" ) )
  {
    door = DIR_NORTH;
  }
  else if ( !str_prefix( argument, "east"  ) )
  {
    door = DIR_EAST;
  }
  else if ( !str_prefix( argument, "south" ) )
  {
    door = DIR_SOUTH;
  }
  else if ( !str_prefix( argument, "west"  ) )
  {
    door = DIR_WEST;
  }
  else if ( !str_prefix( argument, "up"    ) )
  {
    door = DIR_UP;
  }
  else if ( !str_prefix( argument, "down"  ) )
  {
    door = DIR_DOWN;
  }
  else
  {
    /* **************************************************************************** */
    /*            leave through a portal: THIS IS COPIED FROM do_enter()            */
    /* **************************************************************************** */
    portal = get_obj_list( ch, argument, ch->in_room->contents );

    if ( ( portal == NULL )
         ||   ( portal->item_type != ITEM_PORTAL )
         ||   ( IS_SET( portal->value[1], EX_CLOSED )
                &&  !IS_TRUSTED( ch, ANGEL ) ) )
    {
      send_to_char("Go where?\n\r",ch);
      return;
    }

    if ( !IS_TRUSTED( ch, ANGEL )
         &&   !IS_SET( portal->value[2], GATE_NOCURSE )
         && (  IS_AFFECTED( ch,AFF_CURSE ) ) )
    {
      send_to_char("Something prevents you from leaving...\n\r",ch);
      return;
    }

    if ( IS_SET( portal->value[2], GATE_RANDOM ) || portal->value[3] == -1 )
    {
      to_room = get_random_room(ch);
      portal->value[3] = to_room->vnum; /* for record keeping :) */
    }
    else if ( IS_SET( portal->value[2], GATE_BUGGY ) && ( number_percent() < 5 ) )
      to_room = get_random_room( ch );
    else
      to_room = get_room_index( portal->value[3] );

    if ( to_room == NULL
         ||   to_room == in_room
         ||  !can_see_room( ch, to_room )
         || ( room_is_private( to_room ) && !IS_TRUSTED( ch, IMPLEMENTOR ) ) )
    {
      act( "$p doesn't seem to go anywhere.", ch, portal, NULL, TO_CHAR );
      return;
    }

    if ( IS_NPC( ch )
         &&   IS_SET( ch->act, ACT_AGGRESSIVE )
         &&   IS_SET( to_room->room_flags, ROOM_LAW ) )
    {
      send_to_char( "Something prevents you from leaving...\n\r", ch );
      return;
    }

    act( "$n steps into $p.", ch, portal, NULL, TO_ROOM );

    if ( IS_SET( portal->value[2], GATE_NORMAL_EXIT ) )
      act( "You enter $p.", ch, portal, NULL, TO_CHAR );
    else
      act( "You walk through $p and find yourself somewhere else...", ch, portal, NULL, TO_CHAR );

    move_to_room( ch, to_room );

    if ( IS_SET( portal->value[2], GATE_GOWITH ) ) /* take the gate along */
    {
      obj_from_room( portal );
      obj_to_room( portal, to_room );
    }

    if ( !IS_IMMORTAL( ch ) )
    {
      if ( IS_SET( portal->value[2], GATE_NORMAL_EXIT ) )
        act( "$n has arrived.", ch, portal, NULL, TO_ROOM );
      else
        act( "$n has arrived through $p.", ch, portal, NULL, TO_ROOM );
    }

    /* charges */
    if ( portal->value[0] > 0 )
    {
      portal->value[0]--;
      if ( portal->value[0] == 0 )
        portal->value[0] = -1;
    }

    do_function( ch, &do_look, "auto" );

    if ( portal && portal->value[0] == -1 )
    {
      act( "$p fades out of existence.", ch, portal, NULL, TO_CHAR );
      if ( ch->in_room == in_room )
        act( "$p fades out of existence.", ch, portal, NULL, TO_ROOM );
      else if ( in_room->people )
      {
        act( "$p fades out of existence.",
             in_room->people, portal, NULL, TO_CHAR );
        act( "$p fades out of existence.",
             in_room->people, portal, NULL, TO_ROOM );
      }
      extract_obj( portal );
    }
    /* **************************************************************************** */
    /*         END: leave through a portal: THIS WAS COPIED FROM do_enter()         */
    /* **************************************************************************** */
    return;
  } // end direction/portal check

// If still in function, a valid direction name was been found, no portal involved
  if ( door < 0 || door > 5 )
  {
    bug( "Do_move: bad door %d.", door );
    return;
  }

  in_room = ch->in_room;

  if ( IS_SET( in_room->room_flags, ROOM_SHIP ) )
  {
    if ( in_room->state < 0 || in_room->state > 5 ) pexit = NULL;
    else
    {
      if ( ( pexit = in_room->exit[in_room->state] ) )
        if ( strcmp( dir_name[door], pexit->keyword ) ) pexit = NULL;
    }
  }
  else pexit = in_room->exit[door];

  if ( pexit == NULL
       ||   IS_SET( pexit->exit_info, EX_NOEXIT )
       || ( to_room = pexit->u1.to_room ) == NULL
       ||  !can_see_room( ch, pexit->u1.to_room ) )
  {
    send_to_char( "Alas, you cannot go that way.\n\r", ch );
    return;
  }

  if ( IS_SET( pexit->exit_info, EX_CLOSED )
       && ( !IS_AFFECTED( ch, AFF_PASS_DOOR )
            || ( IS_SET( pexit->exit_info, EX_NOPASS )
                 &&  !IS_GHOST( ch ) ) )
       &&  !IS_TRUSTED( ch, ANGEL ) )
  {
    if (IS_AFFECTED(ch, AFF_BLIND))
    {
      strcpy( buf, "The exit" );
    }
    else
    {
      strcpy( buf, "The " );
      if ( pexit->keyword
           &&   pexit->keyword[0] != '\0'
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
    printf_to_char( ch, "%s closed.\n\r", buf );
    return;
  }

  if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master
       &&   in_room == ch->master->in_room
       &&   ch->fighting == NULL )
  {
    send_to_char( "What?  And leave your beloved master?\n\r", ch );
    return;
  }

  if ( ( !is_room_owner( ch, to_room ) && room_is_private( to_room ) )
       && ( get_trust( ch ) < IMPLEMENTOR ) )
  {
    send_to_char( "That room is private right now.\n\r", ch );
    return;
  }

  if ( IS_SET( to_room->room_flags, ROOM_NO_GHOST )
       &&   IS_GHOST( ch ) )
  {
    send_to_char( "Ghosts are not allowed in there.\n\r", ch );
    return;
  }


  if ( !IS_NPC( ch ) )
  {
    int iClass, iGuild;
    int move;

    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
    {
      for ( iGuild = 0; iGuild < MAX_GUILD; iGuild ++)
      {
        if ( iClass != ch->gameclass
             &&   to_room->vnum == class_table[iClass].guild[iGuild]
             &&  !IS_IMMORTAL( ch ) )
        {
          send_to_char( "You aren't allowed in there.\n\r", ch );
          return;
        }
      }
    }
    if ( in_room->sector_type == SECT_AIR
         ||   to_room->sector_type == SECT_AIR )
    {
      if ( !IS_AFFECTED( ch, AFF_FLYING ) && !IS_IMMORTAL( ch ) )
      {
        send_to_char( "You can't fly.\n\r", ch );
        return;
      }
    }

    if ( ( in_room->sector_type == SECT_WATER_NOSWIM
           ||     to_room->sector_type == SECT_WATER_NOSWIM )
         &&    !IS_AFFECTED( ch, AFF_FLYING )
         &&  !IS_SET( ch->spell_aff, SAFF_WALK_ON_WATER ) )
    {
      OBJ_DATA *obj;
      bool found;

      /*
       * Look for a boat.
       */
      found = IS_IMMORTAL( ch );

      if ( !found )
      {
        for ( obj = ch->carrying; obj; obj = obj->next_content )
        {
          if ( obj->item_type == ITEM_BOAT )
          {
            found = TRUE;
            break;
          }
        }
      }
      if ( !found )
      {
        send_to_char( "You need a boat to go there.\n\r", ch );
        return;
      }
    }

    move = movement_loss[UMIN( SECT_MAX-1, in_room->sector_type )] + movement_loss[UMIN( SECT_MAX-1, to_room->sector_type )];
    move /= 2;    /* i.e. the average */

    /* conditional effects */
    if ( IS_AFFECTED( ch, AFF_FLYING ) || IS_AFFECTED( ch, AFF_HASTE ) )
      move /= 2;

    if ( IS_AFFECTED( ch, AFF_SLOW ) )
      move *= 2;

    if ( !IS_GHOST( ch ) )
      if ( ch->move < move )
      {
        send_to_char( "You are too exhausted.\n\r", ch );
        return;
      }

    WAIT_STATE( ch, 1 );
    if ( !IS_GHOST( ch ) )
      ch->move -= move;
  } // if !NPC

  for ( vch = ch->in_room->people ; vch ; vch = vch->next_in_room )
  {
    if ( IS_AFFECTED( ch, AFF_SNEAK ) )
    {
      if (  IS_AFFECTED( vch, AFF_HIDE )
            || (  IS_IMMORTAL( vch ) && IS_SET( vch->act, PLR_HOLYLIGHT )
                  &&    get_trust( vch ) >= ch->invis_level ) )
      {
        mprintf( sizeof( buf ), buf, "$n sneaks %s.", dir_name[door] );
        act( buf, ch, NULL, vch, TO_VICT );
      }
    }
    else //I don't see any need for more than can_see here, since it checks imms.
    {
      if ( can_see( vch, ch ) )
      {
        mprintf( sizeof( buf ), buf, "$n leaves %s.", dir_name[ door ] );
        act( buf, ch, NULL, vch, TO_VICT );
      }
    }
  }

  move_to_room( ch, to_room );

  for ( vch = ch->in_room->people ; vch ; vch = vch->next_in_room )
  {
    if ( IS_AFFECTED( ch, AFF_SNEAK ) )
    {
      if ( IS_AFFECTED( vch, AFF_HIDE )
           || ( IS_IMMORTAL( vch ) && IS_SET( vch->act, PLR_HOLYLIGHT )
                &&   get_trust( vch ) >= ch->invis_level ) )
      {
        mprintf( sizeof( buf ), buf, "$n sneaks in from %s.", dir_from_name[ door ] );
        act( buf, ch, NULL, vch, TO_VICT );
      }
    }
    else //I don't see any need for more than can_see here, since it checks imms.
    {
      if ( can_see( vch, ch ) )
      {
        mprintf( sizeof( buf ), buf, "$n has arrived from %s.", dir_from_name[ door ] );
        act( buf, ch, NULL, vch, TO_VICT );
      }
    }
  } // loop: chars in room

  do_function( ch, &do_look, "" );



// If still in function, an existing door must have been found
  if ( can_carry_w( ch ) < get_carry_weight( ch ) - 1 )
  {
    send_to_char("You are carrying more than you can take with you.\n\r",ch);
    return;
  }

  if ( ch->pcdata->condition[COND_DRUNK] >= 5 /* Is char. drunk? */
       &&   number_percent() < 50 )
  {
    act( "$n stumbles around drunk.", ch, NULL, NULL, TO_ROOM );
    act( "You stumble around drunk.", ch, NULL, NULL, TO_CHAR );
    door = number_range( 0, 5 );
  }

// hallucinating (poison)
  if ( IS_AFFECTED(ch, AFF_POISON)
       &&   number_percent() < 33 ) /* 33% chance */
  {
    AFFECT_DATA *poison;
    if ( ( ( poison = affect_find(ch->affected,gsn_poison) ) != NULL )
         &&     ( poison->modifier == TOX_HALLUCINOGENIC) )
      door = number_range( 0, 5 );
  }

  mp_greet_trigger( ch, dir_name[rev_dir[door]] );
  ap_gronce_trigger( ch );

  if ( IS_SET( ch->act, PLR_AUTOHUNT )
       &&   !IS_NULLSTR( ch->tracking ) )
  {
    real_track( ch, ch->tracking );
  }

}

void do_transport( CHAR_DATA *ch, char *argument )
{
/*
  int start_point = 0;
  int alin_trans = 8472;
  int drek_trans = 5777;
  int pals_trans = 4008;
*/
  if ( IS_NPC(ch) )
    return;

  if ( IS_NULLSTR(argument) )
  {
    send_to_char( "Where do you want to go?\n\r", ch );
    return;
  }
/*
  if ( ch->in_room->vnum == alin_trans )
    start_point = alin_trans;
  else if ( ch->in_room->vnum == drek_trans )
    start_point = drek_trans;
  else if ( ch->in_room->vnum == pals_trans )
    start_point = pals_trans;
  else
  {
    send_to_char( "You cannot transport from here.\n\r", ch );
    return;
  }
*/
  send_to_char( "Where do you want to go?\n\r", ch );
}
