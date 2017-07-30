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
#include "clan.h"
#include "interp.h"

void check_saved(CHAR_DATA *ch);

bool damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type, bool show,bool spell)
{
  // removed damage_old and lair_damage as they were redundant
  OBJ_DATA *corpse;
  OBJ_DATA *weapon = NULL;
  bool immune;
  char log_buf[MSL];
  int tdam =0;
  int loss = 0;
  double percent_change =0.0;
  //int clannie=0;
  int madexp =0;
  char buf[MSL];

#if TESTIT
  printf_to_char(ch,"DAMAGE #1: DT = %d\n",dt);
#endif
  if ( victim->position == POS_DEAD )
    return FALSE;

#if TESTIT
  printf_to_char(ch,"DAMAGE #1-2: DT = %d\n",dt);
#endif

  /* damage reduction */
  if ( dam > 35)
    dam = (dam - 35)/2 + 35;
  if ( dam > 80)
    dam = (dam - 80)/2 + 80;

  check_cheat(&dam, dt, ch);

  if ( victim != ch )
  {
    /*
     * Certain attacks are forbidden.
     * Most other attacks are returned.
     */
    if ( is_safe( ch, victim , FALSE) )
      return FALSE;

    check_killer( ch, victim );

    if (ch != victim && (dt >= TYPE_HIT))
    {
      if ( IS_CLASS_HIGHWAYMAN( victim ) )
      {
        if ( check_blade_weave(ch, victim, TRUE)
             && ( dam > 0 && !check_feint(ch, victim ) ) )
        {
          if (ch->fighting == NULL)
            set_fighting(ch, victim);
          if (victim->fighting == NULL)
            set_fighting(victim, ch);
          if (victim->master == ch)
            stop_follower(victim);
          return FALSE;
        }
      }
      else
      {
        if ( check_parry(ch, victim, TRUE)
             && ( dam > 0 && !check_feint(ch, victim ) ) )
        {
          if (ch->fighting == NULL)
            set_fighting(ch, victim);
          if (victim->fighting == NULL)
            set_fighting(victim, ch);
          if (victim->master == ch)
            stop_follower(victim);
          return FALSE;
        }
      } // parry vs. blade weave

      if ( IS_CLASS_MYSTIC( victim ) )
      {
        if ( check_sidestep(ch, victim)
             && ( dam > 0 && !check_feint(ch, victim ) ) )
        {
          if (ch->fighting == NULL)
            set_fighting(ch, victim);
          if (victim->fighting == NULL)
            set_fighting(victim, ch);
          if (victim->master == ch)
            stop_follower(victim);
          return FALSE;
        }
      }
      else
      {
        if ( check_dodge(ch, victim)
             && ( dam > 0 && !check_feint(ch, victim ) ) )
        {
          if (ch->fighting == NULL)
            set_fighting(ch, victim);
          if (victim->fighting == NULL)
            set_fighting(victim, ch);
          if (victim->master == ch)
            stop_follower(victim);
          return FALSE;
        }
      } // dodge vs. sidestep

      if ( check_shield_block(ch,victim)
           && ( dam > 0 && !check_feint(ch, victim) ) )
      {
        if (ch->fighting == NULL)
          set_fighting(ch, victim);
        if (victim->fighting == NULL)
          set_fighting(victim, ch);
        if (victim->master == ch)
          stop_follower(victim);
        return FALSE;
      }
    }

    if ( victim->position > POS_STUNNED )
    {
      if ( victim->fighting == NULL )
      {
        set_fighting( victim, ch );

        if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_KILL ) )
          mp_percent_trigger( victim, ch, NULL, NULL, TRIG_KILL );
      }

      if (victim->timer <= 4)
        victim->position = POS_FIGHTING;
      if ( ch->fighting == NULL )
        set_fighting( ch, victim );
    }

    /*
     * More charm stuff.
     */
    if ( victim->master == ch )
      stop_follower( victim );
  }

// Taeloch's totally buggy 2nd-weapon check
// this does not work if both weapons have the same dam_type, and a
// player has one skill in critical strike range and one that isn't
  if (!IS_NPC( ch )                                        // no NPCs, thank you
      &&   dt >= TYPE_HIT                                      // is a weapon/hand attack
      && ( weapon = get_eq_char(ch,WEAR_WIELD) )               // player has a weapon
      &&  !spell                                               // attack is not a spell
      &&   show                                                // messages are shown
      &&   dam_type != attack_table[weapon->value[3]].damage ) // dam_type doesn't match primary weapon's dam_type
  {
    weapon = get_eq_char(ch,WEAR_SECONDARY);               // load secondary weapon
    if ( !weapon                                           // no secondary weapon
         || dam_type != attack_table[weapon->value[3]].damage ) // dam_type doesn't match secondary weapon's dam_type
      weapon = get_eq_char(ch,WEAR_WIELD);                 // neither weapon matched dam_type, so guess?
  }
