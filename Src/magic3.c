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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "magic.h"
#include "recycle.h"
#include "interp.h"


extern char *target_name;
bool is_aura_spelled(CHAR_DATA *vch);
bool is_element_affected(CHAR_DATA *vch);

/* ==========DETER AGRRO SPELL ====================== */
/* Original Code by Jason Huang (god@sure.net).       */
/* Permission to use this code is granted provided    */
/* this header is retained and unaltered.             */

void spell_deter( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_SET(victim->spell_aff,SAFF_DETER))
  {
    send_to_char("You want to deter again???\n\r",ch);
    return;
  }
  if ( IS_NPC(ch) )
    return;
  af.where     = TO_SPELL_AFFECTS;
  af.type      = sn;
  af.duration  = number_fuzzy( level / 20 ) + 1;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.level     = level;
  af.bitvector = SAFF_DETER;
  affect_to_char( victim, &af );

  send_to_char( "You are protected from hostile creatures.\n\r", ch );
  act( "$n is protected from hostile creatures.", ch, NULL, victim, TO_ROOM );
  act( "Hostile creatures no longer attack you.", ch, NULL, victim, TO_VICT );
  return;
}

/* =========== AID SPELL ===================== */
/*
 * Baxter the Overlord of Deadland.
 * deadland.ada.com.tr 9000 (195.142.130.3)
 * Date: 02/20/97
 */
void spell_aid( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
  {
    if ( victim == ch )
      send_to_char("You are already aided.\n\r", ch);
    else
      act("$N is already aided.",ch,NULL,victim,TO_CHAR);
    return;
  }

  af.where   = TO_AFFECTS;
  af.type   = sn;
  af.level   = level;
  af.duration   = level/2;
  af.location   = APPLY_HITROLL;
  af.modifier   = 1 + (level - 1) / 10;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  victim->hit = UMIN(victim->hit + 1 + (level - 1) / 10, GET_HP(victim));
  send_to_char("You feel better!\n\r", victim);
  if ( ch != victim )
    act("$N looks aided.",ch,NULL,victim,TO_CHAR);
  return;
}

/* ========== METAMORPH SPELL ====================== */
/* Original Code by Jason Huang (god@sure.net).                       */
/* Permission to use this code is granted provided this header is     */
/* retained and unaltered.                                            */

void spell_metamorphose( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  send_to_char("This spell not functional at this time\n\r",ch);
}
/* ================================ */
/* Original Code by Jason Huang (god@sure.net).                       */
/* Permission to use this code is granted provided this header is     */
/* retained and unaltered.                                            */

void spell_betray( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( !ch->fighting )
  {
    send_to_char( "You may only cast betray during combat.\n\r", ch );
    return;
  }
  if ( victim == ch )
  {
    send_to_char( "Betray yourself?  You're weird.\n\r", ch );
    return;
  }

  if ((!IS_AFFECTED( victim, AFF_CHARM ))
      || (level < victim->level)
      || !IS_AWAKE(victim)
      || (saves_spell( level, victim, DAM_CHARM)))
  {
    send_to_char("Spell failed.\n\r",ch);
    return;
  }


  if ( victim->fighting == ch )
    stop_fighting( victim, TRUE );
  if (victim->hate)
    if (victim->hate->who == ch )
      stop_hating(victim);
  if ( victim->master )
    stop_follower( victim );
  add_follower( victim, ch );
  if (ch->leader)
    victim->leader = ch->leader;
  else
    victim->leader = ch;
  victim->group_num = victim->leader->group_num;

  af.type   = sn;
  af.duration   = number_fuzzy( level / 4 );
  af.location   = APPLY_NONE;
  af.modifier   = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char( victim, &af );

  act( "$N has betrayed!", ch, NULL, victim, TO_ROOM );
  act( "You now follow $n!", ch, NULL, victim, TO_VICT );
  return;
}
/* ================================ */
/* Original Code by Jason Huang (god@sure.net).                       */
/* Permission to use this code is granted provided this header is     */
/* retained and unaltered.                                            */

void spell_quench( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim;

  if ( IS_NPC(ch) )
    return;

  if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
  {
    gain_condition( ch, COND_FULL, level/25);
    gain_condition(ch, COND_THIRST,level/3);

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
      send_to_char( "You are full.\n\r", ch );

    send_to_char( "You have quenched your thirst.\n\r", ch );
    return;
  }

  if ( victim != ch )
  {
    send_to_char( "You cannot cast this on someone else.\n\r", ch );
    return;
  }

  gain_condition( ch, COND_FULL, level/25);
  gain_condition(ch, COND_THIRST,level/3);

  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
    send_to_char( "You are full.\n\r", ch );
  send_to_char( "You have quenched your thirst.\n\r", ch );
  return;

}

void spell_sate( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim;

  if ( IS_NPC(ch) )
    return;

  if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
  {
    gain_condition( ch, COND_FULL, level/25);
    gain_condition(ch, COND_HUNGER,level/3);

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
      send_to_char( "You are full.\n\r", ch );

    send_to_char( "You have sated your hunger.\n\r", ch );
    return;
  }

  if ( victim != ch )
  {
    send_to_char( "You cannot cast this on someone else.\n\r", ch );
    return;
  }

  gain_condition( ch, COND_FULL, level/20);
  gain_condition(ch, COND_HUNGER,level/5);

  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
    send_to_char( "You are full.\n\r", ch );
  send_to_char( "You have sated your hunger.\n\r", ch );
  return;

}
/* ================================ */
void spell_resurrect( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  CHAR_DATA *mob;
  int i;

  /*obj = get_obj_here( ch, target_name );*/
  if (target == TARGET_OBJ)
  {
    if ( obj == NULL )
    {
      send_to_char( "Resurrect what?\n\r", ch );
      return;
    }

    /*
     * NPC's have no restrictions
     * PC's must be empty before you can raise them
     */

    if ( obj->item_type == ITEM_CORPSE_PC
         &&   obj->contains )
    {
      send_to_char(
        "You may not resurrect that until it is empty.\n\r", ch );
      return;
    }

    if ( obj->item_type != ITEM_CORPSE_NPC
         &&  obj->item_type != ITEM_CORPSE_PC
         &&  obj->pIndexData->vnum != 1111 )
    {
      send_to_char( "It would serve no purpose...\n\r", ch );
      return;
    }

    if ( obj->level > (level + 20) )
    {
      send_to_char( "You couldn't call forth such a great spirit.\n\r", ch );
      return;
    }

    if ( ch->pet != NULL )
    {
      send_to_char( "You already have a pet.\n\r", ch );
      return;
    }

    /* Chew on the zombie a little bit, recalculate level-dependant stats */
    mob = create_mobile( get_mob_index( MOB_VNUM_ZOMBIE ) );
    mob->level                  = obj->level-20;

    if (mob->level < 0)
      mob->level = 0;

    mob->max_hit                = mob->level * 80;

    if (mob->max_hit < 1)
      mob->max_hit = 1;

    mob->hit                    = mob->max_hit;
    mob->max_mana               = 100 + dice(mob->level,10);
    mob->mana                   = mob->max_mana;
    mob->move = mob->max_move      = 100 + dice(mob->level,5);

    for (i = 0; i < 3; i++)
      mob->armor[i]           = interpolate(mob->level,100,-100);

    mob->armor[3]               = interpolate(mob->level,100,0);
    mob->hitroll = mob->damroll = number_fuzzy(mob->level / 2);

    mob->damage[0]=(mob->level)/20;
    mob->damage[1]=(mob->level)/2;
    mob->damage[2]=(mob->level)/5;

    for (i = 0; i < MAX_STATS; i++)
      mob->perm_stat[i] = 11 + mob->level/4;

    /* You rang? */
    char_to_room( mob, ch->in_room );
    act( "$p springs to life as a hideous zombie!", ch, obj, NULL, TO_ROOM );
    act( "$p springs to life as a hideous zombie!", ch, obj, NULL, TO_CHAR );

    extract_obj(obj);

    /* Yessssss, massssssster... */
    SET_BIT(mob->affected_by, AFF_CHARM);
    SET_BIT(mob->act, ACT_PET);
    SET_BIT(mob->act, ACT_WARRIOR);
    mob->chan_flags = CHANNEL_NOTELL|CHANNEL_NOSHOUT;
    mob->pen_flags = PEN_NOCHANNELS;
    add_follower( mob, ch );
    mob->leader = ch;
    mob->group_num = ch->group_num;

    if (ch->leader != NULL && ch->leader != ch)
    {
      mob->leader = ch->leader;
      act_new("$N joins $n's group.",ch->leader,NULL,mob,TO_NOTVICT,POS_RESTING);
      act_new("$N joins your group.",ch->leader,NULL,mob,TO_CHAR,POS_SLEEPING);
    }

    if ( ch->clan )
    {
      free_string( mob->clan_name );
      mob->clan_name = str_dup( ch->clan_name, mob->clan_name );
      mob->clan = ch->clan;
    }
    ch->pet = mob;
    /* For a little flavor... */
    do_function(mob, &do_say, "How may I serve you, master?" );
  }
  return;
}

/* ================================ */
/*
   Ok, this is for making trophy bags.  Here is the idea.  Player kills some
major mobile, like Ares and can take the corpse and skin it and make a bag
out of it.  You can do this with player corpses as well, works great if you
have a PKILL mud.

   I originally set this up as a NPC only spell, like dragon breath.  The idea
was there would be a scroll of it, and also a taxidermist shopkeeper who would
do it for you.  On the MUD where it went in, it was made a player spell and
given to some kind of woodsman class.

   You can fiddle with the amount these bags will contain below.  Note that if
you decide to go with the taxidermist idea, and make him a shopkeeper, he will
buy corpses for 1-2gp and make 1000gp trophy bags.  Because of how Merc is set
up, you also need to change the "level" of corpses to level 1 instead of level
0, shopkeepers don't buy level 0 stuff.

*/

void spell_make_bag( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  static char *headers[] = { "corpse of the ", "corpse of The ",
                             "corpse of an ", "corpse of An ",
                             "corpse of a ", "corpse of A ",
                             "corpse of "
                           }; /* (This one must be last)*/
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  OBJ_DATA *t_obj;
  OBJ_DATA *tempobj, *tempobj_next;
  char buf[MAX_STRING_LENGTH];
  int i;

  if (strstr(obj->short_descr, "Embalmed"))
  {
    send_to_char("That item cannot be embalmed.\n\r",ch);
    return;
  }

  if ( obj->item_type != ITEM_CORPSE_NPC
       &&   obj->item_type != ITEM_CORPSE_PC)
  {
    if ((obj->pIndexData->vnum == OBJ_VNUM_GUTS) ||
        (obj->pIndexData->vnum == OBJ_VNUM_SEVERED_HEAD)  ||
        (obj->pIndexData->vnum == OBJ_VNUM_TORN_HEART)  ||
        (obj->pIndexData->vnum == OBJ_VNUM_SLICED_ARM)  ||
        (obj->pIndexData->vnum == OBJ_VNUM_SLICED_LEG)  ||
        (obj->pIndexData->vnum == OBJ_VNUM_BRAINS)  ||
// added by Taeloch
        (obj->pIndexData->vnum == OBJ_VNUM_HANDS)  ||
        (obj->pIndexData->vnum == OBJ_VNUM_FEET)  ||
        (obj->pIndexData->vnum == OBJ_VNUM_FINGERS)  ||
        (obj->pIndexData->vnum == OBJ_VNUM_EAR)  ||
        (obj->pIndexData->vnum == OBJ_VNUM_EYE)  ||
        (obj->pIndexData->vnum == OBJ_VNUM_LONG_TONGUE)  ||
        (obj->pIndexData->vnum == OBJ_VNUM_EYESTALKS)  ||
        (obj->pIndexData->vnum == OBJ_VNUM_TENTACLES)  ||
        (obj->pIndexData->vnum == OBJ_VNUM_FINS)  ||
        (obj->pIndexData->vnum == OBJ_VNUM_WINGS)  ||
        (obj->pIndexData->vnum == OBJ_VNUM_TAIL))
    {


      act( "You have embalmed $p.", ch, obj, NULL, TO_CHAR );
      act( "$n has embalmed $p.", ch, obj, NULL, TO_ROOM );

      obj->timer = 0;
      mprintf(sizeof(buf), buf,"{D(Embalmed){x %s", obj->short_descr);
      free_string(obj->short_descr);
      obj->short_descr = str_dup(buf, obj->short_descr);
      mprintf(sizeof(buf), buf,"{D(Embalmed){x %s", obj->description);
      free_string(obj->description);
      obj->description = str_dup(buf, obj->description);

      return;
    }
    else
    {
      send_to_char("Embalm Failed to find correct item.\n\r",ch);
      return;
    }
  }
  t_obj = create_object( get_obj_index(OBJ_VNUM_BAG ), level/3);

  if (is_name("pcorpse",obj->name))
    mprintf(sizeof(buf), buf,"{D(Embalmed){x %s", obj->name);
  else
    mprintf(sizeof(buf), buf,"{D(Embalmed){x %s", obj->name);
  free_string(t_obj->name);
  t_obj->name = str_dup(buf,t_obj->name);
  mprintf(sizeof(buf), buf,"{D(Embalmed){x %s", obj->short_descr);
  free_string(t_obj->short_descr);
  t_obj->short_descr = str_dup(buf,t_obj->short_descr);
  mprintf(sizeof(buf), buf,"{D(Embalmed){x %s", obj->description);
  free_string(t_obj->description);
  t_obj->description = str_dup(buf,t_obj->description);

  for (i = 0; i < 7; i++)
  {
    int len = strlen(headers[i]);
    if ( memcmp(t_obj->short_descr, headers[i], len) == 0 )
    {
      mprintf( sizeof(buf), buf, "{D(Embalmed){x corpse %s", obj->short_descr+len );
      free_string( t_obj->name );
      t_obj->name = str_dup(buf, t_obj->name);

      mprintf( sizeof(buf), buf, "An {D(Embalmed){x corpse of fine %s hide catches your eye.  ",
               obj->short_descr+len );
      free_string( t_obj->description );
      t_obj->description = str_dup( buf , t_obj->description);

      mprintf( sizeof(buf), buf, "An {D(Embalmed){x corpse made from %s hide", obj->short_descr+len );
      free_string( t_obj->short_descr );
      t_obj->short_descr = str_dup( buf , t_obj->short_descr);

      break;
    }
  }

  t_obj->weight = obj->weight;
  obj_from_char(obj);
  t_obj->level = level;
  t_obj->cost = level * 50;
  t_obj->value[0] = level * 10;                 /* Weight capacity */
  t_obj->value[1] = 1;                          /* Closeable */
  t_obj->value[2] = -1;                         /* No key needed */

  for (tempobj = obj->contains; tempobj; tempobj = tempobj_next)
  {
    tempobj_next = tempobj->next_content;
    obj_from_obj(tempobj);
    obj_to_obj(tempobj, t_obj);
  }

  act( "You embalm $p.", ch, obj, NULL, TO_CHAR );
  act( "$n's embalms $p.", ch, obj, NULL, TO_ROOM );
  obj_to_char(t_obj, ch);
  send_to_char( "Ok.\n\r", ch );
  extract_obj(obj);
  return;
}

