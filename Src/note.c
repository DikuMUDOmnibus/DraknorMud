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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"

/* globals from db.c for load_notes */
#if !defined(macintosh)
extern  int   _filbuf args( ( FILE * ) );
#endif
extern FILE * fpArea;
extern char   strArea[MAX_INPUT_LENGTH];

/* local procedures */
void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time );
void parse_note( CHAR_DATA *ch, char *argument, int64 type );
bool hide_note( CHAR_DATA *ch, NOTE_DATA *pnote, bool ignore_time );
void append_note( NOTE_DATA *pnote );
char *reply_format( char *argument );

bool display_unread( CHAR_DATA *ch, char *arg, NOTE_DATA *list );
bool hide_imm_note ( CHAR_DATA *ch, NOTE_DATA *pnote );

NOTE_DATA *note_list;
NOTE_DATA *idea_list;
NOTE_DATA *penalty_list;
NOTE_DATA *news_list;
NOTE_DATA *changes_list;
NOTE_DATA *rules_list;
NOTE_DATA *rpnote_list;

int count_spool( CHAR_DATA *ch, NOTE_DATA *spool )
{
  int count = 0;
  NOTE_DATA *pnote;

  for ( pnote = spool; pnote; pnote = pnote->next )
    if ( !hide_note( ch, pnote, FALSE ) )
      count++;

  return count;
}

int count_full_spool( CHAR_DATA *ch, NOTE_DATA *spool )
{
  int count = 0;
  NOTE_DATA *pnote;

  for ( pnote = spool; pnote; pnote = pnote->next )
    if ( !hide_note( ch, pnote, TRUE ) )
      count++;

  return count;
}

int forward_spool( CHAR_DATA *ch, NOTE_DATA *spoolfrom, NOTE_DATA *spoolto )
{
  int count = 0;
  NOTE_DATA *pNoteFrom, *pNoteTo, *spoolto_last;

  for ( pNoteFrom = spoolfrom; pNoteFrom; pNoteFrom = pNoteFrom->next )
    if ( is_name( ch->name, pNoteFrom->to_list )
         &&  !hide_note( ch, pNoteFrom, FALSE ) )
    {
      for ( spoolto_last = spoolto; spoolto_last->next ; )
        spoolto_last = spoolto_last->next;

      spoolto_last->next    = pNoteTo = new_note();
      pNoteTo->next          = NULL;
      pNoteTo->sender      = str_dup( pNoteFrom->sender, pNoteTo->sender );
      pNoteTo->date          = str_dup( pNoteFrom->date, pNoteTo->date );
      pNoteTo->date_stamp   = current_time;
      pNoteTo->to_list      = str_dup( ch->name, pNoteTo->to_list );
      pNoteTo->subject      = str_dup( pNoteFrom->subject, pNoteTo->subject );
      pNoteTo->text          = str_dup( pNoteFrom->text, pNoteTo->text );
      pNoteTo->type          = spoolto->type;
      count++;
    }

  return count;
}

bool quicknote_display( CHAR_DATA *ch, char *arg, NOTE_DATA *list )
{
  bool found = FALSE;
  int count = 0;
  if ( ( count = count_spool( ch, list ) ) > 0 )
  {
    found = TRUE;
    printf_to_char( ch, "{C|{c%-10s : {W%-8d{C|{x\n\r", arg, count );
  }
  return ( found );
}

void do_note( CHAR_DATA *ch, char *argument )
{
  parse_note( ch, argument, NOTE_NOTE );
}

void do_idea( CHAR_DATA *ch, char *argument )
{
  parse_note( ch, argument, NOTE_IDEA );
}

void do_penalty( CHAR_DATA *ch, char *argument )
{
  parse_note( ch, argument, NOTE_PENALTY );
}

void do_news( CHAR_DATA *ch, char *argument )
{
  parse_note( ch, argument, NOTE_NEWS );
}

void do_changes( CHAR_DATA *ch, char *argument )
{
  parse_note( ch, argument, NOTE_CHANGES );
}

void do_rules( CHAR_DATA *ch, char *argument )
{
  parse_note( ch, argument, NOTE_RULES );
}

void do_rpnote( CHAR_DATA *ch, char *argument )
{
  parse_note( ch, argument, NOTE_RPNOTE );
}

/*
 * Checks if a note is from named person.
 */
char *note_header_str( CHAR_DATA *ch, NOTE_DATA *pNote, int vnum, bool fList )
{
  static char buf[MAX_STRING_LENGTH];

  if ( fList )
    mprintf(sizeof(buf),buf, "[%3d%s]%s%s: %s\n\r",
            vnum,
            hide_note( ch, pNote, FALSE ) ? " " : "{YN{x",
            IS_NULLSTR( pNote->sender )
            ? " " : IS_IMMORTAL( ch ) ? "{M*{x" : " ",
            IS_NULLSTR( pNote->sender ) ? pNote->sender : pNote->sender,
            pNote->subject );
  return buf;
}


bool is_note_from( CHAR_DATA *ch, NOTE_DATA *pNote, char *name )
{
  if ( IS_NPC( ch ) )
    return FALSE;

  /* Immortals always see the actual sender. */
  if ( IS_IMMORTAL( ch ) && !str_prefix( name, pNote->sender ) )
    return TRUE;

  /* Mortals use sender as long as pNote->as isn't null. */
  if ( IS_NULLSTR( pNote->sender ) )
    return !str_prefix( name, pNote->sender );

  /* Someone posted as someone else. */
  return is_name( name, pNote->sender );
}

void save_notes( int type )
{
  FILE *fp;
  char *name;
  NOTE_DATA *pnote;

  switch ( type )
  {
    default:
      return;
    case NOTE_NOTE:
      name = NOTE_FILE;
      pnote = note_list;
      break;
    case NOTE_IDEA:
      name = IDEA_FILE;
      pnote = idea_list;
      break;
    case NOTE_PENALTY:
      name = PENALTY_FILE;
      pnote = penalty_list;
      break;
    case NOTE_NEWS:
      name = NEWS_FILE;
      pnote = news_list;
      break;
    case NOTE_CHANGES:
      name = CHANGES_FILE;
      pnote = changes_list;
      break;
    case NOTE_RULES:
      name = RULES_FILE;
      pnote = rules_list;
      break;
    case NOTE_RPNOTE:
      name = RPNOTE_FILE;
      pnote = rpnote_list;
      break;
  }

  fclose( fpReserve );
  nFilesOpen--;
  if ( ( fp = fopen( name, "w" ) ) == NULL )
    perror( name );
  else
  {
    nFilesOpen++;
    for ( ; pnote != NULL; pnote = pnote->next )
    {
      fprintf( fp, "Sender  %s~\n", pnote->sender);
      fprintf( fp, "Date    %s~\n", pnote->date);
      fprintf( fp, "Stamp   %ld\n", pnote->date_stamp);
      fprintf( fp, "To      %s~\n", pnote->to_list);
      fprintf( fp, "Subject %s~\n", pnote->subject);
      fprintf( fp, "Text\n%s~\n",   pnote->text);
    }
    fclose( fp );
    nFilesOpen--;
    fpReserve = fopen( NULL_FILE, "r" );
    nFilesOpen++;
    return;
  }
}

void load_notes(void)
{
  load_thread(NOTE_FILE,&note_list, NOTE_NOTE, 60*24*60*60); //Changing to 60 Days 09/12/2010 RWL
  load_thread(IDEA_FILE,&idea_list, NOTE_IDEA, 60*24*60*60); //60 Days
  load_thread(PENALTY_FILE,&penalty_list, NOTE_PENALTY, 0);
  load_thread(NEWS_FILE,&news_list, NOTE_NEWS, 0);
  load_thread(CHANGES_FILE,&changes_list,NOTE_CHANGES, 0);
  load_thread(RULES_FILE,&rules_list,NOTE_RULES, 0);
  load_thread(RPNOTE_FILE,&rpnote_list,NOTE_RPNOTE, 0); //Unlimited
}

