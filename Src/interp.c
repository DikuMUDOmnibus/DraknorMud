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
*        ROM 2.4 is copyright 1993-1996 Russ Taylor                        *
*        ROM has been brought to you by the ROM consortium                 *
*            Russ Taylor (rtaylor@efn.org)                                 *
*            Gabrielle Taylor                                              *
*            Brian Moore (zump@rom.org)                                    *
*        By using this code, you have agreed to follow the terms of the    *
*        ROM license, in the file Rom24/doc/rom.license                    *
****************************************************************************/

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
#include "interp.h"
#include "recycle.h"
#include "clan.h"
#include "db.h"

int  social_prefix args( ( char *name ) );
bool check_social  args( ( CHAR_DATA *ch, char *command, char *argument ) );

/*
 * Log-all switch.
 */
bool fLogAll = FALSE;

/*
 * Command table.
 */
struct cmd_type cmd_table [] =
{
/* Common movement commands */
  { "north",      do_north,            POS_STANDING, 0,  LOG_NEVER,  0, FALSE, CMD_MOVE, CLASS_ALL },
  { "east",       do_east,             POS_STANDING, 0,  LOG_NEVER,  0, FALSE, CMD_MOVE, CLASS_ALL },
  { "south",      do_south,            POS_STANDING, 0,  LOG_NEVER,  0, FALSE, CMD_MOVE, CLASS_ALL },
  { "west",       do_west,             POS_STANDING, 0,  LOG_NEVER,  0, FALSE, CMD_MOVE, CLASS_ALL },
  { "up",         do_up,               POS_STANDING, 0,  LOG_NEVER,  0, FALSE, CMD_MOVE, CLASS_ALL },
  { "down",       do_down,             POS_STANDING, 0,  LOG_NEVER,  0, FALSE, CMD_MOVE, CLASS_ALL },

/* Commands that take priority over alphabetical order */
  { "fill",       do_fill,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "flee",       do_flee,             POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "get",        do_get,              POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "hit",        do_kill,             POS_FIGHTING, 0,  LOG_NORMAL, 0, FALSE, CMD_MOVE, CLASS_ALL },
  { "quaff",      do_quaff,            POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },

/* Common commands */
  { "at",         do_at,               POS_DEAD,     L6, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "goto",       do_goto,             POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "wpeace",     do_wpeace,           POS_DEAD,     L2, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "set",        do_set,              POS_DEAD,     L2, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "setpass",    do_setpassword,      POS_DEAD,     OW, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "sockets",    do_sockets,          POS_DEAD,     L2, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "rest",       do_rest,             POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "rename",     do_rename,           POS_DEAD,     L2, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "cast",       do_cast,             POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "kill",       do_kill,             POS_FIGHTING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "buy",        do_buy,              POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "channels",   do_channels,         POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "exits",      do_exits,            POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "exlist",     do_exlist,           POS_RESTING,  L3, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "glance",     do_glance,           POS_STANDING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "group",      do_group,            POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "balance",    do_balance,          POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "guild",      do_guild,            POS_DEAD,     LC, LOG_ALWAYS, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "auction",    do_auction,          POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "inventory",  do_inventory,        POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "identify",   do_identify,         POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "look",       do_look,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "lore",       do_lore,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_KNIGHT|CLASS_WARLOCK|CLASS_BARBARIAN|CLASS_INQUISITOR|CLASS_OCCULTIST|CLASS_WOODSMAN },
  { "clantalk",   do_clantalk,         POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "clrsockets", do_cleansockets,     POS_DEAD,     L2, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "cleanchar",  do_cleanchar,        POS_DEAD,     L2, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "cleanup",    do_cleanup,          POS_DEAD,     L3, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "dice",       do_newdice,          POS_RESTING,  IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "wake",       do_wake,             POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "warcry",     do_warcry,           POS_FIGHTING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_BARBARIAN },
  { "wartalk",    do_wartalk,          POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "political",  do_politic,          POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "practice",   do_practice,         POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "promote",    do_promote,          POS_SLEEPING, LC, LOG_ALWAYS, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "pkable",     do_pkable,           POS_SLEEPING, L3, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "psearch",    do_psearch,          POS_SLEEPING, IM, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "music",      do_music,            POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "order",      do_order,            POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "rest",       do_rest,             POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "push",       do_push,             POS_SLEEPING, IM, LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "drag",       do_drag,             POS_SLEEPING, IM, LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "sit",        do_sit,              POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "sizeup",     do_size,             POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "stand",      do_stand,            POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "tell",       do_tell,             POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "telnet_ga",  do_telnet_ga,        POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "unlock",     do_unlock,           POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "unguild",    do_unguild,          POS_DEAD,     LC, LOG_ALWAYS, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "wield",      do_wear,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "wizhelp",    do_wizhelp,          POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "duh",        do_duh,              POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "hints",      do_hints,            POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "automap",    do_map,              POS_SLEEPING, 10, LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_DRUID },
  { "mapmud",     do_map_mud,          POS_SLEEPING, IM, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_NONE },
  { "clanlist",   do_clanlist,         POS_SLEEPING, L3, LOG_ALWAYS, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "addlag",     do_addlag,           POS_SLEEPING, L3, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "setcps",     do_cps,              POS_SLEEPING, ML, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "finish",     do_show_fixed_buffer,POS_SLEEPING, L1, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "xlist",      do_xlist,            POS_SLEEPING, IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "check",      do_check,            POS_SLEEPING, L6, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "classlist",  do_classlist,        POS_SLEEPING, L3, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "classcps",   do_classcps,         POS_SLEEPING, L3, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "groupcps",   do_groupcps,         POS_SLEEPING, L3, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "roster",     do_roster,           POS_SLEEPING, LC, LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "demand",     do_demand,           POS_STANDING, 1,  LOG_NORMAL, 0, TRUE,  CMD_MOVE, CLASS_ALL },
  { "clear",      do_cls,              POS_DEAD,     1,  LOG_NORMAL, 0, TRUE,  CMD_INFO, CLASS_ALL },
  { "recite",     do_recite,           POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "recall",     do_recall,           POS_FIGHTING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "/",          do_recall,           POS_FIGHTING, 0,  LOG_NORMAL, 0, TRUE,  CMD_MOVE, CLASS_ALL },
  { "transfer",   do_transfer,         POS_DEAD,     L5, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "transfer",   do_bank_transfer,    POS_DEAD,     0,  LOG_ALWAYS, 1, FALSE, CMD_MOVE, CLASS_ALL },

/* Informational commands */
  { "deposit",    do_deposit,          POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "withdraw",   do_withdraw,         POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "affects",    do_affects,          POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "affremove",  do_affremove,        POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "areas",      do_areas,            POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "bug",        do_bug,              POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "worklist",   do_worklist,         POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "tasklist",   do_tasklist,         POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "catchup",    do_catchup,          POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "changes",    do_changes,          POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "commands",   do_commands,         POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "compare",    do_compare,          POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "consider",   do_consider,         POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "count",      do_newcount,         POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "credits",    do_credits,          POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "equipment",  do_equipment,        POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "examine",    do_examine,          POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "freport",    do_freport,          POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "help",       do_help,             POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "helpcheck",  do_helpcheck,        POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "man",        do_help,             POS_DEAD,     0,  LOG_NORMAL, 0, FALSE, CMD_INFO, CLASS_ALL },
  { "idea",       do_idea,             POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "info",       do_groups,           POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "mastered",   do_mastered,         POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "motd",       do_motd,             POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "news",       do_news,             POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "newsfaerie", do_newsfaerie,       POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "read",       do_read,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "new_eq",     do_new_equipment,    POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "new_dump",   do_new_dump,         POS_RESTING,  ML, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "objcheck",   do_objcheck,         POS_RESTING,  IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "mobcheck",   do_mobcheck,         POS_RESTING,  IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "roomcheck",  do_roomcheck,        POS_RESTING,  IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "report",     do_report,           POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "rpnote",     do_rpnote,           POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "rules",      do_rules,            POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "score",      do_score,            POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "scorio",     do_scorio,           POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "levels",     do_level,            POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "stat",       do_stat,             POS_DEAD,     IM, LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "statistics", do_statistics,       POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "experience", do_experience,       POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "stats",      do_statistics,       POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "skills",     do_skills,           POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "socials",    do_socials,          POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "show",       do_show,             POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "statshow",   do_statshow,         POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "showhints",  do_showhints,        POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "showcmdhist",do_show_cmd_hist,    POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "showdenied", do_show_denied_list, POS_RESTING,  ML, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "cmdhist",    do_cmdhist,          POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_IMM,  CLASS_ALL },
  { "synopsis",   do_synopsis,         POS_DEAD,     L3, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "spells",     do_spells,           POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "spellstat",  do_spellstat,        POS_DEAD,     L3, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "skillstat",  do_skillstat,        POS_DEAD,     L3, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "spellup",    do_spell_maintain,   POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "ospellup",   do_ospellup,         POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "wspellup",   do_wspellup,         POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "story",      do_story,            POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "time",       do_time,             POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "date",       do_time,             POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "typo",       do_typo,             POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "client",     do_client,           POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "wear",       do_wear,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "weather",    do_weather,          POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "who",        do_who,              POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "whisper",    do_whisper,          POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "users",      do_who,              POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "scan",       do_scan,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "donate",     do_donate,           POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "payoff",     do_payoff,           POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "finger",     do_finger,           POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "whois",      do_whois,            POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "wizlist",    do_wizlist,          POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "worth",      do_worth,            POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "tnl",        do_experience,       POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },

/* Configuration commands */
  { "alia",       do_alia,             POS_DEAD,     0,  LOG_NORMAL, 0, TRUE,  CMD_INFO, CLASS_ALL },
  { "alias",      do_alias,            POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "autolist",   do_autolist,         POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "autoall",    do_autoall,          POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "autoassist", do_autoassist,       POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "autodonate", do_autodonate,       POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "autoexit",   do_autoexit,         POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "autogold",   do_autogold,         POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "autoloot",   do_autoloot,         POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "autosac",    do_autosac,          POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "autosplit",  do_autosplit,        POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "autohunt",   do_autohunt,         POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "autostat",   do_autostats,        POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "brief",      do_brief,            POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "ansi",       do_colour,           POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "color",      do_colour,           POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "rub",        do_rub,              POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "combine",    do_combine,          POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "compact",    do_compact,          POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "description",do_description,      POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "delet",      do_delet,            POS_DEAD,     0,  LOG_ALWAYS, 0, TRUE,  CMD_INFO, CLASS_ALL },
  { "delete",     do_delete,           POS_STANDING, 0,  LOG_ALWAYS, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "nofollow",   do_nofollow,         POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "noloot",     do_noloot,           POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "nospam",     do_nospam,           POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "nosummon",   do_nosummon,         POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "nogate",     do_nogate,           POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "automove",   do_nodigmove,        POS_DEAD,     IM, LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "nocancel",   do_nocancel,         POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "outfit",     do_outfit,           POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "password",   do_password,         POS_DEAD,     0,  LOG_NEVER,  1, TRUE,  CMD_INFO, CLASS_ALL },
  { "prompt",     do_prompt,           POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "scroll",     do_scroll,           POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "descry",     do_descry,           POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "title",      do_title,            POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "unalias",    do_unalias,          POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "wimpy",      do_wimpy,            POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "compress",   do_compress,         POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },

/* Communication commands */
  { "afk",        do_afk,              POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "answer",     do_answer,           POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "deaf",       do_deaf,             POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "emote",      do_emote,            POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "pmote",      do_pmote,            POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { ".",          do_gossip,           POS_SLEEPING, 0,  LOG_NORMAL, 0, FALSE, CMD_INFO, CLASS_ALL },
  { "gossip",     do_gossip,           POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "newbie",     do_newbie,           POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "newbieh",    do_newbiehint,       POS_SLEEPING, IM, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { ",",          do_emote,            POS_RESTING,  0,  LOG_NORMAL, 0, FALSE, CMD_MOVE, CLASS_ALL },
  { "grats",      do_grats,            POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "gtell",      do_gtell,            POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { ";",          do_gtell,            POS_DEAD,     0,  LOG_NORMAL, 0, FALSE, CMD_INFO, CLASS_ALL },
  { "note",       do_note,             POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "history",    do_history,          POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "irl",        do_irl,              POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "pose",       do_pose,             POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "quest",      do_quest,            POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "question",   do_question,         POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "questlist",  do_questlist,        POS_SLEEPING, IM, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "quote",      do_quote,            POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "ooc",        do_ooc,              POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "alert",      do_alert,            POS_SLEEPING, L2, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "quiet",      do_quiet,            POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "seeall",     do_seeall,           POS_SLEEPING, IM, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "rbreak",     do_reply_break,      POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "rlock",      do_reply_lock,       POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "reply",      do_reply,            POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "replay",     do_replay,           POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "say",        do_say,              POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "forget",     do_forget,           POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "remove",     do_remove,           POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "'",          do_say,              POS_RESTING,  0,  LOG_NORMAL, 0, FALSE, CMD_MOVE, CLASS_ALL },
  { "shout",      do_shout,            POS_RESTING,  LH, LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "unread",     do_unread,           POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "yell",       do_yell,             POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },

/* Object manipulation commands */
  { "brandish",   do_brandish,         POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "close",      do_close,            POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "drink",      do_drink,            POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "drop",       do_drop,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "eat",        do_eat,              POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "envenom",    do_envenom,          POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "give",       do_give,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "heal",       do_heal,             POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL }, 
  { "hold",       do_wear,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "home",       do_home,             POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "decorate",   do_decorate,         POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "list",       do_list,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "lock",       do_lock,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "open",       do_open,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "pick",       do_pick,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_HIGHWAYMAN },
  { "pour",       do_pour,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "put",        do_put,              POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "remember",   do_remember,         POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "remgrp",     do_remgroup,         POS_RESTING,  L3, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "rroster",    do_remove_clannie,   POS_RESTING,  IM, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "sell",       do_sell,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "dual",       do_second,           POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_KNIGHT|CLASS_BARBARIAN|CLASS_HIGHWAYMAN },
  { "take",       do_get,              POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "touch",      do_touch,            POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "turn",       do_turn,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "press",      do_press,            POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "dig",        do_dig,              POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "pull",       do_pull,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "sacrifice",  do_sacrifice,        POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "destroy",    do_destroy,          POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL }, // sacrifice w/o obj-contains check
  { "tap",        do_sacrifice,        POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "cleanse",    do_cleanse,          POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "sacred",     do_sacred,           POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "junk",       do_junk,             POS_RESTING,  0,  LOG_NORMAL, 0, TRUE,  CMD_MOVE, CLASS_ALL },
  { "value",      do_value,            POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "fvlist",     do_fvlist,           POS_RESTING,  IM, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "wear",       do_wear,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "zap",        do_zap,              POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "deathgrip",  do_deathgrip,        POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_BARBARIAN|CLASS_HIGHWAYMAN },
  { "tithe",      do_tithe,            POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_PRIEST },
  { "hunt",       do_track,            POS_STANDING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "familiar",   do_familiar,         POS_SITTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_DRUID },
  { "bind",       do_bind,             POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_MYSTIC },
  { "backstab",   do_backstab,         POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_HIGHWAYMAN },
  { "bail",       do_bail,             POS_STANDING, 20, LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "crusade",    do_crusade,          POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_INQUISITOR },
  { "sharpen",    do_sharpen,          POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_BARBARIAN|CLASS_KNIGHT },
  { "shelter",    do_shelter,          POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_INQUISITOR|CLASS_BARBARIAN|CLASS_KNIGHT },
  { "layhands",   do_layhands,         POS_FIGHTING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_INQUISITOR|CLASS_OCCULTIST },
  { "author",     do_author,           POS_RESTING,  20, LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "research",   do_research,         POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "study",      do_study,            POS_SLEEPING, 0,  LOG_ALWAYS, 1, TRUE,  CMD_MOVE, CLASS_ALL },

/* Combat commands */
  { "dart",       do_dart,             POS_FIGHTING, 10, LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_KNIGHT|CLASS_HIGHWAYMAN|CLASS_BARBARIAN|CLASS_DRUID|CLASS_MYSTIC },
  { "chomp",      do_chomp,            POS_FIGHTING, 1,  LOG_NORMAL, 0, FALSE, CMD_MOVE, CLASS_ALL },
  { "bite",       do_chomp,            POS_FIGHTING, 1,  LOG_NORMAL, 0, FALSE, CMD_MOVE, CLASS_ALL },
  { "whirlwind",  do_whirlwind,        POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_KNIGHT|CLASS_BARBARIAN },
  { "bash",       do_bash,             POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "tail",       do_tail,             POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "shield bash",do_shield_bash,      POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_KNIGHT },
  { "stomp",      do_stomp,            POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "stake",      do_stake,            POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_PRIEST },
  { "circle",     do_circle,           POS_FIGHTING, 0,  LOG_NORMAL, 0, FALSE, CMD_MOVE, CLASS_HIGHWAYMAN|CLASS_MYSTIC },
  { "bs",         do_backstab,         POS_FIGHTING, 0,  LOG_NORMAL, 0, TRUE,  CMD_MOVE, CLASS_HIGHWAYMAN },
  { "beep",       do_beep,             POS_FIGHTING, IM, LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "berserk",    do_berserk,          POS_FIGHTING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "ironwill",   do_ironwill,         POS_FIGHTING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_KNIGHT|CLASS_MYSTIC|CLASS_BARBARIAN },
  { "dirt",       do_dirt,             POS_FIGHTING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_HIGHWAYMAN|CLASS_KNIGHT|CLASS_DRUID|CLASS_BARBARIAN|CLASS_INQUISITOR },
  { "disarm",     do_disarm,           POS_FIGHTING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "shatter",    do_shatter,          POS_FIGHTING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_MYSTIC },
  { "kick",       do_kick,             POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "fkick",      do_flying_kick,      POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_NONE },
  { "murde",      do_murde,            POS_FIGHTING, LC, LOG_NORMAL, 0, TRUE,  CMD_MOVE, CLASS_ALL },
  { "murder",     do_murder,           POS_FIGHTING, LC, LOG_ALWAYS, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "rescue",     do_rescue,           POS_FIGHTING, 0,  LOG_NORMAL, 0, FALSE, CMD_MOVE, CLASS_KNIGHT|CLASS_INQUISITOR },
  { "trip",       do_trip,             POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_KNIGHT|CLASS_HIGHWAYMAN|CLASS_MYSTIC|CLASS_BARBARIAN },
  { "strangle",   do_strangle,         POS_FIGHTING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_MYSTIC },
  { "crosscut",   do_crosscut,         POS_FIGHTING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_KNIGHT },
  { "gore",       do_gore,             POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "nerve",      do_nerve,            POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_MYSTIC },
  { "takedown",   do_takedown,         POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_KNIGHT|CLASS_MYSTIC },
  { "focus",      do_focus,            POS_FIGHTING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_BARBARIAN|CLASS_INQUISITOR },
  { "buckkick",   do_buckkick,         POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_NONE },
  { "style",      do_style,            POS_FIGHTING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_MYSTIC },
  { "flare",      do_flare,            POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_MYSTIC },
  { "throw",      do_throw,            POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALCHEMIST|CLASS_WOODSMAN },
  { "rebrew",     do_rebrew,           POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALCHEMIST },
  { "transcribe", do_transcribe,       POS_FIGHTING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALCHEMIST },
  { "transport",  do_transport,        POS_STANDING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "chameleon",  do_chameleon,        POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALCHEMIST },

/* Mob command interpreter */
  { "mob",        do_mob,              POS_DEAD,     0,  LOG_NEVER,  0, FALSE, CMD_IMM,  CLASS_ALL },
  { "mobkill",    do_areakill,         POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },

/* Miscellaneous commands */
  { "enter",      do_enter,            POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "follow",     do_follow,           POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "gain",       do_gain,             POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "go",         do_enter,            POS_STANDING, 0,  LOG_NORMAL, 0, TRUE,  CMD_MOVE, CLASS_ALL },
  { "chkcheat",   do_check_valid_eq,   POS_SLEEPING, IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "groups",     do_groups,           POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "hide",       do_hide,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "forage",     do_forage,           POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "repair",     do_repair,           POS_RESTING,  IM, LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "bounty",     do_bounty,           POS_RESTING,  LC, LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "play",       do_play,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "game",       do_game,             POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "qui",        do_qui,              POS_DEAD,     0,  LOG_NORMAL, 0, TRUE,  CMD_INFO, CLASS_ALL },
  { "quit",       do_quit,             POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "rent",       do_rent,             POS_DEAD,     0,  LOG_NORMAL, 0, TRUE,  CMD_MOVE, CLASS_ALL },
  { "save",       do_save,             POS_DEAD,     0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "saveskills", do_saveskills,       POS_DEAD,     ML, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "sleep",      do_sleep,            POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "sleeptells", do_sleeptells,       POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "sneak",      do_sneak,            POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "split",      do_split,            POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "steal",      do_steal,            POS_STANDING, 0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_HIGHWAYMAN },
  { "butcher",    do_butcher,          POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_DRUID|CLASS_KNIGHT|CLASS_BARBARIAN|CLASS_INQUISITOR },
  { "scribe",     do_scribe,           POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_CONJURER|CLASS_WARLOCK|CLASS_DRUID|CLASS_ALCHEMIST },
  { "brew",       do_brew,             POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_PRIEST|CLASS_DRUID|CLASS_ALCHEMIST },
  { "craft",      do_craft,            POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "erase",      do_erase,            POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_CONJURER|CLASS_WARLOCK|CLASS_DRUID|CLASS_ALCHEMIST },
  { "empty",      do_empty,            POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_PRIEST|CLASS_DRUID|CLASS_ALCHEMIST },
  { "train",      do_train,            POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "visible",    do_visible,          POS_SLEEPING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "where",      do_where,            POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "tag",        do_tag,              POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "fish",       do_fish,             POS_RESTING,  0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "mount",      do_mount,            POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "dismount",   do_dismount,         POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },
  { "leave",      do_leave,            POS_STANDING, 0,  LOG_NORMAL, 1, TRUE,  CMD_MOVE, CLASS_ALL },

/* Immortal commands */
  { "advance",    do_advance,          POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "copyove",    do_copyove,          POS_DEAD,     ML, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "copyover",   do_copyover,         POS_DEAD,     ML, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "dump",       do_dump,             POS_DEAD,     ML, LOG_ALWAYS, 0, FALSE, CMD_IMM,  CLASS_ALL },
  { "trust",      do_trust,            POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "snoop",      do_snoop,            POS_DEAD,     L2, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "snooplist",  do_snooplist,        POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "saveclass",  do_saveclass,        POS_DEAD,     ML, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "security",   do_security,         POS_DEAD,     IM, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "setskill",   do_skill,            POS_DEAD,     ML, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "setgrp",     do_grp,              POS_DEAD,     ML, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "aschar",     do_aschar,           POS_DEAD,     ML, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "cloak",      do_cloak,            POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "violate",    do_violate,          POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "protect",    do_protect,          POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "allow",      do_allow,            POS_DEAD,     L3, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "ban",        do_ban,              POS_DEAD,     L3, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "deny",       do_deny,             POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "undeny",     do_undeny,           POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "disconnect", do_disconnect,       POS_DEAD,     L3, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "sedit",      do_sedit,            POS_DEAD,     L4, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "flag",       do_flag,             POS_DEAD,     L4, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "freeze",     do_freeze,           POS_DEAD,     L4, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "permban",    do_permban,          POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "reboo",      do_reboo,            POS_DEAD,     L1, LOG_NORMAL, 0, FALSE, CMD_IMM,  CLASS_ALL },
  { "reboot",     do_reboot,           POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "shutdow",    do_shutdow,          POS_DEAD,     L1, LOG_NORMAL, 0, FALSE, CMD_IMM,  CLASS_ALL },
  { "shutdown",   do_shutdown,         POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "wizlock",    do_wizlock,          POS_DEAD,     L2, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "bank",       do_bank,             POS_DEAD,     IM, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "bankupdate", do_bank_update,      POS_SLEEPING, ML, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "force",      do_force,            POS_DEAD,     L7, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "zforce",     do_silent_force,     POS_DEAD,     L3, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "load",       do_load,             POS_DEAD,     L4, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "newlock",    do_newlock,          POS_DEAD,     L4, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "nochannels", do_nochannels,       POS_DEAD,     L5, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "noemote",    do_noemote,          POS_DEAD,     L5, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "noshout",    do_noshout,          POS_DEAD,     L5, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "notell",     do_notell,           POS_DEAD,     L5, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "pecho",      do_pecho,            POS_DEAD,     L4, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "ispell",     do_ispell,           POS_DEAD,     IM, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "purge",      do_purge,            POS_DEAD,     IM, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "restore",    do_restore,          POS_DEAD,     L4, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "fullrestore",do_fullrestore,      POS_DEAD,     L2, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "sla",        do_sla,              POS_DEAD,     L3, LOG_NORMAL, 0, FALSE, CMD_IMM,  CLASS_ALL },
  { "smite",      do_smite,            POS_DEAD,     L5, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "godname",    do_godname,          POS_DEAD,     IM, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "slay",       do_slay,             POS_DEAD,     IM, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "slookup",    do_slookup,          POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "objaff",     do_objaff,           POS_DEAD,     L5, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "poofin",     do_bamfin,           POS_DEAD,     L8, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "poofout",    do_bamfout,          POS_DEAD,     L8, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "gecho",      do_echo,             POS_DEAD,     L4, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "holylight",  do_holylight,        POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "incognito",  do_incognito,        POS_DEAD,     L7, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "invis",      do_invis,            POS_DEAD,     L7, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "log",        do_log,              POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "logsearch",  do_logsearch,        POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "memory",     do_memory,           POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "mhere",      do_mhere,            POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "mwhere",     do_mwhere,           POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "owhere",     do_owhere,           POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "zwhere",     do_zwhere,           POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "rwhere",     do_rwhere,           POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "peace",      do_peace,            POS_DEAD,     L5, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "penalty",    do_penalty,          POS_DEAD,     L7, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "echo",       do_recho,            POS_DEAD,     L6, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "return",     do_return,           POS_DEAD,     L6, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "string",     do_string,           POS_DEAD,     L5, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "switch",     do_switch,           POS_DEAD,     L6, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "wizinvis",   do_invis,            POS_DEAD,     L7, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "vnum",       do_vnum,             POS_DEAD,     L4, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "zecho",      do_zecho,            POS_DEAD,     L4, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "clone",      do_clone,            POS_DEAD,     L5, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "idle",       do_omni,             POS_DEAD,     L5, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "pload",      do_pload,            POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "plock",      do_plock,            POS_DEAD,     L2, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "punish",     do_punish,           POS_DEAD,     L5, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "punload",    do_punload,          POS_DEAD,     L1, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "wiznet",     do_wiznet,           POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "immtalk",    do_immtalk,          POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "immsay",     do_immsay,           POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "prayer",     do_prayer,           POS_DEAD,     1,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "gm",         do_gm,               POS_DEAD,     L4, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "imotd",      do_imotd,            POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { ":",          do_immtalk,          POS_DEAD,     IM, LOG_NORMAL, 0, FALSE, CMD_IMM,  CLASS_ALL },
  { "vflagtime",  do_vflagtime,        POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "smote",      do_smote,            POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "prefi",      do_prefi,            POS_DEAD,     IM, LOG_NORMAL, 0, FALSE, CMD_INFO, CLASS_ALL },
  { "prefix",     do_prefix,           POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "mpdump",     do_mpdump,           POS_DEAD,     IM, LOG_NEVER,  1, FALSE, CMD_IMM,  CLASS_ALL },
  { "mpstat",     do_mpstat,           POS_DEAD,     IM, LOG_NEVER,  1, FALSE, CMD_IMM,  CLASS_ALL },
  { "rlist",      do_rlist,            POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "olist",      do_olist,            POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "mlist",      do_mlist,            POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "mplist",     do_mplist,           POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "olevel",     do_olevel,           POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "mlevel",     do_mlevel,           POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "rsearch",    do_rsearch,          POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "msearch",    do_msearch,          POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "hsearch",    do_hsearch,          POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },

/* OLC commands */
  { "edit",       do_olc,              POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "asave",      do_asave,            POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "alist",      do_alist,            POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "resets",     do_resets,           POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "redit",      do_redit,            POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "medit",      do_medit,            POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "aedit",      do_aedit,            POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "oedit",      do_oedit,            POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "mpedit",     do_mpedit,           POS_DEAD,     L7, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "hedit",      do_hedit,            POS_DEAD,     L6, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "otype",      do_otype,            POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "cedit",      do_cedit,            POS_DEAD,     L7, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "next",       do_next,             POS_DEAD,     IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },

/* End of list */
  { "",           0,                   POS_DEAD,     0,  LOG_NORMAL, 0, FALSE, CMD_INFO, CLASS_ALL }

/* Removed commands
  { "toggle",     do_toggle,           POS_SLEEPING, 0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "clanmode",   do_clanmode,         POS_SLEEPING, LC, LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "cleanros",   do_clean_roster,     POS_DEAD,     L4, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "clanowe",    do_clanowe,          POS_DEAD,     LC, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "diamonds",   do_diamonds,         POS_SLEEPING, LC, LOG_NORMAL, 1, TRUE,  CMD_INFO, CLASS_ALL },
  { "proglist",   do_proglist,         POS_SLEEPING, IM, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "clanname",   do_clanname,         POS_SLEEPING, ML, LOG_ALWAYS, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "clandesc",   do_clandesc,         POS_SLEEPING, ML, LOG_ALWAYS, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "clanhall",   do_clanhall,         POS_SLEEPING, ML, LOG_ALWAYS, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "clanstate",  do_clanstate,        POS_SLEEPING, ML, LOG_ALWAYS, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "clanstatus", do_clanstatus,       POS_SLEEPING, ML, LOG_ALWAYS, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "clantotal",  do_clantotal,        POS_SLEEPING, ML, LOG_ALWAYS, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "clanfree",   do_clanfree,         POS_SLEEPING, ML, LOG_ALWAYS, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "clanused",   do_clanused,         POS_SLEEPING, ML, LOG_ALWAYS, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "clanpatron", do_clanpatron,       POS_SLEEPING, ML, LOG_ALWAYS, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "clanrequest",do_clanrequest,      POS_SLEEPING, L3, LOG_ALWAYS, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "clandeduct", do_clandeduct,       POS_SLEEPING, L4, LOG_ALWAYS, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "addnewclan", do_addnewclan,       POS_SLEEPING, ML, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "confuse",    do_confuse,          POS_RESTING,  0,  LOG_NORMAL, 1, FALSE, CMD_MOVE, CLASS_ALL },
  { "gendenied",  do_gen_denylist,     POS_RESTING,  ML, LOG_NORMAL, 1, FALSE, CMD_INFO  CLASS_ALL },
  { "oldscore",   do_oldscore,         POS_DEAD,     0,  LOG_NORMAL, 1, FALSE, CMD_INFO, CLASS_ALL },
  { "saveclan",   do_saveclan,         POS_DEAD,     ML, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "setroster",  do_reset_roster,     POS_DEAD,     ML, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "as",         do_as,               POS_DEAD,     ML, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "aslist",     do_aslist,           POS_DEAD,     ML, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "secset",     do_secset,           POS_DEAD,     ML, LOG_ALWAYS, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "sextalk",    do_sextalk,          POS_DEAD,     ML, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
  { "sexpolice",  do_sexpolice,        POS_DEAD,     ML, LOG_NORMAL, 1, FALSE, CMD_IMM,  CLASS_ALL },
*/
};




/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'as', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
  char command[MAX_INPUT_LENGTH];
  char logline[MAX_INPUT_LENGTH];
  char temp_str[MAX_INPUT_LENGTH+50];
  int cmd;
  char log_buf[MSL];
  FILE *fp;
  int trust;
  bool found, cFound;
  int as_found=0;
  int i,strplace=0;
  static int nLostObj = 0;
  static int nLostMob = 0;
  int string_count = nAllocString;
  int perm_count = nAllocPerm;
  int pbyte_count = sAllocPerm;
  char cmd_copy[MAX_INPUT_LENGTH] ;
  char buf[MAX_STRING_LENGTH] ;
  int cmd_num;

  /*
   * Strip leading spaces.
   */
  while ( isspace(*argument) )
    argument++;
  if ( argument[0] == '\0' )
    return;

/* Taeloch: limit special color char input to IMMs only */
/* Restricted { codes: U L * ^ & (underline, blink, and lt, dk, all randoms) */
  if ( !IS_IMMORTAL(ch) )
  {
    for (i=0;i<strlen(argument);i++)
    {
      if (argument[i] == '{')
      {
        if ( ( argument[i+1] != 'U' )
        &&   ( argument[i+1] != 'L' )
        &&   ( argument[i+1] != '*' )
        &&   ( argument[i+1] != '^' )
        &&   ( argument[i+1] != '&' )
        &&   ( argument[i+1] != '\0' ) )
        {
          temp_str[strplace++] = argument[i];
          temp_str[strplace++] = argument[++i];
        }
        else
          i++;
      }
      else
        temp_str[strplace++] = argument[i];
    }
    temp_str[strplace] = '\0' ;
    strcpy(argument,temp_str);
  }
/* End color restriction */

  /*
   * No hiding.
   */
// Taeloch: not every command should remove you from hiding; moved to right before spam check
//  remove_affect(ch, TO_AFFECTS, AFF_HIDE);
  if (!IS_IMMORTAL(ch) && !IS_NPC(ch) ){
      if (IS_AFK(ch)
      && strcmp(capitalize(argument), "Afk") )
      {
          REMOVE_BIT( ch->comm_flags, COMM_AFK);
          if (ch->tells > 0)
          {
            printf_to_char( ch, "AFK mode removed. You have %d tell%s waiting (type {greplay{x to view).\n\r",
              ch->tells,
              ( ch->tells == 1 ? "" : "s") );
          }
          else
            printf_to_char( ch, "AFK mode removed.\n\r");

          if ( ch->idle_snapshot )
          {
            ch->idle_time += current_time - ch->idle_snapshot;
            ch->idle_snapshot = 0;
          }
        /*  if (ch->pcdata->permtitle != NULL) {
              set_title(ch, ch->pcdata->permtitle);
              free_string(ch->pcdata->permtitle);
              ch->pcdata->permtitle = str_dup("");
          }*/
      }
  }
  REMOVE_BIT( ch->affected_by, AFF_LINKDEATH);
              
  /* Test error in buffers check command */
  if (!IS_NPC(ch))
    {
      if (ch->pcdata->buffer->state == BUFFER_FREED)
        {
          bugf("SUPER ERROR: Buffer Freed for player %s for command [%s]!!\n\r",ch->name, interp_cmd);
          ch->pcdata->buffer= new_buf();
          for (cmd_num = MAX_COMMAND_HISTORY - 1; cmd_num >= 0; cmd_num--)
            {
              if (ch->cmd_hist[cmd_num].text[0] )
                  bugf("[%d]: %s", cmd_num, ch->cmd_hist[cmd_num].text);
            }
        }
    }


  /*
   * Implement freeze command.
   */
  if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
    {
      send_to_char( "You're totally frozen!\n\r", ch );
      return;
    }
  
  if (IS_NPC(ch) && ch->wait > 0)
      return;
  /* Memory checking wiznet */
  strcpy(cmd_copy, argument) ;

  
  /*
   * Grab the command word.
   * Special parsing so ' can be a command,
   *   also no spaces needed after punctuation.
   */
  strcpy( logline, argument );
  if ( !isalpha(argument[0])
  &&   !isdigit(argument[0])
  &&   (argument[0] != '/') ) // fix recalling typo ("/west" would recall)
  {
    command[0] = argument[0];
    command[1] = '\0';
    argument++;
    while ( isspace(*argument) )
      argument++;
  }
  else
    {
      argument = one_argument( argument, command );
    }

  /*
   * Look for command in command table.
   */
  found = FALSE;
  cFound = FALSE;
  trust = get_trust( ch );
  for ( cmd = 0; cmd_table[cmd].name[0]; cmd++ )
    {
      if ( command[0] == cmd_table[cmd].name[0]
           &&   !str_prefix( command, cmd_table[cmd].name )
           &&   cmd_table[cmd].level <= trust )
        {
          /* Remove class specific commands from listing */
          if (IS_NPC(ch)) {
            found = TRUE;
            break;
          }
          switch(ch->gameclass) {
          case cConjurer:
            if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                IS_SET(cmd_table[cmd].class_flag, CLASS_CONJURER) )
              cFound = TRUE;
            break;
          case  cPriest:
            if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                IS_SET(cmd_table[cmd].class_flag, CLASS_PRIEST) )
              cFound = TRUE;
            break;
          case  cKnight:
            if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                IS_SET(cmd_table[cmd].class_flag, CLASS_KNIGHT) )
              cFound = TRUE;
            break;
          case  cWarlock:
            if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                IS_SET(cmd_table[cmd].class_flag, CLASS_WARLOCK) )
              cFound = TRUE;
            break;
          case  cDruid:
            if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                IS_SET(cmd_table[cmd].class_flag, CLASS_DRUID) )
              cFound = TRUE;
            break;
          case  cBarbarian:
            if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                IS_SET(cmd_table[cmd].class_flag, CLASS_BARBARIAN) )
              cFound = TRUE;
            break;
          case  cMystic:
            if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                IS_SET(cmd_table[cmd].class_flag, CLASS_MYSTIC) )
              cFound = TRUE;
            break;
          case  cHighwayman:
            if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                IS_SET(cmd_table[cmd].class_flag, CLASS_HIGHWAYMAN) )
              cFound = TRUE;
            break;
          case cInquisitor:
            if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                IS_SET(cmd_table[cmd].class_flag, CLASS_INQUISITOR) )
              cFound = TRUE;
            break;
          case cOccultist:
            if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                IS_SET(cmd_table[cmd].class_flag, CLASS_OCCULTIST) )
              cFound = TRUE;
            break;
          case cAlchemist:
            if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                IS_SET(cmd_table[cmd].class_flag, CLASS_ALCHEMIST) )
              cFound = TRUE;
            break;
          case cWoodsman:
            if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                IS_SET(cmd_table[cmd].class_flag, CLASS_WOODSMAN) )
              cFound = TRUE;
            break;
          default:
            bugf("In command search..  Invalid Class in Command.Gameclass = %d",ch->gameclass);
            break;
          }
          if (cFound || IS_IMMORTAL(ch)) {
            found = TRUE;
            break;
          }
        }
    }

