/***************************************************************************
 *  File: mem.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
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

/*
 * Globals
 */
extern          int                     top_reset;
extern          int                     top_area;
extern          int                     top_exit;
extern          int                     top_ed;
extern          int                     top_room;
extern		    int			            top_mprog_index;
//extern          int                     top_clan;

AREA_DATA		*	area_free;
EXTRA_DESCR_DATA	*	extra_descr_free;
EXIT_DATA		*	exit_free;
ROOM_INDEX_DATA		*	room_index_free;
OBJ_INDEX_DATA		*	obj_index_free;
SHOP_DATA		*	shop_free;
MOB_INDEX_DATA		*	mob_index_free;
RESET_DATA		*	reset_free;
HELP_DATA		*	help_free;

HELP_DATA		*	help_last;

void	free_extra_descr	args( ( EXTRA_DESCR_DATA *pExtra ) );
void	free_affect		args( ( AFFECT_DATA *af ) );
void	free_mprog              args ( ( MPROG_LIST *mp ) );


RESET_DATA *new_reset_data( void )
{
  RESET_DATA *pReset;

  if ( !reset_free )
    {
      pReset          =   alloc_perm( sizeof(*pReset) );
      top_reset++;
    }
  else
    {
      pReset          =   reset_free;
      reset_free      =   reset_free->next;
    }

  pReset->next        =   NULL;
  pReset->command     =   'X';
  pReset->arg1        =   0;
  pReset->arg2        =   0;
  pReset->arg3        =   0;
  pReset->arg4	=   0;

  return pReset;
}



void free_reset_data( RESET_DATA *pReset )
{
  pReset->next            = reset_free;
  reset_free              = pReset;
  return;
}



AREA_DATA *new_area( void )
{
  AREA_DATA *pArea;
  char buf[MAX_INPUT_LENGTH];

  if ( !area_free )
    {
      pArea   =   alloc_perm( sizeof(*pArea) );
      top_area++;
    }
  else
    {
      pArea       =   area_free;
      area_free   =   area_free->next;
    }

  pArea->next             =   NULL;
  pArea->name             =   str_dup( "New area" , pArea->name);
  pArea->area_flags       =   AREA_ADDED;
  pArea->security         =   1;
  pArea->continent        =   3;  // Continent 3 is for ALL unlinked areas
  pArea->builders         =   str_dup( "None", pArea->builders );
  pArea->min_vnum         =   0;
  pArea->max_vnum         =   0;
  pArea->age              =   0;
  pArea->nplayer          =   0;
  pArea->empty            =   TRUE;              /* ROM patch */
  mprintf( sizeof(buf), buf, "area%d.are", pArea->vnum );
  pArea->file_name        =   str_dup( buf , pArea->file_name);
  pArea->vnum             =   top_area-1;
  pArea->reset_rate       =   15;  //30 ticks for default reset rates.

  return pArea;
}



void free_area( AREA_DATA *pArea )
{
  free_string( pArea->name );
  free_string( pArea->file_name );
  free_string( pArea->builders );
  free_string( pArea->credits );

  pArea->next         =   area_free->next;
  area_free           =   pArea;
  return;
}



EXIT_DATA *new_exit( void )
{
  EXIT_DATA *pExit;

  if ( !exit_free )
    {
      pExit           =   alloc_perm( sizeof(*pExit) );
      top_exit++;
    }
  else
    {
      pExit           =   exit_free;
      exit_free       =   exit_free->next;
    }

  pExit->u1.to_room   =   NULL;                  /* ROM OLC */
  pExit->next         =   NULL;
  /*  pExit->vnum         =   0;                        ROM OLC */
  pExit->exit_info    =   0;
  pExit->key          =   0;
  pExit->keyword      =   &str_empty[0];
  pExit->description  =   &str_empty[0];
  pExit->rs_flags     =   0;

  return pExit;
}



void free_exit( EXIT_DATA *pExit )
{
  free_string( pExit->keyword );
  free_string( pExit->description );

  pExit->next         =   exit_free;
  exit_free           =   pExit;
  return;
}


