/***************************************************************************
 *  File: olc.c                                                            *
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
#include "recycle.h"
#include "tables.h"
#include "olc.h"
#include "interp.h"
#include "lookup.h"

/*
 * Local functions.
 */
AREA_DATA *get_area_data  args( ( int vnum ) );


/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor( DESCRIPTOR_DATA *d )
{
  switch ( d->editor )
  {
    case ED_AREA:
      aedit( d->character, d->incomm );
      break;
    case ED_ROOM:
      redit( d->character, d->incomm );
      break;
    case ED_OBJECT:
      oedit( d->character, d->incomm );
      break;
    case ED_MOBILE:
      medit( d->character, d->incomm );
      break;
    case ED_MPCODE:
      mpedit( d->character, d->incomm );
      break;
    case ED_HELP:
      hedit( d->character, d->incomm );
      break;
    case ED_CLAN:
      cedit( d->character, d->incomm );
      break;
    default:
      return FALSE;
  }
  return TRUE;
}



char *olc_ed_name( CHAR_DATA *ch )
{
  static char buf[10];

  buf[0] = '\0';
  switch (ch->desc->editor)
  {
    case ED_AREA:
      mprintf( sizeof( buf ), buf, "AEdit" );
      break;
    case ED_ROOM:
      mprintf( sizeof( buf ), buf, "REdit" );
      break;
    case ED_OBJECT:
      mprintf( sizeof( buf ), buf, "OEdit" );
      break;
    case ED_MOBILE:
      mprintf( sizeof( buf ), buf, "MEdit" );
      break;
    case ED_MPCODE:
      mprintf( sizeof( buf ), buf, "MPEdit" );
      break;
    case ED_HELP:
      mprintf( sizeof( buf ), buf, "HEdit" );
      break;
    case ED_CLAN:
      mprintf( sizeof( buf ), buf, "CEdit" );
      break;
    default:
      mprintf(sizeof(buf),buf, " " );
      break;
  }
  return buf;
}



char *olc_ed_vnum( CHAR_DATA *ch )
{
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *pRoom;
  OBJ_INDEX_DATA *pObj;
  MOB_INDEX_DATA *pMob;
  MPROG_CODE *pMprog;
  HELP_DATA *pHelp;
  CLAN_DATA *clan;
  static char buf[MIL];

  buf[0] = '\0';
  switch ( ch->desc->editor )
  {
    case ED_AREA:
      pArea = (AREA_DATA *)ch->desc->pEdit;
      mprintf(sizeof(buf),buf, "%d", pArea ? pArea->vnum : 0 );
      break;
    case ED_ROOM:
      pRoom = ch->in_room;
      mprintf(sizeof(buf),buf, "%d", pRoom ? pRoom->vnum : 0 );
      break;
    case ED_OBJECT:
      pObj = (OBJ_INDEX_DATA *)ch->desc->pEdit;
      mprintf(sizeof(buf),buf, "%d", pObj ? pObj->vnum : 0 );
      break;
    case ED_MOBILE:
      pMob = (MOB_INDEX_DATA *)ch->desc->pEdit;
      mprintf(sizeof(buf),buf, "%d", pMob ? pMob->vnum : 0 );
      break;
    case ED_MPCODE:
      pMprog = (MPROG_CODE *)ch->desc->pEdit;
      mprintf(sizeof(buf),buf, "%d", pMprog ? pMprog->vnum : 0 );
      break;
    case ED_HELP:
      pHelp = (HELP_DATA *)ch->desc->pEdit;
      mprintf(sizeof(buf),buf, "%s", pHelp ? pHelp->keyword : "" );
      break;
    case ED_CLAN:
      clan = ( CLAN_DATA *)ch->desc->pEdit;
      mprintf( sizeof( buf ), buf, "%s", clan ? clan->name : "" );
      break;
    default:
      mprintf(sizeof(buf),buf, " " );
      break;
  }

  return buf;
}

/* little extra to help editing a whole area */
void do_next( CHAR_DATA *ch, char *arg )
{
  ROOM_INDEX_DATA   *room,  *old_room;
  OBJ_INDEX_DATA    *obj,   *old_obj;
  MOB_INDEX_DATA    *mob,   *old_mob;
  //MPROG_CODE        *mpcode,*old_mpcode;
  int value, max_vnum;
  char *outstring = "There are no more %ss in this area.\n";

  switch ( ch->desc->editor )
  {
    case ED_ROOM:
      if ( ch->in_room )
        room    = ch->in_room;
      else
        room    = (ROOM_INDEX_DATA *)ch->desc->pEdit;
      max_vnum  =  room->area->max_vnum;
      old_room  =  room;

      for ( value = room->vnum + 1; value <= max_vnum; value++ )
      {
        if ( ( room = get_room_index( value ) ) )
          break;
      }
      if ( room == NULL || room == old_room )
        printf_to_char( ch, outstring, "room" );
      else
      {
        ch->desc->pEdit = (void *)room;
        move_to_room( ch, room );
      }
      break;

    case ED_MOBILE:
      mob       = (MOB_INDEX_DATA *)ch->desc->pEdit;
      max_vnum  =  mob->area->max_vnum;
      old_mob   =  mob;

      for ( value = mob->vnum + 1; value <= max_vnum; value++ )
      {
        if ( ( mob = get_mob_index( value ) ) )
          break;
      }
      if ( mob == NULL || mob == old_mob )
        printf_to_char( ch, outstring, "mobile" );
      else
        ch->desc->pEdit = (void *)mob;
      break;

    case ED_OBJECT:
      obj       = (OBJ_INDEX_DATA *)ch->desc->pEdit;
      max_vnum  =  obj->area->max_vnum;
      old_obj   =  obj;

      for ( value = obj->vnum + 1; value <= max_vnum; value++ )
      {
        if ( ( obj = get_obj_index( value ) ) )
          break;
      }
      if ( obj == NULL || obj == old_obj )
        printf_to_char( ch, outstring, "object" );
      else
        ch->desc->pEdit = (void *)obj;
      break;
/* This crashes on mpcode->vnum
    case ED_MPCODE: // Added by Taeloch
      mpcode    = (MPROG_CODE *)ch->desc->pEdit;
      max_vnum  =  mpcode->area->max_vnum;
      old_mpcode=  mpcode;

      if (mpcode != NULL)
      {
        for ( value = mpcode->vnum + 1; value <= max_vnum; value++ )
        {
          if ( (mpcode = get_mprog_index(value)) != NULL )
            break;
        }
      }
      if ( mpcode == NULL || mpcode == old_mpcode )
        printf_to_char( ch, outstring, "mprog" );
      else
        ch->desc->pEdit = (void *)mpcode;
      break;
*/

    default:
      send_to_char( "There is no next of this kind.\n\r", ch );
      break;
  }
}

/*****************************************************************************
 Name:    show_olc_cmds
 Purpose:  Format up the commands from given table.
 Called by:  show_commands(olc_act.c).
 ****************************************************************************/
void show_olc_cmds( CHAR_DATA *ch, const struct olc_cmd_type *olc_table )
{
  char buf  [ MAX_STRING_LENGTH ];
  char buf1 [ MAX_STRING_LENGTH ];
  int  cmd;
  int  col;

  buf1[0] = '\0';
  col = 0;
  for (cmd = 0; olc_table[cmd].name != NULL; cmd++)
  {
    mprintf(sizeof(buf),buf, "%-15.15s", olc_table[cmd].name );
    strcat( buf1, buf );
    if ( ++col % 5 == 0 )
      strcat( buf1, "\n\r" );
  }

  if ( col % 5 != 0 )
    strcat( buf1, "\n\r" );

  send_to_char( buf1, ch );
  return;
}



