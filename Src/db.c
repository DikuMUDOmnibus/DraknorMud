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
*    ROM 2.4 is copyright 1993-1996 Russ Taylor               *
*    ROM has been brought to you by the ROM consortium           *
*        Russ Taylor (rtaylor@efn.org)                   *
*        Gabrielle Taylor                           *
*        Brian Moore (zump@rom.org)                       *
*    By using this code, you have agreed to follow the terms of the       *
*    ROM license, in the file Rom24/doc/rom.license               *
***************************************************************************/
/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include "merc.h"
#include "db.h"
#include "recycle.h"
#include "music.h"
#include "lookup.h"
#include "interp.h"
#include "tables.h"

#if !defined(macintosh)
extern    int    _filbuf        args( (FILE *) );
#endif

#if !defined(OLD_RAND)
/*
long random();
*/
void srandom(unsigned int);
int getpid();
time_t time(time_t *tloc);
#endif

void parse_credits(AREA_DATA *pArea);

/* externals for counting purposes */
extern    OBJ_DATA    *obj_free;
extern    CHAR_DATA    *char_free;
extern  DESCRIPTOR_DATA *descriptor_free;
extern    PC_DATA        *pcdata_free;
extern    QUEST_DATA    *questdata_free;
extern  AFFECT_DATA    *affect_free;

/*
 * Globals.
 */
HELP_DATA *        help_first;
HELP_DATA *        help_last;

SHOP_DATA *        shop_first;
SHOP_DATA *        shop_last;

NOTE_DATA *        note_free;

CHAR_DATA  *    prog_master;
MPROG_CODE *    mprog_list;

char            bug_buf        [2*MAX_INPUT_LENGTH];
CHAR_DATA *        char_list;
CHAR_DATA *        player_list;
DENIED_DATA *           denied_list;
char *            help_greeting;
char            log_buf        [2*MAX_INPUT_LENGTH];
KILL_DATA        kill_table    [MAX_LEVEL];
NOTE_DATA *        note_list;
OBJ_DATA *        object_list;
TIME_INFO_DATA        time_info;
WEATHER_DATA        weather_info;
AUCTION_DATA            auction_info;
extern    BAN_DATA        *ban_list;
extern    BAN_DATA        *ban_free;

/*
 * Draknor Unique gsn
 */
sh_int          gsn_descry;

/*
 * gsns
 */
sh_int            gsn_backstab;
sh_int            gsn_dodge;
sh_int            gsn_envenom;
sh_int            gsn_toxicology;
sh_int            gsn_hide;
sh_int            gsn_cloak_of_assassins;

sh_int            gsn_peek;
sh_int            gsn_pick_lock;
sh_int            gsn_sneak;
sh_int            gsn_steal;

sh_int            gsn_disarm;
sh_int            gsn_enhanced_damage;
sh_int            gsn_kick;
sh_int            gsn_parry;
sh_int            gsn_rescue;
sh_int						gsn_double_shot;
sh_int            gsn_second_attack;
sh_int            gsn_third_attack;
sh_int            gsn_fourth_attack;

sh_int            gsn_blindness;
sh_int            gsn_charm_person;
sh_int                  gsn_change_sex;
sh_int            gsn_curse;
sh_int            gsn_invis;
sh_int            gsn_mass_invis;
sh_int            gsn_poison;
sh_int            gsn_plague;
sh_int            gsn_sleep;
sh_int            gsn_sanctuary;
sh_int            gsn_fly;
sh_int                  gsn_weaken;
/* new gsns */

sh_int          gsn_axe;
sh_int          gsn_dagger;
sh_int          gsn_flail;
sh_int          gsn_mace;
sh_int          gsn_polearm;
sh_int          gsn_crossbow;
sh_int            gsn_shield_block;
sh_int          gsn_spear;
sh_int          gsn_sword;
sh_int          gsn_whip;
 
sh_int          gsn_bash;
sh_int          gsn_berserk;
sh_int          gsn_rampage;
sh_int          gsn_dirt;
sh_int          gsn_hand_to_hand;
sh_int          gsn_trip;
 
sh_int          gsn_fast_healing;
sh_int          gsn_haggle;
sh_int          gsn_lore;
sh_int          gsn_meditation;
 
sh_int      gsn_scrolls;
sh_int      gsn_staves;
sh_int      gsn_wands;
sh_int      gsn_recall;
sh_int      gsn_butcher;
sh_int      gsn_brew;
sh_int      gsn_advanced_brew;
sh_int      gsn_scribe;
sh_int      gsn_advanced_scribe;
sh_int      gsn_erase;
sh_int      gsn_empty;
sh_int      gsn_stake;
sh_int      gsn_counter;
sh_int      gsn_familiar;
sh_int      gsn_critical;
sh_int      gsn_tithe;
sh_int      gsn_whirlwind;
sh_int      gsn_tumble;
sh_int      gsn_circle;
sh_int      gsn_focus;
sh_int      gsn_shatter;
sh_int      gsn_bind;
sh_int      gsn_strangle;
sh_int      gsn_crosscut;
sh_int      gsn_gore;
sh_int      gsn_nerve;
sh_int      gsn_takedown;
sh_int      gsn_focus;
sh_int      gsn_buckkick;
sh_int      gsn_karate;
sh_int      gsn_sharpen;
sh_int      gsn_shelter;
sh_int      gsn_exotic;
sh_int      gsn_track;
sh_int      gsn_second_cast;
sh_int      gsn_third_cast;
sh_int      gsn_automap;
sh_int      gsn_dart;
sh_int      gsn_acid_breath;
sh_int      gsn_feint;
sh_int      gsn_fifth_attack;
sh_int      gsn_dual_attack;
sh_int      gsn_stomp;
sh_int      gsn_flying;
sh_int      gsn_demand;
sh_int      gsn_bite;
sh_int      gsn_second;
sh_int      gsn_hiccup;
sh_int      gsn_yawn;
sh_int      gsn_basic_style;
sh_int      gsn_dragon_style;
sh_int      gsn_drunk_style;
sh_int      gsn_tiger_style;
sh_int      gsn_snake_style;
sh_int      gsn_crane_style;
sh_int      gsn_ironfist_style;
sh_int      gsn_judo_style;
sh_int      gsn_ghost_time;
sh_int      gsn_nochannel;
sh_int      gsn_ironwill;
sh_int      gsn_bear_spirit;
sh_int      gsn_eagle_spirit;
sh_int      gsn_tiger_spirit;
sh_int      gsn_dragon_spirit;
sh_int      gsn_flare;
sh_int      gsn_twit;
sh_int      gsn_martyr;
sh_int      gsn_rub;
sh_int      gsn_dagger_twist;
sh_int      gsn_adrenalize;
sh_int      gsn_investigate;
sh_int      gsn_triple_attack;
sh_int      gsn_forage;
sh_int      gsn_warcry_vigor;
sh_int      gsn_warcry_guarding;
sh_int      gsn_warcry_shout;
sh_int      gsn_warcry_hardening;
sh_int      gsn_warcry_rage;
sh_int      gsn_layhands;
sh_int      gsn_guided_strike;
sh_int      gsn_repair;
sh_int      gsn_crusade;
sh_int      gsn_tail;
sh_int      gsn_sidestep;
sh_int      gsn_blade_weave;
sh_int      gsn_craft;
sh_int      gsn_advanced_craft;
sh_int      gsn_crusade;
sh_int      gsn_guided_strike;
sh_int      gsn_layhands;
sh_int      gsn_throw;
sh_int      gsn_rebrew;
sh_int      gsn_transcribe;
sh_int      gsn_inverted_light;
sh_int      gsn_chameleon;
sh_int      gsn_shield_bash;
sh_int      gsn_sweep;

/*
 * Locals.
 */
MOB_INDEX_DATA *    mob_index_hash        [MAX_KEY_HASH];
OBJ_INDEX_DATA *    obj_index_hash        [MAX_KEY_HASH];
ROOM_INDEX_DATA *    room_index_hash        [MAX_KEY_HASH];
char *            string_hash        [MAX_KEY_HASH];

AREA_DATA *        area_first;
AREA_DATA *        area_last;
AREA_DATA *        area_first_sorted;
CLAN_DATA *     clan_first;
char *            string_space;
char *            top_string;
char            str_empty    [1];

int            top_affect;
int            top_area;
int            top_ed;
int            top_ban;
int            top_exit;
int            top_help;
int            top_mob_index;
int            top_obj_index;
int            top_reset;
int            top_room;
int            top_shop;
int                     top_vnum_room;        /* OLC */
int                     top_vnum_mob;        /* OLC */
int                     top_vnum_obj;        /* OLC */
int            top_mprog_index;    /* OLC */
int             mobile_count = 0;
int            newmobs = 0;
int            newobjs = 0;

void *            rgFreeList    [MAX_MEM_LIST];
const int        rgSizeList    [MAX_MEM_LIST]    =
{
  16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, MAX_ALLOC_SIZE
};

int            nAllocString;
int            sAllocString;
int            nAllocPerm;
int            sAllocPerm;
int            nPermBlock    = 0; /* Perm blocks allocated */
int            nSlackPerm    = 0; /* Slack space for perm blocks */
int            iMemPerm    = 0; /* Current block used */
int                     nFilesOpen      =0;
int                     nDescsOpen      =0;
int                     pObjNum = 0;

/*
 * Semi-locals.
 */
bool            fBootDb;
FILE *            fpArea;
char            strArea[MAX_INPUT_LENGTH];
/* For global debugging */
char interp_cmd[MSL];
struct cmd_hist_st cmd_hist[MAX_SAVE_CMDS];
/*
 * Local booting procedures.
*/
void    init_mm         args( ( void ) );
void    load_area    args( ( FILE *fp ) );
void    new_load_area   args( ( FILE *fp ) );   /* OLC */
void    load_helps    args( ( FILE *fp ) );
void    load_helps_1    args( ( FILE *fp ) );
void    load_old_mob    args( ( FILE *fp ) );
void     load_mobiles    args( ( FILE *fp ) );
void    load_old_obj    args( ( FILE *fp ) );
void     load_objects    args( ( FILE *fp ) );
void    load_resets        args( ( FILE *fp ) );
void    load_rooms      args( ( FILE *fp ) );
void    load_shops        args( ( FILE *fp ) );
void     load_mobprogs       ( FILE *fp );
void    fix_objects         ( void);
void     fix_mobprogs        ( void );
void    fix_roomprogs       ( void );
void    fix_objprogs        ( void );
/*void     load_socials    args( ( FILE *fp ) );*/
void    load_specials    args( ( FILE *fp ) );
void    load_notes    args( ( void ) );
void    load_bans    args( ( void ) );
void update_current_time( void );

void    fix_exits    args( ( void ) );
void     load_bank    args( ( void ) );
void    reset_area    args( ( AREA_DATA * pArea ) );
void     sort_areas_by_level    args ((void));
bool    mp_onload_trigger( CHAR_DATA *ch );

/*
 * Big mama top level function.
 */
void boot_db()
{
  strcpy(interp_cmd,"(none yet)");
#if SORTING_INTERP
  int cmd=0;
  for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ );
  cmd--;
  int_sort(cmd_table, 0, cmd-1);
#endif
#if SORTING_SKILLS
  int_sort(skill_table, 1, MAX_SKILL-1);
#endif
  /*
   * Init some data space stuff.
   */
  {
    if ( ( string_space = calloc( 1, MAX_STRING ) ) == NULL )
      {
    bug( "Boot_db: can't alloc %d string space.", MAX_STRING );
    exit( 1 );
      }
    top_string    = string_space;
    fBootDb        = TRUE;
  }
  /*
   * Init random number generator.
   */
  {
    init_mm( );
  }

  ispell_init();

  auction_info.item           = NULL;
  auction_info.owner          = NULL;
  auction_info.high_bidder    = NULL;
  auction_info.current_bid    = 0;
  auction_info.status         = 0;
  auction_info.gold_held    = 0;
  auction_info.silver_held    = 0;
  auction_info.minimum_bid    = 0;

  /*
   * Set time and weather.
   */
  {
    long lhour, lday, lmonth;

    lhour        = (current_time - 650336715)
      / (PULSE_TICK / PULSE_PER_SECOND);
    time_info.hour    = lhour  % 24;
    lday        = lhour  / 24;
    time_info.day    = lday   % 35;
    lmonth        = lday   / 35;
    time_info.month    = lmonth % 17;
    time_info.year    = lmonth / 17;

    if ( time_info.hour <  5 ) weather_info.sunlight = SUN_DARK;
    else if ( time_info.hour <  6 ) weather_info.sunlight = SUN_RISE;
    else if ( time_info.hour < 19 ) weather_info.sunlight = SUN_LIGHT;
    else if ( time_info.hour < 20 ) weather_info.sunlight = SUN_SET;
    else                            weather_info.sunlight = SUN_DARK;

    weather_info.change    = 0;
    weather_info.mmhg    = 960;
    if ( time_info.month >= 7 && time_info.month <=12 )
      weather_info.mmhg += number_range( 1, 50 );
    else
      weather_info.mmhg += number_range( 1, 80 );

    if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
    else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
    else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
    else                                  weather_info.sky = SKY_CLOUDLESS;

  }
  /* load denied file */
  read_denied_file();
  /*
   * Assign gsn's for skills which have them.
   */
  {
    int sn;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
      {
    if ( skill_table[sn].pgsn != NULL )
      *skill_table[sn].pgsn = sn;
      }

  logit("Done Reading Skill List.\n\r");
  }
  /*    load_classes();
    load_groups();
    load_creation_points();
    load_spell_info(SPELLS_FILE);
  */    
  {
    sh_int i;
    countMax = countMaxDay = 0;
    for ( i = 0; i < 24; i++ )
      countArr[i] = 0;
    for ( i = 0; i < 7; i++ )
      countMaxDoW[i] = 0;
    countHour = ( current_time / 3600 ) % 24;
  }

  setup_revered();
  /*
   * Read in all the area files.
   */

  {
    FILE *fpList;

    if ( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL )
      {
    perror( AREA_LIST );
    exit( 1 );
      }
    nFilesOpen++;
    load_clan_info(CLAN_FILE);
    load_clan_status(CLAN_STATUS_FILE);
    for ( ; ; )
      {
    strcpy( strArea, fread_word( fpList ) );
    if ( strArea[0] == '$' )
      break;

    if ( strArea[0] == '-' )
      {
        fpArea = stdin;
      }
    else
      {
        if ( ( fpArea = fopen( strArea, "r" ) ) == NULL )
          {
        perror( strArea );
        exit( 1 );
          }
        nFilesOpen++;
      }

    for ( ; ; )
      {
        char *word;

        if ( fread_letter( fpArea ) != '#' )
          {
        bug( "Boot_db: # not found.", 0 );
        exit( 1 );
          }

        word = fread_word( fpArea );
        //bug ( "Area loaded", 0 );

        if ( word[0] == '$'               )                 break;
        else if ( !str_cmp( word, "AREA"     ) ) load_area    (fpArea);
        /* OLC */     else if ( !str_cmp( word, "AREADATA" ) ) new_load_area(fpArea);
        else if ( !str_cmp( word, "HELPS"    ) ) load_helps   (fpArea);
        else if ( !str_cmp( word, "HELPS_1"  ) ) load_helps_1 (fpArea);
        else if ( !str_cmp( word, "MOBOLD"   ) ) load_old_mob (fpArea);
        else if ( !str_cmp( word, "MOBILES"  ) ) load_mobiles (fpArea);
        else if ( !str_cmp( word, "MOBPROGS" ) ) load_mobprogs(fpArea);
        else if ( !str_cmp( word, "OBJOLD"   ) ) load_old_obj (fpArea);
        else if ( !str_cmp( word, "OBJECTS"  ) ) load_objects (fpArea);
        else if ( !str_cmp( word, "RESETS"   ) ) load_resets  (fpArea);
        else if ( !str_cmp( word, "ROOMS"    ) ) load_rooms   (fpArea);
        else if ( !str_cmp( word, "SHOPS"    ) ) load_shops   (fpArea);
        else if ( !str_cmp( word, "SPECIALS" ) ) load_specials(fpArea);
        else
          {
        bug( "Boot_db: bad section name.", 0 );
        exit( 1 );
          }
      }

    if ( fpArea != stdin ) {
      fclose( fpArea );
      nFilesOpen--;
    }
    fpArea = NULL;
      }
    fclose( fpList );
    nFilesOpen--;
  }

  /*
   * Fix up exits.
   * Declare db booting over.
   * Reset all areas once.
   * Load up the songs, notes and ban files.
   */
  {
    fix_exits( );
    update_current_time();
    fix_objects();
    update_current_time();
    sort_areas_by_level();
    update_current_time();
    fix_mobprogs( );
    fix_roomprogs( );
    fix_objprogs( );
    update_current_time( );
    fBootDb    = FALSE;
    convert_objects( );        /* ROM OLC */
    area_update( );
    load_notes( );
    load_social_table();
//    load_bans();
    load_bank();
    load_songs();
    load_sys_data();
    load_clan_list(ROSTER_FILE);
    // create the prog_master
    prog_master = create_mobile( get_mob_index( 1 ) );
    //(&CLAN)->name = str_dup( "", (&CLAN)->name );

  }

  return;
}

bool check_str_end( const char *str )
{
    char *p;

    if ( IS_NULLSTR( str ) )
    return TRUE;

    for ( p = (char *)str; *p; p++ );

    if ( *(p-1) == '\r' )
    return TRUE;

    return FALSE;
}

/*
 * Snarf an 'area' header line.
 */
void load_area( FILE *fp )
{
  AREA_DATA *pArea;

  pArea        = alloc_perm( sizeof(*pArea) );
  /*  pArea->reset_first    = NULL;
      pArea->reset_last    = NULL; */
  pArea->file_name  = fread_string(fp);
  pArea->area_flags = AREA_LOADING;         /* OLC */
  pArea->security   = 9;                    /* OLC */ /* 9 -- Hugin */
  pArea->continent  = 1;                    /* OLC */ /* Continents added 1/17/03 RWL */
  pArea->builders   = str_dup( "None",pArea->builders );    /* OLC */
  pArea->vnum       = top_area;             /* OLC */
  pArea->name       = fread_string(fp);
  pArea->credits    = fread_string(fp);
  pArea->min_vnum   = fread_number(fp);
  pArea->max_vnum   = fread_number(fp);
  pArea->age        = 15;
  pArea->version    = 0;
  pArea->align      = AREA_ALIGN_ALL;
  pArea->nplayer    = 0;
  pArea->reset_rate = 0;
  pArea->empty      = TRUE;

  if ( !area_first )
    area_first = pArea;
  if ( area_last )
    {
      area_last->next = pArea;
      REMOVE_BIT(area_last->area_flags, AREA_LOADING);        /* OLC */
    }
  area_last    = pArea;
  pArea->next    = NULL;

  top_area++;
  return;
}

/*
 * OLC
 * Use these macros to load any new area formats that you choose to
 * support on your MUD.  See the new_load_area format below for
 * a short example.
 */
#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                \
                if ( !str_cmp( word, literal ) )    \
                {                                   \
                    field  = value;                 \
                    fMatch = TRUE;                  \
                    break;                          \
                                }

#define SKEY( string, field )                       \
                if ( !str_cmp( word, string ) )     \
                {                                   \
                    free_string( field );           \
                    field = fread_string( fp );     \
                    fMatch = TRUE;                  \
                    break;                          \
                }



/* OLC
 * Snarf an 'area' header line.   Check this format.  MUCH better.  Add fields
 * too.
 *
 * #AREAFILE
 * Name   { All } Locke    Newbie School~
 * Repop  A teacher pops in the room and says, 'Repop coming!'~
 * Recall 3001
 * End
 */
