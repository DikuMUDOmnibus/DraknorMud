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
*    ROM 2.4 is copyright 1993-1996 Russ Taylor               *
*    ROM has been brought to you by the ROM consortium           *
*        Russ Taylor (rtaylor@efn.org)                   *
*        Gabrielle Taylor                           *
*        Brian Moore (zump@rom.org)                       *
*    By using this code, you have agreed to follow the terms of the       *
*    ROM license, in the file Rom24/doc/rom.license               *
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
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"
#include "clan.h"

#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif
extern char *ltof( long flags );
#define PFILE_VERSION 9
int rename(const char *oldfname, const char *newfname);
void clear_combined_save_obj( OBJ_DATA *obj );

char *print_flags(int64 flag)
{
  int count, pos = 0;
  static char buf[53];


  for (count = 0; count < 52;  count++)
  {
    if (IS_SET(flag,1<<count))
    {
      if (count < 26)
        buf[pos] = 'A' + count;
      else
        buf[pos] = 'a' + (count - 26);
      pos++;
    }
  }

  if (pos == 0)
  {
    buf[pos] = '0';
    pos++;
  }

  buf[pos] = '\0';

  return buf;
}


/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST    100
static    OBJ_DATA *    rgObjNest    [MAX_NEST];




/* From note.c */
void    note_attach    args( ( CHAR_DATA *ch, int type ) );


/* From olc_save.c */
char    *fix_string    args( ( const char *str ) );

#if AR
char *flag_string_save( const FLAG_TYPE *flag_table, int bits );
#endif

/*
 * Local functions.
 */
void    fwrite_char    args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fwrite_obj    args( ( CHAR_DATA *ch,  OBJ_DATA  *iobj, OBJ_DATA *obj,
                              FILE *fp, int iNest, bool isLocker ) );
void    fwrite_pet    args( ( CHAR_DATA *pet, FILE *fp) );
void    fread_char    args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fread_pet    args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fread_obj    args( ( CHAR_DATA *ch,  FILE *fp,bool isLocker, bool fCombine ) );



