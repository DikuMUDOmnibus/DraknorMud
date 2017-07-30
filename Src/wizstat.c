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

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "magic.h"
#include "interp.h"

/*
 * Local functions.
 */
ROOM_INDEX_DATA *find_location args( ( CHAR_DATA *ch, char *arg ) );
int total_class_cp(CHAR_DATA *ch, int iClass); // skills.c
AREA_DATA *get_area_data( int vnum );

/*
 * Stat functions added in to do more than just the basic stats...
 */
DECLARE_DO_FUN( do_race_stat    );
DECLARE_DO_FUN( do_astat        );
DECLARE_DO_FUN( do_cstat        );
DECLARE_DO_FUN( do_class_stat   );
DECLARE_DO_FUN( do_liquid_stat  );
DECLARE_DO_FUN( do_skill_stat   );
DECLARE_DO_FUN( do_group_stat   );
DECLARE_DO_FUN( do_frag_stat    );
/* RT to replace the 3 stat commands */

void do_stat ( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char *string;
  OBJ_DATA *obj;
  ROOM_INDEX_DATA *location;
  CHAR_DATA *victim;

  string = one_argument(argument, arg);
  if ( arg[0] == '\0')
  {
    send_to_char("Syntax:\n\r", ch );
    send_to_char("  stat <name>\n\r", ch );
    send_to_char("  stat char|obj|mob|race|class|liquid|skill|spell|group|fragment <name>\n\r", ch );
    send_to_char("  stat obj|mob|room|code <number>\n\r", ch );
    send_to_char("  stat group|liquid all\n\r", ch );
    return;
  }

  if (!str_cmp(arg,"area"))
  {
    do_function(ch, &do_astat,string);
    return;
  }

  if (!str_cmp(arg,"room"))
  {
    do_function(ch, &do_rstat,string);
    return;
  }

  if (!str_cmp(arg,"obj"))
  {
    do_function(ch, &do_ostat,string);
    return;
  }

  if ( !str_cmp( arg, "char" ) )
  {
    do_function(ch, &do_charstat,string);
    return;
  }

  if ( !str_cmp( arg, "mob" )
  ||   !str_cmp( arg, "old" ) )
  {
    do_function(ch, &do_mstat,string);
    return;
  }

  if ( !str_cmp( arg, "race" ) )
  {
    do_function( ch, &do_race_stat, string );
    return;
  }

  if ( !str_cmp( arg, "code" ) )
  {
    do_function( ch, &do_cstat, string );
    return;
  }

  if ( !str_cmp( arg, "class" ) )
  {
    do_function( ch, &do_class_stat, string );
    return;
  }

  if ( !str_cmp( arg, "skill" )
       ||   !str_cmp( arg, "spell" ) )
  {
    do_function( ch, &do_skill_stat, string );
    return;
  }

  if ( !str_cmp( arg, "group" ) )
  {
    do_function( ch, &do_group_stat, string );
    return;
  }

  if ( !str_cmp( arg, "liquid" ) )
  {
    do_function( ch, &do_liquid_stat, string );
    return;
  }

  if ( !str_cmp( arg, "fragment" )
       ||   !str_cmp( arg, "frag" ) )
  {
    do_function( ch, &do_frag_stat, string );
    return;
  }

  /* do it the old way */

  victim = get_char_world(ch,argument);
  if ( victim )
  {
    if ( !IS_NPC(victim) )
      do_function(ch, &do_charstat,argument);
    else
      do_function(ch, &do_mstat,argument);
    return;
  }

  obj = get_obj_world(ch,argument);
  if ( obj )
  {
    do_function(ch, &do_ostat,argument);
    return;
  }

  location = find_location(ch,argument);
  if ( location )
  {
    do_function(ch, &do_rstat,argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}

void do_astat( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea = NULL;

  if ((argument[0] == '\0')
      && ch->in_room)
    pArea = ch->in_room->area;
  else
  {
    if (is_number(argument))
      pArea = get_area_data(atoi(argument));
    else
      pArea = get_area_world(argument);
  }

  if (pArea == NULL)
  {
    send_to_char( "No such area.\n\r", ch );
    return;
  }

  printf_to_char(ch, "{cName:        {x[%5d] %s\n\r", pArea->vnum, pArea->name );
  printf_to_char(ch, "{cFile:        {x%s\n\r", pArea->file_name  );
  printf_to_char(ch, "{cVnums:       {x[%d-%d] (%d)\n\r", pArea->min_vnum, pArea->max_vnum, pArea->max_vnum-pArea->min_vnum+1 );
  printf_to_char(ch, "{cReset rate:  {x[%d]\n\r", pArea->reset_rate );
  printf_to_char(ch, "{cAge:         {x[%d]\n\r", pArea->age      );
  printf_to_char(ch, "{cPlayers:     {x[%d]\n\r", pArea->nplayer  );
  switch (pArea->align)
  {
    case AREA_ALIGN_ALL:
      printf_to_char(ch, "{cAlignment:   {x[All]\n\r");
      break;
    case AREA_ALIGN_GOOD:
      printf_to_char(ch, "{cAlignment:   {x[Good]\n\r");
      break;
    case AREA_ALIGN_EVIL:
      printf_to_char(ch, "{cAlignment:   {x[Evil]\n\r");
      break;
    case AREA_ALIGN_NEUTRAL:
      printf_to_char(ch, "{cAlignment:   {x[Neutral]\n\r");
      break;
    default:
      printf_to_char(ch, "{cAlignment:   {x[?Unknown?]\n\r");
      break;
  }
  printf_to_char(ch, "{cSecurity:    {x[%d]\n\r", pArea->security );
  printf_to_char(ch, "{cSysflags:    {x[%s]\n\r", area_bit_name(pArea->flags)   );
  printf_to_char(ch, "{cBuilders:    {x[%s]\n\r", pArea->builders );
  printf_to_char(ch, "{cLvl Range:   {x[%d %d]\n\r", pArea->low_range, pArea->high_range );
  printf_to_char(ch, "{cCredits:     {x[%s]\n\r",  pArea->credits );
  printf_to_char(ch, "{cFlags:       {x[%s]\n\r", flag_string( area_flags, pArea->area_flags ) );
  printf_to_char(ch, "{cContinent    {x[%d] %s\n\r", pArea->continent, continent_name(pArea) );
  return;
}

void do_rstat( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  OBJ_DATA *obj;
  CHAR_DATA *rch;
  int door;

  one_argument( argument, arg );
  location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
  if ( location == NULL )
  {
    send_to_char( "No such location.\n\r", ch );
    return;
  }

  /*  if (!is_room_owner(ch,location) && ch->in_room != location
      &&  room_is_private( location ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
      send_to_char( "That room is private right now.\n\r", ch );
      return;
    }
  */
  printf_to_char(ch, "{YStatistics for Room {C({W%s{C){c in Area: {C({W%s{C){x\n\r",
                 location->name,
                 location->area->name );

  printf_to_char(ch,
                 "{cVnum: {W%d  {cSector: {W%d  {cLight: {W%d  {cHealing: {W%d  {cMana: {W%d{x\n\r",
                 location->vnum,
                 location->sector_type,
                 location->light,
                 location->heal_rate,
                 location->mana_rate );

  printf_to_char(ch,
                 "{cRoom flags: {g[{W%s{g]{x\n\r{cDescription:\n\r{x%s{x",
                 flag_string( room_flags, location->room_flags),
                 location->description );

  if ( location->extra_descr != NULL )
  {
    EXTRA_DESCR_DATA *ed;

    send_to_char( "{cExtra description keywords: '{W", ch );
    for ( ed = location->extra_descr; ed; ed = ed->next )
    {
      send_to_char( ed->keyword, ch );
      if ( ed->next != NULL )
        send_to_char( " ", ch );
    }
    send_to_char( "{x'.\n\r", ch );
  }

  send_to_char( "{cCharacters:{x", ch );
  for ( rch = location->people; rch; rch = rch->next_in_room )
  {
    if (can_see(ch,rch))
    {
      send_to_char( " ", ch );
      one_argument( rch->name, buf );
      send_to_char( buf, ch );
    }
  }

  send_to_char( "{x.\n\r{cObjects:   {x", ch );
  for ( obj = location->contents; obj; obj = obj->next_content )
  {
    send_to_char( " ", ch );
    one_argument( obj->name, buf );
    send_to_char( buf, ch );
  }
  send_to_char( "{x.\n\r", ch );

  printf_to_char( ch, "{cState [{x %2d {c]{x\n\r", ch->in_room->state );

  for ( door = 0; door <= 5; door++ )
  {
    EXIT_DATA *pexit;

    if ( ( pexit = location->exit[door] ) != NULL )
    {
      char *state;
      int i, length;
      char word[MAX_INPUT_LENGTH];
      char reset_state[MAX_STRING_LENGTH];
      char buf1[MAX_STRING_LENGTH];
      /*
       * Format up the exit info.
       * Capitalize all flags that are not part of the reset info.
       */
      strcpy( reset_state, flag_string( exit_flags, pexit->rs_flags ) );
      state = flag_string( exit_flags, pexit->exit_info );
      strcpy( buf1, " " );
      for (; ;)
      {
        state = one_argument( state, word );
        if ( word[0] == '\0' )
          break;

        if ( str_infix( word, reset_state ) )
        {
          length = strlen(word);
          for (i = 0; i < length; i++)
            word[i] = UPPER(word[i]);
        }
        strcat( buf1, word );
        strcat( buf1, " " );
      }

      printf_to_char(ch,
                     "{cDoor: {W%d{c.  To: {W%d{c.  Key: {W%d{c.  Exit flags: {g[{W%s{g]{x\n\r{cKeyword: '{W%s{c'.  Description: {x%s",
                     door,
                     (pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
                     pexit->key,
                     buf1,
//             pexit->exit_info,
                     pexit->keyword,
                     pexit->description[0] != '\0'
                     ? pexit->description : "(none).\n\r" );
    }
  }
  return;
}

void show_ostat(CHAR_DATA *ch, OBJ_DATA *obj)
{
  OBJ_INDEX_DATA *pObjIndex;
  bool found2 = FALSE;
  char buf[MSL];
  int vnum=0;
  bool suppresscomma=TRUE;

  if (IS_IMMORTAL(ch))
  {
    printf_to_char(ch, "{YStatistics for Obj: {C({W%s{C){x\n\r",
                   obj->name );

    printf_to_char(ch, "{cVnum: {W%d  {cFormat: {W%s  {cType: {W%s  {cResets: {W%d{x\n\r",
                   obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
                   item_name(obj->item_type), obj->pIndexData->reset_num );

    printf_to_char(ch, "{cShort description: {x%s\n\r{cLong description: {x%s{x\n\r",
                   obj->short_descr, obj->description );

    if (obj->pIndexData->clan_name)
      printf_to_char(ch, "{cClan:  {x%s\n\r", obj->pIndexData->clan_name );

    if ( obj->famowner)
      printf_to_char(ch, "{cOwner: {x%s\n\r", obj->famowner );

    if (obj->owner)
      printf_to_char(ch, "{cOwner: {x%s\n\r", obj->owner );

    printf_to_char(ch, "{cWear bits: {g[{W%s{g]{x\n\r{cExtra bits: {g[{W%s{g]{x\n\r",
                   wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );

    printf_to_char(ch, "{cNumber: {g<{W%d{g/{W%d{g>{c  Weight: {g<{W%d{g/{W%d{g/{W%d{g>{c (10th pounds){x\n\r",
                   1,           get_obj_number( obj ),
                   obj->weight, get_obj_weight( obj ),get_true_weight(obj) );

    printf_to_char(ch, "{cLevel: {W%d  {cCost: {W%d  {cCondition: {W%d  {cTimer: {W%d{x\n\r",
                   obj->level, obj->cost, obj->condition, obj->timer );

    printf_to_char(ch,
                   "{cIn room: {W%d  {cIn object: {W%s  {cCarried by: {W%s  {cWear_loc: {W%d\n\r",
                   obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
                   obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
                   obj->carried_by == NULL    ? "(none)" :
                   can_see(ch,obj->carried_by) ? obj->carried_by->name
                   : "someone",
                   obj->wear_loc );

    printf_to_char(ch, "{cValues: {W%d %d %d %d %d %d %d{x\n\r",
                   obj->value[0], obj->value[1], obj->value[2], obj->value[3],
                   obj->value[4], obj->value[5], obj->value[6] );
  }
  else
  {
    mprintf( sizeof(buf), buf,
             "'%s' is type {W%s{x, extra affects %s.\n\rWeight is %d"
             ", value is {W%d{x, level is %d.\n\r",
             obj->name,
             item_name(obj->item_type),
             extra_bit_name( obj->extra_flags ),
             obj->weight / 10,
             obj->cost,
             obj->level
           );
    send_to_char( buf, ch );

  }

  switch ( obj->item_type )
  {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
      printf_to_char(ch, "{cLevel {W%d {cspells of:{W", obj->value[0] );

      if ( ( obj->value[1] >= 0 )
           && ( obj->value[1] < MAX_SKILL )
           && ( str_cmp( skill_table[obj->value[1]].name, "reserved" ) ) )
      {
        send_to_char( " '", ch );
        send_to_char( skill_table[obj->value[1]].name, ch );
        send_to_char( "'", ch );
        suppresscomma = FALSE;
      }

      if ( ( obj->value[2] >= 0 )
           && ( obj->value[2] < MAX_SKILL )
           && ( str_cmp( skill_table[obj->value[2]].name, "reserved" ) ) )
      {
        if (suppresscomma)
          send_to_char( "'", ch );
        else
          send_to_char( ", '", ch );
        send_to_char( skill_table[obj->value[2]].name, ch );
        send_to_char( "'", ch );
        suppresscomma = FALSE;
      }

      if ( ( obj->value[3] >= 0 )
           && ( obj->value[3] < MAX_SKILL )
           && ( str_cmp( skill_table[obj->value[3]].name, "reserved" ) ) )
      {
        if (suppresscomma)
          send_to_char( "'", ch );
        else
          send_to_char( ", '", ch );
        send_to_char( skill_table[obj->value[3]].name, ch );
        send_to_char( "'", ch );
        suppresscomma = FALSE;
      }

      if ( ( obj->value[4] >= 0 )
           && ( obj->value[4] < MAX_SKILL)
           && ( str_cmp( skill_table[obj->value[4]].name, "reserved" ) ) )
      {
        if (suppresscomma)
          send_to_char( "'", ch );
        else
          send_to_char( ", '", ch );
        send_to_char(skill_table[obj->value[4]].name,ch);
        send_to_char("'",ch);
      }

      send_to_char( "{x\n\r", ch );
      break;

    case ITEM_CORPSE_NPC:
      if (IS_IMMORTAL(ch))
        printf_to_char(ch, "Fragment Value is {W%d{x\n\r", obj->value[0] );
      break;

    case ITEM_SPELLBOOK:
      printf_to_char(ch, "{cCan hold {W%d{c scrolls, up to level {W%d{c.{x\n\r",
                     obj->value[0], obj->value[1] );
      break;

    case ITEM_KEYRING:
      printf_to_char(ch, "{cCan hold {W%d{c keys.{x\n\r", obj->value[0] );
      break;

    case ITEM_SCRY_MIRROR:
      printf_to_char(ch, "{cHas {W%d{g({W%d{g){c charges.{x\n\r",
                     obj->value[1], obj->value[0] );
      break;

    case ITEM_QUIVER:
      printf_to_char(ch, "{cCan hold {W%d{c projectiles, cutting item count/weight by {W%d{c.{x\n\r",
                     obj->value[0], obj->value[1] );
      break;

    case ITEM_PROJECTILE:
      if (IS_SET( obj->value[0], ITEM_PROJECTILE_MAGICBOMB))
        printf_to_char(ch, "{cCasts level %d {W%s{x.{x\n\r", obj->value[4], skill_table[obj->value[3]].name);
      else
      {
        send_to_char("{cProjectile is usable by: {W",ch);
        if (IS_SET(obj->value[0], ITEM_PROJECTILE_THROW))
          send_to_char("thrown ",ch);
        if (IS_SET(obj->value[0], ITEM_PROJECTILE_BOMB))
          send_to_char("bomb ",ch);
        if (IS_SET(obj->value[0], ITEM_PROJECTILE_SLING))
          send_to_char("sling ",ch);
        if (IS_SET(obj->value[0], ITEM_PROJECTILE_BOW))
          send_to_char("bow ",ch);
        if (IS_SET(obj->value[0], ITEM_PROJECTILE_CROSSBOW))
          send_to_char("crossbow ",ch);
        if (IS_SET(obj->value[0], ITEM_PROJECTILE_ATLATL))
          send_to_char("atlatl ",ch);
        send_to_char("\n\r",ch);

        printf_to_char(ch,"{cDamage is {W%dd%d {g({caverage {W%d{g){x\n\r", obj->value[1],obj->value[2], (1 + obj->value[2]) * obj->value[1] / 2);
      }
      break;

    case ITEM_WAND:
    case ITEM_STAFF:
      printf_to_char(ch, "{cHas {W%d{g({W%d{g){c charges of level {W%d",
                     obj->value[2], obj->value[1], obj->value[0] );

      if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
      {
        send_to_char( " '", ch );
        send_to_char( skill_table[obj->value[3]].name, ch );
        send_to_char( "'", ch );
      }

      send_to_char( "{x.\n\r", ch );
      break;
    case ITEM_DRINK_CON:
      printf_to_char(ch,"{cIt holds {W%s{c-colored {W%s{x.\n\r",
                     liq_table[obj->value[2]].liq_color,
                     liq_table[obj->value[2]].liq_name);
      break;


    case ITEM_CONTAINER:
      printf_to_char(ch,"{cCapacity: {W%d{c#  {cMaximum weight: {W%d{c#  {cflags: {g[{W%s{g]{x\n\r",
                     obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
      if (obj->value[4] != 100)
      {
        printf_to_char(ch,"{cWeight multiplier: {W%d%%{x\n\r",
                       obj->value[4]);
      }
      break;
    case ITEM_ARMOR:
      printf_to_char( ch, "Item is worn in the %s location.\n\r",
                      flag_string(
                        wear_flags, obj->wear_flags&~(ITEM_TAKE|ITEM_NO_SAC) ) );
      printf_to_char(ch,
                     "{cArmor class is {W%d {cpierce, {W%d {cbash, {W%d {cslash, and {W%d{c vs. magic{x\n\r",
                     obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
      break;

    case ITEM_JEWELRY: // make sure users don't see "item is worn in the none location"
      if ( (obj->wear_flags != 0)
           &&  (obj->wear_flags != ITEM_TAKE) )
      {
        printf_to_char( ch, "Item is worn in the %s location.\n\r",
                        flag_string( wear_flags, obj->wear_flags&~(ITEM_TAKE|ITEM_NO_SAC) ) );
      }
      break;

    case ITEM_TREASURE:
      if ( (obj->wear_flags != 0)
           &&  (obj->wear_flags != ITEM_TAKE) )
      {
        printf_to_char( ch, "Item is worn in the %s location.\n\r",
                        flag_string( wear_flags, obj->wear_flags&~(ITEM_TAKE|ITEM_NO_SAC) ) );
      }
      break;

    case ITEM_WEAPON:
      send_to_char("Weapon type is ",ch);
      switch (obj->value[0])
      {
        case(WEAPON_EXOTIC) : send_to_char("exotic.\n\r",ch);
          break;
        case(WEAPON_SWORD)  : send_to_char("sword.\n\r",ch);
          break;
        case(WEAPON_DAGGER) : send_to_char("dagger.\n\r",ch);
          break;
        case(WEAPON_SPEAR)    : send_to_char("spear/staff.\n\r",ch);
          break;
        case(WEAPON_MACE)     : send_to_char("mace/club.\n\r",ch);
          break;
        case(WEAPON_AXE)    : send_to_char("axe.\n\r",ch);
          break;
        case(WEAPON_FLAIL)    : send_to_char("flail.\n\r",ch);
          break;
        case(WEAPON_WHIP)    : send_to_char("whip.\n\r",ch);
          break;
        case(WEAPON_POLEARM): send_to_char("polearm.\n\r",ch);
          break;
				case(WEAPON_CROSSBOW): send_to_char("crossbow.\n\r",ch);
					break;
        default        :
          send_to_char("unknown.\n\r",ch);
          break;
      }

      if (obj->pIndexData->new_format)
        printf_to_char(ch,"{c%s damage is {W%dd%d {g({caverage {W%d{g){x\n\r",
                       (obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ?
                       capitalize( attack_table[obj->value[3]].noun ) : "Undefined",
                       obj->value[1],obj->value[2],
                       (1 + obj->value[2]) * obj->value[1] / 2);
      else
        printf_to_char(ch, "{cDamage is {W%d {cto {W%d {g({caverage {W%d{c){x\n\r",
                       obj->value[1], obj->value[2],
                       ( obj->value[1] + obj->value[2] ) / 2 );

//      if (IS_IMMORTAL(ch))
//    printf_to_char(ch,"{cDamage noun is {W%s{x.\n\r",
//               (obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ?
//               attack_table[obj->value[3]].noun : "undefined");

      if (obj->value[4])    /* weapon flags */
      {
        printf_to_char(ch,"{cWeapons affects: {g[{W%s{g]{x\n\r",
                       weapon_bit_name(obj->value[4]));
      }

      break;

    case ITEM_KEY:
      if (IS_IMMORTAL(ch))
      {
        int nMatch=0;
        found2 = FALSE;
        for (vnum = 0; nMatch < top_obj_index; vnum++)
        {
          if ((pObjIndex = get_obj_index(vnum)) != NULL)
          {
            nMatch++;
            if (pObjIndex->item_type == ITEM_CONTAINER)
              if (pObjIndex->value[2] == obj->pIndexData->vnum)
              {
                printf_to_char(ch,"{cThis Key unlocks vnum [{W%d{c]{x\n\r", vnum);
                found2 = TRUE;
              }
          }
        }
        if (!found2)
          printf_to_char(ch,"{RThis Key has no Object associated!!\n\r");
      }
      break;
  }


}

void do_ostat( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;
  char buf[MSL];
  bool made_obj =FALSE;
  bool found = FALSE;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Stat what?\n\r", ch );
    return;
  }

  if (is_number(argument))
  {
    if ( ( pObjIndex = get_obj_index( atoi( argument ) ) ) == NULL )
    {
      send_to_char( "No object has that vnum.\n\r", ch );
      return;
    }
    made_obj = TRUE;
    obj = create_object( pObjIndex, get_trust(ch) );
  }
  else if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
  {
    send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
    return;
  }
  /* now give out vital statistics as per identify */


  show_ostat(ch, obj);

  if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
  {
    EXTRA_DESCR_DATA *ed;

    send_to_char( "{cExtra description keywords: '{W", ch );

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
      send_to_char( ed->keyword, ch );
      if ( ed->next != NULL )
        send_to_char( " ", ch );
    }

    for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
    {
      send_to_char( ed->keyword, ch );
      if ( ed->next != NULL )
        send_to_char( " ", ch );
    }

    send_to_char( "{x'\n\r", ch );
  }

  send_to_char("{c---------------------------------------------------------------\n\r",ch);
  send_to_char("Lvl ApplyTo              Mod   Hrs Special Affects\n\r",ch);
  send_to_char("---------------------------------------------------------------{x\n\r",ch);
  for ( paf = obj->affected; paf != NULL; paf = paf->next )
  {
    found = TRUE;
    if (paf->bitvector)
    {
      switch (paf->where)
      {
        case TO_AFFECTS:
          mprintf(sizeof(buf),buf,"{cAdds {W%s{c affect.{x",   affect_bit_name(paf->bitvector));
          break;
        case TO_AFFECTS2:
          mprintf(sizeof(buf),buf,"{cAdds {x%s{c affect2.{x", affect2_bit_name(paf->bitvector));
          break;
        case TO_WEAPON:
          mprintf(sizeof(buf),buf,"{cAdds {W%s{c weapon flags.{x",  weapon_bit_name(paf->bitvector));
          break;
        case TO_OBJECT:
          mprintf(sizeof(buf),buf,"{cAdds {W%s{c object flag.{x",   extra_bit_name(paf->bitvector));
          break;
        case TO_IMMUNE:
          mprintf(sizeof(buf),buf,"{cAdds immunity to {W%s{c.{x",    imm_bit_name(paf->bitvector));
          break;
        case TO_RESIST:
          mprintf(sizeof(buf),buf,"{cAdds resistance to {W%s{c.{x",    imm_bit_name(paf->bitvector));
          break;
        case TO_VULN:
          mprintf(sizeof(buf),buf,"{cAdds vulnerability to {W%s{c.{x",  imm_bit_name(paf->bitvector));
          break;
        default:
          mprintf(sizeof(buf),buf,"{cUnknown bit {W%d{c: {W%d{x",   paf->where,paf->bitvector);
          break;
      }
      mprintf(sizeof(buf),buf,"%25s",buf);
      printf_to_char(ch,"{W%3d {g[{W%-16s{g]{W %5d %5d {W%-25s{x\n\r",
                     paf->level, affect_loc_name( paf->location),
                     paf->modifier, paf->duration,buf);
    }
    else
      printf_to_char(ch,"{W%3d {g[{W%-16s{g]{W %5d %5d {W%-25s{x\n\r",
                     paf->level, affect_loc_name( paf->location),
                     paf->modifier, paf->duration,"None");
  }

  if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
      found = TRUE;
      if (paf->bitvector)
      {
        switch (paf->where)
        {
          case TO_AFFECTS:
            mprintf(sizeof(buf),buf,"{cAdds {W%s{c affect.{x", affect_bit_name(paf->bitvector));
            break;
          case TO_AFFECTS2:
            mprintf(sizeof(buf),buf,"{cAdds {x%s{c affect2.{x",
                    affect2_bit_name(paf->bitvector));
            break;
          case TO_OBJECT:
            mprintf(sizeof(buf),buf,"{cAdds {W%s{c object flag.{x", extra_bit_name(paf->bitvector));
            break;
          case TO_IMMUNE:
            mprintf(sizeof(buf),buf,"{cAdds immunity to {W%s{c.{x", imm_bit_name(paf->bitvector));
            break;
          case TO_RESIST:
            mprintf(sizeof(buf),buf,"{cAdds resistance to {W%s{c.{x", imm_bit_name(paf->bitvector));
            break;
          case TO_VULN:
            mprintf(sizeof(buf),buf,"{cAdds vulnerability to {W%s{c.{x", imm_bit_name(paf->bitvector));
            break;
          default:
            mprintf(sizeof(buf),buf,"{cUnknown bit {W%d{c: {W%d{x", paf->where,paf->bitvector);
            break;
        }
        mprintf(sizeof(buf),buf,"%25s",buf);
        printf_to_char(ch,"{W%3d {g[{W%-16s{g]{W %5d %5d {W%-25s{x\n\r",
                       paf->level, affect_loc_name( paf->location),
                       paf->modifier, paf->duration,buf);
      }
      else
        printf_to_char(ch,"{W%3d {g[{W%-16s{g]{W %5d %5d {W%-25s{x\n\r",
                       paf->level, affect_loc_name( paf->location),
                       paf->modifier, paf->duration,"None");
    }
  if (!found)
    send_to_char("{WNo Affects.{x\n\r",ch);
  send_to_char("{c---------------------------------------------------------------{x\n\r",ch);
  if  (made_obj)
    extract_obj( obj);

  return;
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  char buf[MSL],buf2[MSL];
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *victim;
  bool made_mob = FALSE;
  bool found = FALSE;
  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Stat whom?\n\r", ch );
    return;
  }
  if ( is_number( argument ) )
  {
    if ( ( pMobIndex = get_mob_index( atoi( argument ) ) ) == NULL )
    {
      send_to_char( "No mob has that vnum.\n\r", ch );
      return;
    }
    victim = create_mobile( pMobIndex );
    made_mob = TRUE;
    char_to_room( victim, get_room_index( ROOM_VNUM_LIMBO ) ); // prevent NULL room bug
  }
  else if ( ( victim = get_char_world( ch, argument ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  printf_to_char( ch, "{cStatistics for {x( %s ):\n\n\r", victim->name );

  printf_to_char(ch,
                 "{cLv:       {y%-3d    {cClass: {W%-10s  {cAlign: {W%-5d  {yGold: {W%ld  {wSilver: {W%ld{x\n\r",
                 victim->level,
                 IS_NPC( victim ) ? "mobile" : class_table[victim->gameclass].name,
                 victim->alignment,
                 victim->gold, victim->silver );

  printf_to_char(ch,
                 "{cVnum: {W[{y%6d {x]  {cFormat: {W%s  {cRace: {W%s  {cGroup: {W%d  {cSex: {W%s  {cRoom: {W%d{x\n\n\r",
                 IS_NPC(victim) ? victim->pIndexData->vnum : 0,
                 IS_NPC(victim) ? victim->pIndexData->new_format ?
               "new" : "old" : "pc ",
                 race_table[victim->race].name,
                 IS_NPC(victim) ? victim->group : 0, sex_table[victim->sex].name,
                 victim->in_room == NULL    ?        0 : victim->in_room->vnum
                );

  if ( !IS_NPC( victim )
       &&   victim->level < 100 )
  {
    int XPL = exp_per_level( victim, victim->pcdata->points );
    printf_to_char( ch,
                    "{gExperience {W%d  {gNext Level {W%d  {gPer Level {W%d {gNeeded {W%d\n\r{x",
                    victim->exp,
                    TNL( XPL, victim->level ),
                    TNL( XPL, victim->level ) - TNL( XPL, victim->level - 1 ),
                    TNL( XPL, victim->level ) - victim->exp );
  }

  if (IS_NPC(victim))
  {
    printf_to_char(ch,
                   "{cCount: {W%d  {cKilled: {W%d   {cZone: {W%s  {cResetroom: {x%d\n\r",
                   victim->pIndexData->count,victim->pIndexData->killed,
                   ( victim->zone ? victim->zone->name : "NULL" ),
                   ( victim->reset_room ? victim->reset_room->vnum : 0 ) );
  }

  printf_to_char(ch,
                 "{cStr: {W%d(%d)  {cInt: {W%d(%d)  {cWis: {W%d(%d)  {cDex: {W%d(%d)  {cCon: {W%d(%d)\n\r",
                 victim->perm_stat[STAT_STR],
                 get_curr_stat(victim,STAT_STR),
                 victim->perm_stat[STAT_INT],
                 get_curr_stat(victim,STAT_INT),
                 victim->perm_stat[STAT_WIS],
                 get_curr_stat(victim,STAT_WIS),
                 victim->perm_stat[STAT_DEX],
                 get_curr_stat(victim,STAT_DEX),
                 victim->perm_stat[STAT_CON],
                 get_curr_stat(victim,STAT_CON) );

  if ( !IS_NPC( victim ) )
  {
    printf_to_char(ch, "{cHp: {g<{W%d{g/{W%d{g({R%d{g)> {cMana: {g<{W%d{g/{W%d{g>{c Move {g<{W%d{g/{W%d{g>{x\n\r",
                   victim->hit,  GET_HP(victim),//victim->max_hit,
                   victim->pcdata->perm_hit,
                   victim->mana, GET_MANA(victim),//victim->max_mana,
                   victim->move, victim->max_move,
                   victim->exp   );
    if (victim->pcdata->clanowe_clan)
    {
      printf_to_char(ch, "{cNumClan: {W%d {cOwes{x[%s][{W%d{x] {cdiamond Owes Level[{W%d{c].{x\n\r",
                     victim->pcdata->numclans,
                     victim->pcdata->clanowe_clan,
                     victim->pcdata->clanowe_dia,
                     victim->pcdata->clanowe_level );
    }
    else if ( victim->pcdata->numclans )
    {
      printf_to_char( ch, "{cNumClans: {W%d\n\r",
                      victim->pcdata->numclans );
    }
    printf_to_char( ch, "{cBank: {Y%d.\n\r", victim->pcdata->balance );
  }
  else
  {
    printf_to_char(ch, "{cHp: {g<{W%d{g/{W%d{g> {cMana: {g<{W%d{g/{W%d{g>{c Move {g<{W%d{g/{W%d{g>{x\n\r",
                   victim->hit,  victim->max_hit,
                   victim->mana, victim->max_mana,
                   victim->move, victim->max_move,
                   victim->exp );
  }
  printf_to_char(ch, "{cCarry number: {g<{W%d{g/{W%d{g>  {cCarry weight: {g<{W%ld{g/{W%ld{g>{x\n\r",
                 victim->carry_number, can_carry_n(victim),
                 get_carry_weight(victim) / 10 ,can_carry_w(victim) /10);

  printf_to_char(ch,"{cArmor: pierce: {W%d  {cbash: {W%d  {cslash: {W%d  {cmagic: {W%d{x\n\r",
                 GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
                 GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));

  printf_to_char(ch,
                 "{cHit: {W%d  {cDam: {W%d  {cSaves: {W%d  {cSize: {W%s  {cPosition: {W%s  {cWimpy: {W%d{x\n\r",
                 GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
                 size_table[victim->size].name, position_table[victim->position].name,
                 victim->wimpy );

  if (IS_NPC(victim) && victim->pIndexData->new_format)
  {
    printf_to_char(ch, "{cDamage: {W%dd%d  {cMessage:  {W%s{c\n\r",
                   victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
                   attack_table[victim->dam_type].noun);
  }
  printf_to_char(ch, "{cFighting: {R%s{x\n\r",
                 victim->fighting ? victim->fighting->name : "(none)" );

  if ( !IS_NPC(victim) )
  {
    printf_to_char(ch,
                   "{cThirst: {W%d  {cHunger: {W%d  {cFull: {W%d  {cDrunk: {W%d  {cDegrading {W%d{x\n\r",
                   victim->pcdata->condition[COND_THIRST],
                   victim->pcdata->condition[COND_HUNGER],
                   victim->pcdata->condition[COND_FULL],
                   victim->pcdata->condition[COND_DRUNK],
                   victim->pcdata->degrading);
    printf_to_char(ch,
                   "{cPracs: {W%d {cTrains: {W%d {cQuest Points: {W%d{x\n\r",
                   victim->practice,
                   victim->train,
                   victim->questdata->curr_points);
    printf_to_char(ch,
                   "{cAge: {W%d  {cPlayed: {W%d  {cLast Level: {W%d  {cTimer: {W%d{x\n\r",
                   get_age(victim),
                   TOTAL_PLAY_TIME(victim) / 3600,
                   victim->pcdata->last_level,
                   victim->timer );
  }

  printf_to_char(ch, "{cAct: {g[{W%s{g]{x\n\r",act_bit_name(victim->act));

  if (victim->act2)
    printf_to_char(ch, "{cAct2: {g[{W%s{g]{x\n\r",act2_bit_name(victim->act2));

  if (victim->plr2)
    printf_to_char(ch, "{cPlr2: {g[{W%s{g]{x\n\r",plr2_bit_name(victim->plr2));

  if (victim->comm_flags)
  {
    printf_to_char(ch,"{cComm: {g[{W%s{g]{x\n\r",comm_bit_name(victim->comm_flags));
  }
  if (victim->chan_flags)
  {
    printf_to_char(ch,"{cChannel: {g[{W%s{g]{x\n\r",chan_bit_name(victim->chan_flags));
  }

  if (victim->pen_flags)
  {
    printf_to_char(ch,"{cPenalty: {g[{W%s{g]{x\n\r",pen_bit_name(victim->pen_flags));
  }

  if (IS_NPC(victim) && victim->off_flags)
  {
    printf_to_char(ch, "{cOffense: {g[{W%s{g]{x\n\r",off_bit_name(victim->off_flags));
  }

  if (victim->imm_flags)
  {
    printf_to_char(ch, "{cImmune:  {g[{W%s{g]{x\n\r",imm_bit_name(victim->imm_flags));
  }

  if (victim->res_flags)
  {
    printf_to_char(ch, "{cResist:  {g[{W%s{g]{x\n\r", imm_bit_name(victim->res_flags));
  }

  if (victim->vuln_flags)
  {
    printf_to_char(ch, "{cVuln:    {g[{W%s{g]{x\n\r", imm_bit_name(victim->vuln_flags));
  }

  printf_to_char(ch, "{cForm:    {g[{W%s{g]\n\r{cParts:   {g[{W%s{g]{x\n\r",
                 form_bit_name(victim->form), part_bit_name(victim->parts));

  if (victim->affected_by)
  {
    printf_to_char(ch, "{cAffected by {g[{W%s{g]{x\n\r",
                   affect_bit_name(victim->affected_by));
  }

  if (victim->affected2_by)
    printf_to_char(ch, "{cAffected2 by {g[{w%s{g]{x\n\r",
                   affect2_bit_name(victim->affected2_by));

  if (victim->affected_by)
  {
    printf_to_char(ch, "{cSpell Affected by {g[{W%s{g]{x\n\r",
                   spell_affect_bit_name(victim->spell_aff));
  }

  printf_to_char(ch, "{cMaster: {W%s  {cLeader: {W%s  {cPet: {W%s ",
                 victim->master      ? victim->master->name   : "(none)",
                 victim->leader      ? victim->leader->name   : "(none)",
                 victim->pet         ? victim->pet->name         : "(none)");
  printf_to_char( ch, "{cGroup Num: {W%ld{x\n\r",
                  victim->group_num );
  printf_to_char( ch, "{cHating: {W%s{x  ",
                  victim->hate    ? victim->hate->name : "(none)");
  printf_to_char( ch, "{cAttacker: {W%s{x   ",
                  victim->plevel    ? victim->plevel->name : "(none)");
  printf_to_char( ch, "{cGroup Fight: {W%ld{x\n\r", victim->group_fight);


  if (!IS_NPC(victim))
  {
    printf_to_char(ch, "{cSecurity: {W%d\n\r", victim->pcdata->security ); /* OLC */
  }


  printf_to_char(ch, "{cShort description: {W%s\n\r{cLong  description: {W%s{x",
                 victim->short_descr,
                 victim->long_descr[0] ? victim->long_descr : "(none)\n\r" );
  if ( IS_NPC( victim ) && victim->path && victim->path[0] )
  {
    printf_to_char( ch, "\n\r{cPath:    {x%s\n\r{cPathptr: {x%s\n\r",
                    victim->path, victim->path_ptr );

  }

  if ( IS_NPC( victim )
       &&   IS_SET(victim->act2,ACT2_MOUNTABLE) )
    printf_to_char( ch, "\n\r{cMob is mountable and can carry {x%d {cpassengers.\n\r",
                    victim->max_riders);

  if ( IS_NPC(victim) && victim->spec_fun )
  {
    printf_to_char(ch,"{cMobile has special procedure {W%s.{x\n\r",
                   spec_name(victim->spec_fun));
  }

  if ( IS_NPC(victim) )
  {
    MOB_INDEX_DATA *pMob = victim->pIndexData;
    if ( pMob->mprogs )
    {
      MPROG_LIST *list;
      int cnt;

      printf_to_char(ch,"{cMobile has MProgs:{x");
      for ( cnt = 0, list = pMob->mprogs; list; list = list->next, cnt++ )
      {
        printf_to_char( ch, "{w %d {D(%s)", list->vnum,
                        ((list->code == NULL) ? "{r?" : ((list->code->name == NULL) ? "{Y?" : list->code->name)) );
        if (list->next != NULL)
          printf_to_char( ch, "{w, " );
      }
      printf_to_char( ch, "{x\n\r");
    }


  }

  send_to_char("{c-------------------------------------------------------------------------{x\n\r",ch);
  send_to_char("{cLvl Spells            Duration  Mod ApplyTo            Adds\n\r",ch);
  send_to_char("{c-------------------------------------------------------------------------{x\n\r",ch);
  for ( paf = victim->affected; paf; paf = paf->next )
  {
    if (paf->where == TO_AFFECTS)
    {
      strncpy_color(buf2, flag_string( affect_flags ,  paf->bitvector ), 16, ' ', TRUE );
      mprintf(sizeof(buf),buf, "{g[{W%-16s{g]{x", buf2);
    }

    else if (paf->where == TO_SPELL_AFFECTS)
    {
      strncpy_color(buf2, flag_string( spell_affect_flags ,  paf->bitvector ), 16, ' ', TRUE );
      mprintf(sizeof(buf),buf, "{g[{W%-16s{g]{x", buf2);
    }
    else if (paf->where == TO_AFFECTS2)
    {
      strncpy_color(buf2, flag_string( affect2_flags,  paf->bitvector ), 16, ' ', TRUE );
      mprintf(sizeof(buf),buf, "{g[{W%-16s{g]{x", buf2 );
    }
    else
      mprintf(sizeof(buf),buf,"{g[{W%-16s{g]{x","none");

    strncpy_color(buf2,skill_table[(int) paf->type].name, 16, ' ', TRUE );
    printf_to_char(ch,"{W%3d {g[{W%-16s{g]{W %5d %5d  {g[{W%-16s{g] %s{x\n\r",
                   paf->level,
                   buf2,
                   paf->duration,
                   paf->modifier,
                   affect_loc_name( paf->location ),
                   buf );
    found = TRUE;
  } // paf loop

  if (!found)
    printf_to_char(ch,"{WNo affects.\n\r");

  send_to_char("{c-------------------------------------------------------------------------{x\n\r",ch);

  if (made_mob)
    extract_char(victim, TRUE);

  return;
}

void do_charstat( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MSL],buf2[MSL],buf3[MSL];
  BUFFER *buffer;
  AFFECT_DATA *paf;
  CHAR_DATA *victim;
  one_argument( argument, arg );
  bool afffound = FALSE;
  OBJ_DATA *obj;
  int iWear = 0;
  int flag = 0;

  if ( ( ( victim = get_char_world( ch, argument ) ) == NULL )
  || IS_NPC( victim ) )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  buffer = new_buf();

  sprintf(buf, "{g*{D----------------------------{g*{D-------------------{g*{D----------------------{g*{x\n\r" );
  add_buf(buffer,buf);

  sprintf(buf,
          "{D| {gName  {x: %-17s  {D| {cLevel  {x:     %3d  {D| {mSize    {x:%10s  {D|{x\n\r",
          victim->name,
          victim->level,
          size_table[victim->size].name );
  add_buf(buffer,buf);

  sprintf(buf,
          "{D| {gRace  {x: %-17s  {D| {cSex    {x:%8s  {D| {mPosition{x:%10s  {D|{x\n\r",
          race_table[victim->race].name,
          sex_table[victim->sex].name,
          position_table[victim->position].name );
  add_buf(buffer,buf);

  sprintf(buf,
          "{D| {gClass {x: %-17s  {D| {cPracs  {x:    %4d  {g*{D----------------------{g*{x\n\r",
          class_table[victim->gameclass].name,
          victim->practice );
  add_buf(buffer,buf);

  if (victim->clan)
    strncpy_color(buf2, victim->clan->symbol, 17, ' ', TRUE );
  else
    strcpy(buf2, "None");

  sprintf(buf,
          "{D| {DClan  {x: %-17s  {D| {cTrains {x:    %4d  {D| {WAge     {x:      %4d  {D|{x\n\r",
          buf2,
          victim->train,
          get_age(victim) );
  add_buf(buffer,buf);

  if (victim->clan)
    strncpy_color(buf2, get_rank( victim ), 17, ' ', TRUE );
  else
    strcpy(buf2, "Unclanned");

  sprintf(buf,
          "{D| {DRank  {x: %-17s  {D| {cIn Room{x:   %5d  {D| {WPlayed  {x:    %6d  {D|{x\n\r",
          buf2,
          ( victim->in_room == NULL ? 0 : victim->in_room->vnum ),
          ( TOTAL_PLAY_TIME(victim) / 3600 ) );
  add_buf(buffer,buf);

  sprintf(buf,
          "{g*{D----------------------------{g*{D-------------------{g*{D----------------------{g*{x\n\r" );
  add_buf(buffer,buf);

  sprintf(buf,
          "{D| {RH{rit{RP{rts{x: %5d{D/{x%-5d {r({x%4d{r) {D| {WAlign  {x:   %5d  {D| {yG{Yo{yld    {x:   %7ld  {D|{x\n\r",
          victim->hit,
          GET_HP( victim ),
          victim->pcdata->perm_hit,
          victim->alignment,
          victim->gold);
  add_buf(buffer,buf);

  sprintf(buf,
          "{D| {GM{gana  {x: %5d{D/{x%-5d {g({x%4d{g) {D| {bPierce {x:   %5d  {D| {wSil{Wv{wer  {x:   %7ld  {D|{x\n\r",
          victim->mana,
          GET_MANA(victim),
          victim->pcdata->perm_mana,
          GET_AC(victim,AC_PIERCE),
          victim->silver );
  add_buf(buffer,buf);

  sprintf(buf,
          "{D| {BM{bove  {x: %5d{D/{x%-5d        {D| {bBash   {x:   %5d  {D| {WBanked  {x:   %7d  {D|{x\n\r",
          victim->move,
          victim->max_move,
          GET_AC(victim,AC_BASH),
          victim->pcdata->balance );
  add_buf(buffer,buf);

  sprintf(buf,
          "{D| Items {x:  %4d{D/{x%-4d         {D| {bSlash  {x:   %5d  {g*{D----------------------{g*{x\n\r",
          victim->carry_number,
          can_carry_n(victim),
          GET_AC(victim,AC_SLASH) );
  add_buf(buffer,buf);

  sprintf(buf,
          "{D| Weight{x:%6d{D/{x%-7d      {D| {bMagic  {x:   %5d  {D| {RH{ritroll {x:      %4d  {D|{x\n\r",
          ( get_carry_weight(victim) / 10 ),
          ( can_carry_w(victim) /10 ),
          GET_AC(victim,AC_EXOTIC),
          GET_HITROLL(victim) );
  add_buf(buffer,buf);

  sprintf(buf,
          "{D| {WWimpy {x:  %4d              {D| {BSaves  {x:    %4d  {D| {xD{Damroll {x:      %4d  {D|{x\n\r",
          victim->wimpy,
          victim->saving_throw,
          GET_DAMROLL(victim) );
  add_buf(buffer,buf);

  sprintf(buf,
          "{g*{D----------------------------{g*{D-------------------{g*{D----------------------{g*{x\n\r" );
  add_buf(buffer,buf);

  sprintf(buf,
          "{D| {rR{Ru{rbi{Re{rs    {x:   %6d  {R({r%2d{R) {D| {WHunger {x:      %2d  {D| {gStrength    {x: %s%2d{D/{R%2d  {D|{x\n\r",
          victim->ruby_fragment,
          victim->ruby_counter,
          victim->pcdata->condition[COND_HUNGER],
          victim->perm_stat[STAT_STR] < get_max_train( victim, STAT_STR ) ? "{r" : "{R",
          victim->perm_stat[STAT_STR],
          get_curr_stat(victim,STAT_STR) );
  add_buf(buffer,buf);

  sprintf(buf,
          "{D| {cSap{Bp{bhir{Be{bs {x:   %6d  {B({c%2d{B) {D| {WThirst {x:      %2d  {D| {gDexterity   {x: %s%2d{D/{R%2d  {D|{x\n\r",
          victim->sapphire_fragment,
          victim->sapphire_counter,
          victim->pcdata->condition[COND_THIRST],
          victim->perm_stat[STAT_DEX] < get_max_train( victim, STAT_DEX ) ? "{r" : "{R",
          victim->perm_stat[STAT_DEX],
          get_curr_stat(victim,STAT_DEX) );
  add_buf(buffer,buf);

  sprintf(buf,
          "{D| {gE{Gme{gr{Ga{glds  {x:   %6d  {G({g%2d{G) {D| {WFull   {x:      %2d  {D| {gIntelligence{x: %s%2d{D/{R%2d  {D|{x\n\r",
          victim->emerald_fragment,
          victim->emerald_counter,
          victim->pcdata->condition[COND_FULL],
          victim->perm_stat[STAT_INT] < get_max_train( victim, STAT_INT ) ? "{r" : "{R",
          victim->perm_stat[STAT_INT],
          get_curr_stat(victim,STAT_INT) );
  add_buf(buffer,buf);

  sprintf(buf,
          "{D| {CDi{cam{Wo{wnd{Cs  {x:   %6d  {C({W%2d{C) {D| {WDrunk  {x:      %2d  {D| {gWisdom      {x: %s%2d{D/{R%2d  {D|{x\n\r",
          victim->diamond_fragment,
          victim->diamond_counter,
          victim->pcdata->condition[COND_DRUNK],
          victim->perm_stat[STAT_WIS] < get_max_train( victim, STAT_WIS ) ? "{r" : "{R",
          victim->perm_stat[STAT_WIS],
          get_curr_stat(victim,STAT_WIS) );
  add_buf(buffer,buf);

//        TNL( exp_per_level( victim, victim->pcdata->points ), victim->level )
  int tnlev = TNL( exp_per_level( victim, victim->pcdata->points ), victim->level ) - victim->exp;
  if (victim->level >= LEVEL_HERO)
    tnlev = 0;

  sprintf(buf,
          "{D| {YXP {yTot{w/{YTNL{w:%9llu{D/{x%-5d%s{D| {WDegrade{x:    %4d  {D| {gConstitution{x: %s%2d{D/{R%2d  {D|{x\n\r",
          victim->exp,
          tnlev,
          (tnlev > 99999) ? "" : " ", // if 6-digit TNL, let it  spill into extra space
          victim->pcdata->degrading,
          victim->perm_stat[STAT_CON] < get_max_train( victim, STAT_CON ) ? "{r" : "{R",
          victim->perm_stat[STAT_CON],
          get_curr_stat(victim,STAT_CON) );
  add_buf(buffer,buf);

  if ( IS_IMMORTAL( victim ) )
  {
    sprintf(buf,
            "{g*{D----------------------------{g*{D-------------------{g*{D----------------------{g*{x\n\r" );
    add_buf(buffer,buf);

    if ( victim->invis_level )
      sprintf( buf2, "%4d", victim->invis_level);
    else
      sprintf( buf2, "%4s", "off" );

    sprintf(buf,
            "{D| {wWizinvis  {x: %4s           {D| {cSecurity  {x:   %2d  {D| {DWkList Duhs {x:   %3d  {D|{x\n\r",
            buf2,
            victim->pcdata->security,
            victim->pcdata->wlduhs);
    add_buf(buffer,buf);

    if ( victim->incog_level )
      sprintf( buf2, "%4d", victim->incog_level );
    else
      sprintf( buf2, "%4s", "off" );

    sprintf(buf,
            "{D| {DIncognito {x: %4s           {D| {DWkLst Bugs{x:  %3d  {D| {DWkList Typos{x:   %3d  {D|{x\n\r",
            buf2,
            victim->pcdata->wlbugs,
            victim->pcdata->wltypos );
    add_buf(buffer,buf);

    sprintf( buf2, "%4s", IS_SET( victim->act, PLR_HOLYLIGHT) ? "on" : "off" );

    sprintf(buf,
            "{D| {YHoly Light{x: %4s           {D| {DWkLst Blds{x:  %3d  {D| {DWkList Helps{x:   %3d  {D|{x\n\r",
            buf2,
            victim->pcdata->wlbuild,
            victim->pcdata->wlhelps );
    add_buf(buffer,buf);
  } /* end IMM section */

  sprintf(buf,
          "{g*{D----------------------------{g*{D-------------------{g*{D----------------------{g*{x\n\r" );
  add_buf(buffer,buf);
// Pet date
  if (victim->pet)
  {
    if ( IS_NPC(victim->pet) )
      strncpy_color(buf2, victim->pet->short_descr, 15, ' ', TRUE );
    else
      strncpy_color(buf2, victim->pet->name, 15, ' ', TRUE );
  }
  else
    strcpy(buf2,"(none)");

  sprintf(buf,
          "{D| {yPet       {x: %-15s{D| Submitted WkLists {D| {mQuestPts{x:    %6d  {D|{x\n\r",
          buf2,
          victim->questdata->curr_points );
  add_buf(buffer,buf);

// Group date
  if (victim->leader)
  {
    if ( IS_NPC(victim->leader) )
      strncpy_color(buf2, victim->leader->short_descr, 15, ' ', TRUE );
    else
      strncpy_color(buf2, victim->leader->name, 15, ' ', TRUE );
  }
  else
    strcpy(buf2,"(none)");

  sprintf(buf,
          "{D| {yLeader    {x: %-15s{D|       Bugs{x:%5d  {D| {mTotalPts{x:    %6d  {D|{x\n\r",
          buf2,
          victim->pcdata->fdbugs,
          victim->questdata->accum_points );
  add_buf(buffer,buf);

// Fighting date
  if (victim->fighting)
  {
    if ( IS_NPC(victim->fighting) )
      strncpy_color(buf2, victim->fighting->short_descr, 15, ' ', TRUE );
    else
      strncpy_color(buf2, victim->fighting->name, 15, ' ', TRUE );
  }
  else
    strcpy(buf2,"(none)");

  sprintf(buf,
          "{D| {yFighting  {x: %-15s{D|      Typos{x:%5d  {D| {mAttempts{x:     %5d  {D|{x\n\r",
          buf2,
          victim->pcdata->fdtypos,
          victim->questdata->attempt_num );
  add_buf(buffer,buf);

  if (victim->mount)
  {
    if ( IS_NPC(victim->mount) )
      strncpy_color(buf2, victim->mount->short_descr, 15, ' ', TRUE );
    else
      strncpy_color(buf2, victim->mount->name, 15, ' ', TRUE );
  }
  else
    strcpy(buf2,"(none)");

  sprintf(buf,
          "{D| {yMount     {x: %-15s{D|      Helps{x:%5d  {D| {mComplete{x:     %5d  {D|{x\n\r",
          buf2,
          victim->pcdata->fdhelps,
          victim->questdata->comp_num );
  add_buf(buffer,buf);

  if ( (victim->pcdata->clanowe_dia < 1 )
       && ( victim->pcdata->clanowe_level < 1 ) )
  {
    sprintf(buf,
            "{D| {yGroup #   {x: %-15ld{D|       Duhs{x:%5d  {D| {mQuit    {x:       %3d  {D|{x\n\r",
            victim->group_num,
            victim->pcdata->fdduhs,
            victim->questdata->quit_num );
    add_buf(buffer,buf);
  }
  else
  {
    sprintf(buf,
            "{D| {cOwes Clan {x: %-10s %4d {cG{Ce{xm{cs{x, %2d {yLevels    {D| {mQuit    {x:       %3d  {D|{x\n\r",
            victim->pcdata->clanowe_clan,
            victim->pcdata->clanowe_dia,
            victim->pcdata->clanowe_level,
            victim->questdata->quit_num );
    add_buf(buffer,buf);
  }

  sprintf(buf,
          "{g*{D----------------------------{g*{D-------------------{g*{D----------------------{g*{x\n\r" );
  add_buf(buffer,buf);

// figure out how to truncate and continue (by word) at available print length onto next line
//  sprintf(buf,
//    "{D| {g     {x  {g[{xmorgue_corpse no_summon colour no_cancellation{g]               {D|{x\n\r",
//  add_buf(buffer,buf);

  if (victim->plr2)
  {
    sprintf(buf,
            "{D| {gPlr2 {x: {g[{x%-60s{g] {D|{x\n\r",
            plr2_bit_name(victim->plr2) );
    add_buf(buffer,buf);
  }

  if (victim->comm_flags)
  {
    sprintf(buf,
            "{D| {gComm {x: {g[{x%-60s{g] {D|{x\n\r",
            comm_bit_name(victim->comm_flags) );
    add_buf(buffer,buf);
  }

  if (victim->chan_flags)
  {
    sprintf(buf,
            "{D| {gChann{x: {g[{x%-60s{g] {D|{x\n\r",
            chan_bit_name(victim->chan_flags) );
    add_buf(buffer,buf);
  }

  if (victim->pen_flags)
  {
    sprintf(buf,
            "{D| {gPen  {x: {g[{x%-60s{g] {D|{x\n\r",
            pen_bit_name(victim->pen_flags) );
    add_buf(buffer,buf);
  }

  if (victim->imm_flags)
  {
    sprintf(buf,
            "{D| {gImm  {x: {g[{x%-60s{g] {D|{x\n\r",
            imm_bit_name(victim->imm_flags) );
    add_buf(buffer,buf);
  }

  if (victim->res_flags)
  {
    sprintf(buf,
            "{D| {gRes  {x: {g[{x%-60s{g] {D|{x\n\r",
            imm_bit_name(victim->res_flags) );
    add_buf(buffer,buf);
  }

  if (victim->vuln_flags)
  {
    sprintf(buf,
            "{D| {gVuln {x: {g[{x%-60s{g] {D|{x\n\r",
            imm_bit_name(victim->vuln_flags) );
    add_buf(buffer,buf);
  }

  sprintf(buf,
          "{g*{D-----------------------------------------------------------------------{g*{x\n\r" );
  add_buf(buffer,buf);

  sprintf(buf,"{D|{W Lvl     Spells         Dur  Mod      ApplyTo              Adds        {D|\n\r");
  add_buf(buffer,buf);

  sprintf(buf,"{g*{D-----------------------------------------------------------------------{g*{x\n\r");
  add_buf(buffer,buf);

// Show racial affects first
  for ( flag = 0; affect_flags[flag].name; flag++)
  {
    if ( !is_stat( affect_flags ) && IS_SET(race_table[victim->race].aff, affect_flags[flag].bit) )
    {
      afffound = TRUE;
      sprintf(buf, "{D| {W%3d {g[{WRacial          {g]{W Race Aff  {g[{Wnone           {g] {g[{W%-16s{g] {D|{x\n\r",
              victim->level,
              affect_flags[flag].name );
      add_buf(buffer,buf);
    }
    else
    {
      if ( affect_flags[flag].bit == race_table[victim->race].aff )
      {
        afffound = TRUE;
        sprintf(buf, "{D| {W%3d {g[{WRacial          {g]{W Race Aff  {g[{Wnone           {g] {g[{W%-16s{g] {D|{x\n\r",
                victim->level,
                affect_flags[flag].name );
        add_buf(buffer,buf);
        break;
      }
    }
  }

  for ( flag = 0; affect2_flags[flag].name; flag++)
  {
    if ( !is_stat( affect2_flags ) && IS_SET(race_table[victim->race].aff2, affect2_flags[flag].bit) )
    {
      afffound = TRUE;
      sprintf(buf, "{D| {W%3d {g[{WRacial          {g]{W Race Aff  {g[{Wnone           {g] {g[{W%-16s{g] {D|{x\n\r",
              victim->level,
              affect2_flags[flag].name );
      add_buf(buffer,buf);
    }
    else
    {
      if ( affect2_flags[flag].bit == race_table[victim->race].aff2 )
      {
        afffound = TRUE;
        sprintf(buf, "{D| {W%3d {g[{WRacial          {g]{W Race Aff  {g[{Wnone           {g] {g[{W%-16s{g] {D|{x\n\r",
                victim->level,
                affect2_flags[flag].name );
        add_buf(buffer,buf);
        break;
      }
    }
  }

// Show object affects second
  for ( iWear = 0; iWear < (MAX_WEAR+1); iWear++ )
  {
    if (iWear == MAX_WEAR + 1 )
    {
      if ( ( obj = get_eq_char(victim, WEAR_WIELD)) == NULL )
        continue;
    }
    else
    {
      if ( ( obj = get_eq_char( victim, iWear ) ) == NULL )
        continue;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
      if (paf->bitvector)
      {
        switch (paf->where)
        {
          case TO_AFFECTS:
            strncpy_color(buf2, obj->short_descr, 16, ' ', TRUE );
            sprintf(buf, "{D| {W%3d {g[{W%-16s{g]{W Obj Aff   {g[{W%-15s{g] {g[{W%-16s{g] {D|{x\n\r",
                    paf->level,
                    buf2,
                    affect_loc_name( paf->location ),
                    affect_bit_name( paf->bitvector ) );
            add_buf(buffer,buf);
            afffound = TRUE;
            break;

          case TO_AFFECTS2:
            strncpy_color(buf2, obj->short_descr, 16, ' ', TRUE );
            sprintf(buf, "{D| {W%3d {g[{W%-16s{g]{W Obj Aff   {g[{W%-15s{g] {g[{W%-16s{g] {D|{x\n\r",
                    paf->level,
                    buf2,
                    affect_loc_name( paf->location ),
                    affect2_bit_name( paf->bitvector ) );
            add_buf(buffer,buf);
            afffound = TRUE;
            break;

          default:
            break;
        } // switch paf
      } // if bit vector
    } // obj affect loop

    if (!obj->enchanted)
    {
      for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
      {
        if (paf->bitvector)
        {
          switch (paf->where)
          {
            case TO_AFFECTS:
              strncpy_color(buf2, obj->short_descr, 16, ' ', TRUE );
              sprintf(buf, "{D| {W%3d {g[{W%-16s{g]{W Obj Aff   {g[{W%-15s{g] {g[{W%-16s{g] {D|{x\n\r",
                      paf->level,
                      buf2,
                      affect_loc_name( paf->location ),
                      affect_bit_name( paf->bitvector ) );
              add_buf(buffer,buf);
              afffound = TRUE;
              break;

            case TO_AFFECTS2:
              strncpy_color(buf2, obj->short_descr, 16, ' ', TRUE );
              sprintf(buf, "{D| {W%3d {g[{W%-16s{g]{W Obj Aff   {g[{W%-15s{g] {g[{W%-16s{g] {D|{x\n\r",
                      paf->level,
                      buf2,
                      affect_loc_name( paf->location ),
                      affect2_bit_name( paf->bitvector ) );
              add_buf(buffer,buf);
              afffound = TRUE;
              break;

            default:
              break;
          } // switch paf (!enchanted)
        } // if bit vector (!enchanted)
      } // obj affect loop (!enchanted)
    } // if not enchanted
  } // worn-objects loop

// Show spell affects third
  for ( paf = victim->affected; paf; paf = paf->next )
  {
    if (paf->where == TO_AFFECTS)
      mprintf(sizeof(buf2),buf2, "{g[{W%-16s{g]{x",
              flag_string( affect_flags ,  paf->bitvector ) );
    else if (paf->where == TO_SPELL_AFFECTS)
      mprintf(sizeof(buf2),buf2, "{g[{W%-16s{g]{x",
              flag_string( spell_affect_flags ,  paf->bitvector ) );
    else if (paf->where == TO_AFFECTS2)
      mprintf(sizeof(buf2),buf2, "{g[{W%-16s{g]{x",
              flag_string( affect2_flags,  paf->bitvector ) );
    else
      mprintf(sizeof(buf2),buf2,"{g[{W%-16s{g]{x","none");

    strncpy(buf3, skill_table[(int) paf->type].name, 16);
    buf3[15] = '\0';

    sprintf(buf,"{D| {W%3d {g[{W%-16s{g]{W %3d %4d  {g[{W%-15s{g] %s {D|{x\n\r",
            paf->level,
            buf3,
            paf->duration,
            paf->modifier,
            affect_loc_name( paf->location ),
            buf2 );
    add_buf(buffer,buf);
    afffound = TRUE;
  }

  if (!afffound)
  {
    sprintf(buf,"{WNo affects.\n\r");
    add_buf(buffer,buf);
  }

  sprintf(buf,"{g*{D-----------------------------------------------------------------------{g*{x\n\r");
  add_buf(buffer,buf);

  page_to_char(buf_string(buffer),ch);
  free_buf(buffer);

  return;
} /* end of do_charstat */

void do_oldostat( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;
  bool made_obj =FALSE;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Stat what?\n\r", ch );
    return;
  }

  if (is_number(argument))
  {
    if ( ( pObjIndex = get_obj_index( atoi( argument ) ) ) == NULL )
    {
      send_to_char( "No object has that vnum.\n\r", ch );
      return;
    }
    made_obj = TRUE;
    obj = create_object( pObjIndex, get_trust(ch) );
  }
  else if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
  {
    send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
    return;
  }

  printf_to_char(ch, "Name(s): %s\n\r",
                 obj->name );

  printf_to_char(ch, "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
                 obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
                 item_name(obj->item_type), obj->pIndexData->reset_num );

  printf_to_char(ch, "Short description: %s\n\rLong description: %s\n\r",
                 obj->short_descr, obj->description );

  printf_to_char(ch, "Wear bits: %s\n\rExtra bits: %s\n\r",
                 wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );

  printf_to_char(ch, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n\r",
                 1,           get_obj_number( obj ),
                 obj->weight, get_obj_weight( obj ),get_true_weight(obj) );

  printf_to_char(ch, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
                 obj->level, obj->cost, obj->condition, obj->timer );

  printf_to_char(ch,
                 "In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
                 obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
                 obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
                 obj->carried_by == NULL    ? "(none)" :
                 can_see(ch,obj->carried_by) ? obj->carried_by->name
                 : "someone",
                 obj->wear_loc );

  printf_to_char(ch, "Values: %d %d %d %d %d\n\r",
                 obj->value[0], obj->value[1], obj->value[2], obj->value[3],
                 obj->value[4] );

  /* now give out vital statistics as per identify */

  switch ( obj->item_type )
  {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
      printf_to_char(ch, "Level %d spells of:", obj->value[0] );

      if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
      {
        send_to_char( " '", ch );
        send_to_char( skill_table[obj->value[1]].name, ch );
        send_to_char( "'", ch );
      }

      if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
      {
        send_to_char( " '", ch );
        send_to_char( skill_table[obj->value[2]].name, ch );
        send_to_char( "'", ch );
      }

      if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
      {
        send_to_char( " '", ch );
        send_to_char( skill_table[obj->value[3]].name, ch );
        send_to_char( "'", ch );
      }

      if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
      {
        send_to_char(" '",ch);
        send_to_char(skill_table[obj->value[4]].name,ch);
        send_to_char("'",ch);
      }

      send_to_char( ".\n\r", ch );
      break;

    case ITEM_WAND:
    case ITEM_STAFF:
      printf_to_char(ch, "Has %d(%d) charges of level %d",
                     obj->value[1], obj->value[2], obj->value[0] );

      if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
      {
        send_to_char( " '", ch );
        send_to_char( skill_table[obj->value[3]].name, ch );
        send_to_char( "'", ch );
      }

      send_to_char( ".\n\r", ch );
      break;

    case ITEM_DRINK_CON:
      printf_to_char(ch,"It holds %s-colored %s.\n\r",
                     liq_table[obj->value[2]].liq_color,
                     liq_table[obj->value[2]].liq_name);
      break;


    case ITEM_WEAPON:
      send_to_char("Weapon type is ",ch);
      switch (obj->value[0])
      {
        case(WEAPON_EXOTIC):
                send_to_char("exotic\n\r",ch);
          break;
        case(WEAPON_SWORD):
                send_to_char("sword\n\r",ch);
          break;
        case(WEAPON_DAGGER):
                send_to_char("dagger\n\r",ch);
          break;
        case(WEAPON_SPEAR):
                send_to_char("spear/staff\n\r",ch);
          break;
        case(WEAPON_MACE):
                send_to_char("mace/club\n\r",ch);
          break;
        case(WEAPON_AXE):
                send_to_char("axe\n\r",ch);
          break;
        case(WEAPON_FLAIL):
                send_to_char("flail\n\r",ch);
          break;
        case(WEAPON_WHIP):
                send_to_char("whip\n\r",ch);
          break;
        case(WEAPON_POLEARM):
                send_to_char("polearm\n\r",ch);
          break;
				case(WEAPON_CROSSBOW):
								send_to_char("crossbow\n\r",ch);
					break;
        default:
          send_to_char("unknown\n\r",ch);
          break;
      }
      if (obj->pIndexData->new_format)
        printf_to_char(ch,"Damage is %dd%d (average %d)\n\r",
                       obj->value[1],obj->value[2],
                       (1 + obj->value[2]) * obj->value[1] / 2);
      else
        printf_to_char(ch, "Damage is %d to %d (average %d)\n\r",
                       obj->value[1], obj->value[2],
                       ( obj->value[1] + obj->value[2] ) / 2 );

      printf_to_char(ch,"Damage noun is %s.\n\r",
                     (obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ?
                     attack_table[obj->value[3]].noun : "undefined");

      if (obj->value[4])    /* weapon flags */
  {
        printf_to_char(ch,"Weapons flags: %s\n\r",
                       weapon_bit_name(obj->value[4]));
      }
      break;

    case ITEM_ARMOR:
      printf_to_char(ch,
                     "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
                     obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
      break;

    case ITEM_CONTAINER:
      printf_to_char(ch,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
                     obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
      if (obj->value[4] != 100)
      {
        printf_to_char(ch,"Weight multiplier: %d%%\n\r",
                       obj->value[4]);
      }
      break;
  }


  if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
  {
    EXTRA_DESCR_DATA *ed;

    send_to_char( "Extra description keywords: '", ch );

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
      send_to_char( ed->keyword, ch );
      if ( ed->next != NULL )
        send_to_char( " ", ch );
    }

    for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
    {
      send_to_char( ed->keyword, ch );
      if ( ed->next != NULL )
        send_to_char( " ", ch );
    }

    send_to_char( "'\n\r", ch );
  }

  for ( paf = obj->affected; paf != NULL; paf = paf->next )
  {
    printf_to_char(ch, "Affects %s by %d, level %d",
                   affect_loc_name( paf->location ), paf->modifier,paf->level );
    if ( paf->duration > -1)
      printf_to_char(ch,", %d hours.\n\r",paf->duration);
    else
      printf_to_char(ch,".\n\r");
    if (paf->bitvector)
    {
      switch (paf->where)
      {
        case TO_AFFECTS:
          printf_to_char(ch,"Adds %s affect.\n",
                         affect_bit_name(paf->bitvector));
          break;
        case TO_AFFECTS2:
          printf_to_char(ch,"adds %s affect2.\n",
                         affect2_bit_name(paf->bitvector));
          break;
        case TO_WEAPON:
          printf_to_char(ch,"Adds %s weapon flags.\n",
                         weapon_bit_name(paf->bitvector));
          break;
        case TO_OBJECT:
          printf_to_char(ch,"Adds %s object flag.\n",
                         extra_bit_name(paf->bitvector));
          break;
        case TO_IMMUNE:
          printf_to_char(ch,"Adds immunity to %s.\n",
                         imm_bit_name(paf->bitvector));
          break;
        case TO_RESIST:
          printf_to_char(ch,"Adds resistance to %s.\n\r",
                         imm_bit_name(paf->bitvector));
          break;
        case TO_VULN:
          printf_to_char(ch,"Adds vulnerability to %s.\n\r",
                         imm_bit_name(paf->bitvector));
          break;
        default:
          printf_to_char(ch,"Unknown bit %d: %d\n\r",
                         paf->where,paf->bitvector);
          break;
      }
    }
  }

  if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
      printf_to_char(ch, "Affects %s by %d, level %d.\n\r",
                     affect_loc_name( paf->location ), paf->modifier,paf->level );
      if (paf->bitvector)
      {
        switch (paf->where)
        {
          case TO_AFFECTS:
            printf_to_char(ch,"Adds %s affect.\n",
                           affect_bit_name(paf->bitvector));
            break;
          case TO_AFFECTS2:
            printf_to_char(ch,"Adds %s affect2.\n",
                           affect2_bit_name(paf->bitvector));
            break;
          case TO_OBJECT:
            printf_to_char(ch,"Adds %s object flag.\n",
                           extra_bit_name(paf->bitvector));
            break;
          case TO_IMMUNE:
            printf_to_char(ch,"Adds immunity to %s.\n",
                           imm_bit_name(paf->bitvector));
            break;
          case TO_RESIST:
            printf_to_char(ch,"Adds resistance to %s.\n\r",
                           imm_bit_name(paf->bitvector));
            break;
          case TO_VULN:
            printf_to_char(ch,"Adds vulnerability to %s.\n\r",
                           imm_bit_name(paf->bitvector));
            break;
          default:
            printf_to_char(ch,"Unknown bit %d: %d\n\r",
                           paf->where,paf->bitvector);
            break;
        }
      }
    }

  if  (made_obj)
    extract_obj( obj);

  return;
}


void do_oldrstat( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  OBJ_DATA *obj;
  CHAR_DATA *rch;
  int door;

  one_argument( argument, arg );
  location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
  if ( location == NULL )
  {
    send_to_char( "No such location.\n\r", ch );
    return;
  }

  if (!is_room_owner(ch,location) && ch->in_room != location
      &&  room_is_private( location ) && !IS_TRUSTED(ch,IMPLEMENTOR))
  {
    send_to_char( "That room is private right now.\n\r", ch );
    return;
  }

  printf_to_char(ch, "Name: '%s'\n\rArea: '%s'\n\r",
                 location->name,
                 location->area->name );

  printf_to_char(ch,
                 "Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
                 location->vnum,
                 location->sector_type,
                 location->light,
                 location->heal_rate,
                 location->mana_rate );

  printf_to_char(ch,
                 "Room flags: %d.\n\rDescription:\n\r%s",
                 location->room_flags,
                 location->description );

  if ( location->extra_descr != NULL )
  {
    EXTRA_DESCR_DATA *ed;

    send_to_char( "Extra description keywords: '", ch );
    for ( ed = location->extra_descr; ed; ed = ed->next )
    {
      send_to_char( ed->keyword, ch );
      if ( ed->next != NULL )
        send_to_char( " ", ch );
    }
    send_to_char( "'.\n\r", ch );
  }

  send_to_char( "Characters:", ch );
  for ( rch = location->people; rch; rch = rch->next_in_room )
  {
    if (can_see(ch,rch))
    {
      send_to_char( " ", ch );
      one_argument( rch->name, buf );
      send_to_char( buf, ch );
    }
  }

  send_to_char( ".\n\rObjects:   ", ch );
  for ( obj = location->contents; obj; obj = obj->next_content )
  {
    send_to_char( " ", ch );
    one_argument( obj->name, buf );
    send_to_char( buf, ch );
  }
  send_to_char( ".\n\r", ch );

  for ( door = 0; door <= 5; door++ )
  {
    EXIT_DATA *pexit;

    if ( ( pexit = location->exit[door] ) != NULL )
    {
      printf_to_char(ch,
                     "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

                     door,
                     (pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
                     pexit->key,
                     pexit->exit_info,
                     pexit->keyword,
                     pexit->description[0] != '\0'
                     ? pexit->description : "(none).\n\r" );
    }
  }

  return;
}

/*
 * Race statistics.
 */
void do_race_stat( CHAR_DATA *ch, char *argument )
{
  BUFFER *buffer;
  char    arg[MAX_INPUT_LENGTH];
  sh_int  race,i,col;

  one_argument( argument, arg );

  if ( ( arg[0] == '\0' )
       ||   (!str_cmp(argument,"all")) )
  {
    buffer = new_buf();
    bprintf( buffer,"{x                      Class Proficiencies\n\r{c  Race Name    " );
    for ( i = 0; i < MAX_CLASS; i++ )
    {
      if (pc_race_table[1].class_mult[i] > 0)
        bprintf( buffer, "%s ", class_table[i].who_name);
    }
    bprintf( buffer,"{x\n\r{c ------------- --- --- --- --- --- --- --- --- --- ---{x\n\r");

    race = 0;
    for (race = 1; race < MAX_PC_RACE; race++)
    {
      bprintf( buffer,"{x  %-11s ",pc_race_table[race].name);
      for ( i = 0; i < MAX_CLASS; i++ )
      {
        if (pc_race_table[race].class_mult[i] > 0)
          bprintf( buffer, "%4d", pc_race_table[race].class_mult[i] );
      }
      bprintf( buffer, "{x\n\r" );

      if (race%12 == 0)
      {
        bprintf( buffer, "%15s"," " );
        for ( i = 0; i < MAX_CLASS; i++ )
        {
          if (pc_race_table[1].class_mult[i] > 0)
            bprintf( buffer, "%s ", class_table[i].who_name);
        }
        bprintf( buffer, "{x\n\r" );
      }

    }

    bprintf( buffer,"{c ------------- --- --- --- --- --- --- --- --- --- ---{x\n\r{c  Race Name    " );
    for ( i = 0; i < MAX_CLASS; i++ )
    {
      if (pc_race_table[1].class_mult[i] > 0)
        bprintf( buffer, "%s ", class_table[i].who_name);
    }
    bprintf( buffer,"\n\r\n\r{x  %d {craces displayed.{x\n\r",MAX_PC_RACE);
    bprintf( buffer,"{c  Type {x\"stat race [racename]\" {cfor more information.{x\n\r");

    page_to_char( buf_string( buffer ), ch );
    free_buf( buffer );
    return;
  }

  if ( ( race = race_lookup( arg ) ) == 0 )
  {
    send_to_char( "No such race.\n\r", ch );
    return;
  }

  buffer = new_buf();

  if ( race_table[race].pc_race )
  {
    bool found = FALSE;

    bprintf( buffer, "{cName:{x %s {w(PC){x   {cWhoname:{x [%s]   {cCP:{x %d\n\r",
             race_table[race].name,
             pc_race_table[race].who_name,
             pc_race_table[race].points);

    bprintf( buffer,
             "{cStr:{x {w%d{x/%d  {cInt:{x {w%d{x/%d  {cWis:{x {w%d{x/%d  {cDex:{x {w%d{x/%d  {cCon:{x {w%d{x/%d\n\r\n\r",
             pc_race_table[race].stats[STAT_STR],
             pc_race_table[race].max_stats[STAT_STR],
             pc_race_table[race].stats[STAT_INT],
             pc_race_table[race].max_stats[STAT_INT],
             pc_race_table[race].stats[STAT_WIS],
             pc_race_table[race].max_stats[STAT_WIS],
             pc_race_table[race].stats[STAT_DEX],
             pc_race_table[race].max_stats[STAT_DEX],
             pc_race_table[race].stats[STAT_CON],
             pc_race_table[race].max_stats[STAT_CON] );

    for ( col = i = 0; i < MAX_CLASS; i++ )
    {
      /* Taeloch -- I'm removing unused/NA from the class list.  Remove this to revert */
      if ( pc_race_table[race].class_mult[i] < 1 )
        break;

      if ( pc_race_table[race].class_mult[i] == 0 )
        bprintf( buffer, "%3s: N/A",
                 capitalize( class_table[i].who_name ) );
      else
        bprintf( buffer, "%3s:%4d%%",
                 capitalize( class_table[i].who_name ),
                 pc_race_table[race].class_mult[i] );

      if ( ++col % 6 == 0 )
        bstrcat( buffer, "\n\r" );
      else
        bstrcat( buffer, "  " );
    }

    if ( col % 6 != 0 )
      bstrcat( buffer, "\n\r" );

    bprintf( buffer, "\n\r{cSector:{x [%s]\n\r", flag_string( sector_flags, race_table[race].native_sect ) );
    bprintf( buffer, "{cSize:{x   [%s]\n\r", flag_string( size_flags, pc_race_table[race].size ) );

    for ( i = 0; i < MAX_IN_RACE; i++ )
    {
      if ( IS_NULLSTR( pc_race_table[race].skills[i] ) )
        break;

      if ( !found )
      {
        bstrcat( buffer, "{cRacial:{x [" );
        found = TRUE;
      }

      bprintf( buffer, "%s", pc_race_table[race].skills[i] );

      if ( i < MAX_IN_RACE - 1
           &&   !IS_NULLSTR( pc_race_table[race].skills[i + 1] ) )
        bstrcat( buffer, ", " );
    }

    if ( found )
      bstrcat( buffer, "]\n\r" );
  }
  else // NOT a PC class
  {
    bprintf( buffer, "{cName:{x   %s\n\r", capitalize(race_table[race].name));
    bprintf( buffer, "{cSize:{x   [%s]\n\r", flag_string( size_flags, pc_race_table[race].size ));
  }

  if ( race_table[race].act )
    bprintf( buffer, "{cAct:{x    [%s]\n\r",
             flag_string( act_flags, race_table[race].act ) );

  if ( race_table[race].aff )
    bprintf( buffer, "{cAff:{x    [%s]\n\r",
             flag_string( affect_flags, race_table[race].aff ) );

  if ( race_table[race].aff2 )
    bprintf( buffer, "{cAff2:{x   [%s]\n\r",
             flag_string( affect2_flags, race_table[race].aff2 ) );

  if ( race_table[race].off )
    bprintf( buffer, "{cOff:{x    [%s]\n\r",
             flag_string( off_flags, race_table[race].off ) );

  if ( race_table[race].imm )
    bprintf( buffer, "{cImm:{x    [%s]\n\r",
             flag_string( imm_flags, race_table[race].imm ) );

  if ( race_table[race].res )
    bprintf( buffer, "{cRes:{x    [%s]\n\r",
             flag_string( imm_flags, race_table[race].res ) );

  if ( race_table[race].vuln )
    bprintf( buffer, "{cVuln:{x   [%s]\n\r",
             flag_string( imm_flags, race_table[race].vuln ) );

  bprintf( buffer, "{cForm:{x   [%s]\n\r",
           flag_string( form_flags, race_table[race].form ) );

  bprintf( buffer, "{cParts:{x  [%s]\n\r",
           flag_string( part_flags, race_table[race].parts ) );

  /* ****** Appearance Settings ****** */
  if ( race_table[race].pc_race )
  {
    bprintf( buffer, "\n\r{cRacial Appearance values:{x\n\r{cSkin:{x   [");
    for (i=0;i<MAX_APPR;i++)
    {
      if (pc_race_table[race].skin_color[i] == NULL)
        break;
      else
      {
        if (i != 0)
          bprintf( buffer, ", ");
        bprintf( buffer, "%s", pc_race_table[race].skin_color[i]);
      }
    }

    bprintf( buffer, "]\n\r{cEyes:{x   [");
    for (i=0;i<MAX_APPR;i++)
    {
      if (pc_race_table[race].eye_color[i] == NULL)
        break;
      else
      {
        if (i != 0)
          bprintf( buffer, ", ");
        bprintf( buffer, "%s", pc_race_table[race].eye_color[i]);
      }
    }

    bprintf( buffer, "]\n\r{cHair:{x   [");
    for (i=0;i<MAX_APPR;i++)
    {
      if (pc_race_table[race].hair_color[i] == NULL)
        break;
      else
      {
        if (i != 0)
          bprintf( buffer, ", ");
        bprintf( buffer, "%s", pc_race_table[race].hair_color[i]);
      }
    }
    bprintf( buffer, "]\n\r");
  }

  //bprintf( buffer, "Damage: [%s]\n\r",
  //    FIX_STR( race_table[race].damage_message, "none", "null" ) );

  //bprintf( buffer, "Material: [%s]\n\r",
  //    FIX_STR( race_table[race].material, "none", "null" ) );

  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );
}

/*
 * Code statistics
 */
void do_cstat( CHAR_DATA *ch, char *argument )
{
  MPROG_CODE *pMcode;
  BUFFER     *buffer;
  char       arg[MSL];
  int        vnum;

  one_argument( argument, arg );

  if ( !is_number(arg) || ( vnum = atoi ( arg ) ) < 0 )
  {
    send_to_char( "Your argument must be the code number.\n\r", ch );
    return;
  }

  if ( ( pMcode = get_mprog_index( vnum ) ) == NULL )
  {
    send_to_char( "That mprog doesn't exist, try another vnum.\n\r", ch );
    return;
  }

  buffer = new_buf();

  bprintf( buffer, "Name: %s\n\r", FIX_STR( pMcode->name, "none", "null" ) );

  bprintf( buffer, "Vnum: %d\n\r", pMcode->vnum );

  bprintf( buffer, "Code:\n\r" );
  bstrcat( buffer, pMcode->code );
  bstrcat( buffer, "\n\r" );

  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );

}

void do_class_stat( CHAR_DATA *ch, char *argument )
{
  BUFFER      *buffer;
  char        arg[MAX_INPUT_LENGTH];
  char        attr_name[MSL];
  int         iClass;
  int         i, attr, gn, col=0, ct=0;

  one_argument( argument, arg );

  if ( ( arg[0] == '\0' )
       ||   (!str_cmp(argument,"all")) )
  {
    printf_to_char(ch,"{c  Class Name  Who Prime Weapon t00 t32\n\r");
    printf_to_char(ch,"{c ------------ --- ----- ------ --- ---\n\r");
    for ( i = 0; i < MAX_CLASS; i++ )
    {
      if ( (class_table[i].who_name[0] == 'U')
           ||   (class_table[i].weapon == 0) )
        continue; // Unused

      ct++;

      switch (class_table[i].attr_prime)
      {
        case 0:
          strcpy( attr_name, "Str" );
          break;
        case 1:
          strcpy( attr_name, "Int" );
          break;
        case 2:
          strcpy( attr_name, "Wis" );
          break;
        case 3:
          strcpy( attr_name, "Dex" );
          break;
        default:
          strcpy( attr_name, "Con" );
          break;
      }

      printf_to_char(ch, "{x  %-11s %s  %s   %5d %3d %3d{x\n\r",
                     class_table[i].name,
                     class_table[i].who_name,
                     attr_name,
                     class_table[i].weapon,
                     class_table[i].thac0_00,
                     class_table[i].thac0_32);
    }
    printf_to_char(ch,"{c ------------ --- ----- ------ --- ---\n\r");
    printf_to_char(ch,"{x  %d {cclasses displayed.{x\n\r",ct);
    printf_to_char(ch,"{c  Type {x\"stat class [class]\" {cfor more.{x\n\r");
    return;
  }

  if ( ( iClass = class_lookup( arg ) ) == -1 )
  {
    send_to_char( "No such class.\n\r", ch );
    return;
  }

  buffer = new_buf();

  attr = class_table[iClass].attr_prime;

  if ( attr == 0 )
    strcpy( attr_name, "Strength" );
  else if ( attr == 1 )
    strcpy( attr_name, "Intelligence" );
  else if ( attr == 2 )
    strcpy( attr_name, "Wisdom" );
  else if ( attr == 3 )
    strcpy( attr_name, "Dexterity" );
  else
    strcpy( attr_name, "Constitution" );

  bprintf( buffer, "{cName:       {w%s\n\r{cWho Name:   {D[{x%s{D]\n\r{cPrim Stat:  {w%s\n\r{cWeapon:     {w%d\n\r{cGuilds:     {w",
           class_table[iClass].name,
           class_table[iClass].who_name,
           attr_name,
           class_table[iClass].weapon );

  for ( i = 0 ; i < MAX_GUILD ; i++ )
    bprintf( buffer, "%d ", class_table[iClass].guild[i] );

  bprintf( buffer, "\n\r{cthac0_00:   {w%d\n\r{cthac0_32:   {w%d\n\r{cMin HP:     {w%d\n\r{cMax HP:     {w%d\n\r",
           class_table[iClass].thac0_00,
           class_table[iClass].thac0_32,
           class_table[iClass].hp_min,
           class_table[iClass].hp_max );

  bprintf( buffer, "\n\r{cGains mana:    {w%s\n\r{cBasic Group:   {w%s\n\r{cDefault Group: {w%s{x\n\r{cTotal CP:      {w%d{x\n\r\n\r{cAll Groups:\n\r",
           class_table[iClass].fMana == TRUE ? "Yes" : "No",
           class_table[iClass].base_group,
           class_table[iClass].default_group,
           total_class_cp(ch, iClass) );

  for (gn = 0; gn < MAX_GROUP; gn++)
  {
    if (group_table[gn].name == NULL)
      break;

    if (!str_cmp(group_table[gn].name,"rom basics"))
      continue;

    if (group_table[gn].proficiency[iClass] > 0)
    {
      bprintf( buffer, "{w%-18s{D({c%3d{w%%{D){x ",
               group_table[gn].name, group_table[gn].proficiency[iClass] );
      if (++col % 3 == 0)
        bstrcat( buffer, "\n\r" );
    }
  }

  if ( col % 3 != 0 )
    bstrcat( buffer, "\n\r" );

  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );
}

void do_liquid_stat( CHAR_DATA *ch, char *argument )
{
  BUFFER      *buffer;
  char        arg[MAX_INPUT_LENGTH];
  char        buf1[MSL];
  int         liquid;

  one_argument( argument, arg );

  if ( ( arg[0] == '\0' )
       ||  !str_cmp( arg, "all" ) )
  {
    int i;
    buffer = new_buf();
    bstrcat( buffer, "Name               Color              Proof Full Thirst Hunger SSize\n\r" );

    for ( i = 0 ; i < MAX_LIQUID ; i++ )
    {
      strncpy_color( buf1,
                     FIX_STR( liq_table[i].liq_name, "(none)            ", "(null)            " ),
                     18, ' ', TRUE );

      bprintf( buffer,
               "%-18s %-14s     %3d    %3d    %3d    %3d   %3d\n\r",
               buf1,
               liq_table[i].liq_color,
               liq_table[i].liq_affect[0],
               liq_table[i].liq_affect[1],
               liq_table[i].liq_affect[2],
               liq_table[i].liq_affect[3],
               liq_table[i].liq_affect[4] );
    }
    page_to_char( buf_string( buffer ), ch );
    free_buf( buffer );
    return;
  }
  else if ( ( liquid = liq_lookup( arg ) )  < 0 )
  {
    send_to_char( "No such liquid.\n\r", ch );
    return;
  }

  buffer = new_buf();

  bprintf( buffer, "Name:     %s\n\r", liq_table[liquid].liq_name );

  bprintf( buffer, "Color:    %s\n\r", liq_table[liquid].liq_color );

  bstrcat( buffer, "Valus:    Proof Full Thirst Hunger SSize\n\r" );

  bprintf( buffer, "             %2d   %2d     %2d     %2d    %2d\n\r",
           liq_table[liquid].liq_affect[0],
           liq_table[liquid].liq_affect[1],
           liq_table[liquid].liq_affect[2],
           liq_table[liquid].liq_affect[3],
           liq_table[liquid].liq_affect[4] );

  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );

}

