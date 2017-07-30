/***************************************************************************
 *  File: olc_act.c                                                        *
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
#include "tables.h"
#include "olc.h"
#include "recycle.h"
#include "lookup.h"
#include "interp.h"

char * mprog_type_to_name ( int type );
void purge_room( ROOM_INDEX_DATA *pRoom, bool override );

#define ALT_FLAGVALUE_SET( _blargh, _table, _arg )    \
  {              \
    int64 blah = flag_value( _table, _arg );    \
    _blargh = (blah == NO_FLAG) ? 0 : blah;    \
  }

#define ALT_FLAGVALUE_TOGGLE( _blargh, _table, _arg )    \
  {              \
    int64 blah = flag_value( _table, _arg );    \
    _blargh ^= (blah == NO_FLAG) ? 0 : blah;  \
  }

/* Return TRUE if area changed, FALSE if not. */
#define REDIT( fun )    bool fun( CHAR_DATA *ch, char *argument )
#define OEDIT( fun )    bool fun( CHAR_DATA *ch, char *argument )
#define MEDIT( fun )    bool fun( CHAR_DATA *ch, char *argument )
#define AEDIT( fun )    bool fun( CHAR_DATA *ch, char *argument )
#define HEDIT( fun )        bool fun( CHAR_DATA *ch, char *argument )



struct olc_help_type
{
    char *command;
    const void *structure;
    char *desc;
};



bool show_version( CHAR_DATA *ch, char *argument )
{
    send_to_char( VERSION, ch );
    send_to_char( "\n\r", ch );
    send_to_char( AUTHOR, ch );
    send_to_char( "\n\r", ch );
    send_to_char( DATE, ch );
    send_to_char( "\n\r", ch );
    send_to_char( CREDITS, ch );
    send_to_char( "\n\r", ch );

    return FALSE;
}    

/*
 * This table contains help commands and a brief description of each.
 * ------------------------------------------------------------------
 */
const struct olc_help_type help_table[] =
{
    {  "area",        area_flags,      "Area attributes."     },
    {  "room",        room_flags,      "Room attributes."     },
    {  "sector",     sector_flags,  "Sector types, terrain." },
    {  "exit",        exit_flags,      "Exit types."       },
    {  "type",        type_flags,      "Types of objects."     },
    {  "extra",      extra_flags,  "Object attributes."   },
    {  "wear",        wear_flags,      "Where to wear object."   },
    {  "spec",        spec_table,   "Available special programs."},
    {  "sex",      sex_flags,      "Sexes."           },
    {  "act",      act_flags,    "Mobile attributes."   },
    {   "act2",         act2_flags,     "Mobile attributes."    },
    {  "aff",          affect_flags,  "Mobile affects."     },
    {   "aff2",         affect2_flags,  "Mobile affects2."       },
    {  "wear-loc",      wear_loc_flags,  "Where mobile wears object." },
    {  "spells",     skill_table,  "Names of current spells."    },
    {  "container",  container_flags,"Container status."     },
/* ROM specific settable bits */
    {  "armor",      ac_type,       "Ac for different attacks."    },
    {   "apply",      apply_flags,   "Apply flags"              },
    {  "form",     form_flags,    "Mobile body form."          },
    {  "part",        part_flags,       "Mobile body parts."        },
    {  "imm",        imm_flags,     "Mobile immunity."            },
    {  "res",      res_flags,       "Mobile resistance."          },
    {  "vuln",        vuln_flags,    "Mobile vulnerability."      },
    {  "off",        off_flags,       "Mobile offensive behaviour."  },
    {  "size",     size_flags,       "Mobile size."              },
    {   "position",     position_flags,  "Mobile positions."            },
    {   "wclass",       weapon_class,    "Weapon class."                }, 
    {   "wtype",        weapon_type2,    "Special weapon type."         },
    {  "portal",      portal_flags,   "Portal types."            },
    {  "furniture",  furniture_flags, "Furniture types."            },
    {   "liquid",     liq_table,       "Liquid types."            },
    {  "apptype",      apply_types,   "Apply types."              },
    {  "weapon",     attack_table,   "Weapon types."              },
    {  "mprog",        mprog_flags,   "MobProgram flags."        },
    {   "status",       clan_flags,     "Clan status settings."         },
    {   "rank",         rank_flags,     "Clan rank settings."           },
    {  NULL,        NULL,          NULL                    }

};



/*****************************************************************************
 Name:    show_flag_cmds
 Purpose:  Displays settable flags and stats.
 Called by:  show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds( CHAR_DATA *ch, const struct flag_type *flag_table )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  flag;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for (flag = 0; flag_table[flag].name; flag++)
    {
  if ( flag_table[flag].settable )
  {
      mprintf(sizeof(buf), buf, "%-19.18s", flag_table[flag].name );
      strcat( buf1, buf );
      if ( ++col % 4 == 0 )
    strcat( buf1, "\n\r" );
  }
    }
 
    if ( col % 4 != 0 )
  strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return;
}


/*****************************************************************************
 Name:    show_skill_cmds
 Purpose:  Displays all skill functions.
     Does remove those damn immortal commands from the list.
     Could be improved by:
     (1) Adding a check for a particular class.
     (2) Adding a check for a level range.
 Called by:  show_help(olc_act.c).
 ****************************************************************************/
void show_skill_cmds( CHAR_DATA *ch, int tar )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH*2 ];
    int  sn;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
  if ( !skill_table[sn].name )
      break;

  if ( !str_cmp( skill_table[sn].name, "reserved" )
    || skill_table[sn].spell_fun == spell_null )
      continue;

  if ( tar == -1 || skill_table[sn].target == tar )
  {
      mprintf(sizeof(buf),buf, "%-19.18s", skill_table[sn].name );
      strcat( buf1, buf );
      if ( ++col % 4 == 0 )
    strcat( buf1, "\n\r" );
  }
    }
 
    if ( col % 4 != 0 )
  strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return;
}



/*****************************************************************************
 Name:    show_spec_cmds
 Purpose:  Displays settable special functions.
 Called by:  show_help(olc_act.c).
 ****************************************************************************/
void show_spec_cmds( CHAR_DATA *ch )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  spec;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    send_to_char( "Preceed special functions with 'spec_'\n\r\n\r", ch );
    for (spec = 0; spec_table[spec].function != NULL; spec++)
    {
  mprintf(sizeof(buf),buf, "%-19.18s", &spec_table[spec].name[5] );
  strcat( buf1, buf );
  if ( ++col % 4 == 0 )
      strcat( buf1, "\n\r" );
    }
 
    if ( col % 4 != 0 )
  strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return;
}



/*****************************************************************************
 Name:    show_help
 Purpose:  Displays help for many tables used in OLC.
 Called by:  olc interpreters.
 ****************************************************************************/
bool show_help( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char spell[MAX_INPUT_LENGTH];
    int cnt;

    argument = one_argument( argument, arg );
    one_argument( argument, spell );

    /*
     * Display syntax.
     */
    if ( arg[0] == '\0' )
    {
  send_to_char( "Syntax:  ? [command]\n\r\n\r", ch );
  send_to_char( "[command]  [description]\n\r", ch );
  for (cnt = 0; help_table[cnt].command; cnt++)
  {
      printf_to_char(ch, "%-10.10s -%s\n\r",
          capitalize( help_table[cnt].command ),
    help_table[cnt].desc );
  }
  return FALSE;
    }

    /*
     * Find the command, show changeable data.
     * ---------------------------------------
     */
    for (cnt = 0; help_table[cnt].command != NULL; cnt++)
    {
        if (  arg[0] == help_table[cnt].command[0]
          && !str_prefix( arg, help_table[cnt].command ) )
  {
      if ( help_table[cnt].structure == spec_table )
      {
    show_spec_cmds( ch );
    return FALSE;
      }
      else
      if ( help_table[cnt].structure == liq_table )
      {
          show_liqlist( ch );
          return FALSE;
      }
      else
      if ( help_table[cnt].structure == attack_table )
      {
          show_damlist( ch );
          return FALSE;
      }
      else
      if ( help_table[cnt].structure == skill_table )
      {

    if ( spell[0] == '\0' )
    {
        send_to_char( "Syntax:  ? spells "
            "[ignore/attack/defend/self/object/all]\n\r", ch );
        return FALSE;
    }

    if ( !str_prefix( spell, "all" ) )
        show_skill_cmds( ch, -1 );
    else if ( !str_prefix( spell, "ignore" ) )
        show_skill_cmds( ch, TAR_IGNORE );
    else if ( !str_prefix( spell, "attack" ) )
        show_skill_cmds( ch, TAR_CHAR_OFFENSIVE );
    else if ( !str_prefix( spell, "defend" ) )
        show_skill_cmds( ch, TAR_CHAR_DEFENSIVE );
    else if ( !str_prefix( spell, "self" ) )
        show_skill_cmds( ch, TAR_CHAR_SELF );
    else if ( !str_prefix( spell, "object" ) )
        show_skill_cmds( ch, TAR_OBJ_INV );
    else
        send_to_char( "Syntax:  ? spell "
            "[ignore/attack/defend/self/object/all]\n\r", ch );
        
    return FALSE;
      }
      else
      {
    show_flag_cmds( ch, help_table[cnt].structure );
    return FALSE;
      }
  }
    }

    show_help( ch, "" );
    return FALSE;
}

REDIT( redit_mshow )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] == '\0' )
    {
  send_to_char( "Syntax:  mshow <vnum>\n\r", ch );
  return FALSE;
    }

    if ( !is_number( argument ) )
    {
       send_to_char( "REdit: That is not a number.\n\r", ch);
       return FALSE;
    }

    if ( is_number( argument ) )
    {
  value = atoi( argument );
  if ( !( pMob = get_mob_index( value ) ))
  {
      send_to_char( "REdit:  That mobile does not exist.\n\r", ch );
      return FALSE;
  }

  ch->desc->pEdit = (void *)pMob;
    }
 
    medit_show( ch, argument );
    ch->desc->pEdit = (void *)ch->in_room;
    return FALSE; 
}



REDIT( redit_oshow )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] == '\0' )
    {
  send_to_char( "Syntax:  oshow <vnum>\n\r", ch );
  return FALSE;
    }

    if ( !is_number( argument ) )
    {
       send_to_char( "REdit: That is not a number.\n\r", ch);
       return FALSE;
    }

    if ( is_number( argument ) )
    {
  value = atoi( argument );
  if ( !( pObj = get_obj_index( value ) ))
  {
      send_to_char( "REdit:  That object does not exist.\n\r", ch );
      return FALSE;
  }

  ch->desc->pEdit = (void *)pObj;
    }
 
    oedit_show( ch, argument );
    ch->desc->pEdit = (void *)ch->in_room;
    return FALSE; 
}

/* Help Editor - kermit 1/98 */
HEDIT (hedit_make)
{
  HELP_DATA *pHelp;
     
  if (argument[0] == '\0')
    {
      send_to_char("Syntax: mpedit make [keyword(s)]\n\r",ch);
      return FALSE;
    }
 
  pHelp                        = new_help();
  pHelp->keyword = str_dup(argument,pHelp->keyword);
  pHelp->next                  = help_first;
  help_first                    = pHelp;
  ch->desc->pEdit               = (void *)pHelp;
 
  send_to_char("New Help Entry Created.\n\r",ch);
  return TRUE;
}

HEDIT( hedit_show)
{
  HELP_DATA *pHelp;

  EDIT_HELP(ch,pHelp);

  if(pHelp->delete_it)  {
    send_to_char("\n\nTHIS HELP IS MARKED FOR DELETION!\n\r",ch);
    return FALSE;
  }
    
  printf_to_char(ch, "Level:       [%d]\n\r"
     "Keywords: %s\n\r"
     "Synopsis: %s\n\r"
     "\n\r%s\n\r",
     pHelp->level, pHelp->keyword, pHelp->synopsis, pHelp->text);
  return FALSE;
}

HEDIT( hedit_desc)
{
  HELP_DATA *pHelp;
  EDIT_HELP(ch, pHelp);

  if (argument[0] =='\0')
    {
      string_append(ch, &pHelp->text, APPEND_HELP, pHelp);
      return TRUE;
    }

  send_to_char(" Syntax: desc\n\r",ch);
  return FALSE;
}
HEDIT( hedit_synopsis)
{
  HELP_DATA *pHelp;
  EDIT_HELP(ch, pHelp);

  if (argument[0] =='\0')
    {
      string_append(ch, &pHelp->synopsis, APPEND_HELP, pHelp);
      return TRUE;
    }

  send_to_char(" Syntax: synopsis\n\r",ch);
  return FALSE;
}

HEDIT( hedit_keywords)
{
  HELP_DATA *pHelp;
  EDIT_HELP(ch, pHelp);

  if(argument[0] == '\0')
    {
      send_to_char(" Syntax: keywords [keywords]\n\r",ch);
      return FALSE;
    }

  pHelp->keyword = str_dup(argument,pHelp->keyword);
  send_to_char( "Keyword(s) Set.\n\r", ch);
  return TRUE;
}

HEDIT(hedit_level)
{
  HELP_DATA *pHelp;

  EDIT_HELP(ch, pHelp);

  if ( argument[0] == '\0' || !is_number( argument ) )
    {
      send_to_char( "Syntax:  level [number]\n\r", ch );
      return FALSE;
    }

  pHelp->level = atoi( argument );

  send_to_char( "Level set.\n\r", ch);
  return TRUE;
}

HEDIT( hedit_delete)
{
  HELP_DATA *pHelp;

  EDIT_HELP(ch,pHelp);

  if(!pHelp->delete_it) {
    pHelp->delete_it = TRUE;
    send_to_char("YOU HAVE MARKED THIS HELP FOR DELETION!\n\r",ch);
    return TRUE;
  }

  pHelp->delete_it = FALSE;
  send_to_char("YOU HAVE UNMARKED THIS HELP FOR DELETION!\n\r",ch);
  return TRUE;
}


/*****************************************************************************
 Name:    check_range( lower vnum, upper vnum )
 Purpose:  Ensures the range spans only one area.
 Called by:  aedit_vnum(olc_act.c).
 ****************************************************************************/
bool check_range( int lower, int upper )
{
    AREA_DATA *pArea;
    int cnt = 0;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
  /*
   * lower < area < upper
   */
        if ( ( lower <= pArea->min_vnum && pArea->min_vnum <= upper )
  ||   ( lower <= pArea->max_vnum && pArea->max_vnum <= upper ) )
      ++cnt;

  if ( cnt > 1 )
      return FALSE;
    }
    return TRUE;
}



AREA_DATA *get_vnum_area( int vnum )
{
    AREA_DATA *pArea;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        if ( vnum >= pArea->min_vnum
          && vnum <= pArea->max_vnum )
            return pArea;
    }

    return 0;
}



/*
 * Area Editor Functions.
 */
AEDIT( aedit_show )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    printf_to_char(ch, "Name:     [%5d] %s\n\r", pArea->vnum, pArea->name );

#if 0  /* ROM OLC */
    printf_to_char(ch, "Recall:   [%5d] %s\n\r", pArea->recall,
  get_room_index( pArea->recall )
  ? get_room_index( pArea->recall )->name : "none" );
#endif /* ROM */

    printf_to_char(ch, "File:        %s\n\r", pArea->file_name  );
    printf_to_char(ch, "Vnums:       [%d-%d] (%d)\n\r", pArea->min_vnum, pArea->max_vnum, pArea->max_vnum-pArea->min_vnum+1 );
    printf_to_char(ch, "Reset rate:  [%d]\n\r", pArea->reset_rate );
    printf_to_char(ch, "Age:         [%d]\n\r", pArea->age      );
    printf_to_char(ch, "Players:     [%d]\n\r", pArea->nplayer  );
    switch (pArea->align)
    {
      case AREA_ALIGN_ALL:     printf_to_char(ch, "Alignment:   [All]\n\r");       break;
      case AREA_ALIGN_GOOD:    printf_to_char(ch, "Alignment:   [Good]\n\r");      break;
      case AREA_ALIGN_EVIL:    printf_to_char(ch, "Alignment:   [Evil]\n\r");      break;
      case AREA_ALIGN_NEUTRAL: printf_to_char(ch, "Alignment:   [Neutral]\n\r");   break;
      default:                 printf_to_char(ch, "Alignment:   [?Unknown?]\n\r"); break;
    }
    printf_to_char(ch, "Security:    [%d]\n\r", pArea->security );
    printf_to_char(ch, "Sysflags:    [%s]\n\r", area_bit_name(pArea->flags)   );
    printf_to_char(ch, "Builders:    [%s]\n\r", pArea->builders );
    printf_to_char(ch, "Lvl Range:   [%d %d]\n\r", pArea->low_range,
                                pArea->high_range );
    printf_to_char(ch, "Credits:     [%s]\n\r",  pArea->credits );
    printf_to_char(ch, "Flags:       [%s]\n\r", flag_string( area_flags, pArea->area_flags ) );
    printf_to_char(ch, "Continent    [%d] %s\n\r", pArea->continent, continent_name(pArea) );
    return FALSE;
}


AEDIT( aedit_reset )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    reset_area( pArea );
    send_to_char( "Area reset.\n\r", ch );

    return FALSE;
}

// purges and reloads each room
AEDIT( aedit_reload )
{
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *pRoom;
  int  vnum;

  if (ch->level < 104)
  {
    send_to_char( "You must be level 105 or higher to reload an area.\n\r", ch );
    return FALSE;
  }

  EDIT_AREA(ch, pArea);

  for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
  {
    if ( ( pRoom = get_room_index( vnum ) ) )
    {
      purge_room( pRoom, TRUE );
      reset_room( pRoom );
    }
  }

  send_to_char( "Area fully reloaded.\n\r", ch );

  return FALSE;
}

AEDIT( aedit_sysflag )
{
    int64 value;
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( (value = flag_value( sys_area_flags, argument )) == NO_FLAG )
    {
          send_to_char( "Syntax: sysflag [flags]\n\r", ch );
          return FALSE;
    }

    TOGGLE_BIT(pArea->flags, value);
    send_to_char( "Area Sys flags toggled.\n\r", ch );
    return TRUE;
}



AEDIT( aedit_create )
{
  AREA_DATA *pArea;

  pArea             = new_area();
  area_last->next   = pArea;
  area_last         = pArea;  /* Thanks, Walker. */
  ch->desc->pEdit   = (void *)pArea;
  pArea->version    = 2;
  pArea->reset_rate = 0;
  pArea->low_range  = -1;
  pArea->high_range = -1;
  pArea->continent  = 3; // unlinked
  pArea->flags      = AREA_NO_QUEST|AREA_DRAFT|AREA_NORESCUE;
  SET_BIT( pArea->area_flags, AREA_ADDED );
  send_to_char( "Area Created.\n\r", ch );
  return FALSE;
}



AEDIT( aedit_name )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )
    {
      send_to_char( "Syntax:   name [$name]\n\r", ch );
      return FALSE;
    }

    free_string( pArea->name );
    pArea->name = str_dup( argument ,pArea->name);

    send_to_char( "Name set.\n\r", ch );
    return TRUE;
}

AEDIT( aedit_credits )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )
    {
  send_to_char( "Syntax:   credits [$credits]\n\r", ch );
  return FALSE;
    }

    free_string( pArea->credits );
    pArea->credits = str_dup( argument,pArea->credits );

    send_to_char( "Credits set.\n\r", ch );
    return TRUE;
}


AEDIT( aedit_file )
{
  AREA_DATA *pArea, *test;
  char file[MAX_STRING_LENGTH];
  int i, length;

  EDIT_AREA(ch, pArea);

  one_argument( argument, file );  /* Forces Lowercase */

  if ( argument[0] == '\0' )
    {
      send_to_char( "Syntax:  filename [$file]\n\r", ch );
      return FALSE;
    }

  /*
   * Simple Syntax Check.
   */
  length = strlen( argument );
  if ( length > 8 )
    {
      send_to_char( "No more than eight characters allowed.\n\r", ch );
      return FALSE;
    }
    
  /*
   * Allow only letters and numbers.
   */
  for ( i = 0; i < length; i++ )
    {
      if ( !isalnum( file[i] ) )
  {
    send_to_char( "Only letters and numbers are valid.\n\r", ch );
    return FALSE;
  }
    }    

  for ( test = area_first; test != NULL; test = test->next)
    {
      char buf[MAX_STRING_LENGTH];

      strcat( buf, file );
      strcat( buf, ".are" );

      if (test == pArea)
  continue;

      if (!str_cmp(test->file_name, buf))
        {
    printf_to_char(ch,"%s: filename is already taken.", buf);
    return FALSE;
        }
    }
  free_string( pArea->file_name );
  strcat( file, ".are" );
  pArea->file_name = str_dup( file ,pArea->file_name);

  send_to_char( "Filename set.\n\r", ch );
  return TRUE;
}



AEDIT( aedit_age )
{
    AREA_DATA *pArea;
    char age[MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);

    one_argument( argument, age );

    if ( !is_number( age ) || age[0] == '\0' )
    {
      send_to_char( "Syntax:  age [#xage]\n\r", ch );
      return FALSE;
    }

    pArea->age = atoi( age );

    send_to_char( "Age set.\n\r", ch );
    return TRUE;
}

AEDIT( aedit_reset_rate )
{
  AREA_DATA *pArea;
  int       reset_rate = 0;
  char      arg[MSL];

  EDIT_AREA( ch, pArea );

  if ( ( ch->level < 108 )
  &&   ( strstr( pArea->builders, ch->name ) == '\0' ) )
  {
    send_to_char( "You cannot set this area's reset rate.\n\r", ch );
    return FALSE;
  }

  one_argument( argument, arg );

  if ( !is_number( arg )
  ||   IS_NULLSTR( arg ) )
  {
    send_to_char( "Syntax: reset_rate <number>\n\r", ch );
    return FALSE;
  }

  reset_rate = atoi( arg );

  if ( reset_rate > 30  
  ||   reset_rate < 1 )  //2 ticks minimum, 60 max
  {
    send_to_char( "Out of acceptable range, please choose between 1-30\n\r", ch );
    return FALSE;
  }

  pArea->reset_rate = reset_rate;

  printf_to_char( ch, "Area will reset every %d ticks.\n\r",
    reset_rate * 2 );
  return TRUE;

}


AEDIT( aedit_align )
{
  AREA_DATA *pArea;
  int align;

  EDIT_AREA( ch, pArea );

  if ( ( ch->level < 108 )
  &&   ( strstr( pArea->builders, ch->name ) == '\0' ) )
  {
    send_to_char( "You cannot set this area's alignment.\n\r", ch );
    return FALSE;
  }

  if (IS_NULLSTR(argument))
  {
    send_to_char( "Syntax: align [good/neutral/evil/all]\n\r", ch );
    return FALSE;
  }

  if (!str_prefix(argument, "all"))
    align = AREA_ALIGN_ALL;
  else if (!str_prefix(argument, "good"))
    align = AREA_ALIGN_GOOD;
  else if (!str_prefix(argument, "evil"))
    align = AREA_ALIGN_EVIL;
  else if (!str_prefix(argument, "neutral"))
    align = AREA_ALIGN_NEUTRAL;
  else
  {
    send_to_char( "Syntax: align [good/neutral/evil/all]\n\r", ch );
    return FALSE;
  }

  pArea->align = align;

  printf_to_char( ch, "Alignment set.\n\r");
  return TRUE;
}



AEDIT( aedit_low )
{
  AREA_DATA *pArea;
  char low[MAX_STRING_LENGTH];

  EDIT_AREA(ch, pArea);

  one_argument( argument, low );

  if ( !is_number( low ) || low[0] == '\0' )
    {
      send_to_char( "Syntax:  low [#xlow]\n\r", ch );
      return FALSE;
    }

  pArea->low_range = atoi( low );

  send_to_char( "Low Range set.\n\r", ch );
  return TRUE;
}

AEDIT( aedit_high )
{
  AREA_DATA *pArea;
  char high[MAX_STRING_LENGTH];

  EDIT_AREA(ch, pArea);

  one_argument( argument, high );

  if ( !is_number( high ) || high[0] == '\0' )
    {
      send_to_char( "Syntax:  high [#xhigh]\n\r", ch );
      return FALSE;
    }

  pArea->high_range = atoi( high );

  send_to_char( "High Range set.\n\r", ch );
  return TRUE;
}


#if 0 /* ROM OLC */
AEDIT( aedit_recall )
{
    AREA_DATA *pArea;
    char room[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, room );

    if ( !is_number( argument ) || argument[0] == '\0' )
    {
  send_to_char( "Syntax:  recall [#xrvnum]\n\r", ch );
  return FALSE;
    }

    value = atoi( room );

    if ( !get_room_index( value ) )
    {
  send_to_char( "AEdit:  Room vnum does not exist.\n\r", ch );
  return FALSE;
    }

    pArea->recall = value;

    send_to_char( "Recall set.\n\r", ch );
    return TRUE;
}
#endif /* ROM OLC */


AEDIT( aedit_security )
{
  AREA_DATA *pArea;
  char sec[MAX_STRING_LENGTH];
  int  value;

  EDIT_AREA(ch, pArea);

  one_argument( argument, sec );

  if ( !is_number( sec ) || sec[0] == '\0' )
    {
      send_to_char( "Syntax:  security [#xlevel]\n\r", ch );
      return FALSE;
    }

  value = atoi( sec );

  if ( value > ch->pcdata->security || value < 0 )
    {
      if ( ch->pcdata->security != 0 )
  {
    printf_to_char(ch, "Security is 0-%d.\n\r", ch->pcdata->security );
  }
      else
  send_to_char( "Security is 0 only.\n\r", ch );
      return FALSE;
    }

  pArea->security = value;

  send_to_char( "Security set.\n\r", ch );
  return TRUE;
}

AEDIT( aedit_continent )
{
  AREA_DATA *pArea;
  char con[MAX_STRING_LENGTH];
  int  value;

  EDIT_AREA(ch, pArea);

  one_argument( argument, con );

  if ( !is_number( con ) || con[0] == '\0' )
    {
      send_to_char( "Syntax:  continent [#number]\n\r", ch );
      return FALSE;
    }

  value = atoi( con );

  if ( value > 50 || value < 0 )
    {
      send_to_char( "The highest number of continents allowed is 50, see Help Continent.\n\r", ch );
      return FALSE;
    }

  pArea->continent = value;

  printf_to_char(ch, "Continent set to %d (%s).\n\r", pArea->continent, continent_name(pArea));
  return TRUE;
}



