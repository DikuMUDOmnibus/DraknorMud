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
#include "olc.h"  // to save scholars books

int hand_skill_damage(CHAR_DATA *ch, CHAR_DATA *vch, int *dt);

void do_descry( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA   *victim;
    OBJ_DATA    *obj;
    char        room[MSL];
    char        arg[MSL];

    argument = one_argument( argument, arg );

    if ( !ch )
    {
      bugf( "do_descry: Failed ch" );
      return;
    }

    if ( IS_NPC( ch ) ) return;

    //Immortals should be able to scry anyone I suppose, and without a message.
    if ( IS_IMMORTAL( ch ) )
    {

        if ( IS_NULLSTR( arg ) )
        {
            send_to_char( "Who do you want to descry?\n\r", ch );
            return;
        }

        victim = get_char_world( ch, arg );

        if ( !victim || !ch )
        {
            send_to_char( "Victim was not found.\n\r", ch );
            return;
        }

        mprintf( sizeof( room ), room, "%d look", victim->in_room->vnum );
        do_function( ch, &do_at, room );
        
        return;
    }

    if ( !IS_IMMORTAL( ch ) )
    {
        int chance = 0;

        if ( get_skill( ch, gsn_descry ) == 0
        ||   ch->level < skill_table[ gsn_descry ].skill_level[ ch->gameclass ] )
        {
            send_to_char( "You have no ability in that area.\n\r",ch);
            return;
        }

        if ( IS_NULLSTR( arg ) )
        {
            send_to_char( "Who do you want to descry?\n\r", ch );
            return;
        }
     
        victim = get_char_world( ch, arg );

        if ( !victim )
        {
            act( "$n flickers briefly.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "Your attempt at scrying failed.\n\r", ch );
            return;
        }

        if (!IS_NPC(victim)) // && !is_in_pk_range(ch, victim)
        {
            if (ch->level > 50)
            {
                if (victim->level > (int)(ch->level + 6) )
                {
                  send_to_char("They are too powerful for you to find.\n\r", ch );
                  return;
                }
            }
            else
            {
                if (victim->level > (ch->level + (int)(ch->level * 0.10)))
                {
                  send_to_char("They are too powerful for you to find.\n\r", ch );
                  return;
                }
            }
        }

        if ( IS_IMMORTAL( victim ) )
        {
            send_to_char(
                "The Gods are protected from such... simple skills.\n\r", ch );
            printf_to_char( victim,
                "%s is attempting to descry you.\n\r", ch->name );
            return;
        }

        //No unlinked areas or areas on other continents
        if ( IS_SET( victim->in_room->area->flags, AREA_DRAFT )
        ||   ( victim->in_room->area->continent != ch->in_room->area->continent ) )
        {
          send_to_char( "Your viewing fails to reach such distances.\n\r", ch );
          return;
        }

        obj = get_eq_char( ch, WEAR_HOLD );

        if ( obj == NULL
        || obj->item_type != ITEM_SCRY_MIRROR )
        {
            send_to_char( "You lack the proper materials for that.\n\r", ch );
            return;
        }

        if ( obj->value[1] <= 0 )
        {
            send_to_char( "The mirror has no charges left!\n\r", ch );
            extract_obj( obj );
            return;
        }

        if ( IS_SET( ch->imm_flags, IMM_MENTAL ) )
        {
            printf_to_char( ch,
                "%s is immune to your intrusion.\n\r", victim->name );
            printf_to_char( victim,
                "%s is attempting to descry you.\n\r", ch->name );
            return;
        }

        //Cannot look into nowhere rooms
        if ( IS_SET( victim->in_room->room_flags, ROOM_NOWHERE ) )
        {
          send_to_char( "Your viewing fails.\n\r", ch );
          obj->value[1]--;
          return;
        }

        //Cannot look into clan halls
        if ( IS_SET(victim->in_room->area->flags, AREA_CLANHALL))
        {
          send_to_char( "Your viewing fails.\n\r", ch );
          printf_to_char( victim,
            "%s is attempting to descry your location.\n\r", ch->name );
          obj->value[1]--;
          return;
        }

        chance = get_skill( ch, gsn_descry );

        if ( IS_SAFFECTED( ch, SAFF_FARSIGHT ) )
            chance += 8;

        if ( is_affected( victim, skill_lookup( "mind shield" ) ) )
            chance -= 10;

        if ( IS_SET( ch->res_flags, RES_MENTAL ) )
            chance -= 30;

        if ( IS_SET( victim->vuln_flags, VULN_MENTAL ) )
            chance += 30;
        
        if ( is_affected( ch, skill_lookup( "clear head" ) ) )
            chance += 10;

        if ( is_affected( victim, skill_lookup( "clear head" ) ) )
            chance -= 10;

        if ( IS_AFFECTED( ch, AFF_HIDE) )
            chance += 5;

        if ( IS_AFFECTED( victim, AFF_HIDE) )
            chance -= 5;

        if ( ch->gameclass == cOccultist ) chance -= 5;
        if ( ch->gameclass == cConjurer ) chance += 5;
        if ( ch->gameclass == cPriest ) chance += 5;
        if ( ch->gameclass == cDruid ) chance -= 5;

        chance -= ( ( ch->level - obj->level ) / 2 );
        chance += ( ch->level - victim->level );

        if ( chance > number_percent() )
        {
            act( "$n flickers briefly.", ch, NULL, NULL, TO_ROOM );
            printf_to_char( ch, "You descry upon %s's location.\n\r",
              ( IS_NPC( victim ) ? victim->short_descr : victim->name ) );

            obj->value[1]--;
            if ( obj->value[1] <= 0 )
            {
              act( "$p flares brightly and vanishes!",
                ch, obj, NULL, TO_CHAR );
              extract_obj( obj );
            }

            mprintf( sizeof( room ), room, "%d look", victim->in_room->vnum );
            do_function( ch, &do_at, room );
            check_improve( ch, gsn_descry, TRUE, 1 );
            WAIT_STATE( ch, skill_table[gsn_descry].beats * 3 / 2);
        }
        else
        {
            send_to_char( "Your viewing fails.\n\r", ch );
            printf_to_char( victim,
                "%s is attempting to descry your location.\n\r", ch->name );

            obj->value[1]--;
            if ( obj->value[1] <= 0 )
            {
              act( "$p flares brightly and vanishes!",
                ch, obj, NULL, TO_CHAR );
              extract_obj( obj );
            }

            check_improve( ch, gsn_descry, FALSE, 1 );
            WAIT_STATE( ch, skill_table[gsn_descry].beats);
        }

        return;
    }

    send_to_char( "Syntax: descry <name>\n\r", ch );

}

void do_tail( CHAR_DATA *ch, char *argument )
{
    char        arg[MIL];
    CHAR_DATA   *victim;
    int         chance;

    if ( IS_NPC( ch ) && !IS_SET( ch->off_flags, OFF_TAIL ) )
        return;

    if ( ( chance = get_skill( ch, gsn_tail ) ) == 0
    ||   ( !IS_NPC( ch ) && !is_racial_skill( ch, gsn_tail )
    &&     ch->level < skill_table[gsn_tail].skill_level[ch->gameclass] ) )
    {
        send_to_char( "Tail? What's that?\n\r", ch );
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
        send_to_char( "You run in circles chasing your tail.\n\r", ch );
        return;
    }

    //Make sure victim is = in size or smaller.
    if ( ch->size < victim->size )
    {
        act( "Your tail seems a feeble weapon against $S size.",
            ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( is_safe( ch, victim, FALSE ) )
        return;

    if ( !check_movement( ch, victim, gsn_tail, FALSE ) )
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
        act( "$n lays you out with $s powerful tail!",
            ch, NULL, victim, TO_VICT );
        act( "You slam your tail into $N, sending $M flying!",
            ch, NULL, victim, TO_CHAR );
        act( "$n lays $N out with $s powerful tail.",
            ch, NULL, victim, TO_NOTVICT );
        check_improve( ch, gsn_tail, TRUE, 1 );

        DAZE_STATE( victim, PULSE_VIOLENCE );
        WAIT_STATE( ch, skill_table[gsn_tail].beats );
        victim->position = POS_RESTING;
        damage( ch, victim, number_range( 2, 2 + 2 * ch->size + chance / 20 ),
            gsn_tail, DAM_BASH, FALSE, FALSE);
        update_pos( victim );

    }
    else
    {
        damage( ch, victim, 0, gsn_bash, DAM_BASH, FALSE, FALSE );
        act( "$N deftly avoids your tail!",
            ch, NULL, victim, TO_CHAR );
        act( "$n temporarily loses balance when $s tail misses $N.",
            ch, NULL, victim, TO_NOTVICT );
        act( "You unbalance $n by avoiding $s tail.",
            ch, NULL, victim, TO_VICT );
        check_improve( ch, gsn_tail, FALSE, 1 );

        WAIT_STATE( ch, skill_table[gsn_tail].beats * 3/2 );
        ch->position = POS_RESTING;
    }
}

void do_forage( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *fooditem;
  int  itemvnum = 0;
  int  skill   = 0;
  int  chance  = number_percent();
  int  objpct  = number_percent();
  int  sector  = SECT_UNUSED;
  bool found = FALSE;

  if ( IS_NPC( ch ) )
    return;

  skill = get_skill( ch, gsn_forage );

  if ( ( skill > chance )
  && ( ch->level > skill_table[gsn_forage].skill_level[ch->gameclass] ) )
    sector = ch->in_room->sector_type;

  if ( ch->in_room && ( fooditem = ch->in_room->contents ) )
  {
    while ( fooditem && !found )
    {
      if (fooditem->item_type == ITEM_FOOD) {
        act( "$n looks around for something to eat and finds $p.", ch, fooditem, NULL, TO_ROOM );
        act( "You look for something to eat and find $p.", ch, fooditem, NULL, TO_CHAR );
        get_silent_obj( ch, fooditem, NULL );
        found = TRUE;
        return;
      }
      fooditem = fooditem->next_content;
    }
  }

  switch (sector)
  {
    case SECT_INSIDE:
      if (objpct > 50)
      {
        itemvnum = 31660; // insects
        act( "$n roots around the ground and finds some insects.", ch, NULL, NULL, TO_ROOM );
        act( "You look around the ground and find some tasty insects.", ch, NULL, NULL, TO_CHAR );
      }
      break;
    case SECT_CITY:
    case SECT_FIELD:
    case SECT_GRAVEYARD:
    case SECT_UNDERGROUND:
      if (objpct > 50)
      {
        itemvnum = 31660; // insects
        act( "$n roots around the ground and finds some insects.", ch, NULL, NULL, TO_ROOM );
        act( "You look around the ground and find some tasty insects.", ch, NULL, NULL, TO_CHAR );
      }
      else
      {
        itemvnum = 31661; // rodent
        act( "$n looks down and lunges at a small rodent, capturing it.", ch, NULL, NULL, TO_ROOM );
        act( "You expertly capture a small, delicious rodent.", ch, NULL, NULL, TO_CHAR );
      }
      break;
    case SECT_FOREST:
    case SECT_HILLS:
    case SECT_MOUNTAIN:
      if (objpct > 50)
      {
        itemvnum = 31662; // wild berries
        act( "$n stops to pick some wild berries from a small bush.", ch, NULL, NULL, TO_ROOM );
        act( "You find some delicious wild berries on a bush.", ch, NULL, NULL, TO_CHAR );
      }
      else
      {
        itemvnum = 31661; // rodent
        act( "$n looks down and lunges at a small rodent, capturing it.", ch, NULL, NULL, TO_ROOM );
        act( "You expertly capture a small, delicious rodent.", ch, NULL, NULL, TO_CHAR );
      }
      break;
    case SECT_WATER_SWIM:
      if (objpct > 50)
      {
        itemvnum = 31663; // fish
        act( "$n quickly reaches out and snares a passing fish.", ch, NULL, NULL, TO_ROOM );
        act( "You reach out and grab a fish as it swims by.", ch, NULL, NULL, TO_CHAR );
      }
      else
      {
        itemvnum = 31664; // squid
        act( "$n suddenly grabs a small squid passing by.", ch, NULL, NULL, TO_ROOM );
        act( "You deftly capture a passing squid.", ch, NULL, NULL, TO_CHAR );
      }
      break;
    case SECT_WATER_NOSWIM:
      if (objpct > 50)
      {
        itemvnum = 31663; // fish
        act( "$n quickly reaches out and snares a fish swimming by.", ch, NULL, NULL, TO_ROOM );
        act( "You reach out and grab a fish as it swims by.", ch, NULL, NULL, TO_CHAR );
      }
      else
      {
        itemvnum = 31665; // crayfish
        act( "$n picks up a rock and grabs a large crayfish.", ch, NULL, NULL, TO_ROOM );
        act( "You find a tasty crayfish hiding under a large rock.", ch, NULL, NULL, TO_CHAR );
      }
      break;
    case SECT_AIR:
      if (objpct > 75)
      {
        itemvnum = 31666; // bird
        act( "$n suddenly grabs a bird flying through the air.", ch, NULL, NULL, TO_ROOM );
        act( "You are able to grab a bird as it flies by.", ch, NULL, NULL, TO_CHAR );
      }
      break;
    case SECT_DESERT:
      if (objpct > 50)
      {
        itemvnum = 31667; // scorpions
        act( "$n carefully moves a rock, grabs a scorpion and tears off its tail.", ch, NULL, NULL, TO_ROOM );
        act( "You find a scorpion under a rock and remove its tail.", ch, NULL, NULL, TO_CHAR );
      }
      else
      {
        itemvnum = 31668; // brush berries
        act( "$n stops to pick some berries from a small desert bush.", ch, NULL, NULL, TO_ROOM );
        act( "You find some delicious berries on a small desert bush.", ch, NULL, NULL, TO_CHAR );
      }
      break;
  }

  if (itemvnum == 0)
  {
    check_improve( ch, gsn_forage, FALSE, 1 );
    act( "$n looks around for something to eat, but fails.", ch, NULL, NULL, TO_ROOM );
    act( "You fail to find anything worth eating.", ch, NULL, NULL, TO_CHAR );
    return;
  }
  else
    check_improve( ch, gsn_forage, TRUE, 1 );

  fooditem = create_object( get_obj_index( itemvnum ), 0 );
  fooditem->value[1] = interpolateNew( ch->level, 1, MAX_LEVEL, 4, 16 ); /* Full */
  fooditem->value[0] = interpolateNew( ch->level, 1, MAX_LEVEL, 8, 32 ); /* Food */
  obj_to_char( fooditem, ch );
  return;
}

void do_repair( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  AFFECT_DATA *paf,*af_new;
//  bool aff_removed = FALSE;
  bool aff_found = FALSE;
  int skill = 0, chance = number_percent();

  argument = one_argument( argument, arg );

  if ( IS_NPC(ch)
  || ( ( skill = get_skill( ch, gsn_repair ) ) == 0 ) )
  {
    send_to_char( "You don't know how to repair armor.\n\r", ch );
    return;
  }

  if ( ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
  || !can_see_obj( ch, obj ) )
  {
    send_to_char( "You are not carrying that.\n\r", ch );
    return;
  }

  if ( obj->item_type != ITEM_ARMOR )
  {
    send_to_char( "You can only repair armor.\n\r", ch );
    return;
  }

  if ( skill < chance )
  {
    act( "You fail to repair the damage on $p.", ch, obj, NULL, TO_CHAR );
    act( "$n tried to repair $p, but fails.", ch, obj, NULL, TO_ROOM );
// maybe there should be a chance to destroy the object
    check_improve( ch, gsn_repair, FALSE, 1 );
    return;
  }
  else
    check_improve( ch, gsn_repair, TRUE, 1 );


  for ( paf = obj->affected; paf != NULL; paf = paf->next)
  {
/* attempt #2:
should check if it exists before attempting this

uh... this duplicates all the affects EXCEPT the acid_breath... closer
it also removes enchants, since acid_breath changes the enchant value instead of making a new one
*/
    if ( paf->type != gsn_acid_breath )
    {
      af_new = new_affect();
    
      af_new->next = obj->affected;
      obj->affected = af_new;

      af_new->where    = paf->where;
      af_new->type     = UMAX(0,paf->type);
      af_new->level    = paf->level;
      af_new->duration    = paf->duration;
      af_new->location    = paf->location;
      af_new->modifier    = paf->modifier;
      af_new->bitvector    = paf->bitvector;
    }
    else
      aff_found = TRUE;
  }

/* my first attempt
    if ( paf->type == gsn_acid_breath )
    {
      aff_found = TRUE;

      if (paf->level < ch->level)
      {
        aff_removed = TRUE;
        free_affect( paf ); // this doesn't work... it trashes your EQ...
      }
    }
*/

  if (aff_found)
  {
    act( "$p looks like new!", ch, obj, NULL, TO_CHAR );
    act( "$n successfully repairs $p.", ch, obj, NULL, TO_ROOM );
  }
  else
    act( "You are unable to repair $p.", ch, obj, NULL, TO_CHAR );
  return;
}

void do_fish( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;

  if (IS_NPC(ch))
    return;

  if ( ch->position == POS_FIGHTING )
  {
    send_to_char( "You're a little too busy right now!\n\r", ch );
    return;
  }

  if ( ( !IS_SET( ch->in_room->room_flags, ROOM_FISHING ) )
  &&   ( !IS_SET( ch->in_room->room_flags, ROOM_SHIP ) )
  &&   (  ch->in_room->sector_type != SECT_WATER_SWIM ) )
  {
    send_to_char( "You can't fish here.\n\r", ch );
    return;
  }

  if ( ( ( obj = get_eq_char(ch, WEAR_HOLD) ) == NULL )
  ||     ( obj->item_type != ITEM_FISHING_ROD ) )
  {
    send_to_char( "You aren't holding a fishing rod.\n\r", ch );
    return;
  }

  if (IS_SET(ch->act, PLR_FISHING))
  {
    send_to_char( "You reel in your line and cast again.\n\r", ch );
    act("$n's reels in $s line and casts again.",ch,NULL,NULL,TO_ROOM);
  }
  else
  {
    send_to_char( "You cast out your line.\n\r", ch );
    act("$n's casts out $s line.",ch,NULL,NULL,TO_ROOM);
  }

  SET_BIT(ch->act, PLR_FISHING);
  return;
}
