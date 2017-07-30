/*  All functions written in this file are property of the Draknor Codebase copyright 2007. And are not
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

extern char *target_name;

/*
?Spell - Frenzy-like spell with ??
Ambient Darkness - Temporarily makes a room dark
Slow Metabolism - Slow hunger and/or thirst counter
Turn - Makes undead flee in terror (classic D&D spell)
Reveal - Removes invisibility flag from a person or object
Silence - Temporarily removes ability to speak (in-game) and cast spells(?)
Communion - Speak with the recently-deceased (requires a corpse, probably?)
Intervene - Rescue for Inquisitor
Ward Undead - Protect from attacks by the undead (sorta like protection evil)
Imbue Weapon - Give temporary hit/dam bonus to a weapon (like swordright)
Interrogate? - No idea, but it's a cool word :)
*/

/* copied from spell_continual_light */
/*void spell_ambient_light(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  OBJ_DATA *light;
  light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
  strcpy( light->description, "\0" ); //does this work???
  obj_to_room( light, ch->in_room );
  act( "$n prays to $s god and the room begins to glow brightly.",   ch, NULL, NULL, TO_ROOM );
  act( "You pray to your god and the room begins to glow brightly.", ch, NULL, NULL, TO_CHAR );
  return;
}*/

/* copied from spell_entangle */
/*void spell_ethereal_web( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
    {
      if (victim == ch)
	send_to_char("You are already trapped!\n\r",ch);
      else
	act("$N is already trapped.",
	    ch,NULL,victim,TO_CHAR);
      if (!is_affected(victim, skill_lookup("sleep")))
	damage( ch, victim, 1, sn,DAM_MENTAL,TRUE, TRUE);
      return;
    }

  if (saves_spell(level,victim,DAM_OTHER)
      ||  IS_SET(victim->imm_flags,IMM_MAGIC))
    {
      if (victim != ch)
	send_to_char("Nothing seemed to happen.\n\r",ch);
      send_to_char("You feel restricted for a moment, then it's gone.\n\r",victim);
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
	  send_to_char("You feel restricted for a moment, then it's gone.\n\r",victim);
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
  send_to_char( "An ethereal web appears out of thin air and wraps around you.\n\r", victim );
  act("$n becomes wrapped in an ethereal web.",victim,NULL,NULL,TO_ROOM);
  return;
}*/

void spell_holy_runes( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if( obj->item_type != ITEM_WEAPON )
    {
      send_to_char("You can only cast this spell on weapons.\n\r",ch);
      return;
    }

    if( IS_WEAPON_STAT(obj, WEAPON_HOLY) )
    {
      send_to_char("This weapon is already flaming.\n\r", ch);
      return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 1 + level - skill_table[sn].skill_level[ch->gameclass];
    af.duration = level /2;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = WEAPON_HOLY;
    affect_to_obj(obj, &af);

    act("$n makes {cg{Cl{cowi{Cn{cg{x runes appear on $p{x.", ch, obj, NULL, TO_ROOM);
    act("The runes you create on $p begin to glow with a {yh{Wo{yly {Wlight{x.", ch, obj, NULL, TO_CHAR);
    return;
}


void spell_blessed_shield( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  OBJ_DATA *shield;

  if (target_name[0] != '\0')    /* do a glow on some object */
  {
    shield = get_obj_carry(ch,target_name,ch);

    if (shield == NULL)
    {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }

    if ( !CAN_WEAR( shield, ITEM_WEAR_SHIELD ) )
    {
      send_to_char( "You can only cast this upon shields.\n\r", ch );
      return;
    }

    if (IS_OBJ_STAT(shield,ITEM_BLESSED_SHIELD))
    {
      act("$p is already shimmering with holy power.",ch,shield,NULL,TO_CHAR);
      return;
    }

    af.where       = TO_OBJECT;
    af.type        = sn;
    af.level       = level;
    af.duration    = (level/3);
    af.location    = APPLY_SAVES;
    af.modifier    = -5;
    af.bitvector   = ITEM_BLESSED_SHIELD;
    affect_to_obj(shield,&af);

    SET_BIT(shield->extra_flags,ITEM_BLESSED_SHIELD);
    act("$p glows with a {Yh{yo{Wl{wy light{x.",ch,shield,NULL,TO_ALL);
    return;
  }

  send_to_char( "What do you want to cast this upon?\n\r", ch );
  return;
}

