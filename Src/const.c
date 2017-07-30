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
*  ROM 2.4 is copyright 1993-1996 Russ Taylor                             *
*  ROM has been brought to you by the ROM consortium                      *
*      Russ Taylor (rtaylor@efn.org)                                      *
*      Gabrielle Taylor                                                   *
*      Brian Moore (zump@rom.org)                                         *
*  By using this code, you have agreed to follow the terms of the         *
*  ROM license, in the file Rom24/doc/rom.license                         *
***************************************************************************/
/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"

/* item type list */
const struct item_type item_table[] =
{
  {  ITEM_LIGHT,       "light"       },
  {  ITEM_SCROLL,      "scroll"      },
  {  ITEM_WAND,        "wand"        },
  {  ITEM_STAFF,       "staff"       },
  {  ITEM_WEAPON,      "weapon"      },
  {  ITEM_TREASURE,    "treasure"    },
  {  ITEM_ARMOR,       "armor"       },
  {  ITEM_POTION,      "potion"      },
  {  ITEM_CLOTHING,    "clothing"    },
  {  ITEM_FURNITURE,   "furniture"   },
  {  ITEM_TRASH,       "trash"       },
  {  ITEM_CONTAINER,   "container"   },
  {  ITEM_DRINK_CON,   "drink"       },
  {  ITEM_KEY,         "key"         },
  {  ITEM_FOOD,        "food"        },
  {  ITEM_MONEY,       "money"       },
  {  ITEM_BOAT,        "boat"        },
  {  ITEM_CORPSE_NPC,  "npc_corpse"  },
  {  ITEM_CORPSE_PC,   "pc_corpse"   },
  {  ITEM_FOUNTAIN,    "fountain"    },
  {  ITEM_PILL,        "pill"        },
  {  ITEM_PROTECT,     "protect"     },
  {  ITEM_MAP,         "map"         },
  {  ITEM_PORTAL,      "portal"      },
  {  ITEM_WARP_STONE,  "warp_stone"  },
  {  ITEM_ROOM_KEY,    "room_key"    },
  {  ITEM_GEM,         "gem"         },
  {  ITEM_JEWELRY,     "jewelry"     },
  {  ITEM_JUKEBOX,     "jukebox"     },
  {  ITEM_SLOT_MACHINE,"slot_machine"},
  {  ITEM_MONEY_POUCH, "moneypouch"  },
  {  ITEM_CHECKERS,    "checkers"    },
  {  ITEM_LOCKER,      "locker"      },
  {  ITEM_WHETSTONE,   "whetstone"   },
  {  ITEM_SPELLBOOK,   "spellbook"   },
  {  ITEM_SCRY_MIRROR, "scry_mirror" },
  {  ITEM_BOOK,        "book"        },
  {  ITEM_FISHING_ROD, "fishing_rod" },
  {  ITEM_PROJECTILE,  "projectile"  },
  {  ITEM_QUIVER,      "quiver"      },
  {  ITEM_KEYRING,     "keyring"     },
  {  ITEM_BANDAGE,     "bandage"     },
  {  0,                 NULL         }
};

/* weapon selection table */
const struct weapon_type weapon_table[] =
{
  { "sword",   OBJ_VNUM_SCHOOL_SWORD,   WEAPON_SWORD,   &gsn_sword        , "sword"        },
  { "mace",    OBJ_VNUM_SCHOOL_MACE,    WEAPON_MACE,    &gsn_mace         , "mace"         },
  { "dagger",  OBJ_VNUM_SCHOOL_DAGGER,  WEAPON_DAGGER,  &gsn_dagger       , "dagger"       },
  { "axe",     OBJ_VNUM_SCHOOL_AXE,     WEAPON_AXE,     &gsn_axe          , "axe"          },
  { "staff",   OBJ_VNUM_SCHOOL_STAFF,   WEAPON_SPEAR,   &gsn_spear        , "spear"        },
  { "flail",   OBJ_VNUM_SCHOOL_FLAIL,   WEAPON_FLAIL,   &gsn_flail        , "flail"        },
  { "whip",    OBJ_VNUM_SCHOOL_WHIP,    WEAPON_WHIP,    &gsn_whip         , "whip"         },
  { "polearm", OBJ_VNUM_SCHOOL_POLEARM, WEAPON_POLEARM, &gsn_polearm      , "polearm"      },
  { "crossbow",OBJ_VNUM_SCHOOL_CROSSBOW,WEAPON_CROSSBOW,&gsn_crossbow     , "crossbow"     },
  { "hands",   OBJ_VNUM_NULL,           WEAPON_NULL,    &gsn_hand_to_hand , "hand to hand" },
  { NULL,      0,                       0,              NULL              , NULL           },
};

/* wiznet table and prototype for future flag setting */
const struct wiznet_type wiznet_table[] =
{
  { "on",       WIZ_ON,        IM },
  { "prefix",   WIZ_PREFIX,    IM },
  { "ticks",    WIZ_TICKS,     IM },
  { "logins",   WIZ_LOGINS,    IM },
  { "sites",    WIZ_SITES,     L4 },
  { "links",    WIZ_LINKS,     L7 },
  { "newbies",  WIZ_NEWBIE,    IM },
  { "spam",     WIZ_SPAM,      L5 },
  { "deaths",   WIZ_DEATHS,    IM },
  { "resets",   WIZ_RESETS,    L4 },
  { "mobdeaths",WIZ_MOBDEATHS, L4 },
  { "flags",    WIZ_FLAGS,     L5 },
  { "penalties",WIZ_PENALTIES, L5 },
  { "saccing",  WIZ_SACCING,   L5 },
  { "levels",   WIZ_LEVELS,    IM },
  { "load",     WIZ_LOAD,      L2 },
  { "restore",  WIZ_RESTORE,   L2 },
  { "snoops",   WIZ_SNOOPS,    L2 },
  { "switches", WIZ_SWITCHES,  L2 },
  { "secure",   WIZ_SECURE,    L1 },
  { "memcheck", WIZ_MEMCHECK,  L3 },
  { "murder",   WIZ_MURDER,    L4 },
  { "bugs",     WIZ_BUGS,      L3 },
  { "bank",     WIZ_BANK,      L3 },
  { "cleanse",  WIZ_CLEANSE,   L7 },
  { "worklist", WIZ_WORKLIST,  L6 },
  { "mobprogs", WIZ_MOBPROGS,  L5 },
  { "olc",      WIZ_OLC,       L5 },
  { NULL,       0,             0  }
};

/* attack table  -- not very organized :( */
const   struct attack_type  attack_table  [MAX_DAMAGE_MESSAGE]  =
{
  { "none",     "hit",           -1           },  /*  0 */
  { "slice",    "slice",         DAM_SLASH    },
  { "stab",     "stab",          DAM_PIERCE   },
  { "slash",    "slash",         DAM_SLASH    },
  { "whip",     "whip",          DAM_SLASH    },
  { "claw",     "claw",          DAM_SLASH    },  /*  5 */
  { "blast",    "blast",         DAM_BASH     },
  { "pound",    "pound",         DAM_BASH     },
  { "crush",    "crush",         DAM_BASH     },
  { "grep",     "grep",          DAM_SLASH    },
  { "bite",     "bite",          DAM_PIERCE   },  /* 10 */
  { "pierce",   "pierce",        DAM_PIERCE   },
  { "suction",  "suction",       DAM_BASH     },
  { "beating",  "beating",       DAM_BASH     },
  { "digestion","digestion",     DAM_ACID     },
  { "charge",   "charge",        DAM_BASH     },  /* 15 */
  { "slap",     "slap",          DAM_BASH     },
  { "punch",    "punch",         DAM_BASH     },
  { "wrath",    "wrath",         DAM_ENERGY   },
  { "magic",    "magic",         DAM_ENERGY   },
  { "divine",   "divine power",  DAM_HOLY     },  /* 20 */
  { "cleave",   "cleave",        DAM_SLASH    },
  { "scratch",  "scratch",       DAM_PIERCE   },
  { "peck",     "peck",          DAM_PIERCE   },
  { "peckb",    "peck",          DAM_BASH     },
  { "chop",     "chop",          DAM_SLASH    },  /* 25 */
  { "sting",    "sting",         DAM_PIERCE   },
  { "smash",    "smash",         DAM_BASH     },
  { "shbite",   "shocking bite", DAM_LIGHTNING},
  { "flbite",   "flaming bite",  DAM_FIRE     },
  { "frbite",   "freezing bite", DAM_COLD     },  /* 30 */
  { "acbite",   "acidic bite",   DAM_ACID     },
  { "pobite",   "poisonous bite",DAM_POISON   },
  { "chomp",    "chomp",         DAM_PIERCE   },
  { "drain",    "life drain",    DAM_NEGATIVE },
  { "thrust",   "thrust",        DAM_PIERCE   },  /* 35 */
  { "slime",    "slime",         DAM_ACID     },
  { "shock",    "shock",         DAM_LIGHTNING},
  { "thwack",   "thwack",        DAM_BASH     },
  { "flame",    "flame",         DAM_FIRE     },
  { "chill",    "chill",         DAM_COLD     },  /* 40 */
  { "glare",    "glare",         DAM_LIGHT    },
  { "bolt",     "bolt",          DAM_PIERCE   },
  { NULL,       NULL,            0 }
};

/* race table */
const struct race_type race_table[] =
{
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  { "unused", FALSE, 0, 0, 0, 0, 0, 0, 0, 0, 0, SECT_UNUSED },
  {
    "Human",  TRUE,
    0,    0,  0,   0,
    0,    0,  0,
    A|H|M|V,  A|B|C|D|E|F|G|H|I|J|K,
    SECT_CITY
  },
  {
    "Elf",      TRUE,
    0,    AFF_INFRARED,  0, 0,
    0,    RES_CHARM,  VULN_IRON,
    A|H|M|V,  A|B|C|D|E|F|G|H|I|J|K,
    SECT_FOREST
  },
  {
    "Dwarf",    TRUE,
    0,    AFF_DARK_VISION, 0,  0,
    0,    RES_POISON|RES_DISEASE, VULN_DROWNING,
    A|H|M|V,  A|B|C|D|E|F|G|H|I|J|K,
    SECT_MOUNTAIN
  },
  {
    "Giant",    TRUE,
    0,    0,    0, 0,
    0,    RES_BASH|RES_COLD,  VULN_MENTAL,
    A|H|M|V,  A|B|C|D|E|F|G|H|I|J|K,
    SECT_HILLS
  },
  {
    "Brownie",          TRUE,
    0,          AFF_HASTE|AFF_DARK_VISION,  0,            0,
    IMM_DISEASE,        RES_POISON,             VULN_MENTAL,
    A|H|M|V,    A|B|C|D|E|F|G|H|I|J|K,
    SECT_FOREST
  },
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  {
    "Vampire",       TRUE,
    0,    AFF_DARK_VISION|AFF_PASS_DOOR,  0,  0,
    0,    RES_CHARM|RES_IRON|RES_COLD,    VULN_WOOD|VULN_LIGHT,
    A|H|M|V,  A|B|C|D|E|F|G|H|I|J|K,
    SECT_GRAVEYARD
  },
  {
    "Minotaur",         TRUE,
    0,          AFF_DETECT_INVIS|AFF_DETECT_MAGIC, 0,             0,
    0,           RES_COLD,               VULN_ACID,
    A|H|M|V,    A|B|C|D|E|F|G|H|I|J|K,
    SECT_FIELD
  },
  {
    "Cho-Ja",           TRUE,
    0,          AFF_HASTE|AFF_INFRARED,         0,    0,
    0,          RES_SLASH,              VULN_DISEASE,
    A|H|M|V,    A|B|C|D|E|F|G|H|I|J|K,
    SECT_UNDERGROUND
  },
  {
    "Ettin",            TRUE,
    0,          AFF_INFRARED,           0,   0,
    0,          RES_BASH,               VULN_SHOCK|VULN_WIND,
    A|H|M|V,    A|B|C|D|E|F|G|H|I|J|K,
    SECT_HILLS
  },
  {
    "Quickling",       TRUE,
    0,    AFF_INFRARED|AFF_HASTE|AFF_REGENERATION,  0,    0,
    0,    RES_SLASH|RES_SOUND,    VULN_WIND,
    A|H|M|V,  A|B|C|D|E|F|G|H|I|J|K,
    SECT_FOREST
  },
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  {
    "Werewolf",      TRUE,
    0,    AFF_DARK_VISION|AFF_REGENERATION, 0,    0,
    0,  RES_POISON|RES_LIGHTNING,  VULN_SILVER|VULN_MENTAL,
    A|H|M|V,  A|B|C|D|E|F|G|H|J|K|U|V,
    SECT_FOREST
  },
  {
    "Drow",              TRUE,
    0,                       AFF_DARK_VISION|AFF_INFRARED,      0,  0,
    0,                       RES_MENTAL,           VULN_LIGHT,
    A|H|M|V,    A|B|C|D|E|F|G|H|I|J|K,
    SECT_UNDERGROUND
  },
  {
    "Folaryth",     TRUE,
    0,            AFF_FLYING,             0,  0,
    0,            RES_NEGATIVE|RES_MENTAL,          VULN_FIRE,
    A|H|M|V,  A|B|C|D|E|F|G|H|I|J|K,
    SECT_AIR
  },
  {
    "Sythrakai",      TRUE,
    0,           AFF_FLYING,         0,    0,
    0,           RES_MENTAL|RES_CHARM,   VULN_HOLY|VULN_LIGHT,
    A|H|M|V,  A|B|C|D|E|F|G|H|I|J|K,
    SECT_INSIDE
  },
  {
    "Dryth",       TRUE,
    0,           AFF_FLYING,        0,  0,
    0,           RES_BASH,          VULN_MENTAL, /*Dryth curse*/
    A|H|Z,    A|C|D|E|F|G|H|I|J|K|P|Q|U|V|X,
    SECT_AIR
  },
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  {
    "Simian",     TRUE,
    0,     0,       0,  0,
    0, RES_BASH, VULN_FIRE,
    A|H|M|V,  A|B|C|D|E|F|G|H|I|J|K,
    SECT_FOREST
  },
  {
    "Lynrith",              TRUE,
    0,                       AFF_INFRARED,      0,  0,
    0,                       RES_MENTAL,           VULN_FIRE,
    A|H|M|V,    A|B|C|D|E|F|G|H|I|J|K,
    SECT_FIELD
  },
  {
    "Faerie",              TRUE,
    0,                     AFF_FLYING, 0, 0,
    0,                     RES_SLASH,          VULN_CHARM,
    A|H|M|V,   A|B|C|D|E|F|G|H|I|J|K|P,
    SECT_FOREST
  },
  {
    "Ogre", TRUE,
    0, 0, 0, 0,
    0, RES_BASH, VULN_POISON,
    A|H|M|V,  A|B|C|D|E|F|G|H|I|J|K,
    SECT_MOUNTAIN
  },
  {
    "Avariel", TRUE,
    0, AFF_FLYING, 0, 0,
    0, RES_CHARM, VULN_FIRE,
    A|H|M|V,  A|B|C|D|E|F|G|H|I|J|K|P,
    SECT_AIR
  },  
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  {
    "Orc", TRUE,
    0, AFF_BERSERK, 0, 0,
    0, RES_POISON, VULN_FIRE,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_UNDERGROUND
  },
  {
    "Duegar", TRUE,
    0, AFF_INFRARED, 0, 0,
    0, RES_BASH, VULN_LIGHT,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_UNDERGROUND
  },
  {
    "Goblin", TRUE,
    0, 0, 0, 0,
    0, RES_COLD, VULN_DISEASE,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_UNDERGROUND
  },
  {
    "Halfling", TRUE,
    0, 0, 0, 0,
    0, RES_MENTAL, VULN_BASH,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_MOUNTAIN
  },
  {
    "Icington", TRUE,
    0, AFF_FLYING, 0, 0,
    0, RES_COLD, VULN_FIRE,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K|P|Q,
    SECT_AIR
  },
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  {
    "Firgon", TRUE,
    0, AFF_FLYING, 0, 0,
    0, RES_FIRE, VULN_COLD,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K|P|Q,
    SECT_AIR
  },
  {
    "Sarlith", TRUE,
    0, 0, 0, 0,
    0, RES_BASH, VULN_LIGHTNING,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_CITY
  },
  {
    "Centaur", TRUE,
    0, 0, 0, 0,
    0, RES_DISEASE, VULN_PIERCE,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_FOREST
  },
  {
    "Gnome", TRUE,
    0, 0, 0, 0,
    0, RES_PIERCE, VULN_BASH,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_CITY
  },
  {
    "Half Elf", TRUE,
    0, AFF_DARK_VISION, 0, 0,
    0, RES_SLASH, VULN_ACID,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_HILLS
  },
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  {
    "Svirfneblin", TRUE,
    0, AFF_INFRARED, 0, 0,
    0, RES_NEGATIVE, VULN_LIGHT,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_UNDERGROUND
  },
  {
    "Siren", TRUE,
    0, AFF_SWIM, 0, 0,
    0, RES_DROWNING, VULN_DISEASE,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_WATER_NOSWIM
  },
  {
    "Drider", TRUE,
    0, 0, 0, 0,
    0, RES_DISEASE|RES_POISON, VULN_LIGHT,
    A|H|M|V, A|B|D|E|F|K|J|C,
    SECT_UNDERGROUND
  },
  {
    "Treant", TRUE,
    0, 0, 0, 0,
    0, RES_BASH, VULN_FIRE,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_FOREST
  },
  {
    "Troll", TRUE,
    0, 0, 0, 0,
    0, RES_COLD, VULN_FIRE,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_MOUNTAIN
  },
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  {
    "Pixie", TRUE,
    0, AFF_FLYING, 0, 0,
    0, RES_MENTAL|RES_CHARM, VULN_BASH,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K|P,
    SECT_FOREST
  },
  {
    "Valkyrie", TRUE,
    0, 0, 0, 0,
    0, RES_NEGATIVE, VULN_ACID,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_CITY
  },
  {
    "Kroluth", TRUE,
    0, AFF_HASTE, 0, 0,
    0, RES_DISEASE, VULN_FIRE,
    A|H|M|V, A|B|C|D|E|F|G|H|J|K|Q|V,
    SECT_UNDERGROUND
  },
  {
    "Komdon", TRUE,
    0, AFF_DARK_VISION, 0, 0,
    0, RES_POISON, VULN_MENTAL,
    A|H|M|V, A|B|C|D|E|F|G|H|J|K|Q|V,
    SECT_MOUNTAIN
  },
  {
    "Xiranth", TRUE,
    0, 0, 0, 0,
    0, RES_DISEASE|RES_POISON, VULN_LIGHTNING|VULN_SOUND,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_HILLS
  },
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  {
    "Jal'rai", TRUE,
    0, AFF_SWIM, 0, 0,
    0, RES_DROWNING, VULN_FIRE,
    A|H|M|V, A|B|C|D|E|F|G|H|J|K|Q|V,
    SECT_WATER_NOSWIM
  },
  {
    "Nagashurn", TRUE,
    0, AFF_INFRARED, 0, 0,
    0, RES_DISEASE, VULN_COLD,
    A|H|M|V, A|B|C|D|E|F|G|H|J|K|Q|V,
    SECT_UNDERGROUND
  },
  {
    "Yzendri", TRUE,
    0, AFF_FLYING, 0, 0,
    0, RES_NEGATIVE, VULN_HOLY,
    A|H|M|V, A|B|C|D|E|F|G|H|I|J|K,
    SECT_AIR
  },
  {
    "Oule", TRUE,
    0, AFF_DARK_VISION|AFF_FLYING, 0, 0,
    0, RES_WIND, VULN_LIGHT,
    A|H|M|W, A|C|D|E|H|K|P|aa,
    SECT_AIR
  },
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  // ------> END OF PC RACES
  {
    "bat",      FALSE,
    0,    AFF_FLYING|AFF_DARK_VISION,  0,  OFF_DODGE|OFF_FAST,
    0,    0,    VULN_LIGHT,
    A|G|V,    A|C|D|E|F|H|J|K|P,
    SECT_UNUSED
  },
  {
    "bear",      FALSE,
    0,    0,  0,    OFF_CRUSH|OFF_DISARM|OFF_BERSERK,
    0,    RES_BASH|RES_COLD,  0,
    A|G|V,    A|B|C|D|E|F|H|J|K|U|V,
    SECT_UNUSED
  },
  {
    "cat",      FALSE,
    0,    AFF_DARK_VISION,  0,  OFF_FAST|OFF_DODGE,
    0,    0,    0,
    A|G|V,    A|C|D|E|F|H|J|K|Q|U|V,
    SECT_UNUSED
  },
  {
    "centipede",    FALSE,
    0,    AFF_DARK_VISION,   0,  0,
    0,    RES_PIERCE|RES_COLD,  VULN_BASH,
    A|B|G|O,    A|C|K,
    SECT_UNUSED
  },
  {
    "dog",      FALSE,
    0,    0,   0,    OFF_FAST,
    0,    0,    0,
    A|G|V,    A|C|D|E|F|H|J|K|U|V,
    SECT_UNUSED
  },
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  {
    "doll",      FALSE,
    0,    0,   0,    0,
    IMM_COLD|IMM_POISON|IMM_HOLY|IMM_NEGATIVE|IMM_MENTAL|IMM_DISEASE
    |IMM_DROWNING,  RES_BASH|RES_LIGHT,
    VULN_SLASH|VULN_FIRE|VULN_ACID|VULN_LIGHTNING|VULN_ENERGY,
    E|J|M|cc,  A|B|C|G|H|K,
    SECT_UNUSED
  },
  {
    "dragon",     FALSE,
    0,       AFF_INFRARED|AFF_FLYING,   0,  0,
    0,      RES_FIRE|RES_BASH|RES_CHARM,
    VULN_PIERCE|VULN_COLD,
    A|H|Z,    A|C|D|E|F|G|H|I|J|K|P|Q|U|V|X,
    SECT_UNUSED
  },
  {
    "fido",      FALSE,
    0,    0,   0,    OFF_DODGE|ASSIST_RACE,
    0,    0,      VULN_MAGIC,
    A|B|G|V,  A|C|D|E|F|H|J|K|Q|V,
    SECT_UNUSED
  },
  {
    "fish",         FALSE,
    0,      0,  0,      0,
    IMM_DROWNING,   0, VULN_COLD,
    A|F|bb|cc,  A|D|E|F|J|K|O|Q|X,
    SECT_UNUSED
  },
  {
    "fox",      FALSE,
    0,    AFF_DARK_VISION, 0,  OFF_FAST|OFF_DODGE,
    0,    0,    0,
    A|G|V,    A|C|D|E|F|H|J|K|Q|V,
    SECT_UNUSED
  },
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  {
    "hobgoblin",    FALSE,
    0,    AFF_INFRARED,  0,  0,
    0,    RES_DISEASE|RES_POISON,  0,
    A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K|Y,
    SECT_UNUSED
  },
  {
    "kobold",    FALSE,
    0,    AFF_INFRARED, 0,  0,
    0,    RES_POISON,  VULN_MAGIC,
    A|B|H|M|V,  A|B|C|D|E|F|G|H|I|J|K|Q,
    SECT_UNUSED
  },
  {
    "lizard",    FALSE,
    0,    0,    0,  0,
    0,    RES_POISON,  VULN_COLD,
    A|G|X|cc,  A|C|D|E|F|H|K|Q|V,
    SECT_UNUSED
  },
  {
    "modron",    FALSE,
    0,    AFF_INFRARED,  0,  ASSIST_RACE|ASSIST_ALIGN,
    IMM_CHARM|IMM_DISEASE|IMM_MENTAL|IMM_HOLY|IMM_NEGATIVE,
    RES_FIRE|RES_COLD|RES_ACID,  0,
    H,    A|B|C|G|H|J|K,
    SECT_UNUSED
  },
  {
    "orc",      FALSE,
    0,    AFF_INFRARED,  0,  0,
    0,    RES_DISEASE,  VULN_LIGHT,
    A|H|M|V,  A|B|C|D|E|F|G|H|I|J|K,
    SECT_UNUSED
  },
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  {
    "pig",      FALSE,
    0,    0,    0,   0,
    0,    0,    0,
    A|G|V,     A|C|D|E|F|H|J|K,
    SECT_UNUSED
  },
  {
    "rabbit",    FALSE,
    0,    0,  0,  OFF_DODGE|OFF_FAST,
    0,    0,    0,
    A|G|V,    A|C|D|E|F|H|J|K,
    SECT_UNUSED
  },
  {
    "school monster",  FALSE,
    ACT_NOALIGN,  0,  0,    0,
    IMM_CHARM|IMM_SUMMON,  0,    VULN_MAGIC,
    A|M|V,    A|B|C|D|E|F|H|J|K|Q|U,
    SECT_UNUSED
  },
  {
    "snake",    FALSE,
    0,    0,  0,  0,
    0,    RES_POISON,  VULN_COLD,
    A|G|X|Y|cc,  A|D|E|F|K|L|Q|V|X,
    SECT_UNUSED
  },
  {
    "song bird",    FALSE,
    0,    AFF_FLYING, 0,    OFF_FAST|OFF_DODGE,
    0,    0,    0,
    A|G|W,    A|C|D|E|F|H|K|P,
    SECT_UNUSED
  },
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  {
    "troll",    FALSE,
    0,    AFF_REGENERATION|AFF_INFRARED|AFF_DETECT_HIDDEN,  0,
    OFF_BERSERK,
    0,  RES_CHARM|RES_BASH,  VULN_FIRE|VULN_ACID,
    A|B|H|M|V,    A|B|C|D|E|F|G|H|I|J|K|U|V,
    SECT_UNUSED
  },
  {
    "water fowl",    FALSE,
    0,    AFF_SWIM|AFF_FLYING,  0,   0,
    0,    RES_DROWNING,    0,
    A|G|W,    A|C|D|E|F|H|K|P,
    SECT_UNUSED
  },
  {
    "wolf",      FALSE,
    0,    AFF_DARK_VISION,   0,  OFF_FAST|OFF_DODGE,
    0,    0,    0,
    A|G|V,    A|C|D|E|F|J|K|Q|V,
    SECT_UNUSED
  },
  {
    "wyvern",    FALSE,
    0,    AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN,   0,
    OFF_BASH|OFF_FAST|OFF_DODGE,
    IMM_POISON,  0,  VULN_LIGHT,
    A|B|G|Z,    A|C|D|E|F|H|J|K|Q|V|X,
    SECT_UNUSED
  },
  {
    "giant insect",    FALSE,
    0,    AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN,   0,
    OFF_BASH|OFF_FAST|OFF_DODGE,
    IMM_POISON,  0,  VULN_LIGHT,
    A|B|G|Z,    A|C|D|E|F|H|J|K|Q|V|X,
    SECT_UNUSED
  },
/*{
    name,    pc_race?,
    act bits,  aff_by bits, aff2_by bits, off bits,
    imm,    res,    vuln,
    form,    parts,
    native sector
  }, */
  { "unique",    FALSE, 0, 0, 0, 0, 0, 0, 0, 0, 0, SECT_UNUSED },
  { NULL, FALSE, 0, 0, 0, 0, 0, 0, 0, 0, 0, SECT_UNUSED }
};