/*****************************************************************************
 Name:    show_commands
 Purpose:  Display all olc commands.
 Called by:  olc interpreters.
 ****************************************************************************/
bool show_commands( CHAR_DATA *ch, char *argument )
{
  switch (ch->desc->editor)
  {
    case ED_AREA:
      show_olc_cmds( ch, aedit_table );
      break;
    case ED_ROOM:
      show_olc_cmds( ch, redit_table );
      break;
    case ED_OBJECT:
      show_olc_cmds( ch, oedit_table );
      break;
    case ED_MOBILE:
      show_olc_cmds( ch, medit_table );
      break;
    case ED_MPCODE:
      show_olc_cmds( ch, mpedit_table );
      break;
    case ED_HELP:
      show_olc_cmds( ch, hedit_table );
    case ED_CLAN:
      show_olc_cmds( ch, cedit_table );
      break;
  }

  return FALSE;
}



/*****************************************************************************
 *                           Interpreter Tables.                             *
 *****************************************************************************/

/*
 * Clan Editor - Robert Leonard 10/06
 */
const struct olc_cmd_type cedit_table[] =
{
//  { command,    function              },
  { "commands",    show_commands       },
  { "show",        cedit_show          },
  { "recall",      cedit_recall        },
  { "pit",         cedit_pit           },
  { "gem",         cedit_gem           },
  { "balance",     cedit_balance       },
  { "spent",       cedit_spent         },
  { "total",       cedit_total         },
  { "patron",      cedit_immortal      },
  { "symbol",      cedit_symbol        },
//    { "create",      cedit_create        },
  { "status",      cedit_status        },
  { "max_rank",    cedit_max_rank      },
  { "male_rank",   cedit_male_rank     },
  { "female_rank", cedit_female_rank   },
  { "neutral_rank",cedit_neutral_rank },
  { "rem_rank",    cedit_remove_rank   },
  { "add_rank",    cedit_add_rank      },
  { "rank_flags",  cedit_rank_flags    },
  { "rank_level",  cedit_rank_level    },
  { "locker",      cedit_locker        },
  { "roster_style",cedit_roster_style  },
  { "?",           show_help           },
  { NULL,          0                   }
};

/* Help Editor - kermit 1/98 */
const struct olc_cmd_type hedit_table[] =
{
  /*  {   command    function  }, */

  { "commands", show_commands  },
  { "desc",     hedit_desc     },
  { "keywords", hedit_keywords },
  { "synopsis", hedit_synopsis },
  { "level",    hedit_level    },
  { "make",     hedit_make     },
  { "show",     hedit_show     },
  { "delete",   hedit_delete   },
  {   "?",      show_help      },

  {  NULL, 0, }
};

const struct olc_cmd_type aedit_table[] =
{
  /*  { command    function  }, */
  { "age",        aedit_age       },
  { "builder",    aedit_builder   },
  { "commands",   show_commands   },
  { "create",     aedit_create    },
  { "filename",   aedit_file      },
  { "name",       aedit_name      },
  /*  { "recall",     aedit_recall    }, */
  { "reset",      aedit_reset     },
  { "security",   aedit_security  },
  { "show",       aedit_show      },
  { "vnum",       aedit_vnum      },
  { "lvnum",      aedit_lvnum     },
  { "uvnum",      aedit_uvnum     },
  { "low",        aedit_low       },
  { "high",       aedit_high      },
  { "credits",    aedit_credits   },
  { "?",          show_help       },
  { "version",    show_version    },
  { "sysflag",    aedit_sysflag   },
  { "continent",  aedit_continent },
  { "reset_rate", aedit_reset_rate},
  { "reload",     aedit_reload    },
  { "align",      aedit_align     },
  { NULL,         0,              }
};



const struct olc_cmd_type redit_table[] =
{
  /*  {   command    function  }, */

  {   "commands",  show_commands  },
  {   "create",  redit_create  },
  {   "desc",    redit_desc  },
  {   "ed",    redit_ed  },
  {   "format",  redit_format  },
  {   "name",    redit_name  },
  {  "show",    redit_show  },
  {   "heal",    redit_heal  },
  {  "mana",    redit_mana  },
  {   "clan",    redit_clan  },

  {   "north",  redit_north  },
  {   "south",  redit_south  },
  {   "east",    redit_east  },
  {   "west",    redit_west  },
  {   "up",    redit_up  },
  {   "down",    redit_down  },

  /* New reset commands. */
  {  "mreset",  redit_mreset  },
  {  "oreset",  redit_oreset  },
  {  "mshow",  redit_mshow      },
  {  "oshow",  redit_oshow      },
  {   "owner",  redit_owner      },
  {  "room",    redit_room      },
  {  "sector",  redit_sector  },
  {   "addmprog", redit_addmprog  },
  {   "delmprog", redit_delmprog  },
  {  "copy",    redit_copy    },
  {  "delete",  redit_delete  },
  {   "state",    redit_state     },
  {   "?",    show_help     },
  {   "version",  show_version  },

  {   "fish1",    redit_fish1  },
  {   "fish2",    redit_fish2  },
  {   "fish3",    redit_fish3  },
  {   "fish4",    redit_fish4  },
  {   "fish5",    redit_fish5  },
  {   "fish6",    redit_fish6  },
  {   "fish7",    redit_fish7  },
  {   "fish8",    redit_fish8  },

  {  NULL,    0,    }
};



const struct olc_cmd_type oedit_table[] =
{
  /*  {   command    function  }, */

  {   "addaffect",  oedit_addaffect  },
  {  "addapply",   oedit_addapply  },
  {   "commands",      show_commands  },
  {   "cost",        oedit_cost    },
  {   "create",      oedit_create  },
  {   "delaffect",  oedit_delaffect  },
  {   "ed",        oedit_ed      },
  {   "long",     oedit_long      },
  {   "name",        oedit_name      },
  {   "short",      oedit_short   },
  {  "show",     oedit_show    },
  {   "v0",        oedit_value0  },
  {   "v1",        oedit_value1  },
  {   "v2",       oedit_value2  },
  {   "v3",        oedit_value3  },
  {   "v4",        oedit_value4  },  /* ROM */
  {   "v5",       oedit_value5  },  /* ROM */
  {   "v6",        oedit_value6  },  /* ROM */
  {   "weight",     oedit_weight  },

  {   "extra",        oedit_extra     },  /* ROM */
  {   "wear",         oedit_wear      },  /* ROM */
  {   "type",         oedit_type      },  /* ROM */
  {   "material",     oedit_material  },  /* ROM */
  {   "level",        oedit_level     },  /* ROM */
  {   "condition",    oedit_condition },  /* ROM */
  {   "addmprog",     oedit_addmprog  },
  {   "delmprog",     oedit_delmprog  },
  {  "copy",        oedit_copy      },
  {  "delete",      oedit_delete  },

  {   "?",        show_help     },
  {   "version",      show_version  },
  {   "clan",          oedit_clan  },

  {  NULL,        0,          }
};