AEDIT( aedit_builder )
{
  AREA_DATA *pArea;
  char name[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];

  EDIT_AREA(ch, pArea);

  one_argument( argument, name );

  if ( name[0] == '\0' )
    {
      send_to_char( "Syntax:  builder [$name]  -toggles builder\n\r", ch );
      send_to_char( "Syntax:  builder All      -allows everyone\n\r", ch );
      return FALSE;
    }

  name[0] = UPPER( name[0] );

  if ( strstr( pArea->builders, name ) != '\0' )
    {
      pArea->builders = string_replace( pArea->builders, name, "\0" );
      pArea->builders = string_unpad( pArea->builders );

      if ( pArea->builders[0] == '\0' )
  {
    free_string( pArea->builders );
    pArea->builders = str_dup( "None" ,pArea->builders);
  }
      send_to_char( "Builder removed.\n\r", ch );
      return TRUE;
    }
  else
    {
      buf[0] = '\0';
      if ( strstr( pArea->builders, "None" ) != '\0' )
  {
    pArea->builders = string_replace( pArea->builders, "None", "\0" );
    pArea->builders = string_unpad( pArea->builders );
  }

      if (pArea->builders[0] != '\0' )
  {
    strcat( buf, pArea->builders );
    strcat( buf, " " );
  }
      strcat( buf, name );
      free_string( pArea->builders );
      pArea->builders = string_proper( str_dup( buf ,pArea->builders) );

      send_to_char( "Builder added.\n\r", ch );
      send_to_char( pArea->builders,ch);
      return TRUE;
    }
}



AEDIT( aedit_vnum )
{
    AREA_DATA *pArea;
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    argument = one_argument( argument, lower );
    one_argument( argument, upper );

    if ( !is_number( lower ) || lower[0] == '\0'
    || !is_number( upper ) || upper[0] == '\0' )
    {
  send_to_char( "Syntax:  vnum [#xlower] [#xupper]\n\r", ch );
  return FALSE;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = atoi( upper ) ) )
    {
  send_to_char( "AEdit:  Upper must be larger then lower.\n\r", ch );
  return FALSE;
    }
    
    if ( !check_range( atoi( lower ), atoi( upper ) ) )
    {
  send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
  return FALSE;
    }

    if ( get_vnum_area( ilower )
    && get_vnum_area( ilower ) != pArea )
    {
  send_to_char( "AEdit:  Lower vnum already assigned.\n\r", ch );
  return FALSE;
    }

    pArea->min_vnum = ilower;
    send_to_char( "Lower vnum set.\n\r", ch );

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
  send_to_char( "AEdit:  Upper vnum already assigned.\n\r", ch );
  return TRUE;  /* The lower value has been set. */
    }

    pArea->max_vnum = iupper;
    send_to_char( "Upper vnum set.\n\r", ch );

    return TRUE;
}



AEDIT( aedit_lvnum )
{
    AREA_DATA *pArea;
    char lower[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, lower );

    if ( !is_number( lower ) || lower[0] == '\0' )
    {
  send_to_char( "Syntax:  min_vnum [#xlower]\n\r", ch );
  return FALSE;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = pArea->max_vnum ) )
    {
  send_to_char( "AEdit:  Value must be less than the max_vnum.\n\r", ch );
  return FALSE;
    }
    
    if ( !check_range( ilower, iupper ) )
    {
  send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
  return FALSE;
    }

    if ( get_vnum_area( ilower )
    && get_vnum_area( ilower ) != pArea )
    {
  send_to_char( "AEdit:  Lower vnum already assigned.\n\r", ch );
  return FALSE;
    }

    pArea->min_vnum = ilower;
    send_to_char( "Lower vnum set.\n\r", ch );
    return TRUE;
}



AEDIT( aedit_uvnum )
{
    AREA_DATA *pArea;
    char upper[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, upper );

    if ( !is_number( upper ) || upper[0] == '\0' )
    {
  send_to_char( "Syntax:  max_vnum [#xupper]\n\r", ch );
  return FALSE;
    }

    if ( ( ilower = pArea->min_vnum ) > ( iupper = atoi( upper ) ) )
    {
  send_to_char( "AEdit:  Upper must be larger then lower.\n\r", ch );
  return FALSE;
    }
    
    if ( !check_range( ilower, iupper ) )
    {
  send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
  return FALSE;
    }

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
  send_to_char( "AEdit:  Upper vnum already assigned.\n\r", ch );
  return FALSE;
    }

    pArea->max_vnum = iupper;
    send_to_char( "Upper vnum set.\n\r", ch );

    return TRUE;
}



/*
 * Room Editor Functions.
 */
REDIT( redit_show )
{
    ROOM_INDEX_DATA  *pRoom;
    MPROG_LIST      *list;
    char    buf  [MAX_STRING_LENGTH];
    char    buf1 [2*MAX_STRING_LENGTH];
    OBJ_DATA    *obj;
    CHAR_DATA    *rch;
    int      door;
    bool    fcnt;
    
    EDIT_ROOM(ch, pRoom);

    if ( ch && !IS_BUILDER( ch, ch->in_room->area ) )
    {
        send_to_char( 
            "You you do not have enough security to edit this area.\n\r", ch );
        ch->desc->pEdit = NULL;
        ch->desc->editor = 0;
        return FALSE;
    } 

    buf1[0] = '\0';
   
    mprintf(sizeof(buf),buf, "{cVnum:        {w[{y%6d {w] {c-{x %s\n\n\r",
            pRoom->vnum, pRoom->name );
    strcat( buf1, buf );

    mprintf(sizeof(buf),buf, "{cArea:        {w[{y%6d {w] {c-{x %s\n\r", 
            pRoom->area->vnum, pRoom->area->name );
    strcat( buf1, buf );
 
    mprintf(sizeof(buf),buf, "{cDescription:{x\n\r%s\n\r", pRoom->description );
    strcat( buf1, buf );

    if ( pRoom->heal_rate != 100 || pRoom->mana_rate != 100 )
    {
        mprintf(sizeof(buf),buf, 
            "{cHealth rec:  {w[{y%6d {w]   {cMana rec:   {w[{y%6d {w]{x\n\n\r",
            pRoom->heal_rate , pRoom->mana_rate );
        strcat( buf1, buf );
    }

        mprintf(sizeof(buf),buf, "{cSector:     {x[ %s ]\n\r",
          flag_string( sector_flags, pRoom->sector_type ) );
        strcat( buf1, buf );

        mprintf(sizeof(buf),buf, "{cRoom flags: {x[ %s ]\n\r",
          flag_string( room_flags, pRoom->room_flags ) );
        strcat( buf1, buf );

    //if ( pRoom->clan > 0 )
    //{
  //    mprintf(sizeof(buf),buf, "Clan      : [%d] %s\n\r",
  //      pRoom->clan,
  //      clan_table[pRoom->clan].name );
  //    strcat( buf1, buf );
    //}

    if ( !IS_NULLSTR( pRoom->clan_name ) )
    {
        mprintf( sizeof( buf ), buf, "{cClan Name:  {x[ %s ]\n\r",
            pRoom->clan_name );
        strcat( buf1, buf );
    }

    if ( !IS_NULLSTR(pRoom->owner) )
    {
      mprintf(sizeof(buf),buf, "{cOwner: {x[ %s ]\n\r", pRoom->owner );
      strcat( buf1, buf );
    }

    if ( pRoom->extra_descr )
    {
      EXTRA_DESCR_DATA *ed;

      strcat( buf1, "{cDesc Kwds:  {x[ " );

      for ( ed = pRoom->extra_descr; ed; ed = ed->next )
            strcat( strcat( buf1, ed->keyword ), " " );

      strcat( buf1, "]\n\r" );
    }
    mprintf(sizeof(buf),buf,"{cState:      {x[ {y%d{x ]\n\r",pRoom->state );
    strcat( buf1, buf );

    strcat( buf1, "{cCharacters: {x[ " );
    fcnt = FALSE;
    for ( rch = pRoom->people; rch; rch = rch->next_in_room )
    {
    if ( can_see( ch, rch ) )
      {
        one_argument( rch->name, buf );
      strcat( buf1, buf );
      strcat( buf1, " " );
      fcnt = TRUE;
      }
    }

    if ( fcnt )
    {
/*  int end;

  end = strlen(buf1) - 1;
  buf1[end] = ']';
*/
      strcat( buf1, "]\n\r" );
    }
    else
    strcat( buf1, "none ]\n\r" );

    strcat( buf1, "{cObjects:    {x[ " );
    fcnt = FALSE;
    for ( obj = pRoom->contents; obj; obj = obj->next_content )
    {
      one_argument( obj->name, buf );
    strcat( buf1, buf );
    strcat( buf1, " " );
    fcnt = TRUE;
    }

    if ( fcnt )
    {
/*  int end;

  end = strlen(buf1) - 1;
  buf1[end] = ']';
*/
      strcat( buf1, "]\n\n\r" );
    }
    else
    strcat( buf1, "none ]\n\r\n" );

    if (IS_SET(pRoom->room_flags,ROOM_FISHING))
    { // if it's a fishing room, show current VNUMs
      strcat( buf1, "{c    Fish1   Fish2   Fish3   Fish4   Fish5   Fish6   Fish7   Fish8{x\n\r" );

      mprintf(sizeof(buf),buf,
        "{c    (%2d%%)   (%2d%%)   (%2d%%)   (%2d%%)   (%2d%%)   (%2d%%)   (%2d%%)   (%2d%%){x\n\r",
        FC1, FC2, FC3, FC4, FC5, FC6, FC7, FC8 ); // chances defined in merc.h
      strcat( buf1, buf );

      mprintf(sizeof(buf),buf, "{x    %5d   %5d   %5d   %5d   %5d   %5d   %5d   %5d{x\n\r\n\r",
        pRoom->fish[0], pRoom->fish[1],
        pRoom->fish[2], pRoom->fish[3],
        pRoom->fish[4], pRoom->fish[5],
        pRoom->fish[6], pRoom->fish[7]);
      strcat( buf1, buf );
    }

    for ( door = 0; door < MAX_DIR; door++ )
    {
    EXIT_DATA *pexit;

    if ( ( pexit = pRoom->exit[door] ) )
    {
      char word[MAX_INPUT_LENGTH];
      char reset_state[MAX_STRING_LENGTH];
      char *state;
      int i, length;

      mprintf(sizeof(buf),buf, "{c-%-5s to {x[{y%6d {x] {cKey: {x[{y%6d {x] ",
    capitalize(dir_name[door]),
    pexit->u1.to_room ? pexit->u1.to_room->vnum : 0,      /* ROM OLC */
    pexit->key );
      strcat( buf1, buf );

      /*
       * Format up the exit info.
       * Capitalize all flags that are not part of the reset info.
       */
      strcpy( reset_state, flag_string( exit_flags, pexit->rs_flags ) );
      state = flag_string( exit_flags, pexit->exit_info );
      strcat( buf1, " {cExit flags: {x[ " );
      for (; ;)
      {
          state = one_argument( state, word );

      if ( word[0] == '\0' )
      {
/*        int end;

        end = strlen(buf1) - 1;
        buf1[end] = ']';
*/
        strcat( buf1, "]\n\r" );
        break;
      }

      if ( str_infix( word, reset_state ) )
      {
        length = strlen(word);
        for (i = 0; i < length; i++)
      word[i] = UPPER(word[i]);
      }
      strcat( buf1, word );
      strcat( buf1, " " );
      }

      if ( pexit->keyword && pexit->keyword[0] != '\0' )
      {
      mprintf(sizeof(buf),buf, "{cKwds: {x[ %s ]\n\r", pexit->keyword );
      strcat( buf1, buf );
      }
      if ( pexit->description && pexit->description[0] != '\0' )
      {
      mprintf(sizeof(buf),buf, "%s", pexit->description );
      strcat( buf1, buf );
      }
    }
    }

    /* RProgs goes HERE! Merak - 2006-09-23 */
    if ( pRoom->mprogs )
    {
      int cnt;


      mprintf(sizeof(buf),buf,
                "\n\r{cPrograms for {w[{y%6d{w ]{c:{x %s\n\r\n\r", pRoom->vnum,
                pRoom->name );
      strcat( buf1, buf );

      for ( cnt = 0, list = pRoom->mprogs; list; list = list->next )
      {
        if ( cnt == 0 )
        {
          strcat( buf1, " Number Vnum Trigger Phrase\n\r");
          strcat( buf1, "{c ------ ---- ------- ------{x\n\r");
        }
        
        mprintf(sizeof(buf),buf,"{w[{y%5d{w]{x %4d %7s %s\n\r", cnt,
                list->vnum, mprog_type_to_name( list->trig_type ),
                list->trig_phrase);
        strcat( buf1, buf );
        cnt++;
      }
    }

    send_to_char( buf1, ch );
    return FALSE;
}




/* Local function. */
bool change_exit( CHAR_DATA *ch, char *argument, int door )
{
  ROOM_INDEX_DATA *pRoom;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int64  value;

  EDIT_ROOM(ch, pRoom);

  /*
   * Set the exit flags, needs full argument.
   * ----------------------------------------
   */
  if ( ( value = flag_value( exit_flags, argument ) ) != NO_FLAG )
  {
    ROOM_INDEX_DATA *pToRoom;
    sh_int rev;                                    /* ROM OLC */

    if ( !pRoom->exit[door] )
    {
      send_to_char("The exit does not exist.\n\r",ch);
      return FALSE;
    }
  
    /*
    * This room.
    */
    TOGGLE_BIT(pRoom->exit[door]->rs_flags,  value);

    /* Set door to closed if it is locked */
    if ( IS_SET(pRoom->exit[door]->rs_flags, EX_LOCKED) )
    {
      SET_BIT(pRoom->exit[door]->rs_flags, EX_CLOSED);
      SET_BIT(pRoom->exit[door]->rs_flags, EX_ISDOOR);
    }

    /* Don't toggle exit_info because it can be changed by players. */
    pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;

    /*
    * Connected room.
    */
    pToRoom = pRoom->exit[door]->u1.to_room;     /* ROM OLC */
    rev = rev_dir[door];

    if (!pToRoom)
    {
      send_to_char("Error: Room is not valid exit\n\r",ch);
      return FALSE;
    }
    if (pToRoom->exit[rev] != NULL)
    {
      pToRoom->exit[rev]->rs_flags = pRoom->exit[door]->rs_flags;
      pToRoom->exit[rev]->exit_info = pRoom->exit[door]->exit_info;
    }

    send_to_char( "Exit flag toggled.\n\r", ch );
    return TRUE;
  }

  /*
   * Now parse the arguments.
   */
  argument = one_argument( argument, command );
  one_argument( argument, arg );

  if ( command[0] == '\0' && argument[0] == '\0' )  /* Move command. */
    {
      move_char( ch, door, TRUE );                    /* ROM OLC */
      return FALSE;
    }

  if ( command[0] == '?' )
    {
      do_function(ch, &do_help, "EXIT" );
      return FALSE;
    }

  if ( !str_cmp( command, "delete" ) )
    {
      ROOM_INDEX_DATA *pToRoom;
      sh_int rev;                                     /* ROM OLC */
  
      if ( !pRoom->exit[door] )
    {
      send_to_char( "REdit:  Cannot delete a null exit.\n\r", ch );
      return FALSE;
    }
      if ( IS_SET( pRoom->room_flags, ROOM_SHIP ) && pRoom->state >= 0 )
        set_state_room( pRoom, -1 );
      /*
       * Remove ToRoom Exit.
       */
      rev = rev_dir[door];
      pToRoom = pRoom->exit[door]->u1.to_room;       /* ROM OLC */
  
      if ( pToRoom->exit[rev] )
    {
      free_exit( pToRoom->exit[rev] );
      pToRoom->exit[rev] = NULL;
    }

      /*
       * Remove this exit.
       */
      free_exit( pRoom->exit[door] );
      pRoom->exit[door] = NULL;

      send_to_char( "Exit unlinked.\n\r", ch );
      return TRUE;
    }

  if ( !str_cmp( command, "link" ) )
    {
      EXIT_DATA *pExit;
      ROOM_INDEX_DATA *toRoom;

      if ( arg[0] == '\0' || !is_number( arg ) )
  {
    send_to_char( "Syntax:  [direction] link [vnum]\n\r", ch );
    return FALSE;
  }

      value = atoi( arg );

      if ( ! (toRoom = get_room_index( value )) )
  {
    send_to_char( "REdit:  Cannot link to non-existant room.\n\r", ch );
    return FALSE;
  }

    if ( toRoom->area != ch->in_room->area )
    {
        if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_AREA_LINK ) )
        {
            send_to_char(
                "You don't have the necessary security to link areas.\n\r", ch );
            return FALSE;
        }
    }
            

      if ( !IS_BUILDER( ch, toRoom->area ) )
  {
    send_to_char( "REdit:  Cannot link to that area.\n\r", ch );
    return FALSE;
  }

      if ( toRoom->exit[rev_dir[door]] )
  {
    send_to_char( "REdit:  Remote side's exit already exists.\n\r", ch );
    return FALSE;
  }

      if ( IS_SET( pRoom->room_flags, ROOM_SHIP ) ||
           IS_SET( toRoom->room_flags, ROOM_SHIP ) )
        {
          send_to_char( "REdit: Can't link to or from ship.\n\r", ch );
          return FALSE;
        }

      if ( !pRoom->exit[door] )
      pRoom->exit[door] = new_exit();
      SET_BIT( toRoom->area->area_flags, AREA_CHANGED );

      pRoom->exit[door]->u1.to_room = toRoom;
      pRoom->exit[door]->orig_door = door;
      door                    = rev_dir[door];
      pExit                   = new_exit();
      pExit->u1.to_room       = ch->in_room;
      pExit->orig_door  = door;
      toRoom->exit[door]       = pExit;
      send_to_char( "Two-way link established.\n\r", ch );
      return TRUE;
    }
        
  if ( !str_cmp( command, "dig" ) )
    {
      char buf[MAX_STRING_LENGTH];
  
      if ( arg[0] == '\0' || !is_number( arg ) )
    {
      send_to_char( "Syntax: [direction] dig <vnum>\n\r", ch );
      return FALSE;
    }

      if ( IS_SET( pRoom->room_flags, ROOM_SHIP ) )
      {
        send_to_char( "REdit: Can't link from ship.\n\r", ch );
        return FALSE;
      }
  
      if ( ( redit_create( ch, arg ) != FALSE ) )
      {
        mprintf(sizeof(buf),buf, "link %s", arg );
        change_exit( ch, buf, door);
        if (!IS_SET(ch->act,PLR_NODIGMOVE))
          do_function(ch, &do_goto, arg);
        return TRUE;
      }
    }

    if ( !str_cmp( command, "room" ) )
    {
        ROOM_INDEX_DATA *toRoom=0;

        if ( arg[0] == '\0' || !is_number( arg ) )
      {
          send_to_char( "Syntax:  [direction] room [vnum]\n\r", ch );
          return FALSE;
      }

        value = atoi( arg );

        if ( !(toRoom = get_room_index( value )) )
      {
          send_to_char( "REdit:  Cannot link to non-existant room.\n\r", ch );
          return FALSE;
      }

        if ( toRoom->area != ch->in_room->area )
        {
            if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_AREA_LINK ) )
            {
                send_to_char(
                    "You don't have the necessary security to link areas.\n\r", ch );
                return FALSE;
            }
        }


        if ( IS_SET( toRoom->room_flags, ROOM_SHIP ) )
        {
            send_to_char( "REdit: Can't link to ship.\n\r", ch );
            return FALSE;
        }

        if ( !pRoom->exit[door] )
          pRoom->exit[door] = new_exit();


      pRoom->exit[door]->u1.to_room = toRoom;    /* ROM OLC */
      pRoom->exit[door]->orig_door = door;

      send_to_char( "One-way link established.\n\r", ch );
      return TRUE;
    }

  if ( !str_cmp( command, "key" ) )
    {
      OBJ_INDEX_DATA *key;

      if ( arg[0] == '\0' || !is_number( arg ) )
  {
    send_to_char( "Syntax:  [direction] key [vnum]\n\r", ch );
    return FALSE;
  }

      if ( !pRoom->exit[door] )
  {
    send_to_char("Exit does not exist.\n\r",ch);
    return FALSE;
  }

      value = atoi( arg );

      if (value != -1)
      {
        if (value < -1)
        {
          send_to_char( "REdit:  VNUMs are positive!\n\r", ch );
          return FALSE;
        }

        if ( !(key = get_obj_index( value )) )
        {
          send_to_char( "REdit:  Key doesn't exist.\n\r", ch );
          return FALSE;
        }

        if ( key->item_type != ITEM_KEY )
        {
          send_to_char( "REdit:  Object is not a KEY.\n\r", ch );
         return FALSE;
        }
        pRoom->exit[door]->key = value;
        send_to_char( "Exit key set.\n\r", ch );
      }
      else
      {
        pRoom->exit[door]->key = value;
        send_to_char( "Exit key removed.\n\r", ch );
      }


      return TRUE;
    }

  if ( !str_cmp( command, "name" ) )
    {
      if ( arg[0] == '\0' )
  {
    send_to_char( "Syntax:  [direction] name [string]\n\r", ch );
    send_to_char( "         [direction] name none\n\r", ch );
    return FALSE;
  }

      if ( !pRoom->exit[door] )
  {
    send_to_char("Salida no existe.\n\r",ch);
    return FALSE;
  }

      free_string( pRoom->exit[door]->keyword );

      if (str_cmp(arg,"none"))
      pRoom->exit[door]->keyword = str_dup( arg ,pRoom->exit[door]->keyword);
      else
      pRoom->exit[door]->keyword = str_dup( "" ,pRoom->exit[door]->keyword);

      send_to_char( "Exit name set.\n\r", ch );
      return TRUE;
    }

  if ( !str_prefix( command, "description" ) )
    {
      if ( arg[0] == '\0' )
  {
    if ( !pRoom->exit[door] )
      {
        send_to_char("Salida no existe.\n\r",ch);
        return FALSE;
      }

    string_append( ch, &pRoom->exit[door]->description, APPEND_AREA, pRoom->area );
    return TRUE;
  }

      send_to_char( "Syntax:  [direction] desc\n\r", ch );
      return FALSE;
    }

  return FALSE;
}



REDIT( redit_north )
{
    if ( change_exit( ch, argument, DIR_NORTH ) )
  return TRUE;

    return FALSE;
}



REDIT( redit_south )
{
    if ( change_exit( ch, argument, DIR_SOUTH ) )
  return TRUE;

    return FALSE;
}



REDIT( redit_east )
{
    if ( change_exit( ch, argument, DIR_EAST ) )
  return TRUE;

    return FALSE;
}



REDIT( redit_west )
{
    if ( change_exit( ch, argument, DIR_WEST ) )
  return TRUE;

    return FALSE;
}



REDIT( redit_up )
{
    if ( IS_SET( ch->in_room->room_flags, ROOM_SHIP ) )
    {
      send_to_char("Not allowed in a ship-room.\n\r",ch);
      return FALSE;
    }
    if ( change_exit( ch, argument, DIR_UP ) )
  return TRUE;

    return FALSE;
}



REDIT( redit_down )
{
    if ( IS_SET( ch->in_room->room_flags, ROOM_SHIP ) )
    {
      send_to_char( "Not allowed in a ship-room.\n\r", ch );
      return FALSE;
    }
    if ( change_exit( ch, argument, DIR_DOWN ) )
  return TRUE;

    return FALSE;
}



REDIT( redit_ed )
{
  ROOM_INDEX_DATA *pRoom;
  EXTRA_DESCR_DATA *ed;
  char command[MAX_INPUT_LENGTH];
  char keyword[MAX_INPUT_LENGTH];

  EDIT_ROOM(ch, pRoom);

  argument = one_argument( argument, command );
  argument = one_argument( argument, keyword );

  if ( !IS_NULLSTR(command) && !str_cmp( command, "list" ) )
  {
    for ( ed = pRoom->extra_descr; ed; ed = ed->next )
      printf_to_char( ch, "[ %s ]{c:{x\n\r%s\n\r", 
                      ed->keyword, ed->description );
    return TRUE;
  }

 if ( command[0] == '\0' || keyword[0] == '\0' )
    {
      send_to_char( "Syntax:  ed list\n\r", ch );
      send_to_char( "         ed add [keyword]\n\r", ch );
      send_to_char( "         ed edit [keyword]\n\r", ch );
      send_to_char( "         ed delete [keyword]\n\r", ch );
      send_to_char( "         ed format [keyword]\n\r", ch );
      send_to_char( "         ed rename [keyword] [keyword]\n\r", ch );
      return FALSE;
    }
  /* find an ed from list, this can be useful later :P */
  ed = ed_lookup( keyword, pRoom->extra_descr );

  if ( !str_cmp( command, "add" ) )
    {
      if ( ed )
  {
    printf_to_char( ch, "Keyword %s, [ %s ] already exist:\n\r%s\n\r", 
                          keyword, ed->keyword, ed->description );
    return FALSE;
  }

      ed              =   new_extra_descr();
      ed->keyword        =   str_dup( keyword, ed->keyword );
      ed->description    =   str_dup( "" , ed->description);
      ed->next            =   pRoom->extra_descr;
      pRoom->extra_descr  =   ed;

      string_append( ch, &ed->description, APPEND_AREA, pRoom->area );

      return TRUE;
    }

  if ( !ed ) 
  {
    send_to_char( "Redit: Extra description keyword not found.\n\r", ch );
    return FALSE;
  }

  if ( !str_cmp( command, "edit" ) )
  {
    string_append( ch, &ed->description, APPEND_AREA, pRoom->area );
    return TRUE;
  }

  if ( !str_cmp( command, "delete" ) )
    {
      EXTRA_DESCR_DATA *ped = NULL;

      for ( ped = pRoom->extra_descr; ped; ped = ped->next )
      {
      if ( ped->next == ed )
        break;
    }
      if ( ped )
      ped->next = ed->next;
      else
        pRoom->extra_descr = ed->next;

      free_extra_descr( ed );

      send_to_char( "Extra description deleted.\n\r", ch );
      return TRUE;
    }


  if ( !str_cmp( command, "format" ) )
  {
      ed->description = format_string( ed->description );

      send_to_char( "Extra description formatted.\n\r", ch );
      return TRUE;
  }

  if ( !str_cmp( command, "rename" ) )
  {
    if ( !one_argument( argument, keyword ) || keyword[0] == '\0' )
    {
        send_to_char( "Cannot change name to NULL.\n\r", ch );
        return FALSE;
    }
    free_string( ed->keyword );
    ed->keyword = str_dup( keyword, ed->keyword );
    send_to_char( "Extra description renamed.\n\r", ch );
    return TRUE;
  }

  redit_ed( ch, "" );
  return FALSE;
}