const struct pc_race_type pc_race_table[] =
{
  {
    "null race", "",  0,
    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  -1,  -1},
    { "" },
    { 13, 13, 13, 13, 13 }, { 18, 18, 18, 18, 18 }, 0,
    { "", NULL, NULL, NULL, NULL, NULL, NULL },
    { "", NULL, NULL, NULL, NULL, NULL, NULL },
    { "", NULL, NULL, NULL, NULL, NULL, NULL },
    0, 0, 0, 0
 },
/*{
    "race name", short name, points,
    { class multipliers },
    { bonus skills },
    { base stats }, { max stats }, size,
    { eye color[MAX_APPR] },
    { hair color[MAX_APPR] },
    { skin color[MAX_APPR] },
    min_weight, max_weight, min_height, max_height
  },*/
  {
    "Human",  "{wHum{x",  0,
    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  -1,  -1},
    { "" },
    { 13, 13, 13, 13, 13 },  { 18, 18, 18, 18, 18 },  SIZE_MEDIUM,
    { "blue", "green", "brown", "light blue", "hazel", NULL, NULL },
    { "brown", "black", "red", "blonde", "white", "gray", "bald" },
    { "tan", "pale", "brown", "black", "fair", NULL, NULL },
    95, 240, 55, 78
  },
  {
    "Elf",  "{cE{Cl{cf{x",  6,
    {  90, 100, 100, 105, 100, 120, 100, 95, 105, 105, 100, 100,  -1,  -1},
    { "sneak" },
    { 12, 14, 13, 15, 11 },  { 16, 20, 18, 21, 15 }, SIZE_MEDIUM,
    { "blue", "green", "brown", "light blue", "hazel", "violet", "gray" },
    { "brown", "black", "blonde", "corn silk", "jet black", "honey", NULL },
    { "tan", "pale", "brown", "fair", "alabaster", NULL, NULL },
    85, 190, 50, 70
  },
  {
    "Dwarf",  "{DDw{Ba{x",  4,
    { 110, 95, 110, 90, 95, 105, 100, 115, 105, 105, 110, 115,  -1,  -1},
    { "berserk" },
    { 14, 13, 14, 10, 14 },  { 20, 16, 19, 15, 20 }, SIZE_SMALL,
    { "blue", "green", "brown", "light blue", NULL, NULL, NULL },
    { "brown", "black", "red", "golden", "white", "gray", "bald" },
    { "tan", "brown", "pale", "ruddy", NULL, NULL, NULL },
    50, 90, 36, 50
  },
  {
    "Giant",  "{gGi{Da{x", 8,
    { 120, 120, 120, 95, 110, 80, 100, 115, 115, 115, 120, 105,  -1,  -1},
    { "stomp" },
    { 16, 10, 13, 11, 15 },  { 22, 15, 18, 14, 21 }, SIZE_GIANT,
    { "blue", "green", "brown", "light blue", NULL, NULL, NULL },
    { "brown", "black", "red", "blonde", "white", "gray", "bald" },
    { "tan", "pale", "brown", "black", "fair", "pale blue", NULL },
    500, 800, 110, 180
  },
/*{
    "race name", short name, points,
    { class multipliers },
    { bonus skills },
    { base stats }, { max stats }, size,
    { eye color[MAX_APPR] },
    { hair color[MAX_APPR] },
    { skin color[MAX_APPR] },
    min_weight, max_weight, min_height, max_height
  },*/
  {
    "Brownie",  "{yBw{cn{x",   9,
    { 100, 110, 80, 120, 100, 130, 105, 100, 120, 120, 100, 100,  -1,  -1},
    { "sneak" },
    { 13, 12, 13, 15, 12 },  { 15, 20, 17, 21, 17 },  SIZE_SMALL,
    { "blue", "green", "bright blue", "light blue", "bright green", "light green", NULL },
    { "brown", "black", "dirty blonde", "gray", "blonde", NULL, NULL },
    { "tan", "brown", NULL, NULL, NULL, NULL, NULL },
    45, 90, 36, 50
  },
  {
    "Vampire",  "{RV{ram{x",       14,
    { 105, 105, 100, 110, 105, 105, 105, 105, 110, 110, 100, 120,  -1,  -1},
    { "bite" },
    { 15, 11, 14, 13, 12 },  { 20, 19, 18, 16, 17 },  SIZE_MEDIUM,
    { "blue", "green", "brown", "light blue", "hazel", "pale", "white" },
    { "brown", "blue-black", "red", "ash blonde", "white", "gray", "black" },
    { "white", "pale", "light tan", "alabaster", "porcelyn", NULL, NULL },
    95, 220, 55, 78
  },
  {
    "Minotaur", "{yMi{bn{x",   12,
    { 100, 100, 120, 105, 100, 90, 100, 105, 105, 105, 110, 100,  -1,  -1},
    { "gore" },
    { 15, 10, 12, 11, 17 },    { 20, 16, 16, 17, 21 },  SIZE_LARGE,
    { "blue", "green", "brown", "red", "yellow", NULL, NULL },
    { "brown fur", "black fur", "red fur", "dirty blonde fur", NULL, NULL, NULL },
    { "red hide", "yellow hide", "brown hide", "black hide", NULL, NULL, NULL },
    320, 460, 80, 96
  },
  {
    "Cho-Ja",    "{GCh{go{x",   12,
    { 110, 90, 80, 90, 105, 110, 100, 100, 110, 110, 120, 105,  -1,  -1},
    { "second attack"},
    { 15, 11, 12, 12, 15 },    { 21, 15, 19, 16, 19 },  SIZE_MEDIUM,
    { "bright blue", "green", "brown", "bright green", "black", NULL, NULL },
    { "none", NULL, NULL, NULL, NULL, NULL, NULL },
    { "green exoskeleton", "brown exoskeleton", "black exoskeleton", "gray exoskeleton", "white exoskeleton", NULL, NULL },
    110, 280, 55, 78
  },
/*{
    "race name", short name, points,
    { class multipliers },
    { bonus skills },
    { base stats }, { max stats }, size,
    { eye color[MAX_APPR] },
    { hair color[MAX_APPR] },
    { skin color[MAX_APPR] },
    min_weight, max_weight, min_height, max_height
  },*/
  {
    "Ettin",    "{DE{rtt{x",   16,
    { 120, 120, 130, 90, 125, 90, 110, 120, 120, 120, 125, 125,  -1,  -1},
    { "stomp", "demand"},
    { 18, 10, 11, 9, 17 }, { 23, 13, 18, 14, 22 },  SIZE_GIANT,
    { "blue", "green", "brown", "red", "dull black", "bloodshot", NULL },
    { "brown", "black", "red", "blonde", "white", "gray", "bald" },
    { "tan", "pale", "brown", "black", "fair", NULL, NULL },
    550, 1000, 110, 180
  },
  {
    "Quickling",    "{wQ{Du{wi{x",   8,
    { 100, 100, 80, 120, 100, 130, 90, 100, 125, 125, 110, 115,  -1,  -1},
    { "invisibility"},
    { 12, 12, 12, 18, 11 },    { 15, 18, 17, 23, 17 },  SIZE_SMALL,
    { "blue", "yellow", "brown", "light blue", "orange", "red", "silver" },
    { "silver", "white", "gray", NULL, NULL, NULL, NULL },
    { "pale blue", "light blue", "light tan", "fair", "silver", NULL, NULL },
    40, 85, 36, 45
  },
  {
    "Werewolf", "{yW{re{yr{x",  10,
    { 110, 110, 100, 110, 110, 100, 90,  80, 120, 120, 115,  90,  -1,  -1},
    { "hunt" },
    { 13, 12, 15, 11, 14 },    { 19, 15, 17, 21, 18 }, SIZE_LARGE,
    { "yellow", "bright yellow", "brown", "bright blue", "brown", "red", "black" },
    { "silver", "brown", "gray", "black", "white", NULL, NULL },
    { "silver fur", "brown fur", "gray fur", "black fur", "white fur", NULL, NULL },
    275, 350, 84, 100
  },
  {
    "Drow",  "{mDr{Dw{x",          8,
    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  -1,  -1},
    { "hide","faerie fire" },
    { 11, 15, 16, 15, 12 }, { 16, 19, 18, 20, 17 }, SIZE_MEDIUM,
    { "blue", "green", "white", "purple", "gray", NULL, NULL },
    { "brown", "black", "red", "ash blonde", "white", "gray", NULL },
    { "gray", "pale white", "black", "blue black", NULL, NULL, NULL },
    95, 180, 62, 75
  },
/*{
    "race name", short name, points,
    { class multipliers },
    { bonus skills },
    { base stats }, { max stats }, size,
    { eye color[MAX_APPR] },
    { hair color[MAX_APPR] },
    { skin color[MAX_APPR] },
    min_weight, max_weight, min_height, max_height
  },*/
  {
    "Folaryth",  "{WF{wl{Dy{x",  7,
    {100, 100, 130, 130, 110, 130, 110, 100, 100, 120, 110, 105,  -1,  -1},
    { "second cast" },
    { 12, 15, 15, 12, 11 },  { 16, 20, 20, 18, 16 }, SIZE_MEDIUM,
    { "blue", "green", "brown", "light blue", "emerald", "hazel", "sapphire" },
    { "brown", "black", "red", "blonde", "white", "gray", "bald" },
    { "tan", "pale", "brown", "fair", "alabaster", NULL, NULL },
    95, 200, 60, 72
  },
  {
    "Sythrakai",  "{rS{Dy{wt{x",  8,
    { 100, 105, 120, 150, 120, 140, 130, 110, 120, 100,  95, 125,  -1,  -1},
    { "meditation", "sneak" },
    { 14, 12, 12, 14, 11 },  { 17, 19, 18, 19, 17 }, SIZE_MEDIUM,
    { "blue", "green", "gray", "white", "red", "bloodshot", "black" },
    { "brown", "black", "white", "gray", NULL, NULL, NULL },
    { "pasty white", "pale white", "gray", "white", NULL, NULL, NULL },
    95, 220, 55, 78
  },
  {
    "Dryth",   "{cD{gry{x",   12,
    { 150, 150, 120, 100, 130, 110, 120, 120, 120, 110, 130, 100,  -1,  -1},
    { "tail" },
    { 15, 11, 14, 11, 14 }, { 20, 16, 18, 17, 19 }, SIZE_LARGE,
    { "blue", "green", "brown", "light blue", "black", NULL, NULL },
    { "brown", "black", "red", "blonde", "white", "gray", "green" },
    { "green", "pale green", "deep green", "jade", "cyan", "malachite", "olive" },
    300, 450, 80, 96
  },
  {
    "Simian",   "{yS{Di{ym{x",      9, 
    { 175, 150, 100, 110, 140, 100, 120, 140, 140, 140, 130,  95,  -1,  -1},
    { "fast healing" },
    { 16, 11, 13, 12, 13 }, { 22, 15, 19, 16, 18 }, SIZE_LARGE,
    { "white", "yellow", "green", "blue", "orange", "black", NULL },
    { "brown", "yellow", "red", "gray", NULL, NULL, NULL },
    { "brown fur", "yellow fur", "red fur", "gray fur", NULL, NULL, NULL },
    300, 450, 78, 93
  },
/*{
    "race name", short name, points,
    { class multipliers },
    { bonus skills },
    { base stats }, { max stats }, size,
    { eye color[MAX_APPR] },
    { hair color[MAX_APPR] },
    { skin color[MAX_APPR] },
    min_weight, max_weight, min_height, max_height
  },*/
  {
    "Lynrith",   "{yL{Yy{yn{x", 8,
    { 125, 125, 100, 100, 125, 100, 100, 115, 110, 110, 115,  90,  -1,  -1},
    { "hunt" },
    { 14, 11, 12, 15, 13 }, { 20, 16, 18, 19, 17 }, SIZE_MEDIUM,
    { "blue", "green", "white", "gray", "yellow", NULL, NULL },
    { "yellow", "orange", "red", "white", "gray", "black", NULL },
    { "yellow fur", "orange fur", "red fur", "white fur", "calico fur", "black fur", "gray fur" },
    110, 270, 50, 72
  },
  {
    "Faerie",     "{cFa{me{x",        6,
    { 100, 100, 120, 130, 105, 130, 115, 100, 115, 115, 100, 105,  -1,  -1},
    { "entangle" },
    { 11, 15, 15, 13, 11 }, { 15, 21, 20, 18, 16 }, SIZE_SMALL,
    { "blue", "green", "brown", "light blue", "gray", NULL, NULL },
    { "brown", "black", "blonde", "white", "gray", "bald", "ash blonde" },
    { "tan", "brown", "fair", "deep brown", "dark", NULL, NULL },
    40, 78, 40, 53
  },
  {
    "Ogre",   "{wO{ygr{x",   10,
    { 125, 105, 125, 100, 125, 100, 110, 125, 110, 110, 130, 115,  -1,  -1},
    { "bash" },
    { 14, 11, 13, 13, 14 }, {20, 14, 19, 17, 20 }, SIZE_LARGE,
    { "red", "yellow", "orange", "light green", "bloodshot", NULL, NULL },
    { "brown", "black", "yellow", "white", "gray", "green", NULL },
    { "green", "dark green", "brown", "pale green", NULL, NULL, NULL },
    320, 470, 80, 98
  },
  {
    "Avariel",   "{cA{mv{cl{x",   7,
    { 100, 105, 100, 130, 105, 130, 105, 100, 115, 115, 100, 100,  -1,  -1},
    { "clear head" },
    { 10, 15, 14, 14, 12 }, {16, 20, 18, 19, 17 }, SIZE_MEDIUM,
    { "blue", "green", "brown", "light blue", "gray", "cerulean", "steel blue" },
    { "brown", "black", "red", "blonde", "white", "gray", "auburn" },
    { "tan", "pale", "brown", "fair", "alabaster", NULL, NULL },
    90, 190, 55, 78
  },
/*{
    "race name", short name, points,
    { class multipliers },
    { bonus skills },
    { base stats }, { max stats }, size,
    { eye color[MAX_APPR] },
    { hair color[MAX_APPR] },
    { skin color[MAX_APPR] },
    min_weight, max_weight, min_height, max_height
  },*/
  {
    "Orc",   "{rO{yr{rc{x",   5,
    { 100, 115, 110, 100, 105, 100, 110, 125, 115, 115, 105, 130,  -1,  -1},
    { "butcher" },
    { 15, 11, 13, 12, 14 }, { 20, 16, 16, 18, 20 }, SIZE_MEDIUM,
    { "red", "yellow", "brown", "white", "black", "bloodshot", NULL },
    { "brown", "black", "white", "gray", NULL, NULL, NULL },
    { "tan", "green", "brown", "black", "dull green", "gray", NULL },
    100, 210, 55, 78
  },
  {
    "Duegar",    "{wD{Due{x",    4,
    { 125, 100, 125, 100, 115, 100, 115, 120, 115, 115, 110, 115,  -1,  -1},
    { "berserk" },
    { 14, 14, 10, 13, 14 }, { 20, 15, 19, 16, 20 }, SIZE_SMALL,
    { "black", "green", "brown", "pale green", "red", NULL, NULL },
    { "brown", "black", "dirty blonde", "white", "gray", NULL, NULL },
    { "black", "gray", "light gray", "dark gray", NULL, NULL, NULL },
    45, 92, 36, 52
  },
  {
    "Goblin",    "{gG{yo{gb{x",   6,
    { 125, 115, 100, 100, 115, 100, 110, 125, 125, 125, 105, 110,  -1,  -1},
    { "darksight" },
    { 15, 11, 13, 12, 14 }, { 19, 16, 16, 19, 20 }, SIZE_MEDIUM,
    { "black", "white", "brown", "gray", NULL, NULL, NULL },
    { "brown", "black", "red", "yellow", "white", "gray", NULL },
    { "green", "pale green", "light green", "deep green", NULL, NULL, NULL },
    110, 200, 50, 74
  },
  {
    "Halfling",   "{wHa{yf{x",   5,
    { 125, 100, 100, 125, 115, 125, 105, 115, 120, 120, 110, 120,  -1,  -1},
    { "hide" },
    { 11, 12, 15, 14, 13 }, { 15, 17, 21, 20, 17 }, SIZE_SMALL,
    { "blue", "green", "brown", "gray", NULL, NULL, NULL },
    { "brown", "black", "red", "blonde", "white", "gray", NULL },
    { "tan", "brown", "deep brown", "dark", NULL, NULL, NULL },
    40, 85, 36, 50
  },
/*{
    "race name", short name, points,
    { class multipliers },
    { bonus skills },
    { base stats }, { max stats }, size,
    { eye color[MAX_APPR] },
    { hair color[MAX_APPR] },
    { skin color[MAX_APPR] },
    min_weight, max_weight, min_height, max_height
  },*/
  {
    "Icington", "{wI{cc{bn{x",  14,
    { 100, 100, 130, 130, 100, 130, 105, 120, 110, 110, 100, 105,  -1,  -1},
    { "frost breath", "tail" },
    { 11, 14, 14, 12, 14 }, { 16, 19, 20, 17, 18 }, SIZE_LARGE,
    { "blue", "white", "deep blue", "gray", "cobalt blue", "azure", "steel blue" },
    { "none", NULL, NULL, NULL, NULL, NULL, NULL },
    { "white scales", "blue scales", "ice blue scales", "frost white scales", "silvery scales", NULL, NULL },
    350, 480, 85, 105
  },
  {
    "Firgon", "{yF{Ri{rr{x", 14,
    { 130, 130, 110, 100, 120, 100, 110, 125, 110, 110, 125, 115,  -1,  -1},
    { "fire breath", "tail" },
    { 15, 11, 14, 11, 14 }, { 19, 15, 20, 18, 18 }, SIZE_LARGE,
    { "blue", "green", "brown", "red", "red-brown", "fiery red", NULL },
    { "none", NULL, NULL, NULL, NULL, NULL, NULL },
    { "red scales", "red brown scales", "brown scales", "crimson scales", NULL, NULL, NULL },
    350, 480, 86, 105
  },
  {
    "Sarlith", "{yS{Wa{yr{x", 13,
    { 130, 130, 115,  90, 125,  90, 110, 120, 105, 105, 125, 110,  -1,  -1},
    { "critical strike" },
    { 14, 11, 13, 12, 15 }, { 21, 15, 18, 16, 20 }, SIZE_GIANT,
    { "blue", "green", "brown", "light blue", "purple", "gray", "hazel" },
    { "brown", "black", "red", "blonde", "white", "gray", "bald" },
    { "tan", "pale", "brown", "fair", NULL, NULL, NULL },
    550, 800, 120, 165
  },
  {
    "Centaur", "{wC{yn{wt{x", 9,
    { 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110,  95,  -1,  -1},
    { "kick" },
    { 14, 14, 11, 12, 14 }, { 18, 18, 18, 17, 19 }, SIZE_LARGE,
    { "blue", "green", "brown", "light blue", "hazel", NULL, NULL },
    { "brown", "black", "red", "blonde", "white", "gray", "auburn" },
    { "tan", "pale", "brown", "black", "fair", "dark", NULL },
    275, 410, 80, 96
  },