const struct olc_cmd_type medit_table[] =
{
  /*  {   command    function  }, */

  {   "alignment",  medit_align          },
  {   "commands",      show_commands      },
  {   "create",     medit_create      },
  {   "desc",        medit_desc          },
  {   "level",      medit_level          },
  {   "long",        medit_long          },
  {   "long2",        medit_long2         },
  {   "name",        medit_name        },
  {   "path",         medit_path          },
  {   "riders",       medit_max_riders    },
  {   "shop",     medit_shop        },
  {   "short",      medit_short       },
  {  "show",        medit_show          },
  {   "spec",        medit_spec        },
  {   "sex",          medit_sex           },  /* ROM */
  {   "act",          medit_act           },  /* ROM */
  {   "act2",         medit_act2          },
  /*{   "plr2",    medit_plr2  },*/
  {   "affect",       medit_affect        },  /* ROM */
  {   "affect2",      medit_affect2       },
  {   "aff2",         medit_affect2       },
  {   "armor",        medit_ac            },  /* ROM */
  {   "form",         medit_form          },  /* ROM */
  {   "part",         medit_part          },  /* ROM */
  {   "imm",          medit_imm           },  /* ROM */
  {   "res",          medit_res           },  /* ROM */
  {   "vuln",         medit_vuln          },  /* ROM */
  {   "material",     medit_material      },  /* ROM */
  {   "off",          medit_off           },  /* ROM */
  {   "size",         medit_size          },  /* ROM */
  {   "hitdice",      medit_hitdice       },  /* ROM */
  {   "manadice",     medit_manadice      },  /* ROM */
  {   "damdice",      medit_damdice       },  /* ROM */
  {   "race",         medit_race          },  /* ROM */
  {   "position",     medit_position      },  /* ROM */
  {   "wealth",       medit_gold          },  /* ROM */
  {   "hitroll",      medit_hitroll       },  /* ROM */
  {  "damtype",    medit_damtype     },  /* ROM */
  {   "group",      medit_group       },  /* ROM */
  {   "addmprog",   medit_addmprog      },  /* ROM */
  {  "delmprog",      medit_delmprog      },  /* ROM */
  {  "maxcount",   medit_maxcount    },  /* ROM */
  {  "copy",        medit_copy          },
  {  "delete",     medit_delete      },
  {  "statcopy",      medit_statcopy    },
  {   "?",        show_help         },
  {   "version",    show_version      },
  {   "frags",        medit_frag_number   },

  {  NULL,        0,                }
};

/*****************************************************************************
 *                          End Interpreter Tables.                          *
 *****************************************************************************/



/*****************************************************************************
 Name:    get_area_data
 Purpose:  Returns pointer to area with given vnum.
 Called by:  do_aedit(olc.c).
 ****************************************************************************/
AREA_DATA *get_area_data( int vnum )
{
  AREA_DATA *pArea;

  for (pArea = area_first; pArea; pArea = pArea->next )
  {
    if (pArea->vnum == vnum)
      return pArea;
  }

  return 0;
}



/*****************************************************************************
 Name:    edit_done
 Purpose:  Resets builder information on completion.
 Called by:  aedit, redit, oedit, medit(olc.c)
 ****************************************************************************/
bool edit_done( CHAR_DATA *ch )
{
  ch->desc->pEdit = NULL;
  ch->desc->editor = 0;
  return FALSE;
}



/*****************************************************************************
 *                              Interpreters.                                *
 *****************************************************************************/


