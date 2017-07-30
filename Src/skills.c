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
*   ROM 2.4 is copyright 1993-1996 Russ Taylor                             *
*   ROM has been brought to you by the ROM consortium                      *
*       Russ Taylor (rtaylor@efn.org)                                      *
*       Gabrielle Taylor                                                   *
*       Brian Moore (zump@rom.org)                                         *
*   By using this code, you have agreed to follow the terms of the         *
*   ROM license, in the file Rom24/doc/rom.license                         *
***************************************************************************/

/***************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by              *
 *      Chris Litchfield and Mark Archambault                              *
 *      Sacred has been created with much time and effort from many        *
 *      different people's input and ideas.                                *
 *      By using this code, you have agreed to follow the terms of the     *
 *      Sacred license, in the file doc/sacred.license                     *
\***************************************************************************/

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

enum list_setting {list_group_costs_e, list_group_chosen_e, list_gain_list_e};
void list_listing(CHAR_DATA *ch,enum list_setting setting);

int TNL( int XPL, int level )
{
  if ( level ) return (int)( XPL * ( pow( 1.005, level ) -1 ) / 0.005 );
  return 0;
}

/* used to get new skills */
void do_gain( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *trainer;
  bool knowitall = TRUE;
  int gn = 0, sn = 0, skn = 0;
  char buf[MSL];

  if (IS_NPC(ch))
    return;

  if (IS_GHOST(ch))
  {
    send_to_char("Gaining new skills is useless since you are still {rDEAD{x!!\n\r", ch);
    return;
  }

  /* find a trainer */
  for ( trainer = ch->in_room->people; trainer;
        trainer = trainer->next_in_room )

    if ( IS_NPC( trainer ) && IS_SET( trainer->act, ACT_GAIN ) )
      break;

  if ( trainer == NULL || !can_see( ch, trainer ) )
  {
    send_to_char( "You can't do that here.\n\r", ch );
    return;
  }

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    list_listing( ch, list_gain_list_e );
    return;
  }

  if ( !str_prefix( arg, "list" ) )
  {
    list_listing( ch, list_gain_list_e );
    return;
  }

  if ( !str_prefix( arg, "convert" ) )
  {
    if (ch->practice < 10)
    {
      act("$N tells you 'You are not yet ready.'",
          ch,NULL,trainer,TO_CHAR);
      return;
    }

    act( "$N helps you apply your practice to training.",
         ch, NULL, trainer, TO_CHAR );
    ch->practice -= 10;
    ch->train +=1;

    mprintf(sizeof(buf), buf,
            "{g%s{w converted 10 pracs to 1 train leaving {g%d{wP {D%d{wT{x.",
            ch->name, ch->practice, ch->train );
    log_string( buf );
    wiznet( buf, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );
    return;
  }
  if (!str_prefix(arg,"transfer"))
  {
    if (ch->train <= 0)
    {
      act("$N tells you 'You are not yet ready.'",
          ch,NULL,trainer,TO_CHAR);
      return;
    }
    act( "$N helps you apply your training to practice.",
         ch, NULL, trainer, TO_CHAR );
    ch->practice += 10;
    ch->train -=1;

    mprintf(sizeof(buf), buf,
            "{g%s{w converted 1 train to 10 pracs leaving {g%d{wP {D%d{wT{x.",
            ch->name, ch->practice, ch->train );
    log_string( buf );
    wiznet( buf, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );
    return;
  }

  if (!str_prefix(arg,"points"))
  {
    if (ch->train < 2)
    {
      act("$N tells you 'You are not yet ready.'",
          ch, NULL, trainer, TO_CHAR );
      return;
    }

    if ( ch->pcdata->points <= 40 )
    {
      act("$N tells you 'There would be no point in that.'",
          ch, NULL, trainer, TO_CHAR );
      return;
    }

    act( "$N trains you, and you feel more at ease with your skills.",
         ch, NULL, trainer, TO_CHAR );

    /* Lets try to keep their previous exp_needed when
     * when they train down CP whenever possible. We can
     * default it to the max per level if new # is above per level.
     */
    ch->train -= 2;
    int prev_needed;
    int XPL;
    char log_buf[MSL];

    XPL = exp_per_level( ch, ch->pcdata->points );
    prev_needed = TNL( XPL, ch->level ) - ch->exp;

    ch->pcdata->points -= 1;

    /* We must reset XPL with the new CP's.. */
    XPL = exp_per_level( ch, ch->pcdata->points );

    /* Make sure that they don't go over the max per level */
    if ( prev_needed >
         ( TNL( XPL, ch->level ) - TNL( XPL, ch->level - 1  ) ) )
      ch->exp = TNL( XPL, ch->level - 1 );
    else
      ch->exp = TNL( XPL, ch->level ) - prev_needed;

    mprintf(sizeof(log_buf), log_buf,
            "{g%s{w just used {D2{w trains to lower CP one point to %d.",
            ch->name, ch->pcdata->points );
    log_string( log_buf );
    wiznet( log_buf, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );

    return;
  }

  /* else add a group/skill */

  gn = group_lookup( argument );
  if ( gn > 0 )
  {
    if ( ch->pcdata->group_known[gn] )
    {
      act( "$N tells you 'You already know that group!'",
           ch,NULL,trainer,TO_CHAR);
      return;
    }

    if ( group_table[gn].rating[ch->gameclass] <= 0 )
    {
      act("$N tells you 'That group is beyond your powers.'",
          ch, NULL, trainer, TO_CHAR );
      return;
    }

    if ( ch->train < group_table[gn].rating[ch->gameclass] )
    {
      act("$N tells you 'You are not yet ready for that group.'",
          ch,NULL,trainer,TO_CHAR);
      return;
    }

    /* add the group */
    gn_add( ch, gn );
    act( "$N teaches you the art of $t.",
         ch, group_table[gn].name, trainer, TO_CHAR );
    ch->train -= group_table[gn].rating[ch->gameclass];
    mprintf(sizeof(buf), buf,
            "{g%s{w used {D%d{x trains on {B%s{w leaving {g%d{w trains{x.",
            ch->name, group_table[gn].rating[ch->gameclass],
            group_table[gn].name, ch->train );
    log_string( buf );
    wiznet( buf, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );

    for ( skn = 0; skn < MAX_SKILL; skn++ )
    {
      if ( skill_table[skn].name == NULL ) continue;

      if ( ( skill_table[skn].skill_level[ch->gameclass] < 0 )
           &&   !is_racial_skill( ch,skn ) ) continue;

      if (ch->pcdata->learned[skn] < 1)
      {
        knowitall = FALSE;
        break;
      }
    }

    if (knowitall)
      do_function( ch, &do_info, "I now know all spells and skills for my class!" );



    return;
  }

  sn = skill_lookup( argument );
  if ( sn > -1 )
  {
    if (skill_table[sn].spell_fun != spell_null)
    {
      act("$N tells you 'You must learn the full group.'",
          ch,NULL,trainer,TO_CHAR);
      return;
    }


    if (ch->pcdata->learned[sn])
    {
      act("$N tells you 'You already know that skill!'",
          ch,NULL,trainer,TO_CHAR);
      return;
    }

    if (skill_table[sn].rating[ch->gameclass] <= 0)
    {
      act("$N tells you 'That skill is beyond your powers.'",
          ch,NULL,trainer,TO_CHAR);
      return;
    }

    if (ch->train < skill_table[sn].rating[ch->gameclass])
    {
      act("$N tells you 'You are not yet ready for that skill.'",
          ch,NULL,trainer,TO_CHAR);
      return;
    }

    /* add the skill */
    ch->pcdata->learned[sn] = 1;
    act("$N teaches you the art of $t.",
        ch,skill_table[sn].name,trainer,TO_CHAR);
    ch->train -= skill_table[sn].rating[ch->gameclass];
    mprintf(sizeof(buf), buf,"{g%s{w used {D%d{x trains on {B%s{w leaving {g%d{w trains{x.",
            ch->name, skill_table[sn].rating[ch->gameclass], skill_table[sn].name, ch->train );
    log_string( buf );
    wiznet(buf,NULL,NULL,WIZ_SECURE,0,get_trust(ch));
    return;
  }

  act("$N tells you 'I do not understand...'",ch,NULL,trainer,TO_CHAR);
}

