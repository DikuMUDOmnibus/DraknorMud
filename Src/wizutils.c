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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "lookup.h"
#include "recycle.h"

bool check_connecting_rooms  args( ( ROOM_INDEX_DATA *pRoom) );
bool check_doors    args( ( ROOM_INDEX_DATA *pRoom, ROOM_INDEX_DATA *pNext) );


enum roomcheck_enum {room_none_e, roomcheck_names_e, roomcheck_white_e, roomcheck_link_e, roomcheck_private_e, 
           roomcheck_descr_e, roomcheck_delete_e,
         roomcheck_bleed_e, roomcheck_lines_e};
enum objcheck_enum {obj_none_e, objcheck_avgdam_e,objcheck_hitdam_e,objcheck_flags_e,objcheck_cost_e, objcheck_hps_e, 
        objcheck_manapts_e, objcheck_saves_e,objcheck_spells_e, objcheck_delete_e, objcheck_style_e, 
        objcheck_none_e, objcheck_clan_e, objcheck_descr_e};
enum mobcheck_enum {mob_none_e, mobcheck_style_e, mobcheck_cost_e,
        mobcheck_maxcount_e, mobcheck_delete_e,
        mobcheck_hps_e, mobcheck_position_e, mobcheck_lines_e};

int count_char_in_string(char ch, char *str)
{
  int i=0, count=0;
  for (i=0; i< strlen(str); i++)
    if (str[i] == ch)
      count++;
  return count;
}


/* This file contains more and more implemnetor functions */
/* function subtracts groups for a victim.  Makes it
   easy to subtract groups */
/* this does based on a flag you pass.. give back points or not. */
void do_remgroup(CHAR_DATA *ch, char *argument)
{
  int gn;
  char arg1[MSL],arg2[MSL];
  CHAR_DATA *victim;
  argument = one_argument(argument,arg1);
  argument = one_argument(argument,arg2);

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  if (!is_number(argument))
    {
      send_to_char("Syntax: remgroup victim groupname #",ch);
      send_to_char("# - Number of trains given back\n\r",ch);
      return;
    }
  
  gn = group_lookup(arg2);
  if (gn != -1)
    {
      gn_remove(victim,gn);
      if (victim->pcdata->group_known[gn])
  gn_add(victim,gn);
      group_add(victim,class_table[ch->gameclass].base_group,FALSE);
      group_add(victim,"rom basics",FALSE);
      victim->train += atoi(argument);
      send_to_char("Group dropped.\n\r",ch);
    }
  else {
    send_to_char("No such group exists\n\r",ch);
    return;
  }
}

/* this function will remove all skills that a char cannot have based
   on race/class.  In case a char was set skill 100.  This will clean
   it up to avoid giving out extra skills. */
void do_cleanchar(CHAR_DATA *ch, char *argument)
{
  int sn=0;
  CHAR_DATA *victim;
  /* loop through the items the character has and see if they can
     based on the const.c skill tables and race skills.
  */

  if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  if (IS_NPC(victim)) {
    send_to_char("Duh.. not on a NPC.\n\r",ch);
    return;
  }
  
  for (sn=0; sn < MAX_SKILL; sn++)
    {
      if (victim->pcdata->learned[sn] > 0)
  if (skill_table[sn].skill_level[victim->gameclass] == -1) {
    if (!is_racial_skill(victim, sn))
      {
        printf_to_char(ch,"Removing %s from character.\n\r",skill_table[sn].name);
        victim->pcdata->learned[sn]=0;
      }
  }
    }
}

/* this function will remove all skills that a char cannot have based
   on race/class.  In case a char was set skill 100.  This will clean
   it up to avoid giving out extra skills. */
void do_pkable(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  /* loop through the items the character has and see if they can
     based on the const.c skill tables and race skills.
  */

  if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  if (IS_NPC(victim)) {
    send_to_char("Duh.. not on a NPC.\n\r",ch);
    return;
  }
  
  if (victim->pcdata->old_char) {
    send_to_char("They are now PK able again.\n\r",ch);
    victim->pcdata->old_char = FALSE;
  } else {
    send_to_char("They are not able to PK.\n\r",ch);
    victim->pcdata->old_char = TRUE;
  }    
}