/* Area Interpreter, called by do_aedit. */
void aedit( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int  cmd;
  int64  value;

  EDIT_AREA(ch, pArea);
  smash_tilde( argument );
  strcpy( arg, argument );
  argument = one_argument( argument, command );

  /*#if !defined(TEST_MUD)
    if (get_trust(ch) < IMMORTAL){
        send_to_char( "AEdit:  Insufficient security to modify area.\n\r", ch );
        edit_done( ch );
        return;
  }
  #endif
    if ( !IS_BUILDER( ch, pArea ) )
      {
        send_to_char( "AEdit:  Insufficient security to modify area.\n\r", ch );
        edit_done( ch );
        return;
      }*/

  if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_AREA ) )
  {
    send_to_char( "Insufficient security to edit areas.\n\r", ch );
    return;
  }

  if ( !str_cmp(command, "done") )
  {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' )
  {
    aedit_show( ch, argument );
    return;
  }

  if ( ( value = flag_value( area_flags, command ) ) != NO_FLAG )
  {
    TOGGLE_BIT(pArea->area_flags, value);
    send_to_char( "Flag toggled.\n\r", ch );
    return;
  }

  /* Search Table and Dispatch Command. */
  for ( cmd = 0; aedit_table[cmd].name != NULL; cmd++ )
  {
    if ( !str_prefix( command, aedit_table[cmd].name ) )
    {
      if ( (*aedit_table[cmd].olc_fun) ( ch, argument ) )
      {
        SET_BIT( pArea->area_flags, AREA_CHANGED );
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



/* Room Interpreter, called by do_redit. */
void redit( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *pRoom;
  char arg[MAX_STRING_LENGTH];
  char command[MAX_INPUT_LENGTH];
  int  cmd;

  EDIT_ROOM(ch, pRoom);
  pArea = pRoom->area;

  smash_tilde( argument );
  strcpy( arg, argument );
  argument = one_argument( argument, command );


  if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_ROOM ) )
  {
    send_to_char( "Insufficient security to edit rooms.\n\r", ch );
    return;
  }

  if ( !str_cmp(command, "done") )
  {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' )
  {
    redit_show( ch, argument );
    return;
  }

  /* Search Table and Dispatch Command. */
  for ( cmd = 0; redit_table[cmd].name != NULL; cmd++ )
  {
    if ( !str_prefix( command, redit_table[cmd].name ) )
    {
      if ( (*redit_table[cmd].olc_fun) ( ch, argument ) )
      {
        SET_BIT( pArea->area_flags, AREA_CHANGED );
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



/* Help Editor - kermit 1/98 */
void hedit( CHAR_DATA *ch, char *argument)
{
  char command[MIL];
  char arg[MIL];
  int cmd;

  smash_tilde(argument);
  strcpy(arg, argument);
  argument = one_argument( argument, command);

  if ( !str_cmp(command, "done") )
  {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' )
  {
    hedit_show( ch, argument );
    return;
  }

  for ( cmd = 0; hedit_table[cmd].name != NULL; cmd++ )
  {
    if ( !str_prefix( command, hedit_table[cmd].name ) )
    {
      (*hedit_table[cmd].olc_fun) ( ch, argument );
      return;
    }
  }

  interpret( ch, arg );
  return;
}

/* Help Editor - kermit 1/98 */
void do_hedit( CHAR_DATA *ch, char *argument )
{
  HELP_DATA *pHelp;
  char arg1[MIL];
  char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
  bool found = FALSE;
  char buf[MAX_STRING_LENGTH];

  strcpy(arg1,argument);

  mprintf(sizeof(buf), buf, "HEDIT %s{x", argument );
  wiznet( buf, NULL, NULL, WIZ_OLC, 0, get_trust( ch ) );

  if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_HELP ) )
  {
    send_to_char( "Insufficient security to edit helps.\n\r", ch );
    return;
  }


  if (argument[0] != '\0')
  {
    /* Taken from do_help */
    argall[0] = '\0';
    while (argument[0] != '\0' )
    {
      argument = one_argument(argument,argone);
      if (argall[0] != '\0')
        strcat(argall," ");
      strcat(argall,argone);
    }

    if (is_number(argall))
    {
      int vnum= atoi(argall);
      for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
      {
        if ( pHelp->vnum == vnum )
        {
          ch->desc->pEdit=(void *)pHelp;
          ch->desc->editor= ED_HELP;
          found = TRUE;
          return;
        }
      }
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
      if ( is_name( argall, pHelp->keyword ) )
      {
        ch->desc->pEdit=(void *)pHelp;
        ch->desc->editor= ED_HELP;
        found = TRUE;
        return;
      }
    }
  } // if non-null argument

  if (!found)
  {
    argument = one_argument(arg1, arg1);

    if (!str_cmp(arg1,"make"))
    {
      if (argument[0] == '\0')
      {
        send_to_char("Syntax: hedit make [topic]\n\r",ch);
        return;
      }

      if (hedit_make(ch, argument) )
        ch->desc->editor = ED_HELP;

      return;
    }
    /*
        if(!str_cmp(arg1,"delete"))
        {
          if (argument[0] == '\0')
          {
            send_to_char("Syntax: hedit delete [topic]\n\r",ch);
            return;
          }

    //      if (hedit_delete(ch, argument) )
    //        ch->desc->editor = ED_HELP;
          hedit_delete(ch, argument);

          return;
        }
    */
  }

  send_to_char( "HEdit:  There is no default help to edit.\n\r", ch );
  return;
}

/* Object Interpreter, called by do_oedit. */
void oedit( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea;
  OBJ_INDEX_DATA *pObj;
  char arg[MAX_STRING_LENGTH];
  char command[MAX_INPUT_LENGTH];
  int  cmd;

  smash_tilde( argument );
  strcpy( arg, argument );
  argument = one_argument( argument, command );

  EDIT_OBJ(ch, pObj);
  pArea = pObj->area;

  if (!pArea)
  {
    send_to_char( "ERROR: That object does not have an area associated with it.  Talk to your OWNER.\n\r",ch);
    bugf( "ERROR: That object does not have an area associated with it.  Talk to your OWNER.");
    return;
  }

  /*if ( !IS_BUILDER( ch, pArea ) )
    {
      send_to_char( "OEdit: Insufficient security to modify area.\n\r", ch );
      edit_done( ch );
      return;
    }*/

  if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_OBJECT ) )
  {
    send_to_char( "Insufficient security to edit objects.\n\r", ch );
    return;
  }

  if ( !str_cmp(command, "done") )
  {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' )
  {
    oedit_show( ch, argument );
    return;
  }

  /* Search Table and Dispatch Command. */
  for ( cmd = 0; oedit_table[cmd].name != NULL; cmd++ )
  {
    if ( !str_prefix( command, oedit_table[cmd].name ) )
    {
      if ( (*oedit_table[cmd].olc_fun) ( ch, argument ) )
      {
        SET_BIT( pArea->area_flags, AREA_CHANGED );
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



/* Mobile Interpreter, called by do_medit. */
void medit( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea;
  MOB_INDEX_DATA *pMob;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_STRING_LENGTH];
  int  cmd;

  smash_tilde( argument );
  strcpy( arg, argument );
  argument = one_argument( argument, command );

  EDIT_MOB(ch, pMob);
  pArea = pMob->area;

  /*if ( !IS_BUILDER( ch, pArea ) )
    {
      send_to_char( "MEdit: Insufficient security to modify area.\n\r", ch );
      edit_done( ch );
      return;
    }*/

  if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_MOBILE ) )
  {
    send_to_char( "Insufficient security to edit mobiles.\n\r", ch );
    return;
  }

  if ( !str_cmp(command, "done") )
  {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' )
  {
    medit_show( ch, argument );
    return;
  }

  /* Search Table and Dispatch Command. */
  for ( cmd = 0; medit_table[cmd].name != NULL; cmd++ )
  {
    if ( !str_prefix( command, medit_table[cmd].name ) )
    {
      if ( (*medit_table[cmd].olc_fun) ( ch, argument ) )
      {
        SET_BIT( pArea->area_flags, AREA_CHANGED );
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




const struct editor_cmd_type editor_table[] =
{
  /*  {   command    function  }, */

  {   "area",    do_aedit  },
  {   "room",    do_redit  },
  {   "object",      do_oedit  },
  {   "mobile",      do_medit  },
  {    "mpcode",      do_mpedit  },
  {   "hedit",      do_hedit    },
  {   "clan",       do_cedit    },
  {  NULL,        0,        }
};


/* Entry point for all editors. */
void do_olc( CHAR_DATA *ch, char *argument )
{
  char command[MAX_INPUT_LENGTH];
  int  cmd;

  if ( IS_NPC(ch) )
    return;

  argument = one_argument( argument, command );

  if ( command[0] == '\0' )
  {
    do_function(ch, &do_help, "olc" );
    return;
  }

  /* Search Table and Dispatch Command. */
  for ( cmd = 0; editor_table[cmd].name != NULL; cmd++ )
  {
    if ( !str_prefix( command, editor_table[cmd].name ) )
    {
      (*editor_table[cmd].do_fun) ( ch, argument );
      return;
    }
  }

  /* Invalid command, send help. */
  do_function(ch, &do_help, "olc" );
  return;
}



/* Entry point for editing area_data. */
void do_aedit( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea;
  int value;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;

  mprintf(sizeof(buf), buf, "AEDIT %s{x", argument );
  wiznet( buf, NULL, NULL, WIZ_OLC, 0, get_trust( ch ) );

  if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_AREA ) )
  {
    send_to_char( "Insufficient security to edit areas.\n\r", ch );
    return;
  }

  pArea  = ch->in_room->area;

  argument  = one_argument(argument,arg);

  if ( is_number( arg ) )
  {
    value = atoi( arg );
    if ( !( pArea = get_area_data( value ) ) )
    {
      send_to_char( "That area vnum does not exist.\n\r", ch );
      return;
    }
  }
  else
    if ( !str_cmp( arg, "create" ) )
    {
      if ( ch->pcdata->security < 9 )
      {
        send_to_char( "AEdit : Insufficient Security to modify area.\n\r", ch );
        return;
      }

     //Removing until she decides to play again.
     /* if ( !is_name(ch->name, "Laurelin"))
      {
        send_to_char(
          "All new areas must be approved by Laurelin, see her for creation.\n\r", ch );
        return;
      }*/


      aedit_create( ch, "" );
      ch->desc->editor = ED_AREA;
      return;
    }

  if (!IS_BUILDER(ch,pArea))
  {
    send_to_char("Insufficient security to edit the areas.\n\r",ch);
    return;
  }

  ch->desc->pEdit = (void *)pArea;
  ch->desc->editor = ED_AREA;
  return;
}



/* Entry point for editing room_index_data. */
void do_redit( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *pRoom;
  char arg1[MAX_STRING_LENGTH];
  int temp_room_players=0;
  bool temp_empty = TRUE;

  if ( IS_NPC(ch) )
    return;

  if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_ROOM ) )
  {
    send_to_char( "Insufficient security to edit rooms.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg1 );

  pRoom = ch->in_room;
  if ((is_revered_room(pRoom->vnum)) && (!IS_IMPLEMENTOR(ch)))
  {
    send_to_char("Sorry that is a Revered and protected Vnum\n\r",ch);
    return;
  }

  if ( !str_cmp( arg1, "reset" ) )  /* redit reset */
  {
    if ( !IS_BUILDER( ch, pRoom->area ) )
    {
      send_to_char( "Insufficent Security for OLC.\n\r" , ch );
      return;
    }

    /* To allow a redit reset to work. */
    temp_room_players = pRoom->area->nplayer;
    temp_empty        = pRoom->area->empty;
    pRoom->area->nplayer  = 0;
    pRoom->area->empty    = TRUE;
    reset_room( pRoom );
    pRoom->area->nplayer  = temp_room_players;
    pRoom->area->empty    = temp_empty;
    send_to_char( "Room reset.\n\r", ch );

    return;
  }
  else if ( !str_cmp( arg1, "delete" ) )
  {
    redit_delete( ch, argument ); /* <--- RIGHT HERE */
    return;
  }
  else
    if ( !str_cmp( arg1, "create" ) )  /* redit create <vnum> */
    {
      if ( argument[0] == '\0' || atoi( argument ) == 0 )
      {
        send_to_char( "Syntax:  redit create [vnum]\n\r", ch );
        return;
      }

      if ( redit_create( ch, argument ) ) /* pEdit == nuevo cuarto */
      {
        ch->desc->editor = ED_ROOM;
        move_to_room( ch, ch->desc->pEdit );
        SET_BIT( ((ROOM_INDEX_DATA *)ch->desc->pEdit)->area->area_flags, AREA_CHANGED );
      }

      return;
    }
    else if ( !IS_NULLSTR(arg1) )  /* redit <vnum> */
    {
      if ( !is_number( arg1 ) )
      {
        send_to_char( "You must enter a numeric value.\n\r", ch );
        return;
      }

      if ( !( pRoom = get_room_index( atoi( arg1 ) ) ) )
      {
        send_to_char( "REdit: That room does not exist.\n\r", ch );
        return;
      }

      int count = 0;
      CHAR_DATA *rch;

      for ( rch = pRoom->people; rch != NULL; rch = rch->next_in_room )
        count++;

      if ( !is_room_owner( ch, pRoom ) && room_is_private( pRoom )
           && ( count > 1 || get_trust( ch ) < MAX_LEVEL ) )
      {
        send_to_char( "That room is private right now.\n\r", ch );
        return;
      }

      if ( !IS_BUILDER(ch, pRoom->area) )
      {
        send_to_char( "REdit: Insufficient Security to edit room.\n\r", ch );
        return;
      }

      move_to_room( ch, pRoom );
    }

  if ( !IS_BUILDER(ch, pRoom->area) )
  {
    send_to_char( "REdit : Insufficient Security to edit rooms.\n\r", ch );
    return;
  }

  ch->desc->pEdit  = (void *) pRoom;
  ch->desc->editor  = ED_ROOM;

  return;
}



/* Entry point for editing obj_index_data. */
void do_oedit( CHAR_DATA *ch, char *argument )
{
  OBJ_INDEX_DATA *pObj;
  AREA_DATA *pArea;
  char arg1[MAX_STRING_LENGTH];
  int value;

  if ( IS_NPC( ch ) )
    return;

  if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_OBJECT ) )
  {
    send_to_char( "Insufficient security to edit objects.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg1 );

  if ( is_number( arg1 ) )
  {
    value = atoi( arg1 );
    if ( !( pObj = get_obj_index( value ) ) )
    {
      send_to_char( "OEdit:  That vnum does not exist.\n\r", ch );
      return;
    }

    if ( ( is_revered_obj( value ) ) && ( !IS_IMPLEMENTOR( ch ) ) )
    {
      send_to_char( "Sorry that is a Revered and protected Vnum\n\r", ch );
      return;
    }

    if ( !IS_BUILDER( ch, pObj->area ) )
    {
      send_to_char( "Insufficient Security to edit objects.\n\r", ch );
      return;
    }

    ch->desc->pEdit = (void *)pObj;
    ch->desc->editor = ED_OBJECT;
    return;
  }
  else if ( !str_cmp( arg1, "delete" ) )
  {
    oedit_delete( ch, argument ); /* <--- RIGHT HERE */
    return;
  }
  else
  {
    if ( !str_cmp( arg1, "create" ) )
    {
      value = atoi( argument );
      if ( argument[0] == '\0' || value == 0 )
      {
        send_to_char( "Syntax:  oedit create [vnum]\n\r", ch );
        return;
      }

      pArea = get_vnum_area( value );

      if ( !pArea )
      {
        send_to_char( "OEdit:  That vnum is not assigned an area.\n\r", ch );
        return;
      }

      if ( !IS_BUILDER( ch, pArea ) )
      {
        send_to_char( "Insufficient Security to edit objects.\n\r" , ch );
        return;
      }

      if ( oedit_create( ch, argument ) )
      {
        SET_BIT( pArea->area_flags, AREA_CHANGED );
        ch->desc->editor = ED_OBJECT;
      }
      return;
    }
  }
  send_to_char( "OEdit:  There is no default object to edit.\n\r", ch );
  return;
}

/* Entry point for editing mob_index_data. */
void do_medit( CHAR_DATA *ch, char *argument )
{
  MOB_INDEX_DATA *pMob;
  AREA_DATA *pArea;
  int value;
  char arg1[MAX_STRING_LENGTH];

  argument = one_argument( argument, arg1 );

  if ( IS_NPC( ch ) )
    return;

  if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_MOBILE ) )
  {
    send_to_char( "Insufficient security to edit mobs.\n\r", ch );
    return;
  }

  if ( is_number( arg1 ) )
  {
    value = atoi( arg1 );
    if ( !( pMob = get_mob_index( value ) ))
    {
      send_to_char( "MEdit:  That vnum does not exist.\n\r", ch );
      return;
    }

    if ( ( is_revered_mob( value ) ) && ( !IS_IMPLEMENTOR( ch ) ) )
    {
      send_to_char( "Sorry that is a Revered and protected Vnum\n\r", ch );
      return;
    }

    if ( !IS_BUILDER( ch, pMob->area ) )
    {
      send_to_char( "Insufficient Security to edit mobs.\n\r" , ch );
      return;
    }

    ch->desc->pEdit = (void *)pMob;
    ch->desc->editor = ED_MOBILE;
    return;
  }
  else if ( !str_cmp( arg1, "delete" ) )
  {
    medit_delete( ch, argument ); /* <--- RIGHT HERE */
    return;
  }
  else
  {
    if ( !str_cmp( arg1, "create" ) )
    {
      value = atoi( argument );
      if ( arg1[0] == '\0' || value == 0 )
      {
        send_to_char( "Syntax:  medit create [vnum]\n\r", ch );
        return;
      }

      pArea = get_vnum_area( value );

      if ( !pArea )
      {
        send_to_char( "OEdit:  That vnum is not assigned an area.\n\r", ch );
        return;
      }

      if ( !IS_BUILDER( ch, pArea ) )
      {
        send_to_char( "Insufficient Security to edit mobs.\n\r" , ch );
        return;
      }

      if ( medit_create( ch, argument ) )
      {
        SET_BIT( pArea->area_flags, AREA_CHANGED );
        ch->desc->editor = ED_MOBILE;
      }
      return;
    }
  }
  send_to_char( "MEdit:  There is no default mobile to edit.\n\r", ch );
  return;
}

void display_resets( CHAR_DATA *ch )
{
  ROOM_INDEX_DATA  *pRoom;
  RESET_DATA    *pReset;
  MOB_INDEX_DATA  *pMob = NULL;
  char     buf   [ MAX_STRING_LENGTH ];
  char     final [ MAX_STRING_LENGTH ];
  int     iReset = 0;
  char          tbuf[MSL];
  char          tbuf2[MSL];

  EDIT_ROOM( ch, pRoom );
  final[0]  = '\0';

  send_to_char ( " No.  Loads    Description       Location         Vnum   Mx Mn Description\n\r"
                 "==== ======== ============= =================== ======== ===== ===========\n\r", ch );

  for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
  {
    OBJ_INDEX_DATA  *pObj;
    MOB_INDEX_DATA  *pMobIndex;
    OBJ_INDEX_DATA  *pObjIndex;
    OBJ_INDEX_DATA  *pObjToIndex;
    ROOM_INDEX_DATA *pRoomIndex;

    final[0] = '\0';
    mprintf(sizeof(final), final, "[%2d]{x ", ++iReset );

    switch ( pReset->command )
    {
      default:
        mprintf(sizeof(buf),buf, "Bad reset command: %c.", pReset->command );
        strcat( final, buf );
        break;

      case 'M':
        if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) )
        {
          mprintf(sizeof(buf),buf, "Load Mobile - Bad Mob %d\n\r", pReset->arg1 );
          strcat( final, buf );
          continue;
        }

        if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
        {
          mprintf(sizeof(buf),buf, "Load Mobile - Bad Room %d\n\r", pReset->arg3 );
          strcat( final, buf );
          continue;
        }

        pMob = pMobIndex;
        strip_color(tbuf,pMob->short_descr);
        strip_color(tbuf2,pRoomIndex->name);
        mprintf(sizeof(buf),buf, "M[%5d] %-13.13s{x in room             R[%5d] %2d-%2d{x %-15.15s{x\n\r",
                pReset->arg1, tbuf, pReset->arg3,
                pReset->arg2, pReset->arg4, tbuf2);
        strcat( final, buf );

        /*
         * Check for pet shop.
         * -------------------
         */
        {
          ROOM_INDEX_DATA *pRoomIndexPrev;

          pRoomIndexPrev = get_room_index( pRoomIndex->vnum - 1 );
          if ( pRoomIndexPrev
               && IS_SET( pRoomIndexPrev->room_flags, ROOM_PET_SHOP ) )
            final[5] = 'P';
        }
        break;

      case 'O':
        if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
        {
          mprintf(sizeof(buf),buf, "Load Object - Bad Object %d\n\r", pReset->arg1 );
          strcat( final, buf );
          continue;
        }

        pObj = pObjIndex;

        if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
        {
          mprintf(sizeof(buf),buf, "Load Object - Bad Room %d\n\r", pReset->arg3 );
          strcat( final, buf );
          continue;
        }

        strip_color(tbuf,pObj->short_descr);
        strip_color(tbuf2,pRoomIndex->name);
        mprintf(sizeof(buf),buf, "O[%5d] %-13.13s in room             "
                "R[%5d]       %-15.15s\n\r",
                pReset->arg1, tbuf,
                pReset->arg3, tbuf2 );
        strcat( final, buf );
        break;

      case 'P':
        if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
        {
          mprintf(sizeof(buf),buf, "Put Object - Bad Object %d\n\r", pReset->arg1 );
          strcat( final, buf );
          continue;
        }

        pObj = pObjIndex;

        if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
        {
          mprintf(sizeof(buf),buf, "Put Object - Bad To Object %d\n\r", pReset->arg3 );
          strcat( final, buf );
          continue;
        }

        strip_color(tbuf,pObj->short_descr);
        strip_color(tbuf2,pObjToIndex->short_descr);
        mprintf(sizeof(buf),buf,
                "O[%5d] %-13.13s inside              O[%5d] %2d-%2d %-15.15s\n\r",
                pReset->arg1,
                tbuf,
                pReset->arg3,
                pReset->arg2,
                pReset->arg4,
                tbuf2 );
        strcat( final, buf );
        break;

      case 'G':
      case 'E':
        if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
        {
          mprintf(sizeof(buf),buf, "Give/Equip Object - Bad Object %d\n\r", pReset->arg1 );
          strcat( final, buf );
          continue;
        }
        pObj = pObjIndex;

        if ( !pMob )
        {
          mprintf(sizeof(buf),buf, "Give/Equip Object - No Previous Mobile\n\r" );
          strcat( final, buf );
          break;
        }

        if ( pMob->pShop )
        {
          strip_color(tbuf,pObj->short_descr);
          strip_color(tbuf2,pMob->short_descr);

          mprintf(sizeof(buf),buf,
                  "O[%5d] %-13.13s in the inventory of S[%5d]       %-15.15s\n\r",
                  pReset->arg1,
                  tbuf,
                  pMob->vnum,
                  tbuf2  );
        }
        else
        {
          strip_color(tbuf,pObj->short_descr);
          strip_color(tbuf2,pMob->short_descr);
          mprintf(sizeof(buf),buf,
                  "O[%5d] %-13.13s %-19.19s M[%5d]       %-15.15s\n\r",
                  pReset->arg1,
                  tbuf,
                  (pReset->command == 'G') ? flag_string( wear_loc_strings, WEAR_NONE ) : flag_string( wear_loc_strings, pReset->arg3 ),
                  pMob->vnum,
                  tbuf2 );
        }
        strcat( final, buf );
        break;

        /*
         * Doors are set in rs_flags don't need to be displayed.
         * If you want to display them then uncomment the new_reset
         * line in the case 'D' in load_resets in db.c and here.
         */
      case 'D':
        pRoomIndex = get_room_index( pReset->arg1 );
        mprintf(sizeof(buf),buf, "R[%5d] %s door of %-19.19s reset to %s\n\r",
                pReset->arg1,
                capitalize( dir_name[ pReset->arg2 ] ),
                pRoomIndex->name,
                flag_string( door_resets, pReset->arg3 ) );
        strcat( final, buf );
        break;

        /*
         * End Doors Comment.
         */
      case 'R':
        if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) )
        {
          mprintf(sizeof(buf),buf, "Randomize Exits - Bad Room %d\n\r", pReset->arg1 );
          strcat( final, buf );
          continue;
        }

        mprintf(sizeof(buf),buf, "R[%5d] Exits are randomized in %s\n\r", pReset->arg1, pRoomIndex->name );
        strcat( final, buf );
        break;
    }
    send_to_char( final, ch );
  }

  return;
}



/*****************************************************************************
 Name:    add_reset
 Purpose:  Inserts a new reset in the given index slot.
 Called by:  do_resets(olc.c).
 ****************************************************************************/
void add_reset( ROOM_INDEX_DATA *room, RESET_DATA *pReset, int index )
{
  RESET_DATA *reset;
  int iReset = 0;

  if ( !room->reset_first )
  {
    room->reset_first  = pReset;
    room->reset_last  = pReset;
    pReset->next    = NULL;
    return;
  }

  index--;

  if ( index == 0 )  /* First slot (1) selected. */
  {
    pReset->next = room->reset_first;
    room->reset_first = pReset;
    return;
  }

  /*
   * If negative slot( <= 0 selected) then this will find the last.
   */
  for ( reset = room->reset_first; reset->next; reset = reset->next )
  {
    if ( ++iReset == index )
      break;
  }

  pReset->next  = reset->next;
  reset->next    = pReset;
  if ( !pReset->next )
    room->reset_last = pReset;
  return;
}



void do_resets( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char arg4[MAX_INPUT_LENGTH];
  char arg5[MAX_INPUT_LENGTH];
  char arg6[MAX_INPUT_LENGTH];
  char arg7[MAX_INPUT_LENGTH];
  RESET_DATA *pReset = NULL;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  argument = one_argument( argument, arg4 );
  argument = one_argument( argument, arg5 );
  argument = one_argument( argument, arg6 );
  argument = one_argument( argument, arg7 );

  if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_RESET ) )
  {
    send_to_char( "Insufficient security to edit resets.\n\r", ch );
    return;
  }


  if ( !IS_BUILDER( ch, ch->in_room->area ) )
  {
    send_to_char( "Resets: Invalid security for editing this area.\n\r",
                  ch );
    return;
  }

  /*
   * Display resets in current room.
   * -------------------------------
   */
  if ( arg1[0] == '\0' )
  {
    if ( ch->in_room->reset_first )
    {
      send_to_char(
        "Resets: M = mobile, R = room, O = object, "
        "P = pet, S = shopkeeper\n\r", ch );
      display_resets( ch );
    }
    else
      send_to_char( "No resets in this room.\n\r", ch );
  }

  SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );

  /*
   * Take index number and search for commands.
   * ------------------------------------------
   */
  if ( is_number( arg1 ) )
  {
    ROOM_INDEX_DATA *pRoom = ch->in_room;

    /*
     * Delete a reset.
     * ---------------
     */
    if ( !str_cmp( arg2, "delete" ) )
    {
      int insert_loc = atoi( arg1 );

      if ( !ch->in_room->reset_first )
      {
        send_to_char( "No resets in this area.\n\r", ch );
        return;
      }

      if ( insert_loc-1 <= 0 )
      {
        pReset = pRoom->reset_first;
        pRoom->reset_first = pRoom->reset_first->next;
        if ( !pRoom->reset_first )
          pRoom->reset_last = NULL;
      }
      else
      {
        int iReset = 0;
        RESET_DATA *prev = NULL;

        for ( pReset = pRoom->reset_first;
              pReset;
              pReset = pReset->next )
        {
          if ( ++iReset == insert_loc )
            break;
          prev = pReset;
        }

        if ( !pReset )
        {
          send_to_char( "Reset not found.\n\r", ch );
          return;
        }

        if ( prev )
          prev->next = prev->next->next;
        else
          pRoom->reset_first = pRoom->reset_first->next;

        for ( pRoom->reset_last = pRoom->reset_first;
              pRoom->reset_last->next;
              pRoom->reset_last = pRoom->reset_last->next );
      }

      free_reset_data( pReset );
      send_to_char( "Reset deleted.\n\r", ch );
    }
    else
      /*
       * Add a reset.
       * ------------
       */
      if ( (!str_cmp( arg2, "mob" ) && is_number( arg3 ))
           || (!str_cmp( arg2, "obj" ) && is_number( arg3 )) )
      {
        /*
         * Check for Mobile reset.
         * -----------------------
         */
        if ( !str_cmp( arg2, "mob" ) )
        {
          if (get_mob_index( is_number(arg3) ? atoi( arg3 ) : 1 ) == NULL)
          {
            send_to_char("No such Mob.\n\r",ch);
            return;
          }
          /* Check to see if VNUM entered is in area range */
          if (!check_vnum_range(ch, atoi(arg3)))
            return;
          pReset = new_reset_data();
          pReset->command = 'M';
          pReset->arg1    = atoi( arg3 );
          pReset->arg2    = is_number( arg4 ) ? atoi( arg4 ) : 1; /* Max # */
          pReset->arg3    = ch->in_room->vnum;
          pReset->arg4  = is_number( arg5 ) ? atoi( arg5 ) : 1; /* Min # */
        }
        else
          /*
           * Check for Object reset.
           * -----------------------
           */
          if ( !str_cmp( arg2, "obj" ) )
          {
            pReset = new_reset_data();
            pReset->arg1    = atoi( arg3 );
            /*
             * Inside another object.
             * ----------------------
             */
            if ( !str_prefix( arg4, "inside" ) )
            {
              OBJ_INDEX_DATA *temp;
              temp = get_obj_index(is_number(arg5) ?
                                   atoi(arg5) : 1);
              if (temp == NULL)
              {
                send_to_char("That is not a container.\n\r",ch);
                return;
              }
              if ( ( temp->item_type != ITEM_CONTAINER )
                   &&   ( temp->item_type != ITEM_KEYRING )
                   &&   ( temp->item_type != ITEM_CORPSE_NPC ) )
              {
                send_to_char( "That object is not a container.\n\r", ch);
                return;
              }
              if (!check_vnum_range(ch, atoi(arg5)))
                return;
              /* Check to see if VNUM entered is in area range */
              pReset->command = 'P';
              pReset->arg2    = is_number( arg6 ) ? atoi( arg6 ) : 1;
              pReset->arg3    = is_number( arg5 ) ? atoi( arg5 ) : 1;
              pReset->arg4    = is_number( arg7 ) ? atoi( arg7 ) : 1;
            }
            else
              /*
               * Inside the room.
               * ----------------
               */
              if ( !str_cmp( arg4, "room" ) )
              {
                if (get_obj_index(atoi(arg3)) == NULL)
                {
                  send_to_char( "That Vnum does not exist.\n\r",ch);
                  return;
                }
                if (!check_vnum_range(ch,atoi(arg3)))
                  return;
                pReset->command  = 'O';
                pReset->arg2     = 0;
                pReset->arg3     = ch->in_room->vnum;
                pReset->arg4     = 0;
              }
              else
                /*
                 * Into a Mobile's inventory.
                 * --------------------------
                 */
              {
                if ( flag_value( wear_loc_flags, arg4 ) == NO_FLAG )
                {
                  send_to_char( "Resets: '? wear-loc'\n\r", ch );
                  return;
                }
                if (get_obj_index(atoi(arg3)) == NULL)
                {
                  send_to_char( "That Vnum does not exist.\n\r",ch);
                  return;
                }
                if (!check_vnum_range(ch, atoi(arg3)))
                  return;
                pReset->arg1 = atoi(arg3);
                pReset->arg3 = flag_value( wear_loc_flags, arg4 );
                if ( pReset->arg3 == WEAR_NONE )
                  pReset->command = 'G';
                else
                  pReset->command = 'E';

              }
          }
        add_reset( ch->in_room, pReset, atoi( arg1 ) );
        SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
        send_to_char( "Reset added.\n\r", ch );
      }
      else
        if (!str_cmp( arg2, "random") && is_number(arg3))
        {
          if (atoi(arg3) < 1 || atoi(arg3) > 6)
          {
            send_to_char("Invalid argument.\n\r", ch);
            return;
          }
          if (!check_vnum_range(ch, atoi(arg3)))
            return;
          pReset = new_reset_data ();
          pReset->command = 'R';
          pReset->arg1 = ch->in_room->vnum;
          pReset->arg2 = atoi(arg3);
          add_reset( ch->in_room, pReset, atoi( arg1 ) );
          SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
          send_to_char( "Random exits reset added.\n\r", ch);
        }
        else
        {
          send_to_char( "Syntax: RESET <number> OBJ <vnum> <wear_loc>\n\r", ch );
          send_to_char( "        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]\n\r", ch );
          send_to_char( "        RESET <number> OBJ <vnum> room\n\r", ch );
          send_to_char( "        RESET <number> MOB <vnum> [max #x area] [max #x room]\n\r", ch );
          send_to_char( "        RESET <number> DELETE\n\r", ch );
          send_to_char( "        RESET <number> RANDOM [#x exits]\n\r", ch);
        }
  }
  return;
}



