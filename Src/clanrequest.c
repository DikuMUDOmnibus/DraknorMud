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
#include <math.h>
#include <sys/uio.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include "merc.h"
#include "tables.h"
#include "clan.h"
#include "lookup.h"

struct clan_req_type clan_req_funcs[] = {
{"list",clan_func_list},
{"purchase",clan_func_purchase},
{"items",clan_func_items},
{"status",clan_func_status},
{"increase",clan_func_increase},
{"cancel",clan_func_cancel},
{"request",clan_func_request},
};

struct clan_req_table_st clan_req_table[MAX_CLAN];
enum func_code_enum func_code_lookup(char *arg1);
void display_req_syntax(CHAR_DATA *ch);
void reset_req_tempitem(struct order_item *temp_item);
int find_request_cost(struct order_item temp_item);
void request_clan_item(struct order_item temp_item, CHAR_DATA *ch);
float calc_remaining_time(int cost);
bool remove_dia_from_clan(int clan, int dia);
void transfer_item_clan(int vnum, int num);

void do_clanrequest(CHAR_DATA *ch, char *argument)
{
  char arg1[MSL];
  int i=0;
  if ((!is_clan(ch) && !IS_SET(ch->act, PLR_LEADER)) ||
      !IS_IMMORTAL(ch)){
    send_to_char("Only a clan leader or Immortal can use this system\n\r",ch);
    return;
  }

  /* The followin functions should be allowed for the requesting */
  /* system.
     status - check the status of your request.
     increase "dia" - pays more to increase research
     cancel - cancels the request and gives you back remaining
     diamonds.
     request - starts a note system to allow you to request items.
  */
  if ( argument[0] != '\0' )
    {
      argument = one_argument(argument,arg1);
      /* Parseing the listing to call the appropriate functions.*/
      for (i=0; i < MAX_CLAN_REQ_FUNCS; i++) {
	if (!str_prefix(arg1, clan_req_funcs[i].name)) {
	  (*clan_req_funcs[i].func_name)(ch, argument);
	  return;
	}
      }
    }
  send_to_char("Sacred  Clan Request system v .01A\n\r",ch);
  send_to_char("Syntax as follows:\n\r",ch);
  send_to_char("clanrequest list\n\r",ch);
  send_to_char(" -- Shows the listing of your requests by number.\n\r",ch);
  send_to_char("clanrequest status #\n\r",ch);
  send_to_char(" -- Shows detailed information about that pending request.\n\r",ch);
  send_to_char("clanrequest items\n\r",ch);
  send_to_char(" -- Shows the items you can purchase\n\r",ch);
  send_to_char("clanrequest increase # proj#\n\r",ch);
  send_to_char(" -- increase project numbers diamonds by #\n\r",ch);
  send_to_char("clanrequest purchase X items\n\r",ch);
  send_to_char(" -- Will purchase X items for your clan.\n\r",ch);
  send_to_char("clanrequest cancel proj#\n\r",ch);
  send_to_char(" -- Cancel the Project number R&D\n\r",ch);
  send_to_char("clanrequest request\n\r",ch);
  send_to_char(" -- Starts the request engine for requesting items\n\r",ch);
  send_to_char("\n\r\n\r",ch);
  send_to_char("Coded by The Mage\n\r",ch);
  return;
}

void clan_func_list(CHAR_DATA *ch, char *argument)
{
/*  int i=0;
  *//* This is the listing function for the items that are in progress
   *//*
  send_to_char("Listing of Items Currently being researched\n\r",ch);
  send_to_char("-----------------------------------------------------------------\n\r",ch);
  for (i=1; i <= clan_req_table[ch->clan].num_pending_items; i++) {
    printf_to_char(ch,"Item Name: %s  Current Cost: %d  Estimated Cost %d Time Rem: %.2f\n\r",
		   clan_req_table[ch->clan].pending_item[i].name,
		   clan_req_table[ch->clan].pending_item[i].curr_cost,
		   clan_req_table[ch->clan].pending_item[i].estimated_cost,
		   calc_remaining_time(clan_req_table[ch->clan].pending_item[i].estimated_cost -
				       clan_req_table[ch->clan].pending_item[i].curr_cost));
  }*/
  return;
}

