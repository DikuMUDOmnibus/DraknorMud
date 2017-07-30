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
*    ROM 2.4 is copyright 1993-1995 Russ Taylor               *
*    ROM has been brought to you by the ROM consortium           *
*        Russ Taylor (rtaylor@pacinfo.com)                   *
*        Gabrielle Taylor (gtaylor@pacinfo.com)               *
*        Brian Moore (rom@rom.efn.org)                   *
*    By using this code, you have agreed to follow the terms of the       *
*    ROM license, in the file Rom24/doc/rom.license               *
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  MOBprograms for ROM 2.4 v0.98g (C) M.Nylander 1996                     *
 *  Based on MERC 2.2 MOBprograms concept by N'Atas-ha.                    *
 *  Written and adapted to ROM 2.4 by                                      *
 *          Markku Nylander (markku.nylander@uta.fi)                       *
 *  This code may be copied and distributed as per the ROM license.        *
 *                                                                         *
 ***************************************************************************/

/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include "merc.h"
#include "tables.h"
#include "lookup.h"

extern int64 flag_lookup( const char *word, const struct flag_type *flag_table );
// #ifdef TEST_MUD_4
extern CHAR_DATA *prog_master;
// #endif
/*
 * These defines correspond to the entries in fn_keyword[] table.
 * If you add a new if_check, you must also add a #define here.
 */
#define CHK_RAND           (0)
#define CHK_MOBHERE     (1)
#define CHK_OBJHERE     (2)
#define CHK_MOBEXISTS   (3)
#define CHK_OBJEXISTS   (4)
#define CHK_PEOPLE      (5)
#define CHK_PLAYERS     (6)
#define CHK_MOBS        (7)
#define CHK_CLONES      (8)
#define CHK_ORDER       (9)
#define CHK_HOUR        (10)
#define CHK_ISPC        (11)
#define CHK_ISNPC       (12)
#define CHK_ISGOOD      (13)
#define CHK_ISEVIL      (14)
#define CHK_ISNEUTRAL   (15)
#define CHK_ISIMMORT    (16)
#define CHK_ISCHARM     (17)
#define CHK_ISFOLLOW    (18)
#define CHK_ISACTIVE    (19)
#define CHK_ISDELAY     (20)
#define CHK_ISVISIBLE   (21)
#define CHK_HASTARGET   (22)
#define CHK_ISTARGET    (23)
#define CHK_EXISTS      (24)
#define CHK_AFFECTED    (25)
#define CHK_ACT         (26)
#define CHK_OFF         (27)
#define CHK_IMM         (28)
#define CHK_CARRIES     (29)
#define CHK_WEARS       (30)
#define CHK_HAS         (31)
#define CHK_USES        (32)
#define CHK_NAME        (33)
#define CHK_POS         (34)
#define CHK_CLAN        (35)
#define CHK_RACE        (36)
#define CHK_CLASS       (37)
#define CHK_OBJTYPE     (38)
#define CHK_VNUM        (39)
#define CHK_HPCNT       (40)
#define CHK_ROOM        (41)
#define CHK_SEX         (42)
#define CHK_LEVEL       (43)
#define CHK_ALIGN       (44)
#define CHK_MONEY       (45)
#define CHK_OBJVAL0     (46)
#define CHK_OBJVAL1     (47)
#define CHK_OBJVAL2     (48)
#define CHK_OBJVAL3     (49)
#define CHK_OBJVAL4     (50)
#define CHK_GRPSIZE     (51)
#define CHK_RSTATE      (52)
#define CHK_ISOPEN      (53)
#define CHK_CANCARRY    (54)
#define CHK_HASITEMS    (55)
#define CHK_ISFIGHTING  (56)
#define CHK_ISSAFE      (57)
#define CHK_MPQUEST     (58)
#define CHK_REMEMBERS   (59)

/*
 * These defines correspond to the entries in fn_evals[] table.
 */
#define EVAL_EQ            0
#define EVAL_GE            1
#define EVAL_LE            2
#define EVAL_GT            3
#define EVAL_LT            4
#define EVAL_NE            5

/*
 * if-check keywords:
 */
const char * fn_keyword[] =
{
  "rand",        /* if rand 30        - if random number < 30 */
  "mobhere",        /* if mobhere fido    - is there a 'fido' here */
  "objhere",        /* if objhere bottle    - is there a 'bottle' here */
  /* if mobhere 1233    - is there mob vnum 1233 here */
  /* if objhere 1233    - is there obj vnum 1233 here */
  "mobexists",    /* if mobexists fido    - is there a fido somewhere */
  "objexists",    /* if objexists sword    - is there a sword somewhere */

  "people",        /* if people > 4    - does room contain > 4 people */
  "players",        /* if players > 1    - does room contain > 1 pcs */
  "mobs",        /* if mobs > 2        - does room contain > 2 mobiles */
  "clones",        /* if clones > 3    - are there > 3 mobs of same vnum here */
  "order",        /* if order == 0    - is mob the first in room */
  "hour",        /* if hour > 11        - is the time > 11 o'clock */

  "ispc",        /* if ispc $n         - is $n a pc */
  "isnpc",        /* if isnpc $n         - is $n a mobile */
  "isgood",        /* if isgood $n     - is $n good */
  "isevil",        /* if isevil $n     - is $n evil */
  "isneutral",    /* if isneutral $n     - is $n neutral */
  "isimmort",        /* if isimmort $n    - is $n immortal */
  "ischarm",        /* if ischarm $n    - is $n charmed */
  "isfollow",        /* if isfollow $n    - is $n following someone */
  "isactive",        /* if isactive $n    - is $n's position > SLEEPING */
  "isdelay",        /* if isdelay $i    - does $i have mobprog pending */
  "isvisible",    /* if isvisible $n    - can mob see $n */
  "hastarget",    /* if hastarget $i    - does $i have a valid target */
  "istarget",        /* if istarget $n    - is $n mob's target */
  "exists",        /* if exists $n        - does $n exist somewhere */

  "affected",        /* if affected $n blind - is $n affected by blind */
  "act",        /* if act $i sentinel    - is $i flagged sentinel */
  "off",              /* if off $i berserk    - is $i flagged berserk */
  "imm",              /* if imm $i fire    - is $i immune to fire */
  "carries",        /* if carries $n sword    - does $n have a 'sword' */
  /* if carries $n 1233    - does $n have obj vnum 1233 */
  "wears",        /* if wears $n lantern    - is $n wearing a 'lantern' */
  /* if wears $n 1233    - is $n wearing obj vnum 1233 */
  "has",            /* if has $n weapon    - does $n have obj of type weapon */
  "uses",        /* if uses $n armor    - is $n wearing obj of type armor */
  "name",        /* if name $n puff    - is $n's name 'puff' */
  "pos",        /* if pos $n standing    - is $n standing */
  "clan",        /* if clan $n 'whatever'- does $n belong to clan 'whatever' */
  "race",        /* if race $n dragon    - is $n of 'dragon' race */
  "class",        /* if class $n mage    - is $n's class 'mage' */
  "objtype",        /* if objtype $p scroll    - is $p a scroll */

  "vnum",        /* if vnum $i == 1233      - virtual number check */
  "hpcnt",        /* if hpcnt $i > 30    - hit point percent check */
  "room",        /* if room $i == 1233    - room virtual number */
  "sex",        /* if sex $i == 0    - sex check */
  "level",        /* if level $n < 5    - level check */
  "align",        /* if align $n < -1000    - alignment check */
  "money",        /* if money $n */
  "objval0",        /* if objval0 $p = 1000     - object value[] checks 0..4 */
  "objval1",
  "objval2",
  "objval3",
  "objval4", /* ******** (50th entry) ******** */
  "grpsize",        /* if grpsize $n > 6    - group size check */
  "rstate",     /* if rstate < 0 - room->state check */
  "isopen",     /* if isopen west   - is exit west open? */
  "cancarry",     /* if mob can carry object (weight check) */
  "hasitems",     /* if mob can carry object (weight check) */
  "isfighting", /* if mob $n is fighting */
  "issafe", /* if mob $n is in a safe room*/
  "mpquest", /* if char $n has questbit # set */
  "remembers", /* if mob remembers char $n */
  "\n"        /* Table terminator */
};

