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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/uio.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include "merc.h"
#include "tables.h"
#include "clan.h"
#include "lookup.h"
#include "recycle.h"
#include "interp.h"
#include <dirent.h>
#include "olc.h"                //Added to load in new_clan();
CLAN_DATA *load_clan( char *argument );
void       save_clan( CLAN_DATA *clan );

int     actual_num_clans;
struct    clan_type    clan_table[MAX_CLAN];
struct  clan_roster_st clan_roster[MAX_CLAN];
/* Added for writing to clan files */
//void    fwrite_clan args( ( CHAR_DATA *ch,  FILE *fp ) );

/*+
 * strip_string - Strips Leading and trailing Blanks
 *
 * SYNOPSIS:
 *         strip_string(char *string)
 *
 * DESCRIPTION:
 *         {strip_string} will strip the leading and trailing blanks in
 *    any string sent to it. This is done by finding the Position of
 *    the last non-blank in the string. Then finding the position of
 *    the first non-blank and then copying the rest into the return
 *    value. 
 *
 *  SEE ALSO:
 *
 */
int strip_string( char *input_string )
{
  char    temp[512],temp1[512];
  int        i;
  int     j,k,l;
  strcpy( temp1, input_string );
  for ( i = strlen( input_string )-1;  i >= 0; i-- )
    if ( !isspace( input_string[i] ) /*!=' '*/)
      break;
  for ( k = 0; k < strlen( input_string ); k++ )
    if ( !isspace( input_string[k] ) /*!=' '*/)
      break;
  for ( l = 0, j = k; j <= i; j++, l++ )
    temp[l] = input_string[j];
  temp[l] = '\0';
  strcpy( input_string, temp );
  return( 0 );
}
/*+
 * str_normalize_white_space - removes all existing white space from between strings
 *                             in an input_string and replaces them with a single
 *                             space.
 *
 * SECTION: libstr
 *
 * SYNOPSIS:
 *     str_normalize_white_space(char *input_string)
 *          char *input_string - The string to contain the data to be
 *                               normalized.
 *
 *  DESCRIPTION:
 *    Beginning at the beginning of the string, each character is
 *      checked to see if it is a white space character. If the 
 *      character is a white space character it is replaced by a 
 *      space.
 *
 *  SEE ALSO:
 *      get_next_line()
 */
int str_normalize_white_space( char *input_string )
{
  int i = 0;
  
  for( i=0; i < strlen( input_string ); i++ )
    if ( isspace( input_string[i] ) )
    input_string[i] = ' ';
  return( 0 );
}
/*+
 * str_normalize_white_space - removes all existing white space from between strings
 *                             in an input_string and replaces them with a single
 *                             space.
 *
 * SECTION: libstr
 *
 * SYNOPSIS:
 *     str_normalize_white_space(char *input_string)
 *          char *input_string - The string to contain the data to be
 *                               normalized.
 *
 *  DESCRIPTION:
 *    Beginning at the beginning of the string, each character is
 *      checked to see if it is a white space character. If the 
 *      character is a white space character it is replaced by a 
 *      space.
 *
 *  SEE ALSO:
 *      get_next_line()
 */
int str_cleanup_extra_white_space( char *input_string )
{
  int i = 0;
  int k = 0;
  bool is_found = FALSE;
  char new_string[MSL];
  
  for( i=0; i < strlen( input_string ); i++ ) 
  {
    if ( isspace( input_string[i] ) ) 
    {
      if ( !is_found ) 
      {
        new_string[k++] = input_string[i];
        is_found = TRUE;
      }
      continue;
    }
    new_string[k++] = input_string[i];
    is_found = FALSE;
  }
  new_string[k] = '\0';
  strcpy( input_string,new_string );
  return( 0 );
}
/*+
 * get_next_line - gets the next line of data from an input file
 *
 * SECTION: libstr
 *
 * SYNOPSIS:
 *     get_next_line( FILE *fp, char *str)
 *          FILE *fp  - pointer to the file to be read from and the
 *                      position in that file to begin reading from.
 *          char *str - The string to contain the data read in from
 *                      the file pointed to by fp.
 *
 *  DESCRIPTION:
 *    Beginning at the position pointed to by fp, get_next_line
 *      will read in one line at a time.  If the line read in begins
 *      with a "#" then that line is considered to be a comment line
 *      and is therefore ignored and another line is read in.  Lines
 *      that contain nothing but blanks and/or carriage returns(\n)
 *      are also ignored and another line is read in.  The function
 *      either return TRUE and str will contain valid data or the
 *      function will return EOF.  When the function returns EOF the
 *      contents of str will be the same as what was passed in.
 *
 *  SEE ALSO:
 */
int get_next_line( FILE *fp, char *instr )
{
  char temp_str[512];

  while ( fgets( temp_str, 512, fp ) != NULL )
    if ( temp_str[0] != '#')                    /* if linr is not a comment */
    {
      strip_string( temp_str );                 /* strip off leading and trailing blanks */
      if ( strcmp( temp_str, "" ) )             /* if there is data */
      {
        strncpy( instr, temp_str, strlen( temp_str ) ); /* copy to str */
        instr[strlen( temp_str )] = '\0';       /* make str the right length */
        str_normalize_white_space( instr );     /* replace all white spaces  */
        return TRUE;
      }
    }
  return EOF;
}

/*
 * Name: find_clannie( ch )
 * Use: Will find and return clannie if possible.
 */
ROSTER_DATA *find_clannie( CHAR_DATA *ch )
{
    ROSTER_DATA *clannie = NULL;

    if ( !ch->clan )
        return NULL;

    for ( clannie = ch->clan->roster ; clannie ; clannie = clannie->next )
    {
        if ( !str_cmp( ch->name, clannie->name ) )
            return clannie;
    }

    return NULL;
}

/*
 * Name: find_rank_symbol( ch )
 * Use: Will find and return rank-symbol if possible.
 */
char *find_rank_symbol( CHAR_DATA *ch )
{
//    RANK_DATA *rank;

    if ( !ch->clan && !ch->clan->rank )
        return NULL;

/*    for ( rank = ch->clan->rank; rank; rank = rank->next )
        if ( ch->pcdata->clan_rank == rank->level
        && ch->gameclass == rank->rank_flags )
            break;
            return rank;
*/
    return NULL;
}

/*
 * Name: find_clannie_rank( ch )
 * Use: Will sort through a clannie's roster and return
 *      his rank slot.
 */
RANK_DATA *find_clannie_rank( CHAR_DATA *ch )
{
    RANK_DATA   *rank     = NULL;

    if ( !ch->clan )
        return NULL;
    
    for ( rank = ch->clan->rank; rank; rank = rank->next )
        if ( ch->pcdata->clan_rank == rank->level )
            return rank;

    return NULL;        
}    
            

// Added to handle the new clan-system 2006-10-23 by Merak
void add_clannie( CLAN_DATA *clan, ROSTER_DATA *clannie )
{// This is just preliminary, the list is to be sorted..

    clannie->next = clan->roster;
    clan->roster  = clannie;
    clan->actual_members++;
}

void load_clan_info( char *clan_filename )
{
  FILE *fp;
  char buf[512];
  int i = 0;


  if ( ( fp = fopen( clan_filename, "r" ) ) != NULL )
  {
    nFilesOpen++;
    get_next_line( fp, buf );
    strcpy( clan_table[0].name, "" );
    strcpy( clan_table[0].who_name, "" );
    clan_table[0].independent = 1;
    clan_table[0].hall = 3054;
    actual_num_clans = atoi( buf );
    for ( i = 1; i <= actual_num_clans; i++ )
    {
      if ( i >= MAX_CLAN )
      {
        fclose( fp );
        nFilesOpen--;
        return;
      }
      get_next_line( fp, buf );
      strcpy( clan_table[i].name, buf );
      get_next_line( fp, buf );
      strcpy( clan_table[i].who_name, buf );
      get_next_line( fp, buf );
      clan_table[i].hall = atoi( buf );
      get_next_line( fp, buf );
      clan_table[i].independent = atoi( buf );
      get_next_line( fp, buf );
      clan_table[i].total_dia = atoi( buf );
      get_next_line( fp, buf );
      clan_table[i].used_dia = atoi( buf );
      get_next_line( fp, buf );
      clan_table[i].free_dia= atoi( buf );
      get_next_line( fp, buf );
      strcpy( clan_table[i].patron, buf );
    }
    fclose( fp );
    nFilesOpen--;
  }
}

void do_saveclan( CHAR_DATA *ch, char *argument )
{
  save_clan_info(CLAN_FILE);
  save_clan_list(ROSTER_FILE);
  send_to_char("{WClan and Roster info Saved.{x\n\r",ch);
}


void save_clan_info( char *clan_filename )
{
  FILE *fp;
  int i = 0;

  if ( ( fp = fopen( clan_filename, "w" ) ) != NULL )
  {
    nFilesOpen++;
    fprintf( fp, "# Actual Number clans\n" );
    fprintf( fp, "%d\n", actual_num_clans );
    for ( i = 1; i <= actual_num_clans; i++ )
    {
      fprintf( fp, "# Name of clan \n" );
      fprintf( fp, "%s \n", clan_table[i].name );
      fprintf( fp, "# Clan Color Name\n" );
      fprintf( fp, "%s\n", clan_table[i].who_name );
      fprintf( fp, "# Hall Vnum\n" );
      fprintf( fp, "%d\n", clan_table[i].hall );
      fprintf( fp, "# Is Clan Independent\n" );
      fprintf( fp, "%d\n", clan_table[i].independent );
      fprintf( fp, "# Total Diamonds\n" );
      fprintf( fp, "%d\n", clan_table[i].total_dia );
      fprintf( fp, "# Used Diamonds\n" );
      fprintf( fp, "%d\n", clan_table[i].used_dia );
      fprintf( fp, "# Free diamonds\n" );
      fprintf( fp, "%d\n", clan_table[i].free_dia );
      fprintf( fp, "# Clan Patron name\n" );
      fprintf( fp, "%s\n", clan_table[i].patron );
    }
    fclose( fp );
    nFilesOpen--;
  }
}

void do_clanname( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char( "Syntax: clanname clan_num name", ch );
      return;
    }

  strncpy( clan_table[atoi( arg1 )].name, arg2, MSL );
  printf_to_char( ch, "Clan #%s is now named %s\n\r", arg1, arg2 );
  return;
}

void do_clandesc( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );

  if ( arg1[0] == '\0' || argument[0] == '\0')
    {
      send_to_char( "Syntax: clandesc clan_num Desc", ch );
      return;
    }

  strncpy( clan_table[atoi( arg1 )].who_name, argument, MSL );
  printf_to_char( ch, "Clan #%s is now described as %s\n\r", arg1, argument );
  return;
}

void do_clanhall( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char( "Syntax: clanhall clan_num vnum", ch );
      return;
    }

  clan_table[atoi( arg1 )].hall = atoi( arg2 );
  printf_to_char( ch, "Clan #%s hall is now vnum %s\n\r", arg1, arg2 );
  return;
}

void do_clanstate( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
      send_to_char( "Syntax: clanstatus clan_num status(0= FALSE/1=TRUE)", ch );
      return;
    }

  clan_table[atoi( arg1 )].independent = atoi( arg2 );
  printf_to_char( ch, "Clan #%s status is now %s\n\r", arg1, arg2 );
  return;
}

void do_clantotal( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
      send_to_char( "Syntax: clantotal clan_num diamonds", ch );
      return;
    }

  clan_table[atoi( arg1 )].total_dia = atoi( arg2 );
  printf_to_char( ch, "Clan #%s total is now %s\n\r", arg1, arg2 );
  return;
}


void do_clanfree( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
      send_to_char( "Syntax: clanfree clan_num diamonds", ch );
      return;
    }

  clan_table[atoi( arg1 )].free_dia = atoi( arg2 );
  printf_to_char( ch, "Clan #%s free diamonds is now %s\n\r", arg1, arg2 );
  return;
}

void do_clanused( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
  {
    send_to_char( "Syntax: clanused clan_num diamonds", ch );
    return;
  }
  clan_table[atoi( arg1 )].used_dia = atoi( arg2 );
  printf_to_char( ch, "Clan #%s used diamonds is now %s\n\r", arg1, arg2 );
  return;
}

void do_clanlist( CHAR_DATA *ch, char *argument )
{
  int i = 0;
  send_to_char( "{YListing of Clans:{x\n\r", ch );
  send_to_char( "{W  #     DESC       NAME               HALL  STATUS TOTAL FREE USED PATRON{x\n\r", ch );
  send_to_char( "{C-------------------------------------------------------------------------------{x\n\r", ch );
  for ( i = 1; i <= actual_num_clans; i++ )
    {
      printf_to_char( ch, "{W%3d %s %-20s ",
            i, clan_table[i].who_name, clan_table[i].name );
      printf_to_char( ch, "%5d %5d %5d %5d %5d %s{x\n\r", 
            clan_table[i].hall,
            clan_table[i].independent, clan_table[i].total_dia,
            clan_table[i].free_dia, clan_table[i].used_dia,
            clan_table[i].patron );
    }
  return;
}

void do_clanpatron( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );

  if ( arg1[0] == '\0' || argument[0] == '\0' )
    {
      send_to_char( "Syntax: clanpatron clan_num patron", ch );
      return;
    }

  strncpy( clan_table[atoi( arg1 )].patron, argument, MSL );
  printf_to_char( ch, "Clan #%s patron is now %s\n\r", arg1, argument );
  return;
}


void do_clan_sac( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  int silver;
    
  /* variables for AUTOSPLIT */
  CHAR_DATA *gch;
  int members;
  char buffer[100];

  /* Assume we are in a clan!! */
    
  one_argument( argument, arg );

  if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
  {
    mprintf(sizeof(buf), buf,
                "$n offers $mselt to %s, who graciously declines.",
                ch->clan->clan_immortal );
    act( buf, ch, NULL, NULL, TO_ROOM );
    printf_to_char( ch,
                "%s appreciates your offer and may accept it later.\n\r",
                ch->clan->clan_immortal );
    return;
  }
  obj = get_obj_list( ch, arg, ch->in_room->contents );
  if ( obj == NULL )
  {
    send_to_char( "You can't find it.\n\r", ch );
    return;
  }

  if ( obj->item_type == ITEM_CORPSE_PC )
  {
    if (obj->contains)
    {
      printf_to_char(ch,"%s wouldn't like that.\n\r", ch->clan->clan_immortal );
      return;
    }
  }


  if ( !CAN_WEAR( obj, ITEM_TAKE ) || CAN_WEAR( obj, ITEM_NO_SAC ) )
  {
    act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
    return;
  }

  if ( !can_use_clan_obj( ch, obj ) )
  {
    send_to_char( "That belongs to another clan.\n\r", ch );
    return;
  }

  if ( obj->in_room != NULL )
  {
    for ( gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room )
      if ( gch->on == obj )
      {
        act( "$N appears to be using $p.",
        ch, obj, gch, TO_CHAR );
        return;
      }
  }
        
  silver = UMAX( 1, obj->level * 3 );

  if ( obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC )
    silver = UMIN( silver, obj->cost );

  printf_to_char( ch, 
      "%s gives you %s%s{x {wsi{Wl{Dv{wer{x coin%s for your sacrifice.\n\r",
      ch->clan->clan_immortal, silver == 0 ? "" : "{W",
      numcpy( buf, silver ), silver == 1 ? "" : "s" );
    
  ch->silver += silver;
    
  if ( IS_SET( ch->act, PLR_AUTOSPLIT ) )
  { /* AUTOSPLIT code */
    members = 0;
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
      if ( is_same_group( gch, ch ) )
        members++;

    if ( members > 1 && silver > 1 )
    {
      mprintf(sizeof(buffer), buffer, "%d", silver );
      do_function( ch, &do_split, buffer );    
    }
  }
  mprintf(sizeof(buf), buf, "$n sacrifices $p to %s.", 
            ch->clan->clan_immortal );
  act( buf, ch, obj, NULL, TO_ROOM );

  mprintf( sizeof(buf), buf, "%s sends up %s as a burnt offering.",
    capitalize( ch->name ),
    obj->short_descr );

  wiznet( buf, ch, NULL, WIZ_SACCING, 0, 0 );
  extract_obj( obj );
  return;
}

int recall_pk_fight( CHAR_DATA *ch )
{
  CHAR_DATA *ich;
    
  if ( IS_NPC( ch ) )
    return TRUE;
  if ( !( ch->clan ) )
    return TRUE;

  if ( ch->fighting )
  {
    if ( !IS_NPC( ch->fighting ) )
    {
      if ( ch->clan )
        printf_to_char(ch,"%s deems that a cowardly act and ignores you.\n\r",
               ch->clan->clan_immortal );
      else
        printf_to_char(ch,"%s deem that a cowardly act and ignores you.\n\r",
               "The Gods" );
      return FALSE;
    }
    
    for ( ich = ch->in_room->people; ich; ich = ich->next_in_room )
    {
      if ( !IS_NPC( ich ) && ich->fighting == ch ) 
      {
        if ( ch->clan )
          printf_to_char(ch,"%s deems that a cowardly act and ignores you.\n\r",
                    ch->clan->clan_immortal);
        else    
          printf_to_char(ch,"%s deems that a cowardly act and ignores you.\n\r",
               "The Gods");

        return FALSE;
      }
    }
  }
  return TRUE;
}

