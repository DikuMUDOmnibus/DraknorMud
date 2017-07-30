/*
 * OLC-style clan creation and maintanence.
 * This code is property of the Lands of Draknor and should not be
 * used without permission from the authors.
 */


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
#include "olc.h" 


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
        /*if ( (*redit_table[cmd].olc_fun) ( ch, argument ) )
        {
          SET_BIT( pArea->area_flags, AREA_CHANGED );
          SET_BIT( pArea->area_flags, AREA_CHANGED );
          return;
        }
        else*/
        (*cedit_table[cmd].olc_fun) ( ch, argument );
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

     //clan = &CLAN;
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
        for ( clan = clan_free; clan; clan = clan->next )
            printf_to_char(ch, "Clanname: %s  Symbol:   %s \n\r",
                           clan->name, clan->symbol );
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

    send_to_char( "This command is being worked on currently, try later.\n\r", ch );
    return;
}

void save_clan( CLAN_DATA *clan )
{
    FILE            *fp;
    char            strsave[MIL];
    ROSTER_DATA     *clannie;
    RANK_DATA       *rank;

    if ( clan == NULL )
    {
        bug("save_clan: Failed to operate on NULL clan.", 0 );
        return;
    }

    fclose( fpReserve );
    nFilesOpen--;

    mprintf( sizeof( strsave ), strsave, "%s%s",
                                CLAN_DIR, capitalize( clan->name ) );
    {
        bugf( "Failed to write to clan file %s", clan->name );
        perror( clan->name );
        fpReserve = fopen( NULL_FILE, "r" );
        nFilesOpen++;
        return;
    }

    nFilesOpen++;
    fprintf( fp, "Name %s~\n",      clan->name              );
    fprintf( fp, "Symbol %s~\n",    clan->symbol            );
    fprintf( fp, "Immortal %s~\n",  clan->clan_immortal     );
    fprintf( fp, "Recall %d\n",     clan->recall            );
    fprintf( fp, "Gem %d\n",        clan->donation_gem      );
    fprintf( fp, "Pit %d\n",        clan->donation_obj      );
    fprintf( fp, "Balance %d\n",    clan->donation_balance  );
    fprintf( fp, "Spent %d\n",      clan->donation_spent    );
    fprintf( fp, "Total %d\n",      clan->donation_total    );
    fprintf( fp, "Status %s\n",     ltof( clan->clan_flags ));

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

          fprintf( fp, " %d %s~ %s~ %s~\n", rank->number, rank->male,
              rank->female, rank->neutral );
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
          fprintf( fp, "%s~\n", clannie->name );
      }
    }

    fprintf( fp, "End\n" );
    fclose( fp );

    nFilesOpen--;
    fpReserve = fopen( NULL_FILE, "r" );
    nFilesOpen++;
    return;
}

bool fread_clannie( FILE *fp, CLAN_DATA *clan )
{
    ROSTER_DATA *clannie;
    char        *word;

    clannie = new_clannie();

    word                = fread_word( fp );
    clannie->rank       = fread_number( fp );
    clannie->donated    = fread_number( fp );
    clannie->sex        = fread_number( fp );
    clannie->level      = fread_number( fp );

    free_string( clannie->name );
    clannie->name       = fread_string( fp );

    add_clannie( clan, clannie );

    return ( str_cmp( word, "FALSE" ) );
}