// end 2nd weapon check

  /*
   * Inviso attacks ... not.
   */
  if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
  {
    remove_affect(ch, TO_AFFECTS, AFF_INVISIBLE);
    act( "{G$n{x fades into existence.", ch, NULL, NULL, TO_ROOM );
  }
  if ( IS_AFFECTED(ch, AFF_HIDE) )
  {
    affect_strip( ch, gsn_hide );
    affect_strip( ch, gsn_chameleon );
    REMOVE_BIT( ch->affected_by, AFF_HIDE );
    act( "{G$n{x comes out of hiding.", ch, NULL, NULL, TO_ROOM );
  }

  set_plevel(victim,ch);
#if TESTIT
  printf_to_char(ch,"DAMAGE #2: DT = %d\n",dt);
#endif
  if (victim->hate)
  {
    free_string(victim->hate->name);
    victim->hate->name = str_dup(ch->name, victim->hate->name);
    victim->hate->who = ch;
#if MEMDEBUG
    memdebug_check(victim,"damage: Victim->hate clear");
#endif
  }
  else
    start_hating(victim,ch);

  /* CLASS BASED DAMAGE MODS */
  /* some notes:  Mage, Priests, Druid get bonus for magic use to
     damage.  get penalties for regular fighting.
     Knight, Thief, Barbarian, Mystic get bonus for fighting.  Negative
     for magic.
     Warlock gets MORE spell damage nothing either way.

     CURRENTLY: Believe regular fighting is okay for Knight types.
     So we single penalize the warlock and double the mage classes.
     We need to double/tripple magic damage here for mage classes.
  */
  tdam = dam/4;
  if ( ( ch->gameclass == cConjurer )
       ||   ( ch->gameclass == cPriest )
       ||   ( ch->gameclass == cDruid )
       ||   ( ch->gameclass == cOccultist )
       ||   ( ch->gameclass == cAlchemist ) )
  {
    if (spell)
      dam = dam + (dam - (int)dam/4.0);
    else
      dam = dam - tdam;
  }

  if (ch->gameclass == cDruid)
  {
    if (IS_OUTSIDE(ch))
      dam += dam/4.0;
    else
      dam -= dam/4.0;
  }

  if ((ch->gameclass == cKnight)
      ||  (IS_CLASS_HIGHWAYMAN( ch ) )
      ||  (ch->gameclass == cBarbarian)
      ||  (ch->gameclass == cWoodsman)
      ||  (ch->gameclass == cMystic))
  {
    if (spell)
      dam = dam - tdam;
    else
      dam = dam;
  }

  if (ch->gameclass == cWarlock)
  {
    if (spell)
      dam = dam + tdam;
    else
      dam = dam - dam/6;
  }

  if ((IS_CLASS_INQUISITOR(ch) || IS_CLASS_OCCULTIST(ch)) && IS_SET(victim->act, ACT_UNDEAD))
    dam = dam + ((int) dam / 10.0);	/*Inquisitor and Occultists get 10% dam bonus to  undead.*/

  /*
   * Damage modifiers.
   */
  if ( dam > 1 && !IS_NPC(victim)
       &&   victim->pcdata->condition[COND_DRUNK]  > 10 )
    dam = (int)(dam * 0.9);

  if ( dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY) )
    dam = (int) dam/2;

  if ( dam > 1 && IS_AFFECTED(victim, AFF_AQUA_ALBEDO) )
    dam = (int) dam/2;

  if ( dam > 1 && IS_AFFECTED2( victim, AFF2_INVUN) )
    dam = (int) dam/2;

  if ( dam > 1 && IS_AFFECTED2(victim, AFF2_FADE_OUT) )
    dam = (int) dam/2;

  if ( dam > 1 && IS_AFFECTED2( victim, AFF2_SHROUD) )
    dam = (int) dam/2;

  if ( dam > 1 && IS_AFFECTED2( victim, AFF2_RADIANT) )
    dam = (int) dam/2;

  if ( dam > 1 && IS_AFFECTED2( victim, AFF2_NIRVANA) )
    dam = (int) dam/2;

  if ( (dam > 1 && IS_SET(victim->spell_aff, SAFF_MAGESHIELD))
       && spell)
    dam = (int) dam/2;

  if ( (dam > 1 && IS_SET(victim->spell_aff, SAFF_IRONWILL)))
    dam = (int) dam/2;

  if ( (dam > 1 && IS_SET(victim->spell_aff, SAFF_WARRIORSHIELD))
       && !spell)
    dam = (int) dam/2;

  if ( ( dam > 1
         &&     IS_SET( victim->spell_aff, SAFF_WARCRY_GUARDING ) ) )
    dam = (int) dam / 2;


  if ( dam > 1 &&
       ( (IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch))
         ||  ((IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) ))
         ||  (((IS_AFFECTED2(victim, AFF2_PROTECT_NEUTRAL) || IS_SAFFECTED( victim, SAFF_PROTECT_NEUTRAL))
               && IS_NEUTRAL(ch)))))
    dam -= (int)dam / 4;

  if ((dam > 1)
      && ( IS_SAFFECTED(victim, SAFF_PROTECT_HOLY) )
      && ( dam_type == DAM_HOLY ) )
    dam -= (int)dam / 4;

  if ((dam > 1)
      && ( IS_SAFFECTED(victim, SAFF_PROTECT_NEGATIVE) )
      && ( dam_type == DAM_NEGATIVE ) )
    dam -= (int)dam / 4;

  immune = FALSE;
  /*    if (get_skill(ch, gsn_tumble) > 2) {
  dam *= 0.8;
  }*/


  switch (check_immune(victim,dam_type))
  {
    case(IS_IMMUNE):
            immune = TRUE;
      dam = 0;
      break;
    case(IS_RESISTANT):
            dam -= (int)dam/3;
      break;
    case(IS_VULNERABLE):
            dam += (int)dam/2;
      break;
  }

  if (ch != victim && (dt >= TYPE_HIT))
{
    if ( dam  > 0 )
      if ( check_critical(ch,victim,weapon) ) // Taeloch: trying to account for secondary weapons
        dam += dam * 4 / 10;

#ifdef INQUISITOR
    if ( ( dam > 5 )
         &&   guided_strike( ch, victim ) )
      dam += dam * 4 / 10;
#endif
  }