/* ================================ */
/* Original Code by Jason Huang (god@sure.net).                       */
/* Permission to use this code is granted provided this header is     */
/* retained and unaltered.                                            */

void spell_fear( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if (victim == ch
      || !victim->in_room
      || IS_SET( victim->in_room->room_flags, ROOM_SAFE      )
      || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
      || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  )
      || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
      || victim->level >= level
      || victim->in_room->area != ch->in_room->area
      || (saves_spell( level, victim, DAM_MENTAL ) ) )
  {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  act ("You attempt to fill $N with the urge to flee.",
       ch, NULL, victim, TO_CHAR );
  act( "$n attempts to fill $N with the urge to flee.",
       ch, NULL, victim, TO_ROOM );
  do_function(victim, &do_flee, "" );
  return;
}

/* ================================ */

bool fumble_obj( CHAR_DATA *victim, OBJ_DATA *obj_drop, int level, bool drop )
{
  if ( drop )
  {
    if ( !can_drop_obj( victim, obj_drop ) )
      return FALSE;
  }
  else
  {
    if ( IS_OBJ_STAT( obj_drop, ITEM_NOREMOVE ) )
      return FALSE;
  }

  if ( saves_spell( level, victim, DAM_BASH) )
  {
    act( "You nearly $T $p, but manage to keep your grip.",
         victim, obj_drop, drop ? "drop" : "lose hold of", TO_CHAR );
    act( "$n nearly $T $p, but manages to keep $s grip.",
         victim, obj_drop, drop ? "drops" : "loses hold of", TO_ROOM );
    return FALSE;
  }

  if ( drop )
  {
    obj_from_char( obj_drop );
    obj_to_room( obj_drop, victim->in_room );
  }
  else
  {
    unequip_char( victim, obj_drop );
  }

  act( "You fumble and $T $p!",
       victim, obj_drop, drop ? "drop" : "lose hold of", TO_CHAR );
  act( "$n fumbles and $T $p!",
       victim, obj_drop, drop ? "drops" : "loses hold of", TO_ROOM );
  return TRUE;
}


void spell_fumble( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  OBJ_DATA   *obj;
  OBJ_DATA   *obj_drop;
  int         carry;
  int         check;
  int         count;
  int         drop;
  bool        fumbled = FALSE;

  if ( !IS_AWAKE( victim ) )
    return;

  carry = 0;
  if (( ch->fighting == NULL )&& (victim != ch))
    set_fighting( ch, victim );

  for ( obj = victim->carrying; obj; obj = obj->next_content )
    carry++;

  drop = carry - can_carry_n( victim ) + 5;

  for ( check = 0; check < drop; check++ )
  {
    obj_drop = NULL;
    count = 0;

    for ( obj = victim->carrying; obj; obj = obj->next_content )
    {
      if ( obj->wear_loc == WEAR_NONE
           && number_range( 0, count++ ) == 0 )
        obj_drop = obj;
    }

    if ( !obj_drop )
      break;

    fumbled = fumble_obj( victim, obj_drop, level, TRUE ) || fumbled;
  }

  if ( ( obj_drop = get_eq_char( victim, WEAR_HOLD ) ) )
    fumbled = fumble_obj( victim, obj_drop, level, FALSE ) || fumbled;

  if ( ( obj_drop = get_eq_char( victim, WEAR_LIGHT ) ) )
    fumbled = fumble_obj( victim, obj_drop, level, FALSE ) || fumbled;

  if ( ( obj_drop = get_eq_char( victim, WEAR_WIELD ) ) )
    fumbled = fumble_obj( victim, obj_drop, level, FALSE ) || fumbled;

  if ( !fumbled )
  {
    send_to_char( "You stumble momentarily, but quickly recover.\n\r",
                  victim );
    act( "$n stumbles momentarily, but quickly recovers.",
         victim, NULL, NULL, TO_ROOM );
  }

  return;
}
/* ================================ */

void spell_martyr( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_NPC(ch))
  {
    send_to_char("You cannot martyr yourself.\n\r",ch);
    return;
  }
  if ( IS_SET(ch->spell_aff, SAFF_MARTYR))
  {
    send_to_char("You are already martyred.\n\r",ch);
    return;
  }
  if (victim != ch)
  {
    send_to_char("You cannot cast this on anyone else\n\r",ch);
    return;
  }
  af.where   = TO_SPELL_AFFECTS;
  af.type      = sn;
  af.level   = level;
  af.duration  = level/8;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = SAFF_MARTYR;
  affect_to_char( victim, &af );
  send_to_char( "You feel martyred.\n\r", victim );
  return;
}

void martyr_char(CHAR_DATA *ch, int sn)
{
  CHAR_DATA *pChar, *pChar_next;
  int dam;
  char buf[MSL];
  remove_affect(ch, TO_SPELL_AFFECTS, SAFF_MARTYR);
  dam = dice(ch->level, 120);
  pChar_next = NULL;
  ch->mana = 0;
  if ( ch->clan )
    mprintf( sizeof( buf ), buf,
             "emote has martyred %sself for %s{x.\n\r",
             ch->sex < 2 ? "him" : "her",
             ch->clan->clan_immortal );
  else
    mprintf(sizeof(buf), buf,
            "emote has martyred %sself for {CM{cir{Ml{mya.",
            ch->sex < 2 ? "him" : "her" );

  do_function(ch,&do_info, buf);

  /* This spell will when used.. wipe out your mana */
  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
  {
    pChar_next = pChar->next_in_room;
    if ( !is_safe_spell( ch, pChar, FALSE, TRUE ) &&
         !IS_WIZINVIS(pChar,ch) && (ch != pChar) )
    {
      act( "$n sacrifices themself in a massive {re{Rxp{Yl{ros{Ri{Yo{rn{x.", ch, NULL, pChar, TO_VICT    );
      if ( saves_spell( ch->level, pChar, DAM_FIRE ) )
        dam /= 2;
      fire_effect((void *) pChar, ch->level/2,dam, TARGET_CHAR);
      damage(ch, pChar, dam, sn , DAM_FIRE, TRUE, TRUE);
    }
  }


}
/* ================================ */
void spell_phoenix( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *pChar, *pChar_next;
  int dam;
  bool found = FALSE;
  dam = level * 2;
  pChar_next = NULL;

  /* This spell will when used.. wipe out your mana */
  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
  {
    pChar_next = pChar->next_in_room;
    if ( !is_safe_spell( ch, pChar, FALSE, TRUE ) && (pChar != ch) && !IS_WIZINVIS(pChar,ch))
    {
      act( "$n erupts in {rf{Rl{yam{re{ys{x, scorching everything.", ch, NULL, pChar, TO_VICT    );
      if ( saves_spell( level, pChar, DAM_FIRE ) )
        dam /= 2;
      fire_effect((void *) pChar, level/2,dam, TARGET_CHAR);
      damage(ch, pChar, dam, sn, DAM_FIRE, TRUE, TRUE);
      found = TRUE;
    }
  }
  ch->hit += level;
  if (ch->hit > ch->max_hit)
    ch->hit = ch->max_hit;

  if (!found)
    act( "There is no one here to burn...", ch, NULL, pChar, TO_CHAR );
}

/* ================================ */

void spell_water_walk( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
  {
    if (victim == ch)
      send_to_char("You can already walk on water.\n\r",ch);
    else
      send_to_char("That person can already walk on water.\n\r", ch);
    return;
  }

  af.where   = TO_SPELL_AFFECTS;
  af.type      = sn;
  af.level   = level;
  af.duration  = level/2;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = SAFF_WALK_ON_WATER;
  affect_to_char( victim, &af );
  send_to_char( "Your feet feel {Bbouyant{x.\n\r{x", victim );
  if (victim != ch)
    act("$N is now {Bbouyant{x.", ch, NULL, victim, TO_CHAR);
  return;
}
/* ================================ */

void spell_regeneration( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (victim !=  ch)
  {
    send_to_char("Sorry, you can only cast this on yourself.\n\r",ch);
    return;
  }

  if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_REGENERATION))
  {
    send_to_char("You can't regenerate any faster.\n\r",ch);

    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/4;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_REGENERATION;
  affect_to_char( victim, &af );
  send_to_char( "You feel yourself starting to heal faster.\n\r", victim );
  act("$n starts to regenerate $mself.",victim,NULL,NULL,TO_ROOM);
  return;
}

/* ================================ */
void spell_holy_bolt( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,i;
  ROOM_INDEX_DATA *ch_orig_room, *victim_orig_room;

  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
  {
    send_to_char("You are not righteous enough to be granted your holy powers.\n\r", ch);
    return;
  }

  level  = UMAX(0, level);
  /* 1 Missile for every ten levels of the caster */

  if (ch->level<10) i=1;
  else i=ch->level/10+1;

  ch_orig_room = ch->in_room;
  victim_orig_room = victim->in_room;

  for (; i>=0; i--)
  {
    dam= magic_dam(sn,level);
    if ( saves_spell( level, victim,DAM_HOLY) )
      dam /= 2;
    if (victim->in_room != ch->in_room || victim->position == POS_DEAD)
      return;
    damage( ch, victim, dam, sn, DAM_HOLY ,TRUE, TRUE);
    if (ch->in_room != ch_orig_room || victim->in_room != victim_orig_room)
      i = -1;
  }
  return;
}

#ifdef INQUISITOR
void spell_dark_fire( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,i;
  ROOM_INDEX_DATA *ch_orig_room, *victim_orig_room;

  if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
  {
    send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
    return;
  }

  level = UMAX(0, level);
  /* 1 Missile for every ten levels of the caster */

  if (ch->level<10) i=1;
  else i=ch->level/10+1;

  ch_orig_room = ch->in_room;
  victim_orig_room = victim->in_room;

  for (; i>=0; i--)
  {
    dam= magic_dam(sn,level);
    if ( saves_spell( level, victim,DAM_NEGATIVE) )
      dam /= 2;
    if (victim->in_room != ch->in_room || victim->position == POS_DEAD)
      return;
    damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, TRUE);
    if (ch->in_room != ch_orig_room || victim->in_room != victim_orig_room)
      i = -1;
  }
  return;
}
#endif
/* ================================ */

/*
 * Spell functions.
 */
void spell_ionwave( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *pChar, *pChar_next;
  int dam;
  pChar_next = NULL;

  /* This spell will when used.. wipe out your mana */
  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
  {
    pChar_next = pChar->next_in_room;
    if ( !is_safe_spell( ch, pChar, FALSE, TRUE ) && (pChar != ch) && (!IS_WIZINVIS(pChar,ch)))
    {
      act( "$n sends forth a wave of {Ben{ce{grgy{x.", ch, NULL, pChar, TO_VICT    );
      dam = magic_dam(sn,level);
      if ( saves_spell( level, pChar, DAM_ENERGY ) )
        dam /= 2;
      damage( ch, pChar, dam, sn, DAM_ENERGY,TRUE, TRUE);
    }
  }
  return;
}
/* ================================ */
void spell_vaccine( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

#ifdef INQUISITOR
  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
  {
    send_to_char("You are not righteous enough to be granted your holy powers.\n\r", ch);
    return;
  }
  else if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
  {
    send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
    return;
  }

  if ((IS_CLASS_INQUISITOR(ch) || IS_CLASS_OCCULTIST(ch))
      && victim != ch)
  {
    send_to_char("You may only vaccinate yourself.\n\r", ch);
    return;
  }
#endif
  if ( is_affected( victim, sn )
       || is_affected( victim, skill_lookup("mass vaccine"))
       || (IS_SET(victim->res_flags,IMM_DISEASE)))
  {
    if (victim == ch)
      send_to_char("You are already vaccinated.\n\r",ch);
    else
      act("$N is already vaccinated.",ch,NULL,victim,TO_CHAR);
    return;
  }
  af.where     = TO_RESIST;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = IMM_DISEASE;
  affect_to_char( victim, &af );
  send_to_char( "You are vaccinated from {gd{Di{rs{gease{x.\n\r{x", victim );
  if ( ch != victim )
    act("$N is vaccinated from {gd{Di{rs{gease{x.",ch,NULL,victim,TO_CHAR);
  return;

}
/* ================================ */
void spell_fish_breath( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) || (IS_SET(victim->affected_by, AFF_SWIM)))
  {
    if (victim == ch)
      send_to_char("You can already swim and breathe underwater.\n\r",ch);
    else
      send_to_char("That person can already swim and breathe underwater.\n\r", ch);

    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/2;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_SWIM;
  affect_to_char( victim, &af );
  send_to_char(
    "You feel like you can swim and breathe {Bunder{cwater{x.\n\r{x", victim );

  if (ch != victim)
    act("$N is now able to swim and breathe {Bunder{cwater{x.", ch, NULL, victim, TO_CHAR);

  return;
}

/* ================================ */
void spell_mind_shield( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ||
       (IS_SET(ch->res_flags,RES_MENTAL &&
               IS_SET(ch->res_flags,RES_CHARM))))
  {
    if (victim == ch)
      send_to_char("You already have a mind shield.\n\r",ch);
    else
      act("$N is already protected.",ch,NULL,victim,TO_CHAR);
    return;
  }
  af.where     = TO_RESIST;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/4;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = RES_MENTAL;
  affect_to_char( victim, &af );

  af.bitvector = RES_CHARM;
  affect_to_char( victim, &af );
  send_to_char( "Your mind is protected from intrusion.\n\r{x", victim );
  if ( ch != victim )
    act("$N now has a mind shield.",ch,NULL,victim,TO_CHAR);
  return;

}
/* ================================ */