void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time)
{
  FILE *fp;
  NOTE_DATA *pnotelast;

  if ( ( fp = fopen( name, "r" ) ) == NULL )
    return;
  nFilesOpen++;
  pnotelast = NULL;
  for ( ; ; )
  {
    NOTE_DATA *pnote;
    char letter;

    do
    {
      letter = getc( fp );
      if ( feof(fp) )
      {
        fclose( fp );
        nFilesOpen--;
        return;
      }
    }
    while ( isspace(letter) );
    ungetc( letter, fp );

    pnote           = new_note();

    if ( str_cmp( fread_word( fp ), "sender" ) )
      break;
    pnote->sender   = fread_string( fp );

    if ( str_cmp( fread_word( fp ), "date" ) )
      break;
    pnote->date     = fread_string( fp );

    if ( str_cmp( fread_word( fp ), "stamp" ) )
      break;
    pnote->date_stamp = fread_number(fp);

    if ( str_cmp( fread_word( fp ), "to" ) )
      break;
    pnote->to_list  = fread_string( fp );

    if ( str_cmp( fread_word( fp ), "subject" ) )
      break;
    pnote->subject  = fread_string( fp );

    if ( str_cmp( fread_word( fp ), "text" ) )
      break;
    pnote->text     = fread_string( fp );

    if (free_time && pnote->date_stamp < current_time - free_time)
    {
      free_note(pnote);
      continue;
    }

    pnote->type = type;

    if (*list == NULL)
      *list           = pnote;
    else
      pnotelast->next     = pnote;

    pnotelast       = pnote;
  }

  strcpy( strArea, NOTE_FILE );
  fpArea = fp;
  bugf( "Load_notes: bad key word. Discontinuing LOADING %s", name );
  return;
}

void append_note( NOTE_DATA *pnote )
{
  FILE *fp;
  char *name;
  NOTE_DATA **list;
  NOTE_DATA *last;

  switch ( pnote->type )
  {
    default:
      return;
    case NOTE_NOTE:
      name = NOTE_FILE;
      list = &note_list;
      break;
    case NOTE_IDEA:
      name = IDEA_FILE;
      list = &idea_list;
      break;
    case NOTE_PENALTY:
      name = PENALTY_FILE;
      list = &penalty_list;
      break;
    case NOTE_NEWS:
      name = NEWS_FILE;
      list = &news_list;
      break;
    case NOTE_CHANGES:
      name = CHANGES_FILE;
      list = &changes_list;
      break;
    case NOTE_RULES:
      name = RULES_FILE;
      list = &rules_list;
      break;
    case NOTE_RPNOTE:
      name = RPNOTE_FILE;
      list = &rpnote_list;
      break;
  }

  if ( *list == NULL )
    *list = pnote;
  else
  {
    for ( last = *list; last->next; last = last->next);
    last->next = pnote;
  }

  fclose( fpReserve );
  nFilesOpen--;
  if ( ( fp = fopen( name, "a" ) ) == NULL )
  {
    perror( name );
  }
  else
  {
    nFilesOpen++;
    fprintf( fp, "Sender  %s~\n", pnote->sender       );
    fprintf( fp, "Date    %s~\n", pnote->date         );
    fprintf( fp, "Stamp   %ld\n", pnote->date_stamp   );
    fprintf( fp, "To      %s~\n", pnote->to_list      );
    fprintf( fp, "Subject %s~\n", pnote->subject      );
    fprintf( fp, "Text\n%s~\n",   pnote->text         );
    fclose( fp );
    nFilesOpen--;
  }
  fpReserve = fopen( NULL_FILE, "r" );
  nFilesOpen++;
}

bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote )
{
  if ( !str_cmp( ch->name, pnote->sender ) )
    return TRUE;

  if ( is_exact_name( "all", pnote->to_list ) )
    return TRUE;

  if ( IS_IMMORTAL(ch) && is_exact_name( "immortal", pnote->to_list ) )
    return TRUE;
  if ( IS_IMMORTAL(ch) && is_exact_name( "imm", pnote->to_list ) )
    return TRUE;

  if ( IS_IMMORTAL(ch) && is_exact_name( "imms", pnote->to_list ) )
    return TRUE;

  if ( IS_IMMORTAL(ch) && is_exact_name( "immortals", pnote->to_list ) )
    return TRUE;

  if ( IS_IMMORTAL(ch) && is_exact_name( "clans", pnote->to_list ) )
    return TRUE;

  if ( ch->clan && is_exact_name( "clans", pnote->to_list ) )
    return TRUE;

  if ( IS_IN_CLAN( ch )
       &&   is_exact_name( ch->clan->name, pnote->to_list ) )
    return TRUE;

  if ( IS_IN_CLAN( ch )
       &&  !strcmp(ch->clan->name, "Scholars" )
       &&   is_exact_name( "sokg", pnote->to_list ) )
    return TRUE;

  if ( IS_IN_CLAN( ch )
       &&  !strcmp(ch->clan->name, "Law" )
       &&   is_exact_name( "lod", pnote->to_list ) )
    return TRUE;

  if (IS_IMPLEMENTOR(ch)
      &&  is_exact_name("IMP",pnote->to_list))
    return TRUE;

  if ( is_exact_name( ch->name, pnote->to_list ) )
    return TRUE;

  return FALSE;
}



void note_attach( CHAR_DATA *ch, int type )
{
  NOTE_DATA *pnote;

  if ( ch->pnote != NULL )
    return;

  pnote = new_note();

  pnote->next    = NULL;
  pnote->sender      = str_dup( ch->name ,pnote->sender);
  pnote->date    = str_dup( "" ,pnote->date);
  pnote->to_list  = str_dup( "" ,pnote->to_list);
  pnote->subject  = str_dup( "" ,pnote->subject);
  pnote->text    = str_dup( "" ,pnote->text);
  pnote->type    = type;
  ch->pnote    = pnote;
  return;
}



void note_remove( CHAR_DATA *ch, NOTE_DATA *pnote, bool delete_it)
{
  char to_new[MAX_INPUT_LENGTH];
  char to_one[MAX_INPUT_LENGTH];
  NOTE_DATA *prev;
  NOTE_DATA **list;
  char *to_list;

  if (!delete_it)
  {
    /* make a new list */
    to_new[0]  = '\0';
    to_list  = pnote->to_list;
    while ( *to_list != '\0' )
    {
      to_list  = one_argument( to_list, to_one );
      if ( to_one[0] != '\0' && str_cmp( ch->name, to_one ) )
      {
        strcat( to_new, " " );
        strcat( to_new, to_one );
      }
    }
    /* Just a simple recipient removal? */
    if ( str_cmp( ch->name, pnote->sender ) && to_new[0] != '\0' )
    {
      free_string( pnote->to_list );
      pnote->to_list = str_dup( to_new + 1 , pnote->to_list);
      return;
    }
  }
  /* nuke the whole note */

  switch (pnote->type)
  {
    default:
      return;
    case NOTE_NOTE:
      list = &note_list;
      break;
    case NOTE_IDEA:
      list = &idea_list;
      break;
    case NOTE_PENALTY:
      list = &penalty_list;
      break;
    case NOTE_NEWS:
      list = &news_list;
      break;
    case NOTE_CHANGES:
      list = &changes_list;
      break;
    case NOTE_RULES:
      list = &rules_list;
      break;
    case NOTE_RPNOTE:
      list = &rpnote_list;
      break;
  }

  /*
   * Remove note from linked list.
   */
  if ( pnote == *list )
  {
    *list = pnote->next;
  }
  else
  {
    for ( prev = *list; prev; prev = prev->next )
    {
      if ( prev->next == pnote )
        break;
    }

    if ( prev == NULL )
    {
      bug( "Note_remove: pnote not found.", 0 );
      return;
    }

    prev->next = pnote->next;
  }

  save_notes(pnote->type);
  free_note(pnote);
  return;
}

