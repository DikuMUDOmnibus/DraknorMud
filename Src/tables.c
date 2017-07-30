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
*  ROM 2.4 is copyright 1993-1996 Russ Taylor         *
*  ROM has been brought to you by the ROM consortium       *
*      Russ Taylor (rtaylor@efn.org)           *
*      Gabrielle Taylor               *
*      Brian Moore (zump@rom.org)             *
*  By using this code, you have agreed to follow the terms of the     *
*  ROM license, in the file Rom24/doc/rom.license         *
***************************************************************************/

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
#include <time.h>
#include "merc.h"
#include "tables.h"

/* for position */
const struct position_type position_table[] =
{
    {  "dead",          "dead"  },
    {  "mortally wounded",  "mort"  },
    {  "incapacitated",  "incap"  },
    {  "stunned",        "stun"  },
    {  "sleeping",        "sleep"  },
    {  "resting",        "rest"  },
    {   "sitting",        "sit"   },
    {  "fighting",        "fight"  },
    {  "standing",        "stand"  },
    {   NULL,           NULL  }
};

/* for sex */
const struct sex_type sex_table[] =
{
   {  "none"    },
   {  "male"    },
   {  "female"  },
   {  "either"  },
   {   NULL    }
};

/* for sizes */
const struct size_type size_table[] =
{ 
    {  "tiny"    },
    {  "small"   },
    {  "medium"  },
    {  "large"    },
    {  "huge"     },
    {  "giant"   },
    {  NULL    }
};

const struct where_pos where_table[] =
{
    {  0 },  //    "<used as light>     ",
    {  6 },  //    "<worn on head>      ",
    { 25 },  //    "<worn on left eye>  ",
    { 23 },  //    "<worn on left ear>  ",
    { 24 },  //    "<worn on right ear> ",
    {  3 },  //    "<worn around neck>  ",
    {  4 },  //    "<worn around neck>  ",
    { 21 },  //    "<worn on back>      ",
    {  5 },  //    "<worn on body>      ",
    { 28 },  //    "<worn as crest>     ",
    { 12 },  //    "<worn about torso>  ",
    { 22 },  //    "<worn on lapel>     ",
    { 10 },  //    "<worn on arms>      ",
    { 14 },  //    "<worn around wrist> ",
    { 15 },  //    "<worn around wrist> ",
    {  9 },  //    "<worn on hands>     ",
    {  1 },  //    "<worn on finger>    ",
    {  2 },  //    "<worn on finger>    ",
    { 13 },  //    "<worn about waist>  ",
    { 20 },  //    "<hooked to belt>    "
    {  7 },  //    "<worn on legs>      ",
    { 26 },  //    "<worn as right foot>",
    { 27 },  //    "<worn on left foot> ",
    {  8 },  //    "<worn on feet>      ",
    { 18 },  //    "<floating nearby>   ",
    { 17 },  //    "<held>              ",
    { 11 },  //    "<worn as shield>    ",
    { 16 },  //    "<wielded>           ",
    { 19 },  //    "<secondary weapon>  ".
};



/* various flag tables */
const struct flag_type act_flags[] =
{
  {  "npc",          ACT_IS_NPC,          FALSE  },
  {  "sentinel",        ACT_SENTINEL,      TRUE  },
  {  "scavenger",    ACT_SCAVENGER,    TRUE  },
  {  "banker",        ACT_BANKER,          TRUE  },
  { "noghost",      ACT_NOGHOST,      TRUE  },
  {  "aggressive",    ACT_AGGRESSIVE,      TRUE  },
  {  "stay_area",    ACT_STAY_AREA,    TRUE  },
  {  "wimpy",        ACT_WIMPY,          TRUE  },
  {  "pet",          ACT_PET,          TRUE  },
  {  "train",        ACT_TRAIN,          TRUE  },
  {  "practice",        ACT_PRACTICE,      TRUE  },
  {  "dealer",       ACT_DEALER,          TRUE  },
  {  "locker",        ACT_LOCKER,       TRUE  },
  {  "undead",        ACT_UNDEAD,          TRUE  },
  {  "cleric",       ACT_CLERIC,        TRUE  },
  {  "mage",       ACT_MAGE,         TRUE  },
  {  "thief",        ACT_THIEF,        TRUE  },
  {  "warrior",        ACT_WARRIOR,      TRUE  },
  {  "noalign",        ACT_NOALIGN,         TRUE  },
  {  "nopurge",      ACT_NOPURGE,      TRUE  },
  {  "outdoors",        ACT_OUTDOORS,       TRUE  },
  {  "indoors",        ACT_INDOORS,      TRUE  },
  {  "healer",        ACT_IS_HEALER,    TRUE  },
  {  "gain",          ACT_GAIN,          TRUE  },
  {  "update_always",  ACT_UPDATE_ALWAYS,  TRUE  },
  {  "changer",        ACT_IS_CHANGER,      TRUE  },
  { "forger",           ACT_FORGER,         TRUE    },
  { "bailer",           ACT_IS_BAILER,      TRUE    },
  { "noquest",      ACT_NOQUEST,      TRUE  },
  { "recalls",          ACT_RECALLS,        TRUE    },
  {  NULL,          0,                  FALSE  }
};

//Act2 flags added in October, 2003 - Robert Leonard
const struct flag_type act2_flags[] =
{
  { "ruby_fragment",        ACT2_RFRAG,      TRUE    },
  { "sapphire_fragment",    ACT2_SFRAG,      TRUE    },
  { "emerald_fragment",     ACT2_EFRAG,      TRUE    },
  { "diamond_fragment",     ACT2_DFRAG,      TRUE    },
  { "any_fragment",         ACT2_AFRAG,      TRUE    },
  { "frag_seller",          ACT2_FRAG_SELL,  TRUE    },
  { "identifier",           ACT2_IDENTIFIER, TRUE    },
  { "switcher",             ACT2_SWITCHER,   TRUE    },
  { "nohaggle",             ACT2_NOHAGGLE,   TRUE    },
  { "mountable",            ACT2_MOUNTABLE,  TRUE    },
  { "no_wander_off",        ACT2_NOWANDEROFF,TRUE    },
  { "bloodless",            ACT2_BLOODLESS,  TRUE    },
  { NULL,                   0,               FALSE   }

};

const struct flag_type plr_flags[] =
{
  {  "npc",      PLR_IS_NPC,      FALSE  },
  {  "autoassist",  PLR_AUTOASSIST,  FALSE  },
  {  "autoexit",    PLR_AUTOEXIT,  FALSE  },
  {  "autoloot",    PLR_AUTOLOOT,  FALSE  },
  {  "autosac",    PLR_AUTOSAC,  FALSE  },
  {  "autogold",    PLR_AUTOGOLD,  FALSE  },
  {  "autosplit",  PLR_AUTOSPLIT,  FALSE  },
  {  "nocancel",    PLR_NOCANCEL,  FALSE  },
  {  "nodigmove",  PLR_NODIGMOVE,  FALSE  },
  {  "holylight",  PLR_HOLYLIGHT,  FALSE  },
  {  "can_loot",    PLR_CANLOOT,  FALSE  },
  {  "nosummon",    PLR_NOSUMMON,  FALSE  },
  {  "nofollow",    PLR_NOFOLLOW,  FALSE  },
  {  "colour",    PLR_COLOUR,      FALSE  },
  {  "permit",    PLR_PERMIT,      TRUE  },
  {  "log",      PLR_LOG,      FALSE  },
  {  "deny",      PLR_DENY,      FALSE  },
  {  "freeze",    PLR_FREEZE,      FALSE  },
  {  "thief",    PLR_THIEF,    FALSE  },
  {  "killer",    PLR_KILLER,      FALSE  },
  {  "leader",    PLR_LEADER,      FALSE  },
  {  "questing",    PLR_QUESTING,  FALSE  },
  {  "twit",        PLR_TWIT,      FALSE  },
  {  "violent",    PLR_VIOLENT,  FALSE  },
  { "assistant",    PLR_ASSISTANT,  FALSE   },
//  { "autohunt",   PLR_AUTOHUNT, FALSE },
  {  "autodonate",  PLR_AUTODONATE,  FALSE  },
  {  "fishing",    PLR_FISHING,  FALSE  },
  {  "nogate",    PLR_NOGATE,  FALSE  },
  {  NULL,      0,  0  }
};

