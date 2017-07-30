#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "interp.h"


void act_spam( const char *format, CHAR_DATA *ch, const void *arg1,
               const void *arg2, int type, int min_pos, int flags )
{
  CHAR_DATA *to;
  CHAR_DATA *vch = (CHAR_DATA *) arg2;
  CHAR_DATA *vic;
  char msg[MAX_STRING_LENGTH];

  /* Discard null and zero-length messages. */
  if ( format == NULL || format[0] == '\0' )
    return;

  /* Discard null characters and null rooms. */
  if ( !ch)
    return;

  if (! ch->in_room)
    return;
  /* Global or area specific message. */
  /* HAS BEEN MODIFIED TO ONLY WORK WITH GOCIALS...
     CONSIDER FIXING THIS.  BY ADDING NEW ACT that CAN PASS
     EXCLUDING CERTAIN COMMS.  */
  if ( type == TO_WORLD || type == TO_AREA )
  {
      DESCRIPTOR_DATA *d;

      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        to = d->character;
        if ( d->connected == CON_PLAYING && to != NULL && to != ch
        &&   ( type != TO_AREA || ch->in_room == to->in_room )
        &&   to->position >= min_pos
        &&   to != vch
        &&   !IS_SET(vch->chan_flags,CHANNEL_QUIET)
        &&   !IS_SET(vch->chan_flags,CHANNEL_NOGOSSIP))

        {
          format_act( msg, format, ch, to, arg1, arg2 );
          vic = d->character;
          send_to_char(msg, vic );
        }
      }
      return;
    }

  /* Group or group in room message. */
  if ( type == TO_GROUP || type == TO_GROUP_ROOM )
  {
      DESCRIPTOR_DATA *d;

      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        to = d->character;
        if ( d->connected == CON_PLAYING && to != NULL && to != ch
        &&   is_same_group( ch, to )
        &&   ( type != TO_GROUP_ROOM || ch->in_room == to->in_room )
        &&   to->position >= min_pos )
        {
          format_act( msg, format, ch, to, arg1, arg2 );
          vic = d->character;
          send_to_char(msg, vic );
        }
    }

    return;
  }

  /* Room specific, other message types. */
  to = ch->in_room->people;

  if ( type == TO_VICT )
  {
    if ( vch == NULL )
    {
      bugf( "Act_nospam: null vch with TO_VICT at [%d]. ch:-%s-,format = -%s-, %s",
           ch->in_room->vnum, ch->name, format, interp_cmd);

      return;
    }

    if ( vch->in_room == NULL )
        return;

    to = vch->in_room->people;
  }

  for ( ; to != NULL; to = to->next_in_room )
  {
      if (to == to->next_in_room)
      {
        bugf("Comm.c: Act: nxtroom error ch:[%s]  %s",ch->name, interp_cmd);

        to = ch;
      }

      if ( (to->desc == NULL
      &&  IS_NPC( to )
      && !HAS_TRIGGER( to, TRIG_ACT ) )
      ||    to->position < min_pos
      ||    to == NULL)
        continue;

      if ( type == TO_CHAR && to != ch )
        continue;

      if ( ( type == TO_VICT || type == TO_WIZ_VICT )
      &&   ( to != vch || to == ch ) )
        continue;

      if ( to == ch && ( type == TO_ROOM || type == TO_WIZ_ROOM ) )
        continue;

      if ( ( to == ch || to == vch )
      &&   ( type == TO_NOTVICT || type == TO_WIZ_NOTVICT ) )
        continue;

      /* Silent if wizinvis. */
      if ( ( type == TO_WIZ_ROOM || type == TO_WIZ_NOTVICT
      ||     type == TO_WIZ_VICT )
      &&   get_trust( to ) < ch->invis_level )
        continue;


      if ( flags != 0
      &&   !IS_NPC( to ) )
      {
        if ( IS_SET( flags, NOSPAM_SMISS )
        &&   IS_SET( to->nospam, NOSPAM_SMISS ) )
            continue;

        if ( IS_SET( flags, NOSPAM_OMISS )
        &&   IS_SET( to->nospam, NOSPAM_OMISS ) )
            continue;

        if ( IS_SET( flags, NOSPAM_SHIT )
        &&   IS_SET( to->nospam, NOSPAM_SHIT ) )
            continue;

        if ( IS_SET( flags, NOSPAM_OHIT )
        &&   IS_SET( to->nospam, NOSPAM_OHIT ) )
            continue;

        if ( IS_SET( flags, NOSPAM_SEFFECTS )
        &&   IS_SET( to->nospam, NOSPAM_SEFFECTS ) )
            continue;

        if ( IS_SET( flags, NOSPAM_OEFFECTS )
        &&   IS_SET( to->nospam, NOSPAM_OEFFECTS ) )
            continue;

        if ( IS_SET( flags, NOSPAM_SPARRY )
        &&   IS_SET( to->nospam, NOSPAM_SPARRY ) )
            continue;

        if ( IS_SET( flags, NOSPAM_OPARRY )
        &&   IS_SET( to->nospam, NOSPAM_OPARRY ) )
            continue;

        if ( IS_SET( flags, NOSPAM_SDODGE )
        &&   IS_SET( to->nospam, NOSPAM_SDODGE ) )
            continue;

        if ( IS_SET( flags, NOSPAM_ODODGE )
        &&   IS_SET( to->nospam, NOSPAM_ODODGE ) )
            continue;

      }


      format_act( msg, format, ch, to, arg1, arg2 );

      if ( to->desc != NULL )
      {
        send_to_char(msg, to );
      }

      /*        write_to_buffer( to->desc, buf, point - buf );*/

      /* probably should make HAS_TRIGGER return false if not IS_NPC */
      if ( MOBtrigger && IS_NPC( to ) )
        mp_act_trigger( msg, to, ch, arg1, arg2, TRIG_ACT );
    /*      if ( MOBtrigger && IS_NPC( to ) && HAS_TRIGGER(to, TRIG_ACT) )
     mp_act_trigger( msg, to, ch, arg1, arg2, TRIG_ACT ); */

  }
  return;
}