bool hide_note (CHAR_DATA *ch, NOTE_DATA *pnote, bool ignore_time )
{
  time_t last_read;

  if (IS_NPC(ch))
    return TRUE;

  switch (pnote->type)
  {
    default:
      return TRUE;
    case NOTE_NOTE:
      last_read = ch->pcdata->last_note;
      break;
    case NOTE_IDEA:
      last_read = ch->pcdata->last_idea;
      break;
    case NOTE_PENALTY:
      last_read = ch->pcdata->last_penalty;
      break;
    case NOTE_NEWS:
      last_read = ch->pcdata->last_news;
      break;
    case NOTE_CHANGES:
      last_read = ch->pcdata->last_changes;
      break;
    case NOTE_RULES:
      last_read = ch->pcdata->last_rules;
      break;
    case NOTE_RPNOTE:
      last_read = ch->pcdata->last_rpnote;
      break;
  }

  if ( !ignore_time )
  {
    if (pnote->date_stamp <= last_read)
      return TRUE;
  }

  if (!str_cmp(ch->name,pnote->sender))
    return TRUE;

  if (!is_note_to(ch,pnote))
    return TRUE;

  return FALSE;
}

bool hide_personal_note (CHAR_DATA *ch, NOTE_DATA *pnote)
{
  time_t last_read;

  if (IS_NPC(ch))
    return TRUE;

  switch (pnote->type)
  {
    default:
      return TRUE;
    case NOTE_NOTE:
      last_read = ch->pcdata->last_note;
      break;
    case NOTE_IDEA:
      last_read = ch->pcdata->last_idea;
      break;
    case NOTE_PENALTY:
      last_read = ch->pcdata->last_penalty;
      break;
    case NOTE_NEWS:
      last_read = ch->pcdata->last_news;
      break;
    case NOTE_CHANGES:
      last_read = ch->pcdata->last_changes;
      break;
    case NOTE_RULES:
      last_read = ch->pcdata->last_rules;
      break;
    case NOTE_RPNOTE:
      last_read = ch->pcdata->last_rpnote;
      break;
  }

  if (pnote->date_stamp <= last_read)
    return TRUE;

  if (!str_cmp(ch->name,pnote->sender))
    return TRUE;

  /*  if (!is_note_to(ch,pnote))
      return TRUE; */
  if ( !is_exact_name( ch->name, pnote->to_list ) )
    return TRUE;

  return FALSE;
}

/*
 * Go through the to_list and don't copy any names that match strip_name.
 */
void strip_to_list( char *to_list, const char *strip_name )
{
  char buf[MAX_STRING_LENGTH];
  char name[MAX_INPUT_LENGTH];
  char *list;

  buf[0] = '\0';

  for ( list = to_list; ; )
  {
    list = first_arg( list, name, FALSE);

    if ( name[0] == '\0' )
      break;

    if ( !str_cmp( strip_name, name ) )
      continue;

    if ( buf[0] != '\0' )
      strcat( buf, " " );
    strcat( buf, name );
  }

  strcpy( to_list, buf );
}

bool hide_clan_note (CHAR_DATA *ch, NOTE_DATA *pnote)
{
  time_t last_read;

  if (IS_NPC(ch))
    return TRUE;

  if (!ch->clan)
    return TRUE;

  switch (pnote->type)
  {
    default:
      return TRUE;
    case NOTE_NOTE:
      last_read = ch->pcdata->last_note;
      break;
    case NOTE_IDEA:
      last_read = ch->pcdata->last_idea;
      break;
    case NOTE_PENALTY:
      last_read = ch->pcdata->last_penalty;
      break;
    case NOTE_NEWS:
      last_read = ch->pcdata->last_news;
      break;
    case NOTE_CHANGES:
      last_read = ch->pcdata->last_changes;
      break;
    case NOTE_RULES:
      last_read = ch->pcdata->last_rules;
      break;
    case NOTE_RPNOTE:
      last_read = ch->pcdata->last_rpnote;
      break;
  }

  if (pnote->date_stamp <= last_read)
    return TRUE;

  if (!str_cmp(ch->name,pnote->sender))
    return TRUE;


  //if (!is_exact_name(clan_table[ch->clan].name,pnote->to_list))
  if (!is_exact_name( ch->clan->name, pnote->to_list ) )
    return TRUE;

  return FALSE;
}

bool hide_both_note (CHAR_DATA *ch, NOTE_DATA *pnote)
{
  time_t last_read;

  if (IS_NPC(ch))
    return TRUE;

  switch (pnote->type)
  {
    default:
      return TRUE;
    case NOTE_NOTE:
      last_read = ch->pcdata->last_note;
      break;
    case NOTE_IDEA:
      last_read = ch->pcdata->last_idea;
      break;
    case NOTE_PENALTY:
      last_read = ch->pcdata->last_penalty;
      break;
    case NOTE_NEWS:
      last_read = ch->pcdata->last_news;
      break;
    case NOTE_CHANGES:
      last_read = ch->pcdata->last_changes;
      break;
    case NOTE_RULES:
      last_read = ch->pcdata->last_rules;
      break;
    case NOTE_RPNOTE:
      last_read = ch->pcdata->last_rpnote;
      break;
  }

  if (pnote->date_stamp <= last_read)
    return TRUE;

  if (!str_cmp(ch->name,pnote->sender))
    return TRUE;

  if (!is_exact_name(ch->name, pnote->to_list )
      &&  !is_exact_name( ch->clan->name, pnote->to_list ) )
    //&&  !is_exact_name(clan_table[ch->clan].name, pnote->to_list ) )
    return TRUE;

  return FALSE;
}

void update_read(CHAR_DATA *ch, NOTE_DATA *pnote)
{
  time_t stamp;

  if (IS_NPC(ch))
    return;

  stamp = pnote->date_stamp;

  switch (pnote->type)
  {
    default:
      return;
    case NOTE_NOTE:
      ch->pcdata->last_note = UMAX(ch->pcdata->last_note,stamp);
      break;
    case NOTE_IDEA:
      ch->pcdata->last_idea = UMAX(ch->pcdata->last_idea,stamp);
      break;
    case NOTE_PENALTY:
      ch->pcdata->last_penalty = UMAX(ch->pcdata->last_penalty,stamp);
      break;
    case NOTE_NEWS:
      ch->pcdata->last_news = UMAX(ch->pcdata->last_news,stamp);
      break;
    case NOTE_CHANGES:
      ch->pcdata->last_changes = UMAX(ch->pcdata->last_changes,stamp);
      break;
    case NOTE_RULES:
      ch->pcdata->last_rules = UMAX(ch->pcdata->last_rules,stamp);
      break;
    case NOTE_RPNOTE:
      ch->pcdata->last_rpnote = UMAX(ch->pcdata->last_rpnote,stamp);
  }
}

