/* Online setting of skill/spell levels, 
 * (c) 1996 Erwin S. Andreasen <erwin@pip.dknet.dk>
 *
 */

/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/
#if defined( macintosh )
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <errno.h>		
#include <unistd.h>		
#include <sys/time.h>
#endif
#include <ctype.h>		
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "recycle.h"


/* 

  Class table levels loading/saving
  
*/

/* Save this class */
void save_class (int num)
{
  FILE *fp;
  char buf[MAX_STRING_LENGTH];
  int lev, i;
	
  mprintf (sizeof(buf), buf, "%s/%s", CLASS_DIR, class_table[num].name);
  if (!(fp = fopen (buf, "w")))
    {
      printf ("Could not open file %s in order to save class %s.", buf, class_table[num].name);
      return;
    }
  nFilesOpen++;
	
  for (lev = 0; lev < LEVEL_IMMORTAL; lev++)
    for (i = 0; i < MAX_SKILL; i++)
      {
	if (!skill_table[i].name || !skill_table[i].name[0])
	  continue;
				
	if (skill_table[i].skill_level[num] == lev)
	  fprintf (fp, "%d %s\n", lev, skill_table[i].name);
      }
	
  fprintf (fp, "-1"); /* EOF -1 */
  fclose (fp);
  nFilesOpen--;
}



void save_classes()
{
  int i;
	
  for (i = 0; i < MAX_CLASS; i++)
    save_class (i);
}


/* Load a class */
void load_class (int num)
{
  char buf[MAX_STRING_LENGTH];
  int level,n;
  FILE *fp;
	
  mprintf (sizeof(buf), buf, "%s/%s", CLASS_DIR, class_table[num].name);
	
  if (!(fp = fopen (buf, "r")))
    {
      printf ("Could not open file %s in order to load class %s.", buf, class_table[num].name);
      return;
    }
  nFilesOpen++;
  fscanf (fp, "%d", &level);
	
  while (level != -1)
    {
      fscanf (fp, " %[^\n]\n", buf); /* read name of skill into buf */
		
      n = skill_lookup (buf); /* find index */
		
      if (n == -1)
	{
	  char buf2[200];
	  mprintf (sizeof(buf2), buf2, "Class %s: unknown spell %s", class_table[num].name, buf);
	  bug (buf2, 0);
	}
      else
	skill_table[n].skill_level[num] = level;

      fscanf (fp, "%d", &level);
    }
	
  fclose (fp);
  nFilesOpen--;
}
	
void load_classes ()
{
  int i,j;

  for (i = 0; i < MAX_CLASS; i++)
    {
      for (j = 0; j < MAX_SKILL; j++)
	skill_table[j].skill_level[i] = LEVEL_IMMORTAL;
		
      load_class (i);
    }
}



void do_skill (CHAR_DATA *ch ,char * argument)
{
  char class_name[MIL], skill_name[MIL];
  int sn, level, class_no;
	
  argument = one_argument (argument, class_name);
  argument = one_argument (argument, skill_name);
	
  if (!argument[0])
    {
      send_to_char ("Syntax is: SKILL <class> <skill> <level>.\n\r",ch);
      return;
    }
	
  level = atoi (argument);
	
  if (!is_number(argument) || level < 0 || level > LEVEL_IMMORTAL)
    {
      printf_to_char (ch, "Level range is from 0 to %d.\n\r", LEVEL_IMMORTAL);
      return;
    }
	
	
  if ( (sn = skill_lookup (skill_name)) == -1)
    {
      printf_to_char (ch, "There is no such spell/skill as '%s'.\n\r", skill_name);
      return;
    }
	
  for (class_no = 0; class_no < MAX_CLASS; class_no++)
    if (!str_cmp(class_name, class_table[class_no].who_name))
      break;
	
  if (class_no == MAX_CLASS)
    {
      printf_to_char (ch, "No class named '%s' exists. Use the 3-letter WHO names (Psi, Mag etc.)\n\r", class_name);
      return;
    }
	
  skill_table[sn].skill_level[class_no] = level;
	
  printf_to_char (ch, "OK, %ss will now gain %s at level %d%s.\n\r", class_table[class_no].name,
		  skill_table[sn].name, level, level == LEVEL_IMMORTAL ? " (i.e. never)" : "");
	
	
  save_classes();
}


