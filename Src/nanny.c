/* *\
|* *| This is a new menu-based creation system to replace nanny() in comm.c
|* *| Since it's a long and complicated function, I decided to put it into a
|* *| new file while working on it.  It may stay in its own file... -- Taeloch
\* */

#include <sys/types.h>
#include <sys/time.h>
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
#include <signal.h>
#include <malloc.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "interp.h"
#include "olc.h"
#include "telnet.h"

void close_duplicate_sockets( DESCRIPTOR_DATA *d, const char *name );
void mssp_request    args( ( DESCRIPTOR_DATA *d ) );
bool check_reconnect args( ( DESCRIPTOR_DATA *d, char *name, bool fConn ) );
bool check_playing   args( ( DESCRIPTOR_DATA *d, char *name ) );
void check_dup_socket(CHAR_DATA *ch);
void send_cprompt(CHAR_DATA *ch);
int get_start_room(bool isevil, bool isgood, bool isnewbie);
void randomize_char(CHAR_DATA *ch);
void randomize_looks(CHAR_DATA *ch);
extern const char echo_off_str[];
extern const char echo_on_str[];
extern bool wizlock;
extern bool newlock;

/*
 * Deal with sockets that haven't logged in yet.
 * New char creation
 */
void new_nanny( DESCRIPTOR_DATA *d, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *ch;
  CHAR_DATA *vch;
  char *pwdnew;
  char log_buf[MSL];
  char *p;
  int num = 0;
  int iClass,race,i,weapon;
  bool fOld, valid;

  while ( isspace(*argument) )
    argument++;

  ch = d->character;

  switch ( d->connected )
  {
    default:
      bug( "Nanny: bad d->connected %d.", d->connected );
      close_socket( d );
      return;

/* *************************************************** */
    case CON_ANSI: 
      if (!strcmp(argument,"MSSP-REQUEST"))
      {
        mssp_request( d );
        d->connected = CON_GET_NAME;
        send_to_desc("Who are thee that wish {WEntrance{x to these Lands? ",d);
        break;
      }
      break; // case CON_ANSI

/* *************************************************** */
    case CON_GET_NAME:
      if ( argument[0] == '\0' )
      {
        close_socket( d );
        return;
      }

      if (!strcmp(argument,"MSSP-REQUEST"))
      {
        mssp_request( d );
        send_to_desc("Who are thee that wish {WEntrance{x to these Lands? ",d);
        break;
      }

      if (!str_prefix(capitalize(argument),"Get "))
      {
        send_to_desc("HTTP/1.1 301 Moved Permanently\n\r",d);
        send_to_desc("Location: http://www.draknor.com/index.php\n\r",d);
        sprintf( buf, "HTTP request on socket %d.", d->descriptor );
        wiznet( buf, NULL, NULL, WIZ_SECURE, 0, get_trust(ch) );
        d->outbuf[0] = '\0';
        d->outtop = 0;
        close_socket(d);
        return;
      }

      argument[0] = UPPER(argument[0]);

      if ( !check_parse_name( argument ) )
      {
        send_to_desc("Illegal name, try another.\n\rName: ", d);
        if (strcmp(argument,"Sp"))
        {
          sprintf( buf, "Illegal name '%s' from socket %d.", argument, d->descriptor );
          wiznet( buf, NULL, NULL, WIZ_SECURE, 0, get_trust(ch) );
        }
        return;
      }

      fOld = load_char_obj( d, argument );

      if ( check_ban( d->host, BAN_ALL ) )
      {
        send_to_desc( "Your site has been banned from this MUD.\n\r", d );
        close_socket( d );
        return;
      }

      if (check_ban(d->host,BAN_PERMIT) && !IS_SET(ch->act,PLR_PERMIT))
      {
        send_to_desc("Your site has been banned from this mud.\n\r",d);
        close_socket(d);
        return;
      }

      if ( check_reconnect( d, argument, FALSE ) )
      {
        fOld = TRUE;
      }
      else
      {
        if ( wizlock && !IS_IMMORTAL(d->character)) 
        {
          send_to_desc("The game is wizlocked.\n\r", d );
          close_socket( d );
          return;
        }
      }

      if (fOld)
      {
        ch = d->character;

        if (ch->master)
        {
          if (can_see_room(ch, ch->master->in_room))
          {
            move_to_room(ch, ch->master->in_room);
          }
          else
          {
            move_to_room(ch, get_room_index(ROOM_VNUM_TEMPLE));
          }
        }

        if (IS_SET(ch->act, PLR_DENY))
        {
          int days =0;
          /* Calc day time since last login if denytime set */
          if (ch->pcdata->denytime)
          {
            days = (int)( difftime( current_time, ch->pcdata->denied_time ) / 86400.0 );

            if (ch->pcdata->denytime <= days)
            {
              REMOVE_BIT(ch->act, PLR_DENY);
              ch->pcdata->denytime =0;
              ch->pcdata->denied_time = 0;
              send_to_desc("Your penalty has expired. Your deny is removed.\n", d);
            }
            else
            {
              mprintf( sizeof(log_buf), log_buf, "Denying access to %s@%s.", argument, d->host );
              log_string( log_buf );
              mprintf(sizeof(log_buf), log_buf, "You have %d more days remaining on your denial.\n\r", ch->pcdata->denytime - days);    send_to_desc(log_buf, d );
              send_to_desc("You are denied access.\n\r", d );
              close_socket( d );
              return;
            }
          }
          else
          {
            mprintf( sizeof(log_buf), log_buf, "Denying access to %s@%s.", argument, d->host );
            log_string( log_buf );
            send_to_desc("You are denied access.\n\r", d );
            close_socket( d );
            return;
          }
        }
      } // /fOld

      if ( fOld ) // again?
      {
        /* Old player */
        send_to_desc("Password: ", d );
        write_to_buffer( d, echo_off_str, 0 );
        d->connected = CON_GET_OLD_PASSWORD;
        return;
      }
      else
      {
        /* New player */
        if (newlock)
        {
          send_to_desc( "The game is newlocked.\n\r", d );
          close_socket( d );
          return;
        }

        if (check_ban(d->host,BAN_NEWBIES))
        {
          send_to_desc("New players are not allowed from your site.\n\r",d);
          close_socket(d);
          return;
        }

        mprintf(sizeof(buf), buf, "Did I get that right, %s (Y/N)? ", argument );
        send_to_desc(buf, d );
        d->connected = CON_CONFIRM_NEW_NAME;
        return;
      }
      break; // case CON_GET_NAME

/* *************************************************** */
    case CON_GET_OLD_PASSWORD: // Send_to_char should be valid from here on...
      send_to_desc("\n\r", d);

      if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ))
      {
        send_to_desc("{RWrong password{x.\n\r", d );
        logit("INVALID PASSWORD: %s. Socket: %s ",ch->name, d->host);

        mprintf(sizeof(log_buf), log_buf, "{g%s{w@%s has entered an incorrect password.", ch->name, d->host);
        wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

        close_socket( d );
        return;
      }
 
      write_to_buffer( d, echo_on_str, 0 );

      if (check_playing(d,ch->name))
        return;

      if ( check_reconnect( d, ch->name, TRUE ) )
        return;


      check_dup_socket(ch);
      close_duplicate_sockets(d, ch->name);
      mprintf(sizeof(log_buf), log_buf, "{g%s{w@%s has connected.", ch->name, d->host);
      log_string( log_buf );
      wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

      if ( IS_IMMORTAL(ch) )
      {
        d->connected = CON_READ_IMOTD;
        do_help( ch, "imotd" );
      }
      else
      {
        d->connected = CON_READ_MOTD;
        do_help( ch, "motd" );
      }

      send_to_char("{D[Press {greturn{D to continue]{x",ch);
      break; // case CON_GET_OLD_PASSWORD
 