/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch , bool immoverride)
{
  char strsave[MAX_INPUT_LENGTH];
  FILE *fp;
  int v1;

  if (ch == NULL)
    return;

  if ( IS_NPC(ch) )
    return;

  v1 = ( ch->played + (int) (current_time - ch->logon) ) / 3600;
  if (!immoverride && ch->level <= 1 && v1 < 1)
    return;

  if ( ch->desc != NULL && ch->desc->original != NULL )
    ch = ch->desc->original;
#if MEMDEBUG
  memdebug_check(ch, "Save: Save char_obj");
#endif
#if defined(unix)
  /* create god log */
  if (IS_IMMORTAL(ch) || ch->level >= LEVEL_IMMORTAL)
  {
    fclose(fpReserve);
    nFilesOpen--;
    mprintf(sizeof(strsave),strsave, "%s%s",GOD_DIR, capitalize(ch->name));
    if ((fp = fopen(strsave,"w")) == NULL)
    {
      bug("Save_char_obj: fopen",0);
      perror(strsave);
      return;
    }

    nFilesOpen++;
    /* HMMMM */
    fprintf(fp,"Lev %2d Trust %2d  %s%s\n",
            ch->level, get_trust(ch), ch->name, ch->pcdata->title);
    fclose( fp );
    nFilesOpen--;
    fpReserve = fopen( NULL_FILE, "r" );
    nFilesOpen++;
  }
#endif
  if (fpReserve)
  {
    fclose( fpReserve );
    nFilesOpen--;
  }
  mprintf( sizeof(strsave),strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
  if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
  {
    bug( "Save_char_obj: fopen", 0 );
    perror( strsave );
  }
  else
  {
    nFilesOpen++;
    /* Save clannie info for roster.. and normalize it.. */
    if ( ch->clan )
      copy_roster_clannie( ch );
    fwrite_char( ch, fp );
#if MEMDEBUG
    memdebug_check(ch, "Save: Save char_obj");
#endif
    if ( ch->carrying )
    {
      clear_combined_save_obj(ch->carrying);
      fwrite_obj( ch, ch->carrying, ch->carrying, fp, 0, 0 );
      clear_combined_save_obj(ch->carrying);
    }

    if ( ch->pcdata->locker )
    {
      clear_combined_save_obj(ch->pcdata->locker);
      fwrite_obj( ch, ch->pcdata->locker, ch->pcdata->locker, fp, 0, 1 );
      clear_combined_save_obj(ch->pcdata->locker);
    }

    if ( ch->pet != NULL )
      fwrite_pet(ch->pet,fp);

    fprintf( fp, "#END\n" );
  }
  fclose( fp );
  nFilesOpen--;

#if MEMDEBUG
  memdebug_check(ch, "Save: Save char_obj");
#endif
  rename(TEMP_FILE,strsave);
  fpReserve = fopen( NULL_FILE, "r" );
  nFilesOpen++;
  return;
}


/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
  AFFECT_DATA *paf;
  int sn, gn, mn, mno, pos;
  bool roomfound = FALSE;

  if (IS_NPC(ch))
  {
    send_to_char("Who are you to be trying to save?\n\r",ch);
    bugf("SAVE:Trying to save a NON PC");
    return;
  }

  if (ch->in_room == NULL)
  {
    send_to_char("WHAT? you are not in a room?\n\r",ch);
    bugf("SAVE:Trying to save a char in a NULL room");
    return;
  }

  if (ch->pcdata == NULL)
  {
    send_to_char("WHAT? you are not a Player!!\n\r",ch);
    bugf("SAVE: Trying to save a char without PCDATA");
    return;
  }

  fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"    );
  fprintf( fp, "Vers %d\n",   PFILE_VERSION            );
  fprintf( fp, "Name %s~\n",    capitalize(ch->name)    );
  fprintf( fp, "Pass %s~\n",    ch->pcdata->pwd        );
  fprintf( fp, "Levl %d\n",    ch->level        );
  fprintf( fp, "Id   %ld\n", ch->id            );
  if (ch->desc != NULL)
    fprintf( fp, "Host %s~\n",      ch->desc->host    ); /* TML */
  else
  {
    if (ch->pcdata)
      if (ch->pcdata->host)
        fprintf( fp, "Host %s~\n",      ch->pcdata->host    ); /* TML */

  }
  if (IS_SET(ch->active_flags, ACTIVE_PLOAD))
    fprintf( fp, "LogO %ld\n",ch->pcdata->laston);
  else
    fprintf( fp, "LogO %ld\n",    current_time);

  if (ch->bailed)
    fprintf( fp, "BailTime %d\n", ch->bailed);

  if (ch->num_bailed)
    fprintf(fp, "NumBail %d\n", ch->num_bailed);

  if (ch->short_descr[0] != '\0')
    fprintf( fp, "ShD  %s~\n",    ch->short_descr    );

  if ( ch->long_descr[0] != '\0')
    fprintf( fp, "LnD  %s~\n",    ch->long_descr    );
  if (ch->description[0] != '\0')
    fprintf( fp, "Desc %s~\n",    ch->description    );
  if (ch->pcdata->history[0] != '\0')
    fprintf( fp, "Hist %s~\n",    ch->pcdata->history    );
  fprintf( fp, "Race %s~\n", pc_race_table[ch->race].name );
  fprintf( fp, "Sex  %d\n",    ch->sex            );
  fprintf( fp, "SexDir  %d\n",    ch->sex_dir        ); /* TML */
#if MEMDEBUG
  memdebug_check(ch, "fwrite: 1");
#endif
  fprintf( fp, "Vtimer  %d\n",   ch->vflag_timer);    /* TML */
  fprintf( fp, "Cla  %d\n",    ch->gameclass        );
  if (ch->trust != 0)
    fprintf( fp, "Tru  %d\n",    ch->trust    );
  if (ch->pcdata->denytime)
    fprintf(fp, "Denytime %d\n\r", ch->pcdata->denytime); /* TML */
  if (ch->pcdata->denied_time)
    fprintf(fp, "Dtime %d\n\r", ch->pcdata->denied_time); /* TML */
  fprintf( fp, "Not  %ld %ld %ld %ld %ld %ld %ld\n",
           ch->pcdata->last_note,
           ch->pcdata->last_idea,
           ch->pcdata->last_penalty,
           ch->pcdata->last_news,
           ch->pcdata->last_changes,
           ch->pcdata->last_rules,
           ch->pcdata->last_rpnote); /* TML */
  fprintf( fp, "Scro %d\n",     ch->lines        );
  fprintf( fp, "Room %d\n",
           (  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
              && ch->was_in_room != NULL )
           ? ch->was_in_room->vnum
           : ch->in_room == NULL ? ROOM_VNUM_TEMPLE : ch->in_room->vnum );

  fprintf( fp, "HMV  %d %d %d %d %d %d\n",
           ch->hit,
           ch->max_hit,
           ch->mana,
           ch->max_mana,
           ch->move,
           ch->max_move );

  fprintf( fp, "Fragments %d %d %d %d\n",
           ch->ruby_fragment,
           ch->sapphire_fragment,
           ch->emerald_fragment,
           ch->diamond_fragment );

  fprintf( fp, "FragCounter %d %d %d %d\n",
           ch->ruby_counter,
           ch->sapphire_counter,
           ch->emerald_counter,
           ch->diamond_counter );

  fprintf( fp, "FragPurchases %d %d %d %d %d\n",
           ch->first_frag_level,
           ch->second_frag_level,
           ch->third_frag_level,
           ch->fourth_frag_level,
           ch->fifth_frag_level );

  fprintf( fp, "Dths %d\n",  ch->pcdata->deaths);
  fprintf( fp, "Kills %d\n", ch->pcdata->kills);
  fprintf( fp, "Pkills %d\n",ch->pcdata->pkills );
  fprintf( fp, "Pdths %d\n", ch->pcdata->pdeaths );
  fprintf( fp, "fish_caught %d\n", ch->pcdata->fish_caught );
  fprintf( fp, "fish_lost %d\n", ch->pcdata->fish_lost );
  fprintf( fp, "fish_broken %d\n", ch->pcdata->fish_broken );
  fprintf( fp, "QBits %llu\n", ch->pcdata->mpquests );
  fprintf( fp, "Bloodshards %d\n", ch->pcdata->bloodshards );

  if ( ch->pcdata->home_vnum > 0 )
    fprintf( fp, "Home %d\n", ch->pcdata->home_vnum );

  fprintf( fp, "Locker_Max %d\n",ch->pcdata->locker_max );
  if ( ch->clan )
  {
    fprintf( fp, "Clanname %s~\n",  ch->clan->name );
    fprintf( fp, "Rank %d\n",  ch->pcdata->clan_rank); /* Clans */
    fprintf( fp, "Dias %d\n",  ch->pcdata->donated_dia); /* Clans */

    if (ch->pcdata->clanowe_dia)
      fprintf( fp, "ClanOweDia %d\n",  ch->pcdata->clanowe_dia); /* Clans */
    if (ch->pcdata->clanowe_level)
      fprintf( fp, "ClanOweLevel %d\n",  ch->pcdata->clanowe_level); /* Clans */
    if (ch->pcdata->clanowe_clan)
      fprintf( fp, "ClanOweClan %s~\n",  ch->pcdata->clanowe_clan); /* Clans */
    if (ch->pcdata->numclans)
      fprintf( fp, "NumClans %d\n",  ch->pcdata->numclans); /* Clans */
  }
  if (ch->gold > 0)
    fprintf( fp, "Gold %ld\n",    ch->gold        );
  else
    fprintf( fp, "Gold %d\n", 0            );
  if (ch->pcdata->balance > 0)
    fprintf( fp, "Balance     %d\n",        ch->pcdata->balance );
  else
    fprintf( fp, "Balance 0\n");
  if (ch->pcdata->shares > 0)
    fprintf( fp, "Shares      %d\n",        ch->pcdata->shares  );
  else
    fprintf( fp, "Shares 0\n");
  if (ch->silver > 0)
    fprintf( fp, "Silv %ld\n",ch->silver        );
  else
    fprintf( fp, "Silv %d\n",0            );
  fprintf( fp, "Exp  %llu\n",    ch->exp            );
#if MEMDEBUG
  memdebug_check(ch, "fwrite: 2");
#endif
  if (ch->act)
    fprintf( fp, "Act  %s\n",   print_flags(ch->act));
  if (ch->affected_by)
    fprintf( fp, "AfBy %s\n",   print_flags(ch->affected_by));
  if (ch->affected2_by)
    fprintf( fp, "AfBy2 %s\n",  print_flags(ch->affected2_by));
  if (ch->spell_aff)
    fprintf( fp, "SpAfBy %s\n", print_flags(ch->spell_aff)); /* TML */
  if ( ch->comm_flags )
  {
    if ( ch->reply )
      fprintf( fp, "Comm %s\n",    print_flags( ch->comm_flags &~ COMM_REPLY_LOCK ) );
    else
      fprintf( fp, "Comm %s\n",    print_flags(ch->comm_flags)    );
  }
  if ( ch->chan_flags )
    fprintf( fp, "Chan %s\n",    print_flags(ch->chan_flags)    );
  if ( ch->pen_flags )
    fprintf( fp, "Pen %s\n",    print_flags(ch->pen_flags)    );
  if ( ch->nospam )
    fprintf( fp, "Nospam %s\n", print_flags(ch->nospam ) );
  if ( ch->plr2 )
  {
    fprintf( fp, "Plr2 %s\n",    print_flags( ch->plr2) );
  }
  if (ch->wiznet)
    fprintf( fp, "Wizn %s\n",   print_flags(ch->wiznet));
  if (ch->invis_level)
    fprintf( fp, "Invi %d\n",     ch->invis_level    );
  if (ch->incog_level)
    fprintf(fp,"Inco %d\n",ch->incog_level);
  if (ch->martial_style != 0)
    fprintf(fp, "Style %d\n",   ch->martial_style); /* TML */
  fprintf( fp, "Pos  %d\n",
           ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
  if (ch->practice)
    fprintf( fp, "Prac %d\n",    ch->practice    );
  if (ch->train)
    fprintf( fp, "Trai %d\n",    ch->train    );
  if (ch->saving_throw)
    fprintf( fp, "Save  %d\n",    ch->saving_throw);
  fprintf( fp, "Alig  %d\n",    ch->alignment        );
  if (ch->hitroll)
    fprintf( fp, "Hit   %d\n",    ch->hitroll    );
  if (ch->damroll)
    fprintf( fp, "Dam   %d\n",    ch->damroll    );
  fprintf( fp, "ACs %d %d %d %d\n",
           ch->armor[0],
           ch->armor[1],
           ch->armor[2],
           ch->armor[3]);
  if (ch->wimpy)
    fprintf( fp, "Wimp  %d\n",    ch->wimpy    );
  fprintf( fp, "Attr %d %d %d %d %d\n",
           ch->perm_stat[STAT_STR],
           ch->perm_stat[STAT_INT],
           ch->perm_stat[STAT_WIS],
           ch->perm_stat[STAT_DEX],
           ch->perm_stat[STAT_CON] );

  fprintf (fp, "AMod %d %d %d %d %d\n",
           ch->mod_stat[STAT_STR],
           ch->mod_stat[STAT_INT],
           ch->mod_stat[STAT_WIS],
           ch->mod_stat[STAT_DEX],
           ch->mod_stat[STAT_CON] );

  if (!IS_NPC( ch ))
  {
    if (ch->pcdata->afktime)
      fprintf( fp, "AFKTimer %d\n",  ch->pcdata->afktime);

    if ( ch->pcdata->wlbugs )
      fprintf ( fp, "WLBugs %d\n",  ch->pcdata->wlbugs );
    if ( ch->pcdata->wlduhs )
      fprintf ( fp, "WLDuhs %d\n",  ch->pcdata->wlduhs );
    if ( ch->pcdata->wltypos )
      fprintf ( fp, "WLTypos %d\n", ch->pcdata->wltypos );
    if ( ch->pcdata->wlbuild )
      fprintf ( fp, "WLBuild %d\n", ch->pcdata->wlbuild );
    if ( ch->pcdata->wlhelps )
      fprintf ( fp, "WLHelps %d\n", ch->pcdata->wlhelps );

    if ( ch->pcdata->fdbugs )
      fprintf ( fp, "FdBugs %d\n",  ch->pcdata->fdbugs );
    if ( ch->pcdata->fdtypos )
      fprintf ( fp, "FdTypos %d\n", ch->pcdata->fdtypos );
    if ( ch->pcdata->fdhelps )
      fprintf ( fp, "FdHelps %d\n", ch->pcdata->fdhelps );
    if ( ch->pcdata->fdduhs )
      fprintf ( fp, "FdDuhs %d\n", ch->pcdata->fdduhs );

    if ( ch->pcdata->eye_color )
      fprintf ( fp, "PhyEyes %s~\n", ch->pcdata->eye_color );
    if ( ch->pcdata->hair_color )
      fprintf ( fp, "PhyHair %s~\n", ch->pcdata->hair_color );
    if ( ch->pcdata->skin_color )
      fprintf ( fp, "PhySkin %s~\n", ch->pcdata->skin_color );
    if ( ch->pcdata->height )
      fprintf ( fp, "PhyHeight %d\n", ch->pcdata->height );
    if ( ch->pcdata->weight )
      fprintf ( fp, "PhyWeight %d\n", ch->pcdata->weight );
  }

  if ( IS_NPC(ch) )
  {
    fprintf( fp, "Vnum %d\n",    ch->pIndexData->vnum    );
  }
  else
  {
    /* Voltecs Player Birthdays */
    fprintf(fp, "BDay %d %d %d %d\n",
            ch->birthday.hour,
            ch->birthday.day,
            ch->birthday.month,
            ch->birthday.year);

    if (IS_CHARMED(ch) && !IS_NPC(ch->master))
      fprintf( fp, "CharmMaster  %s~\n",    ch->master->name                );
    fprintf( fp, "Sec  %d\n",             ch->pcdata->security            );    /* OLC */
    fprintf( fp, "SecFlag %s\n",            ltof( ch->pcdata->security_flags ) );    /* OLC */
    fprintf( fp, "Quest_Curr    %d\n",    ch->questdata->curr_points      );
    fprintf( fp, "Quest_Accum  %d\n",     ch->questdata->accum_points     );
    fprintf( fp, "Quest_Num  %d\n",       ch->questdata->quest_number     );
    fprintf( fp, "QuestStreak  %d\n",     ch->questdata->streak           );
    fprintf( fp, "QuestStreakNeed %d\n",  ch->questdata->current_streak   );
    fprintf( fp, "QuestStreakBest %d\n",  ch->questdata->best_streak      );
    fprintf( fp, "Quest_Att    %d\n",     ch->questdata->attempt_num      );
    fprintf( fp, "Quest_Quit   %d\n",     ch->questdata->quit_num         );
    fprintf( fp, "Quest_Comp   %d\n",     ch->questdata->comp_num         );
    fprintf( fp, "Quest_Glory  %d\n",     ch->questdata->glory            );
    fprintf( fp, "Quest_Failed %d\n",     ch->questdata->failed           );

#if MEMDEBUG
    memdebug_check(ch, "fwrite: 3");
#endif
    if (IS_SET(ch->act,PLR_QUESTING))
    {
      fprintf(fp, "Quest 1\n");
      fprintf(fp, "QuestCount   %d\n",  ch->questdata->countdown  );
      fprintf(fp, "QuestAllow   %d\n",  ch->questdata->time_allowed  );
      if (ch->questdata->questgiver)
        fprintf(fp, "QuestGiver  %s~\n",ch->questdata->questgiver->short_descr);
      if (ch->questdata->obj_vnum)
      {
        // save obj quest info (to save through a copyover)
        fprintf( fp, "QuestObj  %d\n", ch->questdata->obj_vnum);
        OBJ_DATA *qobj;
        for ( qobj = object_list; qobj; qobj = qobj->next )
        {
          if ( ( ch->questdata->obj_vnum == qobj->pIndexData->vnum )
               &&     is_exact_name( ch->name, qobj->owner ) )
          {
            fprintf( fp, "QuestName  %s~\n",  qobj->name);
            fprintf( fp, "QuestShort  %s~\n", qobj->short_descr);
            fprintf( fp, "QuestLong  %s~\n",  qobj->description);
            break;
          }
        }
      }
      if (ch->questdata->mob_vnum)
      {
        // rescue quest info (to save through a copyover)
        fprintf( fp, "QuestMob  %d\n", ch->questdata->mob_vnum);
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
              fprintf( fp, "QuestName  %s~\n",  qch->name);
              fprintf( fp, "QuestShort  %s~\n", qch->short_descr);
              fprintf( fp, "QuestLong  %s~\n",  qch->long_descr);
              fprintf( fp, "QuestRoom  %d\n",   qch->in_room->vnum);
              roomfound = TRUE;
              break;
            }
          }
        }
      }

      if ( !roomfound && ch->questdata->room_vnum )
        fprintf( fp, "QuestRoom  %d\n", ch->questdata->room_vnum);

      fprintf( fp,    "QuestText\n%s~\n",  fix_string( buf_string(ch->questdata->log) ) );

    }
    else
    {
      fprintf(fp, "Quest 0\n");
      if (ch->questdata->nextquest)
        fprintf( fp, "QuestNext   %d\n",  ch->questdata->nextquest   );
      if (ch->questdata->countdown)
        fprintf( fp, "QuestCount   %d\n",  10              );
#if MEMDEBUG
      memdebug_check(ch, "fwrite: 4");
#endif
    }
    fprintf( fp, "Degraded %d\n", ch->pcdata->degrading); /* TML */
    if (IS_SET(ch->active_flags, ACTIVE_PLOAD))
    {
      fprintf( fp, "Plyd %d\n", ch->played);
    }
    else
    {
      if (ch->idle_snapshot != 0)
      {
        ch->idle_time += current_time - ch->idle_snapshot;
        ch->idle_snapshot = 0;
      }
      fprintf( fp, "Plyd %d\n", TOTAL_PLAY_TIME(ch) );
    }
    fprintf( fp, "LstLevelPlyd %d\n", ch->last_level);

    if (ch->pcdata->old_char)
      fprintf(fp, "Deadchar 1\n");
    if (ch->prompt != NULL || !str_cmp(ch->prompt,"{R%h{w/{r%H {B%m{w/{b%M {G%v{w/{g%V{w> "))
      fprintf( fp, "Prom %s~\n",      ch->prompt      );
    if (ch->pcdata->bamfin[0] != '\0')
      fprintf( fp, "Bin  %s~\n",    ch->pcdata->bamfin);
    if (ch->pcdata->bamfout[0] != '\0')
      fprintf( fp, "Bout %s~\n",    ch->pcdata->bamfout);

    fprintf( fp, "Titl %s~\n",    ch->pcdata->title    );

    if ( ch->pcdata->afk_title )
      fprintf( fp, "Title_afk %s~\n", ch->pcdata->afk_title );

    if (ch->pcdata->permtitle[0] != '\0')
      fprintf( fp, "PermTitl %s~\n",    ch->pcdata->permtitle    );

    fprintf( fp, "TitSet %d\n",    ch->pcdata->user_set_title    );
    fprintf( fp, "Bounty %d\n",    ch->pcdata->bounty  );
    fprintf( fp, "Pnts %d\n",       ch->pcdata->points      );
    fprintf( fp, "TSex %d\n",    ch->pcdata->true_sex    );
    fprintf( fp, "LLev %d\n",    ch->pcdata->last_level    );
    fprintf( fp, "HMVP %d %d %d\n",
             //GET_HP(ch),
             ch->pcdata->perm_hit,
             ch->pcdata->perm_mana,
             ch->pcdata->perm_move);
    fprintf( fp, "Cnd  %d %d %d %d\n",
             ch->pcdata->condition[0],
             ch->pcdata->condition[1],
             ch->pcdata->condition[2],
             ch->pcdata->condition[3]);

    /* Buffered tells. */
    /* Yes, Stolen from AR  thanks Boreas*/
    if ( ch->tells > 0
         &&   bstrlen( ch->pcdata->buffer ) > 0 )
    {
      fprintf( fp, "BufCnt %d\n",    ch->tells    );
      fprintf( fp, "BufStr\n%s~\n",
               fix_string( buf_string( ch->pcdata->buffer ) ) );
    }

    /* Working on a note. */
    if ( ch->pnote && !IS_NULLSTR( ch->pnote->text ) )
    {
      NOTE_DATA *pNote = ch->pnote;

      fprintf( fp,     "NoteType   %d\n", pNote->type  );
      fprintf( fp,     "NoteTo     %s~\n", pNote->to_list );
      fprintf( fp,     "NoteSubj   %s~\n", pNote->subject );
      fprintf( fp,     "NoteText\n%s~\n",  fix_string( pNote->text ) );
    }
    /* write alias */
    for ( pos = 0; pos < MAX_ALIAS; pos++ )
    {
      if ( ch->pcdata->alias[pos] == NULL
           || ch->pcdata->alias_sub[pos] == NULL )
        break;

      /* Do not write an alias with a quote in it. */
      if ( strchr( ch->pcdata->alias[pos], '\"'/* " */ ) )
        continue;

      fprintf( fp, "Alias \"%s\" %s~\n",
               ch->pcdata->alias[pos],
               ch->pcdata->alias_sub[pos]    );
    }

    for ( mn = 0; mn < MAX_MAINTAINED; mn++ )
    {
      if (ch->pcdata->maintained[mn] > 0)
      {
        if ( skill_table[ch->pcdata->maintained[mn]].name != NULL )
        {
          fprintf( fp, "Mn '%s'\n",
                   skill_table[ch->pcdata->maintained[mn]].name );
        }
      }
    }

    for ( mno = 0; mno < MAX_MAINTAINED; mno++ )
    {
      if (ch->pcdata->maintained_other[mno] > 0)
      {
        if ( skill_table[ch->pcdata->maintained_other[mno]].name != NULL )
        {
          fprintf( fp, "Mno '%s'\n",
                   skill_table[ch->pcdata->maintained_other[mno]].name );
        }
      }
    }

    for ( mno = 0; mno < MAX_MAINTAINED; mno++ )
    {
      if (ch->pcdata->maintained_weapon[mno] > 0)
      {
        if ( skill_table[ch->pcdata->maintained_weapon[mno]].name != NULL )
        {
          fprintf( fp, "Mnw '%s'\n",
                   skill_table[ch->pcdata->maintained_weapon[mno]].name );
        }
      }
    }

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
      if ( skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0 )
      {
        fprintf( fp, "Sk %d '%s'\n",
                 ch->pcdata->learned[sn], skill_table[sn].name );
      }
    }

    for ( gn = 0; gn < MAX_GROUP; gn++ )
    {
      if ( group_table[gn].name != NULL && ch->pcdata->group_known[gn])
      {
        fprintf( fp, "Gr '%s'\n",group_table[gn].name);
      }
    }
  }
#if MEMDEBUG
  memdebug_check(ch, "fwrite: 5");
#endif

  for ( paf = ch->affected; paf != NULL; paf = paf->next )
  {
    if (paf->type < 0 || paf->type >= MAX_SKILL)
    {
      bugf("Affect_error:Affc '%s' %3d %3d %3d %3d %3d %llu\n",
           skill_table[paf->type].name,
           paf->where,
           paf->level,
           paf->duration,
           paf->modifier,
           paf->location,
           paf->bitvector
          );
      continue;
    }
#if MEMDEBUG
    memdebug_check(ch, "fwrite: 6");
#endif

    fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %llu\n",
             skill_table[paf->type].name,
             paf->where,
             paf->level,
             paf->duration,
             paf->modifier,
             paf->location,
             paf->bitvector
           );
