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
 **************************************************************************/

/***************************************************************************
*    ROM 2.4 is copyright 1993-1996 Russ Taylor                            *
*    ROM has been brought to you by the ROM consortium                     *
*        Russ Taylor (rtaylor@efn.org)                                     *
*        Gabrielle Taylor                                                  *
*        Brian Moore (zump@rom.org)                                        *
*    By using this code, you have agreed to follow the terms of the        *
*    ROM license, in the file Rom24/doc/rom.license                        *
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
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "interp.h"
bool has_curse(char *s, int bitname);
char *lowerize( const char *str );

/* RT code to delete yourself */
void auction_channel ( char * msg );
void show_gocial (CHAR_DATA *ch, char *arg, int cmd, char *type, const int bitname, 
         enum special_flags spec_flag, const int channel, char color_code);
bool check_channel_view (CHAR_DATA *ch, DESCRIPTOR_DATA *d, const int bitname, enum special_flags spec_flag);
void update_chan_hist (CHAR_DATA *ch, const int channel, char buf[MAX_INPUT_LENGTH+200]);
void show_chan_hist (CHAR_DATA *ch, const int channel);
void check_4_talk_cheating (CHAR_DATA *ch, char *s);
void reclaim_eq (CHAR_DATA *ch);
char as_string[MAX_NUM_AUTOSNOOP][70];

void do_delet( CHAR_DATA *ch, char *argument)
{
  send_to_char("You must type the full command to delete yourself.\n\r",ch);
}

/* New Delete */
void do_delete( CHAR_DATA *ch, char *argument)
{
  char strsave[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  CLAN_DATA *old_clan = NULL;

  if( IS_NPC(ch) )
  {
    send_to_char( "Yeah, right. Mobs can't delete themselves.\n\r", ch );
    return;
  }
  
  if ( argument[0] == '\0' )
  {
    send_to_char("Syntax: delete [password]\n\r",ch);
    return;
  }

  if ( !str_cmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
  {
    send_to_char("Typing delete cancel will cancel your impending deletion.\n\r",ch);
    send_to_char("Typing delete confirm will delete your character.\n\r",ch);
    send_to_char("WARNING: Remember, this command is irreversible.\n\r",ch);
    ch->pcdata->confirm_delete = TRUE;
    mprintf( sizeof(buf), buf, "%s is contemplating deletion.",
      capitalize( ch->name ) );
    wiznet(buf,ch,NULL,0,0,get_trust(ch));
    free_string(ch->cmd_hist[0].text);
    ch->cmd_hist[0].text = str_dup("delete *********", ch->cmd_hist[0].text);
  }
  
     
  if ( ch->pcdata->confirm_delete )
  {
    if ( !str_cmp( argument, "cancel" ) )
    {
      send_to_char("Delete status removed.\n\r",ch);
      mprintf( sizeof(buf), buf, "%s's will to fight keeps $s in the realm.",
        capitalize( ch->name ),
        ch->sex == 2 ? "her" : "him" );
      wiznet(buf,ch,NULL,0,0,0);
      ch->pcdata->confirm_delete = FALSE;
      return;
    }
    else if ( !str_cmp( argument, "confirm" ) )
    {
      mprintf( sizeof(buf), buf, "%s turns %sself into line noise.",
        capitalize( ch->name ),
        ch->sex == 2 ? "her" : "him" );

      wiznet(buf,ch,NULL,0,0,0);


      stop_fighting(ch,TRUE);
      reclaim_eq(ch);

      if (ch->clan != NULL) {
        old_clan = ch->clan;
        remove_clannie(ch);
        save_clan( old_clan );
        save_clan_list(ROSTER_FILE);
      }

      do_function(ch, &do_quit,"confirm");

      mprintf( sizeof(strsave), strsave, "%s%s",
        PLAYER_DIR,
        capitalize( ch->name ) );

      unlink(strsave); // delete pfile
      return;
    }
    else
    {
      send_to_char("Either type delete cancel or delete confirm.\n\r",ch);
      return;
    }
  }
}

/* take all a player's EQ and put it into a pit - Taeloch */
void reclaim_eq( CHAR_DATA *ch )
{
  int iWear, pit_vnum = 0;
  OBJ_DATA *obj, *pit, *obj_next;

  if ( IS_IN_CLAN( ch )
  && ( !IS_SET( ch->clan->clan_flags, CLAN_INDEPENDENT ) ) )
  {
    pit_vnum = ch->clan->donation_obj[ch->in_room->area->continent];
    if (pit_vnum == 0) // default to Kaishaan pit
      pit_vnum = ch->clan->donation_obj[0];
  }
  if (pit_vnum == 0)
  {
    if ( IS_EVIL( ch ) )
      pit_vnum = OBJ_VNUM_EVIL_DONATE;
    else if ( IS_GOOD( ch ) )
      pit_vnum = OBJ_VNUM_GOOD_DONATE;
    else
      pit_vnum = OBJ_VNUM_NEUT_DONATE;
  }

  pit = find_obj_vnum( pit_vnum );

  if (pit == NULL)
    return; // screw it, just let it all disappear

  if ( ( obj = get_eq_char(ch, WEAR_WIELD)) != NULL )
    unequip_char( ch, obj );

  for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    if ( ( obj = get_eq_char( ch, iWear ) ) != NULL )
      unequip_char( ch, obj );

  for ( obj = ch->carrying; obj != NULL; obj = obj_next )
  {
    obj_next = obj->next_content;
    if ( ( obj->timer <= 0 )
    &&   ( obj->item_type != ITEM_MONEY ) 
    &&   !IS_SET(obj->extra_flags, ITEM_RESTRING)
    &&   ( obj->item_type != ITEM_CORPSE_NPC )
    &&   ( obj->item_type != ITEM_CORPSE_PC ) )
    {
      /* check if not clan item */
      if (obj->pIndexData->clan_name == NULL)
      {
        obj_from_char( obj );
        obj_to_obj( obj, pit );
      }
/*

      if (
        &&  (!strcmp( obj->pIndexData->clan_name, "none" )
          ||  
          ||  (obj->pIndexData->clan_name == &str_empty[0]) ))
*/
    }
  }
}

/* RT code to display channel status */
void do_channels( CHAR_DATA *ch, char *argument)
{
  if (IS_SET(ch->pen_flags, PEN_NOCHANNELS))
    send_to_char("Your CHANNELS have been revoked by the GODS.\n\r",ch);
  
  /* lists all channels and their status */
  send_to_char("{WChannel          Status     Visibility{x\n\r",ch);
  send_to_char("{C------------------------------------------{x\n\r",ch);

  send_to_char( "{gSay{x              Perm       Room\n\r", ch );
  send_to_char( "{cWhisper{x          Perm       Room, Personal\n\r", ch);
  send_to_char( "{RYell{x             Perm       Area\n\r", ch ); 

  printf_to_char(ch,"%s %3s        World\n\r",   "{W[{cGossip{W]{x        ",
    !IS_SET(ch->chan_flags,CHANNEL_NOGOSSIP)? "ON ":"OFF");
  printf_to_char(ch,"%s %3s        World\n\r",   "{W[{yAuction{W]{x       ",
    !IS_SET(ch->chan_flags,CHANNEL_NOAUCTION)? "ON ":"OFF");
  printf_to_char(ch,"%s %3s        World\n\r",   "{W[{mMusic{W]{x         ",
    !IS_SET(ch->chan_flags,CHANNEL_NOMUSIC)? "ON ":"OFF");
  printf_to_char(ch,"%s %3s        World\n\r",   "{W[ {G? {W]{x           ",
    !IS_SET(ch->chan_flags,CHANNEL_NOQUESTION)? "ON ":"OFF");
  printf_to_char(ch,"%s %3s        World\n\r",   "{W[{wQuote{W]{x         ",
    !IS_SET(ch->chan_flags,CHANNEL_NOQUOTE)? "ON ":"OFF");
  printf_to_char(ch,"%s %3s        World\n\r",   "{W[{YGrats{W]{x         ",
    !IS_SET(ch->chan_flags,CHANNEL_NOGRATS)? "ON ":"OFF");
  printf_to_char(ch,"%s %3s        World\n\r",   "{MShout{x           ",
    !IS_SET(ch->chan_flags,CHANNEL_SHOUTSOFF)? "ON ":"OFF");
  if (IS_IMMORTAL(ch))
    printf_to_char(ch,"%s %3s        World\n\r",   "{c[{yIMMTALK{c]{x       ",
      !IS_SET(ch->chan_flags,CHANNEL_NOWIZ)? "ON ":"OFF");
  printf_to_char(ch,"%s %3s        Personal\n\r","{yTells{x           ",
    !IS_SET(ch->chan_flags,CHANNEL_DEAF)? "ON ":"OFF");
  printf_to_char(ch,"%s %3s        World\n\r",   "{W[{BOOC{W]{x           ",
    !IS_SET(ch->chan_flags,CHANNEL_NOOOC)? "ON ":"OFF");
  printf_to_char(ch,"%s %3s        World\n\r",   "{W[{RIRL{W]{x           ",
    !IS_SET(ch->chan_flags,CHANNEL_NOIRL)? "ON ":"OFF");
  printf_to_char(ch,"%s Perm       World\n\r",   "{w[{rInfo{w]{x          ");

  if ( IS_IN_CLAN( ch ) )
    printf_to_char(ch,"{w%s{x        %3s        World, Clan\n\r",
      ch->clan->symbol,
      !IS_SET(ch->chan_flags,CHANNEL_NOCLAN)? "ON ":"OFF");
  else
    printf_to_char(ch,"%s %s\n\r","{W[{BClan{W]{x",
      !IS_SET(ch->chan_flags,CHANNEL_NOCLAN)? "ON ":"OFF");

  printf_to_char(ch,"%s %s        World\n\r",    "{W[{DWarTalk{W]{x       ",
    !IS_SET(ch->chan_flags,CHANNEL_NOWAR)? "ON ":"OFF");
  printf_to_char(ch,"%s %s        World\n\r",    "{W[{CPolitical{W]{x     ",
    !IS_SET(ch->chan_flags,CHANNEL_NOPOLITIC)? "ON ":"OFF");

  printf_to_char(ch,"%-16s %s\n\r","Quiet Mode",
    !IS_SET(ch->chan_flags,CHANNEL_QUIET)? "OFF":"ON ");

  if (IS_AFK(ch))
    send_to_char("You are AFK.\n\r",ch);

  if (IS_SET(ch->pen_flags,PEN_SNOOP_PROOF))
    send_to_char("\n\rYou are immune to snooping.\n\r",ch);
   
  if (ch->lines != PAGELEN)
  {
    if (ch->lines)
      printf_to_char(ch,"You display %d lines of scroll.\n\r",ch->lines+2);
    else
      send_to_char("Scroll buffering is off.\n\r",ch);
  }

  if (ch->prompt != NULL)
    printf_to_char(ch,"Your current prompt is: %s\n\r",ch->prompt);

  if (IS_SET(ch->chan_flags,CHANNEL_NOSHOUT))
    send_to_char("You cannot shout.\n\r",ch);
  
  if (IS_SET(ch->chan_flags,CHANNEL_NOTELL))
    send_to_char("You cannot use tell.\n\r",ch);
 
  if (IS_SET(ch->chan_flags,CHANNEL_NOTELL))
    send_to_char("You cannot use tell.\n\r",ch);

  if (IS_SET(ch->comm_flags,COMM_NO_SLEEP_TELLS))
    send_to_char("You cannot receive tells when asleep.\n\r",ch);

  if (IS_SET(ch->pen_flags,PEN_NOCHANNELS))
    send_to_char("You cannot use channels.\n\r",ch);

  if (IS_SET(ch->chan_flags,CHANNEL_NOEMOTE))
    send_to_char("You cannot show emotions.\n\r",ch);
}

/* Taeloch - allow players to turn off sleeping tells */
void do_sleeptells( CHAR_DATA *ch, char *argument)
{
  if (IS_SET(ch->comm_flags,COMM_NO_SLEEP_TELLS))
  {
    send_to_char("You can now receive tells in your sleep.\n\r",ch);
    REMOVE_BIT(ch->comm_flags,COMM_NO_SLEEP_TELLS);
  }
  else
  {
    send_to_char("You can no longer receive tells in your sleep.\n\r",ch);
    SET_BIT(ch->comm_flags,COMM_NO_SLEEP_TELLS);
  }
}


/* RT deaf blocks out all shouts */
void do_deaf( CHAR_DATA *ch, char *argument)
{
  if (IS_SET(ch->chan_flags,CHANNEL_DEAF))
  {
    send_to_char("You can now hear tells again.\n\r",ch);
    REMOVE_BIT(ch->chan_flags,CHANNEL_DEAF);
  }
  else 
  {
    send_to_char("You will no longer hear tells.\n\r",ch);
    SET_BIT(ch->chan_flags,CHANNEL_DEAF);
  }
}

/* RT quiet blocks out all communication */
void do_quiet ( CHAR_DATA *ch, char * argument)
{
  if (IS_SET(ch->chan_flags,CHANNEL_QUIET))
  {
    send_to_char("Quiet mode removed.\n\r",ch);
    REMOVE_BIT(ch->chan_flags,CHANNEL_QUIET);
  }
  else
  {
    send_to_char("From now on, you will only hear says and view actions.\n\r",ch);
    SET_BIT(ch->chan_flags,CHANNEL_QUIET);
  }
}

void do_seeall ( CHAR_DATA *ch, char * argument)
{
  if (IS_SET(ch->chan_flags,CHANNEL_ALL))
  {
    send_to_char("VIEW ALL Public Channels mode removed.\n\r",ch);
    REMOVE_BIT(ch->chan_flags,CHANNEL_ALL);
  }
  else
  {
    send_to_char("From now on, you will hear all public channels.\n\r",ch);
    SET_BIT(ch->chan_flags,CHANNEL_ALL);
  }
}

void do_newbie ( CHAR_DATA *ch, char * argument)
{
  if (!IS_SET(ch->chan_flags,CHANNEL_NEWBIE))
  {
    send_to_char("The {gnewbie {chint{x channel has been removed.\n\r",ch);
    SET_BIT(ch->chan_flags,CHANNEL_NEWBIE);
  }
  else
  {
    send_to_char("From now on, you will hear the {gnewbie {chint{x channel.\n\r",ch);
    REMOVE_BIT(ch->chan_flags,CHANNEL_NEWBIE);
  }
}

/* afk command */
void do_afk ( CHAR_DATA *ch, char * argument)
{
  CHAR_DATA *gch;
  char title[MSL];
  char buf[MSL];
  int autoafk = 0;

  if ( ch->fighting != NULL )
  {
    send_to_char( " You're {rfighting{x!{x\n\r", ch );
    return;
  }

  if ( IS_NPC( ch ) )
  {
    send_to_char( "Cannot go AFK for a MOB.\n\r", ch );
    return;
  }

  if ( !IS_NULLSTR( argument ) )
  {
    if (is_number(argument))
    {
      autoafk = atoi(argument);

      if (autoafk < MIN_AFK_TIME)
      {
        printf_to_char( ch, "Minimum AFK time is %d.\n\r", MIN_AFK_TIME);
        return;
      }
      if (autoafk > MAX_AFK_TIME)
      {
        printf_to_char( ch, "Maximum AFK time is %d.\n\r", MAX_AFK_TIME);
        return;
      }

      ch->pcdata->afktime = autoafk;
      printf_to_char( ch, "AFK Time set to: %d ticks.\n\r", autoafk);
      return;
    }

    smash_tilde( argument );
    strcpy( title, argument );
    
    if ( colorstrlen( title ) > 45 )
    {
      send_to_char("AFK Title is too long.  Please shorten it.\n\r",ch);
      return;
    }
    else
      strcat(title,"{x\0");

    strip_color( buf, title );
    if ( !strstr( buf, "AFK" )  )
    {
      send_to_char( "You must have AFK somewhere in the title.\n\r", ch );
      return;
    }

    if ( title[0] == '\0' )
      return;

    set_titleafk( ch, title );
    printf_to_char( ch, "AFK Title set to: %s%s\n\r",
      ch->name,
      ch->pcdata->afk_title );

    return;
  }

  if ( IS_AFK( ch ) )
  {
    if (ch->tells > 0)
    {
      printf_to_char( ch, "AFK mode removed. You have %d tell%s waiting (type {greplay{x to view).\n\r",
        ch->tells, ( ch->tells == 1 ? "" : "s") );
    }
    else
      printf_to_char( ch, "AFK mode removed.\n\r");

    act( "{w$n {xis now {MALIVE{x again", ch, NULL, NULL, TO_WIZ_ROOM );
    REMOVE_BIT( ch->comm_flags, COMM_AFK );
  }
  else
  {
    send_to_char( "You are now in AFK mode.\n\r", ch );
    act( "$n is now AFK.", ch, NULL, NULL, TO_WIZ_ROOM);

    for ( gch = player_list; gch != NULL; gch = gch->next_player )
    {
      if (IS_SET(gch->comm_flags,CHANNEL_QUIET))
        continue;
      if (IS_NPC(gch))
        continue;
      if (gch == ch)
        continue;
      if ( is_same_group( gch, ch ) )
      {
        do_function(ch, &do_gtell, "I am AFK (Automessage)");
        break;
      }
    }

    SET_BIT(ch->comm_flags,COMM_AFK);

    if (ch->idle_snapshot == 0)
      ch->idle_snapshot = current_time;
  }
}

void do_replay (CHAR_DATA *ch, char *argument)
{
  if (IS_NPC(ch))
  {
    send_to_char("You can't replay.\n\r",ch);
    return;
  }

  if (ch->tells == 0)
  {
    send_to_char("You have no tells to replay.\n\r",ch);
    return;
  }

  if (ch->pcdata->buffer == NULL && ch->tells)
  {
    send_to_char("You have no tells to replay.\n\r",ch);
    bugf("do_replay, ch->tells = %d, and yet nothing to tell.\n\r",ch->tells);
    return;
  }

  page_to_char(buf_string(ch->pcdata->buffer),ch);
  clear_buf(ch->pcdata->buffer);
  ch->tells = 0;
}

void replace_color(char *argument, char color)
{
  int x = 0;
  
  if (argument[0]=='\0')
    return;

  for (x = 0; x < strlen(argument)-3; x++) 
  {
    if (argument[x] =='\0')
      return;
    if (argument[x+1] =='\0')
      return;
    if( argument[x] == '{' )
    {
      if (argument[x+1] == '{')
      {
        if (argument[x+2] == '\0')
          return;
        else
        {
          x += 2;
          continue;
        }
      }
      if ((argument[x + 1] == 'x') || 
          (argument[x + 1] == 'X'))
      {
        x++;
        argument[x] = color;
      }
    }
  }            
}

/*
  public_ch replaces repetive code from gossip, auction, music, gratz, quote
  and Q/A channels... BY Blade of "- E - "
  this snipet is freeware.. this snippet uses lope's color code,
  configure/modify at your liesure...
  version 1.31 with the guidance of erwin. thnx
  put this in your act_comm.c to replace the current do_gossip and such
  commands, make sure that all functions calling public_ch are located
  after public_ch in the file... thnx.
  !! now includes emote support on channels. !!
  !! "wholist" internal command allows player to see who is on a channel,
  while retaining the functionality of beginign sentences with "who" !!
  now includes text wrapping features - string.c required (olc file)
*/
void public_ch ( CHAR_DATA *ch, char *argument, char *atype, const int bitname,
                 enum special_flags spec_flag, const int channel, char color_code)
{
  /* since the passed string can never be more than max_input_length, why use max_string length? */
  char buf[MAX_INPUT_LENGTH + 200];
  char buf1[MSL],buf2[MSL];
  char act1[MSL],act2[MSL];
  char command[MAX_INPUT_LENGTH + 100];
  DESCRIPTOR_DATA *d;
  int cmd =0;
  bool display_to_char = FALSE;
  bool emot = FALSE;
  bool pmot = FALSE;
  bool social = FALSE;
  bool display_wholist=FALSE;
  char arg_left[MSL];
  CHAR_DATA *victim=NULL;
  char type[MSL];
//  int v1;
  char *letter,*name;
  char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
  int matches = 0;

  strcpy(type,atype);

  if (argument[0] == '\0' )
  {
    show_chan_hist(ch, channel);
    return;
  }
  else  
  {
    if (IS_SET(ch->chan_flags,CHANNEL_QUIET)
    && (channel != CHAN_GROUPTELL)
    && (channel != CHAN_PRAYER) )
    {
      send_to_char("You must turn off quiet mode first.\n\r",ch);
      return;
    }
 
    if (IS_SET(ch->pen_flags,PEN_NOCHANNELS) )
    {
      send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
      return;
    }

    strcpy(arg_left,argument);

    if (has_curse(arg_left, bitname)
    && (channel != CHAN_CLAN)
    && (channel != CHAN_GROUPTELL)
    ) // allow it on clan chan
    {
      send_to_char("That line is not permitted due to foul language rules.\n\r",ch);
      return;
    }

    check_4_talk_cheating(ch,arg_left);
    argument = one_argument( argument, command );
    social = FALSE;

    if (!strstr(argument," "))
    {
      for (cmd = 0; social_table[cmd].name[0] != '\0'; cmd++)
      {
        if (command[0] == social_table[cmd].name[0]
        && !str_cmp( command,social_table[cmd].name ) )
        {
          social = TRUE;
          break;
        }
      }
    }

    if (IS_IMMORTAL(ch) && channel == CHAN_PRAYER)
    {
      victim = get_char_world_ordered( ch, command, "chars" );

      if ( (victim == NULL)
      ||   IS_NPC(victim) )
      {
        send_to_char("They aren't here.\n\r", ch);
        return;
      }
      if (IS_IMMORTAL(victim))
      {
        send_to_char("Use Immtalk channel.\n\r", ch);
        return;
      }
      strcpy(arg_left, argument);
      mprintf(sizeof(type), type, "{c[{W%s answers %s's Prayers{c]{W", ch->name, victim->name);
    }

    if (social) {
      if ( ( get_char_world(ch,argument) != NULL)
      ||     IS_NULLSTR(argument) )
      {
        show_gocial(ch, argument, cmd, type, bitname, spec_flag, channel, color_code);
        return;
      }
    }

    if (!str_cmp(command,"emote"))
      emot = TRUE;
    else if (!str_cmp(command,"pmote"))
      pmot = TRUE;
    else if (!str_cmp(command, "wholist"))
    {
      display_wholist=TRUE;
      printf_to_char(ch,"{WPlayers on %s{x\n\r", type);
      send_to_char("{c-------------------{x\n\r",ch);
    }
    else if (!str_cmp(command, "toggle"))
    {
      if (IS_SET(ch->chan_flags,(bitname)))
      {
        printf_to_char(ch, "%s channel is now ON.{x\n\r", type);
        REMOVE_BIT(ch->chan_flags,(bitname));
        return;
      }
      else
      {
        printf_to_char(ch, "%s channel is now OFF.{x\n\r", type);
        SET_BIT(ch->chan_flags,(bitname));
        return;
      }
    }

    if ( IS_SET( ch->chan_flags, bitname ) )
    {
      send_to_char( "You don't have that channel on.\n\r", ch );
      return;
    }

    if (emot)
    {
      if (IS_NPC(ch))
        mprintf( sizeof(buf), buf, "%s You emote : %s %s{x",
          type,
          ch->short_descr,
          argument );
      else
        mprintf( sizeof(buf), buf, "%s You emote : %s %s{x",
          type,
          ch->name,
          argument );

      if (IS_NPC(ch))
        mprintf( sizeof(buf1), buf1, "%s %s %s{x",
          type,
          ch->short_descr,
          argument );
      else
        mprintf( sizeof(buf1), buf1, "%s %s %s{x",
          type,
          ch->name,
          argument );

      if (IS_IMMORTAL(ch) && (!IS_SWITCHED(ch)))
        mprintf( sizeof(buf2), buf2, "%s An Unknown Entity %s{x",
          type,
          argument);
      else
        mprintf( sizeof(buf2), buf2, "%s Someone %s{x",
          type,
          argument);

      /*act_new( "$t{x", ch, buf, NULL,TO_CHAR,POS_SLEEPING );*/
      replace_color(buf,color_code);
      act_channels( "$t{x", ch, buf, NULL,TO_CHAR,POS_SLEEPING, channel );

    }
    else if (pmot)
    {
      if (IS_NPC(ch))
        mprintf( sizeof(buf), buf, "%s You emote : %s %s{x",
          type,
          ch->short_descr,
          argument );
      else
        mprintf( sizeof(buf), buf, "%s You emote : %s %s{x",
          type,
          ch->name,
          argument );

      if (IS_NPC(ch))
        mprintf( sizeof(buf1), buf1, "%s %s %s{x",
          type,
          ch->short_descr,
          argument );
      else
        mprintf( sizeof(buf1), buf1, "%s %s %s{x",
          type,
          ch->name,
          argument );

      if (IS_IMMORTAL(ch) && (!IS_SWITCHED(ch)))
        mprintf( sizeof(buf2), buf2, "%s An Unknown Entity %s{x",
          type,
          argument);
      else
        mprintf( sizeof(buf2), buf2, "%s Someone %s{x",
          type,
          argument);

      /*act_new( "$t{x", ch, buf, NULL,TO_CHAR,POS_SLEEPING );*/
      replace_color(buf,color_code);
      act_channels( "$t{x", ch, buf, NULL,TO_CHAR,POS_SLEEPING, channel );

    }
    else if (!display_wholist)
    {    
      mprintf( sizeof(buf), buf, "%s You say: '%s'{x",
        type,
        new_capitalize( arg_left ) );

      if (IS_NPC(ch))
        mprintf( sizeof(buf1), buf1, "%s %s: '%s'{x",
          type,
          ch->short_descr,
          new_capitalize( arg_left ) );
      else
        mprintf( sizeof(buf1), buf1, "%s %s: '%s'{x",
          type,
          ch->name,
          new_capitalize( arg_left ) );

      if (IS_IMMORTAL(ch) && !IS_SWITCHED(ch))
        mprintf( sizeof(buf2), buf2, "%s An Unknown Entity: '%s'{x",
          type,
          new_capitalize( arg_left ) );
      else
        mprintf( sizeof(buf2), buf2, "%s Someone: '%s'{x",
          type,
          new_capitalize( arg_left ) );

      /*act_new( "$t{x", ch, buf, NULL,TO_CHAR,POS_SLEEPING );*/
      replace_color(buf,color_code);
      act_channels( "$t{x", ch, buf, NULL,TO_CHAR,POS_SLEEPING, channel );
    }
    /* Die spammer, die! */

    if ( ch->desc && ch->desc->repeat > 1 )
      return;

      
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
      CHAR_DATA *vch;
      display_to_char = check_channel_view(ch,d,bitname, spec_flag);
      /*      vch = d->original ? d->original : d->character; */

      vch = d->character;
      if (vch == NULL)
        continue;

      // Taeloch -- quiet wasn't working right?
/*
      if ( !IS_SET(vch->chan_flags,(bitname))
      ||    IS_SET(vch->chan_flags,CHANNEL_QUIET) )
        continue;
*/

      strcpy(act1,buf1);
      strcpy(act2,buf2);

      if ( (channel == CHAN_GROUPTELL)
      &&   !is_same_group( ch, vch ) )
        display_to_char = FALSE;

      if ( display_to_char
      ||   ( (channel == CHAN_GROUPTELL) && is_same_group( ch, vch ) )
      ||  ( IS_IMMORTAL(ch) && ( channel == CHAN_PRAYER ) && ( vch == victim ) ) )
      {
        if (!display_wholist)
        {
// pmote, yo! -- replace name for "you" in act1, act2
          if ( ( pmot )
          &&   ( social == FALSE ) )
          {
            // buf1, replace name with "you"
            if ((letter = strstr(act1,vch->name)) != NULL)
            {
              strcpy(temp,act1);
              temp[strlen(act1) - strlen(letter)] = '\0';
              last[0] = '\0';
              name = vch->name;
    
              for (; *letter != '\0'; letter++)
              { 
                if (*letter == '\'' && matches == strlen(vch->name))
                {
                  strcat(temp,"r");
                  continue;
                }

                if (*letter == 's' && matches == strlen(vch->name))
                {
                  matches = 0;
                  continue;
                }
        
                if (matches == strlen(vch->name))
                  matches = 0;

                if (*letter == *name)
                {
                  matches++;
                  name++;
                  if (matches == strlen(vch->name))
                  {
                    strcat(temp,"you");
                    last[0] = '\0';
                    name = vch->name;
                    continue;
                  }
                  strncat(last,letter,1);
                  continue;
                }

                matches = 0;
                strcat(temp,last);
                strncat(temp,letter,1);
                last[0] = '\0';
                name = vch->name;
              }
              strcpy(act1,temp);
            }

            // repeat for act2
            if ((letter = strstr(act2,vch->name)) != NULL)
            {
              strcpy(temp,act2);
              temp[strlen(act2) - strlen(letter)] = '\0';
              last[0] = '\0';
              name = vch->name;
    
              for (; *letter != '\0'; letter++)
              { 
                if (*letter == '\'' && matches == strlen(vch->name))
                {
                  strcat(temp,"r");
                  continue;
                }

                if (*letter == 's' && matches == strlen(vch->name))
                {
                  matches = 0;
                  continue;
                }
        
                if (matches == strlen(vch->name))
                  matches = 0;

                if (*letter == *name)
                {
                  matches++;
                  name++;
                  if (matches == strlen(vch->name))
                  {
                    strcat(temp,"you");
                    last[0] = '\0';
                    name = vch->name;
                    continue;
                  }
                  strncat(last,letter,1);
                  continue;
                }

                matches = 0;
                strcat(temp,last);
                strncat(temp,letter,1);
                last[0] = '\0';
                name = vch->name;
              }
              strcpy(act2,temp);
            }
          }
// end pmote

// If they can see the char, channel is OOC, or in the same clan, but NOT if they are incog/wizi
          if ( ( ( can_see(vch,ch) )
          ||   ( channel == CHAN_OOC )
          ||   ( ( vch->clan == ch->clan )
          &&     ( vch->clan != NULL ) ) )
          && ( ch->incog_level <= vch->level )
          && ( ch->invis_level <= vch->level ))
          {
            replace_color(act1,color_code);
            act_channels( "$t{x", ch, act1, vch,TO_VICT,POS_SLEEPING, channel );
          }
          else
          {
            replace_color(act2,color_code);
            act_channels( "$t{x", ch, act2, vch,TO_VICT,POS_SLEEPING, channel );
          }          
        }
        else
          if (can_see(ch,vch))
            printf_to_char(ch,"{W%s{x\n\r", vch->name);
      }
    }
  }
}

void update_chan_hist( CHAR_DATA *ch, const int channel, char msg[MAX_INPUT_LENGTH+200])
{
  int hist_num;
  char timestamp[MSL];
  char buf[MSL+250];

  for (hist_num = MAX_HIST - 1; hist_num > 0; hist_num--)
  {
    free_string(ch->chan_hist[channel][hist_num].text);
    ch->chan_hist[channel][hist_num].text =
      str_dup(ch->chan_hist[channel][hist_num - 1].text, ch->chan_hist[channel][hist_num].text);
  }

  strftime( timestamp, MSL, "%X", localtime( &current_time ) );
  mprintf(sizeof(buf), buf, "%s %s{x", timestamp, msg );

  free_string(ch->chan_hist[channel][0].text);
  ch->chan_hist[channel][0].text = str_dup(buf, ch->chan_hist[channel][0].text);
}

void show_chan_hist( CHAR_DATA *ch, const int channel)
{
  int hist_num;
  printf_to_char(ch,"{WChannel History.{x\n\r");
  printf_to_char(ch,"{c================{x\n\r");
  for (hist_num = MAX_HIST - 1; hist_num >= 0; hist_num--) {
    if (ch->chan_hist[channel][hist_num].text[0] == '\0')
      continue;
    printf_to_char(ch, "%s", ch->chan_hist[channel][hist_num].text);
  }
  printf_to_char(ch,"{c================{x\n\r");
  return;
}


/* Channels */
void do_gossip( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument, "{W[{cGossip{W]{c", CHANNEL_NOGOSSIP, spec_public_flag, CHAN_GOSSIP, 'c');
}