/*****************************************************************************
 Name:    do_alist
 Purpose:  Normal command to list areas and display area information.
 Called by:  interpreter(interp.c)
 ****************************************************************************/
void do_alist( CHAR_DATA *ch, char *argument )
{
  char buf [ MAX_STRING_LENGTH ];
  char tbuf[MSL];
  char abuf[MSL];
  char check_buf[MIL];
  bool search = FALSE;
  bool srchauthor = FALSE;
  bool slinked = TRUE;
  bool sunlinked = TRUE;
  /*  char result [ MAX_STRING_LENGTH*2 ]; */
  AREA_DATA *pArea;
  BUFFER *output;
  int count =0;


  if (IS_NPC(ch))
    return;
  if (argument[0] != '\0')
  {
    if (is_exact_name(argument,"sort"))
    {
      do_function(ch,&do_alist_sort,"");
      return;
    }

    one_argument( argument, buf );

    if (!str_prefix(buf,"author"))
    {
      srchauthor = TRUE;
      argument = one_argument( argument, buf );
      mprintf(sizeof(check_buf),check_buf,"%s",to_upper(argument));
    }
    else if (is_exact_name(argument,"linked"))
      sunlinked = FALSE;
    else if (is_exact_name(argument,"unlinked"))
      slinked = FALSE;
    else
    {
      mprintf(sizeof(check_buf),check_buf,"%s",to_upper(argument));
      search = TRUE;
    }
  }

  output = new_buf();

  /*  sprintf( result, "[%3s] [%-27s] (%-5s-%5s) (%-3s-%3s) [%-10s] %3s [%-10s]\n\r",
     "Num", "Area Name", "lvnum", "uvnum", "lr","hr",
     "Filename", "Sec", "Builders" ); */
  for ( pArea = area_first; pArea; pArea = pArea->next )
  {
    if (search)
      if (!strstr(to_upper(pArea->name), check_buf))
        continue;

    if (srchauthor
        &&  !strstr(to_upper(pArea->builders), check_buf) )
      continue;

    if (!sunlinked
        && ( pArea->low_range == -1 ) )
      continue;

    if (!slinked
        && ( pArea->low_range != -1 ) )
      continue;

    strcpy(tbuf,pArea->file_name);

    switch (pArea->align)
    {
      case AREA_ALIGN_ALL:     strcpy(abuf, "All");  break;
      case AREA_ALIGN_GOOD:    strcpy(abuf, "Good"); break;
      case AREA_ALIGN_EVIL:    strcpy(abuf, "Evil"); break;
      case AREA_ALIGN_NEUTRAL: strcpy(abuf, "Neut"); break;
      default:                 strcpy(abuf, "????"); break;
    }

    mprintf( sizeof(buf),buf, "{W[{c%3d{W][{c%-19.19s{W][{c%-5d{W-{c%5d{W][{c%-3d{W-{c%3d{W]{c%-12.12s{W[{c%d{W][{c%-4s{W][{c%-10s{W]{x\n\r",
             pArea->vnum,
             pArea->name,
             pArea->min_vnum,
             pArea->max_vnum,
             pArea->low_range,
             pArea->high_range,
             /*         pArea->file_name,*/
             tbuf,
             pArea->security,
             abuf,
             pArea->builders );
    add_buf( output, buf );
    count ++;
  }

  if (count)
  {
    page_to_char(buf_string(output), ch );
  }
  else
  {
    send_to_char("No Matching Criteria for the alist.\n\r",ch);
  }
  free_buf(output);
  return;
}