REDIT( redit_create )
{
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *pRoom;
  int value;
  int iHash;
    
  EDIT_ROOM(ch, pRoom);

  value = atoi( argument );

  if ( argument[0] == '\0' || value <= 0 )
    {
      send_to_char( "Syntax:  create [vnum > 0]\n\r", ch );
      return FALSE;
    }

  pArea = get_vnum_area( value );
  if ( !pArea )
    {
      send_to_char( "REdit:  That vnum is not assigned an area.\n\r", ch );
      return FALSE;
    }

  if ( !IS_BUILDER( ch, pArea ) )
    {
      send_to_char( "REdit:  Vnum in an area you cannot build in.\n\r", ch );
      return FALSE;
    }

  if ( pArea != ch->in_room->area )
  {
      if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_AREA_LINK ) )
      {
        send_to_char(
            "You do not have the security necessary to link areas.\n\r", ch );
        return FALSE;
      }
  }

  if ( get_room_index( value ) )
    {
      send_to_char( "REdit:  Room vnum already exists.\n\r", ch );
      return FALSE;
    }

  if ((is_revered_room(value)) && (!IS_IMPLEMENTOR(ch)))
    {
      send_to_char("Sorry that is a Revered and protected Vnum\n\r",ch);
      return FALSE;
    }
  if ( !check_area_vnum(pArea, value, ch))
    return FALSE;

  pRoom      = new_room_index();
  pRoom->area      = pArea;
  pRoom->vnum      = value;

  if ( value > top_vnum_room )
    top_vnum_room = value;

  iHash      = value % MAX_KEY_HASH;
  pRoom->next      = room_index_hash[iHash];
  room_index_hash[iHash]  = pRoom;
  ch->desc->pEdit    = (void *)pRoom;

  send_to_char( "Room created.\n\r", ch );
  return TRUE;
}



REDIT( redit_name )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( ch && !IS_BUILDER( ch, ch->in_room->area ) )
    {
        send_to_char(
            "You you do not have enough security to edit this area.\n\r", ch );
        ch->desc->pEdit = NULL;
        ch->desc->editor = 0;
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
  send_to_char( "Syntax:  name [name]\n\r", ch );
  return FALSE;
    }

    free_string( pRoom->name );
    pRoom->name = str_dup( argument ,pRoom->name);

    send_to_char( "Name set.\n\r", ch );
    return TRUE;
}



REDIT( redit_desc )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
      string_append( ch, &pRoom->description, APPEND_AREA, pRoom->area );
      return TRUE;
    }

    send_to_char( "Syntax:  desc\n\r", ch );
    return FALSE;
}

REDIT( redit_heal )
{
    ROOM_INDEX_DATA *pRoom;
    
    EDIT_ROOM(ch, pRoom);
    
    if (is_number(argument))
       {
          pRoom->heal_rate = atoi ( argument );
          send_to_char ( "Heal rate set.\n\r", ch);
          return TRUE;
       }

    send_to_char ( "Syntax : heal <#xnumber>\n\r", ch);
    return FALSE;
}       

REDIT( redit_mana )
{
    ROOM_INDEX_DATA *pRoom;
    
    EDIT_ROOM(ch, pRoom);
    
    if (is_number(argument))
       {
          pRoom->mana_rate = atoi ( argument );
          send_to_char ( "Mana rate set.\n\r", ch);
          return TRUE;
       }

    send_to_char ( "Syntax : mana <#xnumber>\n\r", ch);
    return FALSE;
}       

REDIT( redit_clan )
{
  ROOM_INDEX_DATA *pRoom;

  EDIT_ROOM(ch, pRoom);

  if ( IS_NULLSTR( argument ) )
  {
    send_to_char( "Syntax:  clan <clan name>\n\r", ch );
    send_to_char( "         clan none\n\r", ch );
    return FALSE;
  }

  free_string( pRoom->clan_name );

  if ( !str_cmp( argument, "none" ) )
     pRoom->clan_name = &str_empty[0];
  else
     pRoom->clan_name = str_dup( argument ,pRoom->clan_name);

  send_to_char ( "Clan set.\n\r", ch);
  return TRUE;
}
      
REDIT( redit_format )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    pRoom->description = format_string( pRoom->description );

    send_to_char( "String formatted.\n\r", ch );
    return TRUE;
}



REDIT( redit_mreset )
{
    ROOM_INDEX_DATA  *pRoom;
    MOB_INDEX_DATA  *pMobIndex;
    CHAR_DATA    *newmob;
    char    arg [ MAX_INPUT_LENGTH ];
    char    arg2 [ MAX_INPUT_LENGTH ];

    RESET_DATA    *pReset;
    
    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' || !is_number( arg ) )
    {
  send_to_char ( "Syntax:  mreset <vnum> <max #x> <mix #x>\n\r", ch );
  return FALSE;
    }

    if ( !( pMobIndex = get_mob_index( atoi( arg ) ) ) )
    {
  send_to_char( "REdit: No mobile has that vnum.\n\r", ch );
  return FALSE;
    }

    if ( pMobIndex->area != pRoom->area )
    {
  send_to_char( "REdit: No such mobile in this area.\n\r", ch );
  return FALSE;
    }
    /*
     * Create the mobile reset.
     */
    pReset              = new_reset_data();
    pReset->command  = 'M';
    pReset->arg1  = pMobIndex->vnum;
    pReset->arg2  = is_number( arg2 ) ? atoi( arg2 ) : MAX_MOB;
    pReset->arg3  = pRoom->vnum;
    pReset->arg4  = is_number( argument ) ? atoi (argument) : 1;
    add_reset( pRoom, pReset, 0/* Last slot*/ );
    /*
     * Create the mobile.
     */
    newmob = create_mobile( pMobIndex );
    char_to_room( newmob, pRoom );

    printf_to_char(ch, "%s (%d) has been loaded and added to resets.\n\r"
  "There will be a maximum of %d loaded to this room.\n\r",
  capitalize( pMobIndex->short_descr ),
  pMobIndex->vnum,
  pReset->arg2 );
    act( "$n has created $N!", ch, NULL, newmob, TO_ROOM );
    return TRUE;
}



struct wear_type
{
    int  wear_loc;
    int  wear_bit;
};



const struct wear_type wear_table[] =
{
    {  WEAR_NONE,      ITEM_TAKE        },
    {  WEAR_LIGHT,      ITEM_LIGHT        },
    {  WEAR_FINGER_L,  ITEM_WEAR_FINGER  },
    {  WEAR_FINGER_R,  ITEM_WEAR_FINGER  },
    {  WEAR_NECK_1,  ITEM_WEAR_NECK    },
    {  WEAR_NECK_2,  ITEM_WEAR_NECK    },
    {  WEAR_BODY,      ITEM_WEAR_BODY    },
    {  WEAR_HEAD,    ITEM_WEAR_HEAD    },
    {  WEAR_LEGS,      ITEM_WEAR_LEGS    },
    {  WEAR_FEET,    ITEM_WEAR_FEET    },
    {  WEAR_HANDS,      ITEM_WEAR_HANDS    },
    {  WEAR_ARMS,    ITEM_WEAR_ARMS    },
    {  WEAR_SHIELD,  ITEM_WEAR_SHIELD  },
    {  WEAR_ABOUT,      ITEM_WEAR_ABOUT    },
    {  WEAR_WAIST,   ITEM_WEAR_WAIST    },
    {  WEAR_WRIST_L,  ITEM_WEAR_WRIST    },
    {  WEAR_WRIST_R,  ITEM_WEAR_WRIST    },
    {  WEAR_WIELD,      ITEM_WIELD        },
    {  WEAR_HOLD,    ITEM_HOLD          },
    {  WEAR_BAG,      ITEM_WEAR_BAG    },
    {   WEAR_BACK,      ITEM_WEAR_BACK      },
    {   WEAR_EAR_L,     ITEM_WEAR_EAR       },
    {   WEAR_EAR_R,     ITEM_WEAR_EAR       },
    {   WEAR_LAPEL,     ITEM_WEAR_LAPEL     },
    {   WEAR_CREST,     ITEM_WEAR_CREST     },
    {  NO_FLAG,      NO_FLAG           }
};



/*****************************************************************************
 Name:    wear_loc
 Purpose:  Returns the location of the bit that matches the count.
     1 = first match, 2 = second match etc.
 Called by:  oedit_reset(olc_act.c).
 ****************************************************************************/
int wear_loc( int bits, int count )
{
    int flag;
 
    for ( flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++ )
    {
        if ( IS_SET( bits, wear_table[flag].wear_bit ) && --count < 1 )
            return wear_table[flag].wear_loc;
    }
 
    return NO_FLAG;
}



/*****************************************************************************
 Name:    wear_bit
 Purpose:  Converts a wear_loc into a bit.
 Called by:  redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit( int loc )
{
    int flag;
 
    for ( flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++ )
    {
        if ( loc == wear_table[flag].wear_loc )
            return wear_table[flag].wear_bit;
    }
 
    return 0;
}



REDIT( redit_oreset )
{
  ROOM_INDEX_DATA  *pRoom;
  OBJ_INDEX_DATA  *pObjIndex;
  OBJ_DATA        *newobj;
  OBJ_DATA      *to_obj;
  CHAR_DATA        *to_mob;
  char             arg1[MAX_INPUT_LENGTH];
  char             arg2[MAX_INPUT_LENGTH];
  int           olevel = 0;

  RESET_DATA    *pReset;

    
  EDIT_ROOM(ch, pRoom);

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
      send_to_char ( "Syntax:  oreset <vnum> <args>\n\r", ch );
      send_to_char ( "        -no_args               = into room\n\r", ch );
      send_to_char ( "        -<obj_name>            = into obj\n\r", ch );
      send_to_char ( "        -<mob_name> <wear_loc> = into mob\n\r", ch );
      return FALSE;
    }

  if ( !( pObjIndex = get_obj_index( atoi( arg1 ) ) ) )
    {
      send_to_char( "REdit: No object has that vnum.\n\r", ch );
      return FALSE;
    }

  if ( pObjIndex->area != pRoom->area )
    {
      send_to_char( "REdit: No such object in this area.\n\r", ch );
      return FALSE;
    }

  /*
   * Load into room.
   */
  if ( arg2[0] == '\0' )
    {
      pReset    = new_reset_data();
      pReset->command  = 'O';
      pReset->arg1  = pObjIndex->vnum;
      pReset->arg2  = 0;
      pReset->arg3  = pRoom->vnum;
      pReset->arg4  = 0;
      add_reset( pRoom, pReset, 0/* Last slot*/ );

      newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
      obj_to_room( newobj, pRoom );

      printf_to_char(ch, "%s (%d) has been loaded and added to resets.\n\r",
         capitalize( pObjIndex->short_descr ),
         pObjIndex->vnum );
    }
  else
    /*
     * Load into object's inventory.
     */
    if ( argument[0] == '\0'
   && ( ( to_obj = get_obj_list( ch, arg2, pRoom->contents ) ) != NULL ) )
      {
  pReset    = new_reset_data();
  pReset->command  = 'P';
  pReset->arg1  = pObjIndex->vnum;
  pReset->arg2  = 0;
  pReset->arg3  = to_obj->pIndexData->vnum;
  pReset->arg4  = 1;
  add_reset( pRoom, pReset, 0/* Last slot*/ );

  newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
  newobj->cost = 0;
  obj_to_obj( newobj, to_obj );

  printf_to_char(ch, "%s (%d) has been loaded into "
           "%s (%d) and added to resets.\n\r",
           capitalize( newobj->short_descr ),
           newobj->pIndexData->vnum,
           to_obj->short_descr,
           to_obj->pIndexData->vnum );
      }
    else
      /*
       * Load into mobile's inventory.
       */
      if ( ( to_mob = get_char_room( ch, arg2 ) ) != NULL )
  {
    int64  wear_loc;

    /*
     * Make sure the location on mobile is valid.
     */
    if ( (wear_loc = flag_value( wear_loc_flags, argument )) == NO_FLAG )
      {
        send_to_char( "REdit: Invalid wear_loc.  '? wear-loc'\n\r", ch );
        return FALSE;
      }

    /*
     * Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
     */
    if ( !IS_SET( pObjIndex->wear_flags, wear_bit(wear_loc) ) )
      {
        printf_to_char(ch,
           "%s (%d) has wear flags: [%s]\n\r",
           capitalize( pObjIndex->short_descr ),
           pObjIndex->vnum,
           flag_string( wear_flags, pObjIndex->wear_flags ) );
        return FALSE;
      }

    /*
     * Can't load into same position.
     */
    if ( get_eq_char( to_mob, wear_loc ) )
      {
        send_to_char( "REdit:  Object already equipped.\n\r", ch );
        return FALSE;
      }

    pReset    = new_reset_data();
    pReset->arg1  = pObjIndex->vnum;
    pReset->arg2  = wear_loc;
    if ( pReset->arg2 == WEAR_NONE )
      pReset->command = 'G';
    else
      pReset->command = 'E';
    pReset->arg3  = wear_loc;

    add_reset( pRoom, pReset, 0/* Last slot*/ );

    olevel  = URANGE( 0, to_mob->level - 2, LEVEL_HERO );
    newobj = create_object( pObjIndex, number_fuzzy( olevel ) );

    if ( to_mob->pIndexData->pShop )  /* Shop-keeper? */
      {
        switch ( pObjIndex->item_type )
    {
    default:     olevel = 0;        break;
    case ITEM_PILL:   olevel = number_range(  0, 10 );  break;
    case ITEM_POTION:   olevel = number_range(  0, 10 );  break;
    case ITEM_SCROLL:   olevel = number_range(  5, 15 );  break;
    case ITEM_PROJECTILE:   olevel = number_range(  5, 15 );  break;
    case ITEM_SCRY_MIRROR:   olevel = number_range(  15, 25 );  break;
    case ITEM_SPELLBOOK: olevel = number_range(  5, 15 );  break; // does this affect the fact that it's now a scroll container?
    case ITEM_WAND:   olevel = number_range( 10, 20 );  break;
    case ITEM_STAFF:   olevel = number_range( 15, 25 );  break;
    case ITEM_ARMOR:   olevel = number_range(  5, 15 );  break;
    case ITEM_WEAPON:  if ( pReset->command == 'G' )
      olevel = number_range( 5, 15 );
    else
      olevel = number_fuzzy( olevel );
    break;
    }

        newobj = create_object( pObjIndex, olevel );
        if ( pReset->arg2 == WEAR_NONE )
    SET_BIT( newobj->extra_flags, ITEM_INVENTORY );
      }
    else
      newobj = create_object( pObjIndex, number_fuzzy( olevel ) );

    obj_to_char( newobj, to_mob );
    if ( pReset->command == 'E' )
      equip_char( to_mob, newobj, pReset->arg3 );

    printf_to_char(ch, "%s (%d) has been loaded "
       "%s of %s (%d) and added to resets.\n\r",
       capitalize( pObjIndex->short_descr ),
       pObjIndex->vnum,
       flag_string( wear_loc_strings, pReset->arg3 ),
       to_mob->short_descr,
       to_mob->pIndexData->vnum );
  }
      else  /* Display Syntax */
  {
    send_to_char( "REdit:  That mobile isn't here.\n\r", ch );
    return FALSE;
  }

  act( "$n has created $p!", ch, newobj, NULL, TO_ROOM );
  return TRUE;
}



/*
 * Object Editor Functions.
 */
void show_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *obj )
{
  switch( obj->item_type )
    {
    default:  /* No values. */
      break;
            
    case ITEM_LOCKER:
      if (obj->value[0])
  printf_to_char(ch, "Clan Locker : [%d] %s\n\r",
           obj->value[0],
           clan_table[obj->value[0]].name );
      else
  printf_to_char(ch, "Unclanned Locker\n\r");
 
      break;
    case ITEM_CHECKERS:
      printf_to_char( ch,
          "[v0] Checker board 1: [%lu]\n\r"
          "[v1] Checker board 2: [%lu]\n\r"
          "[v2] Checker board 3: [%lu]\n\r"
          "[v3]                : Reserved\n\r"
          "[v4]                : Reserved\n\r",
          obj->value[0], obj->value[1], obj->value[2] );
      break; 
    case ITEM_LIGHT:
      if ( obj->value[2] == -1 || obj->value[2] == 999 ) /* ROM OLC */
  printf_to_char(ch, "[v2] Light:  Infinite[-1]\n\r" );
      else
  printf_to_char(ch, "[v2] Light:  [%d]\n\r", obj->value[2] );
      break;

    case ITEM_SLOT_MACHINE:
      printf_to_char( ch, 
          "[v0] Gold Cost:       [%d]\n\r"
          "[v1] Jackpot Value:   [%d]\n\r"
          "[v2] Number of Bars:  [%d]\n\r"
          "[v3] Partial Jackpot: [%d]\n\r"
          "[v4] Freeze Jackpot:  [%d]\n\r", 
          obj->value[0], 
          obj->value[1], 
          obj->value[2], 
          obj->value[3], 
          obj->value[4] );
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      printf_to_char(ch,
         "[v0] Level:          [%d]\n\r"
         "[v1] Charges Total:  [%d]\n\r"
         "[v2] Charges Left:   [%d]\n\r"
         "[v3] Spell:          %s\n\r",
         obj->value[0],
         obj->value[1],
         obj->value[2],
         obj->value[3] != -1 ? skill_table[obj->value[3]].name
         : "none" );
      break;

    case ITEM_PORTAL:
      printf_to_char(ch,
         "[v0] Charges:        [%d]\n\r"
         "[v1] Exit Flags:     %s\n\r"
         "[v2] Portal Flags:   %s\n\r"
         "[v3] Goes to (vnum): [%d]\n\r",
         obj->value[0],
         flag_string( exit_flags, obj->value[1]),
         flag_string( portal_flags , obj->value[2]),
         obj->value[3] );
      break;
      
    case ITEM_FURNITURE:          
      printf_to_char(ch,
         "[v0] Max people:      [%d]\n\r"
         "[v1] Max weight:      [%d]\n\r"
         "[v2] Furniture Flags: %s\n\r"
         "[v3] Heal bonus:      [%d]\n\r"
         "[v4] Mana bonus:      [%d]\n\r",
         obj->value[0],
         obj->value[1],
         flag_string( furniture_flags, obj->value[2]),
         obj->value[3],
         obj->value[4] );
      break;

    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
      printf_to_char(ch,
         "[v0] Level:  [%d]\n\r"
         "[v1] Spell:  %s\n\r"
         "[v2] Spell:  %s\n\r"
         "[v3] Spell:  %s\n\r"
         "[v4] Spell:  %s\n\r",
         obj->value[0],
         obj->value[1] != -1 ? skill_table[obj->value[1]].name
         : "none",
         obj->value[2] != -1 ? skill_table[obj->value[2]].name
         : "none",
         obj->value[3] != -1 ? skill_table[obj->value[3]].name
         : "none",
         obj->value[4] != -1 ? skill_table[obj->value[4]].name
         : "none" );
      break;

    case ITEM_SPELLBOOK:
      printf_to_char(ch,
         "[v0] Max scrolls:      %d\n\r"
         "[v1] Max scroll level: %d\n\r",
         obj->value[0],
         obj->value[1]);
      break;

    case ITEM_KEYRING:
      printf_to_char(ch, "[v0] Max keys:      %d\n\r", obj->value[0]);
      break;

    case ITEM_QUIVER:
      printf_to_char(ch,
         "[v0] Max projectiles: %d\n\r"
         "[v1] Item divisor:    %d\n\r",
         obj->value[0],
         obj->value[1]);
      break;

    case ITEM_PROJECTILE:
      if (IS_SET( obj->value[0], ITEM_PROJECTILE_MAGICBOMB))
      {
        printf_to_char(ch,
          "     Magic Projectile\n\r"
          "[v3] Spell:  %s\n\r"
          "[v4] Level:  %s\n\r",
         obj->value[3] != -1 ? skill_table[obj->value[3]].name : "none",
         obj->value[4]);
      }
      else
      {
        printf_to_char(ch, "[v0] Projectile flags: [");
        if (IS_SET(obj->value[0], ITEM_PROJECTILE_THROW))
          send_to_char("thrown ",ch);
        if (IS_SET(obj->value[0], ITEM_PROJECTILE_BOMB))
          send_to_char("bomb ",ch);
        if (IS_SET(obj->value[0], ITEM_PROJECTILE_SLING))
          send_to_char("sling ",ch);
        if (IS_SET(obj->value[0], ITEM_PROJECTILE_BOW))
          send_to_char("bow ",ch);
        if (IS_SET(obj->value[0], ITEM_PROJECTILE_CROSSBOW))
          send_to_char("crossbow ",ch);
        if (IS_SET(obj->value[0], ITEM_PROJECTILE_ATLATL))
          send_to_char("atlatl ",ch);
        printf_to_char(ch, "]\n\r", obj->value[1] );
        printf_to_char(ch, "[v1] Number of dice:   [%d]\n\r", obj->value[1] );
        printf_to_char(ch, "[v2] Type of dice:     [%d] (min=%d avg=%d max=%d)\n\r", 
           obj->value[2],
           DICE_MIN( obj->value[1], obj->value[2] ),
           DICE_AVG( obj->value[1], obj->value[2] ),
           DICE_MAX( obj->value[1], obj->value[2] ));
      }
      break;

    case ITEM_SCRY_MIRROR:
      printf_to_char(ch,
         "[v0] Max Charges:      %d\n\r"
         "[v1] Current Charges:  %d\n\r",
         obj->value[0],
         obj->value[1]);
      break;

    case ITEM_FISHING_ROD:
      printf_to_char(ch,
         "[v0] Rod Length:   %d\n\r"
         "[v1] Rod Strength: %d\n\r",
         obj->value[0],
         obj->value[1]);
      break;

      /* ARMOR for ROM */
    case ITEM_ARMOR:
      printf_to_char(ch,
         "[v0] Ac pierce       [%d]\n\r"
         "[v1] Ac bash         [%d]\n\r"
         "[v2] Ac slash        [%d]\n\r"
         "[v3] Ac exotic       [%d]\n\r",
         obj->value[0],
         obj->value[1],
         obj->value[2],
         obj->value[3] );
      break;

      /* WEAPON changed in ROM: */
      /* I had to split the output here, I have no idea why, but it helped -- Hugin */
      /* It somehow fixed a bug in showing scroll/pill/potions too ?! */
    case ITEM_WEAPON:
      printf_to_char(ch, "[v0] Weapon class:   %s\n\r",
         flag_string( weapon_class, obj->value[0] ) );
      printf_to_char(ch, "[v1] Number of dice: [%d]\n\r", obj->value[1] );
      printf_to_char(ch, "[v2] Type of dice:   [%d] (min=%d avg=%d max=%d)\n\r", 
         obj->value[2],
         DICE_MIN( obj->value[1], obj->value[2] ),
             DICE_AVG( obj->value[1], obj->value[2] ),
             DICE_MAX( obj->value[1], obj->value[2] ));
      printf_to_char(ch, "[v3] Type:           [%s]\n\r",
             get_attack_name( obj->value[3] ) );
      printf_to_char( ch, "                     %s\n\r",
             get_attack_noun( obj->value[3] ) );
      printf_to_char(ch, "[v4] Special type:   %s\n\r",
         flag_string( weapon_type2,  obj->value[4] ));
      printf_to_char(ch, "[v5] Acid Affect:    %d\n\r",
        obj->value[5] );
      break;

    case ITEM_CONTAINER:
      printf_to_char(ch,
         "[v0] Weight:                 [%d kg]\n\r"
         "[v1] Flags:                  [%s]\n\r"
         "[v2] Key:                    [%d] %s\n\r"
         "[v3] Max Weight Per Item:    [%d]\n\r"
         "[v4] Weight Mult             [%d]\n\r",
         obj->value[0],
         flag_string( container_flags, obj->value[1] ),
         obj->value[2],
         get_obj_index(obj->value[2])
         ? get_obj_index(obj->value[2])->short_descr
         : "none",
         obj->value[3],
         obj->value[4] );
      break;

    case ITEM_MONEY_POUCH:
      printf_to_char(ch,
         "[v0] Weight Mult:[%d]\n\r",
         obj->value[0]);
      break;

    case ITEM_DRINK_CON:
      printf_to_char(ch,
         "[v0] Liquid Total: [%d]\n\r"
         "[v1] Liquid Left:  [%d]\n\r"
         "[v2] Liquid:       %s\n\r"
         "[v3] Poisoned:     %s\n\r",
         obj->value[0],
         obj->value[1],
         liq_table[obj->value[2]].liq_name,
         obj->value[3] != 0 ? "Yes" : "No" );
      break;

    case ITEM_FOUNTAIN:
      printf_to_char(ch,
         "[v0] Liquid Total: [%d]\n\r"
         "[v1] Liquid Left:  [%d]\n\r"
         "[v2] Liquid:      %s\n\r",
         obj->value[0],
         obj->value[1],
         liq_table[obj->value[2]].liq_name );
      break;
          
    case ITEM_FOOD:
      printf_to_char(ch,
         "[v0] Food hours: [%d]\n\r"
         "[v1] Full hours: [%d]\n\r"
         "[v3] Poisoned:   %s\n\r",
         obj->value[0],
         obj->value[1],
         obj->value[3] != 0 ? "Yes" : "No" );
      break;

    case ITEM_CORPSE_NPC:
      printf_to_char(ch, "[v0] Frag Number [%d]\n\r", obj->value[0] );
      break;
    
    case ITEM_MONEY:
      printf_to_char(ch, "[v0] Silver: [%d]\n\r", obj->value[0] );
      printf_to_char(ch, "[v1] Gold:   [%d]\n\r", obj->value[1] );
      break;
    }

  return;
}