#if MEMDEBUG
    memdebug_check(ch, "fwrite: 7");
#endif

  }

  fprintf( fp, "End\n\n" );
  return;
}

/* write a pet */
void fwrite_pet( CHAR_DATA *pet, FILE *fp)
{
  AFFECT_DATA *paf;

  fprintf(fp,"#PET\n");

  fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);
  fprintf(fp,"Name %s~\n", pet->name);
  fprintf(fp,"LogO %ld\n", current_time);

  if (pet->short_descr != pet->pIndexData->short_descr)
    fprintf(fp,"ShD  %s~\n", pet->short_descr);
  if (pet->long_descr != pet->pIndexData->long_descr)
    fprintf(fp,"LnD  %s~\n", pet->long_descr);
  if (pet->description != pet->pIndexData->description)
    fprintf(fp,"Desc %s~\n", pet->description);
  if (pet->race != pet->pIndexData->race)
    fprintf(fp,"Race %s~\n", race_table[pet->race].name);
  if ( pet->clan_name )
    fprintf( fp, "Clanname %s~\n", pet->clan_name );
  fprintf(fp,"Sex  %d\n", pet->sex );
  if (pet->level != pet->pIndexData->level )
    fprintf(fp,"Levl %d\n", pet->level );
  fprintf(fp, "HMV  %d %d %d %d %d %d\n",
          pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
  if (pet->gold > 0)
    fprintf(fp,"Gold %ld\n",pet->gold);
  if (pet->silver > 0)
    fprintf(fp,"Silv %ld\n",pet->silver);
  if (pet->exp > 0)
    fprintf(fp, "Exp  %llu\n", pet->exp);
  if (pet->act != pet->pIndexData->act)
    fprintf(fp, "Act  %s\n", print_flags(pet->act));
  if (pet->affected_by != pet->pIndexData->affected_by)
    fprintf(fp, "AfBy %s\n", print_flags(pet->affected_by));
  if (pet->affected2_by != pet->pIndexData->affected2_by)
    fprintf(fp, "AfBy2 %s\n", print_flags(pet->affected2_by));
  if (pet->comm_flags)
    fprintf(fp, "Comm %s\n", print_flags(pet->comm_flags));
  if (pet->pen_flags)
    fprintf(fp, "Pen %s\n", print_flags(pet->pen_flags));
  if (pet->chan_flags)
    fprintf(fp, "Chan %s\n", print_flags(pet->chan_flags));
  fprintf(fp,"Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
  if (pet->saving_throw != 0)
    fprintf(fp, "Save %d\n", pet->saving_throw);
  if (pet->alignment != pet->pIndexData->alignment)
    fprintf(fp, "Alig %d\n", pet->alignment);
  if (pet->hitroll != pet->pIndexData->hitroll)
    fprintf(fp, "Hit  %d\n", pet->hitroll);
  if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
    fprintf(fp, "Dam  %d\n", pet->damroll);
  fprintf(fp, "ACs  %d %d %d %d\n",
          pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);
  fprintf(fp, "Attr %d %d %d %d %d\n",
          pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
          pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
          pet->perm_stat[STAT_CON]);
  fprintf(fp, "AMod %d %d %d %d %d\n",
          pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
          pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
          pet->mod_stat[STAT_CON]);

  for ( paf = pet->affected; paf != NULL; paf = paf->next )
  {
    if (paf->type < 0 || paf->type >= MAX_SKILL)
      continue;

    fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %llu\n",
            skill_table[paf->type].name,
            paf->where,
            paf->level,
            paf->duration,
            paf->modifier,
            paf->location,
            paf->bitvector);
  }

  fprintf(fp,"End\n");
  return;
}

/*
 * Returns TRUE if it's ok to save an object with a character.
 */
bool can_save_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
  /* Combined object saves! */
  if ( IS_SET( obj->active_flags, ACTIVE_OBJ_COMBINED_SAVE ) )
    return FALSE;

  /* Everything is saved during copyover or for ploaded. */
  if ( IS_PLOADED( ch ) )
    return TRUE;

  if ( obj->item_type == ITEM_KEY )
  {
    if (obj->in_obj &&  obj->in_obj->item_type == ITEM_KEYRING )
      return TRUE;

    return FALSE;
  }

  /*
   * Items 3 levels or greater above character level do not save.
   */
  if ( get_trust(ch) < obj->level - 2 )
    return FALSE;

  return TRUE;
}

int save_obj_count( OBJ_DATA *iobj, OBJ_DATA *obj , CHAR_DATA *ch)
{
  OBJ_DATA *wobj = iobj;
  int count = 1;


  if ( IS_SET( obj->active_flags, ACTIVE_OBJ_COMBINED_SAVE ) )
  {
    bugf( "Save_obj_count: combined_save obj %d (%s)",
          obj->pIndexData->vnum, obj->short_descr );
    return 1;
  }


  if ( obj->wear_loc != WEAR_NONE
       ||     obj->enchanted
       ||     obj->affected
       ||     obj->contains
       ||     obj->extra_descr
       ||     !obj->pIndexData->new_format )
    return 1;

  for (wobj=iobj; wobj; wobj= wobj->next_content)
  {

    if ( obj->pIndexData    != wobj->pIndexData
         ||   wobj->wear_loc    != WEAR_NONE
         ||   wobj->enchanted
         ||   wobj->affected
         ||   wobj->contains
         ||   wobj->extra_descr
         ||   strcmp(obj->name, wobj->name)
         ||   strcmp(obj->short_descr,wobj->short_descr)
         ||   strcmp(obj->description,wobj->description)
         ||   obj->material     != wobj->material
         ||   obj->extra_flags    != wobj->extra_flags
         ||   obj->wear_flags    != wobj->wear_flags
         ||   obj->item_type    != wobj->item_type
         ||   obj->weight    != wobj->weight
         ||   obj->condition    != wobj->condition
         ||   obj->level     != wobj->level
         ||   obj->timer     != wobj->timer
         ||   obj->cost       != wobj->cost

         ||   obj->value[0]    != wobj->value[0]
         ||   obj->value[1]    != wobj->value[1]
         ||   obj->value[2]    != wobj->value[2]
         ||   obj->value[3]    != wobj->value[3]
         ||   obj->value[4]    != wobj->value[4]
         ||   obj->value[5]    != wobj->value[5]
         ||   obj->value[6]    != wobj->value[6]

         ||   ( IS_NULLSTR( obj->owner ) && !IS_NULLSTR( wobj->owner ) )
         ||   ( !IS_NULLSTR( obj->owner ) && IS_NULLSTR( wobj->owner ) )
         ||   ( !IS_NULLSTR( obj->owner ) && !IS_NULLSTR( wobj->owner )
                && str_cmp( obj->owner, wobj->owner ) )

         ||   !wobj->pIndexData->new_format
         ||   IS_SET( wobj->active_flags, ACTIVE_OBJ_COMBINED_SAVE )
         ||   obj == wobj )
      continue;

    SET_BIT( wobj->active_flags, ACTIVE_OBJ_COMBINED_SAVE );
    count++;
  }

  SET_BIT( obj->active_flags, ACTIVE_OBJ_COMBINED_SAVE );

  return count;
}
/*
 * Remove all combined-save flags from objs in character's inventory.
 */
void clear_combined_save_obj( OBJ_DATA *obj )
{
  OBJ_DATA *iobj;

  for ( iobj = obj; iobj; iobj = iobj->next_content )
  {
    REMOVE_BIT( iobj->active_flags, ACTIVE_OBJ_COMBINED_SAVE );
    if ( iobj->contains )
      clear_combined_save_obj( iobj->contains );

  }
}

/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *iobj, OBJ_DATA *obj, FILE *fp, int iNest, bool isLocker )
{
  EXTRA_DESCR_DATA *ed;
  AFFECT_DATA *paf;
  int count = 0;
  /*
   * Slick recursion to write lists backwards,
   *   so loading them will load in forwards order.
   */
  if ( obj->next_content )
    fwrite_obj( ch, iobj, obj->next_content, fp, iNest, isLocker );

  /*
   * Castrate storage characters.
   */
  if ( !can_save_obj(ch, obj))
    return;

  count = save_obj_count(iobj, obj, ch);

  if (count < 1)
  {
    bugf("Error, Count in saving is wrong.\n\r");
    send_to_char("Error in saving objects, count wrong.\n\r",ch);
    return;
  }

  if (1 == count)
  {
    if (isLocker)
      fprintf( fp, "#LOCKER\n" );
    else
      fprintf( fp, "#O\n" );
  }
  else
  {
    if (isLocker)
      fprintf( fp, "#LOCKERC %d\n", count );
    else
      fprintf( fp, "#OC %d\n", count );
  }
  fprintf( fp, "Vnum %d\n",   obj->pIndexData->vnum        );
  if (!obj->pIndexData->new_format)
    fprintf( fp, "Oldstyle\n");
  if (obj->enchanted)
    fprintf( fp,"Enchanted\n");
  fprintf( fp, "Nest %d\n",    iNest               );

  /* these data are only used if they do not match the defaults */

  if ( obj->name != obj->pIndexData->name)
    fprintf( fp, "Name %s~\n",    obj->name                );
  if ( obj->short_descr != obj->pIndexData->short_descr)
    fprintf( fp, "ShD  %s~\n",    obj->short_descr        );
  if ( obj->description != obj->pIndexData->description)
    fprintf( fp, "Desc %s~\n",    obj->description        );
  if ( obj->extra_flags != obj->pIndexData->extra_flags)
    fprintf( fp, "ExtF %llu\n",    obj->extra_flags        );
  if ( obj->wear_flags != obj->pIndexData->wear_flags)
    fprintf( fp, "WeaF %llu\n",    obj->wear_flags            );
  if ( obj->item_type != obj->pIndexData->item_type)
    fprintf( fp, "Ityp %d\n",    obj->item_type            );
  if ( obj->weight != obj->pIndexData->weight)
    fprintf( fp, "Wt   %d\n",    obj->weight                );
  if ( obj->condition != obj->pIndexData->condition)
    fprintf( fp, "Cond %d\n",    obj->condition            );
  if ( obj->owner != NULL)
    fprintf( fp, "Owner %s~\n", obj->owner              );
  if ( obj->famowner != NULL )
    fprintf( fp, "Famowner %s~\n",    obj->famowner            );
  if ( obj->material != obj->pIndexData->material )
    fprintf( fp, "Mat %s~\n",    obj->material        );

  /* variable data */

  fprintf( fp, "Wear %d\n",   obj->wear_loc                );
  if (obj->level != obj->pIndexData->level)
    fprintf( fp, "Lev  %d\n",    obj->level             );
  if (obj->timer != 0)
    fprintf( fp, "Time %d\n",    obj->timer         );

  fprintf( fp, "Cost %d\n",    obj->cost             );
  if (obj->value[0] != obj->pIndexData->value[0]
      ||  obj->value[1] != obj->pIndexData->value[1]
      ||  obj->value[2] != obj->pIndexData->value[2]
      ||  obj->value[3] != obj->pIndexData->value[3]
      ||  obj->value[4] != obj->pIndexData->value[4]
      ||  obj->value[5] != obj->pIndexData->value[5]
      ||  obj->value[6] != obj->pIndexData->value[6])

    fprintf( fp, "Val  %d %d %d %d %d %d %d\n",
             obj->value[0], obj->value[1], obj->value[2], obj->value[3],
             obj->value[4], obj->value[5], obj->value[6]);

  switch ( obj->item_type )
  {
    case ITEM_POTION:
    case ITEM_SCROLL:
    case ITEM_PILL:
      if ( obj->value[1] > 0 )
      {
        fprintf( fp, "Spell 1 '%s'\n",
                 skill_table[obj->value[1]].name );
      }

      if ( obj->value[2] > 0 )
      {
        fprintf( fp, "Spell 2 '%s'\n",
                 skill_table[obj->value[2]].name );
      }

      if ( obj->value[3] > 0 )
      {
        fprintf( fp, "Spell 3 '%s'\n",
                 skill_table[obj->value[3]].name );
      }

      if ( obj->value[4] > 0 )
      {
        fprintf( fp, "Spell 4 '%s'\n",
                 skill_table[obj->value[4]].name );
      }

      break;

    case ITEM_STAFF:
    case ITEM_WAND:
      if ( obj->value[3] > 0 )
      {
        fprintf( fp, "Spell 3 '%s'\n",
                 skill_table[obj->value[3]].name );
      }

      break;
  }

  for ( paf = obj->affected; paf != NULL; paf = paf->next )
  {
    if (paf->type < 0 || paf->type >= MAX_SKILL)
      continue;
    fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %llu\n",
             skill_table[paf->type].name,
             paf->where,
             paf->level,
             paf->duration,
             paf->modifier,
             paf->location,
             paf->bitvector
           );
  }

  for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
  {
    fprintf( fp, "ExDe %s~ %s~\n",
             ed->keyword, ed->description );
  }

  fprintf( fp, "End\n\n" );

  if ( obj->contains )
    fwrite_obj( ch, obj->contains, obj->contains, fp, iNest + 1, isLocker );

  return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
  char strsave[MAX_INPUT_LENGTH];
  char strsave1[MAX_INPUT_LENGTH];
  char buf[100];
  CHAR_DATA *ch;
  FILE *fp;
  bool found, sfound;
  int stat;
  int sn=0;

  ch = new_char();
  ch->pcdata = new_pcdata();
  ch->questdata               = new_questdata();

  d->character            = ch;
  ch->desc                = d;
  ch->name                = str_dup( name,ch->name );
