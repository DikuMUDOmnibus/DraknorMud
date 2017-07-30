/*  All functions written in this file are property of the Draknor Codebase copyright 2003. And are not
 *  allowed to be used without the direct permission of the author(s) of that MUD.*/

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

//Local Declarations
bool is_occ_evil( CHAR_DATA *ch );


/*
 * Function: is_occ_evil( ch )
 * Check to see if the Occultist is evil aligned, return FALSE if not.
 */
bool is_occ_evil( CHAR_DATA *ch )
{
  if ( IS_CLASS_OCCULTIST( ch )
  &&   !IS_EVIL( ch ) )
  {
      send_to_char( "You are not wicked enough to be granted your dark powers.\n\r", ch );
      return FALSE;
  }

  return TRUE;
} 

void spell_pestilence( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{

  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( !is_occ_evil( ch ) )
    return;

  if ( is_affected( victim, skill_lookup("plague" ) )
  &&   is_affected( victim, skill_lookup("poison" ) )
  &&   is_affected( victim, skill_lookup("curse" ) ) )
  {
    send_to_char( "They are already afflicted with a pestilence.\n\r", ch );
    return;
  }

  if ( !is_affected( victim, skill_lookup("plague" ) ) )
    spell_plague(skill_lookup("plague"), level, ch, (void*)victim, TAR_CHAR_OFFENSIVE);

  if ( !is_affected( victim, skill_lookup("poison" ) ) )
    spell_poison(skill_lookup("poison"), level, ch, (void*)victim, TAR_OBJ_CHAR_OFF);

  if ( !is_affected( victim, skill_lookup("curse" ) ) )
    spell_curse(skill_lookup("curse"), level, ch, (void*)victim, TAR_OBJ_CHAR_OFF);

}

void spell_mass_blindness( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *gch;

  if ( !is_occ_evil( ch ) )
    return;

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
  {
      if ( !is_same_group( gch, victim )
      || !can_see(ch, gch)
      || IS_WIZINVIS(gch,ch))
        continue;

      if ( is_affected( gch, skill_lookup( "blindness" ) ) )
      {
        if (ch != gch)
            act("$N is already blind.",ch,NULL,gch,TO_CHAR);

        continue;
      }

      af.where     = TO_AFFECTS;
      af.type      = skill_lookup( "blindness" );
      af.level     = level;
      af.location  = APPLY_HITROLL;
      af.modifier  = -4;
      af.duration  = 1+level;
      af.bitvector = AFF_BLIND;
      affect_to_char( gch, &af );
      send_to_char( "You are blinded!\n\r", gch );

      if (!is_affected( gch, skill_lookup("sleep")))
        damage( ch, gch, 1, skill_lookup( "blindness" ),DAM_MENTAL, FALSE, TRUE);

        act("$N appears to be blinded.",ch,NULL,gch,TO_NOTVICT);

  }
  send_to_char( "Ok.\n\r", ch );

  return;
}

void spell_mass_weaken( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *gch;

  if ( !is_occ_evil( ch ) )
    return;

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
  {
      if ( !is_same_group( gch, victim )
      || !can_see(ch, gch)
      || IS_WIZINVIS(gch,ch))
        continue;

      if ( is_affected( gch, skill_lookup( "weaken" ) ) )
      {
        if (ch != gch)
            act("$N is already weakened.",ch,NULL,gch,TO_CHAR);

        continue;
      }

      af.where     = TO_AFFECTS;
      af.type      = skill_lookup( "weaken" );
      af.level     = level;
      af.duration  = level / 2;
      af.location  = APPLY_STR;
      af.modifier  = -1 * (level / 5);
      af.bitvector = AFF_WEAKEN;
      affect_to_char( gch, &af );
      
      send_to_char( "You begin to feel weaker!\n\r", gch );

      if (!is_affected( gch, skill_lookup("sleep")))
        damage( ch, gch, 1, skill_lookup( "weaken" ),DAM_MENTAL, FALSE, TRUE);

        act("$N appears weaker.",ch,NULL,gch,TO_CHAR);

  }
  send_to_char( "Ok.\n\r", ch );

  return;
}

void spell_mass_curse( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *gch;

  if ( !is_occ_evil( ch ) )
    return;

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
  {
      if ( !is_same_group( gch, victim )
      || !can_see(ch, gch)
      || IS_WIZINVIS(gch,ch))
        continue;

      if ( is_affected( gch, skill_lookup( "curse" ) ) )
      {
        if (ch != gch)
            act("$N is already cursed.",ch,NULL,gch,TO_CHAR);

        continue;
      }

      af.where     = TO_AFFECTS;
      af.type      = skill_lookup( "curse" );
      af.level     = level;
      af.duration  = 2*level;
      af.location  = APPLY_HITROLL;
      af.modifier  = -1 * (level / 8);
      af.bitvector = AFF_CURSE;
      affect_to_char( gch, &af );

      af.location  = APPLY_SAVING_SPELL;
      af.modifier  = level / 8;
      affect_to_char( gch, &af );


      if (!is_affected( gch, skill_lookup("sleep")))
        damage( ch, gch, 1, skill_lookup( "curse" ),DAM_MENTAL, FALSE, TRUE);

      send_to_char( "You feel unclean.\n\r", gch );
      act("$N looks very uncomfortable.",ch,NULL,gch,TO_CHAR);

  }
  send_to_char( "Ok.\n\r", ch );

  return;
}

void spell_bone_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;

  victim = (CHAR_DATA *) vo;

  if ( !is_occ_evil( ch ) )
    return;

  if ( ch != victim )
  {
    send_to_char( "You can not cast this upon another.\n\r", ch );
    return;
  }

  if (is_affected( victim, sn ) )
  {
    act("You already have an armor of bone.",ch,NULL,victim,TO_CHAR);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level   = level;
  af.duration  = level;
  af.location  = APPLY_AC;
  af.modifier  = -( level / 3 );
  af.bitvector = 0;
  affect_to_char( victim, &af );

  af.location  = APPLY_SAVING_SPELL;
  af.modifier  = -( level / 15 );
  affect_to_char( victim, &af );

  send_to_char( "A protective armor of bone covers your body.\n\r", victim );
  return;
}

/*
 * Spell: Ghostly Wail
 * Concept: attack one target with with Sound damage.
 */
void spell_ghostly_wail( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ( !is_occ_evil( ch ) )
    return;

  dam = magic_dam(sn,level);

  if ( saves_spell( level, victim, DAM_SOUND ) )
    dam /= 2;

  damage( ch, victim, dam, sn, DAM_SOUND, TRUE, TRUE );
  return;
}

/*
 * Spell: Skeletal Spike
 * Concept: Attack one target with a spike (Pierce dmg)
 */
void spell_skeletal_spike( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ( !is_occ_evil( ch ) )
    return;

  dam = magic_dam(sn,level);

  if ( saves_spell( level, victim, DAM_PIERCE ) )
    dam /= 2;

  damage( ch, victim, dam, sn, DAM_PIERCE, TRUE, TRUE );
  return;
}

/*
 * Spell: Nighmare
 * Concept: Disrupt someone's dreams, prevent any regeneration
 */
void spell_nightmare( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( ch == victim && !IS_IMMORTAL( ch ) )
    {
        send_to_char( "You can't seem to terrorize your own dreams.\n\r", ch );
        return;
    }

    if ( !is_occ_evil( ch ) )
        return;

    if ( is_affected( victim, sn ) )
    {
        send_to_char( "Their head is already filled with nightmares.\n\r", ch );
        return;
    }

    if ( saves_spell( level, victim, DAM_MENTAL ) )
    {
        send_to_char("You failed to terrorize their dreams.\n\r",ch);

        if ( !IS_AFFECTED( victim, AFF_SLEEP ) )
            damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);

        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = ( level * 1.5 );
    af.bitvector = 0;
    affect_to_char( victim, &af );

    printf_to_char( ch, "You intrude %s's mind, leaving terror in your wake.\n\r",
        victim->name );
    printf_to_char( victim, "%s has filled your thoughts with terror.\n\r",
        ch->name );

    return;
}

void spell_bone_rot( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice( level, 19 );

  if ( !is_occ_evil( ch ) )
    return;

  if ( saves_spell( level, victim, DAM_DISEASE ) )
    dam /= 2;

  if ( dam > 0 )
    act("$n twitches in intense pain as $s bone's begin to rot from the inside.{x",
        victim, NULL, NULL, TO_ROOM );

  damage( ch, victim, dam, sn, DAM_DISEASE, TRUE, TRUE);
  return;
}

void spell_poisonous_dart( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,i;
  char buf[MIL];

  i = UMAX( 2, level / 10 );

  mprintf( sizeof( buf ), buf,
    "{gPo{Gi{gs{Gon{gou{Gs{x darts form above %s's fingertips, they shoot towards %s.",
        ch->name, IS_NPC( victim ) ? victim->short_descr : victim->name );

  act( buf, ch, NULL, victim, TO_NOTVICT);

  for(; i>0; i--)
  {
    dam = magic_dam(sn,level);

    if ( saves_spell( level, victim, DAM_POISON ) )
      dam /= 2;

    if (victim->in_room != ch->in_room || victim->position == POS_DEAD)
        return;

    damage( ch, victim, dam, sn,DAM_POISON,TRUE, TRUE);
  }

  return;
}

void spell_spirit_leech( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ( saves_spell( level, victim,DAM_NEGATIVE) )
  {
      send_to_char("You feel a momentary chill.\n\r",victim);
      send_to_char("Your attempt to leech their spirit failed.\n\r",ch);
      return;
  }

  dam = dice( level, 15 );

  if ( !is_occ_evil( ch ) )
    return;

  if ( IS_EVIL( victim ) )
    dam /= 2;

  if ( saves_spell( level, victim, DAM_NEGATIVE ) )
    dam /= 2;

  ch->hit += ( level * 3 ) / 5;
  if ( ch->hit > GET_HP( ch ) )
    ch->hit = GET_HP( ch );

  send_to_char("A dark force feeds on your {rl{Ri{rfe{x force!\n\r",victim);
  send_to_char("You {rl{Ree{rch{x your opponent's life force.\n\r",ch);
  damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, TRUE);

  return;
}