void do_grats( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument, "{W[{YGrats{W]{Y", CHANNEL_NOGRATS, spec_public_flag, CHAN_GRATS, 'Y');
}

void do_quote( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument, "{W[{wQuote{W]{x", CHANNEL_NOQUOTE, spec_public_flag, CHAN_QUOTE,'w');
}

void do_question( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument, "{W[ {G? {W]{g", CHANNEL_NOQUESTION, spec_public_flag, CHAN_QUESTION, 'g');
}

void do_answer( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument, "{W[ {Y! {W]{y", CHANNEL_NOQUESTION, spec_public_flag, CHAN_QUESTION, 'W');
}

void do_music( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument, "{W[{mMusic{W]{m", CHANNEL_NOMUSIC, spec_public_flag, CHAN_MUSIC, 'B');
}

void do_ooc( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument, "{W[{BOOC{W]{w", CHANNEL_NOOOC, spec_public_flag, CHAN_OOC, 'w');
}

void do_info( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument, "{w[{rInfo{w]{x", CHANNEL_NOINFO, spec_public_flag, CHAN_ALERT, 'w');
}

void do_gtell( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument, "{W[{GGroup{W]{G", CHANNEL_NOTELL, spec_public_flag, CHAN_GROUPTELL, 'w');
}

void do_alert( CHAR_DATA *ch, char *argument )
{
  if (IS_SET(ch->chan_flags,CHANNEL_NOINFO))
  {
    printf_to_char(ch, "Alert channel is now ON.{x\n\r");
    REMOVE_BIT(ch->chan_flags,CHANNEL_NOINFO);
  }
  else
  {
    printf_to_char(ch, "Alert channel is now OFF.{x\n\r");
    SET_BIT(ch->chan_flags,CHANNEL_NOINFO);
  }
}

/* clan channels */
void do_wartalk( CHAR_DATA *ch, char *argument )
{
  if (!is_clan(ch)/* || clan_table[ch->clan].independent*/)
  {
    send_to_char("You aren't in a clan.\n\r",ch);
    return;
  }

  if ( is_clan(ch) && IS_SET( ch->clan->clan_flags, CLAN_PEACEFUL ) )
  {
    send_to_char( "Peaceful clans cannot use this channel.\n\r", ch );
    return;
  }

  if (IS_SET(ch->chan_flags,CHANNEL_NOWAR))
  {
    send_to_char("This channel is for Wartalking between clans.\n\r",ch);
    send_to_char("While vulgarity is not allowed, cursing for RP\n\r",ch);
    send_to_char("purposes is permitted.  If this kind of language\n\r", ch);
    send_to_char("does not appeal to you, turn the channel off.\n\r",ch);
  }
  public_ch( ch, argument, "{W[{RWar{DTalk{W]{R", CHANNEL_NOWAR, spec_allclan_flag, CHAN_WARTALK, 'R');
  return;
}

void do_politic( CHAR_DATA *ch, char *argument )
{
  if (!is_clan(ch)/* || clan_table[ch->clan].independent*/)
  {
    send_to_char("You aren't in a clan.\n\r",ch);
    return;
  }
  public_ch( ch, argument, "{W[{CPolitical{W]{W", CHANNEL_NOPOLITIC, spec_allclan_flag, CHAN_POLITICAL, 'W');
  return;
}

void do_clantalk ( CHAR_DATA *ch, char *argument)
{
  char buf[MSL];
  if (!IS_IN_CLAN(ch) /*|| clan_table[ch->clan].independent*/ )
  {
    send_to_char("You aren't in a clan.\n\r",ch);
    return;
  }

  mprintf(sizeof(buf), buf,"{w%s{C", ch->clan->symbol );
  replace_color(buf,'w');
  public_ch( ch, argument,buf, CHANNEL_NOCLAN, spec_clan_flag, CHAN_CLAN, 'C');
  /*  public_ch( ch, argument, "{W[{BClan{W]{c", COMM_NOCLAN, spec_clan_flag);*/
  return;
}

void do_immtalk( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument,"{c[{yImmtalk{c]{c", CHANNEL_NOWIZ, spec_imm_flag, CHAN_IMMTALK,'c');
}

void do_prayer( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument,"{c[{WPrayer{c]{W", CHANNEL_NOWIZ, spec_imm_flag, CHAN_PRAYER,'G');
}

void do_sextalk( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument,"{W[{mSex{bTalk{W]{W", CHANNEL_NOWIZ, spec_public_flag, CHAN_SEXTALK, 'W');
}

void do_sexpolice( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument,"{W[{MSex{BPolice{W]{B", CHANNEL_NOWIZ, spec_public_flag, CHAN_SEXPOLICE, 'B');
}

