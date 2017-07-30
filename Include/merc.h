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

#include "bank.h"
#define MERC_H 1

/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )             ( )
#define DECLARE_DO_FUN( fun )    void fun( )
#define DECLARE_SPEC_FUN( fun )  bool fun( )
#define DECLARE_SPELL_FUN( fun ) void fun( )
#else
#define args( list )             list
#define DECLARE_DO_FUN( fun )    DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )  SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun ) SPELL_FUN fun
#endif

//comment out to revert to old creation
#define NEWCREATION 1

/* system calls */
int unlink();
//int system();  -- old prototype was wrong
int system( const char *command );

#define NO_SECRET 1

#define INQUISITOR

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if    !defined(FALSE)
#define FALSE     0
#endif

#if    !defined(TRUE)
#define TRUE     1
#endif

/* This is for our expanded 64-bit bitvals
   it means the code will no longer compile
   on many older systems. -- Taeloch */
typedef unsigned long long int int64;

#if    defined(_AIX)
#if    !defined(const)
#define const
#endif
typedef int           sh_int;
typedef int           bool;
#define unix
#else
typedef short   int   sh_int;
#if !defined(USE_CPP)
typedef unsigned char bool;
#endif
#endif

/* mccp: support bits */
#if !defined(SPARC)
#include <zlib.h>

#define TELOPT_COMPRESS  85
#define TELOPT_COMPRESS2 86

//#define COMPRESS_BUF_SIZE 16384
#define COMPRESS_BUF_SIZE 1024
#endif


/*
 * Structure types.
 */
typedef struct affect_data      AFFECT_DATA;
typedef struct trap_data        TRAP_DATA;
typedef struct area_data        AREA_DATA;
typedef struct clan_data        CLAN_DATA;
typedef struct roster_data      ROSTER_DATA;
typedef struct rank_data        RANK_DATA;
typedef struct ban_data         BAN_DATA;
typedef struct buf_type         BUFFER;
typedef struct char_data        CHAR_DATA;
typedef struct denied_data      DENIED_DATA;
typedef struct descriptor_data  DESCRIPTOR_DATA;
typedef struct exit_data        EXIT_DATA;
typedef struct extra_descr_data EXTRA_DESCR_DATA;
typedef struct help_data        HELP_DATA;
typedef struct kill_data        KILL_DATA;
typedef struct mem_data         MEM_DATA;
typedef struct mob_index_data   MOB_INDEX_DATA;
typedef struct note_data        NOTE_DATA;
typedef struct obj_data         OBJ_DATA;
typedef struct obj_index_data   OBJ_INDEX_DATA;
typedef struct pc_data          PC_DATA;
typedef struct quest_data       QUEST_DATA;
typedef struct gen_data         GEN_DATA;
typedef struct reset_data       RESET_DATA;
typedef struct room_index_data  ROOM_INDEX_DATA;
typedef struct shop_data        SHOP_DATA;
typedef struct time_info_data   TIME_INFO_DATA;
typedef struct weather_data     WEATHER_DATA;
typedef struct mprog_list       MPROG_LIST;
typedef struct mprog_code       MPROG_CODE;
typedef struct hate_data        HATE_DATA;
typedef struct chan_hist_st     CHAN_HIST;
typedef struct char_cmd_hist_st CMD_HIST;
/*
 * Function types.
 */
typedef void DO_FUN    args( ( CHAR_DATA *ch, char *argument ) );
typedef bool SPEC_FUN  args( ( CHAR_DATA *ch ) );
typedef void SPELL_FUN args( ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) );


/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH        1024
#define MAX_STRING_LENGTH   8192
#define MAX_INPUT_LENGTH    256
#define PAGELEN             24
#define MAX_EXITS           6
#define MSL MAX_STRING_LENGTH
#define MIL MAX_INPUT_LENGTH

/*
 * Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing.
 */
#define MAX_STRING           8192000
#define MAX_PERM_BLOCK       262144
#define MAX_MEM_LIST         14
#define MAX_ALLOC_SIZE       262144
#define DEFAULT_DESC_OUTSIZE 2000


/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
/*#define MAX_SOCIALS           256 */
#define MAX_SKILL             387 //Last added Double Shot - 2014-05-02
#define MAX_DAMAGE            2200
#define MAX_RACE_SKILLS       5
#define MAX_MAINTAINED        25
#define MAX_GROUP             54 // last added leadership - 2010-06-01
#define MAX_IN_GROUP          30
#define MAX_IN_RACE           5
#define MAX_ALIAS             30
#define MAX_CLASS             18
#define MAX_PC_RACE           45
#define MAX_CLAN              20
#define MAX_CONTINENT         4
#define MAX_CHAOS_NAMES       2
#define MAX_DAMAGE_MESSAGE    44 //Bolt - 2014-05-02
#define MAX_LEVEL             109
#define MAX_LIQUID            43
#define MIN_AGGRO_LEVELS      5  /* Taeloch: level difference for aggro to work */
#define MAX_AGGRO_LEVELS      30 /* Taeloch: level difference for aggro to work */
#define MIN_AFK_TIME          5
#define MAX_AFK_TIME          20
#define LEVEL_CHANNEL         2
#define LEVEL_GRP_RANGE       7
#define LEVEL_REVEAL_AC       25
#define LEVEL_REVEAL_ALIGN    15
#define LEVEL_GUARD_CHECK_RNG 5
#define LEVEL_RECALL_COSTS    15
#define LEVEL_CLAN            20
#define LEVEL_KILLRANGE       10
#define NUM_CLANNIES          12
#define LEVEL_ASSIST          6
#define LEVEL_STEAL_RNG       7
#define LEVEL_NEWBIE          5
#define MAX_PLAYERS           1000
#define LEVEL_HERO            (MAX_LEVEL - 9)
#define LEVEL_IMMORTAL        (MAX_LEVEL - 8)
#define SUPER_MAX_ITEMS       150000
#define GHOST_TIME            15
#define VFLAG_TIME            60
#define PULSE_PER_SECOND      4
#define PULSE_VIOLENCE        ( 3 * PULSE_PER_SECOND)
#define PULSE_MOBILE          ( 4 * PULSE_PER_SECOND)
#define PULSE_MUSIC           ( 6 * PULSE_PER_SECOND)
#define PULSE_UNDERWATER      (20 * PULSE_PER_SECOND)
#define PULSE_TICK            (60 * PULSE_PER_SECOND)
#define PULSE_AREA            (120 * PULSE_PER_SECOND)
#define PULSE_ASAVE           (600 * PULSE_PER_SECOND)  /* 10 minutes */
#define PULSE_HOUR            (3600 * PULSE_PER_SECOND) /* 60 minutes */
#define PULSE_CHAR_ACT        (10 * PULSE_PER_SECOND) // created for fishing

#define IMPLEMENTOR           MAX_LEVEL
#define CREATOR               (MAX_LEVEL - 2)
#define SUPREME               (MAX_LEVEL - 3)
#define DEITY                 (MAX_LEVEL - 4)
#define GOD                   (MAX_LEVEL - 5)
#define IMMORTAL              (MAX_LEVEL - 6)
#define DEMI                  (MAX_LEVEL - 7)
#define ANGEL                 (MAX_LEVEL - 8)
#define AVATAR                (MAX_LEVEL - 9)
#define HERO                  LEVEL_HERO

#define PLAYER_KILL_LEVEL     5
#define MAX_QUEST_PER_LEVEL   1000
#define QUEST_STRING_SIZE     6
/*
 * Colour stuff by Lope of Loping Through The MUD
 */
/*#define CLEAR        "[0m[0;37m"         Resets Colour    */
#define C_RED       "[0;31m"    /* Normal Colours    */
#define C_GREEN     "[0;32m"
#define C_YELLOW    "[0;33m"
#define C_BLUE      "[0;34m"
#define C_MAGENTA   "[0;35m"
#define C_CYAN      "[0;36m"
#define C_WHITE     "[0;37m"
//#define C_BLINK     "^[[0;5m"
#define CLEAR       C_WHITE
#define C_D_GREY    "[1;30m"      /* Light Colors        */
#define C_B_RED     "[1;31m"
#define C_B_GREEN   "[1;32m"
#define C_B_YELLOW  "[1;33m"
#define C_B_BLUE    "[1;34m"
#define C_B_MAGENTA "[1;35m"
#define C_B_CYAN    "[1;36m"
#define C_B_WHITE   "[1;37m"
#define C_BLINK     "[5m"
#define C_UNDERLINE "[4m"
#define C_REVERSE   "[7m"
/*
 * Site ban structure.
 */

/*
 * Site ban structure.
 */

#define BAN_SUFFIX    (A)
#define BAN_PREFIX    (B)
#define BAN_NEWBIES   (C)
#define BAN_ALL       (D)
#define BAN_PERMIT    (E)
#define BAN_PERMANENT (F)
#define BAN_AUTOLOG   (G)
#define BAN_NOCHAN    (H)
#define BAN_DURATION  (I)
#define BAN_NONOTE    (J)
#define BAN_NOIDENT   (Z) // Some firewall users have 3-4 minute lag

#define LIST_SEARCH_ROOM    0
#define LIST_SEARCH_MOB     1
#define LIST_SEARCH_OBJ     2
#define LIST_SEARCH_MPROG   3

struct ban_data
{
  BAN_DATA *   next;
  unsigned int valid;
  sh_int       ban_flags;
  sh_int       level;
  time_t       date_stamp;
  sh_int       duration;
  char *       name;
};

struct buf_type
{
  BUFFER * next;
  bool     valid;
  sh_int   state;  /* error state of the buffer */
  sh_int   size;   /* size in k */
  char *   string; /* buffer's string */
  int      len;
};



/*
 * Time and weather stuff.
 */
#define SUN_DARK      0
#define SUN_RISE      1
#define SUN_LIGHT     2
#define SUN_SET       3

#define SKY_CLOUDLESS 0
#define SKY_CLOUDY    1
#define SKY_RAINING   2
#define SKY_LIGHTNING 3

struct time_info_data
{
  int hour;
  int day;
  int month;
  int year;
};

struct weather_data
{
  int mmhg;
  int change;
  int sky;
  int sunlight;
};

/*
 * Communications/Channel Defn's
 */

#define MAX_CHANNEL      24
#define CHAN_ALERT       0
#define CHAN_GOSSIP      1
#define CHAN_OOC         2
#define CHAN_WARTALK     3
#define CHAN_POLITICAL   4
#define CHAN_CLAN        5
#define CHAN_MUSIC       6
#define CHAN_QUESTION    7
#define CHAN_QUOTE       8
#define CHAN_GRATS       9
#define CHAN_SHOUT       10
#define CHAN_IMMTALK     11
#define CHAN_AUCTION     12
#define CHAN_GAMESMASTER 13
#define CHAN_SEXTALK     14
#define CHAN_SEXPOLICE   15
#define CHAN_HINT        16
#define CHAN_PRAYER      17
#define CHAN_TELL        18
#define CHAN_GROUPTELL   19
#define CHAN_SAY         20
#define CHAN_IRL         21
#define CHAN_IMMSAY      22
#define CHAN_YELL        23

#define MAX_HIST         20

struct chan_hist_st
{
  char * text;
};

#define MAX_COMMAND_HISTORY 10

struct char_cmd_hist_st
{
  char * text;
};


/*
 * Connected state for a channel.
 */
#define CON_PLAYING              0
#define CON_GET_NAME             1
#define CON_GET_OLD_PASSWORD     2
#define CON_CONFIRM_NEW_NAME     3
#define CON_GET_NEW_PASSWORD     4
#define CON_CONFIRM_NEW_PASSWORD 5
#define CON_GET_NEW_RACE         6
#define CON_GET_NEW_SEX          7
#define CON_GET_NEW_CLASS        8
#define CON_GET_ALIGNMENT        9
#define CON_DEFAULT_CHOICE       10
#define CON_GEN_GROUPS           11
#define CON_PICK_WEAPON          12
#define CON_READ_IMOTD           13
#define CON_READ_MOTD            14
#define CON_BREAK_CONNECT        15
#define CON_COPYOVER_RECOVER     16
#define CON_ANSI                 17
#define CON_ROLL_STATS           18
#define CON_GET_STORY            19
#define CON_GET_STORY2           20
#define CON_GET_NEWBIE           21
#define CON_GET_DISCLAIMER       22
#define CON_GET_CONFIRMATION     23
#define CON_GET_ANSI_COLOR       24
#define CON_PROMPT_DISCLAIMER    25
#define CON_NEW_CREATION         26

/*
 * Descriptor (channel) structure.
 */
struct descriptor_data
{
  DESCRIPTOR_DATA * next;
  DESCRIPTOR_DATA * snoop_by;
  CHAR_DATA       * character;
  CHAR_DATA       * original;
  bool              valid;
  char            * host;
  sh_int            descriptor;
  sh_int            connected;
  bool              fcommand;
  char              inbuf  [4 * MAX_INPUT_LENGTH];
  char              incomm [MAX_INPUT_LENGTH];
  char              inlast [MAX_INPUT_LENGTH];
  int               repeat;
  char            * outbuf;
  int               outsize;
  int               outtop;
  char            * showstr_head;
  char            * showstr_point;
  int               showstr_size;
  sh_int            showstr_page;
  sh_int            showstr_page_count;
  bool              ansi;
  void            * pEdit;   /* OLC */
  char           ** pString; /* OLC */
  int               editor;  /* OLC */

#if !defined(SPARC)
  /* mccp: support data */
  z_stream      * out_compress;
  unsigned char * out_compress_buf;
  unsigned char   compressing;
#endif
};

struct denied_data {
  char *name;
  DENIED_DATA *next;
};

enum sex_dir_enum {upward_e,downward_e};


/*
 * Attribute bonus structures.
 */
struct str_app_type
{
  sh_int tohit;
  sh_int todam;
  sh_int carry;
  sh_int wield;
};

struct int_app_type
{
  sh_int learn;
  sh_int mana_add;
  sh_int manap;
};

struct wis_app_type
{
  sh_int practice;
  sh_int frag_percent;
};

struct dex_app_type
{
  sh_int defensive;
  sh_int movep;
  sh_int item_bonus;
};

struct con_app_type
{
  sh_int hitp;
  sh_int shock;
  sh_int hp_add;
};


struct avg_dam_st {
  sh_int num;
  sh_int dice;
};

/*
 * TO types for act.
 */
#define TO_ROOM        0  /* Room only, except character. */
#define TO_NOTVICT     1  /* Room only, except character & victim. */
#define TO_VICT        2  /* Victim only. */
#define TO_CHAR        3  /* Character only. */
#define TO_ALL         4  /* Everyone in room. */
#define TO_ROOM_ALL    4  /* Everyone in room. */
#define TO_WORLD       5  /* Every player in mud, except character. */
#define TO_AREA        6  /* Every player in area, except character. */
#define TO_GROUP       7  /* Every player in group, except character. */
#define TO_GROUP_ROOM  8  /* Every player in group & in same room except character. */
#define TO_WIZ_ROOM    9  /* As TO_ROOM except wizinvis chars are silent. */
#define TO_WIZ_NOTVICT 10 /* As TO_NOTVICT except wizinvis chars are silent. */
#define TO_WIZ_VICT    11 /* As TO_VICT except wizinvis chars are silent. */

/*
 * Help table types.
 */
struct help_data
{
  HELP_DATA * next;
  sh_int      level;
  char *      keyword;
  char *      text;
  char *      synopsis;
  int         vnum;
  bool        delete_it;
};

/*
 * Shop types.
 */
#define MAX_TRADE 5

struct shop_data
{
  SHOP_DATA * next;                 /* Next shop in list           */
  sh_int      keeper;               /* Vnum of shop keeper mob     */
  sh_int      buy_type [MAX_TRADE]; /* Item types shop will buy    */
  sh_int      profit_buy;           /* Cost multiplier for buying  */
  sh_int      profit_sell;          /* Cost multiplier for selling */
  sh_int      open_hour;            /* First opening hour          */
  sh_int      close_hour;           /* First closing hour          */
};


/*
 * Per-class stuff.
 */

#define MAX_GUILD 4
#define MAX_STATS 5
#define MAX_APPR  7
#define STAT_STR  0
#define STAT_INT  1
#define STAT_WIS  2
#define STAT_DEX  3
#define STAT_CON  4

struct    class_type
{
  char   name[MSL];          /* the full name of the class  */
  char   who_name[9];        /* Three-letter name for 'who' */
  sh_int attr_prime;         /* Prime attribute             */
  sh_int weapon;             /* First weapon                */
  sh_int guild[MAX_GUILD];   /* Vnum of guild rooms         */
  sh_int skill_adept;        /* Maximum skill level         */
  sh_int thac0_00;           /* Thac0 for level  0          */
  sh_int thac0_32;           /* Thac0 for level 32          */
  sh_int hp_min;             /* Min hp gained on leveling   */
  sh_int hp_max;             /* Max hp gained on leveling   */
  bool   fMana;              /* Class gains mana on level   */
  char   base_group[MSL];    /* base skills gained          */
  char   default_group[MSL]; /* default skills gained       */
};

struct item_type
{
  int    type;
  char * name;
};

struct weapon_type
{
  char   * name;
  sh_int   vnum;
  sh_int   type;
  sh_int * gsn;
  char   * skillname;
};

struct wiznet_type
{
  char * name;
  long   flag;
  int    level;
};

struct game_type
{
  char   * name;
  DO_FUN * function;
};

struct attack_type
{
  char * name;            /* name */
  char * noun;            /* message */
  int    damage;          /* damage class */
};

struct race_type
{
  char * name;           /* call name of the race */
  bool   pc_race;        /* can be chosen by pcs */
  int64  act;            /* act bits for the race */
  int64  aff;            /* aff bits for the race */
  int64  aff2;           /* aff2 bits for the races */
  int64  off;            /* off bits for the race */
  int64  imm;            /* imm bits for the race */
  int64  res;            /* res bits for the race */
  int64  vuln;           /* vuln bits for the race */
  int64  form;           /* default form flag for the race */
  int64  parts;          /* default parts for the race */
  long   native_sect;    /* native sector type for race */
};


struct pc_race_type  /* additional data for pc races */
{
  char   * name;                      /* MUST be in race_type */
  char     who_name[15];
  sh_int   points;                    /* cost in points of the race */
  sh_int   class_mult[MAX_CLASS];     /* exp multiplier for class, * 100 */
  char   * skills[MAX_RACE_SKILLS];   /* bonus skills for the race */
  sh_int   stats[MAX_STATS];          /* starting stats */
  sh_int   max_stats[MAX_STATS];      /* maximum stats */
  sh_int   size;                      /* aff bits for the race */
  char   * eye_color[MAX_APPR];
  char   * hair_color[MAX_APPR];
  char   * skin_color[MAX_APPR];
  sh_int   min_weight;                // Minimun weight a race can be
  sh_int   max_weight;                // Max weight
  sh_int   min_height;                // Shortest a race can be (in inches)
  sh_int   max_height;                // Tallest height
};


struct spec_type
{
  char     * name;
  SPEC_FUN * function;
};