#if MEMDEBUG
  ch->memdebug_name            = str_dup( name,ch->memdebug_name );
#endif
  ch->id                = get_pc_id();
  ch->group_num                = ch->id;
  ch->group_fight            = 0;
  ch->race                = race_lookup("human");
  ch->act                = PLR_NOSUMMON;
  ch->comm_flags            = COMM_COMBINE | COMM_PROMPT;
  ch->plr2 = 0;
  ch->prompt                 = str_dup( "{c<%hhp %mm %vmv>{x " ,ch->prompt);
  ch->idle_snapshot = 0;
  ch->idle_time = 0;
#if MEMDEBUG
  ch->memdebug_prompt             = str_dup( "{c<%hhp %mm %vmv>{x " , ch->memdebug_prompt);
#endif
  ch->pcdata->confirm_delete    = FALSE;
  ch->pcdata->history            = str_dup( "", ch->pcdata->history);
  ch->pcdata->pwd                = str_dup( "", ch->pcdata->pwd);
#if MEMDEBUG
  ch->pcdata->memdebug_pwd        = str_dup( "", ch->pcdata->memdebug_pwd);
#endif
  ch->pcdata->bamfin            = str_dup( "", ch->pcdata->bamfin       );
  ch->pcdata->bamfout            = str_dup( "", ch->pcdata->bamfout      );
  ch->pcdata->title                = str_dup( "", ch->pcdata->title        );
  ch->pcdata->permtitle            = str_dup( "", ch->pcdata->permtitle    );

  ch->pcdata->height = 0;
  ch->pcdata->weight = 0;
  ch->pcdata->eye_color            = str_dup( "", ch->pcdata->eye_color    );
  ch->pcdata->hair_color            = str_dup( "", ch->pcdata->hair_color    );
  ch->pcdata->skin_color            = str_dup( "", ch->pcdata->skin_color    );
#if MEMDEBUG
  ch->pcdata->memdebug_title    = str_dup( "", ch->pcdata->memdebug_title );
  ch->pcdata->memdebug_permtitle= str_dup( "", ch->pcdata->memdebug_permtitle );
#endif
  ch->pcdata->degrading        = 0;
  ch->pcdata->user_set_title        = FALSE;
  ch->pcdata->bounty            = 0;
  for (stat =0; stat < MAX_STATS; stat++)
    ch->perm_stat[stat]        = 13;
  ch->pcdata->condition[COND_THIRST]    = 48;
  ch->pcdata->condition[COND_FULL]    = 48;
  ch->pcdata->condition[COND_HUNGER]    = 48;
  ch->pcdata->security        = 0;    /* OLC */
  ch->pcdata->pkills = 0;
  ch->pcdata->pdeaths = 0;
  ch->pcdata->bloodshards = 0;

  found = FALSE;
  if (fpReserve)
  {
    fclose( fpReserve );
    nFilesOpen--;
  }

