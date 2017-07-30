/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik Staerfeldt, Tom Madsen, and Katja Nyboe.  *
 *									   *
 *  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either    *
 *  of these copyright notices. 					   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/

/***************************************************************************
 *	ROM 2.4 is copyright 1993-1998 Russ Taylor			   *
 *	ROM has been brought to you by the ROM consortium		   *
 *	    Russ Taylor (rtaylor@hypercube.org) 			   *
 *	    Gabrielle Taylor (gtaylor@hypercube.org)			   *
 *	    Brian Moore (zump@rom.org)					   *
 *	By using this code, you have agreed to follow the terms of the	   *
 *	ROM license, in the file Rom24/doc/rom.license			   *
 ***************************************************************************/

/***************************************************************************
 *  Abysmal Realms is copyright (C) 1997-1998 by Dennis Reichel and	   *
 *  Christopher Eaker as a work in progress.  This code is not to be used  *
 *  or distributed except with the express permission of the authors.	   *
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
#include <time.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fnmatch.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"


/*
 * Local structures.
 */
BAN_DATA *ban_list = NULL;


/*
 * Local functions.
 */
void save_bans			args( ( void ) );

/*
 * Long-to-flag.  Returns string in the form fread_flag reads.
 * Can be called up to 5 times within a single statement.
 */
char *ltof( long flags )
{
    static char		buf[5][33];
    static sh_int	cnt;
    long		offset;
    int			i = 0;

    if ( ++cnt > 4 )
	cnt = 0;

    if ( flags == 0 )
    {
	strcpy( buf[cnt], "0" );
	return buf[cnt];
    }

    /* 32 -- number of bits in a long */
    buf[cnt][0] = '\0';
    for ( offset = 0L; offset < 32; offset++ )
    {
	if ( IS_SET( flags, 1L << offset ) )
	{
	    if ( offset <= 'Z' - 'A' )
		buf[cnt][i++] = 'A' + offset;
	    else
		buf[cnt][i++] = 'a' + offset - ( 'Z' - 'A' + 1 );
	}
    }
    buf[cnt][i] = '\0';

    return buf[cnt];
}

void ban_update( void )
{
    char	buf[MAX_STRING_LENGTH];
    BAN_DATA	*pBan;
    BAN_DATA	*pBan_next;
    BAN_DATA	*pBan_prev = NULL;
    bool	found = FALSE;

    for ( pBan = ban_list; pBan; pBan = pBan_next )
    {
	pBan_next = pBan->next;

	if ( !IS_SET( pBan->ban_flags, BAN_DURATION )
	||   pBan->date_stamp >= current_time - pBan->duration * 60 * 60 )
	{
	    pBan_prev = pBan;
	    continue;
	}

	if ( pBan_prev == NULL )
	    ban_list = pBan_next;
	else
	    pBan_prev->next = pBan_next;

	found = TRUE;

	logit( "Ban duration expired for %s, lvl %d.",
	    pBan->name, pBan->level );
	mprintf( sizeof(buf), buf, "Ban lifted on %s, duration expired.", pBan->name );
	wiznet( buf, NULL, NULL, WIZ_PENALTIES, 0, pBan->level );

	free_ban( pBan );
    }

    if ( found )
	save_bans();
}


void save_bans( void )
{
    BAN_DATA	*pBan;
    FILE	*fp;
    bool	found = FALSE;

    fclose( fpReserve );
    nFilesOpen--;
    if ( ( fp = fopen( BAN_FILE, "w" ) ) == NULL )
    {
	bug( "Save_bans: fopen", 0 );
	perror( BAN_FILE );
	fpReserve = fopen( NULL_FILE, "r" );
	nFilesOpen++;
	return;
    }

    for ( pBan = ban_list; pBan; pBan = pBan->next )
    {
	if ( IS_SET( pBan->ban_flags, BAN_PERMANENT ) )
	{
	    found = TRUE;
	    fprintf( fp, "%-24s %-2d %s %ld %d\n",
		pBan->name,
		pBan->level,
		ltof( pBan->ban_flags ),
		pBan->date_stamp,
		IS_SET( pBan->ban_flags, BAN_DURATION )
		    ? pBan->duration : 0 );
	}
     }

     fclose( fp );
     nFilesOpen--;
     fpReserve = fopen( NULL_FILE, "r" );
     nFilesOpen++;
     if ( !found )
	unlink( BAN_FILE );
}