/*
 * Data structure for notes.
 */

#define NOTE_NOTE    0
#define NOTE_IDEA    1
#define NOTE_PENALTY 2
#define NOTE_NEWS    3
#define NOTE_CHANGES 4
#define NOTE_RULES   5
#define NOTE_RPNOTE  6

struct note_data
{
  NOTE_DATA * next;
  bool        valid;
  sh_int      type;
  char      * sender;
  char      * date;
  char      * to_list;
  char      * subject;
  char      * text;
  time_t      date_stamp;
  bool        reply;
};

/*
 * An affect.
 */
struct affect_data
{
  AFFECT_DATA * next;
  bool          valid;
  sh_int        where;
  sh_int        type;
  sh_int        level;
  sh_int        duration;
  sh_int        location;
  sh_int        modifier;
  int64         bitvector;
};

/* where definitions */
#define TO_AFFECTS       0
#define TO_OBJECT        1
#define TO_IMMUNE        2
#define TO_RESIST        3
#define TO_VULN          4
#define TO_WEAPON        5
#define TO_SPELL_AFFECTS 6
#define TO_ACT           7
#define TO_COMM          8
#define TO_PENALTY       9
#define TO_CHANNEL       10
#define TO_AFFECTS2      11
#define TO_DUMMY         15

/*
 * A kill structure (indexed by level).
 */
struct kill_data
{
  sh_int number;
  sh_int killed;
};


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_FIDO            3090
#define MOB_VNUM_CITYGUARD       3060
#define MOB_VNUM_VAMPIRE         3404
#define MOB_VNUM_COW             613
#define MOB_VNUM_WOLF            3094
#define MOB_VNUM_BEAR            3715
#define MOB_VNUM_CAT             3066
#define MOB_VNUM_SNAIL           3713
#define MOB_VNUM_BOAR            3711
#define MOB_VNUM_SLIME           3422
#define MOB_VNUM_RABBIT          3709
#define MOB_VNUM_PATROLMAN       2106
#define GROUP_VNUM_TROLLS        2100
#define GROUP_VNUM_OGRES         2101
#define MOB_VNUM_ZOMBIE          10668
#define MOB_VNUM_FAMILIAR        1363
#define MOB_VNUM_FIRE_ELEMENTAL  7719
#define MOB_VNUM_ICE_ELEMENTAL   10439
#define MOB_VNUM_NEWBIE_HINT     3016
#define MOB_VNUM_CLAY_GOLEM      9217
#define MOB_VNUM_SKELETAL_WARRIO 3400
#define MOB_VNUM_QUEST           3390

/* RT ASCII conversions -- used so we can have letters in this file */
/* This is the old and limited "int" way of tracking bit values
#define A           1
#define B           2
#define C           4
#define D           8
#define E          16
#define F          32
#define G          64
#define H         128
#define I         256
#define J         512
#define K        1024
#define L        2048
#define M        4096
#define N        8192
#define O       16384
#define P       32768
#define Q       65536
#define R      131072
#define S      262144
#define T      524288
#define U     1048576
#define V     2097152
#define W     4194304
#define X     8388608
#define Y    16777216
#define Z    33554432
#define aa   67108864
#define bb  134217728
#define cc  268435456
#define dd  536870912
#define ee 1073741824
#define ff 0x80000000
*/
/* This is the new and slightly less limited "unsigned long long int" (64-bit) way of tracking bit values */
#define A   0x0000000000000001ULL /* "ULL" forces "unsigned long long int" */
#define B   0x0000000000000002ULL /*       to prevent compiler warnings    */
#define C   0x0000000000000004ULL
#define D   0x0000000000000008ULL
#define E   0x0000000000000010ULL
#define F   0x0000000000000020ULL
#define G   0x0000000000000040ULL
#define H   0x0000000000000080ULL
#define I   0x0000000000000100ULL
#define J   0x0000000000000200ULL
#define K   0x0000000000000400ULL
#define L   0x0000000000000800ULL
#define M   0x0000000000001000ULL
#define N   0x0000000000002000ULL
#define O   0x0000000000004000ULL
#define P   0x0000000000008000ULL
#define Q   0x0000000000010000ULL
#define R   0x0000000000020000ULL
#define S   0x0000000000040000ULL
#define T   0x0000000000080000ULL
#define U   0x0000000000100000ULL
#define V   0x0000000000200000ULL
#define W   0x0000000000400000ULL
#define X   0x0000000000800000ULL
#define Y   0x0000000001000000ULL
#define Z   0x0000000002000000ULL
#define aa  0x0000000004000000ULL
#define bb  0x0000000008000000ULL
#define cc  0x0000000010000000ULL
#define dd  0x0000000020000000ULL
#define ee  0x0000000040000000ULL
#define ff  0x0000000080000000ULL /* end of 32-bit values */
#define gg  0x0000000100000000ULL
#define hh  0x0000000200000000ULL
#define ii  0x0000000400000000ULL
#define jj  0x0000000800000000ULL
#define kk  0x0000001000000000ULL
#define ll  0x0000002000000000ULL
#define mm  0x0000004000000000ULL
#define nn  0x0000008000000000ULL
#define oo  0x0000010000000000ULL
#define pp  0x0000020000000000ULL
#define qq  0x0000040000000000ULL
#define rr  0x0000080000000000ULL
#define ss  0x0000100000000000ULL
#define tt  0x0000200000000000ULL
#define uu  0x0000400000000000ULL
#define vv  0x0000800000000000ULL
#define ww  0x0001000000000000ULL
#define xx  0x0002000000000000ULL
#define yy  0x0004000000000000ULL
#define zz  0x0008000000000000ULL // print_flags() limit is 52 because it saves flags as [A-Z]+[a-z]

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC         (A)        /* Auto set for mobs    */
#define ACT_SENTINEL       (B)        /* Stays in one room    */
#define ACT_SCAVENGER      (C)        /* Picks up objects    */
#define ACT_BANKER         (D)        /* Banker */
#define ACT_NOGHOST        (E)        /* Mobs don't spawn undead spirits. Sartan--12/18/00 */
#define ACT_AGGRESSIVE     (F)            /* Attacks PC's        */
#define ACT_STAY_AREA      (G)        /* Won't leave area    */
#define ACT_WIMPY          (H)
#define ACT_PET            (I)        /* Auto set for pets    */
#define ACT_TRAIN          (J)        /* Can train PC's    */
#define ACT_PRACTICE       (K)        /* Can practice PC's    */
#define ACT_DEALER         (L)
#define ACT_LOCKER         (M)
#define ACT_IS_BAILER      (N)
#define ACT_UNDEAD         (O)
#define ACT_NOQUEST        (P)        /*No quests to mobs with this flag*/
#define ACT_CLERIC         (Q)
#define ACT_MAGE           (R)
#define ACT_THIEF          (S)
#define ACT_WARRIOR        (T)
#define ACT_NOALIGN        (U)
#define ACT_NOPURGE        (V)
#define ACT_OUTDOORS       (W)
//#define ACT_UNUSED         (X)
#define ACT_INDOORS        (Y)
#define ACT_IS_HEALER     (aa)
#define ACT_GAIN          (bb)
#define ACT_UPDATE_ALWAYS (cc)
#define ACT_IS_CHANGER    (dd)
#define ACT_FORGER        (ee)  /* RH 2/1/98 forger */
#define ACT_RECALLS       (ff)  /* doesn't like being moved */

/* ACT2 FLAGS -- Added Robert Leonard 10/20/03 */
#define ACT2_RFRAG       (A)
#define ACT2_SFRAG       (B)
#define ACT2_EFRAG       (C)
#define ACT2_DFRAG       (D)
#define ACT2_AFRAG       (E)
#define ACT2_FRAG_SELL   (F)
#define ACT2_IDENTIFIER  (G)
#define ACT2_SWITCHER    (H)
#define ACT2_NOHAGGLE    (I)
#define ACT2_MOUNTABLE   (J)
#define ACT2_NOWANDEROFF (K)
#define ACT2_BLOODLESS   (L)

/* damage classes */
#define DAM_NONE      0
#define DAM_BASH      1
#define DAM_PIERCE    2
#define DAM_SLASH     3
#define DAM_FIRE      4
#define DAM_COLD      5
#define DAM_LIGHTNING 6
#define DAM_ACID      7
#define DAM_POISON    8
#define DAM_NEGATIVE  9
#define DAM_HOLY     10
#define DAM_ENERGY   11
#define DAM_MENTAL   12
#define DAM_DISEASE  13
#define DAM_DROWNING 14
#define DAM_LIGHT    15
#define DAM_OTHER    16
#define DAM_HARM     17
#define DAM_CHARM    18
#define DAM_SOUND    19
#define DAM_WIND     20
#define DAM_SHOCK    21

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK (A)
#define OFF_BACKSTAB    (B)
#define OFF_BASH        (C)
#define OFF_BERSERK     (D)
#define OFF_DISARM      (E)
#define OFF_DODGE       (F)
#define OFF_FADE        (G)
#define OFF_FAST        (H)
#define OFF_KICK        (I)
#define OFF_KICK_DIRT   (J)
#define OFF_PARRY       (K)
#define OFF_RESCUE      (L)
#define OFF_TAIL        (M)
#define OFF_TRIP        (N)
#define OFF_CRUSH       (O)
#define OFF_FLYING      (Y)
#define OFF_BITE        (X)
#define ASSIST_ALL      (P)
#define ASSIST_ALIGN    (Q)
#define ASSIST_RACE     (R)
#define ASSIST_PLAYERS  (S)
#define ASSIST_GUARD    (T)
#define ASSIST_VNUM     (U)
#define ASSIST_MOBILE   (V)
#define ASSIST_AREA     (W)

/* return values for check_imm */
#define IS_NORMAL     0
#define IS_IMMUNE     1
#define IS_RESISTANT  2
#define IS_VULNERABLE 3

/* IMM bits for mobs */
#define IMM_SUMMON    (A)
#define IMM_CHARM     (B)
#define IMM_MAGIC     (C)
#define IMM_WEAPON    (D)
#define IMM_BASH      (E)
#define IMM_PIERCE    (F)
#define IMM_SLASH     (G)
#define IMM_FIRE      (H)
#define IMM_COLD      (I)
#define IMM_LIGHTNING (J)
#define IMM_ACID      (K)
#define IMM_POISON    (L)
#define IMM_NEGATIVE  (M)
#define IMM_HOLY      (N)
#define IMM_ENERGY    (O)
#define IMM_MENTAL    (P)
#define IMM_DISEASE   (Q)
#define IMM_DROWNING  (R)
#define IMM_LIGHT     (S)
#define IMM_SOUND     (T)
#define IMM_WOOD      (X)
#define IMM_SILVER    (Y)
#define IMM_IRON      (Z)
#define IMM_WIND     (aa)
#define IMM_SHOCK    (bb)

/* RES bits for mobs */
#define RES_SUMMON    (A)
#define RES_CHARM     (B)
#define RES_MAGIC     (C)
#define RES_WEAPON    (D)
#define RES_BASH      (E)
#define RES_PIERCE    (F)
#define RES_SLASH     (G)
#define RES_FIRE      (H)
#define RES_COLD      (I)
#define RES_LIGHTNING (J)
#define RES_ACID      (K)
#define RES_POISON    (L)
#define RES_NEGATIVE  (M)
#define RES_HOLY      (N)
#define RES_ENERGY    (O)
#define RES_MENTAL    (P)
#define RES_DISEASE   (Q)
#define RES_DROWNING  (R)
#define RES_LIGHT     (S)
#define RES_SOUND     (T)
#define RES_WOOD      (X)
#define RES_SILVER    (Y)
#define RES_IRON      (Z)
#define RES_WIND     (aa)
#define RES_SHOCK    (bb)

/* VULN bits for mobs */
#define VULN_SUMMON    (A)
#define VULN_CHARM     (B)
#define VULN_MAGIC     (C)
#define VULN_WEAPON    (D)
#define VULN_BASH      (E)
#define VULN_PIERCE    (F)
#define VULN_SLASH     (G)
#define VULN_FIRE      (H)
#define VULN_COLD      (I)
#define VULN_LIGHTNING (J)
#define VULN_ACID      (K)
#define VULN_POISON    (L)
#define VULN_NEGATIVE  (M)
#define VULN_HOLY      (N)
#define VULN_ENERGY    (O)
#define VULN_MENTAL    (P)
#define VULN_DISEASE   (Q)
#define VULN_DROWNING  (R)
#define VULN_LIGHT     (S)
#define VULN_SOUND     (T)
#define VULN_WOOD      (X)
#define VULN_SILVER    (Y)
#define VULN_IRON      (Z)
#define VULN_WIND     (aa)
#define VULN_SHOCK    (bb)

/* body form */
#define FORM_EDIBLE        (A)
#define FORM_POISON        (B)
#define FORM_MAGICAL       (C)
#define FORM_INSTANT_DECAY (D)
#define FORM_OTHER         (E)  /* defined by material bit */
/* actual form */
#define FORM_ANIMAL        (G)
#define FORM_SENTIENT      (H)
#define FORM_UNDEAD        (I)
#define FORM_CONSTRUCT     (J)
#define FORM_MIST          (K)
#define FORM_INTANGIBLE    (L)
#define FORM_BIPED         (M)
#define FORM_CENTAUR       (N)
#define FORM_INSECT        (O)
#define FORM_SPIDER        (P)
#define FORM_CRUSTACEAN    (Q)
#define FORM_WORM          (R)
#define FORM_BLOB          (S)
/* animal form */
#define FORM_MAMMAL        (V)
#define FORM_BIRD          (W)
#define FORM_REPTILE       (X)
#define FORM_SNAKE         (Y)
#define FORM_DRAGON        (Z)
#define FORM_AMPHIBIAN    (aa)
#define FORM_FISH         (bb)
#define FORM_COLD_BLOOD   (cc)

/* body parts */
#define PART_HEAD        (A)
#define PART_ARMS        (B)
#define PART_LEGS        (C)
#define PART_HEART       (D)
#define PART_BRAINS      (E)
#define PART_GUTS        (F)
#define PART_HANDS       (G)
#define PART_FEET        (H)
#define PART_FINGERS     (I)
#define PART_EAR         (J)
#define PART_EYE         (K)
#define PART_LONG_TONGUE (L)
#define PART_EYESTALKS   (M)
#define PART_TENTACLES   (N)
#define PART_FINS        (O)
#define PART_WINGS       (P)
#define PART_TAIL        (Q)
/* for combat */
#define PART_CLAWS       (U)
#define PART_FANGS       (V)
#define PART_HORNS       (W)
#define PART_SCALES      (X)
#define PART_TUSKS       (Y)
#define PART_HOOF        (Z)
#define PART_TALONS     (aa)
#define PART_BUD        (bb)
#define PART_PETAL      (cc)
#define PART_LEAF       (dd)
#define PART_FLOWER     (ee)
#define PART_ROOT       (ff)

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */

#define AFF_BLIND         (A)
#define AFF_INVISIBLE     (B)
#define AFF_DETECT_EVIL   (C)
#define AFF_DETECT_INVIS  (D)
#define AFF_DETECT_MAGIC  (E)
#define AFF_DETECT_HIDDEN (F)
#define AFF_DETECT_GOOD   (G)
#define AFF_SANCTUARY     (H)
#define AFF_FAERIE_FIRE   (I)
#define AFF_INFRARED      (J)
#define AFF_CURSE         (K)
#define AFF_UNUSED_FLAG   (L)    /* unused */
#define AFF_POISON        (M)
#define AFF_PROTECT_EVIL  (N)
#define AFF_PROTECT_GOOD  (O)
#define AFF_SNEAK         (P)
#define AFF_HIDE          (Q)
#define AFF_SLEEP         (R)
#define AFF_CHARM         (S)
#define AFF_FLYING        (T)
#define AFF_PASS_DOOR     (U)
#define AFF_HASTE         (V)
#define AFF_CALM          (W)
#define AFF_PLAGUE        (X)
#define AFF_WEAKEN        (Y)
#define AFF_DARK_VISION   (Z)
#define AFF_BERSERK      (aa)
#define AFF_SWIM         (bb)
#define AFF_REGENERATION (cc)
#define AFF_SLOW         (dd)
#define AFF_LINKDEATH    (ee)
#define AFF_AQUA_ALBEDO  (ff)
#define AFF_AQUA_REGIA   (gg)
#define AFF_ALACRITY     (hh)

/*
 *   Bits for 'affected2_by'.
 *   Used in #MOBILES.
 */
#define AFF2_NIRVANA          (A)
#define AFF2_FADE_OUT         (B)
#define AFF2_RADIANT          (C)
#define AFF2_SHROUD           (D)
#define AFF2_PROTECT_NEUTRAL  (E)
#define AFF2_INVUN            (F)
#define AFF2_WARCRY_HARDENING (G)
#define AFF2_WARCRY_RAGE      (H)
#define AFF2_WARCRY_VIGOR     (I)
#define AFF2_BINDED           (J)

/* Affects for certain spells */

#define SAFF_MARTYR           (A)
#define SAFF_WALK_ON_WATER    (B)
#define SAFF_DETER            (C)
#define SAFF_INVUN            (D)
#define SAFF_FARSIGHT         (E)
#define SAFF_MAGESHIELD       (F)
#define SAFF_REPLENISH        (G)
#define SAFF_WARRIORSHIELD    (H)
#define SAFF_YAWN             (I)
#define SAFF_HICCUP           (J)
#define SAFF_GHOST            (K)
#define SAFF_IRONWILL         (L)
#define SAFF_NIRVANA          (M)
#define SAFF_PROTECT_NEUTRAL  (N)
#define SAFF_ADRENALIZE       (O)
#define SAFF_PROTECT_HOLY     (P)
#define SAFF_PROTECT_NEGATIVE (Q)
#define SAFF_MANA_DRAIN       (R)
#define SAFF_LIFE_DRAIN       (S)
#define SAFF_FLAME_SHROUD     (T)
#define SAFF_ICE_SHROUD       (U)
#define SAFF_ELECTRIC_SHROUD  (V)
#define SAFF_POISON_SHROUD    (W)
#define SAFF_WARCRY_GUARDING  (X)

/* 
 * Clan Flags
 */
#define CLAN_INDEPENDENT (A)
#define CLAN_PEACEFUL    (B)
#define CLAN_LAW         (C)
#define CLAN_GWYLIAD     (D)
#define CLAN_CHANGED     (E)

/*
 * Rank Flags
 */
#define RANK_IMMORTAL   (A)
#define RANK_LEADER     (B)
#define RANK_ASSISTANT  (C)
#define RANK_AMBASSADOR (D)
//Class Ranks
#define RANK_CONJURER   (E)
#define RANK_PRIEST     (F)
#define RANK_HIGHWAYMAN (G)
#define RANK_KNIGHT     (H)
#define RANK_WARLOCK    (I)
#define RANK_BARBARIAN  (J)
#define RANK_MYSTIC     (K)
#define RANK_DRUID      (L)
#define RANK_INQUISITOR (M)
#define RANK_OCCULTIST  (N)
#define RANK_ALCHEMIST  (O)
#define RANK_WOODSMAN   (P)