void new_load_area( FILE *fp )
{
  AREA_DATA *pArea;
  char      *word;
  bool      fMatch;

  pArea             = alloc_perm( sizeof(*pArea) );
  pArea->age        = 15;
  pArea->nplayer    = 0;
  pArea->empty      = TRUE;
  pArea->file_name  = str_dup( strArea , pArea->file_name);
  pArea->vnum       = top_area;
  pArea->name       = str_dup( "New Area", pArea->name );
  pArea->builders   = str_dup( "" , pArea->builders);
  pArea->security   = 9;                    /* 9 -- Hugin */
  pArea->continent  = 1;
  pArea->min_vnum   = 0;
  pArea->max_vnum   = 0;
  pArea->area_flags = 0;
  pArea->version    = 0;
  pArea->align      = AREA_ALIGN_ALL;
  pArea->reset_rate = 0;

  for ( ; ; )
    {
      word   = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

    switch ( UPPER(word[0]) )
    {
    case 'A':
      KEY( "Align", pArea->align, fread_number( fp ) );
      break;
    case 'N':
      SKEY( "Name", pArea->name );
      break;
    case 'S':
      KEY( "Security", pArea->security, fread_number( fp ) );
      break;
    case 'V':
          KEY( "Vers",    pArea->version, fread_number( fp ) );
      
      if ( !str_cmp( word, "VNUMs" ) )
            {
          pArea->min_vnum = fread_number( fp );
          pArea->max_vnum = fread_number( fp );
            }
      break;
    case 'R':
      if ( !str_cmp( word, "Range" ) )
            {
          pArea->low_range = fread_number( fp );
          pArea->high_range = fread_number( fp );
            }
      if ( !str_cmp( word, "Reset_rate" ) )
          pArea->reset_rate = fread_number( fp );

      break;
    case 'E':
      if ( !str_cmp( word, "End" ) )
        {
          fMatch = TRUE;
          if ( area_first == NULL )
        area_first = pArea;
          if ( area_last  != NULL )
        area_last->next = pArea;
          area_last   = pArea;
          pArea->next = NULL;
          top_area++;
          return;
            }
      break;
    case 'B':
      SKEY( "Builders", pArea->builders );
      break;
    case 'C':
      KEY( "Continent", pArea->continent, fread_number( fp ) );
      SKEY( "Credits", pArea->credits );
      parse_credits(pArea);
      break;
    case 'F':
        KEY( "Flags", pArea->flags, fread_flag(fp));
        break;
         }
    }

  if (fMatch) // compile warning removal trick
    pArea->age = 15;

  return;
}

/*
 * Sets vnum range for area using OLC protection features.
 */
void assign_area_vnum( int vnum )
{
  if ( area_last->min_vnum == 0 || area_last->max_vnum == 0 )
    area_last->min_vnum = area_last->max_vnum = vnum;
  if ( vnum != URANGE( area_last->min_vnum, vnum, area_last->max_vnum ) ) {
    if ( vnum < area_last->min_vnum )
      area_last->min_vnum = vnum;
    else
      area_last->max_vnum = vnum;
  }
  return;
}

/*
 * Snarf a help section.
 */
void load_helps( FILE *fp )
{
    int number=1;
  HELP_DATA *pHelp;

  for ( ; ; )
    {
      pHelp        = alloc_perm( sizeof(*pHelp) );
      pHelp->level    = fread_number( fp );
      pHelp->keyword    = fread_string( fp );
      pHelp->vnum    = number++;
      if ( pHelp->keyword[0] == '$' )
    break;
      pHelp->text    = fread_string( fp );

      if ( !str_cmp( pHelp->keyword, "greeting" ) )
        help_greeting = pHelp->text;

      if ( help_first == NULL )
        help_first = pHelp;
      if ( help_last  != NULL )
        help_last->next = pHelp;

      help_last    = pHelp;
      pHelp->next    = NULL;
      top_help++;
    }

  return;
}
/*
 * Snarf a help section.
 */
void load_helps_1( FILE *fp )
{
  int number=1;
  HELP_DATA *pHelp;

  for ( ; ; )
    {
      pHelp        = alloc_perm( sizeof(*pHelp) );
      pHelp->level    = fread_number( fp );
      pHelp->keyword    = fread_string( fp );
      pHelp->vnum    = number++;
      if ( pHelp->keyword[0] == '$' )
    break;
      pHelp->synopsis    = fread_string( fp );
      pHelp->text    = fread_string( fp );

      if ( !str_cmp( pHelp->keyword, "greeting" ) )
    help_greeting = pHelp->text;

      if ( help_first == NULL )
    help_first = pHelp;
      if ( help_last  != NULL )
    help_last->next = pHelp;

      help_last    = pHelp;
      pHelp->next    = NULL;
      top_help++;
    }

  return;
}



/*
 * Snarf a mob section.  old style 
 */
void load_old_mob( FILE *fp )
{
  MOB_INDEX_DATA *pMobIndex;
  /* for race updating */
  int race;
  char name[MAX_STRING_LENGTH];

  if ( !area_last )   /* OLC */
    {
      bug( "Load_mobiles: no #AREA seen yet.", 0 );
      /*    alive_bug("Load_mobiles: no #AREA seen yet.");*/
      return;
      /*
      exit( 1 ); */
    }
  printf("loading old mobs\n\r");
  for ( ; ; )
    {
      sh_int vnum;
      char letter;
      int iHash;

      letter                = fread_letter( fp );
      if ( letter != '#' )
    {
      bug( "Load_mobiles: # not found.", 0 );
      exit( 1 );
    }

      vnum                = fread_number( fp );
      if ( vnum == 0 )
    break;

      fBootDb = FALSE;
      if ( get_mob_index( vnum ) != NULL )
    {
      bug( "Load_mobiles: vnum %d duplicated.", vnum );
      exit( 1 );
    }
      fBootDb = TRUE;

      pMobIndex            = alloc_perm( sizeof(*pMobIndex) );
      pMobIndex->vnum            = vnum;
      pMobIndex->area                 = area_last;               /* OLC */
      pMobIndex->new_format        = FALSE;
      pMobIndex->player_name        = fread_string( fp );
      pMobIndex->short_descr        = fread_string( fp );
      pMobIndex->long_descr        = fread_string( fp );
      pMobIndex->description        = fread_string( fp );

      pMobIndex->long_descr[0]    = UPPER(pMobIndex->long_descr[0]);
      pMobIndex->description[0]    = UPPER(pMobIndex->description[0]);

      pMobIndex->act            = fread_flag( fp ) | ACT_IS_NPC;
      pMobIndex->affected_by        = fread_flag( fp );
      pMobIndex->pShop        = NULL;
      pMobIndex->alignment        = fread_number( fp );
      letter                = fread_letter( fp );
      pMobIndex->level        = fread_number( fp );

      /*
       * The unused stuff is for imps who want to use the old-style
       * stats-in-files method.
       */
      fread_number( fp );    /* Unused */
      fread_number( fp );    /* Unused */
      fread_number( fp );    /* Unused */
      /* 'd'        */          fread_letter( fp );    /* Unused */
      fread_number( fp );    /* Unused */
      /* '+'        */          fread_letter( fp );    /* Unused */
      fread_number( fp );    /* Unused */
      fread_number( fp );    /* Unused */
      /* 'd'        */          fread_letter( fp );    /* Unused */
      fread_number( fp );    /* Unused */
      /* '+'        */          fread_letter( fp );    /* Unused */
      fread_number( fp );    /* Unused */
      pMobIndex->wealth               = fread_number( fp )/20;    
      /* xp can't be used! */          fread_number( fp );    /* Unused */
      pMobIndex->start_pos        = fread_number( fp );    /* Unused */
      pMobIndex->default_pos        = fread_number( fp );    /* Unused */

      if (pMobIndex->start_pos < POS_SLEEPING)
    pMobIndex->start_pos = POS_SLEEPING;
      if (pMobIndex->default_pos < POS_SLEEPING)
    pMobIndex->default_pos = POS_DEAD;

      /*
       * Back to meaningful values.
       */
      pMobIndex->sex            = fread_number( fp );

      /* compute the race BS */
      one_argument(pMobIndex->player_name,name);
 
      if (name[0] == '\0' || (race =  race_lookup(name)) == 0)
       {
      /* fill in with blanks */
      pMobIndex->race = race_lookup("human");
      pMobIndex->off_flags = OFF_DODGE|OFF_DISARM|OFF_TRIP|ASSIST_VNUM;
      pMobIndex->imm_flags = 0;
      pMobIndex->res_flags = 0;
      pMobIndex->vuln_flags = 0;
      pMobIndex->form = FORM_EDIBLE|FORM_SENTIENT|FORM_BIPED|FORM_MAMMAL;
      pMobIndex->parts = PART_HEAD|PART_ARMS|PART_LEGS|PART_HEART|
        PART_BRAINS|PART_GUTS;
        }
      else
        {
      pMobIndex->race = race;
      pMobIndex->off_flags = OFF_DODGE|OFF_DISARM|OFF_TRIP|ASSIST_RACE|
        race_table[race].off;
      pMobIndex->imm_flags = race_table[race].imm;
      pMobIndex->res_flags = race_table[race].res;
      pMobIndex->vuln_flags = race_table[race].vuln;
      pMobIndex->form = race_table[race].form;
      pMobIndex->parts = race_table[race].parts;
        }

      if ( letter != 'S' )
    {
      bug( "Load_mobiles: vnum %d non-S.", vnum );
      exit( 1 );
    }

      convert_mobile( pMobIndex );                           /* ROM OLC */

      iHash            = vnum % MAX_KEY_HASH;
      pMobIndex->next        = mob_index_hash[iHash];
      mob_index_hash[iHash]    = pMobIndex;
      top_mob_index++;
      top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;  /* OLC */
      assign_area_vnum( vnum );                                  /* OLC */
      kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }

  return;
}

/*
 * Snarf an obj section.  old style 
 */
void load_old_obj( FILE *fp )
{
  OBJ_INDEX_DATA *pObjIndex;

  if ( !area_last )   /* OLC */
    {
      bug( "Load_objects: no #AREA seen yet.", 0 );
      exit( 1 );
    }

  for ( ; ; )
    {
      sh_int vnum;
      char letter;
      int iHash;

      letter                = fread_letter( fp );
      if ( letter != '#' )
    {
      bug( "Load_objects: # not found.", 0 );
      exit( 1 );
    }

      vnum                = fread_number( fp );
      if ( vnum == 0 )
    break;

      fBootDb = FALSE;
      if ( get_obj_index( vnum ) != NULL )
    {
      bug( "Load_objects: vnum %d duplicated.", vnum );
      exit( 1 );
    }
      fBootDb = TRUE;

      pObjIndex            = alloc_perm( sizeof(*pObjIndex) );
      pObjIndex->vnum            = vnum;
      pObjIndex->area                 = area_last;            /* OLC */
      pObjIndex->new_format        = FALSE;
      pObjIndex->reset_num         = 0;
      pObjIndex->name            = fread_string( fp );
      pObjIndex->short_descr        = fread_string( fp );
      pObjIndex->description        = fread_string( fp );
      /* Action description */      fread_string( fp );

      pObjIndex->short_descr[0]    = LOWER(pObjIndex->short_descr[0]);
      pObjIndex->description[0]    = UPPER(pObjIndex->description[0]);
      pObjIndex->material        = str_dup("", pObjIndex->material);

      pObjIndex->item_type        = fread_number( fp );
      pObjIndex->extra_flags        = fread_flag( fp );
      pObjIndex->wear_flags        = fread_flag( fp );
      pObjIndex->value[0]        = fread_number( fp );
      pObjIndex->value[1]        = fread_number( fp );
      pObjIndex->value[2]        = fread_number( fp );
      pObjIndex->value[3]        = fread_number( fp );
      pObjIndex->value[4]        = 0;
      pObjIndex->level        = 0;
      pObjIndex->condition         = 100;
      pObjIndex->weight        = fread_number( fp );
      pObjIndex->cost            = fread_number( fp );    /* Unused */
      /* Cost per day */          fread_number( fp );


      if (pObjIndex->item_type == ITEM_WEAPON)
    {
      if (is_name("two",pObjIndex->name) 
          ||  is_name("two-handed",pObjIndex->name) 
          ||  is_name("claymore",pObjIndex->name))
        SET_BIT(pObjIndex->value[4],WEAPON_TWO_HANDS);
    }

      for ( ; ; )
    {
      char letter;

      letter = fread_letter( fp );

      if ( letter == 'A' )
        {
          AFFECT_DATA *paf;

          paf            = new_affect();
          paf->where        = TO_OBJECT;
          paf->type        = -1;
          paf->level        = 20; /* RT temp fix */
          paf->duration        = -1;
          paf->location        = fread_number( fp );
          paf->modifier        = fread_number( fp );
          paf->bitvector        = 0;
          paf->next        = pObjIndex->affected;
          pObjIndex->affected    = paf;
          top_affect++;
        }

      else if ( letter == 'E' )
        {
          EXTRA_DESCR_DATA *ed;

          ed            = new_extra_descr();
          ed->keyword        = fread_string( fp );
          ed->description        = fread_string( fp );
          ed->next        = pObjIndex->extra_descr;
        ed->valid = TRUE;
          pObjIndex->extra_descr    = ed;
          top_ed++;
        }

      else
        {
          ungetc( letter, fp );
          break;
        }
    }

      /* fix armors */
      if (pObjIndex->item_type == ITEM_ARMOR)
        {
      pObjIndex->value[1] = pObjIndex->value[0];
      pObjIndex->value[2] = pObjIndex->value[1];
        }

      /*
       * Translate spell "slot numbers" to internal "skill numbers."
       */
      switch ( pObjIndex->item_type )
    {
    case ITEM_PILL:
    case ITEM_POTION:
    case ITEM_SPELLBOOK:
    case ITEM_SCROLL:
      pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
      pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
      pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
      pObjIndex->value[4] = slot_lookup( pObjIndex->value[4] );
      break;

    case ITEM_STAFF:
    case ITEM_WAND:
    case ITEM_PROJECTILE:
      pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
      break;
    }

      iHash            = vnum % MAX_KEY_HASH;
      pObjIndex->next        = obj_index_hash[iHash];
      obj_index_hash[iHash]    = pObjIndex;
      top_obj_index++;
      top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;   /* OLC */
      assign_area_vnum( vnum );                                   /* OLC */
    }

  return;
}

/*
 * Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c
 */
void new_reset( ROOM_INDEX_DATA *pR, RESET_DATA *pReset )
{
  RESET_DATA *pr;

  if ( !pR )
    return;

  pr = pR->reset_last;

  if ( !pr )
    {
      pR->reset_first = pReset;
      pR->reset_last  = pReset;
    }
  else
    {
      pR->reset_last->next = pReset;
      pR->reset_last       = pReset;
      pR->reset_last->next = NULL;
    }

  top_reset++;
  return;
}

/*
 * Snarf a reset section.
 */
void load_resets( FILE *fp )
{
  RESET_DATA *pReset;
  int         iLastRoom = 0;
  int         iLastObj  = 0;

  if ( !area_last )
    {
      bug( "Load_resets: no #AREA seen yet.", 0 );
      exit( 1 );
    }

  for ( ; ; )
    {
      ROOM_INDEX_DATA *pRoomIndex;
      EXIT_DATA *pexit;
      char letter;
      OBJ_INDEX_DATA *temp_index;

      if ( ( letter = fread_letter( fp ) ) == 'S' )
    break;

      if ( letter == '*' )
    {
      fread_to_eol( fp );
      continue;
    }

      pReset        = alloc_perm( sizeof(*pReset) );
      pReset->command    = letter;
      /* if_flag */      fread_number( fp );
      pReset->arg1    = fread_number( fp );
      pReset->arg2    = fread_number( fp );
      pReset->arg3    = (letter == 'G' || letter == 'R')
    ? 0 : fread_number( fp );
      pReset->arg4    = (letter == 'P' || letter == 'M')
    ? fread_number(fp) : 0;
      fread_to_eol( fp );

      /*
       * Validate parameters.
       * We're calling the index functions for the side effect.
       */
      switch ( letter )
    {
    default:
      bug( "Load_resets: bad command '%c'.", letter );
      exit( 1 );
      break;

    case 'M':
      get_mob_index  ( pReset->arg1 );
      if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) )
            {
          new_reset( pRoomIndex, pReset );
          iLastRoom = pReset->arg3;
            }
      break;

    case 'O':
      temp_index = get_obj_index  ( pReset->arg1 );
      temp_index->reset_num++;
      if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) )
            {
          new_reset( pRoomIndex, pReset );
          iLastObj = pReset->arg3;
            }
      break;

    case 'P':
      temp_index = get_obj_index  ( pReset->arg1 );
      temp_index->reset_num++;
      if ( ( pRoomIndex = get_room_index ( iLastObj ) ) )
            {
          new_reset( pRoomIndex, pReset );
            }
      break;

    case 'G':
    case 'E':
      temp_index = get_obj_index  ( pReset->arg1 );
      temp_index->reset_num++;
      if ( ( pRoomIndex = get_room_index ( iLastRoom ) ) )
            {
          new_reset( pRoomIndex, pReset );
          iLastObj = iLastRoom;
            }
      break;

    /*case 'F':
      pRoomIndex = get_room_index( pReset->arg1 );

       if (pReset->arg2 < 0
           || pReset->arg2 > (MAX_DIR - 1)
           || !pRoomIndex
           || !( pexit = pRoomIndex->exit[pReset->arg2] )
           || !IS_SET( pexit->rs_flags, EX_FENCED ) )
       {
           bug( "Load_resets: 'F': exit %d not fenced.", pReset->arg2 );
           exit( 1 );
       }
           break;*/       
    
    case 'D':
      pRoomIndex = get_room_index( pReset->arg1 );

      if ( pReset->arg2 < 0
           ||   pReset->arg2 > (MAX_DIR - 1)
           || !pRoomIndex
           || !( pexit = pRoomIndex->exit[pReset->arg2] )
           || !IS_SET( pexit->rs_flags, EX_ISDOOR ))
        {
          bug( "Load_resets: 'D': exit %d not door.", pReset->arg2 );
          exit( 1 );
        }

      switch ( pReset->arg3 )
            {
        default:
          bug( "Load_resets: 'D': bad 'locks': %d." , pReset->arg3);
        case 0: break;
        case 1: 
          SET_BIT( pexit->rs_flags, EX_CLOSED );
          SET_BIT( pexit->exit_info, EX_CLOSED ); 
          break;
        case 2: 
          SET_BIT( pexit->rs_flags, EX_CLOSED | EX_LOCKED );
          SET_BIT( pexit->exit_info, EX_CLOSED | EX_LOCKED ); 
          break;
        case 3:
          SET_BIT( pexit->rs_flags, EX_CLOSED | EX_LOCKED | EX_HIDDEN);
          SET_BIT( pexit->exit_info, EX_CLOSED | EX_LOCKED | EX_HIDDEN);
            break;
            }

      break;

    case 'R':
      pRoomIndex        = get_room_index( pReset->arg1 );

      if ( pReset->arg2 < 0 || pReset->arg2 > MAX_DIR )
        {
          bug( "Load_resets: 'R': bad exit %d.", pReset->arg2 );
          exit( 1 );
        }

      if ( pRoomIndex )
        new_reset( pRoomIndex, pReset );

      break;
    }

    }

  return;
}

/*
 * Snarf a room section.
 */