/* *************************************************** */
    case CON_BREAK_CONNECT:
      switch( *argument )
      {
        case 'y':
        case 'Y':
          close_duplicate_sockets( d, ch->name );
          if (check_reconnect(d,ch->name,TRUE))
            return;
          write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);
          if ( d->character != NULL )
          {
            nuke_pets(d->character, TRUE);
            free_char( d->character );
            d->character = NULL;
          }
          d->connected = CON_GET_NAME;
          break;

        case 'n':
        case 'N':
          write_to_buffer(d,"Name: ",0);
          if ( d->character != NULL )
          {
            nuke_pets( d->character, TRUE );
            free_char( d->character );
            d->character = NULL;
          }
          d->connected = CON_GET_NAME;
          break;

        default:
          write_to_buffer(d,"Please type Y or N? ",0);
          break;
      } // CON_BREAK_CONNECT y/n switch
      break; // case CON_BREAK_CONNECT

/* *************************************************** */
    case CON_CONFIRM_NEW_NAME:
      switch ( *argument )
      {
        case 'y':
        case 'Y':
          mprintf( sizeof(buf), buf, "New character.\n\rGive me a password for %s: %s",
              ch->name, echo_off_str );
          write_to_buffer( d, buf, 0 );
          d->connected = CON_GET_NEW_PASSWORD;
          break;

        case 'n':
        case 'N':
          write_to_buffer( d, "Ok, what IS it, then? ", 0 );
          nuke_pets( d->character, TRUE);
          free_char( d->character );
          d->character = NULL;
          d->connected = CON_GET_NAME;
          break;

        default:
          write_to_buffer( d, "Please type Yes or No? ", 0 );
          break;
      } // CON_CONFIRM_NEW_NAME y/n switch
      break; // case CON_CONFIRM_NEW_NAME

/* *************************************************** */
    case CON_GET_NEW_PASSWORD:
      write_to_buffer( d, "\n\r", 2 );

      if ( strlen(argument) < 7 )
      {
        write_to_buffer( d, "Password must be at least eight characters long.\n\rPassword: ", 0 );
        return;
      }

      pwdnew = crypt( argument, ch->name );
      for ( p = pwdnew; *p != '\0'; p++ )
      {
        if ( *p == '~' )
        {
          write_to_buffer( d, "New password not acceptable, try again.\n\rPassword: ", 0 );
          return;
        }
      }

      if (!strstr(argument,"1")
      &&  !strstr(argument,"2")
      &&  !strstr(argument,"3")
      &&  !strstr(argument,"4")
      &&  !strstr(argument,"5")
      &&  !strstr(argument,"6")
      &&  !strstr(argument,"7")
      &&  !strstr(argument,"8")
      &&  !strstr(argument,"9")
      &&  !strstr(argument,"0"))
      {
        send_to_char("You must have a number in your password.\n\rPassword:",ch);
        return;
      }

      free_string( ch->pcdata->pwd );
      ch->pcdata->pwd  = str_dup( pwdnew, ch->pcdata->pwd );
#if MEMDEBUG
      free_string (ch->pcdata->memdebug_pwd);
      ch->pcdata->memdebug_pwd = str_dup(pwdnew, ch->pcdata->memdebug_pwd);
#endif
      write_to_buffer( d, "Please retype password: ", 0 );
      d->connected = CON_CONFIRM_NEW_PASSWORD;
      break; // case CON_GET_NEW_PASSWORD

/* *************************************************** */
    case CON_CONFIRM_NEW_PASSWORD:
      if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
      {
        write_to_buffer( d, "\n\r", 2 );
        write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",  0 );
        d->connected = CON_GET_NEW_PASSWORD;
        return;
      }

      do_function( ch, &do_colour, "" );
      send_to_char("\n\r",ch);
      ch->lines = 35;
      ch->wimpy = 5;
      send_to_char("{R(-------DISCLAIMER-------){x\n\r\n\r", ch);
      do_function(ch,&do_help,"disclaimer");
      send_to_char("{wYou must accept to agreement to continue the creation process.{x\n\r", ch);
      send_to_char("{wDo you accept this agreement? (Y/N): {x", ch);
      d->connected = CON_GET_DISCLAIMER;
      return; // case CON_CONFIRM_NEW_PASSWORD

/* *************************************************** */
    case CON_GET_DISCLAIMER:
      switch ( argument[0] )
      {
        case 'y': case 'Y':
          // logging and notification
          mprintf( sizeof(log_buf), log_buf, "{g%s{x@%s new player.", ch->name, d->host );
          mprintf( sizeof(buf), buf, "Newbie alert!  %s sighted.", capitalize( ch->name ) );
          wiznet(buf,ch,NULL,WIZ_NEWBIE,0,0);
          wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));
          log_string( log_buf );

          send_to_char("\n\r", ch);
          send_to_char("{wWould you like to read the story? (Y/N)? {x", ch);
          d->connected = CON_GET_STORY;
          break;
        case 'n': case 'N':
          send_to_char("You cannot create a new character unless you accept this agreement.\n\r{x", ch);
          close_socket( d );
          return;
          break;
        default:
          send_to_char("Please answer with a 'Y' or an 'N'.\n\r{wDo you accept this agreement? (Y/N): {x", ch);
          return;
      }
      break; // case CON_GET_DISCLAIMER

/* *************************************************** */
    case CON_GET_STORY:
      switch ( argument[0] )
      {
        case 'y':
        case 'Y':
          send_to_char("\n\r",ch);
          do_function(ch,&do_help,"story");
          send_to_char( "\n\rHit {greturn{x to continue\n\r", ch );
          d->connected = CON_GET_STORY2;
          break;
        case 'n':
        case 'N':
          send_to_char("\n\r",ch);
          send_to_char("You have entered into the world of Draknor. You are creating \n\r",ch);
          send_to_char("who you will be in this game, take time to think about the \n\r",ch);
          send_to_char("decisions you are about to make, many cannot be changed later.\n\r\n\r",ch);
          send_to_char("{RDo you consider yourself a beginner?{x\n\r", ch );
          send_to_char("This will affect the way you begin the game. By choosing \n\r",ch);
          send_to_char("{gyes{x you will start with a guide to playing, a map of the \n\r",ch);
          send_to_char("town you will start in, and with newbie hints enabled. This will \n\r",ch);
          send_to_char("Also toggle some of your automatic commands and explain them.\n\r",ch);
          send_to_char("So are you a beginner? {g(Y/N){x ", ch );
          d->connected = CON_GET_NEWBIE;
          break;
        default:
          send_to_char("{wThat's not an answer.\n\rDo you want The Story? (Y/N)?{x", ch);
          return;
      }
      break; // case CON_GET_STORY

