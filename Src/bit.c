/***************************************************************************
 *  File: bit.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was written by Jason Dinkel and inspired by Russ Taylor,     *
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

/*
 The code below uses a table lookup system that is based on suggestions
 from Russ Taylor.  There are many routines in handler.c that would benefit
 with the use of tables.  You may consider simplifying your code base by
 implementing a system like below with such functions. -Jason Dinkel
 */



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
#include "lookup.h"


/*****************************************************************************
 Name:		flag_stat_table
 Purpose:	This table catagorizes the tables following the lookup
 		functions below into stats and flags.  Flags can be toggled
 		but stats can only be assigned.  Update this table when a
 		new set of flags is installed.
 ****************************************************************************/
const struct flag_stat_type flag_stat_table[] =
{
/*  {	structure		stat	}, */
    {	area_flags, 		TRUE	},
    {   sex_flags,	    	TRUE	},
    {   exit_flags,		    FALSE	},
    {   door_resets,		TRUE	},
    {   room_flags, 		FALSE	},
    {   sector_flags,		TRUE	},
    {   type_flags,	    	TRUE	},
    {   extra_flags,		FALSE	},
    {   wear_flags,		    FALSE	},
    {   act_flags,  		FALSE	},
    {   act2_flags,         FALSE   },
    {   affect_flags,		FALSE	},
    {   spell_affect_flags,	FALSE	},
    {   affect2_flags,      FALSE   },
    {   apply_flags,		TRUE	},
    {   wear_loc_flags,		TRUE	},
    {   wear_loc_strings,	TRUE	},
    {   container_flags,	FALSE	},
    {   spool_flags,        TRUE	},
    {   mprog_flags,	    FALSE	},
    {   plr2_flags,	        FALSE	},
    {   pen_flags,	        FALSE	},
    {   chan_flags,	        FALSE	},
    {   comm_flags,	        FALSE	},

/* ROM specific flags: */

    {   form_flags,             FALSE   },
    {   part_flags,             FALSE   },
    {   ac_type,                TRUE    },
    {   size_flags,             TRUE    },
    {   position_flags,         TRUE    },
    {   off_flags,              FALSE   },
    {   imm_flags,              FALSE   },
    {   res_flags,              FALSE   },
    {   vuln_flags,             FALSE   },
    {   weapon_class,           TRUE    },
    {   apply_types,		TRUE,   FALSE 	},
    {   0,			0,	0	}
};
    
// Function template
bool is_prefix( const struct flag_type *flag_table );

/*****************************************************************************
 Name:		is_stat( table )
 Purpose:	Returns TRUE if the table is a stat table and FALSE if flag.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
bool is_stat( const struct flag_type *flag_table )
{
    int flag;

    for ( flag = 0; flag_stat_table[flag].structure; flag++ )
    {
	  if ( flag_stat_table[flag].structure == flag_table
	  &&   flag_stat_table[flag].stat )
	    return TRUE;
    }
    return FALSE;
}


/*
 * Return the value of a single, settable flag from the table.
 * Local function only.
 *
 * This function is Russ Taylor's creation.  Thanks Russ!
 * All code copyright (C) Russ Taylor, permission to use and/or distribute
 * has NOT been granted.  Use only in this OLC package has been granted.
 */
int64 flag_lookup2( const char *name, const struct flag_type *flag_table, bool fPrefix )
{
    int64 flag = 0;

  if ( fPrefix )
	for ( ; flag_table[flag].name; flag++ )
    {
	    if ( LOWER( name[0] ) == LOWER( flag_table[flag].name[0] )
	    &&  !str_prefix( name, flag_table[flag].name )
	    &&   flag_table[flag].settable )
		  return flag_table[flag].bit;
    }
  else
    for ( ; flag_table[flag].name; flag++ )
    {
	    if ( LOWER( name[0] ) == LOWER( flag_table[flag].name[0] )
	    &&  !str_prefix( name, flag_table[flag].name )
	    &&   flag_table[flag].settable )
		  return flag_table[flag].bit;
    }
  return NO_FLAG;
}

/*****************************************************************************
 Name:		flag_value( table, flag )
 Purpose:	Returns the value of the flags entered.  Multi-flags accepted.
 Called by:	olc.c and olc_act.c.
 ****************************************************************************/
