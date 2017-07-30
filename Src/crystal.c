/******************************************************************************
*                                                    *
* The design on the Crystal system belongs to the Lands of Draknor,           * 
* and Robert Leonard.  Use of this system without consent is not allowed.     *
* Concept Design: do_cleanse:                                                 *
*         Check for NPC_CORPSE && extra_flag crystal.                         *
*         Check for area flag crystal (make sure we're in a well-type area)   *
*         Use ACT flags to determine the TYPE of crystal you going to load... *
*         Between ruby_fragment, sapphire_fragment, emerald_fragment, diamond *
*         fragment.  (From common to rare)  Also a option for any_fragment.   *
*         Find level of corpse.  Find # of fragments and automatically split  *
*         them amongst the group.  Reduce the load # for any_frament IMHO...  *
*                                                    *
*******************************************************************************/

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

//headers added in locally here.
int fragmentSplit( CHAR_DATA *ch,  int fragmentNumber );
bool is_frag_level_ready( CHAR_DATA *ch );
void purchase_stat( CHAR_DATA *ch, int stat );
void discover_study_strength( CHAR_DATA *ch );
void discover_study_dexterity( CHAR_DATA *ch );
void discover_study_wisdom( CHAR_DATA *ch );
void discover_study_constitution( CHAR_DATA *ch );
void discover_study_intelligence( CHAR_DATA *ch );
void study_listing( CHAR_DATA *ch );

/*
 * Return true if player is ready to level.
 */

bool is_frag_level_ready(CHAR_DATA *ch)
{
  if ((ch->level >= 80 && ch->ruby_fragment >= 50000)
  || ( ch->level >= 85 && ch->ruby_fragment >= 75000)
  || ( ch->level >= 90 && ch->ruby_fragment >= 100000)
  || ( ch->level >= 95 && ch->ruby_fragment >= 250000))
    return TRUE;
  return FALSE;
}

void purchase_stat( CHAR_DATA *ch, int stat )
{
    char str_buf[MSL], buf[MSL];

    if ( ch->perm_stat[stat] < get_max_train( ch, stat ) )
    {
        send_to_char( "You must max your racial strength first.\n\r", ch );
        return;
    }

    if ( stat == 0 )
        sprintf( buf, "%s", "strength" );
    else if ( stat == 1 )
        sprintf( buf, "%s", "intelligence" );
    else if ( stat == 2 )
        sprintf( buf, "%s", "wisdom" );
    else if ( stat == 3 )
        sprintf( buf, "%s", "dexterity" );
    else if ( stat == 4 )
        sprintf( buf, "%s", "constitution" );

    if ( ch->perm_stat[stat] == get_max_train( ch, stat ) )
    {
        if ( ch->ruby_counter >= 1 )
        {
            ch->ruby_counter--;
            ch->perm_stat[stat]++;
            ch->first_frag_level++;
            printf_to_char( ch, "You study %s!\n\r", buf );
            mprintf(sizeof(str_buf),str_buf,"I just gained %d %s!{x",
                ch->perm_stat[stat], buf );
            do_function( ch, &do_info, str_buf );
        }
        else
            printf_to_char( ch, "You are not ready for the next level of %s.\n\r", buf );

        return;
    }
    else if ( ch->perm_stat[stat] == get_stage1_max_train( ch, stat ) )
    {
        if ( ch->ruby_counter >= 1
        &&   ch->sapphire_counter >= 1 )
        {
            ch->ruby_counter--;
            ch->sapphire_counter--;
            ch->perm_stat[stat]++;
            ch->second_frag_level++;
            printf_to_char( ch, "You study %s!\n\r", buf );
            mprintf(sizeof(str_buf),str_buf,"I just gained %d %s!{x\n\r",
                ch->perm_stat[stat], buf );
            do_function( ch, &do_info, str_buf);
        }
        else
            send_to_char( "You are not ready for that.\n\r", ch );
    }
    else if ( ch->perm_stat[stat] == get_stage2_max_train( ch, stat ) )
    {
        if ( ch->ruby_counter > 0
        &&   ch->sapphire_counter > 0
        &&   ch->emerald_counter > 0 )
        {
          //They have enough counters, but how many lvl 3 purchases?
          if ( ch->third_frag_level > 2 )
          {
            send_to_char( "Sorry, you can only study three stats to +3.\n\r", ch );
            return;
          }

          ch->ruby_counter--;
          ch->sapphire_counter--;
          ch->emerald_counter--;
          ch->perm_stat[stat]++;
          ch->third_frag_level++;
          printf_to_char( ch, "You study %s!\n\r", buf );
          mprintf(sizeof(str_buf),str_buf,"I just gained %d %s!{x\n\r",
            ch->perm_stat[stat], buf );
          do_function( ch, &do_info, str_buf);
        }
        else
          send_to_char( "You are not ready for that.\n\r", ch );
    }
    else if ( ch->perm_stat[stat] == get_stage3_max_train( ch, stat ) )
    {
        if ( ch->ruby_counter > 0
        &&   ch->sapphire_counter > 0
        &&   ch->emerald_counter > 0
        &&   ch->diamond_counter > 0 )
        {
          //They have enough counters, but how many lvl 4 purchases?
          if ( ch->fourth_frag_level > 1 )
          {
            send_to_char( "Sorry, you can only study two stats to +4.\n\r", ch );
            return;
          }

          ch->ruby_counter--;
          ch->sapphire_counter--;
          ch->emerald_counter--;
          ch->diamond_counter--;
          ch->perm_stat[stat]++;
          ch->fourth_frag_level++;
          printf_to_char( ch, "You study %s!\n\r", buf );
          mprintf(sizeof(str_buf),str_buf,"I just gained %d %s!{x\n\r",
            ch->perm_stat[stat], buf );
          do_function( ch, &do_info, str_buf);
        }
        else
          send_to_char( "You are not ready for that.\n\r", ch );
    }
    else
        send_to_char( "You have studied that as much as possible!\n\r", ch );

    return;
}

