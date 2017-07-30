#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <time.h>
#include "merc.h"
#include <math.h>
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "interp.h"
#include "olc.h"
#include "genlist.h"

void do_nospam( CHAR_DATA *ch, char *argument )
{
    BUFFER  *buffer;
    long    flags;
    int     i, col = 0;
    char    arg[MSL];

    argument = one_argument( argument, arg );
    buffer = new_buf( );

    if ( IS_NPC( ch ) )
        return;

    if ( IS_NULLSTR( arg ) )
    {
        bprintf( buffer, "Nospam Options:\n\r" );

        for ( i = 0; nospam_flags[i].name; i++)
        {

            bprintf( buffer, "%-14s{g %-6s{x", nospam_flags[i].name,
               IS_SET( ch->nospam, nospam_flags[i].bit)
                    ? "{gON{x    " : "{rOFF{x   " );
            col++;

            if ( col == 2 )
            {
                bstrcat( buffer, "\n\r" );
                col = 0;
            }
      
       } 
       /* To avoid color bleeding */
       bstrcat( buffer, "{x\n\r" );

       page_to_char( buf_string( buffer ), ch );
       free_buf( buffer );
       return;
        
    }

    if ( ( flags = flag_new_value( ch, nospam_flags, arg,
            ch->nospam ) ) != NO_FLAG )
    {
        ch->nospam = flags;

        for ( i = 0; nospam_flags[i].name; i++)
        {

            bprintf( buffer, "%-14s{g %-3s   {x", nospam_flags[i].name,
               IS_SET( ch->nospam, nospam_flags[i].bit)
                    ? "{gON{x " : "{rOFF{x" );
            col++;

            if (col==2)
            {
                bstrcat( buffer, "\n\r" );
                col=0;
            }

       }
       /* To avoid color bleeding */
       bstrcat( buffer, "\n\r{x" );

    }

    page_to_char( buf_string( buffer ), ch );
    free_buf( buffer );

}

void do_toggle( CHAR_DATA *ch, char *argument )
{
    BUFFER  *buffer;
    long    flags;
    int     i, col = 0;
    char    arg[MSL];

    buffer = new_buf( );
    argument = one_argument( argument, arg );

    if ( IS_NULLSTR( arg ) )
    {
        bprintf( buffer, "Toggle Options:\n\r" );

        for ( i = 0; toggle_flags[i].name; i++)
        {

            bprintf( buffer, "{c%-14s{g %-6s{x", toggle_flags[i].name,
               IS_SET( ch->toggle, toggle_flags[i].bit)
                    ? "{gON{x    " : "{rOFF{x   " );
            col++;

            if ( col == 2 )
            {
                bstrcat( buffer, "\n\r" );
                col = 0;
            }

       }
       /* To avoid color bleeding */
       bstrcat( buffer, "{x" );

       page_to_char( buf_string( buffer ), ch );
       free_buf( buffer );
       return;

    }

    if ( ( flags = flag_new_value( ch, toggle_flags, arg,
            ch->toggle ) ) != NO_FLAG )
    {
        ch->toggle = flags;

        for ( i = 0; toggle_flags[i].name; i++)
        {

            bprintf( buffer, "{c%-14s{g %-3s   {x", toggle_flags[i].name,
               IS_SET( ch->toggle, toggle_flags[i].bit)
                    ? "{gON{x " : "{rOFF{x" );

            col++;

            if (col==2)
            {
                bstrcat( buffer, "\n\r" );
                col=0;
            }

       }
       /* To avoid color bleeding */
       bstrcat( buffer, "{x" );

    }

    page_to_char( buf_string( buffer ), ch );
    free_buf( buffer );

}
    


