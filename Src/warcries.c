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
void warcry_hardening( CHAR_DATA *ch );
void warcry_rage( CHAR_DATA *ch );
void warcry_vigor( CHAR_DATA *ch );
//void warcry_regeneration( CHAR_DATA *ch );
void warcry_guarding( CHAR_DATA *ch );
void warcry_aggression( CHAR_DATA *ch );
void warcry_shout( CHAR_DATA *ch );

/*
 * Warcry: Hardening
 * Affect: RES_WEAPON for a small amount of time
 *   Cost: 40 mana
 *  Level: 33
 */
void warcry_hardening( CHAR_DATA *ch )
{
    AFFECT_DATA af;
    int chance = 0;

    if ( ( chance = get_skill( ch, gsn_warcry_hardening ) ) == 0
    || ( ch->level < skill_table[gsn_warcry_hardening].skill_level[ch->gameclass]
    && !is_racial_skill( ch, gsn_warcry_hardening ) ) )
    {
      send_to_char("Try as you might, that warcry is beyond your ability.\n\r",ch);
      return;
    }

    if ( IS_SET( ch->res_flags, RES_WEAPON )
    &&   !IS_AFFECTED2( ch, AFF2_WARCRY_HARDENING ) )
    {
        send_to_char( "You already feel hardened.\n\r", ch );
        return;
    }
 

    if ( number_percent() <= chance )
    {
        if ( ch->move < 40 )
        {
            send_to_char( "You don't have the stamina for that.\n\r", ch );
            return;
        }
        
        //If affected clear it and update the affect time.
        if ( IS_AFFECTED2( ch, AFF2_WARCRY_HARDENING )  )
            remove_affect( ch, TO_AFFECTS2, AFF2_WARCRY_HARDENING );

        af.where     = TO_RESIST;
        af.type      = gsn_warcry_hardening;
        af.level     = ch->level;
        af.duration  = ch->level/8;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = RES_WEAPON;
        affect_to_char( ch, &af );

        ch->move -= 40;

        send_to_char( "You cry out to your god, and your muscles harden.\n\r", ch );
        act( "$n calls out to $s god, and is rewarded with hardened skin.",
             ch, NULL, NULL, TO_ROOM );

        check_improve(ch, gsn_warcry_hardening, TRUE, 2);

    }
    else
    {
        send_to_char( "Your battle cry fails to appease your god.\n\r", ch );
        act( "$n fails to achieve the attention of $s god.",
            ch, NULL, NULL, TO_ROOM );

        check_improve(ch, gsn_warcry_hardening, FALSE, 2);

    }

}

/*
 * Warcry: Rage
 * Affect: Raises hitroll/damroll
 *   Cost: 40 moves
 *  Level: 13
 */
void warcry_rage( CHAR_DATA *ch )
{
    AFFECT_DATA af;
    int chance = 0;

    if ( ( chance = get_skill( ch, gsn_warcry_rage ) ) == 0
    || ( ch->level < skill_table[gsn_warcry_rage].skill_level[ch->gameclass]
    && !is_racial_skill( ch, gsn_warcry_rage ) ) )
    {
      send_to_char("Try as you might, that warcry is beyond your ability.\n\r",ch);
      return;
    }

    if ( number_percent() <= chance )
    {

        if ( ch->move < 40 )
        {
            send_to_char( "You don't have the stamina for that.\n\r", ch );
            return;
        }

        if ( IS_AFFECTED2( ch, AFF2_WARCRY_RAGE )  )
            remove_affect( ch, TO_AFFECTS2, AFF2_WARCRY_RAGE );

        af.where     = TO_AFFECTS2;
        af.type      = gsn_warcry_rage;
        af.level     = ch->level;
        af.duration  = ch->level / 4;
        af.modifier  = UMAX( 1, ch->level / 5 );
        af.bitvector = AFF2_WARCRY_RAGE;

        af.location   = APPLY_HITROLL;
        affect_to_char(ch,&af);

        af.location   = APPLY_DAMROLL;
        affect_to_char(ch,&af);

        ch->move -= 40;

        send_to_char( "You cry out to your god, and are filled with powerful rage.\n\r", ch );
        act( "$n calls out to $s god, and is rewarded with a powerful rage.",
             ch, NULL, NULL, TO_ROOM );

        check_improve(ch, gsn_warcry_rage, TRUE, 2);

    }
    else
    {
        send_to_char( "Your battle cry fails to appease your god.\n\r", ch );
        act( "$n fails to achieve the attention of $s god.",
            ch, NULL, NULL, TO_ROOM );

        check_improve(ch, gsn_warcry_rage, FALSE, 2);
    }

}

/*
 * Warcry: Vigor
 * Affect: Adds haste and adds movement
 *   Cost: 50 mana
 *  Level: 13
 */