bool spell_imprint( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  OBJ_DATA  *obj = (OBJ_DATA *) vo;
  int       sp_slot, i, j, mana, count;
  char      buf[MSL], buf2[MSL], buf3[MSL];
  bool      failed_percent= FALSE, repeat;

  if (skill_table[sn].spell_fun == spell_null )
  {
    send_to_char("That is not a spell.\n\r",ch);
    return FALSE;
  }

  /* counting the number of spells contained within */

  for (sp_slot = i = 1; i < 4; i++)
    if (obj->value[i] != 0 && obj->value[i] != -1)
      sp_slot++;

  if (sp_slot > 4)
  {
    act ("$p cannot contain any more spells.", ch, obj, NULL, TO_CHAR);
    return FALSE;
  }

  /* scribe/brew/craft costs 4 times the normal mana required to cast the spell */
  if (target < 2)
    mana = 4 * skill_table[sn].min_mana;
  else
    mana = 6 * skill_table[sn].min_mana;


  if ( !IS_NPC(ch) && ch->mana < mana )
  {
    send_to_char( "You don't have enough mana.\n\r", ch );
    return FALSE;
  }


  if ( number_percent( ) > get_skill(ch, sn) )
  {
    send_to_char( "You lost your concentration.\n\r", ch );
    ch->mana -= mana / 2;
    return FALSE;
  }

  /* executing the imprinting process */
  obj->value[sp_slot] = sn;

  if (IS_CLASS_ALCHEMIST(ch))
  {
    ch->mana -= (mana*2/3);
    switch ( sp_slot )
    {
      default:
        bug( "sp_slot has more than %d spells.", sp_slot );
        return FALSE;
      case 1:
        if ( number_percent() > 60 )
          failed_percent = TRUE;
        break;
      case 2:
        if ( number_percent() > 45 )
          failed_percent = TRUE;
        break;
      case 3:
        if ( number_percent() > 30 )
          failed_percent = TRUE;
        break;
      case 4:
        if ( number_percent() > 15 )
          failed_percent = TRUE;
        break;
    }
  }
  else
  {
    ch->mana -= mana;
    switch ( sp_slot )
    {
      default:
        bug( "sp_slot has more than %d spells.", sp_slot );
        return FALSE;
      case 1:
        if ( number_percent() > 90 )
          failed_percent = TRUE;
        break;
      case 2:
        if ( number_percent() > 55 )
          failed_percent = TRUE;
        break;
      case 3:
        if ( number_percent() > 35 )
          failed_percent = TRUE;
        break;
      case 4:
        if ( number_percent() > 20 )
          failed_percent = TRUE;
        break;
    }
  }
  if (failed_percent)
  {
    act ("$p vanishes, the magic enchantment has failed.", ch, obj, NULL, TO_CHAR);
    extract_obj( obj );
    return FALSE;
  }


  /* labeling the item */

  mprintf (sizeof(buf),  buf, "a %s of ",
           flag_string(type_flags, obj->item_type));
  mprintf (sizeof(buf3), buf3, "A %s of ",
           flag_string( type_flags, obj->item_type ) );

  for (i = 1; i <= sp_slot ; i++)
  {
    repeat = FALSE;
    if (obj->value[i] != 0)
    {
      if (i > 1)
      {
        for (j = 1; j < i; j++)
        {
          if (!str_cmp(skill_table[obj->value[i]].name, skill_table[obj->value[j]].name))
            repeat = TRUE;
        }
      }

      if (!repeat)
      {
        count = 1;

        if (i > 1)
        {
          strcat(buf, ", ");
          strcat(buf3,", ");
        }

        for (j = i + 1; j <= sp_slot; j++)
        {
          if (!str_cmp(skill_table[obj->value[i]].name, skill_table[obj->value[j]].name))
            count++;
        }

        if (count > 1)
        {
          mprintf(sizeof(buf2), buf2, "%d x %s", count, skill_table[obj->value[i]].name );
        }
        else
        {
          mprintf(sizeof(buf2), buf2, "%s", skill_table[obj->value[i]].name );
        }

        strcat(buf, buf2);
        strcat(buf3, buf2);
      }
    }
  }

  free_string( obj->short_descr );
  obj->short_descr = str_dup(buf, obj->short_descr );

  strcat( buf3, "." );
  free_string(obj->description );
  obj->description = str_dup(buf3, obj->description );

  free_string(obj->name);
  obj->name = str_dup( buf ,obj->name);

  act ("You have created $p.", ch, obj, NULL, TO_CHAR);

  return TRUE;
}

/* ================================ */

void spell_erase( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int       sp_slot, i;

  /* counting the number of spells contained within */

  for (sp_slot = i = 1; i < 4; i++)
    if (obj->value[i] != 0 || obj->value[i] != -1)
      sp_slot++;

  if ((number_percent() + (sp_slot*3)) >  get_skill(ch,sn))
  {
    if (sn == gsn_empty)
      act ("$p vanishes, the magic emptying has failed.", ch, obj, NULL, TO_CHAR);
    else
      act ("$p vanishes, the magic erasing has failed.", ch, obj, NULL, TO_CHAR);
    extract_obj( obj );
    return;
  }

  act ("You have removed all spells from $p.", ch, obj, NULL, TO_CHAR);
  extract_obj(obj);
  if (sn == gsn_erase)
    obj = create_object (get_obj_index(OBJ_VNUM_CLEAN_SCROLL),0);
  else
    obj = create_object (get_obj_index(OBJ_VNUM_CLEAN_VIAL),0);
  obj_to_char(obj,ch);
  return;
}


/* ================================ */

/*
 * Spell functions.
 */
void spell_incinerate( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int i=3;
  int dam;

  for (i=3; i>0; i--)
  {
    dam = magic_dam(sn,level);
    if ( saves_spell( level, victim, DAM_ACID ) )
      dam /= 2;
    if ((victim->in_room == ch->in_room) && (victim->position != POS_DEAD))
      if (damage( ch, victim, dam, sn,DAM_ACID,TRUE, TRUE) == FALSE)
        return;
  }
  return;
}

/* ================================ */


/*
 * Spell functions.
 */
void spell_icicle( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int dam;

  if (is_affected(victim,sn))
  {
    send_to_char("They are already frozen.\n\r",ch);
    return;
  }

  dam = magic_dam(sn,level);
  if ( !saves_spell( level, victim, DAM_COLD ) )
  {
    act("$n turns {Bbl{cue{x and {csh{Bi{cver{Bs{x.",victim,NULL,NULL,TO_ROOM);
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/8;
    af.location  = APPLY_STR;
    af.modifier  = -2;
    af.bitvector = 0;
    affect_join( victim, &af );
  }
  else
  {
    dam /= 2;
  }
  damage( ch, victim, dam, sn,DAM_COLD,TRUE, TRUE);
  return;
}

/* ================================ */

void spell_bigby_bash( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  level  = UMAX(0, level);
  dam = magic_dam(sn,level);
  if ( saves_spell( level, victim, DAM_MENTAL) )
    dam /= 2;
  damage( ch, victim, dam, sn, DAM_MENTAL ,TRUE, TRUE);
  return;
}
/* ================================ */
/*
Flame aura: makes caster resist fire
*/
void spell_flame_aura( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_aura_spelled(victim) )
  {
    if (victim == ch)
      send_to_char("You already have an aura.\n\r",ch);
    else
      act("$N already has an aura.",ch,NULL,victim,TO_CHAR);
    return;
  }
  if ((IS_SET(victim->res_flags,RES_FIRE)) )
  {
    if (victim == ch)
      send_to_char("You are already protected from fire.\n\r",ch);
    else
      act("$N is already protected from fire.",ch,NULL,victim,TO_CHAR);
    return;
  }

  af.where     = TO_RESIST;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/8;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = RES_FIRE;
  affect_to_char( victim, &af );
  send_to_char( "You feel protected by a {rf{yl{Ra{rme{x aura.\n\r{x", victim );
  if ( ch != victim )
    act("$N is protected by a {rf{yl{Ra{rme{x aura.",ch,NULL,victim,TO_CHAR);
  return;

}
/*
Poison aura: makes caster resist poison
*/
void spell_poison_aura( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_aura_spelled(victim) )
  {
    if (victim == ch)
      send_to_char("You already have an aura.\n\r",ch);
    else
      act("$N already has an aura.",ch,NULL,victim,TO_CHAR);
    return;
  }
  if ((IS_SET(victim->res_flags,RES_POISON)) )
  {
    if (victim == ch)
      send_to_char("You are already protected from poison.\n\r",ch);
    else
      act("$N is already protected from poison.",ch,NULL,victim,TO_CHAR);
    return;
  }

  af.where     = TO_RESIST;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/8;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = RES_POISON;
  affect_to_char( victim, &af );
  send_to_char( "You feel protected by a {gpo{ci{Gs{gon{x aura.\n\r{x", victim );
  if ( ch != victim )
    act("$N is protected by a {gpo{ci{Gs{gon{x aura.",ch,NULL,victim,TO_CHAR);
  return;

}
/* ================================ */
/*
Frost aura: makes caster resist cold
*/
void spell_frost_aura( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_aura_spelled(victim) )
  {
    if (victim == ch)
      send_to_char("You already have an aura.\n\r",ch);
    else
      act("$N already has an aura.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if ((IS_SET(victim->res_flags,RES_COLD)) )
  {
    if (victim == ch)
      send_to_char("You are already protected from the cold.\n\r",ch);
    else
      act("$N is already protected from the cold.",ch,NULL,victim,TO_CHAR);
    return;
  }

  af.where     = TO_RESIST;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/8;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = RES_COLD;
  affect_to_char( victim, &af );
  send_to_char( "You feel protected by a {Wf{Cr{Bo{bst{x aura.\n\r{x", victim );
  if ( ch != victim )
    act("$N is protected by a {Wf{Cr{Bo{bst{x aura.",ch,NULL,victim,TO_CHAR);
  return;

}
/* ================================ */
/*
Electric aura: makes caster resist lightning
*/
void spell_electric_aura( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_aura_spelled(victim) )
  {
    if (victim == ch)
      send_to_char("You already have an aura.\n\r",ch);
    else
      act("$N already has an aura.",ch,NULL,victim,TO_CHAR);
    return;
  }
  if ((IS_SET(victim->res_flags,RES_LIGHTNING)) )
  {
    if (victim == ch)
      send_to_char("You are already protected from lightning.\n\r",ch);
    else
      act("$N is already protected from lightning.",ch,NULL,victim,TO_CHAR);
    return;
  }
  af.where     = TO_RESIST;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/8;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = RES_LIGHTNING;
  affect_to_char( victim, &af );
  send_to_char( "You feel protected by an {ye{Wl{ye{Yc{Wt{yr{Wi{yc{x aura.\n\r{x", victim );
  if ( ch != victim )
    act("$N is protected by an {ye{Wl{ye{Yc{Wt{yr{Wi{yc{x aura.",ch,NULL,victim,TO_CHAR);
  return;

}

/* ================================ */
/*
Corrosive aura: makes caster resist acid
*/
void spell_corrosive_aura( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_aura_spelled(victim) )
  {
    if (victim == ch)
      send_to_char("You already have an aura.\n\r",ch);
    else
      act("$N already has an aura.",ch,NULL,victim,TO_CHAR);
    return;
  }
  if ((IS_SET(victim->res_flags,RES_ACID)) )
  {
    if (victim == ch)
      send_to_char("You are already protected from acid.\n\r",ch);
    else
      act("$N is already protected from acid.",ch,NULL,victim,TO_CHAR);
    return;
  }
  af.where     = TO_RESIST;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/8;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = RES_ACID;
  affect_to_char( victim, &af );
  send_to_char( "You feel protected by a {cco{gr{Cr{cos{Gi{cve{x aura.\n\r{x", victim );
  if ( ch != victim )
    act("$N is protected by a {cco{gr{Cr{cos{Gi{cve{x aura.",ch,NULL,victim,TO_CHAR);
  return;

}


/* ================================ */
/*
Arcane aura: makes caster resist magic
*/
void spell_arcane_aura( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_aura_spelled(victim) )
  {
    if (victim == ch)
      send_to_char("You already have an aura.\n\r",ch);
    else
      act("$N already has an aura.",ch,NULL,victim,TO_CHAR);
    return;
  }
  if ((IS_SET(ch->res_flags,RES_ENERGY)) )
  {
    if (victim == ch)
      send_to_char("You are already protected from energy magic.\n\r",ch);
    else
      act("$N is already protected from energy magic.",ch,NULL,victim,TO_CHAR);
    return;
  }
  af.where     = TO_RESIST;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/8;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = RES_ENERGY;
  affect_to_char( victim, &af );
  send_to_char( "You feel protected by an {Barc{mane{x aura.\n\r{x", victim );
  if ( ch != victim )
    act("$N is protected an {Barc{mane{x aura.",ch,NULL,victim,TO_CHAR);
  return;

}


/* ================================ */
/*
Holy aura: makes caster resist drain/negative/vampiric
*/
void spell_holy_aura( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_aura_spelled(victim) )
  {
    if (victim == ch)
      send_to_char("You already have an aura.\n\r",ch);
    else
      act("$N already has an aura.",ch,NULL,victim,TO_CHAR);
    return;
  }
  if ((IS_SET(ch->res_flags,RES_NEGATIVE)))
  {
    if (victim == ch)
      send_to_char("You are already protected from evil.\n\r",ch);
    else
      act("$N is already protected from evil.",ch,NULL,victim,TO_CHAR);
    return;
  }
  af.where     = TO_RESIST;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/8;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = RES_NEGATIVE;
  affect_to_char( victim, &af );
  send_to_char( "You feel protected by a {yh{Wo{yly{x aura.\n\r{x", victim );
  if ( ch != victim )
    act("$N is protected by a {yh{Wo{yly{x aura.",ch,NULL,victim,TO_CHAR);
  return;

}