const struct flag_type ban_flags[] =
{
    {  "suffix",    BAN_SUFFIX,     FALSE  },
    {  "prefix",    BAN_PREFIX,        FALSE  },
    {  "duration",    BAN_DURATION,    FALSE  },
    {  "permanent",  BAN_PERMANENT,      FALSE  },
    {  "all",      BAN_ALL,        TRUE  },
    {  "autolog",    BAN_AUTOLOG,    TRUE  },
    {  "channels",    BAN_NOCHAN,        TRUE  },
    {  "newbies",    BAN_NEWBIES,    TRUE  },
    {  "notes",    BAN_NONOTE,        TRUE  },
    {  "permit",    BAN_PERMIT,        TRUE  },
    {  NULL,      0,      0  }
};
const struct flag_type affect_flags[] =
{
  {  "blind",          AFF_BLIND,                TRUE  },
  {  "invisible",      AFF_INVISIBLE,            TRUE  },
  {  "detect_evil",    AFF_DETECT_EVIL,          TRUE  },
  {  "detect_invis",    AFF_DETECT_INVIS,         TRUE  },
  {  "detect_magic",    AFF_DETECT_MAGIC,          TRUE  },
  {  "detect_hidden",  AFF_DETECT_HIDDEN,        TRUE  },
  {  "detect_good",    AFF_DETECT_GOOD,          TRUE  },
  {  "sanctuary",      AFF_SANCTUARY,            TRUE  },
  {  "faerie_fire",    AFF_FAERIE_FIRE,          TRUE  },
  {  "infrared",        AFF_INFRARED,             TRUE  },
  {  "curse",          AFF_CURSE,                TRUE  },
  {  "poison",          AFF_POISON,               TRUE  },
  {  "protect_evil",    AFF_PROTECT_EVIL,          TRUE  },
  {  "protect_good",    AFF_PROTECT_GOOD,          TRUE  },
  {  "sneak",          AFF_SNEAK,                TRUE  },
  {  "hide",            AFF_HIDE,                 TRUE  },
  {  "sleep",          AFF_SLEEP,                TRUE  },
  {  "charm",          AFF_CHARM,                TRUE  },
  {  "flying",          AFF_FLYING,               TRUE  },
  {  "pass_door",      AFF_PASS_DOOR,            TRUE  },
  {  "haste",          AFF_HASTE,                TRUE  },
  {  "calm",            AFF_CALM,                 TRUE  },
  {  "plague",          AFF_PLAGUE,               TRUE  },
  {  "weaken",          AFF_WEAKEN,                TRUE  },
  {  "dark_vision",    AFF_DARK_VISION,          TRUE  },
  {  "berserk",        AFF_BERSERK,              TRUE  },
  {  "swim",            AFF_SWIM,                 TRUE  },
  {  "regeneration",    AFF_REGENERATION,          TRUE  },
  {  "slow",            AFF_SLOW,                  TRUE  },
  {  "linkdeath",      AFF_LINKDEATH,            TRUE  },
  {  "aqua_albedo",    AFF_AQUA_ALBEDO,          TRUE  },
  {  "aqua_regia",      AFF_AQUA_REGIA,            TRUE  },
  {  "alacrity",        AFF_ALACRITY,              TRUE  },
  //{ "nirvana",          AFF_NIRVANA,            TRUE    },
  {  NULL, 0, 0 }
};

const struct flag_type affect2_flags[] =
{
  {     "nirvana",              AFF2_NIRVANA,           TRUE    },
  {     "fadeout",              AFF2_FADE_OUT,          TRUE    },
  {     "shroud",               AFF2_SHROUD,            TRUE    },
  {     "radiance",             AFF2_RADIANT,           TRUE    },
  {     "protect_neutral",      AFF2_PROTECT_NEUTRAL,   TRUE    },
  {     "globe",                AFF2_INVUN,             TRUE    },
  {     "warcry_hardening",     AFF2_WARCRY_HARDENING,  TRUE    },
  {     "warcry_rage",          AFF2_WARCRY_RAGE,       TRUE    },
  {     "warcry_vigor",         AFF2_WARCRY_VIGOR,      TRUE    },
  {     "binded",               AFF2_BINDED,            TRUE    },
  {     NULL,                   0,                      0       }
};

const struct flag_type spell_affect_flags[] =
{
  {  "martyr",            SAFF_MARTYR,          TRUE  },
  {  "walkwater",        SAFF_WALK_ON_WATER,      TRUE  },
  {  "deter",            SAFF_DETER,             TRUE  },
  {  "globe",            SAFF_INVUN,             TRUE  },
  {  "farsight",            SAFF_FARSIGHT,          TRUE  },
  {  "mageshield",          SAFF_MAGESHIELD,      TRUE  },
  {  "replenish",        SAFF_REPLENISH,          TRUE  },
  {  "warriorshield",      SAFF_WARRIORSHIELD,     TRUE  },
  {  "yawn",                  SAFF_YAWN,              TRUE  },
  {  "hiccup",              SAFF_HICCUP,            TRUE  },
  {  "ironwill",              SAFF_IRONWILL,          TRUE  },
  {  "protect neutral",      SAFF_PROTECT_NEUTRAL,   TRUE  },
  {  "adrenalize",          SAFF_ADRENALIZE,        TRUE  },
  {  "manadrain",        SAFF_MANA_DRAIN,      TRUE  },
  {  "lifedrain",        SAFF_LIFE_DRAIN,      TRUE  },
  { "flame_shroud",         SAFF_FLAME_SHROUD,      TRUE    },
  { "ice_shroud",           SAFF_ICE_SHROUD,        TRUE    },
  { "electric_shroud",      SAFF_ELECTRIC_SHROUD,   TRUE    },
  { "poison_shroud",        SAFF_POISON_SHROUD,     TRUE    },
  { "warcry_guarding",      SAFF_WARCRY_GUARDING,   TRUE    },
  {  NULL,      0,  0  }
};

const struct flag_type dummy_flags[] =
{
  { NULL,       0,      0       }
};

const struct flag_type off_flags[] =
{
  {  "area_attack",    OFF_AREA_ATTACK,TRUE  },
  {  "backstab",        OFF_BACKSTAB,  TRUE  },
  {  "bash",          OFF_BASH,      TRUE    },
  {  "berserk",        OFF_BERSERK,  TRUE  },
  {  "disarm",        OFF_DISARM,      TRUE     },
  {  "dodge",        OFF_DODGE,    TRUE    },
  {  "fade",          OFF_FADE,      TRUE     },
  {  "fast",       OFF_FAST,     TRUE    },
  {  "kick",          OFF_KICK,      TRUE     },
  {  "dirt_kick",    OFF_KICK_DIRT,  TRUE  },
  {  "parry",          OFF_PARRY,    TRUE    },
  {  "rescue",       OFF_RESCUE,      TRUE    },
  {  "tail",          OFF_TAIL,     TRUE  },
  {  "trip",          OFF_TRIP,      TRUE  },
  {  "crush",        OFF_CRUSH,    TRUE  },
  {  "assist_all",    ASSIST_ALL,      TRUE  },
  {  "assist_align",    ASSIST_ALIGN,  TRUE  },
  {  "assist_race",    ASSIST_RACE,  TRUE  },
  {  "assist_players",  ASSIST_PLAYERS,  TRUE  },
  {  "assist_guard",    ASSIST_GUARD,  TRUE  },
  {  "assist_vnum",    ASSIST_VNUM,  TRUE  },
  {  "flying_kick",    OFF_FLYING,   TRUE  },
  {  "bite",          OFF_BITE,      TRUE  },
//New Offenses added
  { "assist_mobile",    ASSIST_MOBILE,  TRUE    }, //Assist any fighting mobile
  { "assist_area",      ASSIST_AREA,    TRUE    }, //Assist mobs from area.
  {  NULL,      0,  0  }
};

const struct flag_type imm_flags[] =
{
  {  "summon",    IMM_SUMMON,  TRUE  },
  {  "charm",    IMM_CHARM,  TRUE  },
  {  "magic",          IMM_MAGIC,  TRUE  },
  {  "weapon",    IMM_WEAPON,  TRUE  },
  {  "bash",      IMM_BASH,  TRUE  },
  {  "pierce",    IMM_PIERCE,  TRUE  },
  {  "slash",    IMM_SLASH,  TRUE  },
  {  "fire",      IMM_FIRE,  TRUE  },
  {  "cold",      IMM_COLD,  TRUE  },
  {  "lightning",    IMM_LIGHTNING,  TRUE  },
  {  "acid",      IMM_ACID,  TRUE  },
  {  "poison",    IMM_POISON,  TRUE  },
  {  "negative",    IMM_NEGATIVE,  TRUE  },
  {  "holy",      IMM_HOLY,  TRUE  },
  {  "energy",    IMM_ENERGY,  TRUE  },
  {  "mental",    IMM_MENTAL,  TRUE  },
  {  "disease",    IMM_DISEASE,  TRUE  },
  {  "drowning",    IMM_DROWNING,  TRUE  },
  {  "light",    IMM_LIGHT,  TRUE  },
  {  "sound",    IMM_SOUND,  TRUE  },
  {  "wood",      IMM_WOOD,  TRUE  },
  {  "silver",    IMM_SILVER,  TRUE  },
  {  "iron",      IMM_IRON,  TRUE  },
  {  "wind",           IMM_WIND,  TRUE  },
  {  "shock",         IMM_SHOCK,  TRUE  },
  {  NULL,      0,  0  }
};

const struct flag_type form_flags[] =
{
  {  "edible",       FORM_EDIBLE,    TRUE  },
  {  "poison",        FORM_POISON,    TRUE  },
  {  "magical",        FORM_MAGICAL,    TRUE  },
  {  "instant_decay",  FORM_INSTANT_DECAY,  TRUE  },
  {  "other",        FORM_OTHER,        TRUE  },
  {  "animal",        FORM_ANIMAL,    TRUE  },
  {  "sentient",        FORM_SENTIENT,    TRUE  },
  {  "undead",       FORM_UNDEAD,    TRUE  },
  {  "construct",    FORM_CONSTRUCT,    TRUE  },
  {  "mist",          FORM_MIST,        TRUE  },
  {  "intangible",    FORM_INTANGIBLE,  TRUE  },
  {  "biped",        FORM_BIPED,        TRUE  },
  {  "centaur",        FORM_CENTAUR,    TRUE  },
  {  "insect",       FORM_INSECT,    TRUE  },
  {  "spider",        FORM_SPIDER,    TRUE  },
  {  "crustacean",    FORM_CRUSTACEAN,  TRUE  },
  {  "worm",          FORM_WORM,        TRUE  },
  {  "blob",          FORM_BLOB,        TRUE  },
  {  "mammal",        FORM_MAMMAL,    TRUE  },
  {  "bird",          FORM_BIRD,        TRUE  },
  {  "reptile",        FORM_REPTILE,    TRUE  },
  {  "snake",        FORM_SNAKE,        TRUE  },
  {  "dragon",        FORM_DRAGON,    TRUE  },
  {  "amphibian",    FORM_AMPHIBIAN,    TRUE  },
  {  "fish",          FORM_FISH,      TRUE  },
  {  "cold_blood",    FORM_COLD_BLOOD,  TRUE  },
  {  NULL,      0,      0  }
};

