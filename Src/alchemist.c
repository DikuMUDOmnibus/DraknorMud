/* Draknor custom class Alchemist */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "magic.h"
#include "recycle.h"
#include "interp.h"

/*
New item type projectile
V0 - Weapon usage (bitwise)
V1 - Damage Dice // (if not magic)
V2 - Dice size // (if not magic)
V3 - Spell (only used if IS_SET(obj->value[0], ITEM_PROJECTILE_MAGICBOMB)
V4 - Spell level
*/

/* I've based this on do_recite -- Taeloch */
void do_throw( CHAR_DATA *ch, char *argument )
{
  char tpot[MAX_INPUT_LENGTH];
  char tvic[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *bomb; // *quiver;
  int chance;

  argument = one_argument( argument, tpot );
  argument = one_argument( argument, tvic );

  if (IS_GHOST(ch)) {
    send_to_char("The dead are unable to throw objects{x.\n\r",ch);
    return;
  }

  if (!IS_CLASS_ALCHEMIST(ch)
  &&  !IS_CLASS_WOODSMAN(ch)) {
    send_to_char("You don't know how to throw objects{x.\n\r",ch);
    return;
  }

  if ( (bomb = get_obj_carry( ch, tpot, ch )) == NULL)
  {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }

  // if no target is given, assume we're throwing these bombs
  // at the person who's trying to kill us (seems safe)
  if ( (tvic[0] == '\0')
  || ( (victim = get_char_room ( ch, tvic )) == NULL) )
  {
    if ( (ch->position == POS_FIGHTING)
    &&   (ch->fighting != NULL )
    &&   (ch->fighting->in_room == ch->in_room) )
    {
      victim = ch->fighting;
    }
    else
    {
      send_to_char( "You do not not see them here.\n\r", ch );
      return;
    }
  }

  if (ch == victim)
  {
    send_to_char( "You have a feeling that would be unwise.\n\r", ch );
    return;
  }

  if ((bomb->item_type != ITEM_PROJECTILE )
  ||  !IS_SET(bomb->value[0], ITEM_PROJECTILE_MAGICBOMB) )
  {
    send_to_char( "You can't throw that.\n\r", ch );
    return;
  }

  if ( !can_use_clan_obj( ch, bomb ) )
  { // shouldn't happen, but you never know
    send_to_char( "That belongs to another clan.\n\r", ch );
    return;
  }

  if ( ch->level < bomb->level)
  {
    printf_to_char(ch, "The power of %s causes you to drop it.\n\r", bomb->short_descr );
    obj_from_char( bomb );
    obj_to_room( bomb, ch->in_room );
    return;
  }

  act( "$n throws $p at $N.", ch, bomb, victim, TO_ROOM );
  act( "You throw $p at $N.", ch, bomb, victim, TO_CHAR );

  obj_from_char( bomb );

  if ( IS_AFFECTED(ch, AFF_ALACRITY) )
    WAIT_STATE( ch, PULSE_VIOLENCE*1.5);
  else
    WAIT_STATE( ch, PULSE_VIOLENCE*2);

  chance = 20 + ( (get_skill(ch,gsn_throw) * 4) / 5) + (get_curr_stat(ch,STAT_DEX) - 18) + (get_curr_stat(ch,STAT_STR) - 18);

  if (number_percent() >= chance)
  {
    if (number_percent() <= 5)
    {
      act( "You fumble $p, breaking it on the ground.", ch, bomb, victim, TO_CHAR );
      check_improve(ch,gsn_throw,FALSE,4);
      extract_obj( bomb );
    }
    else
    {
      send_to_char("You fail to throw it hard enough.\n\r",ch);
      check_improve(ch,gsn_throw,FALSE,4);
      obj_to_room( bomb, ch->in_room );
      damage(ch,victim,0,0,DAM_BASH,FALSE,FALSE);
    }
  }
  else
  {
    obj_cast_spell( bomb->value[3], bomb->value[4], ch, victim, bomb, FALSE );
    check_improve(ch,gsn_throw,TRUE,2);
    extract_obj( bomb );
  }

  return;
}

/* can copy scrolls at level.  If it's a spell they don't have, lower the level */
void do_transcribe( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *parch, *obj, *clone;
  int i, manacost;
  bool hasspells;

  if ( IS_NPC( ch )                                                  
  || ch->level < skill_table[gsn_transcribe].skill_level[ch->gameclass] )
  {                                          
    send_to_char( "You do not know how to transcribe scrolls.\n\r", ch );
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("Stop moving around, you are {rDEAD{x!\n\r",ch);
    return;
  }

  if (IS_ROOM_SAFE(ch) || IS_ARENA(ch))
  {
      send_to_char("You fail due to the properties of this room.\n\r",ch);
      return;
  }

  if ( argument[0] == '\0' )
  {
    send_to_char( "Transcribe what scroll?\n\r", ch );
    return;
  }

  if ( (obj = get_obj_carry (ch, argument, ch)) == NULL)
  {
    send_to_char( "You can't find that scroll.\n\r", ch );
    return;
  }

  /* Do we have a parchment to scribe spells? */
  for ( parch = ch->carrying; parch; parch = parch->next_content )
  {
    if ((parch->item_type == ITEM_SCROLL)
    &&  (parch->wear_loc == WEAR_HOLD)
    &&  (parch->value[1] < 1)
    &&  (parch->value[2] < 1)
    &&  (parch->value[3] < 1)
    &&  (parch->value[4] < 1))
      break;
  }

  if ( !parch )
  {
    send_to_char( "You are not holding a blank parchment.\n\r", ch );
    return;
  }

  manacost = 0;
  hasspells = TRUE;
  for (i=1;i<5;i++)
  {
    if ( (skill_table[obj->value[i]].spell_fun == spell_null )
    ||   (ch->level < skill_table[obj->value[i]].skill_level[ch->gameclass] )
    ||   (ch->pcdata->learned[obj->value[i]] == 0 ) )
    {
      hasspells = FALSE;
      manacost += number_range(75,125);
    }
    else
      manacost += skill_table[obj->value[i]].min_mana * 1.25;
  }

  act( "$n begins copying a scroll.", ch, obj, NULL, TO_ROOM );
  if (is_affected( ch, skill_lookup( "clear head" ) ) )
    WAIT_STATE( ch, (skill_table[gsn_transcribe].beats*0.75) );
  else
    WAIT_STATE( ch, skill_table[gsn_transcribe].beats );

  if ( ch->mana < manacost )
  {
    send_to_char( "You don't have enough mana.\n\r", ch );
    return;
  }

  if ( number_percent( ) > get_skill(ch, gsn_transcribe))
  {
    act( "$p bursts in {rf{Rl{ya{rmes{x!", ch, parch, NULL, TO_CHAR );
    act( "$p bursts in {rf{Rl{ya{rmes{x!", ch, parch, NULL, TO_ROOM );
    extract_obj( parch );
    spell_fireball(skill_lookup("fireball"), LEVEL_HERO - 1, ch, ch,0); 
    check_improve(ch,gsn_transcribe,FALSE,3);
    return;
  }

  unequip_char( ch, parch );
  extract_obj( parch );

  clone = create_object(obj->pIndexData,0); 
  clone_object(obj,clone);
  obj_to_char(clone,ch);
  equip_char( ch, clone, WEAR_HOLD );

  if (!hasspells)
  {
    clone->level = clone->level*2/3;
    clone->value[0] = clone->value[0]*2/3;
  }

  check_improve(ch,gsn_transcribe,TRUE,4);

  ch->mana -= manacost;
  act( "You have successfully copied $p{x!", ch, clone, NULL, TO_CHAR );

  return;
}

void do_rebrew( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *vial, *obj, *clone;
  int i, manacost;
  bool hasspells;

  if ( IS_NPC( ch )                                                  
  || ch->level < skill_table[gsn_rebrew].skill_level[ch->gameclass] )
  {                                          
    send_to_char( "You do not know how to rebrew potions.\n\r", ch );
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("Stop moving around, you are {rDEAD{x!\n\r",ch);
    return;
  }

  if (IS_ROOM_SAFE(ch) || IS_ARENA(ch))
  {
      send_to_char("You fail due to the properties of this room.\n\r",ch);
      return;
  }

  if ( argument[0] == '\0' )
  {
    send_to_char( "Rebrew what potion?\n\r", ch );
    return;
  }

  if ( (obj = get_obj_carry (ch, argument, ch)) == NULL)
  {
    send_to_char( "You can't find that potion.\n\r", ch );
    return;
  }

  /* Do we have a vialment to scribe spells? */
  for ( vial = ch->carrying; vial; vial = vial->next_content )
  {
    if ((vial->item_type == ITEM_POTION)
    &&  (vial->wear_loc == WEAR_HOLD)
    &&  (vial->value[1] < 1)
    &&  (vial->value[2] < 1)
    &&  (vial->value[3] < 1)
    &&  (vial->value[4] < 1))
      break;
  }

  if ( !vial )
  {
    send_to_char( "You are not holding an empty vial.\n\r", ch );
    return;
  }

  manacost = 0;
  hasspells = TRUE;
  for (i=1;i<5;i++)
  {
    if ( (skill_table[obj->value[i]].spell_fun == spell_null )
    ||   (ch->pcdata->learned[obj->value[i]] == 0 ) )
      continue;

    if (ch->level < skill_table[obj->value[i]].skill_level[ch->gameclass] )
    {
      hasspells = FALSE;
      manacost += 100;
    }
    else
      manacost += skill_table[obj->value[i]].min_mana * 1.25;
  }

  if (!hasspells)
  {
    send_to_char( "You can only rebrew spells you know.\n\r", ch );
    return;
  }

  act( "$n begins brewing a potion.", ch, obj, NULL, TO_ROOM );
  if (is_affected( ch, skill_lookup( "clear head" ) ) )
    WAIT_STATE( ch, (skill_table[gsn_rebrew].beats*0.75) );
  else
    WAIT_STATE( ch, skill_table[gsn_rebrew].beats );

  if ( ch->mana < manacost )
  {
    send_to_char( "You don't have enough mana.\n\r", ch );
    return;
  }

  if ( number_percent( ) > get_skill(ch, gsn_rebrew))
  {
    act( "$p bursts in {rf{Rl{ya{rmes{x!", ch, vial, NULL, TO_CHAR );
    act( "$p bursts in {rf{Rl{ya{rmes{x!", ch, vial, NULL, TO_ROOM );
    extract_obj( vial );
    spell_fireball(skill_lookup("fireball"), LEVEL_HERO - 1, ch, ch,0); 
    check_improve(ch,gsn_rebrew,FALSE,3);
//    if (IS_CLASS_ALCHEMIST(ch))
//      check_improve(ch,sn,TRUE,1); // alchemists can't cast, have to improve somehow
    return;
  }

  unequip_char( ch, vial );
  extract_obj( vial );

  clone = create_object(obj->pIndexData,0); 
  clone_object(obj,clone);
  obj_to_char(clone,ch);
  equip_char( ch, clone, WEAR_HOLD );

  check_improve(ch,gsn_rebrew,TRUE,4);

  ch->mana -= manacost;
  act( "You have successfully brewed a copy of $p{x!", ch, clone, NULL, TO_CHAR );

  return;
}

void spell_aqua_regia ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ // acidic shroud
  AFFECT_DATA af;

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.duration  = level / 4;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.level     = level;
  af.bitvector = AFF_AQUA_REGIA;
  affect_to_char( ch, &af );

  send_to_char("You are surrounded by an {cac{gi{cd{Ci{cc{x shroud.\n\r", ch);
  act( "$n is surrounded by an {cac{gi{cd{Ci{cc{x shroud.", ch, NULL, NULL, TO_ROOM);
  return;
}