#if defined(unix)
  /* decompress if .gz file exists */
  mprintf(sizeof(strsave), strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
  if ( ( fp = fopen( strsave, "r" ) ) != NULL )
  {
    nFilesOpen++;
    fclose(fp);
    nFilesOpen--;
    mprintf(sizeof(buf),buf,"gzip -dfq %s",strsave);
    system(buf);
  }
  /* decompress if .gz file exists */
  mprintf(sizeof(strsave1), strsave1, "%s%s%s", DEAD_PLAYER_DIR, capitalize(name),".gz");
  if ( ( fp = fopen( strsave1, "r" ) ) != NULL )
  {
    nFilesOpen++;
    fclose(fp);
    nFilesOpen--;
    mprintf(sizeof(buf),buf,"gzip -dfq %s",strsave1);
    system(buf);
  }
#endif

  mprintf(sizeof(strsave), strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
  fp = fopen(strsave,"r");
  if (!fp)
  {
    mprintf( sizeof(strsave1),strsave1, "%s%s", DEAD_PLAYER_DIR, capitalize( name ) );
    fp = fopen(strsave1,"r");
    if (fp)
    {
      nFilesOpen++;
      fclose(fp);
      nFilesOpen--;
      rename(strsave1, strsave);
    }
  }
  else
    fclose(fp);
  fp = fopen(strsave,"r");
  if ( fp )
  {
    int iNest;
    nFilesOpen++;
    for ( iNest = 0; iNest < MAX_NEST; iNest++ )
      rgObjNest[iNest] = NULL;

    found = TRUE;
    for ( ; ; )
    {
      char letter;
      char *word;

      letter = fread_letter( fp );
      if ( letter == '*' )
      {
        fread_to_eol( fp );
        continue;
      }

      if ( letter != '#' )
      {
        bug( "Load_char_obj: # not found.", 0 );
        break;
      }

      word = fread_word( fp );
      if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
      else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( ch, fp, 0 , 0);
      else if ( !str_cmp( word, "O"      ) ) fread_obj  ( ch, fp, 0 , 0);
      else if ( !str_cmp( word, "OC"      ) ) fread_obj  ( ch, fp, 0, 1 );
      else if ( !str_cmp( word, "LOCKER" ) ) fread_obj  ( ch, fp, 1, 0 );
      else if ( !str_cmp( word, "LOCKERC" ) ) fread_obj  ( ch, fp, 1, 1 );
      else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
      else if ( !str_cmp( word, "END"    ) ) break;
      else
      {
        bug( "Load_char_obj: bad section.", 0 );
        break;
      }
      SET_BIT(ch->act, PLR_NOSUMMON);
      REMOVE_BIT(ch->act, PLR_CANLOOT);
      SET_BIT(ch->act, PLR_NOCANCEL);
      if (!IS_IMMORTAL(ch))
      {
        for (sn=0; sn < MAX_SKILL; sn++)
        {
          if (ch->pcdata->learned[sn] > 0)
            if (skill_table[sn].skill_level[ch->gameclass] == -1)
            {
              if (!is_racial_skill(ch, sn))
              {
                logit("Removing %s from character %s.\n\r",skill_table[sn].name, ch->name);
                ch->pcdata->learned[sn]=0;
              }
            }
        }
      }
      if (IS_SET(ch->spell_aff, SAFF_YAWN))
      {
        remove_affect(ch, TO_SPELL_AFFECTS, SAFF_YAWN);
      }
      if (ch->pet)
      {
        remove_affect(ch->pet, TO_SPELL_AFFECTS, SAFF_YAWN);
        REMOVE_BIT(ch->pet->spell_aff, SAFF_YAWN);
      }

    }
    fclose( fp );
    nFilesOpen--;
  }
  fpReserve = fopen( NULL_FILE, "r" );
  nFilesOpen++;
  if ( ch->clan ) save_clan( ch->clan );

  /* initialize race */
  if (found)
  {
    int i;

    if (ch->race == 0)
      ch->race = race_lookup("human");

    ch->size = pc_race_table[ch->race].size;
    ch->dam_type = 17; /*punch */

    for (i = 0; i < 5; i++)
    {
      if (pc_race_table[ch->race].skills[i] == NULL)
        break;
      group_add(ch,pc_race_table[ch->race].skills[i],FALSE);
    }
    ch->affected_by = ch->affected_by|race_table[ch->race].aff;
    ch->affected2_by  = ch->affected2_by|race_table[ch->race].aff2;
    ch->imm_flags    = ch->imm_flags | race_table[ch->race].imm;
    ch->res_flags    = ch->res_flags | race_table[ch->race].res;
    ch->vuln_flags    = ch->vuln_flags | race_table[ch->race].vuln;
    ch->form    = race_table[ch->race].form;
    ch->parts    = race_table[ch->race].parts;
  }


  /* RT initialize skills */

  if (found && ch->version < 2)  /* need to add the new skills */
  {
    group_add(ch,"rom basics",FALSE);
    group_add(ch,class_table[ch->gameclass].base_group,FALSE);
    group_add(ch,class_table[ch->gameclass].default_group,TRUE);
    ch->pcdata->learned[gsn_recall] = 50;
  }

  /* fix levels */
  if (found && ch->version < 3 && (ch->level > 35 || ch->trust > 35))
  {
    switch (ch->level)
    {
      case(40) : ch->level = 60;
        break;  /* imp -> imp */
      case(39) : ch->level = 58;
        break;    /* god -> supreme */
      case(38) : ch->level = 56;
        break;    /* deity -> god */
      case(37) : ch->level = 53;
        break;    /* angel -> demigod */
    }

    switch (ch->trust)
    {
      case(40) : ch->trust = 60;
        break;    /* imp -> imp */
      case(39) : ch->trust = 58;
        break;    /* god -> supreme */
      case(38) : ch->trust = 56;
        break;    /* deity -> god */
      case(37) : ch->trust = 53;
        break;    /* angel -> demigod */
      case(36) : ch->trust = 51;
        break;    /* hero -> hero */
    }
  }

  /* ream gold */
  if (found && ch->version < 4)
    ch->gold /= 100;

  /* throw together some random player attributes if they have none */
  if (found)
  {
    if (ch->pcdata->height == 0)
      ch->pcdata->height = number_range(pc_race_table[ch->race].min_height, pc_race_table[ch->race].max_height);

    if (ch->pcdata->weight == 0)
    {
      if ( (ch->sex == SEX_FEMALE)
           &&   (ch->size >= SIZE_MEDIUM) )
        ch->pcdata->weight = number_range(pc_race_table[ch->race].min_weight, (pc_race_table[ch->race].max_weight - 50));
      else
        ch->pcdata->weight = number_range(pc_race_table[ch->race].min_weight, pc_race_table[ch->race].max_weight);
    }

    if (!strcmp(ch->pcdata->eye_color,""))
    {
      sfound = FALSE;
      while (!sfound)
      {
        stat = number_range( 0, MAX_APPR-1 );
        if (pc_race_table[ch->race].eye_color[stat] != NULL)
        {
          free_string(ch->pcdata->eye_color);
          ch->pcdata->eye_color = str_dup(pc_race_table[ch->race].eye_color[stat], ch->pcdata->eye_color);
          sfound = TRUE;
        }
      }
    } // eyes

    if (!strcmp(ch->pcdata->hair_color,""))
    {
      sfound = FALSE;
      while (!sfound)
      {
        stat = number_range( 0, MAX_APPR-1 );
        if (pc_race_table[ch->race].hair_color[stat] != NULL)
        {
          free_string(ch->pcdata->hair_color);
          ch->pcdata->hair_color = str_dup(pc_race_table[ch->race].hair_color[stat], ch->pcdata->hair_color);
          sfound = TRUE;
        }
      }
    } // hair

    if (!strcmp(ch->pcdata->skin_color,""))
    {
      sfound = FALSE;
      while (!sfound)
      {
        stat = number_range( 0, MAX_APPR-1 );
        if (pc_race_table[ch->race].skin_color[stat] != NULL)
        {
          free_string(ch->pcdata->skin_color);
          ch->pcdata->skin_color = str_dup(pc_race_table[ch->race].skin_color[stat], ch->pcdata->skin_color);
          sfound = TRUE;
        }
      }
    } // skin
  } // if found, check for phy attributes

  return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                    \
  if ( !str_cmp( word, literal ) )    \
  {                    \
    field  = value;            \
    fMatch = TRUE;            \
    break;                \
  }

#if defined(KEYD)
#undef KEYD
#endif

#define KEYD( literal, field, value )                    \
  if ( !str_cmp( word, literal ) )    \
  {                    \
    field  = value;            \
    fMatch = TRUE;            \
  }

/* provided to free strings */
#if defined(KEYS)
#undef KEYS
#endif

#define KEYS( literal, field, value )                    \
  if ( !str_cmp( word, literal ) )    \
  {                    \
    free_string(field);            \
    field  = value;            \
    fMatch = TRUE;            \
    break;                \
  }
#if defined(KEYSD)
#undef KEYSD
#endif

#define KEYSD( literal, field, field2, value )                    \
  if ( !str_cmp( word, literal ) )    \
  {                    \
    free_string(field);            \
    free_string(field2);            \
    field  = value;            \
    field2  = str_dup(field, "");            \
    fMatch = TRUE;            \
    break;                \
  }

void fread_char( CHAR_DATA *ch, FILE *fp )
{
  char buf[MAX_STRING_LENGTH];
  char b1[MAX_STRING_LENGTH],b2[MAX_STRING_LENGTH],b3[MAX_STRING_LENGTH];
  char *bufQN;
  char *bufQS;
  char *bufQL;
  char *word;
  bool fMatch;
  int count = 0;
  int lastlogoff = current_time;
  int percent;
  CHAR_DATA     *fch;
  ROSTER_DATA   *clannie;
  int linenum =0;
  /*mprintf(sizeof(buf),buf,"Loading %s.",ch->name);
  log_string(buf);*/

  bufQN = b1;
  bufQS = b2;
  bufQL = b3;
  strcpy(bufQN,"Test");
  ch->martial_style = STYLE_NONE; // set a default style
  ch->pcdata->afktime = 12;
  ch->pcdata->mpquests = 0;

  for ( ; ; )
  {
    word   = feof( fp ) ? "End" : fread_word( fp );
    fMatch = FALSE;
    linenum++;
    switch ( UPPER(word[0]) )
    {
      case '*':
        fMatch = TRUE;
        fread_to_eol( fp );
        break;

      case 'A':
        KEY( "Act",        ch->act,             fread_flag( fp ) );
        KEY( "AffectedBy", ch->affected_by,     fread_flag( fp ) );
        KEY( "AfBy2",      ch->affected2_by,    fread_flag( fp ) );
        KEY( "AfBy",       ch->affected_by,     fread_flag( fp ) );
        KEY( "Alignment",  ch->alignment,       fread_number( fp ) );
        KEY( "Alig",       ch->alignment,       fread_number( fp ) );
        KEY( "AFKTimer",   ch->pcdata->afktime, fread_number( fp ) );

        if (!str_cmp( word, "Alia"))
        {
          if (count >= MAX_ALIAS)
          {
            fread_to_eol(fp);
            fMatch = TRUE;
            break;
          }
          /* ERROR ERROR look berlow */
          ch->pcdata->alias[count]     = str_dup(fread_word(fp),ch->pcdata->alias[count]);
          ch->pcdata->alias_sub[count]    = str_dup(fread_word(fp),ch->pcdata->alias_sub[count]);
          count++;
          fMatch = TRUE;
          break;
        }

        if (!str_cmp( word, "Alias"))
        {
          if (count >= MAX_ALIAS)
          {
            fread_to_eol(fp);
            fMatch = TRUE;
            break;
          }
          /* ERROR ERROR look below and wonder why above error is
          diff?? */
          ch->pcdata->alias[count]        = str_dup(fread_word(fp),ch->pcdata->alias[count]);
          ch->pcdata->alias_sub[count]    = fread_string(fp);
          count++;
          fMatch = TRUE;
          break;
        }

        if (!str_cmp( word, "AC") || !str_cmp(word,"Armor"))
        {
          fread_to_eol(fp);
          fMatch = TRUE;
          break;
        }

        if (!str_cmp(word,"ACs"))
        {
          int i;

          for (i = 0; i < 4; i++)
            ch->armor[i] = fread_number(fp);
          fMatch = TRUE;
          break;
        }

        if (!str_cmp(word, "AffD"))
        {
          AFFECT_DATA *paf;
          int sn;

          paf = new_affect();

          sn = skill_lookup_err(fread_word(fp));
          if (sn < 0)
            bug("Fread_char: unknown skill.",0);
          else
            paf->type = sn;

          paf->level    = fread_number( fp );
          paf->duration    = fread_number( fp );
          paf->modifier    = fread_number( fp );
          paf->location    = fread_number( fp );
          paf->bitvector    = fread_number( fp );
          paf->next    = ch->affected;
          ch->affected    = paf;
          fMatch = TRUE;
          break;
        }

        if (!str_cmp(word, "Affc"))
        {
          AFFECT_DATA *paf;
          int sn;

          paf = new_affect();

          sn = skill_lookup_err(fread_word(fp));
          if (sn < 0)
            bug("Fread_char: unknown skill.",0);
          else
            paf->type = sn;

          paf->where      = fread_number( fp );
          paf->level      = fread_number( fp );
          paf->duration   = fread_number( fp );
          paf->modifier   = fread_number( fp );
          paf->location   = fread_number( fp );
          paf->bitvector  = fread_number( fp );
          paf->next       = ch->affected;
          ch->affected    = paf;
          /*affect_to_char(ch,paf);*/
          fMatch = TRUE;
          break;
        }

        if ( !str_cmp( word, "AttrMod"  ) || !str_cmp(word,"AMod"))
        {
          int stat;
          for (stat = 0; stat < MAX_STATS; stat ++)
            ch->mod_stat[stat] = fread_number(fp);
          fMatch = TRUE;
          break;
        }

        if ( !str_cmp( word, "AttrPerm" ) || !str_cmp(word,"Attr"))
        {
          int stat;

          for (stat = 0; stat < MAX_STATS; stat++)
            ch->perm_stat[stat] = fread_number( fp );
          fMatch = TRUE;
          break;
        }
        break;

      case 'B':
        KEYS( "Bamfin",    	ch->pcdata->bamfin,     	fread_string( fp ) );
        KEYS( "Bamfout",    ch->pcdata->bamfout,    	fread_string( fp ) );
        KEYS( "Bin",        ch->pcdata->bamfin,     	fread_string( fp ) );
        KEYS( "Bout",       ch->pcdata->bamfout,    	fread_string( fp ) );
        KEY( "BailTime",  	ch->bailed,             	fread_number( fp ) );
        KEY( "Balance",   	ch->pcdata->balance,   	 	fread_number( fp ) );
        KEY( "Shares",    	ch->pcdata->shares,    		fread_number( fp ) );
        KEY( "Bounty",    	ch->pcdata->bounty,      	fread_number( fp ) );
			  KEY( "Bloodshards", ch->pcdata->bloodshards, 	fread_number( fp ) ); //2015-06-11

        if (!strcmp(word, "BDay")) /* Voltecs player birthdays! */
        {
          ch->birthday.hour  = fread_number(fp);
          ch->birthday.day   = fread_number(fp);
          ch->birthday.month = fread_number(fp);
          ch->birthday.year  = fread_number(fp);
          fMatch = TRUE;
          break;
        }

        KEY( "BufCnt",    ch->tells,    fread_number( fp ) );

        if ( !str_cmp( word, "BufStr" ) )
        {
          char *tmp;

          clear_buf( ch->pcdata->buffer );
          tmp = fread_string( fp );
          bstrcat( ch->pcdata->buffer, tmp );
          free_string( tmp );
          fMatch = TRUE;
          break;
        }
        break;

      case 'C':
        KEY( "Class",        ch->gameclass,    fread_number( fp ) );
        KEY( "Cla",        ch->gameclass,    fread_number( fp ) );
        //KEY( "Clan",    ch->clan,        fread_number(fp));
        KEY( "ClanOweDia",    ch->pcdata->clanowe_dia, fread_number(fp)); /* Clans */
        KEY( "ClanOweLevel",  ch->pcdata->clanowe_level, fread_number(fp)); /* Clans */
        KEYS( "ClanOweClan",   ch->pcdata->clanowe_clan, fread_string(fp)); /* Clans */

        if ( !str_cmp( word, "Clan" ) )
        {
          int i = 0;

          i      = fread_number( fp );
          strip_color( buf, clan_table[i].name );
          buf[0] = UPPER( buf[0] );
          ch->clan_name = str_dup(  buf, ch->clan_name );
          ch->clan = get_clan( ch->clan_name, TRUE );
          if ( ch->clan )
          {
            if ( !find_clan_roster_member( ch ) )
            {
              clannie = new_clannie();
              clannie->rank     = ch->pcdata->clan_rank;
              clannie->donated  = 0;
              clannie->sex      = ch->sex;
              clannie->level    = ch->level;
              clannie->alignment= ch->alignment;

              free_string( clannie->name );
              clannie->name = str_dup( ch->name, clannie->name );

              add_clannie( ch->clan, clannie );
            }
          }
          else
            free_string( ch->clan_name );

          fMatch = TRUE;
          break;
        }

        if ( !str_cmp( word, "Clanname" ) )
        {
          ch->clan_name     = fread_string( fp );
          ch->clan = get_clan( ch->clan_name, TRUE );
          if ( !find_clan_roster_member( ch ) )
          {
            ch->clan = NULL;
            free_string( ch->clan_name );
            // Taeloch 3/30/17: you can't send_to_char() to "ch" here,
            // as "ch" is a player loaded from a file.  It has no
            // connection/descriptor to send to!
            // send_to_char( "Character not found in roster.\n\r", ch );
          }
          fMatch = TRUE;
          break;
        }

        if (!str_cmp(word,"CharmMaster"))
        {
          char *tmp = fread_string(fp);
          bool playing = FALSE;
          fMatch = TRUE;
          /* Check for already playing (reconnecting) */
          for (fch = player_list; fch; fch = fch->next_player)
          {
            if (ch->id == fch->id)
            {
              playing = TRUE;
              break;
            }
          }
          if (playing)
          {
            free_string(tmp);
            break;
          }
          for (fch = player_list; fch; fch = fch->next_player)
          {
            if (!strcmp(fch->name,tmp))
            {
              SET_BIT(ch->affected_by, AFF_CHARM);
              ch->master = fch;
              ch->leader = fch;
              send_to_char("Your servant has reconnected!.\n\r",fch);
              break;
            }
          }
          free_string(tmp);
          break;
        }

        if ( !str_cmp( word, "Condition" ) || !str_cmp(word,"Cond"))
        {
          ch->pcdata->condition[0] = fread_number( fp );
          ch->pcdata->condition[1] = fread_number( fp );
          ch->pcdata->condition[2] = fread_number( fp );
          fMatch = TRUE;
          break;
        }
        if (!str_cmp(word,"Cnd"))
        {
          ch->pcdata->condition[0] = fread_number( fp );
          ch->pcdata->condition[1] = fread_number( fp );
          ch->pcdata->condition[2] = fread_number( fp );
          ch->pcdata->condition[3] = fread_number( fp );
          /*          ch->pcdata->condition[4] = fread_number(fp ); */
          fMatch = TRUE;
          break;
        }
        if (ch->version <=7 )
        {
          KEY("Comm",        ch->comm_flags,        fread_flag( fp ) );
          /* bit anding to mask off the improper previous bits.
             I hope this is correct. WHY THE HELL AINT THIS WORKING??*/
          ch->pen_flags = (ch->comm_flags & 0x41400000);
          ch->chan_flags = (ch->comm_flags & 0x0CBE87ff);
          ch->comm_flags = (ch->comm_flags & 0x32017800);

        }
        else
        {
          KEY("Comm",        ch->comm_flags,        fread_flag( fp ) );
        }
        KEY("Chan",        ch->chan_flags,        fread_flag( fp ) );

        break;

      case 'D':
        KEY( "Damroll",    ch->damroll,        fread_number( fp ) );
        KEY( "Dam",        ch->damroll,        fread_number( fp ) );
        KEY( "Denytime",    ch->pcdata->denytime,        fread_number( fp ) );
        KEY( "Dtime",    ch->pcdata->denied_time,        fread_number( fp ) );
        KEYS( "Description",    ch->description,    fread_string( fp ) );
        KEYS( "Desc",    ch->description,    fread_string( fp ) );
        KEY( "Dias",    ch->pcdata->donated_dia, fread_number( fp) );
        KEY( "Degraded",    ch->pcdata->degrading, fread_number( fp) );
        KEY( "Deadchar", ch->pcdata->old_char, fread_number(fp));
        KEY( "Dths", ch->pcdata->deaths, fread_number(fp));
        break;

      case 'E':
        if ( !str_cmp( word, "End" ) )
        {
          /* adjust hp mana move up  -- here for speed's sake */
          ch->group_num = ch->id;
          percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);
          percent = UMIN(percent,100);

          if (ch->level < LEVEL_HERO) // Taeloch: now with XP after hero!
          {
            int XPL = exp_per_level( ch, ch->pcdata->points );
            if ( ch->exp < TNL( XPL, ch->level - 1 ) )
              ch->exp = TNL( XPL, ch->level - 1 );
            if ( ch->exp > TNL( XPL, ch->level ) )
              ch->exp = TNL( XPL, ch->level ) - 1;//makes him just below level
          }

          if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
              &&  !IS_AFFECTED(ch,AFF_PLAGUE))
          {
            ch->hit    += (ch->max_hit - ch->hit) * percent / 100;
            ch->mana    += (ch->max_mana - ch->mana) * percent / 100;
            ch->move    += (ch->max_move - ch->move)* percent / 100;
          }

          // is player on a rescue quest?
          if ( ch->questdata->mob_vnum == MOB_VNUM_QUEST )
          {
            // recreate rescue mob - Taeloch
            if ( ( strcmp(bufQN,"") )
            &&   ( strcmp(bufQS,"") )
            &&   ( strcmp(bufQL,"") ) ) // make sure all three values have been loaded
            {
              CHAR_DATA *qch, *lch = NULL;
              bool qfound = FALSE;

              for ( qch = char_list ; qch ; qch = qch->next)
              {
                // first make sure the mob doesn't already exist somehow
                if ( !IS_NPC( qch ) )
                  continue;

                if ( strstr(qch->name,ch->name )
                && ( qch->pIndexData->vnum == MOB_VNUM_QUEST ) )
                {
                  // if this is a duplicate quest mob, get rid of the dupe!
                  if (qfound)
                  {
                    char_from_room(qch);
                    extract_char(qch, FALSE);
                  }
                  else // otherwise, it's the live one
                    lch = qch;

                  qfound = TRUE;
                } // found a rescue mob for player
              } // char_list loop

              if (!qfound) // doesn't exist?  Create it
              {
                lch = create_mobile( get_mob_index(MOB_VNUM_QUEST) );
                if (lch->short_descr)
                  free_string(lch->short_descr);
                if (lch->long_descr)
                  free_string(lch->long_descr);
                if (lch->name)
                  free_string(lch->name);
                mprintf(sizeof(buf),buf, "%s QQ%s", bufQN, ch->name);
                lch->name        = str_dup(bufQN, lch->name);
                lch->long_descr  = str_dup(bufQL, lch->long_descr);
                lch->short_descr = str_dup(bufQS, lch->short_descr);
              }

              if (lch != NULL) // move quest mob
              {
                if (lch->in_room)
                  char_from_room(lch);
                char_to_room(lch, get_room_index(ch->questdata->room_vnum));
              }
            } // if all three strings have been loaded
          } // end rescue mob recreation (if rescue quest)

          return;
        }
        KEY( "Exp",        ch->exp,        fread_number( fp ) );
        break;


      case 'F':
        if ( !str_cmp( word, "Fragments" ) )
        {
          ch->ruby_fragment      = fread_number( fp );
          ch->sapphire_fragment  = fread_number( fp );
          ch->emerald_fragment   = fread_number( fp );
          ch->diamond_fragment   = fread_number( fp );
          fMatch = TRUE;
        }

        if ( !str_cmp( word, "FragCounter" ) )
        {
          ch->ruby_counter    = fread_number( fp );
          ch->sapphire_counter= fread_number( fp );
          ch->emerald_counter = fread_number( fp );
          ch->diamond_counter = fread_number( fp );
          fMatch = TRUE;
        }

        if ( !str_cmp( word, "FragPurchases" ) )
        {
          ch->first_frag_level  = fread_number(fp);
          ch->second_frag_level = fread_number(fp);
          ch->third_frag_level  = fread_number(fp);
          ch->fourth_frag_level = fread_number(fp);
          ch->fifth_frag_level  = fread_number(fp);
          fMatch = TRUE;
        }

        KEY( "FdBugs",  ch->pcdata->fdbugs,  fread_number(fp ) );
        KEY( "FdDuhs",  ch->pcdata->fdduhs,  fread_number(fp ) );
        KEY( "FdTypos", ch->pcdata->fdtypos, fread_number(fp ) );
        KEY( "FdHelps", ch->pcdata->fdhelps, fread_number(fp ) );
        KEY( "fish_caught", ch->pcdata->fish_caught, fread_number(fp) );
        KEY( "fish_lost", ch->pcdata->fish_lost, fread_number(fp) );
        KEY( "fish_broken", ch->pcdata->fish_broken, fread_number(fp) );
        break;

      case 'G':
        KEY( "Gold",    ch->gold,        fread_number( fp ) );
        if ( !str_cmp( word, "Group" )  || !str_cmp(word,"Gr"))
        {
          int gn;
          char *temp;

          temp = fread_word( fp ) ;
          gn = group_lookup(temp);
          /* gn    = group_lookup( fread_word( fp ) ); */
          if ( gn < 0 )
          {
            fprintf(stderr,"%s",temp);
            bug( "Fread_char: unknown group. ", 0 );
          }
          else
            gn_add(ch,gn);
          fMatch = TRUE;
        }
        break;

      case 'H':
        KEY( "Hitroll",    ch->hitroll,           fread_number( fp ) );
        KEY( "Hit",        ch->hitroll,           fread_number( fp ) );
        KEYS( "Hist",      ch->pcdata->history,   fread_string( fp ) );
        KEYS("Host",       ch->pcdata->host,      fread_string(fp)    ); /* TML */
        KEY( "Home",       ch->pcdata->home_vnum, fread_number(fp) );

        if ( !str_cmp( word, "HpManaMove" ) || !str_cmp(word,"HMV"))
        {
          ch->hit        = fread_number( fp );
          ch->max_hit    = fread_number( fp );
          ch->mana    = fread_number( fp );
          ch->max_mana    = fread_number( fp );
          ch->move    = fread_number( fp );
          ch->max_move    = fread_number( fp );
          fMatch = TRUE;
          break;
        }

        if ( !str_cmp( word, "HpManaMovePerm" ) || !str_cmp(word,"HMVP"))
        {
          //GET_HP(ch)             = fread_number( fp );
          ch->pcdata->perm_hit    = fread_number( fp );
          ch->pcdata->perm_mana   = fread_number( fp );
          ch->pcdata->perm_move   = fread_number( fp );
          fMatch = TRUE;
          break;
        }

        break;

      case 'I':
        KEY( "Id",        ch->id,            fread_number( fp ) );
        KEY( "InvisLevel",    ch->invis_level,    fread_number( fp ) );
        KEY( "Inco",    ch->incog_level,    fread_number( fp ) );
        KEY( "Invi",    ch->invis_level,    fread_number( fp ) );
        break;
      case 'K':
        KEY( "Kills",    ch->pcdata->kills,    fread_number(fp ) );
        break;
      case 'L':
        KEY( "LastLevel",    ch->pcdata->last_level, fread_number( fp ) );
        KEY( "LstLevelPlyd",    ch->last_level, fread_number( fp ) );
        KEY( "LLev",    ch->pcdata->last_level, fread_number( fp ) );
        KEY( "Level",    ch->level,        fread_number( fp ) );
        KEY( "Lev",        ch->level,        fread_number( fp ) );
        KEY( "Levl",    ch->level,        fread_number( fp ) );
        KEY( "Locker_Max",    ch->pcdata->locker_max, fread_number( fp ) );
        KEY( "LogO",    ch->pcdata->laston,        fread_number( fp ) );
        KEYS( "LongDescr",    ch->long_descr,        fread_string( fp ) );
        KEYS( "LnD",        ch->long_descr,        fread_string( fp ) );
        break;

      case 'M':
        if ( !str_cmp(word,"Mn"))
        {
          int mn;
          int sn;
          char *temp;

          temp = fread_word( fp ) ;
          sn = skill_lookup(temp);
          if ( sn < 0 )
          {
            fprintf(stderr,"%s",temp);
            bugf( "Fread_char: unknown skill in maintained. " );
          }
          else
          {
            for (mn = 0; mn < MAX_MAINTAINED; mn ++)
            {
              if (ch->pcdata->maintained[mn] == sn)
              {
                fMatch = TRUE;
                break;
              }

              if (ch->pcdata->maintained[mn] <= 0)
              {
                fMatch = TRUE;
                ch->pcdata->maintained[mn] = sn;
                break;
              }
            }
          }
        }

        if ( !str_cmp(word,"Mno"))
        {
          int mno;
          int sn;
          char *temp;

          temp = fread_word( fp ) ;
          sn = skill_lookup(temp);
          if ( sn < 0 )
          {
            fprintf(stderr,"%s",temp);
            bugf( "Fread_char: unknown skill in maintained_other. " );
          }
          else
          {
            for (mno = 0; mno < MAX_MAINTAINED; mno++)
            {
              if (ch->pcdata->maintained_other[mno] == sn)
              {
                fMatch = TRUE;
                break;
              }

              if (ch->pcdata->maintained_other[mno] <= 0)
              {
                fMatch = TRUE;
                ch->pcdata->maintained_other[mno] = sn;
                break;
              }
            }
          }
        }

        if ( !str_cmp(word,"Mnw"))
        {
          int mno;
          int sn;
          char *temp;

          temp = fread_word( fp ) ;
          sn = skill_lookup(temp);
          if ( sn < 0 )
          {
            fprintf(stderr,"%s",temp);
            bugf( "Fread_char: unknown skill in maintained_weapon. " );
          }
          else
          {
            for (mno = 0; mno < MAX_MAINTAINED; mno++)
            {
              if (ch->pcdata->maintained_weapon[mno] == sn)
              {
                fMatch = TRUE;
                break;
              }

              if (ch->pcdata->maintained_weapon[mno] <= 0)
              {
                fMatch = TRUE;
                ch->pcdata->maintained_weapon[mno] = sn;
                break;
              }
            }
          }
        }

        break;
      case 'N':
#if MEMDEBUG
        KEYSD( "Name",    ch->name,ch->memdebug_name,        fread_string( fp ) );
#else
        KEYS( "Name",    ch->name,        fread_string( fp ) );
#endif
        KEY( "Nospam",           ch->nospam,            fread_flag( fp ) );
        KEY( "Note",    ch->pcdata->last_note,    fread_number( fp ) );
        KEY( "NumClans",ch->pcdata->numclans,    fread_number( fp ) );
        KEY( "NumBail",ch->num_bailed,    fread_number( fp ) );
        if (!str_cmp(word,"Not"))
        {
          ch->pcdata->last_note            = fread_number(fp);
          ch->pcdata->last_idea            = fread_number(fp);
          ch->pcdata->last_penalty        = fread_number(fp);
          ch->pcdata->last_news            = fread_number(fp);
          ch->pcdata->last_changes        = fread_number(fp);
          if (ch->version > 5)
            ch->pcdata->last_rules        = fread_number(fp);
          if (ch->version > 6)
            ch->pcdata->last_rpnote        = fread_number(fp);
          fMatch = TRUE;
          break;
        }
        if ( !str_cmp( word, "NoteType" ) )
        {
          if ( ch->pnote == NULL )
            note_attach( ch, NOTE_NOTE );
          ch->pnote->type =fread_number( fp );
          fMatch = TRUE;
          break;
        }
        if ( !str_cmp( word, "NoteTo" ) )
        {
          if ( ch->pnote == NULL )
            note_attach( ch, NOTE_NOTE );
          free_string( ch->pnote->to_list );
          ch->pnote->to_list        = fread_string( fp );
          fMatch = TRUE;
          break;
        }
        if ( !str_cmp( word, "NoteSubj" ) )
        {
          if ( ch->pnote == NULL )
            note_attach( ch, NOTE_NOTE );
          free_string( ch->pnote->subject );
          ch->pnote->subject        = fread_string( fp );
          fMatch = TRUE;
          break;
        }
        if ( !str_cmp( word, "NoteText" ) )
        {
          if ( ch->pnote == NULL )
            note_attach( ch, NOTE_NOTE );
          free_string( ch->pnote->text );
          ch->pnote->text            = fread_string( fp );
          fMatch = TRUE;
          break;
        }


        break;

      case 'P':
#if MEMDEBUG
        KEYSD( "Password",    ch->pcdata->pwd,ch->pcdata->memdebug_pwd,    fread_string( fp ) );
        KEYSD( "Pass",    ch->pcdata->pwd,ch->pcdata->memdebug_pwd,    fread_string( fp ) );
#else
        KEYS( "Password",    ch->pcdata->pwd,    fread_string( fp ) );
        KEYS( "Pass",    ch->pcdata->pwd,    fread_string( fp ) );
#endif
        KEY( "Pkills",    ch->pcdata->pkills, fread_number( fp ) );
        KEY( "Pdths",     ch->pcdata->pdeaths,fread_number( fp ) );
        KEY( "Played",    ch->played,            fread_number( fp ) );
        KEY( "Plyd",        ch->played,            fread_number( fp ) );
        KEY( "Points",    ch->pcdata->points,    fread_number( fp ) );
        KEY( "Pnts",        ch->pcdata->points,    fread_number( fp ) );
        KEY( "Position",    ch->position,        fread_number( fp ) );
        KEY( "Pos",        ch->position,        fread_number( fp ) );
        KEY( "Practice",    ch->practice,        fread_number( fp ) );
        KEY( "Pen",        ch->pen_flags,        fread_flag( fp ) );
        KEY( "Prac",        ch->practice,        fread_number( fp ) );

        KEY( "PhyEyes",     ch->pcdata->eye_color,  fread_string( fp ) );
        KEY( "PhyHair",     ch->pcdata->hair_color, fread_string( fp ) );
        KEY( "PhySkin",     ch->pcdata->skin_color, fread_string( fp ) );
        KEY( "PhyHeight",   ch->pcdata->height,     fread_number( fp ) );
        KEY( "PhyWeight",   ch->pcdata->weight,     fread_number( fp ) );


#if MEMDEBUG
        KEYSD( "Prompt",      ch->prompt,ch->memdebug_prompt,             fread_string( fp ) );
        KEYSD( "Prom",    ch->prompt,ch->memdebug_prompt,        fread_string( fp ) );
#else
        KEYS( "Prompt",      ch->prompt,             fread_string( fp ) );
        KEYS( "Prom",    ch->prompt,        fread_string( fp ) );
#endif
        KEY("Plr2",        ch->plr2,        fread_flag( fp ) );
        if ( !str_cmp( word, "PermTitl"))
        {
          ch->pcdata->permtitle = fread_string( fp );
#if MEMDEBUG
          free_string(ch->pcdata->memdebug_permtitle);
          ch->pcdata->memdebug_permtitle = str_dup
                                           ( ch->pcdata->permtitle, ch->pcdata->memdebug_permtitle );
#endif
          if (ch->pcdata->permtitle[0] != '.' && ch->pcdata->permtitle[0] != ','
              &&  ch->pcdata->permtitle[0] != '!' && ch->pcdata->permtitle[0] != '?')
          {
            mprintf(sizeof(buf),buf, " %s", ch->pcdata->permtitle );
#if MEMDEBUG
            free_string( ch->pcdata->memdebug_permtitle );
            ch->pcdata->memdebug_permtitle = str_dup( buf, ch->pcdata->memdebug_permtitle );
#endif
            free_string( ch->pcdata->permtitle );
            ch->pcdata->permtitle = str_dup( buf ,ch->pcdata->permtitle);
          }
          fMatch = TRUE;
          break;
        }
        break;

      case 'Q':
        KEY( "QuestObj",        ch->questdata->obj_vnum,      fread_number( fp ) );
        KEY( "QuestMob",        ch->questdata->mob_vnum,      fread_number( fp ) );
        KEY( "QuestRoom",       ch->questdata->room_vnum,     fread_number( fp ) );
        KEY( "Quest_Curr",      ch->questdata->curr_points,   fread_number( fp ) );
        KEY( "Quest_Accum",     ch->questdata->accum_points,  fread_number( fp ) );
        KEY( "QuestCount",      ch->questdata->countdown,     fread_number( fp ));
        KEY( "QuestAllow",      ch->questdata->time_allowed,  fread_number( fp ));
        KEY( "QuestNext",       ch->questdata->nextquest,     fread_number( fp ));
        KEY( "QuestStreak",     ch->questdata->streak,        fread_number( fp ));
        KEY( "QuestStreakNeed", ch->questdata->current_streak,fread_number( fp ));
        KEY( "QuestStreakBest", ch->questdata->best_streak,   fread_number( fp ));
        KEY( "Quest_Num",       ch->questdata->quest_number,  fread_number( fp ));
        KEY( "Quest_Att",       ch->questdata->attempt_num,   fread_number( fp ));
        KEY( "Quest_Quit",      ch->questdata->quit_num,      fread_number( fp ));
        KEY( "Quest_Comp",      ch->questdata->comp_num,      fread_number( fp ));
        KEY( "Quest_Glory",     ch->questdata->glory,         fread_number( fp ));
        KEY( "Quest_Failed",    ch->questdata->failed,        fread_number( fp ));
        KEY( "QBits",           ch->pcdata->mpquests,         fread_number( fp ));
        KEY( "QuestName",  bufQN, fread_string( fp ));
        KEY( "QuestShort", bufQS, fread_string( fp ));
        KEY( "QuestLong",  bufQL, fread_string( fp ));

        if ( !str_cmp(word, "Quest"))
        {
          int tmp = fread_number(fp);
          if (tmp)
            SET_BIT(ch->act,PLR_QUESTING);
          fMatch = TRUE;
          break;
        }
        if (!str_cmp(word,"QuestGiver"))
        {
          char *tmp = fread_string(fp);
          fMatch = TRUE;
          for (fch = char_list; fch; fch = fch->next)
            if (!strcmp(fch->short_descr,tmp))
            {
              ch->questdata->questgiver = fch;
              free_string(tmp);
              break;
            }
          free_string(tmp);
          break;
        }
        if ( !str_cmp( word, "QuestText" ) )
        {
          char *tmp;
          clear_buf(ch->questdata->log );
          tmp = fread_string( fp );
          add_buf(ch->questdata->log, tmp);
          free_string(tmp);
          fMatch = TRUE;
          break;
        }

        break;
      case 'R':
        KEY( "Rank",     ch->pcdata->clan_rank,    fread_number(fp) ); /*Clans */
        if ( !str_cmp(word, "Race"))
        {
          char *tmp = fread_string(fp);
          ch->race = race_lookup(tmp);
          free_string(tmp);
          fMatch = TRUE;
          break;
        }

        if ( !str_cmp( word, "Room" ) )
        {
          ch->in_room = get_room_index( fread_number( fp ) );
          if ( ch->in_room == NULL )
            ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
          fMatch = TRUE;
          break;
        }

        break;

      case 'S':
        KEY( "SavingThrow",    ch->saving_throw,    fread_number( fp ) );
        KEY( "Save",    ch->saving_throw,    fread_number( fp ) );
        KEY( "Scro",    ch->lines,        fread_number( fp ) );
        KEY( "Sex",        ch->sex,        fread_number( fp ) );
        KEY( "SexDir",    ch->sex_dir,        fread_number( fp ) );
        KEYS( "ShortDescr",    ch->short_descr,    fread_string( fp ) );
        KEYS( "ShD",        ch->short_descr,    fread_string( fp ) );
        KEY( "Sec",         ch->pcdata->security,    fread_number( fp ) );    /* OLC */
        KEY( "SecFlag",    ch->pcdata->security_flags, fread_flag( fp ) ); /* OLC */
        KEY( "Silv",        ch->silver,             fread_number( fp ) );
        KEY( "Balance", ch->pcdata->balance, fread_number( fp ) );
        KEY( "Shares",  ch->pcdata->shares,  fread_number( fp ) );
        KEY( "Style",   ch->martial_style,  fread_number( fp ) );
        KEY( "SpAfBy",    ch->spell_aff,        fread_flag( fp ) );

        if ( !str_cmp( word, "Skill" ) || !str_cmp(word,"Sk"))
        {
          int sn;
          int value;
          char *temp;

          value = fread_number( fp );
          temp = fread_word( fp ) ;
          sn = skill_lookup_err(temp);
          /* sn    = skill_lookup( fread_word( fp ) ); */
          if ( sn < 0 )
          {
            fprintf(stderr,"%s",temp);
            bug( "Fread_char: unknown skill. ", 0 );
          }
          else
            ch->pcdata->learned[sn] = value;
          fMatch = TRUE;
        }

        break;

      case 'T':
        KEY( "TrueSex",     ch->pcdata->true_sex,      fread_number( fp ) );
        KEY( "TSex",    ch->pcdata->true_sex,   fread_number( fp ) );
        KEY( "Trai",    ch->train,        fread_number( fp ) );
        KEY( "Trust",    ch->trust,        fread_number( fp ) );
        KEY( "Tru",        ch->trust,        fread_number( fp ) );
        KEY( "TitSet",    ch->pcdata->user_set_title,    fread_number( fp ) );

        if ( !str_cmp( word, "Title" )  || !str_cmp( word, "Titl"))
        {
          // ch->pcdata->title = fread_string( fp );
          char *tmp = fread_string( fp );
          set_title( ch, tmp );
          free_string( tmp );
          fMatch = TRUE;
          break;
        }

        if ( !str_cmp( word, "Title_afk" ) )
        {
          // ch->pcdata->title = fread_string( fp );
          char *tmp = fread_string( fp );
          set_titleafk( ch, tmp );
          free_string( tmp );
          fMatch = TRUE;
          break;
        }


        break;

      case 'V':
        KEY( "Version",     ch->version,        fread_number ( fp ) );
        KEY( "Vers",    ch->version,        fread_number ( fp ) );
        KEY( "Vtimer",ch->vflag_timer,        fread_number ( fp ) );
        if ( !str_cmp( word, "Vnum" ) )
        {
          ch->pIndexData = get_mob_index( fread_number( fp ) );
          fMatch = TRUE;
          break;
        }
        break;

      case 'W':
        KEY( "Wimpy",   ch->wimpy,           fread_number( fp ) );
        KEY( "Wimp",    ch->wimpy,           fread_number( fp ) );
        KEY( "Wizn",    ch->wiznet,          fread_flag( fp ) );
        KEY( "WLBugs",  ch->pcdata->wlbugs,  fread_number(fp ) );
        KEY( "WLDuhs",  ch->pcdata->wlduhs,  fread_number(fp ) );
        KEY( "WLTypos", ch->pcdata->wltypos, fread_number(fp ) );
        KEY( "WLBuild", ch->pcdata->wlbuild, fread_number(fp ) );
        KEY( "WLHelps", ch->pcdata->wlhelps, fread_number(fp ) );
        break;
    }

    if ( !fMatch )
    {

      bugf( "Fread_char: no match word %s line number %d.",
            word, linenum );
      fread_to_eol( fp );
    }
  }
}

