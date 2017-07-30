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
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@efn.org)				   *
*	    Gabrielle Taylor						   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"


int top_buffer=0;
int			*magic;
const int		sizeof_magic = sizeof( *magic );
const int		rgBufSizeList[MAX_BUF_LIST] =
{
  512, 1024, 2048, 4096, 8192, 16384,16384,32768,65536,131072
  /*    512, 1024, 2048, 4096, 8192, 16384*/
};


/* stuff for recyling notes */
NOTE_DATA *note_free;

NOTE_DATA *new_note()
{
  NOTE_DATA *note;

  if (note_free == NULL)
    note = alloc_perm(sizeof(*note));
  else
    { 
      note = note_free;
      note_free = note_free->next;
    }
  VALIDATE(note);
  return note;
}

void free_note(NOTE_DATA *note)
{
  char buf[MSL];
  if (!IS_VALID(note)) {
    mprintf(sizeof(buf),buf,
	    "Memcheck : {RFREE BUF trying to free invalid NOTE{x") ;
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    bugf(buf);
    VALIDATE(note);
  }

  free_string( note->text    );
  free_string( note->subject );
  free_string( note->to_list );
  free_string( note->date    );
  free_string( note->sender  );
  INVALIDATE(note);

  note->next = note_free;
  note_free   = note;
}

    
/* stuff for recycling ban structures */
BAN_DATA *ban_free;

/*
 * Ban structures.
 */
BAN_DATA *new_ban( void )
{
  static BAN_DATA	ban_zero;
  BAN_DATA		*pBan;

  if ( ban_free == NULL )
    {
      pBan		= alloc_perm( sizeof( *pBan ) );
    }
  else
    {
      pBan		= ban_free;
      ban_free	= ban_free->next;
    }

  *pBan = ban_zero;
  VALIDATE( pBan );

  pBan->name		= &str_empty[0];
  pBan->date_stamp	= current_time;

  return pBan;
}

void free_ban( BAN_DATA *pBan )
{
  if ( !IS_VALID( pBan ) )
    return;

  free_string( pBan->name );

  INVALIDATE( pBan );

  pBan->next	= ban_free;
  ban_free	= pBan;
}

/* stuff for recycling descriptors */
DESCRIPTOR_DATA *descriptor_free;

DESCRIPTOR_DATA *new_descriptor(void)
{
  static DESCRIPTOR_DATA d_zero;
  DESCRIPTOR_DATA *d;

  if (descriptor_free == NULL) {
    d = alloc_perm(sizeof(*d));
    /*logit("Allocating descriptor %d",d);*/
  }
  else
    {
      d = descriptor_free;
      descriptor_free = descriptor_free->next;
    }
  nDescsOpen++;
  *d = d_zero;
  VALIDATE(d);
 
  d->connected	= CON_ANSI;
  d->showstr_head	= NULL;
  d->showstr_point = NULL;
  d->outsize	= 2000;
  d->ansi   = FALSE;
  d->outbuf	= alloc_mem( d->outsize );
    
  return d;
}

void free_descriptor(DESCRIPTOR_DATA *d)
{
  char buf[MSL];
  if (!IS_VALID(d)) {
    mprintf(sizeof(buf),buf,
	    "Memcheck : {RFREE BUF trying to free invalid Descriptor{x") ;
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    VALIDATE(d);
  }

  if ( d->showstr_head )
#if OLD_MEM
    free_mem( d->showstr_head, strlen( d->showstr_head ) + 1 );
#else
    free_mem( d->showstr_head);
#endif
    /*    free_mem( d->showstr_head, strlen( d->showstr_size )); */
  free_string( d->host );
#if OLD_MEM
  free_mem( d->outbuf, d->outsize );
#else
  free_mem( d->outbuf);
#endif
    /* MCCP:  Free up compression buffer. */
  /* already done in comm.c */

  INVALIDATE(d);
  d->next = descriptor_free;
  descriptor_free = d;
  nDescsOpen--;
}

/* stuff for recycling gen_data */
GEN_DATA *gen_data_free;

GEN_DATA *new_gen_data(void)
{
  static GEN_DATA gen_zero;
  GEN_DATA *gen;

  if (gen_data_free == NULL)
    gen = alloc_perm(sizeof(*gen));
  else
    {
      gen = gen_data_free;
      gen_data_free = gen_data_free->next;
    }
  *gen = gen_zero;
  VALIDATE(gen);
  return gen;
}