void do_gm( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument,"{W[{GGames{gMaster{W]{W", CHANNEL_NOWIZ, spec_public_flag, CHAN_GAMESMASTER, 'W');
}

void do_newbiehint( CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument,"{W[{gNewbie {cHint{W]{W", CHANNEL_NEWBIE, spec_public_flag, CHAN_HINT, 'W');
}

void do_irl (CHAR_DATA *ch, char *argument )
{
  public_ch( ch, argument,"{W[{RIRL{W]{w", CHANNEL_NOIRL, spec_public_flag, CHAN_IRL, 'w');
}

void do_say( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_INPUT_LENGTH + 200];
  CHAR_DATA *vch;

  if ( argument[0] == '\0' )
  {
    show_chan_hist(ch, CHAN_SAY);
    return;
  }

  check_4_talk_cheating(ch,argument);
  argument = makedrunk(argument,ch);
  replace_color(argument,'G');
  //argument = UPPER( argument[0] );
  act( "{gYou say '{w$T{g'{x", ch, NULL, new_capitalize( argument ), TO_CHAR );

  mprintf(sizeof(buf), buf, "{gYou said '{w%s{g'{x\n\r", new_capitalize( argument ) );

  update_chan_hist(ch, CHAN_SAY, buf);

  /*Die spammer, die*/
  if (ch->desc && ch->desc->repeat > 1)
    return;

  act( "{g$n says '{w$T{g'{x", ch, NULL, new_capitalize( argument ), TO_ROOM );
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
  {
    if (vch != ch)
    {
      mprintf(sizeof(buf), buf, "{g%s said '{w%s{g'{x\n\r",
        PERS(ch,vch), new_capitalize( argument ) );
      update_chan_hist(vch, CHAN_SAY, buf);
    }
  }
  return;
}

void do_immsay( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *rch, *rch_next;
  char buf[MAX_INPUT_LENGTH + 200];

  if ( argument[0] == '\0' )
  {
    show_chan_hist(ch, CHAN_IMMSAY);
    //send_to_char( "Immsay what?\n\r", ch );
    return;
  }

  check_4_talk_cheating(ch,argument);
  argument = makedrunk(argument,ch);
  replace_color(argument, 'M');
  act( "{DYou immsay {w'{g$T{w'{x", ch, NULL, new_capitalize( argument ), TO_CHAR );

  mprintf(sizeof(buf), buf, "{DYou immsaid '{g%s{m'{x\n\r",
    new_capitalize( argument ) );

  update_chan_hist(ch, CHAN_IMMSAY, buf);

  /*Die spammer, die*/
  if (ch->desc && ch->desc->repeat > 1)
    return;
  for ( rch = ch->in_room->people; rch; rch = rch_next )
  {
    rch_next = rch->next_in_room;

    if ( rch == ch || !IS_IMMORTAL( rch ))
      continue;

    act( "{D$n immsays {w'{g$t{w'{x", ch, new_capitalize( argument ), rch, TO_VICT );

    mprintf(sizeof(buf), buf, "{D%s immsaid {w'{g%s{w'{x\n\r",
      PERS(ch,rch), new_capitalize( argument ) );

    update_chan_hist(rch, CHAN_IMMSAY, buf);
  }
  return;
}


void do_shout( CHAR_DATA *ch, char *argument )
{
  check_4_talk_cheating(ch,argument);
  public_ch( ch, argument,"{c[{MShouts{c]: {M", CHANNEL_SHOUTSOFF, spec_public_flag, CHAN_SHOUT, 'M');
  WAIT_STATE( ch, 12 );
}

void do_tell( CHAR_DATA *ch, char *argument)
{
  int pos;
  char arg[MIL], buf[MSL];
  CHAR_DATA *victim;

  if (IS_LINKDEAD(ch))
  {
    send_to_char("Holy toledo, you are LINKDEAD...Not anymore.\n\r",ch);
    REMOVE_BIT(ch->affected_by, AFF_LINKDEATH);
  }

  if ( IS_SET(ch->chan_flags, CHANNEL_NOTELL)
  || IS_SET(ch->chan_flags,CHANNEL_DEAF))
  {
    send_to_char( "Your message didn't get through.\n\r", ch );
    return;
  }

  if ( IS_SET(ch->chan_flags, CHANNEL_QUIET) )
  {
    send_to_char( "You must turn off quiet mode first.\n\r", ch);
    return;
  }

  if (IS_SET(ch->chan_flags,CHANNEL_DEAF))
  {
    send_to_char("You must turn off deaf mode first.\n\r",ch);
    return;
  }

  argument = one_argument( argument, arg );

  argument = makedrunk(argument,ch);
  if ( arg[0] == '\0' || argument[0] == '\0' )
  {
    show_chan_hist(ch, CHAN_TELL);
    //send_to_char( "Tell whom what?\n\r", ch );
    return;
  }

  /*
   * Can tell to PC's anywhere, but NPC's only in same room.
   * -- Furey
   */
  if ( ( victim = get_char_world( ch, arg ) ) == NULL
  || ( IS_NPC(victim)
  && ( victim->in_room != ch->in_room ) ) )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  
  if (!IS_NPC(victim))
  {
    for (pos = 0; pos < MAX_FORGET; pos++)
    {
      if (victim->pcdata->forget[pos] == NULL)
        continue;
      if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
      {
        send_to_char("That person has forgotten you.\n\r",ch);
        return; 
      }
    }
  }

  if ( victim->desc == NULL && !IS_NPC(victim))
  {
    check_4_talk_cheating(ch,buf);
    act("$N seems to have misplaced $S link...try again later.", ch,NULL,victim,TO_CHAR);
    replace_color(argument, 'Y');

    /*Die spammer, die*/
    if (ch->desc && ch->desc->repeat > 1)
      return;

    mprintf(sizeof(buf), buf, "{w%s tells you{Y '%s'{x\n\r",
      PERS(ch,victim), new_capitalize( argument ) );

    buf[0] = UPPER(buf[0]);
    if (add_buf(victim->pcdata->buffer,buf) == FALSE)
    {
      send_to_char("Tell was not saved to character.\n\r",ch);
      mprintf(sizeof(buf), buf, "%s's tell to %s was not saved.  %s was linkdead.\n\r",
        ch->name, victim->name, victim->name);
      return;
    }

    update_chan_hist(victim, CHAN_TELL, buf);
    victim->tells++;

    mprintf(sizeof(buf), buf, "{gYou told %s '{W%s{g'{x\n\r",
      PERS(victim, ch),
      new_capitalize( argument ) );
    update_chan_hist(ch, CHAN_TELL, buf);

    if ( !IS_SET( victim->comm_flags, COMM_REPLY_LOCK )
    && !IS_NPC(victim) && !IS_NPC(ch))
      victim->reply = ch;

    return;
  }

/* Taeloch -- Aarchane wanted tells to work while asleep
   Taeloch -- Then Draxal decided it should be optional.
*/

  if (IS_SET(ch->comm_flags, COMM_NO_SLEEP_TELLS)
  && !IS_AWAKE(ch) )
  {
    send_to_char("You have sleep tells turned off!\n\r",ch);
    return;
  }

  if (!(IS_IMMORTAL(ch) && (ch->level > LEVEL_IMMORTAL))
  &&    IS_SET(victim->comm_flags, COMM_NO_SLEEP_TELLS)
  &&   !IS_AWAKE(victim) )
  {
    send_to_char( "They can't hear you.\n\r", ch );
    return;
  }

  if ((IS_SET(victim->chan_flags,CHANNEL_QUIET)
  || IS_SET(victim->chan_flags,CHANNEL_DEAF))
  && !IS_IMMORTAL(ch))
  {
    act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
    return;
  }

  if (IS_AFK(victim))
  {
    if (IS_NPC(victim))
    {
      act("$E is AFK, and not receiving tells.",ch,NULL,victim,TO_CHAR);
      return;
    }

    act("$E is AFK, but your tell will go through when $E returns.",
        ch,NULL,victim,TO_CHAR);
    replace_color(argument, 'Y');

    /*Die spammer, die*/
    if (ch->desc && ch->desc->repeat > 1)
      return;

    mprintf(sizeof(buf), buf, "{g%s told you '{W%s{g'{x\n\r",
      PERS(ch,victim), new_capitalize( argument ) );

    check_4_talk_cheating(ch,buf);
    send_to_char("\n\r",victim);
    if (add_buf(victim->pcdata->buffer,buf) == FALSE)
    {
      send_to_char("Tell was not saved to character.\n\r",ch);
      mprintf(sizeof(buf), buf, "%s's tell to %s was not saved.  %s was AFK.\n\r",
        ch->name, victim->name, victim->name);
      return;
    }

    update_chan_hist(victim, CHAN_TELL, buf);
    victim->tells +=1;

    mprintf(sizeof(buf), buf, "{gYou told %s '{W%s{g'{x\n\r",
      PERS(victim,ch), new_capitalize( argument ) );

    update_chan_hist(ch, CHAN_TELL, buf);

    if ( !IS_SET( victim->comm_flags, COMM_REPLY_LOCK )
    &&   !IS_NPC(victim) && !IS_NPC(ch))
      victim->reply = ch;

    return;
  }

  check_4_talk_cheating(ch,argument);
  replace_color(argument, 'Y');

/* tells work while asleep now
  act( " {gYou tell $N '{W$t{g'{x", ch, new_capitalize( argument ), victim, TO_CHAR );
*/
  act_new(" {gYou tell $N '{W$t{g'{x", ch, new_capitalize( argument ), victim, TO_CHAR, POS_DEAD);

  mprintf(sizeof(buf), buf, "{gYou told %s '{W%s{g'{x\n\r",
    PERS(victim,ch), new_capitalize( argument ) );

  update_chan_hist(ch, CHAN_TELL, buf);

  /*Die spammer, die*/
  if (ch->desc && ( ch->desc->repeat > 1) )
    return;

  act_new(" {g$n tells you '{W$t{g'{x", ch, new_capitalize( argument ), victim, TO_VICT, POS_DEAD);

  mprintf(sizeof(buf), buf, "{g%s told you '{W%s{g'{x\n\r",
    PERS(ch,victim), new_capitalize( argument ) );

  update_chan_hist(victim, CHAN_TELL, buf);

  if ( !IS_SET( victim->comm_flags, COMM_REPLY_LOCK )
  && !IS_NPC(victim)
  && !IS_NPC(ch))
    victim->reply = ch;

  if ( !IS_NPC(ch)
  && IS_NPC(victim)
  && HAS_TRIGGER(victim,TRIG_SPEECH) )
    mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );

  return;
}

void do_whisper( CHAR_DATA *ch, char *argument )
{
  int       pos;
  char      arg[MIL];
  char      buf[MSL];
  char      buf1[MSL];
  CHAR_DATA *victim;
  CHAR_DATA *vch;

  //argument = makedrunk(argument,ch);
  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' || argument[0] == '\0' )
  {
    send_to_char( "whisper to whom what?\n\r", ch);
    return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
  //|| ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if (!IS_NPC(victim))
  {
    for (pos = 0; pos < MAX_FORGET; pos++)
    {
      if (victim->pcdata->forget[pos] == NULL)
        continue;

      if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
      {
        send_to_char("That person has forgotten you.\n\r",ch);
        return; 
      }
    }
  }

  replace_color(argument, 'c');
  if ( victim->desc == NULL && !IS_NPC(victim))
  {
    check_4_talk_cheating(ch,buf);
    act("$N seems to have misplaced $S link...try again later.",
    ch,NULL,victim,TO_CHAR);
    /*Die spammer, die*/

    if (ch->desc && ch->desc->repeat > 1)
      return;

    mprintf(sizeof(buf), buf,"{c%s whispers '{D%s{c'{x\n\r",
      PERS(ch,victim),
      argument);

    buf[2] = UPPER(buf[2]);

    if (add_buf(victim->pcdata->buffer,buf) == FALSE)
    {
      send_to_char("Whisper was not saved to character\n\r.", ch);
      return;
    }

    victim->tells++;
    return;
  }

  if ( !IS_IMMORTAL( ch )
  && !IS_AWAKE( victim ) )
  {
    act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
    return;
  }
  
  if (IS_AFK(victim))
  {
    if (IS_NPC(victim))
    {
      act("$E is AFK, and cannot see your whisper.",ch,NULL,victim,TO_CHAR);
      return;
    }
    act("$E is AFK, but your tell will go through when $E returns.",
    ch,NULL,victim,TO_CHAR);

    /*Die spammer, die*/
    if (ch->desc && ch->desc->repeat > 1)
      return;

    mprintf(sizeof(buf), buf, "{c%s {Gwhispers '{D%s{c'{x\n\r",
      PERS(ch,victim),
      argument);
    buf[2] = UPPER(buf[2]);
    check_4_talk_cheating(ch,buf);

    if (add_buf(victim->pcdata->buffer,buf) == FALSE)
    {
      send_to_char("Whisper not saved to char.\n\r", ch);
      return;
    }

    victim->tells++;
    return;
  }

  check_4_talk_cheating(ch,argument);
  mprintf( sizeof( buf1 ), buf1, "{cYou whisper into %s's ear '{D%s{c'{x",
    PERS( victim, ch ),
    argument );
  act( buf1, ch, NULL, victim, TO_CHAR );

  /*Die spammer, die*/
  if (ch->desc && ch->desc->repeat > 1)
    return;

  mprintf(sizeof(buf), buf, "{c%s whispers into your ear '{D%s{c'{x",
    PERS(ch,victim),
    argument);

  buf[2] = UPPER(buf[2]);
  act(buf,ch,NULL,victim,TO_VICT);

  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
  {
    if ( IS_NPC( vch ) )
      continue;

    if ( vch != ch
    &&   vch != victim )
    {
      if ( IS_IMMORTAL( vch ) )
      {
        if (vch->level >= ch->level && vch->level >= victim->level)
          printf_to_char(vch, "{c%s whispers into %s's ear '{D%s{c'{x\n\r",
            PERS(ch, vch),
            PERS(victim, vch), argument);
        else
          printf_to_char( vch, "%s whispers something into %s's ear.\n\r",
            PERS( ch, vch ),
            PERS( victim, vch ) );
      }
      else if ( !IS_NPC( ch ) )
        printf_to_char( vch, "%s whispers something into %s's ear.\n\r",
          PERS( ch, vch ),
          PERS( victim, vch ) );
    }
  }

  if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
    mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );

  return;
}

void do_reply( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char buf[MSL];
  int pos=0;

  if ( argument[0] == '\0' )
  {
    show_chan_hist(ch, CHAN_TELL);
    return;
  }

  if ( IS_SET(ch->chan_flags, CHANNEL_NOTELL) )
  {
    send_to_char( "Your message didn't get through.\n\r", ch );
    return;
  }

  if ( ( victim = ch->reply ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }
 
  if (!IS_NPC(victim))
  {
    for (pos = 0; pos < MAX_FORGET; pos++)
    {
      if (victim->pcdata->forget[pos] == NULL)
        continue;

      if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
      {
        send_to_char("That person has forgotten you.\n\r",ch);
        return; 
      }
    }
  }

  if (!(IS_IMMORTAL(ch) && (ch->level > LEVEL_IMMORTAL))
  &&    IS_SET(victim->comm_flags, COMM_NO_SLEEP_TELLS)
  &&   !IS_AWAKE(victim) )
  {
    send_to_char( "They can't hear you.\n\r", ch );
    return;
  }

  replace_color(argument, 'Y');

  if ( victim->desc == NULL && !IS_NPC(victim))
  {
    act("$N seems to have misplaced $S link...try again later.",
      ch,NULL,victim,TO_CHAR);

    /*Die spammer, die*/
    if (ch->desc && ch->desc->repeat < 1)
      return;

    mprintf(sizeof(buf), buf, "{g %s tells you '{W%s{g'{x\n\r",
      PERS(ch,victim),
      new_capitalize( argument ) );

    buf[0] = UPPER(buf[0]);

    if (add_buf(victim->pcdata->buffer,buf) == FALSE)
    {
      send_to_char("Reply was not saved to character.\n\r", ch);
      return;
    }

    victim->tells++;
    return;
  }

/* tells work asleep
  if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
  {
    act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
    return;
  }
*/

  if ((IS_SET(victim->chan_flags,CHANNEL_QUIET)
  || IS_SET(victim->chan_flags,CHANNEL_DEAF))
  &&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim))
  {
    act_new( "$E is not receiving tells.", ch, 0, victim, TO_CHAR,POS_DEAD);
    return;
  }

/* tells work asleep
  if (!IS_IMMORTAL(victim) && !IS_AWAKE(ch))
  {
    send_to_char( "In your dreams, or what?\n\r", ch );
    return;
  }
*/

  if (IS_AFK(victim))
  {
    if (IS_NPC(victim))
    {
      act_new("$E is AFK, and not receiving tells.",
        ch,NULL,victim,TO_CHAR,POS_DEAD);
      return;
    }
 
    act_new("$E is AFK, but your tell will go through when $E returns.",
      ch,NULL,victim,TO_CHAR,POS_DEAD);

    /*Die spammer, die*/
    if (ch->desc && ch->desc->repeat > 1)
      return;

    mprintf(sizeof(buf), buf, "{g%s told you '{W%s{g'{x\n\r",
      PERS(ch,victim), new_capitalize( argument ) );

    send_to_char( "\n\r", victim );
    buf[0] = UPPER(buf[0]);
    check_4_talk_cheating(ch,buf);

    if (add_buf(victim->pcdata->buffer,buf) == FALSE)
    {
      send_to_char("Reply was not saved to character.\n\r", ch);
      return;
    }

    mprintf(sizeof(buf), buf, "{g%s told you '{W%s{g'{x\n\r",
      PERS(ch,victim), new_capitalize( argument ) );

    update_chan_hist(victim, CHAN_TELL, buf);
    victim->tells +=1;

    mprintf(sizeof(buf), buf, "{gYou told %s '{W%s{g'{x\n\r",
      PERS(victim, ch), new_capitalize( argument ) );

    update_chan_hist(ch, CHAN_TELL, buf);
    return;
  }

  argument = makedrunk(argument,ch);

  act_new(" {gYou tell $N '{W$t{g'{x",ch, new_capitalize( argument ),victim,TO_CHAR,POS_DEAD);

  /*Die spammer, die*/
  if (ch->desc && ch->desc->repeat > 1)
    return;

  act_new(" {g$n tells you '{W$t{g'{x",ch, new_capitalize( argument ),victim,TO_VICT,POS_DEAD);

  mprintf(sizeof(buf), buf, "{gYou told %s '{W%s{g'{x\n\r",
    PERS(victim, ch), new_capitalize( argument ) );

  //buf[0] = UPPER(buf[0]);
  check_4_talk_cheating(ch,buf);
  update_chan_hist(ch, CHAN_TELL, buf);

  mprintf(sizeof(buf), buf, "{g%s told you '{W%s{g'{x\n\r",
    PERS(ch,victim), new_capitalize( argument ) );

  update_chan_hist(victim, CHAN_TELL, buf);

  if ( !IS_SET( victim->comm_flags, COMM_REPLY_LOCK ) && (!IS_NPC(ch)))
    victim->reply = ch;

  return;
}