const struct flag_type part_flags[] =
{
  {  "head",        PART_HEAD,        TRUE  },
  {  "arms",        PART_ARMS,        TRUE  },
  {  "legs",        PART_LEGS,        TRUE  },
  {  "heart",      PART_HEART,       TRUE  },
  {  "brains",      PART_BRAINS,      TRUE  },
  {  "guts",        PART_GUTS,        TRUE  },
  {  "hands",      PART_HANDS,        TRUE  },
  {  "feet",        PART_FEET,        TRUE  },
  {  "fingers",    PART_FINGERS,      TRUE  },
  {  "ear",        PART_EAR,         TRUE  },
  {  "eye",        PART_EYE,          TRUE  },
  {  "long_tongue",PART_LONG_TONGUE,  TRUE  },
  {  "eyestalks",  PART_EYESTALKS,    TRUE  },
  {  "tentacles",  PART_TENTACLES,    TRUE  },
  {  "fins",        PART_FINS,        TRUE  },
  {  "wings",      PART_WINGS,        TRUE  },
  {  "tail",        PART_TAIL,        TRUE  },
  {  "claws",      PART_CLAWS,       TRUE  },
  {  "fangs",      PART_FANGS,        TRUE  },
  {  "horns",      PART_HORNS,        TRUE  },
  {  "scales",      PART_SCALES,      TRUE  },
  {  "tusks",      PART_TUSKS,       TRUE  },
  { "hoof",       PART_HOOF,        TRUE  },
  { "talons",     PART_TALONS,      TRUE  },
  { "bud",        PART_BUD,         TRUE  },
  { "petal",      PART_PETAL,       TRUE  },
  { "leaf",       PART_LEAF,        TRUE  },
  { "flower",     PART_FLOWER,      TRUE  },
  { "root",       PART_ROOT,        TRUE  },
  {   NULL,        0,                0     }
};

const struct flag_type comm_flags[] =
{
  {   "compact",    COMM_COMPACT,    TRUE  },
  {   "brief",    COMM_BRIEF,    TRUE  },
  {   "prompt",    COMM_PROMPT,    TRUE  },
  {   "combine",    COMM_COMBINE,    TRUE  },
  {   "show_affects",    COMM_SHOW_AFFECTS,  TRUE  },
  {   "afk",      COMM_AFK,    TRUE  },
  {   "rlock",    COMM_REPLY_LOCK,  TRUE  },
  {   "stats_show",         COMM_STATS_SHOW,   TRUE  },
  {  NULL,      0,      0  }
};
const struct flag_type chan_flags[] =
{
  {  "quiet",    CHANNEL_QUIET,    TRUE  },
  {   "deaf",      CHANNEL_DEAF,    TRUE  },
  {   "nowiz",    CHANNEL_NOWIZ,    TRUE  },
  {   "noclangossip",    CHANNEL_NOAUCTION,    TRUE  },
  {   "nogossip",    CHANNEL_NOGOSSIP,    TRUE  },
  {   "noquestion",    CHANNEL_NOQUESTION,  TRUE  },
  {   "nomusic",    CHANNEL_NOMUSIC,    TRUE  },
  {   "noclan",    CHANNEL_NOCLAN,    TRUE  },
  {   "noquote",    CHANNEL_NOQUOTE,    TRUE  },
  {   "shoutsoff",    CHANNEL_SHOUTSOFF,    TRUE  },
  {   "nograts",    CHANNEL_NOGRATS,    TRUE  },
  {   "noemote",    CHANNEL_NOEMOTE,    FALSE  },
  {   "noshout",    CHANNEL_NOSHOUT,    FALSE  },
  {   "notell",    CHANNEL_NOTELL,    FALSE  },
  {   "noooc",    CHANNEL_NOOOC,    TRUE  },
  {   "nowar",    CHANNEL_NOWAR,    TRUE  },
  {   "noinfo",    CHANNEL_NOINFO,    TRUE  },
  {   "nopolitic",    CHANNEL_NOPOLITIC,    TRUE  },
  {   "all",    CHANNEL_ALL,  TRUE  },
  {   "nonewbie",    CHANNEL_NEWBIE,  TRUE  },
  {  NULL,      0,      0  }
};
const struct flag_type pen_flags[] =
{
  {   "nochannels",    PEN_NOCHANNELS,  FALSE  },
  {   "snoop_proof",    PEN_SNOOP_PROOF,  FALSE  },
  {   "notedeny",    PEN_NOTE,  TRUE  },
  {  NULL,      0,      0  }
};
const struct flag_type plr2_flags[] =
{
  {  "stats",    PLR2_STATS,    TRUE  },
  /*{  "noquest",    PLR2_NOQUEST,    TRUE  },*/
  {    "telnet_ga",    PLR2_TELNET_GA,    TRUE  },
  {    "no_fragxp",    PLR2_NO_FRAGXP,    TRUE  },
  {  NULL,      0,      0  }
};

const struct flag_type mprog_flags[] =
{
  { "act",    TRIG_ACT,    TRUE },
  { "bribe",  TRIG_BRIBE,  TRUE },
  { "death",  TRIG_DEATH,  TRUE },
  { "entry",  TRIG_ENTRY,  TRUE },
  { "fight",  TRIG_FIGHT,  TRUE },
  { "give",   TRIG_GIVE,   TRUE },
  { "greet",  TRIG_GREET,  TRUE },
  { "grall",  TRIG_GRALL,  TRUE },
  { "gronce", TRIG_GRONCE, TRUE },
  { "kill",   TRIG_KILL,   TRUE },
  { "hpcnt",  TRIG_HPCNT,  TRUE },
  { "random", TRIG_RANDOM, TRUE },
  { "speech", TRIG_SPEECH, TRUE },
  { "exit",   TRIG_EXIT,   TRUE },
  { "exall",  TRIG_EXALL,  TRUE },
  { "delay",  TRIG_DELAY,  TRUE },
  { "surr",   TRIG_SURR,   TRUE },
  { "touch",  TRIG_TOUCH,  TRUE },
  { "wear",   TRIG_WEAR,   TRUE },
  { "remove", TRIG_REMOVE, TRUE },
  { "dig",    TRIG_DIG,    TRUE },
  { "pull",   TRIG_PULL,   TRUE },
  { "press",  TRIG_PRESS,  TRUE },
  { "turn",   TRIG_TURN,   TRUE },
  { "get",    TRIG_GET,    TRUE },
  { "social", TRIG_SOCIAL, TRUE },
  { "onload", TRIG_ONLOAD, TRUE },
  { NULL,     0,           TRUE }
};

const struct flag_type area_flags[] =
{
  {  "none",      AREA_NONE,    FALSE  },
  {  "changed",  AREA_CHANGED,  TRUE  },
  {  "added",    AREA_ADDED,    TRUE  },
  {  "loading",  AREA_LOADING,  FALSE  },
  {  NULL,      0,      0  }
};

const struct flag_type sys_area_flags[] =
{
  { "noquest",  AREA_NO_QUEST,  FALSE  },
  { "draft",    AREA_DRAFT,     TRUE  },
  { "crystal",  AREA_CRYSTAL,   FALSE },
  { "nogate",   AREA_NOGATE,    TRUE  },
  { "nosummon", AREA_NOSUMMON,  TRUE  },
  { "norescue", AREA_NORESCUE,  TRUE  },
  { "library",  AREA_LIBRARY,   TRUE  },
  { "clan_hall",AREA_CLANHALL,  TRUE  },
  {  NULL,      0,      0  }
};


const struct flag_type sex_flags[] =
{
  { "male",      SEX_MALE,      TRUE  },
  { "female",    SEX_FEMALE,    TRUE  },
  { "neutral",   SEX_NEUTRAL,   TRUE  },
  { "random",    3,             TRUE  },   /* ROM */
  { "none",      SEX_NEUTRAL,   TRUE  },
  { NULL,        0,             0     }
};



const struct flag_type exit_flags[] =
{
  {   "door",        EX_ISDOOR,    TRUE    },
  {   "closed",        EX_CLOSED,    TRUE  },
  {   "locked",        EX_LOCKED,    TRUE  },
  {   "multi",          EX_MULTI,       TRUE    },
  {   "pickproof",      EX_PICKPROOF,  TRUE  },
  {   "nopass",     EX_NOPASS,    TRUE  },
  {   "easy",        EX_EASY,    TRUE  },
  {   "hard",        EX_HARD,    TRUE  },
  {   "infuriating",  EX_INFURIATING,  TRUE  },
  {   "noclose",      EX_NOCLOSE,    TRUE  },
  {   "nolock",     EX_NOLOCK,    TRUE  },
  {   "hidden",        EX_HIDDEN,    TRUE  },
  {   "fenced",         EX_FENCED,      TRUE    },
  {   "noexit",         EX_NOEXIT,      TRUE    },
  {   "seethrough",     EX_SEETHROUGH,  TRUE    },
  {   NULL,            0,        0      }
};