/* do_spells, by Taeloch (09-10) */
void do_spells (CHAR_DATA *ch, char *argument)
{
  BUFFER *buffer;
  char   spell_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
  char   buf[MAX_STRING_LENGTH];
  char   buf2[MAX_STRING_LENGTH];
  char   buf3[MAX_STRING_LENGTH];
  char   arg[MIL];
  char   color[2];
  int    lev_min, lev_max, lev_tmp;
  int    level, fGroup, gn, sn, mana;
  char   str_match[MIL];
  bool   sMatch = FALSE;
  bool   found = FALSE;

  if ( IS_NPC( ch ) )
  {
    send_to_char( "Not on NPCs.\n\r", ch );
    return;
  }

  // set default level range
  lev_min = 1;
  lev_max = (ch->level < LEVEL_HERO) ? ch->level : LEVEL_HERO;
  strcpy(str_match,"");

  argument = one_argument( argument, arg );

  // If it's a number, must be a level max or range
  if (is_number(arg))
  {
    lev_tmp = atoi(arg);
    argument = one_argument( argument, arg );

    if (is_number(arg)) // a second number means it's a range
    {
      lev_min = lev_tmp;
      lev_max = atoi(arg);
      // a third argument would be a search keyword
      argument = one_argument( argument, arg );
    }
    else // no second number means first was upper limit
      lev_max = lev_tmp;

    // if there is an argument still there (and not "all") it must be a search word
    if ( strcmp(arg,"")
         &&   strcmp(arg,"all") )
    {
      sMatch = TRUE;
      strcpy(str_match,arg);
    }
  }
  else if (!str_prefix( capitalize(arg), "All" )
           &&  !IS_NULLSTR(arg) )
    lev_max = LEVEL_HERO;
  else if (strcmp(arg,"")) // a non-null string was found
  {
    sMatch = TRUE;
    strcpy(str_match,arg);
  }
  // ******** END of parameter detection ********

  // Make sure the levels are in range
  if ( ( lev_min < 1 ) || ( lev_min > LEVEL_HERO )
       ||   ( lev_max < 1 ) || ( lev_max > LEVEL_HERO )
       ||   ( lev_max < lev_min ) )
  {
    printf_to_char(ch,"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
    return;
  }

  // Init the arrays
  for ( lev_tmp = 0; lev_tmp <= LEVEL_HERO; lev_tmp++ )
  {
    spell_list[lev_tmp][0] = '\0';
  }

  // If matching by keyword, and a group name matches, add all its spells
  if (sMatch && ( ( fGroup = group_lookup( str_match ) ) != -1 ) )
  {
    for ( gn = 0; gn < MAX_IN_GROUP; gn++ )
    {
      if ( IS_NULLSTR( group_table[fGroup].spells[gn] ) )
        continue;

      sn = skill_lookup( group_table[fGroup].spells[gn] );
      level = is_racial_skill( ch, sn ) ? 0 : skill_table[sn].skill_level[ch->gameclass];

      if ( ((level < lev_min)
            && !is_racial_skill( ch, sn ) )
           ||   (level > lev_max)
           ||   (skill_table[sn].spell_fun == spell_null)
           ||   (ch->pcdata->learned[sn] <= 0 ) )
        continue;

      found = TRUE;
      strncpy_color(buf2,skill_table[sn].name,18,' ',TRUE);

      if ( ch->level < level )
        sprintf( buf, "%-18s n/a       ", buf2 );
      else
      {
        mana = UMAX( skill_table[sn].min_mana, 100 / ( 2 + ch->level - level ) );
        sprintf( color, "%c", color_scale( ch->pcdata->learned[sn], "rRYG" ) );
        sprintf( buf, "{w[{c%-20s{w][{B%3d{w]{g[{%s%3d%%{g] {D(%s){x",
                 buf2, mana, ch->pcdata->learned[sn] == 100 ? "W" : color,
                 ch->pcdata->learned[sn],
                 capitalize(group_table[fGroup].name) );
      }

      if ( spell_list[level][0] == '\0' )
      {
        if ( level == 0 )
          sprintf( spell_list[level], "\n\r{xRacial:     %s", buf );
        else
          sprintf( spell_list[level], "\n\r{xLevel %2d: %s", level, buf );
      }
      else  /* append */
      {
        strcat( spell_list[level], "\n\r          " );
        strcat( spell_list[level], buf );
      }
    } // for gn = 0 to MAX_IN_GROUP
  } // if keyword matches a group name

  // Loop through all spells now
  for ( sn = 0; sn < MAX_SKILL; sn++ )
  {
    if ( IS_NULLSTR( skill_table[sn].name ) )
      continue;

    if ( sMatch && strstr( skill_table[sn].name, str_match ) == NULL )
      continue;

    level = is_racial_skill( ch, sn ) ? 0 : skill_table[sn].skill_level[ch->gameclass];

    if ( ((level < lev_min)
          && !is_racial_skill( ch, sn ) )
         ||   (level > lev_max)
         ||   (skill_table[sn].spell_fun == spell_null)
         ||   (ch->pcdata->learned[sn] <= 0 ) )
      continue;

    found = TRUE;
    strncpy_color(buf2,skill_table[sn].name,18,' ',TRUE);

    sprintf( buf3, "{c%s", skill_table[sn].name );

    // don't list the same spell twice in case it matches group AND spell name
    if (strstr(spell_list[level], buf3 ) != NULL )
      continue;

    if ( ch->level < level )
      sprintf( buf, "%-18s n/a       ", buf2 );
    else
    {
      mana = UMAX( skill_table[sn].min_mana, 100 / ( 2 + ch->level - level ) );
      sprintf( color, "%c", color_scale(ch->pcdata->learned[sn], "rRYG"));
      sprintf( buf, "{w[{c%-20s{w][{B%3d{w]{g[{%s%3d%%{g]{x",
               buf2, mana,
               ch->pcdata->learned[sn] == 100 ? "W" : color,
               ch->pcdata->learned[sn]);
    }

    if ( spell_list[level][0] == '\0' )
    {
      if ( level == 0 )
        sprintf( spell_list[level], "\n\r{xRacial:   %s", buf );
      else
        sprintf( spell_list[level], "\n\r{xLevel %2d: %s", level, buf );
    }
    else /* append */
    {
      strcat( spell_list[level], "\n\r          " );
      strcat( spell_list[level], buf );
    }
  } // for (sn = 0 to MAX_SKILL

  if ( !found )
  {
    send_to_char( "No spells found.\n\r", ch );
    return;
  }

  // Display the collected data:
  buffer = new_buf();
  for ( lev_tmp = 0; lev_tmp <= LEVEL_HERO; lev_tmp++ )
    if ( spell_list[lev_tmp][0] != '\0' )
      bstrcat( buffer, spell_list[lev_tmp] );
  bstrcat( buffer, "\n\r" );
  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );

  return;
}

