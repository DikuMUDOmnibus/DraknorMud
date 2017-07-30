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
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "interp.h"

/*
 * Local functions.
 */

void    check_spirit    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void show_to_char(char *str, CHAR_DATA *ch, bool show);
int hand_skill_damage(CHAR_DATA *ch, CHAR_DATA *vch, int *dt);
sh_int martial_style_dt( CHAR_DATA *ch );
int get_style_dam_type(CHAR_DATA *ch);
void make_ghost(CHAR_DATA *ch, CHAR_DATA *victim);
int style_check(CHAR_DATA *ch, CHAR_DATA *vch, int gsn);
void get_size_fight_groups(CHAR_DATA *ch, CHAR_DATA *victim, int *num_attackers,
               int *num_defenders);
bool all_flags_found(CHAR_DATA *ch);

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  CHAR_DATA *victim;

  for ( ch = char_list; ch != NULL; ch = ch_next )
    {
      ch_next    = ch->next;
      if (IS_NPC(ch))
    if (ch->wait > 0 ) {
      ch->wait = ch->wait - 1;
    }
      victim = ch->fighting;
      if (ch->fighting == NULL && victim != NULL) {
    bugf("ERROR in violence update.  Assignment DID not work!\n\r");
    continue;
      }

      if (ch->fighting == NULL)
    continue;

      if ((ch->in_room == NULL )|| 
      (IS_SET(ch->active_flags, ACTIVE_HAS_ATTACKED)))
    continue;

      /* A THOUGHT of CODE */
      /*
    if (!IS_NPC(victim) && (victim->desc == NULL) {
    stop_fighting(victim,TRUE);
    send_to_char("Your victim has gone {Rlinkdead{x\n\r",ch);
    continue;
    }
      */ 
      victim = ch->fighting;
      if (victim) 
    if (!victim->name) {
      bugf("ALERT: NULL  BUG HIT!");
      continue;
    }
      
    
      if ( IS_AWAKE(ch) && (ch->in_room == victim->in_room) ){
    multi_hit( ch, victim, TYPE_UNDEFINED );
    SET_BIT(ch->active_flags, ACTIVE_HAS_ATTACKED);
      }
      else
    stop_fighting( ch, FALSE );

      if ( ( victim = ch->fighting ) == NULL )
    continue;

      /*
       * Fun for the whole family!
       */
      check_assist(ch,victim);

    
      if ( IS_NPC( ch ) )
    {
      if ( HAS_TRIGGER( ch, TRIG_FIGHT ) )
        mp_percent_trigger( ch, victim, NULL, NULL, TRIG_FIGHT );
      if ( HAS_TRIGGER( ch, TRIG_HPCNT ) )
        mp_hprct_trigger( ch, victim );
    }

    if ( ( number_percent() >= 75 )
    &&   ( IS_NPC(ch) )
    &&   ( !is_affected( ch,AFF_CHARM ) )
    &&   IS_SET( ch->act2, ACT2_SWITCHER ) )
        switch_update(ch);
  }

  for ( ch = char_list; ch != NULL; ch = ch->next )
    REMOVE_BIT( ch->active_flags, ACTIVE_HAS_ATTACKED );
  return;
}

//Check a players movement so that they can't fight with < 0..
bool check_movement( CHAR_DATA *ch, CHAR_DATA *vch, int dt, bool silent )
{
    const char *attack;

    if ( IS_NPC( ch ) || ch->move > 0 )
        return TRUE;

    if ( silent )
        return FALSE;

    if ( dt == TYPE_UNDEFINED )
        attack  = "hit";
    else if ( dt >= 0 && dt < MAX_SKILL )
        attack  = skill_table[dt].noun_damage;
    else if ( dt >= TYPE_HIT && dt < TYPE_HIT + MAX_DAMAGE_MESSAGE )
        attack  = get_attack_noun( dt - TYPE_HIT );
    else
    {
        bugf( "Dam_message: bad dt %d", dt );
        attack  = get_attack_name( 0 );
    }

    if ( vch )
    {
        act( "You have run out of movement and cannot $t $N!",
            ch, attack, vch, TO_CHAR );
        act( "$n tries to $t you but is too fatigued!",
            ch, attack, vch, TO_VICT );
        act( "$n tries to $t $N but is too exhausted.",
            ch, attack, vch, TO_NOTVICT );
    }
    else
    {
        act( "You have run out of movement and cannot $t!",
            ch, attack, vch, TO_CHAR );
        act( "$n tries to $t but is too exhausted.",
            ch, attack, vch, TO_ROOM );
    }
    return FALSE;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
  CHAR_DATA *rch, *rch_next;

  for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
      rch_next = rch->next_in_room;
    
      if (IS_AWAKE(rch) && rch->fighting == NULL && can_see(rch,victim))
    {

      /* quick check for ASSIST_PLAYER */
      if (!IS_NPC(ch) && IS_NPC(rch) 
          && IS_SET(rch->off_flags,ASSIST_PLAYERS)
          &&  rch->level + 6 > victim->level)
        {
          do_function(rch,&do_emote,"screams and attacks!");
          SET_BIT(rch->active_flags, ACTIVE_HAS_ATTACKED);
          multi_hit(rch,victim,TYPE_UNDEFINED);
          continue;
        }

      /* PCs next */
      /*if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM) || IS_PET(ch))*/
      if (is_same_group(ch, rch) )
        {
        if ( ( (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
              ||     (IS_AFFECTED(rch,AFF_CHARM) || IS_PET(rch))) 
            &&   (rch->position > POS_SITTING)
            &&   !is_safe(rch, victim, FALSE)){
            if (!IS_NPC(victim) ) {
            if (!IS_NPC(rch) && !is_in_pk_range(rch,victim)) 
                continue; 
            }
            SET_BIT(rch->active_flags, ACTIVE_HAS_ATTACKED);
            multi_hit (rch,victim,TYPE_UNDEFINED);
        }
        continue;
        }
      
      /* now check the NPC cases */
        
      if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
    
        {
          if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

           ||   (IS_NPC(rch) && rch->group && rch->group == ch->group)

           ||   (IS_NPC(rch) && rch->race == ch->race 
             && IS_SET(rch->off_flags,ASSIST_RACE))

           ||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
             &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
                   ||  (IS_EVIL(rch)    && IS_EVIL(ch))
                   ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch)))) 

           ||   (rch->pIndexData == ch->pIndexData 
             && IS_SET(rch->off_flags,ASSIST_VNUM))
           
           ||    (IS_NPC(rch) && (ch->pet == rch)))

        {
          CHAR_DATA *vch;
          CHAR_DATA *target;
          int number;

          if (number_bits(1) == 0)
            continue;
        
          target = NULL;
          number = 0;
          for (vch = ch->in_room->people; vch; vch = vch->next)
            {
              if (can_see(rch,vch)
              &&  is_same_group(vch,victim)
              &&  number_range(0,number) == 0)
            {
              target = vch;
              number++;
            }
            }

          if (target != NULL)
            {
              do_function(rch,&do_emote,"screams and attacks!");
              SET_BIT(rch->active_flags, ACTIVE_HAS_ATTACKED);
              multi_hit(rch,target,TYPE_UNDEFINED);
            }
        }    
        }
    }
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
  int     chance;
  bool second_dual = FALSE;

  /* decrement the wait */
  if (ch->desc == NULL)
    ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);

  if (ch->desc == NULL)
    ch->daze = UMAX(0,ch->daze - PULSE_VIOLENCE); 


  /* no attacks for stunnies -- just a check */
  if (ch->position < POS_RESTING)
    return;

  if (IS_NPC(ch))
    {
      mob_hit(ch,victim,dt);
      return;
    }
 
  one_hit( ch, victim, dt, FALSE );


  if (ch->fighting != victim)
    return;


  if (IS_AFFECTED(ch,AFF_HASTE))
    one_hit(ch,victim,dt, FALSE);

  if (is_spirit_affected(ch) && !IS_OUTSIDE(ch) && (dt != gsn_circle)
      && (dt != gsn_backstab))
    spirit_hit(ch, victim);
  
  if ( ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle)
    return;

  second_dual = FALSE;

  if (get_eq_char (ch, WEAR_SECONDARY))
  {
    if (number_percent() < get_skill(ch,gsn_second))
    {
        one_hit( ch, victim, dt, TRUE );
        second_dual = TRUE;
    }

    check_improve(ch,gsn_second,TRUE,2);
    if ( ch->fighting != victim )
        return;
  }

  //Check for Second Attack
  chance = get_skill(ch,gsn_second_attack)/2;

  if (IS_AFFECTED(ch,AFF_SLOW))
    chance /= 2;

  if ( number_percent( ) < chance )
  {
    one_hit( ch, victim, dt, FALSE );
    check_improve(ch,gsn_second_attack,TRUE,3);

    if ( ch->fighting != victim )
        return;
  }

  //Check For Double Shot
  chance = get_skill(ch,gsn_double_shot)/2;

  if (IS_AFFECTED(ch,AFF_SLOW))
		chance /= 2;

  if (number_percent() < chance )
  {
		one_hit( ch, victim, dt, FALSE );
		check_improve(ch,gsn_double_shot,TRUE,3);

		if ( ch->fighting != victim )
			return;
	}

	//Check For Third Attack
  chance = get_skill(ch,gsn_third_attack)/3;

  if (IS_AFFECTED(ch,AFF_SLOW))
    chance = 0;;

  if ( number_percent( ) < chance )
  {
    one_hit( ch, victim, dt, FALSE );

    if (get_skill(ch,gsn_second_attack) > 89)
      check_improve(ch,gsn_third_attack,TRUE,4);

    if ( ch->fighting != victim )
      return;
  }

  chance = get_skill(ch,gsn_fourth_attack)/3;

  if (IS_AFFECTED(ch,AFF_SLOW))
    chance = 0;;

  if ( number_percent( ) < chance )
  {
    one_hit( ch, victim, dt, FALSE );

    if ( ( get_skill(ch,gsn_third_attack ) > 89)
    && ( get_skill(ch,gsn_second_attack ) > 89) )
      check_improve(ch,gsn_fourth_attack,TRUE,3);

    if ( ch->fighting != victim )
      return;
  }

  chance = get_skill(ch,gsn_fifth_attack)/3;

  if (IS_AFFECTED(ch,AFF_SLOW))
    chance = 0;

  if ( number_percent( ) < chance )
  {
    one_hit( ch, victim, dt, FALSE );

    if ((get_skill(ch,gsn_fourth_attack) > 89)
    &&  (get_skill(ch,gsn_third_attack ) > 89)
    &&  (get_skill(ch,gsn_second_attack) > 89))
      check_improve(ch,gsn_fifth_attack,TRUE,3);

    if ( ch->fighting != victim )
      return;
  }

  if (second_dual)
  {
    //Dual Attack
    chance = get_skill(ch,gsn_dual_attack)/3;

    if (IS_AFFECTED(ch,AFF_SLOW))
      chance = 0;

    if ( number_percent( ) < chance )
    {
        one_hit( ch, victim, dt, TRUE );
        check_improve(ch,gsn_dual_attack,TRUE,3);

        if ( ch->fighting != victim )
          return;
    }

    //Triple Attack
    chance = get_skill(ch,gsn_triple_attack)/3;

    if (IS_AFFECTED(ch,AFF_SLOW))
      chance = 0;

    if ( number_percent( ) < chance )
    {
      one_hit( ch, victim, dt, TRUE );
      check_improve(ch,gsn_triple_attack,TRUE,3);

      if ( ch->fighting != victim )
        return;
    }

  }
  return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
  int chance,number;
  CHAR_DATA *vch, *vch_next;

  one_hit(ch,victim,dt, FALSE);

  if (ch->fighting != victim)
    return;

  /* Area attack -- BALLS nasty! */
 
  if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
      for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next;
      if ((vch != victim && vch->fighting == ch))
        one_hit(ch,vch,dt, FALSE);
    }
    }

  if (IS_AFFECTED(ch,AFF_HASTE) 
      ||  (IS_SET(ch->off_flags,OFF_FAST) && !IS_AFFECTED(ch,AFF_SLOW)))
    one_hit(ch,victim,dt, FALSE);

  if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle)
    return;

  chance = get_skill(ch,gsn_second_attack)/2;

  if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
    chance /= 2;

  if (number_percent() < chance)
    {
      one_hit(ch,victim,dt, FALSE);
      if (ch->fighting != victim)
    return;
    }

  chance = get_skill(ch,gsn_third_attack)/4;

  if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
    chance = 0;

  if (number_percent() < chance)
    {
      one_hit(ch,victim,dt, FALSE);
      if (ch->fighting != victim)
    return;
    } 

  chance = get_skill(ch,gsn_fourth_attack)/6;

  if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
    chance = 0;

  if (number_percent() < chance)
    {
      one_hit(ch,victim,dt, FALSE);
      if (ch->fighting != victim)
    return;
    } 

  /* oh boy!  Fun stuff! */

  if (ch->wait > 0)
    return;

  number = number_range(0,2);

  if (number == 1 && IS_SET(ch->act,ACT_MAGE))
    {
      /*  { mob_cast_mage(ch,victim); return; } */ ;
    }

  if (number == 2 && IS_SET(ch->act,ACT_CLERIC))
    {    
      /* { mob_cast_cleric(ch,victim); return; } */ ;
    }

  /* now for the skills */

  number = number_range(0,10);

  switch(number) 
    {
    case (0) :
      if (IS_SET(ch->off_flags,OFF_BASH))
    do_function(ch,&do_bash,"");
      break;

    case (1) :
      if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
    do_function(ch,&do_berserk,"");
      break;


    case (2) :
      if (IS_SET(ch->off_flags,OFF_DISARM) 
      || (get_weapon_sn(ch) != gsn_hand_to_hand
          && (IS_SET(ch->act,ACT_WARRIOR)
          ||  IS_SET(ch->act,ACT_THIEF))))
    do_function(ch,&do_disarm,"");
      break;

    case (3) :
      if (IS_SET(ch->off_flags,OFF_KICK))
    do_function(ch,&do_kick,"");
      break;

    case (4) :
      if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
    do_function(ch,&do_dirt,"");
      break;

    case (5) :
      if (IS_SET(ch->off_flags,OFF_TAIL))
    {
      /* do_tail(ch,"") */ ;
    }
      break; 

    case (6) :
      if (IS_SET(ch->off_flags,OFF_TRIP))
    do_function(ch,&do_trip,"");
      break;

    case (7) :
      if (IS_SET(ch->off_flags,OFF_CRUSH))
    {
      /* do_crush(ch,"") */ ;
    }
      break;
    case (8) :
      if (IS_SET(ch->off_flags,OFF_BACKSTAB))
    {
      do_function(ch,&do_backstab,"");
    }
    case (9) :
      if (IS_SET(ch->off_flags,OFF_FLYING))
    do_function(ch,&do_flying_kick,"");
      break;
    case (10) :
      if (IS_SET(ch->off_flags,OFF_BITE))
    do_function(ch,&do_chomp,"");
      break;
    }
}
    

