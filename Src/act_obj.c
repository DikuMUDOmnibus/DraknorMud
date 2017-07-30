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
extern char *target_name;

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"
#include "magic.h"
#include <dirent.h>
#include "interp.h"
#include "special.h"
#include "tables.h" //Added 2-13-07

/*
 * Local functions.
 */
#define CD CHAR_DATA
#define OD OBJ_DATA

bool  remove_obj      args( ( CD *ch, int iWear, bool fReplace ) );
void  wear_obj      args( ( CD *ch, OD *obj, bool fReplace ) );
CD *  find_keeper      args( ( CD *ch ) );
//int      get_cost      args( ( CD *keeper, OD *obj, bool fBuy ) );
void   obj_to_keeper  args( ( OD *obj, CD *ch ) );
//OD *  get_obj_keeper  args( ( CD *ch, CD *keeper, char *argument ) );
bool    check_pre_player_names( char *tempstr );

#undef OD
#undef CD
char *formatted_list( OBJ_DATA *list, CHAR_DATA *ch,
                      bool fShort,
                      bool fShowNothing,
                      char *arg1,
                      CHAR_DATA *victim,
                      CHAR_DATA *fch,
                      bool (*run_func)(CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *victim),
                      bool (*run_func2)(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container),
                      OBJ_DATA *container,
                      char *fail_message);

static int get_temp_weight = 0;
static int get_temp_number = 0;
bool spell_imprint( int sn, int level, CHAR_DATA *ch, void *vo, int target);


int split_string( char *string, char *token, char *str1, char *str2 )
{
  /*
   * divide a char string based on the token delimeter
   * check if token is first char and if so, add a leading space
   */

  char   temp_str[MSL];
  char   *sp;

  if ( strlen( string ) > MSL )
  {
    bug( "split_string: Strlen > MSL.", 0 );
    *str1 = *str2 = '\0';
    return FALSE;
  }

  strcpy( temp_str, string );

  if ( ( sp = strchr( temp_str, *token ) ) )
  {
    *sp = '\0';
    strcpy( str1, string );
    strcpy( str2, sp + 1 );
    return TRUE;
  }
  return FALSE;
}

/* RT part of the corpse looting code */
bool can_loot( CHAR_DATA *ch, OBJ_DATA *obj )
{
  CHAR_DATA *owner, *wch;

  if ( IS_IMMORTAL( ch ) )
    return TRUE;

  if ( obj->owner == NULL )
    return TRUE;

  if ( IS_NPC( ch )
       &&   obj->item_type == ITEM_CORPSE_PC )
    return FALSE;

  owner = NULL;
  /* use player_list to speed things up */
  for ( wch = player_list; wch; wch = wch->next_player )
    if ( is_exact_name( wch->name, obj->owner ) )
      owner = wch;

  if ( owner == NULL )
    return TRUE;

  if ( !str_cmp( ch->name, owner->name ) )
    return TRUE;

  if ( !IS_NPC( owner ) && IS_SET( owner->act, PLR_CANLOOT ) )
    return TRUE;

  if ( IS_AFFECTED( owner, AFF_CHARM ) )
    return FALSE;

//  if ( is_same_group( ch, owner ) )
//    return TRUE;

  return FALSE;
}

void get_mult_obj( CHAR_DATA *ch, char *arg2, OBJ_DATA *container, int count )
{
  CHAR_DATA *gch;
  OBJ_DATA  *obj = NULL;
  OBJ_DATA  *progobj = NULL;
  int        progfound = FALSE;
  int    members;
  char   buffer[100];
  char   buf[MAX_STRING_LENGTH];
  char   error_buf[MSL];
  char **pObjStrShow;
  bool  *pObjWhere;
  int   *pObjNumShow;
  bool   bObjWhere  = 0;
  bool   fCombine   = 0;
  int    nShow;
  int    iShow;
  int    num_found  = 0;
  int    max_found  = 50;
  int    i          = 0;
  bool own_container = FALSE;
  //int locker;

  pObjStrShow   = alloc_mem( max_found * sizeof(char *) );
  pObjWhere      = alloc_mem( max_found * sizeof(bool)  );
  pObjNumShow   = alloc_mem( max_found * sizeof(int)  );
  nShow  = 0;
  strcpy( error_buf, "" );
  for ( i = 0; i < count; i++ )
  {
    if ( container )
    {
      if (container->carried_by == ch)
        own_container = TRUE;

      if ( container->item_type == ITEM_LOCKER )
      {
        obj = get_obj_list( ch, arg2, ch->pcdata->locker );

        if ( IS_NPC( ch ) )
        {
          mprintf(sizeof(error_buf), error_buf,
                  "MOBs don't have lockers.\n\r" );
          break;
        }

        if ( ch->clan )
        {
          if ( ch->clan->locker != container->pIndexData->vnum )
          {
            send_to_char( "That isn't your locker.\n\r", ch );
            break;
          }
        }
        else
        {
          if ( obj->value[0] )
          {
            send_to_char( "That isn't your locker.\n\r", ch );
            break;
          }
        }

        if ( ch->pcdata->locker_max <= 0 )
        {
          mprintf(sizeof(error_buf), error_buf,
                  "You don't have a locker assigned to you.\n\r");
          break;
        }

        //obj = get_obj_list( ch, arg2, ch->pcdata->locker );
      }
      else
      {
        obj = get_obj_list( ch, arg2, container->contains );
      }
      if ( obj == NULL )
      {
        mprintf(sizeof(error_buf), error_buf,
                "You see nothing like that in %s.", container->short_descr );
        break;
      }
    }
    else
    {
      obj = get_obj_list( ch, arg2, ch->in_room->contents );
      if ( obj == NULL )
      {
        mprintf(sizeof(error_buf),error_buf, "You see no %s here.", arg2 );
        break;
      }
    }
    if ( !CAN_WEAR( obj, ITEM_TAKE ) )
    {
      mprintf(sizeof(error_buf), error_buf, "You can't take that.\n\r" );
      break;
    }

    if ( ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
         && !own_container ) // You can get items from your own container when at max items
    {
      mprintf(sizeof(error_buf), error_buf,
              "You can't carry that many items." );
      break;
    }

    if ( obj->pIndexData->vnum == OBJ_VNUM_QUEST )
    {
      if ( !is_exact_name( ch->name, obj->owner ) && !IS_IMMORTAL( ch ) )
      {
        mprintf(sizeof(error_buf), error_buf,
                "You cannot take another person's quest item.\n\r" );
        break;
      }
    }

    if ( ( !obj->in_obj || obj->in_obj->carried_by != ch )
         &&   ( get_carry_weight( ch ) + ( ( ( obj->item_type == ITEM_MONEY )
                                             &&   ( has_money_pouch( ch ) ) ) ?
                                           get_obj_weight( obj ) / MONEY_POUCH_WEIGHT_MULT :
                                           get_obj_weight( obj ) ) > can_carry_w( ch ) ) )

    {
      mprintf(sizeof(error_buf),error_buf,
              "You can't carry that much weight." );
      break;
    }

    if ( !can_loot( ch, obj ) )
    {
      mprintf(sizeof(error_buf), error_buf, "That does not belong to you." );
      break;
    }

    if ( obj->in_room )
    {
      for ( gch = obj->in_room->people; gch; gch = gch->next_in_room )
        if ( gch->on == obj )
        {
          act( "$N appears to be using $p.", ch, obj, gch, TO_CHAR );
          break;
        }
      if ( gch )
        break;
    }


    if ( container )
    {
      if ( container->pIndexData->vnum == OBJ_VNUM_PIT
           &&   get_trust( ch ) < obj->level )
      {
        mprintf(sizeof(error_buf),error_buf,
                "You are not powerful enough to use it.\n\r", ch );
        break;
      }
      /*** handle get from locker ***/
      if ( container->item_type == ITEM_LOCKER )
      {
        num_found++;
        mprintf(sizeof(buf), buf, "%s", obj->short_descr );
        obj_from_locker( obj, ch );
      }
      else
      {
        if ( container->pIndexData->vnum == OBJ_VNUM_PIT
             &&  !CAN_WEAR( container, ITEM_TAKE )
             &&  !IS_OBJ_STAT( obj, ITEM_HAD_TIMER ) )
          obj->timer = 0;
        mprintf(sizeof(buf), buf, "%s", obj->short_descr );
        REMOVE_BIT( obj->extra_flags, ITEM_HAD_TIMER );
        num_found++;
        obj_from_obj( obj );
        if (mp_get_check( ch, obj ))
        {
          progobj = obj;
          progfound = TRUE;
        }

      }
    }
    else
    {
      mprintf(sizeof(buf), buf,"%s", obj->short_descr );
      num_found++;
      obj_from_room( obj );
      if (mp_get_check( ch, obj ))
      {
        progobj = obj;
        progfound = TRUE;
      }
    }

    if ( obj->item_type == ITEM_MONEY )
    {
      ch->silver += obj->value[0];
      ch->gold += obj->value[1];
      if ( IS_SET( ch->act, PLR_AUTOSPLIT ) )
      {
        /* AUTOSPLIT code */
        if (container
            && (container->item_type != ITEM_CORPSE_PC) )
        {
          members = 0;
          for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
          {
            if ( !IS_AFFECTED( gch, AFF_CHARM ) && is_same_group( gch, ch ) )
              members++;
          }

          if ( members > 1 && ( obj->value[0] > 1 || obj->value[1] ) )
          {
            mprintf(sizeof(buffer), buffer, "%d %d", obj->value[0], obj->value[1] );
            do_function( ch, &do_split, buffer );
          }
        }
      }
      extract_obj( obj );
    }
    else
      obj_to_char( obj, ch );
    /*
     * Look for duplicates, case sensitive.
     * Matches tend to be near end so run loop backwords.
     */
    fCombine = FALSE;
    for ( iShow = nShow - 1; iShow >= 0; iShow-- )
    {
      if ( pObjWhere[iShow] == bObjWhere
           &&  !strcmp( pObjStrShow[iShow], buf ) )
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
  }
  /*
   * Output the formatted list.
   */
  if ( !num_found && ( error_buf[0] ) )
  {
    printf_to_char( ch, "{w%s\n\r", error_buf );
  }
  else
  {
    if ( container )
    {
      act( "You get the following item(s) from $P{x:",
           ch, obj, container, TO_CHAR );
      act( "$n gets the following item(s) from $P{x:",
           ch, obj, container, TO_ROOM );
    }
    else
    {
      act( "You get the following item(s):", ch, NULL, NULL, TO_CHAR );
      act( "$n gets the following item(s):", ch, NULL, NULL, TO_ROOM );
    }

    for ( iShow = 0; iShow < nShow; iShow++ )
    {
      if ( pObjStrShow[iShow][0] == '\0' )
      {
        free_string( pObjStrShow[iShow] );
        continue;
      }

      printf_to_char( ch, "(%d) %s\n\r",
                      pObjNumShow[iShow], pObjStrShow[iShow] );
      /* loop room */
      mprintf(sizeof(buf), buf,"(%d) %s",
              pObjNumShow[iShow], pObjStrShow[iShow] );
      act( buf, ch, NULL, NULL, TO_ROOM );
      free_string( pObjStrShow[iShow] );
    }
    if ( error_buf[0] )
      printf_to_char( ch, "{w%s\n\r", error_buf );
  }
  if (progfound)
    mp_get_trigger( ch, progobj );

  free_mem( pObjStrShow );
  free_mem( pObjWhere   );
  free_mem( pObjNumShow );
}

void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
  CHAR_DATA *gch;
  int members;
  char buffer[100];

  if ( !CAN_WEAR( obj, ITEM_TAKE ) )
  {
    send_to_char( "You can't take that.\n\r", ch );
    return;
  }

  if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch )
       && ( obj->carried_by != ch ) ) // You can get items from your own container when at max items
  {
    act( "$d: you can't carry that many items.",
         ch, NULL, obj->name, TO_CHAR );
    return;
  }

  if ( obj->pIndexData->vnum == OBJ_VNUM_QUEST )
    if (!is_exact_name( ch->name, obj->owner ) && !IS_IMMORTAL( ch ) )
    {
      send_to_char( "You cannot take another person's quest item.\n\r", ch );
      return;
    }

  if ( ( !obj->in_obj || obj->in_obj->carried_by != ch ) &&
       ( get_carry_weight( ch ) + ( ( ( obj->item_type == ITEM_MONEY ) &&
                                      ( has_money_pouch( ch ) ) ) ?
                                    get_obj_weight( obj ) / MONEY_POUCH_WEIGHT_MULT :
                                    get_obj_weight( obj ) ) > can_carry_w( ch ) ) )

  {
    act( "$d: you can't carry that much weight.",
         ch, NULL, obj->name, TO_CHAR );
    return;
  }

  if ( !can_loot( ch, obj ) )
  {
    act( "That does not belong to you.", ch, NULL, NULL, TO_CHAR );
    return;
  }

  if ( obj->in_room )
  {
    for ( gch = obj->in_room->people; gch; gch = gch->next_in_room )
      if ( gch->on == obj )
      {
        act( "$N appears to be using $p.",
             ch, obj, gch, TO_CHAR );
        return;
      }
  }


  if ( container )
  {
    /*** handle get from locker ***/
    if ( container->item_type == ITEM_LOCKER )
    {
      act( "You get {G$p{x from {y$P{x.", ch, obj, container, TO_CHAR );
      act( "$n gets {G$p{x from {y$P{x.", ch, obj, container, TO_ROOM );
      obj_from_locker( obj, ch );
    }
    else
    {
      if ( container->pIndexData->vnum == OBJ_VNUM_PIT &&
           get_trust( ch ) < obj->level )
      {
        send_to_char( "You are not powerful enough to use it.\n\r", ch );
        return;
      }

      if ( container->pIndexData->vnum == OBJ_VNUM_PIT
           &&  !CAN_WEAR( container, ITEM_TAKE )
           &&  !IS_OBJ_STAT( obj, ITEM_HAD_TIMER ) )
        obj->timer = 0;
      act( "You get {G$p{x from {y$P{x.", ch, obj, container, TO_CHAR );
      act( "$n gets {G$p{x from {y$P{x.", ch, obj, container, TO_ROOM );
      REMOVE_BIT( obj->extra_flags, ITEM_HAD_TIMER );
      obj_from_obj( obj );
    }
  }
  else
  {
    act( "You get $p.", ch, obj, container, TO_CHAR );
    act( "$n gets $p.", ch, obj, container, TO_ROOM );
    obj_from_room( obj );
  } // End of locker code...

  if ( obj->item_type == ITEM_MONEY )
  {
    ch->silver += obj->value[0];
    ch->gold += obj->value[1];

    if ( IS_SET( ch->act, PLR_AUTOSPLIT ) )
    {
      /* AUTOSPLIT code */
      if (container
          && (container->item_type != ITEM_CORPSE_PC) )
      {
        members = 0;
        for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
          if ( !IS_AFFECTED( gch, AFF_CHARM ) && is_same_group( gch, ch ) )
            members++;

        if ( members > 1 && ( obj->value[0] > 1 || obj->value[1] ) )
        {
          mprintf(sizeof(buffer), buffer, "%d %d", obj->value[0], obj->value[1] );
          do_function( ch, &do_split, buffer );
        }
      }
    }
    extract_obj( obj );
  }
  else
    obj_to_char( obj, ch );
  return;
}

void do_get( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA  *progobj = NULL;
  int  progfound = FALSE;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char objlist[MSL];
  char *Pobj = objlist;
  char buf[MSL];
  OBJ_DATA *obj = NULL;
  OBJ_DATA *obj_next;
  OBJ_DATA *container = NULL;
  bool found, all = FALSE;
  int count  = 0;
  //int locker;
  void *source, *target;
  int  from_flag, to_flag;

  /* Default values */
  source    = ch->in_room;
  from_flag = FROM_ROOM;
  target    = ch;
  to_flag   = FROM_CH;

  parse_objhandling( ch, objlist, &source, &from_flag,
                     &target, &to_flag, argument );

  argument = one_argument( argument, arg1 );

  if ( arg1[0] == '\0' )
  {
    /* Find the first named obj. */
    if ( ch->in_room && ( obj = ch->in_room->contents ) )
    {
      while ( obj && IS_NULLSTR( obj->name ) && IS_DELETED( obj ) )
        obj = obj->next_content;
      if ( obj && CAN_WEAR( obj, ITEM_TAKE ) )
      {
        sprintf( buf, "\"%s\"", obj->name );
        do_function( ch, &do_get, buf );
        return;
      }
    }
    send_to_char( "Get what?\n\r", ch );
    return;
  }

  Pobj  = one_argument( Pobj, arg1 );
  count = mult_argument( arg1, arg2 );

  if ( count <= 0 )
  {
    send_to_char( "Quit being silly, try DROP instead.\n\r", ch );
    return;
  }

  if ( !str_prefix( "all.", arg2 ) )
  {
    strcpy( arg2, &arg2[4] );
    all = TRUE;
  }

  if ( IS_SET( from_flag, FROM_NOT_FOUND ) )
  {
    act( "You don't see that container here.", ch, NULL, NULL, TO_CHAR );
    return;
  }
  if ( IS_SET( from_flag, FROM_OBJ ) ) // This line added -- Merak
  {
    container = ( OBJ_DATA *)source;
    switch ( container->item_type )
    {
      default:
        send_to_char( "That's not a container.\n\r", ch );
        return;

      case ITEM_SPELLBOOK: // these are now scroll containers
      case ITEM_CONTAINER:
      case ITEM_KEYRING:
      case ITEM_CORPSE_NPC:
        break;

      case ITEM_LOCKER:
        if ( IS_NPC( ch ) )
        {
          send_to_char( "MOBs don't have lockers.\n\r", ch );
          return;
        }
        if ( ( ch->clan && ch->clan->locker != container->pIndexData->vnum )
             ||   ( !( ch->clan ) && container->value[0] ) )
        {
          send_to_char( "That isn't your locker.\n\r", ch );
          return;
        }
        if ( ch->pcdata->locker_max <= 0 )
        {
          send_to_char( "You don't have a locker assigned to you.\n\r", ch );
          return;
        }
        break;

      case ITEM_CORPSE_PC:
        if ( !can_loot( ch, container ) )
        {
          send_to_char( "Corpse looting is not permitted by you.\n\r", ch );
          return;
        }
    }

    if ( IS_SET( container->value[1], CONT_CLOSED )
         &&   container->item_type != ITEM_SPELLBOOK )
    {
      act( "$p is closed.", ch, container, NULL, TO_CHAR );
      return;
    }
  }    /* Now we have a name for an object, and the container if it exist */

  if ( strcmp( arg2, "all" ) )
  {
    if ( IS_SET( from_flag, FROM_OBJ ) )
    {
      if ( container->item_type == ITEM_LOCKER )
        obj = ch->pcdata->locker;
      else
        obj = container->contains;

      if ( ( obj = get_obj_list( ch, arg2, obj ) ) == NULL )
      {
        act( "You don't see that in $P.", ch, NULL, container, TO_CHAR );
        return;
      }
    }
    else
    {
      if ( ( obj = get_obj_list( ch, arg2, ch->in_room->contents ) ) == NULL )
      {
        send_to_char( "You don't see that here.\n\r", ch );
        return;
      }
    }
  }

  /* 'get NNNN coins' */
  int weight;
  int amount_silver = 0, amount_gold = 0;

  if ( !str_prefix( arg2, "coins"  )
       ||   !str_prefix( arg2, "gold"   )
       ||   !str_prefix( arg2, "silver" ) )
  {
    if ( obj->item_type == ITEM_MONEY )
    {
      if ( ( obj->value[1] <= 0 ) && ( obj->value[0] <= 0 ) )
      {
        send_to_char( "There are no coins there.\n\r", ch );
        return;
      }

      amount_gold = obj->value[1];
      amount_silver = obj->value[0];

      weight = GOLD_WEIGHT( amount_gold ) + SILVER_WEIGHT( amount_silver );
      if ( has_money_pouch( ch ) )
        weight /= MONEY_POUCH_WEIGHT_MULT;

      /* end of get-gold change */
      if ( get_carry_weight( ch ) + weight > can_carry_w( ch ) )
      {
        send_to_char(
          "You're carrying too much weight to take that much.\n\r", ch );
        return;
      }

      if ( ( obj->value[0] - amount_silver ) <= 0
           &&   ( obj->value[1] - amount_gold ) <= 0 )
      {
        if ( container == NULL )
        {
          act( "You get $p.", ch, obj, NULL, TO_CHAR );
          act( "$n gets $p.", ch, obj, NULL, TO_ROOM );
          extract_obj( obj );
        }
        else
        {
          act( "You get $p from $P.", ch, obj, container, TO_CHAR );
          act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
          extract_obj( obj );
        }
      }
      else
      {
        char coinstr[MAX_STRING_LENGTH];

        if ( amount_gold )
          mprintf( sizeof(coinstr), coinstr, "{Y%d {yg{Yo{yld{x coin%s",
                   amount_gold, amount_gold == 1 ? "" : "s" );
        else
          mprintf( sizeof(coinstr), coinstr, "{W%d {wsi{Wl{Dv{wer{x coin%s",
                   amount_silver, amount_silver == 1 ? "" : "s" );

        change_money( obj, obj->value[1] - amount_gold,
                      obj->value[0] - amount_silver );

        if ( IS_SET( from_flag, FROM_ROOM ) )
          mprintf( sizeof(buf), buf, "You get %s leaving $p.", coinstr );
        else
          mprintf( sizeof(buf), buf, "You get %s leaving $p in $P.", coinstr );
        act( buf, ch, obj, container, TO_CHAR );

        if ( IS_SET( from_flag, FROM_ROOM ) )
          mprintf( sizeof(buf), buf, "$n gets %s leaving $p.", coinstr );
        else
          mprintf( sizeof(buf), buf, "$n gets %s leaving $p in $P.", coinstr );
        act( buf, ch, obj, container, TO_ROOM );
      }

      ch->silver += amount_silver;
      ch->gold   += amount_gold;

      if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOSPLIT ) )
      {
        CHAR_DATA   *gch;
        int     members = 0;

        if (container
            && (container->item_type != ITEM_CORPSE_PC) )
        {

          for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
            if ( !IS_AFFECTED( gch, AFF_CHARM ) && is_same_group( gch, ch )
                 &&   IS_AWAKE( gch ) && !IS_DELETED( gch ) )
              members++;

          if ( amount_gold > 1 || amount_silver > 1 )
          {
            mprintf( sizeof(buf), buf, "%d %d", amount_silver, amount_gold );
            do_function( ch, &do_split, buf );
          }
        }
      }
      return;
    } // I think this is right... The gold found is not money, so...
  }
  if ( all )
  {
    strcat( strcpy( buf, "all." ), arg2 );
    strcpy( arg2, buf );
  }

  if ( IS_SET( from_flag, FROM_ROOM ) )
  {
    if ( str_cmp( arg2, "all" ) && str_prefix( "all.", arg2 ) )
    {
      if ( count < 1 )
      {
        send_to_char( "Get how many?!\n\r", ch );
        return;
      }
      if (count > 50)
      {
        send_to_char(
          "You are trying to get more than 50, which is most you can get at one time.\n\r",ch);
        return;
      }

      get_mult_obj( ch, arg2, NULL, count );
    }
    else
    {
      /* 'get all' or 'get all.obj' */
      get_obj_list_to_char( ch->in_room->contents, ch, TRUE, TRUE, arg2, FALSE, NULL );
      get_temp_weight = 0;
      get_temp_number = 0;
      for ( obj = ch->in_room->contents; obj; obj = obj_next )
      {
        obj_next = obj->next_content;
        if ( obj->next_content == obj )
        {
          bugf( "infinite obj loop. %s", interp_cmd );
          obj_next = NULL;
        }
        if ( ( obj->wear_loc == WEAR_NONE )
             && ( arg2[3] == '\0' || is_name( &arg2[4], obj->name ))
             &&   can_see_obj( ch, obj ) )
        {
          get_silent_obj( ch, obj, NULL );

          if (mp_get_check( ch, obj ))
          {
            progobj = obj;
            progfound = TRUE;
          }
        }
      }
    }
  }
  else // container...
  {
    /* 'get ... container' */
    if ( str_cmp( arg2, "all" ) && str_prefix( "all.", arg2 ) )
    {
      if ( count < 1 )
      {
        send_to_char( "Get how many?!\n\r", ch );
        return;
      }
      if ( count > 50 )
      {
        send_to_char( "You can't get more than 50 items at one time.\n\r",ch);
        return;
      }
      /* 'get obj container' */
      get_mult_obj( ch, arg2, container, count );
    }
    else
    {
      /* 'get all container' or 'get all.obj container' */
      found = FALSE;
      if ( container->item_type == ITEM_LOCKER )
      {
        get_obj_list_to_char( ch->pcdata->locker, ch, TRUE, TRUE,
                              arg2, TRUE, container );
        obj = ch->pcdata->locker;
      }
      else
      {
        get_obj_list_to_char( container->contains, ch, TRUE, TRUE,
                              arg2, TRUE, container );
        obj = container->contains;
      }
      get_temp_weight = 0;
      get_temp_number = 0;
      for ( ; obj; obj = obj_next )
      {
        obj_next = obj->next_content;
        if ( obj->next_content == obj )
        {
          bugf( "infinite obj loop. %s", interp_cmd );
          obj_next = NULL;
        }
        if ( ( arg2[3] == '\0' || is_name( &arg2[4], obj->name ) )
             &&     can_see_obj( ch, obj ) )
        {
          found = TRUE;
          if ( container->pIndexData->vnum == OBJ_VNUM_PIT
               &&  !IS_IMMORTAL( ch ) )
          {
            send_to_char( "Don't be so greedy!\n\r", ch );
            return;
          }
          get_silent_obj( ch, obj, container );

          if (mp_get_check( ch, obj ))
          {
            progobj = obj;
            progfound = TRUE;
          }

        }
      }

      if ( !found )
      {
        if ( arg1[3] == '\0' )
          act( "You see nothing in $p.",
               ch, container, NULL, TO_CHAR );
        else
          act( "You see nothing like that in $p.",
               ch, container, NULL, TO_CHAR );
      }
    }
  }
  if (progfound)
    mp_get_trigger( ch, progobj );

  return;
}