bool set_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, int value_num, char *argument)
{
  int cs=0;
  switch( pObj->item_type )
    {
    default:
      break;
            
        case ITEM_CHECKERS:
        switch ( value_num )
        {
            default:
                send_to_char( "set v0 - v2 to the checker board positions.\n\r", ch );
                return FALSE;
            case 0:
                send_to_char( "CHECKER BOARD 1 SET.\n\r\n\r", ch );
                pObj->value[0] = atoi( argument );
                break;
            case 1:
                send_to_char( "CHECKER BOARD 2 SET.\n\r\n\r", ch );
                pObj->value[1] = atoi( argument );
                break;
            case 2:
                send_to_char( "CHECKER BOARD 3 SET.\n\r\n\r", ch );
                pObj->value[2] = atoi( argument );
                break;
        }
        break;
    case ITEM_LIGHT:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_LIGHT" );
    return FALSE;
  case 2:
    send_to_char( "HOURS OF LIGHT SET.\n\r\n\r", ch );
    pObj->value[2] = atoi( argument );
    break;
  }
      break;

    case ITEM_LOCKER:
      switch ( value_num )
  {
  default:
    send_to_char("set clan in v0\n\r" ,ch );
    return FALSE;
  case 0:
    send_to_char( "CLAN SET.\n\r\n\r", ch );
    pObj->value[0] = clan_lookup( argument );
          if(pObj->value[0] < 0)
              pObj->value[0] = 0;
    break;
  }
      break;

    case ITEM_SLOT_MACHINE:
      switch (value_num)
  {
  case 0:
    send_to_char( "GOLD COST SET.\n\r\n\r", ch);
    pObj->value[0] = atoi( argument );
    break;
  case 1:
    send_to_char( "JACKPOT VALUE.\n\r\n\r", ch);
    pObj->value[1] = atoi( argument );
    break;
  case 2:
    send_to_char( "NUMBER OF BARS SET.\n\r\n\r", ch);
    pObj->value[2] = atoi( argument );
    break;
  case 3:
    send_to_char( "PARTIAL JACKPOT SET.\n\r\n\r", ch);
    pObj->value[3] = atoi( argument );
    break;
  case 4:
    send_to_char( "FREEZE JACKPOT SET.\n\r\n\r", ch);
    pObj->value[4] = atoi(argument);
    break;
  }
       break;
    case ITEM_WAND:
    case ITEM_STAFF:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_STAFF_WAND" );
    return FALSE;
  case 0:
    send_to_char( "SPELL LEVEL SET.\n\r\n\r", ch );
    pObj->value[0] = atoi( argument );
    break;
  case 1:
    send_to_char( "TOTAL NUMBER OF CHARGES SET.\n\r\n\r", ch );
    pObj->value[1] = atoi( argument );
    break;
  case 2:
    send_to_char( "CURRENT NUMBER OF CHARGES SET.\n\r\n\r", ch );
    pObj->value[2] = atoi( argument );
    break;
  case 3:
    send_to_char( "SPELL TYPE SET.\n\r", ch );
    pObj->value[3] = skill_lookup( argument );
    break;
  }
      break;

    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_SCROLL_POTION_PILL" );
    return FALSE;
  case 0:
    send_to_char( "SPELL LEVEL SET.\n\r\n\r", ch );
    pObj->value[0] = atoi( argument );
    break;
  case 1:
    send_to_char( "SPELL TYPE 1 SET.\n\r\n\r", ch );
    pObj->value[1] = skill_lookup( argument );
    break;
  case 2:
    send_to_char( "SPELL TYPE 2 SET.\n\r\n\r", ch );
    pObj->value[2] = skill_lookup( argument );
    break;
  case 3:
    send_to_char( "SPELL TYPE 3 SET.\n\r\n\r", ch );
    pObj->value[3] = skill_lookup( argument );
    break;
  case 4:
    send_to_char( "SPELL TYPE 4 SET.\n\r\n\r", ch );
    pObj->value[4] = skill_lookup( argument );
    break;
  }
      break;

    case ITEM_SPELLBOOK:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_SPELLBOOK" );
    return FALSE;
  case 0:
    send_to_char( "MAX SCROLLS SET.\n\r\n\r", ch );
    pObj->value[0] = atoi( argument );
    break;
  case 1:
    send_to_char( "MAX SCROLL LEVEL SET.\n\r\n\r", ch );
    pObj->value[1] = atoi( argument );
    break;
  }
    break;

    case ITEM_KEYRING:
      switch ( value_num )
      {
        default:
          do_function(ch, &do_help, "ITEM_KEYRING" );
          return FALSE;
        case 0:
          send_to_char( "MAX KEYS SET.\n\r\n\r", ch );
          pObj->value[0] = atoi( argument );
          break;
      }
      break;

    case ITEM_QUIVER:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_QUIVER" );
    return FALSE;
  case 0:
    send_to_char( "MAX ITEMS SET.\n\r\n\r", ch );
    pObj->value[0] = atoi( argument );
    break;
  case 1:
    send_to_char( "ITEM CT/WT DIVISOR SET.\n\r\n\r", ch );
    pObj->value[1] = atoi( argument );
    break;
  }
    break;

    case ITEM_PROJECTILE:
      switch ( value_num )
      {
        default:
          do_function(ch, &do_help, "ITEM_PROJECTILE" );
          return FALSE;
        case 0:
          if (!str_prefix(argument, "magicbomb"))
          {
            SET_BIT(pObj->value[0], ITEM_PROJECTILE_MAGICBOMB);
            send_to_char( "PROJECTILE FLAG SET.\n\r\n\r", ch );
          }
          else if (!str_prefix(argument, "thrown"))
          {
            SET_BIT(pObj->value[0], ITEM_PROJECTILE_THROW);
            send_to_char( "PROJECTILE FLAG SET.\n\r\n\r", ch );
          }
          else if (!str_prefix(argument, "bomb"))
          {
            SET_BIT(pObj->value[0], ITEM_PROJECTILE_BOMB);
            send_to_char( "PROJECTILE FLAG SET.\n\r\n\r", ch );
          }
          else if (!str_prefix(argument, "sling"))
          {
            SET_BIT(pObj->value[0], ITEM_PROJECTILE_SLING);
            send_to_char( "PROJECTILE FLAG SET.\n\r\n\r", ch );
          }
          else if (!str_prefix(argument, "bow"))
          {
            SET_BIT(pObj->value[0], ITEM_PROJECTILE_BOW);
            send_to_char( "PROJECTILE FLAG SET.\n\r\n\r", ch );
          }
          else if (!str_prefix(argument, "crossbow"))
          {
            SET_BIT(pObj->value[0], ITEM_PROJECTILE_CROSSBOW);
            send_to_char( "PROJECTILE FLAG SET.\n\r\n\r", ch );
          }
          else if (!str_prefix(argument, "atlatl"))
          {
            SET_BIT(pObj->value[0], ITEM_PROJECTILE_ATLATL);
            send_to_char( "PROJECTILE FLAG SET.\n\r\n\r", ch );
          }
          else
          {
            send_to_char( "Invalid selection.\n\rOptions are magicbomb, thrown, bomb, sling, bow, crossbow, atlatl.\n\r\n\r", ch );
            return FALSE;
          }
          break;
        case 1:
          if (IS_SET(pObj->value[0], ITEM_PROJECTILE_MAGICBOMB))
          {
            send_to_char( "That can't be set on Magic Bombs.\n\r\n\r", ch );
            return FALSE;
          }
          else
          {
            send_to_char( "NUMBER OF DICE SET.\n\r\n\r", ch );
            pObj->value[1] = atoi( argument );
          }
          break;
        case 2:
          if (IS_SET(pObj->value[0], ITEM_PROJECTILE_MAGICBOMB))
          {
            send_to_char( "That can't be set on Magic Bombs.\n\r\n\r", ch );
            return FALSE;
          }
          else
          {
            send_to_char( "TYPE OF DICE SET.\n\r\n\r", ch );
            pObj->value[2] = atoi( argument );
          }
          break;
        case 3:
          if (IS_SET(pObj->value[0], ITEM_PROJECTILE_MAGICBOMB))
          {
            send_to_char( "BOMB SPELL SET.\n\r\n\r", ch );
            pObj->value[3] = skill_lookup( argument );
          }
          else
          {
            send_to_char( "That can only be set on Magic Bombs.\n\r\n\r", ch );
            return FALSE;
          }
          break;
        case 4:
          if (IS_SET(pObj->value[0], ITEM_PROJECTILE_MAGICBOMB))
          {
            send_to_char( "SPELL LEVEL SET.\n\r\n\r", ch );
            pObj->value[4] = atoi( argument );
          }
          else
          {
            send_to_char( "That can only be set on Magic Bombs.\n\r\n\r", ch );
            return FALSE;
          }
          break;
      }
      break;

    case ITEM_SCRY_MIRROR:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_SCRY_MIRROR" );
    return FALSE;
  case 0:
    send_to_char( "MAX CHARGES SET.\n\r\n\r", ch );
    pObj->value[0] = atoi( argument );
    break;
  case 1:
    send_to_char( "CURRENT CHARGES SET.\n\r\n\r", ch );
    pObj->value[1] = atoi( argument );
    break;
  }
    break;

    case ITEM_FISHING_ROD:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_FISHING_ROD" );
    return FALSE;
  case 0:
      cs = atoi( argument );
      if ( (cs < 1) || (cs > 5) )
        send_to_char( "Rod length should be between 1 and 5.\n\r\n\r", ch );
      else
      {
        send_to_char( "ROD LENGTH SET.\n\r\n\r", ch );
        pObj->value[0] = cs;
      }
      break;
    case 1:
      cs = atoi( argument );
      if ( (cs < 1) || (cs > 100) )
        send_to_char( "Rod strength should be between 1 and 100 (% chance of NOT breaking).\n\r\n\r", ch );
      else
      {
        send_to_char( "ROD STRENGTH SET.\n\r\n\r", ch );
        pObj->value[1] = cs;
      }
      break;
    }
    break;

      /* ARMOR for ROM: */

    case ITEM_ARMOR:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_ARMOR" );
    return FALSE;
  case 0:
    send_to_char( "AC PIERCE SET.\n\r\n\r", ch );
    pObj->value[0] = atoi( argument );
    break;
  case 1:
    send_to_char( "AC BASH SET.\n\r\n\r", ch );
    pObj->value[1] = atoi( argument );
    break;
  case 2:
    send_to_char( "AC SLASH SET.\n\r\n\r", ch );
    pObj->value[2] = atoi( argument );
    break;
  case 3:
    send_to_char( "AC EXOTIC SET.\n\r\n\r", ch );
    pObj->value[3] = atoi( argument );
    break;
  }
      break;

      /* WEAPONS changed in ROM */

    case ITEM_WEAPON:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_WEAPON" );
    return FALSE;
  case 0:
    send_to_char( "WEAPON CLASS SET.\n\r\n\r", ch );
    ALT_FLAGVALUE_SET( pObj->value[0], weapon_class, argument );
    break;
  case 1:
    send_to_char( "NUMBER OF DICE SET.\n\r\n\r", ch );
    pObj->value[1] = atoi( argument );
    break;
  case 2:
    send_to_char( "TYPE OF DICE SET.\n\r\n\r", ch );
    pObj->value[2] = atoi( argument );
    break;
  case 3:
    send_to_char( "WEAPON TYPE SET.\n\r\n\r", ch );
    pObj->value[3] = attack_lookup( argument );
    break;
  case 4:
    send_to_char( "SPECIAL WEAPON TYPE TOGGLED.\n\r\n\r", ch );
    ALT_FLAGVALUE_TOGGLE( pObj->value[4], weapon_type2, argument );
    break;
  }
      break;

    case ITEM_PORTAL:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_PORTAL" );
    return FALSE;
              
  case 0:
    send_to_char( "CHARGES SET.\n\r\n\r", ch);
    pObj->value[0] = atoi ( argument );
    break;
  case 1:
    send_to_char( "EXIT FLAGS SET.\n\r\n\r", ch);
    ALT_FLAGVALUE_SET( pObj->value[1], exit_flags, argument );
    break;
  case 2:
    send_to_char( "PORTAL FLAGS SET.\n\r\n\r", ch);
    ALT_FLAGVALUE_SET( pObj->value[2], portal_flags, argument );
    break;
  case 3:
    send_to_char( "EXIT VNUM SET.\n\r\n\r", ch);
    pObj->value[3] = atoi ( argument );
    break;
  }
      break;

    case ITEM_FURNITURE:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_FURNITURE" );
    return FALSE;
              
  case 0:
    send_to_char( "NUMBER OF PEOPLE SET.\n\r\n\r", ch);
    pObj->value[0] = atoi ( argument );
    break;
  case 1:
    send_to_char( "MAX WEIGHT SET.\n\r\n\r", ch);
    pObj->value[1] = atoi ( argument );
    break;
  case 2:
    send_to_char( "FURNITURE FLAGS TOGGLED.\n\r\n\r", ch);
    ALT_FLAGVALUE_TOGGLE( pObj->value[2], furniture_flags, argument );
    break;
  case 3:
    send_to_char( "HEAL BONUS SET.\n\r\n\r", ch);
    pObj->value[3] = atoi ( argument );
    break;
  case 4:
    send_to_char( "MANA BONUS SET.\n\r\n\r", ch);
    pObj->value[4] = atoi ( argument );
    break;
  }
      break;
     
    case ITEM_CONTAINER:
      switch ( value_num )
  {
    int64 value;
    
  default:
    do_function(ch, &do_help, "ITEM_CONTAINER" );
    return FALSE;
  case 0:
    send_to_char( "WEIGHT CAPACITY SET.\n\r\n\r", ch );
    pObj->value[0] = atoi( argument );
    break;
  case 1:
    if ( ( value = flag_value( container_flags, argument ) )!= NO_FLAG )
      TOGGLE_BIT(pObj->value[1], value);
    else
      {
        do_function(ch, &do_help, "ITEM_CONTAINER" );
        return FALSE;
      }
    send_to_char( "CONTAINER TYPE SET.\n\r\n\r", ch );
    break;
  case 2:
    if ( atoi(argument) != 0 )
      {
        if ( !get_obj_index( atoi( argument ) ) )
    {
      send_to_char( "THERE IS NO SUCH ITEM.\n\r\n\r", ch );
      return FALSE;
    }

        if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY )
    {
      send_to_char( "THAT ITEM IS NOT A KEY.\n\r\n\r", ch );
      return FALSE;
    }
      }
    send_to_char( "CONTAINER KEY SET.\n\r\n\r", ch );
    pObj->value[2] = atoi( argument );
    break;
  case 3:
    send_to_char( "CONTAINER MAX WEIGHT SET.\n\r", ch);
    pObj->value[3] = atoi( argument );
    break;
  case 4:
    send_to_char( "WEIGHT MULTIPLIER SET.\n\r\n\r", ch );
    pObj->value[4] = atoi ( argument );
    break;
  }
      break;

    case ITEM_DRINK_CON:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_DRINK" );
    /* OLC        do_help( ch, "liquids" );    */
    return FALSE;
  case 0:
    send_to_char( "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch );
    pObj->value[0] = atoi( argument );
    break;
  case 1:
    send_to_char( "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch );
    pObj->value[1] = atoi( argument );
    break;
  case 2:
    send_to_char( "LIQUID TYPE SET.\n\r\n\r", ch );
    pObj->value[2] = ( liq_lookup(argument) != -1 ?
           liq_lookup(argument) : 0 );
    break;
  case 3:
    send_to_char( "POISON VALUE TOGGLED.\n\r\n\r", ch );
    pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
    break;
  }
      break;

    case ITEM_MONEY_POUCH:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_DRINK" );
    /* OLC        do_help( ch, "liquids" );    */
    return FALSE;
  case 0:
    send_to_char( "WIEGHT MULTIPLIER SET.\n\r\n\r", ch );
    pObj->value[0] = atoi( argument );
    break;
  }
      break;

    case ITEM_FOUNTAIN:
      switch (value_num)
  {
  default:
    do_function(ch, &do_help, "ITEM_FOUNTAIN" );
    /* OLC        do_help( ch, "liquids" );    */
    return FALSE;
  case 0:
    send_to_char( "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch );
    pObj->value[0] = atoi( argument );
    break;
  case 1:
    send_to_char( "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch );
    pObj->value[1] = atoi( argument );
    break;
  case 2:
    send_to_char( "LIQUID TYPE SET.\n\r\n\r", ch );
    pObj->value[2] = ( liq_lookup( argument ) != -1 ?
           liq_lookup( argument ) : 0 );
    break;
  }
      break;
          
    case ITEM_FOOD:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_FOOD" );
    return FALSE;
  case 0:
    send_to_char( "HOURS OF FOOD SET.\n\r\n\r", ch );
    pObj->value[0] = atoi( argument );
    break;
  case 1:
    send_to_char( "HOURS OF FULL SET.\n\r\n\r", ch );
    pObj->value[1] = atoi( argument );
    break;
  case 3:
    send_to_char( "POISON VALUE TOGGLED.\n\r\n\r", ch );
    pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
    break;
  }
      break;

    case ITEM_MONEY:
      switch ( value_num )
  {
  default:
    do_function(ch, &do_help, "ITEM_MONEY" );
    return FALSE;
  case 0:
    send_to_char( "GOLD AMOUNT SET.\n\r\n\r", ch );
    pObj->value[0] = atoi( argument );
    break;
  case 1:
    send_to_char( "SILVER AMOUNT SET.\n\r\n\r", ch );
    pObj->value[1] = atoi( argument );
    break;
  }
      break;
    }

  show_obj_values( ch, pObj );

  return TRUE;
}



OEDIT( oedit_show )
{
  OBJ_INDEX_DATA    *pObj;
  AFFECT_DATA       *paf;
  char buf[MSL];
  int cnt;

  EDIT_OBJ(ch, pObj);

  if ( pObj->area )
    printf_to_char( ch, "{cArea:        {w[{y%6d {w] {c-{x %s\n\r\n\r",
                        pObj->area->vnum, pObj->area->name );
  else
   printf_to_char( ch, "               {R--------- - No Area ---{x\n\r\n\r" );

  printf_to_char(ch, 
    "{cObject:      {w[{y%6d {w]      {cName:        {w[{x %s {w]{x\n\r",
       pObj->vnum, pObj->name );

  printf_to_char(ch, 
    "{cLevel:       {w[{y%6d {w]      {cType:        {w[{x %s {w]{x\n\r",
       pObj->level, flag_string( type_flags, pObj->item_type ) );

  printf_to_char(ch, 
    "{cWeight:      {w[{y%6d {w]      {cMaterial:    {w[{x %s {w]{x\n\r",
       pObj->weight, pObj->material );

  printf_to_char(ch, 
    "{cCost:        {w[{y%6d {w]      {cCondition:   {w[{y %d {w]{x\n\r",
      pObj->cost, pObj->condition );

  if ( !IS_NULLSTR( pObj->clan_name ) )
  {
    printf_to_char(ch, 
      "{cClan:        {w[{x %-9s {w]{x\n\r\n\r",
      pObj->clan_name);
  }

  printf_to_char(ch, "{cWear flags:  {w[{x %s {w]{x\n\r\n\r",
     flag_string( wear_flags, pObj->wear_flags ) );
  printf_to_char(ch, "{cExtra flags: {w[{x %s {w]{x\n\r\n\r",
     flag_string( extra_flags, pObj->extra_flags ) );
 
  if ( pObj->extra_descr )
    {
      EXTRA_DESCR_DATA *ed;

      send_to_char( "{cEx desc kwd:{x", ch );

      for ( ed = pObj->extra_descr; ed; ed = ed->next )
        printf_to_char( ch, "{w [ {x%s{w ]{x", ed->keyword );
      send_to_char( "\n\r", ch );
    }

  printf_to_char(ch, "{cShort desc:{x  %s\n\r{cLong desc:{x\n\r     %s\n\r\n\r",
     pObj->short_descr, pObj->description );

  for ( cnt = 0, paf = pObj->affected; paf; paf = paf->next )
  {
    if ( cnt == 0 )
    {
      send_to_char( "Number Modifier Applies         Affects\n\r", ch );
      send_to_char( "------ -------- --------------- ------------------------\n\r", ch );
    }
    strcpy(buf,"");
    if (paf->bitvector)
    {
      switch(paf->where)
      {
        case TO_AFFECTS:
          mprintf(sizeof(buf),buf,"AFF: %s", affect_bit_name(paf->bitvector));
          break;
        case TO_AFFECTS2:
          mprintf(sizeof(buf),buf,"AF2: %s", affect2_bit_name(paf->bitvector));
          break;
        case TO_OBJECT:
          mprintf(sizeof(buf),buf,"EXT: %s", extra_bit_name(paf->bitvector));
          break;
        case TO_IMMUNE:
          mprintf(sizeof(buf),buf,"IMM: %s", imm_bit_name(paf->bitvector));
          break;
        case TO_RESIST:
          mprintf(sizeof(buf),buf,"RES: %s", imm_bit_name(paf->bitvector));
          break;
        case TO_VULN:
          mprintf(sizeof(buf),buf,"VUL: %s", imm_bit_name(paf->bitvector));
          break;
        default:
          mprintf(sizeof(buf),buf,"ADD: %d", paf->bitvector);
          break;
      }
    }

    printf_to_char(ch, "[%4d] %-8d %-15s %s\n\r",
      cnt, paf->modifier,
      flag_string( apply_flags, paf->location ), buf );

    cnt++;
  }

  show_obj_values( ch, pObj );
  
  if ( pObj->mprogs )
    show_proglist( ch, pObj->mprogs, pObj->vnum, pObj->short_descr );

  return FALSE;
}


/*
 * Need to issue warning if flag isn't valid. -- does so now -- Hugin.
 */
OEDIT( oedit_addaffect )
{
  int64 value;
  OBJ_INDEX_DATA *pObj;
  AFFECT_DATA *pAf;
  char loc[MAX_STRING_LENGTH];
  char mod[MAX_STRING_LENGTH];

  EDIT_OBJ(ch, pObj);

  argument = one_argument( argument, loc );
  one_argument( argument, mod );

  if ( loc[0] == '\0' || mod[0] == '\0' || !is_number( mod ) )
    {
      send_to_char( "Syntax:  addaffect [location] [#xmod]\n\r", ch );
      return FALSE;
    }

  if ( ( value = flag_value( apply_flags, loc ) ) == NO_FLAG ) /* Hugin */
    {
      send_to_char( "Valid affects are:\n\r", ch );
      show_help( ch, "apply" );
      return FALSE;
    }

  pAf             =   new_affect();
  pAf->location   =   value;
  pAf->modifier   =   atoi( mod );
  pAf->where    =   TO_OBJECT;
  pAf->type       =  -1;
  pAf->duration   =  -1;
  pAf->bitvector  =   0;
  pAf->level      =    pObj->level;
  pAf->next       =   pObj->affected;
  pObj->affected  =   pAf;

  send_to_char( "Affect added.\n\r", ch);
  return TRUE;
}

OEDIT( oedit_addapply )
{
  int64 value,bv,typ;
  OBJ_INDEX_DATA *pObj;
  AFFECT_DATA *pAf;
  char loc[MAX_STRING_LENGTH];
  char mod[MAX_STRING_LENGTH];
  char type[MAX_STRING_LENGTH];
  char bvector[MAX_STRING_LENGTH];

  EDIT_OBJ(ch, pObj);

  argument = one_argument( argument, type );
  argument = one_argument( argument, loc );
  argument = one_argument( argument, mod );
  one_argument( argument, bvector );

    if ( type[0] == '\0' || 
         ( typ = flag_value( apply_types, type ) ) == NO_FLAG )
    {
      send_to_char( "Invalid apply type. Valid apply types are:\n\r", ch);
      show_help( ch, "apptype" );
      return FALSE;
    }

    if ( loc[0] == '\0' || 
         ( value = flag_value( apply_flags, loc ) ) == NO_FLAG )
    {
      send_to_char( "Valid applys are:\n\r", ch );
      show_help( ch, "apply" );
      return FALSE;
    }

    if ( bvector[0] == '\0' || 
         ( bv = flag_value( bitvector_type[typ].table, bvector ) ) == NO_FLAG )
    {
      send_to_char( "Invalid bitvector type.\n\r", ch );
      send_to_char( "Valid bitvector types are:\n\r", ch );
      show_help( ch, bitvector_type[typ].help );
      return FALSE;
    }

    if ( mod[0] == '\0' || !is_number( mod ) )
    {
      send_to_char("Syntax: addapply [type] [location] [#xmod] [bitvector]\n\r"
                    , ch );
      return FALSE;
    }

  pAf             =   new_affect();
  pAf->location   =   value;
  pAf->modifier   =   atoi( mod );
  pAf->where    =   apply_types[typ].bit;
  pAf->type        =   -1;
  pAf->duration   =  -1;
  pAf->bitvector  =   bv;
  pAf->level      =    pObj->level;
  pAf->next       =   pObj->affected;
  pObj->affected  =   pAf;

  send_to_char( "Apply added.\n\r", ch);
  return TRUE;
}

/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
OEDIT( oedit_delaffect )
{
  OBJ_INDEX_DATA *pObj;
  AFFECT_DATA *pAf;
  AFFECT_DATA *pAf_next;
  char affect[MAX_STRING_LENGTH];
  int  value;
  int  cnt = 0;

  EDIT_OBJ(ch, pObj);

  one_argument( argument, affect );

  if ( !is_number( affect ) || affect[0] == '\0' )
    {
      send_to_char( "Syntax:  delaffect [#xaffect]\n\r", ch );
      return FALSE;
    }

  value = atoi( affect );

  if ( value < 0 )
    {
      send_to_char( "Only non-negative affect-numbers allowed.\n\r", ch );
      return FALSE;
    }

  if ( !( pAf = pObj->affected ) )
    {
      send_to_char( "OEdit:  Non-existant affect.\n\r", ch );
      return FALSE;
    }

  if( value == 0 )  /* First case: Remove first affect */
    {
      pAf = pObj->affected;
      pObj->affected = pAf->next;
      free_affect( pAf );
    }
  else    /* Affect to remove is not the first */
    {
      while ( ( pAf_next = pAf->next ) && ( ++cnt < value ) )
  pAf = pAf_next;

      if( pAf_next )    /* See if it's the next affect */
  {
    pAf->next = pAf_next->next;
    free_affect( pAf_next );
  }
      else                                 /* Doesn't exist */
  {
    send_to_char( "No such affect.\n\r", ch );
    return FALSE;
  }
    }

  send_to_char( "Affect removed.\n\r", ch);
  return TRUE;
}



OEDIT( oedit_name )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( argument[0] == '\0' )
    {
      send_to_char( "Syntax:  name [string]\n\r", ch );
      return FALSE;
    }

  free_string( pObj->name );
  pObj->name = str_dup( argument,pObj->name );

  send_to_char( "Name set.\n\r", ch);
  return TRUE;
}



OEDIT( oedit_short )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( argument[0] == '\0' )
  {
    send_to_char( "Syntax: short <short desc>\n\r", ch );
    return FALSE;
  }

  free_string( pObj->short_descr );
  pObj->short_descr = str_dup( argument ,pObj->short_descr);
  pObj->short_descr[0] = LOWER( pObj->short_descr[0] );

  send_to_char( "Short description set.\n\r", ch);
  return TRUE;
}



OEDIT( oedit_long )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( argument[0] == '\0' )
  {
    send_to_char( "Syntax: long <string>|none.\n\r", ch );
    return FALSE;
  }
        
  free_string( pObj->description );
  if ( str_cmp( argument, "none" ) )
    {
        pObj->description = str_dup( capitalize_color( argument, FALSE ), pObj->description );
        send_to_char( "Long description set.\n\r", ch);

        if ( !check_punct( argument ) )
            send_to_char( "{RLong descriptions should be punctuated.{x\n\r", ch );
    }
    else
    {
        pObj->description = &str_empty[0];
        send_to_char( "Long description cleared.\n\r", ch);
    }

    return TRUE;

  /*pObj->description = str_dup( argument,pObj->description );
  pObj->description[0] = UPPER( pObj->description[0] );

  send_to_char( "Long description set.\n\r", ch);
  return TRUE;*/
}