void spell_aqua_fortis ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ // armor
  AFFECT_DATA af;

  if ( is_affected( ch, sn ) )
  {
    send_to_char("You are already protected.\n\r",ch);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level / 2;
  af.duration  = level / 2;
  af.modifier  = 0 - (10+(level/2)); // -15 @ L10, -60 @ L100
  af.location  = APPLY_AC;
  af.bitvector = 0;
  affect_to_char( ch, &af );
  send_to_char( "You are protected by a shield of water.\n\r{x", ch );
  act( "$n is protected by a shield of water.", ch, NULL, NULL, TO_ROOM);
  return;
}

void spell_aqua_landhi ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ // waterproof
  AFFECT_DATA af;

  if ( is_affected( ch, sn )
  ||   is_affected( ch, skill_lookup("fish breath") ) )
  {
    send_to_char("You can already breathe water.\n\r",ch);
    return;
  }

  af.where   = TO_AFFECTS;
  af.type      = sn;
  af.level   = level;
  af.duration  = level/2;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = 0;
  affect_to_char( ch, &af );
  send_to_char( "You can now breathe water.\n\r{x", ch );
  return;
}

void spell_aqua_vitae ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ // healing water
  int heal;

  heal = dice(2, 8) + (level *0.75) ;
  ch->hit = UMIN( ch->hit + heal, GET_HP(ch) );
  update_pos( ch );
  send_to_char( "You feel better!\n\r", ch );
  if (ch->hit >= GET_HP(ch))
    send_to_char("Fully healed.\n\r",ch);
  return;
}