/* if command causes movement, remove AFF_HIDING flag */
  if ( found && ( cmd_table[cmd].cmd_style == CMD_MOVE ) ) { remove_affect(ch, TO_AFFECTS, AFF_HIDE); }

  /* Remove the repeat settings for commands that equate to a FALSE boolean
   *  value under spam_protection
   */
  if ( found && !cmd_table[cmd].spam_protection && ch->desc )
        ch->desc->repeat = 0;

  /*
   * Log and snoop.
   */
  if ( cmd_table[cmd].log == LOG_NEVER )
    strcpy( logline, "" );

  if ( ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
       ||   fLogAll
       ||   cmd_table[cmd].log == LOG_ALWAYS )
  && ( logline[0] != '\0') )
    {
      smash_dollar(logline);
      if (!strcmp(cmd_table[cmd].name,"delete") ||
          !strcmp(cmd_table[cmd].name,"delet"))
        mprintf( sizeof(log_buf), log_buf, "Log %s: Delete <PASSWORD>{x", ch->name );
      else
        mprintf( sizeof(log_buf), log_buf, "Log %s: %s{x", ch->name, logline );
      wiznet( log_buf, NULL, NULL,WIZ_SECURE, 0, get_trust( ch ) );
      log_string( log_buf );
    }
  /* Autosnoop */
  if ( ch->desc ) {
    as_found = 0;
    for (i=0; i < MAX_NUM_AUTOSNOOP; i++) {
      if (!strcmp(as_string[i],ch->name)) {
        as_found = i;
      }
    }
    if (as_found) {
      mprintf(sizeof(temp_str), temp_str,"%s/%s.txt",AS_DIR_STRING,as_string[as_found]);
      if ( ( fp = fopen( temp_str, "a" ) ) )
      {
            nFilesOpen++;
            fprintf(fp,"\n");
            fwrite(logline, strlen(logline), 1, fp);
            fprintf(fp,"\n\r");
            fclose(fp);
            nFilesOpen--;
      }
    }
  }

  if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {

      if (!strcmp(cmd_table[cmd].name,"delete") ||
          !strcmp(cmd_table[cmd].name,"delet"))
        strcpy(logline,"delete");

      write_to_buffer( ch->desc->snoop_by, "% ",    2 );
      write_to_buffer( ch->desc->snoop_by, logline, 0 );
      write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

  if ( !found )
    {
      /*
       * Look for command in socials table.
       */
    
    if ( !check_social( ch, command, argument ) )
    {
    int dnum;  
            dnum = number_range (1,12);
            switch (dnum)
        {
                        case 1:
                send_to_char( "Come again?\n\r", ch );
                break;
                case 2:
            send_to_char( "Pardon?\n\r", ch );
            break;
                case 3:
            send_to_char( "Typos! Typos! Typos!\n\r", ch );
            break;
                case 4:
            send_to_char( "Garbledygoops...\n\r", ch );
            break;
                case 5:
            send_to_char( "As you can now see, typing doesn't work so well with your toes!\n\r", ch );
            break;
                case 6:
            send_to_char( "The small rom-bug loved that one so much he died laughing! Way to go...\n\r", ch );
            break;
                case 7:
            send_to_char( "Didn't we tell you typing was a prerequisite to mudding!?\n\r", ch );
            break;
                case 8:
            send_to_char( "If you are messing up on purpose to read these...just know we do too!\n\r", ch );
            break;
                case 9:
            send_to_char( "If I got a nickel for every typo, I would mud ALL the time.\n\r", ch );
            break;
                case 10:
            send_to_char( "Now that is a wonderful typo!\n\r", ch );
            break;
                case 11:
            send_to_char( "That homerun almost made it to the typo hall of fame!\n\r", ch );
            break;
                default:
            send_to_char( "Huh!?\n\r", ch );
            break;
        }
    }
      return;
  }
  /*
   * Character not in position for command?
   */
  if ( ch->position < cmd_table[cmd].position )
    {
      switch( ch->position )
        {
        case POS_DEAD:
          send_to_char( "Lie still; you are {rDEAD{x.\n\r", ch );
          break;

        case POS_MORTAL:
        case POS_INCAP:
          send_to_char( "You are hurt far too bad for that.\n\r", ch );
          break;

        case POS_STUNNED:
          send_to_char( "You are too stunned to do that.\n\r", ch );
          break;

        case POS_SLEEPING:
          send_to_char( "In your dreams, or what?\n\r", ch );
          break;

        case POS_RESTING:
          send_to_char( "Nah... You feel too relaxed...\n\r", ch);
          break;

        case POS_SITTING:
          send_to_char( "Better stand up first.\n\r",ch);
          break;

        case POS_FIGHTING:
          send_to_char( "No way!  You are still fighting!\n\r", ch);
          break;

        }
      return;
    }
  perm_count = nAllocPerm;
  string_count = nAllocString;
  nLostObj = count_obj_loss();
  nLostMob = count_mob_loss();


  /*
   * Dispatch the command.
   */
  
  if ( !IS_NPC( ch ) 
  && (  ch->level < MAX_LEVEL + 1 ) //No need to track mobs or Owners.
  && ( str_prefix( cmd_table[cmd].name, "north" ) )
  && ( str_prefix( cmd_table[cmd].name, "south" ) )
  && ( str_prefix( cmd_table[cmd].name, "east" ) )
  && ( str_prefix( cmd_table[cmd].name, "west" ) )
  && ( str_prefix( cmd_table[cmd].name, "up" ) )
  && ( str_prefix( cmd_table[cmd].name, "down" ) )
  && ( str_prefix( cmd_table[cmd].name, "quit" ) ) // prevent passwords from appearing in cmdhist
  && ( str_prefix( cmd_table[cmd].name, "delete" ) ) )
  {
    mprintf(sizeof(interp_cmd), interp_cmd,
        "{c[%s{c] Comm: [{w%s{c] : [{w%s{c]{x.",
            ch->name, cmd_table[cmd].name, argument);

    add_cmd_hist(interp_cmd);
  }

  /* if 1st arg is "help", do_help(cmd)" */
  if (!strcmp(argument, "help"))
  {
    do_function(ch, &do_help, cmd_table[cmd].name );
    return;
  }

  (*cmd_table[cmd].do_fun) ( ch, argument );
  
  if (string_count < nAllocString)
  {
      mprintf(sizeof(buf), buf,
              "Memcheck : Increase in strings :{R%d{x:Strings:%d %s : %s",
              nAllocString - string_count,nAllocString, ch->name, cmd_copy) ;
      wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,ch->level) ;
  }

  if (perm_count < nAllocPerm)
  {
      mprintf(sizeof(buf), buf,
              "Increase in perms :{R%d{x:Perm:%d:Bytes:%d %s : %s", nAllocPerm -
              perm_count, nAllocPerm, sAllocPerm - pbyte_count,ch->name, cmd_copy) ;
      wiznet(buf, NULL, NULL, WIZ_MEMCHECK, 0,ch->level) ;
  }
 
  if (nLostObj < count_obj_loss()){
    bugf("Increase in OBJECT LOSS: Cmd: %s, Name %s, Interp %s\n\r",
         cmd_copy, ch->name, interp_cmd);

  if (nLostMob < count_mob_loss())
    bugf("Increase in MOB LOSS: Cmd: %s, Name %s, Interp %s\n\r",
         cmd_copy, ch->name, interp_cmd);
  }