/* Save this class */
void save_group (int num)
{
  FILE *fp;
  char buf[MAX_STRING_LENGTH];
  int lev, i;
	
  mprintf (sizeof(buf), buf, "%s/%s", GROUP_DIR, group_table[num].name);
  if (!(fp = fopen (buf, "w")))
    {
      printf ("Could not open file %s in order to save GROUP %s.", buf, group_table[num].name);
      return;
    }
  nFilesOpen++;
	
  for (lev = 0; lev < LEVEL_IMMORTAL; lev++)
    for (i = 0; i < MAX_CLASS; i++)
      {
	if (!class_table[i].name || !class_table[i].name[0])
	  continue;
				
	if (group_table[num].rating[i] == lev)
	  fprintf (fp, "%d %s\n", lev, class_table[i].name);
      }
	
  fprintf (fp, "-1"); /* EOF -1 */
  fclose (fp);
  nFilesOpen--;
}



void save_groups()
{
  int i;
	
  for (i = 0; i < MAX_GROUP; i++)
    save_group (i);
}


/* Load a class */
void load_group (int num)
{
  char buf[MAX_STRING_LENGTH];
  int level,n;
  FILE *fp;
	
  mprintf (sizeof(buf), buf, "%s/%s", GROUP_DIR, group_table[num].name);
	
  if (!(fp = fopen (buf, "r")))
    {
      printf ("Could not open file %s in order to load class %s.", buf, group_table[num].name);
      return;
    }
  nFilesOpen++;
  fscanf (fp, "%d", &level);
	
  while (level != -1)
    {
      fscanf (fp, " %[^\n]\n", buf); /* read name of class into buf */
		
      n = class_lookup (buf); /* find index */
		
      if (n == -1)
	{
	  char buf2[200];
	  mprintf (sizeof(buf2), buf2, "Class %s: unknown spell %s", group_table[num].name, buf);
	  bug (buf2, 0);
	}
      else {
	group_table[num].rating[n] = level;
      }

      fscanf (fp, "%d", &level);

    }
	
  fclose (fp);
  nFilesOpen--;
}
	
void load_groups ()
{
  int i,j;

  for (i = 0; i < MAX_GROUP; i++)
    {
      for (j = 0; j < MAX_CLASS; j++)
	group_table[i].rating[j] = 0;
		
      load_group (i);
    }
}



void do_grp (CHAR_DATA *ch ,char * argument)
{
  char group_name[MIL], class_name[MIL];
  int sn, level, group_no;
	
  argument = one_argument (argument, group_name);
  argument = one_argument (argument, class_name);
	
  if (!argument[0])
    {
      send_to_char ("Syntax is: SETGRP <group> <class> <CPs>.\n\r",ch);
      return;
    }
	
  level = atoi (argument);
	
  if (!is_number(argument) || level < 0 || level > LEVEL_IMMORTAL)
    {
      printf_to_char (ch, "Level range is from 0 to %d.\n\r", LEVEL_IMMORTAL);
      return;
    }
	
	
  if ( ( sn = class_lookup( class_name ) ) == -1 )
    {
      printf_to_char (ch, "There is no such class as '%s'.\n\r", class_name);
      return;
    }
	
  for (group_no = 0; group_no < MAX_GROUP; group_no++)
    if (!str_cmp(group_name, group_table[group_no].name))
      break;
	
  if (group_no == MAX_GROUP)
    {
      printf_to_char (ch, "No GROUP named '%s' exists. )\n\r", group_name);
      return;
    }
	
  group_table[group_no].rating[sn]= level;
	
  printf_to_char (ch, "OK, %ss will now gain %s at CPs %d%s.\n\r", class_table[sn].name,
		  group_table[group_no].name, level, level == LEVEL_IMMORTAL ? " (i.e. never)" : "");
	
	
  save_groups();
}

void do_classname(CHAR_DATA *ch, char *argument)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  int sn;
  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if ( arg1[0] == '\0'|| arg2[0] == '\0')
    {
      send_to_char("Syntax: classname oldclassname newclassname",ch);
      return;
    }

  sn = class_lookup(arg1);
  if (sn == -1) {
    send_to_char("That class does not exist\0",ch);
    return;
  }
  /*    strncpy(class_table[sn].name,arg2,MSL);
	mprintf(sizeof(buf), buf,"Class #%s is now named %s\n\r",arg1,arg2);
	send_to_char(buf,ch);*/
  return;
}