void do_old_get( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char arg4[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  OBJ_DATA *container = NULL;
  bool found;
  int count = 0;
  int amount = 0;
  //int locker;

  argument = one_argument( argument, arg1 );

  if ( arg1[0] == '\0' )
  {
    if ( ch->in_room && ( obj = ch->in_room->contents ) )
    {
      /* Find the first named obj. */
      while ( obj && IS_NULLSTR( obj->name ) && IS_DELETED( obj ) )
        obj = obj->next_content;
      if ( obj && CAN_WEAR( obj, ITEM_TAKE ) )
      {
        char buf[MAX_STRING_LENGTH];
        sprintf( buf, "\"%s\"", obj->name );
        do_function( ch, &do_get, buf );
        return;
      }
    }
    send_to_char( "Get what?\n\r", ch );
    return;
  }


  /* Get type. */

  if ( is_number( arg1 ) )
  {
    if ( ( amount = atoi( arg1 ) ) <= 0 )
    {
      send_to_char( "Quit being silly.\n\r", ch );
      return;
    }
    argument = one_argument( argument, arg1 );
  }

  argument = one_argument( argument, arg2 );

  if ( !str_cmp( arg2, "from" ) )
    argument = one_argument( argument, arg2 );

  argument = one_argument( argument, arg3 );


  /* 'get NNNN coins' */
  int weight;
  int amount_silver = 0, amount_gold = 0;

  if ( !str_prefix( arg1, "coins"  )
       ||   !str_prefix( arg1, "gold"   )
       ||   !str_prefix( arg1, "silver" ) )
  {
    if ( arg2[0] == '\0' )
    {
      if ( ( obj = get_obj_list( ch, arg1, ch->in_room->contents ) ) == NULL )
      {
        send_to_char( "You don't see that here.\n\r", ch );
        return;
      }
    }
    else
    {
      if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
      {
        act( "You see no $T here.", ch, NULL, arg2, TO_CHAR );
        return;
      }

      switch ( container->item_type )
      {
        default:
          send_to_char( "That's not a container.\n\r", ch );
          return;
        case ITEM_DRINK_CON:
          send_to_char( "That's only a drink container.\n\r", ch );
          return;
        case ITEM_SPELLBOOK: // these are now scroll containers
        case ITEM_CONTAINER:
        case ITEM_KEYRING:
        case ITEM_CORPSE_NPC:
          break;
        case ITEM_CORPSE_PC:
          if ( !can_loot( ch, container ) )
          {
            act( "$p: corpse looting is not permitted.",
                 ch, container, NULL, TO_CHAR );
            return;
          }
      }

      if ( IS_SET( container->value[1], CONT_CLOSED ) )
      {
        act( "$p is closed.", ch, container, NULL, TO_CHAR );
        return;
      }

      if ( ( obj = get_obj_list( ch, arg1, container->contains ) ) == NULL )
      {
        act( "You don't see that in $P.", ch, NULL, container, TO_CHAR );
        return;
      }
    }
//    container = NULL;

    if ( obj->item_type == ITEM_MONEY )
    {
      if ( !str_prefix( arg1, "gold" ) )
      {
        if ( obj->value[1] <= 0 )
        {
          send_to_char( "There is no {yg{Yo{yld{x there.\n\r", ch );
          return;
        }
        else if ( obj->value[1] < amount )
        {
          amount = obj->value[1];
        }
        amount_gold = amount;
        weight = GOLD_WEIGHT( amount );
        if ( has_money_pouch( ch ) )
          weight /= MONEY_POUCH_WEIGHT_MULT;
      }
      else
      {
        if ( obj->value[0] <= 0 )
        {
          send_to_char( "There is no {wsi{Wl{Dv{wer{x there.\n\r", ch );
          return;
        }
        else if ( obj->value[0] < amount )
        {
          amount = obj->value[0];
        }
        amount_silver = amount;
        weight = SILVER_WEIGHT( amount );
        if ( has_money_pouch ( ch ) )
          weight /= MONEY_POUCH_WEIGHT_MULT;
      }

      if ( get_carry_weight( ch ) + weight > can_carry_w( ch ) )
      {
        send_to_char(
          "You're carrying too much weight to take that much.\n\r", ch );
        return;
      }

      if ( ( obj->value[0] - amount_silver ) <= 0
           &&   ( obj->value[1] - amount_gold ) <= 0 )
      {
        if ( container == NULL )
        {
          act( "You get $p.", ch, obj, NULL, TO_CHAR );
          act( "$n gets $p.", ch, obj, NULL, TO_ROOM );
          extract_obj( obj );
        }
        else
        {
          act( "You get $p from $P.", ch, obj, container, TO_CHAR );
          act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
          extract_obj( obj );
        }
      }
      else
      {
        char coinstr[MAX_STRING_LENGTH];
        char buf[MAX_STRING_LENGTH];

        if ( amount_gold )
          mprintf( sizeof(coinstr), coinstr, "{Y%d {yg{Yo{yld{x coin%s",
                   amount_gold, amount_gold == 1 ? "" : "s" );
        else
          mprintf( sizeof(coinstr), coinstr, "{W%d {wsi{Wl{Dv{wer{x coin%s",
                   amount_silver, amount_silver == 1 ? "" : "s" );

        change_money( obj, obj->value[1] - amount_gold,
                      obj->value[0] - amount_silver );

        if ( container == NULL )
          mprintf( sizeof(buf), buf, "You get %s leaving $p.", coinstr );
        else
          mprintf( sizeof(buf), buf, "You get %s leaving $p in $P.", coinstr );
        act( buf, ch, obj, container, TO_CHAR );

        if ( container == NULL )
          mprintf( sizeof(buf), buf, "$n gets %s leaving $p.", coinstr );
        else
          mprintf( sizeof(buf), buf, "$n gets %s leaving $p in $P.", coinstr );
        act( buf, ch, obj, container, TO_ROOM );
      }

      ch->silver += amount_silver;
      ch->gold   += amount_gold;

      if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOSPLIT ) )
      {
        CHAR_DATA  *gch;
        char  buf[MAX_STRING_LENGTH];
        int   members = 0;

        if (container
            && (container->item_type != ITEM_CORPSE_PC) )
        {
          for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
            if ( !IS_AFFECTED( gch, AFF_CHARM ) && is_same_group( gch, ch )
                 &&   IS_AWAKE( gch ) && !IS_DELETED( gch ) )
              members++;

          if ( members > 1 && ( amount_gold > 1 || amount_silver > 1 ) )
          {
            mprintf( sizeof(buf), buf, "%d %d", obj->value[0], obj->value[1] );
            do_function( ch, &do_split, buf );
          }
        }
      }
      return;
    } // I think this is right... The gold found is not money, so...
  }

  if ( arg2[0] == '\0' )
  {
    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
      /* 'get obj' */
      count = mult_argument(arg1, arg2);
      if ( amount > 0 ) count = amount;
      if ( count < 1 )
      {
        send_to_char( "Get how many?!\n\r", ch );
        return;
      }
      if (count > 50)
      {
        send_to_char(
          "You are trying to get more than 50, which is most you can get at one time.\n\r",ch);
        return;
      }
      get_mult_obj( ch, arg2, NULL, count );
    }
    else
    {
      /* 'get all' or 'get all.obj' */
      get_obj_list_to_char( ch->in_room->contents, ch, TRUE, TRUE,
                            arg1, FALSE, NULL );
      get_temp_weight = 0;
      get_temp_number = 0;
      for ( obj = ch->in_room->contents; obj; obj = obj_next )
      {
        obj_next = obj->next_content;
        if ( obj->next_content == obj )
        {
          bugf( "infinite obj loop. %s", interp_cmd );
          obj_next = NULL;
        }
        if ( ( obj->wear_loc == WEAR_NONE )
             && ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ))
             &&   can_see_obj( ch, obj ) )
          get_silent_obj( ch, obj, NULL );
      }
    }
  }
  else // arg2[0] != '\0'
  {
    /* 'get ... container' */
    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
      send_to_char( "You can't do that.\n\r", ch );
      return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
      act( "You see no $T here.", ch, NULL, arg2, TO_CHAR );
      return;
    }
    switch ( container->item_type )
    {
      default:
        send_to_char( "That's not a container.\n\r", ch );
        return;

      case ITEM_SPELLBOOK: // these are now scroll containers
      case ITEM_CONTAINER:
      case ITEM_KEYRING:
      case ITEM_CORPSE_NPC:
        break;

      case ITEM_LOCKER:
        if ( IS_NPC( ch ) )
        {
          send_to_char( "MOBs don't have lockers.\n\r", ch );
          return;
        }

        if ( ch->clan )
        {

          if ( ch->clan->locker != container->pIndexData->vnum )
          {
            send_to_char("That isn't your locker.\n\r", ch);
            return;
          }
        }
        else
        {
          if ( container->value[0] != 0 )
          {
            send_to_char( "That isn't your locker.\n\r", ch );
            return;
          }

        }

        if ( ch->pcdata->locker_max <= 0 )
        {
          send_to_char("You don't have a locker assigned to you.\n\r",ch);
          return;
        }

        break;

      case ITEM_CORPSE_PC:
        if ( !can_loot( ch, container ) )
        {
          send_to_char( "Corpse looting is not permitted by you.\n\r", ch );
          return;
        }
    }


    if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
      act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
      return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {

      count = mult_argument( arg1, arg4 );
      if ( count < 1 )
      {
        send_to_char( "Get how many?!\n\r", ch );
        return;
      }
      if ( count > 50 )
      {
        send_to_char( "You can't get more than 50 items at one time.\n\r",ch);
        return;
      }

      /* 'get obj container' */
      get_mult_obj( ch, arg4, container, count );
    }
    else
    {
      /* 'get all container' or 'get all.obj container' */
      found = FALSE;

      if ( container->item_type == ITEM_LOCKER )
      {

        if ( IS_NPC( ch ) )
        {
          send_to_char( "MOBs don't have lockers.\n\r", ch );
          return;
        }

        if ( ch->clan )
        {

          if ( ch->clan->locker != container->pIndexData->vnum )
          {
            send_to_char( "That isn't your locker.\n\r", ch );
            return;
          }
        }
        else
        {
          if ( container->value[0] != 0 )
          {
            send_to_char( "That isn't your locker.\n\r", ch );
            return;
          }

        }

        if ( ch->pcdata->locker_max <= 0 )
        {
          send_to_char( "You don't have a locker assigned to you.\n\r", ch );
          return;
        }

        get_obj_list_to_char( ch->pcdata->locker, ch, TRUE, TRUE,
                              arg1, TRUE, container );
        obj = ch->pcdata->locker;
      }
      else
      {
        get_obj_list_to_char( container->contains, ch, TRUE, TRUE,
                              arg1, TRUE, container );
        obj = container->contains;
      }
      get_temp_weight = 0;
      get_temp_number=0;
      for ( ; obj; obj = obj_next )
      {
        obj_next = obj->next_content;
        if (obj->next_content == obj)
        {
          bugf("infinite obj loop. %s",interp_cmd);
          obj_next = NULL;
        }
        if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
             &&     can_see_obj( ch, obj ) )
        {
          found = TRUE;
          if (container->pIndexData->vnum == OBJ_VNUM_PIT
              &&  !IS_IMMORTAL(ch))
          {
            send_to_char("Don't be so greedy!\n\r",ch);
            return;
          }
          get_silent_obj( ch, obj, container );

        }
      }

      if ( !found )
      {
        if ( arg1[3] == '\0' )
          act( "You see nothing in $p.",
               ch, container, NULL, TO_CHAR );
        else
          act( "You see nothing like that in $p.",
               ch, container, NULL, TO_CHAR );
      }
    }
  }
  return;
}



void do_put( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg4[MAX_INPUT_LENGTH];
  char error_buf[MSL];
  OBJ_DATA *container;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  int count=0, i=0;
  char buf[MAX_STRING_LENGTH];
  char buf1[MIL];
  char  **pObjStrShow;
  bool  *pObjWhere;
  int   *pObjNumShow;
  bool  bObjWhere=0;
  bool  fCombine;
  int   nShow;
  int   iShow;
  int  num_found=0;
  int  max_found=50;
  //int  locker = 0;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (!str_cmp(arg2,"in") || !str_cmp(arg2,"on"))
    argument = one_argument(argument,arg2);

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  {
    send_to_char( "Put what in what?\n\r", ch );
    return;
  }

  /*    if (split_string(arg2, '.', str1,str2) || !str_cmp( arg2, "all" ));
   */

  strcpy( error_buf, "" );
  if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
  {
    send_to_char( "You can't do that.\n\r", ch );
    return;
  }

  if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
  {
    act( "You see no $T here.", ch, NULL, arg2, TO_CHAR );
    return;
  }

  if ( container->item_type != ITEM_CONTAINER
       &&   container->item_type != ITEM_LOCKER
       &&   container->item_type != ITEM_KEYRING
       &&   container->item_type != ITEM_SPELLBOOK )
  {
    send_to_char( "That's not a container.\n\r", ch );
    return;
  }

  if ( container->item_type == ITEM_CONTAINER
       &&   IS_SET( container->value[1], CONT_CLOSED ) )
  {
    act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
    return;
  }

  if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
  {
    count = mult_argument( arg1, arg4 );
    if ( count < 1 )
    {
      send_to_char( "Put how many?\n\r", ch );
      return;
    }
    if ( count > 50 )
    {
      send_to_char( "50 is the max limit to put.\n\r", ch );
      return;
    }

    pObjStrShow = alloc_mem( max_found * sizeof(char *) );
    pObjWhere  = alloc_mem( max_found * sizeof(bool)    );
    pObjNumShow = alloc_mem( max_found * sizeof(int)  );
    nShow      = 0;
    for ( i = 0; i < count; i++ )
    {
      /* 'put obj container' */
      if ( ( obj = get_obj_carry( ch, arg4, ch ) ) == NULL )
      {
        if (count == 1)
          mprintf(sizeof(error_buf),error_buf,
                  "You don't have that item.\n\r" );
        else
          mprintf(sizeof(error_buf),error_buf,
                  "You don't have %d items.\n\r", count );
        break;
      }

      if ( obj == container )
      {
        mprintf(sizeof(error_buf),error_buf,
                "You can't fold it into itself.\n\r" );
        break;
      }

      if ( !can_drop_obj( ch, obj ) )
      {
        mprintf(sizeof(error_buf),error_buf, "You can't let go of it.\n\r" );
        break;
      }

      if ( container->item_type == ITEM_CONTAINER )
      {
        if ( WEIGHT_MULT( obj ) != 100 )
        {
          mprintf(sizeof(error_buf),error_buf,
                  "You have a feeling that would be a bad idea.\n\r");
          break;
        }

        if ( get_obj_weight( obj ) + get_true_weight( container )
             > ( container->value[0] * 10 )
             ||  get_obj_weight( obj ) > ( container->value[3] * 10 ) )
        {
          mprintf(sizeof(error_buf),error_buf, "It won't fit.\n\r");
          break;
        }

        if ( container->pIndexData->vnum == OBJ_VNUM_PIT
             &&  !CAN_WEAR( container, ITEM_TAKE ) )
        {
          if ( obj->timer )
            SET_BIT( obj->extra_flags, ITEM_HAD_TIMER );
          else
            obj->timer = number_range( 100, 200 );
        }
        obj_from_char( obj );
        obj_to_obj( obj, container );
        num_found++;
      }
      else if ( container->item_type == ITEM_LOCKER )
      {
        /*** handle lockers ***/
        if ( IS_NPC( ch ) )
        {
          mprintf(sizeof(error_buf),error_buf, "MOBs don't have lockers!\n\r");
          break;
        }

        if ( ch->clan )
        {
          if ( ch->clan->locker != container->pIndexData->vnum )
          {
            send_to_char("That isn't your locker.\n\r", ch);
            break;
          }
        }
        else
        {
          if ( container->value[0] )
          {
            send_to_char( "That isn't your locker.\n\r", ch );
            break;
          }
        }

        if ( ch->pcdata->locker_max <= 0 )
        {
          mprintf(sizeof(error_buf),error_buf,
                  "You don't have a locker assigned to you.\n\r");
          break;
        }

        if ( ( get_obj_number( obj ) + ch->pcdata->locker_content ) >
             ch->pcdata->locker_max )
        {
          mprintf(sizeof(error_buf),error_buf, "It won't fit.\n\r");
          break;
        }

        // Taeloch 3/30/17: "!obj->wear_loc == WEAR_NONE" doesn't make sense
        // Can't put in a locker if it's currently being worn or no-drop
         if ( ( obj->wear_loc != WEAR_NONE ) || !can_drop_obj( ch, obj ) )
        {
          mprintf(sizeof(error_buf),error_buf,
                  "You cannot place this item that way.\n\r" );
          break;
        }
        obj_from_char( obj );
        obj_to_locker( obj, ch );
        num_found++;
      }
      else if ( container->item_type == ITEM_SPELLBOOK )
      {
        if ( obj->item_type != ITEM_SCROLL )
        {
          mprintf(sizeof(error_buf),error_buf, "Item is not a scroll.\n\r");
          break;
        }

        if ( ( obj->value[0] > container->value[1] )
             ||   ( obj->level    > container->value[1] ) )
        {
          mprintf(sizeof(error_buf),error_buf, "That scroll is too powerful for this spellbook.\n\r");
          break;
        }

        if ( (get_obj_number(container)) > container->value[0] )
        {
          mprintf(sizeof(error_buf),error_buf,
                  "That spellbook cannot contain any more items (%d).\n\r",
                  container->value[0] );
          break;
        }

        obj_from_char( obj );
        obj_to_obj( obj, container );
        num_found++;
      }
      else if ( container->item_type == ITEM_KEYRING )
      {
        if ( obj->item_type != ITEM_KEY )
        {
          mprintf(sizeof(error_buf),error_buf, "That is not a key.\n\r");
          break;
        }

        if ( (get_obj_number(container)) > container->value[0] )
        {
          mprintf(sizeof(error_buf),error_buf,  "That keyring cannot hold any more keys.\n\r" );
          break;
        }

        obj_from_char( obj );
        obj_to_obj( obj, container );
        num_found++;
      }

      mprintf(sizeof(buf), buf, "%s", obj->short_descr);

      /*
       * Look for duplicates, case sensitive.
       * Matches tend to be near end so run loop backwords.
       */
      fCombine = FALSE;
      for ( iShow = nShow - 1; iShow >= 0; iShow-- )
      {
        if ( pObjWhere[iShow] == bObjWhere
             &&  !strcmp( pObjStrShow[iShow], buf ) )
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
    }

    /*
     * Output the formatted list.
     */

    if (!num_found && (error_buf[0] != '\0'))
      printf_to_char(ch,"{w%s\n\r",error_buf);
    else
    {
      if ( IS_SET( container->value[1], CONT_PUT_ON ) )
      {
        mprintf(sizeof(buf),buf,"You put the following %s item%s onto $p:",
                numcpy( buf1, num_found ), num_found == 1 ? "" : "s" );
        act( buf, ch, container, NULL, TO_CHAR );
      }
      else
      {
        mprintf(sizeof(buf),buf,"You put the following %s item%s into $p:",
                numcpy( buf1, num_found ), num_found == 1 ? "" : "s" );
        act( buf, ch, container, NULL, TO_CHAR );
      }
      if ( IS_SET( container->value[1], CONT_PUT_ON ) )
      {
        mprintf(sizeof(buf),buf,"$n puts the following %s item%s onto $p:",
                numcpy( buf1, num_found ), num_found == 1 ? "" : "s" );
        act( buf, ch, container, NULL, TO_ROOM );
      }
      else
      {
        mprintf(sizeof(buf),buf,"$n puts the following %s item%s into $p:",
                numcpy( buf1, num_found ), num_found == 1 ? "" : "s" );
        act( buf, ch, container, NULL, TO_ROOM );
      }
      for ( iShow = 0; iShow < nShow; iShow++ )
      {
        if ( pObjStrShow[iShow][0] == '\0' )
        {
          free_string( pObjStrShow[iShow] );
          continue;
        }
        printf_to_char(ch, "(%d) %s\n\r",
                       pObjNumShow[iShow], pObjStrShow[iShow] );
        /* loop room */
        mprintf(sizeof(buf), buf,"(%d) %s",
                pObjNumShow[iShow], pObjStrShow[iShow] );
        act( buf, ch, NULL, NULL, TO_ROOM );
        free_string( pObjStrShow[iShow] );
      }
      if ( error_buf[0] )
        printf_to_char( ch, "{w%s\n\r", error_buf );
    }
    free_mem( pObjStrShow);
    free_mem( pObjWhere );
    free_mem( pObjNumShow);
  }
  else
  {
    /*** handle lockers and bail first, put all or put all. ***/
    if ( container->item_type == ITEM_LOCKER )
    {
      if ( IS_NPC( ch ) )
      {
        send_to_char( "MOBs don't have lockers.\n\r", ch );
        return;
      }

      if ( ch->clan )
      {
        if ( ch->clan->locker != container->pIndexData->vnum )
        {
          send_to_char("That isn't your locker.\n\r", ch);
          return;
        }
      }
      else
      {
        if ( container->value[0] )
        {
          send_to_char( "That isn't your locker.\n\r", ch );
          return;
        }
      }

      if ( ch->pcdata->locker_max <= 0 )
      {
        send_to_char( "You don't have a locker assigned to you.\n\r", ch );
        return;
      }

      act( "$n puts several objects in $P.", ch, NULL, container, TO_ROOM );

      for ( obj = ch->carrying; obj; obj = obj_next )
      {
        obj_next = obj->next_content;
        if ( obj->next_content == obj )
        {
          bugf("infinite obj loop. %s",interp_cmd);
          obj_next = NULL;
        }
        if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
             &&     obj->wear_loc == WEAR_NONE
             &&     can_drop_obj( ch, obj )
             &&   ( get_obj_number( obj ) + ch->pcdata->locker_content) <=
             ch->pcdata->locker_max )
        {
          act( "You put $p in $P.", ch, obj, container, TO_CHAR );
          obj_from_char( obj );
          obj_to_locker( obj, ch );
        }
      }
      return;
    }

    if ( container->item_type == ITEM_SPELLBOOK )
    {
      send_to_char( "You cannot put all your scrolls in at once.\n\r", ch );
      return;
    }
    if ( container->item_type == ITEM_KEYRING )
    {
      send_to_char( "You cannot attach all your keys at once.\n\r", ch );
      return;
    }

    /* 'put all container' or 'put all.obj container' */
    get_temp_weight = 0;
    put_obj_list_to_char( ch->carrying, ch, TRUE, TRUE, arg1, TRUE, container );
    get_temp_weight = 0;
    get_temp_number=0;
    for ( obj = ch->carrying; obj ; obj = obj_next )
    {
      obj_next = obj->next_content;
      if ( obj->next_content == obj )
      {
        bugf( "infinite obj loop. %s",interp_cmd );
        obj_next = NULL;
      }
      get_temp_weight = 0;
      get_temp_number = 0;
      if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
           &&     can_put_item( ch, obj, container ) )
      {
        if ( container->pIndexData->vnum == OBJ_VNUM_PIT
             &&  !CAN_WEAR( obj, ITEM_TAKE ) )
        {
          if ( obj->timer )
            SET_BIT( obj->extra_flags, ITEM_HAD_TIMER );
          else
            obj->timer = number_range( 100, 200 );
        }
        obj_from_char( obj );
        obj_to_obj( obj, container );
      }
    }
  }
  return;
}

void do_drop( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  char   arg[MAX_INPUT_LENGTH];
  char   arg4[MAX_INPUT_LENGTH];
  char   error_buf[MSL];
  int    count = 0, i = 0;
  bool   found;
  char   buf[MAX_STRING_LENGTH];
  char **pObjStrShow;
  bool  *pObjWhere;
  int   *pObjNumShow;
  char  *oarg = argument;
  bool   bObjWhere=0;
  bool   fCombine;
  int    nShow;
  int    iShow;
  int    num_found=0;
  int    max_found=50;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Drop what?\n\r", ch );
    return;
  }

  strcpy(error_buf,"\0");

  if ( is_number( arg ) )
  {
    /* 'drop NNNN coins' */
    int amount, gold = 0, silver = 0;

    amount   = atoi(arg);
    argument = one_argument( argument, arg );

    if ( amount <= 0
         || ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" )
              &&   str_cmp( arg, "gold"  ) && str_cmp( arg, "sil" ) && str_cmp( arg, "silver") ) )
    {
      send_to_char( "Sorry, you can't do that.\n\r", ch );
      return;
    }

    if ( !str_cmp( arg, "coins") || !str_cmp(arg,"coin")
         ||   !str_cmp( arg, "silver") || !str_cmp( arg, "sil"))
    {
      if (ch->silver < amount)
      {
        send_to_char("You don't have that much silver.\n\r",ch);
        return;
      }

      ch->silver -= amount;
      silver = amount;
    }
    else
    {
      if (ch->gold < amount)
      {
        send_to_char("You don't have that much gold.\n\r",ch);
        return;
      }

      ch->gold -= amount;
      gold = amount;
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
    {
      obj_next = obj->next_content;
      if (obj->next_content == obj)
      {
        bugf("infinite obj loop. %s",interp_cmd);
        obj_next = NULL;
      }

      switch ( obj->pIndexData->vnum )
      {
        case OBJ_VNUM_SILVER_ONE:
          silver += 1;
          extract_obj(obj);
          break;
        case OBJ_VNUM_GOLD_ONE:
          gold += 1;
          extract_obj( obj );
          break;
        case OBJ_VNUM_SILVER_SOME:
          silver += obj->value[0];
          extract_obj(obj);
          break;
        case OBJ_VNUM_GOLD_SOME:
          gold += obj->value[1];
          extract_obj( obj );
          break;
        case OBJ_VNUM_COINS:
          silver += obj->value[0];
          gold += obj->value[1];
          extract_obj(obj);
          break;
      }
    }

    obj_to_room( create_money( gold, silver ), ch->in_room );
    act( "$n drops some coins.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You drop the following item(s):\n\r", ch);
    printf_to_char(ch, "     %d %s coin%s\n\r", amount, (!str_cmp(arg, "gold") ? "{yg{Yo{yld{x" : "{wsi{Wl{Dv{wer{x"), (amount == 1 ? "" : "s") );
    return;
  } // if isnumber(arg)

  if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
  {
    count = mult_argument(arg,arg4);
    if (count < 1)
    {
      send_to_char("Drop how many?\n\r",ch);
      return;
    }
    if (count > 50)
    {
      send_to_char("50 is max limit to Drop.\n\r",ch);
      return;
    }

    pObjStrShow = alloc_mem( max_found * sizeof(char *) );
    pObjWhere   = alloc_mem( max_found * sizeof(bool) );
    pObjNumShow = alloc_mem( max_found * sizeof(int) );
    nShow  = 0;

    for (i=0; i < count; i++)
    {
      /* 'drop obj' */
      if ( ( obj = get_obj_carry( ch, arg4, ch ) ) == NULL )
      {
        if (count == 1)
          mprintf(sizeof(error_buf), error_buf,"You do not have that item.\n\r");
        else
          mprintf(sizeof(error_buf), error_buf,"You do not have (%d) items.\n\r", count);
        break;
      }

      if ( !can_drop_obj( ch, obj ) )
      {
        mprintf(sizeof(error_buf), error_buf,"You can't let go of it.\n\r");
        break;
      }

      obj_from_char( obj );
      obj_to_room( obj, ch->in_room );
      num_found++;
      melt_drop(ch,obj);
      mprintf(sizeof(buf), buf,"%s", obj->short_descr);

      /*
      * Look for duplicates, case sensitive.
      * Matches tend to be near end so run loop backwords.
      */
      fCombine = FALSE;
      for ( iShow = nShow - 1; iShow >= 0; iShow-- )
      {
        if ( pObjWhere[iShow] == bObjWhere
             &&   !strcmp( pObjStrShow[iShow], buf ) )
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
    } // for

    /*
     * Output the formatted list.
     */
    if (!num_found && (error_buf[0] != '\0'))
      printf_to_char(ch,"{w%s\n\r",error_buf);
    else
    {
      send_to_char("You drop the following item(s): \n\r",ch);
      act("$n drops the following item(s):",ch, NULL, NULL, TO_ROOM);
      for ( iShow = 0; iShow < nShow; iShow++ )
      {
        if ( pObjStrShow[iShow][0] == '\0' )
        {
          free_string( pObjStrShow[iShow] );
          continue;
        }

        printf_to_char(ch, "(%d) %s\n\r", pObjNumShow[iShow],pObjStrShow[iShow]);
        /* loop room */
        mprintf(sizeof(buf), buf,"(%d) %s", pObjNumShow[iShow], pObjStrShow[iShow]);
        act( buf, ch, NULL, NULL, TO_ROOM );
        free_string( pObjStrShow[iShow] );
      }

      if (error_buf[0] != '\0')
        printf_to_char(ch,"{w%s\n\r",error_buf);

    }
    free_mem( pObjStrShow);
    free_mem( pObjWhere );
    free_mem( pObjNumShow);
  }
  else
  {
    /* 'drop all' or 'drop all.obj' */
    found = FALSE;
    put_obj_list_to_char(ch->carrying, ch, TRUE, TRUE, oarg, FALSE, NULL);
    get_temp_weight = 0;
    get_temp_number = 0;

    for ( obj = ch->carrying; obj; obj = obj_next )
    {
      obj_next = obj->next_content;
      if (obj->next_content == obj)
      {
        bugf("infinite obj loop. %s",interp_cmd);
        obj_next = NULL;
      }

      // if "all" was first and ONLY arg, or if 'all.NAME' or 'all NAME' match
      if (can_put_item(ch, obj, NULL)
          && ( is_name( &oarg[4], obj->name )
               || (oarg[3] == '\0') ) )
      {
        found = TRUE;
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        melt_drop(ch,obj);
      }
    }

    if ( !found )
    {
      if ( arg[3] == '\0' )
        act( "You are not carrying anything.", ch, NULL, arg, TO_CHAR );
      else
        act( "You are not carrying any $T.", ch, NULL, &arg[4], TO_CHAR );
    }
  }

  return;
}