int recall( CHAR_DATA *ch, bool cost_moves, int room )
{
  ROOM_INDEX_DATA *location;

  if ( IS_NPC( ch ) && !IS_PET( ch ) )
  {
      send_to_char( "Only players or pets can recall.\n\r", ch );
      return FALSE;
  }

  if ( IS_PET( ch ) )
    ch->position = POS_STANDING;
    
  if ( IS_AFFECTED( ch, AFF_CHARM ) && !IS_PET( ch ) )
  {
      send_to_char( "You cannot leave your nice master.\n\r", ch );
      return FALSE;
  }

  if ( IS_SAFFECTED( ch, SAFF_ADRENALIZE ) && !IS_PET( ch ) )
  {
      send_to_char( "You cannot call upon your god.\n\r", ch );
      return FALSE;
  }

  if ( IS_AFFECTED( ch, AFF_CURSE) && !IS_PET( ch ) )
  {
      send_to_char( "You are cursed.\n\r", ch );
      if ( ch->master )
        send_to_char( "Your pet is cursed and cannot follow you.\n\r", ch );
      return FALSE;
  }

  if ( cost_moves )
    if ( ch->move < ch->level && !IS_PET( ch ) 
    && ( ch->level >= LEVEL_RECALL_COSTS ) )
    {
        if ( !IS_GHOST( ch ) ) 
        {
          send_to_char("You do not have enough moves to recall.\n\r",ch);
          return FALSE;
        }
    }

  if ( ( location = get_room_index( room ) ) == NULL )
  {
      send_to_char( "You are completely lost.\n\r", ch );
      return FALSE;
  }

  if ( ch->in_room == location )
  {
    printf_to_char(ch, "You are already at recall!\n\r" );
    return FALSE;
  }

  //if ( ch->in_room->area->continent != 0
  //&&   !IS_IMMORTAL( ch ) )
  //{
  //  send_to_char( "You are in lands too far from home to recall.\n\r", ch );
  //  return FALSE;
  //}

  if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
  ||   IS_AFFECTED(ch, AFF_CURSE))
  {
      if ( ch->clan )
        printf_to_char( ch, "%s has forsaken you.\n\r", 
                        ch->clan->clan_immortal );
      else
        printf_to_char (ch, "%s has forsaken you.\n\r", "{CM{cir{Ml{mya{x" );
      return FALSE;
  }

  if ( !recall_pk_fight( ch ) )
    return FALSE;

  return TRUE;
}