#if MEMDEBUG
  memdebug_check(ch,cmd_copy);
#endif
  tail_chain( );
  return;
}

int social_prefix( char *name )
{
  int cmd;

  for ( cmd = 0; cmd < maxSocial; cmd++ )
  {
    if ( !str_prefix( name, social_table[cmd].name ) )
      return cmd;
  }
  return -1;
}

bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int cmd;

  if ( ( cmd = social_prefix( command ) ) == -1 )
    return FALSE;

  if ( !IS_NPC(ch) && IS_SET(ch->chan_flags, CHANNEL_NOEMOTE) )
    {
      send_to_char( "You are anti-social!\n\r", ch );
      return TRUE;
    }

  switch ( ch->position )
    {
    case POS_DEAD:
      send_to_char( "Lie still; you are {rDEAD{x.\n\r", ch );
      return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
      send_to_char( "You are hurt far too bad for that.\n\r", ch );
      return TRUE;

    case POS_STUNNED:
      send_to_char( "You are too stunned to do that.\n\r", ch );
      return TRUE;

    case POS_SLEEPING:
      /*
       * I just know this is the path to a 12 'if' statement.  :(
       * But two players asked for it already!  -- Furey
       */
      if ( !str_cmp( social_table[cmd].name, "snore" ) )
        {
          send_to_char( "Zzzzzzz.\n\r", ch );
          break;
        }
      send_to_char( "In your dreams, or what?\n\r", ch );
      return TRUE;

    }

  one_argument( argument, arg );
  victim = NULL;
  if ( arg[0] == '\0' )
    {
      act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
      act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
  else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
    }
  else if ( victim == ch )
    {
      act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
      act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
    }
  else
    {
      act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
      act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
      act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );

      if ( IS_NPC( victim )
      && ( HAS_TRIGGER( victim, TRIG_SOCIAL) ) )
        mp_social_trigger( victim, ch, social_table[cmd].name );
      else
      { // suppress automatic reactions on MOBPROG trigger
        if ( !IS_NPC(ch) && IS_NPC(victim)
        &&   !IS_AFFECTED(victim, AFF_CHARM)
        &&   IS_AWAKE(victim) 
        &&   victim->desc == NULL)
        {
          switch ( number_bits( 4 ) )
          {
            case 0:
            case 1: case 2: case 3: case 4:
            case 5: case 6: case 7: case 8:
              act( social_table[cmd].others_found, victim, NULL, ch, TO_NOTVICT );
              act( social_table[cmd].char_found,   victim, NULL, ch, TO_CHAR    );
              act( social_table[cmd].vict_found,   victim, NULL, ch, TO_VICT    );
              break;

            case 9: case 10: case 11: case 12:
              act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
              act( "You slap $N.",  victim, NULL, ch, TO_CHAR    );
              act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
              break;
          }
        }
      }
    }
  return TRUE;
}



 /*
  * Return true if an argument is completely numeric.
  */