void free_gen_data(GEN_DATA *gen)
{
  if (!IS_VALID(gen))
    return;

  INVALIDATE(gen);

  gen->next = gen_data_free;
  gen_data_free = gen;
} 

/* stuff for recycling extended descs */
EXTRA_DESCR_DATA *extra_descr_free;

EXTRA_DESCR_DATA *new_extra_descr(void)
{
  EXTRA_DESCR_DATA *ed;

  if (extra_descr_free == NULL)
    ed = alloc_perm(sizeof(*ed));
  else
    {
      ed = extra_descr_free;
      extra_descr_free = extra_descr_free->next;
    }

  ed->keyword = &str_empty[0];
  ed->description = &str_empty[0];
  VALIDATE(ed);
  return ed;
}

void free_extra_descr(EXTRA_DESCR_DATA *ed)
{
  char buf[MSL];
  if (!IS_VALID(ed)) {
    mprintf(sizeof(buf),buf,
	    "Memcheck : {RFREE BUF trying to free invalid Extra Descriptor{x") ;
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    VALIDATE(ed);
  }
  free_string(ed->keyword);
  free_string(ed->description);
  INVALIDATE(ed);
    
  ed->next = extra_descr_free;
  extra_descr_free = ed;
}


/* stuff for recycling affects */
AFFECT_DATA *affect_free;

AFFECT_DATA *new_affect(void)
{
  static AFFECT_DATA af_zero;
  AFFECT_DATA *af;

  if (affect_free == NULL)
    af = alloc_perm(sizeof(*af));
  else
    {
      af = affect_free;
      affect_free = affect_free->next;
    }

  *af = af_zero;


  VALIDATE(af);
  return af;
}

void free_affect(AFFECT_DATA *af)
{
  char buf[MSL];
  if (!IS_VALID(af)) {
    mprintf(sizeof(buf),buf,
	    "Memcheck : {RFREE BUF trying to free invalid Affect{x") ;
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    VALIDATE(af);
  }

  INVALIDATE(af);
  af->next = affect_free;
  affect_free = af;
}

/* stuff for recycling objects */
OBJ_DATA *obj_free;

OBJ_DATA *new_obj(void)
{
  static OBJ_DATA obj_zero;
  OBJ_DATA *obj;

  if (obj_free == NULL)
    obj = alloc_perm(sizeof(*obj));
  else
  {
      obj = obj_free;
      obj_free = obj_free->next;
  }

  *obj = obj_zero;
  VALIDATE(obj);

  return obj;
}

void free_obj(OBJ_DATA *obj)
{
  AFFECT_DATA *paf, *paf_next;
  EXTRA_DESCR_DATA *ed, *ed_next;
  char buf[MSL];

  if (!IS_VALID(obj))
  {
    mprintf(sizeof(buf),buf,
	    "Memcheck : {RFREE BUF trying to free invalid Affect{x") ;
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    VALIDATE(obj);

  }

  for (paf = obj->affected; paf != NULL; paf = paf_next)
  {
      paf_next = paf->next;
      VALIDATE(paf);
      free_affect(paf);
  }
  obj->affected = NULL;

  for (ed = obj->extra_descr; ed != NULL; ed = ed_next )
  {
      ed_next = ed->next;
      free_extra_descr(ed);
  }
  obj->extra_descr = NULL;
   
  
  free_string( obj->name        );
  free_string( obj->famowner    );
  free_string( obj->description );
  free_string( obj->short_descr );
  free_string( obj->owner     );
  free_string( obj->material);
  INVALIDATE(obj);
  pObjNum--;
  obj->next   = obj_free;
  obj_free    = obj; 
}


/* stuff for recyling characters */
CHAR_DATA *char_free;

DENIED_DATA *new_denied(void)
{
  static DENIED_DATA denied_zero;
  DENIED_DATA *denied;
  denied = alloc_perm(sizeof(*denied));
  *denied = denied_zero;
  denied->name = &str_empty[0];
  return denied;
}