// Raising spellgroup added in Dec '03
void spell_clay_golem( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *elemental;

  if ( num_followers( ch ) > 0 )
  {
    send_to_char("You already have a follower.\n\r",ch);
    return;
  }

  if ( IS_SET( ch->in_room->room_flags, ROOM_PRIVATE )
  ||   IS_SET( ch->in_room->room_flags, ROOM_SOLITARY ) )
  {
    send_to_char( "You cannot raise a clay golem here.\n\r", ch );
    return;
  }

  pMobIndex = get_mob_index( MOB_VNUM_CLAY_GOLEM );

  elemental = create_mobile(pMobIndex);

  elemental->level = level - 1;
  elemental->hit = elemental->max_hit = number_fuzzy( elemental->level * 40 + 1000 );
  elemental->mana = elemental->max_mana = 100;
  elemental->move = elemental->max_move = number_fuzzy( elemental->level * 10 + 150 );
  elemental->armor[0]=elemental->armor[1]=elemental->armor[2]=elemental->armor[3]=number_fuzzy(elemental->level * -3 +180);
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

  elemental->name = str_dup("clay golem",elemental->name);
  elemental->short_descr = str_dup("a clay golem",elemental->short_descr);
  elemental->long_descr = str_dup("A clay {Dgol{yem{x stomps about.\n\r",elemental->long_descr);
  elemental->description =
    str_dup("This golem has been raised by the power of death.  He looks menacing.\n\r",elemental->description);
  elemental->dam_type = 27;          /* smash. */

  SET_BIT(elemental->affected_by, AFF_CHARM);
  SET_BIT(elemental->act, ACT_PET);
  SET_BIT(elemental->act, ACT_WARRIOR);
  SET_BIT(elemental->act,ACT_NOGHOST);
  SET_BIT(elemental->affected_by, AFF_HASTE);

  elemental->alignment = ch->alignment;

  act( "You create a clay {Dgol{yem{x.", ch, NULL, NULL, TO_CHAR );
  act( "$n has created a clay {Dgol{yem{x.", ch, NULL, NULL, TO_ROOM);

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

void spell_raise_skeleton( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *elemental;
  int i = 0;
  OBJ_DATA *obj = (OBJ_DATA *) vo;


  if ( ( obj->item_type != ITEM_CORPSE_NPC )
  &&  ( obj->item_type != ITEM_CORPSE_PC )
  &&   obj->pIndexData->vnum != 1111  )
  {
    send_to_char( "You must have a corpse to turn into a skeleton.\n\r", ch );
    return;
  }

  if ( obj->wear_loc != -1 )
  {
    send_to_char( "You are not carrying that corpse.\n\r", ch);
    return;
  }

  if ( obj->item_type == ITEM_CORPSE_PC
  &&   obj->contains )
  {
    send_to_char("This corpse is protected by the gods and cannot be sent.\n\r",ch);
    return;
  }

  if ( num_followers( ch ) > 0 )
  {
    send_to_char("You already have a follower.\n\r",ch);
    return;
  }

  if ( IS_SET( ch->in_room->room_flags, ROOM_PRIVATE )
  ||   IS_SET( ch->in_room->room_flags, ROOM_SOLITARY ) )
  {
    send_to_char( "You cannot raise a skeleton here.\n\r", ch );
    return;
  }

  pMobIndex = get_mob_index( 2699 );

  elemental = create_mobile(pMobIndex);

  elemental->level = number_fuzzy(level / 2);
  elemental->mana = elemental->max_mana = 20;
  elemental->hit = elemental->max_hit = number_fuzzy(GET_HP(ch) / 2);
  for(i = 0; i < 4; i++)
    elemental->armor[i] = number_fuzzy(ch->armor[i] - 10);
  elemental->hitroll = number_fuzzy(level / 30);
  elemental->damroll = number_fuzzy(level / 30);
  elemental->gold = 0;
  elemental->silver = 0;

  //elemental->damage[0]=(elemental->level)/20;
  //elemental->damage[1]=(elemental->level)/2;
  //elemental->damage[2]=(elemental->level)/5;

  free_string(elemental->name);
  free_string(elemental->short_descr);
  free_string(elemental->long_descr);
  free_string(elemental->description);

  elemental->name = str_dup("skeleton",elemental->name);
  elemental->short_descr = str_dup("an obedient skeleton",elemental->short_descr);
  elemental->long_descr = str_dup("An obedient skeleton stands ready.\n\r",elemental->long_descr);
  elemental->description =
    str_dup("This skeleton has been raised by the power of death.  He stands ready.\n\r",elemental->description);
  elemental->dam_type = 18;          /* wrath. */

  SET_BIT(elemental->affected_by, AFF_CHARM);
  SET_BIT(elemental->act, ACT_PET);
  SET_BIT(elemental->act, ACT_NOGHOST);

  elemental->alignment = ch->alignment;

  act( "You raise an obedient skeleton.", ch, NULL, NULL, TO_CHAR );
  act( "$n has raised a skeleton as a follower{x.", ch, NULL, NULL, TO_ROOM);

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

  //get rid of the corpse
  obj_from_char( obj );
  extract_obj( obj );

  return;
}

void spell_demonic_screech( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *pChar, *pChar_next;
  int dam;
  pChar_next = NULL;

  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
  {
    pChar_next = pChar->next_in_room;

    if ( !is_safe_spell( ch, pChar, FALSE, TRUE ) && (pChar != ch) && (!IS_WIZINVIS(pChar,ch)))
    {
      act( "$n sends forth waves of screeching {ren{Re{Drg{ry{x.", ch, NULL, pChar, TO_VICT );
      dam = magic_dam(sn,level);

      if ( saves_spell( level, pChar, DAM_SOUND ) )
        dam /= 2;

      damage( ch, pChar, dam, sn, DAM_SOUND,TRUE, TRUE);
    }
  }
  return;
}