void do_affremove(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;

  //loop through the items the character has and see if they can
  //based on the const.c skill tables and race skills.

  if (is_name(argument,"all"))
  {
    //remove everyones affects at once.
    for ( victim = player_list; victim; victim = victim->next_player )
    {
      if ( victim->desc == NULL || victim->desc->connected != CON_PLAYING )
        continue;

      while ( victim->affected )
      {
        affect_remove( victim, victim->affected );
        if ( ch->hit > GET_HP(ch) )
          ch->hit = GET_HP(ch);

        if ( ch->mana > GET_MANA(ch) )
          ch->mana = GET_MANA(ch);
      }

      victim->affected_by  = race_table[victim->race].aff;
      send_to_char("You have been cleansed by the Gods.\n\r",victim);
    }
    return;
  }

  if ( ( victim = get_char_world( ch, argument ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("Duh.. not on a NPC.\n\r",ch);
    return;
  }

  send_to_char("Removing all spells and affects on the character.\n\r",ch);

  while ( victim->affected )
  {
    affect_remove( victim, victim->affected );
    if ( ch->hit > GET_HP(ch) )
      ch->hit = GET_HP(ch);

    if ( ch->mana > GET_MANA(ch) )
     ch->mana = GET_MANA(ch);
  }

  victim->affected_by  = race_table[victim->race].aff;
  send_to_char("You have been cleansed by the Gods.\n\r",victim);
}

void do_areakill(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  AREA_DATA *pArea;
  BUFFER *buffer=0;
  int x =0;
  bool found = FALSE;

  for ( pArea = area_first_sorted ;pArea ;pArea = pArea->next_sort )
    {
      if (pArea->numkills > 0) {
  if (!found) {
    found = TRUE;
    buffer = new_buf();
    add_buf(buffer,"\n                     {Y======= {RAREA KILLS {Y========{x\n");
  }
  mprintf(sizeof(buf), buf, "{C%-30s  {C[{W%4d{C]{x ",pArea->name, pArea->numkills);
  add_buf(buffer, buf);
  if (x) {
    add_buf(buffer,"\n\r");
    x =0;
  } else
    x =1;
      }
    }
   
  /* Exit if no areas match */
  if (!found)
    {
      send_to_char_bw( "No areas meeting those criteria.\n\r", ch );
      return;
    } 
  else
    {
      add_buf(buffer,"\n\r");
      page_to_char(buf_string(buffer),ch);
      free_buf(buffer);
    }
  return;
}

void display_old_objcheck_syntax(CHAR_DATA *ch)
{
  send_to_char("SYNTAX:\n",ch);
  send_to_char("objcheck <command> [screen]\n\r",ch);
  send_to_char("    avgdam   - Do Average Damage check on Weapons.\n\r",ch);
  send_to_char("    hitdam   - To Hitdice/Damdice check.\n\r",ch);
  send_to_char("    flags    - Check number of flags on items.\n\r",ch);
  send_to_char("    cost     - Check cost of items.\n\r",ch);
  send_to_char("    hps      - Check Max Hitpoints on items.\n\r",ch);
  send_to_char("    manapts  - Check Max Manapoints on items.\n\r",ch);
  send_to_char("    saves    - Check Saves on an object.\n\r",ch);
  send_to_char("    spells   - Check Spell levels on an object.\n\r",ch);
  send_to_char("    delete   - Check DELETE status on objects.\n\r",ch);
  send_to_char("    style    - Check for material labeled OLDSTYLE on objects.\n\r",ch);
  send_to_char("    none     - Check for NONE/RESERVED spells on objects.\n\r",ch);
  send_to_char("    clan     - Check for clan items that are not claned.\n\r",ch);
  send_to_char("    [screen] - optional - after each command display to screen?\n\r",ch);
}

/* This isn't used any more
void do_old_objcheck( CHAR_DATA *ch, char *argument )
{
  OBJ_INDEX_DATA *pObjIndex;
  FILE *fp=0;
  int vnum,nMatch = 0;
  char buf[MAX_STRING_LENGTH];
  int avgdam=0;
  int totalpts = 0;
  BUFFER *buffer;
  char        arg1 [ MAX_INPUT_LENGTH ];
  char        arg2 [ MAX_INPUT_LENGTH ];
  sh_int weapon_flags=0;
  AFFECT_DATA *paf;
  OBJ_DATA *obj;
  bool violation = FALSE;
  bool screen = FALSE;
  int numflags = 0;
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (!str_cmp(arg1,"\0")) {
    display_old_objcheck_syntax(ch);
    return;
  }

  if (!str_cmp(arg2,"screen"))
    screen = TRUE;
  else
    screen = FALSE;
        
    
  if (!str_cmp(arg1,"hitdam"))
    fp = fopen("../Txt/objhitdam.txt","w");
  else if (!str_cmp(arg1,"avgdam"))
    fp = fopen("../Txt/objavgdam.txt","w");
  else if (!str_cmp(arg1,"flags"))
    fp = fopen("../Txt/objflags.txt","w");
  else if (!str_cmp(arg1,"cost"))
    fp = fopen("../Txt/objcost.txt","w");
  else if (!str_cmp(arg1,"hps"))
    fp = fopen("../Txt/objhps.txt","w");
  else if (!str_cmp(arg1,"manapts"))
    fp = fopen("../Txt/objmanapts.txt","w");
  else if (!str_cmp(arg1,"saves"))
    fp = fopen("../Txt/objsaves.txt","w");
  else if (!str_cmp(arg1,"spells"))
    fp = fopen("../Txt/objspells.txt","w");
  else if (!str_cmp(arg1,"delete"))
    fp = fopen("../Txt/objdelete.txt","w");
  else if (!str_cmp(arg1,"style"))
    fp = fopen("../Txt/objstyle.txt","w");
  else if (!str_cmp(arg1,"none"))
    fp = fopen("../Txt/objnone.txt","w");
  else if (!str_cmp(arg1,"clan"))
    fp = fopen("../Txt/objclan.txt","w");
  else {
    display_old_objcheck_syntax(ch);
    return;
  }
  if (fp == NULL) {
    display_old_objcheck_syntax(ch);
    return;
  }
  nFilesOpen++;
  buffer = new_buf();
  if (screen) {
    bprintf(buffer,"\nObject Analysis\n");
    bprintf(buffer,  "---------------\n");
  }
  nMatch = 0;
  fprintf(fp,"\nObject Analysis\n");
  fprintf(fp,  "---------------\n");
  for (vnum = 0; nMatch < top_obj_index; vnum++)
    if ((pObjIndex = get_obj_index(vnum)) != NULL)
      {
        nMatch++;
        obj = create_object( pObjIndex, 0 );

  if (!str_cmp(arg1,"avgdam")) {

    if (obj->item_type == ITEM_WEAPON) {
      if (IS_WEAPON_STAT(obj,WEAPON_CLAN))
        continue;
      if (obj->pIndexData->new_format)
        avgdam = (1 + obj->value[2]) * obj->value[1] / 2;
      else
        avgdam = ( obj->value[1] + obj->value[2] ) / 2;
      if (IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)) {
        if ((avgdam > obj->level+2) && (obj->level < LEVEL_HERO)) {
        
    if (avgdam > 7 || obj->level > 5 ) {
      mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d AVGDAM:%d\n",vnum,obj->name,
        obj->level,avgdam);
      if (screen)
        bprintf(buffer,buf);
      fprintf( fp, buf );
    }
        }
      } else {
        if ((avgdam > obj->level) && (obj->level < LEVEL_HERO)) {

    if (avgdam > 5 || obj->level > 5 ) {
      mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d AVGDAM:%d\n",vnum,obj->name,
        obj->level,avgdam);
      if (screen)
        bprintf(buffer,buf);
      fprintf( fp, buf );
    }
        }
      }
    }

  }else if (!str_cmp(arg1,"hitdam")) {
    if (obj->item_type == ITEM_WEAPON) {
      if (IS_WEAPON_STAT(obj,WEAPON_CLAN))
        continue;
    }
    for ( paf = pObjIndex->affected; paf != NULL; paf = paf->next )
      {
        if (paf->location == APPLY_HITROLL) {
    if (obj->level <=20) {
      if (paf->modifier > 2) {
        mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d HITROLL %d\n",vnum,obj->name,
          obj->level,paf->modifier);
        if (screen)
          bprintf(buffer,buf);
        fprintf(fp,buf);
      }
    } else {
      if (obj->level < ((paf->modifier*10)+1)) {
        mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d HITROLL %d\n",vnum,obj->name,
          obj->level,paf->modifier);
        if (screen)
          bprintf(buffer,buf);
        fprintf(fp,buf);
      }
    }
        }else if (paf->location == APPLY_DAMROLL) {
    if (obj->level <= 10) {
      if (paf->modifier > 1) {
        mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d DAMROLL %d\n",vnum,obj->name,
          obj->level,paf->modifier);
        if (screen)
          bprintf(buffer,buf);
        fprintf(fp,buf);
      }
    } else if (obj->level <=30) {
      if (paf->modifier > 2){
        mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d DAMROLL %d\n",vnum,obj->name,
          obj->level,paf->modifier);
        if (screen)
          bprintf(buffer,buf);
        fprintf(fp,buf);
      }
    }else {
      if (obj->level < ((paf->modifier*10)+1)) {
        mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d DAMROLL %d\n",vnum,obj->name,
          obj->level,paf->modifier);
        if (screen)
          bprintf(buffer,buf);
        fprintf(fp,buf);
      }
    }
        } else
    continue;
    
      }
    
        
  } else if (!str_cmp(arg1,"flags")) {
    if (obj->item_type == ITEM_WEAPON) {
      violation = FALSE;
      if (IS_WEAPON_STAT(obj,WEAPON_CLAN))
        continue;
      numflags =0;
      if (obj->value[4]) {
        weapon_flags = obj->value[4];
        if (weapon_flags & WEAPON_FLAMING   ) numflags ++;
        if (weapon_flags & WEAPON_FROST   ) numflags ++;
        if (weapon_flags & WEAPON_VAMPIRIC ) numflags ++;
        if (weapon_flags & WEAPON_SHARP   ) numflags ++;
        if (weapon_flags & WEAPON_VORPAL   ) numflags ++;
        if (weapon_flags & WEAPON_SHOCKING ) numflags ++;
        if (weapon_flags & WEAPON_POISON   ) numflags ++;
        if (weapon_flags & WEAPON_MANA_DRAIN ) numflags ++;
        if (weapon_flags & WEAPON_HOLY ) numflags++;
        if (weapon_flags & WEAPON_UNHOLY ) numflags++;
        if ((obj->level / 30) + 2 < numflags)
    violation = TRUE;
        if (obj->level > 90 && numflags < 6)
    violation = FALSE;
        if (violation) {
    mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d NUMFLAGS %d\n",vnum,obj->name,
      obj->level,numflags);
          
    if (screen)
      bprintf(buffer,buf);
    fprintf(fp,buf);
        }
      }
        
    }
    else if (!str_cmp(arg1,"cost")) {
      if (obj->cost > obj->level * 3) {
        mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d COST %d\n",vnum,obj->name,
          obj->level,obj->cost);
        if (screen)
    bprintf(buffer,buf);
        fprintf(fp,buf);
      }
        
    }
  }else if (!str_cmp(arg1,"hps")) {
    if (obj->item_type == ITEM_WEAPON) {
      if (IS_WEAPON_STAT(obj,WEAPON_CLAN))
        continue;
    }
    totalpts = 0;
    for ( paf = pObjIndex->affected; paf != NULL; paf = paf->next )
      {
        if (paf->location == APPLY_HIT) {
    totalpts += paf->modifier;
    if (obj->level < 6) {
      if (totalpts > 10 + obj->level) {
        mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d HITPTS %d\n",vnum,obj->name,
          obj->level,totalpts);
        if (screen)
          bprintf(buffer,buf);
        fprintf(fp,buf);
      }
    } else if (obj->level < 90) {
      if (totalpts > 2*obj->level) {
        mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d HITPTS %d\n",vnum,obj->name,
          obj->level, totalpts);
        if (screen)
          bprintf(buffer,buf);
        fprintf(fp,buf);
      }
    } else {
      if (totalpts > 3*obj->level) {
        mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d HITPTS %d\n",vnum,obj->name,
          obj->level,totalpts);
        if (screen)
          bprintf(buffer,buf);
        fprintf(fp,buf);
      }
      
    }
        
        }
      }
  }else if (!str_cmp(arg1,"manapts")) {
    if (obj->item_type == ITEM_WEAPON) {
      if (IS_WEAPON_STAT(obj,WEAPON_CLAN))
        continue;
    }
    totalpts = 0;
    for ( paf = pObjIndex->affected; paf != NULL; paf = paf->next )
      {
        if (paf->location == APPLY_MANA) {
    totalpts += paf->modifier;
    if (obj->level < 6) {
      if (totalpts > 10 + obj->level) {
        mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d MANAPTS %d\n",vnum,obj->name,
          obj->level,totalpts);
        if (screen)
          bprintf(buffer,buf);
        fprintf(fp,buf);
      }
    } else if (obj->level < 90) {
      if (totalpts > 2*obj->level) {
        mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d MANAPTS %d\n",vnum,obj->name,
          obj->level,totalpts);
        if (screen)
          bprintf(buffer,buf);
        fprintf(fp,buf);
      }
    } else {
      if (totalpts > 3*obj->level) {
        mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d MANAPTS %d\n",vnum,obj->name,
          obj->level,totalpts);
        if (screen)
          bprintf(buffer,buf);
        fprintf(fp,buf);
      }
    }
        }
        
      }
  }else if (!str_cmp(arg1,"saves")) {
    for ( paf = pObjIndex->affected; paf != NULL; paf = paf->next )
      {
        if (paf->location == APPLY_SAVES ||
      paf->location == APPLY_SAVING_ROD ||
      paf->location == APPLY_SAVING_PETRI ||
      paf->location == APPLY_SAVING_BREATH ||
      paf->location == APPLY_SAVING_SPELL ) {
    if ((obj->level/20 + 1) *-1 > paf->modifier) {
      mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d SAVES %d\n",vnum,obj->name,
        obj->level,paf->modifier);
      if (screen)
        bprintf(buffer,buf);
      fprintf(fp,buf);
    }
        }
        
      }
  }else if (!str_cmp(arg1,"spells")) {
    if (pObjIndex->item_type == ITEM_POTION ||
        pObjIndex->item_type == ITEM_SCROLL ||
        pObjIndex->item_type == ITEM_WAND)
      {
        if (obj->value[0] > obj->level) {
    mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d Spell Level %d\n\r",vnum, obj->name,
      obj->level,obj->value[0]);
    if (screen)
      bprintf(buffer,buf);
    fprintf(fp,buf);
        }
      }
  }else if (!str_cmp(arg1,"delete")) {
    if (is_name("delete",pObjIndex->name))
      {
        mprintf(sizeof(buf),buf,"[%d]%-30s LVL:%d\n\r",vnum, obj->name,
          obj->level);
        if (screen)
    bprintf(buffer,buf);
        fprintf(fp,buf);
      }
  }else if (!str_cmp(arg1,"style")) {
    if (is_name("style",pObjIndex->material))
      {
        mprintf(sizeof(buf),buf,"[%d]%-30s LVL:%d\n\r",vnum, obj->name,
          obj->level);
        if (screen)
    bprintf(buffer,buf);
        fprintf(fp,buf);
      }
  }else if (!str_cmp(arg1,"none")) {
    if (pObjIndex->item_type == ITEM_POTION ||
        pObjIndex->item_type == ITEM_SCROLL ||
        pObjIndex->item_type == ITEM_WAND)
      {
        if ( obj->value[1] <= 0) {
    mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d Spell NONE/RESERVED position 1\n\r",vnum, obj->name,
      obj->level);
    if (screen)
      bprintf(buffer,buf);
    fprintf(fp,buf);
        }
      }
  }else if (!str_cmp(arg1,"clan")) {
    if (obj->item_type == ITEM_WEAPON)
      {
        if (IS_WEAPON_STAT(obj,WEAPON_CLAN)){
      mprintf(sizeof(buf),buf,"[%d]%30s LVL:%d CLAN ITEM\n\r",vnum, obj->name,
        obj->level);
      if (screen)
        bprintf(buffer,buf);
      fprintf(fp,buf);
    }
      }
  }
  extract_obj(obj);
      }
  if (screen)
    page_to_char(buf_string(buffer),ch);
  free_buf(buffer);

  fclose(fp);
  nFilesOpen--;
  if (!screen)
    send_to_char( "Done writing files...\n\r", ch );
}
*/
void display_objcheck_syntax(CHAR_DATA *ch)
{
  send_to_char("SYNTAX:\n",ch);
  send_to_char("objcheck <command>\n\r",ch);
  send_to_char("    avgdam   - Do Average Damage check on Weapons.\n\r",ch);
  send_to_char("    hitdam   - To Hitdice/Damdice check.\n\r",ch);
  send_to_char("    flags    - Check number of flags on items.\n\r",ch);
  send_to_char("    cost     - Check cost of items.\n\r",ch);
  send_to_char("    hps      - Check Max Hitpoints on items.\n\r",ch);
  send_to_char("    manapts  - Check Max Manapoints on items.\n\r",ch);
  send_to_char("    saves    - Check Saves on an object.\n\r",ch);
  send_to_char("    spells   - Check Spell levels on an object.\n\r",ch);
  send_to_char("    delete   - Check DELETE status on objects.\n\r",ch);
  send_to_char("    style    - Check for material labeled OLDSTYLE on objects.\n\r",ch);
  send_to_char("    none     - Check for NONE/RESERVED spells on objects.\n\r",ch);
  send_to_char("    clan     - Check for clan items that are not claned.\n\r",ch);
  send_to_char("    descr    - Check for No description/Short description.\n\r",ch);
}

int  get_input_object_enum(char *arg1)
{
  if (is_name(arg1,"avgdam")) return(objcheck_avgdam_e);
  if (is_name(arg1,"hitdam")) return(objcheck_hitdam_e);
  if (is_name(arg1,"flags")) return(objcheck_flags_e);
  if (is_name(arg1,"cost")) return(objcheck_cost_e);
  if (is_name(arg1,"hps")) return(objcheck_hps_e);
  if (is_name(arg1,"manapts")) return(objcheck_manapts_e);
  if (is_name(arg1,"saves")) return(objcheck_saves_e);
  if (is_name(arg1,"spells")) return(objcheck_spells_e);
  if (is_name(arg1,"delete")) return(objcheck_delete_e);
  if (is_name(arg1,"style")) return(objcheck_style_e);
  if (is_name(arg1,"none")) return(objcheck_none_e);
  if (is_name(arg1,"clan")) return(objcheck_clan_e);
  if (is_name(arg1,"descr")) return(objcheck_descr_e);
  return(obj_none_e);
}

void do_objcheck( CHAR_DATA *ch, char *argument )
{
  OBJ_INDEX_DATA *pObjIndex;
  int vnum;
  int avgdam=0;
  int totalpts = 0;
  BUFFER *buffer;
  char        arg1 [ MAX_INPUT_LENGTH ];
  sh_int weapon_flags=0;
  AFFECT_DATA *paf;
  /*bool finish = FALSE;*/
  enum objcheck_enum input = obj_none_e;
    
  bool violation = FALSE;
  /* open file */
  int numflags = 0;
  /*  fclose(fpReserve); */
  argument = one_argument( argument, arg1 );

  if (!str_cmp(arg1,"\0")) {
    display_objcheck_syntax(ch);
    return;
  }

  input = get_input_object_enum(arg1);

  buffer = new_buf();
  bprintf(buffer,"\nObject Analysis\n");
  bprintf(buffer,  "---------------\n");

  for (vnum = 0; vnum < 32600; vnum++)
    
    if ((pObjIndex = get_obj_index(vnum)) != NULL)
      {
  switch (input) {
  case objcheck_avgdam_e:
    if (pObjIndex->item_type != ITEM_WEAPON) 
      break;

    if (IS_WEAPON_STAT(pObjIndex,WEAPON_CLAN))
      break;
    
    if (pObjIndex->new_format)
      avgdam = (1 + pObjIndex->value[2]) * pObjIndex->value[1] / 2;
    else
      avgdam = ( pObjIndex->value[1] + pObjIndex->value[2] ) / 2;
    /*      sprintf(buf,"[%d]%30s LVL:%d AVGDAM:%d, NUM: %d, DICE: %d\n",vnum,obj->name,
        obj->level,avgdam, obj->value[1],
        obj->value[2]);
        fprintf( fp, buf );*/
    if (IS_WEAPON_STAT(pObjIndex,WEAPON_TWO_HANDS)) {
      if ((avgdam > pObjIndex->level+2) && (pObjIndex->level < LEVEL_HERO)) {
        
        if (avgdam > 7 || pObjIndex->level > 5 ) {
    bprintf(buffer,"[%d]%30s LVL:%d AVGDAM:%d\n",vnum,pObjIndex->name,
      pObjIndex->level,avgdam);
        }
      }
    } else {
      if ((avgdam > pObjIndex->level) && (pObjIndex->level < LEVEL_HERO)) {

        if (avgdam > 5 || pObjIndex->level > 5 ) {
    bprintf(buffer,"[%d]%30s LVL:%d AVGDAM:%d\n",vnum,pObjIndex->name,
      pObjIndex->level,avgdam);
        }
      }
    }
    break;
  case objcheck_hitdam_e:
    if (pObjIndex->item_type == ITEM_WEAPON) {
      if (IS_WEAPON_STAT(pObjIndex,WEAPON_CLAN))
        break;
    }
    for ( paf = pObjIndex->affected; paf != NULL; paf = paf->next )
      {
        if (paf->location == APPLY_HITROLL) {
    if (pObjIndex->level <=20) {
      if (paf->modifier > 2) {
        bprintf(buffer,"[%d]%30s LVL:%d HITROLL %d\n",vnum,pObjIndex->name,
          pObjIndex->level,paf->modifier);
      }
    } else {
      if (pObjIndex->level < ((paf->modifier*10)+1)) {
        bprintf(buffer,"[%d]%30s LVL:%d HITROLL %d\n",vnum,pObjIndex->name,
          pObjIndex->level,paf->modifier);
      }
    }
        }else if (paf->location == APPLY_DAMROLL) {
    if (pObjIndex->level <= 10) {
      if (paf->modifier > 1) {
        bprintf(buffer,"[%d]%30s LVL:%d DAMROLL %d\n",vnum,pObjIndex->name,
          pObjIndex->level,paf->modifier);
      }
    } else if (pObjIndex->level <=30) {
      if (paf->modifier > 2){
        bprintf(buffer,"[%d]%30s LVL:%d DAMROLL %d\n",vnum,pObjIndex->name,
          pObjIndex->level,paf->modifier);
      }
    }else {
      if (pObjIndex->level < ((paf->modifier*10)+1)) {
        bprintf(buffer,"[%d]%30s LVL:%d DAMROLL %d\n",vnum,pObjIndex->name,
          pObjIndex->level,paf->modifier);
      }
    }
        } 
      }
    break;
  case objcheck_flags_e:
    if (pObjIndex->item_type != ITEM_WEAPON) 
      break;
    violation = FALSE;
    if (IS_WEAPON_STAT(pObjIndex,WEAPON_CLAN))
      break;
    numflags =0;
    if (pObjIndex->value[4]) {
      weapon_flags = pObjIndex->value[4];
      if (weapon_flags & WEAPON_FLAMING   ) numflags ++;
      if (weapon_flags & WEAPON_FROST   ) numflags ++;
      if (weapon_flags & WEAPON_VAMPIRIC ) numflags ++;
      if (weapon_flags & WEAPON_SHARP   ) numflags ++;
      if (weapon_flags & WEAPON_VORPAL   ) numflags ++;
      if (weapon_flags & WEAPON_SHOCKING ) numflags ++;
      if (weapon_flags & WEAPON_POISON   ) numflags ++;
      if (weapon_flags & WEAPON_MANA_DRAIN ) numflags ++;
      if ((pObjIndex->level / 30) + 2 < numflags)
        violation = TRUE;
      if (pObjIndex->level > 90 && numflags < 6)
        violation = FALSE;
      if (violation) {
        bprintf(buffer,"[%d]%30s LVL:%d NUMFLAGS %d\n",vnum,pObjIndex->name,
          pObjIndex->level,numflags);
        
      }
    }

    break;
  case objcheck_cost_e:
    if (pObjIndex->cost/100 > pObjIndex->level * 2) {
      bprintf(buffer,"[%d]%30s LVL:%d COST(gold) %d\n",vnum,pObjIndex->name,
        pObjIndex->level, pObjIndex->cost/100);
    }
    break;
  case objcheck_hps_e:
    if (pObjIndex->item_type == ITEM_WEAPON) {
      if (IS_WEAPON_STAT(pObjIndex,WEAPON_CLAN))
        continue;
    }
    totalpts = 0;
    for ( paf = pObjIndex->affected; paf != NULL; paf = paf->next )
      {
        if (paf->location == APPLY_HIT) {
    totalpts += paf->modifier;
    if (pObjIndex->level < 6) {
      if (totalpts > 10 + pObjIndex->level) {
        bprintf(buffer,"[%d]%30s LVL:%d HITPTS %d\n",vnum,pObjIndex->name,
          pObjIndex->level,totalpts);
      }
    } else if (pObjIndex->level < 90) {
      if (totalpts > 2*pObjIndex->level) {
        bprintf(buffer,"[%d]%30s LVL:%d HITPTS %d\n",vnum,pObjIndex->name,
          pObjIndex->level, totalpts);
      } else {
        if (totalpts > 3*pObjIndex->level) {
          bprintf(buffer,"[%d]%30s LVL:%d HITPTS %d\n",vnum,pObjIndex->name,
            pObjIndex->level,totalpts);
        }
      
      }
        
    }
        }
      }
    break;
  case objcheck_manapts_e:
    if (pObjIndex->item_type == ITEM_WEAPON) {
      if (IS_WEAPON_STAT(pObjIndex,WEAPON_CLAN))
        continue;
    }
    totalpts = 0;
    for ( paf = pObjIndex->affected; paf != NULL; paf = paf->next )
      {
        if (paf->location == APPLY_MANA) {
    totalpts += paf->modifier;
    if (pObjIndex->level < 6) {
      if ((totalpts > 10 + pObjIndex->level)
          || (pObjIndex->level < 90  && totalpts > 2*pObjIndex->level)
          || totalpts > 3*pObjIndex->level)
        {
          bprintf(buffer,"[%d]%30s LVL:%d MANAPTS %d\n",vnum,pObjIndex->name,
            pObjIndex->level,totalpts);
        }
    }
        }
      }
        
  case objcheck_saves_e:
    for ( paf = pObjIndex->affected; paf != NULL; paf = paf->next )
      {
        if (paf->location == APPLY_SAVES ||
      paf->location == APPLY_SAVING_ROD ||
      paf->location == APPLY_SAVING_PETRI ||
      paf->location == APPLY_SAVING_BREATH ||
      paf->location == APPLY_SAVING_SPELL ) {
    if ((pObjIndex->level/50 + 1) *-1 > paf->modifier) {
      bprintf(buffer,"[%d]%30s LVL:%d SAVES %d\n",vnum,pObjIndex->name,
        pObjIndex->level,paf->modifier);
    }
        }
      }      
    break;
  case objcheck_spells_e:
    if (pObjIndex->item_type == ITEM_POTION ||
        pObjIndex->item_type == ITEM_SCROLL ||
        pObjIndex->item_type == ITEM_WAND)
      {
        if (pObjIndex->value[0] > pObjIndex->level) {
    bprintf(buffer,"[%d]%30s LVL:%d Spell Level %d\n\r",vnum, pObjIndex->name,
      pObjIndex->level,pObjIndex->value[0]);
        }
      }
    break;
  case objcheck_delete_e:
    if (is_name("delete",pObjIndex->name))
      {
        bprintf(buffer,"[%d]%-30s LVL:%d\n\r",vnum, pObjIndex->name,
          pObjIndex->level);
      }
    break;
  case objcheck_style_e:
    if (is_name("style",pObjIndex->material))
      {
        bprintf(buffer,"[%d]%-30s LVL:%d\n\r",vnum, pObjIndex->name,
          pObjIndex->level);
      }
  case objcheck_none_e:
    if (pObjIndex->item_type == ITEM_POTION ||
        pObjIndex->item_type == ITEM_SCROLL ||
        pObjIndex->item_type == ITEM_WAND)
      {
        if ( pObjIndex->value[1] <= 0) {
    bprintf(buffer,"[%d]%30s LVL:%d Spell NONE/RESERVED position 1\n\r",vnum, pObjIndex->name,
      pObjIndex->level);
        }
      }
    break;
  case objcheck_clan_e:
    if (pObjIndex->item_type == ITEM_WEAPON)
      {
        if (IS_WEAPON_STAT(pObjIndex,WEAPON_CLAN)){
    bprintf(buffer,"[%d]%30s LVL:%d CLAN ITEM\n\r",vnum, pObjIndex->name, pObjIndex->level);
        }
      }
    break;
  case objcheck_descr_e:
    if (pObjIndex->short_descr != '\0'
        && pObjIndex->description != '\0')
      break;
    bprintf(buffer,"[%d]%30s LVL:%d CLAN ITEM\n\r",vnum, pObjIndex->name,
      pObjIndex->level);
    break;
  case obj_none_e:
  default:
    {
      display_objcheck_syntax(ch);
      return;
    }
  }
      }
 
  page_to_char(buf_string(buffer),ch);
  free_buf(buffer);
}

void display_old_mobcheck_syntax(CHAR_DATA *ch)
{
  send_to_char("SYNTAX:\n",ch);
  send_to_char("mobcheck <command> [screen]\n\r",ch);
  send_to_char("    style    - Check if a mob is Oldstyle.\n\r",ch);
  send_to_char("    cost     - Check if a mob is too rich.\n\r",ch); 
  send_to_char("    maxcount - Check if a mob has maxcount.\n\r",ch);
  send_to_char("    delete   - Check if a mob is to be deleted.  Can reuse.\n\r",ch);
  send_to_char("    [screen] - optional - after each command display to screen?\n\r",ch);
}

/*
void do_old_mobcheck( CHAR_DATA *ch, char *argument )
{
  MOB_INDEX_DATA *pMobIndex;
  FILE *fp=0;
  bool screen;
  int vnum,nMatch = 0;
  char buf[MAX_STRING_LENGTH];
  int count =0;
  BUFFER *buffer;
  char        arg1 [ MAX_INPUT_LENGTH ], arg2[MIL];
  CHAR_DATA *mob;
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (!str_cmp(arg1,"\0")) {
    display_old_mobcheck_syntax(ch);
    return;
  }
 
  if (!str_cmp(arg2,"screen"))
    screen = TRUE;
  else
    screen = FALSE;
   
  if (!str_cmp(arg1,"cost"))
    fp = fopen("../Txt/mobcost.txt","w");
  else if (!str_cmp(arg1,"style"))
    fp = fopen("../Txt/mobstyle.txt","w");
  else if (!str_cmp(arg1,"maxcount"))
    fp = fopen("../Txt/mobcount.txt","w");
  else if (!str_cmp(arg1,"delete"))
    fp = fopen("../Txt/mobdelete.txt","w");
  else {
    display_old_mobcheck_syntax(ch);
    return;
  }
  
  if (fp == NULL) {
    display_old_mobcheck_syntax(ch);
    return;
  }
  buffer = new_buf();
  if (screen) {
    bprintf(buffer,"\nMobile Analysis -%s-\n", arg1);
    bprintf(buffer,  "---------------\n");
  }
  fprintf(fp,"\nMOBILE Analysis -%s- \n",arg1);
  fprintf(fp,  "---------------\n");
  nMatch = 0;

  for (vnum = 0; nMatch < 32600; vnum++) {
    if (count >= 200)
      nMatch = top_mob_index;
    if ((pMobIndex = get_mob_index(vnum)) != NULL)
      {
        nMatch++;
        mob = create_mobile( pMobIndex);

  if (!str_cmp(arg1,"style")) {
    if (is_name("oldstyle", mob->name)) {
      mprintf(sizeof(buf),buf,"[%d]%-30s\n",vnum,mob->name);
      fprintf( fp, buf );
      count++;
      if (screen)
        bprintf( buffer, buf );
    }
  } else if (!str_cmp(arg1,"cost")) {
    if ((mob->level * 2) < (mob->gold + (mob->silver/100))){
      mprintf(sizeof(buf),buf,"[Vnum:%5d][Lvl:%3d][Cost:%5ld]%-30s\n",vnum,mob->level, (mob->gold + (mob->silver/100)), mob->name);
      fprintf(fp,buf);
      count ++;
      if (screen)
        bprintf(buffer,buf);
    }
  } else if (!str_cmp(arg1,"maxcount")) {
    if (pMobIndex->max_count >= 500){
      mprintf(sizeof(buf),buf,"[%d]%-30s\n",vnum,mob->name);
      fprintf(fp,buf);
      count ++;
      if (screen)
        bprintf(buffer,buf);
    }
  }else if (!str_cmp(arg1,"delete")) {
    if (is_name("delete",mob->name)) {
      mprintf(sizeof(buf),buf,"[%d]%-30s\n",vnum,mob->name);
      fprintf( fp, buf );
      count++;
      if (screen)
        bprintf( buffer, buf );
    }
  } 
  extract_char(mob, TRUE);
      }
  }
  if (screen)
    page_to_char(buf_string(buffer),ch);
  free_buf(buffer);
  fclose(fp);
  nFilesOpen--; 
  if (!screen)
    send_to_char( "Done writing files...\n\r", ch );
}
*/

void display_mobcheck_syntax(CHAR_DATA *ch)
{
  send_to_char("SYNTAX:\n",ch);
  send_to_char("mobcheck <command>\n\r",ch);
  send_to_char("    style    - Check if a mob is Oldstyle.\n\r",ch);
  send_to_char("    cost     - Check if a mob is too rich.\n\r",ch); 
  send_to_char("    maxcount - Check if a mob has maxcount.\n\r",ch);
  send_to_char("    delete   - Check if a mob is to be deleted.  Can reuse.\n\r",ch);
  send_to_char("    hps      - Check if a mob is too easy.\n\r",ch);
  send_to_char("    lines    - Check mob description for minimum lines.\n\r",ch);
  send_to_char("    position - Check if a mob has a valid starting and default position.\n\r", ch);
}

int get_input_mob_enum(char *arg1)
{
  if (is_name(arg1,"style")) return(mobcheck_style_e);
  if (is_name(arg1,"cost")) return(mobcheck_cost_e);
  if (is_name(arg1,"maxcount")) return(mobcheck_maxcount_e);
  if (is_name(arg1,"delete")) return(mobcheck_delete_e);
  if (is_name(arg1,"hps")) return(mobcheck_hps_e);
  if (is_name(arg1,"position")) return(mobcheck_position_e);
  if (is_name(arg1,"lines")) return(mobcheck_lines_e);
  return(mob_none_e);
}

void do_mobcheck( CHAR_DATA *ch, char *argument )
{
  MOB_INDEX_DATA *pMobIndex;
  int vnum= 0;
  int count =0, temphit= 0;
  BUFFER *buffer;
  char        arg1 [ MAX_INPUT_LENGTH ];
  enum mobcheck_enum input = mob_none_e;
  /* open file */
  argument = one_argument( argument, arg1 );

  if (!str_cmp(arg1,"\0")) {
    display_mobcheck_syntax(ch);
    return;
  }
  input = get_input_mob_enum(arg1);

  buffer = new_buf();
  bprintf(buffer,"\nMobile Analysis -%s-\n", arg1);
  bprintf(buffer,  "---------------\n");

  for (vnum = 0; vnum < top_mob_index; vnum++) {
    if (count >= 100)
      vnum = top_mob_index;
    if ((pMobIndex = get_mob_index(vnum))== NULL)
      continue;

    switch (input) {
    case mobcheck_style_e:
      if (is_name("oldstyle", pMobIndex->player_name)) {
  bprintf(buffer,"[%d]%-30s\n",vnum,pMobIndex->player_name);
  count++;
      }
      break;
    case mobcheck_cost_e:
      if ((pMobIndex->level * 2) < ((pMobIndex->wealth/100))){
  if (pMobIndex->pShop == NULL) {
    bprintf(buffer,"[Vnum:%5d][Lvl:%3d][Cost(Wealth/100):%5ld]%-30s\n", vnum,pMobIndex->level, 
      (pMobIndex->wealth), pMobIndex->player_name);
    count ++;
  }
      }
      break;
    case mobcheck_maxcount_e:
      if (pMobIndex->max_count >= 500){
  bprintf(buffer,"[%d]%-30s\n",vnum,pMobIndex->player_name);
  count ++;
      }
      break;
    case mobcheck_lines_e:
      if (count_char_in_string('\n',pMobIndex->description) >= 3)
  break;

      bprintf(buffer, "[%5d] %s (%s)\n\r",
        vnum, pMobIndex->player_name, pMobIndex->short_descr );
      count++;
      break;
    case mobcheck_delete_e:
      if (is_name("delete",pMobIndex->player_name)) {
  bprintf(buffer,"[%d]%-30s\n",vnum,pMobIndex->player_name);
  count++;
      }
      break;
    case mobcheck_hps_e:
      temphit = pMobIndex->hit[DICE_NUMBER] + pMobIndex->hit[DICE_BONUS]; 
      if (temphit < (pMobIndex->level * pMobIndex->level)) {
  bprintf(buffer,"{W[{C%d{W][{cMinHit:{C%d{W]][{cLvl:{C%d{W]{x%-30s\n",vnum,temphit, pMobIndex->level, pMobIndex->player_name);
  count++;
      }
      break;
    case mobcheck_position_e:
      if (pMobIndex->start_pos < 0 || pMobIndex->default_pos < 0) {
        bprintf(buffer, "{W[{C%d{W][{cStart Pos:{C %d{W][{cDefault Pos:{C %d{W]{x%-30s\n",vnum, pMobIndex->start_pos, pMobIndex->default_pos, pMobIndex->player_name);
        count++;
      }
      break;
    case mob_none_e:
    default:
      display_mobcheck_syntax(ch);
      return;
    } 
  }

  /* close file */
  page_to_char(buf_string(buffer),ch);
  free_buf(buffer);
}

void do_xlist( CHAR_DATA *ch, char *argument )
 {
  ROOM_INDEX_DATA  *pRoom;
  RESET_DATA     *pReset;
  AREA_DATA     *pArea;
  char     arg[MIL], arg1[MIL], arg2[MIL];
  char     outbuf[ MSL*8 ];
  bool    found = FALSE;
  bool    fAll = FALSE;
  bool    fMob = FALSE;
  bool    fObj = FALSE;
  int      rvnum;
  int      vnum = 0, high_vnum=0;
  int     items_found = 0;
  char    tbuf[MSL], buf[MSL];

  pArea = ch->in_room->area;

  if ( argument[0] == '\0' )
    fAll = TRUE;
  else
  {
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1);
    argument = one_argument( argument, arg2);

    fMob = !str_cmp( arg, "mob" );
    fObj = !str_cmp( arg, "obj" );
    
    if ( !fMob && !fObj )
    {
      send_to_char( "Syntax: xlist\n\r"
                    "        xlist mob|obj <vnum>\n\r"
                    "        xlist mob|obj <min_vnum> <max_vnum>\n\r", ch );
      return;
    }
    else if ( is_number( arg1 ) )
    {
      vnum = atoi( arg1 );

      if ( !IS_NULLSTR(arg2)
      &&   is_number(arg2) )
      {
        high_vnum = atoi(arg2);
        
        if ( (high_vnum < pArea->min_vnum)
        ||   (high_vnum > pArea->max_vnum) )
        {
          send_to_char( "Upper level range is outside of this area.\n\r", ch );
          return;
        }

        if ( high_vnum < vnum )
        {
          send_to_char( "Upper level cannot be smaller than the lower.\n\r", ch );
          return;
        }
      }

      if ( high_vnum == 0 )
        high_vnum = vnum;  

      if ( (vnum < pArea->min_vnum)
      ||   (vnum > pArea->max_vnum) )
      {
        send_to_char( "Invalid vnum for this area!\n\r", ch );
        return;
      }
    }
  }

  strcpy( outbuf, "{c  VNUM   {wDescription                   {gLocation {YRoom {WWorld{D/{wRoom\n\r" );
  strcat( outbuf, "{D= ===== ============================== ======== ==== =========={x\n\r" );

  for  ( rvnum = pArea->min_vnum; rvnum <= pArea->max_vnum; rvnum++ )
  {
    //No level range entered
    if ( high_vnum == 0 )
      high_vnum = pArea->max_vnum;

    if ( (pRoom = get_room_index( rvnum )) != NULL )
    {
      for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
      {
        MOB_INDEX_DATA  *pMob;
        OBJ_INDEX_DATA  *pObj;

        switch( pReset->command )
        {
          case 'M':
      
          if ( fAll || fMob )
          {
            if ( fMob && ( pReset->arg1 < vnum ) && (vnum != 0 ))
              break;

            pMob = get_mob_index( pReset->arg1 );
            if ( pMob->vnum > high_vnum )
              continue;

            strncpy_color( tbuf, pMob->short_descr, 30, ' ', TRUE );
            mprintf( sizeof(buf),buf, "{rM {c%5d {w%-30s {gin room {Y%5d {D[{W%3d{D/{w%3d {D]{x\n\r",
              pReset->arg1, tbuf, 
              pReset->arg3, pReset->arg2, pReset->arg4 );
            strcat( outbuf, buf );
            found = TRUE;
            items_found++;
          }
          break;

          case 'O':

          if ( fAll || fObj )
          {
            if ( fObj && ( pReset->arg1 < vnum ) && (vnum != 0) )
              break;

            pObj = get_obj_index( pReset->arg1 );
            if ( pObj->vnum > high_vnum )
              continue;

            strncpy_color( tbuf, pObj->short_descr, 30, ' ', TRUE );

            mprintf( sizeof(buf),buf, "{gO {c%5d {w%-30s {gin room {Y%5d {D[{W%3d{D/    {D]{x\n\r",
              pReset->arg1, tbuf, 
              pRoom->vnum, pReset->arg2 );
            strcat( outbuf, buf );
            found = TRUE;
            items_found++;
          }
          break;

           case 'P':
      
          if ( fAll || fObj )
          {
            if ( fObj && ( pReset->arg1 < vnum ) && (vnum != 0) )
              break;

            pObj = get_obj_index( pReset->arg1 );
            if ( pObj->vnum > high_vnum )
              continue;

            strncpy_color( tbuf, pObj->short_descr, 30, ' ', TRUE );

            mprintf(sizeof(buf), buf, "{gO {c%5d {w%-30s {gin obj  {Y%5d {D[{W%3d{D/{w%3d {D]{x\n\r",
              pReset->arg1, tbuf,
              pRoom->vnum, pReset->arg2, pReset->arg4 );
            strcat( outbuf, buf );
            found = TRUE;
            items_found++;
          }
          break;

          case 'G':
          case 'E':

          if ( fAll || fObj )
          {
            if ( fObj && ( pReset->arg1 < vnum ) && (vnum != 0) )
              break;
  
            pObj = get_obj_index( pReset->arg1 );
            if ( pObj->vnum > high_vnum )
              continue;

            strncpy_color( tbuf, pObj->short_descr, 30, ' ', TRUE );

            mprintf( sizeof(buf),buf, "{gO {c%5d {w%-30s {gmob inv {Y%5d {D[{W%3d{D/    {D]{x\n\r",
              pReset->arg1, tbuf,
              pRoom->vnum, pReset->arg2 );
            strcat( outbuf, buf );
            found = TRUE;
            items_found++;
          }
          break;   

          default:
            break; /* ignore all other resets ( D, R ) */
        }
      }
    }
  } 

  if ( items_found > 0 )
  {
    sprintf( buf, "\n\rResets Found: %d\n\r", items_found );
    strcat(outbuf, buf);
  }

  if ( !found )
  {
    send_to_char( "No reset(s) found.\n\r", ch );
    return;
  }

  page_to_char( outbuf, ch );

  return;
}

void do_vflagtime( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
 
  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );
 
  if ( arg[0] == '\0' )
    {
      send_to_char( "VFLAG TIME from whom?", ch );
      return;
    }
 
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }
 
  if ( is_number( arg2 ) ) {
    if ( IS_SET(victim->act, PLR_VIOLENT) )
      {
  victim->vflag_timer = atoi(arg2);
  printf_to_char(ch,"That person has %d ticks left on their flag.\n\r",victim->vflag_timer);

      } else 
  {
    send_to_char("DOH, no Violent flag on them.\n\r",ch);
  }

  }  else
    {
      if ( IS_SET(victim->act, PLR_VIOLENT) )
  {
    printf_to_char(ch,"That person has %d ticks left on their flag.\n\r",victim->vflag_timer);
  }
      else
  {
    send_to_char("DOH, no Violent flag on them.\n\r",ch);
  }
    } 
  return;
}