const struct flag_type spool_flags[] =
{
    {  "note",      NOTE_NOTE,    TRUE  },
    {  "idea",      NOTE_IDEA,    TRUE  },
    {  "penalty",    NOTE_PENALTY,  TRUE  },
    {  "news",      NOTE_NEWS,    TRUE  },
    {  "change",    NOTE_CHANGES,  TRUE  },
    {  "rules",    NOTE_RULES,    TRUE  },
    {  "rpnote",    NOTE_RPNOTE,  TRUE  },
    {  NULL,      0,          0     }
};



const struct flag_type door_resets[] =
{
  {  "open and unlocked",  0,    TRUE  },
  {  "closed and unlocked",  1,    TRUE  },
  {  "closed and locked",  2,    TRUE  },
  {  NULL,      0,    0  }
};



const struct flag_type room_flags[] =
{
  { "dark",      ROOM_DARK,      TRUE  },
  { "no_mob",    ROOM_NO_MOB,    TRUE  },
  { "indoors",    ROOM_INDOORS,    TRUE  },
  { "ship",         ROOM_SHIP,          TRUE    },
  { "fishing",      ROOM_FISHING,       TRUE    },
  { "private",    ROOM_PRIVATE,    TRUE    },
  { "safe",      ROOM_SAFE,        TRUE  },
  { "solitary",    ROOM_SOLITARY,    TRUE  },
  { "pet_shop",    ROOM_PET_SHOP,    TRUE  },
  { "no_recall",  ROOM_NO_RECALL,    TRUE  },
  { "imp_only",    ROOM_IMP_ONLY,    TRUE    },
  { "gods_only",    ROOM_GODS_ONLY,    TRUE    },
  { "heroes_only",  ROOM_HEROES_ONLY,  TRUE  },
  { "newbies_only",  ROOM_NEWBIES_ONLY,  TRUE  },
  { "law",      ROOM_LAW,        TRUE  },
  { "nowhere",    ROOM_NOWHERE,    TRUE  },
  { "nomagic",    ROOM_NOMAGIC,    TRUE  },
  { "arena",    ROOM_ARENA,        TRUE  },
  { "no_weather",  ROOM_NOWEATHER,      TRUE  },
  { "underwater",  ROOM_UNDER_WATER,  TRUE  },
  { "noghost",    ROOM_NO_GHOST,     TRUE  },
  { "no_teleport",  ROOM_NO_TELEPORT,  TRUE  },
  { NULL,      0,              0     }
};



const struct flag_type sector_flags[] =
{
  {  "inside",    SECT_INSIDE,    TRUE  },
  {  "city",      SECT_CITY,      TRUE  },
  {  "field",    SECT_FIELD,        TRUE  },
  {  "forest",    SECT_FOREST,    TRUE  },
  {  "hills",    SECT_HILLS,        TRUE  },
  {  "mountain",    SECT_MOUNTAIN,    TRUE  },
  {  "swim",      SECT_WATER_SWIM,  TRUE  },
  {  "noswim",    SECT_WATER_NOSWIM,  TRUE  },
  { "unused",    SECT_UNUSED,    TRUE  },
  {  "air",      SECT_AIR,        TRUE  },
  {  "desert",    SECT_DESERT,    TRUE  },
  {  "underground",  SECT_UNDERGROUND,  TRUE  },
  {  "graveyard",  SECT_GRAVEYARD,    TRUE  },
  {  NULL,      0,              0      }
};



const struct flag_type type_flags[] =
{
  {  "light",          ITEM_LIGHT,          TRUE  },
  {  "scroll",          ITEM_SCROLL,        TRUE  },
  {  "spellbook",      ITEM_SPELLBOOK,      TRUE  },
  {  "wand",            ITEM_WAND,          TRUE  },
  {  "staff",          ITEM_STAFF,          TRUE  },
  {  "weapon",          ITEM_WEAPON,        TRUE  },
  {  "treasure",        ITEM_TREASURE,      TRUE  },
  {  "armor",          ITEM_ARMOR,          TRUE  },
  {  "potion",          ITEM_POTION,        TRUE  },
  {  "furniture",      ITEM_FURNITURE,      TRUE  },
  {  "trash",          ITEM_TRASH,         TRUE  },
  {  "container",      ITEM_CONTAINER,      TRUE  },
  {  "drinkcontainer",  ITEM_DRINK_CON,      TRUE  },
  {  "key",            ITEM_KEY,           TRUE  },
  {  "food",            ITEM_FOOD,          TRUE  },
  {  "money",          ITEM_MONEY,          TRUE  },
  {  "boat",            ITEM_BOAT,          TRUE  },
  {  "npccorpse",      ITEM_CORPSE_NPC,    TRUE  },
  {  "pc corpse",      ITEM_CORPSE_PC,      FALSE  },
  {  "fountain",        ITEM_FOUNTAIN,      TRUE  },
  {  "pill",            ITEM_PILL,          TRUE  },
  {  "protect",        ITEM_PROTECT,        TRUE  },
  {  "map",            ITEM_MAP,            TRUE  },
  { "portal",          ITEM_PORTAL,        TRUE  },
  { "warpstone",      ITEM_WARP_STONE,    TRUE  },
  {  "roomkey",        ITEM_ROOM_KEY,      TRUE  },
  { "gem",            ITEM_GEM,           TRUE  },
  {  "jewelry",        ITEM_JEWELRY,        TRUE  },
  {  "jukebox",        ITEM_JUKEBOX,        TRUE  },
  {  "slotmachine",    ITEM_SLOT_MACHINE,  TRUE  },
  {  "moneypouch",      ITEM_MONEY_POUCH,    TRUE  },
  {  "checkers",        ITEM_CHECKERS,      TRUE  },
  {  "locker",          ITEM_LOCKER,        TRUE  },
  {  "whetstone",      ITEM_WHETSTONE,      TRUE  },
  { "scry_mirror",    ITEM_SCRY_MIRROR,   TRUE  },
  { "book",           ITEM_BOOK,          TRUE  },
  { "fishing rod",    ITEM_FISHING_ROD,   TRUE  },
  { "projectile",     ITEM_PROJECTILE,    TRUE  },
  { "quiver",         ITEM_QUIVER,        TRUE  },
  { "keyring",        ITEM_KEYRING,       TRUE  },
  { "bandage",        ITEM_BANDAGE,       TRUE  },
  {  NULL,      0,              0      }
};


const struct flag_type extra_flags[] =
{
  {  "glow",      ITEM_GLOW,        TRUE  },
  {  "hum",      ITEM_HUM,       TRUE  },
  {  "dark",      ITEM_DARK,        TRUE  },
  {  "lock",      ITEM_LOCK,        TRUE  },
  {  "evil",      ITEM_EVIL,      TRUE  },
  {  "invis",    ITEM_INVIS,        TRUE  },
  {  "magic",    ITEM_MAGIC,        TRUE  },
  {  "nodrop",    ITEM_NODROP,      TRUE  },
  {  "bless",    ITEM_BLESS,        TRUE  },
  {  "antigood",    ITEM_ANTI_GOOD,    TRUE  },
  {  "antievil",    ITEM_ANTI_EVIL,    TRUE  },
  {  "antineutral",  ITEM_ANTI_NEUTRAL,  TRUE  },
  {  "noremove",    ITEM_NOREMOVE,    TRUE  },
  {  "inventory",  ITEM_INVENTORY,    TRUE  },
  {  "nopurge",    ITEM_NOPURGE,    TRUE  },
  {  "rotdeath",    ITEM_ROT_DEATH,    TRUE  },
  {  "visdeath",    ITEM_VIS_DEATH,    TRUE  },
  { "nonmetal",    ITEM_NONMETAL,    TRUE  },
  { "nolocate",     ITEM_NOLOCATE,      TRUE    },
  {  "meltdrop",    ITEM_MELT_DROP,    TRUE  },
  {  "hadtimer",    ITEM_HAD_TIMER,    TRUE  },
  {  "sellextract",  ITEM_SELL_EXTRACT,  TRUE  },
  {  "burnproof",  ITEM_BURN_PROOF,  TRUE  },
  {  "nouncurse",  ITEM_NOUNCURSE,    TRUE  },
  {  "restring",    ITEM_RESTRING,    TRUE  },
  { "claneq",       ITEM_CLANEQ,        TRUE    },
  { "plural",       ITEM_PLURAL,        TRUE    },
  { "pit",          ITEM_DONATION_PIT,  TRUE    },
  { "player_house", ITEM_PLAYER_HOUSE,  TRUE    },
  { "blessed_shield",ITEM_BLESSED_SHIELD, TRUE },
  {  NULL,      0,      0  }
};