void do_alist_sort( CHAR_DATA *ch, char *argument )
{
  char buf    [ MAX_STRING_LENGTH ];
  char result [ MAX_STRING_LENGTH*2 ];  /* May need tweaking. */
  AREA_DATA *pArea;
  BUFFER *output;
  int x=0;
  if (IS_NPC(ch))
    return;

  output = new_buf();


  mprintf( sizeof(result),result, "[%3s] [%-27s] (%-5s-%5s) [%-10s] %3s [%-10s]\n\r",
           "Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec", "Builders" );
  for (x =0; x < 33000; x++)
  {
    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
      if (pArea->max_vnum == x)
      {
        mprintf(sizeof(buf), buf, "{W[{c%3d{W][{c%-19.19s{W][{c%-5d{W-{c%5d{W][{c%-3d{W-{c%3d{W]{c%-12.12s{W[{c%d{W][{c%-10s{W]{x\n\r",
                pArea->vnum,
                pArea->name,
                pArea->min_vnum,
                pArea->max_vnum,
                pArea->low_range,
                pArea->high_range,
                pArea->file_name,
                pArea->security,
                pArea->builders );
        add_buf( output, buf );
      }
    }
  }
  page_to_char(buf_string(output), ch );
  free_buf(output);
  return;
}

bool check_vnum_range(CHAR_DATA *ch, int vnum)
{
  AREA_DATA *area;

  for ( area = area_first ; area ; area = area->next )
    if ( area->vnum == 0 )
      return TRUE;

  if ( IN_RANGE( ch->in_room->area->min_vnum, vnum,
                 ch->in_room->area->max_vnum ) )
    return TRUE;

  send_to_char("That VNUM is out of range for this area\n\r",ch);
  return FALSE;
}