/*
 * Given a percent $pct$ and a list of letters $colors$ that represents the
 * color codes of each equally divided level in a color scale, we'll return
 * the letter of $colors$ corresponding to the proper level in the scale.
 */
char color_scale( int pct, char *colors )
{
  int levels = strlen( colors );
  pct = URANGE( 0, pct, 99 );
  return colors[ levels * pct / 100 ];
}

/* do_skills, by Taeloch (10-10)*/
void do_skills (CHAR_DATA *ch, char *argument)
{
  BUFFER *buffer;
  char   skill_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
  char   buf[MAX_STRING_LENGTH];
  char   buf2[MAX_STRING_LENGTH];
  char   arg[MIL];
  char   color[2];
  int    lev_min, lev_max, lev_tmp;
  int    level, fGroup, gn, sn;
  char   str_match[MIL];
  bool   sMatch = FALSE;
  bool   found = FALSE;

  if ( IS_NPC( ch ) )
  {
    send_to_char( "Not on NPCs.\n\r", ch );
    return;
  }

  // set default level range
  lev_min = 1;
  lev_max = (ch->level < LEVEL_HERO) ? ch->level : LEVEL_HERO;
  strcpy(str_match,"");

  argument = one_argument( argument, arg );

  // If it's a number, must be a level max or range
  if (is_number(arg))
  {
    lev_tmp = atoi(arg);
    argument = one_argument( argument, arg );

    if (is_number(arg)) // a second number means it's a range
    {
      lev_min = lev_tmp;
      lev_max = atoi(arg);
      // a third argument would be a search keyword
      argument = one_argument( argument, arg );
    }
    else // no second number means first was upper limit
      lev_max = lev_tmp;

    // if there is an argument still there (and not "all") it must be a search word
    if ( strcmp(arg,"")
         &&   strcmp(arg,"all") )
    {
      sMatch = TRUE;
      strcpy(str_match,arg);
    }
  }
  else if (!str_prefix( capitalize(arg), "All" )
           &&  !IS_NULLSTR(arg) )
    lev_max = LEVEL_HERO;
  else if (strcmp(arg,"")) // a non-null string was found
  {
    sMatch = TRUE;
    strcpy(str_match,arg);
  }
  // ******** END of parameter detection ********

  // Make sure the levels are in range
  if ( ( lev_min < 1 ) || ( lev_min > LEVEL_HERO )
       ||   ( lev_max < 1 ) || ( lev_max > LEVEL_HERO )
       ||   ( lev_max < lev_min ) )
  {
    printf_to_char(ch,"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
    return;
  }

  // Init the arrays
  for ( lev_tmp = 0; lev_tmp <= LEVEL_HERO; lev_tmp++ )
  {
    skill_list[lev_tmp][0] = '\0';
  }

  // If matching by keyword, and a group name matches, add all its spells
  if (sMatch && ( ( fGroup = group_lookup( str_match ) ) != -1 ) )
  {
    for ( gn = 0; gn < MAX_IN_GROUP; gn++ )
    {
      if ( IS_NULLSTR( group_table[fGroup].spells[gn] ) )
        continue;

      sn = skill_lookup( group_table[fGroup].spells[gn] );
      level = is_racial_skill( ch, sn ) ? 0 : skill_table[sn].skill_level[ch->gameclass];

      if ( ((level < lev_min)
            && !is_racial_skill( ch, sn ) )
           ||   (level > lev_max)
           ||   (skill_table[sn].spell_fun != spell_null)
           ||   (ch->pcdata->learned[sn] <= 0 ) )
        continue;

      found = TRUE;
      strncpy_color(buf2,skill_table[sn].name,18,' ',TRUE);

      if ( ch->level < level )
        sprintf( buf, "%-18s n/a       ", buf2 );
      else
      {
        sprintf( color, "%c", color_scale( ch->pcdata->learned[sn], "rRYG" ) );
        sprintf( buf, "{w[{c%-20s{w]{g[{%s%3d%%{g] {D(%s){x",
                 buf2, ch->pcdata->learned[sn] == 100 ? "W" : color,
                 ch->pcdata->learned[sn], capitalize(group_table[fGroup].name) );
      }

      if ( skill_list[level][0] == '\0' )
      {
        if ( level == 0 )
          sprintf( skill_list[level], "\n\r{xRacial:     %s", buf );
        else
          sprintf( skill_list[level], "\n\r{xLevel %2d: %s", level, buf );
      }
      else  /* append */
      {
        strcat( skill_list[level], "\n\r          " );
        strcat( skill_list[level], buf );
      }
    } // for gn = 0 to MAX_IN_GROUP
  } // if keyword matches a group name


  // Loop through all spells now
  for ( sn = 0; sn < MAX_SKILL; sn++ )
  {
    if ( IS_NULLSTR( skill_table[sn].name ) )
      continue;

    if ( sMatch && strstr( skill_table[sn].name, str_match ) == NULL )
      continue;

    level = is_racial_skill( ch, sn ) ? 0 : skill_table[sn].skill_level[ch->gameclass];

    if ( ((level < lev_min)
          && !is_racial_skill( ch, sn ) )
         ||   (level > lev_max)
         ||   (skill_table[sn].spell_fun != spell_null)
         ||   (ch->pcdata->learned[sn] <= 0 ) )
      continue;

    found = TRUE;
    strncpy_color(buf2,skill_table[sn].name,18,' ',TRUE);

    // don't list the same spell twice in case it matches group AND spell name
    if (strstr(skill_list[level], skill_table[sn].name ) != NULL )
      continue;

    if ( ch->level < level )
      sprintf( buf, "%-18s n/a       ", buf2 );
    else
    {
      sprintf( color, "%c", color_scale(ch->pcdata->learned[sn], "rRYG"));
      sprintf( buf, "{w[{c%-20s{w]{g[{%s%3d%%{g]{x",
               buf2,
               ch->pcdata->learned[sn] == 100 ? "W" : color,
               ch->pcdata->learned[sn]);
    }

    if ( skill_list[level][0] == '\0' )
    {
      if ( is_racial_skill( ch, sn ) )//level == 0 )
        sprintf( skill_list[level], "\n\r{xRacial:   %s", buf );

      if ( level == 0 )
        sprintf( skill_list[level], "\n\r{xRacial:   %s", buf );
      else
        sprintf( skill_list[level], "\n\r{xLevel %2d: %s", level, buf );
    }
    else /* append */
    {
      strcat( skill_list[level], "\n\r          " );
      strcat( skill_list[level], buf );
    }
  } // for (sn = 0 to MAX_SKILL)

  if ( !found )
  {
    send_to_char( "No skills found.\n\r", ch );
    return;
  }

  // Display the collected data:
  buffer = new_buf();
  for ( lev_tmp = 0; lev_tmp <= LEVEL_HERO; lev_tmp++ )
    if ( skill_list[lev_tmp][0] != '\0' )
      bstrcat( buffer, skill_list[lev_tmp] );
  bstrcat( buffer, "\n\r" );
  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );

  return;
}

