/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/
/* Automated quest code originall written by Vassago (Who i'm giving
credit to despite the fact he gives no credit to the people who's code he
copies (aka the rom consortium).  
Revamped by Kharas (mud@fading.tcimet.net) */

/* Real quest.c stuff.. above is just installation crud :) */
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
#include "merc.h"
#include "db.h"
#include "recycle.h"
#include "magic.h"
#include "interp.h"

struct reward_type 
{
  char * name;
  char * keyword;
  int    cost;
  bool   object;
  int    value;
  void * where;
};

struct quest_desc_type {
  char name[MSL];
  char short_descr[MSL];
  char long_descr[MSL];
};

struct questmob_desc_type {
  char name[MSL];
  char short_descr[MSL];
  char long_descr[MSL];
  int sex;
};

#define NUM_QUEST_OBJS 4 /* Number of items in the structure below */
#define NUM_QUEST_MOBS 6 /* Number of mobs in the structure below */

/* Descriptions of quest items go here:
Format is: "keywords", "Short description", "Long description" */
const struct quest_desc_type quest_desc[] =
{
  {"Quest sceptre",  "{&The Sceptre of Courage{x",
   "The {&Sceptre of Courage{x is lying here, waiting to be returned to its owner.{w"},

  {"Quest crown",    "{&The Crown of Wisdom{x", 
   "The {&Crown of Wisdom{x is lying here, waiting to be returned to its owner."},

  {"Quest gauntlet", "{&The Gauntlets of Strength{x", 
   "The {&Gauntlets of Strength{x are lying here, waiting to be returned to its owner."},

  {"Quest diamond",  "{&A Quest Diamond{x", 
   "The {&Quest Diamond{x is lying here, waiting to be returned to its owner."},

  {"\0", "\0", "\0"}
};

/* Descriptions of rescue quest mobs go here:
Format is: "keywords", "Short description", "Long description"
*/

const struct questmob_desc_type questmob_desc[] =
{
  {"druid priestess", "a druid priestess",
   "{&A druid priestess{x prays to be returned to her temple.{w\n\r",
   SEX_FEMALE },

  {"lost child", "a lost child",
   "{&A lost child{x looks scared and sad.{w\n\r",
   SEX_MALE },

  {"damsel distress", "a damsel in distress",
   "{&A beautiful damsel{x appears distressed about her situation{w\n\r",
   SEX_FEMALE },

  {"heiress corada", "the Heiress of Corada",
   "{&The Heiress of Corada{x seems confused about her surroundings.{w\n\r",
   SEX_FEMALE },

  {"friend Janna", "Queen Kaiyren's friend Janna",
   "{&Queen Kaiyren's friend Janna{x is bold, but visibly scared.{w\n\r",
   SEX_FEMALE },

  {"prince Renier", "Prince Renier",
   "{&Prince Renier{x puts up a confident front, but is obviously lost.{w\n\r",
   SEX_MALE },

  {"\0", "\0", "\0"}
};

/* Local functions */
void generate_quest     args(( CHAR_DATA *ch, CHAR_DATA *questman ));
void quest_update       args(( void ));
bool quest_level_diff   args(( int clevel, int mlevel));

