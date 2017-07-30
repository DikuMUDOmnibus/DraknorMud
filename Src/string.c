/***************************************************************************
 *  File: string.c                                                         *
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
#include "interp.h"

  
char *string_linedel( char *, int );
char *string_lineadd( char *, char *, int );
char *string_getline( char *str, char *line );
char *numlineas( char * );
int   cstrlen( char *str );
  
char *reply_format_string_lines( char *oldstring, int line1, int line2 ,
				 bool reply_formatting);
char *format_string_lines	args ( ( char *oldstring, int line1,
					 int line2 ) );
int string_linecount( const char *str );
//bool check_punct( char *str );

/********************************
 Name:      numstr
 Purpose:   helproutine for numcat and numcpy

 Makes the actual conversion from num to str
 ********************************/
char *numstr( int num )
{
  static char buf[6]; // I don't bother optimizing, 6 is enogh, but 
                      // probably too much :P

  if ( num < 0 ) return "";
  if ( num > 9999 ) return "a treasure trove of";
  if ( num > 12 )
  {
    mprintf( sizeof(buf), buf, "%d", num );
    return buf;
  }
  switch ( num )
  {
    case  0: return "no";
    case  1: return "one";
    case  2: return "two";
    case  3: return "three";
    case  4: return "four";
    case  5: return "five";
    case  6: return "six";
    case  7: return "seven";
    case  8: return "eight";
    case  9: return "nine";
    case 10: return "ten";
    case 11: return "eleven";
    case 12: return "twelve";
  }
  return "";
}

/********************************
 Name:      numcat
 Purpose:   Complement to strcat, but for numbers

 Takes num and convert it to a string:
 0 -> "no", 1 - 12 -> "one" - "twelve",  13 -999 -> "13" - "999"
 Above 999 -> "many". Negative numbers are not treated.
 ********************************/
char *numcat( char *buf, int num )
{
  strcat( buf, numstr( num ) );
  return buf;
}

/********************************
 Name:      numcpy
 Purpose:   Complement to strcpy, but for numbers

 Takes num and convert it to a string:
 0 -> "no", 1 - 12 -> "one" - "twelve",  13 -999 -> "13" - "999"
 Above 999 -> "many". Negative numbers are not treated.
 ********************************/
char *numcpy( char *buf, int num )
{
  strcpy( buf, numstr( num ) );
  return buf;
}

/*****************************
 Name:      spacecat
 Purpose:   To seamlessly add a space into a string
 *****************************/
char *spacecat( char *buf )
{
    strcat( buf, " " );
    return buf;
}

/*****************************
 Name:      del_last_line
 Purpose:   Removes last line from string
 Called by: many.
 ******************/
char * del_last_line( char * string)
{
  int len;
  bool found = FALSE;

  char xbuf[MAX_STRING_LENGTH];

  xbuf[0] = '\0';
  if (string == NULL || string[0] == '\0')
    return(str_dup(xbuf, ""));
	
  strcpy(xbuf, string);

  for (len = strlen(xbuf); len > 0; len--)
    {
      if (xbuf[len] == '\r')
	{
	  if (!found) /* back it up */
	    {
	      if ( len > 0)
		len--;
	      found = TRUE;
	    }
	  else /* found second one */
	    {
	      xbuf[len +1] = '\0';
	      free_string(string);
	      return( str_dup(xbuf,""));
	    }
	}
    }
  xbuf[0] = '\0';
  free_string(string);
  return( str_dup(xbuf,""));
}

/*****************************************************************************
 Name:		string_edit
 Purpose:	Clears string and puts player into editing mode.
 Called by:	none
 ****************************************************************************/
void string_edit( CHAR_DATA *ch, char **pString )
{
  send_to_char( "{C-========- Entering EDIT Mode -=========-\n\r", ch );
  send_to_char( "    Type .{Yh{C or ,{Yh{C on a new line for help\n\r", ch );
  send_to_char( " Terminate with a {Y~{C or {Y@{C on a blank line.\n\r", ch );
  send_to_char( "-=======================================-{x\n\r", ch );

  if ( *pString == NULL )
    {
      *pString = str_dup( "", *pString );
    }
  else
    {
      **pString = '\0';
    }

  ch->desc->pString = pString;

  return;
}