void spell_aqua_citrinitas ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ // clear head
  AFFECT_DATA af;

  if ( is_affected( ch, sn ) )
  {
    send_to_char("You are already enlightened.\n\r",ch);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 1 + level / 9;
  af.location  = APPLY_INT;
  af.modifier  = +1;
  af.bitvector = 0;
  affect_to_char( ch, &af );
  act( "$n becomes enlightened.{x", ch, NULL, NULL, TO_ROOM );
  send_to_char( "You feel enlightened.\n\r", ch );
  return;
}

void spell_aqua_rubedo ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ // bless-ish
  AFFECT_DATA af;
  if ( is_affected( ch, sn ))
  {
    send_to_char("You can't get any closer to your god.\n\r",ch);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 3;
  af.location  = APPLY_HITROLL;
  af.modifier  = level / 6;
  af.bitvector = 0;
  affect_to_char( ch, &af );

  send_to_char( "You feel closer to your god.\n\r", ch );
}

void spell_aqua_albedo ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ // sanctuary
  AFFECT_DATA af;

  if (is_sanc_spelled(ch))
  {
    send_to_char("You are already protected.\n\r",ch);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 5;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_AQUA_ALBEDO;
  affect_to_char( ch, &af );
  act( "$n is surrounded by a {Wshimmering{x aura.", ch, NULL, NULL, TO_ROOM );
  send_to_char( "You are surrounded by a {Wshimmering{x aura.\n\r", ch );
  return;
}