void do_recall( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *location;
  int skill;

  act( "$n prays for transportation!", ch, NULL, NULL, TO_WIZ_ROOM );

  if ( !recall( ch, TRUE, get_recall_room( ch ) ) )
    return;

  location = get_room_index( get_recall_room( ch ) );

  if (location->vnum == ch->in_room->vnum)
  {
    printf_to_char(ch, "You are already at recall!\n\r" );
    return;
  }

  if ( !IS_PET( ch ) )
  {
    skill = get_skill( ch, gsn_recall );
    if ( ( victim = ch->fighting ) != NULL )
    {
      if ( number_percent() < 80 * skill / 100 )
      {
        check_improve( ch, gsn_recall, FALSE, 4 );
        printf_to_char (ch, "You failed!\n\r" );
        return;
      }

      if ( !IS_NPC( ch ) && !IS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
      {
        ch->questdata->curr_points -= 2;
        if ( ch->questdata->curr_points < 0 )
          ch->questdata->curr_points = 0;
        printf_to_char(ch, "You recall from combat!  You lose 2 glory.\n\r" );
      }
      check_improve( ch, gsn_recall, TRUE, 3 );
      stop_fighting( ch, TRUE );
    }
    else
    {
      if ( number_percent() > skill )
      {
        send_to_char( "You failed!\n\r", ch );
        check_improve( ch, gsn_recall, FALSE, 6 );
        return;
      }
      else
      {
        check_improve( ch, gsn_recall, TRUE, 6 );
      }
    }

    if ( ch->level >= LEVEL_RECALL_COSTS )
      ch->move -= ch->level;
    if ( ( ch->move <= 0 ) && ( ch->level >= LEVEL_RECALL_COSTS ) )
      ch->move = 0;

    WAIT_STATE( ch, 10 );
  } // end if is not pet

  act( "$n disappears.", ch, NULL, NULL, TO_WIZ_ROOM );

  if ( ( ch->level <= LEVEL_HERO ) && ( ch->incog_level ) )
  {
    send_to_char( "{RRemoving {WCloaking{G: Leaving Area.{x\n\r", ch );
    ch->incog_level = 0;
  }

  move_to_room( ch, location );
  do_function( ch, &do_look, "" );
  act( "$n appears in the room.", ch, NULL, NULL, TO_WIZ_ROOM );

  if (ch->pet && (ch->pet->position > POS_RESTING) )
    do_function( ch->pet, &do_recall, "" );

  return;
}

int get_clan_dia_cost( CLAN_DATA *clan )
{
  int cost=0;
  {
/*  int actclan=0;
  *//* Cost in diamonds :) */
/*  if ((actclan = num_act_clan(clan)) > NUM_CLANNIES) {
  */  /* charge em :) */ /*
    cost = (25 * (actclan-NUM_CLANNIES))  - 20;
    if (clan == clan_lookup("goddess")) */
      cost = 0;
  }
  return cost;
}

void do_unguild( CHAR_DATA *ch, char *argument )
{
    char        arg[MIL];
    char        buf[MSL];
    char        buf1[MSL];
    char        note_subject[MSL];
    char        note_body[MSL];
    char        note_to[MSL];
    CHAR_DATA   *victim;
    ROSTER_DATA *clannie    = NULL;
    RANK_DATA   *rank       = NULL;
    CLAN_DATA   *old_clan   = NULL;
    CLAN_DATA   *clan       = NULL;
    bool        choffline   = FALSE;
    bool        isChar      = FALSE;
    int         old_rank    = 0;
    DESCRIPTOR_DATA d;

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR( arg ) )
    {
        send_to_char( "Syntax: unguild <clan member>\n\r", ch );
        return;
    }


    if ( !IS_IMMORTAL( ch )
    &&   ( rank = find_clannie_rank( ch ) )
    &&   !IS_SET( rank->rank_flags, RANK_LEADER ) )
    {
        send_to_char( "You are not a clan leader.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    { /* Char not online.  Attempt to load offline character */
      arg[0] = UPPER(arg[0]);

      isChar = load_char_obj(&d, arg); /* char pfile exists? */

      if (!isChar)
      {
        send_to_char("That person does not exist.\n\r", ch);
        return;
      }

      choffline = TRUE;

      if ( strlen(argument) < 5 )
      {
        send_to_char("You must provide a reason to unguild an offline player.\n\r", ch);
        send_to_char("unguild [player] [reason]\n\r", ch);
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

      /* move player out of clan hall */
      if ( d.character->in_room != NULL )
      {
        d.character->old_room = d.character->in_room;
        if ( d.character->alignment < -500 ) { char_to_room( d.character, get_room_index( ROOM_VNUM_EVIL_RECALL ) ); }
        else if ( d.character->alignment > 500 ) { char_to_room( d.character, get_room_index( ROOM_VNUM_TEMPLE ) ); }
        else { char_to_room( d.character, get_room_index (ROOM_VNUM_NEUT_RECALL ) ); }
      }

      if (d.character->pet != NULL)
      {
        char_to_room(d.character->pet,d.character->in_room);
      }

      victim = d.character;
    } /* /taeloch: offline char */

    if ( !victim->clan )
    {
        send_to_char( "They are not even guilded, why would you do that?\n\r", ch );
        return;
    }

    if ( ( victim->clan != ch->clan )
    &&   !IS_IMMORTAL( ch ) )
    {
        send_to_char( "You can't unguild someone from another clan.\n\r", ch );
        return;
    }
    
    if ( !( clannie = find_clannie( victim ) ) )
    {
        send_to_char( "Clan member not found in the roster listing.\n\r", ch );
        return;
    }

    if ( !IS_IMMORTAL( victim ) )
    {
        mprintf( sizeof( buf ), buf, "%s has been removed from clan status.",
            victim->name );
        //do_function( ch, &do_info, buf );
        if ( choffline ) { strcat( buf, argument ); }
        log_string( buf );
    }

    printf_to_char( ch, "Removing %s from %s.\n\r",
        victim->name, victim->clan->name );
    send_to_char( "You have been unguilded.\n\r", victim );

    old_clan = victim->clan;
    old_rank = victim->pcdata->clan_rank;
    remove_clannie( victim );

    victim->pcdata->clan_rank     = 0;
    victim->pcdata->donated_dia   = 0;

    if ( old_rank > 1 )
    {
      if ( victim->level > LEVEL_HERO - 4
      &&   victim->level < LEVEL_HERO )
      {
        victim->pcdata->clanowe_dia   = victim->level * 4;
        victim->pcdata->clanowe_level = LEVEL_HERO;
        victim->pcdata->clanowe_clan  = old_clan->name;
      }
      else if ( victim->level >= LEVEL_HERO )
      {
        victim->pcdata->clanowe_dia   = victim->level * 8;
        victim->pcdata->clanowe_clan  = old_clan->name;
      }
      else
      {
        victim->pcdata->clanowe_dia   = victim->level * 4;
        victim->pcdata->clanowe_level = victim->level + 4;
        victim->pcdata->clanowe_clan  = old_clan->name;
      }
    }

    save_clan( old_clan );
    save_char_obj( victim, FALSE );

    if ( old_rank > 1 )
    {
      if ( victim->pcdata->clanowe_level > victim->level )
        sprintf( buf1,
          "\n\r\n\r%s, you must obtain level %d and donate %d gems before you may join\n\ranother clan.\n\r",
            victim->name, victim->pcdata->clanowe_level, victim->pcdata->clanowe_dia );
      else
        sprintf( buf1,
          "\n\r\n\r%s, you must donate %d gems before you may join another clan.\n\r",
            victim->name, victim->pcdata->clanowe_dia );
    }
    else
      sprintf( buf1, "%s", "\n\r" );

    if ( choffline )
    {
      mprintf(sizeof(note_subject), note_subject,
        "%s's removal from %s.", victim->name, old_clan->symbol );
      mprintf(sizeof(note_body), note_body,
        "You have been removed from your clan while you were not logged in. If you\n\rfeel there was an error in this decision you may write a note to the clan\n\rleader or immortal.%s", buf1 );
      sprintf( note_to, "%s Immortal", victim->name );
      note_line( "{WClan NewsFaerie{x", NOTE_NOTE, note_to,
        note_subject, note_body );
    }
    else
    {
      if ( old_rank > 1 )
      {
        if ( victim->pcdata->clanowe_level > victim->level )
          printf_to_char( victim,
            "You must obtain level %d and donate %d gems before you may join another clan.\n\r",
              victim->pcdata->clanowe_level, victim->pcdata->clanowe_dia );
        else
          printf_to_char( victim,
            "You must donate %d gems before joining another clan.\n\r",
              victim->pcdata->clanowe_dia );
      }
    }

    if ( old_rank < 2 )
    {
      if (choffline) { do_function(victim, &do_quit,"imm-overide"); }
      return;
    }

    if ( !( clan = get_clan( "Wanderer", TRUE ) ) )
    {
        loggedf( "Wanderer failed to load, leaving %s unclanned.",
            victim->name );
        return;
    }
    else
    { // there is no rank 0
      victim->pcdata->clan_rank = 1;
    }

    if ( !IS_IMMORTAL( victim ) )
    {
        victim->clan = clan;
        victim->clan_name = str_dup( clan->name, victim->clan_name );
        clannie = new_clannie( );
        free_string( clannie->name );
        clannie->name    = str_dup( victim->name, clannie->name );
        free_string( clannie->title );
        clannie->title   = str_dup( victim->pcdata->title, clannie->title );
        free_string( clannie->rank_symbol );
        clannie->rank_symbol = str_dup( get_rank( victim ), clannie->rank_symbol );
        clannie->level   = victim->level;
        clannie->alignment= victim->alignment;
        clannie->donated = victim->pcdata->donated_dia;
        clannie->rank    = victim->pcdata->clan_rank;
        clannie->sex     = victim->sex;
        add_clannie( clan, clannie );
        copy_roster_clannie( victim );
        save_clan( clan );
        save_char_obj( victim, FALSE );
    }

    if (choffline) { do_function(victim, &do_quit,"imm-overide"); }
}

void unguild_clanne( CHAR_DATA *ch, CLAN_DATA *clan )
{
  
    remove_clannie( ch );

    ch->pcdata->clan_rank =0;
    ch->pcdata->donated_dia =0;
    ch->pcdata->clanowe_dia = 0;
    ch->pcdata->clanowe_level = 0;
    ch->pcdata->clanowe_clan = NULL;

    save_clan( clan );
    save_char_obj( ch, FALSE );  
    
}
    


void do_guild( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char log_buf[MAX_STRING_LENGTH];
  CHAR_DATA   *victim;
  CLAN_DATA   *clan     = NULL;
  RANK_DATA   *rank     = NULL;
  ROSTER_DATA *clannie  = NULL;
  CLAN_DATA   *prevclan = NULL;
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  
  if ( arg1[0] == '\0' )
  {
    send_to_char( "Syntax: guild <char> <cln name>\n\r",ch);
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
  {
    send_to_char( "They aren't playing.\n\r", ch );
    return;
  }

  if ( IS_NPC( victim ) )
  {
    send_to_char( "Cannot guild a NPC\n\r", ch );
    return;
  }
    
  if ( IS_NPC( ch ) )
  {
    send_to_char( "What, you are an NPC geez..\n\r", ch );
    return;
  } 

  if ( ( victim->level < LEVEL_CLAN ) && ( !IS_IMMORTAL( ch ) ) )
  {
    send_to_char( "Your target is too young to be guilded\n\r", ch );
    return;
  }

  if ( !IS_IMMORTAL( ch ) )
  {
    if ( !( rank = find_clannie_rank( ch ) ) )
    {
      send_to_char( "Your rank was not found.\n\r", ch );
      return;
    }

    if ( ( !IS_SET( rank->rank_flags, RANK_LEADER )
    &&     !IS_SET( rank->rank_flags, RANK_ASSISTANT )
    &&     !IS_SET( rank->rank_flags, RANK_AMBASSADOR ) ) )
    {
      send_to_char("Only a Leader, Assistant, or Ambassador may guild new members.\n\r", ch );
      return;
    }
  }
        
  if ( IS_IMMORTAL( victim ) && ( victim->level > ch->level ) )
  {
    send_to_char( "You cannot guild/unguild that immortal\n\r", ch );
    return;
  }

  if ( !str_cmp( arg2, "none" ) && ( IS_IMMORTAL( ch ) ) )
  {
    if ( !victim->clan )
    {
      send_to_char( "They aren't in a clan!\n\r", ch );
      return;
    }

    prevclan = victim->clan;
    remove_clannie( victim );
    send_to_char( "They are now clanless.\n\r", ch );
    send_to_char( "You are now a member of no clan!\n\r", victim );
      
    if ( IS_SET( victim->act, PLR_LEADER ) )
      REMOVE_BIT( victim->act, PLR_LEADER );

    if ( victim->pet )
      victim->pet->clan = NULL;

    victim->pcdata->clan_rank =0;
    victim->pcdata->donated_dia =0;

    if ( !IS_IMMORTAL( victim ) )
    {
      mprintf( sizeof( buf ), buf, "%s has been removed from clan status.",  victim->name );
      log_string( buf );
    }

    victim->pcdata->clanowe_dia = 0;
    victim->pcdata->clanowe_level = 0;
    victim->pcdata->clanowe_clan = NULL;
    save_clan( prevclan );
    save_char_obj( victim, FALSE );
    return;
  }

  if ( ( victim->clan )
  &&   ( !IS_IMMORTAL( ch ) ) )
  {
    send_to_char( "Cannot Guild this character. Only an IMM can\n\r", ch );
    return;
  }

  if ( IS_NULLSTR(arg2) )
  {
    if ( ch->clan )
      clan = ch->clan;
    else
    {
      send_to_char( "Syntax: guild <player> <clan>\n\r", ch );
      return;
    }
  }

  if ( !IS_NULLSTR( arg2)
  &&   !( clan = get_clan( arg2, TRUE ) ) )
  {
    send_to_char( "No such clan exists.\n\r", ch );
    return;
  }

  if ( !clan )//this shouldn't happen...
  {
    send_to_char( "There is some problem, it is being logged.\n\r", ch );
    bugf( "do_guild: clan variable is NULL and shouldn't be (clan.c) ch: %s - victim: %s",
      ch->name, victim->name );
    return;
  }

  if ( ( ch->clan != clan ) && ( !IS_IMMORTAL( ch ) ) )
  {
    send_to_char( "Cannot guild someone to another guild\n\r", ch );
    return;
  }


  for ( clannie = clan->roster ; clannie ; clannie = clannie->next )
  {
    if ( !str_cmp( clannie->name, victim->name ) )
    {
      send_to_char( "They are already in that clan.\n\r", ch );
      return;
    }
  }

  printf_to_char( ch, "They are now a member of clan %s.\n\r", clan->symbol );
  printf_to_char( victim, "You are now a member of clan %s.\n\r", clan->symbol );

  if ( !IS_IMMORTAL( victim ) )
  {
    mprintf( sizeof( buf ), buf, "%s is now a member of %s", victim->name, clan->symbol );
    do_function( ch, &do_info, buf );

    mprintf( sizeof( log_buf ), log_buf, "%s has been guilded into %s", victim->name, clan->symbol );
    logit( log_buf );
  }

  victim->pcdata->clanowe_dia = 0;
  victim->pcdata->clanowe_level = 0;
  victim->pcdata->clanowe_clan = 0;
  victim->pcdata->numclans += 1;

  remove_clannie(victim);
  victim->clan      = clan;
  victim->clan_name = str_dup( clan->name, victim->clan_name );

  if ( victim->pet )
    victim->pet->clan = clan;


  victim->pcdata->clan_rank = 1;

  /* Start Imms off at lovest IMM rank, if it exists */
  if ( IS_IMMORTAL( victim ) ) 
  {
    for ( rank = clan->rank ; rank ; rank = rank->next )
    {
      if ( IS_SET(rank->rank_flags, RANK_IMMORTAL )
      &&  ( (victim->pcdata->clan_rank > rank->level)
        ||  (victim->pcdata->clan_rank == 1) ) )
        victim->pcdata->clan_rank = rank->level;
    }
  }

  victim->pcdata->donated_dia = 0;

  clannie = new_clannie();
 
  free_string( clannie->name );
  clannie->name    = str_dup( victim->name, clannie->name );
  free_string( clannie->title );
  clannie->title   = str_dup( victim->pcdata->title, clannie->title );
  free_string( clannie->rank_symbol );
  clannie->rank_symbol = str_dup( get_rank( victim ), clannie->rank_symbol );
  clannie->level   = victim->level;
  clannie->alignment= victim->alignment;
  clannie->donated = victim->pcdata->donated_dia;
  clannie->rank    = victim->pcdata->clan_rank;
  clannie->sex     = victim->sex;
  clannie->logon   = victim->logon;
  clannie->kills   = 0;
  clannie->pkills  = 0;
  clannie->pdeaths = 0;
  clannie->deaths  = 0;

  add_clannie( clan, clannie );

  save_char_obj( victim, FALSE );
  save_clan( clan );
}


void do_diamonds( CHAR_DATA *ch, char *argument)
{
  int i = 0;
  CLAN_DATA *clan;
  if (ch->clan == 0 ) {
    send_to_char ("Huh?\n\r",ch);
    return;
  }
  send_to_char("{CDiamond listing for the clans{x\n\r",ch);
  send_to_char("{r--------------------------------------------{x\n\r",ch);
  send_to_char("{WTOTAL  USED   BANK         CLAN             {x\n\r",ch);
  send_to_char("{r--------------------------------------------{x\n\r",ch);
  for ( clan = clan_free; clan; clan = clan->next )
    {
      if (clan_table[i].independent)
    continue;
      printf_to_char(ch,"{W%-6d %-6d %-6d %s{x\n\r",
             clan_table[i].total_dia,clan_table[i].used_dia,
             clan_table[i].free_dia, clan->symbol);
    }

}

void do_short_diamonds( CHAR_DATA *ch, char *argument)
{
  if (ch->clan == NULL ) {
    send_to_char ("Huh?\n\r",ch);
    return;
  }
  printf_to_char(ch,"{cClan {x%s{x\n\r",ch->clan->name);
  printf_to_char(ch,"{cTOTAL:{W%-6d{cUSED:{W%-6d{cBANK:{W%-6d{x\n\r",
         ch->clan->donation_total, ch->clan->donation_spent,
         ch->clan->donation_balance);

}

void do_roster(CHAR_DATA *ch, char *argument)
{
  int  z=0;

  CLAN_DATA   *clan    = NULL;
  RANK_DATA   *rank    = NULL;
  ROSTER_DATA *clannie = NULL;

  char buf[MSL];
  char rstyle[MSL];
  char *rspt;
  BUFFER *buffer;
//  int cost =0;
//  int actclan =0;
  bool loggedin = FALSE;
  char arg[MIL];
  CHAR_DATA *vch;
  bool lord = FALSE;
  bool leader = FALSE;
  bool immortal = FALSE;
  bool ambassador = FALSE;
  bool suppress_l = FALSE;
  int  day_counter = 0;
  int  fieldlen = 0;
  argument = one_argument( argument, arg );

  if ( ( ch->clan == NULL && !IS_IMMORTAL( ch ) )
  ||   IS_NPC( ch ) ) 
  {
    send_to_char( "Huh?\n\r", ch );
    return;
  }

  if ( IS_IMMORTAL( ch )
  &&   !ch->clan
  &&   IS_NULLSTR( arg ) )
  {
    send_to_char( "You aren't in a clan... how can you check its roster.\n\r", ch );
    return;
  }

  if ( IS_NULLSTR( arg )
  ||   !IS_IMMORTAL( ch ) )
    clan = ch->clan;
  else if ( !(clan = get_clan( arg, TRUE ) ) )
  {
    send_to_char( "No such clan exists, try another name.\n\r", ch );
    return;
  }

  if ( !clan->rank )
  {
    send_to_char( "The clan's ranks have not been set yet.\n\r", ch );
    return;
  }

  if ( !IS_IMMORTAL( ch ) )
  {
    if ( !( rank = find_clannie_rank( ch ) ) )
    {
        send_to_char( "Unable to find your rank status.\n\r", ch );
        return;
    }
  }

  if ( !IS_IMMORTAL( ch )
  &&   clan != ch->clan )
  {
    send_to_char( "You may not view the roster of that clan.\n\r", ch );
    return;
  }

  if ( IS_IMMORTAL( ch ) )
    immortal = TRUE;

  if ( !IS_IMMORTAL( ch ) )
    if ( IS_SET( rank->rank_flags, RANK_ASSISTANT ) )
        lord = TRUE;

  if ( !IS_IMMORTAL( ch ) )
    if ( IS_SET( rank->rank_flags, RANK_LEADER  ) )
        leader = TRUE;

  if ( !IS_IMMORTAL( ch ) )
    if ( IS_SET( rank->rank_flags, RANK_AMBASSADOR ) )
      ambassador = TRUE;

  if ( IS_SET( clan->clan_flags, CLAN_INDEPENDENT )
  &&   !IS_IMMORTAL( ch ) )
  {
    printf_to_char(ch, 
        "There is no roster for %s{x, it is an independent organization.\n\r",
            clan->name);
    return;
  }

  strcpy(rstyle,clan->roster_style);
// if user has customizable style, replace rstyle here

  buffer = new_buf();
  mprintf( sizeof( buf ), buf, " %s\n\r\n\r{x", clan->symbol );
  add_buf( buffer, buf );

  int i = 0;
  
  for ( z = clan->max_rank; z >= 0; z-- )
  {
    for ( clannie = clan->roster; clannie; clannie = clannie->next ) 
    {
      if ( z == clannie->rank )
      {
        i++;
        rspt = rstyle;

        while( *rspt != '\0' )
        {
          if ( *rspt == '%' )
          {
            ++rspt;

            fieldlen = 0;
            mprintf(sizeof(buf), buf,"%c",*rspt);
            if (is_number(buf))
            {
              fieldlen = atoi(buf);
              ++rspt;
            }

            switch( *rspt )
            {
              default :
                mprintf(sizeof(buf), buf," "); break;
              case 'o' :
                loggedin = FALSE;
                for ( vch = player_list; vch; vch = vch->next_player )
                  if ( !strcmp( vch->name, clannie->name ) && vch->clan 
                  && clan->name == vch->clan->name && can_see( ch, vch ) )
                    loggedin = TRUE;
                if (loggedin)
                  mprintf(sizeof(buf), buf,"*");
                else
                  mprintf(sizeof(buf), buf," ");
                break;
              case 'l' :
                mprintf(sizeof(buf), buf,"%3d",clannie->level); break;
              case 'r' :
                mprintf(sizeof(buf), buf,"%s",clannie->race); break;
              case 'c' :
                mprintf(sizeof(buf), buf,"%s",clannie->pc_class); break;
              case 'n' :
                mprintf(sizeof(buf), buf,"%s",clannie->name); break;
              case 't' :
                mprintf(sizeof(buf), buf,"%s",clannie->title); break;
              case 's' :
                mprintf(sizeof(buf), buf,"%s",clannie->rank_symbol); break;
              case 'a' :
                mprintf(sizeof(buf), buf,"%5d",clannie->alignment); break;
              case 'A' :
                if (clannie->alignment > 500)
                  mprintf(sizeof(buf), buf,"{W");
                else if (clannie->alignment < -500)
                  mprintf(sizeof(buf), buf,"{D");
                else
                  mprintf(sizeof(buf), buf,"{w");
                break;
              case 'p' :
                mprintf(sizeof(buf), buf,"%*d",fieldlen,clannie->pkills); break;
              case 'k' :
                mprintf(sizeof(buf), buf,"%*d",fieldlen,clannie->kills); break;
              case 'x' :
                mprintf(sizeof(buf), buf,"%s",clannie->sex == SEX_FEMALE ? "F" : "M"); break;
              case 'd' :
                day_counter = ( int )( current_time - clannie->logon )/60/60/24;
                if ( day_counter > 1 )
                {
                  if ( ( clannie->level >= LEVEL_IMMORTAL )
                  && ( !immortal ) )
                    mprintf(sizeof(buf), buf,"%*s",fieldlen,"");
                  else
                    mprintf(sizeof(buf), buf,"%*d",fieldlen,day_counter);
                }
                else
                  mprintf(sizeof(buf), buf,"%*s",fieldlen,"");
                break;
              case 'g' : // gems donated
                if ( clannie->level < LEVEL_IMMORTAL )
                  mprintf(sizeof(buf), buf,"%*d",fieldlen,clannie->donated);
                else
                  mprintf(sizeof(buf), buf,"%*d",fieldlen,0);
                break;
            }
          } // char was '%'
          else if ( ( *rspt == '?' ) && ( *(rspt+1) == 'l' ) )
          {
            mprintf(sizeof(buf), buf,"");
            if ( immortal || leader || lord || ambassador )
              ++rspt;
            else
            {
              suppress_l = TRUE;
              ++rspt;
            }
          }
          else if ( ( *rspt == '!' ) && ( *(rspt+1) == 'l' ) )
          {
           mprintf(sizeof(buf), buf,"");
           if ( immortal || leader || lord || ambassador )
              ++rspt;
            else
            {
              suppress_l = FALSE;
              ++rspt;
            }
          }
          else
            mprintf(sizeof(buf), buf,"%c",*rspt);

          if (suppress_l == FALSE) add_buf( buffer, buf );

          ++rspt;
        } // while
        mprintf( sizeof( buf ), buf, "\n\r" );
        add_buf( buffer, buf );
      } // if clannie is this rank
    } // clannie "for" loop
  } // rank "for" loop

  mprintf( sizeof( buf ), buf,
    "\n\rClan Members Found: %d\n\r", i );
  add_buf( buffer, buf );


/*
  actclan = 0; // num_act_clan(clan);

  if ( actclan > NUM_CLANNIES ) {
    cost = (25 * (actclan-NUM_CLANNIES))  - 20;
      if (clan == clan_lookup("goddess"))
    cost = 0;
  }
  else
    cost = 0;
  mprintf(sizeof(buf),buf,"{WTotal clan members = {C%d{x{W Mortals Only = {R%d{C({R%d{Wdia{C){x\n\r",clan_roster[clan].actual_num_clannies,actclan,cost);
  add_buf(buffer,buf);
*/
  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );
}


void save_clan_list(char *clan_roster_filename)
{
  FILE *fp;
  int x=0;
  int i=0;

  if ((fp = fopen(clan_roster_filename, "w"))!= NULL) {
    nFilesOpen++;
    fprintf(fp,"# Actual Number clans\n");
    fprintf(fp,"%d\n",actual_num_clans);
    for (x=1; x <= actual_num_clans; x++) {
      fprintf(fp,"# Name of  \n");
      fprintf(fp,"%s\n",clan_table[x].name);
      fprintf(fp,"# Actual number of clannies\n");
      fprintf(fp,"%d\n",clan_roster[x].actual_num_clannies);
      for (i=0; i < clan_roster[x].actual_num_clannies; i++) {
    /*    printf("CLAN %d :Saving %s at level %d\n",x,clan_roster[x].clannie[i].name, clan_roster[x].clannie[i].level);*/
    fprintf(fp,"# Name of clannie\n");
    fprintf(fp,"%s\n",clan_roster[x].clannie[i].name);
    fprintf(fp,"# Level of Clannie\n");
    fprintf(fp,"%d\n",clan_roster[x].clannie[i].level);
    fprintf(fp,"# diamonds donated\n");
    fprintf(fp,"%d\n",clan_roster[x].clannie[i].diamonds);
    fprintf(fp,"# Last Logon\n");
    fprintf(fp,"%d\n",(int)clan_roster[x].clannie[i].logon);
    fprintf(fp,"# Rank of clannie\n");
    fprintf(fp,"%d\n",clan_roster[x].clannie[i].rank);
    fprintf(fp,"# Rank of clannie\n");
    fprintf(fp,"%d\n",clan_roster[x].clannie[i].sex);
    fprintf(fp,"# PKs of clannie\n");
    fprintf(fp,"%d\n",clan_roster[x].clannie[i].kills);
    fprintf(fp,"# Deaths of clannie\n");
    fprintf(fp,"%d\n",clan_roster[x].clannie[i].killed);
      }
    }
    fclose(fp);
    nFilesOpen--;
  }
  /* and level needs to be saved */
}
void load_clan_list(char *clan_roster_filename)
{
  FILE *fp;
  char buf[512];
  int x=0;
  int i=0;

  /*  printf("Loading Clan Listing\n\r");*/
  if ((fp = fopen(clan_roster_filename, "r"))!= NULL) {
    nFilesOpen++;
    /*    printf("OPENED FILENAME %s\n\r",clan_roster_filename);*/
    get_next_line(fp,buf);/* Number of clans we ignore */
    for (x=1; x <= actual_num_clans; x++)
    {
      get_next_line(fp,buf); /* Name of clan we ignore*/
      get_next_line(fp,buf); /*Actual Number clannies */
      clan_roster[x].actual_num_clannies = atoi(buf);

      for (i=0; i < clan_roster[x].actual_num_clannies; i++)
      {
        get_next_line(fp,buf);/*Name of clannie*/
        strcpy(clan_roster[x].clannie[i].name,buf);
        get_next_line(fp,buf);/*Level of Clannie*/
        clan_roster[x].clannie[i].level= atoi(buf);
        get_next_line(fp,buf);/*diamonds donated*/
        clan_roster[x].clannie[i].diamonds= atoi(buf);
        get_next_line(fp,buf);/*Last Logon*/
        clan_roster[x].clannie[i].logon = atoi(buf);
        get_next_line(fp,buf);/*Rank of clannie*/
        clan_roster[x].clannie[i].rank = atoi(buf);
        get_next_line(fp,buf);/*Rank of clannie*/
        clan_roster[x].clannie[i].sex = atoi(buf);
        get_next_line(fp,buf);/*Pk's of clannie*/
        clan_roster[x].clannie[i].kills = atoi(buf);
        get_next_line(fp,buf);/*Deaths of clannie*/
        clan_roster[x].clannie[i].killed = atoi(buf);
      }
    }

    fclose(fp);
    nFilesOpen--;
  }
}

/* Used to reset the clan roster table. */
void do_reset_roster(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  int i=0,j=0;
  struct dirent *dp;
  DESCRIPTOR_DATA d;
  char arg[MAX_STRING_LENGTH];
  bool load_char = FALSE;
  DIR *dirptr;
  int return_int, return_int2;
  int arrcnum_files=0;

  if ((dirptr = opendir(PLAYER_DIR)) == NULL){
    bug("Player dir unable to be opened",0);
    return;
  } 
  for (i=1;i <= actual_num_clans; i++)
    clan_roster[i].actual_num_clannies = 0;

  while((dp=readdir(dirptr)) != NULL){
    return_int = memcmp(&dp->d_name[0],".",1);
    return_int2 = memcmp(&dp->d_name[0],"..",2);
    if ((return_int != 0 ) && (return_int2 != 0 )) {
      /*Get Start and Stop Time for DERG File here!!  */
      strcpy(arg,dp->d_name);
      arrcnum_files++;
    }
    if (arg[0] == '\0')
      continue;

    if(((victim = get_char_world(ch, arg)) != NULL) && (!IS_NPC(victim))) {
      load_char = FALSE;
    } else {
      d.original = NULL;
      if (!load_char_obj(&d, capitalize(arg))) /* char pfile exists? */
    {
      printf_to_char(ch, "{RNo such player: {W%s{x.\n\r", capitalize(arg));
      free_char(d.character);
      continue;
    }
      d.character->desc = NULL; /* safe than sorry */
      victim = d.character;
      load_char = TRUE;
    }
    
    if (strcmp(victim->name,"")) {
      if (victim->clan) {
    /*    for (i=0;i <num_clannies;i++)
          printf("CLAN PLAYER %s\n",clan_player[i]);*/
    j = 0; //victim->clan;
    i = 0; // clan_roster[victim->clan].actual_num_clannies;
    /*      printf("Reseting for clan player %s\n",clan_player[i]);*/
    clan_roster[j].clannie[i].diamonds = victim->pcdata->donated_dia;
    clan_roster[j].clannie[i].rank = victim->pcdata->clan_rank;
    clan_roster[j].clannie[i].sex = victim->sex;
    clan_roster[j].clannie[i].logon = victim->logon;
    clan_roster[j].clannie[i].level = victim->level;
    clan_roster[j].clannie[i].kills = victim->pcdata->kills;
    clan_roster[j].clannie[i].killed = victim->pcdata->deaths;
    strcpy(clan_roster[j].clannie[i].name,victim->name);
    /*      printf("loaded %s in clan %d as level %d\n\r",
        clan_roster[j].clannie[i].name, j, clan_roster[j].clannie[i].level);*/
    // LOOK clan_roster[victim->clan].actual_num_clannies++;
      }
      if (load_char) {
    nuke_pets(d.character,FALSE);
    free_char(d.character);
      }
    }
  }
  closedir(dirptr);
  
  return;

}

ROSTER_DATA *find_clan_roster_member(CHAR_DATA *ch)
{
  ROSTER_DATA *clannie = NULL;

  for ( clannie = ch->clan->roster; clannie; clannie = clannie->next ) 
  {
    if ( !str_cmp( ch->name, clannie->name ) )
    return( clannie );
  }
/*  printf("Did not find any clannie!!!\n\r");*/
  return( NULL );  
}


void do_donate_clan( CHAR_DATA *ch, OBJ_DATA *obj, int frominv, CLAN_DATA *clan )
{
  if ( !clan )
  {
    bugf( "Error in do_donate_clan, clan is not valid" );
    return;
  }

  if ( ch->clan == clan ) //Make sure it's not a dept payment
    ch->pcdata->donated_dia += 1;

  copy_roster_clannie( ch );

  clan->donation_balance += 1;
  clan->donation_total += 1;

  if (frominv) { obj_from_char(obj); }
  else { obj_from_room(obj); }
  extract_obj(obj);

  save_char_obj(ch, FALSE);
  save_clan(clan);
  //do_worth( ch, "" );
  //do_short_diamonds(ch, "");
  return;
}

void copy_roster_clannie(CHAR_DATA *ch)
{
  ROSTER_DATA *clannie = NULL;
  
  if ( !ch->clan )
    return;

  if ( IS_NPC( ch ) )
    return;

  for ( clannie = ch->clan->roster; clannie; clannie = clannie->next )
  {
        if ( !str_cmp( clannie->name, ch->name ) )
        {
            free_string( clannie->name );
            clannie->name   = str_dup( ch->name, clannie->name );

            free_string( clannie->rank_symbol );
            clannie->rank_symbol = str_dup( get_rank( ch ), clannie->rank_symbol );

            free_string( clannie->title );
            clannie->title = str_dup( ch->pcdata->title, clannie->title );
    
            free_string( clannie->afk_title );
            clannie->afk_title = str_dup( ch->pcdata->title,
                clannie->afk_title );

            free_string( clannie->pc_class );
            clannie->pc_class = str_dup(  class_table[ch->gameclass].who_name,
                clannie->pc_class );

            free_string( clannie->race );
            clannie->race = str_dup( pc_race_table[ch->race].who_name,
                clannie->race );

            clannie->rank   = ch->pcdata->clan_rank;
            clannie->sex    = ch->sex;
            clannie->donated= ch->pcdata->donated_dia;
            clannie->level  = ch->level;
            clannie->alignment  = ch->alignment;
            clannie->logon  = current_time;
            clannie->pkills = ch->pcdata->pkills;
            clannie->pdeaths= ch->pcdata->pdeaths;
            clannie->kills  = ch->pcdata->kills;
            clannie->deaths = ch->pcdata->deaths;
        }
    }

  save_clan( ch->clan );
}

void remove_rank( CHAR_DATA *ch, char *argument, CLAN_DATA *clan )
{
    //CLAN_DATA   *clan;
    RANK_DATA   *rank;  // those assignments are only neccessary if
                        // there is a risk that they go uninitialized :)         
    //clan = ch->clan;

    if ( clan )
    {
        for ( rank = clan->rank; rank; rank = rank->next )
        {
            if ( rank->number == atoi( argument ) )
                break;
        }

        if ( rank == NULL )
        {
            bugf( "Rank not found in the rank listing for %s.", clan->name );
            return;
        }

        smash_rank( clan, rank );
        clan->actual_ranks--;
        save_clan( clan );
        return;
    }
}

void remove_clannie( CHAR_DATA *ch )
{
  CLAN_DATA     *clan;
  ROSTER_DATA   *clannie;

  clan = ch->clan;

  if ( clan )
  {
    for ( clannie = clan->roster; clannie; clannie = clannie->next )
        if ( !str_cmp( ch->name, clannie->name ) )
            break;

    if ( clannie == NULL )
    {
        bugf( "Clannie not found in roster listing %s", clan->name );
        ch->clan = NULL;
        free_string( ch->clan_name );
        return; 
    }

    smash_clannie( clan, clannie );
    clan->actual_members--;

    ch->clan = NULL;
    free_string( ch->clan_name );
    //printf_to_char( ch, "%s removed from clan %s.\n\r",
    //                ch->name, clan->name );
    save_char_obj( ch, FALSE );
    save_clan( clan );
    return;     
  }
}

void do_remove_clannie( CHAR_DATA *ch, char *argument )
{
//  int i=0;
  int clannie_num = -1;
  char buf[MSL], buf2[MSL], buf3[MSL];
  CHAR_DATA *victim;
  

  if ( !IS_IMMORTAL( ch ) ) 
  {
    send_to_char( "Sorry this is an immortal only function\n\r", ch );
    return;
  }
/* LOOK
  if (clan_table[ch->clan].independent) 
    {
      clan_roster[ch->clan].actual_num_clannies = 0;
      send_to_char("Next copyover, no clannies in Loner.\n\r",ch);
      return;
    }
*/
  /* Find clannie in roster */
/*  for (i =0; i < clan_roster[ch->clan].actual_num_clannies; i++) {
    if (!strcmp(capitalize(argument), clan_roster[ch->clan].clannie[i].name))
      clannie_num = i;
  }*/

  if (clannie_num == -1) {
    /*  printf("Did not find any clannie!!!\n\r");*/
    send_to_char("That clannie does not exist in your clan roster\n\r",ch);
    return;
  }
/*
  do_function(ch,&do_pload,clan_roster[ch->clan].clannie[clannie_num].name);
  mprintf(sizeof(buf),buf,"%s wanderer",clan_roster[ch->clan].clannie[clannie_num].name); */
  if ((victim = get_char_world(ch, argument)) == NULL)
  /*  smash_clannie(clannie_num, ch->clan)*/;
  else {
    if (victim->level+ 5 < LEVEL_HERO) 
/*      mprintf(sizeof(buf2),buf2,
          "{WHear Ye, Hear Ye, One {R%s{W has been "
          "removed from clan {x%s{x\n\r"
          "{WThe cost of reclanning is {C%d{W Diamonds and "
          "they must attain level {C%d{W.{x\n\r\n\r",
          clan_roster[ch->clan].clannie[clannie_num].name,
          get_clan_desc(ch->clan),
          victim->level * 10,
          victim->level + 5)*/;
    else if (victim->level == LEVEL_HERO)
/*      mprintf(sizeof(buf2),buf2,
          "{WHear Ye, Hear Ye, One {R%s{W has been "
          "removed from clan {x%s{x\n\r"
          "{WThe cost of reclanning is {C%d{W Diamonds and "
          "they must attain Sartan's permission to rejoin a clan.{x\n\r\n\r",
          clan_roster[ch->clan].clannie[clannie_num].name,
          get_clan_desc(ch->clan),
          victim->level * 10)*/;
    else
/*      mprintf(sizeof(buf2),buf2,
          "{WHear Ye, Hear Ye, One {R%s{W has been "
          "removed from clan {x%s{x\n\r"
          "{WThe cost of reclanning is {C%d{W Diamonds and "
          "they must attain level {C%d{W and\n\r"
          "attain permission from Sartan before they rejoin a clan.{x\n\r\n\r",
          clan_roster[ch->clan].clannie[clannie_num].name,
          ch->clan->symbol *//* get_clan_desc(ch->clan) *//*,
          victim->level * 10,
          100)*/;
/* LOOK      
    mprintf(sizeof(buf3),buf3, "Clan Removal of %s.",
        clan_roster[ch->clan].clannie[clannie_num].name); */
    send_to_char("Player is removed from your roster\n\r",ch);
    if (!IS_IMMORTAL(victim))
      note_line("{YTown Crier{x", NOTE_NOTE,
        "All",
        buf3,
        buf2);
  }
  do_function(ch,&do_guild,buf);
  do_function(ch,&do_punload,argument);
}

void clannie_roster_rename(CHAR_DATA *ch, char *arg)
{/* LOOK
  int ros_num;
  if ((ros_num = find_clan_roster_member(ch)) == -1) 
    return;
  strcpy(clan_roster[ch->clan].clannie[ros_num].name,capitalize(arg));
  save_clan_list(ROSTER_FILE);
  return; */
}

int smash_rank( CLAN_DATA *clan, RANK_DATA *rank )
{
    if ( clan->rank == rank )
        clan->rank = rank->next;
    else
    {
        RANK_DATA *rank_prev;
        
        for ( rank_prev = clan->rank ; rank_prev ; rank_prev = rank_prev->next )
        {
            if ( rank_prev->next == rank )
                break;
        }
    
        rank_prev->next = rank->next;
    }
    free_rank( rank );
    return 0;
}

int smash_clannie( CLAN_DATA *clan, ROSTER_DATA *clannie )
{
  if ( clan->roster == clannie )
    clan->roster = clannie->next;
  else
  {
    ROSTER_DATA *clannie_prev;
    for ( clannie_prev = clan->roster; clannie_prev; 
          clannie_prev = clannie_prev->next )

        if ( clannie_prev->next == clannie )
            break;

    clannie_prev->next = clannie->next;
  }
/*
  strcpy(clan_roster[clan].clannie[clannie].name,
     clan_roster[clan].clannie[clan_roster[clan].actual_num_clannies-1].name); 
  clan_roster[clan].clannie[clannie].level =
    clan_roster[clan].clannie[clan_roster[clan].actual_num_clannies-1].level; 
  clan_roster[clan].clannie[clannie].diamonds =
    clan_roster[clan].clannie[clan_roster[clan].actual_num_clannies-1].diamonds; 
  clan_roster[clan].clannie[clannie].rank =
    clan_roster[clan].clannie[clan_roster[clan].actual_num_clannies-1].rank; 
  clan_roster[clan].clannie[clannie].sex = 
    clan_roster[clan].clannie[clan_roster[clan].actual_num_clannies-1].sex; 
  clan_roster[clan].clannie[clannie].logon = 
    clan_roster[clan].clannie[clan_roster[clan].actual_num_clannies-1].logon; 
  clan_roster[clan].clannie[clannie].kills = 
    clan_roster[clan].clannie[clan_roster[clan].actual_num_clannies-1].kills; 
  clan_roster[clan].clannie[clannie].killed = 
    clan_roster[clan].clannie[clan_roster[clan].actual_num_clannies-1].killed; 
  clan_roster[clan].actual_num_clannies-=1;
*/
  free_clannie( clannie );
  return(0);
}

void do_promote( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
  int old_rank;
  CHAR_DATA *victim;
  RANK_DATA *rank;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( IS_NPC(ch) )
  {
      send_to_char( "NPC's can not promote someone.\n\r",ch);
      return;
  }

  if ( !IS_IMMORTAL( ch ) )
  {
    if ( !ch->clan )
    {
      send_to_char( "You are not in a clan!\n\r", ch );
      return;
    }

    if ( !( rank = find_clannie_rank( ch ) ) )
    {
        send_to_char( "Your place was not found in this clan.\n\r", ch );
        return;
    }

    if ( ( !IS_SET( rank->rank_flags, RANK_LEADER )
    &&     !IS_SET( rank->rank_flags, RANK_ASSISTANT ) ) )
    {
        send_to_char( "You must be a Leader or Assistant to promote someone.\n\r",ch);
        return;
    }
  }

  if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
  {
      send_to_char( "They must be present to be promoted.\n\r", ch );
      return;
  }

  if ( !victim->clan )
  {
    send_to_char( "They are not in a clan.\n\r", ch );
    return;
  }
       
  if (( arg1[0] == '\0' || arg2[0] == '\0' 
  ||    atoi(arg2) < 1 || atoi(arg2) > victim->clan->max_rank - 1 )
  && ( !IS_IMMORTAL( ch ) ) )
  {
      send_to_char( "Syntax: promote <char> <rank #>\n\r",ch);
      return;
  }

  if (( arg1[0] == '\0' || arg2[0] == '\0' 
  ||    atoi(arg2) < 1  || atoi(arg2) > victim->clan->max_rank )
  && (  IS_IMMORTAL( ch ) ) )
    {
      send_to_char( "Syntax: promote <char> <rank #>\n\r",ch);
      return;
    }

  if ( IS_NPC(victim) )
    {
      send_to_char("You must be mad.\n\r",ch);
      return;
    }
    
  if ( ( !IS_IMMORTAL( ch )
  &&     victim->clan->name != ch->clan->name ) )
  {
      send_to_char("You can not promote a player who is not in your clan.\n\r",ch);
      return;
  }
    
  if ( !IS_IMMORTAL(ch) && IS_SET( victim->act, PLR_LEADER ) )
  {
      send_to_char("You cannot promote a leader.\n\r",ch);
      return;
  }

  if ( !IS_IMMORTAL( ch )
  &&   victim->pcdata->clan_rank >= ch->pcdata->clan_rank )
  {
      send_to_char("You cannot demote someone with a greater or equal rank than you.\n\r", ch);
      return;
  }

  if ( !IS_IMMORTAL(ch) && ch->pcdata->clan_rank <= atoi(arg2))
  {
      send_to_char("You cannot promote someone to a greater or equal rank than you.\n\r", ch);
      return;
  }

  if ( !IS_IMMORTAL( ch )
  &&   atoi( arg2 ) < 2 )
  {
    send_to_char( "You may not promote below the second rank.\n\r", ch );
    return;
  }

  old_rank = victim->pcdata->clan_rank;
  victim->pcdata->clan_rank = atoi(arg2);
  save_char_obj(victim, FALSE);

  for ( rank = victim->clan->rank ; rank ; rank = rank->next )
     if ( IS_SET( rank->rank_flags, RANK_LEADER ) )
         if ( victim->pcdata->clan_rank == rank->level )
             SET_BIT( victim->act, PLR_LEADER );
 
  copy_roster_clannie( victim );

  if ( victim->pcdata->clan_rank > old_rank )
  {
    mprintf(sizeof(arg2), arg2, "{W%s has been promoted to %d!{x", victim->name, get_rank( victim ) );
    act(arg2, victim, NULL, NULL, TO_ROOM );

    // Now accounted-for by act(,,,,TO_ROOM)
    //printf_to_char( ch, "You have promoted %s to %s!\n\r", victim->name, get_rank( victim ) );
    printf_to_char( victim, "Congratulations, you have been promoted to %s!\n\r",
        get_rank( victim ) );
  }
  else
  {
    mprintf(sizeof(arg2), arg2, "{W%s has been demoted to %d!{x", victim->name, get_rank( victim ) );
    act(arg2, victim, NULL, NULL, TO_ROOM );

    // Now accounted-for by act(,,,,TO_ROOM)
    //printf_to_char( ch, "You have demoted %s to %s.\n\r", victim->name, get_rank( victim ) );
    printf_to_char( victim, "Ouch, you've been demoted to %s!\n\r",
        get_rank( victim ) );
  }

  return;
}    

/*
  void update_roster()
{
  int i=0,j=0;
  for (i=0; i < actual_num_clans; i++)
    for (j=0; j < clan[i].actual_num_clannies; j++)
      {
    clan_roster[i].clannie[i].logon; 
      }
}
*/
void do_clandeduct(CHAR_DATA *ch, char *argument)
{
  int deductamt=0;
  int clan=0;
  char buf[MSL];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument(argument, arg1);
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char("Syntax: clandeduct clan diamonds",ch);
      return;
    }
  if (!is_valid_clan_name(arg1)) {
    send_to_char("Sorry, that is invalid clan name.\n\r",ch);
    return;
  }
  if (!is_number(arg2)) {
    send_to_char("Sorry that is an invalid number to deduct.\n\r",ch);
    return;
  }
  if ((clan = clan_lookup(arg1)) == 0)
    {
      send_to_char("No such clan exists.\n\r",ch);
      return;
    }

  deductamt = atoi(arg2);
  if (deductamt > clan_table[clan].free_dia) {
    send_to_char("Not enough diamonds in clan coffers.\n\r",ch);
    return;
  }
  clan_table[clan].used_dia += deductamt;
  clan_table[clan].free_dia -= deductamt;
  printf_to_char(ch,"Clan %s has been deducted {C%s{x diamonds.\n\rTotal is now {C%d{x.\n\r",
         arg1, arg2, clan_table[clan].used_dia);
   //mprintf(sizeof(buf), buf,"{g%s{w restored room {R%d{w ({g%d{w character%s{w){x.",
   //     ch->name, ch->in_room->vnum, count, count <= 1 ? "" : "s" );
   //   wiznet(buf,NULL,NULL,WIZ_RESTORE,0,get_trust(ch));
  mprintf(sizeof(buf), buf,"%s has deducted {C%s{x from clan %s, making their balance {C%d{x",
                         ch->name ,arg2, arg1, clan_table[clan].used_dia);
      wiznet(buf,ch,NULL,WIZ_SECURE,0,0);
  return;
}

bool is_valid_clan_name(char *str)
{
  CLAN_DATA *clan;

  if ( ( clan = get_clan( str, TRUE ) ) )
    return TRUE;

  if ( !strcmp( str, "none" ) )
    return TRUE;

  return FALSE;
  /*int i=0;
  for (i=1; i <= actual_num_clans; i++)
    if (!strcmp(clan_table[i].name,str))
      return(TRUE);
  if (!strcmp(str,"none"))
    return(TRUE);
  return(FALSE);*/
}

int num_act_clan(int clan)
{
  int i=0;
  int count=0;

  if (clan == 0 ) {
    return 0;
  }
  for (i=0; i < clan_roster[clan].actual_num_clannies; i++) 
      if (clan_roster[clan].clannie[i].level <= LEVEL_HERO) 
    count++;
  return(count);
      
}

void do_clean_roster(CHAR_DATA *ch, char *argument)
{
/* LOOK
  int i=0,j=0;
  int clan;

  clan = ch->clan;
3
  if (clan == 0 )
  {
    return;
  }
  for (i=0; i < clan_roster[clan].actual_num_clannies; i++) 
    for (j=0; j < clan_roster[clan].actual_num_clannies; j++)
      if (i !=j)
      {
        if (!strcmp(clan_roster[clan].clannie[i].name, clan_roster[clan].clannie[j].name))
        {
            if (i < j)
            {
                printf_to_char(ch,"Clannie %s has been removed.\n\r",clan_roster[clan].clannie[j].name);
                smash_clannie(j, clan);
                return;
            }
            else
            {
                printf_to_char(ch,"Clannie %s has been removed.\n\r",clan_roster[clan].clannie[i].name);
                smash_clannie(i, clan);
                return;
            }
        }
     } */
}
char *get_clan_desc_char(CHAR_DATA *ch)
{
  return ch->clan->symbol; // (get_clan_desc(ch->clan));
}

char *get_clan_desc( CLAN_DATA *clan )
{
  if (clan == NULL) return("         ");
/*  if (clan == 4) 
        return(chaos_names[number_range(0, MAX_CHAOS_NAMES-1)].who_name); */
  return( clan->symbol );
}

char *get_clan_rank_char( CHAR_DATA *ch )
{
  // LOOK return(get_clan_rank(ch->clan, ch->pcdata->clan_rank, ch->sex));
  return "";
}



char *get_clan_rank_roster(int clan, int i)
{
  if (clan==0) return("");
  if (clan==2) return(order_clan_rank_table[clan_roster[clan].clannie[i].rank].title_of_rank[clan_roster[clan].clannie[i].sex]);
  if (clan==5) return(light_clan_rank_table[clan_roster[clan].clannie[i].rank].title_of_rank[clan_roster[clan].clannie[i].sex]);
  if (clan==6) return("");
  if (clan==4) return(velg_clan_rank_table[clan_roster[clan].clannie[i].rank].title_of_rank[clan_roster[clan].clannie[i].sex]);
  if (clan==7) return(xanadu_clan_rank_table[clan_roster[clan].clannie[i].rank].title_of_rank[clan_roster[clan].clannie[i].sex]);
  if (clan==8) return(cov_clan_rank_table[clan_roster[clan].clannie[i].rank].title_of_rank[clan_roster[clan].clannie[i].sex]);
  return( clan_rank_table[clan_roster[clan].clannie[i].rank].title_of_rank[clan_roster[clan].clannie[i].sex] );
}

char *get_clan_rank(int clan, int clan_rank, int sex )
{
  if (clan == 0) return("");
  if (clan == 2) return(order_clan_rank_table[clan_rank].title_of_rank[sex]);
  if (clan == 5) return(light_clan_rank_table[clan_rank].title_of_rank[sex]);
  if (clan == 6) return("");
  if (clan == 4) return(velg_clan_rank_table[clan_rank].title_of_rank[sex]);
  if (clan == 7) return(xanadu_clan_rank_table[clan_rank].title_of_rank[sex]);
  if (clan == 8) return(cov_clan_rank_table[clan_rank].title_of_rank[sex]);
  return(clan_rank_table[clan_rank].title_of_rank[sex]);
}
/*********
  New Clan WAR code
********/

void load_clan_status(char *filename)
{
  FILE *fp;
  int i,j;
  char buf[MSL];
  fp = fopen(filename,"r");
  if (fp == NULL) {
    bugf("ACK, Failure to read CLAN STATUS");
    return;
  }
  nFilesOpen++;
  for (i=0;i < MAX_CLAN;i++)
    for(j=0; j < MAX_CLAN; j++)
      {
    get_next_line(fp,buf);
    clan_status[i][j] = atoi(buf);
      }
  printf("Finished loading Clan Status.\n\r");
  fclose (fp);
  nFilesOpen--;
}

void save_clan_status(char *filename)
{
  FILE *fp;
  int i,j;
  fp = fopen(filename,"w");
  if (fp == NULL) {
    bugf("ACK, Failure to WRITE CLAN STATUS");
    return;
  }
  nFilesOpen++;
  for (i=0;i < MAX_CLAN;i++)
    for(j=0; j < MAX_CLAN; j++)
      {
    fprintf(fp,"%d\n",clan_status[i][j]);
      }
  printf("Finished saving Clan Status.\n\r");
  fclose (fp);
  nFilesOpen--;
}

void do_clanstatus(CHAR_DATA *ch, char *argument)
{
  CLAN_DATA *clan, *clan2;

  char arg1[MIL];
  char arg2[MIL];
  char arg3[MIL];
  int status;
  
  argument = one_argument(argument,arg1);

  if (arg1[0] == '\0')
    {
      send_to_char("Syntax: clanstatus show.\n\r",ch);
      send_to_char("                   save.\n\r",ch);
      send_to_char("                   name.\n\r",ch);
      send_to_char("                   name name2.\n\r",ch);
      send_to_char("                   name name2 NEWSTATUS.\n\r",ch);
      send_to_char("                                       .\n\r",ch);
      send_to_char("       NEWSTATUS: = war,ally,friend,none,hate.\n\r",ch);
      return;
    }
  if (!strcmp(arg1,"save")) {
    save_clan_status(CLAN_STATUS_FILE);
    send_to_char("Clan status information saved.\n\r",ch);
    return;
  } 
 if (!strcmp(arg1,"show")) {
   send_to_char("sorry.. disabled at the moment.\n\r",ch);
   return;
  }
    
  if (is_valid_clan_name(arg1)) {
    argument = one_argument(argument,arg2);
    if (is_valid_clan_name(arg2)) {
      argument = one_argument(argument,arg3);
      if (is_valid_clan_status(arg3)) {
    clan  = get_clan( arg1, FALSE ); // clan_lookup(arg1);
    clan2 = get_clan( arg2, FALSE );
    status = clan_status_lookup( arg3 );
    set_clan_status( clan, clan2, status );
    return;
      } else {
    clan  = get_clan( arg1, FALSE ); // clan_lookup(arg1);
    clan2 = get_clan( arg2, FALSE );
    show_clan_status(ch,clan, clan2);
    return;
      }
    } else {
      clan = get_clan( arg1, FALSE ); // clan_lookup(arg1);
      show_clan_status( ch, clan, NULL );
      return;
    }
  }
  /* syntax talk */
  do_function(ch,&do_clanstatus,"");
}

bool is_valid_clan_status(char *str)
{
  if (!strcmp("war",str))
    return TRUE;
  if (!strcmp("ally",str))
    return TRUE;
  if (!strcmp("friend",str))
    return TRUE;
  if (!strcmp("none",str))
    return TRUE;
  if (!strcmp("hate",str))
    return(TRUE);
  return(FALSE);
}

int clan_status_lookup(char *str)
{
  if (!strcmp("war",str))
    return 0;
  if (!strcmp("ally",str))
    return 8;
  if (!strcmp("friend",str))
    return 4;
  if (!strcmp("none",str))
    return 2;
  if (!strcmp("hate",str))
    return 1;
  return(FALSE);
}

void show_clan_status(CHAR_DATA *ch, CLAN_DATA *clan1, CLAN_DATA *clan2)
{
  printf_to_char(ch,"{WSTATUS for clan %s{x\n\r", clan1->symbol );
  if ( clan2 == NULL )
    show_single_clan_status( ch, clan1 );
  else
    show_line_clan_status( ch, clan1, clan2 );
}

void show_single_clan_status(CHAR_DATA *ch, CLAN_DATA *clan)
{
  CLAN_DATA *clan1;
  for ( clan1 = clan_free; clan1; clan1 = clan1->next )
    show_line_clan_status( ch, clan, clan1 );
}
void show_line_clan_status( CHAR_DATA *ch, CLAN_DATA *clan1, CLAN_DATA *clan2 )
{
  printf_to_char( ch,"%s %s %s.\n\r",clan1->symbol,
                  get_clan_status(clan1,clan2), clan2->symbol );
}

char *get_clan_status(CLAN_DATA *clan1, CLAN_DATA *clan2)
{
  switch (-1)/* (clan_status[clan1][clan2])*/ {
  default:
    return("ERROR");
  case 0:
    return("is at war with");
  case 1:
    return("hates");
  case 2:
    return("cares neither way for");
  case 4:
    return("is friends with");
  case 8:
    return("is allies with");
  }
}

void set_clan_status(CLAN_DATA *clan1, CLAN_DATA *clan2, int status)
{
/*  clan_status[clan1][clan2] = status;
  save_clan_status(CLAN_STATUS_FILE);
*/
}

void do_clanmode(CHAR_DATA *ch, char *argument)
{
  if (ch->clan)
    show_clan_status( ch, ch->clan, NULL );
  else
    send_to_char("You are not in a clan.\n\r",ch);
}


void do_donate_clan_all( CHAR_DATA *ch, char *arg)
{
  CLAN_DATA *clan = NULL;
  OBJ_DATA *obj, *obj_next;
  char buf[MIL];
  int found = FALSE;
  int count =0;
  /* Asssume these checks are already done :) */   

  for ( obj = ch->carrying; obj; obj = obj_next )
  {

      obj_next = obj->next_content;
      if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
       && can_drop_obj( ch, obj )
       && obj->timer <= 0
       && obj->wear_loc == WEAR_NONE
       && obj->item_type != ITEM_MONEY
       && obj->item_type != ITEM_CORPSE_NPC
       && obj->item_type != ITEM_CORPSE_PC
       && IS_IN_CLAN( ch ) )
       {
            if ( ( clan = get_clan( ch->clan_name, FALSE ) ) )
            {
                if ( obj->pIndexData->vnum == clan->donation_gem )
                {
                    found = TRUE;
                    count++;
                    ch->pcdata->donated_dia += 1;
                    ch->clan->donation_balance += 1;
                    ch->clan->donation_total += 1;
                    obj_from_char( obj );
                    extract_obj( obj );
                }
            }   
       }
  }
  if ( found )
  {
    if ( ch->in_room != get_room_index( ROOM_VNUM_ALTAR ) )
      act( "{W$n {Cdonates a lot of diamonds.{x", ch, NULL, NULL, TO_ROOM );

    printf_to_char( ch, "{CYou donate {Y%s {Cdiamond%s.{x\n\r",
                    numcpy( buf, count), count == 1 ? "" : "s" );

    copy_roster_clannie( ch );
    save_clan( clan );

    do_short_diamonds( ch, "" );
    return;
  }
}