void load_rooms( FILE *fp )
{
  ROOM_INDEX_DATA *pRoomIndex;

  if ( area_last == NULL )
    {
      bug( "Load_resets: no #AREA seen yet.", 0 );
      exit( 1 );
    }

  for ( ; ; )
    {
      sh_int vnum;
      char letter;
      int door,fish;
      int iHash;

      letter                = fread_letter( fp );
      if ( letter != '#' )
    {
      bug( "Load_rooms: # not found.", 0 );
      exit( 1 );
    }

      vnum                = fread_number( fp );
      if ( vnum == 0 )
    break;

      fBootDb = FALSE;
      if ( get_room_index( vnum ) != NULL )
    {
      bug( "Load_rooms: vnum %d duplicated.", vnum );
      exit( 1 );
    }
      fBootDb = TRUE;

      pRoomIndex                = alloc_perm( sizeof(*pRoomIndex) );
      pRoomIndex->owner            = str_dup("",pRoomIndex->owner);
      pRoomIndex->people        = NULL;
      pRoomIndex->contents        = NULL;
      pRoomIndex->extra_descr    = NULL;
      pRoomIndex->area          = area_last;
      pRoomIndex->vnum            = vnum;
      pRoomIndex->name            = fread_string( fp );
      pRoomIndex->description    = fread_string( fp );

      if ( !check_str_end( pRoomIndex->description ) )
      {
        char tstr[MSL];
        bugf( "R[%5d] description doesn't end with \\n\\r", vnum );

// So let's fucking fix it! - Taeloch
        mprintf(sizeof(tstr),tstr, "%s\n\r", pRoomIndex->description);
        free_string(pRoomIndex->description);
        pRoomIndex->description = str_dup(tstr, pRoomIndex->description);
      }

      /* Area number unused in rom*/          fread_number( fp );
      pRoomIndex->room_flags        = fread_flag( fp );
      /* horrible hack to save Midgaard */
      if ( 3000 <= vnum && vnum < 3400)
        SET_BIT(pRoomIndex->room_flags,ROOM_LAW);

      pRoomIndex->sector_type        = fread_number( fp );
      pRoomIndex->light        = 0;

      for ( fish = 0; fish <= 7; fish++ )
        pRoomIndex->fish[fish] = 0;

      for ( door = 0; door <= 5; door++ )
    pRoomIndex->exit[door] = NULL;


      /* defaults */
      pRoomIndex->heal_rate = 100;
      pRoomIndex->mana_rate = 100;
        /* added by Merak 2006-08-31 */
      pRoomIndex->mprog_delay =  0;
      pRoomIndex->state  =      -1;

      for ( ; ; )
    {
      letter = fread_letter( fp );

      if ( letter == 'S' )
        break;

      if ( letter == 'H') /* healing room */
        pRoomIndex->heal_rate = fread_number(fp);
    
      else if ( letter == 'M') /* mana room */
        pRoomIndex->mana_rate = fread_number(fp);

      else if ( letter == 'C') /* clan */
        {
          if ( pRoomIndex->clan )
            {
            bug("Load_rooms: duplicate clan fields.",0);
            exit(1);
          }

          pRoomIndex->clan      = clan_lookup(fread_string(fp));
          free_string( pRoomIndex->clan_name );
          pRoomIndex->clan_name = str_dup( clan_table[ pRoomIndex->clan ].name, pRoomIndex->clan_name );
        }

        //else if ( !pRoomIndex->clan_name )
        else if ( letter == 'L' ) /* New Clan System */
        {
            free_string( pRoomIndex->clan_name );
            pRoomIndex->clan_name   = fread_string( fp );
        }
      else if ( letter == 'D' )
      {
          EXIT_DATA *pexit;

          door = fread_number( fp );
          if ( door < 0 || door > 5 )
        {
          bug( "Fread_rooms: vnum %d has bad door number.", vnum );
          exit( 1 );
        }

          pexit            = alloc_perm( sizeof(*pexit) );
          pexit->description    = fread_string( fp );
          pexit->keyword        = fread_string( fp );
          pexit->exit_info        = fread_number( fp );
          pexit->rs_flags       = pexit->exit_info;
          pexit->key        = fread_number( fp );
          pexit->u1.vnum    = fread_number( fp );
          pexit->orig_door    = door;            /* OLC */

          pRoomIndex->exit[door]    = pexit;
          pRoomIndex->old_exit[door] = pexit;
          top_exit++;
        }
      else if ( letter == 'E' )
        {
          EXTRA_DESCR_DATA *ed;

          ed            = new_extra_descr();
          ed->keyword        = fread_string( fp );
          ed->description        = fread_string( fp );
          if ( !check_str_end( ed->description ) )
        bugf( "R[%5d] extra_descr doesn't end with \\n\\r", vnum );
          ed->next        = pRoomIndex->extra_descr;
          pRoomIndex->extra_descr    = ed;
          top_ed++;
        }

      else if (letter == 'O')
        {
          if (pRoomIndex->owner[0] != '\0')
        {
          bug("Load_rooms: duplicate owner.",0);
          exit(1);
        }

          pRoomIndex->owner = fread_string(fp);
        }

      else if( letter == 'P' ) /* M is used already, so I use P */
      {
        MPROG_LIST *prog, *proglist;
        char *word;
        int64 trigger = 0;

        prog    = alloc_perm(sizeof(*prog));
        word    = fread_word( fp );
        if( !(trigger = flag_lookup( word, mprog_flags )) )
        {
          bug("ROOMProgs: Invalid trigger.", 0);
          exit( 1 );
        }
        SET_BIT( pRoomIndex->mprog_flags, trigger );
        prog->trig_type     = trigger;
        prog->vnum          = fread_number( fp );
        prog->trig_phrase   = fread_string( fp );
        prog->next          = NULL;
        if ( pRoomIndex->mprogs == NULL )
          pRoomIndex->mprogs = prog;
        else
        {
          proglist = pRoomIndex->mprogs;
          while( proglist->next != NULL )
            proglist = proglist->next;
          proglist->next = prog;
        }

      }

      else if( letter == 'F' ) /* custom fish VNUMs */
      {
        for ( fish = 0; fish <= 7; fish++ )
          pRoomIndex->fish[fish] = fread_number( fp );
      }

      else
        {
          bug( "Load_rooms: vnum %d has flag not 'DES'.", vnum );
          exit( 1 );
        }
    }

      iHash            = vnum % MAX_KEY_HASH;
      pRoomIndex->next    = room_index_hash[iHash];
      room_index_hash[iHash]    = pRoomIndex;
      top_room++;
      top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
      assign_area_vnum( vnum );                                    /* OLC */
    }

  return;
}



/*
 * Snarf a shop section.
 */
void load_shops( FILE *fp )
{
  SHOP_DATA *pShop;

  for ( ; ; )
    {
      MOB_INDEX_DATA *pMobIndex;
      int iTrade;

      pShop            = alloc_perm( sizeof(*pShop) );
      pShop->keeper        = fread_number( fp );
      if ( pShop->keeper == 0 )
    break;
      for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
    pShop->buy_type[iTrade]    = fread_number( fp );
      pShop->profit_buy    = fread_number( fp );
      pShop->profit_sell    = fread_number( fp );
      pShop->open_hour    = fread_number( fp );
      pShop->close_hour    = fread_number( fp );
      fread_to_eol( fp );
      pMobIndex        = get_mob_index( pShop->keeper );
      pMobIndex->pShop    = pShop;

      if ( shop_first == NULL )
    shop_first = pShop;
      if ( shop_last  != NULL )
    shop_last->next = pShop;

      shop_last    = pShop;
      pShop->next    = NULL;
      top_shop++;
    }

  return;
}


/*
 * Load the bank information (economy info)
 * By Maniac from Mythran Mud
 */
void load_bank( void )
{
  FILE *fp;
  int   number = 0;

  if ( !( fp = fopen( BANK_FILE, "r" ) ) )
    return;
  nFilesOpen++;
  for ( ; ; )
    {
      char *word;
      char  letter;

      do
        {
      letter = getc( fp );
      if ( feof( fp ) )
            {
          fclose( fp );
              nFilesOpen--;
          return;
            }
        }
      while ( isspace( letter ) );
      ungetc( letter, fp );

      word = fread_word( fp );

      if ( !str_cmp( word, "SHARE_VALUE" ) )
        {
      number = fread_number( fp );
      if ( number > 0 )
        share_value = number;
        }
    }
}

/*
 * Snarf spec proc declarations.
 */
void load_specials( FILE *fp )
{
  for ( ; ; )
    {
      MOB_INDEX_DATA *pMobIndex;
      char letter;

      switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_specials: letter '%c' not *MS.", letter );
      exit( 1 );

    case 'S':
      return;

    case '*':
      break;

    case 'M':
      pMobIndex        = get_mob_index    ( fread_number ( fp ) );
      pMobIndex->spec_fun    = spec_lookup    ( fread_word   ( fp ) );
      if ( pMobIndex->spec_fun == 0 )
        {
          bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
          exit( 1 );
        }
      break;
    }

      fread_to_eol( fp );
    }
}


/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
  extern const sh_int rev_dir [];
  char buf[MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *pRoomIndex;
  ROOM_INDEX_DATA *to_room;
  OBJ_INDEX_DATA *pobj;
  EXIT_DATA *pexit;
  EXIT_DATA *pexit_rev;
  int iHash;
  int door;

  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
      for ( pRoomIndex  = room_index_hash[iHash];
        pRoomIndex != NULL;
        pRoomIndex  = pRoomIndex->next )
    {
      bool fexit;

      fexit = FALSE;
      for ( door = 0; door <= 5; door++ )
        {
        if ( ( pexit = pRoomIndex->exit[door] ) != NULL )
        {
            if ( pexit->key == 0 )
            pexit->key = -1;
            if ( !IS_SET( pexit->rs_flags, EX_ISDOOR )
            &&   IS_SET( pexit->rs_flags, EX_CLOSED ) )
            bugf( "Fix_exits: %5d:%s !door closed",
                pRoomIndex->vnum, dir_abbr[door] );
            if ( IS_SET( pexit->rs_flags, EX_LOCKED )
            &&   !IS_SET( pexit->rs_flags, EX_CLOSED ) )
            bugf( "Fix_exits: %5d:%s locked !closed",
                pRoomIndex->vnum, dir_abbr[door] );
            if ( pexit->u1.vnum <= 0
            ||   get_room_index( pexit->u1.vnum ) == NULL )
            pexit->u1.to_room = NULL;
            else
            {
            fexit = TRUE;
            pexit->u1.to_room = get_room_index( pexit->u1.vnum );
            if ( IS_SET( pexit->rs_flags, EX_LOCKED )
            &&   pexit->key == -1 )
                bugf( "Fix_exits: %5d:%s no_key",
                pRoomIndex->vnum, dir_abbr[door] );
            else if ( IS_SET( pexit->rs_flags, EX_LOCKED )) {
              pobj = get_obj_index(pexit->key);
              if (pobj == NULL)
                bugf( "Fix_exits: %5d:%s no item %d",
                pRoomIndex->vnum, dir_abbr[door],pexit->key );
              else 
                if (pobj->item_type != ITEM_KEY)
                  bugf( "Fix_exits: %5d:%s key is not %d ITEM_KEY",
                    pRoomIndex->vnum, dir_abbr[door], pexit->key );
            }
            }
        }
        }
      if (!fexit)
        SET_BIT(pRoomIndex->room_flags,ROOM_NO_MOB);
    }
    }

  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
      for ( pRoomIndex  = room_index_hash[iHash];
        pRoomIndex != NULL;
        pRoomIndex  = pRoomIndex->next )
    {
      for ( door = 0; door <= 5; door++ )
        {
          if ( ( pexit     = pRoomIndex->exit[door]       ) != NULL
           &&   ( to_room   = pexit->u1.to_room            ) != NULL
           &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
           &&   pexit_rev->u1.to_room != pRoomIndex 
           &&   (pRoomIndex->vnum < 1200 || pRoomIndex->vnum > 1299))
        {
          mprintf( sizeof(buf), buf, "Fix_exits: %d:%s -> %d:%s -> %d.",
               pRoomIndex->vnum, dir_abbr[door],
               to_room->vnum,    dir_abbr[rev_dir[door]],
               (pexit_rev->u1.to_room == NULL)
               ? 0 : pexit_rev->u1.to_room->vnum );
          bug( buf, 0 );
        }
        }
    }
    }

  return;
}

/*
 * Load mobprogs section
 */
void load_mobprogs( FILE *fp )
{
  MPROG_CODE *pMprog;

  if ( area_last == NULL )
    {
      bug( "Load_mobprogs: no #AREA seen yet.", 0 );
      exit( 1 );
    }

  for ( ; ; )
    {
      sh_int vnum;
      char letter;

      letter          = fread_letter( fp );
      if ( letter != '#' )
    {
      bug( "Load_mobprogs: # not found.", 0 );
      exit( 1 );
    }

      vnum         = fread_number( fp );
      if ( vnum == 0 )
    break;

      fBootDb = FALSE;
      if ( get_mprog_index( vnum ) != NULL )
    {
      bug( "Load_mobprogs: vnum %d duplicated.", vnum );
      exit( 1 );
    }
      fBootDb = TRUE;

      pMprog        = alloc_perm( sizeof(*pMprog) );
      pMprog->vnum      = vnum;

     // if ( area_last->version >= 1 )
        pMprog->name            = fread_string( fp );

      pMprog->code      = fread_string( fp );
      if ( mprog_list == NULL )
    mprog_list = pMprog;
      else
    {
      pMprog->next = mprog_list;
      mprog_list     = pMprog;
    }
      top_mprog_index++;
    }
  return;
}

/*
 *  Translate mobprog vnums pointers to real code
 */
void fix_mobprogs( void )
{
  MOB_INDEX_DATA *pMobIndex;
  MPROG_LIST        *list;
  MPROG_CODE        *code;
  int iHash;

  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
      for ( pMobIndex   = mob_index_hash[iHash];
        pMobIndex   != NULL;
        pMobIndex   = pMobIndex->next )
    {
      for( list = pMobIndex->mprogs; list != NULL; list = list->next )
        {
          if ( ( code = get_mprog_index( list->vnum ) ) != NULL )
        list->code = code;
          else
        {
          bug( "Fix_mobprogs: code vnum %d not found.", list->vnum );
         // exit( 1 );
        }
        }
    }
    }
}

void fix_roomprogs( void )
{
  ROOM_INDEX_DATA   *room;
  MPROG_LIST        *list;
  MPROG_CODE        *code;
  int                iHash;

  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
  {
    for ( room = room_index_hash[iHash]; room; room = room->next )
    {
      for ( list = room->mprogs; list; list = list->next )
      {
        if ( ( code = get_mprog_index( list->vnum ) ) )
          list->code = code;
        else
        {
          bug("Fix_roomprogs: code vnum %d not found.", list->vnum );
          exit( 1 );
        }
      }
    }
  }
}

void fix_objprogs( void )
{

  OBJ_INDEX_DATA    *obj;
  MPROG_LIST        *list;
  MPROG_CODE        *code;
  int                iHash;

  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
  {
    for ( obj = obj_index_hash[iHash]; obj; obj = obj->next )
    {
      for ( list = obj->mprogs; list; list = list->next )
      {
        if ( ( code = get_mprog_index( list->vnum ) ) )
          list->code = code;
        else
        {
          bug("Fix_objprogs: code vnum %d not found.", list->vnum );
          exit( 1 );
        }
      }
    }
  } 
}
/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
  AREA_DATA *pArea;
  char buf[MAX_STRING_LENGTH];
  int areamorts = 0;
  CHAR_DATA *fch;
  bool resetarea = FALSE;

  for ( pArea = area_first; pArea; pArea = pArea->next )
  {

    if ( ++pArea->age < 3 )
      continue;

    areamorts = pArea->nplayer;
    for (fch = char_list; fch != NULL; fch = fch->next)
    {
      if ( IS_IMMORTAL(fch)
      && ( fch->in_room->area == pArea) )
        areamorts--;
    }

    if (areamorts < 0)
      areamorts = 0; // shouldn't happen, but just in case


    resetarea = FALSE;

    if (pArea->age >= 20) // absolute max age
    {
      resetarea = TRUE;
    }
    else if ( areamorts == 0 ) // if we haven't hit max, see if chars are in area
    {
      if ( pArea->reset_rate > 0 ) // if reset rate defined, see if we've reached it
      {
        if ( pArea->age >= pArea->reset_rate )
          resetarea = TRUE;
      }
      else  // if reset rate ISN'T defined, see if we've reached 10 (default)
      {
        if ( pArea->age >= 10 )
          resetarea = TRUE;
      }
    }

    /* Check age and reset */
    if (resetarea)
    {
      reset_area( pArea );
      mprintf(sizeof(buf), buf,"%s has been reset, at age %d (rate: %d).",
        pArea->name,
        pArea->age,
        pArea->reset_rate );
      wiznet(buf,NULL,NULL,WIZ_RESETS,0,0);
      pArea->age = 0;
      if (pArea->nplayer == 0) 
        pArea->empty = TRUE;

    }
  }

  return;
}

/* Taeloch empties a room of contents */
void purge_room( ROOM_INDEX_DATA *pRoom, bool override )
{
  CHAR_DATA *vnext,*victim;
  OBJ_DATA  *obj_next,*obj;

  for ( victim = pRoom->people; victim; victim = vnext )
  {
    vnext = victim->next_in_room;

    if ( IS_NPC(victim)
    && (override || !IS_SET(victim->act,ACT_NOPURGE))
    && !victim->master )
      extract_char( victim, TRUE );
  }

  for ( obj = pRoom->contents; obj; obj = obj_next )
  {
    obj_next = obj->next_content;
    if (override || !IS_OBJ_STAT( obj, ITEM_NOPURGE ) )
      extract_obj( obj );
  }

  return;
}

/* OLC
 * Reset one room.  Called by reset_area and olc.
 */