bool set_value( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, char *argument, int value )
{
  if ( argument[0] == '\0' )
    {
      set_obj_values( ch, pObj, -1, "" );     /* '\0' changed to "" -- Hugin */
      return FALSE;
    }

  if ( set_obj_values( ch, pObj, value, argument ) )
    return TRUE;

  return FALSE;
}

/*****************************************************************************
 Name:    oedit_values
 Purpose:  Finds the object and sets its value.
 Called by:  The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool oedit_values( CHAR_DATA *ch, char *argument, int value )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( set_value( ch, pObj, argument, value ) )
    return TRUE;

  return FALSE;
}


OEDIT( oedit_value0 )
{
  if ( oedit_values( ch, argument, 0 ) )
    return TRUE;

  return FALSE;
}



OEDIT( oedit_value1 )
{
  if ( oedit_values( ch, argument, 1 ) )
    return TRUE;

  return FALSE;
}



OEDIT( oedit_value2 )
{
  if ( oedit_values( ch, argument, 2 ) )
    return TRUE;

  return FALSE;
}



OEDIT( oedit_value3 )
{
  if ( oedit_values( ch, argument, 3 ) )
    return TRUE;

  return FALSE;
}



OEDIT( oedit_value4 )
{
  if ( oedit_values( ch, argument, 4 ) )
    return TRUE;

  return FALSE;
}
OEDIT( oedit_value5 )
{
  if ( oedit_values( ch, argument, 5 ) )
    return TRUE;

  return FALSE;
}
OEDIT( oedit_value6 )
{
  if ( oedit_values( ch, argument, 6 ) )
    return TRUE;

  return FALSE;
}




OEDIT( oedit_weight )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( argument[0] == '\0' || !is_number( argument ) )
    {
      send_to_char( "Syntax:  weight [number]\n\r", ch );
      return FALSE;
    }

  pObj->weight = atoi( argument );

  send_to_char( "Weight set.\n\r", ch);
  return TRUE;
}

OEDIT( oedit_cost )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( argument[0] == '\0' || !is_number( argument ) )
    {
      send_to_char( "Syntax:  cost [number]\n\r", ch );
      return FALSE;
    }

  pObj->cost = atoi( argument );

  send_to_char( "Cost set.\n\r", ch);
  return TRUE;
}



OEDIT( oedit_create )
{
  OBJ_INDEX_DATA *pObj;
  AREA_DATA *pArea;
  int  value;
  int  iHash;

  value = atoi( argument );
  if ( argument[0] == '\0' || value == 0 )
    {
      send_to_char( "Syntax:  oedit create [vnum]\n\r", ch );
      return FALSE;
    }

  pArea = get_vnum_area( value );
  if ( !pArea )
    {
      send_to_char( "OEdit:  That vnum is not assigned an area.\n\r", ch );
      return FALSE;
    }

  if ( !IS_BUILDER( ch, pArea ) )
    {
      send_to_char( "OEdit:  Vnum in an area you cannot build in.\n\r", ch );
      return FALSE;
    }

  if ((is_revered_obj(value)) && (!IS_IMPLEMENTOR(ch)))
  {
    send_to_char("Sorry that is a Revered and protected Vnum\n\r",ch);
    return FALSE;
  }
  if ( get_obj_index( value ) )
    {
      send_to_char( "OEdit:  Object vnum already exists.\n\r", ch );
      return FALSE;
    }
  if ( !check_area_vnum(pArea, value, ch))
    return FALSE;

  pObj      = new_obj_index();
  pObj->vnum      = value;
  pObj->area      = pArea;
        
  if ( value > top_vnum_obj )
    top_vnum_obj = value;

  iHash      = value % MAX_KEY_HASH;
  pObj->next      = obj_index_hash[iHash];
  obj_index_hash[iHash]  = pObj;
  ch->desc->pEdit    = (void *)pObj;

  send_to_char( "Object Created.\n\r", ch );
  return TRUE;
}



OEDIT( oedit_ed )
{
  OBJ_INDEX_DATA *pObj;
  EXTRA_DESCR_DATA *ed;
  char command[MAX_INPUT_LENGTH];
  char keyword[MAX_INPUT_LENGTH];

  EDIT_OBJ(ch, pObj);

  argument = one_argument( argument, command );
  argument = one_argument( argument, keyword );

  if ( !IS_NULLSTR(command) && !str_cmp( command, "list" ) )
  {
    for ( ed = pObj->extra_descr; ed; ed = ed->next )
      printf_to_char( ch, "[ %s ]:\n\r%s\n\r", ed->keyword, ed->description );
    return TRUE;
  }

  if ( command[0] == '\0' || keyword[0] == '\0' )
    {
      send_to_char( "Syntax:  ed list\n\r", ch );
      send_to_char( "         ed add [keyword]\n\r", ch );
      send_to_char( "         ed delete [keyword]\n\r", ch );
      send_to_char( "         ed edit [keyword]\n\r", ch );
      send_to_char( "         ed format [keyword]\n\r", ch );
      send_to_char( "         ed rename [keyword] [keyword]\n\r", ch );
      return FALSE;
    }

  if ( !str_cmp( command, "add" ) )
    {
      ed                  =   new_extra_descr();
      ed->keyword         =   str_dup( keyword ,ed->keyword);
      ed->next            =   pObj->extra_descr;
      pObj->extra_descr   =   ed;

      string_append( ch, &ed->description, APPEND_AREA, pObj->area );

      return TRUE;
    }

  if ( !( ed = ed_lookup( keyword, pObj->extra_descr ) ) )
  {
    send_to_char( "Oedit: Extra description keyword not found.\n\r",ch );
    return FALSE;
  }

  if ( !str_cmp( command, "edit" ) )
    {
      string_append( ch, &ed->description, APPEND_AREA, pObj->area );
      return TRUE;
    }

  if ( !str_cmp( command, "delete" ) )
    {
      EXTRA_DESCR_DATA *ped = NULL;

      for ( ped = pObj->extra_descr; ped; ped = ped->next )
  {
    if ( ped->next == ed )
      break;
  }
    if ( ped )
        ped->next = ed->next;
    else
      pObj->extra_descr = ed->next;

      free_extra_descr( ed );

      send_to_char( "Extra description deleted.\n\r", ch );
      return TRUE;
    }


  if ( !str_cmp( command, "format" ) )
    {
      ed->description = format_string( ed->description );

      send_to_char( "Extra description formatted.\n\r", ch );
      return TRUE;
    }

  if ( !str_cmp( command, "rename" ) )
  {

    if ( !one_argument( argument, keyword ) || keyword[0] == '\0' )
    {
      send_to_char( "Cannot change keyword to NULL.\n\r", ch );
      return FALSE;
    }

    free_string( ed->keyword );
    ed->keyword = str_dup( keyword, ed->keyword );
    return TRUE;
  }

  oedit_ed( ch, "" );
  return FALSE;
}





/* ROM object functions : */

OEDIT( oedit_extra )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
    OBJ_INDEX_DATA *pObj;
    int64 value;

    if ( argument[0] != '\0' )
    {
  EDIT_OBJ(ch, pObj);

  if ( ( value = flag_value( extra_flags, argument ) ) != NO_FLAG )
  {
      TOGGLE_BIT(pObj->extra_flags, value);

      send_to_char( "Extra flag toggled.\n\r", ch);
      return TRUE;
  }
    }

    send_to_char( "Syntax:  extra [flag]\n\r"
      "Type '? extra' for a list of flags.\n\r", ch );
    return FALSE;
}


OEDIT( oedit_wear )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
    OBJ_INDEX_DATA *pObj;
    int64 value;

     if ( argument[0] != '\0' )
    {
  EDIT_OBJ(ch, pObj);

  if ( ( value = flag_value( wear_flags, argument ) ) != NO_FLAG )
  {
      TOGGLE_BIT(pObj->wear_flags, value);

      send_to_char( "Wear flag toggled.\n\r", ch);
      return TRUE;
  }
    }

    send_to_char( "Syntax:  wear [flag]\n\r"
      "Type '? wear' for a list of flags.\n\r", ch );
    return FALSE;
}


OEDIT( oedit_type )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
    OBJ_INDEX_DATA *pObj;
    int64 value;

    if ( argument[0] != '\0' )
    {
  EDIT_OBJ(ch, pObj);

  if ( ( value = flag_value( type_flags, argument ) ) != NO_FLAG )
  {
      pObj->item_type = value;

      send_to_char( "Type set.\n\r", ch);

      /*
       * Clear the values.
       */
      pObj->value[0] = 0;
      pObj->value[1] = 0;
      pObj->value[2] = 0;
      pObj->value[3] = 0;
      pObj->value[4] = 0;     /* ROM */

      return TRUE;
  }
    }

    send_to_char( "Syntax:  type [flag]\n\r"
      "Type '? type' for a list of flags.\n\r", ch );
    return FALSE;
}

OEDIT( oedit_material )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
  send_to_char( "Syntax:  material [string]\n\r", ch );
  return FALSE;
    }

    free_string( pObj->material );
    pObj->material = str_dup( argument,pObj->material );

    send_to_char( "Material set.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_level )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
  send_to_char( "Syntax:  level [number]\n\r", ch );
  return FALSE;
    }

    pObj->level = atoi( argument );

    send_to_char( "Level set.\n\r", ch);
    return TRUE;
}



OEDIT( oedit_condition )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0'
    && ( value = atoi (argument ) ) >= 0
    && ( value <= 100 ) )
    {
  EDIT_OBJ( ch, pObj );

  pObj->condition = value;
  send_to_char( "Condition set.\n\r", ch );

  return TRUE;
    }

    send_to_char( "Syntax:  condition [number]\n\r"
      "Where number can range from 0 (ruined) to 100 (perfect).\n\r",
      ch );
    return FALSE;
}

OEDIT( oedit_clan )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( argument[0] == '\0' )
  {
    send_to_char( "Syntax:  clan [clan name]\n\r", ch );
    return FALSE;
  }

  if ( !str_cmp( argument, "none" ) )
  {
    free_string( pObj->clan_name );
    pObj->clan_name = &str_empty[0];
    send_to_char ( "Clan removed.\n\r", ch);
    return TRUE;
  }

  if (!is_valid_clan_name(argument))
  {
    send_to_char ( "No such clan exists.\n\r", ch);
    return FALSE;
  }

  free_string( pObj->clan_name );
  pObj->clan_name = str_dup( argument ,pObj->clan_name);
  send_to_char ( "Clan set.\n\r", ch);
  return TRUE;
}


/*
 * Mobile Editor Functions.
 */
MEDIT( medit_show )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    char *tmp;

    if ( pMob->area )
        printf_to_char( ch, 
        "{cArea:        {W[{Y%6d {W] {c- {x %s {c( Max in area: {Y%d {c){x\n\r\n\r",
        pMob->area->vnum, pMob->area->name, pMob->max_count );
    else
        printf_to_char( ch, "       ----    NO AREA    ----\n\r\n\r" );

    printf_to_char( ch, 
                    "{cMob:  {W       [ {Y%5d{W ]      {cName:    {W[{x %-15s {W]{x\n\r", 
                    pMob->vnum, pMob->player_name );
    printf_to_char( ch,
                    "{cLevel:       {W[ {Y%5d{W ]      {cRace:    {W[{x %-15s {W]{x\n\r",
                    pMob->level, race_table[pMob->race].name );
    printf_to_char( ch,
                    "{cWealth:      {W[ {Y%5d{W ]      {cSex:     {W[{x %-15s {W]{x\n\r",
                    pMob->wealth, 
                    pMob->sex == SEX_MALE   ? "male" :
                    pMob->sex == SEX_FEMALE ? "female" :
                    pMob->sex == 3          ? "random" :
                                              "neutral" );

    printf_to_char( ch,
                    "{cHitroll:     {W[ {Y%5d {W]      {cDam type:{W[ {w%-15s {W]\n\r",
                    pMob->hitroll, attack_table[pMob->dam_type].name );

    printf_to_char( ch,
                    "{cFrag number: {W[ {Y%5d {W]      {cDam Name:{W[ {w%-15s {W]\n\r",
                    pMob->frag_number,
                    attack_table[pMob->dam_type].noun );

    printf_to_char( ch,
                    "{cAlign:       {W[ {Y%5d {W]               [{x %s       {W]{x\n\r\n\r",
                    pMob->alignment, get_align_str( pMob->alignment ) );


    printf_to_char( ch, 
     "           {cHit dice            Mana dice            Dam dice{x\n\r" );
    printf_to_char( ch,
    "     -------------------- -------------------- --------------------\n\r" );
    printf_to_char( ch, 
    "     |{Y%4d{cd{Y%3d {c+{Y%7d {x| |{Y%4d{cd{Y%3d {c+{Y%7d {x| |{Y%4d{cd{Y%3d {c+{Y%7d {x|\n\r" ,                    pMob->hit[DICE_NUMBER], pMob->hit[DICE_TYPE], pMob->hit[DICE_BONUS],            pMob->mana[DICE_NUMBER], pMob->mana[DICE_TYPE], pMob->mana[DICE_BONUS],
    pMob->damage[DICE_NUMBER],pMob->damage[DICE_TYPE],pMob->damage[DICE_BONUS]);
    printf_to_char( ch,
    "{cMin:{x |   {Y%9d{x      | |   {Y%9d{x      | |   {Y%9d{x      |\n\r",
    DICE_MIN( pMob->hit[DICE_NUMBER], pMob->hit[DICE_TYPE] ) + 
              pMob->hit[DICE_BONUS],
    DICE_MIN( pMob->mana[DICE_NUMBER], pMob->mana[DICE_TYPE] ) +
              pMob->mana[DICE_BONUS],
    DICE_MIN( pMob->damage[DICE_NUMBER], pMob->damage[DICE_TYPE] ) +
              pMob->damage[DICE_BONUS] );
    printf_to_char( ch,
    "{cAvg:{x |   {Y%9d{x      | |   {Y%9d{x      | |   {Y%9d{x      |\n\r",
    DICE_AVG( pMob->hit[DICE_NUMBER], pMob->hit[DICE_TYPE] ) +
              pMob->hit[DICE_BONUS],
    DICE_AVG( pMob->mana[DICE_NUMBER], pMob->mana[DICE_TYPE] ) +
              pMob->mana[DICE_BONUS],
    DICE_AVG( pMob->damage[DICE_NUMBER], pMob->damage[DICE_TYPE] ) +
              pMob->damage[DICE_BONUS] );   
    printf_to_char( ch,
    "{cMax:{x |   {Y%9d{x      | |   {Y%9d{x      | |   {Y%9d{x      |\n\r",
    DICE_MAX( pMob->hit[DICE_NUMBER], pMob->hit[DICE_TYPE] ) +
              pMob->hit[DICE_BONUS],
    DICE_MAX( pMob->mana[DICE_NUMBER], pMob->mana[DICE_TYPE] ) +
              pMob->mana[DICE_BONUS],
    DICE_MAX( pMob->damage[DICE_NUMBER], pMob->damage[DICE_TYPE] ) +
              pMob->damage[DICE_BONUS] );
    printf_to_char( ch,
    "     -------------------- -------------------- --------------------\n\r\n\r" );

    printf_to_char( ch,
    "{cArmor:       {W[ {cpierce:{Y%4d {cbash:{Y%4d {cslash:{Y%4d {cmagic:{Y%4d {W]{x\n\r\n\r",
    pMob->ac[AC_PIERCE], pMob->ac[AC_BASH],
    pMob->ac[AC_SLASH], pMob->ac[AC_EXOTIC] );

    printf_to_char( ch, "{cAct:         {W[ {w%s {W]{x\n\r",
    flag_string( act_flags, pMob->act ) );

    printf_to_char( ch, "{cAct2:        {W[ {w%s {W]{x\n\r",
    flag_string( act2_flags, pMob->act2 ) );

    printf_to_char( ch, "{cAffected by  {W[ {w%s {W]{x\n\r",
    flag_string( affect_flags, pMob->affected_by ) );

    printf_to_char( ch, "{cAff2 by      {W[ {w%s {W]{x\n\r",
    flag_string( affect2_flags, pMob->affected2_by ) );

    printf_to_char(ch, "{cImm:         {W[ {w%s {W]{x\n\r",
  flag_string( imm_flags, pMob->imm_flags ) );

    printf_to_char(ch, "{cRes:         {W[ {w%s {W]{x\n\r",
  flag_string( res_flags, pMob->res_flags ) );

    printf_to_char(ch, "{cVuln:        {W[ {w%s {W]{x\n\r",
  flag_string( vuln_flags, pMob->vuln_flags ) );

    printf_to_char(ch, "{cOff:         {W[ {w%s {W]{x\n\r",
  flag_string( off_flags,  pMob->off_flags ) );

    printf_to_char( ch, "{cForm:        {W[ {w%s {W]{x\n\r",
    flag_string( form_flags, pMob->form ) );

    printf_to_char( ch, "{cParts:       {W[ {w%s {W]{x\n\r\n\r",
    flag_string( part_flags, pMob->parts ) );


    printf_to_char(ch, "{cSize:        {W[ {w%10s {W]{x   ",
  flag_string( size_flags, pMob->size ) );

    printf_to_char(ch, "{cMaterial:    {W[ {w%s {W]{x\n\r",
        pMob->material );

    printf_to_char(ch, "{cStart pos.   {W[ {w%10s {W]{x   ",
  flag_string( position_flags, pMob->start_pos ) );

    printf_to_char(ch, "{cDefault pos  {W[ {w%s {W]{x\n\r",
  flag_string( position_flags, pMob->default_pos ) );

    if ( pMob->group )
        printf_to_char( ch, "Group: {W[{Y%6d {W]{x\n\r", pMob->group );

/* ROM values end */

    if ( pMob->spec_fun )
    {
  printf_to_char(ch, "Spec fun:    [%s]\n\r",  spec_name( pMob->spec_fun ) );
    }

    printf_to_char(ch, "{cShort descr:{x %s\n\r{cLong descr:{x\n\r%s",
  pMob->short_descr,
  pMob->long_descr );

    tmp = pMob->long2_descr;
    if ( tmp && tmp[0] && tmp[0] != ' ' && strcmp( "(null)", tmp ) )
        printf_to_char( ch, "{cLong desc2:{x\n\r%s", tmp );

    printf_to_char(ch, "{cDescription:{x\n\r%s\n", pMob->description );

    tmp = pMob->path;
    if ( tmp && tmp[0] && tmp[0] != ' ' && strcmp( "(null)", tmp ) )
        printf_to_char( ch, "{cPath:{x\n\r%s\n", tmp );

    if (IS_SET(pMob->act2,ACT2_MOUNTABLE))
      printf_to_char( ch, "{cMax_riders:{x %d\n", pMob->max_riders );

    if ( pMob->pShop )
    {
  SHOP_DATA *pShop;
  int iTrade;

  pShop = pMob->pShop;

  printf_to_char(ch,
    "Shop data for [%5d]:\n\r"
    "  Markup for purchaser: %d%%\n\r"
    "  Markdown for seller:  %d%%\n\r",
      pShop->keeper, pShop->profit_buy, pShop->profit_sell );
  printf_to_char(ch, "  Hours: %d to %d.\n\r",
      pShop->open_hour, pShop->close_hour );

  for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
  {
      if ( pShop->buy_type[iTrade] != 0 )
      {
    if ( iTrade == 0 ) {
        send_to_char( "  Number Trades Type\n\r", ch );
        send_to_char( "  {c------ -----------{x\n\r", ch );
    }
    printf_to_char(ch, "  [%4d] %s\n\r", iTrade,
        flag_string( type_flags, pShop->buy_type[iTrade] ) );
      }
  }
    }

    if ( pMob->mprogs )
        show_proglist( ch, pMob->mprogs, pMob->vnum, pMob->short_descr );

    return FALSE;
}

MEDIT( medit_create )
{
  MOB_INDEX_DATA *pMob;
  AREA_DATA *pArea;
  int  value;
  int  iHash;

  value = atoi( argument );
  if ( argument[0] == '\0' || value == 0 )
  {
    send_to_char( "Syntax:  medit create [vnum]\n\r", ch );
    return FALSE;
  }

  pArea = get_vnum_area( value );

  if ( !pArea )
  {
    send_to_char( "MEdit:  That vnum is not assigned an area.\n\r", ch );
    return FALSE;
  }

  if ( !IS_BUILDER( ch, pArea ) )
  {
    send_to_char( "MEdit:  Vnum in an area you cannot build in.\n\r", ch );
    return FALSE;
  }

  if ( get_mob_index( value ) )
  {
    send_to_char( "MEdit:  Mobile vnum already exists.\n\r", ch );
    return FALSE;
  }
  
  if ( ( is_revered_mob( value ) ) && ( !IS_IMPLEMENTOR( ch ) ) )
  {
  send_to_char("Sorry that is a Revered and protected Vnum\n\r",ch);
  return FALSE;
  }
  if ( !check_area_vnum( pArea, value, ch ) )
    return FALSE;

  pMob      = new_mob_index();
  pMob->vnum  = value;
  pMob->area  = pArea;
        
  if ( value > top_vnum_mob )
    top_vnum_mob = value;        

  pMob->act            = ACT_IS_NPC;
  iHash              = value % MAX_KEY_HASH;
  pMob->next      = mob_index_hash[iHash];
  mob_index_hash[iHash]  = pMob;
  ch->desc->pEdit    = (void *)pMob;

  send_to_char( "Mobile Created.\n\r", ch );
  return TRUE;
}

MEDIT( medit_spec )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB( ch, pMob );

  if ( argument[0] == '\0' )
  {
    send_to_char( "Syntax:  spec [special function]\n\r", ch );
    return FALSE;
  }


  if ( !str_cmp( argument, "none" ) )
  {
    pMob->spec_fun = NULL;

    send_to_char( "Spec removed.\n\r", ch);
    return TRUE;
  }

  if ( spec_lookup( argument ) )
  {
    pMob->spec_fun = spec_lookup( argument );
    send_to_char( "Spec set.\n\r", ch);
    return TRUE;
  }

  send_to_char( "MEdit: No such special function.\n\r", ch );
  return FALSE;
}

MEDIT( medit_damtype )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
  send_to_char( "Syntax:  damtype [damage message]\n\r", ch );
  send_to_char( "for a help listing for weapons use '? weapon'.\n\r", ch );
  return FALSE;
    }

    pMob->dam_type = attack_lookup(argument);
    send_to_char( "Damage type set.\n\r", ch);
    return TRUE;
}


MEDIT( medit_align )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
  send_to_char( "Syntax:  alignment [number]\n\r", ch );
  return FALSE;
    }

    pMob->alignment = atoi( argument );

    send_to_char( "Alignment set.\n\r", ch);
    return TRUE;
}



MEDIT( medit_level )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
  send_to_char( "Syntax:  level [number]\n\r", ch );
  return FALSE;
    }

    pMob->level = atoi( argument );
    // set hit/dam/hp/mana and stuff after level
    send_to_char( "Level set.\n\r", ch);
    return TRUE;
}



MEDIT( medit_desc )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
  string_append( ch, &pMob->description, APPEND_AREA, pMob->area );
  return TRUE;
    }

    send_to_char( "Syntax:  desc    - line edit\n\r", ch );
    return FALSE;
}




MEDIT( medit_long )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' )
  {
    send_to_char( "Syntax: long <string>\n\r        long <none>\n\r", ch );
    return FALSE;
  }

  if ( !str_prefix( argument, "none" ) )
  {
    free_string( pMob->long_descr );
    send_to_char( "Long description cleared.\n\r", ch );
    pMob->long_descr = &str_empty[0];
    return TRUE;
  }

  free_string( pMob->long_descr );
  strcat( argument, "\n\r" );
  pMob->long_descr = str_dup( argument,pMob->long_descr );
  pMob->long_descr[0] = UPPER( pMob->long_descr[0]  );

  send_to_char( "Long description set.\n\r", ch);
  return TRUE;
}

MEDIT( medit_long2 )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB( ch, pMob );

  if ( argument[0] == '\0' )
  {
    send_to_char( "Syntax: long2 <string>\n\r        long2 <none>\n\r", ch );
    return FALSE;
  }

  if ( !str_prefix( argument, "none" ) )
  {
    free_string( pMob->long2_descr );
    send_to_char( "Long description 2 cleared.\n\r", ch );
    pMob->long2_descr = &str_empty[0];
    return TRUE;
  }

  free_string( pMob->long2_descr );
  strcat( argument, "\n\r" );
  pMob->long2_descr = str_dup( argument, pMob->long2_descr );
  pMob->long2_descr[0] = UPPER( pMob->long2_descr[0]  );

  send_to_char( "Long description 2 set.\n\r", ch);
  return TRUE;
}

MEDIT( medit_path )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB( ch, pMob );

  if ( argument[0] == '\0' )
  {
    send_to_char( "Syntax: path <string>\n\r        path <none>\n\r", ch );
    return FALSE;
  }

  if ( !str_prefix( argument, "none" ) )
  {
    free_string( pMob->path );
    send_to_char( "Path cleared.\n\r", ch );
    pMob->path = &str_empty[0];
    return TRUE;
  }

  free_string( pMob->path );
  pMob->path = str_dup( argument, pMob->path );

  send_to_char( "Path set.\n\r", ch);
  return TRUE;
}

MEDIT( medit_max_riders )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB( ch, pMob );

  if ( argument[0] == '\0' )
  {
    send_to_char( "Syntax: max_riders <number>\n\r", ch );
    return FALSE;
  }

  pMob->max_riders = atoi( argument );

  send_to_char( "Max Riders set.\n\r", ch);
  return TRUE;
}

MEDIT( medit_short )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' ) {
    send_to_char( "String is reset to NOTHING.\n\r", ch );
    strcpy(argument,"NOTHING");
  }

  free_string( pMob->short_descr );
  pMob->short_descr = str_dup( argument ,pMob->short_descr);

  send_to_char( "Short description set.\n\r", ch);
  return TRUE;
}