void clan_func_items(CHAR_DATA *ch, char *argument)
{
/*  int i=0;
    
  send_to_char("Items that can be purchased and cost\n\r",ch);
  send_to_char("-----------------------------------------------------------------\n\r",ch);
  for (i=1; i <= clan_req_table[ch->clan].num_items; i++) {
    if (clan_req_table[ch->clan].item[i].dia_mod){
      printf_to_char(ch,"1 %s can be purchased for %d diamonds\n\r",
		     clan_req_table[ch->clan].item[i].name,
		     clan_req_table[ch->clan].item[i].num_per_dia);
    } else {
  */    /* Note ASSUMES no ITEM costs more than 1 DIAMOND AFTER you have */
      /* R&Ded it*/
    /*  printf_to_char(ch,"%d %s can be purchased for 1 diamonds\n\r",
		     clan_req_table[ch->clan].item[i].num_per_dia,
		     clan_req_table[ch->clan].item[i].name);
    }
  }*/
  return;
}

void clan_func_status(CHAR_DATA *ch, char *argument)
{
/*  int i=0;
  if (!is_number(argument)) {
    send_to_char("Error: Not a valid item number\n\r",ch);
    return;
  }
  i = atoi(argument);
  if (i == 0 || (i > clan_req_table[ch->clan].num_pending_items)) {
    send_to_char("Error: Item number out of range\n\r",ch);
    return;
  }
  printf_to_char(ch,"Item Name: %s  \nCurrent Cost: %d  \nEstimated Cost %d \nTime Rem: %.2f\n\r",
		 clan_req_table[ch->clan].pending_item[i].name,
		 clan_req_table[ch->clan].pending_item[i].curr_cost,
		 clan_req_table[ch->clan].pending_item[i].estimated_cost,
		 calc_remaining_time(clan_req_table[ch->clan].pending_item[i].estimated_cost -
				     clan_req_table[ch->clan].pending_item[i].curr_cost));
  */ return;
}

void clan_func_purchase(CHAR_DATA *ch, char *argument)
{
  int i=0,j=0; // ,k=0;
  char arg1[MSL];
  argument = one_argument(argument,arg1);
  if (!is_number(argument) || !is_number(arg1)) {
    send_to_char("Error: Not a valid item number\n\r",ch);
    return;
  }
  i = atoi(argument);
  j = atoi(arg1);
/*  if (i == 0 || (i > clan_req_table[ch->clan].num_items)) {
    send_to_char("Error: Item number out of range\n\r",ch);
    return;
  }
  k = j * clan_req_table[ch->clan].item[i].num_per_dia;
  if (!remove_dia_from_clan(ch->clan,k)) {
    send_to_char("Error: Not enough diamonds in your clan\n\r",ch);
  return;

  } */
/*  printf_to_char(ch,"You have ordered %d of Item Name: %s  \nCost was: %d  \n\n\r",
		 j,
		 clan_req_table[ch->clan].item[i].name,
		 k);
  transfer_item_clan(clan_req_table[ch->clan].item[i].vnum, j);
  */ return;
}

void clan_func_increase(CHAR_DATA *ch, char *argument)
{
  send_to_char("Increasing the initial Payment by 0 cause NOT Implmented\n\r",ch);
  return;
}

void clan_func_cancel(CHAR_DATA *ch, char *argument)
{
  int i=0;
  if (!is_number(argument)) {
    send_to_char("Error: Not a valid item number\n\r",ch);
    return;
  }
  i = atoi(argument);
/* LOOK   if (i == 0 || (i > clan_req_table[ch->clan].num_pending_items)) {
    send_to_char("Error: Item number out of range\n\r",ch);
    return;
  }
  printf_to_char(ch,"Item Name: %s is now canceled.\n  You clan has wasted %d diamonds on this effort.\n\r",
		 clan_req_table[ch->clan].pending_item[i].name,
		 clan_req_table[ch->clan].pending_item[i].curr_cost);
  return; */
}