/* *************************************************** */
    case CON_GET_STORY2:
      send_to_char("\n\r",ch);
      send_to_char("You have entered into the world of Draknor. You are creating \n\r",ch);
      send_to_char("who you will be in this game, take time to think about the \n\r",ch);
      send_to_char("decisions you are about to make, many cannot be changed later.\n\r\n\r",ch);
      send_to_char("{RDo you consider yourself a beginner?{x\n\r", ch );
      send_to_char("This will affect the way you begin the game. By choosing \n\r",ch);
      send_to_char("{gyes{x you will start with a guide to playing, a map of the \n\r",ch);
      send_to_char("town you will start in, and with newbie hints enabled. This will \n\r",ch);
      send_to_char("Also toggle some of your automatic commands and explain them.\n\r",ch);
      send_to_char("So are you a beginner? {g(Y/N){x ", ch );

      d->connected = CON_GET_NEWBIE;
      break; // case CON_GET_STORY2

/* *************************************************** */
    case CON_GET_NEWBIE:
      switch ( argument[0] )
      {
        case 'y':
        case 'Y':
          ch->newbie = TRUE;    
          break;
        case 'n':
        case 'N':
          ch->newbie = FALSE; 
          break;
        default:
          send_to_char("{wThat's not an answer.\n\r{w Are you a beginner?{x ", ch );
          return;
      }

// if still here, move on to creation.  But first, set some default values
      strcpy(argument,"");
      randomize_char(ch);
      ch->max_hit =  number_range(18,25);

      d->connected = CON_NEW_CREATION;
// no break: if valid answer was given, go straight to CON_NEW_CREATION menu
     // case CON_GET_NEWBIE

