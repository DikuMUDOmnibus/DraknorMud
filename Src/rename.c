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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "interp.h"

void do_rename( CHAR_DATA *ch, char *argument )
{
  char arg1[MSL],arg2[MSL];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  char str[MSL];

  if (!IS_IMMORTAL(ch))
    {
      send_to_char("Who do you think you are?  A God?\n\r",ch);
      return;
    }

  argument = one_argument(argument,arg1);
  argument = one_argument(argument,arg2);

  if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char( "Syntax: RENAME NAME NEW_NAME\n\r", ch );
      return;
    }
  if (!str_cmp(arg1, ch->name))
    {
      send_to_char("You cannot rename yourself.\n\r", ch);
      return;
    }

  if ((victim = get_char_room(ch,arg1)) == NULL)
    {
      send_to_char("They aren't here in the room.\n\r",ch);
      return;
    }

  if ( !str_cmp( arg1, arg2 ) )
    {
      send_to_char("The new name must be different from the original.\n\r", ch);
      return;
    }

  if ( !check_parse_name( arg2 ) )
    {
      send_to_char("Illegal name, try another.\n\r", ch );
      return;
    }

  if ((rename_char(victim, arg2)!= 0))
    {
      send_to_char("Error in renaming char, either is NPC or Already exists a Char.\n=r",ch);
      return;
    }
  printf_to_char(victim,"Your name has been renamed to %s.\n\r",arg2);
  sprintf(str,"%s%s",PLAYER_DIR, capitalize(arg1));
  unlink(str);
  /* Fix ownership of objects. */
  for ( obj = victim->carrying; obj; obj = obj->next )
    {
      if ( !IS_NULLSTR( obj->owner )
	   &&   !IS_DELETED( obj )
	   &&   !str_cmp( obj->owner, capitalize(arg1) ) )
	{
	  free_string( obj->owner );
	  obj->owner = str_dup( victim->name, obj->owner );
	}
    }

  /* Fix note sender, if any. */
  if ( victim->pnote )
    {
      free_string( victim->pnote->sender );
      victim->pnote->sender = str_dup( victim->name, victim->pnote->sender );
    }
  printf_to_char(ch,"%s has been renamed to %s\n\r", arg1, victim->name);
  return;
}

int rename_char(CHAR_DATA *ch, char *to_name)
{
  char afile2[MSL];
  FILE *fp;
  if (IS_NPC(ch))
    return(-1);

  if ( ch->desc != NULL && ch->desc->original != NULL )
    ch = ch->desc->original;
  
  mprintf(sizeof(afile2),afile2,"%s%s",PLAYER_DIR,capitalize(to_name));
  if ((fp = fopen(afile2,"r")) == NULL)
    {
      clannie_roster_rename(ch, to_name);

      free_string(ch->name);
      ch->name = str_dup(capitalize(to_name), ch->name);
#if MEMDEBUG
      free_string(ch->memdebug_name);
      ch->memdebug_name =str_dup(capitalize(to_name), ch->memdebug_name);
#endif
      save_char_obj(ch, FALSE);
      return(0);
    }
  else
    {
      nFilesOpen++;
      fclose(fp);
      nFilesOpen--;
      return(-1);
    }
}
  