MEDIT( medit_name )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
  send_to_char( "Syntax:  name [string]\n\r", ch );
  return FALSE;
    }

    free_string( pMob->player_name );
    pMob->player_name = str_dup( argument ,pMob->player_name);

    send_to_char( "Name set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_shop )
{
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );
    argument = one_argument( argument, arg1 );

    EDIT_MOB(ch, pMob);

    if ( command[0] == '\0' )
    {
  send_to_char( "Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch );
  send_to_char( "         shop profit [#xbuying%] [#xselling%]\n\r", ch );
  send_to_char( "         shop type [#x0-4] [item type]\n\r", ch );
  send_to_char( "         shop assign\n\r", ch );
  send_to_char( "         shop remove\n\r", ch );
  return FALSE;
    }


    if ( !str_cmp( command, "hours" ) )
    {
  if ( arg1[0] == '\0' || !is_number( arg1 )
  || argument[0] == '\0' || !is_number( argument ) )
  {
      send_to_char( "Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch );
      return FALSE;
  }

  if ( !pMob->pShop )
  {
      send_to_char( "MEdit:  There is no shop Assigned.\n\r", ch );
      return FALSE;
  }

  pMob->pShop->open_hour = atoi( arg1 );
  pMob->pShop->close_hour = atoi( argument );

  send_to_char( "Shop hours set.\n\r", ch);
  return TRUE;
    }


    if ( !str_cmp( command, "profit" ) )
    {
  if ( arg1[0] == '\0' || !is_number( arg1 )
  || argument[0] == '\0' || !is_number( argument ) )
  {
      send_to_char( "Syntax:  shop profit [#xbuying%] [#xselling%]\n\r", ch );
      return FALSE;
  }

  if ( !pMob->pShop )
  {
      send_to_char( "MEdit:  There is no shop Assigned.\n\r", ch );
      return FALSE;
  }

  pMob->pShop->profit_buy     = atoi( arg1 );
  pMob->pShop->profit_sell    = atoi( argument );

  send_to_char( "Shop profit set.\n\r", ch);
  return TRUE;
    }


    if ( !str_cmp( command, "type" ) )
    {
  int64 value;

  if ( arg1[0] == '\0' || !is_number( arg1 )
  || argument[0] == '\0' )
  {
      send_to_char( "Syntax:  shop type [#x0-4] [item type]\n\r", ch );
      return FALSE;
  }

  if ( atoi( arg1 ) >= MAX_TRADE )
  {
      printf_to_char(ch, "MEdit:  May sell %d items max.\n\r", MAX_TRADE );
      return FALSE;
  }

  if ( !pMob->pShop )
  {
      send_to_char( "MEdit:  There is no shop Assigned.\n\r", ch );
      return FALSE;
  }

  if ( ( value = flag_value( type_flags, argument ) ) == NO_FLAG )
  {
      send_to_char( "MEdit:  That type of item is not known.\n\r", ch );
      return FALSE;
  }

  pMob->pShop->buy_type[atoi( arg1 )] = value;

  send_to_char( "Shop type set.\n\r", ch);
  return TRUE;
    }

    /* shop assign && shop delete by Phoenix */

    if ( !str_prefix(command, "assign") )
    {
      if ( pMob->pShop )
      {
          send_to_char("Mob already has a shop assigned to it.\n\r", ch);
          return FALSE;
      }

      pMob->pShop    = new_shop();
      if ( !shop_first )
          shop_first  = pMob->pShop;
      if ( shop_last )
        shop_last->next  = pMob->pShop;
      shop_last    = pMob->pShop;

      pMob->pShop->keeper  = pMob->vnum;

      send_to_char("New shop assigned to mobile.\n\r", ch);
      return TRUE;
    }

    if ( !str_prefix( command, "remove" ) )
    {
    SHOP_DATA *pShop;

    pShop     = pMob->pShop;
    pMob->pShop  = NULL;

    if ( pShop == shop_first )
    {
      if (pShop == NULL)
        {
        send_to_char("ER??  BUG LOG THAT a SHOP was NULL!!\n\r",ch);
        bugf("ER??  BUG LOG THAT a SHOP was NULL!! -%s-",interp_cmd);
        return FALSE;
      }
      if ( !pShop->next )
    {
      shop_first = NULL;
      shop_last = NULL;
    }
    else
      shop_first = pShop->next;
    }
    else
    {
    SHOP_DATA *ipShop;

    if ( pShop == NULL )
      {
      send_to_char("ER??  BUG LOG THAT a SHOP was NULL!!\n\r",ch);
      bugf("Part 2ER??  BUG LOG THAT a SHOP was NULL!! -%s-",interp_cmd);
      return FALSE;
    }
    for ( ipShop = shop_first; ipShop; ipShop = ipShop->next )
    {
      if ( ipShop->next == pShop )
      {
        if ( !pShop->next )
        {
          shop_last = ipShop;
          shop_last->next = NULL;
        }
        else
          ipShop->next = pShop->next;
      }
    }
  }

  free_shop( pShop );

  send_to_char( "Mobile is no longer a shopkeeper.\n\r", ch );
  return TRUE;
  }

  medit_shop( ch, "" );
  return FALSE;
}

/* ROM medit functions: */

MEDIT( medit_sex ) /* Moved out of medit() due to naming conflicts -- Hugin */
{
    MOB_INDEX_DATA *pMob;
    int64 value;

    if ( argument[0] )
    {
  EDIT_MOB( ch, pMob );

  if ( ( value = flag_value( sex_flags, argument ) ) != NO_FLAG )
  {
      pMob->sex = value;

      send_to_char( "Sex set.\n\r", ch);
      return TRUE;
  }
    }

    send_to_char( "Syntax: sex [sex]\n\r"
      "Type '? sex' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_maxcount ) /* Moved out of medit() - naming conflicts -- Hugin */
{
  MOB_INDEX_DATA *pMob;

  if ( argument[0] )
    {
      EDIT_MOB( ch, pMob );

      pMob->max_count = atoi(argument);

      send_to_char( "Max Count set.\n\r", ch);
      return TRUE;
    }

  send_to_char( "Syntax: maxcount [count]\n\r",ch);
  return FALSE;
}


MEDIT( medit_act )          /* Moved out of medit() due to naming conflicts -- Hugin */
{
    MOB_INDEX_DATA *pMob;
    int64 value;

    if ( argument[0] )
    {
  EDIT_MOB( ch, pMob );

  if ( ( value = flag_value( act_flags, argument ) ) != NO_FLAG )
  {
      pMob->act ^= value;
      SET_BIT( pMob->act, ACT_IS_NPC );

      send_to_char( "Act flag toggled.\n\r", ch);
      return TRUE;
  }
    }

    send_to_char( "Syntax: act [flag]\n\r"
      "Type '? act' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_act2 )          /* ACT2 created Oct, 2003 by Robert Leonard */
{
    MOB_INDEX_DATA *pMob;
    int64 value;

    if ( argument[0] )
    {
        EDIT_MOB( ch, pMob );

        if ( ( value = flag_value( act2_flags, argument ) ) != NO_FLAG )
        {
            pMob->act2 ^= value;

            send_to_char( "Act2 flag toggled.\n\r", ch);
            return TRUE;
        }
    }

    send_to_char( "Syntax: act2 [flag]\n\r"
                  "Type '? act2' for a list of flags.\n\r", ch );
    return FALSE;
}

/*MEDIT( medit_saffect )
{
    MOB_INDEX_DATA *pMob;
    int64 value;

    if ( argument[0] != '\0' )
    {
        EDIT_MOB( ch, pMob );

        if ( ( value = flag_value( saff_flags, argument ) ) != NO_FLAG )
        {
            pMob->saffected_by
*/

MEDIT( medit_affect ) /* Moved out of medit() - naming conflicts -- Hugin */
{
    MOB_INDEX_DATA *pMob;
    int64 value;

    if ( argument[0] )
    {
  EDIT_MOB( ch, pMob );

  if ( ( value = flag_value( affect_flags, argument ) ) != NO_FLAG )
  {
      pMob->affected_by ^= value;

      send_to_char( "Affect flag toggled.\n\r", ch);
      return TRUE;
  }
    }

    send_to_char( "Syntax: affect [flag]\n\r"
      "Type '? affect' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_affect2 )      /* AFF2 */
{
    MOB_INDEX_DATA *pMob;
    int64 value;

    if ( argument[0] )
    {
    EDIT_MOB( ch, pMob );

    if ( !str_cmp( argument, "none" ) )
    {
    pMob->affected2_by = 0;
    send_to_char( "Affect2 flag cleared.\n\r", ch );
    return TRUE;
    }

    if ( ( value = flag_value( affect2_flags, argument ) ) != NO_FLAG )
    {
    pMob->affected2_by ^= value;
    send_to_char( "Affect2 flag toggled.\n\r", ch);
    return TRUE;
    } 
    }
    
    send_to_char( "Syntax:  aff2 none|<flags>\n\r"
        "Type '? AFFECT2' for a list of flags.\n\r", ch );
    return FALSE;
}


MEDIT( medit_ac )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    int pierce, bash, slash, exotic;

    do   /* So that I can use break and send the syntax in one place */
    {
    if ( argument[0] == '\0' )  break;

    EDIT_MOB( ch, pMob );
    argument = one_argument( argument, arg );

    if ( !is_number( arg ) )  break;
    pierce = atoi( arg );
    argument = one_argument( argument, arg );

    if ( arg[0] )
    {
      if ( !is_number( arg ) )  break;
      bash = atoi( arg );
      argument = one_argument( argument, arg );
    }
    else
      bash = pMob->ac[AC_BASH];

    if ( arg[0] )
    {
      if ( !is_number( arg ) )  break;
      slash = atoi( arg );
      argument = one_argument( argument, arg );
    }
    else
      slash = pMob->ac[AC_SLASH];

    if ( arg[0] )
    {
      if ( !is_number( arg ) )  break;
      exotic = atoi( arg );
    }
    else
      exotic = pMob->ac[AC_EXOTIC];

    pMob->ac[AC_PIERCE] = pierce;
    pMob->ac[AC_BASH]   = bash;
    pMob->ac[AC_SLASH]  = slash;
    pMob->ac[AC_EXOTIC] = exotic;
  
    send_to_char( "Ac set.\n\r", ch );
    return TRUE;
    } 
    while ( FALSE );    /* Just do it once.. */

    send_to_char( 
        "Syntax:  armor [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n\r"
    "help MOB_AC  gives a list of reasonable ac-values.\n\r", ch );
    return FALSE;
}

MEDIT( medit_form )
{
    MOB_INDEX_DATA *pMob;
    int64 value;

    if ( argument[0] )
    {
  EDIT_MOB( ch, pMob );

  if ( ( value = flag_value( form_flags, argument ) ) != NO_FLAG )
  {
      pMob->form ^= value;
      send_to_char( "Form toggled.\n\r", ch );
      return TRUE;
  }
    }

    send_to_char( "Syntax: form [flags]\n\r"
      "Type '? form' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_part )
{
    MOB_INDEX_DATA *pMob;
    int64 value;

    if ( argument[0] )
    {
      EDIT_MOB( ch, pMob );

      if (!str_cmp(argument,"none"))
      {
        pMob->parts = 0;
        send_to_char( "Parts cleared.\n\r", ch );
        return TRUE;
      }
      else
      {
        if ( ( value = flag_value( part_flags, argument ) ) != NO_FLAG )
        {
          pMob->parts ^= value;
          send_to_char( "Parts toggled.\n\r", ch );
          return TRUE;
        }
      }
    }

    send_to_char( "Syntax: part [flag]\n\rType '? part' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_imm )
{
    MOB_INDEX_DATA *pMob;
    int64 value;

    if ( argument[0] )
    {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( imm_flags, argument ) ) != NO_FLAG )
    {
      pMob->imm_flags ^= value;
      send_to_char( "Immunity toggled.\n\r", ch );
      return TRUE;
    }
    }

    send_to_char( "Syntax: imm [flags]\n\r"
      "Type '? imm' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_res )
{
    MOB_INDEX_DATA *pMob;
    int64 value;

    if ( argument[0] )
    {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( res_flags, argument ) ) != NO_FLAG )
    {
      pMob->res_flags ^= value;
      send_to_char( "Resistance toggled.\n\r", ch );
      return TRUE;
    }
    }

    send_to_char( "Syntax: res [flags]\n\r"
      "Type '? res' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_vuln )
{
    MOB_INDEX_DATA *pMob;
    int64 value;

    if ( argument[0] )
    {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( vuln_flags, argument ) ) != NO_FLAG )
    {
      pMob->vuln_flags ^= value;
      send_to_char( "Vulnerability toggled.\n\r", ch );
      return TRUE;
    }
    }

    send_to_char( "Syntax: vuln [flags]\n\r"
      "Type '? vuln' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_material )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
    send_to_char( "Syntax:  material [string]\n\r", ch );
    return FALSE;
    }

    free_string( pMob->material );
    pMob->material = str_dup( argument, pMob->material );

    send_to_char( "Material set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_off )
{
    MOB_INDEX_DATA *pMob;
    int64 value;

    if ( argument[0] )
    {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( off_flags, argument ) ) != NO_FLAG )
    {
      pMob->off_flags ^= value;
      send_to_char( "Offensive behaviour toggled.\n\r", ch );
      return TRUE;
    }
    }

    send_to_char( "Syntax: off [flags]\n\r"
      "Type '? off' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_size )
{
    MOB_INDEX_DATA *pMob;
    int64 value;

    if ( argument[0] )
    {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( size_flags, argument ) ) != NO_FLAG )
    {
      pMob->size = value;
      send_to_char( "Size set.\n\r", ch );
      return TRUE;
    }
    }

    send_to_char( "Syntax: size [size]\n\r"
      "Type '? size' for a list of sizes.\n\r", ch );
    return FALSE;
}

MEDIT( medit_hitdice )
{
  static char syntax[] = "Syntax:  hitdice <number> d <type> + <bonus>\n\r         hitdice avg\n\r";
  char *num, *type, *bonus, *cp;
  int mlvl, avehps, flucperc, fluc, dienum, dietype, diebonus, dice_ave;
  int lower_fluc, higher_fluc;
  MOB_INDEX_DATA *pMob;

  EDIT_MOB( ch, pMob );

  if ( argument[0] == '\0' )
  {
      send_to_char( syntax, ch );
      return FALSE;
  }

  if ( (!str_cmp(argument, "avg") ) )   /* Average routine done by Sartan February 2001 */
    {
      mlvl = pMob->level;
      if (mlvl < 1)
  {
    send_to_char("Mob level must be greater than 0 to compute average hitpoints.\n\r", ch);
    return FALSE;
  }
      if (mlvl < 51) { avehps = 2.0 * mlvl * mlvl; }
      else { avehps = 2.5 * mlvl * mlvl; }
    if (mlvl <= 2) avehps *= 1.5;

      flucperc = number_range( 5, 10 );
    if (mlvl <= 5) flucperc *= 5;
    if ( (mlvl > 5) && (mlvl <= 10) ) flucperc *= 3;
      fluc = 0.01 * flucperc * avehps;
    if (fluc == 0) fluc = 1;

      for (dienum = 1; dienum <= 75; dienum++) 
  {
    dietype = dienum - 5;
    if (dietype < 2) dietype = 2;
    while ( (dietype < dienum + 5) && (dietype < 3 * dienum) )
      {
        dice_ave = dienum * (dietype + 1) / 2;
        lower_fluc = 0.95 * fluc;
        higher_fluc = 1.05 * fluc;
        if (higher_fluc == fluc) higher_fluc++;
        if ( (dice_ave >= lower_fluc) && (dice_ave <= higher_fluc) && (dice_ave <= avehps) )
    {
      diebonus = avehps - dice_ave;
      pMob->hit[DICE_NUMBER] = dienum;
      pMob->hit[DICE_TYPE] = dietype;
      pMob->hit[DICE_BONUS] = diebonus;

      send_to_char("Hitdice set.\n\r", ch);
      return TRUE;
    }
      dietype++;
      }
  }
      send_to_char("Unable to set hitdice according to average.\n\r", ch);
      return FALSE;
    }

  num = cp = argument;

  while ( isdigit( *cp ) ) ++cp;
  while ( *cp && !isdigit( *cp ) )  *(cp++) = '\0';

  type = cp;

  while ( isdigit( *cp ) ) ++cp;
  while ( *cp && !isdigit( *cp ) ) *(cp++) = '\0';

  bonus = cp;

  while ( isdigit( *cp ) ) ++cp;
  if ( *cp ) *cp = '\0';

  if ( ( !is_number( num   ) || atoi( num   ) < 1 )
  ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
  ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
      send_to_char( syntax, ch );
      return FALSE;
    }

  pMob->hit[DICE_NUMBER] = atoi( num   );
  pMob->hit[DICE_TYPE]   = atoi( type  );
  pMob->hit[DICE_BONUS]  = atoi( bonus );

  send_to_char( "Hitdice set.\n\r", ch );
  return TRUE;
}

MEDIT( medit_manadice )
{
    static char syntax[] = "Syntax:  manadice <number> d <type> + <bonus>\n\r         manadice avg\n\r";
    char *num, *type, *bonus, *cp;
  int mlvl, avemana, flucperc, fluc, dienum, dietype, diebonus, dice_ave;
  int lower_fluc, higher_fluc;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
  send_to_char( syntax, ch );
  return FALSE;
    }

  if ( (!str_cmp(argument, "avg") ) )  /* Average routine done by Sartan March 2001 */
    {
    mlvl = pMob->level;
    avemana = mlvl * mlvl;
    if (avemana < 100) avemana = 100;
    flucperc = number_range(5, 10);
    fluc = 0.01 * flucperc * avemana;
    if (fluc == 0) fluc = 1;
    for (dienum = 1; dienum <=75; dienum++)
    {
      dietype = dienum - 5;
      if (dietype < 2) dietype = 2;
      while ( (dietype < dienum + 5) && (dietype < 3 * dienum) )
      {
      dice_ave = dienum * (dietype + 1) / 2;
      lower_fluc = 0.95 * fluc;
      higher_fluc = 1.05 * fluc;
      if (higher_fluc == fluc) higher_fluc++;
      if ( (dice_ave >= lower_fluc) && (dice_ave <= higher_fluc) && (dice_ave <= avemana) )
      {
        diebonus = avemana - dice_ave;
        pMob->mana[DICE_NUMBER] = dienum;
        pMob->mana[DICE_TYPE] = dietype;
        pMob->mana[DICE_BONUS] = diebonus;

        send_to_char("Manadice set.\n\r", ch);
        return TRUE;
      }
      dietype++;
      }
    }
    send_to_char("Unable to set manadice according to average.\n\r", ch);
    return FALSE;
    }

    num = cp = argument;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp && !isdigit( *cp ) )  *(cp++) = '\0';

    type = cp;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp && !isdigit( *cp ) ) *(cp++) = '\0';

    bonus = cp;

    while ( isdigit( *cp ) ) ++cp;
    if ( *cp ) *cp = '\0';

    if ( !( is_number( num ) && is_number( type ) && is_number( bonus ) ) )
    {
  send_to_char( syntax, ch );
  return FALSE;
    }

    if ( ( !is_number( num   ) || atoi( num   ) < 1 )
    ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
    ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
  send_to_char( syntax, ch );
  return FALSE;
    }

    pMob->mana[DICE_NUMBER] = atoi( num   );
    pMob->mana[DICE_TYPE]   = atoi( type  );
    pMob->mana[DICE_BONUS]  = atoi( bonus );

    send_to_char( "Manadice set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_damdice )
{
    static char syntax[] = "Syntax:  damdice <number> d <type> + <bonus>\n\r         damdice avg\n\r";
    char *num, *type, *bonus, *cp;
  int mlvl, avedam, flucperc, fluc, dienum, dietype, diebonus, dice_ave;
  int lower_fluc, higher_fluc;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
  send_to_char( syntax, ch );
  return FALSE;
    }

  if ( (!str_cmp(argument, "avg") ) )  /* Average routine done by Sartan March 2001 */
    {
    mlvl = pMob->level;
    if (mlvl < 1)
    {
      send_to_char("Mob level must be greater than 0 to computer average damage.\n\r", ch);
    return FALSE;
    }
    avedam = 3.5 * mlvl;
    if (mlvl <= 100) avedam = 3.0 * mlvl;
    if (mlvl <=  75) avedam = 2.5 * mlvl;
    if (mlvl <=  50) avedam = 2.0 * mlvl;
    if (mlvl <=  25) avedam = 1.5 * mlvl;

    flucperc = number_range(10, 20);
    fluc = 0.01 * flucperc * avedam;
    if (fluc == 0) fluc = 1;
    for (dienum = 1; dienum < 90; dienum++)
    {
    dietype = dienum - 5;
    if (dietype < 2) dietype = 2;
    while ( (dietype < dienum + 5) && (dietype < 3 * dienum) )
    {
      dice_ave = dienum * (dietype + 1) / 2;
      lower_fluc = 0.95 * fluc;
      higher_fluc = 1.05 * fluc;
      if (higher_fluc == fluc) higher_fluc++;
      if ( (dice_ave >= lower_fluc) && (dice_ave <= higher_fluc) && (dice_ave <= avedam) )
      {
      diebonus = avedam - dice_ave;
      pMob->damage[DICE_NUMBER] = dienum;
      pMob->damage[DICE_TYPE] = dietype;
      pMob->damage[DICE_BONUS] = diebonus;

      send_to_char("Damdice set.\n\r", ch);
      return TRUE;
      }
      dietype++;
    }
    }
    send_to_char("unable to set damdice according to average.\n\r", ch);
    return FALSE;
  }

    num = cp = argument;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp && !isdigit( *cp ) )  *(cp++) = '\0';

    type = cp;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp && !isdigit( *cp ) ) *(cp++) = '\0';

    bonus = cp;

    while ( isdigit( *cp ) ) ++cp;
    if ( *cp ) *cp = '\0';

    if ( !( is_number( num ) && is_number( type ) && is_number( bonus ) ) )
    {
  send_to_char( syntax, ch );
  return FALSE;
    }

    if ( ( !is_number( num   ) || atoi( num   ) < 1 )
    ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
    ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
  send_to_char( syntax, ch );
  return FALSE;
    }

    pMob->damage[DICE_NUMBER] = atoi( num   );
    pMob->damage[DICE_TYPE]   = atoi( type  );
    pMob->damage[DICE_BONUS]  = atoi( bonus );

    send_to_char( "Damdice set.\n\r", ch );
    return TRUE;
}


MEDIT( medit_race )
{
    MOB_INDEX_DATA *pMob;
    int race;

    if ( argument[0]
    && ( race = race_lookup( argument ) ) )
    {
    EDIT_MOB( ch, pMob );

      pMob->race = race;
      pMob->act          |= race_table[race].act;
      pMob->affected_by |= race_table[race].aff;
      pMob->off_flags    = race_table[race].off;
      pMob->imm_flags    = race_table[race].imm;
      pMob->res_flags    = race_table[race].res;
      pMob->vuln_flags   = race_table[race].vuln;
      pMob->form         = race_table[race].form;
      pMob->parts        = race_table[race].parts;

      if ( race_table[race].pc_race )
        pMob->size = pc_race_table[race].size;
      else
        send_to_char( "Non-PC race, manually update size.\n\r", ch );     

      send_to_char( "Race set.\n\r", ch );
      return TRUE;
    }

    if ( argument[0] == '?' )
    {

      send_to_char( "Available races are:", ch );

      for ( race = 0; race_table[race].name; race++ )
      {
        if ( ( race % 3 ) == 0 )
          send_to_char( "\n\r", ch );

        printf_to_char(ch, " %-15s", race_table[race].name );
      }

      send_to_char( "\n\r", ch );
      return FALSE;
    }

    send_to_char( "Syntax:  race [race]\n\r"
      "Type 'race ?' for a list of races.\n\r", ch );
    return FALSE;
}


MEDIT( medit_position )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    int64 value;

    argument = one_argument( argument, arg );

    switch ( arg[0] )
    {
    default:
  break;

    case 'S':
    case 's':
  if ( str_prefix( arg, "start" ) )
      break;

  if ( ( value = flag_value( position_flags, argument ) ) == NO_FLAG )
      break;

  EDIT_MOB( ch, pMob );

  pMob->start_pos = value;
  send_to_char( "Start position set.\n\r", ch );
  return TRUE;

    case 'D':
    case 'd':
  if ( str_prefix( arg, "default" ) )
      break;

  if ( ( value = flag_value( position_flags, argument ) ) == NO_FLAG )
      break;

  EDIT_MOB( ch, pMob );

  pMob->default_pos = value;
  send_to_char( "Default position set.\n\r", ch );
  return TRUE;
    }

    send_to_char( "Syntax:  position [start/default] [position]\n\r"
      "Type '? position' for a list of positions.\n\r", ch );
    return FALSE;
}

MEDIT( medit_gold )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
    send_to_char( "Syntax:  wealth [number]\n\r", ch );
    return FALSE;
    }

    pMob->wealth = atoi( argument );

    send_to_char( "Wealth set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_hitroll )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
    send_to_char( "Syntax:  hitroll [number]\n\r", ch );
    return FALSE;
    }

    pMob->hitroll = atoi( argument );

    send_to_char( "Hitroll set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_frag_number )
{
  MOB_INDEX_DATA *pMob;
  EDIT_MOB( ch, pMob );
  int f = 0;

  if ( argument[0] == '\0' || !is_number( argument ) )
  {
    send_to_char( "Syntax: fragment number [number]\n\r", ch );
    return FALSE;
  }

  f = atoi( argument );
  if (f < 0)
  {
    send_to_char( "Fragment number must be 0 or more.\n\r", ch);
    return TRUE;
  }


  pMob->frag_number = f;

  send_to_char( "fragment number set.\n\r", ch);
  return TRUE;
}

void show_liqlist(CHAR_DATA *ch)
{
    int liq;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];
    
    buffer = new_buf();
    
    for ( liq = 0; liq_table[liq].liq_name; liq++)
    {
    if ( (liq % 21) == 0 )
      add_buf(buffer,"Name                 Color          Proof Full Thirst Food Ssize\n\r");

    mprintf(sizeof(buf),buf, "%-20s %-14s %5d %4d %6d %4d %5d\n\r",
    liq_table[liq].liq_name,liq_table[liq].liq_color,
    liq_table[liq].liq_affect[0],liq_table[liq].liq_affect[1],
    liq_table[liq].liq_affect[2],liq_table[liq].liq_affect[3],
    liq_table[liq].liq_affect[4] );
    add_buf(buffer,buf);
    }

    page_to_char( buf_string( buffer ), ch );
    free_buf( buffer );

    return;
}

void show_damlist( CHAR_DATA *ch )
{
    int att;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];
    
    buffer = new_buf();
    
    for ( att = 0; attack_table[att].name; att++)
    {
    if ( (att % 21) == 0 )
      add_buf(buffer,"Name                 Noun\n\r");

    mprintf(sizeof(buf),buf, "%-20s %-20s\n\r",
    attack_table[att].name,attack_table[att].noun );
    add_buf(buffer,buf);
    }

    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);

    return;
}

MEDIT( medit_group )
{
    MOB_INDEX_DATA *pMob;
    MOB_INDEX_DATA *pMTemp;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int temp;
    BUFFER *buffer;
    bool found = FALSE;
    
    EDIT_MOB(ch, pMob);
    
    if ( argument[0] == '\0' )
    {
      send_to_char( "Syntax: group [number]\n\r", ch);
      send_to_char( "        group show [number]\n\r", ch);
      return FALSE;
    }
    
    if ( is_number( argument ) )
    {
    pMob->group = atoi( argument );
      send_to_char( "Group set.\n\r", ch );
    return TRUE;
    }
    
    argument = one_argument( argument, arg );
    
    if ( !strcmp( arg, "show" ) && is_number( argument ) )
    {
    if ( atoi( argument ) == 0 )
    {
    send_to_char( "Are you crazy?\n\r", ch);
    return FALSE;
    }

    buffer = new_buf ();

      for ( temp = 0; temp < 65536; temp++ )
      {
        pMTemp = get_mob_index( temp );
        if ( pMTemp && ( pMTemp->group == atoi( argument ) ) )
        {
        found = TRUE;
          mprintf(sizeof(buf),buf, "[%5d] %s\n\r",
                        pMTemp->vnum, pMTemp->player_name );
        add_buf( buffer, buf );
        }
      }

    if ( found )
    page_to_char( buf_string(buffer), ch );
    else
    send_to_char( "No mobs in that group.\n\r", ch );

    free_buf( buffer );
      return FALSE;
    }
    return FALSE;
}

REDIT( redit_owner )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
  send_to_char( "Syntax:  owner [owner]\n\r", ch );
  send_to_char( "         owner none\n\r", ch );
  return FALSE;
    }

    free_string( pRoom->owner );
    if (!str_cmp(argument, "none"))
    {
      pRoom->owner = str_dup("",pRoom->owner);
      send_to_char( "Owner cleared.\n\r", ch );
    }
    else
    {
      pRoom->owner = str_dup( argument ,pRoom->owner);
      send_to_char( "Owner set.\n\r", ch );
    }

    return TRUE;
}