void do_yell( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA *d;
  char buf[MSL];

  if ( IS_SET(ch->chan_flags, CHANNEL_NOSHOUT) )
  {
    send_to_char( "You can't yell.\n\r", ch );
    return;
  }

  if ( !IS_NPC(ch) && IS_NOCHANNELED(ch))
  {
    send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
    return;
  }

  if ( argument[0] == '\0' )
  {
    show_chan_hist (ch, CHAN_YELL);
    return;
  }

  argument = makedrunk(argument,ch);
  replace_color(argument, 'R');
  act("{WYou yell {R'$t{R'{x",ch,new_capitalize(argument),NULL,TO_CHAR);

  mprintf(sizeof(buf), buf, "{WYou yell {R'%s{R'{x\n\r", new_capitalize(argument));
  update_chan_hist(ch, CHAN_YELL, buf);

// chan_hist

  /*Die spammer, die*/
  if (ch->desc && ch->desc->repeat > 1)
    return;

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    if ( d->connected == CON_PLAYING
    &&   d->character != ch
    &&   d->character->in_room != NULL
    &&   d->character->in_room->area == ch->in_room->area 
    &&   !IS_SET(d->character->chan_flags,CHANNEL_QUIET) )
    {
      act("{W$n yells {R'$t{R'{x",ch,new_capitalize(argument),d->character,TO_VICT);
      mprintf(sizeof(buf), buf, "{W%s yells {R'%s{R'{x\n\r",
        IS_NPC(ch) ? capitalize(ch->short_descr) : ch->name, new_capitalize(argument));
      update_chan_hist(d->character, CHAN_YELL, buf);
    }
  }

  return;
}

void do_emote( CHAR_DATA *ch, char *argument )
{
  char buf[MSL], buf1[MSL];
  char *NOSPACECHAR = ",'";

  if ( !IS_NPC(ch) && IS_SET(ch->chan_flags, CHANNEL_NOEMOTE) )
  {
    send_to_char( "You can't show your emotions.\n\r", ch );
    return;
  }
 
  if ( argument[0] == '\0' )
  {
    send_to_char( "Emote what?\n\r", ch );
    return;
  }

  buf[0] = '\0';

  if ( !strchr( NOSPACECHAR, *argument ) )
    strcat( buf, " " );

  strcat( buf, argument );
  replace_color( argument, 'w');

  if (!IS_NPC(ch))
    sprintf( buf1, "%s%s", ch->name, buf );
  else
    sprintf( buf1, "%s%s", ch->short_descr, buf );

  act( buf1, ch, NULL, argument, TO_CHAR );

  /*Die spammer, die*/
  if (ch->desc && ch->desc->repeat > 1)
    return;

  MOBtrigger = FALSE;
//  act( "$n $T{x", ch, NULL, argument, TO_ROOM );
  act( buf1, ch, NULL, argument, TO_ROOM );
  MOBtrigger = TRUE;
  return;
}

void do_pmote( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *vch;
  char *letter,*name;
  char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
  int matches = 0;

  if ( !IS_NPC(ch) && IS_SET(ch->chan_flags, CHANNEL_NOEMOTE) )
  {
    send_to_char( "You can't show your emotions.\n\r", ch );
    return;
  }
 
  if ( argument[0] == '\0' )
  {
    send_to_char( "Emote what?\n\r", ch );
    return;
  }

  act( "$n $t", ch, argument, NULL, TO_CHAR );
  /*Die spammer, die*/
  if (ch->desc && ch->desc->repeat > 1)
    return;

  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
  {
    if (vch->desc == NULL || vch == ch)
      continue;

    if ((letter = strstr(argument,vch->name)) == NULL)
    {
      MOBtrigger = FALSE;
      act("$N $t",vch,argument,ch,TO_CHAR);
      MOBtrigger = TRUE;
      continue;
    }

    strcpy(temp,argument);
    temp[strlen(argument) - strlen(letter)] = '\0';
    last[0] = '\0';
    name = vch->name;
    
    for (; *letter != '\0'; letter++)
    { 
      if (*letter == '\'' && matches == strlen(vch->name))
      {
        strcat(temp,"r");
        continue;
      }

      if (*letter == 's' && matches == strlen(vch->name))
      {
        matches = 0;
        continue;
      }
        
      if (matches == strlen(vch->name))
        matches = 0;

      if (*letter == *name)
      {
        matches++;
        name++;
        if (matches == strlen(vch->name))
        {
          strcat(temp,"you");
          last[0] = '\0';
          name = vch->name;
          continue;
        }
        strncat(last,letter,1);
        continue;
      }

      matches = 0;
      strcat(temp,last);
      strncat(temp,letter,1);
      last[0] = '\0';
      name = vch->name;
    }

    MOBtrigger = FALSE;
    act("$N $t",vch,temp,ch,TO_CHAR);
    MOBtrigger = TRUE;
  }

  return;
}


#define POSES 10

const char * pose_table[POSES][2*MAX_CLASS] =
{
  {
    "You sizzle with energy.",            // Cjr self
    "$n sizzles with energy.",            // Cjr room
    "You feel very holy.",                // Pri self
    "$n looks very holy.",                // Pri room
    "You perform a small card trick.",    // Hwy self
    "$n performs a small card trick.",    // Hwy room
    "You show off your bulging muscles.", // Kni self
    "$n shows off $s bulging muscles.",   // Kni room
    "You sizzle with energy.",            // Wlk self
    "$n sizzles with energy.",            // Wlk room
    "You show off your bulging muscles.", // Bar self
    "$n shows off $s bulging muscles.",   // Bar room
    "You strike a judo stance.",          // Mys self
    "$n strikes a judo stance.",          // Mys room
    "You summon a swarm of butterflies.", // Dru self
    "$n summons a swarm of butterflies.", // Dru room
    "You feel very holy.",                // Inq self
    "$n looks very holy.",                // Inq room
    "You glow with an unholy light.",     // Occ self
    "$n glows with an unholy light.",     // Occ room
    "A lightbulb appears over your head.",// Alc self
    "A lightbulb appears over $n's head.",// Alc room
    "You call for cute woodland critter.",// Wds self
    "$n calls and a cute bunny rabbit appears.", // Wds room
    "You strike a boring pose!",          // un5 self
    "$n strikes a boring pose!",          // un5 room
    "You strike a boring pose!",          // un6 self
    "$n strikes a boring pose!"          // un6 room
  },

  {
    "You turn into a butterfly, then return to your normal shape.", // Cjr self
    "$n turns into a butterfly, then returns to $s normal shape.", // Cjr room
    "You nonchalantly turn wine into water.", // Pri self
    "$n nonchalantly turns wine into water.", // Pri room
    "You wiggle your ears alternately.", // Hwy self
    "$n wiggles $s ears alternately.", // Hwy room
    "You crack nuts between your fingers.", // Kni self
    "$n cracks nuts between $s fingers.", // Kni room
    "You turn into a butterfly, then return to your normal shape.", // Wlk self
    "$n turns into a butterfly, then returns to $s normal shape.", // Wlk room
    "You crack nuts between your fingers.", // Bar self
    "$n cracks nuts between $s fingers.", // Bar room
    "You karate chop at the air.", // Mys self
    "$n karate chops at the air.", // Mys room
    "The sun pierces through the clouds to illuminate you.", // Dru self
    "The sun pierces through the clouds to illuminate $n.", // Dru room
    "You nonchalantly turn wine into water.", // Inq self
    "$n nonchalantly turns wine into water.", // Inq room
    "The stones dance to your command.", // Occ self
    "The stones dance to $n's command.", // Occ room
    "You up in a puff of smoke due to an adverse chemical reaction.", // Alc self
    "$n goes up in a puff of smoke due to an adverse chemical reaction.", // Alc room
    "w1s", // Wds self
    "w1o", // Wds room
    "You strike a boring pose!",          // un5 self
    "$n strikes a boring pose!",          // un5 room
    "You strike a boring pose!",          // un6 self
    "$n strikes a boring pose!"          // un6 room
  },

  {
    "Blue sparks fly from your fingers.", // Cjr self
    "Blue sparks fly from $n's fingers.", // Cjr room
    "A halo appears over your head.", // Pri self
    "A halo appears over $n's head.", // Pri room
    "You nimbly tie yourself into a knot.", // Hwy self
    "$n nimbly ties $mself into a knot.", // Hwy room
    "You grizzle your teeth and look mean.", // Kni self
    "$n grizzles $s teeth and looks mean.", // Kni room
    "Blue sparks fly from your fingers.", // Wlk self
    "Blue sparks fly from $n's fingers.", // Wlk room
    "You grizzle your teeth and look mean.", // Bar self
    "$n grizzles $s teeth and looks mean.", // Bar room
    "You try to intimidate someone with your snake style.", // Mys self
    "$n tries to intimidate you with $s snake style.", // Mys room
    "The ocean parts before you.", // Dru self
    "The ocean parts before $n.", // Dru room
    "A halo appears over your head.", // Inq self
    "A halo appears over $n's head.", // Inq room
    "Smoke and fumes leak from your nostrils.", // Occ self
    "Smoke and fumes leak from $n's nostrils.", // Occ room
    "A failed potion blows up in your face.", // Alc self
    "A failed potion blows up in $n's face.", // Alc room
    "w2s", // Wds self
    "w2o", // Wds room
    "You strike a boring pose!",          // un5 self
    "$n strikes a boring pose!",          // un5 room
    "You strike a boring pose!",          // un6 self
    "$n strikes a boring pose!"          // un6 room
  },

  {
    "Little red lights dance in your eyes.", // Cjr self
    "Little red lights dance in $n's eyes.", // Cjr room
    "You recite words of wisdom.", // Pri self
    "$n recites words of wisdom.", // Pri room
    "You juggle with daggers, apples, and eyeballs.", // Hwy self
    "$n juggles with daggers, apples, and eyeballs.", // Hwy room
    "You hit your head, and your eyes roll.", // Kni self
    "$n hits $s head, and $s eyes roll.", // Kni room
    "Little red lights dance in your eyes.", // Wlk self
    "Little red lights dance in $n's eyes.", // Wlk room
    "You hit your head, and your eyes roll.", // Bar self
    "$n hits $s head, and $s eyes roll.", // Bar room
    "You flash brightly with an energy flare.", // Mys self
    "$n flashes brightly with an energy flare.", // Mys room
    "A thunder cloud kneels to you.", // Dru self
    "A thunder cloud kneels to $n.", // Dru room
    "You recite words of wisdom.", // Inq self
    "$n recites words of wisdom.", // Inq room
    "Little red lights dance in your eyes.", // Occ self
    "Little red lights dance in $n's eyes.", // Occ room
    "Everyone's pocket explodes with your fireworks.", // Alc self
    "Your pocket explodes with $n's fireworks.", // Alc room
    "w3s", // Wds self
    "w3o", // Wds room
    "You strike a boring pose!",          // un5 self
    "$n strikes a boring pose!",          // un5 room
    "You strike a boring pose!",          // un6 self
    "$n strikes a boring pose!"          // un6 room
  },

  {
    "A slimy green monster appears before you and bows.", // Cjr self
    "A slimy green monster appears before $n and bows.", // Cjr room
    "Deep in prayer, you levitate.", // Pri self
    "Deep in prayer, $n levitates.", // Pri room
    "You steal the underwear from every person in the room.", // Hwy self
    "Your underwear is gone: $n stole it!", // Hwy room
    "Crunch, crunch -- you munch a bottle.", // Kni self
    "Crunch, crunch -- $n munches a bottle.", // Kni room
    "A slimy green monster appears before you and bows.", // Wlk self
    "A slimy green monster appears before $n and bows.", // Wlk room
    "Crunch, crunch -- you munch a bottle.", // Bar self
    "Crunch, crunch -- $n munches a bottle.", // Bar room
    "You try to show off your style, but lose your balance.", // Mys self
    "$n almost falls over trying to strike a pose.", // Mys room
    "The sky changes color to match your eyes.", // Dru self
    "The sky changes color to match $n's eyes.", // Dru room
    "Deep in prayer, you levitate.", // Inq self
    "Deep in prayer, $n levitates.", // Inq room
    "A skeleton appears before you and bows.", // Occ self
    "A skeleton appears before $n and bows.", // Occ room
    "A smoke bomb explodes, singing your hair.", // Alc self
    "A smoke bomb explodes, singing $n's hair.", // Alc room
    "w4s", // Wds self
    "w4o", // Wds room
    "You strike a boring pose!",          // un5 self
    "$n strikes a boring pose!",          // un5 room
    "You strike a boring pose!",          // un6 self
    "$n strikes a boring pose!"          // un6 room
  },

  {
    "You turn everybody into a little pink elephant.", // Cjr self
    "You are turned into a little pink elephant by $n.", // Cjr room
    "An angel consults you.", // Pri self
    "An angel consults $n.", // Pri room
    "The dice roll ... and you win again.", // Hwy self
    "The dice roll ... and $n wins again.", // Hwy room
    "... 98, 99, 100 ... you do pushups.", // Kni self
    "... 98, 99, 100 ... $n does pushups.", // Kni room
    "You turn everybody into a little pink elephant.", // Wlk self
    "You are turned into a little pink elephant by $n.", // Wlk room
    "You start doing pushups.", // Bar self
    "$n does pushups to impress you.", // Bar room
    "Deep in prayer, you levitate.", // Mys self
    "Deep in prayer, $n levitates.", // Mys room
    "The heavens and grass change colour as you smile.", // Dru self
    "The heavens and grass change colour as $n smiles.", // Dru room
    "An angel consults you.", // Inq self
    "An angel consults $n.", // Inq room
    "A demon consults you.", // Occ self
    "A demon consults $n.", // Occ room
    "A scroll ignites, singing your hair.", // Alc self
    "A scroll ignites, singing $n's hair.", // Alc room
    "w5s", // Wds self
    "w5o", // Wds room
    "You strike a boring pose!",          // un5 self
    "$n strikes a boring pose!",          // un5 room
    "You strike a boring pose!",          // un6 self
    "$n strikes a boring pose!"          // un6 room
  },

  {
    "A small ball of light dances on your fingertips.", // Cjr self
    "A small ball of light dances on $n's fingertips.", // Cjr room
    "Your body glows with an unearthly light.", // Pri self
    "$n's body glows with an unearthly light.", // Pri room
    "You count the money in everyone's pockets.", // Hwy self
    "Check your money, $n is counting it.", // Hwy room
    "Bodybuilders admire your physique.", // Kni self
    "Bodybuilders admire $n's physique.", // Kni room
    "A small ball of light dances on your fingertips.", // Wlk self
    "A small ball of light dances on $n's fingertips.", // Wlk room
    "Bodybuilders admire your physique.", // Bar self
    "Bodybuilders admire $n's physique.", // Bar room
    "Your karate chop splits a tree.", // Mys self
    "$n's karate chop splits a tree", // Mys room
    "You turn into a butterfly, then return to your normal shape.", // Dru self
    "$n turns into a butterfly, then returns to $s normal shape.", // Dru room
    "Your body glows with a holy light.", // Inq self
    "$n's body glows with a holy light.", // Inq room
    "Your body glows with an unearthly light.", // Occ self
    "$n's body glows with an unearthly light.", // Occ room
    "A small ball of light dances on your fingertips.", // Alc self
    "A small ball of light dances on $n's fingertips.", // Alc room
    "w6s", // Wds self
    "w6o", // Wds room
    "You strike a boring pose!",          // un5 self
    "$n strikes a boring pose!",          // un5 room
    "You strike a boring pose!",          // un6 self
    "$n strikes a boring pose!"          // un6 room
  },

  {
    "The light flickers as you rap in magical languages.", // Cjr self
    "The light flickers as $n raps in magical languages.", // Cjr room
    "Everyone levitates as you pray.", // Pri self
    "You levitate as $n prays.", // Pri room
    "You produce a coin from everyone's ear.", // Hwy self
    "$n produces a coin from your ear.", // Hwy room
    "Oomph!  You squeeze water out of a granite boulder.", // Kni self
    "Oomph!  $n squeezes water out of a granite boulder.", // Kni room
    "The light flickers as you rap in magical languages.", // Wlk self
    "The light flickers as $n raps in magical languages.", // Wlk room
    "Oomph!  You squeeze water out of a granite boulder.", // Bar self
    "Oomph!  $n squeezes water out of a granite boulder.", // Bar room
    "Oomph!  You squeeze water out of a granite boulder.", // Mys self
    "Oomph!  $n squeezes water out of a granite boulder.", // Mys room
    "A cool breeze refreshes you.", // Dru self
    "A cool breeze refreshes $n.", // Dru room
    "Everyone levitates as you pray.", // Inq self
    "You levitate as $n prays.", // Inq room
    "You sizzle with energy.", // Occ self
    "$n sizzles with energy.", // Occ room
    "You construct a strange mechanical device from pieces of wood.", // Alc self
    "$n constructs a strange mechanical device from pieces of wood.", // Alc room
    "w7s", // Wds self
    "w7o", // Wds room
    "You strike a boring pose!",          // un5 self
    "$n strikes a boring pose!",          // un5 room
    "You strike a boring pose!",          // un6 self
    "$n strikes a boring pose!"          // un6 room
  },

  {
    "Your head disappears.", // Cjr self
    "$n's head disappears.", // Cjr room
    "A cool breeze refreshes you.", // Pri self
    "A cool breeze refreshes $n.", // Pri room
    "You step behind your shadow.", // Hwy self
    "$n steps behind $s shadow.", // Hwy room
    "You pick your teeth with a spear.", // Kni self
    "$n picks $s teeth with a spear.", // Kni room
    "Your head disappears.", // Wlk self
    "$n's head disappears.", // Wlk room
    "You pick your teeth with a spear.", // Bar self
    "$n picks $s teeth with a spear.", // Bar room
    "Everyone's pocket explodes with your fireworks.", // Mys self
    "Your pocket explodes with $n's fireworks.", // Mys room
    "The sun pierces through the clouds to illuminate you.", // Dru self
    "The sun pierces through the clouds to illuminate $n.", // Dru room
    "A cool breeze refreshes you.", // Inq self
    "A cool breeze refreshes $n.", // Inq room
    "Blue sparks fly from your fingers.", // Occ self
    "Blue sparks fly from $n's fingers.", // Occ room
    "You bend the light around your head, causing it to disappear.", // Alc self
    "$n bends the light around your head, causing it to disappear.", // Alc room
    "w8s", // Wds self
    "w8o", // Wds room
    "You strike a boring pose!",          // un5 self
    "$n strikes a boring pose!",          // un5 room
    "You strike a boring pose!",          // un6 self
    "$n strikes a boring pose!"          // un6 room
  },

  {
    "A fire elemental singes your hair.", // Cjr self 0
    "A fire elemental singes $n's hair.", // Cjr room 1
    "The sun pierces through the clouds to illuminate you.", // Pri self 2
    "The sun pierces through the clouds to illuminate $n.", // Pri room 3
    "Your eyes dance with greed.", // Hwy self 4
    "$n's eyes dance with greed.", // Hwy room 5
    "Everyone is swept off their feet by your hug.", // Kni self 6
    "You are swept off your feet by $n's hug.", // Kni room 7
    "A fire elemental singes your hair.", // Wlk self 8
    "A fire elemental singes $n's hair.", // Wlk room 9
    "A boulder cracks at your frown.", // Bar self 10
    "A boulder cracks at $n's frown.", // Bar room 11
    "Your hands move too fast to see.", // Mys self 12
    "$n's hands move too fast to see.", // Mys room 13
    "A fire elemental singes your hair.", // Dru self 14
    "A fire elemental singes $n's hair.", // Dru room 15
    "The sun pierces through the clouds to illuminate you.", // Inq self 16
    "The sun pierces through the clouds to illuminate $n.", // Inq room 17
    "A fire elemental singes your hair.", // Occ self 18
    "A fire elemental singes $n's hair.", // Occ room 19
    "A small explosion blows a hole in your pants.", // Alc self 20
    "A small explosion blows a hole in $n's pants.", // Alc room 21
    "w9s", // Wds self 22
    "w9o", // Wds room 23
    "You strike a boring pose!",          // un5 self 24
    "$n strikes a boring pose!",          // un5 room 25
    "You strike a boring pose!",          // un6 self 26
    "$n strikes a boring pose!"          // un6 room 27
  }
};

