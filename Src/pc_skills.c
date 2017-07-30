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

void shatter( CHAR_DATA *ch, CHAR_DATA *victim );

/* -------------------------------------------
 * Counter Skill for ROM24, 1.02 - 3/30/98
 * -------------------------------------------
 * The counter skill is an advanced defensive
 * fighting technique that allows a skilled
 * fighter to reverse his opponent's attack and
 * actually hit him back with it.
 *
 * As with all else, please email me if you use
 * this code so I know my time is not going
 * to waste. :) And also, with this one I could
 * really use some suggesstions for improvement!
 *
 * Brian Babey (aka Varen)
 * [bribe@erols.com]
 * ------------------------------------------- */


/* ---------------------------------------------
 * In fight.c, insert this function wherever
 * it belongs alphabetically (or wherever you'd
 * like it to be - it really doesn't matter) */

bool check_counter( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt)
{
  int chance;
  int dam_type;
  int odt;
  OBJ_DATA *wield;

  if (    ( get_eq_char(victim, WEAR_WIELD) == NULL ) ||
    ( !IS_AWAKE(victim) ) ||
    ( !can_see(victim,ch) ) ||
    ( dam <= 0) ||
    ( get_skill(victim,gsn_counter) < 1 )
    )
    return FALSE;

  wield = get_eq_char(victim,WEAR_WIELD);

  chance = get_skill(victim,gsn_counter) / 6;
  chance += ( victim->level - ch->level ) / 2;
  chance += 2 * (get_curr_stat(victim,STAT_DEX) - get_curr_stat(ch,STAT_DEX));
  chance += get_weapon_skill(victim,get_weapon_sn(victim)) -
    get_weapon_skill(ch,get_weapon_sn(ch));
  chance += (get_curr_stat(victim,STAT_STR) - get_curr_stat(ch,STAT_STR) );

  if ( number_percent( ) >= chance )
    return FALSE;

  odt = dt;
  dt = gsn_counter;
  
  if ( dt == TYPE_UNDEFINED )
    {
      dt = TYPE_HIT;
      if ( wield && wield->item_type == ITEM_WEAPON )
  dt += wield->value[3];
      else 
  dt += ch->dam_type;
    }

  if (dt < TYPE_HIT) {
    if (wield ) {
      if (wield->value[3] > 100)
  bugf("Value greater than should be for %s\n\r",wield->name);
      dam_type = attack_table[wield->value[3]].damage;
    }
    else
      dam_type = attack_table[ch->dam_type].damage;
  }
  else
    dam_type = attack_table[dt - TYPE_HIT].damage;

  if (dam_type == -1)
    dam_type = DAM_BASH;
#if TESTIT
  printf_to_char(ch,"COUNTER: DT = %d, NEWDT = %d\n\r",odt,dt);
#endif
  damage(ch,victim, 0 , odt , dam_type ,TRUE, FALSE ); /* DAM MSG NUMBER!! */
  act( "You reverse $n's attack and counter with your own!", ch, NULL, victim, TO_VICT    );
  act( "$N reverses your attack!", ch, NULL, victim, TO_CHAR    );

  damage(victim,ch,dam/2, gsn_counter , dam_type ,TRUE, FALSE ); /* DAM MSG NUMBER!! */

  check_improve(victim,gsn_counter,TRUE,6);

  return TRUE;
}


/**************************************************/
/* ********************************************************************
   This is the find familiar skill.
   April 4 1998 by Gothar
   
   This skill allows your players to
   have a companion like those loveable 
   pets in the pet shops.
   
   Email me if you use it. Leave this header
   to say Gothar had a great idea there.
   gothar@magma.ca
   mcco0055@algonquinc.on.ca
 * ******************************************************************** */

void do_familiar (CHAR_DATA *ch, char *argument)
{
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *familiar;
  char buf[MAX_STRING_LENGTH];
  int i, chance;

  if (IS_GHOST(ch)) {
    send_to_char("That is useless as you are still DEAD.\n\r",ch);
    return;
  }

  if (((chance = get_skill(ch,gsn_familiar)) < 1)
    || (IS_NPC(ch)))
    {
      send_to_char("You don't know where to start.\n\r",ch);
      return;
    }

  if ( ch->pet != NULL )
    {
      send_to_char("You already have a companion.\n\r",ch);
      return;
    }
  if(ch->position == POS_FIGHTING)
    {
      send_to_char("You can't study the ritual while in combat!\n\r",ch);
      return;
    }
    
  if ( ( pMobIndex = get_mob_index(MOB_VNUM_FAMILIAR) ) == NULL )
    {
      send_to_char( "The familiar mob doesn't exist.\n\r", ch );
      return;
    }
  /* can't cast the spell in these sectors */
  if(ch->in_room->sector_type == SECT_INSIDE
     || ch->in_room->sector_type == SECT_WATER_SWIM 
     || ch->in_room->sector_type == SECT_WATER_NOSWIM
     || ch->in_room->sector_type == SECT_AIR )
    {
      send_to_char("You are feeling too sick to concentrate.\n\r",ch);
      return;
    }

  if ( number_percent( ) > chance) {
    send_to_char("You failed.\n\r",ch);
    WAIT_STATE(ch, PULSE_MOBILE);
    return;
  }

  familiar = create_mobile( pMobIndex );
    
  familiar->level = number_fuzzy(ch->level / 2);
  familiar->mana = familiar->max_mana = 20;
  familiar->hit = familiar->max_hit = number_fuzzy(ch->max_hit / 2);
  for(i = 0; i < 4; i++)
    familiar->armor[i] = number_fuzzy(ch->armor[i] - 10);
  familiar->hitroll = number_fuzzy(ch->level / 30);
  familiar->damroll = number_fuzzy(ch->level / 30);

  /* free up the old mob names */ 
  free_string(familiar->description);
  free_string(familiar->name);
  free_string(familiar->short_descr);
  free_string(familiar->long_descr);
    
  /* terrain */
  switch(ch->in_room->sector_type)
    {
    default:
    case(SECT_CITY): /* rat */
    case(SECT_FIELD):
    case(SECT_UNDERGROUND):
      strcpy(buf,"You see a large furry rat.  Long whiskers hang down from it's nose.\n\r");
      strcat(buf,"You can feel the dirt and disease crawling off this beast.\n\r");
      familiar->short_descr = str_dup("a large rat",familiar->short_descr);
      familiar->long_descr = str_dup("A large furry {yr{wo{ydent{x is here.\n\r", familiar->long_descr);
      familiar->name = str_dup("familiar rat", familiar->name);
      familiar->dam_type = 22; /* scratch */
      break;
    case(SECT_FOREST):  /* falcon */
    case(SECT_HILLS):
      strcpy(buf,"You see a large falcon.  Golden brown feathers frame powerful\n\r");
      strcat(buf,"wings.  Long talons grasp at nothingness in vain attempts at\n\r");
      strcat(buf,"getting some rabbit or rodent for dinner.\n\r");
      familiar->short_descr = str_dup("a large falcon",familiar->short_descr);
      familiar->long_descr = str_dup("A large {Ffa{wl{yc{won{x screams here.\n\r",familiar->long_descr);
      familiar->name = str_dup("familiar falcon",familiar->name);
      familiar->dam_type = 5; /* claw */
      break;
    case(SECT_MOUNTAIN): /* familiarain lion */
      strcpy(buf,"You see a very large familiarain lion.  One wrong look and it could\n\r");
      strcat(buf,"have your head lying at your feet.  You should think better than\n\r");
      strcat(buf,"cross this beast especial if you have a weapon in your hand.\n\r");
      familiar->short_descr = str_dup("a large familiarain lion",familiar->short_descr);
      familiar->long_descr = str_dup("A large familiarain {yli{Yo{yn{x claws the ground here.\n\r", familiar->long_descr);
      familiar->name = str_dup("familiar familiarain lion", familiar->name);
      familiar->dam_type = 10; /* bite */
      break;
    case(SECT_DESERT): /* sandworm */
      strcpy(buf,"You see a large white sandworm wiggling in the light.\n\r");
      strcat(buf,"A red spot on one end makes you guess it is a mouth.\n\r");
      strcat(buf,"A loud moan comes from the direction of that red spot.\n\r");
      familiar->short_descr = str_dup("a sandworm", familiar->short_descr);
      familiar->long_descr = str_dup("A white {ws{Wan{wd{yw{Wo{yrm{x wiggles on the ground here.\n\r", familiar->long_descr);
      familiar->name = str_dup("familiar sandworm", familiar->name);
      familiar->dam_type = 12; /* suction */
      break;
    }
    strcat(buf,"It keeps an eye out for any threats to ");
    strcat(buf,ch->name);
    strcat(buf,".\n\r");
    familiar->description = str_dup(buf, familiar->description);

  /* player seen stuff here */
  do_function(ch, &do_sit,"");


  char_to_room( familiar, ch->in_room );
  act( "You begin to chant and call to $N!",ch,NULL,familiar,TO_CHAR);
  act( "$n begins to chant and calls to $N!", ch, NULL, familiar, TO_ROOM );
  WAIT_STATE(ch, 2 * PULSE_MOBILE);
  add_follower( familiar, ch );
  if (ch->leader)
    familiar->leader = ch->leader;
  else
    familiar->leader = ch;

  if ( ch->clan )
  {
    free_string( familiar->clan_name );
    familiar->clan_name = str_dup( ch->clan_name, familiar->clan_name );
    familiar->clan = ch->clan;
  }
  ch->pet = familiar;
  do_function(ch, &do_stand,"");

  SET_BIT(familiar->act, ACT_PET);
  SET_BIT(familiar->affected_by, AFF_CHARM);
  SET_BIT(familiar->act,ACT_NOGHOST);
  ch->move -= (familiar->level / 2);  /* physically draining lose of move */
  check_improve(ch,gsn_familiar,TRUE,6);
  return;
}



/**************************************************/
/* ------------------------------------------------
 * Varen's Deathgrip, revision 2.00 ... 5/31/98
 *
 * Deathgrip is a skill that adds a damroll
 * affect to the player, and sets any weapon
 * they are wielding to a death weapon,
 * _while it is wielded_ .. when they remove it
 * it is no longer this type of weapon. As they
 * wield other weapons they become death weapons
 * too. The death flag does a bit of extra damage
 * to good aligned chars, and lowers their alignment
 * if they are PCs.
 *
 * If you choose to use this skill on your mud,
 * feel free to do so, but please send me an email
 * telling me, so I know if this is actually
 * a waste of time or not. Also please mail me with
 * any bugs or suggestions at: [bribe@erols.com]
 *
 * Brian Babey (aka Varen)
 * ------------------------------------------------ */


/* -----------------------------------------------
 * Now for the code: place this function inside
 * fight.c, preferable in correct alphabetical
 * based on the other fight actions */