/* load a pet from the forgotten reaches */
void fread_pet( CHAR_DATA *ch, FILE *fp )
{
  char *word;
  CHAR_DATA *pet;
  bool fMatch;
  int vnum=0;
  int lastlogoff = current_time;
  int percent;

  /* first entry had BETTER be the vnum or we barf */
  word = feof(fp) ? "END" : fread_word(fp);
  if (!str_cmp(word,"Vnum"))
  {

    vnum = fread_number(fp);
    if (get_mob_index(vnum) == NULL)
    {
      bug("Fread_pet: bad vnum %d.",vnum);
      pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
    }
    else
      pet = create_mobile(get_mob_index(vnum));
  }
  else
  {
    bug("Fread_pet: no vnum in file.",0);
    pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
  }

  for ( ; ; )
  {
    word     = feof(fp) ? "END" : fread_word(fp);
    fMatch = FALSE;

    switch (UPPER(word[0]))
    {
      case '*':
        fMatch = TRUE;
        fread_to_eol(fp);
        break;

      case 'A':
        KEY( "Act",        pet->act,        fread_flag(fp));
        KEY( "AfBy",    pet->affected_by,    fread_flag(fp));
        KEY( "AfBy2",    pet->affected2_by,    fread_flag( fp ) );
        KEY( "Alig",    pet->alignment,        fread_number(fp));

        if (!str_cmp(word,"ACs"))
        {
          int i;

          for (i = 0; i < 4; i++)
            pet->armor[i] = fread_number(fp);
          fMatch = TRUE;
          break;
        }

        if (!str_cmp(word,"AffD"))
        {
          AFFECT_DATA *paf;
          int sn;

          paf = new_affect();

          sn = skill_lookup_err(fread_word(fp));
          if (sn < 0)
            bug("Fread_char: unknown skill.",0);
          else
            paf->type = sn;

          paf->level    = fread_number(fp);
          paf->duration    = fread_number(fp);
          paf->modifier    = fread_number(fp);
          paf->location    = fread_number(fp);
          paf->bitvector    = fread_number(fp);
          paf->next        = pet->affected;
          pet->affected    = paf;
          fMatch        = TRUE;
          break;
        }

        if (!str_cmp(word,"Affc"))
        {
          AFFECT_DATA *paf;
          int sn;

          paf = new_affect();

          sn = skill_lookup_err(fread_word(fp));
          if (sn < 0)
            bug("Fread_char: unknown skill.",0);
          else
            paf->type = sn;

          paf->where    = fread_number(fp);
          paf->level      = fread_number(fp);
          paf->duration   = fread_number(fp);
          paf->modifier   = fread_number(fp);
          paf->location   = fread_number(fp);
          paf->bitvector  = fread_number(fp);
          if (!check_pet_affected(vnum,paf))
          {
            paf->next       = pet->affected;
            pet->affected   = paf;
          }
          else
          {
            free_affect(paf);
          }
          fMatch          = TRUE;
          break;
        }

        if (!str_cmp(word,"AMod"))
        {
          int stat;

          for (stat = 0; stat < MAX_STATS; stat++)
            pet->mod_stat[stat] = fread_number(fp);
          fMatch = TRUE;
          break;
        }

        if (!str_cmp(word,"Attr"))
        {
          int stat;

          for (stat = 0; stat < MAX_STATS; stat++)
            pet->perm_stat[stat] = fread_number(fp);
          fMatch = TRUE;
          break;
        }
        break;

      case 'C':
        if ( !str_cmp( word, "Clan" ) )
        {
          int i = 0;
          i = fread_number( fp );
          pet->clan_name = str_dup( clan_table[ i ].name, pet->clan_name );
          pet->clan = get_clan( pet->clan_name, TRUE );
          fMatch = TRUE;
          break;
        }

        if ( !strcmp( word, "Clanname" ) )
        {
          pet->clan_name = fread_string( fp );
          pet->clan = get_clan( pet->clan_name, TRUE );
          fMatch = TRUE;
          break;
        }

        KEY( "Comm",    pet->comm_flags,     fread_flag(fp));
        KEY( "Chan",    pet->comm_flags,     fread_flag(fp));
        break;

      case 'D':
        KEY( "Dam",    pet->damroll,        fread_number(fp));
        KEYS( "Desc",    pet->description,    fread_string(fp));
        break;

      case 'E':
        if (!str_cmp(word,"End"))
        {
          pet->leader = ch;
          pet->master = ch;
          pet->group_num = ch->group_num;
          pet->group_fight = 0;
          ch->pet = pet;
          /* adjust hp mana move up  -- here for speed's sake */
          percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);

          if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
              &&  !IS_AFFECTED(ch,AFF_PLAGUE))
          {
            percent = UMIN( percent,100);
            pet->hit      += ( GET_HP( pet ) - pet->hit) * percent / 100;
            pet->mana   += ( GET_MANA( pet ) - pet->mana) * percent / 100;
            pet->move   += ( pet->max_move - pet->move)* percent / 100;
          }
          return;
        }
        KEY( "Exp",    pet->exp,        fread_number(fp));
        break;

      case 'G':
        KEY( "Gold",    pet->gold,        fread_number(fp));
        break;

      case 'H':
        KEY( "Hit",    pet->hitroll,        fread_number(fp));

        if (!str_cmp(word,"HMV"))
        {
          pet->hit    = fread_number(fp);
          pet->max_hit    = fread_number(fp);
          pet->mana    = fread_number(fp);
          pet->max_mana    = fread_number(fp);
          pet->move    = fread_number(fp);
          pet->max_move    = fread_number(fp);
          fMatch = TRUE;
          break;
        }
        break;

      case 'L':
        KEY( "Levl",    pet->level,        fread_number(fp));
        KEYS( "LnD",    pet->long_descr,    fread_string(fp));
        KEY( "LogO",    lastlogoff,        fread_number(fp));
        break;

      case 'N':
        KEYS( "Name",    pet->name,        fread_string(fp));
        break;

      case 'P':
        KEY( "Pos",    pet->position,        fread_number(fp));
        KEY( "Pen",    pet->pen_flags,     fread_flag(fp));
        break;

      case 'R':
        if ( !str_cmp( word, "Race" ) )
        {
          char *tmp = fread_string(fp);
          ch->race = race_lookup(tmp);
          free_string(tmp);
          fMatch = TRUE;
          break;
        }
        break;

      case 'S' :
        KEY( "Save",    pet->saving_throw,    fread_number(fp));
        KEY( "Sex",        pet->sex,        fread_number(fp));
        KEYS( "ShD",        pet->short_descr,    fread_string(fp));
        KEY( "Silv",        pet->silver,            fread_number( fp ) );
        break;

    }
    if ( !fMatch )
    {
      bugf("Fread_pet: no match.");
      fread_to_eol(fp);
    }
  }
}

