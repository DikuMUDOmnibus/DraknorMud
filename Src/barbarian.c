/*
 * All code in this file belongs solely to the Lands of Draknor. Use without
 * permission of the authors is prohibited.
 */

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
#include <math.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "interp.h"

//Local Declarations for Warcry functions
bool warcry_hardening( CHAR_DATA *ch );  //res weapon
bool warcry_rage( CHAR_DATA *ch );       //hit/dam
bool warcry_vigor( CHAR_DATA *ch );      //haste/move
bool warcry_guarding( CHAR_DATA *ch );   //sanc
bool warcry_aggression( CHAR_DATA *ch ); //ACT_AGGRESSIVE
bool warcry_shout( CHAR_DATA *ch );      //make victim(s) flee
bool warcry_control( CHAR_DATA *ch );

/*
 * Warcry: Control
 * Clears all warcries from a target.
 */

bool warcry_control( CHAR_DATA *ch )
{

    if ( IS_NPC( ch ) ) return FALSE;

    if ( is_affected( ch, gsn_warcry_hardening ) )
        affect_strip( ch, gsn_warcry_hardening );

    if ( is_affected( ch, gsn_warcry_guarding ) )
        affect_strip( ch, gsn_warcry_guarding );

    if ( is_affected( ch, gsn_warcry_rage ) )
        affect_strip( ch, gsn_warcry_rage );

    if ( is_affected( ch, gsn_warcry_vigor ) )
        affect_strip( ch, gsn_warcry_vigor );

    send_to_char( "With effort you come to control yourself.\n\r", ch );
    WAIT_STATE(ch, PULSE_VIOLENCE); 
    ch->move /= 2;

    return TRUE;
}

/*
 * Warcry: Hardening
 * Affect: RES_WEAPON for a small amount of time
 *   Cost: 40 mana
 *  Level: 33
 */
bool warcry_hardening( CHAR_DATA *ch )
{

    AFFECT_DATA af;
    int chance = 0;

    if ( ch->fighting )
    {
        send_to_char( "No way!  You are still fighting!.\n\r", ch );
        return FALSE;
    }

    if ( ( chance = get_skill( ch, gsn_warcry_hardening ) ) == 0
    || ( ch->level < skill_table[gsn_warcry_hardening].skill_level[ch->gameclass]
    && !is_racial_skill( ch, gsn_warcry_hardening ) ) )
    {
        send_to_char("Try as you might, that warcry is beyond your ability.\n\r",ch);
        return FALSE;
    }

    if ( is_affected( ch, gsn_warcry_hardening ) )
    {
        send_to_char( "You already feel hardened.\n\r", ch );
        return FALSE;
    }

    if ( number_percent() <= chance )
    {
        if ( ch->move < 40 )
        {
            send_to_char( "You don't have the stamina for that.\n\r", ch );
            return FALSE;
        }
        
        af.where     = TO_RESIST;
        af.type      = gsn_warcry_hardening;
        af.level     = ch->level;
        af.duration  = ch->level/8;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = RES_WEAPON;
        affect_to_char( ch, &af );

        ch->move -= 40;

        act( "You cry out to $g, and your muscles harden.",
          ch, NULL, NULL, TO_CHAR);
        act( "$n calls out to $g, and is rewarded with hardened skin.",
          ch, NULL, NULL, TO_ROOM );

        WAIT_STATE(ch, PULSE_VIOLENCE);

        return TRUE;
    }
    else
    {
        act( "Your battle cry fails to appease $g.",
          ch, NULL, NULL, TO_CHAR);
        act( "$n fails to achieve the attention of $g.",
            ch, NULL, NULL, TO_ROOM );

        WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

        return FALSE;
    }

    return FALSE;
}

/*
 * Warcry: Rage
 * Affect: Raises hitroll/damroll
 *   Cost: 40 moves
 *  Level: 13
 */