/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void string_append( CHAR_DATA *ch, char **pString, sh_int saving, void *vo )
{
  send_to_char( "{C-=======- Entering APPEND Mode -========-\n\r", ch );
  send_to_char( "    Type .{Yh{C or ,{Yh{C on a new line for help\n\r", ch );
  send_to_char( " Terminate with a {Y~{C or {Y@{C on a blank line.\n\r", ch );
  send_to_char( "-=======================================-{x\n\r", ch );

  if ( *pString == NULL )
  {
    *pString = str_dup( "",*pString );
  }

  send_to_char( numlineas(*pString), ch );

  ch->desc->pString = pString;

  return;
}



/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char * string_replace( char * orig, char * old, char * new_str )
{
  char xbuf[MAX_STRING_LENGTH];
  int i;

  xbuf[0] = '\0';
  strcpy( xbuf, orig );
  if ( strstr( orig, old ) != NULL )
  {
    i = strlen( orig ) - strlen( strstr( orig, old ) );
    xbuf[i] = '\0';
    strcat( xbuf, new_str );
    strcat( xbuf, &orig[i+strlen( old )] );
    free_string( orig );
  }

  return str_dup( xbuf,"" );
}



/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void string_add( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  int  line;

  /*
   * Thanks to James Seng
   */
  smash_tilde( argument );

  if ( *argument == '.' || *argument == ',' )
  {
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char tmparg3 [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg1 );
    argument = first_arg( argument, arg2, FALSE );
    strcpy( tmparg3, argument );
    argument = first_arg( argument, arg3, FALSE );

    if ( !str_cmp( arg1, ".c" ) || !str_cmp( arg1, ",c") )
    {
	    send_to_char( "String cleared.\n\r", ch );
	    free_string(*ch->desc->pString);
  	  *ch->desc->pString = str_dup( "", *ch->desc->pString );
	    return;
    }

    if ( !str_cmp( arg1, ".s" ) || !str_cmp(arg1, ",s") )
    {
	    send_to_char( "String so far:\n\r", ch );
	    send_to_char( numlineas(*ch->desc->pString), ch );
	    return;
    }

    if ( !str_cmp( arg1, ".r" ) || !str_cmp( arg1, ",r"))
    {
	    if ( arg2[0] == '\0' )
      {
	      send_to_char(
			     "usage:  .r or ,r \"old string\" \"new string\"\n\r", ch );
	      return;
      }

	    *ch->desc->pString =
	      string_replace( *ch->desc->pString, arg2, arg3 );
	    printf_to_char(ch, "'%s' replaced with '%s'.\n\r", arg2, arg3 );
	    return;
    }

    if ( !str_cmp( arg1, ".d" ) || !str_cmp( arg1, ",d"))
  	{
	    *ch->desc->pString = del_last_line( *ch->desc->pString );
	    send_to_char( " Line removed.\n\r", ch );
	    return;
  	}

  	if ( !str_cmp( arg1, ".f" ) || !str_cmp( arg1, ",f" ) )
	  {
	    if ( arg2[0] != '\0' )
	    {
		    int line1, line2;

		    if ( arg3[0] == '\0' )
		    {
		      send_to_char(
			      "Must specify a begin and end line number.\n\r", ch );
		      return;
		    }

		    if ( !is_number( arg2 ) || !is_number( arg3 ) )
		    {
		      send_to_char(
		        "Must specify numbers for the begin and end line number.\n\r", ch );
		    }

		    line1 = atoi( arg2 );
		    line1 = URANGE( 1, line1, string_linecount( *ch->desc->pString ) + 1 );

		    line2 = atoi( arg3 );
		    line2 = URANGE( 1, line2, string_linecount( *ch->desc->pString ) + 1 );

		    if ( line1 > line2 )
		    {
		      line  = line2;
		      line2 = line1;
		      line1 = line;
    		}

		    if (IS_SET(ch->active_flags, ACTIVE_REPLY))
		    {
		      *ch->desc->pString = reply_format_string_lines(
					  *ch->desc->pString, line1, line2, TRUE );
		    }
		    else
		    {
		      *ch->desc->pString = format_string_lines(
					   *ch->desc->pString, line1, line2 );
		    }

    		printf_to_char( ch, "String formatted from line %d to %d.\n\r",
		      line1, line2 );

		    return;
	    }

	    if (IS_SET(ch->active_flags, ACTIVE_REPLY))
      {
    		*ch->desc->pString = reply_format_string_lines(*ch->desc->pString, 0, MSL, TRUE );
	    }
	    else
	    {
		    *ch->desc->pString = format_string( *ch->desc->pString );
	    }

	    send_to_char( "String formatted.\n\r", ch );
	    return;
	  }

  	if ( !str_cmp( arg1, ".rf" ) || !str_cmp( arg1, ",rf" ) )
	  {
	    if ( arg2[0] != '\0' )
	    {
		    int line1, line2;

		    if ( arg3[0] == '\0' )
		    {
		      send_to_char(
  			    "Must specify a begin and end line number.\n\r", ch );
		      return;
	  	  }

		    if ( !is_number( arg2 ) || !is_number( arg3 ) )
		    {
		      send_to_char(
            "Must specify numbers for the begin and end line number.\n\r", ch );
		    }

		    line1 = atoi( arg2 );
		    line1 = URANGE( 1, line1, string_linecount( *ch->desc->pString ) + 1 );

  		  line2 = atoi( arg3 );
    		line2 = URANGE( 1, line2, string_linecount( *ch->desc->pString ) + 1 );

	    	if ( line1 > line2 )
		    {
		      line  = line2;
  		    line2 = line1;
	  	    line1 = line;
		    }

		    *ch->desc->pString = reply_format_string_lines(
							 *ch->desc->pString, line1, line2, TRUE );
		    printf_to_char( ch, "String formatted from line %d to %d.\n\r",
		      line1, line2 );
  		  return;
	    }

      *ch->desc->pString = reply_format_string_lines(
		     *ch->desc->pString, 0, MSL, TRUE );
      send_to_char( "String formatted.\n\r", ch );
      return;
	  }
 
  	if (!str_cmp(arg1, ".i") || !str_cmp(arg1, ",i"))
	  {
	    if (!IS_NULLSTR(arg2))
	      do_function(ch, &do_ispell, arg2);
  	  else
	      ispell_string(ch) ;
	    return;
  	}
        
    if ( !str_cmp( arg1, ".ld" ) || !str_cmp( arg1, ",ld"))
  	{
	    *ch->desc->pString = string_linedel( *ch->desc->pString, atoi(arg2) );
	    send_to_char( "Line deleted.\n\r", ch );
  	  return;
	  }

    if ( !str_cmp( arg1, ".li" ) || !str_cmp( arg1, ",li") )
	  {
	    *ch->desc->pString = string_lineadd(
	      *ch->desc->pString,/*  arg3 */tmparg3 , atoi(arg2) );
  	  send_to_char( "Line inserted.\n\r", ch );
	    return;
	  }

    if ( !str_cmp( arg1, ".lr" ) || !str_cmp( arg1, ",lr"))
	  {
	    *ch->desc->pString = string_linedel( *ch->desc->pString, atoi(arg2) );
  	  *ch->desc->pString = string_lineadd(
	      *ch->desc->pString,/* arg3 */tmparg3 , atoi(arg2) );
	    send_to_char( "Line replaced.\n\r", ch );
	    return;
	  }

    if ( !str_cmp( arg1, ".h" ) || !str_cmp( arg1, ",h") )
    {
      send_to_char( "Sedit help (commands on blank line):   \n\r", ch );
	    send_to_char( ".r/,r 'old' 'new'    - replace a substring \n\r", ch );
  	  send_to_char( "                     (requires '', \"\") \n\r", ch );
	    send_to_char( ".h/,h                - get help (this info)\n\r", ch );
	    send_to_char( ".s/,s                - show string so far  \n\r", ch );
  	  send_to_char( ".f/,f                - (word wrap) string  \n\r", ch );
	    send_to_char( ".f/,f <num1> <num2>  - Format from line num1 to line num2\n\r", ch );
	    send_to_char( ".rf/,rf <num1> <num2>- REPLY Format from line num1 to line num2\n\r", ch );
	    send_to_char( ".d/,d                - delete last line    \n\r", ch );
  	  send_to_char( ".c/,c                - clear string so far \n\r", ch );
	    send_to_char( ".ld/,ld <num>        - Delete line number <num>\n\r", ch );
  	  send_to_char( ".li/,li <num> <str>  - insert <str> in line <num>\n\r", ch );
	    send_to_char( ".lr/,lr <num> <str>  - replace line <num> with <str>\n\r", ch );
	    send_to_char( ".i/,i < word>        - do a check on the single word given or\n\r",ch);
  	  send_to_char( ".i/,i                - spellcheck the whole string.\n\r",ch);
	    send_to_char( "@                    - end string          \n\r", ch );
	    return;
    }

    send_to_char( "SEdit:  Invalid dot command.\n\r", ch );
    return;
  }

  if ( *argument == '~' || *argument == '@' )
  {
    if ( ch->desc->editor == ED_MPCODE ) /* para los mobprogs */
	  {
	    MOB_INDEX_DATA *mob;
	    int hash;
	    MPROG_LIST *mpl;
	    MPROG_CODE *mpc;

	    EDIT_MPCODE(ch, mpc);

	    if ( mpc )
	      for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
	        for ( mob = mob_index_hash[hash]; mob; mob = mob->next )
		        for ( mpl = mob->mprogs; mpl; mpl = mpl->next )
		          if ( mpl->vnum == mpc->vnum )
		          {
		            printf_to_char(ch, "Fixing mob %d.\n\r", mob->vnum );
		            mpl->code = mpc;
		          }
	  }

    ch->desc->pString = NULL;
    REMOVE_BIT(ch->active_flags, ACTIVE_REPLY);
    return;
  }

  strcpy( buf, *ch->desc->pString );

  /*
   * Truncate strings to MAX_STRING_LENGTH.
   * --------------------------------------
   */
  if ( strlen( buf ) + strlen( argument ) >= ( MAX_STRING_LENGTH - 4 ) )
  {
    send_to_char( "String too long, last line skipped.\n\r", ch );

    /* Force character out of editing mode. */
    ch->desc->pString = NULL;
    return;
  }


  if ((strlen(*ch->desc->pString) + strlen( argument )) > MAX_STRING)
  {
    send_to_char("Your input exceeds The Maximum Size.\n\r",ch);
    ch->desc->pString = NULL;
    return;
  }

  /*
   * Ensure no tilde's inside string.
   * --------------------------------
   */
  smash_tilde( argument );

  strcat( buf, argument );
  strcat( buf, "\n\r" );
  free_string( *ch->desc->pString );
  *ch->desc->pString = str_dup( buf, *ch->desc->pString );
  return;
}