void do_clanowe(CHAR_DATA *ch, char *argument)
{
  char arg1[MIL], arg2[MIL], arg3[MIL], arg4[MIL];
  CHAR_DATA *victim;
  int clan;
    
  if (!IS_IMMORTAL(ch) && !IS_LEADER(ch)) {
    send_to_char("Go play elsewhere than this command.\n\r",ch);
    return;
  }
  argument = one_argument(argument,arg1);
  argument = one_argument(argument,arg2);
  argument = one_argument(argument,arg3);
  argument = one_argument(argument,arg4);

    
  if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      if (IS_IMMORTAL(ch)) {
    if (arg3[0] =='\0') {
      printf_to_char(ch,"Syntax: clanowe: <victim> <clan> <key> <value>\n\r");
      printf_to_char(ch,"       key: dia   - # of diamonds owed.\n\r");
      printf_to_char(ch,"       key: level - # of levels owed.\n\r");
      printf_to_char(ch,"       key: show  - shows what they owe.\n\r");
      printf_to_char(ch,"       key: done  - Sets them as completed.\n\r");
    }
      } else {
    printf_to_char(ch,"Syntax: clanowe: <victim> <key>\n\r");
    printf_to_char(ch,"       key: show  - shows what they owe.\n\r");
    printf_to_char(ch,"       key: done  - Sets them as completed.\n\r");
      }
      return;
    }

  if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
      send_to_char( "They aren't playing.\n\r", ch );
      return;
    }

  if (IS_NPC(victim)) {
    send_to_char("Not on NPC's\n\r",ch);
    return;
  }

  if (IS_IMMORTAL(ch)) {
    if (!is_valid_clan_name(arg2)) {
      send_to_char("Sorry, that is invalid clan name.\n\r",ch);
      return;
    }
    if ((clan = clan_lookup(arg2)) == 0)
      {
    send_to_char("No such clan exists.\n\r",ch);
    return;
      }

    if (!str_cmp(arg3,"dia")) {
      if (arg4[0] =='\0') {
    send_to_char("Value is required.\n\r",ch);
    return;
      }
     if ( is_number( arg4 ) ) 
     {
        victim->pcdata->clanowe_dia = atoi(arg4);
     } 
     else 
     {
        send_to_char("Value is not a number.\n\r",ch);
        return;
     }
    } else if (!str_cmp(arg3,"level")) {
      if (arg4[0] =='\0') {
    send_to_char("Value is required.\n\r",ch);
    return;
      }
      if (is_number(arg4)) {
    victim->pcdata->clanowe_level = atoi(arg4);
      } else {
    send_to_char("Value is not a number.\n\r",ch);
    return;
      }
    } else if (!str_cmp(arg3,"show")) {
      if (victim->pcdata->clanowe_clan) 
    printf_to_char(ch, "%s owes %d diamonds to %s and needs level %d.\n\r",
               victim->name,
               victim->pcdata->clanowe_dia,
               victim->pcdata->clanowe_clan,
               victim->pcdata->clanowe_level);
      else
    send_to_char("They owe nothing.\n\r",ch);
    } else if  (!str_cmp(arg3,"done")) {
      victim->pcdata->clanowe_clan = 0;
      victim->pcdata->clanowe_dia = 0;
      victim->pcdata->clanowe_level = 0;
    } else    {
      send_to_char("Dia or Level please.\n\r",ch);
      return;
    }
  } 
  else 
  {
    if ( str_cmp( ch->clan_name, victim->pcdata->clanowe_clan ) ) 
    {
      send_to_char( "You cannot do anything to that person.\n\r", ch );
      return;
    } 
    if ( !str_cmp( arg2, "show" ) ) 
    {
      if ( victim->pcdata->clanowe_clan ) 
        printf_to_char( ch, 
            "%s owes %d diamonds to %s and needs level %d.\n\r",
            victim->name,
            victim->pcdata->clanowe_dia,
            victim->pcdata->clanowe_clan,
            victim->pcdata->clanowe_level );
      else
        send_to_char( "They owe nothing.\n\r", ch );
    } 
    else if ( !str_cmp( arg2, "done" ) ) 
    {
      victim->pcdata->clanowe_clan = 0;
      victim->pcdata->clanowe_dia = 0;
      victim->pcdata->clanowe_level = 0;
    } 
    else 
    {
      send_to_char( "Syntax: Show/Done.\n\r", ch );
      return;
    }
      
  }
  save_char_obj(victim, FALSE);
}
void do_payoff(CHAR_DATA *ch, char *argument)
{
  CLAN_DATA *clan;
  int amount, count = 0;
  char clan_name[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];
  char note_subject[MAX_INPUT_LENGTH], note_body[MAX_INPUT_LENGTH];
  char buf[MIL];
  OBJ_DATA *obj, *obj_next;
  bool found = FALSE;

  if (IS_GHOST(ch))
    {
      send_to_char("You cannot payoff anything as you are still quite {RDEAD{x!\n\r", ch);
      return;
    }

  if (ch->pcdata->clanowe_dia == 0)
    {
      send_to_char("You don't owe any clan any diamonds.\n\r", ch);
      return;
    }

  argument = one_argument(argument, clan_name);
  clan = get_clan( clan_name, FALSE );

  if ( clan == NULL )
    {
      send_to_char("There is no such clan.\n\r", ch);
      return;
    }

  if ( str_cmp( clan->name, ch->pcdata->clanowe_clan ) )
    {
      send_to_char("You don't owe that clan any diamonds.\n\r", ch);
      return;
    }

  argument = one_argument(argument, arg);
  if (!is_number(arg))
    {
      send_to_char("Syntax: payoff <clan> <number of diamonds>\n\r", ch);
      return;
    }

  amount = atoi(arg);

  if (amount <= 0)
    {
      send_to_char("Very funny.\n\r", ch);                                    
      return;                                                                 
    }                                                                         
                                                                              
  if (amount > ch->pcdata->clanowe_dia)                                       
    {                                                                         
      printf_to_char(ch, "You only owe %s diamond%s.\n\r", 
            numcpy( buf, ch->pcdata->clanowe_dia ), 
            ch->pcdata->clanowe_dia == 1 ? "" : "s"  );
      amount = ch->pcdata->clanowe_dia;                                       
    }                                                                         
                                                                              
  for (obj = ch->carrying; obj; obj = obj_next)                               
    {                                                                           
      obj_next = obj->next_content;                                             
      if (    IS_DIAMOND(obj)                                                   
          && can_drop_obj(ch, obj)                                              
          && obj->wear_loc == WEAR_NONE)                                        
    {                                                                       
      found = TRUE;                                                             
      count++;                                                                  
      clan->donation_balance++;                                              
      clan->donation_total++;                                             
      obj_from_char(obj);
      extract_obj(obj);
    }                                                                       
      if (count == amount)                                                      
    break;                                                                  
    }                                                                           
  if ( found )                                                                  
  {                                                                         
    if (count < amount)                                                     
      printf_to_char( ch, "You only had %s diamonds available to donate.\n\r", 
                          numcpy( buf, count ) );
    printf_to_char( ch, "You have reimbursed %s %s diamond%s.\n\r",
                        clan->name, numcpy( buf, count ), 
                        count == 1 ? "" : "s" );
    ch->pcdata->clanowe_dia -= count;                                       
    if (ch->pcdata->clanowe_dia < 0)                                        
      ch->pcdata->clanowe_dia = 0;                                              
    if (ch->pcdata->clanowe_dia > 0)                                        
      printf_to_char(ch, "You still owe %s diamond%s.\n\r", 
                          numcpy( buf, ch->pcdata->clanowe_dia ),
                          ch->pcdata->clanowe_dia == 1 ? "" : "s" );
    else 
    {
      mprintf(sizeof(note_subject), note_subject, 
                "%s's debt to your clan.", ch->name);
      mprintf(sizeof(note_body), note_body, 
                "%s's diamond debt has been paid in full\n\rand deposited into your clan's account.\n\r", ch->name);
      note_line("{YTown Crier{x", NOTE_NOTE, 
                clan_name, note_subject, note_body);
      send_to_char("You have completely paid off your diamond debt.\n\r", 
                ch );
      if ( ch->pcdata->clanowe_level == 0 )
            ch->pcdata->clanowe_clan = 0;
    }
    save_clan_info(CLAN_FILE);                                              
      /*Insert Autonote to clan here.*/                                       
  } 
  else 
  {                                                                  
    send_to_char("You don't have any diamonds available to donate.\n\r", ch);
    return;                                                                 
  }                                                                         
}