const char *fn_evals[] =
{
  "==",
  ">=",
  "<=",
  ">",
  "<",
  "!=",
  "\n"
};

/*
 * Return a valid keyword from a keyword table
 */
int keyword_lookup( const char **table, char *keyword )
{
  register int i;
  for( i = 0; table[i][0] != '\n'; i++ )
    if( !str_cmp( table[i], keyword ) )
      return( i );
  return -1;
}

/*
 * Perform numeric evaluation.
 * Called by cmd_eval()
 */
int num_eval( int lval, int oper, int rval )
{
  switch( oper )
    {
    case EVAL_EQ:
      return ( lval == rval );
    case EVAL_GE:
      return ( lval >= rval );
    case EVAL_LE:
      return ( lval <= rval );
    case EVAL_NE:
      return ( lval != rval );
    case EVAL_GT:
      return ( lval > rval );
    case EVAL_LT:
      return ( lval < rval );
    default:
      bug( "num_eval: invalid oper", 0 );
      return 0;
    }
}

/* This is my own version of assign/release prog_master.
   May heaven forgive me :S Merak 2006 - 10 - 01 */
CHAR_DATA *assign_prog_master( void *vo, CHAR_DATA *ch, int prog_type )
{
  ROOM_INDEX_DATA   *room = NULL;
  OBJ_DATA          *obj;
  OBJ_DATA          *obj1;

  if ( prog_master->in_room ) return ( NULL );
  if ( prog_type == OBJ_PROG )
  {
    obj = ( OBJ_DATA *)vo;
    if ( ( obj1 = obj->in_obj ) )
    {
      if ( !( room = obj1->in_room ) )
        if ( !obj1->carried_by || !( room = obj1->carried_by->in_room ) )
          return NULL;
    }
    if ( !room && !( room = obj->in_room ) )
      if ( !obj->carried_by || !( room = obj->carried_by->in_room ) )
        return NULL;

    prog_master->name               = obj->name;
    prog_master->short_descr        = obj->short_descr;
    prog_master->level              = obj->level;
    prog_master->pIndexData->mprogs = obj->pIndexData->mprogs;
    prog_master->pIndexData->mprog_flags = obj->pIndexData->mprog_flags;
/* room for future plans.......   
    prog_master->mprog_delay        = obj->mprog_delay;
    prog_master->mprog_target       = obj->mprog_target;
*/
  }
  else if ( prog_type == ROOM_PROG )
  {
    room = (ROOM_INDEX_DATA *)vo;

    prog_master->name               = "room";
    prog_master->short_descr        = room->name;
    prog_master->level              = 1;
    prog_master->pIndexData->mprogs = room->mprogs;
    prog_master->pIndexData->mprog_flags = room->mprog_flags;
    prog_master->mprog_delay        = room->mprog_delay;
    prog_master->mprog_target       = room->mprog_target;
  }

  char_to_room( prog_master, room );
  return ( prog_master );
}

void release_prog_master( void *vo, int prog_type )
{
  ROOM_INDEX_DATA   *room;
/* This does nothing? - Taeloch
  OBJ_DATA          *obj;
  if ( prog_type == OBJ_PROG )
  {
    obj = ( OBJ_DATA *)vo;
  }
  else if...
*/
  if ( prog_type == ROOM_PROG )
  {
    room = ( ROOM_INDEX_DATA *)vo;
    room->mprog_delay   = prog_master->mprog_delay;
    room->mprog_target  = prog_master->mprog_target;
  }
  prog_master->pIndexData->mprogs = NULL;
  prog_master->pIndexData->mprog_flags = 0;

  char_from_room( prog_master );
}


/*
 * ---------------------------------------------------------------------
 * UTILITY FUNCTIONS USED BY CMD_EVAL()
 * ----------------------------------------------------------------------
 */

/*
 * Get a random PC in the room (for $r parameter)
 */
CHAR_DATA *get_random_char( CHAR_DATA *mob )
{
  CHAR_DATA *vch, *victim = NULL;
  int now = 0, highest = 0;
  for( vch = mob->in_room->people; vch; vch = vch->next_in_room )
  {
    if ( mob != vch 
    &&  !IS_NPC( vch ) 
    &&   can_see( mob, vch )
    && ( now = number_percent() ) > highest )
    {
      victim = vch;
      highest = now;
    }
  }
  return victim;
}

/* 
 * How many other players / mobs are there in the room
 * iFlag: 0: all, 1: players, 2: mobiles 3: mobs w/ same vnum 4: same group
 */
int count_people_room( CHAR_DATA *mob, int iFlag )
{
  CHAR_DATA *vch;
  int count;
  for ( count = 0, vch = mob->in_room->people; vch; vch = vch->next_in_room )
    if ( mob != vch 
    && ( iFlag == 0
    || ( iFlag == 1 && !IS_NPC( vch ) ) 
    || ( iFlag == 2 && IS_NPC( vch ) )
    || ( iFlag == 3 && IS_NPC( mob ) && IS_NPC( vch ) 
    &&   mob->pIndexData->vnum == vch->pIndexData->vnum )
    || ( iFlag == 4 && is_same_group( mob, vch ) ) )
    &&   can_see( mob, vch ) )
      count++;
  return ( count );
}

/*
 * Get the order of a mob in the room. Useful when several mobs in
 * a room have the same trigger and you want only the first of them
 * to act 
 */
int get_order( CHAR_DATA *ch )
{
  CHAR_DATA *vch;
  int i;

  if ( !IS_NPC( ch ) )
    return 0;
  for ( i = 0, vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
      if ( vch == ch )
    return i;
      if ( IS_NPC( vch ) 
      &&   vch->pIndexData->vnum == ch->pIndexData->vnum )
    i++;
    }
  return 0;
}

/*
 * Check if ch has a given item or item type
 * vnum: item vnum or -1
 * item_type: item type or -1
 * fWear: TRUE: item must be worn, FALSE: don't care
 */
bool has_item( CHAR_DATA *ch, sh_int vnum, sh_int item_type, bool fWear )
{
  OBJ_DATA *obj;
  for ( obj = ch->carrying; obj; obj = obj->next_content )
    if ( (  vnum < 0      || obj->pIndexData->vnum == vnum )
    &&   (  item_type < 0 || obj->pIndexData->item_type == item_type )
    &&   ( !fWear         || obj->wear_loc != WEAR_NONE ) )
      return TRUE;
  return FALSE;
}

/*
 * Check if there's a mob with given vnum in the room
 */
bool get_mob_vnum_room( CHAR_DATA *ch, sh_int vnum )
{
  CHAR_DATA *mob;
  for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    if ( IS_NPC( mob ) && mob->pIndexData->vnum == vnum )
      return TRUE;
  return FALSE;
}

/*
 * Check if there's an object with given vnum in the room
 */
bool get_obj_vnum_room( CHAR_DATA *ch, sh_int vnum )
{
  OBJ_DATA *obj;
  for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
    if ( obj->pIndexData->vnum == vnum )
      return TRUE;
  return FALSE;
}

/*
 * Check if an exit is passable
 */
bool isopen( CHAR_DATA *ch, char *arg )
{
  EXIT_DATA *pexit;
  int door;

  if ( ( door = find_door( ch, arg ) ) < 0
  ||   ( pexit = ch->in_room->exit[door] ) == NULL
  ||     IS_SET( pexit->exit_info, EX_CLOSED )
  ||     IS_SET( pexit->exit_info, EX_NOEXIT ) )
    return FALSE;

  return TRUE;
}

/* ---------------------------------------------------------------------
 * CMD_EVAL
 * This monster evaluates an if/or/and statement
 * There are five kinds of statement:
 * 1) keyword and value (no $-code)        if random 30
 * 2) keyword, comparison and value        if people > 2
 * 3) keyword and actor                    if isnpc $n
 * 4) keyword, actor and value            if carries $n sword
 * 5) keyword, actor, comparison and value  if level $n >= 10
 *
 *----------------------------------------------------------------------
 */