void clan_func_request(CHAR_DATA *ch, char *argument)
{
  char arg1[MSL], arg2[MSL];
//  static struct order_item temp_item[MAX_CLAN];
  static bool creating[MAX_CLAN];
  int temp_flg = NO_FLAG;
  ORDER_ITEM *temp_item;
  CLAN_DATA *clan;

  enum func_code_enum func_code=fc_uninit_e;
  
  argument = one_argument(argument,arg1);
  one_argument(argument,arg2);

  func_code = func_code_lookup(arg1);
  if ((fc_new_e != func_code)&&(!creating)) {
    send_to_char("Please create a new temp item\n\r",ch);
    return;
  }

  clan = ch->clan;
  temp_item = ( ORDER_ITEM *)clan->temp_item;

  switch(func_code) {
  case fc_new_e:
    if ( clan->creating )
      {
	send_to_char("One already exists to be created \n\r",ch);
	return;
      }
    reset_req_tempitem(temp_item);
    temp_item->clan = clan;
    clan->creating = TRUE;
    send_to_char("Temporary Item created to be manipulated\n\r",ch);
    break;
  case fc_type_e:
    if (!strcmp(arg2,"armor")){
      temp_item->weapon = FALSE;
      temp_item->light = FALSE;
      temp_item->armor = TRUE;
    }
    else if (!strcmp(arg2,"weapon")) {
      temp_item->armor = FALSE;
      temp_item->weapon = TRUE;
      temp_item->light = FALSE;
    }
    else if (!strcmp(arg2,"light")) {
      temp_item->armor = FALSE;
      temp_item->weapon = FALSE;
      temp_item->light = TRUE;
    }
    else {
      send_to_char("Invalid option for TYPE\n\r",ch);
      return;
    }
    
    break;
  case fc_shw_e:
    {
      printf_to_char(ch,"Temp Item Name: %s\n\r",temp_item->name);
      printf_to_char(ch,"Type = %s\n\r", temp_item->weapon ? "Weapon" :
		     ((temp_item->armor)? "Armor" : ((temp_item->light) ?
						    "Light" :
						    "Unknown")));
      if (temp_item->weapon)
	printf_to_char(ch, "Class = %s\n\r",
                    flag_string( weapon_class, temp_item->cls ) );

      printf_to_char(ch,"HIT = %d, DAM = %d, INT = %d, STR = %d\n\r",
		     temp_item->hit, temp_item->dam, 
             temp_item->intel, temp_item->str );
      printf_to_char(ch,"DEX = %d, CON =%d, WIS = %d\n\r",
		     temp_item->dex, temp_item->con, 
             temp_item->wis);
      if ( temp_item->armor )
	printf_to_char(ch,"AC = %d", temp_item->ac);
      printf_to_char(ch,"WGT = %d\n\r",
		     temp_item->wgt);
      if (temp_item->weapon)
	printf_to_char(ch,"flg1 = %s\nflg2 = %s\nflg3=%s\n\r",
		       flag_string( weapon_type2, temp_item->flg1 ),
		       flag_string( weapon_type2, temp_item->flg2 ),
		       flag_string( weapon_type2, temp_item->flg3 ));
    }
    break;
  case fc_name_e:
    if (argument[0] == '\0') {
      display_req_syntax(ch);
      return;
    }
    strcpy( temp_item->name, argument );
    break;
  case fc_hit_e:
    if (!is_number(arg2)) {
      display_req_syntax(ch);
      return;
    }
    temp_item->hit = atoi(arg2);
    break;      
  case fc_dam_e:
    if (!is_number(arg2)) {
      display_req_syntax(ch);
      return;
    }
    temp_item->dam = atoi(arg2);
    break;      
  case fc_int_e:
    if (!is_number(arg2)) {
      display_req_syntax(ch);
      return;
    }
    temp_item->intel = atoi(arg2);
    break;      
  case fc_str_e:
    if (!is_number(arg2)) {
      display_req_syntax(ch);
      return;
    }
    temp_item->str = atoi(arg2);
    break;      
  case fc_dex_e:
    if (!is_number(arg2)) {
      display_req_syntax(ch);
      return;
    }
    temp_item->dex = atoi(arg2);
    break;      
  case fc_con_e:
    if (!is_number(arg2)) {
      display_req_syntax(ch);
      return;
    }
    temp_item->con = atoi(arg2);
    break;      
  case fc_wis_e:
    if (!is_number(arg2)) {
      display_req_syntax(ch);
      return;
    }
    temp_item->wis = atoi(arg2);
    break;      
  case fc_ac_e:
    if (temp_item->armor) {
      if (!is_number(arg2)) {
	display_req_syntax(ch);
	return;
      }
      temp_item->ac = atoi(arg2);
    } else {
      send_to_char("Error: Cannot set AC on a non-Armor type\n\r",ch);
      return;
    }
      
    break;      
  case fc_wgt_e:
    if (!is_number(arg2)) {
      display_req_syntax(ch);
      return;
    }
    temp_item->wgt = atoi(arg2);
    break;      
  case fc_cls_e:
    if (temp_item->weapon) {
      if ((temp_flg = flag_value(weapon_class,arg2)) == NO_FLAG)
	{
	  send_to_char("No such weapon type\n\r",ch);
	  return;
	}
      temp_item->cls = temp_flg;
    } else if (temp_item->armor) {
      if ((temp_flg = flag_value(wear_flags,arg2)) == NO_FLAG)
	{
	  send_to_char("No such wear type\n\r",ch);
	  return;
	}
      temp_item->cls = temp_flg;
    } else {
      send_to_char("Do not reconize command\n\r",ch);
      return;
    }
    break;
  case fc_flg1_e:
    if (temp_item->weapon) {
      if ((temp_flg = flag_value(weapon_type2, arg2)) == NO_FLAG)
	{
	  send_to_char("No such flag value\n\r",ch);
	  return;
	}
      temp_item->flg1 = temp_flg;
    } else {
      send_to_char("Cannot set flags on a non_weapon\n\r",ch);
      return;
    }
    break;
  case fc_flg2_e:
    if (temp_item->weapon) {
      if ((temp_flg = flag_value(weapon_type2, arg2)) == NO_FLAG)
	{
	  send_to_char("No such flag value\n\r",ch);
	  return;
	}
      temp_item->flg2 = temp_flg;
    } else {
      send_to_char("Cannot set flags on a non_weapon\n\r",ch);
      return;
    }
    break;
  case fc_flg3_e:
    if (temp_item->weapon) {
      if ((temp_flg = flag_value(weapon_type2, arg2)) == NO_FLAG)
	{
	  send_to_char("No such flag value\n\r",ch);
	  return;
	}
      temp_item->flg3 = temp_flg;
    } else {
      send_to_char("Cannot set flags on a non_weapon\n\r",ch);
      return;
    }
    break;
  case fc_clr_e:
    send_to_char("Temp Item reset to nothing\n\r",ch);
    reset_req_tempitem(temp_item);
    temp_item->clan = clan;
    break;
  case fc_odr_e:
    send_to_char("Item now on order.\n\r",ch);
    request_clan_item(*temp_item, ch);
    clan->creating = FALSE;
    break;
  case fc_cnl_e:
    send_to_char("Canceled current temp item\n\r",ch);
    clan->creating = FALSE;
    break;
  default:
    send_to_char("unrecognized option\n\r",ch);
    display_req_syntax(ch);
    return;
    break;
  }
  temp_item->cost = find_request_cost(*temp_item);
  printf_to_char(ch,"Current Item Cost is %d Diamonds\n\r",temp_item->cost);
  return;
}


