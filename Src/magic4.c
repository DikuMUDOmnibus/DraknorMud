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

extern char *target_name;

/*
 * Local Declarations
 */
void remove_shroud_effect( CHAR_DATA *ch );
bool has_inquisitor_aura( CHAR_DATA *ch );

/*
 * Remove's the shroud on a player, called by each shroud function*
 */
void remove_shroud_effect( CHAR_DATA *ch )
{

    if ( IS_SET( ch->spell_aff, SAFF_FLAME_SHROUD ) )
    {
        remove_affect( ch, TO_SPELL_AFFECTS, SAFF_FLAME_SHROUD );
        return;
    }
    else if ( IS_SET( ch->spell_aff, SAFF_ICE_SHROUD ) )
    {
        remove_affect( ch, TO_SPELL_AFFECTS, SAFF_ICE_SHROUD );
        return;
    }
    else if ( IS_SET( ch->spell_aff, SAFF_ELECTRIC_SHROUD ) )
    {
        remove_affect( ch, TO_SPELL_AFFECTS, SAFF_ELECTRIC_SHROUD );
        return;
    }
    else if ( IS_SET( ch->spell_aff, SAFF_POISON_SHROUD ) )
    {
        remove_affect( ch, TO_SPELL_AFFECTS, SAFF_POISON_SHROUD );
        return;
    }
    else if ( IS_SET( ch->spell_aff, SAFF_LIFE_DRAIN ) )
    {
        remove_affect( ch, TO_SPELL_AFFECTS, SAFF_LIFE_DRAIN );
        return;
    }
    else if ( IS_SET( ch->spell_aff, SAFF_MANA_DRAIN ) )
    {
        remove_affect( ch, TO_SPELL_AFFECTS, SAFF_MANA_DRAIN );
        return;
    }
    
}

	
void spell_turmoil( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

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

    dam = magic_dam(sn,level);
   
    if (IS_NEUTRAL(victim))
         dam = dam * 3 / 4;
   
    if (IS_CLASS_INQUISITOR(ch) && IS_GOOD(victim) )
	 dam /= 2;
    else if (IS_CLASS_OCCULTIST(ch) && IS_EVIL(victim))
	 dam /= 2;
    
  if (IS_CLASS_INQUISITOR(ch))
  {
	  act("You hit $N with a {Dwa{wv{De{x of {Dun{rho{Dl{ry {Dturm{wo{ril{x. ",ch, NULL, victim, TO_CHAR);
	  act("$n hits you with a {Dwa{wv{De{x of {Dun{rho{Dl{ry {Dturm{wo{ril{x.", ch, NULL, victim, TO_VICT);
	  act("$n hits $N with a {Dwa{wv{De{x of {Dun{rho{Dl{ry {Dturm{wo{ril{x.", ch, NULL, victim, TO_NOTVICT);
  }
  else
  {
          act("You hit $N with a {Dwa{wv{De{x of {Dun{rho{Dl{ry {Dturm{wo{ril{x.", ch, NULL, victim, TO_CHAR);
          act("$n hits you with a {Dwa{wv{De{x of {Dun{rho{Dl{ry {Dturm{wo{ril{x.", ch, NULL, victim, TO_VICT);
          act("$n hits $N with a {Dwa{wv{De{x of {Dun{rho{Dl{ry {Dturm{wo{ril{x.", ch, NULL, victim, TO_NOTVICT);
   }
   
    damage( ch, victim, dam, sn,DAM_NEGATIVE,TRUE, TRUE);
        return;
}
/* ================================ */