CHAR_DATA *new_char (void)
{
  static CHAR_DATA ch_zero;
  CHAR_DATA *ch;
  /*char title [MSL];*/
  int i, chan_num, hist_num, cmd_num;

  if (char_free == NULL)
    ch = alloc_perm(sizeof(*ch));
  else
    {
      ch = char_free;
      char_free = char_free->next;
    }

  *ch	            		  = ch_zero;
  VALIDATE(ch);
  ch->name                    = &str_empty[0];
  ch->short_descr             = &str_empty[0];
  ch->long_descr              = &str_empty[0];
  ch->description             = &str_empty[0];
  ch->path                    = &str_empty[0];
  ch->max_riders              = 0;
  ch->prompt                  = &str_empty[0];
#if MEMDEBUG
  ch->memdebug_name	          = &str_empty[0];
  ch->memdebug_prompt	      = &str_empty[0];
#endif
  ch->prefix		          = &str_empty[0];
  ch->logon                   = current_time;
  ch->last_level              = 0;
  ch->lines                   = PAGELEN;
  for (i = 0; i < 4; i++)
    ch->armor[i]              = 100;
  ch->position                = POS_STANDING;
  ch->hit                     = 20;
  ch->max_hit                 = 20;
  ch->mana                    = 100;
  ch->max_mana                = 100;
  ch->move                    = 100;
  ch->max_move                = 100;
  ch->active_flags	          = 0;
  ch->fixed_buffer            = new_buf();
  ch->reset_room              = NULL; // Only valid for mobs -- Merak
  for (i = 0; i < MAX_STATS; i ++)
    {
      ch->perm_stat[i] = 13;
      ch->mod_stat[i] = 0;
    }

  for (chan_num = 0; chan_num < MAX_CHANNEL; chan_num++) 
    {
      for (hist_num = 0; hist_num < MAX_HIST; hist_num++) 
        ch->chan_hist[chan_num][hist_num].text = &str_empty[0];
    }

  for (cmd_num = 0; cmd_num < MAX_COMMAND_HISTORY; cmd_num++)
    {
      ch->cmd_hist[cmd_num].text = &str_empty[0];
    }
 
  return ch;
}

void free_chan_hist(CHAR_DATA *ch)
{
  int i, channel;
  for (channel = 0; channel < MAX_CHANNEL; channel++)
    {
      for (i = 0; i < MAX_HIST; i++)
        {
          free_string(ch->chan_hist[channel][i].text);
        }
    }
}

void free_char_cmd_hist(CHAR_DATA *ch)
{
  int cmd_num;
  for (cmd_num = 0; cmd_num < MAX_COMMAND_HISTORY; cmd_num++)
    {
      free_string(ch->cmd_hist[cmd_num].text);
    }
}

void free_char (CHAR_DATA *ch)
{
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;
  char buf[MSL];
  
  if (!IS_VALID(ch)) {
    mprintf(sizeof(buf),buf,
	    "Memcheck : {RFREE BUF trying to free invalid Character{x") ;
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    VALIDATE(ch);
  }

  if (IS_NPC(ch))
    mobile_count--;
    /* Erwin's suggested fix to light problem */
  ch->in_room = NULL;

  for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      VALIDATE(obj);
      extract_obj(obj);
    }
  for (paf = ch->affected; paf != NULL; paf = paf_next)
    {
      paf_next = paf->next;
      VALIDATE(paf);
      free_affect(paf);
    }

  free_string(ch->name);
  free_string(ch->short_descr);
  free_string(ch->long_descr);
  free_string(ch->description);
  free_string( ch->path );
  free_string(ch->prompt);
#if MEMDEBUG
  free_string(ch->memdebug_name);
  free_string(ch->memdebug_prompt);
#endif
  free_string(ch->prefix);
  free_string(ch->material);
  free_string(ch->tracking);
  if (ch->hate) {
    free_string(ch->hate->name);
    ch->hate->who = NULL;
  }

  clear_buf(ch->fixed_buffer);
  free_buf(ch->fixed_buffer);

  free_chan_hist(ch);
  free_char_cmd_hist(ch);
#if SARTAN_QUEST
  for (i = 0; i < QUEST_STRING_SIZE; i++)
    if (ch->quest_log[i] != NULL)
      free_string(ch->quest_log[i]);
#endif

  if (ch->pnote != NULL)
    free_note(ch->pnote);
  if (ch->pcdata != NULL)
    free_pcdata(ch->pcdata);
  if (ch->questdata)
    free_questdata(ch->questdata);
  if (ch->gen_data)
    free_gen_data(ch->gen_data);
  INVALIDATE(ch);
  ch->next = char_free;
  char_free  = ch;
}

