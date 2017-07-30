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
 *    ROM 2.4 is copyright 1993-1996 Russ Taylor                           *
 *    ROM has been brought to you by the ROM consortium                    *
 *        Russ Taylor (rtaylor@efn.org)                                    *
 *        Gabrielle Taylor                                                 *
 *        Brian Moore (zump@rom.org)                                       *
 *    By using this code, you have agreed to follow the terms of the       *
 *    ROM license, in the file Rom24/doc/rom.license                       *
 ***************************************************************************/

/***************************************************************************
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by              *
 *      Chris Litchfield and Mark Archambault                              *
 *      Sacred has been created with much time and effort from many        *
 *      different people's input and ideas.                                *
 *      By using this code, you have agreed to follow the terms of the     *
 *      Sacred license, in the file doc/sacred.license                     *
 ***************************************************************************/

#if !defined(_INTERP_H)
#define _INTERP_H

/* this is a listing of all the commands and command related data */

/* for command types */
#define OW MAX_LEVEL + 1  /* Owner */
#define ML MAX_LEVEL      /* implementor */
#define L1 MAX_LEVEL - 1  /* creator */
#define L2 MAX_LEVEL - 2  /* supreme being */
#define L3 MAX_LEVEL - 3  /* deity */
#define L4 MAX_LEVEL - 4  /* god */
#define L5 MAX_LEVEL - 5  /* immortal */
#define L6 MAX_LEVEL - 6  /* demigod */
#define L7 MAX_LEVEL - 7  /* angel */
#define L8 MAX_LEVEL - 8  /* avatar */
#define IM LEVEL_IMMORTAL /* avatar */
#define HE LEVEL_HERO     /* hero */
#define LC LEVEL_CLAN
#define LH LEVEL_CHANNEL

#define COM_INGORE 1

/* cmd_type.cmd_style shows whether a command causes movement, is informational, etc */
#define CMD_INFO 1
#define CMD_MOVE 2
#define CMD_IMM  3

/* Command logging types */
#define LOG_NORMAL  0
#define LOG_ALWAYS  1
#define LOG_NEVER   2

/*
 * Structure for a command in the command lookup table.
 */
struct cmd_type
{
  char   * name;
  DO_FUN * do_fun;
  sh_int   position;
  sh_int   level;
  sh_int   log;
  sh_int   show;
  bool     spam_protection;
  sh_int   cmd_style;
  long     class_flag;
};

/* the command table itself */
extern struct cmd_type cmd_table[];
#if SORTING_INTERP
void int_sort( struct cmd_type cmd_table[], int loBound, int hiBound);
#endif
#if SORTING_SKILLS
void int_sort( struct skill_type askill_table[], int loBound, int hiBound);
#endif

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN( do_advance          );
DECLARE_DO_FUN( do_sacred           );
DECLARE_DO_FUN( do_affects          );
DECLARE_DO_FUN( do_afk              );
DECLARE_DO_FUN( do_alia             );
DECLARE_DO_FUN( do_alias            );
DECLARE_DO_FUN( do_allow            );
DECLARE_DO_FUN( do_answer           );
DECLARE_DO_FUN( do_areas            );
DECLARE_DO_FUN( do_at               );
DECLARE_DO_FUN( do_auction          );

/* Library Commands */
DECLARE_DO_FUN( do_author           );
DECLARE_DO_FUN( do_research         );