void do_freport( CHAR_DATA *ch, char *argument ) {
// should make DEFINES for frag maxes
  CHAR_DATA *gch;
  char buf[MAX_INPUT_LENGTH];
  char arg[MIL];

  if ( IS_NPC( ch ) ) { return; }
  if ( ch->level < 80 ) {
    printf_to_char(ch,"You can't use fragments yet.\n\r" );
    return;
  }

  argument = one_argument( argument, arg );

  if (arg[0] == '\0')
  {
    if ( ch->position > POS_SLEEPING )
    {
      mprintf(sizeof(buf), buf,
        "{WI have {D<{R%d.%02d {rR{Ru{rby{D> <{C%d.%02d {cSap{Bp{bhi{cre{D> <{G%d.%02d {gE{Gme{gr{Ga{gld{D> <{C%d.%02d {CDi{cam{Wo{wnd{D>{x",
        ch->ruby_counter,     (ch->ruby_fragment     * 100 / 250000),
        ch->sapphire_counter, (ch->sapphire_fragment * 100 / 200000),
        ch->emerald_counter,  (ch->emerald_fragment  * 100 / 150000),
        ch->diamond_counter,  (ch->diamond_fragment  * 100 / 100000) );

        do_function(ch, &do_say, buf);
    }
    else
    {
      send_to_char( "In your dreams, or what?\n\r", ch );
      return;
    }
  }
  else
  { // there is an argument
    int argfound = 0;

    if ( !str_prefix( arg, "group" ) )
    {
      argfound = 1;

      if ( IS_SET( ch->chan_flags, CHANNEL_NOTELL ) )
      {
        send_to_char( "Your message didn't get through!\n\r", ch );
        return;
      }

      if (IS_SET(ch->chan_flags,CHANNEL_QUIET))
      {
        send_to_char("You must turn off quiet mode first.\n\r",ch);
        return;
      }
      mprintf(sizeof(buf), buf,
        "{WI have {D<{R%d.%02d {rR{Ru{rby{D> <{C%d.%02d {cSap{Bp{bhi{cre{D> <{G%d.%02d {gE{Gme{gr{Ga{gld{D> <{C%d.%02d {CDi{cam{Wo{wnd{D>{x",
        ch->ruby_counter,     (ch->ruby_fragment     * 100 / 250000),
        ch->sapphire_counter, (ch->sapphire_fragment * 100 / 200000),
        ch->emerald_counter,  (ch->emerald_fragment  * 100 / 150000),
        ch->diamond_counter,  (ch->diamond_fragment  * 100 / 100000) );
      do_function(ch, &do_gtell, buf);

    } // if 'group' arg


    if ( !str_prefix( arg, "tell" ) ) //This will search all 3.
    {
      if ( ch->position > POS_SLEEPING )
      {

        argfound = 1;

        if ( IS_SET( ch->chan_flags, CHANNEL_NOTELL ) ) {
          send_to_char( "Your message didn't get through!\n\r", ch );
          return;
        }

        if (IS_SET(ch->chan_flags,CHANNEL_QUIET)) {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }

        one_argument( argument, arg );

        if ( ( gch = get_char_world( ch, arg ) ) == NULL ) {
          printf_to_char(ch,"They cannot be found.{x\n\r");
          return;
        } else {
          if ( IS_NPC( gch ) ) {
            send_to_char( "They cannot be found!\n\r", ch );
            return;
          }

          if ( ( IS_SET( gch->chan_flags, CHANNEL_NOTELL ) ) || ( IS_SET( gch->chan_flags,CHANNEL_QUIET ) ) ){
            send_to_char( "Your message didn't get through!\n\r", ch );
            return;
          }

          /*printf_to_char(ch,
            "{gYou tell %s '{WI have {D<{R%d.%02d {rR{Ru{rby{D> <{C%d.%02d {cSap{Bp{bhi{cre{D> <{G%d.%02d {gE{Gme{gr{Ga{gld{D> <{C%d.%02d {CDi{cam{Wo{wnd{D>{g'{x\n\r",
            gch->name,
            ch->ruby_counter,     (ch->ruby_fragment     * 100 / 250000),
            ch->sapphire_counter, (ch->sapphire_fragment * 100 / 200000),
            ch->emerald_counter,  (ch->emerald_fragment  * 100 / 150000),
            ch->diamond_counter,  (ch->diamond_fragment  * 100 / 100000) );
*/
          mprintf(sizeof(buf), buf,
            "%s {WI have {D<{R%d.%02d {rR{Ru{rby{D> <{C%d.%02d {cSap{Bp{bhi{cre{D> <{G%d.%02d {gE{Gme{gr{Ga{gld{D> <{C%d.%02d {CDi{cam{Wo{wnd{D>{g{x",
            gch->name,
            ch->ruby_counter,     (ch->ruby_fragment     * 100 / 250000),
            ch->sapphire_counter, (ch->sapphire_fragment * 100 / 200000),
            ch->emerald_counter,  (ch->emerald_fragment  * 100 / 150000),
            ch->diamond_counter,  (ch->diamond_fragment  * 100 / 100000) );
            do_function(ch, &do_tell, buf);
          //act_channels(buf,ch,argument,gch,TO_VICT,POS_SLEEPING, CHAN_GROUPTELL);
        } // victim found
      }
      else
      {
        send_to_char( "In your dreams, or what?\n\r", ch );
        return;
      }
    } // if 'tell' arg
    if ( !argfound ) { printf_to_char(ch,"What???{x\n\r"); }
  } // arg is not null
  return;
} // freport

