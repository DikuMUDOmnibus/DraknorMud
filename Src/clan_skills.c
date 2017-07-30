/**************************************************************************\
*      The Draknor Codebase(Draknor) is copyright 2003 by Robert Leonard. *
*      Draknor has been created with much time and effort from many       *
*      different people's input and ideas.                                *
*                                                                         *
*      Using this code without the direct permission of its writers is    *
*      not allowed.                                                       *
\**************************************************************************/
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "magic.h"
#include "recycle.h"
#include "interp.h"
#include "olc.h"  // to save scholars books

// Scholars book VNUMs
const int book_sociology    = 25250;
const int book_combatology  = 25251;
const int book_illuminology = 25252;
const int book_theology     = 25254;
const int book_cartography  = 25255;
const int book_history      = 25256;

/*
   Command: author
   Created by: Taeloch
   Purpose: For members of Scholars clan to write books, based on their rank.
   Scholars clan book creation command
   Scholars.are: 25200-25299
*/
void do_author( CHAR_DATA *ch, char *argument )
{
  BUFFER           *buffer;
  OBJ_INDEX_DATA   *pObjIndex;
  EXTRA_DESCR_DATA *pObjEd;
  EXTRA_DESCR_DATA *ped = NULL;
  char arg[MAX_INPUT_LENGTH];
  //char buf[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  int authcommand = 0;
  int vnum = 0;
  int count = 0;
  int canedit;

  if ( IS_NPC(ch)
  ||  !ch->clan
  ||   strcmp(capitalize(ch->clan->name),"Scholars") )
  { // authoring is only for PCs in the clan Scholars
    send_to_char("Huh!?\n\r",ch);
    return;
  }

// first argument is the name of the book we are editing
  argument = one_argument(argument,arg);
  if (arg[0] == '\0')
  {
    send_to_char("Syntax: author [bookname] [command] <keywords>\n\r",ch);
    return;
  }

  if (!str_prefix( arg, "sociology"))
    vnum = book_sociology;
  if (!str_prefix( arg, "combatology"))
    vnum = book_combatology;
  if (!str_prefix( arg, "illuminology"))
    vnum = book_illuminology;
  if (!str_prefix( arg, "theology"))
    vnum = book_theology;
  if (!str_prefix( arg, "cartography"))
    vnum = book_cartography;
  if (!str_prefix( arg, "history"))
    vnum = book_history;
  if (!str_prefix( arg, "save"))
  {
    vnum = book_sociology;
    authcommand = 7; // allow "save" as a first argument
  }

  if ( vnum == 0 )
  {
    send_to_char("Syntax: author [bookname] [command] <keywords>\n\r",ch);
    send_to_char("Valid book names are: sociology, combatology,\n\r",ch);
    send_to_char("illuminology, theology, cartography, history\n\r",ch);
    return;
  }

// second argument is the authoring command
  argument = one_argument(argument,arg);
  if (( arg[0] == '\0')
  && (authcommand != 7) ) // save can be first argument
  {
    send_to_char("Syntax: author [bookname] [command] <keywords>\n\r",ch);
    return;
  }

  if (!str_prefix(arg,"show"))
    authcommand = 1;
  if (!str_prefix(arg,"replace"))
    authcommand = 2;
  if (!str_prefix(arg,"edit"))
    authcommand = 3;
  if (!str_prefix(arg,"remove"))
    authcommand = 4;
  if (!str_prefix(arg,"write"))
    authcommand = 5;
  if (!str_prefix(arg,"list"))
    authcommand = 6;
  if (!str_prefix(arg,"save"))
    authcommand = 7;

  if ( authcommand == 0 )
  {
    send_to_char("Syntax: author [bookname] [command] <keywords>\n\r",ch);
    send_to_char("Valid commands are: show, replace, edit, write, remove, list, save\n\r",ch);
    return;
  }

// here is where we check rank vs. editability.  hope the rank levels don't change...
  switch (ch->clan->rank->level)
  {
    case 16: // Omniscient
    case 15: // Overseer
    case 14: // Keeper
    case 13: // Archivist
    case 12: // Librarian
      canedit = TRUE;
      break;
    case 11: // Historian
      if (vnum == book_history)
        canedit = TRUE;
      break;
    case 10: // Cartographer
      if (vnum == book_cartography)
        canedit = TRUE;
      break;
    case 9:  // Theologian
      if (vnum == book_theology)
        canedit = TRUE;
      break;
    case 8:  // Illuminist
      if (vnum == book_illuminology)
        canedit = TRUE;
      break;
    case 7:  // Combatist
      if (vnum == book_combatology)
        canedit = TRUE;
      break;
    case 6:  // Sociologist
      if (vnum == book_sociology)
        canedit = TRUE;
      break;
    default:
      canedit = FALSE;
      break;
  }

  if (!canedit)
  {
    send_to_char("You do not have sufficient rank to edit this book.\n\r",ch);
    return;
  }

// Awesome!  We now have a book, command, and keyword[s]
  pObjIndex = get_obj_index(vnum);
  if (!pObjIndex)
  { // this should never happen, but we have to account for it, just in case
    send_to_char("The book's object was not found!  Please tell an IMM.\n\r",ch);
    return;
  }

  if (authcommand == 6) // list
  {
    buffer = new_buf();

    bprintf( buffer, "{xThese pages are in %s:\n\r", pObjIndex->short_descr);    
    bstrcat( buffer, "{c+----------------------+----------------------+----------------------+{x\n\r");

    for ( pObjEd = pObjIndex->extra_descr; pObjEd; pObjEd = pObjEd->next )
    {
      count++;
      strncpy_color(buf2, pObjEd->keyword, 20, ' ', TRUE );
      bprintf( buffer, "{c|{x %-20s{x ", buf2 );

      if ((count % 3) == 0)
        bstrcat( buffer, "{c|{x\n\r" );
    }
    if ((count % 3) == 1)
    {
      bprintf( buffer, "{c| %-20s | %-20s |{x\n\r", "", "" );
    }
    if ((count % 3) == 2)
    {
      bprintf( buffer, "{c| %-20s |{x\n\r", "" );
    }

    bstrcat( buffer, "{c+----------------------+----------------------+----------------------+{x\n\r");

    bprintf( buffer, "{x%d {Dpages found{x\n\r", count);

    page_to_char( buf_string(buffer), ch );
    free_buf( buffer );
    return;
  }

  if (authcommand == 7) // save
  {
    save_area( pObjIndex->area );
    send_to_char("Scholars library saved.\n\r",ch);
    return;
  }

  if ( *argument == '\0' )
  {
    send_to_char("Syntax: author [bookname] [command] <keywords>\n\r",ch);
    return;
  }

  pObjEd = ed_lookup( argument, pObjIndex->extra_descr );
  if (authcommand == 5) // 5=write
  {
    if (pObjEd)
    {
      send_to_char("That page already exists in the book.\n\r",ch);
      return;
    }
    pObjEd = new_extra_descr();
    pObjEd->keyword = str_dup( argument ,pObjEd->keyword);
    pObjEd->next = pObjIndex->extra_descr;
    pObjIndex->extra_descr = pObjEd;
  }
  else
  {
    if (!pObjEd)
    {
      send_to_char("That page was not found in the book.\n\r",ch);
      return;
    }
  }

// Supercool!  We have the command, the object and one of its extra_descs to work with
  switch (authcommand)
  {
    case 1: // show
      buffer = new_buf();
      add_buf(buffer,pObjEd->description);
      page_to_char(buf_string(buffer),ch);
      free_buf(buffer);
      break;
    case 2: // replace
      free_string(pObjEd->description);
      pObjEd->description = str_dup("", pObjEd->description);
      string_append(ch, &pObjEd->description, APPEND_AREA, pObjIndex->area);
      break;
    case 3: // edit
      string_append(ch, &pObjEd->description, APPEND_AREA, pObjIndex->area);
      break;
    case 4: // remove
      for ( ped = pObjIndex->extra_descr; ped; ped = ped->next )
      {
        if ( ped->next == pObjEd )
          break;
      }
      if ( ped )
        ped->next = pObjEd->next;
      else
        pObjIndex->extra_descr = pObjEd->next;
      free_extra_descr( pObjEd ); // --> Memcheck : FREE BUF trying to free invalid Extra Descriptor
// OLC ed editor also causes these errors, so I'm not sure what's up
      send_to_char("Page removed.\n\r",ch);
      break;
    case 5: // write
      free_string(pObjEd->description);
      pObjEd->description = str_dup("", pObjEd->description);
      string_append(ch, &pObjEd->description, APPEND_AREA, pObjIndex->area);
      break;
    default:
      send_to_char("Invalid authcommand!  Tell an IMM!\n\r",ch);
      break;
  }
  return;
}