const struct flag_type wear_flags[] =
{
  {  "take",          ITEM_TAKE,       TRUE  },
  {  "finger",        ITEM_WEAR_FINGER,  TRUE  },
  {  "neck",          ITEM_WEAR_NECK,    TRUE  },
  {  "body",          ITEM_WEAR_BODY,    TRUE  },
  {  "head",          ITEM_WEAR_HEAD,    TRUE  },
  {  "legs",          ITEM_WEAR_LEGS,    TRUE  },
  {  "feet",          ITEM_WEAR_FEET,    TRUE  },
  {  "hands",        ITEM_WEAR_HANDS,  TRUE  },
  {  "arms",          ITEM_WEAR_ARMS,    TRUE  },
  {  "shield",        ITEM_WEAR_SHIELD,  TRUE  },
  {  "about",        ITEM_WEAR_ABOUT,  TRUE  },
  {  "waist",        ITEM_WEAR_WAIST,  TRUE  },
  {  "wrist",        ITEM_WEAR_WRIST,  TRUE  },
  {  "wield",        ITEM_WIELD,        TRUE  },
  {  "hold",          ITEM_HOLD,        TRUE  },
  { "nosac",        ITEM_NO_SAC,    TRUE  },
  {  "float",      ITEM_WEAR_FLOAT,  TRUE  },
  {  "wearbag",        ITEM_WEAR_BAG,      TRUE  },
  { "back",             ITEM_WEAR_BACK,     TRUE    },
  { "ear",              ITEM_WEAR_EAR,      TRUE    },
  { "lapel",            ITEM_WEAR_LAPEL,    TRUE    },
  { "eye",              ITEM_WEAR_EYE,      TRUE    },
  { "rfoot",            ITEM_WEAR_RFOOT,    TRUE    },
  { "lfoot",            ITEM_WEAR_LFOOT,    TRUE    },
  { "crest",            ITEM_WEAR_CREST,    TRUE    },
  /*    {   "twohands",              ITEM_TWO_HANDS,         TRUE    }, */
  {  NULL,      0,      0  }
};

/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] =
{
  {  "none",      APPLY_NONE,        TRUE  },
  {  "strength",    APPLY_STR,      TRUE  },
  {  "dexterity",  APPLY_DEX,        TRUE  },
  {  "intelligence",  APPLY_INT,      TRUE  },
  {  "wisdom",    APPLY_WIS,        TRUE  },
  {  "constitution",  APPLY_CON,        TRUE  },
  {  "sex",      APPLY_SEX,        TRUE  },
  {  "class",    APPLY_CLASS,    TRUE  },
  {  "level",    APPLY_LEVEL,    TRUE  },
  {  "age",      APPLY_AGE,        TRUE  },
  {  "height",    APPLY_HEIGHT,    TRUE  },
  {  "weight",    APPLY_WEIGHT,    TRUE  },
  {  "mana",      APPLY_MANA,     TRUE  },
  {  "hp",      APPLY_HIT,      TRUE  },
  {  "move",      APPLY_MOVE,        TRUE  },
  {  "gold",      APPLY_GOLD,        TRUE  },
  {  "experience",  APPLY_EXP,        TRUE  },
  {  "ac",      APPLY_AC,        TRUE  },
  {  "hitroll",    APPLY_HITROLL,    TRUE  },
  {  "damroll",    APPLY_DAMROLL,    TRUE  },
  {  "saves",    APPLY_SAVES,    TRUE  },
  {  "savingpara",  APPLY_SAVING_PARA,  TRUE  },
  {  "savingrod",  APPLY_SAVING_ROD,  TRUE  },
  {  "savingpetri",  APPLY_SAVING_PETRI,  TRUE  },
  {  "savingbreath",  APPLY_SAVING_BREATH,TRUE  },
  {  "savingspell",  APPLY_SAVING_SPELL,  TRUE  },
  {  "spellaffect",  APPLY_SPELL_AFFECT,  FALSE  },
  {  NULL,      0,            0      }
};



/*
 * What is seen.
 */
const struct flag_type wear_loc_strings[] =
{
  {  "in the inventory",      WEAR_NONE,    TRUE  },
  {  "as a {Ylight{x",      WEAR_LIGHT,      TRUE  },
  {  "on the left finger",  WEAR_FINGER_L,  TRUE  },
  {  "on the right finger",  WEAR_FINGER_R,  TRUE  },
  {  "around the neck (1)",  WEAR_NECK_1,  TRUE  },
  {  "around the neck (2)",  WEAR_NECK_2,  TRUE  },
  {  "on the body",        WEAR_BODY,    TRUE  },
  {  "over the head",      WEAR_HEAD,      TRUE  },
  {  "on the legs",        WEAR_LEGS,    TRUE  },
  {  "on the feet",        WEAR_FEET,      TRUE  },
  {  "on the hands",     WEAR_HANDS,   TRUE  },
  {  "on the arms",        WEAR_ARMS,      TRUE  },
  {  "as a shield",        WEAR_SHIELD,  TRUE  },
  {  "about the shoulders",  WEAR_ABOUT,   TRUE  },
  {  "around the waist",      WEAR_WAIST,      TRUE  },
  {  "on the left wrist",  WEAR_WRIST_L,  TRUE  },
  {  "on the right wrist",  WEAR_WRIST_R,  TRUE  },
  {  "wielded",            WEAR_WIELD,      TRUE  },
  {  "held in the hands",  WEAR_HOLD,    TRUE  },
  {  "floating nearby",    WEAR_FLOAT,      TRUE  },
  {  "hooked about the hip",  WEAR_BAG,     TRUE  },
  { "on the back",          WEAR_BACK,      TRUE    },
  { "on the left ear",      WEAR_EAR_L,     TRUE    },
  { "on the right ear",     WEAR_EAR_R,     TRUE    },
  { "worn on lapel",        WEAR_LAPEL,     TRUE    },
  { "over left eye",        WEAR_LEYE,      TRUE    },
  { "on right foot",        WEAR_RFOOT,     TRUE    },
  { "on left foot",         WEAR_LFOOT,     TRUE    },
  { "as a crest",           WEAR_CREST,     TRUE    },  
  {  NULL,              0,              0     }
}; 


const struct flag_type wear_loc_flags[] =
{
  {  "none",    WEAR_NONE,  TRUE  },
  {  "light",  WEAR_LIGHT,  TRUE  },
  {  "lfinger",  WEAR_FINGER_L,  TRUE  },
  {  "rfinger",  WEAR_FINGER_R,  TRUE  },
  {  "neck1",  WEAR_NECK_1,  TRUE  },
  {  "neck2",  WEAR_NECK_2,  TRUE  },
  {  "body",    WEAR_BODY,  TRUE  },
  {  "head",    WEAR_HEAD,  TRUE  },
  {  "legs",    WEAR_LEGS,  TRUE  },
  {  "feet",    WEAR_FEET,  TRUE  },
  {  "hands",  WEAR_HANDS,  TRUE  },
  {  "arms",    WEAR_ARMS,  TRUE  },
  {  "shield",  WEAR_SHIELD,  TRUE  },
  {  "about",  WEAR_ABOUT,  TRUE  },
  {  "waist",  WEAR_WAIST,  TRUE  },
  {  "lwrist",  WEAR_WRIST_L,  TRUE  },
  {  "rwrist",  WEAR_WRIST_R,  TRUE  },
  {  "wielded",  WEAR_WIELD,  TRUE  },
  {  "hold",    WEAR_HOLD,  TRUE  },
  {  "floating",  WEAR_FLOAT,  TRUE  },
  {  "hooked",  WEAR_BAG,  TRUE  },
  { "back",         WEAR_BACK,      TRUE    },
  { "lapel",        WEAR_LAPEL,     TRUE    },
  { "lear",         WEAR_EAR_L,     TRUE    },
  { "rear",         WEAR_EAR_R,     TRUE    },
  { "eye",          WEAR_LEYE,      TRUE    },
  { "rfoot",        WEAR_RFOOT,     TRUE    },
  { "lfoot",        WEAR_LFOOT,     TRUE    },
  { "crest",        WEAR_CREST,     TRUE    },
  {  NULL,    0,    0  }
};

const struct flag_type container_flags[] =
{
  {  "closeable",  (A),    TRUE  },
  {  "pickproof",  (B),    TRUE  },
  {  "closed",    (C),    TRUE  },
  {  "locked",    (D),    TRUE  },
  {  "puton",    (E),    TRUE  },
  {   NULL,         0,     0     }
};

/*****************************************************************************
                      ROM - specific tables:
 ****************************************************************************/




const struct flag_type ac_type[] =
{
  {   "pierce",        AC_PIERCE,            TRUE    },
  {   "bash",          AC_BASH,              TRUE    },
  {   "slash",         AC_SLASH,             TRUE    },
  {   "exotic",        AC_EXOTIC,            TRUE    },
  {   NULL,              0,                    0       }
};


const struct flag_type size_flags[] =
{
  {   "tiny",          SIZE_TINY,            TRUE    },
  {   "small",         SIZE_SMALL,           TRUE    },
  {   "medium",        SIZE_MEDIUM,          TRUE    },
  {   "large",         SIZE_LARGE,           TRUE    },
  {   "huge",          SIZE_HUGE,            TRUE    },
  {   "giant",         SIZE_GIANT,           TRUE    },
  {   NULL,              0,                    0       },
};