void do_give( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  //char arg3 [MIL];
  char arg4 [MAX_INPUT_LENGTH];
  char error_buf[MSL];
  int count = 0, i = 0;
  char buf[MAX_STRING_LENGTH];
  char buf1[MIL];
  CHAR_DATA *victim;
  OBJ_DATA  *obj = NULL, *obj_next, *obj_first = NULL;
  char  **pObjStrShow;
  bool  *pObjWhere;
  int   *pObjNumShow;
  bool   bObjWhere = 0;
  bool   fCombine;
  int    nShow;
  int    iShow;
  int    num_found = 0;
  int    max_found = 50;
  int    amount    = 0;
  int    silver = 0, gold = 0;
  bool   moneyobj = 0;
  bool   all = FALSE;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( IS_GHOST( ch ) )
  {
    send_to_char( "Giving is useless as you are still {rDEAD{x.\n\r", ch );
    return;
  }

  strcpy( error_buf, "" );
  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  {
    send_to_char( "Give what to whom?\n\r", ch );
    return;
  }

  if ( is_number( arg1 ) )
  {
    amount   = atoi( arg1 );
    strcpy( arg1, arg2 );
    argument = one_argument( argument, arg2 );
    if ( amount <= 0 )
    {
      printf_to_char( ch, "Amount %s is to small.\n\r", arg1 );
      return;
    }
  }

  if ( !str_prefix( "all.", arg1 ) )
  {
    all = TRUE;
    amount = 1;
    strcpy( arg1, &arg1[4] );
  }

  if ( !str_cmp( "all", arg1 )
       &&   (argument[0] != '\0' ) )
  {
    all = TRUE;
    amount = 1;
    strcpy( arg1, arg2 );
    argument = one_argument( argument, arg2 );
  }

  if ( arg2[0] == '\0' )
  {
    printf_to_char( ch, "Give %s what to whom??\n\r", arg1 );
    return;
  }

  if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( victim == ch )
  {
    send_to_char( "Giving to yourself is strange.\n\r", ch );
    return;
  }

  /* 'give NNNN coins victim' */
  if ( !str_cmp( arg1, "coins" )
       ||   !str_cmp( arg1, "gold" )
       ||   !str_cmp( arg1, "silver" )
       ||   !str_cmp( arg1, "silv" ) )
  {
    if ( str_prefix( arg1, "gold" ) )
      silver = amount;
    else
      gold = amount;

    if ( all )
    {
      if ( silver ) silver = ch->silver;
      else          gold   = ch->gold;
    }

    if ( ( ch->gold   < gold   )
         ||   ( ch->silver < silver ) )
    {
      send_to_char( "You haven't got that much.\n\r", ch );
      return;
    }

    ch->silver    -= silver;
    victim->silver   += silver;
    ch->gold        -= gold;
    victim->gold      += gold;

    numcpy( buf1, silver ? silver : gold );
    mprintf(sizeof(buf), buf, "$n gives you %s%s %s.",
            silver ? "{W" : "{Y",
            buf1,
            silver ? "{wsi{Wl{Dv{wer{x" : "{yg{Yo{yld{x" );
    act( buf, ch, NULL, victim, TO_VICT    );
    act( "$n gives $N some coins.",  ch, NULL, victim, TO_NOTVICT );
    mprintf(sizeof(buf), buf, "You give $N %s%s %s.",
            silver ? "{W" : "{Y",
            buf1,
            silver ? "{wsi{Wl{Dv{wer{x" : "{yg{Yo{yld{x" );
    act( buf, ch, NULL, victim, TO_CHAR );

    /*
     * Bribe trigger
     */
    if ( IS_NPC( victim ) )
    {
      if ( HAS_TRIGGER( victim, TRIG_BRIBE ) )
        mp_bribe_trigger( victim, ch, silver ? silver : gold * 100 );

      if ( IS_SET( victim->act, ACT_IS_CHANGER ) )
      {
        spec_changer( victim, ch, gold, silver );
        return;
      }

      if ( IS_SET( victim->act, ACT_LOCKER ) )
      {
        spec_locker( victim, ch, gold, silver );
        return;
      }
    }
    return;
  }

  if ( IS_NPC( victim ) && victim->pIndexData->pShop )
  {
    act( "$N tells you 'Sorry, you'll have to sell that.'",
         ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( /* !str_prefix( "all.", arg1 )*/ all || !str_cmp( "all", arg1 ) )
  {
    if ( all )
    {
      strcat( strcpy( buf, "all." ), arg1 );
      strcpy ( arg1, buf );
    } // stupid hack, just so it works right now -- Merak
    give_obj_list_to_char( ch->carrying, ch, TRUE, TRUE, arg1, victim );
    for ( obj = ch->carrying; obj; obj = obj_next )
    {
      obj_next = obj->next_content;
      if ( obj->next_content == obj )
      {
        bugf( "infinite obj loop. %s", interp_cmd );
        obj_next = NULL;
      }
      get_temp_weight = 0;
      get_temp_number = 0;
      if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
           &&   can_give_item( ch, obj, victim))
      {
        obj_from_char( obj );
        obj_to_char( obj, victim );
      }
    }
    return;
  }

  count = mult_argument( arg1, arg4 );
  if ( amount > 0 ) count = amount;
  if ( count < 1 )
  {
    send_to_char( "Give how many?\n\r", ch );
    return;
  }
  if ( count > 50 )
  {
    send_to_char( "50 is max limit to give.\n\r", ch );
    return;
  }

  pObjStrShow   = alloc_mem( max_found * sizeof( char *) );
  pObjWhere      = alloc_mem( max_found * sizeof( bool  ) );
  pObjNumShow   = alloc_mem( max_found * sizeof( int   ) );
  nShow  = 0;
  for ( i = 0; i < count; i++ )
  {
    if ( ( obj = get_obj_carry( ch, arg4, ch ) ) == NULL )
    {
      if ( count == 1 )
        mprintf(sizeof(error_buf), error_buf,
                "You do not have that item.\n\r");
      else
        mprintf(sizeof(error_buf), error_buf,
                "You do not have (%d) items.\n\r",count );
      break;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
      mprintf(sizeof(error_buf), error_buf, "You must remove it first.\n\r" );
      break;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
      mprintf(sizeof(error_buf), error_buf, "You can't let go of it.\n\r" );
      break;
    }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
      mprintf(sizeof(error_buf), error_buf,
              "%s has their hands full.\n\r", PERS( victim, ch ) );
      break;
    }

    if ( get_carry_weight( victim ) + get_obj_weight( obj ) >
         can_carry_w( victim ) )
    {
      mprintf(sizeof(error_buf), error_buf,
              "%s can't carry that much weight.\n\r", PERS( victim, ch ) );
      break;
    }

    if ( !can_see_obj( victim, obj ) )
    {
      mprintf(sizeof(error_buf), error_buf,
              "%s can't see it.\n\r", PERS( victim, ch ) );
      break;
    }
    obj_from_char( obj );

    //New allowance to give money obj's. - Taeloch
    if ( ( obj->item_type == ITEM_MONEY ) && ((obj->value[1] > 0) || (obj->value[0] > 0)) )
    {
      moneyobj = 1;

      victim->silver += obj->value[0];
      victim->gold += obj->value[1];

      if ( (obj->value[1] > 0) && (obj->value[0] > 0) )
      {
        mprintf(sizeof(buf), buf, "$n gives you %d {yg{Yo{yld{x and %d {wsi{Wl{Dv{wer{x.", obj->value[1], obj->value[0]);
        act( buf, ch, NULL, victim, TO_VICT );
        mprintf(sizeof(buf), buf, "You give $N %d {yg{Yo{yld{x and %d {wsi{Wl{Dv{wer{x.", obj->value[1], obj->value[0]);
        act( buf, ch, NULL, victim, TO_CHAR );
      }
      else
      {
        if (obj->value[1] > 0)
        {
          mprintf(sizeof(buf), buf, "$n gives you %d {yg{Yo{yld{x.", obj->value[1]);
          act( buf, ch, NULL, victim, TO_VICT );
          mprintf(sizeof(buf), buf, "You give $N %d {yg{Yo{yld{x.", obj->value[1]);
          act( buf, ch, NULL, victim, TO_CHAR );
        }
        else
        {
          mprintf(sizeof(buf), buf, "$n gives you %d {wsi{Wl{Dv{wer{x.", obj->value[0]);
          act( buf, ch, NULL, victim, TO_VICT );
          mprintf(sizeof(buf), buf, "You give $N %d {wsi{Wl{Dv{wer{x.", obj->value[0]);
          act( buf, ch, NULL, victim, TO_CHAR );
        }
      }

      act( "$n gives $N some coins.",  ch, NULL, victim, TO_NOTVICT );
      break;
    }
    else // item is not ITEM_MONEY, so give it to them
    {
      obj_to_char( obj, victim );
    }
    num_found++;
    MOBtrigger = FALSE;
    mprintf(sizeof(buf), buf, "%s", obj->short_descr );
    /*
     * Look for duplicates, case sensitive.
     * Matches tend to be near end so run loop backwords.
     */
    fCombine = FALSE;
    for ( iShow = nShow - 1; iShow >= 0; iShow-- )
    {
      if ( pObjWhere[iShow] == bObjWhere
           &&   !strcmp( pObjStrShow[iShow], buf ) )
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
    MOBtrigger = TRUE;

    /*
     * Give trigger
     */
    if ( num_found == 1 )
      obj_first = obj;
    else if ( IS_NPC(victim) && HAS_TRIGGER( victim, TRIG_GIVE ) )
      mp_give_trigger( victim, ch, obj );
  }
  /*
   * Output the formatted list.
   */
  if ( !num_found && ( error_buf[0] ) )
    printf_to_char( ch, "{w%s\n\r", error_buf );
  else
  {
    if (!moneyobj)
    {
      strcpy( buf1, " the following item" );
      if ( nShow > 1 || pObjNumShow[0] > 1 ) strcat( buf1, "s" );

      mprintf(sizeof(buf), buf, "You give%s to $N:", buf1 );
      act( buf, ch, NULL, victim, TO_CHAR );
      mprintf(sizeof(buf), buf, "$n gives you%s:", buf1 );
      act( buf, ch, NULL, victim, TO_VICT );
      mprintf(sizeof(buf), buf, "$n gives%s to $N:", buf1 );
      act( buf, ch, NULL, victim, TO_NOTVICT);

      for ( iShow = 0; iShow < nShow; iShow++ )
      {
        if ( pObjStrShow[iShow][0] == '\0' )
        {
          free_string( pObjStrShow[iShow] );
          continue;
        }

        printf_to_char(ch, "(%d) %s\n\r",
                       pObjNumShow[iShow],pObjStrShow[iShow]);

        if ( victim->position > POS_SLEEPING )
          printf_to_char(victim, "(%d) %s\n\r",
                         pObjNumShow[iShow],pObjStrShow[iShow]);
        /* loop room */
        mprintf(sizeof(buf), buf, "(%d) %s",
                pObjNumShow[iShow], pObjStrShow[iShow]);
        act( buf, ch, NULL, victim, TO_NOTVICT );
        free_string( pObjStrShow[iShow] );
      }
      if ( error_buf[0] )
        printf_to_char( ch, error_buf );
    }
  }
  if ( obj_first && IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_GIVE ) )
    mp_give_trigger( victim, ch, obj_first );

  if (moneyobj)
  {
    extract_obj( obj );  // if item was ITEM_MONEY, remove
  }

  free_mem( pObjStrShow);
  free_mem( pObjWhere );
  free_mem( pObjNumShow);
  return;
}