void warcry_vigor( CHAR_DATA *ch )
{
    AFFECT_DATA af;
    int chance = 0;

    if ( ( chance = get_skill( ch, gsn_warcry_vigor ) ) == 0
    || ( ch->level < skill_table[gsn_warcry_vigor].skill_level[ch->gameclass]
    && !is_racial_skill( ch, gsn_warcry_vigor ) ) )
    {
      send_to_char("Try as you might, that warcry is beyond your ability.\n\r",ch);
      return;
    }

    if ( number_percent() <= chance )
    {

        if ( ch->mana < 50 )
        {
            send_to_char( "You don't have the energy for that.\n\r", ch );
            return;
        }

        if ( IS_AFFECTED2( ch, AFF2_WARCRY_VIGOR )  )
            remove_affect( ch, TO_AFFECTS2, AFF2_WARCRY_VIGOR );

        af.where     = TO_AFFECTS2;
        af.type      = gsn_warcry_vigor;
        af.level     = ch->level;
        af.duration  = ch->level / 4;
        af.location  = APPLY_MOVE;
        af.modifier  = UMAX( 50, ch->level * 2 );
        af.bitvector = AFF2_WARCRY_VIGOR;
        affect_to_char( ch, &af );

        if (  !IS_AFFECTED( ch, AFF_HASTE ) )
        {
            af.where     = TO_AFFECTS;
            af.location  = APPLY_NONE;
            af.bitvector = AFF_HASTE;
            affect_to_char( ch, &af );
            send_to_char( "You begin to move faster.\n\r", ch );
        }

        ch->mana -= 50;

        send_to_char(
            "You cry out to your god, and are filled with a feeling of quickness.\n\r", ch );
        act( "$n calls out to $s god, and is rewarded with a quickened step.",
             ch, NULL, NULL, TO_ROOM );

        check_improve(ch, gsn_warcry_vigor, TRUE, 2);

    }
    else
    {
        send_to_char( "Your battle cry fails to appease your god.\n\r", ch );
        act( "$n fails to achieve the attention of $s god.",
            ch, NULL, NULL, TO_ROOM );

        check_improve(ch, gsn_warcry_vigor, FALSE, 2);
    }

}

/*
 * Warcry: Guarding
 * Affect: Reduces damage by 1/2 (Sanctuary)
 *   Cost: 40 mana 60 move
 *  Level: 35
 */
void warcry_guarding( CHAR_DATA *ch )
{
    AFFECT_DATA af;
    int chance = 0;

    if ( ( chance = get_skill( ch, gsn_warcry_guarding ) ) == 0
    || ( ch->level < skill_table[gsn_warcry_guarding].skill_level[ch->gameclass]
    && !is_racial_skill( ch, gsn_warcry_guarding ) ) )
    {
      send_to_char("Try as you might, that warcry is beyond your ability.\n\r",ch);
      return;
    }

    if ( number_percent() <= chance )
    {

        if ( ch->mana < 40 || ch->move < 60 )
        {
            send_to_char( "You don't have the energy for that.\n\r", ch );
            return;
        }

        if ( IS_SAFFECTED( ch, SAFF_WARCRY_GUARDING )  )
            remove_affect( ch, TO_SAFFECTS, SAFF_WARCRY_GUARDING );

        af.where     = TO_AFFECTS2;
        af.type      = gsn_warcry_guarding;
        af.level     = ch->level;
        af.duration  = ch->level / 4;
        af.location  = APPLY_MOVE;
        af.modifier  = UMAX( 50, ch->level * 2 );
        af.bitvector = SAFF_WARCRY_GUARDING;
        affect_to_char( ch, &af );

        ch->mana -= 40;
        ch->move -= 60;

        send_to_char(
            "You cry out to your god, and are filled with a feeling of quickness.\n\r", ch );
        act( "$n calls out to $s god, and is rewarded with a quickened step.",
             ch, NULL, NULL, TO_ROOM );

        check_improve(ch, gsn_warcry_vigor, TRUE, 2);

    }
    else
    {
        send_to_char( "Your battle cry fails to appease your god.\n\r", ch );
        act( "$n fails to achieve the attention of $s god.",
            ch, NULL, NULL, TO_ROOM );

        check_improve(ch, gsn_warcry_vigor, FALSE, 2);
    }

}


void do_warcry( CHAR_DATA *ch, char *argument )
{
    //char arg[MIL];

    if ( IS_NPC( ch ) ) return;

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: warcry <type>\n\r", ch );
        send_to_char( "        warcry list\n\r", ch );
        send_to_char( "        warcry maintain\n\r", ch );
        return;
    }

    if ( !str_prefix( argument, "hardening" ) )
    {
        warcry_hardening( ch );
        return;
    }

    if ( !str_prefix( argument, "rage" ) )
    {
        warcry_rage( ch );
        return;
    }

    if ( !str_prefix( argument, "vigor" ) )
    {
        warcry_vigor( ch );
        return;
    }

    send_to_char( "Go on w/ your bad self.\n\r", ch );

}