ROOM_INDEX_DATA *new_room_index( void )
{
  ROOM_INDEX_DATA *pRoom;
  int door;

  if ( !room_index_free )
    {
      pRoom           =   alloc_perm( sizeof(*pRoom) );
      top_room++;
    }
  else
    {
      pRoom           =   room_index_free;
      room_index_free =   room_index_free->next;
    }

  pRoom->next             =   NULL;
  pRoom->people           =   NULL;
  pRoom->contents         =   NULL;
  pRoom->extra_descr      =   NULL;
  pRoom->area             =   NULL;
  pRoom->mprogs           =   NULL;

  for ( door=0; door < MAX_DIR; door++ )
    pRoom->exit[door]   =   NULL;

  pRoom->name             =   str_dup( "new_room", pRoom->name );//&str_empty[0];
  pRoom->description      =   &str_empty[0];
  pRoom->owner	          =	  &str_empty[0];
  pRoom->vnum             =   0;
  pRoom->room_flags       =   0;
  pRoom->mprog_flags      =   0;
  pRoom->mprog_delay      =   0;
  pRoom->state            =  -1;
  pRoom->light            =   0;
  pRoom->sector_type      =   0;
  pRoom->clan		      =   0;
  pRoom->clan_name        =   &str_empty[0];
  pRoom->heal_rate	      = 100;
  pRoom->mana_rate	      = 100;

  return pRoom;
}



void free_room_index( ROOM_INDEX_DATA *pRoom )
{
  int door;
  EXTRA_DESCR_DATA *pExtra, *wExtra;
  RESET_DATA *pReset, *wReset;

  free_string( pRoom->name );
  free_string( pRoom->description );
  free_string( pRoom->owner );

  for ( door = 0; door < MAX_DIR; door++ )
    {
      if ( pRoom->exit[door] ) {
	free_exit( pRoom->exit[door] );
	pRoom->exit[door] = NULL;
      }
    }

  for ( pExtra = pRoom->extra_descr; pExtra; pExtra = wExtra )
    {
      wExtra = pExtra->next;
      free_extra_descr( pExtra );
    }

  for ( pReset = pRoom->reset_first; pReset; pReset = wReset )
    {
      wReset = pReset->next;
      free_reset_data( pReset );
    }

  pRoom->next     =   room_index_free;
  room_index_free =   pRoom;
  return;
}

extern AFFECT_DATA *affect_free;


SHOP_DATA *new_shop( void )
{
  SHOP_DATA *pShop;
  int buy;

  if ( !shop_free )
    {
      pShop           =   alloc_perm( sizeof(*pShop) );
      top_shop++;
    }
  else
    {
      pShop           =   shop_free;
      shop_free       =   shop_free->next;
    }

  pShop->next         =   NULL;
  pShop->keeper       =   0;

  for ( buy=0; buy<MAX_TRADE; buy++ )
    pShop->buy_type[buy]    =   0;

  pShop->profit_buy   =   100;
  pShop->profit_sell  =   100;
  pShop->open_hour    =   0;
  pShop->close_hour   =   23;

  return pShop;
}



void free_shop( SHOP_DATA *pShop )
{
  pShop->next = shop_free;
  shop_free   = pShop;
  return;
}



OBJ_INDEX_DATA *new_obj_index( void )
{
  OBJ_INDEX_DATA *pObj;
  int value;

  if ( !obj_index_free )
    {
      pObj           =   alloc_perm( sizeof(*pObj) );
      top_obj_index++;
    }
  else
    {
      pObj            =   obj_index_free;
      obj_index_free  =   obj_index_free->next;
    }

  pObj->next          =   NULL;
  pObj->extra_descr   =   NULL;
  pObj->affected      =   NULL;
  pObj->area          =   NULL;
/*  pObj->mprogs        =   NULL;
  pObj->mprog_flags   =   0;
  pObj->mprog_target  =   NULL; */
  pObj->name          =   str_dup( "no name" ,pObj->name);
  pObj->short_descr   =   str_dup( "(no short description)",pObj->short_descr );
  pObj->description   =   str_dup( "(no description)" ,pObj->description);
  pObj->vnum          =   0;
  pObj->item_type     =   ITEM_TRASH;
  pObj->extra_flags   =   0;
  pObj->wear_flags    =   0;
  pObj->count         =   0;
  pObj->weight        =   0;
  pObj->cost          =   0;
  pObj->material      =   str_dup( "unknown",pObj->material );      /* ROM */
  pObj->condition     =   100;                        /* ROM */
  for ( value = 0; value < 7; value++ )               /* 5 - ROM */
    pObj->value[value]  =   0;

  pObj->new_format    = TRUE; /* ROM */

  return pObj;
}