/* ================================ */
/*
Dark aura: makes caster resist holy

maybe make this group for both mage and Priest but rule out
holy and dark for mage, and electric corrosive and arcane for Priest
*/


void spell_dark_aura( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_aura_spelled(victim) )
  {
    if (victim == ch)
      send_to_char("You already have an aura.\n\r",ch);
    else
      act("$N already has an aura.",ch,NULL,victim,TO_CHAR);
    return;
  }
  if ((IS_SET(ch->res_flags,RES_HOLY)) )
  {
    if (victim == ch)
      send_to_char("You are already protected from good.\n\r",ch);
    else
      act("$N is already protected from good.",ch,NULL,victim,TO_CHAR);
    return;
  }
  af.where     = TO_RESIST;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/8;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = RES_HOLY;
  affect_to_char( victim, &af );
  send_to_char( "You feel protected by a {rda{Dr{rk{x aura.\n\r{x", victim );
  if ( ch != victim )
    act("$N is protected by a {rda{Dr{rk{x aura.",ch,NULL,victim,TO_CHAR);
  return;

}
/* ================================ */
/* Life transfer spell coded by Blizzard (blizzard_imp@hotmail.com) */
void spell_life_transfer( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  ch->hit /= 2;

  victim->hit += ch->hit;
  if ((victim->hit >= victim->max_hit ||
       victim->hit < 0))
    victim->hit=victim->max_hit;


  act( "You give half your life to $N.\n\r", ch, NULL, victim, TO_CHAR );
  act( "$n gives you half of their life.\n\r",  ch, NULL, victim, TO_VICT);
  act( "$n gives half of their life to $N.\n\r", ch, NULL, victim, TO_NOTVICT);

  return;
}

/* ================================ */
void spell_moon_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
  {
    if (victim == ch)
      send_to_char("You are already in moonlight.\n\r",ch);
    else
      act("$N is already bathed in moonlight.",ch,NULL,victim,TO_CHAR);
    return;
  }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/4 + 5;
  af.modifier  = -100;
  af.location  = APPLY_AC;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  send_to_char( "{wYou are bathed in {cm{Do{Bo{cn{Wlight{x.\n\r{x", victim );
  if ( ch != victim )
    act("$N is bathed in {cm{Do{Bo{cn{Wlight{x.",ch,NULL,victim,TO_CHAR);
  return;
}
/* ================================ */
void spell_power_transfer( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  ch->mana /= 2;

  victim->mana += ch->mana/2;
  if ((victim->mana > GET_MANA(victim)) ||
      victim->mana < 0)
    victim->mana = GET_MANA(victim);


  act( "You give half your {gmana{x to $N.\n\r", ch, NULL, victim, TO_CHAR );
  act( "$n gives you a fourth of their {gmana{x.\n\r",  ch, NULL, victim, TO_VICT);
  act( "$n gives a lot of {gmana{x to $N.\n\r", ch, NULL, victim, TO_NOTVICT);

  return;
}
/* ================================ */
void spell_confine( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (!IS_NPC(victim) || (IS_PET(victim)))
  {
    send_to_char("Spell failed\n\r",ch);
    if (!is_affected(victim, skill_lookup("sleep")))
      damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);
    return;
  };

  if ( is_affected( victim, sn ) ||  (IS_SET(victim->act, ACT_SENTINEL)))
  {
    if (victim == ch)
      send_to_char("You are already confined.\n\r",ch);
    else if (IS_SET(victim->act, ACT_WIMPY))
      act("You confine $N to this place.", ch, NULL, victim, TO_CHAR);
    else
      act("$N is already confined.",ch,NULL,victim,TO_CHAR);
    if (!is_affected(victim, skill_lookup("sleep")))
      damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);
    if (IS_SET(victim->act, ACT_WIMPY))
      REMOVE_BIT(victim->act, ACT_WIMPY);
    return;
  }
  af.where     = TO_ACT;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/8;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = ACT_SENTINEL;
  affect_to_char( victim, &af );
  if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);
  if (IS_SET(victim->act, ACT_WIMPY))
    REMOVE_BIT(victim->act, ACT_WIMPY);

  act( "You confine $N to this place.\n\r", ch, NULL, victim, TO_CHAR );
  act( "$n confines you to this place.\n\r",  ch, NULL, victim, TO_VICT);
  act( "$n confines $N to this place.\n\r", ch, NULL, victim, TO_NOTVICT);

  return;
}
/* ================================ */
/*
 * Spell functions.
 */
void spell_cone_of_cold( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = magic_dam(sn,level);
  if ( saves_spell( level, victim, DAM_COLD ) )
    dam /= 2;
  damage( ch, victim, dam, sn,DAM_COLD,TRUE, TRUE);
  return;
}

/* ================================ */
void spell_fire_elemental( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *elemental;

  if (num_followers(ch) > 0)
  {
    send_to_char("You already have a follower.\n\r",ch);
    return;
  }

  if (   IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
         || IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) )
  {
    send_to_char("You cannot summon a fire elemental here.\n\r", ch);
    return;
  }

  pMobIndex = get_mob_index( MOB_VNUM_FIRE_ELEMENTAL );

  elemental = create_mobile(pMobIndex);

  elemental->level = level-3;
  elemental->hit = elemental->max_hit = number_fuzzy( elemental->level * 40 + 500 );
  elemental->mana = elemental->max_mana = 100;
  elemental->move = elemental->max_move = number_fuzzy( elemental->level * 10 + 150 );
  elemental->armor[0]=elemental->armor[1]=elemental->armor[2]=elemental->armor[3]=number_fuzzy(elemental->level * -3 +180);
  elemental->hitroll = elemental->damroll = number_fuzzy(elemental->level / 2);

  elemental->damage[0]=(elemental->level)/20;
  elemental->damage[1]=(elemental->level)/2;
  elemental->damage[2]=(elemental->level)/5;

  free_string(elemental->name);
  free_string(elemental->short_descr);
  free_string(elemental->long_descr);
  free_string(elemental->description);

  elemental->name = str_dup("fire elemental flame",elemental->name);
  elemental->short_descr = str_dup("a fire elemental",elemental->short_descr);
  elemental->long_descr =
    str_dup("A {rf{yi{rre{x elemental scorches the air.\n\r",elemental->long_descr);
  elemental->description =
    str_dup("This fire elemental has been summoned from the depths.  It will burn you.\n\r",
            elemental->description);
  elemental->dam_type = 29;          /* scratch */
  SET_BIT(elemental->affected_by, AFF_CHARM);
  SET_BIT(elemental->act, ACT_PET);
  SET_BIT(elemental->act, ACT_WARRIOR);
  SET_BIT(elemental->act, ACT_NOGHOST);
  SET_BIT(elemental->affected_by, AFF_HASTE);
  elemental->clan = ch->clan;
  elemental->alignment = ch->alignment;
  act( "You create a {rf{yi{rre{x elemental.\n\r", ch, NULL, NULL, TO_CHAR );
  act( "$n has created a {rf{yi{rre{x elemental.\n\r", ch, NULL, NULL, TO_ROOM);
  char_to_room( elemental, ch->in_room );
  add_follower( elemental, ch );
  elemental->leader = ch;

  if ( ch->clan )
  {
    free_string( elemental->clan_name );
    elemental->clan_name = str_dup( ch->clan_name, elemental->clan_name );
    elemental->clan = ch->clan;
  }

  ch->pet = elemental;
  elemental->timer = level/2;

  if (ch->leader != NULL && ch->leader != ch)
  {
    elemental->leader = ch->leader;
    act_new("$N joins $n's group.",ch->leader,NULL,elemental,TO_NOTVICT,POS_RESTING);
    act_new("$N joins your group.",ch->leader,NULL,elemental,TO_CHAR,POS_SLEEPING);
  }

  return;
}

void spell_ice_elemental( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *elemental;

  if (num_followers(ch) > 0)
  {
    send_to_char("You already have a follower.\n\r",ch);
    return;
  }

  if (  IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
        || IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) )
  {
    send_to_char("You cannot summon an ice elemental here.\n\r", ch);
    return;
  }

  pMobIndex = get_mob_index( MOB_VNUM_ICE_ELEMENTAL );

  elemental = create_mobile(pMobIndex);

  elemental->level = level-1;
  elemental->hit = elemental->max_hit = number_fuzzy( elemental->level * 40 + 1000 );
  elemental->mana = elemental->max_mana = 100;
  elemental->move = elemental->max_move = number_fuzzy( elemental->level * 10 + 150 );
  elemental->armor[0]=elemental->armor[1]=elemental->armor[2]=elemental->armor[3] =
                                            number_fuzzy(elemental->level * -3 +180);
  elemental->hitroll = elemental->damroll = number_fuzzy(elemental->level / 2);
  elemental->gold = 0;
  elemental->silver = 0;

  elemental->damage[0]=(elemental->level)/20;
  elemental->damage[1]=(elemental->level)/2;
  elemental->damage[2]=(elemental->level)/5;

  free_string(elemental->name);
  free_string(elemental->short_descr);
  free_string(elemental->long_descr);
  free_string(elemental->description);

  elemental->name = str_dup("ice elemental",elemental->name);
  elemental->short_descr = str_dup("an ice elemental",elemental->short_descr);
  elemental->long_descr =
    str_dup("An {bi{cce{x elemental freezes the land.\n\r",elemental->long_descr);
  elemental->description =
    str_dup("This ice elemental has been summoned from the glaciers.  The cold is unbearable.\n\r",
            elemental->description);
  elemental->dam_type = 30;          /* freezing bite. */
  SET_BIT(elemental->affected_by, AFF_CHARM);
  SET_BIT(elemental->act, ACT_PET);
  SET_BIT(elemental->act, ACT_WARRIOR);
  SET_BIT(elemental->act, ACT_NOGHOST);
  SET_BIT(elemental->affected_by, AFF_HASTE);
  elemental->clan = ch->clan;
  elemental->alignment = ch->alignment;
  act( "You create an {bi{cce{x elemental.\n\r", ch, NULL, NULL, TO_CHAR );
  act( "$n has created an {bi{cce{x elemental.\n\r", ch, NULL, NULL, TO_ROOM);
  char_to_room( elemental, ch->in_room );
  add_follower( elemental, ch );
  elemental->leader = ch;

  if ( ch->clan )
  {
    free_string( elemental->clan_name );
    elemental->clan_name = str_dup( ch->clan_name, elemental->clan_name );
    elemental->clan = ch->clan;
  }

  ch->pet = elemental;
  elemental->timer = level/2;

  if (ch->leader != NULL && ch->leader != ch)
  {
    elemental->leader = ch->leader;
    act_new("$N joins $n's group.",ch->leader,NULL,elemental,TO_NOTVICT,POS_RESTING);
    act_new("$N joins your group.",ch->leader,NULL,elemental,TO_CHAR,POS_SLEEPING);
  }

  return;
}


/* ================================ */
/*
 * Spell functions.
 */
void spell_banshee_scream( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *pChar, *pChar_next;
  int dam;
  pChar_next = NULL;

  /* This spell will when used.. wipe out your mana */
  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
  {
    pChar_next = pChar->next_in_room;
    if ( !is_safe_spell( ch, pChar, FALSE, TRUE ) && (pChar != ch) && !IS_WIZINVIS(pChar,ch))
    {
      act( "$n lets loose an ear piercing {yscr{De{ram{x.", ch, NULL, pChar, TO_VICT    );
      dam = magic_dam(sn,level);
      if ( saves_spell( level, pChar, DAM_SOUND ) )
        dam /= 2;
      damage( ch, pChar, dam, sn, DAM_SOUND,TRUE, TRUE);
    }
  }
  return;
}
/* ================================ */
void spell_sunbeam( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ( !IS_OUTSIDE(ch) )
  {
    send_to_char( "You must be out of doors.\n\r", ch );
    return;
  }

  dam = magic_dam(sn,level);
  if ( saves_spell( level, victim, DAM_LIGHT ) )
    dam /= 2;
  damage( ch, victim, dam, sn,DAM_LIGHT,TRUE, TRUE);
  return;
}
/* ================================ */
void spell_sonic_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = magic_dam(sn,level);
  if ( saves_spell( level, victim, DAM_SOUND ) )
    dam /= 2;
  damage( ch, victim, dam, sn, DAM_SOUND,TRUE, TRUE);
  return;
}

/* ================================ */
void spell_insect_swarm( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,i;
  level  = UMAX(0, level);

  if (level<10) i=1;
  else i=level/10+1;

  act( "$n conjures up a large {gsw{carm{x of biting {Ginsect{gs{x.", ch, NULL, victim, TO_ROOM);
  for (; i>=0; i--)
  {
    dam = magic_dam(sn,level);
    if ( saves_spell( level, victim, DAM_PIERCE ) )
      dam /= 2;
    if (victim->in_room != ch->in_room || victim->position == POS_DEAD)
      return;
    damage( ch, victim, dam, sn,DAM_PIERCE,TRUE, TRUE);
  }
  return;
}
/* ================================ */
/*
 * Spell functions.
 */
void spell_tornado( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *pChar, *pChar_next;
  int dam;
  pChar_next = NULL;

  if ( !IS_OUTSIDE(ch) )
  {
    send_to_char( "You must be out of doors.\n\r", ch );
    return;
  }


  /* This spell will when used.. wipe out your mana */
  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
  {
    pChar_next = pChar->next_in_room;
    if ( !is_safe_spell( ch, pChar, FALSE, TRUE ) && (pChar != ch) && !IS_WIZINVIS(pChar,ch))
    {
      act( "$n conjures a raging {Dtorn{wa{Dd{wo{x.", ch, NULL, pChar, TO_VICT    );
      dam = magic_dam(sn,level);
      if ( saves_spell( level, pChar, DAM_WIND ) )
        dam /= 2;
      damage( ch, pChar, dam, sn, DAM_WIND,TRUE, TRUE);
    }
  }
  return;
}
/* ================================ */
void spell_hammer_of_thor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = magic_dam(sn,level);
  if ( saves_spell( level, victim, DAM_BASH ) )
    dam /= 2;
  act( "$n conjures up a {YHuge glowing {Dhammer{x that flies towards you!{x", ch, NULL, victim, TO_VICT);
  damage( ch, victim, dam, sn,DAM_BASH,TRUE, TRUE);
  return;
}