void do_skill_stat( CHAR_DATA *ch, char *argument )
{
  BUFFER      *buffer;
  char        arg[MAX_INPUT_LENGTH];
  char        pos_name[MSL];
  int         sn, position;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Syntax:  stat skill|spell <name>\n\r", ch );
    return;
  }

  if ( ( sn = skill_lookup( arg ) ) < 0 )
  {
    send_to_char( "No such skill.\n\r", ch );
    return;
  }

  buffer = new_buf();

  bprintf( buffer, "Name: %s\n\r", skill_table[sn].name );

  bstrcat( buffer,
           "{bC{mjr{x {yP{wri{x {DH{wwy {yK{Dni{x {bW{ylk{x {yB{rar{x " );

  bstrcat( buffer,
           "{mM{Dys{x {gD{yru{x {WInq{x {DOcc {mA{clc{x\n\r" );

  bprintf( buffer, "%3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n\r",
           skill_table[sn].skill_level[0],
           skill_table[sn].skill_level[1],
           skill_table[sn].skill_level[2],
           skill_table[sn].skill_level[3],
           skill_table[sn].skill_level[4],
           skill_table[sn].skill_level[5],
           skill_table[sn].skill_level[6],
           skill_table[sn].skill_level[7],
           skill_table[sn].skill_level[8],
           skill_table[sn].skill_level[9],
           skill_table[sn].skill_level[10] );

  bprintf( buffer, "%3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n\r",
           skill_table[sn].rating[0],
           skill_table[sn].rating[1],
           skill_table[sn].rating[2],
           skill_table[sn].rating[3],
           skill_table[sn].rating[4],
           skill_table[sn].rating[5],
           skill_table[sn].rating[6],
           skill_table[sn].rating[7],
           skill_table[sn].rating[8],
           skill_table[sn].rating[9],
           skill_table[sn].rating[10] );

  bprintf( buffer, "\n\rMana:    %d\n\r", skill_table[sn].min_mana );

  position = skill_table[sn].minimum_position;

  if ( position == 0 )
    strcpy( pos_name, "Dead" );
  else if ( position == 1 )
    strcpy( pos_name, "Mortally Wounded" );
  else if ( position == 2 )
    strcpy( pos_name, "Incapacitated" );
  else if ( position == 3 )
    strcpy( pos_name, "Stunned" );
  else if ( position == 4 )
    strcpy( pos_name, "Sleeping" );
  else if ( position == 5 )
    strcpy( pos_name, "Resting" );
  else if ( position == 6 )
    strcpy( pos_name, "Sitting" );
  else if ( position == 7 )
    strcpy( pos_name, "Fighting" );
  else
    strcpy( pos_name, "Standing" );

  bprintf( buffer, "Min Pos: %s\n\r", pos_name );

  bprintf( buffer, "Beats:   %d\n\r", skill_table[sn].beats );

  bprintf( buffer, "Slot:    %d\n\r", skill_table[sn].slot );

  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );

}