void do_pose( CHAR_DATA *ch, char *argument )
{
  int pose;

  if ( IS_NPC(ch) )
    return;

  pose = number_range(0, POSES-1);

  if (ch->desc && ch->desc->repeat > 1)
    return;

  act( pose_table[pose][(ch->gameclass * 2)], ch, NULL, NULL, TO_CHAR );
  act( pose_table[pose][(ch->gameclass * 2) + 1], ch, NULL, NULL, TO_ROOM );

  return;
}
//   wiznet( buf, NULL, NULL, WIZ_WORKLIST, 0, get_trust(ch) );



void do_duh( CHAR_DATA *ch, char *argument )
{
  if (IS_SET(ch->pen_flags, PEN_NOTE))
  {
    send_to_char("Your note privileges hava been revoked by the gods.\n\r",ch);
    return;
  }
  ch->pcdata->fdduhs++;
  append_file( ch, DUH_FILE, argument );
  send_to_char( "DUH! logged.\n\r", ch );
  return;
}

void do_bug( CHAR_DATA *ch, char *argument )
{
  if (IS_SET(ch->pen_flags, PEN_NOTE))
  {
    send_to_char("Your note privileges have been revoked by the gods.\n\r",ch);
    return;
  }
  ch->pcdata->fdbugs++;
  append_file( ch, BUG_FILE, argument );
  send_to_char( "Bug logged.\n\r", ch );
  return;
}

void do_typo( CHAR_DATA *ch, char *argument )
{
  if (IS_SET(ch->pen_flags, PEN_NOTE))
  {
    send_to_char("Your note privileges have been revoked by the gods.\n\r",ch);
    return;
  }
  ch->pcdata->fdtypos++;
  append_file( ch, TYPO_FILE, argument );
  send_to_char( "Typo logged.\n\r", ch );
  return;
}

void do_client( CHAR_DATA *ch, char *argument )
{
  if ( IS_SET(ch->pen_flags, PEN_NOTE) )
  {
    send_to_char("Your note privileges have been revoked by the gods.\n\r",ch);
    return;
  }
  ch->pcdata->wlbuild++;
  append_file( ch, BUILDER_FILE, argument );
  send_to_char( "Client suggestion logged.\n\r", ch );
  return;
}

void do_hints( CHAR_DATA *ch, char *argument )
{
  if (!IS_IMMORTAL(ch))
  {
    do_function( ch, &do_newbie, "" );
    return;
  }

  if (IS_SET(ch->pen_flags, PEN_NOTE))
  {
    send_to_char("Your note privileges have been revoked by the gods.\n\r",ch);
    return;
  }
  append_hint( ch, argument );
  send_to_char( "Hint logged.\n\r", ch );
  return;
}

void do_rent( CHAR_DATA *ch, char *argument )
{
  send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
  return;
}

void do_qui( CHAR_DATA *ch, char *argument )
{
  send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
  return;
}
void do_quit( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA *d,*d_next;
  OBJ_DATA *onobj;
  char log_buf[MSL], buf[MSL];
  char strsave[MSL];
  int id;
  int v1;

  if ( IS_NPC(ch) )
    return;

  if ( ch->position == POS_FIGHTING )
  {
    send_to_char( "No way! You are fighting.\n\r", ch );
    return;
  }

  if ( ch->position == POS_STUNNED  )
  {
    send_to_char( "You can't leave while stunned.\n\r", ch );
    return;
  }

  if ( ch->position  < POS_STUNNED  )
  {
    send_to_char( "You're not DEAD yet.\n\r", ch );
    return;
  }

  if ( auction_info.high_bidder == ch || auction_info.owner == ch )
  {
    send_to_char("You still have a stake in the auction!\n\r",ch);
    return;
  }

  if (strcmp(argument, "imm-overide"))
  {
    if (IS_AFFECTED(ch, AFF_CHARM))
    {
      send_to_char("Your master would not like that.\n\r",ch);
      return;
    }

    if (IS_AFFECTED(ch, AFF_SLEEP))
    {
      send_to_char ( "You are SLEPT.  You cannot quit. To do so, or go LD is punishable by corpse eating.\n\r", ch);
      return;
    }
  }

  if ((ch->level <= LEVEL_HERO)&&(ch->incog_level))
  {
    send_to_char("Removing cloaking as you are leaving the game.\n\r",ch);
    ch->incog_level = 0;
  }

  if (ch->level <= 1 && (strcmp(argument, "imm-overide") != 0) )
  {
    mprintf(sizeof(strsave), strsave, "%s%s", PLAYER_DIR, capitalize(ch->name));
    remove(strsave);
  }

  v1 = ( ch->played + (int) (current_time - ch->logon) ) / 3600;
  if (ch->level <= 1 && strcmp(argument, "confirm") && v1 < 1 )
  {
    send_to_char("The system will not save your character because you\n\r",ch);
    send_to_char("have not played for an hour, nor reached level 2.\n\r\n\r",ch);
    send_to_char("In order to confirm, type \"quit confirm.\"\n\r",ch);
    return;
  }

  if ( ( ( onobj = ch->on ) != NULL ) // check if they are on quest bedroll
  &&  ( ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_MANA )
  ||    ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_HP )
  ||    ( onobj->pIndexData->vnum == OBJ_VNUM_BEDROLL_BOTH ) ) )
  {
    obj_from_room( onobj );
    obj_to_char( onobj, ch );
  }

  if ( ch->questdata->obj_vnum )
  {
    OBJ_DATA *obj;

    for ( obj = object_list; obj; obj = obj->next )
    {
      if ( ch->questdata->obj_vnum == obj->pIndexData->vnum
      && !strcmp( obj->owner, ch->name ) )
        break;
    }
    if ( obj && obj->carried_by != ch )
    {
      obj_from_room( obj );
      obj_to_char( obj, ch );
      // extract_quest_object( ch );
    }
  }

  if ( ( ch->pet != NULL )
  && ( ch->pet->in_room != ch->in_room ) )
  { // Save the animals!!!
    mprintf( sizeof(buf), buf, "You yell for %s to rejoin you.\n\r",
      ch->pet->short_descr);
    send_to_char( buf ,ch);
    move_to_room( ch->pet, ch->in_room );
  }
  send_to_char( "The Addiction will call you back...\n\r",ch);

  if ( ch->in_room
  &&   strcmp( argument, "imm-overide" ) )
    act( "$n returns to the dreams of $g.", ch, NULL, NULL, TO_WIZ_ROOM );

  if ( strcmp( argument, "imm-overide" ) )
  {
    if ( ch->desc )
      mprintf( sizeof(log_buf), log_buf, "{R%s{w@%s{x has quit. [VNUM: %d]", 
        ch->name,
        ch->desc->host,
        ch->in_room->vnum );
    else
      mprintf( sizeof(log_buf), log_buf, "{R%s{w has quit without descriptor.", 
        ch->name);
  }
  else
  {
    if ( ch->desc )
      mprintf( sizeof(log_buf), log_buf, "{R%s{w@%s{x has been unloaded. [VNUM: %d]",
        ch->name,
        ch->desc->host,
        ch->in_room->vnum );
    else
      mprintf( sizeof(log_buf), log_buf, "{R%s{w has been unloaded without descriptor.",
        ch->name);
  }

  REMOVE_BIT( ch->affected_by,AFF_LINKDEATH );
  log_string( log_buf );

  if ( strcmp( argument, "imm-overide" ) )
    if (ch->desc)
    {
      if (ch->desc->host)
      {
        if (ch->in_room) 
          mprintf( sizeof(buf), buf, "{R%s{w@%s rejoins the real world. [VNUM: %d]",
            capitalize(ch->name),
            ch->desc->host,
            ch->in_room->vnum);
        else
          mprintf( sizeof(buf), buf, "{R%s{w@%s rejoins the real world.",
            capitalize(ch->name),
            ch->desc->host);
          wiznet (buf, ch,NULL,WIZ_SITES,0,get_trust(ch));
      }
    }

  /*
   * After extract_char the ch is no longer valid!
   */
  if ( strcmp( argument, "imm-overide" ) )
    save_char_obj( ch, FALSE );
  id = ch->id;
  d = ch->desc;

  /* unload rescue quest mob, if they have one */
  if (ch->questdata->mob_vnum == MOB_VNUM_QUEST)
  {
    CHAR_DATA *qch;
    for ( qch = char_list ; qch ; qch = qch->next)
    {
      if ( !IS_NPC( qch ) )
        continue;
      if ( strstr(qch->name,ch->name )
      && ( qch->pIndexData->vnum == MOB_VNUM_QUEST ) )
      {
        extract_char(qch, TRUE);
        break;
      }
    }
  }

  extract_char( ch, TRUE );
  if ( d != NULL )
    close_socket( d );

  /* toast evil cheating bastards */
  for (d = descriptor_list; d != NULL; d = d_next)
  {
    CHAR_DATA *tch;

    d_next = d->next;
    tch = d->original ? d->original : d->character;
    if (tch && tch->id == id)
    {
      extract_char(tch,TRUE);
      close_socket(d);
    } 
  }

  return;
}

void do_save( CHAR_DATA *ch, char *argument )
{
  int v1;

  if ( IS_NPC(ch) )
    return;

  v1 = ( ch->played + (int) (current_time - ch->logon) ) / 3600;
  
  if ((ch->level < 2) && (v1 < 1)) 
  {
    send_to_char("To save you must play {g1 hour{w or reach {glevel 2{w.{x\n\r",ch);
    return;
  }

  save_char_obj( ch, FALSE );
  send_to_char("{wSaving. Remember that {RD{rr{wa{Dk{wn{ro{Rr{w also has automatic saving.{x\n\r", ch);
  WAIT_STATE(ch,PULSE_VIOLENCE);
  return;
}

void do_follow( CHAR_DATA *ch, char *argument )
{
  /* RT changed to allow unlimited following and follow the NOFOLLOW rules */
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Follow whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_CHARMED(ch))
  {
    act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
    return;
  }

  if ( victim == ch )
  {
    if ( ch->master == NULL )
    {
      send_to_char( "You already follow yourself.\n\r", ch );
      return;
    }
    stop_follower(ch);
    return;
  }

  if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_IMMORTAL(ch))
  {
    act("$N doesn't seem to want any followers.\n\r",
    ch,NULL,victim, TO_CHAR);
    return;
  }

  REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    
  if ( ch->master != NULL )
    stop_follower( ch );

  add_follower( ch, victim );
  return;
}

void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
  if ( ch->master != NULL )
  {
      bugf( "Add_follower: non-null master.");
      return;
  }

  ch->master = master;
  ch->leader = NULL;

  if ( can_see( master, ch ) )
    act( "$n now follows you.", ch, NULL, master, TO_VICT );

  act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

  return;
}

void stop_follower( CHAR_DATA *ch )
{
  if ( ch->master == NULL )
  {
    bugf( "Stop_follower: null master.");
    return;
  }

  if ( IS_AFFECTED(ch, AFF_CHARM) )
  {
    REMOVE_BIT( ch->affected_by, AFF_CHARM );
    affect_strip( ch, gsn_charm_person );
  }

  REMOVE_BIT(ch->act, ACT_PET);

  if ( ( can_see( ch->master, ch )
  || (is_same_group(ch, ch->master) ) )
    && ( ch->in_room != NULL ) )
  {
    printf_to_char( ch->master, "%s stops following you.\n\r",
      IS_NPC(ch) ? new_capitalize(ch->short_descr) : ch->name );
    act( "You stop following $N.",  ch, NULL, ch->master, TO_CHAR );
  }

  if (ch->master->pet == ch)
    ch->master->pet = NULL;

  if (IS_NPC(ch))
    ch->clan = NULL;

  ch->master = NULL;
  ch->leader = NULL;

  if (ch->pet != NULL) 
  {
    ch->pet->master = ch;
    ch->pet->leader = ch;
  }

  return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch, bool show )
{    
  CHAR_DATA *pet;

  if ((pet = ch->pet) != NULL)
  {
    stop_follower(pet);

    if ( (pet->in_room != NULL)
    &&   show )
      act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);

    extract_char(pet,TRUE);
  }
  ch->pet = NULL;

  return;
}

void die_follower( CHAR_DATA *ch )
{
  CHAR_DATA *fch, *fch_next;

  if ( ch->master != NULL )
  {
    if (ch->master->pet == ch)
      ch->master->pet = NULL;
    stop_follower( ch );
  }

  ch->leader = NULL;

  for ( fch = char_list; fch != NULL; fch = fch_next )
  {
    fch_next = fch->next;
    if ( fch->leader == ch )
      fch->leader = fch;
    if ( fch->master == ch )
    {
      //if (IS_NPC(fch) )
      //{
      //  act("$N returns to $S home.", ch, NULL, fch, TO_ROOM_ALL);
      //  extract_char(fch, TRUE);
      //} 
      //else
      //{

      if ( strstr(fch->name,ch->name )
      && ( fch->pIndexData->vnum == MOB_VNUM_QUEST ) )
        continue;

          stop_follower( fch );
      //}
    }
  }

  return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *och;
  CHAR_DATA *och_next;
  bool found;
  bool fAll;
  bool recovering = FALSE;

  argument = one_argument( argument, arg );
  one_argument(argument,arg2);

  if (is_name(arg2,"delete")
  || is_name(arg2,"mob")
  || is_name(arg2,"afk")
  || is_name(arg2,"ooc")
  || is_name(arg2,"quit")
  || is_name(arg2,"noloot")
  || is_name(arg2,"nocancel")
  || is_name(arg2,"autoall")
  || is_name(arg2,"compact")
  || is_name(arg2,"nofollow")
  || is_name(arg2,"autolist")
  || is_name(arg2,"autosplit")
  || is_name(arg2,"autosac")
  || is_name(arg2,"autodonate")
  || is_name(arg2,"autogold")
  || is_name(arg2,"autoloot")
  || is_name(arg2,"ansi")
  || is_name(arg2,"bank")
  || is_name(arg2,"quest")
  //|| is_name(arg2,"drop")
  //|| is_name(arg2,"give")
  //|| is_name(arg2,"remove")
  || is_name(arg2,"color")
  || is_name(arg2,"password")
  || is_name(arg2,"bounty")
  || is_name(arg2,"nosummon")
  || is_name(arg2,"donate")
  || is_name(arg2,"sacrifice")
  || is_name(arg2,"junk")
  || is_name(arg2,"practice")
  || is_name(arg2,"transfer")
  || is_name(arg2,"save")
  || is_name(arg2,"goto")
  || is_name(arg2,"train")
  || ( is_name(arg2,"slay")
  &&   !is_name( arg2, "sleep" ) ) //interesting solution ;)
  || is_name(arg2,"advance")
  || is_name(arg2,"smite")
  || is_name(arg2,"nochannel")
  || is_name(arg2,"gain")
  || is_name(arg2,"note")
  || is_name(arg2,"idea")
  || ( is_name(arg2,"rules")
  &&   !is_name(arg2,"remove")
  &&   !is_name(arg2,"rest") )
  || is_name(arg2,"penalty")
  || is_name(arg2,"change")
  || is_name(arg2,"news")
  || ( is_name(arg2,"rbreak")
  &&   !is_name(arg2,"remove")
  &&   !is_name(arg2,"rest") )
  || is_name(arg2,"ignore")
  || is_name(arg2,"tithe")
  || ( is_name(arg2,"remember")
  &&   !is_name(arg2,"remove")
  &&   !is_name(arg2,"rest") )
  || is_name(arg2,"brief")
  || is_name(arg2,"quiet")
  || is_name(arg2,"guild")
  || is_name(arg2,"promote")  )
  //|| is_name(arg2,"yell"))
  {
    send_to_char("That will NOT be done.\n\r",ch);
    return;
  }

  if ( arg[0] == '\0' || argument[0] == '\0' )
  {
    send_to_char( "Order whom to do what?\n\r", ch );
    return;
  }

  if ( IS_AFFECTED( ch, AFF_CHARM ) )
  {
    send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg, "all" ) )
  {
    if ( (is_name(arg2,"rescue"))
    &&   !is_name(arg2,"rest") )
    {
      send_to_char("Sorry.. no can do.\n\r",ch);
      return;
    }
    if ( (is_name(arg2,"report"))
    &&   !is_name(arg2,"rest") )
    {
      send_to_char("Sorry...have them report individually.\n\r", ch);
      return;
    }
    fAll   = TRUE;
    victim = NULL;
  }
  else
  {
    fAll   = FALSE;
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    if ( victim == ch )
    {
      send_to_char( "Aye aye, right away!\n\r", ch );
      return;
    }

    if (!IS_AFFECTED(victim, AFF_CHARM)
    ||  (IS_IMMORTAL(victim) && victim->trust >= ch->trust))
    {
      send_to_char( "Do it yourself!\n\r", ch );
      return;
    }

    if (victim->master != ch )
    {
      mprintf( sizeof(arg), arg, "2.%s", arg ); // See if there is a 2nd mob w/ same name
      if ( ( victim = get_char_room( ch, arg ) ) == NULL )
      {
        send_to_char( "Do it yourself!\n\r", ch );
        return;
      }

      if ( victim == ch )
      {
        send_to_char( "Aye aye, right away!\n\r", ch );
        return;
      }

      if (!IS_AFFECTED(victim, AFF_CHARM)
      ||  (IS_IMMORTAL(victim) && victim->trust >= ch->trust))
      {
        send_to_char( "Do it yourself!\n\r", ch );
        return;
      }

    }


    if (!IS_NPC(victim) && is_name("report", arg2) )
    {
      send_to_char("Your charmie hands you a piece of paper.\n\r", ch);
      send_to_char("It says: 'Tomorrow's weather.  Overcast with a 60% chance of showers.'\n\r", ch);
    }

    if (!IS_SET(ch->act, PLR_AUTOASSIST) && !IS_NPC(victim))
    {
      send_to_char("Your charmie looks at you.. wondering if you are still strong enough to hold them.\n\r",ch);
      return;
    }

    if (!IS_SET(ch->spell_aff, SAFF_DETER) && !IS_NPC(victim))
    {
      send_to_char("Your charmie looks at you.. wondering if you are still strong enough to hold them.\n\r",ch);
      return;
    }

    if (is_name(arg2,"rescue"))
    {
      if (!IS_NPC(victim)
      || (!IS_SET(victim->act,ACT_WARRIOR))
      || (!IS_PET(victim)))
      {
        send_to_char("You can only order warrior pets to rescue.\n\r",ch);
        return;
      }
    }
  }

  found = FALSE;
  for ( och = ch->in_room->people; och != NULL; och = och_next )
  {
    och_next = och->next_in_room;

    if ( IS_AFFECTED(och, AFF_CHARM)
    &&   och->master == ch
    && ( fAll || och == victim ) )
    {
      if (!IS_SET(ch->act, PLR_AUTOASSIST) && !IS_NPC(och))
      {
        send_to_char("Your charmie looks at you.. wondering if you are still strong enough to hold them.\n\r",ch);
        continue;
      }

      if (och->wait > 0)
      {
        send_to_char("Your charmie is still recovering from their last action.\n\r", ch);
        recovering = TRUE;
        continue;
      }

      found = TRUE;
      mprintf( sizeof(buf), buf, "$n orders you to '%s'.", argument );
      act( buf, ch, NULL, och, TO_VICT );
      interpret( och, argument );
    }
  }

  if ( found )
  {
    WAIT_STATE(ch,PULSE_VIOLENCE);
    send_to_char( "Ok.\n\r", ch );
  }
  else
    if (!recovering)
      send_to_char( "You have no followers here.\n\r", ch );

  return;
}