PC_DATA *pcdata_free;

PC_DATA *new_pcdata(void)
{
  int alias;

  static PC_DATA pcdata_zero;
  PC_DATA *pcdata;

  if (pcdata_free == NULL)
    pcdata = alloc_perm(sizeof(*pcdata));
  else    {
    pcdata = pcdata_free;
    pcdata_free = pcdata_free->next;
  }
  *pcdata = pcdata_zero;

  for (alias = 0; alias < MAX_ALIAS; alias++)
    {
      pcdata->alias[alias] = NULL;
      pcdata->alias_sub[alias] = NULL;
    }

  pcdata->history = &str_empty[0];
  pcdata->buffer = new_buf();
    
  VALIDATE(pcdata);
  return pcdata;
}
	

void free_pcdata(PC_DATA *pcdata)
{
  int alias;
  int forg;
  char buf[MSL];
  OBJ_DATA *obj, *obj_next;

  if (!IS_VALID(pcdata)) {
    mprintf(sizeof(buf),buf,
	    "Memcheck : {RFREE BUF trying to free invalid pcdata{x") ;
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    VALIDATE(pcdata);
  }
  for (obj = pcdata->locker; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      obj->carried_by = NULL;
      VALIDATE(obj);
      extract_obj(obj);
    }

  free_string(pcdata->pwd);
  free_string(pcdata->bamfin);
  free_string(pcdata->bamfout);
  free_string(pcdata->host);
#if MEMDEBUG
  free_string(pcdata->memdebug_pwd);
  free_string(pcdata->memdebug_title);
  free_string(pcdata->memdebug_permtitle);
#endif
  free_string(pcdata->title);
  free_string(pcdata->permtitle);
  free_string(pcdata->history);
  free_buf(pcdata->buffer);
  pcdata->balance = 0;    /*     Maniac, for bank */
  pcdata->shares = 0;     /*     Maniac, for bank */
  
  for (alias = 0; alias < MAX_ALIAS; alias++)
    {
      free_string(pcdata->alias[alias]);
      free_string(pcdata->alias_sub[alias]);
    }
  for (forg = 0; forg < MAX_FORGET; forg++)
    {
      free_string(pcdata->forget[forg]);
    }
  INVALIDATE(pcdata);
  pcdata->next = pcdata_free;
  pcdata_free = pcdata;
}

QUEST_DATA *questdata_free;

QUEST_DATA *new_questdata(void)
{
  static QUEST_DATA questdata_zero;
  QUEST_DATA *questdata;

  if (questdata_free == NULL)
    questdata = alloc_perm(sizeof(*questdata));
  else    {
    questdata = questdata_free;
    questdata_free = questdata_free->next;
  }
  *questdata = questdata_zero;

  questdata->log = new_buf();
    
  VALIDATE(questdata);
  return questdata;
}
	

void free_questdata(QUEST_DATA *questdata)
{
  char buf[MSL];

  if (!IS_VALID(questdata)) {
    mprintf(sizeof(buf),buf,"Memcheck : {RFREE BUF trying to free invalid questdata{x") ;
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    VALIDATE(questdata);
  }

  free_buf(questdata->log);
  INVALIDATE(questdata);
  questdata->next = questdata_free;
  questdata_free = questdata;
}

	


/* stuff for setting ids */
long	last_pc_id;
long	last_mob_id;

long get_pc_id(void)
{
  int val;

  val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
  last_pc_id = val;
  return val;
}

long get_mob_id(void)
{
  return(++last_mob_id);
}

MEM_DATA *mem_data_free;

/* procedures and constants needed for buffering */

BUFFER *buf_free;

MEM_DATA *new_mem_data(void)
{
  MEM_DATA *memory;
  
  if (mem_data_free == NULL)
    memory = alloc_mem(sizeof(*memory));
  else
    {
      memory = mem_data_free;
      mem_data_free = mem_data_free->next;
    }

  memory->next = NULL;
  memory->id = 0;
  memory->reaction = 0;
  memory->when = 0;
  VALIDATE(memory);

  return memory;
}

void free_mem_data(MEM_DATA *memory)
{
  char buf[MSL];
  if (!IS_VALID(memory)) {
    mprintf(sizeof(buf),buf,
	    "Memcheck : {RFREE BUF trying to free invalid Memory{x") ;
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    VALIDATE(memory);
  }
  INVALIDATE( memory);

  memory->next = mem_data_free;
  mem_data_free = memory;
}



