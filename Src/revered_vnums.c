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
/* This file contains functions to protect for revered vnums */

int revered_vnums_obj[MAX_REVERED];
int revered_vnums_mob[MAX_REVERED];
int revered_vnums_room[MAX_REVERED];


void setup_revered()
{
  int i=0;
  revered_vnums_obj[i++]= OBJ_VNUM_SILVER_ONE;
  revered_vnums_obj[i++]= OBJ_VNUM_GOLD_ONE;
  revered_vnums_obj[i++]= OBJ_VNUM_GOLD_SOME;
  revered_vnums_obj[i++]= OBJ_VNUM_SILVER_SOME;
  revered_vnums_obj[i++]= OBJ_VNUM_COINS;
  revered_vnums_obj[i++]= OBJ_VNUM_CORPSE_NPC;
  revered_vnums_obj[i++]= OBJ_VNUM_CORPSE_PC;
  revered_vnums_obj[i++]= OBJ_VNUM_SEVERED_HEAD;
  revered_vnums_obj[i++]= OBJ_VNUM_TORN_HEART;
  revered_vnums_obj[i++]= OBJ_VNUM_SLICED_ARM;
  revered_vnums_obj[i++]= OBJ_VNUM_SLICED_LEG;
  revered_vnums_obj[i++]= OBJ_VNUM_GUTS;
  revered_vnums_obj[i++]= OBJ_VNUM_BRAINS;
  revered_vnums_obj[i++]= OBJ_VNUM_MUSHROOM;
  revered_vnums_obj[i++]= OBJ_VNUM_STEAK;
  revered_vnums_obj[i++]= OBJ_VNUM_LIGHT_BALL;
  revered_vnums_obj[i++]= OBJ_VNUM_SPRING;
  revered_vnums_obj[i++]= OBJ_VNUM_DISC;
  revered_vnums_obj[i++]= OBJ_VNUM_PORTAL;
  revered_vnums_obj[i++]= OBJ_VNUM_ROSE;
  revered_vnums_obj[i++]= OBJ_VNUM_PIT;
  revered_vnums_obj[i++]= OBJ_VNUM_SCHOOL_MACE;
  revered_vnums_obj[i++]= OBJ_VNUM_SCHOOL_DAGGER;
  revered_vnums_obj[i++]= OBJ_VNUM_SCHOOL_SWORD;
  revered_vnums_obj[i++]= OBJ_VNUM_SCHOOL_SPEAR;
  revered_vnums_obj[i++]= OBJ_VNUM_SCHOOL_STAFF;
  revered_vnums_obj[i++]= OBJ_VNUM_SCHOOL_AXE;
  revered_vnums_obj[i++]= OBJ_VNUM_SCHOOL_FLAIL;
  revered_vnums_obj[i++]= OBJ_VNUM_SCHOOL_WHIP;
  revered_vnums_obj[i++]= OBJ_VNUM_SCHOOL_POLEARM;
  revered_vnums_obj[i++]= OBJ_VNUM_SCHOOL_VEST;
  revered_vnums_obj[i++]= OBJ_VNUM_SCHOOL_SHIELD;
  revered_vnums_obj[i++]= OBJ_VNUM_SCHOOL_BANNER;
  revered_vnums_obj[i++]= OBJ_VNUM_MAP;
  revered_vnums_obj[i++]= OBJ_VNUM_ALINDRAK_MAP;
  revered_vnums_obj[i++]= OBJ_VNUM_DREKABUS_MAP;
  revered_vnums_obj[i++]= OBJ_VNUM_WHISTLE;
  revered_vnums_obj[i++]= OBJ_VNUM_CLEAN_SCROLL;
  revered_vnums_obj[i++]= OBJ_VNUM_CLEAN_VIAL;
  revered_vnums_obj[i++]= OBJ_VNUM_QUEST;
  revered_vnums_obj[i++]= OBJ_VNUM_RESTRING;
  revered_vnums_obj[i++]= OBJ_VNUM_BAG;
  revered_vnums_obj[i++]= OBJ_VNUM_TALE;
  revered_vnums_obj[i++]= OBJ_VNUM_HANDS;
  revered_vnums_obj[i++]= OBJ_VNUM_FEET;
  revered_vnums_obj[i++]= OBJ_VNUM_FINGERS;
  revered_vnums_obj[i++]= OBJ_VNUM_EAR;
  revered_vnums_obj[i++]= OBJ_VNUM_EYE;
  revered_vnums_obj[i++]= OBJ_VNUM_LONG_TONGUE;
  revered_vnums_obj[i++]= OBJ_VNUM_EYESTALKS;
  revered_vnums_obj[i++]= OBJ_VNUM_TENTACLES;
  revered_vnums_obj[i++]= OBJ_VNUM_FINS;
  revered_vnums_obj[i++]= OBJ_VNUM_WINGS;
  revered_vnums_obj[i++]= OBJ_VNUM_TAIL;
  revered_vnums_obj[i++]= OBJ_VNUM_CLAWS;
  revered_vnums_obj[i++]= OBJ_VNUM_FANGS;
  revered_vnums_obj[i++]= OBJ_VNUM_HORNS;
  revered_vnums_obj[i++]= OBJ_VNUM_SCALES;
  revered_vnums_obj[i++]= OBJ_VNUM_TUSKS;
  revered_vnums_obj[i++]= OBJ_VNUM_HOOF;

  i=0;

  revered_vnums_mob[i++]=MOB_VNUM_FIDO;
  revered_vnums_mob[i++]=MOB_VNUM_CITYGUARD;
  revered_vnums_mob[i++]=MOB_VNUM_VAMPIRE;
  revered_vnums_mob[i++]=MOB_VNUM_COW;
  revered_vnums_mob[i++]=MOB_VNUM_WOLF;
  revered_vnums_mob[i++]=MOB_VNUM_BEAR;
  revered_vnums_mob[i++]=MOB_VNUM_CAT;
  revered_vnums_mob[i++]=MOB_VNUM_SNAIL;
  revered_vnums_mob[i++]=MOB_VNUM_SLIME;
  revered_vnums_mob[i++]=MOB_VNUM_RABBIT;
  revered_vnums_mob[i++]=MOB_VNUM_PATROLMAN;
  revered_vnums_mob[i++]=GROUP_VNUM_TROLLS;
  revered_vnums_mob[i++]=GROUP_VNUM_OGRES;
  revered_vnums_mob[i++]=MOB_VNUM_ZOMBIE;
  revered_vnums_mob[i++]=MOB_VNUM_FAMILIAR;
  revered_vnums_mob[i++]=MOB_VNUM_FIRE_ELEMENTAL;
  revered_vnums_mob[i++]=MOB_VNUM_ICE_ELEMENTAL;
  revered_vnums_mob[i++]=MOB_VNUM_NEWBIE_HINT;
  i=0;
  revered_vnums_room[i++]=ROOM_VNUM_LIMBO;
  revered_vnums_room[i++]=ROOM_VNUM_CHAT;
  revered_vnums_room[i++]=ROOM_VNUM_TEMPLE;
  revered_vnums_room[i++]=ROOM_VNUM_ALTAR	;
  revered_vnums_room[i++]=ROOM_VNUM_SCHOOL;
  revered_vnums_room[i++]=ROOM_VNUM_GOOD_START;
//  revered_vnums_room[i++]=ROOM_VNUM_BALANCE;
//  revered_vnums_room[i++]=ROOM_VNUM_CIRCLE;
//  revered_vnums_room[i++]=ROOM_VNUM_DEMISE;
//  revered_vnums_room[i++]=ROOM_VNUM_HONOR;
  revered_vnums_room[i++]=ROOM_VNUM_EVIL_START;
  revered_vnums_room[i++]=ROOM_VNUM_EVIL_SCHOOL;
  revered_vnums_room[i++]=ROOM_VNUM_EVIL_RECALL;
  revered_vnums_room[i++]=ROOM_VNUM_EVIL_ALTAR;
  revered_vnums_room[i++]=ROOM_VNUM_EVIL_HEALER;

  revered_vnums_room[i++]=ROOM_VNUM_NEUT_START;
  revered_vnums_room[i++]=ROOM_VNUM_NEUT_SCHOOL;
  revered_vnums_room[i++]=ROOM_VNUM_NEUT_RECALL;
  revered_vnums_room[i++]=ROOM_VNUM_NEUT_ALTAR;
  revered_vnums_room[i++]=ROOM_VNUM_NEUT_HEALER;

}

bool is_revered_obj(int vnum)
{
  int i=0;

  for (i=0; i < MAX_REVERED; i++)
    if (revered_vnums_obj[i] == vnum)
      return(TRUE);

  return (FALSE);
}
bool is_revered_room(int vnum)
{
  int i=0;

  for (i=0; i < MAX_REVERED; i++)
    if (revered_vnums_room[i] == vnum)
      return(TRUE);

  return (FALSE);
}
bool is_revered_mob(int vnum)
{
  int i=0;

  for (i=0; i < MAX_REVERED; i++)
    if (revered_vnums_mob[i] == vnum)
      return(TRUE);

  return (FALSE);
}