int cmd_eval( sh_int vnum, char *line, int check,
    CHAR_DATA *mob, CHAR_DATA *ch, 
    const void *arg1, const void *arg2, CHAR_DATA *rch )
{
  CHAR_DATA *lval_char = mob;
  CHAR_DATA *vch = (CHAR_DATA *) arg2;
  OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
  OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
  OBJ_DATA  *lval_obj = NULL;

  char *original, buf[MAX_INPUT_LENGTH], code;
  int lval = 0, oper = 0, rval = -1;
  int eval = -1;

  original = line;
  line = one_argument( line, buf );
  if ( buf[0] == '\0' || mob == NULL )
    return FALSE;

  /*
   * If this mobile has no target, let's assume our victim is the one
   */
/* Let's not!  It makes remembering mostly useless - Taeloch
  if ( mob->mprog_target == NULL )
    mob->mprog_target = ch;
*/
  switch ( check )
    {
      /*
       * Case 1: keyword and value
       */
    case CHK_RAND:
      return( atoi( buf ) < number_percent() );
    case CHK_ISOPEN:
      return( isopen( mob, buf ) );
    case CHK_MOBHERE:
      if ( is_number( buf ) )
        return( get_mob_vnum_room( mob, atoi( buf ) ) );
      else
        return( (bool) ( get_char_room( mob, buf ) != NULL ) );
    case CHK_OBJHERE:
      if ( is_number( buf ) )
        return( get_obj_vnum_room( mob, atoi( buf ) ) );
      else
        return( (bool) ( get_obj_here( mob, buf ) != NULL ) );
    case CHK_MOBEXISTS:
      return( (bool) ( get_char_world( mob, buf ) != NULL ) );
    case CHK_OBJEXISTS:
      return( (bool) ( get_obj_world( mob, buf )  != NULL ) );
      /*
       * Case 2 begins here: We sneakily use rval to indicate need
       *                for numeric eval...
       */
        /* we no longer use rval, but eval instead - Merak */
    case CHK_PEOPLE:
      rval = count_people_room( mob, 0 ); eval = 1; break;
    case CHK_PLAYERS:
      rval = count_people_room( mob, 1 ); eval = 1; break;
    case CHK_MOBS:
      rval = count_people_room( mob, 2 ); eval = 1; break;
    case CHK_CLONES:
      rval = count_people_room( mob, 3 ); eval = 1; break;
    case CHK_ORDER:
      rval = get_order( mob ); eval = 1; break;
    case CHK_HOUR:
      rval = time_info.hour; eval = 1;  break;
    case CHK_RSTATE:
      rval = mob->in_room->state; eval = 1; break;
    default:;
    }

  /*
   * Case 2 continued: evaluate expression
   */
  if ( eval >= 0 )
  {
    if ( ( oper = keyword_lookup( fn_evals, buf ) ) < 0 )
    {
      mprintf(sizeof(buf),buf, "Cmd_eval: prog %d syntax error(2) '%s'",
           vnum, original );
      bug( buf, 0 );
      return FALSE;
    }
    one_argument( line, buf );
    lval = rval;
    rval = atoi( buf );
    return( num_eval( lval, oper, rval ) );
  }

  /*
   * Case 3,4,5: Grab actors from $* codes
   */
  if ( buf[0] != '$' || buf[1] == '\0' )
    {
      mprintf(sizeof(buf),buf, "Cmd_eval: prog %d syntax error(3) '%s'",
           vnum, original );
      bug( buf, 0 );
      return FALSE;
    }
  else
    code = buf[1];
  switch( code )
    {
    case 'i':
      lval_char = mob; break;
    case 'n':
      lval_char = ch; break;
    case 't':
      lval_char = vch; break;
    case 'r':
      lval_char = rch == NULL ? get_random_char( mob ) : rch ; break;
    case 'o':
      lval_obj = obj1; break;
    case 'p':
      lval_obj = obj2; break;
    case 'q':
      lval_char = mob->mprog_target; break;
    default:
      mprintf(sizeof(buf),buf, "Cmd_eval: prog %d syntax error(4) '%s'",
           vnum, original );
      bug( buf, 0 );
      return FALSE;
    }
  /*
   * From now on, we need an actor, so if none was found, bail out
   */
  if ( lval_char == NULL && lval_obj == NULL )
    return FALSE;

  /*
   * Case 3: Keyword, comparison and value
   */
  switch( check )
    {
    case CHK_ISPC:
      return( lval_char != NULL && !IS_NPC( lval_char ) );
    case CHK_ISNPC:
      return( lval_char != NULL && IS_NPC( lval_char ) );
    case CHK_ISGOOD:
      return( lval_char != NULL && IS_GOOD( lval_char ) );
    case CHK_ISEVIL:
      return( lval_char != NULL && IS_EVIL( lval_char ) );
    case CHK_ISNEUTRAL:
      return( lval_char != NULL && IS_NEUTRAL( lval_char ) );
    case CHK_ISIMMORT:
      return( lval_char != NULL && IS_IMMORTAL( lval_char ) );
    case CHK_ISCHARM: /* A relic from MERC 2.2 MOBprograms */
      return( lval_char != NULL && IS_AFFECTED( lval_char, AFF_CHARM ) );
    case CHK_ISFOLLOW:
      return( lval_char != NULL && lval_char->master != NULL 
          && lval_char->master->in_room == lval_char->in_room );
    case CHK_ISACTIVE:
      return( lval_char != NULL && lval_char->position > POS_SLEEPING );
    case CHK_ISFIGHTING:
      return( lval_char != NULL && lval_char->position == POS_FIGHTING);
    case CHK_ISSAFE:
      if (lval_char == NULL) return FALSE;
      if (IS_SET(lval_char->in_room->room_flags,ROOM_SAFE)) return TRUE;
    
      if (IS_SET(lval_char->act,ACT_TRAIN)
      ||  IS_SET(lval_char->act,ACT_PRACTICE)
      ||  IS_SET(lval_char->act,ACT_IS_HEALER)
      ||  IS_SET(lval_char->act,ACT_IS_CHANGER)
      || (lval_char->spec_fun == spec_lookup( "spec_questmaster" )))
        return TRUE;

      return FALSE;
    case CHK_ISDELAY:
      return( lval_char != NULL && lval_char->mprog_delay > 0 );
    case CHK_ISVISIBLE:
      switch( code )
    {
    default :
    case 'i':
    case 'n':
    case 't':
    case 'r':
    case 'q':
      return( lval_char != NULL && can_see( mob, lval_char ) );
    case 'o':
    case 'p':
      return( lval_obj != NULL && can_see_obj( mob, lval_obj ) );
    }
    case CHK_HASTARGET:
      return( lval_char != NULL && lval_char->mprog_target != NULL
          &&  lval_char->in_room == lval_char->mprog_target->in_room );
    case CHK_ISTARGET:
      return( lval_char != NULL && mob->mprog_target == lval_char );
    case CHK_HASITEMS:
      return( (lval_char != NULL) && (lval_char->carrying != NULL ) );
    default:;
    }

  /* 
   * Case 4: Keyword, actor and value
   */
  line = one_argument( line, buf );
  switch( check )
    {
    case CHK_AFFECTED:
      return( lval_char && 
              IS_SET(lval_char->affected_by, flag_lookup(buf, affect_flags)) );
    case CHK_ACT:
      return( lval_char && 
              IS_SET(lval_char->act, flag_lookup(buf, act_flags)) );
    case CHK_IMM:
      return( lval_char &&
              IS_SET(lval_char->imm_flags, flag_lookup(buf, imm_flags)) );
    case CHK_OFF:
      return( lval_char &&
              IS_SET(lval_char->off_flags, flag_lookup(buf, off_flags)) );
    case CHK_CARRIES:
      if ( is_number( buf ) )
    return( lval_char && has_item( lval_char, atoi(buf), -1, FALSE ) );
      else
    return( lval_char && (get_obj_carry( lval_char, buf, lval_char ) ) );

    case CHK_MPQUEST:
      if ( !is_number( buf ) )
        return FALSE;
      if (IS_NPC(lval_char))
        return FALSE;
      if ((atoi(buf) > 63) || (atoi(buf) < 0))
        return FALSE;
      return( lval_char && IS_SET(lval_char->pcdata->mpquests, pow(2,atoi(buf))));
      break;

    case CHK_WEARS:
      if ( is_number( buf ) )
    return( lval_char && has_item( lval_char, atoi(buf), -1, TRUE ) );
      else
    return( lval_char && (get_obj_wear( lval_char, buf ) ) );
    case CHK_HAS:
      return( lval_char && has_item( lval_char, -1, item_lookup(buf), FALSE ) );
    case CHK_USES:
      return( lval_char && has_item( lval_char, -1, item_lookup(buf), TRUE ) );
    case CHK_REMEMBERS:
      if ((mob->mprog_target != NULL) && (lval_char != NULL) && (mob->mprog_target == lval_char))
        return TRUE;
      else
        return FALSE;
    case CHK_NAME:
      switch( code )
    {
    default :
    case 'i':
    case 'n':
    case 't':
    case 'r':
    case 'q':
      return is_name( buf, lval_char->name );

    case 'o':
    case 'p':
      return( lval_obj && is_name( buf, lval_obj->name ) );
    }
    case CHK_POS:
      return( lval_char && lval_char->position == position_lookup( buf ) );
    case CHK_CLAN:
      return( lval_char && lval_char->clan == get_clan( buf, FALSE ) ); //clan_lookup( buf ) );
    case CHK_RACE:
      return( lval_char && lval_char->race == race_lookup( buf ) );
    case CHK_CLASS:
      return( lval_char && lval_char->gameclass == class_lookup( buf ) );
    case CHK_OBJTYPE:
      return( lval_obj && lval_obj->item_type == item_lookup( buf ) );
    case CHK_CANCARRY:
      return( lval_char
      &&    ( ( lval_obj = get_obj_carry( mob, buf, mob ) ) != NULL )
      &&    ( ( lval_char->carry_number + 1 ) <= can_carry_n( lval_char ) )
      &&    ( ( get_carry_weight( lval_char ) + get_obj_weight( lval_obj ) ) < can_carry_w( lval_char ) ) );
    default:;
    }

  /*
   * Case 5: Keyword, actor, comparison and value
   */
  if ( ( oper = keyword_lookup( fn_evals, buf ) ) < 0 )
    {
      mprintf(sizeof(buf),buf, "Cmd_eval: prog %d syntax error(5): '%s'",
           vnum, original );
      bug( buf, 0 );
      return FALSE;
    }
  one_argument( line, buf );
  rval = atoi( buf );

  switch( check )
    {
    case CHK_VNUM:
      switch( code )
    {
    default :
    case 'i':
    case 'n':
    case 't':
    case 'r':
    case 'q':
      if( lval_char && IS_NPC( lval_char ) )
        lval = lval_char->pIndexData->vnum;
      break;
    case 'o':
    case 'p':
      if ( lval_obj )
        lval = lval_obj->pIndexData->vnum;
    }
      break;
    case CHK_HPCNT:
      if ( lval_char ) lval = (lval_char->hit * 100)/(UMAX(1,lval_char->max_hit)); break;
    case CHK_ROOM:
      if ( lval_char && lval_char->in_room != NULL )
    lval = lval_char->in_room->vnum; break;
    case CHK_SEX:
      if ( lval_char ) lval = lval_char->sex; break;
    case CHK_LEVEL:
      if ( lval_char ) lval = lval_char->level; break;
    case CHK_ALIGN:
      if ( lval_char ) lval = lval_char->alignment; break;
    case CHK_MONEY:  /* Money is converted to silver... */
      if ( lval_char ) 
        lval = lval_char->silver + (lval_char->gold * 100); break;
    case CHK_OBJVAL0:
      if ( lval_obj ) lval = lval_obj->value[0]; break;
    case CHK_OBJVAL1:
      if ( lval_obj ) lval = lval_obj->value[1]; break;
    case CHK_OBJVAL2: 
      if ( lval_obj ) lval = lval_obj->value[2]; break;
    case CHK_OBJVAL3:
      if ( lval_obj ) lval = lval_obj->value[3]; break;
    case CHK_OBJVAL4:
      if ( lval_obj ) lval = lval_obj->value[4]; break;
    case CHK_GRPSIZE:
      if( lval_char ) lval = count_people_room( lval_char, 4 ); break;
    default:
      return FALSE;
    }
  return( num_eval( lval, oper, rval ) );
}