/*
 * Start of all original code for the Lands of Draknor.
 */



void set_claneq( OBJ_DATA *obj, CHAR_DATA *ch )
{
    switch ( obj->item_type )
    {
        case ITEM_ARMOR:                                // CLAN CREST
            obj->value[0] = UMIN( ch->level / 5, 20 );  // level/5 or 20, whichever is smaller
            obj->value[1] = obj->value[0];
            obj->value[2] = obj->value[0];
            obj->value[3] = obj->value[0] * 3 / 4;      // ArmorVSMagic should always be lower than the rest.
            obj->level = ch->level;                     // Default to level of player
            break;
        case ITEM_WEAPON:                               // CLAN WEAPONS
            obj->value[1] = UMAX( ch->level / 12, 5 );  // lvl/12 or 5, whichever is higher
            obj->value[2] = ( ch->level + 7 ) / 5 + 2;  // Same value as in sample, worked the best.
            obj->level = ch->level;                     // Default to level of player
            break;
    }
}

/*
 * New Clan System starts including local declarations and functions
 */

#define CEDIT( fun )        bool fun( CHAR_DATA *ch, char *argument )

/*
 * Name: do_balance
 * Show someone their clan balance, and the balance of their allies.
 * Immortals can adjust the balance of clans by level.
 * Immortals also see the balance of all loaded clans.
 */