/* Save this class */
void save_creation_point (int num)
{
  FILE *fp;
  char buf[MAX_STRING_LENGTH];
  int lev, i;
	
  mprintf (sizeof(buf), buf, "%s/%s", GROUP_DIR, class_table[num].name);
  if (!(fp = fopen (buf, "w")))
    {
      printf ("Could not open file %s in order to save class %s.", buf, class_table[num].name);
      return;
    }
  nFilesOpen++;
	
  for (lev = 0; lev < LEVEL_IMMORTAL; lev++)
    for (i = 0; i < MAX_SKILL; i++)
      {
	if (!skill_table[i].name || !skill_table[i].name[0])
	  continue;
				
	if (skill_table[i].rating[num] == lev)
	  fprintf (fp, "%d %s\n", lev, skill_table[i].name);
      }
	
  fprintf (fp, "-1"); /* EOF -1 */
  fclose (fp);
  nFilesOpen--;
}



void save_creation_points()
{
  int i;
	
  for (i = 0; i < MAX_CLASS; i++)
    save_creation_point(i);
}


/* Load a class */
void load_creation_point (int num)
{
  char buf[MAX_STRING_LENGTH];
  int level,n;
  FILE *fp;
	
  mprintf (sizeof(buf), buf, "%s/%s", GROUP_DIR, class_table[num].name);
	
  if (!(fp = fopen (buf, "r")))
    {
      printf ("Could not open file %s in order to load class %s.", buf, class_table[num].name);
      return;
    }
  nFilesOpen++;
  fscanf (fp, "%d", &level);
	
  while (level != -1)
    {
      fscanf (fp, " %[^\n]\n", buf); /* read name of skill into buf */
		
      n = skill_lookup (buf); /* find index */
		
      if (n == -1)
	{
	  char buf2[200];
	  mprintf (sizeof(buf2), buf2, "Class %s: unknown spell %s", class_table[num].name, buf);
	  bug (buf2, 0);
	}
      else
	skill_table[n].rating[num] = level;

      fscanf (fp, "%d", &level);
    }
	
  fclose (fp);
  nFilesOpen--;
}
	
void load_creation_points ()
{
  int i,j;

  for (i = 0; i < MAX_CLASS; i++)
    {
      for (j = 0; j < MAX_SKILL; j++)
	skill_table[j].rating[i] = LEVEL_IMMORTAL;
		
      load_creation_point (i);
    }
}



void do_cps (CHAR_DATA *ch ,char * argument)
{
  char class_name[MIL], skill_name[MIL], skill2_name[MIL];
  int sn, level, class_no;
	
  argument = one_argument (argument, class_name);
  argument = one_argument (argument, skill2_name);
	
  if (!argument[0])
    {
      send_to_char ("Syntax is: SETCPS <class> <skill> <cps>.\n\r",ch);
      return;
    }
	
  level = atoi (argument);
	
  if (!is_number(argument) || level < 0 || level > 99)
    {
      printf_to_char (ch, "Level range is from 0 to %d.\n\r", 99);
      return;
    }
	
  strip_color(skill_name,skill2_name);
  
  if ( (sn = skill_lookup (skill_name)) == -1)
    {
      printf_to_char (ch, "There is no such spell/skill as '%s'.\n\r", skill_name);
      return;
    }
	
  for (class_no = 0; class_no < MAX_CLASS; class_no++)
    if (!str_cmp(class_name, class_table[class_no].who_name))
      break;
	
  if (class_no == MAX_CLASS)
    {
      printf_to_char (ch, "No class named '%s' exists. Use the 3-letter WHO names (Psi, Mag etc.)\n\r", class_name);
      return;
    }
	
  skill_table[sn].rating[class_no] = level;
	
  printf_to_char (ch, "OK, %ss will now gain %s at level %d%s.\n\r", class_table[class_no].name,
		  skill_table[sn].name, level, level == LEVEL_IMMORTAL ? " (i.e. never)" : "");
	
	
  save_creation_points();
}