/* ************************************************************************* */
/* ***************** Here is where the new creation starts ***************** */
/* ************************************************************************* */
/* *************************************************** */
    case CON_NEW_CREATION:
      fOld = TRUE; // reusing bool var, display menu or not

      if (argument[0] != '\0') // something was typed: find out what
      {
        argument = one_argument(argument,arg); // arg now contains command word, rest in *argument

        if (!str_prefix(capitalize(arg),"Help"))
        {
          if (argument[0] != '\0')
            do_function(ch,&do_help,argument);
          else
          {
            send_to_char("\n\r{cCommands: {Csex{c, {Crace{c, {Cclass{c, {Calign{c, {Cansi{c, {Cnoloot{c,\n\r", ch);
            send_to_char("          {Cscroll{c, {Cwimpy{c, {Chelp{c, {Crandomize{c, {Cdone{x\n\r", ch);
          }
          send_cprompt(ch);
          return;
        }
        else if (!str_prefix(capitalize(arg),"Sex"))
        {
          switch ( argument[0] )
          {
            case 'm':
            case 'M':
              ch->sex = SEX_MALE;    
              ch->pcdata->true_sex = SEX_MALE;
              break;
            case 'f':
            case 'F':
              ch->sex = SEX_FEMALE; 
              ch->pcdata->true_sex = SEX_FEMALE;
              if ( (ch->size >= SIZE_MEDIUM)
              &&   (ch->pcdata->weight > (pc_race_table[ch->race].max_weight - 50) ) )
                ch->pcdata->weight = number_range(pc_race_table[ch->race].min_weight, (pc_race_table[ch->race].max_weight - 50));
              break;
            default:
              send_to_char("\n\r{cThat's not a valid sex.  Enter \"{Csex M{c\" or \"{Csex F{c\":{x\n\r ", ch );
              send_cprompt(ch);
              return;
          } // switch "sex"
        } // end sex
        else if (!str_prefix(capitalize(arg),"Race"))
        {
          if (argument[0] == '\0')
          {
            do_function( ch, &do_help, "races" );
            send_to_char("\n\r{cTo choose a race, type \"{Crace{c\" followed by the race you want.{x\n\r ", ch );
            send_cprompt(ch);
            return;
          }

          race = race_lookup(argument);

          if (race == 0 || !race_table[race].pc_race)
          {
            send_to_char("\n\rThat is not a valid race.  Enter \"help races\" for a list.\n\r",ch);
            send_cprompt(ch);
            return;
          }

          if ((race == race_lookup("vampire"))
          && ((ch->gameclass == cPriest) || (ch->gameclass == cInquisitor)))
          {
            send_to_char("\n\r{RVampires {rcannot be Priests or Inquisitors.{x\n\r",ch);
            send_cprompt(ch);
            return;
          }

          ch->race = race;

          for (i = 0; i < MAX_STATS; i++) // initialize stats
            ch->perm_stat[i] = pc_race_table[race].stats[i];
          ch->affected_by    = ch->affected_by|race_table[race].aff;
          ch->imm_flags      = ch->imm_flags|race_table[race].imm;
          ch->res_flags      = ch->res_flags|race_table[race].res;
          ch->vuln_flags     = ch->vuln_flags|race_table[race].vuln;
          ch->form           = race_table[race].form;
          ch->parts          = race_table[race].parts;
          ch->size           = pc_race_table[race].size;
          ch->pcdata->points = pc_race_table[race].points;
          randomize_looks(ch);
        } // end race
        else if (!str_prefix(capitalize(arg),"Class"))
        {
          if ( argument[0] == '\0' )
          {
            do_function( ch, &do_help, "classes" );
            send_to_char("\n\r{cTo choose a class, type \"{Cclass{c\" followed by the class you want.{x\n\r ", ch );
            send_cprompt(ch);
            return;
          }

          iClass = class_lookup(argument);

          if ((iClass == class_lookup("unused1"))
          ||  (iClass == class_lookup("unused2"))
          ||  (iClass == class_lookup("unused4"))
          ||  (iClass == class_lookup("unused5"))
          ||  (iClass == class_lookup("Woodsman")) // disabled, for now
          ||  (iClass == class_lookup("unused6")))
          {
            send_to_char("\n\rThat is not a valid class.  Enter \"help classes\" for a list.\n\r",ch);
            send_cprompt(ch);
            return;
          }

          if ( iClass == -1 )
          {
            send_to_char("\n\rThat is not a valid class.  Enter \"help classes\" for a list.\n\r",ch);
            send_cprompt(ch);
            return;
          }

          if ((ch->race == race_lookup("vampire"))
          && ((iClass == cPriest) || (iClass == cInquisitor)))
          {
            send_to_char("\n\r{RVampires {rcannot be Priests or Inquisitors.{x\n\r",ch);
            send_cprompt(ch);
            return;
          }

          ch->gameclass = iClass;
          if (IS_CLASS_INQUISITOR(ch))
            ch->alignment = 1000;
          else if (IS_CLASS_OCCULTIST(ch))
            ch->alignment = -1000;
        } // end class
        else if (!str_prefix(capitalize(arg),"Alignment"))
        {
          if (argument[0] == '\0')
          {
            do_function( ch, &do_help, "alignment" );
            send_to_char("\n\r{cTo choose an alignment, type \"{Calign{c\" followed by the alignment you want.{x\n\r ", ch );
            send_cprompt(ch);
            return;
          }

          if (!str_prefix(capitalize(argument),"Good"))
          {
            if (IS_CLASS_OCCULTIST(ch))
            {
              send_to_char("\n\r{GOccultists must have an evil alignment.{x\n\r",ch);
              send_cprompt(ch);
              return;
            }
            ch->alignment = 1000;
          }
          else if (!str_prefix(capitalize(argument),"Evil"))
          {
            if (IS_CLASS_INQUISITOR(ch))
            {
              send_to_char("\n\r{GInquisitors must have a good alignment.{x\n\r",ch);
              send_cprompt(ch);
              return;
            }
            ch->alignment = -1000;
          }
          else if (!str_prefix(capitalize(argument),"Neutral"))
          {
            if (IS_CLASS_OCCULTIST(ch))
            {
              send_to_char("\n\r{GOccultists must have an evil alignment.{x\n\r",ch);
              send_cprompt(ch);
              return;
            }
            if (IS_CLASS_INQUISITOR(ch))
            {
              send_to_char("\n\r{GInquisitors must have a good alignment.{x\n\r",ch);
              send_cprompt(ch);
              return;
            }
            ch->alignment = 0;
          }
          else
          {
            send_to_char("\n\rThat's not a valid alignment.  Enter \"align [good|neutral|evil]\".\n\r",ch);
            send_cprompt(ch);
            return;
          } // "align" if/else block
        } // end alignment
        else if (!str_prefix(capitalize(arg),"Ansi"))
        {
          do_function( ch, &do_colour, "" );
        } // end ansi
        else if (!str_prefix(capitalize(arg),"Noloot"))
        {
          if (arg[1] != '\0') // so that we don't react to "N" from "beginner" question
          {
            if (argument[0] == '\0')
            {
              do_function( ch, &do_help, "noloot" );
              send_cprompt(ch);
              return;
            }

            if (argument[0] == '\0')
              do_function( ch, &do_noloot, "" );
            else if (!str_prefix(capitalize(argument),"Lootable"))
            {
               SET_BIT( ch->act, PLR_CANLOOT );
               REMOVE_BIT( ch->act,PLR_MORGUE );
            }
            else if (!str_prefix(capitalize(argument),"Morgue"))
            {
               REMOVE_BIT( ch->act, PLR_CANLOOT );
               SET_BIT( ch->act,PLR_MORGUE );
            }
            else if (!str_prefix(capitalize(argument),"Safe"))
            {
               REMOVE_BIT( ch->act, PLR_CANLOOT );
               REMOVE_BIT( ch->act,PLR_MORGUE );
            }
            else
            {
              send_to_char("\n\rThat's not a valid alignment.  Enter \"align [good|neutral|evil]\".\n\r",ch);
              send_cprompt(ch);
              return;
            }
          }
        } // end noloot
        else if (!str_prefix(capitalize(arg),"Scroll"))
        {
          if (argument[0] == '\0')
          {
            send_to_char("\n\rTo set your scroll lines, enter \"scroll #\".\n\r",ch);
            send_cprompt(ch);
            return;
          }

          num = atoi(argument);
          if ((num < 10)
          ||  (num > 75))
          {
            send_to_char("\n\rScroll length must be between 10 and 75\n\r",ch);
            send_cprompt(ch);
            return;
          }

          ch->lines = num;
        } // end scroll
        else if (!str_prefix(capitalize(arg),"Wimpy"))
        {
          do_function( ch, &do_wimpy, argument );
        } // end wimpy
        else if (!str_prefix(capitalize(arg),"Randomize"))
        {
          randomize_char(ch);
        }
        else if (!str_prefix(capitalize(arg),"Eyes"))
        {
          valid = FALSE;
          if (argument[0] != '\0')
          {
            for (num=0; ((num < MAX_APPR) && (valid == FALSE)); num++)
            {
              if ( (pc_race_table[ch->race].eye_color[num] != NULL)
              &&   !str_prefix(argument, pc_race_table[ch->race].eye_color[num]) )
              {
                valid = TRUE;
                free_string(ch->pcdata->eye_color);
                ch->pcdata->eye_color = str_dup(pc_race_table[ch->race].eye_color[num], ch->pcdata->eye_color);
              }
            }
          }

          if (!valid)
          {
            printf_to_char(ch, "{cValid eye colors for this race are: ");
            for (num=0; num < MAX_APPR; num++)
            {
              if (pc_race_table[ch->race].eye_color[num] != NULL)
                printf_to_char(ch, "{C%s{c", pc_race_table[ch->race].eye_color[num]);
              if ( ((num+1) < MAX_APPR)
              &&   (pc_race_table[ch->race].eye_color[num+1] != NULL) )
                printf_to_char(ch, ", ");
            }
            printf_to_char(ch, "{x\n\r");
            send_cprompt(ch);
            return;
          }
        } // end eye color
        else if (!str_prefix(capitalize(arg),"Hair"))
        {
          valid = FALSE;
          if (argument[0] != '\0')
          {
            for (num=0; ((num < MAX_APPR) && (valid == FALSE)); num++)
            {
              if ( (pc_race_table[ch->race].hair_color[num] != NULL)
              &&   !str_prefix(argument, pc_race_table[ch->race].hair_color[num]) )
              {
                valid = TRUE;
                free_string(ch->pcdata->hair_color);
                ch->pcdata->hair_color = str_dup(pc_race_table[ch->race].hair_color[num], ch->pcdata->hair_color);
              }
            }
          }

          if (!valid)
          {
            printf_to_char(ch, "{cValid hair colors for this race are: ");
            for (num=0; num < MAX_APPR; num++)
            {
              if (pc_race_table[ch->race].hair_color[num] != NULL)
                printf_to_char(ch, "{C%s{c", pc_race_table[ch->race].hair_color[num]);
              if ( ((num+1) < MAX_APPR)
              &&   (pc_race_table[ch->race].hair_color[num+1] != NULL) )
                printf_to_char(ch, ", ");
            }
            printf_to_char(ch, "{x\n\r");
            send_cprompt(ch);
            return;
          }
        } // end hair
        else if (!str_prefix(capitalize(arg),"Skin"))
        {
          valid = FALSE;
          if (argument[0] != '\0')
          {
            for (num=0; ((num < MAX_APPR) && (valid == FALSE)); num++)
            {
              if ( (pc_race_table[ch->race].skin_color[num] != NULL)
              &&   !str_prefix(argument, pc_race_table[ch->race].skin_color[num]) )
              {
                valid = TRUE;
                free_string(ch->pcdata->skin_color);
                ch->pcdata->skin_color = str_dup(pc_race_table[ch->race].skin_color[num], ch->pcdata->skin_color);
              }
            }
          }

          if (!valid)
          {
            printf_to_char(ch, "{cValid skin colors for this race are: ");
            for (num=0; num < MAX_APPR; num++)
            {
              if (pc_race_table[ch->race].skin_color[num] != NULL)
                printf_to_char(ch, "{C%s{c", pc_race_table[ch->race].skin_color[num]);
              if ( ((num+1) < MAX_APPR)
              &&   (pc_race_table[ch->race].skin_color[num+1] != NULL) )
                printf_to_char(ch, ", ");
            }
            printf_to_char(ch, "{x\n\r");
            send_cprompt(ch);
            return;
          }
        } // end skin
        else if (!str_prefix(capitalize(arg),"Height"))
        {
          if (argument[0] == '\0')
            num = 0;
          else
            num = atoi(argument);

          if ( (num < pc_race_table[ch->race].min_height)
          ||   (num > pc_race_table[ch->race].max_height) )
          {
            printf_to_char(ch, "{cThe valid height range for this race is from {C%d{c to {C%d{c.\n\r",
              pc_race_table[ch->race].min_height, pc_race_table[ch->race].max_height);
            send_cprompt(ch);
            return;
          }

          ch->pcdata->height = num;
        } // end height
        else if (!str_prefix(capitalize(arg),"Weight"))
        {
          if (argument[0] == '\0')
            num = 0;
          else
            num = atoi(argument);
  
          if ( (ch->sex == SEX_FEMALE)
          &&   (ch->size >= SIZE_MEDIUM) )
          {
            if ( (num < pc_race_table[ch->race].min_weight)
            ||   (num > (pc_race_table[ch->race].max_weight - 50)) )
            {
              printf_to_char(ch, "{cThe valid weight range for this race is from {C%d{c to {C%d{c.\n\r",
                pc_race_table[ch->race].min_weight, (pc_race_table[ch->race].max_weight - 50));
              send_cprompt(ch);
              return;
            }
          }
          else
          {
            if ( (num < pc_race_table[ch->race].min_weight)
            ||   (num > pc_race_table[ch->race].max_weight) )
            {
              printf_to_char(ch, "{cThe valid weight range for this race is from {C%d{c to {C%d{c.\n\r",
                pc_race_table[ch->race].min_weight, pc_race_table[ch->race].max_weight);
              send_cprompt(ch);
              return;
            }
          }


          ch->pcdata->weight = num;
        } // end weight
        else if (!str_cmp(capitalize(arg),"Done"))
        {
          // Set race stats
          ch->perm_stat[STAT_STR] = pc_race_table[ch->race].stats[STAT_STR];
          ch->perm_stat[STAT_INT] = pc_race_table[ch->race].stats[STAT_INT];
          ch->perm_stat[STAT_WIS] = pc_race_table[ch->race].stats[STAT_WIS];
          ch->perm_stat[STAT_DEX] = pc_race_table[ch->race].stats[STAT_DEX];
          ch->perm_stat[STAT_CON] = pc_race_table[ch->race].stats[STAT_CON];

          // set the char's prompt
          if (ch->gameclass == cMystic)
          {
            ch->martial_style = STYLE_NONE; 
            mprintf(sizeof(buf), buf,"{D[Style: %%S]%%c{D({R%%h{r/%%H{D-{G%%m{g/%%M{D-{B%%v{b/%%V{D)({g%%X{D){c(%%q){Y%%g {W%%s(%w%%%%){x");
          }
          else
            mprintf(sizeof(buf), buf,"{D({R%%h{r/%%H{D-{G%%m{g/%%M{D-{B%%v{b/%%V{D)({g%%X{D){c(%%q){Y%%g {W%%s(%w%%%%){x");
          free_string( ch->prompt );
          ch->prompt = str_dup( buf , ch->prompt);

          // set miscellaneaus bits of info
          SET_BIT(ch->act,PLR_AUTOEXIT);

          // add racial skills
          for (i = 0; i < 5; i++)
          {
            if (pc_race_table[ch->race].skills[i] == NULL)
              break;
            group_add(ch,pc_race_table[ch->race].skills[i],FALSE);
            num = skill_lookup(pc_race_table[ch->race].skills[i]);
            ch->pcdata->learned[num] = 1;
          }

          // setup basic groups
          group_add(ch,"rom basics",FALSE);
          group_add(ch,class_table[ch->gameclass].base_group,FALSE);
          ch->pcdata->learned[gsn_recall] = 50;

          // prompt for next menu and go there
          send_to_char("\n\r{wDo you wish to customize this character?\n\r",ch);
          send_to_char("Customization takes time, but allows a wider range of skills and abilities.\n\r",ch);
          send_to_char("Customize {W(Y/N){x? ",ch);
          d->connected = CON_DEFAULT_CHOICE;

          fOld = FALSE; // don't display menu
          // return; // do we need to not return, not display menu, and continue to next CON_?
        } // end done
        else
        {
          if (arg[0] != '\0')
            printf_to_char(ch, "{YInvalid command.\n\r");
          send_cprompt(ch);
          return;
        }

      } // if a command was entered

      if (fOld)
      {
        mprintf( sizeof(arg), arg, "%s!", ch->name ); // put name right next to !
        printf_to_char(ch, "\n\r\n\r");
        printf_to_char(ch, "{R*{r----------------{D------------------------------------------------{r----------------{R*{x\n\r");
        printf_to_char(ch, "{r|{x                                                                                {r|{x\n\r");
        printf_to_char(ch, "{r|{x Welcome to Draknor, %-30s                             {r|{x\n\r", arg);
        printf_to_char(ch, "{r|{x                                                                                {r|{x\n\r");
        printf_to_char(ch, "{r|{x   At any time feel free to type {ydone{x to enter the game. Your character has     {r|{x\n\r");
        printf_to_char(ch, "{r|{x been chosen at random, based on good race and class combinations.  You can     {r|{x\n\r");
        printf_to_char(ch, "{r|{x create another random combination by typing {yrandom{x, or customize these         {r|{x\n\r");
        printf_to_char(ch, "{r|{x options by typing the keyword you are interested in.  Doing so will give       {r|{x\n\r");
        printf_to_char(ch, "{r|{x you more information about each option.                                        {r|{x\n\r");
        printf_to_char(ch, "{r|{x                                                                                {r|{x\n\r");
        printf_to_char(ch, "{r|{x {RExample:{x race xiranth                                                          {r|{x\n\r");
        printf_to_char(ch, "{D|{x Options listed in {yyellow{x can all be modified.                                  {D|{x\n\r");
        printf_to_char(ch, "{D|{x                                                                                {D|{x\n\r");
        printf_to_char(ch, "{D|{x {cCharacter Attributes                          Game Settings{x                    {D|{x\n\r");
        printf_to_char(ch, "{D|{x --------------------                          -------------                    {D|{x\n\r");
        printf_to_char(ch, "{D|{x {ySex{x:   %-20s                   {yANSI Color{x:   %-10s         {D|{x\n\r",
          (ch->sex == 1 ? "Male" : "Female"), (IS_SET( ch->act, PLR_COLOUR ) ? "On " : "Off") );
        printf_to_char(ch, "{D|{x {yRace{x:  %-20s                   {yNoloot{x:       %-10s         {D|{x\n\r",
          race_table[ch->race].name,
          ( IS_SET( ch->act, PLR_CANLOOT ) ? "Lootable" : (IS_SET(ch->act,PLR_MORGUE) ? "Morgue" : "Safe") ) );
        printf_to_char(ch, "{D|{x {yClass{x: %-20s                   {yScroll Lines{x: %-5d              {D|{x\n\r",
          class_table[ch->gameclass].name,
          ch->lines);
        printf_to_char(ch, "{D|{x {yAlign{x: %s                                {yWimpy{x:        %-5d              {D|{x\n\r",
          (ch->alignment > 500) ? "{yG{Wo{yod{x   " : (ch->alignment < -500) ? "{rEv{Di{rl{x   " : "{wNeutral{x",
          ch->wimpy);
        printf_to_char(ch, "{D|{x                                                                                {D|{x\n\r");
        printf_to_char(ch, "{r|{x {RNote:{x Changing your race will affect your statistics and attributes.           {r|{x\n\r");
        printf_to_char(ch, "{r|{x                                                                                {r|{x\n\r");
        printf_to_char(ch, "{r|{x {cStatistics{x                                    {cPhysical Attributes{x              {r|{x\n\r");
        printf_to_char(ch, "{r|{x ----------                                    -------------------              {r|\n\r");
        printf_to_char(ch, "{r|{x Strength:     %-2d                              {yEye Color{x:    %-18s {r|{x\n\r",
          pc_race_table[ch->race].stats[STAT_STR],
          capitalize(ch->pcdata->eye_color) );
        printf_to_char(ch, "{r|{x Dexterity:    %-2d                              {yHair Color{x:   %-18s {r|{x\n\r",
          pc_race_table[ch->race].stats[STAT_DEX],
          capitalize(ch->pcdata->hair_color) );
        printf_to_char(ch, "{r|{x Intelligence: %-2d                              {ySkin Color{x:   %-18s {r|{x\n\r",
          pc_race_table[ch->race].stats[STAT_INT],
          capitalize(ch->pcdata->skin_color) );

        int feet = 0, inches = 0;
        feet = ch->pcdata->height / 12;
        inches = ch->pcdata->height % 12;

        mprintf( sizeof(arg), arg, "%d in. (%d'%d\")",
          ch->pcdata->height, feet, inches ); // put "in" right next to #

        printf_to_char(ch, "{r|{x Wisdom:       %-2d                              {yHeight{x:       %-18s {r|{x\n\r",
          pc_race_table[ch->race].stats[STAT_WIS],
          arg );

        mprintf( sizeof(arg), arg, "%d lbs.",
          ch->pcdata->weight );

        printf_to_char(ch, "{r|{x Constitution: %-2d                              {yWeight{x:       %-18s {r|{x\n\r",
          pc_race_table[ch->race].stats[STAT_CON],
          arg );
        printf_to_char(ch, "{r|{x                                                                                {r|{x\n\r");
        printf_to_char(ch, "{R*{r---------------{D------------------------------------------------{r-----------------{R*{x\n\r");
      } // if (fOld) (if display menu)

      if (d->connected == CON_NEW_CREATION) // so it doesn't appear after customization prompt
        send_cprompt(ch);
      break; // case CON_NEW_CREATION