void do_balance( CHAR_DATA *ch, char * argument )
{
    CLAN_DATA   *clan       = NULL;
    char        buf[MSL];
    char        arg[MSL];
    char        arg1[MSL];
    int         adjustment = 0;
    argument = one_argument( argument, arg  );
    argument = one_argument( argument, arg1 );
   
    if ( !IS_IMMORTAL( ch )
    &&   !ch->clan ) 
    {
        if ( ch->pcdata->balance > 0 )
        {
            printf_to_char( ch, "Your current bank balance is %d.\n\r",
                ch->pcdata->balance );
            return;
        }
        else
        {
            send_to_char( "Your bank account is currently empty.\n\r", ch );
            return;
        }
    }

    if ( !IS_IMMORTAL( ch )
    &&   !ch->clan )
    {
        send_to_char( "But you are not in a clan.\n\r", ch );
        return;
    }

    if ( IS_NULLSTR( argument )
    &&   IS_NULLSTR( arg )
    &&   IS_NULLSTR( arg1 ) )
    {
        if ( IS_IMMORTAL( ch ) )
        {
            send_to_char( "Clan       Balance Spent  Total\n\r", ch );
            send_to_char( "{c====       ======= =====  =====\n\r\n\r{x", ch );
            for ( clan = clan_free ; clan ; clan = clan->next )
            {
              if (!IS_SET( clan->clan_flags, CLAN_INDEPENDENT ))
              {
                strncpy_color( buf,
                    FIX_STR( clan->symbol, "  (none)  ", "  (null)  " ),
                    10, ' ', TRUE );

                printf_to_char( ch,
                    "%10s %-6d  %-6d %-6d\n\r",
                        buf, clan->donation_balance,
                        clan->donation_spent, clan->donation_total );
              }
            }
            send_to_char( "\n\r{c===============================\n\r{x", ch );

            return;
        }
        else
        {
           
            clan = get_clan( ch->clan->name, FALSE );

            if ( ch->pcdata->balance > 0 )
                printf_to_char( ch, "Your personal bank balance is %d.\n\r\n\r",
                    ch->pcdata->balance );
            else
                send_to_char( "Your personal bank account is currently empty.\n\r", ch );

            if ( IS_SET( clan->clan_flags, CLAN_INDEPENDENT ) )
            {
                send_to_char( "You are in an independent organization.\n\r", ch );
                return;
            }

            strncpy_color( buf,
             FIX_STR( clan->symbol, "  (none)  ", "  (null)  " ),
             20, ' ', TRUE );

            send_to_char( "Clan      Balance Spent  Total\n\r", ch );
            send_to_char( "{c====      ======= =====  =====\n\r\n\r{c", ch );

            printf_to_char( ch,
                "%10s %-6d  %-6d %-6d\n\r",
                    clan->symbol, clan->donation_balance,
                    clan->donation_spent, clan->donation_total );


            return;
        }
    }

    if ( !IS_IMMORTAL( ch ) ) //Mortals stop here, later leaders keep going.
    {
        send_to_char( "You may only view balances, not adjust them.\n\r", ch );
        return;
    }

    if ( is_number( arg ) )
    {
        send_to_char(
            "Syntax: balance <clan> <(+/-)amount> <Reason for changing>\n\r", ch );
        return;
    }

    if ( !( clan = get_clan( arg, TRUE ) ) )
    {
        send_to_char( "Clan was not found to adjust.\n\r", ch );
        printf_to_char( ch, "%s is not a valid clan-name.\n\r",
            argument );
        return;
    }

    if ( IS_NULLSTR( arg1 )
    ||   IS_NULLSTR( argument ) )
    {
        send_to_char(
            "Syntax: balance <clan> <(+/-)amount> <Reason for changing>\n\r", ch );
        return;
    }

    if ( !is_number( arg1 )
    ||   is_number( argument ) )
    {
        send_to_char(
            "Syntax: balance <clan> <(+/-)amount> <Reason for changing>\n\r", ch );
        return;
    }

    adjustment = atoi( arg1 );
    clan->donation_balance += adjustment;
    clan->donation_spent   -= adjustment;
    char direc[20];
    if (adjustment < 0)
      strcpy(direc,"decreased");
    else
      strcpy(direc,"increased");

    printf_to_char( ch, "You have %s %s's clan balance to %d (%d spent).\n\r",
        direc, clan->name, clan->donation_balance, clan->donation_spent );


    mprintf(sizeof(buf), buf,"{m%s{w %s %s's balance by %d to %d (%d spent) {m[%s{m]{x.",
        ch->name,
        direc,
        clan->name,
        adjustment > 0 ? adjustment : 0-adjustment,
        clan->donation_balance,
        clan->donation_spent,
        argument );

    wiznet(buf,NULL,NULL,WIZ_SECURE,0,get_trust(ch));
    log_string( buf ); 

    save_clan( clan );
}

/*
 * Name: get_clan
 * Run a search on the clans and focus on one. BOOL value to allow the
 * clan to be loaded if desired.
 */
CLAN_DATA *get_clan( char *arg, bool force_load )
{
    CLAN_DATA   *clan;
    char        cmpstr[MIL];
    char        buf[MSL];
    int         index,i;

    if ( IS_NULLSTR( arg ) ) return NULL;

    strcpy( cmpstr, capitalize( arg ) );

    for ( clan = clan_free; clan; clan = clan->next )
        if ( !strcmp( cmpstr, clan->name ) )
            return clan;

    if ( force_load )
    {
        if ( ( clan = load_clan( arg ) ) ) 
            return clan;

        // Need to strip the colors first...
        for (index = 0; index < actual_num_clans; index++ )
        {
            strip_color( buf, clan_table[index].name );
            buf[0] = UPPER( buf[0] );
            if ( !strcmp( buf, arg ) )
            {
                clan = new_clan();
                clan->name = str_dup( buf, clan->name );
                clan->symbol = str_dup( clan_table[index].who_name, clan->symbol );
                clan->clan_immortal = str_dup( clan_table[index].patron, clan->clan_immortal );
                for (i=0;i<MAX_CONTINENT;i++)
                  clan->recall[i] = clan_table[index].recall_room[i];
                clan->donation_balance = clan_table[index].free_dia;
                clan->donation_spent   = clan_table[index].used_dia;
                clan->donation_total   = clan_table[index].total_dia;
                save_clan( clan );
                return clan;
            }
        }
    }
    return NULL;
}

char *rank_sex( CHAR_DATA *ch, RANK_DATA *rank )
{
    switch ( ch->sex )
    {
        case 0 :
            return rank->neutral;
        case 1 :
            return rank->male;
        case 2 :
            return rank->female;
        default  :
            return rank->neutral;
    }

}

char *get_rank( CHAR_DATA *ch )
{
    RANK_DATA   *rank;
    int          flag;

    for ( rank = ch->clan->rank; rank; rank = rank->next )
    {
        if ( rank->level == ch->pcdata->clan_rank )
        {
            if ( ( flag = rank->rank_flags & 65520 ) == 0 )
                return rank_sex( ch, rank );

            switch ( ch->gameclass )
            {
                case cConjurer :
                    if ( IS_SET( flag, RANK_CONJURER ) )
                        return rank_sex( ch, rank );
                    break;
                case cPriest :
                    if ( IS_SET( flag, RANK_PRIEST ) )
                        return rank_sex( ch, rank );
                    break;
                case cHighwayman :
                    if ( IS_SET( flag, RANK_HIGHWAYMAN ) )
                        return rank_sex( ch, rank );
                    break;
                case cKnight :
                    if ( IS_SET( flag, RANK_KNIGHT ) )
                        return rank_sex( ch, rank );
                    break;
                case cWarlock :
                    if ( IS_SET( flag, RANK_WARLOCK ) )
                        return rank_sex( ch, rank );
                    break;
                case cBarbarian :
                    if ( IS_SET( flag, RANK_BARBARIAN ) )
                        return rank_sex( ch, rank );
                    break;
                case cMystic :
                    if ( IS_SET( flag, RANK_MYSTIC ) )
                        return rank_sex( ch, rank );
                    break;
                case cDruid :
                    if ( IS_SET( flag, RANK_DRUID ) )
                        return rank_sex( ch, rank );
                    break;
                case cInquisitor :
                    if ( IS_SET( flag, RANK_INQUISITOR ) )
                        return rank_sex( ch, rank );
                    break;
                case cOccultist :
                    if ( IS_SET( flag, RANK_OCCULTIST ) )
                        return rank_sex( ch, rank );
                    break;
                case cAlchemist :
                    if ( IS_SET( flag, RANK_ALCHEMIST ) )
                        return rank_sex( ch, rank );
                    break;
                case cWoodsman :
                    if ( IS_SET( flag, RANK_WOODSMAN ) )
                        return rank_sex( ch, rank );
                    break;

            }
        }
    }
    return "";
}

void sort_rank_number( CLAN_DATA *clan )
{
    RANK_DATA *rank;
    int i = 1;

    if ( clan->rank )
    {
        for ( rank = clan->rank; rank; rank = rank->next )
            rank->number = i++;
        
        i--;
        clan->max_rank = i;

/*        if ( i > clan->max_rank )
            clan->max_rank = i;
    
        if ( i < clan->max_rank )
            clan->max_rank = i; //If for some reason the numbers are off.
*/
    }
}

void add_rank( CLAN_DATA *clan, RANK_DATA *rank )
{
    RANK_DATA *rank_prev = NULL;

    //So, clan->rank is NULL so return.
    if ( !clan->rank )
    {
        clan->rank = rank;
        return;
    }
    //If new rank is greater than existing rank
    if ( clan->rank->level < rank->level )
    {
        //next rank will be the greater rank.
        rank->next = clan->rank;
        clan->rank = rank;
        return;
    }
    //loop through 1 rank back, to 1 rank forward (if !NULL)
    for ( rank_prev = clan->rank; rank_prev; rank_prev = rank_prev->next )
    {
        if ( rank_prev->next == NULL || rank_prev->next->level < rank->level )
        {
            rank->next = rank_prev->next;
            rank_prev->next = rank;
            return;
        }
    }

}

/*bool is_donation_gem( OBJ_DATA *obj )
{
    CLAN_DATA *clan;

    for ( clan = clan->first ; clan ; clan = clan->next )
    {
        switch ( obj->pIndexData->vnum )
        {
            case clan->donation_gem :
                return TRUE;
                break;
            default :
                return FALSE;
                break;
            break;
        }
    }
    return FALSE;
}*/
                    
/* Clan Interpreter, called by do_cedit. */
void cedit( CHAR_DATA *ch, char *argument )
{
  CLAN_DATA *clan;
  char arg[MAX_STRING_LENGTH];
  char command[MAX_INPUT_LENGTH];
  int  cmd;

  EDIT_CLAN( ch, clan );
  
  smash_tilde( argument );
  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_CLAN ) )
  {
         send_to_char( "Insufficient security to edit clans.\n\r", ch );
          return;
  }

  if ( !str_cmp(command, "done") )
  {
      edit_done( ch );
      return;
  }

  if ( command[0] == '\0' )
  {
      cedit_show( ch, argument );
      return;
  }

  if ( !str_prefix( command, "save" ) )  //Force Save a Clan
  {
      save_clan( clan );
      printf_to_char( ch, "You have saved the clan %s.\n\r", clan->name );
      return;
  }

  /* Search Table and Dispatch Command. */
  for ( cmd = 0; cedit_table[cmd].name != NULL; cmd++ )
  {
      if ( !str_prefix( command, cedit_table[cmd].name ) )
      {
        if ( (*cedit_table[cmd].olc_fun) ( ch, argument ) )
        {
          SET_BIT( clan->clan_flags, CLAN_CHANGED );
          return;
        }
        else
            return; 
      }
   }

  /* Default to Standard Interpreter. */
  interpret( ch, arg );
  return;
}

/*
 * Name: do_cedit
 * The structure of the new clan system. Will call all other clan functions.
 *   -- Later will enter cedit, as we transfer this system into pure OLC format
 */
void do_cedit( CHAR_DATA *ch, char *argument )
{
     //CLAN_DATA CLAN;
     //FILE *fp;
     char arg[MIL];
     //char strsave[MIL];
     CLAN_DATA *clan = NULL;
     char arg1[MIL];
  char buf[MAX_STRING_LENGTH];

     //clan = &CLAN;
    mprintf(sizeof(buf), buf, "CEDIT %s{x", argument );
    wiznet( buf, NULL, NULL, WIZ_OLC, 0, get_trust( ch ) );

    argument = one_argument( argument, arg );

    argument = one_argument( argument, arg1 ); //  <-- this is the clanname... 
    
     if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_CLAN ) )
     {
        send_to_char( "Insufficient security to edit clans.\n\r", ch );
        return;
     }

     if (arg[0] == '\0')
     {
        send_to_char( "Syntax: cedit <clan name>\n\r        cedit <create>\n\r", ch );
        return;
     }

    if ( !str_prefix( arg, "create" ) )
    {
        if ( arg1[0] == '\0' )
        {
            send_to_char( "Syntax:  cedit create <name>\n\r", ch );
            return;
        }

        if ( cedit_create( ch, arg1 ) )
        {
            ch->desc->pEdit = (void *)clan;
            ch->desc->editor = ED_CLAN;
            return;
        }
        return;
    }

    if ( !strcmp( "list", arg ) )
    {
        send_to_char( "Name:            Symbol    Recall  Total  Spent  Balance Flags\n\r", ch );
        for ( clan = clan_free; clan; clan = clan->next )
        {
            printf_to_char(ch, "%-15s  %9s %-6d  %-6d %-6d %-6d  %s\n\r",
                clan->name,
                clan->symbol,
                clan->recall[0],
                clan->donation_total,
                clan->donation_spent,
                clan->donation_balance,
                flag_string( clan_flags, clan->clan_flags ) );
        }
        return;
    }

    if ( ( clan = get_clan( arg, TRUE ) ) )
    {
        if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_CLAN ) )
        {
            send_to_char( "Insuffiecient security to edit clans.\n\r", ch );
            return;
        }
    
        ch->desc->pEdit = (void *)clan;
        ch->desc->editor = ED_CLAN;
        return;
    }

    if ( !str_prefix( arg, "load" ) )
    {
        if ( IS_NULLSTR( arg1 ) )
        {
            send_to_char("Syntax: Cedit load <clan-name>.\n\r", ch);
            return;
        }
        
        if ( clan == NULL )
            clan = load_clan( arg1 );

        if ( clan == NULL )
        {
            printf_to_char( ch, "You failed to load %s.\n\r", arg1 );
            return;
        }
        return;
    } 

    send_to_char( "Syntax: cedit <clan name>\n\r        cedit <create>\n\r", ch );
    return;
}