enum func_code_enum func_code_lookup(char *arg1)
{
  if (!strcmp(arg1,"new"))
    return (fc_new_e);
  if (!strcmp(arg1,"name"))
    return (fc_name_e);
  if (!strcmp(arg1,"type"))
    return (fc_type_e);
  if (!strcmp(arg1,"cls"))
    return (fc_cls_e);
  if (!strcmp(arg1,"hit"))
    return (fc_hit_e);
  if (!strcmp(arg1,"dam"))
    return (fc_dam_e);
  if (!strcmp(arg1,"int"))
    return (fc_int_e);
  if (!strcmp(arg1,"str"))
    return (fc_str_e);
  if (!strcmp(arg1,"dex"))
    return (fc_dex_e);
  if (!strcmp(arg1,"con"))
    return (fc_con_e);
  if (!strcmp(arg1,"wis"))
    return (fc_wis_e);
  if (!strcmp(arg1,"ac"))
    return (fc_ac_e);
  if (!strcmp(arg1,"wgt"))
    return (fc_wgt_e);
  if (!strcmp(arg1,"flg1"))
    return (fc_flg1_e);
  if (!strcmp(arg1,"flg2"))
    return (fc_flg2_e);
  if (!strcmp(arg1,"flg3"))
    return (fc_flg3_e);
  if (!strcmp(arg1,"clr"))
    return (fc_clr_e);
  if (!strcmp(arg1,"shw"))
    return (fc_shw_e);
  if (!strcmp(arg1,"odr"))
    return (fc_odr_e);
  if (!strcmp(arg1,"cnl"))
    return (fc_cnl_e);
  return(fc_uninit_e); /* failed lookup */
}
void display_req_syntax(CHAR_DATA *ch)
{
  send_to_char("Syntax of Function:\n\r",ch);
  send_to_char("  clan request NEW <start a new one>\n\r",ch);
  send_to_char("  clan request NAME weapon/armor/light\n\r",ch);
  send_to_char("  clan request TYPE weapon/armor/light\n\r",ch);
  send_to_char("  clan request CLS <if weapon -> sword etc.>\n\r",ch);
  send_to_char("  clan request HIT <number of +hit>\n\r",ch);
  send_to_char("  clan request DAM <number of +dam>\n\r",ch);
  send_to_char("  clan request INT <number of +hitdam>\n\r",ch);
  send_to_char("  clan request STR <number of +strength>\n\r",ch);
  send_to_char("  clan request DEX <number of +dex>\n\r",ch);
  send_to_char("  clan request CON <number of +constituion>\n\r",ch);
  send_to_char("  clan request WIS <number of +wisdom>\n\r",ch);
  send_to_char("  clan request AC  <number of -AC>\n\r",ch);
  send_to_char("  clan request WGT <weight of object>\n\r",ch);
  send_to_char("  clan request FLG1 <sharp, vorpal, vampiric>\n\r",ch);
  send_to_char("  clan request FLG2 <frost, flaming, shocking,>\n\r",ch);
  send_to_char("  clan request FLG3 <poison, twohands>\n\r",ch);
  send_to_char("  clan request CLR <Clear the item>\n\r",ch);
  send_to_char("  clan request SHW <Show current item being made>\n\r",ch);
  send_to_char("  clan request ODR <Order the R&D of current item>\n\r",ch);
  send_to_char("  clan request CNL <Cancel the temp item>\n\r",ch);
}
void reset_req_tempitem(struct order_item *temp_item)
{
  temp_item->weapon = temp_item->armor = temp_item->light = FALSE;
  temp_item->cost = 0;
  strcpy(temp_item->name,"\0");
  temp_item->hit = temp_item->dam = temp_item->intel = temp_item->str =
    temp_item->dex = temp_item->con = temp_item->wis = 0;
  temp_item->ac = temp_item->wgt = 0;
  temp_item->cls = temp_item->flg1= temp_item->flg2 = temp_item->flg3
    = NO_FLAG;
}