/*
 * Identify
 * needs to check inventory, any shop in the room, and later "item storage"
 */
void do_identify( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA   *vch;
    SHOP_DATA   *shop;
    OBJ_DATA    *obj;
    char        buf[MSL];
    int         cost, gold, silver;

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: identify <object>\n\r", ch );
        return;
    }

    for ( vch = ch->in_room->people ; vch ; vch = vch->next_in_room )
    {
        //first things first, if vch != ch, no more mortals
        if ( !IS_NPC( vch ) && ( vch != ch ) )
            continue;

        if ( vch == ch )
            continue;
        
        //If we have a shop keeper
        if ( IS_NPC( vch )
        && ( shop = vch->pIndexData->pShop ) != NULL ) 
        {

            if ( ( obj = get_obj_keeper( ch, vch, argument ) ) == NULL )
                continue;
            
            cost = get_cost( vch, obj, TRUE );
            cost = UMAX( 1000, cost / 10 );

            if ( !can_see_obj( ch, obj ) )
                continue;

            if ( ( ch->silver + ( ch->gold * 100 ) ) < cost )
            {
                send_to_char( "You can't afford to identify that.\n\r", ch );
                return;
            }

            if ( IS_IMMORTAL( ch ) )
                cost = 0;

            new_deduct_cost( ch, cost, &gold, &silver );

            vch->gold    += gold;
            vch->silver  += silver;

            if ( gold == 0 )
                sprintf( buf, "You give {W%d {wsi{Wl{Dv{wer{x to %s.\n\r%s teaches you about $p.",
                    silver, vch->short_descr, vch->sex == SEX_FEMALE ? "She" : "He" );
            else if ( silver == 0 )
                sprintf( buf, "You give {Y%d {yg{Yo{yld{x to %s.\n\r%s teaches you about $p.",
                    gold, vch->short_descr, vch->sex == SEX_FEMALE ? "She" : "He" );
            else if ( silver < 0 )
                sprintf( buf,
                    "You give {Y%d {yg{Yo{yld{x and get {W%d {wsi{Wl{Dv{wer{x back from %s.\n\r%s teaches you about $p.",
                        gold, UABS( silver ), vch->short_descr, vch->sex == SEX_FEMALE ? "She" : "He" );
            else
                sprintf( buf, "You give {Y%d {yg{Yo{yld{x and {W%d {wsi{Wl{Dv{wer{x to %s.\n\r%s teaches you about $p.",
                    gold, silver, vch->short_descr, vch->sex == SEX_FEMALE ? "She" : "He" );

            act( buf, ch, obj, NULL, TO_CHAR );

            show_obj_stats( ch, obj );

            return;
        }

        if ( IS_SET( vch->act2, ACT2_IDENTIFIER ) )
        {
            if ( ( obj = get_obj_carry( ch, argument, ch ) ) == NULL )
            {
                send_to_char( "Item not found\n\r", ch );
                return;
            }
            
            cost = obj->cost;
            cost = UMAX( 1000, cost / 10 );

            if ( ( ch->silver + ( ch->gold * 100 ) ) < cost )
            {
                send_to_char( "You can't afford to identify that.\n\r", ch );
                return;
            }

            if ( IS_IMMORTAL( ch ) )
                cost = 0;

            new_deduct_cost( ch, cost, &gold, &silver );

            vch->gold    += gold;
            vch->silver  += silver;

            if ( gold == 0 )
                sprintf( buf, "You give {W%d {wsi{Wl{Dv{wer{x to %s.\n\r%s teaches you about $p.",
                    silver, vch->short_descr, vch->sex == SEX_FEMALE ? "She" : "He" );
            else if ( silver == 0 )
                sprintf( buf, "You give {Y%d {yg{Yo{yld{x to %s.\n\r%s teaches you about $p.",
                    gold, vch->short_descr, vch->sex == SEX_FEMALE ? "She" : "He" );
            else if ( silver < 0 )
                sprintf( buf,
                    "You give {Y%d {yg{Yo{yld{x and get {W%d {wsi{Wl{Dv{wer{x back from %s.\n\r%s teaches you about $p.",
                        gold, UABS( silver ), vch->short_descr, vch->sex == SEX_FEMALE ? "She" : "He" );
            else
                sprintf( buf, "You give {Y%d {yg{Yo{yld{x and {W%d {wsi{Wl{Dv{wer{x to %s.\n\r%s teaches you about $p.",
                    gold, silver, vch->short_descr, vch->sex == SEX_FEMALE ? "She" : "He" );

            act( buf, ch, obj, NULL, TO_CHAR );

            show_obj_stats( ch, obj );

            return;

        }
        continue;
    }

    //mob is ACT_IDENTIFIER
    if ( IS_IMMORTAL( ch ) )
    {
        if ( ( obj = get_obj_carry( ch, argument, ch ) ) == NULL )
        {
            send_to_char( "Item not found\n\r", ch );
            return;
        }

        show_obj_stats( ch, obj );
        return;
    }
    else
    {
        send_to_char( "There is no identifier here.\n\r", ch );
        return;
    }

}