/*****************************************************************************
 Name:    redit_fish
 Purpose:  Finds the room and sets its fish VNUMs.
 Called by:  The eight fishX functions below. (Taeloch)
 ****************************************************************************/
bool redit_fish( CHAR_DATA *ch, int fish, char *argument )
{
  ROOM_INDEX_DATA *pRoom;
  OBJ_INDEX_DATA  *pObj = NULL;
  int cfish = 0;

  EDIT_ROOM(ch, pRoom);

  if ( argument[0] == '\0' )
  {
    send_to_char( "Syntax:  fish[#] [FishObj VNum]\n\r", ch );
    return FALSE;
  }

  if (!IS_SET(pRoom->room_flags,ROOM_FISHING))
  {
    send_to_char( "OEdit: room_fishing flag not set in this room.\n\r", ch );
    return FALSE;
  }

  cfish = atoi(argument);
  if ( ( cfish > 0 )
  &&  !( pObj = get_obj_index(cfish) ) )
  {
    send_to_char("OEdit: That object does not exist.\n\r",ch);
    return FALSE;
  }

  if (cfish == 0)
    printf_to_char( ch, "\n\rFish #%d set to default.\n\r",fish);
  else
    printf_to_char( ch, "\n\rFish #%d set to %s (%d).\n\r",
      fish, pObj->short_descr, cfish);

  pRoom->fish[fish-1] = cfish;
  return TRUE;
}

REDIT( redit_fish1 ) { return redit_fish( ch, 1, argument ); }
REDIT( redit_fish2 ) { return redit_fish( ch, 2, argument ); }
REDIT( redit_fish3 ) { return redit_fish( ch, 3, argument ); }
REDIT( redit_fish4 ) { return redit_fish( ch, 4, argument ); }
REDIT( redit_fish5 ) { return redit_fish( ch, 5, argument ); }
REDIT( redit_fish6 ) { return redit_fish( ch, 6, argument ); }
REDIT( redit_fish7 ) { return redit_fish( ch, 7, argument ); }
REDIT( redit_fish8 ) { return redit_fish( ch, 8, argument ); }

MPROG_LIST *addmprog( CHAR_DATA *ch, char *argument )
{
  MPROG_LIST *list;
  MPROG_CODE *code;

  char trigger[MAX_INPUT_LENGTH];
  char phrase[MAX_INPUT_LENGTH];
  char num[MAX_INPUT_LENGTH];
  int64 value;

  argument = one_argument( argument, num );
  argument = one_argument( argument, trigger );
  strcpy( phrase, argument );

  if ( !is_number( num ) || trigger[0] == '\0' || phrase[0] == '\0' )
  {
    send_to_char( "Syntax: addmprog [vnum] [trigger] [phrase]\n\r", ch );
    send_to_char( "      ( Type ? mprog for available triggers.\n\r", ch );
    return NULL;
  }

  if ( ( value = flag_value( mprog_flags, trigger ) ) == NO_FLAG )
  {
    send_to_char("Valid flags are:\n\r", ch);
    show_help( ch, "mprog" );
    return NULL;
  }

  if ( ( code = get_mprog_index( atoi( num ) ) ) == NULL )
  {
    send_to_char("No such MOBProgram.\n\r", ch);
    return NULL;
  }

  list              = new_mprog();
  list->vnum        = atoi( num );
  list->trig_type   = value;
  list->trig_phrase = str_dup( phrase, list->trig_phrase );
  list->code        = code;
  return list;
}

OEDIT ( oedit_addmprog )
{
  OBJ_INDEX_DATA   *pObj;
  MPROG_LIST        *list;

  EDIT_OBJ(ch, pObj);

  if ( ( list = addmprog( ch, argument ) ) == NULL ) 
    return FALSE;

  SET_BIT( pObj->mprog_flags, list->trig_type );
  list->next        = pObj->mprogs;
  pObj->mprogs      = list;

  send_to_char("OProg added.\n\r", ch);
  return TRUE;
}

REDIT ( redit_addmprog )
{
  ROOM_INDEX_DATA *pRoom;
  MPROG_LIST    *list;

  EDIT_ROOM(ch, pRoom);

  if ( ( list = addmprog( ch, argument ) ) == NULL )
    return FALSE;

  SET_BIT( pRoom->mprog_flags, list->trig_type );
  list->next        = pRoom->mprogs;
  pRoom->mprogs     = list;

  send_to_char("RProg added.\n\r",ch);
  return TRUE;
}

MEDIT ( medit_addmprog )
{
//  int64 value;
  MOB_INDEX_DATA *pMob;
  MPROG_LIST *list;
/*
  MPROG_CODE *code;
  char trigger[MAX_STRING_LENGTH];
  char phrase[MAX_STRING_LENGTH];
  char num[MAX_STRING_LENGTH];
*/

  EDIT_MOB(ch, pMob);

/*
  argument=one_argument(argument, num);
  argument=one_argument(argument, trigger);
  strcpy(phrase,argument);

  if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0' )
  {
    send_to_char("Syntax:   addmprog [vnum] [trigger] [phrase]\n\r",ch);
    return FALSE;
  }

  if ( (value = flag_value (mprog_flags, trigger) ) == NO_FLAG)
  {
    send_to_char("Valid flags are:\n\r",ch);
    show_help( ch, "mprog");
    return FALSE;
  }

  if ( ( code = get_mprog_index (atoi(num) ) ) == NULL)
  {
    send_to_char("No such MOBProgram.\n\r",ch);
    return FALSE;
  }

  list                  = new_mprog();
  list->vnum            = atoi(num);
  list->trig_type       = value;
  list->trig_phrase     = str_dup(phrase, list->trig_phrase);
  list->code            = code->code;
*/
  if ( ( list = addmprog( ch, argument ) ) == NULL )
    return FALSE;

  SET_BIT(pMob->mprog_flags,list->trig_type);
  list->next            = pMob->mprogs;
  pMob->mprogs          = list;

  send_to_char( "Mprog Added.\n\r",ch);
  return TRUE;
}

REDIT ( redit_delmprog )
{
  ROOM_INDEX_DATA *room;
  MPROG_LIST      *list;
  MPROG_LIST      *list_next;
  char mprog[MAX_STRING_LENGTH];
  int value;
  int cnt = 0;

  EDIT_ROOM(ch, room);

  one_argument( argument, mprog );
  if ( !is_number( mprog ) || mprog[0] == '\0' )
  {
    send_to_char("Syntax: delmprog [#mprog]\n\r", ch);
    return FALSE;
  }

  value = atoi( mprog );

  if( value < 0 )
  {
    send_to_char("Only non-negative mprog-numbers allowed.\n\r", ch);
    return FALSE;
  }

  if( !(list = room->mprogs) )
  {
    send_to_char("Medit: Non-existent RProg. \n\r", ch);
    return FALSE;
  }

  if( value == 0 )
  {
    REMOVE_BIT( room->mprog_flags, room->mprogs->trig_type );
    list = room->mprogs;
    room->mprogs = list->next;
    free_mprog( list );
  }
  else
  {
    while( ( list_next = list->next ) && ( ++cnt < value ) )
      list = list_next;

    if( list_next )
    {
      REMOVE_BIT( room->mprog_flags, list_next->trig_type );
      list->next = list_next->next;
      free_mprog( list_next );
    }
    else
    {
      send_to_char("MEdit: Nonexistent mprog.\n\r", ch);
      return FALSE;
    }
  }

  send_to_char("Roomprog removed.\n\r", ch);
  return TRUE;
}

OEDIT ( oedit_delmprog )
{
  OBJ_INDEX_DATA *pObj;
  MPROG_LIST     *list;
  MPROG_LIST     *list_next;
  char mprog[MAX_INPUT_LENGTH];
  int value;
  int cnt = 0;

  EDIT_OBJ( ch, pObj );

  one_argument( argument, mprog );
  if ( !is_number( mprog ) || mprog[0] == '\0' )
  {
    send_to_char("Syntax: delmprog [#mprog]\n\r", ch);
    return FALSE;
  }
  
  if ( ( value = atoi( mprog ) ) < 0 )
  {
    send_to_char("Only non-negative mprog-numbers allowed.\n\r", ch);
    return FALSE;
  }

  if ( !(list = pObj->mprogs) )
  {
    send_to_char("OEdit: Now existent oprog.\n\r", ch);
    return FALSE;
  }

  if ( value == 0 )
  {
    REMOVE_BIT(pObj->mprog_flags, pObj->mprogs->trig_type);
    list = pObj->mprogs;
    pObj->mprogs = list->next;
    free_mprog( list );
  }
  else
  {
    while ( (list_next = list->next) && (++cnt < value ) )
      list = list_next;
    if ( list_next )
    {
      REMOVE_BIT(pObj->mprog_flags, list_next->trig_type);
      list->next = list_next->next;
      free_mprog(list_next);
    }
    else
    {
      send_to_char("No such oprog.\n\r", ch);
      return FALSE;
    }
  }

  send_to_char("Oprogram removed.\n\r", ch);
  return TRUE;
}

MEDIT ( medit_delmprog )
{
  MOB_INDEX_DATA *pMob;
  MPROG_LIST *list;
  MPROG_LIST *list_next;
  char mprog[MAX_STRING_LENGTH];
  int value;
  int cnt = 0;

  EDIT_MOB(ch, pMob);

  one_argument( argument, mprog );
  if (!is_number( mprog ) || mprog[0] == '\0' )
  {
    send_to_char("Syntax: delmprog [#mprog]\n\r",ch);
    return FALSE;
  }

  value = atoi ( mprog );

  if ( value < 0 )
  {
    send_to_char("Only non-negative mprog-numbers allowed.\n\r",ch);
    return FALSE;
  }

  if ( !(list = pMob->mprogs) )
  {
    send_to_char("MEdit: Non existent mprog.. none linked in memory\n\r",ch);
    return FALSE;
  }

  if ( value == 0 )
  {
    REMOVE_BIT(pMob->mprog_flags, pMob->mprogs->trig_type);
    list = pMob->mprogs;
    pMob->mprogs = list->next;
    free_mprog( list );
  }
  else
  {
    while ( (list_next = list->next) && (++cnt < value ) )
      list = list_next;

    if ( list_next )
    {
    REMOVE_BIT(pMob->mprog_flags, list_next->trig_type);
    list->next = list_next->next;
    free_mprog(list_next);
    }
    else
    {
    send_to_char("No such mprog.\n\r",ch);
    return FALSE;
    }
  }

  send_to_char("Mprog removed.\n\r", ch);
  return TRUE;
}

REDIT( redit_room )
{
  ROOM_INDEX_DATA *room;
  int64 value;
  int i;

  EDIT_ROOM(ch, room);

  if ( (value = flag_value( room_flags, argument )) == NO_FLAG )
    {
      send_to_char( "Syntax: room [flags]\n\r", ch );
      return FALSE;
    }
  if ( IS_SET( value, ROOM_SHIP ) && !IS_SET( room->room_flags, ROOM_SHIP ) )
  {
    for ( i = 0; i <= 5; i++ )
    {
      if ( room->exit[i] 
      &&   room->exit[i]->u1.to_room->exit[rev_dir[i]]
      &&   room->exit[i]->u1.to_room->exit[rev_dir[i]]->u1.to_room == room )
      {
        send_to_char( 
        "REdit: May not set shipflag if linked exits exist.\n\r",ch );
        return FALSE;
      }
    }
  }

  if ( IS_SET( value, ROOM_SHIP ) && IS_SET( room->room_flags, ROOM_SHIP ) )
       set_state_room( room, -1 );

  TOGGLE_BIT( room->room_flags, value );
  send_to_char( "Room flags toggled.\n\r", ch );
  return TRUE;
}

REDIT( redit_state )
{
  ROOM_INDEX_DATA *room;
  char num[MAX_INPUT_LENGTH];

  EDIT_ROOM(ch, room);

  one_argument(argument, num);
  set_state_room( room, atoi(num) );
  if ( room->state == atoi(num) )
    send_to_char("State set.\n\r",ch);

  return TRUE;
}

REDIT( redit_sector )
{
  ROOM_INDEX_DATA *room;
  int64 value;

  EDIT_ROOM(ch, room);

  if ( (value = flag_value( sector_flags, argument )) == NO_FLAG )
    {
      send_to_char( "Syxtax: sector [flags]\n\r", ch );
      return FALSE;
    }

  room->sector_type = value;
  send_to_char( "Sector type set.\n\r", ch );

  return TRUE;
}

REDIT( redit_copy )
{
  ROOM_INDEX_DATA  *pRoom;
  ROOM_INDEX_DATA  *pRoom2; /* Room to copy */
  EXTRA_DESCR_DATA *ed;
  EXTRA_DESCR_DATA *pExtra;

  int vnum;

  if ( argument[0] == '\0' )
    {
      send_to_char("Syntax: copy <vnum> \n\r",ch);
      return FALSE;
    }

  if ( !is_number(argument) )
    {
      send_to_char("REdit: You must enter a number (vnum).\n\r",ch);
      return FALSE;
    }
  else /* argument is a number */
    {
      vnum = atoi(argument);
      if( !( pRoom2 = get_room_index(vnum) ) )
  {
    send_to_char("REdit: That room does not exist.\n\r",ch);
    return FALSE;
  }
    }

  EDIT_ROOM(ch, pRoom);

  free_string( pRoom->description );
  pRoom->description = str_dup( pRoom2->description ,pRoom->description);
    
  free_string( pRoom->name );
  pRoom->name = str_dup( pRoom2->name ,pRoom->name);

  /* sector flags */
  pRoom->sector_type = pRoom2->sector_type;

  /* room flags */
  pRoom->room_flags = pRoom2->room_flags;

  pRoom->heal_rate = pRoom2->heal_rate;
  pRoom->mana_rate = pRoom2->mana_rate;

  pRoom->clan = pRoom2->clan;

  free_string( pRoom->owner );

  pRoom->owner = str_dup( pRoom2->owner ,pRoom->owner);

  for ( pExtra = pRoom2->extra_descr; pExtra; pExtra = pExtra->next )
    {
      ed      =   new_extra_descr();
      ed->keyword    =   str_dup( pExtra->keyword, ed->keyword );
      ed->description  =   str_dup( pExtra->description , ed->description);
      ed->next    =   pRoom->extra_descr;
      pRoom->extra_descr  =   ed;
    }  

  send_to_char( "Room info copied.", ch );
  return TRUE;
}


/*
 * oedit_copy function thanks to Zanthras of Mystical Realities MUD.
 */
OEDIT( oedit_copy )
{
  OBJ_INDEX_DATA    *pObj;
  OBJ_INDEX_DATA    *pObj2; /* The object to copy */
  EXTRA_DESCR_DATA  *ed;
  EXTRA_DESCR_DATA  *pExtra;
  AFFECT_DATA       *pAf;
  AFFECT_DATA       *pAf2;

  int vnum, i;

  if ( argument[0] == '\0' )
    {
      send_to_char("Syntax: copy <vnum> \n\r",ch);
      return FALSE;
    }

  if ( !is_number(argument) )
    {
      send_to_char("OEdit: You must enter a number (vnum).\n\r",ch);
      return FALSE;
    }
  else /* argument is a number */
    {
      vnum = atoi(argument);
      if( !( pObj2 = get_obj_index(vnum) ) )
  {
    send_to_char("OEdit: That object does not exist.\n\r",ch);
    return FALSE;
  }
    }

  EDIT_OBJ(ch, pObj);

  free_string( pObj->name );
  pObj->name = str_dup( pObj2->name,pObj->name );

  pObj->item_type = pObj2->item_type;

  pObj->level = pObj2->level;

  pObj->wear_flags  = pObj2->wear_flags;
  pObj->extra_flags = pObj2->extra_flags;

  free_string( pObj->material );
  pObj->material = str_dup( pObj2->material ,pObj->material);
    
  pObj->condition = pObj2->condition;

  pObj->weight = pObj2->weight;
  pObj->cost   = pObj2->cost;

  for ( pExtra = pObj2->extra_descr; pExtra; pExtra = pExtra->next )
  {
      ed      =   new_extra_descr();
      ed->keyword    =   str_dup( pExtra->keyword, ed->keyword );
      ed->description  =   str_dup( pExtra->description , ed->description);
      ed->next    =   pObj->extra_descr;
      pObj->extra_descr  =   ed;
  }  

  free_string( pObj->short_descr );
  pObj->short_descr = str_dup( pObj2->short_descr,pObj->short_descr );

  free_string( pObj->description );
  pObj->description = str_dup( pObj2->description,pObj->description );


  // pObj->affected = pObj2->affected;
  for ( pAf2 = pObj2->affected; pAf2; pAf2 = pAf2->next )
  {
    pAf             =   new_affect();
    pAf->location   =   pAf2->location;
    pAf->modifier   =   pAf2->modifier;
    pAf->where      =   pAf2->where;
    pAf->type       =   pAf2->type;
    pAf->duration   =   pAf2->duration;
    pAf->bitvector  =   pAf2->bitvector;
    pAf->level      =   pAf2->level;
    pAf->next       =   pObj->affected; // Adds the copied aff to the new obj
    pObj->affected  =   pAf;
  } // There you go, a deep copy. It _might_ work like this too:
/*
  for ( pAf2 = pObj2->affected; pAf2; pAf2 = pAf2->next )
  {
    pAf = new_affect();
    *pAf = *pAf2;
    pAf->next = pObj->affected;
    pObj->affected = pAf;
  }
*/  // But I prefer the first... safety you know --- Merak

  for (i = 0; i < 5; i++)
    {
      pObj->value[i] = pObj2->value[i];
    }

  send_to_char( "Object info copied.", ch );
  return TRUE;
}


/*
 * medit_copy function thanks to Zanthras of Mystical Realities MUD.
 * Thanks to Ivan for what there is of the incomplete mobprog part.
 * Hopefully it will be finished in a later release of this snippet.
 */
MEDIT( medit_copy )
{
    MOB_INDEX_DATA *pMob;
    MOB_INDEX_DATA *pMob2; /* The mob to copy */
    int vnum;

    /* MPROG_LIST *list; */ /* Used for the mob prog for loop */

    if ( argument[0] == '\0' )
    {
      send_to_char("Syntax: copy <vnum> \n\r",ch);
      return FALSE;
    }

    if ( !is_number(argument) )
    {
      send_to_char("MEdit: You must enter a number (vnum).\n\r",ch);
      return FALSE;
    }
    else /* argument is a number */
    {
      vnum = atoi(argument);
      if( !( pMob2 = get_mob_index(vnum) ) )
      {
  send_to_char("MEdit: That mob does not exist.\n\r",ch);
  return FALSE;
      }
    }

    EDIT_MOB(ch, pMob);

    free_string( pMob->player_name );
    pMob->player_name = str_dup( pMob2->player_name ,pMob->player_name);

    pMob->new_format = pMob2->new_format;
    pMob->act = pMob2->act;
    pMob->sex = pMob2->sex;
    pMob->max_count = pMob2->max_count; //1-30-09 RWL
 
    pMob->race = pMob2->race;

    pMob->level = pMob2->level;
    
    pMob->alignment = pMob2->alignment;
    
    pMob->hitroll = pMob2->hitroll;
    
    pMob->dam_type = pMob2->dam_type;

    pMob->group = pMob2->group;

    pMob->hit[DICE_NUMBER] = pMob2->hit[DICE_NUMBER];
    pMob->hit[DICE_TYPE]   = pMob2->hit[DICE_TYPE];
    pMob->hit[DICE_BONUS]  = pMob2->hit[DICE_BONUS];

    pMob->damage[DICE_NUMBER] = pMob2->damage[DICE_NUMBER];
    pMob->damage[DICE_TYPE]   = pMob2->damage[DICE_TYPE];
    pMob->damage[DICE_BONUS]  = pMob2->damage[DICE_BONUS];
    
    pMob->mana[DICE_NUMBER] = pMob2->mana[DICE_NUMBER];
    pMob->mana[DICE_TYPE]   = pMob2->mana[DICE_TYPE];
    pMob->mana[DICE_BONUS]  = pMob2->mana[DICE_BONUS];

    pMob->affected_by = pMob2->affected_by;
    pMob->affected2_by = pMob2->affected2_by;
    
    pMob->ac[AC_PIERCE] = pMob2->ac[AC_PIERCE];
    pMob->ac[AC_BASH]   = pMob2->ac[AC_BASH];
    pMob->ac[AC_SLASH]  = pMob2->ac[AC_SLASH];
    pMob->ac[AC_EXOTIC] = pMob2->ac[AC_EXOTIC];
    

    pMob->form  = pMob2->form;
    pMob->parts = pMob2->parts;

    pMob->imm_flags  = pMob2->imm_flags;
    pMob->res_flags  = pMob2->res_flags;
    pMob->vuln_flags = pMob2->vuln_flags;
    pMob->off_flags  = pMob2->off_flags;

    pMob->size     = pMob2->size;

    free_string( pMob->material );
    pMob->material = str_dup( pMob2->material ,pMob->material); 

    pMob->start_pos   = pMob2->start_pos;
    pMob->default_pos = pMob2->default_pos;

    pMob->wealth = pMob2->wealth;

    pMob->spec_fun = pMob2->spec_fun;

    free_string( pMob->short_descr );
    pMob->short_descr = str_dup( pMob2->short_descr,pMob->short_descr );

    free_string( pMob->long_descr );
    pMob->long_descr = str_dup( pMob2->long_descr  ,pMob->long_descr );

    free_string( pMob->description );
    pMob->description = str_dup( pMob2->description ,pMob->description);

    /* Hopefully get the shop data to copy later
     * This is the fields here if you get it copying send me
     * a copy and I'll add it to the snippet with credit to
     * you of course :) same with the mobprogs for loop :)
     */
    if ( pMob2->pShop )
    {
    SHOP_DATA *pShop, *pShop2;
      int        iTrade;

      pMob->pShop     = new_shop();

      if ( !shop_first )
        shop_first  = pMob->pShop;
      if ( shop_last )
        shop_last->next = pMob->pShop;

      shop_last       = pMob->pShop;

      pMob->pShop->keeper = pMob->vnum;

    pShop =  pMob->pShop;
    pShop2 = pMob2->pShop;
 
    pShop->profit_buy  = pShop2->profit_buy;
    pShop->profit_sell = pShop2->profit_sell;
  
    pShop->open_hour  = pShop2->open_hour;
    pShop->close_hour = pShop2->close_hour;

    for ( iTrade = 0; iTrade <= MAX_TRADE; iTrade++ )
      pShop->buy_type[iTrade] = pShop2->buy_type[iTrade];
    }

/*  for loop thanks to Ivan, still needs work though

    for (list = pMob->mprogs; list; list = list->next )
    {
      MPROG_LIST *newp = new_mprog();
      newp->trig_type = list->trig_type;

      free_string( newp->trig_phrase );
      newp->trig_phrase = str_dup( list->trig_phrase );

      newp->vnum = list->vnum;

      free_string( newp->code )
      newp->code = str_dup( list->code );

      pMob->mprogs = newp;
    }
*/

    send_to_char( "Mob info copied.", ch );
    return FALSE;
}

/*** stat only copy, trust me it's useful
     Chris Jensen 11/4/2000
***/
MEDIT( medit_statcopy )
{
  MOB_INDEX_DATA *pMob;
  MOB_INDEX_DATA *pMob2; /* The mob to copy */
  int vnum;

  /* MPROG_LIST *list; */ /* Used for the mob prog for loop */

  if ( argument[0] == '\0' )
    {
      send_to_char("Syntax: copy <vnum> \n\r",ch);
      return FALSE;
    }

  if ( !is_number(argument) )
    {
      send_to_char("MEdit: You must enter a number (vnum).\n\r",ch);
      return FALSE;
    }
  else /* argument is a number */
    {
      vnum = atoi(argument);
      if( !( pMob2 = get_mob_index(vnum) ) )
  {
    send_to_char("MEdit: That mob does not exist.\n\r",ch);
    return FALSE;
  }
    }

  EDIT_MOB(ch, pMob);

  pMob->new_format = pMob2->new_format;
  pMob->act = pMob2->act;
  pMob->sex = pMob2->sex;
 
  pMob->race = pMob2->race;

  pMob->level = pMob2->level;
    
  pMob->alignment = pMob2->alignment;
    
  pMob->hitroll = pMob2->hitroll;
    
  pMob->dam_type = pMob2->dam_type;

  pMob->group = pMob2->group;

  pMob->hit[DICE_NUMBER] = pMob2->hit[DICE_NUMBER];
  pMob->hit[DICE_TYPE]   = pMob2->hit[DICE_TYPE];
  pMob->hit[DICE_BONUS]  = pMob2->hit[DICE_BONUS];

  pMob->damage[DICE_NUMBER] = pMob2->damage[DICE_NUMBER];
  pMob->damage[DICE_TYPE]   = pMob2->damage[DICE_TYPE];
  pMob->damage[DICE_BONUS]  = pMob2->damage[DICE_BONUS];
    
  pMob->mana[DICE_NUMBER] = pMob2->mana[DICE_NUMBER];
  pMob->mana[DICE_TYPE]   = pMob2->mana[DICE_TYPE];
  pMob->mana[DICE_BONUS]  = pMob2->mana[DICE_BONUS];

  pMob->affected_by = pMob2->affected_by;
    
  pMob->ac[AC_PIERCE] = pMob2->ac[AC_PIERCE];
  pMob->ac[AC_BASH]   = pMob2->ac[AC_BASH];
  pMob->ac[AC_SLASH]  = pMob2->ac[AC_SLASH];
  pMob->ac[AC_EXOTIC] = pMob2->ac[AC_EXOTIC];
    

  pMob->form  = pMob2->form;
  pMob->parts = pMob2->parts;

  pMob->imm_flags  = pMob2->imm_flags;
  pMob->res_flags  = pMob2->res_flags;
  pMob->vuln_flags = pMob2->vuln_flags;
  pMob->off_flags  = pMob2->off_flags;

  pMob->size     = pMob2->size;

  free_string( pMob->material );
  pMob->material = str_dup( pMob2->material ,pMob->material); 

  pMob->start_pos   = pMob2->start_pos;
  pMob->default_pos = pMob2->default_pos;

  pMob->wealth = pMob2->wealth;

  pMob->spec_fun = pMob2->spec_fun;

  send_to_char( "Mob stats copied.", ch );
  return FALSE;
}