/* for poisoning weapons and food/drink */
void do_envenom(CHAR_DATA *ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  AFFECT_DATA af;
  int percent,skill,ease;
  int toxtype = TOX_VENOM;
  bool toxic = FALSE; // was an advanced toxin used?

  /* find out what */
  if (argument[0] == '\0')
  {
    send_to_char("Syntax: envenom <object>\n\r", ch);
    if ( get_skill( ch, gsn_toxicology ) > 1 )
    {
      send_to_char("        envenom <object> <toxin type>\n\r", ch);
      send_to_char("Valid toxins: venom (default), elemental, fungal, acidic,\n\r", ch);
      send_to_char("              necrotic, viral, neurotoxic, hallucinogenic\n\r", ch);
    }
    return;
  }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (IS_GHOST(ch))
  {
    send_to_char("Envenoming is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  obj =  get_obj_list( ch, arg1, ch->carrying );

  if (obj == NULL)
  {
    send_to_char("You don't have that item.\n\r",ch);
    return;
  }

  if ( ( skill = get_skill( ch, gsn_envenom ) ) < 1 )
  {
    send_to_char("Are you crazy? You'd poison yourself!\n\r",ch);
    return;
  }

  if (arg2[0] != '\0')
  {
    // tox arg specified
    if (!str_prefix( arg2, "venom" ))
      toxtype = TOX_VENOM;
    else if (!str_prefix( arg2, "elemental" ))
      toxtype = TOX_ELEMENTAL;
    else if (!str_prefix( arg2, "fungal" ))
      toxtype = TOX_FUNGAL;
    else if (!str_prefix( arg2, "acidic" ))
      toxtype = TOX_ACIDIC;
    else if (!str_prefix( arg2, "necrotic" ))
      toxtype = TOX_NECROTIC;
    else if (!str_prefix( arg2, "viral" ))
      toxtype = TOX_VIRAL;
    else if (!str_prefix( arg2, "neurotoxic" ))
      toxtype = TOX_NEUROTOXIC;
    else if (!str_prefix( arg2, "hallucinogenic" ))
      toxtype = TOX_HALLUCINOGENIC;
    else
    {
      send_to_char("You don't know how to make that kind of poison.\n\r",ch);
      return;
    }

    if ( toxtype != TOX_VENOM)
    {
      if ( get_skill( ch, gsn_toxicology ) < 1 )
      {
        send_to_char("You don't know how to make poisons.\n\r",ch);
        return;
      }

      if (obj->item_type != ITEM_WEAPON )
      {
        send_to_char("Advanced toxins can only be used on weapons.\n\r",ch);
        return;
      }

      toxic = TRUE;
    }
  } // end: 2nd arg not NULL

  if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
  {
    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
    {
      act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
      return;
    }

    if (obj->value[3])
    {
      act("$n treats $p with deadly {gp{Go{ci{gson{x.",ch,obj,NULL,TO_ROOM);
      act("You treat $p with deadly {gp{Go{ci{gson{x.",ch,obj,NULL,TO_CHAR);
      send_to_char("That seemed too easy...\n\r",ch);
      return;
    }

    if (number_percent() < skill)  /* success! */
    {
      act("$n treats $p with deadly {gp{Go{ci{gson{x.",ch,obj,NULL,TO_ROOM);
      act("You treat $p with deadly {gp{Go{ci{gson{x.",ch,obj,NULL,TO_CHAR);
      if (!obj->value[3])
      {
        obj->value[3] = 1;
        check_improve(ch,gsn_envenom,TRUE,4);
      }
      WAIT_STATE(ch,skill_table[gsn_envenom].beats);
      return;
    }

    act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
    if (!obj->value[3])
      check_improve(ch,gsn_envenom,FALSE,4);
    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
    return;
  }

  if (obj->item_type == ITEM_WEAPON)
  {
    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
    {
      act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
      return;
    }

    if ( (obj->value[3] < 0)
         ||   (attack_table[obj->value[3]].damage == DAM_BASH) )
    {
      send_to_char("You can only envenom edged weapons.\n\r",ch);
      return;
    }

    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
    {
      act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
      return;
    }

    switch (toxtype) // determine ease of application
    {
      case TOX_ELEMENTAL:
      case TOX_FUNGAL:
        ease = 10;
        break;
      case TOX_NEUROTOXIC:
        ease = -10;
        break;
      case TOX_VIRAL:
      case TOX_NECROTIC:
        ease = -5;
        break;
      default:
        ease = 0;
        break;
    }

    percent = number_percent();
    if (percent < (skill+ease))
    {
      af.where     = TO_WEAPON;
      af.type      = gsn_poison;
      af.location  = APPLY_NONE;
      af.modifier  = toxtype;
      af.bitvector = WEAPON_POISON;

      switch (toxtype) // determine how long weapon will stay toxic
      {
        case TOX_NECROTIC: // default toxin has def duratin, but level is important
        case TOX_NEUROTOXIC:
        case TOX_HALLUCINOGENIC:
          af.duration = ch->level/2 * percent / 100;
          af.level    = ch->level;
          break;
        case TOX_VIRAL: // Viruses die quickly on a weapon
        case TOX_ACIDIC: // Acid dries up quickly
          af.duration = ((ch->level/2 * percent / 100) * 2) / 3;
          af.level    = ch->level;
          break;
        case TOX_ELEMENTAL: // Elemental and fungal toxes will stay longer
        case TOX_FUNGAL:
          af.duration = (ch->level/2 * percent / 100) * 2;
          af.level    = ch->level;
          break;
        default:
          af.duration = ch->level/2 * percent / 100;
          af.level    = ch->level * percent / 100;
          break;
      }
      affect_to_obj(obj,&af);

      act("$n coats $p with deadly {gven{Go{cm{x.",ch,obj,NULL,TO_ROOM);
      act("You coat $p with {gven{Go{cm{x.",ch,obj,NULL,TO_CHAR);
      check_improve(ch,gsn_envenom,TRUE,3);
      if (toxic)
        check_improve(ch,gsn_toxicology,TRUE,3);
      WAIT_STATE(ch,skill_table[gsn_envenom].beats);
      return;
    }
    else
    {
      act("You fail to envenom $p.",ch,obj,NULL,TO_CHAR);
      check_improve(ch,gsn_envenom,FALSE,3);
      if (toxic)
        check_improve(ch,gsn_toxicology,FALSE,3);
      WAIT_STATE(ch,skill_table[gsn_envenom].beats);
      return;
    }
  }

  act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
  return;
}

void do_fill( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *fountain;
  bool found;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' )
  {
    send_to_char( "Fill what?\n\r", ch );
    return;
  }
  if (IS_GHOST(ch))
  {
    send_to_char("Filling something is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
  {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }

  found = FALSE;
  for ( fountain = ch->in_room->contents; fountain != NULL;
        fountain = fountain->next_content )
  {
    if ( ( fountain->item_type == ITEM_FOUNTAIN )  &&
         ( (arg2[0] == '\0') || ( liq_lookup(arg2) == fountain->value[2] ) ) )
    {
      found = TRUE;
      break;
    }
  }

  if ( !found )
  {
    if (arg2[0] == '\0') send_to_char( "There is no fountain here.\n\r", ch );
    else send_to_char( "There is no fountain of that kind here.\n\r", ch );
    return;
  }

  /*if ( ( fountain = get_obj_here( ch, arg2 ) ) == NULL )
    {
      send_to_char( "There is nothing like that here from which to fill your container.\n\r", ch);
      return;
    }

  if ( fountain->item_type != ITEM_FOUNTAIN )
    {
       send_to_char( "You can't fill it from that.\n\r", ch);
       return;
    }*/

  if ( obj->item_type != ITEM_DRINK_CON )
  {
    send_to_char( "You can't fill that.\n\r", ch );
    return;
  }

  if ( obj->value[1] != 0 && obj->value[2] != fountain->value[2] )
  {
    send_to_char( "There is already another liquid in it.\n\r", ch );
    return;
  }

  if ( obj->value[1] >= obj->value[0] )
  {
    send_to_char( "Your container is full.\n\r", ch );
    return;
  }

  mprintf(sizeof(buf), buf,"You fill $p with %s from $P.",
          liq_table[fountain->value[2]].liq_name);
  act( buf, ch, obj,fountain, TO_CHAR );
  mprintf(sizeof(buf), buf,"$n fills $p with %s from $P.",
          liq_table[fountain->value[2]].liq_name);
  act(buf,ch,obj,fountain,TO_ROOM);
  obj->value[2] = fountain->value[2];
  obj->value[1] = obj->value[0];
  return;
}

void do_pour (CHAR_DATA *ch, char *argument)
{
  char arg[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
  OBJ_DATA *out, *in;
  CHAR_DATA *vch = NULL;
  int amount;

  argument = one_argument(argument,arg);

  if (arg[0] == '\0' || argument[0] == '\0')
  {
    send_to_char("Pour what into what?\n\r",ch);
    return;
  }


  if ((out = get_obj_carry(ch,arg, ch)) == NULL)
  {
    send_to_char("You don't have that item.\n\r",ch);
    return;
  }

  if (out->item_type != ITEM_DRINK_CON)
  {
    send_to_char("That's not a drink container.\n\r",ch);
    return;
  }

  if (!str_cmp(argument,"out"))
  {
    if (out->value[1] == 0)
    {
      send_to_char("It's already empty.\n\r",ch);
      return;
    }

    out->value[1] = 0;
    out->value[3] = 0;
    mprintf(sizeof(buf), buf,"You invert $p, spilling %s all over the ground.",
            liq_table[out->value[2]].liq_name);
    act(buf,ch,out,NULL,TO_CHAR);

    mprintf(sizeof(buf), buf,"$n inverts $p, spilling %s all over the ground.",
            liq_table[out->value[2]].liq_name);
    act(buf,ch,out,NULL,TO_ROOM);
    return;
  }

  if ((in = get_obj_here(ch,argument)) == NULL)
  {
    vch = get_char_room(ch,argument);

    if (vch == NULL)
    {
      send_to_char("Pour into what?\n\r",ch);
      return;
    }

    in = get_eq_char(vch,WEAR_HOLD);

    if (in == NULL)
    {
      send_to_char("They aren't holding anything.",ch);
      return;
    }
  }

  if (in->item_type != ITEM_DRINK_CON)
  {
    send_to_char("You can only pour into other drink containers.\n\r",ch);
    return;
  }

  if (in == out)
  {
    send_to_char("You cannot change the laws of physics!\n\r",ch);
    return;
  }

  if (in->value[1] != 0 && in->value[2] != out->value[2])
  {
    send_to_char("They don't hold the same liquid.\n\r",ch);
    return;
  }

  if (out->value[1] == 0)
  {
    act("There's nothing in $p to pour.",ch,out,NULL,TO_CHAR);
    return;
  }

  if (in->value[1] >= in->value[0])
  {
    act("$p is already filled to the top.",ch,in,NULL,TO_CHAR);
    return;
  }

  amount = UMIN(out->value[1],in->value[0] - in->value[1]);

  in->value[1] += amount;
  out->value[1] -= amount;
  in->value[2] = out->value[2];

  if (vch == NULL)
  {
    mprintf(sizeof(buf), buf,"You pour %s from $p into $P.",
            liq_table[out->value[2]].liq_name);
    act(buf,ch,out,in,TO_CHAR);
    mprintf(sizeof(buf), buf,"$n pours %s from $p into $P.",
            liq_table[out->value[2]].liq_name);
    act(buf,ch,out,in,TO_ROOM);
  }
  else
  {
    mprintf(sizeof(buf), buf,"You pour some %s for $N.",
            liq_table[out->value[2]].liq_name);
    act(buf,ch,NULL,vch,TO_CHAR);
    mprintf(sizeof(buf), buf,"$n pours you some %s.",
            liq_table[out->value[2]].liq_name);
    act(buf,ch,NULL,vch,TO_VICT);
    mprintf(sizeof(buf), buf,"$n pours some %s for $N.",
            liq_table[out->value[2]].liq_name);
    act(buf,ch,NULL,vch,TO_NOTVICT);

  }
}

void do_drink( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int amount;
  int liquid;

  one_argument( argument, arg );

  if (IS_GHOST(ch))
  {
    send_to_char("Drinking is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( arg[0] == '\0' )
  {
    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
    {
      if ( obj->item_type == ITEM_FOUNTAIN )
        break;
    }

    if ( obj == NULL )
    {
      send_to_char( "Drink what?\n\r", ch );
      return;
    }
  }
  else
  {
    if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
      send_to_char( "You can't find it.\n\r", ch );
      return;
    }
  }

  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
  {
    send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
    return;
  }

  switch ( obj->item_type )
  {
    default:
      send_to_char( "You can't drink from that.\n\r", ch );
      return;

    case ITEM_FOUNTAIN:
      if ( ( liquid = obj->value[2] )  < 0 )
      {
        bug( "Do_drink: bad liquid number %d.", liquid );
        liquid = obj->value[2] = 0;
      }
      amount = liq_table[liquid].liq_affect[4] * 3;
      break;

    case ITEM_DRINK_CON:
      if ( obj->value[1] <= 0 )
      {
        send_to_char( "It is already empty.\n\r", ch );
        return;
      }

      if ( ( liquid = obj->value[2] )  < 0 )
      {
        bug( "Do_drink: bad liquid number %d.", liquid );
        liquid = obj->value[2] = 0;
      }

      amount = liq_table[liquid].liq_affect[4];
      amount = UMIN(amount, obj->value[1]);
      break;
  }
  if (!IS_NPC(ch) && !IS_IMMORTAL(ch)
      &&  ch->pcdata->condition[COND_FULL] > 45)
  {
    send_to_char("You're too full to drink more.\n\r",ch);
    return;
  }

  act( "$n drinks $T from $p.",
       ch, obj, liq_table[liquid].liq_name, TO_ROOM );
  act( "You drink $T from $p.",
       ch, obj, liq_table[liquid].liq_name, TO_CHAR );

  if ( ( liquid == liq_lookup("blood") ) && ( IS_VAMPIRE(ch) ) && (!IS_NPC(ch)) )
  {
    gain_condition( ch, COND_THIRST, 20 );
    gain_condition( ch, COND_HUNGER, 20 );
    gain_condition( ch, COND_FULL,    5 );
  }
  else if ( ( liquid != liq_lookup("blood") ) && ( IS_VAMPIRE(ch) ) && (!IS_NPC(ch) ) )
    send_to_char( "That didn't do you any good at all. Try drinking blood.\n\r", ch );
  else
  {
    gain_condition( ch, COND_DRUNK,
                    amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36 );
    gain_condition( ch, COND_FULL,
                    amount * liq_table[liquid].liq_affect[COND_FULL] / 4 );
    gain_condition( ch, COND_THIRST,
                    amount * liq_table[liquid].liq_affect[COND_THIRST] / 2 );
    gain_condition(ch, COND_HUNGER,
                   amount * liq_table[liquid].liq_affect[COND_HUNGER] / 10 );
  }

  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
    send_to_char( "You feel drunk.\n\r", ch );
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
    send_to_char( "You are full.\n\r", ch );
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
    send_to_char( "You quench your thirst.\n\r", ch );

  if ( obj->value[3] != 0 )
  {
    /* The drink was poisoned ! */
    AFFECT_DATA af;

    act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You choke and gag.\n\r", ch );
    af.where     = TO_AFFECTS;
    af.type      = gsn_poison;
    af.level   = number_fuzzy(amount);
    af.duration  = 3 * amount;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_POISON;
    affect_join( ch, &af );
  }

  if (obj->value[0] > 0)
    obj->value[1] -= amount;

  return;
}



void do_eat( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {
    send_to_char( "Eat what?\n\r", ch );
    return;
  }
  if (IS_GHOST(ch))
  {
    send_to_char("Eating is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
  {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }

  if ( !IS_IMMORTAL(ch) )
  {
    if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
    {
      send_to_char( "That's not edible.\n\r", ch );
      return;
    }

    if ((obj->level > ch->level) && (obj->item_type == ITEM_PILL))
    {
      send_to_char( "It is too hard for you to eat.\n\r",ch);
      return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 )
    {
      send_to_char( "You are too full to eat more.\n\r", ch );
      return;
    }
  }

  act( "$n eats $p.",  ch, obj, NULL, TO_ROOM );
  act( "You eat $p.", ch, obj, NULL, TO_CHAR );

  switch ( obj->item_type )
  {

    case ITEM_FOOD:
      if ( !IS_NPC(ch) )
      {
        int condition;

        condition = ch->pcdata->condition[COND_HUNGER];
        gain_condition( ch, COND_HUNGER, obj->value[0] );
        gain_condition( ch, COND_FULL, obj->value[1]);
        if ( condition == 0 && ch->pcdata->condition[COND_HUNGER] > 0 )
          send_to_char( "You are no longer hungry.\n\r", ch );
        else if ( ch->pcdata->condition[COND_FULL] > 40 )
          send_to_char( "You are full.\n\r", ch );
      }

      if ( obj->value[3] != 0 )
      {
        /* The food was poisoned! */
        AFFECT_DATA af;

        act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
        send_to_char( "You choke and gag.\n\r", ch );

        af.where   = TO_AFFECTS;
        af.type      = gsn_poison;
        af.level    = number_fuzzy(obj->value[0]);
        af.duration  = 2 * obj->value[0];
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF_POISON;
        affect_join( ch, &af );
      }
      break;

    case ITEM_PILL:
      obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL , FALSE);
      obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL , FALSE);
      obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL , FALSE);
      obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL , FALSE);
      break;
  }

  extract_obj( obj );
  return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
  OBJ_DATA *obj;

  if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
    return TRUE;

  if ( !fReplace )
    return FALSE;

  if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) && !IS_IMMORTAL(ch)  )
  {
    act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
    return FALSE;
  }

  unequip_char( ch, obj );
  if ( IS_SET( obj->pIndexData->mprog_flags, TRIG_REMOVE ) )
    ap_percent_trigger( obj, ch, NULL, NULL, TRIG_REMOVE, OBJ_PROG );
  else
  {
    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
  }
  return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace )
{
  AFFECT_DATA *paf;
  if ( ch->level < obj->level)
  {
    printf_to_char(ch, "You must be level %d to use this object.\n\r",
                   obj->level );
    act( "$n tries to use $p, but is too inexperienced.",
         ch, obj, NULL, TO_ROOM );
    return;
  }
  if (is_sanc_spelled(ch))
  {
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
      if (paf->where == TO_AFFECTS && paf->bitvector ==  AFF_SANCTUARY)
        return;
  }

  if ( obj->item_type == ITEM_LIGHT )
  {
    if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
      return;

    act( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
    act( "You light $p and hold it.",  ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_LIGHT );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
  {
    if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
         &&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
         &&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
         &&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
      return;

    if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
    {
      act( "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM );
      act( "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_FINGER_L );
      return;
    }

    if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
    {
      act( "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM );
      act( "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_FINGER_R );
      return;
    }

    bug( "Wear_obj: no free finger.", 0 );
    send_to_char( "You already wear two rings.\n\r", ch );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
  {
    if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
         &&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
         &&   !remove_obj( ch, WEAR_NECK_1, fReplace )
         &&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
      return;

    if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
    {
      act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
      act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_NECK_1 );
      return;
    }

    if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
    {
      act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
      act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_NECK_2 );
      return;
    }

    bug( "Wear_obj: no free neck.", 0 );
    send_to_char( "You already wear two neck items.\n\r", ch );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
  {
    if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
      return;

    act( "$n wears $p on $s body.",   ch, obj, NULL, TO_ROOM );
    act( "You wear $p on your body.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_BODY );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_CREST ) )
  {
    if ( !remove_obj( ch, WEAR_CREST, fReplace ) )
      return;
    act( "$n wears $p as $s clan crest.",   ch, obj, NULL, TO_ROOM );
    act( "You wear $p as your crest.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_CREST );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
  {
    if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
      return;
    act( "$n wears $p on $s head.",   ch, obj, NULL, TO_ROOM );
    act( "You wear $p on your head.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_HEAD );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
  {
    if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
      return;
    act( "$n wears $p on $s legs.",   ch, obj, NULL, TO_ROOM );
    act( "You wear $p on your legs.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_LEGS );
    return;
  }

  if (CAN_WEAR( obj, ITEM_WEAR_BACK ) )
  {
    if ( !remove_obj( ch, WEAR_BACK, fReplace ) )
      return;
    act( "$n wears $p on $s back.",   ch, obj, NULL, TO_ROOM );
    act( "You wear $p on your back.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_BACK );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_EAR ) )
  {
    if ( get_eq_char( ch, WEAR_EAR_L ) != NULL
         &&   get_eq_char( ch, WEAR_EAR_R ) != NULL
         &&   !remove_obj( ch, WEAR_EAR_L, fReplace )
         &&   !remove_obj( ch, WEAR_EAR_R, fReplace ) )
      return;

    if ( get_eq_char( ch, WEAR_EAR_L ) == NULL )
    {
      act( "$n wears $p on $s ear.",   ch, obj, NULL, TO_ROOM );
      act( "You wear $p on your ear.", ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_EAR_L );
      return;
    }

    if ( get_eq_char( ch, WEAR_EAR_R ) == NULL )
    {
      act( "$n wears $p on $s ear.",   ch, obj, NULL, TO_ROOM );
      act( "You wear $p on your ear.", ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_EAR_R );
      return;
    }

    bug( "Wear_obj: no free ear.", 0 );
    send_to_char( "You already wear two ear items.\n\r", ch );
    return;
  }

  if (CAN_WEAR( obj, ITEM_WEAR_LAPEL) )
  {
    if ( !remove_obj( ch, WEAR_LAPEL, fReplace ) )
      return;
    act( "$n pins $p on $s lapel.",   ch, obj, NULL, TO_ROOM );
    act( "You pin $p to your lapel.",ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_LAPEL );
    return;
  }

  if (CAN_WEAR( obj, ITEM_WEAR_RFOOT ) && IS_TRUSTED(ch,CREATOR) )
  {
    if ( !remove_obj( ch, WEAR_RFOOT, fReplace ) )
      return;
    act( "$n sticks $p onto $s right stub.",  ch, obj, NULL, TO_ROOM );
    act( "You stick $p onto your right stub.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_RFOOT );
    return;
  }

  if (CAN_WEAR( obj, ITEM_WEAR_LFOOT ) && IS_TRUSTED(ch,CREATOR) )
  {
    if ( !remove_obj( ch, WEAR_LFOOT, fReplace ) )
      return;
    act( "$n wears $p on $s left foot.",  ch, obj, NULL, TO_ROOM );
    act( "You wear $p on your left foot.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_LFOOT );
    return;
  }

  if (CAN_WEAR( obj, ITEM_WEAR_EYE ) && IS_TRUSTED(ch,CREATOR) )
  {
    if ( !remove_obj( ch, WEAR_LEYE, fReplace ) )
      return;
    act( "$n wears $p over $s left eye.",  ch, obj, NULL, TO_ROOM );
    act( "You wear $p over your left eye.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_LEYE );
    return;
  }




  if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
  {
    if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
      return;
    act( "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
    act( "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_FEET );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
  {
    if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
      return;
    act( "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
    act( "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_HANDS );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
  {
    if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
      return;
    act( "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
    act( "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_ARMS );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
  {
    if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
      return;
    act( "$n wears $p about $s torso.",   ch, obj, NULL, TO_ROOM );
    act( "You wear $p about your torso.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_ABOUT );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
  {
    if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
      return;
    act( "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
    act( "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_WAIST );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
  {
    if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
         &&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
         &&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
         &&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
      return;

    if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
    {
      act( "$n wears $p around $s left wrist.",
           ch, obj, NULL, TO_ROOM );
      act( "You wear $p around your left wrist.",
           ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_WRIST_L );
      return;
    }

    if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
    {
      act( "$n wears $p around $s right wrist.",
           ch, obj, NULL, TO_ROOM );
      act( "You wear $p around your right wrist.",
           ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_WRIST_R );
      return;
    }

    bug( "Wear_obj: no free wrist.", 0 );
    send_to_char( "You already wear two wrist items.\n\r", ch );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
  {
    OBJ_DATA *weapon;

    if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
      return;

    weapon = get_eq_char(ch,WEAR_WIELD);

    if (weapon != NULL
        && ch->size < SIZE_LARGE
        &&  IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS)
        && ch->gameclass != cBarbarian)
    {
      send_to_char("Your hands are tied up with your weapon!\n\r",ch);
      return;
    }

    if (get_eq_char (ch, WEAR_SECONDARY))
    {
      send_to_char ("You cannot use a shield while using two weapons.\n\r",ch);
      return;
    }

    act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
    act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_SHIELD );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WIELD ) )
  {
    int sn,skill;

    if ((!IS_NPC(ch)
         && IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
         && (get_eq_char(ch,WEAR_SHIELD) != NULL
             || get_eq_char(ch,WEAR_SECONDARY) != NULL
             || get_eq_char(ch, WEAR_HOLD) != NULL))
        && (ch->gameclass != cBarbarian)
        &&   ch->size < SIZE_LARGE )
    {
      send_to_char("You need two hands free for that weapon.\n\r",ch);
      return;
    }

    if ( !remove_obj( ch, WEAR_WIELD, fReplace ) )
      return;

    /* A second check is needed to ensure a secondary weapon didn't move into
       the WEAR_WIELD slot. */
    if ( !remove_obj( ch, WEAR_WIELD, fReplace ) )
      return;

    if ( !IS_NPC(ch)
         &&   get_obj_weight(obj) > (str_app[get_curr_stat(ch,STAT_STR)].wield * 10))
    {
      send_to_char( "It is too heavy for you to wield.\n\r", ch );
      return;
    }

    /*          if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
      {
    send_to_char ("You cannot wield a primary weopn before you remove your secondary.\n\r",ch);
    return;
    }*/
    if ( ( !IS_NPC( ch )
           &&   IS_WEAPON_STAT( obj, WEAPON_TWO_HANDS )
           &&   ( get_eq_char( ch, WEAR_SHIELD ) != NULL
                  ||    get_eq_char( ch, WEAR_SECONDARY ) != NULL ) )
         &&   ch->size < SIZE_LARGE )
    {
      send_to_char("You need two hands free for that weapon.\n\r",ch);
      return;
    }

    if (IS_WEAPON_STAT(obj,WEAPON_CLAN) && !IS_IN_CLAN( ch ) )
    {
      send_to_char("The item burns you..\n\r",ch);
      return;
    }

    act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
    act( "You wield $p.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_WIELD );

    sn = get_weapon_sn(ch);

    if (sn == gsn_hand_to_hand)
      return;

    skill = get_weapon_skill(ch,sn);

    if (skill >= 100)
      act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
    else if (skill > 85)
      act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
    else if (skill > 70)
      act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
    else if (skill > 50)
      act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
    else if (skill > 25)
      act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
    else if (skill > 1)
      act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
    else
      act("You don't even know which end is up on $p.",
          ch,obj,NULL,TO_CHAR);

    return;
  }

  if ( CAN_WEAR( obj, ITEM_HOLD ) )
  {
    if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
      return;

    if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
    {
      send_to_char ("You cannot hold an item while using two weapons.\n\r",ch);
      return;
    }

    if (get_eq_char(ch, WEAR_WIELD) != NULL)
    {
      if ( IS_WEAPON_STAT( get_eq_char( ch, WEAR_WIELD), WEAPON_TWO_HANDS )
           && ch->size < SIZE_LARGE)
      {
        send_to_char ("You cannot hold an item while wielding a two-handed weapon.\n\r", ch);
        return;
      }
    }

    act( "$n holds $p in $s hand.",   ch, obj, NULL, TO_ROOM );
    act( "You hold $p in your hand.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_HOLD );
    return;
  }

  if ( CAN_WEAR(obj,ITEM_WEAR_FLOAT) )
  {
    if (!remove_obj(ch,WEAR_FLOAT, fReplace) )
      return;
    act("$n releases $p to float next to $m.",ch,obj,NULL,TO_ROOM);
    act("You release $p and it floats next to you.",ch,obj,NULL,TO_CHAR);
    equip_char(ch,obj,WEAR_FLOAT);
    return;
  }
  if ( CAN_WEAR( obj, ITEM_WEAR_BAG ) )
  {
    if ( !remove_obj( ch, WEAR_BAG, fReplace ) )
      return;
    act( "$n wears $p on $s bag hook.",   ch, obj, NULL, TO_ROOM );
    act( "You wear $p on your bag hook.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_BAG );
    return;
  }

  if ( fReplace )
    send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

  return;
}



void do_wear( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *second;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Wear, wield, or hold what?\n\r", ch );
    return;
  }

  if (IS_AFFECTED(ch, AFF_BLIND))
  {
    send_to_char("You cannot see anything.\n\r",ch);
    return;
  }

  if ( !str_cmp( arg, "all" ) )
  {
    OBJ_DATA *obj_next;

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
      obj_next = obj->next_content;

      if (obj->next_content == obj)
      {
        bugf("infinite obj loop. %s",interp_cmd);
        obj_next = NULL;
      }

      if ( obj->owner
           &&  !IS_NULLSTR(obj->owner)
           &&   strcmp( ch->name, obj->owner )
           &&  !IS_IMMORTAL( ch ) )
      {
        send_to_char( "That belongs to someone else.\n\r", ch );
        continue;
      }

      if ( !can_use_clan_obj( ch, obj ) )
      {
        send_to_char( "That belongs to another clan.\n\r", ch );
        continue;
      }

      if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
      {
        wear_obj( ch, obj, FALSE );
        if ( obj->wear_loc != WEAR_NONE
             && IS_SET( obj->pIndexData->mprog_flags, TRIG_WEAR ) )
          ap_percent_trigger( obj, ch, NULL, NULL, TRIG_WEAR, OBJ_PROG );
      }
    }

    return;
  }
  else
  {
    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
      send_to_char( "You do not have that item.\n\r", ch );
      return;
    }

    if ( obj->owner
         &&  !IS_NULLSTR(obj->owner)
         &&   strcmp( ch->name, obj->owner )
         &&  !IS_IMMORTAL( ch ) )
    {
      send_to_char( "That belongs to someone else.\n\r", ch );
      return;
    }

    if ( !can_use_clan_obj( ch, obj ) )
    {
      send_to_char( "That belongs to another clan.\n\r", ch );
      return;
    }

    /* check if the secondary weapon is at least half as light as
    the primary weapon */
    if (obj->item_type == ITEM_WEAPON)
    {
      if ((second = get_eq_char(ch,WEAR_SECONDARY)) != NULL)
      {
        if (cBarbarian != ch->gameclass)
        {
          if ( (get_obj_weight (obj)/2) < get_obj_weight(second))
          {
            send_to_char (
              "Your secondary weapon has to be considerably lighter than the primary one.\n\r", ch);
            return;
          }

          if (IS_WEAPON_STAT(second, WEAPON_TWO_HANDS))
          {
            send_to_char("You cannot use a primary weapon while wielding a two-handed weapon.\n\r",ch);
            return;
          }
        }
      }
    }

    wear_obj( ch, obj, TRUE );

    if ( obj->wear_loc == WEAR_NONE )
      return;

    if ( IS_SET( obj->pIndexData->mprog_flags, TRIG_WEAR ) )
      ap_percent_trigger( obj, ch, NULL, NULL, TRIG_WEAR, OBJ_PROG );
  }

  return;
}



void do_remove( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  char buf[MSL];
  int iWear;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Remove what?\n\r", ch );
    return;
  }
  strcpy(buf,capitalize(arg));
  if (!strcmp(buf,"All"))
  {
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
      if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
        continue;
      remove_obj(ch, obj->wear_loc, TRUE);
    }

    if ( ( obj = get_eq_char(ch, WEAR_WIELD)) != NULL )
      remove_obj(ch, obj->wear_loc, TRUE);

    return;
  }

  if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
  {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }

  remove_obj( ch, obj->wear_loc, TRUE );
  return;
}


// Changes made here must be made in do_destroy too!
void do_sacrifice( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int silver;

  CHAR_DATA *gch;
  int members;
  char buffer[100];

  one_argument( argument, arg );

  obj = get_obj_list( ch, arg, ch->in_room->contents );

  if ( obj == NULL )
  {
    send_to_char( "You can't find it.\n\r", ch );
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("Sacrificing is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  /*
   * Don't allow saccing if the object contains anything unless it is an NPC
   * corpse.
   */
  //if ( obj->item_type != ITEM_CORPSE_NPC && obj->contains)
  if ( obj->contains ) //Laur wants this for any corpse/container - Aarchane
  {
    if ( ch->clan )
      act( "$g thinks that would be unwise.", ch, NULL, NULL, TO_CHAR );
    else
      act( "{CM{cir{Ml{mya{x thinks that would be unwise.",
           ch, NULL, NULL, TO_CHAR );
    return;
  }

  if (is_clan(ch))
  {
    do_function(ch, &do_clan_sac,argument);
    return;
  }

  one_argument( argument, arg );

  if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
  {
    act( "$n offers $mself to {CM{cir{Ml{mya{x, who graciously declines.",
         ch, NULL, NULL, TO_ROOM );
    send_to_char(
      "{CM{cir{Ml{mya{x appreciates your offer and may accept it later.\n\r", ch );
    return;
  }


  if ( obj->item_type == ITEM_CORPSE_PC )
  {
    if (obj->contains)
    {
      send_to_char(
        "{CM{cir{Ml{mya{x would not be pleased.\n\r",ch);
      return;
    }
  }

  if ( !CAN_WEAR(obj, ITEM_TAKE)
       ||   CAN_WEAR(obj, ITEM_NO_SAC))
  {
    act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
    return;
  }

  if ( !can_use_clan_obj( ch, obj ) )
  {
    send_to_char( "That belongs to another clan.\n\r", ch );
    return;
  }

  if (obj->in_room != NULL)
  {
    for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
      if (gch->on == obj)
      {
        act("$N appears to be using $p.",
            ch,obj,gch,TO_CHAR);
        return;
      }
    }
  }

  silver = UMAX(1,obj->level * 3);

  if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    silver = UMIN(silver,obj->cost);

  if (silver == 1)
    send_to_char(
      "{CM{cir{Ml{mya{x gives you one {wsi{Wl{Dv{wer{x coin for your sacrifice.\n\r", ch );
  else
  {
    printf_to_char(ch,"{CM{cir{Ml{mya{x gives you %d {wsi{Wl{Dv{wer{x coins for your sacrifice.\n\r",
                   silver);
  }

  ch->silver += silver;

  if (IS_SET(ch->act,PLR_AUTOSPLIT) )
  {
    /* AUTOSPLIT code */
    members = 0;
    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
      if ( is_same_group( gch, ch ) )
        members++;
    }

    if ( members > 1 && silver > 1)
    {
      mprintf(sizeof(buffer), buffer,"%d",silver);
      do_function(ch, &do_split,buffer);
    }
  }

  act( "$n sacrifices $p to {CM{cir{Ml{mya{x.", ch, obj, NULL, TO_ROOM );
  mprintf( sizeof(buf), buf, "%s sends up %s as a burnt offering.",
           capitalize( ch->name ),
           obj->short_descr );
  wiznet(buf, ch,NULL,WIZ_SACCING,0,0);
  extract_obj( obj );
  return;
}


void do_destroy( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int silver;

  CHAR_DATA *gch;
  int members;
  char buffer[100];

  one_argument( argument, arg );

  obj = get_obj_list( ch, arg, ch->in_room->contents );

  if ( obj == NULL )
  {
    send_to_char( "You can't find it.\n\r", ch );
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("Sacrificing is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if (is_clan(ch))
  {
    do_function(ch, &do_clan_sac,argument);
    return;
  }

  one_argument( argument, arg );

  if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
  {
    act( "$n offers $mself to {CM{cir{Ml{mya{x, who graciously declines.",
         ch, NULL, NULL, TO_ROOM );
    send_to_char( "{CM{cir{Ml{mya{x appreciates your offer and may accept it later.\n\r", ch );
    return;
  }


  if ( obj->item_type == ITEM_CORPSE_PC )
  {
    if (obj->contains)
    {
      send_to_char( "{CM{cir{Ml{mya{x would not be pleased.\n\r",ch);
      return;
    }
  }

  if ( !CAN_WEAR(obj, ITEM_TAKE)
       ||   CAN_WEAR(obj, ITEM_NO_SAC))
  {
    act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
    return;
  }

  if ( !can_use_clan_obj( ch, obj ) )
  {
    send_to_char( "That belongs to another clan.\n\r", ch );
    return;
  }

  if (obj->in_room != NULL)
  {
    for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
      if (gch->on == obj)
      {
        act("$N appears to be using $p.",
            ch,obj,gch,TO_CHAR);
        return;
      }
    }
  }

  silver = UMAX(1,obj->level * 3);

  if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    silver = UMIN(silver,obj->cost);

  if (silver == 1)
    send_to_char(
      "{CM{cir{Ml{mya{x gives you one {wsi{Wl{Dv{wer{x coin for your sacrifice.\n\r", ch );
  else
  {
    printf_to_char(ch,"{CM{cir{Ml{mya{x gives you %d {wsi{Wl{Dv{wer{x coins for your sacrifice.\n\r",
                   silver);
  }

  ch->silver += silver;

  if (IS_SET(ch->act,PLR_AUTOSPLIT) )
  {
    /* AUTOSPLIT code */
    members = 0;
    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
      if ( is_same_group( gch, ch ) )
        members++;
    }

    if ( members > 1 && silver > 1)
    {
      mprintf(sizeof(buffer), buffer,"%d",silver);
      do_function(ch, &do_split,buffer);
    }
  }

  act( "$n sacrifices $p to {CM{cir{Ml{mya{x.", ch, obj, NULL, TO_ROOM );
  mprintf( sizeof(buf), buf, "%s sends up %s as a burnt offering.",
           capitalize( ch->name ),
           obj->short_descr );
  wiznet(buf, ch,NULL,WIZ_SACCING,0,0);
  extract_obj( obj );
  return;
}

void do_junk( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  int jcount = 0;

  if ( !str_prefix( "all.", argument ) )
  {
    strcpy( arg, &argument[4] );
    while ( ( obj = get_obj_carry( ch, arg, ch ) ) != NULL )
    {
      jcount++;
      obj_from_char( obj );
      obj_to_room( obj, ch->in_room );
      do_function( ch, &do_sacrifice, arg );
      if (jcount >= 50)
      {
        send_to_char( "You can only junk 50 items at once.\n\r", ch );
        break;
      }
    }
    return;
  }

  if ( ( obj = get_obj_carry( ch, argument, ch ) ) == NULL )
  {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }

  if ( !can_drop_obj( ch, obj ) )
  {
    send_to_char( "You can't let go of it.\n\r", ch );
    return;
  }

  obj_from_char( obj );
  obj_to_room( obj, ch->in_room );

  if ( IS_OBJ_STAT( obj, ITEM_MELT_DROP ) )
  {
    act( "You drop $p and it dissolves into smoke.", ch, obj, NULL, TO_CHAR );
    act( "$n drops $p and it dissolves into smoke.", ch, obj, NULL, TO_ROOM );
    extract_obj( obj );
    return;
  }

  act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
  act( "You drop $p.", ch, obj, NULL, TO_CHAR );

  sprintf( buf, "\"%s\"", obj->name );
  do_function( ch, &do_sacrifice, buf );
}


void do_quaff( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  one_argument( argument, arg );

  if (IS_GHOST(ch))
  {
    send_to_char("Quaffing is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( arg[0] == '\0' )
  {
    send_to_char( "Quaff what?\n\r", ch );
    return;
  }

  if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
  {
    send_to_char( "You do not have that potion.\n\r", ch );
    return;
  }

  if ( obj->item_type != ITEM_POTION )
  {
    send_to_char( "You can quaff only potions.\n\r", ch );
    return;
  }

  if ( !can_use_clan_obj( ch, obj ) )
  {
    send_to_char( "That belongs to another clan.\n\r", ch );
    return;
  }

  if (ch->level < obj->level)
  {
    send_to_char("This liquid is too powerful for you to drink.\n\r",ch);
    return;
  }

  if (!IS_NPC(ch) && !IS_IMMORTAL(ch)
      &&  ch->pcdata->condition[COND_FULL] > 45)
  {
    send_to_char("You're too full to quaff anymore.\n\r",ch);
    return;
  }

  if ((obj->value[1] < 1)
      && (obj->value[2] < 1)
      && (obj->value[3] < 1)
      && (obj->value[4] < 1))
  {
    send_to_char("That vial is empty!\n\r",ch);
    return;
  }

  if ( ch->size < SIZE_LARGE )
    gain_condition( ch, COND_FULL, 2 );
  else
    gain_condition( ch, COND_FULL, 1 );
  act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
  act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );
  WAIT_STATE( ch, PULSE_VIOLENCE/4);

  obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL , FALSE);
  obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL , FALSE);
  obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL , FALSE);
  obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL , FALSE);

  extract_obj( obj );
  return;
}

void do_recite( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *scroll = NULL;
  OBJ_DATA *spellbook = NULL;
  OBJ_DATA *obj;
  int bypass = FALSE;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (IS_GHOST(ch))
  {
    send_to_char("Reciting is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if (arg2[0] != '\0')
  {
    spellbook = get_obj_carry( ch, arg2, ch );
    if ( ( spellbook != NULL )
         &&   ( spellbook->item_type == ITEM_SPELLBOOK ) )
    {
      scroll = get_obj_list( ch, arg1, spellbook->contains );

      if ( scroll
           && can_use_clan_obj( ch, scroll )
           && ( ch->level >= scroll->level ) )
        obj_from_obj( scroll );

      argument = one_argument( argument, arg2 ); // still might be a target in the arg list
    } // else arg2 could be a person (target)
  }

  if (!scroll) // must not have been in a spellbook, so try again
    scroll = get_obj_carry( ch, arg1, ch );

  if ( !scroll )
  {
    send_to_char( "You do not have that scroll.\n\r", ch );
    return;
  }

  if ( scroll->item_type != ITEM_SCROLL )
  {
    send_to_char( "You can recite only scrolls.\n\r", ch ); //  "or spellbooks?"
    return;
  }

  if ( !can_use_clan_obj( ch, scroll ) )
  {
    send_to_char( "That belongs to another clan.\n\r", ch );
    return;
  }

  if ( ch->level < scroll->level)
  {
    send_to_char(
      "This item is too complex for you to comprehend.\n\r",ch);
    return;
  }

  if (scroll->value[1] < 1)
  {
    send_to_char("That parchment is blank!\n\r",ch);
    return;
  }

  obj = NULL;
  if ( arg2[0] == '\0' )
  {
    victim = ch;
  }
  else
  {
    if (scroll->value[1] == skill_lookup("gate") ||
        scroll->value[1] == skill_lookup("summon") ||
        scroll->value[1] == skill_lookup("scry") ||
        scroll->value[1] == skill_lookup("portal") ||
        scroll->value[1] == skill_lookup("nexus") ||
        scroll->value[1] == skill_lookup("locate object"))
    {
      target_name = str_dup(arg2, target_name);
      victim = NULL;
      bypass = TRUE;
    }
    else
    {
      if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
           &&   ( obj = get_obj_here  ( ch, arg2 ) ) == NULL )
      {
        send_to_char( "You can't find it.\n\r", ch );
        return;
      }
      if (victim != NULL && IS_GHOST(victim))
      {
        send_to_char("Yeah, right.  In case you hadn't noticed, that person is a GHOST!\n\r", ch);
        return;
      }
    }
  }

  act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
  act( "You recite $p.", ch, scroll, NULL, TO_CHAR );
  WAIT_STATE( ch, PULSE_VIOLENCE/2);
  if (number_percent() >= 20 + get_skill(ch,gsn_scrolls) * 4/5)
  {
    send_to_char("You mispronounce a syllable.\n\r",ch);
    check_improve(ch,gsn_scrolls,FALSE,4);
  }

  else
  {
    obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim,
                    obj, bypass );
    obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim,
                    obj, bypass );
    obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim,
                    obj, bypass );
    obj_cast_spell( scroll->value[4], scroll->value[0], ch, victim,
                    obj, bypass );
    check_improve(ch,gsn_scrolls,TRUE,2);
  }

  extract_obj( scroll );
  return;
}



void do_brandish( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next, *tch;
  OBJ_DATA *staff=0, *staff1=0, *staff2=0;
  int sn;
  char arg[MSL];
  int tar = FALSE;
  bool isstaff = FALSE;

  if (IS_GHOST(ch))
  {
    send_to_char("Brandishing is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  argument = one_argument(argument, arg);

  if (arg[0] != '\0')
  {
    tar = TRUE;
    tch = get_char_room(ch, arg);
  }
  else
    tch = ch;

  if (( ( staff1 = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
      &&  ( ( staff2 = get_eq_char(ch, WEAR_WIELD ) ) == NULL))
  {
    send_to_char( "You hold nothing in your hands.\n\r", ch );
    return;
  }

  if (staff1 != NULL)
    if ( staff1->item_type == ITEM_STAFF )
    {
      isstaff = TRUE;
      staff = staff1;
    }

  if (staff2 != NULL)
    if ( staff2->item_type == ITEM_STAFF )
    {
      isstaff = TRUE;
      staff = staff2;
    }

  if (!isstaff)
  {
    send_to_char( "You can brandish only with a staff.\n\r", ch );
    return;
  }

  if ( !can_use_clan_obj( ch, staff ) )
  {
    send_to_char( "That belongs to another clan.\n\r", ch );
    return;
  }

  if ( ( sn = staff->value[3] ) < 0
       ||   sn >= MAX_SKILL
       ||   skill_table[sn].spell_fun == 0 )
  {
    send_to_char( "You cannot brandish an empty staff.\n\r", ch );
    bug( "Do_brandish: bad sn %d.", sn );
    return;
  }

  WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

  if ( staff->value[2] > 0 )
  {
    act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
    act( "You brandish $p.",  ch, staff, NULL, TO_CHAR );
    if ( ch->level < staff->level
         ||   number_percent() >= 20 + get_skill(ch,gsn_staves) * 4/5)
    {
      act ("You fail to invoke $p.",ch,staff,NULL,TO_CHAR);
      act ("...and nothing happens.",ch,NULL,NULL,TO_ROOM);
      check_improve(ch,gsn_staves,FALSE,2);
    }

    else for ( vch = ch->in_room->people; vch; vch = vch_next )
      {
        vch_next  = vch->next_in_room;
        if (!can_see(ch,vch))
          continue;
        switch ( skill_table[sn].target )
        {
          default:
            bug( "Do_brandish: bad target for sn %d.", sn );
            return;

          case TAR_IGNORE:
            if ( vch != ch )
              continue;
            break;

          case TAR_CHAR_OFFENSIVE:
            if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
              continue;
            break;

          case TAR_CHAR_DEFENSIVE:
            if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
              continue;
            break;

          case TAR_OBJ_CHAR_DEF:
            if (!tch)
            {
              send_to_char("The staff cannot find the target.\n\r",ch);
              return;
            }
            if (!tar)
            {
              if ( vch != ch )
                continue;
              break;
            }
            else
            {
              if ( tch != vch)
                continue;
              break;
            }
            break;
          case TAR_CHAR_SELF:
            if ( vch != ch )
              continue;
            break;
        }

        obj_cast_spell( staff->value[3], staff->value[0], ch, vch,
                        NULL , FALSE);
        check_improve(ch,gsn_staves,TRUE,2);
      }
  }

  if ( --staff->value[2] <= 0 )
  {
    act( "$p blazes brightly and is gone from $n's hand.", ch, staff, NULL, TO_ROOM );
    act( "$p blazes brightly and is gone.", ch, staff, NULL, TO_CHAR );
    extract_obj( staff );
  }

  return;
}



void do_zap( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *wand;
  OBJ_DATA *obj;

  if (IS_GHOST(ch))
  {
    send_to_char("Zapping is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }
  one_argument( argument, arg );
  if ( arg[0] == '\0' && ch->fighting == NULL )
  {
    send_to_char( "Zap whom or what?\n\r", ch );
    return;
  }

  if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
  {
    send_to_char( "You hold nothing in your hand.\n\r", ch );
    return;
  }

  if ( wand->item_type != ITEM_WAND )
  {
    send_to_char( "You can zap only with a wand.\n\r", ch );
    return;
  }

  if ( !can_use_clan_obj( ch, wand ) )
  {
    send_to_char( "That belongs to another clan.\n\r", ch );
    return;
  }

  obj = NULL;
  if ( arg[0] == '\0' )
  {
    if ( ch->fighting != NULL )
    {
      victim = ch->fighting;
    }
    else
    {
      send_to_char( "Zap whom or what?\n\r", ch );
      return;
    }
  }
  else
  {
    if ( ( victim = get_char_room ( ch, arg ) ) == NULL
         &&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )
    {
      send_to_char( "You can't find it.\n\r", ch );
      return;
    }
  }

  WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

  if ( wand->value[2] > 0 )
  {
    if ( victim != NULL )
    {
      if ( victim == ch )
      {
        act( "$n zaps $mself with $p.", ch, wand, victim, TO_NOTVICT );
        act( "You zap yourself with $p.", ch, wand, victim, TO_CHAR );
      }
      else
      {
        act( "$n zaps $N with $p.", ch, wand, victim, TO_NOTVICT );
        act( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
        act( "$n zaps you with $p.",ch, wand, victim, TO_VICT );
      }
    }
    else
    {
      act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
      act( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
    }

    if (ch->level < wand->level
        ||  number_percent() >= 20 + get_skill(ch,gsn_wands) * 4/5)
    {
      act( "Your efforts with $p produce only smoke and sparks.",
           ch,wand,NULL,TO_CHAR);
      act( "$n's efforts with $p produce only smoke and sparks.",
           ch,wand,NULL,TO_ROOM);
      check_improve(ch,gsn_wands,FALSE,2);
    }
    else
    {
      obj_cast_spell( wand->value[3], wand->value[0], ch, victim,
                      obj , FALSE);
      check_improve(ch,gsn_wands,TRUE,2);
    }
  }

  if ( --wand->value[2] <= 0 )
  {
    act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
    act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
    extract_obj( wand );
  }

  return;
}



void do_steal( CHAR_DATA *ch, char *argument )
{
  char buf  [MAX_STRING_LENGTH];
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  char bufname[MSL];
  int percent;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  {
    send_to_char( "Steal what from whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( victim == ch )
  {
    send_to_char( "That's pointless.\n\r", ch );
    return;
  }

  if (IS_SET(ch->in_room->room_flags, ROOM_ARENA))
  {
    send_to_char( "You cannot steal in an arena room.\n\r",ch);
    return;
  }

  if (is_safe(ch,victim, TRUE))
    return;

  if (check_killsteal(ch,victim))
    return;
  if ( victim->position == POS_FIGHTING )
  {
    send_to_char( "Do you really think you could do that now? \n\r"
                  "Wait until the battle is over before trying again.\n\r",ch);
    act( "$n's attempt to steal failed because of combat.\n\r", ch, NULL, victim, TO_VICT );
    return;
  }

  WAIT_STATE( ch, skill_table[gsn_steal].beats );
  percent  = number_percent();
  percent += 75;

  if (!IS_AWAKE(victim))
    percent -= 50;
  else if (!can_see(victim,ch))
    percent -= 20;

  /*
   * Stealing fails if:
   * - Highwayman fails the skill check
   * - NPC tried to steal from another NPC
   * - Highwayman is not clanned
   * - Highwayman is a player and not in victim's pk range
   */
  if ( ( percent > get_skill(ch,gsn_steal) ) ||
       ( IS_NPC(ch) && IS_NPC(victim) ) ||
       ( !is_clan(ch) ) ||
       ( !IS_NPC(ch)
         && !IS_NPC(victim)
         && !is_in_pk_range(victim, ch)))
  {
    /*
     * Failure.
     */
    send_to_char( "Oops.\n\r", ch );
    affect_strip(ch,gsn_sneak);
    REMOVE_BIT(ch->affected_by,AFF_SNEAK);

    act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT    );
    act( "$n tried to steal from $N.\n\r",  ch, NULL, victim,
         TO_NOTVICT );
    if (can_see(victim,ch))
      strcpy(bufname,ch->name);
    else
      strcpy(bufname,"Someone");
    switch (number_range(0,3))
    {
      case 0 :
        mprintf(sizeof(buf), buf, "%s is a lousy thief!", bufname );
        break;
      case 1 :
        mprintf(sizeof(buf), buf, "%s couldn't rob %s way out of a paper bag!",
                bufname,(ch->sex == 2) ? "her" : "his");
        break;
      case 2 :
        mprintf(sizeof(buf), buf,"%s tried to rob me!",bufname );
        break;
      case 3 :
        mprintf(sizeof(buf), buf,"Keep your hands out of there, %s!",bufname);
        break;
    }
    if (IS_AWAKE(victim))
      do_function(victim, &do_yell, buf );
    if ( !IS_NPC(ch) )
    {
      if ( IS_NPC(victim) )
      {
        check_improve(ch,gsn_steal,FALSE,2);
        multi_hit( victim, ch, TYPE_UNDEFINED );
      }
      else
      {
        if (!IS_SET(ch->act, PLR_THIEF))
        {
          if (is_in_pk_range(ch,victim) && is_in_pk_range( victim, ch))
          {
            send_to_char( "*** You are now a {RV{riolent {DT{rhief{x!! ***\n\r", ch );
            SET_BIT(ch->act, PLR_THIEF);
            if ( !IS_SET(ch->act, PLR_VIOLENT))
              SET_BIT(ch->act, PLR_VIOLENT);
            mprintf(sizeof(buf), buf,"{W%s is attempting to steal from %s{x",ch->name,victim->name);
            wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
            save_char_obj( ch, FALSE );
          }
        }

      }
    }
    if (ch->fighting != ch)
    {
      if (ch->fighting)
        stop_fighting(ch, FALSE);
      set_fighting(ch,victim);
    }
    if (victim->fighting != ch)
    {
      if (victim->fighting)
        stop_fighting(victim, FALSE);
      set_fighting(victim,ch);
    }
    if (!IS_AWAKE(victim))
      do_function(victim, &do_wake,"");
    return;
  }

  if ( !str_cmp( arg1, "coin"  )
       ||   !str_cmp( arg1, "coins" )
       ||   !str_cmp( arg1, "gold"  )
       ||   !str_cmp( arg1, "silver"))
  {
    int gold, silver;

    gold = victim->gold * number_range(1, ch->level) / MAX_LEVEL;
    silver = victim->silver * number_range(1,ch->level) / MAX_LEVEL;
    if ( gold <= 0 && silver <= 0 )
    {
      send_to_char( "You couldn't get any coins.\n\r", ch );
      return;
    }

    ch->gold       += gold;
    ch->silver     += silver;
    victim->silver   -= silver;
    victim->gold   -= gold;
    if (silver <= 0)
      mprintf(sizeof(buf), buf, "Score!  You got {Y%d {yg{Yo{yld{x coins.\n\r", gold );
    else if (gold <= 0)
      mprintf(sizeof(buf), buf, "Score!  You got {W%d {wsi{Wl{Dv{wer {xcoins.\n\r",silver);
    else
      mprintf(sizeof(buf), buf, "Score!  You got {W%d {wsi{Wl{Dv{wer{x and {Y%d {yg{Yo{yld{x coins.\n\r",
              silver,gold);

    send_to_char( buf, ch );
    check_improve(ch,gsn_steal,TRUE,2);
    return;
  }

  if ( ( obj = get_obj_carry( victim, arg1, ch ) ) == NULL )
  {
    send_to_char( "You can't find it.\n\r", ch );
    return;
  }

  if ( !can_drop_obj( ch, obj )
       ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
       ||   obj->level > ch->level )
  {
    send_to_char( "You can't pry it away.\n\r", ch );
    return;
  }

  if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
  {
    send_to_char( "You have your hands full.\n\r", ch );
    return;
  }

  if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
  {
    send_to_char( "You can't carry that much weight.\n\r", ch );
    return;
  }

  if (IS_SET(obj->extra_flags, ITEM_NODROP))
  {
    send_to_char("You cannot pull the item away from them\n\r",ch);
    return;
  }

  if ( !IS_NULLSTR( obj->pIndexData->clan_name )
       ||    IS_SET(obj->extra_flags, ITEM_RESTRING) )
  {
    send_to_char( "You can't take that item.\n\r", ch );
    return;
  }

  obj_from_char( obj );
  obj_to_char( obj, ch );
  act("You pocket $p.",ch,obj,NULL,TO_CHAR);
  check_improve(ch,gsn_steal,TRUE,2);
  send_to_char( "Got it!\n\r", ch );
  return;
}



/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *keeper;
  SHOP_DATA *pShop;

  pShop = NULL;
  for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
  {
    if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
      break;
  }

  if ( pShop == NULL )
  {
    send_to_char( "You can't do that here.\n\r", ch );
    return NULL;
  }

  /*
   * Undesirables.
   *
   if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_KILLER) )
   {
   do_function(keeper, &do_say, "Killers are not welcome!" );
   mprintf(sizeof(buf), buf, "%s the KILLER is over here!\n\r", ch->name );
   do_function(keeper, &do_yell, buf );
   return NULL;
   }
    
   if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_THIEF) )
   {
   do_function( keeper,&do_say, "Thieves are not welcome!" );
   mprintf(sizeof(buf), buf, "%s the THIEF is over here!\n\r", ch->name );
   do_function(keeper,&do_yell, buf );
   return NULL;
   }
  */

  if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_TWIT) )
  {
    do_function( keeper,&do_say, "Twits are not welcome!" );
    mprintf(sizeof(buf), buf, "%s the TWIT is over here!\n\r", ch->name );
    do_function(keeper,&do_yell, buf );
    return NULL;
  }
  /*
   * Shop hours.
   */
  if ( time_info.hour < pShop->open_hour )
  {
    do_function(keeper,&do_say, "Sorry, I am closed. Come back later." );
    return NULL;
  }

  if ( time_info.hour > pShop->close_hour )
  {
    do_function(keeper,&do_say, "Sorry, I am closed. Come back tomorrow." );
    return NULL;
  }

  /*
   * Invisible or hidden people.
   */
  if ( !can_see( keeper, ch )  && !IS_IMMORTAL(ch) )
  {
    do_function(keeper,&do_say, "I don't trade with folks I can't see." );
    return NULL;
  }

  return keeper;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper( OBJ_DATA *obj, CHAR_DATA *ch )
{
  OBJ_DATA *t_obj, *t_obj_next;

  /* see if any duplicates are found */
  for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next)
  {
    t_obj_next = t_obj->next_content;
    if (t_obj->next_content == t_obj)
    {
      bugf("infinite obj loop. %s",interp_cmd);
      t_obj_next = NULL;
    }

    if (obj->pIndexData == t_obj->pIndexData
        &&  !str_cmp(obj->short_descr,t_obj->short_descr))
    {
      /* if this is an unlimited item, destroy the new one */
      if (IS_OBJ_STAT(t_obj,ITEM_INVENTORY))
      {
        extract_obj(obj);
        return;
      }
      obj->cost = t_obj->cost; /* keep it standard */
      break;
    }
  }

  if (t_obj == NULL)
  {
    obj->next_content = ch->carrying;
    ch->carrying = obj;
  }
  else
  {
    obj->next_content = t_obj->next_content;
    t_obj->next_content = obj;
  }

  obj->carried_by      = ch;
  obj->in_room         = NULL;
  obj->in_obj          = NULL;
  ch->carry_number    += get_obj_number( obj );
  ch->carry_weight    += get_obj_weight( obj );
}

/* get an object from a shopkeeper's list */
OBJ_DATA *get_obj_keeper( CHAR_DATA *ch, CHAR_DATA *keeper, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;

  number = number_argument( argument, arg );
  count  = 0;
  for ( obj = keeper->carrying; obj != NULL; obj = obj->next_content )
  {
    if (obj->wear_loc == WEAR_NONE
        &&  can_see_obj( keeper, obj )
        &&  can_see_obj(ch,obj)
        &&  is_name( arg, obj->name ) )
    {
      if ( ++count == number )
        return obj;

      /* skip other objects of the same name */
      while (obj->next_content != NULL
             && obj->pIndexData == obj->next_content->pIndexData
             && !str_cmp(obj->short_descr,obj->next_content->short_descr))
        obj = obj->next_content;
    }
  }

  return NULL;
}

int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
  SHOP_DATA *pShop;
  int cost;

  if ( obj == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
    return 0;

  if ( fBuy )
  {
    cost = obj->cost * pShop->profit_buy  / 100;
  }
  else
  {
    OBJ_DATA *obj2;
    int itype;

    cost = 0;
    for ( itype = 0; itype < MAX_TRADE; itype++ )
    {
      if ( obj->item_type == pShop->buy_type[itype] )
      {
        cost = obj->cost * pShop->profit_sell / 100;
        break;
      }
    }

    if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
      for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
      {
        if ( obj->pIndexData == obj2->pIndexData
             &&   !str_cmp(obj->short_descr,obj2->short_descr) )
        {
          if (IS_OBJ_STAT(obj2,ITEM_INVENTORY))
            cost /= 2;
          else
            cost = cost * 3 / 4;
        }
      }
  }

  if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
  {
    if (obj->value[1] == 0)
      cost /= 4;
    else
      cost = cost * obj->value[2] / obj->value[1];
  }

  return cost;
}

bool check_similiar_name(char *str, char *name)
{
  int i;

  for ( i = 0; i < 2; i++ )
  {
    if ( ( str[i] ) && ( name[i] ) )
    {
      if ( LOWER( str[i] ) != LOWER( name[i] ) )
        return FALSE;
    }
    else
      return FALSE;
  }

  return TRUE;

}


void do_buy( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  int cost = 0, roll, steal_number, origcost = 0;
  int gold, silver;     /* Added for new_deduct_cost function RWLIII */
  char strsave[MSL];
  FILE *fp;

  if ( argument[0] == '\0' )
  {
    send_to_char( "Buy what?\n\r", ch );
    return;
  }

  if ( IS_AFFECTED( ch, AFF_BLIND ) && !IS_IMMORTAL( ch ) )
  {
    send_to_char( "You can't see anything to buy.\n\r", ch );
    return;
  }

  if ( IS_GHOST( ch ) )
  {
    send_to_char( "They will not deal with a GHOST.\n\r", ch );
    return;
  }

  if ( IS_SET( ch->in_room->room_flags, ROOM_PET_SHOP ) )
  {
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *pet;
    ROOM_INDEX_DATA *pRoomIndexNext;
    ROOM_INDEX_DATA *in_room;

    smash_tilde(argument);

    if ( IS_NPC( ch ) )
      return;

    argument = one_argument(argument,arg);

    pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
    if ( pRoomIndexNext == NULL )
    {
      bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
      send_to_char( "Sorry, you can't buy that here.\n\r", ch );
      return;
    }

    in_room     = ch->in_room;
    ch->in_room = pRoomIndexNext;
    pet         = get_char_room( ch, arg );
    ch->in_room = in_room;

    if ( pet == NULL || !IS_PET( pet ) )
    {
      send_to_char( "Sorry, you can't buy that here.\n\r", ch );
      return;
    }

    if ( IS_SET(ch->act,PLR_NOFOLLOW) )
    {
      send_to_char( "You do not currently accept followers.\n\r", ch );
      return;
    }

    if ( !IS_NPC( pet ) )
    {
      send_to_char("Purchasing Characters cannot be done.\n\r",ch);
      return;
    }
    if ( ch->pet )
    {
      send_to_char("You already own a pet.\n\r",ch);
      return;
    }

    cost = 10 * pet->level * pet->level;

    /* haggle */
    origcost = cost;
    if ( number_percent() < get_skill( ch, gsn_haggle ) )
      cost = cost * number_range(75,99) / 100;

    if ( ( ch->silver + 100 * ch->gold ) < cost )
    {
      send_to_char( "You can't afford it.\n\r", ch );
      return;
    }

    if ( ch->level < pet->level )
    {
      send_to_char( "You're not powerful enough to master this pet.\n\r", ch );
      return;
    }

    if (origcost != cost)
    {
      printf_to_char( ch, "You haggle the price down to %d coins.\n\r", cost );
      check_improve( ch, gsn_haggle, TRUE, 4 );
    }
    else
    {
      printf_to_char( ch, "You spent %d coins on your pet.\n\r", cost );
    }
    deduct_cost(ch,cost);

    pet = create_mobile( pet->pIndexData );

    SET_BIT( pet->act, ACT_PET );
    SET_BIT( pet->act, ACT_SENTINEL );

    SET_BIT( pet->affected_by, AFF_CHARM );

    pet->chan_flags = CHANNEL_NOTELL | CHANNEL_NOSHOUT;
    pet->pen_flags  = PEN_NOCHANNELS;

    argument = one_argument( argument, arg );
    if ( arg[0] != '\0')
    {
      if ( !is_name( arg, ch->name )
           &&   !check_similiar_name( arg, ch->name )
           &&    check_parse_name( arg )
           &&   !check_pre_player_names(arg)
           &&   !is_name( ch->name, arg ) )
      {
        mprintf( sizeof(strsave), strsave, "%s%s",
                 PLAYER_DIR, capitalize( arg ) );
        if ( ( fp = fopen( strsave, "r" ) ) == NULL )
        {
          mprintf(sizeof(buf), buf, "%s %s", pet->name, arg );
          free_string( pet->name );
          pet->name = str_dup( buf , pet->name);
#if MEMDEBUG
          pet->memdebug_name = str_dup(buf, pet->memdebug_name);
          memdebug_check(ch,"do_buy");
#endif
          send_to_char( "Your pet has just taken a name.\n\r", ch );
          mprintf(sizeof(buf), buf, "%sA neck tag says 'My name is %s'.\n\r",
                  pet->description, capitalize( arg ) );
          free_string( pet->description);
          pet->description = str_dup( buf, pet->description );
          //mprintf(sizeof(buf), buf,
          //        "%s's pet, %s named %s, stands ready.\n\r",
          //        ch->name, pet->short_descr, capitalize( arg ) );
          //free_string( pet->long_descr );
          //pet->long_descr = str_dup( buf, pet->long_descr );
        }
        else
        {
          nFilesOpen++;
          send_to_char( "Your pet refuses to answer to that name.\n\r",ch);
          fclose( fp );
          nFilesOpen--;
        }
      }
      else
        send_to_char( "Your pet refuses to answer to that name.\n\r", ch );
    }

    mprintf(sizeof(buf), buf, "%sA neck tag says 'I belong to %s'.\n\r",
            pet->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf , pet->description );

    char_to_room( pet, ch->in_room );
    add_follower( pet, ch );
    pet->group_num = ch->group_num;
    if ( ch->leader == NULL || ch->leader == ch )
      pet->leader = ch;
    else
    {
      pet->leader = ch->leader;
      act_new("$N joins your group.", ch->leader, NULL, pet,
              TO_CHAR,POS_SLEEPING );
    }
    if ( ch->clan )
    {
      free_string( pet->clan_name );
      pet->clan_name = str_dup( ch->clan_name, pet->clan_name );
      pet->clan = ch->clan;
    }
    ch->pet = pet;
    SET_BIT(pet->act,ACT_NOPURGE);
    SET_BIT(pet->act,ACT_NOGHOST);
    restore_char(ch->pet); // they were appearing with less than 100% HP
    send_to_char( "Enjoy your pet.\n\r", ch );
    act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
    return;
  }
  else
  {
    CHAR_DATA *keeper;
    OBJ_DATA *obj,*t_obj;
    char arg[MAX_INPUT_LENGTH];
    int number, count = 0, steal_check = 0;

    number = mult_argument(argument,arg);
    if ( number < 1 || number > 1000 ) return;

    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
      if ( ( IS_NPC(keeper) && (keeper->pIndexData->pShop != NULL ) )
           &&   ( ( obj = get_obj_keeper( ch,keeper, arg ) ) != NULL )
           &&   ( get_cost( keeper, obj, TRUE ) > 0 )
           &&   can_see_obj( ch, obj ) )
      {
        count++;
        break;
      }
    }

    if (count <= 0)
    {
      send_to_char( "That item is not available.\n\r", ch );
      return;
    }

    obj  = get_obj_keeper( ch,keeper, arg );
    cost = get_cost( keeper, obj, TRUE );

    /* haggle */
    origcost = cost;
    if ( !IS_SET( keeper->act2, ACT2_NOHAGGLE )
         && ( !IS_SET(keeper->in_room->area->flags, AREA_CLANHALL))
       )
    {
      if ( ( number_percent() < get_skill( ch, gsn_haggle ) )
           && ( obj->item_type != ITEM_GEM ) )
        cost = cost * number_range(75,99) / 100;
    }

    count = 1;
    if (number < 1 || number > 99)
    {
      act("{g $n tells you '{WGet real!{g'{x",keeper,NULL,ch,TO_VICT);
      return;
    }

    if (!IS_OBJ_STAT(obj,ITEM_INVENTORY))
    {
      for (t_obj = obj->next_content; count < number && t_obj != NULL;  t_obj = t_obj->next_content)
      {
        if (t_obj->pIndexData == obj->pIndexData
            &&  !str_cmp(t_obj->short_descr,obj->short_descr))
          count++;
        else
          break;
      }

      if (count < number)
      {
        act(" {g$n tells you '{WI don't have that many in stock{g'{x",
            keeper,NULL,ch,TO_VICT);
        return;
      }
    }

    if ( (ch->silver + ch->gold * 100) < cost * number )
    {
      if (number > 1)
        act(" {g$n tells you '{WYou can't afford to buy that many{g'{x",
            keeper,obj,ch,TO_VICT);
      else
        act( " {g$n tells you '{WYou can't afford to buy $p{g'{x",
             keeper, obj, ch, TO_VICT );

      return;
    }

    if ( obj->level > ch->level )
    {
      act( " {g$n tells you '{WYou can't use $p yet{g'{x",
           keeper, obj, ch, TO_VICT );
      return;
    }

    if ( !can_use_clan_obj( ch, obj ) )
    {
      act( " {g$n tells you '{WThat belongs to another clan.\n\r",
           keeper, obj, ch, TO_VICT );
      return;
    }

    if (ch->carry_number +  number * get_obj_number(obj) > can_carry_n(ch))
    {
      send_to_char( "You can't carry that many items.\n\r", ch );
      return;
    }

    if ( ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch))
    {
      send_to_char( "You can't carry that much weight.\n\r", ch );
      return;
    }

    if (cost != origcost)
    {
      act("You haggle with $N.",ch,NULL,keeper,TO_CHAR);
      check_improve(ch,gsn_haggle,TRUE,4);
    }
    /*
     * Steal begin
     *
     * If you buy 11 or more items at a time, you have a chance to
     * steal 1-4 more of the same kind. Chance is at most 25% each time.
     *
     */
    roll = number_percent();
    steal_number = number;
    if (number > 10
        && (roll < get_skill(ch,gsn_steal) / 4)
        && obj->item_type != ITEM_GEM
        && !IS_IMMORTAL(ch))
    {
      number += 1 + (roll % 3);

      while (ch->carry_number + number * get_obj_number(obj) > can_carry_n(ch))
      {
        steal_check = 1;
        number--;
      }

      if (!IS_OBJ_STAT(obj,ITEM_INVENTORY))
        while ( count < number )
        {
          steal_check = 2;
          number--;
        }

      switch (steal_check)
      {
        case 1:
          act("You tried to steal extra but couldn't carry it all!",
              ch, NULL, NULL, TO_CHAR);
          break;
        case 2:
          act("You steal the shopkeeper dry of that item!",
              ch, NULL, NULL, TO_CHAR);
          break;
        default:
          act("You steal some extra!",ch,NULL,NULL,TO_CHAR);
          break;
      }

      check_improve(ch,gsn_steal,TRUE,8);
    }
    else
      check_improve(ch,gsn_steal,FALSE,8);

    steal_number = number - steal_number;
    /*
     * Steal end
     */

    /* New cost deduction so we can add gold to purchases, and add some color */
    new_deduct_cost( ch, cost * (number - steal_number), &gold, &silver );

    keeper->gold    += gold;
    keeper->silver  += silver;

    if ( number > 1 )
    {
      sprintf( buf, "$n buys $p[%d].", number );
      act( buf, ch, obj, NULL, TO_ROOM );

      if ( gold == 0 )
        sprintf( buf, "You buy $p[%d] for {W%d {wsi{Wl{Dv{wer{x.",
                 number, silver );
      else if ( silver == 0 )
        sprintf( buf, "You buy $p[%d] for {Y%d {yg{Yo{yld{x.",
                 number, gold );
      else if ( silver < 0 )
        sprintf( buf,
                 "You buy $p[%d] for {Y%d {yg{Yo{yld{x and get {W%d {wsi{Wl{Dv{wer{x back.",
                 number, gold, UABS( silver ) );
      else
        sprintf( buf,
                 "You buy $p[%d] for {Y%d {yg{Yo{yld{x and {W%d {wsi{Wl{Dv{wer{x.",
                 number, gold, silver );
      act( buf, ch, obj, NULL, TO_CHAR );
    }
    else
    {
      act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
      if ( gold == 0 )
        sprintf( buf, "You buy $p for {W%d {wsi{Wl{Dv{wer{x.", silver );
      else if ( silver == 0 )
        sprintf( buf, "You buy $p for {Y%d {yg{Yo{yld{x.", gold );
      else if ( silver < 0 )
        sprintf( buf,
                 "You buy $p for {Y%d {yg{Yo{yld{x and get {W%d {wsi{Wl{Dv{wer{x back.",
                 gold, UABS( silver ) );
      else
        sprintf( buf, "You buy $p for {Y%d {yg{Yo{yld{x and {W%d {wsi{Wl{Dv{wer{x.",
                 gold, silver );
      act( buf, ch, obj, NULL, TO_CHAR );
    }


    for (count = 0; count < number; count++)
    {
      if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
        t_obj = create_object( obj->pIndexData, obj->level );
      else
      {
        t_obj = obj;
        obj = obj->next_content;
        obj_from_char( t_obj );
      }

      if (t_obj->timer > 0 && !IS_OBJ_STAT(t_obj,ITEM_HAD_TIMER))
        t_obj->timer = 0;

      REMOVE_BIT(t_obj->extra_flags,ITEM_HAD_TIMER);

      //if (IS_WEAPON_STAT(t_obj, WEAPON_CLAN))
      //{
      //  if (!strstr(t_obj->short_descr,ch->name))
      //  {
      //    mprintf(sizeof(buf), buf,"%s's %s",ch->name,t_obj->pIndexData->short_descr);
      //    free_string(t_obj->short_descr);
      //    t_obj->short_descr = str_dup(buf, t_obj->short_descr);
      //    mprintf(sizeof(buf), buf,"%s lies at your feet.",t_obj->pIndexData->short_descr);
      //    free_string(t_obj->description);
      //    t_obj->description = str_dup(buf, t_obj->description);
      //  }
      //}

      obj_to_char( t_obj, ch );

      if (cost < t_obj->cost)
        t_obj->cost = cost;
    }
  }
#if MEMDEBUG
  memdebug_check(ch,"do_buy");
#endif
}



void do_list( CHAR_DATA *ch, char *argument )
{
  BUFFER *buffer = NULL;
  bool found = FALSE;

  if ( IS_GHOST( ch ) )
  {
    send_to_char("They will not deal with a GHOST.\n\r",ch);
    return;
  }

  if ( IS_AFFECTED(ch, AFF_BLIND ) && !IS_IMMORTAL( ch ) )
  {
    send_to_char( "You can't see anything for sale here.\n\r",ch);
    return;
  }

  if ( IS_SET( ch->in_room->room_flags, ROOM_PET_SHOP ) )
  {
    ROOM_INDEX_DATA *pRoomIndexNext;
    CHAR_DATA *pet;

    /* hack to make new thalos pets work */
    if ( ch->in_room->vnum == 9621 )
      pRoomIndexNext = get_room_index( 9706 );
    else
      pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );

    if ( pRoomIndexNext == NULL )
    {
      bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
      send_to_char( "You can't do that here.\n\r", ch );
      return;
    }

    found = FALSE;

    for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
    {
      if ( IS_PET(pet) && IS_NPC(pet))
      {
        if ( !found )
        {
          found = TRUE;
          send_to_char( "Pets for sale:\n\r", ch );
        }

        printf_to_char(ch, "[%2d] %8d - %s\n\r",
                       pet->level,
                       10 * pet->level * pet->level,
                       pet->short_descr );
      }
    }

    if ( !found ) send_to_char( "Sorry, we're out of pets right now.\n\r", ch );

    return;
  }
  else
  {
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    SHOP_DATA *pShop;
    int cost=0,count,firstshop=1;
    bool found = FALSE;
    char buf1[MSL];
    buffer = new_buf();

    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
      if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
      {
        found = TRUE;
        if (!firstshop) send_to_char("\n\r",ch);
        firstshop = 0;

        sprintf(buf1, "{W[{CLv Price Qty{W]{m %s's items{x\n\r",keeper->short_descr);
        send_to_char(buf1,ch);
        buffer = new_buf();

        for ( obj = keeper->carrying; obj; obj = obj->next_content )
        {
          if ( obj->wear_loc == WEAR_NONE
               &&   can_see_obj( ch, obj )
               &&   ( cost = get_cost( keeper, obj, TRUE ) ) > 0 )
          {
            found = TRUE;

            if ( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
            {
              if ( obj->item_type == ITEM_WEAPON )
              {
                if ( obj->value[4] == 0 )
                {
                  sprintf( buf1, "%s", " ");
                }
                else
                {
                  sprintf( buf1, "{w({m%s{w){x", flag_string( weapon_type2, obj->value[4] ) );
                }

                if ( IS_AFFECTED( ch, AFF_DETECT_MAGIC ) )
                {
                  bprintf( buffer, "{W[{C%2d %5d -- {W]{x %s {w({c%s{w) %s{x\n\r",
                           obj->level, cost,
                           obj->short_descr,
                           flag_string( weapon_class, obj->value[0] ),
                           buf1 );
                }
                else
                {
                  bprintf( buffer, "{W[{C%2d %5d -- {W]{x %s {w({c%s{w){x\n\r",
                           obj->level, cost,
                           obj->short_descr,
                           flag_string( weapon_class, obj->value[0] ) );
                }
              }
              else if ( obj->item_type == ITEM_ARMOR )
              {
                bprintf( buffer, "{W[{C%2d %5d -- {W]{x %s {w({y%s{w){x\n\r",
                         obj->level, cost,
                         obj->short_descr,
                         flag_string(wear_flags, obj->wear_flags&~(ITEM_TAKE|ITEM_NO_SAC) ) );
              }
              else
                bprintf( buffer, "{W[{C%2d %5d -- {W]{x %s{x\n\r",
                         obj->level, cost, obj->short_descr );
            }
            else
            {
              count = 1;

              while (obj->next_content != NULL
                     && obj->pIndexData == obj->next_content->pIndexData
                     && !str_cmp(obj->short_descr, obj->next_content->short_descr ) )
              {
                obj = obj->next_content;
                count++;
              }

              bprintf(buffer,"{W[{C%2d %5d %2d {W]{x %s{x\n\r",
                      obj->level,cost,count,obj->short_descr);
            }
          }
        }

        if ( !found ) send_to_char( "There is nothing in the shopkeeper's listing.\n\r", ch );
        else
        {
          page_to_char(buf_string(buffer),ch);
        }
        free_buf(buffer);
      } // MOB is a shopkeeper
    } // loop through people in room
    free_buf(buffer);
    if (!found)
      send_to_char( "There are no shopkeepers here.\n\r", ch );
    return;
  }
}



void do_sell( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *keeper;
  int gold, silver;           /* Added for new_clan_deduct function RWLIII */
  OBJ_DATA *obj;
  int cost,roll;
  OBJ_DATA *obj_next;
  int tcostgold = 0, tcostsilver = 0, count = 0;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Sell what?\n\r", ch );
    return;
  }
  if ( IS_GHOST( ch ) )
  {
    send_to_char( "They will not deal with a GHOST.\n\r", ch );
    return;
  }

  if ( ( keeper = find_keeper( ch ) ) == NULL )
    return;

  if ( !str_cmp( arg, "all" ) )
  {
    for ( obj = ch->carrying; obj; obj = obj_next)
    {
      obj_next = obj->next_content;
      if ( obj->next_content == obj)
      {
        bugf( "infinite obj loop. %s", interp_cmd );
        obj_next = NULL;
      }

      if ( can_drop_obj( ch, obj )
           && obj->wear_loc == WEAR_NONE
           && can_see_obj( keeper, obj )
           && ( ( cost = get_cost( keeper, obj, FALSE ) ) > 0 )
           && ( cost < ( keeper->silver + 100 * keeper->gold ) ) )
      {
        if ( !can_use_clan_obj( ch, obj ) )
        {
          act( " {g$n tells you '{WThat belongs to another clan.\n\r",
               keeper, obj, ch, TO_VICT );
          return;
        }

        tcostgold += cost/100;
        tcostsilver += cost - (cost/100) * 100;
        count++;
        ch->gold   += cost/100;
        ch->silver += cost - (cost/100) * 100;
        deduct_cost( keeper, cost );
        if ( keeper->gold < 0 )
          keeper->gold = 0;
        if ( keeper->silver < 0 )
          keeper->silver = 0;

        if ((keeper->silver + 100 * keeper->gold) == 0)
        {
          act( " {g$n tells you '{WI have run out of money for now!{x\n\r", keeper, obj, ch, TO_VICT );
          break;
        }

        if ( obj->item_type == ITEM_TRASH
             ||   IS_OBJ_STAT( obj, ITEM_SELL_EXTRACT ) )
        {
          extract_obj(obj);
        }
        else
        {
          printf_to_char( ch, "Selling %s...\n\r", obj->short_descr );
          obj_from_char( obj );
          if ( obj->timer )
            SET_BIT( obj->extra_flags, ITEM_HAD_TIMER );
          else
            obj->timer = number_range( 50, 100 );
          obj_to_keeper( obj, keeper );
        }
      }           /* salable item ifcheck                       */
    }           /* for loop */
    printf_to_char( ch,
                    "You sell %d items for {Y%d {yg{Yo{yld {xand {W%d {wsi{Wl{Dv{wer{x.\r\n",
                    count, tcostgold,tcostsilver);
    return;
  }               /* 'all' if check */

  if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
  {
    act( "$n tells you 'You don't have that item'.",
         keeper, NULL, ch, TO_VICT );
    return;
  }

  if ( !can_drop_obj( ch, obj ) )
  {
    send_to_char( "You can't let go of it.\n\r", ch );
    return;
  }

  if (!can_see_obj(keeper,obj))
  {
    act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
    return;
  }

  if ( !can_use_clan_obj( ch, obj ) )
  {
    act( " {g$n tells you '{WThat belongs to another clan.\n\r",
         keeper, obj, ch, TO_VICT );
    return;
  }

  if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
  {
    act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
    return;
  }

  if ( cost > (keeper-> silver + (100 * keeper->gold)) )
  {
    act("$n tells you 'I'm afraid I don't have enough wealth to buy $p.",
        keeper,obj,ch,TO_VICT);
    return;
  }

  //act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
  /* haggle */
  roll = number_percent();
  if ( !IS_SET( keeper->act2, ACT2_NOHAGGLE )
       && !IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT)
       && roll < get_skill(ch,gsn_haggle) )
  {
    send_to_char("You haggle with the shopkeeper.\n\r",ch);
    // Taeloch's brand new Haggle code
    cost += cost * roll / 400; // add up to 25% on top
    cost = UMIN(cost,(keeper->silver + 100 * keeper->gold)); // if keeper doesn't have that, empty his wallet

    /*
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN(cost,95 * get_cost(keeper,obj,TRUE) / 100);
        cost = UMIN(cost,(keeper->silver + 100 * keeper->gold));
        cost = (int)((cost > 1.15 * get_cost(keeper, obj, FALSE)) ? 1.15 * get_cost(keeper, obj, FALSE) : cost);
        // cost sometimes went up
        if (cost <= obj->cost)
          cost = (int) (obj->cost * 1.1);
    */
    check_improve(ch,gsn_haggle,TRUE,4);
  }
  /*
   * Steal begin
   *
   * If you sell something worth more than 1000 silver, you have a
   * chance to steal some more silver, max 250, average 125.
   * Chance is at most 25%
   */
  roll = number_percent();
  if (cost > 1000 &&
      (roll < (get_skill(ch,gsn_steal) / 4)))
  {
    cost += 10*roll;
    send_to_char("You manage to steal some extra {wsi{Wl{Dv{wers{x!\n\r",ch);
    cost = UMIN(cost,(keeper->silver + 100 * keeper->gold));
    check_improve(ch,gsn_steal,TRUE,8);
  }
  /*
   * Steal end
   */

  /* New function to get more indepth into gold and silver. */
  new_deduct_cost( keeper, cost, &gold, &silver );

  if ( GET_CARRY_WEIGHT(ch) + SILVER_WEIGHT(silver) + GOLD_WEIGHT(gold) > can_carry_w( ch ) )
  {
    act( "{g $n {gtells you '{WYou can't carry that much wealth!{g'{x",
         keeper, NULL, ch, TO_VICT );
    keeper->gold   += gold;
    keeper->silver += silver;
    return;
  }

  act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
  if ( gold == 0 )
    sprintf( buf, "You sell $p for {W%d {wsi{Wl{Dv{wer{x.", silver );
  else if ( silver == 0 )
    sprintf( buf, "You sell $p for {Y%d {yg{Yo{yld{x.", gold );
  else if ( silver < 0 )
    sprintf( buf,
             "You sell $p for {Y%d {yg{Yo{yld{x and give back {W%d {wsi{Wl{Dv{wer{x.",
             gold, UABS( silver ) );
  else
    sprintf( buf, "You sell $p for {Y%d {yg{Yo{yld{x and {W%d {wsi{Wl{Dv{wer{x.",
             gold, silver );
  act( buf, ch, obj, NULL, TO_CHAR );

  ch->gold   += gold;
  ch->silver += silver;

  if ( keeper->gold < 0 )
    keeper->gold = 0;
  if ( keeper->silver< 0)
    keeper->silver = 0;

  if ( obj->item_type == ITEM_TRASH || IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
  {
    extract_obj( obj );
  }
  else
  {
    obj_from_char( obj );
    if (obj->timer)
      SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
    else
      obj->timer = number_range(50,100);
    obj_to_keeper( obj, keeper );
  }

  return;
}



void do_value( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *keeper;
  OBJ_DATA *obj;
  int cost;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Value what?\n\r", ch );
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("They will not deal with a GHOST.\n\r",ch);
    return;
  }

  if ( ( keeper = find_keeper( ch ) ) == NULL )
    return;

  if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
  {
    act( "$n tells you 'You don't have that item'.",
         keeper, NULL, ch, TO_VICT );
    return;
  }

  if (!can_see_obj(keeper,obj))
  {
    act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
    return;
  }

  if ( !can_drop_obj( ch, obj ) )
  {
    send_to_char( "You can't let go of it.\n\r", ch );
    return;
  }

  if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
  {
    act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
    return;
  }

  mprintf(sizeof(buf), buf,
          "$n tells you 'I'll give you {W%d {wsi{Wl{Dv{wer {xand {Y%d {yg{Yo{yld{x coins for $p'.",
          cost - (cost/100) * 100, cost/100 );
  act( buf, keeper, obj, ch, TO_VICT );

  return;
}

void do_second (CHAR_DATA *ch, char *argument)
/* wear object as a secondary weapon */
{
  OBJ_DATA *obj;
  OBJ_DATA *wield;

  if (get_skill(ch,gsn_second) < 1)
  {
    send_to_char("Sorry, you do NOT know how to hold two weapons.\n\r",ch);
    return;
  }

  if (argument[0] == '\0') /* empty */
  {
    send_to_char ("Wear which weapon in your off-hand?\n\r",ch);
    return;
  }

  obj = get_obj_carry (ch, argument, ch); /* find the obj withing ch's inventory */

  if (obj == NULL)
  {
    send_to_char ("You have no such thing on you.\n\r",ch);
    return;
  }


  /* check if the char is using a shield or a held weapon */

  if ( get_eq_char (ch,WEAR_SHIELD) != NULL)
  {
    send_to_char( "You cannot wear a shield and a secondary weapon.\n\r", ch );
    return;
  }

  if ( get_eq_char ( ch, WEAR_HOLD ) != NULL )
  {
    send_to_char ("You cannot use a secondary weapon while holding an item.\n\r",ch);
    return;
  }

  if (obj->item_type != ITEM_WEAPON )
  {
    send_to_char("You can only dual weapons.\n\r",ch);
    return;
  }

  if ( get_trust(ch) < obj->level )
  {
    printf_to_char(ch, "You must be level %d to use this object.\n\r",
                   obj->level );
    act( "$n tries to use $p, but is too inexperienced.",
         ch, obj, NULL, TO_ROOM );
    return;
  }

  /* check that the character is using a first weapon at all */
  if ( ( wield =get_eq_char (ch, WEAR_WIELD ) ) == NULL )
  {
    send_to_char ("You need to wield a primary weapon, before using a secondary one!\n\r",ch);
    return;
  }

  if ( ch->size < SIZE_LARGE )
  {
    if ( IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS))
    {
      send_to_char("You cannot use a two-handed weapon as a dual item.\n\r",ch);
      return;
    }

    if ( IS_WEAPON_STAT( wield, WEAPON_TWO_HANDS ) )
    {
      send_to_char("You cannot use a secondary weapon while wielding a two-handed weapon.\n\r",ch);
      return;
    }

    /* check for str - secondary weapons have to be lighter */
    if ( get_obj_weight( obj ) > ((str_app[get_curr_stat(ch,STAT_STR)].wield*10) / 2) )
    {
      send_to_char( "This weapon is too heavy for you.\n\r", ch );
      return;
    }

    /* MAke sure the secondary weapon doesn't weight more than the primary */
    if ( (get_obj_weight (obj) ) > get_obj_weight( get_eq_char (ch, WEAR_WIELD)) )
    {
      send_to_char ( "Your secondary weapon must be lighter than your primary.\n\r", ch );
      return;
    }
  }
  else
  {
    /* check for str - secondary weapons have to be lighter */
    if ( get_obj_weight( obj ) > ((str_app[get_curr_stat(ch,STAT_STR)].wield*10)) )
    {
      send_to_char( "This weapon is too heavy to be used as a secondary weapon by you.\n\r", ch );
      return;
    }

    /* MAke sure the secondary weapon doesn't weight the same as the primary */
    if ( (get_obj_weight (obj) ) > get_obj_weight( get_eq_char (ch, WEAR_WIELD)) )
    {
      send_to_char ( "Your secondary weapon must weigh the same or less than your primary.\n\r", ch );
      return;
    }
  }
  /* at last - the char uses the weapon */

  if (!remove_obj(ch, WEAR_SECONDARY, TRUE)) /* remove the current weapon if any */
    return;                                /* remove obj tells about any no_remove */

  /* char CAN use the item! that didn't take long at aaall */

  act ("$n wields $p in $s off-hand.",ch,obj,NULL,TO_ROOM);
  act ("You wield $p in your off-hand.",ch,obj,NULL,TO_CHAR);
  equip_char ( ch, obj, WEAR_SECONDARY);
  return;
}
/* Original Code by Todd Lair.                                        */
/* Improvements and Modification by Jason Huang (huangjac@netcom.com).*/
/* Permission to use this code is granted provided this header is     */
/* retained and unaltered.                                            */

void do_brew ( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH], arg1[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int sn, manamult, potion_level=0, i;
  bool adv_brew = FALSE;

  if ( IS_NPC( ch )
       || ch->level < skill_table[gsn_brew].skill_level[ch->gameclass] )
  {
    send_to_char( "You do not know how to brew potions.\n\r", ch );
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("Brewing is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if (IS_ROOM_SAFE(ch) || IS_ARENA(ch))
  {
    send_to_char("You fail due to the properties of this room.\n\r",ch);
    return;
  }
  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Brew what spell?\n\r", ch );
    return;
  }

  /* Do we have a vial to brew potions? */
  for ( obj = ch->carrying; obj; obj = obj->next_content )
  {
    if ( obj->item_type == ITEM_POTION && obj->wear_loc == WEAR_HOLD )
      break;
  }

  if ( !obj )
  {
    send_to_char( "You are not holding a vial.\n\r", ch );
    return;
  }

  if ( ( sn = find_spell(ch, arg) ) < 1
       || skill_table[sn].spell_fun == spell_null
       || ch->level < skill_table[sn].skill_level[ch->gameclass]
       || ch->pcdata->learned[sn] == 0)
  {
    send_to_char( "You don't know any spells by that name.\n\r", ch );
    return;
  }

  /* preventing potions of gas breath, acid blast, etc.; doesn't make sense
     when you quaff a gas breath potion, and then the mobs in the room are
     hurt. Those TAR_IGNORE spells are a mixed blessing. - JH */

  if ( ( skill_table[sn].target != TAR_CHAR_DEFENSIVE )
       &&   ( skill_table[sn].target != TAR_CHAR_SELF )
       &&   ( skill_table[sn].target != TAR_OBJ_CHAR_DEF ) )
  {
    if (!IS_CLASS_ALCHEMIST(ch))
    {
      send_to_char( "You cannot brew that spell.\n\r", ch );
      return;
    }

    for (i=1;i<5;i++)
    {
      if (obj->value[i] > 0)
      {
        send_to_char( "You require an empty vial for that spell.\n\r", ch );
        return;
      }
    }

// alchemist, offensive spell -- make it a bomb
    if ( ch->mana > skill_table[sn].min_mana )
    {
      if (is_affected( ch, skill_lookup( "clear head" ) ) )
        WAIT_STATE( ch, (skill_table[gsn_brew].beats*0.5) );
      else
        WAIT_STATE( ch, skill_table[gsn_brew].beats*0.75 );

      ch->mana -= skill_table[sn].min_mana;
      if ( number_percent() < ch->pcdata->learned[gsn_brew] )
      {
        char ana[3];
        obj->level = ch->level;
        obj->item_type = ITEM_PROJECTILE;
        obj->value[0] = ITEM_PROJECTILE_MAGICBOMB;
        obj->value[1] = 0;
        obj->value[2] = 0;
        obj->value[3] = sn;
        obj->value[4] = ch->level;
        check_improve(ch,gsn_brew,TRUE,4);

        if (IS_CLASS_ALCHEMIST(ch))
          check_improve(ch,sn,TRUE,1); // alchemists can't cast, have to improve somehow

        switch (skill_table[sn].name[0])
        {
          case 'a':
          case 'A':
          case 'e':
          case 'E':
          case 'i':
          case 'I':
          case 'o':
          case 'O':
          case 'u':
          case 'U':
            strcpy(ana,"an");
            break;
          default:
            strcpy(ana,"a");
            break;
        }

        if (strstr(skill_table[sn].name, " bomb"))
          mprintf(sizeof(arg1), arg1, "%s %s", ana, skill_table[sn].name);
        else
          mprintf(sizeof(arg1), arg1, "%s %s bomb", ana, skill_table[sn].name);

        free_string( obj->short_descr );
        obj->short_descr = str_dup(arg1, obj->short_descr );

        mprintf(sizeof(arg1), arg1, "bomb %s glass container",  skill_table[sn].name);
        free_string( obj->name );
        obj->name = str_dup(arg1, obj->name );

        free_string(obj->description );
        obj->description = str_dup("A glass container full of sparkling liquid is here.", obj->description );

        act( "You have created $p!", ch, obj, NULL, TO_CHAR );
        act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
        return;
      }
      else
      {
        act( "$p explodes violently!", ch, obj, NULL, TO_CHAR );
        act( "$p explodes violently!", ch, obj, NULL, TO_ROOM );
        obj_cast_spell( sn, (ch->level / 2), ch, ch, obj, FALSE );
        check_improve(ch,gsn_brew,FALSE,4);
        if (IS_VALID(obj)) // make sure the object wasn't destroyed
          extract_obj( obj );
        return;
      }
    }
    else
    {
      send_to_char( "You don't have enough mana.\n\r", ch );
      return;
    }
  } // offensive spells, alchemist

  if (get_skill(ch, gsn_advanced_brew) > number_percent())
  {
    argument = one_argument( argument, arg1);
    adv_brew = TRUE;
    if (arg1[0] == '\0' || (obj->value[1] != 0 && obj->value[1] != -1))
      potion_level = ch->level;
    else if (is_number(arg1))
    {
      potion_level = atoi(arg1);
      if (potion_level < 1 || potion_level > ch->level)
      {
        send_to_char("The potion's level must be between 1 and your level.\n\r", ch);
        return;
      }
    }
    else
    {
      send_to_char("SYNTAX: brew <spell> <scroll level>\n\r", ch);
      return;
    }
  }

  act( "$n begins preparing a potion.", ch, obj, NULL, TO_ROOM );
  if (IS_CLASS_ALCHEMIST(ch))
  {
    if (is_affected( ch, skill_lookup( "clear head" ) ) )
      WAIT_STATE( ch, (skill_table[gsn_brew].beats*0.5) );
    else
      WAIT_STATE( ch, skill_table[gsn_brew].beats*0.75 );
  }
  else
  {
    if (is_affected( ch, skill_lookup( "clear head" ) ) )
      WAIT_STATE( ch, (skill_table[gsn_brew].beats*0.75) );
    else
      WAIT_STATE( ch, skill_table[gsn_brew].beats );
  }

  if ( adv_brew )
    manamult = 6;
  else
    manamult = 4;

  /* Check the skill percentage, fcn(wis,int,skill) */
  if ( !IS_NPC(ch)
       && ( ch->mana > manamult * skill_table[sn].min_mana )
       && ( number_percent( ) > ch->pcdata->learned[gsn_brew] ))
  {
    ch->mana -= ((manamult/2) * skill_table[sn].min_mana);
    act( "$p explodes violently!", ch, obj, NULL, TO_CHAR );
    act( "$p explodes violently!", ch, obj, NULL, TO_ROOM );
    spell_hyracal_pressure(skill_lookup("hyracal pressure"), (ch->level/2), ch, ch, 0);
    check_improve(ch,gsn_brew,FALSE,4);
    if (IS_CLASS_ALCHEMIST(ch))
      check_improve(ch,sn,FALSE,1); // alchemists can't cast, have to improve somehow
    extract_obj( obj );
    return;
  }

  if (IS_CLASS_ALCHEMIST(ch))
    check_improve(ch,sn,TRUE,1); // alchemists can't cast, have to improve somehow

  if (adv_brew)
  {
    obj->level = potion_level;
    obj->value[0] = potion_level;
    check_improve(ch,gsn_advanced_brew,TRUE,4);
    if (spell_imprint(sn, ch->level, ch, obj, 3))
      check_improve(ch,gsn_brew,TRUE,4);
  }
  else
  {
    obj->level = ch->level/2;
    obj->value[0] = ch->level/3;
    check_improve(ch,gsn_advanced_brew,FALSE,1);
    if (spell_imprint(sn, ch->level, ch, obj, 0))
      check_improve(ch,gsn_brew,TRUE,4);
  }

}

void do_scribe ( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH], arg1[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int sn, manamult, scroll_level=0;
  int gn, fsn;
  bool adv_scribe = FALSE;

  if ( IS_NPC( ch )
       || ch->level < skill_table[gsn_scribe].skill_level[ch->gameclass] )
  {
    send_to_char( "You do not know how to scribe scrolls.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );

  if (IS_GHOST(ch))
  {
    send_to_char("Scribing is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  /* Why?  It's not offensive.  -- Taeloch (Draxal bug)
    if (IS_ROOM_SAFE(ch) || IS_ARENA(ch))
    {
        send_to_char("You fail due to the properties of this room.\n\r",ch);
        return;
    }
  */
  if ( arg[0] == '\0' )
  {
    send_to_char( "Scribe what spell?\n\r", ch );
    return;
  }

  /* Do we have a parchment to scribe spells? */
  for ( obj = ch->carrying; obj; obj = obj->next_content )
  {
    if ( obj->item_type == ITEM_SCROLL && obj->wear_loc == WEAR_HOLD )
      break;
  }

  if ( !obj )
  {
    send_to_char( "You are not holding a parchment.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg1 );

  // If no argument, don't even try advanced scribe.
  if ( !IS_NULLSTR(arg1) )
  {
    if (get_skill(ch, gsn_advanced_scribe) > number_percent())
    {
      adv_scribe = TRUE;

      if (is_number(arg1))
      {
        scroll_level = atoi(arg1);

        if (scroll_level < 1 || scroll_level > ch->level)
        {
          send_to_char("The scroll's level must be between 1 and your level.\n\r", ch);
          return;
        }
      }
      else
      {
        send_to_char("SYNTAX: scribe <spell> <scroll level>\n\r", ch);
        return;
      }
    }
    else
      adv_scribe = FALSE;
  }
  else
    adv_scribe = FALSE;

  if ( ( sn = find_spell(ch, arg) ) < 1
       || skill_table[sn].spell_fun == spell_null
       || ch->level < skill_table[sn].skill_level[ch->gameclass]
       || ch->pcdata->learned[sn] == 0 )
  {
    send_to_char( "You don't know any spells by that name.\n\r", ch );
    return;
  }

  // Alchemy spells can only be brewed
  gn = group_lookup("alchemy");
  if (gn != -1)
  {
    for (fsn = 0; fsn < MAX_IN_GROUP; fsn++)
    {
      if ( group_table[gn].spells[fsn] == NULL )
        break;

      if (sn == skill_lookup_exact( group_table[gn].spells[fsn]) )
      {
        // this spell is in alchemy!
        send_to_char( "You can only brew alchemy spells.\n\r", ch );
        return;
      }
    }
  }

  if ( adv_scribe )
    manamult = 6;
  else
    manamult = 4;

  if (ch->mana < ( manamult * skill_table[sn].min_mana ) )
  {
    send_to_char( "You do not have enough mana.\n\r", ch );
    return;
  }

  act( "$n begins writing a scroll.", ch, obj, NULL, TO_ROOM );
  if (IS_CLASS_ALCHEMIST(ch))
  {
    if (is_affected( ch, skill_lookup( "clear head" ) ) )
      WAIT_STATE( ch, (skill_table[gsn_scribe].beats*0.5) );
    else
      WAIT_STATE( ch, skill_table[gsn_scribe].beats*0.75 );
  }
  else
  {
    if (is_affected( ch, skill_lookup( "clear head" ) ) )
      WAIT_STATE( ch, (skill_table[gsn_scribe].beats*0.75) );
    else
      WAIT_STATE( ch, skill_table[gsn_scribe].beats );
  }

  /* Check the skill percentage, fcn(int,wis,skill) */
  if ( !IS_NPC(ch)
       && ( ch->mana > manamult * skill_table[sn].min_mana )
       && ( number_percent( ) > get_skill(ch, gsn_scribe)))
  {
    act( "$p bursts in {rf{Rl{ya{rmes{x!", ch, obj, NULL, TO_CHAR );
    act( "$p bursts in {rf{Rl{ya{rmes{x!", ch, obj, NULL, TO_ROOM );
    if (IS_VALID(obj)) // make sure the object wasn't destroyed
      extract_obj( obj );
    spell_fireball(skill_lookup("fireball"), LEVEL_HERO - 1, ch, ch,0);
    check_improve(ch,gsn_scribe,FALSE,3);
    if (IS_CLASS_ALCHEMIST(ch))
      check_improve(ch,sn,FALSE,1); // alchemists can't cast, have to improve somehow

    return;
  }

  if (IS_CLASS_ALCHEMIST(ch))
    check_improve(ch,sn,TRUE,1); // alchemists can't cast, have to improve somehow

  if (adv_scribe)
  {
    obj->level = scroll_level;
    obj->value[0] = scroll_level;
    check_improve(ch,gsn_advanced_scribe,TRUE,4);
    //Check This
    if (spell_imprint(sn, ch->level, ch, obj, 2))
      check_improve(ch,gsn_brew,TRUE,4);
  }
  else
  {
    obj->level = ch->level*2/3;
    obj->value[0] = ch->level*2/3;
    check_improve(ch,gsn_scribe,TRUE,4);
    //Check This
    if (spell_imprint(sn, ch->level, ch, obj, 0))
      check_improve(ch,gsn_brew,TRUE,4);
  }
}

void do_erase ( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  if ( !IS_NPC( ch )
       &&   ch->level < skill_table[gsn_erase].skill_level[ch->gameclass] )
  {
    send_to_char( "You do not know how to erase scrolls.\n\r", ch );
    return;
  }
  if (IS_GHOST(ch))
  {
    send_to_char("Erasing is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Erase what scroll?\n\r", ch );
    return;
  }

  if ( (obj = get_obj_carry (ch, arg, ch)) == NULL)
  {
    send_to_char("You do not have that!\n\r",ch);
    return;
  }

  if ( obj->item_type != ITEM_SCROLL)
  {
    send_to_char( "That is not a  scroll?\n\r", ch );
    return;
  }

  if ( !can_use_clan_obj( ch, obj ) )
  {
    send_to_char( "That belongs to another clan.\n\r", ch );
    return;
  }

  if (obj->value[1] < 1)
  {
    send_to_char("That parchment is blank!\n\r",ch);
    return;
  }

  act( "$n begins erasing a scroll.", ch, obj, NULL, TO_ROOM );
  act( "You begin erasing a scroll.", ch, obj, NULL, TO_CHAR );
  WAIT_STATE( ch, skill_table[gsn_erase].beats );

  /* Check the skill percentage, fcn(int,wis,skill) */
  if ( !IS_NPC(ch)
       && ( number_percent( ) > (ch->pcdata->learned[gsn_erase] +
                                 (ch->level - obj->level))))
  {
    act( "$p bursts in {rf{Rl{ya{rmes{x!", ch, obj, NULL, TO_CHAR );
    act( "$p bursts in {rf{Rl{ya{rmes{x!", ch, obj, NULL, TO_ROOM );
    spell_fireball(skill_lookup("fireball"), LEVEL_HERO - 1, ch, ch,0);
    check_improve(ch,gsn_erase,FALSE,4);
    if (IS_VALID(obj)) // make sure the object wasn't destroyed
      extract_obj( obj );
    return;
  }

  /* basically, making scrolls more potent than potions; also, scrolls
     are not limited in the choice of spells, i.e. scroll of enchant weapon
     has no analogs in potion forms --- JH */

  check_improve(ch,gsn_erase,TRUE,4);
  spell_erase(gsn_erase, ch->level, ch, obj, 1);

}

/* craft wands/staves -- depends on class */
/* make sure you can't cast multiple times */
void do_craft ( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH], arg1[MIL];
  char buf[MSL], buf1[MSL], buf2[MSL];
  OBJ_DATA *obj;
  int sn, wand_level=0, gn, fsn;
  bool adv_craft = FALSE;
  int craftitem, manamult;
  char craftname[10];

  switch ( ch->gameclass )
  {
    case cWarlock:
    case cInquisitor:
      craftitem = ITEM_STAFF;
      strcpy(craftname,"staff");
      break;
    default:
      craftitem = ITEM_WAND;
      strcpy(craftname,"wand");
      break;
  }

  if ( IS_NPC( ch )
       || ch->level < skill_table[gsn_craft].skill_level[ch->gameclass] )
  {
    printf_to_char( ch, "You do not know how to craft %ss.\n\r", craftname);
    return;
  }

  argument = one_argument( argument, arg );

  if (IS_GHOST(ch))
  {
    send_to_char("Crafting is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if (IS_ROOM_SAFE(ch) || IS_ARENA(ch))
  {
    send_to_char("Something about this room prevents it...\n\r",ch);
    return;
  }

  if ( arg[0] == '\0' )
  {
    printf_to_char( ch, "Craft the %s with what spell?\n\r", craftname);
    return;
  }

  /* this is where we check for the necessary elements */
  for ( obj = ch->carrying; obj; obj = obj->next_content )
  {
    if ( ( obj->item_type == craftitem )
         &&   ( obj->wear_loc == WEAR_HOLD ) )
      break;
  }

  if ( !obj )
  {
    send_to_char( "You are not holding the proper materials.\n\r", ch );
    return;
  }

  if ( str_cmp( skill_table[obj->value[3]].name, "reserved" ) )
  {
    printf_to_char( ch, "This %s already has a spell attached to it.\n\r", craftname);
    return;
  }

  if (get_skill(ch, gsn_advanced_craft) > number_percent())
  {
    argument = one_argument( argument, arg1);
    adv_craft = TRUE;
    if (arg1[0] == '\0'
        || (obj->value[1] != 0 && obj->value[1] != -1) )
      wand_level = ch->level;
    else if (is_number(arg1))
    {
      wand_level = atoi(arg1);
      if (wand_level < 1 || wand_level > ch->level)
      {
        printf_to_char( ch, "The %s's level must be between 1 and your level.\n\r", craftname);
        return;
      }
    }
    else
    {
      printf_to_char( ch, "SYNTAX: craft <spell> <%s level>\n\r", craftname);
      return;
    }
  }

  if ( ( sn = find_spell(ch, arg) ) < 1
       || skill_table[sn].spell_fun == spell_null
       || ch->level < skill_table[sn].skill_level[ch->gameclass]
       || ch->pcdata->learned[sn] == 0 )
  {
    send_to_char( "You don't know any spells by that name.\n\r", ch );
    return;
  }

  // Alchemy spells can only be brewed
  gn = group_lookup("alchemy");
  if (gn != -1)
  {
    for (fsn = 0; fsn < MAX_IN_GROUP; fsn++)
    {
      if ( group_table[gn].spells[fsn] == NULL )
        break;

      if (sn == skill_lookup_exact( group_table[gn].spells[fsn]) )
      {
        // this spell is in alchemy!
        send_to_char( "You can only brew alchemy spells.\n\r", ch );
        return;
      }
    }
  }

  sprintf(arg1, "$n begins crafting a %s.", craftname);
  act( arg1, ch, obj, NULL, TO_ROOM );
  if (IS_CLASS_ALCHEMIST(ch))
  {
    if (is_affected( ch, skill_lookup( "clear head" ) ) )
      WAIT_STATE( ch, (skill_table[gsn_craft].beats*0.5) );
    else
      WAIT_STATE( ch, skill_table[gsn_craft].beats*0.75 );
  }
  else
  {
    if (is_affected( ch, skill_lookup( "clear head" ) ) )
      WAIT_STATE( ch, (skill_table[gsn_craft].beats*0.75) );
    else
      WAIT_STATE( ch, skill_table[gsn_craft].beats );
  }

  /* Check the skill percentage, fcn(int,wis,skill) */
  if ( !IS_NPC(ch)
       && ( number_percent( ) > get_skill(ch, gsn_craft)))
  {
    act( "$p explodes in {rf{Rl{ya{rmes{x!", ch, obj, NULL, TO_CHAR );
    act( "$p explodes in {rf{Rl{ya{rmes{x!", ch, obj, NULL, TO_ROOM );
    if (IS_VALID(obj)) // make sure the object wasn't destroyed
      extract_obj( obj );
    spell_fireball(skill_lookup("fireball"), LEVEL_HERO - 1, ch, ch,0);
    check_improve(ch,gsn_craft,FALSE,4);
    if (IS_CLASS_ALCHEMIST(ch))
      check_improve(ch,sn,FALSE,1); // alchemists can't cast, have to improve somehow
    return;
  }

  check_improve(ch,gsn_craft,TRUE,4);
  if (IS_CLASS_ALCHEMIST(ch))
    check_improve(ch,sn,TRUE,1); // alchemists can't cast, have to improve somehow

  if (adv_craft)
  {
    obj->level = wand_level;
    obj->value[0] = wand_level;
    obj->value[1] = ch->level/7;
    obj->value[2] = ch->level/7;
    check_improve(ch,gsn_advanced_craft,TRUE,4);
  } // warlocks should be lower charges, since less magic-oriented
  else
  {
    obj->level = ch->level*2/3;
    obj->value[0] = ch->level/3;
    obj->value[1] = ch->level/10;
    obj->value[2] = ch->level/10;
    check_improve(ch,gsn_advanced_craft,FALSE,1);
  }

  obj->value[3] = sn;

  mprintf (sizeof(buf), buf,
           "a %s of %s",
           craftname,
           skill_table[sn].name );

  printf_to_char( ch, "You have created %s!\n\r", buf );

  if ( adv_craft )
    manamult = 3;
  else
    manamult = 2;

  ch->mana -= (manamult * skill_table[sn].min_mana);
  if (ch->mana < 0)
    ch->mana = 0;

  free_string( obj->short_descr );
  obj->short_descr = str_dup(buf, obj->short_descr );

  //The item name/long desc did not change - Aarchane
  sprintf( buf1, "%s %s", craftname, skill_table[sn].name );
  free_string( obj->name );
  obj->name = str_dup( buf1, obj->name );

  sprintf( buf2, "The magic of %s created %s.", ch->name, buf );
  free_string( obj->description );
  obj->description = str_dup( buf2, obj->description );

}

void do_empty ( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  if ( !IS_NPC( ch )
       && ch->level < skill_table[gsn_empty].skill_level[ch->gameclass] )
  {
    send_to_char( "You do not know how to empty potions.\n\r", ch );
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("Emptying is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Empty what potion?\n\r", ch );
    return;
  }

  if ( (obj = get_obj_carry (ch, arg, ch)) == NULL)
  {
    send_to_char("You do not have that!\n\r",ch);
    return;
  }

  if ( obj->item_type != ITEM_POTION)
  {
    send_to_char( "That is not a potion.\n\r", ch );
    return;
  }

  if ( !can_use_clan_obj( ch, obj ) )
  {
    send_to_char( "That belongs to another clan.\n\r", ch );
    return;
  }

  if ((obj->value[1] < 1)
      && (obj->value[2] < 1)
      && (obj->value[3] < 1)
      && (obj->value[4] < 1))
  {
    send_to_char("That vial is empty!\n\r",ch);
    return;
  }

  act( "$n begins emptying a potion.", ch, obj, NULL, TO_ROOM );
  act( "You begin emptying a potion.", ch, obj, NULL, TO_CHAR );
  WAIT_STATE( ch, skill_table[gsn_empty].beats );

  /* Check the skill percentage, fcn(int,wis,skill) */
  if ( !IS_NPC(ch)
       && ( number_percent( ) > (ch->pcdata->learned[gsn_empty] +
                                 (ch->level - obj->level))))
  {
    act( "$p bursts in {rf{Rl{ya{rmes{x!", ch, obj, NULL, TO_CHAR );
    act( "$p bursts in {rf{Rl{ya{rmes{x!", ch, obj, NULL, TO_ROOM );
    spell_fireball(skill_lookup("fireball"), LEVEL_HERO - 1, ch, ch,0);
    check_improve(ch,gsn_empty,FALSE,4);
    if (IS_VALID(obj)) // make sure the object wasn't destroyed
      extract_obj( obj );
    return;
  }

  /* basically, making scrolls more potent than potions; also, scrolls
     are not limited in the choice of spells, i.e. scroll of enchant weapon
     has no analogs in potion forms --- JH */

  check_improve(ch,gsn_empty,TRUE,4);
  spell_erase(gsn_empty, ch->level, ch, obj, 1);

}


void do_lore( CHAR_DATA *ch, char *argument )
{
  char object_name[MAX_INPUT_LENGTH + 100];

  OBJ_DATA *obj;
  argument = one_argument(argument, object_name);

  if ( ( obj = get_obj_carry( ch, object_name, ch ) ) == NULL )
  {
    send_to_char( "You are not carrying that.\n\r", ch );
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("Pondering is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }
  send_to_char( "You ponder the item.\n\r", ch );
  if ( number_percent() < get_skill( ch, gsn_lore )
       &&   ch->level >= obj->level )
  {

    printf_to_char(ch,
                   "Object '%s' is type %s, extra flags %s.\n\rWeight is %.2f, value is %d, level is %d.\n\r",
                   obj->name,
                   item_name(obj->item_type),
                   extra_bit_name( obj->extra_flags ),
                   (float)(obj->weight / 10),
                   obj->cost,
                   obj->level );

    switch ( obj->item_type )
    {
      case ITEM_SCROLL:
      case ITEM_POTION:
      case ITEM_PILL:
        printf_to_char(ch, "Some level spells of:");

        if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
        {
          send_to_char( " '", ch );
          send_to_char( skill_table[obj->value[1]].name, ch );
          send_to_char( "'", ch );
        }

        if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
        {
          send_to_char( " '", ch );
          send_to_char( skill_table[obj->value[2]].name, ch );
          send_to_char( "'", ch );
        }

        if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
        {
          send_to_char( " '", ch );
          send_to_char( skill_table[obj->value[3]].name, ch );
          send_to_char( "'", ch );
        }

        if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
        {
          send_to_char(" '",ch);
          send_to_char(skill_table[obj->value[4]].name,ch);
          send_to_char("'",ch);
        }

        send_to_char( ".\n\r", ch );
        break;

      case ITEM_WAND:
      case ITEM_STAFF:
        printf_to_char(ch, "Has some charges of some level" );

        if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
        {
          send_to_char( " '", ch );
          send_to_char( skill_table[obj->value[3]].name, ch );
          send_to_char( "'", ch );
        }

        send_to_char( ".\n\r", ch );
        break;

      case ITEM_DRINK_CON:
        printf_to_char(ch,"It holds %s-colored %s.\n\r",
                       liq_table[obj->value[2]].liq_color,
                       liq_table[obj->value[2]].liq_name);
        break;

      case ITEM_CONTAINER:
        printf_to_char(ch,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
                       obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
        if (obj->value[4] != 100)
        {
          printf_to_char(ch,"Weight multiplier: %d%%\n\r",
                         obj->value[4]);
        }
        break;

      case ITEM_WEAPON:
        send_to_char("Weapon type is ",ch);

        switch (obj->value[0])
        {
          case(WEAPON_EXOTIC) : send_to_char("exotic.\n\r",ch);
            break;
          case(WEAPON_SWORD)  : send_to_char("sword.\n\r",ch);
            break;
          case(WEAPON_DAGGER) : send_to_char("dagger.\n\r",ch);
            break;
          case(WEAPON_SPEAR)  : send_to_char("spear/staff.\n\r",ch);
            break;
          case(WEAPON_MACE)   : send_to_char("mace/club.\n\r",ch);
            break;
          case(WEAPON_AXE)  : send_to_char("axe.\n\r",ch);
            break;
          case(WEAPON_FLAIL)  : send_to_char("flail.\n\r",ch);
            break;
          case(WEAPON_WHIP)  : send_to_char("whip.\n\r",ch);
            break;
          case(WEAPON_POLEARM): send_to_char("polearm.\n\r",ch);
            break;
					case(WEAPON_CROSSBOW) : send_to_char("crossbow.\n\r",ch);
						break;
          default    :
            send_to_char("unknown.\n\r",ch);
            break;
        }

        if (obj->pIndexData->new_format)
          printf_to_char(ch,"Damage is %dd%d (average %d).\n\r",
                         obj->value[1],obj->value[2],
                         (1 + obj->value[2]) * obj->value[1] / 2);
        else
          printf_to_char(ch, "Damage is %d to %d (average %d).\n\r",
                         obj->value[1], obj->value[2],
                         ( obj->value[1] + obj->value[2] ) / 2 );

        if (obj->value[4])  /* weapon flags */
        {
          printf_to_char(ch,"Weapons flags: %s\n\r",weapon_bit_name(obj->value[4]));
        }
        break;

      case ITEM_ARMOR:
        printf_to_char( ch, "Item is worn in the %s location.\n\r",
                        flag_string(
                          wear_flags, obj->wear_flags&~(ITEM_TAKE|ITEM_NO_SAC) ) );
        printf_to_char(ch,
                       "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r",
                       obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
        break;
    }
    check_improve( ch, gsn_lore, TRUE, 2 );
  }
  else
    check_improve( ch, gsn_lore, FALSE, 3 );

  return;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void get_obj_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort,
                           bool fShowNothing, char *arg1,
                           bool is_container, OBJ_DATA *container )
{
  CHAR_DATA *vch;
  char buf[MSL];
  char buf1[MIL];
  char buf2[MIL];
  char *pChar;

  if ( ch->desc == NULL )
    return;

  if ( ch->in_room )
  {
    get_temp_weight = 0;
    get_temp_number = 0;
    strcpy( buf, formatted_list ( list, ch, TRUE, TRUE, arg1, ch, NULL, NULL,
                                  can_get_item, container, "You can't carry that much.\n\r" ) );

    strcpy( buf1, " the following item" );
    pChar = strchr( buf, '\n' );
    pChar = strchr( pChar+1, '\n' );
    if ( pChar ) strcat( buf1, "s" );
    if ( container )
      strcat( strcat( buf1, " from " ), container->short_descr );
    strcat( buf1, ":");

    printf_to_char( ch, "You get%s\n\r", buf1 );
    send_to_char( buf, ch );

    strcat( strcpy( buf2, "$n gets" ), buf1 );

    act( buf2, ch, NULL, NULL, TO_ROOM );

    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
      if ( vch->position == POS_SLEEPING )
        continue;

      if ( vch != ch )
      {
        get_temp_weight = 0;
        get_temp_number = 0;
        send_to_char( formatted_list ( list, ch, TRUE, TRUE,
                                       arg1, ch, vch,
                                       NULL , can_get_item, container, NULL ), vch );
      }
    }
  }
  get_temp_weight = 0;
  get_temp_number = 0;
}

bool can_get_item( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
  CHAR_DATA *gch;
  bool own_container = FALSE;

  if (container
      && (container->carried_by == ch) )
    own_container = TRUE;
  if ( obj == NULL )
    return FALSE;
  if ( !CAN_WEAR( obj, ITEM_TAKE ) )
    return FALSE;
  if ( ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
       && !own_container )
    return FALSE;


  if ( ( !obj->in_obj || obj->in_obj->carried_by != ch )
       &&  ( get_carry_weight( ch ) + get_obj_weight( obj ) > can_carry_w( ch ) )
       &&   !own_container )
  {
// can_* functions should never print anything (Taeloch)
//    send_to_char( "You can't carry any more weight.\n\r", ch );
    return FALSE;
  }

  if ( !can_loot( ch, obj ) )
    return FALSE;
  if ( obj->in_room )
  {
    for ( gch = obj->in_room->people; gch; gch = gch->next_in_room )
      if ( gch->on == obj )
        return FALSE;
  }
  /* weight check - Use global variable reset before formattel list called. */
  if ( ( get_carry_weight( ch ) + get_temp_weight + get_obj_weight( obj )  > can_carry_w( ch ) )
       &&   !own_container )
  {
// can_* functions should never print anything (Taeloch)
//    send_to_char( "You can't carry any more weight.\n\r", ch );
    return FALSE;
  }

  /* This HAS to be after the check, so that a large obj wont stop a smaller obj coming in later. in the linked list */
  get_temp_weight += get_obj_weight(obj);

  /* number check - Use global variable reset before formattel list called. */
  get_temp_number += get_obj_number(obj);
  if ( (ch->carry_number + get_temp_number + 1 > can_carry_n(ch))
       &&   !own_container )
    return FALSE;

  //get_temp_number += get_obj_number(obj);

  if ( container )
  {
    if (container->pIndexData->vnum == OBJ_VNUM_PIT
        &&  get_trust(ch) < obj->level)
      return FALSE;

    if (container->pIndexData->vnum == OBJ_VNUM_PIT
        &&  !CAN_WEAR(container, ITEM_TAKE)
        &&  !IS_OBJ_STAT(obj,ITEM_HAD_TIMER))
      obj->timer = 0;
    return TRUE;
  }
  else
    return TRUE;

}

bool get_silent_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
  CHAR_DATA *gch;
  int members;
  char buffer[100];

  if ( obj == NULL )
    return FALSE;

  if ( !CAN_WEAR( obj, ITEM_TAKE ) )
    return FALSE;

  if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    return FALSE;

  if ( ( !obj->in_obj || obj->in_obj->carried_by != ch )
       &&   ( get_carry_weight( ch ) + get_obj_weight( obj ) > can_carry_w( ch ) ) )
    return FALSE;

  if ( !can_loot( ch, obj ) )
    return FALSE;

  if (obj->in_room )
  {
    for ( gch = obj->in_room->people; gch; gch = gch->next_in_room )
      if ( gch->on == obj )
        return FALSE;
  }

  if ( container )
  {
    if ( container->pIndexData->vnum == OBJ_VNUM_PIT
         &&   get_trust( ch ) < obj->level )
      return FALSE;

    if ( container->pIndexData->vnum == OBJ_VNUM_PIT
         &&  !CAN_WEAR( container, ITEM_TAKE )
         &&  !IS_OBJ_STAT( obj, ITEM_HAD_TIMER ) )
      obj->timer = 0;
    REMOVE_BIT( obj->extra_flags, ITEM_HAD_TIMER );

    if ( container->item_type == ITEM_LOCKER )
      obj_from_locker(obj,ch);
    else
      obj_from_obj( obj );
  }
  else
    obj_from_room( obj );

  if ( obj->item_type == ITEM_MONEY)
  {
    ch->silver += obj->value[0];
    ch->gold += obj->value[1];
    if ( IS_SET( ch->act, PLR_AUTOSPLIT ) )
    {
      /* AUTOSPLIT code */
      if (container
          && (container->item_type != ITEM_CORPSE_PC) )
      {
        members = 0;
        for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
          if ( !IS_AFFECTED( gch, AFF_CHARM ) && is_same_group( gch, ch ) )
            members++;

        if ( members > 1 && ( obj->value[0] > 1 || obj->value[1] ) )
        {
          mprintf(sizeof(buffer), buffer, "%d %d", obj->value[0], obj->value[1] );
          do_function( ch, &do_split, buffer );
        }
      }
    }
    extract_obj( obj );
  }
  else
    obj_to_char( obj, ch );

  return TRUE;
}

bool can_put_item( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
  if ( container )
  {
    if ( can_see_obj( ch, obj )
         &&   WEIGHT_MULT(obj) == 100
         &&   obj->wear_loc == WEAR_NONE
         &&   obj != container
         &&   can_drop_obj( ch, obj )
         &&   ( get_temp_weight + get_obj_weight( obj ) +
                get_true_weight( container ) ) <= ( container->value[0] * 10 )
         &&   get_obj_weight( obj ) < ( container->value[3] * 10 ) )
    {
      get_temp_weight += get_obj_weight( obj );
      return TRUE;
    }
  }
  else
    if ( can_see_obj( ch, obj )
         &&   obj->wear_loc == WEAR_NONE
         &&   can_drop_obj( ch, obj ) )
      return TRUE;

  return FALSE;
}

void put_obj_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort,
                           bool fShowNothing,char *arg1, bool
                           is_container, OBJ_DATA *container )
{
  CHAR_DATA *vch;
  char  buf[MSL];
  char  buf1[MIL];
  char  buf2[MIL];
  char *pChar;

  if ( ch->desc == NULL )
    return;

  if ( ch->in_room )
  {
    get_temp_weight = 0;

    strcpy( buf, formatted_list ( list, ch, TRUE, TRUE, arg1, ch, NULL, NULL,
                                  can_put_item, container, NULL ) );

    strcpy( buf1, " the following item" );
    pChar = strchr( buf, '\n' );
    pChar = strchr( pChar+1, '\n' );
    if ( pChar ) strcat( buf1, "s" );
    if ( container )
    {
      if ( IS_SET( container->value[1], CONT_PUT_ON ) )
        strcat( buf1, " onto " );
      else
        strcat( buf1, " into " );

      strcat( strcat( buf1, container->short_descr), ":");
      printf_to_char( ch, "You put%s\n\r", buf1 );
    }
    else
      printf_to_char( ch, "You drop%s\n\r", buf1 );

    send_to_char( buf, ch );

    if ( container )
      strcat( strcpy( buf2, "$n puts" ), buf1 );
    else
      strcat( strcpy( buf2, "$n drops" ), buf1 );

    act( buf2, ch, NULL, NULL, TO_ROOM );

    get_temp_weight=0;

    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
      if (vch->position == POS_SLEEPING)
        continue;

      if ( vch != ch )
      {
        get_temp_weight = 0;
        send_to_char(formatted_list (list, ch, TRUE, TRUE, arg1, ch, vch,
                                     NULL , can_put_item, container, NULL),vch);
      }
    }
  }
  get_temp_weight = 0;
}

bool can_give_item( CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *victim)
{
  if ( !can_see_obj( ch, obj ) )
    return FALSE;

  if ( !can_see_obj( victim,obj ) )
    return FALSE;

  if ( obj->wear_loc != WEAR_NONE )
    return FALSE;

  if ( !can_drop_obj( ch, obj ) )
    return FALSE;

  /* weight check - Use global variable reset before formattel list called. */
  if ( get_carry_weight( victim ) + get_temp_weight + get_obj_weight( obj )
       > can_carry_w( victim ) )
    return FALSE;
  /* This HAS to be after the check, so that a large obj wont stop a smaller obj coming in later. in the linked list */
  get_temp_weight += get_obj_weight( obj );

  /* number check - Use global variable reset before formattel list called. */
  if ( victim->carry_number + get_temp_number + 1 > can_carry_n( victim ) )
    return FALSE;

  get_temp_number += get_obj_number( obj );

  return TRUE;
}

void give_obj_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort,
                            bool fShowNothing,char *arg1, CHAR_DATA *victim )
{
  CHAR_DATA *vch;
  char buf[MSL];
  char buf1[MIL];
  char buf2[MIL];
  char *pChar;

  if ( ch->desc == NULL && !IS_NPC( ch ) )
    return;

  if ( ch->in_room )
  {

    get_temp_weight = 0;
    get_temp_number = 0;
    strcpy( buf, formatted_list( ch->carrying, ch, TRUE, TRUE,
                                 arg1, victim, NULL,
                                 can_give_item ,NULL, NULL, NULL ) );

    strcpy( buf1, " the following item" );
    pChar = strchr( buf, '\n' );
    pChar = strchr( pChar+1, '\n' );
    if ( pChar ) strcat( buf1, "s" );

    printf_to_char( ch, "You give%s to %s:\n\r", buf1, victim->name );
    send_to_char( buf, ch );

    if ( victim->position > POS_SLEEPING )
    {
      printf_to_char( victim, "%s gives you%s:\n\r", PERS(ch,victim), buf1 );
      send_to_char( buf, victim );
    }

    strcat( strcat( strcpy( buf2, "$n gives" ), buf1 ), " to $N:" );
    act( buf2, ch, NULL, victim, TO_NOTVICT );

    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
      if ( vch->position == POS_SLEEPING )
        continue;

      if ( vch != victim && vch != ch )
      {
        get_temp_weight = 0 ;
        get_temp_number = 0;
        send_to_char( formatted_list ( ch->carrying, ch, TRUE, TRUE,
                                       arg1, victim, vch,
                                       can_give_item , NULL, NULL, NULL ), vch );
      }
    }
  }
}

void do_size( CHAR_DATA *ch, char *argument )
{
  char object_name[MAX_INPUT_LENGTH + 100];
  OBJ_DATA *obj;

  if ( ch->position == POS_SLEEPING )
  {
    send_to_char( "How can you see an object when you are sleeping.\n\r", ch );
    return;
  }

  argument = one_argument( argument, object_name );
  if ( ( obj = get_obj_carry( ch, object_name, ch ) ) == NULL )
  {
    send_to_char( "You are not carrying that.\n\r", ch );
    return;
  }

  send_to_char( "You size up the item.\n\r", ch );

  printf_to_char( ch,
                  "Object '%s' is type %s.\n\rWeight is %.2f, level is %d.\n\r",
                  obj->name,
                  item_name( obj->item_type ),
                  ( float)(obj->weight / 10 ),
                  obj->level );
  return;
}

/*
 * Return a string based on amount of coins.
 */
char *get_coin_str( long amount, int coin_type )
{
  switch ( coin_type )
  {
    case 0:
      if ( amount == 1 )  return "one miserable #Wsilver#n coin";
      if ( amount == 2 )  return "a couple #Wsilver#n coins";
      if ( amount <= 10 )  return "a few #Wsilver#n coins";
      if ( amount <= 100 )  return "some #Wsilver#n coins";
      if ( amount <= 500 )  return "a lot of #Wsilver#n coins";
      if ( amount <= 2500 )  return "an enormous number of #Wsilver#n coins";
      return "a giant's horde of #Wsilver#n coins";
    case 1:
      if ( amount == 1 )  return "a shiny #Ogold#n coin";
      if ( amount == 2 )  return "a couple #Ogold#n coins";
      if ( amount <= 10 )  return "a few #Ogold#n coins";
      if ( amount <= 100 )  return "some #Ogold#n coins";
      if ( amount <= 500 )  return "a lot of #Ogold#n coins";
      if ( amount <= 2500 )  return "a treasure trove of #Ogold#n coins";
      return "a dragon's horde of #Ogold#n coins";
  }
  bug( "Get_coins_str: invalid coin_type", 0 );
  return "some coins";
}
#ifdef ARCODE
void do_give_new( CHAR_DATA * ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int count, result;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  {
    send_to_char( "Give what to whom?\n\r", ch );
    return;
  }

  if ( is_number( arg1 )
       ||   ( !str_cmp( arg1, "all" ) && argument[0] != '\0' ) )
  {
    // 'give NNNN coins target' or 'give all coins target'
    char  coin_str[MAX_STRING_LENGTH];
    int  amount=0;
    int  v_can_carry;
    bool  silver;
    bool  fAll = FALSE;
    bool  fCarry;

    if ( !str_cmp( arg1, "all" ) )
      fAll = TRUE;

    if ( !fAll && ( amount = atoi( arg1 ) ) <= 0 )
    {
      send_to_char( "Quit being silly.\n\r", ch );
      return;
    }

    if ( arg2[0] == '\0'
         ||   ( str_prefix( arg2, "coins" )
                &&     str_prefix( arg2, "gold" )
                &&     str_prefix( arg2, "silver" ) ) )
    {
      send_to_char( "#WSilver#n or #Ogold#n?\n\r", ch );
      return;
    }

    silver = str_prefix( arg2, "gold" );
    argument = one_argument( argument, arg2 );
    if ( arg2[0] == '\0' )
    {
      send_to_char( "Give to whom?\n\r", ch );
      return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    if ( victim == ch )
    {
      send_to_char( "What would be the point in that?\n\r", ch );
      return;
    }

    v_can_carry = can_carry_w( victim ) - get_carry_weight( victim );

    fCarry = IS_NPC( victim )
             && ( HAS_TRIGGER( victim, TRIG_BRIBE )
                  ||   IS_SET( victim->act, ACT_IS_CHANGER ) );

    if ( silver )
    {
      if ( fAll && ( amount = ch->silver ) <= 0 )
      {
        send_to_char( "But you don't have any #Wsilver#n coins!\n\r",
                      ch );
        return;
      }

      if ( ch->silver < amount )
      {
        if ( ch->silver == 1L )
          send_to_char(
            "You only have one miserable #Wsilver#n coin.\n\r", ch );
        else
          printf_to_char( ch,
                          "You only have #W%ld silver#n coins.\n\r", ch->silver );
        return;
      }

      /* Bribeable mobs & money changers should be unlimited. */
      if  ( v_can_carry < SILVER_WEIGHT( amount ) && !fCarry )
      {
        if ( fAll )
        {
          amount = 0;
          while ( ( amount += 1000 ) < ch->silver
                  && v_can_carry >= SILVER_WEIGHT( amount ) );
          amount = UMIN( amount, ch->silver );
          while ( amount > 0
                  && v_can_carry < SILVER_WEIGHT( amount ) )
            --amount;
        }

        if ( !fAll || amount <= 0 )
        {
          act( "$t: $N can't carry that much weight.",
               ch, "#WSilver#n", victim, TO_CHAR );
          return;
        }
      }

      ch->silver     -= amount;
      victim->silver += amount;
      strcpy( coin_str, get_coin_str( amount, 0 ) );
    }
    else
    {
      if ( fAll && ( amount = ch->gold ) <= 0 )
      {
        send_to_char( "But you don't have any #Ogold#n coins!\n\r",
                      ch );
        return;
      }

      if ( ch->gold < amount )
      {
        if ( ch->gold == 1L )
          send_to_char(
            "You only have one shiny #Ogold#n coin.\n\r", ch );
        else
          printf_to_char( ch, "You only have #O%ld gold#n coins.\n\r",
                          ch->gold );
        return;
      }

      /* Bribeable mobs and money changers should be unlimited. */
      if  ( v_can_carry < GOLD_WEIGHT( amount ) && !fCarry )
      {
        if ( fAll )
        {
          amount = 0;
          while ( ( amount += 1000 ) < ch->gold
                  && v_can_carry >= GOLD_WEIGHT( amount ) );
          amount = UMIN( amount, ch->gold );
          while ( amount > 0
                  && v_can_carry < GOLD_WEIGHT( amount ) )
            --amount;
        }

        if ( !fAll || amount <= 0 )
        {
          act( "$t: $N can't carry that much weight.",
               ch, "#OGold#n", victim, TO_CHAR );
          return;
        }
      }

      ch->gold   -= amount;
      victim->gold += amount;
      strcpy( coin_str, get_coin_str( amount, 1 ) );
    }

    mprintf( sizeof(buf), buf, "$n gives you %s%d %s",
             silver ? "#W" : "#O", amount,
             silver ? "silver#n coin" : "gold#n coin" );
    if ( amount > 1 ) strcat( buf, "s.");
    else strcat( buf, "." );
    act( buf, ch, NULL, victim, TO_VICT );

    mprintf( sizeof(buf), buf, "$n gives $N %s.", coin_str );
    act( buf, ch, NULL, victim, TO_NOTVICT );

    mprintf( sizeof(buf), buf, "You give $N %s%d %s",
             silver ? "#W" : "#O", amount,
             silver ? "silver#n coin" : "gold#n coin" );
    if ( amount > 1 ) strcat( buf, "s.");
    else strcat( buf, ".");
    act( buf, ch, NULL, victim, TO_CHAR );

    // Bribe trigger.
    if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_BRIBE ) )
      mp_bribe_trigger( victim, ch, silver ? amount : amount * 100 );

    if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_IS_CHANGER ) )
    {
      int change;

      change = (silver ? 95 * amount / 100 / 100 : 95 * amount);

      if ( !silver && change > victim->silver )
        victim->silver += change;

      if ( silver && change > victim->gold )
        victim->gold += change;

      if ( change < 1 && can_see( victim, ch ) )
      {
        act(
          "#g $n #gtells you 'I'm sorry, you did not give me enough to change.'#n",
          victim, NULL, ch, TO_VICT );
        mprintf( sizeof(buf), buf, "%d %s \"%s\"", amount, silver ? "silver" : "gold",
                 ch->name );
        do_function( victim, &do_give, buf );
      }
      else if ( can_see( victim, ch ) )
      {
        mprintf( sizeof(buf), buf, "%d %s \"%s\"", change, silver ? "gold" : "silver",
                 ch->name );
        do_function( victim, &do_give, buf );

        if ( silver )
        {
          mprintf( sizeof(buf), buf, "%d silver \"%s\"",
                   ( 95 * amount / 100 - change * 100 ), ch->name );
          do_function( victim, &do_give, buf );
        }

        act( "#g $n #gtells you 'Thank you, come again.'#n",
             victim, NULL, ch, TO_VICT );
      }
    }
    return;
  }

  if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( victim == ch )
  {
    send_to_char( "What would be the point in that?\n\r", ch );
    return;
  }

  if ( IS_NPC( victim ) && victim->pIndexData->pShop != NULL )
  {
    act( "#g $N #gtells you 'Sorry, you'll have to sell that.'#n",
         ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( !str_cmp( arg1, "all" ) )
  {
    if ( obj_command( 5, ch, ch->carrying, NULL, victim, -1 )
         ==   OBJCMD_FAIL )
      act( "You have nothing to give $N.",
           ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( !str_prefix( "all.", arg1 ) )
  {
    if ( obj_command( 5, ch, ch->carrying, arg1+4, victim, -1 )
         ==   OBJCMD_FAIL )
      act( "You have nothing you can give $N.",
           ch, NULL, victim, TO_CHAR );
    return;
  }

  count = mult_argument( arg1, arg3 );
  if ( count < 1 )
  {
    send_to_char( "Give how many?!\n\r", ch );
    return;
  }

  result = obj_command( 5, ch, ch->carrying, arg3, victim, count );
  if ( result == OBJCMD_FAIL )
    act( "You have nothing like that which you can give $N.",
         ch, NULL, victim, TO_CHAR );
  else if ( result > 0 && result < count )
    act( "You have nothing more like that which you can give $N.",
         ch, NULL, victim, TO_CHAR );
}
#endif

char *formatted_list( OBJ_DATA *list, CHAR_DATA *ch,
                      bool fShort,
                      bool fShowNothing,
                      char *arg1,
                      CHAR_DATA *victim,
                      CHAR_DATA *fch,
                      bool (*run_func)(CHAR_DATA *ch, OBJ_DATA *obj,
                                       CHAR_DATA *victim),
                      bool (*run_func2)( CHAR_DATA *ch, OBJ_DATA *obj,
                                         OBJ_DATA *container ),
                      OBJ_DATA *container,
                      char *fail_message )
{
  char buf[MAX_STRING_LENGTH];
  static char output[196605]; // was 65535, * 3 to prevent crash... maybe
  char *prgpstrShow[500];
  int   prgnShow[500];
  char *pstrShow = 0;
  OBJ_DATA *obj;
  int  nShow = 0;
  int  iShow;
  bool fCombine;
  bool showMsg = FALSE;

  if ( ch->desc == NULL && !IS_NPC( ch ) )
    return( "Nothing\n\r" );

  /*
   * Alloc space for output lines.
   */
  output[0] = '\0';

  /*
   * Format the list of objects.
   */
  for ( obj = list; obj; obj = obj->next_content )
  {
    if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
         &&   can_see_obj( ch, obj ) )
    {
      if ( run_func )
      {
        if ( !run_func( ch, obj, victim ) )
          continue;
      }
      else if ( run_func2 )
      {
        if ( !run_func2( ch, obj, container ) )
        {
          if ( CAN_WEAR( obj, ITEM_TAKE ) )
            showMsg = TRUE;
          continue;
        }
      }
      else
        return( "Nothing\n\r" );

      if ( fch )
      {
        if ( !can_see_obj( fch, obj ) )
          pstrShow = str_dup( "something", pstrShow );
        else
          pstrShow = format_obj_to_char( obj, ch, fShort, FALSE );
      }
      else
        pstrShow = format_obj_to_char( obj, ch, fShort, FALSE );

      fCombine = FALSE;

      if ( IS_NPC( victim ) || IS_SET( victim->comm_flags, COMM_COMBINE ) )
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
        prgpstrShow [nShow] = str_dup( pstrShow, "" );
        prgnShow    [nShow] = 1;
        nShow++;
      }
    }
  }
  /*
   * Output the formatted list.
   */
  for ( iShow = 0; iShow < nShow; iShow++ )
  {
    if ( prgpstrShow[iShow][0] == '\0' )
    {
      free_string( prgpstrShow[iShow] );
      continue;
    }

    if ( IS_NPC( victim ) || IS_SET( victim->comm_flags, COMM_COMBINE ) )
    {
      if ( prgnShow[iShow] != 1 )
      {
        mprintf( sizeof(buf), buf, "(%2d) ", prgnShow[iShow] );
        strcat( output, buf );
      }
      else
        strcat( output, "     " );

      strcat( output, prgpstrShow[iShow] );
      strcat( output, "\n\r" );
      free_string( prgpstrShow[iShow] );
    }
  }

  if ( fShowNothing && nShow == 0 )
    strcpy (output, "Nothing\n\r" );

  if (showMsg && (fail_message != NULL))   // Taeloch!
  {
    strcat( output, fail_message );
    strcat( output, "\n\r" );
  }

  return ( output );
}

bool check_pre_player_names(char *tempstr)
{
  struct dirent *dp;
  DIR *dirptr;
  int return_int, return_int2;

  if ( ( dirptr = opendir( PLAYER_DIR ) ) == NULL )
  {
    bug( "Player dir unopenable.", 0 );
    return( FALSE );
  }

  while ( ( dp = readdir( dirptr ) ) )
  {
    /* read in filename */
    return_int = memcmp( &dp->d_name[0], ".", 1 );
    return_int2 = memcmp( &dp->d_name[0], "..", 2 );
    if ( ( return_int ) && ( return_int2 ) )
    {
      /*Get Start and Stop Time for DERG File here!!  */
      if ( tempstr[0] && dp->d_name[0] )
      {
        if ( LOWER( tempstr[0] ) == LOWER( dp->d_name[0] ) )
        {
          if ( tempstr[1] && dp->d_name[1] )
          {
            if ( LOWER( tempstr[1] ) == LOWER( dp->d_name[1] ) )
            {
              if ( tempstr[2] && dp->d_name[2] )
              {
                if (LOWER(tempstr[2]) == LOWER( dp->d_name[2] ) )
                {
                  return TRUE;
                }
              }
            }
          }
        }
      }
    }
  }
  closedir( dirptr );
  return ( FALSE );
}

void do_wspellup( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  AFFECT_DATA *paf;
  char arg[MIL];
  char arg1[MIL];
  int sn,i;
  bool found;

  if ( IS_NPC( ch ) )
    return;

  if ( IS_GHOST( ch ) )
  {
    send_to_char("Casting spells or maintaining a spell list is useless as you are still {rDEAD{x.\n\r", ch);
    return;
  }

  argument = one_argument( argument,arg );
  argument = one_argument( argument, arg1 );

  if ( IS_NULLSTR( arg ) )
  {
    send_to_char( "Syntax: wspellup <weapon>\n\r", ch );
    send_to_char( "        wspellup edit <spell to add or remove>\n\r", ch );
    send_to_char( "        wspellup list\n\r", ch );
    return;
  }

  if (!str_cmp(arg,"list"))
  {
    found = FALSE;
    for (i=0; i < MAX_MAINTAINED; i++)
      if (ch->pcdata->maintained_weapon[i] > 0 )
      {
        printf_to_char(ch,"{[{c%-20s{w] {wis maintained.{x\n\r",
                       skill_table[ch->pcdata->maintained_weapon[i]].name);
        found = TRUE;
      }

    if (found == FALSE)
      send_to_char( "No spells in the list.\n\r", ch );

    return;
  }

  if (!str_cmp(arg,"empty"))
  {
    for (i=0; i < MAX_MAINTAINED; i++)
      ch->pcdata->maintained_weapon[i] = -1;
    send_to_char("{xWSpellup list emptied.\n\r", ch);
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
      if (ch->pcdata->maintained_weapon[i] == sn)
      {
        printf_to_char(ch,"You remove {c%s{x from your list.{x\n\r",
                       skill_table[sn].name);
        ch->pcdata->maintained_weapon[i] = -1;
        return;
      }
    }

    if (get_skill(ch, sn) <= 0)
    {
      send_to_char("{xWhat spell?\n\r", ch);
      return;
    }

    if ( ( skill_table[sn].target != TAR_OBJ_INV )
         && strcmp(skill_table[sn].name, "bless")
         && strcmp(skill_table[sn].name, "poison")
         && strcmp(skill_table[sn].name, "curse")
         && strcmp(skill_table[sn].name, "invisibility") )
    {
      send_to_char( "You may not add this spell.\n\r", ch );
      return;
    }

    for (i = 0; i < MAX_MAINTAINED; i++)
    {
      if (ch->pcdata->maintained_weapon[i] <= 0)
      {
        printf_to_char(ch,"You begin to maintain {c%s{x.\n\r",
                       skill_table[sn].name);
        ch->pcdata->maintained_weapon[i] = sn;
        return;
      }
    }
    send_to_char("Sorry, you may not memorize any more at this time.\n\r",ch);
  }

  if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
  {
    send_to_char( "You aren't carrying that.\n\r", ch );
    return;
  }

  if (obj->item_type != ITEM_WEAPON)
  {
    send_to_char( "That is not a weapon.\n\r", ch );
    return;
  }

  if ( ch->position == POS_SLEEPING )
  {
    send_to_char("You cannot cast spells while asleep.{x\n\r",ch);
    return;
  }

  send_to_char("Starting spellup...{x\n\r",ch);

  for (i = 0 ; i < MAX_MAINTAINED; i++)
  {
    if ( ch->pcdata->maintained_weapon[i] > 0 )
    {
      found = FALSE;

      if (!strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "sharp blade") )
      {
        // can only cast sharp on blade weapons
        switch (obj->value[0])
        {
          case WEAPON_SWORD:
          case WEAPON_DAGGER:
          case WEAPON_SPEAR:
          case WEAPON_AXE:
          case WEAPON_POLEARM:
            break;
          default:
            found = TRUE;
            break;
        }
      }

      if (obj->value[4]) // check for weapon perm affects
      {
        if ( ( obj->value[4] & WEAPON_FLAMING )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "flame blade") ) )
          found = TRUE;

        if ( ( obj->value[4] & WEAPON_FROST )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "frost blade") ) )
          found = TRUE;

        if ( ( obj->value[4] & WEAPON_VAMPIRIC )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "drain blade") ) )
          found = TRUE;

        if ( ( obj->value[4] & WEAPON_SHARP )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "sharp blade") ) )
          found = TRUE;

        if ( ( obj->value[4] & WEAPON_VORPAL )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "vorpal blade") ) )
          found = TRUE;

        if ( ( obj->value[4] & WEAPON_SHOCKING )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "shocking blade") ) )
          found = TRUE;

        if ( ( obj->value[4] & WEAPON_MANA_DRAIN )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "resilience blade") ) )
          found = TRUE;

        if ( ( obj->value[4] & WEAPON_POISON )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "envenom") ) )
          found = TRUE;

        if ( ( obj->value[4] & WEAPON_SHARP )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "sharpen") ) )
          found = TRUE;

        // check for frost and flaming opposites, as a weapon can't hold both
        if ( ( obj->value[4] & WEAPON_FROST )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "flame blade") ) )
          found = TRUE;

        if ( ( obj->value[4] & WEAPON_FLAMING )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "frost blade") ) )
          found = TRUE;
      } // end perm affect check

      // check item's extra affects
      if ( IS_OBJ_STAT( obj, ITEM_INVIS )
           && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "invisibility") ) )
        found = TRUE;

      if ( IS_OBJ_STAT( obj, ITEM_BLESS )
           && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "bless") ) )
        found = TRUE;

      if ( IS_OBJ_STAT( obj, ITEM_BLESS ) // blessed weapons can't take these affects
           && ( ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "resilience blade") )
                || ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "drain blade") )
                || ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "envenom") ) ) )
        found = TRUE;

      if ( IS_OBJ_STAT( obj, ITEM_EVIL )
           && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "curse") ) )
        found = TRUE;
      // end extra affects

      for ( paf=obj->affected; paf && !found; paf=paf->next )
      {
        // check for conflicting and existing affects
        if ( ( !strcmp(skill_table[(int) paf->type].name, "curse") )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "bless") ) )
          found = TRUE;

        if ( ( !strcmp(skill_table[(int) paf->type].name, "bless") )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "curse") ) )
          found = TRUE;

        if ( ( !strcmp(skill_table[(int) paf->type].name, "frost blade") )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "flame blade") ) )
          found = TRUE;

        if ( ( !strcmp(skill_table[(int) paf->type].name, "flame blade") )
             && ( !strcmp(skill_table[ch->pcdata->maintained_weapon[i]].name, "frost blade") ) )
          found = TRUE;

        if (paf->type == ch->pcdata->maintained_weapon[i])
          found = TRUE;
      }

      if (!found)
      {
        // not already affected, so cast it
        if ( skill_table[ch->pcdata->maintained_weapon[i]].spell_fun == spell_null )
        {
          // skills are commands, not casted
          printf_to_char(ch,"Using {c%s{x.\n\r",skill_table[ch->pcdata->maintained_weapon[i]].name);
          strcat(ch->desc->inbuf, skill_table[ch->pcdata->maintained_weapon[i]].name );
          strcat(ch->desc->inbuf, " " );
          strcat(ch->desc->inbuf, arg );
          strcat(ch->desc->inbuf, "\n" );
        }
        else
        {
          printf_to_char(ch,"Casting {c%s{x.\n\r",skill_table[ch->pcdata->maintained_weapon[i]].name);
          strcat(ch->desc->inbuf, "cast '" );
          strcat(ch->desc->inbuf, skill_table[ch->pcdata->maintained_weapon[i]].name );
          strcat(ch->desc->inbuf, "' " );
          strcat(ch->desc->inbuf, arg );
          strcat(ch->desc->inbuf, "\n" );
        }
      }
    }
  }

  send_to_char("Finished spellup.{x\n\r",ch);
  return;
}