void reset_room( ROOM_INDEX_DATA *pRoom )
{
  RESET_DATA  *pReset;
  CHAR_DATA   *pMob;
  CHAR_DATA    *mob;
  OBJ_DATA    *pObj;
  CHAR_DATA   *LastMob = NULL;
  OBJ_DATA    *LastObj = NULL;
  int iExit;
  int level = 0;
  bool last;

  if ( !pRoom )
    return;

  pMob        = NULL;
  last        = FALSE;

    
  if ( !IS_SET( pRoom->room_flags, ROOM_SHIP ) )
  {
    for ( iExit = 0;  iExit < MAX_DIR;  iExit++ )
    {
      EXIT_DATA *pExit;

      if ( ( pExit = pRoom->exit[iExit] )
      /*  && !IS_SET( pExit->exit_info, EX_BASHED )   ROM OLC */ )  
      {
        if ( pExit->u1.to_room == NULL
        ||   IS_SET( pExit->u1.to_room->room_flags, ROOM_SHIP ) )
          continue;

        pExit->exit_info = pExit->rs_flags;
        if ( ( pExit = pExit->u1.to_room->exit[rev_dir[iExit]] ) )
        {
          /* nail the other side */
          pExit->exit_info = pExit->rs_flags;
        }
      }
    }
  }

  for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
  {
      MOB_INDEX_DATA  *pMobIndex;
      OBJ_INDEX_DATA  *pObjIndex;
      OBJ_INDEX_DATA  *pObjToIndex;
      ROOM_INDEX_DATA *pRoomIndex;
      char buf[MAX_STRING_LENGTH];
      int count,limit=0;

      switch ( pReset->command )
      {
        default:
          bug( "Reset_room: bad command %c.", pReset->command );
          break;

        case 'M':
          if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) )
          {
            bug( "Reset_room: 'M': bad vnum %d.", pReset->arg1 );
            continue;
          }

          if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
          {
            bug( "Reset_area: 'R': bad vnum %d.", pReset->arg3 );
            continue;
          }
          if ( pMobIndex->count >= pMobIndex->max_count )
          {
            last = FALSE;
            break;
          } 
          count = 0;
          for ( mob = pRoomIndex->people; mob; mob = mob->next_in_room )
            if ( mob->pIndexData == pMobIndex )
            {
              count++;
              if ( count >= pReset->arg4 )
              {
                last = FALSE;
                break;
              }
            }

          if ( count >= pReset->arg4 )
            break;

          pMob = create_mobile( pMobIndex );

      /*
       * Some more hard coding.
       */
          if ( room_is_dark( pRoom ) )
            SET_BIT( pMob->affected_by, AFF_INFRARED );

      /*
       * Pet shop mobiles get ACT_PET set.
       */
          ROOM_INDEX_DATA *pRoomIndexPrev;

          pRoomIndexPrev = get_room_index( pRoom->vnum - 1 );
          if ( pRoomIndexPrev
          &&   IS_SET( pRoomIndexPrev->room_flags, ROOM_PET_SHOP ) )
            SET_BIT( pMob->act, ACT_PET);

          char_to_room( pMob, pRoom );
          pMob->reset_room = pRoom;

          /* on_load mob prog */
          if ( (pMob != NULL) && HAS_TRIGGER(pMob, TRIG_ONLOAD) )
            mp_onload_trigger(pMob);

          LastMob = pMob;
          level  = URANGE( 0, pMob->level - 2, LEVEL_HERO - 1 ); /* -1 ROM */
          last = TRUE;
          break;

        case 'O':
          if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
          {
            bug( "Reset_room: 'O' 1 : bad vnum %d", pReset->arg1 );
            mprintf (sizeof(buf), buf,"%d %d %d %d",
                     pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4 );
            bug( buf, 1 );
            continue;
          }

          if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
          {
            bug( "Reset_room: 'O' 2 : bad vnum %d.", pReset->arg3 );
            mprintf (sizeof(buf), buf,"%d %d %d %d",
                     pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4 );
            bug( buf, 1 );
            continue;
          }

          if ( pRoom->area->nplayer > 0
          ||   count_obj_list( pObjIndex, pRoom->contents ) > 0 )
          {
            last = FALSE;
            break;
          }

          pObj = create_object( pObjIndex,              /* UMIN - ROM OLC */
                UMIN( number_fuzzy( level ), LEVEL_HERO - 1 ) );
          pObj->cost = 0;
          obj_to_room( pObj, pRoom );
          last = TRUE;
          break;

        case 'P':
            if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'P': bad vnum %d.", pReset->arg1 );
                continue;
            }

            if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
            {
                bug( "Reset_room: 'P': bad vnum %d.", pReset->arg3 );
                continue;
            }

            if ( pReset->arg2 > 50 ) /* old format */
                limit = 6;
            else if ( pReset->arg2 == -1 ) /* no limit */
                limit = 999;
            else
                limit = pReset->arg2;

            if ( pRoom->area->nplayer > 0
            || ( LastObj = get_obj_type( pObjToIndex ) ) == NULL
            || ( LastObj->in_room == NULL && !last )
            || ( pObjIndex->count >= limit /* && number_range(0,4) != 0 */ )
            || ( count = count_obj_list( pObjIndex, LastObj->contains ) ) > 
                                         pReset->arg4  )
            {
                last = FALSE;
                // bug( "I'm getting here.", 0 );
                break;
            }
            /* lastObj->level  -  ROM */

            while ( count < pReset->arg4 )
            {
                pObj = create_object( pObjIndex, number_fuzzy( LastObj->level ) );
                obj_to_obj( pObj, LastObj );
                count++;
                if ( pObjIndex->count >= limit )
                break;
            }

            /* fix object lock state! */
            LastObj->value[1] = LastObj->pIndexData->value[1];
            last = TRUE;
            break;

        case 'G':
        case 'E':
          if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
          {
            bug( "Reset_room: 'E' or 'G': bad vnum %d.", pReset->arg1 );
            continue;
          }

          if ( !last )
            break;

          if ( !LastMob )
          {
            bug( "Reset_room: 'E' or 'G': null mob for vnum %d.",
                 pReset->arg1 );
            last = FALSE;
            break;
          }

          if ( LastMob->pIndexData->pShop )   /* Shop-keeper? */
          {
            int olevel=0,i,j;

            if ( !pObjIndex->new_format )
              switch ( pObjIndex->item_type )
              {
                default:                olevel = 0;                      break;
                case ITEM_PILL:
                case ITEM_POTION:
                case ITEM_SCROLL:
                case ITEM_SPELLBOOK:
                   olevel = 53;
                  for (i = 1; i < 5; i++)
                  {
                    if ( pObjIndex->value[i] > 0 )
                    {
                      for (j = 0; j < MAX_CLASS; j++)
                      {
                        olevel = UMIN( olevel, 
                          skill_table[pObjIndex->value[i]].skill_level[j] );
                      }
                    }
                  }
           
                  olevel = UMAX( 0, ( olevel * 3 / 4) - 2 );
                  break;
            
                case ITEM_WAND:         olevel = number_range( 10, 20 ); break;
                case ITEM_PROJECTILE:   olevel = number_range( 10, 20 ); break;
                case ITEM_STAFF:        olevel = number_range( 15, 25 ); break;
                case ITEM_ARMOR:        olevel = number_range(  5, 15 ); break;
            /* ROM patch weapon, treasure */
                case ITEM_WEAPON:       olevel = number_range(  5, 15 ); break;
                case ITEM_TREASURE:     olevel = number_range( 10, 20 ); break;

#if 0 /* envy version */
          case ITEM_WEAPON:       if ( pReset->command == 'G' )
            olevel = number_range( 5, 15 );
          else
            olevel = number_fuzzy( level );
#endif /* envy version */

                break;
              }

            pObj = create_object( pObjIndex, olevel );
            SET_BIT( pObj->extra_flags, ITEM_INVENTORY );  /* ROM OLC */

#if 0 /* envy version */
          if ( pReset->command == 'G' )
        SET_BIT( pObj->extra_flags, ITEM_INVENTORY );
#endif /* envy version */

        }
        else   /* ROM OLC else version */
        {
          int limit;
          if ( pReset->arg2 > 50 )  /* old format */
            limit = 6;
          else if ( pReset->arg2 == -1 || pReset->arg2 == 0 )  /* no limit */
            limit = 999;
          else
            limit = pReset->arg2;

          if ( pObjIndex->count < limit || number_range( 0 , 4 ) == 0 )
          {
            pObj = create_object( pObjIndex, 
                   UMIN( number_fuzzy( level ), LEVEL_HERO - 1 ) );
          /* error message if it is too high */
            if ( pObj->level > LastMob->level + 3
            || ( pObj->item_type == ITEM_WEAPON 
            &&   pReset->command == 'E' 
            &&   pObj->level < LastMob->level -30 && pObj->level < 45 ) )
              bugf("Err: obj %s (%d) -- %d, mob %s (%d) -- %d\n",
              pObj->short_descr,pObj->pIndexData->vnum,pObj->level,
              LastMob->short_descr,LastMob->pIndexData->vnum,LastMob->level );
          }
          else
            break;
        }
                                     
#if 0 /* envy else version */
      else
            {
          pObj = create_object( pObjIndex, number_fuzzy( level ) );
            }
#endif /* envy else version */

        obj_to_char( pObj, LastMob );
        if ( pReset->command == 'E' )
          equip_char( LastMob, pObj, pReset->arg3 );
          last = TRUE;
          break;

        case 'D':
          break;

        case 'R':
          if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) )
          {
            bug( "Reset_room: 'R': bad vnum %d.", pReset->arg1 );
            continue;
          }

          {
            EXIT_DATA *pExit;
            int d0;
            int d1;

            for ( d0 = 0; d0 < pReset->arg2 - 1; d0++ )
            {
              d1                   = number_range( d0, pReset->arg2-1 );
              pExit                = pRoomIndex->exit[d0];
              pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
              pRoomIndex->exit[d1] = pExit;
            }
          }
          break;
    }
  }
  return;
}

/* OLC
 * Reset one area.
 */
void reset_area( AREA_DATA *pArea )
{
  ROOM_INDEX_DATA *pRoom;
  int  vnum;

  for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    if ( ( pRoom = get_room_index( vnum ) ) )
      reset_room( pRoom );
  return;
}

/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex )
{
  CHAR_DATA *mob;
  int i;
  AFFECT_DATA af;

  mobile_count++;

  if ( pMobIndex == NULL )
    {
      bug( "Create_mobile: NULL pMobIndex.", 0 );
      exit( 1 );
    }

  mob = new_char();

  mob->pIndexData    = pMobIndex;

  mob->name            = str_dup( pMobIndex->player_name, mob->name );    /* OLC */
  mob->short_descr    = str_dup( pMobIndex->short_descr , mob->short_descr);    /* OLC */
  mob->long_descr    = str_dup( pMobIndex->long_descr,
                               mob->long_descr );     /* OLC */
  mob->description    = str_dup( pMobIndex->description, mob->description );    /* OLC */
  mob->path         = str_dup( pMobIndex->path, mob->path );
  mob->id            = get_mob_id();
  mob->spec_fun        = pMobIndex->spec_fun;
  mob->prompt        = NULL;
  mob->mprog_target = NULL;

  if ( pMobIndex->wealth == 0 )
  {
      mob->silver = 0;
      mob->gold   = 0;
  }
  else
  {
      long wealth;

      wealth = number_range( pMobIndex->wealth / 2, 3 * pMobIndex->wealth / 2 );
      mob->gold = number_range( wealth / 200, wealth / 100 );
      mob->silver = wealth - ( mob->gold * 100 );
  } 

  if ( pMobIndex->new_format )
    /* load in new style */
  {
      /* read from prototype */
      mob->group        = pMobIndex->group;
      mob->act             = pMobIndex->act;
      mob->act2         = pMobIndex->act2;
      mob->pen_flags    = PEN_NOCHANNELS;
      mob->chan_flags   = CHANNEL_NOSHOUT | CHANNEL_NOTELL;
      mob->affected_by    = pMobIndex->affected_by;
      mob->affected2_by = pMobIndex->affected2_by;
      mob->alignment    = pMobIndex->alignment;
      mob->level        = pMobIndex->level;
      mob->hitroll        = pMobIndex->hitroll;
      mob->frag_number  = pMobIndex->frag_number;
      mob->damroll        = pMobIndex->damage[DICE_BONUS];
      mob->max_hit        = dice(pMobIndex->hit[DICE_NUMBER],
                               pMobIndex->hit[DICE_TYPE])
                             + pMobIndex->hit[DICE_BONUS];
      mob->hit            = mob->max_hit;
      mob->max_mana        = dice(pMobIndex->mana[DICE_NUMBER],
                               pMobIndex->mana[DICE_TYPE])
                             + pMobIndex->mana[DICE_BONUS];
      mob->mana            = mob->max_mana;
      mob->damage[DICE_NUMBER]= pMobIndex->damage[DICE_NUMBER];
      mob->damage[DICE_TYPE]  = pMobIndex->damage[DICE_TYPE];
      mob->dam_type        = pMobIndex->dam_type;

      if (mob->dam_type == 0)
        switch(number_range(1,3))
        {
          case (1): mob->dam_type = 3;        break;  /* slash */
          case (2): mob->dam_type = 7;        break;  /* pound */
          case (3): mob->dam_type = 11;       break;  /* pierce */
        }
      for (i = 0; i < 4; i++)
      mob->armor[i]            = pMobIndex->ac[i]; 
      mob->off_flags        = pMobIndex->off_flags;
      mob->imm_flags        = pMobIndex->imm_flags;
      mob->res_flags        = pMobIndex->res_flags;
      mob->vuln_flags        = pMobIndex->vuln_flags;
      mob->start_pos        = pMobIndex->start_pos;
      mob->default_pos        = pMobIndex->default_pos;
      mob->sex                = pMobIndex->sex;
      if (mob->sex == 3) /* random sex */
        mob->sex            = number_range(1,2);
      mob->race                = pMobIndex->race;
      mob->form                = pMobIndex->form;
      mob->parts            = pMobIndex->parts;
      mob->size                = pMobIndex->size;
      mob->material            = str_dup(pMobIndex->material,mob->material);

      /* computed on the spot */

      for (i = 0; i < MAX_STATS; i ++)
        mob->perm_stat[i] = UMIN(25,11 + mob->level/4);
            
      if (IS_SET(mob->act,ACT_WARRIOR))
      {
        mob->perm_stat[STAT_STR] += 3;
        mob->perm_stat[STAT_INT] -= 1;
        mob->perm_stat[STAT_CON] += 2;
      }
        
      if (IS_SET(mob->act,ACT_THIEF))
      {
        mob->perm_stat[STAT_DEX] += 3;
        mob->perm_stat[STAT_INT] += 1;
        mob->perm_stat[STAT_WIS] -= 1;
      }
        
      if (IS_SET(mob->act,ACT_CLERIC))
      {
        mob->perm_stat[STAT_WIS] += 3;
        mob->perm_stat[STAT_DEX] -= 1;
        mob->perm_stat[STAT_STR] += 1;
      }
        
      if (IS_SET(mob->act,ACT_MAGE))
      {
        mob->perm_stat[STAT_INT] += 3;
        mob->perm_stat[STAT_STR] -= 1;
        mob->perm_stat[STAT_DEX] += 1;
      }
        
      if (IS_SET(mob->off_flags,OFF_FAST))
        mob->perm_stat[STAT_DEX] += 2;
            
      mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
      mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;

      /* let's get some spell action */
      if ( IS_AFFECTED( mob, AFF_SANCTUARY ) )
      {
        af.where     = TO_AFFECTS;
        af.type      = skill_lookup("sanctuary");
        af.level     = mob->level;
        af.duration  = -1;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF_SANCTUARY;
        affect_to_char( mob, &af );
      }

    if ( IS_AFFECTED2( mob, AFF2_INVUN ) )
    {
        af.where     = TO_AFFECTS2;
        af.type      = skill_lookup( "globe of invulnerability" );
        af.level     = mob->level;
        af.duration  = -1;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF2_INVUN;
        affect_to_char( mob, &af );
    }

    if ( IS_AFFECTED2( mob, AFF2_NIRVANA ) )
    {
        af.where     = TO_AFFECTS2;
        af.type      = skill_lookup( "nirvana" );
        af.level     = mob->level;
        af.duration  = -1;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF2_NIRVANA;
        affect_to_char( mob, &af );
    }

    if ( IS_AFFECTED2( mob, AFF2_FADE_OUT ) )
    {
        af.where     = TO_AFFECTS2;
        af.type      = skill_lookup( "fade out" );
        af.level     = mob->level;
        af.duration  = -1;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF2_FADE_OUT;
        affect_to_char( mob, &af );
    }

    if ( IS_AFFECTED2( mob, AFF2_RADIANT ) )
    {
        af.where     = TO_AFFECTS2;
        af.type      = skill_lookup( "radiance" );
        af.level     = mob->level;
        af.duration  = -1;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF2_RADIANT;
        affect_to_char( mob, &af );
    }

    if ( IS_AFFECTED2( mob, AFF2_SHROUD ) )
    {
        af.where     = TO_AFFECTS2;
        af.type      = skill_lookup( "malevolent shroud" );
        af.level     = mob->level;
        af.duration  = -1;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF2_SHROUD;
        affect_to_char( mob, &af );
    }

    if ( IS_AFFECTED( mob, AFF_AQUA_ALBEDO ) )
      {
        af.where     = TO_AFFECTS;
        af.type      = skill_lookup("aqua albedo");
        af.level     = mob->level;
        af.duration  = -1;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF_AQUA_ALBEDO;
        affect_to_char( mob, &af );
      }

    if ( IS_AFFECTED( mob, AFF_HASTE ) )
      {
        af.where       = TO_AFFECTS;
        af.type      = skill_lookup("haste");
        af.level     = mob->level;
        af.duration  = -1;
        af.location  = APPLY_DEX;
        af.modifier  = 1 + (mob->level >= 18) + (mob->level >= 25) + 
                           (mob->level >= 32);
        af.bitvector = AFF_HASTE;
        affect_to_char( mob, &af );
      }

      if (IS_AFFECTED(mob,AFF_PROTECT_EVIL))
    {
      af.where       = TO_AFFECTS;
      af.type       = skill_lookup("protection evil");
      af.level       = mob->level;
      af.duration  = -1;
      af.location  = APPLY_SAVES;
      af.modifier  = -1;
      af.bitvector = AFF_PROTECT_EVIL;
      affect_to_char(mob,&af);
    }

      if (IS_AFFECTED(mob,AFF_PROTECT_GOOD))
        {
      af.where       = TO_AFFECTS;
      af.type      = skill_lookup("protection good");
      af.level     = mob->level;
      af.duration  = -1;
      af.location  = APPLY_SAVES;
      af.modifier  = -1;
      af.bitvector = AFF_PROTECT_GOOD;
      affect_to_char(mob,&af);
        }
  }
  else /* read in old format and convert */
  {
      mob->act            = pMobIndex->act;
      mob->act2         = pMobIndex->act2;
      mob->affected_by    = pMobIndex->affected_by;
      mob->affected2_by = pMobIndex->affected2_by;
      mob->alignment    = pMobIndex->alignment;
      mob->level        = pMobIndex->level;
      mob->hitroll        = pMobIndex->hitroll;
      mob->frag_number  = pMobIndex->frag_number;
      mob->damroll        = 0;
      mob->max_hit        = mob->level * 8 + number_range(
                                mob->level * mob->level/4,
                                mob->level * mob->level);
      mob->max_hit     *= .9;
      mob->hit            = mob->max_hit;
      mob->max_mana        = 100 + dice(mob->level,10);
      mob->mana            = mob->max_mana;

      switch(number_range(1,3))
      {
        case (1): mob->dam_type = 3;     break;  /* slash */
        case (2): mob->dam_type = 7;    break;  /* pound */
        case (3): mob->dam_type = 11;    break;  /* pierce */
      }

      for ( i = 0; i < 3; i++ )
        mob->armor[i] = interpolate(mob->level,100,-100);

      mob->armor[3]            = interpolate(mob->level,100,0);
      mob->race                = pMobIndex->race;
      mob->off_flags        = pMobIndex->off_flags;
      mob->imm_flags        = pMobIndex->imm_flags;
      mob->res_flags        = pMobIndex->res_flags;
      mob->vuln_flags        = pMobIndex->vuln_flags;
      mob->start_pos        = pMobIndex->start_pos;
      mob->default_pos        = pMobIndex->default_pos;
      mob->sex                = pMobIndex->sex;
      mob->form                = pMobIndex->form;
      mob->parts            = pMobIndex->parts;
      mob->size                = SIZE_MEDIUM;  // Eh?? Merak didn't do it!
      mob->material            = "";

      for (i = 0; i < MAX_STATS; i ++)
        mob->perm_stat[i] = 11 + mob->level/4;
  }

  mob->position    = mob->start_pos;
  mob->group_num   = 0;
  mob->group_fight = 0;


  /* link the mob to the world list */
  mob->next        = char_list;
  char_list        = mob;
  pMobIndex->count++;
  return mob;
}