extern    OBJ_DATA    *obj_free;

void fread_obj( CHAR_DATA *ch, FILE *fp, bool isLocker, bool fCombine )
{
  OBJ_DATA *obj;
  char *word;
  int iNest;
  bool fMatch;
  bool fNest;
  bool fVnum;
  bool first;
  int count;
  bool new_format;  /* to prevent errors */
  bool make_new;    /* update object */
  int combine_count = 1;
  fVnum = FALSE;
  obj = NULL;
  first = TRUE;  /* used to counter fp offset */
  new_format = FALSE;
  make_new = FALSE;

  if (fCombine)
    combine_count = fread_number(fp);

  word   = feof( fp ) ? "End" : fread_word( fp );
  if (!str_cmp(word,"Vnum" ))
  {
    int vnum;
    first = FALSE;  /* fp will be in right place */

    vnum = fread_number( fp );
    if (  get_obj_index( vnum )  == NULL )
    {
      bug( "Fread_obj: bad vnum %d.", vnum );
    }
    else
    {
      obj = create_object(get_obj_index(vnum),-1);
      new_format = TRUE;
    }

  }

  if (obj == NULL)  /* either not found or old style */
  {
    obj = new_obj();
    obj->name        = &str_empty[0];
    obj->short_descr    = &str_empty[0];
    obj->description    = &str_empty[0];
    obj->owner    = &str_empty[0];
    obj->material    = &str_empty[0];
  }

  fNest        = FALSE;
  fVnum        = TRUE;
  iNest        = 0;

  for ( ; ; )
  {
    if (first)
      first = FALSE;
    else
      word   = feof( fp ) ? "End" : fread_word( fp );
    fMatch = FALSE;

    switch ( UPPER(word[0]) )
    {
      case '*':
        fMatch = TRUE;
        fread_to_eol( fp );
        break;

      case 'A':
        if (!str_cmp(word,"AffD"))
        {
          AFFECT_DATA *paf;
          int sn;

          paf = new_affect();

          sn = skill_lookup_err(fread_word(fp));
          if (sn < 0)
            bug("Fread_obj: unknown skill.",0);
          else
            paf->type = sn;

          paf->level    = fread_number( fp );
          paf->duration    = fread_number( fp );
          paf->modifier    = fread_number( fp );
          paf->location    = fread_number( fp );
          paf->bitvector    = fread_number( fp );

          paf->next    = obj->affected;
          obj->affected    = paf;
          fMatch        = TRUE;
          break;
        }
        if (!str_cmp(word,"Affc"))
        {
          AFFECT_DATA *paf;
          int sn;

          paf = new_affect();

          sn = skill_lookup_err(fread_word(fp));
          if (sn < 0)
            bug("Fread_obj: unknown skill.",0);
          else
            paf->type = sn;

          paf->where    = fread_number( fp );
          paf->level      = fread_number( fp );
          paf->duration   = fread_number( fp );
          paf->modifier   = fread_number( fp );
          paf->location   = fread_number( fp );
          paf->bitvector  = fread_number( fp );

          // Old acid etched items now have condition lowered.
          if ( paf->type == gsn_acid_breath
               &&   paf->location == APPLY_AC
               &&   paf->modifier >= 0 )
          {
            obj->condition -= dice( 1, 2 );
            if ( obj->condition < 1 )
              obj->condition = 1;
            free_affect( paf );
            fMatch = TRUE;
            break;
          }

          paf->next       = obj->affected;
          obj->affected   = paf;
          fMatch          = TRUE;
          break;
        }
        break;

      case 'C':
        KEY( "Cond",    obj->condition,        fread_number( fp ) );
        KEY( "Cost",    obj->cost,        fread_number( fp ) );
        break;

      case 'D':
        KEYS( "Description",    obj->description,    fread_string( fp ) );
        KEYS( "Desc",    obj->description,    fread_string( fp ) );
        break;

      case 'E':

        if ( !str_cmp( word, "Enchanted"))
        {
          obj->enchanted = TRUE;
          fMatch     = TRUE;
          break;
        }

        KEY( "ExtraFlags",    obj->extra_flags,    fread_number( fp ) );
        KEY( "ExtF",    obj->extra_flags,    fread_number( fp ) );

        if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe"))
        {
          EXTRA_DESCR_DATA *ed;

          ed = new_extra_descr();

          ed->keyword        = fread_string( fp );
          ed->description        = fread_string( fp );
          ed->next        = obj->extra_descr;
          ed->valid = TRUE;
          obj->extra_descr    = ed;
          fMatch = TRUE;
        }

        if ( !str_cmp( word, "End" ) )
        {
          if ( !fNest || ( fVnum && obj->pIndexData == NULL ) )
          {
            bugf( "Fread_obj: incomplete %sobject",
                  fCombine ? "combined " : "" );
            if ( obj->pIndexData )
              extract_obj( obj );
            else
              free_obj( obj );
            return;
          }

          {
            if ( !fVnum )
            {
              if ( obj->pIndexData )
                extract_obj( obj );
              else
                free_obj( obj );
              obj = create_object( get_obj_index( OBJ_VNUM_DUMMY ), 0 );
              new_format = TRUE;
            }

            if (!new_format)
            {
              obj->next    = object_list;
              object_list    = obj;
              obj->pIndexData->count++;
            }

            if (!obj->pIndexData->new_format
                && obj->item_type == ITEM_ARMOR
                &&  obj->value[1] == 0)
            {
              obj->value[1] = obj->value[0];
              obj->value[2] = obj->value[0];
            }
            if (make_new)
            {
              int wear = obj->wear_loc;
              extract_obj(obj);

              obj = create_object(obj->pIndexData, 0  ); /* ATTENTION AR has something different here.. why? */
              obj->wear_loc = wear;
            }

            /* Combined objects. */
            for ( count = 1; count < combine_count; count++ )
            {
              OBJ_DATA *clone = create_object( obj->pIndexData, 0 );

              clone_object( obj, clone );
              if ( iNest == 0 || rgObjNest[iNest] == NULL )
              {
                if (isLocker)
                  obj_to_locker(clone, ch);
                else
                  obj_to_char( clone, ch );
              }
              else
                obj_to_obj( clone, rgObjNest[iNest-1] );
            }


            if ( iNest == 0 || rgObjNest[iNest] == NULL )
            {
              if (isLocker)
                obj_to_locker( obj, ch );
              else
                obj_to_char( obj, ch );
            }
            else
              obj_to_obj( obj, rgObjNest[iNest-1] );

            return;
          }
        }
        break;

      case 'F':
        KEYS( "Famowner", obj->famowner, fread_string( fp ) );
        break;

      case 'I':
        KEY( "ItemType",    obj->item_type,        fread_number( fp ) );
        KEY( "Ityp",    obj->item_type,        fread_number( fp ) );
        break;

      case 'L':
        KEY( "Level",    obj->level,        fread_number( fp ) );
        KEY( "Lev",        obj->level,        fread_number( fp ) );
        break;

      case 'M':
        KEYS("Mat",        obj->material,        fread_string( fp ) );
        break;

      case 'N':
        KEYS( "Name",    obj->name,        fread_string( fp ) );

        if ( !str_cmp( word, "Nest" ) )
        {
          iNest = fread_number( fp );
          if ( iNest < 0 || iNest >= MAX_NEST )
          {
            bug( "Fread_obj: bad nest %d.", iNest );
          }
          else
          {
            rgObjNest[iNest] = obj;
            fNest = TRUE;
          }
          fMatch = TRUE;
        }
        break;

      case 'O':
        KEYS( "Owner",    obj->owner,    fread_string( fp ) );

        if ( !str_cmp( word,"Oldstyle" ) )
        {
          /*        if (obj->pIndexData != NULL && obj->pIndexData->new_format)
                make_new = TRUE;*/
          /* Commenting out recommended by a file off the rom */
          /* list. */
          fMatch = TRUE;
        }
        break;


      case 'S':
        KEYS( "ShortDescr",    obj->short_descr,    fread_string( fp ) );
        KEYS( "ShD",        obj->short_descr,    fread_string( fp ) );

        if ( !str_cmp( word, "Spell" ) )
        {
          int iValue;
          int sn;

          iValue = fread_number( fp );
          sn     = skill_lookup_err( fread_word( fp ) );
          if ( iValue < 0 || iValue > 6 )
            bug( "Fread_obj: bad iValue %d.", iValue );
          else if ( sn < 0 )
            bug( "Fread_obj: unknown skill.", 0 );
          else
            obj->value[iValue] = sn;
          fMatch = TRUE;
          break;
        }

        break;

      case 'T':
        KEY( "Timer",    obj->timer,        fread_number( fp ) );
        KEY( "Time",    obj->timer,        fread_number( fp ) );
        break;

      case 'V':
        if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals"))
        {
          obj->value[0]    = fread_number( fp );
          obj->value[1]    = fread_number( fp );
          obj->value[2]    = fread_number( fp );
          obj->value[3]    = fread_number( fp );
          if (ch->version >= 9)
          {
            obj->value[5]    = fread_number( fp );
            obj->value[6]    = fread_number( fp );

          }
          if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
            obj->value[0] = obj->pIndexData->value[0];
          fMatch        = TRUE;
          break;
        }

        if ( !str_cmp( word, "Val" ) )
        {
          obj->value[0]     = fread_number( fp );
          obj->value[1]    = fread_number( fp );
          obj->value[2]     = fread_number( fp );
          obj->value[3]    = fread_number( fp );
          obj->value[4]    = fread_number( fp );
          if (ch->version >= 9)
          {
            obj->value[5]    = fread_number( fp );
            obj->value[6]    = fread_number( fp );

          }
          fMatch = TRUE;
          break;
        }

        if ( !str_cmp( word, "Vnum" ) )
        {
          int vnum;

          vnum = fread_number( fp );
          if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
            bug( "Fread_obj: bad vnum %d.", vnum );
          else
            fVnum = TRUE;
          fMatch = TRUE;
          break;
        }
        break;

      case 'W':
        KEY( "WearFlags",    obj->wear_flags,    fread_number( fp ) );
        KEY( "WeaF",    obj->wear_flags,    fread_number( fp ) );
        KEY( "WearLoc",    obj->wear_loc,        fread_number( fp ) );
        KEY( "Wear",    obj->wear_loc,        fread_number( fp ) );
        KEY( "Weight",    obj->weight,        fread_number( fp ) );
        KEY( "Wt",        obj->weight,        fread_number( fp ) );
        break;

    }

    if ( !fMatch )
    {
      bugf( "Fread_obj: no match word %s.", word );
      fread_to_eol( fp );
    }

    // set owner on restrings
    if ( IS_SET(obj->extra_flags, ITEM_RESTRING)
         &&  (obj->owner != &str_empty[0]) )
      obj->owner = str_dup( ch->name, obj->owner );
  }
}