/* ************************************************************************* */
/* ****************** Here is where the new creation ends ****************** */
/* ************************************************************************* */

/* *************************************************** */
    case CON_DEFAULT_CHOICE:
      send_to_char("\n\r\n\r",ch);
      switch ( argument[0] )
      {
        case 'y':
        case 'Y':
          ch->gen_data = new_gen_data();
          ch->gen_data->points_chosen = ch->pcdata->points;
          do_function(ch,&do_help,"group header");
          list_group_costs(ch);
          do_function(ch,&do_help,"menu choice");
          d->connected = CON_GEN_GROUPS;
          break;
        case 'n':
        case 'N':
          group_add(ch,class_table[ch->gameclass].default_group,TRUE);
          num = 0;
          for ( i = 0; weapon_table[i].name != NULL; i++)
            if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
              num++;

          if (num < 2)
          {
            for ( i = 0; weapon_table[i].name != NULL; i++)
            {
              if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
              {
                weapon = i;
                ch->pcdata->learned[*weapon_table[weapon].gsn] = 40;
                printf_to_char(ch,"{wYou are given the {B%s{W.\n\r",weapon_table[weapon].name);
#if !defined(ANSIAUTODETECTION)
                if (ch->desc->ansi)
                  SET_BIT(ch->act, PLR_COLOUR);
                else
                  REMOVE_BIT(ch->act, PLR_COLOUR);
#endif
                do_function(ch,&do_help,"motd");
                send_to_char("{D[Press {greturn{D to continue]{x",ch);
                d->connected = CON_READ_MOTD;
                break;
              }
            }
          } // if num > 2
          else
          {
            display_weapon_choices(ch);
            d->connected = CON_PICK_WEAPON;
          }
          break;
        default:
          send_to_char("Please answer (Y/N)? ", ch );
          return;
      } // switch ( argument[0] )
      break; // case CON_DEFAULT_CHOICE