void load_bans( void )
{
    FILE	*fp;
    BAN_DATA	*ban_last;

    if ( ( fp = fopen( BAN_FILE, "r" ) ) == NULL )
	return;
    nFilesOpen++;
    ban_last = NULL;
    for ( ; ; )
    {
	BAN_DATA *pBan;

	if ( feof( fp ) )
	{
	    fclose( fp );
            nFilesOpen--;
	    return;
	}

	pBan = new_ban();

	pBan->name	 = str_dup( fread_word( fp ), pBan->name );
	pBan->level	 = fread_number( fp );
	pBan->ban_flags  = fread_flag( fp );
	pBan->date_stamp = fread_number( fp );
	pBan->duration	 = fread_number( fp );
	fread_to_eol( fp );

	/*
	 * Convert prefix/suffix flags back to "*" in the string.
	 */
	{
	    char buf[MAX_INPUT_LENGTH];
	    if ( IS_SET( pBan->ban_flags, BAN_PREFIX )
	    ||	 IS_SET( pBan->ban_flags, BAN_SUFFIX ) )
	    {
		buf[0] = '\0';
		if ( IS_SET( pBan->ban_flags, BAN_PREFIX ) )
		    strcat( buf, "*" );
		strcat( buf, pBan->name );
		if ( IS_SET( pBan->ban_flags, BAN_SUFFIX ) )
		    strcat( buf, "*" );
		free_string( pBan->name );
		pBan->name = str_dup( buf , pBan->name);
	    }
	    REMOVE_BIT( pBan->ban_flags, BAN_PREFIX|BAN_SUFFIX );
	}

	if ( ban_list == NULL )
	    ban_list = pBan;
	else
	    ban_last->next = pBan;
	ban_last = pBan;
    }
}


bool check_ban( char *site, int type )
{
    BAN_DATA	*pBan;
    char	host[MAX_STRING_LENGTH];

    if ( ( site == NULL )
    || (site[0] == '\0') )
      return FALSE;
      
    strcpy( host, capitalize( site ) );
    host[0] = LOWER( host[0] );

    for ( pBan = ban_list; pBan; pBan = pBan->next )
    {
	if ( ( IS_SET( pBan->ban_flags, type ) || type == 0 )
	&&   !fnmatch( pBan->name, host, FNM_PERIOD ) )
	    return TRUE;
    }

    return FALSE;
}