/*{
    "race name", short name, points,
    { class multipliers },
    { bonus skills },
    { base stats }, { max stats }, size,
    { eye color[MAX_APPR] },
    { hair color[MAX_APPR] },
    { skin color[MAX_APPR] },
    min_weight, max_weight, min_height, max_height
  },*/
  {
    "Gnome", "{cGn{ym{x", 8,
    { 100, 100, 100, 140, 110, 140, 100, 120, 105, 105, 105, 100,  -1,  -1},
    { "sneak", "hide" },
    { 11, 13, 15, 14, 12 }, { 15, 19, 19, 20, 17 }, SIZE_SMALL,
    { "blue", "green", "brown", "light blue", "yellow", "light green", "hazel" },
    { "brown", "black", "red", "blonde", "white", "gray", "bald" },
    { "tan", "pale", "brown", "fair", "dark", "light", NULL },
    40, 80, 36, 50
  },
  {
    "Half Elf", "{cHlf{x", 6,
    { 105, 105, 125, 105, 105, 105, 105, 105, 105, 105, 100, 100,  -1,  -1},
    { "hide" },
    { 12, 13, 15, 14, 11 }, { 19, 18, 21, 16, 16 }, SIZE_MEDIUM,
    { "blue", "green", "brown", "light blue", "gray", NULL, NULL },
    { "brown", "black", "red", "blonde", "white", "gray", NULL },
    { "tan", "pale", "fair", "light brown", "alabaster", NULL, NULL },
    90, 180, 55, 78
  },
  {
    "Svirfneblin", "{DSv{yf{x", 4,
    { 125, 100, 130, 100, 115, 100, 110, 120, 110, 110, 115, 110,  -1,  -1},
    { "sneak" },
    { 12, 15, 14, 12, 12 }, { 16, 20, 20, 17, 17 }, SIZE_SMALL,
    { "gray", "white", "black", "brown", "red", NULL, NULL },
    { "brown", "black", "red", "dirty blonde", "white", "gray", "bald" },
    { "gray", "brown", "deep gray", "fair", NULL, NULL, NULL },
    45, 80, 38, 49
  },
  {
    "Siren", "{mSr{rn{x", 8,
    { 100, 100, 130, 130, 110, 130, 125, 105, 115, 115, 100, 110,  -1,  -1},
    { "charm person" },
    { 11, 15, 14, 13, 12 }, { 16, 21, 20, 16, 17 }, SIZE_MEDIUM,
    { "blue", "green", "brown", "cerulean", "maroon", "sea green", "aquamarine" },
    { "brown", "black", "red", "blonde", "white", "gray", "bald" },
    { "tan", "pale", "brown", "black", "fair", "alabaster", NULL },
    90, 180, 55, 73
  },
/*{
    "race name", short name, points,
    { class multipliers },
    { bonus skills },
    { base stats }, { max stats }, size,
    { eye color[MAX_APPR] },
    { hair color[MAX_APPR] },
    { skin color[MAX_APPR] },
    min_weight, max_weight, min_height, max_height
  },*/
  {
    "Drider", "{wD{mri{x", 9,
    { 105, 115, 105, 105, 105, 105, 105, 105, 105, 105, 105, 120,  -1,  -1},
    { "envenom" },
    { 13, 14, 12, 15, 11 }, { 18, 19, 17, 20, 16  }, SIZE_MEDIUM,
    { "blue", "green", "brown", "light blue", "red", "yellow", "purple" },
    { "brown", "black", "red", "yellow", "white", "gray", "bald" },
    { "tan", "pale", "brown", "black", "fair", "gray", NULL },
    95, 200, 58, 79
  },
  {
    "Treant", "{gT{Gr{ge{x", 13,
    { 120, 100, 120, 100, 120, 100, 110, 110, 110, 110, 130,  85,  -1,  -1},
    { "bark skin" },
    { 14, 13, 15, 10, 13 }, { 20, 17, 19, 15, 19 }, SIZE_GIANT,
    { "blue", "green", "brown", "red", "hazel", "black", NULL },
    { "green leaves", "autumn leaves", "white flowers", "silver leaves", "pink flowers", NULL, NULL },
    { "brown bark", "tan bark", "silver bark", "white bark", "gray bark", "black bark", "charred bark" },
    550, 800, 150, 185
  },
  {
    "Troll", "{gTr{yo{x", 9,
    { 120, 120, 100, 100, 110, 100, 110, 110, 100, 100, 115, 110,  -1,  -1},
    { "rub" },
    { 15, 12, 11, 14, 13 }, { 19, 16, 16, 20, 19 }, SIZE_LARGE,
    { "white", "green", "brown", "gray", "yellow", "red", NULL },
    { "brown", "black", "sandy yellow", "white", "gray", "bald", NULL },
    { "yellow", "brown", "tan", "black", "deep brown", NULL, NULL },
    300, 430, 86, 100
  },
  {
    "Pixie", "{cP{Ci{bx{x", 6,
    {  90,  90, 100, 140, 100, 140, 110, 100, 115, 115,  95, 110,  -1,  -1},
    { "second cast" },
    { 10, 16, 15, 14, 10 }, { 15, 22, 20, 19, 14 }, SIZE_TINY,
    { "blue", "green", "brown", "aqua", "pink", "red", "violet" },
    { "brown", "black", "red", "blonde", "bald", "silver", "pink" },
    { "pink", "white", "peach", "green", "hot pink", "aqua", "silver" },
    15, 28, 12, 20
  },
/*{
    "race name", short name, points,
    { class multipliers },
    { bonus skills },
    { base stats }, { max stats }, size,
    { eye color[MAX_APPR] },
    { hair color[MAX_APPR] },
    { skin color[MAX_APPR] },
    min_weight, max_weight, min_height, max_height
  },*/
  {
    "Valkyrie", "{WVal{x", 11,
    { 115, 125, 100, 100, 115, 100, 105, 120, 110, 110, 115, 100,  -1,  -1},
    { "guided strike" },
    { 14, 12, 13, 13, 13 }, { 19, 18, 17, 19, 17 }, SIZE_MEDIUM,
    { "blue", "green", "brown", "light blue", "gray", "golden", "steel blue" },
    { "blonde", "brown", "light brown", "golden blonde", "white", "gray", "bald" },
    { "tan", "pale", "light brown", "fair", "alabaster", NULL, NULL },
    120, 260, 55, 78
  },
  {
    "Kroluth", "{cK{gr{co{x", 12,
    { 130, 130,  90, 100, 120, 100,  90, 115, 110, 110, 120, 105,  -1,  -1},
    { "nimbleness", "tail" },
    { 14, 11, 11, 15, 14 }, { 19, 14, 16, 21, 20 }, SIZE_MEDIUM,
    { "blue", "green", "brown", "pink", "black", "yellow", "red" },
    { "none", "green antennae", "blue antennae", "black antennae", "brown antennae", "red antennae", "pink antennae" },
    { "green exoskeleton", "brown exoskeleton", "black exoskeleton", "red exoskeleton", NULL, NULL, NULL },
    95, 230, 55, 75
  },
  {
    "Komdon", "{gKom{x", 10,
    { 110, 130, 100, 100, 115, 100, 110, 120, 110, 110, 110, 120,  -1,  -1}, 
    { "envenom", "tail" },
    { 14, 14, 12, 12, 13 }, { 20, 18, 16, 17, 19 }, SIZE_MEDIUM,
    { "blue", "green", "brown", "yellow", "red", NULL, NULL },
    { "none", NULL, NULL, NULL, NULL, NULL, NULL },
    { "green", "pale green", "brown", "maroon", "red", "gray", "olive" },
    95, 250, 58, 70
  },
  {
    "Xiranth", "{WX{cir{x", 10,
    { 90, 100, 100, 115, 105, 115, 110, 100, 115, 115,  85, 120,  -1,  -1},
    { "teleport" },
    { 11, 15, 16, 13, 10 }, { 16, 20, 22, 18, 14 }, SIZE_TINY,
    { "black", "red", "yellow", "golden", "crimson", NULL, NULL },
    { "black", "gray", "white", "pale black", NULL, NULL, NULL },
    { "blue", "deep blue", "pale blue", "green", "pale green", "deep green", NULL },
    18, 35, 15, 24
  },
/*{
    "race name", short name, points,
    { class multipliers },
    { bonus skills },
    { base stats }, { max stats }, size,
    { eye color[MAX_APPR] },
    { hair color[MAX_APPR] },
    { skin color[MAX_APPR] },
    min_weight, max_weight, min_height, max_height
  },*/
  {
    "Jal'rai", "{DJ{ga{cl{x", 10,
    { 110, 105, 90, 100, 105, 100, 95, 105, 115, 105, 110, 110,  -1,  -1},
    { "electric aura" },
    { 13, 12, 11, 15, 14 }, { 18, 17, 14, 21, 20 }, SIZE_MEDIUM,
    { "black", "gray", "white", NULL, NULL, NULL, NULL },
    { "none", NULL, NULL, NULL, NULL, NULL, NULL },
    { "blue", "blue green", "green", "gray", NULL, NULL, NULL },
    90, 220, 55, 78
  },
  {
    "Nagashurn", "{DN{gag{x", 10,
    { 100, 100, 100, 110, 105, 120, 105, 95, 100, 105, 100, 115,  -1,  -1},
    { "fast healing", "tail" },
    { 11, 14, 15, 15, 10 }, { 15, 21, 19, 20, 15 }, SIZE_MEDIUM,
    { "blue", "green", "white", "purple", "hazel", NULL, NULL },
    { "brown", "black", "red", "blonde", "white", "gray", "bald" },
    { "gray", "pale white", "black", NULL, NULL, NULL, NULL },
    100, 200, 55, 75
  },
  {
    "Yzendri", "{DYzn{x", 10,
    { 100, 105, 100, 110, 110, 110, 110, 105, 115, 95,  90, 130,  -1,  -1},
    { "parry" },
    { 12, 14, 15, 12, 12 }, { 17, 19, 20, 17, 17 }, SIZE_SMALL,
    { "white", "green", "brown", "red", "purple", NULL, NULL },
    { "black", "white", "gray", "deep gray", NULL, NULL, NULL },
    { "gray", "deep gray", "black", NULL, NULL, NULL, NULL },
    45, 90, 40, 52
  },
  {
    "Oule", "{yOul{x", 9,
    { 105,  95, 100, 100,  95, 110, 110, 100, 110, 110, 110, 100,  -1,  -1},
    { "peek" },
    { 11, 14, 17, 13, 10 }, { 16, 19, 21, 18, 16 }, SIZE_SMALL,
    { "blue", "green", "brown", "red", "black", "hazel", NULL },
    { "brown down", "red down", "green down", "gray down", "blue down", "yellow down", NULL },
    { "brown feathers", "red feathers", "green feathers", "gray feathers", "yellow feathers", NULL, NULL },
    40, 85, 30, 45
  }
};
/*
 * Class table.
 */
const struct class_type class_table[MAX_CLASS] =
{
/*{
    classname,  whoname,   primeStat, defaultWeapon,
    { Guilds[4] }, maxPrac, thac0_00, thac0_32, minHP, maxHP, ManaUser?,
    "basics groups", "default group"
  },*/
  {
    "Conjurer", "{bC{mjr{x",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
    { 5659, 8410, 3972, 11215 },  75,  20, 6,  7,  11, TRUE,
    "conjurer basics", "conjurer default"
  },
  {
    "Priest", "{yP{wri{x",  STAT_WIS,  OBJ_VNUM_SCHOOL_MACE,
    { 5671, 8408, 3961, 11216 },  75,  20, 2,  7, 11, TRUE,
    "priest basics", "priest default"
  },
  {
    "Highwayman", "{DH{wwy",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
    { 5730, 8430, 3973, 11217 },  75,  20,  -4,  10, 13, TRUE,
    "highwayman basics", "highwayman default"
  },
  {
    "Knight", "{yK{Dni{x",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
    { 5726, 8422, 3963, 11138 },  75,  20,  -10, 11, 18, FALSE,
    "knight basics", "knight default"
  },
  {
    "Warlock", "{bW{ylk{x",  STAT_WIS,  OBJ_VNUM_SCHOOL_SWORD,
    { 5654, 8415, 3971, 11137 },  75,  20,  -5,  9, 13, TRUE,
    "warlock basics", "warlock default"
  },
/*{
    classname,  whoname,   primeStat, defaultWeapon,
    { Guilds[4] }, maxPrac, thac0_00, thac0_32, minHP, maxHP, ManaUser?,
    "basics groups", "default group"
  },*/
  {
    "Barbarian", "{yB{rar{x",  STAT_CON,  OBJ_VNUM_SCHOOL_AXE,
    { 5656, 8426, 3964, 11141 },  75,  15,  -10,  12, 18, FALSE,
    "barbarian basics", "barbarian default"
  },
  {
    "Mystic", "{mM{Dys{x",  STAT_DEX,  OBJ_VNUM_SCHOOL_SPEAR,
    { 5657, 8428, 3969, 11203 },  75,  10,  -15, 10, 14, TRUE,
    "mystic basics", "mystic default"
  },
  {
    "Druid", "{gD{yru{x",  STAT_INT,  OBJ_VNUM_SCHOOL_STAFF,
    { 5780, 8413, 3962, 20521 },  75,  20,  -1,  8, 13, TRUE,
    "druid basics", "druid default"
  },
  {
    "Inquisitor", "{WInq{x",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
    { 5781, 3960, 8432, 11140 },  75,  20,  -8,  9, 14, TRUE,
    "inquisitor basics", "inquisitor default"
  },
  {
    "Occultist", "{DOcc{x",  STAT_INT,  OBJ_VNUM_SCHOOL_STAFF,
    { 5728, 8419, 3970, 11139 },  75,  20,  -8,  8, 13, TRUE,
    "occultist basics", "occultist default"
  },
/*{
    classname,  whoname,   primeStat, defaultWeapon,
    { Guilds[4] }, maxPrac, thac0_00, thac0_32, minHP, maxHP, ManaUser?,
    "basics groups", "default group"
  },*/
  {
    "Alchemist", "{mA{clc{x",  STAT_WIS,  OBJ_VNUM_SCHOOL_FLAIL,
    { 5672, 8697, 4007, 4007 },  75,  20,  0,  7, 11, TRUE,
    "alchemist basics", "alchemist default"
  },
  {
    "Woodsman", "{yW{gds{x",  STAT_CON,  OBJ_VNUM_SCHOOL_AXE,
    { 3965, 3965, 3965, 3965 },  75,  15,  -71,  10, 15, FALSE,
    "woodsman basics", "woodsman default"
  },
  {
    "unused5", "US5",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
    { 1223, 1223, 1223, 1223 },  75,  20,  -10,  11, 15, FALSE,
    "knight basics", "knight default"
  },
  {
    "unused6", "US6",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
    { 1223, 1223, 1223, 1223 },  75,  20,  -10,  11, 15, FALSE,
    "knight basics", "knight default"
  }
/*{
    classname,  whoname,   primeStat, defaultWeapon,
    { Guilds[4] }, maxPrac, thac0_00, thac0_32, minHP, maxHP, ManaUser?,
    "basics groups", "default group"
  },*/
};

/*
 * Attribute bonus tables.
 */
const struct str_app_type str_app[31] = // Strength
{
  { -5, -4,   0,  0 },  /* 0  */
  { -5, -4,   3,  1 },  /* 1  */
  { -3, -2,   3,  2 },
  { -3, -1,  10,  3 },  /* 3  */
  { -2, -1,  25,  4 },
  { -2, -1,  55,  5 },  /* 5  */
  { -1,  0,  80,  6 },
  { -1,  0,  90,  7 },
  {  0,  0, 100,  8 },
  {  0,  0, 100,  9 },
  {  0,  0, 115, 10 }, /* 10  */
  {  0,  0, 115, 11 },
  {  0,  0, 130, 12 },
  {  0,  0, 130, 13 }, /* 13  */
  {  0,  1, 140, 14 },
  {  1,  1, 150, 15 }, /* 15  */
  {  1,  2, 165, 16 },
  {  2,  3, 180, 22 },
  {  2,  3, 200, 25 }, /* 18  */
  {  3,  4, 225, 30 },
  {  3,  5, 250, 35 }, /* 20  */
  {  4,  6, 300, 40 },
  {  4,  6, 350, 45 },
  {  5,  7, 400, 50 },
  {  5,  8, 450, 55 },
  {  6,  9, 500, 60 }, /* 25   */
  { 10, 13, 600, 65 },
  { 15, 18, 700, 70 }, /* 27 */
  { 21, 24, 800, 75 },
  { 28, 31,1000, 90 },
  { 36, 39,1300,110 } /* 30 */
};

const struct int_app_type int_app[31] = // Intelligence
{
  {  3, 0,  -4  },  /*  0 */
  {  5, 0,  -3  },  /*  1 */
  {  7, 0,  -2  },
  {  8, 0,  -1  },  /*  3 */
  {  9, 0,  -1  },
  { 10, 0,  -1  },  /*  5 */
  { 11, 0,   0  },
  { 12, 0,   0  },
  { 13, 0,   0  },
  { 15, 0,   0  },
  { 17, 0,   0  },  /* 10 */
  { 19, 0,   0  },
  { 22, 0,   0  },
  { 25, 0,   0  },
  { 28, 0,   0  },
  { 31, 0,   1  },  /* 15 */
  { 34, 0,   2  },
  { 37, 0,   2  },
  { 40, 0,   3  },  /* 18 */
  { 44, 0,   3  },
  { 49, 0,   4  },  /* 20 */
  { 55, 0,   4  },
  { 60, 0,   5  },
  { 70, 50,  6  },
  { 80, 100, 7  },
  { 85, 150, 8  }, /* 25 */
  { 85, 250, 9  },
  { 85, 350, 10 },
  { 90, 450, 12 }, /* 28 */
  { 90, 600, 15 },
  { 95, 750, 20 }  /* 30 */
};

const struct wis_app_type wis_app[31] = // Wisdom
{
  {  0, 60  },  /*  0 */
  {  0, 60  },  /*  1 */
  {  0, 60  },
  {  0, 60  },  /*  3 */
  {  0, 70  },
  {  1, 70  },  /*  5 */
  {  1, 70  },
  {  1, 70  },
  {  1, 78  },
  {  1, 78  },
  {  1, 78  },  /* 10 */
  {  1, 78  },
  {  1, 78  },
  {  1, 78  },
  {  1, 78  },
  {  2, 78  },  /* 15 */
  {  2, 78  },
  {  2, 78  },
  {  3, 80  },  /* 18 */
  {  3, 83  },
  {  3, 85  },  /* 20 */
  {  3, 88  },
  {  4, 90  },
  {  4, 93  },
  {  4, 95  },
  {  5, 100 },  /* 25 */
  {  6, 105 },
  {  7, 110 },
  { 10, 115 },
  { 15, 125 },
  { 20, 140 }  /* 30 */
};

const struct dex_app_type dex_app[31] = // Dexterity
{
  {   60, -4, 0 },   /* 0 */
  {   50, -3, 0 },   /* 1 */
  {   50, -3, 0 },
  {   40, -2, 0 },
  {   30, -2, 0 },
  {   20, -1, 0 },   /* 5 */
  {   10, -1, 0 },
  {    0, -1, 0 },
  {    0,  0, 0 },
  {    0,  0, 0},
  {    0,  0, 0 },   /* 10 */
  {    0,  0, 0 },
  {    0,  0, 0 },
  {    0,  0, 0 },
  {    0,  0, 0 },
  { - 10,  1, 0 },   /* 15 */
  { - 15,  1, 0 },
  { - 20,  2, 0 },
  { - 30,  2, 10 },
  { - 40,  3, 25 },
  { - 50,  4, 50 },   /* 20 */
  { - 60,  5, 60 },
  { - 75,  6, 70 },
  { - 90,  7, 80 },
  { -105,  8, 90 },
  { -120,  9, 100 },   /* 25 */
  { -150, 10, 105 },
  { -190, 11, 110 },
  { -240, 12, 125 },
  { -300, 15, 150 },
  { -360, 20, 200 }    /* 30 */
};

const struct con_app_type con_app[31] = // Constitution
{
  { -4, 20, 0  },   /*  0 */
  { -3, 25, 0  },   /*  1 */
  { -2, 30, 0  },
  { -2, 35, 0  },    /*  3 */
  { -1, 40, 0  },
  { -1, 45, 0  },   /*  5 */
  { -1, 50, 0  },
  {  0, 55, 0  },
  {  0, 60, 0  },
  {  0, 65, 0  },
  {  0, 70, 0  },   /* 10 */
  {  0, 75, 0  },
  {  0, 80, 0  },
  {  0, 85, 0  },
  {  0, 88, 0  },
  {  1, 90, 0  },   /* 15 */
  {  2, 95, 0  },
  {  2, 97, 0  },
  {  3, 99, 0  },   /* 18 */
  {  3, 99, 0  },
  {  4, 99, 0  },   /* 20 */
  {  4, 99, 0  },
  {  5, 99, 0  },
  {  6, 99, 50  },
  {  7, 99, 100 },
  {  8, 99, 150 },  /* 25 */
  {  9, 99, 250 },
  { 10, 99, 350 },
  { 11, 99, 450 },
  { 12, 99, 600 },
  { 13, 99, 750 }   /* 30 */
};

/*
 * Liquid properties.
 * Used in world.obj.
 */