/*
 * Display mobile location list (only mobiles outside their home area).
 */
void do_zwhere(CHAR_DATA *ch, char *argument )
{
    char  arg1[MAX_INPUT_LENGTH];
    char  arg2[MAX_INPUT_LENGTH];
    char  tmp1[MAX_STRING_LENGTH];
    char  tmp2[MAX_STRING_LENGTH];
    BUFFER  *buffer;
    CHAR_DATA  *vch;
    int    nCount = 0;
    int    nMatch = 0;
    /*    int    nPage = 1;
    bool  fAll = TRUE;
    */
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );
    buffer = new_buf();

    for ( vch = char_list; vch != NULL; vch = vch->next )
    {
  if ( !IS_NPC( vch ) 
       || vch->in_room == NULL 
       || IS_DELETED( vch )
       ||   vch->pIndexData->area == vch->in_room->area
       ||   IS_AFFECTED( vch, AFF_CHARM ) || vch->desc != NULL )
      continue;

  nCount++;

  strncpyft_color( tmp1,
      FIX_STR( vch->short_descr, "(none)", "(null)" ), 25, ' ', TRUE );
  strncpyft_color( tmp2,
      FIX_STR( vch->in_room->name, "(none)", "(null)" ), 24, '\0', TRUE );

  bprintf( buffer, "%3d) [%3d|%5d]%s%s #n[%3d|%5d]%s%s\n\r#n",
      nCount,
      vch->pIndexData->area->vnum,
      vch->pIndexData->vnum,
      AREA_IS_OPEN( vch->pIndexData->area ) ? " " : "*", tmp1,
      vch->in_room->area->vnum,
      vch->in_room->vnum,
      AREA_IS_OPEN( vch->in_room->area ) ? " " : "*", tmp2 );

  if ( ++nMatch >= 100 )
      break;
    }

    if ( nMatch > 0 )
  page_to_char( buf_string( buffer ), ch );
    else 
  send_to_char( "No miszoned mobiles found.\n\r", ch );

    free_buf( buffer );
}