const struct flag_type weapon_class[] =
{
  {   "exotic",  WEAPON_EXOTIC,    TRUE    },
  {   "sword",  WEAPON_SWORD,    TRUE    },
  {   "dagger",  WEAPON_DAGGER,    TRUE    },
  {   "spear",  WEAPON_SPEAR,    TRUE    },
  {   "mace",  WEAPON_MACE,    TRUE    },
  {   "axe",  WEAPON_AXE,    TRUE    },
  {   "flail",  WEAPON_FLAIL,    TRUE    },
  {   "whip",  WEAPON_WHIP,    TRUE    },
  {   "polearm",WEAPON_POLEARM,    TRUE    },
  {   "crossbow",WEAPON_CROSSBOW,  TRUE    },
  {   "hands",  WEAPON_NULL,        TRUE    },
  {   NULL,  0,      0       }
};


const struct flag_type weapon_type2[] =
{
  {   "flaming",        WEAPON_FLAMING,      TRUE    },
  {   "frost",          WEAPON_FROST,       TRUE    },
  {   "vampiric",       WEAPON_VAMPIRIC,    TRUE    },
  {   "sharp",          WEAPON_SHARP,        TRUE    },
  {   "vorpal",         WEAPON_VORPAL,      TRUE    },
  {   "twohands",       WEAPON_TWO_HANDS,   TRUE    },
  {   "shocking",       WEAPON_SHOCKING,     TRUE    },
  {   "poison",          WEAPON_POISON,      TRUE    },
  {   "clan",            WEAPON_CLAN,        TRUE    },
  {   "mana",            WEAPON_MANA_DRAIN,  TRUE    },
  {   "holy",           WEAPON_HOLY,        TRUE    },
  {   "unholy",         WEAPON_UNHOLY,      TRUE    },
  {   NULL,              0,                    0    }
};

const struct flag_type res_flags[] =
{
  {   "summon",         RES_SUMMON,       TRUE    },
  {   "charm",         RES_CHARM,            TRUE    },
  {   "magic",         RES_MAGIC,            TRUE    },
  {   "weapon",        RES_WEAPON,           TRUE    },
  {   "bash",          RES_BASH,             TRUE    },
  {   "pierce",        RES_PIERCE,           TRUE    },
  {   "slash",         RES_SLASH,            TRUE    },
  {   "fire",          RES_FIRE,             TRUE    },
  {   "cold",          RES_COLD,             TRUE    },
  {   "lightning",     RES_LIGHTNING,        TRUE    },
  {   "acid",          RES_ACID,             TRUE    },
  {   "poison",        RES_POISON,           TRUE    },
  {   "negative",      RES_NEGATIVE,         TRUE    },
  {   "holy",          RES_HOLY,             TRUE    },
  {   "energy",        RES_ENERGY,           TRUE    },
  {   "mental",        RES_MENTAL,           TRUE    },
  {   "disease",       RES_DISEASE,          TRUE    },
  {   "drowning",      RES_DROWNING,         TRUE    },
  {   "light",         RES_LIGHT,            TRUE    },
  {   "sound",         RES_SOUND,       TRUE    },
  {   "wood",         RES_WOOD,       TRUE    },
  {   "silver",         RES_SILVER,       TRUE    },
  {   "iron",         RES_IRON,       TRUE    },
  {   "wind",         RES_WIND,       TRUE    },
  {   "shock",         RES_SHOCK,       TRUE    },
  {   NULL,            0,                       0    }
};


const struct flag_type vuln_flags[] =
{
    {  "summon",   VULN_SUMMON,    TRUE  },
    {  "charm",  VULN_CHARM,    TRUE  },
    {   "magic",         VULN_MAGIC,           TRUE    },
    {   "weapon",        VULN_WEAPON,          TRUE    },
    {   "bash",          VULN_BASH,            TRUE    },
    {   "pierce",        VULN_PIERCE,          TRUE    },
    {   "slash",         VULN_SLASH,           TRUE    },
    {   "fire",          VULN_FIRE,            TRUE    },
    {   "cold",          VULN_COLD,            TRUE    },
    {   "lightning",     VULN_LIGHTNING,       TRUE    },
    {   "acid",          VULN_ACID,            TRUE    },
    {   "poison",        VULN_POISON,          TRUE    },
    {   "negative",      VULN_NEGATIVE,        TRUE    },
    {   "holy",          VULN_HOLY,            TRUE    },
    {   "energy",        VULN_ENERGY,          TRUE    },
    {   "mental",        VULN_MENTAL,          TRUE    },
    {   "disease",       VULN_DISEASE,         TRUE    },
    {   "drowning",      VULN_DROWNING,        TRUE    },
    {   "brightlight",   VULN_LIGHT,           TRUE    },
    {  "sound",   VULN_SOUND,    TRUE  },
    {   "wood",          VULN_WOOD,            TRUE    },
    {   "silver",        VULN_SILVER,          TRUE    },
    {   "iron",          VULN_IRON,            TRUE    },
    {   "wind",          VULN_WIND,            TRUE    },
    {   "shock",          VULN_SHOCK,            TRUE    },
    {   NULL,              0,                    0       }
};

const struct flag_type position_flags[] =
{
    {   "dead",           POS_DEAD,            FALSE   },
    {   "mortal",         POS_MORTAL,          FALSE   },
    {   "incap",          POS_INCAP,           FALSE   },
    {   "stunned",        POS_STUNNED,         FALSE   },
    {   "sleeping",       POS_SLEEPING,        TRUE    },
    {   "resting",        POS_RESTING,         TRUE    },
    {   "sitting",        POS_SITTING,         TRUE    },
    {   "fighting",       POS_FIGHTING,        FALSE   },
    {   "standing",       POS_STANDING,        TRUE    },
    {   NULL,              0,                    0       }
};

const struct flag_type portal_flags[]=
{
    {   "normal_exit",    GATE_NORMAL_EXIT,  TRUE  },
    {  "no_curse",    GATE_NOCURSE,    TRUE  },
    {   "go_with",    GATE_GOWITH,    TRUE  },
    {   "buggy",    GATE_BUGGY,    TRUE  },
    {  "random",    GATE_RANDOM,    TRUE  },
    {   NULL,      0,      0  }
};

const struct flag_type furniture_flags[]=
{
  { "stand_at",    STAND_AT,    TRUE  },
  { "stand_on",    STAND_ON,    TRUE  },
  { "stand_in",    STAND_IN,    TRUE  },
  { "sit_at",      SIT_AT,      TRUE  },
  { "sit_on",      SIT_ON,      TRUE  },
  { "sit_in",      SIT_IN,      TRUE  },
  { "rest_at",     REST_AT,     TRUE  },
  { "rest_on",     REST_ON,     TRUE  },
  { "rest_in",     REST_IN,     TRUE  },
  { "sleep_at",    SLEEP_AT,    TRUE  },
  { "sleep_on",    SLEEP_ON,    TRUE  },
  { "sleep_in",    SLEEP_IN,    TRUE  },
  { "put_at",      PUT_AT,      TRUE  },
  { "put_on",      PUT_ON,      TRUE  },
  { "put_in",      PUT_IN,      TRUE  },
  { "put_inside",  PUT_INSIDE,  TRUE  },
  { "stand_under", STAND_UNDER, TRUE  },
  { "sit_under",   SIT_UNDER,   TRUE  },
  { "rest_under",  REST_UNDER,  TRUE  },
  { "sleep_under", SLEEP_UNDER, TRUE  },
  { "put_under",   PUT_UNDER,   TRUE  },
  {  NULL,         0,           0     }
};

const  struct  flag_type  apply_types  []  =
{
  {  "affects",  TO_AFFECTS,          TRUE  },
  {  "object",  TO_OBJECT,        TRUE  },
  {  "immune",  TO_IMMUNE,          TRUE  },
  {  "resist",  TO_RESIST,        TRUE  },
  {  "vuln",    TO_VULN,          TRUE  },
  {  "weapon",  TO_WEAPON,          TRUE  },
  { "saff",     TO_SPELL_AFFECTS,   TRUE    },
  { "dummy",    TO_DUMMY,           TRUE    },
  { "dummy",    TO_DUMMY,           TRUE    },
  { "dummy",    TO_DUMMY,           TRUE    },
  { "dummy",    TO_DUMMY,           TRUE    },
  { "aff2",     TO_AFFECTS2,        TRUE    },
  {  NULL,    0,                TRUE  }
};

const  struct  bit_type  bitvector_type  []  =
{
  {  affect_flags,      "affect"  },
  { apply_flags,        "apply"     },
  {  imm_flags,          "imm"      },
  {  res_flags,          "res"      },
  {  vuln_flags,          "vuln"    },
  {  weapon_type2,      "weapon"  },
  { spell_affect_flags, "saff"      },
  { dummy_flags,        "dummy"     },
  { dummy_flags,        "dummy"     },
  { dummy_flags,        "dummy"     },
  { dummy_flags,        "dummy"     },
  { affect2_flags,      "aff2"      },
};

const struct  flag_type  spools_flags[] =
{
    {  "note",      NOTE_NOTE,    TRUE  },
    {  "idea",      NOTE_IDEA,    TRUE  },
    {  "penalty",    NOTE_PENALTY,  TRUE  },
    {  "news",      NOTE_NEWS,    TRUE  },
    {  "change",    NOTE_CHANGES,  TRUE  },
    {  "rules",    NOTE_RULES,    TRUE  },
    {  NULL,      0,          0     }
};

