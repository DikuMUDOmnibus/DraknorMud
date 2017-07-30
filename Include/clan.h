/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/

extern char *ltof( long flags );

#define MAX_PLAYER_PER_CLAN 500

struct clan_req_type {
    char *name;
    void (*func_name)(CHAR_DATA *ch, char *argument);
};

#define MAX_CLAN_REQ_FUNCS 7
#define MAX_CLAN_ITEMS 10

/* Clan edit function added 8-27-06 */

struct order_item {
  bool weapon, armor, light;
  struct clan_data *clan;
  int cost, hit, dam, wis, intel, str, dex, con;
  char name[MSL];
  int wgt, ac;
  int cls, flg1, flg2, flg3;
};
#define ORDER_ITEM struct order_item

struct clan_item_type_st {
  char name[MSL];
  int num_per_dia;
  bool dia_mod;
  int vnum;
  int curr_cost, estimated_cost;
  struct order_item object_item;
};	  

struct clan_req_table_st {
  int num_items,num_pending_items,extend_dia;
  struct clan_item_type_st item[MAX_CLAN_ITEMS];
  struct clan_item_type_st pending_item[MAX_CLAN_ITEMS];
};


struct clannie_st {
  char name[MSL];
  int level;
  int diamonds;
  long logon;
  int rank;
  int sex;
  int kills;
  int killed;

};

struct clan_roster_st {
  int actual_num_clannies;
  struct clannie_st clannie[MAX_PLAYER_PER_CLAN];
};


int clan_status[MAX_CLAN][MAX_CLAN];

enum func_code_enum {fc_uninit_e=-1,fc_new_e,fc_name_e,fc_type_e, fc_cls_e, fc_hit_e, fc_dam_e,
		       fc_int_e, fc_str_e, fc_dex_e, fc_con_e,
		       fc_wis_e, fc_ac_e, fc_wgt_e, fc_flg1_e,
		       fc_flg2_e, fc_flg3_e, fc_clr_e, fc_shw_e,
		     fc_odr_e, fc_cnl_e};


extern struct clan_roster_st clan_roster[MAX_CLAN];
extern struct clan_req_table_st clan_req_table[MAX_CLAN];
extern struct clan_req_type clan_req_funcs[];

/*
 * New Clan Functions for Draknor
 */