/* for now, no arguments, just list the current area */
void do_exlist (CHAR_DATA *ch, char * argument)
{
  AREA_DATA* pArea;
  ROOM_INDEX_DATA* room;
  int i;
  char buffer[MAX_STRING_LENGTH];
  
  pArea = ch->in_room->area; /* this is the area we want info on */
  for (i = 0; i < MAX_KEY_HASH; i++) /* room index hash table */
    for (room = room_index_hash[i]; room != NULL; room = room->next)
      /* run through all the rooms on the MUD */
  
      {
  checkexits (room, pArea, buffer);
  send_to_char (buffer, ch);
      }
} 

/* show a list of all used AreaVNUMS */

#define COLUMNS     6   /* number of columns */
#define MAX_ROW     ((MAX_SHOW_VNUM / COLUMNS)+1) /* rows */
#define MAX_SHOW_VNUM     321 /* show only 1 - 100*100 */

void do_fvlist (CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  AREA_DATA *pArea;
  BUFFER *buf;
  int lower,vnum;
  int col = 0;
  bool fArea = TRUE;
  bool fInArea;
  bool found = FALSE;
  int j=0,i=0;
  
  one_argument(argument,arg);
 
  if (arg[0] == '\0')
    {
      send_to_char("Syntax:\n\r",ch);
      send_to_char("  fvlist obj\n\r",ch);
      send_to_char("  fvlist mob\n\r",ch);
      send_to_char("  fvlist room\n\r",ch);
      send_to_char("  fvlist area\n\r",ch);
      send_to_char("  fvlist vlist\n\r",ch);
      return;
    }
  j=0;
  if (!str_cmp(arg,"obj"))
    {
      printf_to_char(ch,"{WFree {C%s{W vnum listing for area {C%s{x\n\r",arg,
         ch->in_room->area->name);
      printf_to_char(ch,"{Y=============================================================================={C\n\r");
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) {
  if (get_obj_index(i) == NULL) {
    printf_to_char(ch,"%8d, ",i);
    if ( (++j % 7) == 0 )
      send_to_char("\n\r",ch);
  }
      }
      send_to_char("{x\n\r",ch);
      printf_to_char(ch,"{WTotal: {C%d{x\n\r",j);
      return;
    }

  if (!str_cmp(arg,"mob"))
    { 
      printf_to_char(ch,"{WFree {C%s {Wvnum listing for area {C%s{x\n\r",arg,
         ch->in_room->area->name);
      printf_to_char(ch,"{Y=============================================================================={C\n\r");
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) {
  if (get_mob_index(i) == NULL) {
    printf_to_char(ch,"%8d, ",i);
    if ( (++j % 7) == 0 )
      send_to_char("\n\r",ch);
  }
      }
      send_to_char("{x\n\r",ch);
      printf_to_char(ch,"{WTotal: {C%d{x\n\r",j);
      return;
    }
  if (!str_cmp(arg,"room"))
    { 
      printf_to_char(ch,"{WFree {C%s {Wvnum listing for area {C%s{x\n\r",arg,
         ch->in_room->area->name);
      printf_to_char(ch,"{Y=============================================================================={C\n\r");
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) {
  if (get_room_index(i) == NULL) {
    printf_to_char(ch,"%8d, ",i);
    if ( ++j % 7 == 0 )
      send_to_char("\n\r",ch);
  }
      }
      send_to_char("{x\n\r",ch);
      printf_to_char(ch,"{WTotal: {C%d{x\n\r",j);
      return;
    }

  if (!str_cmp(arg,"area")) {
    buf = new_buf();
    bprintf(buf,"{WFree {C%s {Wvnum listing for the {DLands of {RD{rr{Da{wk{Dn{ro{Rr{x\n\r",arg,
      ch->in_room->area->name);
    bprintf(buf,"{Y=============================================================================={C\n\r");

    for ( lower = vnum = 1; vnum < 32000; )
      {
  if ( !fArea )
    vnum++;

  fInArea = FALSE;
  for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {
      if ( vnum >= pArea->min_vnum && vnum <= pArea->max_vnum )
        {
    if ( !fArea && lower != vnum )
      {
        if ( lower == vnum - 1 )
          bprintf( buf, "%6d         ", vnum - 1 );
        else
          bprintf( buf, "%6d - %-6d", lower, vnum - 1 );

        if ( ++col % 4 == 0 )
          add_buf( buf, "\n\r" );
        else
          add_buf( buf, "   " );
      }
    lower = vnum = pArea->max_vnum + 1;
    found = fInArea = TRUE;
    continue;
        }
    }

  fArea = fInArea;
      }

    if ( col % 4 != 0 )
      add_buf( buf, "\n\r" );

/*
    bprintf( buf,
       "\n\rFirst assigned vnum:  %5d\n\r"
       "Last assigned vnum:   %5d{x\n\r",
       0,
       0 );
*/
    if ( found )
      page_to_char( buf_string( buf ), ch );
    else
      send_to_char( "No unassigned vnums found.\n\r", ch );

    free_buf( buf );
    return;
  }
  if (!str_cmp(arg,"vlist")) {
      int i,j,vnum;
      ROOM_INDEX_DATA *room;
      char buffer[MAX_ROW*100]; /* should be plenty */
      char buf2 [100];
  
      for (i = 0; i < MAX_ROW; i++)
      {
    strcpy (buffer, "");  /* clear the buffer for this row */
    
    for (j = 0; j < COLUMNS; j++) /* for each column */
    {
        vnum = ((j*MAX_ROW) + i); /* find a vnum whih should be there */
        if (vnum < MAX_SHOW_VNUM)
        {
      room = get_room_index (vnum * 100 + 1); /* each zone has to have a XXX01 room */
      mprintf (sizeof(buf2),buf2, "%3d %-8.8s  ", vnum, 
         room ? area_name(room->area) : "-" ); 
      /* something there or unused ? */
      strcat (buffer,buf2);        
        } 
    } /* for columns */
    
    send_to_char (buffer,ch);
    send_to_char ("\n\r",ch);
      } /* for rows */
      return;
  }
  do_function(ch,&do_fvlist,"");
}