void save_clan( CLAN_DATA *clan )
{
    FILE            *fp;
    char            strsave[MIL];
    ROSTER_DATA     *clannie;
    RANK_DATA       *rank;
    int i;

    if ( clan == NULL )
    {
        bug("save_clan: Failed to operate on NULL clan.", 0 );
        return;
    }

//    fclose( fpReserve );
//    nFilesOpen--;

    mprintf( sizeof( strsave ), strsave, "%s%s", 
                                CLAN_DIR, capitalize( clan->name ) );

    if( !( fp=fopen( strsave, "w" ) ) )
    {
        bugf( "Failed to write to clan file %s", clan->name );
        perror( clan->name );
//      fpReserve = fopen( NULL_FILE, "r" );
//      nFilesOpen++;
        return;
    }
    
    nFilesOpen++;
    fprintf( fp, "Name %s~\n",      clan->name              );
    fprintf( fp, "Symbol %s~\n",    clan->symbol            );
    fprintf( fp, "Immortal %s~\n",  clan->clan_immortal     );
    for (i=0;i<MAX_CONTINENT;i++)
    {
      fprintf( fp, "Recall%d %d\n",
        i, clan->recall[i]);
      fprintf( fp, "Pit%d %d\n",
        i, clan->donation_obj[i]);
    }
    fprintf( fp, "Gem %d\n",        clan->donation_gem      );
    fprintf( fp, "Balance %d\n",    clan->donation_balance  );
    fprintf( fp, "Spent %d\n",      clan->donation_spent    );
    fprintf( fp, "Total %d\n",      clan->donation_total    );
    fprintf( fp, "Status %s\n",     ltof( clan->clan_flags ));
    fprintf( fp, "Locker %d\n",     clan->locker            );
    fprintf( fp, "RosterStyle %s~\n", clan->roster_style    );

    fprintf( fp, "MaxRanks %d\n",   clan->max_rank          );

    if ( clan->rank )
    {
      fprintf( fp, "Ranks\n" );
      for ( rank = clan->rank ; rank ; rank = rank->next )
      {
          if ( rank->next )
              fprintf( fp, "TRUE" );
          else
              fprintf( fp, "FALSE" );

          fprintf( fp, " %d %s~ %s~ %s~ %s\n", rank->level, rank->male,
              rank->female, rank->neutral, ltof( rank->rank_flags ) );
      }
    }

    if ( clan->roster )
    {
      fprintf( fp, "Roster\n" );

      for ( clannie = clan->roster; clannie; clannie = clannie->next )
      {
          if ( clannie->next )
              fprintf( fp, "TRUE" );
          else
              fprintf( fp, "FALSE" );
    // This is _not_ printed in stone... It can be changed around..
          fprintf( fp, " %d %d %d %d\n", clannie->rank, clannie->donated,
                     clannie->sex, clannie->level );
          fprintf( fp, "%s~ %s~ NeW_ValuE~\n",
                     clannie->name, clannie->rank_symbol );
          fprintf( fp, "%s~ %s~\n",
                    clannie->title, clannie->afk_title );
          fprintf( fp, "%d %d %d %d %d %d %ld\n",
                    clannie->kills, clannie->deaths, clannie->pkills,
                    clannie->pdeaths, clannie->promoted, clannie->guilded,
                    clannie->logon );
          fprintf( fp, "%s~ %s~\n",
                    clannie->race, clannie->pc_class );
          //fprintf( fp, "%s ~\n", clannie->rank_symbol );
          //fprintf( fp, "%s ~\n", clannie->title );
      }
    }

    fprintf( fp, "End\n" );
    fclose( fp );

    nFilesOpen--;
//    fpReserve = fopen( NULL_FILE, "r" );
//    nFilesOpen++;
    return;
}

bool fread_clannie( FILE *fp, CLAN_DATA *clan )
{
    ROSTER_DATA *clannie;
    char        buf[MSL], buf1[MSL];
    char        *word;
    char        *tmpstr;

    clannie = new_clannie();

    word                = fread_word( fp );
    // Remember that fread_word returns a pointer to a static.

    clannie->rank       = fread_number( fp );
    clannie->donated    = fread_number( fp );
    clannie->sex        = fread_number( fp );
    clannie->level      = fread_number( fp );

    free_string( clannie->name );
    clannie->name       = fread_string( fp );

    free_string( clannie->rank_symbol );
    clannie->rank_symbol= fread_string( fp );

    tmpstr              = fread_string( fp );
    if ( strcmp( tmpstr, "NeW_ValuE" ) ) // If NULL, then newstyle..
    {
      free_string( clannie->title );
      clannie->title      = tmpstr;
    }
    else // Discard the string "NeW" and read new data...
    {
      free_string( tmpstr );
    
      free_string( clannie->title );
      clannie->title      = fread_string( fp );

      strcpy(buf1, clannie->title);
      if ( !strchr( ".,?!':", *buf1 ) )
      {
        mprintf( sizeof( buf ), buf, " %s", clannie->title );
      }
      else
      {
        mprintf( sizeof( buf ), buf, "%s", clannie->title );
      }


//      mprintf( sizeof( buf ), buf, " %s", clannie->title );
      free_string( clannie->title );
      str_dup( buf, clannie->title );
    
      free_string( clannie->afk_title );
      clannie->afk_title  = fread_string( fp );

      clannie->kills      = fread_number( fp );
      clannie->deaths     = fread_number( fp );
      clannie->pkills     = fread_number( fp );
      clannie->pdeaths    = fread_number( fp );
      clannie->promoted   = fread_number( fp );
      clannie->guilded    = fread_number( fp );
      clannie->logon      = fread_number( fp );

      free_string( clannie->race );
      clannie->race       = fread_string( fp );

      free_string( clannie->pc_class );
      clannie->pc_class      = fread_string( fp );

    }

    add_clannie( clan, clannie );

    return ( str_cmp( word, "FALSE" ) );
}

bool fread_rank( FILE *fp, CLAN_DATA *clan )
{
    RANK_DATA   *rank;
    char        *word;

    rank = new_rank();

    word                = fread_word( fp );
    // Remember that fread_word returns a pointer to a static.

    rank->level         = fread_number( fp );
    free_string( rank->male );
    rank->male          = fread_string( fp );
    free_string( rank->female );
    rank->female        = fread_string( fp );
    free_string( rank->neutral );
    rank->neutral       = fread_string( fp );
    rank->rank_flags    = fread_flag( fp );

    add_rank( clan, rank );
//    rank->next = clan->rank;
//    clan->rank = rank;

    return ( str_cmp( word, "FALSE" ) );
}

//For use of SKEY until I move it to db.c
#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                \
                if ( !str_cmp( word, literal ) )    \
                {                                   \
                    field  = value;                 \
                    fMatch = TRUE;                  \
                    break;                          \
                                }
#define SKEY( string, field )                       \
                if ( !str_cmp( word, string ) )     \
                {                                   \
                    free_string( field );           \
                    field = fread_string( fp );     \
                    fMatch = TRUE;                  \
                    break;                          \
                }

CLAN_DATA *load_clan( char *argument )
{
    FILE        *fp;
    char        *word;
    bool         fMatch = TRUE;
    CLAN_DATA   *clan;
    char         strload[MIL];
    int          i;

    mprintf( sizeof( strload ), strload, "%s%s", 
             CLAN_DIR, capitalize( argument ) );

    if ( ( fp = fopen( strload, "r" ) ) )
    {

        if ( !( clan = get_clan( argument, FALSE ) ) )
        {
            clan = new_clan();
            clan->name = str_dup( capitalize( argument ), clan->name );
        }
        clan->clan_immortal     = str_dup( "{gAa{Dr{wc{gh{wa{gne{x", 
                                            clan->clan_immortal );
        clan->symbol            = str_dup( "[ Empty ]", clan->symbol );
        clan->roster_style      = str_dup( "{x[%o%l %r %c] %n%t{x %s ?l{D[%d]{x %g!l{x", clan->roster_style );
        clan->donation_spent    = 0;
        clan->donation_total    = 0;
        clan->donation_balance  = 0;
        for (i=0;i<MAX_CONTINENT;i++)
        {
          clan->recall[i]       = 0;
          clan->donation_obj[i] = 0;
        }
        clan->donation_obj[0]   = 8400;
        clan->donation_gem      = 0;
        clan->locker            = 0;
        clan->roster            = NULL;

        for ( ; fMatch ; )
        {
            word = feof( fp ) ? "End" : fread_word( fp );
            if ( !strcmp( word, "End" ) )
                break;

            fMatch = FALSE;
            switch( UPPER( word[0]  )  )
            {
                case 'B' :
                    KEY( "Balance", clan->donation_balance, fread_number( fp ) );
                    break;
                case 'G' :
                    KEY( "Gem", clan->donation_gem, fread_number( fp ) );
                    break;
                case 'I' :
                    SKEY( "Immortal", clan->clan_immortal );
                    break;
                case 'L' :
                    KEY( "Locker", clan->locker, fread_number( fp ) );
                    break;
                case 'M' :
                    KEY( "MaxRanks", clan->max_rank, fread_number( fp ) );
                    break;
                case 'N' :
                    SKEY( "Name", clan->name );
                    break;
                case 'P' :
                    KEY( "Pit", clan->donation_obj[0], fread_number( fp ) ); // account for old clan pit data
                    for (i=0;i<MAX_CONTINENT;i++) // per-continent donate data
                    {
                      mprintf( sizeof(strload), strload, "Pit%d", i );
                      KEY( strload, clan->donation_obj[i], fread_number( fp ) );
                    }
                    break;
                case 'R':
                    KEY( "Recall", clan->recall[0], fread_number( fp ) ); // account for old clan recall data
                    for (i=0;i<MAX_CONTINENT;i++) // per-continent recall data
                    {
                      mprintf( sizeof(strload), strload, "Recall%d", i );
                      KEY( strload, clan->recall[i], fread_number( fp ) );
                    }

                    if ( !strcmp( word, "Ranks" ) )
                    { // The idea may be good, but there is no ranklist yet..
                      // You must do an exact copy of the roster read....
                        for ( ;fread_rank( fp, clan ); );
                        sort_rank_number( clan );
                        fMatch = TRUE;
                        break;
                    }

                    if ( !strcmp( word, "Roster" ) )
                    {
                        for ( ;fread_clannie( fp, clan ); );
                        fMatch = TRUE;
                        break;
                    }
                    SKEY( "RosterStyle", clan->roster_style );


                case 'S' :
                    KEY( "Spent", clan->donation_spent, fread_number( fp ) );
                    KEY( "Status",   clan->clan_flags, fread_flag( fp ) );
                    SKEY( "Symbol", clan->symbol );
                    break;
                case 'T' :
                    KEY( "Total", clan->donation_total, fread_number( fp ) );
                    break;
                break;
            }
        }
        fclose( fp );
    }
    else
    {
        // bug("load_clan: Failed to find clan.", 0 );
        clan = NULL;
    }

    return clan;
}

/*
 *  Name: cedit_recall
 *  Designed to set the recall room vnum for each clan.
 *  Starts knowing character and clan name
 *  Returns recall
 */
CEDIT ( cedit_recall )
{
    CLAN_DATA *clan;
    ROOM_INDEX_DATA *room;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    EDIT_CLAN( ch, clan );

    if ( clan == NULL )
    {
        bug( "cedit_recall: Failed to operate on NULL clan.", 0 );
        return FALSE;
    }

    if ( IS_NULLSTR( arg1 )
    ||   IS_NULLSTR( arg2 ))
    {
        send_to_char( "Syntax: recall <area#> <room vnum>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( arg1 ) )
    {
        send_to_char( "The continent # must be a numeric value.\n\r", ch );
        return FALSE;
    }

    if ( !is_number( arg2 ) )
    {
        send_to_char( "The Recall room must be a numeric value.\n\r", ch );
        return FALSE;
    }

    if (atoi(arg2) == 0)
    {
      clan->recall[atoi(arg1)] = 0;
      printf_to_char( ch, "Clan Recall for continent %d has been set to default.\n\r", atoi(arg1) );
      loggedf( "Recall #%d removed for clan.", atoi(arg1) );
      return TRUE;
    }

    if ( ( room = find_location( ch, arg2 ) ) == NULL )
    {
      send_to_char( "You can only set the clan recall to a room that exists...\n\r", ch );
      return FALSE;
    }

    if ( IS_SET( clan->clan_flags, CLAN_INDEPENDENT ) )
    {
        send_to_char( "Independent clans recall based on alignment.\n\r", ch );
        clan->recall[atoi(arg1)] = 0; //Might as well..
        return FALSE;
    }

    clan->recall[atoi(arg1)] = atoi(arg2);
    printf_to_char( ch, "Clan Recall for continent %d has been set to %d.\n\r", atoi(arg1), atoi(arg2) );
    loggedf( "Recall #%d Vnum %d set for clan.", atoi(arg1), atoi(arg2) );
    return TRUE;
}

CEDIT( cedit_show )
{
    CLAN_DATA *clan;
    RANK_DATA *rank;
    BUFFER    *buffer;
    char       buf[MSL];
    char       buf1[MSL];
    char       buf2[MSL];
    int        i;

    EDIT_CLAN( ch, clan );

    if ( clan == NULL ) return FALSE;

    buffer = new_buf();

    bstrcat( buffer,
        "\n\r {g*{x==============================================================={g*\n\r{x" );

    bprintf( buffer,
        " |  {cBasic Information                {x |  {cDonation Information{x    |\n\r",
            clan->name );
    
    bprintf( buffer,
        " |  Name           %-10s         |  Balance   %-7d       |\n\r",
            clan->name, clan->donation_balance );

    strncpy_color( buf,
             FIX_STR( clan->symbol, "  (none)  ", "  (null)  " ),
             10, ' ', TRUE );

    bprintf( buffer,
        " |  Symbol         %-10s         |  Spent     %-7d       |\n\r",
            buf, clan->donation_spent );

    strncpy_color( buf1,
             FIX_STR( clan->clan_immortal, "  (none)  ", "  (null)  " ),
             10, ' ', TRUE );

    bprintf( buffer,
        " |  Immortal       %-10s         |  Total     %-7d       |\n\r",
            buf1, clan->donation_total );

    bprintf( buffer,
        " |  Locker         %-10d         |  Gem       %-7d       |\n\r",
            clan->locker, clan->donation_gem );


    for (i=0;i<MAX_CONTINENT;i++)
    {
      if (continent_table[i].playerarea)
      {
        strncpy_color( buf,continent_table[i].displayname, 11, ' ', TRUE );
        bprintf( buffer,
          " |  Recall %-2d      %-5d (%s)|  Pit %-2d    %-7d       |\n\r",
          i, clan->recall[i], buf, i, clan->donation_obj[i] );
      }
    }

    bstrcat( buffer,
        " {g*{x==============================================================={g*{x\n\r" );

    bprintf( buffer,
        "    {cStatus Flags{x [  %s  ]\n\r{x",
            flag_string( clan_flags, clan->clan_flags ) );

    bstrcat( buffer,
        " {g*{x==============================================================={g*\n\r{x" );

    bstrcat( buffer,
        "    {cRoster Information{x\n\r" );

    bprintf( buffer,
        "    {cRoster_style{x %s{x\n\r\n\r",
            clan->roster_style );

    bprintf( buffer,
        "    Number of Ranks %d\n\r\n\r",
            clan->max_rank );

    if ( clan->rank )
    {
        bstrcat( buffer,
            "{cNum Lvl Male             Female            Neutral{x\n\r\n\r" );

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

            bprintf( buffer,
                "%3d %3d %-18s %-18s  %-18s %s\n\r",
                    rank->number,
                    rank->level,
                    buf,
                    buf1,
                    buf2,
                    flag_string( rank_flags, rank->rank_flags ) );
        }
    }

    page_to_char( buf_string( buffer ), ch );
    free_buf( buffer );

    return TRUE;
}

CEDIT( cedit_pit )
{
    CLAN_DATA *clan;
    OBJ_INDEX_DATA *pObj;

    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    EDIT_CLAN( ch, clan );

    if ( clan == NULL )
    {
        bug( "cedit_recall: Failed to operate on NULL clan.", 0 );
        return FALSE;
    }

    if ( IS_NULLSTR( arg1 )
    ||   IS_NULLSTR( arg2 ))
    {
        send_to_char( "Syntax: pit <continent#> <value>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( arg1 ) )
    {
        send_to_char( "The continent # must be a numeric value.\n\r", ch );
        return FALSE;
    }

    if ( !is_number( arg2 ) )
    {
        send_to_char( "The donation pit must be a numeric value.\n\r", ch );
        return FALSE;
    }


    if ( !( pObj = get_obj_index( atoi( argument ) ) ) )
    {
      send_to_char( "CEdit Donation Pit:  That vnum does not exist.\n\r", ch );
      return FALSE;
    }

    if ( pObj->item_type != ITEM_CONTAINER )
    {
        send_to_char( "You can only set a donation pit to a container.\n\r", ch );
        return FALSE;
    }

    if (atoi(arg2) == 0)
    {
      clan->donation_obj[atoi(arg1)] = 0;
      printf_to_char( ch, "Clan Pit for continent %d has been set to default.\n\r", atoi(arg1) );
      loggedf( "Pit #%d removed for clan.", atoi(arg1) );
      return TRUE;
    }

    if ( IS_SET( clan->clan_flags, CLAN_INDEPENDENT ) )
    {
        send_to_char( "Independent clans donate by alignment.\n\r", ch );
        return FALSE;
    }

    clan->donation_obj[atoi(arg1)] = atoi(arg2);
    printf_to_char( ch, "Clan Pit for continent %d has been set to %d.\n\r", atoi(arg1), atoi(arg2) );
    loggedf( "Pit #%d Vnum %d set for clan.", atoi(arg1), atoi(arg2) );
    return TRUE;
}

CEDIT( cedit_gem )
{
    CLAN_DATA *clan;
    OBJ_INDEX_DATA *pObj;
    
    EDIT_CLAN( ch, clan );

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: gem <value>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( argument ) )
    {
        send_to_char( "A donation gem must be a numeric Vnum.\n\r", ch );
        return FALSE;
    }

    if ( !( pObj = get_obj_index( atoi( argument ) ) ) )
    {
      send_to_char( "CEdit Donation Gem:  That vnum does not exist.\n\r", ch );
      return FALSE;
    }

    if ( pObj->item_type != ITEM_GEM )
    {
        send_to_char( "You can only set a donation gem to a gem-type object.\n\r", ch );
        return FALSE;
    }

    if ( IS_SET( clan->clan_flags, CLAN_INDEPENDENT ) )
    {
        send_to_char( "Independent clans can't donate gems.\n\r", ch );
        clan->donation_gem = 0;
        return FALSE;
    }

    clan->donation_gem = atoi( argument );
    printf_to_char( ch, "Donation Gem has been set to %d.\n\r", clan->donation_gem );
    loggedf( "Donation Gem Vnum %d set for clan %s.",
        clan->donation_gem, clan->name );
    return TRUE;

}

CEDIT( cedit_balance )
{
    CLAN_DATA *clan;
    int old_balance = 0;

    EDIT_CLAN( ch, clan );

    old_balance = clan->donation_balance;

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: balance <value>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( argument ) )
    {
        send_to_char( "The clan balance must be a numeric value.\n\r", ch );
        return FALSE;
    }
    
    if ( atoi( argument ) < 0 )
    {
        send_to_char( "Come on, you can't set a balance below 0.\n\r", ch );
        return FALSE;
    }

    if ( clan->donation_balance > 0 
    && ( get_trust( ch ) < 108 ) )
    {
        send_to_char( "When adjusting a clan balance, please use the balance command.\n\r", ch );
        return FALSE;
    }

    clan->donation_balance = atoi( argument );
    printf_to_char( ch, "Clan balance has been changed from %d to %d.\n\r",
        old_balance, clan->donation_balance );
    loggedf( "Clan Balance changed from %d to %d for clan %s.",
        old_balance, clan->donation_balance, clan->name );
    return TRUE;

}

CEDIT( cedit_spent )
{
    CLAN_DATA *clan;
    int old_spent = 0;

    EDIT_CLAN( ch, clan );

    old_spent = clan->donation_spent;

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: spent <value>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( argument ) )
    {
        send_to_char( "The clan spent must be a numeric value.\n\r", ch );
        return FALSE;
    }

    if ( atoi( argument ) < 0 )
    {
        send_to_char( "Come on, you can't set what a clan spent below 0.\n\r", ch );
        return FALSE;
    }

    if ( clan->donation_spent > 0
    && ( get_trust( ch ) < 108 ) )
    {
        send_to_char( "You are not powerful enough to edit this value.\n\r", ch );
        return FALSE;
    }

    clan->donation_spent = atoi( argument );
    printf_to_char( ch, "Clan spent has been changed from %d to %d.\n\r",
        old_spent, clan->donation_spent );
    loggedf( "Clan Spent changed from %d to %d for clan %s.",
        old_spent, clan->donation_spent, clan->name );
    return TRUE;

}