/* buffer sizes */
const int buf_size[MAX_BUF_LIST] =
{
  16,32,64,128,256,1024,2048,4096,8192,16384,32768,65536,
};

/*
 * Local function for finding the next acceptable size.
 * -1 indicates an out-of-boundary error.
 */
int get_size( int val )
{
    sh_int i;

    for ( i = 0; i < MAX_BUF_LIST; i++ )
	if ( rgBufSizeList[i] - sizeof_magic >= val )
	    return rgBufSizeList[i] - sizeof_magic;

    return -1;
}

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int old_get_size (int val)
{
  int i;

  for (i = 0; i < MAX_BUF_LIST; i++)
    if (buf_size[i] >= val)
      {
	return buf_size[i];
      }
    
  return -1;
}

/*
 * Create a new buffer.
 */
BUFFER *new_buf()
{
  BUFFER *buffer;

  if ( buf_free == NULL )
    {
      buffer		= alloc_perm( sizeof( *buffer ) );
      top_buffer++;
    }
  else
    {
      buffer		= buf_free;
      buf_free	= buf_free->next;
    }

  VALIDATE( buffer );
  buffer->next	= NULL;
  buffer->state	= BUFFER_SAFE;
  buffer->size	= get_size( BASE_BUF );

  buffer->string	= alloc_mem( buffer->size );
  buffer->string[0]	= '\0';
  buffer->len 	= 0;

  return buffer;
}
BUFFER *old_new_buf()
{
  BUFFER *buffer;

  if (buf_free == NULL) 
    buffer = alloc_perm(sizeof(*buffer));
  else
    {
      buffer = buf_free;
      buf_free = buf_free->next;
    }

  buffer->next	= NULL;
  buffer->state	= BUFFER_SAFE;
  buffer->size	= get_size(BASE_BUF);

  buffer->string	= alloc_mem(buffer->size);
  buffer->string[0]	= '\0';
  VALIDATE(buffer);

  return buffer;
}

BUFFER *new_buf_size(int size)
{
  BUFFER *buffer;
 
  if (buf_free == NULL)
    buffer = alloc_perm(sizeof(*buffer));
  else
    {
      buffer = buf_free;
      buf_free = buf_free->next;
    }
 
  buffer->next        = NULL;
  buffer->state       = BUFFER_SAFE;
  buffer->size        = get_size(size);
  if (buffer->size == -1)
    {
      bug("new_buf: buffer size %d too large.",size);
      exit(1);
    }
  buffer->string      = alloc_mem(buffer->size);
  buffer->string[0]   = '\0';
  VALIDATE(buffer);
 
  return buffer;
}

/*
 * Free buffer from memory.
 */
void free_buf( BUFFER *buffer )
{
  if ( !IS_VALID( buffer ) )
    return;

#if OLD_MEM
  free_mem( buffer->string, buffer->size );
#else
  free_mem( buffer->string);
#endif
  buffer->string	= NULL;
  buffer->size	= 0;
  buffer->len 	= 0;
  buffer->state	= BUFFER_FREED;
  INVALIDATE( buffer );

  buffer->next	= buf_free;
  buf_free 	= buffer;
}

void old_free_buf(BUFFER *buffer)
{

  char buf[MSL];
  if (!IS_VALID(buffer)) {
    mprintf(sizeof(buf),buf,
	    "Memcheck : {RFREE BUF trying to free invalid Buffer{x") ;
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    VALIDATE(buffer);
  }

#if OLD_MEM
  free_mem(buffer->string,buffer->size);
#else
  free_mem(buffer->string);
#endif
  buffer->string = NULL;
  buffer->size   = 0;
  buffer->state  = BUFFER_FREED;
  INVALIDATE(buffer);

  buffer->next  = buf_free;
  buf_free      = buffer;
}