/* *************************************************** */
    case CON_PICK_WEAPON:
      send_to_char("\n\r",ch);

      weapon = weapon_lookup(argument);
      if (weapon == -1 || ch->pcdata->learned[*weapon_table[weapon].gsn] <= 0)
      {
        send_to_char("{RThat's not a valid selection.{x\n\r",ch);
        display_weapon_choices(ch);
        return;
      }

      ch->pcdata->learned[*weapon_table[weapon].gsn] = 40;
      send_to_char("\n\r",ch);
#if !defined(ANSIAUTODETECTION)
      if (ch->desc->ansi)
  SET_BIT(ch->act, PLR_COLOUR);
      else
  REMOVE_BIT(ch->act, PLR_COLOUR);
#endif
      do_function(ch,&do_help,"motd");
      send_to_char("{D[Press {greturn{D to continue]{x",ch);
      d->connected = CON_READ_MOTD;
      break; // case CON_PICK_WEAPON

/* *************************************************** */
    case CON_GEN_GROUPS:
      send_to_char("\n\r",ch);

      if (!str_cmp(argument,"done"))
      {
        if (ch->pcdata->points == pc_race_table[ch->race].points)
        {
          send_to_char("You didn't pick anything.\n\r",ch);
          break;
        }

        if (ch->pcdata->points < 40 + pc_race_table[ch->race].points)
        {
          printf_to_char(ch,"You must take at least %d points of skills and groups.\n\r", 40 + pc_race_table[ch->race].points );
          break;
        }

        if (ch->pcdata->points > MAX_CPS)
        {
          send_to_char("{RYou are over the Maximum amount of Creation points for your character.{x\n\r",ch);
        }
        else
        {
          printf_to_char(ch,"Creation points: %d\n\r",ch->pcdata->points);
          printf_to_char(ch,"Experience per level: %d\n\r",
            exp_per_level(ch,ch->gen_data->points_chosen));

          if (ch->pcdata->points < 40)
            ch->train = (40 - ch->pcdata->points + 1) / 2;

          free_gen_data(ch->gen_data);
          ch->gen_data = NULL;
          write_to_buffer( d, "\n\r", 2 );
          num = 0;

          for ( i = 0; weapon_table[i].name != NULL; i++)
            if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
              num++;

          if (num < 2)
          {
            for ( i = 0; weapon_table[i].name != NULL; i++)
            {
              if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
              {
                weapon = i;
                ch->pcdata->learned[*weapon_table[weapon].gsn] = 40;
                printf_to_char(ch,"You are given the %s.\n\r",weapon_table[weapon].name);
#if !defined(ANSIAUTODETECTION)
                if (ch->desc->ansi)
                  SET_BIT(ch->act, PLR_COLOUR);
                else
                  REMOVE_BIT(ch->act, PLR_COLOUR);
#endif
                do_function(ch,&do_help,"motd");
                send_to_char("{D[Press {greturn{D to continue]{x",ch);
                d->connected = CON_READ_MOTD;
                break;
              }
            }
          } // if (num < 2)
          else
          {
            display_weapon_choices(ch);
            d->connected = CON_PICK_WEAPON;
          }
          break;
        } // else
      } // if (!str_cmp(argument,"done"))

      if (!parse_gen_groups(ch,argument))
        send_to_char( "{WChoices: {C[{Glist,learned,premise,add,drop,info,levels,help,done{C]{x\n\r",ch);

      do_function(ch,&do_help,"menu choice");
      break; // case CON_GEN_GROUPS