// replaced by Taeloch's new code
void do_old_skills(CHAR_DATA *ch, char *argument)
{
  BUFFER *buffer;
  char arg[MAX_INPUT_LENGTH];
  char skill_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
  char skill_columns[LEVEL_HERO + 1];
  char skill_name[MAX_INPUT_LENGTH];
  int sn, level, min_lev = 1, max_lev = LEVEL_HERO;
  bool fAll = FALSE, found = FALSE;
  char buf[MAX_STRING_LENGTH];
  char color[2];

  if (IS_NPC(ch))
    return;

  strcpy(skill_name,"");

  if (argument[0] != '\0')
  {
    fAll = TRUE;

    if ( str_prefix( argument, "all" ) )
    {
      argument = one_argument(argument,arg);

      if (!is_number(arg))
      {
        strcpy(skill_name,arg);
      }
      else
      {
        max_lev = atoi(arg);

        if (max_lev < 1 || max_lev > LEVEL_HERO)
        {
          printf_to_char(ch,"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
          return;
        }

        if (argument[0] != '\0')
        {
          argument = one_argument(argument,arg);

          if (!is_number(arg))
          {
            send_to_char("Arguments must be numerical, 'all', or a skill name.\n\r",ch);
            return;
          }

          min_lev = max_lev;
          max_lev = atoi(arg);


          if (max_lev < 1 || max_lev > LEVEL_HERO)
          {
            printf_to_char(ch, "Levels must be between 1 and %d.\n\r",LEVEL_HERO);
            return;
          }

          if (min_lev > max_lev)
          {
            send_to_char("That would be silly.\n\r",ch);
            return;
          }
        }
      }
    }
  }


  /* initialize data */
  for (level = 0; level < LEVEL_HERO + 1; level++)
  {
    skill_columns[level] = 0;
    skill_list[level][0] = '\0';
  }

  for (sn = 0; sn < MAX_SKILL; sn++)
  {
    /*printf_to_char(ch," \n\rSN = %d MAX = %d Test:", sn, MAX_SKILL);*/
    if (skill_table[sn].name == NULL )
      continue;

    if (is_racial_skill(ch,sn))
      level = 0;
    else
      level = skill_table[sn].skill_level[ch->gameclass];

    /* printf_to_char(ch, "SN = %d Skill: %s : Fall = %d, level = %d, min_lev ="
         "%d MAXlev = %d, Learned = %d, Race %d, skill_table fun = %d   ",
         sn, skill_table[sn].name, fAll, level, min_lev,
         max_lev, ch->pcdata->learned[sn],
         is_racial_skill(ch,sn), skill_table[sn].spell_fun);
    */
    if ( ( level < LEVEL_HERO + 1 )
         && ( fAll
              || ( level <= ch->level) )
         && ( ( level >= min_lev )
              || is_racial_skill( ch, sn ) )
         && ( level <= max_lev )
         && ( skill_table[sn].spell_fun == spell_null )
         && ( ch->pcdata->learned[sn] > 0 )

         && ( !str_prefix(skill_name, skill_table[sn].name) )

       )
    {
      found = TRUE;
      //    level = skill_table[sn].skill_level[ch->gameclass];
      mprintf( sizeof(color), color, "%c", color_scale( ch->pcdata->learned[sn], "rRYG") );

      if (ch->level < level)
        mprintf(sizeof(buf), buf,"%-18.18s {Dn/a      {x", skill_table[sn].name);
      else if (level == 0)
      {
        mprintf(sizeof(buf), buf,"{x%-18.18s {%s%3d%%      {x",skill_table[sn].name,
                ch->pcdata->learned[sn] == 100 ? "W" : color, ch->pcdata->learned[sn]);
        /*printf_to_char(ch, "SN: %d\n\r", sn);*/
      }
      else
        mprintf(sizeof(buf), buf,"{x%-18.18s {%s%3d%%      {x",skill_table[sn].name,
                ch->pcdata->learned[sn] == 100 ? "W" : color, ch->pcdata->learned[sn]);


      /*printf_to_char(ch,"Special SN = %d\n\r",sn);*/

      if (skill_list[level][0] == '\0')
      {

        if ( level == 0 )
        {
          //sprintf( skill_list[level], "\n\rRacial:     %s", buf );
          if ( min_lev <= 1 )
            mprintf( sizeof( skill_list[level]), skill_list[level],
                     "\n\rRacial:   %s{x", buf );
        }
        else
          mprintf( sizeof( skill_list[level]), skill_list[level],
                   "\n\rLevel %2d: %s{x", level, buf );
        /*printf_to_char(ch, "NULL entry BUF = %s\n\r", buf);*/
      }
      else /* append */
      {
        if ( ++skill_columns[level] % 2 == 0)
          strcat(skill_list[level],"\n\r         {x ");

        strcat(skill_list[level],buf);
        /*printf_to_char(ch, "NOT NULL LEVEL = %d BUF = %s\n\r",
           level,buf);
        printf_to_char(ch, "NOT NULL Array = %s\n\r", skill_list[level]);*/

      }
    }
    /*printf_to_char(ch,"Special SN 2 = %d\n\r",sn);*/
  }

  /*printf_to_char(ch,"Exiting code....  \n\r");*/
  /* return results */

  if (!found)
  {
    send_to_char("No skills found.\n\r",ch);
    return;
  }

  buffer = new_buf();
  for (level = 0; level < LEVEL_HERO + 1; level++)
    if (skill_list[level][0] != '\0')
      add_buf(buffer,skill_list[level]);
  add_buf(buffer,"{x\n\r");
  page_to_char(buf_string(buffer),ch);
  free_buf(buffer);
}

/* shows skills, groups and costs (only if not bought) */
void list_group_costs(CHAR_DATA *ch)
{

  if (IS_NPC(ch))
    return;
  list_listing(ch,list_group_costs_e);
  printf_to_char( ch, "Creation points: %d\n\r", ch->pcdata->points );
  printf_to_char( ch, "Experience per level: %d\n\r",
                  exp_per_level( ch, ch->gen_data->points_chosen ) );
  return;
}


void list_group_chosen(CHAR_DATA *ch)
{
  if (IS_NPC(ch))
    return;

  list_listing( ch, list_group_chosen_e );
  printf_to_char( ch, "Creation points: %d\n\r", ch->gen_data->points_chosen );
  printf_to_char( ch, "Experience per level: %d\n\r",
                  exp_per_level( ch, ch->gen_data->points_chosen ) );
  return;
}

int exp_per_level( CHAR_DATA *ch, int points )
{
  int expl,inc;
  int class_mult;
  char buf[MSL];


  if (IS_NPC(ch))
    return 1000;

  class_mult = pc_race_table[ch->race].class_mult[ch->gameclass];

  if ( class_mult < 50 )
  {
    mprintf(sizeof(buf), buf,"Class %s has a multiplier of %d",
            class_table[ch->gameclass].who_name, class_mult );
    bug( buf, 0 );
    class_mult = 110;
  }

  expl = 1000;
  inc = 500;

  // Changed to check greater than so nobody starts
  // at 1000 experience, if < 40 ( exp + class modifier )
  if ( points > 40 )
  {
    /* processing */
    points -= 40;

    while ( points > 9 )
    {
      expl += inc;
      //points -= 10;

      if ( ( points -= 10 ) > 9)
      {
        expl += inc;
        inc *= 2;
        points -= 10;
      }
    }
  }
  else
    points = 1; // Now gaining down to 40 CP's won't error give them 3k expl...


  expl += points * inc / 10;

  /* I feel we can add in level here...so that their numbers
   * will grow as they get bigger.
   */
//  if ( ch->level > 1 )
//    level = ch->level;


  //printf_to_char( ch, "expl: %d\n\r", expl );
//  expl += level * expl * 0.005;

  //printf_to_char( ch, "expl: %d\n\r", expl );
//  expl = level * 1.05 + expl * class_mult / 100;
  //printf_to_char( ch, "expl: %d\n\r", expl );

//if ( false )
//     send_to_char( "This is strange, since expl is always at least 1050", ch );

  return UMAX( expl, 1000 );
}

void do_level( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  int XPL = 0;
  int start_exp = 0;
  int i   = 0;
  int levmin = 1,levmax = LEVEL_HERO;
  BUFFER *buffer;
  CHAR_DATA *vch = ch;

  if ( IS_NPC( ch ) )
    return;

  argument=one_argument(argument, arg);

  if (is_number(arg))
  {
    levmin = atoi(arg);
    if (levmin < 1)
      levmin = 1;
    if (levmin > LEVEL_HERO)
      levmin = LEVEL_HERO;

    argument=one_argument(argument, arg);
    if (!is_number(arg) )
    {
      levmax = levmin;
      levmin = 1;
    }
    else
    {
      levmax = atoi(arg);
      if (levmax < 1)
        levmax = 1;
      if (levmax > LEVEL_HERO)
        levmax = LEVEL_HERO;
    }
  }
  else if (IS_IMMORTAL(ch))
  {
    vch = get_char_world(ch, arg);
    if ((vch == NULL) || IS_NPC(vch))
      vch = ch;
  }

  buffer = new_buf();

  XPL = exp_per_level( vch, vch->pcdata->points );
  start_exp = TNL( XPL, 1 ) - TNL( XPL, 0 );

  bprintf( buffer, "Experience per level for %d.\n\r", start_exp );
  bstrcat( buffer, "Level    Per/level  Total Exp\n\r\n\r" );


  for ( i = levmin ; i <= levmax ; i++ )
  {
    if ( i == vch->level )
      bprintf( buffer, "{CLevel {W%-3d    %5d {G[%7d]{x     ",
               i,
               TNL( XPL, i ) - TNL( XPL, i - 1 ),
               TNL( XPL, i ) );
    else
      bprintf( buffer, "{cLevel {w%-3d    %5d {g[%7d]{x     ",
               i,
               TNL( XPL, i ) - TNL( XPL, i - 1 ),
               TNL( XPL, i ) );

    if ( i % 2 == 0 )
      bstrcat( buffer, "\n\r" );
  }


  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );

}