void spell_holy_ritual( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	
	//one_argument(argument, arg);

	//obj = get_obj_list( ch, arg, ch->in_room->contents );
	if ( (obj->item_type != ITEM_CORPSE_NPC)
    && (obj->item_type != ITEM_CORPSE_PC) )
        {
                send_to_char( "You can only send off corpses, nothing else.\n\r", ch );
                return;
        }
	
	if ( obj->wear_loc != -1 )
	{
		send_to_char( "You are not carrying that corpse.\n\r", ch);
		return;
	}

        if (obj->item_type == ITEM_CORPSE_PC)
        {
                send_to_char("This corpse is protected by the gods and cannot be sent.\n\r",ch);
                return;
	}
	
	if ( IS_EVIL( ch ) )
	{
		send_to_char( "Your heart is too dark for this spell.\n\r", ch);
		return;
	}
	
	if (number_percent() < 90 )
	{
		int mp;
		int ldiff;
		int omana;
		int clevel = URANGE( 1, level, MAX_LEVEL );
		int olevel = URANGE( 1, obj->level, MAX_LEVEL );;
		
		omana  = interpolateNew( olevel, 1, MAX_LEVEL, 33, MAX_LEVEL * 5 );

		ldiff = MAX_LEVEL + clevel - olevel;

		mp = omana * MAX_LEVEL / UMAX( 1, ldiff )
			* interpolateNew( clevel, 1, MAX_LEVEL, MAX_LEVEL/2, MAX_LEVEL )
	 		/ MAX_LEVEL;
		mp = UMAX( mp,  25 );
		
		if ( mp > MAX_LEVEL*4 )
			    mp = ( mp - MAX_LEVEL*4 ) / 2 + MAX_LEVEL*4;
		if ( mp > MAX_LEVEL*5 )
			    mp = ( mp - MAX_LEVEL*5 ) / 2 + MAX_LEVEL*5;
		if ( mp > MAX_LEVEL*6 )
			    mp = ( mp - MAX_LEVEL*6 ) / 2 + MAX_LEVEL*6;
		if ( mp > MAX_LEVEL*7 )
			    mp = ( mp - MAX_LEVEL*7 ) / 2 + MAX_LEVEL*7;

		if ( mp > 5 )
			    mp += number_range( 1, mp / 5 ) - number_range( 1, mp / 5 );
		mp = URANGE( 1, mp, GET_MANA(ch) / 2 );
		
		WAIT_STATE( ch, PULSE_VIOLENCE );

		if ( ch->mana >= ch->max_mana )
			 send_to_char( "You can't have any more mana.\n\r",ch );
			
		else
		{
		  printf_to_char( ch, "Your holy ritual produces {G%d {gmana{x%s.\n\r",
		    mp,
			  ch->mana + mp > GET_MANA(ch)
        ? ", replenishing you completely" : "" );

		  ch->mana = UMIN( ch->mana + mp, GET_MANA(ch) );
      obj_from_char( obj );
      extract_obj( obj );
      act("$n performs an incantation, causing $p to disappear.\n\r",ch,obj,NULL,TO_ROOM);

	  }
	}
	else
	{
		act("You attempt to give $p a proper send off, but ruin it.\n\r",ch,obj,NULL, TO_CHAR);
		act("$n destroys $p while attempting to give it a proper send off.\n\r",ch,obj,NULL,TO_ROOM);
      obj_from_char( obj );
      extract_obj( obj );
	}
}
								

void spell_strengthen(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

	if ( victim != ch )
        {
                send_to_char( "You cannot cast this on someone else.\n\r", ch );
                return;
        }

	if ( is_affected( victim, sn ) )
    	{
        	send_to_char("You already feel strengthened.\n\r",ch);
		return;
    	}

  af.where     = TO_AFFECTS2;
  af.type      = sn;
  af.level       = level;
  af.duration  = level;
  af.location  = APPLY_STR;
  af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
  af.bitvector = 0;
  affect_to_char( victim, &af );
  send_to_char( "You feel strengthened.\n\r", victim );
  act("$n's muscles bulge with strength.",victim,NULL,NULL,TO_ROOM);
  return;
}

void spell_nimbleness(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

        if ( is_affected( victim, sn ) )
  	{
	          if (victim == ch)
		          send_to_char("You are already feeling nimble.\n\r",ch);
	          else
	                  act("$N can't get much nimbler than that.",ch,NULL,victim,TO_CHAR);
	          return;
	}
  	
  af.where     = TO_AFFECTS2;
  af.type      = sn;
  af.level       = level;
  af.duration  = level;
  af.location  = APPLY_DEX;
  af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
  af.bitvector = 0;
  affect_to_char( victim, &af );
  send_to_char( "Your body starts to feel more nimble.\n\r", victim );
  act("$n's begins to look more agile.",victim,NULL,NULL,TO_ROOM);
  return;
}