/* *************************************************** */
    case CON_READ_IMOTD:
      write_to_buffer(d,"\n\r",2);
      do_function( ch, &do_worklist, "show change" );
      do_function( ch, &do_tasklist, "show" );
      send_to_char("{w[{DPress {gReturn{w to continue]{x",ch);
      d->connected = CON_READ_MOTD;
      break; // case CON_READ_IMOTD

/* *************************************************** */
    case CON_READ_MOTD:
      if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
      {
        write_to_buffer( d, "Warning! Null password!\n\r",0 );
        write_to_buffer( d, "Please report old password with bug.\n\r",0);
        write_to_buffer( d, "Type 'password null <new password>' to fix.\n\r",0);
      }

      send_to_char("\n\r{wWelcome to the {gLand{cs{x of {RD{rr{Da{wk{Dn{ro{Rr{w!!{x\n\r",ch);
      send_to_desc("{D----------------------------------------------------------{x\n\r",d);

      if (d->ansi)
        send_to_desc("{wAutoDetect: {gAnsi {wenabled!{x\n\r",d);
      else
        send_to_desc("AutoDetect: Ansi is Disabled.\n\r",d);

#if !defined(SPARC)
      if (d->out_compress)
      {
        if (d->compressing == TELOPT_COMPRESS2)
          send_to_desc("{CCompression Enabled ->Version 2 of the MCP Protocol. Good Show!{x\n\r",d);
        else
          send_to_desc("{CCompression Enabled ->Version 1 of the MCP Protocol. Good Show!{x\n\r",d);
      }
      else
      {
        send_to_desc("{RCOMPRESSION NOT ENABLED.{x\n\r",d);
        send_to_desc("{WPlease download the compression proxy software from:\n\r",d);
        send_to_desc("http://www.mageslair.net{x.\n\r",d);
      }
#endif
      send_to_desc("{w----------------------------------------------------------{x\n\r",d);
      do_function(ch,&do_unread,"");
      if ( ch->pnote )
      {
        char *tmp = flag_string( spool_flags, ch->pnote->type );
        printf_to_char( ch, "You are working on a %s note.\n\r",tmp);
        printf_to_char(ch, "%s: %s\n\rTo: %s\n\r", ch->pnote->sender, ch->pnote->subject, ch->pnote->to_list );
        page_to_char( ch->pnote->text, ch );
      }

      ch->next  = char_list;
      char_list  = ch;
      d->connected  = CON_PLAYING;
      ch->next_player = player_list;
      player_list = ch;
      reset_char(ch);

      if ( ch->level == 0 )
      {
        ch->perm_stat[class_table[ch->gameclass].attr_prime] += 3;

        ch->level  = 1;
        ch->exp  = 0; // exp_per_level(ch,ch->pcdata->points);
        ch->hit  = ch->max_hit;
        ch->mana  = ch->max_mana;
        ch->move  = ch->max_move;
        ch->train   = 4;
        ch->practice = 6;
        ch->pcdata->condition[COND_THIRST] = 48;
        ch->pcdata->condition[COND_HUNGER] = 48;
        ch->pcdata->condition[COND_FULL] = 48;
        ch->pcdata->condition[COND_DRUNK] = 0;
        ch->pcdata->afktime = 12;

        if ( ch->gameclass == cMystic )
          ch->martial_style = STYLE_NONE;
        ch->birthday = time_info;

/* Set default class title */
        strcpy( buf, default_title_table[ch->gameclass].title );
        set_title( ch, buf );

        ch->gold = 2;
        if (!ch->newbie)
        {
          do_function(ch,&do_outfit,"");
          do_function(ch,&do_autoall,"");
          SET_BIT(ch->chan_flags, CHANNEL_NEWBIE);

          char_to_room( ch, get_room_index( get_start_room(IS_EVIL(ch), IS_GOOD(ch), FALSE)));
        }
        else
        {
          do_function(ch,&do_outfit,"");
          obj_to_char(create_object(get_obj_index(OBJ_VNUM_TGUIDE1),0),ch);
    
          if (IS_EVIL(ch))
            obj_to_char(create_object(get_obj_index(OBJ_VNUM_DREKABUS_MAP),0),ch);
          else if (IS_GOOD(ch))
            obj_to_char(create_object(get_obj_index(OBJ_VNUM_ALINDRAK_MAP),0),ch);
          else
            obj_to_char(create_object(get_obj_index(OBJ_VNUM_PALSARRIEN_MAP),0),ch);

          char_to_room( ch, get_room_index( get_start_room(IS_EVIL(ch), IS_GOOD(ch), FALSE)));

          send_to_char("\n\r",ch);
          do_function(ch,&do_autoall,"");
          send_to_char("{Rautoall done for you. This edits things like autoexit,{x\n\r",ch);
          send_to_char("{Rautogold, autoloot, and more. Autolist will show you{x\n\r",ch);
          send_to_char("{Rall of the options.{x\n\r\n\r",ch);
        } // if (!ch->newbie)
      } // if ( ch->level == 0 )
      else if ( ch->in_room )
      {
        char_to_room( ch, ch->in_room );
      }
      else if ( IS_IMMORTAL(ch) )
      {
        char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
      }
      else
      {
        char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
      }

      for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
      {
        if (!IS_IMMORTAL(ch) || IS_IMMORTAL(vch) )
          act( "$n awakens from the dreams of $g.", ch, NULL, vch, TO_VICT );
      }

      if ( ch->questdata->obj_vnum )
      {
        OBJ_DATA *obj;

        for ( obj = ch->carrying; obj; obj = obj->next_content )
          if ( obj->pIndexData->vnum == ch->questdata->obj_vnum )
            break;

        if ( obj )
        {
            obj_from_char( obj );
            obj_to_room( obj, get_room_index( ch->questdata->room_vnum ) );
        }
      } // if ( ch->questdata->obj_vnum )

      do_function( ch,&do_look, "auto" );

      mprintf( sizeof(buf), buf, "%s has left real life behind.", capitalize( ch->name ) );
      wiznet( buf, ch, NULL, WIZ_LOGINS, 0, get_trust( ch ) );

      if ( ch->clan )
        copy_roster_clannie(ch);

      if (ch->pet != NULL)
      {
        char_to_room(ch->pet,ch->in_room);
        act("$n awakens from the dreams of $g.",ch->pet,NULL,NULL,TO_ROOM);
      }

      break; // case CON_READ_MOTD
  } // switch (d->connected)
  return;
} // new_nanny()