bool warcry_rage( CHAR_DATA *ch )
{
    AFFECT_DATA af;
    int chance = 0;

    if ( ch->fighting )
    {
        send_to_char( "No way!  You are still fighting!.\n\r", ch );
        return FALSE;
    }

    if ( ( chance = get_skill( ch, gsn_warcry_rage ) ) == 0
    || ( ch->level < skill_table[gsn_warcry_rage].skill_level[ch->gameclass]
    && !is_racial_skill( ch, gsn_warcry_rage ) ) )
    {
      send_to_char("Try as you might, that warcry is beyond your ability.\n\r",ch);
      return FALSE;
    }

    if ( is_affected( ch, gsn_warcry_rage ) )
    {
        send_to_char( "You are already enraged.\n\r", ch );
        return FALSE;
    }

    if ( number_percent() <= chance )
    {

        if ( ch->move < 40 )
        {
            send_to_char( "You don't have the stamina for that.\n\r", ch );
            return FALSE;
        }

        af.where     = TO_AFFECTS;
        af.type      = gsn_warcry_rage;
        af.level     = ch->level;
        af.duration  = ch->level / 4;
        af.modifier  = UMAX( 1, ch->level / 5 );
        af.bitvector = 0;

        af.location   = APPLY_HITROLL;
        affect_to_char(ch,&af);

        af.location   = APPLY_DAMROLL;
        affect_to_char(ch,&af);

        ch->move -= 40;

        act( "You cry out to $g, and are filled with powerful rage.",
          ch, NULL, NULL, TO_CHAR);
        act( "$n calls out to $g, and is rewarded with a powerful rage.",
             ch, NULL, NULL, TO_ROOM );

        WAIT_STATE(ch, PULSE_VIOLENCE);

        return TRUE;
    }
    else
    {
        act( "Your battle cry fails to appease $g.",
          ch, NULL, NULL, TO_CHAR);
        act( "$n fails to achieve the attention of $g.",
            ch, NULL, NULL, TO_ROOM );

        WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

        return FALSE;
    }

    return FALSE;
}

/*
 * Warcry: Vigor
 * Affect: Adds haste and adds movement
 *   Cost: 50 mana
 *  Level: 13
 */
bool warcry_vigor( CHAR_DATA *ch )
{
    AFFECT_DATA af;
    int chance = 0;

    if ( ch->fighting )
    {
        send_to_char( "No way!  You are still fighting!.\n\r", ch );
        return FALSE;
    }

    if ( ( chance = get_skill( ch, gsn_warcry_vigor ) ) == 0
    || ( ch->level < skill_table[gsn_warcry_vigor].skill_level[ch->gameclass]
    && !is_racial_skill( ch, gsn_warcry_vigor ) ) )
    {
      send_to_char("Try as you might, that warcry is beyond your ability.\n\r",ch);
      return FALSE;
    }

    if ( is_affected( ch, gsn_warcry_vigor ) )
    {
        send_to_char( "You are already in a state of vigor.\n\r", ch );
        return FALSE;
    }

    if ( number_percent() <= chance )
    {

        if ( ch->mana < 50 )
        {
            send_to_char( "You don't have the energy for that.\n\r", ch );
            return FALSE;
        }

        af.where     = TO_AFFECTS;
        af.type      = gsn_warcry_vigor;
        af.level     = ch->level;
        af.duration  = ch->level / 4;
        af.location  = APPLY_MOVE;
        af.modifier  = UMAX( 50, ch->level * 2 );
        af.bitvector = 0;
        affect_to_char( ch, &af );

        if (  !IS_AFFECTED( ch, AFF_HASTE ) )
        {
            af.where     = TO_AFFECTS;
            af.location  = APPLY_NONE;
            af.modifier  = 0;
            af.bitvector = AFF_HASTE;
            affect_to_char( ch, &af );
//            send_to_char( "You begin to move faster.\n\r", ch );
        }

        ch->mana -= 50;

        act( "You cry out to $g, and are filled with a feeling of quickness.",
          ch, NULL, NULL, TO_CHAR);
        act( "$n calls out to $g, and is rewarded with a quickened step.",
             ch, NULL, NULL, TO_ROOM );

        WAIT_STATE(ch, PULSE_VIOLENCE);

        return TRUE;
    }
    else
    {
        act( "Your battle cry fails to appease $g.",
          ch, NULL, NULL, TO_CHAR);
        act( "$n fails to achieve the attention of $g.",
            ch, NULL, NULL, TO_ROOM );

        WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

        return FALSE;
    }

    return FALSE;
}

/*
 * Warcry: Guarding
 * Affect: Reduces damage by 1/2 (Sanctuary)
 *   Cost: 40 mana 60 move
 *  Level: 35
 */