void ban_site( CHAR_DATA *ch, char *argument, bool fPerm )
{
  char	buf[MAX_STRING_LENGTH];
  char	buf2[MAX_STRING_LENGTH];
  char	arg1[MAX_INPUT_LENGTH];
  char	arg2[MAX_INPUT_LENGTH];
  char	arg3[MAX_INPUT_LENGTH];
  char	arg4[MAX_INPUT_LENGTH];
  char	name[MAX_INPUT_LENGTH];
  BUFFER	*buffer;
  BAN_DATA	*pBan;
  BAN_DATA	*pBan_next;
  BAN_DATA	*pBan_prev = NULL;
  bool	found = FALSE;
  int64 	type;
  int 	ban_duration = 0;

  argument = one_argument( argument, arg1 );	/* Site */
  argument = one_argument( argument, arg2 );	/* Type */
  arg3[0] = '\0';
  while ( argument[0] != '\0' )
    {
      argument = one_argument( argument, arg3 );
      if ( arg3[0] == '\0' || is_number( arg3 ) )
	break;
      strcat( arg2, " " );
      strcat( arg2, arg3 );
      arg3[0] = '\0';
    }
  argument = one_argument( argument, arg4 );	/* Time unit. */

  if ( arg1[0] == '\0' )
    {
      if ( ban_list == NULL )
	{
	  send_to_char( "No sites banned at this time.\n\r", ch );
	  return;
	}
      buffer = new_buf();

      bstrcat( buffer,
	       "Banned sites                   Lvl Type                  "
	       "Status  Date\n\r"
	       "========================================================="
	       "====================\n\r" );

      for ( pBan = ban_list; pBan; pBan = pBan->next )
	{
	  if ( IS_SET( pBan->ban_flags, BAN_DURATION ) )
	    {
	      ban_duration = (int)difftime( pBan->date_stamp,
					    current_time - pBan->duration * 60 * 60 );
	      ban_duration = UMAX( 0, ban_duration / 60 / 60 );

	      if ( ban_duration < 1 )
		strcpy( buf, "< 1 h" );
	      else if ( ban_duration <= 48 )
		mprintf( sizeof(buf), buf, "%d h", ban_duration + 1 );
	      else
		mprintf( sizeof(buf), buf, "%d d", ( ban_duration + 12 ) / 24 );
	    }
	  else if ( IS_SET( pBan->ban_flags, BAN_PERMANENT ) )
	    strcpy( buf, "perm" );
	  else
	    strcpy( buf, "temp" );

	  strftime( buf2, MAX_STRING_LENGTH,
		    "%b %d, %Y", localtime( &pBan->date_stamp ) );

	  bprintf( buffer, "%-30s %3d %-20s  %-6s  %s\n\r",
		   pBan->name, pBan->level,
		   flag_string( ban_flags, pBan->ban_flags ), buf, buf2 );
	}

      page_to_char( buf_string( buffer ), ch );
      free_buf( buffer );
      return;
    }

  /* Find out what type of ban. */
  if ( arg2[0] == '\0' )
    type = BAN_ALL;
  else if ( ( type = flag_value( ban_flags, arg2 ) ) == NO_FLAG )
    {
      send_to_char(
		   "Acceptable ban types are all, newbies, autolog, "
		   "channels, notes, and permit.\n\r", ch );
      return;
    }

  /* Exclusive ban types. */
  if ( IS_SET( type, BAN_ALL ) )
    REMOVE_BIT( type, BAN_AUTOLOG|BAN_NOCHAN|BAN_NEWBIES|BAN_NONOTE|BAN_PERMIT );
  else if ( IS_SET( type, BAN_NEWBIES ) )
    REMOVE_BIT( type, BAN_NOCHAN|BAN_NONOTE|BAN_PERMIT );
  else if ( IS_SET( type, BAN_PERMIT ) )
    REMOVE_BIT( type, BAN_NOCHAN|BAN_NEWBIES|BAN_NONOTE );

  /* Setting a ban duration. */
  if ( arg3[0] != '\0' )
    {
      int value;

      if ( !fPerm )
	{
	  send_to_char( "Use PERMBAN to set a timed duration ban.\n\r", ch );
	  return;
	}

      if ( !is_number( arg3 ) )
	{
	  send_to_char(
		       "Must specify a number for a time duration in days.\n\r",
		       ch );
	  return;
	}

      if ( ( value = atoi( arg3 ) ) <= 0 )
	{
	  send_to_char( "Duration must be greater than zero.\n\r", ch );
	  return;
	}

      /* Ban duration is saved in hours. */
      if ( arg4[0] == '\0' || !str_prefix( arg4, "days" ) )
	ban_duration = value * 24;
      else if ( !str_prefix( arg4, "hours" ) )
	ban_duration = value;
      else
	ban_duration = value * 24;
    }

  strcpy( name, capitalize( arg1 ) );
  name[0] = LOWER( name[0] );

  for ( pBan = ban_list; pBan; pBan_prev = pBan, pBan = pBan_next )
    {
      pBan_next = pBan->next;

      if ( !str_cmp( name, pBan->name ) )
	{
	  if ( pBan->level > get_trust( ch ) )
	    {
	      send_to_char( "That ban was set by a higher power.\n\r", ch );
	      return;
	    }
	  else
	    {
	      found = TRUE;
	      if ( pBan_prev == NULL )
		ban_list = pBan_next;
	      else
		pBan_prev->next = pBan_next;
	      free_ban( pBan );
	      send_to_char( "Modifying an existing ban:\n\r", ch );
	      continue;
	    }
	}
    }

  if ( !found )
    send_to_char( "Adding a new ban:\n\r", ch );

  pBan		= new_ban();
  pBan->name		= str_dup( name , pBan->name);
  pBan->level 	= get_trust( ch );
  pBan->ban_flags	= type;
  pBan->duration	= ban_duration;

  if ( fPerm )
    SET_BIT( pBan->ban_flags, BAN_PERMANENT );
  if ( ban_duration > 0 )
    SET_BIT( pBan->ban_flags, BAN_DURATION );

  pBan->next	= ban_list;
  ban_list	= pBan;
  save_bans();

  if ( ban_duration > 0 && ban_duration < 24 )
    mprintf( sizeof(buf), buf, "for %d hour%s",
	     ban_duration, ban_duration == 1 ? "" : "s" );
  else if ( ban_duration > 0 && ban_duration % 24 == 0 )
    mprintf( sizeof(buf), buf, "for %d day%s",
	     ban_duration / 24, ban_duration / 24 == 1 ? "" : "s" );
  else if ( ban_duration > 0 )
    mprintf( sizeof(buf), buf, "for %d day%s and %d hour%s",
	     ban_duration / 24, ban_duration / 24 == 1 ? "" : "s",
	     ban_duration % 24, ban_duration % 24 == 1 ? "" : "s" );
  else if ( fPerm )
    strcpy( buf, "permanently" );
  else
    strcpy( buf, "temporarily" );

  printf_to_char( ch, "%s has been banned %s.\n\r", pBan->name, buf );

  ban_update();
}


void do_ban( CHAR_DATA *ch, char *argument )
{
  ban_site( ch, argument, FALSE );
}


void do_permban( CHAR_DATA *ch, char *argument )
{
  ban_site( ch, argument, TRUE );
}


void do_allow( CHAR_DATA *ch, char *argument )
{
  BAN_DATA	*pBan;
  BAN_DATA	*pBan_next;
  BAN_DATA	*pBan_prev = NULL;
  char	arg[MAX_INPUT_LENGTH];
  one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
      send_to_char( "Remove which site from the ban list?\n\r", ch );
      return;
    }

  for ( pBan = ban_list; pBan; pBan_prev = pBan, pBan = pBan_next )
    {
      pBan_next = pBan->next;

      if ( !str_cmp( arg, pBan->name ) )
	{
	  if ( pBan->level > get_trust( ch ) )
	    {
	      send_to_char(
			   "You are not powerful enough to lift that ban.\n\r", ch );
	      return;
	    }

	  if ( pBan_prev == NULL )
	    ban_list = pBan_next;
	  else
	    pBan_prev->next = pBan_next;

	  printf_to_char( ch, "Ban on %s lifted.\n\r", pBan->name );

	  free_ban( pBan );
	  save_bans();
	  return;
	}
    }

  send_to_char( "That site is not banned.\n\r", ch );
}