/* this procedure handles the input parsing for the skill generator */
bool parse_gen_groups(CHAR_DATA *ch,char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int gn,sn;

  if (argument[0] == '\0')
    return FALSE;

  argument = one_argument(argument,arg);

  if (!str_prefix(arg,"help"))
  {
    if (argument[0] == '\0')
    {
      do_function(ch, &do_help,"group help");
      return TRUE;
    }

    do_function(ch, &do_help,argument);
    return TRUE;
  }

  if (!str_prefix(arg,"add"))
  {
    if (argument[0] == '\0')
    {
      send_to_char("You must provide a skill name.\n\r",ch);
      return TRUE;
    }
    if (ch->pcdata->points >= MAX_CPS)
    {
      send_to_char("Maximum amount of Creation points reached or exceeded\n\r",ch);
      return TRUE;
    }
    gn = group_lookup(argument);
    if (gn != -1)
    {
      if (ch->gen_data->group_chosen[gn]
          ||  ch->pcdata->group_known[gn])
      {
        send_to_char("You already know that group or Cannot add it in customization\n\r",ch);
        return TRUE;
      }

      if (group_table[gn].rating[ch->gameclass] < 1)
      {
        send_to_char("That group is not available.\n\r",ch);
        return TRUE;
      }

      /* Close security hole */
      if (ch->gen_data->points_chosen + group_table[gn].rating[ch->gameclass]
          > MAX_CPS)
      {
        printf_to_char(ch,
                       "You cannot take more than %d creation points.\n\r",MAX_CPS);
        return TRUE;
      }

      printf_to_char(ch,"%s group added\n\r",group_table[gn].name);
      ch->gen_data->group_chosen[gn] = TRUE;
      ch->gen_data->points_chosen += group_table[gn].rating[ch->gameclass];
      gn_add(ch,gn);
      ch->pcdata->points += group_table[gn].rating[ch->gameclass];
      return TRUE;
    }

    sn = skill_lookup(argument);
    if (sn != -1)
    {
      if (ch->gen_data->skill_chosen[sn]
          ||  ch->pcdata->learned[sn] > 0)
      {
        send_to_char("You already know that skill!\n\r",ch);
        return TRUE;
      }

      if (skill_table[sn].rating[ch->gameclass] < 1
          ||  skill_table[sn].spell_fun != spell_null)
      {
        send_to_char("That skill is not available.\n\r",ch);
        return TRUE;
      }



      /* Close security hole */
      if (ch->gen_data->points_chosen + skill_table[sn].rating[ch->gameclass]
          > MAX_CPS)
      {
        printf_to_char(ch,
                       "You cannot take more than %d creation points.\n\r", MAX_CPS);
        return TRUE;
      }

      printf_to_char(ch, "%s skill added\n\r",skill_table[sn].name);
      ch->gen_data->skill_chosen[sn] = TRUE;
      ch->gen_data->points_chosen += skill_table[sn].rating[ch->gameclass];
      ch->pcdata->learned[sn] = 1;
      ch->pcdata->points += skill_table[sn].rating[ch->gameclass];
      return TRUE;
    }

    send_to_char("No skills or groups by that name...\n\r",ch);
    return TRUE;
  }

  if (!strcmp(arg,"drop"))
  {
    if (argument[0] == '\0')
    {
      send_to_char("You must provide a skill to drop.\n\r",ch);
      return TRUE;
    }

    gn = group_lookup(argument);
    if (gn != -1 && ch->gen_data->group_chosen[gn])
    {
      send_to_char("Group dropped.\n\r",ch);
      ch->gen_data->group_chosen[gn] = FALSE;
      ch->gen_data->points_chosen -= group_table[gn].rating[ch->gameclass];
      gn_remove(ch,gn);
      /*      for (i = 0; i < MAX_GROUP; i++)
          {*/
      if (ch->gen_data->group_chosen[gn])
        gn_add(ch,gn);
      /*
        }*/
      ch->pcdata->points -= group_table[gn].rating[ch->gameclass];
      group_add(ch,class_table[ch->gameclass].base_group,FALSE);
      group_add(ch,"rom basics",FALSE);
      return TRUE;
    }

    sn = skill_lookup(argument);
    if (sn != -1 && ch->gen_data->skill_chosen[sn])
    {
      send_to_char("Skill dropped.\n\r",ch);
      ch->gen_data->skill_chosen[sn] = FALSE;
      ch->gen_data->points_chosen -= skill_table[sn].rating[ch->gameclass];
      ch->pcdata->learned[sn] = 0;
      ch->pcdata->points -= skill_table[sn].rating[ch->gameclass];
      group_add(ch,class_table[ch->gameclass].base_group,FALSE);
      group_add(ch,"rom basics",FALSE);
      return TRUE;
    }

    send_to_char("You haven't bought any such skill or group.\n\r",ch);
    return TRUE;
  }

  if (!str_prefix(arg,"premise"))
  {
    do_function(ch, &do_help,"premise");
    return TRUE;
  }

  if (!str_prefix(arg,"list"))
  {
    list_group_costs(ch);
    return TRUE;
  }

  if (!str_prefix(arg,"learned"))
  {
    list_group_chosen(ch);
    return TRUE;
  }

  if (!str_prefix(arg,"info"))
  {
    do_function(ch, &do_groups,argument);
    return TRUE;
  }

  if (!str_prefix(arg,"levels"))
  {
    do_function(ch, &do_level,"");
    return TRUE;
  }

  return FALSE;
}