void do_research( CHAR_DATA *ch, char *argument )
{
  char              arg[MIL];
  //char              buf[MSL];
  int               vnum = 0;
  BUFFER            *buffer;
  OBJ_DATA          *obj;
  EXTRA_DESCR_DATA  *objED;

  argument = one_argument( argument, arg );

  if ( IS_NULLSTR( arg ) )
  {
    send_to_char( "Syntax: research <subject>\n\r", ch );
    send_to_char( "        research <subject> <title>\n\r", ch );
    return;
  }

  //First argument is to find a book
  if (!str_prefix( arg, "sociology"))
    vnum = book_sociology;
  if (!str_prefix( arg, "combatology"))
    vnum = book_combatology;
  if (!str_prefix( arg, "illuminology"))
    vnum = book_illuminology;
  if (!str_prefix( arg, "theology"))
    vnum = book_theology;
  if (!str_prefix( arg, "cartography"))
    vnum = book_cartography;
  if (!str_prefix( arg, "history"))
    vnum = book_history;

  if ( !IS_SET(ch->in_room->area->flags, AREA_LIBRARY) ) 
  {
    send_to_char( "You must be in one of the great libraries.\n\r", ch );
    return;
  }

  if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
  {
    send_to_char( "That book is not here.\n\r", ch );
    return;
  }

  if (!obj)
  { // this should never happen, but we have to account for it, just in case
    send_to_char(
      "There was a problem with your research, the immortals are being informed.\n\r",ch);
    bugf( "do_research: Book failed to load, vnum %d.", vnum );
    return;
  }

  if ( IS_NULLSTR( argument ) )
  {
    objED = ed_lookup( "index", obj->pIndexData->extra_descr );
    if (!objED)
    {
      send_to_char("What page would you like to find?\n\r",ch);
      return;
    }

    buffer = new_buf();
    add_buf(buffer, objED->description);
    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);
    return;
  }

  if ( obj->pIndexData->vnum != vnum )
  {
    send_to_char( "You do not see that book here.\n\r", ch );
    return;
  }

  if ( obj->item_type != ITEM_BOOK )
  {
    send_to_char( "That is not a book, it cannot be researched.\n\r", ch );
    return;
  }

  objED = ed_lookup( argument, obj->pIndexData->extra_descr );
  if (!objED)
  {
    send_to_char("That page was not found in the book.\n\r",ch);
    return;
  }

  buffer = new_buf();
  add_buf(buffer, objED->description);
  page_to_char(buf_string(buffer),ch);
  free_buf(buffer);

}