/* 
 * Study listing code
 */
void study_listing( CHAR_DATA *ch )
{
/*
  int level_needed = ch->level + 1;
  int frag_level_cost=0;

  if (level_needed >=80 && level_needed <= 85)
          frag_level_cost = 50000;
  else if (level_needed > 85 && level_needed <= 90 )
          frag_level_cost = 75000;
  else if (level_needed > 90 && level_needed <= 95)
          frag_level_cost = 100000;
  else if (level_needed > 95 && level_needed < 100)
          frag_level_cost = 250000;

  if (ch->level < LEVEL_HERO && ch->level >= 80 )
  {
    printf_to_char( ch, "For level %d you will need %d Ruby Fragments.\n\r\n\r", level_needed, frag_level_cost);
  }
*/

  send_to_char( "{WRequirements:{x\n\r\n\r", ch );
        send_to_char( "{gPurchase     Purchase  Cost{x\n\r", ch );
        send_to_char( "{G========     ========  ===={x\n\r", ch );
  
    //Strength
    if ( ch->perm_stat[STAT_STR] == get_max_train( ch, STAT_STR ) )
        printf_to_char( ch, "Strength     %-2d        {rR{Ru{rby{x\n\r",
            ch->perm_stat[STAT_STR] + 1);
    else if ( ch->perm_stat[STAT_STR] == get_stage1_max_train( ch, STAT_STR ) && ch->level >= 85)
        printf_to_char( ch, "Strength     %-2d        {rR{Ru{rby{x/{cSap{Bp{bhi{cre{x\n\r",
            ch->perm_stat[STAT_STR] + 1);
    else if ( ch->perm_stat[STAT_STR] == get_stage2_max_train( ch, STAT_STR ) && ch->level >= 90)
    {
      if ( ch->third_frag_level > 2 )
        send_to_char( "Strength     Max\n\r", ch );
      else
        printf_to_char( ch, "Strength     %-2d        {rR{Ru{rby{x/{cSap{Bp{bhi{cre{x/{gE{Gme{gr{Ga{gld{x\n\r",
            ch->perm_stat[STAT_STR] + 1);
    }
    else if ( ch->perm_stat[STAT_STR] == get_stage3_max_train( ch, STAT_STR ) && ch->level >= 95)
    {
      if ( ch->fourth_frag_level > 1 )
        send_to_char( "Strength     Max\n\r", ch );
      else
        printf_to_char( ch, "Strength     %-2d        {rR{Ru{rby{x/{cSap{Bp{bhi{cre{x/{gE{Gme{gr{Ga{gld{x/{CDi{cam{Wo{wnd{x\n\r",
            ch->perm_stat[STAT_STR] + 1);
    }
    else
        printf_to_char( ch, "Strength     Max\n\r" );

    //Dexterity
    if ( ch->perm_stat[STAT_DEX] == get_max_train( ch, STAT_DEX ) )
        printf_to_char( ch, "Dexterity    %-2d        {rR{Ru{rby{x\n\r",
            ch->perm_stat[STAT_DEX] + 1);
    else if ( ch->perm_stat[STAT_DEX] == get_stage1_max_train( ch, STAT_DEX ) && ch->level >= 85)
        printf_to_char( ch, "Dexterity    %-2d        {rR{Rub{ry{x/{cSap{Bp{bhi{cre{x\n\r",
            ch->perm_stat[STAT_DEX] + 1);
    else if ( ch->perm_stat[STAT_DEX] == get_stage2_max_train( ch, STAT_DEX ) && ch->level >= 90)
    {
      if ( ch->third_frag_level > 2 )
        send_to_char( "Dexterity    Max\n\r", ch );
      else
        printf_to_char( ch, "Dexterity    %-2d        {rR{Rub{ry{x/{cSap{Bp{bhi{cre{x{x/{gE{Gme{gr{Ga{gld{x\n\r",
            ch->perm_stat[STAT_DEX] + 1);
    }
    else if ( ch->perm_stat[STAT_DEX] == get_stage3_max_train( ch, STAT_DEX ) && ch->level >= 95)
    {
      if ( ch->fourth_frag_level > 1 )
        send_to_char( "Dexterity    Max\n\r", ch );
      else
        printf_to_char( ch, "Dexterity    %-2d        {rR{Rub{ry{x/{cSap{Bp{bhi{cre{x{x/{gE{Gme{gr{Ga{gld{x/{CDi{cam{Wo{wnd{x\n\r",
            ch->perm_stat[STAT_DEX] + 1);
    }
    else
        printf_to_char( ch, "Dexterity    Max\n\r" );

    //Wisdom
    if ( ch->perm_stat[STAT_WIS] == get_max_train( ch, STAT_WIS ) )
        printf_to_char( ch, "Wisdom       %-2d        {rR{Ru{rby{x\n\r",
            ch->perm_stat[STAT_WIS] + 1);
    else if ( ch->perm_stat[STAT_WIS] == get_stage1_max_train( ch, STAT_WIS ) && ch->level >= 85)
        printf_to_char( ch, "Wisdom       %-2d        {rR{Ru{rby{w/{cSap{Bp{bhi{cre{x\n\r",
            ch->perm_stat[STAT_WIS] + 1);
    else if ( ch->perm_stat[STAT_WIS] == get_stage2_max_train( ch, STAT_WIS ) && ch->level >= 90)
    {
      if ( ch->third_frag_level > 2 )
        send_to_char( "Wisdom       Max\n\r", ch );
      else
        printf_to_char( ch, "Wisdom       %-2d        {rR{Ru{rby{w/{cSap{Bp{bhi{cre{x/{gE{Gme{gr{Ga{gld{x\n\r",
            ch->perm_stat[STAT_WIS] + 1);
    }
    else if ( ch->perm_stat[STAT_WIS] == get_stage3_max_train( ch, STAT_WIS ) && ch->level >= 95)
    {
      if ( ch->fourth_frag_level > 1 )
        send_to_char( "Wisdom       Max\n\r", ch );
      else
        printf_to_char( ch, "Wisdom       %-2d        {rR{Ru{rby{w/{cSap{Bp{bhi{cre{x/{gE{Gme{gr{Ga{gld{x/{CDi{cam{Wo{wnd{x\n\r",
            ch->perm_stat[STAT_WIS] + 1);
    }
    else
        printf_to_char( ch, "Wisdom       Max\n\r" );

    //Intelligence
    if ( ch->perm_stat[STAT_INT] == get_max_train( ch, STAT_INT ) )
        printf_to_char( ch, "Intelligence %-2d        {rR{Ru{rby{x\n\r",
            ch->perm_stat[STAT_INT] + 1);
    else if ( ch->perm_stat[STAT_INT] == get_stage1_max_train( ch, STAT_INT ) && ch->level >= 85)
        printf_to_char( ch, "Intelligence %-2d        {rR{Ru{rby{x/{cSap{Bp{bhi{cre{x\n\r",
            ch->perm_stat[STAT_INT] + 1);
    else if ( ch->perm_stat[STAT_INT] == get_stage2_max_train( ch, STAT_INT ) && ch->level >= 90)
    {
      if ( ch->third_frag_level > 2 )
        send_to_char( "Intelligence Max\n\r", ch );
      else
        printf_to_char( ch, "Intelligence %-2d        {rR{Ru{rby{x/{cSap{Bp{bhi{cre{x/{gE{Gme{gr{Ga{gld{x\n\r",
            ch->perm_stat[STAT_INT] + 1);
    }
    else if ( ch->perm_stat[STAT_INT] == get_stage3_max_train( ch, STAT_INT ) && ch->level >= 95)
    {
      if ( ch->fourth_frag_level > 1 )
        send_to_char( "Intelligence Max\n\r", ch );
      else
        printf_to_char( ch, "Intelligence %-2d        {rR{Ru{rby{x/{cSap{Bp{bhi{cre{x/{gE{Gme{gr{Ga{gld{x/{CDi{cam{Wo{wnd{x\n\r",
            ch->perm_stat[STAT_INT] + 1);
    }
    else
        printf_to_char( ch, "Intelligence Max\n\r" );

    //Constitution
    if ( ch->perm_stat[STAT_CON] == get_max_train( ch, STAT_CON ) )
      printf_to_char( ch, "Constitution %-2d        {rR{Ru{rby{x\n\r",
          ch->perm_stat[STAT_CON] + 1);
    else if ( ch->perm_stat[STAT_CON] == get_stage1_max_train( ch, STAT_CON ) && ch->level >= 85)
      printf_to_char( ch, "Constitution %-2d        {rR{Ru{rby{x/{cSap{Bp{bhi{cre{x\n\r",
        ch->perm_stat[STAT_CON] + 1);
    else if ( ch->perm_stat[STAT_CON] == get_stage2_max_train( ch, STAT_CON ) && ch->level >= 90)
    {
      if ( ch->third_frag_level > 2 )
        send_to_char( "Constitution Max\n\r", ch );
      else
        printf_to_char( ch, "Constitution %-2d        {rR{Ru{rby{x/{cSap{Bp{bhi{cre{x/{gE{Gme{gr{Ga{gld{x\n\r",
          ch->perm_stat[STAT_CON] + 1);
    }
    else if ( ch->perm_stat[STAT_CON] == get_stage3_max_train( ch, STAT_CON ) && ch->level >= 95)
    {
      if ( ch->fourth_frag_level > 1 )
        send_to_char( "Constitution Max\n\r", ch );
      else
        printf_to_char( ch, "Constitution %-2d        {rR{Ru{rby{x/{cSap{Bp{bhi{cre{x/{gE{Gme{gr{Ga{gld{x/{CDi{cam{Wo{wnd{x\n\r",
          ch->perm_stat[STAT_CON] + 1);
    }
    else
      printf_to_char( ch, "Constitution Max\n\r" );
}