void spell_unholy_ritual( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;

 
  if ( ( obj->item_type != ITEM_CORPSE_NPC )
  &&  ( obj->item_type != ITEM_CORPSE_PC )
  &&   obj->pIndexData->vnum != 1111  )
  {
    send_to_char( "You can only send off corpses.\n\r", ch );
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

  if ( IS_GOOD( ch ) )
  {
    send_to_char( "Your heart is not dark enough for this spell.\n\r", ch);
    return;
  }

  if (number_percent() < 90 )
  {
    int mp;
    int ldiff;
    int omana;
    int clevel = URANGE( 1, level, MAX_LEVEL );
    int olevel = URANGE( 1, obj->level, MAX_LEVEL );;

    omana  = interpolateNew( olevel, 1, MAX_LEVEL, 33, MAX_LEVEL * 5 );

    ldiff = MAX_LEVEL + clevel - olevel;

    mp = omana * MAX_LEVEL / UMAX( 1, ldiff )
      * interpolateNew( clevel, 1, MAX_LEVEL, MAX_LEVEL/2, MAX_LEVEL )
      / MAX_LEVEL;
    mp = UMAX( mp,  25 );

    if ( mp > MAX_LEVEL * 4 )
      mp = ( mp - MAX_LEVEL * 4 ) / 2 + MAX_LEVEL * 4;
    else if ( mp > MAX_LEVEL * 5 )
      mp = ( mp - MAX_LEVEL * 5 ) / 2 + MAX_LEVEL * 5;
    else if ( mp > MAX_LEVEL * 6 )
      mp = ( mp - MAX_LEVEL * 6 ) / 2 + MAX_LEVEL * 6;
    else if ( mp > MAX_LEVEL * 7 )
      mp = ( mp - MAX_LEVEL * 7 ) / 2 + MAX_LEVEL * 7;
    else
      mp += number_range( 1, mp / 5 ) - number_range( 1, mp / 5 );

    mp = URANGE( 1, mp, GET_MANA( ch ) / 2 );

    WAIT_STATE( ch, PULSE_VIOLENCE );

    //GET_MANA includes bonus' from Intelligence.
    if ( ch->mana >= GET_MANA(ch) )
      send_to_char( "You can't have any more mana.\n\r",ch );
    else
    {
      printf_to_char( ch, "Your unholy ritual produces {G%d {gmana{x%s.\n\r",
        mp,
        ch->mana + mp > GET_MANA(ch) ? ", replenishing you completely" : "" );
      ch->mana = UMIN( ch->mana + mp, GET_MANA(ch) );

			//Add bloodshards 2015-06-11
			ch->pcdata->bloodshards += UMAX( 1, obj->level / 20 );
			printf_to_char( ch, "You draw {r%d bloodshards{x from the corpse.\n\r",
				UMAX(1, obj->level / 20) );

      obj_from_char( obj );
      extract_obj( obj );
      act("$n performs an incantation, causing $p to disappear.\n\r",ch,obj,NULL,TO_ROOM);
    }
  }
  else
  {
    act("You attempt to give $p a proper send off, but ruin it.\n\r",ch,obj,NULL, TO_CHAR);
    act("$n destroys $p while attempting to give it a proper send off.\n\r",ch,obj,NULL,TO_ROOM);
    obj_from_char( obj );
    extract_obj( obj );
  }
}

/* Mystic Shroud Spells.  Added Dec '03  */
void spell_soul_shroud( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  AFFECT_DATA af;

  remove_shroud_effect( ch );

  af.where    = TO_SPELL_AFFECTS;
  af.type     = sn;
  af.duration  = level / 5;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.level     = level;
  af.bitvector = SAFF_LIFE_DRAIN;
  affect_to_char( ch, &af );

  send_to_char("Your hands are shrouded in the power of {msoul{rs{x.\n\r", ch);
  act( "$n focuses and $s hands are shrouded in the power of {msoul{rs{x.", ch, NULL, NULL, TO_ROOM);
  return;
}

void spell_spirit_shroud( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  AFFECT_DATA af;

  remove_shroud_effect( ch );

  af.where    = TO_SPELL_AFFECTS;
  af.type     = sn;
  af.duration  = level / 5;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.level     = level;
  af.bitvector = SAFF_MANA_DRAIN;
  affect_to_char( ch, &af );

  send_to_char("Your hands are shrouded in the power of {Dsp{Gi{gr{Di{gt{x.\n\r", ch);
  act( "$n focuses and $s hands are shrouded in the power of {Dsp{Gi{gr{Di{gt{x.", ch, NULL, NULL, TO_ROOM);
  return;
}

void spell_flame_shroud( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  AFFECT_DATA af;

  remove_shroud_effect( ch );

  af.where    = TO_SPELL_AFFECTS;
  af.type     = sn;
  af.duration  = level / 5;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.level     = level;
  af.bitvector = SAFF_FLAME_SHROUD;
  affect_to_char( ch, &af );

  send_to_char("Your hands are shrouded in {rf{Yl{Ra{ym{re{x.\n\r", ch);
  act( "$n focuses and $s hands are shrouded in {rf{Yl{Ra{ym{re{x.", ch, NULL, NULL, TO_ROOM);
  return;
}

void spell_frost_shroud( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  AFFECT_DATA af;

  remove_shroud_effect( ch );

  af.where    = TO_SPELL_AFFECTS;
  af.type     = sn;
  af.duration  = level / 5;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.level     = level;
  af.bitvector = SAFF_ICE_SHROUD;
  affect_to_char( ch, &af );

  send_to_char("Your hands are shrouded in {wi{cc{be{x.\n\r", ch);
  act( "$n focuses and $s hands are shrouded in {wi{cc{be{x.", ch, NULL, NULL, TO_ROOM);
  return;
}

void spell_electric_shroud( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  AFFECT_DATA af;

  remove_shroud_effect( ch );

  af.where    = TO_SPELL_AFFECTS;
  af.type     = sn;
  af.duration  = level / 5;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.level     = level;
  af.bitvector = SAFF_ELECTRIC_SHROUD;
  affect_to_char( ch, &af );

  send_to_char("Your hands are shrouded in {ye{Wl{yec{Wt{yr{Yi{yc{Wi{Yt{yy{x.\n\r", ch);
  act( "$n focuses and $s hands are shrouded in {ye{Wl{yec{Wt{yr{Yi{yc{Wi{Yt{yy{x.", ch, NULL, NULL, TO_ROOM);
  return;
}

void spell_poison_shroud( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
  AFFECT_DATA af;

  remove_shroud_effect( ch );

  af.where    = TO_SPELL_AFFECTS;
  af.type     = sn;
  af.duration  = level / 5;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.level     = level;
  af.bitvector = SAFF_POISON_SHROUD;
  affect_to_char( ch, &af );

  send_to_char("Your hands are shrouded in {Gpo{ci{gson{x.\n\r", ch);
  act( "$n focuses and $s hands are shrouded in {Gpo{ci{gson{x.", ch, NULL, NULL, TO_ROOM);
  return;
}

void spell_rejuvination( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;

	heal = dice(3, 8) + (level * 3) - 6;
	victim->hit = UMIN( victim->hit + heal, GET_HP(victim) );
	update_pos( victim );
	send_to_char( "You feel a surge of energy flow into you!\n\r", victim );

    if (victim->hit >= GET_HP(victim))
		send_to_char("Fully healed.\n\r",ch);
	else
	{
		if ( ch != victim )
			act( "You have given $N a surge of energy, greatly healing wounds.",
			ch, NULL, victim, TO_CHAR );
	}
 
	return;
}

/*
 * Spell: Cure Weaken
 * Class: Priest only
 */
void spell_cure_weaken( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( !is_affected( victim, gsn_weaken ) )
  {
    if (victim == ch)
        send_to_char("You don't feel weakened.\n\r",ch);
    else
        act("$N doesn't appear to be weakened.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if (check_dispel(level,victim,gsn_weaken)) // dispel has its own TO_CHAR message
    act("$n looks much stronger.",victim,NULL,NULL,TO_ROOM);
  else
    send_to_char("Spell failed.\n\r",ch);
}

/*
 * has_inquisitor_aura
 * Check to see if they one one of the Inquisitor group affects
 */
bool has_inquisitor_aura( CHAR_DATA *ch )
{
  if ( is_affected( ch, skill_lookup("prayer") )
  ||   is_affected( ch, skill_lookup("conviction") )
  ||   is_affected( ch, skill_lookup("faith") ) )
    return TRUE;
  
  return FALSE;
}


/*
 * Spell: Prayer
 * Class: Inquisitor
 * Give whole party an AC bonus
 */
void spell_prayer( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  CHAR_DATA *gch;

  if ( IS_NPC(ch) ) return;

  if ( has_inquisitor_aura(ch) )
  {
    send_to_char( "You are already protected by an aura.\n\r", ch );
    return;
  }

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
  {
    if ( !is_same_group(gch, ch)
    || !can_see(ch, gch)
    || IS_WIZINVIS(gch,ch))
      continue;

    if ( is_affected(gch, sn) )
    {
      if ( ch != gch )
        act( "$N is already protected by a prayer.", ch, NULL, gch, TO_CHAR);

      continue;
    }

    if ( has_inquisitor_aura(gch) )
      continue;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.modifier  = -(level / 3);
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( gch, &af );
    printf_to_char( gch, "You feel guarded by a prayer.\n\r", ch->name );

    if ( ch != gch )
      act( "$N is given a prayer.", ch, NULL, gch, TO_CHAR);
  }

  send_to_char( "Ok.\n\r", ch );
  return;
}

void spell_conviction( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  CHAR_DATA *gch;

  if ( IS_NPC(ch) ) return;

  if ( has_inquisitor_aura(ch) )
  {
    send_to_char( "You are already protected by an aura.\n\r", ch );
    return;
  }


  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
  {
    if ( !is_same_group(gch, ch)
    || !can_see(ch, gch)
    || IS_WIZINVIS(gch,ch) )
      continue;

    if ( is_affected(gch, sn) )
    {
      if ( ch != gch )
        act( "$N already has a strong conviction.", ch, NULL, gch, TO_CHAR );
      continue;
    }

    if ( has_inquisitor_aura(gch) )
      continue;

    af.where  = TO_AFFECTS;
    af.type   = sn;
    af.level  = ch->level;
    af.duration = number_fuzzy( ch->level/8 );
    af.modifier = UMAX( 1, ch->level/6 );
    af.bitvector  = 0;

    af.location = APPLY_HITROLL;
    affect_to_char(gch,&af);

    af.location = APPLY_DAMROLL;
    affect_to_char(gch,&af);

    printf_to_char( gch, "You feel the strength of conviction.\n\r", ch->name );

    if ( ch != gch )
      act( "$N is given a stronger conviction.", ch, NULL, gch, TO_CHAR );
  }

  send_to_char( "Ok.\n\r", ch );

  return;
}

void spell_faith( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  AFFECT_DATA af;
  CHAR_DATA *gch;

  if ( IS_NPC(ch) ) return;

  if ( has_inquisitor_aura(ch) )
  {
    send_to_char( "You are already protected by an aura.\n\r", ch );
    return;
  }

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
  {
    if ( !is_same_group(gch, ch)
    || !can_see(ch, gch)
    || IS_WIZINVIS(gch,ch) )
      continue;

    if ( is_affected(gch, sn) )
    {
      if ( ch != gch )
        act( "$N already has great faith.", ch, NULL, gch, TO_CHAR );
      continue;
    }

    if ( has_inquisitor_aura(gch) )
      continue;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  =  -(level/7);
    af.bitvector = 0;
    affect_to_char( gch, &af );

    printf_to_char( gch, "You are filled with faith, protecting your mind from harm.\n\r", ch->name );

    if ( ch != gch )
      act( "$N is given great faith.", ch, NULL, gch, TO_CHAR );
  }

  send_to_char( "Ok.\n\r", ch );

  return;
}