/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary )
{
  OBJ_DATA *wield, *t_obj;
  int victim_ac;
  int thac0;
  int thac0_00;
  int tdt;
  int thac0_32;
  int dam;
  int diceroll;
  int sn,skill;
  int dam_type;
  int hit_roll;
  bool result=FALSE;
  

  sn = -1;
  /* just in case */
  if (victim == ch || ch == NULL || victim == NULL)
    return;

  /*
   * Can't beat a dead char!
   * Guard against weird room-leavings.
   */
  if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
    return;

  /* Look for < 0 movement. */
  if ( !check_movement( ch, victim, TYPE_UNDEFINED, FALSE ) )
    return;

// Taeloch's new mounting code!
// maybe we can have a skill that allows you to fight while on a mount
  if ( victim->mount != NULL )
  {
    printf_to_char(victim, "%s knocks you off of %s.\n\r",
      ( IS_NPC( ch ) ? ch->short_descr : ch->name ),
      ( IS_NPC( victim->mount ) ? victim->mount->short_descr : victim->mount->name ) );
  }

  /*
   * Figure out the type of damage message.
   */
  if (!secondary)
    wield = get_eq_char( ch, WEAR_WIELD );
  else
    wield = get_eq_char( ch, WEAR_SECONDARY );

  if ( dt == TYPE_UNDEFINED )
  {
      dt = TYPE_HIT;

      if ( wield != NULL && wield->item_type == ITEM_WEAPON )
          dt += wield->value[3];
      else
      {
        if ( ch->gameclass == cMystic )
        {
          dt += get_style_dam_type(ch);
        }
        else
        {
            dt += ch->dam_type;
        }
      }
  }

  if (dt < TYPE_HIT)
  { 
    if (wield != NULL)
      dam_type = attack_table[wield->value[3]].damage;
    else
    {
      if (ch->martial_style)
      {
          dam_type = get_style_dam_type(ch);
        //bugf( "Damage is %d, Style is %d", dam_type, ch->martial_style );
      }
      else
      {
          dam_type = attack_table[ch->dam_type].damage;
        //bugf( "Damage is %d, Style is %d", dam_type, ch->martial_style );
      }
    }
  }
  else
    dam_type = attack_table[dt - TYPE_HIT].damage;

  if (dam_type == TYPE_UNDEFINED)
    dam_type = DAM_BASH;

  /* get the weapon skill */
  if (!secondary)
  {
    sn = get_weapon_sn(ch);
  }
  else
  {
    sn = get_weapon_sn_2(ch);
  }

  skill = 20 + get_weapon_skill(ch,sn);

  /*
   * Calculate to-hit-armor-class-0 versus armor.
   */
  if ( IS_NPC(ch) )
  {
      thac0_00 = 20;
      thac0_32 = -4;   /* as good as a thief */ 

      if ( IS_SET( ch->act, ACT_WARRIOR ) ) thac0_32 = -10;
      else if ( IS_SET( ch->act, ACT_THIEF ) ) thac0_32 = -4;
      else if ( IS_SET( ch->act, ACT_CLERIC ) ) thac0_32 = 2;
      else if ( IS_SET( ch->act, ACT_MAGE ) ) thac0_32 = 6;
  }
  else
  {
      thac0_00 = class_table[ch->gameclass].thac0_00;
      thac0_32 = class_table[ch->gameclass].thac0_32;
  }

  thac0  = interpolate( ch->level, thac0_00, thac0_32 );

  if (thac0 < 0)
    thac0 = thac0/2;

  if (thac0 < -5)
    thac0 = -5 + (thac0 + 5) / 2;

  hit_roll = GET_HITROLL( ch );

  if ( hit_roll > 20 )
    hit_roll = 20 + (hit_roll - 20) / 2;

  thac0 -= hit_roll * skill/100;
  thac0 += 5 * ( 100 - skill ) / 100;

  if (dt == gsn_backstab)
    thac0 -= 10 * (100 - get_skill(ch,gsn_backstab));

  if (dt == gsn_circle)
    thac0 -= 10 * (100 - get_skill(ch,gsn_circle));

  switch(dam_type)
  {
    case ( DAM_PIERCE ) : victim_ac = GET_AC( victim, AC_PIERCE ) / 10;    break;
    case ( DAM_BASH )   : victim_ac = GET_AC( victim, AC_BASH ) / 10;    break;
    case ( DAM_SLASH )  : victim_ac = GET_AC( victim, AC_SLASH ) / 10;    break;
    default             : victim_ac = GET_AC( victim, AC_EXOTIC ) / 10;    break;
  }; 

  /*
  Must be based on 50 lvl system
  800 = 28
  600 = 24
  400 = 20
  200 = 16
  */
//  if (victim_ac < -15)
//    victim_ac = (victim_ac + 15) / 5 - 15;
  
  if ( !can_see( ch, victim ) )
    victim_ac -= 4;
  
  if ( victim->position < POS_FIGHTING)
    victim_ac += 4;
  
  if (victim->position < POS_RESTING)
    victim_ac += 6;

  if ( wield != NULL )
    if ( wield->famowner != NULL )
      if ( !str_cmp( wield->famowner, ch->name ) )
        victim_ac += 4;
  
  /*
   * The moment of excitement!
   */
  //while ( ( diceroll = number_bits( 5 ) ) >= 20 ) ;
  diceroll = number_range(0, 19);

  if ( diceroll == 0
  ||   ( diceroll != 19
  &&     diceroll < thac0 - victim_ac ) )
  {
      /* Miss. */
    if ( ( ch->gameclass == cMystic )
    && ( wield == NULL ) ) // give correct damtype for mystic failures
      damage( ch, victim, 0, martial_style_dt(ch), get_style_dam_type(ch), TRUE, FALSE );
    else
      damage( ch, victim, 0, dt, dam_type, TRUE, FALSE );

    tail_chain( );
    return;
  }

  /*
   * Hit.
   * Calc damage.
   */
  if ( IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
  {
    if (!ch->pIndexData->new_format)
    {
        dam = number_range( ch->level / 2, ch->level * 3 / 2 );

        if ( wield != NULL )
          dam += dam / 2;
    }
    else
      dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);

    dam += GET_DAMROLL(ch);/* * UMIN(100,skill) /100;*/
  }
  else
  {
      if (sn != -1)
        check_improve(ch,sn,TRUE,5); 

      if ( wield )
      {
        if (wield->pIndexData->new_format)
            dam = dice(wield->value[1],wield->value[2]) * skill/100;
        else
            dam = number_range( wield->value[1] * skill/100, wield->value[2] * skill/100);

          if (get_eq_char(ch,WEAR_SHIELD) == NULL)  /* no shield = more */
            dam = dam * 11/10;

        /* sharpness! */
        if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
        {
          int percent;

          if ((percent = number_percent()) <= (skill / 8))
            dam = 2 * dam + (dam * 2 * percent / 100);
        }
        /* sharpness! */
        if (IS_WEAPON_STAT(wield,WEAPON_VORPAL))
        {
          int percent;

          if ((percent = number_percent()) <= (skill / 10))
            dam = 2 * dam + (dam * 2 * percent / 50);
        }

        dam += GET_DAMROLL(ch);/* * UMIN(100,skill) /100;*/
    }
    else
    {
      //bugf(ch,"INITIAL DT: %d ",dt);
      tdt = dt;
      dam = hand_skill_damage(ch, victim, &tdt);

      if (tdt != 0)
        dt = tdt;
      //bugf(ch,"RESULT DT: %d\n\r",dt);
    }        
  }
  //bugf(ch,"POINT B  DT = %d:\n\r",dt);

  /*
   * Bonuses.
   */
  if ( get_skill(ch,gsn_enhanced_damage) > 0 )
  {
      diceroll = number_percent();

      if (diceroll <= get_skill(ch,gsn_enhanced_damage))
      {
        check_improve(ch,gsn_enhanced_damage,TRUE,6);
        dam += 2 * ( dam * diceroll/300);
      }
  }

  if ( get_skill(ch,gsn_dagger_twist) > 0 )
  {
    t_obj = get_eq_char(ch, WEAR_WIELD);

    if (t_obj)
    {
      if (t_obj->item_type == ITEM_WEAPON)
      {
        if (t_obj->value[0] == WEAPON_DAGGER)
        {
          diceroll = number_percent();

          if (diceroll <= get_skill(ch,gsn_dagger_twist))
          {
            check_improve(ch,gsn_dagger_twist,TRUE,3);
            dam += 2 * ( dam * diceroll/300 * 1.5);
          }
        }
      }
    }
  }

  if (ch->gameclass == cBarbarian)
  {
      if ((t_obj = get_eq_char(ch, WEAR_WIELD)) != NULL)
        if (t_obj->item_type == ITEM_WEAPON) 
          if (IS_WEAPON_STAT(t_obj,WEAPON_TWO_HANDS)) 
            if ((t_obj = get_eq_char(ch, WEAR_SECONDARY)) != NULL)
              if (t_obj->item_type == ITEM_WEAPON) 
                if (IS_WEAPON_STAT(t_obj,WEAPON_TWO_HANDS)) 
                {
                    dam += 2 * ( dam * diceroll/300 * 1.5);
                }
  }


  if ( !IS_AWAKE(victim) )
    dam *= 2;
  else if (victim->position < POS_FIGHTING)
    dam = dam * 3 / 2;

  if ( dt == gsn_backstab && wield != NULL)
  {
      if ( wield->value[0] != 2 )
          dam *= 2 + (ch->level / 10); 
      else 
          dam *= 2 + (ch->level / 8);
  }

  if ( dt == gsn_circle && wield != NULL)
  {
    if ( wield->value[0] != 2 )
      dam *= 1+ (ch->level/14);
    else
      dam *= 1+(ch->level/12);
  }

// if victim is vulnerable to iron, increase damage by 10%
  if ( ( wield )
  &&   (!str_cmp(capitalize(wield->pIndexData->material),"Iron"))
  &&   ( IS_SET( victim->vuln_flags, VULN_IRON ) ) )
    dam += (dam*.1);

  if ( ( wield )
  &&   (!str_cmp(capitalize(wield->pIndexData->material),"Wood"))
  &&   ( IS_SET( victim->vuln_flags, VULN_WOOD ) ) )
    dam += (dam*.1);

  if ( ( wield != NULL )
  &&   ( is_affected(ch,skill_lookup("deathgrip"))))
  {
    if (victim->alignment > 700)
      dam = (110 * dam) / 100;

    else if (victim->alignment > 350)
      dam = (105*dam) / 100;
    else
      dam = (102*dam) / 100;
  }

  if ( dam <= 0 )
    dam = 1;
  if ( ( dt != gsn_stake)
  &&   dt != gsn_backstab)
  {
    if ( !check_counter( ch, victim, dam, dt ) )
    {
      //bugf(ch,"ONEHIT #9: DT = %d\n",dt);
      result = damage( ch, victim, dam, dt, dam_type, TRUE, FALSE );
    }
    else
    {
      //bugf("ONEHIT  COUNTERED EXIT: DT = %d\n",dt);
      return;
    }
  }
  else
  { 
    //bugf("ONEHIT #10: DT = %d\n",dt);
    result = damage( ch, victim, dam, dt, dam_type, TRUE, FALSE );
  }
  //bugf("POINT C  DT = %d:\n\r",dt);

  /* but do we have a funky weapon? */
  if ( result )
  { 
    int dam;

    //Entry Point to Check for wield == NULL
    if ( wield == NULL )
    {
      if (ch->fighting != NULL)
        if ( IS_SET( ch->spell_aff, SAFF_LIFE_DRAIN ) )
        {
            act_spam("You draw {rlife{x from $N.",
                ch,NULL,victim,TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
            act_spam("You feel $n draw {rlife {xfrom you.",
                ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_OEFFECTS );
            act_spam("$n draws {rlife {xfrom $N.",
                ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS );
        
            dam = number_range(1, ch->level / 5 + 1);
            damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE, FALSE);

            ch->hit += dam/2;

            if (ch->hit >= GET_HP(ch))
               ch->hit = GET_HP(ch);
        }

      if (ch->fighting != NULL)
        if  ( IS_SET( ch->spell_aff, SAFF_MANA_DRAIN ) )
        {
            if (victim->mana > 0)
            {
              act_spam("You draw {gmagic{x from $N.",
                  ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
              act_spam("You feel $n draw {gmagic{x from you.",
                  ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_OEFFECTS );
              act_spam("$n draws {gmagic{x from $N.",
                  ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS );
            }

            dam = number_range(1, ch->level / 5 + 1);
            damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE, FALSE);

            if (victim->mana < dam)
            {
                dam = victim->mana;
                victim->mana = 0;
            }
             else
                victim->mana -= dam;

            if (victim->mana > 0)
              ch->mana += dam/2;
            if (ch->mana >= GET_MANA(ch))
                ch->mana = GET_MANA(ch);
        
        }

      if (ch->fighting != NULL)
        if (  IS_SET( ch->spell_aff, SAFF_FLAME_SHROUD ) )
        {
            dam = number_range(1, ch->level / 4 + 1);

            act_spam( "You {rbu{Rr{rn{x $N with your hands.",
                ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
            act_spam( "$n {rsea{Rr{rs{x your flesh.",
                ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_OEFFECTS );
            act_spam( "$n {rsea{Rr{rs{x $N's flesh with $s hands.",
                ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS );

            fire_effect( (void *) victim, ch->level/2,dam, TARGET_CHAR );
            damage(ch,victim,dam,0,DAM_FIRE,FALSE, FALSE);
        }

      if (ch->fighting != NULL)
        if (  IS_SET( ch->spell_aff, SAFF_ICE_SHROUD ) )
        {
            dam = number_range(1, ch->level / 4 + 1);
            act_spam( "You {Bf{bre{ce{Cz{xe $N with your hands.",
                ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
            act_spam( "$n {Bf{bre{ce{Cz{wes{x your flesh.",
                ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_OEFFECTS );
            act_spam( "$n {Bf{bre{ce{Cz{wes{x $N's flesh with $s hands.",
                ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS );

            cold_effect( (void *) victim, ch->level/2,dam, TARGET_CHAR );
            damage(ch,victim,dam,0,DAM_COLD,FALSE, FALSE);
        }

      if (ch->fighting != NULL)
        if (  IS_SET( ch->spell_aff, SAFF_ELECTRIC_SHROUD ) )
        {
            dam = number_range(1, ch->level / 5 + 1);

            act_spam( "You {ws{Wh{Yo{yck{x $N with your hands.",
                ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
            act_spam( "$n {ws{Wh{Yo{yck{xs your flesh.",
                ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_OEFFECTS  );
            act_spam( "$n {ws{Wh{Yo{yck{xs $N's flesh with $s hands.",
                ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS  );

            shock_effect( (void *) victim, ch->level/2,dam, TARGET_CHAR );
            damage(ch,victim,dam,0,DAM_LIGHTNING, FALSE, FALSE);
        }
        
      if (ch->fighting != NULL)
        if (  IS_SET( ch->spell_aff, SAFF_POISON_SHROUD ) )
        {
            if ( !saves_spell( ch->level / 2, victim, DAM_POISON ) )
            {

                AFFECT_DATA af;

                send_to_char(
                    "You feel {gpo{ci{Gs{gon{x coursing through your veins.\n\r",
                        victim);
                act_spam( "$N is {gpo{ci{Gs{gon{ce{gd{x by your hands.",
                    ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
                act_spam("$N is {gpo{ci{Gs{goned{x by $n.",
                    ch, NULL, victim, TO_ROOM, POS_RESTING, NOSPAM_OEFFECTS );

                af.where     = TO_AFFECTS;
                af.type      = gsn_poison;
                af.level     = ch->level * 3/4;
                af.duration  = ch->level / 4;
                af.location  = APPLY_STR;
                af.modifier  = -1;
                af.bitvector = AFF_POISON;
                affect_join( victim, &af );
            }
        }

      if (ch->fighting != NULL)
        if (  IS_SET( ch->affected_by, AFF_AQUA_REGIA ) )
        {
            dam = number_range(1, ch->level / 4 + 1);

            act_spam( "$N is burned by your {cac{gi{cd{Ci{cc{x shroud.",
                ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
            act_spam( "You are burned by $n's {cac{gi{cd{Ci{cc{x shroud.",
                ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_OEFFECTS );
            act_spam( "$n burns $N with $s {cac{gi{cd{Ci{cc{x shroud.",
                ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS );

            acid_effect( victim, ch->level,dam, TARGET_CHAR );
            damage(ch,victim,dam,0,DAM_ACID,FALSE, FALSE);
        }

        return;

    }

// TOX change starts here, up to #if TESTIT
    if (ch->fighting != NULL)
      if (  IS_WEAPON_STAT(wield,WEAPON_POISON))
      {
        int level,duration;
        AFFECT_DATA *poison, af;

        if ((poison = affect_find(wield->affected,gsn_poison)) == NULL)
          level = wield->level;
        else
          level = poison->level;
  
        if (!saves_spell(level / 2,victim,DAM_POISON)) 
        {
          if (poison) {
            switch (poison->modifier)
            {
              case TOX_VIRAL:
                duration = (level * 3) / 2;
                break;
              case TOX_ELEMENTAL:
              case TOX_FUNGAL:
                duration = (level * 2) / 3;
                break;
              case TOX_ACIDIC:
              case TOX_HALLUCINOGENIC:
                duration = (level / 3);
                break;
              case TOX_NEUROTOXIC:
              case TOX_NECROTIC:
              default:
                duration = level / 2;
                break;
            }
          }
          else
            duration = level / 2;

          // actual affect:
          af.where     = TO_AFFECTS;
          af.type      = gsn_poison;
          af.bitvector = AFF_POISON;
          af.level     = level * 3/4;
          af.duration  = duration;
          if (poison)
          {
            switch (poison->modifier)
            {
              case TOX_ELEMENTAL:
                send_to_char("You feel {gto{cx{Gi{gns{x coursing through your veins.\n\r", victim);
                act_spam("$n is {gpo{ci{Gs{goned{x by the elemental toxin on $p.", victim,wield,NULL,TO_ROOM, POS_RESTING, NOSPAM_OEFFECTS );
                af.location  = APPLY_STR;
                af.modifier  = -3;
                break;
              case TOX_FUNGAL:
                send_to_char("You feel {gto{cx{Gi{gns{x coursing through your veins.\n\r", victim);
                act_spam("$n is {gpo{ci{Gs{goned{x by the fungus on $p.", victim,wield,NULL,TO_ROOM, POS_RESTING, NOSPAM_OEFFECTS );
                af.location  = APPLY_AC;
                af.modifier  = 0-level;
                break;
              case TOX_ACIDIC:
                send_to_char("You feel {ca{Cc{ci{gd{x coursing through your veins.\n\r", victim);
                act_spam("$n is {gpo{ci{Gs{goned{x by the acid on $p.", victim,wield,NULL,TO_ROOM, POS_RESTING, NOSPAM_OEFFECTS );
                af.location  = APPLY_DEX;
                af.modifier  = -2;
                break;
              case TOX_NECROTIC:
                send_to_char("You feel an intense burning sensation in the wound.\n\r", victim);
                act_spam("$n is {gpo{ci{Gs{goned{x by the necrotoxin on $p.", victim,wield,NULL,TO_ROOM, POS_RESTING, NOSPAM_OEFFECTS );
                af.location  = APPLY_HIT;
                af.modifier  = (0-level)/2;
                break;
              case TOX_VIRAL:
                // viruses won't show up at first
                act_spam("$n is {gpo{ci{Gs{goned{x by the virus on $p.", victim,wield,NULL,TO_ROOM, POS_RESTING, NOSPAM_OEFFECTS );
                af.location  = APPLY_STR;
                af.modifier  = -2;
                break;
              case TOX_NEUROTOXIC:
                // neurotoxins won't show up at first
                act_spam("$n is {gpo{ci{Gs{goned{x by the neurotexin on $p.", victim,wield,NULL,TO_ROOM, POS_RESTING, NOSPAM_OEFFECTS );
                af.location  = APPLY_INT;
                af.modifier  = -2;
                break;
              case TOX_HALLUCINOGENIC:
                send_to_char("You feel temporarily lightheaded.\n\r", victim);
                act_spam("$n is {gpo{ci{Gs{goned{x by the hallucinogen on $p.", victim,wield,NULL,TO_ROOM, POS_RESTING, NOSPAM_OEFFECTS );
                af.location  = APPLY_HITROLL;
                af.modifier  = 0-(level/5);
                break;
              default: // traditional venom
                send_to_char("You feel {gpo{ci{Gs{gon{x coursing through your veins.\n\r", victim);
                act_spam("$n is {gpo{ci{Gs{goned{x by the venom on $p.", victim,wield,NULL,TO_ROOM, POS_RESTING, NOSPAM_OEFFECTS );
                af.location  = APPLY_STR;
                af.modifier  = -1;
                break;
            }
          }
          else
          {
            send_to_char("You feel {gpo{ci{Gs{gon{x coursing through your veins.\n\r", victim);
            act_spam("$n is {gpo{ci{Gs{goned{x by the venom on $p.", victim,wield,NULL,TO_ROOM, POS_RESTING, NOSPAM_OEFFECTS );
            af.location  = APPLY_STR;
            af.modifier  = -1;
          }
          affect_to_char( victim, &af );

          // identifier affect -- MUST BE LAST TO BE IDed:
          af.where     = TO_AFFECTS;
          af.type      = gsn_poison;
          af.bitvector = AFF_POISON;
          af.level     = level * 3/4;
          af.duration  = duration;
          af.location  = APPLY_NONE;
          if (poison)
            af.modifier  = poison->modifier;
          else
            af.modifier  = TOX_VENOM;
          affect_to_char( victim, &af );
        }

        /* weaken the poison if it's temporary */
/* Causes poison to wear off in 1 tick: every _HIT_ reduces time
        if (poison != NULL)
        {
          poison->level = UMAX(0,poison->level - 2);
          poison->duration = UMAX(0,poison->duration - 1);
  
          //if (poison->level == 0 || poison->duration == 0)
          //  act("The {gpo{ci{Gs{gon{x on $p has worn off.",ch,wield,NULL,TO_CHAR);
        }
*/
      }

#if TESTIT
      printf_to_char(ch,"ONEHIT BELOW initial expectations.: DT = %d\n",dt);
#endif
    if (ch->fighting != NULL)
    if (IS_WEAPON_STAT(wield,WEAPON_MANA_DRAIN) || IS_SAFFECTED(ch, SAFF_MANA_DRAIN))
    {
      if (IS_CLASS_INQUISITOR(ch) 
         && !IS_WEAPON_STAT(wield, WEAPON_MANA_DRAIN)
         && (( !IS_GOOD(ch) || IS_GOOD(victim) ) || number_percent() >= 10))
      {}
      else
      {
          dam = number_range(1, wield->level / 5 + 1);

          if (IS_WEAPON_STAT(wield, WEAPON_MANA_DRAIN)) //|| are they affected by shroud && using their hands.
          {
            if (victim->mana > 0)
            {
              act_spam("$p draws {gmagic{x from $N.",
                ch, wield, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS);
                act_spam("$p draws {gmagic{x from $N.",
                  ch,wield,victim,TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS);
                act_spam("You feel $p drawing your {gmagic{x away.",
                  victim,wield,NULL,TO_CHAR, POS_RESTING, NOSPAM_OEFFECTS);
            }
          }
          else
          {
            if (victim->mana > 0)
            {
              act_spam("You draw {gmagic{x from $N.",
                ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS);
              act_spam("You feel $n draw {gmagic{x from you.",
                ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_OEFFECTS);
              act_spam("$n draws {gmagic{x from $N.",
                ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS);
            }
          }

          /*damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE, FALSE);*/
          if (victim->mana < dam)
          {
            dam = victim->mana;
            victim->mana = 0;
          }
          else
            victim->mana -= dam;

        if (victim->mana > 0)
          ch->mana += dam/2;
          //if (number_percent() < ch->level / 2) 
            //align_change(ch, number_range(1,5), TRUE, FALSE);
          if (ch->mana >= GET_MANA(ch))
            ch->mana = GET_MANA(ch);
        }
        }

    if (ch->fighting != NULL)
    if (IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC) || IS_SAFFECTED(ch, SAFF_LIFE_DRAIN))
    {
          if ( IS_CLASS_OCCULTIST(ch)
          &&   !IS_WEAPON_STAT(wield, WEAPON_MANA_DRAIN)
          &&   (IS_GOOD(ch)))
          {}
          else
          {
            dam = number_range(1, wield->level / 5 + 1);

            if (IS_WEAPON_STAT(wield, WEAPON_VAMPIRIC))
            {
              act_spam( "$p draws {rlife{x from $N.",
                ch, wield, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
              act_spam( "$p draws {rlife {xfrom $N.",
                ch,wield,victim,TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS);
              act_spam( "You feel $p drawing your {rlife{x away.",
                victim,wield,NULL,TO_CHAR, POS_RESTING, NOSPAM_OEFFECTS);
            }
            else 
            {
                act_spam("You draw {rlife {xfrom $N.",
                    ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS);
                act_spam("You feel $N draw {rlife {xfrom you.",
                    ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_OEFFECTS);
                act_spam("$n draws {rlife{x from $N.",
                    ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS);
            }

          damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE, FALSE);
          //if (number_percent() < ch->level / 2) 
            //align_change(ch, number_range(1,5), FALSE, FALSE);
          ch->hit += dam/2;
          if (ch->hit >= GET_HP(ch))
            ch->hit = GET_HP(ch);
           }
    }

  if (ch->fighting != NULL)
    if ( IS_WEAPON_STAT(wield,WEAPON_FLAMING))
      {
        dam = number_range(1,wield->level / 4 + 1);
      act_spam( "$N is {rbu{Rr{rn{Re{rd{x by $p.",
        ch, wield, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
        act_spam("$N is {rbu{Rr{rn{Re{rd{x by $p.",
        ch,wield,victim,TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS);
        act_spam("$p {rsea{Rr{rs{x your flesh.",
        victim,wield,NULL,TO_CHAR, POS_RESTING, NOSPAM_OEFFECTS);
        fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
        damage(ch,victim,dam,0,DAM_FIRE,FALSE, FALSE);
      }

  if ( IS_EVIL(victim) )
    if (ch->fighting != NULL)
      if ( IS_WEAPON_STAT(wield,WEAPON_HOLY))
      {
        dam = number_range(1,wield->level / 4 + 1);
        act_spam( "$N is {ybu{Wr{yned{x by the {Wlight{x of $p.",
          ch, wield, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
        act_spam( "$N is {ybu{Wr{yned{x by the {Wlight{x of $p.",
          ch,wield,victim,TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS);
        act_spam( "$p {ys{We{yars{x your flesh with its holy {Cen{cergy{x.",
          victim,wield,NULL,TO_CHAR, POS_RESTING, NOSPAM_OEFFECTS);
        //fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
        //no holy_effect, so increase damage a little
        dam += dam /2;
        damage(ch,victim,dam,0,DAM_HOLY,FALSE, FALSE);
      }

  if (ch->fighting != NULL)
    if (IS_WEAPON_STAT(wield,WEAPON_FROST))
      {
        dam = number_range(1,wield->level / 6 + 2);
        act_spam("$p {Bf{bre{ce{Cz{wes{x $N.",
          ch,wield,victim,TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS);
        act_spam("$p {Bf{bre{ce{Cz{wes{x $N.",
        ch,wield,victim,TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS);
        act_spam("The {Bf{bre{ce{Cz{Wi{wng{x bite of $p surrounds you with ice.",
          victim,wield,NULL,TO_CHAR, POS_RESTING, NOSPAM_OEFFECTS);
        cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
        damage(ch,victim,dam,0,DAM_COLD,FALSE, FALSE);
      }

  if (ch->fighting != NULL)
    if (IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
      {
        dam = number_range(1,wield->level/5 + 2);

      act_spam("$N is struck by {wli{Wgh{Yt{yn{Wi{yng{x from $p.",
        ch,wield,victim,TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS);
        act_spam("$N is struck by {wli{Wgh{Yt{yn{Wi{yng{x from $p.",
        ch,wield,victim,TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS);
        act_spam("You are {ws{Wh{Yo{yck{wed{x by $p.",
        victim,wield,NULL,TO_CHAR, POS_RESTING, NOSPAM_OEFFECTS );

        shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
        damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE, FALSE);
    }

  if (ch->fighting != NULL)
    if ( (is_affected(ch,skill_lookup("deathgrip") && wield != NULL )))
      {
        dam = number_range(1,wield->level / 10 + 4);

      act_spam("The evil power of $p torments $N.",
        ch, wield, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
        act_spam("The evil power of $p torments $N.",
        ch,wield,victim,TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS);
        act_spam("The evil power of $p torments you.",
        victim,wield,NULL,TO_CHAR, POS_RESTING, NOSPAM_OEFFECTS );
        damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE, FALSE);

        if ( !IS_NPC(victim) && ( victim->alignment > 0 )  )
          victim->mana -= (ch->level)/8;
      }
  }

  tail_chain( );
  return;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim, bool showflag)
{
  return(check_safe(ch,victim,FALSE,FALSE,showflag));
}
 
bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area, bool showflag )
{
  return(check_safe(ch,victim,area,TRUE,showflag));
}

/*
 * New version of this function by Sartan
 */
#define NEW_CHECK_KILLER
#ifdef NEW_CHECK_KILLER
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
  char buf[MAX_STRING_LENGTH];
  /*bool adrenalize = FALSE;
  AFFECT_DATA af;*/
  CHAR_DATA *rch, *rch_next;
  int num_attackers, num_defenders;
  AFFECT_DATA af;

  if ( IS_ARENA( ch ) || IS_ARENA( victim ) )
    return;

  /*
   * Follow charm thread to responsible character.
   * Attacking someone's charmed char is hostile!
   */

  if (ch == victim)
    return;
  
  while ( (IS_PET(ch) || IS_AFFECTED(ch, AFF_CHARM)) && ch->master != NULL )
    ch = ch->master;

  while ( victim->master && (IS_PET(victim) || IS_AFFECTED( victim, AFF_CHARM ) ))
    victim = victim->master;

  if (IS_NPC(victim) && victim->master == NULL)
    return;

  /* Should attacker be adrenalized? */

  for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
  {
    rch_next = rch->next_in_room;

    if (IS_NPC(rch))
        continue;

      if (IS_NPC(victim))
          continue;

      if ( is_same_group(rch, ch)
        &&   rch->group_fight != victim->group_num
        &&   victim->group_fight != rch->group_num
        &&   !IS_SAFFECTED(rch, SAFF_ADRENALIZE)
      &&   rch->clan
      //&& is_clan(rch)
        &&   rch->pcdata->old_char == FALSE
        &&   victim->pcdata->old_char == FALSE )   /*  <== rch & victim are non-pkable */
        {
          if ((victim->level < 51 && (rch->level < victim->level - 6))
        ||  (victim->level >= 51 && (rch->level < victim->level - (int)(victim->level * 0.1))))
          {
              send_to_char( "{RYou have become (( ADRENALIZED ))!{x\n\r", rch );
              affect_strip(rch, gsn_adrenalize);
              af.where     = TO_SPELL_AFFECTS;
              af.type      = gsn_adrenalize;
              af.level     = rch->level;
              af.duration  = 4;
              af.location  = APPLY_NONE;
              af.modifier  = 0;
              af.bitvector = SAFF_ADRENALIZE;
              affect_to_char( rch, &af );
        }
      }
  }

//  if (ch->in_room->sector_type != SECT_CITY)
//    return;



  /*
   * Charm-o-rama.
   */
  if ( IS_SET(ch->affected_by, AFF_CHARM) && ( ch->master == NULL ))
  {

      bugf("Check_killer: %s bad AFF_CHARM",
           IS_NPC(ch) ? ch->short_descr : ch->name );
      affect_strip( ch, gsn_charm_person );
      REMOVE_BIT( ch->affected_by, AFF_CHARM );
      return;
     /*  stop_follower( ch );
         return;
     */
    }


  if ( ch->group_fight != victim->group_num
  &&   victim->group_fight != ch->group_num
  &&  !IS_NPC(ch) )
  {
    num_attackers = 0;
    num_defenders = 0;
    get_size_fight_groups(ch, victim, &num_attackers, &num_defenders);

    if (num_attackers > 1 && num_defenders > 1 && !URANGE( -2, num_attackers - num_defenders, 2) )
        return;

    for (rch = player_list; rch; rch = rch->next_player)
      {
        if (  rch->group_fight == ch->group_fight
          && !IS_SET(rch->act, PLR_KILLER)
          && !IS_SET(rch->act, PLR_VIOLENT)
          && !IS_NPC(rch)
          && !all_flags_found(victim)
          && is_same_group(ch, rch)
      && !is_same_group(victim, rch)
//      && rch != victim
//      && ( victim->master && victim->master != rch )
          && (rch->clan) )
        {
          send_to_char( "{R*** You are now considered VIOLENT!!{x ***{x\n\r", rch );
          SET_BIT(rch->act, PLR_VIOLENT);
          mprintf(sizeof(buf), buf,
          "{R%s{x is attempting to murder {R%s{x at {g%s {x[{g%d{x]",
            ch->name,
            victim->name,
            victim->in_room->name,
            victim->in_room->vnum );
          wiznet(buf,ch,NULL,WIZ_MURDER,0,0);
          save_char_obj( ch , FALSE);
        }
      }
  }
}

bool all_flags_found(CHAR_DATA *ch)
{
  CHAR_DATA *rch, *rch_next;

  for (rch = player_list; rch != NULL; rch = rch_next)
  {
    rch_next = rch->next_player;

    if ( is_same_group(rch, ch)
      &&   rch->in_room == ch->in_room
      &&   !IS_SET(rch->act, PLR_KILLER)
      &&   !IS_SET(rch->act, PLR_VIOLENT)
      &&   !IS_SET(rch->act, PLR_THIEF) )
        return FALSE;
  }

  return TRUE;
}

void get_size_fight_groups(CHAR_DATA *ch, CHAR_DATA *victim, int *num_attackers,
               int *num_defenders)
{
  CHAR_DATA *rch, *rch_next;

  if (ch->leader != NULL && ch->leader->in_room != ch->in_room)
    change_group_leader(ch->leader, ch);
  if (victim->leader != NULL && victim->leader->in_room != victim->in_room)
    change_group_leader(victim->leader, victim);

  for (rch = player_list; rch != NULL; rch = rch_next)
  {
      rch_next = rch->next_player;
      if (is_same_group(ch, rch) && is_clan(rch) )
      {
          if (ch->in_room == rch->in_room && rch->pcdata->old_char == FALSE)
          {
              (*num_attackers)++;
              rch->group_fight = victim->group_num;
          }
          else
          {
            if ( ch != rch )
            {
              stop_follower(rch);
              do_function(rch, &do_group,rch->name);
            }
          }
      }
      if (is_same_group(victim, rch) && is_clan(rch) )
      {
          if (victim->in_room == rch->in_room && rch->pcdata->old_char == FALSE)
          {
              (*num_defenders)++;
          }
          else
          {
            if ( victim != rch )
            {
              stop_follower(rch);
              do_function(rch, &do_group,rch->name);
            }
          }
      }
  }
}
#else
/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
  char buf[MAX_STRING_LENGTH];
  bool adrenalize = FALSE;
  AFFECT_DATA af;

  if ( IS_ARENA( ch ) || IS_ARENA( victim ) )
    return;

  /*
   * Follow charm thread to responsible character.
   * Attacking someone's charmed char is hostile!
   */

  while ( (IS_PET(ch) || IS_AFFECTED(ch, AFF_CHARM)) && ch->master != NULL )
    ch = ch->master;

  while ( victim->master && (IS_PET(victim) || IS_AFFECTED( victim, AFF_CHARM ) ))
    victim = victim->master;

  /*
   * Charm-o-rama.
   */
  if ( IS_SET(ch->affected_by, AFF_CHARM) && ( ch->master == NULL ))
    {

      bugf("Check_killer: %s bad AFF_CHARM",
       IS_NPC(ch) ? ch->short_descr : ch->name );
      affect_strip( ch, gsn_charm_person );
      REMOVE_BIT( ch->affected_by, AFF_CHARM );
      return;
      /*  stop_follower( ch );
      return;
      */
    }

#if MEMDEBUG
  memdebug_check(ch,"Fight:Check Killer");
#endif
  /*
   * NPC's are fair game.
   * So are killers and thieves.
   */
  /*
   * NPC's are cool of course (as long as not charmed).
   * Hitting yourself is cool too (bleeding).
   * So is being immortal (Alander's idea).
   * And current killers stay as they are.
   */


  if (IS_SAFFECTED(victim, SAFF_ADRENALIZE))
    return;

  if ( IS_NPC(victim)
  ||   IS_NPC(ch)
  ||   ch == victim
  ||   ch->level >= LEVEL_IMMORTAL
  ||   victim->level >= LEVEL_IMMORTAL
  ||   !is_clan(ch)
  ||   ch->fighting == victim
  ||   victim->fighting == ch )
    return;

  adrenalize = FALSE;
  if (ch->level < 51){ 
    if (!IN_RANGE(0, victim->level, ch->level+6)) 
      adrenalize = TRUE;
  }
  else
  {
    if (!IN_RANGE(0, victim->level, ch->level + (int)(ch->level *0.10)+1))
      adrenalize = TRUE;
  }
  /* Clan Dredd does not get adrenalized. */
  /*if (ch->clan == 9) 
    adrenalize = FALSE;
  if ((victim->clan == 9) && (victim->level > ch->level + 5))
    adrenalize = FALSE;*/

  if (adrenalize)
  {
    if (!IS_SAFFECTED(ch, SAFF_ADRENALIZE))
      send_to_char("You are now full of adrenaline.\n\r",ch);

    affect_strip(ch, gsn_adrenalize);
    af.where     = TO_SPELL_AFFECTS;
    af.type      = gsn_adrenalize;
    af.level     = ch->level;
    af.duration  = 2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SAFF_ADRENALIZE;
    affect_to_char( ch, &af );
    // }
  }

  //So if the victim has any flags, the attacker gets none...? Removing.
/*  if (IS_SET(victim->act, PLR_KILLER)
  ||   IS_SET(victim->act, PLR_THIEF)
  ||   IS_SET(victim->act, PLR_TWIT)
  ||   IS_SET(victim->act, PLR_VIOLENT) )
    return; */


  /* Check entire victim charmie tree.  If all members of tree have flags,
     then the attacker does not get flags. */

  /*all_flags_found = TRUE;*/   /* Assume victim and all charmies have flags */
  /*for (vch = victim->in_room->people; ( (vch != NULL) && all_flags_found); vch = vch_next)
    {
      vch_next = vch->next_in_room;
      if (IS_AFFECTED(vch, AFF_CHARM) )
    {
      vch_master = vch->master;
      victim_found = FALSE;
      while (vch_master && !victim_found)
        {
          if (vch_master == victim)
        {
          victim_found = TRUE;
          if ( !IS_SET(vch->act, PLR_KILLER)
              && !IS_SET(vch->act, PLR_THIEF)
              && !IS_SET(vch->act, PLR_TWIT)
              && !IS_SET(vch->act, PLR_VIOLENT)
                 && !IS_NPC(vch) )
            all_flags_found = FALSE;*/ /*Some charmie in tree has no flags */
        /*} else {
          vch_master = vch_master->master;
        }
        }
    } else {
      if ( (vch == victim) &&
           ( !IS_SET(vch->act, PLR_KILLER)
          && !IS_SET(vch->act, PLR_THIEF)
          && !IS_SET(vch->act, PLR_TWIT)
          && !IS_SET(vch->act, PLR_VIOLENT) ) )
        all_flags_found = FALSE;*/ /* Victim has no flags */
/*    }
    }
  if (all_flags_found)*/
    /*return;*/    /* Returns if all members of victim charmie tree have flags */

  if (!IS_SET(ch->act, PLR_KILLER)){
    if ( ( ch->clan && victim->clan )
    &&   ch->clan != victim->clan )
//    if (clan_status[ch->clan][victim->clan]  > 1 )
    {
        send_to_char( "{R*** You are now a VIOLENT KILLER!! ***{x\n\r", ch );
    
        SET_BIT(ch->act, PLR_VIOLENT);
        //SET_BIT(ch->act, PLR_KILLER);
        ch->vflag_timer = VFLAG_TIME;
        mprintf(sizeof(buf), buf,"{W%s is attempting to murder %s{x",ch->name,victim->name);
        wiznet(buf,ch,NULL,WIZ_MURDER,0,0);
        save_char_obj( ch , FALSE);
    }
  }
  return;
}
#endif

/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim, bool showflag )
{
  int chance, dnum;
  char *tochar, *tovict, *toother;

  if ( !IS_AWAKE(victim) ||
       get_skill(victim,gsn_parry) < 1 )
    return FALSE;

  chance = get_skill(victim,gsn_parry) / 2;

  if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
    {
      if (IS_NPC(victim) )
    chance /= 2;
      else
    return FALSE;
    }

  if (!can_see(ch,victim))
    chance /= 2;

  if ( number_percent( ) >= chance + victim->level - ch->level )
    return FALSE;
  
  if (showflag) {
    dnum = number_range (1,5);
    switch (dnum) {
    case 1:
      tochar  = "$N slaps away your attack.";
      tovict  = "You slap away $n's attack.";
      toother = "$N slaps away $n's attack.";
    break;
    case 2:
      tochar  = "$N easily redirects your attack away from $M.";
      tovict  = "You easily redirect $n's attack away from you.";
      toother = "$N easily redirects $n's attack away from $M.";
    break;
    case 3:
      tochar  = "$N laughs as $E knocks away your attack.";
      tovict  = "You laugh as you knock away $n's attack.";
      toother = "$N laughs as $E knocks away $n's attack.";
    break;
    case 4:
      tochar  = "$N beats back your attack.";
      tovict  = "You beat back $n's attack.";
      toother = "$N beats back $n's attack.";
    break;
    case 5:
      tochar  = "$N deftly parries your attack.";
      tovict  = "You deftly parry $n's attack.";
      toother = "$N deftly parries $n's attack.";
    break;
    default:
      tochar  = "You parry $n's attack.";
      tovict  = "$N parries your attack.";
      toother =  "$N parries $n's attack.";
    break;
    }

    act_spam( tochar,  ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SPARRY );
    act_spam( tovict,  ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_OPARRY );
    act_spam( toother, ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_OPARRY );
  }
  check_improve(victim,gsn_parry,TRUE,2);
  return TRUE;
}

/*
 * Check for blade weave (hwy parry).
 */
bool check_blade_weave( CHAR_DATA *ch, CHAR_DATA *victim, bool showflag )
{
  int chance, dnum;
  char *tochar, *tovict, *toother;

  if ( !IS_AWAKE(victim) ||
       get_skill(victim,gsn_blade_weave) < 1 )
    return FALSE;

  chance = get_skill(victim,gsn_blade_weave) / 2;

  if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
    {
      if (IS_NPC(victim) )
    chance /= 2;
      else
    return FALSE;
    }

  if (!can_see(ch,victim))
    chance /= 2;

  if ( number_percent( ) >= chance + victim->level - ch->level )
    return FALSE;
  
  if (showflag) {
    dnum = number_range (1,4);
    switch (dnum) {
    case 1:
      tochar  = "$N nimbly jumps just left of your attack.";
      tovict  = "You nimbly jump just left of $n's attack.";
      toother = "$N nimbly jumps just left of $n's attack.";
    break;
    case 2:
      tochar  = "$N deftly jumps just right of your attack.";
      tovict  = "You deftly jump just right of $n's attack.";
      toother = "$N deftly jumps just right of $n's attack.";
    break;
    case 3:
      tochar  = "$N agilely jumps away from your attack.";
      tovict  = "You agilely jump away from $n's attack.";
      toother = "$N agilely jumps away from $n's attack.";
    break;
    case 4:
      tochar  = "$N swats down your attack.";
      tovict  = "You swat down $n's attack.";
      toother = "$N swats down $n's attack.";
    break;
    default:
      tochar  = "You weave to avoid $n's attack.";
      tovict  = "$N weaves to avoid your attack.";
      toother = "$N weaves to avoid $n's attack.";
    break;
    }

    act_spam( tochar,  ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SPARRY );
    act_spam( tovict,  ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_OPARRY );
    act_spam( toother, ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_OPARRY );
  }
  check_improve(victim,gsn_blade_weave,TRUE,2);
  return TRUE;
}

/*
 * Check for sidestep (mystic dodge).
 */
bool check_sidestep( CHAR_DATA *ch, CHAR_DATA *victim )
{
  int chance, dnum;
  char *tochar, *tovict, *toother;

  if ( !IS_AWAKE(victim)
  ||   get_skill(victim,gsn_sidestep) < 1 )
    return FALSE;

  chance = get_skill(victim,gsn_sidestep) / 2;

  if (!can_see(victim,ch))
    chance /= 2;
  
  if (number_percent() < get_skill(victim, gsn_tumble))
    chance += get_skill(victim,gsn_tumble)/4;

  if ( number_percent( ) >= chance + victim->level - ch->level )
      return FALSE;

  dnum = number_range (1,5);
  switch (dnum) {
    case 1:
        tochar = "$N easily steps away from your attack.";
        tovict = "You easily step away from $n's attack.";
        toother = "$N easily steps away from $n's attack.";
        break;
    case 2:
        tochar = "$N laughs as $E easily avoids your attack.";
        tovict = "You laugh as you easily avoid $n's attack.";
        toother = "$N laughs as $E easily avoids $n's attack.";
        break;
    case 3:
        tochar = "$N steps back, evading your attack.";
        tovict = "You step back, evading $n's attack.";
        toother = "$N steps back, evading $n's attack.";
        break;
    case 4:
        tochar = "$N sighs and sidesteps your attack.";
        tovict = "You sigh and sidestep $n's attack.";
        toother = "$N sighs and sidesteps $n's attack.";
        break;
    case 5:
        tochar = "$N smoothly rolls away from your attack.";
        tovict = "You smoothly roll away from $n's attack.";
        toother = "$N smoothly rolls away from $n's attack.";
        break;
    default:
        tochar = "You dodge $n's attack.";
        tovict = "$N dodges your attack.";
        toother = "$N dodges $n's attack.";
        break;
  }

  act_spam( tochar,  ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SDODGE );
  act_spam( tovict,  ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_ODODGE );
  act_spam( toother, ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_ODODGE );


  check_improve(victim,gsn_sidestep,TRUE,6);
  check_improve(victim,gsn_tumble, TRUE, 1);
  return TRUE;
}

/*
 * Check for shield block.
 */
bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
  int chance, dnum, dam;
  OBJ_DATA *shield;
  

  if ( !IS_AWAKE(victim)
  ||   get_skill(victim,gsn_shield_block) < 1 )
    return FALSE;

  chance = get_skill(victim,gsn_shield_block) / 5 + 3;

  if ( get_eq_char( victim, WEAR_SHIELD ) == NULL )
    return FALSE;

  if ( number_percent( ) >= chance + victim->level - ch->level )
    return FALSE;

  dnum = number_range(1,5);

  switch (dnum)
  {
    case 1:
      act( "$N easily stops your attack with $S shield.",   ch, NULL, victim, TO_CHAR );
      act( "You easily stop $n's attack with your shield.", ch, NULL, victim, TO_VICT );
      act( "$N easily stops $n's attack with $S shield.",   ch, NULL, victim, TO_NOTVICT);
      break;
    case 2:
      act( "$N's shield interrupts your attack.", ch, NULL, victim, TO_CHAR );
      act( "Your shield interrupts $n's attack.", ch, NULL, victim, TO_VICT );
      act( "$N's shield interrupts $n's attack.", ch, NULL, victim, TO_NOTVICT);
      break;
    case 3:
      act( "$N redirects your attack with $S shield.",   ch, NULL, victim, TO_CHAR );
      act( "You redirect $n's attack with your shield.", ch, NULL, victim, TO_VICT );
      act( "$N redirects $n's attack with $S shield.",   ch, NULL, victim, TO_NOTVICT);
      break;
    case 4:
      act( "$N slams $S shield in front of your attack.",   ch, NULL, victim, TO_CHAR );
      act( "You slam your shield in front of $n's attack.", ch, NULL, victim, TO_VICT );
      act( "$N slams $S shield in front of $n's attack.",   ch, NULL, victim, TO_NOTVICT);
      break;
    case 5:
      act( "Your attack crashes against $N's shield.", ch, NULL, victim, TO_CHAR );
      act( "$n's attack crashes against your shield.", ch, NULL, victim, TO_VICT );
      act( "$n's attack crashes against $N's shield.", ch, NULL, victim, TO_NOTVICT);
      break;
    default:
      act( "You block $n's attack with your shield.",  ch, NULL, victim, TO_VICT );
      act( "$N blocks your attack with a shield.", ch, NULL, victim, TO_CHAR );
      act( "$N blocks $n's attack with a shield.", ch, NULL, victim, TO_NOTVICT);
      break;
  }

  check_improve(victim,gsn_shield_block,TRUE,6);

  if ( IS_NPC(victim) )
    return TRUE;

  if ( victim->gameclass == cInquisitor )
  {
    shield = get_eq_char( victim, WEAR_SHIELD );

    if (!shield)
      return TRUE;

    if ( !IS_SET( shield->extra_flags, ITEM_BLESSED_SHIELD ) )
      return TRUE;

    if ( ch->alignment < 1 )
      return TRUE;

    //Now, all should be right to damage them back.
    //act_spam so nospam picks it up.
    dam = number_range( shield->level / 4 , shield->level / 3 + 10);
    act_spam( "$p reflects a wave of {Yh{yo{wl{Wy{x energy upon $N.",
      victim, shield, ch, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
    act_spam( "$p reflects a wave of {Yh{yo{wl{Wy{x energy upon $N.",
      victim, shield,ch, TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS);
    act_spam( "$p reflects {Yh{yo{wl{Wy{x energy{x upon you.",
      ch, shield, NULL, TO_CHAR, POS_RESTING, NOSPAM_OEFFECTS);
    dam += dam /2;
    damage(ch,victim,dam,0,DAM_HOLY,FALSE, FALSE);
    bugf( "Damage %d", dam );
  }

  return TRUE;
}


/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
  int chance, dnum;
  char *tochar, *tovict, *toother;

  if ( !IS_AWAKE(victim)
  ||   get_skill(victim,gsn_dodge) < 1 )
    return FALSE;

  chance = get_skill(victim,gsn_dodge) / 2;

  if (!can_see(victim,ch))
    chance /= 2;
  
  if (number_percent() < get_skill(victim, gsn_tumble))
    chance += get_skill(victim,gsn_tumble)/4;

  if ( number_percent( ) >= chance + victim->level - ch->level )
      return FALSE;

  dnum = number_range (1,5);
  switch (dnum) {
    case 1:
        tochar = "$N easily avoids your attack.";
        tovict = "You easily avoid $n's attack.";
        toother = "$N easily avoids $n's attack.";
        break;
    case 2:
        tochar = "$N chuckles as $E evades your attack.";
        tovict = "You chuckle as you evade $n's attack.";
        toother = "$N chuckles as $E evades $n's attack.";
        break;
    case 3:
        tochar = "$N nimbly jumps away from your attack.";
        tovict = "You nimbly jump away from $n's attack.";
        toother = "$N nimbly jumps away from $n's attack.";
        break;
    case 4:
        tochar = "$N yawns and dodges your attack.";
        tovict = "You yawn and dodge $n's attack.";
        toother = "$N yawns and dodges $n's attack.";
        break;
    case 5:
        tochar = "$N smoothly ducks under your attack.";
        tovict = "You smoothly duck under $n's attack.";
        toother = "$N smoothly ducks under $n's attack.";
        break;
    default:
        tochar = "You dodge $n's attack.";
        tovict = "$N dodges your attack.";
        toother = "$N dodges $n's attack.";
        break;
  }

  act_spam( tochar,  ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SDODGE );
  act_spam( tovict,  ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_ODODGE );
  act_spam( toother, ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_ODODGE );


  check_improve(victim,gsn_dodge,TRUE,6);
  check_improve(victim,gsn_tumble, TRUE, 1);
  return TRUE;
}



/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
  if ( victim->hit > 0 )
  {
    if ( victim->position <= POS_STUNNED )
      victim->position = POS_STANDING;
    return;
  }

  if ( IS_NPC(victim) && victim->hit < 1 )
  {
      victim->position = POS_DEAD;
      return;
  }

  if ( victim->hit <= -11 )
  {
      victim->position = POS_DEAD;
      return;
  }

  if ( victim->hit <= -6 )      victim->position = POS_MORTAL;
  else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
  else                          victim->position = POS_STUNNED;

  return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
  OBJ_DATA *onobj = NULL;

  if ( ( ( onobj = ch->on ) != NULL ) // check if they are on quest bedroll
  &&   ( ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_MANA )
  ||     ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_HP )
  ||     ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_BOTH ) ) )
  {
    obj_from_room( onobj );
    obj_to_char( onobj, ch );
  }

  if ( ch->fighting != NULL )
    {
      bugf( "Set_fighting: already fighting: ch:%s vic:%s, %s",
        ch->name, victim->name, interp_cmd );
      return;
    }

  if ( IS_AFFECTED(ch, AFF_SLEEP) )
    affect_strip( ch, gsn_sleep );

  ch->fighting = victim;
  ch->position = POS_FIGHTING;
  ch->bs_flag = victim->bs_flag = TRUE;
  return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
  CHAR_DATA *fch;
  bool group_fight_continues = FALSE;

  if (ch->fighting == NULL && !fBoth)
    return;

  if (ch->in_room == NULL)
    {
      logit("Question: Why null here? %s", interp_cmd);
    } else {
       
      /* First determine if a group fight will continue; */
      for ( fch = ch->in_room->people; fch != NULL; fch = fch->next_in_room)
    {
      if (fch == ch)
        continue;

      if (is_same_group(fch, ch) && is_clan(fch) )
        group_fight_continues = TRUE;
    }
    }
  for ( fch = char_list; fch != NULL; fch = fch->next )
    {
      if ( fch == ch || ( fBoth && fch->fighting == ch ) )
    {
      fch->bs_flag     = FALSE;
      fch->fighting    = NULL;
      if (!group_fight_continues || fch == ch)
        fch->group_fight = 0;
      fch->position    = IS_NPC(fch) ? fch->pIndexData->start_pos : POS_STANDING;
      update_pos( fch );
    }
    }
  return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch, CHAR_DATA *killer )
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *corpse;
  OBJ_DATA *obj, *obj_next;
  //  OBJ_DATA *con_obj, *next_con_obj;
  char *name;
  int dia_count = 0;

  if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
    return;

  if ( IS_NPC( ch ) )
  {
    name          = ch->short_descr;
    corpse        = create_object( get_obj_index( OBJ_VNUM_CORPSE_NPC ), 0);
    corpse->timer = number_range( 8, 15 );

    if ( ch->gold > 0 || ch->silver > 0)
    {
      obj_to_obj( create_money( ch->gold, ch->silver ), corpse );
      ch->gold    = 0;
      ch->silver  = 0;
    }

    corpse->cost = 0;

    if ( ch->frag_number > 0 )
      corpse->value[0] = ch->frag_number;

    corpse->value[4] = ch->pIndexData->vnum;
    corpse->value[5] = (ch->level / 25) + 1;

    if ( IS_SET( ch->act2, ACT2_RFRAG ) )
    {
      mprintf(sizeof(buf),buf, "ruby %s",corpse->name );
        free_string( corpse->name );
        corpse->name = str_dup( buf, corpse->name );
    }
    
    if ( IS_SET( ch->act2, ACT2_EFRAG ) )
    {
      mprintf(sizeof(buf),buf, "emerald %s",corpse->name );
      free_string(corpse->name);
      corpse->name = str_dup(buf, corpse->name);
    }

    if ( IS_SET( ch->act2, ACT2_SFRAG ) )
    {
      mprintf(sizeof(buf),buf, "sapphire %s",corpse->name );
      free_string(corpse->name);
      corpse->name = str_dup(buf, corpse->name);
    }

    if ( IS_SET( ch->act2, ACT2_DFRAG ) )
    {
      mprintf(sizeof(buf),buf, "diamond %s",corpse->name );
      free_string(corpse->name);
      corpse->name = str_dup(buf, corpse->name);
    }

    if ( IS_SET( ch->act2, ACT2_AFRAG ) )
    {
      mprintf(sizeof(buf),buf, "any %s",corpse->name );
      free_string(corpse->name);
      corpse->name = str_dup(buf, corpse->name);
    }
  }
  else
  {
    name          = ch->name;
    corpse        = create_object( get_obj_index( OBJ_VNUM_CORPSE_PC ), 0 );
    corpse->timer = number_range( 30, 60 );

#if LOOTING
    REMOVE_BIT( ch->act, PLR_CANLOOT );
#endif

    if ( !is_clan( ch ) )
    {
      corpse->owner = str_dup( ch->name, corpse->owner );
#if MEMDEBUG
      memdebug_check( ch, "fight: Make corpse" );
#endif
    }
    else
    {
#if LOOTING
      corpse->owner = NULL;
#else
      corpse->owner = str_dup( ch->name, corpse->owner );
#endif
      if ( ch->gold > 1 || ch->silver > 1 )
      {
        if ( IS_SET( ch->act, PLR_MORGUE ) && !IS_NPC( killer ) 
        &&   IS_SET( killer->act, PLR_AUTOGOLD ) )
        {
          killer->gold    += ch->gold;
          killer->silver  += ch->silver;
          printf_to_char( killer, "You get {Y%d {yg{Yo{yld{x and {W%d {wsi{Wl{Dv{wer{x from %s.\n\r",
            ch->gold, ch->silver, ch->name );
        }
        else
          obj_to_obj( create_money( ch->gold / 2, ch->silver / 2 ), corpse );

        ch->gold      = 0;
        ch->silver    = 0;
      }
    }
        
    mprintf(sizeof(buf),buf, "pcorpse %s %s", corpse->name, name);
    free_string( corpse->name );
    corpse->name = str_dup(buf, corpse->name);
    corpse->cost = 0;
  }

  corpse->level = ch->level;

  mprintf(sizeof(buf), buf, corpse->short_descr, name );
  free_string( corpse->short_descr );
  corpse->short_descr = str_dup( buf , corpse->short_descr);

  mprintf(sizeof(buf), buf, corpse->description, name );
  free_string( corpse->description );
  corpse->description = str_dup( buf , corpse->description);

  if ( IS_SET( ch->act, PLR_MORGUE ) && killer )
  {
    for ( obj = ch->carrying; obj; obj = obj_next )
    {
      obj_next = obj->next_content;

      if ( IS_DIAMOND( obj ) )
        dia_count++;

      if ( obj->item_type == ITEM_CONTAINER )
        dia_count += get_diamond_container_number( obj );
    }

    if ( IS_NPC( killer ) )
      remove_diamonds_char( ch, NULL, dia_count );
    else if ( IS_SET( killer->act, PLR_AUTOGOLD ) )
      remove_diamonds_char( ch, killer, dia_count );
  }

  for ( obj = ch->carrying; obj; obj = obj_next )
  {
    bool floating = FALSE;

    obj_next = obj->next_content;

    if ( obj->wear_loc == WEAR_FLOAT )
      floating = TRUE;

    obj_from_char( obj );

    if ( obj->item_type == ITEM_POTION )
      obj->timer = number_range( 500,1000 );

    if ( obj->item_type == ITEM_SCROLL )
      obj->timer = number_range( 1000,2500 );

    if ( IS_SET( obj->extra_flags, ITEM_ROT_DEATH ) && !floating )
    {
      obj->timer = number_range( 5,10 );
      REMOVE_BIT( obj->extra_flags, ITEM_ROT_DEATH );
    }

    REMOVE_BIT( obj->extra_flags, ITEM_VIS_DEATH );

    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
      extract_obj( obj );
    else if ( floating )
    {
      if ( IS_OBJ_STAT( obj, ITEM_ROT_DEATH ) ) /* get rid of it! */
      {
        if ( obj->contains )
        {
          OBJ_DATA *in, *in_next;

          act( "$p evaporates, scattering its contents.", 
              ch, obj, NULL, TO_ROOM );

          for ( in = obj->contains; in; in = in_next )
          {
            in_next = in->next_content;
            obj_from_obj( in );
            obj_to_room( in, ch->in_room );
          }
        }
        else
          act("$p evaporates.", ch, obj, NULL, TO_ROOM );

        extract_obj( obj );
      }
      else
      {
        //No looting on Draknor, hold onto floaters
        act("As a last effort $n grabs ahold of $p.", ch, obj, NULL, TO_ROOM );
        obj_to_obj( obj, corpse );
      }
    }
    else
      obj_to_obj( obj, corpse );
  }

  if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_MORGUE ) )
  {
    if ( ch->in_room->area->continent == 1 ) //New cont, easy hack for now.
      obj_to_room( corpse, get_room_index( ROOM_VNUM_CORUS_MORGUE ) );
    else
      obj_to_room( corpse, get_room_index( IS_EVIL( ch ) ? ROOM_VNUM_EVIL_MORGUE :
        (IS_GOOD( ch ) ? ROOM_VNUM_MORGUE : ROOM_VNUM_NEUT_MORGUE ) ) );
  }
  else
    obj_to_room( corpse, ch->in_room );

  return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
  ROOM_INDEX_DATA *was_in_room;
  char *msg;
  int door;
  int vnum;

  vnum = 0;
  msg = "You hear $n's death cry.";

  switch ( number_range( 0, 24 ) )
    {
    default:
    case  0: msg  = "{W$n{x hits the ground ... {rDEAD{x.";            break;
    case  1: 
      if ( ch->material == 0 )
      {
        msg  = "$n sprays {rblo{Ro{rd{x across your face.";        
        break;
      }
    case  2:                             
      if ( IS_SET( ch->parts, PART_GUTS ) )
      {
        msg = "$n spills $s {rgut{ys{x all over the floor.";
        vnum = OBJ_VNUM_GUTS;
      }
      break;
    case  3: 
      if ( IS_SET( ch->parts, PART_HEAD ) )
      {
        msg  = "$n's severed {rh{yead{x plops on the ground.";
        vnum = OBJ_VNUM_SEVERED_HEAD;                
      }
      break;
    case  4: 
      if ( IS_SET( ch->parts, PART_HEART ) )
      {
        msg  = "$n's {rh{be{rart{x is torn from $s chest.";
        vnum = OBJ_VNUM_TORN_HEART;                
      }
      break;
    case  5: 
      if ( IS_SET( ch->parts, PART_ARMS ) )
      {
        msg  = "$n's {rar{ym{x is sliced from $s dead body.";
        vnum = OBJ_VNUM_SLICED_ARM;                
      }
      break;
    case  6: 
      if ( IS_SET( ch->parts, PART_LEGS ) )
      {
        msg  = "$n's {yleg{x is sliced from $s dead body.";
        vnum = OBJ_VNUM_SLICED_LEG;                
      }
      break;
    case 7:
      if ( IS_SET( ch->parts, PART_HANDS ) )
      {
        msg = "$n's {yha{wn{rd{x is chopped off from $s dead body.";
        vnum = OBJ_VNUM_HANDS;
      }
      break;
    case 8:
      if ( IS_SET( ch->parts, PART_BRAINS ) )
      {
        msg = "$n's head is shattered, and $s {rbra{yi{rn{ys{x splash all over you.";
        vnum = OBJ_VNUM_BRAINS;
      }
      break;
    case 9:
      if ( IS_SET( ch->parts, PART_FEET ) )
      {
        msg = "$n's {yf{roo{yt{x is cut off at the ankle.";
        vnum = OBJ_VNUM_FEET;
      }
      break;
    case 10:
      if ( IS_SET( ch->parts, PART_FINGERS ) )
      {
        msg = "$n's twitching {yfing{rer{x falls to the ground.";
        vnum = OBJ_VNUM_FINGERS;
      }
      break;
    case 11:
      if ( IS_SET( ch->parts, PART_EAR ) )
      {
        msg = "$n's head is shattered, and $s {ye{rar{x falls to the ground.";
        vnum = OBJ_VNUM_EAR;
      }
      break;
    case 12:
      if ( IS_SET( ch->parts, PART_EYE ) )
      {
        msg = "$n's head is shattered, and $s {re{wy{De{x pops out.";
        vnum = OBJ_VNUM_EYE;
      }
      break;
    case 13:
      if ( IS_SET( ch->parts, PART_LONG_TONGUE ) )
      {
        msg = "$n's head is shattered, and $s {rtong{mue{x is ripped lose.";
        vnum = OBJ_VNUM_LONG_TONGUE;
      }
      break;
    case 14:
      if ( IS_SET( ch->parts, PART_EYESTALKS ) )
      {
        msg = "$n's head is shattered, and $s {we{Dy{we{rstalk{x are ripped off.";
        vnum = OBJ_VNUM_EYESTALKS;
      }
      break;
    case 15:
      if ( IS_SET( ch->parts, PART_TENTACLES ) )
      {
        msg = "$n's body is massacred, and $s {rtent{mac{wl{re{x falls to the ground.";
        vnum = OBJ_VNUM_TENTACLES;
      }
      break;
    case 16:
      if ( IS_SET( ch->parts, PART_FINS ) )
      {
        msg = "$n's body is mangled, and $s {Df{wi{Dn{x falls off.";
        vnum = OBJ_VNUM_FINS;
      }
      break;
    case 17:
      if ( IS_SET( ch->parts, PART_WINGS ) )
      {
        msg = "$n's body is annihilated, and $s {Ww{Di{wng{x falls to the ground.";
        vnum = OBJ_VNUM_WINGS;
      }
      break;
    case 18:
      if ( IS_SET( ch->parts, PART_TAIL ) )
      {
        msg = "$n's body is decimated, and $s {yt{Dail{x falls to the ground.";
        vnum = OBJ_VNUM_TAIL;
      }
      break;
    case 19:
      if ( IS_SET( ch->parts, PART_CLAWS ) )
      {
        msg = "$n's body is massacred, and $s {wc{rl{waw{rs{x are ripped off.";
        vnum = OBJ_VNUM_CLAWS;
      }
      break;
    case 20:
      if ( IS_SET( ch->parts, PART_FANGS ) )
      {
        msg = "$n's body is cut up, and $s {wf{ra{wng{x is knocked out.";
        vnum = OBJ_VNUM_FANGS;
      }
      break;
    case 21:
      if ( IS_SET( ch->parts, PART_HORNS ) )
      {
        msg = "$n's head falls apart, and $s {wh{Wo{wrn{x falls to the ground.";
        vnum = OBJ_VNUM_HORNS;
      }
      break;
    case 22:
      if ( IS_SET( ch->parts, PART_SCALES ) )
      {
        msg = "$n's body is ripped apart, and $s {Dsca{wl{De{x falls to the ground.";
        vnum = OBJ_VNUM_SCALES;
      }
      break;
    case 23:
      if ( IS_SET( ch->parts, PART_TUSKS ) )
      {
        msg = "$n's head is butchered, and $s {Wtusk{Ds{x falls to the ground.";
        vnum = OBJ_VNUM_TUSKS;
      }
      break;
    case 24:
      if ( IS_SET( ch->parts, PART_HOOF ) )
      {
        msg = "$n's leg is mangled, and $s {Dh{yoo{Df{x falls to the ground.";
        vnum = OBJ_VNUM_HOOF;
      }
      break;
    }

  act( msg, ch, NULL, NULL, TO_ROOM );
  if ( ch->in_room == NULL )
  {
    bugf("Char died and was not in a room after the act.\n\r");
    return;
  }
  if ( vnum && !IS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
  {
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    char *name;

    name = IS_NPC( ch ) ? ch->short_descr : ch->name;
    obj     = create_object( get_obj_index( vnum ), 0 );
    obj->timer    = number_range( 4, 7 );

    mprintf(sizeof(buf), buf, obj->short_descr, name );
    free_string( obj->short_descr );
    obj->short_descr = str_dup( buf , obj->short_descr);

    mprintf(sizeof(buf), buf, obj->description, name );
    free_string( obj->description );
    obj->description = str_dup( buf, obj->description );

    if ( obj->item_type == ITEM_FOOD )
    {
      if ( IS_SET( ch->form, FORM_POISON ) )
        obj->value[3] = 1;
      else if ( !IS_SET( ch->form, FORM_EDIBLE ) )
        obj->item_type = ITEM_TRASH;
    }
    obj_to_room( obj, ch->in_room );
  }

  if ( IS_NPC(ch) )
    msg = "You hear something's {rdeath{x cry.";
  else
    msg = "You hear someone's {rdeath{x cry.";

  was_in_room = ch->in_room;
  EXIT_DATA *pexit;

  if ( IS_SET( was_in_room->room_flags, ROOM_SHIP ) )
  {
    if ( was_in_room->state >= 0 )
    {
      if ( ( pexit = was_in_room->exit[was_in_room->state] )
      &&     pexit->u1.to_room );
      {
        ch->in_room = pexit->u1.to_room;
        act( msg, ch, NULL, NULL, TO_ROOM );
      }
    }
  }
  else
  {
    for ( door = 0; door <= 5; door++ )
    {
      if ( ( pexit = was_in_room->exit[door] )
      &&     pexit->u1.to_room
      &&     pexit->u1.to_room != was_in_room )
      {
        ch->in_room = pexit->u1.to_room;
        act( msg, ch, NULL, NULL, TO_ROOM );
      }
    }
  }
  ch->in_room = was_in_room;

/* rescue mob quest failure */
  if ( IS_NPC( ch )
  && ( ch->pIndexData->vnum == MOB_VNUM_QUEST) )
  {
    CHAR_DATA *rch;
    for ( rch = char_list ; rch ; rch = rch->next)
    {
      if ( IS_NPC( rch ) )
        continue;

      if ( strstr(ch->name,rch->name)
      && ( rch->questdata->mob_vnum == MOB_VNUM_QUEST ) )
      {
        rch->questdata->nextquest = 10;
        printf_to_char(rch,
          "{&You have failed your quest because your target has died!\n\r{xYou may quest again in {R%d{x hours.\n\r",
          rch->questdata->nextquest);
          rch->questdata->streak=0;
          rch->questdata->failed++;
        clear_quest(rch,FALSE);
      }
    }
  }

  return;
}

