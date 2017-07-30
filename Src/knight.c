/**************************************************************************\
 *      The Draknor Codebase(Draknor) is copyright 2003 by Robert Leonard *
 *      Draknor has been created with much time and effort from many      *
 *      different people's input and ideas.                               *
 *      Using this code without the direct permission of the writers is   *
 *      not allowed.                                                      *
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

void do_shield_bash( CHAR_DATA *ch, char *argument )
{
    char        arg[MIL];
    CHAR_DATA   *victim;
    int         chance, dam;
    OBJ_DATA    *shield;

    if ( IS_NPC(ch) )
        return;

    if ( ( chance = get_skill( ch, gsn_shield_bash ) ) == 0
    &&     ch->level < skill_table[gsn_shield_bash].skill_level[ch->gameclass] )
    {
        send_to_char( "You don't know how to do that.\n\r", ch );
        return;
    }

    shield = get_eq_char(ch, WEAR_SHIELD);

    if (!shield)
    {
      send_to_char("You are not wearing a shield.\n\r",ch);
      return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        if ( ( victim = ch->fighting ) == NULL )
        {
            send_to_char( "But you aren't fighting anyone!\n\r", ch );
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
        send_to_char( "You want to slam your shield... into yourself?\n\r", ch );
        return;
    }

    if ( is_safe( ch, victim, FALSE ) )
        return;

    if ( !check_movement( ch, victim, gsn_shield_bash, FALSE ) )
        return;

    if ( victim->position < POS_FIGHTING )
    {
        act( "You'll have to let $N get back up first.",
            ch, NULL, victim, TO_CHAR );

        return;
    }

    /* Determine the chance of success */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 15;
    else
        chance += (ch->size - victim->size) * 10;


    chance += get_curr_stat(ch,STAT_STR);
    chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;
    chance -= GET_AC(victim,AC_BASH) /25;

    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 30;

    chance += (ch->level - victim->level);

    if ( !IS_NPC( victim )
    &&  chance < get_skill( victim, gsn_dodge ) )
    {
        chance -= 3 * ( get_skill( victim, gsn_dodge ) - chance );
    }
    if ( number_percent() < chance )
    {
        act( "$n slams $s shield into you!",
            ch, NULL, victim, TO_VICT );
        act( "You slam your shield into $N, knocking $M briefly off $s feet!",
            ch, NULL, victim, TO_CHAR );
        act( "$n slams $s shield into $N.",
            ch, NULL, victim, TO_NOTVICT );
        check_improve( ch, gsn_shield_bash, TRUE, 1 );

        DAZE_STATE( victim, PULSE_VIOLENCE );
        WAIT_STATE( ch, skill_table[gsn_shield_bash].beats );
        victim->position = POS_RESTING;

        dam = (shield->level * 3) /2;
        dam += ch->size * 10;
        dam = number_range( dam, (dam *3)/2);

        damage( ch, victim, dam, gsn_shield_bash, DAM_BASH, FALSE, FALSE);
        update_pos( victim );

    }
    else
    {
        damage( ch, victim, 0, gsn_bash, DAM_BASH, FALSE, FALSE );
        act( "$N moves away from your attack!",
            ch, NULL, victim, TO_CHAR );
        act( "$n temporarily loses balance when $s shield bash misses $N.",
            ch, NULL, victim, TO_NOTVICT );
        act( "You unbalance $n by avoiding $s shield bash.",
            ch, NULL, victim, TO_VICT );
        check_improve( ch, gsn_shield_bash, FALSE, 1 );

        WAIT_STATE( ch, skill_table[gsn_shield_bash].beats * 3/2 );
        ch->position = POS_RESTING;
    }

}