/* remove duplicate affects on pets */
bool check_pet_affected(int vnum, AFFECT_DATA *paf)
{
  MOB_INDEX_DATA *petIndex;

  petIndex = get_mob_index(vnum);
  if (petIndex == NULL)
    return FALSE;

  if (paf->where == TO_AFFECTS)
    if (IS_AFFECTED(petIndex, paf->bitvector))
      return TRUE;

  return FALSE;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mobile(CHAR_DATA *parent, CHAR_DATA *clone)
{
  int i;
  AFFECT_DATA *paf;

  if ( parent == NULL || clone == NULL || !IS_NPC(parent))
    return;
    
  /* start fixing values */ 
  clone->name             = str_dup( parent->name, clone->name );
  clone->version        = parent->version;
  clone->short_descr    = str_dup( parent->short_descr, clone->short_descr );
  clone->long_descr        = str_dup( parent->long_descr, clone->long_descr );
  clone->description    = str_dup( parent->description, clone->description );
  clone->group            = parent->group;
  clone->sex            = parent->sex;
  clone->gameclass        = parent->gameclass;
  clone->race            = parent->race;
  clone->level            = parent->level;
  clone->trust            = 0;
  clone->timer            = parent->timer;
  clone->wait            = parent->wait;
  clone->hit            = parent->hit;
  clone->max_hit        = parent->max_hit;
  clone->mana            = parent->mana;
  clone->max_mana        = parent->max_mana;
  clone->move            = parent->move;
  clone->max_move        = parent->max_move;
  clone->gold            = parent->gold;
  clone->silver            = parent->silver;
  clone->exp            = parent->exp;
  clone->act            = parent->act;
  clone->act2           = parent->act2;
  clone->comm_flags        = parent->comm_flags;
  clone->pen_flags        = parent->pen_flags;
  clone->chan_flags        = parent->chan_flags;
  clone->imm_flags        = parent->imm_flags;
  clone->res_flags        = parent->res_flags;
  clone->vuln_flags        = parent->vuln_flags;
  clone->invis_level    = parent->invis_level;
  clone->affected_by    = parent->affected_by;
  clone->affected2_by   = parent->affected2_by;
  clone->position        = parent->position;
  clone->practice        = parent->practice;
  clone->train            = parent->train;
  clone->saving_throw    = parent->saving_throw;
  clone->alignment        = parent->alignment;
  clone->hitroll        = parent->hitroll;
  clone->frag_number    = parent->frag_number;
  clone->damroll        = parent->damroll;
  clone->wimpy            = parent->wimpy;
  clone->form            = parent->form;
  clone->parts            = parent->parts;
  clone->size            = parent->size;
  clone->material        = str_dup(parent->material, clone->material);
  clone->off_flags        = parent->off_flags;
  clone->dam_type       = parent->dam_type;
  clone->start_pos        = parent->start_pos;
  clone->default_pos    = parent->default_pos;
  clone->spec_fun        = parent->spec_fun;
  clone->reset_room     = parent->reset_room;
    
  for (i = 0; i < 4; i++)
    clone->armor[i]    = parent->armor[i];

  for ( i = 0; i < MAX_STATS; i++ )
  {
      clone->perm_stat[i]    = parent->perm_stat[i];
      clone->mod_stat[i]    = parent->mod_stat[i];
  }

  for ( i = 0; i < 3; i++ )
    clone->damage[i] = parent->damage[i];

  /* now add the affects */
  for ( paf = parent->affected; paf; paf = paf->next )
    affect_to_char( clone, paf );

}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level )
{
  AFFECT_DATA *paf;
  OBJ_DATA *obj;
  int i;

  if ( pObjIndex == NULL )
    {
      bug( "Create_object: NULL pObjIndex.", 0 );
      exit( 1 );
    }

  obj = new_obj();

  obj->pIndexData    = pObjIndex;
  obj->in_room        = NULL;
  obj->enchanted    = FALSE;
  obj->mprog_target = NULL;
  obj->mprog_delay  = 0;
  if (pObjIndex->new_format)
    obj->level = pObjIndex->level;
  else
    obj->level        = URANGE(0,level, MAX_LEVEL);
  obj->wear_loc    = WEAR_NONE;

  obj->name        = str_dup( pObjIndex->name, obj->name );           /* OLC */
  obj->short_descr    = str_dup( pObjIndex->short_descr, obj->short_descr );    /* OLC */
  obj->description    = str_dup( pObjIndex->description, obj->description );    /* OLC */
  obj->material    = str_dup(pObjIndex->material, obj->material);
  obj->item_type    = pObjIndex->item_type;
  obj->extra_flags    = pObjIndex->extra_flags;
  obj->wear_flags    = pObjIndex->wear_flags;

  for ( i = 0; i <= 6; i++ )
    obj->value[i]    = pObjIndex->value[i];

  obj->weight        = pObjIndex->weight;

  if (level == -1 || pObjIndex->new_format)
    obj->cost    = pObjIndex->cost;
  else
    obj->cost    = number_fuzzy( 10 )
      * number_fuzzy( level ) * number_fuzzy( level );

  /*
   * Mess with object properties.
   */
  switch ( obj->item_type )
    {
    default:
      bugf( "Read_object: vnum %d bad type. type = %d, INTERP_CMD[%s]",
        pObjIndex->vnum, obj->item_type, interp_cmd );
      break;

    case ITEM_CHECKERS:
        if ( obj->value[0] == 0 )
        {
            obj->value[0] = A|B|C|D|E|F|G|H|I|J|K|L|U|V|W|X|Y|Z|aa|bb|cc|dd|ee|ff;
            obj->value[1] = A|B|C|D|E|F|G|H|I|J|K|L;
            obj->value[2] = 0;
        }
        break;
    case ITEM_LIGHT:
      if (obj->value[2] == 999)
    obj->value[2] = -1;
      break;

    case ITEM_SLOT_MACHINE:
      if (obj->value[2] > 5)
    obj->value[2] = 5;
      else if(obj->value[2] < 3)
    obj->value[2] = 3;
    case ITEM_FURNITURE:
    case ITEM_MONEY_POUCH:
    case ITEM_TRASH:
    case ITEM_BANDAGE:
    case ITEM_CONTAINER:
    case ITEM_DRINK_CON:
    case ITEM_KEY:
    case ITEM_FOOD:
    case ITEM_BOAT:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_FOUNTAIN:
    case ITEM_MAP:
    case ITEM_CLOTHING:
    case ITEM_PORTAL:
      if (!pObjIndex->new_format)
    obj->cost /= 5;
      break;

    case ITEM_BOOK:
    case ITEM_LOCKER:
    case ITEM_TREASURE:
    case ITEM_WARP_STONE:
    case ITEM_ROOM_KEY:
    case ITEM_GEM:
    case ITEM_JEWELRY:
    case ITEM_WHETSTONE:
		case ITEM_QUIVER:
      break;

    case ITEM_JUKEBOX:
      for (i = 0; i < 5; i++)
    obj->value[i] = -1;
      break;

    case ITEM_FISHING_ROD:
      if (level != -1 && !pObjIndex->new_format)
      {
        obj->value[0] = number_fuzzy( obj->value[0] );
        obj->value[1] = number_fuzzy( obj->value[1] );
      }
      break;

    case ITEM_SCRY_MIRROR:
      if (level != -1 && !pObjIndex->new_format)
        obj->value[0] = number_fuzzy( obj->value[0] );
      break;

    case ITEM_SCROLL:
      if (level != -1 && !pObjIndex->new_format)
    obj->value[0]    = number_fuzzy( obj->value[0] );
      break;

    case ITEM_SPELLBOOK:
      if (level != -1 && !pObjIndex->new_format) {
    obj->value[0]    = number_fuzzy( obj->value[0] );
    obj->value[1]    = 20;
    obj->value[2]    = 0;
      }
      break;

    case ITEM_KEYRING:
      if (level != -1 && !pObjIndex->new_format)
        obj->value[0]    = number_fuzzy( obj->value[0] );
      break;

    case ITEM_WAND:
    case ITEM_STAFF:
      if (level != -1 && !pObjIndex->new_format)
    {
      obj->value[0]    = number_fuzzy( obj->value[0] );
      obj->value[1]    = number_fuzzy( obj->value[1] );
      obj->value[2]    = obj->value[1];
    }
      if (!pObjIndex->new_format)
    obj->cost *= 2;
      break;

    case ITEM_WEAPON:
      if (level != -1 && !pObjIndex->new_format)
    {
      obj->value[1] = number_fuzzy( number_fuzzy( 1 * level / 4 + 2 ) );
      obj->value[2] = number_fuzzy( number_fuzzy( 3 * level / 4 + 6 ) );
    }
      break;

    case ITEM_ARMOR:
      if (level != -1 && !pObjIndex->new_format)
    {
      obj->value[0]    = number_fuzzy( level / 5 + 3 );
      obj->value[1]    = number_fuzzy( level / 5 + 3 );
      obj->value[2]    = number_fuzzy( level / 5 + 3 );
    }
      break;

    case ITEM_POTION:
    case ITEM_PILL:
      if (level != -1 && !pObjIndex->new_format)
    obj->value[0] = number_fuzzy( number_fuzzy( obj->value[0] ) );
      break;

    case ITEM_MONEY:
      if (!pObjIndex->new_format)
    obj->value[0]    = obj->cost;
      break;
    }
  
  for (paf = pObjIndex->affected; paf; paf = paf->next) 
    if ( paf->location == APPLY_SPELL_AFFECT )
      affect_to_obj(obj,paf);
  
  obj->next        = object_list;
  object_list        = obj;
  pObjIndex->count++;
  pObjNum++;
  if (!check_obj_loop(obj)) {
    print_cmd_hist();
    bugf("CRASH CRASH:Create obj. Obj Vnum = %d, Obj Name = %s: Num Objects %d %s", pObjIndex->vnum, pObjIndex->name, pObjNum, interp_cmd);
    exit(1);
  }
  return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(OBJ_DATA *parent, OBJ_DATA *clone)
{
  int i;
  AFFECT_DATA *paf;
  EXTRA_DESCR_DATA *ed,*ed_new;

  if (parent == NULL || clone == NULL)
    return;

  /* start fixing the object */
  clone->name     = str_dup(parent->name, clone->name);
  clone->short_descr     = str_dup(parent->short_descr, clone->short_descr);
  clone->description    = str_dup(parent->description, clone->description);
  clone->item_type    = parent->item_type;
  clone->extra_flags    = parent->extra_flags;
  clone->wear_flags    = parent->wear_flags;
  clone->weight    = parent->weight;
  clone->cost        = parent->cost;
  clone->level    = parent->level;
  clone->condition    = parent->condition;
  clone->material    = str_dup(parent->material, clone->material);
  clone->timer    = parent->timer;

  for (i = 0;  i < 5; i ++)
    clone->value[i]    = parent->value[i];

  /* affects */
  clone->enchanted    = parent->enchanted;
  
  for (paf = parent->affected; paf != NULL; paf = paf->next) 
    affect_to_obj(clone,paf);

  /* extended desc */
  for (ed = parent->extra_descr; ed != NULL; ed = ed->next)
    {
      ed_new                  = new_extra_descr();
      ed_new->keyword        = str_dup( ed->keyword, ed_new->keyword);
      ed_new->description     = str_dup( ed->description , ed_new->description);
      ed_new->next               = clone->extra_descr;
      clone->extra_descr      = ed_new;
    }

}



/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA *ch )
{
  static CHAR_DATA ch_zero;
  int i;

  *ch                   = ch_zero;
  ch->name              = &str_empty[0];
  ch->short_descr        = &str_empty[0];
  ch->long_descr        = &str_empty[0];
  ch->description        = &str_empty[0];
  ch->prompt                  = &str_empty[0];
#if MEMDEBUG
  ch->memdebug_name     = &str_empty[0];
  ch->memdebug_prompt   = &str_empty[0];
#endif
  ch->logon                = current_time;
  ch->lines                = PAGELEN;

  for (i = 0; i < 4; i++)
    ch->armor[i]        = 100;

  ch->position        = POS_STANDING;
  ch->hit            = 20;
  ch->max_hit        = 20;
  ch->mana            = 100;
  ch->max_mana        = 100;
  ch->move            = 100;
  ch->max_move        = 100;
  ch->on            = NULL;

  for (i = 0; i < MAX_STATS; i ++)
  {
      ch->perm_stat[i] = 13; 
      ch->mod_stat[i] = 0;
  }

  return;
}

/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
  for ( ; ed; ed = ed->next )
  {
      if ( is_name( (char *)name, ed->keyword ) )
        return ed->description;
  }
  return NULL;
}

EXTRA_DESCR_DATA *ed_lookup( char *name, EXTRA_DESCR_DATA *ed )
{
  EXTRA_DESCR_DATA *list;

  for ( list = ed; list; list = list->next )
    if ( is_name( name, list->keyword ) )
      return list;
  return NULL;
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
  MOB_INDEX_DATA *pMobIndex;

  for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
    pMobIndex != NULL;
    pMobIndex  = pMobIndex->next )
    {
      if ( pMobIndex->vnum == vnum )
    return pMobIndex;
    }

  if ( fBootDb )
    {
      bug( "Get_mob_index: bad vnum %d.", vnum );
      exit( 1 );
    }

  return NULL;
}



/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
  OBJ_INDEX_DATA *pObjIndex;
  if ( vnum < 0 )
    {
      bugf( "Get_obj_index: bad vnum %d", vnum );
      return NULL;
    }
  
  for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
    pObjIndex != NULL;
    pObjIndex  = pObjIndex->next )
    {
      if ( pObjIndex->vnum == vnum )
    return pObjIndex;
    }

  if ( fBootDb )
    {
      bug( "Get_obj_index: bad vnum %d.", vnum );
      return NULL;
    }

  return NULL;
}



/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( int vnum )
{
  ROOM_INDEX_DATA *pRoomIndex;

  for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
    pRoomIndex != NULL;
    pRoomIndex  = pRoomIndex->next )
    {
      if ( pRoomIndex->vnum == vnum )
    return pRoomIndex;
    }

  if ( fBootDb )
    {
      bug( "Get_room_index: bad vnum %d.", vnum );
      exit( 1 );
    }

  return NULL;
}

MPROG_CODE *get_mprog_index( int vnum )
{
  MPROG_CODE *prg;
  for( prg = mprog_list; prg; prg = prg->next )
    {
      if ( prg->vnum == vnum )
    return( prg );
    }
  return NULL;
}    



/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
  char c;

  do
    {
      c = getc( fp );
    }
  while ( isspace(c) );

  return c;
}



/*
 * Read a number from a file.
 */
int64 fread_number( FILE *fp )
{
  int64 number;
  bool sign;
  char c;

  do
    {
      c = getc( fp );
    }
  while ( isspace(c) );

  number = 0;

  sign   = FALSE;
  if ( c == '+' )
    {
      c = getc( fp );
    }
  else if ( c == '-' )
    {
      sign = TRUE;
      c = getc( fp );
    }

  if ( !isdigit(c) )
    {
      bug( "Fread_number: bad format.", 0 );
      exit( 1 );
    }

  while ( isdigit(c) )
    {
      number = number * 10 + c - '0';
      c      = getc( fp );
    }

  if ( sign )
    number = 0 - number;

  if ( c == '|' )
    number += fread_number( fp );
  else if ( c != ' ' )
    ungetc( c, fp );

  return number;
}

int64 fread_flag( FILE *fp)
{
  int64 number;
  char c;
  bool negative = FALSE;

  do
    {
      c = getc(fp);
    }
  while ( isspace(c));

  if (c == '-')
    {
      negative = TRUE;
      c = getc(fp);
    }

  number = 0;

  if (!isdigit(c))
    {
      while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
    {
      number += flag_convert(c);
      c = getc(fp);
    }
    }

  while (isdigit(c))
    {
      number = number * 10 + c - '0';
      c = getc(fp);
    }

  if (c == '|')
    number += fread_flag(fp);

  else if  ( c != ' ')
    ungetc(c,fp);

  if (negative)
    return -1 * number;

  return number;
}

int64 flag_convert(char letter )
{
  int64 bitsum = 0;
  char i;

  if ('A' <= letter && letter <= 'Z') 
    {
      bitsum = 1;
      for (i = letter; i > 'A'; i--)
    bitsum *= 2;
    }
  else if ('a' <= letter && letter <= 'z')
    {
      bitsum = 67108864; /* 2^26 */
      for (i = letter; i > 'a'; i --)
    bitsum *= 2;
    }

  return bitsum;
}




/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 *   this function takes 40% to 50% of boot-up time.
 */
char *fread_string( FILE *fp )
{
  char *plast;
  char c;

  plast = top_string + sizeof(char *);
  if ( plast > &string_space[MAX_STRING - MAX_STRING_LENGTH] )
    {
      bug( "Fread_string: MAX_STRING %d exceeded.", MAX_STRING );
      exit( 1 );
    }

  /*
   * Skip blanks.
   * Read first char.
   */
  do
    {
      c = getc( fp );
    }
  while ( isspace(c) );

  if ( ( *plast++ = c ) == '~' )
    return &str_empty[0];

  for ( ;; )
    {
      /*
       * Back off the char type lookup,
       *   it was too dirty for portability.
       *   -- Furey
       */

      switch ( *plast = getc(fp) )
    {
        default:
      plast++;
      break;
 
        case EOF:
      /* temp fix */
      bug( "Fread_string: EOF", 0 );
      return NULL;
      /* exit( 1 ); */
      break;
 
        case '\n':
      plast++;
      *plast++ = '\r';
      break;
 
        case '\r': 
      break;
 
        case '~':
      plast++;
      {
        union
        {
          char *    pc;
          char    rgc[sizeof(char *)];
        } u1;
        int ic;
        int iHash;
        char *pHash;
        char *pHashPrev;
        char *pString;

        plast[-1] = '\0';
        iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
        for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
          {
        for ( ic = 0; ic < sizeof(char *); ic++ )
          u1.rgc[ic] = pHash[ic];
        pHashPrev = u1.pc;
        pHash    += sizeof(char *);

        if ( top_string[sizeof(char *)] == pHash[0]
             &&   !strcmp( top_string+sizeof(char *)+1, pHash+1 ) )
          return pHash;
          }

        if ( fBootDb )
          {
        pString        = top_string;
        top_string        = plast;
        u1.pc        = string_hash[iHash];
        for ( ic = 0; ic < sizeof(char *); ic++ )
          pString[ic] = u1.rgc[ic];
        string_hash[iHash]    = pString;

        nAllocString += 1;
        sAllocString += top_string - pString;
        return pString + sizeof(char *);
          }
        else
          {
        return str_dup( top_string + sizeof(char *) , "");
          }
      }
    }
    }
}

char *fread_string_eol( FILE *fp )
{
  static bool char_special[256-EOF];
  char *plast;
  char c;
 
  if ( char_special[EOF-EOF] != TRUE )
    {
      char_special[EOF -  EOF] = TRUE;
      char_special['\n' - EOF] = TRUE;
      char_special['\r' - EOF] = TRUE;
    }
 
  plast = top_string + sizeof(char *);
  if ( plast > &string_space[MAX_STRING - MAX_STRING_LENGTH] )
    {
      bug( "Fread_string: MAX_STRING %d exceeded.", MAX_STRING );
      exit( 1 );
    }
 
  /*
   * Skip blanks.
   * Read first char.
   */
  do
    {
      c = getc( fp );
    }
  while ( isspace(c) );
 
  if ( ( *plast++ = c ) == '\n')
    return &str_empty[0];
 
  for ( ;; )
    {
      if ( !char_special[ ( *plast++ = getc( fp ) ) - EOF ] )
    continue;
 
      switch ( plast[-1] )
        {
        default:
      break;
 
        case EOF:
      bug( "Fread_string_eol  EOF", 0 );
      exit( 1 );
      break;
 
        case '\n':  case '\r':
      {
        union
        {
          char *      pc;
          char        rgc[sizeof(char *)];
        } u1;
        int ic;
        int iHash;
        char *pHash;
        char *pHashPrev;
        char *pString;
 
        plast[-1] = '\0';
        iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
        for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
          {
        for ( ic = 0; ic < sizeof(char *); ic++ )
          u1.rgc[ic] = pHash[ic];
        pHashPrev = u1.pc;
        pHash    += sizeof(char *);
 
        if ( top_string[sizeof(char *)] == pHash[0]
             &&   !strcmp( top_string+sizeof(char *)+1, pHash+1 ) )
          return pHash;
          }
 
        if ( fBootDb )
          {
        pString             = top_string;
        top_string          = plast;
        u1.pc               = string_hash[iHash];
        for ( ic = 0; ic < sizeof(char *); ic++ )
          pString[ic] = u1.rgc[ic];
        string_hash[iHash]  = pString;
 
        nAllocString += 1;
        sAllocString += top_string - pString;
        return pString + sizeof(char *);
          }
        else
          {
        return str_dup( top_string + sizeof(char *) , "");
          }
      }
        }
    }
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
  char c;

  do
    {
      c = getc( fp );
    }
  while ( c != '\n' && c != '\r' );

  do
    {
      c = getc( fp );
    }
  while ( c == '\n' || c == '\r' );

  ungetc( c, fp );
  return;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
  static char word[MAX_INPUT_LENGTH];
  char *pword;
  int i=0;
  char temp_word[MSL];
  char cEnd;

  do
  {
      cEnd = getc( fp );
  }
  while ( isspace( cEnd ) );

  if ( cEnd == '\'' || cEnd == '"' )
  {
      pword   = word;
  }
  else
  {
      word[0] = cEnd;
      pword   = word+1;
      cEnd    = ' ';
  }

  for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
  {
      *pword = getc( fp );
      temp_word[i++] = *pword;
      if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
      {
        if ( cEnd == ' ' )
          ungetc( *pword, fp );
        *pword = '\0';
        return word;
      }
  }

  bugf( "Fread_word: word too long %s,%s.", pword,temp_word );
  crash_fix();
  exit(1);

  return NULL;
}
#if OLD_MEM
/*
 * Allocate some ordinary memory,
 *   with the expectation of freeing it someday.
 */