/* for every exit in 'room' which leads to or from pArea but NOT both, print it */
/* get the 'short' name of an area (e.g. MIDGAARD, MIRROR etc. */
/* assumes that the filename saved in the AREA_DATA struct is something like midgaard.are */
char * area_name (AREA_DATA *pArea)
{
  static char buffer[64]; /* short filename */
  char  *period;

  if (!pArea)
    bugf("ACK AREA is EQUAL TO NULL\n\r");
  
  strncpy (buffer, pArea->file_name, 64); /* copy the filename */  
  period = strchr (buffer, '.'); /* find the period (midgaard.are) */
  if (period) /* if there was one */
    *period = '\0'; /* terminate the string there (midgaard) */
    
  return buffer;  
}

char * continent_name (AREA_DATA *pArea)
{
  static char buffer[64]; /* short filename */

  if (!pArea) { bugf("ACK AREA is EQUAL TO NULL\n\r"); }

  strncpy(buffer, continent_table[pArea->continent].displayname, 64);

  return buffer;
}

/* depending on status print > or < or <> between the 2 rooms */
void room_pair (ROOM_INDEX_DATA* left, ROOM_INDEX_DATA* right, exit_status ex, char *buffer)
{
  char *sExit;
  char buf[MSL];
  
  switch (ex)
    {
    default:
      sExit = "??"; break; /* invalid usage */
    case exit_from:
      sExit = "< "; break;
    case exit_to:
      sExit = " >"; break;
    case exit_both:
      sExit = "<>"; break;
    }
  
  mprintf (sizeof(buf),buf, "[%5d] %-30s %-3s [%5d] %s <%s>\n\r",
     left->vnum, left->name, 
     sExit,
     right->vnum, right->name,
     right->area->name  );

  strcpy(buffer,buf);
}
void checkexits (ROOM_INDEX_DATA *room, AREA_DATA *pArea, char* buffer)
{
  char buf[MAX_STRING_LENGTH];
  int i;
  EXIT_DATA *exit;
  ROOM_INDEX_DATA *to_room;
  
  strcpy (buffer, "");
  for (i = 0; i < 6; i++)
    {
      if ( IS_SET( room->room_flags, ROOM_SHIP ) )
      {exit = NULL; i = 6;
        mprintf( sizeof(buf), buf, "%d\n\r",room->vnum );
        strcat( buffer, buf );
        if ( room->state >= 0 ) 
        {
          if ( (exit = room->exit[room->state]) != NULL
                && exit->keyword != NULL )
          {
            i = arg_to_dirnum( exit->keyword );
            if ( i < 0 ) {exit = NULL; i = 6; }
          }
        }
      }
      else 
        exit = room->exit[i];
      if (!exit)
      continue;
      else
      to_room = exit->u1.to_room;
    
      if (to_room) {  /* there is something on the other side */

      if ( (room->area == pArea) && (to_room->area != pArea) )
      { /* an exit from our area to another area */
      /* check first if it is a two-way exit */
      
        if ( to_room->exit[opposite_dir[i]] &&
        to_room->exit[opposite_dir[i]]->u1.to_room == room )
          room_pair (room,to_room,exit_both,buf); /* <> */
        else
          room_pair (room,to_room,exit_to,buf); /* > */
          strcat (buffer, buf);        
      }      
      else      
      if ( (room->area != pArea) && (exit->u1.to_room->area == pArea) )
      { /* an exit from another area to our area */

        if  (!
       (to_room->exit[opposite_dir[i]] &&
        to_room->exit[opposite_dir[i]]->u1.to_room == room )
       )
        /* two-way exits are handled in the other if */
      {            
        room_pair (to_room,room,exit_from,buf);
        strcat (buffer, buf);
      }
        
      } /* if room->area */
      }
      if ( IS_SET( room->room_flags, ROOM_SHIP ) ) i = 6; 
    } /* for */
  
}
void display_roomcheck_syntax(CHAR_DATA *ch)
{
  send_to_char("SYNTAX:\n",ch);
  send_to_char("roomcheck <command>\n\r",ch);
  send_to_char("    names   - Check rooms for lack of names.\n\r",ch);
  send_to_char("    white   - Check Room names for white.\n\r",ch);
  send_to_char("    link    - Check Room's for links.\n\r",ch);
  send_to_char("    private - Check Room for Private w/o SAFE.\n\r",ch);
  send_to_char("    descr   - Check Room for BLANK descriptions.\n\r",ch);
  send_to_char("    delete  - Check DELETED status on rooms.\n\r",ch);
  send_to_char("    bleed   - Check room Names for bleed over.\n\r",ch);
  send_to_char("    lines   - Check minimum lines in a room descr.\n\r",ch);
}