/* Show how close to 100% skill/spell mastery someone is */
void do_mastered( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *gch;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  int pskillct  = 0;
  int pskilltot = 0;
  int pskillgain= 0;
  int pskillmast= 0;

  int pspellct  = 0;
  int pspelltot = 0;
  int pspellgain= 0;
  int pspellmast= 0;
  double totalmastery = 0.0;
  int sn;

  one_argument( argument, arg );
  if ( IS_IMMORTAL( ch ) )
  {
    if ( ( gch = get_char_world_ordered( ch, arg, "chars" ) ) == NULL )
      gch = ch;
    else
      if ( IS_NPC( gch ) )
        gch = ch;
  }
  else
    gch = ch;
  
  for ( sn = 0; sn < MAX_SKILL; sn++ )
  {
    if ( skill_table[sn].name == NULL ) continue;

    if ( ( skill_table[sn].skill_level[gch->gameclass] < 0 )
    &&   !is_racial_skill( gch,sn ) ) continue;

    if ( skill_table[sn].spell_fun == spell_null )
    {
      if (gch->pcdata->learned[sn] == 0)
        pskillgain++;
      else
      {
        if (gch->pcdata->learned[sn] == 100) 
          pskillmast++;
        pskillct++;
        pskilltot += gch->pcdata->learned[sn];
      }
    }
    else
    {
      if (gch->pcdata->learned[sn] == 0)
        pspellgain++;
      else
      {
        if (gch->pcdata->learned[sn] == 100)
          pspellmast++;
        pspellct++;
        pspelltot += gch->pcdata->learned[sn];
      }
    }
  }

  sprintf( buf, "{C                  Skills    Spells    Total\n\r" );
  send_to_char( buf, ch );

  sprintf( buf, "{c  =============  ========  ========  =======\n\r");
  send_to_char( buf, ch );

  sprintf( buf, "{w      Learned:     %3d       %3d       %3d\n\r",
    pskillct,
    pspellct,
    ( pskillct   + pspellct   ) );
  send_to_char( buf, ch );

  sprintf( buf, "{w      To gain:     %3d       %3d       %3d\n\r",
    pskillgain,
    pspellgain,
    ( pskillgain + pspellgain ) );
  send_to_char( buf, ch );

  sprintf( buf, "{w     Mastered:     %3d       %3d       %3d\n\r",
    pskillmast,
    pspellmast,
    ( pskillmast + pspellmast ) );
  send_to_char( buf, ch );

  if (pspellct > 0)
  {
    sprintf( buf, "{w    Average %%:     %3d       %3d       %3d\n\r",
      ( pskilltot / pskillct ),
      ( pspelltot / pspellct ),
      ( pskilltot  + pspelltot ) / ( pskillct + pspellct ) );
  }
  else
  {
    sprintf( buf, "{w    Average %%:     %3d         0       %3d\n\r",
      ( pskilltot / pskillct ),
      ( pskilltot  + pspelltot ) / ( pskillct + pspellct ) );
  }
  send_to_char( buf, ch );

  sprintf( buf, "{c ==============  ========  ========  =======\n\r");
  send_to_char( buf, ch );

  totalmastery = ( pskilltot + pspelltot );
  totalmastery =  totalmastery / ( pskillct + pspellct + pskillgain + pspellgain );

  if (totalmastery == 100)
  {
    if (gch == ch)
      sprintf( buf, "{WYou are a {Rmaster{W %s.{x\n\r",
        class_table[gch->gameclass].name );
    else
      sprintf( buf, "{W%s is a {Rmaster {W%s.{x\n\r",
        gch->name,
        class_table[gch->gameclass].name );
  }
  else
  {
    if (gch == ch)
      sprintf( buf, "{WYou are {Y%2.2f{y%%{W of the way to complete mastery.{x\n\r",
        totalmastery );
    else
      sprintf( buf, "{W%s is {Y%2.2f{y%%{W of the way to complete mastery.{x\n\r",
        gch->name,
        totalmastery );
  }
  send_to_char( buf, ch );

  return;
}