CEDIT( cedit_total )
{
    CLAN_DATA *clan;
    int old_total = 0;

    EDIT_CLAN( ch, clan );

    old_total = clan->donation_total;

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: total <value>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( argument ) )
    {
        send_to_char( "The clan total must be a numeric value.\n\r", ch );
        return FALSE;
    }

    if ( atoi( argument ) < 0 )
    {
        send_to_char( "Come on, you can't set a clan total below 0.\n\r", ch );
        return FALSE;
    }

    if ( clan->donation_spent > 0
    && ( get_trust( ch ) < 108 ) )
    {
        send_to_char( "You are not powerful enough to edit this value.\n\r", ch );
        return FALSE;
    }

    clan->donation_total = atoi( argument );
    printf_to_char( ch, "Clan total has been changed from %d to %d.\n\r",
        old_total, clan->donation_total );
    loggedf( "Clan Total changed from %d to %d for clan %s.",
        old_total, clan->donation_total, clan->name );
    return TRUE;

}

CEDIT( cedit_immortal )
{
    CLAN_DATA *clan;

    EDIT_CLAN( ch, clan );

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: patron <name>\n\r", ch );
        return FALSE;
    }

    if ( is_number( argument ) )
    {
        send_to_char( "Cedit Patron: number found where string was expected.\n\r", ch );
        return FALSE;
    }

    free_string( clan->clan_immortal );
    clan->clan_immortal = str_dup( argument ,clan->clan_immortal);

    send_to_char( "Clan Immortal Set.\n\r", ch);
    loggedf( "Clan immortal set to %s for clan %s.",
        clan->clan_immortal, clan->name );
    return TRUE;
}

CEDIT( cedit_symbol )
{
    CLAN_DATA *clan;

    EDIT_CLAN( ch, clan );

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: symbol <name>\n\r", ch );
        return FALSE;
    }

    if ( is_number( argument ) )
    {
        send_to_char( "Cedit Symbol: number found where string was expected.\n\r", ch );
        return FALSE;
    }

    free_string( clan->symbol );
    clan->symbol = str_dup( argument ,clan->symbol );

    send_to_char( "Clan Symbol Set.\n\r", ch);

    loggedf( "Clan Symbol set to %s for clan %s.",
        clan->symbol, clan->name );
    return TRUE;
}

CEDIT( cedit_roster_style )
{
    CLAN_DATA *clan;

    EDIT_CLAN( ch, clan );

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: roster_style <string>\n\r", ch );
        return FALSE;
    }

    if ( is_number( argument ) )
    {
        send_to_char( "Cedit Roster_Style: number found where string was expected.\n\r", ch );
        return FALSE;
    }

    free_string( clan->roster_style );
    clan->roster_style = str_dup( argument ,clan->roster_style );

    send_to_char( "Clan Roster Style Set.\n\r", ch);

    loggedf( "Clan Roster Style set to %s for clan %s.",
        clan->roster_style, clan->name );
    return TRUE;
}

CEDIT( cedit_create )
{
    CLAN_DATA *clan;
    FILE *fp;
    char strsave[MIL];

    EDIT_CLAN( ch, clan );

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: cedit create <name>\n\r", ch );
        return FALSE;
    }

    if ( ( clan = get_clan( capitalize( argument ), TRUE ) ) )
    {
        send_to_char( "clan name already exists, use cedit <clan name>.\n\r", ch );
        return FALSE;
    }


    clan = new_clan();

    mprintf( sizeof( strsave ), strsave, "%s%s", CLAN_DIR, 
                                capitalize( argument ) );
    clan->name = str_dup( capitalize( argument ), clan->name );

    /* Open an initial save point, just make the file and quit */
    if ( ( fp = fopen( strsave, "w" ) ) == NULL)
    {
        bugf( "Could not create Clan %s: fopen", capitalize( argument ) );
        perror(strsave);
        return FALSE;
    }

    nFilesOpen++;
    fclose( fp );
    nFilesOpen--;

    save_clan( clan );
    printf_to_char( ch, "You have created the clan, %s.\n\r", clan->name );
    
    return TRUE;
}

CEDIT( cedit_status )
{
    CLAN_DATA *clan;
    long flags;

    EDIT_CLAN( ch, clan );

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: status <flag(s)>\n\r", ch );
        send_to_char( "        ? status\n\r", ch );
        return FALSE;
    }

    if ( ( flags = flag_new_value( ch, clan_flags, argument,
           clan->clan_flags ) ) != NO_FLAG )
    {
        clan->clan_flags = flags;
        printf_to_char( ch, "Clan status flags set. [%s]\n\r",
            flag_string( clan_flags, clan->clan_flags ) );
        loggedf( "%s: Clan status flags set for %s [%s]",
            ch->name, clan->name,
            flag_string( clan_flags, clan->clan_flags ) );
        return TRUE;
    }

    return FALSE;
}

CEDIT( cedit_max_rank )
{
    CLAN_DATA   *clan;
    char        arg[MIL];
    char        arg1[MIL];

    EDIT_CLAN( ch, clan );

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );

    if ( IS_NULLSTR( arg ) )
    {
        send_to_char( "Syntax: max_rank <number>", ch );
        return FALSE;
    }

    if ( !is_number( arg ) )
    {
        send_to_char( "This value must be a number.\n\r", ch );
        return FALSE;
    }

    if ( clan->max_rank
    && ( str_cmp( arg1, "overwrite" ) ) )
    {
        send_to_char( "Number of ranks already set.\n\r", ch );
        send_to_char( "Syntax: max_rank <# of ranks> <overwrite>\n\r", ch );
        send_to_char( "{RWarning{x: overwrite will set all ranks back to default.\n\r", ch );
        return FALSE;
    }

    clan->max_rank = atoi( arg );
    printf_to_char( ch, "Max number of ranks set at %d, for clan %s.\n\r",
        clan->max_rank, clan->name );
    loggedf( "[%s] - Max Rank set to %d.",
        clan->name, clan->max_rank );

    RANK_DATA *rank, *rank_next;

    for ( rank = clan->rank; rank; rank = rank_next )
    {
        // since we are deleting ranks, we MUST save the pointer to next
        // in another variable, rank_next.
      rank_next = rank->next;
      free_rank( rank );
    }

    int i;
    int j=13;
    clan->actual_ranks = 0;
    clan->rank = NULL;

    for ( i = clan->max_rank ; i > 0 ; i-- )
    {        
        rank = new_rank();
        clan->actual_ranks++;
//        rank->next = clan->rank;
//        clan->rank = rank;
        add_rank( clan, rank );

          rank->number = ( rank->level = i );
          rank->male      = str_dup( clan_rank_table[j].title_of_rank[1], 
                                     rank->male );
          rank->female    = str_dup( clan_rank_table[j].title_of_rank[2], 
                                     rank->female );
          rank->neutral   = str_dup( clan_rank_table[j].title_of_rank[0], 
                                     rank->neutral );

        if ( j > 0 )
            j--;
    }

    return TRUE;

 }

CEDIT( cedit_male_rank )
{
    CLAN_DATA   *clan;
    RANK_DATA   *rank;
    char        arg[MIL];

    EDIT_CLAN( ch, clan );

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR( arg ) )
    {
        send_to_char( "Syntax: male_rank <#> <string>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( arg )
    ||   IS_NULLSTR( argument )
    || ( atoi( arg ) < 1 || atoi( arg ) > clan->max_rank ) )
    {
        send_to_char( "Invalid Syntax\n\rSyntax: male_rank <#> <string> \n\r", ch );
        return FALSE;
    }

    for ( rank = clan->rank ; rank ; rank = rank->next )
    {
        if ( ( rank->number == atoi( arg ) ) )
        {
            
            free_string( rank->male );
            rank->male = str_dup( argument , rank->male );

            printf_to_char( ch, "You have set the male rank number %d to %s.\n\r",
                rank->number, rank->male );

            loggedf( "You have set the male rank number %d to %s[%s].",
                rank->number, rank->male, clan->name );

            return TRUE;
                
        }
    }

    return FALSE;
}

CEDIT( cedit_female_rank )
{
    CLAN_DATA   *clan;
    RANK_DATA   *rank;
    char        arg[MIL];

    EDIT_CLAN( ch, clan );

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR( arg )
    ||   IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: female_rank <#> <string> \n\r", ch );
        return FALSE;
    }

    if ( !is_number( arg )
    || ( atoi( arg ) < 1 || atoi( arg ) > clan->max_rank ) )
    {
        send_to_char( "Invalid Syntax\n\rSyntax: female_rank <#> <string> \n\r", ch );
        return FALSE;
    }

    for ( rank = clan->rank ; rank ; rank = rank->next )
    {
        if ( ( rank->number == atoi( arg ) ) )
        {

            free_string( rank->female );
            rank->female = str_dup( argument , rank->female );

            printf_to_char( ch, "You have set the female rank number %d to %s.\n\r",
                rank->number, rank->female );

            loggedf( "Rank number %d set to %s for clan %s.",
                rank->number, rank->female, clan->name );

            return TRUE;

        }
    }

    return FALSE;
}

CEDIT( cedit_neutral_rank )
{
    CLAN_DATA   *clan;
    RANK_DATA   *rank;
    char        arg[MIL];

    EDIT_CLAN( ch, clan );

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR( arg )
    ||   IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: neutral_rank <#> <string> \n\r", ch );
        return FALSE;
    }

    if ( !is_number( arg )
    || ( atoi( arg ) < 1 || atoi( arg ) > clan->max_rank ) )
    {
        send_to_char( "Invalid Syntax\n\rSyntax: neutral_rank <#> <string>\n\r", ch );
        return FALSE;
    }

    for ( rank = clan->rank ; rank ; rank = rank->next )
    {
        if ( ( rank->number == atoi( arg ) ) )
        {

            free_string( rank->neutral );
            rank->neutral = str_dup( argument , rank->neutral );

            printf_to_char( ch, "You have set the neutral rank number %d to %s.\n\r",
                rank->number, rank->neutral );

            loggedf( "Rank number %d set to %s.",
                rank->number, rank->neutral );

            return TRUE;

        }
    }

    return FALSE;
}

CEDIT( cedit_remove_rank )
{
    CLAN_DATA *clan;
    //RANK_DATA *rank;

    EDIT_CLAN( ch, clan );

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: rem_rank <#>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( argument ) )
    {
        send_to_char( "You must enter a rank number to delete.\n\r", ch );
        return FALSE;
    }

    if ( atoi( argument ) < 0 || atoi( argument ) > clan->max_rank )
    {
        printf_to_char( ch, "Valid numbers are from 1 to %d.\n\r", clan->max_rank );
        return FALSE;
    }

    remove_rank( ch, argument, clan );
    clan->actual_ranks--;
    clan->max_rank--;

    sort_rank_number( clan );
    send_to_char( "Rank removed.\n\r", ch );
    return TRUE;
}

CEDIT( cedit_rank_flags )
{
    RANK_DATA *rank;
    CLAN_DATA *clan;
    char      arg[MIL];
    char      arg1[MIL];
    long      flags;

    EDIT_CLAN( ch, clan );
   
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );

    if ( IS_NULLSTR( arg ) )
    {
        send_to_char( "Syntax: rank_flags <flag(s)>\n\r", ch );
        send_to_char( "        ? rank\n\r", ch );
        return FALSE;
    }

    if ( !is_number( arg ) )
    {
        send_to_char( "That is not a valid rank number.\n\r", ch );
        return FALSE;
    }

    /*if ( atoi( arg ) < 1 || atoi( arg ) > ch->clan->max_rank )
    {
        printf_to_char( ch, "Valid ranks are number 1 through %d.\n\r",
            ch->clan->max_rank );
        return FALSE;
    }*/

    for ( rank = clan->rank; rank; rank = rank->next )
    {
        if ( rank->number == atoi( arg ) )
        {
            if ( ( flags = flag_new_value( ch, rank_flags, arg1,
                   rank->rank_flags ) ) != NO_FLAG )
            {
                rank->rank_flags = flags;

                printf_to_char( ch, "Clan rank flags changed [%s]\n\r",
                    flag_string( rank_flags, rank->rank_flags ) );
                loggedf( "%s: Clan rank flags set for %s [%s]",
                    ch->name, clan->name,
                    flag_string( rank_flags, rank->rank_flags ) );

                return TRUE;
            }
        }
    }

    return FALSE;
}

CEDIT ( cedit_add_rank )
{
    CLAN_DATA *clan;
    RANK_DATA *rank;

    EDIT_CLAN( ch, clan );

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: add_rank <#>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( argument ) )
    {
        send_to_char( "Must insert a numeric value.\n\r", ch );
        return FALSE;
    }

    if ( !clan->rank )
    {
        send_to_char( "No ranks found, use 'max_rank #' instead.\n\r", ch );
        return FALSE;
    }

    rank = new_rank();
    clan->actual_ranks++;
    clan->max_rank++;

    rank->level     = atoi( argument );
    rank->male      = str_dup( clan_rank_table[0].title_of_rank[1],
                                 rank->male );
    rank->female    = str_dup( clan_rank_table[0].title_of_rank[2],
                                 rank->female );
    rank->neutral   = str_dup( clan_rank_table[0].title_of_rank[0],
                                 rank->neutral );

    add_rank( clan, rank );
    sort_rank_number( clan );

    printf_to_char( ch, "Rank level %d added to the clan.\n\r",
        rank->level );
    return TRUE;
}

CEDIT( cedit_rank_level )
{
    CLAN_DATA *clan;
    RANK_DATA *rank = NULL;
    char arg[MIL];

    EDIT_CLAN( ch, clan );

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR( arg )
    ||   IS_NULLSTR( argument ) 
    ||   !is_number( arg )
    ||   !is_number( argument ) )
    {
        send_to_char( "Syntax: rank_level <rank #> <level #>\n\r", ch );
        return FALSE;
    }

    if ( !clan->rank )
    {
        send_to_char( "You must set default ranks before changing them.\n\r", ch );
        return FALSE;
    }

    for ( rank = clan->rank ; rank ; rank = rank->next )
    {
        if ( atoi( arg ) == rank->number )
        {
            rank->level = atoi( argument );
            printf_to_char( ch, "Rank number %d set to level %d.\n\r",
                rank->number, rank->level );
            return TRUE;
        }
    }
    
    return FALSE;
}

CEDIT( cedit_locker )
{
    CLAN_DATA *clan;
    OBJ_INDEX_DATA *pObj;

    EDIT_CLAN( ch, clan );

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: locker <value>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( argument ) )
    {
        send_to_char( "A locker must be a numeric Vnum.\n\r", ch );
        return FALSE;
    }

    if ( !( pObj = get_obj_index( atoi( argument ) ) ) )
    {
      send_to_char( "CEdit Locker:  That vnum does not exist.\n\r", ch );
      return FALSE;
    }

    if ( pObj->item_type != ITEM_LOCKER )
    {
        send_to_char( "You can only set a clan locker to a locker.\n\r", ch );
        return FALSE;
    }

    clan->locker = atoi( argument );
    printf_to_char( ch, "Locker has been set to %d.\n\r",
                        clan->locker );
    loggedf( "Locker Vnum %d set for clan %s.",
        clan->locker, clan->name );

    return TRUE;
}

/*
 * Returns TRUE if the object can be used.
 */
bool can_use_clan_obj( CHAR_DATA *ch, OBJ_DATA *obj)
{
  if ( !obj ) return FALSE; //just in case

  if (!IS_NPC(ch)
  &&   IS_IMMORTAL(ch) )
    return TRUE;

  if ( IS_NULLSTR(obj->pIndexData->clan_name)
  || !strcmp( obj->pIndexData->clan_name, "none" ) )
    return TRUE;

  if ( !ch->clan )
    return FALSE;

  if ( strcmp(ch->clan->name, capitalize(obj->pIndexData->clan_name)) )
    return FALSE;

  return TRUE;
}