/*
 * ------------------------------------------------------------------------
 * EXPAND_ARG
 * This is a hack of act() in comm.c. I've added some safety guards,
 * so that missing or invalid $-codes do not crash the server
 * ------------------------------------------------------------------------
 */
void expand_arg( char *buf, 
    const char *format, 
    CHAR_DATA *mob, CHAR_DATA *ch, 
    const void *arg1, const void *arg2, CHAR_DATA *rch )
{
  static char * const he_she  [] = { "it",  "he",  "she" };
  static char * const him_her [] = { "it",  "him", "her" };
  static char * const his_her [] = { "its", "his", "her" };
  const char *someone = "someone";
  const char *something = "something";
  const char *someones = "someone's";
 
  char fname[MAX_INPUT_LENGTH];
  CHAR_DATA *vch = (CHAR_DATA *) arg2;
  OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
  OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
  const char *str;
  const char *i;
  char *point;
 
  /*
   * Discard null and zero-length messages.
   */
  if ( format == NULL || format[0] == '\0' )
    return;

  point   = buf;
  str     = format;
  while ( *str != '\0' )
    {
      if ( *str != '$' )
        {
      *point++ = *str++;
      continue;
        }
      ++str;

      switch ( *str )
        {
    default:  bug( "Expand_arg: bad code %d.", *str );
      i = " <@@@> ";                        break;
    case 'i':
      one_argument( mob->name, fname );
      i = fname;                                 break;
    case 'I': i = mob->short_descr;                     break;
    case 'n': 
      i = someone;
      if ( ch != NULL && can_see( mob, ch ) )
        {
          one_argument( ch->name, fname );
          i = capitalize(fname);
        }                        break;
    case 'N': 
      i = (ch != NULL && can_see( mob, ch ) )
        ? ( IS_NPC( ch ) ? ch->short_descr : ch->name )
        : someone;                                 break;
    case 't': 
      i = someone;
      if ( vch != NULL && can_see( mob, vch ) )
        {
          one_argument( vch->name, fname );
          i = capitalize(fname);
        }                        break;
    case 'T': 
      i = (vch != NULL && can_see( mob, vch ))
        ? ( IS_NPC( vch ) ? vch->short_descr : vch->name )
        : someone;                                 break;
    case 'r': 
      if ( rch == NULL ) 
        rch = get_random_char( mob );
      i = someone;
      if( rch != NULL && can_see( mob, rch ) )
        {
          one_argument( rch->name, fname );
          i = capitalize(fname);
        }                         break;
    case 'R': 
      if ( rch == NULL ) 
        rch = get_random_char( mob );
      i  = ( rch != NULL && can_see( mob, rch ) )
        ? ( IS_NPC( ch ) ? ch->short_descr : ch->name )
        :someone;                    break;
    case 'q':
      i = someone;
      if ( mob->mprog_target != NULL && can_see( mob, mob->mprog_target ) )
        {
          one_argument( mob->mprog_target->name, fname );
          i = capitalize( fname );
        }                         break;
    case 'Q':
      i = (mob->mprog_target != NULL && can_see( mob, mob->mprog_target ))
        ? ( IS_NPC( mob->mprog_target ) ? mob->mprog_target->short_descr : mob->mprog_target->name )
        : someone;                                 break;
    case 'j': i = he_she  [URANGE(0, mob->sex, 2)];     break;
    case 'e': 
      i = (ch != NULL && can_see( mob, ch ))
        ? he_she  [URANGE(0, ch->sex, 2)]        
        : someone;                    break;
    case 'E': 
      i = (vch != NULL && can_see( mob, vch ))
        ? he_she  [URANGE(0, vch->sex, 2)]        
        : someone;                    break;
    case 'J': 
      i = (rch != NULL && can_see( mob, rch ))
        ? he_she  [URANGE(0, rch->sex, 2)]        
        : someone;                    break;
    case 'X':
      i = (mob->mprog_target != NULL && can_see( mob, mob->mprog_target))
        ? he_she  [URANGE(0, mob->mprog_target->sex, 2)]
        : someone;                    break;
    case 'k': i = him_her [URANGE(0, mob->sex, 2)];    break;
    case 'm': 
      i = (ch != NULL && can_see( mob, ch ))
        ? him_her [URANGE(0, ch  ->sex, 2)]
        : someone;                        break;
    case 'M': 
      i = (vch != NULL && can_see( mob, vch ))
        ? him_her [URANGE(0, vch ->sex, 2)]        
        : someone;                    break;
    case 'K': 
      if ( rch == NULL ) 
        rch = get_random_char( mob );
      i = (rch != NULL && can_see( mob, rch ))
        ? him_her [URANGE(0, rch ->sex, 2)]
        : someone;                    break;
    case 'Y': 
      i = (mob->mprog_target != NULL && can_see( mob, mob->mprog_target ))
        ? him_her [URANGE(0, mob->mprog_target->sex, 2)]        
        : someone;                    break;
    case 'l': i = his_her [URANGE(0, mob ->sex, 2)];    break;
    case 's': 
      i = (ch != NULL && can_see( mob, ch ))
        ? his_her [URANGE(0, ch ->sex, 2)]
        : someones;                    break;
    case 'S': 
      i = (vch != NULL && can_see( mob, vch ))
        ? his_her [URANGE(0, vch ->sex, 2)]
        : someones;                    break;
    case 'L': 
      if ( rch == NULL ) 
        rch = get_random_char( mob );
      i = ( rch != NULL && can_see( mob, rch ) )
        ? his_her [URANGE(0, rch ->sex, 2)]
        : someones;                    break;
    case 'Z': 
      i = (mob->mprog_target != NULL && can_see( mob, mob->mprog_target ))
        ? his_her [URANGE(0, mob->mprog_target->sex, 2)]
        : someones;                    break;
    case 'o':
      i = something;
      if ( obj1 != NULL && can_see_obj( mob, obj1 ) )
        {
          one_argument( obj1->name, fname );
          i = fname;
        }                         break;
    case 'O':
      i = (obj1 != NULL && can_see_obj( mob, obj1 ))
        ? obj1->short_descr
        : something;                    break;
    case 'p':
      i = something;
      if ( obj2 != NULL && can_see_obj( mob, obj2 ) )
        {
          one_argument( obj2->name, fname );
          i = fname;
        }                         break;
    case 'P':
      i = (obj2 != NULL && can_see_obj( mob, obj2 ))
        ? obj2->short_descr
        : something;                    break;
        }
 
      ++str;
      while ( ( *point = *i ) != '\0' )
    ++point, ++i;
 
    }
  *point = '\0';
 
  return;
}    