void *alloc_mem( int sMem )
{
  void *pMem;
  int *magic;
  int iList;

  sMem += sizeof(*magic);

  for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
      if ( sMem <= rgSizeList[iList] )
    break;
    }

  if ( iList == MAX_MEM_LIST )
    {
      bug( "Alloc_mem: size %d too large.", sMem );
      exit( 1 );
    }

  if ( rgFreeList[iList] == NULL )
    {
      pMem              = alloc_perm( rgSizeList[iList] );
    }
  else
    {
      pMem              = rgFreeList[iList];
      rgFreeList[iList] = * ((void **) rgFreeList[iList]);
    }

  magic = (int *) pMem;
  *magic = MAGIC_NUM;
  pMem += sizeof(*magic);

  return pMem;
}



/*
 * Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 */
void free_mem( void *pMem, int sMem )
{
  int iList;
  int *magic;

  pMem -= sizeof(*magic);
  magic = (int *) pMem;

  if (*magic != MAGIC_NUM)
    {
      bug("Attempt to recyle invalid memory of size %d.",sMem);
      bug((char*) pMem + sizeof(*magic),0);
      return;
    }

  *magic = 0;
  sMem += sizeof(*magic);

  for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
      if ( sMem <= rgSizeList[iList] )
    break;
    }

  if ( iList == MAX_MEM_LIST )
    {
      bug( "Free_mem: size %d too large.", sMem );
      exit( 1 );
    }

  * ((void **) pMem) = rgFreeList[iList];
  rgFreeList[iList]  = pMem;

  return;
}
#else
/*From: Nathan Strong <gblues@jps.net>
To: ROM Mailing List <rom@rom.org>
Subject: [SNIPPET] buffer overflow detection
Date: Tuesday, August 10, 1999 7:07 AM

I've decided to release the new memory allocation routines I wrote as a
snippet. The actual algorithm is the same as in ROM 2.4, but I've added a
few features along the way ;)

Note that you'll need to go through your code ('grep free_mem *.[ch]') and
change all the calls to free_mem() from "free_mem(foo, bar);" to
"free_mem(foo);", as well
as change the prototype in merc.h.
*/
/* New memory allocation routines by Calath (gblues@jps.net),    *
 * (c) August 1999. The code may be freely used for any purpose, *
 * as long as this header remains intact. If you decide to use   *
 * this code, write me an e-mail telling me you're using it.     *
 *                                                               *
 * HOW IT WORKS: alloc_mem() generates a random 8-bit number and *
 * puts it at the start and end of your allocated buffer. Later, *
 * when it tries to free the memory (in free_mem) it checks to   *
 * see if the numbers match. If they don't, it's a good sign the *
 * buffer has been overflown.                                    *
 *                                                               *
 * I got the idea from a magazine that had an article on stack-  *
 * smashing. One of the tools mentioned was a compiler mod that  *
 * basically does the same thing I'm doing here. That's where    *
 * the "canary" concept came from ;)                             */

void *alloc_mem( int sMem )
{
  int iList;
  int *magic;
  char *pcanary;
  void *ptr;
  unsigned char canary = number_bits(8) | (H);

  sMem += 3 * sizeof(*magic);

  for( iList = 0;
       iList < MAX_MEM_LIST && rgSizeList[iList] < sMem;
       iList++ );
  if( iList == MAX_MEM_LIST )
  {
    bugf("Hunk size %d too large.", sMem);
    return NULL;
  }

  if( rgFreeList[iList] == NULL )
    ptr = alloc_perm( rgSizeList[iList] );
  else
  {
    ptr = rgFreeList[iList];
    rgFreeList[iList] = *( (void **) rgFreeList[iList] );
  }

  magic    = (int  *) ptr;
  pcanary  = (char *) ptr;
  magic[0] = rgSizeList[iList];
  pcanary[ sizeof(*magic) ]                       = canary;
  pcanary[ rgSizeList[iList] - sizeof(*magic) ]   = canary;
  pcanary[ rgSizeList[iList] - sizeof(*magic)-1 ] = '\0';
  ptr += 2 * sizeof( *magic );

  return ptr;
}

void free_mem( void *pMem )
{
  int *magic;
  char *canary1, *canary2;
  int iList;

  pMem -= ( 2 * sizeof(*magic) );
  magic = (int *) pMem;

  if( magic[0] == 0 )
  {
    print_cmd_hist();
    bugf("Attempting to free hunk twice. %s", interp_cmd);
    return;
  }

  for( iList = 0;
       iList < MAX_MEM_LIST && rgSizeList[iList] != *magic;
       iList++ );
  if( iList == MAX_MEM_LIST )
  {
    print_cmd_hist();
    bugf("Attempting to recycle invalid hunk size %d, %s", *magic, interp_cmd);
    return;
  }

  canary1 = (char *) pMem + sizeof(*magic);
  canary2 = (char *) pMem + ( rgSizeList[iList] - sizeof(*magic) );

  if( *canary1 != *canary2 )
  {
    print_cmd_hist();
    bugf("ACK! The canary died! Buffer overflow! Aborting. %s", interp_cmd);
    crash_fix();
    abort();
  }

  magic[0] = 0;
  *( (void **) pMem ) = rgFreeList[iList];
  rgFreeList[iList] = pMem;
  return;
}

/*Calath
gblues@jps.net
*/

#endif

/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely.
 */
void *alloc_perm( int sMem )
{
  static char *pMemPerm;
  static int iMemPerm;
  static int    iLastMemPerm;
  static char *pLastMemPerm;
  void *pMem=0;

  while ( sMem % sizeof(long) != 0 )
    sMem++;
  if ( sMem > MAX_PERM_BLOCK )
    {
      bugf( "Alloc_perm: %d too large. %s", sMem, interp_cmd );
      exit( 1 );
    }

  /* Try to fill up the last perm memory block first. */
  if ( pLastMemPerm && iLastMemPerm + sMem <= MAX_PERM_BLOCK )
    {
      nSlackPerm    -= sMem;

      pMem         = pLastMemPerm + iLastMemPerm;
      if ( ( iLastMemPerm += sMem ) > MAX_PERM_BLOCK - rgSizeList[0] )
    {
      pLastMemPerm = NULL;
      iLastMemPerm = 0;
    }

      nAllocPerm++;
      sAllocPerm    += sMem;
      return pMem;
    }

  if ( pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK )
    {
      if ( pMemPerm )
    nSlackPerm += MAX_PERM_BLOCK - iMemPerm;

      /* Set the last perm memory block if there is enough space. */
      if ( pMemPerm && iMemPerm <= MAX_PERM_BLOCK - rgSizeList[0]
       && ( pLastMemPerm == NULL || iLastMemPerm > iMemPerm ) )
    {
      pLastMemPerm = pMemPerm;
      iLastMemPerm = iMemPerm;
    }

      if ( ( pMemPerm = calloc( 1, MAX_PERM_BLOCK ) ) == NULL )
    {
      perror( "Alloc_perm" );
      exit( EXIT_FAILURE );
    }
      iMemPerm = 0;
      nPermBlock++;
    }

  pMem    = pMemPerm + iMemPerm;
  iMemPerm   += sMem;
  nAllocPerm++;
  sAllocPerm += sMem;
  return pMem;

  /*if ( pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK )
    {
    iMemPerm = 0;
    if ( ( pMemPerm = calloc( 1, MAX_PERM_BLOCK ) ) == NULL )
    {
    perror( "Alloc_perm" );
    exit( 1 );
    }
    }

    pMem        = pMemPerm + iMemPerm;
    iMemPerm   += sMem;
    nAllocPerm += 1;
    sAllocPerm += sMem;
    return pMem;
  */
}



/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
char *str_dup( const char *str , const char *checkstr)
{
  char *str_new;
  
  if ( str[0] == '\0' )
    return &str_empty[0];

  /* This seems to be checkings to see if the address of str
     is between the start block and less than the top string.  This
     is rather confusing I think.

     str is a pointer to an address string.
     string_space is the pointer to the total size of MAX_STRING allocated
     space.  This is our string section.

     top_string is?

     insure++ gives this message:

     >>   if ( str >= string_space && str < top_string )
     Expression compares unrelated pointers: str >= string_space
     Left hand side : 0x0e5e1600
     In block       : 0x0e5e1600 thru 0x0e5e16ff (256 bytes)
     strArea, declared at db.c, 283
     Right hand side: 0x40545008
     In block       : 0x40545008 thru 0x40d15007 (8192000 bytes)

     So the question remains..  what is actually wrong, if anything of the following
     line.
  */
  if ( str >= string_space && str < top_string )
    return (char *) str;

  str_new = alloc_mem( strlen(str) + 1 );
  strcpy( str_new, str );
  return str_new;
}



/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */
void free_string( char *pstr )
{
  if ( pstr == NULL)
    return;
  if ( pstr == &str_empty[0] )
    return;
  if ( ( pstr >= string_space && pstr < top_string ) )
    return;

#if OLD_MEM
  free_mem( pstr, strlen(pstr) + 1 );
#else
  free_mem( pstr );
#endif

  pstr = &str_empty[0];

  return;
}

long convert_level ( char * arg )
/* Code by Nebseni of Clandestine MUD */
{
  if (arg[0] == '\0')
    return 0;
  if (is_number(arg))
    return atoi(arg);
  else if (!str_cmp(arg, "imm"))
    return LEVEL_IMMORTAL;
  else if (!str_cmp(arg, "hero") || !str_cmp(arg, "hero+"))
    return LEVEL_HERO;
  else return 0;
}


long get_area_level(AREA_DATA *pArea)
/* Code by Nebseni of Clandestine MUD ...
   thanks to Erwin Andreasan for help with sscanf */
/* Returns long (MAX_LEVEL + 1)*low-level + high-level */
/* Example: Area with credits line: 
       {10 25} Nebseni  Clandestine Golf Course
   Max level 220
   returns 221*10 + 25 = 2235 */
/* Use modulo arithmetic for individual numbers, e.g.,
   lo_level = get_area_level(Area) / (MAX_LEVEL+1);
   hi_level = get_area_level(Area) % (MAX_LEVEL+1); */
{
  return (pArea->low_range*(MAX_LEVEL+1) + 
    pArea->high_range > 0) ? pArea->high_range : 
     ((pArea->low_range == LEVEL_HERO) ? (LEVEL_IMMORTAL-1) :     
     MAX_LEVEL);
}


void sort_areas_by_level ( void )
/* Code by Nebseni of Clandestine MUD */
/* Creates a linked list starting with area_first_sorted, linking
   on pArea->next_sort. Sorted on get_area_level. */
{
  AREA_DATA * pArea1;
  AREA_DATA * pArea2;
  int thislevel;

  area_first_sorted = area_first;
  area_first_sorted->next_sort = NULL;

 
  /* if only one area it is already sorted! */
  if ( area_first->next == NULL )
    return;
   
  /* iterate through the rest of the areas */
  for ( pArea1 = area_first->next; ; )
    {
      if ( (thislevel = get_area_level(pArea1)) <= 
       get_area_level(area_first_sorted))
    {
      /* this area goes at head of sorted list */
      pArea1->next_sort = area_first_sorted;
      area_first_sorted = pArea1;
    }
      else 
    {
      /* iterate through list and insert appropriately */
      for ( pArea2 = area_first_sorted ; ; )
        {
          if ( pArea2->next_sort == NULL ||
           (thislevel <= get_area_level(pArea2->next_sort)))
        {
          pArea1->next_sort = pArea2->next_sort;
          pArea2->next_sort = pArea1;
          break;
        } 
          pArea2 = pArea2->next_sort;
        }
    }
      if (pArea1->next != NULL)
    {
      pArea1 = pArea1->next;
    }
      else
    break;
    }
}


void do_areas( CHAR_DATA *ch, char *argument )
/* Modified from ROM 2.4 code by Nebseni of Clandestine MUD */
/* Outputs sorted area list with arguments:
areas gives all areas 0 to LEVEL_IMMORTAL-1 (morts) or MAX_LEVEL (imms)
areas 30 gives all areas with min <= 30 <= max
areas 30 45 gives all areas with min <= 45 and 30 <= max 
*/
{
  AREA_DATA *pArea;
  BUFFER    *buffer = NULL;

  char arg1 [ MAX_INPUT_LENGTH ];
  char arg2 [ MAX_INPUT_LENGTH ];
  char buf [MSL], buf2 [MSL];
  char abuf[MSL];
  int  lo_level = 0, hi_level = LEVEL_HERO;
  sh_int match_align = AREA_ALIGN_GOOD;
  bool found    = FALSE;
  bool search   = FALSE;
  bool bylevel  = FALSE;
  bool byalign  = FALSE;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( (arg1[0] != '\0')
    &&  (!str_prefix( arg1, "suggested")
      || !str_prefix( arg1, "help")
      || !str_prefix( arg1, "newbie") ) )
  {
    bylevel = TRUE;
    lo_level = ch->level - 5;
    hi_level = ch->level + 10;
    if (lo_level < 0)
      lo_level = 0;
    if (lo_level < LEVEL_HERO)
      lo_level = LEVEL_HERO;
  }
  else if ((arg1[0] != '\0')
    &&  (!str_prefix( arg1, "good")
      || !str_prefix( arg1, "evil")
      || !str_prefix( arg1, "neutral") ) )
  {
    byalign = TRUE;
    if (!str_prefix( arg1, "good"))
      match_align = AREA_ALIGN_GOOD;
    else if (!str_prefix( arg1, "evil"))
      match_align = AREA_ALIGN_EVIL;
    else if (!str_prefix( arg1, "neutral"))
      match_align = AREA_ALIGN_NEUTRAL;
  }
  else if ( !is_number( arg1 ) )
    search = TRUE;

  if ( arg1[0] == '\0' )
    search = FALSE;

  if ( search )
  {
    if ( IS_IMMORTAL( ch ) )
    {
      lo_level = -1;
      hi_level = MAX_LEVEL;
    }
    else
    {
      lo_level = 1;
      hi_level = LEVEL_IMMORTAL-1;;
    }
  }
  else if ((bylevel == FALSE) && (byalign == FALSE))
  {
    lo_level = convert_level( arg1 ) ?
      URANGE(1,convert_level(arg1),MAX_LEVEL) : 0;

    if ( IS_IMMORTAL( ch ) )
      hi_level = convert_level( arg2 ) ?
        URANGE( 1, convert_level( arg2 ), MAX_LEVEL ) : MAX_LEVEL;
    else
      hi_level = convert_level( arg2 ) ?
        URANGE( 1, convert_level( arg2 ), LEVEL_IMMORTAL-1 ) : LEVEL_IMMORTAL;
  }

  if ( !bylevel &&  !byalign && !search && arg1[0] && arg2[0] == '\0' ) hi_level = lo_level;


  /* Output matching areas */
  for ( pArea = area_first_sorted ; ; )
  {

    if ( pArea == NULL )
      break;

    if (!IS_IMMORTAL(ch) && IS_SET(pArea->flags, AREA_CLANHALL))
    {
      pArea = pArea->next_sort;
      continue;
    }

    if (byalign && (match_align != pArea->align))
    {
      pArea = pArea->next_sort;
      continue;
    }

    switch (pArea->align)
    {
      case AREA_ALIGN_GOOD:    strcpy(abuf, "Good"); break;
      case AREA_ALIGN_EVIL:    strcpy(abuf, "Evil"); break;
      case AREA_ALIGN_NEUTRAL: strcpy(abuf, "Neut"); break;
      default:                 strcpy(abuf, "All");  break;
    }

    //Continent Names
    strncpy_color(buf, continent_name(pArea), 12, ' ', TRUE );
    strcpy( buf2, continent_table[pArea->continent].name );

    if ( search )
    {
      if ( ( is_name( arg1, pArea->name ) || is_name( arg1, pArea->builders )
      ||     ( is_name(arg1, buf2) && continent_table[pArea->continent].playerarea && !IS_IMMORTAL(ch) )
      ||     ( is_name(arg1, buf2) ) )
      && (   pArea->high_range <= hi_level ) 
      && (   pArea->low_range  >= lo_level ) )
      {
        if ( !found )
        {
          buffer = new_buf();
          found = TRUE;
        }

        bprintf( buffer, "{y<{c%-2d %3d{y> {W%-32s {w%-12s {y%-4s {C%-20s{x\n\r",
                 pArea->low_range, pArea->high_range,
                 pArea->name, buf, abuf, pArea->builders );
      }
    }
    else
    {
      if ( hi_level == lo_level )
      {
        if ( pArea->high_range >= hi_level && pArea->low_range <= lo_level )
        {
          if ( !found )
          {
            buffer = new_buf();
            found = TRUE;
          }
          
          bprintf( buffer, "{y<{c%-2d %3d{y> {W%-32s {w%-12s {y%-4s {c%-20s{x\n\r",
                   pArea->low_range, pArea->high_range,
                   pArea->name, buf, abuf, pArea->builders );
        }
      }
      else
      {
        if ( bylevel
        && (pArea->low_range <= ch->level)
        && (pArea->high_range >= ch->level) )
        {
          if ( !found )
          {
            buffer = new_buf();
            found = TRUE;
          }

          bprintf( buffer, "{y<{c%-2d %3d{y> {W%-32s {w%-12s {y%-4s {C%-20s{x\n\r",
                   pArea->low_range, pArea->high_range,
                   pArea->name, buf, abuf, pArea->builders);
        }
        else if ( pArea->high_range <= hi_level && pArea->low_range >= lo_level )
        {
          if ( !found )
          {
            buffer = new_buf();
            found = TRUE;
          }

          bprintf( buffer, "{y<{c%-2d %3d{y> {W%-32s {w%-12s {y%-4s {C%-20s{x\n\r",
                   pArea->low_range, pArea->high_range,
                   pArea->name, buf, abuf, pArea->builders);
        }
      }
    }
    if ( pArea )
      pArea = pArea->next_sort;
    else
      break;
  }

  /* Exit if no areas match */
  if ( !found )
  {
    send_to_char_bw( "No areas meeting those criteria.\n\r", ch );
    return;
  }
  else
  {
    page_to_char( buf_string( buffer ), ch );
    free_buf( buffer );
  }
  return;
}


void do_memory( CHAR_DATA *ch, char *argument )
{
  printf_to_char(ch, "Affects            %5d\n\r", top_affect    ); 
  printf_to_char(ch, "Areas              %5d\n\r", top_area      );
  printf_to_char(ch, "ExDes              %5d\n\r", top_ed        );
  printf_to_char(ch, "Exits              %5d\n\r", top_exit      );
  printf_to_char(ch, "Helps              %5d\n\r", top_help      );
  printf_to_char(ch, "Socials            %5d\n\r", maxSocial     );
  printf_to_char(ch, "Mobs               %5d\n\r", top_mob_index ); 
  printf_to_char(ch, "(in use)           %5d\n\r", mobile_count  );
  printf_to_char(ch, "Mob Loss           %5d\n\r", count_mob_loss()      );
  printf_to_char(ch, "Objs               %5d\n\r", top_obj_index ); 
  printf_to_char(ch, "(in use)           %5d\n\r", pObjNum       ); 
  printf_to_char(ch, "Obj Loss           %5d\n\r", count_obj_loss()      );
  printf_to_char(ch, "Resets             %5d\n\r", top_reset     );
  printf_to_char(ch, "Rooms              %5d\n\r", top_room      );
  printf_to_char(ch, "Shops              %5d\n\r", top_shop      );
  printf_to_char(ch, "Strings            %5d strings of %7d bytes (max %d).\n\r",
         nAllocString, sAllocString, MAX_STRING );

  printf_to_char(ch, "Perms             %6d blocks  of %7d bytes.\n\r",
         nAllocPerm, sAllocPerm );
  printf_to_char(ch, "Slack Mem:      %8d bytes.\n\r",nSlackPerm);
  printf_to_char(ch, "Open Files:     %8d files.\n\r",nFilesOpen);
  printf_to_char(ch, "Open Descs:     %8d Descriptors.\n\r",nDescsOpen);
  printf_to_char(ch, "Update time:       %5d{x.\n\r",update_time);
  printf_to_char(ch, "Largest Lag time:  %5d.\n\r",lag_update_time);
  return;
}
void do_chkmemory( CHAR_DATA *ch, char *argument )
{
  return;
}