/*
 * Special string formating and word-wrapping.
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 *
 * Color wise, this function presumes: 1) that the string does not end with a
 * color tag character, 2) that there are no invaild color tags within the
 * string.  The simplest way to achieve this is to use fix_str_color before
 * any call to format_string.
 */
char *format_string( char *oldstring )
{
  char xbuf[MAX_STRING_LENGTH];
  char xbuf2[MAX_STRING_LENGTH];
  char *rdesc;
  int  i = 0;
  int  c;
  bool cap = TRUE;

  if ( IS_NULLSTR( oldstring ) )
    return &str_empty[0];

  xbuf[0] = xbuf2[0] = 0;

  for ( rdesc = oldstring; *rdesc; rdesc++ )
  {
    if ( *rdesc == '{' )
  	{
	    xbuf[i++] = *rdesc++;
	    xbuf[i++] = *rdesc;
  	}
    else if ( *rdesc == '\n' )
	  {
	    if ( i > 0 && xbuf[i-1] != ' ' )
	      xbuf[i++] = ' ';
  	}
    else if ( *rdesc == '\r' )
	    ;
    else if ( *rdesc == ' ' )
	  {
	    if ( i > 0 && xbuf[i-1] != ' ' )
	      xbuf[i++] = ' ';
	  }
    else if ( *rdesc == ')' )
	  {
	    if ( i > 2 && xbuf[i-1] == ' ' && xbuf[i-2] == ' '
	    && ( xbuf[i-3] == '.' || xbuf[i-3] == '?' || xbuf[i-3] == '!' ) )
	    {
	      xbuf[i-2] = *rdesc;
	      xbuf[i-1] = ' ';
	      xbuf[i++] = ' ';
	    }
	    else
	    {
	      xbuf[i++] = *rdesc;
	    }
	  }
    else if ( *rdesc == '.' || *rdesc == '?' || *rdesc == '!' )
	  {
	    if ( i > 0 && isdigit( xbuf[i-1] ) && isdigit( *(rdesc + 1) ) )
	    {
	      xbuf[i++] = *rdesc;
	    }
	    else if ( i > 2 && xbuf[i-1] == ' ' && xbuf[i-2] == ' '
		  && (      xbuf[i-3] == '.' || xbuf[i-3] == '?' || xbuf[i-3] == '!'
			||        xbuf[i-3] == ':' ) )
	    {
	      xbuf[i-2] = *rdesc;

	      if ( *(rdesc+1) != '"' )
    		{
		      xbuf[i-1] = ' ';
		      xbuf[i++] = ' ';
		    }
	      else
		    {
		      xbuf[i-1] = '"';
		      xbuf[i++] = ' ';
		      xbuf[i++] = ' ';
		      rdesc++;
		    }
	      cap = TRUE;
	    }
	    else
	    {
	      xbuf[i++] = *rdesc;

	      if ( *(rdesc+1) != '"' )
    		{
		      xbuf[i++]=' ';
    		  xbuf[i++]=' ';
		    }
	      else
    		{
  		    xbuf[i++] = '"';
	  	    xbuf[i++] = ' ';
		      xbuf[i++] = ' ';
    		  rdesc++;
		    }

	      cap = TRUE;
	    }
  	}
    else
	  {
	    xbuf[i] = *rdesc;

  	  if ( cap )
	    {
	      cap = FALSE;
	      xbuf[i] = UPPER( xbuf[i] );
	    }
	    i++;
	  }
  }

  xbuf[i] = '\0';
  strcpy( xbuf2, xbuf );
  rdesc = xbuf2;
  xbuf[0] = 0;

  for ( ; ; )
  {
    for ( c = i = 0; i < 77; )
	  {
	    if ( *(rdesc+i+c) == '\0' )
	      break;

  	  if ( *(rdesc+i+c) == '{' )
	    {
	      c++;

	      switch ( *(rdesc+i+c) )
	    	{
      		default:  c++; break;
      		case 'v':
      		case '-':
      		case 'x': i++; break;
		    }
	    }
	    else
	      i++;
	  }

    if ( i < 77 )
    	break;

    for ( c = i = 0; i < (xbuf[0]?77:76); )
  	{
	    if ( *(rdesc+i+c) == '{' )
	    {
	      c++;
	      switch ( *(rdesc+i+c) )
		    {
		      default:  c++; break;
      		case 'v':
      		case '-':
      		case 'x': i++; break;
	  	  }
	    }
	    else
	      i++;
	  }

    for ( ; i > 0; i-- )
	  {
	    if ( *(rdesc+i+c) == ' ' )
	      break;
	  }

    if ( i > 0 )
	  {
	    *(rdesc+i+c) = '\0';
  	  strcat( xbuf, rdesc );
	    strcat( xbuf, "\n\r" );
	    rdesc += i+c+1;

  	  while ( *rdesc == ' ' )
	      rdesc++;
	  }
    else
	  {
  	  bug( "Format_string: No spaces in line. Breaking it.", 0 );
	    *(rdesc + 75) = '\0';
	    strcat( xbuf, rdesc );
  	  strcat( xbuf, "-\n\r" );
	    rdesc += 76;
  	}
  }

  i += c;

  while ( i > 0 && ( *(rdesc+i) == ' ' ||
		     *(rdesc+i) == '\n' ||
		     *(rdesc+i) == '\r' ) )
  {
    *(rdesc+i--) = '\0';
  }

  while ( i > 1 && *(rdesc+i-2) == '{' )
    i -= 2;

  c = i-1;
  while ( c > 0 && ( *(rdesc+c) == ' '  ||
		     *(rdesc+c) == '\n' ||
		     *(rdesc+c) == '\r' ) )
  {
    *(rdesc+c--) = '\0';
  }

  if ( *rdesc != '\0' )
    strcat( xbuf, rdesc );

  if ( c != i-1 )
    strcat( xbuf, rdesc+i );

  if ( ( i = strlen(xbuf) ) > 1 && xbuf[i-2] != '\n' )
    strcat( xbuf, "\n\r" );

  free_string( oldstring );
  return( str_dup( xbuf ,"") );
}