void parse_note( CHAR_DATA *ch, char *argument, int64 type )
{
  BUFFER *buffer;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  NOTE_DATA *pnote;
  NOTE_DATA **list;
  char *list_name;
  int vnum;
  int anum;

  if ( IS_NPC(ch) )
    return;

  if ( IS_SET( ch->pen_flags, PEN_NOTE ) )
  {
    send_to_char("Your note privileges have been revoked by the gods.\n\r",ch);
    return;
  }
  switch (type)
  {
    default:
      return;
    case NOTE_NOTE:
      list = &note_list;
      break;
    case NOTE_IDEA:
      list = &idea_list;
      break;
    case NOTE_PENALTY:
      list = &penalty_list;
      break;
    case NOTE_NEWS:
      list = &news_list;
      break;
    case NOTE_CHANGES:
      list = &changes_list;
      break;
    case NOTE_RULES:
      list = &rules_list;
      break;
    case NOTE_RPNOTE:
      list = &rpnote_list;
      break;
  }
  list_name = flag_string(spool_flags, type);

  argument = one_argument( argument, arg );
  smash_tilde( argument );

  if ( arg[0] == '\0' || !str_prefix( arg, "read" ) )
  {
    bool fAll;

    if ( !str_cmp( argument, "all" ) )
    {
      fAll = TRUE;
      anum = 0;
    }

    else if ( argument[0] == '\0' || !str_prefix(argument, "next"))
      /* read next unread note */
    {
      vnum = 0;
      for ( pnote = *list; pnote; pnote = pnote->next)
      {
        if (!hide_note(ch,pnote,FALSE))
        {
          printf_to_char(ch, "\n\r{W[{Y%3d{W] {C%s{x: {B%s{x\n\r{c%s\n\r{WTo{x: {Y%s{x\n\r",
                         vnum,
                         pnote->sender,
                         pnote->subject,
                         pnote->date,
                         pnote->to_list);
          page_to_char( pnote->text, ch );
          update_read(ch,pnote);
          return;
        }
        else if (is_note_to(ch,pnote))
          vnum++;
      }
      if (!str_cmp(list_name, "penalty"))
        send_to_char("You have no unread penalties.\n\r", ch);
      else if (!str_cmp(list_name, "news"))
        send_to_char("You have no unread news.\n\r", ch);
      else if (!str_cmp(list_name, "rules"))
        send_to_char("You have no unread rules.\n\r", ch);
      else
        printf_to_char(ch,"You have no unread %ss.\n\r",list_name);
      return;
    }

    else if ( is_number( argument ) )
    {
      fAll = FALSE;
      anum = atoi( argument );
    }
    else
    {
      send_to_char( "Read which number?\n\r", ch );
      return;
    }

    vnum = 0;
    for ( pnote = *list; pnote; pnote = pnote->next )
    {
      if ( is_note_to( ch, pnote ) && ( vnum++ == anum || fAll ) )
      {
        printf_to_char(ch, "\n\r{W[{Y%3d{W] {C%s{x: {B%s{x\n\r{c%s\n\r{WTo{x: {Y%s{x\n\r",
                       vnum - 1,
                       pnote->sender,
                       pnote->subject,
                       pnote->date,
                       pnote->to_list
                      );
        page_to_char( pnote->text, ch );
        update_read(ch,pnote);
        return;
      }
    }

    if ( (!str_cmp(list_name, "note")) || (!str_cmp(list_name, "change")) || (!str_cmp(list_name, "rpnote")) || (!str_cmp(list_name, "idea")))
    {
      printf_to_char(ch,"There aren't that many %ss.\n\r",list_name);
    }
    else if (!str_cmp(list_name, "penalty"))
    {
      printf_to_char(ch,"There aren't that many penalties.\n\r");
    }
    else
    {
      printf_to_char(ch,"There aren't that many %s.\n\r",list_name);
    }
    return;
  }

  if ( !str_prefix( arg, "list" ) )
  {
    bool fAll = FALSE;
    bool found = FALSE;
    bool subsearch = FALSE;  /*Added by Sartan*/
    bool newsearch = FALSE;

    if ( argument[0] == '\0' || !str_cmp( argument, "all" ) )
      fAll = TRUE;

    if ( is_number( argument ) )
    {
      anum = atoi( argument );
      fAll = TRUE;
    }
    else
      anum = 0;

    if (!str_prefix("subject", argument))
    {
      /* Subject search added by Sartan  Apr. 20, 2001 */
      subsearch = TRUE;
      argument = one_argument(argument, arg);
    }
    else if (!str_prefix("new", argument))
    {
      /* Subject search added by Sartan  Apr. 20, 2001 */
      newsearch = TRUE;
    }
    buffer = new_buf();

    vnum = 0;
    for ( pnote = *list; pnote; pnote = pnote->next )
    {
      if ( is_note_to( ch, pnote ) )
      {
        if (subsearch)
        {
          if ( ( fAll || !str_infix( argument, pnote->subject ) ) && anum <= vnum )
          {
            found = TRUE;
            add_buf( buffer,
                     note_header_str( ch, pnote, vnum, TRUE ) );
          }
        }
        else if (newsearch)
        {
          if (!hide_note(ch, pnote, FALSE) )
          {
            found = TRUE;
            add_buf( buffer,
                     note_header_str( ch, pnote, vnum, TRUE ) );
          }

        }
        else
        {
          if ( ( fAll || is_note_from( ch, pnote, argument ) )
               &&   anum <= vnum )
          {
            found = TRUE;
            add_buf( buffer,
                     note_header_str( ch, pnote, vnum, TRUE ) );
          }
        }
        vnum++;
      }
    }

    if ( found )
      page_to_char( buf_string( buffer ), ch );
    else
    {
      if ( fAll )
      {
        if (!str_cmp(list_name, "penalty"))
          send_to_char("There are no penalties for you.\n\r", ch);
        else if (!str_cmp(list_name, "rules"))
          send_to_char("There are no rules for you.\n\r", ch);
        else
          printf_to_char( ch, "There are no %ss for you.\n\r",
                          list_name );
      }
      else
      {
        if (newsearch)
        {
          if (!str_cmp(list_name, "penalty"))
            send_to_char("No new penalties found.\n\r", ch);
          else
            printf_to_char( ch, "No new %ss found.\n\r",
                            list_name);
        }
        if (subsearch)
          printf_to_char( ch, "No %s found in reference to %s.\n\r",
                          list_name, argument );

        if (!subsearch && !newsearch)
          printf_to_char( ch, "No %s found from %s.\n\r",
                          list_name, capitalize( argument ) );
      }
    }
    free_buf( buffer );
    return;
  }
  if ( (ch->level < 2)
       &&  (TOTAL_PLAY_TIME(ch) /3600) < 5)
  {
    send_to_char("You are not allowed notes yet.\n\r",ch);
    return;
  }


  if ( !str_prefix( arg, "forward" ) && get_trust( ch ) >= ANGEL )
  {
    if ( ch->pnote )
    {
      printf_to_char( ch,
                      "Clear or post the %s you are working on first.\n\r",
                      list_name );
      return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
      printf_to_char( ch, "Forward what %s number?\n\r", list_name );
      return;
    }

    if ( !is_number( arg ) )
    {
      printf_to_char( ch, "Must specify a %s number to forward.\n\r",
                      list_name );
      return;
    }

    anum = atoi( arg );
    vnum = 0;
    for ( pnote = *list; pnote; pnote = pnote->next )
      if ( is_note_to( ch, pnote ) && vnum++ == anum )
        break;

    if ( pnote == NULL )
    {
      printf_to_char( ch, "There is no %s of that number.\n\r",
                      list_name );
      return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] != '\0' )
    {
      if ( ( type = flag_value( spools_flags, arg ) ) == NO_FLAG )
      {
        send_to_char(
          "That's not a valid message board to forward to.\n\r",
          ch );
        return;
      }

      if ( ( type == NOTE_NEWS && !IS_TRUSTED( ch, ANGEL ) )
           ||   ( type == NOTE_CHANGES && !IS_TRUSTED( ch, CREATOR ) )
           ||   ( type == NOTE_RULES && !IS_TRUSTED( ch, CREATOR ) ) )
      {
        printf_to_char( ch,
                        "You aren't high enough level to forward to %s.",
                        flag_string( spools_flags, type ) );
        return;
      }
    }

    note_attach( ch, type );

    /* Fix subject. */
    if ( str_prefix( "fwd:", pnote->subject ) )
      mprintf(sizeof(buf),buf, "Fwd: %s", pnote->subject );
    else
      strcpy( buf, pnote->subject );
    free_string( ch->pnote->subject );
    ch->pnote->subject = str_dup( buf ,ch->pnote->subject);

    /* Copy text. */
    mprintf(sizeof(buf),buf, "\n\rOriginal message from %s:\n\r\n\r%s", pnote->sender, pnote->text);
    free_string( ch->pnote->text );
    ch->pnote->text = str_dup( buf, ch->pnote->text );

    /* Copy originals. */
    free_string( ch->pnote->sender );
    ch->pnote->sender  = str_dup( ch->name ,ch->pnote->sender);
    free_string( ch->pnote->to_list );
    ch->pnote->to_list  = str_dup( pnote->to_list ,ch->pnote->to_list);

    if ( pnote->type == type )
      buf[0] = '\0';
    else
      mprintf(sizeof(buf),buf, " to %s", flag_string( spools_flags, type ) );

    printf_to_char( ch,
                    "Forwarding %s's %s [%d]%s:\n\r"
                    "Subject: %s\n\r"
                    "Original To: %s\n\r\n\r",
                    pnote->sender,
                    list_name,
                    anum,
                    buf,
                    ch->pnote->subject,
                    ch->pnote->to_list );

    return;
  }

  if ( !str_prefix( arg, "remove" ) )
  {
    if ( !is_number( argument ) )
    {
      send_to_char( "Note remove which number?\n\r", ch );
      return;
    }

    anum = atoi( argument );
    vnum = 0;
    for ( pnote = *list; pnote; pnote = pnote->next )
    {
      if ( is_note_to( ch, pnote ) && vnum++ == anum )
      {
        note_remove( ch, pnote, FALSE );
        send_to_char( "Ok.\n\r", ch );
        return;
      }
    }

    printf_to_char(ch,"There aren't that many %s.",list_name);
    return;
  }

  if ( !str_prefix( arg, "delete" ) && get_trust(ch) >= MAX_LEVEL - 1)
  {
    if ( !is_number( argument ) )
    {
      send_to_char( "Note delete which number?\n\r", ch );
      return;
    }

    anum = atoi( argument );
    vnum = 0;
    for ( pnote = *list; pnote; pnote = pnote->next )
    {
      if ( is_note_to( ch, pnote ) && vnum++ == anum )
      {
        logit("Note %d: To: %s, From: %s, Subj: %s del by %s.",anum,
              pnote->to_list, pnote->sender, pnote->subject, ch->name);
        note_remove( ch, pnote,TRUE );
        send_to_char( "Ok.\n\r", ch );
        return;
      }
    }

    printf_to_char(ch,"There aren't that many %s.",list_name);
    return;
  }

  if (!str_prefix(arg,"catchup"))
  {
    switch (type)
    {
      case NOTE_NOTE:
        ch->pcdata->last_note = current_time;
        strcpy(buf,"notes");
        break;
      case NOTE_IDEA:
        ch->pcdata->last_idea = current_time;
        strcpy(buf,"ideas");
        break;
      case NOTE_PENALTY:
        ch->pcdata->last_penalty = current_time;
        strcpy(buf,"penalties");
        break;
      case NOTE_NEWS:
        ch->pcdata->last_news = current_time;
        strcpy(buf,"news");
        break;
      case NOTE_CHANGES:
        ch->pcdata->last_changes = current_time;
        strcpy(buf,"changes");
        break;
      case NOTE_RULES:
        ch->pcdata->last_rules = current_time;
        strcpy(buf,"rules");
        break;
      case NOTE_RPNOTE:
        ch->pcdata->last_rpnote = current_time;
        strcpy(buf,"rpnotes");
        break;
    }
    printf_to_char(ch,"Your %s are now up to date.\n\r",buf);
    return;
  }

  if (!str_prefix(arg,"set"))
  {
    if ( !is_number( argument ) )
    {
      send_to_char( "Note set your notes to which number?\n\r", ch );
      return;
    }

    anum = atoi( argument );
    if (anum == 0)
    {
      switch (type)
      {
        case NOTE_NOTE:
          ch->pcdata->last_note = 0;
          break;
        case NOTE_IDEA:
          ch->pcdata->last_idea = 0;
          break;
        case NOTE_PENALTY:
          ch->pcdata->last_penalty =0;
          break;
        case NOTE_NEWS:
          ch->pcdata->last_news = 0;
          break;
        case NOTE_CHANGES:
          ch->pcdata->last_changes = 0;
          break;
        case NOTE_RULES:
          ch->pcdata->last_rules = 0;
          break;
        case NOTE_RPNOTE:
          ch->pcdata->last_rpnote = 0;
          break;
      }
      printf_to_char(ch,"Your %s are now set.\n\r",list_name);
      return;
    }
    vnum = 0;
    for ( pnote = *list; pnote; pnote = pnote->next )
    {
      if ( is_note_to( ch, pnote ) && ( vnum++ == anum ) )
      {
        switch (type)
        {
          case NOTE_NOTE:
            ch->pcdata->last_note = pnote->date_stamp;
            break;
          case NOTE_IDEA:
            ch->pcdata->last_idea = pnote->date_stamp;
            break;
          case NOTE_PENALTY:
            ch->pcdata->last_penalty =pnote->date_stamp;
            break;
          case NOTE_NEWS:
            ch->pcdata->last_news = pnote->date_stamp;
            break;
          case NOTE_CHANGES:
            ch->pcdata->last_changes = pnote->date_stamp;
            break;
          case NOTE_RULES:
            ch->pcdata->last_rules = pnote->date_stamp;
            break;
          case NOTE_RPNOTE:
            ch->pcdata->last_rpnote = pnote->date_stamp;
            break;
        }
        printf_to_char(ch,"Your %s are now set.\n\r",list_name);
        return;
      }
    }
    printf_to_char(ch,"That %s number is not found.\n\r",list_name);
  }

  if (!str_prefix(arg,"personal"))
  {
    vnum = 0;
    for ( pnote = *list; pnote; pnote = pnote->next)
    {
      if (!hide_personal_note(ch,pnote))
      {
        printf_to_char(ch, "[%3d] %s{x: %s{x\n\r%s\n\rTo: %s{x\n\r",
                       vnum,
                       pnote->sender,
                       pnote->subject,
                       pnote->date,
                       pnote->to_list);
        page_to_char( pnote->text, ch );
        update_read(ch,pnote);
        return;
      }
      else if (is_note_to(ch,pnote))
        vnum++;
    }
    printf_to_char(ch,"You have no unread personal %s.\n\r",list_name);
    return;
  }
  if (!str_prefix(arg,"clan"))
  {
    vnum = 0;
    for ( pnote = *list; pnote; pnote = pnote->next)
    {
      if (!hide_clan_note(ch,pnote))
      {
        printf_to_char(ch, "[%3d] %s{x: %s{x\n\r%s\n\rTo: %s{x\n\r",
                       vnum,
                       pnote->sender,
                       pnote->subject,
                       pnote->date,
                       pnote->to_list);
        page_to_char( pnote->text, ch );
        update_read(ch,pnote);
        return;
      }
      else if (is_note_to(ch,pnote))
        vnum++;
    }
    printf_to_char(ch,"You have no unread clan %s.\n\r",list_name);
    return;
  }
  if (!str_prefix(arg,"both"))
  {
    vnum = 0;
    for ( pnote = *list; pnote; pnote = pnote->next)
    {
      if (!hide_both_note(ch,pnote))
      {
        printf_to_char(ch, "[%3d] %s{x: %s{x\n\r%s\n\rTo: %s{x\n\r",
                       vnum,
                       pnote->sender,
                       pnote->subject,
                       pnote->date,
                       pnote->to_list);
        page_to_char( pnote->text, ch );
        update_read(ch,pnote);
        return;
      }
      else if (is_note_to(ch,pnote))
        vnum++;
    }
    printf_to_char(ch,"You have no unread clan or personal %s.\n\r",list_name);
    return;
  }

  /* below this point only certain people can edit notes */
  if ((type == NOTE_NEWS && !IS_TRUSTED(ch,ANGEL))
      ||  (type == NOTE_CHANGES && !IS_TRUSTED(ch,CREATOR))
      ||  (type == NOTE_RULES && !IS_TRUSTED(ch,CREATOR)))
  {
    printf_to_char(ch,"You aren't high enough level to write %s.",list_name);
    return;
  }

  if ( !str_cmp( arg, "+" ) )
  {
    note_attach( ch,type );
    if (ch->pnote->type != type)
    {
      send_to_char(
        "You already have a different note in progress.\n\r",ch);
      return;
    }

    if (strlen(ch->pnote->text)+strlen(argument) > 4000)
    {
      send_to_char( "Note too long.\n\r", ch );
      return;
    }

    buffer = new_buf();

    add_buf(buffer,ch->pnote->text);
    add_buf(buffer,argument);
    add_buf(buffer,"\n\r");
    free_string( ch->pnote->text );
    ch->pnote->text = str_dup( buf_string(buffer),ch->pnote->text );
    free_buf(buffer);
    send_to_char( "Ok.\n\r", ch );
    return;
  }

  if (!str_cmp(arg,"-"))
  {
    int len;
    bool found = FALSE;

    note_attach(ch,type);
    if (ch->pnote->type != type)
    {
      send_to_char(
        "You already have a different note in progress.\n\r",ch);
      return;
    }

    if (ch->pnote->text == NULL || ch->pnote->text[0] == '\0')
    {
      send_to_char("No lines left to remove.\n\r",ch);
      return;
    }

    strcpy(buf,ch->pnote->text);

    for (len = strlen(buf); len > 0; len--)
    {
      if (buf[len] == '\r')
      {
        if (!found)  /* back it up */
        {
          if (len > 0)
            len--;
          found = TRUE;
        }
        else /* found the second one */
        {
          buf[len + 1] = '\0';
          free_string(ch->pnote->text);
          ch->pnote->text = str_dup(buf, ch->pnote->text);
          return;
        }
      }
    }
    buf[0] = '\0';
    free_string(ch->pnote->text);
    ch->pnote->text = str_dup(buf, ch->pnote->text);
    return;
  }
  if ( !str_cmp( arg, "desc" ) || !str_cmp( arg, "write") )
  {
    note_attach( ch,type );
    if (ch->pnote->type != type)
    {
      send_to_char(
        "You already have a different note in progress.\n\r",ch);
      return;
    }

    if ( argument[0] == '\0' )
    {
      string_append( ch, &ch->pnote->text, APPEND_IGNORE, NULL );
    }
    return;
  }

  if ( !str_prefix( arg, "reply" ) )
  {
    char *temp;
    if ( ch->pnote )
    {
      printf_to_char( ch,  "Clear or post the (%s) note you are working on first.\n\r",  list_name );
      return;
    }

    if ( argument[0] == '\0' )
    {
      printf_to_char( ch, "Reply to what %s number?\n\r", list_name );
      return;
    }

    if ( !is_number( argument ) )
    {
      printf_to_char( ch, "Must specify a %s number to reply to.\n\r",
                      list_name );
      return;
    }

    anum = atoi( argument );
    vnum = 0;
    for ( pnote = *list; pnote; pnote = pnote->next )
      if ( is_note_to( ch, pnote ) && vnum++ == anum )
        break;

    if ( pnote == NULL )
    {
      printf_to_char( ch, "There is no %s of that number.\n\r",list_name );
      return;
    }

    note_attach( ch, type );

    /* Fix subject. */
    if ( str_prefix( "re:", pnote->subject ) )
      mprintf(sizeof(buf), buf, "Re: %s", pnote->subject );
    else
      strcpy( buf, pnote->subject );

    free_string( ch->pnote->subject );
    ch->pnote->subject = str_dup( buf , ch->pnote->subject);

    /* Fix the recipient. */
    if ( strlen( pnote->to_list ) + strlen( pnote->sender )   < MAX_INPUT_LENGTH - 1 )
    {
      strcpy( buf, pnote->to_list );

      if ( !is_exact_name( "all", pnote->to_list )
           &&   !is_exact_name( pnote->sender, pnote->to_list ))
      {
        strcat( buf, " " );
        strcat( buf, pnote->sender );
      }

      strip_to_list( buf, ch->name );

      free_string( ch->pnote->to_list );
      ch->pnote->to_list = str_dup( buf, ch->pnote->to_list );
    }
    else
    {
      free_string( ch->pnote->to_list );
      ch->pnote->to_list = str_dup( pnote->to_list , ch->pnote->to_list);
    }
    //      logit("BEFORE");
    free_string(ch->pnote->text);
    temp = NULL;
    temp = str_dup(pnote->text, temp);

    ch->pnote->text =  str_dup(reply_format(temp),
                               ch->pnote->text) ;
    free_string(temp);
    //      logit("BEFORE 2");
    mprintf(sizeof(buf),buf, "%s{x\n\r\n\r",ch->pnote->text);
    free_string(ch->pnote->text);
    ch->pnote->text = str_dup(buf,ch->pnote->text);
    //      logit("AFTER");
    printf_to_char( ch,
                    "{YReplying to {x%s's {c%s{x {W[{G%d{W]{x:\n\r"
                    "{WSubject: %s{x\n\rTo: %s{x\n\r",
                    pnote->sender,
                    list_name,
                    anum,
                    ch->pnote->subject,
                    ch->pnote->to_list );
    SET_BIT(ch->active_flags,ACTIVE_REPLY);
    ch->pnote->reply = TRUE;

    switch (ch->pnote->type)
    {
      default:
        return;
      case NOTE_NOTE:
        do_function(ch, &do_note, "write");
        break;
      case NOTE_IDEA:
        do_function(ch, &do_idea, "write");
        break;
      case NOTE_PENALTY:
        do_function(ch, &do_penalty, "write");
        break;
      case NOTE_NEWS:
        do_function(ch, &do_news, "write");
        break;
      case NOTE_CHANGES:
        do_function(ch, &do_changes, "write");
        break;
      case NOTE_RULES:
        do_function(ch, &do_rules, "write");
        break;
      case NOTE_RPNOTE:
        do_function(ch, &do_rpnote, "write");
        break;
    }
    return;
  }

  if ( !str_prefix( arg, "subject" ) )
  {
    note_attach( ch,type );
    if (ch->pnote->type != type)
    {
      send_to_char(
        "You already have a different note in progress.\n\r",ch);
      return;
    }
    strcat(argument,"{x");
    free_string( ch->pnote->subject );
    ch->pnote->subject = str_dup( argument ,ch->pnote->subject);
    send_to_char( "Ok.\n\r", ch );
    return;
  }

  if ( !str_prefix( arg, "to" ) )
  {
    note_attach( ch,type );
    if (ch->pnote->type != type)
    {
      printf_to_char( ch, "You already have a different %s in progress.\n\r",
                      list_name );
      //send_to_char(
      //       "You already have a different note in progress.\n\r",ch);
      return;
    }
    free_string( ch->pnote->to_list );
    ch->pnote->to_list = str_dup( argument , ch->pnote->to_list);
    send_to_char( "Ok.\n\r", ch );
    return;
  }

  if ( !str_prefix( arg, "clear" ) )
  {
    if ( ch->pnote )
    {
      free_note(ch->pnote);
      REMOVE_BIT(ch->active_flags, ACTIVE_REPLY);
      ch->pnote = NULL;
    }

    send_to_char( "Ok.\n\r", ch );
    return;
  }

  if ( !str_prefix( arg, "show" ) )
  {
    if ( ch->pnote == NULL )
    {
      printf_to_char( ch, "You have no %s in progress.\n\r",
                      list_name );
      //send_to_char( "You have no note in progress.\n\r", ch );
      return;
    }

    if (ch->pnote->type != type)
    {
      send_to_char("You aren't working on that kind of note.\n\r",ch);
      return;
    }

    printf_to_char(ch, "{WSubject: {C%s{x\n\r{WTo: {Y%s{x\n\r",
                   ch->pnote->subject,
                   ch->pnote->to_list
                  );
    page_to_char( ch->pnote->text, ch );
    return;
  }

  if ( !str_prefix( arg, "post" ) || !str_prefix( arg, "send" ) )
  {
    char *strtime;

    if ( ch->pnote == NULL )
    {
      send_to_char( "You have no note in progress.\n\r", ch );
      return;
    }

    if (ch->pnote->type != type)
    {
      send_to_char( "You aren't working on that kind of note.\n\r", ch );
      return;
    }

    if ( !str_cmp( ch->pnote->to_list, "" ) )
    {
      send_to_char(
        "You need to provide a recipient (all, immortal, or name).\n\r", ch );
      return;
    }

    if ( !str_cmp( ch->pnote->subject,"" ) )
    {
      send_to_char( "You need to provide a subject.\n\r", ch );
      return;
    }

    argument = one_argument( argument, arg );
    if ( !strcmp( arg, "as" ) )
    {
      if ( !IS_TRUSTED( ch, CREATOR - 3 ) )
      {
        send_to_char(
          "Your level is not high enough to post as alias.\n\r", ch );
        return;
      }
      if ( !IS_NULLSTR( argument ) )
      {
        free_string( ch->pnote->sender );
        ch->pnote->sender = str_dup( argument, ch->name );
      }
    }

    ch->pnote->next          = NULL;
    strtime                = ctime( &current_time );
    strtime[strlen(strtime)-1]  = '\0';
    ch->pnote->date          = str_dup( strtime,ch->pnote->date );
    ch->pnote->date_stamp    = current_time;
    strcpy(buf,ch->pnote->text);
    free_string(ch->pnote->text);
    strcat(buf,"{x");
    ch->pnote->text = str_dup(buf,ch->pnote->text);

    append_note(ch->pnote);
    printf_to_char( ch, "You have successfully posted your %s to %s.\n\r",
                    list_name, ch->pnote->to_list );
    for ( d = descriptor_list; d; d = d->next )
    {
      if ( d->connected == CON_PLAYING
           &&  IS_SET( d->character->comm_flags, COMM_NEWSFAERIE )
           &&  is_note_to( d->character, ch->pnote )
           &&  d->character != ch )
      {
        if ( type == NOTE_PENALTY && !IS_IMMORTAL( d->character ) )
          continue;
        else
          printf_to_char( d->character,
                          "{WThe NewsFaerie tells you:  %s {Whas posted to %s.{x\n\r",
                          ch->pnote->sender, list_name );
      }
    }
    ch->pnote = NULL;
    return;
  }
  send_to_char( "You can't do that.\n\r", ch );
  return;
}