const struct liq_type liq_table [MAX_LIQUID] =
{
/*    name              color     { proof, full, thirst, food, ssize } }, */
  { "water",            "clear",        {   0, 1, 10, 0, 16 } },
  { "beer",             "amber",        {  12, 1,  8, 1, 12 } },
  { "red wine",         "burgundy",     {  30, 1,  8, 1,  5 } },
  { "ale",              "brown",        {  15, 1,  8, 1, 12 } },
  { "dark ale",         "dark",         {  16, 1,  8, 1, 12 } },
  { "whiskey",          "golden",       { 120, 1,  5, 0,  2 } },
  { "lemonade",         "pink",         {   0, 1,  9, 2, 12 } },
  { "firebreather",     "boiling",      { 190, 0,  4, 0,  2 } },
  { "local specialty",  "clear",        { 151, 1,  3, 0,  2 } },
  { "slimemold juice",  "green",        {   0, 2, -8, 1,  2 } },
  { "milk",             "white",        {   0, 2,  9, 3, 12 } },
  { "tea",              "tan",          {   0, 1,  8, 0,  6 } },
  { "coffee",           "black",        {   0, 1,  8, 0,  6 } },
  { "blood",            "red",          {   0, 2, -1, 2,  6 } },
  { "salt water",       "clear",        {   0, 1, -2, 0,  1 } },
  { "dr pepper",        "brown",        {   0, 2,  9, 2, 12 } },
  { "root beer",        "brown",        {   0, 2,  9, 2, 12 } },
  { "elvish wine",      "green",        {  35, 2,  8, 1,  5 } },
  { "white wine",       "golden",       {  28, 1,  8, 1,  5 } },
  { "champagne",        "golden",       {  32, 1,  8, 1,  5 } },
  { "mead",             "honey-colored",{  34, 2,  8, 2, 12 } },
  { "rose wine",        "pink",         {  26, 1,  8, 1,  5 } },
  { "benedictine wine", "burgundy",     {  40, 1,  8, 1,  5 } },
  { "vodka",            "clear",        { 130, 1,  5, 0,  2 } },
  { "cranberry juice",  "red",          {   0, 1,  9, 2, 12 } },
  { "orange juice",     "orange",       {   0, 2,  9, 3, 12 } },
  { "absinthe",         "green",        { 200, 1,  4, 0,  2 } },
  { "brandy",           "golden",       {  80, 1,  5, 0,  4 } },
  { "aquavit",          "clear",        { 140, 1,  5, 0,  2 } },
  { "schnapps",         "clear",        {  90, 1,  5, 0,  2 } },
  { "icewine",          "purple",       {  50, 2,  6, 1,  5 } },
  { "amontillado",      "burgundy",     {  35, 2,  8, 1,  5 } },
  { "sherry",           "red",          {  38, 2,  7, 1,  5 } },
  { "framboise",        "red",          {  50, 1,  7, 1,  5 } },
  { "rum",              "amber",        { 151, 1,  4, 0,  2 } },
  { "cordial",          "clear",        { 100, 1,  5, 0,  2 } },
  { "rabbit stew",      "cloudy",       {   0, 2,  3, 5,  4 } },
  { "{gmargarita{x",    "green",        {  20, 0,  3, 1,  5 } },
  { "{yurine{x",        "yellow",       {   0, 2,  8, 0, 12 } },
  { "coke",             "brown",        {   0, 2,  9, 2, 12 } },
  { "hot cocoa",        "brown",        {   0, 2,  9, 2, 12 } },
  { "jaegermeister",    "red",          {  38, 2,  7, 1,  5 } },
  { NULL,               NULL,           {   0, 0,  0, 0,  0 } }
};

/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)  n

