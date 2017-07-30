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
#include <sys/time.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include "merc.h"
#include "tables.h"
#include "spells.h"

int actual_num_classes = MAX_CLASS;
int actual_num_skills = MAX_SKILL -1;
void load_spell_info(char *spell_filename);
void save_spell_info(char *spell_filename);
int find_spell_info_table_pos(char *comp_name);

void save_spell_info(char *spell_filename)
{
  FILE *fp;
  int i=0;
  int x=0;

  if ((fp = fopen(spell_filename, "w"))!= NULL) {
    nFilesOpen++;
    fprintf(fp,"#This is the SPELL RUNTIME table \n");
    fprintf(fp,"# Actual Number of Classes are saved \n");
    fprintf(fp,"%d\n",actual_num_classes);
    fprintf(fp,"# Actual number of skills are also saved\n");
    fprintf(fp,"%d\n",actual_num_skills);
    for (i = 0; i <= actual_num_skills; i++) {
      fprintf(fp,"# Spell Name\n");
      fprintf(fp,"%s\n",skill_table[i].name);
      printf("LOADING -%s-\n",skill_table[i].name);
      for (x=0;x< actual_num_classes; x++) {
	fprintf(fp,"#Skill level for class #%d named -%s-\n",x,class_table[x].who_name);
	fprintf(fp,"%d\n",skill_table[i].skill_level[x]);
      }
      for (x=0;x< actual_num_classes; x++) {
	fprintf(fp,"#Skill Rating for class #%d named -%s-\n",x,class_table[x].who_name);
	fprintf(fp,"%d\n",skill_table[i].rating[x]);
      }
      fprintf(fp,"# Targets.. legal Targets\n");
      fprintf(fp,"%d\n",skill_table[i].minimum_position);
      fprintf(fp,"# Slot for Object loading\n");
      fprintf(fp,"%d\n",skill_table[i].slot);
      fprintf(fp,"# Minimum Mana Cost\n");
      fprintf(fp,"%d\n",skill_table[i].min_mana);
      fprintf(fp,"# Waiting Time after use\n");
      fprintf(fp,"%d\n",skill_table[i].beats);
      fprintf(fp,"# Damage message\n");
      if (!strcmp(skill_table[i].noun_damage,""))
	fprintf(fp,"NULL\n");
      else
	fprintf(fp,"%s\n",skill_table[i].noun_damage);
      fprintf(fp,"# Wear Off Message\n");
      if (!strcmp(skill_table[i].msg_off,""))
	fprintf(fp,"NULL\n");
      else
	fprintf(fp,"%s\n",skill_table[i].msg_off);
      fprintf(fp,"# Wear off for Objects\n");
      if (!strcmp(skill_table[i].msg_obj,""))
	fprintf(fp,"NULL\n");
      else
	fprintf(fp,"%s\n",skill_table[i].msg_obj);
    }
    fclose(fp);
    nFilesOpen--; 
    printf("FILE SAVED %s\n",spell_filename);
  }

}

void load_spell_info(char *spell_filename)
{
  FILE *fp;
  char buf[512];
  int i=0;
  int x=0;
  int ret_val;

  printf("LOADING SPELLS NOW\n");
  if ((fp = fopen(spell_filename, "r"))!= NULL) {
    nFilesOpen++;
    get_next_line(fp,buf);
    actual_num_classes=atoi(buf);
    get_next_line(fp,buf);
    actual_num_skills= atoi(buf);
    for (i = 0; i <= actual_num_skills; i++) {
      get_next_line(fp,buf);
      strcpy(skill_table[i].name,buf);
      printf("Loading Skill/Spell %s\n",buf);
      for (x=0;x< actual_num_classes; x++) {
	get_next_line(fp,buf);
	skill_table[i].skill_level[x] = atoi(buf);
      }
      for (x=0;x< actual_num_classes; x++) {
	get_next_line(fp,buf);
	skill_table[i].rating[x] = atoi(buf);
      }
      get_next_line(fp,buf);
      skill_table[i].minimum_position = atoi(buf);
      get_next_line(fp,buf);
      skill_table[i].slot = atoi(buf);
      get_next_line(fp,buf);
      skill_table[i].min_mana = atoi(buf);
      get_next_line(fp,buf);
      skill_table[i].beats = atoi(buf);
      /*	    get_next_line(fp,buf);*/
      get_next_line(fp,buf);
      if (!strcmp(buf,"NULL"))
	strcpy(skill_table[i].noun_damage,"");
      else
	strcpy(skill_table[i].noun_damage,buf);
      get_next_line(fp,buf);
      fprintf(fp,"# Wear Off Message\n");
      if (!strcmp(buf,"NULL"))
	strcpy(skill_table[i].msg_off,"");
      else
	strcpy(skill_table[i].msg_off,buf);
      get_next_line(fp,buf);
      if (!strcmp(buf,"NULL"))
	strcpy(skill_table[i].msg_obj,"");
      else
	strcpy(skill_table[i].msg_obj,buf);
      ret_val = find_spell_info_table_pos(skill_table[i].name);
      skill_table[i].spell_fun =
	spell_func_table[ret_val].spell_function;
      skill_table[i].pgsn = spell_func_table[ret_val].gsn_pointer;
    }
    fclose(fp);
    nFilesOpen--;
    printf("FILE SAVED %s\n",spell_filename);
  }

}


int find_spell_info_table_pos(char *comp_name)
{
  int i;
  for (i=0; i <= actual_num_skills; i++) {
    if (!strcmp(comp_name,spell_func_table[i].spell_name))
      return(i);
  }
  printf("ERROR.. DAMN ERROR NO THING DFOUND\n");
    
  return(0);
}