/*
 * Nospam Flags
 */
#define NOSPAM_SMISS    (A)
#define NOSPAM_OMISS    (B)
#define NOSPAM_SHIT     (C)
#define NOSPAM_OHIT     (D)
#define NOSPAM_SEFFECTS (E)
#define NOSPAM_OEFFECTS (F)
#define NOSPAM_SPARRY   (I)
#define NOSPAM_OPARRY   (J)
#define NOSPAM_SDODGE   (K)
#define NOSPAM_ODODGE   (L)
#define NOSPAM_MSPLIT   (M)

/* Mystic battle styles */
#define STYLE_NONE     0
#define STYLE_DRAGON   1
#define STYLE_DRUNK    2
#define STYLE_TIGER    3
#define STYLE_SNAKE    4
#define STYLE_CRANE    5
#define STYLE_IRONFIST 6
#define STYLE_BASIC    7
#define STYLE_JUDO     8
#define STYLE_KARATE   9

/* local affects during online.  active_flags */
#define ACTIVE_HAS_ATTACKED      (A)
#define ACTIVE_CHAR_IN_ROOM      (B)
#define ACTIVE_HAS_SWITCHED      (C)
#define ACTIVE_PLOAD             (D)
#define ACTIVE_OBJ_COMBINED_SAVE (E)
#define ACTIVE_THREAD            (F)
#define ACTIVE_REPLY             (J)

/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL 0
#define SEX_MALE    1
#define SEX_FEMALE  2

/* AC types */
#define AC_PIERCE   0
#define AC_BASH     1
#define AC_SLASH    2
#define AC_EXOTIC   3

/* dice */
#define DICE_NUMBER 0
#define DICE_TYPE   1
#define DICE_BONUS  2

/* size */
#define SIZE_TINY   0
#define SIZE_SMALL  1
#define SIZE_MEDIUM 2
#define SIZE_LARGE  3
#define SIZE_HUGE   4
#define SIZE_GIANT  5

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define MAX_REVERED 100
extern int revered_vnums[MAX_REVERED];

extern sh_int countCur;
extern sh_int countMax;
extern sh_int countMaxDay;
extern sh_int countHour;
extern sh_int countArr      [24];
extern sh_int countMaxDoW    [7];
extern int    port, control;
extern int    pObjNum;

#define OBJ_VNUM_SILVER_ONE     1
#define OBJ_VNUM_GOLD_ONE       2
#define OBJ_VNUM_GOLD_SOME      3
#define OBJ_VNUM_SILVER_SOME    4
#define OBJ_VNUM_COINS          5
#define OBJ_VNUM_DIAMOND     3377
#define OBJ_VNUM_CORPSE_NPC    10
#define OBJ_VNUM_CORPSE_PC     11
#define OBJ_VNUM_SEVERED_HEAD  12
#define OBJ_VNUM_TORN_HEART    13
#define OBJ_VNUM_SLICED_ARM    14
#define OBJ_VNUM_SLICED_LEG    15
#define OBJ_VNUM_GUTS          16
#define OBJ_VNUM_BRAINS        17
#define OBJ_VNUM_HANDS         31
#define OBJ_VNUM_FEET          32
#define OBJ_VNUM_FINGERS       33
#define OBJ_VNUM_EAR           34
#define OBJ_VNUM_EYE           35
#define OBJ_VNUM_LONG_TONGUE   36
#define OBJ_VNUM_EYESTALKS     37
#define OBJ_VNUM_TENTACLES     38
#define OBJ_VNUM_FINS          39
#define OBJ_VNUM_WINGS         50
#define OBJ_VNUM_TAIL          51
#define OBJ_VNUM_CLAWS         52
#define OBJ_VNUM_FANGS         53
#define OBJ_VNUM_HORNS         54
#define OBJ_VNUM_SCALES        55
#define OBJ_VNUM_TUSKS         56
#define OBJ_VNUM_HOOF          57

#define OBJ_VNUM_MUSHROOM      20
#define OBJ_VNUM_STEAK       6644
#define OBJ_VNUM_LIGHT_BALL    21
#define OBJ_VNUM_SPRING        22
#define OBJ_VNUM_DISC          23
#define OBJ_VNUM_PORTAL        25

#define OBJ_VNUM_ROSE        1001

#define OBJ_VNUM_PIT         3010

#define OBJ_VNUM_SCHOOL_MACE    3700
#define OBJ_VNUM_SCHOOL_DAGGER  3701
#define OBJ_VNUM_SCHOOL_SWORD   3702
#define OBJ_VNUM_SCHOOL_SPEAR   3717
#define OBJ_VNUM_SCHOOL_STAFF   3718
#define OBJ_VNUM_STAFF          1320
#define OBJ_VNUM_SHELTER        3367
#define OBJ_VNUM_SCHOOL_AXE     3719
#define OBJ_VNUM_SCHOOL_FLAIL   3720
#define OBJ_VNUM_SCHOOL_WHIP    3721
#define OBJ_VNUM_SCHOOL_POLEARM 3722
// #define OBJ_VNUM_NULL          NULL
#define OBJ_VNUM_NULL              0

#define OBJ_VNUM_SCHOOL_VEST   3703
#define OBJ_VNUM_SCHOOL_SHIELD 3704
#define OBJ_VNUM_SCHOOL_BANNER 3716
#define OBJ_VNUM_SCHOOL_CROSSBOW 3729
#define OBJ_VNUM_MAP           3162
#define OBJ_VNUM_CAL_MAP      24474
#define OBJ_VNUM_TGUIDE1       3158

#define OBJ_VNUM_DREKABUS_MAP   5600
#define OBJ_VNUM_ALINDRAK_MAP   8402
#define OBJ_VNUM_PALSARRIEN_MAP 4000

#define OBJ_VNUM_WHISTLE       2116
#define OBJ_VNUM_CLEAN_SCROLL  31610 // vnum init area
#define OBJ_VNUM_CLEAN_VIAL    31611 // vnum init area
#define OBJ_VNUM_QUEST         3398
#define OBJ_VNUM_RESTRING      3382
#define OBJ_VNUM_BAG           1111
#define OBJ_VNUM_TALE          3015
#define OBJ_VNUM_FULL_REGEN    3078
#define OBJ_VNUM_DRUID_STAFF   1320

#define OBJ_VNUM_BEDROLL_HP    1255
#define OBJ_VNUM_BEDROLL_MANA  1256
#define OBJ_VNUM_BEDROLL_BOTH  1258

#define OBJ_VNUM_DEF_FISH1    31670 // fishing code, default fish
#define OBJ_VNUM_DEF_FISH2    31671
#define OBJ_VNUM_DEF_FISH3    31672
#define OBJ_VNUM_DEF_FISH4    31673
#define OBJ_VNUM_DEF_FISH5    31674
#define OBJ_VNUM_DEF_FISH6    31675
#define OBJ_VNUM_DEF_FISH7    31676
#define OBJ_VNUM_DEF_FISH8    31677

// Fishing: % chance of getting each fish type
#define FC1  24
#define FC2  21
#define FC3  17
#define FC4  14
#define FC5  11
#define FC6  7
#define FC7  4
#define FC8  100-(FC1+FC2+FC3+FC4+FC5+FC6+FC7)

/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT         1
#define ITEM_SCROLL        2
#define ITEM_WAND          3
#define ITEM_STAFF         4
#define ITEM_WEAPON        5
#define ITEM_TREASURE      8
#define ITEM_ARMOR         9
#define ITEM_POTION       10
#define ITEM_CLOTHING     11
#define ITEM_FURNITURE    12
#define ITEM_TRASH        13
#define ITEM_CONTAINER    15
#define ITEM_DRINK_CON    17
#define ITEM_KEY          18
#define ITEM_FOOD         19
#define ITEM_MONEY        20
#define ITEM_BOAT         22
#define ITEM_CORPSE_NPC   23
#define ITEM_CORPSE_PC    24
#define ITEM_FOUNTAIN     25
#define ITEM_PILL         26
#define ITEM_PROTECT      27
#define ITEM_MAP          28
#define ITEM_PORTAL       29
#define ITEM_WARP_STONE   30
#define ITEM_ROOM_KEY     31
#define ITEM_GEM          32
#define ITEM_JEWELRY      33
#define ITEM_JUKEBOX      34
#define ITEM_MONEY_POUCH  35
#define ITEM_SLOT_MACHINE 36
#define ITEM_CHECKERS     37
#define ITEM_LOCKER       38
#define ITEM_WHETSTONE    39
#define ITEM_SPELLBOOK    40
#define ITEM_SCRY_MIRROR  41
#define ITEM_BOOK         42
#define ITEM_FISHING_ROD  43
#define ITEM_PROJECTILE   44
#define ITEM_QUIVER       45
#define ITEM_KEYRING      46
#define ITEM_BANDAGE      47

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW         (A)
#define ITEM_HUM          (B)
#define ITEM_DARK         (C)
#define ITEM_LOCK         (D)
#define ITEM_EVIL         (E)
#define ITEM_INVIS        (F)
#define ITEM_MAGIC        (G)
#define ITEM_NODROP       (H)
#define ITEM_BLESS        (I)
#define ITEM_ANTI_GOOD    (J)
#define ITEM_ANTI_EVIL    (K)
#define ITEM_ANTI_NEUTRAL (L)
#define ITEM_NOREMOVE     (M)
#define ITEM_INVENTORY    (N)
#define ITEM_NOPURGE      (O)
#define ITEM_ROT_DEATH    (P)
#define ITEM_VIS_DEATH    (Q)
#define ITEM_NONMETAL     (S)
#define ITEM_NOLOCATE     (T)
#define ITEM_MELT_DROP    (U)
#define ITEM_HAD_TIMER    (V)
#define ITEM_SELL_EXTRACT (W)
#define ITEM_DONATION_PIT (X)
#define ITEM_BURN_PROOF   (Y)
#define ITEM_NOUNCURSE    (Z)
#define ITEM_RESTRING     (aa)
#define ITEM_CLANEQ       (bb)
#define ITEM_PLURAL       (cc)
#define ITEM_PLAYER_HOUSE (ee)
#define ITEM_BLESSED_SHIELD (ff)

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE        (A)
#define ITEM_WEAR_FINGER (B)
#define ITEM_WEAR_NECK   (C)
#define ITEM_WEAR_BODY   (D)
#define ITEM_WEAR_HEAD   (E)
#define ITEM_WEAR_LEGS   (F)
#define ITEM_WEAR_FEET   (G)
#define ITEM_WEAR_HANDS  (H)
#define ITEM_WEAR_ARMS   (I)
#define ITEM_WEAR_SHIELD (J)
#define ITEM_WEAR_ABOUT  (K)
#define ITEM_WEAR_WAIST  (L)
#define ITEM_WEAR_WRIST  (M)
#define ITEM_WIELD       (N)
#define ITEM_HOLD        (O)
#define ITEM_NO_SAC      (P)
#define ITEM_WEAR_FLOAT  (Q)
#define ITEM_WEAR_BAG    (R)
#define ITEM_WEAR_BACK   (S)
#define ITEM_WEAR_LAPEL  (T)
#define ITEM_WEAR_EAR    (U)
#define ITEM_WEAR_EYE    (V)
#define ITEM_WEAR_RFOOT  (W)
#define ITEM_WEAR_LFOOT  (X)
#define ITEM_WEAR_CREST (aa)

/* projectile flags */
#define ITEM_PROJECTILE_MAGICBOMB (A) // for Alchemist thrown projectiles
#define ITEM_PROJECTILE_THROW     (B) // NOT for Alchemist spells
#define ITEM_PROJECTILE_BOMB      (C) // was an idea for highwaymen originally
#define ITEM_PROJECTILE_SLING     (D) // small objects, like rocks
#define ITEM_PROJECTILE_BOW       (E) // long arrows for a full/mid bow
#define ITEM_PROJECTILE_CROSSBOW  (F) // short arrows (bolts) for a crossbow/hand bow
#define ITEM_PROJECTILE_ATLATL    (G) // a small spear chucking stick

/* weapon class */
#define WEAPON_EXOTIC  0
#define WEAPON_SWORD   1
#define WEAPON_DAGGER  2
#define WEAPON_SPEAR   3
#define WEAPON_MACE    4
#define WEAPON_AXE     5
#define WEAPON_FLAIL   6
#define WEAPON_WHIP    7
#define WEAPON_POLEARM 8
#define WEAPON_CROSSBOW 9
#define WEAPON_NULL    10

/* weapon types */
#define WEAPON_FLAMING    (A)
#define WEAPON_FROST      (B)
#define WEAPON_VAMPIRIC   (C)
#define WEAPON_SHARP      (D)
#define WEAPON_VORPAL     (E)
#define WEAPON_TWO_HANDS  (F)
#define WEAPON_SHOCKING   (G)
#define WEAPON_POISON     (H)
#define WEAPON_CLAN       (I)
#define WEAPON_MANA_DRAIN (J)
#define WEAPON_HOLY       (K)
#define WEAPON_UNHOLY     (L)

/* gate flags */
#define GATE_NORMAL_EXIT  (A)
#define GATE_NOCURSE      (B)
#define GATE_GOWITH       (C)
#define GATE_BUGGY        (D)
#define GATE_RANDOM       (E)

/* furniture flags */
#define STAND_AT    (A)
#define STAND_ON    (B)
#define STAND_IN    (C)
#define STAND_UNDER (S)
#define SIT_AT      (D)
#define SIT_ON      (E)
#define SIT_IN      (F)
#define SIT_UNDER   (Q)
#define REST_AT     (G)
#define REST_ON     (H)
#define REST_IN     (I)
#define REST_UNDER  (T)
#define SLEEP_AT    (J)
#define SLEEP_ON    (K)
#define SLEEP_IN    (L)
#define SLEEP_UNDER (R)
#define PUT_AT      (M)
#define PUT_ON      (N)
#define PUT_IN      (O)
#define PUT_INSIDE  (P)
#define PUT_UNDER   (U) // U is last

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE            0
#define APPLY_STR             1
#define APPLY_DEX             2
#define APPLY_INT             3
#define APPLY_WIS             4
#define APPLY_CON             5
#define APPLY_SEX             6
#define APPLY_CLASS           7
#define APPLY_LEVEL           8
#define APPLY_AGE             9
#define APPLY_HEIGHT         10
#define APPLY_WEIGHT         11
#define APPLY_MANA           12
#define APPLY_HIT            13
#define APPLY_MOVE           14
#define APPLY_GOLD           15
#define APPLY_EXP            16
#define APPLY_AC             17
#define APPLY_HITROLL        18
#define APPLY_DAMROLL        19
#define APPLY_SAVES          20
#define APPLY_SAVING_PARA    20
#define APPLY_SAVING_ROD     21
#define APPLY_SAVING_PETRI   22
#define APPLY_SAVING_BREATH  23
#define APPLY_SAVING_SPELL   24
//#define APPLY_SAVING_CAST    20
//#define APPLY_SAVING_STAFF   21
//#define APPLY_SAVING_OTHER   22
//#define APPLY_SAVING_BREATH  23
//#define APPLY_SAVING_POTION  24
#define APPLY_SPELL_AFFECT   25
//#define APPLY_SAVING_ALL     26

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE        1
#define CONT_PICKPROOF        2
#define CONT_CLOSED           4
#define CONT_LOCKED           8
#define CONT_PUT_ON          16
#define CONT_FENCED          32


/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO            2
#define ROOM_VNUM_CHAT          1200
// Good rooms
#define ROOM_VNUM_TEMPLE        8401
#define ROOM_VNUM_ALTAR         8402
#define OBJ_VNUM_GOOD_DONATE    8400
#define ROOM_VNUM_SCHOOL       10000
#define ROOM_VNUM_GOOD_START   10026
#define ROOM_VNUM_MORGUE        8453
// Neutral rooms
#define OBJ_VNUM_NEUT_DONATE    3901
#define ROOM_VNUM_NEUT_MORGUE   3922
#define ROOM_VNUM_NEUT_START    2600
#define ROOM_VNUM_NEUT_SCHOOL   2600
#define ROOM_VNUM_NEUT_RECALL   3900
#define ROOM_VNUM_NEUT_ALTAR    3900
#define ROOM_VNUM_NEUT_HEALER   3901
// Evil rooms
#define OBJ_VNUM_EVIL_DONATE    5700
#define ROOM_VNUM_EVIL_MORGUE   5736
#define ROOM_VNUM_EVIL_START    3762
#define ROOM_VNUM_EVIL_SCHOOL   3700
#define ROOM_VNUM_EVIL_RECALL   5601
#define ROOM_VNUM_EVIL_ALTAR    5600
#define ROOM_VNUM_EVIL_HEALER   5600
// Calipsu'Tai rooms
#define ROOM_VNUM_CORUS_RECALL 11142
#define ROOM_VNUM_CORUS_DEATH  11142
#define ROOM_VNUM_CORUS_MORGUE 11143

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK         (A)
#define ROOM_NO_MOB       (C)
#define ROOM_INDOORS      (D)
#define ROOM_SHIP         (E)
#define ROOM_FISHING      (F)
#define ROOM_PRIVATE      (J)
#define ROOM_SAFE         (K)
#define ROOM_SOLITARY     (L)
#define ROOM_PET_SHOP     (M)
#define ROOM_NO_RECALL    (N)
#define ROOM_IMP_ONLY     (O)
#define ROOM_GODS_ONLY    (P)
#define ROOM_HEROES_ONLY  (Q)
#define ROOM_NEWBIES_ONLY (R)
#define ROOM_LAW          (S)
#define ROOM_NOWHERE      (T)
#define ROOM_NOMAGIC      (U)
#define ROOM_ARENA        (V)
#define ROOM_NOWEATHER    (W)
#define ROOM_UNDER_WATER  (X)
#define ROOM_NO_GHOST     (Y)
#define ROOM_NO_TELEPORT  (Z)

/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH  0
#define DIR_EAST   1
#define DIR_SOUTH  2
#define DIR_WEST   3
#define DIR_UP     4
#define DIR_DOWN   5

/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR      (A)
#define EX_CLOSED      (B)
#define EX_LOCKED      (C)
#define EX_MULTI       (D)
#define EX_PICKPROOF   (F)
#define EX_NOPASS      (G)
#define EX_EASY        (H)
#define EX_HARD        (I)
#define EX_INFURIATING (J)
#define EX_NOCLOSE     (K)
#define EX_NOLOCK      (L)
#define EX_HIDDEN      (M)
#define EX_FENCED      (N)
#define EX_NOEXIT      (O)
#define EX_SEETHROUGH  (P)
#define EX_OBSCURED    (Q)