/* ================================ */
void spell_entangle( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim,AFF_SLOW))
  {
    //Check entangled first
    if ( is_affected(ch, sn) )
    {
      if (victim == ch)
        send_to_char("You are already entangled!\n\r",ch);
      else
        act("$N is already entangled in vines!", ch,NULL,victim,TO_CHAR);
    }
    else //Just slow...
    {
      if (victim == ch)
        send_to_char("You are already moving slowly!\n\r",ch);
      else
        act("$N is already moving slowly.", ch,NULL,victim,TO_CHAR);
    }

    if (!is_affected(victim, skill_lookup("sleep")))
      damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);

    return;
  }

  if (saves_spell(level,victim,DAM_OTHER)
      ||  IS_SET(victim->imm_flags,IMM_MAGIC))
  {
    if (victim != ch)
      send_to_char("Nothing seemed to happen.\n\r",ch);

    send_to_char("You feel a slight tugging at your feet.\n\r",victim);

    if (!is_affected(victim, skill_lookup("sleep")))
      damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);
    return;
  }

  if (IS_AFFECTED(victim,AFF_HASTE))
  {
    if (!check_dispel(level,victim,skill_lookup("haste")))
    {
      if (victim != ch)
        send_to_char("Spell failed.\n\r",ch);

      send_to_char("You feel momentarily slower.\n\r",victim);

      if (!is_affected(victim, skill_lookup("sleep")))
        damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);
      return;
    }

    act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);

    if (!is_affected(victim, skill_lookup("sleep")))
      damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);
    return;
  }


  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/18;
  af.location  = APPLY_DEX;
  af.modifier  = -1 - (level >= 9) - (level >= 18) - (level >= 25) -
                 (level >= 34) - (level >=42) - (level >=50);
  af.bitvector = AFF_SLOW;
  affect_to_char( victim, &af );
  if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);
  send_to_char( "You feel yourself becoming {gen{yt{gan{Gg{yl{ged{x in clinging vines.\n\r", victim );
  act("$n starts to become {gen{yt{gan{Gg{yl{ged{x in clinging vines.",victim,NULL,NULL,TO_ROOM);
  return;
}

/* ================================ */
void spell_flash( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *pChar, *pChar_next;
  int dam;
  pChar_next = NULL;

  /* This spell will when used.. wipe out your mana */
  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
  {
    pChar_next = pChar->next_in_room;
    if ( !is_safe_spell( ch, pChar, FALSE, TRUE ) && (pChar != ch) && !IS_WIZINVIS(pChar,ch))
    {
      act( "$n calls a blinding {Wflash{x of {ylight{x.", ch, NULL, pChar, TO_VICT    );
      dam = magic_dam(sn,level);
      if ( saves_spell( level, pChar, DAM_LIGHT ) )
        dam /= 2;
      else
        spell_blindness(skill_lookup("blindness"),
                        level/2, ch, (void *)pChar, TARGET_CHAR);
      damage( ch, pChar, dam, sn, DAM_LIGHT,TRUE, TRUE);
    }
  }
  return;
}
/* ================================ */
void spell_familiarize_weapon( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  /* characters */
  victim = (CHAR_DATA *) vo;

  if (IS_NPC(victim))
  {
    send_to_char("You failed.\n\r",ch);
    return;
  }

  obj = get_eq_char( victim, WEAR_WIELD );
  if (!obj || obj->item_type != ITEM_WEAPON)
  {
    send_to_char("There needs to be a weapon for that.\n\r",ch);
    return;
  }

  obj->famowner = str_dup(victim->name,obj->famowner);
  send_to_char("You suddenly feel as though you intimately know the secrets of your weapon.\n\r",victim);
  send_to_char("The weapon crackles with {marc{ba{mn{ce{x magic.\n\r",ch);
}


