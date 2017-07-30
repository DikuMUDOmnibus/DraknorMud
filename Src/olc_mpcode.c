/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/
/* The following code is based on ILAB OLC by Jason Dinkel */
/* Mobprogram code by Lordrom for Nevermore Mud */

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
#include "tables.h"
#include "olc.h"
#include "recycle.h"

#define MPEDIT( fun )           bool fun(CHAR_DATA *ch, char*argument)


const struct olc_cmd_type mpedit_table[] =
{
  /*	{	command		function	}, */

  {	"commands",	show_commands	},
  {	"create",	mpedit_create	},
  {	"code",		mpedit_code	    },
  {	"show",		mpedit_show	    },
  {	"list",		mpedit_list	    },
  {   "name",     mpedit_name     },
  {	"?",		show_help	    },

  {	NULL,		0		        }
};

void mpedit( CHAR_DATA *ch, char *argument)
{
  MPROG_CODE *pMcode;
  char arg[MAX_INPUT_LENGTH];
  char command[MAX_INPUT_LENGTH];
  int cmd;
  AREA_DATA *ad;

  smash_tilde(argument);
  strcpy(arg, argument);
  argument = one_argument( argument, command);

  EDIT_MPCODE(ch, pMcode);

  if (pMcode)
  {
    ad = get_vnum_area( pMcode->vnum );

    if ( ad == NULL ) /* ??? */
    {
      edit_done(ch);
      return;
    }

    if ( !IS_BUILDER(ch, ad) )
    {
      send_to_char("MPEdit: Insufficient security to modify code.\n\r", ch);
      edit_done(ch);
      return;
    }
  }

  if (command[0] == '\0')
  {
    mpedit_show(ch, argument);
    return;
  }

  if (!str_cmp(command, "done") )
  {
    edit_done(ch);
    return;
  }

  for (cmd = 0; mpedit_table[cmd].name; cmd++)
  {
    if (!str_prefix(command, mpedit_table[cmd].name) )
    {
      if ((*mpedit_table[cmd].olc_fun) (ch, argument) && pMcode)
        if ((ad = get_vnum_area(pMcode->vnum)) )
          SET_BIT(ad->area_flags, AREA_CHANGED);
      return;
    }
  }

  interpret(ch, arg);

  return;
}

void do_mpedit(CHAR_DATA *ch, char *argument)
{
  MPROG_CODE *pMcode;
  char command[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_PROGRAM ) )
  {
    send_to_char( "Insufficient security to edit mob programs.\n\r", ch );
    return;
  }

  mprintf(sizeof(buf), buf, "MPEDIT %s{x", argument );
  wiznet( buf, NULL, NULL, WIZ_OLC, 0, get_trust( ch ) );

  argument = one_argument(argument, command);

  if ( is_number(command) )
  {
    int vnum = atoi(command);
    AREA_DATA *ad;

    if ( (pMcode = get_mprog_index(vnum)) == NULL )
    {
      send_to_char("MPEdit : That vnum does not exist.\n\r",ch);
      return;
    }

    ad = get_vnum_area(vnum);

    if ( ad == NULL )
    {
      send_to_char( "MPEdit : That VNUM is not assigned to an area.\n\r", ch );
      return;
    }

    if ( !IS_BUILDER(ch, ad) )
    {
      send_to_char("MPEdit : Insufficient security to edit that area.\n\r", ch );
      return;
    }

    ch->desc->pEdit		= (void *)pMcode;
    ch->desc->editor	= ED_MPCODE;

    return;
  }

  if ( !str_cmp(command, "create") )
  {
    if (argument[0] == '\0')
    {
      send_to_char( "Syntax: mpedit create [vnum]\n\r", ch );
      return;
    }

    mpedit_create(ch, argument);
    return;
  }

  send_to_char( "Syntax: mpedit [vnum]\n\r", ch );
  send_to_char( "        mpedit create [vnum]\n\r", ch );

  return;
}