bool fread_rank( FILE *fp, CLAN_DATA *clan )
{
    RANK_DATA   *rank;
    char        *word;

    rank = new_rank();

    word                = fread_word( fp );

    rank->number        = fread_number( fp );
    free_string( rank->male );
    rank->male          = fread_string( fp );
    free_string( rank->female );
    rank->female        = fread_string( fp );
    free_string( rank->neutral );
    rank->neutral       = fread_string( fp );

    rank->next = clan->rank;
    clan->rank = rank;

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
        clan->donation_spent    = 0;
        clan->donation_total    = 0;
        clan->donation_balance  = 0;
        clan->donation_obj      = 8400;
        clan->recall            = 8400;
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
                    KEY( "Pit", clan->donation_obj, fread_number( fp ) );
                    break;
                case 'R':
                    KEY( "Recall", clan->recall, fread_number( fp ) );

                    if ( !strcmp( word, "Ranks" ) )
                    { // The idea may be good, but there is no ranklist yet..
                      // You must do an exact copy of the roster read....
                        for ( ;fread_rank( fp, clan ); );
                        fMatch = TRUE;
                        break;
                    }

                    if ( !strcmp( word, "Roster" ) )
                    {
                        for ( ;fread_clannie( fp, clan ); );
                        fMatch = TRUE;
                        break;
                    }

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

    EDIT_CLAN( ch, clan );

    if ( clan == NULL )
    {
        bug( "cedit_recall: Failed to operate on NULL clan.", 0 );
        return FALSE;
    }

    if ( /*argument[0] == '\0'*/ IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: recall <room vnum>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( argument ) )
    {
        send_to_char( "The Recall room must be a numeric value.\n\r", ch );
        return FALSE;
    }

    if ( ( room = find_location( ch, argument ) ) == NULL )
    {
      send_to_char( "You can only set the clan recall to a room that exists...\n\r", ch );
      return FALSE;
    }

    clan->recall = atoi( argument );
    printf_to_char( ch, "Clan Recall has been set to %d.\n\r", clan->recall );
    loggedf( "Recall Vnum %d set for clan.", clan->recall );
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

    EDIT_CLAN( ch, clan );

    if ( clan == NULL ) return FALSE;

    buffer = new_buf();

    bstrcat( buffer,
        "\n\r {g*{x==============================================================={g*\n\r{x" );

    bprintf( buffer,
        " |  {cBasic Information             {x |  {cDonation Information{x       |\n\r",
            clan->name );

    bprintf( buffer,
        " |  Name           %-10s      |  Balance   %-7d          |\n\r",
            clan->name, clan->donation_balance );

    strncpy_color( buf,
             FIX_STR( clan->symbol, "  (none)  ", "  (null)  " ),
             10, ' ', TRUE );

    bprintf( buffer,
        " |  Symbol         %-10s      |  Spent     %-7d          |\n\r",
            buf, clan->donation_spent );

    strncpy_color( buf1,
             FIX_STR( clan->clan_immortal, "  (none)  ", "  (null)  " ),
             10, ' ', TRUE );

    bprintf( buffer,
        " |  Immortal       %-10s      |  Total     %-7d          |\n\r",
            buf1, clan->donation_total );

    bprintf( buffer,
        " |  Recall         %-10d      |  Gem       %-7d          |\n\r",
            clan->recall, clan->donation_gem );

    bprintf( buffer,
        " |                                 |  Pit       %-7d          |\n\r",
            clan->donation_obj );

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
        "    Number of Ranks %d\n\r\n\r",
            clan->max_rank );

    if ( clan->rank )
    {
        bstrcat( buffer,
            "{cNum Male               Female              Neutral{x\n\r\n\r" );

        for ( rank = clan->rank ; rank ; rank = rank->next )
        {
                strncpy_color( buf,
                    FIX_STR( rank->male, "<Recruit", "<Recruit" ),
                    18, ' ', TRUE );

                strncpy_color( buf1,
                    FIX_STR( rank->female, "<Recruit>", "<Recruit>" ),
                    18, ' ', TRUE );

                strncpy_color( buf2,
                    FIX_STR( rank->neutral, "<Recruit>", "<Recruit>" ),
                    18, ' ', TRUE );

            bprintf( buffer,
                "%3d %-18s %-18s  %-18s\n\r",
                    rank->number,
                    buf,
                    buf1,
                    buf2 );
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

    EDIT_CLAN( ch, clan );

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: pit <value>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( argument ) )
    {
        send_to_char( "A donation pit must be a numeric Vnum.\n\r", ch );
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

    clan->donation_obj = atoi( argument );
    printf_to_char( ch, "Donation Pit has been set to %d.\n\r",
                        clan->donation_obj );
    loggedf( "Donation Object Vnum %d set for clan %s.",
        clan->donation_obj, clan->name );
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

    if ( atoi( argument ) <= 0 )
    {
        send_to_char( "Come on, you can't set a balance to 0 or below.\n\r", ch );
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

    if ( atoi( argument ) <= 0 )
    {
        send_to_char( "Come on, you can't set what a clan spent to 0 or below.\n\r", ch );
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

    if ( atoi( argument ) <= 0 )
    {
        send_to_char( "Come on, you can't set a clan total to 0 or below.\n\r", ch );
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
        send_to_char( "Syntax: immortal <name>\n\r", ch );
        return FALSE;
    }

    if ( is_number( argument ) )
    {
        send_to_char( "Cedit Immortal: number found where string was expected.\n\r", ch );
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
        return FALSE;
    }

    if ( ( flags = flag_new_value( ch, clan_flags, argument,
           clan->clan_flags ) ) != NO_FLAG )
    {
        clan->clan_flags = flags;
        send_to_char( "Clan Status Flags set.\n\r", ch );
        loggedf( "%s: Clan Status flags set for %s [%s]",
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

    int i;
    int j=13;

    for ( i = clan->max_rank ; i > 0 ; i-- )
    {
        RANK_DATA   *rank;

        rank = new_rank();
        rank->next = clan->rank;
        clan->rank = rank;

          rank->number    = i;
          rank->male      = str_dup( clan_rank_table[j].title_of_rank[1],
                                     rank->male );
          rank->female    = str_dup( clan_rank_table[j].title_of_rank[2],
                                     rank->female );
          rank->neutral   = str_dup( clan_rank_table[j].title_of_rank[0],
                                     rank->neutral );

        printf_to_char( ch, "Ranks set to %s, %s, and %s. Number set to %d.\n\r",
            rank->male, rank->female,  rank->neutral, rank->number );

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
        send_to_char( "Syntax: male_rank <#> <string> <flags>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( arg )
    ||   IS_NULLSTR( argument )
    || ( atoi( arg ) < 1 || atoi( arg ) > clan->max_rank ) )
    {
        send_to_char( "Invalid Syntax\n\rSyntax: male_rank <#> <string> <flags>\n\r", ch );
        return FALSE;
    }

    for ( rank = clan->rank ; rank ; rank = rank->next )
    {
        if ( ( rank->number == atoi( arg ) ) )
        {

            free_string( rank->male );
            rank->male = str_dup( argument , rank->male );

            printf_to_char( ch, "Rank number %d set to %s.\n\r",
                rank->number, rank->male );

            loggedf( "Rank number %d set to %s for Clan %s",
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
        send_to_char( "Syntax: female_rank <#> <string> <flags>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( arg )
    || ( atoi( arg ) < 1 || atoi( arg ) > clan->max_rank ) )
    {
        send_to_char( "Invalid Syntax\n\rSyntax: female_rank <#> <string> <flags>\n\r", ch );
        return FALSE;
    }

    for ( rank = clan->rank ; rank ; rank = rank->next )
    {
        if ( ( rank->number == atoi( arg ) ) )
        {

            free_string( rank->female );
            rank->female = str_dup( argument , rank->female );

            printf_to_char( ch, "Rank number %d set to %s.\n\r",
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
        send_to_char( "Syntax: neutral_rank <#> <string> <flags>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( arg )
    || ( atoi( arg ) < 1 || atoi( arg ) > clan->max_rank ) )
    {
        send_to_char( "Invalid Syntax\n\rSyntax: neutral_rank <#> <string> <flags>\n\r", ch );
        return FALSE;
    }

    for ( rank = clan->rank ; rank ; rank = rank->next )
    {
        if ( ( rank->number == atoi( arg ) ) )
        {

            free_string( rank->neutral );
            rank->neutral = str_dup( argument , rank->neutral );

            printf_to_char( ch, "Rank number %d set to %s.\n\r",
                rank->number, rank->neutral );

            loggedf( "Rank number %d set to %s.",
                rank->number, rank->neutral );

            return TRUE;

        }
    }

    return FALSE;