void do_group( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *gch, *victim;
  char buf1[MSL],buf2[MSL], buf3[MSL], buf4[MSL], gr_name[MSL];
  bool same_clan = TRUE, oldset = FALSE;
  int max_level, min_level;

  one_argument( argument, arg );
  if (IS_GHOST(ch)) {
    send_to_char("Grouping someone is useless as you are still DEAD.\n\r",ch);
    return;
  }
  if ( arg[0] == '\0' )
  {
    /*CHAR_DATA *gch;*/
    CHAR_DATA *leader;

    leader = (ch->leader != NULL) ? ch->leader : ch;
    printf_to_char(ch, "%s's group:\n\r", PERS(leader, ch) );

    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
      if ( is_same_group( gch, ch ) )
      {
        oldset = FALSE;
        if (gch != ch)
        {
          if (IS_SET(gch->plr2, PLR2_STATS))
            oldset = TRUE;
          REMOVE_BIT(gch->plr2, PLR2_STATS);
        }

        if (gch->master == ch && IS_PET(gch))
        {
          if (IS_SET(ch->plr2, PLR2_STATS))
            SET_BIT(gch->plr2, PLR2_STATS);
          else
            REMOVE_BIT(gch->plr2,PLR2_STATS);
        }
        strcpy(buf1,show_stat_info(gch,gch->hit, gch->max_hit,10,"=","-",FALSE));
        strcpy(buf2,show_stat_info(gch,gch->mana, gch->max_mana,10,"=","-",TRUE));
        strcpy(buf3,show_stat_info(gch,gch->move, gch->max_move,10,"=","-",FALSE));
        strcpy(buf4, capitalize( PERS(gch, ch) ) );
        strncpy_color(gr_name, buf4, 16, ' ', TRUE ); 
          
        printf_to_char(ch,
          "{D[{R%3d {w%s{D] {w%-16s {R%5d/{r%5d hp {G%5d/{g%5d mana {B%5d/{b%5d mv {gExp: {D%d {x\n\r",
          gch->level,
          IS_NPC(gch) ? "Mob" : class_table[gch->gameclass].who_name,
          gr_name, //capitalize( PERS(gch, ch) ),
          gch->hit,
          GET_HP(gch),
          gch->mana,
          GET_MANA(gch),
          gch->move,
          gch->max_move,
          IS_NPC( gch ) ? gch->exp :
            gch->level >= 100 ? gch->exp :
              TNL( exp_per_level( gch, gch->pcdata->points ), 
              gch->level ) - gch->exp
              );
        if ( gch != ch )
        {
          if ( oldset )
            SET_BIT( gch->plr2, PLR2_STATS );
        }
      }
    }
    return;
  }

// "group all" will add all non-grouped followers to a group
  if (!strcmp( arg, "all" ) )
  {
    oldset = FALSE; // reusing a variable
    for (gch = ch->in_room->people; gch ; gch = gch->next_in_room)
    {
      if ( ( gch != ch )       // not self
      && ( gch->master == ch ) // gch is following ch
      && ( gch->leader != ch ) // gch is following ch
      && ( gch->position != POS_SLEEPING )
      && ( !IS_AFK( gch ) ) 
      && ( !IS_LINKDEAD( gch ) )
      && ( !IS_NPC( gch ) ) 
      && ( !IS_GHOST( gch ) )
      && ( !IS_AFFECTED(gch,AFF_CHARM) )
      && ( can_see( ch, gch ) ) )
      {
        do_function( ch, &do_group, gch->name );
        oldset = TRUE;
      }
    }
    if (!oldset)
      send_to_char( "No ungrouped characters are following you.\n\r", ch );
    return;
  }

  if ( ch->position == POS_SLEEPING )
  {
    send_to_char( "You can't group someone while you are asleep.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( ch->master != NULL
  || ( ch->leader != NULL
  && ch->leader != ch ) )
  {
    send_to_char( "But you are following someone else!\n\r", ch );
    return;
  }

  if (IS_GHOST(victim)) {
    send_to_char("Grouping someone who is a ghost is stupid.\n\r",ch);
    return;
  }

  /*if ((ch->clan != victim->clan)&&((LEVEL_GRP_RANGE < abs(ch->level - victim->level))))
    {
      send_to_char( "You cannot group with them!\n\r", ch );
      return;
    }*/


  max_level = min_level = ch->level;
  if (ch->clan != victim->clan)
    same_clan = FALSE;

  for (gch = player_list; gch != NULL; gch = gch->next_player)
  {
    if (gch != ch  && !IS_NPC(gch) && is_same_group(gch, ch))
    {
      if (gch->level < min_level)
        min_level = gch->level;
      if (gch->level > max_level)
        max_level = gch->level;
      if (gch->clan != victim->clan)
        same_clan = FALSE;
    }
  }

  if ( ( (LEVEL_GRP_RANGE < victim->level - min_level)
  || (LEVEL_GRP_RANGE < max_level - victim->level) )
  &&  !same_clan && victim->level < 80 )
  {
    send_to_char("{cThey cannot be added to your group.\n\r{x", ch);
    return;
  }

  if ( (max_level - min_level > LEVEL_GRP_RANGE) && !same_clan && victim->level < 80)
  {
    send_to_char("{cThey cannot be added to your group.\n\r{x", ch);
    return;
  }

  if ( victim->master != ch && ch != victim )
  {
    act_new("{c$N isn't following you.{c",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
    return;
  }

  if (IS_AFFECTED(victim,AFF_CHARM))
  {
    send_to_char("{cYou can't remove charmed mobs from your group.\n\r{x",ch);
    return;
  }

  if ( IS_NPC( victim )
  && ( victim->pIndexData->vnum == MOB_VNUM_QUEST ) )
  {
    act_new("{cYou cannot add $m to your group!{x",
        ch,NULL,victim,TO_CHAR,POS_SLEEPING);
    return;
  }

  if (IS_AFFECTED(ch,AFF_CHARM))
  {
    act_new("{cYou like your master too much to leave $m!{x",
        ch,NULL,victim,TO_VICT,POS_SLEEPING);
    return;
  }

  if ( is_same_group( victim, ch ) && ch != victim )
  {
    victim->leader = NULL;
    victim->group_num = victim->id;
    act_new("{c$n removes $N from $s group.{x",
        ch,NULL,victim,TO_NOTVICT,POS_RESTING);
    act_new("{c$n removes you from $s group.{x",
        ch,NULL,victim,TO_VICT,POS_SLEEPING);
    act_new("{cYou remove $N from your group.{x",
        ch,NULL,victim,TO_CHAR,POS_SLEEPING);
    if (victim->pet != NULL && is_same_group( victim->pet, ch ) )
    {
      victim->pet->leader = victim;
      victim->pet->group_num = victim->group_num;
      act_new("{c$n removes $N from $s group.{x",
          ch,NULL,victim->pet,TO_NOTVICT,POS_RESTING);
      act_new("{cYou remove $N from your group.{x",
          ch,NULL,victim->pet,TO_CHAR,POS_SLEEPING);
    }
    return;
  }

  if (victim == ch)
  {
    send_to_char("{cJoining your own group and setting you to leader.\n\r{x",ch);
    act_new("{c$n is conceited and won't join your group.{x",ch,NULL, NULL,TO_ROOM,POS_RESTING);
    victim->leader = ch;
    victim->group_num = victim->id;
    if (victim->pet != NULL)
    {
      act_new("{c$N joins $n's group.{x",ch,NULL,victim->pet,TO_NOTVICT,POS_RESTING);
      act_new("{c$N joins your group.{x",ch,NULL,victim->pet,TO_CHAR,POS_SLEEPING);
      victim->pet->leader = ch;
      victim->pet->group_num = victim->id;
    }
  }
  else
  {
    victim->leader = ch;
    victim->group_num = ch->group_num;
    act_new("{c$N joins $n's group.{x",ch,NULL,victim,TO_NOTVICT,POS_RESTING);
    act_new("{cYou join $n's group.{x",ch,NULL,victim,TO_VICT,POS_SLEEPING);
    act_new("{c$N joins your group.{x",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
    if (victim->pet != NULL)
    {
      victim->pet->leader = ch;
      victim->pet->group_num = ch->group_num;
      act_new("{c$N joins $n's group.{x",ch,NULL,victim->pet,TO_NOTVICT,POS_RESTING);
      act_new("{c$N joins your group.{x",ch,NULL,victim->pet,TO_CHAR,POS_SLEEPING);
    }
  }
  return;
}

void change_group_leader (CHAR_DATA *old_leader, CHAR_DATA *new_leader)
{
  CHAR_DATA *rch, *rch_next;

  if (old_leader == NULL)
  {
    act("OLD_LEADER = NULL", NULL, NULL, NULL, TO_ALL);
    return;
  }

  if (new_leader == NULL)
  {
    act("NEW_LEADER = NULL", NULL, NULL, NULL, TO_ALL);
    return;
  }

  if (IS_NPC(old_leader) || IS_NPC(new_leader))
  {
    bugf("change_group_leader: %s is an NPC.\n\r", IS_NPC(old_leader) ?
      "old_leader" : "new_leader");
    return;
  }

  new_leader->leader = NULL;
  new_leader->master = NULL;
  new_leader->group_num = new_leader->id;

  if (new_leader->pet != NULL)
  {
    new_leader->pet->leader = new_leader;
    new_leader->pet->group_num = new_leader->group_num;
  }

  for (rch = player_list; rch != NULL; rch = rch_next)
  {
    rch_next = rch->next_player;
    if (rch == old_leader)
      continue;

    if (is_same_group(old_leader, rch))
    {
      rch->leader = new_leader;
      rch->master = new_leader;
      rch->group_num = new_leader->group_num;
      if (rch->pet != NULL)
      {
        rch->pet->leader = new_leader;
        rch->pet->group_num = new_leader->group_num;
      }
    }
  }

  old_leader->leader = new_leader;
  old_leader->master = new_leader;
  old_leader->group_num = new_leader->group_num;
  if (old_leader->pet != NULL)
  {
    old_leader->pet->leader = new_leader;
    old_leader->pet->group_num = new_leader->group_num;
  }

  act("$n is now the group's leader.", new_leader, NULL, NULL, TO_GROUP);
  send_to_char("You are now the group's leader.\n\r", new_leader);
}

/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *gch;
  int members;
  int amount_gold = 0, amount_silver = 0;
  int share_gold, share_silver;
  int extra_gold, extra_silver;

  argument = one_argument( argument, arg1 );
  one_argument( argument, arg2 );

  if ( arg1[0] == '\0' )
  {
    send_to_char( "Split how much?\n\r", ch );
    return;
  }
    
  amount_silver = atoi( arg1 );

  if (arg2[0] != '\0')
    amount_gold = atoi(arg2);

  if ( amount_gold < 0 || amount_silver < 0)
  {
    send_to_char( "Your group wouldn't like that.\n\r", ch );
    return;
  }

  if ( amount_gold == 0 && amount_silver == 0 )
  {
    send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
    return;
  }

  if ( ch->gold <  amount_gold || ch->silver < amount_silver)
  {
    send_to_char( "You don't have that much to split.\n\r", ch );
    return;
  }
  
  members = 0;
  for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
  {
    if ( is_same_group( gch, ch )
    && !IS_AFFECTED(gch,AFF_CHARM))
      members++;
  }
/*
  if ( members < 2 )
  {
    send_to_char( "Just keep it all.\n\r", ch );
    return;
  }
*/

  share_silver = amount_silver / members;
  extra_silver = amount_silver % members;

  share_gold   = amount_gold / members;
  extra_gold   = amount_gold % members;

  if ( share_gold == 0 && share_silver == 0 )
  {
    send_to_char( "Don't even bother, cheapskate.\n\r", ch );
    return;
  }

  ch->silver    -= amount_silver;
  ch->silver    += share_silver + extra_silver;
  ch->gold     -= amount_gold;
  ch->gold     += share_gold + extra_gold;


  if ( ( share_silver > 0 )
  && !IS_SET( ch->nospam, NOSPAM_MSPLIT ) )
    printf_to_char(ch,
      "You split{W %d {wsi{Wl{Dv{wer{x coins. Your share is {W%d {wsi{Wl{Dv{wer{x.\n\r",
      amount_silver,
      share_silver + extra_silver);

  if ( ( share_gold > 0 )
  && !IS_SET( ch->nospam, NOSPAM_MSPLIT ) )
    printf_to_char(ch,
      "You split {Y%d {yg{Yo{yld{x coins. Your share is {Y%d {yg{Yo{yld{x.\n\r",
      amount_gold,
      share_gold + extra_gold);

  if (share_gold == 0)
    mprintf(sizeof(buf), buf,
      "$n splits {W%d {wsi{Wl{Dv{wer{x coins. Your share is {W%d {wsi{Wl{Dv{wer{x.",
      amount_silver,
      share_silver);
  else if (share_silver == 0)
    mprintf(sizeof(buf), buf,"$n splits {Y%d {yg{Yo{yld{x coins. Your share is {Y%d {yg{Yo{yld{x.",
      amount_gold,
      share_gold);
  else
    mprintf(sizeof(buf), buf,
      "$n splits {W%d {wsi{Wl{Dv{wer{x and {Y%d {yg{Yo{yld {xcoins, giving you {W%d {wsi{Wl{Dv{wer {xand {Y%d {yg{Yo{yld{x.\n\r",
      amount_silver,
      amount_gold,
      share_silver,
      share_gold);

  for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
  {
    if ( gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
    {
      if ( !IS_SET( gch->nospam, NOSPAM_MSPLIT ) )
        act( buf, ch, NULL, gch, TO_VICT );
      gch->gold += share_gold;
      gch->silver += share_silver;
    }
  }

  return;
}

/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
  if ( ach == NULL || bch == NULL)
    return FALSE;
  if ( ach->leader)
    ach = ach->leader;
  if ( bch->leader )
    bch = bch->leader;

  return ach == bch;
}

/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool test_is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
  if ( ach == NULL || bch == NULL)
    return FALSE;

  if ( ach->leader)
    ach = ach->leader;
  else
    if (ach->master)
      ach = ach->master;
  
  if ( bch->leader )
    bch = bch->leader;
  else
    if (bch->master)
      bch = bch->master;
  
  return ach == bch;
}

/*
 * Colour setting and unsetting, way cool, Lope Oct '94
 */
void do_colour( CHAR_DATA *ch, char *argument )
{
  char arg[ MAX_STRING_LENGTH ];

  argument = one_argument( argument, arg );

  if( !*arg )
  {
    if( !IS_SET( ch->act, PLR_COLOUR ) )
    {
      SET_BIT( ch->act, PLR_COLOUR );
      send_to_char( "{bC{ro{yl{co{mu{gr{x is now {rON{x, Way Cool!\n\r", ch );
    }
    else
    {
      send_to_char_bw( "Colour is now OFF, <sigh>\n\r", ch );
      REMOVE_BIT( ch->act, PLR_COLOUR );
    }
    return;
  }
  else
  {
    send_to_char_bw( "Colour Configuration is unavailable in this\n\r", ch );
    send_to_char_bw( "version of colour, sorry\n\r", ch );
  }

  return;
}