bool is_number ( char *arg )
{
  if ( *arg == '\0' )
    return FALSE;

  if ( *arg == '+' || *arg == '-' )
    arg++;

  for ( ; *arg; arg++ )
    {
      if ( !isdigit( *arg ) )
        return FALSE;
    }

  return TRUE;
}



 /*
  * Given a string like 14.foo, return 14 and 'foo'
  */
int number_argument( char *argument, char *arg )
{
  char *pdot;
  int number;

  for ( pdot = argument; *pdot; pdot++ )
  {
      if ( *pdot == '.' )
          {
            *pdot = '\0';
            number = atoi( argument );
            *pdot = '.';
            strcpy( arg, pdot+1 );
            return number;
          }
  }
  strcpy( arg, argument );
  return 1;
}

 /* 
  * Given a string like 14*foo, return 14 and 'foo'
 */
int mult_argument(char *argument, char *arg)
{
  char *pdot;
  int number;

  for ( pdot = argument; *pdot; pdot++ )
  {
    if ( *pdot == '*' )
    {
      *pdot = '\0';
      number = atoi( argument );
      *pdot = '*';
      strcpy( arg, pdot+1 );
      return number;
    }

    if ( *pdot == ' ' )
    {
      *pdot = '\0';
      number = atoi( argument );
      *pdot = ' ';
      if (number > 0)
      {
        strcpy( arg, pdot+1 );
        return number;
      }
    }
  }
  strcpy( arg, argument );
  return 1;
}


 /*
  * Pick off one argument from a string and return the rest.
  * Understands quotes.
  */