void do_glance( CHAR_DATA *ch, char *argument )
{
  char arg[MIL];
  char buf[MSL];
  char buf2[MSL];
  CHAR_DATA *gch;
  int found = 0;
  int percent = 0;

  one_argument( argument, arg );

  for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
  {
    //Probably don't need to see themselves
    if ( gch == ch )
      continue;

    if ( !can_see( ch, gch ))
      continue; 
  
    if ( gch->max_hit > 0 )
      percent = ( 100 * gch->hit ) / GET_HP( gch );
    else
      percent = -1;

    if (percent >= 100)
      sprintf( buf, " is in excellent condition." );
    else if (percent >= 95 )
      sprintf( buf, " is in good condition." );
    else if (percent >= 87 )
      sprintf( buf, " has a few scratches." );
    else if (percent >= 80 )
      sprintf( buf, " has a few small wounds." );
    else if (percent >= 70 )
      sprintf( buf," has some small wounds and bruises." );
    else if (percent >= 60 )
      sprintf( buf," has some wounds and bruises." );
    else if (percent >=  50 )
      sprintf( buf, " has quite a few wounds." );
    else if (percent >= 40 )
      sprintf( buf, " has some big wounds and scratches." );
    else if (percent >= 30 )
      sprintf( buf, " has some big nasty wounds and scratches." );
    else if (percent >= 15 )
      sprintf ( buf, " looks pretty hurt." );
    else if (percent >= 7 )
      sprintf ( buf, " looks very hurt." );
    else if (percent >= 0 )
      sprintf ( buf, " is in awful condition." );
    else
      sprintf(buf, " is bleeding to death." );

    if ( gch->alignment > 0 )
      sprintf( buf2, "{y" );
    else if ( gch->alignment == 0 )
      sprintf( buf2, "{w" );
    else
      sprintf( buf2, "{r" );
 
    printf_to_char( ch, "%s%3s %3s %s%s\n\r",
      buf2,
      IS_NPC( gch ) ? "Mob{x" : "Plr{x",
      gch->fighting ? "{REng{x" : "{WN/F{x",
      IS_NPC( gch ) ? gch->short_descr : gch->name,
      buf );
    found++;
  }
    
  if ( found == 0 )
    send_to_char( "You seem to be all alone.\n\r", ch );

  return;
}