bool check_area_vnum(AREA_DATA *pArea, int vnum, CHAR_DATA *ch)
{
  if (IN_RANGE(pArea->min_vnum, vnum, pArea->max_vnum))
    return TRUE;

  send_to_char("The VNUM is not valid for this area\n\r",ch);
  return FALSE;
      
}

void unlink_room_index( ROOM_INDEX_DATA *pRoom )
{
  int iHash;
  ROOM_INDEX_DATA *iRoom, *sRoom;

  iHash = pRoom->vnum % MAX_KEY_HASH;

  sRoom = room_index_hash[iHash];

  if( sRoom->next == NULL ) /* only entry */
    room_index_hash[iHash] = NULL;
  else if( sRoom == pRoom ) /* first entry */
    room_index_hash[iHash] = pRoom->next;
  else /* everything else */
    {
      for( iRoom = sRoom; iRoom != NULL; iRoom = iRoom->next )
  {
    if( iRoom == pRoom )
      {
        sRoom->next = pRoom->next;
        break;
      }
    sRoom = iRoom;
  }
    }
}

REDIT( redit_delete )
{
  ROOM_INDEX_DATA *pRoom, *pRoom2;
  RESET_DATA *pReset;
  EXIT_DATA *ex;
  OBJ_DATA *Obj, *obj_next;
  CHAR_DATA *wch, *wnext;
  EXTRA_DESCR_DATA *pExtra;
  char arg[MIL];
  char buf[MSL];
  int index, i, iHash, rcount, ecount, mcount, ocount, edcount;

  if ( argument[0] == '\0' )
    {
      send_to_char( "Syntax:  redit delete [vnum]\n\r", ch );
      return FALSE;
    }
  if (ch->level < 110)
    return FALSE;

  one_argument( argument, arg );

  if( is_number( arg ) )
    {
      index = atoi( arg );
      if ((is_revered_room(index)) && (!IS_IMPLEMENTOR(ch)))
  {
    send_to_char("Sorry that is a Revered and protected Vnum\n\r",ch);
    return FALSE;
  }
      pRoom = get_room_index( index );
    }
  else
    {
      send_to_char( "That is not a number.\n\r", ch );
      return FALSE;
    }

  if( !pRoom )
    {
      send_to_char( "No such room.\n\r", ch );
      return FALSE;
    }

  /* Move the player out of the room. */
  if( ch->in_room->vnum == index )
    {
      send_to_char( "Moving you out of the room"
        " you are deleting.\n\r", ch);
      if( ch->fighting != NULL )
  stop_fighting( ch, TRUE );

      char_from_room( ch );
      char_to_room( ch, get_room_index( 3 ) ); /* limbo */
      ch->was_in_room = ch->in_room;
      /*      ch->from_room = ch->in_room;*/
    }

  SET_BIT( pRoom->area->area_flags, AREA_CHANGED );

  /* Count resets. They are freed by free_room_index. */
  rcount = 0;

  for( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
    {
      rcount++;
    }

  /* Now contents */
  ocount = 0;
  for( Obj = pRoom->contents; Obj; Obj = obj_next )
    {
      obj_next = Obj->next_content;

      extract_obj( Obj );
      ocount++;
    }

  /* Now PCs and Mobs */
  mcount = 0;
  for( wch = pRoom->people; wch; wch = wnext )
    {
      wnext = wch->next_in_room;
      if( IS_NPC( wch ) )
  {
    extract_char( wch, TRUE );
    mcount++;
  }
      else
  {
    send_to_char( "This room is being deleted. Moving" 
      " you somewhere safe.\n\r", ch );
    if( wch->fighting != NULL )
      stop_fighting( wch, TRUE );

    char_from_room( wch );

    /* Midgaard Temple */
    char_to_room( wch, get_room_index( 3054 ) ); 
    wch->was_in_room = wch->in_room;
    /*    wch->from_room = wch->in_room;*/
  }
    }

  /* unlink all exits to the room. */
  ecount = 0;
  for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
      for( pRoom2 = room_index_hash[iHash]; pRoom2; pRoom2 = pRoom2->next )
  {
    for( i = 0; i <= MAX_DIR; i++ )
      {
        if( !( ex = pRoom2->exit[i] ) )
    continue;

        if( pRoom2 == pRoom )
    {
      /* these are freed by free_room_index */
      ecount++;
      continue;
    }

        if( ex->u1.to_room == pRoom )
    {
      free_exit( pRoom2->exit[i] );
      pRoom2->exit[i] = NULL;
      SET_BIT( pRoom2->area->area_flags, AREA_CHANGED );
      ecount++;
    }
      }
  }
    }

  /* count extra descs. they are freed by free_room_index */
  edcount = 0;
  for ( pExtra = pRoom->extra_descr; pExtra; pExtra = pExtra->next )
    {
      edcount++;
    }

  if( top_vnum_room == index )
    for( i = 1; i < index; i++ )
      if( get_room_index( i ) )
  top_vnum_room = i;

  top_room--;

  unlink_room_index( pRoom );

  pRoom->area = NULL;
  pRoom->vnum = 0;

  free_room_index( pRoom );

  /* Na na na na! Hey Hey Hey, Good Bye! */

  mprintf(sizeof(buf),buf, "Removed room vnum {C%d{x, %d resets, %d extra "
     "descriptions and %d exits.\n\r", index, rcount, edcount, ecount );
  send_to_char( buf, ch );
  mprintf(sizeof(buf),buf, "{C%d{x objects and {C%d{x mobiles were extracted "
     "from the room.\n\r", ocount, mcount );
  send_to_char( buf, ch );

  return TRUE;
}


/* unlink a given reset from a given room */
void unlink_reset( ROOM_INDEX_DATA *pRoom, RESET_DATA *pReset )
{
  RESET_DATA *prev, *wReset;

  prev = pRoom->reset_first;
  for( wReset = pRoom->reset_first; wReset; wReset = wReset->next )
    {
      if( wReset == pReset )
  {
    if( pRoom->reset_first == pReset )
      {
        pRoom->reset_first = pReset->next;
        if( !pRoom->reset_first )
    pRoom->reset_last = NULL;
      }
    else if( pRoom->reset_last == pReset )
      {
        pRoom->reset_last = prev;
        prev->next = NULL;
      }
    else
      prev->next = prev->next->next;

    if( pRoom->area->reset_first == pReset )
      pRoom->area->reset_first = pReset->next;

    if( !pRoom->area->reset_first )
      pRoom->area->reset_last = NULL;
  }

      prev = wReset;
    }
}

void unlink_obj_index( OBJ_INDEX_DATA *pObj )
{
  int iHash;
  OBJ_INDEX_DATA *iObj, *sObj;

  iHash = pObj->vnum % MAX_KEY_HASH;

  sObj = obj_index_hash[iHash];

  if( sObj->next == NULL ) /* only entry */
    obj_index_hash[iHash] = NULL;
  else if( sObj == pObj ) /* first entry */
    obj_index_hash[iHash] = pObj->next;
  else /* everything else */
    {
      for( iObj = sObj; iObj != NULL; iObj = iObj->next )
  {
    if( iObj == pObj )
      {
        sObj->next = pObj->next;
        break;
      }
    sObj = iObj;
  }
    }
}

void unlink_mob_index( MOB_INDEX_DATA *pMob )
{
  int iHash;
  MOB_INDEX_DATA *iMob, *sMob;

  iHash = pMob->vnum % MAX_KEY_HASH;

  sMob = mob_index_hash[iHash];

  if( sMob->next == NULL ) /* only entry */
    mob_index_hash[iHash] = NULL;
  else if( sMob == pMob ) /* first entry */
    mob_index_hash[iHash] = pMob->next;
  else /* everything else */
    {
      for( iMob = sMob; iMob != NULL; iMob = iMob->next )
  {
    if( iMob == pMob )
      {
        sMob->next = pMob->next;
        break;
      }
    sMob = iMob;
  }
    }
}
OEDIT( oedit_delete )
{
  OBJ_DATA *obj, *obj_next;
  OBJ_INDEX_DATA *pObj;
  RESET_DATA *pReset, *wReset;
  ROOM_INDEX_DATA *pRoom;
  char arg[MIL];
  char buf[MSL];
  int index, rcount, ocount, i, iHash;

  if ( argument[0] == '\0' )
    {
      send_to_char( "Syntax:  oedit delete [vnum]\n\r", ch );
      return FALSE;
    }

  one_argument( argument, arg );

  if( is_number( arg ) )
    {
      index = atoi( arg );
      if ((is_revered_obj(index)) && (!IS_IMPLEMENTOR(ch)))
  {
    send_to_char("Sorry that is a Revered and protected Vnum\n\r",ch);
    return FALSE;
  }

      pObj = get_obj_index( index );
    }
  else
    {
      send_to_char( "That is not a number.\n\r", ch );
      return FALSE;
    }
  if (ch->level < 110)
    return FALSE;
  if( !pObj )
    {
      send_to_char( "No such object.\n\r", ch );
      return FALSE;
    }

  SET_BIT( pObj->area->area_flags, AREA_CHANGED );

  if( top_vnum_obj == index )
    for( i = 1; i < index; i++ )
      if( get_obj_index( i ) )
  top_vnum_obj = i;


  top_obj_index--;

  /* remove objects */
  ocount = 0;
  for( obj = object_list; obj; obj = obj_next )
    {
      obj_next = obj->next;

      if( obj->pIndexData == pObj )
  {
    extract_obj( obj );
    ocount++;
  }
    }

  /* crush resets */
  rcount = 0;
  for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
      for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
  {
    for( pReset = pRoom->reset_first; pReset; pReset = wReset )
      {
        wReset = pReset->next;
        switch( pReset->command )
    {
    case 'O':
    case 'E':
    case 'P':
    case 'G':
      if( ( pReset->arg1 == index ) ||
          ( ( pReset->command == 'P' ) && (
                   pReset->arg3 == index ) ) )
        {
          unlink_reset( pRoom, pReset );
          free_reset_data( pReset );

          rcount++;
          SET_BIT( pRoom->area->area_flags,
             AREA_CHANGED );

        }
    }
      }
  }
    }

  unlink_obj_index( pObj );

  pObj->area = NULL;
  pObj->vnum = 0;

  free_obj_index( pObj );

  mprintf(sizeof(buf),buf, "Removed object vnum {C%d{x and"
     " {C%d{x resets.\n\r", index,rcount );

  send_to_char( buf, ch );

  mprintf(sizeof(buf),buf, "{C%d{x occurences of the object"
     " were extracted from the mud.\n\r", ocount );

  send_to_char( buf, ch );

  return TRUE;
}


MEDIT( medit_delete )
{
  CHAR_DATA *wch, *wnext;
  MOB_INDEX_DATA *pMob;
  RESET_DATA *pReset, *wReset;
  ROOM_INDEX_DATA *pRoom;
  char arg[MIL];
  int index, mcount, rcount, iHash, i;
  bool foundmob = FALSE;
  bool foundobj = FALSE;

  if( argument[0] == '\0' )
    {
      send_to_char( "Syntax:  medit delete [vnum]\n\r", ch );
      return FALSE;
    }

  one_argument( argument, arg );

  if( is_number( arg ) )
    {
      index = atoi( arg );
      if ((is_revered_mob(index)) && (!IS_IMPLEMENTOR(ch)))
  {
    send_to_char("Sorry that is a Revered and protected Vnum\n\r",ch);
    return FALSE;
  }
      pMob = get_mob_index( index );
    }
  else
    {
      send_to_char( "That is not a number.\n\r", ch );
      return FALSE;
    }

  if (ch->level < 110)
    return FALSE;
  if( !pMob )
    {
      send_to_char( "No such mobile.\n\r", ch );
      return FALSE;
    }

  SET_BIT( pMob->area->area_flags, AREA_CHANGED );

  if( top_vnum_mob == index )
    for( i = 1; i < index; i++ )
      if( get_mob_index( i ) )
  top_vnum_mob = i;

  top_mob_index--;

  /* Now crush all resets and take out mobs while were at it */
  rcount = 0;
  mcount = 0;
  
  for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
      for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
  {

    for( wch = pRoom->people; wch; wch = wnext )
      {
        wnext = wch->next_in_room;
        if( wch->pIndexData == pMob )
    {
      extract_char( wch, TRUE );
      mcount++;
    }
      }

    for( pReset = pRoom->reset_first; pReset; pReset = wReset )
      {
        wReset = pReset->next;
        switch( pReset->command )
    {
    case 'M':
      if( pReset->arg1 == index )
        {
          foundmob = TRUE;

          unlink_reset( pRoom, pReset );
          free_reset_data( pReset );

          rcount++;
          SET_BIT( pRoom->area->area_flags,
             AREA_CHANGED );

        }
      else
        foundmob = FALSE;

      break;
    case 'E':
    case 'G':
      if( foundmob )
        {
          foundobj = TRUE;

          unlink_reset( pRoom, pReset );
          free_reset_data( pReset );

          rcount++;
          SET_BIT( pRoom->area->area_flags,
             AREA_CHANGED );

        }
      else
        foundobj = FALSE;

      break;
    case '0':
      foundobj = FALSE;
      break;
    case 'P':
      if( foundobj && foundmob )
        {
          unlink_reset( pRoom, pReset );
          free_reset_data( pReset );

          rcount++;
          SET_BIT( pRoom->area->area_flags,
             AREA_CHANGED );
        }
    }
      }
  }
    }

  unlink_mob_index( pMob );

  pMob->area = NULL;
  pMob->vnum = 0;

  free_mob_index( pMob );

  printf_to_char( ch, "Removed mobile vnum {C%d{x and"
      " {C%d{x resets.\n\r", index, rcount );
  printf_to_char( ch, "{C%d{x mobiles were extracted"
      " from the mud.\n\r",mcount );
  return TRUE;
}

void glist( CHAR_DATA *ch, char *argument , int search_type)
{
  ROOM_INDEX_DATA  *pRoomIndex;
  MOB_INDEX_DATA  *pMobIndex;
  OBJ_INDEX_DATA  *pObjIndex;
  MPROG_CODE        *mprog;
  AREA_DATA    *pArea = NULL;
  char    buf  [ MAX_STRING_LENGTH   ];
  BUFFER    *buffer;
  char    arg  [ MAX_INPUT_LENGTH    ];
  int vnum;
  int nMatch = 1;
  int  col = 0;

  argument = one_argument( argument, arg );

  if (IS_NULLSTR( arg ) )
    pArea = ch->in_room->area;
  else if ( is_number( arg ) )
  {
    if ( ( pArea = get_area_data( atoi( arg ) ) ) == NULL )
  {
    send_to_char( "No such area vnum.\n\r", ch );
    return;
  }
  }
  else if ((pArea = get_area_world(arg)) == NULL)
  {
    switch (search_type)
  {
  case LIST_SEARCH_ROOM:
    send_to_char("Syntax:  rlist [area vnum/area name]\n\r", ch);
    break;
  case LIST_SEARCH_MOB:
    send_to_char("Syntax:  mlist [area vnum/area name]\n\r", ch);
    break;
  case LIST_SEARCH_OBJ:
    send_to_char("Syntax:  olist [area vnum/area name]\n\r", ch);
    break;
    case LIST_SEARCH_MPROG:
      send_to_char("Syntax:  mplist [area vnum/area name]\n\r", ch );
      break;
  }
    return;
  }

  if ( pArea == NULL )
  {
    send_to_char( "No such area.\n\r", ch );
    return;
  }

  if (!IS_BUILDER( ch, pArea ) )
  {
    send_to_char( "Insufficient security to view room lists.\n\r", ch );
    return;
  }

  buffer = new_buf();

  for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
  {
    if (search_type == LIST_SEARCH_ROOM)
    {
    if ( ( pRoomIndex = get_room_index( vnum ) ) == NULL )
      continue;

    strncpy_color( buf, FIX_STR( pRoomIndex->name, "(none)", "(null)" ),
       15, ( col + 1 ) % 3 == 0 ? '\0' : ' ', TRUE );
  }
    if (search_type == LIST_SEARCH_MOB)
  {
    if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
      continue;

    strncpy_color( buf,
       FIX_STR( pMobIndex->short_descr, "(none)", "(null)" ),
       15, ( col + 1 ) % 3 == 0 ? '\0' : ' ', TRUE );
  }
    if (search_type == LIST_SEARCH_OBJ)
  {
    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
      continue;

    strncpy_color( buf,
       FIX_STR( pObjIndex->short_descr, "(none)", "(null)" ),
       15, ( col + 1 ) % 3 == 0 ? '\0' : ' ', TRUE );
  }
    if ( search_type == LIST_SEARCH_MPROG )
    {
      if ( ( mprog = get_mprog_index( vnum ) ) == NULL )
        continue;

      strncpy_color( buf,
             FIX_STR( mprog->name, "(none)", "(NULL)" ),
             15, ( col + 1 ) % 3 == 0 ? '\0' : ' ', TRUE );
    }

      if ( ++col % 3 == 0 )
      bprintf( buffer, "[%5d] %s\n\r{x", vnum, buf );
      else
      bprintf( buffer, "[%5d] %s{x ", vnum, buf );

      if ( ++nMatch >= 300 )
      break;
  }

  if ( col % 3 != 0 )
    bstrcat( buffer, "\n\r" );

  if ( nMatch > 0 )
    {
    switch (search_type)
  {
      case LIST_SEARCH_ROOM:
      bprintf( buffer, "%d room%s listed.%s\n\r",
       nMatch - 1, nMatch == 1 ? "" : "s", nMatch == 300 ? " (More)" : "" );
      break;
    case LIST_SEARCH_MOB:
      bprintf( buffer, "%d mobile%s listed.%s\n\r",
       nMatch -1, nMatch == 1 ? "" : "s", nMatch == 300 ? " (More)" : "" );
      break;
    case LIST_SEARCH_OBJ:
      bprintf( buffer, "%d object%s listed.%s\n\r",
            nMatch - 1, nMatch == 1 ? "" : "s", nMatch == 300 ? " (More)" : "" );
      break;
      case LIST_SEARCH_MPROG:
        bprintf( buffer, "%d MProg%s listed.%s\n\r",
           nMatch-1, nMatch == 1 ? "" : "s", nMatch == 300 ? " (More)" : "" );
        break;
  }
    page_to_char( buf_string( buffer ), ch );
  }
  else
    send_to_char( "No rooms found in the area.\n\r", ch );

  free_buf( buffer );
}

void do_mlist( CHAR_DATA *ch, char *argument )
{
/* Taeloch - old do_mlist() was a single function call:
  glist(ch,argument,LIST_SEARCH_MOB);
*/
  AREA_DATA *pArea = NULL;
  ROOM_INDEX_DATA *oRoomIndex,*pRoomIndex;
  char arg[MAX_INPUT_LENGTH];
  int vnum;
  bool rMatch = FALSE;

  argument = one_argument( argument, arg );

  if (IS_NULLSTR( arg ) )
    pArea = ch->in_room->area;
  else if ( is_number( arg ) )
  {
    if ( ( pArea = get_area_data( atoi( arg ) ) ) == NULL )
    {
      send_to_char( "No such area vnum.\n\r", ch );
      return;
    }
  }
  else if ((pArea = get_area_world(arg)) == NULL)
  {
    send_to_char("Syntax:  mlist [area vnum/area name]\n\r", ch);
    return;
  }

  if ( pArea == NULL )
  {
    send_to_char( "No such area.\n\r", ch );
    return;
  }

  for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
  {
    if ( ( pRoomIndex = get_room_index( vnum ) ) == NULL )
      continue;

    oRoomIndex = ch->in_room;
    move_to_room( ch, pRoomIndex );
    do_function(ch, &do_mlevel, "area" );
    move_to_room( ch, oRoomIndex );
    rMatch = TRUE;
    break;
  }

  if (!rMatch)
    send_to_char( "No rooms in the area, listing failed.\n\r", ch );

  return;
}

void do_olist( CHAR_DATA *ch, char *argument )
{
/* Taeloch - old do_wlist() was a single function call:
  glist(ch,argument,LIST_SEARCH_OBJ);
*/
  AREA_DATA *pArea = NULL;
  ROOM_INDEX_DATA *oRoomIndex,*pRoomIndex;
  char arg[MAX_INPUT_LENGTH];
  int vnum;
  bool rMatch = FALSE;

  argument = one_argument( argument, arg );

  if (IS_NULLSTR( arg ) )
    pArea = ch->in_room->area;
  else if ( is_number( arg ) )
  {
    if ( ( pArea = get_area_data( atoi( arg ) ) ) == NULL )
    {
      send_to_char( "No such area vnum.\n\r", ch );
      return;
    }
  }
  else if ((pArea = get_area_world(arg)) == NULL)
  {
    send_to_char("Syntax:  olist [area vnum/area name]\n\r", ch);
    return;
  }

  if ( pArea == NULL )
  {
    send_to_char( "No such area.\n\r", ch );
    return;
  }

  for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
  {
    if ( ( pRoomIndex = get_room_index( vnum ) ) == NULL )
      continue;

    oRoomIndex = ch->in_room;
    move_to_room( ch, pRoomIndex );
    do_function(ch, &do_olevel, "area" );
    move_to_room( ch, oRoomIndex );
    rMatch = TRUE;
    break;
  }

  if (!rMatch)
    send_to_char( "No rooms in the area, listing failed.\n\r", ch );

  return;
}

void do_rlist( CHAR_DATA *ch, char *argument)
{
/* Taeloch - old do_rlist() was a single function call:
  glist (ch, argument, LIST_SEARCH_ROOM);
*/

  AREA_DATA *pArea = NULL;
  ROOM_INDEX_DATA *pRoomIndex;
  EXTRA_DESCR_DATA *ped;
  RESET_DATA *pReset;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  int vnum;
  int nMatch = 1;
  int i, oresets, mresets, edescs, desclines;
  char exits[7];

  argument = one_argument( argument, arg );

  if (IS_NULLSTR( arg ) )
    pArea = ch->in_room->area;
  else if ( is_number( arg ) )
  {
    if ( ( pArea = get_area_data( atoi( arg ) ) ) == NULL )
    {
      send_to_char( "No such area vnum.\n\r", ch );
      return;
    }
  }
  else if ((pArea = get_area_world(arg)) == NULL)
  {
    send_to_char("Syntax:  rlist [area vnum/area name]\n\r", ch);
    return;
  }

  if ( pArea == NULL )
  {
    send_to_char( "No such area.\n\r", ch );
    return;
  }

  if (!IS_BUILDER( ch, pArea ) )
  {
    send_to_char( "Insufficient security to view room lists.\n\r", ch );
    return;
  }

  buffer = new_buf();

  bprintf( buffer, "{x  {cVNUM  {wRoom Name                  {BORs {RMRs {gEDs {mDLn {xSector       {YExits{x\n\r");
  bprintf( buffer, "{D ----- --------------------------- --- --- --- --- ------------ ------{x\n\r");

  for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
  {
    if ( ( pRoomIndex = get_room_index( vnum ) ) == NULL )
      continue;

    oresets = 0;
    mresets = 0;
    edescs = 0;
    desclines = 0;
    strcpy(exits, "------");

    /* Get obj/mob reset counts */
    if ( ch->in_room->reset_first )
    {
      for ( pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next )
      {
        switch ( pReset->command )
        {
          case 'M':
            mresets++;
            break;
          case 'O':
          case 'P':
          case 'G':
          case 'E':
            oresets++;
            break;
        }
      }
    }

    /* get Extra Desc count */
    for ( ped = pRoomIndex->extra_descr; ped; ped = ped->next )
    {
      edescs++;
    }

    /* Get exit list */
    if ( pRoomIndex->exit[0] )
      exits[0] = 'N';
    if ( pRoomIndex->exit[1] )
      exits[1] = 'E';
    if ( pRoomIndex->exit[2] )
      exits[2] = 'S';
    if ( pRoomIndex->exit[3] )
      exits[3] = 'W';
    if ( pRoomIndex->exit[4] )
      exits[4] = 'U';
    if ( pRoomIndex->exit[5] )
      exits[5] = 'D';

    /* get description line count */
    for (i=0; pRoomIndex->description[i] != '\0'; i++)
    {
      if (pRoomIndex->description[i] == '\n')
        desclines++;
    }

    strncpy_color( buf, FIX_STR( pRoomIndex->name, "(none)", "(null)" ), 26, ' ', TRUE );

    bprintf( buffer, " {c%5d {w%26s   {B%2d  {R%2d  {g%2d  {m%2d {x%-12s {Y%s{x\n\r",
      vnum, buf,
      oresets, mresets, edescs, desclines,
      capitalize(flag_string( sector_flags, pRoomIndex->sector_type )),
      exits);

    if ( ++nMatch >= 300 )
      break;
  }

  if ( nMatch > 0 )
  {
    bprintf( buffer, "\n\r{DArea #{x%d{D, {x%s{D: {w%d {Droom%s listed.{x\n\r",
      pArea->vnum, pArea->name, (nMatch - 1),
      nMatch == 1 ? "" : "s", nMatch == 300 ? " (More)" : "" );
      
    page_to_char( buf_string( buffer ), ch );
  }
  else
    send_to_char( "No rooms found in the area.\n\r", ch );

  free_buf( buffer );
}

void do_mplist( CHAR_DATA *ch, char *argument )
{
  glist( ch, argument, LIST_SEARCH_MPROG );
}