void do_dump( CHAR_DATA *ch, char *argument )
{
  int count,count2,num_pcs,aff_count;
  CHAR_DATA *fch;
  MOB_INDEX_DATA *pMobIndex;
  PC_DATA *pc;
  OBJ_DATA *obj;
  OBJ_INDEX_DATA *pObjIndex;
  ROOM_INDEX_DATA *room;
  EXIT_DATA *exit;
  DESCRIPTOR_DATA *d;
  AFFECT_DATA *af;
  FILE *fp;
  int vnum,nMatch = 0;

  /* open file */
  fclose(fpReserve);
  nFilesOpen--;
  fp = fopen("mem.dmp","w");
  nFilesOpen++;
  /* report use of data structures */
    
  num_pcs = 0;
  aff_count = 0;

  /* mobile prototypes */
  fprintf(fp,"MobProt    %4d (%8lu bytes)\n",
      top_mob_index, top_mob_index * (sizeof(*pMobIndex))); 

  /* mobs */
  count = 0;  count2 = 0;
  for (fch = char_list; fch != NULL; fch = fch->next)
    {
      count++;
      if (fch->pcdata != NULL)
    num_pcs++;
      for (af = fch->affected; af != NULL; af = af->next)
    aff_count++;
    }
  for (fch = char_free; fch != NULL; fch = fch->next)
    count2++;

  fprintf(fp,"Mobs    %4d (%8lu bytes), %2d free (%lu bytes)\n",
      count, count * (sizeof(*fch)), count2, count2 * (sizeof(*fch)));

  /* pcdata */
  count = 0;
  for (pc = pcdata_free; pc != NULL; pc = pc->next)
    count++; 

  fprintf(fp,"Pcdata    %4d (%8lu bytes), %2d free (%lu bytes)\n",
      num_pcs, num_pcs * (sizeof(*pc)), count, count * (sizeof(*pc)));

  /* descriptors */
  count = 0; count2 = 0;
  for (d = descriptor_list; d != NULL; d = d->next)
    count++;
  for (d= descriptor_free; d != NULL; d = d->next)
    count2++;

  fprintf(fp, "Descs    %4d (%8lu bytes), %2d free (%lu bytes)\n",
      count, count * (sizeof(*d)), count2, count2 * (sizeof(*d)));

  /* object prototypes */
  for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
      {
    for (af = pObjIndex->affected; af != NULL; af = af->next)
      aff_count++;
    nMatch++;
      }

  fprintf(fp,"ObjProt    %4d (%8lu bytes)\n",
      top_obj_index, top_obj_index * (sizeof(*pObjIndex)));


  /* objects */
  count = 0;  count2 = 0;
  for (obj = object_list; obj != NULL; obj = obj->next)
    {
      count++;
      for (af = obj->affected; af != NULL; af = af->next)
    aff_count++;
    }
  for (obj = obj_free; obj != NULL; obj = obj->next)
    count2++;

  fprintf(fp,"Objs    %4d (%8lu bytes), %2d free (%lu bytes)\n",
      count, count * (sizeof(*obj)), count2, count2 * (sizeof(*obj)));

  /* affects */
  count = 0;
  for (af = affect_free; af != NULL; af = af->next)
    count++;

  fprintf(fp,"Affects    %4d (%8lu bytes), %2d free (%lu bytes)\n",
      aff_count, aff_count * (sizeof(*af)), count, count * (sizeof(*af)));

  /* rooms */
  fprintf(fp,"Rooms    %4d (%8lu bytes)\n",
      top_room, top_room * (sizeof(*room)));

  /* exits */
  fprintf(fp,"Exits    %4d (%8lu bytes)\n",
      top_exit, top_exit * (sizeof(*exit)));

  fclose(fp);
  nFilesOpen--;
  /* start printing out mobile data */
  fp = fopen("mob.dmp","w");
  nFilesOpen++;
  fprintf(fp,"\nMobile Analysis\n");
  fprintf(fp,  "---------------\n");
  nMatch = 0;
  for (vnum = 0; nMatch < top_mob_index; vnum++)
    if ((pMobIndex = get_mob_index(vnum)) != NULL)
      {
    nMatch++;
    fprintf(fp,"#%-4d %3d active %3d killed     %s\n",
        pMobIndex->vnum,pMobIndex->count,
        pMobIndex->killed,pMobIndex->short_descr);
      }
  fclose(fp);
  nFilesOpen--;
  /* start printing out object data */
  fp = fopen("obj.dmp","w");
  nFilesOpen++;
  fprintf(fp,"\nObject Analysis\n");
  fprintf(fp,  "---------------\n");
  nMatch = 0;
  for (vnum = 0; nMatch < top_obj_index; vnum++)
    if ((pObjIndex = get_obj_index(vnum)) != NULL)
      {
    nMatch++;
    fprintf(fp,"#%-4d %3d active %3d reset      %s\n",
        pObjIndex->vnum,pObjIndex->count,
        pObjIndex->reset_num,pObjIndex->short_descr);
      }

  /* close file */
  fclose(fp);
  nFilesOpen--;
  fpReserve = fopen( NULL_FILE, "r" );
  nFilesOpen++;
}

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
  switch ( number_bits( 2 ) )
    {
    case 0:  number -= 1; break;
    case 3:  number += 1; break;
    }

  return UMAX( 1, number );
}

/*
 * Generate a random number.
 */
int number_range( int from, int to )
{
  int power;
  int number;

  if (from == 0 && to == 0) return 0;

  if ( ( to = to - from + 1 ) <= 1 ) return from;

  for ( power = 2; power < to; power <<= 1 );

  while ( ( number = number_mm() & (power -1 ) ) >= to );

  return from + number;
}

/*
 * Generate a percentile roll.
 */
int number_percent( void )
{
  int percent;

  while ( (percent = number_mm() & (128-1) ) > 99 );

  return 1 + percent;
}



/*
 * Generate a random door.
 */
int number_door( void )
{
  int door;

  while ( ( door = number_mm() & (8-1) ) > 5);

  return door;
}

int number_bits( int width )
{
  return number_mm( ) & ( ( 1 << width ) - 1 );
}

/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */

/* I noticed streaking with this random number generator, so I switched
   back to the system srandom call.  If this doesn't work for you, 
   define OLD_RAND to use the old system -- Alander */

#if defined (OLD_RAND)
static  int     rgiState[2+55];
#endif
 
void init_mm( )
{
#if defined (OLD_RAND)
  int *piState;
  int iState;
 
  piState     = &rgiState[2];
 
  piState[-2] = 55 - 55;
  piState[-1] = 55 - 24;
 
  piState[0]  = ((int) current_time) & ((1 << 30) - 1);
  piState[1]  = 1;
  for ( iState = 2; iState < 55; iState++ )
    {
      piState[iState] = (piState[iState-1] + piState[iState-2])
    & ((1 << 30) - 1);
    }
#else
  srandom(time(NULL)^getpid());
#endif
  return;
}

long number_mm( void )
{
#if defined (OLD_RAND)
  int *piState;
  int iState1;
  int iState2;
  int iRand;
 
  piState = &rgiState[2];
  iState1 = piState[-2];
  iState2 = piState[-1];
  iRand   = (piState[iState1] + piState[iState2]) & ((1 << 30) - 1);
  piState[iState1]    = iRand;

  if ( ++iState1 == 55 )
    iState1 = 0;

  if ( ++iState2 == 55 )
    iState2 = 0;

  piState[-2] = iState1;
  piState[-1] = iState2;
  return iRand >> 6;
#else
  return random() >> 6;
#endif
}


/*
 * Roll some dice.
 */
int dice( int number, int size )
{
  int idice;
  int sum;

  switch ( size )
    {
    case 0: return 0;
    case 1: return number;
    }

  for ( idice = 0, sum = 0; idice < number; idice++ )
    sum += number_range( 1, size );

  return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
  return value_00 + level * (value_32 - value_00) / 32;
}

/*
 * Simple linear interpolation.
 */
int interpolateNew( int p, int p0, int p1, int v0, int v1 )
{
    if ( p1 - p0 == 0 )
    return v1;
    return v0 + ( p - p0 ) * ( v1 - v0 ) / ( p1 - p0 );
}


/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
  for ( ; *str != '\0'; str++ )
    {
      if ( *str == '~' )
    *str = '-';
    }

  return;
}

/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_percent( char *str )
{
  for ( ; *str != '\0'; str++ )
    {
      if ( *str == '%' )
    *str = 'V';
    }

  return;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
  if ( astr == NULL )
    {
      bug( "Str_cmp: null astr.", 0 );
      return TRUE;
    }

  if ( bstr == NULL )
    {
      bug( "Str_cmp: null bstr.", 0 );
      return TRUE;
    }

  for ( ; *astr || *bstr; astr++, bstr++ )
    {
      if ( LOWER(*astr) != LOWER(*bstr) )
    return TRUE;
    }

  return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
  if ( astr == NULL )
    {
      bug( "Str_prefix: null astr.", 0 );
      return TRUE;
    }

  if ( bstr == NULL )
    {
      bug( "Str_prefix: null bstr.", 0 );
      return TRUE;
    }

  for ( ; *astr; astr++, bstr++ )
    {
      if ( LOWER(*astr) != LOWER(*bstr) )
    return TRUE;
    }

  return FALSE;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
  int sstr1;
  int sstr2;
  int ichar;
  char c0;

  if ( ( c0 = LOWER(astr[0]) ) == '\0' )
    return FALSE;

  sstr1 = strlen(astr);
  sstr2 = strlen(bstr);

  for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
    {
      if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
    return FALSE;
    }

  return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
  int sstr1;
  int sstr2;

  sstr1 = strlen(astr);
  sstr2 = strlen(bstr);
  if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
    return FALSE;
  else
    return TRUE;
}



/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
  static char strcap[MAX_STRING_LENGTH];
  int i;

  for ( i = 0; str[i] != '\0'; i++ )
    strcap[i] = LOWER(str[i]);
  strcap[i] = '\0';
  strcap[0] = UPPER(strcap[0]);
  return strcap;
}

/*
 * Unlike capitalize(), this will not lowercase other letters in the line.
 */
char *new_capitalize( const char *str )
{
  static char strcap[MAX_STRING_LENGTH];
  int i;

  for ( i = 0; str[i] != '\0'; i++ )
      strcap[i] = str[i];
  strcap[i] = '\0';
  strcap[0] = UPPER(strcap[0]);
  return strcap;
}



/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, char *file, char *str )
{
  FILE *fp;
  char buf[MSL];
  char worklist[MSL];

  if ( IS_NPC(ch) || str[0] == '\0' )
    return;

  fclose( fpReserve );
  nFilesOpen--;
  if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
      perror( file );
      send_to_char( "Could not open the file!\n\r", ch );
    }
  else
    {
      nFilesOpen++;
      mprintf(sizeof(buf), buf,"{C [{c%-28s{C] {G[{g%-5d{G] {y%s{Y:{W %s{x",(char *) ctime( &current_time ),
          ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
      str_normalize_white_space(buf);
      str_cleanup_extra_white_space(buf);
      strip_string(buf);
      fprintf(fp,"%s\n",buf);
      fclose( fp );
      nFilesOpen--;

      if (!strcmp(file,DUH_FILE))
        strcpy(worklist,"Duh");
      else if (!strcmp(file,TYPO_FILE))
        strcpy(worklist,"Typo");
      else if (!strcmp(file,BUG_FILE))
        strcpy(worklist,"Bug");
      else if (!strcmp(file,TODO_FILE))
        strcpy(worklist,"Help");
      else if (!strcmp(file,BUILDER_FILE))
        strcpy(worklist,"Client");
      else
        strcpy(worklist,"(unknown)");

      if (IS_IMMORTAL(ch))
      {
        if ( !IS_SET(ch->wiznet,WIZ_ON)
        ||   !IS_SET(ch->wiznet,WIZ_WORKLIST) )
          printf_to_char( ch, "Added new \"%s\" entry: %s{x\n\r", worklist, buf );
      }

      mprintf(sizeof(buf), buf,"New \"%s\" entry:%s{x", worklist, buf );
      wiznet( buf, NULL, NULL, WIZ_WORKLIST, 0, 0 );
    }

  fpReserve = fopen( NULL_FILE, "r" );
  nFilesOpen++;
  return;
}

void append_hint( CHAR_DATA *ch, char *str )
{
  FILE *fp;
  char buf[MSL];
  char *file = HINTS_FILE;

  if ( IS_NPC(ch) || str[0] == '\0' )
    return;

  fclose( fpReserve );
  nFilesOpen--;
  if ( ( fp = fopen( file, "a" ) ) == NULL )
  {
    perror( file );
    send_to_char( "Could not open the file!\n\r", ch );
  }
  else
  {
    nFilesOpen++;
    mprintf(sizeof(buf), buf,"%s{x", str );
    str_normalize_white_space(buf);
    str_cleanup_extra_white_space(buf);
    strip_string(buf);
    fprintf(fp,"%s\n",buf);
    fclose( fp );
    nFilesOpen--;

    if (IS_IMMORTAL(ch))
    {
      if ( !IS_SET(ch->wiznet,WIZ_ON)
      ||   !IS_SET(ch->wiznet,WIZ_WORKLIST) )
        printf_to_char( ch, "Added newbie hint: %s{x\n\r", buf );
    }

    mprintf(sizeof(buf), buf,"New \"Hint\" entry:%s{x", buf );
    wiznet( buf, NULL, NULL, WIZ_WORKLIST, 0, 0 );
  }

  fpReserve = fopen( NULL_FILE, "r" );
  nFilesOpen++;
  return;
}



/*
 * Reports a bug.
 */
void bug( const char *str, int param )
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MSL+10];
  if ( fpArea != NULL )
    {
      int iLine;
      int iChar;

      if ( fpArea == stdin )
    {
      iLine = 0;
    }
      else
    {
      iChar = ftell( fpArea );
      fseek( fpArea, 0, 0 );
      for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
        {
          while ( getc( fpArea ) != '\n' )
        ;
        }
      fseek( fpArea, iChar, 0 );
    }

      mprintf(sizeof(buf), buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
      log_string( buf );
    }


  sprintf(buf, str, param );
  wiznet(buf, NULL, NULL, WIZ_BUGS, 0, 0);
  sprintf( buf2, "[*****] BUG: %s", buf );
  log_string( buf2 );
  return;
}



/*
 * Writes a string to the log.
 */
void log_string( const char *str )
{
  char *strtime;

  strtime                    = ctime( &current_time );
  strtime[strlen(strtime)-1] = '\0';
  fprintf( stderr, "%s :: %s\n", strtime, str );
  return;
}



/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
  return;
}

/*
 * Removes the dollar signs from a string.
 * Used for player-entered strings that go throuh act functions.
 */
void smash_dollar( char *str )
{
    for ( ; *str != '\0'; str++ )
    {
    if ( *str == '$' )
        *str = 'S';
    }
}
#ifdef TEST_MUD_4
#define AFF_MAX_LIST 10000
/*
 * Shows memory stats & the primary linked lists.
 */