int get_input_room_enum(char *arg1)
{
  if (is_name(arg1,"names")) return(roomcheck_names_e);
  if (is_name(arg1,"white")) return(roomcheck_white_e);
  if (is_name(arg1,"bleed")) return(roomcheck_bleed_e);
  if (is_name(arg1,"link")) return(roomcheck_link_e);
  if (is_name(arg1,"private")) return(roomcheck_private_e);
  if (is_name(arg1,"descr")) return(roomcheck_descr_e);
  if (is_name(arg1,"delete")) return(roomcheck_delete_e);
  if (is_name(arg1,"lines")) return(roomcheck_lines_e);
  return (room_none_e);
}

void do_roomcheck( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  char arg1[MIL];
  int first = TRUE;
  OBJ_INDEX_DATA *pObjIndex;
  AREA_DATA *area;
  ROOM_INDEX_DATA *room, *room2;
  enum roomcheck_enum  input = room_none_e;
  bool finish = FALSE;
  int number = 0, max_found = 100, vnum=0, tvnum=0, exit = 0;

  argument = one_argument( argument, arg1 );

  if (!str_cmp(arg1,"\0")) {
    display_roomcheck_syntax(ch);
    return;
  }
  
  input = get_input_room_enum(arg1);
  finish = FALSE;
  buffer = new_buf();
  bprintf(buffer,"{WROOMCHECK RESULTS for -{C%s{W-\n\e",arg1);

  for ( area = area_first; area; area = area->next )
    for ( vnum = area->min_vnum; vnum <= area->max_vnum; vnum++ )
      {
  if (finish)
    continue;

  if ( !( room = get_room_index( vnum ) ) )
    continue;

  switch (input) {
  case roomcheck_names_e:
    strip_color(buf,room->name);
    if ( !strstr( "asadafa",buf ) )
      break;

    ++number; /*count it if we found a match */
 
    bprintf( buffer, "%3d) [%5d] %s (%s)\n\r",
       number, vnum, room->name, area->name );

    if ( number >= max_found )
      finish = TRUE;
    break;
  case roomcheck_bleed_e:
    if ((strstr( room->name,"{x") != NULL) ||
         (strstr(room->name,"{X") != NULL) ||
        (strstr(room->name,"{w") != NULL))
      break;

    ++number; /*count it if we found a match */
 
    bprintf(buffer, "%3d) [%5d] %s (%s)\n\r",
       number, vnum, room->name, area->name );

    if ( number >= max_found )
      finish = TRUE;
    break;
  case roomcheck_white_e:
    if ( strstr( room->name,"{W" ) )
      break;

    ++number; /*count it if we found a match */
 
    bprintf(buffer, "%3d) [%5d] %s (%s)\n\r",
       number, vnum, room->name, area->name );

    if ( number >= max_found )
      finish = TRUE;
    break;
  case roomcheck_private_e:
    if ( !IS_SET(room->room_flags, ROOM_PRIVATE) )
      break;
    if ( IS_SET(room->room_flags, ROOM_SAFE) )
      break;
    ++number; /*count it if we found a match */
 
    bprintf(buffer, "%3d) [%5d] %s (%s)\n\r",
       number, vnum, room->name, area->name );

    if ( number >= max_found )
      finish = TRUE;
    break;
  case roomcheck_delete_e:
    if (!is_name("delete",room->name))
      break;
      
    ++number; /*count it if we found a match */
 
    bprintf( buffer, "%3d) [%5d] %s (%s)\n\r",
       number, vnum, room->name, area->name );

    if ( number >= max_found )
      finish = TRUE;

    break;
  case roomcheck_descr_e:
    if (room->description[0] != '\0') 
      break;

    ++number; /*count it if we found a match */
    bprintf(buffer, "%3d) [%5d] %s (%s)\n\r",
       number, vnum, room->name, area->name );

    if ( number >= max_found )
      finish = TRUE;
    break;
  case roomcheck_lines_e:
    if (count_char_in_string('\n',room->description) >= 3)
      break;

    ++number; /*count it if we found a match */
    bprintf(buffer, "%3d) [%5d] %s (%s)\n\r",
       number, vnum, room->name, area->name );

    if ( number >= max_found )
      finish = TRUE;
    break;
  case roomcheck_link_e:
    if (first) {
      first = FALSE;
      /* Clear the Linked Flag */
      for(tvnum=0; tvnum<=top_vnum_room; tvnum++) 
        {
    if( (room2 = get_room_index(tvnum)) == NULL)
      continue;
    room2->linked = FALSE;
        }

      /* Find all the rooms each room is linked and set the flag */
      for(tvnum=0; tvnum<=top_vnum_room; tvnum++) 
        {
      
    if( (room2 = get_room_index(tvnum)) == NULL)
      continue;
        if( IS_SET( room2->room_flags, ROOM_SHIP ) )
          continue;
    for(exit=0; exit<MAX_EXITS; exit++) {
      if( room2->exit[exit]) {
        room2->linked = TRUE;
        room2->exit[exit]->u1.to_room->linked = TRUE;
      }
    }
        }
      /* Find all the portal endings from the objects */
      for (tvnum = 0; tvnum < top_obj_index; tvnum++)
        if ((pObjIndex = get_obj_index(tvnum)) != NULL)
    {
      if (pObjIndex->item_type != ITEM_PORTAL)
        continue;
      room2 = get_room_index(pObjIndex->value[3]);
      if (room2)
        room2->linked = TRUE;
    }
   
      
      /* Find all the pet storage rooms */
      for(tvnum=0; tvnum<=top_vnum_room; tvnum++) 
        {
      
    if( (room2 = get_room_index(tvnum)) == NULL)
      continue;

    if ( !IS_SET(room2->room_flags, ROOM_PET_SHOP) )
      continue;
    
    if( (room2 = get_room_index(tvnum+1)) == NULL)
      continue;
      
    room2->linked = TRUE;
        }
    }

    if (room->linked)
      break;
    ++number; /*count it if we found a match */
    bprintf( buffer, "%3d) [%5d] %s (%s)\n\r",
       number, vnum, room->name, area->name );

    if ( number >= max_found ) 
      finish = TRUE;
    break;
  case room_none_e:
  default:
    display_roomcheck_syntax(ch);
    return;
  }
      }

  if ( !number )
    send_to_char( "No matching criteria.\n\r", ch );
  else
    page_to_char(buf_string(buffer),ch);
    
  free_buf(buffer);

}
void display_helpcheck_syntax(CHAR_DATA *ch)
{
  send_to_char("SYNTAX:\n",ch);
  send_to_char("helpcheck <command>\n\r",ch);
  send_to_char("    names   - Check help names for NON unique ones.\n\r",ch);
  send_to_char("    switch  - CHANGE (IMP) Helps to uppercase.\n\r",ch);
  send_to_char("    dups    - Check helps for duplicate keywords.\n\r",ch);
  send_to_char("    level   - Check helps for level 0 (should be -1).\n\r",ch);
  send_to_char("    show    - Show first 500 helps for testing.\n\r",ch);
  send_to_char("    shownext- Show next 500 helps for testing.\n\r",ch);
}