/* shows all groups, or the sub-members of a group */
void do_groups(CHAR_DATA *ch, char *argument)
{
  int gn,sn,col;
  int skilllevel=0;
  int fn;
  char buf[MSL];
  char arg[MIL], arg2[MIL];
  bool combine_arguments = FALSE;

  if (IS_NPC(ch))
    return;

  col = 0;

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );

  if (arg[0] == '\0')
  {
    // show all groups
    for (gn = 0; gn < MAX_GROUP; gn++)
    {
      if (group_table[gn].name == NULL)
        break;

      if (!str_cmp(group_table[gn].name,"rom basics"))
        continue;

      if (ch->pcdata->group_known[gn])
      {
        printf_to_char(ch,"{c%3d%%{x %-20s ",
                       group_table[gn].proficiency[ch->gameclass],
                       group_table[gn].name );

        if (++col % 3 == 0)
          send_to_char("\n\r",ch);
      }
    }

    if ( col % 3 != 0 )
      send_to_char( "\n\r", ch );

    int XPL = exp_per_level( ch, ch->pcdata->points );
    printf_to_char(ch,"\n\rCreation points: %d   Exp/Lvl: %d\n\r",
                   ch->pcdata->points,
                   TNL( XPL, ch->level )
                   - TNL( XPL, ch->level - 1 ) );
    //exp_per_level(ch,ch->pcdata->points));
    return;
  }

  if (!str_cmp(arg,"all")) /* show all groups */
  {
    for (gn = 0; gn < MAX_GROUP; gn++)
    {
      if (group_table[gn].name == NULL)
        break;

      printf_to_char(ch,"%-20s ",group_table[gn].name);

      if (++col % 3 == 0)
        send_to_char("\n\r",ch);
    }

    if ( col % 3 != 0 )
      send_to_char( "\n\r", ch );

    return;
  }

  /* show the sub-members of a group */
  if (is_name(arg,"background"))
  {
    if (!IS_IMMORTAL(ch))
    {
      send_to_char("That group cannot be viewed by you.\n\r",ch);
      return;
    }
  }

  if ( !IS_NULLSTR(arg2) )
  {
    if ( str_cmp( arg2, "all" ) )
    {
      strcat( arg, " " );
      strcat( arg, arg2 );
      combine_arguments = TRUE;
    }
  }

  gn = group_lookup(arg);

  if (gn == -1)
  {
    send_to_char("No group of that name exist.\n\r",ch);
    send_to_char(
      "Type 'groups all' or 'info all' for a full listing.\n\r",ch);
    return;
  }

  for (sn = 0; sn < MAX_IN_GROUP; sn++)
  {
    if ( group_table[gn].spells[sn] == NULL )
      break;

    fn = skill_lookup_exact( group_table[gn].spells[sn] );

    if (fn == -1)
    {
      mprintf(sizeof(buf), buf,"(%s)",group_table[gn].spells[sn]);
      printf_to_char(ch,"%-23s     ",buf);
      col ++;
    }
    else
    {
      skilllevel = skill_table[fn].skill_level[ch->gameclass];

      if ( skilllevel < 1
           &&   !IS_IMMORTAL(ch)
           &&   ( IS_NULLSTR(arg2) //For 3 args...
                  ||     ( !IS_NULLSTR(arg2) && combine_arguments )
                  ||     ( !IS_NULLSTR(arg2) && !combine_arguments && str_cmp(arg2, "all") ) )
           &&   ( IS_NULLSTR(argument) //For 4 args...
                  ||     ( !IS_NULLSTR(argument) && str_prefix(argument, "all") ) ) )
        continue;
      else
      {
        printf_to_char(ch,"%-21s [%3d] ",group_table[gn].spells[sn],
                       skilllevel);
        col++;
      }
    }

    if (col % 3 == 0)
      send_to_char("\n\r",ch);
  }

  if ( col % 3 != 0 )
    send_to_char( "\n\r", ch );

}