void do_memory( CHAR_DATA *ch, char *argument )
{
  AFFECT_DATA     *pAf;
  AREA_DATA        *pArea;
  BAN_DATA        *pBan;
  BUFFER        *pBuf;
  CHAR_DATA        *pCh;
  DESCRIPTOR_DATA    *d;
  EXIT_DATA        *pExit;
  EXTRA_DESCR_DATA    *pEd;
  GEN_DATA        *pGen;
  HELP_DATA        *pHelp;
  MOB_INDEX_DATA    *pMobIndex;
  MPROG_LIST        *pProg;
  MPROG_CODE        *pCode;
  NOTE_DATA        *pNote;
  OBJ_DATA        *pObj;
  OBJ_INDEX_DATA    *pObjIndex;
  PC_DATA        *pc;
  RESET_DATA        *pReset;
  ROOM_INDEX_DATA    *pRoom;
  SHOP_DATA        *pShop;
  BUFFER        *buffer;
  char        buf[MAX_STRING_LENGTH];
  char        arg[MAX_INPUT_LENGTH];
  int         iHash;
  int         door;
  int         n0;
  int         n1;
  int         n2;
  int         n3;
  int         n4;
  int         n_aff[AFF_MAX_LIST];
  int         n_daf[AFF_MAX_LIST];
  int         n_faf[AFF_MAX_LIST];
  int         n_buf = 1; /* Using one here. :-) */
  int         n_ch  = 0;
  int         n_d   = 0;
  int            n_dch = 0;
  int            n_dob = 0;
  int         n_ed  = 0;
  int         n_ex  = 0;
  int         n_gen = 0;
  int         n_min = 0;
  int         n_mp  = 0;
  int         n_not = 0;
  int         n_obj = 0;
  int         n_oin = 0;
  int         n_op  = 0;
  int         n_pcd = 0;
  int         n_res = 0;
  int         n_rin = 0;
  int         n_rp  = 0;
  bool        show_bytes = FALSE;
  bool        show_blocks = FALSE;

  for ( ; ; )
    {
      argument = one_argument( argument, arg );

      if ( arg[0] == '\0' )
    break;

      if ( !str_prefix( arg, "blocks" ) )
    show_blocks = TRUE;
      else if ( !str_prefix( arg, "bytes" ) )
    show_bytes = TRUE;
      else
    {
      send_to_char( "Syntax:  memory [blocks] [bytes]\n\r", ch );
      return;
    }
    }

  if ( show_blocks )
    {
      show_memory_blocks( ch );
      return;
    }

  for ( n1 = 0; n1 < AFF_MAX_LIST; n1++ )
    n_aff[n1] = n_daf[n1] = n_faf[n1] = 0;

  buf[0] = '\0';

  buffer = new_buf();

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5s %8s | ", "Bytes", "Total" );

  bprintf( buffer, "Category        %s%6s %6s %5s %5s\n\r",
       buf, "Top", "Used", "Free", "Lost" );

  if ( show_bytes )
    strcpy( buf, "===================" );

  bprintf( buffer, "%s=========================================\n\r", buf );

  for ( pCh = char_list; pCh != NULL; pCh = pCh->next )
    {
      n_ch++;
      if ( IS_DELETED( pCh ) )
    n_dch++;
      if ( pCh->pcdata != NULL )
    {
      n_pcd++;
      if ( pCh->pcdata->buffer != NULL )
        n_buf++;
      if ( pCh->pcdata->pNote != NULL )
        n_not++;
    }
      if ( pCh->gen_data != NULL )
    n_gen++;
      for ( pAf = pCh->affected; pAf != NULL; pAf = pAf->next )
    {
      if ( IS_DELETED( pAf ) )
        n_daf[pAf->list]++;
      n_aff[pAf->list]++;
    }
    }

  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
      for ( pObjIndex = obj_index_hash[iHash]; pObjIndex != NULL;
        pObjIndex = pObjIndex->next )
    {
      n_oin++;
      for ( pAf = pObjIndex->affected; pAf != NULL; pAf = pAf->next )
        n_aff[pAf->list]++;
      for ( pEd = pObjIndex->extra_descr; pEd != NULL; pEd = pEd->next )
        n_ed++;
      for ( pProg = pObjIndex->mprogs; pProg != NULL;
        pProg = pProg->next )
        n_op++;
    }

      for ( pMobIndex = mob_index_hash[iHash]; pMobIndex != NULL;
        pMobIndex = pMobIndex->next )
    {
      n_min++;
      for ( pProg = pMobIndex->mprogs; pProg != NULL;
        pProg = pProg->next )
        n_mp++;
    }

      for ( pRoom = room_index_hash[iHash]; pRoom != NULL;
        pRoom = pRoom->next )
    {
      n_rin++;
      for ( pEd = pRoom->extra_descr; pEd != NULL; pEd = pEd->next )
        n_ed++;
      for ( door = 0; door < MAX_DIR; door++ )
        if ( pRoom->exit[door] != NULL ) n_ex++;
      for ( pReset = pRoom->reset_first; pReset != NULL;
        pReset = pReset->next )
        n_res++;
      for ( pProg = pRoom->mprogs; pProg != NULL;
        pProg = pProg->next )
        n_rp++;
    }
    }

  for ( pObj = object_list; pObj != NULL; pObj = pObj->next )
    {
      n_obj++;
      if ( IS_DELETED( pObj ) )
    n_dob++;
      for ( pAf = pObj->affected; pAf != NULL; pAf = pAf->next )
    {
      if ( IS_DELETED( pAf ) )
        n_daf[pAf->list]++;
      n_aff[pAf->list]++;
    }
      for ( pEd = pObj->extra_descr; pEd != NULL; pEd = pEd->next )
    n_ed++;
    }

  for ( d = descriptor_list; d != NULL; d = d->next )
    {
      n_d++;
      if ( d->connected != CON_PLAYING && d->character != NULL )
    {
      n_ch++;
      pCh = d->character;
      if ( pCh->pcdata != NULL )
        {
          n_pcd++;
          if ( pCh->pcdata->buffer != NULL )
        n_buf++;
          if ( pCh->pcdata->pNote != NULL )
        n_not++;
        }
      if ( pCh->gen_data != NULL )
        n_gen++;
      for ( pAf = pCh->affected; pAf != NULL; pAf = pAf->next )
        {
          if ( IS_DELETED( pAf ) )
        n_daf[pAf->list]++;
          n_aff[pAf->list]++;
        }
    }
    }

  n0 = n1 = n2 = n3 = 0;
  for ( n4 = 0; n4 < AFF_MAX_LIST; n4++ )
    {
      n0 += top_affect[n4];
      n1 += n_aff[n4];
      n3 += n_daf[n4];
      for ( pAf = affect_free[n4]; pAf != NULL; pAf = pAf->next )
    n_faf[n4]++;
      n2 += n_faf[n4];
    }

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pAf ), n0 * sizeof( *pAf ) );

  bprintf( buffer, "Affects:        %s%6d %6d %5d %5d (%d deleted)\n\r",
       buf, n0, n1, n2, n0 - n1 - n2, n3 );

  for ( n4 = 0; n4 < AFF_MAX_LIST; n4++ )
    {
      char *sAf;

      switch ( n4 )
    {
    default:        sAf = "Unknown:";    break;
    case AFF_OBJ:        sAf = "Object:";    break;
    case AFF_OBJ_INDEX:    sAf = "ObjIndex:";    break;
    case AFF_PC:        sAf = "Player:";    break;
    case AFF_NPC:        sAf = "Mobile:";    break;
    case AFF_ROOM:        sAf = "Room:";        break;
    }

      if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pAf ), top_affect[n4] * sizeof( *pAf ) );

      bprintf( buffer, "  %-13s %s%6d %6d %5d %5d (%d deleted)\n\r",
           sAf, buf, top_affect[n4], n_aff[n4], n_faf[n4],
           top_affect[n4] - n_aff[n4] - n_faf[n4], n_daf[n4] );
    }

  n1 = n2 = 0;
  for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    n1++;
  for ( pArea = area_free; pArea != NULL; pArea = pArea->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pArea ), top_area * sizeof( *pArea ) );

  bprintf( buffer, "Areas:          %s%6d %6d %5d %5d\n\r",
       buf, top_area, n1, n2, top_area - n1 - n2 );

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pBan ), top_ban * sizeof( *pBan ) );

  n1 = n2 = 0;
  for ( pBan = ban_list; pBan != NULL; pBan = pBan->next )
    n1++;
  for ( pBan = ban_free; pBan != NULL; pBan = pBan->next )
    n2++;

  bprintf( buffer, "Bans:           %s%6d %6d %5d %5d\n\r",
       buf, top_ban, n1, n2, top_ban - n1 - n2 );

  n2 = 0;
  for ( pBuf = buffer_free; pBuf != NULL; pBuf = pBuf->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pBuf ), top_buffer * sizeof( *pBuf ) );

  bprintf( buffer, "Buffers:        %s%6d %6d %5d %5d\n\r",
       buf, top_buffer, n_buf, n2, top_buffer - n_buf - n2 );

  n2 = 0;
  for ( pCh = char_free; pCh != NULL; pCh = pCh->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pCh ), top_char * sizeof( *pCh ) );

  bprintf( buffer, "Characters:     %s%6d %6d %5d %5d (%d deleted)\n\r",
       buf, top_char, n_ch, n2, top_char - n_ch - n2, n_dch );

  n2 = 0;
  for ( d = descriptor_free; d != NULL; d = d->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *d ), top_descriptor * sizeof( *d ) );

  bprintf( buffer, "Descriptors:    %s%6d %6d %5d %5d\n\r",
       buf, top_descriptor, n_d, n2, top_descriptor - n_d - n2 );

  n2 = 0;
  for ( pEd = extra_descr_free; pEd != NULL; pEd = pEd->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pEd ), top_ed * sizeof( *pEd ) );

  bprintf( buffer, "Extra descr:    %s%6d %6d %5d %5d\n\r",
       buf, top_ed, n_ed, n2, top_ed - n_ed - n2 );

  n2 = 0;
  for ( pExit = exit_free; pExit != NULL; pExit = pExit->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pExit ), top_exit * sizeof( *pExit ) );

  bprintf( buffer, "Exits:          %s%6d %6d %5d %5d\n\r",
       buf, top_exit, n_ex, n2, top_exit - n_ex - n2 );

  n1 = n2 = 0;
  for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    n1++;
  for ( pHelp = help_free; pHelp != NULL; pHelp = pHelp->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pHelp ), top_help * sizeof( *pHelp ) );

  bprintf( buffer, "Help items:     %s%6d %6d %5d %5d\n\r",
       buf, top_help, n1, n2, top_help - n1 - n2 );

  n1 = n2 = 0;
  for ( pHelpArea = help_area_first; pHelpArea != NULL;
    pHelpArea = pHelpArea->next )
    n1++;
  for ( pHelpArea = help_area_free; pHelpArea != NULL;
    pHelpArea = pHelpArea->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pHelpArea ), top_help_area * sizeof( *pHelpArea ) );

  bprintf( buffer, "Help areas:     %s%6d %6d %5d %5d\n\r",
       buf, top_help_area, n1, n2, top_help_area - n1 - n2 );

  n2 = 0;
  for ( pMobIndex = mob_index_free; pMobIndex != NULL;
    pMobIndex = pMobIndex->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pMobIndex ), top_mob_index * sizeof( *pMobIndex ) );

  bprintf( buffer, "Mobile indices: %s%6d %6d %5d %5d (%d new)\n\r",
       buf, top_mob_index, n_min, n2, top_mob_index - n_min - n2, newmobs );

  n2 = 0;
  for ( pProg = mprog_free; pProg != NULL; pProg = pProg->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pProg ), top_mprog * sizeof( *pProg ) );

  bprintf( buffer, "MobProg links:  %s%6d %6d %5d %5d\n\r",
       buf, top_mprog, n_mp, n2, top_mprog - n_mp - n2 );

  n2 = 0;
  for ( pNote = note_list; pNote != NULL; pNote = pNote->next ) n_not++;
  for ( pNote = idea_list; pNote != NULL; pNote = pNote->next ) n_not++;
  for ( pNote = penalty_list; pNote != NULL; pNote = pNote->next ) n_not++;
  for ( pNote = news_list; pNote != NULL; pNote = pNote->next ) n_not++;
  for ( pNote = changes_list; pNote != NULL; pNote = pNote->next ) n_not++;
  for ( pNote = note_free; pNote != NULL; pNote = pNote->next ) n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pNote ), top_note * sizeof( *pNote ) );

  bprintf( buffer, "Notes:          %s%6d %6d %5d %5d\n\r",
       buf, top_note, n_not, n2, top_note - n_not - n2 );

  n2 = 0;
  for ( pObjIndex = obj_index_free; pObjIndex != NULL;
    pObjIndex = pObjIndex->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pObjIndex ), top_obj_index * sizeof( *pObjIndex ) );

  bprintf( buffer, "Object indices: %s%6d %6d %5d %5d (%d new)\n\r",
       buf, top_obj_index, n_oin, n2, top_obj_index - n_oin - n2, newobjs );

  n2 = 0;
  for ( pObj = obj_free; pObj != NULL; pObj = pObj->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pObj ), top_object * sizeof( *pObj ) );

  bprintf( buffer, "Objects:        %s%6d %6d %5d %5d (%d deleted)\n\r",
       buf, top_object, n_obj, n2, top_object - n_obj - n2, n_dob );

  n2 = 0;
  for ( pProg = oprog_free; pProg != NULL; pProg = pProg->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pProg ), top_oprog * sizeof( *pProg ) );

  bprintf( buffer, "ObjProg links:  %s%6d %6d %5d %5d\n\r",
       buf, top_oprog, n_op, n2, top_oprog - n_op - n2 );

  n2 = 0;
  for ( pc = pcdata_free; pc != NULL; pc = pc->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pc ), top_pcdata * sizeof( *pc ) );

  bprintf( buffer, "PC data:        %s%6d %6d %5d %5d\n\r",
       buf, top_pcdata, n_pcd, n2, top_pcdata - n_pcd - n2 );

  n2 = 0;
  for ( pGen = gen_data_free; pGen != NULL; pGen = pGen->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pGen ), top_gen * sizeof( *pGen ) );

  bprintf( buffer, "PC generation:  %s%6d %6d %5d %5d\n\r",
       buf, top_gen, n_gen, n2, top_gen - n_gen - n2 );

  n1 = n2 = 0;
  for ( pCode = mprog_list; pCode != NULL; pCode = pCode->next )
    n1++;
  for ( pCode = mpcode_free; pRoom != NULL; pCode = pCode->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pCode ), top_mpcode_index * sizeof( *pCode ) );

  bprintf( buffer, "Program code:   %s%6d %6d %5d %5d\n\r",
       buf, top_mpcode_index, n1, n2, top_mpcode_index - n1 - n2 );

  n2 = 0;
  for ( pReset = reset_free; pReset != NULL; pReset = pReset->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pReset ), top_reset * sizeof( *pReset ) );

  bprintf( buffer, "Resets:         %s%6d %6d %5d %5d\n\r",
       buf, top_reset, n_res, n2, top_reset - n_res - n2 );

  n2 = 0;
  for ( pRoom = room_index_free; pRoom != NULL; pRoom = pRoom->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pRoom ), top_room * sizeof( *pRoom ) );

  bprintf( buffer, "Rooms:          %s%6d %6d %5d %5d\n\r",
       buf, top_room, n_rin, n2, top_room - n_rin - n2 );

  n2 = 0;
  for ( pProg = rprog_free; pProg != NULL; pProg = pProg->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pProg ), top_rprog * sizeof( *pProg ) );

  bprintf( buffer, "RoomProg links: %s%6d %6d %5d %5d\n\r",
       buf, top_rprog, n_rp, n2, top_rprog - n_rp - n2 );

  n1 = n2 = 0;
  for ( rd = roster_first; rd != NULL; rd = rd->next )
    n1++;
  for ( rd = roster_free; rd != NULL; rd = rd->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *rd ), top_roster * sizeof( *rd ) );

  bprintf( buffer, "Roster:         %s%6d %6d %5d %5d\n\r",
       buf, top_roster, n1, n2, top_roster - n1 - n2 );

  n1 = n2 = 0;
  for ( pShop = shop_first; pShop != NULL; pShop = pShop->next )
    n1++;
  for ( pShop = shop_free; pShop != NULL; pShop = pShop->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pShop ), top_shop * sizeof( *pShop ) );

  bprintf( buffer, "Shops:          %s%6d %6d %5d %5d\n\r",
       buf, top_shop, n1, n2, top_shop - n1 - n2 );

  n1 = n2 = 0;
  for ( pSocial = social_first; pSocial != NULL; pSocial = pSocial->next )
    n1++;
  for ( pSocial = social_free; pSocial != NULL; pSocial = pSocial->next )
    n2++;

  if ( show_bytes )
    mprintf( sizeof(buf), buf, "| %5d %8d | ",
         sizeof( *pSocial ), top_social * sizeof( *pSocial ) );

  bprintf( buffer, "Socials:        %s%6d %6d %5d %5d\n\r\n\r",
       buf, top_social, n1, n2, top_social - n1 - n2 );

  bprintf( buffer, "Strings:  %6d strings = %8ld bytes (%ld max)\n\r",
       nAllocString, sAllocString, MAX_STRING );

  bprintf( buffer, " (free):  %6s         = %8ld bytes (%1.1f%%)\n\r",
       "", MAX_STRING - sAllocString,
       ( MAX_STRING - sAllocString ) * 100.0 / MAX_STRING );

  bprintf( buffer, " (dup):   %6d strings = %8ld bytes (saved)\n\r",
       nDupString, sDupString );

  bprintf( buffer, "Perms:    %6d blocks  = %8ld bytes\n\r",
       nAllocPerm, sAllocPerm );

  bprintf( buffer, " (boot):  %6d blocks  = %8ld bytes\n\r",
       nBootAllocPerm, sBootAllocPerm );

  bprintf( buffer, " (diff):  %6d blocks  = %8ld bytes\n\r",
       nAllocPerm - nBootAllocPerm, sAllocPerm - sBootAllocPerm );

  bprintf( buffer, "Total:    %6d blocks  = %8ld bytes\n\r",
       nAllocPerm + nAllocString, sAllocPerm + sAllocString );

  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );
}
#endif
void parse_credits(AREA_DATA *pArea)
{
  if (pArea->low_range ==0 && pArea->high_range ==0) {
    pArea->low_range = get_area_level(pArea) / (MAX_LEVEL+1);
    pArea->high_range = get_area_level(pArea) % (MAX_LEVEL+1);
  }
}
/*
 * Return string at the first occurrence of c in s, case insensitive.
 * Return NULL if not found.
 */
char *str_chr( const char *s, const char c )
{
  const char ch = LOWER( c );

  for ( ; LOWER( *s ) != ch; ++s )
    if ( *s == '\0' )
      return NULL;
  return (char *)s;
}

/*
 * Check for bad keys with containers.
 */
void fix_objects( void )
{
  OBJ_INDEX_DATA    *pObjIndex;
  OBJ_INDEX_DATA    *pObjKey;
  ROOM_INDEX_DATA       *pRoomKey;
  int            iHash;

  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
      for ( pObjIndex = obj_index_hash[iHash]; pObjIndex != NULL;
        pObjIndex = pObjIndex->next )
    {

      if ( pObjIndex->item_type == ITEM_CONTAINER )
        {
          if (  IS_SET( pObjIndex->value[1], CONT_LOCKED ) )
        {
          /* Object is locked but not closed. */
          if ( !IS_SET( pObjIndex->value[1], CONT_CLOSED ) )
            {
              bugf( "Fix_objects: obj %d locked but not closed",
                pObjIndex->vnum );
            }

          /* Object is locked & pickproof with no key. */
          if ( pObjIndex->value[2] <= 0
               && IS_SET( pObjIndex->value[1], CONT_PICKPROOF ) )
            {
              bugf( "Fix_objects: obj %d locked, no key %d",
                pObjIndex->vnum, pObjIndex->value[2] );
            }
        }

          /* No key specified, so don't check the rest of this. */
          if ( pObjIndex->value[2] <= 0 )
        continue;

          /* Item has key but isn't closeable. */
          if ( !IS_SET( pObjIndex->value[1], CONT_CLOSEABLE ) )
        {
          bugf( "Fix_objects: obj %d not closeable, key %d",
            pObjIndex->vnum, pObjIndex->value[2] );
        }

          fBootDb = FALSE;
          pObjKey = get_obj_index( pObjIndex->value[2] );
          fBootDb = TRUE;

          /* Key vnum doesn't exist. */
          if ( pObjKey == NULL )
        {
          bugf( "Fix_objects: obj %d, no key %d",
            pObjIndex->vnum, pObjIndex->value[2] );
          continue;
        }

          /* Key vnum isn't an KEY item type. */
          if ( pObjKey->item_type != ITEM_KEY )
        {
          bugf( "Fix_objects: obj %d, bad key %d",
            pObjIndex->vnum, pObjIndex->value[2] );
          continue;
        }
        }
      else if ( pObjIndex->item_type == ITEM_PORTAL )
        {
          if ( IS_SET( pObjIndex->value[1], EX_LOCKED ) )
        {
          /* Object is locked but not a door.*/
          if ( !IS_SET( pObjIndex->value[1], EX_ISDOOR ) )
            {
              bugf( "Fix_objects: portal %d locked but not a door",
                pObjIndex->vnum );
            }

          /* Object is locked but not closed.*/
          if ( !IS_SET( pObjIndex->value[1], EX_CLOSED ) )
            {
              bugf( "Fix_objects: portal %d locked but not closed",
                pObjIndex->vnum );
            }

          /* Object is locked, nopass, & pickproof with no
              key. */
          if ( pObjIndex->value[4] <= 0
               && IS_SET( pObjIndex->value[1], EX_PICKPROOF )
               && IS_SET( pObjIndex->value[1], EX_NOPASS ) )
            {
              bugf( "Fix_objects: portal %d locked, no key %d",
                pObjIndex->vnum, pObjIndex->value[4] );
            }
        }

          /* No key specified, so don't check the rest of this. */
          if ( pObjIndex->value[4] <= 0 )
        continue;

          /* Item has key but isn't a door. */
          if ( !IS_SET( pObjIndex->value[1], EX_ISDOOR ) )
        {
          bugf( "Fix_objects: portal %d not a door, key %d",
            pObjIndex->vnum, pObjIndex->value[4] );
        }

          /* Item has key but isn't closeable. */
          if ( IS_SET( pObjIndex->value[1], EX_NOCLOSE ) )
        {
          bugf( "Fix_objects: portal %d not closeable, key %d",
            pObjIndex->vnum, pObjIndex->value[4] );
        }

          /* Item has key but isn't lockable.*/
          if ( IS_SET( pObjIndex->value[1], EX_NOLOCK ) )
        {
          bugf( "Fix_objects: portal %d not lockable, key %d",
            pObjIndex->vnum, pObjIndex->value[4] );
        }

          fBootDb = FALSE;
          pObjKey = get_obj_index( pObjIndex->value[4] );
          fBootDb = TRUE;
          fBootDb = FALSE;
          pRoomKey = get_room_index(pObjIndex->value[3]);
          fBootDb = TRUE;
          if (pRoomKey == NULL)
        {
          bugf( "Fix_objects: portal %d, No destination Vnum %d.",
            pObjIndex->vnum, pObjIndex->value[3]);
          continue;
        }
          /* Key vnum doesn't exist.*/
          if ( pObjKey == NULL )
        {
          bugf( "Fix_objects: portal %d, no key %d",
            pObjIndex->vnum, pObjIndex->value[4] );
          continue;
        }

          /* Key vnum isn't an KEY item type.*/
          if ( pObjKey->item_type != ITEM_KEY )
        {
          bugf( "Fix_objects: portal %d, bad key %d",
            pObjIndex->vnum, pObjIndex->value[4] );
          continue;
        }
          
        }
    }
    }
}
/*
 * Used for updating time while booting.
 */
void update_current_time( void )
{
#if defined(macintosh) || defined(MSDOS)
    struct timeval last_time;
    current_time = (time_t) last_time.tv_sec;
#elif defined(unix)
    struct timeval last_time;
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;
#endif
}


/*
 * Formatted output for log_string.
 */
int loggedf( const char *format, ... )
{
    char    buf[MAX_STRING_LENGTH];
    va_list    args;
    int     result;

    if ( format == NULL )
    {
    bugf( "Logf: NULL format" );
    return -1;
    }

    va_start( args, format );
    result = vsnprintf( buf, sizeof( buf ), format, args );
    va_end( args );

    if ( result < 0 )
    bug( "Logf: string overflowed buffer (%d bytes)", sizeof( buf ) );

    log_string( buf );

    return 0; // Dummy return value
}