char *one_argument( char *argument, char *arg_first )
{
  char cEnd;

  while ( isspace( *argument ) )
    argument++;

  cEnd = ' ';
  if ( *argument == '\'' || *argument == '"' )
    cEnd = *argument++;

  while ( *argument )
  {
    if ( *argument == cEnd )
        {
          argument++;
          break;
        }
    *arg_first = LOWER( *argument );
    arg_first++;
    argument++;
  }
  *arg_first = '\0';

  while ( isspace( *argument ) )
    argument++;

  return argument;
}

 /*
  * Contributed by Alander.
  */
void do_commands( CHAR_DATA *ch, char *argument )
{
  int cmd;
  int col;
  char char_level;
  bool cFound = FALSE;
  col = 0;
  
  for ( char_level='a'; char_level <= 'z'; char_level++) {
    for ( cmd = 0; cmd_table[cmd].name[0]; cmd++ )
      {
        cFound = FALSE;
        if ( cmd_table[cmd].level <  LEVEL_HERO
             &&   cmd_table[cmd].level <= get_trust( ch ) 
             &&   cmd_table[cmd].show
             &&   cmd_table[cmd].name[0] == char_level)
        {
            if (IS_NPC(ch))
              cFound = TRUE;
            else
        {
              switch(ch->gameclass)
          {
                case cConjurer:
                        if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                            IS_SET(cmd_table[cmd].class_flag, CLASS_CONJURER) )
                            cFound = TRUE;
                        break;
                case  cPriest:
                        if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                            IS_SET(cmd_table[cmd].class_flag, CLASS_PRIEST) )
                            cFound = TRUE;
                        break;
                case  cKnight:
                        if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                            IS_SET(cmd_table[cmd].class_flag, CLASS_KNIGHT) )
                            cFound = TRUE;
                        break;
                case  cWarlock:
                        if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                            IS_SET(cmd_table[cmd].class_flag, CLASS_WARLOCK) )
                            cFound = TRUE;
                        break;
                case  cDruid:
                        if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                            IS_SET(cmd_table[cmd].class_flag, CLASS_DRUID) )
                            cFound = TRUE;
                        break;
                case  cBarbarian:
                        if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                            IS_SET(cmd_table[cmd].class_flag, CLASS_BARBARIAN) )
                            cFound = TRUE;
                        break;
                case  cMystic:
                        if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                            IS_SET(cmd_table[cmd].class_flag, CLASS_MYSTIC) )
                            cFound = TRUE;
                        break;
                case  cHighwayman:
                        if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                            IS_SET(cmd_table[cmd].class_flag, CLASS_HIGHWAYMAN) )
                            cFound = TRUE;
                        break;
                case cInquisitor:
                        if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                            IS_SET(cmd_table[cmd].class_flag, CLASS_INQUISITOR) )
                            cFound = TRUE;
                        break;
                case cOccultist:
                        if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                            IS_SET(cmd_table[cmd].class_flag, CLASS_OCCULTIST) )
                            cFound = TRUE;
                        break;
                case cAlchemist:
                        if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                            IS_SET(cmd_table[cmd].class_flag, CLASS_ALCHEMIST) )
                            cFound = TRUE;
                        break;
                case cWoodsman:
                        if (IS_SET(cmd_table[cmd].class_flag, CLASS_ALL) ||
                            IS_SET(cmd_table[cmd].class_flag, CLASS_WOODSMAN) )
                            cFound = TRUE;
                        break;
                default:
                        bugf("In command search..  Invalid Class in Command = %d.", ch->gameclass);
                    break;
              }
            }
            if (cFound || IS_IMMORTAL(ch)){
                printf_to_char(ch, "%-12s", cmd_table[cmd].name );
            if ( ++col % 6 == 0 )
                    send_to_char( "\n\r", ch );
            }
          }
      }
  }
  if ( col % 6 )
    send_to_char( "\n\r", ch );
  return;
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
  int cmd;
  int col;
  char arg[MSL];
  char char_level, clevel;
  int level;

  argument = one_argument( argument, arg );
  
  if ( arg[0] )
  {
    if ( is_number( arg ) )
    {
      level = atoi( arg );
      if ( get_trust( ch ) < level )
      {
            send_to_char( "Can't see commands higher than your level.\n\r", ch );
            return;
      }
      col = 0;
      for( clevel = LEVEL_HERO + 1; clevel < level + 1; clevel++ ) 
            for ( char_level='a'; char_level <= 'z'; char_level++ )
        {
              for ( cmd = 0; cmd_table[cmd].name[0]; cmd++ )
              {
                if ( cmd_table[cmd].level >= LEVEL_HERO
                    &&   cmd_table[cmd].level <= get_trust( ch ) 
                    &&   cmd_table[cmd].show
                    &&   cmd_table[cmd].level == clevel
                    &&   cmd_table[cmd].name[0] == char_level )
                    {
                      printf_to_char( ch, "{g[{x%-3d{g] %-12s{x",
                              cmd_table[cmd].level, cmd_table[cmd].name );
                      if ( ++col % 5 == 0 )
                        send_to_char( "\n\r", ch );
                    }
              }
            }
      if ( col % 5 )
            send_to_char( "\n\r", ch );
    }
    else
    {
      send_to_char( "Invalid Argument.\n\r", ch );
      return;
    }
  }
  else
  {
    col = 0;
    for ( char_level='a'; char_level <= 'z'; char_level++ )
    {
      for ( cmd = 0; cmd_table[cmd].name[0]; cmd++ )
          {
            if ( cmd_table[cmd].level >= LEVEL_HERO
            &&   cmd_table[cmd].level <= get_trust( ch ) 
                &&   cmd_table[cmd].show
                &&   cmd_table[cmd].name[0] == char_level)
            {
                  printf_to_char( ch, "{g[{x%-3d{g] %-12s{x",
                          cmd_table[cmd].level, cmd_table[cmd].name );

                  if ( ++col % 5 == 0 )
                    send_to_char( "\n\r", ch );
            }
          }
    }
    if ( col % 5 )
      send_to_char( "\n\r", ch );
  }
  return;
}