/*
 * ------------------------------------------------------------------------
 *  PROGRAM_FLOW
 *  This is the program driver. It parses the mob program code lines
 *  and passes "executable" commands to interpret()
 *  Lines beginning with 'mob' are passed to mob_interpret() to handle
 *  special mob commands (in mob_cmds.c)
 *-------------------------------------------------------------------------
 */

#define MAX_NESTED_LEVEL 12 /* Maximum nested if-else-endif's (stack size) */
#define BEGIN_BLOCK       0 /* Flag: Begin of if-else-endif block */
#define IN_BLOCK         -1 /* Flag: Executable statements */
#define END_BLOCK        -2 /* Flag: End of if-else-endif block */
#define MAX_CALL_LEVEL    5 /* Maximum nested calls */

void program_flow(  sh_int pvnum,  /* For diagnostic purposes */
                    char *source,  /* the actual MOBprog code */
                    CHAR_DATA *mob, CHAR_DATA *ch,
                    const void *arg1, const void *arg2 )
{
  CHAR_DATA *rch = NULL, *ch_save = NULL, *ch_next = NULL;
  char *code, *line, *loopptr = NULL;
  char buf[MAX_STRING_LENGTH];
  char control[MAX_INPUT_LENGTH], data[MAX_STRING_LENGTH];

  static int call_level; /* Keep track of nested "mpcall"s */
  static int looplevel;  /* Keep track of nested loops     */

  int level, eval, check;
  int state[MAX_NESTED_LEVEL], /* Block state (BEGIN,IN,END) */
      cond[MAX_NESTED_LEVEL];  /* Boolean value based on the last if-check */

  sh_int mvnum = mob->pIndexData->vnum;

  if( ++call_level > MAX_CALL_LEVEL )
  {
    bugf( "MOBprogs: MAX_CALL_LEVEL exceeded, vnum %d", mob->pIndexData->vnum );
      /*      return; */
    call_level--;
  }

  /*
   * Reset "stack"
   */
  for ( level = 0; level < MAX_NESTED_LEVEL; level++ )
  {
    state[level] = IN_BLOCK;
    cond[level]  = TRUE;
  }
  level = 0;

  code = source;
  /*
   * Parse the MOBprog code
   */
  while ( *code )
  {
    bool first_arg = TRUE;
    char *b = buf, *c = control, *d = data;
      /*
       * Get a command line. We sneakily get both the control word
       * (if/and/or) and the rest of the line in one pass.
       */
    while( isspace( *code ) && *code ) code++;
    while ( *code )
    {
      if ( *code == '\n' || *code == '\r' )
        break;
      else if ( isspace(*code) )
      {
        if ( first_arg )
          first_arg = FALSE;
        else
          *d++ = *code;
      }
      else
      {
        if ( first_arg )
          *c++ = *code;
        else
          *d++ = *code;
      }
      *b++ = *code++;
    }
    *b = *c = *d = '\0';

    if ( buf[0] == '\0' )
      break;
    if ( buf[0] == '*' ) /* Comment */
      continue;

    line = data;
      /* 
       * Match control words
       */
    if ( !str_cmp( control, "wiznet" ) )
    {
      mprintf(sizeof(buf),buf, "{WMProg {w%d: {x%s{x", pvnum, data);
      wiznet(buf, NULL, NULL, WIZ_MOBPROGS, 0, 0) ;
      continue;
    }

    if ( !str_cmp( control, "if" ) )
    {
      if ( state[level] == BEGIN_BLOCK )
      {
        mprintf(sizeof(buf),buf, 
                "Mobprog: misplaced if statement, mob %d prog %d",
                                                    mvnum, pvnum );
        bug( buf, 0 );
        return;
      }
      state[level] = BEGIN_BLOCK;
      if ( ++level >= MAX_NESTED_LEVEL )
      {
        mprintf(sizeof(buf),buf, 
                "Mobprog: Max nested level exceeded, mob %d prog %d",
                                                        mvnum, pvnum );
        bug( buf, 0 );
        return;
      }
      if ( level && cond[level-1] == FALSE ) 
      {
        cond[level] = FALSE;
        continue;
      }
      line = one_argument( line, control );
      if ( ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
      {
        cond[level] = cmd_eval( pvnum, line, check, mob, ch, arg1, arg2, rch );
      }
      else
      {
        mprintf(sizeof(buf),buf,
          "Mobprog: invalid if_check (if), mob %d, prog %d, command %s", mvnum, pvnum, line );
        bug( buf, 0 );
        return;
      }
      state[level] = END_BLOCK;
      }
      else if ( !str_cmp( control, "or" ) )
      {
      if ( !level || state[level-1] != BEGIN_BLOCK )
      {
        mprintf(sizeof(buf),buf, "Mobprog: or without if, mob %d prog %d",
               mvnum, pvnum );
        bug( buf, 0 );
        return;
      }
      if ( level && cond[level-1] == FALSE ) continue;
      line = one_argument( line, control );
      if ( ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
      {
        eval = cmd_eval( pvnum, line, check, mob, ch, arg1, arg2, rch );
      }
      else
      {
        mprintf(sizeof(buf),buf, 
                "Mobprog: invalid if_check (or), mob %d prog %d",
                                                    mvnum, pvnum );
        bug( buf, 0 );
        return;
      }
      cond[level] = (eval == TRUE) ? TRUE : cond[level];
    }
    else if ( !str_cmp( control, "and" ) )
    {
      if ( !level || state[level-1] != BEGIN_BLOCK )
      {
        mprintf(sizeof(buf),buf, "Mobprog: and without if, mob %d prog %d",
               mvnum, pvnum );
        bug( buf, 0 );
        return;
      }
      if ( level && cond[level-1] == FALSE ) continue;
      line = one_argument( line, control );
      if ( ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
      {
        eval = cmd_eval( pvnum, line, check, mob, ch, arg1, arg2, rch );
      }
      else
      {
        mprintf(sizeof(buf),buf, 
                "Mobprog: invalid if_check (and), mob %d prog %d",
                                                    mvnum, pvnum );
        bug( buf, 0 );
        return;
      }
      cond[level] = (cond[level] == TRUE) && (eval == TRUE) ? TRUE : FALSE;
    }
    else if ( !str_cmp( control, "endif" ) )
    {
      if ( !level || state[level-1] != BEGIN_BLOCK )
      {
        mprintf(sizeof(buf),buf, "Mobprog: endif without if, mob %d prog %d",
               mvnum, pvnum );
        bug( buf, 0 );
        return;
      }
      cond[level] = TRUE;
      state[level] = IN_BLOCK;
      state[--level] = END_BLOCK;
    }
    else if ( !str_cmp( control, "else" ) )
    {
      if ( !level || state[level-1] != BEGIN_BLOCK )
      {
        mprintf(sizeof(buf),buf, "Mobprog: else without if, mob %d prog %d",
               mvnum, pvnum );
        bug( buf, 0 );
        return;
      }
      if ( level && cond[level-1] == FALSE ) continue;
      state[level] = IN_BLOCK;
      cond[level] = (cond[level] == TRUE) ? FALSE : TRUE;
    }
    else if (  cond[level] == TRUE
    &&      ( !str_cmp( control, "break" ) || !str_cmp( control, "end" ) ) )
    {
      call_level--;
      return;
    }
    else if ( cond[level] == TRUE && !str_cmp( control, "do" ) )
    {
      if ( state[level] == BEGIN_BLOCK || loopptr )
      {
        mprintf(sizeof(buf),buf,
                "Mobprog: misplaced DO statement, mob %d prog %d",
                                                    mvnum, pvnum );
        call_level--;
        return;
      }
      ch_save = ch;
      ch      = mob->in_room->people;
      ch_next = ch->next_in_room;
      if ( ch == mob )
      {
        ch  = ch->next_in_room;
        ch_next = ch_next == NULL ? NULL : ch_next->next_in_room;
      }
      loopptr = code;
      looplevel   = level;
    }
    else if ( cond[level] == TRUE && !str_cmp( control, "loop" ) )
    {
      if ( looplevel != level || state[level] == BEGIN_BLOCK
      ||   loopptr == NULL )
      {
        mprintf(sizeof(buf),buf,
                "Mobprog: misplaced LOOP statement, mob %d prog %d",
                                                    mvnum, pvnum );
        call_level--;
        return;
      }
      if ( ch_next == mob )
        ch_next = mob->next_in_room;
      if ( ch_next == NULL )
      {
        ch = ch_save;
        loopptr = NULL;
      }
      else
      {
        ch = ch_next;
        ch_next = ch_next->next_in_room;
        code = loopptr;
      }
    }

    else if ( ( !level || cond[level] == TRUE ) && buf[0] )
    {
      state[level] = IN_BLOCK;
      expand_arg( data, buf, mob, ch, arg1, arg2, rch );
      if ( !str_cmp( control, "mob" ) )
      {
          /* 
           * Found a mob restricted command, pass it to mob interpreter
           */
        line = one_argument( data, control );
        mob_interpret( mob, line );
      }
      else
      {
          /* 
           * Found a normal mud command, pass it to interpreter
           */
        interpret( mob, data );
      }
    }
  }
  call_level--;
}

/* 
 * ---------------------------------------------------------------------
 * Trigger handlers. These are called from various parts of the code
 * when an event is triggered.
 * ---------------------------------------------------------------------
 */

/*
 * A general purpose string trigger. Matches argument to a string trigger
 * phrase.
 */
void mp_act_trigger( 
    char *argument, CHAR_DATA *mob, CHAR_DATA *ch, 
    const void *arg1, const void *arg2, int type )
{
  MPROG_LIST *prg;

  for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
    {
      if ( prg->trig_type == type 
       /*       &&  (!strcmp( argument, prg->trig_phrase )))*/
       &&  strstr( argument, prg->trig_phrase ) )
        {
      program_flow( prg->vnum, prg->code->code, mob, ch, arg1, arg2 );
      break;
    }
    }
  return;
}

/*
 * A general purpose percentage trigger. Checks if a random percentage
 * number is less than trigger phrase
 */
bool mp_percent_trigger( CHAR_DATA *mob, CHAR_DATA *ch, 
            const void *arg1, const void *arg2, int type )
{
  MPROG_LIST *prg;

  for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
  {
    if ( prg->trig_type == type
    &&   number_percent() <= atoi( prg->trig_phrase ) )
    {
      program_flow( prg->vnum, prg->code->code, mob, ch, arg1, arg2 );
      return ( TRUE );
    }
  }
  return ( FALSE );
}

bool ap_percent_trigger( void *vo, CHAR_DATA *ch,
            const void *arg1, const void *arg2, int type, int prog_type )
{
  CHAR_DATA         *mob  = NULL;
  bool              prog_flag;

  if ( prog_type == MOB_PROG )
  {
    mob = (CHAR_DATA *)vo;
  }
  else 
  {
    if ( ( mob = assign_prog_master( vo, ch, prog_type ) ) == NULL )
      return ( FALSE );
  }

  prog_flag = mp_percent_trigger( mob, ch, arg1, arg2, type );

  if ( prog_type != MOB_PROG )
    release_prog_master( vo, prog_type );

  return ( prog_flag );
}

void mp_bribe_trigger( CHAR_DATA *mob, CHAR_DATA *ch, int amount )
{
  MPROG_LIST *prg;

  /*
   * Original MERC 2.2 MOBprograms used to create a money object
   * and give it to the mobile. WFT was that? Funcs in act_obj()
   * handle it just fine.
   */
  for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
    {
      if ( prg->trig_type == TRIG_BRIBE
       &&   amount >= atoi( prg->trig_phrase ) )
    {
      program_flow( prg->vnum, prg->code->code, mob, ch, NULL, NULL );
      break;
    }
    }
  return;
}

bool mp_exit_trigger( CHAR_DATA *ch, int dir )
{
  CHAR_DATA *mob;
  MPROG_LIST   *prg;

  for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
  {    
    if ( IS_NPC( mob )
    && ( HAS_TRIGGER(mob, TRIG_EXIT) || HAS_TRIGGER(mob, TRIG_EXALL) ) )
    {
      for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
      {
          /*
           * Exit trigger works only if the mobile is not busy
           * (fighting etc.). If you want to be sure all players
           * are caught, use ExAll trigger
           */
        if ( prg->trig_type == TRIG_EXIT
        &&   dir == atoi( prg->trig_phrase )
        && ( mob->pIndexData->default_pos == POS_DEAD
        ||   mob->position == mob->pIndexData->default_pos )
        &&  can_see( mob, ch ) )
        {
          program_flow( prg->vnum, prg->code->code, mob, ch, NULL, NULL );
          return TRUE;
        }
        else if ( prg->trig_type == TRIG_EXALL
        &&   dir == atoi( prg->trig_phrase ) )
        {
          program_flow( prg->vnum, prg->code->code, mob, ch, NULL, NULL );
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

/* mobprog that runs when a mob loads/pops */
bool mp_onload_trigger( CHAR_DATA *ch )
{
  MPROG_LIST   *prg;

  if (!IS_NPC(ch))
    return FALSE;

  if (!HAS_TRIGGER(ch, TRIG_ONLOAD))
    return FALSE;

  for ( prg = ch->pIndexData->mprogs; prg; prg = prg->next )
  {
    if ( prg->trig_type == TRIG_ONLOAD)
    {
      program_flow( prg->vnum, prg->code->code, ch, ch, NULL, NULL );
      return TRUE;
    }
  }

  return FALSE;
}

void mp_give_trigger( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj )
{

  char        buf[MAX_INPUT_LENGTH], *p;
  MPROG_LIST  *prg;

  for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
    if ( prg->trig_type == TRIG_GIVE )
      {
    p = prg->trig_phrase;
    /*
     * Vnum argument
     */
    if ( is_number( p ) )
      {
        if ( obj->pIndexData->vnum == atoi(p) )
          {
        program_flow(prg->vnum, prg->code->code, mob, ch, (void *) obj, NULL);
        return;
          }
      }
    /*
     * Object name argument, e.g. 'sword'
     */
    else
      {
        while( *p )
          {
        p = one_argument( p, buf );

        if ( is_name( buf, obj->name )
             ||   !str_cmp( "all", buf ) )
          {
            program_flow(prg->vnum, prg->code->code, 
                         mob, ch, (void *)obj, NULL);
            return;
          }
          }
      }
      }
}

void mp_dig_trigger( CHAR_DATA *ch, char *argument )
{
  MPROG_LIST        *prg;
  CHAR_DATA         *mob;
  ROOM_INDEX_DATA   *room;

  room = ch->in_room;

  for ( prg = room->mprogs; prg; prg = prg->next )
  {
    if (  prg->trig_type == TRIG_DIG
    && ( !str_prefix( argument, prg->trig_phrase ) ) )
    {
      if ( assign_prog_master( room, ch, ROOM_PROG ) )
        mob = prog_master;
      else
        return;
      program_flow( prg->vnum, prg->code->code, mob, ch, NULL, NULL );
      release_prog_master( room, ROOM_PROG );
      break;
    }
  }
}

void mp_pull_trigger( CHAR_DATA *ch, char *argument )
{
  MPROG_LIST        *prg;
  CHAR_DATA         *mob;
  ROOM_INDEX_DATA   *room;

  room = ch->in_room;

  for ( prg = room->mprogs; prg; prg = prg->next )
  {
    if (  prg->trig_type == TRIG_PULL
    && ( !str_prefix( argument, prg->trig_phrase ) ) )
    {
      if ( assign_prog_master( room, ch, ROOM_PROG ) )
        mob = prog_master;
      else
        return;
      program_flow( prg->vnum, prg->code->code, mob, ch, NULL, NULL );
      release_prog_master( room, ROOM_PROG );
      break;
    }
  }
}

void mp_press_trigger( CHAR_DATA *ch, char *argument )
{
  MPROG_LIST        *prg;
  CHAR_DATA         *mob;
  ROOM_INDEX_DATA   *room;

  room = ch->in_room;

  for ( prg = room->mprogs; prg; prg = prg->next )
  {
    if (  prg->trig_type == TRIG_PRESS
    && ( !str_prefix( argument, prg->trig_phrase ) ) )
    {
      if ( assign_prog_master( room, ch, ROOM_PROG ) )
        mob = prog_master;
      else
        return;
      program_flow( prg->vnum, prg->code->code, mob, ch, NULL, NULL );
      release_prog_master( room, ROOM_PROG );
      break;
    }
  }
}

void mp_turn_trigger( CHAR_DATA *ch, char *argument )
{
  MPROG_LIST        *prg;
  CHAR_DATA         *mob;
  ROOM_INDEX_DATA   *room;

  room = ch->in_room;

  for ( prg = room->mprogs; prg; prg = prg->next )
  {
    if (  prg->trig_type == TRIG_TURN
    && ( !str_prefix( argument, prg->trig_phrase ) ) )
    {
      if ( assign_prog_master( room, ch, ROOM_PROG ) )
        mob = prog_master;
      else
        return;
      program_flow( prg->vnum, prg->code->code, mob, ch, NULL, NULL );
      release_prog_master( room, ROOM_PROG );
      break;
    }
  }
}

void mp_touch_trigger( CHAR_DATA *ch, char *argument )
{
  MPROG_LIST        *prg;
  CHAR_DATA         *mob;
  ROOM_INDEX_DATA   *room;

  room = ch->in_room;

  for ( prg = room->mprogs; prg; prg = prg->next )
  {
    if (  prg->trig_type == TRIG_TOUCH 
    && ( !str_prefix(argument, prg->trig_phrase ) ) )
    {
      if ( assign_prog_master( room, ch, ROOM_PROG ) )
        mob = prog_master;
      else
        return;
      program_flow( prg->vnum, prg->code->code, mob, ch, NULL, NULL );
      release_prog_master( room, ROOM_PROG );
      break;
    }
  }
}
void ap_greet_trigger( CHAR_DATA *ch );
void mp_greet_trigger( CHAR_DATA *ch, char *argument )
{
  MPROG_LIST        *prg;
  CHAR_DATA         *mob;
  ROOM_INDEX_DATA   *room;

  room = ch->in_room;

  if ( IS_SET( room->mprog_flags, TRIG_GREET )
  ||   IS_SET( room->mprog_flags, TRIG_GRALL ) )
  {
    for ( prg = room->mprogs; prg; prg = prg->next )
    {
      if (  prg->trig_type == TRIG_GREET || prg->trig_type == TRIG_GRALL )
      {
        if ( !is_number( prg->trig_phrase ) )
        {
          if ( !str_prefix( prg->trig_phrase, argument ) ) 
          {
            if ( assign_prog_master( room, ch, ROOM_PROG ) )
              mob = prog_master;
            else
              break;
            program_flow( prg->vnum, prg->code->code, mob, ch, NULL, NULL );
            release_prog_master( room, ROOM_PROG );
            break;
          }
        }
      }
    }
  }
  for ( mob = room->people; mob; mob = mob->next_in_room )
  {
    if (  IS_NPC( mob )
    && (  HAS_TRIGGER( mob, TRIG_GREET )
    ||    HAS_TRIGGER( mob, TRIG_GRALL ) )
    && ( !IS_AFFECTED( mob, AFF_CHARM ) ) )
    {
      /*
       * Greet trigger works only if the mobile is not busy
       * (fighting etc.). If you want to catch all players, use
       * GrAll trigger
       */
      for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
      {
        if (  prg->trig_type == TRIG_GREET
        && (  mob->pIndexData->default_pos == POS_DEAD
        ||    mob->position == mob->pIndexData->default_pos )
        &&   can_see( mob, ch ) )
        {
          if ( !is_number( prg->trig_phrase ) )
          {
            if ( !str_prefix( prg->trig_phrase, argument ) )
              program_flow( prg->vnum, prg->code->code, mob, ch, NULL, NULL );
            break;
          }
        }
        if (  prg->trig_type == TRIG_GRALL
        && (  mob->pIndexData->default_pos == POS_DEAD
        ||    mob->position == mob->pIndexData->default_pos )
        &&   can_see( mob, ch ) )
        {
          if ( !is_number( prg->trig_phrase ) )
          {
            if ( !str_prefix( prg->trig_phrase, argument ) )
              program_flow( prg->vnum, prg->code->code, mob, ch, NULL, NULL );
            break;
          }
        }
      }
    }
  }
  ap_greet_trigger( ch );
}

void ap_greet_trigger( CHAR_DATA *ch )
{
  CHAR_DATA *mob;

  if ( IS_SET( ch->in_room->mprog_flags, TRIG_GREET ) )
    ap_percent_trigger( ch->in_room, ch, NULL, NULL, TRIG_GREET, ROOM_PROG );
  else if ( IS_SET( ch->in_room->mprog_flags, TRIG_GRALL ) )
    ap_percent_trigger( ch->in_room, ch, NULL, NULL, TRIG_GRALL, ROOM_PROG );


  for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
  {    
    if ( IS_NPC( mob )
    && (  HAS_TRIGGER( mob, TRIG_GREET )
    ||    HAS_TRIGGER( mob, TRIG_GRALL ) )
    && ( !IS_AFFECTED( mob, AFF_CHARM ) ) )
    {
      /*
       * Greet trigger works only if the mobile is not busy
       * (fighting etc.). If you want to catch all players, use
       * GrAll trigger
       */
      if ( HAS_TRIGGER( mob, TRIG_GREET )
      && (  mob->pIndexData->default_pos == POS_DEAD
      ||    mob->position == mob->pIndexData->default_pos )
      &&   can_see( mob, ch ) )
        ap_percent_trigger( mob, ch, NULL, NULL, TRIG_GREET, MOB_PROG );
      else if ( HAS_TRIGGER( mob, TRIG_GRALL ) )
        ap_percent_trigger( mob, ch, NULL, NULL, TRIG_GRALL, MOB_PROG );
    }
  }
  return;
}

void ap_gronce_trigger( CHAR_DATA *ch )
{
  CHAR_DATA *mob;

  if ( IS_SET( ch->in_room->mprog_flags, TRIG_GRONCE ) )
    ap_percent_trigger( ch->in_room, ch, NULL, NULL, TRIG_GRONCE, ROOM_PROG );

  for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
  {
    if ( IS_NPC( mob )
    &&   HAS_TRIGGER( mob, TRIG_GRONCE )
    &&  !IS_AFFECTED( mob, AFF_CHARM ) )
      ap_percent_trigger( mob, ch, NULL, NULL, TRIG_GRONCE, MOB_PROG );
  }
  return;
}

void mp_hprct_trigger( CHAR_DATA *mob, CHAR_DATA *ch )
{
  MPROG_LIST *prg;

  for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
    if ( ( prg->trig_type == TRIG_HPCNT )
    && ( ( 100 * mob->hit / mob->max_hit ) < atoi( prg->trig_phrase ) ) )
    {
      program_flow( prg->vnum, prg->code->code, mob, ch, NULL, NULL );
      break;
    }
}

void mp_get_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
  char       buf[MAX_INPUT_LENGTH], *p;
  CHAR_DATA  *mob;
  MPROG_LIST *prg;

  for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
  { // Loop through all mobs/chars in room
    if (IS_NPC(mob))
    { // Only mobs have MProgs
      for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
      { // Loop through each mob's progs
        if ( prg->trig_type == TRIG_GET )
        {
          p = prg->trig_phrase; // get trigger phrase

          if ( is_number( p ) )
          { // phrase is a number
            if ( obj->pIndexData->vnum == atoi(p) )
            { // does the obj vnum matches the phrase vnum?
              program_flow(prg->vnum, prg->code->code, mob, ch, (void *) obj, NULL);
              return;
            }
          } // end phrase was a number
          else
          { // phrase is a word
            while( *p )
            { // Loop through arguments
              p = one_argument( p, buf );

              if ( is_name( buf, obj->name )
              ||  !str_cmp( "all", buf ) )
              { // does the object name match the phrase name?
                program_flow(prg->vnum, prg->code->code, mob, ch, (void *)obj, NULL);
                return;
              }

            } // end argument loop
          } // end prog phrase was a word
        } // end prog type check (TRIG_GET)
      } // end prog loop
    } // end mobs only
  } // end in_room loop
}

void mp_social_trigger( CHAR_DATA *mob, CHAR_DATA *ch, char *soccmd )
{
  MPROG_LIST        *prg;

  for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
  {
    if ( prg->trig_type == TRIG_SOCIAL
    &&   can_see( mob, ch )
    &&  !str_cmp(soccmd,prg->trig_phrase) )
    {
      program_flow( prg->vnum, prg->code->code, mob, ch, NULL, NULL );
      break;
    }
  }
}

bool mp_get_check( CHAR_DATA *ch, OBJ_DATA *obj )
{
  char       buf[MAX_INPUT_LENGTH], *p;
  CHAR_DATA  *mob;
  MPROG_LIST *prg;

  for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
  { // Loop through all mobs/chars in room
    if (IS_NPC(mob))
    { // Only mobs have MProgs
      for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
      { // Loop through each mob's progs
        if ( prg->trig_type == TRIG_GET )
        {
          p = prg->trig_phrase; // get trigger phrase

          if ( is_number( p ) )
          { // phrase is a number
            if ( obj->pIndexData->vnum == atoi(p) )
            { // does the obj vnum matches the phrase vnum?
              return TRUE;
            }
          } // end phrase was a number
          else
          { // phrase is a word
            while( *p )
            { // Loop through arguments
              p = one_argument( p, buf );

              if ( is_name( buf, obj->name )
              ||  !str_cmp( "all", buf ) )
              { // does the object name match the phrase name?
                return TRUE;
              }

            } // end argument loop
          } // end prog phrase was a word
        } // end prog type check (TRIG_GET)
      } // end prog loop
    } // end mobs only
  } // end in_room loop
  return FALSE;
}

#ifdef TEST_MUD_4
bool assign_prog_master( CHAR_DATA *ch, void *actor, bool type )
{
  char buf[MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *pRoom;
  OBJ_DATA *pObj;

  if ( ( pRoom = ch->in_room ) == NULL )
    {
      return FALSE;
    }

/*  if ( prog_master->in_room != NULL )
    {
      mprintf(sizeof(buf),buf, "MOBprograms: prog_master already in use!\n\r"
           "Char: %s Room: %d Actor: %s%d\n\r",
           ch->name, ch->in_room == NULL ? -1 : ch->in_room->vnum,
  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
      for ( pObjIndex = obj_index_hash[iHash]; pObjIndex != NULL;
        pObjIndex = pObjIndex->next )
    {
           !type ? "(obj) " : "(room) ",
           !type ? ((OBJ_DATA*)actor)->pIndexData->vnum :
           ((ROOM_INDEX_DATA*)actor)->vnum );
      bug( buf, 0 );
      exit( 1 );
    }
    }
  char_to_room( prog_master, pRoom );
  // a couple hours of debugger results in proxy mobs who stand :)
  // I question running these calls through the player interpret
  // function though.
  prog_master->position = POS_STANDING;
  if ( !type )
    {
      pObj = (OBJ_DATA *)actor;
      prog_master->name = str_dup(pObj->name);
      prog_master->short_descr = str_dup(pObj->short_descr);
      prog_master->level = pObj->level;
    }
  else
    {
      prog_master->name = str_dup("room");
      prog_master->short_descr = str_dup("the room");
      prog_master->level = 1;
    }
 */ return TRUE;
}


/* This MUST be called after a call to obj or room script! */
void release_prog_master( void )
{
  if ( prog_master->in_room != NULL )
    {
      free_string( prog_master->name );
      free_string( prog_master->short_descr );

      char_from_room( prog_master );
    }
}

bool rp_exit_trigger( CHAR_DATA *ch, int dir )
{
  MPROG_LIST    *pProg;
  char    buf[MAX_STRING_LENGTH];
    
  if ( ch->invis_level >= LEVEL_IMMORTAL )
    return FALSE;
    
  for ( pProg = ch->in_room->mprogs; pProg != NULL; pProg = pProg->next )
    {
      if ( ( pProg->trig_type == TRIG_EXIT || pProg->trig_type == TRIG_EXALL )
       &&   dir == atoi( pProg->trig_phrase )
       &&   assign_prog_master( ch, ch->in_room, 1 ) )
    {
      program_flow( pProg->vnum, pProg->prog, prog_master, ch, NULL, NULL );
      release_prog_master();
      return TRUE;
    }
    }
  return FALSE;
}
#endif