void free_obj_index( OBJ_INDEX_DATA *pObj )
{
  EXTRA_DESCR_DATA *pExtra, *wExtra;
  AFFECT_DATA *pAf, *wAf;

  free_string( pObj->name );
  free_string( pObj->short_descr );
  free_string( pObj->description );

  for ( pAf = pObj->affected; pAf; pAf = wAf )
    {
      wAf = pAf->next;
      free_affect( pAf );
    }

  for ( pExtra = pObj->extra_descr; pExtra; pExtra = wExtra )
    {
      wExtra = pExtra->next;
      free_extra_descr( pExtra );
    }

  pObj->next = obj_index_free;
  obj_index_free = pObj;
  return;
}



MOB_INDEX_DATA *new_mob_index( void )
{
  MOB_INDEX_DATA *pMob;

  if ( !mob_index_free )
    {
      pMob           =   alloc_perm( sizeof(*pMob) );
      top_mob_index++;
    }
  else
    {
      pMob            =   mob_index_free;
      mob_index_free  =   mob_index_free->next;
    }

  pMob->next          =   NULL;
  pMob->spec_fun      =   NULL;
  pMob->pShop         =   NULL;
  pMob->area          =   NULL;
  pMob->player_name   =   str_dup( "no name",pMob->player_name );
  pMob->short_descr   =   str_dup( "(no short)",pMob->short_descr);
  pMob->long_descr    =   str_dup( "(no long)\n\r",pMob->long_descr );
  pMob->description   =   &str_empty[0];
  pMob->path          =   &str_empty[0];
  pMob->max_riders    =   0;
  pMob->vnum          =   0;
  pMob->count         =   0;
  pMob->killed        =   0;
  pMob->sex           =   0;
  pMob->level         =   0;
  pMob->act           =   ACT_IS_NPC;
  /*pMob->plr2	      =	  0;*/
  pMob->affected_by   =   0;
  pMob->alignment     =   0;
  pMob->hitroll	      =   0;
  pMob->max_count     =   1;
  pMob->race                =   race_lookup( "human" ); /* - Hugin */
  pMob->form                =   0;           /* ROM patch -- Hugin */
  pMob->parts               =   0;           /* ROM patch -- Hugin */
  pMob->imm_flags           =   0;           /* ROM patch -- Hugin */
  pMob->res_flags           =   0;           /* ROM patch -- Hugin */
  pMob->vuln_flags          =   0;           /* ROM patch -- Hugin */
  pMob->material            =   str_dup("unknown",pMob->material);/* Hugin */
  pMob->off_flags           =   0;           /* ROM patch -- Hugin */
  pMob->size                =   SIZE_MEDIUM; /* ROM patch -- Hugin */
  pMob->ac[AC_PIERCE]	    =   0;           /* ROM patch -- Hugin */
  pMob->ac[AC_BASH]	        =   0;           /* ROM patch -- Hugin */
  pMob->ac[AC_SLASH]	    =   0;           /* ROM patch -- Hugin */
  pMob->ac[AC_EXOTIC]	    =   0;           /* ROM patch -- Hugin */
  pMob->hit[DICE_NUMBER]	=   0;           /* ROM patch -- Hugin */
  pMob->hit[DICE_TYPE]	    =   0;           /* ROM patch -- Hugin */
  pMob->hit[DICE_BONUS]	    =   0;           /* ROM patch -- Hugin */
  pMob->mana[DICE_NUMBER]	=   0;           /* ROM patch -- Hugin */
  pMob->mana[DICE_TYPE]	    =   0;           /* ROM patch -- Hugin */
  pMob->mana[DICE_BONUS]	=   0;           /* ROM patch -- Hugin */
  pMob->damage[DICE_NUMBER]	=   0;           /* ROM patch -- Hugin */
  pMob->damage[DICE_TYPE]	=   0;           /* ROM patch -- Hugin */
  pMob->damage[DICE_NUMBER]	=   0;           /* ROM patch -- Hugin */
  pMob->start_pos           =   POS_STANDING;/*  -- Hugin */
  pMob->default_pos         =   POS_STANDING;/*  -- Hugin */
  pMob->wealth              =   0;

  pMob->new_format          =   TRUE;  /* ROM */

  return pMob;
}