/* function to keep argument safe in all commands -- no static strings */
void do_function ( CHAR_DATA *ch, DO_FUN *do_fun, char *argument )
{
  char *command_string, *command_save;
   
  /* copy the string */
  command_string = str_dup( argument, "" );
  command_save   = command_string; // make certain whole string is freed
     
  /* dispatch the command */
  ( *do_fun)( ch, command_string );
     
  /* free the string */
  free_string( command_save );
}
     


#if SORTING_INTERP
void int_sort( struct cmd_type cmd_table[], int loBound, int hiBound)
{
  int loSwap;
  int hiSwap;
  struct cmd_type temp, pivot;
  /*    //string based Quicksort algorithm
        //algorithm source: Data Abstraction and Structures using C++, 
        //by Headington and Riley (1994) */
    
  if(hiBound-loBound == 1)        /*two items to sort*/
    {
      if(strcmp(cmd_table[loBound].name, cmd_table[hiBound].name)>0)
        {
          temp = cmd_table[loBound];
          cmd_table[loBound] = cmd_table[hiBound];
          cmd_table[hiBound] = temp;
        }
      return;
    }
  pivot = cmd_table[(loBound+hiBound)/2]; /*three or more items to sort*/
  cmd_table[(loBound+hiBound)/2] = cmd_table[loBound];
  cmd_table[loBound] =  pivot;
  loSwap  = loBound +1;
  hiSwap = hiBound;
  do{
    while(loSwap <=hiSwap && strcmp(cmd_table[loSwap].name, pivot.name)<=0)
      loSwap++;
    while (strcmp(cmd_table[hiSwap].name, pivot.name)>0)
      hiSwap--;
    if(loSwap < hiSwap)
      {
        temp = cmd_table[loSwap];
        cmd_table[loSwap]= cmd_table[hiSwap];
        cmd_table[hiSwap]= temp;
      }
  } while (loSwap < hiSwap);
  cmd_table[loBound] = cmd_table[hiSwap];
  cmd_table[hiSwap] = pivot;
    
  if(loBound < hiSwap-1)        /*2 or more items in 1st subvec*/
    int_sort(cmd_table, loBound, hiSwap-1);
    
  if(hiSwap+1 < hiBound)        /*2 or more items in 2nd subvec */
    int_sort(cmd_table, hiSwap+1, hiBound);
}