int find_request_cost(struct order_item temp_item)
{
  /* This is the pricing function.  This is where you set prices to
     each and every item they input.*/
  int cost=0;
  /* Base item cost */
  if (temp_item.weapon)
    cost = 450;
  else if (temp_item.armor)
    cost = 650;
  else if (temp_item.light)
    cost = 400;
  /* Hit dam changes */
  cost += (temp_item.hit * 100);
  cost += (temp_item.dam * 100);
  /* Stat changes */
  cost += (temp_item.intel * 150);
  cost += (temp_item.dex * 150);
  cost += (temp_item.con * 150);
  cost += (temp_item.str * 150);
  cost += (temp_item.wis * 150);
  /* Wgt changes */
  /* Class changes */
  /* AC changes */
  /* flg changes */
  if (temp_item.flg1 != NO_FLAG)
    cost +=200;
  if (temp_item.flg2 != NO_FLAG)
    cost +=200;
  if (temp_item.flg3 != NO_FLAG)
    cost +=200;
  return(cost);
}

void request_clan_item(struct order_item temp_item, CHAR_DATA *ch) // LOOK!
{/*
  int num_pending_items = clan_req_table[ch->clan].num_pending_items +1;
  strcpy(clan_req_table[ch->clan].pending_item[num_pending_items].name, temp_item.name);
  clan_req_table[ch->clan].pending_item[num_pending_items].curr_cost = 0;
  clan_req_table[ch->clan].pending_item[num_pending_items].estimated_cost = temp_item.cost;
  memcpy(&clan_req_table[ch->clan].pending_item[num_pending_items].object_item,
	 &temp_item, sizeof(struct order_item));
  clan_req_table[ch->clan].num_pending_items++;
  printf_to_char(ch,"Num days til completion =  %f",
		 calc_remaining_time(temp_item.cost));
  save_clanreq(); */
}