void spell_sulfur_blast ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int i=3;
  int dam;

  for(i=3; i>0; i--) {
    dam = magic_dam(sn,level) * 1.1;
    if ( saves_spell( level, victim, DAM_ACID ) )
      dam /= 2;
    if ((victim->in_room == ch->in_room) && (victim->position != POS_DEAD))
      if (damage( ch, victim, dam, sn,DAM_ACID,TRUE, TRUE) == FALSE)
  return;
  }
  return;
}

// Tonic that increases reflexes (-40AC)
void spell_adrenaline_rush ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( ch, sn ) )
    {
      if (victim == ch)
    send_to_char("Your reflexes are already heightened.\n\r",ch); 
      else
    act("$N is already as attentive as can be.",ch,NULL,victim,TO_CHAR);
      return;
    }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_AC;
  af.modifier  = -40;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  act( "$n looks more attentive.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "Your reflexes increase from {gad{cre{Dna{cli{gne{x.\n\r", victim );
  return;
}

// DAM_FIRE attack, highest level attack spell
void spell_dragon_flame ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,hp_dam,dice_dam;
  int hpch;

  act("$n is engulfed in a column of {yf{Ri{rre{x!",victim,NULL,NULL,TO_ROOM);

  hpch = UMAX( 10, ch->hit );
  hp_dam  = number_range( hpch/9+1, hpch/5 );
  dice_dam = dice(level,20);

  dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
  fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);

  if (saves_spell(level,victim,DAM_FIRE))
  {
    fire_effect(victim,level/2,dam/4,TARGET_CHAR);
    damage(ch,victim,dam/2,sn,DAM_FIRE,TRUE, TRUE);
  }
  else
  {
    fire_effect(victim,level,dam,TARGET_CHAR);
    damage(ch,victim,dam,sn,DAM_FIRE,TRUE, TRUE);
  }
}

// A trick of bending the light (AFF_INVIS)
void spell_inverted_light ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;

  //Check for already invisible.
  if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
  {
    send_to_char( "You can't get more invisible than you already are.\n\r", ch );
    return;
  }

  act( "The light begins to bend around $n.", ch, NULL, NULL, TO_ROOM );

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level + 12;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_INVISIBLE;
  affect_to_char( ch, &af );
  send_to_char( "The light begins to bend around you.\n\r", ch );
  return;
}

// fire breath affect
void spell_smoke_bomb ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( !IS_AFFECTED(victim,AFF_BLIND) )
  {
    act("$n is blinded by {Ds{wm{Doke{x!",victim,NULL,NULL,TO_ROOM);
    act("Your eyes tear up from {Ds{wm{Doke{x...you can't see a thing!",victim,NULL,NULL,TO_CHAR);
 
    af.where        = TO_AFFECTS;
    af.type         = skill_lookup("fire breath");
    af.level        = level;
    af.duration     = number_range(0,level/15);
    af.location     = APPLY_HITROLL;
    af.modifier     = -4;
    af.bitvector    = AFF_BLIND;
    affect_to_char(victim,&af);
  }
}

// slow
void spell_naja_naja( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
  {
      if (victim == ch)
    send_to_char("You can't move any slower!\n\r",ch);
      else
    act("$N can't get any slower than that.",
        ch,NULL,victim,TO_CHAR);
      if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
      return;
  }

  if ( ch == victim
  &&   is_affected( ch, skill_lookup( "haste" ) )
  &&   ch->pcdata->learned[sn] > 89 )
  {
    remove_affect( ch, TO_AFFECTS, AFF_HASTE );
    send_to_char( "You are no longer moving so quickly.\n\r", ch );
    return;
  }

  if ( saves_spell( level, victim, DAM_OTHER )
  ||   IS_SET( victim->imm_flags, IMM_MAGIC ) )
  {
    if (victim != ch)
      send_to_char("Nothing seemed to happen.\n\r",ch);

    send_to_char("You feel momentarily lethargic.\n\r",victim);

    if (!is_affected(victim, skill_lookup("sleep")))
      damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);

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
        damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);

      return;
    }

    act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);

    if (!is_affected(victim, skill_lookup("sleep")))
      damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);

    return;
  }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level*3/4;
  af.location  = APPLY_DEX;
  af.modifier  = level / 9;
  af.bitvector = AFF_SLOW;
  affect_to_char( victim, &af );

  send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
  act("$n starts to move more slowly.",victim,NULL,NULL,TO_ROOM);
  return;
}

