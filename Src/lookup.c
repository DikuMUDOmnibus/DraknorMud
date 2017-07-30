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
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@efn.org)				   *
*	    Gabrielle Taylor						   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
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
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "merc.h"
#include "tables.h"

int64 flag_lookup (const char *name, const struct flag_type *flag_table)
{
  int flag;

  for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
      if (LOWER(name[0]) == LOWER(flag_table[flag].name[0])
	  &&  !str_prefix(name,flag_table[flag].name))
	return flag_table[flag].bit;
    }

  return NO_FLAG;
}



int clan_lookup(const char *name)
{
  int clan;

  if (!str_cmp("none", name))
    return 0;

  for (clan = 0; clan <= actual_num_clans; clan++)
    {
      if (LOWER(name[0]) == LOWER(clan_table[clan].name[0])
	  &&  !str_prefix(name,clan_table[clan].name))
	return clan;
    }

  return -1;
}
/*
int new_clan_lookup( const char *name )
{
    int clan;

    if ( !str_cmp( "none", name ) )
        return 0;

    for ( clan = 0; clan <= actual_num_clans2; clan++ )
    {
        if (LOWER(name[0]) == LOWER(clan->name[0])
        && !str_prefix( name, clan-> ) )
            return clan;
    }

   return -1;
}
*/

#define _LOOKUP(name, table, ret) \
                  for (ret=0; table[ret].name != NULL; ret++) \
                  { \
		      if (LOWER(name[0]) == LOWER(table[ret].name[0]) \
			  && !str_prefix(name,table[ret].name)) \
			return(ret); \
				       }
		      
int position_lookup (const char *name)
{
  int pos;
  _LOOKUP(name, position_table, pos);
  return -1;
}

int sex_lookup (const char *name)
{
   int sex;

   _LOOKUP(name,sex_table,sex);
   return -1;
}

int size_lookup (const char *name)
{
   int size;
   _LOOKUP(name,size_table,size);
   return -1;
}
/* returns race number */
int race_lookup (const char *name)
{
   int race;
   _LOOKUP(name,race_table,race);
   return 0;
} 
int liq_lookup (const char *name)
{
    int liq;

    for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
	if (LOWER(name[0]) == LOWER(liq_table[liq].liq_name[0])
	&& !str_prefix(name,liq_table[liq].liq_name))
            return liq;
    }
    return -1;
}


int item_lookup(const char *name)
{
  int type;

  for (type = 0; item_table[type].name != NULL; type++)
    {
      if (LOWER(name[0]) == LOWER(item_table[type].name[0])
	  &&  !str_prefix(name,item_table[type].name))
	return item_table[type].type;
    }
 
  return -1;
}



HELP_DATA * help_lookup( char *keyword )
{
	HELP_DATA *pHelp;
	char temp[MIL], argall[MIL];

	argall[0] = '\0';

	while (keyword[0] != '\0' )
	{
		keyword = one_argument(keyword, temp);
		if (argall[0] != '\0')
			strcat(argall," ");
		strcat(argall, temp);
	}

	for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
		if ( is_name( argall, pHelp->keyword ) )
		return pHelp;

	return NULL;
}