float calc_remaining_time(int cost)
{
  return(cost/60/60);
}

void save_clanreq()
{
  int fd = -1;
  fd = open(CLAN_REQ_FILE,  O_WRONLY | O_TRUNC | O_CREAT | O_SYNC,
	    0666);
  if (fd == -1)
    return;
  
  write(fd, &clan_req_table, sizeof(struct clan_req_table_st) * MAX_CLAN);
  close (fd);
}

void read_clanreq()
{
  int fd = -1;
  fd = open(CLAN_REQ_FILE,  O_RDONLY);
  if (fd == -1)
    return;
  read(fd, &clan_req_table, sizeof(struct clan_req_table_st) * MAX_CLAN);
  close (fd);
}

void update_clanreq()
{
  int x,i;
  for (i=0; i <=actual_num_clans; i++) {
    for (x=1; x <= clan_req_table[i].num_pending_items; x++)
      {
	if (!remove_dia_from_clan(i,10)) {
	  if (number_percent() <25)
	    clan_req_table[i].pending_item[x].estimated_cost +=
	      number_range(1,10);
	  continue;
	}
	clan_req_table[i].pending_item[x].curr_cost += 10;
	if (number_percent() < 2){
	  clan_req_table[i].pending_item[x].estimated_cost += number_range(1,100);
	  clan_req_table[i].extend_dia++;
	}
	if (clan_req_table[i].extend_dia) {
	  if (number_percent() <
	      (2*clan_req_table[i].extend_dia))
	    {
	      /* Failure of an item */
	    }
	}
	if (clan_req_table[i].pending_item[x].estimated_cost <=
	    clan_req_table[i].pending_item[x].curr_cost)
	  {
	    /* Item is now finished */
	  }
      }
  }
}

bool remove_dia_from_clan(int clan, int dia)
{
  if (clan_table[clan].free_dia < dia)
    return(FALSE);
  clan_table[clan].free_dia -= dia;
  clan_table[clan].used_dia += dia;
  return (TRUE);
}

void transfer_item_clan(int vnum, int num)
{
  /* this is where we transfer X amount of the new item to the clan
     store for that clan */
  /* Probably should get the fvlist working for area to see when we
     create the object automatically.  This was we can keep the clan
     items in the proper place.  Now how in the hell do we set X
     number of items per shop??  and not have it reset.. but save the
     items as we go? */
  
}
