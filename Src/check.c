/***************************************************************************
*       FALLEN ANGLEL mud is protected by french law of intelectual        *
*                            property. we share freely all we have coded   *
*                            provided this message isn't removed from the  *
*                            files and you respect the name of all the     *
*                            coders,area builders, and all the Diku,Merc,  *
*                            Rom licences.                                 *
*                                                                          *
*                   Thank to : ROM consortium .                            *
*                   Big thank to : Gary Gygax !!!!!!!!!!!!!!               *
*                                                                          *
*       Fallen Angel project by : Loran      ( laurent zilbert )           *
*                                 Silfen or                                *
*                                 Gwendoline ( jerome despret  )           *
*                                                                          *
*       Despret@ecoledoc.lip6.fr ...                                       *
***************************************************************************/

/***************************************************************************
*                                                                          *
*  To use this snipet you must set the following line in the "check" help  *
*                                                                          *
*    Coded for Fallen Angels by : Zilber laurent,Despret jerome.           *
*                                                                          *
*  And send a mail to say you use it ( feel free to comment ) at :         *
*                                                                          *
*  [despret@ecoledoc.lip6.fr] or/and at [loran@hotmail.com]                *
****************************************************************************/

/***************************************************************************
*                                                                          *
* If you want to put this snipet on your web site you are allowed to but   *
*  the file must remain unchanged and you have to send us a mail at :      *
*                                                                          *
*  [despret@ecoledoc.lip6.fr] or/and at [loran@hotmail.com]                *
*  with the site URL.                                                      *
*                                                                          *
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
   This imm command is to fight cheaters ....
   Allow you to quick detect/compare modified chars...

    Syntax: 'check'       display info about players of all players
            'check stats' display info and resume stats of all players
            'check eq'    resume eq of all players
            'check snoop' show who snoop who ( to avoid lowest imm abuse )
    Use the stat command in case of doubt about someone...
*/
    
#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "db.h"
#include "recycle.h"
#include "interp.h"
#include "magic.h"


/* Immortal command */