/*Stock UNREAD -- Re-added 9/3/11 Aarchane */
void do_unread(CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
    return;

  send_to_char("{Y_  _ ____ ____ ____ ____ ____ ____    ___  ____ ____ ____ ___  \n\r", ch);
  send_to_char("|\\/| |___ [__  [__  |__| | __ |___    |__] |  | |__| |__/ |  \\ \n\r", ch);
  send_to_char("|  | |___ ___] ___] |  | |__] |___    |__] |__| |  | |  \\ |__/ {x\n\r", ch);
  send_to_char("{y|                                                            |{x\n\r", ch);
  printf_to_char(ch, "{y|{x   Notes{x..... : %s%3d{x/{c%-3d{x             News{x...... : %s%3d{x/{c%-3d    {y|{x\n\r",
                 count_spool(ch, note_list) > 0 ? "{C" : "{c",
                 count_spool(ch, note_list),
                 count_full_spool(ch, note_list),
                 count_spool(ch, news_list) > 0 ? "{C" : "{c",
                 count_spool(ch, news_list),
                 count_full_spool(ch, news_list) );
  printf_to_char(ch, "{y|{x   Ideas{x..... : %s%3d{x/{c%-3d{x             Changes{x... : %s%3d{x/{c%-3d    {y|{x\n\r",
                 count_spool(ch, idea_list) > 0 ? "{C" : "{c",
                 count_spool(ch, idea_list),
                 count_full_spool(ch, idea_list),
                 count_spool(ch, changes_list) > 0 ? "{C" : "{c",
                 count_spool(ch, changes_list),
                 count_full_spool(ch, changes_list) );
  printf_to_char(ch, "{y|{x   RP Notes{x.. : %s%3d{x/{c%-3d{x             Rules{x..... : %s%3d{x/{c%-3d    {y|{x\n\r",
                 count_spool(ch, rpnote_list) > 0 ? "{C" : "{c",
                 count_spool(ch, rpnote_list),
                 count_full_spool(ch, rpnote_list),
                 count_spool(ch, rules_list) > 0 ? "{C" : "{c",
                 count_spool(ch, rules_list),
                 count_full_spool(ch, rules_list)  );
  send_to_char("{y|                                                            |{x\n\r", ch );
  printf_to_char(ch, "{y\\------------------------------------------------------------/{x\n\r" );

  /*  if (!IS_TRUSTED(ch,ANGEL) && (count =
                                forward_spool( ch, penalty_list, note_list )) > 0)
    {
      found = TRUE;

      printf_to_char(ch,"%d %s been forwarded to your note spool.\n\r",
              count, count > 1 ? "penalties have" : "penalty has");
    }
  */
}