bool warcry_guarding( CHAR_DATA *ch )
{
    AFFECT_DATA af;
    int chance = 0;

    if ( ch->fighting )
    {
        send_to_char( "No way!  You are still fighting!.\n\r", ch );
        return FALSE;
    }

    if ( ( chance = get_skill( ch, gsn_warcry_guarding ) ) == 0
    || ( ch->level < skill_table[gsn_warcry_guarding].skill_level[ch->gameclass]
    && !is_racial_skill( ch, gsn_warcry_guarding ) ) )
    {
      send_to_char("Try as you might, that warcry is beyond your ability.\n\r",ch);
      return FALSE;
    }

    if ( is_sanc_spelled( ch ) )
    {
        send_to_char( "You are already protected.\n\r", ch );
        return FALSE;
    }

    if ( number_percent() <= chance )
    {

        if ( ch->mana < 30 || ch->move < 40 )
        {
            send_to_char( "You don't have the energy for that.\n\r", ch );
            return FALSE;
        }

        af.where      = TO_SPELL_AFFECTS;
        af.type       = gsn_warcry_guarding;
        af.level      = ch->level;
        af.duration   = number_fuzzy(ch->level / 8);
        af.modifier   = 0;
        af.bitvector  = SAFF_WARCRY_GUARDING;
        af.location   = APPLY_NONE;
        affect_to_char( ch, &af );


        ch->mana -= 30;
        ch->move -= 40;
        ch->hit += number_range( 1, ( ch->level * 3 ) / 2 );
        if ( ch->hit > GET_HP( ch ) )
            ch->hit = GET_HP( ch );

        act( "You cry out to $g, and are filled with a powerful guarding.",
          ch, NULL, NULL, TO_CHAR);
        act( "$n calls out to $g, and is rewarded with a powerful guarding.",
             ch, NULL, NULL, TO_ROOM );

        WAIT_STATE(ch, PULSE_VIOLENCE);

        return TRUE;
    }
    else
    {
        act( "Your battle cry fails to appease $g.",
          ch, NULL, NULL, TO_CHAR);
        act( "$n fails to achieve the attention of $g.",
            ch, NULL, NULL, TO_ROOM );

        WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

        return FALSE;
    }

    return FALSE;
}

bool warcry_shout( CHAR_DATA *ch )
{
    CHAR_DATA *victim = NULL;
    int       chance = 0;

    if ( ( chance = get_skill( ch, gsn_warcry_shout ) ) == 0
    || ( ch->level < skill_table[gsn_warcry_shout].skill_level[ch->gameclass]
    && !is_racial_skill( ch, gsn_warcry_shout ) ) )
    {
      send_to_char("Try as you might, that warcry is beyond your ability.\n\r",ch);
      return FALSE;
    }

    if ( !ch->fighting )
    {
        send_to_char( "You are not in combat.\n\r", ch );
        return FALSE;
    }

    if ( number_percent() <= chance )
    {

        if ( ch->move < 50 )
        {
            send_to_char( "You don't have the stamina for that.\n\r", ch );
            return FALSE;
        }

        act( "You cry out to $g, hoping to strike fear into your enemy.",
          ch, NULL, NULL, TO_CHAR);
        act( "$n cries out a powerful shout to $g, hoping to frighten away $s enemy.",
             ch, NULL, NULL, TO_ROOM );

        for ( victim = ch->in_room->people ;victim ;victim = victim->next_in_room )
        {
            if ( victim->fighting )
            {
                if ( ch->fighting != victim
                || victim == ch  )
                    continue;
    
                if ( !victim->in_room
                ||   IS_SET( victim->in_room->room_flags, ROOM_SAFE      )
                ||   IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
                ||   IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  )
                ||   IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
                ||   victim->in_room->area != ch->in_room->area
                ||   ( saves_spell( ch->level, victim, DAM_MENTAL ) ) )
                    continue;

                act( "$n compels you to flee with $s mighty warcry!",
                    ch, NULL, victim, TO_VICT );
                act( "You fill $N with a sense of fear, making $M flee in terror!",
                    ch, NULL, victim, TO_CHAR );
                act( "$n forces $N to flee in terror due to $s mighty warcry!",
                    ch, NULL, victim, TO_NOTVICT );

                do_function( victim, &do_flee, "" );
                victim->move -= 20;
                if ( victim->move < 0 )
                    victim->move = 0;
            }
        }

        ch->move -= 50;
        WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
        return TRUE;
    }
    else
    {
        act( "Your battle cry fails to appease $g.",
          ch, NULL, NULL, TO_CHAR);
        act( "$n fails to achieve the attention of $s god.",
            ch, NULL, NULL, TO_ROOM );

        WAIT_STATE(ch, 3 * PULSE_VIOLENCE);
        return FALSE;
    }

    return FALSE;
}    