/* Recall to a player's home */
void do_home( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA   *room;
  CHAR_DATA         *victim;
  char              arg[MIL];

  argument = one_argument( argument, arg );

  if ( IS_IMMORTAL(ch)
  &&   !IS_NULLSTR(arg) )
  {   
    if ( IS_NPC( ch ) )
      return;

    if ( is_number(arg) )
    {
      if ( ( room = find_location( ch, arg ) ) == NULL )
      {
        send_to_char( "No such location.\n\r", ch );
        return;
      }

      ch->pcdata->home_vnum = room->vnum;
      printf_to_char( ch, "Your home has been set to room %d.\n\r",
        ch->pcdata->home_vnum );
      return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "Player not found.\n\r", ch );
      return;
    }

    if ( IS_NPC( victim ) )
      return;

    if ( IS_NULLSTR( argument ) )
    {
      printf_to_char( ch, "%s's home vnum is set to %d.\n\r",
        victim->name,
        victim->pcdata->home_vnum );
      return;
    }

    if ( !is_number(argument) )
    {
      send_to_char( "Syntax: home <player> <room vnum>\n\r", ch );
      return;
    }    
    
    if ( ( room = find_location( ch, argument ) ) == NULL )
    {
      send_to_char( "No such location.\n\r", ch );
      return;
    }

    victim->pcdata->home_vnum = atoi(argument);
    printf_to_char( ch, "%s's home has been set to room %d.\n\r",
      victim->name,
      victim->pcdata->home_vnum );
    return;
  }

  /* Pets should look for their PC master's home, if they exist */
  if ( IS_NPC( ch ) )
  {
    CHAR_DATA *pet_owner;

    if (!ch->master
    ||   IS_NPC(ch->master) )
      return;

    if ( (pet_owner = ch->master) == NULL )
      return;

    if ( ( room = get_room_index(pet_owner->pcdata->home_vnum) ) == NULL )
      return;
  }
  else // not an NPC, so ch->pcdata exists
  {
    if ( ( room = get_room_index(ch->pcdata->home_vnum) ) == NULL )
    {
      send_to_char( "No such location.\n\r", ch );
      return;
    }
  }

  if ( ch->in_room == room )
  {
    if ( !IS_NPC( ch ) )
      send_to_char( "You are already home.\n\r", ch );
    return;
  }

  if ( !IS_IMMORTAL(ch) )
  {
    if ( IS_NPC( ch ) )
    {
      if (!ch->master
      ||   IS_NPC(ch->master) )
      {
        if ( !recall(ch, FALSE, ch->master->pcdata->home_vnum) )
          return;
      }
    }
    else
    {
      if ( !recall(ch, FALSE, ch->pcdata->home_vnum) )
        return;
    }

    if ( ch->in_room->area->continent != room->area->continent )
    {
      send_to_char( "You must be on the same continent!\n\r", ch );
      return;
    }
  }

  act( "$n disappears.", ch, NULL, NULL, TO_WIZ_ROOM );

  move_to_room( ch, room );
  if ( ch->pet )
    move_to_room( ch->pet, room );

  do_function( ch, &do_look, "" );
  act( "$n appears in the room.", ch, NULL, NULL, TO_WIZ_ROOM );

  if ( !IS_IMMORTAL(ch) )
    WAIT_STATE( ch, 10 );

  return;
}

void do_decorate( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *room = NULL;
  OBJ_DATA        *obj = NULL;
  OBJ_DATA        *obj_new = NULL;
  char            arg[MIL], arg2[MIL], arg3[MIL];

  argument = one_argument( argument, arg );
  
  if ( str_cmp(ch->in_room->owner, ch->name) )
  {
    send_to_char( "You can only do this in your house.\n\r", ch );
    return;
  }

  if ( IS_NULLSTR(arg) )
  {
    send_to_char( "Syntax: decorate <name|description>\n\r", ch );
    send_to_char( "        decorate object <keyword> <extra>\n\r", ch );
    send_to_char( "        decorate object <keyword> <name|short|long> <description>\n\r", ch );
    return;
  }

  //Must type out object for now, if it doesn't interfere w/ other things can str_prefix it
  if ( !str_cmp( arg, "object" ) )
  {
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( IS_NULLSTR(arg3)
    ||   IS_NULLSTR(argument) )
    {
      send_to_char( "Syntax: decorate <name|description>\n\r", ch );
      send_to_char( "        decorate object <keyword> <extra>\n\r", ch );
      send_to_char( "        decorate object <keyword> <name|short|long> <description>\n\r", ch );
      return;
    }

    if ( (obj = get_obj_here( ch, arg2 )) == NULL )
    {
      send_to_char( "You don't see that here.\n\r", ch );
      return;
    }

    if ( !IS_SET( obj->extra_flags, ITEM_PLAYER_HOUSE ) )
    {
      send_to_char( "You are not authorized to decorate that object.\n\r", ch );
      return;
    }

    if ( !str_prefix( arg3, "name" ) )
    {
      free_string( obj->pIndexData->name );
      obj->pIndexData->name = str_dup( argument ,obj->pIndexData->name);
      save_area( ch->in_room->area );
      printf_to_char( ch,  "Name set to %s.\n\r", argument);

      //Replace the Item
      if ( obj->carried_by != NULL ) //Just in case
        obj_from_char( obj );
      extract_obj( obj );
      obj_new = create_object( obj->pIndexData, obj->level );
      obj_to_room( obj_new, ch->in_room );

      return;
    }

    if ( !str_prefix( arg3, "short" ) )
    {
      free_string( obj->pIndexData->short_descr );
      obj->pIndexData->short_descr = str_dup( argument ,obj->pIndexData->short_descr);
      obj->pIndexData->short_descr[0] = LOWER( obj->pIndexData->short_descr[0] );
      save_area( ch->in_room->area );
      printf_to_char( ch,  "Short description set to %s.\n\r", argument);

      //Replace the Item
      if ( obj->carried_by != NULL ) //Just in case
        obj_from_char( obj );
      extract_obj( obj );
      obj_new = create_object( obj->pIndexData, obj->level );
      obj_to_room( obj_new, ch->in_room );

      return;
    }

    if ( !str_prefix( arg3, "long" ) )
    {
      free_string( obj->pIndexData->description );
      obj->pIndexData->description = str_dup( argument ,obj->pIndexData->description);
      save_area( ch->in_room->area );
      printf_to_char( ch,  "Long description set to '%s'\n\r", argument);

      //Replace the Item
      if ( obj->carried_by != NULL ) //Just in case
        obj_from_char( obj );
      extract_obj( obj );
      obj_new = create_object( obj->pIndexData, obj->level );
      obj_to_room( obj_new, ch->in_room );

      return;
    }

    return;
  } //end objects

  room = ch->in_room;

  if ( IS_NULLSTR(argument) )
  {
    if ( !str_prefix(arg, "description") )
    {
      string_append( ch, &room->description, APPEND_AREA, room->area );
      SET_BIT( room->area->area_flags, AREA_CHANGED );
    }
    else if ( !str_prefix(arg, "save") )
    {
      save_area( room->area );
      REMOVE_BIT( room->area->area_flags, AREA_CHANGED );
      send_to_char( "Your decorations have been saved.\n\r", ch );
      return;
    }
  }
  else
  {
    if ( !str_prefix(arg, "name") )
    {
      free_string( room->name );
      room->name = str_dup( argument ,room->name);
      send_to_char( "Room name updated.\n\r", ch );
      save_area( room->area );
      REMOVE_BIT( room->area->area_flags, AREA_CHANGED );
      //sprintf( buf, "%s updated house name, room %d", ch->name, room->vnum );
      /* Add in player_house wiznet */
      return;
    }
  }
  return;
}