/*
 * OLC security settings.
 */
void do_security( CHAR_DATA *ch, char *argument )
{
  char        arg[MAX_INPUT_LENGTH];
  CHAR_DATA   *vch;
  long        flags;

  if ( IS_NPC( ch ) )
    return;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    printf_to_char( ch,
                    "{DSecurity settings for you are:{x\n\r"
                    " {DLevel:{g  %d{x\n\r"
                    " {DAvailable Commands:{g  %s{x\n\r",
                    ch->pcdata->security,
                    flag_string( olc_security_flags, ch->pcdata->security_flags ) );
    return;
  }

  if ( !str_cmp( arg, "list" ) )
  {
    char buf[MAX_STRING_LENGTH];
    int i;

    buf[0] = '\0';
    for ( i = 0; olc_security_flags[i].name != NULL; i++ )
    {
      strcat( buf, " " );
      strcat( buf, olc_security_flags[i].name );
    }

    printf_to_char( ch,
                    "{gAvailable OLC security:{x\n\r"
                    " Levels:{g  0 - 9{x\n\r"
                    " Flags:{g   %s{x\n\r", buf + 1 );
    return;
  }

  if ( !OLC_SECURITY_FLAGS( ch, OLC_SEC_SECURITY )
       &&   get_trust( ch ) < IMPLEMENTOR )
  {
    send_to_char(
      "You have to be an implementor to see or adjust others security.\n\r",
      ch );
    return;
  }

  if ( ( vch = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "No such character by that name.\n\r", ch );
    return;
  }

  if ( IS_NPC( vch ) )
  {
    send_to_char( "NPCs don't have OLC security.\n\r", ch );
    return;
  }

  /* Can't view or modify security for those trusted at same or higher level,
   * unless character is trusted to IMPLEMENTOR.
   */
  if ( get_trust( ch ) < IMPLEMENTOR
       &&   get_trust( ch ) < get_trust( vch ) )
  {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  {
    printf_to_char( ch,
                    "Security settings for{g %s:{x\n\r"
                    " Level:{g  %d{x\n\r"
                    " Flags:{g  %s{x\n\r",
                    vch->name,
                    vch->pcdata->security,
                    flag_string( olc_security_flags, vch->pcdata->security_flags ) );
    return;
  }

  if ( is_number( argument ) )
  {
    sh_int value = atoi( argument );

    if ( value < 0 || value > 9 )
    {
      send_to_char( "Valid security levels are 0 through 9.\n\r", ch );
      return;
    }

    vch->pcdata->security = value;
    send_to_char( "Level of security set.\n\r", ch );
    loggedf( "%s: security set for %s (%d)",
             ch->name, vch->name, vch->pcdata->security );
    return;
  }

  if ( !str_cmp( argument, "none" ) )
  {
    vch->pcdata->security_flags = 0;
    send_to_char( "Security flags cleared.\n\r", ch );
    loggedf( "%s: security flags cleared for %s", ch->name, vch->name );
    return;
  }

  if ( ( flags = flag_new_value( ch, olc_security_flags, argument,
                                 vch->pcdata->security_flags ) ) != NO_FLAG )
  {
    vch->pcdata->security_flags = flags;
    send_to_char( "Security flags set.\n\r", ch );
    loggedf( "%s: security flags set for %s [%s]",
             ch->name, vch->name,
             flag_string( olc_security_flags, vch->pcdata->security_flags ) );
    return;
  }
}