void free_mob_index( MOB_INDEX_DATA *pMob )
{
  MPROG_LIST *list, *mp_next;

  free_string( pMob->player_name );
  free_string( pMob->short_descr );
  free_string( pMob->long_descr );
  free_string( pMob->description );
  free_string( pMob->path );
  for( list = pMob->mprogs; list; list = mp_next )
    {
      mp_next = list->next;
      free_mprog( pMob->mprogs );
    }

  if( pMob->pShop )
    {
      free_shop( pMob->pShop );
    }

  pMob->next = mob_index_free;
  mob_index_free = pMob;
  return;
}

MPROG_CODE              *       mpcode_free;

MPROG_CODE *new_mpcode(void)
{
  MPROG_CODE *NewCode;

  if (!mpcode_free)
    {
      NewCode = alloc_perm(sizeof(*NewCode) );
      top_mprog_index++;
    }
  else
    {
      NewCode     = mpcode_free;
      mpcode_free = mpcode_free->next;
    }

  NewCode->vnum    = 0;
  NewCode->code    = str_dup("", NewCode->code);
  NewCode->next    = NULL;

  return NewCode;
}

void free_mpcode(MPROG_CODE *pMcode)
{
  free_string(pMcode->code);
  pMcode->next = mpcode_free;
  mpcode_free  = pMcode;
  return;
}

/* Help Editor - kermit 1/98 */
HELP_DATA *new_help(void)
{
  HELP_DATA *NewHelp;

  NewHelp = alloc_perm(sizeof(*NewHelp) );

  NewHelp->level    = 0;
  NewHelp->keyword = str_dup("",NewHelp->keyword);
  NewHelp->text    = str_dup("",NewHelp->text);
  NewHelp->next    = NULL;

  return NewHelp;
}

/* Clan Editor - Robert Leonard 10/2006 */

CLAN_DATA       *       clan_free;

CLAN_DATA *new_clan( void )
{
  CLAN_DATA *clan;
  int i;
  //char buf[MAX_INPUT_LENGTH];


  clan                      = alloc_perm( sizeof( *clan ) );
  clan->name                = str_dup( "New Clan" , clan->name);
  clan->donation_gem        = 0;
  for (i=0;i<MAX_CONTINENT;i++)
  {
    clan->donation_obj[i]   = 0;
    clan->recall[i]         = 0;
  }
  clan->donation_obj[0]     = 8400;
  clan->clan_immortal       = str_dup( "{gAa{Dr{wc{gh{wa{gne{x", 
                                       clan->clan_immortal );
  clan->symbol              = str_dup( "[ empty ]", clan->symbol );
  clan->roster_style        = str_dup( "{x[%o%l %r %c] %n%t{x %s ?l{D[%d]{x %g!l{x", clan->roster_style );
  clan->donation_spent      = 0;
  clan->donation_total      = 0;
  clan->donation_balance    = 0;
  clan->locker              = 0;
  clan->roster              = NULL;
  clan->rank                = NULL;


  clan->next = clan_free;
  clan_free = clan;

  return clan;
}