const struct clan_titles clan_rank_table[] =
{   
    {{"<{YWildthing{x>"   ,"<{YFreeman{x>"    ,"<{YFreewoman{x>" }, 0 },
    {{"<{YRecruit{x>"    ,"<{YRecruit{x>"    ,"<{YRecruit{x>"   },-3 },
    {{"<{YAcolyte{x>"     ,"<{YAcolyte{x>"    ,"<{YAcolyte{x>"   },-2 },
    {{"<{YAdept{x>"       ,"<{YAdept{x>"      ,"<{YAdept{x>"     },-1 },
    {{"<{YOfficer{x>"     ,"<{YOfficer{x>"    ,"<{YOfficer{x>"   },-1 },
    {{"<{YLieutenant{x>"  ,"<{YLieutenant{x>" ,"<{YLieutenant{x>"}, 0 },
    {{"<{YVassal{x>"      ,"<{YVassal{x>"     ,"<{YVassal{x>"    }, 0 },
    {{"<{YBaron{x>"       ,"<{YBaron{x>"      ,"<{YBaroness{x>"  }, 1 },
    {{"<{YMagistrate{x>"  ,"<{YMagistrate{x>" ,"<{YMagistrate{x>"}, 2 },
    {{"<{YLord{x>"        ,"<{YLord{x>"       ,"<{YLady{x>"      }, 3 },
    {{"<{YLeader{x>"      ,"<{YLeader{x>"     ,"<{YLeader{x>"    }, 4 },
    {{"<{YAngel{x>"       ,"<{YAngel{x>"      ,"<{YAngel{x>"     }, 0 },
    {{"<{YAdvisor{x>"     ,"<{YAdvisor{x>"    ,"<{YAdvisor{x>"   }, 0 },
    {{"<{YOverGod{x>"     ,"<{YGod{x>"        ,"<{YGoddess{x>"   }, 0 },
    {{ NULL, NULL, NULL }}         
};

const struct clan_titles xanadu_clan_rank_table[] =
{   
    {{"{D<{&Hack{D>{x"           ,"{D<{&Hack{D>{x"            ,"{D<{&Hack{D>{x"          },  0 },
    {{"{D<{&Debauchee{D>{x"      ,"{D<{&Rake{D>{x"            ,"{D<{&Chippie{D>{x"       }, -3 },
    {{"{D<{&Bon Vivant{D>{x"     ,"{D<{&Bon Vivant{D>{x"      ,"{D<{&Bon Vivant{D>{x"    }, -2 },
    {{"{D<{&Epicure{D>{x"        ,"{D<{&Epicure{D>{x"         ,"{D<{&Epicure{D>{x"       }, -1 },
    {{"{D<{&Gourmand{D>{x"       ,"{D<{&Gourmand{D>{x"        ,"{D<{&Gourmand{D>{x"      }, -1 },
    {{"{D<{&Sensualist{D>{x"     ,"{D<{&Sensualist{D>{x"      ,"{D<{&Sensualist{D>{x"    },  0 },
    {{"{D<{&Voluptuary{D>{x"     ,"{D<{&Voluptuary{D>{x"      ,"{D<{&Voluptuary{D>{x"    },  0 },
    {{"{D<{&Libertine{D>{x"      ,"{D<{&Libertine{D>{x"       ,"{D<{&Libertine{D>{x"     },  1 },
    {{"{D<{&Pleasuremonger{D>{x" ,"{D<{&Pleasuremonger{D>{x"  ,"{D<{&Pleasuremonger{D>{x"},  2 },
    {{"{D<{&Hedonist{D>{x"       ,"{D<{&Hedonist{D>{x"        ,"{D<{&Hedonist{D>{x"      },  3 },
    {{"{D<{&Sybarite{D>{x"       ,"{D<{&Sybarite{D>{x"        ,"{D<{&Sybarite{D>{x"      },  4 },
    {{"{D<{&Seraphim{D>{x"       ,"{D<{&Malakim{D>{x"         ,"{D<{&Elohim{D>{x"        },  0 },
    {{"{D<{&Slave{D>{x"          ,"{D<{&Slaveboy{D>{x"        ,"{D<{&Slavegirl{D>{x"     },  0 },
    {{"{D<{&God{D>{x"            ,"{D<{&God{D>{x"             ,"{D<{&Goddess{D>{x"       },  0 },
    {{ NULL, NULL, NULL }}         
};

const struct clan_titles velg_clan_rank_table[] =
{   
    {{"<{DFodder{x>"   ,"<{DFodder{x>"    ,"<{DFodder{x>" }, 0 },
    {{"<{DCommoner{x>"    ,"<{DCommoner{x>"    ,"<{DCommoner{x>"   },-3 },
    {{"<{DAcolyte{x>"     ,"<{DAcolyte{x>"    ,"<{DAcolyte{x>"   },-2 },
    {{"<{DAdept{x>"       ,"<{DAdept{x>"      ,"<{DAdept{x>"     },-1 },
    {{"<{DOfficer{x>"     ,"<{DOfficer{x>"    ,"<{DOfficer{x>"   },-1 },
    {{"<{DLieutenant{x>"  ,"<{DLieutenant{x>" ,"<{DLieutenant{x>"}, 0 },
    {{"<{DVassal{x>"      ,"<{DVassal{x>"     ,"<{DVassal{x>"    }, 0 },
    {{"<{DEmissary{x>"       ,"<{DEmissary{x>"      ,"<{DEmissary{x>"  }, 1 },
    {{"<{DSlave Master{x>"  ,"<{DSlave Master{x>" ,"<{DSlave Mistress{x>"}, 2 },
    {{"<{Priest{x>"        ,"<{DWeaponsmaster{x>"       ,"<{DHigh Priestess{x>"      }, 3 },
    {{"<{DPatron{x>"      ,"<{DPatron{x>"     ,"<{DMatron Mother{x>"    }, 4 },
    {{"<{DAngel{x>"       ,"<{DAngel{x>"      ,"<{DAngel{x>"     }, 0 },
    {{"<{DAdvisor{x>"     ,"<{DAdvisor{x>"    ,"<{DAdvisor{x>"   }, 0 },
    {{"<{DDeity{x>"       ,"<{DDeity{x>"      ,"<{DDeity{x>"     }, 0 },
    {{ NULL, NULL, NULL }}         
};

const struct clan_titles light_clan_rank_table[] =
{
    {{"<{GWildthing{x>"   ,"<{GFreeman{x>"    ,"<{GFreewoman{x>" }, 0 },
    {{"<{GRecruit{x>"     ,"<{GRecruit{x>"    ,"<{GRecruit{x>"   },-3 },
    {{"<{GAcolyte{x>"     ,"<{GAcolyte{x>"    ,"<{GAcolyte{x>"   },-2 },
    {{"<{GAdept{x>"       ,"<{GAdept{x>"      ,"<{GAdept{x>"     },-1 },
    {{"<{GOfficer{x>"     ,"<{GOfficer{x>"    ,"<{GOfficer{x>"   },-1 },
    {{"<{GLieutenant{x>"  ,"<{GLieutenant{x>" ,"<{GLieutenant{x>"}, 0 },
    {{"<{GVassal{x>"      ,"<{GVassal{x>"     ,"<{GVassal{x>"    }, 0 },
    {{"<{GBaron{x>"       ,"<{GBaron{x>"      ,"<{GBaroness{x>"  }, 1 },
    {{"<{GMagistrate{x>"  ,"<{GMagistrate{x>" ,"<{GMagistrate{x>"}, 2 },
    {{"<{GLord{x>"        ,"<{GLord{x>"       ,"<{GLady{x>"      }, 3 },
    {{"<{GLeader{x>"      ,"<{GLeader{x>"     ,"<{GLeader{x>"    }, 4 },
    {{"<{GAngel{x>"       ,"<{GAngel{x>"      ,"<{GAngel{x>"     }, 0 },
    {{"<{GAdvisor{x>"     ,"<{GAdvisor{x>"    ,"<{GAdvisor{x>"   }, 0 },
    {{"<{GGod{x>"         ,"<{GGod{x>"        ,"<{GGoddess{x>"   }, 0 },
    {{ NULL, NULL, NULL }}
};

const struct clan_titles cov_clan_rank_table[] =
{
    {{"{D<{rWildthing{D>{x"   ,"{D<{rFreeman{D>{x"    ,"{D<{rFreewoman{D>{x" }, 0 },
    {{"{D<{rRecruit{D>{x"     ,"{D<{rRecruit{D>{x"    ,"{D<{rRecruit{D>{x"   },-3 },
    {{"{D<{rAcolyte{D>{x"     ,"{D<{rAcolyte{D>{x"    ,"{D<{rAcolyte{D>{X"   },-2 },
    {{"{D<{rAdept{D>{x"       ,"{D<{rAdept{D>{x"      ,"{D<{rAdept{D>{x"     },-1 },
    {{"{D<{rOfficer{D>{x"     ,"{D<{rOfficer{D>{x"    ,"{D<{rOfficer{D>{x"   },-1 },
    {{"{D<{rLieutenant{D>{x"  ,"{D<{rLieutenant{D>{x" ,"{D<{rLieutenant{D>{x"}, 0 },
    {{"{D<{rVassal{D>{x"      ,"{D<{rVassal{D>{x"     ,"{D<{rVassal{D>{x"    }, 0 },
    {{"{D<{rBaron{D>{x"       ,"{D<{rBaron{D>{x"      ,"{D<{rBaroness{D>{x"  }, 1 },
    {{"{D<{rMagistrate{D>{x"  ,"{D<{rMagistrate{D>{x" ,"{D<{rMagistrate{D>{x"}, 2 },
    {{"{D<{rLord{D>{x"        ,"{D<{rLord{D>{x"       ,"{D<{rLady{D>{x"      }, 3 },
    {{"{D<{rLeader{D>{x"      ,"{D<{rLeader{D>{x"     ,"{D<{rLeader{D>{x"    }, 4 },
    {{"{D<{rAngel{D>{x"       ,"{D<{rAngel{D>{x"      ,"{D<{rAngel{D>{x"     }, 0 },
    {{"{D<{rAdvisor{D>{x"     ,"{D<{rAdvisor{D>{x"    ,"{D<{rAdvisor{D>{x"   }, 0 },
    {{"{D<{rGod{D>{x"         ,"{D<{rGod{D>{x"        ,"{D<{rGoddess{D>{x"  }, 0 },
    {{ NULL, NULL, NULL }}
};