// plague
void spell_crotalus( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim, sn)) {
    send_to_char("Their body is riddled with {gd{Di{rs{gease{x.\n\r",ch);
      if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
    return;
  }
    
  if (saves_spell(level,victim,DAM_DISEASE) || 
      (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
    {
      if (ch == victim)
    send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
      else
    act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
      if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
      return;
    }

  af.where     = TO_AFFECTS;
  af.type       = sn;
  af.level      = level;
  af.duration  = level*1.1;
  af.location  = APPLY_STR;
  af.modifier  = 0 - (level / 15); 
  af.bitvector = AFF_PLAGUE;
  affect_join(victim,&af);
  if (!is_affected(victim, skill_lookup("sleep")))
    damage( ch, victim, 1, sn,DAM_MENTAL, FALSE, TRUE);
   
  send_to_char
    ("You scream in agony as plague {rs{Do{gres{x erupt from your skin.\n\r",victim);
  act("$n screams in agony as plague {rs{Do{gres{x erupt from $s skin.",
      victim,NULL,NULL,TO_ROOM);
}

void spell_acidic_gas( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,i;
  ROOM_INDEX_DATA *ch_orig_room, *victim_orig_room;
  level    = UMAX(0, level);

  if (level<10)
    i=1;
  else
    i = level/10+1;

  ch_orig_room = ch->in_room;
  victim_orig_room = victim->in_room;

  for(; i>=0; i--)
  {
    dam = magic_dam(sn,level);

    if ( saves_spell( level, victim, DAM_POISON) )
      dam /= 2;

    if ( victim->in_room != ch->in_room
    ||   victim->position == POS_DEAD )
      return;

    if (damage( ch, victim, dam, sn, DAM_POISON ,TRUE, TRUE) == FALSE)
      return;

    if ( ch->in_room != ch_orig_room
    ||   victim->in_room != victim_orig_room)
      i = -1;
  }
  return;

}

void spell_clarity( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( !IS_CLASS_ALCHEMIST(ch))
  {
    send_to_char( "You do not have the mental focus for this.\n\r", ch );
    return;
  } 

  if ( is_affected( victim, sn )
  ||   (IS_SET(ch->res_flags,RES_MAGIC )))
  {
    send_to_char("You are already guarded from magical attacks.\n\r",ch);
    return;
  }

  af.where     = TO_RESIST;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/4;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = RES_MAGIC;
  affect_to_char( victim, &af );

  send_to_char( "Your mind is keener and more guarded.\n\r{x", victim );
  return;
}

void spell_alacrity( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( !IS_CLASS_ALCHEMIST(ch))
  {
    send_to_char( "You do not have the mental focus for this.\n\r", ch );
    return;
  }

  if ( is_affected( victim, sn )
  ||   (IS_SET(ch->res_flags,RES_MAGIC )))
  {
    send_to_char("You are already moving with alacrity.\n\r",ch);
    return;
  }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 1 + level/9;
  af.location  = APPLY_DEX;
  af.modifier  = level / 33;
  af.bitvector = AFF_ALACRITY;
  affect_to_char( victim, &af );

  send_to_char( "You are filled with a sense of alacrity.\n\r{x", victim );
  return;
}

void spell_hyracal_pressure(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  level    = UMAX(0, level);
  dam    = magic_dam(sn,((level * 3) / 2));
  if ( saves_spell( level, victim,DAM_WIND) )
    dam /= 2;
  damage( ch, victim, dam, sn, DAM_WIND ,TRUE, TRUE);
  return;
}