int64 flag_value( const struct flag_type *flag_table, char *argument)
{
    char word[MAX_INPUT_LENGTH];
    int64  bit;
    int64  marked = 0;
    bool found = FALSE;

    if ( is_stat( flag_table ) )
    {
	  one_argument( argument, word );
	  return flag_lookup( word, flag_table );
    }

    /*
     * Accept multiple flags.
     */
    for (; ;)
    {
        argument = one_argument( argument, word );

        if ( word[0] == '\0' )
	      break;

        if ( ( bit = flag_lookup( word, flag_table ) ) != NO_FLAG )
        {
            SET_BIT( marked, bit );
            found = TRUE;
        }
    }

    if ( found )
	return marked;
    else
	return NO_FLAG;
}



/*****************************************************************************
 Name:		flag_string( table, flags/stat )
 Purpose:	Returns string with name(s) of the flags or stat entered.
 Called by:	act_olc.c, olc.c, and olc_save.c.
 ****************************************************************************/
char *flag_string( const struct flag_type *flag_table, long bits )
{
  static char buf[10][MIL];
  int  flag;
  static int toggle = 0;

  toggle = ( 1 + toggle ) % 10;

  buf[toggle][0] = '\0';

  for ( flag = 0; flag_table[flag].name; flag++)
    {
      if ( !is_stat( flag_table ) && IS_SET(bits, flag_table[flag].bit) )
	{
	  strcat( buf[toggle], " " );
	  strcat( buf[toggle], flag_table[flag].name );
	}
      else
      {
	if ( flag_table[flag].bit == bits )
	  {
	    strcat( buf[toggle], " " );
	    strcat( buf[toggle], flag_table[flag].name );
	    break;
	  }
      }
    }

  return ( buf[toggle][0] ) ? buf[toggle] + 1 : "none";
}


/*
 * Return the value of the flags entered.  Multi-flags accepted, with the
 * addition of '+' and '-' to set or clear specifically... otherwise the
 * value returned toggles the bits.
 */
int64 flag_new_value( CHAR_DATA *ch, const struct flag_type *flag_table,
                    char *argument, int bits )
{
    char	word[MAX_INPUT_LENGTH];
    char	errstr[MAX_STRING_LENGTH];
    char	type;
    char	*pos;
    sh_int	err = 0;
    long	new_flag;
    int64	bit;
    bool	fPrefix = is_prefix( flag_table );

    if ( is_stat( flag_table ) )
    {
	  one_argument( argument, word );
	  return flag_lookup2( word, flag_table, fPrefix );
    }

    type	= argument[0];
    pos		= ( type == '=' || type == '-' || type == '+' )
		    ? argument + 1 : argument;
    errstr[0]	= '\0';

    if ( type != '=' )
	  new_flag = bits;
    else
	  new_flag = 0L;

    /* Accept multiple flags. */
    for ( ; ; )
    {
	  pos = one_argument( pos, word );
	  if ( word[0] == '\0' )
	    break;

	  if ( ( bit = flag_lookup2( word, flag_table, fPrefix ) ) == NO_FLAG )
	  {
	    err++;
	    strcat( errstr, " " );
	    strcat( errstr, word );
	    continue;
	  }

	  switch ( type )
	  {
	    case '+':
	    case '=':
	        SET_BIT( new_flag, bit );
	        break;
	    case '-':
	        REMOVE_BIT( new_flag, bit );
	        break;
	    default:
	        if ( IS_SET( new_flag, bit ) )
		        REMOVE_BIT( new_flag, bit );
	        else
		        SET_BIT( new_flag, bit );
	        break;
	  }
    }

    if ( err == 0 )
	  return new_flag;

    printf_to_char( ch, "Invalid flag%s: [%s]\n\r",
	err == 1 ? "" : "s", errstr + 1 );
    return NO_FLAG;
}

/*
 * Return TRUE if the table uses prefix matching.  FALSE if not.
 * Local funtion here only.
 */

bool is_prefix( const struct flag_type *flag_table )
{
	int flag;

	for ( flag = 0;flag_stat_table[flag].structure; flag++ )
	{
		if ( flag_stat_table[flag].structure == flag_table
		&&   flag_stat_table[flag].prefix )
			return TRUE;
	}
	return FALSE;
}