/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE       0
#define SECT_CITY         1
#define SECT_FIELD        2
#define SECT_FOREST       3
#define SECT_HILLS        4
#define SECT_MOUNTAIN     5
#define SECT_WATER_SWIM   6
#define SECT_WATER_NOSWIM 7
#define SECT_UNUSED       8
#define SECT_AIR          9
#define SECT_DESERT      10
#define SECT_GRAVEYARD   11
#define SECT_UNDERGROUND 12
#define SECT_MAX         13

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE      -1
#define WEAR_LIGHT      0
#define WEAR_FINGER_L   1
#define WEAR_FINGER_R   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAIST     13
#define WEAR_WRIST_L   14
#define WEAR_WRIST_R   15
#define WEAR_WIELD     16
#define WEAR_HOLD      17
#define WEAR_FLOAT     18
#define WEAR_SECONDARY 19
#define WEAR_BAG       20
#define WEAR_BACK      21
#define WEAR_LAPEL     22
#define WEAR_EAR_L     23
#define WEAR_EAR_R     24
#define WEAR_LEYE      25
#define WEAR_RFOOT     26
#define WEAR_LFOOT     27
#define WEAR_CREST     28
#define MAX_WEAR       29

#define CLASS_NONE      (ee)
#define CLASS_ALL        (A)
#define CLASS_PRIEST     (B)
#define CLASS_CONJURER   (C)
#define CLASS_WARLOCK    (D)
#define CLASS_HIGHWAYMAN (E)
#define CLASS_KNIGHT     (F)
#define CLASS_DRUID      (G)
#define CLASS_BARBARIAN  (H)
#define CLASS_MYSTIC     (I)
#define CLASS_INQUISITOR (J)
#define CLASS_OCCULTIST  (K)
#define CLASS_ALCHEMIST  (L)
#define CLASS_WOODSMAN   (M)

/* The following is based on the order in class_table
   to correspond with class_lookup. */
#define  cConjurer   0
#define  cPriest     1
#define  cHighwayman 2
#define  cKnight     3
#define  cWarlock    4
#define  cBarbarian  5
#define  cMystic     6
#define  cDruid      7
#define  cInquisitor 8
#define  cOccultist  9
#define  cAlchemist  10
#define  cWoodsman   11

/* Toxicology: types of poison for envenom */
#define TOX_VENOM           0 // default: traditional poison
#define TOX_ELEMENTAL       1
#define TOX_FUNGAL          2
#define TOX_ACIDIC          3
#define TOX_NECROTIC        4
#define TOX_VIRAL           5
#define TOX_NEUROTOXIC      6
#define TOX_HALLUCINOGENIC  7

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Prog-types. Added by Merak 2006 - 10 - 01
 */
#define MOB_PROG   0
#define ROOM_PROG  1
#define OBJ_PROG   2

/*
 * Parse-flags. Added by Merak 2006 - 11 - 30
 */
#define FROM_ROOM      (A)
#define FROM_OBJ       (B) 
#define FROM_LOCKER    (C)
#define FROM_CH        (D)
#define FROM_DEFAULT   (O)
#define FROM_NOT_FOUND (P)

/*
 * Conditions.
 */
#define COND_DRUNK  0
#define COND_FULL   1
#define COND_THIRST 2
#define COND_HUNGER 3

/*
 * Positions.
 */
#define POS_DEAD     0
#define POS_MORTAL   1
#define POS_INCAP    2
#define POS_STUNNED  3
#define POS_SLEEPING 4
#define POS_RESTING  5
#define POS_SITTING  6
#define POS_FIGHTING 7
#define POS_STANDING 8


/*
 * Append Conditions. Added by Aarchane 2009-2-5
 */
#define APPEND_IGNORE 0
#define APPEND_CHAR   1
#define APPEND_AREA   2
#define APPEND_HELP   3

/*
 * ACT bits for players.
 */
#define PLR_IS_NPC     (A)        /* Don't EVER set.    */
/* RT auto flags */
#define PLR_AUTOASSIST (C)
#define PLR_AUTOEXIT   (D)
#define PLR_AUTOLOOT   (E)
#define PLR_AUTOSAC    (F)
#define PLR_AUTOGOLD   (G)
#define PLR_AUTOSPLIT  (H)
#define PLR_NOCANCEL   (I)
#define PLR_AUTODONATE (S)
/* RT personal flags */
#define PLR_NODIGMOVE  (J)
#define PLR_AUTOHUNT   (K)
#define PLR_FISHING    (L)
#define PLR_HIDEQUEST  (M)
#define PLR_HOLYLIGHT  (N)
#define PLR_MORGUE     (O)
#define PLR_CANLOOT    (P)
#define PLR_NOSUMMON   (Q)
#define PLR_NOFOLLOW   (R)
#define PLR_NOGATE     (B)
#define PLR_COLOUR     (T)
/* penalty flags */
#define PLR_PERMIT     (U)
#define PLR_BAILED     (V)
#define PLR_LOG        (W)
#define PLR_DENY       (X)
#define PLR_FREEZE     (Y)
#define PLR_THIEF      (Z)
#define PLR_KILLER    (aa)
#define PLR_LEADER    (bb)
#define PLR_QUESTING  (cc)
#define PLR_TWIT      (dd)
#define PLR_VIOLENT   (ee)
#define PLR_ASSISTANT (ff)

/* RT comm flags -- may be used on both mobs and chars */
#define CHANNEL_QUIET      (A)
#define CHANNEL_DEAF       (B)
#define CHANNEL_NOWIZ      (C)
#define CHANNEL_NOAUCTION  (D)
#define CHANNEL_NOGOSSIP   (E)
#define CHANNEL_NOQUESTION (F)
#define CHANNEL_NOMUSIC    (G)
#define CHANNEL_NOCLAN     (H)
#define CHANNEL_NOQUOTE    (I)
#define CHANNEL_SHOUTSOFF  (J)
#define CHANNEL_NOOOC      (K)
#define CHANNEL_NEWBIE     (P)
#define CHANNEL_NOGRATS    (R)
#define CHANNEL_NOINFO     (S)
#define CHANNEL_NOEMOTE    (T)
#define CHANNEL_NOSHOUT    (U)
#define CHANNEL_NOTELL     (V)
#define CHANNEL_ALL        (X)
#define CHANNEL_NOWAR     (aa)
#define CHANNEL_NOPOLITIC (bb)
#define CHANNEL_NOIRL     (cc)

/* display flags */
#define COMM_COMPACT       (L)
#define COMM_BRIEF         (M)
#define COMM_PROMPT        (N)
#define COMM_COMBINE       (O)
#define COMM_SHOW_AFFECTS  (Q)
#define COMM_REPLY_LOCK   (dd)
#define COMM_NEWSFAERIE   (cc)
#define COMM_STATS_SHOW   (ff)
#define COMM_NO_OLC_PROMPT (Y) // Taeloch -- removes auto OLC prompt for IMMs
#define COMM_AFK           (Z)
#define COMM_NO_SLEEP_TELLS (ee)

/* penalties */
#define PEN_NOCHANNELS     (W)
#define PEN_SNOOP_PROOF    (Y)
#define PEN_NOTE          (ee)

/* PLR2 Flags */
#define PLR2_STATS         (A)
/*#define PLR2_NOQUEST       (B)*/
#define PLR2_TELNET_GA     (P)
#define PLR2_NO_FRAGXP     (X)

/* WIZnet flags */
#define WIZ_ON        (A)
#define WIZ_TICKS     (B)
#define WIZ_LOGINS    (C)
#define WIZ_SITES     (D)
#define WIZ_LINKS     (E)
#define WIZ_DEATHS    (F)
#define WIZ_RESETS    (G)
#define WIZ_MOBDEATHS (H)
#define WIZ_FLAGS     (I)
#define WIZ_PENALTIES (J)
#define WIZ_SACCING   (K)
#define WIZ_LEVELS    (L)
#define WIZ_SECURE    (M)
#define WIZ_SWITCHES  (N)
#define WIZ_SNOOPS    (O)
#define WIZ_RESTORE   (P)
#define WIZ_LOAD      (Q)
#define WIZ_NEWBIE    (R)
#define WIZ_PREFIX    (S)
#define WIZ_SPAM      (T)
#define WIZ_MEMCHECK  (U)
#define WIZ_MURDER    (V)
#define WIZ_BUGS      (W)
#define WIZ_BANK      (X)
#define WIZ_CLEANSE   (Y)
#define WIZ_WORKLIST  (Z)
#define WIZ_CLANGEMS (aa)
#define WIZ_MOBPROGS (bb)
#define WIZ_OLC      (cc)

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct mob_index_data
{
  MOB_INDEX_DATA * next;
  SPEC_FUN       * spec_fun;
  SHOP_DATA      * pShop;
  MPROG_LIST     * mprogs;
  AREA_DATA      * area;
  sh_int           vnum;
  sh_int           group;
  bool             new_format;
  sh_int           count;
  int              max_count;
  sh_int           killed;
  char           * player_name;
  char           * short_descr;
  char           * long_descr;
  char           * long2_descr;
  char           * description;
  char           * path;
  int              max_riders;
  int64            act;
  int64            act2;
  int64            affected_by;
  int64            affected2_by;
  sh_int           alignment;
  sh_int           level;
  sh_int           hitroll;
  int              frag_number;
  int              hit[3];  // was sh_int.  Changed to int to allow for larger values
  int              mana[3]; // was sh_int.  Changed to int to allow for larger values
  sh_int           damage[3];
  sh_int           ac[4];
  sh_int           dam_type;
  int64            off_flags;
  int64            imm_flags;
  int64            res_flags;
  int64            vuln_flags;
  sh_int           start_pos;
  sh_int           default_pos;
  sh_int           sex;
  sh_int           race;
  long             wealth;
  int64            form;
  int64            parts;
  sh_int           size;
  char           * material;
  long             mprog_flags;
};

/* memory settings */
#define MEM_CUSTOMER A
#define MEM_SELLER   B
#define MEM_HOSTILE  C
#define MEM_AFRAID   D

/* memory for mobs */
struct mem_data
{
  MEM_DATA * next;
  bool       valid;
  int        id;
  int        reaction;
  time_t     when;
};

/*
 * One character (PC or NPC).
 */
struct char_data
{
  CHAR_DATA       * next;
  CHAR_DATA       * next_player;
  CHAR_DATA       * next_in_room;
  CHAR_DATA       * master;
  CHAR_DATA       * leader;
  CHAR_DATA       * fighting;
  CHAR_DATA       * hunting;
  CHAR_DATA       * reply;
  CHAR_DATA       * pet;
  HATE_DATA       * hate;
  CHAR_DATA       * plevel;
  CHAR_DATA       * mprog_target;
  MEM_DATA        * memory;
  SPEC_FUN        * spec_fun;
  MOB_INDEX_DATA  * pIndexData;
  DESCRIPTOR_DATA * desc;
  AFFECT_DATA     * affected;
  NOTE_DATA       * pnote;
  OBJ_DATA        * carrying;
  OBJ_DATA        * on;
  ROOM_INDEX_DATA * in_room;
  ROOM_INDEX_DATA * old_room;
  ROOM_INDEX_DATA * was_in_room;
  ROOM_INDEX_DATA * reset_room; // Only valid for mobs -- Merak
  AREA_DATA       * zone;
  PC_DATA         * pcdata;
  QUEST_DATA      * questdata;
  GEN_DATA        * gen_data;
  TIME_INFO_DATA    birthday;
  BUFFER          * fixed_buffer;
  char            * clan_name;
  bool              valid;
  char            * name;
  long              id;
  sh_int            version;
  char            * short_descr;
  char            * long_descr;
  char            * description;
  char            * path;       // Only valid for mobs.... I think -- Merak
  int               max_riders; // mountable is an ACT2 flag
  char            * path_ptr;
  char            * prompt;
  char            * prefix;
  sh_int            group;
  CLAN_DATA       * clan;
  sh_int            sex;
  sh_int            gameclass;
  enum sex_dir_enum sex_dir;
  sh_int            race;
  sh_int            level;
  sh_int            trust;
  int               played;
  int               lines;  /* for the pager */
  time_t            logon;
  int               last_level;
  int               bailed;
  int               num_bailed;
  sh_int            timer;
  sh_int            wait;
  sh_int            daze;
  int               hit;      // was sh_int.  Changed to int to allow for larger values
  int               max_hit;  // was sh_int.  Changed to int to allow for larger values
  int               mana;     // was sh_int.  Changed to int to allow for larger values
  int               max_mana; // was sh_int.  Changed to int to allow for larger values
  int               move;     // was sh_int.  Changed to int to allow for larger values
  int               max_move; // was sh_int.  Changed to int to allow for larger values
  long              gold;
  long              silver;
  int64             exp;
  int64             act;
  int64             act2;
  int64             plr2;
  int64             nospam;
  long              toggle;
  int64             comm_flags;
  int64             chan_flags;
  int64             pen_flags;
  int64             wiznet;
  int64             imm_flags;
  int64             res_flags;
  int64             vuln_flags;
  sh_int            invis_level;
  sh_int            incog_level;
  int64             affected_by;
  int64             affected2_by;
  int64             spell_aff;
  long              active_flags;
  sh_int            position;
  sh_int            practice;
  sh_int            train;
  sh_int            carry_weight;
  sh_int            carry_number;
  sh_int            saving_throw;
  sh_int            alignment;
  sh_int            hitroll;
  int               frag_number;
  sh_int            damroll;
  sh_int            armor[4];
  sh_int            wimpy;
  sh_int            perm_stat[MAX_STATS];
  sh_int            mod_stat[MAX_STATS];
  int64             form;
  int64             parts;
  sh_int            size;
  char            * material;
  int64             off_flags;
  sh_int            damage[3];
  sh_int            dam_type;
  sh_int            start_pos;
  sh_int            default_pos;
  sh_int            mprog_delay;
  sh_int            tells;
  bool              bs_flag;
  char            * tracking;
  int               vflag_timer;
  int               martial_style;
  bool              newbie;
  int               idle_snapshot;
  int               idle_time;
  long              group_num;
  long              group_fight;
  int               ruby_fragment;
  int               ruby_counter;
  int               sapphire_fragment;
  int               sapphire_counter;
  int               emerald_fragment;
  int               emerald_counter;
  int               diamond_fragment;
  int               diamond_counter;
  sh_int            first_frag_level;  // To keep track of fragment purchases - 5 allowed
  sh_int            second_frag_level; // 5 allowed
  sh_int            third_frag_level;  // 3 allowed
  sh_int            fourth_frag_level; // 2 allowed
  sh_int            fifth_frag_level;  // 1 allowed
  CHAN_HIST         chan_hist[MAX_CHANNEL][MAX_HIST];
  CMD_HIST          cmd_hist[MAX_COMMAND_HISTORY];
  CHAR_DATA       * mount;      // set to mob the player is riding (or NULL)
#if MEMDEBUG
  char            * memdebug_name;
  char            * memdebug_prompt;
#endif
};

#define MAX_FORGET 5
struct hate_data
{
  char      * name;
  CHAR_DATA * who;
};

struct quest_data
{
  QUEST_DATA * next;
  bool         valid;
  int          curr_points;
  int          accum_points;
  CHAR_DATA  * questgiver;
  int          points;
  int          nextquest;
  int          countdown;
  int          time_allowed;
  int          obj_vnum;
  int          mob_vnum;
  int          quest_number;
  int          room_vnum;
  int          streak;
  int          current_streak;
  int          best_streak;
  int          quit_num;
  int          comp_num;
  int          attempt_num;
  int          glory;
  int          failed;
  BUFFER     * log;
};

/*
 * Data which only PC's have.
 */
struct    pc_data
{
  PC_DATA   * next;
  BUFFER    * buffer;
  bool        valid;
  char      * pwd;
  char      * bamfin;
  char      * bamfout;
  char      * afk_title;
  char      * title;
  char      * permtitle;
  char      * host;
  long        laston;
  time_t      last_note;
  time_t      last_idea;
  time_t      last_penalty;
  time_t      last_news;
  time_t      last_changes;
  time_t      last_rules;
  time_t      last_rpnote;
  sh_int      perm_hit;
  sh_int      perm_mana;
  sh_int      perm_move;
  sh_int      true_sex;
  int         last_level;
  sh_int      condition         [4];
  sh_int      learned           [MAX_SKILL];
  sh_int      maintained        [MAX_MAINTAINED];
  sh_int      maintained_other  [MAX_MAINTAINED];
  sh_int      maintained_weapon [MAX_MAINTAINED];
  bool        group_known       [MAX_GROUP];
  sh_int      points;
  bool        confirm_delete;
  char      * alias             [MAX_ALIAS];
  char      * alias_sub         [MAX_ALIAS];
  int         balance;
  int         shares;
  int         security;
  long        security_flags;
  sh_int      clan_rank; /* 0 for unguilded and loner 1-10 */
  /* otherwise */
  int         donated_dia;
  int         bounty;
  int         degrading;
  bool        user_set_title;
  bool        old_char;
  char      * history;
  char      * forget[MAX_FORGET];
  int         home_vnum;
  /* score record */
  int         pkills;
  int         pdeaths;
  int         kills;
  int         deaths;
  /* denial time */
  int         denytime;
  int         denied_time;
  /* clan data */
  int         clanowe_dia;
  int         clanowe_level;
  char      * clanowe_clan;
  int         numclans;
 /* locker data (currently disabled) */
  sh_int      locker_max;
  sh_int      locker_content;
  OBJ_DATA  * locker;
  /* Tag game data (will probably move to a GAME_DATA structure) */
  sh_int      num_tags;
  sh_int      num_tagged;
  AREA_DATA * tag_area;
  bool        tag_it;
  /* Fishing results */
  int         fish_caught;
  int         fish_lost;
  int         fish_broken; // rods
  /* IMM worklist counters */
  int         wlbugs;
  int         wltypos;
  int         wlduhs;
  int         wlbuild;
  int         wlhelps;
  /* PC worklist items submitted */
  int         fdbugs;
  int         fdtypos;
  int         fdhelps;
  int         fdduhs;
  /* Physical attributes */
  char *      eye_color;
  char *      hair_color;
  char *      skin_color;
  sh_int      weight; // lb
  sh_int      height; // in
  sh_int      afktime;
  int64       mpquests;
  // Occultist Bloodshards
  int         bloodshards;
#if MEMDEBUG
  /* Extra fields for comparisons */
  char      * memdebug_title;
  char      * memdebug_permtitle;
  char      * memdebug_pwd;
#endif
};

/* Data for generating characters -- only used during generation */
struct gen_data
{
    GEN_DATA * next;
    bool       valid;
    bool       skill_chosen[MAX_SKILL];
    bool       group_chosen[MAX_GROUP];
    int        points_chosen;
};

/*
 *  * OLC Security flags.
 *   */
#define OLC_SEC_AREA       (A)
#define OLC_SEC_HELP       (B)
#define OLC_SEC_HELPAREA   (C)
#define OLC_SEC_MOBILE     (D)
#define OLC_SEC_OBJECT     (E)
#define OLC_SEC_PROGRAM    (F)
#define OLC_SEC_ROOM       (G)
#define OLC_SEC_SOCIAL     (H)
#define OLC_SEC_AREA_LINK  (I)
#define OLC_SEC_RESET      (J)
#define OLC_SEC_SAVE       (K)
#define OLC_SEC_CLAN       (L)
#define OLC_SEC_SECURITY  (aa)