DECLARE_DO_FUN( do_aset             );
DECLARE_DO_FUN( do_autoassist       );
DECLARE_DO_FUN( do_autodonate       );
DECLARE_DO_FUN( do_autoexit         );
DECLARE_DO_FUN( do_autogold         );
DECLARE_DO_FUN( do_autolist         );
DECLARE_DO_FUN( do_autoloot         );
DECLARE_DO_FUN( do_autosac          );
DECLARE_DO_FUN( do_autosplit        );
DECLARE_DO_FUN( do_autohunt         ); 
DECLARE_DO_FUN( do_backstab         );
DECLARE_DO_FUN( do_balance          );
DECLARE_DO_FUN( do_bamfin           );
DECLARE_DO_FUN( do_bamfout          );
DECLARE_DO_FUN( do_ban              );
DECLARE_DO_FUN( do_bash             );
DECLARE_DO_FUN( do_berserk          );
DECLARE_DO_FUN( do_brandish         );
DECLARE_DO_FUN( do_brief            );
DECLARE_DO_FUN( do_bug              );
DECLARE_DO_FUN( do_buy              );
DECLARE_DO_FUN( do_cast             );
DECLARE_DO_FUN( do_catchup          );
DECLARE_DO_FUN( do_changes          );
DECLARE_DO_FUN( do_channels         );
DECLARE_DO_FUN( do_charstat         );
DECLARE_DO_FUN( do_cleanse          ); /* Crystal System by Aarchane */
DECLARE_DO_FUN( do_clone            );
DECLARE_DO_FUN( do_close            );
DECLARE_DO_FUN( do_colour           ); /* Colour Command By Lope */
DECLARE_DO_FUN( do_commands         );
DECLARE_DO_FUN( do_combine          );
DECLARE_DO_FUN( do_compact          );
DECLARE_DO_FUN( do_compare          );
DECLARE_DO_FUN( do_consider         );
DECLARE_DO_FUN( do_copyove          );
DECLARE_DO_FUN( do_copyover         );
DECLARE_DO_FUN( do_count            );
DECLARE_DO_FUN( do_credits          );
DECLARE_DO_FUN( do_deaf             );
DECLARE_DO_FUN( do_delet            );
DECLARE_DO_FUN( do_delete           );
DECLARE_DO_FUN( do_deny             );
DECLARE_DO_FUN( do_description      );
DECLARE_DO_FUN( do_destroy          );
DECLARE_DO_FUN( do_dirt             );
DECLARE_DO_FUN( do_disarm           );
DECLARE_DO_FUN( do_disconnect       );
DECLARE_DO_FUN( do_down             );
DECLARE_DO_FUN( do_drink            );
DECLARE_DO_FUN( do_drop             );
DECLARE_DO_FUN( do_dump             );
DECLARE_DO_FUN( do_east             );
DECLARE_DO_FUN( do_eat              );
DECLARE_DO_FUN( do_echo             );
DECLARE_DO_FUN( do_emote            );
DECLARE_DO_FUN( do_enter            );
DECLARE_DO_FUN( do_envenom          );
DECLARE_DO_FUN( do_equipment        );
DECLARE_DO_FUN( do_examine          );
DECLARE_DO_FUN( do_exits            );
DECLARE_DO_FUN( do_experience       );
DECLARE_DO_FUN( do_fill             );
DECLARE_DO_FUN( do_fish             );
DECLARE_DO_FUN( do_flag             );
DECLARE_DO_FUN( do_flare            );
DECLARE_DO_FUN( do_flee             );
DECLARE_DO_FUN( do_follow           );
DECLARE_DO_FUN( do_forage           );
DECLARE_DO_FUN( do_force            );
DECLARE_DO_FUN( do_freeze           );
DECLARE_DO_FUN( do_freport          );
DECLARE_DO_FUN( do_gain             );
DECLARE_DO_FUN( do_get              );
DECLARE_DO_FUN( do_give             );
DECLARE_DO_FUN( do_gossip           );
DECLARE_DO_FUN( do_goto             );
DECLARE_DO_FUN( do_grats            );
DECLARE_DO_FUN( do_group            );
DECLARE_DO_FUN( do_groups           );
DECLARE_DO_FUN( do_gtell            );
DECLARE_DO_FUN( do_guild            );
DECLARE_DO_FUN( do_unguild          );
DECLARE_DO_FUN( do_heal             );
DECLARE_DO_FUN( do_help             );
DECLARE_DO_FUN( do_hide             );
DECLARE_DO_FUN( do_holylight        );
DECLARE_DO_FUN( do_idea             );
DECLARE_DO_FUN( do_immtalk          );
DECLARE_DO_FUN( do_incognito        );
DECLARE_DO_FUN( do_clantalk         );
DECLARE_DO_FUN( do_identify         );
DECLARE_DO_FUN( do_imotd            );
DECLARE_DO_FUN( do_inventory        );
DECLARE_DO_FUN( do_invis            );
DECLARE_DO_FUN( do_kick             );
DECLARE_DO_FUN( do_kill             );
DECLARE_DO_FUN( do_leave            );
DECLARE_DO_FUN( do_level            );
DECLARE_DO_FUN( do_list             );
DECLARE_DO_FUN( do_listprogs        );
DECLARE_DO_FUN( do_load             );
DECLARE_DO_FUN( do_lock             );
DECLARE_DO_FUN( do_log              );
DECLARE_DO_FUN( do_look             );
DECLARE_DO_FUN( do_memory           );
DECLARE_DO_FUN( do_mfind            );
DECLARE_DO_FUN( do_mlevel           );
DECLARE_DO_FUN( do_mlist            );
DECLARE_DO_FUN( do_mount            );
DECLARE_DO_FUN( do_dismount         );
DECLARE_DO_FUN( do_mset             );
DECLARE_DO_FUN( do_mstat            );
DECLARE_DO_FUN( do_mhere            );
DECLARE_DO_FUN( do_mwhere           );
DECLARE_DO_FUN( do_mob              );
DECLARE_DO_FUN( do_mastered         );
DECLARE_DO_FUN( do_motd             );
DECLARE_DO_FUN( do_mpstat           );
DECLARE_DO_FUN( do_mpdump           );
DECLARE_DO_FUN( do_mplist           );
DECLARE_DO_FUN( do_murde            );
DECLARE_DO_FUN( do_murder           );
DECLARE_DO_FUN( do_music            );
DECLARE_DO_FUN( do_newlock          );
DECLARE_DO_FUN( do_news             );
DECLARE_DO_FUN( do_nochannels       );
DECLARE_DO_FUN( do_noemote          );
DECLARE_DO_FUN( do_nofollow         );
DECLARE_DO_FUN( do_noloot           );
DECLARE_DO_FUN( do_north            );
DECLARE_DO_FUN( do_nospam           );
DECLARE_DO_FUN( do_noshout          );
DECLARE_DO_FUN( do_nogate           );
DECLARE_DO_FUN( do_nosummon         );
DECLARE_DO_FUN( do_note             );
DECLARE_DO_FUN( do_notell           );
DECLARE_DO_FUN( do_ofind            );
DECLARE_DO_FUN( do_olevel           );
DECLARE_DO_FUN( do_olist            );
DECLARE_DO_FUN( do_open             );
DECLARE_DO_FUN( do_order            );
DECLARE_DO_FUN( do_oset             );
DECLARE_DO_FUN( do_ostat            );
DECLARE_DO_FUN( do_outfit           );
DECLARE_DO_FUN( do_owhere           );
DECLARE_DO_FUN( do_rwhere           );
DECLARE_DO_FUN( do_password         );
DECLARE_DO_FUN( do_peace            );
DECLARE_DO_FUN( do_pecho            );
DECLARE_DO_FUN( do_penalty          );
DECLARE_DO_FUN( do_permban          );
DECLARE_DO_FUN( do_pick             );
DECLARE_DO_FUN( do_play             );
DECLARE_DO_FUN( do_pmote            );
DECLARE_DO_FUN( do_pose             );
DECLARE_DO_FUN( do_pour             );
DECLARE_DO_FUN( do_practice         );
DECLARE_DO_FUN( do_prefi            );
DECLARE_DO_FUN( do_prefix           );
DECLARE_DO_FUN( do_prompt           );
DECLARE_DO_FUN( do_protect          );
DECLARE_DO_FUN( do_purge            );
DECLARE_DO_FUN( do_put              );
DECLARE_DO_FUN( do_quaff            );
DECLARE_DO_FUN( do_question         );
DECLARE_DO_FUN( do_qui              );
DECLARE_DO_FUN( do_quiet            );
DECLARE_DO_FUN( do_quit             );
DECLARE_DO_FUN( do_quote            );
DECLARE_DO_FUN( do_read             );
DECLARE_DO_FUN( do_reboo            );
DECLARE_DO_FUN( do_reboot           );
DECLARE_DO_FUN( do_recall           );
DECLARE_DO_FUN( do_recho            );
DECLARE_DO_FUN( do_recite           );
DECLARE_DO_FUN( do_remove           );
DECLARE_DO_FUN( do_rent             );
DECLARE_DO_FUN( do_repair           );
DECLARE_DO_FUN( do_replay           );
DECLARE_DO_FUN( do_reply            );
DECLARE_DO_FUN( do_report           );
DECLARE_DO_FUN( do_rescue           );
DECLARE_DO_FUN( do_rest             );
DECLARE_DO_FUN( do_restore          );
DECLARE_DO_FUN( do_fullrestore      );
DECLARE_DO_FUN( do_return           );
DECLARE_DO_FUN( do_rlist            );
DECLARE_DO_FUN( do_rpnote           );
DECLARE_DO_FUN( do_rsearch          );
DECLARE_DO_FUN( do_msearch          );
DECLARE_DO_FUN( do_hsearch          );
DECLARE_DO_FUN( do_rset             );
DECLARE_DO_FUN( do_rstat            );
DECLARE_DO_FUN( do_rules            );
DECLARE_DO_FUN( do_sacrifice        );
DECLARE_DO_FUN( do_save             );
DECLARE_DO_FUN( do_saveskills       );
DECLARE_DO_FUN( do_say              );
DECLARE_DO_FUN( do_score            );
DECLARE_DO_FUN( do_scorio           );
DECLARE_DO_FUN( do_security         );
DECLARE_DO_FUN( do_oldscore         );
DECLARE_DO_FUN( do_scroll           );
DECLARE_DO_FUN( do_second           );
DECLARE_DO_FUN( do_sell             );
DECLARE_DO_FUN( do_set              );
DECLARE_DO_FUN( do_shout            );
DECLARE_DO_FUN( do_show             );
DECLARE_DO_FUN( do_statshow         );
DECLARE_DO_FUN( do_shutdow          );
DECLARE_DO_FUN( do_shutdown         );
DECLARE_DO_FUN( do_sit              );
DECLARE_DO_FUN( do_skills           );
DECLARE_DO_FUN( do_sla              );
DECLARE_DO_FUN( do_slay             );
DECLARE_DO_FUN( do_sleep            );
DECLARE_DO_FUN( do_sleeptells       );
DECLARE_DO_FUN( do_slookup          );
DECLARE_DO_FUN( do_smote            );
DECLARE_DO_FUN( do_sneak            );
DECLARE_DO_FUN( do_snoop            );
DECLARE_DO_FUN( do_socials          );
DECLARE_DO_FUN( do_south            );
DECLARE_DO_FUN( do_sockets          );
DECLARE_DO_FUN( do_spells           );
DECLARE_DO_FUN( do_split            );
DECLARE_DO_FUN( do_sset             );
DECLARE_DO_FUN( do_stand            );
DECLARE_DO_FUN( do_statistics       );
DECLARE_DO_FUN( do_stats            );
DECLARE_DO_FUN( do_stat             );
DECLARE_DO_FUN( do_steal            );
DECLARE_DO_FUN( do_butcher          );
DECLARE_DO_FUN( do_story            );
DECLARE_DO_FUN( do_string           );
DECLARE_DO_FUN( do_surrender        );
DECLARE_DO_FUN( do_switch           );
DECLARE_DO_FUN( do_tell             );
DECLARE_DO_FUN( do_time             );
DECLARE_DO_FUN( do_title            );
DECLARE_DO_FUN( do_toggle           );
DECLARE_DO_FUN( do_touch            );
DECLARE_DO_FUN( do_press            );
DECLARE_DO_FUN( do_dig              );
DECLARE_DO_FUN( do_pull             );
DECLARE_DO_FUN( do_turn             );
DECLARE_DO_FUN( do_train            );
DECLARE_DO_FUN( do_transfer         );
DECLARE_DO_FUN( do_trip             );
DECLARE_DO_FUN( do_trust            );
DECLARE_DO_FUN( do_typo             );
DECLARE_DO_FUN( do_unalias          );
DECLARE_DO_FUN( do_unlock           );
DECLARE_DO_FUN( do_unread           );
DECLARE_DO_FUN( do_up               );
DECLARE_DO_FUN( do_value            );
DECLARE_DO_FUN( do_visible          );
DECLARE_DO_FUN( do_violate          );
DECLARE_DO_FUN( do_vnum             );
DECLARE_DO_FUN( do_wake             );
DECLARE_DO_FUN( do_wear             );
DECLARE_DO_FUN( do_weather          );
DECLARE_DO_FUN( do_west             );
DECLARE_DO_FUN( do_where            );
DECLARE_DO_FUN( do_who              );
DECLARE_DO_FUN( do_whois            );
DECLARE_DO_FUN( do_wimpy            );
DECLARE_DO_FUN( do_withdraw         );
DECLARE_DO_FUN( do_wizhelp          );
DECLARE_DO_FUN( do_wizlock          );
DECLARE_DO_FUN( do_wizlist          );
DECLARE_DO_FUN( do_wiznet           );
DECLARE_DO_FUN( do_worth            );
DECLARE_DO_FUN( do_yell             );
DECLARE_DO_FUN( do_zap              );
DECLARE_DO_FUN( do_zecho            );
DECLARE_DO_FUN( do_olc              );
DECLARE_DO_FUN( do_asave            );
DECLARE_DO_FUN( do_alist            );
DECLARE_DO_FUN( do_alist_sort       );
DECLARE_DO_FUN( do_resets           );
DECLARE_DO_FUN( do_redit            );
DECLARE_DO_FUN( do_aedit            );
DECLARE_DO_FUN( do_medit            );
DECLARE_DO_FUN( do_oedit            );
DECLARE_DO_FUN( do_mpedit           );
DECLARE_DO_FUN( do_hedit            );
DECLARE_DO_FUN( do_cedit            );
DECLARE_DO_FUN( do_next             );
DECLARE_DO_FUN( do_bank             );
DECLARE_DO_FUN( do_bank_transfer    );
DECLARE_DO_FUN( do_omni             );
DECLARE_DO_FUN( do_scan             );
DECLARE_DO_FUN( do_descry           );
DECLARE_DO_FUN( do_finger           );
DECLARE_DO_FUN( do_gocial           );
DECLARE_DO_FUN( do_donate           );
DECLARE_DO_FUN( do_payoff           );
DECLARE_DO_FUN( do_pload            );
DECLARE_DO_FUN( do_plock            );
DECLARE_DO_FUN( do_punload          );
DECLARE_DO_FUN( do_sedit            );
DECLARE_DO_FUN( do_as               );
DECLARE_DO_FUN( do_aslist           );
DECLARE_DO_FUN( do_cloak            );
DECLARE_DO_FUN( do_secset           );
DECLARE_DO_FUN( do_ooc              );
DECLARE_DO_FUN( do_irl              );
DECLARE_DO_FUN( do_promote          );
DECLARE_DO_FUN( do_wartalk          );
DECLARE_DO_FUN( do_diamonds         );
DECLARE_DO_FUN( do_saveclan         );
DECLARE_DO_FUN( do_saveclass        );
DECLARE_DO_FUN( do_roster           );
DECLARE_DO_FUN( do_clanname         );
DECLARE_DO_FUN( do_clandesc         );
DECLARE_DO_FUN( do_clanhall         );
DECLARE_DO_FUN( do_clanstate        );
DECLARE_DO_FUN( do_clantotal        );
DECLARE_DO_FUN( do_clanfree         );
DECLARE_DO_FUN( do_clanused         );
DECLARE_DO_FUN( do_clanlist         );
DECLARE_DO_FUN( do_clanpatron       );
DECLARE_DO_FUN( do_addnewclan       );
DECLARE_DO_FUN( do_push             );
DECLARE_DO_FUN( do_drag             );
DECLARE_DO_FUN( do_skill            );
DECLARE_DO_FUN( do_grp              );
DECLARE_DO_FUN( do_cps              );
DECLARE_DO_FUN( do_classlist        );
DECLARE_DO_FUN( do_classcps         );
DECLARE_DO_FUN( do_groupcps         );
DECLARE_DO_FUN( do_brew             );
DECLARE_DO_FUN( do_scribe           );
DECLARE_DO_FUN( do_craft            );
DECLARE_DO_FUN( do_imprint          );
DECLARE_DO_FUN( do_erase            );
DECLARE_DO_FUN( do_empty            );
DECLARE_DO_FUN( do_check            );
DECLARE_DO_FUN( do_otype            );
DECLARE_DO_FUN( do_bounty           );
DECLARE_DO_FUN( do_spellstat        );
DECLARE_DO_FUN( do_skillstat        );
DECLARE_DO_FUN( do_wpeace           );
DECLARE_DO_FUN( do_quest            );
DECLARE_DO_FUN( do_bank_update      );
DECLARE_DO_FUN( do_info             );
DECLARE_DO_FUN( do_alert            );
DECLARE_DO_FUN( do_clanrequest      );
DECLARE_DO_FUN( do_lore             );
DECLARE_DO_FUN( do_reset_roster     );
DECLARE_DO_FUN( do_stake            );
DECLARE_DO_FUN( do_fixed            );
DECLARE_DO_FUN( do_duh              );
DECLARE_DO_FUN( do_rename           );
DECLARE_DO_FUN( do_new_equipment    );
DECLARE_DO_FUN( do_exlist           );
DECLARE_DO_FUN( do_fvlist           );
DECLARE_DO_FUN( do_new_dump         );
DECLARE_DO_FUN( do_familiar         );
DECLARE_DO_FUN( do_deathgrip        );
DECLARE_DO_FUN( do_study            ); /* by Aarchane */
DECLARE_DO_FUN( do_tithe            ); /* by Plasma */
DECLARE_DO_FUN( do_whirlwind        );
DECLARE_DO_FUN( do_nocancel         );
DECLARE_DO_FUN( do_map              );
DECLARE_DO_FUN( do_map_mud          );
DECLARE_DO_FUN( do_remove_clannie   );
DECLARE_DO_FUN( do_addlag           );
DECLARE_DO_FUN( do_circle           );
DECLARE_DO_FUN( do_shatter          );
DECLARE_DO_FUN( do_familiar         );
DECLARE_DO_FUN( do_bind             );
DECLARE_DO_FUN( do_autoall          );
DECLARE_DO_FUN( do_politic          );
DECLARE_DO_FUN( do_newsfaerie       );
DECLARE_DO_FUN( do_strangle         );
DECLARE_DO_FUN( do_crosscut         );
DECLARE_DO_FUN( do_gore             );
DECLARE_DO_FUN( do_nerve            );
DECLARE_DO_FUN( do_takedown         );
DECLARE_DO_FUN( do_focus            );
DECLARE_DO_FUN( do_buckkick         );
DECLARE_DO_FUN( do_remgroup         );
DECLARE_DO_FUN( do_cleansockets     );
DECLARE_DO_FUN( do_size             );
DECLARE_DO_FUN( do_cleanchar        );
DECLARE_DO_FUN( do_pkable           );
DECLARE_DO_FUN( do_affremove        );
DECLARE_DO_FUN( do_clandeduct       );
DECLARE_DO_FUN( do_areakill         );
DECLARE_DO_FUN( do_objcheck         );
DECLARE_DO_FUN( do_mobcheck         );
DECLARE_DO_FUN( do_roomcheck        );
DECLARE_DO_FUN( do_nodigmove        );
DECLARE_DO_FUN( do_track            );
DECLARE_DO_FUN( do_dart             );
DECLARE_DO_FUN( do_compress         );
DECLARE_DO_FUN( do_xlist            );
DECLARE_DO_FUN( do_clean_roster     );
DECLARE_DO_FUN( do_stomp            );
DECLARE_DO_FUN( do_history          );
DECLARE_DO_FUN( do_flying_kick      );
DECLARE_DO_FUN( do_demand           );
DECLARE_DO_FUN( do_deposit          );
DECLARE_DO_FUN( do_chomp            );
DECLARE_DO_FUN( do_objaff           );
DECLARE_DO_FUN( do_reply_break      );
DECLARE_DO_FUN( do_reply_lock       );
DECLARE_DO_FUN( do_forget           );
DECLARE_DO_FUN( do_remember         );
DECLARE_DO_FUN( do_sextalk          );
DECLARE_DO_FUN( do_sexpolice        );
DECLARE_DO_FUN( do_gm               );
DECLARE_DO_FUN( do_vflagtime        );
DECLARE_DO_FUN( do_seeall           );
DECLARE_DO_FUN( do_newbie           );
DECLARE_DO_FUN( do_newbiehint       );
DECLARE_DO_FUN( do_game             );
DECLARE_DO_FUN( do_style            );
DECLARE_DO_FUN( do_zwhere           );
DECLARE_DO_FUN( do_clanstatus       );
DECLARE_DO_FUN( do_silent_force     );
DECLARE_DO_FUN( do_aschar           );
DECLARE_DO_FUN( do_newcount         );
DECLARE_DO_FUN( do_clanmode         );
DECLARE_DO_FUN( do_hints            );
DECLARE_DO_FUN( do_autostats        );
DECLARE_DO_FUN( do_whisper          );
DECLARE_DO_FUN( do_smite            );
DECLARE_DO_FUN( do_ironwill         );
DECLARE_DO_FUN( do_prayer           );
DECLARE_DO_FUN( do_telnet_ga        );
DECLARE_DO_FUN( do_immsay           );
DECLARE_DO_FUN( do_helpcheck        );
DECLARE_DO_FUN( do_clanowe          );
DECLARE_DO_FUN( do_spell_maintain   );
DECLARE_DO_FUN( do_ospellup         );
DECLARE_DO_FUN( do_wspellup         );
DECLARE_DO_FUN( do_spellup          );
DECLARE_DO_FUN( do_glance           );
DECLARE_DO_FUN( do_showhints        );
DECLARE_DO_FUN( do_show_cmd_hist    );
DECLARE_DO_FUN( do_rub              );
DECLARE_DO_FUN( do_godname          );
DECLARE_DO_FUN( do_gen_denylist     );
DECLARE_DO_FUN( do_show_denied_list );
DECLARE_DO_FUN( do_undeny           );
DECLARE_DO_FUN( do_worklist         );
DECLARE_DO_FUN( do_tasklist         );
DECLARE_DO_FUN( do_synopsis         );
DECLARE_DO_FUN( do_logsearch        );
DECLARE_DO_FUN( do_cls              );
DECLARE_DO_FUN( do_sharpen          );
DECLARE_DO_FUN( do_shelter          );
DECLARE_DO_FUN( do_bail             );
DECLARE_DO_FUN( do_cmdhist          );
DECLARE_DO_FUN( do_show_fixed_buffer);
DECLARE_DO_FUN( do_ispell           );
DECLARE_DO_FUN( do_dice             );
DECLARE_DO_FUN( do_newdice          );
DECLARE_DO_FUN( do_crusade          ); /*Inquisitor skill */
DECLARE_DO_FUN( do_layhands         );
DECLARE_DO_FUN( do_questlist        );
DECLARE_DO_FUN( do_punish           );
DECLARE_DO_FUN( do_check_valid_eq   );
DECLARE_DO_FUN( do_setpassword      );
DECLARE_DO_FUN( do_cleanup          );
DECLARE_DO_FUN( do_beep             );
DECLARE_DO_FUN( do_junk             );
DECLARE_DO_FUN( do_snooplist        );
DECLARE_DO_FUN( do_score_stats      );
DECLARE_DO_FUN( do_other_scorio     );
DECLARE_DO_FUN( do_score_clan       );
DECLARE_DO_FUN( do_warcry           );
DECLARE_DO_FUN( do_tail             );
DECLARE_DO_FUN( do_home             );
DECLARE_DO_FUN( do_decorate         );
DECLARE_DO_FUN( do_throw            );
DECLARE_DO_FUN( do_rebrew           );
DECLARE_DO_FUN( do_transcribe       );
DECLARE_DO_FUN( do_chameleon        );
DECLARE_DO_FUN( do_client           );
DECLARE_DO_FUN( do_psearch          );
DECLARE_DO_FUN( do_shield_bash      );
DECLARE_DO_FUN( do_transport        );
#endif