struct skill_type skill_table [MAX_SKILL] =
{
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "reserved",                                                // name
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // level
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // rating (learning difficulty)
    0,      TAR_IGNORE,    POS_STANDING,                       // spell_fun, targetType, min_pos
    NULL,      SLOT(0),   0,   0,                              // gsn ptr, #OBJECT-SLOT, min_mana, beats
    "",      "",    ""                                         // dam_msg, wearoff_msg, wearoff_obj_msg
  },
  {
    "acid blast",
    { 30, -1, -1, -1, 32, -1, -1, -1, -1, -1, 36, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_acid_blast,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(1),  20,  12,
    "acid blast",    "!Acid Blast!",    ""
  },
  {
    "armor",
    {  3,  2, -1,  5,  5, -1,  4,  4,  5,  5,  7, -1, -1, -1},
    {  1,  1, -1,  2,  2, -1,  1,  1,  1,  1,  1, -1, -1, -1},
    spell_armor,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(2),   5,  12,
    "",      "You feel less armored.",  ""
  },
  {
    "bless",
    { -1,  7, -1, -1, -1, -1, -1, -1, 10, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1},
    spell_bless,    TAR_OBJ_CHAR_DEF,  POS_STANDING,
    NULL,      SLOT(3),   5,  12,
    "",      "You feel less righteous.", "$p's holy aura fades."
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "blindness",
    { 12,  8, -1, -1, 24, -1, -1, -1, -1,  5, 19, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1, -1, -1, -1,  1,  1, -1, -1, -1},
    spell_blindness,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    &gsn_blindness,    SLOT(4),   5,  12,
    "blindness",      "You can see again.",  ""
  },
  {
    "burning hands",
    {  7, -1, -1, -1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_burning_hands,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(5),  15,  12,
    "burning hands",  "!Burning Hands!",   ""
  },
  {
    "call lightning",
    { -1, 18, -1, -1, -1, -1, -1, 23, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_call_lightning,  TAR_CHAR_OFFENSIVE,    POS_FIGHTING,
    NULL,      SLOT(6),  15,  12,
    "lightning bolt",  "!Call Lightning!",  ""
  },
  {
    "calm",
    { 48, 16, -1, -1, -1, -1, 24, 19, -1, -1, -1, -1, -1, -1},
    {  1,  1, -1, -1, -1, -1,  2,  1, -1, -1, -1, -1, -1, -1},
    spell_calm,    TAR_CHAR_OFFENSIVE,    POS_FIGHTING,
    NULL,      SLOT(7),  30,  12,
    "",      "You have lost your peace of mind.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "cancellation",
    { 18, 26, -1, -1, 20, -1, 23, 20, -1, 23, 25, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1,  2,  2, -1,  1,  1, -1, -1, -1},
    spell_cancellation,  TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(8),  20,  12,
    ""      "!cancellation!",  ""
  },
  {
    "cause critical",
    { -1, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_cause_critical,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(9),  20,  12,
    "cause critical",    "!Cause Critical!",  ""
  },
  {
    "cause light",
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_cause_light,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(10),  15,  12,
    "cause light",    "!Cause Light!",  ""
  },
  {
    "cause serious",
    { -1,  11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_cause_serious,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(11),  17,  12,
    "cause serious",    "!Cause Serious!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "chain lightning",
    { 33, -1, -1, -1, 35, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_chain_lightning,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(12),  25,  12,
    "lightning",    "!Chain Lightning!",  ""
  },
  {
    "change sex",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_change_sex,  TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    &gsn_change_sex,      SLOT(13),  15,  12,
    "",      "Your body feels familiar again.",  ""
  },
  {
    "charm person",
    { 30, 50, -1, -1, -1, -1, -1, 35, -1, 38, -1, -1, -1, -1},
    {  1,  2, -1, -1, -1, -1, -1,  1, -1,  1, -1, -1, -1, -1},
    spell_charm_person,    TAR_CHAR_OFFENSIVE,  POS_STANDING,
    &gsn_charm_person,    SLOT(14),   30,  12,
    "",      "You feel more self-confident.",  ""
  },
  {
    "chill touch",
    {  4, -1, -1, -1,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_chill_touch,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(15),  15,  12,
    "chilling touch",  "You feel less {cc{bo{Wl{cd{x.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "colour spray",
    { 16, -1, -1, -1, 18, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_colour_spray,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(16),  15,  12,
    "colour spray",    "!Colour Spray!",  ""
  },
  {
    "continual light",
    {  6,  4, -1,  9, -1, -1, -1,  1,  9, -1, -1, -1, -1, -1},
    {  1,  1, -1,  2, -1, -1, -1,  1,  2, -1, -1, -1, -1, -1},
    spell_continual_light,  TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(17),   7,  12,
    "",      "!Continual Light!",  ""
  },
  {
    "control weather",
    { -1, 19, -1, -1, -1, -1, -1, 11, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_control_weather,  TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(18),  25,  12,
    "",      "!Control Weather!",  ""
  },
  {
    "create food",
    { 10,  5, -1,  5, -1, -1, -1,  6,  7, -1, -1, -1, -1, -1},
    {  1,  1, -1,  2, -1, -1, -1,  1,  2, -1, -1, -1, -1, -1},
    spell_create_food,  TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(19),   5,  12,
    "",      "!Create Food!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "create rose",
    { 16, 11, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1},
    {  1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_create_rose,  TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(20),  30,   12,
    "",      "!Create Rose!",  ""
  },
  {
    "create spring",
    { 14, 17, -1, -1, -1, -1, -1, 17, -1, -1, -1, -1, -1, -1},
    {  1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_create_spring,  TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(21),  20,  12,
    "",      "!Create Spring!",  ""
  },
  {
    "create water",
    {  8,  3, -1,  5, -1, -1, -1,  5, 16, -1, -1, -1, -1, -1},
    {  1,  1, -1,  2, -1, -1, -1,  1,  2, -1, -1, -1, -1, -1},
    spell_create_water,  TAR_OBJ_INV,    POS_STANDING,
    NULL,      SLOT(22),   5,  12,
    "",      "!Create Water!",  ""
  },
  {
    "cure blindness",
    { -1,  6, -1,  8, -1, -1, -1, -1, 15, -1,  5, -1, -1, -1},
    { -1,  1, -1,  2, -1, -1, -1, -1,  2, -1,  1, -1, -1, -1},
    spell_cure_blindness,  TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(23),   5,  12,
    "",      "!Cure Blindness!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "cure critical",
    { -1, 15, -1, -1, -1, -1, -1, 22, 17, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1,  2,  2, -1, -1, -1, -1, -1},
    spell_cure_critical,  TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(24),  20,  12,
    "",      "!Cure Critical!",  ""
  },
  {
    "cure disease",
    { -1, 13, -1, -1, -1, -1, -1, -1, -1, -1, 23, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_cure_disease,  TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(25),  20,  12,
    "",      "!Cure Disease!",  ""
  },
  {
    "cure light",
    { -1,  1, -1, 15, -1, -1, -1,  3, 8, -1, -1, -1, -1, -1},
    { -1,  1, -1,  2, -1, -1, -1,  2, 2, -1, -1, -1, -1, -1},
    spell_cure_light,  TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(26),  10,  12,
    "",      "!Cure Light!",    ""
  },
  {
    "cure poison",
    { -1, 14, -1, 35, -1, -1, -1, -1, -1, -1, 14, -1, -1, -1},
    { -1,  1, -1,  2, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_cure_poison,  TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(27),   5,  12,
    "",      "!Cure Poison!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "cure serious",
    { -1, 11, -1, -1, -1, -1, -1, -1, 11, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1},
    spell_cure_serious,  TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(28),  15,  12,
    "",      "!Cure Serious!",  ""
  },
  {
    "curse",
    { 18, 18, -1, -1, 19, -1, -1, -1, -1, 8, -1, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1, -1, -1, -1, 1, -1, -1, -1, -1},
    spell_curse,    TAR_OBJ_CHAR_OFF,  POS_FIGHTING,
    &gsn_curse,    SLOT(29),  20,  12,
    "curse",    "The curse wears off.",
    "$p is no longer impure."
  },
  {
    "demonfire",
    { -1, 34, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_demonfire,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(30),  20,  12,
    "torment",    "!Demonfire!",    ""
  },
  {
    "detect evil",
    { 11,  4, -1, -1, -1, -1, -1, -1,  3, -1, -1, -1, -1, -1},
    {  1,  1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1},
    spell_detect_evil,  TAR_CHAR_SELF,    POS_STANDING,
    NULL,      SLOT(31),   5,  12,
    "",      "The {rr{De{rd{x tint in your vision disappears.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "detect good",
    { 11,  4, -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1},
    {  1,  1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_detect_good,      TAR_CHAR_SELF,          POS_STANDING,
    NULL,                   SLOT(32),        5,     12,
    "",                     "The {yg{Wo{yld{x in your vision disappears.",  ""
  },
  {
    "detect hidden",
    { 15, 11, 12, -1, -1, -1, -1, 31, -1, 13, 16, -1, -1, -1},
    {  1,  1,  2, -1, -1, -1, -1,  1, -1,  1,  1, -1, -1, -1},
    spell_detect_hidden,  TAR_CHAR_SELF,    POS_STANDING,
    NULL,      SLOT(33),   5,  12,
    "",      "You feel less aware of your surroundings.",
    ""
  },
  {
    "detect invis",
    {  3,  8,  6, -1,  7, -1, -1, 21, -1,  6,  6, -1, -1, -1},
    {  1,  1,  2, -1,  1, -1, -1,  1, -1,  1,  1, -1, -1, -1},
    spell_detect_invis,  TAR_CHAR_SELF,    POS_STANDING,
    NULL,      SLOT(34),   5,  12,
    "",      "You no longer see {Dinv{cisible{x objects.",
    ""
  },
  {
    "detect magic",
    {  2,  6, -1, -1,  8, -1, -1, -1, -1, -1,  3, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_detect_magic,  TAR_CHAR_SELF,    POS_STANDING,
    NULL,      SLOT(35),   5,  12,
    "",      "You no longer see {mmagic{bal{x auras.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "detect poison",
    { 15,  7,  9, -1, 19, -1, -1, -1, 10, 10,  4, -1, -1, -1},
    {  1,  1,  2, -1,  1, -1, -1, -1,  1,  1,  1, -1, -1, -1},
    spell_detect_poison,  TAR_OBJ_INV,    POS_STANDING,
    NULL,      SLOT(36),   5,  12,
    "",      "!Detect Poison!",  ""
  },
  {
    "dispel magic",
    { 16, 24, -1, -1, 20, -1, -1, 26, -1, 19, 31, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1, -1,  2, -1,  1,  1, -1, -1, -1},
    spell_dispel_magic,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(39),  15,  12,
    "dispel magic",      "!Dispel Magic!",  ""
  },
  {
    "dispel evil",
    { -1, 15, -1, -1, -1, -1, -1, -1, 30, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1},
    spell_dispel_evil,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(37),  15,  12,
    "dispel evil",    "!Dispel Evil!",  ""
  },
  {
    "dispel good",
    { -1, 15, -1, -1, -1, -1, -1, -1, -1, 16, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_dispel_good,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
    NULL,                   SLOT(38),      15,     12,
    "dispel good",          "!Dispel Good!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "earthquake",
    { 10, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_earthquake,  TAR_IGNORE,    POS_FIGHTING,
    NULL,      SLOT(40),  15,  12,
    "earthquake",    "!Earthquake!",    ""
  },
  {
    "enchant armor",
    { 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_enchant_armor,  TAR_OBJ_INV,    POS_STANDING,
    NULL,      SLOT(41),  100,  24,
    "",      "!Enchant Armor!",  ""
  },
  {
    "enchant weapon",
    { 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_enchant_weapon,  TAR_OBJ_INV,    POS_STANDING,
    NULL,      SLOT(42),  100,  24,
    "",      "!Enchant Weapon!",  ""
  },
  {
    "energy drain",
    { 19, -1, -1, -1, 27, -1, -1, -1, -1, 26, -1, -1, -1, -1},
    {  1, -1,  2, -1,  1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_energy_drain,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(43),  35,  12,
    "energy drain",    "!Energy Drain!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "faerie fire",
    { -1,  3, -1, -1, -1, -1, -1, 4, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1},
    spell_faerie_fire,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(44),   5,  12,
    "faerie fire",    "The {mpink{x aura around you fades away.",
    ""
  },
  {
    "faerie fog",
    { -1, 21, -1, -1, -1, -1, -1, 17, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_faerie_fog,  TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(45),  12,  12,
    "faerie fog",    "!Faerie Fog!",    ""
  },
  {
    "farsight",
    { 14, 16, 16, -1, 16, -1, -1, 23, -1, 25, 22, -1, -1, -1},
    {  1,  1,  2, -1,  1, -1, -1,  1, -1,  1,  1, -1, -1, -1},
    spell_farsight,    TAR_CHAR_SELF,    POS_STANDING,
    NULL,      SLOT(46),  25,  20,
    "",    "You are no longer with the farsight.",    ""
  },
  {
    "fireball",
    { 22, -1, -1, -1, 23, -1, -1, -1, -1, -1, 24, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_fireball,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(47),  15,  12,
    "fireball",    "!Fireball!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "fireproof",
    { 13, 12, -1, -1, 15, -1, 12, -1, -1, -1, 25, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1,  2, -1, -1, -1,  1, -1, -1, -1},
    spell_fireproof,  TAR_OBJ_INV,    POS_STANDING,
    NULL,      SLOT(48),  10,  12,
    "",      "",  "$p's protective aura fades."
  },
  {
    "flamestrike",
    { -1, 20, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_flamestrike,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(49),  10,  12,
    "flamestrike",    "!Flamestrike!",    ""
  },
  {
    "fly",
    { 10, 18, -1, -1,  9, -1, 13, 11, -1, -1, 12, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1,  1,  2, -1, -1,  1, -1, -1, -1},
    spell_fly,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(50),  10,  18,
    "",      "You slowly float to the ground.",  ""
  },
  {
    "floating disc",
    {  4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_floating_disc,  TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(51),  40,  24,
    "",      "!Floating disc!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "frenzy",
    { -1, 24, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_frenzy,           TAR_CHAR_DEFENSIVE,     POS_STANDING,
    NULL,                   SLOT(52),      30,     24,
    "",                     "Your rage ebbs.",  ""
  },
  {
    "gate",
    { 27, 17, -1, -1, 25, -1, 30, 15, -1, 28, -1, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1,  1,  1, -1,  1, -1, -1, -1, -1},
    spell_gate,    TAR_IGNORE,    POS_FIGHTING,
    NULL,      SLOT(53),  40,  12,
    "",      "!Gate!",    ""
  },
  {
    "giant strength",
    { 11, -1, 22, -1, -1, -1, -1, 12, -1, -1, -1, -1, -1, -1},
    {  1, -1,  2, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1},
    spell_giant_strength,  TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(54),  20,  12,
    "",      "You feel weaker.",  ""
  },
  {
    "harm",
    { -1, 23, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_harm,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(55),  20,  12,
    "harm",    "!Harm!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "haste",
    { 21, -1, 26, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_haste,    TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(56),  30,  12,
    "",      "You feel yourself slow down.",  ""
  },
  {
    "heal",
    { -1, 21, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_heal,    TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(57),  50,  12,
    "",      "!Heal!",    ""
  },
  {
    "heat metal",
    { -1, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_heat_metal,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(58 ),   25,  18,
    "heat metal",    "!Heat Metal!",    ""
  },
  {
    "holy word",
    { -1, 36, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_holy_word,  TAR_IGNORE,  POS_FIGHTING,
    NULL,      SLOT(59 ),   200,  24,
    "divine wrath",    "!Holy Word!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "identify",
    { 15, 16, 18, -1, 12, -1, -1, 19, -1, 16, 10, -1, -1, -1},
    {  1,  1,  2, -1,  1, -1, -1,  2, -1,  1,  1, -1, -1, -1},
    spell_identify,    TAR_OBJ_INV,    POS_STANDING,
    NULL,      SLOT(60 ),  12,  24,
    "",      "!Identify!",    ""
  },
  {
    "infravision",
    {  9, -1, 10, 15, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    {  1,  1,  2,  2, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_infravision,  TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(61),   5,  18,
    "",      "You no longer see warm-blooded creatures in the dark.",  ""
  },
  {
    "invisibility",
    {  5, -1,  9, -1, -1, -1, -1, -1, -1, 42, -1, -1, -1, -1},
    {  1, -1,  2, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_invis,    TAR_OBJ_CHAR_DEF,  POS_STANDING,
    &gsn_invis,    SLOT(62),   5,  12,
    "",      "You are no longer {Dinv{cisible{x.",
    "$p fades into view."
  },
  {
    "know alignment",
    { 12,  9, -1, -1, 11, -1, -1, 13,  4,  4,  9, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1, -1,  1,  1,  1,  1, -1, -1, -1},
    spell_know_alignment,  TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(63),   9,  12,
    "",      "!Know Alignment!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "lightning bolt",
    { 11,  6, -1, -1, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_lightning_bolt,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(64),  15,  12,
    "lightning bolt",  "!Lightning Bolt!",  ""
  },
  {
    "locate object",
    {  9, 15, 11, -1, 13, -1, -1, -1, -1, -1, 34, -1, -1, -1},
    {  1,  1,  2, -1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_locate_object,  TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(65),  20,  18,
    "",      "!Locate Object!",  ""
  },
  {
    "magic missile",
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_magic_missile,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(66),  15,  12,
    "magic missile",  "!Magic Missile!",  ""
  },
  {
    "mass healing",
    { -1, 38, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_mass_healing,  TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(67),  100,  36,
    "",      "!Mass Healing!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "mass invis",
    { 22, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_mass_invis,  TAR_IGNORE,    POS_STANDING,
    &gsn_mass_invis,  SLOT(68),  20,  24,
    "",      "You are no longer {Dinv{cisible{x.",    ""
  },
  {
    "nexus",
    { 40, 35, -1, -1, 35, -1, -1, -1, -1, 45, -1, -1, -1, -1},
    {  2,  2, -1, -1,  1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_nexus,            TAR_IGNORE,             POS_STANDING,
    NULL,                   SLOT(69),       150,   36,
    "",                     "!Nexus!",    ""
  },
  {
    "pass door",
    { 24, 32, -1, -1, 22, -1, 28, 28, -1, -1, 20, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1,  1,  1, -1, -1,  1, -1, -1, -1},
    spell_pass_door,  TAR_CHAR_SELF,    POS_STANDING,
    NULL,      SLOT(70),  20,  12,
    "",      "You feel solid again.",  ""
  },
  {
    "plague",
    { -1, 17, -1, -1, 20, -1, -1, -1, -1, 14, -1, -1, -1, -1},
    { -1,  1, -1, -1,  1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_plague,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    &gsn_plague,    SLOT(71),  20,  12,
    "plague",    "Your {Ds{ro{gres{x vanish.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "poison",
    { -1, 12, -1, -1, 21, -1, -1, -1, -1, 10, 17, -1, -1, -1},
    { -1,  1, -1, -1,  1, -1, -1, -1, -1,  1,  1, -1, -1, -1},
    spell_poison,    TAR_OBJ_CHAR_OFF,  POS_FIGHTING,
    &gsn_poison,    SLOT(72),  10,  12,
    "poison",    "You feel less {cs{gick{x.",
    "The {gpoi{Gs{con{x on $p dries up."
  },
  {
    "portal",
    { 35, 30, -1, -1, 32, -1, 40, -1, -1, -1, -1, -1, -1, -1},
    {  2,  2, -1, -1,  1, -1,  3, -1, -1, -1, -1, -1, -1, -1},
    spell_portal,           TAR_IGNORE,             POS_STANDING,
    NULL,                   SLOT(73),       80,     24,
    "",                     "!Portal!",    ""
  },
  {
    "protection evil",
    { 12,  9, -1, -1, 13, -1, -1, -1, 12, -1, -1, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1, -1, -1},
    spell_protection_evil,  TAR_CHAR_SELF,    POS_STANDING,
    NULL,      SLOT(74),   5,  12,
    "",      "You feel less protected.",  ""
  },
  {
    "protection good",
    { 12,  9, -1, -1, 13, -1, -1, -1, -1, 12, -1, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_protection_good,  TAR_CHAR_SELF,          POS_STANDING,
    NULL,                   SLOT(75),       5,     12,
    "",                     "You feel less protected.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "ray of truth",
    { -1, 35, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_ray_of_truth,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
    NULL,                   SLOT(76),      20,     12,
    "ray of truth",  ""
  },
  {
    "recharge",
    {  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_recharge,    TAR_OBJ_INV,    POS_STANDING,
    NULL,      SLOT(77),  60,  24,
    "",      "!Recharge!",    ""
  },
  {
    "refresh",
    {  8,  5, 12, 10, -1, -1, -1, 11, -1, -1, -1, -1, -1, -1},
    {  1,  1,  2,  2, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_refresh,    TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(78),  12,  18,
    "refresh",    "!Refresh!",    ""
  },
  {
    "remove curse",
    { -1, 18, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_remove_curse,  TAR_OBJ_CHAR_DEF,  POS_STANDING,
    NULL,      SLOT(79),   5,  12,
    "",      "!Remove Curse!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "sanctuary",
    { -1, 20, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_sanctuary,  TAR_CHAR_DEFENSIVE,  POS_STANDING,
    &gsn_sanctuary,    SLOT(80),  75,  12,
    "",      "The {Wwhite{x aura around your body fades.",
    ""
  },
  {
    "shield",
    { 20, 35, -1, -1, 16, -1, 18, 19, 40, 20, -1, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1,  1,  2,  1,  1, -1, -1, -1, -1},
    spell_shield,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(81),  12,  18,
    "",      "Your force shield shimmers then fades away.",
    ""
  },
  {
    "shocking grasp",
    {  3, -1, -1, -1, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_shocking_grasp,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(82),  15,  12,
    "shocking grasp",  "!Shocking Grasp!",  ""
  },
  {
    "sleep",
    { 10, 24, -1, -1, -1, -1, 12, 12, -1, 29, -1, -1, -1, -1},
    {  1,  2, -1, -1, -1, -1,  1,  1, -1,  1, -1, -1, -1, -1},
    spell_sleep,    TAR_CHAR_OFFENSIVE,  POS_STANDING,
    &gsn_sleep,    SLOT(83),  15,  12,
    "sleep",      "You feel less tired.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "slow",
    { 23, -1, -1, -1, 22, -1, -1, -1, -1, 21, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_slow,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
    NULL,                   SLOT(84),      30,     12,
    "slow",                     "You feel yourself speed up.",  ""
  },
  {
    "stone skin",
    { 25, -1, -1, -1, 17, -1, 29, 27, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1,  1,  1, -1, -1, -1, -1, -1, -1},
    spell_stone_skin,  TAR_CHAR_SELF,    POS_STANDING,
    NULL,      SLOT(85),  12,  18,
    "",      "Your skin feels soft again.",  ""
  },
  {
    "summon",
    { 35, 12, -1, -1, 35, -1, -1, 19, 35, 34, -1, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1, -1,  1,  1,  1, -1, -1, -1, -1},
    spell_summon,    TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(86),  50,  12,
    "",      "!Summon!",    ""
  },
  {
    "teleport",
    { 13, 22, -1, -1, 18, -1, -1, 17, -1, -1, -1, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_teleport,    TAR_CHAR_OFFENSIVE,    POS_FIGHTING,
    NULL,       SLOT(87),  35,  12,
    "",      "!Teleport!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "ventriloquate",
    {  1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_ventriloquate,  TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(88),   5,  12,
    "",      "!Ventriloquate!",  ""
  },
  {
    "weaken",
    { 11, -1, -1, -1, 15, -1, -1, -1, -1, 11, 11, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1,  1,  1, -1, -1, -1},
    spell_weaken,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    &gsn_weaken,      SLOT(89),  20,  12,
    "weaken",    "You feel stronger.",  ""
  },
  {
    "word of recall",
    { 32, 28, -1, -1, 28, -1, -1, 27, -1, 32, 27, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1, -1,  1, -1,  1,  1, -1, -1, -1},
    spell_word_of_recall,  TAR_CHAR_SELF,    POS_RESTING,
    NULL,      SLOT(90),   20,  12,
    "",      "!Word of Recall!",  ""
  },
  {
    "acid breath",
    { 31, -1, -1, -1, 32, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_acid_breath,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    &gsn_acid_breath,      SLOT(91),  50,  24,
    "blast of acid",  "!Acid Breath!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "fire breath",
    { 40, -1, -1, -1, 41, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_fire_breath,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(92),  100,  24,
    "blast of flame",  "The {Ds{wm{Doke{x leaves your eyes.",  ""
  },
  {
    "frost breath",
    { 34, -1, -1, -1, 35, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_frost_breath,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(93),  75,  24,
    "blast of frost",  "!Frost Breath!",  ""
  },
  {
    "gas breath",
    { 39, -1, -1, -1, 40, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_gas_breath,  TAR_IGNORE,    POS_FIGHTING,
    NULL,      SLOT(94),  100,  24,
    "blast of gas",    "!Gas Breath!",    ""
  },
  {
    "lightning breath",
    { 37, -1, -1, -1, 38, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_lightning_breath,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(95),  75,  24,
    "blast of lightning",  "!Lightning Breath!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "general purpose",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_general_purpose,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
    NULL,                   SLOT(96),      50,      12,
    "general purpose ammo", "!General Purpose Ammo!",  ""
  },
  {
    "judgement",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_high_explosive,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
    NULL,                   SLOT(97),      50,      12,
    "judgement",  "!High Explosive Ammo!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */

/* combat and weapons skills */
  {
    "axe",
    { -1, -1, -1, 1, 1, 1, -1, -1, 1, -1, -1,  1, -1, -1},
    { -1, -1, -1, 4, 5, 5, -1, -1, 5, -1, -1,  3, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_axe,              SLOT( 0),       0,      0,
    "",                     "!Axe!",    ""
  },
  {
    "dagger",
    { 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, -1, -1},
    { 2,  2, 2, 2, 2, 2, 2, 2, 3, 3, 3,  4, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_dagger,            SLOT( 0),       0,      0,
    "",                     "!Dagger!",    ""
  },
  {
    "flail",
    { 1, 1, -1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1},
    { 6, 3, -1, 4, 5, 4, 4, 4, 4, 4, 3, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_flail,              SLOT( 0),       0,      0,
    "",                     "!Flail!",    ""
  },
  {
    "mace",
    { 1, 1, 1, 1, 1, 1, -1, 1, 1, 1, -1, -1, -1, -1},
    { 5, 2, 3, 3, 3, 3, -1, 3, 3, 3, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_mace,              SLOT( 0),       0,      0,
    "",                     "!Mace!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "polearm",
    { -1, -1, -1, 1, 1, 1, -1, 1, 1, -1, -1, -1, -1, -1},
    { -1, -1, -1, 4, 5, 4, -1, 5, 4, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_polearm,           SLOT( 0),       0,      0,
    "",                     "!Polearm!",    ""
  },
  {
    "shield block",
    {  1, 1, 1, 1, -1, -1, -1, 5, 1, 1, 5, -1, -1, -1},
    {  4, 4, 6, 2, -1, -1, -1, 5, 3, 3, 5, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_shield_block,  SLOT(0),  0,  0,
    "",      "!Shield!",    ""
  },
  {
    "spear",
    { -1, -1, -1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, 3, 4, 3, 3, 2, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_spear,              SLOT( 0),       0,      0,
    "",                     "!Spear!",    ""
  },
  {
    "sword",
    { -1, -1, 1, 1, 1,  1, 1, -1, 1, 1, -1,  1, -1, -1},
    { -1, -1, 3, 2, 2,  2, 1, -1, 2, 2, -1,  2, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_sword,              SLOT( 0),       0,      0,
    "",                     "!sword!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "exotic",
    { 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1,  1, -1, -1},
    { 5, 5, 3, 2, 2, 4, 3, 5, -1, -1, -1,  3, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_exotic,              SLOT( 0),       0,      0,
    "",                     "!exotic!",    ""
  },
  {
    "whip",
    { -1, 1, 1, 1, 1, 1, -1, 1, -1, -1, 1, -1, -1, -1},
    { -1, 5, 5, 4, 4, 5, -1, 4, -1, -1, 4, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_whip,              SLOT( 0),       0,      0,
    "",                     "!Whip!",  ""
  },
  {
    "backstab",
    { -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_STANDING,
    &gsn_backstab,          SLOT( 0),        0,     24,
    "backstab",             "!Backstab!",    ""
  },
  {
    "bash",
    { -1, -1, -1,  1, -1, 1, -1, -1,  9, -1, -1, 11, -1, -1},
    { -1, -1, -1,  4, -1, 6, -1, -1,  5, -1, -1,  6, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_bash,              SLOT( 0),       0,      24,
    "bash",                 "!Bash!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "stake",
    { -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_stake,              SLOT( 0),       0,      24,
    "stake",                 "!Stake!",    ""
  },
  {
    "berserk",
    { -1, -1, -1,  8, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1,  6, -1,  6, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_berserk,          SLOT( 0),       0,      24,
    "",                     "You feel your pulse slow down.",  ""
  },
  {
    "rampage",
    { -1, -1, -1, -1, -1, 22, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1,  6, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_rampage,           SLOT( 0),       0,      24,
    "rampage",              "!Rampage!",          ""
  },
  {
    "dirt kicking",
    { -1, -1, 3, 3, -1, 8, -1, 10, 13, -1, -1,  2, -1, -1},
    { -1, -1, 4, 4, -1, 7, -1,  6,  5, -1, -1,  3, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_dirt,    SLOT( 0),  0,  24,
    "kicked dirt",    "You rub the dirt out of your eyes.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "disarm",
    { -1, -1, 12, 11, 12, 12, -1, -1, 12, -1, -1, 30, -1, -1},
    { -1, -1,  6,  4,  5,  7, -1, -1,  4, -1, -1,  5, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_disarm,            SLOT( 0),        0,     24,
    "",                     "!Disarm!",    ""
  },
  {
    "dodge",
    { 20, 22, -1, 13, 18, 21, -1,  16, 15, 15, 23, 17, -1, -1},
    {  8,  8, -1,  6,  7,  6, -1,   6,  7,  7,  7,  6, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_dodge,             SLOT( 0),        0,     0,
    "",                     "!Dodge!",    ""
  },
  {
    "enhanced damage",
    { -1, -1, -1,  1, -1, 1, -1, -1, -1, -1, -1, 15, -1, -1},
    { -1, -1, -1,  3, -1, 5, -1, -1, -1, -1, -1,  5, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_enhanced_damage,   SLOT( 0),        0,     0,
    "",                     "!Enhanced Damage!",  ""
  },
  {
    "envenom",
    { -1, -1, 10, -1 , -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1,  4, -1 , -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_OBJ_INV,      POS_RESTING,
    &gsn_envenom,    SLOT(0),  0,  24,
    "",      "!Envenom!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "toxicology",
    { -1, -1, 32, -1 , -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1,  5, -1 , -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_OBJ_INV,      POS_RESTING,
    &gsn_toxicology,    SLOT(0),  0,  0,
    "",      "!Toxicology!",    ""
  },
  {
    "hand to hand",
    { 10, 10, 15, 6, 7, 1,  1, 10,  5,  6, 12,  8, -1, -1},
    {  8,  5,  6, 4, 4, 5,  3,  4,  4,  4,  7,  4, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_hand_to_hand,  SLOT( 0),  0,  0,
    "clenched fist",      "!Hand to Hand!",  ""
  },
  {
    "kick",
    { -1, -1, 14, 8, 10, 15, 3, 18, 11, 11, 21,  6, -1, -1},
    { -1, -1,  6, 3,  4,  4, 4,  5,  3,  3,  6,  5, -1, -1},
    spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
    &gsn_kick,              SLOT( 0),        0,     12,
    "kick",                 "!Kick!",    ""
  },
  {
    "parry",
    { -1, -1, -1, 1, 7, -1, -1, -1,  1, -1, -1, -1, -1, -1},
    { -1, -1, -1, 4, 4, -1, -1, -1,  4, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_parry,             SLOT( 0),        0,     0,
    "",                     "!Parry!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "sidestep",
    { -1, -1, -1, -1, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, 6, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_sidestep,          SLOT( 0),        0,     0,
    "",                     "!Sidestep!",    ""
  },
  {
    "blade weave",
    { -1, -1, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_blade_weave,       SLOT( 0),        0,     0,
    "",                     "!Blade Weave!",    ""
  },
  {
    "rescue",
    { -1, -1, -1, 1, -1, -1, -1, -1, 53, -1, -1, 28, -1, -1},
    { -1, -1, -1, 4, -1, -1, -1, -1,  6, -1, -1,  5, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_rescue,            SLOT( 0),        0,     12,
    "",                     "!Rescue!",    ""
  },
  {
    "trip",
    { -1, -1, 1, 15, -1, 25, -1, -1, -1, -1, -1,  7, -1, -1},
    { -1, -1, 4,  8, -1,  9, -1, -1, -1, -1, -1,  8, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_trip,    SLOT( 0),  0,  24,
    "trip",      "!Trip!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "second attack",
    { 30, 24, 11, 5, 8, 6, 6, 8,  6,  6, 30, 50, -1, -1},
    {  8,  8,  5, 3, 4, 4, 4, 4,  3,  3,  6,  5, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_second_attack,     SLOT( 0),        0,     0,
    "",                     "!Second Attack!",  ""
  },
  {
    "third attack",
    { -1, -1, -1, 12, 20, 35, 38, -1, 18, -1, -1, -1, -1, -1},
    { -1, -1, -1,  4,  7,  5,  6, -1,  4, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_third_attack,      SLOT( 0),        0,     0,
    "",                     "!Third Attack!",  ""
  },
  {
    "fourth attack",
    { -1, -1, -1, 16, -1, -1, -1, -1, 26, -1, -1, -1, -1, -1},
    { -1, -1, -1,  6, -1, -1, -1, -1,  7, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_fourth_attack,      SLOT( 0),        0,     0,
    "",                     "!Fourth Attack!",  ""
  },
  {
    "fast healing",
    { 15, 9, 14, 6, 8, 4, 2, 5,  8,  8, 29, 10, -1, -1},
    {  8, 5,  6, 4, 5, 5, 4, 5,  4,  4,  7,  6, -1, -1},
    spell_null,    TAR_IGNORE,    POS_SLEEPING,
    &gsn_fast_healing,  SLOT( 0),  0,  0,
    "",      "!Fast Healing!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "haggle",
    { 7, 18, 1, 14, 20, 35, 23, 18, 14, 14, 14, -1, -1, -1},
    { 5,  8, 3,  6,  8, 12,  6,  5,  7,  7,  8, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_RESTING,
    &gsn_haggle,    SLOT( 0),  0,  0,
    "",      "!Haggle!",    ""
  },
  {
    "hide",
    { -1, -1, 1, 12, 7, -1, 15, 12, -1, -1,  9, 14, -1, -1},
    { -1, -1, 4,  6, 7, -1,  5,  4, -1, -1,  6,  5, -1, -1},
    spell_null,    TAR_IGNORE,    POS_RESTING,
    &gsn_hide,    SLOT( 0),   0,  12,
    "",      "!Hide!",    ""
  },
  {
    "cloak of assassins", /* advanced hiding */
    { -1, -1, 37, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_RESTING,
    &gsn_cloak_of_assassins,    SLOT( 0),   0,  12,
    "",      "!Cloak of Assassins!",    ""
  },
  {
    "lore",
    { -1, -1, -1, 20, 23, 18, -1, -1, 22, -1, -1, 36, -1, -1},
    { -1, -1, -1,  8,  2,  7, -1, -1,  8, -1, -1,  6, -1, -1},
    spell_null,    TAR_IGNORE,    POS_RESTING,
    &gsn_lore,    SLOT( 0),  0,  36,
    "",      "!Lore!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "meditation",
    { 6, 6, 15, 15, 10, -1, 10, 7, 24, 24,  8, -1, -1, -1},
    { 5, 5,  8,  8,  6, -1,  4, 8,  7,  7,  6, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_SLEEPING,
    &gsn_meditation,  SLOT( 0),  0,  0,
    "",      "Meditation",    ""
  },
  {
    "peek",
    { -1, -1, 2, -1, -1, -1, 25, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, 3, -1, -1, -1,  6, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_peek,    SLOT( 0),   0,   0,
    "",      "!Peek!",    ""
  },
  {
    "pick lock",
    { -1, -1, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_pick_lock,    SLOT( 0),   0,  12,
    "",      "!Pick!",    ""
  },
  {
    "sneak",
    { -1, -1, 4, -1, -1, -1, 15, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, 4, -1, -1, -1,  9, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_sneak,    SLOT( 0),   0,  12,
    "",      "You no longer feel stealthy.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "steal",
    { -1, -1, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_steal,    SLOT( 0),   0,  24,
    "",      "!Steal!",    ""
  },
  {
    "forage",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  6, -1, -1},
    spell_null,             TAR_IGNORE,             POS_STANDING,
    &gsn_forage,          SLOT( 0),        0,     12,
    "forage",             "!Forage!",    ""
  },
  {
    "scrolls",
    { 1, 1, 1, 1, 1, 1, 1,  1,  1,  1,  1,  1, -1, -1},
    { 2, 3, 5, 8, 4, 9, 4,  7,  7,  7,  3,  7, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_scrolls,    SLOT( 0),  0,  24,
    "",      "!Scrolls!",    ""
  },
  {
    "staves",
    { 1, 1, 1, 1, 1, 1, 1,  1,  1,  1,  1,  1, -1, -1},
    { 2, 3, 5, 8, 4, 9, 4,  3,  7,  7,  2,  8, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_staves,    SLOT( 0),  0,  12,
    "",      "!Staves!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "wands",
    { 1, 1, 1, 1, 1,  1, 1, 1,  1,  1,  1,  1, -1, -1},
    { 2, 3, 5, 8, 4, 11, 4, 3,  7,  7,  2,  9, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_wands,    SLOT( 0),  0,  12,
    "",      "!Wands!",    ""
  },
  {
    "recall",
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1},
    { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_recall,    SLOT( 0),  0,  12,
    "",      "!Recall!",    ""
  },
  {
    "drain blade",
    { 30, -1, -1, -1, 41, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_drain_blade,  TAR_OBJ_INV,    POS_STANDING,
    NULL,                   SLOT(98),       15,     12,
    "",      "!Drain Blade!",  "$p looks less {Dm{Ra{Dle{rv{wo{Dlent{x."
  },
  {
    "shocking blade",
    { 31, -1, -1, -1, 40, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_shocking_blade,  TAR_OBJ_INV,    POS_STANDING,
    NULL,                   SLOT(99),       15,     12,
    "",      "!Shocking Blade!",  "$p loses its {ye{Yl{yec{Wtr{wi{yc{Ya{yl{x charge."
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "flame blade",
    { 32, -1, -1, -1, 39, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_flame_blade,  TAR_OBJ_INV,    POS_STANDING,
    NULL,                   SLOT(100),       15,     12,
    "",      "!Flame Blade!",  "$p loses its {yf{Ri{rery{x glow."
  },
  {
    "frost blade",
    { 33, -1, -1, -1, 38, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_frost_blade,  TAR_OBJ_INV,    POS_STANDING,
    NULL,                   SLOT(101),       15,     12,
    "",      "!Frost Blade!",  "$p loses its {wi{cc{by{x enchantment."
  },
  {
    "sharp blade",
    { 38, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_sharp_blade,  TAR_OBJ_INV,    POS_STANDING,
    NULL,                   SLOT(102),       15,     12,
    "",      "!Sharp Blade!",  "$p loses its {we{Wdg{we{x."
  },
  {
    "vorpal blade",
    { 65, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_vorpal_blade,  TAR_OBJ_INV,    POS_STANDING,
    NULL,                   SLOT(103),       15,     12,
    "",      "!Vorpal Blade!",  "$p loses its {mm{ba{mgical{x strength."
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "sharpen",
    { -1, -1, -1, 28, -1, 32, -1, -1, -1, -1, -1, 19, -1, -1},
    { -1, -1, -1,  1, -1,  1, -1, -1, -1, -1, -1,  1, -1, -1},
    spell_null,    TAR_OBJ_INV,    POS_STANDING,
    &gsn_sharpen,      SLOT(0),      0,     12,
    "",      "!Sharpen!",    "$p loses its sharpness."
  },
  {
    "erase",
    { 10, -1, -1, -1, 14, -1, -1, 15, -1, -1, 27, -1, -1, -1},
    {  5, -1, -1, -1,  6, -1, -1,  5, -1, -1,  4, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_erase,             SLOT(0),        0,     12,
    "",      "!erase!",  ""
  },
  {
    "empty",
    { -1, 10, -1, -1, -1, -1, -1, 17, -1, -1,  7, -1, -1, -1},
    { -1,  8, -1, -1, -1, -1, -1,  5, -1, -1,  4, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_empty,             SLOT(0),        0,     12,
    "",      "!empty!",  ""
  },
  {
    "butcher",
    { -1, -1, -1, 10, -1, 12, -1, 2, 10, -1, -1, 22, -1, -1},
    { -1, -1, -1,  4, -1,  3, -1, 3,  5, -1, -1,  4, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_butcher,    SLOT( 0),   0,  24,
    "",      "!Butcher!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "brew",
    { -1,35,-1,-1,-1,-1,-1, 19,-1, -1,  1, -1, -1, -1},
    { -1, 7,-1,-1,-1,-1,-1,  5,-1, -1,  4, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_brew,    SLOT(0),  0,   24,
    "",      "!Brew!",    ""
  },
  {
    "craft",
    { -1,-1,-1,-1,38,-1,-1, -1,-1, 32, 18, -1, -1, -1},
    { -1,-1,-1,-1, 8,-1,-1, -1,-1,  6,  5, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_craft,    SLOT(0),  0,   24,
    "",      "!Craft!",    ""
  },
  {
    "scribe",
    { 27,-1,-1,-1,36,-1,-1, 21,-1, -1, 25, -1, -1, -1},
    {  8,-1,-1,-1,10,-1,-1,  5,-1, -1,  4, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_scribe,    SLOT(0),  0,   24,
    "",      "!Scribe!",    ""
  },
  {
    "repair",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_RESTING,
    &gsn_repair,          SLOT( 0),        0,     12,
    "repair",             "!Repair!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "create fountain",
    { 24, 19, -1, -1, -1, -1, -1, 10, -1, -1, -1, -1, -1, -1},
    {  1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_create_fountain,  TAR_IGNORE,    POS_STANDING,
    NULL,                   SLOT(104),       25,     15,
    "",      "!Create Fountain!",  "$p dries up."
  },
  {
    "incinerate",
    { 40, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_incinerate,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(105),  50,  24,
    "incinerate",    "!Incinerated!",    ""
  },
  {
    "icicle",
    { 28, -1, -1, -1, 31, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_icicle,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(106),  20,  12,
    "icicle",    "You no longer feel like an {Bic{ci{Bc{cle{x.",    ""
  },
  {
    "bigby bash",
    { 25, -1, 30, -1, -1, -1, 23, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1,  2, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1},
    spell_bigby_bash,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(107),  15,  12,
    "Bigby's bash",    "!Bigby's Bash!",      ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "flame aura",
    { 28,  27, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1,   1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_flame_aura,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(108),   50,  12,
    "",      "You feel less resistant to {rf{yl{Ra{rme{x.",  ""
  },
  {
    "frost aura",
    { 31,  28, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1,   1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_frost_aura,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 109),   50,  12,
    "",      "You feel less resistant to the {Wc{Co{Bl{bd{x.",  ""
  },
  {
    "electric aura",
    { 32,  29, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1,   1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_electric_aura,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 110),   50,  12,
    "",      "You feel less resistant to {yl{Wi{ygh{Yt{Wn{yin{Wg{x.",  ""
  },
  {
    "corrosive aura",
    { 33,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_corrosive_aura,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 111),   50,  12,
    "",      "You feel less resistant to {ca{gc{Ci{cd{x.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "arcane aura",
    { 31,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_arcane_aura,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 112),   50,  12,
    "",      "You feel less resistant to {Bmag{mic{x.",  ""
  },
  {
    "holy aura",
    { -1,  31, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,   1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_holy_aura,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 112),   50,  12,
    "",      "You are no longer protected by a {yh{Wo{yly{x aura.",  ""
  },
  {
    "dark aura",
    { -1,  32, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,   1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_dark_aura,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 114),   50,  12,
    "",      "You are no longer protected by a {rda{Dr{rk{x aura.",  ""
  },
  {
    "deter",
    { 20,  -1, 22, -1, -1, -1, -1, -1, -1, 36, -1, -1, -1, -1},
    {  1,  -1,  1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_deter,    TAR_CHAR_SELF,  POS_STANDING,
    NULL,      SLOT( 115),   25,  12,
    "",      "You feel scared of hostile creatures.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "aid",
    { -1,  -1, -1,  4, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1},
    { -1,  -1, -1,  1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1},
    spell_aid,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 116),   20,  12,
    "",      "You feel not so aided anymore.",  ""
  },
  {
    "metamorphose",
    { -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_metamorphose,    TAR_CHAR_OFFENSIVE,  POS_STANDING,
    NULL,      SLOT( 117),  65,  12,
    "",      "You feel normal again.",  ""
  },
  {
    "betray",
    { 30,  -1, 35, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1,  -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_betray,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT( 118),  30,  12,
    "betray",      "!Betray.!",  ""
  },
  {
    "quench",
    { 30,  30, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1,   1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_quench,    TAR_IGNORE,  POS_STANDING,
    NULL,      SLOT( 119),  20,  12,
    "",        ".",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "sate",
    { 30,  30, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1,   1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_sate,    TAR_IGNORE,  POS_STANDING,
    NULL,      SLOT( 120),  20,  12,
    "",        "",    ""
  },
  {
    "resurrect",
    { -1,  -1, -1, -1, 47, -1, -1, -1, -1, 48, -1, -1, -1, -1},
    { -1,  -1, -1, -1,  2, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_resurrect,    TAR_OBJ_INV,  POS_STANDING,
    NULL,      SLOT( 121),  65,  12,
    "",        "",    ""
  },
  {
    "embalm",
    { -1,  -1, -1, -1, 55, -1, -1, -1, -1,  3, -1, -1, -1, -1},
    { -1,  -1, -1, -1,  2, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_make_bag,  TAR_OBJ_INV,    POS_STANDING,
    NULL,      SLOT(122),  20,   24,
    "",      "!Make Bag!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "fear",
    { 35,  -1, -1, -1, -1, -1, -1, -1, -1, 37, -1, -1, -1, -1},
    {  1,  -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_fear,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT( 123),  25,  12,
    "fear",      "!Fear!",  ""
  },
  {
    "fumble",
    { -1,  -1, -1, -1, -1, -1, 23, -1, -1, -1, -1, -1, -1, -1},
    { -1,  -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1},
    spell_fumble,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT( 124),  25,  12,
    "fumble",      "!Fumble!",  ""
  },
  {
    "counter",
    { -1,  -1, -1, 32, -1, -1, 40, -1, 38, -1, -1, 56, -1, -1},
    { -1,  -1, -1,  5, -1, -1,  6, -1,  6, -1, -1,  6, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_counter,           SLOT( 0),       0,      0,
    "counterattack",        "!Counter!",   ""
  },
  {
    "find familiar",
    { -1,  -1, -1, -1, -1, -1, -1, 3, -1, -1, -1,  5, -1, -1},
    { -1,  -1, -1, -1, -1, -1, -1, 2, -1, -1, -1,  5, -1, -1},
    spell_null,  TAR_IGNORE,    POS_RESTING,
    &gsn_familiar,  SLOT(0),  0,  0,
    "",    "!Find Familiar!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "deathgrip",
    { -1,  -1, 45, -1, -1, 40, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  -1,  6, -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_RESTING,
    NULL,                   SLOT( 0),       0,      12,
    "",                     "The {Dd{wa{Drk{x shroud leaves your hands.",   ""
  },
  {
    "critical strike",
    { -1,  -1, -1, 41, -1, 22, -1, -1, -1, -1, -1, 44, -1, -1},
    { -1,  -1, -1,  7, -1,  6, -1, -1, -1, -1, -1,  5, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_critical,          SLOT( 0),       0,      0,
    "",                     "!Critical Strike!",   ""
  },
  {
    "tithe",
    { -1, 23, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,        TAR_IGNORE,     POS_STANDING,
    &gsn_tithe,        SLOT(0),   5,      12,
    "",                     "!Tithe!",    ""
  },
  {
    "whirlwind",
    { -1,  -1, -1, 34, -1, 23, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  -1, -1,  8, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_whirlwind,  SLOT( 0),  0,  36,
    "whirlwind",    "!Whirlwind!",     ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "martyr",
    { -1,  10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,   1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_martyr,    TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    &gsn_martyr,      SLOT( 125),   40,  12,
    "sacrifice",      "You feel less like dying.",  ""
  },
  {
    "life transfer",
    { -1,  27, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,   1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_life_transfer,  TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    NULL,      SLOT( 126),   5,  12,
    "",      "",  ""
  },
  {
    "moon armor",
    { -1, 40, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_moon_armor,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 127),   30,  12,
    "",      "You are no longer bathed in {cm{Do{Bo{cn{Wlight{x.",  ""
  },
  {
    "tumbling",
    { -1,  -1, -1, -1, -1, -1, 5, -1, -1, -1, -1, -1, -1, -1},
    { -1,  -1, -1, -1, -1, -1, 5, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_tumble,  SLOT( 0),  0,  12,
    "tumbling",    "!Tumbling!",     ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "phoenix",
    { -1,  -1, -1, -1, -1, -1, -1, 44, -1, -1, -1, -1, -1, -1},
    { -1,  -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_phoenix,    TAR_IGNORE,  POS_FIGHTING,
    NULL,      SLOT( 128),   40,  12,
    "phoenix",      "!Phoenix!",  ""
  },
  {
    "circle",
    { -1,  -1, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_circle,  SLOT( 0),  0,  24,
    "circle",    "!Circle!",     ""
  },
  {
    "shatter",
    { -1, -1, -1, -1, -1, -1, 41, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  9, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_shatter,            SLOT( 0),        0,     60,
    "",                     "!Shatter!",    ""
  },
  {
    "bind",
    { -1, -1, -1, -1, -1, -1, 12, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  3, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_CHAR_DEFENSIVE,             POS_STANDING,
    &gsn_bind,            SLOT( 0),        0,     24,
    "bind",       "Your bandages are no longer usable.",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "water walk",
    {  8,  -1, -1, -1, -1, -1, -1, 12, -1, -1, -1, -1, -1, -1},
    {  1,  -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_water_walk,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 129),   5,  12,
    "",      "Your feet are no longer {Bbouyant{x.",  ""
  },
  {
    "regeneration",
    { -1,  14, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,   1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_regeneration,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 130),   40,  12,
    "",      "You are no longer healing so fast.",  ""
  },
  {
    "ionwave",
    { 38,  -1, -1, -1, 40, -1, -1, -1, -1, -1, 43, -1, -1, -1},
    {  1,  -1, -1, -1,  2, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_ionwave,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT( 131),   25,  12,
    "ionwave",      "!Ionwave!",  ""
  },
  {
    "holy bolt",
    { -1,  1, -1, -1, -1, -1, -1, -1, 14, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1,  3, -1, -1, -1, -1, -1},
    spell_holy_bolt,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT( 132),   15,  12,
    "holy bolt",  "!Holy bolt!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "vaccine",
    { -1,  11, -1, -1, -1, -1, -1, -1,  6, -1, -1, -1, -1, -1},
    { -1,   1, -1, -1, -1, -1, -1, -1,  3, -1, -1, -1, -1, -1},
    spell_vaccine,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 133),   15,  12,
    "",      "You are no longer vaccinated from {gd{Di{rs{gease{x.",  ""
  },
  {
    "fish breath",
    { -1,  -1, -1, -1, -1, -1, -1, 15, -1, -1, -1, -1, -1, -1},
    { -1,  -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_fish_breath,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 134),   5,  12,
    "",      "You feel too weak to swim now.",  ""
  },
  {
    "mind shield",
    { 25,  -1, -1, -1, -1, -1, -1, 21, -1, 46, -1, -1, -1, -1},
    {  1,  -1, -1, -1, -1, -1, -1,  1, -1,  1, -1, -1, -1, -1},
    spell_mind_shield,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 135),   30,  12,
    "",    "You feel your mind becoming vulnerable again.",  ""
  },
  {
    "power transfer",
    { -1,  -1, -1, -1, -1, -1, -1, 29, -1, -1, -1, -1, -1, -1},
    { -1,  -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_power_transfer,  TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 136),   10,  12,
    "",      "",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "confine",
    { 19,  -1, -1, -1, -1, -1, -1, -1, -1, 44, -1, -1, -1, -1},
    {  1,  -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_confine,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT( 137),   20,  12,
    "confine",      "You no longer feel so confined",  ""
  },

  {
    "cone of cold",
    { 19, -1, -1, -1, 20, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_cone_of_cold,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(138),  10,  12,
    "cone of cold",    "!Cone of Cold!",    ""
  },
  {
    "fire elemental",
    { -1,  -1, -1, -1, -1, -1, -1, 22, -1, -1, -1, -1, -1, -1},
    { -1,  -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_fire_elemental,    TAR_IGNORE,  POS_FIGHTING,
    NULL,      SLOT( 139),   100,  12,
    "",      "",  ""
  },
  {
    "banshee scream",
    { 26,  -1, -1, -1, 28, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1,  -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_banshee_scream,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT( 140),   20,  24,
    "banshee scream",  "!Banshee Scream!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "sunbeam",
    { -1, -1, -1, -1, -1, -1, -1, 27, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_sunbeam,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(141),  12,  12,
    "sunbeam",    "!Sunbeam!",    ""
  },
  {
    "sonic blast",
    { 25, -1, -1, -1, 28, -1, -1, -1, -1, -1, 32, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_sonic_blast,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(142),  20,  12,
    "sonic blast",    "!Sonic Blast!",    ""
  },
  {
    "insect swarm",
    { -1, -1, -1, -1, -1, -1, -1, 7, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1},
    spell_insect_swarm,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(143),  15,  12,
    "insect swarm",    "!Insect Swarm!",    ""
  },
  {
    "tornado",
    { -1,  -1, -1, -1, -1, -1, -1, 40, -1, -1, -1, -1, -1, -1},
    { -1,  -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_tornado,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT( 144),   25,  12,
    "tornado",  "!Tornado!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "hammer of thor",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_hammer_of_thor,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(145),  50,  12,
    "Hammer of Thor",    "!Hammer of Thor!",    ""
  },
  {
    "entangle",
    { -1, -1, -1, -1, -1, -1, -1, 19, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_entangle,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
    NULL,                   SLOT(146),      30,     12,
    "entangle",                     "You break free from the {gen{yt{gan{Gg{yl{gement{x.",  ""
  },
  {
    "flash",
    {  5,  -1, -1, -1, 7, -1, -1, -1, -1, -1,  9, -1, -1, -1},
    {  1,  -1, -1, -1, 1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_flash,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT( 147),   10,  12,
    "flash",    "!Flash!",  ""
  },
  {
    "strangle",
    { -1, -1, -1, -1, -1, -1, 10, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  5, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
    &gsn_strangle,            SLOT( 0),        0,     24,
    "strangle",                  "You can breath again.",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "buckkick",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
    &gsn_buckkick,            SLOT( 0),        0,     24,
    "bucking kick",     "You get your wind back from the bucking kick.",    ""
  },
  {
    "gore",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1 ,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
    &gsn_gore,            SLOT( 0),        0,     24,
    "gore",                     "",    ""
  },
  {
    "nerve pinch",
    { -1, -1, -1, -1, -1, -1, 32, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  4, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
    &gsn_nerve,            SLOT( 0),        0,     48,
    "nerve pinch",                     "The nerve pain subsides",    ""
  },
  {
    "takedown",
    { -1, -1, -1, -1, -1, -1, 17, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  4, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
    &gsn_takedown,            SLOT( 0),        0,    24,
    "takedown",                     "!takedown!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "focus",
    { -1, -1, -1, -1, -1, 20, -1, -1, 23, -1, -1, 20, -1, -1},
    { -1, -1, -1, -1, -1,  3, -1, -1,  3, -1, -1,  2, -1, -1},
    spell_null,        TAR_CHAR_OFFENSIVE,        POS_FIGHTING,
    &gsn_focus,          SLOT( 0),         0,    12,
    "focus",         "!Focus!",    ""
  },
  {
    "crosscut",
    { -1, -1, -1, 35, -1, -1, -1, -1, -1, -1, -1, 67, -1, -1},
    { -1, -1, -1,  7, -1, -1, -1, -1, -1, -1, -1,  6, -1, -1},
    spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
    &gsn_crosscut,            SLOT( 0),        0,   24,
    "crosscut",                     "!Crosscut!",    ""
  },
  {
    "familiarize weapon",
    { 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_familiarize_weapon,  TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(148),   15,  24,
    "",      "!Familiarize Weapon!",  ""
  },
  {
    "karate",
    { -1, -1, -1,-1,-1,-1, 5, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1,-1,-1,-1, 3, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_karate,  SLOT( 0),  0,  12,
    "karate move",      "!Karate|",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "globe of invulnerability",
    { 28, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_invulnerability,  TAR_CHAR_SELF,  POS_STANDING,
    NULL,    SLOT(149),  70,  12,
    "",      "The {Bgl{mobe{x protecting you fades.",
    ""
  },
  {
    "thunder",
    { -1,  -1, -1, -1, -1, -1, -1, 14, -1, -1, -1, -1, -1, -1},
    { -1,  -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_thunder,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT( 150),   14,  12,
    "thunder",  "!Thunder!",  ""
  },
  {
    "empower weapon",
    { 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_empower_weapon,    TAR_OBJ_INV,    POS_STANDING,
    NULL,      SLOT(151),  20,  24,
    "",      "!Empower weapon!",    ""
  },
  {
    "dislocation",
    { 30, -1, -1, -1, 35, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_dislocation,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 152),   25,  12,
    "",      "You are fully back in this plane.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "bear spirit",
    { -1, -1, -1, -1, -1, -1, 15, 20, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  1,  1, -1, -1, -1, -1, -1, -1},
    spell_bear_spirit,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    &gsn_bear_spirit,      SLOT( 153),   40,  12,
    "bear spirit",      "You feel the spirit of the {Dbe{wa{Dr{x leave you.",  ""
  },
  {
    "eagle spirit",
    { -1, -1, -1, -1, -1, -1, 20, 25, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  1,  1, -1, -1, -1, -1, -1, -1},
    spell_eagle_spirit,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    &gsn_eagle_spirit,      SLOT( 153),   40,  12,
    "eagle spirit{x",      "You feel the spirit of the {wea{Wg{Dle{x leave you.",  ""
  },
  {
    "tiger spirit",
    { -1, -1, -1, -1, -1, -1, 25, 30, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  1,  1, -1, -1, -1, -1, -1, -1},
    spell_tiger_spirit,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    &gsn_tiger_spirit,      SLOT( 153),   40,  12,
    "tiger spirit{x",  "You feel the spirit of the {yt{Di{yg{De{yr{x leave you.",  ""
  },
  {
    "dragon spirit",
    { -1, -1, -1, -1, -1, -1, 30, 35, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  1,  1, -1, -1, -1, -1, -1, -1},
    spell_dragon_spirit,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    &gsn_dragon_spirit,      SLOT( 153),   40,  12,
    "dragon spirit{x",      "You feel the spirit of the {gdrag{con{x leave you.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "hunt",
    { -1, -1, 35,-1,-1,-1,-1, 65, -1, -1, -1,  8, -1, -1},
    { -1, -1,  7,-1,-1,-1,-1,  7, -1, -1, -1,  5, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_track,  SLOT( 0),  0,  12,
    "",      "!HUNT!",  ""
  },
  {
    "scry",
    { -1, -1, -1, -1, -1, -1, -1, 76, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_scry,    TAR_IGNORE,  POS_STANDING,
    NULL,      SLOT( 154),   100,  48,
    "",      "!scry!",  ""
  },
  {
    "second cast",
    { 65, 70, -1,-1,82,-1,-1,75, -1, 65, -1, -1, -1, -1},
    {  7,  7, -1,-1, 9,-1,-1, 7, -1,  7, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_second_cast,  SLOT( 0),  0,  12,
    "",      "!Second Cast!",  ""
  },
  {
    "third cast",
    { 82, 85, -1,-1,-1,-1,-1, -1, -1, 82, -1, -1, -1, -1},
    {  8,  8, -1,-1,-1,-1,-1, -1, -1,  8, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_third_cast,  SLOT(0),  0,  12,
    "",      "!Third Cast!",          ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "cannibalism",
    { -1, -1, -1,-1, 62,-1,-1, -1, -1, 41, -1, -1, -1, -1},
    { -1, -1, -1,-1,  1,-1,-1, -1, -1,  1, -1, -1, -1, -1},
    spell_cannibalism,    TAR_CHAR_SELF,  POS_FIGHTING,
    NULL,      SLOT( 155),   5,  5,
    "",      "You feel more vigorous.",  ""
  },
  {
    "automap",
    { -1, -1, -1,-1,-1,-1,-1, 62, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1,-1,-1,-1,-1,  5, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_automap,  SLOT(0),  0,  12,
    "",      "!AUTOMAP!",          ""
  },
  {
    "dart",
    { -1, -1, 20,64,-1,48,35, 55, -1, -1, -1, 33, -1, -1},
    { -1, -1,  3, 4,-1, 3, 4,  5, -1, -1, -1,  5, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_dart,  SLOT(0),  0,  12,
    "dart",      "Dart",          ""
  },
  {
    "clairvoyance",
    { 45, 48, -1,-1,50,-1,-1, 45, -1, -1, 44, -1, -1, -1},
    {  1,  1, -1,-1, 1,-1,-1,  1, -1, -1,  1, -1, -1, -1},
    spell_clairvoyance,    TAR_CHAR_SELF,  POS_STANDING,
    NULL,      SLOT( 156),   35,  12,
    "",      "You are less clairvoyant.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "fatigue",
    { 50, -1, -1,-1,58,-1,-1, -1, -1, 33, -1, -1, -1, -1},
    {  1, -1, -1,-1, 1,-1,-1, -1, -1,  1, -1, -1, -1, -1},
    spell_fatigue,    TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT( 156),   45,  24,
    "fatigue",      "!fatigue.",  ""
  },
  {
    "mass summon",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_mass_summon,    TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(157),  250,  24,
    "",      "!Mass Summon!",    ""
  },
  {
    "feint",
    { -1, -1, -1, 56, -1, 46, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1,  7, -1,  6, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_feint,             SLOT( 0),        0,     0,
    "",                     "!Feint!",    ""
  },
  {
    "fifth attack",
    { -1, -1, -1, 80, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_fifth_attack,      SLOT( 0),        0,     0,
    "",                     "!Fifth Attack!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "dual attack",
    { -1, -1, -1, -1, -1, 15, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1,  5, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_dual_attack,      SLOT( 0),        0,     0,
    "",                     "!Dual Attack!",  ""
  },
  {
    "stomp",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_stomp,      SLOT( 0),        0,     24,
    "stomp",                     "!Stomp!",  ""
  },
  {
    "replenish",
    { -1, 60, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    spell_replenish,    TAR_CHAR_SELF,  POS_STANDING,
    NULL,      SLOT( 158),   350,  36,
    "",      "Your divine aid wears off.",  ""
  },
  {
   "flying kick",
    { -1, -1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
    &gsn_flying,              SLOT( 0),        0,     12,
    "flying kick{x",                 "!Flying Kick!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "demand",
    { -1, -1, -1,-1,-1,30,-1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1,-1,-1, 4,-1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,     POS_STANDING,
    &gsn_demand,              SLOT( 0),        0,     12,
    "{x",                 "!Demand!",    ""
  },
  {
    "bite",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
    &gsn_bite,            SLOT( 0),        0,     24,
    "bite",                     "",    ""
  },
  {
    "clear head",
    { 32, -1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    spell_clearhead,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 159),   35,  36,
    "",       "You lose your aura of {mmag{bi{mc{ba{ml{x confidence.",  ""
  },
  {
    "avalanche",
    { -1, -1, -1, -1, -1, -1, -1, 60, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_avalanche,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(160),  60,  12,
    "avalanche",    "!Avalanche!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "extra weapon",
    { -1, -1, 4,  3, -1, 7, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, 7,  5, -1, 6, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_second,             SLOT( 0),        0,     0,
    "",                     "!Second!",    ""
  },
  {
    "darksight",
    { 9,  -1, 10, -1, -1, -1,  -1,  8, -1, 9, -1, -1, -1, -1},
    { 1,   1,  2, -1, -1, -1,  -1,  1, -1, 1, -1, -1, -1, -1},
    spell_darksight,  TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(161),   5,  18,
    "",      "You no longer see in the dark.",  ""
  },
  {
    "mageshield",
    { -1, -1, -1, -1, 45, -1,  -1,  -1, -1, -1, -1, -1, -1, -1},
    { -1,  -1, -1, -1, 1, -1,  -1,  -1, -1, -1, -1, -1, -1, -1},
    spell_mageshield,  TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(162),  20,  12,
    "",  "The {bm{Bag{bic{x shield around you fades and disappears.",  ""
  },
  {
    "warriorshield",
    { -1, -1, -1, -1, 43, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_warriorshield,  TAR_CHAR_DEFENSIVE,    POS_STANDING,
    NULL,      SLOT(163),  20,  13,
    "",       "The {yba{Yttl{ye{x shield around you fades and disappears.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "bark skin",
    {-1, -1, -1, -1, -1, -1, -1, 54, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_bark_skin, TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,   SLOT(164),   30,   12,
    "",     "Your skin no longer has the look of {gtree{ybark{x.",     ""
  },
  {
    "resilience blade",
    { 45, -1, -1, -1, 60, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_resilience_blade,  TAR_OBJ_INV,    POS_STANDING,
    NULL,                   SLOT(164),       15,     12,
    "",      "!Resilient Blade!",  "$p looks less {gres{wi{gl{wi{gent{x."
  },
  {
    "hiccup",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_hiccup,  TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    &gsn_hiccup,      SLOT(165),  15,  12,
    "",      "Your hiccups leave.",  ""
  },
  {
    "yawn",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_yawn,  TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    &gsn_yawn,      SLOT(166),  15,  12,
    "You Yawn Widely",      "You feel revived.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "basic style",
    { -1, -1, -1, -1, -1, -1, 10, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  3, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_basic_style,  SLOT( 0),  0,  0,
    "martial arts",      "!Basic Style!",  ""
  },
  {
    "dragon style",
    { -1, -1, -1, -1, -1, -1, 25, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  3, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_dragon_style,  SLOT( 0),  0,  0,
    "dragon style",      "!Dragon Style!",  ""
  },
  {
    "drunk style",
    { -1, -1, -1, -1, -1, -1, 35, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  3, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_drunk_style,  SLOT( 0),  0,  0,
    "drunk style",      "!Drunk Style!",  ""
  },
  {
    "snake style",
    { -1, -1, -1, -1, -1, -1, 48, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  3, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_snake_style,  SLOT( 0),  0,  0,
    "snake style",      "!Snake Style!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "crane style",
    { -1, -1, -1, -1, -1, -1, 65, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  4, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_crane_style,  SLOT( 0),  0,  0,
    "crane style",      "!Crane Style!",  ""
  },
  {
    "tiger style",
    { -1, -1, -1, -1, -1, -1, 72, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  4, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_tiger_style,  SLOT( 0),  0,  0,
    "tiger style",      "!Tiger Style!",  ""
  },
  {
    "ironfist style",
    { -1, -1, -1, -1, -1, -1, 83, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  4, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_ironfist_style,  SLOT( 0),  0,  0,
    "ironfist style",      "!Ironfist Style!",  ""
  },
  {
    "judo style",
    { -1, -1, -1, -1, -1, -1, 94, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  5, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_judo_style,  SLOT( 0),  0,  0,
    "judo style",      "!Judo Style!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "air element",
    { -1, -1, -1, -1, -1, -1, -1, 33, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_air_element,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 167),   40,  12,
    "",      "You feel the {Wai{cr{x leave you.",  ""
  },
  {
    "earth element",
    { -1, -1, -1, -1, -1, -1, -1, 24, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_earth_element,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 168),   40,  12,
    "",      "You feel the {Dear{yth{x leave you.",  ""
  },
  {
    "fire element",
    { -1, -1, -1, -1, -1, -1, -1, 42, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_fire_element,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 169),   40,  12,
    "",      "You feel the {Rfi{yres{x leave your being.",  ""
  },
  {
    "water element",
    { -1, -1, -1, -1, -1, -1, -1, 50, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_water_element,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT( 170),   40,  12,
    "",      "You feel the {Bwa{cters{x leave your being.",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "create staff",
    { -1, -1, -1, -1, -1, -1, -1, 20, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_create_staff,  TAR_IGNORE,    POS_FIGHTING,
    NULL,      SLOT(171),  50,   12,
    "",      "!Create Staff!",  ""
  },
  {
   "shelter",
   { -1, -1, -1, 78, -1, 66, -1, -1, 43, -1, -1, 31, -1, -1},
   { -1, -1, -1,  5, -1,  4, -1, -1,  3, -1, -1,  2, -1, -1},
   spell_null,     TAR_IGNORE,     POS_STANDING,
   &gsn_shelter,    SLOT(0),  0,  12,
   "",      "A shelter crumbles to dust.",  ""
  },
  {
   "ghost",
    { -1, -1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,     POS_DEAD,
    &gsn_ghost_time,              SLOT( 0),        0,     12,
    "ghosted",                 "You are no longer a ghost{x",    ""
  },
  {
   "nochannel",
    { -1, -1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,     POS_DEAD,
    &gsn_nochannel,              SLOT( 0),        0,     12,
    "{WNochanneled{x",          "You can speak to the world again.",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "banish",
    { -1, -1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    spell_banish,    TAR_CHAR_OFFENSIVE,    POS_FIGHTING,
    NULL,       SLOT(172),  35,  12,
    "",      "!Banish!",    ""
  },
  {
    "devour soul",
    { -1, -1, -1, -1, 65, -1, -1, -1, -1, 68, -1, -1, -1, -1},
    { -1, -1, -1, -1, 10, -1, -1, -1, -1, 1, -1, -1, -1, -1},
    spell_devour_soul,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(173),  750,  24,
    "devour soul",    "!DEvour Soul!",    ""
  },
  {
    "ironwill",
    { -1, -1, -1, 49, -1, -1, 57, -1, -1, -1, -1, 48, -1, -1},
    { -1, -1, -1,  6, -1, -1,  6, -1, -1, -1, -1,  6, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_ironwill,  SLOT( 0),  0,  0,
    "iron will",  "You can no longer maintain an {Diron{ywill{x.",  ""
  },
  {
    "nirvana",
    { -1, -1, -1, -1, -1, -1, -1, 28, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_nirvana,  TAR_CHAR_DEFENSIVE,  POS_FIGHTING,
    NULL,    SLOT(174),  70,  12,
    "",      "You are no longer in {gnirv{cana{x with nature.",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
   "twit",
    { -1, -1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,     POS_DEAD,
    &gsn_twit,              SLOT( 0),        0,     12,
    "{WTwitted{x",          "You are no longer a twit! (right).",    ""
  },
  {
    "aura read",
    {31, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_aura_read,  TAR_CHAR_DEFENSIVE,      POS_STANDING,
    NULL,      SLOT(175),  30,  10,
    "",    "!Aura Read!",  ""
  },
  {
    "rub",
    { -1, -1, -1,-1,-1,42,-1, -1, -1, -1, -1, 25, -1, -1},
    { -1, -1, -1,-1,-1, 3,-1, -1, -1, -1, -1,  2, -1, -1},
    spell_null,             TAR_IGNORE,     POS_STANDING,
    &gsn_rub,                  SLOT( 0),        12,     12,
    "{x",                 "!Demand!",    ""
  },
  {
    "heal group",
    { -1, 45, -1, -1, -1, -1, -1, -1, 58, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1,  3, -1, -1, -1, -1, -1},
    spell_heal_group,  TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(176),  180,  24,
    "",      "!Heal Group!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "poison aura",
    { 44,  39, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1,   1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_poison_aura,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(177),   50,  12,
    "",      "You feel less resistant to {gpo{ci{Gs{gon{x.",  ""
  },
  {
    "ice elemental",
    { -1,  -1, -1, -1, -1, -1, -1, 66, -1, -1, -1, -1, -1, -1},
    { -1,  -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1},
    spell_ice_elemental,    TAR_IGNORE,  POS_FIGHTING,
    NULL,      SLOT( 178),   150,  12,
    "",      "",  ""
  },
  {
    "protection neutral",
    { 12,  9, -1, -1, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  1,  1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_protection_neutral,  TAR_CHAR_SELF,    POS_STANDING,
    NULL,      SLOT(179),   5,  12,
    "",      "You feel less protected.",  ""
  },
  {
    "dagger twist",
    { -1, -1, 23, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_dagger_twist,        SLOT( 0),        0,     0,
    "",                     "!Dagger Twist!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "mass bless",
    { -1, 73, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_mass_bless,  TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(180),  30,  36,
    "",      "You feel less righteous.", "$p's holy aura fades."
  },
  {
    "advanced scribe",
    { 56,-1,-1,-1,-1,-1,-1,-1,-1, -1, 58, -1, -1, -1},
    {  4,-1,-1,-1,-1,-1,-1,-1,-1, -1,  3, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_advanced_scribe,    SLOT(0),  0,   24,
    "",      "!Advanced Scribe!",    ""
  },
  {
    "advanced brew",
    { -1,58,-1,-1,-1,-1,-1, -1,-1, -1,  6, -1, -1, -1},
    { -1, 5,-1,-1,-1,-1,-1, -1,-1, -1,  4, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_advanced_brew,    SLOT(0),  0,   24,
    "",      "!Advanced Brew!",    ""
  },
  {
    "advanced craft",
    { -1,-1,-1,-1,57,-1,-1,-1,-1, 52, 47, -1, -1, -1},
    { -1,-1,-1,-1, 3,-1,-1,-1,-1,  4,  3, -1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_STANDING,
    &gsn_advanced_craft,    SLOT(0),  0,   24,
    "",      "!Advanced Craft!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "mass vaccine",
    { -1, 62, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_mass_vaccine,  TAR_IGNORE,    POS_STANDING,
    NULL,      SLOT(181),  180,  24,
    "",      "You are no longer vaccinated from {gpo{ci{Gs{gon{x.", ""
  },
  {
   "adrenalize",
    { -1, -1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,     POS_DEAD,
    &gsn_adrenalize,              SLOT( 0),        0,     12,
    "{WAdrenalized{x",                 "You are no longer so full of adrenaline.",    ""
  },
  {
    "full regen",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_full_regen,  TAR_CHAR_DEFENSIVE,    POS_STANDING,
    NULL,      SLOT(182),  90,  24,
    "",      "!Regen!", ""
  },
  { 
    "shadow walk",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_shadow_walk,  TAR_IGNORE,      POS_STANDING,
    NULL,      SLOT(183),  40,  12,
    "",      "!Shadow Walk!", ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  { 
    "magma burst",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_magma_burst,  TAR_CHAR_OFFENSIVE,      POS_FIGHTING,
    NULL,      SLOT(184),  70,  12,
    "{RMo{Ylt{Ren Ma{Ygm{Ra{x",      "!Molten Magma!", ""
  },
  { 
    "lava burst",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_lava_burst,  TAR_CHAR_OFFENSIVE,      POS_FIGHTING,
    NULL,      SLOT(185),  90,  12,
    "{RL{Ya{Rv{Ya {WBurst{x",      "!Lava Burst!", ""
  },
  { 
    "windtomb",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_windtomb,  TAR_CHAR_DEFENSIVE,      POS_STANDING,
    NULL,      SLOT(186),  20,  12,
    "",      "The Wind dies down around you.", ""
  },
  { 
    "star storm",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_star_storm,  TAR_CHAR_OFFENSIVE,      POS_FIGHTING,
    NULL,      SLOT(187),  60,  12,
    "{WS{Dta{Wr S{Dtor{Wm{x",      "!Star Storm!", ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  { 
    "jump",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_jump,          TAR_CHAR_DEFENSIVE,      POS_STANDING,
    NULL,      SLOT(187),  60,  12,
    "{x",      "You feel the strength in your legs weaken."
  },
  { 
    "smoke screen",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_smoke_screen,          TAR_CHAR_DEFENSIVE,      POS_STANDING,
    NULL,      SLOT(188),  30,  12,
    "{x",      "The {Dh{wa{Dze{x around you dissipates."
  },
  { 
    "petrify",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_petrify,  TAR_CHAR_OFFENSIVE,      POS_FIGHTING,
    NULL,      SLOT(189),  60,  12,
    "Petrify",      "You feel a big relief as fear leaves you.", ""
  },
  { 
    "immolation",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_immolation,  TAR_CHAR_OFFENSIVE,      POS_FIGHTING,
    NULL,      SLOT(190),  300,  36,
    "Immolation",      "!Immolation!", ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  { 
    "typhoon",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_typhoon,  TAR_CHAR_OFFENSIVE,      POS_FIGHTING,
    NULL,      SLOT(191),  120,  12,
    "Typhoon{x",      "!Typhoon!", ""
  },
  {
    "protection negative",
    { -1, -1, -1, -1, -1, -1, -1, -1, 22, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1},
    spell_protection_negative,    TAR_CHAR_SELF,                 POS_STANDING,
    NULL,                       SLOT(192),      5,    12,
    "",                        "You feel the {yd{Wi{yv{Wi{yne{x energies leave you.", ""
  },
  {
    "protection holy",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 24, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_protection_holy,    TAR_CHAR_SELF,                     POS_STANDING,
    NULL,                       SLOT(193),      5,    12,
    "",                        "You feel the {rdem{Do{rn{Di{rc{x energies leave you.", ""
  },
  {
    "divine intervention",
    { -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1},
    spell_divine_intervention,    TAR_CHAR_OFFENSIVE,            POS_FIGHTING,
    NULL,                       SLOT(194),      6,    12,
    "divine intervention",                        "!Divine Intervention!", ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "demonic intervention",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  3, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_demonic_intervention,    TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
    NULL,                       SLOT(195),      6,    12,
    "demonic intervention",                        "!Demonic Intervention!", ""
  },
  {
    "force of faith",
    { -1, -1, -1, -1, -1, -1, -1, -1, 27, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1},
    spell_force_of_faith_darkflow,    TAR_CHAR_OFFENSIVE,         POS_FIGHTING,
    NULL,                       SLOT(196),      12,    12,
    "force of faith",                        "!Force of Faith!", ""
  },
  {
    "darkflow",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 18, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_force_of_faith_darkflow,    TAR_CHAR_OFFENSIVE,         POS_FIGHTING,
    NULL,                       SLOT(197),      12,    12,
    "darkflow",                        "!Darkflow!", ""
  },
  {
    "dark fire",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_dark_fire,    TAR_CHAR_OFFENSIVE,         POS_FIGHTING,
    NULL,                       SLOT(197),      12,    12,
    "dark fire",                        "!Darkflow!", ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "noble truth",
    { -1, -1, -1, -1, -1, -1, -1, -1, 56, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1,  5, -1, -1, -1, -1, -1},
    spell_noble_truth,  TAR_IGNORE,         POS_FIGHTING,
    NULL,                       SLOT(198),      18,    12,
    "noble truth",                     "Your {Yno{yb{Wl{we{x spirit slips away.", ""
  },
  {
    "vile intent",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 56, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  3, -1, -1, -1, -1},
    spell_vile_intent,    TAR_IGNORE,         POS_FIGHTING,
    NULL,                       SLOT(199),      18,    12,
    "vile intent",                     "Your {rv{Ri{Dl{re{x intentions abandon you.", ""
  },
  {
    "layhands",
    { -1, -1, -1, -1, -1, -1, -1, -1,  3,  3, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1,  1,  1, -1, -1, -1, -1},
    spell_null,       TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
    &gsn_layhands,             SLOT(0),      0,     24,
    "laying of hands",     "!Lay Hands!",  ""
  },
  {
    "guided strike",
    { -1, -1, -1, -1, -1, -1, -1, -1, 44, 44, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1,  7,  7, -1, -1, -1, -1},
    spell_null,            TAR_IGNORE,             POS_FIGHTING,
    &gsn_guided_strike,         SLOT(0),      0,     0,
    "",                 "!Guided Strike!",       ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "crusade",
    { -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1,  3, -1, -1, -1, -1, -1},
    spell_null,           TAR_IGNORE,             POS_STANDING,
    &gsn_crusade,               SLOT(0),      0,     0,
    "",              "Your crusade comes to an end.",  ""
  },
  {
    "radiance",
    { -1, -1, -1, -1, -1, -1, -1, -1, 29, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1},
    spell_radiance,           TAR_IGNORE,             POS_STANDING,
    NULL,               SLOT(306),      75,     12,
    "",              "Your {Wradian{cce{x fades away.",  ""
  },
  {
    "malevolent shroud",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 30, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_malevolent_shroud,  TAR_IGNORE,             POS_STANDING,
    NULL,               SLOT(305),      75,     12,
    "",    "Your malevolent {rs{Dhroud{x dissipates.",  ""
  },
  {
    "turmoil",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 27, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_turmoil,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
    NULL,                       SLOT(200),      12,     12,
    "turmoil",              "!turmoil!",            ""
    },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "fade out",
    { -1, -1, 42, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_fade_out,      TAR_CHAR_DEFENSIVE,      POS_STANDING,
    NULL,             SLOT(300),     70,  12,
    "",            "You feel you cannot {Wf{wa{Dde{x around attacks anymore.",      ""
  },
  {
    "strengthen",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_strengthen,       TAR_CHAR_DEFENSIVE,     POS_STANDING,
    NULL,                       SLOT(301),       20,     12,
    "",                 "You feel less strengthened.",     ""
  },
  {
    "nimbleness",
    { 16, -1, -1, -1, -1, -1, -1, -1, -1, 17, -1, -1, -1, -1},
    {  1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_nimbleness,       TAR_CHAR_DEFENSIVE,     POS_STANDING,
    NULL,                       SLOT(302),       20,     12,
    "",                 "You feel less nimble.",     ""
  },
  {
    "holy ritual",
   { -1, -1, -1, -1, -1, -1, -1, -1, 60, -1, -1, -1, -1, -1},
   { -1, -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1},
   spell_holy_ritual,       TAR_OBJ_INV,     POS_STANDING,
   NULL,                       SLOT(303),       20,     12,
    "",          "!holy ritual",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "unholy ritual",
   { -1, -1, -1, -1, -1, -1, -1, -1, -1, 57, -1, -1, -1, -1},
   { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
   spell_unholy_ritual,       TAR_OBJ_INV,     POS_STANDING,
   NULL,                       SLOT(304),       20,     12,
    "",          "!unholy ritual",    ""
  },
  {
    "investigate",
    { -1, -1, 18, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,         TAR_IGNORE,             POS_RESTING,
    &gsn_investigate,  SLOT( 0),       0,      0,
    "",                 "!Investigate!",       ""
  },
  {
    "soul shroud",
    { -1, -1, -1, -1, -1, -1, 50, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1},
    spell_soul_shroud,    TAR_IGNORE,         POS_FIGHTING,
    NULL,                       SLOT(305),      18,    12,
    "soul shroud",                     "You lose the shroud of {msoul{rs{x surrounding your hands.", ""
  },
  {
    "spirit shroud",
    { -1, -1, -1, -1, -1, -1, 60, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1},
    spell_spirit_shroud,    TAR_IGNORE,         POS_FIGHTING,
    NULL,                       SLOT(306),      18,    12,
    "spirit shroud",                     "You lose the shroud of {Dsp{Gi{gr{Di{gt{x surrounding your hands.", ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "flame shroud",
    { -1, -1, -1, -1, -1, -1, 35, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1},
    spell_flame_shroud,    TAR_IGNORE,         POS_FIGHTING,
    NULL,                       SLOT(307),      18,    12,
    "flame shroud",                     "You lose the shroud of {rf{Yl{Ra{ym{re{x surrounding your hands.", ""
  },
  {
    "frost shroud",
    { -1, -1, -1, -1, -1, -1, 24, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1},
    spell_frost_shroud,    TAR_IGNORE,         POS_FIGHTING,
    NULL,                       SLOT(308),      18,    12,
    "frost shroud",                     "You lose the shroud of {wi{cc{be{x surrounding your hands.", ""
  },
  {
    "electric shroud",
    { -1, -1, -1, -1, -1, -1, 18, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1},
    spell_electric_shroud,    TAR_IGNORE,         POS_FIGHTING,
    NULL,                       SLOT(309),      18,    12,
    "electric shroud",                     "You lose the shroud of {ye{Wl{yec{Wt{yr{Wi{yc{Wi{Yt{yy{x surrounding your hands.", ""
  },
  {
    "poison shroud",
    { -1, -1, -1, -1, -1, -1, 12, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1},
    spell_poison_shroud,    TAR_IGNORE,         POS_FIGHTING,
    NULL,                       SLOT(310),      18,    12,
    "poison shroud",                     "You lose the shroud of {Gpo{ci{gson{x surrounding your hands.", ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "flare",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,    TAR_IGNORE,         POS_FIGHTING,
    &gsn_flare,                       SLOT(0),      0,    24,
    "flare",                     "!Flare!", ""
  },
  {
    "clay golem",
    { -1,  -1, -1, -1, 60, -1, -1, -1, -1, 70, -1, -1, -1, -1},
    { -1,  -1, -1, -1,  2, -1, -1, -1, -1,  2, -1, -1, -1, -1},
    spell_clay_golem,                TAR_IGNORE,     POS_STANDING,
    NULL,                       SLOT( 311),      150,   12,
    "",                 "",     ""
  },
  {
    "rejuvination",
    { -1, 85, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_rejuvination,                TAR_CHAR_DEFENSIVE,     POS_STANDING,
    NULL,                       SLOT( 312),      80,   12,
    "",                 "!rejuvination!",     ""
  },
  {
    "descry",
    { 10, 10, -1, -1, 20, -1, 50, 15, -1, 40, -1, -1, -1, -1},
    {  5,  5, -1, -1,  6, -1,  8,  6, -1,  7, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_STANDING,
    &gsn_descry,              SLOT( 0),       0,      24,
    "descry",                 "!Descry!",       ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "warcry hardening",
    { -1, -1, -1, -1, -1, 56, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_STANDING,
    &gsn_warcry_hardening,      SLOT( 0 ),    0,      24,
    "warcry hardening",         "Your body loses its hard edge.",    ""
  },
  {
    "warcry rage",
    { -1, -1, -1, -1, -1, 13, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1,  3, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_STANDING,
    &gsn_warcry_rage,      SLOT( 0 ),    0,      24,
    "warcry rage",         "You no longer feel your god's rage.",    ""
  },
  {
    "warcry vigor",
    { -1, -1, -1, -1, -1, 31, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1,  5, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_STANDING,
    &gsn_warcry_vigor,      SLOT( 0 ),    0,      24,
    "warcry vigor",         "You no longer feel as agile.",    ""
  },
  {
    "warcry guarding",
    { -1, -1, -1, -1, -1, 36, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1,  5, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_STANDING,
    &gsn_warcry_guarding,      SLOT( 0 ),    0,      24,
    "warcry guarding",         "You no longer feel guarded.",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "warcry shout",
    { -1, -1, -1, -1, -1, 67, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1,  6, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_warcry_shout,      SLOT( 0 ),    0,      24,
    "warcry shout",         "You no longer feel guarded.",    ""
  },
  {
    "cure weaken",
    { -1, 34, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_cure_weaken,  TAR_CHAR_DEFENSIVE, POS_FIGHTING,
    NULL,           SLOT(313),   25, 12,
    "",         "!Cure Weaken!",    ""
  },
  {
    "pestilence",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 65, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1},
    spell_pestilence,  TAR_CHAR_OFFENSIVE, POS_FIGHTING,
    NULL,           SLOT(314),   90, 12,
    "",         "!Pestilence!",    ""
  },
  {
    "mass blindness",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 22, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_mass_blindness,  TAR_CHAR_OFFENSIVE, POS_FIGHTING,
    NULL,           SLOT(315),   30, 12,
    "",         "!Mass Blindness!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "mass weaken",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 31, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_mass_weaken,  TAR_CHAR_OFFENSIVE, POS_FIGHTING,
    NULL,           SLOT(316),   50, 12,
    "",         "!Mass Weakness!",    ""
  },
  {
    "mass curse",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 39, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_mass_curse,  TAR_CHAR_OFFENSIVE, POS_FIGHTING,
    NULL,           SLOT(317),   80, 12,
    "",         "!Mass Curse!",    ""
  },
  {
    "bone armor",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 40, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_bone_armor,  TAR_CHAR_DEFENSIVE, POS_STANDING,
    NULL,           SLOT(318),   40, 12,
    "",         "Your armor of bone fades away.",    ""
  },
  {
    "skeletal spike",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 35, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_skeletal_spike,   TAR_CHAR_OFFENSIVE, POS_FIGHTING,
    NULL,           SLOT(319),    20, 12,
    "skeletal spike",       "!Skeletal Spike!",     ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "nightmare",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 43, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_nightmare,   TAR_CHAR_OFFENSIVE, POS_FIGHTING,
    NULL,           SLOT(320),   15,  12,
    "nightmare",       "Your mind is now free of terror.",     ""
  },
  {
    "bone rot",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 55, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_bone_rot,    TAR_CHAR_OFFENSIVE, POS_FIGHTING,
    NULL,           SLOT(321),  35, 12,
    "bone rot",        "!Bone Rot!",      ""
  },
  {
    "poisonous dart",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 7, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1},
    spell_poisonous_dart, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
    NULL,           SLOT(322),  15, 12,
    "poisonous dart",     "!Poisonous Dart!",       ""
  },
  {
    "tail",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_tail,              SLOT( 0),       0,      24,
    "tail",                 "!Tail!",       ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "spirit leech",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 69, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_spirit_leech, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
    NULL,           SLOT(323),  15, 12,
    "spirit leech",     "!Spirit Leech!",       ""
  },
  {
    "triple attack",
    { -1, -1, -1, -1, -1, 45, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1,  5, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_triple_attack,      SLOT( 0),        0,     0,
    "",                     "!Triple Attack!",  ""
  },
  {
    "ghostly wail",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 80, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_ghostly_wail, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
    NULL,           SLOT(324),  40,   12,
    "{Dgh{Wo{wstly {Dwa{wi{Dl{x",     "!Ghostly Wail!",       ""
  },
  {
    "raise skeleton",
    { -1,  -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    { -1,  -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_raise_skeleton,                TAR_OBJ_INV,     POS_STANDING,
    NULL,                       SLOT( 325),      40,   12,
    "",                 "",     ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "holy runes",
    { -1,  -1, -1, -1, -1, -1, -1, -1, 25, -1, -1, -1, -1, -1},
    { -1,  -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1},
    spell_holy_runes,                TAR_OBJ_INV,     POS_STANDING,
    NULL,                       SLOT( 326),     75,   12,
    "",     "!Holy Runes!",  "The {yh{Wo{yly {Wrunes{x on $p fade away."
  },
  {
    "throw",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, 15, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  4,  4, -1, -1},
    spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
    &gsn_throw,             SLOT( 0),        0,     20,
    "throw",                "!Throw!",    ""
  },
  {
    "rebrew",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 65, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  7, -1, -1, -1},
    spell_null,             TAR_IGNORE,     POS_STANDING,
    &gsn_rebrew,            SLOT( 0),        0,     24,
    "rebrew",               "!Rebrew!",    ""
  },
  {
    "transcribe",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 41, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  6, -1, -1, -1},
    spell_null,             TAR_IGNORE,     POS_STANDING,
    &gsn_transcribe,        SLOT( 0),          0,   24,
    "transcribe",           "!Transcribe!",    ""
  },
  {
    "aqua albedo", // sanctuary
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 33, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_aqua_albedo, TAR_CHAR_DEFENSIVE, POS_STANDING,
    NULL,           SLOT(327),  70,   12,
    "aqua albedo",     "The protective waters leave your body.",       ""
  },
  {
    "aqua regia", // acidic shroud
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 48, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1},
    spell_aqua_regia, TAR_CHAR_DEFENSIVE, POS_STANDING,
    NULL,           SLOT(328),  25,   12,
    "aqua regia",        "You lose your {cac{gi{cd{Ci{cc{x shroud.",       ""
  },
  {
    "aqua landhi", // water walk
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 26, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1},
    spell_aqua_landhi, TAR_IGNORE, POS_STANDING,
    NULL,           SLOT(0),  50,   18,
    "aqua landhi",     "You can no longer breathe water.",       ""
  },
  {
    "aqua fortis", // shield
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_aqua_fortis, TAR_CHAR_DEFENSIVE, POS_STANDING,
    NULL,           SLOT(329),  12,   12,
    "aqua fortis",     "Your liquid shield falls with a splash.",       ""
  },
  {
    "aqua vitae", // healing water
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 30, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1},
    spell_aqua_vitae, TAR_CHAR_DEFENSIVE, POS_STANDING,
    NULL,           SLOT(330),  25,   12,
    "aqua vitae",     "!Aqua Vitae!",       ""
  },
  {
    "aqua citrinitas", // clear head
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  8, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_aqua_citrinitas, TAR_CHAR_DEFENSIVE, POS_STANDING,
    NULL,           SLOT(331),  15,   12,
    "aqua citrinitas",     "Your thoughts are no longer so focused.",       ""
  },
  {
    "aqua rubedo", // bless-ish
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 53, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1},
    spell_aqua_rubedo, TAR_CHAR_DEFENSIVE, POS_STANDING,
    NULL,           SLOT(332),  33,   12,
    "aqua rubedo",     "You have lost your connection to the gods.",       ""
  },
  {
    "sulfur blast", // like incinerate
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 42, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_sulfur_blast,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(333),  33,  24,
    "sulfur blast",    "!Sulfur Blast!",    ""
  },
  {
    "adrenaline rush", // Tonic that increases reflexes (-40AC)
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 29, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_adrenaline_rush,  TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(334),  15,  18,
    "",    "Your reflexes slow.",    ""
  },
  {
    "dragon flame", // DAM_FIRE attack, highest level attack spell
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 55, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_dragon_flame,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(335),  75,  24,
    "dragon flame",    "!Dragon Flame!",    ""
  },
  {
    "inverted light", // A trick of bending the light (AFF_INVIS)
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 13, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_inverted_light,  TAR_CHAR_DEFENSIVE,  POS_STANDING,
    &gsn_inverted_light,      SLOT(336),  8,  12,
    "inverted light",    "With a flash, light stops bending around you.",
    "$p flashes into view."
  },
  {
    "chameleon",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 22, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1},
    spell_null,    TAR_IGNORE,    POS_RESTING,
    &gsn_chameleon,    SLOT( 0),   0,  12,
    "",      "!Chameleon!",    ""
  },
  {
    "smoke bomb", // fire blindness
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 18, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_smoke_bomb,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(335),  75,  24,
    "smoke bomb",    "!Smoke Bomb!",    ""
  },
  {
    "naja naja", // slow
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 41, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_naja_naja,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(336),  75,  24,
    "naja naja",    "!Naja Naja!",    ""
  },
  {
    "crotalus", // disease
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 28, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_crotalus,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(337),  75,  24,
    "crotalus",    "!Crotalus!",    ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "acidic gas",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_acidic_gas,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(338),  15,  12,
    "acidic gas",    "!Acidic Gas!",  ""
  },
  {
    "clarity",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1},
    spell_clarity,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(339),   35,  12,
    "",    "You are no longer in a state of clarity.",  ""
  },
  {
    "alacrity",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 60, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1},
    spell_alacrity,    TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(340),   50,  12,
    "",    "You are no longer in a state of clarity.",  ""
  },
  {
    "hyracal pressure",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 18, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    spell_hyracal_pressure,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(341),  15,  12,
    "hyracal pressure",    "!Hyracal Pressure!",  ""
  },
  {
    "prayer",
    { -1, -1, -1, -1, -1, -1, -1, -1, 20, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1,  1, -1,  1, -1, -1, -1},
    spell_prayer,  TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(342),  18,  12,
    "prayer",    "You are no longer guarded by a prayer.",  ""
  },
  {
    "conviction",
    { -1, -1, -1, -1, -1, -1, -1, -1, 45, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1,  1, -1,  1, -1, -1, -1},
    spell_conviction,  TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(343),  40,  12,
    "conviction",    "You lose your heightened conviction.",  ""
  },
  {
    "faith",
    { -1, -1, -1, -1, -1, -1, -1, -1, 65, -1, -1, -1, -1, -1},
    { -1,  1, -1, -1, -1, -1, -1, -1,  1, -1,  1, -1, -1, -1},
    spell_faith,  TAR_CHAR_DEFENSIVE,  POS_STANDING,
    NULL,      SLOT(344),  65,  12,
    "faith",    "Your mind weakens from a loss of faith.",  ""
  },
  {
    "blessed shield",
    { -1, -1, -1, -1, -1, -1, -1, -1, 15, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1,  4, -1, -1, -1, -1, -1},
    spell_blessed_shield,    TAR_OBJ_INV,  POS_STANDING,
    NULL,      SLOT( 345),  35,  24,
    "",       "!blessed shield!",  ""
  },
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
  {
    "thorn blast", // lev must be higher than bark skin (54)
    { -1, -1, -1, -1, -1, -1, -1, 63, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1, -1, -1},
    spell_thorn_blast,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(346),  25,  12,
    "thorn blast",    "!Thorn Blast!",  ""
  },
  {
    "shield bash",
    { -1, -1, -1, 25, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { -1, -1, -1,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_shield_bash,              SLOT( 0),       0,      24,
    "shield bash",                 "!Shield Bash!",       ""
  },
  {
    "demonic screech",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, 75, -1, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1},
    spell_demonic_screech,  TAR_CHAR_OFFENSIVE,  POS_FIGHTING,
    NULL,      SLOT(347),  40,  12,
    "demonic screech",    "!Demonic Screech!",  ""
  },
  {
    "rune wipe",
    { 82, -1, -1, -1, 85, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {  3, -1, -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    spell_rune_wipe,  TAR_OBJ_INV,  POS_STANDING,
    NULL,      SLOT(348),  40,  12,
    "rune wipe",    "!Rune Wipe!",  ""
  },
  {
    "sweep",
    { -1, -1, 43, -1, -1, -1, -1, -1, -1, -1, -1, 40, -1, -1},
    { -1, -1,  6, -1, -1, -1, -1, -1, -1, -1, -1,  7, -1, -1},
    spell_null,    TAR_IGNORE,    POS_FIGHTING,
    &gsn_sweep,    SLOT( 0),  0,  18,
    "",      "!Sweep!",    ""
  },
  {
    "crossbow",
    { -1, -1,  1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1},
    { -1, -1,  5, -1, -1, -1, -1, -1, -1, -1,  4, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_crossbow,           SLOT( 0),       0,      0,
    "",                     "!Crossbow!",    ""
  },
  {
    "double shot",
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  13, -1, -1, -1},
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,   5, -1, -1, -1},
    spell_null,             TAR_IGNORE,             POS_FIGHTING,
    &gsn_double_shot,     SLOT( 0),        0,     0,
    "",                     "!Double Shot!",  ""
  }
/*  {Cjr,Pri,Hwy,Kni,Wrl,Bar,Mys,Dru,Inq,Occ,Alc,Wds,un5,un5} */
};
/* name,
   level[],
   rating[] (learning difficulty)
   spell_fun, targetType, min_pos
   gsn ptr, #OBJECT-SLOT, min_mana, beats
   dam_msg, wearoff_msg, wearoff_obj_msg
*/


struct group_type group_table[MAX_GROUP] =
{
  {
    "rom basics",
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
    { "scrolls", "staves", "wands", "recall" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "conjurer basics",
    {   0,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "dagger","meditation" }
  },
  {
    "priest basics",
    {  -1,   0,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "mace","stake" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "highwayman basics",
    {  -1,  -1,   0,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "dagger", "steal" }
  },
  {
    "knight basics",
    {  -1,  -1,  -1,   0,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "sword", "second attack" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "warlock basics",
    {  -1,  -1,  -1,  -1,   0,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "sword", "dodge" }
  },
  {
    "barbarian basics",
    {  -1,  -1,  -1,  -1,  -1,   0,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "axe", "extra weapon" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "mystic basics",
    {  -1,  -1,  -1,  -1,  -1,  -1,   0,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "hand to hand", "blade weave" }
  },
  {
    "druid basics",
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,   0,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1},
    { "spear", "dodge" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "inquisitor basics",
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,   0,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1},
    { "sword", "dodge" }
  },
  {
    "occultist basics",
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,   0,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1},
    { "sword", "dodge" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "alchemist basics",
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,   0,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1},
    { "crossbow", "brew" }
  },
  {
    "woodsman basics",
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,   0,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1},
    { "axe", "hunt" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "conjurer default",
    {  40,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "beguiling", "combat", "detection", "enhancement",
      "maledictions", "protective", "transportation","creation"  }
  },
  {
    "priest default",
    {  -1,  40,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "retribution", "curative",  "benedictions",
      "detection", "healing", "maledictions", "protective" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "highwayman default",
    {  -1,  -1,  40,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "dart", "backstab", "disarm", "blade weave", "second attack",
      "hide", "peek", "pick lock", "sneak", "dirt kicking" }
  },
  {
    "knight default",
    {  -1,  -1,  -1,  40,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "weaponsmaster", "shield block", "bash", "disarm", "enhanced damage",
      "parry", "rescue", "third attack","exotic" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "warlock default",
    {  -1,  -1,  -1,  -1,  40,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "maledictions", "combat","protective", "parry", "detection",
      "necromancy" }
  },
  {
    "barbarian default",
    {  -1,  -1,  -1,  -1,  -1,  40,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "whirlwind", "dodge", "bash", "berserk", "critical strike",
      "enhanced damage","second attack","third attack" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "mystic default",
    {  -1,  -1,  -1,  -1,  -1,  -1,  40,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "beguiling", "martial arts", "sidestep", "tumbling",
      "kick", "nerve pinch" }
  },
  {
    "druid default",
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  40,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1},
    { "beguiling", "weather", "enhancement", "transportation","nature",
      "creation", "protective", "find familiar" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "inquisitor default",
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  40,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1},
    { "weaponsmaster", "retribution", "detection", "second attack",
      "layhands", "crusade" }
  },
  {
    "occultist default",
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  40,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1},
    { "necromancy", "retribution", "detection", "second attack",
      "layhands", "meditation", "protective", "enhancement", "shield block" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "alchemist default",
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  40,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1},
    { "meditation", "craft", "scribe", "erase", "advanced brew",
      "protective", "alchemy", "empty", "throw" }
  },
  {
    "woodsman default",
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  40,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1},
    { "rub", "dirt kicking", "forage", "hide", "sharpen", "shelter",
      "focus", "rescue", "dodge" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "weaponsmaster",
    {  -1,  -1,  -1,  20,  -1,  -1,  -1,  -1,  22,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1},
    { "axe", "dagger", "flail", "mace", "polearm", "spear", "sword","whip" }
  },
  {
    "retribution",
    {  -1,   6,  -1,  -1,  -1,  -1,  -1,  -1,   7,   7,  -1,  -1,  -1,  -1},
    {  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1, 100, 101,  -1,  -1,  -1,  -1},
    { "demonfire", "dispel evil", "dispel good", "earthquake",
      "flamestrike", "heat metal", "ray of truth", "holy bolt",
      "martyr", "force of faith", "darkflow", "divine intervention",
      "demonic intervention", "dark fire", "turmoil", "noble truth",
      "skeletal spike", "bone rot", "poisonous dart", "spirit leech",
      "ghostly wail", "holy runes", "demonic screech" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "beguiling",
    {   9,   5,   7,  -1,  -1,  -1,   7,   6,  -1,   9,  -1,  -1,  -1,  -1},
    { 102, 100,  98,  -1,  -1,  -1,  80, 100,  -1, 100,  -1,  -1,  -1,  -1},
    { "calm", "charm person", "sleep","bigby bash", "mind shield","fear",
      "betray","invisibility", "mass invis", "ventriloquate", "confine",
      "deter", "fumble", "fatigue", "nightmare" }
  },
  {
    "benedictions",
    {  -1,   8,  -1,  -1,  -1,  -1,  -1,  -1,   4,  -1,  -1,  -1,  -1,  -1},
    {  -1, 101,  -1,  -1,  -1,  -1,  -1,  -1,  80,  -1,  -1,  -1,  -1,  -1},
    { "bless", "calm", "frenzy", "holy word", "remove curse",
      "regeneration", "vaccine", "mass bless", "mass vaccine", "holy ritual",
      "blessed shield" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "combat",
    {   6,  -1,  -1,  -1,  10,  -1,  -1,  -1,  -1,  -1,   9,  -1,  -1,  -1},
    { 101,  -1,  -1,  -1, 102,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1},
    { "acid blast", "burning hands", "chain lightning", "chill touch",
      "colour spray", "fireball", "lightning bolt", "magic missile",
      "shocking grasp", "incinerate", "icicle", "cone of cold",
      "banshee scream","sonic blast", "flash", "ionwave", "earthquake",
      "sulfur blast", "dragon flame", "acidic gas", "hyracal pressure" }
  },
  {
    "creation",
    {   4,   4,  -1,  -1,  -1,  -1,  -1,   4,  -1,  -1,  -1,  -1,  -1,  -1},
    { 100, 100,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1},
    { "continual light", "create food", "create spring", "create water",
      "create rose", "floating disc", "create fountain","sate", "quench",
      "create staff"}
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "curative",
    {  -1,   6,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,   4,  -1,  -1,  -1},
    {  -1, 102,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1},
    { "cure blindness", "cure disease", "cure poison", "cure weaken" }
  },
  {
    "detection",
    {   5,   6,   8,  -1,   7,  -1,  -1,   6,   6,   6,   6,  -1,  -1,  -1},
    { 100, 100, 100,  -1,  90,  -1,  -1, 100,  80, 100, 100,  -1,  -1,  -1},
    { "detect evil", "detect good", "detect hidden", "detect invis",
      "detect magic", "detect poison", "farsight", "identify",
      "know alignment", "locate object","scry", "clairvoyance",
      "aura read"  }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "draconian",
    {   8,  -1,  -1,  -1,  12,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { 100,  -1,  -1,  -1,  90,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "acid breath", "fire breath", "frost breath", "gas breath", "lightning breath"  }
  },
  {
    "enchantment",
    {   9,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { 103,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "enchant armor", "enchant weapon", "fireproof", "recharge",
      "empower weapon","familiarize weapon" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "enhancement",
    {   6,  -1,  11,  -1,  -1,  -1,  -1,   5,  -1,   7,  -1,  -1,  -1,  -1},
    { 100,  -1, 100,  -1,  -1,  -1,  -1, 100,  -1, 100,  -1,  -1,  -1,  -1},
    { "giant strength", "haste", "infravision", "refresh", "clear head",
      "darksight", "vile intent", "fade out", "strengthen", "nimbleness" }
  },
  {
    "harmful",
    {  -1,   3,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "cause critical", "cause light", "cause serious", "harm" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "healing",
    {  -1,   5,  -1,  -1,  -1,  -1,  -1,   7,   8,  -1,  -1,  -1,  -1,  -1},
    {  -1, 101,  -1,  -1,  -1,  -1,  -1, 100,  98,  -1,  -1,  -1,  -1,  -1},
    { "cure critical", "cure light", "cure serious", "heal", "mass healing",
      "refresh", "life transfer","replenish", "heal group", "rejuvination" }
  },
  {
    "maledictions",
    {   5,   8,  -1,  -1,   8,  -1,  -1,  -1,  -1,   4,   6,  -1,  -1,  -1},
    { 100, 100,  -1,  -1,  98,  -1,  -1,  -1,  -1, 102, 100,  -1,  -1,  -1},
    { "blindness", "change sex", "curse", "energy drain", "plague",
      "poison", "slow", "weaken", "pestilence", "mass blindness",
      "mass weaken", "mass curse", "fatigue", "crotalus", "naja naja" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "protective",
    {   6,   5,  -1,  -1,   8,  -1,   7,   6,   7,   5,   7,  -1,  -1,  -1},
    { 101, 103,  -1,  -1,  90,  -1,  80, 101,  80, 101, 100,  -1,  -1,  -1},
    { "armor", "cancellation", "dispel magic", "fireproof",
      "protection evil", "protection good", "protection neutral", "sanctuary", 
      "shield",  "stone skin", "moon armor", "globe of invulnerability",
      "dislocation","mageshield","warriorshield", "protection holy",
      "protection negative", "malevolent shroud", "radiance", "bone armor",
      "adrenaline rush", "clarity", "alacrity" }
  },
  {
    "transportation",
    {   6,   6,  -1,  -1,   8,  -1,   6,   6,   2,   7,   5,  -1,  -1,  -1},
    { 100, 100,  -1,  -1,  98,  -1,  80, 100,  80, 100, 100,  -1,  -1,  -1},
    { "fly", "gate", "nexus", "pass door", "portal", "summon", "teleport",
      "word of recall", "water walk" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "shrouds",
    {  -1,  -1,  -1,  -1,  -1,  -1,   6,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "soul shroud", "spirit shroud", "flame shroud", "frost shroud",
       "electric shroud", "poison shroud" }
  },
  {
    "weather",
    {  -1,   6,  -1,  -1,  -1,  -1,  -1,   6,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1, 100,  -1,  -1,  -1,  -1,  -1, 102,  -1,  -1,  -1,  -1,  -1,  -1},
    { "call lightning", "control weather", "faerie fire", "faerie fog",
      "lightning bolt","tornado", "sunbeam","thunder","avalanche" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "forger",
    {   7,  -1,  -1,  -1,   7,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { 100,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "drain blade", "shocking blade", "flame blade", "resilience blade",
      "frost blade",  "vorpal blade" , "sharp blade", "rune wipe" }
  },
  {
    "campaign",
    {  -1,  -1,  -1,   5,  -1,  -1,  -1,  -1,   8,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  75,  -1,  -1,  -1,  -1,  80,  -1,  -1,  -1,  -1,  -1},
    { "create food", "create water", "armor", "refresh" ,"cure light" ,
      "cause light", "cure poison", "infravision", "aid", "cure blindness",
      "blindness", "word of recall", "continual light" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "auras",
    {   7,   7,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { 100, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "flame aura", "frost aura", "electric aura", "corrosive aura",
      "holy aura", "dark aura", "arcane aura", "poison aura" }
  },
  {
    "background",
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {
      "judgement", "general purpose", "hammer of thor", "buckkick", "mass summon",
      "metamorphose","flying kick", "full regen", "magma burst", "lava burst",
      "windtomb", "star storm", "typhoon", "petrify", "smoke screen", "immolation"
    }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "martial arts",
    {  -1,  -1,  -1,  -1,  -1,  -1,  20,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {
      "karate", "dragon style", "basic style", "drunk style", "crane style",
      "ironfist style", "snake style", "tiger style", "judo style" }
  },
  {
    "nature",
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,   9,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1, 103,  -1,  -1,  -1,  -1,  -1,  -1},
    {"fish breath", "power transfer", "entangle", "insect swarm", "thorn blast",
     "phoenix", "fire elemental", "bark skin", "air element", "fire element", 
     "water element", "earth element", "nirvana", "ice elemental"}
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "spiritual",
    {  -1,  -1,  -1,  -1,  -1,  -1,   5,   6,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  98, 101,  -1,  -1,  -1,  -1,  -1,  -1},
    {"bear spirit", "eagle spirit", "dragon spirit", "tiger spirit"}
  },
  {
    "necromancy",
    {  -1,  -1,  -1,  -1,   4,  -1,  -1,  -1,  -1,   5,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  90,  -1,  -1,  -1,  -1, 103,  -1,  -1,  -1,  -1},
    {"resurrect", "cannibalism", "embalm","devour soul", "unholy ritual",
     "clay golem", "raise skeleton" }
  },
//    Cjr, Pri, Hwy, Kni, Wrl, Bar, Mys, Dru, Inq, Occ, Alc, Wds, UNU, UNU
  {
    "warcries",
    {  -1,  -1,  -1,  -1,  -1,  20,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},
    { "warcry rage", "warcry vigor", "warcry guarding", "warcry hardening",
      "warcry shout" }
  },
  {
    "alchemy",
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,   6,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1},
    { "aqua albedo", "aqua regia", "aqua fortis", "aqua landhi", "aqua vitae",
      "aqua citrinitas", "aqua rubedo", "inverted light", "smoke bomb" }
  },
  {
    "leadership",
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,   5,  -1,  -1,  -1,  -1,  -1},
    {  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 100,  -1,  -1,  -1,  -1,  -1},
    { "prayer", "conviction", "faith" }
  },
};