/*
 * Toggle Flags
 */
#define TOGGLE_STATSHOW    (A)
#define TOGGLE_AFFSHOW     (B)


/*
 * Liquids.
 */
#define LIQ_WATER 0

struct liq_type
{
  char   * liq_name;
  char   * liq_color;
  sh_int   liq_affect[5];
};

typedef struct auction_data AUCTION_DATA;

struct auction_data
{
  OBJ_DATA  * item;
  CHAR_DATA * owner;
  CHAR_DATA * high_bidder;
  sh_int      status;
  long        current_bid;
  long        silver_held;
  long        gold_held;
  long        minimum_bid;
};

extern AUCTION_DATA auction_info;

#define PULSE_AUCTION  (20 * PULSE_PER_SECOND)
#define MINIMUM_BID    100
#define AUCTION_LENGTH   5

/*
 * Extra description data for a room or object.
 */
struct    extra_descr_data
{
    EXTRA_DESCR_DATA * next;        /* Next in list                     */
    bool               valid;
    char             * keyword;  /* Keyword in look/examine          */
    char             * description;  /* What to see                  */
};

/*
 * Traps, started 15/11 -06 -- Merak
 */
struct  trap_data
{
    TRAP_DATA * next;
    char      * trig_msg;       // What it says when it goes of..
    int         dam_type;
    int         level;
    int         delay;          // The time left before it vanish..
    int         trap_flags;
};
/*
 * Prototype for an object.
 */
struct obj_index_data
{
  OBJ_INDEX_DATA   * next;
  EXTRA_DESCR_DATA * extra_descr;
  AFFECT_DATA      * affected;
  AREA_DATA        * area;        /* OLC */
  MPROG_LIST       * mprogs;
  sh_int             clan;
  char             * clan_name;
  int                mprog_flags;
  bool               new_format;
  char             * name;
  char             * short_descr;
  char             * description;
  sh_int             vnum;
  sh_int             reset_num;
  char             * material;
  sh_int             item_type;
  int64              extra_flags;
  int64              wear_flags;
  sh_int             level;
  sh_int             condition;
  sh_int             count;
  sh_int             weight;
  int                cost;
  int                value[7];
};

/*
 * One object.
 */
struct obj_data
{
  OBJ_DATA         * next;
  OBJ_DATA         * next_content;
  OBJ_DATA         * contains;
  OBJ_DATA         * in_obj;
  OBJ_DATA         * on;
  CHAR_DATA        * carried_by;
  EXTRA_DESCR_DATA * extra_descr;
  AFFECT_DATA      * affected;
  OBJ_INDEX_DATA   * pIndexData;
  ROOM_INDEX_DATA  * in_room;
  bool               valid;
  bool               enchanted;
  int64              active_flags;
  char             * owner;
  char             * name;
  char             * short_descr;
  char             * description;
  sh_int             item_type;
  int64              extra_flags;
  int64              wear_flags;
  sh_int             wear_loc;
  sh_int             weight;
  int                cost;
  sh_int             level;
  sh_int             condition;
  char             * material;
  sh_int             timer;
  char             * famowner;
  char             * mprog_target;
  sh_int             mprog_delay;
  int                value[7];
};

/*
 * Exit data.
 */
struct    exit_data
{
  union
  {
    ROOM_INDEX_DATA * to_room;
    sh_int            vnum;
  } u1;
  sh_int              exit_info;
  sh_int              key;
  char              * keyword;
  char              * description;
  EXIT_DATA         * next;        /* OLC */
  int                 rs_flags;    /* OLC */
  int                 closed_flags; 
  int                 open_flags;
  int                 orig_door;    /* OLC */
};

/*
 * Area-reset definition.
 */
struct reset_data
{
  RESET_DATA * next;
  char         command;
  sh_int       arg1;
  sh_int       arg2;
  sh_int       arg3;
  sh_int       arg4;
};

/*
 * Area Sys Flags
 */
#define AREA_NO_QUEST   (A)
#define AREA_DRAFT      (B)
#define AREA_CRYSTAL    (C)
#define AREA_NOGATE     (D)
#define AREA_NOSUMMON   (E)
#define AREA_NORESCUE   (F)
#define AREA_LIBRARY    (G)
#define AREA_CLANHALL   (H)

/*
 * Area Align
 */
#define AREA_ALIGN_ALL     0
#define AREA_ALIGN_GOOD    1
#define AREA_ALIGN_EVIL    2
#define AREA_ALIGN_NEUTRAL 3

/*
 * Area definition.
 */
struct    area_data
{
  AREA_DATA  * next;
  AREA_DATA  * next_sort;
  RESET_DATA * reset_first;
  RESET_DATA * reset_last;
  char       * file_name;
  char       * name;
  char       * credits;
  sh_int       age;
  sh_int       nplayer;
  sh_int       low_range;
  sh_int       high_range;
  sh_int       min_vnum;
  sh_int       max_vnum;
  sh_int       align;
  bool         empty;
  char       * builders;      /* OLC */ /* Listing of */
  int          vnum;            /* OLC */ /* Area vnum  */
  int          area_flags;    /* OLC */ /* System Flags */
  int          security;      /* OLC */ /* Value 1-9  */
  int          numkills;
  sh_int       version;
  long         flags;
  int          continent;  /* OLC */ /* continent settings */
  int          reset_rate; /* OLC */ /* Set repop rates */
};

/* Clan System Started late 2006 RWL III */
struct clan_data
{
  AREA_DATA   * area;                   //For clan hall referencing
  CLAN_DATA   * next;                   //Sort through clans in for loop
  RANK_DATA   * rank;                   // A list of the available ranks
  int           max_rank;
  char        * name;                   //Clan Name
  char        * symbol;                 //Clan Symbol
  char        * clan_immortal;          //Reference to who they bless to, donate to, sacrifice to etc.
  ROSTER_DATA * roster;                 //Roster list
  char        * roster_style;           //Roster display style (a la prompt)
  int           actual_members;
  int           actual_ranks;
  int64         clan_flags;             //Clan Status ( Independent, Peaceful, Law, etc )
  int           locker;                 //Locker obj vnum
  int           recall[MAX_CONTINENT];  //Recall room vnums (per-continent)
  int           donation_obj[MAX_CONTINENT]; //Obj vnum for clan donating
  int           donation_gem;           //Clan Donation Gem
  int           donation_total;         //Total # of Clan Donations
  int           donation_balance;       //Current Balance of Clan Donations
  int           donation_spent;         //Amount Spent from the Clan Total
  void        * temp_item;
  bool          creating;
};

/* Roster Information 10/06 */
struct roster_data
{
    ROSTER_DATA * next;
    char        * name;
    char        * title;
    char        * afk_title;
    char        * rank_symbol;
    char        * race;
    char        * pc_class;
    int           alignment;
    int           rank;
    int           donated;
    int           sex;
    int           level;
    int           pkills;
    int           pdeaths;
    int           kills;
    int           deaths;
    int           promoted;
    int           guilded;
    long          logon;
};

struct rank_data
{
    RANK_DATA * next;
    int         number; // The position in the list
    int         level;  // This is the actual rank...
    char      * male;
    char      * female;
    char      * neutral;
    int64       rank_flags;
}; 

/*
 * Room type.
 */
struct room_index_data
{
  ROOM_INDEX_DATA  * next;
  CHAR_DATA        * people;
  OBJ_DATA         * contents;
  EXTRA_DESCR_DATA * extra_descr;
  AREA_DATA        * area;
  EXIT_DATA        * exit    [MAX_EXITS];
  EXIT_DATA        * old_exit[6];
  MPROG_LIST       * mprogs;
  RESET_DATA       * reset_first;    /* OLC */
  RESET_DATA       * reset_last;    /* OLC */
  char             * name;
  char             * description;
  char             * owner;
  sh_int             vnum;
  int64              room_flags;
  sh_int             light;
  sh_int             sector_type;
  sh_int             heal_rate;
  sh_int             mana_rate;
  sh_int             clan;
  char             * clan_name;
  bool               mapped_room;
  bool               linked;
  CHAR_DATA        * mprog_target;
  long               mprog_flags;
  sh_int             mprog_delay;
  sh_int             state;
  sh_int             fish[7];

};

#define CLANOWE_LEVEL      5
#define CLANOWE_DIA_LEVEL 10

/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED  -1
#define TYPE_HIT      1000

/*
 *  Target types.
 */
#define TAR_IGNORE         0
#define TAR_CHAR_OFFENSIVE 1
#define TAR_CHAR_DEFENSIVE 2
#define TAR_CHAR_SELF      3
#define TAR_OBJ_INV        4
#define TAR_OBJ_CHAR_DEF   5
#define TAR_OBJ_CHAR_OFF   6

#define TARGET_CHAR        0
#define TARGET_OBJ         1
#define TARGET_ROOM        2
#define TARGET_NONE        3


/*
 * Skills include spells as a particular case.
 */
struct skill_type
{
  char        name[MSL];                /* Name of skill        */
  sh_int      skill_level[MAX_CLASS];    /* Level needed by class    */
  sh_int      rating[MAX_CLASS];      /* How hard it is to learn    */
  SPELL_FUN * spell_fun;                /* Spell pointer (for spells)    */
  sh_int      target;                    /* Legal targets        */
  sh_int      minimum_position;       /* Position for caster / user    */
  sh_int    * pgsn;                    /* Pointer to associated gsn    */
  sh_int      slot;                    /* Slot for #OBJECT loading    */
  sh_int      min_mana;                 /* Minimum mana used        */
  sh_int      beats;                    /* Waiting time after use    */
  char        noun_damage[MSL];        /* Damage message        */
  char        msg_off[MSL];            /* Wear off message        */
  char        msg_obj[MSL];            /* Wear off message for obects    */
};

struct group_type
{
  char   * name;
  sh_int   rating[MAX_CLASS];
  sh_int   proficiency[MAX_CLASS];
  char   * spells[MAX_IN_GROUP];
};

/*
 * MOBprog definitions
 */
#define TRIG_ACT    (A)
#define TRIG_BRIBE  (B)
#define TRIG_DEATH  (C)
#define TRIG_ENTRY  (D)
#define TRIG_FIGHT  (E)
#define TRIG_GIVE   (F)
#define TRIG_GREET  (G)
#define TRIG_GRALL  (H)
#define TRIG_KILL   (I)
#define TRIG_HPCNT  (J)
#define TRIG_RANDOM (K)
#define TRIG_SPEECH (L)
#define TRIG_EXIT   (M)
#define TRIG_EXALL  (N)
#define TRIG_DELAY  (O)
#define TRIG_SURR   (P)
#define TRIG_TOUCH  (Q)
#define TRIG_WEAR   (R)
#define TRIG_REMOVE (S)
#define TRIG_DIG    (T)
#define TRIG_PRESS  (U)
#define TRIG_TURN   (V)
#define TRIG_GRONCE (W)
#define TRIG_PULL   (X)
#define TRIG_GET    (Y)
#define TRIG_SOCIAL (Z)
#define TRIG_ONLOAD (aa)

struct mprog_list
{
    int          trig_type;
    char       * trig_phrase;
    sh_int       vnum;
//    char       * code;
    MPROG_CODE * code;
    MPROG_LIST * next;
    bool         valid;
    
};

struct mprog_code
{
    AREA_DATA  * area;
    sh_int       vnum;
    char       * code;
    MPROG_CODE * next;
    char       * name;
};

/*
 * These are skill_lookup return values for common skills and spells.
 */

/*
 * Stock gsn
 */
extern sh_int gsn_backstab;
extern sh_int gsn_dodge;
extern sh_int gsn_envenom;
extern sh_int gsn_toxicology;
extern sh_int gsn_hide;
extern sh_int gsn_cloak_of_assassins;
extern sh_int gsn_peek;
extern sh_int gsn_pick_lock;
extern sh_int gsn_sneak;
extern sh_int gsn_steal;

extern sh_int gsn_disarm;
extern sh_int gsn_enhanced_damage;
extern sh_int gsn_kick;
extern sh_int gsn_parry;
extern sh_int gsn_rescue;
extern sh_int gsn_double_shot;
extern sh_int gsn_second_attack;
extern sh_int gsn_third_attack;
extern sh_int gsn_fourth_attack;

extern sh_int gsn_blindness;
extern sh_int gsn_change_sex;
extern sh_int gsn_charm_person;
extern sh_int gsn_curse;
extern sh_int gsn_invis;
extern sh_int gsn_mass_invis;
extern sh_int gsn_plague;
extern sh_int gsn_poison;
extern sh_int gsn_sleep;
extern sh_int gsn_fly;
extern sh_int gsn_sanctuary;
extern sh_int gsn_weaken;
extern sh_int gsn_holy_rights;

/* new gsns */
extern sh_int gsn_axe;
extern sh_int gsn_dagger;
extern sh_int gsn_flail;
extern sh_int gsn_mace;
extern sh_int gsn_polearm;
extern sh_int gsn_crossbow;
extern sh_int gsn_shield_block;
extern sh_int gsn_spear;
extern sh_int gsn_sword;
extern sh_int gsn_whip;

extern sh_int gsn_bash;
extern sh_int gsn_berserk;
extern sh_int gsn_rampage;
extern sh_int gsn_dirt;
extern sh_int gsn_hand_to_hand;
extern sh_int gsn_trip;
extern sh_int gsn_sweep;

extern sh_int gsn_fast_healing;
extern sh_int gsn_haggle;
extern sh_int gsn_lore;
extern sh_int gsn_meditation;

extern sh_int gsn_scrolls;
extern sh_int gsn_staves;
extern sh_int gsn_wands;
extern sh_int gsn_recall;
extern sh_int gsn_butcher;
extern sh_int gsn_brew;
extern sh_int gsn_advanced_brew;
extern sh_int gsn_scribe;
extern sh_int gsn_advanced_scribe;
extern sh_int gsn_craft;
extern sh_int gsn_advanced_craft;
extern sh_int gsn_erase;
extern sh_int gsn_empty;
extern sh_int gsn_stake;
extern sh_int gsn_counter;
extern sh_int gsn_familiar;
extern sh_int gsn_critical;
extern sh_int gsn_tithe;
extern sh_int gsn_whirlwind;
extern sh_int gsn_tumble;
extern sh_int gsn_circle;
extern sh_int gsn_shatter;
extern sh_int gsn_bind;
extern sh_int gsn_strangle;
extern sh_int gsn_crosscut;
extern sh_int gsn_gore;
extern sh_int gsn_karate;
extern sh_int gsn_nerve;
extern sh_int gsn_takedown;
extern sh_int gsn_focus;
extern sh_int gsn_buckkick;
extern sh_int gsn_karate;
extern sh_int gsn_exotic;
extern sh_int gsn_track;
extern sh_int gsn_second_cast;
extern sh_int gsn_third_cast;
extern sh_int gsn_automap;
extern sh_int gsn_dart;
extern sh_int gsn_acid_breath;
extern sh_int gsn_feint;
extern sh_int gsn_fifth_attack;
extern sh_int gsn_dual_attack;
extern sh_int gsn_triple_attack;
extern sh_int gsn_stomp;
extern sh_int gsn_flying;
extern sh_int gsn_demand;
extern sh_int gsn_bite;
extern sh_int gsn_second;
extern sh_int gsn_hiccup;
extern sh_int gsn_yawn;
extern sh_int gsn_basic_style;
extern sh_int gsn_dragon_style;
extern sh_int gsn_drunk_style;
extern sh_int gsn_tiger_style;
extern sh_int gsn_snake_style;
extern sh_int gsn_crane_style;
extern sh_int gsn_ironfist_style;
extern sh_int gsn_judo_style;
extern sh_int gsn_ghost_time;
extern sh_int gsn_nochannel;
extern sh_int gsn_ironwill;
extern sh_int gsn_bear_spirit;
extern sh_int gsn_eagle_spirit;
extern sh_int gsn_tiger_spirit;
extern sh_int gsn_dragon_spirit;
extern sh_int gsn_twit;
extern sh_int gsn_martyr;
extern sh_int gsn_rub;
extern sh_int gsn_dagger_twist;
extern sh_int gsn_adrenalize;
extern sh_int gsn_sharpen;
extern sh_int gsn_shelter;
extern sh_int gsn_investigate;
extern sh_int gsn_warcry_hardening;
extern sh_int gsn_warcry_rage;
extern sh_int gsn_warcry_vigor;
extern sh_int gsn_warcry_guarding;
extern sh_int gsn_warcry_shout;
extern sh_int gsn_tail;
extern sh_int gsn_forage;
extern sh_int gsn_repair;
extern sh_int gsn_descry;
extern sh_int gsn_sidestep;
extern sh_int gsn_blade_weave;
extern sh_int gsn_flare;
extern sh_int gsn_crusade;
extern sh_int gsn_guided_strike;
extern sh_int gsn_layhands;
extern sh_int gsn_throw;
extern sh_int gsn_rebrew;
extern sh_int gsn_transcribe;
extern sh_int gsn_inverted_light;
extern sh_int gsn_chameleon;
extern sh_int gsn_shield_bash;

/* memory functional */
extern int nAllocString;
extern int nAllocPerm;
extern int sAllocPerm;

/*
 * Utility macros.
 */
#define IS_VALID(data)       ((data) != NULL && (data)->valid)
#define VALIDATE(data)       ((data)->valid = TRUE)
#define INVALIDATE(data)     ((data)->valid = FALSE)
#define UMIN(a, b)           ((a) < (b) ? (a) : (b))
#define UMAX(a, b)           ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)      ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define PERCENT(n, d)        ((d)==0 ? 0 : 100*(n)/(d))
#define DICE_MIN(n, d)       ((n)>0&&(d)>0 ? (n) : 0)
#define DICE_AVG(n, d)       ((n)>0&&(d)>0 ? (n)*((d)+1)/2 : 0)
#define DICE_MAX(n, d)       ((n)>0&&(d)>0 ? (n)*(d) : 0)
#define UABS(a)              ((a) < 0 ? -(a) : (a))
#define LOWER(c)             ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)             ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define DIFF(a, b)           ((a > b) ? (a - b) : (b - a))
#define IS_SET(flag, bit)    (((int64)flag) & ((int64)bit))
#define SET_BIT(var, bit)    ((var) |= ((int64)bit))
#define REMOVE_BIT(var, bit) ((var) &= ~((int64)bit))
#define IS_NULLSTR(str)      ((str) == NULL || (str)[0] == '\0')
#define ENTRE(min,num,max)   ( ((min) < (num)) && ((num) < (max)) )
#define LEVEL_PLAY_TIME(ch)  (ch->played + (int)(current_time - ch->last_level ) )
#define TOTAL_PLAY_TIME(ch)  (ch->played + (int)(current_time - ch->logon - ch->idle_time) )
#define IS_ARENA(ch)         ((ch)->in_room && IS_SET( (ch)->in_room->room_flags, ROOM_ARENA))
#define GODNAME(ch,vch)      (can_see((vch),(ch)) ?             \
                               IS_NULLSTR((ch)->short_descr) ?  \
                               (ch)->name : (ch)->short_descr : \
                               "Someone powerful" )