/* ================================ */
void spell_invulnerability( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (victim != ch)
  {
    send_to_char("You cannot cast this on another.\n\r",ch);
    return;
  }

  if (is_sanc_spelled(victim))
  {
    send_to_char("You are already surrounded by a globe.\n\r",ch);
    return;
  }

  af.where     = TO_AFFECTS2;
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 5;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF2_INVUN;
  affect_to_char( victim, &af );
  act( "$n is surrounded by a {Bmag{mical {bglobe{x.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "{wYou are surrounded by a {Bmag{mical {bglobe{w.\n\r", victim );
  return;
}
/* ================================ */
/*
 * Spell functions.
 */
void spell_thunder( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *pChar, *pChar_next;
  int dam;
  pChar_next = NULL;

  if ( !IS_OUTSIDE(ch) )
  {
    send_to_char( "You must be out of doors.\n\r", ch );
    return;
  }

  if ( weather_info.sky < SKY_CLOUDY )
  {
    send_to_char( "You need bad weather.\n\r", ch );
    return;
  }


  /* This spell will when used.. wipe out your mana */
  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
  {
    pChar_next = pChar->next_in_room;
    if ( !is_safe_spell( ch, pChar, FALSE, TRUE ) && (pChar != ch) && !IS_WIZINVIS(pChar,ch))
    {
      act( "$n conjures a horrendous clash of {Bth{Du{Bn{Dd{ber{x.", ch, NULL, pChar, TO_VICT    );
      dam = magic_dam(sn,level);
      if ( saves_spell( level, pChar, DAM_SOUND ) )
        dam /= 2;
      damage( ch, pChar, dam, sn, DAM_SOUND,TRUE,TRUE);
    }
  }
  return;
}
/* ================================ */
void spell_empower_weapon( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  OBJ_DATA *obj;
#if USE_EMPOWER_AVGDAM
  int avgdam=0;
#endif
  /* characters */
  obj = (OBJ_DATA *) vo;

  if (obj ==NULL)
  {
    send_to_char("That item is not in your inventory.\n\r",ch);
    return;
  }


  if (obj->level >= LEVEL_HERO)
  {
    send_to_char("That item cannot be raised any more.\n\r",ch);
    return;
  }


  obj->level += 1;
#if USE_EMPOWER_AVGDAM
  if (obj->item_type == ITEM_WEAPON)
  {
    if (obj->pIndexData->new_format)
    {
      avgdam = (1 + obj->value[2]) * obj->value[1] / 2;
      if (avgdam+1 > obj->level)
        avgdam = obj->level;
      obj->value[1] = avgdamtable[avgdam+1].num;
      obj->value[2] = avgdamtable[avgdam+1].dice;
    }
    else
    {
      obj->value[2] ++;
      obj->value[1] ++;
      /*  avgdam = ( obj->value[1] + obj->value[2] ) / 2;*/
    }

  }
#endif

  send_to_char("You have raised the power of the item.\n\r",ch);
}
/* ================================ */
void spell_dislocation( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;

  /* dea with the object case first */
  victim = (CHAR_DATA *) vo;

  if (is_affected( victim, sn ) )
  {
    if (victim == ch)
      send_to_char("You are already dislocated.\n\r",ch);
    else
      act("$N already is dislocated.",ch,NULL,victim,TO_CHAR);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level   = level;
  af.duration  = level;
  af.location  = APPLY_AC;
  af.modifier  = -30;
  af.bitvector = 0;
  affect_to_char( victim, &af );

  af.location  = APPLY_SAVING_SPELL;
  af.modifier  = -3;
  affect_to_char( victim, &af );
  send_to_char( "You feel out of sorts.\n\r", victim );
  if ( ch != victim )
    act("You dislocate $N from this plane.",ch,NULL,victim,TO_CHAR);
  return;
}

/* ================================ */
void spell_bear_spirit( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;
  /* dea with the object case first */
  victim = (CHAR_DATA *) vo;

  if (victim != ch)
  {
    send_to_char("You cannot cast this on another.\n\r",ch);
    return;
  }

  if (is_spirit_affected(victim))
  {
    send_to_char("You are already filled with a spirit.\n\r",ch);
    return;
  }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level   = level;
  af.duration  = level;
  af.location  = APPLY_STR;
  af.modifier  = +(1+ (int)(level/30));
  af.bitvector = 0;
  affect_to_char( victim, &af );

  af.location  = APPLY_HIT;
  af.modifier  = + (3 * (level/5));
  affect_to_char( victim, &af );

  af.location  = APPLY_DAMROLL;
  af.modifier  = + (1 * (level/5));
  affect_to_char( victim, &af );
  send_to_char( "You are filled with the spirit of the {Dbe{wa{Dr{w.{x\n\r", victim );
  return;
}
/* ================================ */
void spell_eagle_spirit( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;

  /* dea with the object case first */
  victim = (CHAR_DATA *) vo;

  if (victim != ch)
  {
    send_to_char("You cannot cast this on another.\n\r",ch);
    return;
  }

  if (is_spirit_affected(victim))
  {
    send_to_char("You are already filled with a spirit.\n\r",ch);
    return;
  }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level   = level;
  af.duration  = level;
  af.location  = APPLY_DEX;
  af.modifier  = +(1+ (int)(level/30));
  af.bitvector = 0;
  affect_to_char( victim, &af );

  af.location  = APPLY_MOVE;
  af.modifier  = + (3 * (level/5));
  affect_to_char( victim, &af );

  af.location  = APPLY_HITROLL;
  af.modifier  = + (1 * (level/5));
  affect_to_char( victim, &af );

  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_FLYING;
  affect_to_char( victim, &af );
  send_to_char( "You are filled with the spirit of the {wea{Wg{Dle.{x\n\r", victim );
  return;
}
/* ================================ */
void spell_tiger_spirit( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;

  /* dea with the object case first */
  victim = (CHAR_DATA *) vo;

  if (victim != ch)
  {
    send_to_char("You cannot cast this on another.\n\r",ch);
    return;
  }

  if (is_spirit_affected(victim))
  {
    send_to_char("You are already filled with a spirit.\n\r",ch);
    return;
  }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_STR;
  af.modifier  = +(1+ (int)(level/20));
  af.bitvector = 0;
  affect_to_char( victim, &af );

  af.location  = APPLY_MOVE;
  af.modifier  = + (3 * (level/5));
  affect_to_char( victim, &af );

  af.location  = APPLY_HITROLL;
  af.modifier  = + (1 * (level/15));
  affect_to_char( victim, &af );
  send_to_char( "You are filled with the spirit of the {yt{Di{yg{De{yr{w.{x\n\r", victim );
  return;
}
/* ================================ */
void spell_dragon_spirit( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;

  /* dea with the object case first */
  victim = (CHAR_DATA *) vo;

  if (victim != ch)
  {
    send_to_char("You cannot cast this on another.\n\r",ch);
    return;
  }

  if (is_spirit_affected(victim))
  {
    send_to_char("You are already filled with a spirit.\n\r",ch);
    return;
  }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_WIS;
  af.modifier  = +(1+ (int)(level/20));
  af.bitvector = 0;
  affect_to_char( victim, &af );

  af.location  = APPLY_DAMROLL;
  af.modifier  = + (1 * (level/5));
  affect_to_char( victim, &af );

  af.location  = APPLY_HITROLL;
  af.modifier  = + (1 * (level/5));
  affect_to_char( victim, &af );
  send_to_char( "You are filled with the spirit of the {gdrag{con{w.{x\n\r", victim );
  return;
}


/* ================================ */
void spell_scry( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA  *vch;
  char command[MSL];
  if ( ( vch = get_char_world( ch, target_name ) ) == NULL
       ||   vch == ch
       ||   vch->in_room == NULL
       || (  !can_see_room( ch, vch->in_room ) && number_percent( ) > 5 )
       ||   vch->level >= level + 3
       || (  !IS_NPC( vch ) && vch->level >= LEVEL_IMMORTAL )
       ||   saves_spell( level - number_range( 0, 5 ), vch, DAM_OTHER ) )

  {
    send_to_char( "You feel yourself weakening and cannot gain a clear view.\n\r", ch );
    if ( vch && IS_AWAKE( vch ) )
      act( "You sense $n viewing you from afar.", ch, NULL, vch, TO_VICT );
    return;
  }

  if ( is_affected( vch, skill_lookup( "mind shield" ) ) )
  {
    if (get_curr_stat(ch, STAT_WIS) < number_percent())
    {
      act( "You sense $n trying to see through your eyes.", ch, NULL, vch, TO_VICT );
      send_to_char("You feel a strong blocking force preventing you from seeing.\n\r",ch);
      return;
    }
  }


  act( "$n flickers briefly.", ch, NULL, NULL, TO_ROOM );

  if ( get_curr_stat( ch, STAT_INT ) < get_curr_stat( vch, STAT_WIS )
       &&   IS_AWAKE( vch ) )
    act( "You sense $n looking over your shoulder.", ch, NULL, vch, TO_VICT );

  printf_to_char(ch,"{YArea:{C %s{x\n\r",vch->in_room->area->name);
  mprintf( sizeof(command), command, "%d look", vch->in_room->vnum );

  do_function(ch, &do_at, command);
  return;
}
/* ================================ */
void spell_cannibalism( int sn, int level, CHAR_DATA *ch, void *vo,
                        int target )
{
  AFFECT_DATA af;
  CHAR_DATA  *vch = (CHAR_DATA *) vo;
  int   transfer = level * 2 + number_range( -level / 2, level / 2 );

  vch->mana  = UMIN( vch->mana + transfer, GET_MANA(vch) );
  ch->hit  = UMAX( ch->hit - transfer, -10 );

  af.where  = TO_AFFECTS;
  af.type   = sn;
  af.level   = level;
  af.duration  = 1;
  af.location  = APPLY_STR;
  af.modifier  = number_percent() < 33 ? -1 : 0;
  af.bitvector = 0;
  affect_join( ch, &af );

  ch->pcdata->condition[COND_FULL] += 4;

  if ( ch->hit > 0 )
  {
    if ( ch == vch )
      send_to_char(
        "Your {rl{Ri{rfe{x force is drained to support your {mmag{Mi{mc{x.\n\r", ch );
    else
    {
      act( "Your {rl{Ri{rfe{x force is drained to support $N's {mmag{Mi{mc{x.",
           ch, NULL, vch, TO_CHAR );
      act( "$n's drains $s {rl{Ri{rfe{x force to support your {mmag{Mi{mc{x.",
           ch, NULL, vch, TO_VICT );
    }
  }
  else
  {
    send_to_char(
      "You feel the last wisps of your {rl{Ri{rfe{x slip away.\n\r"
      "Maybe that wasn't such a bright idea...\n\r", ch );

    if ( ch != vch )
    {
      act( "$n drains the last of $s {rl{Ri{rfe{x force to support your {mmag{Mi{mc{x.",
           ch, NULL, vch, TO_VICT );
    }
  }
  update_pos( ch );
}

/* ================================ */
void spell_clairvoyance( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  AFFECT_DATA af;

  if (is_affected(ch, sn))
  {
    send_to_char("You are already clairvoyant.\n\r",ch);
    return;
  }

  if (is_affected( ch, skill_lookup("farsight") )
      &&  is_affected( ch, skill_lookup("detect invis") )
      &&  is_affected( ch, skill_lookup("detect hidden") ) )
  {
    send_to_char("You can already see everything!\n\r",ch);
    return;
  }


  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/2;
  af.location  = APPLY_NONE;
  af.modifier  = 0;

  if (  !IS_AFFECTED(ch, AFF_DETECT_HIDDEN) )
  {
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( ch, &af );
    send_to_char( "Your awareness improves.\n\r", ch );
  }
  if ( !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
  {
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( ch, &af );
    send_to_char( "Your eyes tingle.\n\r", ch );
  }

  if (!IS_SET(ch->spell_aff, SAFF_FARSIGHT))
  {
    af.where     = TO_SPELL_AFFECTS;
    af.bitvector = SAFF_FARSIGHT;
    affect_to_char( ch, &af );
    send_to_char("You have become one with the farsight.\n\r",ch);
  }
}

/* ================================ */
void spell_fatigue( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice(level,1);
  if ( saves_spell( level, victim, DAM_MENTAL ) )
    dam /= 2;
  victim->move -= dam;
  if (victim->move < 0)
    victim->move = 0;
  act("You mentally make $N tired.",ch,NULL,victim,TO_CHAR);
  act("$n mentally struggles with you and you feel more tired.",ch,NULL,victim,TO_VICT);
  act("$n mentally makes $N tired.",ch,NULL,victim,TO_NOTVICT);
  if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
  return;
}

/* ================================ */
void spell_mass_summon( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim;
  CHAR_DATA *ch_next, *fch;
  int count=0;

  for ( fch = char_list; fch != NULL; fch = ch_next )
  {
    ch_next  = fch->next;
    if (can_see(ch,fch) && is_name(target_name,fch->name))
      victim = fch;
    else
      continue;
    if (      victim == ch
              ||   victim->in_room == NULL
              ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
              ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
              ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
              ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
              ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
              ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
              ||   victim->level >= level + 3
              ||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)
              ||   victim->fighting != NULL
              ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
              ||   (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
              ||   (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON))
              ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER)))
      continue;

    if (ch->mana >= 50)
      ch->mana -= 50;
    else
      continue;
    act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
    move_to_room( victim, ch->in_room );
    act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
    act( "$n has summoned you!", ch, NULL, victim,   TO_VICT );
    do_function(victim, &do_look, "auto" );
    count++;
  }
  if (count)
    printf_to_char(ch,"You have summoned %d of your target.\n\r",count);

  return;
}
/* ================================ */

void spell_replenish(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *gch;
  AFFECT_DATA af;

  if (IS_SET(ch->spell_aff, SAFF_REPLENISH))
  {
    send_to_char("You cannot call for divine aid so often.\n\r",ch);
    return;
  }
  gch = ch->in_room->people;
  for ( ; gch != NULL; gch = gch->next_in_room)
  {
    if ( IS_WIZINVIS(gch,ch) || gch == ch )
      continue;
    if ((IS_NPC(ch) && IS_NPC(gch)) ||
        (!IS_NPC(ch) && !IS_NPC(gch)))
    {
      gch->hit = GET_HP(gch);
      gch->mana = GET_MANA(gch);
      gch->move = gch->max_move;
      act("You feel completely replenished by $n's {Wdiv{Yi{yne {Wind{Yu{ylgence{x.",ch,NULL,gch,TO_VICT);
    }
  }

  af.where     = TO_SPELL_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 45-level/5;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = SAFF_REPLENISH;
  affect_to_char( ch, &af );
  send_to_char("Your {Wdiv{yine {Wpo{Yw{yer{w spreads to the whole room.\n\r",ch);
  return;
}

/* ================================ */
void spell_avalanche( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice( level, 18 );

  if ( !IS_OUTSIDE(ch) )
    dam /= 2;

  if ( saves_spell( level, victim, DAM_COLD ) )
    dam /= 2;

  act("$n is hit with an {Wav{Ba{cl{Wanc{ch{be{x of {Wsnow{x and {cice{x.{x", victim, NULL, NULL, TO_ROOM );
  damage( ch, victim, dam, sn, DAM_COLD, TRUE, TRUE);
  return;
}


/* ================================ */
void spell_clearhead( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA  *vch = (CHAR_DATA *)vo;
  AFFECT_DATA af;

  if ( is_affected( vch, sn ) )
  {
    if (vch == ch)
      send_to_char("You are already in a state of enlightenment.\n\r",ch);
    else
      act("$N is already clearheaded.",ch,NULL,vch,TO_CHAR);
    return;
  }

  af.where   = TO_AFFECTS;
  af.type   = sn;
  af.level   = level;
  af.duration  = 1 + level / 9;
  af.location  = APPLY_INT;
  af.modifier  = +1;
  af.bitvector = 0;
  affect_to_char( vch, &af );
  act( "$n radiates an aura of {mmag{bi{mc{ba{ml{x competence.{x", vch, NULL, NULL, TO_ROOM );
  send_to_char( "You radiate an aura of {mmag{bi{mc{ba{ml{x competence.\n\r", vch );
  return;
}
/* ================================ */

void spell_darksight( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA  *victim = (CHAR_DATA *)vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED( victim, AFF_DARK_VISION ) )
  {
    if ( victim == ch )
      send_to_char( "You can already see in the dark.\n\r", ch );
    else
      act( "$N already has darksight.", ch, NULL, victim, TO_CHAR );
    return;
  }

  af.where   = TO_AFFECTS;
  af.type   = sn;
  af.level   = level;
  af.duration  = level;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_DARK_VISION;
  affect_to_char( victim, &af );

  act( "$n's eyes glow {ggreen{x.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You can now see in the dark.\n\r", victim );
}
/* ================================ */

void spell_mageshield( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( ch == victim )
  {
    if ( IS_SAFFECTED( victim, SAFF_MAGESHIELD )
         ||   IS_SAFFECTED( victim, SAFF_INVUN )
         ||   IS_AFFECTED2( victim, AFF2_INVUN )
         ||   IS_AFFECTED(  victim, AFF_SANCTUARY )
         ||   IS_AFFECTED(  victim, AFF_AQUA_ALBEDO )
         ||   IS_AFFECTED2( victim, AFF2_FADE_OUT )
         ||   IS_AFFECTED2( victim, AFF2_SHROUD )
         ||   IS_AFFECTED2( victim, AFF2_RADIANT )
         ||   IS_AFFECTED2( victim, AFF2_NIRVANA )
         ||   IS_SAFFECTED( victim, SAFF_IRONWILL )
         ||   IS_SAFFECTED( victim, SAFF_WARCRY_GUARDING ) )
    {
      send_to_char( "You already have a mageshield.\n\r", ch );
      return;
    }
  }
  else
  {
    if (is_sanc_spelled(victim))
    {
      act("$N is already surrounded by a mageshield.",ch,NULL,victim,TO_CHAR);
      return;
    }
  }

  af.where     = TO_SPELL_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 6;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = SAFF_MAGESHIELD;
  affect_to_char( victim, &af );
  act( "$n is surrounded by an invoked shield of {bm{Bag{bic{x.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You are surrounded by an invoked shield of {bm{Bag{bic{x.\n\r", victim );
  return;
}
/* ================================ */
void spell_warriorshield( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( ch == victim )
  {
    if ( IS_SAFFECTED( victim, SAFF_WARRIORSHIELD )
         ||   IS_SAFFECTED( victim, SAFF_INVUN )
         ||   IS_AFFECTED2( victim, AFF2_INVUN )
         ||   IS_AFFECTED( victim, AFF_SANCTUARY )
         ||   IS_AFFECTED( victim, AFF_AQUA_ALBEDO )
         ||   IS_AFFECTED2( victim, AFF2_FADE_OUT )
         ||   IS_AFFECTED2( victim, AFF2_SHROUD )
         ||   IS_AFFECTED2( victim, AFF2_RADIANT )
         ||   IS_AFFECTED2( victim, AFF2_NIRVANA )
         ||   IS_SAFFECTED( victim, SAFF_IRONWILL )
         ||   IS_SAFFECTED( victim, SAFF_WARCRY_GUARDING ) )
    {
      send_to_char( "You already have a warriorshield.\n\r", ch );
      return;
    }
  }
  else
  {
    if ( is_sanc_spelled(victim))
    {
      act("$N is already surrounded by a warriorshield.",ch,NULL,victim,TO_CHAR);
      return;
    }
  }

  af.where     = TO_SPELL_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 6;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = SAFF_WARRIORSHIELD;
  affect_to_char( victim, &af );
  act( "$n is engulfed within a hazy {yba{Yttl{ye{x shield.{x", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You are engulfed within a hazy {yba{Yttl{ye{x shield.\n\r", victim );
  return;
}
/* ================================ */
void spell_bark_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn) )
  {
    if (victim == ch)
      send_to_char("Your skin is already as durable as treebark.\n\r",ch);
    else
      act("$N is already as durable as can be.",ch,NULL,victim,TO_CHAR);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/2 + 5;
  af.location  = APPLY_AC;
  af.modifier  = -100;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  act( "$n's skin takes on the look of {gtree{ybark{x.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "Your skin takes on the durability of {gtree{ybark{x.\n\r", victim );
  return;
}

void spell_hiccup( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) || IS_SET(victim->spell_aff,SAFF_HICCUP))
  {
    if (victim == ch)
      send_to_char("You've already been hiccuped.\n\r",ch);
    else
      act("$N has already had been hiccuped.",ch,NULL,victim,TO_CHAR);
    return;
  }
  if (saves_spell(level ,victim,DAM_OTHER))
    return;

  af.where     = TO_SPELL_AFFECTS;
  af.type      = gsn_hiccup;
  af.duration  = 2 * level;
  af.level     = level;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = SAFF_HICCUP;
  affect_to_char( victim, &af );
  send_to_char( "You cough a second.\n\r", victim );
  act("$n coughs for a second....",victim,NULL,NULL,TO_ROOM);
  return;
}

void spell_yawn( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) || IS_SET(victim->spell_aff,SAFF_YAWN))
  {
    if (victim == ch)
      send_to_char("You've already feel tired.\n\r",ch);
    else
      act("$N is already feeling tired.",ch,NULL,victim,TO_CHAR);
    return;
  }
  if (saves_spell(level ,victim,DAM_OTHER))
    return;

  af.where     = TO_SPELL_AFFECTS;
  af.type      = gsn_yawn;
  af.duration  = 2 * level;
  af.level     = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = SAFF_YAWN;
  affect_to_char( victim, &af );
  send_to_char( "You feel tired.\n\r", victim );
  act("$n seems to be tired...",victim,NULL,NULL,TO_ROOM);
  return;
}


bool is_sanc_spelled(CHAR_DATA *vch)
{
  return( IS_SET(vch->spell_aff, SAFF_INVUN) ||
          IS_AFFECTED2(vch, AFF2_INVUN)      ||
          IS_AFFECTED(vch, AFF_SANCTUARY)    ||
          IS_AFFECTED( vch, AFF_AQUA_ALBEDO ) ||
          IS_AFFECTED2(vch, AFF2_FADE_OUT)   ||
          IS_AFFECTED2(vch, AFF2_SHROUD)     ||
          IS_AFFECTED2(vch, AFF2_RADIANT)    ||
          IS_AFFECTED2(vch, AFF2_NIRVANA)     ||
          IS_SET(vch->spell_aff, SAFF_MAGESHIELD) ||
          IS_SET(vch->spell_aff, SAFF_IRONWILL) ||
          IS_SET(vch->spell_aff, SAFF_WARCRY_GUARDING ) ||
          IS_SET(vch->spell_aff, SAFF_WARRIORSHIELD));
}
bool is_aura_spelled(CHAR_DATA *vch)
{
  if (is_affected(vch,skill_lookup("flame aura"))
      || is_affected(vch,skill_lookup("frost aura"))
      || is_affected(vch,skill_lookup("electric aura"))
      || is_affected(vch,skill_lookup("corrosive aura"))
      || is_affected(vch,skill_lookup("holy aura"))
      || is_affected(vch,skill_lookup("dark aura"))
      || is_affected(vch,skill_lookup("poison aura"))
      || is_affected(vch,skill_lookup("arcane aura")))
    return TRUE;
  return FALSE;
}
/* ================================ */
void spell_air_element( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;
  /* dea with the object case first */
  victim = (CHAR_DATA *) vo;

  if (victim != ch)
  {
    send_to_char("You cannot cast this on another.\n\r",ch);
    return;
  }

  if (is_element_affected(victim))
  {
    send_to_char("You are already one with the elements.\n\r",ch);
    return;
  }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level   = level;
  af.duration  = level;
  af.location  = APPLY_DEX;
  af.modifier  = +(1+ (int)(level/15));
  af.bitvector = 0;
  affect_to_char( victim, &af );

  af.location  = APPLY_HITROLL;
  af.modifier  = + (3 + (level/10));
  affect_to_char( victim, &af );
  send_to_char( "You have become one with the element of {Wai{cr{x.\n\r", victim );
  return;
}

void spell_earth_element( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;
  /* dea with the object case first */
  victim = (CHAR_DATA *) vo;

  if (victim != ch)
  {
    send_to_char("You cannot cast this on another.\n\r",ch);
    return;
  }

  if (is_element_affected(victim))
  {
    send_to_char("You are already one with the elements.\n\r",ch);
    return;
  }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_AC;
  af.modifier  = -50;
  af.bitvector = 0;
  affect_to_char( victim, &af );

  af.location  = APPLY_HIT;
  af.modifier  = + (4 * (level/10));
  affect_to_char( victim, &af );
  send_to_char( "You have become one with the element of {Dear{yth{x.\n\r", victim );
  return;
}
void spell_fire_element( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;
  /* dea with the object case first */
  victim = (CHAR_DATA *) vo;

  if (victim != ch)
  {
    send_to_char("You cannot cast this on another.\n\r",ch);
    return;
  }

  if (is_element_affected(victim))
  {
    send_to_char("You are already one with the elements.\n\r",ch);
    return;
  }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_STR;
  af.modifier  = + ((int)level/20);
  af.bitvector = 0;
  affect_to_char( victim, &af );

  af.location  = APPLY_DAMROLL;
  af.modifier  = + (3 + (level/10));
  affect_to_char( victim, &af );
  send_to_char( "You have become one with the element of {Rfi{yre{x.\n\r", victim );
  return;
}

void spell_water_element( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;
  /* dea with the object case first */
  victim = (CHAR_DATA *) vo;

  if (victim != ch)
  {
    send_to_char("You cannot cast this on another.\n\r",ch);
    return;
  }

  if (is_element_affected(victim))
  {
    send_to_char("You are already one with the elements.\n\r",ch);
    return;
  }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_SAVES;
  af.modifier  = - (3 + (int)level/20);
  af.bitvector = 0;
  affect_to_char( victim, &af );

  af.location  = APPLY_CON;
  af.modifier  = + (level/10);
  affect_to_char( victim, &af );
  send_to_char( "You have become one with the element of {Bwa{cter{x.\n\r", victim );
  return;
}

void spell_banish( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{

  return;
}

bool is_element_affected(CHAR_DATA *vch)
{
  if (is_affected(vch,skill_lookup("air element"))
      || is_affected(vch,skill_lookup("fire element"))
      || is_affected(vch,skill_lookup("earth element"))
      || is_affected(vch,skill_lookup("water element")))
    return TRUE;
  return FALSE;
}
bool is_spirit_affected(CHAR_DATA *vch)
{
  if (is_affected(vch,skill_lookup("bear spirit"))
      || is_affected(vch,skill_lookup("eagle spirit"))
      || is_affected(vch,skill_lookup("tiger spirit"))
      || is_affected(vch,skill_lookup("dragon spirit")))
    return TRUE;
  return FALSE;
}

void spell_create_staff( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  OBJ_DATA *staff;
  char     buf[MAX_STRING_LENGTH];

  staff = create_object( get_obj_index( OBJ_VNUM_STAFF ), 0);
  mprintf(sizeof(buf), buf, "$n has created a staff.");
  act_new( buf, ch, staff, NULL, TO_ROOM, POS_RESTING );
  printf_to_char(ch, "You create the staff of Druid power.\n\r");
  free_string(staff->name);
  staff->name = str_dup("staff power druid", staff->name);
  free_string(staff->short_descr);
  staff->short_descr = str_dup("a staff of Druid power", staff->short_descr);
  free_string(staff->description);
  staff->description = str_dup("a staff of Druid power",staff->description);
  staff->cost = 0;

  switch ( number_range(0,5) )
  {
    case 0:
      staff->value[1] = 1;
      staff->value[2] = number_range( ((level-3)*2), ((level+2)*2));
      break;
    case 1:
      staff->value[1] = 2;
      staff->value[2] = number_range( (level-3), (level+2) );
      break;
    case 2:
      staff->value[1] = 3;
      staff->value[2] = number_range( ((level-3)*2)/3, ((level+2)*2)/3 );
      break;
    case 3:
      staff->value[1] = 4;
      staff->value[2] = number_range( ((level-3)/2), ((level+1)/2));
      break;
    case 4:
      staff->value[1] = 6;
      staff->value[2] = number_range( ((level-3)/3), ((level+1)/3));
      break;
    case 5:
      staff->value[1] = 10;
      staff->value[2] = number_range( ((level-3)/5), (level/5));
      break;
    default:
      staff->value[1] = 1;
      staff->value[2] = level*2;
      break;
  }

  if (ch->alignment <=0 )
    staff->value[4] = WEAPON_VAMPIRIC;
  else
    staff->value[4] = WEAPON_MANA_DRAIN;

  if (level >= 30)
    staff->value[4] = staff->value[4] | WEAPON_SHOCKING;
  if (level >= 60)
    staff->value[4] = staff->value[4] | WEAPON_VORPAL;
  if (level >= 90)
    staff->value[4] = staff->value[4] | WEAPON_FLAMING;

  staff->timer = level;
  staff->level = ch->level;
  obj_to_char( staff, ch );
}

/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_devour_soul( int sn, int level, CHAR_DATA *ch, void *vo,int
                        target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if (victim != ch)
  {
    //align_change(ch, UMAX(-1000, ch->alignment - 20), FALSE, TRUE);
    //align_change(victim,UMAX(-1000, victim->alignment - 100), FALSE, TRUE);
  }
  dam = dice(level, 30);

  if ( victim->level <= 2 )
  {
    dam = ch->hit + 1;
  }
  else
  {
    victim->mana /= 3;
    victim->move /= 3;
  }

  ch->hit /= 2;

  if ( saves_spell( level, victim,DAM_NEGATIVE) )
  {
    send_to_char("You feel a sharp pain in your chest.\n\r",victim);
    damage(ch, victim,dam , sn, DAM_NEGATIVE, TRUE, TRUE);
    if ((victim->in_room == ch->in_room) && (victim->position != POS_DEAD))
      damage(ch, victim,dam , sn, DAM_NEGATIVE, TRUE, TRUE);
    return;
  }


  send_to_char("Your soul is wrenched from your chest.\n\r",victim);
  send_to_char("You wrench their soul from their body and snap it back causing tremendous damage but draining you as well.\n\r",ch);
  damage(ch, victim,dam , sn, DAM_NEGATIVE, TRUE, TRUE);

  if ((victim->in_room == ch->in_room) && (victim->position != POS_DEAD))
    damage(ch, victim,dam , sn, DAM_NEGATIVE, TRUE, TRUE);
  if ((victim->in_room == ch->in_room) && (victim->position != POS_DEAD))
    damage(ch, victim,dam , sn, DAM_NEGATIVE, TRUE, TRUE);
  if ((victim->in_room == ch->in_room) && (victim->position != POS_DEAD))
    damage(ch, victim,dam , sn, DAM_NEGATIVE, TRUE, TRUE);

  return;
}
/* ================================ */
void spell_nirvana( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( !IS_OUTSIDE(ch) )
  {
    send_to_char( "You must be out of doors.\n\r", ch );
    return;
  }

  if (victim != ch)
  {
    send_to_char("You cannot cast this on another\n\r",ch);
    return;
  }

  if (is_sanc_spelled(ch))
  {
    send_to_char("You are already in some sort of nirvana.\n\r",ch);
    return;
  }

  af.where     = TO_AFFECTS2;
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 3;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF2_NIRVANA;
  affect_to_char( victim, &af );
  act( "$n has become in {gnirv{cana{x with nature.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You are in {gnirv{cana{x with nature.{x\n\r", victim );
  return;
}

/* ====== Taristar's Aura Read ====== */
void spell_aura_read( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  AFFECT_DATA *paf, *paf_last = NULL;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  char buf[MAX_STRING_LENGTH];

  if ( victim->affected != NULL )
  {
    if ( IS_NPC(victim) )
      printf_to_char( ch, "%s is affected by the following spells:\n\r", victim->short_descr);
    else
    {
      if (victim == ch)
        send_to_char( "You are affected by the following spells:\n\r", ch);
      else
        printf_to_char( ch, "%s is affected by the following spells:\n\r", victim->name );
    }

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
      if (paf_last != NULL && paf->type == paf_last->type)
      {
        if (ch->level >= LEVEL_REVEAL_AC)
          mprintf(sizeof(buf), buf, "                      ");
        else
          continue;
      }
      else
        mprintf(sizeof(buf), buf, "Spell: {C%-15s{x", skill_table[paf->type].name );

      send_to_char( buf, ch );

      if ( ch->level >= LEVEL_REVEAL_AC )
      {
        printf_to_char(ch, ": modifies %s by %d ",
                       affect_loc_name( paf->location ),  paf->modifier);

        if ( paf->duration == -1 )
          printf_to_char(ch, "permanently" );
        else
          printf_to_char(ch, "for {W%d{w hours", paf->duration );
      }

      send_to_char( "\n\r", ch );
      paf_last = paf;
    }
  }
  else
  {
    if ( IS_NPC(victim) )
      printf_to_char( ch, "%s is not affected by any spells.\n\r", victim->short_descr);
    else
    {
      if (victim == ch)
        send_to_char( "You are not affected by any spells.\n\r", ch);
      else
        printf_to_char( ch, "%s is not affected by any spells.\n\r", victim->name);
    }
  }

  return;
}

void spell_heal_group( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *gch;
  int heal;

  heal = level*4 + dice(2,8);
  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
  {
    if ( IS_WIZINVIS(gch,ch))
      continue;

    if (is_same_group(gch,ch))
    {
      if (gch->hit < GET_HP(gch))
      {
        gch->hit = UMIN( gch->hit + heal, GET_HP(gch) );
        update_pos( gch );
        send_to_char( "You feel better!\n\r", gch );

        if ( ch != gch )
          act("You apply healing to $N's wounds.",ch, NULL, gch, TO_CHAR);
        else
          send_to_char("You apply healing to your wounds.\n\r",ch);
      }
      else
			  send_to_char( "You are already healed.\n\r", gch);
    }
  }
  return;
}
void spell_protection_neutral(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL)
       ||   IS_AFFECTED(victim, AFF_PROTECT_GOOD)
       ||   IS_SAFFECTED(victim, SAFF_PROTECT_NEUTRAL)
       ||   IS_SAFFECTED(victim, SAFF_PROTECT_HOLY)
       ||   IS_SAFFECTED(victim, SAFF_PROTECT_NEGATIVE))
  {
    if (victim == ch)
      send_to_char("You are already protected.\n\r",ch);
    else
      act("$N is already protected.",ch,NULL,victim,TO_CHAR);
    return;
  }

  af.where     = TO_SPELL_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 24;
  af.location  = APPLY_SAVING_SPELL;
  af.modifier  = -1;
  af.bitvector = SAFF_PROTECT_NEUTRAL;
  affect_to_char( victim, &af );
  send_to_char( "You feel extreme.\n\r", victim );
  if ( ch != victim )
    act("$N is protected from waffling nuetral things.",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_mass_bless( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  CHAR_DATA *gch;

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
  {
    if ( !is_same_group( gch, ch )
         || !can_see(ch, gch)
         || IS_WIZINVIS(gch,ch))
      continue;

    if (is_affected( gch, sn )
        || is_affected( gch, skill_lookup("bless")))
    {
      if (ch != gch)
        act("$N already has divine favor.",ch,NULL,gch,TO_CHAR);
      continue;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level   = level;
    af.duration  = level;
    af.location  = APPLY_HITROLL;
    af.modifier  = level / 8;
    af.bitvector = 0;
    affect_to_char( gch, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 0 - level / 8;
    affect_to_char( gch, &af );
    send_to_char( "You feel righteous.\n\r", gch );
    if (IS_IMMORTAL(ch) && (ch != gch))
      act("You grant your favor to $N.",ch,NULL,gch,TO_CHAR);
    else if ( ch != gch )
      act("You grant $N the favor of $g.",ch,NULL,gch,TO_CHAR);
  }
  send_to_char( "Ok.\n\r", ch );

  return;
}

void spell_mass_vaccine( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  CHAR_DATA *gch;

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
  {
    if ( !is_same_group( gch, ch )
         || !can_see(ch, gch)
         || IS_WIZINVIS(gch,ch))
      continue;

    if (is_affected( gch, sn )
        || (IS_SET(gch->res_flags,IMM_DISEASE))
        || is_affected( gch, skill_lookup("vaccine")))
    {
      if (ch != gch)
        act("$N is already vaccinated.",ch,NULL,gch,TO_CHAR);
      continue;
    }

    af.where     = TO_RESIST;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = IMM_DISEASE;
    affect_to_char( gch, &af );
    send_to_char( "You feel vaccinated from disease.\n\r", gch );
    if ( ch != gch )
      act("$N is vaccinated from disease.",ch,NULL,gch,TO_CHAR);
  }
  send_to_char( "Ok.\n\r", ch );

  return;
}

void spell_full_regen( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  restore_char(victim);
  send_to_char( "A {RH{YO{RT{x flash fills your body.\n\r", victim );
  return;
}

#ifdef INQUISITOR
void spell_divine_intervention( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  int dam;

  victim = (CHAR_DATA *)vo;

  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
  {
    send_to_char("You are not righteous enough to be granted your holy powers.\n\r", ch);
    return;
  }

  if (!IS_SET(victim->act, ACT_UNDEAD) || IS_GOOD(victim))
  {
    send_to_char("Your holy attack has almost no affect.\n\r", ch);
    damage(ch, victim, 1, sn, DAM_HOLY, TRUE, TRUE);
    return;
  }

  act("Divine energy courses through $N.", ch, NULL, victim, TO_NOTVICT);
  act("Divine energy courses through $N.", ch, NULL, victim, TO_CHAR);
  act("Divine energy courses through you.", ch, NULL, victim, TO_VICT);
  dam = magic_dam(sn, level);
  dam /= 2;
  if (saves_spell(level, victim, DAM_HOLY))
    dam /= 2;
  damage(ch, victim, dam, sn, DAM_HOLY, TRUE, TRUE);
  return;
}

void spell_demonic_intervention( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  int dam;

  victim = (CHAR_DATA *)vo;

  if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
  {
    send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
    return;
  }

  if (!IS_SET(victim->act, ACT_UNDEAD) || IS_EVIL(victim))
  {
    send_to_char("Your unholy attack has almost no affect.\n\r", ch);
    damage(ch, victim, 1, sn, DAM_NEGATIVE, TRUE, TRUE);
    return;
  }

  act("Demonic energy courses through $N.", ch, NULL, victim, TO_NOTVICT);
  act("Demonic energy courses through $N.", ch, NULL, victim, TO_CHAR);
  act("Demonic energy courses through you.", ch, NULL, victim, TO_VICT);
  dam = magic_dam(sn, level);
  dam /= 2;
  if (saves_spell(level, victim, DAM_NEGATIVE))
    dam /= 2;
  damage(ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE);
  return;
}

void spell_force_of_faith_darkflow(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  int dam;

  victim = (CHAR_DATA *)vo;

  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
  {
    send_to_char("You are not righteous enough to be granted your holy powers.\n\r", ch);
    return;
  }
  else if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
  {
    send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
    return;
  }

  if (IS_CLASS_INQUISITOR(ch) && IS_GOOD(victim))
  {
    send_to_char("Your force of faith has little effect on noble souls.\n\r", ch);
    ch->alignment -= 0; //Changed cause we have no align change.
    damage(ch, victim, 1, sn, DAM_HOLY, TRUE, TRUE);
    return;
  }
  else if (IS_CLASS_OCCULTIST(ch) && IS_EVIL(victim))
  {
    send_to_char("Your darkflow has little effect on treacherous souls.\n\r", ch);
    ch->alignment += 0; //Changed cause we have no align change.
    damage(ch, victim, 1, sn, DAM_NEGATIVE, TRUE, TRUE);
    return;
  }

  dam = magic_dam(sn, level) / 2;

  if (IS_NEUTRAL(victim))
    dam = dam * 3 / 4;

  if (IS_CLASS_INQUISITOR(ch))
  {
    act("You unleash a {ww{Wa{wve{x of {yr{Wi{Yg{yhte{Wo{yus{x energy towards $N.", ch, NULL, victim, TO_CHAR);
    act("$n unleashes a {ww{Wa{wve{x of {yr{Wi{Yg{yhte{Wo{yus{x energy towards you!", ch, NULL, victim, TO_VICT);
    act("$n unleashes a {ww{Wa{wve{x of {yr{Wi{Yg{yhte{Wo{yus{x energy towards $N.", ch, NULL, victim, TO_NOTVICT);
  }
  else
  {
    act("A {Ds{rt{Dream{x of {rda{Dr{rk{Dn{ress{x snakes from your hand and envelops $N.", ch, NULL, victim, TO_CHAR);
    act("A {Ds{rt{Dream{x of {rda{Dr{rk{Dn{ress{x snakes from $n's hand and envelops you!", ch, NULL, victim, TO_VICT);
    act("A {Ds{rt{Dream{x of {rda{Dr{rk{Dn{ress{x snakes from $n's hand and envelops $N.", ch, NULL, victim, TO_NOTVICT);
  }

  damage(ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE);
}

void spell_noble_truth( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  AFFECT_DATA af;
  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
  {
    send_to_char("You are not righteous enough to be granted your holy powers.\n\r", ch);
    return;
  }

  if (IS_SAFFECTED(ch, SAFF_MANA_DRAIN))
  {
    send_to_char("You already possess a noble spirit.\n\r", ch);
    return;
  }

  af.where    = TO_SPELL_AFFECTS;
  af.type     = sn;
  af.duration  = level / 5;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.level     = level;
  af.bitvector = SAFF_MANA_DRAIN;
  affect_to_char( ch, &af );

  send_to_char("You are filled with a {Yno{yb{Wl{we{x sense of truth.\n\r", ch);
  act( "$n is filled with a {Yno{yb{Wl{we{x sense of truth.", ch, NULL, NULL, TO_ROOM);
  return;
}

void spell_vile_intent( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  AFFECT_DATA af;
  if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
  {
    send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
    return;
  }

  if (IS_SAFFECTED(ch, SAFF_LIFE_DRAIN))
  {
    send_to_char("You already possess a vile intent.\n\r", ch);
    return;
  }

  af.where    = TO_SPELL_AFFECTS;
  af.type     = sn;
  af.duration  = level / 5;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.level     = level;
  af.bitvector = SAFF_LIFE_DRAIN;
  affect_to_char( ch, &af );

  send_to_char("You are filled with a {rv{Ri{Dl{re{x intent.\n\r", ch);
  act( "$n is filled with a {rv{Ri{Dl{re{x intent.", ch, NULL, NULL, TO_ROOM);
  return;
}


#endif

void spell_magma_burst( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  act("A stream of {Rmo{Ylt{Ren ma{Ygm{Ra {xshoots from your outstretched palms.",ch, NULL, NULL, TO_CHAR);
  act("A stream of {Rmo{Ylt{Ren ma{Ygm{Ra {xshoots from $n to $N.",ch, NULL, victim, TO_NOTVICT);
  act("A stream of {Rmo{Ylt{Ren ma{Ygm{Ra {xshoots from $n into you.",ch, NULL, victim, TO_VICT);
  dam = dice( level, 16 );
  if (IS_SET(ch->in_room->room_flags, ROOM_UNDER_WATER))
  {
    send_to_char("Your magma is cooled by the water.\n\r", ch);
    dam /= 3;
  }
  if ( saves_spell( level, victim, DAM_FIRE ) )
    dam /= 3;
  damage( ch, victim, dam, sn,DAM_FIRE,TRUE, TRUE);
  return;
}

void spell_lava_burst( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  act("A stream of {Rmo{Ylt{Ren L{Ya{Rv{Ya{x shoots from your outstretched palms.",ch, NULL, NULL, TO_CHAR);
  act("A stream of {Rmo{Ylt{Ren L{Ya{Rv{Ya{x shoots from $n to $N.",ch, NULL, victim, TO_NOTVICT);
  act("A stream of {Rmo{Ylt{Ren L{Ya{Rv{Ya{x shoots from $n into you.",ch, NULL, victim, TO_VICT);

  dam = dice( level, 20 );
  if (IS_SET(ch->in_room->room_flags, ROOM_UNDER_WATER))
  {
    send_to_char("Your lava is cooled by the water.\n\r", ch);
    dam /= 3;
  }
  if ( saves_spell( level, victim, DAM_FIRE ) )
    dam /= 5;
  damage( ch, victim, dam, sn,DAM_FIRE,TRUE, TRUE);
  return;
}

void spell_windtomb( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
  {
    if (victim == ch)
      send_to_char("You have that spell on you already.\n\r",ch);
    else
      act("$N is already within a windtomb.",ch,NULL,victim,TO_CHAR);
    return;
  }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.modifier  = -40 - (level *2);
  af.location  = APPLY_AC;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  send_to_char( "You are enveloped within a roaring hurricane.\n\r", victim );
  act("A roaring wind envelops $n.",victim,NULL,NULL,TO_ROOM);
  return;
}

void spell_star_storm( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  send_to_char( "Hundreds of fiery stars burst from the heavens.\n\r", ch );
  act( "$n calls down a swarm of fiery comets from the sky.", ch, NULL, NULL, TO_ROOM );

  for ( vch = char_list; vch; vch = vch_next )
  {
    vch_next        = vch->next;
    if ( vch->in_room == NULL )
      continue;
    if ( vch->in_room == ch->in_room )
    {
      if ( vch != ch )
        damage( ch,vch,level + dice(level, 50), sn, DAM_FIRE,TRUE, TRUE);
      continue;
    }

    if ( vch->in_room->area == ch->in_room->area )
      send_to_char( "You hear a huge explosion in the distance.\n\r", vch );
  }
  return;
}

void spell_jump( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_FLYING) )
  {
    if (victim == ch)
      send_to_char("You are already airborne.\n\r",ch);
    else
      act("$N doesn't need your help to Jump.",ch,NULL,victim,TO_CHAR);
    return;
  }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_FLYING;
  affect_to_char( victim, &af );
  send_to_char( "You jump up into the air.\n\r", victim );
  act( "$n jumps high into the air.", victim, NULL, NULL, TO_ROOM );
  victim->position = POS_STANDING;
  return;
}
void spell_smoke_screen (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  if (is_affected (victim, sn))
  {
    if (victim == ch)
    {
      send_to_char ("You are already in a shroud of smoke.\n\r", ch);
    }
    else
    {
      act ("$N is already protected.", ch, NULL, victim, TO_CHAR);
    }
    return;
  }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 8 + level;
  af.location = APPLY_AC;
  af.modifier = -80;
  af.bitvector = 0;
  affect_to_char (victim, &af);
  act ("$n is surrounded by {Ds{wm{Wo{Dke{x.", victim, NULL, NULL, TO_ROOM);
  send_to_char ("You are surrounded by a shroud of {Ds{wm{Wo{Dke{x.\n\r", victim);
  return;
}

void spell_petrify (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected (victim, sn) || IS_AFFECTED (victim, AFF_SLOW))
  {
    if (victim == ch)
    {
      send_to_char ("You are already petrified!\n\r", ch);
    }
    else
    {
      act ("$N is already petrified from head to toe.", ch, NULL, victim,
           TO_CHAR);
    }
    return;
  }
  if (saves_spell (level, victim, DAM_OTHER)
      || IS_SET (victim->imm_flags, IMM_MAGIC))
  {
    if (victim != ch)
      send_to_char ("Nothing seemed to happen.\n\r", ch);
    send_to_char ("You feel a bit stiff.\n\r", victim);
    return;
  }
  if (IS_AFFECTED (victim, AFF_HASTE))
  {
    if (!check_dispel (level, victim, skill_lookup ("haste")))
    {
      if (victim != ch)
        send_to_char ("Spell failed.\n\r", ch);
      send_to_char ("You feel a bit stiff.\n\r", victim);
      return;
    }
  }
  if (IS_AFFECTED (victim, AFF_FLYING))
  {
    if (!check_dispel (level, victim, skill_lookup ("fly")))
    {
      if (victim != ch)
        send_to_char ("Spell failed.\n\r", ch);
      send_to_char ("You feel a bit stiff.\n\r", victim);
      return;
    }
  }
  act ("$n is petrified.", victim, NULL, NULL, TO_ROOM);

  damage(ch, victim, 1, sn, DAM_BASH, TRUE, TRUE);

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 2;
  af.location = APPLY_DEX;
  af.modifier = -1 - (level >= 18) - (level >= 25) - (level >= 32);
  af.bitvector = AFF_SLOW;
  affect_to_char (victim, &af);
  send_to_char ("You panic and are paralyzed from fright .\n\r", victim);
  act ("A look of shear terror paralyzes $n.", victim, NULL, NULL, TO_ROOM);
  return;
}

void spell_immolation (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const sh_int dam_each[] =
    { 0, 380, 306, 306, 307, 308, 309, 330, 330, 330, 330, 330, 330, 340, 360,
      330, 336, 340, 346, 360, 366, 360, 366, 370, 376, 380, 383, 384, 386, 388,
      390, 430, 440, 460, 468,
      600, 903, 904, 909, 908, 990, 993, 974, 989, 999, 1000, 1033, 1034, 1037,
      1038, 1090
    };
  int dam;
  level = UMIN (level, sizeof (dam_each) / sizeof (dam_each[0]) - 1);
  level = UMAX (0, level);
  dam = number_range (dam_each[level] / 2, dam_each[level] * 6);
  if (saves_spell (level, victim, DAM_LIGHT))
  {
    dam /= 2;
  }
  else
  {
    spell_blindness (skill_lookup ("blindness"), level / 2, ch,
                     (void *) victim, TARGET_CHAR);
  }

  if (!IS_IMMORTAL(victim) && number_percent () > 98)
  {
    victim->position = POS_FIGHTING; // keeps mobs from crashing the MUD! - Taeloch
    raw_kill (victim, ch);
    group_gain (ch, victim);
    return;
  }
  damage(ch, victim, dam, sn, DAM_LIGHT, TRUE, TRUE);
  return;
}

void spell_typhoon (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  dam = number_range (290, 920);
  if (saves_spell (level, victim, DAM_PIERCE))
    dam /= 2;
  damage(ch, victim, dam, sn, DAM_PIERCE, TRUE, TRUE);
  return;
  dam = number_range (135, 200);
  if (saves_spell (level, victim, DAM_BASH))
    dam /= 2;
  damage(ch, victim, dam, sn, DAM_BASH, TRUE, TRUE);
  return;
  dam = number_range (35, 135);
  if (saves_spell (level, victim, DAM_BASH))
    dam /= 2;
  damage(ch, victim, dam, sn, DAM_BASH, TRUE, TRUE);
  return;
}

void spell_radiance (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  AFFECT_DATA af;

  if (is_sanc_spelled(ch))
  {
    send_to_char("You are already protected.\n\r", ch);
    return;
  }

  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
  {
    send_to_char("You are not righteous enough to be granted your holy powers.\n\r", ch);
    return;
  }

  af.where     = TO_AFFECTS2;
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 6;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF2_RADIANT;
  affect_to_char( ch, &af );
  act ("$n is bathed in a brilliant {Wradian{cce{x.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You are bathed in a brilliant {Wradian{cce{x.\n\r", ch);
}

void spell_malevolent_shroud (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  AFFECT_DATA af;

  if (is_sanc_spelled(ch))
  {
    send_to_char("You are already protected.\n\r", ch);
    return;
  }

  if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
  {
    send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
    return;
  }

  af.where     = TO_AFFECTS2;
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 6;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF2_SHROUD;
  affect_to_char( ch, &af );
  act ("$n is shrouded in a sinister {rf{Dog{x.", ch, NULL, NULL, TO_ROOM);
  send_to_char("You are shrouded in a sinister {rf{Dog{x.\n\r", ch);
}

/* ================================ */
void spell_thorn_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,i;
  int bark_sn = skill_lookup("bark skin");
  level  = UMAX(0, level);

  if (!is_affected( ch, bark_sn) )
  {
    send_to_char("You must have {ybark {gskin{x to blast thorns!\n\r", ch);
    return;
  }

  if (level < 25)
    i = 1;
  else
    i = level / 25 + 1;

  act( "$n blasts forth {ythorns{x from his skin{x.", ch, NULL, victim, TO_ROOM);
  for (; i>=0; i--)
  {
    dam = magic_dam(sn,level*2.5);

    if ( saves_spell( level, victim, DAM_PIERCE ) )
      dam /= 2;

    if (victim->in_room != ch->in_room || victim->position == POS_DEAD)
      return;

    damage( ch, victim, dam, sn,DAM_PIERCE,TRUE, TRUE);
  }
  return;
}