/*
 * Used above in string_add.  Because this function does not
 * modify case if fCase is FALSE and because it understands
 * parenthesis, it would probably make a nice replacement
 * for one_argument.
 */
/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quates, parenthesis (barring ) ('s) and
 		percentages.
 Called by:	string_add(string.c)
 ****************************************************************************/
char *first_arg( char *argument, char *arg_first, bool fCase )
{
  char cEnd;

  while ( *argument == ' ' )
    argument++;

  cEnd = ' ';
  if ( *argument == '\'' || *argument == '"'
  || *argument == '%'  || *argument == '(' )
  {
    if ( *argument == '(' )
    {
	    cEnd = ')';
	    argument++;
    }
    else
      cEnd = *argument++;
  }

  while ( *argument != '\0' )
  {
    if ( *argument == cEnd )
	  {
	    argument++;
	    break;
	  }

    if ( fCase )
      *arg_first = LOWER(*argument);
    else *arg_first = *argument;
      arg_first++;

    argument++;
  }
  *arg_first = '\0';

  while ( *argument == ' ' )
    argument++;

  return argument;
}




/*
 * Used in olc_act.c for aedit_builders.
 */
char * string_unpad( char * argument )
{
  char buf[MAX_STRING_LENGTH];
  char *s;

  s = argument;

  while ( *s == ' ' )
    s++;

  strcpy( buf, s );
  s = buf;

  if ( *s != '\0' )
    {
      while ( *s != '\0' )
	s++;
      s--;

      while( *s == ' ' )
	s--;
      s++;
      *s = '\0';
    }

  free_string( argument );
  return str_dup( buf,"" );
}



/*
 * Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder.
 */
char * string_proper( char * argument )
{
  char *s;

  s = argument;

  while ( *s != '\0' )
    {
      if ( *s != ' ' )
        {
	  *s = UPPER(*s);
	  while ( *s != ' ' && *s != '\0' )
	    s++;
        }
      else
        {
	  s++;
        }
    }

  return argument;
}

char *string_linedel( char *string, int line )
{
  char *strtmp = string;
  char buf[MAX_STRING_LENGTH];
  int cnt = 1, tmp = 0;

  buf[0] = '\0';

  for ( ; *strtmp != '\0'; strtmp++ )
    {
      if ( cnt != line )
	buf[tmp++] = *strtmp;

      if ( *strtmp == '\n' )
	{
	  if ( *(strtmp + 1) == '\r' )
	    {
	      if ( cnt != line )
		buf[tmp++] = *(++strtmp);
	      else
		++strtmp;
	    }

	  cnt++;
	}
    }

  buf[tmp] = '\0';

  free_string(string);
  return str_dup(buf,"");
}

char *string_lineadd( char *string, char *newstr, int line )
{
  char *strtmp = string;
  int cnt = 1, tmp = 0;
  bool done = FALSE;
  char buf[MAX_STRING_LENGTH];

  buf[0] = '\0';

  for ( ; *strtmp != '\0' || (!done && cnt == line); strtmp++ )
    {
      if ( cnt == line && !done )
	{
	  strcat( buf, newstr );
	  strcat( buf, "\n\r" );
	  tmp += strlen(newstr) + 2;
	  cnt++;
	  done = TRUE;
	}

      buf[tmp++] = *strtmp;

      if ( done && *strtmp == '\0' )
	break;

      if ( *strtmp == '\n' )
	{
	  if ( *(strtmp + 1) == '\r' )
	    buf[tmp++] = *(++strtmp);

	  cnt++;
	}

      buf[tmp] = '\0';
    }

  free_string(string);
  return str_dup(buf,"");
}

/* buf queda con la linea sin \n\r */
char *sgetline( char *str, char *buf )
{
  int tmp = 0;
  bool found = FALSE;

  while ( *str )
    {
      if ( *str == '\n' )
	{
	  found = TRUE;
	  break;
	}

      buf[tmp++] = *(str++);
    }

  if ( found )
    {
      if ( *(str + 1) == '\r' )
	str += 2;
      else
	str += 1;
    } /* para que quedemos en el inicio de la prox linea */

  buf[tmp] = '\0';

  return str;
}

char *numlineas( char *string )
{
  int cnt = 1;
  static char buf[MAX_STRING_LENGTH*2];
  char buf2[MAX_STRING_LENGTH], tmpb[MAX_STRING_LENGTH];

  buf[0] = '\0';

  while ( *string )
    {
      string = sgetline( string, tmpb );
      mprintf(sizeof(buf2), buf2, "%2d. %s\n\r", cnt++, tmpb );
      strcat( buf, buf2 );
    }

  return buf;
}


int colorstrlen(char *argument)
{
  char *str;
  int strlength;

  if (argument == NULL || argument[0] == '\0')
    return 0;

  strlength = 0;
  str = argument;

  while (*str != '\0')
    {
      if ( *str != '{' )
	{			
	  str++;
	  strlength++;
	  continue;
	}

      if (*(++str) == '{')
	strlength++;

      str++;
    }
  return strlength;
}

char *percent_bar( int current, int max, int length, char *fill, char *blank )
{
  static char buf[100];
  int blocks = (100*current/max)*length/100;
  strcpy(buf,"[");
  strcat( buf, make_bar( fill, blocks ));
  strcat( buf, make_bar( blank, length-blocks-2 ));
  strcat( buf,"]");
  return buf;    
}

char *color_percent_bar( int current, int max, int length, char *fill, char *blank )
{
  static char buf[100];
  int blocks = (100*current/max)*length/100;
  if (current <=0)
    blocks = 2;
  if (blocks <= 2)
    blocks = 2;
  
  strcpy(buf,"{W[");
  /* get percent, 67 and above = green :) */
  if (current > (0.66 * max))
    strcat(buf,"{G");
  else if (current > (0.33 * max))
    strcat(buf,"{Y");
  else
    strcat(buf,"{R");
  strcat( buf, make_bar( fill, blocks-2 ));
  strcat( buf, make_bar( blank, length-blocks ));
  strcat( buf,"{W]{x");
  return buf;    
}

char *make_bar( char * c, int length )
{
  static char buf[MAX_STRING_LENGTH];
  char temp[MAX_STRING_LENGTH];
  int i;

  strcpy( buf, c );
  temp[0] = '\0';

  if ( length <= 0 )
    {
      strcpy( buf, "\0" );
      return buf;
    }

  for ( i=0; i< length; i++ )
    {
      strcat( temp, c );
      strcpy( buf, temp );
    }

  return buf;
}

char *center_align( char * string, int width )
{
  static char buf[MAX_STRING_LENGTH];
  char temp[MAX_STRING_LENGTH];
  int left,right, diff;

  temp[0] = '\0';

  diff = width - cstrlen(string);

  if ( diff % 2 == 0 )
    left = right = diff/2;
  else
    {
      left = (diff+1)/2;
      right = left-1;
    }        
    
  strcat( temp, make_bar( " ", left ) );
  strcat( temp, string );
  strcat( temp, make_bar( " ", right ) );

  strcpy( buf, temp );
  return buf;
}

int cstrlen( char *str )
{
  int length=0;
  for ( ; *str != '\0'; str++ )
    {
      if ( *str == '{' )
        {
	  str++;
	  if ( *str == '{' ) /* Add to this line with || for  tildes,etc. */
	    length++;
	  continue;
        }
      length++;
    }
  return length;
}

char *format_ip( char *str )
{
  static char buf[MAX_STRING_LENGTH];
  char tempstr[5];
  int i,j=0,k=0,ip[4];

  for ( i=0; i<3; i++ )
    {
      for ( ; str[j] != '.'; j++ )
        {
	  tempstr[k++] = str[j];
        }
      ++j;
      tempstr[k] = '\0';
      ip[i] = atoi( tempstr );
      k=0;
    }

  for ( ; str[j] != '\0'; j++ )
    tempstr[k++] = str[j];
  ip[i] = atoi( tempstr );
  mprintf( sizeof(buf), buf, "%3d.%3d.%3d.%3d", ip[0], ip[1], ip[2], ip[3] );
  return buf;
}

char *to_upper( char *str )
{
  static char strcap[MAX_STRING_LENGTH];
  int i;

  for ( i = 0; str[i] != '\0'; i++ )
    strcap[i] = UPPER(str[i]);
  strcap[i] = '\0';
  return strcap;
}

char *show_stat_info( CHAR_DATA *ch, int current, int max, int length, char *fill, char *blank, bool mana_check )
{
  static char buf[100];
  if (max == 0)
  {
    //Let Imms give mobs 0 mana if they want - RWL 12-28-2010
    if ( !mana_check )
      bugf("%s has a max of %d?.\n\r", ch->name, max);
    return("ZERO");
  }
    
  
  if (IS_SET(ch->plr2, PLR2_STATS)) {
    mprintf(sizeof(buf), buf,"{W[%5d/%-5d]{x",current, max);
    strcpy(buf,center_align(buf,length));
  }
  else
    strcpy(buf,color_percent_bar( current, max, length, fill, blank ));

  return buf;    
}

/*
 * Formats a certain range of lines in a string.
 * line1 = begin line, line2 = end line
 */
char *format_string_lines( char *oldstring, int line1, int line2)
{
  char	xbuf[MAX_STRING_LENGTH];
  BUFFER	*block1;
  BUFFER	*block2;
  BUFFER	*block3;
  char *str = oldstring;
  int	 i;

  if ( IS_NULLSTR( oldstring ) )
    return &str_empty[0];

  block1 = new_buf();
  block2 = new_buf();
  block3 = new_buf();

  /* Get the first block - not going to format this. */
  for ( i = 1; i < line1 && *str != '\0'; i++ )
    {
      str = string_getline( str, xbuf );
      bstrcat( block1, xbuf );
      bstrcat( block1, "\n\r" );
    }

  /* Get the block to format. */
  for ( i = line1; i <= line2 && *str != '\0'; i++ )
    {
      str = string_getline( str, xbuf );
      bstrcat( block2, xbuf );
      bstrcat( block2, "\n\r" );
    }

  /* Get the last block - not going to format this. */
  for ( ; *str != '\0'; )
    {
      str = string_getline( str, xbuf );
      bstrcat( block3, xbuf );
      bstrcat( block3, "\n\r" );
    }

  /* Format the middle block & free it. */
  str = str_dup( buf_string( block2 ), str );
  free_buf( block2 );
  str = format_string( str);

  /* Add formatted block to first block & free it. */
  bstrcat( block1, str );
  free_string( str );

  /* Add last black & free it. */
  bstrcat( block1, buf_string( block3 ) );
  free_buf( block3 );

  /* Check length. */
  if ( bstrlen( block1 ) >= MAX_STRING_LENGTH - 6 )
    {
      free_buf( block1 );
      return oldstring;
    }

  /* Free oldstring and copy new. */
  free_string( oldstring );
  str = str_dup( buf_string( block1 ) , str);
  free_buf( block1 );

  return str;
}
/*
 * Formats a certain range of lines in a string.
 * line1 = begin line, line2 = end line
 */
char *reply_format_string_lines( char *oldstring, int line1, int line2 ,
			   bool reply_formatting)
{
  char	xbuf[MAX_STRING_LENGTH];
  BUFFER	*block1;
  BUFFER	*block2;
  char *str = oldstring;
  int	 i=0;

  if ( IS_NULLSTR( oldstring ) )
    return &str_empty[0];

  block1 = new_buf();
  block2 = new_buf();

  /* Get the first block - not going to format this. */
  for (i=0 ; *str != '\0';i++ )
    {
      str = string_getline( str, xbuf );
      if (xbuf[0] != '>' || xbuf[1] != '>')
	{
	  bstrcat(block2, xbuf);
	  bstrcat( block2, "\n\r" );
  	  break;
	}
      bstrcat( block1, xbuf );
      bstrcat( block1, "\n\r" );
    }
  /* Get the block to format. */
  for ( ; *str != '\0'; )
    {
      str = string_getline( str, xbuf );
      bstrcat( block2, xbuf );
      bstrcat( block2, "\n\r" );
    }


  /* Format the middle block & free it. */
  str = str_dup( buf_string( block2 ), str );
  free_buf( block2 );
  str = format_string_lines( str , line1 - i, line2 -i);

  /* Add formatted block to first block & free it. */
  bstrcat( block1, str );
  free_string( str );


  /* Check length. */
  if ( bstrlen( block1 ) >= MAX_STRING_LENGTH - 6 )
    {
      free_buf( block1 );
      return oldstring;
    }

  /* Free oldstring and copy new. */
  free_string( oldstring );
  str = str_dup( buf_string( block1 ) , str);
  free_buf( block1 );

  return str;
}


/*
 * Count number of lines in string.
 */
int string_linecount( const char *str )
{
  char *s;
  int  cnt = 0;

  for ( s = (char *)str; *s != '\0'; )
    if ( *s++ == '\n' )
      cnt++;

  return cnt;
}


/*
 * Returns TRUE if the string ends with a punctuation mark.
 */
bool check_punct( char *str )
{
  bool punctuation = FALSE;
  char *p;

  if ( IS_NULLSTR( str ) )
    return FALSE;

  for ( p = str; *p != '\0'; )
    {
      if ( *p == '#' )
	p += 2;
      else if ( *p == '!' || *p == '.' || *p == '?' || *p == ','
		||        *p == ':' || *p == ';' )
	p++, punctuation = TRUE;
      else if ( isspace( *p ) )
	p++;
      else
	p++, punctuation = FALSE;
    }

  return punctuation;
}


/*
 * Get a single line of text from string.
 */
char *string_getline( char *str, char *line )
{
  int  tmp = 0;
  bool found = FALSE;

  while ( *str != '\0' )
    {
      if ( *str == '\n' )
	{
	  found = TRUE;
	  break;
	}
      line[tmp++] = *(str++);
    }

  if ( found )
    {
      if ( *(str+1) == '\r' )
	str += 2;
      else
	str += 1;
    }

  line[tmp] = '\0';

  return str;
}