/* checks for skill improvement */
void check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier )
{
  int chance;
  int rating;
  bool mastered = TRUE;
  int skn;

  if ( IS_NPC( ch ) )
    return;

  if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
    return;

  if ( ch->pcdata->learned[sn] >= 100 )
    ch->pcdata->learned[sn] = 100;

  if ( ( ch->pcdata->learned[sn] <=   0 )
       ||   ( ch->pcdata->learned[sn] >= 100 ) )
    return;  /* skill is not known */

  if ( !is_racial_skill( ch, sn ) )
  {
    if ( ( ch->level < skill_table[sn].skill_level[ch->gameclass]
           ||     skill_table[sn].rating[ch->gameclass] == 0 ) )
      return;
  }

  if ( is_racial_skill( ch, sn ) )
    rating = 1;
  else
    rating = skill_table[sn].rating[ch->gameclass];

  /* check to see if the character has a chance to learn */
  chance = 10 * int_app[get_curr_stat( ch, STAT_INT )].learn;
  chance /= ( multiplier * rating * 4 );
  chance += ch->level;

  if ( number_range( 1, 1000 ) > chance )
    return;

  /* now that the character has a CHANCE to learn, see if they really have */

  if ( success )
  {
    chance = URANGE( 5, 100 - ch->pcdata->learned[sn], 95 );
    if ( number_percent() < chance )
    {
      if ( ch->pcdata->learned[sn]+1 == 100 )
      {
        ch->pcdata->learned[sn]++;

        if ( ch->level < LEVEL_HERO )
        {
          gain_exp( ch, 8 * rating );
          printf_to_char( ch, "{&You have mastered %s!\n\rYou gain %d experience for your mastery!{x\n\r",
                          skill_table[sn].name,
                          ( 8 * rating ) );
        }
        else if ( ch->level == LEVEL_HERO )
        {
          ch->questdata->glory += (8*rating);
          ch->questdata->curr_points += (8*rating);
          printf_to_char(ch,"{&You have mastered %s!\n\rYou gain %d glory for your mastery!{x\n\r",
                         skill_table[sn].name,
                         ( 2 * rating ) );
        }

        for ( skn = 0; skn < MAX_SKILL; skn++ )
        {
          if ( skill_table[skn].name == NULL ) continue;

          if ( ( skill_table[skn].skill_level[ch->gameclass] < 0 )
               &&   !is_racial_skill( ch,skn ) ) continue;

          if (ch->pcdata->learned[skn] < 100)
          {
            mastered = FALSE;
            break;
          }
        }

        if (mastered)
          do_function( ch, &do_info, "I have just mastered my class!" );
      }
      else
      {
        ch->pcdata->learned[sn]++;

        if ( ch->level < LEVEL_HERO )
        {
          gain_exp( ch, 2 * rating );
          printf_to_char(ch,"{&You have become better at %s, increasing it to %d%%!\n\rYou gain %d experience for your improvement.{x\n\r",
                         skill_table[sn].name,
                         ch->pcdata->learned[sn],
                         ( 2 * rating ) );
        }
        else if ( ch->level == LEVEL_HERO )
        {
          ch->questdata->glory += (2*rating);
          ch->questdata->curr_points += (2*rating);
          printf_to_char(ch,"{&You have become better at %s, increasing it to %d%%!\n\rYou gain %d glory for your improvement.{x\n\r",
                         skill_table[sn].name,
                         ch->pcdata->learned[sn],
                         ( 2 * rating ) );
        }
      }
    }
  }
  else
  {
    chance = URANGE( 5, ch->pcdata->learned[sn] / 2, 30 );
    if ( number_percent() < chance )
    {
      ch->pcdata->learned[sn] += number_range( 1, 3 );
      ch->pcdata->learned[sn] = UMIN( ch->pcdata->learned[sn], 100 );
      if ( ch->pcdata->learned[sn] == 100 )
      {
        if ( ch->level < LEVEL_HERO )
        {
          gain_exp( ch, 16 * rating );
          printf_to_char( ch,
                          "{&You learn from your mistake and master your %s ability!\n\rYou gain %d experience for your improvement.{x\n\r",
                          skill_table[sn].name,
                          16 * rating );
        }
        else
        {
          ch->questdata->glory += (16*rating);
          ch->questdata->curr_points += (16*rating);
          printf_to_char( ch,
                          "{&You learn from your mistake and master your %s ability!\n\rYou gain %d glory for your improvement.{x\n\r",
                          skill_table[sn].name,
                          16 * rating );
        }

        for ( skn = 0; skn < MAX_SKILL; skn++ )
        {
          if ( skill_table[skn].name == NULL ) continue;

          if ( ( skill_table[skn].skill_level[ch->gameclass] < 0 )
               &&   !is_racial_skill( ch,skn ) ) continue;

          if (ch->pcdata->learned[skn] < 100)
          {
            mastered = FALSE;
            break;
          }
        }

        if (mastered)
          do_function( ch, &do_info, "I have just mastered my class!" );

      }
      else
      {
        if ( ch->level < LEVEL_HERO )
        {
          gain_exp( ch, 4 * rating );
          printf_to_char( ch,
                          "{&You learn from your mistakes, and your %s ability improves to %d%%.\n\rYou gain %d experience for your improvement.{x\n\r",
                          skill_table[sn].name, ch->pcdata->learned[sn], (4 * rating) );
        }
        else if ( ch->level == LEVEL_HERO )
        {
          ch->questdata->glory += (4*rating);
          ch->questdata->curr_points += (4*rating);
          printf_to_char(ch,
                         "{&You learn from your mistakes, and your %s ability improves to %d%%.\n\rYou gain %d glory for your improvement.{x\n\r",
                         skill_table[sn].name,
                         ch->pcdata->learned[sn],
                         ( 4 * rating ) );
        }
      }
    }
  }
}

/* returns a group index number given the name */
int group_lookup( const char *name )
{
  int gn;

  for ( gn = 0; gn < MAX_GROUP; gn++ )
  {
    if ( group_table[gn].name == NULL )
      break;

    if ( LOWER(name[0]) == LOWER(group_table[gn].name[0])
         &&   !str_prefix( name, group_table[gn].name ) )
      return gn;
  }

  return -1;
}