#if TESTIT
  printf_to_char(ch,"DAMAGE #X - right before dam message: DT = %d\n",dt);
#endif
  if (show)
    dam_message( ch, victim, dam, dt, immune );

  if (!dam)
    return FALSE;

  /*
   * Hurt the victim.
   * Inform the victim of his new state.
   */

  /* To prevent underflow.. */
  if (victim->hit + 12 < dam)
    victim->hit = -12;
  else
    victim->hit -= dam;

  if ( !IS_NPC( victim )
       &&    victim->level >= LEVEL_IMMORTAL
       &&    victim->hit < 1 )
    victim->hit = 1;

  check_saved( victim );

  update_pos( victim );

  switch ( victim->position )
  {
    case POS_MORTAL:
      act( "$n is mortally wounded, and will {rdie{x soon, if not aided.",
           victim, NULL, NULL, TO_ROOM );
      send_to_char(
        "You are mortally wounded, and will {rdie{x soon, if not aided.\n\r",
        victim );
      break;

    case POS_INCAP:
      act( "$n is incapacitated and will slowly {rdie{x, if not aided.",
           victim, NULL, NULL, TO_ROOM );
      send_to_char(
        "You are incapacitated and will slowly {rdie{x, if not aided.\n\r",
        victim );
      break;

    case POS_STUNNED:
      act( "$n is stunned, but will probably recover.",
           victim, NULL, NULL, TO_ROOM );
      send_to_char("You are stunned, but will probably recover.\n\r",
                   victim );
      break;

    case POS_DEAD:
      if (victim->in_room && IS_SET(victim->in_room->room_flags, ROOM_ARENA))
      {
        act( "{w$n{x has been {YDEFEATED{x!!", victim, 0, 0, TO_ROOM );
        send_to_char( "{WYou have been {YDEFEATED{x!!\n\r\n\r", victim );
      }
      else
      {
        act( "{w$n{x is {rDEAD{x!!", victim, 0, 0, TO_ROOM );
        send_to_char( "{WYou have been {rKILLED{x!!\n\r\n\r", victim );
      }

      /*
       * Death trigger
       */
      if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_DEATH) )
      {
        victim->position = POS_STANDING;
        victim->wait = 0; // allows triggers to execute upon death, if mob is in wait state
        ap_percent_trigger( victim, ch, NULL, NULL, TRIG_DEATH, MOB_PROG );
        victim->position = POS_DEAD;
      }
      break;

    default:
      if ( dam > victim->max_hit / 4 )
        send_to_char( "That really did {RHURT{x!\n\r", victim );
      if ( victim->hit < victim->max_hit / 4 )
        send_to_char( "You sure are {RBLEEDING{x!\n\r", victim );
      break;
  }
  /*
   * Sleep spells and extremely wounded folks.
   */
  if ( !IS_AWAKE(victim) )
    stop_fighting( victim, FALSE );
  /*
   * Payoff for killing things.
   */
  if ( victim->position == POS_DEAD )
  {
    group_gain( ch, victim );

    /* track kill/death tallies */
    if ( !IS_NPC( victim ) )
    {
      if ( !IS_NPC( ch ) )
      {
        /* char kills char */
        if (ch != victim)
        {
          ch->pcdata->pkills++;
          victim->pcdata->pdeaths++;
        }
      }
      else
      {
        /* mob kills char */
        victim->pcdata->deaths++;
        if ( ( ch->master != NULL )
             && !IS_NPC( ch->master )
             && ( ch->in_room == ch->master->in_room ) )
        {
          /* pet kills char (while master in room) */
          ch->master->pcdata->pkills++;
        }
      }
    }
    else
    {
      if ( !IS_NPC( ch ) )
      {
        /* char kills mob */
        ch->pcdata->kills++;
      }
      else
      {
        /* mob kills mob */
        if ( ( ch->master != NULL )
             && !IS_NPC( ch->master )
             && ( ch->in_room == ch->master->in_room ) )
        {
          /* pet kills mob (while master in room) */
          ch->master->pcdata->kills++;
        }
      }
    }
    /* end death/kill tallies */

    if ( !IS_NPC(victim) )
    {
      mprintf( sizeof(log_buf), log_buf, "%s killed by %s at %d",
               victim->name,
               (IS_NPC(ch) ? ch->short_descr : ch->name),
               ch->in_room->vnum );
      log_string( log_buf );

      /*
       * Dying penalty:
       * 2/3 way back to previous level.
       */
      if ( !IS_IMMORTAL( ch )
           && ( !IS_SET( ch->in_room->room_flags, ROOM_ARENA ) ) )
      {
        int XPL, qloss;
        XPL = exp_per_level( victim, victim->pcdata->points );

        madexp = ( victim->exp - TNL( XPL, victim->level - 1 ) );
        if ( madexp > 0 )
        {
          ch->num_bailed = 0;
          percent_change = number_range( 20, 80 ) * .01;
          loss = -1 * madexp * percent_change;
          qloss = XPL / 100;
          victim->questdata->curr_points -= qloss;
          victim->questdata->glory -= qloss;

          if ( victim->questdata->glory < 0 )
            victim->questdata->glory = 0;

          if ( victim->questdata->curr_points < 0 )
          {
            qloss += victim->questdata->curr_points;
            victim->questdata->curr_points = 0;
          }

          printf_to_char(victim, "You lost {R%d glory {xfor your death.\n\r",
                         qloss );
          gain_exp( victim, loss );
        }
        if ( madexp < 0 )
        {
          if ( victim->exp < TNL( XPL, victim->level - 1 ) )
            victim->exp = TNL( XPL, victim->level - 1 );
        }
        /*	    bugf("GAIN: madeexp = %d, total loss = %d. change percent = %f\n\r",madexp,
          loss, percent_change);

          if ( victim->exp > exp_per_level(victim,victim->pcdata->points)
          * victim->level )
          gain_exp( victim, (2 * (exp_per_level(victim,victim->pcdata->points)
          * victim->level - victim->exp)/3) +
          50 );*/
      }
    }

    mprintf( sizeof(log_buf), log_buf,
             "{W%s{x got {Rtoasted{x by {C%s{x at {G%s{x [room %d]",
             (IS_NPC(victim) ? victim->short_descr : victim->name),
             (IS_NPC(ch) ? ch->short_descr : ch->name),
             ch->in_room->name, ch->in_room->vnum);

    if (IS_NPC(victim))
      wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
    else
      wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

    if (!IS_NPC(ch) && !IS_NPC(victim) && victim->pcdata->bounty > 0)
    {
      if ( ( ch->clan->name == victim->clan->name )
           &&    !IS_SET( ch->clan->clan_flags, CLAN_INDEPENDENT ) )
      {
        send_to_char("{RNo Bounty received, same clan{x\n\r",ch);
        send_to_char("{WYour bounty remains. Same clan cannot claim it{x\n\r",
                     victim );
      }
      else
      {
        if (victim != ch)
        {
          if (!IS_SET(victim->in_room->room_flags,ROOM_ARENA) && !IS_IMMORTAL(ch))
          {
            printf_to_char(ch,
                           "You receive a {Y%d {yg{Yo{yld{x bounty, for killing %s.\n\r",
                           victim->pcdata->bounty, victim->name);
            ch->gold += victim->pcdata->bounty;
            victim->pcdata->bounty =0;
          }
        }
      }
    }

    if (!IS_SET(victim->in_room->room_flags,ROOM_ARENA))
    {
      if (!IS_NPC(ch) && !IS_NPC(victim))
      {
        /* --- Commented away for now, shall be revived! LOOK
        	  if ((clannie =find_clan_roster_member(ch)) != -1)
        	    clan_roster[ch->clan].clannie[clannie].kills = ch->pcdata->kills;
        	  if ((clannie =find_clan_roster_member(victim)) != -1)
        	    clan_roster[ch->clan].clannie[clannie].killed = ch->pcdata->deaths;
        */
      }

      if (IS_SET(victim->spell_aff,SAFF_MARTYR))
      {
        /* char explodes and hits everyone in room..  friend and not */
        martyr_char(victim, gsn_martyr);
      }
    }
    /* dump the flags */
    if (  ch != victim
          && ( !IS_NPC( ch ) || IS_PET( ch ) )
          &&    ch->clan_name != victim->clan_name
          //|| ( IS_SET( ch->clan->clan_flags, CLAN_INDEPENDENT ) ) )
          && !IS_SET(ch->in_room->room_flags, ROOM_ARENA)
          && !IS_IMMORTAL( ch ) )
    {
      REMOVE_BIT(victim->act,PLR_KILLER);
      REMOVE_BIT(victim->act,PLR_THIEF);
      REMOVE_BIT(victim->act,PLR_VIOLENT);

      if ( ch->clan
           &&   victim->clan
           &&   IS_SET( ch->clan->clan_flags, CLAN_LAW )
           &&   !IS_NPC(victim) )
      {
        sprintf( buf, "Justice has been served on %s.", victim->name );
        do_function( ch, &do_info, buf );
      }

      victim->vflag_timer = 0;
    }

    raw_kill( victim, ch );
    /* RT new auto commands */

    if (!IS_NPC(ch)
        &&  (corpse = get_obj_list(ch,"corpse",ch->in_room->contents)) != NULL
        &&  (corpse->item_type == ITEM_CORPSE_NPC)
        && can_see_obj(ch,corpse)
        && (ch->position != POS_DEAD ) )
    {
      OBJ_DATA *coins;

      corpse = get_obj_list( ch, "corpse", ch->in_room->contents );

      if ( IS_SET(ch->act, PLR_AUTOLOOT)
           && corpse && corpse->contains) /* exists and not empty */
        do_function( ch,&do_get, "all corpse" );

      if (IS_SET(ch->act,PLR_AUTOGOLD)
          && corpse
          && corpse->contains
          && !IS_SET(ch->act,PLR_AUTOLOOT))
        if ( ( coins = get_obj_list( ch, "gcash", corpse->contains ) ) != NULL )
          do_function(ch,&do_get, "all.gcash corpse");

      if ( IS_SET(ch->act, PLR_AUTODONATE)
           &&   !ch->fighting ) /* Added to avoid donating when Undead show */
      {
        if ( ( IS_SET( ch->act, PLR_AUTOLOOT )
               ||     IS_SET( ch->act, PLR_AUTOGOLD ) )
             &&     corpse
             &&     corpse->contains )
          return TRUE;  /* leave if corpse has treasure */
        else if (!corpse->contains)
        {
          bool stillfighting = FALSE;
          CHAR_DATA *fch;

          for ( fch = ch->in_room->people; fch; fch = fch->next_in_room )
          {
            if (fch->fighting == ch)
            {
              stillfighting = TRUE;
              break;
            }

            if ( ch->pet
                 &&   fch->fighting == ch->pet)
            {
              stillfighting = TRUE;
              break;
            }
          }

          if ( !stillfighting )
            do_function( ch,&do_donate, "corpse" );
        }
      }

      if ( IS_SET(ch->act, PLR_AUTOSAC) )
      {
        if ( ( IS_SET( ch->act, PLR_AUTOLOOT )
               ||     IS_SET( ch->act, PLR_AUTOGOLD ) )
             &&     corpse
             &&     corpse->contains )
          return TRUE;  /* leave if corpse has treasure */
        else
          do_function( ch,&do_sacrifice, "corpse" );
      }
    }
    else
    {
      if ( ch->master
           && !IS_NPC(ch->master)
           && (corpse = get_obj_list(ch->master,"corpse",ch->master->in_room->contents)) != NULL
           && corpse->item_type == ITEM_CORPSE_NPC
           && can_see_obj(ch->master,corpse)
           && (ch->position != POS_DEAD ) )
      {
        OBJ_DATA *coins;

        corpse = get_obj_list( ch->master, "corpse", ch->master->in_room->contents );

        if ( IS_SET(ch->master->act, PLR_AUTOLOOT)
             && corpse
             && corpse->contains) /* exists and not empty */
          do_function( ch->master,&do_get, "all corpse" );

        if (IS_SET(ch->master->act,PLR_AUTOGOLD)
            && corpse
            && corpse->contains
            && !IS_SET(ch->master->act,PLR_AUTOLOOT))
          if ( ( coins = get_obj_list( ch->master, "gcash", corpse->contains ) ) != NULL )
            do_function(ch->master,&do_get, "all.gcash corpse");

        if ( IS_SET(ch->master->act, PLR_AUTODONATE)
             &&   !ch->master->fighting )
        {
          if ( ( IS_SET( ch->master->act, PLR_AUTOLOOT )
                 ||     IS_SET( ch->master->act, PLR_AUTOGOLD ) )
               &&     corpse
               &&     corpse->contains )
            return TRUE;  /* leave if corpse has treasure */
          else
            if (!corpse->contains) do_function( ch->master,&do_donate, "corpse" );
        }

        if ( IS_SET(ch->master->act, PLR_AUTOSAC) )
        {
          if ( ( IS_SET( ch->master->act, PLR_AUTOLOOT )
                 ||     IS_SET( ch->master->act, PLR_AUTOGOLD ) )
               &&     corpse
               &&     corpse->contains )
            return TRUE;  /* leave if corpse has treasure */
          else
            do_function( ch->master,&do_sacrifice, "corpse" );
        }
      } // pet master autos
    }
    return TRUE;
  }

  if ( victim == ch )
    return TRUE;

  /*
   * Take care of link dead people.
   */
  if ( !IS_NPC(victim) && victim->desc == NULL )
  {
    if ( number_range( 0, victim->wait ) == 0 )
    {
      do_function(victim, &do_recall, "" );
      return TRUE;
    }
  }

  /*
   * Wimp out?
   */
  if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
  {
    if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
           &&   victim->hit < victim->max_hit / 5)
         ||   ( IS_CHARMED(victim) &&  victim->master->in_room != victim->in_room ) )
      do_function(victim, &do_flee, "" );
  }

  if ( !IS_NPC(victim)
       &&   victim->hit > 0
       &&   victim->hit <= victim->wimpy
       &&   victim->wait < PULSE_VIOLENCE / 2 )
    do_function(victim, &do_flee, "" );

  tail_chain( );
  return TRUE;
}