void randomize_char(CHAR_DATA *ch) // pick some random stats
{
  int i, num, race, iClass;
  bool found;

  // Pick a random class
  found = FALSE; // reusing this bool
  i = 0;
  while (!found)
  { // "13" because MAX_CLASS is way off, but we need to account for new classes
    iClass = number_range( 0, 13 );
    if ((iClass != class_lookup("unused1"))
    &&  (iClass != class_lookup("unused2"))
    &&  (iClass != class_lookup("unused3"))
    &&  (iClass != class_lookup("unused4"))
    &&  (iClass != class_lookup("unused5"))
    &&  (iClass != class_lookup("Woodsman")) // disabled, for now
    &&  (iClass != class_lookup("unused6")))
      found = TRUE;

    if (++i > 50)
    {
      iClass = 0;
      found = TRUE;
    }
  }
  ch->gameclass = iClass;

  // Pick random races until we get a half-decent r/c combo
  found = FALSE;
  i = 0;
  while (!found)
  {
    race = number_range( 1, MAX_PC_RACE-1 ); // ignore [0], "null race"
    if ( ( pc_race_table[race].name != NULL )
    &&   ( pc_race_table[race].class_mult[ch->gameclass] > 89 )
    &&   ( pc_race_table[race].class_mult[ch->gameclass] < 105 ) )
      found = TRUE;

    if (++i > 50)
    {
      race = 1;
      found = TRUE;
    }
  }
  ch->race = race;

  // Set race stats to be displayed in menu
  ch->perm_stat[STAT_STR] = pc_race_table[ch->race].stats[STAT_STR];
  ch->perm_stat[STAT_INT] = pc_race_table[ch->race].stats[STAT_INT];
  ch->perm_stat[STAT_WIS] = pc_race_table[ch->race].stats[STAT_WIS];
  ch->perm_stat[STAT_DEX] = pc_race_table[ch->race].stats[STAT_DEX];
  ch->perm_stat[STAT_CON] = pc_race_table[ch->race].stats[STAT_CON];
  ch->size = pc_race_table[race].size;

  // Pick a random alignment, if applicable
  if (IS_CLASS_OCCULTIST(ch))
    ch->alignment = -1000;
  else if (IS_CLASS_INQUISITOR(ch))
    ch->alignment = 1000;
  else
  {
    num = number_range( 0, 2 );
    switch (num)
    {
      case 1:
        ch->alignment = -1000;
        break;
      case 2:
        ch->alignment = 1000;
        break;
      default:
        ch->alignment = 0;
        break;
    }
  }

  // Pick a random sex
  found = FALSE;
  while (!found)
  {
    num = number_range(1,2);
    if (num == 1)
    {
      ch->sex = SEX_MALE;
      ch->pcdata->true_sex = SEX_MALE;
      found = TRUE;
    }
    else if (num == 2)
    {
      ch->sex = SEX_FEMALE;
      ch->pcdata->true_sex = SEX_FEMALE;
      found = TRUE;
    }
  }

  randomize_looks(ch); // must be after sex, due to weight diffs in genders
  return;
}

void randomize_looks(CHAR_DATA *ch) // pick some random stats
{
  int num;
  bool found;

  // pick random attributes based on race restrictions
  ch->pcdata->height = number_range(pc_race_table[ch->race].min_height, pc_race_table[ch->race].max_height);
  if ( (ch->sex == SEX_FEMALE)
  &&   (ch->size >= SIZE_MEDIUM) )
    ch->pcdata->weight = number_range(pc_race_table[ch->race].min_weight, (pc_race_table[ch->race].max_weight - 50));
  else
    ch->pcdata->weight = number_range(pc_race_table[ch->race].min_weight, pc_race_table[ch->race].max_weight);

  found = FALSE;
  while (!found)
  {
    num = number_range( 0, MAX_APPR-1 );
    if (pc_race_table[ch->race].eye_color[num] != NULL)
    {
      free_string(ch->pcdata->eye_color);
      ch->pcdata->eye_color = str_dup(pc_race_table[ch->race].eye_color[num], ch->pcdata->eye_color);
      found = TRUE;
    }
  }

  found = FALSE;
  while (!found)
  {
    num = number_range( 0, MAX_APPR-1 );
    if (pc_race_table[ch->race].hair_color[num] != NULL)
    {
      free_string(ch->pcdata->hair_color);
      ch->pcdata->hair_color = str_dup(pc_race_table[ch->race].hair_color[num], ch->pcdata->hair_color);
      found = TRUE;
    }
  }

  found = FALSE;
  while (!found)
  {
    num = number_range( 0, MAX_APPR-1 );
    if (pc_race_table[ch->race].skin_color[num] != NULL)
    {
      free_string(ch->pcdata->skin_color);
      ch->pcdata->skin_color = str_dup(pc_race_table[ch->race].skin_color[num], ch->pcdata->skin_color);
      found = TRUE;
    }
  }
}

void send_cprompt(CHAR_DATA *ch) // only so it only needs to be set in one place
{
  printf_to_char(ch, "\n\r{gEnter a command, or \"{Gdone{g\" when finished{G:{x ");
  return;
}