/* recursively adds a group given its number -- uses group_add */
void gn_add( CHAR_DATA *ch, int gn)
{
  int i;

  ch->pcdata->group_known[gn] = TRUE;

  for ( i = 0; i < MAX_IN_GROUP; i++)
  {
    if (group_table[gn].spells[i] == NULL)
      break;

    group_add(ch,group_table[gn].spells[i],FALSE);
  }
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove( CHAR_DATA *ch, int gn)
{
  int i;

  ch->pcdata->group_known[gn] = FALSE;

  for ( i = 0; i < MAX_IN_GROUP; i ++)
  {
    if (group_table[gn].spells[i] == NULL)
      break;
    group_remove(ch,group_table[gn].spells[i]);
  }
}

/* use for processing a skill or group for addition  */
void group_add( CHAR_DATA *ch, const char *name, bool deduct)
{
  int sn,gn;

  if (IS_NPC(ch)) /* NPCs do not have skills */
    return;

  sn = skill_lookup(name);

  /* added by Merak 06-08-20 */
  if (ch->gameclass != -1)
  {
    if (skill_table[sn].skill_level[ch->gameclass] == -1)
      return;
  }

  if (sn != -1)
  {
    if (ch->pcdata->learned[sn] == 0) /* i.e. not known */
    {
      ch->pcdata->learned[sn] = 1;

      if (deduct)
        ch->pcdata->points += skill_table[sn].rating[ch->gameclass];
    }

    return;
  }

  /* now check groups */

  gn = group_lookup(name);

  if (gn != -1)
  {
    if (ch->pcdata->group_known[gn] == FALSE)
    {
      ch->pcdata->group_known[gn] = TRUE;

      if (deduct)
        ch->pcdata->points += group_table[gn].rating[ch->gameclass];
    }

    gn_add(ch,gn); /* make sure all skills in the group are known */
  }
}

/* used for processing a skill or group for deletion -- no points back! */

void group_remove(CHAR_DATA *ch, const char *name)
{
  int sn, gn;

  sn = skill_lookup(name);

  if (sn != -1)
  {
    ch->pcdata->learned[sn] = 0;
    return;
  }

  /* now check groups */

  gn = group_lookup(name);

  if (gn != -1 && ch->pcdata->group_known[gn] == TRUE)
  {
    ch->pcdata->group_known[gn] = FALSE;
    gn_remove(ch,gn);  /* be sure to call gn_add on all remaining groups */
  }
}


void list_listing(CHAR_DATA *ch,enum list_setting setting)
{
  int gn,sn,col;

  if (IS_NPC(ch))
    return;

  col = 0;
  printf_to_char(ch,"{C%32s{x\n\r","GROUPS");
  printf_to_char(ch,"{C|==========================================================|{x\n\r");
  printf_to_char(ch, "{C|{W%-22s %-5s{C|{W%-22s %-5s {C|{x\n\r",
                 "Group","Cost","Group","Cost");
  printf_to_char(ch,"{C|----------------------------------------------------------|\n\r|{W");

  for (gn = 0; gn < MAX_GROUP; gn++)
  {
    if (group_table[gn].name == NULL)
      break;

    if ( (ch->level > 1)
         && (!strcmp(group_table[gn].name,"priest default")
             ||  !strcmp(group_table[gn].name,"conjurer default")
             ||  !strcmp(group_table[gn].name,"highwayman default")
             ||  !strcmp(group_table[gn].name,"knight default")
             ||  !strcmp(group_table[gn].name,"warlock default")
             ||  !strcmp(group_table[gn].name,"mystic default")
             ||  !strcmp(group_table[gn].name,"druid default")
             ||  !strcmp(group_table[gn].name,"inquisitor default")
             ||  !strcmp(group_table[gn].name,"occultist default")
             ||  !strcmp(group_table[gn].name,"barbarian default")
             ||  !strcmp(group_table[gn].name,"alchemist default")
             ||  !strcmp(group_table[gn].name,"woodsman default") ) )
      continue;

    if (list_group_costs_e == setting)
    {
      if (ch->gen_data->group_chosen[gn])
        continue;
      if (ch->pcdata->group_known[gn])
        continue;
      if (group_table[gn].rating[ch->gameclass] <= 0)
        continue;
    }
    if (list_gain_list_e == setting)
    {
      if (ch->pcdata->group_known[gn])
        continue;
      if (group_table[gn].rating[ch->gameclass] <= 0)
        continue;
    }
    if (list_group_chosen_e == setting)
    {

      if (!ch->gen_data->group_chosen[gn])
        continue;
      if (group_table[gn].rating[ch->gameclass] <= 0)
        continue;
    }

    printf_to_char(ch,"%-22s %3d  ",
                   group_table[gn].name,group_table[gn].rating[ch->gameclass]);
    if (++col % 2 == 0)
      send_to_char("{C |\n\r|{W",ch);
    else
      send_to_char("{C|{W",ch);
  }
  if (col % 2 != 0)
    printf_to_char(ch,"%-29s{C|\n\r|{x"," ");
  printf_to_char(ch,"{C==========================================================|{x\n\r");
  printf_to_char(ch,"{Y%32s{x\n\r","SKILLS");
  col = 0;

  printf_to_char(ch,"{Y|==========================================================|{x\n\r");
  printf_to_char(ch, "{Y|{W%-18s %-8s{Y|{W%-18s %-8s {Y|{x\n\r",
                 "Skill","Cost[Lvl]","Skill","Cost[Lvl]");
  printf_to_char(ch,"{Y|----------------------------------------------------------|\n\r|{W");

  for (sn = 0; sn < MAX_SKILL; sn++)
  {
    if (skill_table[sn].name == NULL)
      break;

    if (list_group_costs_e == setting)
    {
      if (ch->gen_data->skill_chosen[sn] )
        continue;
      if (ch->pcdata->learned[sn] != 0)
        continue;
      if (skill_table[sn].spell_fun != spell_null)
        continue;
      if (skill_table[sn].rating[ch->gameclass] <= 0)
        continue;
    }

    if (list_gain_list_e == setting)
    {
      if (ch->pcdata->learned[sn] != 0)
        continue;
      if (skill_table[sn].spell_fun != spell_null)
        continue;
      if (skill_table[sn].rating[ch->gameclass] <= 0)
        continue;
    }

    if (list_group_chosen_e == setting)
    {
      if (!ch->gen_data->skill_chosen[sn])
        continue;
      if (skill_table[sn].rating[ch->gameclass] <= 0)
        continue;
    }
    printf_to_char(ch,"%-18s {c%3d{W[{c%2d{W]{x  ",
                   skill_table[sn].name,skill_table[sn].rating[ch->gameclass],
                   skill_table[sn].skill_level[ch->gameclass]);
    if (++col % 2 == 0)
      send_to_char("{Y |\n\r|{W",ch);
    else
      send_to_char("{Y|{W",ch);
  }
  if (col % 2 != 0)
    printf_to_char(ch,"%-29s{Y|\n\r|{x"," ");
  printf_to_char(ch,"{Y==========================================================|{x\n\r");

}

bool is_racial_skill( CHAR_DATA * ch, const int racial_sn )
{
  int i;
  bool skill_found = FALSE;

  if (IS_NPC(ch))
    return FALSE;

  //if (pc_race_table[ch->race].skills[i] != NULL)
  //  printf_to_char(ch,"racial_sn is %d\n\r",racial_sn);

  if (racial_sn < 0)
    return FALSE;

  for ( i = 0; i < MAX_RACE_SKILLS; i++ )
  {
    if (pc_race_table[ch->race].skills[i] == NULL)
      break;
    if ( racial_sn == skill_lookup( pc_race_table[ch->race].skills[i] ) )
      skill_found = TRUE;
  }

  return skill_found;
}

/* Find the smallest total creation points for a class */
int total_class_cp(CHAR_DATA *ch, int iClass)
{
  int dgn, gn, sn, gsn, tot = 0;
  bool add_skill = FALSE, debug_fun = FALSE;

  // start off with the default group
  dgn = group_lookup(class_table[iClass].default_group);
  tot = group_table[dgn].rating[iClass];
  if (debug_fun) printf_to_char(ch,"D: %s (gn %d, %d pt)\n\r", class_table[iClass].default_group, dgn, tot);

  // loop through the groups
  for (gn = 0; gn < MAX_GROUP; gn++)
  {
    if (group_table[gn].name == NULL)
      break;

    if ( strstr(group_table[gn].name,"default") )
      continue;

    if ( strstr(group_table[gn].name,"background") )
      continue;

    if ( strstr(group_table[gn].name,"basics") )
      continue;

    add_skill = TRUE;

    if (group_table[gn].rating[iClass] > 0)
    {
      // make sure this group is not in the default group
      for (gsn = 0; gsn < MAX_IN_GROUP; gsn++)
      {
        if ( group_table[dgn].spells[gsn] == NULL )
          break;

        if (!strcmp(group_table[gn].name, group_table[dgn].spells[gsn]))
          add_skill = FALSE;
      }

      if (add_skill)
      {
        tot += group_table[gn].rating[iClass];
        if (debug_fun) printf_to_char(ch,"G: %s (gn %d, %d pt)\n\r", group_table[gn].name, gn, group_table[gn].rating[iClass]);
      } // if add group
    } // group class check
  } // group loop

  // loop through the skills that aren't in the class' groups
  for ( sn = 0; sn < MAX_SKILL; sn++ )
  {
    if ( skill_table[sn].name == NULL )
      continue;

    if ( strstr(skill_table[sn].name,"recall") )
      continue;

    if ( strstr(skill_table[sn].name,"wands") )
      continue;

    if ( strstr(skill_table[sn].name,"staves") )
      continue;

    if ( strstr(skill_table[sn].name,"scrolls") )
      continue;

    // class gets this skill - only add if not in a group for that class
    if (skill_table[sn].rating[iClass] > 0)
    {
      add_skill = TRUE;

      // go through all the groups the class gets, looking for that skill
      for (gn = 0; gn < MAX_GROUP; gn++)
      {
        if (group_table[gn].name == NULL)
          break;

        // class gets this group, see if skill is in it
        if (group_table[gn].rating[iClass] > 0)
        {
          // if found in group, don't add to total
          for (gsn = 0; gsn < MAX_IN_GROUP; gsn++)
          {
            if ( group_table[gn].spells[gsn] == NULL )
              break;

            // need to see if group is in default group
            if (skill_lookup(group_table[gn].spells[gsn]) == sn)
              add_skill = FALSE;
          }

        } // if class group
      } // group loop

      if (add_skill)
      {
        tot += skill_table[sn].rating[iClass];
        if (debug_fun) printf_to_char(ch,"S: %s (gn %d, %d pt)\n\r", skill_table[sn].name, sn, skill_table[sn].rating[iClass]);
      }
    } // if class skill
  } // skill loop

  return tot;
}