MPEDIT (mpedit_create)
{
  MPROG_CODE *pMcode;
  int value = atoi(argument);
  AREA_DATA *ad;

  if (IS_NULLSTR(argument) || value < 1)
  {
    send_to_char( "Syntax: mpedit create [vnum]\n\r", ch );
    return FALSE;
  }

  ad = get_vnum_area(value);

  if ( ad == NULL )
  {
    send_to_char( "MPEdit : VNUM no asignado a ningun area.\n\r", ch );
    return FALSE;
  }

  if ( !IS_BUILDER(ch, ad) )
  {
    send_to_char("MPEdit : Insuficiente seguridad para crear MobProgs.\n\r", ch);
    return FALSE;
  }

  if ( get_mprog_index(value) )
  {
    send_to_char("MPEdit: Code vnum already exists.\n\r",ch);
    return FALSE;
  }

  pMcode			= new_mpcode();
  pMcode->vnum		= value;
  pMcode->next		= mprog_list;
  mprog_list			= pMcode;
  ch->desc->pEdit		= (void *)pMcode;
  ch->desc->editor		= ED_MPCODE;

  send_to_char("MobProgram Code Created.\n\r",ch);

  return TRUE;
}

MPEDIT(mpedit_show)
{
  MPROG_CODE *pMcode;
  char buf[MAX_STRING_LENGTH];

  EDIT_MPCODE(ch,pMcode);

  mprintf(sizeof(buf),buf,
          "Name:       [%s]\n\r"
          "Vnum:       [%d]\n\r"
          "Code:\n\r%s\n\r",
          pMcode->name, pMcode->vnum, pMcode->code);
  send_to_char(buf, ch);

  return FALSE;
}

MPEDIT(mpedit_code)
{
  MPROG_CODE *pMcode;
  EDIT_MPCODE(ch, pMcode);

  if (argument[0] =='\0')
  {
    string_append(ch, &pMcode->code, APPEND_AREA, pMcode->area);
    return TRUE;
  }
  else if ( !str_cmp( argument, "none" ) )
  {
    free_string( pMcode->code );
    pMcode->code = &str_empty[0];
    send_to_char( "Program code cleared.\n\r", ch );
    return TRUE;
  }

  send_to_char("Syntax: code\n\r",ch);
  return FALSE;
}

MPEDIT( mpedit_list )
{
  int count = 1;
  MPROG_CODE *mprg;
  char buf[MAX_STRING_LENGTH];
  BUFFER *buffer;
  bool fAll = !str_cmp(argument, "all");
  char blah;
  char buf1[MSL];
  AREA_DATA *ad;
  int	col = 0;
  int	max_col = 3;

  buffer = new_buf();

  for (mprg = mprog_list; mprg; mprg = mprg->next)
    if ( fAll || ENTRE(ch->in_room->area->min_vnum, mprg->vnum, ch->in_room->area->max_vnum) )
    {
      ad = get_vnum_area(mprg->vnum);

      if ( ad == NULL )
        blah = '?';
      else
        if ( IS_BUILDER(ch, ad) )
          blah = '*';
        else
          blah = ' ';

      strncpyft_color( buf1,
                       FIX_STR( mprg->name, "(none)", "(null)" ),
                       17, ( col + 1 ) % max_col == 0 ? '\0' : ' ', TRUE );

      mprintf(sizeof(buf),buf, "[%3d]%c %5d %s\n\r", count, blah, mprg->vnum, buf1 );
      add_buf(buffer, buf);

      count++;
    }

  if ( count == 1 )
  {
    if ( fAll )
      add_buf( buffer, "No existing MobPrograms.\n\r" );
    else
      add_buf( buffer, "No existing MobPrograms in this area.\n\r" );
  }

  page_to_char(buf_string(buffer), ch);
  free_buf(buffer);

  return FALSE;
}

/*
 * Edit code name.
 */
MPEDIT( mpedit_name )
{
  MPROG_CODE	*pMcode;

  EDIT_MPCODE( ch, pMcode );

  if ( argument[0] == '\0' )
  {
    send_to_char( "Syntax:  name <string>|none\n\r", ch );
    return FALSE;
  }

  smash_tilde( argument );
  //strip_str_color( argument );

  free_string( pMcode->name );
  if ( str_cmp( argument, "none" ) )
  {
    pMcode->name = str_dup( argument, "" );
    send_to_char( "Name set.\n\r", ch );
  }
  else
  {
    pMcode->name = &str_empty[0];
    send_to_char( "Name cleared.\n\r", ch );
  }

  return TRUE;
}