void do_warcry( CHAR_DATA *ch, char *argument )
{

    if ( IS_NPC( ch ) ) return;

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: warcry <type>\n\r", ch );
        return;
    }

    if ( ch->position == POS_SLEEPING )
    {
      send_to_char( "You can't cry out in your sleep!\n\r", ch );
      return;
    }

    if ( IS_GHOST( ch ) )
    {
        send_to_char( "You are far too stunned for that.\n\r", ch );
        return;
    }

    // Group improvement values.
    if ( !str_prefix( argument, "shout" ) )
    {
      if ( warcry_shout( ch ) )
        check_improve(ch, gsn_warcry_shout, TRUE, 2);
      else
        check_improve(ch, gsn_warcry_shout, FALSE, 2);
      return;
    }

    if ( !str_prefix( argument, "control" ) )
    {
      if ( !warcry_control( ch ) )
      {
        send_to_char( "You failed to control your emotions.\n\r", ch );
        return;
      }
      return;
    }

    if ( ch->position != POS_FIGHTING )
    { // don't improve at these while fighting

      if ( !str_prefix( argument, "hardening" ) )
      {
        if ( !is_affected( ch, gsn_warcry_hardening ) )
        {
          if ( warcry_hardening( ch ) )
            check_improve(ch, gsn_warcry_hardening, TRUE, 2);
          else
            check_improve(ch, gsn_warcry_hardening, FALSE, 2);
        }
        else
          send_to_char( "You already feel hardened.\n\r", ch );

        return;
      }

      if ( !str_prefix( argument, "rage" ) )
      {
        if ( !is_affected( ch, gsn_warcry_rage ) )
        {
          if ( warcry_rage( ch ) )
            check_improve(ch, gsn_warcry_rage, TRUE, 2);
          else
            check_improve(ch, gsn_warcry_rage, FALSE, 2);
        }
        else
          send_to_char( "You are already enraged.\n\r", ch );

        return;
      }

      if ( !str_prefix( argument, "vigor" ) )
      {
        if ( !is_affected( ch, gsn_warcry_vigor ) )
        {
          if ( warcry_vigor( ch ) )
            check_improve(ch, gsn_warcry_vigor, TRUE, 2);
          else
            check_improve(ch, gsn_warcry_vigor, FALSE, 2);
        }
        else
          send_to_char( "You are already in a state of vigor.\n\r", ch );

        return;
      }

      if ( !str_prefix( argument, "guarding" ) )
      {
        if ( !is_affected( ch, gsn_warcry_guarding ) )
        {
          if ( warcry_guarding( ch ) )
            check_improve(ch, gsn_warcry_guarding, TRUE, 2);
          else
            check_improve(ch, gsn_warcry_guarding, FALSE, 2);
        }
        else
          send_to_char( "You are already protected.\n\r", ch );

        return;
      }
    } // end fighting check
    else
    { // if we get here, char is fighting and didn't try something he can do while fighting
      send_to_char( "No way!  You are still fighting!.\n\r", ch );
      return;
    }

    send_to_char( "What warcry is that?\n\r", ch );
    return;
}

void do_overwhelm( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *fch;

  one_argument( argument, arg );
  if ( IS_NULLSTR(arg) )
  {
    send_to_char( "Overwhelm whom?\n\r", ch );
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("A ghost cannot overwhelm anybody{x.\n\r",ch);
    return;
  }

  if ( ( victim = get_char_room(ch, arg) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if (is_safe(ch,victim, FALSE))
    return;

  if (check_killsteal(ch,victim))
      return;

  if ( victim == ch )
  {
    send_to_char( "What about fleeing instead?\n\r", ch );
    return;
  }

  if ( !check_movement( ch, victim, TYPE_UNDEFINED, TRUE ) )
  {
    act( "You have run out of movement and cannot overwhelm $N!",
      ch, NULL, victim, TO_CHAR );
    act( "$n tries to overwhelm $N but is too fatigued!",
      ch, NULL, victim, TO_VICT );
    act( "$n tries to overwhelm $N but is too fatigued.",
      ch, NULL, victim, TO_NOTVICT );
    return;
  }

  if ( IS_NPC(victim) )
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

  if (is_safe(ch,fch, FALSE))
  {
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
  check_killer( ch, fch );

  if ( ch->fighting == NULL )
    set_fighting( ch, fch );

  set_fighting( fch, ch );
  return;
}