void do_helpcheck( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  char *temp_keyword=0;
  char arg1[MIL];
  HELP_DATA *pHelp, *pHelp2;
  int number = 0, max_found = 1000;

  argument = one_argument( argument, arg1 );

  if (!str_cmp(arg1,"\0")) {
    display_helpcheck_syntax(ch);
    return;
  }

  buffer = new_buf();
  for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
      if (!str_cmp(arg1,"names")) {
  for ( pHelp2 = help_first; pHelp2 != NULL; pHelp2 = pHelp2->next )
    {
      if (is_name(pHelp->keyword,pHelp2->keyword) &&
    strcmp(pHelp->keyword,pHelp2->keyword)) {
        ++number; /*count it if we found a match */
        mprintf( sizeof(buf),buf, "%3d) %s (%s)\n\r",
           number,  pHelp->keyword, pHelp2->keyword );
        add_buf( buffer, buf );

        if ( number >= max_found )
    break;
      }
    }
      } else if (!str_cmp(arg1,"switch")) {
  if (!IS_IMPLEMENTOR(ch))
    return;
  free_string(temp_keyword);
  temp_keyword = str_dup(pHelp->keyword, temp_keyword);
  free_string(pHelp->keyword);
  pHelp->keyword = str_dup(to_upper(temp_keyword), pHelp->keyword);
  ++number; /*count it if we found a match */
      } else if (!str_cmp(arg1,"dups")) {
  for ( pHelp2 = help_first; pHelp2; pHelp2 = pHelp2->next )
    {
      if (!strcmp(pHelp->keyword,pHelp2->keyword) && (pHelp->vnum != pHelp2->vnum)) {
        ++number; /*count it if we found a match*/
        mprintf( sizeof(buf),buf, "%3d) %s (%s)\n\r",
           number,  pHelp->keyword, pHelp2->keyword );
        add_buf( buffer, buf );

        if ( number >= max_found )
    break;
      }
    }
      } else if (!str_cmp(arg1,"level")){
  if (pHelp->level != 0)
    continue;
  ++number; /*count it if we found a match*/
  mprintf( sizeof(buf),buf, "%3d)[%3d] (%s)\n\r",
     number,  pHelp->level, pHelp->keyword );
  add_buf( buffer, buf );
  pHelp->level = -1;
  if ( number >= max_found )
    break;
      } else if (!str_cmp(arg1,"show")){
  if (number >= 500)
    break;
  ++number; /*count it if we found a match*/
  bprintf( buffer, "%3d){W[{C%3d{W][{cSyn:{Y%s{W]({C%s{W){x\n\r", 
     number, pHelp->level, (pHelp->synopsis) ? "Y": "{RN", pHelp->keyword);
  if ( number >= max_found )
    break;
      } else if (!str_cmp(arg1,"shownext")){
  ++number; /*count it if we found a match*/
  if (number <=500)
    continue;
  bprintf( buffer, "%3d){W[{C%3d{W][{cSyn:{Y%s{W]({C%s{W){x\n\r", 
     number, pHelp->level, (pHelp->synopsis) ? "Y": "{RN", pHelp->keyword);
  if ( number >= max_found )
    break;
      } else {
  display_helpcheck_syntax(ch);
  return;
      }
      if ( number >= max_found )
  break;
    }
  if ( !number )
    send_to_char( "No matching criteria.\n\r", ch );
  else
    page_to_char(buf_string(buffer),ch);
    
  free_buf(buffer);

}

void do_beep( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;

  if ( argument[0] == '\0' )
  {
      send_to_char( "Beep who?\n\r", ch );
      return; 
  }

  if ( ( victim=get_char_world( ch, argument ) ) == NULL )
  {
      send_to_char( "They are not playing.\n\r", ch );
      return;
  }

  send_to_char( "Beeping them.\n\r", ch );
  act( "{RBE{w$n{REP{x", ch, NULL, victim, TO_VICT );  
  send_to_char( "\7\7\7", victim );
}

void do_proglist(CHAR_DATA *ch, char *argument)
{
  /*  MOB_PROG_DATA *mp;
  int ln;
  */
  send_to_char("Full Mob Programs Listings\n\r",ch);
  /*
  for (mp = mob_prog_list; mp != NULL; mp = mp->next)
    {
      send_to_char(mp->proc_name, ch);
      send_to_char("\n\r{",ch);
      for (ln = 0; mp->line[ln] != NULL; ln++)
  {
    send_to_char("\n\r    ",ch);
    send_to_char(mp->line[ln], ch);
  }
      send_to_char("\n\r}\n\r",ch);
    }
  */
}