/*
 * Auto-Split fragment code.
 */
int fragmentSplit( CHAR_DATA *ch, int fragmentNumber )
{
  CHAR_DATA *gch;
  int members;
  int fragment_share, fragment_extra, fragment_group=0;

  members = 0;
  for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
  {
    if ( is_same_group( gch, ch )
    && !IS_AFFECTED( gch, AFF_CHARM )
    && !IS_NPC(gch)) //verify NO pets or mobs...
      members++;
  }

  if ( members >= 5 )
    fragment_group = fragmentNumber * 2;
  else
    fragment_group = fragmentNumber;
  
  fragment_share = fragment_group / members;
  fragment_extra = fragment_group % members;
  
  //fragment_final = fragment_share + fragment_extra;
  for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
  {
    if ( gch == ch )
      fragment_group = fragment_share + fragment_extra;
    if ( gch != ch && is_same_group( gch, ch ) && !IS_AFFECTED( gch, AFF_CHARM ))
      fragment_group = fragment_share;
  }

  return fragment_group;
}    

/*
 * do_cleanse on a corpse to dertermine what type of fragments corpse will have.
 * check for !is_here and inform of lost frags to that group mem.
 */
void do_cleanse( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int silver;
  char buf[MSL];
  char buf2[MSL];
  int fragment_final;

  /* variables for AUTOSPLIT */
  CHAR_DATA *gch;
  int members;
  char buffer[100];

  if (IS_NPC(ch))
    return;

  if (!str_cmp( argument, "fragxp" ) )
  {
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
      if ( IS_NPC(gch) && IS_SET(gch->act2, ACT2_FRAG_SELL) )
        break;
    } 

    if (gch == NULL)
      send_to_char( "You can't do that here.\n\r", ch );
    else
    {
      if ( IS_SET( ch->plr2, PLR2_NO_FRAGXP ) )
      {
        send_to_char( "You will now receive experience for fragment mob kills.\n\r", ch );
        REMOVE_BIT( ch->plr2, PLR2_NO_FRAGXP );
      }
      else
      {
        send_to_char( "You will no longer receive experience for fragment mob kills.\n\r", ch );
        SET_BIT( ch->plr2, PLR2_NO_FRAGXP );
      }
    }

    return;
  }

  one_argument( argument, arg );

  //check the room's contents (looking for a corpse only..)
  obj = get_obj_list( ch, arg, ch->in_room->contents );

  //nuthing found
  if ( obj == NULL )
  {
    send_to_char( "You can't find it.\n\r", ch );
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("Cleansing is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  /*
   * If just a corpse, bug out, no need to go on..
   * Don't allow the cleanse if the object contains anything unless it is an NPC
   * corpse.  Just to be safe...
   */
  if ( is_name( obj->name, "corpse" ))
  {
      send_to_char("This corpse cannot be cleansed.\n\r", ch );
      return;
  }

  //Never on a players corpse.
  if ( obj->item_type == ITEM_CORPSE_PC )
  {
    act( "I don't think $g would approve.", ch, NULL, NULL, TO_CHAR );
    return;
  }

  if ( obj->item_type != ITEM_CORPSE_NPC )
  {
    send_to_char( "This can only be done on corpses, try sacrificing it.\n\r", ch );
      return;
  }

  if ( !IS_SET( ch->in_room->area->flags, AREA_CRYSTAL ) )
  {
    send_to_char( "You cannot do that here.\n\r", ch );
    return;
  }

  if ( obj->contains)
  {
      act( "$g thinks that would be unwise.", ch, NULL, NULL, TO_CHAR );
      return;
  }

  one_argument( argument, arg );

  //cleanse alone only shows this, doesn't really do anything as you can't cleanse yourself.
  if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
      act( "$n attempted to cleanse $mself, but $g doesn't allow that.",
           ch, NULL, NULL, TO_ROOM );
      act( "I don't think $g would approve, try it on a corpse.", ch, NULL, NULL, TO_CHAR );
      return;
    }

  //If its not in the room cause someone else picked it up.
  if (obj->in_room != NULL)
  {
      for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
        if (gch->on == obj)
        {
            act("$N picked up $p.",
                ch,obj,gch,TO_CHAR);
            return;
        }
  }

  //Go ahead and give them some silver like sacrifice, because this kinda IS the new sacrifice.
  silver = UMAX(1,obj->level * 3);

  if (obj->item_type != ITEM_CORPSE_NPC
  &&  obj->item_type != ITEM_CORPSE_PC)
    silver = UMIN(silver,obj->cost);


  if ( ch->clan )
  {
    if ( silver == 1 )
        act( "$g gives you one si{Wl{Dv{wer{x coin for the cleanse.", ch, NULL, NULL, TO_CHAR );
    else
        printf_to_char( ch,
            "%s gives you %d si{Wl{Dv{wer{x coins for your cleanse.\n\r",
                ch->clan->clan_immortal, silver );
  }
  else
  {
    if (silver == 1)
        send_to_char(
            "{CM{cir{Ml{mya{x gives you one si{Wl{Dv{wer{x coin for this cleanse.\n\r", ch );
    else
      printf_to_char(ch,"{CM{cir{Ml{mya{x gives you %d si{Wl{Dv{wer{x coins for your cleanse.\n\r",
                     silver);
  }
  
  ch->silver += silver;

  members = 0;
  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
  {
    if ( is_same_group( gch, ch )
    &&   !IS_NPC( gch ) )
      members++;
  }

  if (IS_SET(ch->act,PLR_AUTOSPLIT) )
  { /* AUTOSPLIT code */

    if ( members > 1 && silver > 1)
    {
      mprintf(sizeof(buffer), buffer,"%d",silver);
      do_function(ch, &do_split,buffer);
    }
  }

  act( "$n performs a ritual and cleanses $p.", ch, obj, NULL, TO_ROOM );
 
  //Ruby Corpses
  if ( is_name( obj->name, "ruby corpse" ))
  {
    int fragmentNumber;
  
    fragmentNumber = obj->value[0];
    fragmentNumber = fragmentNumber * wis_app[get_curr_stat( ch, STAT_WIS )].frag_percent / 100;
  
    if ( ch->level > 90  && obj->level < 80 )
      fragmentNumber = fragmentNumber / 2;

    if ( ch->level >= 100 && obj->level < 90 )
      fragmentNumber = fragmentNumber - (fragmentNumber / 4);

    fragment_final = fragmentSplit( ch, fragmentNumber );
    mprintf( sizeof(buf), buf,
      "$n pulls a set {rr{Ru{rby{x fragment essences from the corpse giving you %d, which fuse with your body.\n\r",
        fragment_final);

    for( gch = ch->in_room->people ; gch != NULL ; gch = gch->next_in_room )
    {
      if ( gch != ch && is_same_group( gch, ch ) && !IS_AFFECTED( gch, AFF_CHARM ) && !IS_NPC(gch) )
      {
        act( buf, ch, NULL, gch, TO_VICT );
        gch->ruby_fragment += fragment_final;

        if ( gch->ruby_fragment > 250000 )
        {
           gch->ruby_counter += 1;
           gch->ruby_fragment -= 250000;
        }
      }
    }

    printf_to_char( ch,
        "You cleanse a corpse and fuse with %d {rr{Ru{rby{x fragment essences.\n\r",
            fragment_final );
    ch->ruby_fragment += fragment_final;

    if ( members > 1 )
    {
      sprintf( buf2,
        "{y%s{x cleansed a {rR{Ru{rby{x corpse for %d fragments, which was shared by %d group members (%d).",
        ch->name, fragmentNumber, members, ch->ruby_fragment );
      wiznet( buf2, NULL, NULL, WIZ_CLEANSE, 0, get_trust(ch) );
    }
    else
    {
      sprintf( buf2, "{y%s{x cleansed a {rR{Ru{rby{x corpse for %d fragments (%d).",
        ch->name, fragment_final, ch->ruby_fragment );
      wiznet( buf2, NULL, NULL, WIZ_CLEANSE, 0, get_trust(ch) );
    }


    if ( ch->ruby_fragment > 250000 )
    {
        ch->ruby_counter += 1;
        ch->ruby_fragment -= 250000;
    }
  }
  
  //Sapphire Corpses
  if ( is_name( obj->name, "sapphire corpse" ))
  {
    int fragmentNumber;

    fragmentNumber = obj->value[0];
  fragmentNumber = fragmentNumber * wis_app[get_curr_stat( ch, STAT_WIS )].frag_percent / 100;

    if ( ch->level > 90  && obj->level < 80 )
        fragmentNumber = fragmentNumber / 2;

    if ( ch->level >= 100 && obj->level < 90 )
        fragmentNumber = fragmentNumber - (fragmentNumber / 4);

    fragment_final = fragmentSplit( ch, fragmentNumber );
    mprintf( sizeof(buf), buf,
        "$n pulls a set of {csap{Bp{bhi{cre{x fragment essences from the corpse giving you %d, which fuse with your body.\n\r",
            fragment_final);

    for( gch = ch->in_room->people ; gch != NULL ; gch = gch->next_in_room )
    {
        if ( gch != ch && is_same_group( gch, ch ) && !IS_AFFECTED( gch, AFF_CHARM ) && !IS_NPC(gch))
        {
            act( buf, ch, NULL, gch, TO_VICT );
            gch->sapphire_fragment += fragment_final;
            if ( gch->sapphire_fragment > 200000 )
            {
                gch->sapphire_counter++;
                gch->sapphire_fragment -= 200000;
            }
       }
    }

    printf_to_char( ch,
      "You cleanse a corpse and fuse with %d {csap{Bp{bhi{cre{x fragment essences.\n\r",
    fragment_final );
    ch->sapphire_fragment += fragment_final;

    if ( members > 1 )
    {
      sprintf( buf2,
        "{y%s{x cleansed a {cSap{Bp{bhi{cre{x corpse for %d fragments, which was shared by %d group members (%d).",
        ch->name, fragmentNumber, members, ch->sapphire_fragment );
      wiznet( buf2, NULL, NULL, WIZ_CLEANSE, 0, get_trust(ch) );
    }
    else
    {
      sprintf( buf2, "{y%s{x cleansed a {cSap{Bp{bhi{cre{x corpse for %d fragments (%d).",
        ch->name, fragment_final, ch->sapphire_fragment );
      wiznet( buf2, NULL, NULL, WIZ_CLEANSE, 0, get_trust(ch) );
    }

    if ( ch->sapphire_fragment > 200000 )
    {
        ch->sapphire_counter += 1;
        ch->sapphire_fragment -= 200000;
    }
  }

  //Emerald Corpses
  if ( is_name( obj->name, "emerald corpse" ))
  {
    int fragmentNumber;

    fragmentNumber = obj->value[0];
    fragmentNumber = fragmentNumber * wis_app[get_curr_stat( ch, STAT_WIS )].frag_percent / 100;

    if ( ch->level > 90  && obj->level < 80 )
        fragmentNumber = fragmentNumber / 2;

    if ( ch->level >= 100 && obj->level < 90 )
        fragmentNumber = fragmentNumber - (fragmentNumber / 4);

    fragment_final = fragmentSplit( ch, fragmentNumber );
    mprintf( sizeof(buf), buf,
        "$n pulls a set of {ge{Gme{gr{Ga{gld{x fragment essences from the corpse giving you %d, which fuse with your body.\n\r",
            fragment_final);

    for( gch = ch->in_room->people ; gch != NULL ; gch = gch->next_in_room )
    {
        if ( gch != ch && is_same_group( gch, ch ) && !IS_AFFECTED( gch, AFF_CHARM ) && !IS_NPC(gch))
        {
            act( buf, ch, NULL, gch, TO_VICT );
            gch->emerald_fragment += fragment_final;

            if ( gch->emerald_fragment > 150000 )
            {
                gch->emerald_counter += 1;
                gch->emerald_fragment -= 150000;
            }
        }
    }

    printf_to_char( ch, "You cleanse a corpse and fuse with %d {ge{Gme{gr{Ga{gld{x fragment essences.\n\r",
        fragment_final );
    ch->emerald_fragment += fragment_final;

    if ( members > 1 )
    {
      sprintf( buf2,
        "{y%s{x cleansed a {gE{Gme{gr{Ga{gld{x corpse for %d fragments, which was shared by %d group members (%d).",
        ch->name, fragmentNumber, members, ch->emerald_fragment );
      wiznet( buf2, NULL, NULL, WIZ_CLEANSE, 0, get_trust(ch) );
    }
    else
    {
      sprintf( buf2, "{y%s{x cleansed a {gE{Gme{gr{Ga{gld{x corpse for %d fragments (%d).",
        ch->name, fragment_final, ch->emerald_fragment );
      wiznet( buf2, NULL, NULL, WIZ_CLEANSE, 0, get_trust(ch) );
    }

    if ( ch->emerald_fragment > 150000 )
    {
        ch->emerald_counter += 1;
        ch->emerald_fragment -= 150000;
    }
  }

  //Diamond Corpses
  if ( is_name( obj->name, "diamond corpse" ))
  {
    int fragmentNumber;

    fragmentNumber = obj->value[0];
    fragmentNumber = fragmentNumber * wis_app[get_curr_stat( ch, STAT_WIS )].frag_percent / 100;

    if ( ch->level > 90  && obj->level < 80 )
        fragmentNumber = fragmentNumber / 2;

    if ( ch->level >= 100 && obj->level < 90 )
        fragmentNumber = fragmentNumber - (fragmentNumber / 4);

    fragment_final = fragmentSplit( ch, fragmentNumber );
    mprintf( sizeof(buf), buf,
        "$n pulls a set of {Cdi{ca{Wm{wond{x fragment essences from the corpse giving you %d, which fuse with your body.\n\r",
            fragment_final);

    for( gch = ch->in_room->people ; gch != NULL ; gch = gch->next_in_room )
    {
        if ( gch != ch && is_same_group( gch, ch ) && !IS_AFFECTED( gch, AFF_CHARM ) && !IS_NPC(gch))
        {
            act( buf, ch, NULL, gch, TO_VICT );
            gch->diamond_fragment += fragment_final;

            if ( gch->diamond_fragment > 100000 )
            {
                gch->diamond_counter += 1;
                gch->diamond_fragment -= 100000;
            }
        }
    }

    printf_to_char( ch, "You cleanse a corpse and fuse with %d {Cdi{ca{Wm{wond{x fragment essences.\n\r",
        fragment_final );
    ch->diamond_fragment += fragment_final;

    if ( members > 1 )
    {
      sprintf( buf2,
        "{y%s{x cleansed a {Cdi{ca{Wm{wond{x corpse for %d fragments, which was shared by %d group members (%d).",
        ch->name, fragmentNumber, members, ch->diamond_fragment );
      wiznet( buf2, NULL, NULL, WIZ_CLEANSE, 0, get_trust(ch) );
    }
    else
    {
      sprintf( buf2, "{y%s{x cleansed a {Cdi{ca{Wm{wond{x corpse for %d fragments (%d).",
        ch->name, fragment_final, ch->diamond_fragment );
      wiznet( buf2, NULL, NULL, WIZ_CLEANSE, 0, get_trust(ch) );
    }

    if ( ch->diamond_fragment > 100000 )
    {
        ch->diamond_counter += 1;
        ch->diamond_fragment -= 100000;
    }
  }

  //Make the corpse disappear after the cleanse.
  extract_obj( obj );
  return;
}