char *get_clan_desc_char(CHAR_DATA *ch);
char *get_clan_desc(CLAN_DATA *clan);
char *get_clan_rank_char(CHAR_DATA *ch);
char *get_clan_rank(int clan, int clan_rank, int sex);
char *get_clan_rank_roster(int clan, int i);

/*SECURITY FEATURES - Added End Sept, 2002*/
#define OLC_SECURITY(ch)    ( IS_NPC( ch ) ? 0 : (ch)->pcdata->security )
#define OLC_SECURITY_FLAGS(ch,bit) ( IS_NPC( ch ) ? FALSE \
                : IS_SET( (ch)->pcdata->security_flags, bit ) )

/*
 * OLC_CLAN_FLAGS
 * For an easy if-check on Clan Status Flags
 */

/*
 * Character macros.
 */
#define IS_NPC(ch)             (IS_SET((ch)->act, ACT_IS_NPC))
#define IS_IMMORTAL(ch)        (get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_IMPLEMENTOR(ch)     (get_trust(ch) >= IMPLEMENTOR )
#define IS_PET(ch)             (IS_NPC((ch)) && IS_SET((ch)->act, ACT_PET))
#define IS_LEADER(ch)          (!IS_NPC((ch)) && ((ch)->clan) && ((ch)->pcdata->clan_rank == 10))
#define IS_IN_CLAN( ch )       ( ch->clan )
#define IS_INDEPENDENT(ch)     (!IS_NPC(ch) && clan_table[(ch)->clan].independent)
#define IS_BLIND(ch)           (IS_AFFECTED((ch),AFF_BLIND))
#define IS_GHOST(ch)           (!IS_NPC(ch) && IS_SET((ch)->spell_aff,SAFF_GHOST))
#define IS_VIOLENT(ch)         (!IS_NPC(ch) && IS_SET((ch)->act,PLR_VIOLENT))
#define IS_KILLER(ch)          (!IS_NPC(ch) && IS_SET((ch)->act,PLR_KILLER))
#define IS_THIEF(ch)           (!IS_NPC(ch) && IS_SET((ch)->act,PLR_THIEF))
#define IS_TWIT(ch)            (!IS_NPC(ch) && IS_SET((ch)->act,PLR_TWIT))
#define IS_ADRENALIZED(ch)     (!IS_NPC(ch) && IS_SAFFECTED(ch, SAFF_ADRENALIZE))
#define IS_SHOPKEEPER(ch)      (IS_NPC(ch) && (ch)->pIndexData->pShop != NULL)
#define IS_CHARMED(ch)         (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL)
#define IS_FIGHTING(ch)        (ch->fighting != NULL)
#define IS_HERO(ch)            (get_trust(ch) >= LEVEL_HERO)
#define IS_TRUSTED(ch,level)   (get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)    (IS_SET((ch)->affected_by, (sn)))
#define IS_AFFECTED2(ch, sn)   (IS_SET((ch)->affected2_by, (sn)))
#define IS_LINKDEAD(ch)        (IS_SET((ch)->affected_by, AFF_LINKDEATH))
#define IS_PLOADED(ch)         (IS_SET((ch)->active_flags, ACTIVE_PLOAD))
#define IS_AFK(ch)             (IS_SET((ch)->comm_flags, COMM_AFK))
#define IS_NOCHANNELED(ch)     (IS_SET((ch)->pen_flags, PEN_NOCHANNELS))
#define IS_ROOM_SAFE(ch)       (IS_SET((ch)->in_room->room_flags,ROOM_SAFE))
#define IS_SAFFECTED(ch, sn)   (IS_SET((ch)->spell_aff, (sn)))
#define IN_RANGE(a,b,c)        (((a)<(c)) ? (((b)>=(a))&&((b)<=(c))) : (((b)<=(a))&&((b)>=(c))))     /* b inclusive between a and c, up or down*/
#define IS_WIZINVIS(ch,looker) (get_trust(looker) < (ch)->invis_level)
#define IS_DIAMOND(obj)        ((obj)->pIndexData->vnum == 3377 || \
                                (obj)->pIndexData->vnum == 9604 || \
                                (obj)->pIndexData->vnum == 27020|| \
                                (obj)->pIndexData->vnum == 8413 || \
                                (obj)->pIndexData->vnum == 5650 )
#define DOES_RESIST(ch, sn)    (IS_SET((ch)->res_flags, (sn)))
#define HAS_PK_FLAGS(ch)       (IS_SET(ch->act, PLR_KILLER ) || \
                                IS_SET(ch->act, PLR_THIEF  ) || \
                                IS_SET(ch->act, PLR_TWIT   ) || \
                                IS_SET(ch->act, PLR_VIOLENT) )
#define IS_NIGHT               (time_info.hour < 5 || time_info.hour > 20)

bool melt_drop(CHAR_DATA *ch, OBJ_DATA *obj);
#define is_clan(ch)            (ch->clan)
#define is_same_clan(ch, vic)  ( is_clan( ch ) ? ( ( ch->clan == \
                                 vic->clan) \
                                 ? 1: 0 ): 0 )
#define material_lookup(name)  (0)
//#define get_age(ch)  (17+(ch->played + (int)(current_time-ch->logon))/72000)

#define GET_AGE(ch)        ((int) (17 + ((ch)->played + current_time - (ch)->logon )/72000))
AFFECT_DATA *affect_exist(CHAR_DATA *ch, int gsn);

#define IS_GOOD(ch)          (ch->alignment >= 350)
#define IS_EVIL(ch)          (ch->alignment <= -350)
#define IS_NEUTRAL(ch)       (!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)         (ch->position > POS_SLEEPING)
#define GET_AC(ch,type)      ((ch)->armor[type] \
                               + ( IS_AWAKE(ch)   \
                               ? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0 ))
#define GET_HP(ch)           ((ch)->max_hit+con_app[get_curr_stat(ch,STAT_CON)].hp_add )

#define GET_MANA(ch)         ((ch)->max_mana+int_app[get_curr_stat(ch,STAT_INT)].mana_add )

#define GET_HITROLL(ch)      ((ch)->hitroll+str_app[get_curr_stat(ch,STAT_STR)].tohit)
#define GET_DAMROLL(ch)      ((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam)

#define GET_FRAG_PERCENT(ch) (wis_app[get_curr_stat(ch,STAT_WIS)].frag_percent / 100 )

#define IS_OUTSIDE(ch)         ( !IS_SET((ch)->in_room->room_flags,ROOM_INDOORS)   \
							 &&  ((ch)->in_room->sector_type != SECT_INSIDE)       \
                             &&  ((ch)->in_room->sector_type != SECT_UNDERGROUND)  )

#define GET_CARRY_WEIGHT(ch) ((ch)->carry_weight + SILVER_WEIGHT((ch)->silver) + GOLD_WEIGHT((ch)->gold))
#define SILVER_WEIGHT(coins) ((coins)/10)
#define GOLD_WEIGHT(coins)   (2*(coins)/5)
#define GET_WEALTH(ch)       ( (ch)->silver + ((ch)->gold * 100) )

/* New Macros for class, race, and clan added by Sartan 10/30/01 */
#define IS_CLASS_CONJURER(ch)   (ch->gameclass == cConjurer)
#define IS_CLASS_KNIGHT(ch)     (ch->gameclass == cKnight)
#define IS_CLASS_PRIEST(ch)     (ch->gameclass == cPriest)
#define IS_CLASS_HIGHWAYMAN(ch) (ch->gameclass == cHighwayman)
#define IS_CLASS_DRUID(ch)      (ch->gameclass == cDruid)
#define IS_CLASS_WARLOCK(ch)    (ch->gameclass == cWarlock)
#define IS_CLASS_BARBARIAN(ch)  (ch->gameclass == cBarbarian)
#define IS_CLASS_MYSTIC(ch)     (ch->gameclass == cMystic)
#define IS_CLASS_INQUISITOR(ch) (ch->gameclass == cInquisitor)
#define IS_CLASS_OCCULTIST(ch)  (ch->gameclass == cOccultist)
#define IS_CLASS_ALCHEMIST(ch)  (ch->gameclass == cAlchemist)
#define IS_CLASS_WOODSMAN(ch)   (ch->gameclass == cWoodsman)

#define IS_HUMAN(ch)            (ch->race == race_lookup("human") )
#define IS_ELF(ch)              (ch->race == race_lookup("elf") )
#define IS_DWARF(ch)            (ch->race == race_lookup("dwarf") )
#define IS_GIANT(ch)            (ch->race == race_lookup("giant") )
#define IS_BROWNIE(ch)          (ch->race == race_lookup("brownie") )
#define IS_VAMPIRE(ch)          (ch->race == race_lookup("vampire") )
#define IS_MINOTAUR(ch)         (ch->race == race_lookup("minotaur") )
#define IS_CHOJA(ch)            (ch->race == race_lookup("cho-ja") )
#define IS_ETTIN(ch)            (ch->race == race_lookup("ettin") )
#define IS_QUICKLING(ch)        (ch->race == race_lookup("quickling") )
#define IS_WEREWOLF(ch)         (ch->race == race_lookup("werewolf") )

bool is_in_pk_range(CHAR_DATA *ch,CHAR_DATA *victim);
int  num_followers(CHAR_DATA *ch);

#define WAIT_STATE(ch, npulse)  ((ch)->wait = UMAX((ch)->wait, (npulse)))
#define DAZE_STATE(ch, npulse)  ((ch)->daze = UMAX((ch)->daze, (npulse)))
#define HAS_TRIGGER(ch,trig)    ((IS_NPC(ch)) && (IS_SET((ch)->pIndexData->mprog_flags,(trig))))
#define IS_SWITCHED( ch )       ( ch->desc && ch->desc->original )
#define IS_BUILDER(ch, Area)    ( !IS_NPC(ch) && !IS_SWITCHED( ch )       \
                                  && ( OLC_SECURITY(ch) >= Area->security \
                                  || strstr( Area->builders, ch->name )   \
                                  || strstr( Area->builders, "All" ) ) )
#define act(format,ch,arg1,arg2,type) act_new((format),(ch),(arg1),(arg2),(type),POS_RESTING)

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)      (IS_SET((obj)->wear_flags,  (part)))
#define IS_WORN(obj)             ((obj)->wear_loc!=WEAR_NONE)
#define IS_OBJ_STAT(obj, stat)   (IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat) (IS_SET((obj)->value[4],(stat)))
#define WEIGHT_MULT(obj)         ((obj)->item_type == ITEM_CONTAINER ? (obj)->value[4] : 100)
#define MONEY_POUCH_WEIGHT_MULT  5

/*
 * Description macros.
 */
#define PERS(ch, looker)             ( can_see( looker, (ch) ) ? \
                                       (     IS_NPC(ch) ? (ch)->short_descr   : (ch)->name ) : \
                                       (IS_IMMORTAL(ch) ? "An unknown entity" : "Someone") )
#define AREA_IS_OPEN(area)           ((area) != NULL && (area)->security > 7 )
#define FIX_STR(str,if_zero,if_null) ((str)==NULL?(if_null):(str)[0]=='\0'?(if_zero):(str))
#define IS_DELETED(data)             ((data)!=NULL)

/*
 * Structure for a social in the socials table.
 */
struct social_type
{
    char   name[20];
    char * char_no_arg;
    char * others_no_arg;
    char * char_found;
    char * others_found;
    char * vict_found;
    char * char_not_found;
    char * char_auto;
    char * others_auto;
};

/*
 * Global constants.
 */
extern const struct str_app_type  str_app       [31];
extern const struct int_app_type  int_app       [31];
extern const struct wis_app_type  wis_app       [31];
extern const struct dex_app_type  dex_app       [31];
extern const struct con_app_type  con_app       [31];
extern const struct class_type    class_table   [MAX_CLASS];
extern const struct weapon_type   weapon_table  [];
extern const struct item_type     item_table    [];
extern const struct wiznet_type   wiznet_table  [];
extern const struct game_type     game_table    [];
extern const struct attack_type   attack_table  [];
extern const struct race_type     race_table    [];
extern const struct pc_race_type  pc_race_table [];
extern const struct spec_type     spec_table    [];
extern const struct liq_type      liq_table     [];
extern       struct skill_type    skill_table   [MAX_SKILL];
extern       struct group_type    group_table   [MAX_GROUP];
extern       struct social_type * social_table;

/*
 * Global variables.
 */
extern HELP_DATA       * help_first;
extern SHOP_DATA       * shop_first;
extern CHAR_DATA       * char_list;
extern CHAR_DATA       * player_list;
extern DENIED_DATA     * denied_list;
extern DESCRIPTOR_DATA * descriptor_list;
extern OBJ_DATA        * object_list;
extern CHAR_DATA       * prog_master;
extern MPROG_CODE      * mprog_list;
extern char              bug_buf       [];
extern time_t            current_time;
extern time_t            update_time;
extern time_t            lag_update_time;
extern time_t            update_time;
extern time_t            tml_boot_time;
extern bool              fLogAll;
extern bool              MOBtrigger;
extern FILE            * fpReserve;
extern KILL_DATA         kill_table    [];
extern char              log_buf       [];
extern TIME_INFO_DATA    time_info;
extern WEATHER_DATA      weather_info;
extern int               share_value;
extern AREA_DATA       * area_first_sorted;

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if defined(_AIX)
char * crypt  args( ( const char *key, const char *salt ) );
#endif

#if defined(apollo)
int    atoi   args( ( const char *string ) );
void * calloc args( ( unsigned nelem, size_t size ) );
char * crypt  args( ( const char *key, const char *salt ) );
#endif

#if defined(hpux)
char * crypt  args( ( const char *key, const char *salt ) );
#endif

#if defined(linux)
char * crypt  args( ( const char *key, const char *salt ) );
#endif

#if defined(macintosh)
#define NOCRYPT
#if defined(unix)
#undef unix
#endif
#endif

#if defined(MIPS_OS)
char * crypt  args( ( const char *key, const char *salt ) );
#endif

#if defined(MSDOS)
#define NOCRYPT
#if defined(unix)
#undef unix
#endif
#endif

#if defined(NeXT)
char * crypt  args( ( const char *key, const char *salt ) );
#endif

#if defined(sequent)
char * crypt   args( ( const char *key, const char *salt ) );
int    fclose  args( ( FILE *stream ) );
int    fprintf args( ( FILE *stream, const char *format, ... ) );
int    fread   args( ( void *ptr, int size, int n, FILE *stream ) );
int    fseek   args( ( FILE *stream, long offset, int ptrname ) );
void   perror  args( ( const char *s ) );
int    ungetc  args( ( int c, FILE *stream ) );
#endif

#if defined(sun)
char * crypt   args( ( const char *key, const char *salt ) );
int    fclose  args( ( FILE *stream ) );
int    fprintf args( ( FILE *stream, const char *format, ... ) );
#if    defined(SYSV)
siz_t  fread   args( ( void *ptr, size_t size, size_t n, FILE *stream) );
#endif
int  fseek     args( ( FILE *stream, long offset, int ptrname ) );
void perror    args( ( const char *s ) );
int  ungetc    args( ( int c, FILE *stream ) );
#endif

#if defined(ultrix)
char * crypt   args( ( const char *key, const char *salt ) );
#endif

#if defined(NOCRYPT)
#define crypt(s1, s2) (s1)
#endif

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(macintosh)
#define PLAYER_DIR   ""
#define TEMP_FILE    "mtltmp"
#define NULL_FILE    "proto.are"
#endif

#if defined(MSDOS)
#define PLAYER_DIR   ""
#define TEMP_FILE    "mtltmp"
#define NULL_FILE    "nul"
#endif

#if defined(unix)
#define PLAYER_DIR      "../player/"
#define GOD_DIR         "../gods/"
#define TEMP_FILE       "../player/mtltmp"
#define NULL_FILE        "/dev/null"
#endif
#define LOG_DIR          "../log"
#define DEAD_PLAYER_DIR  "../Deadplayer/"
#define CLASS_DIR        "../Classes"
#define GROUP_DIR        "../Groups"
#define CLAN_DIR         "../Txt/Clan/"
#define CLAN_TEMP_FILE   "../Txt/Clan/mtltmp"
#define CLAN_FILE        "../Txt/clan.txt"
#define ROSTER_FILE      "../Txt/roster.txt"
#define SPELLS_FILE      "../Txt/skills.txt"
#define WOR_FILE         "../Txt/wor.dat"
#define WOR_SRT          "../Txt/wor.srt"
#define AREA_LIST        "area.lst"
#define BUG_FILE         "../Txt/bugs.txt"
#define BUILDER_FILE     "../Txt/builder.txt"
#define HINTS_FILE       "../Txt/hints.txt"
#define TYPO_FILE        "../Txt/typos.txt"
#define DUH_FILE         "../Txt/duh.txt"
#define NOTE_FILE        "../Msg/notes.not"
#define TODO_FILE        "../Txt/helps.txt"
#define TASK_FILE        "../Txt/tasks.txt"
#define IDEA_FILE        "../Msg/ideas.not"
#define PENALTY_FILE     "../Msg/penal.not"
#define NEWS_FILE        "../Msg/news.not"
#define CHANGES_FILE     "../Msg/chang.not"
#define RULES_FILE       "../Msg/rules.not"
#define RPNOTE_FILE      "../Msg/rpnotes.not"
#define SHUTDOWN_FILE    "../Txt/shutdown.txt"
#define BAN_FILE         "../Txt/ban.txt"
#define MUSIC_FILE       "../Txt/music.txt"
#define CLAN_STATUS_FILE "../Txt/clanstat.bin"
#define BANK_FILE        "../Txt/bank.txt"
#define AS_DIR_STRING    "../log"
#define CLAN_REQ_FILE    "clanreq.dat"
#define SYSTEM_FILE      "../Txt/sys%d.txt"
#define DENIED_FILE      "../Txt/denied.txt"
#define MAX_NUM_AUTOSNOOP 5

#ifdef CONST_SOCIAL
#define SOCIAL_FILE     "../Txt/social.are"
#else
#define SOCIAL_FILE     "../Txt/social.txt"
#endif

#define CH(descriptor) ((descriptor)->original ? (descriptor)->original : (descriptor)->character)

/* This file holds the copyover data */

/* This is the executable file */
#define EXE_FILE      "../Proc/sacred"

extern int nFilesOpen;
extern int nDescsOpen;

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD  CHAR_DATA
#define MID MOB_INDEX_DATA
#define OD  OBJ_DATA
#define OID OBJ_INDEX_DATA
#define RID ROOM_INDEX_DATA
#define SF  SPEC_FUN
#define AD  AFFECT_DATA
#define MPC MPROG_CODE

/* Maximum Creation points to prevent a bug */
#define MAX_CPS    200