void do_group_stat( CHAR_DATA *ch, char *argument )
{
  int fn,gn,sn,ct=0,oct=0;
  char buf[MSL];

  if (IS_NPC(ch))
    return;

  if ( ( argument[0] == '\0' )
       ||   (!str_cmp(argument,"all")) )
  {
    printf_to_char(ch,"{x                            Creation Points\n\r");
    printf_to_char(ch,"{c  Group Name      {bC{mjr {yP{wri {DH{wwy {yK{Dni {bW{ylk {yB{rar {mM{Dys {gD{yru {WInq {DOcc {mA{clc {yW{gds{x\n\r");
    printf_to_char(ch,"{c ---------------- --- --- --- --- --- --- --- --- --- --- --- ---{x\n\r");

    for (gn = 0; gn < MAX_GROUP; gn++)
    {
      if (group_table[gn].name == NULL)
        break;

      if ( strstr(group_table[gn].name,"background") ) // special case, unused group
        continue;

      if ( strstr(group_table[gn].name,"default") )
      {
        oct++;
        continue;
      }

      if ( strstr(group_table[gn].name,"basics") )
      {
        oct++;
        continue;
      }

      ct++;

      strncpy_color(buf, group_table[gn].name, 16, ' ', TRUE );
      printf_to_char(ch,"{x  %-16s",buf);

      if (group_table[gn].rating[cConjurer] >= 0)
        printf_to_char(ch," %2d ", group_table[gn].rating[cConjurer]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (group_table[gn].rating[cPriest] >= 0)
        printf_to_char(ch," %2d ", group_table[gn].rating[cPriest]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (group_table[gn].rating[cHighwayman] >= 0)
        printf_to_char(ch," %2d ", group_table[gn].rating[cHighwayman]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (group_table[gn].rating[cKnight] >= 0)
        printf_to_char(ch," %2d ", group_table[gn].rating[cKnight]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (group_table[gn].rating[cWarlock] >= 0)
        printf_to_char(ch," %2d ", group_table[gn].rating[cWarlock]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (group_table[gn].rating[cBarbarian] >= 0)
        printf_to_char(ch," %2d ", group_table[gn].rating[cBarbarian]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (group_table[gn].rating[cMystic] >= 0)
        printf_to_char(ch," %2d ", group_table[gn].rating[cMystic]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (group_table[gn].rating[cDruid] >= 0)
        printf_to_char(ch," %2d ", group_table[gn].rating[cDruid]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (group_table[gn].rating[cInquisitor] >= 0)
        printf_to_char(ch," %2d ", group_table[gn].rating[cInquisitor]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (group_table[gn].rating[cOccultist] >= 0)
        printf_to_char(ch," %2d ", group_table[gn].rating[cOccultist]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (group_table[gn].rating[cAlchemist] >= 0)
        printf_to_char(ch," %2d ", group_table[gn].rating[cAlchemist]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (group_table[gn].rating[cWoodsman] >= 0)
        printf_to_char(ch," %2d ", group_table[gn].rating[cWoodsman]);
      else
        printf_to_char(ch,"  {rX{x ");

      printf_to_char(ch,"{x\n\r");
    }

    printf_to_char(ch,"{c ---------------- --- --- --- --- --- --- --- --- --- --- --- ---{x\n\r");
    printf_to_char(ch,"{w  %d {cgroups displayed, {w%d {cclass defaults/basics suppressed{x\n\r",ct,oct-1); // -1 because of rom basics
    printf_to_char(ch,"{c  Type {x\"stat group [groupname]\" {cfor more information.{x\n\r");
    return;
  }

  ct = 0;
  gn = group_lookup(argument);

  if (gn == -1)
  {
    send_to_char("No group of that name exists.\n\r",ch);
    return;
  }

  printf_to_char(ch,"{x                                Levels\n\r");

  printf_to_char(ch,"{c  Skill Name           {bC{mjr {yP{wri {DH{wwy {yK{Dni {bW{ylk {yB{rar {mM{Dys {gD{yru {WInq {DOcc {mA{clc {yW{gds{x\n\r");
  printf_to_char(ch,"{c --------------------- --- --- --- --- --- --- --- --- --- --- --- ---{x\n\r");

  for (sn = 0; sn < MAX_IN_GROUP; sn++)
  {
    if ( group_table[gn].spells[sn] == NULL )
      break;

    ct++;
    fn = skill_lookup_exact( group_table[gn].spells[sn] );

    strncpy_color(buf, group_table[gn].spells[sn], 21, ' ', TRUE );
    printf_to_char(ch,"{x  %-21s",buf); // a group in a group

    if (fn == -1)
      printf_to_char(ch," {c(group){x\n\r",group_table[gn].spells[sn]); // a group in a group
    else
    {
      if (skill_table[fn].skill_level[cConjurer] >= 0)
        printf_to_char(ch," %2d ", skill_table[fn].skill_level[cConjurer]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (skill_table[fn].skill_level[cPriest] >= 0)
        printf_to_char(ch," %2d ", skill_table[fn].skill_level[cPriest]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (skill_table[fn].skill_level[cHighwayman] >= 0)
        printf_to_char(ch," %2d ", skill_table[fn].skill_level[cHighwayman]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (skill_table[fn].skill_level[cKnight] >= 0)
        printf_to_char(ch," %2d ", skill_table[fn].skill_level[cKnight]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (skill_table[fn].skill_level[cWarlock] >= 0)
        printf_to_char(ch," %2d ", skill_table[fn].skill_level[cWarlock]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (skill_table[fn].skill_level[cBarbarian] >= 0)
        printf_to_char(ch," %2d ", skill_table[fn].skill_level[cBarbarian]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (skill_table[fn].skill_level[cMystic] >= 0)
        printf_to_char(ch," %2d ", skill_table[fn].skill_level[cMystic]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (skill_table[fn].skill_level[cDruid] >= 0)
        printf_to_char(ch," %2d ", skill_table[fn].skill_level[cDruid]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (skill_table[fn].skill_level[cInquisitor] >= 0)
        printf_to_char(ch," %2d ", skill_table[fn].skill_level[cInquisitor]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (skill_table[fn].skill_level[cOccultist] >= 0)
        printf_to_char(ch," %2d ", skill_table[fn].skill_level[cOccultist]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (skill_table[fn].skill_level[cAlchemist] >= 0)
        printf_to_char(ch," %2d ", skill_table[fn].skill_level[cAlchemist]);
      else
        printf_to_char(ch,"  {rX{x ");

      if (skill_table[fn].skill_level[cWoodsman] >= 0)
        printf_to_char(ch," %2d ", skill_table[fn].skill_level[cWoodsman]);
      else
        printf_to_char(ch,"  {rX{x ");

      printf_to_char(ch,"{x\n\r");
    }
  }
  printf_to_char(ch,"{c --------------------- --- --- --- --- --- --- --- --- --- --- --- ---{x\n\r");
  printf_to_char(ch,"{w  %d {cgroup items displayed{x\n\r",ct);

  return;
}

void do_frag_stat( CHAR_DATA *ch, char *argument )
{
  BUFFER      *buffer;
  CHAR_DATA   *victim;
  char        buf[MSL];
  int         frag_stat = 0;

  if ( ( ( victim = get_char_world( ch, argument ) ) == NULL )
       || IS_NPC( victim ) )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  buffer = new_buf();

  // Display the fragment totals collected
  bstrcat( buffer, "Fragments Gained:\n\r" );
  bprintf( buffer, "{R%d.%02d {rR{Ru{rby\n\r",
           victim->ruby_counter,     (victim->ruby_fragment     * 100 / 250000) );

  bprintf( buffer, "{B%d.%02d {cSap{Bp{bhi{cre\n\r",
           victim->sapphire_counter, (victim->sapphire_fragment * 100 / 200000) );

  bprintf( buffer, "{G%d.%02d {gE{Gme{gr{Ga{gld\n\r",
           victim->emerald_counter,  (victim->emerald_fragment  * 100 / 150000) );

  bprintf( buffer, "{C%d.%02d {CDi{cam{Wo{wnd{x\n\r\n\r",
           victim->diamond_counter,  (victim->diamond_fragment  * 100 / 100000) );

  // Display current statistics
  bstrcat( buffer, "Current Stats:\n\r" );
  frag_stat = victim->perm_stat[STAT_STR] - get_max_train(victim, STAT_STR);
  mprintf(sizeof(buf),buf,"(+%d)", frag_stat );
  bprintf( buffer,
           "{wStr{x:    %2d %4s{x\n\r",
           victim->perm_stat[STAT_STR],
           victim->perm_stat[STAT_STR] > get_max_train(victim, STAT_STR) ? buf : "(+0)" );

  frag_stat = victim->perm_stat[STAT_DEX] - get_max_train(victim, STAT_DEX);
  mprintf(sizeof(buf),buf,"(+%d)", frag_stat );
  bprintf( buffer,
           "{wDex{x:    %2d %4s{x\n\r",
           victim->perm_stat[STAT_DEX],
           victim->perm_stat[STAT_DEX] > get_max_train(victim, STAT_DEX) ? buf : "(+0)" );

  frag_stat = victim->perm_stat[STAT_INT] - get_max_train(victim, STAT_INT);
  mprintf(sizeof(buf),buf,"(+%d)", frag_stat );
  bprintf( buffer,
           "{wInt{x:    %2d %4s{x\n\r",
           victim->perm_stat[STAT_INT],
           victim->perm_stat[STAT_INT] > get_max_train(victim, STAT_INT) ? buf : "(+0)" );

  frag_stat = victim->perm_stat[STAT_WIS] - get_max_train(victim, STAT_WIS);
  mprintf(sizeof(buf),buf,"(+%d)", frag_stat );
  bprintf( buffer,
           "{wWis{x:    %2d %4s{x\n\r",
           victim->perm_stat[STAT_WIS],
           victim->perm_stat[STAT_WIS] > get_max_train(victim, STAT_WIS) ? buf : "(+0)" );

  frag_stat = victim->perm_stat[STAT_CON] - get_max_train(victim, STAT_CON);
  mprintf(sizeof(buf),buf,"(+%d)", frag_stat );
  bprintf( buffer,
           "{wCon{x:    %2d %4s{x\n\r\n\r",
           victim->perm_stat[STAT_CON],
           victim->perm_stat[STAT_CON] > get_max_train(victim, STAT_CON) ? buf : "(+0)" );

  //Display total number of purchases
  bstrcat( buffer, "Fragment Purchases:\n\r" );
  bprintf( buffer, "1st: %d/5\n\r", victim->first_frag_level );
  bprintf( buffer, "2nd: %d/5\n\r", victim->second_frag_level );
  bprintf( buffer, "3rd: %d/3\n\r", victim->third_frag_level );
  bprintf( buffer, "4th: %d/2\n\r", victim->fourth_frag_level );
  bprintf( buffer, "5th: %d/1\n\r", victim->fifth_frag_level );

  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );
}