bool old_add_buf(BUFFER *buffer, char *string)
{
  int len;
  char *oldstr;
  int oldsize;

  oldstr = buffer->string;
  oldsize = buffer->size;

  if (buffer->state == BUFFER_OVERFLOW) /* don't waste time on bad strings! */
    return FALSE;

  len = strlen(buffer->string) + strlen(string) + 1;

  while (len >= buffer->size) /* increase the buffer size */
    {
      buffer->size 	= get_size(buffer->size + 1);
      {
	if (buffer->size == -1) /* overflow */
	  {
	    buffer->size = oldsize;
	    buffer->state = BUFFER_OVERFLOW;
	    bug("buffer overflow past size %d",buffer->size);
	    return FALSE;
	  }
      }
    }

  if (buffer->size != oldsize)
    {
      buffer->string	= alloc_mem(buffer->size);

      strcpy(buffer->string,oldstr);
#if OLD_MEM
      free_mem(oldstr,oldsize);
#else
      free_mem(oldstr );
#endif
    }

  strcat(buffer->string,string);
  return TRUE;
}

bool add_buf(BUFFER *buffer, char *string)
{
  char *new_str;
  int  new_size;
  int  str_len;

  /* Don't process these cases. */
  if ( buffer == NULL || string == NULL || string[0] == '\0' )
    return FALSE;

  /* Don't waste time on overflowed strings. */
  if ( buffer->state == BUFFER_OVERFLOW )
    return FALSE;

  if ( buffer->state == BUFFER_FREED ) 
    {
      bugf("Buffer is freed in Add_buf.\n\r");
      return FALSE;
    }

  str_len = strlen( string );

  /* Error happening, dont know why spo wll add a protection here */
  if (IS_NULLSTR(buffer->string) && buffer->len)
    {
      buffer->len = 0;
      bugf("ADD_BUF: buffer string size error.\n\r");
    }
  /* Increase buffer size if necessary. */
  if ( buffer->len + str_len + 1 >= buffer->size )
  {
            //printf("--Ouch--: String passed in was -%s-\n\r",string);
    if ( ( new_size = get_size( buffer->len + str_len + 1 ) ) == -1 )
	{
	  buffer->state = BUFFER_OVERFLOW;
	  bugf( "Bstrcat: buffer overflowed with %d bytes",
		buffer->len + str_len );
	  	  //printf("Ouch: String passed in was -%s-\n\r",string);
	  return FALSE;
	}

      /* Allocate space for new buffer. */
      new_str 	= alloc_mem( new_size );

      /* Copy old buffer into new and deallocate old. */
      memcpy( new_str, buffer->string, buffer->len );
#if OLD_MEM
      free_mem( buffer->string, buffer->size );
#else
      free_mem( buffer->string );
#endif
      buffer->string	= new_str;
      buffer->size	= new_size;
    }

  /* Copy str onto end of buffer. */
  memcpy( buffer->string + buffer->len, string, str_len );

  /* Update length and null terminate. */
  buffer->len += str_len;
  buffer->string[buffer->len] = '\0';
  return TRUE;
}

/*
 * Strcat to a buffer, increasing memory allocation if necessary.
 */
void bstrcat( BUFFER *buffer, const char *str )
{
  char *new_str;
  int  new_size;
  int  str_len;

  /* Don't process these cases. */
  if ( buffer == NULL || str == NULL || str[0] == '\0' )
    return;

  /* Don't waste time on overflowed strings. */
  if ( buffer->state == BUFFER_OVERFLOW )
    return;

  str_len = strlen( str );

  /* Increase buffer size if necessary. */
  if ( buffer->len + str_len + 1 >= buffer->size )
  {
      if ( ( new_size = get_size( buffer->len + str_len + 1 ) ) == -1 )
	  {
	    buffer->state = BUFFER_OVERFLOW;
	    bug( "Bstrcat: buffer overflowed with %d bytes",
	       buffer->len + str_len );
	    printf("Ouch 3: String passed in was %s\n\r",str);
	    return;
	  }

      /* Allocate space for new buffer. */
      new_str 	= alloc_mem( new_size );

      /* Copy old buffer into new and deallocate old. */
      memcpy( new_str, buffer->string, buffer->len );
#if OLD_MEM
      free_mem( buffer->string, buffer->size );
#else
      free_mem( buffer->string );
#endif
      buffer->string	= new_str;
      buffer->size	= new_size;
  }

  /* Copy str onto end of buffer. */
  memcpy( buffer->string + buffer->len, str, str_len );

  /* Update length and null terminate. */
  buffer->len += str_len;
  buffer->string[buffer->len] = '\0';
}

/*
 * Clear buffer to an empty string.
 */