/*** New unread note display support functions
     Jan 2001 CBJ & Ainya ***/
int split_count_spool(CHAR_DATA *ch, NOTE_DATA *spool, sh_int *immcount, sh_int *clancount, sh_int *perscount, sh_int *allcount)
{
  int count = 0;
  NOTE_DATA *pnote;
  *immcount = *clancount = *perscount = *allcount = 0;
  for (pnote = spool; pnote; pnote = pnote->next)
  {
    if (!hide_personal_note(ch,pnote))
    {
      (*perscount)++;
      continue;
    }
    else if (!hide_clan_note(ch,pnote))
    {
      (*clancount)++;
      continue;
    }
    else if (!hide_imm_note(ch,pnote))
    {
      (*immcount)++;
      continue;
    }
    else if (!hide_note(ch,pnote,FALSE))
    {
      count++;
      continue;
    }
    else
      (*allcount)++;
  }
  return count;
}


bool display_unread(CHAR_DATA *ch,char *arg, NOTE_DATA *list)
{
  bool found = FALSE;
  int count =0;
  sh_int immcount=0, clancount=0, perscount=0, allcount=0;

  if ((count = split_count_spool(ch,list,&immcount,&clancount,&perscount,&allcount)) > 0)
  {
    found = TRUE;
    /*** handle split counts!!! ***/
    if (IS_TRUSTED(ch,ANGEL))
      printf_to_char(ch,"%-12s :    {Y%-8d {C%-8d  {W%-8d  {M%-8d{x\n\r", arg,count,clancount,perscount,immcount);
    else
      printf_to_char(ch,"%-12s :    {Y%-8d {C%-8d  {W%-8d{x\n\r", arg,count,clancount,perscount);
  }
  return(found);
}