void raw_kill( CHAR_DATA *victim, CHAR_DATA *ch )
{
  int i;
  char buf[MSL];
  CHAR_DATA *fch, *fch_next, *rch, *rch_next;
  AFFECT_DATA tempaf;
  AFFECT_DATA tempaf2;
  bool had_affect   = FALSE;
  bool had_affect2  = FALSE;
  bool was_charmed  = FALSE;

  for ( fch = player_list; fch; fch = fch_next )
  {
    fch_next = fch->next_player;

    if ( !IS_NPC( victim ) && IS_SET( fch->act, PLR_VIOLENT )
    &&    fch->group_fight == victim->group_num)
    {
      REMOVE_BIT( fch->act, PLR_VIOLENT );
      SET_BIT( fch->act, PLR_KILLER );
      send_to_char("{R*** You are now a KILLER!! ***{x\n\r", fch);
    }
  }
  stop_fighting( victim, TRUE );

  for ( fch = victim->in_room->people; fch; fch = fch_next )
  {
      fch_next = fch->next_in_room;
      if ( fch == victim )
        continue;
      if ( is_same_group( fch, victim ) && victim->position == POS_DEAD )
      {
      if ( victim->leader == NULL )
      {
        change_group_leader(victim, fch);
        for ( rch = ch->in_room->people; rch; rch = rch_next )
        {
          rch_next = rch->next_in_room;
          if ( IS_NPC( rch ) )
            continue;
          if ( rch->group_fight == victim->id )
          {
            rch->group_fight = fch->group_num;
            if (rch->pet != NULL)
                 rch->pet->group_fight = fch->group_num;
          }
        }
      }
    }
  }
  death_cry( victim );
  if ( !IS_SET( victim->in_room->room_flags, ROOM_ARENA ) || IS_NPC( victim ) )
    make_corpse( victim, ch );

  for ( fch = char_list; fch; fch = fch->next )
  {
    if( fch->plevel == victim )
        fch->plevel = NULL;
      /* The following clause clears up a bug where if a groupie who backstabbed
         is rescued, and the mob skilled before the groupie gets back into the
         fight, the bs_flag is removed from that groupie */
    if ( fch != ch && fch != victim 
    &&   is_same_group( fch, ch ) && fch->fighting == NULL)
        fch->bs_flag = FALSE;
  }
  victim->plevel = NULL;
  if ( IS_NPC(victim) )
  {
    victim->pIndexData->killed++;
    //fch->pcdata->kills++;
    kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
    victim->in_room->area->numkills++;
    stop_hating_ch( victim );
    extract_char( victim, TRUE );
    if ( !IS_SET( victim->form, FORM_UNDEAD ) && !IS_PET( victim ) )
      check_spirit( ch, victim );
    return;
  }
  stop_hating_ch( victim );
  stop_hating( victim );
  if ( !IS_SET( victim->in_room->room_flags, ROOM_ARENA ) )
    extract_char( victim, FALSE );
  if ( IS_AFFECTED( victim, AFF_CHARM ) )
    was_charmed = TRUE;
  else
    was_charmed = FALSE;
  while ( victim->affected )
  {
    if ( victim->affected->where == TO_PENALTY )
      if ( victim->affected->bitvector == PEN_NOCHANNELS )
      {
        tempaf = *( victim->affected );
        had_affect = TRUE;
      }
    if ( victim->affected->where == TO_ACT )
      if (victim->affected->bitvector == PLR_TWIT )
      {
        tempaf2 = *( victim->affected );
        had_affect2 = TRUE;
      }
    affect_remove( victim, victim->affected );
  }
  if ( had_affect )
    affect_to_char( victim, &tempaf );
  if ( had_affect2 )
    affect_to_char( victim, &tempaf2 );
  if ( was_charmed )
  {
    do_function( victim, &do_nofollow, "" );
    do_function( victim, &do_nofollow, "" );
  }
  victim->affected_by    = race_table[victim->race].aff;
  if ( !IS_SET( victim->in_room->room_flags, ROOM_ARENA ) )
    for ( i = 0; i < 4; i++)
      victim->armor[i]= 100;
  /*  victim->position    = POS_RESTING;*/
  victim->hit    = UMAX( 1, victim->hit  );
  victim->mana    = UMAX( 1, victim->mana );
  victim->move    = UMAX( 1, victim->move );

  if (IS_SET(victim->in_room->room_flags,ROOM_ARENA))
  {
    mprintf(sizeof(buf), buf, "I was killed in the %s {WArena!{x",
      victim->in_room->name );

    if ( IS_IN_CLAN( victim )
    &&  !IS_SET( victim->clan->clan_flags, CLAN_INDEPENDENT ) )
    {

      move_to_room( victim, get_room_index( get_death_room( victim, victim->in_room ) ) );
    }
    else
    {
      if ( IS_EVIL( victim ) )
          move_to_room( victim, get_room_index( ROOM_VNUM_EVIL_HEALER ) );
      if ( IS_GOOD( victim ) )
          move_to_room( victim, get_room_index( ROOM_VNUM_ALTAR ) );
      else
          move_to_room( victim, get_room_index( ROOM_VNUM_NEUT_HEALER ) );
    }

    do_function( victim, &do_info, buf );
  }
  else
  {
    victim->pcdata->condition[COND_HUNGER] = 30;
    victim->pcdata->condition[COND_THIRST] = 30;
    make_ghost( ch, victim );
  }

  if ( !IS_SET( victim->in_room->room_flags, ROOM_ARENA ) )
        save_char_obj( victim, FALSE ); 
        /*we're stable enough to not need this :) */
#if MEMDEBUG
      memdebug_check(victim,"fight: raw_kill vic");
#endif
#if MEMDEBUG
      memdebug_check(ch,"fight: raw_kill ch");
#endif
  return;
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
  CHAR_DATA *gch;
  CHAR_DATA *lch;
  CHAR_DATA *rch;
  int xp;
  int members;
  int group_levels;
  int highestlevel = 0;
  /*
   * Monsters don't get kill xp's or alignment changes.
   * P-killing doesn't help either.
   * Dying of mortal wounds or poison doesn't give xp to anyone!
   */
  if ( victim == ch )
    return;

  if ( !victim->in_room )
  {
    bugf( "Group Gain: Victim %s, is NOT in a room.\n\r",
        victim->name, interp_cmd );
    return;
  }

  if ( !ch->in_room )
  {
    bugf( "Group Gain: Char %s, is NOT in a room.\n\r",
        ch->name, interp_cmd );
    return;
  }

  if ( !IS_NPC( ch ) && ( !IS_NPC( victim ) ) )
    return;

  members = 0;
  group_levels = 0;

  for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
  {
      if ( is_same_group( gch, ch ) )
      {
        members++;
        group_levels += IS_NPC(gch) ? gch->level / 2 : gch->level;
      }
  }

  if ( members == 0 )
  {
      bug( "Group_gain: members.", members );
      members = 1;
      group_levels = ch->level ;
  }

// WTF is this?
  if (victim->plevel != NULL)
  {
    if (victim->plevel != ch)
    {
      if (!is_same_group(ch,victim->plevel))
      {
        send_to_char("You did not do sufficient damage to warrant experience.\n\r",ch);
        return;
      }
    }
  }

  
  for (lch = ch->in_room->people; lch != NULL; lch = lch->next_in_room)
  {
      if ( !is_same_group( lch, ch ))
        continue;

      if (lch->level > highestlevel)
        highestlevel = lch->level;
  }

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
  {
      OBJ_DATA *obj;
      OBJ_DATA *obj_next;

      if ( !is_same_group( gch, ch ) || IS_NPC(gch))
        continue;

      if ( gch->level - highestlevel < -7
      &&   gch->level < 80 )
      {
        send_to_char(
        "Your powers are useless to such an advanced group of adventurers.\n\r",gch);
        continue;
      }

      xp = xp_compute( gch, victim, group_levels );

      if (IS_SET(victim->in_room->room_flags, ROOM_ARENA))
        xp = 0;
      if ( gch->level < LEVEL_HERO+1 )
      {
        if ( (!IS_NPC(ch) && IS_SET( ch->plr2, PLR2_NO_FRAGXP ))
        && ( IS_SET( victim->act2, ACT2_RFRAG )
          || IS_SET( victim->act2, ACT2_SFRAG )
          || IS_SET( victim->act2, ACT2_EFRAG )
          || IS_SET( victim->act2, ACT2_DFRAG )
          || IS_SET( victim->act2, ACT2_AFRAG ) ) )
          printf_to_char(gch, "You receive no experience for your kill{x.\n\r", xp );
        else
        {
          printf_to_char(gch, "You receive {g%d {xexp points{x.\n\r", xp );
          gain_exp( gch, xp );
        }
      }

      for ( obj = gch->carrying; obj; obj = obj_next )
      {
        obj_next = obj->next_content;

        if ( obj->wear_loc == WEAR_NONE )
            continue;

        if ( ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL ) && IS_EVIL(gch) )
        ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD ) && IS_GOOD( gch ) )
        ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL ) && IS_NEUTRAL(gch) ) )
        &&   ( !IS_OBJ_STAT(obj, ITEM_NOREMOVE)  ) )
        {
          act( "You are zapped by $p.", gch, obj, NULL, TO_CHAR );
          act( "$n is zapped by $p.",   gch, obj, NULL, TO_ROOM );
          
          /*if (IS_OBJ_STAT(obj, ITEM_NOREMOVE) )
          {
             act ( "It causes you great pain.", ch, obj, NULL, TO_CHAR );
            damage(ch, ch, 2*obj->level, TYPE_UNDEFINED, DAM_SHOCK, FALSE, FALSE);
          }
          else */
          if (!melt_drop(gch,obj))
          {
            obj_from_char( obj );

            if ( IS_OBJ_STAT(obj,ITEM_NODROP) )
                obj_to_char( obj, gch );
            else
                obj_to_room( obj, gch->in_room );
          }
        }
    }

      rch = gch;
      /* Follow charm thread back to responsible person.
        Only responsible person gets quest credit for kill. */
      while ( (IS_PET(rch) || IS_AFFECTED(rch, AFF_CHARM) ) && rch->master != NULL)
        rch = rch->master;

      if ( !IS_NPC(rch) && IS_SET(rch->act,PLR_QUESTING )
      &&   IS_NPC(victim ) )
      {
        if (rch->questdata->mob_vnum == victim->pIndexData->vnum)
        {
          send_to_char("{&You have almost completed your quest!\n\r",rch);
          send_to_char("Return to the questmaster before you run of time.{x\n\r",rch);
          rch->questdata->mob_vnum = -1;
        }
      }
    }

  return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels )
{
  int xp,base_exp;
  int level_range;
//  int align, change;
  int time_per_level;

  if ( victim->hate )
    if ( ( victim->hate->who != gch )
    && (  !is_same_group( victim->hate->who, gch ) )
    &&   ( victim->hate->who->in_room != gch->in_room ) )
      return 0;

  level_range = victim->level - gch->level;
 
  /* compute the base exp */
  switch ( level_range )
  {
    default :     base_exp =   0;        break;
    case -9 :    base_exp =   1;        break;
    case -8 :    base_exp =   2;        break;
    case -7 :    base_exp =   5;        break;
    case -6 :     base_exp =   9;        break;
    case -5 :    base_exp =  11;        break;
    case -4 :    base_exp =  22;        break;
    case -3 :    base_exp =  33;        break;
    case -2 :    base_exp =  50;        break;
    case -1 :    base_exp =  66;        break;
    case  0 :    base_exp =  83;        break;
    case  1 :    base_exp =  99;        break;
    case  2 :    base_exp = 121;        break;
    case  3 :    base_exp = 143;        break;
    case  4 :    base_exp = 165;        break;
  } 
    
  if ( level_range > 4 )
    base_exp = 160 + 20 * (level_range - 4);

  /* do alignment computations */
/*COMMENTING OUT ALIGNMENT CHANGES FOR KILLING MOBS
  align = victim->alignment - gch->alignment;

  if (IS_SET(victim->act,ACT_NOALIGN) || IS_SET(gch->in_room->room_flags,ROOM_ARENA))
    {
      // no change
    }
  else if (align > 500) // monster is more good than slayer
    {
      change = (align - 500) * base_exp / 500 * gch->level/total_levels;
      change = UMAX(1,change);
      gch->alignment = UMIN(-1000,gch->alignment - change);
    }

  else if (align < -500) // monster is more evil than slayer
    {
      change =  ( -1 * align - 500) * base_exp/500 * gch->level/total_levels;
      change = UMAX(1,change);
      gch->alignment = UMAX(1000,gch->alignment + change);
    }

  else // improve this someday
    {
      change =  gch->alignment * base_exp/500 * gch->level/total_levels;
      gch->alignment -= change;
    }
  if (gch->alignment >= 1000)
    gch->alignment = 1000;
  if (gch->alignment <= -1000)
    gch->alignment = -1000;

*/
    
  /* calculate exp multiplier */
  if (IS_SET(victim->act,ACT_NOALIGN))
    xp = base_exp;
  else if (gch->alignment > 500) /* for goodie two shoes */
  {
      if (victim->alignment < -750)
        xp = (base_exp *4)/3;
      else if (victim->alignment < -500)
        xp = (base_exp * 5)/4;
      else if (victim->alignment > 750)
        xp = base_exp / 4;
      else if (victim->alignment > 500)
        xp = base_exp / 2;
      else if (victim->alignment > 250)
        xp = (base_exp * 3)/4; 
      else
        xp = base_exp;
  }
  else if (gch->alignment < -500) /* for baddies */
  {
      if (victim->alignment > 750)
        xp = (base_exp * 5)/4;
      else if (victim->alignment > 500)
        xp = (base_exp * 11)/10; 
      else if (victim->alignment < -750)
        xp = base_exp/2;
      else if (victim->alignment < -500)
        xp = (base_exp * 3)/4;
      else if (victim->alignment < -250)
        xp = (base_exp * 9)/10;
      else
        xp = base_exp;
  }

  else if (gch->alignment > 200) /* a little good */
  {

      if (victim->alignment < -500)
        xp = (base_exp * 6)/5;
      else if (victim->alignment > 750)
        xp = base_exp/2;
      else if (victim->alignment > 0)
        xp = (base_exp * 3)/4; 
      else
        xp = base_exp;
  }
  else if (gch->alignment < -200) /* a little bad */
  {
      if (victim->alignment > 500)
        xp = (base_exp * 6)/5;
      else if (victim->alignment < -750)
        xp = base_exp/2;
      else if (victim->alignment < 0)
        xp = (base_exp * 3)/4;
      else
        xp = base_exp;
  }
  else            /* neutral */
  {
      if (victim->alignment > 500 || victim->alignment < -500)
        xp = (base_exp * 4)/3;
      else if (victim->alignment < 200 && victim->alignment > -200)
        xp = base_exp/2;
      else
        xp = base_exp;
  }

  /* more exp at the low levels */
  if (gch->level < 6)
    xp = 10 * xp / (gch->level + 4);

  /* less at high -- Raising to level 80 for now (was 55 -- Aarchane-5-11-09) */
  if ( gch->level >= 79 )
    xp -= ( ( gch->level * xp * 2 ) / 280 );
//    xp -= ( (  xp * 35 ) / ( gch->level - 35 ) ); //Verify this formula idea w/ testing..

  /*
   * What is the point of reducing the shit outa exp...then adding a lot back. Changing.
   */
  //xp = xp + (xp * (get_curr_stat(gch,STAT_INT) + get_curr_stat(gch, STAT_WIS)*2)/200);

  /* reduce for playing time */    
  {
    /* compute quarter-hours per level */
    time_per_level = 4 * LEVEL_PLAY_TIME(gch) / 3600 / gch->level;
    time_per_level = URANGE(2,time_per_level,12);

    if (gch->level < 15)    /* make it a curve */
      time_per_level = UMAX(time_per_level,(15 - gch->level));

    xp = xp * time_per_level / 12;
  }

  /* randomize the rewards */
  xp = number_range (xp * 3/4, xp * 5/4);

  /* adjust for grouping */
  if ( gch->level < total_levels )
    xp = xp * gch->level/( number_range( gch->level + 1, ( total_levels * 3 ) / 4  ) );


  /*
   * Cap experience
   * 800 minus a random number between 1 and 99
   */
  if ( xp >= 800 )
  {
    int reduction = 0;

    reduction = xp - 800;

    while ( reduction > 100 )
      reduction -= 100;

    reduction = number_range( 1, reduction );

    bugf( "CHAR %s received %d experience from %s, amount is being reduced to %d.",
      gch->name,
      xp,
      IS_NPC( victim ) ? victim->short_descr : victim->name,
      800 - reduction );

    xp = 800 - reduction;
    //gain = 800 + ( ( gain - 800 ) * number_percent() );
  }

  return xp;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune )
{
  char buf1[256], buf2[256], buf3[256];
  const char *vs;
  const char *vp;
  const char *attack;
  char punct;
  char dbuf[15];
  if (ch == NULL || victim == NULL)
    return;

  if ( dam == 0 )       { vs = "{wmiss{x";                 vp = "{wmisses{x"; }
  else if ( dam <=   5) { vs = "{rscratch{x";              vp = "{rscratches{x";    }
  else if ( dam <=  10) { vs = "{rgraze{x";                vp = "{rgrazes{x";        }
  else if ( dam <=  15) { vs = "{rhit{x";                  vp = "{rhits{X";        }
  else if ( dam <=  20) { vs = "{rbruise{x";               vp = "{rbruises{x";        }
  else if ( dam <=  25) { vs = "{rinjure{x";               vp = "{rinjures{X";        }
  else if ( dam <=  30) { vs = "{rwound{x";                vp = "{rwounds{x";        }
  else if ( dam <=  35) { vs = "{rmaul{x";                 vp = "{rmauls{x";        }
  else if ( dam <=  40) { vs = "{rdecimate{x";             vp = "{rdecimates{x";    }
  else if ( dam <=  55) { vs = "{rs{Rma{rsh{x";            vp = "{rs{Rma{rshes{x";        }
  else if ( dam <=  70) { vs = "{rbludg{Reo{rn{x";         vp = "{rbludg{Reo{rns{x";    }
  else if ( dam <=  85) { vs = "{rde{Rva{rst{Ra{rte{x";    vp = "{rde{Rva{rst{Ra{rtes{x";    }
  else if ( dam <= 100) { vs = "{rM{RAI{rM{x";             vp = "{rM{RAI{rMS{x";        }
  else if ( dam <= 115) { vs = "{rM{RUT{rIL{RA{rTE{x";     vp = "{rM{RUT{rIL{RA{rTES{x";    }
  else if ( dam <= 130) { vs = "{rDIS{REM{rB{RO{rW{REL{x"; vp = "{rDIS{REM{rB{RO{rW{REL{rS{x";    }
  else if ( dam <= 145) { vs = "{RDI{rSM{REM{rB{RE{rR{x";  vp = "{RDI{rSM{REM{rB{RE{rRS{x";    }
  else if ( dam <= 160) { vs = "{rMA{RSS{rAC{RR{rE{x";     vp = "{rMA{RSS{rAC{RR{rES{x";    }
  else if ( dam <= 175) { vs = "{rM{RAN{rG{RL{rE{x";       vp = "{rM{RAN{rG{RL{rE{RS{x";    }
  else if ( dam <= 200) { vs = "{w*** {gDEMOLISH {w***{x";   vp = "{w*** {gDEMOLISHES {w***{x";            }
  else if ( dam <= 225) { vs = "{w*** {cEVISCERATE {w***{x"; vp = "{w*** {cEVISCERATES {w***{x";            }
  else if ( dam <= 250) { vs = "{w*** {mOBLITERATE {w***{x"; vp = "{w*** {mOBLITERATES {w***{x";        }
  else if ( dam <= 275) { vs = "{w*** {DANNIHILATE{w ***{x"; vp = "{w*** {DANNIHILATES {w***{x";        }
  else if ( dam <= 300) { vs = "{w=== {yERADICATE {w==={x";  vp = "{w=== {yERADICATES {w==={x";            }
  else if ( dam <= 325) { vs = "{w==={r DESTROY {w==={x";    vp = "{w==={r DESTROYS{w ==={x";        }
  else if ( dam <= 350) { vs = "{w=== {bVAPORIZE {w==={x";   vp = "{w=== {bVAPORIZES {w==={x";        }
  else if ( dam <= 375) { vs = "{w/\\/ {gPULV{GER{gI{GZ{gE {w\\/\\{x";       vp = "{w/\\/ {gPULV{GER{gI{GZ{gES {w\\/\\{x"; }
  else if ( dam <= 400) { vs = "{w/\\/ {yE{YX{yTERM{YI{yN{YAT{yE {w\\/\\{x"; vp = "{w/\\/ {yE{YX{yTERM{YI{yN{YAT{yES {w\\/\\{x"; }
  else if ( dam <= 425) { vs = "{w/\\/ {cA{CTO{cMI{CZ{cE {w\\/\\{x";         vp = "{w/\\/ {cA{CT{cOM{CIZ{cES {w\\/\\{x";            }
  else { vs = "lays {r{lW{RA{r{lS{RT{r{lE{x to";             vp = "lays {r{lW{RA{r{lS{RT{r{lE{x to";        }

  punct   = (dam <= 40) ? '.' : '!';
#if TESTIT
  printf_to_char(ch,"DAMMESG #1: DT = %d\n",dt);
#endif
  if (IS_IMMORTAL(ch))
    mprintf(sizeof(dbuf), dbuf,"[%d]",dam);
  else
    strcpy(dbuf,"");
  
  if ( dt == TYPE_HIT )
  {
    if (ch  == victim)
      {
        mprintf(sizeof(buf1), buf1, "$n %s $melf%c",vp,punct);
        mprintf(sizeof(buf2), buf2, "Your %s%s yourself%c",vs,dbuf,punct);
      }
    else
      {
        mprintf(sizeof(buf1), buf1, "$n %s $N%c",  vp, punct );
        mprintf(sizeof(buf2), buf2, "You %s%s $N%c", vs,dbuf, punct );
        mprintf(sizeof(buf3), buf3, "$n %s you%s%c", vp, dbuf, punct );
      }
  }
  else
  {
    if ( dt >= 0 && dt < MAX_SKILL ) {
        attack    = skill_table[dt].noun_damage;
#if TESTIT
        printf_to_char(ch,"Test value 1: dt = %d, -%s-\n\r",dt, attack);
#endif
    }
    else if ( dt >= TYPE_HIT
        && dt < TYPE_HIT + MAX_DAMAGE_MESSAGE)
    {
        attack    = attack_table[dt - TYPE_HIT].noun;
#if TESTIT
        printf_to_char(ch,"Test value 2: dt = %d, -%s-\n\r",dt,
               attack);
#endif
    }
    else
      {
        bug( "Dam_message: bad dt %d.", dt );
        dt  = TYPE_HIT;
        attack  = attack_table[0].name;
      }

    if (immune)
      {
        if (ch == victim)
        {
            mprintf(sizeof(buf1), buf1,"$n is unaffected by $s own %s.",attack);
            mprintf(sizeof(buf2), buf2,"Luckily, you are immune to that.");
        } 
        else
        {
          mprintf(sizeof(buf1), buf1,"$N is unaffected by $n's %s!",attack);
          mprintf(sizeof(buf2), buf2,"$N is unaffected by your %s!",attack);
          mprintf(sizeof(buf3), buf3,"$n's %s is powerless against you.",attack);
        }
      }
    else
      {
        if (ch == victim)
        {
          mprintf(sizeof(buf1), buf1, "$n's %s %s $m%c",attack,vp,punct);
          mprintf(sizeof(buf2), buf2, "{WYour{x %s %s%s you%c",attack,vp,dbuf,punct);
        }
        else
        {
          mprintf(sizeof(buf1), buf1, "$n's %s %s $N%c",  attack, vp, punct );
          mprintf(sizeof(buf2), buf2, "{WYour{x %s %s%s $N%c",  attack, vp,dbuf, punct );
          mprintf(sizeof(buf3), buf3, "$n's %s %s you%c", attack, vp, punct );
        }
      }
  }

  if (ch == victim)
  {
    if ( ( dam == 0 ) && (!immune) )
    {
      act_spam(buf1,ch,NULL,NULL,TO_ROOM, POS_RESTING, NOSPAM_OMISS);
      act_spam(buf2,ch,NULL,NULL,TO_CHAR, POS_RESTING, NOSPAM_SMISS);
    }
    else
    {
      act_spam(buf1,ch,NULL,NULL,TO_ROOM, POS_RESTING, NOSPAM_OHIT);
      act_spam(buf2,ch,NULL,NULL,TO_CHAR, POS_RESTING, NOSPAM_SHIT);
    }
  }
  else
  {
    if ( ( dam == 0 ) && (!immune) )
    {  
      act_spam( buf1, ch, NULL, victim, TO_NOTVICT,POS_RESTING, NOSPAM_OMISS );
      act_spam( buf2, ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SMISS );
      act_spam( buf3, ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_OMISS );
    }
    else
    {
      act_spam( buf1, ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_OHIT );
      act_spam( buf2, ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SHIT );
      act_spam( buf3, ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_SHIT );
    }
  }

  return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
  OBJ_DATA *obj;

  if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    return;

  if (IS_GHOST(ch)) {
    send_to_char("Disarming is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
      act("$S weapon won't budge!",ch,NULL,victim,TO_CHAR);
      act("$n tries to disarm you, but your weapon won't budge!",
      ch,NULL,victim,TO_VICT);
      act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
      return;
    }

  act( "$n {YDISARMS{x you and sends your weapon flying!", 
       ch, NULL, victim, TO_VICT    );
  act( "You {Wdisarm{x $N!",  ch, NULL, victim, TO_CHAR    );
  act( "$n disarms $N!",  ch, NULL, victim, TO_NOTVICT );

  obj_from_char( obj );
  if (!melt_drop(victim,obj))
  {
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
        obj_to_char( obj, victim );
    else
    {
        obj_to_room( obj, victim->in_room );
      /* Removed for now - RWL */
      /*if (IS_NPC(victim)
      && victim->wait == 0
      && can_see_obj(victim,obj))
          get_obj( victim, obj, NULL );*/
    }
  }
  return;
}

void do_berserk( CHAR_DATA *ch, char *argument)
{
  int chance, hp_percent, time_length = 0;

  if (IS_GHOST(ch)) {
    send_to_char("Beserking is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ((chance = get_skill(ch,gsn_berserk)) == 0
  ||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
  ||  (!IS_NPC(ch)
  &&  ( ch->level < skill_table[gsn_berserk].skill_level[ch->gameclass]
  &&    !is_racial_skill(ch,gsn_berserk) ) ) )
  {
      send_to_char("You turn red in the face, but nothing happens.\n\r",ch);
      return;
  }

  if (IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,gsn_berserk)
  ||  is_affected(ch,skill_lookup("frenzy")))
  {
      send_to_char("You get a little madder.\n\r",ch);
      return;
  }

  if (IS_AFFECTED(ch,AFF_CALM))
    {
      send_to_char("You're feeling too mellow to berserk.\n\r",ch);
      return;
    }

  if (ch->mana < 50)
    {
      send_to_char("You can't get up enough energy.\n\r",ch);
      return;
    }

  //modifiers
  if (ch->position == POS_FIGHTING)
    chance += 10;

  hp_percent = 100 * ch->hit/GET_HP(ch);
  chance += 25 - hp_percent/4;

  //berserk Success
  if (number_percent() < chance)
  {
    AFFECT_DATA af;

    WAIT_STATE(ch,PULSE_VIOLENCE);
    ch->mana -= 50;
//      ch->move /= 2;
// put a bit of variance in berserk cost: random from mv/2 up to (high):
//           mov     (100 - skill)
//  mov  -  ----- * --------------
//            2          100
// I know this may look very confusing, but I spent a half-hour calculating it on paper first :)
    ch->move = number_range((ch->move/2),(ch->move-((ch->move*(100-get_skill(ch,gsn_berserk)))/200)) );

    if ( ( (ch->hit*100) / GET_HP(ch) ) < 30 )
      ch->hit += ch->level * 4;
    else
      ch->hit += ch->level * 2;

    ch->hit = UMIN(ch->hit,GET_HP(ch));

    //we'll decide the msg below
    send_to_char("Your pulse races as you are consumed by rage!\n\r",ch);
    act("$n gets a wild look in $s eyes.",ch,NULL,NULL,TO_ROOM);
    check_improve(ch,gsn_berserk,TRUE,2);

		time_length = number_fuzzy(ch->level / 8 );		
/*
    af.where    = TO_AFFECTS;
    af.type        = gsn_berserk;
    af.level    = ch->level;
    af.duration    = number_fuzzy(ch->level / 8);
    af.modifier    = UMAX(1,ch->level/3);
    af.bitvector     = AFF_BERSERK;

    af.location    = APPLY_HITROLL;
    affect_to_char(ch,&af);

    af.location    = APPLY_DAMROLL;
    affect_to_char(ch,&af);

    af.modifier    = UMAX(10,10 * (ch->level/ 8));
    af.location    = APPLY_AC;
    affect_to_char(ch,&af);
*/
    //Start of Rampage Code
    if ( ch->fighting )
    {
      CHAR_DATA *victim;
      int rchance,i,damtype,damdone;
      OBJ_DATA *obj_weapon;

      victim = ch->fighting;

      if (victim == NULL)
      {
        send_to_char("But you aren't fighting anyone!\n\r",ch);
        return;
      }

      if ( (victim->in_room != ch->in_room)
      ||   (victim == ch) )
      { // always account for the "impossible"
        send_to_char("Your target appears to have left!\n\r",ch);
        return;
      }

      rchance = get_skill(ch,gsn_rampage);

      if ( rchance <= 0 )
        return; //silently quit

      if ((obj_weapon = get_eq_char( ch, WEAR_WIELD )) != NULL )
        damtype = attack_table[obj_weapon->value[3]].damage;
      else
        damdone = hand_skill_damage(ch, victim, &damtype); // easiest way to set damtype, damdone is junk value

      // these chance mods were blatantly stolen from do_gore:
      rchance += get_curr_stat(ch,STAT_DEX);
      rchance -= get_curr_stat(victim,STAT_DEX);

      if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        rchance += 10;

      rchance += (ch->level - victim->level);

      if (rchance <= 0)
        return;

			
			time_length = number_fuzzy(ch->level / 16 );
      act("Your berserk drives you into a rampage and you attack $N!",    ch, NULL, victim, TO_CHAR);
      act("$n is driven by their rage and runs straight at you in a rampage!",  ch, NULL, victim, TO_VICT);
      act("$n is driven by their rage and runs straight at $N in a rampage!",ch, NULL, victim, TO_NOTVICT);

      for (i = 0; i <= (ch->level/20); i++) // 1-5, based on level
      {
        if (rchance > number_percent())
        {
          if (obj_weapon)
          {
            damdone = dice(obj_weapon->value[1],obj_weapon->value[2]) * rchance/100;
          }
          else // hand_skill rampage damage far more effective w/o a 5x reduction (a carefully studied guess)
            damdone = (hand_skill_damage(ch, victim, &damtype) / 5) * rchance/100;
        }
        else
          damdone = 0;

        damdone = damdone * 15; // needs a major boost
        damage(ch,victim,damdone,gsn_rampage,damtype,TRUE, FALSE);
      }

      check_improve(ch,gsn_rampage,TRUE,1);
      af.where    = TO_AFFECTS;
    	af.type        = gsn_berserk;
    	af.level    = ch->level;
    	af.duration    = time_length; // number_fuzzy(ch->level / 8);
    	af.modifier    = UMAX(1,ch->level/3);
    	af.bitvector     = AFF_BERSERK;

    	af.location    = APPLY_HITROLL;
    	affect_to_char(ch,&af);

    	af.location    = APPLY_DAMROLL;
    	affect_to_char(ch,&af);

    	af.modifier    = UMAX(10,10 * (ch->level/ 8));
    	af.location    = APPLY_AC;
    	affect_to_char(ch,&af);
			//WAIT_STATE(ch,4 * PULSE_VIOLENCE);
      return;
    }

		af.where    = TO_AFFECTS;
    af.type        = gsn_berserk;
    af.level    = ch->level;
    af.duration    = time_length; // number_fuzzy(ch->level / 8);
    af.modifier    = UMAX(1,ch->level/3);
    af.bitvector     = AFF_BERSERK;

    af.location    = APPLY_HITROLL;
    affect_to_char(ch,&af);

    af.location    = APPLY_DAMROLL;
    affect_to_char(ch,&af);

    af.modifier    = UMAX(10,10 * (ch->level/ 8));
    af.location    = APPLY_AC;
    affect_to_char(ch,&af);

  }
  else //berserk failed
  {
    WAIT_STATE(ch,3 * PULSE_VIOLENCE);
    ch->mana -= 25;
    ch->move /= 2;

    send_to_char("Your pulse speeds up, but nothing happens.\n\r",ch);
    check_improve(ch,gsn_berserk,FALSE,2);
  }
}

void do_bash( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument(argument,arg);
 
  if (IS_GHOST(ch))
  {
    send_to_char("Bashing is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( (chance = get_skill(ch,gsn_bash)) == 0
  ||     (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))
  ||     (!IS_NPC(ch)
  &&      ch->level < skill_table[gsn_bash].skill_level[ch->gameclass]))
  {    
      send_to_char("Bashing? What's that?\n\r",ch);
      return;
  }
 
  if (arg[0] == '\0')
  {
      victim = ch->fighting;
      if (victim == NULL)
      {
        send_to_char("But you aren't fighting anyone!\n\r",ch);
        return;
      }
  }
  else if ( ( victim = get_char_room_ordered( ch, arg, "mobs" ) ) == NULL)
  {
      send_to_char("They aren't here.\n\r",ch);
      return;
  }

  if (victim->position < POS_FIGHTING)
  {
      act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
      return;
  } 

  if (victim == ch)
  {
      send_to_char("You try to bash your brains out, but fail.\n\r",ch);
      return;
  }

  /* Too tired to bash? */
  if ( !check_movement( ch, victim, gsn_bash, FALSE ) )
  {
      send_to_char("You are too tired.\n\r",ch);
      return;
  }

  if (is_safe(ch,victim, FALSE))
  {
      send_to_char("You cannot bash them here!\n\r",ch);
      return;
  }

  if (check_killsteal(ch,victim))
  {
      send_to_char("Someone else is fighting them!\n\r",ch);
      return;
  }

  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
  {
      act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
      return;
  }

  /* modifiers */

  /* size  and weight */
  chance += ch->carry_weight / 250;
  chance -= victim->carry_weight / 200;

  if (ch->size < victim->size)
    chance += (ch->size - victim->size) * 15;
  else
    chance += (ch->size - victim->size) * 10; 


  /* stats */
  chance += get_curr_stat(ch,STAT_STR);
  chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;
  chance += GET_AC(victim,AC_BASH) /50;

  /* speed */
  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 30;

  /* level */
  chance += (ch->level - victim->level);

  if (!IS_NPC(victim) 
  && chance < get_skill(victim,gsn_dodge) )
  {    /*
      act("$n tries to bash you, but you dodge it.",ch,NULL,victim,TO_VICT);
      act("$N dodges your bash, you fall flat on your face.",ch,NULL,victim,TO_CHAR);
      WAIT_STATE(ch,skill_table[gsn_bash].beats);
      return; */
      chance -= 3 * (get_skill(victim,gsn_dodge) - chance);
  }

  /* now the attack */
  if (number_percent() < chance )
  {
      act("$n sends you sprawling with a powerful bash!",
      ch,NULL,victim,TO_VICT);
      act("You slam into $N, and send $M flying!",ch,NULL,victim,TO_CHAR);
      act("$n sends $N sprawling with a powerful bash.",
      ch,NULL,victim,TO_NOTVICT);
      check_improve(ch,gsn_bash,TRUE,1);

      DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
      WAIT_STATE(ch,skill_table[gsn_bash].beats);
      victim->position = POS_RESTING;
      damage(ch,victim,number_range(2,2 + 2 * ch->size + chance/20),gsn_bash,
         DAM_BASH,(IS_NPC(ch)) ? FALSE : TRUE, FALSE);
      update_pos(victim);
  }
  else
  {
      damage(ch,victim,0,gsn_bash,DAM_BASH,FALSE, FALSE);
      act("You fall flat on your face!",
      ch,NULL,victim,TO_CHAR);
      act("$n falls flat on $s face.",
      ch,NULL,victim,TO_NOTVICT);
      act("You evade $n's bash, causing $m to fall flat on $s face.",
      ch,NULL,victim,TO_VICT);
      check_improve(ch,gsn_bash,FALSE,1);
      ch->position = POS_RESTING;
      WAIT_STATE(ch,skill_table[gsn_bash].beats * 3/2); 
  }
}

void do_dirt( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument(argument,arg);

  if (IS_GHOST(ch)) {
    send_to_char("Dirt kicking is useless as you are still DEAD.\n\r",ch);
    return;
  }

  if (!IS_OUTSIDE(ch) && (ch->in_room->sector_type != SECT_UNDERGROUND))
  {
    send_to_char("There is no dirt inside.\n\r",ch);
    return;
  }

  if (IS_SET(ch->in_room->room_flags,ROOM_UNDER_WATER))
  {
    send_to_char("There is no dirt under water.\n\r",ch);
    return;
  }

  if ( (chance = get_skill(ch,gsn_dirt)) == 0
       ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT))
       ||   (!IS_NPC(ch)
         &&    ch->level < skill_table[gsn_dirt].skill_level[ch->gameclass]))
    {
      send_to_char("You get your feet dirty.\n\r",ch);
      return;
    }

  if (arg[0] == '\0')
    {
      victim = ch->fighting;
      if (victim == NULL)
    {
      send_to_char("But you aren't in combat!\n\r",ch);
      return;
    }
    }

  else if ((victim = get_char_room(ch,arg)) == NULL)
    {
      send_to_char("They aren't here.\n\r",ch);
      return;
    }

  if (IS_AFFECTED(victim,AFF_BLIND))
    {
      act("$E's already been blinded.",ch,NULL,victim,TO_CHAR);
      return;
    }

  if (victim == ch)
    {
      send_to_char("Very funny.\n\r",ch);
      return;
    }

  if ( !check_movement( ch, victim, gsn_dirt, TRUE ) )
    {
        act( "You have run out of movement and cannot kick dirt at $N!",
            ch, NULL, victim, TO_CHAR );
        act( "$n tries to kick dirt at you but is too fatigued!",
            ch, NULL, victim, TO_VICT );
        act( "$n tries to kick dirt at $N but is too fatigued.",
            ch, NULL, victim, TO_NOTVICT );
        return;
    }

  if (is_safe(ch,victim, FALSE))
    return;

  if (check_killsteal(ch,victim))
      return;

  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
      act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
      return;
    }

  /* modifiers */

  /* dexterity */
  chance += get_curr_stat(ch,STAT_DEX);
  chance -= 3.5 * get_curr_stat(victim,STAT_DEX);

  /* speed  */
  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
    chance += 5;
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 25;

  /* level */
  chance += (ch->level - victim->level);

  /* sloppy hack to prevent false zeroes */
  if (chance % 5 == 0)
    chance += 1;

  /* terrain */

  switch(ch->in_room->sector_type)
    {
    case(SECT_INSIDE):         chance -= 20;    break;
    case(SECT_CITY):         chance -= 10;    break;
    case(SECT_FIELD):         chance +=  5;    break;
    case(SECT_FOREST):            break;
    case(SECT_HILLS):            break;
    case(SECT_MOUNTAIN):     chance -= 10;    break;
    case(SECT_WATER_SWIM):     chance  =  0;    break;
    case(SECT_WATER_NOSWIM): chance  =  0;    break;
    case(SECT_AIR):             chance  =  0;      break;
    case(SECT_DESERT):         chance += 10;   break;
    case(SECT_GRAVEYARD):     chance += 10;    break;
    case(SECT_UNDERGROUND):     chance += 10;    break;
    }

  if (chance <= 0)
  {
      send_to_char("You get your feet dirty.\n\r",ch);
      return;
  }

  /* now the attack */
  if (number_percent() < chance)
  {
      AFFECT_DATA af;
      act("$n is blinded by the dirt in $s eyes!",victim,NULL,NULL,TO_ROOM);
      act("$n kicks dirt in your eyes!",ch,NULL,victim,TO_VICT);

      af.where    = TO_AFFECTS;
      af.type     = gsn_dirt;
      af.level     = ch->level;
      af.duration    = 1;
      af.location    = APPLY_HITROLL;
      af.modifier    = -4;
      af.bitvector     = AFF_BLIND;

      affect_to_char(victim,&af);
      damage(ch,victim,number_range(2,5),gsn_dirt,DAM_NONE,FALSE, FALSE);
      send_to_char("You can't see a thing!\n\r",victim);
      check_improve(ch,gsn_dirt,TRUE,2);
      WAIT_STATE(ch,skill_table[gsn_dirt].beats);
  }
  else
  {
      damage(ch,victim,0,gsn_dirt,DAM_NONE,TRUE, FALSE);
      check_improve(ch,gsn_dirt,FALSE,2);
      WAIT_STATE(ch,skill_table[gsn_dirt].beats);
  }
}