/* act_comm.c */
void   check_sex        args( ( CHAR_DATA *ch) );
void   add_follower     args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void   stop_follower    args( ( CHAR_DATA *ch ) );
void   nuke_pets        args( ( CHAR_DATA *ch, bool show ) );
void   die_follower     args( ( CHAR_DATA *ch ) );
bool   is_same_group    args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
char * capitalize_color args( ( const char *str, bool lower ) );
void   logit            args((char * fmt, ...));
void   bugf          (char * fmt, ...);
enum   special_flags {spec_public_flag, spec_clan_flag, spec_allclan_flag, spec_imm_flag};
void   strip_color   (char *buffer, const char *txt);

/* act_enter.c */
RID  * get_random_room  args ( (CHAR_DATA *ch) );
void   update_chan_hist( CHAR_DATA *ch, const int channel, char buf[MAX_INPUT_LENGTH+200]);

/* act_info.c */
#define CLAN_DIA_STR "diamond gem" /* Serious midgard :) */
ROOM_INDEX_DATA *find_location( CHAR_DATA *, char * );

char * continent_name (AREA_DATA *pArea);
void   set_title    args( ( CHAR_DATA *ch, char *title ) );
void   set_titleafk  args( ( CHAR_DATA *ch, char *title ) );
void   do_diamonds( CHAR_DATA *ch, char *argument);
void   clrstrcenter(char *input_string, int size_of_string);
void   strlpad(char *string, int num);
void   strrpad(char *string,int num);
void   look_checkers   args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );

/* act_move.c */
int    find_door           args( ( CHAR_DATA *ch, char *arg ) );
void   set_state_room      args( ( ROOM_INDEX_DATA *room, int state ) );
int    arg_to_dirnum       args( ( char *arg ) );
int    move_char           args( ( CHAR_DATA *ch, int door, bool follow ) );
char * format_obj_to_char  args( ( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort, bool fShowLevel ) );
void   show_list_to_char   args( ( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing, bool fShowLevel, bool fDonationPit ) );
void   show_char_to_char_0 args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void   show_char_to_char_1 args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void   show_char_to_char   args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool   check_blind         args( ( CHAR_DATA *ch ) );

/* act_obj.c */
bool   can_loot    args( (CHAR_DATA *ch, OBJ_DATA *obj) );
void   wear_obj    args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace) );
void   get_obj     args( ( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container ) );
void   get_obj_list_to_char ( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing, char *arg1, bool is_container, OBJ_DATA *container );
bool   get_silent_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container );
bool   can_get_item( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container );
void   put_obj_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort,
               bool fShowNothing,char *arg1, bool
               is_container, OBJ_DATA *container );
bool   can_put_item( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container );
void   give_obj_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort,
               bool fShowNothing,char *arg1, CHAR_DATA *victim );
bool   can_give_item( CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *victim);
OBJ_DATA *  get_obj_keeper  args( ( CHAR_DATA *ch, CHAR_DATA *keeper, char *argument ) );
int    get_cost     args( ( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );

/* act_wiz.c */
typedef enum {exit_from, exit_to, exit_both} exit_status;
extern const sh_int opposite_dir [];
void   wiznet               args( (char *string, CHAR_DATA *ch, OBJ_DATA *obj, long flag, long flag_skip, int min_level ) );
void   wiznet_special       args( (char *string, CHAR_DATA *ch, long flag, long flag_skip, int min_level ) );
void   copyover_recover     args((void));
void   restore_char         (CHAR_DATA *vch);
void   fullrestore_char     (CHAR_DATA *vch, CHAR_DATA *ch);
ROOM_INDEX_DATA *find_location    args( ( CHAR_DATA *ch, char *arg ) );
char * area_name            (AREA_DATA *pArea);
void   room_pair            (ROOM_INDEX_DATA* left, ROOM_INDEX_DATA* right, exit_status ex, char *buffer);
void   checkexits           (ROOM_INDEX_DATA *room, AREA_DATA *pArea, char* buffer);
int    add_to_denied_file   (char *name);
bool   check_if_denied_file (char *filename);
int    read_denied_file     ( );

/* alias.c */
void substitute_alias args( (DESCRIPTOR_DATA *d, char *input) );

/* ban.c */
bool check_ban args( ( char *site, int type) );

/* clan.c */
void          set_claneq        args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
char        * get_rank          ( CHAR_DATA *ch );
bool          is_donation_gem   ( OBJ_DATA *obj );
void          sort_rank_number  ( CLAN_DATA *clan );
void          unguild_clannie   ( CHAR_DATA *ch, CLAN_DATA *clan );
ROSTER_DATA * find_clannie      ( CHAR_DATA *ch );
RANK_DATA   * find_rank         ( CLAN_DATA *clan );
RANK_DATA   * find_clannie_rank ( CHAR_DATA *ch );
void          add_rank          ( CLAN_DATA *clan, RANK_DATA *rank );

/* comm.c */
void show_string     args( ( struct descriptor_data *d, char *input) );
void close_socket    args( ( DESCRIPTOR_DATA *dclose ) );
void write_to_buffer args( ( DESCRIPTOR_DATA *d, const char *txt, int length ) );
void send_to_char    args( ( const char *txt, CHAR_DATA *ch ) );
void send_to_desc          ( const char *txt, DESCRIPTOR_DATA *d );
void page_to_char    args( ( const char *txt, CHAR_DATA *ch ) );
void act             args( ( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type ) );
void act_new         args( ( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, int min_pos) );
void act_spam        args( ( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, int min_pos, int flags ) );
void act_channels    args( ( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, int min_pos, const int channel) );
void act_channels_pmote args( ( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, int min_pos, const int channel) );
int  get_recall_room       ( CHAR_DATA *ch );
int  get_death_room        ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom );
int  format_act            ( char *msg, const char *format, CHAR_DATA *ch, CHAR_DATA *to, const void *arg1, const void *arg2 );

extern char as_string[MAX_NUM_AUTOSNOOP][70];
void display_race_table(CHAR_DATA *ch);
void display_class_table(CHAR_DATA *ch);

#if defined(unix)
bool write_to_descriptor    args( ( DESCRIPTOR_DATA *d, char *txt, int length ) );
bool write_to_descriptor_2    args( ( int desc, char *txt, int length ) );
#endif

/*
 * Colour stuff by Lope of Loping Through The MUD
 */
int  colour          args( ( char type, char *string ) );
void colourconv      args( ( char *buffer, const char *txt, CHAR_DATA *ch ) );
void send_to_char_bw args( ( const char *txt, CHAR_DATA *ch ) );
void page_to_char_bw args( ( const char *txt, CHAR_DATA *ch ) );
void printf_to_char  args( ( CHAR_DATA *ch, char *fmt, ...) );
void mprintf         args( ( int size, char *str, char *fmt, ...) );
void display_ansi_greeting(char *filename, DESCRIPTOR_DATA *d);
int  display_weapon_choices(CHAR_DATA *ch);

/* act_info.c */
char * get_align_str( int align );
bool   check_parse_name    args( ( char *name ) );

/* db.c */
void   reset_area       args( ( AREA_DATA * pArea ) );        /* OLC */
void   reset_room       args( ( ROOM_INDEX_DATA *pRoom ) );    /* OLC */
char * print_flags      args( ( int64 flag ) );
void   boot_db          args( ( void ) );
void   area_update      args( ( void ) );
CD   * create_mobile    args( ( MOB_INDEX_DATA *pMobIndex ) );
void   clone_mobile     args( ( CHAR_DATA *parent, CHAR_DATA *clone) );
OD   * create_object    args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void   clone_object     args( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
void   clear_char       args( ( CHAR_DATA *ch ) );
EXTRA_DESCR_DATA *ed_lookup args( ( char *name, EXTRA_DESCR_DATA *ed ) );
char * get_extra_descr  args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID  * get_mob_index    args( ( int vnum ) );
OID  * get_obj_index    args( ( int vnum ) );
RID  * get_room_index   args( ( int vnum ) );
MPC  * get_mprog_index  args( ( int vnum ) );
char   fread_letter     args( ( FILE *fp ) );
int64  fread_number     args( ( FILE *fp ) );
int64  fread_flag       args( ( FILE *fp ) );
char * fread_string     args( ( FILE *fp ) );
char * fread_string_eol args(( FILE *fp ) );
void   fread_to_eol     args( ( FILE *fp ) );
char * fread_word       args( ( FILE *fp ) );
int64  flag_convert     args( ( char letter) );
void * alloc_mem        args( ( int sMem ) );
void * alloc_perm       args( ( int sMem ) );
#if OLD_MEM
void   free_mem         args( ( void *pMem, int sMem ) );
#else
void   free_mem( void *pMem );
#endif
char * str_dup        args( ( const char *str,const char *checkstr ) );
char * str_chr( const char *s, const char c );
void   free_string    args( ( char *pstr ) );
int    number_fuzzy   args( ( int number ) );
int    number_range   args( ( int from, int to ) );
int    number_percent args( ( void ) );
int    number_door    args( ( void ) );
int    number_bits    args( ( int width ) );
long   number_mm      args( ( void ) );
int    dice           args( ( int number, int size ) );
int    interpolate    args( ( int level, int value_00, int value_32 ) );
int    interpolateNew args( ( int p, int p0, int p1, int v0, int v1 ) );
void   smash_tilde    args( ( char *str ) );
void   smash_percent  args( ( char *str ) );
void   smash_dollar   args( ( char *str ) );
bool   str_cmp        args( ( const char *astr, const char *bstr ) );
bool   str_prefix     args( ( const char *astr, const char *bstr ) );
bool   str_infix      args( ( const char *astr, const char *bstr ) );
bool   str_suffix     args( ( const char *astr, const char *bstr ) );
char * capitalize     args( ( const char *str ) );
char * new_capitalize args( ( const char *str ) );
void   append_file    args( ( CHAR_DATA *ch, char *file, char *str ) );
void   append_hint    args( ( CHAR_DATA *ch, char *str ) );
void   bug            args( ( const char *str, int param ) );
void   log_string     args( ( const char *str ) );
void   tail_chain     args( ( void ) );
char * capitalize( const char *str );
long   convert_level ( char * arg );
long   get_area_level(AREA_DATA *pArea);
bool   check_str_end( const char *str );
extern char interp_cmd[MSL];
struct cmd_hist_st { char command[MSL]; };
void    add_cmd_hist(char *cmd);
void    print_cmd_hist();
#define MAX_SAVE_CMDS 20
extern struct cmd_hist_st cmd_hist[MAX_SAVE_CMDS];
int        loggedf( const char *format, ... );

/* effect.c */
void acid_effect   args( (void *vo, int level, int dam, int target) );
void cold_effect   args( (void *vo, int level, int dam, int target) );
void fire_effect   args( (void *vo, int level, int dam, int target) );
void poison_effect args( (void *vo, int level, int dam, int target) );
void shock_effect   args( (void *vo, int level, int dam, int target) );

/* damage.c */
bool check_cheat(int *dam, int dt, CHAR_DATA *ch);
bool damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type, bool show, bool spell);

/* fight.c */
bool is_safe            args( (CHAR_DATA *ch, CHAR_DATA *victim, bool showflag ) );
bool is_safe_spell      args( (CHAR_DATA *ch, CHAR_DATA *victim, bool area, bool showflag ) );
void violence_update    args( ( void ) );
void multi_hit          args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void update_pos         args( ( CHAR_DATA *victim ) );
void stop_fighting      args( ( CHAR_DATA *ch, bool fBoth ) );
void check_killer       args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
bool is_hating          args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void stop_hating        args( ( CHAR_DATA *ch ) );
void stop_hating_ch     args( ( CHAR_DATA *ch ) );
void stop_fighting      args( ( CHAR_DATA *ch, bool fBoth ) );
void check_assist       args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_dodge        args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void check_killer       args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_parry        args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool showflag ) );
bool check_shield_block args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void dam_message        args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, bool immune ) );
bool check_sidestep     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool check_blade_weave  args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool showflag ) );
void death_cry          args( ( CHAR_DATA *ch ) );
void group_gain         args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int  xp_compute         args( ( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels ) );
bool is_safe            args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool showflag ) );
void make_corpse        args( ( CHAR_DATA *ch, CHAR_DATA *killer ) );
void one_hit            args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary ) );
void mob_hit            args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void raw_kill           args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void set_fighting       args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void disarm             args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void spirit_hit( CHAR_DATA *ch, CHAR_DATA *victim);
int  switch_update(CHAR_DATA *ch);
bool is_hating(CHAR_DATA *ch, CHAR_DATA *victim);
void stop_hating(CHAR_DATA *ch);
void start_hating(CHAR_DATA *ch, CHAR_DATA *victim);
bool check_safe(CHAR_DATA *ch, CHAR_DATA *victim, bool spell, bool area, bool showflag);
bool check_killsteal(CHAR_DATA *ch, CHAR_DATA *victim);
void set_plevel(CHAR_DATA *ch, CHAR_DATA *victim);
bool check_movement( CHAR_DATA *ch, CHAR_DATA *vch, int dt, bool silent );

/* games.c */
void do_tag( CHAR_DATA *ch, char *arguement );

/* handler.c */
AD   * affect_find          args( ( AFFECT_DATA *paf, int sn));
void   affect_check         args( ( CHAR_DATA *ch, int where, int vector) );
int    count_users          args( ( OBJ_DATA *obj) );
void   deduct_cost          args( ( CHAR_DATA *ch, int cost) );
void   new_deduct_cost      args( ( CHAR_DATA *ch, int cost, int *deducted_gold, int *deducted_silver ) );
void   affect_enchant       args( ( OBJ_DATA *obj) );
int    check_immune         args( ( CHAR_DATA *ch, int dam_type) );
int    liq_lookup           args( ( const char *name) );
int    material_lookup      args( ( const char *name) );
int    weapon_lookup        args( ( const char *name) );
int    weapon_type          args( ( const char *name) );
char * weapon_name          args( ( int weapon_Type) );
int    item_lookup          args( ( const char *name) );
char * item_name            args( ( int item_type) );
int    attack_lookup        args( ( const char *name) );
char * get_attack_name      args( ( int dam_msg ) );
char * get_attack_noun      args( ( int dam_msg ) );
int    race_lookup          args( ( const char *name) );
long   wiznet_lookup        args( ( const char *name) );
int    class_lookup         args( ( const char *name) );
bool   is_clan              args( ( CHAR_DATA *ch) );
bool   is_same_clan         args( ( CHAR_DATA *ch, CHAR_DATA *victim));
bool   is_old_mob           args( ( CHAR_DATA *ch) );
int    get_skill            args( ( CHAR_DATA *ch, int sn ) );
int    get_weapon_sn        args( ( CHAR_DATA *ch ) );
int    get_weapon_sn_2      args( ( CHAR_DATA *ch ) );
int    get_weapon_skill     args( ( CHAR_DATA *ch, int sn ) );
int    get_age              args( ( CHAR_DATA *ch ) );
void   reset_char           args( ( CHAR_DATA *ch )  );
int    get_trust            args( ( CHAR_DATA *ch ) );
int    get_curr_stat        args( ( CHAR_DATA *ch, int stat ) );
int    get_max_train        args( ( CHAR_DATA *ch, int stat ) );
int    get_stage1_max_train args( ( CHAR_DATA *ch, int stat ) );  // Fragment System Stage 1 Stats 26 max
int    get_stage2_max_train args( ( CHAR_DATA *ch, int stat ) );  // Stage 2 - 27 max
int    get_stage3_max_train args( ( CHAR_DATA *ch, int stat ) );  // Stage 3 - 28 max
int    can_carry_n          args( ( CHAR_DATA *ch ) );
int    can_carry_w          args( ( CHAR_DATA *ch ) );
bool   is_name              args( ( char *str, char *namelist ) );
bool   is_exact_name        args( ( char *str, char *namelist ) );
void   affect_to_char       args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void   affect_to_obj        args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void   affect_remove        args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void   remove_affect(CHAR_DATA *ch, int where, long affect);
void   affect_remove_obj    args( (OBJ_DATA *obj, AFFECT_DATA *paf ) );
void   affect_strip         args( ( CHAR_DATA *ch, int sn ) );
bool   is_affected          args( ( CHAR_DATA *ch, int sn ) );
void   affect_join          args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void   char_from_room       args( ( CHAR_DATA *ch ) );
void   char_to_room         args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void   move_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex );
void   obj_to_char          args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void   obj_from_char        args( ( OBJ_DATA *obj ) );
void   obj_to_locker        args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void   obj_from_locker      args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
int    apply_ac             args( ( OBJ_DATA *obj, int iWear, int type ) );
OD   * get_eq_char          args( ( CHAR_DATA *ch, int iWear ) );
void   equip_char           args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void   unequip_char         args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int    count_obj_list       args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void   obj_from_room        args( ( OBJ_DATA *obj ) );
void   obj_to_room          args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void   obj_to_obj           args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void   obj_from_obj         args( ( OBJ_DATA *obj ) );
void   remove_diamonds_char( CHAR_DATA *ch, CHAR_DATA *recipient, int amount);
void   remove_diamonds_container( OBJ_DATA *container, CHAR_DATA *recipient, int *amount);
int    get_diamond_container_number(OBJ_DATA *container);
void   obj_from_spellbook(OBJ_DATA *spellbook);
void   obj_to_spellbook( OBJ_DATA *obj, OBJ_DATA *spellbook );
void   extract_obj          args( ( OBJ_DATA *obj ) );
void   extract_char         args( ( CHAR_DATA *ch, bool fPull ) );
bool   parse_objhandling( CD *ch, char *objlist, void **source, int *from_flag, void **target, int *to_flag, char *argument );
void   change_money( OBJ_DATA *obj, int gold, int silver );
void   auto_toggle( CHAR_DATA *ch, char *argument, int64 *flag, long bitvector, const char *onstr, const char *offstr, char *samestr );
void   show_file_to_char(CHAR_DATA *ch, char *filename, char *argument);
void   align_change(CHAR_DATA *ch, int change, bool isgood, bool straight);
bool   check_obj_loop(OBJ_DATA *obj);
void   cleanup_restrings(CHAR_DATA *ch);
void   change_group_leader(CHAR_DATA *old_leader, CHAR_DATA *new_leader);