/*** New check for imm notes
     Jan 2001 CBJ & Ainya ***/
bool hide_imm_note (CHAR_DATA *ch, NOTE_DATA *pnote)
{
  time_t last_read;

  if (IS_NPC(ch))
    return TRUE;
  if (!IS_TRUSTED(ch,ANGEL))
    return TRUE;

  switch (pnote->type)
  {
    default:
      return TRUE;
    case NOTE_NOTE:
      last_read = ch->pcdata->last_note;
      break;
    case NOTE_IDEA:
      last_read = ch->pcdata->last_idea;
      break;
    case NOTE_PENALTY:
      last_read = ch->pcdata->last_penalty;
      break;
    case NOTE_NEWS:
      last_read = ch->pcdata->last_news;
      break;
    case NOTE_CHANGES:
      last_read = ch->pcdata->last_changes;
      break;
    case NOTE_RULES:
      last_read = ch->pcdata->last_rules;
      break;
    case NOTE_RPNOTE:
      last_read = ch->pcdata->last_rpnote;
      break;
  }

  if (pnote->date_stamp <= last_read)
    return TRUE;

  if (!str_cmp(ch->name,pnote->sender))
    return TRUE;

  if (!is_exact_name("immortal",pnote->to_list)
      &&  !is_exact_name("imm",     pnote->to_list)
      &&  !is_exact_name("immortals",pnote->to_list)
      &&  !is_exact_name("imms",     pnote->to_list)
      &&  !is_exact_name("god",     pnote->to_list)
      &&  !is_exact_name("gods",    pnote->to_list)
      &&  !is_exact_name("admin",   pnote->to_list) )
    return TRUE;

  return FALSE;
}
void note_line(char *sender, int note_type, char *to, char *subject,
               char *text)
{
  char *strtime;
  DESCRIPTOR_DATA *d;

  NOTE_DATA *pNoteTo;

  pNoteTo = new_note();
  pNoteTo->next                  = NULL;
  pNoteTo->sender          = str_dup(sender   ,pNoteTo->sender);
  strtime      = ctime( &current_time );
  strtime[strlen(strtime)-1]  = '\0';
  pNoteTo->date                  = str_dup( strtime,pNoteTo->date );
  pNoteTo->date_stamp           = current_time;
  pNoteTo->to_list          = str_dup( to ,pNoteTo->to_list);
  pNoteTo->subject          = str_dup( subject,pNoteTo->subject);
  pNoteTo->text                  = str_dup( text ,pNoteTo->text);
  pNoteTo->type                  = note_type;
  append_note(pNoteTo);

  for (d = descriptor_list; d; d = d->next)
  {
    if (d->connected == CON_PLAYING
        && IS_SET(d->character->comm_flags, COMM_NEWSFAERIE)
        && is_note_to(d->character, pNoteTo))
    {
      printf_to_char(d->character,"{WThe NewsFaerie tells you:  %s {Whas posted to %s.{x\n\r",
                     sender, "notes");
    }
  }
}