const struct clan_titles order_clan_rank_table[] =
{
    {{"<{DMarked{x>"       ,"<{DMarked{x>"        ,"<{DMarked{x>"         }, 0 },
    {{"<{DRecruit{x>"      ,"<{DRecruit{x>"       ,"<{DRecruit{x>"        },-3 },
    {{"<{DPage{x>"         ,"<{DPage{x>"          ,"<{DPage{x>"           },-2 },
    {{"<{DGuildsman{x>"    ,"<{DGuildsman{x>"     ,"<{DGuildress{x>"      },-1 },
    {{"<{DScout{x>"        ,"<{DScout{x>"         ,"<{DScout{x>"          },-1 },
    {{"<{DOfficer{x>"      ,"<{DOfficer{x>"       ,"<{DOfficer{x>"        }, 0 },
    {{"<{DBattlemaster{x>" ,"<{DBattlemaster{x>"  ,"<{Dbattlemaster{x>"   }, 0 },
    {{"<{DMinister{x>"     ,"<{DMinister{x>"      ,"<{DMinister{x>"       }, 1 },
    {{"<{DSteward{x>"      ,"<{DSteward{x>"       ,"<{DDame{x>"           }, 2 },
    {{"<{DLordy{x>"        ,"<{DLord{x>"          ,"<{DLady{x>"           }, 3 },
    {{"<{DBlademaster{x>"  ,"<{DBlademaster{x>"   ,"<{DBlademaster{x>"    }, 4 },
    {{"<{DOracle{x>"       ,"<{DOracle{x>"        ,"<{DOracle{x>"         }, 0 },
    {{"<{DGod{x>"          ,"<{DGod{x>"           ,"<{DGoddess{x>"        }, 0 },
    {{"<{DPatron{x>"       ,"<{DPatron{x>"        ,"<{DMatron{x>"         }, 0 },
    {{ NULL, NULL, NULL }}
};

const struct avg_dam_st avgdamtable[] =
{
  /* number of dice,  dice type */
  {1,1},/*0*/
  {1,2},/*1*/
  {1,3},/*2*/
  {2,2},/*3*/
  {2,3},/*4*/
  {2,4},/*5*/
  {3,4},/*6*/
  {2,6},/*7*/
  {2,7},/*8*/
  {2,9},/*9*/
  {3,6},/*10*/
  {2,10},/*11*/
  {5,4},/*12*/
  {3,8},/*13*/
  {4,6},/*14*/
  {5,5},/*15*/
  {4,7},/*16*/
  {5,6},/*17*/
  {4,8},/*18*/
  {2,18},/*19*/
  {5,7},/*20*/
  {6,6},/*21*/
  {5,8},/*22*/
  {2,22},/*23*/
  {7,6},/*24*/
  {5,9},/*25*/
  {4,12},/*26*/
  {6,8},/*27*/
  {7,7},/*28*/
  {2,28},/*29*/
  {6,9},/*30*/
  {7,8},/*31*/
  {8,7},/*32*/
  {6,10},/*33*/
  {7,9},/*34*/
  {7,9},/*35*/
  {8,8},/*36*/
  {2,36},/*37*/
  {7,10},/*38*/
  {6,12},/*39*/
  {8,9},/*40*/
  {2,40},/*41*/
  {7,11},/*42*/
  {2,42},/*43*/
  {8,10},/*44*/
  {9,9},/*45*/
  {3,30},/*46*/
  {2,46},/*47*/
  {2,47},/*48*/
  {7,13},/*49*/
  {10,9},/*50*/
  {10,10},
};
const struct chaos_name chaos_names[] =
{
  {"{mV{Del{wg'{wL{Dar{mn{x"},
  {"{mV{Del{wg'{wL{Dar{mn{x"},
};


/*New Security System - Flags to be allowed*/
const struct flag_type olc_security_flags[] =
{
    {  "area",     OLC_SEC_AREA,    TRUE  },
    {  "help",     OLC_SEC_HELP,    TRUE  },
    {  "mobile",    OLC_SEC_MOBILE,   TRUE  },
    {  "object",    OLC_SEC_OBJECT,   TRUE  },
    {  "program",    OLC_SEC_PROGRAM,  TRUE  },
    {  "reset",    OLC_SEC_RESET,    TRUE  },
    {  "room",     OLC_SEC_ROOM,    TRUE  },
    {  "save",     OLC_SEC_SAVE,    TRUE  },
    {  "security",    OLC_SEC_SECURITY,  TRUE  },
    {  "social",    OLC_SEC_SOCIAL,   TRUE  },
    {   "clan",         OLC_SEC_CLAN,       TRUE    },
    {   "area_link",    OLC_SEC_AREA_LINK,  TRUE    },
    {  NULL,      0,      0  }
};

/*
 * Clan Status Settings
 */
const struct flag_type clan_flags[] =
{
  { "independent",      CLAN_INDEPENDENT,   TRUE    },
  { "peaceful",         CLAN_PEACEFUL,      TRUE    },
  { "law",              CLAN_LAW,           TRUE    },
  { "gwyliad",          CLAN_GWYLIAD,       TRUE    },
  { "changed",          CLAN_CHANGED,       TRUE    },
  { NULL,               0,                  0       }
};

/*
 * Clan Rank Settings
 */
const struct flag_type rank_flags[] =
{
  { "immortal",         RANK_IMMORTAL,      TRUE    },
  { "leader",           RANK_LEADER,        TRUE    },
  { "assistant",        RANK_ASSISTANT,     TRUE    },
  { "ambassador",       RANK_AMBASSADOR,    TRUE    },
  { "conjurer",         RANK_CONJURER,      TRUE    },
  { "priest",           RANK_PRIEST,        TRUE    },
  { "highwayman",       RANK_HIGHWAYMAN,    TRUE    },
  { "knight",           RANK_KNIGHT,        TRUE    },
  { "warlock",          RANK_WARLOCK,       TRUE    },
  { "barbarian",        RANK_BARBARIAN,     TRUE    },
  { "mystic",           RANK_MYSTIC,        TRUE    },
  { "druid",            RANK_DRUID,         TRUE    },
  { "inquisitor",       RANK_INQUISITOR,    TRUE    },
  { "occultist",        RANK_OCCULTIST,     TRUE    },
  { "alchemist",        RANK_ALCHEMIST,     TRUE    },
  { "woodsman",         RANK_WOODSMAN,      TRUE    },
  { NULL,               0,                  0       }
};

/*
 * Nospam Settings
 */
const struct flag_type nospam_flags[] =
{
  { "self_miss",        NOSPAM_SMISS,       TRUE    },
  { "other_miss",       NOSPAM_OMISS,       TRUE    },
  { "self_hit",         NOSPAM_SHIT,        TRUE    },
  { "other_hit",        NOSPAM_OHIT,        TRUE    },
  { "self_effects",     NOSPAM_SEFFECTS,    TRUE    },
  { "other_effects",    NOSPAM_OEFFECTS,    TRUE    },
  { "self_parry",       NOSPAM_SPARRY,      TRUE    },
  { "other_parry",      NOSPAM_OPARRY,      TRUE    },
  { "self_dodge",       NOSPAM_SDODGE,      TRUE    },
  { "other_dodge",      NOSPAM_ODODGE,      TRUE    },
  { "money_split",      NOSPAM_MSPLIT,      TRUE    },
  { NULL,               0,                  0       }
};

const struct flag_type toggle_flags[] =
{
  { "statshow",         TOGGLE_STATSHOW,    TRUE    },
  { "affshow",          TOGGLE_AFFSHOW,     TRUE    },

  { "NULL",             0,                  0       }
};

const struct continent_type continent_table[] =
{ // be sure to increase MAX_CONTINENTS in merc.h (for clan recall data)
  { 0, "kaishaan",   "{DK{wa{Di{rs{Rh{raan{x", TRUE },
  { 1, "calipsutai", "{WCal{ci{gpsu{W'{ct{gai{x", TRUE },
  { 2, "ocean",      "{bO{Bce{wa{Wn{x", FALSE },
  { 3, "unlinked",   "{RUnlinked{x", FALSE }
};

/* class_table order: conjurer, priest, highwayman, knight, warlock, barbarian, mystic, druid, inquisitor, occultist */
const struct default_title_type default_title_table[] = 
{
  { "the magician" },
  { "the benevolent" },
  { "the rogue" },
  { "the brave" },
  { "the battle mage" },
  { "the aggressive" },
  { "the student" },
  { "the natural" },
  { "the honorable" },
  { "the necromancer" },
  { "the scientist" },
  { "the ranger" },
  { "" }
};