void do_gocial(CHAR_DATA *ch, char *argument)
{
  char command[MAX_INPUT_LENGTH];
  /*CHAR_DATA *victim;*/
  int cmd;
  bool found;
  char arg[MAX_INPUT_LENGTH];
  /*  int counter;
  int count;
  char buf2[MAX_STRING_LENGTH];
  */
  argument = one_argument(argument,command);

  if (command[0] == '\0')
  {
    send_to_char("What do you wish to gocial?\n\r",ch);
    return;
  }

  found = FALSE;
  for (cmd = 0; social_table[cmd].name[0] != '\0'; cmd++)
  {
    if (command[0] == social_table[cmd].name[0]
    && !str_prefix( command,social_table[cmd].name ) )
    {
      found = TRUE;
      break;
    }
  }

  if (!found)
  {
    send_to_char("What kind of social is that?!?!\n\r",ch);
    return;
  }

  if (!IS_NPC(ch) && IS_SET(ch->chan_flags,CHANNEL_QUIET))
  {
    send_to_char("You must turn off quiet mode first.\n\r",ch);
    return;
  }

  if ( !IS_NPC(ch) && IS_SET(ch->chan_flags, CHANNEL_NOGOSSIP))
  {
    send_to_char("But you have the gossip channel turned off!\n\r",ch);
    return;
  }

  if ( !IS_NPC(ch) && IS_SET(ch->pen_flags, PEN_NOCHANNELS))
  {
    send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
    return;
  }

  switch (ch->position)
  {
    case POS_DEAD:
      send_to_char("Lie still; you are DEAD!\n\r",ch);
      return;
    case POS_INCAP:
    case POS_MORTAL:
      send_to_char("You are hurt far too bad for that.\n\r",ch);
      return;
    case POS_STUNNED:
      send_to_char("You are too stunned for that.\n\r",ch);
      return;
  }

  if ((social_table[cmd].char_no_arg == NULL)
  || (social_table[cmd].others_no_arg == NULL)
  || (social_table[cmd].char_found == NULL)
  || (social_table[cmd].others_found == NULL)
  || (social_table[cmd].vict_found == NULL)
  || (social_table[cmd].char_auto == NULL)
  || (social_table[cmd].others_auto == NULL))
    bugf("SOCIAL (GOCIAL) :%s is missing an arg.\n\r",social_table[cmd].name);

  one_argument(argument,arg);
  return;
}