#if OLDSTUFF
void do_unread(CHAR_DATA *ch, char *argument)
{
  bool found = FALSE;

  if (IS_NPC(ch))
    return;

  send_to_char("\n\r{C/--- {YMessage Board{C ---\\\n\r",ch);
  if (quicknote_display(ch,"News.....",news_list))
    found = TRUE;
  if (quicknote_display(ch,"Changes...",changes_list))
    found = TRUE;
  if (quicknote_display(ch,"Notes.....",note_list))
    found = TRUE;
  if (quicknote_display(ch,"Ideas.....",idea_list))
    found = TRUE;
  if (quicknote_display(ch,"Rules.....",rules_list))
    found = TRUE;
  if (quicknote_display(ch,"RP Notes..",rpnote_list))
    found = TRUE;
  if (IS_TRUSTED(ch,ANGEL))
    if (quicknote_display(ch,"Penalties",penalty_list))
      found = TRUE;
  /*  if (!IS_TRUSTED(ch,ANGEL) && (count =
        forward_spool( ch, penalty_list, note_list )) > 0)
    {
      found = TRUE;
    
      printf_to_char(ch,"%d %s been forwarded to your note spool.\n\r",
        count, count > 1 ? "penalties have" : "penalty has");
    }
  */
  if (!found)
    send_to_char("{C|-- {WNo New Messages{C --|\n\r",ch);
  send_to_char("{C\\---------------------/{x\n\r",ch);
}
#endif

char *reply_format(char *argument)
{
  static char buf[MSL];
  int x=0,y=0, z=0;
  buf[0]='\0';
  buf[y++] = '>';
  buf[y++] = '>';
  z = strlen(argument);
  //  logit("ARgument is -%s-, Z is %d", argument, z);
  for (x = 0; x < z-3 && y < MSL- 3; x++)
  {
    if (argument[x] =='\0')
    {
      //  logit("Breaking reply_format");
      break;
    }

    if ( argument[x] == '\n' )
    {
      if (argument[x+1] == '\r')
      {
        buf[y++] = '\n';
        buf[y++] = '\r';
        buf[y++] = '>';
        buf[y++] = '>';
        x++;
        //        logit("Got a N R");
      }
      else
      {
        buf[y++] = '\n';
        buf[y++] = '>';
        buf[y++] = '>';
        //      logit("GOT A N");
      }
    }
    else
      buf[y++] = argument[x];
    buf[y]='\0';
    //      logit("-%s-NULLING THE END of BUF", buf);
  }

  //  logit("Return Buf is -%s-", buf);
  return (buf);
}

void do_catchup( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {
    send_to_char("Usage: catchup <argument>\n\r",ch);
    send_to_char("Arguments: all, notes, ideas, penalties, news, changes, rules, rpnotes\n\r",ch);
    return;
  }

  if (!str_prefix( arg, "notes" ) )
  {
    do_function( ch, &do_note, "catchup" );
    return;
  }

  if (!str_prefix( arg, "ideas" ) )
  {
    do_function( ch, &do_idea, "catchup" );
    return;
  }

  if ( (!str_prefix( arg, "penalties" ) )
       || (!str_prefix( arg, "penalties" ) ) )
  {
    do_function( ch, &do_penalty, "catchup" );
    return;
  }

  if (!str_prefix( arg, "news" ) )
  {
    do_function( ch, &do_news, "catchup" );
    return;
  }

  if (!str_prefix( arg, "changes" ) )
  {
    do_function( ch, &do_changes, "catchup" );
    return;
  }

  if (!str_prefix( arg, "rules" ) )
  {
    do_function( ch, &do_rules, "catchup" );
    return;
  }

  if (!str_prefix( arg, "rpnote" ) )
  {
    do_function( ch, &do_rpnote, "catchup" );
    return;
  }

  if (!str_prefix( arg, "all" ) )
  {
    ch->pcdata->last_note = current_time;
    ch->pcdata->last_idea = current_time;
    ch->pcdata->last_penalty = current_time;
    ch->pcdata->last_news = current_time;
    ch->pcdata->last_changes = current_time;
    ch->pcdata->last_rules = current_time;
    ch->pcdata->last_rpnote = current_time;
    send_to_char("All note spools are now up to date.\n\r",ch);
    return;
  }

  send_to_char("Usage: catchup <argument>\n\r",ch);
  send_to_char("Arguments: all, notes, ideas, penalties, news, changes, rules, rpnotes\n\r",ch);

}