#endif
#if SORTING_SKILLS
void int_sort( struct skill_type askill_table[], int loBound, int hiBound)
{
  int loSwap;
  int hiSwap;
  struct skill_type temp, pivot;
  /*    //string based Quicksort algorithm
        //algorithm source: Data Abstraction and Structures using C++, 
        //by Headington and Riley (1994) */
    
  if(hiBound-loBound == 1)        /*two items to sort*/
    {
      if(strcmp(askill_table[loBound].name, askill_table[hiBound].name)>0)
        {
          temp = askill_table[loBound];
          askill_table[loBound] = askill_table[hiBound];
          askill_table[hiBound] = temp;
        }
      return;
    }
  pivot = askill_table[(loBound+hiBound)/2]; /*three or more items to sort*/
  askill_table[(loBound+hiBound)/2] = askill_table[loBound];
  askill_table[loBound] =  pivot;
  loSwap  = loBound +1;
  hiSwap = hiBound;
  do{
    while(loSwap <=hiSwap && strcmp(askill_table[loSwap].name, pivot.name)<=0)
      loSwap++;
    while (strcmp(askill_table[hiSwap].name, pivot.name)>0)
      hiSwap--;
    if(loSwap < hiSwap)
      {
        temp = askill_table[loSwap];
        askill_table[loSwap]= askill_table[hiSwap];
        askill_table[hiSwap]= temp;
      }
  } while (loSwap < hiSwap);
  askill_table[loBound] = askill_table[hiSwap];
  askill_table[hiSwap] = pivot;
    
  if(loBound < hiSwap-1)        /*2 or more items in 1st subvec*/
    int_sort(askill_table, loBound, hiSwap-1);
    
  if(hiSwap+1 < hiBound)        /*2 or more items in 2nd subvec */
    int_sort(askill_table, hiSwap+1, hiBound);
}