char * makedrunk (char *string, CHAR_DATA * ch)
{
  /* This structure defines all changes for a character */
  struct struckdrunk drunk[] =
  {
    {3, 10,
     {"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"}},
    {8, 5,
     {"b", "b", "b", "B", "B", "vb"}},
    {3, 5,
     {"c", "c", "C", "cj", "sj", "zj"}},
    {5, 2,
     {"d", "d", "D"}},
    {3, 3,
     {"e", "e", "eh", "E"}},
    {4, 5,
     {"f", "f", "ff", "fff", "fFf", "F"}},
    {8, 2,
     {"g", "g", "G"}},
    {9, 6,
     {"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}},
    {7, 6,
     {"i", "i", "Iii", "ii", "iI", "Ii", "I"}},
    {9, 5,
     {"j", "j", "jj", "Jj", "jJ", "J"}},
    {7, 2,
     {"k", "k", "K"}},
    {3, 2,
     {"l", "l", "L"}},
    {5, 8,
     {"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"}},
    {6, 6,
     {"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
    {6, 6,
     {"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
    {3, 6,
     {"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"}},
    {3, 2,
     {"p", "p", "P"}},
    {5, 5,
     {"q", "q", "Q", "ku", "ququ", "kukeleku"}},
    {4, 2,
     {"r", "r", "R"}},
    {2, 5,
     {"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"}},
    {5, 2,
     {"t", "t", "T"}},
    {3, 6,
     {"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"}},
    {4, 2,
     {"v", "v", "V"}},
    {4, 2,
     {"w", "w", "W"}},
    {5, 6,
     {"x", "x", "X", "ks", "iks", "kz", "xz"}},
    {3, 2,
     {"y", "y", "Y"}},
    {2, 9,
     {"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}}
  };

  char buf[1024];
  char temp;
  int pos = 0;
  int drunklevel;
  int randomnum;

  /* Check how drunk a person is... */
  if (IS_NPC(ch))
    drunklevel = 0;
  else
    drunklevel = ch->pcdata->condition[COND_DRUNK];

  if (drunklevel > 0)
  {
    do
    {
      if ( pos >= MAX_STRING_LENGTH )
        break;  /* Make sure the slurred sting isnt too long! */
      temp = toupper (*string);
      if ((temp >= 'A') && (temp <= 'Z'))
      {
        if (drunklevel > drunk[temp - 'A'].min_drunk_level)
        {
          randomnum = number_range (0, drunk[temp - 'A'].number_of_rep);
          strcpy (&buf[pos], drunk[temp - 'A'].replacement[randomnum]);
          pos += strlen (drunk[temp - 'A'].replacement[randomnum]);
        }
        else
          buf[pos++] = *string;
      }
      else
      {
        if ((temp >= '0') && (temp <= '9'))
        {
          temp = '0' + number_range (0, 9);
          buf[pos++] = temp;
        }
        else
          buf[pos++] = *string;
      }
    }
    while (*string++);
    buf[pos] = '\0';
    /* Mark end of the string... */
    strcat(buf," <*HicCup*>\0");
    strcpy(string, buf);
    return(string);
  }

  return (string);
}


void do_as(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MSL];
  int position=-1;
  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2);
  if ( arg[0] == '\0' || arg2[0] == '\0')
  {
    send_to_char( "Syntax: AS NUMBER NEW_STRING\n\r", ch );
    return;
  }

  position = atoi(arg);
  if ((position < 0) || (position >= MAX_NUM_AUTOSNOOP))
  {
    send_to_char("Position is outta range\n\r", ch);
    return;
  }

  printf_to_char(ch,"{GSwitching from using %s to use %s{x\n\r",as_string[position],capitalize(arg2));
  strcpy(as_string[position],capitalize(arg2));
  save_sys_data();
  return;
}

void do_aslist(CHAR_DATA *ch, char *argument)
{
  int i;

  send_to_char ("{RAUTOSNOOPING THE FOLLOWING:{x\n\r", ch);
  for (i=0; i < MAX_NUM_AUTOSNOOP; i++)
  {
    send_to_char("{C",ch);
    send_to_char(as_string[i], ch);
    send_to_char("{x\n\r", ch);
  }
}

void do_newsfaerie( CHAR_DATA *ch, char *argument)
{
  if ( IS_SET(ch->comm_flags, COMM_NEWSFAERIE) )
  {
    send_to_char("{gNewsFaerie will no longer inform you of message board postings.{x\n\r", ch);
    REMOVE_BIT( ch->comm_flags, COMM_NEWSFAERIE);
  }
  else
  {
    send_to_char("{gNewsFaerie will now inform you of message board postings.{x\n\r", ch);
    SET_BIT( ch->comm_flags, COMM_NEWSFAERIE);
  }
}

void clean_auction() 
{
  auction_info.item        = NULL;
  auction_info.owner       = NULL;
  auction_info.high_bidder = NULL;
  auction_info.current_bid = 0;
  auction_info.status      = 0;
  auction_info.gold_held   = 0;
  auction_info.silver_held = 0;
  auction_info.minimum_bid = 0;
}

void do_auction( CHAR_DATA *ch, char * argument )
{
  long gold=0,silver=0;
  OBJ_DATA *    obj;
  long minimum=0;
  char arg1[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  char temp[30],temp1[30];
  argument = one_argument( argument, arg1 );

  if ( ch == NULL || IS_NPC(ch) )
    return;

  if (IS_NOCHANNELED(ch) && !IS_GHOST(ch))
  {
    send_to_char("The gods have revoked your channel privledges.\n\r", ch);
    return;
  }

  if ( IS_NULLSTR( arg1 ) )
  {
    send_to_char( "Syntax: auction <info|toggle|stop>\n\r", ch );
    send_to_char( "        auction <item> <min_price>\n\r", ch );
    send_to_char( "        auction bid <number of item>\n\r", ch );
    return;
  }

  if ( !str_cmp( arg1, "toggle" ) )
  {
    if ( IS_SET(ch->chan_flags,CHANNEL_NOAUCTION) )
    {
      REMOVE_BIT(ch->chan_flags,CHANNEL_NOAUCTION );
      send_to_char("{yAuction channel is now ON.{x\n\r",ch);
      return;
    }

    SET_BIT(ch->chan_flags,CHANNEL_NOAUCTION);
    send_to_char("{yAuction channel is now OFF.{x\n\r",ch);
    return;
  }

  if ( !str_cmp( arg1, "info" ) )
  {
    obj = auction_info.item;

    if ( !obj )
    {
      send_to_char("There is nothing up for auction right now.\n\r",ch);
      return;
    }

    if ( auction_info.owner == ch )
    {
      printf_to_char(ch, "You are currently auctioning %s.\n\r",
        obj->short_descr );
      printf_to_char(ch, "Current high bid is %ld gold.\n\r",
        auction_info.current_bid);
      return;
    }
    else
    {
      if (IS_IMMORTAL(ch)) {
        send_to_char("{Y---------------------------------------------------{x\n\r",ch);
        if (auction_info.high_bidder)
          printf_to_char(ch,"{cTOP Bidder: {W%s.\n\r",auction_info.high_bidder->name);
        if (auction_info.owner)
          printf_to_char(ch,"{W%s {cis auctioning the following item:\n\r",auction_info.owner->name);
        send_to_char("{Y---------------------------------------------------{x\n\r",ch);
      }
      show_obj_stats( ch, obj );
    }

    return;
  }

  if ( !str_cmp( arg1, "stop" )&& ( auction_info.owner == ch ))
  {
    mprintf(sizeof(buf), buf, "Item %s is removed from bidding.\n\r",
      auction_info.item->short_descr);
    auction_channel( buf );
  
    obj_to_char( auction_info.item, auction_info.owner );
      
    printf_to_char(auction_info.owner, "%s is returned to you.\n\r",
      capitalize(auction_info.item->short_descr) );

    if (auction_info.high_bidder)
    {
      printf_to_char(auction_info.high_bidder,"Your %ld gold and %ld silver is returned to you.\n\r",
        auction_info.gold_held, auction_info.silver_held);
      auction_info.high_bidder->gold += auction_info.gold_held;
      auction_info.high_bidder->silver += auction_info.silver_held;
    }
    clean_auction();
    return;
  }

  if ( !str_cmp( arg1, "sell" )&& ( auction_info.owner == ch ))
  {
    if ( auction_info.current_bid == 0 )
    {
      mprintf(sizeof(buf), buf, "No bids on %s - item removed.\n\r",
          auction_info.item->short_descr);
      auction_channel( buf );

      obj_to_char( auction_info.item, auction_info.owner );

      printf_to_char(auction_info.owner, "%s is returned to you.\n\r",
             capitalize(auction_info.item->short_descr) );
    
      clean_auction();
      return;
    }
    else
    {
      mprintf(sizeof(buf), buf,"%s SOLD for %ld gold.\n\r",
          capitalize(auction_info.item->short_descr),
          auction_info.current_bid );
      auction_channel( buf );

      auction_info.owner->gold += auction_info.gold_held;
      auction_info.owner->silver += auction_info.silver_held;

      mprintf(sizeof(temp1), temp1, "%ld gold ", auction_info.gold_held );
      mprintf(sizeof(temp), temp,  "%ld silver ", auction_info.silver_held );
      printf_to_char(auction_info.owner, "You receive %s%s%scoins.\n\r",
             auction_info.gold_held > 0 ? temp1 : "",
             ((auction_info.gold_held > 0) &&
              (auction_info.silver_held > 0)) ? "and " : "",
             auction_info.silver_held > 0 ? temp : "" );
      obj_to_char( auction_info.item, auction_info.high_bidder );

      printf_to_char(auction_info.high_bidder, "%s appears in your hands.\n\r",
             capitalize(auction_info.item->short_descr) );
      clean_auction();
      return;
    }
  }

  if ( !str_cmp( arg1, "bid" ) )
  {
    long bid;
    obj = auction_info.item;

    if ( !obj )
    {
      send_to_char("There is nothing up for auction right now.\n\r",ch);
      return;
    }

    if ( argument[0] == '\0' )
    {
      send_to_char("You must enter an amount to bid.\n\r",ch);
      return;
    }

    bid = atol( argument );

    if ( bid <= auction_info.current_bid )
    {
      printf_to_char(ch, "You must bid above the current bid of %ld gold.\n\r",
             auction_info.current_bid );
      return;
    }

    if ( bid < auction_info.minimum_bid )
    {
      printf_to_char( ch, "The minimum bid is %ld gold.\n\r",auction_info.minimum_bid);
      return;
    }

    if ( (ch->gold + (ch->silver/100)) < bid )
    {
      send_to_char("You can't cover that bid.\n\r",ch);
      return;
    }

    mprintf(sizeof(buf), buf, "%s has offered %ld gold for %s.\n\r",
        capitalize(ch->name), bid, auction_info.item->short_descr);
    auction_channel( buf );

    if ( auction_info.high_bidder != NULL )
    {
      auction_info.high_bidder->gold += auction_info.gold_held;
      auction_info.high_bidder->silver += auction_info.silver_held;
    }

    gold = UMIN( ch->gold, bid );

    if ( gold < bid )
    {
      silver =(( bid - gold) * 100);
    }

    ch->gold -= gold;
    ch->silver -= silver;

    auction_info.gold_held    = gold;
    auction_info.silver_held    = silver;
    auction_info.high_bidder    = ch;
    auction_info.current_bid    = bid;
    auction_info.status         = 0;
    return;    
  }

  if ( auction_info.item != NULL )
  {
    send_to_char("There is already another item up for bid.\n\r",ch);
    return;
  }

  if ( (obj = get_obj_carry( ch, arg1, ch )) == NULL )
  {
    send_to_char("You aren't carrying that item.\n\r",ch);
    return;
  }

  if (obj->cost <= 1000) {
    send_to_char("That item is worthless over auction channels.\n\r",ch);
    return;
  }

  if (IS_NULLSTR(argument))
    minimum = 0;
  else
    minimum = atol( argument );

  if (minimum <=0)
    minimum = 0;

  printf_to_char(ch,"Your minimum bid is set at %ld gold.\n\r",minimum);

  if ( IS_OBJ_STAT( obj, ITEM_NODROP ) )
  {
    send_to_char("You can't let go of that item.\n\r",ch);
    return;
  }

  if ( IS_OBJ_STAT( obj, ITEM_NOREMOVE ) )
  {
    send_to_char("You can't let go of that item.\n\r",ch);
    return;
  }

  if ( IS_OBJ_STAT( obj, ITEM_ROT_DEATH ) )
  {
    send_to_char("You can't let go of that item.\n\r",ch);
    return;
  }

  if (obj->timer > 0)
  {
    send_to_char("You cannot let it go for fear it will disappear.\n\r",ch);
    return;
  }

  auction_info.owner        = ch;
  auction_info.item        = obj;
  auction_info.current_bid    = 0;
  auction_info.status        = 0;
  auction_info.minimum_bid      = minimum;
  mprintf(sizeof(buf), buf,"Min Bid[%ld]:Now taking bids on %s. \n\r",
    minimum,
    obj->short_descr );

  auction_channel( buf );

  obj_from_char( obj );

  return;

}

void auction_update( )
{
  char buf[MAX_STRING_LENGTH];
  char temp[MAX_STRING_LENGTH];
  char temp1[MAX_STRING_LENGTH];

  if ( auction_info.item == NULL )
    return;

  auction_info.status++;

  if ( auction_info.status == AUCTION_LENGTH )
  {
    mprintf(sizeof(buf), buf,"%s SOLD for %ld gold.\n\r",
        auction_info.item->short_descr,
        auction_info.current_bid );
    auction_channel( buf );

    auction_info.owner->gold += auction_info.gold_held;
    auction_info.owner->silver += auction_info.silver_held;

    mprintf(sizeof(temp1), temp1, "%ld gold ", auction_info.gold_held );
    mprintf(sizeof(temp), temp,  "%ld silver ", auction_info.silver_held );
    printf_to_char(auction_info.owner, "You receive %s%s%scoins.\n\r",
        auction_info.gold_held > 0 ? temp1 : "",
        ((auction_info.gold_held > 0) &&
         (auction_info.silver_held > 0)) ? "and " : "",
        auction_info.silver_held > 0 ? temp : "" );
    obj_to_char( auction_info.item, auction_info.high_bidder );

    printf_to_char(auction_info.high_bidder, "%s appears in your hands.\n\r",
        auction_info.item->short_descr);
    clean_auction();
    return;
  }

  if ( auction_info.status == AUCTION_LENGTH - 1 )
  {
    mprintf(sizeof(buf), buf, "%s - going twice at %ld gold.\n\r",
        auction_info.item->short_descr,
        auction_info.current_bid );
    auction_channel( buf );
    return;
  }

  if ( auction_info.status == AUCTION_LENGTH - 2 )
  {
    if ( auction_info.current_bid == 0 )
    {
      mprintf(sizeof(buf), buf, "No bids on %s - item removed.\n\r",
          auction_info.item->short_descr);
      auction_channel( buf );

      obj_to_char( auction_info.item, auction_info.owner );

      printf_to_char(auction_info.owner, "%s is returned to you.\n\r",
          auction_info.item->short_descr );
    
      clean_auction();
      return;
    }

    mprintf(sizeof(buf), buf, "%s - going once at %ld gold.\n\r",
        auction_info.item->short_descr,
        auction_info.current_bid );
    auction_channel( buf );
    return;
  }

  return;
}

void auction_channel( char * msg )
{
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;

  mprintf(sizeof(buf), buf, "{W[{yAuction{W]{Y %s{x", msg ); /* Add color if you wish */

  for ( d = descriptor_list; d != NULL; d = d->next )
  {
    CHAR_DATA *victim;

    victim = d->original ? d->original : d->character;

    if ( d->connected == CON_PLAYING
    && !IS_SET(victim->chan_flags,CHANNEL_NOAUCTION)
    && !IS_SET(victim->chan_flags,CHANNEL_QUIET) )
      send_to_char( buf, victim );
  }
  return;
}

/*
 * Show_obj_stats: code taken from stock identify spell (-Brian)
 */
void show_obj_stats( CHAR_DATA *ch, OBJ_DATA *obj )
{
  char buf[MAX_STRING_LENGTH];
  AFFECT_DATA *paf;
  bool found;

  show_ostat(ch, obj);

  if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
      if ( paf->location != APPLY_NONE && paf->modifier != 0 )
      {
        printf_to_char(ch, "Affects %s by %d.\n\r",  affect_loc_name( paf->location ), paf->modifier );
        if (paf->bitvector)
        {
          switch(paf->where)
          {
            case TO_AFFECTS:
              mprintf(sizeof(buf), buf,"Adds %s affect.\n",
                affect_bit_name(paf->bitvector));
              break;
            case TO_AFFECTS2:
              mprintf(sizeof(buf), buf,"Adds %s affect2.\n",
                affect2_bit_name(paf->bitvector));
              break;
            case TO_OBJECT:
              mprintf(sizeof(buf), buf,"Adds %s object flag.\n",
                extra_bit_name(paf->bitvector));
              break;
            case TO_IMMUNE:
              mprintf(sizeof(buf), buf,"Adds immunity to %s.\n",
                imm_bit_name(paf->bitvector));
              break;
            case TO_RESIST:
              mprintf(sizeof(buf), buf,"Adds resistance to %s.\n\r",
                imm_bit_name(paf->bitvector));
              break;
            case TO_VULN:
              mprintf(sizeof(buf), buf,"Adds vulnerability to %s.\n\r",
                imm_bit_name(paf->bitvector));
              break;
            default:
              mprintf(sizeof(buf), buf,"Unknown bit %d: %d\n\r",
                paf->where,paf->bitvector);
              break;
          }
          send_to_char( buf, ch );
        }
      }
    }

  for ( paf = obj->affected; paf != NULL; paf = paf->next )
  {
    if ( paf->location != APPLY_NONE && paf->modifier != 0 )
    {
      printf_to_char(ch, "Affects %s by %d", affect_loc_name( paf->location ), paf->modifier );
      if ( paf->duration > -1)
        printf_to_char(ch,", %d hours.\n\r",paf->duration);
      else
        printf_to_char(ch,".\n\r");

      if (paf->bitvector)
      {
        switch(paf->where)
        {
          case TO_AFFECTS:
            mprintf(sizeof(buf), buf,"Adds %s affect.\n",      affect_bit_name(paf->bitvector));
            break;
          case TO_AFFECTS2:
            mprintf(sizeof(buf), buf,"Adds %s affect2.\n", affect2_bit_name(paf->bitvector));
            break;
          case TO_OBJECT:
            mprintf(sizeof(buf), buf,"Adds %s object flag.\n",  extra_bit_name(paf->bitvector));
            break;
          case TO_WEAPON:
            mprintf(sizeof(buf), buf,"Adds %s weapon flags.\n",  weapon_bit_name(paf->bitvector));
            break;
          case TO_IMMUNE:
            mprintf(sizeof(buf), buf,"Adds immunity to %s.\n", imm_bit_name(paf->bitvector));
            break;
          case TO_RESIST:
            mprintf(sizeof(buf), buf,"Adds resistance to %s.\n\r", imm_bit_name(paf->bitvector));
            break;
          case TO_VULN:
            mprintf(sizeof(buf), buf,"Adds vulnerability to %s.\n\r", imm_bit_name(paf->bitvector));
            break;
          default:
            mprintf(sizeof(buf), buf,"Unknown bit %d: %d\n\r", paf->where,paf->bitvector);
            break;
        }
        send_to_char(buf,ch);
      }
    }
  }

  found = FALSE;
  if ( ( obj->item_type == ITEM_WEAPON )
  &&   ( ch->level >= LEVEL_REVEAL_AC ) )
  { // show affect times
    send_to_char("{c---------------------------------------------------------------\n\r",ch);
    send_to_char("Lvl ApplyTo              Mod   Hrs Special Affects\n\r",ch);
    send_to_char("---------------------------------------------------------------{x\n\r",ch);

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
      if ( (paf->bitvector)
      &&   (paf->where == TO_WEAPON ) )
      {
        if (!((!strcmp(skill_table[(int) paf->type].name, "flame blade")
        ||     !strcmp(skill_table[(int) paf->type].name, "frost blade")
        ||     !strcmp(skill_table[(int) paf->type].name, "drain blade")
        ||     !strcmp(skill_table[(int) paf->type].name, "sharp blade")
        ||     !strcmp(skill_table[(int) paf->type].name, "vorpal blade")
        ||     !strcmp(skill_table[(int) paf->type].name, "shocking blade")
        ||     !strcmp(skill_table[(int) paf->type].name, "resilience blade") )
          &&  ( ch->gameclass != cConjurer )
          &&  ( ch->gameclass != cWarlock ) ) )
        { // only Conjurers and Warlocks can see these particular affects for some reason
          found = TRUE;
          printf_to_char(ch,"{W%3d {g[{W%-16s{g]{W %5d %5d  {cMakes weapon {W%s{x{x\n\r",  
            paf->level, skill_table[(int) paf->type].name, // affect_loc_name( paf->location)
            paf->modifier, paf->duration,
            weapon_bit_name(paf->bitvector) );
        }
      }
    }

    if (!found)
      send_to_char("{WNo Affects.{x\n\r",ch);
    send_to_char("{c---------------------------------------------------------------{x\n\r",ch);
  }
  return;
}

void show_gocial(CHAR_DATA *ch, char *arg, int cmd, char *type, const int bitname, 
         enum special_flags spec_flag, const int channel, char color_code)
{
  char buf[MSL];
  char msg[MSL];
  bool display_to_char= FALSE;
  bool to_world = FALSE;
  CHAR_DATA *victim = NULL;
  DESCRIPTOR_DATA *d;

  if (arg[0] == '\0')
  {
    mprintf(sizeof(buf), buf, "%s %s{x", type, social_table[cmd].char_no_arg );
    replace_color(buf,color_code);
    act_channels(buf,ch,NULL,NULL,TO_CHAR,POS_DEAD,channel);
    mprintf(sizeof(buf), buf, "%s %s{x", type, social_table[cmd].others_no_arg );
    replace_color(buf,color_code);
  }
  else if ((victim = get_char_world(ch,arg)) == NULL)
  {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }
  else
  {
    if (IS_NPC(victim))
    {
      send_to_char("They aren't here.\n\r",ch);
      return;
    }   

    if (victim == ch)
    {
      mprintf(sizeof(buf), buf,"%s %s{x", type, social_table[cmd].char_auto);
      replace_color(buf,color_code);
      act_channels(buf,ch,NULL,NULL,TO_CHAR,POS_DEAD,channel);

      /* Die spammer, die! */
      if ( ch->desc && ch->desc->repeat > 1 )
        return;

      mprintf(sizeof(buf), buf,"%s %s{x", type, social_table[cmd].others_auto);
      replace_color(buf,color_code);
    }                
    else
    {
      if ( (channel == CHAN_GROUPTELL)
      &&   !is_same_group( ch, victim ) )
      { 
        send_to_char("They aren't in your group.\n\r",ch);
        return;
      }
      else
      {
        mprintf(sizeof(buf), buf,"%s %s{x", type,social_table[cmd].char_found);
        replace_color(buf,color_code);
        act_channels(buf,ch,NULL,victim,TO_CHAR,POS_DEAD,channel);

        /* Die spammer, die! */
        if ( ch->desc && ch->desc->repeat > 1 )
          return;

        if (victim->desc != NULL)
        {
          if (check_channel_view(ch,victim->desc, bitname, spec_flag))
          {
            mprintf(sizeof(buf), buf,"%s %s{x", type,social_table[cmd].vict_found);
            replace_color(buf,color_code);
            act_channels(buf,ch,NULL,victim,TO_VICT,POS_DEAD,channel);
          }
        }
        mprintf(sizeof(buf), buf,"%s %s{x", type,social_table[cmd].others_found);
        replace_color(buf,color_code);
        to_world = TRUE;
      }
    }
  }

  /* Die spammer, die! */
  if ( ch->desc && ch->desc->repeat > 1 )
    return;
  
  for (d = descriptor_list; d != NULL; d = d->next)
  {
    CHAR_DATA *vch;
    display_to_char = check_channel_view(ch,d,bitname, spec_flag);
    vch = d->original ? d->original : d->character;

    if ( (channel == CHAN_GROUPTELL)
    &&   !is_same_group( ch, vch ) )
      display_to_char = FALSE;

    if (display_to_char)
    {
      if (!to_world)
      {
        /*act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);*/
        act_channels(buf,ch,NULL,vch,TO_VICT,POS_DEAD,channel);
      }
      else
      {
        format_act( msg, buf, ch, vch, NULL,victim );
        if ((ch != vch) && (vch != victim))
        {
          send_to_char(msg, vch );

          update_chan_hist(vch, channel, msg);
        }
      }
    }
  }
}

/*
 * Locks replies to an invidual (or no one).
 */
void do_reply_lock( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];

  one_argument( argument, arg );

  if ( arg[0] != '\0' )
  {
    CHAR_DATA *vch;

    if ( !str_cmp( arg, "none" ) )
    {
      ch->reply = NULL;
      REMOVE_BIT( ch->comm_flags, COMM_REPLY_LOCK );
      send_to_char( "Replies are locked to no one.\n\r", ch );
      return;
    }

    for ( vch = char_list; vch; vch = vch->next )
      if ( (vch != NULL)
      &&   is_name( arg, vch->name )
      &&   can_see( ch, vch ) 
      &&   !IS_NPC(vch))
        break;

    if ( vch == NULL )
    {
      send_to_char( "No one of that name is available.\n\r", ch );
      return;
    }

    if ( ch == vch )
    {
      send_to_char( "You must really love yourself.\n\r", ch );
      return;
    }
    if (!IS_SET(ch->comm_flags, COMM_REPLY_LOCK))
    {
      SET_BIT( ch->comm_flags, COMM_REPLY_LOCK );
      ch->reply = vch;
    }

    printf_to_char( ch, "Replies are now locked to %s.\n\r", vch->name );
    return;
  }

  if ( IS_SET( ch->comm_flags, COMM_REPLY_LOCK ) )
  {
    send_to_char( "Replies are now unlocked.\n\r", ch );
    ch->reply = NULL;
    REMOVE_BIT( ch->comm_flags, COMM_REPLY_LOCK );
  }
  else
  {
    if ( ch->reply == NULL )
      send_to_char( "Replies are locked to no one.\n\r", ch );
    else
    {
      act_new( "Replies are locked to $N.",
         ch, NULL, ch->reply, TO_CHAR, POS_SLEEPING );
      SET_BIT( ch->comm_flags, COMM_REPLY_LOCK );
    }
  }
}

/*
 * Breaks all replies to an invidual.
 */
void do_reply_break( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *vch;
  bool found = FALSE;

  for ( vch = char_list; vch != NULL; vch = vch->next )
  {
    if ( vch->reply == ch && (vch != NULL) )
    {
      REMOVE_BIT( vch->comm_flags, COMM_REPLY_LOCK );
      vch->reply = NULL;
      found = TRUE;
    }
  }

  if ( found )
    send_to_char( "All replies to you are broken.\n\r", ch );
  else
    send_to_char( "There are no replies to you.\n\r", ch );
}

void do_forget(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *rch;
  char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  int pos;
  bool found = FALSE;

  if (ch->desc == NULL)
    rch = ch;
  else
    rch = ch->desc->original ? ch->desc->original : ch;

  if (IS_NPC(rch))
    return;

  smash_tilde( argument );

  argument = one_argument(argument,arg);
    
  if (arg[0] == '\0')
  {
    if (rch->pcdata->forget[0] == NULL)
    {
      send_to_char("You are not forgetting anyone.\n\r",ch);
      return;
    }
    send_to_char("You are currently forgetting:\n\r",ch);

    for (pos = 0; pos < MAX_FORGET; pos++)
    {
      if (rch->pcdata->forget[pos] == NULL)
        break;
      printf_to_char(ch,"    %s\n\r",rch->pcdata->forget[pos]);
    }
    return;
  }

  for (pos = 0; pos < MAX_FORGET; pos++)
  {
    if (rch->pcdata->forget[pos] == NULL)
      break;

    if (!str_cmp(capitalize(arg),rch->pcdata->forget[pos]))
    {
      send_to_char("You have already forgotten that person.\n\r",ch);
      return;
    }
  }

  for (d = descriptor_list; d != NULL; d = d->next)
  {
    CHAR_DATA *wch;

    if (d->connected != CON_PLAYING || !can_see(ch,d->character))
      continue;
    
    wch = ( d->original != NULL ) ? d->original : d->character;

    if (!can_see(ch,wch))
      continue;

    if (!str_cmp(arg,wch->name))
    {
      found = TRUE;
      if (wch == ch)
      {
        send_to_char("You forget yourself for a moment, but it passes.\n\r",ch);
        return;
      }

      if (wch->level >= LEVEL_IMMORTAL)
      {
        send_to_char("That person is very hard to forget.\n\r",ch);
        return;
      }
    }
  }

  if (!found)
  {
    send_to_char("No one by that name is playing.\n\r",ch);
    return;
  }

  for (pos = 0; pos < MAX_FORGET; pos++)
  {
    if (rch->pcdata->forget[pos] == NULL)
      break;
  }

  if (pos >= MAX_FORGET)
  {
    send_to_char("Sorry, you have reached the forget limit.\n\r",ch);
    return;
  }
  
  /* make a new forget */
  rch->pcdata->forget[pos]        = str_dup(capitalize(arg), rch->pcdata->forget[pos]);
  mprintf(sizeof(buf), buf,"You are now deaf to %s.\n\r",arg);
  send_to_char(buf,ch);
}

void do_remember(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *rch;
  char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  int pos;
  bool found = FALSE;
 
  if (ch->desc == NULL)
    rch = ch;
  else
    rch = ch->desc->original ? ch->desc->original : ch;
 
  if (IS_NPC(rch))
    return;
 
  argument = one_argument(argument,arg);

  if (arg[0] == '\0')
  {
    if (rch->pcdata->forget[0] == NULL)
    {
      send_to_char("You are not forgetting anyone.\n\r",ch);
      return;
    }
    send_to_char("You are currently forgetting:\n\r",ch);

    for (pos = 0; pos < MAX_FORGET; pos++)
    {
      if (rch->pcdata->forget[pos] == NULL)
        continue;

      mprintf(sizeof(buf), buf,"    %s\n\r",rch->pcdata->forget[pos]);
      send_to_char(buf,ch);
    }
    return;
  }

  for (pos = 0; pos < MAX_FORGET; pos++)
  {
    if (rch->pcdata->forget[pos] == NULL)
      continue;

    if (found)
    {
      rch->pcdata->forget[pos-1]        = rch->pcdata->forget[pos];
      rch->pcdata->forget[pos]        = NULL;
      continue;
    }

    if(!strcmp(capitalize(arg),rch->pcdata->forget[pos]))
    {
      send_to_char("Forget removed.\n\r",ch);
      free_string(rch->pcdata->forget[pos]);
      rch->pcdata->forget[pos] = NULL;
      found = TRUE;
    }
  }

  if (!found)
    send_to_char("No one by that name is forgotten.\n\r",ch);
}

bool check_channel_view(CHAR_DATA *ch, DESCRIPTOR_DATA *d, const int bitname,
            enum special_flags spec_flag)
{
  bool display_to_char = TRUE;
  int pos=0;
  CHAR_DATA *vch;
  vch = d->original ? d->original : d->character;

  if (vch == NULL)
    return (FALSE);

  if (!IS_NPC(vch)) {
    for (pos = 0; pos < MAX_FORGET; pos++)
    {
      if (vch->pcdata->forget[pos] == NULL)
        continue;
      if (!str_cmp(ch->name,vch->pcdata->forget[pos]))
        display_to_char = FALSE; 
    }
  }

  if (IS_IMMORTAL(vch) 
  && (d->connected == CON_PLAYING)
  && IS_SET(vch->chan_flags, CHANNEL_ALL))
    return TRUE;

  if (d->original)
    return TRUE;
  
  if (d->connected == CON_PLAYING
  && d->character != ch
  && !IS_SET(vch->chan_flags,bitname)
  && (!IS_SET(vch->chan_flags,CHANNEL_QUIET)
  || (spec_flag == spec_imm_flag)))
  {
    switch (spec_flag)
    {
      case spec_clan_flag:
        if (ch->clan !=vch->clan)
          display_to_char = FALSE;
        break;
      case spec_allclan_flag:
        if (!is_clan(vch))
          display_to_char = FALSE;
        break;
      case spec_public_flag:
        break;
      case spec_imm_flag:
        if (!IS_IMMORTAL(vch))
          display_to_char = FALSE;
        break;
    }
  }
  else
    display_to_char = FALSE;

  return (display_to_char);
}

#define MAX_TALK_CHEAT 2
#define MAX_CURSE_STRING 7
struct string_st {
  char str[MSL];
};

struct string_st curse_string[] =
{
  {"asshole"},
  {"pussy"},
  {"shit"},
  {"fag"},
  {"cunt"},
  {"fuck"},
  {"prick"},
  {"cock"},
  {"c*ck"},
  {"phuck"},
  {"f***"},
  {"f*ck"},
  {"fvck"},
  {"sh*t"},
  {"s***"},
  {"dick"},
  {"d*ck"},
  {"dyke"},
  {"bitch"},
  {"b*tch"},
  {"b****"},
  {"assh*le"},
  {"fugger"},
  {"f*gger"},
  {"fck"},
  {"fuk"}
};

struct string_st talk_cheat[] =
{
  {"bug"},
  {"cheat"},
};

bool has_curse(char *s, int bitname)
{
  int i;
  char str[MSL];
 
  if (bitname == CHANNEL_NOWIZ)
    return FALSE;

  strcpy(str,s);
  strip_color(str,lowerize(s));

  for (i=0; i < MAX_CURSE_STRING; i++)
  {
    if (strstr(str,curse_string[i].str))
      return TRUE;
  }

  return FALSE;
}

char *lowerize( const char *str )
{
  static char strcap[MAX_STRING_LENGTH];
  int i;

  for ( i = 0; str[i] != '\0'; i++ )
    strcap[i] = LOWER(str[i]);

  strcap[i] = '\0';
  return strcap;
}



void check_4_talk_cheating(CHAR_DATA *ch, char *s)
{
  int i;
  char str[MSL];
  strcpy(str,s);
  strip_color(str,lowerize(str));
  for (i=0; i < MAX_TALK_CHEAT; i++)
  {
    if (strstr(str,talk_cheat[i].str))
      logit("POS CHEAT: [%s] %s",ch->name,str);
  }
}

void do_cls (CHAR_DATA *ch, char *argument)
{
  send_to_char ("\x01B[2J", ch);
}