void mssp_request( DESCRIPTOR_DATA *d )
{
  char buf[MSL];
  int count, mcount, ocount, rcount, pcount, xcount, i;
  DESCRIPTOR_DATA *ds;
  AREA_DATA *pArea;
  HELP_DATA *pHelp;
  MOB_INDEX_DATA *pMobIndex;
  OBJ_INDEX_DATA *pObjIndex;
  ROOM_INDEX_DATA *pRoomIndex;
  RESET_DATA *pReset;
  MPROG_LIST *pMprog;

  count = 0;
  for ( ds = descriptor_list; ds; ds = ds->next )
  {
    if (ds->character)
    {
      if ( ds->connected != CON_PLAYING)
        continue;
      if (ds->character->invis_level > 0)
        continue;
      if (ds->character->incog_level > 0)
        continue;
      count++;
    }
  }

  send_to_desc("\r\nMSSP-REPLY-START\r\n", d);
  send_to_desc("NAME\tThe Lands of Draknor\r\n", d);

  sprintf( buf,"PLAYERS\t%d\r\n", count );
  send_to_desc(buf, d);
  sprintf( buf,"UPTIME\t%lld\r\n", (long long int) tml_boot_time );  // Unix time value of startup
  send_to_desc(buf, d);

  send_to_desc("HOSTNAME\twww.landsofdraknor.net\r\n", d);

  sprintf( buf,"PORT\t%d\r\n", port );
  send_to_desc(buf, d);
  send_to_desc("CODEBASE\tSacred 1.0\r\n", d);
  send_to_desc("CREATED\t2001\r\n", d);
  send_to_desc("ICON\thttp://www.landsofdraknor.net/mud/images/icon32.bmp\r\n", d);
  send_to_desc("IP\t68.187.39.218\r\n", d);
  send_to_desc("LANGUAGE\tEnglish\r\n", d);
  send_to_desc("LOCATION\tUS\r\n", d);
  send_to_desc("MINIMUM AGE\t0\r\n", d);
  send_to_desc("WEBSITE\thttp://www.landsofdraknor.net/\r\n", d);
  send_to_desc("FAMILY\tDikuMUD\r\n", d);
  send_to_desc("GENRE\tFantasy\r\n", d);
  send_to_desc("GAMEPLAY\tAdventure\r\n", d);
  send_to_desc("GAMESYSTEM\tD&D\r\n", d);
  send_to_desc("INTERMUD\t\r\n", d);
  send_to_desc("STATUS\tLive\r\n", d);
  send_to_desc("SUBGENRE\tMedieval Fantasy\r\n", d);

  count = 0;
  for ( pArea = area_first; pArea; pArea = pArea->next ) { if ( (pArea->low_range > -1 ) && (pArea->high_range > -1 ) ) { count++; } }
  sprintf( buf,"AREAS\t%d\r\n", count );
  send_to_desc(buf, d);

  count = 0;
  for ( pHelp = help_first; pHelp ; pHelp = pHelp->next ) { if (pHelp->level < 101 ) { count++; } }
  sprintf( buf,"HELPFILES\t%d\r\n", count );
  send_to_desc(buf, d);

  mcount = 0;
  ocount = 0;
  rcount = 0;
  pcount = 0;
  xcount = 0;

  for (i=0; i<=32767; i++)
  {
    if ((pMobIndex = get_mob_index( i )) != NULL)
    {
      mcount++;
      for ( pMprog = pMobIndex->mprogs; pMprog; pMprog = pMprog->next )
        pcount++;
    }

    if ((pObjIndex = get_obj_index( i )) != NULL)
    {
      ocount++;
      for ( pMprog = pObjIndex->mprogs; pMprog; pMprog = pMprog->next )
        pcount++;
    }

    if ((pRoomIndex = get_room_index( i )) != NULL)
    {
      rcount++;
      for ( pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next )
        xcount++;
    }
  }

  sprintf( buf,"MOBILES\t%d\r\n", mcount );
  send_to_desc(buf, d);
  sprintf( buf,"OBJECTS\t%d\r\n", ocount );
  send_to_desc(buf, d);
  sprintf( buf,"ROOMS\t%d\r\n", rcount );
  send_to_desc(buf, d);
  sprintf( buf,"RESETS\t%d\r\n", xcount );
  send_to_desc(buf, d);
  sprintf( buf,"MUDPROGS\t%d\r\n", pcount );
  send_to_desc(buf, d);
  sprintf( buf,"MUDTRIGS\t%d\r\n", pcount );
  send_to_desc(buf, d);

  send_to_desc("CLASSES\t10\r\n", d);
  send_to_desc("LEVELS\t100\r\n", d);

  sprintf( buf,"RACES\t%d\r\n", (MAX_PC_RACE-1) );
  send_to_desc(buf, d);
  sprintf( buf,"SKILLS\t%d\r\n", (MAX_SKILL-1) );
  send_to_desc(buf, d);

  send_to_desc("WORLDS\t1\r\n", d);
  send_to_desc("ANSI\t1\r\n", d);
  send_to_desc("MCCP\t1\r\n", d);
  send_to_desc("MCP\t1\r\n", d);
  send_to_desc("MSP\t0\r\n", d);
  send_to_desc("SSL\t0\r\n", d);
  send_to_desc("MXP\t0\r\n", d);
  send_to_desc("PUEBLO\t0\r\n", d);
  send_to_desc("VT100\t1\r\n", d);
  send_to_desc("XTERM 256 COLORS\t0\r\n", d);
  send_to_desc("PAY TO PLAY\t0\r\n", d);
  send_to_desc("PAY FOR PERKS\t0\r\n", d);
  send_to_desc("HIRING BUILDERS\t1\r\n", d);
  send_to_desc("HIRING CODERS\t0\r\n", d);
  send_to_desc("MSSP-REPLY-END\r\n", d);
  return;
}