void do_deathgrip( CHAR_DATA *ch, char *argument )
{
  int sn;
  AFFECT_DATA af;

  sn = skill_lookup("deathgrip");

  if (IS_GHOST(ch)) {
    send_to_char("Deathgrip is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( is_affected(ch,sn) )
    {
      send_to_char("You already have a grip of death.\n\r",ch);
      return;
    }

  if ( get_skill(ch,sn) < 1 )
    {
      send_to_char("What's that?\n\r",ch);
      return;
    }

  if (  get_skill(ch,sn)  <  (number_range(0, 100))  )
    {
      send_to_char("You failed to create a grip of death.\n",ch);
      WAIT_STATE(ch, 1.5 * PULSE_VIOLENCE);
      check_improve(ch,sn,FALSE,1);
      return;
    }

  /* Now for adding the affect to the player */

  af.where        = TO_AFFECTS;
  af.type         = sn;  
  af.level        = ch->level;
  af.duration     = ch->level / 3;
  af.location     = APPLY_DAMROLL;
  af.modifier     = ch->level / 7;
  af.bitvector    = 0;

  affect_to_char(ch, &af);

  act("$n's hands are shrouded with a {Dd{wa{Drk{x mist.",ch,NULL,NULL,TO_ROOM);
  send_to_char("Your hands are shrouded with a {Dd{wa{Drk{x mist.\n\r",ch);
  WAIT_STATE(ch, 1 * PULSE_VIOLENCE);

  check_improve(ch,sn,TRUE,1);
}


/**************************************************/
/* The study command will allow your players to learn spells from studying
wands, staves, or scrolls.  If you only want them to be able to study one
of these items, the others can easily be removed.  Study was originally
written for Yrth, yrth.mudservices.com 5000.  I don't ask for any credit,
except to leave the study by Absalom comment in void do_study.  I can be
reached for questions about the snippet at clap@wolfenet.com.  Reports on
bugs, enhancements, etc. are greatly welcomed. */


/*void do_study( CHAR_DATA *ch, char *argument )  study by Absalom */
/*{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int sn = 0;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
      send_to_char( "Study what?\n\r", ch );
      return;
    }

  if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
      send_to_char( "You do not have that item.\n\r", ch );
      return;
    }

  if ( obj->item_type != ITEM_STAFF && obj->item_type != ITEM_WAND &&
       obj->item_type != ITEM_SCROLL )
    {
      send_to_char( "You can only study scrolls, wands, and staves.\n\r", ch );
      return;
    }

  act( "$n studies $p.", ch, obj, NULL, TO_ROOM );
  act( "You study $p.", ch, obj, NULL, TO_CHAR );

  if (ch->level < obj->level)
    {
      send_to_char("You cannot glean any knowledge from it.\n\r",ch);
      act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
      extract_obj( obj );
    }

  if (obj->item_type == ITEM_STAFF)
    {
      sn = obj->value[3];
      if ( sn < 0 || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
  {
    bug( "Do_study: bad sn %d.", sn );
    return;
  }
      if ( number_percent() >= 20 + get_skill(ch,gsn_staves) * 4/5)
  {
    send_to_char("You cannot glean any knowledge from it.\n\r",ch);
    check_improve(ch,gsn_staves,FALSE,2);
    act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
    extract_obj( obj );
    return;
  }
      if ( ch->pcdata->learned[sn])
  {
    send_to_char("You already know that spell!\n\r",ch);
    return;
  }
      ch->pcdata->learned[sn] = 1;
      act("You have learned the art of $t!",
    ch,skill_table[sn].name,NULL,TO_CHAR);
      check_improve(ch,gsn_staves,TRUE,2);
      act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
      extract_obj( obj );
      return;
    }

  if (obj->item_type == ITEM_WAND)
    {
      sn = obj->value[3];
      if ( sn < 0 || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
  {
    bug( "Do_study: bad sn %d.", sn );
    return;
  }
      if ( number_percent() >= 20 + get_skill(ch,gsn_wands) * 4/5)
  {
    send_to_char("You cannot glean any knowledge from it.\n\r",ch);
    check_improve(ch,gsn_wands,FALSE,2);
    act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
    extract_obj( obj );
    return;
  }
      if ( ch->pcdata->learned[sn])
  {
    send_to_char("You already know that spell!\n\r",ch);
    return;
  }
      ch->pcdata->learned[sn] = 1;
      act("You have learned the art of $t!",
    ch,skill_table[sn].name,NULL,TO_CHAR);
      check_improve(ch,gsn_wands,TRUE,2);
      act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
      extract_obj( obj );
      return;
    }

  if (obj->item_type == ITEM_SCROLL)
    {
      sn = obj->value[1];
      if ( sn < 0 || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
  {
    bug( "Do_study: bad sn %d.", sn );
    return;
  }
      if ( number_percent() >= 20 + get_skill(ch,gsn_scrolls) * 4/5)
  {
    send_to_char("You cannot glean any knowledge from it.\n\r",ch);
    check_improve(ch,gsn_scrolls,FALSE,2);
    act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
    extract_obj( obj );
    return;
  }
      if ( ch->pcdata->learned[sn])
  {
    send_to_char("You already know that spell!\n\r",ch);
    return;
  }
      ch->pcdata->learned[sn] = 1;
      act("You have learned the art of $t!",
    ch,skill_table[sn].name,NULL,TO_CHAR);
      check_improve(ch,gsn_scrolls,TRUE,2);
      act( "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
      extract_obj( obj );
      return;
    }

}*/


/**************************************************/
/* -------------------------------------------
 * Critical Strike for ROM24, 1.00 - 3/28/98
 * -------------------------------------------
 * The critical strike is an automatic skill
 * that allows players with 100% skill in the
 * weapon they are using to get a little bonus
 * out of it once in a while.
 *
 * As with all else, please email me if you use
 * this code so I know my time is not going
 * to waste. :)
 *
 * Brian Babey (aka Varen)
 * [bribe@erols.com]
 * ------------------------------------------- */

/* modified by Taeloch to account for secondary weapons */
bool check_critical(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *weapon)
{
  int sn=-1;

  if (weapon)
  {
    switch (weapon->value[0])
    {
      case(WEAPON_EXOTIC):  sn = gsn_exotic;  break;
      case(WEAPON_SWORD):   sn = gsn_sword;   break;
      case(WEAPON_DAGGER):  sn = gsn_dagger;  break;
      case(WEAPON_SPEAR):   sn = gsn_spear;   break;
      case(WEAPON_MACE):    sn = gsn_mace;    break;
      case(WEAPON_AXE):     sn = gsn_axe;     break;
      case(WEAPON_FLAIL):   sn = gsn_flail;   break;
      case(WEAPON_WHIP):    sn = gsn_whip;    break;
      case(WEAPON_POLEARM): sn = gsn_polearm; break;
      case(WEAPON_CROSSBOW): sn = gsn_crossbow; break;
      default:              sn = -1;          break;
    }
  }

  if ( ( weapon == NULL )
  ||   ( get_skill(ch,gsn_critical)  <  1 )
  ||   ( get_weapon_skill(ch,sn) <  95 )
  ||   ( number_range(0,100) > get_skill(ch,gsn_critical) ) )
    return FALSE;

  if ( number_range(0,100) > 10)
    return FALSE;

  /*act("{W$p {L{RCR{YITICAL{RLY {DST{WRIK{DES{x{W $n!{x",victim,obj,ch,TO_NOTVICT);*/
  act_spam("You hit $N with a staggering {yCRITICAL {RSTRIKE{x!", ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
  act_spam("$n hits you with a staggering {yCRITICAL {RSTRIKE{x!", ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_OEFFECTS );
  act_spam("$n hits $N with a staggering {yCRITICAL {RSTRIKE{x!", ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS );

  check_improve(ch,gsn_critical,TRUE,6);
  return TRUE;
}

/**************************************************/

/*
 * Tithe Command for Monk and Priest Classes
 * By Plasma (morrisal@pirates.armstrong.edu)
 * Modified by Winston, Kona.. etc
 */

void do_tithe( CHAR_DATA *ch, char *argument )
{
  int offering, healing;

  if ( IS_NPC(ch) || (get_skill(ch,gsn_tithe) < 1))
  {
    send_to_char("Go join a church.\n\r", ch);
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("Tithing is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if (ch->position <= POS_SLEEPING)
  {
    send_to_char("You have to be awake to tithe.\n\r",ch);
    return;
  }

  if (ch->mana >= GET_MANA(ch))
  {
    send_to_char("You're already at full mana.\n\r", ch);
    return;
  }

  if (get_skill(ch,gsn_tithe) < 2 )
  {
    send_to_char("Worship some more.\n\r", ch); 
    return;
  }

  if (ch->fighting != NULL) 
  {
    send_to_char("Your god will not help you during violent times.\n\r",ch);
    return;  
  }

// determine the offering amount, in silver
  if (is_number(argument) )
    offering = atoi(argument) * 100;
  else // if offering undefined, default 10%
    offering = GET_WEALTH(ch) / 10;

  if (offering > GET_WEALTH(ch))
  {
    send_to_char("You do not have that much!\n\r",ch);
    return;  
  }

  if ((get_skill(ch,gsn_tithe) - number_percent()) < 0)
  {
    deduct_cost(ch, (offering / 3));
    send_to_char("You fail to get your god's attention.\n\r",ch);
    WAIT_STATE(ch,PULSE_TICK/6);
    check_improve( ch, gsn_tithe, TRUE, 2);
    return;
  }

  if (offering < (ch->level * 4))
  {
    deduct_cost(ch, offering);
    send_to_char("Your god is not impressed .\n\r",ch);
    WAIT_STATE(ch,PULSE_TICK/6);
    return;
  }

  healing = (offering / 1000) * ((get_skill(ch,gsn_tithe) + ch->level) / 4);
  
  printf_to_char(ch, "You piously {ctithe{x %d {yg{Yo{yld{x to the gods, ", (offering / 100));
  if ((ch->mana + healing) > GET_MANA(ch))
  {
    printf_to_char(ch, "who replenish your mana completely (%d).\n\r",healing);
    ch->mana = GET_MANA(ch);
    deduct_cost(ch, offering);
    check_improve( ch, gsn_tithe, TRUE, 2);
  }
  else
  {
    printf_to_char(ch, "who give you %d mana in return.\n\r",healing);
    ch->mana = ch->mana + healing;
    deduct_cost(ch, offering);
    check_improve( ch, gsn_tithe, TRUE, 2);
  }

  WAIT_STATE(ch,PULSE_TICK/4);
  return;
}

/*
 * Tithe Command for Monk and Priest Classes
 * By Plasma (morrisal@pirates.armstrong.edu)
 * Modified by Winston, Kona.. etc
 */
/* Original tithe code
void do_tithe( CHAR_DATA *ch, char *argument )
{
  int base_cost, iCost,AgCost,AuCost, mana_needed,skillroll;
  int level, healrate;
  char buf[MSL];
  float fCost;

  if ( IS_NPC(ch) || (get_skill(ch,gsn_tithe) < 1))
    {
      send_to_char("Go join a church.\n\r", ch);
      return;
    }

  if (IS_GHOST(ch))
    {
      send_to_char("Tithing is useless as you are still {rDEAD{x.\n\r",ch);
      return;
    }
  if (ch->position <= POS_SLEEPING)
    {
      send_to_char("You have to be awake to tithe.\n\r",ch);
      return;
    }  
  if (ch->mana >= ch->max_mana)
    {
      send_to_char("You're already at full mana.\n\r", ch);
      return;
    }

  if (get_skill(ch,gsn_tithe) < 2 )
    {
      send_to_char("Worship some more.\n\r", ch); 
      return;
    }
       
  if (ch->fighting != NULL) 
    {
      send_to_char("Your god will not help you during violent times.\n\r",ch);
      return;  
    }       


  mana_needed = GET_MANA(ch) - ch->mana;

  skillroll = (get_skill(ch,gsn_tithe) - number_percent());
       
  // *** Base Cost to Get Gods to listen
 
  base_cost = (ch->level/7+1)*3000;

  if (skillroll >= 0)
    {
      fCost = (float)base_cost  - skillroll;
      iCost = (int)fCost;
    }
  else
    iCost = base_cost*2;

  if (ch->gold*100+ch->silver < iCost)
    {
      send_to_char("You begin tithing to your god, but run out of money before you are done.\n\r",ch);
      send_to_char("They scoff at your contribution and grant you 1 {gmana{x for your efforts.\n\r",ch);
      ch->gold = 0;
      ch->silver = 0;
      ch->mana++;
      ch->move /=2;
      WAIT_STATE(ch,PULSE_TICK/4);
      return;
    }
  else if (ch->silver >= iCost)
    ch->silver -= iCost;
  else
    {
      AgCost = ch->silver;
      AuCost = (iCost - AgCost)/100;   
      ch->silver -= AgCost;    
      ch->gold   -= AuCost;    
    }

  // *** Rate at which they give you healing if you have enough money

  level = ch->level;
  if      (get_skill(ch,gsn_tithe) > 95)
    healrate = URANGE(5,level/10+1,12);
  else if (get_skill(ch,gsn_tithe) > 90)
    healrate = URANGE(4,level/10+1,11);
  else if (get_skill(ch,gsn_tithe) > 85)
    healrate = URANGE(3,level/10+1,10);
  else if (get_skill(ch,gsn_tithe) > 80)
    healrate = URANGE(2,level/11+1,9);
  else if (get_skill(ch,gsn_tithe) > 70)
    healrate = URANGE(1,level/12+1,8);
  else if (get_skill(ch,gsn_tithe) > 50)
    healrate = URANGE(1,level/15+1,7);
  else if (get_skill(ch,gsn_tithe) > 30)
    healrate = URANGE(1,level/18+1,6);
  else if (get_skill(ch,gsn_tithe) > 15)
    healrate = URANGE(1,level/20+1,5);
  else
    healrate = URANGE(1,level/22+1,4);

  if (skillroll < 0)
    healrate = healrate/2;

  iCost += mana_needed/healrate+1;
  if (ch->gold*100+ch->silver >= mana_needed/healrate+1)
  {
    if (ch->silver >= mana_needed/healrate+1)
    {
      ch->silver -= mana_needed/healrate+1;
      mprintf(sizeof(buf),buf,
        "You piously {ctithe{x %d {wsi{Wl{Dv{wer{x to the gods.\n\r",iCost);
    }
    else
    {
      AgCost = ch->silver;
      AuCost = (mana_needed/healrate+1 - AgCost)/100;   
      ch->silver -= AgCost;    
      ch->gold   -= AuCost;    
      mprintf(sizeof(buf),buf,
        "You piously {ctithe{x %d in {yg{Yo{yld{x and {wsi{Wl{Dv{wer{x to the gods.\n\r",iCost);
    }

    ch->mana = GET_MANA(ch);
    ch->move = 150;
    send_to_char(buf,ch);
    send_to_char("The gods listen to your pleas and take pity, restoring you to full {gmana{x!\n\r",ch);
    check_improve( ch, gsn_tithe, TRUE, 2);
  }
  else
  {
    ch->mana += (ch->silver/100 + ch->gold)*healrate;
    ch->gold   = 0;
    ch->silver = 0;
    send_to_char(
      "You piously {ctithe{x all your money to the gods.\n\r",ch);
    send_to_char(
      "Alas! You run out of money before you can finish all your prayers.\n\r",ch);
    send_to_char(
      "The gods listen to your pleas and graciously heal some of your {gmana{x!\n\r",ch);
  }
  WAIT_STATE(ch,PULSE_TICK/3);
  return;
}
*/

/**************************************************/
void do_whirlwind( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *pChar;
  CHAR_DATA *pChar_next;
  OBJ_DATA *wield;
  int lagged = 0;
  bool found = FALSE;
   
  if (    !IS_NPC( ch ) 
    && (get_skill(ch,gsn_whirlwind) < 1))
    {
      send_to_char( "You don't know how to do that...\n\r", ch );
      return;
    }
 
  if (IS_GHOST(ch)) {
    send_to_char("Whirlwind is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
    {
      send_to_char( "You need to wield a weapon first...\n\r", ch );
      return;
    }
   
  act( "$n holds $p firmly, and starts spinning round...", ch, wield, NULL, TO_ROOM );
  act( "You hold $p firmly, and start spinning round...",  ch, wield, NULL, TO_CHAR );
   
  pChar_next = NULL;   
  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
  {
    pChar_next = pChar->next_in_room;

//    if ( !is_safe( ch, pChar, TRUE )
    if ( !check_safe(ch, pChar, FALSE, TRUE, TRUE) // ch, vict, spell?, area?, show?
    &&   !is_same_group(ch, pChar)
    &&   !IS_IMMORTAL(pChar)
    &&   !(is_same_clan(ch,pChar))
    &&    (ch != pChar)
    &&    (pChar->master != ch)
    &&    (IS_NPC( pChar ) && pChar->master != ch )
    &&    (number_percent() <= get_skill(ch,gsn_whirlwind)))
    {
      found = TRUE;
      act( "$n turns towards YOU!", ch, NULL, pChar, TO_VICT    );
      one_hit( ch, pChar, gsn_whirlwind, FALSE);
    }
  }
   
  if ( !found )
    {
      act( "$n looks dizzy, and a tiny bit embarassed.", ch, NULL, NULL, TO_ROOM );
      act( "You feel dizzy, and a tiny bit embarassed.", ch, NULL, NULL, TO_CHAR );
    }
  
  lagged = skill_table[gsn_whirlwind].beats;
  if ( IS_AFFECTED2(ch, AFF2_WARCRY_VIGOR) )
    lagged -= 6;
  WAIT_STATE(ch, lagged );
  //WAIT_STATE( ch, skill_table[gsn_whirlwind].beats );
  check_improve(ch,gsn_whirlwind,TRUE,2);
  if ( !found && number_percent() < 25 )
    {
      act( "$n loses $s balance and falls into a heap.",  ch, NULL, NULL, TO_ROOM );
      act( "You lose your balance and fall into a heap.", ch, NULL, NULL, TO_CHAR );
      ch->position = POS_STUNNED;
    }
   
  return;
}      

/**************************************************/

/************************************************************************/
/* mlkesl@stthomas.edu  =====>  Ascii Automapper utility    */
/* Let me know if you use this. Give a newbie some credit,    */
/* at least I'm not asking how to add classes...      */
/* Also, if you fix something could ya send me mail about, thanks  */
/* PLEASE mail me if you use this ot like it, that way I will keep it up*/
/************************************************************************/
/* MapArea ->   when given a room, ch, x, and y,...      */
/*       this function will fill in values of map as it should   */
/* ShowMap ->   will simply spit out the contents of map array    */
/*    Would look much nicer if you built your own areas  */
/*    without all of the overlapping stock Rom has    */
/* do_map  ->  core function, takes map size as argument    */
/************************************************************************/
/* To install::               */
/*  remove all occurances of "u1." (or union your exits)    */
/*  add do_map prototypes to interp.c and merc.h (or interp.h)  */
/*  customize the first switch with your own sectors    */
/*  remove the color codes or change to suit your own color coding  */
/* Other stuff::              */
/*  make a skill, call from do_move (only if ch is not in city etc) */
/*  allow players to actually make ITEM_MAP objects      */
/*   change your areas to make them more suited to map code! :)  */
/************************************************************************/   

#define MAX_MAP 150
#define MAX_MAP_DIR 4

char *map[MAX_MAP][MAX_MAP];
int offsets[4][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0,-1} };

void MapArea (ROOM_INDEX_DATA *room, CHAR_DATA *ch, int x, int y, int min, int max)
{
  ROOM_INDEX_DATA *prospect_room;
  EXIT_DATA *pexit;
  int door;

  /* marks the room as visited */
  switch (room->sector_type)
    {
    case SECT_INSIDE:    map[x][y]="{W%";    break;
    case SECT_CITY:       map[x][y]="{W#";    break;
    case SECT_FIELD:    map[x][y]="{G\"";    break;
    case SECT_FOREST:    map[x][y]="{g@";    break;
    case SECT_HILLS:    map[x][y]="{G^";    break;
    case SECT_MOUNTAIN:    map[x][y]="{y^";    break;
    case SECT_WATER_SWIM:  map[x][y]="{B~";    break;
    case SECT_WATER_NOSWIM:  map[x][y]="{b~";    break;
    case SECT_UNUSED:    map[x][y]="{DX";    break;
    case SECT_AIR:      map[x][y]="{C:";    break;
    case SECT_DESERT:    map[x][y]="{Y=";    break;
    case SECT_UNDERGROUND:  map[x][y]="{y%";    break;
    case SECT_GRAVEYARD:  map[x][y]="{D@";    break;
      /*    case SECT_ROAD:    map[x][y]="{m+";    break;*/
    default:       map[x][y]="{yo";
    }

  for ( door = 0; door < MAX_MAP_DIR; door++ ) 
    {
      if (
    (pexit = room->exit[door]) != NULL
    /*    &&   pexit->u1.to_room != NULL 
    &&   can_see_room(ch,pexit->u1.to_room)   optional */
    /*&&   !IS_SET(pexit->exit_info, EX_CLOSED)*/
    )
        { /* if exit there */

    prospect_room = pexit->u1.to_room;

    if ( prospect_room->exit[rev_dir[door]] &&
         prospect_room->exit[rev_dir[door]]->u1.to_room!=room)
      { /* if not two way */
        if ((prospect_room->sector_type==SECT_CITY)
      ||  (prospect_room->sector_type==SECT_INSIDE))
    map[x][y]="{W@";      
        else
    map[x][y]="{D?";
        return;
      } /* end two way */

    if ((x<=min)||(y<=min)||(x>=max)||(y>=max)) return;
    if (map[x+offsets[door][0]][y+offsets[door][1]]==NULL) {
      MapArea (pexit->u1.to_room,ch,
         x+offsets[door][0], y+offsets[door][1],min,max);
    }

  } /* end if exit there */
    }
  return;
}
void MudMapArea (ROOM_INDEX_DATA *room, CHAR_DATA *ch, int x, int y,
     int min, int max, AREA_DATA *pArea)
{
  ROOM_INDEX_DATA *prospect_room;
  EXIT_DATA *pexit;
  int door;

  /* marks the room as visited */
  switch (room->sector_type)
    {
    case SECT_INSIDE:    map[x][y]="{W%";    break;
    case SECT_CITY:       map[x][y]="{W#";    break;
    case SECT_FIELD:    map[x][y]="{G\"";    break;
    case SECT_FOREST:    map[x][y]="{g@";    break;
    case SECT_HILLS:    map[x][y]="{G^";    break;
    case SECT_MOUNTAIN:    map[x][y]="{y^";    break;
    case SECT_WATER_SWIM:  map[x][y]="{B~";    break;
    case SECT_WATER_NOSWIM:  map[x][y]="{b~";    break;
    case SECT_UNUSED:    map[x][y]="{DX";    break;
    case SECT_AIR:    map[x][y]="{C:";    break;
    case SECT_DESERT:    map[x][y]="{Y=";    break;
    case SECT_UNDERGROUND:  map[x][y]="{y%";    break;
    case SECT_GRAVEYARD:  map[x][y]="{D@";    break;
      /*    case SECT_ROAD:    map[x][y]="{m+";    break;*/
    default:       map[x][y]="{yo";
    }

  for ( door = 0; door < MAX_MAP_DIR; door++ ) 
    {
      if ( ( pexit = room->exit[door] )
    /*    &&   pexit->u1.to_room != NULL 
    &&   can_see_room(ch,pexit->u1.to_room)   optional */
    /*&&   !IS_SET(pexit->exit_info, EX_CLOSED)*/
    )
        { /* if exit there */

    prospect_room = pexit->u1.to_room;

    if ( prospect_room->exit[rev_dir[door]] &&
         prospect_room->exit[rev_dir[door]]->u1.to_room!=room)
      { /* if not two way */
        if ((prospect_room->sector_type==SECT_CITY)
      ||  (prospect_room->sector_type==SECT_INSIDE))
    map[x][y]="{W@";      
        else
    map[x][y]="{D?";
        return;
      } /* end two way */

    if ((x<=min)||(y<=min)||(x>=max)||(y>=max)) return;
    if (map[x+offsets[door][0]][y+offsets[door][1]]==NULL) {
      if (pexit->u1.to_room->area == pArea)
      MudMapArea (pexit->u1.to_room,ch,
         x+offsets[door][0], y+offsets[door][1],min,max, pArea);
    }

  } /* end if exit there */
    }
  return;
}

void ShowMap( CHAR_DATA *ch, int min, int max)
{
  int x,y;

  for (x = min; x < max; ++x) 
    {
      for (y = min; y < max; ++y)
  {
    if (map[x][y]==NULL) send_to_char(" ",ch);    
    else     send_to_char(map[x][y],ch);   
  }
      send_to_char("\n\r",ch); 
    }   

  return;
}

void do_map( CHAR_DATA *ch, char *argument )
{
  int size,center,x,y,min,max;
  char arg1[10];

  if (get_skill(ch,gsn_automap) < 1)
    {
      send_to_char("You cannot think straight.\n\r",ch);
      return;
    }

  if (get_skill(ch,gsn_automap) <  number_percent() )
    {
      send_to_char("You lose track of where you are.\n\r",ch);
      return;
    }

  one_argument( argument, arg1 );
  size = atoi (arg1);

  size=URANGE(7,size,MAX_MAP);
  center=MAX_MAP/2;

  min = MAX_MAP/2-size/2;
  max = MAX_MAP/2+size/2;

  for (x = 0; x < MAX_MAP; ++x)
    for (y = 0; y < MAX_MAP; ++y)
      map[x][y]=NULL;

  /* starts the mapping with the center room */
  MapArea(ch->in_room, ch, center, center, min, max); 

  /* marks the center, where ch is */
  map[center][center]="{R*";  
  ShowMap (ch, min, max); 
  send_to_char("{x",ch);
  check_improve(ch,gsn_automap,TRUE,2);
  WAIT_STATE( ch, 4 );
  return;
}

void do_map_mud( CHAR_DATA *ch, char *argument )
{
  int size,center,x,y,min,max;
  char arg1[10];

  one_argument( argument, arg1 );
  size = atoi (arg1);

  size=URANGE(7,size,MAX_MAP);
  center=MAX_MAP/2;

  min = MAX_MAP/2-size/2;
  max = MAX_MAP/2+size/2;

  for (x = 0; x < MAX_MAP; ++x)
    for (y = 0; y < MAX_MAP; ++y)
      map[x][y]=NULL;

  /* starts the mapping with the center room */
  MudMapArea(ch->in_room, ch, center, center, min, max, ch->in_room->area); 

  /* marks the center, where ch is */
  map[center][center]="{R*";  
  ShowMap (ch, min, max); 

  return;
}

void do_circle( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
 
  one_argument( argument, arg );
 
  if (get_skill(ch,gsn_circle) < 1) {
    send_to_char("Huh?\n\r",ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You cannot see.\n\r",ch);
    return;
  }
  if (arg[0] == '\0')
    {
      send_to_char("Circle whom?\n\r",ch);
      return;
    }
 
 
  else if ((victim = get_char_room(ch,arg)) == NULL)
    {
      send_to_char("They aren't here.\n\r",ch);
      return;
    }
 
  if (ch->move < 30) {
    send_to_char("You are too tired to circle.\n\r",ch);
    return;
  }
    
  if ( is_safe( ch, victim, FALSE ) )
    return;
 
  if (check_killsteal(ch,victim))
      return;
 
  if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
      send_to_char( "You need to wield a weapon to circle.\n\r", ch );
      return;
    }
 
  if ( ( victim = ch->fighting ) == NULL )
    {
      send_to_char( "You must be fighting in order to circle.\n\r", ch );
      return;
    }

  if (ch == victim->fighting)
    {
      act( "$N tracks your movements so you cannot circle.{x", ch, NULL,
     victim, TO_CHAR    );
      return;
    }

  if ( victim->hit < victim->max_hit / 3)
  {
    send_to_char( "They are too alert for you to sneak in.\n\r",ch);
    return;
  }
  
  WAIT_STATE( ch, skill_table[gsn_circle].beats );
  if ( number_percent( ) < get_skill(ch,gsn_circle)
  || ( get_skill(ch,gsn_circle) >= 2 && !IS_AWAKE(victim) ) )
    {
      check_improve(ch,gsn_circle,TRUE,1);
      multi_hit( ch, victim, gsn_circle );
    }
  else
    {
      check_improve(ch,gsn_circle,FALSE,1);
      damage( ch, victim, 0, gsn_circle,DAM_NONE,TRUE, FALSE);
    }
  ch->move -= 20;
  return;
}
 
void do_shatter( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int chance;

  if ((chance = get_skill(ch,gsn_shatter)) == 0)
    {
      send_to_char( "You don't know how to shatter weapons.\n\r", ch );
      return;
    }

  if (IS_GHOST(ch)) {
    send_to_char("Shatter is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( get_eq_char( ch, WEAR_WIELD ) != NULL) 
    {
      send_to_char( "You must NOT wield a weapon to shatter.\n\r", ch );
      return;
    }

  if ( ( victim = ch->fighting ) == NULL )
    {
      send_to_char( "You aren't fighting anyone.\n\r", ch );
      return;
    }

  if (IS_SET(ch->in_room->room_flags, ROOM_ARENA))
    {
      send_to_char("Arena does not allow this function.\n\r",ch);
      return;
    }

  if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
      send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
      return;
    }

  chance = 15 + ((ch->level - obj->level) * 5);

  /* and now the attack */
  if (number_percent() < chance)
    {
      WAIT_STATE( ch, skill_table[gsn_shatter].beats );
      shatter( ch, victim );
      check_improve(ch,gsn_shatter,TRUE,1);
    }
  else
    {
      WAIT_STATE(ch,skill_table[gsn_shatter].beats);
      act("You fail to shatter $N's weapon.",ch,NULL,victim,TO_CHAR);
      act("$n tries to shatter your weapon, but fails.",ch,NULL,victim,TO_VICT);
      act("$n tries to shatter $N's weapon, but fails.",ch,NULL,victim,TO_NOTVICT);
      check_improve(ch,gsn_shatter,FALSE,1);
    }
  check_killer(ch,victim);
  return;
}
/*
 * Shatter
 * Caller must check for successful attack.
 */
void shatter( CHAR_DATA *ch, CHAR_DATA *victim )
{
  OBJ_DATA *obj;

  if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    return;

  if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
      act("$S weapon won't shatter!",ch,NULL,victim,TO_CHAR);
      act("$n tries to shatter your weapon, but your weapon is stronger than him!",
    ch,NULL,victim,TO_VICT);
      act("$n tries to shatter $N's weapon, but fails.",ch,NULL,victim,TO_NOTVICT);
      return;
    }

  act( "$n {Dsha{Wtt{wers{x your weapon, sending pieces flying!", 
       ch, NULL, victim, TO_VICT    );
  act( "You {Dsha{Wtt{wer{x $N's weapon, sending pieces flying!",  ch, NULL, victim, TO_CHAR    );
  act( "$n {Dsha{Wtt{wers{x $N's weapon, sending pieces flying.",  ch, NULL, victim, TO_NOTVICT );

  obj_from_char( obj );
  extract_obj(obj);
  /*
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
    obj_to_char( obj, victim );
    else
    {
    obj_to_room( obj, victim->in_room );
    if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
    get_obj(victim,obj,NULL);
    }
  */
  return;
}

void do_bind( CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int heal_amount=0, band_count=0, bandage=0;
  OBJ_DATA *obj=NULL, *obj_next=NULL;
  AFFECT_DATA af;

  one_argument(argument,arg);

  if ((victim = get_char_room(ch,arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }

  if ( IS_AFFECTED2( victim, AFF2_BINDED ) )
  {
    if ( ch==victim )
      send_to_char( "You are already bandaged up tight.\n\r", ch );
    else
      send_to_char( "They are already bandaged up tight.\n\r", ch );
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("Binding is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  //75% Health or more, do nothing
  if ( ((100*victim->hit)/GET_HP(victim)) > 74 )
  {
    if ( victim == ch )
      send_to_char( "Your wounds do not warrant the waste of bandages.\n\r", ch );
    else
      send_to_char("They are just fine without your help.\n\r",ch);
    return;
  }

  if ( victim->hit < 1 )
  {
    heal_amount = GET_HP(victim) / 3;
    bandage = 4;
  }
  else if ( PERCENT(victim->hit, GET_HP(victim)) < 10 ) //10% Health
  {
    heal_amount = GET_HP(victim) / 4;
    bandage = 3;
  }
  else if ( PERCENT(victim->hit, GET_HP(victim)) < 50 ) //50% Health
  {
    heal_amount = GET_HP(victim) / 7;
    bandage = 2;
  }
  else //50-74% Health, weakest healing
  {
    heal_amount = GET_HP(victim) / 12;
    bandage = 1;
  }

  int i=0;
  for ( obj = ch->carrying; obj != NULL; obj = obj_next )
  {
    obj_next = obj->next_content;

    if (obj->next_content == obj)
    {
      bugf("Error in bind: Infinite obj loop. %s",interp_cmd);
      obj_next = NULL;
    }

    if ( obj->item_type == ITEM_BANDAGE )
      band_count++;
  }

  if ( band_count < bandage )
  {
    printf_to_char( ch, "You would need %d bandage%s to have a chance at helping.\n\r",
      bandage,
      bandage > 1 ? "s" : "" );
    return;
  }

  if ( number_range(1,25) >= 20)
  {
    send_to_char("You failed.\n\r",ch);
    check_improve(ch,gsn_bind,FALSE,1);
    return;
  }

  if ( ch==victim )
  {
    act("$n kneels down and wraps $s wounds in bandages.\n\r",ch,NULL,victim,TO_NOTVICT);
    act("You apply bandages to your wounds.\n\r",ch,NULL,victim,TO_CHAR);
  }
  else
  {
    act("$n kneels down and begins to bandage $N.\n\r",ch,NULL,victim,TO_NOTVICT);
    act("$n kneels down and touches bandages to your wounds.\n\r",ch,NULL,victim,TO_VICT);
    act("You apply bandages to $N's wounds.\n\r",ch,NULL,victim,TO_CHAR);
  }
  victim->hit += heal_amount;
  if ( victim->hit > GET_HP(victim) )
   victim->hit = GET_HP(victim);
  victim->position = POS_STANDING;
  check_improve(ch,gsn_bind,TRUE,2);

  af.where     = TO_AFFECTS2;
  af.type      = gsn_bind;
  af.level     = ch->level;
  af.duration  = ch->level / 11;
  af.location  = APPLY_MOVE;
  af.modifier  = UMAX( 50, ch->level );
  af.bitvector = AFF2_BINDED;
  affect_to_char( victim, &af );

  //remove the bandages

  obj_next = NULL;

  i = bandage;
  for ( obj = ch->carrying; obj != NULL; obj = obj_next )
  {
    obj_next = obj->next_content;

    if (obj->next_content == obj)
    {
      bugf("Error in bind: Infinite obj loop. %s",interp_cmd);
      obj_next = NULL;
    }

    if ( i > 0 )
    {
      if ( obj->item_type == ITEM_BANDAGE )
      {
        obj_from_char(obj);
        extract_obj(obj);
      }
      i--;
    }
  }

  return;
}

void do_strangle( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim; 
  char arg[MSL];
  int chance;
 
  one_argument( argument, arg );
  
  if ( IS_GHOST( ch ) )
  {
    send_to_char( "Strangling is useless as you are still {rDEAD{x.\n\r", ch );
    return;
  }

  if ( arg[0] == '\0' )
  {
    if ( ( victim = ch->fighting ) == NULL )
    {
    send_to_char( "But you aren't in combat!\n\r", ch );
    return;
  }
  }

  else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }
 
  if ( victim == ch )
  {
    send_to_char("You think twice about it and decide against it.\n\r",ch);
    return;
  }  

  if ( is_safe( ch, victim, FALSE ) )
    return;

  if ( check_killsteal( ch, victim ) )
    return;

  if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
  {
    act( "But $N is such a good friend.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( ( chance = get_skill( ch, gsn_strangle ) / 2 ) == 0 )
  {
    send_to_char( "You don't know how to strangle.\n\r", ch );
    return;
  }
  /*no head, no neck, can't strangle*/
  /*  if (!IS_SET(victim, PART_HEAD))
    { 
      send_to_char("it doesn't seem to have a neck to strangle.\n\r",ch);
      return;
    } 
  */
  if ( IS_AFFECTED( victim, AFF_SLEEP ) )
  {
    send_to_char( "They are already gasping for breath.\n\r", ch );
    return;
  }

  if ( IS_SET( victim->form, FORM_UNDEAD ) )
  {
    send_to_char("Attempting to strangle the undead doesn't seem to have an effect.\n\r", ch );
    return;
  }

  if ( ( get_eq_char( ch, WEAR_WIELD )
  ||   ( get_eq_char( ch, WEAR_SECONDARY ) ) ) )
  {
      send_to_char( "You need to have your hands free.\n\r", ch );
      return;
  }
 
  WAIT_STATE ( ch, 2 * PULSE_VIOLENCE );

  if( number_percent() < chance )
  {
      AFFECT_DATA af;
   
      af.where      =  TO_AFFECTS;
      af.type    =   gsn_strangle;
      af.level      =  ch->level;
      af.duration   =  number_range( 1, 3 );
      af.location  =  APPLY_NONE;
      af.modifier  =  0;
      af.bitvector  =  AFF_SLEEP;
      affect_to_char( victim, &af );

      if ( IS_AFFECTED( ch, AFF_INVISIBLE ) )
        remove_affect( ch, TO_AFFECTS, AFF_INVISIBLE );

      act( "$n squeezes your throat...and you pass out.\n\r",
            ch, NULL, victim, TO_VICT );
      act( "You strangle $N.", ch, NULL, victim, TO_CHAR );
      act( "$n strangles $N!", ch, NULL, victim, TO_NOTVICT );
      stop_fighting( ch, TRUE );
      check_improve( ch, gsn_strangle, TRUE, 2 );
  }
  else
  {
      char buf[MAX_STRING_LENGTH];
      mprintf(sizeof(buf), buf, "Help!  %s tried to strangle me.",ch->name );
      do_function( victim, &do_yell, buf );
      act( "$n attempted to strangle $N.", ch, NULL, victim, TO_NOTVICT );
      act( "$N breaks your strangle hold.", ch, NULL, victim, TO_CHAR );
      act( "$n tries to strangle you, but you break the hold.",
          ch, NULL, victim, TO_VICT );
      /*      set_fighting(ch, victim);
        check_killer(ch,victim);*/
      check_improve( ch, gsn_strangle, FALSE, 1 );
    }
        
}
void do_crosscut( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance=0;
  bool using_dagger = FALSE, using_axe = FALSE;
  OBJ_DATA *obj_weapon;

  one_argument(argument,arg); 

  /*  Must have sword or clan weapon wielded*/
  if ((obj_weapon = get_eq_char( ch, WEAR_WIELD ))!= NULL )  
  {  
    if ( obj_weapon->value[0] != WEAPON_SWORD
    &&   obj_weapon->value[0] != WEAPON_AXE
    &&   obj_weapon->value[0] != WEAPON_DAGGER )
    //&&   obj_weapon->value[0] != WEAPON_SPEAR ) Maybe? Spears are also staves right now...
    {
      send_to_char("You cannot perform a crosscut with that weapon.\n\r",ch);
      return;
    }
  }
  else
  {
    send_to_char("You must use a weapon to perform a crosscut.\n\r",ch);
    return;
  }

  if (ch->fighting == NULL)
    {
      send_to_char("By honor, you cannot start a fight with a crosscut.\n\r"
       ,ch);
      return;
    } 


  if ((chance = get_skill(ch,gsn_crosscut)) < 1)
    {
      send_to_char("You have no idea how to crosscut.\n\r",ch);
      return;
    }

  if (arg[0] == '\0')
    {
      victim = ch->fighting;
      if (victim == NULL)
  {
    send_to_char("But you aren't in combat.\n\r",ch);
    return;
  }
    }

  else if ((victim = get_char_room(ch,arg)) == NULL)
    {
      send_to_char("They aren't here.\n\r",ch);
      return;
    }
 
  if (victim == ch)
  {
    send_to_char("You don't remember why you wanted to kill yourself.\n\r",ch);
    return;
  }  

  if (is_safe(ch,victim, FALSE))
    return;

  if (check_killsteal(ch,victim))
      return;

  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
  {
    act("But $N is such a good friend.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if ( ch->move < 20 )
  {
    send_to_char( "You feel too tired to crosscut.\n\r", ch );
    return;
  }

  chance = get_skill(ch,gsn_crosscut);
  chance -=25;
  /* modifiers */

  /* dexterity */
  chance += get_curr_stat(ch,STAT_DEX);
  chance -= get_curr_stat(victim,STAT_DEX);

  /* wisdom   wise martial artists know what to expect from opponents */
  /*chance += (3*get_curr_stat(ch,STAT_WIS))/2;    
    chance -= get_curr_stat(victim,STAT_WIS);  */

  /* speed  */
  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if (IS_AFFECTED(victim, AFF_SLOW))
    chance +=10;
  /* level */
  
  chance += (ch->level - victim->level);
  if (chance == 0) chance = 1;  

  /* now the attack */

  if (number_percent() < chance)
  {
    int dam;
    OBJ_DATA *wield;
        
    act("You perform a well-executed {wcr{Wo{Dss{wcut{x on $N.", ch, NULL, victim, TO_CHAR);
    act("$n performs a well-executed {wcr{Wo{Dss{wcut{x on you.",
      ch, NULL, victim, TO_VICT);
    act("$n performs a well-executed {wcr{Wo{Dss{wcut{x on $N!",
      ch, NULL, victim, TO_NOTVICT);
    wield = get_eq_char( ch, WEAR_WIELD );

    if (wield)
      dam = dice(wield->value[1],wield->value[2]) * get_skill(ch, gsn_crosscut)/100;
    else
      dam =0;

    //Adding some variety to the skill - RWL 11/09
    if ( obj_weapon->value[0] == WEAPON_DAGGER )
    {
      dam = dam * 75 / 100; //Daggers are lighter, 25% less damage but less lag
      using_dagger = TRUE;
    }

    if (IS_WEAPON_STAT(obj_weapon, WEAPON_TWO_HANDS))
    {
      dam = dam + (dam * 10 / 100 ); //Two-handers get a 10% bonus
      using_axe = TRUE;
    }

    dam += dice(ch->level,20) + GET_DAMROLL(ch);
    damage(ch,victim,dam,gsn_crosscut,DAM_SLASH,TRUE, FALSE);
    check_improve(ch,gsn_crosscut,TRUE,2);
    ch->move -= 20;
  }
  else
  {
    send_to_char("You wait for an opening in your opponent's defense.\n\r", ch);
  }

  if ( using_dagger )
    WAIT_STATE(ch, number_range(1, 2) * PULSE_VIOLENCE);
  else if ( using_axe )
    WAIT_STATE(ch, number_range(2, 3) * PULSE_VIOLENCE);
  else
    WAIT_STATE(ch, number_range(1, 3) * PULSE_VIOLENCE);
}

void do_gore( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument(argument,arg); 

  if ( ch->race != race_lookup("minotaur"))
    {
      send_to_char("You don't even have any horns!\n\r",ch);
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
 
  if (victim == ch)
    {
      send_to_char("How the hell are you going to do that?\n\r",ch);
      return;
    }  

  if (is_safe(ch,victim, FALSE))
    return;

  if (check_killsteal(ch,victim))
      return;

  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
      act("But $N is such a good friend.",ch,NULL,victim,TO_CHAR);
      return;
    }

  chance = get_skill(ch,gsn_gore);
  /* modifiers */

  /* dexterity */
  chance += get_curr_stat(ch,STAT_DEX);
  chance -= get_curr_stat(victim,STAT_DEX);

  /* speed  */
  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 25;

  /* level */
  chance += (ch->level - victim->level);
  if (chance <= 0) chance = 1;

  if (number_percent() < chance)
    {
      int result;
      result = number_range((int) ((float)ch->level/3), ch->level);
      result += get_curr_stat(ch,STAT_STR) + get_curr_stat(ch,STAT_DEX);
      result -= get_curr_stat(victim,STAT_DEX) + get_curr_stat(ch,STAT_WIS);
      send_to_char("You gore your Victim.\n\r",ch);
      act("$n is hit by $N's impaling horns!",victim,NULL,ch,TO_ROOM);
      act("$n rams you, impaling you on $s's horns!",ch,NULL,victim,TO_VICT);
      damage(ch,victim,result*2,gsn_gore,DAM_PIERCE,TRUE, FALSE);
      check_improve(ch,gsn_gore,TRUE,2);
    } else {
      check_improve(ch,gsn_gore,FALSE,2);
      send_to_char("You fail to gore your victim.\n\r",ch);
      damage(ch,victim,0,gsn_gore,DAM_PIERCE,TRUE, FALSE);
    }
  WAIT_STATE(ch,skill_table[gsn_gore].beats);
}


void do_nerve( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  OBJ_DATA *pri_obj, *sec_obj;
  AFFECT_DATA af;

  one_argument(argument,arg);

  pri_obj = get_eq_char( ch, WEAR_WIELD );
  sec_obj = get_eq_char( ch, WEAR_SECONDARY );

  if ( pri_obj && sec_obj)
  {  
      send_to_char("You need a hand free for that.\n\r",ch);
      return;
  }

  if (get_skill(ch, gsn_nerve) < 1)
  {
      send_to_char("Nerve strikes are beyond your skill.\n\r", ch);
      return;
  }

  if (arg[0] == '\0')
  {
      victim = ch->fighting;

      if (victim == NULL)
    {
      send_to_char("Strike who's nerve?\n\r",ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch,arg)) == NULL)
  {
      send_to_char("They aren't here.\n\r",ch);
      return;
  }
 
 if (victim == ch)
    {
      send_to_char("Very funny.\n\r",ch);
      return;
    }  

  if (is_safe(ch,victim, TRUE))
    return;

  if (check_killsteal(ch,victim))
    return;
  
  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
      act("But $N is such a good friend.",ch,NULL,victim,TO_CHAR);
      return;
    }

  /* if victim has no brain, then no nerves, and nerve pinch can't work*/
  if (!IS_SET(victim->parts, PART_BRAINS)) /*victim has no neverous sys*/
    {
      act("It seems $N has no nervous system to attack.",ch,NULL,
    victim,TO_CHAR);
      return;
    }
    
  if ( (chance = get_skill(ch, gsn_nerve)) == 0
       || (!IS_NPC(ch) &&  ch->level < skill_table[gsn_nerve].skill_level[ch->gameclass]))
    {
      send_to_char("You are unfamiliar with how to pinch nerves.\n\r",ch);
      return;
    }

  chance *= (get_skill(ch, gsn_nerve));

  /* modifiers */

  /* dexterity */
  chance += get_curr_stat(ch,STAT_DEX);
  chance -= 2 * get_curr_stat(victim,STAT_DEX);

  /* wisdom   wise martial artists know what to expect from opponents */
  chance += (3*get_curr_stat(ch,STAT_WIS))/2;    
  chance -= get_curr_stat(victim,STAT_WIS);  

  /* speed  */
  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 25;

  /* level */
  chance += (ch->level - victim->level);
  if (chance <= 0) chance = 1; 

  /* now the attack */

  if (number_percent() < chance)
  {
    /* !!! REUSE CHANCE VARIABLE FOR DAMAGE !!! */
    chance = (int) ( (float) (ch->level - victim->level)*1.5);

    act("$n is hit by a nerve pinch.",victim,NULL,NULL,TO_ROOM);
    act("$n strikes a sensitive nerve.",ch,NULL,victim,TO_VICT);
    damage(ch,victim,number_range(chance,
      ch->level),gsn_nerve,DAM_SHOCK,TRUE, FALSE);
    check_improve(ch,gsn_nerve,TRUE,6);
    if (!is_affected(victim,gsn_nerve))
    {
      af.where  = TO_AFFECTS;
      af.location  = APPLY_HITROLL;
      af.modifier  = -2;
      af.type   = gsn_nerve;
      af.level   = ch->level;
      af.duration  = UMAX( 1, (ch->level/33) );
      af.bitvector = 0;
      affect_to_char(victim,&af);
      DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
    }
    /* reuse karate_stun function*/
    }
    else
    {
      act("$n twists out of a nerve pinch.",victim,NULL,NULL,TO_ROOM);
      act("$n misses a sensitive nerve.",ch,NULL,victim,TO_VICT);
      damage(ch,victim,0,gsn_nerve,DAM_SHOCK,TRUE, FALSE);
      check_improve(ch,gsn_nerve,FALSE,2);
    }

    WAIT_STATE(ch,skill_table[gsn_nerve].beats);
}

void do_takedown( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument(argument,arg); 

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You cannot see.\n\r",ch);
    return;
  }

  if ( ((chance = get_skill(ch, gsn_takedown)) == 0)
       ||   ((!IS_NPC(ch)
        &&   ch->level < skill_table[gsn_takedown].skill_level[ch->gameclass])
       /* Must have Karate prerequisit */
       &&  (get_skill(ch,gsn_takedown) > 1)))
    {
      send_to_char("You are far too inexperienced to try that.\n\r",ch);
      return;
    }

  if (arg[0] == '\0')
    {
      victim = ch->fighting;
      if (victim == NULL)
  {
    send_to_char("But you aren't in combat.\n\r",ch);
    return;
  }
    }

  else if ((victim = get_char_room(ch,arg)) == NULL)
    {
      send_to_char("They aren't here.\n\r",ch);
      return;
    }

  if (victim == ch)
    {
      send_to_char("Very funny.\n\r",ch);
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
  chance -= 2 * get_curr_stat(victim,STAT_DEX);

  /* speed  */
  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 25;

  /* level */
  chance += (ch->level - victim->level);
  if (chance <= 0) chance = 1;  /* if you made it this far w/o returning,
           then you have the skill, but it is hard */

  /* now the attack */

  if (number_percent() < chance)
    {

      act("$N is thrown across the room!",ch,NULL,victim,TO_NOTVICT);
      act("$n takes you down!",ch,NULL,victim,TO_VICT);
      act("your takedown sends $N flying to the ground.",ch,NULL,victim,TO_CHAR);
      check_improve(ch,gsn_takedown,TRUE,6);

      DAZE_STATE(victim, 2*PULSE_VIOLENCE);
      WAIT_STATE(ch,2*PULSE_VIOLENCE);
      victim->position = POS_RESTING;
      damage(ch,victim,number_range(ch->level/2,
                    ch->level),gsn_takedown,DAM_BASH,TRUE, FALSE);
      update_pos( victim );
      /*WAIT_STATE(victim,PULSE_VIOLENCE); */  /*victim is lagged */
  }
  else
  {
      damage(ch,victim,0,gsn_takedown,DAM_BASH,TRUE, FALSE);

      act("Your takedown misses and you fall to the floor!",
      ch,NULL,victim,TO_CHAR);
      act("$n misses $s target and falls on $s face.",
      ch,NULL,victim,TO_NOTVICT);
      act("You avoid $n's takedown, and watch $m to fall flat on $s face.",
      ch,NULL,victim,TO_VICT);

      check_improve(ch,gsn_takedown,FALSE,2);

      WAIT_STATE(ch,2*PULSE_VIOLENCE);
    }
}

void do_buckkick( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument(argument,arg); 

  if ( ch->race != race_lookup("choja"))
    {
      send_to_char("You don't even have strong enough legs!\n\r",ch);
      return;
    }

  if (arg[0] == '\0')
    {
      victim = ch->fighting;
      if (victim == NULL)
  {
    send_to_char("Buck who?\n\r",ch);
    return;
  }
    }
  else if ((victim = get_char_room(ch,arg)) == NULL)
    {
      send_to_char("They aren't here.\n\r",ch);
      return;
    }
 
  if (victim == ch)
    {
      send_to_char("How the hell are you going to do that?\n\r",ch);
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

  chance = get_skill(ch, gsn_buckkick) / 2;
  chance += get_curr_stat(ch,STAT_DEX);
  chance -= 3 * get_curr_stat(victim,STAT_DEX) /2;

  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 25;

  chance += (ch->level - victim->level);
  if (chance == 0) chance = 1;

  if (number_percent() < chance)
    {
      int result;
      result = number_range((int) ((float)ch->level/3),
          (int) ((float) ch->level/2));
      result *= (get_curr_stat(ch,STAT_STR) / 17);
      act("$n is hit by $s bucking kick!",ch,NULL,victim,TO_NOTVICT);
      act("$n bucks you with a donkey kick!",ch,NULL,victim,TO_VICT);
      act("Your bucking kick sends $N flying!",ch,NULL,victim,TO_CHAR);
      damage(ch,victim,result,gsn_buckkick,DAM_BASH,TRUE, FALSE);
    }
  else
    {
      act("You miss $N with your bucking kick.",ch,NULL,victim,TO_CHAR);
      act("$n misses you with a bucking kick.",ch,NULL,victim,TO_VICT);
      act("$n misses $N with a bucking kick.",ch,NULL,victim,TO_NOTVICT);
      damage(ch,victim,0,gsn_buckkick,DAM_BASH,TRUE, FALSE);
    }
  check_improve(ch,gsn_buckkick,TRUE,2);
  WAIT_STATE(ch,skill_table[gsn_buckkick].beats);
}

void do_dart(CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int chance;
  char arg[MSL];
  int dam,skill;

  one_argument(argument,arg);

  if (IS_NPC(ch))
  {
    if (ch->level >= 50)
      chance = number_range(50,ch->level);
    else
      chance = 50;
  }
  else
    chance = get_skill(ch,gsn_dart);

  if (chance == 0)
    {
      send_to_char( "You don't know how to toss weapons.\n\r", ch );
      return;
    }
  
  if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
      send_to_char( "You do not have that item.\n\r", ch );
      return;
    }

  if (obj->item_type != ITEM_WEAPON) 
    {
      send_to_char("You cannot throw a non-weapon.\n\r",ch);
      return;
    }

  if (IS_WEAPON_STAT(obj, WEAPON_CLAN) )
    {
      send_to_char("You cannot throw a clan weapon.\n\r",ch);
      return;
    }

  if ( !can_use_clan_obj( ch, obj ) )
  {
    send_to_char( "That belongs to another clan.\n\r", ch );
    return;
  }

  switch (obj->value[0])
  {
    case(WEAPON_DAGGER) :
      /* Knight, Barbarian, Mystic, Druid, Highwayman  -> managed by const.c */
      break;
    case(WEAPON_SWORD)  :
      /* Knight only */
      if (ch->gameclass != cKnight) {
        send_to_char("You cannot throw that weapon effectively.\n\r",ch);
        return;
      }
      break;
    case(WEAPON_AXE) :
      if (ch->gameclass != cBarbarian) {
        send_to_char("You cannot throw that weapon effectively.\n\r",ch);
        return;
      }
      break;
    default:
      send_to_char("You cannot throw that weapon effectively.\n\r",ch);
      return;
  }

  if (obj->level > ch->level) 
  {
    send_to_char("You fail to retrieve such a hard object from your inventory.\n\r",ch);
    return;
  }

  if ( ( victim = ch->fighting ) == NULL )
  {
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }

  skill = chance;
  chance -= 15 + (get_curr_stat(ch,STAT_DEX));

  if (number_percent() < chance)
  {
    act("You hit $N with your tossed weapon.",ch,NULL,victim,TO_CHAR);
    act("$n hits you with a tossed weapon.",ch,NULL,victim,TO_VICT);
    act("$n hits $N with a tossed weapon.",ch,NULL,victim,TO_NOTVICT);
    if ( check_shield_block(ch,victim)) {
      act("$N blocks your tossed weapon with thier shield.",ch,NULL,victim,TO_CHAR);
      act("you block $n tossed weapon with your shield.",ch,NULL,victim,TO_VICT);
      act("$N blocks $n tossed weapon.",ch,NULL,victim,TO_NOTVICT);
      check_improve(ch, gsn_dart, TRUE, 1);
      WAIT_STATE( ch, skill_table[gsn_dart].beats );
      return;
    }

    if (obj->pIndexData->new_format)
      dam = dice(obj->value[1],obj->value[2]) * skill/10;
    else
      dam = number_range( obj->value[1] * skill/10, obj->value[2] * skill/10);

    if (get_eq_char(ch,WEAR_SHIELD) == NULL)  /* no shield = more */
      dam = dam * 11/10;

    /* sharpness! */
    if (IS_WEAPON_STAT(obj,WEAPON_SHARP))
    {
      int percent;
      if ((percent = number_percent()) <= (skill / 8))
        dam = 2 * dam + (dam * 2 * percent / 100);
    }

    /* sharpness! */
    if (IS_WEAPON_STAT(obj,WEAPON_VORPAL))
    {
      int percent;

      if ((percent = number_percent()) <= (skill / 10))
        dam = 2 * dam + (dam * 2 * percent / 50);
    }

    dam += GET_DAMROLL(ch);/* * UMIN(100,skill) /100;*/
    damage( ch, victim, dam, gsn_dart, DAM_OTHER, TRUE, FALSE );
    if (number_percent() < 8)
    {
      obj_from_char(obj);
      obj_to_char(obj, victim);
    }
    else
    {
      extract_obj(obj);
    }
    check_improve(ch,gsn_dart,TRUE,1);
    WAIT_STATE( ch, skill_table[gsn_dart].beats );
  }
  else
  {
    act("You fail to hit $N with your weapon.",ch,NULL,victim,TO_CHAR);
    act("$n tries to hit you with a tossed weapon, but fails.",ch,NULL,victim,TO_VICT);
    act("$n tries to hit $N with a tossed weapon, but fails.",ch,NULL,victim,TO_NOTVICT);
    extract_obj(obj);
    check_improve(ch,gsn_dart,FALSE,1);
    damage( ch, victim, 0, gsn_dart, DAM_OTHER, TRUE, FALSE );
    WAIT_STATE(ch,skill_table[gsn_dart].beats);
  }

  return;
}

#ifdef JOG
void do_jog( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_INPUT_LENGTH],arg[MAX_INPUT_LENGTH];
  char *p;
  bool dFound = FALSE;

  if (!ch->desc || *argument == '\0')
    {
      send_to_char("You run in place.\n\r",ch);
      return;
    }

  buf[0] = '\0';

  while (*argument != '\0')
    {
      argument = one_argument(argument,arg);
      strcat(buf,arg);
    }

  for( p = buf + strlen(buf)-1; p >= buf; p--)
    {
      if (!isdigit(*p))
  {
    switch( *p )
      {
      case 'n':
      case 's':
      case 'e':
      case 'w':
      case 'u':
      case 'd': dFound = TRUE;
        break;

      case 'o': break;

      default: send_to_char("Invalid direction.\n\r",ch);
        return;
      }
  }
      else if (!dFound) *p = '\0';
    }

  if (!dFound)
    {
      send_to_char("No directions specified.\n\r",ch); 
      return;
    }

  ch->desc->run_buf = str_dup( buf );
  ch->desc->run_head = ch->desc->run_buf;
  send_to_char("You start running...\n\r",ch);
  return;
}
#endif

bool check_feint(CHAR_DATA *ch, CHAR_DATA *victim)
{
  int chance;
  if ((get_skill(ch,gsn_feint) < 2) && 
      !is_racial_skill(ch,gsn_feint))
    return FALSE;

  chance = get_skill(ch,gsn_feint) / 10;
  chance += (ch->level - victim->level) * 2;
  chance += get_curr_stat(ch,STAT_DEX) - get_curr_stat(victim,STAT_DEX);
  chance = UMAX(5,chance);

  if (number_percent() > chance)
    {
      check_improve(ch,gsn_feint,FALSE,8);
      return FALSE;
    }
  else
    {
      act("Your clever {cfeint{x bypasses $N's defense.",ch,NULL,victim,TO_CHAR);
      act("$n's skillful {cfeint{x bypasses your defense.",ch,NULL,victim,TO_VICT);
      act("$n's {cfeint{x bypasses $N's defense.",ch,NULL,victim,TO_NOTVICT);
      check_improve(ch,gsn_feint,TRUE,8);
      return TRUE;
    }
}


void do_stomp( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim=0;
  int chance;

  one_argument(argument,arg);

  if (( (chance = get_skill(ch,gsn_stomp)) < 2)
  ||  ((IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH)))
  ||  (IS_NPC(ch) && (ch->size != SIZE_GIANT))
  ||  (!IS_NPC(ch) && (ch->level < skill_table[gsn_stomp].skill_level[ch->gameclass])))
  {
    send_to_char("Stomp on them? Oh please!!\n\r",ch);
    return;
  }

  if ( IS_SET( ch->in_room->room_flags,ROOM_UNDER_WATER ) )
  {
    send_to_char( "You can't do that under water.\n\r", ch );
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

  else if ((victim = get_char_room(ch,arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }

  if (ch->size <= victim->size)
  {
    send_to_char("But they're just as big as you are!\n\r",ch);
    return;
  }

  if (victim == ch)
  {
    send_to_char("You try to stomp on your own foot, inflicting great pain.\n\r",ch);
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

  /* size */
  chance += (ch->size - victim->size) + (get_skill(ch,gsn_stomp)/2);


  /* stats */
  chance += get_curr_stat(ch,STAT_STR)/2;
  chance -= (get_curr_stat(victim,STAT_DEX) * 2);
  chance += (get_curr_stat(ch,STAT_DEX) * 2);
  /* speed */
  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 10;

  /* level */
  chance += (ch->level - victim->level);

  if (!IS_NPC(victim)
      && chance < get_skill(victim,gsn_dodge) )
    { 
      chance -= 3 * (get_skill(victim,gsn_dodge) - chance);
    }

  /* Ensure that there is always at least a 5% chance of success/failure */
  chance = URANGE(5, chance, 95);

  /* now the attack */
  if (number_percent() < chance )
    {

      act("$n raises $s leg, and brings it crashing down, stomping you to the ground!",
    ch,NULL,victim,TO_VICT);
      act("You stomp on $N, driving $M to the ground!",ch,NULL,victim,TO_CHAR);
      act("$n drives $N to the ground with a stomp of $s foot.",
    ch,NULL,victim,TO_NOTVICT);
      check_improve(ch,gsn_stomp,TRUE,1);

      DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
      WAIT_STATE(ch,skill_table[gsn_stomp].beats);
      victim->position = POS_RESTING;
      damage(ch,victim,
       dice(ch->level,ch->size - victim->size),
       gsn_stomp,
       DAM_BASH,TRUE, FALSE);

    }
  else
    {
      damage(ch,victim,0,gsn_stomp,DAM_BASH,FALSE, FALSE);
      act("You fall flat on your face!",
    ch,NULL,victim,TO_CHAR);
      act("$n falls flat on $s face.",
    ch,NULL,victim,TO_NOTVICT);
      act("You evade $n's bash, causing $m to fall flat on $s face.",
    ch,NULL,victim,TO_VICT);
      check_improve(ch,gsn_stomp,FALSE,1);
      ch->position = POS_RESTING;
      WAIT_STATE(ch,skill_table[gsn_stomp].beats * 3/2);
    }
  check_killer(ch,victim);
}


void do_flying_kick( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;

  if ( !IS_NPC(ch)
  &&   ch->level < skill_table[gsn_flying].skill_level[ch->gameclass] )
  {
    send_to_char( "You are not yet trained for such a skill.\n\r", ch );
    return;
  }

  if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_FLYING))
    return;

  if ( ( victim = ch->fighting ) == NULL )
  {
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }

  WAIT_STATE( ch, skill_table[gsn_flying].beats );
  if ( get_skill(ch,gsn_flying) > number_percent())
    {
      damage(ch,victim,number_range( 1, ch->level ), gsn_flying,DAM_BASH,TRUE, FALSE);
      check_improve(ch,gsn_flying,TRUE,1);
    }
  else
    {
      damage( ch, victim, 0, gsn_flying,DAM_BASH,TRUE, FALSE);
      check_improve(ch,gsn_flying,FALSE,1);
    }
  return;
}


void do_demand(CHAR_DATA *ch,char *argument)
{
  CHAR_DATA  *victim;
  OBJ_DATA *obj;
  char i_name[MAX_INPUT_LENGTH];
  char m_name[MAX_INPUT_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char *target_name;
  int chance, vlevel;
  
  target_name = one_argument(argument,i_name);
  one_argument(target_name,m_name);
  chance = get_skill(ch,gsn_demand);
  if (chance == 0
      || ch->level < skill_table[gsn_demand].skill_level[ch->gameclass])
    {
      send_to_char("You are hardly intimidating enough to make demands of others.\n\r",ch);
      return;
    }

  if (IS_GHOST(ch)) {
    send_to_char("Your victim laughs at you cause you are {rDEAD{x.\n\r",ch);
    return;
  }

  if (IS_NPC(ch))
    return;

  

  if ((victim = get_char_room(ch,m_name)) == NULL )
    {
      send_to_char("They aren't here.\n\r",ch);
      return;
    }

  if (!IS_NPC(victim))
    {
      send_to_char("Why not just threaten them in person?\n\r",ch);
      return;
    }

  if (!IS_AWAKE(victim))
    {
      send_to_char("You can't get their attention.\n\r",ch);
      return;
    }

  if (IS_SET(victim->act,ACT_TRAIN)
      ||  IS_SET(victim->act,ACT_PRACTICE)
      ||  IS_SET(victim->act,ACT_IS_HEALER)
      ||  IS_SET(victim->act,ACT_BANKER)
      ||  IS_SET(victim->act,ACT_DEALER)
      ||  IS_SET(victim->act,ACT_IS_CHANGER)
      ||  (victim->pIndexData->pShop != NULL))
    {
      send_to_char("They laugh at you rather harshly.\n\r",ch);
      return;
    }
  vlevel = victim->level;
  chance /= 2;
  chance += (3*ch->level);
  chance -= 2*vlevel;

  if (IS_GOOD(victim))
    chance -= 4*vlevel;
  else if (IS_EVIL(victim))
    chance -= 2*vlevel;
  else
    chance -= 3*vlevel;

  vlevel += 8;

  if ((obj = get_obj_list(victim, i_name, victim->carrying)) == NULL)
    {
      send_to_char("They do not have that object.\n\r",ch);
      return;
    }
  if (!can_see_obj(ch,obj) )
    {
      send_to_char("They do not have that object.\n\r",ch);
      return;
    }

  if (vlevel > ch->level
      || number_percent() > chance)
    {
      check_improve(ch,gsn_demand,FALSE,2);
      mprintf(sizeof(buf1),buf1,"I don't think I'd give my belongings to one as weak as you!");
      mprintf(sizeof(buf2),buf2,"Help! I'm being attacked by %s!",victim->short_descr);
      do_function(victim,&do_say,buf1);
      do_function(ch, &do_yell,buf2);
      multi_hit(victim,ch,TYPE_UNDEFINED);
      return;
    }

  if (!can_see(victim,ch))
    {
      act("$N tells you, 'I can't give to those I can't see.'",ch,0,victim,TO_CHAR);
      return;
    }

  if (!can_see_obj(victim,obj))
    {
      act("$N tells you, 'I can't see such an object.'",ch,0,victim,TO_CHAR);
      return;
    }

  if (obj->level > ch->level + 8)
    {
      do_function(victim, &do_say, "That item is far to precious to hand over to scum like you!");
      mprintf(sizeof(buf1), buf1,"Help! I'm being attacked by %s!",victim->short_descr);
      do_function(ch,&do_yell,buf1);
      multi_hit(victim,ch,TYPE_UNDEFINED);
      return;
    }

  if (ch->move < obj->level)
    {
      act("$N tells you, 'Hah! You couldn't even get away if I chased you!'.",ch,0,victim,TO_CHAR);
      mprintf(sizeof(buf1), buf1,"Help! I'm being attacked by %s!",victim->short_descr);
      do_function(ch,&do_yell,buf1);
      multi_hit(victim,ch,TYPE_UNDEFINED);
      return;
    }

  if (ch->hit < (ch->max_hit*3/7))
    {
      do_function(victim, &do_say,"Hah! You look weak enough that even I could kill you!");
      mprintf(sizeof(buf1), buf1,"Help! I'm being attacked by %s!",victim->short_descr);
      do_function(ch,&do_yell,buf1);
      multi_hit(victim,ch,TYPE_UNDEFINED);
      return;
    }

  if (ch->mana < ch->level/2)
    {
      send_to_char("You don't have the mana.\n\r",ch);
      return;
    }

  if (ch->hit < ch->level/2)
    {
      send_to_char("You don't have the hit points.\n\r",ch);
      return;
    }

  if ((obj->wear_loc != WEAR_NONE) && IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
      act("$N tells you, 'I'm unable to release $p'.",ch,obj,victim,TO_CHAR);
      act("$N cowers back from you in fright.",ch,0,victim,TO_CHAR);
      act("$N cowers back from $n in fright.",ch,0,victim,TO_NOTVICT);
      return;
    }
  if (IS_SET(obj->extra_flags,ITEM_NODROP) )
    {
      act("$N tells you, 'I'm unable to release $p'.",ch,obj,victim,TO_CHAR);
      act("$N cowers back from you in fright.",ch,0,victim,TO_CHAR);
      act("$N cowers back from $n in fright.",ch,0,victim,TO_NOTVICT);
      return;
    }

  if ( ch->carry_weight + get_obj_weight(obj)  > can_carry_w(ch) )
    {
      act("$N tells you, 'You can't carry the weight $n.'",ch,0,victim,TO_CHAR);
      return;
    }
  if ( ch->carry_number + 1 > can_carry_n(ch) )
    {
      act("$N tells you, 'You can't carry that many items $n.'",ch,0,victim,TO_CHAR);
      return;
    }

  act("$N caves in to the bully tactics of $n.",ch,0,victim,TO_NOTVICT);
  act("$N shivers in fright and caves in to your bully tactics.",ch,0,victim,TO_CHAR);

  if (obj->wear_loc != WEAR_NONE)
    {
      act("$n stops using $p.",victim,obj,NULL,TO_ROOM);
      act("You stop using $p.",victim,obj,NULL,TO_CHAR);
    }
  act("$N gives $p to $n.",ch,obj,victim,TO_NOTVICT);
  act("$N gives you $p.",ch,obj,victim,TO_CHAR);
  check_improve(ch,gsn_demand,TRUE,2);
  WAIT_STATE(ch,24);
  obj_from_char(obj);
  obj_to_char(obj,ch);
  ch->mana -= ch->level/2;
  ch->hit -= ch->level/2;
  return;
}

void do_chomp( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim = NULL;
  OBJ_DATA *corpse;
  AFFECT_DATA af;
  int chance;

  one_argument(argument,arg); 

  if (( (chance = get_skill(ch,gsn_bite)) < 2)
  ||  ((IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BITE)))
  ||  (!IS_NPC(ch) && (ch->level < skill_table[gsn_bite].skill_level[ch->gameclass])))
  {
    send_to_char("Bite them? Oh please!!\n\r",ch);
    return;
  }

  if (!IS_NPC(ch)
  && ((corpse = get_obj_here(ch,arg)) != NULL)
  &&  ( (corpse->item_type == ITEM_CORPSE_NPC)
  ||    (corpse->item_type == ITEM_CORPSE_PC) ) )
  { // feed from a recent corpse
    if (corpse->value[5] <= 0)
    {
      send_to_char("It appears to be drained of blood.\n\r",ch);
      return;
    }
    else
    {
      if ( ( ch->pcdata->condition[COND_THIRST] < 48 )
      ||   ( ch->pcdata->condition[COND_HUNGER] < 48 ) )
      {
        corpse->value[5]--;
        gain_condition( ch, COND_THIRST, 5 );
        gain_condition( ch, COND_HUNGER, 5 );
        send_to_char("Your thirst for blood is slightly quenched.\n\r",ch);
        return;
      }
      else
      {
        send_to_char("You do not need to feed.\n\r",ch);
        return;
      }
    }
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
 
  if (victim == ch)
  {
    send_to_char("How the hell are you going to do that?\n\r",ch);
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

  if (IS_SET(victim->in_room->room_flags,ROOM_ARENA))
  {
    send_to_char("Charm Failed, Immortals Prevent this here.\n\r",ch);
    return;
  }

  if ( IS_SET(victim->act2, ACT2_BLOODLESS) )
  {
    send_to_char( "They have no blood in them.\n\r", ch );
    return;
  }

  if (!IS_AWAKE(victim)) {
      if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   (IS_SET(victim->act, ACT_AGGRESSIVE) && IS_NPC(victim))
    ||   saves_spell( ch->level, victim,DAM_CHARM)
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   ch->level < victim->level)
      {
      send_to_char("Your bite charm fails.\n\r",ch);  
      victim->position = POS_STANDING;
      return;
      }

      if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
      {
       send_to_char(
           "The mayor does not allow charming in the city limits.\n\r",ch);
       return;
      } 

      do_function(victim, &do_stand,"");

      if ( victim->master )
      stop_follower( victim );

      add_follower( victim, ch );

      victim->leader = ch;

      af.where     = TO_AFFECTS;
      af.type      = gsn_charm_person;
      af.level     = ch->level;
      af.duration  = number_fuzzy( ch->level / 4 );
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = AFF_CHARM;
      affect_to_char( victim, &af );
      act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
      victim->position = POS_STANDING;
      if ( ch != victim )
    act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
      return;
  }
  
  chance = get_skill(ch,gsn_bite);
  /* modifiers */

  /* dexterity */
  chance += get_curr_stat(ch,STAT_DEX);
  
  chance -= 2 * get_curr_stat(victim,STAT_DEX);
  /* speed  */
  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 25;

  /* level */
  chance += (ch->level - victim->level);
  if (chance <= 0) chance = 1;

  WAIT_STATE(ch,skill_table[gsn_bite].beats);
  if (number_percent() < chance)
    {
      printf_to_char( ch,
        "You take %s into a deadly embrace, {wf{Wan{wgs{x sinking deep in %s skin.\n\r",
            victim->short_descr,
            victim->sex == 2 ? "her" : "his" );
      act("$N takes $n into a deadly embrace, $S {wf{Wan{wgs{x sinking deep into $s flesh.",
        victim,NULL,ch,TO_NOTVICT);
      printf_to_char( victim,
        "You are taken into %s deadly embrace, %s {wf{Wan{wgs{x sinking deep in your flesh.\n\r",
            victim->short_descr,
            ch->sex == 2 ? "her" : "his" ); 
      damage(ch,victim,dice(ch->level,5),gsn_bite,DAM_PIERCE,TRUE, FALSE);
      check_improve(ch,gsn_bite,TRUE,2);
      ch->hit += victim->level/6;
      if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] < 40 )
      {
          gain_condition( ch, COND_THIRST, 5 );
    gain_condition( ch, COND_HUNGER, 5 );
    send_to_char("Your thirst for blood is slightly quenched.\n\r",ch);
      }
      else 
                send_to_char("Your thirst for blood is already quenched for now.\n\r",ch);
      return;
    }
    else 
    {
      damage(ch,victim,0,gsn_bite,DAM_PIERCE,TRUE, FALSE);
      send_to_char("You fail to bite your victim.\n\r",ch);
      check_improve( ch, gsn_bite, FALSE, 2 );
    }
}

void do_ironwill( CHAR_DATA *ch, char *argument)
{
  int chance, hp_percent;

  if (IS_GHOST(ch)) {
    send_to_char("Trying to ironwill is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ((chance = get_skill(ch,gsn_ironwill)) == 0)
    {
      send_to_char("You turn red in the face, but nothing happens.\n\r",ch);
      return;
    }

  if (is_sanc_spelled(ch))
  {
      send_to_char("You already have set your will to control your body.\n\r",ch);
      return;
    }

  //Reduced to 30 mana Aarchane - 6/20/09
  if (ch->mana < 30)
  {
    send_to_char("You can't get up enough energy.\n\r",ch);
    return;
  }

  if ( ch->move < 50 )
  {
    send_to_char( "You are too tired to do this.\n\r", ch );
    return;
  }

  //modifiers
  // fighting
  if (ch->position == POS_FIGHTING)
    chance += 10;

  // damage -- below 50% of hp helps, above hurts
  hp_percent = 100 * ch->hit/GET_HP(ch);
  chance += 25 - hp_percent/2;

  if (number_percent() < chance)
  {
    AFFECT_DATA af;

    WAIT_STATE(ch,PULSE_VIOLENCE);
    ch->move -= 50;
    ch->mana -= 30;

    //heal a little damage
    ch->hit += ch->level * 2;
    ch->hit = UMIN(ch->hit,GET_HP(ch));

    send_to_char("Your pulse races as you bring yourself to control your body with an {yiron{Dwill{x.\n\r",ch);
    act("$n gains control of $s body, maintaining an {yiron{Dwill{x.",ch,NULL,NULL,TO_ROOM);
    check_improve(ch,gsn_ironwill,TRUE,2);

    af.where    = TO_SPELL_AFFECTS;
    af.type    = gsn_ironwill;
    af.level    = ch->level;
    af.duration  = number_fuzzy(ch->level / 8);
    af.modifier  = 0;
    af.bitvector   = SAFF_IRONWILL;
    af.location  = APPLY_NONE;
    affect_to_char(ch,&af);
  }
  else
  {
    WAIT_STATE(ch,3 * PULSE_VIOLENCE);
    ch->mana -= 25;
    ch->move /= 2;
    send_to_char("Your pulse speeds up, but you cannot gain control of your body.\n\r",ch);
    check_improve(ch,gsn_ironwill,FALSE,2);
  }
}

/* RT added to cure plague */
void do_rub(CHAR_DATA *ch, char *argument)
{
  int chance, found = FALSE;
  int fb=skill_lookup("fire breath");

  if (IS_GHOST(ch)) {
    send_to_char("Trying to rub is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }
  
  if ( ( chance = get_skill( ch, gsn_rub ) ) <= 0 )
  {
      send_to_char("You scratch at your eyes. People wonder what for.\n\r",ch);
      return;
  }
  
  if (( !is_affected( ch, gsn_dirt ) ) && (!is_affected(ch, fb)))
    {
      check_improve(ch,gsn_rub,FALSE,4);
      send_to_char("You don't have smoke or dirt in your eyes.\n\r",ch);
      return;
    }

  if (is_affected(ch, gsn_dirt))
  {
    if (number_percent() < chance)
    {
      send_to_char("You rub the dirt from your eyes.\n\r",ch);
      affect_strip(ch, gsn_dirt);
      check_improve(ch,gsn_rub,TRUE,2);
      found = TRUE;
    }
  }
  if (is_affected(ch, fb))
  {
    if (number_percent() < chance)
    {
      send_to_char("You rub the {Dsm{wo{Dke{x from your eyes.\n\r",ch);
      affect_strip(ch, fb);
      check_improve(ch,gsn_rub,TRUE,2);
      found = TRUE;
    }
  }

  if (!found)
    send_to_char("You rub for nothing.\n\r",ch);

  WAIT_STATE(ch,skill_table[gsn_rub].beats);
}

void do_sharpen(CHAR_DATA *ch, char *argument)
{

  OBJ_DATA *obj, *stone;
  char weapon[MAX_INPUT_LENGTH];
  AFFECT_DATA af;

  if (IS_GHOST(ch)) {
    send_to_char("Sharpening a weapon is useless as you are still {rDEAD{x\n\r", ch);
    return;
  }

  if (argument[0] == '\0') {
    send_to_char("What weapon do you want to forge?\n\r", ch);
    return;
  }

  argument = one_argument(argument, weapon);

  if ( (obj = get_obj_carry( ch, weapon, ch) ) == NULL ) {
    send_to_char("You don't have that.\n\r", ch);
    return;
  }

  if (obj->item_type != ITEM_WEAPON) {
    send_to_char("You can only sharpen bladed weapons.\n\r", ch);
    return;
  }

  if (  obj->value[0] == WEAPON_MACE
     || obj->value[0] == WEAPON_WHIP
     || obj->value[0] == WEAPON_EXOTIC) {
    send_to_char("You can only sharpen bladed weapons.\n\r", ch);
    return;
  }

  if (IS_WEAPON_STAT(obj, WEAPON_SHARP)) {
    send_to_char("This weapon is already sharp.\n\r", ch);
    return;
  }

  stone = get_eq_char(ch, WEAR_HOLD);
  if (stone == NULL || stone->item_type != ITEM_WHETSTONE)
    {
      send_to_char("What do you intend to sharpen your weapon with?  Your sleeve?\n\r", ch);
      return;
    }

  if ( number_percent() > get_skill(ch, gsn_sharpen)
       || ch->level < obj->level)
    {
      send_to_char("You failed and have ruined your whetstone.\n\r", ch);
      extract_obj(stone);
      if ( number_percent() > 95 ) {  /* 5% chance you ruin your weapon */
        send_to_char("In fact, you messed up so badly, you've also ruined your weapon.\n\r", ch);
        extract_obj(obj);
      }
      check_improve(ch, gsn_sharpen, FALSE, 1);
      return;
    } else {
      act("You skillfully sharpen your $p.", ch, obj, NULL, TO_CHAR);
      act("In the process, you've rendered your $p useless.", ch, stone, NULL, TO_CHAR);
      act("$n skillfully sharpens $s $p.", ch, obj, NULL, TO_ROOM);
      extract_obj(stone);
      check_improve(ch, gsn_sharpen, TRUE, 1);
    }

  if (number_percent() > 97 ) { /* 3% chance to sharpen to vorpal */
    send_to_char("You did such a good job, you've made your weapon {Wvorpal{x.\n\r", ch);
    af.where    = TO_WEAPON;
    af.type     = skill_lookup("sharpen");
    af.level    = ch->level;
    af.duration = 30;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = WEAPON_VORPAL;
  } else {
    af.where    = TO_WEAPON;
    af.type     = skill_lookup("sharpen");
    af.level    = ch->level;
    af.duration = 30;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = WEAPON_SHARP;
  }
  affect_to_obj(obj, &af);

  return;
}

void do_shelter(CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *shelter;
  char buf[MAX_STRING_LENGTH];
  int capacity;

  if ((get_skill(ch, gsn_shelter) == 0)
  || (!IS_NPC(ch) && ch->level < skill_table[gsn_shelter].skill_level[ch->gameclass]))
  {
     send_to_char("You don't have a clue how to do that.\n\r", ch);
     return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("Building a shelter is pointless as you are {rDEAD{x.\n\r", ch);
    return;
  }

  if ( number_range(0,100) > get_skill(ch,gsn_shelter) )
  {
    send_to_char("You fail to create a shelter.\n\r",ch);
    check_improve(ch, gsn_shelter, FALSE, 2);
    WAIT_STATE( ch, 3 * PULSE_VIOLENCE );
    return;
  }

  if (IS_SET(ch->in_room->room_flags, ROOM_INDOORS))
  {
    send_to_char("You can only erect a shelter outdoors.\n\r", ch);
    return;
  }

  shelter = get_obj_list(ch, "shelter", ch->in_room->contents);
  if (shelter != NULL)
  {
    printf_to_char( ch, "You inspect the shelter, noting it will last %d more hour%s.\n\r",
      shelter->timer,
      shelter->timer > 1 ? "s" : "" );
    return;
  }

  if (argument[0] == '\0') {
    capacity = 1;
  } else {
    if (!is_number(argument) ) {
      send_to_char("You want the shelter to be how big?\n\r", ch);
      return;
    }

    capacity = atoi(argument);

    if (capacity < 0) {
      send_to_char("The number of people the shelter can hold must be greater than zero.\n\r", ch);
      return;
    }

    if ( (capacity > (ch->level / 10) ) && ch->level < LEVEL_IMMORTAL ) {
      printf_to_char(ch, "The largest shelter you can build can only hold %d people.\n\r",
        ch->level / 10);
      return;
    }
  }

  shelter = create_object( get_obj_index(OBJ_VNUM_SHELTER), 0);
  mprintf(sizeof(buf), buf, "$n erects a shelter.");
  act_new( buf, ch, shelter, NULL, TO_ROOM, POS_RESTING);
  send_to_char("You raise a temporary shelter.\n\r", ch);
  check_improve(ch, gsn_shelter, TRUE, 2);

  free_string(shelter->name);
  shelter->name = str_dup("shelter tent", shelter->name);
  free_string(shelter->short_descr);
  shelter->short_descr = str_dup("a temporary shelter", shelter->short_descr);
  free_string(shelter->description);
  shelter->description = str_dup("A temporary shelter has been raised here.", shelter->description);
  shelter->value[0] = capacity;
  shelter->value[1] = 99999;
  shelter->value[2] = 2340;
  shelter->value[3] = 125;
  shelter->value[4] = 125;
  shelter->timer = ch->level / 10;
  shelter->level = ch->level;
  REMOVE_BIT(shelter->extra_flags, ITEM_NOPURGE);
  obj_to_room(shelter, ch->in_room);
  WAIT_STATE(ch, PULSE_VIOLENCE);

  return;
}

/*
 * Function do_leave being added for a new shelter system going to a "shelter room" instead of making
 * a object type furniture.  Changed so those w/ shelter can use bedrolls inside their shelter if they
 * want to.
 */
//void do_leave( CHAR_DATA *ch, char *argument )
//{
  

void do_focus( CHAR_DATA *ch, char *argument )
{

  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int chance;

  one_argument(argument, arg);

  if ( ((chance = get_skill(ch, gsn_focus)) == 0)
       || (!IS_NPC(ch) && ch->level < skill_table[gsn_focus].skill_level[ch->gameclass])) {
     send_to_char("You're not experienced enough to try that.\n\r", ch);
     return;
  }

  if (ch->fighting == NULL) {
    send_to_char("You aren't fighting anyone.\n\r", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You are blind.  You can't see anyone to focus on.\n\r", ch);
    return;
  }

  if (arg[0] == '\0') {
    send_to_char("On whom do you want to focus?\n\r", ch);
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL) {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Ha ha, very funny.\n\r", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
    act("But $N is just so darned friendly.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if (ch->fighting == victim) {
    act("You are already fighting $N.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if (is_safe(ch, victim, TRUE))
        return;
  
  if (check_killsteal(ch, victim)) 
    return;

  chance -= 10;  /* No more than a 90% sucess rate */
  if (chance < 1)
    chance = 1;
  
  if (number_percent() < chance) {
    act("You refocus your attack on $N!", ch, NULL, victim, TO_CHAR);
    act("$n refocuses $s attack on you!", ch, NULL, victim, TO_VICT);
    act("$n refocuses $s attack on $N.",  ch, NULL, victim, TO_NOTVICT);
    /*damage(ch, victim, 0, gsn_focus, DAM_MENTAL, FALSE, FALSE);*/
    check_improve(ch, gsn_focus, TRUE, 3);
    stop_fighting(ch, FALSE);
    set_fighting(ch, victim);
  } else {
    send_to_char("You are unable to refocus your attack.\n\r", ch);
    check_improve(ch, gsn_focus, FALSE, 2);
  }
}

#ifdef INQUISITOR
void do_crusade( CHAR_DATA *ch, char *argument)
{
  int chance;
  int hp_percent = 0;
  int mp_percent = 0;
  int crusade_percent = 0;
  AFFECT_DATA af;

  if (IS_GHOST(ch))
  {
    send_to_char("Crusading is useless as you are still {rDEAD{x.\n\r", ch);
    return;
  }

  if ( ( chance = get_skill(ch, gsn_crusade) ) == 0
  ||  IS_NPC(ch)
  ||  ch->level < skill_table[gsn_crusade].skill_level[ch->gameclass] )
  {
    send_to_char("You don't even know where to begin in leading a crusade.\n\r", ch);
    return;
  }

  if (is_affected(ch, gsn_crusade))
  {
    send_to_char("You are already on a crusade.\n\r",ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_CALM))
  {
    send_to_char("Nah, you don't feel up to it right now.\n\r", ch);
    return;
  }

  if (ch->fighting)
  {
    send_to_char("You are too busy fighting to plan a crusade.\n\r", ch);
    return;
  }

  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
  {
    send_to_char("You aren't noble enough for such a worthy cause.\n\r", ch);
    return;
  }

  if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
  {
    send_to_char("You aren't wicked enough for such a vile cause.\n\r", ch);
    return;
  }

  mp_percent = ( (ch->mana *100) / GET_MANA(ch) );
  hp_percent = ( (ch->hit  *100) / GET_HP(ch) );
  crusade_percent = ( (hp_percent + mp_percent) / 2 );

  ch->mana -= ch->max_mana / 5;

  if (ch->mana < 0)
    ch->mana = 0;

  if (number_percent() < chance) 
  {
    //Hit Points
    af.where     = TO_AFFECTS;
    af.type      = gsn_crusade;
    af.level     = ch->level;
    af.duration  = ( UMAX(ch->level / 2, 10) ) *crusade_percent / 100;
    af.location  = APPLY_HIT;
    af.modifier  = ( UMAX(ch->level * 2, 25) ) *crusade_percent / 100;
    af.bitvector = 0;
    affect_to_char(ch, &af);

    //Armor Class
    af.modifier = ( UMIN( -(ch->level * 2), -10) ) *crusade_percent / 100;
    af.location  = APPLY_AC;
    affect_to_char(ch, &af);

    //Saves
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 0 - (ch->level / 8) + number_range( 0, 3);
    affect_to_char( ch, &af );

    ch->hit += ( UMAX(ch->level * 2, 25) ) *crusade_percent / 100;
    if ( ch->hit > GET_HP(ch) )
      ch->hit = GET_HP(ch);
      
    if ( IS_CLASS_INQUISITOR( ch ) )
    {
      act("You launch a {yh{Wo{Yl{yy{x crusade!", ch, NULL, NULL, TO_CHAR);
      act("$n launches a {yh{Wo{Yl{yy{x crusade!", ch, NULL, NULL, TO_ROOM);
    }

    if ( IS_CLASS_OCCULTIST( ch ) )
    {
      act("You launch an {Dun{rh{Do{rly{x crusade!", ch, NULL, NULL, TO_CHAR);
      act("$n just launched an {Dun{rh{Do{rly{x crusade!", ch, NULL, NULL, TO_ROOM);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE);
    check_improve(ch, gsn_crusade, TRUE, 2);
  }
  else
  {
    act("You failed.  Perhaps your cause isn't worthy enough.", ch, NULL, NULL, TO_CHAR);
    act("$n attempts to launch a crusade, but fails.", ch, NULL, NULL, TO_ROOM);
    WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
    check_improve(ch, gsn_crusade, FALSE, 1);
  }
}

bool guided_strike(CHAR_DATA *ch, CHAR_DATA *victim)
{
  OBJ_DATA *obj;

  obj = get_eq_char(ch, WEAR_WIELD);  /*Doesn't account for dual weapon.*/

  if (    obj == NULL
       || (get_skill(ch, gsn_guided_strike) < 1)
       || (number_range(0,100) > get_skill(ch, gsn_guided_strike)))
    return FALSE;
 
  if ( (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch) ) ||
       (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch) ) )
    return FALSE;

  if (number_range(0,100) > get_weapon_skill(ch, get_weapon_sn(ch)) / 10)
    return FALSE;

  /* If the character has gotten to this point, the weapon has been guided. */

  if (IS_CLASS_OCCULTIST(ch))
    {
      act_spam("Your atrocious acts result in a {DGUIDED {rSTRIKE{x!", ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
      act_spam("$n lands a {DGUIDED {rSTRIKE{x against you!", ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_OEFFECTS );
      act_spam("$n lands a {DGUIDED {rSTRIKE{x against $N!", ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS );
//      act("our holy conviction results in a {WGUIDED {cSTRIKE{x!", ch, NULL, victim, TO_CHAR);
//      act("$n lands a {DGUIDED {rSTRIKE{x against you!", ch, NULL, victim, TO_VICT);
//      act("$n lands a {DGUIDED {rSTRIKE{x against $N!", ch, NULL, victim, TO_NOTVICT);
    } else {
      act_spam("Your holy conviction results in a {WGUIDED {cSTRIKE{x!", ch, NULL, victim, TO_CHAR, POS_RESTING, NOSPAM_SEFFECTS );
      act_spam("$n lands a {DGUIDED {rSTRIKE{x against you!", ch, NULL, victim, TO_VICT, POS_RESTING, NOSPAM_OEFFECTS );
      act_spam("$n lands a {DGUIDED {rSTRIKE{x against $N!", ch, NULL, victim, TO_NOTVICT, POS_RESTING, NOSPAM_OEFFECTS );
//      act("Your holy conviction results in a {WGUIDED {cSTRIKE{x!", ch, NULL, victim, TO_CHAR);
//      act("$n lands a {WGUIDED {cSTRIKE{x against you!", ch, NULL, victim, TO_VICT);
//      act("$n lands a {WGUIDED {cSTRIKE{x against $N!", ch, NULL, victim, TO_NOTVICT);
    }
  check_improve(ch, gsn_guided_strike, TRUE, 6);
  return TRUE;
}

void do_layhands(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  AFFECT_DATA af;

  if (IS_GHOST(ch))
    {
      send_to_char("You cannot lay hands because you are still {rDEAD{x!\n\r", ch);
      return;
    }

  if (IS_CLASS_INQUISITOR(ch) && !IS_GOOD(ch))
    {
      send_to_char("You are not righteous enough to be granted your holy powers.\n\r", ch);
      return;
    }
  if (IS_CLASS_OCCULTIST(ch) && !IS_EVIL(ch))
    {
      send_to_char("You are not wicked enough to be granted your dark powers.\n\r", ch);
      return;
    }

  if ((victim = get_char_room(ch, argument)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (victim == ch)
  {
    send_to_char("You cannot lay hands upon yourself.\n\r", ch);
    return;
  }

  if ((IS_CLASS_INQUISITOR(ch)) && ( ch->mana < 40 ))
  {
    send_to_char("You do not have sufficient mana to attempt this now.\n\r", ch);
    return;
  }
  else if ( ch->mana < 100 ) // Occultist's costs 100
  {
    send_to_char("You do not have sufficient mana to attempt this now.\n\r", ch);
    return;
  }

  if (IS_CLASS_INQUISITOR(ch))
  {
    act("You lay your hands on $N and they begin to glow a soft shade of {yyellow{x.", ch, NULL, victim, TO_CHAR);
    act("$n lays $s hands on $N and they begin to glow a soft shade of {yyellow{x.", ch, NULL, victim, TO_NOTVICT);
    if (!is_affected(victim, skill_lookup("sleep")))
      act("$n lays $s hands on you and they begin to glow a soft shade of {yyellow{x.", ch, NULL, victim, TO_VICT);

    ch->mana -= 40;

    if (is_affected(victim, gsn_plague))
    {
      if (check_dispel(ch->level, victim, gsn_plague))
      {
        act("$n looks relieved as $s {Ds{ro{gres{x vanish.", victim, NULL, NULL, TO_ROOM);
        send_to_char("Your {Ds{ro{gres{x heal and vanish.", victim);
      }
      else
        act("You are unable to cure $N's disease.", ch, NULL, victim, TO_CHAR);
    }
    else
    {
      act("$N is not diseased.", ch, NULL, victim, TO_CHAR);
    } // if plagued (else)

    if (is_affected(victim, gsn_poison))
    {
      if (check_dispel(ch->level, victim, gsn_poison))
      {
        send_to_char("A warm feeling runs through your body.\n\r", victim);
        act("$n looks much better.", victim, NULL, NULL, TO_ROOM);
      }
      else
      {
        act("You are unable to rid $N's body of poison.", ch, NULL, victim, TO_CHAR);
      }
    }
    else
    {
      act("$N is not poisoned.", ch, NULL, victim, TO_CHAR);
    } // if poisoned (else)

    check_improve(ch, gsn_layhands, TRUE, 3);
    WAIT_STATE(ch, 1.5 * PULSE_VIOLENCE);
  } // if Inquisitor
  else
  {
    act("You lay your hands on $N and they begin to glow as {Dblack{x as a moonless night.", ch, NULL, victim, TO_CHAR);
    act("$n lays $s hands on $N and they begin to glow as {Dblack{x as a moonless night.", ch, NULL, victim, TO_NOTVICT);
    if(!is_affected(victim, skill_lookup("sleep")))
      act("$n lays $s hands on you and they begin to glow as {Dblack{x as a moonless night.", ch, NULL, victim, TO_VICT);

    // Taeloch: Layhands cost half your TOTAL mana, yet did
    // less than the pestilence spell that costs 90 mana.
    // Granted, layhands is level 3 and pestilence is level
    // 65, so the cost must be significant, but appropriate.
    // ch->mana -= ch->max_mana / 2;
    ch->mana -= 100;

    if (is_affected(victim, gsn_plague))
    {
      act("$N's body is already riddled with disease.", ch, NULL, victim, TO_CHAR);
      damage(ch, victim, 1, gsn_plague, DAM_DISEASE, FALSE, TRUE);
    } // already plagued
    else if (saves_spell(ch->level, victim, DAM_DISEASE) || (IS_NPC(victim) && IS_SET(victim->act, ACT_UNDEAD)))
    {
      act("You are unable to infect $N with disease.", ch, NULL, victim, TO_CHAR);
      damage(ch, victim, 1, gsn_plague, DAM_DISEASE, FALSE, TRUE);
    } // disease saving throw or undead
    else
    {
      af.where     = TO_AFFECTS;
      af.type      = gsn_plague;
      af.level     = ch->level * 3/4;
      af.duration  = ch->level;
      af.location  = APPLY_STR;
      af.modifier  = -5;
      af.bitvector = AFF_PLAGUE;
      affect_join (victim, &af);

      send_to_char("You scream in agony as {rs{Do{gres{x erupt from your skin.\n\r", victim);
      act("$n screams in agony as {rs{Do{gres{x erupt from $s skin.", victim, NULL, NULL, TO_ROOM);
      damage(ch, victim, 1, gsn_plague, DAM_DISEASE, TRUE, TRUE);
    } // successful plague

    if (is_affected(victim, gsn_poison))
    {
      act("$N already has poison coursing through $s veins.", ch, NULL, victim, TO_CHAR);
      damage(ch, victim, 1, gsn_poison, DAM_POISON, FALSE, TRUE);
    } // already poisoned
    else if (saves_spell( ch->level, victim, DAM_POISON))
    {
      act("You are unable to poison $N.", ch, NULL, victim, TO_CHAR);
      damage(ch, victim, 1, gsn_poison, DAM_POISON, FALSE, TRUE);
    } // poison saving throw
    else
    {
      af.where     = TO_AFFECTS;
      af.type      = gsn_poison;
      af.level     = ch->level;
      af.duration  = ch->level;
      af.location  = APPLY_STR;
      af.modifier  = -2;
      af.bitvector = AFF_POISON;
      affect_join(victim, &af);

      send_to_char("You feel very {cs{gick{x.\n\r", victim);
      act("$n looks very {ci{gll{x.", victim, NULL, NULL, TO_ROOM);
      damage(ch, victim, 1, gsn_poison, DAM_POISON, TRUE, TRUE);
    } // successful poison

    check_improve(ch, gsn_layhands, TRUE, 3);
    WAIT_STATE(ch, 1.5 * PULSE_VIOLENCE);
  } // if Inquisitor (else, ergo Occultist)
} // layhands()

#endif