void do_trip( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  OBJ_DATA *wield=NULL;

  one_argument(argument,arg);

  if ( IS_GHOST(ch) )
  {
    send_to_char("Tripping is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( (chance = get_skill(ch,gsn_trip) ) == 0
  ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
  ||   (!IS_NPC(ch) 
  && ch->level < skill_table[gsn_trip].skill_level[ch->gameclass]))
  {
    send_to_char("Tripping?  What's that?\n\r",ch);
    return;
  }


  if (arg[0] == '\0')
  {
    victim = ch->fighting;

    if (victim == NULL)
    {
      send_to_char("But you aren't fighting anyone!\n\r",ch);
      return;
    }
  }

  else if ( ( victim = get_char_room(ch,arg) ) == NULL )
  {
      send_to_char("They aren't here.\n\r",ch);
      return;
  }

  //no tripping if tired
  if ( !check_movement( ch, victim, gsn_trip, FALSE ) )
    return;

  if ( is_safe(ch,victim, FALSE) )
    return;

  if ( check_killsteal(ch,victim) )
    return;
    
  if ( IS_AFFECTED(victim,AFF_FLYING) )
  {
    act( "$S feet aren't on the ground.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if (victim->position < POS_FIGHTING)
  {
    act( "$N is already down.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if (victim == ch)
  {
    send_to_char( "You fall flat on your face!\n\r", ch );
    WAIT_STATE( ch, 2 * skill_table[gsn_trip].beats );
    act( "$n trips over $s own feet!", ch, NULL, NULL, TO_ROOM );
    return;
  }

  if ( IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim )
  {
    act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
    return;
  }

  /* modifiers */
  /* size */
  if ( ch->size < victim->size )
    chance += ( ch->size - victim->size ) * 10;  /* bigger = harder to trip */

  /* dex */
  chance += get_curr_stat( ch, STAT_DEX );
  chance -= get_curr_stat( victim, STAT_DEX ) * 3 / 2;

  /* speed */
  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 20;

  /* level */
  chance += (ch->level - victim->level) * 2;


  /* now the attack */
  if (number_percent() < chance)
  {
    //Begin Sweep Check
    //have sweep high enough %. Have dagger. Trip Above 85%
    wield = get_eq_char( ch, WEAR_WIELD );

    if ( wield
    && ( ch->level >= skill_table[gsn_sweep].skill_level[ch->gameclass] )
    && ( number_percent() < get_skill(ch,gsn_sweep) )
    && ( wield->value[0] == WEAPON_DAGGER ) )
    {
      //Sweep Success
      act("$n sweeps you off your feet, thrusting a dagger into you!",ch,NULL,victim,TO_VICT);
      act("You sweep $N to the ground, thrusting a dagger into $M!", ch, NULL, victim, TO_CHAR);
      act("$n sweeps $N off $S feet, thrusting a dagger into $M.", ch, NULL, victim, TO_NOTVICT);
      check_improve( ch, gsn_sweep, TRUE, 1 );

      DAZE_STATE( victim, 3 * PULSE_VIOLENCE );
      WAIT_STATE( ch, skill_table[gsn_sweep].beats );
      victim->position = POS_RESTING;
      damage( ch, victim, (dice(wield->value[1],wield->value[2]) * get_skill(ch, gsn_sweep)/100), gsn_trip,
        DAM_BASH, TRUE, FALSE );
    }
    else
    {
      //Trip instead
      act("$n trips you and you go down!",ch,NULL,victim,TO_VICT);
      act("You trip $N and $N goes down!",ch,NULL,victim,TO_CHAR);
      act("$n trips $N, sending $M to the ground.",ch,NULL,victim,TO_NOTVICT);
      check_improve(ch,gsn_trip,TRUE,1);

      DAZE_STATE(victim, 2.5 * PULSE_VIOLENCE);
      WAIT_STATE(ch,skill_table[gsn_trip].beats);
      victim->position = POS_RESTING;
      damage(ch,victim,number_range(2, 2 +  2 * victim->size),gsn_trip,
        DAM_BASH,TRUE, FALSE);
    }
  }
  else
  {
    damage(ch,victim,0,gsn_trip,DAM_BASH,TRUE, FALSE);
    WAIT_STATE(ch,skill_table[gsn_trip].beats*2/3);
    check_improve(ch,gsn_trip,FALSE,1);
  } 
}

void do_kill( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if (IS_GHOST(ch)) {
    send_to_char("Killing is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if (IS_LINKDEAD(ch)) {
    send_to_char("Holy Toledo. You are LINKDEAD...not anymore\n\r",ch);
    REMOVE_BIT(ch->affected_by, AFF_LINKDEATH);
    return;
  }
  if ( arg[0] == '\0' )
    {
      send_to_char( "Kill whom?\n\r", ch );
      return;
    }

  if ( ( victim = get_char_room_ordered( ch, arg, "mobs" ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  /*  Allow player killing*/
  if ( !IS_NPC(victim) )
    {
      if ( !IS_SET(victim->act, PLR_KILLER)
       && !IS_SET(victim->act, PLR_THIEF)
       && !IS_SET(victim->act, PLR_VIOLENT)
       && !IS_SET(victim->act, PLR_TWIT))
        {
      send_to_char( "You must MURDER a player.\n\r", ch );
      return;
        }
    }

  if ( victim == ch )
    {
      send_to_char( "You hit yourself.  Ouch!\n\r", ch );
      multi_hit( ch, ch, TYPE_UNDEFINED );
      return;
    }

  if ( is_safe( ch, victim, FALSE ) )
  {
    send_to_char( "It wouldn't be wise to attack someone here.\n\r",ch);
    return;
  }

  if ( is_same_group( ch, victim ) )
  {
    send_to_char( "You can't attack a group member.\n\r",ch);
    return;
  }

  if (check_killsteal(ch,victim))
      return;
  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
      act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
      return;
    }

  if ( ch->position == POS_FIGHTING )
    {
      send_to_char( "You are already fighting! You are doing the best you can!\n\r", ch );
      return;
    }

  WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
  multi_hit( ch, victim, TYPE_UNDEFINED );
  return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
  send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
  return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
      send_to_char( "Murder whom?\n\r", ch );
      return;
    }

  if (IS_GHOST(ch)) {
    send_to_char("Murdering is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  //If it's a mob that can't see, just return.
  if ( IS_NPC(ch) && !can_see(ch, victim) )
    return;

  if (IS_GHOST(victim)) {
    send_to_char("Your abilities are useless against this ghost.\n\r",ch);
    return;
  }
  if ( victim == ch )
    {
      send_to_char( "Suicide is a mortal sin.\n\r", ch );
      return;
    }

  if (IS_NPC(victim)) {
    send_to_char("You can only murder a master player.\n\r",ch);
    return;
  }

  if ( is_safe( ch, victim, TRUE ) )
    return;

  if (check_killsteal(ch,victim))
      return;

  if ( is_in_pk_range(ch, victim) )
  {
    send_to_char( "You can't attack them.\n\r", ch );
    return;
  } 

  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
      act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
      return;
    }

  if ( ch->position == POS_FIGHTING )
    {
      send_to_char( "You do the best you can!\n\r", ch );
      return;
    }

  WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
  if (IS_NPC(ch))
    mprintf(sizeof(buf), buf, "Help! I am being attacked by %s!",ch->short_descr);
  else
    mprintf(sizeof(buf), buf, "Help!  I am being attacked by %s!", ch->name );
  if (!IS_SET(ch->in_room->room_flags, ROOM_ARENA)) {
    do_function( victim,&do_yell, buf );
    do_function( victim,&do_clantalk, buf );
  }
  multi_hit( ch, victim, TYPE_UNDEFINED );
  return;
}

void do_backstab( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  one_argument( argument, arg );

  if (get_skill(ch,gsn_backstab) < 1 )
  {
    send_to_char("Huh?\n\r",ch);
    return;
  }
    
  if (IS_GHOST(ch)) {
    send_to_char("Backstabbing is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if (arg[0] == '\0')
    {
      send_to_char("Backstab whom?\n\r",ch);
      return;
    }

  if (ch->fighting != NULL)
    {
      send_to_char("You're facing the wrong end.\n\r",ch);
      return;
    }
 
  else if ( ( victim = get_char_room_ordered( ch, arg, "mobs" ) ) == NULL )
  {
    send_to_char("They aren't here.\n\r",ch);
    WAIT_STATE( ch, skill_table[gsn_backstab].beats /2 );
    return;
  }

  if ( victim == ch )
  {
    send_to_char( "How can you sneak up on yourself?\n\r", ch );
    return;
  }

  //No backstabing when tired.
  if ( !check_movement( ch, victim, gsn_backstab, FALSE ) )
    return;

  if (is_safe(ch,victim, TRUE)) {
    send_to_char("Sorry, but you cannot attack this person.\n\r",ch);
    return;
  }    

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You're blind as a bat.  How can you expect to see well enough to backstab someone?\n\r", ch);
    return;
  }

  if (ch->bs_flag) {
    send_to_char("You have already backstabbed this fight...leave it be.\n\r",ch);
    return;
  }
    
  if (check_killsteal(ch,victim))
      return;

  if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
  {
    send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
    return;
  }

  if ( victim->hit < victim->max_hit / 3)
  {
    act( "$N is hurt and suspicious...you can't sneak up.",
      ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( ( ( IS_NPC( ch ) && ch->master ) //charmed mob (attacker)
  ||       !IS_NPC( ch ) )              //player (attacker)
  &&   !IS_NPC( victim ))             //player (victim)
  {
    mprintf(sizeof(buf), buf,
      "{R%s{x is attempting to backstab {R%s{x at {g%s {x[{g%d{x]",
      ch->name,
      victim->name,
      victim->in_room->name,
      victim->in_room->vnum );
    wiznet(buf,ch,NULL,WIZ_MURDER,0,0);
  }

  WAIT_STATE( ch, skill_table[gsn_backstab].beats );
  if ( number_percent( ) < get_skill(ch,gsn_backstab)
  || ( get_skill(ch,gsn_backstab) >= 2 && !IS_AWAKE(victim) ) )
  {
      check_improve(ch,gsn_backstab,TRUE,1);
      multi_hit( ch, victim, gsn_backstab);
  }
  else
  {
      check_improve(ch,gsn_backstab,FALSE,1);
      damage( ch, victim, 0, gsn_backstab,DAM_NONE,TRUE, FALSE);
  }

  return;
}

void do_flee( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *was_in;
  ROOM_INDEX_DATA *now_in;
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  int attempt;

  if ( ( victim = ch->fighting ) == NULL )
  {
    if ( ch->position == POS_FIGHTING )
      ch->position = POS_STANDING;
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }

  if ( is_affected( ch, skill_lookup("entangle") ) )
  {
    send_to_char( "You are too entangled to escape.\n\r", ch );
    return;
  }

  if ( is_affected( ch, skill_lookup("confine") ) )
  {
    send_to_char( "You feel confined to this place.\n\r", ch );
    return;
  }

  if ( IS_AFFECTED(ch,AFF_SLOW) )
  {
    if ( number_range( 1,2 ) == 1 )
    {
      send_to_char( "You feel too tired right now.\n\r", ch );
      return;
    }
  }

  was_in = ch->in_room;
  for ( attempt = 0; attempt < 6; attempt++ )
  {
      if (ch->hit < 1)
        continue;

      EXIT_DATA *pexit;
      int door=0;

      door = number_door( );

      if ( ( IS_CLASS_HIGHWAYMAN( ch ) )
      &&     !IS_NULLSTR( argument ) )
      {
        if ( !str_prefix( argument, "north" ) ) { door = DIR_NORTH; }
        else if ( !str_prefix( argument, "east"  ) ) { door = DIR_EAST; }
        else if ( !str_prefix( argument, "south" ) ) { door = DIR_SOUTH; }
        else if ( !str_prefix( argument, "west"  ) ) { door = DIR_WEST; }
        else if ( !str_prefix( argument, "up"    ) ) { door = DIR_UP; }
        else if ( !str_prefix( argument, "down"  ) ) { door = DIR_DOWN; }
        else
        {
            send_to_char( "That is not a valid direction.\n\r", ch );
            return;
        }
        // changed mind about 100% Hwy success
        if (number_percent() > 50) // remember, it tries 6 times, so 50% here is 1.5625% failure rate...
          continue;
      }


      if ( IS_SET( was_in->room_flags, ROOM_SHIP ) && door != was_in->state )
        continue;
      else
      {
        if ( ( pexit = was_in->exit[door] ) == NULL
        ||     IS_SET( pexit->exit_info, EX_NOEXIT )
        ||     pexit->u1.to_room == NULL
        || (   IS_SET( pexit->exit_info, EX_CLOSED )
        && (  !IS_AFFECTED( ch, AFF_PASS_DOOR )
        ||     IS_SET( pexit->exit_info, EX_NOPASS ) ) )
        ||     number_range( 0, ch->daze )
        ||   ( IS_NPC( ch )
        &&   ( IS_SET( pexit->u1.to_room->room_flags, ROOM_NO_MOB )
        ||     ( IS_SET( pexit->exit_info, EX_FENCED )
        &&     ( ch->pIndexData->vnum != MOB_VNUM_QUEST ) ) ) ) )
          continue;
      }

      if ( IS_NPC( ch ) // attempt two!
      &&   IS_SET( pexit->exit_info, EX_FENCED )
      &&   ( ch->pIndexData->vnum != MOB_VNUM_QUEST ) )
        continue;


      if ( IS_SET( was_in->room_flags, ROOM_SHIP ) )
      {
        for ( door = 0; door <= 5; door++ )
          if ( !strcmp( dir_name[door], was_in->exit[was_in->state]->keyword ) )
            break;
      }

      move_char( ch, door, FALSE );

      if ( ( now_in = ch->in_room ) == was_in )
        continue;

      ch->in_room = was_in;
      act( "$n has fled!", ch, NULL, NULL, TO_ROOM );

      if ( (ch->pet != NULL) && (ch->in_room == ch->pet->in_room) ){ // pets auto flee
        act( "$n has fled!", ch->pet, NULL, NULL, TO_ROOM );
        move_char( ch->pet, door, FALSE );
        ch->pet->position = POS_STANDING;
      }

      ch->in_room = now_in;

      if ( !IS_NPC( ch ) && !IS_SET( was_in->room_flags, ROOM_ARENA ) )
      {
        send_to_char( "You flee from combat!\n\r", ch );
        if ((ch->pet != NULL) && (ch->in_room == ch->pet->in_room) && ( can_see(ch,ch->pet) ) ) { // pets auto flee
          strcpy(buf, ch->pet->short_descr);
          strcat(buf, " has fled from combat!\n\r");
          send_to_char( buf, ch);
        }
        if ( ( ch->gameclass == 2 )
        && ( number_percent() < 3 * ( ch->level / 2 ) ) )
          send_to_char( "You snuck away safely.\n\r", ch);
        else
        {
          if (( ch->questdata->curr_points > 0 )
          ||  ( ch->questdata->glory > 0 ) )
            send_to_char( "You lost 1 glory.\n\r", ch );

          if ( ch->questdata->curr_points > 0 )
            ch->questdata->curr_points -= 1;
          if ( ch->questdata->glory > 0 )
            ch->questdata->glory -= 1;
        }
    }
/* maybe this will prevent mobs from attacking wrong target in wierd circumstances */
    bool quitit = TRUE;
    CHAR_DATA *rch;
    for (rch = victim->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
      if (rch->fighting == victim)
      {
        quitit = FALSE;
        continue;
      }
    }
/* end of taeloch's wrong-target fix */

    stop_fighting( ch, TRUE );
    if (ch->pet != NULL) { stop_fighting( ch->pet, TRUE ); }

    if (quitit)
      stop_fighting( victim, TRUE );

    return;
  }
  send_to_char( "PANIC! You couldn't escape!\n\r", ch );
  return;
}



void do_rescue( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *fch;

  one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
      send_to_char( "Rescue whom?\n\r", ch );
      return;
    }

  if (IS_GHOST(ch)) {
    send_to_char("Rescueing is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  if ( victim == ch )
    {
      send_to_char( "What about fleeing instead?\n\r", ch );
      return;
    }

  if ( !check_movement( ch, victim, TYPE_UNDEFINED, TRUE ) )
    {
        act( "You have run out of movement and cannot rescue $N!",
            ch, NULL, victim, TO_CHAR );
        act( "$n tries to rescue you but is too fatigued!",
            ch, NULL, victim, TO_VICT );
        act( "$n tries to rescue $N but is too fatigued.",
            ch, NULL, victim, TO_NOTVICT );
        return;
    }

  if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
      if ( (!IS_PET(victim) ) || (victim->master != ch) )
    {
          send_to_char( "Doesn't need your help!\n\r", ch );
          return;
    }
    }

  if ( ch->fighting == victim )
    {
      send_to_char( "Too late.\n\r", ch );
      return;
    }
   
  if ( ( fch = victim->fighting ) == NULL )
    {
      send_to_char( "That person is not fighting right now.\n\r", ch );
      return;
    }

  if (!is_same_group(ch, victim))
    {
      send_to_char("That person is not in your group.\n\r",ch);
      return;
    }

  if (is_safe(ch,fch, FALSE)) {
      send_to_char("You cannot rescue them...they are in a fight with someone you would rather not fight.\n\r",ch);
      return;
  }
  if (check_killsteal(ch,victim->fighting))
      return;

  WAIT_STATE( ch, skill_table[gsn_rescue].beats );
  if ( number_percent( ) > get_skill(ch,gsn_rescue))
    {
      send_to_char( "You fail the rescue.\n\r", ch );
      check_improve(ch,gsn_rescue,FALSE,1);
      return;
    }

  act( "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
  act( "$n rescues you!", ch, NULL, victim, TO_VICT    );
  act( "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );
  check_improve(ch,gsn_rescue,TRUE,1);

  stop_fighting( fch, FALSE );
  stop_fighting( victim, FALSE );
  fch->bs_flag = TRUE;
  //victim->bs_flag = TRUE; //No reason rescue should stop BS for now.
  check_killer( ch, fch );
  if ( ch->fighting == NULL )
    set_fighting( ch, fch );
  set_fighting( fch, ch );
  return;
}



void do_kick( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;

  if ( !IS_NPC(ch)
  && ( ch->level < skill_table[gsn_kick].skill_level[ch->gameclass] )
  && ( ch->race != race_lookup("centaur") ) ) // centaurs get kick racially
  {
    send_to_char( "You have not yet learned to focus your kicks.\n\r", ch );
    return;
  }

  if ((get_skill(ch,gsn_kick)) < 1) {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  
  if (IS_GHOST(ch)) {
    send_to_char("Kicking is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK))
    return;

  if ( is_affected( ch, skill_lookup( "nerve pinch" ) ) )
  {
    send_to_char( "It's hard enough catching your breath.\n\r", ch );
    return;
  }

  if ( ( victim = ch->fighting ) == NULL )
    {
      send_to_char( "You aren't fighting anyone.\n\r", ch );
      return;
    }

  WAIT_STATE( ch, skill_table[gsn_kick].beats );
  if ( get_skill(ch,gsn_kick) > number_percent())
    {
      damage( ch, victim, number_range( 1, ch->level ),
         gsn_kick, DAM_BASH, TRUE, FALSE );
      check_improve( ch, gsn_kick, TRUE, 1 );
    }
  else
    {
      damage( ch, victim, 0, gsn_kick, DAM_BASH, TRUE, FALSE );
      check_improve( ch, gsn_kick, FALSE, 1 );
    }
  return;
}




void do_disarm( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;

  hth = 0;

  if (IS_GHOST(ch)) {
    send_to_char("Disarming is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }
  if ((chance = get_skill(ch,gsn_disarm)) == 0)
    {
      send_to_char( "You don't know how to disarm opponents.\n\r", ch );
      return;
    }
/*
  if ( get_eq_char( ch, WEAR_WIELD ) == NULL 
       &&   ((hth = get_skill(ch,gsn_hand_to_hand)) == 0
         ||    (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_DISARM))))
    {
      send_to_char( "You must wield a weapon to disarm.\n\r", ch );
      return;
    }
*/
  if ( ( victim = ch->fighting ) == NULL )
    {
      send_to_char( "You aren't fighting anyone.\n\r", ch );
      return;
    }

  if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
      send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
      return;
    }

  if ( !check_movement( ch, victim, TYPE_UNDEFINED, TRUE ) )
    {
        act( "You have run out of movement and cannot disarm $N!",
            ch, NULL, victim, TO_CHAR );
        act( "$n tries to disarm you but is too fatigued!",
            ch, NULL, victim, TO_VICT );
        act( "$n tries to disarm $N but is too fatigued.",
            ch, NULL, victim, TO_NOTVICT );
        return;
    }

  /* find weapon skills */
  ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch));
  vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim));
  ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim));

  /* modifiers */

  /* skill */
  if ( get_eq_char(ch,WEAR_WIELD) == NULL)
    chance = chance * hth/150;
  else
    chance = chance * ch_weapon/100;

  chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

  /* dex vs. strength */
  chance += get_curr_stat(ch,STAT_DEX);
  chance -= 2 * get_curr_stat(victim,STAT_STR);

  /* level */
  chance += (ch->level - victim->level) * 2;
 
  /* and now the attack */
  if (number_percent() < chance)
    {
      WAIT_STATE( ch, skill_table[gsn_disarm].beats );
      disarm( ch, victim );
      check_improve(ch,gsn_disarm,TRUE,1);
    }
  else
    {
      WAIT_STATE(ch,skill_table[gsn_disarm].beats);
      act("You fail to disarm $N.",ch,NULL,victim,TO_CHAR);
      act("$n tries to disarm you, but fails.",ch,NULL,victim,TO_VICT);
      act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
      check_improve(ch,gsn_disarm,FALSE,1);
    }
  check_killer(ch,victim);
  return;
}
/* Ancient removing it as game balance annoying */
void do_surrender( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *mob;
  if ( (mob = ch->fighting) == NULL )
    {
      send_to_char( "But you're not fighting!\n\r", ch );
      return;
    }
  act( "You surrender to $N!", ch, NULL, mob, TO_CHAR );
  act( "$n surrenders to you!", ch, NULL, mob, TO_VICT );
  act( "$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT );
  stop_fighting( ch, TRUE );

  if ( !IS_NPC( ch ) && IS_NPC( mob ) 
       &&   ( !HAS_TRIGGER( mob, TRIG_SURR ) 
          || !mp_percent_trigger( mob, ch, NULL, NULL, TRIG_SURR ) ) )
    {
      act( "$N seems to ignore your cowardly act!", ch, NULL, mob, TO_CHAR );
      multi_hit( mob, ch, TYPE_UNDEFINED );
    }
}

void do_sla( CHAR_DATA *ch, char *argument )
{
  send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
  return;
}

void do_slay( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  one_argument( argument, arg );

  if ( (ch->level < L3 )
  &&   !IS_BUILDER(ch,ch->in_room->area ) )
  {
    send_to_char( "You can only slay in your own areas at your level.\n\r", ch );
    return;
  }

  if ( arg[0] == '\0' )
    {
      send_to_char( "Slay whom?\n\r", ch );
      return;
    }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  if ( ch == victim )
    {
      send_to_char( "Suicide is a mortal sin.\n\r", ch );
      return;
    }

  if ( !IS_NPC(victim) && victim->level >= get_trust(ch) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }

  if ( !IS_NPC(victim)
  && (ch->level < L3 ) )
  {
    send_to_char( "You can't slay players at your level!\n\r", ch );
    return;
  }

  act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR    );
  act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
  act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );

  victim->position = POS_FIGHTING; // keeps mobs from crashing the MUD! - Taeloch
  raw_kill( victim, ch );
  return;
}

void do_stake( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument(argument,arg);
 
  if (IS_GHOST(ch)) {
    send_to_char("Staking is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if (( chance = get_skill(ch,gsn_stake)) < 1) {
    send_to_char("Whats that?  Stake?  You hungry?\n\r",ch);
    return;
  }
  
      
  if (ch->level < skill_table[gsn_stake].skill_level[ch->gameclass])
    {    
      send_to_char("Stake?  What's that?\n\r",ch);
      return;
    }


  if (arg[0] == '\0')
    {
      victim = ch->fighting;
      if (victim == NULL)
    {
      send_to_char("Stake what undead??\n\r",ch);
      return;
    }
    }

  else if ((victim = get_char_room(ch,arg)) == NULL)
    {
      send_to_char("They aren't here.\n\r",ch);
      return;
    }

  if (!IS_NPC(victim) && victim->race != race_lookup("vampire"))
    {
      send_to_char("You cannot stake a non-vampire player.\n\r",ch);
      return;
    }
  if (IS_NPC(victim) && (!is_name("vampire",victim->name) &&
             !is_name("undead", victim->name) &&
             !is_name("skeleton", victim->name) &&
             !is_name("zombie", victim->name) &&
             !is_name("ghost", victim->name) &&
             !is_name("rotting", victim->name) &&
             !is_name("corpse", victim->name)  &&
             !IS_SET(victim->act, ACT_UNDEAD))) {

    send_to_char("You cannot stake this mob.\n\r",ch);
    return;
  }

  if (victim == ch)
    {
      send_to_char("You aren't undead...you cannot stake yourself.\n\r",ch);
      return;
    }

  if (is_safe(ch,victim, FALSE))
    return;

  if (check_killsteal(ch,victim))
      return;
  
  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
      act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
      return;
    }

  /* modifiers */

  if (ch->size < victim->size)
    chance += (ch->size - victim->size) * 15;
  else
    chance += (ch->size - victim->size) * 10; 


  /* stats */
  chance += GET_AC(victim,AC_PIERCE) /50;
  /* speed */
  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 10;

  /* level */
  chance += (ch->level - victim->level);

  /* now the attack */
  if (number_percent() < chance )
    {
    
      act("$n has shoved a stake into your {rh{Re{rart{x!",
      ch,NULL,victim,TO_VICT);
      act("You slam a stake into $N!",ch,NULL,victim,TO_CHAR);
      act("$n shoves a stake into $N.",
      ch,NULL,victim,TO_NOTVICT);
      check_improve(ch,gsn_stake,TRUE,1);

      DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
      WAIT_STATE(ch,skill_table[gsn_stake].beats);
      victim->position = POS_RESTING;
    
      damage(ch,victim,2*(dice((int)ch->level,10) * ch->level/12),gsn_stake,
         DAM_PIERCE,TRUE, FALSE);
    
    }
  else
    {
      damage(ch,victim,0,gsn_stake,DAM_PIERCE,TRUE, FALSE);
      act("Your stake misses its mark!",
      ch,NULL,victim,TO_CHAR);
      act("$n's stake misses his victim.",
      ch,NULL,victim,TO_NOTVICT);
      act("You nimbly evade $n's stake.",
      ch,NULL,victim,TO_VICT);
      check_improve(ch,gsn_stake,FALSE,1);
      ch->position = POS_RESTING;
      WAIT_STATE(ch,skill_table[gsn_stake].beats * 3/2); 
    }
}

int switch_update(CHAR_DATA *ch)
{
  CHAR_DATA *vch, *vch_next;
  /* switch for group */
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
  {
    vch_next =vch->next_in_room;
    
    if ( ( vch->fighting == ch )
    &&   ( vch != ch->fighting ) )
    {
        stop_fighting(ch,FALSE);
        set_fighting(ch,vch);
        return(0);
    }
  }

  return(0);
}

bool is_hating(CHAR_DATA *ch, CHAR_DATA *victim)
{
  if (!ch->hate)
    return FALSE;
  if (!ch->hate->name)
    return FALSE;
  if (!str_cmp(ch->hate->name, victim->name))
    return TRUE;
  return FALSE;
}
void stop_hating(CHAR_DATA *ch)
{
  if (ch->hate)
    {
      free_string(ch->hate->name );
#if OLD_MEM
      free_mem(ch->hate, sizeof (HATE_DATA));
#else
      free_mem(ch->hate);
#endif
      ch->hate = NULL;
    }
  return;
}
void stop_hating_ch(CHAR_DATA *ch)
{
  CHAR_DATA *vch, *vch_next;

  for (vch=char_list; vch != NULL; vch = vch_next) {
    if (vch->next == NULL)
      return;
    vch_next = vch->next;
    if (is_hating(vch,ch))
      stop_hating(vch);
  }    
}
void start_hating(CHAR_DATA *ch, CHAR_DATA *victim)
{
  if (ch == victim)
    return;
  if (IS_NPC(ch) && IS_NPC(victim))
    if (ch->pIndexData->vnum == victim->pIndexData->vnum)
      return;
  if (ch->hate)
    stop_hating(ch);
  
  ch->hate = alloc_mem(sizeof( HATE_DATA));
  ch->hate->name = str_dup(victim->name, "");
  ch->hate->who = victim;
#if MEMDEBUG
    memdebug_check(victim,"fight: start hating");
#endif
  return;
}
void set_plevel(CHAR_DATA *ch, CHAR_DATA *victim)
{
  if (ch == victim)
    return;
  if (ch->hit >= (ch->max_hit * 0.80))
      ch->plevel = victim;
      
  return;
}

bool check_safe(CHAR_DATA *ch, CHAR_DATA *victim, bool spell, bool area, bool showflag)
{
  if (IS_GHOST(victim)) {
    show_to_char("Your victim is a ghost...you would have no affect on him.\n\r",ch,showflag);
    return TRUE;
  }
  if (IS_GHOST(ch)) {
    show_to_char("You are a ghost...you would have no affect on him.\n\r",ch,showflag);
    return TRUE;
  }

  if (spell) {
    if (victim == ch && area)
      return TRUE;
  }

  if (victim->in_room == NULL || ch->in_room == NULL)
    {
      show_to_char("You are not in the room with them.\n\r",ch,showflag);
      return TRUE;
    }

  if (IS_LINKDEAD(victim) ||  IS_AFK(victim)) {
    stop_fighting(victim,TRUE);
    show_to_char("That person is currently unavailable.\n\r",ch,showflag);
    return TRUE;
  }

  if (IS_LINKDEAD(ch)) {
    stop_fighting(ch,TRUE);
    return TRUE;
  }

  if (IS_AFK(ch)) {
    stop_fighting(ch,TRUE);
    show_to_char("YOU ARE AFK!!\n\r",ch,showflag);
    return TRUE;
  }

  if (IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL)
    return FALSE;
  /* purely for arena work. */

  if (IS_SET(victim->in_room->room_flags, ROOM_ARENA)) {
    if (!is_same_group(ch, victim))
      return FALSE;
  }
  
  if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) 
  {
    if (victim->desc != NULL)
      if (( victim->desc->original == NULL ) && (can_see(ch, victim)))
    {
      show_to_char("Attacking a God is NOT Smart.\n\r",ch,showflag);
      return TRUE;
    }
  }

  if( is_same_group(ch, victim))
  {
    if(victim->desc != NULL)
       if ( victim->desc->original == NULL )
       {
           if ( ch != victim )
           {
             if (spell)
               show_to_char("A group member gets behind you for the spell.\n\r",ch,showflag);
             else
               show_to_char("A group member ducks under your attack.\n\r",ch,showflag);
           }
           return TRUE;
       }
  }

  /* killing mobiles */
  if (IS_NPC(victim))
    {
      /* safe room? */
      if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
    {
      show_to_char("Not in this room.\n\r",ch,showflag);
      return TRUE;
    }

      /*      if (victim->pIndexData->pShop != NULL)
       */
      if ( IS_SHOPKEEPER(victim)
      &&  !IS_BUILDER(ch, ch->in_room->area ) )
    {
      show_to_char("The shopkeeper wouldn't like that.\n\r",ch,showflag);
      return TRUE;
    }

    /* no killing healers, trainers, etc */
    if (IS_SET(victim->act,ACT_TRAIN)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_IS_HEALER)
    ||  IS_SET(victim->act,ACT_IS_CHANGER)
    || (victim->spec_fun == spec_lookup( "spec_questmaster" )))
    {
      if ( ch->clan )
        act( "I don't think $g would approve.", ch, NULL, NULL, TO_CHAR );
      else
        show_to_char("I don't think {CM{cir{Ml{mya{x would approve.\n\r",ch,showflag);
      return TRUE;
    }

      if (!IS_NPC(ch))
    {
      /* no pets.. ie: unless they are already attacking you */
      if ((IS_PET(victim)
      || (IS_AFFECTED(victim,AFF_CHARM)))
      && (victim->master != NULL)
      && (ch->fighting != victim)
      && (victim->fighting != ch)
      && (!is_in_pk_range(ch,victim->master))) 
      {
        if (!showflag)
            act("But $N looks like you can't touch them...",
              ch,NULL,victim,TO_CHAR);
        return TRUE;
      }

    if ( is_same_group(ch, victim->master)
    ||   is_same_group(ch, victim) )
    {
      if (!showflag)
        act("$N is a part of your group!",
          ch,NULL,victim,TO_CHAR);
      return TRUE;
    }

      /* legal kill? -- cannot hit mob fighting non-group member */
      if (victim->fighting != NULL
          && !is_same_group(ch,victim->fighting)
          && victim->fighting != ch
          && ch->fighting != victim) {
        show_to_char("Kill stealing is not permitted.\n\r",ch,!showflag);
        return TRUE;
      }
    }
      else
    {
      if (spell) {

        /* area effect spells do not hit other mobs */
        if (area && !is_same_group(victim,ch->fighting))
          return TRUE;
      }
      
      if ((victim->master != NULL)
          && (ch->master != NULL)
          && !IS_NPC(victim->master)
          && !IS_NPC(ch->master)    
          && !is_same_group(victim,ch)
          && !is_in_pk_range(ch->master, victim->master)) {
        show_to_char("NO WAY JOSE.\n\r",ch, showflag);
        return TRUE;
      }
    }
    }
  /* killing players */
  else
    {
      if (spell) {
    if (area && IS_IMMORTAL(victim) && victim->level > LEVEL_IMMORTAL)
      return TRUE;
      }
      /* NPC doing the killing */
      if (IS_NPC(ch))
    {
      /* charmed mobs and pets cannot attack players while owned */
      if ((IS_AFFECTED(ch,AFF_CHARM)
           || IS_PET(victim))
          && ch->master != NULL
          && ch->master->fighting != victim
          && ch->fighting != victim
          && victim->fighting != ch
          && !is_in_pk_range(ch->master, victim))
        {
          show_to_char("Players are your friends!\n\r",ch,showflag);
          return TRUE;
        }
    
      /* safe room? */
      if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
        {
          show_to_char("Not in this room.\n\r",ch,showflag);
          return TRUE;
        }

      /* legal kill? -- mobs only hit players grouped with opponent*/
      if (ch->fighting != NULL && !is_same_group(ch->fighting,victim))
        return TRUE;
    }

      /* player doing the killing */
      else
    {
      /* safe room? */
      if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
        {
          show_to_char("Not in this room.\n\r",ch,showflag);
          return TRUE;
        }
      if (victim->pcdata->old_char || ch->pcdata->old_char)
        {
          show_to_char("Sorry, You cannot Attack due to OLD Style Character restrictions.",ch,showflag);
          return TRUE;
        }

      if (IS_AFFECTED(ch, AFF_CHARM) && victim == ch->master) {
        show_to_char("Sorry, Attacking your master is immpossible.\n\r",ch,showflag);
        return TRUE;
      }

      if (!is_clan(victim) && victim->fighting != ch) {
        return TRUE;
      }
      /* NEW PK Range formula goes here */
      if (!victim || !ch) {
        bugf("Victim/Ch dead in check_safe.%s\n\r",interp_cmd);
        return TRUE;
      }
       
      if ((ch->fighting != victim)
          && (victim->fighting != ch)
          && (ch != victim))
        {
        if (!is_in_pk_range(ch,victim)) {
            show_to_char("Pick on someone your own size.\n\r",ch,showflag);

            return TRUE;
        }
        }
    
    }
    }

  return FALSE;
}


/* -------------------------------------------------------------
   handler.c : Define the function, I just put it at the bottom */

void check_spirit( CHAR_DATA *ch, CHAR_DATA *victim )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *spirit;

  /* only happens 1 in 25 times and only to NPCs */
  if (IS_IMMORTAL(ch))
    return;

  /* No pets */
  if (IS_PET(victim))
    return;

  if ( number_range(0,24) != 0 || !IS_NPC(victim) || IS_SET(victim->act, ACT_NOGHOST))
    return;

  spirit = create_mobile ( victim->pIndexData );
  SET_BIT ( spirit->form,
        FORM_INSTANT_DECAY | FORM_UNDEAD | FORM_INTANGIBLE );
  start_hating( spirit, ch );
  set_plevel( spirit, ch );
  SET_BIT ( spirit->affected_by, AFF_PASS_DOOR );
  if ( !IS_SET(spirit->act, ACT_UNDEAD ) )
    SET_BIT( spirit->act, ACT_UNDEAD );
  
  mprintf(sizeof(buf), buf,"the undead spirit of %s",victim->short_descr);
  spirit->short_descr = str_dup(buf, spirit->short_descr);

  mprintf(sizeof(buf), buf,"undead spirit %s",victim->name);
  spirit->name = str_dup(buf, spirit->name);
  spirit->position = POS_STANDING;
  spirit->gold = 0; /* Undead spirits shouldn't have gold */
  char_to_room( spirit, ch->in_room );

  act("You cower in fear as $N appears before you!",ch,NULL,spirit,TO_CHAR);
  act("$N suddenly appears and attacks $n!",ch,NULL,spirit,TO_ROOM);

  multi_hit( spirit, ch, TYPE_UNDEFINED );

  return;
}

void show_to_char(char *str, CHAR_DATA *ch, bool show)
{
  if (!show)
    return;
  send_to_char(str,ch);
}

bool check_killsteal(CHAR_DATA *ch, CHAR_DATA *victim)
{
    /* Cases to check for...
       If They are a NPC .. we want to be able to PK someone who is
       fighting.
       If you are not in the same group as the person the victim is
       fighting.
       If you are not already set to being fighting them.
    */   
    if (IS_NPC(victim)
    && victim->fighting != NULL
    && !is_same_group(ch,victim->fighting)
    && ch->fighting != victim
    && victim->fighting != ch)
    {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return TRUE;
    }
    return FALSE;
}

int hand_skill_damage(CHAR_DATA *ch,CHAR_DATA *vch, int *dt)
{
  int dam=-1;
  int numhits = 0;

  dam = dice (1,ch->level/8);
  if (get_skill(ch, gsn_hand_to_hand) > 1) {
    if (ch->level >= skill_table[gsn_hand_to_hand].skill_level[ch->gameclass]){
      if (number_percent() < get_skill(ch,gsn_hand_to_hand)){
          dam = dice(ch->level,2);
        check_improve(ch,gsn_hand_to_hand,TRUE,1);
      }
      else
      {
          check_improve(ch,gsn_hand_to_hand,FALSE,1);
      }
      *dt = gsn_hand_to_hand;
    }
  }

  if ( (get_skill(ch, gsn_karate) > 1) )
  {
    if (ch->level >= skill_table[gsn_karate].skill_level[ch->gameclass])
    { 
      if (number_percent() < get_skill(ch,gsn_karate))
        {
          dam = dice(ch->level, 6);
          dam += GET_DAMROLL(ch);/* * UMIN(100,skill) /100;*/
        if (ch->martial_style == STYLE_KARATE)
          check_improve(ch,gsn_karate,TRUE,1);
        }
      else
      {
        if (ch->martial_style == STYLE_KARATE)
          check_improve(ch,gsn_karate,FALSE,1);
        }
      *dt = gsn_karate;
    }
  }

  if (ch->gameclass == cMystic)
  {
    if ( ch->martial_style )
      *dt = martial_style_dt(ch);
    else
      *dt = gsn_basic_style;
    numhits =0;
    numhits += style_check(ch,vch,gsn_basic_style);
    numhits += style_check(ch,vch,gsn_dragon_style);
    numhits += style_check(ch,vch,gsn_drunk_style);
    numhits += style_check(ch,vch,gsn_snake_style);
    numhits += style_check(ch,vch,gsn_crane_style);
    numhits += style_check(ch,vch,gsn_tiger_style);
    numhits += style_check(ch,vch,gsn_judo_style);
    numhits += style_check(ch,vch,gsn_ironfist_style);

    if (number_percent() > get_skill(ch,martial_style_dt(ch)))
      dam = 0; // that means we missed
    else if (numhits)
    {
      dam = dice(ch->level+(numhits*2), numhits) + GET_DAMROLL(ch) + get_curr_stat(ch, STAT_STR);

      switch (numhits)
      {
        case 0: 
            act("You perform a wicked side kick on $N's head.",ch,NULL,vch,TO_CHAR);
            act("$n performs a kick move and hits you on your head.",ch,NULL,vch,TO_VICT);
            act("$n spins and kicks $N in the head.",ch,NULL,vch,TO_NOTVICT);
            break;
        case 1:
            act("You chop a hand across $N's neck.",ch,NULL,vch,TO_CHAR);
            act("$n chops $s hand into your neck.",ch,NULL,vch,TO_VICT);
            act("$n chops $s hand as a blade into $N's neck.",ch,NULL,vch,TO_NOTVICT);
            break;
        case 2:
            act("You jump and smash your knee into $N's chest.",ch,NULL,vch,TO_CHAR);
            act("$n jumps and smashes their knee into your chest.",ch,NULL,vch,TO_VICT);
            act("$n jumps and smashes their knee into $N's chest.",ch,NULL,vch,TO_NOTVICT);
            break;
        case 3:
            act("You drop low and reverse kick $N in the knee.",ch,NULL,vch,TO_CHAR);
            act("$n drops low and reverse kicks you in your knee.",ch,NULL,vch,TO_VICT);
            act("$n drops low and reverse kicks $N in the knee.",ch,NULL,vch,TO_NOTVICT);
            break;
        case 4:
            act("You send an uppercut smashing through $N's chin.",ch,NULL,vch,TO_CHAR);
            act("$n sends an uppercut smashing through your chin.",ch,NULL,vch,TO_VICT);
            act("$n sends a powerful uppercut smashing through $N's chin.",ch,NULL,vch,TO_NOTVICT);
            break; 
        case 5:
            act("You tear at $N's arm and try to pull it out of socket.",ch,NULL,vch,TO_CHAR);
            act("$n tries to pull your arm out of socket.",ch,NULL,vch,TO_VICT);
            act("$n tries to pull $N's arm out of socket.",ch,NULL,vch,TO_NOTVICT);
            break;
        case 6:
            act("You kick $N in the chin and fly up and back over to the ground.",ch,NULL,vch,TO_CHAR);
            act("$n kicks you in the chin and performs an amazing leap to land back on their feet.",ch,NULL,vch,TO_VICT);
            act("$n kicks $N in the chin and flips over and lands on their feet in an amazing martial arts display.",ch,NULL,vch,TO_NOTVICT);
            break;
        case 7:
            act("You whirl into the air and double punch $N in the face.",ch,NULL,vch,TO_CHAR);
            act("$n whirls into the air and double punches you in the face.",ch,NULL,vch,TO_VICT);
            act("$n whirls into the air double punching $N in the face.",ch,NULL,vch,TO_NOTVICT);
            break;
        case 8:
            act("You send a vibro punch deep into $N's stomach causing great damage.",ch,NULL,vch,TO_CHAR);
            act("$n sends a vibro punch deep into your stomach.  You keel over for a moment.",ch,NULL,vch,TO_VICT);
            act("$n vibro punches $N in the stomach causing a scream to sound.",ch,NULL,vch,TO_NOTVICT);
            break; 
        default:
            act("You perform a wicked side kick on $N's head.",ch,NULL,vch,TO_CHAR);
            act("$n performs a kick move and hits you on your head.",ch,NULL,vch,TO_VICT);
            act("$n spins and kicks $N in the head.",ch,NULL,vch,TO_NOTVICT);
            break;
      }
    }
  }
  return(dam);
}

sh_int martial_style_dt( CHAR_DATA *ch )
{
  switch ( ch->martial_style )
  {
    case STYLE_BASIC:
    default:
      return gsn_basic_style;
      break;
    case STYLE_DRAGON:
      return gsn_dragon_style;
      break;
    case STYLE_DRUNK:
      return gsn_drunk_style;
      break;
    case STYLE_TIGER:
      return gsn_tiger_style;
      break;
    case STYLE_SNAKE:
      return gsn_snake_style;
      break;
    case STYLE_CRANE:
      return gsn_crane_style;
      break;
    case STYLE_IRONFIST:
      return gsn_ironfist_style;
      break;
    case STYLE_JUDO:
      return gsn_judo_style;
      break;
    case STYLE_KARATE:
      return gsn_karate;
      break;
    case STYLE_NONE:
      return gsn_basic_style;
  }
}

int get_style_dam_type(CHAR_DATA *ch)
{
  switch (ch->martial_style)
  {
    case STYLE_BASIC:
    default:
      return DAM_BASH;
      break;
    case STYLE_DRAGON:
      return DAM_SLASH;
      break;
    case STYLE_DRUNK:
      return DAM_BASH;
      break;
    case STYLE_TIGER:
      return DAM_SLASH;
      break;
    case STYLE_SNAKE:
      return DAM_PIERCE;
      break;
    case STYLE_CRANE:
      return DAM_PIERCE;
      break;
    case STYLE_IRONFIST:
      return DAM_BASH;
      break;
    case STYLE_JUDO:
      return DAM_BASH;
      break;
    case STYLE_KARATE:
      return DAM_SLASH;
      break;
    case STYLE_NONE:
      return DAM_BASH;
  }
}

void make_ghost(CHAR_DATA *ch, CHAR_DATA *victim)
{
  AFFECT_DATA af; //, *paf;
//  int time;

  if (ch == victim)
    return;
  
  if ((!IS_NPC(victim) && !IS_NPC(ch) && !IS_IMMORTAL(ch)) 
      ||  (IS_PET(ch) && !IS_IMMORTAL(ch->master))){
    af.where     = TO_SPELL_AFFECTS;
    af.type      = gsn_ghost_time;
    af.level     = victim->level;
    af.duration  = GHOST_TIME;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SAFF_GHOST;
    affect_to_char( victim, &af );

    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("pass door");;
    af.level     = victim->level;
    af.duration  = GHOST_TIME;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );

    af.where     = TO_AFFECTS;
    af.type     = skill_lookup("darksight");
    af.level     = victim->level;
    af.duration  = GHOST_TIME;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DARK_VISION;
    affect_to_char( victim, &af );

//    time = GHOST_TIME;
//    if (IS_NOCHANNELED(victim)) {
//      if (is_affected(victim, gsn_nochannel)) {
//    for (paf = victim->affected; paf; paf = paf->next) {
//      if (paf->where == TO_PENALTY && paf->bitvector == PEN_NOCHANNELS) {
//        time += paf->duration;
//      }
//    }
//      } else {
//        time = -1;
//      }
//      remove_affect(victim, TO_PENALTY, PEN_NOCHANNELS);
//      REMOVE_BIT(victim->pen_flags, PEN_NOCHANNELS);
//    }
//    if (time > 0) {
//      af.where     = TO_PENALTY;
//      af.type     = gsn_nochannel;
//      af.level     = victim->level;
//      af.duration  = time;
//      af.location  = APPLY_NONE;
 //     af.modifier  = 0;
//      af.bitvector = PEN_NOCHANNELS;
//      affect_to_char( victim, &af );
//    } else {
//      SET_BIT(victim->pen_flags,PEN_NOCHANNELS);
//    }
    
    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("fly");
    af.level     = victim->level;
    af.duration  = GHOST_TIME;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );

    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("detect invis");
    af.level     = victim->level;
    af.duration  = GHOST_TIME;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( victim, &af );

    victim->move = victim->level + 10;
    send_to_char("\nYou are now a {WGhost{x.\n\r",victim);
    do_function(victim, &do_nofollow, "");
    victim->group_num = victim->id;

    //if (IS_SET(victim->act,PLR_QUESTING))
    //{
    //victim->questdata->nextquest = GHOST_TIME;
    //victim->questdata->streak = 0;
    //printf_to_char(victim,"Your quest has failed.\n\r");
    //clear_quest(victim,FALSE);    
    //}

  }
}
/*
 * Hit one guy once.
 */
void spirit_hit( CHAR_DATA *ch, CHAR_DATA *victim)
{
  int victim_ac;
  int thac0;
  int thac0_00;
  int thac0_32;
  int dam, dammod =1;
  int diceroll;
  int sn=-1,skill;
  int dam_type;
  int hit_roll;

  /* just in case */
  if (victim == ch || ch == NULL || victim == NULL) 
    return;

  /*
   * Can't beat a dead char!
   * Guard against weird room-leavings.
   */
  if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
    return;

  /*
   * Figure out the type of damage message.
   * if secondary == true, use the second weapon.
   */

  if (is_affected(ch,skill_lookup("bear spirit"))) {
    dammod = 2;
    sn = gsn_bear_spirit;
  } else 

    if ( is_affected(ch,skill_lookup("eagle spirit"))) {
      dammod = 3;
      sn = gsn_eagle_spirit;
    } else 
  
      if (is_affected(ch,skill_lookup("tiger spirit"))) {
    dammod = 4;
    sn = gsn_tiger_spirit;
      } else
    if ( is_affected(ch,skill_lookup("dragon spirit"))) {
      dammod = 5;
      sn = gsn_dragon_spirit;
    }

  dam_type = DAM_MENTAL;

  skill = 10 + get_skill(ch,sn);
  /*
   * Calculate to-hit-armor-class-0 versus armor.
   */
  if ( IS_NPC(ch) )
    {
      thac0_00 = 20;
      thac0_32 = -4;        /* as good as a thief */ 
      if (IS_SET(ch->act,ACT_WARRIOR))
    thac0_32 = -10;
      else if (IS_SET(ch->act,ACT_THIEF))
    thac0_32 = -4;
      else if (IS_SET(ch->act,ACT_CLERIC))
    thac0_32 = 2;
      else if (IS_SET(ch->act,ACT_MAGE))
    thac0_32 = 6;
    }
  else
    {
      thac0_00 = class_table[ch->gameclass].thac0_00;
      thac0_32 = class_table[ch->gameclass].thac0_32;
    }
  thac0  = interpolate( ch->level, thac0_00, thac0_32 );
#if TESTIT
  printf_to_char(ch,"POINT A:\n\r");
#endif
  if (thac0 < 0)
    thac0 = thac0/2;

  if (thac0 < -5)
    thac0 = -5 + (thac0 + 5) / 2;

  hit_roll = GET_HITROLL( ch );

  if ( hit_roll > 20 )
    hit_roll = 20 + (hit_roll - 20) / 2;

  thac0 -= hit_roll * skill/100;
  thac0 += 5 * ( 100 - skill ) / 100;

  switch(dam_type)
    {
    case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;    break;
    case(DAM_BASH):  victim_ac = GET_AC(victim,AC_BASH)/10;    break;
    case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;    break;
    default:         victim_ac = GET_AC(victim,AC_EXOTIC)/10;    break;
    }; 
    
  if (victim_ac < -15)
    victim_ac = (victim_ac + 15) / 5 - 15;
  
  if ( !can_see( ch, victim ) )
    victim_ac -= 4;
  
  if ( victim->position < POS_FIGHTING)
    victim_ac += 4;
  
  if (victim->position < POS_RESTING)
    victim_ac += 6;
  /*
   * The moment of excitement!
   */
  while ( ( diceroll = number_bits( 5 ) ) >= 20 )
    ;

  if ( diceroll == 0
       || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
      /* Miss. */
      damage( ch, victim, 0, sn, dam_type, TRUE, FALSE );
      tail_chain( );
      return;
    }

  /*
   * Hit.
   * Calc damage.
   */
  dam = dice(6, ch->level+ dammod) * skill/100;
  dam += GET_DAMROLL(ch);    /* * UMIN(100,skill) /100;*/

  /*
   * Bonuses.
   */
  if ( get_skill(ch,gsn_enhanced_damage) > 0 )
    {
      diceroll = number_percent();
      if (diceroll <= get_skill(ch,gsn_enhanced_damage))
        {
      check_improve(ch,gsn_enhanced_damage,TRUE,6);
      dam += 2 * ( dam * diceroll/300);
        }
    }

  if ( !IS_AWAKE(victim) )
    dam *= 2;
  else if (victim->position < POS_FIGHTING)
    dam = dam * 3 / 2;


  if (saves_spell(ch->level, victim, DAM_MENTAL))
    dam = dam/2;
  
  if ( dam <= 0 )
    dam = 1;

  damage( ch, victim, dam, sn, dam_type, TRUE, FALSE );

  tail_chain( );
  return;
}

int style_check(CHAR_DATA *ch, CHAR_DATA *vch, int gsn)
{
  int chance=0;
  if ((get_skill(ch, gsn)) < 0)
    return 0;

  if (ch->level < skill_table[gsn].skill_level[ch->gameclass])
    return 0;

  chance = get_skill(ch, gsn);

  if (get_curr_stat(ch,STAT_DEX) > get_curr_stat(vch, STAT_DEX))
    chance += 20;
  else if (get_curr_stat(ch,STAT_DEX) < get_curr_stat(vch, STAT_DEX))
    chance -= 20;

  if (check_parry(ch, vch, FALSE))
    return 0;

/* to put the improvement algorithm back, remove this block and uncomment at end */
  if ((gsn == gsn_karate) && (ch->martial_style == STYLE_KARATE))
    check_improve(ch,gsn,TRUE,1);
  else if ((gsn == gsn_basic_style) && (ch->martial_style == STYLE_BASIC))
    check_improve(ch,gsn,TRUE,1);
  else if ((gsn == gsn_dragon_style) && (ch->martial_style == STYLE_DRAGON))
    check_improve(ch,gsn,TRUE,1);
  else if ((gsn == gsn_drunk_style) && (ch->martial_style == STYLE_DRUNK))
    check_improve(ch,gsn,TRUE,1);
  else if ((gsn == gsn_snake_style) && (ch->martial_style == STYLE_SNAKE))
    check_improve(ch,gsn,TRUE,1);
  else if ((gsn == gsn_crane_style) && (ch->martial_style == STYLE_CRANE))
    check_improve(ch,gsn,TRUE,1);
  else if ((gsn == gsn_tiger_style) && (ch->martial_style == STYLE_TIGER))
    check_improve(ch,gsn,TRUE,1);
  else if ((gsn == gsn_judo_style) && (ch->martial_style == STYLE_JUDO))
    check_improve(ch,gsn,TRUE,1);
  else if ((gsn == gsn_ironfist_style) && (ch->martial_style == STYLE_IRONFIST))
    check_improve(ch,gsn,TRUE,1);


  if (number_percent() < chance)
  {
//    check_improve(ch,gsn,TRUE,1);
    return 1;
  }
  else
    return 0;

}