#endif
/*
 * Get a command table entry.
 */
int get_command( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char *arg = argument;
    int  trust = get_trust( ch );
    int  cmd;

    while ( isspace( *arg ) ) arg++;
    if ( arg[0] == '\0' ) return -1;

    if ( !isalpha( arg[0] ) && !isdigit( arg[0] ) )
    {
            command[0] = arg[0];
            command[1] = '\0';
    }
    else
            one_argument( arg, command );

    for ( cmd = 0; cmd_table[cmd].name[0]; cmd++ )
    {
            if ( LOWER( command[0] ) == LOWER( cmd_table[cmd].name[0] )
            &&  !str_prefix( command, cmd_table[cmd].name )
            &&   cmd_table[cmd].level <= trust )
              return cmd;
    }
    return -1;
}

/*
 * Proxy command.
 */
void do_aschar( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA                *vch;
  DESCRIPTOR_DATA        *vd;
  char                arg[MAX_INPUT_LENGTH];
  int                 cmd;
  int                        vl;
  int vtimer = 0;
  bool wasafk = FALSE;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
      send_to_char( "Syntax:  as <name> <command>\n\r", ch );
      return;
    }

  if ( ( vch = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "No such character.\n\r", ch );
      return;
    }

  if ( get_trust( ch ) < get_trust( vch ))// && !IS_TRUSTED( ch, IMPLEMENTOR ) )
  {
      send_to_char( "You failed.\n\r", ch );
      return;
  }

  if ( !vch->in_room )
    {
      send_to_char( "No such location.\n\r", ch );
      return;
    }

  if ( !is_room_owner( ch, vch->in_room ) && ch->in_room != vch->in_room
  &&        room_is_private( vch->in_room ) && !IS_TRUSTED( ch, IMPLEMENTOR ) )
    {
      send_to_char( "That room is private right now.\n\r", ch );
      return;
    }

  /*
   * Certain commands are not going to be allowed, check through the command
   * table as interpret does.
   */
  cmd = get_command( vch, argument );

  /*
   * Do not allow these commands.
   */
  if ( cmd >= 0 )
    {
      if ( cmd_table[cmd].do_fun == do_force
           ||   cmd_table[cmd].do_fun == do_return
           ||   cmd_table[cmd].do_fun == do_switch
           ||   cmd_table[cmd].do_fun == do_delete
           ||   cmd_table[cmd].do_fun == do_mob
           ||   cmd_table[cmd].do_fun == do_quit 
           ||  (cmd_table[cmd].do_fun == do_cmdhist && ch->level < IMPLEMENTOR))
           //||   cmd_table[cmd].do_fun == do_scorio
                  //||   cmd_table[cmd].do_fun == do_statistics )
        {
          send_to_char( "That will not be done.\n\r", ch );
          return;
        }
    }

  if (IS_AFK(vch))
    wasafk = TRUE;
  vtimer = vch->timer;

  vd                = vch->desc;
  vch->desc        = ch->desc;
  ch->desc        = NULL;
  vl                = vch->lines;
  vch->lines        = ch->lines;

  interpret( vch, argument );

  ch->desc        = vch->desc;
  vch->desc        = vd;
  vch->lines        = vl;

  if (wasafk)
    SET_BIT(vch->comm_flags,COMM_AFK);
  vch->timer = vtimer;

}

void add_cmd_hist( char *cmd )
{
  int i;
  for ( i = 0; i < MAX_SAVE_CMDS-1; i++ )
    strcpy( cmd_hist[i].command, cmd_hist[i+1].command );

  strcpy( cmd_hist[MAX_SAVE_CMDS-1].command, cmd );

}

void print_cmd_hist()
{
  int i;
  for ( i = 0; i < MAX_SAVE_CMDS; i++ )
  {
    if ( cmd_hist[i].command[0] )
    {
        //logit("[CMDHIST][%d]: %s",i,cmd_hist[i].command);
    }
  }
}

void do_show_cmd_hist(CHAR_DATA *ch, char *argument)
{
  int i;

  for ( i = 0; i < MAX_SAVE_CMDS; i++ )
    if ( cmd_hist[i].command[0] )
      printf_to_char( ch, "{c[{w%2d{c]:{x %s\n\r", i, cmd_hist[i].command );
}

void do_synopsis(CHAR_DATA *ch, char *argument)
{
  HELP_DATA *pHelp;
  BUFFER *output;
  int count = 0;
  bool found = FALSE;
  char char_level;
  int level ;
  int cmd;

  output = new_buf();
  bprintf(output,"{CMatching Helps Found. For More Information <help #>{x\n\r");
  bprintf(output,"{W==================================================={x\n\r");
  
  for ( char_level='a'; char_level <= 'z'; char_level++) {
    for ( cmd = 0; cmd_table[cmd].name[0]; cmd++ )
      {
        if ( cmd_table[cmd].level >= 100
             &&   cmd_table[cmd].show
             &&   cmd_table[cmd].name[0] == char_level)
          {
            found = FALSE;
            for ( pHelp = help_first; pHelp; pHelp = pHelp->next )
              {
                level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

                if (level > get_trust( ch ) )
                  continue;
        
                if (!pHelp->synopsis)
                  pHelp->synopsis = str_dup("(null)", pHelp->synopsis);

                if ( is_name( cmd_table[cmd].name, pHelp->keyword ) )
                  {
                    if (strcmp(pHelp->synopsis,"(null)"))
                      bprintf(output,"{W[{G%3d{W] {C%s{x\r",pHelp->vnum, pHelp->synopsis);
                    else
                      bprintf(output,"{W[{G%3d{W] {C%s{x\n\r",pHelp->vnum, pHelp->keyword);
                    found = TRUE;
                    count++;
                  }
              }
            if (!found)
              bprintf(output, "ERROR: No help found for: %s.\n\r", cmd_table[cmd].name);
          }
      }
  }
  page_to_char(buf_string(output),ch);
  free_buf(output);
}