CD   * get_char_room          args( ( CHAR_DATA *ch, char *argument ) );
CD   * get_char_room_ordered  args( ( CHAR_DATA *ch, char *argument, char *sortfirst ) );
CD   * get_race_room          args( ( CHAR_DATA *ch, char *argument ) );
CD   * get_char_world         args( ( CHAR_DATA *ch, char *argument ) );
CD   * get_char_world_ordered args( ( CHAR_DATA *ch, char *argument, char *sortfirst ) );
OD   * get_obj_type           args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD   * get_obj_list           args( ( CHAR_DATA *ch, char *argument, OBJ_DATA *list ) );
OD   * get_obj_list_donation_pit  args( ( CHAR_DATA *ch, char *argument, OBJ_DATA *list ) );
OD   * get_obj_carry          args( ( CHAR_DATA *ch, char *argument, CHAR_DATA *viewer ) );
OD   * get_obj_wear           args( ( CHAR_DATA *ch, char *argument ) );
OD   * get_obj_here           args( ( CHAR_DATA *ch, char *argument ) );
OD   * get_obj_world          args( ( CHAR_DATA *ch, char *argument ) );
OD   * find_obj_vnum          args( ( int vnum ) );
OD   * create_money           args( ( int gold, int silver ) );
int    get_obj_number         args( ( OBJ_DATA *obj ) );
int    get_obj_weight         args( ( OBJ_DATA *obj ) );
int    get_true_weight        args( ( OBJ_DATA *obj ) );
bool   room_is_dark           args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool   is_room_owner          args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room) );
bool   room_is_private        args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool   can_see                args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool   can_see_obj            args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool   can_see_room           args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool   can_drop_obj           args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char * affect_loc_name        args( ( int location ) );
char * affect_bit_name        args( ( int64 vector ) );
char * affect2_bit_name       args( ( int64 vector ) );
char * spell_affect_bit_name  args( ( int64 vector ) );
char * extra_bit_name         args( ( int64 extra_flags ) );
char * wear_bit_name          args( ( int64 wear_flags) );
char * act_bit_name           args( ( int64 act_flags ) );
char * act2_bit_name          args( ( int64 act2_flags) );
char * off_bit_name           args( ( int64 off_flags ) );
char * imm_bit_name           args( ( int64 imm_flags ) );
char * form_bit_name          args( ( int64 form_flags) );
char * part_bit_name          args( ( int64 part_flags) );
char * weapon_bit_name        args( ( int64 weapon_flags ) );
char * comm_bit_name          args( ( int64 comm_flags) );
char * chan_bit_name          args( ( int64 chan_flags) );
char * pen_bit_name           args( ( int64 pen_flags ) );
char * cont_bit_name          args( ( int64 cont_flags) );
char * plr2_bit_name          args( ( int64 plr2_flags) );
int    roll_stat( CHAR_DATA *ch, int stat );
int    has_money_pouch(CHAR_DATA *ch);
int    get_carry_weight(CHAR_DATA *ch);
void   change_sex(CHAR_DATA *ch,int mod, bool fAdd);
char * strncpyft_color( char *s1, const char *s2, size_t n, char fill, bool terminate );
void   memdebug_check( CHAR_DATA *ch , char *calling_func);
char * strncpy_color args( ( char *s1, const char *s2, size_t n, char fill, bool terminate ) );

/* interp.c */
void   interpret       args( ( CHAR_DATA *ch, char *argument ) );
bool   is_number       args( ( char *arg ) );
int    number_argument args( ( char *argument, char *arg ) );
int    mult_argument   args( ( char *argument, char *arg) );
char * one_argument    args( ( char *argument, char *arg_first ) );
void   do_function     args( ( CHAR_DATA *ch, DO_FUN *do_fun, char *argument));
void   crash_fix();

/* magic.c */
int  find_spell         args( ( CHAR_DATA *ch, const char *name) );
int  skill_lookup       args( ( const char *name ) );
int  skill_lookup_exact args( ( const char *name ) );
int  skill_lookup_err   args( ( const char *name ) );
int  slot_lookup        args( ( int slot ) );
bool saves_spell        args( ( int level, CHAR_DATA *victim, int dam_type ) );
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj , int bypass);
bool magic_dispel(int level, CHAR_DATA *victim);
int  mana_cost(CHAR_DATA *ch, int min_mana, int level);

/* magic3.c */
bool is_sanc_spelled(CHAR_DATA *vch);
bool is_spirit_affected(CHAR_DATA *vch);

struct spell_power_table_st
{
  char *name;
  int  number;
  int  size;
};

int magic_dam(int sn,int level);
extern struct spell_power_table_st spell_power_table[];
void martyr_char(CHAR_DATA *ch, int sn);

/* mob_prog.c */
void program_flow       args( ( sh_int vnum, char *source, CHAR_DATA *mob, CHAR_DATA *ch, const void *arg1, const void *arg2 ) );
void mp_act_trigger     args( ( char *argument, CHAR_DATA *mob, CHAR_DATA *ch, const void *arg1, const void *arg2, int type ) );
bool mp_percent_trigger args( ( CHAR_DATA *mob, CHAR_DATA *ch, const void *arg1, const void *arg2, int type ) );
void mp_bribe_trigger   args( ( CHAR_DATA *mob, CHAR_DATA *ch, int amount ) );
bool mp_exit_trigger    args( ( CHAR_DATA *ch, int dir ) );
void mp_give_trigger    args( ( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj ) );
void mp_greet_trigger   args( ( CHAR_DATA *ch, char *argument ) );
void ap_gronce_trigger  args( ( CHAR_DATA *ch ) );
void mp_hprct_trigger   args( ( CHAR_DATA *mob, CHAR_DATA *ch ) );
/* Added by Merak for room/obj-progs 2006 - 10 - 01 */
bool ap_percent_trigger args( ( void *vo, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, int prog_type) );
void mp_touch_trigger   args( ( CHAR_DATA *ch, char *argument ) );
void mp_dig_trigger     args( ( CHAR_DATA *ch, char *argument ) );
void mp_press_trigger   args( ( CHAR_DATA *ch, char *argument ) );
void mp_turn_trigger    args( ( CHAR_DATA *ch, char *argument ) );
void mp_pull_trigger    args( ( CHAR_DATA *ch, char *argument ) );
void mp_get_trigger     args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool mp_get_check       args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void mp_social_trigger  args( ( CHAR_DATA *mob, CHAR_DATA *ch, char *soccmd ) );


/* mob_cmds.c */
void mob_interpret args( ( CHAR_DATA *ch, char *argument ) );

/* mystic.c */
char * martial_style_prompt args( ( CHAR_DATA *ch ) );

/* save.c */
void save_char_obj args( ( CHAR_DATA *ch, bool immoverride ) );
bool load_char_obj args( ( DESCRIPTOR_DATA *d, char *name ) );
void save_sys_data();
void load_sys_data();

/* skills.c */
int  TNL              args( ( int XPL, int level ) );
bool parse_gen_groups args( ( CHAR_DATA *ch,char *argument ) );
void list_group_costs args( ( CHAR_DATA *ch ) );
void list_group_known args( ( CHAR_DATA *ch ) );
int  exp_per_level    args( ( CHAR_DATA *ch, int points ) );
void check_improve    args( ( CHAR_DATA *ch, int sn, bool success, int multiplier ) );
int  group_lookup     args( (const char *name) );
void gn_add           args( ( CHAR_DATA *ch, int gn) );
void gn_remove        args( ( CHAR_DATA *ch, int gn) );
void group_add        args( ( CHAR_DATA *ch, const char *name, bool deduct) );
void group_remove     args( ( CHAR_DATA *ch, const char *name) );
char color_scale      args( (int pct, char *colors) );
bool is_racial_skill( CHAR_DATA * ch, const int racial_sn );

/* special.c */
SF   * spec_lookup args( ( const char *name ) );
char * spec_name   args( ( SPEC_FUN *function ) );

/* teleport.c */
RID * room_by_name args( ( char *target, int level, bool error) );

/* update.c */
void advance_level     args( ( CHAR_DATA *ch, bool hide ) );
void advance_claneq    args( ( CHAR_DATA *ch ) );
void gain_exp          args( ( CHAR_DATA *ch, int gain ) );
void gain_condition    args( ( CHAR_DATA *ch, int iCond, int value ) );
void update_handler    args( ( void ) );
void bank_update       args( ( void) );
void update_birthday(void);
void underwater_update args( ( void ) );

/* social-edit.c */
void load_social_table();
void save_social_table();

/* string.c */
char * numcat         args( ( char *buf, int num ) );
char * numcpy         args( ( char *buf, int num ) );
char * spacecat       args( ( char *buf ) );
bool   check_punct    args( ( char *str ) );
void   string_edit    args( ( CHAR_DATA *ch, char **pString ) );
void   string_append  args( ( CHAR_DATA *ch, char **pString, sh_int saving, void *vo ) );
char * string_replace args( ( char * orig, char * old, char * new_str ) );
void   string_add     args( ( CHAR_DATA *ch, char *argument ) );
char * format_string  args( ( char *oldstring /*, bool fSpace */ ) );
char * first_arg      args( ( char *argument, char *arg_first, bool fCase ) );
char * string_unpad   args( ( char * argument ) );
char * string_proper  args( ( char * argument ) );
int    colorstrlen(char *argument);
char * percent_bar( int current, int max, int length, char *fill, char *blank );
char * color_percent_bar( int current, int max, int length, char *fill, char *blank );
char * make_bar( char * c, int length );
char * center_align( char * string, int width );
char * format_ip( char *str );
char * to_upper( char *str );
char * show_stat_info( CHAR_DATA *ch, int current, int max, int length, char *fill, char *blank, bool mana_check );

/* olc.c */
bool   run_olc_editor args( ( DESCRIPTOR_DATA *d ) );
char * olc_ed_name    args( ( CHAR_DATA *ch ) );
char * olc_ed_vnum    args( ( CHAR_DATA *ch ) );
bool   check_vnum_range(CHAR_DATA *ch, int vnum);
bool   check_area_vnum(AREA_DATA *pArea, int vnum, CHAR_DATA *ch);

#undef CD
#undef MID
#undef OD
#undef OID
#undef RID
#undef SF
#undef AD

/*****************************************************************************
 *                                    OLC                                    *
 *****************************************************************************/

/*
 * Object defined in limbo.are
 * Used in save.c to load objects that don't exist.
 */
#define OBJ_VNUM_DUMMY 30

/*
 * Area flags.
 */
#define AREA_NONE    0
#define AREA_CHANGED 1    /* Area has been modified. */
#define AREA_ADDED   2    /* Area has been added to. */
#define AREA_LOADING 4    /* Used for counting in db.c */

#define MAX_DIR      6
#define NO_FLAG    -99    /* Must not be used in flags or stats. */

/*
 * Global Constants
 */
extern char * const  dir_name  [];
extern char * const  dir_abbr  [];
extern const  sh_int rev_dir   [];          /* sh_int - ROM OLC */
extern const  struct spec_type spec_table [];

/*
 * Global variables
 */
extern AREA_DATA * area_first;
extern AREA_DATA * area_first_sorted;
extern AREA_DATA * area_last;
extern SHOP_DATA * shop_last;
extern CLAN_DATA * clan_free;

extern int  top_affect;
extern int  top_area;
extern int  top_ed;
extern int  top_exit;
extern int  top_help;
extern int  top_mob_index;
extern int  top_obj_index;
extern int  top_reset;
extern int  top_room;
extern int  top_shop;
extern int  top_vnum_mob;
extern int  top_vnum_obj;
extern int  top_vnum_room;
extern char str_empty    [1];
extern MOB_INDEX_DATA  * mob_index_hash  [MAX_KEY_HASH];
extern OBJ_INDEX_DATA  * obj_index_hash  [MAX_KEY_HASH];
extern ROOM_INDEX_DATA * room_index_hash [MAX_KEY_HASH];

void show_obj_stats( CHAR_DATA *ch, OBJ_DATA *obj );

char * makedrunk  args( (char *string ,CHAR_DATA *ch) );
/*
 * Drunk struct
 */
struct struckdrunk
{
  int     min_drunk_level;
  int     number_of_rep;
  char    *replacement[11];
};

/* clan.c */
extern int  actual_num_clans;
extern int  actual_num_clans2;
CLAN_DATA * get_clan(char *arg, bool force_load);
void        save_clan( CLAN_DATA *clan );
int         strip_string(char *input_string);
int         str_normalize_white_space(char *input_string);
int         str_cleanup_extra_white_space(char *input_string);
int         get_next_line( FILE *fp, char *instr);
void        load_clan_info(char *clan_filename);
void        save_clan_info(char *clan_filename);
void        do_clanname(    CHAR_DATA *ch, char *argument );
void        do_clandesc(    CHAR_DATA *ch, char *argument );
void        do_clanhall(    CHAR_DATA *ch, char *argument );
void        do_clanstatus(  CHAR_DATA *ch, char *argument );
void        do_clantotal(   CHAR_DATA *ch, char *argument );
void        do_clanfree(    CHAR_DATA *ch, char *argument );
void        do_clanused(    CHAR_DATA *ch, char *argument );
void        do_clanlist(    CHAR_DATA *ch, char *argument );
void        do_clanpatron(  CHAR_DATA *ch, char *argument );
void        do_clan_sac(    CHAR_DATA *ch, char *argument );
void        do_addnewclan(  CHAR_DATA *ch, char *argument );
extern char player_table[MAX_PLAYERS][MSL];
extern char clan_player[MAX_PLAYERS][MSL];
void        do_saveclan(    CHAR_DATA *ch, char *argument );
void        save_clan_list(char *clan_roster_filename);
void        load_clan_list(char *clan_roster_filename);
void        do_reset_roster(CHAR_DATA *ch, char *argument);
ROSTER_DATA *find_clan_roster_member(CHAR_DATA *ch);
void        add_clannie( CLAN_DATA *clan, ROSTER_DATA *clannie );
void        clannie_roster_rename(CHAR_DATA *ch, char *arg);
void        do_donate_clan( CHAR_DATA *ch, OBJ_DATA *obj, int frominv, CLAN_DATA *clan);
void        do_donate_clan_all( CHAR_DATA *ch,char *arg);
void        copy_roster_clannie(CHAR_DATA *ch);
void        remove_clannie( CHAR_DATA *ch );
void        do_promote(     CHAR_DATA *ch, char *argument );
int         smash_clannie( CLAN_DATA *clan, ROSTER_DATA *clannie );
int         smash_rank( CLAN_DATA *clan, RANK_DATA *rank );
void        do_short_diamonds( CHAR_DATA *ch, char *argument);
void        do_clandeduct(  CHAR_DATA *ch, char *argument );
bool        is_valid_clan_name(char *str);
int         num_act_clan(int clan);
bool        is_valid_clan_status(char *str);
int         clan_status_lookup(char *str);
void        show_clan_status(CHAR_DATA *ch,CLAN_DATA *clan1, CLAN_DATA *clan2);
void        show_single_clan_status(CHAR_DATA *ch, CLAN_DATA *clan);
void        show_line_clan_status(CHAR_DATA *ch,  CLAN_DATA *clan1, CLAN_DATA *clan2);
char        *get_clan_status(CLAN_DATA *clan1, CLAN_DATA *clan2);
void        set_clan_status(CLAN_DATA *clan1, CLAN_DATA *clan2, int status);
extern int  clan_status[MAX_CLAN][MAX_CLAN];
void        load_clan_status(char *filename);
int         recall_pk_fight(CHAR_DATA *ch);
int         recall(CHAR_DATA *ch, bool cost_moves, int room);
bool        can_use_clan_obj( CHAR_DATA *ch, OBJ_DATA *obj);


/* clanrequest.c */
void clan_func_list(     CHAR_DATA *ch, char *argument );
void clan_func_items(    CHAR_DATA *ch, char *argument );
void clan_func_status(   CHAR_DATA *ch, char *argument );
void clan_func_purchase( CHAR_DATA *ch, char *argument );
void clan_func_increase( CHAR_DATA *ch, char *argument );
void clan_func_cancel(   CHAR_DATA *ch, char *argument );
void clan_func_request(  CHAR_DATA *ch, char *argument );
void read_clanreq();
void save_clanreq();
void update_clanreq();

/* spells.c file */
void save_spell_info(char *spell_filename);
int  find_spell_info_table_pos(char *comp_name);

/* rename.c */
int  rename_char( CHAR_DATA *ch, char *to_name );
void do_rename(   CHAR_DATA *ch, char *argument );

void   do_new_equipment( CHAR_DATA *ch, char *argument );
char * eq_worn( CHAR_DATA *ch, int iWear, CHAR_DATA *show_ch );
void   show_char_eq_to_char( CHAR_DATA *ch, CHAR_DATA *victim );

/* pc_skills.c */
bool check_counter   args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt ) );
void do_familiar (CHAR_DATA *ch, char *argument);
void do_deathgrip( CHAR_DATA *ch, char *argument );
bool check_critical  args( ( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *weapon ) ); /* Taeloch: modified to account for 2nd weapon */
bool guided_strike   args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void do_study( CHAR_DATA *ch, char *argument ); /* study by Absalom */
void do_tithe( CHAR_DATA *ch, char *argument );
void do_whirlwind( CHAR_DATA *ch, char *argument );
bool check_feint(CHAR_DATA *ch, CHAR_DATA *victim);

/* revered_vnums.c */
bool is_revered_mob(int vnum);
bool is_revered_obj(int vnum);
bool is_revered_room(int vnum);
void setup_revered();
/* disk_search.c */
bool mud_read_player_dir(char temp_table[MAX_PLAYERS][MSL], int *num_files);
bool get_specific_field_from_char_file(char *player, char *key_field, int *field, int state);
bool get_specific_field_from_char_file2(char *player, char *key_field, char *field);
bool mud_read_clan_players(int *num_clannies, int clan);

/* quest.c */
void quest_update        ( void );
void count_update        ( void );
void extract_quest_object( CHAR_DATA *ch );
void extract_quest_mobile( CHAR_DATA *ch );
void clear_quest         ( CHAR_DATA *ch, bool fPull );

struct flag_stat_type
{
    const struct flag_type *structure;
    bool stat;
    bool prefix;
};

extern const struct flag_stat_type flag_stat_table[];

struct chaos_name 
{
  char *who_name;
};

extern const struct chaos_name chaos_names[];
extern const struct chaos_name newbie_hints[];

bool   is_stat( const struct flag_type *flag_table );
int64    flag_value( const struct flag_type *flag_table, char *argument);
char * flag_string( const struct flag_type *flag_table, long bits );

/* track.c */
bool   quarry_in_room( CHAR_DATA *ch, int key, char *name );
sh_int find_path( CHAR_DATA *ch, int start, int limit, char *name );
void   real_track( CHAR_DATA *ch, char *name );

/* mccp.c */
bool compressStart   args( ( DESCRIPTOR_DATA *d, unsigned char telopt ) );
bool compressEnd(DESCRIPTOR_DATA *desc);
bool processCompressed(DESCRIPTOR_DATA *desc);
bool writeCompressed(DESCRIPTOR_DATA *desc, char *txt, int length);

/* auction.c */
void auction_update  args( ( void ) );
void auction_channel args( ( char *msg ) );
struct thread_pass_st {
  CHAR_DATA *ch;
  char * args;
  
};

/* wizstat.c */
void show_ostat(CHAR_DATA *ch, OBJ_DATA *obj);

/* ispell.c */
void   ispell_init();
void   ispell_done();
char * get_ispell_line(char *word);
void   ispell_string(CHAR_DATA * ch);


/* magic.c */
void cast_a_spell(int sn, int lvl, CHAR_DATA *ch, void *vo, int target, int object);


void note_line(char *sender, int note_type, char *to, char *subject, char *text);
bool check_pet_affected(int vnum, AFFECT_DATA *paf);
int count_obj_loss();
int count_mob_loss();

//olc_act.c
char      * area_bit_name(int64 area_flags);
AREA_DATA * get_area_world(char *argument );

/* websvr.c */
void init_web(int port);
void handle_web(void);
void shutdown_web(void);
