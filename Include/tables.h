/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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



struct clan_titles
{
    char *title_of_rank[3];
    int	pk_range;
};

struct flag_type
{
    char *name;
    int64 bit;
    bool settable;
};

struct clan_type
{
    char 	name[MAX_STRING_LENGTH];
    char 	who_name[MAX_STRING_LENGTH];
    char    patron[MAX_STRING_LENGTH];
    sh_int 	hall;
    int		independent; /* true for loners */
    int		total_dia;
    int		used_dia;
    int		free_dia;
    int   recall_room[MAX_CONTINENT];
};

struct position_type
{
    char *name;
    char *short_name;
};

struct sex_type
{
    char *name;
};

struct	bit_type
{
	const	struct	flag_type *	table;
	char *				help;
};
struct size_type
{
    char *name;
};

struct where_pos
{
    int where;
};

struct continent_type
{
    int  cindex;
    char name[MAX_STRING_LENGTH];
    char displayname[MAX_STRING_LENGTH];
    bool playerarea;
};

struct default_title_type
{
    char title[MAX_STRING_LENGTH];
};

/* game tables */
extern	struct	clan_type	clan_table[MAX_CLAN];
extern	const	struct	position_type	position_table[];
extern	const	struct	sex_type	sex_table[];
extern	const	struct	size_type	size_table[];

/* flag tables */
extern	const	struct	flag_type	act_flags[];
extern  const   struct  flag_type       act2_flags[];
extern	const	struct	flag_type	plr_flags[];
extern	const	struct	flag_type	affect_flags[];
extern	const	struct	flag_type	spell_affect_flags[];
extern  const   struct  flag_type       affect2_flags[];
extern	const	struct	flag_type	off_flags[];
extern	const	struct	flag_type	imm_flags[];
extern	const	struct	flag_type	form_flags[];
extern	const	struct	flag_type	part_flags[];
extern	const	struct	flag_type	comm_flags[];
extern	const	struct	flag_type	chan_flags[];
extern	const	struct	flag_type	pen_flags[];
extern	const	struct	flag_type	plr2_flags[];
extern	const	struct	flag_type	extra_flags[];
extern	const	struct	flag_type	wear_flags[];
extern	const	struct	flag_type	weapon_flags[];
extern	const	struct	flag_type	container_flags[];
extern	const	struct	flag_type	portal_flags[];
extern	const	struct	flag_type	room_flags[];
extern	const	struct	flag_type	exit_flags[];
extern 	const	struct  flag_type	mprog_flags[];
extern	const	struct	flag_type	area_flags[];
extern	const	struct	flag_type	sys_area_flags[];
extern	const	struct	flag_type	sector_flags[];
extern	const	struct	flag_type	door_resets[];
extern	const	struct	flag_type	wear_loc_strings[];
extern	const	struct	flag_type	wear_loc_flags[];
extern	const	struct	flag_type	res_flags[];
extern	const	struct	flag_type	imm_flags[];
extern	const	struct	flag_type	spool_flags[];
extern	const	struct	flag_type	vuln_flags[];
extern	const	struct	flag_type	type_flags[];
extern	const	struct	flag_type	apply_flags[];
extern	const	struct	flag_type	ban_flags[];
extern	const	struct	flag_type	sex_flags[];
extern	const	struct	flag_type	furniture_flags[];
extern	const	struct	flag_type	weapon_class[];
extern	const	struct	flag_type	apply_types[];
extern	const	struct	flag_type	weapon_type2[];
extern	const	struct	flag_type	apply_types[];
extern	const	struct	flag_type	size_flags[];
extern	const	struct	flag_type	position_flags[];
extern	const	struct	flag_type	ac_type[];
extern	const	struct	flag_type	spools_flags[];
extern	const	struct	bit_type	bitvector_type[];
extern	const	struct  flag_type	olc_security_flags[];
extern  const   struct  flag_type   clan_flags[];
extern  const   struct  flag_type   rank_flags[];
extern  const   struct  flag_type   nospam_flags[];
extern  const   struct  flag_type   toggle_flags[];
// Add at top with all the other externs
extern  const   struct  where_pos       where_table[];

extern  const   struct  clan_titles     order_clan_rank_table[];
extern  const   struct  clan_titles     light_clan_rank_table[];
extern  const   struct  clan_titles     velg_clan_rank_table[];
extern  const   struct  clan_titles     xanadu_clan_rank_table[];
extern  const   struct  clan_titles     cov_clan_rank_table[];
extern  const   struct  clan_titles     clan_rank_table[];
extern  const   struct  avg_dam_st      avgdamtable[];

extern  const   struct  continent_type     continent_table[];
extern  const   struct  default_title_type default_title_table[];