/* The main quest function */
void do_quest(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *questman;
  OBJ_DATA *obj=NULL, *obj_next;
  char buf [MSL];
  char buf3 [MSL], buf4[MSL];
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  int i;
  int chance = 0;


  /* Add rewards as follows: "Obj name on "quest list", "keywords", "QP Cost", Load_object?,
     (if TRUE) Obj_VNum (else) a value to set in next field, part of char_data to be modified */ 
  const struct reward_type reward_table[]= 
  {
    { "{wan {caer{wi{Dal{w plate{x",                 "aerial plate", 2000,  TRUE, 3397,              0 },
    { "{wa si{Wl{Dve{wr {cflask{x",                  "silver flask", 5000,  TRUE, 3396,              0 },
    { "{wa {Dcha{wli{Dc{we of {rbl{Roo{rd{x",        "chalice blood",8000,  TRUE, 3391,              0 },
    { "{wa {rFight{yer{r'{ys{x Bedroll Token",       "fighter",      10000, TRUE, 3384,              0 },
    { "{wa {mCast{ber{m'{bs{x Bedroll Token",        "caster",       10000, TRUE, 3385,              0 },
    { "{wa {rR{we{Ds{Rtr{ri{wn{rg {DT{wo{Dk{Re{rn{x","restring",     25000, TRUE, OBJ_VNUM_RESTRING, 0 },
    { NULL,                                          NULL,           0,     FALSE,0,                 0 } /* Never remove this!!! */
  };

  if (IS_GHOST(ch)) {
    send_to_char("Questing is unavailable to one such as yourself.\n\r",ch);
    return;
  }

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if(IS_NPC(ch))
    { send_to_char("NPC's can't quest.\n\r",ch); return; }

  if (arg1[0] == '\0')
    {
      send_to_char("{DQuest commands: {gInformation Time Request Complete Quit List Buy Showinfo{D.{x\n\r",ch);
      send_to_char("{DFor more information, type '{gHelp Quest{D'.{x\n\r",ch);
      return;
    }

  if ( ch->questdata->current_streak < 10 )
  {
    ch->questdata->current_streak = 10;
  }

  /*
   * Quest Information
   */
  if ( !str_prefix(arg1, "showinfo" ) )
  {
    if ( IS_SET( ch->act, PLR_HIDEQUEST ) )
    {
      REMOVE_BIT(ch->act,PLR_HIDEQUEST);
      send_to_char( "Others can now see detailed quest information on you.\n\r", ch );
    }
    else
    {
      SET_BIT(ch->act,PLR_HIDEQUEST);
      send_to_char( "Others can no longer see detailed quest information on you.\n\r", ch );
    }
    return;
  }

  /*
   * Quest Information
   */
  if ( !str_prefix(arg1, "information" ) )
  {
    CHAR_DATA *gch = NULL;

    if ( !IS_NULLSTR( arg2 ) )
    {
        if ( ( gch = get_char_world( ch, arg2 ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }

        if ( IS_NPC( gch ) )
            gch = ch;

    }
    else
        gch = ch;

    if ( !IS_IMMORTAL( ch )
    &&   !is_same_group( ch, gch ) )
    {
        send_to_char(
            "There is nobody with that name questing in your group.\n\r", ch );
        return;
    }

    BUFFER *buffer;

    buffer = new_buf();

    bstrcat( buffer, "{G*{g---------------------------------------------------{G*{x\n\r" );

    if ( !IS_SET( gch->act, PLR_HIDEQUEST )
    || ( ch == gch )
    || IS_IMMORTAL( ch ) )
    {
      bprintf( buffer, " Current: %-7d             Attp/Comp:  %6d/%-6d\n\r",
        gch->questdata->curr_points,
        gch->questdata->attempt_num,
        gch->questdata->comp_num );

      bprintf( buffer, " Earned:  %-7d             Fail/Quit:  %6d/%-6d\n\r",
        gch->questdata->accum_points,
        gch->questdata->failed,
        gch->questdata->quit_num );

      bprintf( buffer, " Glory:   %-7d             This Level:       %d\n\r",
        gch->questdata->glory,
        gch->questdata->quest_number );

      bprintf( buffer, " Total:   %-7d             Streak:           %d\n\r",
        gch->questdata->accum_points + gch->questdata->glory,
        gch->questdata->streak );

      bprintf( buffer, "\n\r" );
    }


    if ( !IS_SET( gch->act,PLR_QUESTING ) )
    {
      if (gch->questdata->nextquest > 1)
          bprintf( buffer, " Current Status: {g%d{x hours remaining.\n\r\n\r",
                 gch->questdata->nextquest );
      else if ( gch->questdata->nextquest == 1 )
          bstrcat( buffer, " Current Status: less than one hour remaining.{x\n\r\n\r" );
      else
          bprintf( buffer, " Current Status: %s are not currently questing.\n\r\n\r",
                ( gch == ch ? "You" : "They" ) );
    }
    else if ( gch->questdata->obj_vnum > 0
    ||        gch->questdata->mob_vnum > 0 )
        bprintf( buffer, " Current Status: {g%d{x minutes to complete %s quest.\n\r\n\r",
            gch->questdata->countdown,
            ( gch == ch ? "your" : "their" ) );


    if ( IS_SET( gch->act, PLR_QUESTING ) )
    {
        if ( gch->questdata->questgiver == NULL )
        {
          bugf("ch->questdata->questgiver == NULL.  ch = %s\n\r", gch->name);
          return;
        }

        if ( gch->questdata->mob_vnum == -1
        &&  gch->questdata->questgiver->short_descr != NULL )
        {
            bprintf( buffer, " %s quest is {&ALMOST{w complete!{x\n\r",
                ( gch == ch ? "Your" : "Their" ) );
            if ( gch == ch )
                bprintf( buffer, " Get back to {&%s{w before your time runs out!{x\n\r",
                    gch->questdata->questgiver->short_descr );
        }
        else if ( gch->questdata->obj_vnum > 0
        ||        gch->questdata->mob_vnum > 0 )
        {
            bprintf( buffer, "%s", buf_string( gch->questdata->log ) );
            //send_to_char( buf_string( gch->questdata->log ), ch );
        }
        else
        {
            bstrcat( buffer,
            " Hey, I am just an automated machine.  What do I know.\n\r" );
          bugf( "QUEST: Quest Info Empty" );
        }
    }

    bstrcat( buffer, "{G*{g---------------------------------------------------{G*{x\n\r" );   

    page_to_char( buf_string( buffer ), ch );
    free_buf( buffer );
 
    return;
}

  else if (!strcmp(arg1, "time"))
  {

    CHAR_DATA *gch = NULL;

    if ( !IS_NULLSTR( arg2 ) )
    {
        if ( ( gch = get_char_world( ch, arg2 ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }

        if ( IS_NPC( gch ) )
            gch = ch;

    }
    else
        gch = ch;

    if ( !IS_IMMORTAL( ch )
    &&   !is_same_group( ch, gch ) )
    {
        send_to_char(
            "There is nobody with that name questing in your group.\n\r", ch );
        return;
    }


    if ( !IS_SET( gch->act, PLR_QUESTING ) )
    {
      printf_to_char( gch,
        "%s aren't currently on a quest.{x\n\r",
            ( gch == ch ? "You" : "They" ) );

      if ( gch->questdata->nextquest > 1 )
      {
          printf_to_char( gch,
            "There are {g%d{w hours remaining until %s can go on another quest.{x\n\r", 
                gch->questdata->nextquest,
                ( gch == ch ? "you" : "they" ) );
      }
      else if ( gch->questdata->nextquest == 1 )
      {
          printf_to_char( gch,
            "There is less than an hour remaining until %s can go on another quest.{x\n\r",
                ( gch == ch ? "you" : "they" ) );
      }
      return;
    }
    else if ( gch->questdata->countdown > 0 )
        printf_to_char( gch,
            "Time left for current quest:{g %d{x\n\r",
                gch->questdata->countdown);

    return;
  }

  for ( questman = ch->in_room->people; questman;questman = questman->next_in_room )
    {
      if (!IS_NPC(questman)) 
    continue;
      if (questman->spec_fun == spec_lookup( "spec_questmaster" ))
    break;
    }

  if (questman == NULL || questman->spec_fun != spec_lookup("spec_questmaster" ))
    {
      send_to_char("You can't do that here.\n\r",ch);
      return;
    }

  if (questman->position == POS_FIGHTING)
    {
      send_to_char("Wait until the fighting stops.\n\r",ch);
      return;
    }

  ch->questdata->questgiver = questman;

  if (!str_prefix(arg1, "list"))
    {
      act("$n asks $N for a list of quest items.", ch, NULL, questman, TO_ROOM); 
      act("You ask $N for a list of quest items.",ch, NULL, questman, TO_CHAR);
      send_to_char("Current Quest Items available for Purchase:\n\r", ch);
      if(reward_table[0].name == NULL)
    send_to_char("  Nothing.\n\r",ch);
      else {
    send_to_char("  [{WCost{w]       [{BName{w]\n\r",ch);
    for(i=0;reward_table[i].name != NULL;i++)
      printf_to_char(ch,"   {W%5d{w       {b%s{w\n\r",
             reward_table[i].cost,reward_table[i].name);
    send_to_char("\n\rTo buy an item, type 'Quest buy <item>'.\n\r",ch);
    printf_to_char(ch, "You currently have %d points available to spend.\n\r", ch->questdata->curr_points );
    return;
      }
    }
  else if (!strcmp(arg1, "buy"))
    {
      bool found=FALSE;

      if (arg2[0] == '\0')
      {
        send_to_char("To buy an item, type 'Quest buy <item>'.\n\r",ch);
        return;
      }

      for (i=0; reward_table[i].name != NULL; i++)
      {
        if (is_name(arg2, reward_table[i].keyword))
        { 
          found = TRUE;
          if (ch->questdata->curr_points >= reward_table[i].cost)
          {
            ch->questdata->curr_points -= reward_table[i].cost;

            if (reward_table[i].object)
              obj = create_object(get_obj_index(reward_table[i].value),ch->level);
            else
            {
              printf_to_char(ch,"In exchange for {&%d Quest Points{x, %s gives you %s.\n\r",
                  reward_table[i].cost, questman->short_descr, reward_table[i].name );
  
              *(int *)reward_table[i].where += reward_table[i].value;
            }
            mprintf(sizeof(buf), buf, "{g%s{w just bought %s for %d quest points.{x",
              ch->name, reward_table[i].name, reward_table[i].cost );
            log_string( buf );
            wiznet( buf, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );

            act("$n buys $p from $N.", ch, obj, questman, TO_ROOM); 

            break;
          }
          else
          {
            mprintf(sizeof(buf),buf, "'%s' Sorry, %s, but you don't have enough Quest Points for that.", ch->name, ch->name);
            do_function(questman,&do_whisper,buf);
            return;
          }
        }
      } // end 'for'

      if (!found)
      {
        mprintf(sizeof(buf),buf, "'%s' I don't have that item, %s.",ch->name, ch->name);
        do_function(questman,&do_whisper,buf);
      }

      if (obj != NULL)
      {
        printf_to_char(ch,"In exchange for {&%d Quest Points{x, %s gives you %s.\n\r",
             reward_table[i].cost, questman->short_descr, obj->short_descr);
	free_string( obj->owner );
	obj->owner = str_dup( ch->name, obj->owner );
        obj_to_char(obj, ch);
      }
      return;
    }
  else if (!str_prefix(arg1, "request"))
  {
    if (ch->level >= 5 && (IS_NULLSTR(ch->pcdata->history) || IS_NULLSTR(ch->description)))
      {
        printf_to_char( ch,
        "I'm sorry %s but you will need to describe yourself a little more to me.\n\rAlso tell me a little about your history before I allow you to quest any more.\n\r", ch->name);
        return;
      }

    if ( ch->position < POS_RESTING )
    {
      send_to_char( "You must be awake to do that.\n\r", ch );
      return;
    }

    {
      act("$n asks $N for a quest.", ch, NULL, questman, TO_ROOM); 
      act ("You ask $N for a quest.",ch, NULL, questman, TO_CHAR);

      if (IS_SET(ch->act,PLR_QUESTING))
        {
          mprintf(sizeof(buf),buf, "'%s' But you're already on a quest!", ch->name);
          do_function(questman,&do_whisper,buf);
          return;
        }
      if (ch->questdata->nextquest > 0)
        {
          mprintf(sizeof(buf),buf, "'%s' You're very brave, %s, but let someone else have a chance.",
              ch->name, ch->name);
          do_function(questman,&do_whisper,buf);
          mprintf(sizeof(buf),buf, "'%s' Come back later.", ch->name);
          do_function(questman,&do_whisper,buf);
          return;
        }

      mprintf(sizeof(buf),buf,"'%s' {&%s{c, thank you for coming.  I am in need of your assistance.{x",
        ch->name, ch->name);
      do_function(questman, &do_whisper, buf);
      ch->questdata->mob_vnum = ch->questdata->obj_vnum = 0;

      if (ch->level < LEVEL_HERO)
      {
          if (ch->questdata->quest_number >= MAX_QUEST_PER_LEVEL)
        {
            send_to_char("Sorry, you have reached your maximum amount of quests for your level.\n\r",ch);
            return;
          }
      }
      generate_quest(ch, questman);

      if (ch->questdata->mob_vnum > 0 || ch->questdata->obj_vnum > 0)
        {
          if (ch->questdata->countdown <= 0)
          {
            if (ch->level < 4)
              ch->questdata->countdown = number_range(25,60);
            else 
              ch->questdata->countdown = number_range(10,30);

            ch->questdata->time_allowed = ch->questdata->countdown;
//            if ( ch->questdata->mob_vnum == MOB_VNUM_QUEST )
//              ch->questdata->countdown = number_range(20,50);
          }

          ch->questdata->quest_number++;
          ch->questdata->attempt_num++;

          if (ch->questdata->quest_number >= MAX_QUEST_PER_LEVEL
          && ch->level < LEVEL_HERO) 
          {
            mprintf(sizeof(buf),buf, 
            "'%s' {&This will be your last Quest this level.{x\n\r", ch->name);
            do_function(questman,&do_whisper,buf);
          }

          SET_BIT(ch->act,PLR_QUESTING);
          mprintf(sizeof(buf),buf, 
          "'%s' {cYou have {&%d{c hours to complete this quest.",
            ch->name, ch->questdata->countdown);
          do_function(questman,&do_whisper,buf);
          mprintf(sizeof(buf),buf, "'%s' {cMay the gods go with you!", ch->name);
          do_function(questman,&do_whisper,buf);
        }

      return;
    }
  }

  else if (!str_prefix(arg1, "complete"))
  {
    if ( ch->position < POS_RESTING )
    {
      send_to_char( "You must be awake to do that.\n\r", ch );
      return;
    }


    act( "$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_ROOM); 
    act("You inform $N you have completed your quest.",ch, NULL,questman, TO_CHAR);

    if (ch->questdata->questgiver != questman)
    {
      mprintf(sizeof(buf),buf, "'%s' I never sent you on a quest! Perhaps you're thinking of someone else.", ch->name);
      do_function(questman,&do_whisper,buf);
      return;
    }

    if (IS_SET(ch->act,PLR_QUESTING))
    {
      bool obj_found = FALSE;
      if (ch->questdata->obj_vnum > 0 && ch->questdata->countdown > 0)
        {
          for (obj = ch->carrying; obj != NULL; obj= obj_next)
            {
          obj_next = obj->next_content;
        
          if (obj != NULL && obj->pIndexData->vnum == ch->questdata->obj_vnum)
            {
              obj_found = TRUE;
              break;
            }
        }
      }

      int bonus = 100;
      /* rescue mob quest success check */
      if (ch->questdata->mob_vnum == MOB_VNUM_QUEST)
      {
        CHAR_DATA *rch;
        for ( rch = ch->in_room->people ; rch ; rch = rch->next_in_room)
        {
          if ( !IS_NPC( rch ) )
            continue;

          sprintf( buf, "QQ%s",ch->name);
          if ( strstr(rch->name,buf)
          && ( rch->pIndexData->vnum == MOB_VNUM_QUEST) )
          {
            bonus = number_range(100,150);
            ch->questdata->mob_vnum = -1;
            break;
          }
        }
      }

      if ((ch->questdata->mob_vnum == -1 || (ch->questdata->obj_vnum && obj_found)) 
          && ch->questdata->countdown > 0)
        {
          int reward, pointreward, expgained;
          reward = number_range(25,100)*(bonus/100);
          pointreward = number_range(30,70)*(bonus/100);
          expgained = number_range(20,60)*(bonus/100);

          mprintf(sizeof(buf),buf, "'%s' Congratulations on completing your quest!",
            ch->name);
          do_function(questman,&do_whisper,buf);

          if (ch->questdata->time_allowed - ch->questdata->countdown > 0)
          {
            mprintf(sizeof(buf),buf, "'%s' You completed your quest in {W%d{D hour%s.",
              ch->name,
              ch->questdata->time_allowed - ch->questdata->countdown,
              ( ch->questdata->time_allowed - ch->questdata->countdown ) > 1 ? "s" : "" );
          }
          else
            mprintf(sizeof(buf),buf, "'%s' You completed your quest in less than one hour.", ch->name);
          do_function(questman,&do_whisper,buf);

          mprintf(sizeof(buf),buf,"'%s' As a reward, I am giving you {&%d Quest Points{D, and {Y%d {yg{Yo{yld{D.{x",
            ch->name,
            pointreward,
            reward);
          do_function(questman,&do_whisper,buf);

              if (ch->level <= LEVEL_HERO)
           {
                mprintf(sizeof(buf),buf,"'%s' You gain {g%d{D exp from your quest.{x",ch->name, expgained);
               do_function(questman,&do_whisper,buf);
           }

/*
 * This Code was commented out. I believe its from Abysmal Realms because it had their
 * color codes on the messages. I am not sure why they were using the other way that I
 * commented out below..but I think this looks better. So I'm adding it back in.
 * RWL 10-15-06
 */
        /*15% chance at highest mortal level, decreasing to 1% at lowest.*/
        chance = 15 - ( LEVEL_HERO - ch->level ) / ( LEVEL_HERO / 14 );
        chance = URANGE( 1, chance, 15 );

        bool rewarded = FALSE;
        int streak = 0;

        ch->questdata->streak++;

        if (ch->questdata->streak > ch->questdata->best_streak)
          ch->questdata->best_streak = ch->questdata->streak;

        streak = ch->questdata->streak;

        while ( streak > 100 )
          streak -= 100;

        if ( streak == 30 )
        {
          send_to_char( "{&You receive 1 streak train!\n\r{x", ch );
          act( "$n looks like $e has been rewarded for $s quest streak!", ch, NULL, NULL, TO_ROOM );
          ch->train += 1;
    
          mprintf(sizeof(buf3), buf3,
            "{g%s{w just gained 1 train for a quest streak. (%d total){x",
            ch->name, ch->train );
          log_string( buf3 );
          wiznet( buf3, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );
          rewarded = TRUE;
        }
        else if ( streak == 60 )
        {
          send_to_char( "{&You receive 2 streak trains!\n\r{x", ch );
          act( "$n looks like $e has been rewarded well for $s quest streak!", ch, NULL, NULL, TO_ROOM );
          ch->train += 2;

          mprintf(sizeof(buf3), buf3,
            "{g%s{w just gained 2 trains for a quest streak. (%d total){x",
            ch->name, ch->train );
          log_string( buf3 );
          wiznet( buf3, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );
          rewarded = TRUE;
        }
        else if ( streak == 100 )
        {
          char buf[MSL];

          sprintf( buf, "I just finished a 100 quest streak!%s", "" );
          do_function( ch, &do_info, buf );
          send_to_char( "{&You receive 4 streak trains!\n\r{x", ch );
          act( "$n looks like $e has been greatly rewarded for $s quest streak!", ch, NULL, NULL, TO_ROOM );
          ch->train += 4;

          mprintf(sizeof(buf3), buf3,
            "{g%s{w just gained 4 trains for a quest streak. (%d total){x.",
            ch->name, ch->train );
          log_string( buf3 );
          wiznet( buf3, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );
          rewarded = TRUE;
        }

        if (!rewarded
        &&   number_percent() <= chance
        &&   ch->questdata->streak != 29
        &&   ch->questdata->streak != 59
        &&   ch->questdata->streak != 99 )
        {
          int max_practices;
          int practices;

            /* 2 to 4 practices, increasing with level, w/a +1 bonuus at HERO. */
            max_practices = ch->level / ( MAX_LEVEL / 5 );
            max_practices = URANGE( 2, max_practices, 4 );

            if ( ch->level >= LEVEL_HERO )
              max_practices++;
          
            practices = number_range( 1, max_practices );
        
            printf_to_char( ch, "{&You receive %d practice%s!\n\r{x", practices, practices == 1 ? "" : "s" );
            act( "$n looks like $e's been practicing!", ch, NULL, NULL, TO_ROOM );

            ch->practice += practices;

            mprintf(sizeof(buf4), buf4,
              "{g%s{w just gained %d practice%s for a quest. (%d total){x",
              ch->name, practices,
              practices > 1 ? "s" : "",
              ch->practice );
          log_string( buf4 );
          wiznet( buf4, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );
          rewarded = TRUE;
        } // if they won some pracs
          
        if (!rewarded && (number_percent() > 99))
        {
          send_to_char( "{&You receive a jackpot train!\n\r{x", ch );
          act( "$n looks like $e hit the jackpot!", ch, NULL, NULL, TO_ROOM );

          ch->train += 1;

          mprintf(sizeof(buf4), buf4,
            "{g%s{w just gained 1 train for a quest jackpot. (%d total){x",
             ch->name, ch->train );
          log_string( buf4 );
          wiznet( buf4, NULL, NULL, WIZ_SECURE, 0, get_trust( ch ) );
          rewarded = TRUE;
        }

        clear_quest(ch,TRUE);
        ch->questdata->comp_num += 1;
        ch->questdata->nextquest = 10;
        ch->gold += reward;
        gain_exp(ch,expgained);
        ch->questdata->curr_points += pointreward;
        ch->questdata->accum_points += pointreward;
        return;
      }
      else if((ch->questdata->mob_vnum > 0 || ch->questdata->obj_vnum > 0)
          && ch->questdata->countdown > 0)
        {
          mprintf(sizeof(buf),buf, "'%s'You haven't completed the quest yet, but there is still time!", 
              ch->name);
          do_function(questman,&do_whisper,buf);
          return;
        }
    }
      if (ch->questdata->nextquest > 0)
    mprintf(sizeof(buf),buf,"'%s' You are not on a quest, You need to wait until I am ready to give you a quest.", ch->name);
      else 
    mprintf(sizeof(buf),buf, "'%s' You have to request a quest first, %s.",ch->name, ch->name);
      do_function(questman,&do_whisper,buf);
      return;
    }

  else if (!strcmp(arg1, "quit"))
    {
      act("$n informs $N $e wishes to quit $s quest.", ch,  NULL,questman, TO_ROOM); 
      act ("You inform $N you wish to quit $s quest.",ch, NULL,  questman, TO_CHAR);
      if (ch->questdata->questgiver != questman)
    {
      mprintf(sizeof(buf),buf, 
          "'%s' I never sent you on a quest! Perhaps you're thinking of someone else.", 
          ch->name);
      do_function(questman,&do_whisper,buf);
      return;
    }

      if (IS_SET(ch->act,PLR_QUESTING))
    {
      clear_quest(ch,TRUE);
      ch->questdata->nextquest = 20;
      mprintf(sizeof(buf),buf, 
          "'%s' Your quest is over, but for your cowardly behavior, you may not quest again for 20 hours.", 
          ch->name);
      do_function(questman,&do_whisper,buf);
      ch->questdata->streak=0;
      ch->questdata->quit_num += 1;
      return;
        }
      else
    {
      send_to_char("You aren't on a quest!\n\r",ch);
      return;
    } 
    }

  send_to_char("Quest commands: Info, Time, Request, Complete, Quit, List, and Buy.\n\r",ch);
  send_to_char("For more information, type 'Help Quest'.\n\r",ch);
  return;
}

#define MAX_QUEST_MOBS 250
void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
  CHAR_DATA *victim=NULL;
  CHAR_DATA *vsearch=NULL;
  CHAR_DATA *mob_array[MAX_QUEST_MOBS];
  ROOM_INDEX_DATA *room=0;
  OBJ_DATA *questitem;
  char buf [MAX_STRING_LENGTH];
  int nmobs = 0;
  bool rescuequest = FALSE;

  if (number_percent() < 10) // determine if it's a rescue quest
    rescuequest = TRUE; // have to check this here because of norescue flag

  for ( vsearch = char_list; vsearch; vsearch = vsearch->next )
  {    
    /* Don't want players or pets. */
    if ( !IS_NPC( vsearch )
    || IS_PET( vsearch ) )
      continue;

    /* Don't want mobs not in a room or area. */
    if ( vsearch->in_room == NULL)
      continue;

    /* Don't want a room the character can't go to. */
    if ( !can_see_room( ch, vsearch->in_room ) )
      continue;

    // Another instance of unlinked areas
    if ((vsearch->in_room->area->low_range == -1)
    ||  (vsearch->in_room->area->high_range == -1))
      continue;

    //Inq's don't need to get quests to kill good people
    if ( IS_CLASS_INQUISITOR(ch)
    &&  ( vsearch->alignment > 0 ) )
      continue;
      
    if (rescuequest)
    {
      bool doorfound = FALSE;
      int door;
      EXIT_DATA *pexit;

      if (IS_SET(vsearch->in_room->area->flags, AREA_NORESCUE))
        continue;

      for ( door = 0; door <= 5; door++ )
      {
        if ( ( pexit = vsearch->in_room->exit[door] )
        &&   IS_SET( pexit->exit_info, EX_ISDOOR ) )
          doorfound = TRUE;
      }

      if (!doorfound)
        continue;
    }

    if (!IN_RANGE(vsearch->in_room->area->low_range, ch->level, vsearch->in_room->area->high_range)
    &&  !IS_IMMORTAL(ch))
      continue;

    if ( ch->in_room->area->continent != vsearch->in_room->area->continent  )
      continue;

    if (IS_SET(vsearch->in_room->area->flags, AREA_NO_QUEST)
    ||  IS_SET(vsearch->in_room->area->flags, AREA_DRAFT))
      continue;

    if (!quest_level_diff(ch->level, vsearch->level))
      continue;
      
    if (IS_SET(vsearch->in_room->room_flags, ROOM_SAFE))
      continue;

    if (IS_SET(vsearch->act, ACT_TRAIN)
    || IS_SET(vsearch->act, ACT_PRACTICE)
    || IS_SET(vsearch->act, ACT_IS_HEALER)
    || IS_SET(vsearch->act, ACT_LOCKER)
    || IS_SET(vsearch->act, ACT_IS_CHANGER)
    || IS_SET(vsearch->act, ACT_NOQUEST)
    || IS_SET(vsearch->affected_by, AFF_CHARM)
    || IS_SHOPKEEPER(vsearch)
    || IS_SET(vsearch->affected_by, AFF_INVISIBLE)
    || number_percent() > 35)
      continue;

    mob_array[nmobs++] = vsearch;
    if (nmobs >= MAX_QUEST_MOBS)
      break;
  }

  if (nmobs >=1) {
    nmobs--;
    victim = mob_array[number_range(0,nmobs)];
  }
  else
  {
    mprintf(sizeof(buf),buf,  "'%s' I'm sorry, but I don't have any quests for you at this time.", ch->name);
    do_function(questman,&do_whisper,buf);
    mprintf(sizeof(buf),buf, "'%s' {&Try again later.", ch->name);
    do_function(questman,&do_whisper,buf);
    ch->questdata->nextquest = 2;
    return;
  }
  strcpy(buf,victim->name);
  room = victim->in_room;
  
  /*  if ((room = find_location( ch, buf ) ) == NULL ) */
  if (room == NULL)
  {
    mprintf(sizeof(buf),buf, "'%s' I'm sorry, but I don't have any quests for you at this time.",ch->name);
    do_function(questman,&do_whisper,buf);
    mprintf(sizeof(buf),buf, "'%s' {&Try again later.", ch->name);
    do_function(questman,&do_whisper,buf);
    ch->questdata->nextquest = 2;
    return;
  }

  clear_buf(ch->questdata->log);

  if ( rescuequest ) //Temp changed to always pick rescue
  { /* 10% chance for a rescue quest */
    int descnum = 0;

    descnum = number_range(0,NUM_QUEST_MOBS-1);

    victim = create_mobile( get_mob_index(MOB_VNUM_QUEST) );

    if ( descnum > -1)
    {
      if (victim->short_descr)
        free_string(victim->short_descr);
      if (victim->long_descr)
        free_string(victim->long_descr);
      if (victim->name)
        free_string(victim->name);

      mprintf(sizeof(buf),buf, "%s QQ%s",
        questmob_desc[descnum].name,
        ch->name); // so we can identify the proper rescuer

      victim->name        = str_dup(buf, victim->name);
      victim->long_descr  = str_dup(questmob_desc[descnum].long_descr,  victim->long_descr);
      victim->short_descr = str_dup(questmob_desc[descnum].short_descr, victim->short_descr);
      victim->sex         = questmob_desc[descnum].sex;
      SET_BIT(victim->act2,ACT2_NOWANDEROFF);
    }

    char_to_room(victim, room);

    ch->questdata->room_vnum = room->vnum;
    ch->questdata->mob_vnum = MOB_VNUM_QUEST;

    if (ch->alignment > -500)
      mprintf(sizeof(buf),buf, "'%s' {&%s{c has gotten lost!", ch->name, victim->short_descr);
    else
      mprintf(sizeof(buf),buf, "'%s' {cA very important person wants you to kidnap {&%s{c!", ch->name, victim->short_descr);

    do_function(questman,&do_whisper,buf);

    mprintf(sizeof(buf),buf, "'%s' {cThey were last seen in the general area of {&%s{c, at {&%s{c!{x",
      ch->name,
      room->area->name,
      room->name);
    do_function(questman,&do_whisper,buf);

      //For Quest Info
    bprintf( ch->questdata->log, "{W Target: {x%s\n\r", victim->short_descr);
    bprintf( ch->questdata->log, "{W Room:   {x%s\n\r", room->name );
    bprintf( ch->questdata->log, "{W Area:   {x%s\n\r", room->area->name );

    return;
// end rescue quest
  } /* Otherwise, 40% chance it will send the player on a 'recover item' quest. */
  else if ( ( number_percent() < 40 ) && ( ch->level > 3 ) )
  {
    int numobjs=0;
    int descnum = 0;

    numobjs= NUM_QUEST_OBJS - 1;
    descnum = number_range(0,numobjs);
    questitem = create_object( get_obj_index(OBJ_VNUM_QUEST), ch->level );
    questitem->owner = str_dup(ch->name,questitem->owner);
    if(descnum > -1)
    {
      if(questitem->short_descr)
        free_string(questitem->short_descr);
      if(questitem->description)
        free_string(questitem->description);
      if(questitem->name)
        free_string(questitem->name);

      questitem->name        = str_dup(quest_desc[descnum].name, questitem->name);
      questitem->description = str_dup(quest_desc[descnum].long_descr, questitem->description);
      questitem->short_descr = str_dup(quest_desc[descnum].short_descr, questitem->short_descr);
    }

    obj_to_room(questitem, room);
    ch->questdata->room_vnum = room->vnum;
    ch->questdata->obj_vnum = questitem->pIndexData->vnum;

    mprintf(sizeof(buf),buf, "'%s' {cVile pilferers have stolen {&%s{c from the royal treasury!",ch->name, questitem->short_descr);
    do_function(questman,&do_whisper,buf);
    mprintf(sizeof(buf),buf,"'%s' {cMy court wizardess, with her magic mirror, has pinpointed its location.", ch->name);
    do_function(questman, &do_whisper, buf);

    mprintf(sizeof(buf),buf, "'%s' {cLook in the general area of {&%s{c for {&%s{c!{x",ch->name, room->area->name, room->name);
    do_function(questman,&do_whisper,buf);

    //For Quest Info
    bprintf( ch->questdata->log, "{W Target: {x%s\n\r", questitem->short_descr);
    bprintf( ch->questdata->log, "{W Room:   {x%s\n\r", room->name );
    bprintf( ch->questdata->log, "{W Area:   {x%s\n\r", room->area->name );
    return;
  }
  else /* if ( number_percent() < 80 ) -- 10% chance for mob rescue quest */
  { /* Quest to kill a mob */
    switch (number_range(0,1))
    {
      case 0:
        mprintf(sizeof(buf),buf, "'%s' {cAn enemy of mine, {&%s{c, is making vile threats against the crown.",ch->name,victim->short_descr);
        do_function(questman,&do_whisper,buf);
        mprintf(sizeof(buf),buf, "'%s' {cThis threat must be eliminated!", ch->name);
        do_function(questman,&do_whisper,buf);
        bprintf( ch->questdata->log, "{W Target:{x %s\n\r", victim->short_descr);
        break;
      case 1:
        mprintf(sizeof(buf),buf, "'%s' {cA most heinous criminal, {&%s{c, has escaped from the dungeon!", ch->name, victim->short_descr);
        do_function(questman,&do_whisper,buf);
        mprintf(sizeof(buf),buf, "'%s' {cSince the escape, {&%s{c has murdered %d civillians!", ch->name, victim->short_descr, number_range(2,20));
        do_function(questman,&do_whisper,buf);
        mprintf(sizeof(buf),buf, "'%s' {cThe penalty for this crime is death, and you are to deliver the sentence!", ch->name);
        do_function(questman, &do_whisper,buf);
        bprintf( ch->questdata->log, "{W Target:{x %s\n\r", victim->short_descr);
        break;
    }

    if (room->name != NULL)
    {
      mprintf(sizeof(buf),buf, "'%s' {cSeek {&%s{c out somewhere in the vicinity of {&%s!", ch->name, victim->short_descr,room->name);
      do_function(questman,&do_whisper,buf);
      mprintf(sizeof(buf),buf, "'%s' {cThat location is in the general area of {&%s{c.{x", ch->name, room->area->name);
      do_function(questman,&do_whisper,buf);
      bprintf( ch->questdata->log, "{W Room:{x   %s\n\r", room->name );
      bprintf( ch->questdata->log, "{W Area:{x   %s\n\r", room->area->name );
    }

    ch->questdata->mob_vnum = victim->pIndexData->vnum;
  }

  return;
}

bool quest_level_diff(int clevel, int mlevel)
{
  int tempint=0;

  if (clevel <= 90)
  {
    tempint = clevel + (clevel/10);

    if (IN_RANGE(tempint - 4,mlevel,tempint + 2))
      return TRUE;
    else
      return FALSE;
  }
 
  if ( clevel >= 95 )
  {
    if ( mlevel >= 95 )
        return TRUE;
    else
        return FALSE;
  }

  if ( mlevel >= clevel )
    return TRUE;
  else
    return FALSE;
}
        
/* Called from update_handler() by pulse_area */

void quest_update(void)
{
  CHAR_DATA *vch;

  for ( vch = player_list; vch; vch = vch->next_player )
    {
      if(IS_NPC(vch))
    continue;

      if (IS_LINKDEAD(vch))
    continue;

      if ( (IS_AFK(vch))
      && (vch->timer > 1) )
        continue;

    if (vch->questdata->nextquest > 0)
    {
      vch->questdata->nextquest--;
      if (vch->questdata->nextquest <= 0)
      {
          if (vch->questdata->quest_number < MAX_QUEST_PER_LEVEL || IS_HERO(vch))
            send_to_char("{&You may now quest again{x.\n\r",vch);
          else
            send_to_char("You may quest again once you have reached your next level.\n\r", vch);
      }
    }
    else if (IS_SET(vch->act,PLR_QUESTING))
    {
      --vch->questdata->countdown;
      if ( vch->questdata->countdown <= 0 )
      {
          vch->questdata->nextquest = 10;
          printf_to_char(vch,
                 "You have run out of time for your quest!\n\rYou may quest again in {R%d{x hours.\n\r",
                 vch->questdata->nextquest);
          vch->questdata->streak=0;
          vch->questdata->failed++;
          clear_quest(vch,TRUE);
      }
      if (vch->questdata->countdown > 0 && vch->questdata->countdown < 6)
      {
          send_to_char("{&Better hurry{x, you're almost out of time for your quest!{x\n\r",vch);
      }
      else
      {
          send_to_char("\n\r",vch);
      }
    }
    else
    {
      vch->questdata->nextquest = 0;
      if (vch->questdata->countdown > 0)
      {
          bugf( "[%s]Players countdown is %d, and is not set to QUESTING.%s.\n\r",
            vch->name, vch->questdata->countdown, interp_cmd);

          /* Doing a temp fix. */
          SET_BIT(vch->act,PLR_QUESTING);
      }
    }
  }
}  

void extract_quest_object( CHAR_DATA *ch )
{
  OBJ_DATA *obj;

  for ( obj = object_list ; obj ; obj = obj->next )
  {
    if ( ch->questdata && obj->owner &&
         obj->pIndexData->vnum == ch->questdata->obj_vnum &&
         !strcmp( obj->owner, ch->name ) )
    {
      extract_obj( obj );
      return;
    }
  }
}

void extract_quest_mobile( CHAR_DATA *ch )
{
  char buf[MSL];
  CHAR_DATA *rch;
  CHAR_DATA *nextrch;

  for ( rch = char_list ; rch ; rch = nextrch )
  {
    nextrch = rch->next;
    if ( !IS_NPC( rch ) )
      continue;

    sprintf( buf, "QQ%s",ch->name);
    if ( strstr(rch->name,buf )
    && ( rch->pIndexData->vnum == MOB_VNUM_QUEST ) )
    {
      do_function(rch,&do_follow,"self");
      do_function(rch,&do_nofollow,"");
      extract_char( rch, TRUE );
    }
  }
}

void clear_quest( CHAR_DATA *ch, bool qPull )
{
  extract_quest_object( ch );
  if (qPull)
    extract_quest_mobile( ch );
  REMOVE_BIT(ch->act,PLR_QUESTING);
  clear_buf( ch->questdata->log );
  ch->questdata->questgiver         = NULL;
  ch->questdata->countdown          = 0;
  ch->questdata->time_allowed       = 0;
  ch->questdata->mob_vnum           = 0;
  ch->questdata->obj_vnum           = 0;
  ch->questdata->room_vnum          = 0;
}

void do_questlist(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *vch;
  bool found = FALSE;

  for ( vch = player_list; vch; vch = vch->next_player )
  {
    if (IS_SET(vch->act, PLR_QUESTING))
    {
      if (!found)
        send_to_char("{WQuests          {YTim {RType  {CGoal  {BRoom\n\r{c=============== === ==== ===== ====={x\n\r",ch);

      printf_to_char(ch,"{W%-15s  {Y%2d ", vch->name, vch->questdata->countdown);

      if ( vch->questdata->mob_vnum == -1 )
        printf_to_char(ch,"{RMob   {xAlmost complete{x\n\r");
      if (vch->questdata->mob_vnum == MOB_VNUM_QUEST)
        printf_to_char(ch,"{RResc {C%5d {B%5d{x\n\r",vch->questdata->mob_vnum, vch->questdata->room_vnum);
      else if (vch->questdata->mob_vnum > 0)
        printf_to_char(ch,"{RMob  {C%5d {B%5d{x\n\r",vch->questdata->mob_vnum, vch->questdata->room_vnum);
      if ( vch->questdata->obj_vnum == -1 )
        printf_to_char(ch,"{RObj   {xAlmost complete{x\n\r");
      else if (vch->questdata->obj_vnum > 0)
        printf_to_char(ch,"{RObj  {C%5d {B%5d{x\n\r",vch->questdata->mob_vnum, vch->questdata->room_vnum);
/*
      printf_to_char(ch,"{YName: %s Obj: %d Mob: %d  Room: %d Time %d{x\n\r",
        vch->name,
        vch->questdata->obj_vnum,
        vch->questdata->mob_vnum,
        vch->questdata->room_vnum,
        vch->questdata->countdown);

*/
      printf_to_char(ch,"%s{x\n\r",buf_string(vch->questdata->log));
      found = TRUE;
    }
  }

  if (!found)
    send_to_char("There are no active quests.\n\r",ch);

  return;
}

/*

@ The Back Room of the Inn
[rm 8446] ==]========- questlist
QUESTLIST:
========================================================================
Name: Taeloch Obj: 0 Mob: 7522  Room: 0 Time 23
 Target: a studying apprentice
 Room:   North western study hall
 Area:   Troll Cavern


@ North eastern study hall
[rm 7597] ==]========- questlist
QUESTLIST:
========================================================================
Name: Taeloch Obj: 0 Mob: -1  Room: 0 Time 22
 Target: a studying apprentice
 Room:   North western study hall
 Area:   Troll Cavern

*/