/*
 * Write data to system file.
 */
void fwrite_sys_data( FILE *fp )
{
  int i;

  /* 24 hr count array. */
  fprintf( fp, "CountArr" );
  for ( i = 0; i < 24; i++ )
    fprintf( fp, " %d", countArr[i] );
  fprintf( fp, "\n" );

  fprintf( fp, "CountHr %d\n", countHour );
  fprintf( fp, "MaxDay  %d\n", countMaxDay );
  fprintf( fp, "MaxEver %d\n", countMax );

  /* Day-of-Week array. */
  fprintf( fp, "MaxDow" );
  for ( i = 0; i < 7; i++ )
    fprintf( fp, " %d", countMaxDoW[i] );
  fprintf( fp, "\n" );
  for (i=0; i < 5; i++)
    fprintf(fp, "As%d %s~\n",i,as_string[i]);

  fprintf( fp, "End\n\n" );

}
/*
 * Save system file for copyover.
 */
void save_sys_data( void )
{
  FILE *fp;
  char filestr[MAX_INPUT_LENGTH];

  mprintf(sizeof(filestr), filestr, SYSTEM_FILE, port );
  if (fpReserve)
  {
    fclose( fpReserve );
    nFilesOpen--;
  }
  if ( ( fp = fopen( filestr, "w" ) ) == NULL )
  {
    bug( "Save_sys_data: fopen", 0 );
    perror( filestr );
  }
  else
  {
    nFilesOpen++;
    fwrite_sys_data( fp );
    fclose( fp );
    nFilesOpen--;
  }
  fpReserve = fopen( NULL_FILE, "r" );
  nFilesOpen++;
}


/*
 * Read data from the system file.
 */
void fread_sys_data( FILE *fp )
{
  char *word;
  bool fMatch;
  int  i;

  for ( ; ; )
  {
    word   = feof( fp ) ? "End" : fread_word( fp );
    fMatch = FALSE;
    switch ( UPPER( word[0] ) )
    {
      case 'A':
        if (!str_cmp(word, "As0"))
        {
          char *tmp = fread_string(fp);
          strcpy(as_string[0],tmp);
          free_string(tmp);
          fMatch = TRUE;
          break;
        }
        if (!str_cmp(word, "As1"))
        {
          char *tmp = fread_string(fp);
          strcpy(as_string[1],tmp);
          free_string(tmp);
          fMatch = TRUE;
          break;
        }
        if (!str_cmp(word, "As2"))
        {
          char *tmp = fread_string(fp);
          strcpy(as_string[2],tmp);
          free_string(tmp);
          fMatch = TRUE;
          break;
        }
        if (!str_cmp(word, "As3"))
        {
          char *tmp = fread_string(fp);
          strcpy(as_string[3],tmp);
          free_string(tmp);
          fMatch = TRUE;
          break;
        }
        if (!str_cmp(word, "As4"))
        {
          char *tmp = fread_string(fp);
          strcpy(as_string[4],tmp);
          free_string(tmp);
          fMatch = TRUE;
          break;
        }

        break;
      case 'C':

        KEY( "CountHr",        countHour,    fread_number( fp ) );
        if ( !str_cmp( word, "CountArr" ) )
        {
          for ( i = 0; i < 24 ; i++ )
            countArr[i] = fread_number( fp );
          fMatch = TRUE;
          break;
        }
        break;
      case 'E':
        if ( !str_cmp( word, "End" ) )
        {
          return;
        }
        break;
      case 'M':
        if ( !str_cmp( word, "MaxDoW" ) )
        {
          for ( i = 0; i < 7; i++ )
            countMaxDoW[i] = fread_number( fp );
          fMatch = TRUE;
          break;
        }
        KEY( "MaxDay",        countMaxDay,    fread_number( fp ) );
        KEY( "MaxOn",        countMaxDay,    fread_number( fp ) );
        KEY( "MaxEver",        countMax,    fread_number( fp ) );
        break;
    }

    if ( !fMatch )
    {
      bugf( "Fread_sys_data: no match (%s)", word );
      fread_to_eol( fp );
    }
  }
}


/*
 * Load the system file.
 */
void load_sys_data( void )
{
  FILE *fp;
  char filestr[512];

  mprintf(sizeof(filestr), filestr, SYSTEM_FILE, port );
  log_string("loading System Data...");

  fclose( fpReserve );
  nFilesOpen--;
  if ( ( fp = fopen( filestr, "r" ) ) == NULL )
  {
    bug( "Load_sys_data: fopen", 0 );
    perror( filestr );
  }
  else
  {
    nFilesOpen++;
    fread_sys_data( fp );
    fclose( fp );
    nFilesOpen--;
  }
  fpReserve = fopen( NULL_FILE, "r" );
  nFilesOpen++;
}