bool check_cheat(int *dam, int dt, CHAR_DATA *ch)
{
  OBJ_DATA *obj;

  /*
   * Stop up any residual loopholes.
   */
  if ( *dam > MAX_DAMAGE && dt >= TYPE_HIT)
  {
    /*
    bugf( "Damage: %d: more than %d points!", *dam, MAX_DAMAGE );
    */
    *dam = MAX_DAMAGE;
    if (!IS_IMMORTAL(ch))
    {
      obj = get_eq_char( ch, WEAR_WIELD );
      send_to_char("You really shouldn't cheat.\n\r",ch);
      if (obj != NULL)
        extract_obj(obj);
    }

  }
  return (0);
}


void check_saved(CHAR_DATA *ch)
{
  if (IS_NPC(ch))
    return;

  if (ch->level >= LEVEL_CLAN)
    return;

  if (IS_SET(ch->in_room->room_flags, ROOM_ARENA) )
    return;

  if (ch->hit > ( GET_HP(ch) / 10 ) )
    return;

  if (number_percent() > 10)
    return;

  send_to_char("A warm, healing {yl{Yi{yght{x bathes you with a blessing from the gods.\n\r",ch);
  send_to_char("{WYou{x are warned...{rDeath{x stalks you.\n\r",ch);
  ch->hit = GET_HP( ch );

}