void do_experience( CHAR_DATA *ch, char *argument )
{
  int XPL;

  if (IS_NPC(ch))
    return;

  printf_to_char( ch, " Total experience: %llu\n\r", ch->exp);

  if ( ch->level >= LEVEL_HERO )
    printf_to_char(ch, " XP at next level: N/A\n\rCurrent per-level: N/A\n\r    To next level: N/A\n\r");
  else
  {
    XPL = exp_per_level( ch, ch->pcdata->points );
    printf_to_char(ch, " XP at next level: %llu\n\rCurrent per-level: %d\n\r    To next level: %d\n\r",
      TNL( XPL, ch->level ),
      TNL( XPL, ch->level ) - TNL( XPL, ch->level - 1 ),
      TNL( XPL, ch->level ) - ch->exp );
  }

  return;
}

/*
 * Function: do_psearch
 * Search through player files for matches on a variety of things: name, IP, class, race, etc.
 */

void do_psearch(CHAR_DATA *ch, char *argument)
{
  char arg[MIL];
  struct dirent *dp;
  DIR *dirptr;
  GENERIC_LIST *gl; // NOT a pointer

  if ( IS_NULLSTR(argument) )
  {
    send_to_char( "Syntax: psearch name <keyword>\n\r", ch );
    return;
  }
  
  argument = one_argument( argument, arg );

  if ( strcmp(arg, "name") || IS_NULLSTR(argument) )
  {
    send_to_char( "Syntax: psearch name <name>\n\r", ch );
    return;
  }

  if ((dirptr = opendir(PLAYER_DIR)) == NULL){
    send_to_char("Player dir unable to be opened\n\r",ch);
    return;
  }

  gl = gl_new();

  while((dp = readdir(dirptr)) != NULL)
  {
    if ( !str_cmp( argument, "." ) )
      continue;

    if ( !str_cmp( argument, ".." ) )
      continue;

    if ( str_prefix( argument, dp->d_name ) )
      continue;
    
    gl_push(gl, 1, dp->d_name, NULL);
  }

  if (gl_count_items(gl) > 0)
  {
    gl_sort_by_name(gl);
    gl_send_names_vcolumns(ch, gl, 4, 19);
  }
  else
    send_to_char("No Matches Found.\n\r", ch);

  gl_close(gl); // must always run at end
  closedir(dirptr);
  return;
} 