void do_study( CHAR_DATA *ch, char *argument )
{
  //char buf[MSL];
  //char str_buf[MSL];
  CHAR_DATA *mob;
  //int frag_level_cost =0, level_needed=0;

  if ( IS_NPC(ch) )
    return;

  if (IS_GHOST(ch)) 
  {
    send_to_char("Studying is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( !IS_AWAKE(ch) )
  {
    send_to_char( "In your dreams, or what?\n\r", ch );
    return;
  }

  if ( ch->level < 80 )
  {
    send_to_char( "You can't figure out what you should be studying.\n\r", ch );
    return;
  }

  for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
  {
    if ( IS_NPC(mob) && IS_SET(mob->act2, ACT2_FRAG_SELL) )
    break;
  } 

  if ( mob == NULL )
  {
    send_to_char( "You can't do that here.\n\r", ch );
    return;
  }

  if ( IS_IMMORTAL(ch) )
  {
    send_to_char( "Immortals should use set, not study.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  {
    study_listing( ch );
    return;
  }

  if ( !str_prefix( argument, "list" ) )
  {
    study_listing( ch );
    return;
  }

  if ( !str_prefix( argument, "strength" ) )
  {
    purchase_stat( ch, STAT_STR );
    return;
  }

  if ( !str_prefix( argument, "dexterity" ) )
  {
    purchase_stat( ch, STAT_DEX );
    return;
  }

  if ( !str_prefix( argument, "intelligence" ) )
  {
    purchase_stat( ch, STAT_INT );
    return;
  }

  if ( !str_prefix( argument, "wisdom" ) )
  {
    purchase_stat( ch, STAT_WIS );
    return;
  }

  if ( !str_prefix( argument, "constitution" ) )
  {
    purchase_stat( ch, STAT_CON );
    return;
  }

  send_to_char(
    "syntax: study <strength|intelligence|constitution|wisdom|dexterity>\n\r"
    "        study <list>\n\r", ch );

  return;
}