void clear_buf( BUFFER *buffer )
{
  if (buffer->state == BUFFER_FREED) 
  {
    buffer=new_buf();
    return;
  }

  free_buf(buffer);
  buffer=new_buf();
  return;
  /*

  buffer->string[0]	= '\0';
  buffer->len 	= 0;
  buffer->state	= BUFFER_SAFE;
  */
}

void old_clear_buf(BUFFER *buffer)
{
  buffer->string[0] = '\0';
  buffer->state     = BUFFER_SAFE;
}

/*
 * Sprintf and add to end of buffer.
 */
int bprintf( BUFFER *buffer, char *fmt, ... )
{
  char	buf[MAX_STRING_LENGTH];
  va_list	args;
  int 	result;

  if ( fmt == NULL )
    {
      bug( "Bprintf: null format", 0 );
      return 0;
    }

  va_start( args, fmt );
  result = vsnprintf( buf, MAX_STRING_LENGTH, fmt, args );
  va_end( args );

  if ( result < 0 )
    bugf( "Bprintf: string overflowed MAX_STRING_LENGTH" );

  bstrcat( buffer, buf );

  return result;
}

char *buf_string(BUFFER *buffer)
{
  return buffer->string;
}

RANK_DATA *rank_free;

RANK_DATA *new_rank( void )
{
    static RANK_DATA    rank_zero;
    RANK_DATA           *rank;

    if ( rank_free == NULL )
        rank = alloc_perm( sizeof( *rank ) );
    else
    {
        rank = rank_free;
        rank_free = rank_free->next;
    }

    *rank               = rank_zero;
    rank->number        = 0;
    rank->male          = &str_empty[0];
    rank->female        = &str_empty[0];
    rank->neutral       = &str_empty[0];
    rank->rank_flags    = 0;

    return rank;
}

void free_rank( RANK_DATA *rank )
{
    free_string( rank->male     );
    free_string( rank->female   );
    free_string( rank->neutral  );

    rank->next = rank_free;
    rank_free  = rank;
}


ROSTER_DATA *roster_free;

ROSTER_DATA *new_clannie( void )
{
    static ROSTER_DATA clannie_zero;
    ROSTER_DATA       *clannie;
    
    if ( roster_free == NULL )
        clannie = alloc_perm(sizeof(*clannie));
    else
    {
        clannie = roster_free;
        roster_free = roster_free->next;
    }

    *clannie = clannie_zero;

    clannie->name       = &str_empty[0];
    clannie->title      = &str_empty[0];
    clannie->afk_title  = &str_empty[0];
    clannie->rank_symbol= &str_empty[0];
    clannie->race       = &str_empty[0];
    clannie->pc_class   = &str_empty[0];
    clannie->donated    = 0;
    clannie->logon      = 0;
    clannie->kills      = 0;
    clannie->deaths     = 0;
    clannie->pkills     = 0;
    clannie->pdeaths    = 0;
    clannie->promoted   = 0;
    clannie->guilded    = 0;

    clannie->next  = NULL;
    return clannie;
}

void free_clannie( ROSTER_DATA *clannie )
{
    free_string( clannie->name          );
    free_string( clannie->title         );
    free_string( clannie->rank_symbol   );

    clannie->next = roster_free;
    roster_free   = clannie;
}

/* stuff for recycling mobprograms */
MPROG_LIST *mprog_free;

MPROG_LIST *new_mprog(void)
{
  static MPROG_LIST mp_zero;
  MPROG_LIST *mp;

  if (mprog_free == NULL)
    mp = alloc_perm(sizeof(*mp));
  else
    {
      mp = mprog_free;
      mprog_free=mprog_free->next;
    }

  *mp = mp_zero;
  mp->vnum             = 0;
  mp->trig_type        = 0;
  mp->code             = NULL; // str_dup("", mp->code);
  VALIDATE(mp);
  return mp;
}

void free_mprog(MPROG_LIST *mp)
{
  char buf[MSL];
  if (!IS_VALID(mp)) {
    mprintf(sizeof(buf),buf,
	    "Memcheck : {RFREE BUF trying to free invalid MObProg{x") ;
    wiznet(buf, NULL, NULL, WIZ_MEMCHECK,0,0) ;
    VALIDATE(mp);
  }
  free_string(mp->trig_phrase);
  INVALIDATE(mp);
  mp->next = mprog_free;
  mprog_free = mp;
}

/*
 * Return length of string in buffer.
 */
int bstrlen( BUFFER *buffer )
{
  return buffer->len;
}