void do_classlist(CHAR_DATA *ch, char *argument)
{
  int i,j;
  BUFFER *buffer;
  char buf[MSL];

  buffer = new_buf();
  if ((argument[0] == '\0') || !str_cmp(argument,"all")) {
    add_buf(buffer,"{WLISTING OF ITEMS FOR CLASSES{x\n\r");
    for (i=0; i < MAX_CLASS; i++){
      add_buf(buffer," ");
      mprintf(sizeof(buf), buf,"%s",class_table[i].who_name);
      add_buf(buffer,buf);
    }
    add_buf(buffer,"{C SKILLNAME{x\n\r");
    for (i= 0; i < MAX_SKILL;i++) {
      for (j=0;j < MAX_CLASS; j++) {
	mprintf(sizeof(buf), buf," %2d ",skill_table[i].skill_level[j]);
	add_buf(buffer,buf);
      }
      add_buf(buffer," ");
      add_buf(buffer,skill_table[i].name);
      add_buf(buffer,"\n\r");
    }
  } else {
    mprintf(sizeof(buf), buf,"{WLISTING OF skill/spell %s FOR CLASSES{x\n\r",argument);
    add_buf(buffer,buf);
    for (i=0; i < MAX_CLASS; i++){
      add_buf(buffer," ");
      mprintf(sizeof(buf), buf,"%s",class_table[i].who_name);
      add_buf(buffer,buf);
    }
    add_buf(buffer,"{C SKILLNAME{x\n\r");
    for (i= 0; i < MAX_SKILL;i++) {
      if (is_name(argument,skill_table[i].name)) {
	for (j=0;j < MAX_CLASS; j++) {
	  mprintf(sizeof(buf), buf," %2d ",skill_table[i].skill_level[j]);
	  add_buf(buffer,buf);
	}
	add_buf(buffer," ");
	add_buf(buffer,skill_table[i].name);
	add_buf(buffer,"\n\r");
      }
    }
  }
  page_to_char(buf_string(buffer),ch);
  free_buf(buffer);
}


void do_groupcps(CHAR_DATA *ch, char *argument)
{
  int i,j;
  FILE *fp;
  int temp_int = -1;
  char temp_str[5];
  fp = fopen("GROUPMAP.TXT","w");
  nFilesOpen++;
  send_to_char("{WLISTING OF CPS for GROUPS via CLASSES..\n\r",ch);
  fprintf(fp,"MAP of GROUPS in the MAGE's LAIR MUD \n\r");
  for (i=0; i < MAX_CLASS; i++){
    send_to_char(" ",ch);
    send_to_char(class_table[i].who_name,ch);
  }

  for (i=0; i < MAX_CLASS; i++){
    fprintf(fp,"%s",class_table[i].name);
    fprintf(fp," ");
  }
  fprintf(fp," GROUPNAME \n\r");
  send_to_char("{CGROUPNAME{x ",ch);
  send_to_char("\n\r",ch);
  for (i= 0; i < MAX_GROUP;i++) {
      
    for (j=0;j < MAX_CLASS; j++) {
      temp_int = group_table[i].rating[j];
      mprintf(sizeof(temp_str), temp_str,"%d",temp_int);
      fprintf(fp," %-2s ", (99 == temp_int ||
			    temp_int -1) ? " "
	      : "X");
	  
      printf_to_char(ch," %-2s ",(99 == temp_int ||
				  temp_int == -1) ? "-"
		     : temp_str);
    }
    fprintf(fp," %s \n\r",group_table[i].name);
    send_to_char(group_table[i].name,ch);
    send_to_char("\n\r",ch);
  }
  fclose(fp);
  nFilesOpen--;
}

void do_classcps(CHAR_DATA *ch, char *argument)
{
  BUFFER *buffer;
  char buf[MSL];
  int temp_int = -1;
  char temp_str[5];
  int i,j;

  buffer = new_buf();
  add_buf(buffer,"{WLISTING OF CPS FOR CLASSES\n\r");
  for (i=0; i < MAX_CLASS; i++){
    mprintf(sizeof(buf), buf," %s",class_table[i].who_name);
    add_buf(buffer,buf);
  }
  add_buf(buffer,"{CCLASSNAME{x \n\r");
  for (i= 0; i < MAX_SKILL;i++) {
    for (j=0;j < MAX_CLASS; j++) {
      temp_int =  skill_table[i].rating[j];
      mprintf(sizeof(temp_str), temp_str,"%d",temp_int);
      mprintf(sizeof(buf), buf," %-2s ",(99 == temp_int ||
			    temp_int == -1) ? "-"
	      : temp_str);
      add_buf(buffer,buf);
    }
    mprintf(sizeof(buf), buf,"{c%s{x\n\r",skill_table[i].name);
    add_buf(buffer,buf);
  }
  page_to_char(buf_string(buffer),ch);
  free_buf(buffer);
}

void do_saveclass(CHAR_DATA *ch, char *argument)
{
  save_classes();
  save_groups();
  save_creation_points();
  send_to_char("Classes and Creations are saved.\n\r",ch);
}