void do_check( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  CHAR_DATA *victim;
  int count = 1;
  bool found = FALSE;
  int cmd =0;
    
  one_argument( argument, arg );
    
  if (arg[0] == '\0'|| !str_prefix(arg,"stats"))
    {
      buffer = new_buf();
      for (victim = char_list; victim != NULL; victim = victim->next)
    	{
	  if (IS_NPC(victim) || !can_see(ch,victim)) 
	    continue;
    	    	
	  if (victim->desc == NULL)
	    {
	      mprintf(sizeof(buf), buf,"%3d) %s is linkdead.\n\r", count, victim->name);
	      add_buf(buffer, buf);
	      count++;
	      continue;	    	
	    }
	    
	  if (victim->desc->connected >= CON_GET_NEW_RACE
	      && victim->desc->connected <= CON_PICK_WEAPON)
	    {
	      mprintf(sizeof(buf), buf,"%3d) %s is being created.\n\r",
		      count, victim->name);
	      add_buf(buffer, buf);
	      count++;
	      continue;
	    }
	    
	  if ( (victim->desc->connected == CON_GET_OLD_PASSWORD
		|| victim->desc->connected >= CON_READ_IMOTD)
	       && get_trust(victim) <= get_trust(ch) )
	    {
	      mprintf(sizeof(buf), buf,"%3d) %s is connecting.\n\r",
		      count, victim->name);
	      add_buf(buffer, buf);
	      count++;
	      continue; 	    		 
	    }
	    
	  if (victim->desc->connected == CON_PLAYING)
	    {
	      if (get_trust(victim) > get_trust(ch))
		mprintf(sizeof(buf), buf,"%3d) %s.\n\r", count, victim->name);
	      else
	        {
		  mprintf(sizeof(buf), buf,"%3d) %s, Level %d connected since %d hours (%d total hours)\n\r",
			  count, victim->name,victim->level,
			  ((int)(current_time - victim->logon)) /3600, 
			  TOTAL_PLAY_TIME(victim) /3600 );
		  add_buf(buffer, buf);
		  if (arg[0]!='\0' && !str_prefix(arg,"stats"))
		    {
		      mprintf(sizeof(buf), buf,"  %d HP %d Mana (%d %d %d %d %d) %ld golds %d Tr %d Pr %d Qpts.\n\r",
			      victim->max_hit, victim->max_mana,victim->perm_stat[STAT_STR],
			      victim->perm_stat[STAT_INT],victim->perm_stat[STAT_WIS],
			      victim->perm_stat[STAT_DEX],victim->perm_stat[STAT_CON],
			      victim->gold + victim->silver/100,
			      victim->train, victim->practice, victim->questdata->points);
		      add_buf(buffer, buf);
		    }
		  count++;
		}
	      continue;
	    }
	    
	  mprintf(sizeof(buf), buf,"%3d) bug (oops)...please report to Loran: %s %d\n\r",
		  count, victim->name, victim->desc->connected);
	  add_buf(buffer, buf);
	  count++;   
    	}
      page_to_char(buf_string(buffer),ch);
      free_buf(buffer);
      return;
    }
    
  if (!str_prefix(arg,"eq"))
    {
      buffer = new_buf();
      for (victim = char_list; victim != NULL; victim = victim->next)
	{
	  if (victim->desc != NULL) {
	    if (IS_NPC(victim) 
		|| victim->desc->connected != CON_PLAYING
		|| !can_see(ch,victim)
		|| get_trust(victim) > get_trust(ch) )
	      continue;
    	    	
	    mprintf(sizeof(buf), buf,"%3d) %s, %d items (weight %d) Hit:%d Dam:%d Save:%d AC:%d %d %d %d.\n\r",
		    count, victim->name, victim->carry_number, victim->carry_weight, 
		    victim->hitroll, victim->damroll, victim->saving_throw,
		    victim->armor[AC_PIERCE], victim->armor[AC_BASH],
		    victim->armor[AC_SLASH], victim->armor[AC_EXOTIC]);
	    add_buf(buffer, buf);
	    count++;
	  }
	  page_to_char(buf_string(buffer),ch);
	  free_buf(buffer);    	
	  return;
	}
    }

  if (!str_prefix(arg,"snoop")) /* this part by jerome */
    {
      char bufsnoop [100];

      if(ch->level < MAX_LEVEL )
	{
	  send_to_char("You can't use this check option.\n\r",ch);
	  return;
	}
      buffer = new_buf();

      for (victim = char_list; victim != NULL; victim = victim->next)
        {
	  if (IS_NPC(victim)
	      || victim->desc->connected != CON_PLAYING
	      || !can_see(ch,victim)
	      || get_trust(victim) > get_trust(ch) )
	    continue;

	  if(victim->desc->snoop_by != NULL)
	    mprintf(sizeof(bufsnoop), bufsnoop," %15s .",victim->desc->snoop_by->character->name);
	  else
	    mprintf(sizeof(bufsnoop), bufsnoop,"     (none)      ." );

	  mprintf(sizeof(buf), buf,"%3d %15s : %s \n\r",count,victim->name, bufsnoop);
	  add_buf(buffer, buf);
	  count++;
        }
      page_to_char(buf_string(buffer),ch);
      free_buf(buffer);
      return;
    }

        
  if (!str_prefix(arg,"player"))
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;
      buffer = new_buf();
      for ( vch = player_list; vch != NULL; vch = vch_next )
	{
	  if (vch->next == NULL) 
	    return;
	  vch_next = vch->next_player;
	  if (IS_NPC(vch)) {
	    bprintf(buffer,"%s is a NPC.\n\r",vch->name);
	    continue;
	  }

    if ( !can_see(ch,vch))
        break;
      
	  if (IS_LINKDEAD(vch) ||
	      IS_AFK(vch)) {
	    bprintf(buffer,"%s is a AFK/LINKDEAD.\n\r",vch->name);
	    continue;
	  }
	  bprintf(buffer,"%s is currently playing.\n\r",vch->name);
	}
      page_to_char(buf_string(buffer),ch);
      free_buf(buffer);
      return;
    }
  if (!str_prefix(arg,"socials"))
    {
      buffer = new_buf();
      found = FALSE;
      for (cmd = 0; social_table[cmd].name[0] != '\0'; cmd++)
	{
	  if (social_table[cmd].char_no_arg[0] == '\0'
	      || social_table[cmd].others_no_arg[0] == '\0'
	      || social_table[cmd].char_found[0] == '\0'
	      || social_table[cmd].others_found[0] == '\0'
	      || social_table[cmd].vict_found[0] == '\0'
	      || social_table[cmd].char_auto[0]  == '\0'
	      || social_table[cmd].others_auto[0] == '\0')
	    /*	      || social_table[cmd].char_not_found == NULL*/
	    {
	      found = TRUE;
	      bprintf(buffer,"Incomplete Social: %s\n\r",social_table[cmd].name);
	    }
	}
      if (found)
	page_to_char(buf_string(buffer),ch);
      else
	send_to_char("All socials check out correct.\n\r",ch);
      free_buf(buffer);
      return;
    }
  
  if (!str_prefix(arg,"helps"))
    {
     bool any_found;
      int cmd = 0;
      HELP_DATA *pHelp;

      buffer = new_buf();
      any_found = FALSE;
      for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
	{
	  if (cmd_table[cmd].show != 1)
	    continue;

	  found = FALSE;
	  for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
	    {
	      if(pHelp->delete_it)
		continue;
	      
	      
	      if (is_name(cmd_table[cmd].name, pHelp->keyword)) {
		any_found = found = TRUE;
		break;
	      }
	    }

	  if ( !found )
	    bprintf(buffer,"Missing Help: %s\n\r",cmd_table[cmd].name);
	}
      if (any_found)
	page_to_char(buf_string(buffer),ch);
      else
	send_to_char("All helps check out correct.\n\r",ch);
      free_buf(buffer);
      return;
    }
  if (!str_prefix(arg,"commands"))
    {
      int cmd = 0;
      buffer = new_buf();
      found = FALSE;
      for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
	{
	  if (cmd_table[cmd].show != 1) {
	    found = TRUE;
	    bprintf(buffer,"NotShow Command: %s\n\r",cmd_table[cmd].name);
	  }
	}
      if (found)
	page_to_char(buf_string(buffer),ch);
      else
	send_to_char("All helps check out correct.\n\r",ch);
      free_buf(buffer);
      return;
    }
  send_to_char("Syntax: 'check'        display info about players\n\r",ch);
  send_to_char("        'check stats'  display info and resume stats\n\r",ch);
  send_to_char("        'check eq'     resume eq of all players\n\r",ch);
  send_to_char("        'check player' player list check\n\r",ch);
  send_to_char("        'check social' socials check\n\r",ch);
  send_to_char("        'check helps'  helps on items check\n\r",ch);
  send_to_char("        'check commands'  Shows nonshown commands\n\r",ch);
  send_to_char("Use the stat command in case of doubt about someone...\n\r",ch);
  return;
}
