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
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                         DESCRIPTOR_DATA *d              *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/
/***************************************************************************
*  ROM 2.4 is copyright 1993-1996 Russ Taylor                              *
*  ROM has been brought to you by the ROM consortium                       *
*      Russ Taylor (rtaylor@efn.org)                                       *
*      Gabrielle Taylor                                                    *
*      Brian Moore (zump@rom.org)                                          *
*  By using this code, you have agreed to follow the terms of the          *
*  ROM license, in the file Rom24/doc/rom.license                          *
***************************************************************************/
/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

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
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "interp.h"
#include "olc.h"

/*
 * Malloc debugging stuff
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif
void init_signals();
void sig_handler(int signal);

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern  int  malloc_debug  args( ( int  ) );
extern  int  malloc_verify  args( ( void ) );
#endif


/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if  defined(macintosh) || defined(MSDOS)
const  char  echo_off_str  [] = { '\0' };
const  char  echo_on_str  [] = { '\0' };
const  char  go_ahead_str  [] = { '\0' };
const  char  compress_will   [] = { '\0' };
const  char  compress_do     [] = { '\0' };
const  char  compress_dont   [] = { '\0' };
const  char  compress_start  [] = { '\0' };
#ifdef ANSIAUTODETECTION
const  char  ansi_will  [] = { '\0' };
#endif
#endif

#if  defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "telnet.h"

const  char  echo_off_str  [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const  char  echo_on_str  [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const  char  echo_do    [] = { IAC, DO, TELOPT_ECHO, '\0' };
const  char  echo_dont  [] = { IAC, DONT, TELOPT_ECHO, '\0' };
const  char   go_ahead_str  [] = { IAC, GA, '\0' };
#if !defined(SPARC)
/* mccp: compression negotiation strings */
const   char    compress2_will   [] = { IAC, WILL, TELOPT_COMPRESS2, '\0' };
const   char    compress_will   [] = { IAC, WILL, TELOPT_COMPRESS, '\0' };
const   char    compress_do     [] = { IAC, DO, TELOPT_COMPRESS, '\0' };
const   char    compress2_do     [] = { IAC, DO, TELOPT_COMPRESS2, '\0' };
const   char    compress_dont   [] = { IAC, DONT, TELOPT_COMPRESS,'\0' };
const   char    compress2_dont   [] = { IAC, DONT, TELOPT_COMPRESS2,'\0' };
#ifdef ANSIAUTODETECTION
#define CTRL(c) ((c) & 0x1f)
const  char  ansi_will  [] = { CTRL('['), '[', '6', 'n', '\0' };
#endif
#endif
#endif


/*
 * OS-dependent declarations.
 */
#if  defined(_AIX)
#include <sys/select.h>
int      accept        args( ( int s, struct sockaddr *addr, int *addrlen ) );
int      bind        args( ( int s, struct sockaddr *name, int namelen ) );
void  bzero       args( ( char *b, int length ) );
int      getpeername      args( ( int s, struct sockaddr *name, int *namelen ) );
int      getsockname   args( ( int s, struct sockaddr *name, int *namelen ) );
int      gettimeofday  args( ( struct timeval *tp, struct timezone *tzp ) );
int      listen        args( ( int s, int backlog ) );
int      setsockopt      args( ( int s, int level, int optname, void *optval,
                                 int optlen ) );
int      socket        args( ( int domain, int type, int protocol ) );
#endif

#if  defined(apollo)
#include <unistd.h>
void  bzero    args( ( char *b, int length ) );
#endif

#if  defined(__hpux)
int      accept        args( ( int s, void *addr, int *addrlen ) );
int      bind        args( ( int s, const void *addr, int addrlen ) );
void  bzero        args( ( char *b, int length ) );
int      getpeername      args( ( int s, void *addr, int *addrlen ) );
int      getsockname   args( ( int s, void *name, int *addrlen ) );
int      gettimeofday  args( ( struct timeval *tp, struct timezone *tzp ) );
int      listen        args( ( int s, int backlog ) );
int      setsockopt    args( ( int s, int level, int optname,
                               const void *optval, int optlen ) );
int      socket      args( ( int domain, int type, int protocol ) );
#endif

#if  defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if  defined(linux)
/*
    Linux shouldn't need these. If you have a problem compiling, try
    uncommenting accept and bind.
int  accept    args( ( int s, struct sockaddr *addr, int *addrlen ) );
int  bind    args( ( int s, struct sockaddr *name, int namelen ) );
*/

int  close    args( ( int fd ) );
/*
int  getpeername  args( ( int s, struct sockaddr *name, int *namelen ) );
int  getsockname  args( ( int s, struct sockaddr *name, int *namelen ) );
*/
int  gettimeofday  args( ( struct timeval *tp, struct timezone *tzp ) );
/*
int  listen    args( ( int s, int backlog ) );
*/
/*int  read    args( ( int fd, char *buf, int nbyte ) );*/
int  select    args( ( int width, fd_set *readfds, fd_set *writefds,
                       fd_set *exceptfds, struct timeval *timeout ) );
int  socket    args( ( int domain, int type, int protocol ) );
/*int  write    args( ( int fd, char *buf, int nbyte ) );*/
#endif

#if  defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct  timeval
{
  time_t  tv_sec;
  time_t  tv_usec;
};
#if  !defined(isascii)
#define  isascii(c)    ( (c) < 0200 )
#endif
static  long      theKeys  [4];

int  gettimeofday    args( ( struct timeval *tp, void *tzp ) );
#endif

#if  defined(sequent)
int  accept    args( ( int s, struct sockaddr *addr, int *addrlen ) );
int  bind    args( ( int s, struct sockaddr *name, int namelen ) );
int  close    args( ( int fd ) );
int  fcntl    args( ( int fd, int cmd, int arg ) );
int  getpeername  args( ( int s, struct sockaddr *name, int *namelen ) );
int  getsockname  args( ( int s, struct sockaddr *name, int *namelen ) );
int  gettimeofday  args( ( struct timeval *tp, struct timezone *tzp ) );
#if  !defined(htons)
u_short  htons    args( ( u_short hostshort ) );
#endif
int  listen    args( ( int s, int backlog ) );
#if  !defined(ntohl)
u_long  ntohl    args( ( u_long hostlong ) );
#endif
int  read    args( ( int fd, char *buf, int nbyte ) );
int  select    args( ( int width, fd_set *readfds, fd_set *writefds,
                       fd_set *exceptfds, struct timeval *timeout ) );
int  setsockopt  args( ( int s, int level, int optname, caddr_t optval,
                         int optlen ) );
int  socket    args( ( int domain, int type, int protocol ) );
int  write    args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int  accept    args( ( int s, struct sockaddr *addr, int *addrlen ) );
#if !defined(SPARC)
int  bind    args( ( int s, struct sockaddr *name, int
                     namelen ) );
#endif
void  bzero    args( ( char *b, int length ) );
int  close    args( ( int fd ) );
int  getpeername  args( ( int s, struct sockaddr *name, int *namelen ) );
int  getsockname  args( ( int s, struct sockaddr *name, int *namelen ) );
#if !defined(WORK_SRC)
int  gettimeofday  args( ( struct timeval *tp, struct timezone
                           *tzp ) );
#endif
int  listen    args( ( int s, int backlog ) );
#if !defined(WORK_SRC)
int  read    args( ( int fd, char *buf, int nbyte ) );
#endif
int  select    args( ( int width, fd_set *readfds, fd_set *writefds,
                       fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
int setsockopt    args( ( int s, int level, int optname,
                          const char *optval, int optlen ) );
#else
#if !defined(WORK_SRC)
int  setsockopt  args( ( int s, int level, int optname, void *optval,
                         int optlen ) );
#endif
#endif
int  socket    args( ( int domain, int type, int protocol ) );
#if !defined(WORK_SRC)
int  write    args( ( int fd, char *buf, int nbyte ) );
#endif
#endif

#if defined(ultrix)
int  accept    args( ( int s, struct sockaddr *addr, int *addrlen ) );
int  bind    args( ( int s, struct sockaddr *name, int namelen ) );
void  bzero    args( ( char *b, int length ) );
int  close    args( ( int fd ) );
int  getpeername  args( ( int s, struct sockaddr *name, int *namelen ) );
int  getsockname  args( ( int s, struct sockaddr *name, int *namelen ) );
int  gettimeofday  args( ( struct timeval *tp, struct timezone *tzp ) );
int  listen    args( ( int s, int backlog ) );
int  read    args( ( int fd, char *buf, int nbyte ) );
int  select    args( ( int width, fd_set *readfds, fd_set *writefds,
                       fd_set *exceptfds, struct timeval *timeout ) );
int  setsockopt  args( ( int s, int level, int optname, void *optval,
                         int optlen ) );
int  socket    args( ( int domain, int type, int protocol ) );
int  write    args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list = NULL;  /* All open descriptors    */
DESCRIPTOR_DATA *   d_next;    /* Next descriptor in loop  */
FILE *        fpReserve;    /* Reserved file handle    */
bool        god;    /* All new chars are gods!  */
bool        merc_down;    /* Shutdown      */
bool        wizlock;    /* Game is wizlocked    */
bool        newlock;    /* Game is newlocked    */
char        str_boot_time[MAX_INPUT_LENGTH];
time_t        current_time;  /* time of this pulse */
time_t        update_time;  /* time of this pulse */
time_t        lag_update_time;  /* time of this pulse */
time_t        boot_time;  /* time of this pulse */
time_t        tml_boot_time;  /* tml time since game began */
bool        MOBtrigger = TRUE;  /* act() switch                 */

bool      fCopyover;
bool      fCopyoverRecover;

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void  game_loop_mac_msdos  args( ( void ) );
bool  read_from_descriptor  args( ( DESCRIPTOR_DATA *d ) );
bool  write_to_descriptor  args( ( int desc, char *txt, int length ) );
#endif

#if defined(unix)
void  game_loop_unix    args( ( int control ) );
int  init_socket    args( ( int port ) );
void  init_descriptor    args( ( int control ) );
bool  read_from_descriptor  args( ( DESCRIPTOR_DATA *d ) );
#endif




/*
 * Other local functions (OS-independent).
 */
bool  check_reconnect    args( ( DESCRIPTOR_DATA *d, char *name, bool fConn ) );
bool  check_playing    args( ( DESCRIPTOR_DATA *d, char *name ) );
int  main      args( ( int argc, char **argv ) );
void  nanny      args( ( DESCRIPTOR_DATA *d, char *argument ) );
void  new_nanny    args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool  process_output    args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void  read_from_buffer  args( ( DESCRIPTOR_DATA *d ) );
void  stop_idling    args( ( CHAR_DATA *ch ) );
void    bust_a_prompt           args( ( CHAR_DATA *ch ) );
int get_start_room(bool isevil, bool isgood, bool isnewbie);
void mssp_request args( ( DESCRIPTOR_DATA *d ) );
void sig_shutdown();

/* Needs to be global because of do_copyover */
int port, control;
/* Count stuff. */
sh_int      countCur = 0;
sh_int      countMax = 0;
sh_int      countMaxDay = 0;
sh_int      countHour = 0;
sh_int      countArr[24];
sh_int      countMaxDoW[7];
/* This file holds the copyover data. */
#define COPYOVER_FILE    "../area/copyover%04d.dat"
#define COPYOVER_GROUP_FILE  "../area/cgrp%04d.dat"
#define COPYOVER_TIME_FILE  "../Txt/ctime%04d.dat"


int main( int argc, char **argv )
{
  struct timeval now_time;
  bool fCopyOver = FALSE;
  char log_buf[MSL];
  char name[MSL];
  int  len=25;
  /*
   * Memory debugging if needed.
   */
#if defined(MALLOC_DEBUG)
  malloc_debug( 2 );
#endif

  /*
   * Init time.
   */
  gettimeofday( &now_time, NULL );
  current_time   = boot_time = (time_t) now_time.tv_sec;
  strcpy( str_boot_time, ctime( &current_time ) );

  gethostname(name, len);
  logit("Hostname is %s", name);
  /*
   * Macintosh console initialization.
   */
#if defined(macintosh)
  console_options.nrows = 31;
  cshow( stdout );
  csetmode( C_RAW, stdin );
  cecho2file( "log file", 1, stderr );
#endif

  /*
   * Reserve one channel for our use.
   */
  if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
  {
    perror( NULL_FILE );
    exit( 1 );
  }
  nFilesOpen++;
  /*
   * Get the port number.
   */
  port = 4000;
  if ( argc > 1 )
  {
    if ( !is_number( argv[1] ) )
    {
      fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
      exit( 1 );
    }
    else if ( ( port = atoi( argv[1] ) ) <= 1024 )
    {
      fprintf( stderr, "Port number must be above 1024.\n" );
      exit( 1 );
    }
    /* Are we recovering from a copyover? */
    if (argv[2] && argv[2][0])
    {
      fCopyOver = TRUE;
      control = atoi(argv[3]);
    }
    else
    {
      fCopyOver = FALSE;
      tml_boot_time = boot_time;
    }

  }

  /*
   * Run the game.
   */
  if (!fCopyOver)
    control = init_socket( port );

  boot_db();

//  init_web(port+1); // websvr.c


  mprintf(sizeof(log_buf),log_buf, "--Draknor has been successfully started on port %d TIME - %ld.",port, (long)difftime( current_time, boot_time )  );
  log_string( log_buf );


  if (fCopyOver)
    copyover_recover();

  init_signals();
  game_loop_unix( control );
  close (control);

//  shutdown_web();
  /*
   * That's all, folks.
   */
  log_string( "Normal termination of game." );
  exit( 0 );
  return 0;
}



#if defined(unix)
int init_socket( int port )
{
  static struct sockaddr_in sa_zero;
  struct sockaddr_in sa;
  int x = 1;
  int fd;

  if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
  {
    perror( "Init_socket: socket" );
    exit( 1 );
  }

  if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
                   (char *) &x, sizeof(x) ) < 0 )
  {
    bugf( "Init_socket: SO_REUSEADDR" );
    close(fd);
    exit( 1 );
  }
#if !defined(WORK_SRC)
#if defined(SO_DONTLINGER) && !defined(SYSV)
  {
    struct  linger  ld;

    ld.l_onoff  = 1;
    ld.l_linger = 1000;

    if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
                     (char *) &ld, sizeof(ld) ) < 0 )
    {
      perror( "Init_socket: SO_DONTLINGER" );
      close(fd);
      exit( 1 );
    }
  }
#endif
#endif
  sa        = sa_zero;
  sa.sin_family   = AF_INET;
  sa.sin_port      = htons( port );

  if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
  {
    bugf("Init socket: bind" );
    /* too many log files cause of this .. */
    close(fd);
    exit(1);
  }


  if ( listen( fd, 3 ) < 0 )
  {
    perror("Init socket: listen");
    close(fd);
    exit(1);
  }

  return fd;
}
#endif


void check_dup_socket(CHAR_DATA *ch)
{
  DESCRIPTOR_DATA *d, *d_next;
  char log_buf[MSL];
  for ( d = descriptor_list; d != NULL; d = d_next )
  {
    d_next  = d->next;
    if (!strcmp(d->host, ch->desc->host))
    {
      if (d != ch->desc)
      {
        if (d->character)
        {
          mprintf(sizeof(log_buf), log_buf,
                  "2 chars (%s) and (%s) logged SAME SITE: %s." ,
                  d->character->name,
                  ch->name,ch->desc->host);
          if (ch->level > d->character->level)
            wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));
          else
            wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(d->character));
        }
      }
    }
  }
}
/*
 * Close any duplicate character descriptors.
 */
void close_duplicate_sockets( DESCRIPTOR_DATA *d, const char *name )
{
  DESCRIPTOR_DATA *d_old, *d_old_next;

  for ( d_old = descriptor_list; d_old; d_old = d_old_next )
  {
    d_old_next = d_old->next;

    if ( d_old == d || d_old->character == NULL )
      continue;

    if ( str_cmp( name, CH(d_old)->name ) )
      continue;

    close_socket( d_old );
  }
}

#if defined(unix)
void game_loop_unix( int control )
{
  static struct timeval null_time;
  struct timeval last_time;
  time_t start_time;
  pthread_t thread;

  signal( SIGPIPE, SIG_IGN );
  gettimeofday( &last_time, NULL );
  current_time = (time_t) last_time.tv_sec;
  lag_update_time = 0;
  /* Main loop */
  while ( !merc_down )
  {
    fd_set in_set;
    fd_set out_set;
    fd_set exc_set;
    DESCRIPTOR_DATA *d;
    int maxdesc;

#if defined(MALLOC_DEBUG)
    if ( malloc_verify( ) != 1 )
      abort( );
#endif
    gettimeofday( &last_time, NULL );
    start_time = (time_t) last_time.tv_sec;

    /*
     * Poll all active descriptors.
     */
    FD_ZERO( &in_set  );
    FD_ZERO( &out_set );
    FD_ZERO( &exc_set );
    FD_SET( control, &in_set );
    maxdesc  = control;
    for ( d = descriptor_list; d; d = d->next )
    {
      maxdesc = UMAX( maxdesc, d->descriptor );
      FD_SET( d->descriptor, &in_set  );
      FD_SET( d->descriptor, &out_set );
      FD_SET( d->descriptor, &exc_set );
    }

    if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
    {
      perror( "Game_loop: select: poll" );
      exit( 1 );
    }

    /*
     * New connection?
     */
    if ( FD_ISSET( control, &in_set ) )
    {
#if !defined(SPARC)
      pthread_create( &thread, NULL,(void *)init_descriptor,(void *)((long int)control) );
      pthread_detach(thread);
#else
      init_descriptor(control);
#endif
    }
    /*
     * Kick out the freaky folks.
     */
    for ( d = descriptor_list; d; d = d_next )
    {
      d_next = d->next;

      if ( FD_ISSET( d->descriptor, &exc_set ) )
      {
        FD_CLR( d->descriptor, &in_set  );
        FD_CLR( d->descriptor, &out_set );

        if ( d->character && d->connected == CON_PLAYING)
          save_char_obj( d->character, FALSE );
        d->outtop  = 0;
        close_socket( d );
      }
    }

    /*
     * Process input.
     */
    for ( d = descriptor_list; d; d = d_next )
    {
      d_next  = d->next;
      d->fcommand  = FALSE;

      if ( FD_ISSET( d->descriptor, &in_set ) )
      {
        if ( d->character )
          d->character->timer = 0;

        if ( !read_from_descriptor( d ) )
        {
          FD_CLR( d->descriptor, &out_set );

          if ( d->character && d->connected == CON_PLAYING)
            save_char_obj( d->character, FALSE );

          d->outtop  = 0;
          close_socket( d );
          continue;
        }
      }

      if (d->character && d->character->daze > 0)
        --d->character->daze;

      if ( d->character && d->character->wait > 0 )
      {
        --d->character->wait;
        continue;
      }

      read_from_buffer( d );

      if ( d->incomm[0] )
      {
        d->fcommand  = TRUE;

        if (d->character)
          stop_idling( d->character );

        /* OLC */
        if ( d->showstr_point )
          show_string( d, d->incomm );
        else
          if ( d->pString )
            string_add( d->character, d->incomm );
          else
            switch ( d->connected )
            {
              case CON_PLAYING:
                if ( !run_olc_editor( d ) )
                  substitute_alias( d, d->incomm );
                break;
              default:
                new_nanny( d, d->incomm );
                break;
            }

        d->incomm[0]  = '\0';
      }
    }

//      handle_web(); // websvr.c

    /*
     * Autonomous game motion.
     */
    update_handler( );

    /*
     * Output.
     */
    for ( d = descriptor_list; d; d = d_next )
    {
      d_next = d->next;

      if ( ( d->fcommand || d->outtop > 0
#if !defined(SPARC)
             || d->out_compress
#endif
           )
           &&   FD_ISSET(d->descriptor, &out_set) )
      {
        bool ok = TRUE;

        if ( d->fcommand || d->outtop > 0 )
          ok = process_output( d, TRUE );

#if !defined(SPARC)
        if (ok && d->out_compress)
          ok = processCompressed(d);
#endif
        if (!ok)
        {
          if ( d->character && d->connected == CON_PLAYING)
            save_char_obj( d->character, FALSE );
          d->outtop  = 0;
          close_socket( d );
        }
      }
    }

    /*
     * Synchronize to a clock.
     * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
     * Careful here of signed versus unsigned arithmetic.
     */
    {
      struct timeval now_time;
      long secDelta;
      long usecDelta;

      gettimeofday( &now_time, NULL );
      usecDelta  = ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
                   + 1000000 / PULSE_PER_SECOND;
      secDelta  = ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
      while ( usecDelta < 0 )
      {
        usecDelta += 1000000;
        secDelta  -= 1;
      }

      while ( usecDelta >= 1000000 )
      {
        usecDelta -= 1000000;
        secDelta  += 1;
      }

      if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
      {
        struct timeval stall_time;

        stall_time.tv_usec = usecDelta;
        stall_time.tv_sec  = secDelta;
        if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
        {
          perror( "Game_loop: select: stall" );
          exit( 1 );
        }
      }
    }

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;
    update_time = current_time - start_time;
    if (update_time > lag_update_time)
      lag_update_time = update_time;
  }

  return;
}
#endif



#if defined(unix)

void init_descriptor( int control )
{
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *dnew;
  struct sockaddr_in sock;
  struct hostent *from;
  int desc;
  unsigned int size;
  char log_buf[MSL];

  size = sizeof(sock);
  getsockname( control, (struct sockaddr *) &sock, &size );
  if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
  {
    perror( "New_descriptor: accept" );
    pthread_exit(NULL);
  }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

  if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
  {
    perror( "New_descriptor: fcntl: FNDELAY" );
    pthread_exit(NULL);
  }

  /*
   * Cons a new descriptor.
   */
  dnew = new_descriptor(); /* new_descriptor now also allocates things */
  dnew->descriptor = desc;
  dnew->character = NULL;

  size = sizeof(sock);
  if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
  {
    perror( "New_descriptor: getpeername" );
    dnew->host = str_dup( "(unknown)",dnew->host );
  }
  else
  {
    int addr;

    addr = ntohl( sock.sin_addr.s_addr );
    mprintf(sizeof(buf), buf, "%d.%d.%d.%d",
            ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
            ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
           );
    mprintf(sizeof(log_buf), log_buf, "Sock.sinaddr:  %s", buf );
    log_string( log_buf );
    from = gethostbyaddr( (char *) &sock.sin_addr,
                          sizeof(sock.sin_addr), AF_INET );
    dnew->host = str_dup( from ? from->h_name : buf , dnew->host);
  }

  if ( check_ban(dnew->host,BAN_ALL))
  {
    write_to_descriptor_2( desc,
                           "Your site has been banned from this mud.\n\r", 0 );
    close( desc );
    free_descriptor(dnew);
    pthread_exit(NULL);
  }
  /*
   * Init descriptor data.
   */
  dnew->next      = descriptor_list;
  descriptor_list    = dnew;

  /*
   * Send the greeting.
   */

#if !defined(SPARC)
  /* mccp: tell the client we support compression */
  write_to_buffer( dnew, compress2_will, 0 );
  write_to_buffer( dnew, compress_will, 0 );
#endif
// INTROO
  dnew->ansi = TRUE;

  {
    dnew->connected = CON_GET_NAME;
    extern char * help_greeting;
    if (help_greeting)
    {
      send_to_desc("\n\r", dnew );
      if ( help_greeting[0] == '.' )
        send_to_desc(help_greeting+1, dnew );
      else
        send_to_desc(help_greeting, dnew );
    }
  }
  write_to_buffer( dnew, echo_off_str, 0 );
  send_to_desc("\n\rDraknor Copyright Robert Leonard (2003){x\n\r", dnew );
  send_to_desc("Draknor is based on Sacred 1.0: Litchfield & Archambault(97-03){x\n\r", dnew );
  send_to_desc("Sacred is based on ROM 2.4 beta, copyright Russ Taylor(93-96)\n\r", dnew );
  send_to_desc("ROM is based on Merc 2.1: Furey, Hatchet, Kahn.\n\r", dnew );
  send_to_desc("Merc is based on Diku: Staerfeldt, Madsen, Nyboe, Seifert & Hammer\n\n\r", dnew );
  send_to_desc("Who are thee that wish {WEntrance{x to these Lands? ", dnew );
  write_to_buffer( dnew, echo_on_str, 0 );

  pthread_exit(NULL);
}
#endif



void close_socket( DESCRIPTOR_DATA *dclose )
{
  CHAR_DATA *ch;
  char buf[MAX_INPUT_LENGTH];

  if ( dclose->outtop > 0 )
    process_output( dclose, FALSE );

  /*  Attempt at pk fixing
  if (dclose->character) {
    if ((is_affected(dclose->character,AFF_CHARM)) ||
  (is_affected(dclose->character, AFF_SLEEP)) ||
  (is_affected(dclose->character, AFF_CURSE)))
      return;
  }
  */
  if ( dclose->snoop_by != NULL )
  {
    write_to_buffer( dclose->snoop_by,
                     "Your snooped character has left the game.\n\r", 0 );
  }

  {
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
      if ( d->snoop_by == dclose )
        d->snoop_by = NULL;
    }
  }

  if ( ( ch = dclose->character ) != NULL )
  {
    /*mprintf(sizeof(log_buf), log_buf, "Closing link to %s.", ch->name );
    log_string( log_buf );*/
    /* cut down on wiznet spam when rebooting */
    /* add this */
    if ( ch->pet && ch->pet->in_room == NULL )
    {
      char_to_room( ch->pet, get_room_index(ROOM_VNUM_LIMBO) );
      extract_char( ch->pet, TRUE );
    }
    if ( dclose->connected == CON_PLAYING && !merc_down )
    {
      act( "$n has lost $s link.", ch, NULL, NULL, TO_WIZ_ROOM );
      logit( "%s has fallen linkdead.", ch->name);
      if (!IS_NPC(ch))
      {
        SET_BIT(ch->affected_by, AFF_LINKDEATH);
        if (ch->idle_snapshot == 0)
          ch->idle_snapshot = current_time;
      }
      else
      {
        if (ch->desc)
        {
          if (ch->desc->original)
          {
            SET_BIT(ch->desc->original->affected_by,
                    AFF_LINKDEATH);
          }
        }
      }

      mprintf( sizeof(buf), buf, "Net death has claimed %s.",
               capitalize( ch->name ) );

      if (IS_IMMORTAL(ch))
        wiznet(buf,ch,NULL,WIZ_LINKS,0,IMPLEMENTOR);
      else
        wiznet(buf,ch,NULL,WIZ_LINKS,0,0);
      ch->desc = NULL;
    }
    else
    {
      nuke_pets(dclose->original ? dclose->original :
                dclose->character, TRUE);
      free_char(dclose->original ? dclose->original :
                dclose->character );
    }
  }

  if ( d_next == dclose )
    d_next = d_next->next;

  if ( dclose == descriptor_list )
  {
    descriptor_list = descriptor_list->next;
  }
  else
  {
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d && d->next != dclose; d = d->next )
      ;
    if ( d )
      d->next = dclose->next;
    else
      bug( "Close_socket: dclose not found.", 0 );
  }

#if !defined(SPARC)
  if (dclose->out_compress)
  {
    deflateEnd(dclose->out_compress);
#if OLD_MEM
    free_mem(dclose->out_compress_buf, COMPRESS_BUF_SIZE);
    free_mem(dclose->out_compress, sizeof(z_stream));
#else
    free_mem(dclose->out_compress_buf);
    free_mem(dclose->out_compress );
#endif
  }
#endif

  close( dclose->descriptor );
  free_descriptor(dclose);
  count_update();
#if defined(MSDOS) || defined(macintosh)
  exit(1);
#endif
  return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
  int iStart;
  char log_buf[MSL];

  /* Hold horses if pending command already. */
  if ( d->incomm[0] != '\0' )
    return TRUE;

  /* Check for overflow. */
  iStart = strlen(d->inbuf);
  if ( iStart >= sizeof(d->inbuf) - 10 )
  {
    mprintf(sizeof(log_buf), log_buf, "%s input overflow!", d->host );
    log_string( log_buf );
    write_to_descriptor( d,
                         "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
    return FALSE;
  }

  /* Snarf input. */
#if defined(macintosh)
  for ( ; ; )
  {
    int c;
    c = getc( stdin );
    if ( c == '\0' || c == EOF )
      break;
    putc( c, stdout );
    if ( c == '\r' )
      putc( '\n', stdout );
    d->inbuf[iStart++] = c;
    if ( iStart > sizeof(d->inbuf) - 10 )
      break;
  }
#endif

#if defined(MSDOS) || defined(unix)
  for ( ; ; )
  {
    int nRead;

    nRead = read( d->descriptor, d->inbuf + iStart,
                  sizeof(d->inbuf) - 10 - iStart );
    if ( nRead > 0 )
    {
      iStart += nRead;
      if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
        break;
    }
    else if ( nRead == 0 )
    {
      log_string( "EOF encountered on read - Character Linkdead or Gone." );
      return FALSE;
    }
    else if ( errno == EWOULDBLOCK )
      break;
    else
    {
      perror( "Read_from_descriptor" );
      return FALSE;
    }
  }
#endif

  d->inbuf[iStart] = '\0';
  return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
  int i, j, k, cmd_num;
  char cmd_copy[MAX_STRING_LENGTH];
  bool found=FALSE;
  char buf[MAX_INPUT_LENGTH];

  /*
   * Hold horses if pending command already.
   */
  if ( d->incomm[0] != '\0' || d->inbuf[0] == '\0' )
    return;
  /*  logf( "%hx %hx %hx %hx %hx %hx %hx %hx %hx %hx",
      (char)d->inbuf[0], (char)d->inbuf[1], (char)d->inbuf[2],
      (char)d->inbuf[3], (char)d->inbuf[4], (char)d->inbuf[5],
      (char)d->inbuf[6], (char)d->inbuf[7], (char)d->inbuf[8], (char)d->inbuf[9] );
  */

  //#ifdef ANSIAUTODETECTION
  /* ANSI escape sequence. */
  /* MCCP Telnet checking */
  while ( d->inbuf[0] == (signed char)IAC )
  {
    if ( !memcmp( &d->inbuf[0], compress_do, strlen( compress_do ) ) )
    {
      i = strlen( compress_do );
      compressStart( d , TELOPT_COMPRESS);
    }
    if ( !memcmp( &d->inbuf[0], compress2_do, strlen( compress2_do ) ) )
    {
      i = strlen( compress2_do );
      compressStart( d , TELOPT_COMPRESS2);
    }
    else if ( !memcmp( &d->inbuf[0], compress_dont, strlen( compress_dont ) ) )
    {
      i = strlen( compress_dont );
      if (d->compressing == TELOPT_COMPRESS)
        compressEnd( d );
    }
    else if ( !memcmp( &d->inbuf[0], compress2_dont, strlen( compress2_dont ) ) )
    {
      i = strlen( compress_dont );
      if (d->compressing == TELOPT_COMPRESS2)
        compressEnd( d );
    }
    else if ( !memcmp( &d->inbuf[0], echo_do, strlen( echo_do ) ) )
    {
      i = strlen( echo_do );
    }
    else if ( !memcmp( &d->inbuf[0], echo_dont, strlen( echo_dont ) ) )
    {
      i = strlen( echo_dont );
    }
    else
      break;

    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ );
  }

  if ( d->inbuf[0] == CTRL('[') && d->inbuf[1] == '[' )
  {
    int line, col;
    if ( sscanf( &d->inbuf[1], "[%d;%dR", &line, &col ) == 2 )
    {
      i = 2;
      while ( d->inbuf[i] != 'R' )
        i++;
      for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j+1] ) != '\0'; j++ )
        ;
      d->ansi = TRUE;
    }
  }
  //#endif

  /*
   * Look for at least one new line.
   */
  for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
  {
    if ( d->inbuf[i] == '\0' )
      return;
  }

  /*
   * Canonical input processing.
   */
  for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
  {
    if ( k >= MAX_INPUT_LENGTH - 2 )
    {
      write_to_descriptor( d, "Line too long.\n\r", 0 );

      /* skip the rest of the line */
      for ( ; d->inbuf[i] != '\0'; i++ )
      {
        if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
          break;
      }
      d->inbuf[i]   = '\n';
      d->inbuf[i+1] = '\0';
      break;
    }

    if ( d->inbuf[i] == '\b' && k > 0 )
      --k;
    else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
      d->incomm[k++] = d->inbuf[i];
#if !defined(SPARC)
    else if (d->inbuf[i] == (signed char)IAC)
    {
      if (!memcmp(&d->inbuf[i], compress_do, strlen(compress_do)))
      {
        i += strlen(compress_do) - 1;
        compressStart(d, TELOPT_COMPRESS);
      }
      if (!memcmp(&d->inbuf[i], compress2_do, strlen(compress2_do)))
      {
        i += strlen(compress2_do) - 1;
        compressStart(d, TELOPT_COMPRESS2);
      }
      else if (!memcmp(&d->inbuf[i], compress_dont, strlen(compress_dont)))
      {
        i += strlen(compress_dont) - 1;
        compressEnd(d);
      }
    }
#endif
    //#ifdef ANSIAUTODETECTION
    /* ANSI escape sequence. */
    else if ( d->inbuf[i] == CTRL('[') && d->inbuf[i+1] == '[' )
    {
      int line, col;

      if ( sscanf( &d->inbuf[i+1], "[%d;%dR", &line, &col ) == 2 )
      {
        i += 2;
        while ( d->inbuf[i] != 'R' )
          i++;
        d->ansi = TRUE;
      }
      else
        d->ansi = FALSE;
    }
  }
  /*
   * Finish off the line.
   */
  if ( k == 0 )
    d->incomm[k++] = ' ';
  d->incomm[k] = '\0';

  /*
   * Deal with bozos with #repeat 1000 ...
   */

  if ( k > 1 || d->incomm[0] == '!' || d->incomm[0] != ' ' )
  {
    if ( (d->incomm[0] != '!'
          &&   strcmp( d->incomm, d->inlast )))
    {
      d->repeat = 0;
    }
    else
    {
      if (++d->repeat >= 25 && d->character
          &&  d->connected == CON_PLAYING)
      {
        if (!IS_SET(d->character->act,PLR_FREEZE))
        {
          logit("%s input spamming!", d->host );
          mprintf( sizeof(buf), buf, "Spam spam spam %s spam spam spam spam spam!",
                   capitalize( d->character->name ) );

          wiznet(buf, d->character,NULL,WIZ_SPAM,0,get_trust(d->character));

          if (d->incomm[0] == '!')
          {
            smash_dollar(d->inlast);
            wiznet(d->inlast,d->character,NULL,WIZ_SPAM,0,
                   get_trust(d->character));
          }
          else
          {
            smash_dollar(d->incomm);
            wiznet(d->incomm,d->character,NULL,WIZ_SPAM,0,
                   get_trust(d->character));
          }
        }

        d->repeat = 0;
        if ( !IS_IMMORTAL(d->character) )
        {
          write_to_descriptor( d, "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
          strcpy( d->incomm, "quit" );
        }
      }
    }
  }


  /*
   * Do '!' substitution.
   */
  if (!d->character)
  {
    if ( d->incomm[0] == '!' )
      strcpy( d->incomm, d->inlast );
    else
      strcpy( d->inlast, d->incomm );
  }
  else
  {

    if (d->incomm[0] == '!')
    {
      strcpy(cmd_copy, d->incomm + 1);
      strip_string(cmd_copy);
      if (cmd_copy[0] == '\0' || cmd_copy[0] == '!')
      {
        strcpy(d->incomm, d->inlast);
        found = TRUE;
      }
      else if (is_number(cmd_copy))
      {
        cmd_num = atoi(cmd_copy);
        if (cmd_num < MAX_COMMAND_HISTORY && d->character->cmd_hist[cmd_num].text[0] != '\0')
        {
          strcpy(d->incomm, d->character->cmd_hist[cmd_num].text);
          strcpy(d->inlast, d->incomm);
          found = TRUE;
        }
        else
        {
          write_to_descriptor(d, "Not that many commands in your history.\n\r", 0);
          strcpy(d->incomm, "\n");
          strcpy(d->inlast, d->incomm);
          while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
            i++;
          for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
            ;
          return;
        }
      }
      else
      {
        for (cmd_num = 0; cmd_num < MAX_COMMAND_HISTORY; cmd_num++)
        {
          if (!str_prefix(cmd_copy, d->character->cmd_hist[cmd_num].text) && !found)
          {
            strcpy(d->incomm, d->character->cmd_hist[cmd_num].text);
            strcpy(d->inlast, d->incomm);
            found = TRUE;
          }
        }
      }
      if (!found)
      {
        write_to_descriptor(d, "No such command in your history.\n\r", 0);
        strcpy(d->incomm, "\n");
        strcpy(d->inlast, d->incomm);
        while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
          i++;
        for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
          ;
        return;
      }
    }
    else
    {
      strcpy(d->inlast, d->incomm);
    }

//    if (d->incomm[0] != '\n' && d->incomm[0] != '\r' && d->incomm[0] != '\0' && str_cmp(d->incomm, " "))
// trying to remove directions
    if ( ( d->incomm[0] != '\n' )
         && ( d->incomm[0] != '\r' )
         && ( d->incomm[0] != '\0' )
         && ( str_prefix( d->incomm, "north" ) )
         && ( str_prefix( d->incomm, "south" ) )
         && ( str_prefix( d->incomm, "east" ) )
         && ( str_prefix( d->incomm, "west" ) )
         && ( str_prefix( d->incomm, "up" ) )
         && ( str_prefix( d->incomm, "down" ) )
         && ( str_cmp(d->incomm, " ") ) )
    {
      for (cmd_num = MAX_COMMAND_HISTORY - 1; cmd_num > 0; cmd_num--)
      {
        if (!IS_NULLSTR(d->character->cmd_hist[cmd_num].text))
          free_string(d->character->cmd_hist[cmd_num].text);
        d->character->cmd_hist[cmd_num].text = str_dup(d->character->cmd_hist[cmd_num - 1].text, d->character->cmd_hist[cmd_num].text);
      }
      if (!IS_NULLSTR(d->character->cmd_hist[0].text))
        free_string(d->character->cmd_hist[0].text);
      d->character->cmd_hist[0].text = str_dup(d->incomm, d->character->cmd_hist[0].text);
    }
  }

  /*
   * Shift the input buffer.
   */
  while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
    i++;
  for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
    ;
  return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
  extern bool merc_down;
  char temp_str[150];
  int found=0;
  /*  char temp_buf[70000];*/
  FILE *fp;
  int i;
  CHAR_DATA *ch;
  CHAR_DATA *victim;

  /*
   * Bust a prompt.
   */
  if ( !merc_down )
    ch = d->character;

  if ( d->showstr_point && fPrompt && !merc_down)
    send_to_char("{W[{gPaging Text{w: {GReturn {wto {gContinue, {GQ {wto {gquit{W]{x\n\r",d->character);
  else if ( fPrompt && d->pString && d->connected == CON_PLAYING && !merc_down )
    send_to_desc("> ", d );
  else if ( !merc_down && fPrompt && d->connected == CON_PLAYING )
  {
    ch = d->character;

    /* battle prompt */
    if ( ( victim = ch->fighting ) && can_see( ch, victim ) )
    {
      int percent;
      char wound[100];
      char *pbuff;
      char buf[MAX_STRING_LENGTH];
      char buffer[MAX_STRING_LENGTH*2];

      if (victim->max_hit > 0)
        percent = victim->hit * 100 / victim->max_hit;
      else
        percent = -1;

      if (percent >= 100)
        mprintf(sizeof(wound), wound,"is in excellent condition.");
      else if ( percent >= 97 )
        mprintf(sizeof(wound), wound, "is in good condition." );
      else if ( percent >= 90 )
        mprintf(sizeof(wound), wound,"has a few scratches.");
      else if ( percent >= 80 )
        mprintf(sizeof(wound), wound,"has some scratches.");
      else if ( percent >= 70 )
        mprintf(sizeof(wound), wound,"has some small wounds and bruises.");
      else if (percent >= 60 )
        mprintf(sizeof(wound), wound,"has some wounds and bruises.");
      else if (percent >= 50)
        mprintf(sizeof(wound), wound,"has quite a few wounds.");
      else if (percent >= 40)
        mprintf(sizeof(wound), wound,"has some big wounds and scratches.");
      else if (percent >= 30)
        mprintf(sizeof(wound), wound,"has some big nasty wounds and scratches.");
      else if (percent >= 15)
        mprintf(sizeof(wound), wound,"looks pretty hurt.");
      else if (percent >= 7)
        mprintf(sizeof(wound), wound,"looks very hurt.");
      else if (percent >= 0)
        mprintf(sizeof(wound), wound,"is in awful condition.");
      else
        mprintf(sizeof(wound), wound,"is {rbleeding{x to death.");

      mprintf(sizeof(buf), buf,"%s %s \n\r",
              IS_NPC( victim ) ? victim->short_descr : victim->name, wound );
      buf[0]  = UPPER( buf[0] );
      pbuff  = buffer;
      colourconv( pbuff, buf, d->character );
      write_to_buffer( d, buffer, 0);
    }


    ch = d->original ? d->original : d->character;
    if (!IS_SET(ch->comm_flags, COMM_COMPACT))
      write_to_buffer( d, "\n\r", 2 );


    if ( IS_SET(ch->comm_flags, COMM_PROMPT) )
    {
      bust_a_prompt( d->character );
    }
    if (IS_SET(ch->plr2,PLR2_TELNET_GA))
      write_to_buffer(d,go_ahead_str,0);
  }

  /*
   * Short-circuit if nothing to write.
   */
  if ( d->outtop == 0 )
    return TRUE;


  /*
   * Auto-Snoop-o-rama.
   */
  if ( d->character )
  {
    found = 0;
    for (i=0; i < MAX_NUM_AUTOSNOOP; i++)
    {
      if (!strcmp(as_string[i],d->character->name))
      {
        found = i;
      }
    }
    if (found)
    {
      mprintf(sizeof(temp_str),temp_str,"%s/%s.txt",AS_DIR_STRING,as_string[found]);
      if ((fp = fopen(temp_str,"a")) != NULL)
      {
        nFilesOpen++;
        if (d->outtop > 0)
          fwrite(d->outbuf, d->outtop, 1, fp);
        else
          fwrite(d->outbuf, strlen(d->outbuf),1,fp);
        fclose(fp);
        nFilesOpen--;
      }
      else
      {
        perror("Autosnoop Failed to open File.");
      }
    }
  }

  if ( d->snoop_by != NULL )
  {

    if (d->character != NULL)
      write_to_buffer( d->snoop_by, d->character->name,0);
    write_to_buffer( d->snoop_by, "> ", 2 );
    write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
  }

  /*
   * OS-dependent output.
   */
  if ( !write_to_descriptor( d, d->outbuf, d->outtop ) )
  {
    d->outtop = 0;
    return FALSE;
  }
  else
  {
    d->outtop = 0;
    return TRUE;
  }
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt( CHAR_DATA *ch )
{
  CHAR_DATA *gch = NULL;
  CHAR_DATA *sch = NULL;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  const char *str;
  const char *i;
  char *point;
  char *pbuff;
  char buffer[ MAX_STRING_LENGTH*2 ];
  char doors[MAX_INPUT_LENGTH];
  EXIT_DATA *pexit;
  bool found;
  bool suppress_p = FALSE;
  bool suppress_o = FALSE;
  bool suppress_q = FALSE;
  bool suppress_h = FALSE;
  bool suppress_t = FALSE;
  bool suppress_f = FALSE;
  const char *dir_name[] = {"N","E","S","W","U","D"};
  int door;
  int carry_weight;
  int can_carry_weight;
  int weight_percentage;
  int tnlprint = 0;
  int v1;

  point = buf;
  str = ch->prompt;
  if (IS_AFK(ch))
  {
    if (ch->tells)
      printf_to_char(ch, "{W[{gA{GF{gK{W]{x{W[{R%d{W]{x ", ch->tells);
    else
      printf_to_char(ch, "{W[{gA{GF{gK{W]{x ");
  }

  if (IS_SWITCHED(ch))
    printf_to_char(ch,"{B[{WSWITCH{B]{r<{x%s{r>{x", ch->name);

  if ( !IS_SET(ch->comm_flags, COMM_NO_OLC_PROMPT)
       &&    ch->desc
       &&    ch->desc->editor )
    printf_to_char(ch,"{B[{cOLC %s %s{B]{x ",
                   olc_ed_name( ch ),
                   olc_ed_vnum( ch ) );

  if ( !str || str[0] == '\0')
  {
    printf_to_char(ch, "{w({R%d{r/%d{w-{G%d{g/%d{w-{B%d{b/%d{w)({g%d{w){x %s",
                   ch->hit,GET_HP(ch),ch->mana,GET_MANA(ch), ch->move,ch->max_move,
                   TNL( exp_per_level(ch,ch->pcdata->points), ch->level ) - ch->exp, ch->prefix );
    return;
  }

  for ( sch = char_list; sch != NULL; sch = sch->next )
  {
    if ( IS_AFFECTED(sch, AFF_CHARM)
         &&   sch->master == ch )
    {
      gch = sch;
      break;
    }
  }


  while ( *str != '\0' )
  {
    if (*str == '?')
    {
      switch (*(str+1))
      {
        case 'p': // "?p" found, start IfPet prompt conditional
          if ( gch != NULL )
            suppress_p = FALSE;
          else
            suppress_p = TRUE;
          str = str + 2;
          continue;
          break;
        case 'o': // "?o" found, start IfOLC prompt conditional
          if ( IS_IMMORTAL(ch) )
          {
            suppress_o = TRUE;
            if ( ( ch->desc )
                 &&   ( ch->desc->editor ) )
              suppress_o = FALSE;
            str = str + 2;
            continue;
          }
          break;
        case 'q': // "?q" found, start IfQuest prompt conditional
          if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_QUESTING) )
            suppress_q = FALSE;
          else
            suppress_q = TRUE;
          str = str + 2;
          continue;
          break;
        case 'h': // "?h" found, start IfHungry prompt conditional
          if ( !IS_NPC(ch) && ch->pcdata->condition[COND_HUNGER] == 0 )
            suppress_h = FALSE;
          else
            suppress_h = TRUE;
          str = str + 2;
          continue;
          break;
        case 't': // "?t" found, start IfThirsty prompt conditional
          if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] == 0 )
            suppress_t = FALSE;
          else
            suppress_t = TRUE;
          str = str + 2;
          continue;
          break;
        case 'f': // "?t" found, start IfThirsty prompt conditional
          if ( ch->fighting )
            suppress_f = FALSE;
          else
            suppress_f = TRUE;
          str = str + 2;
          continue;
          break;
      }
    } // conditional

    if (*str == '|')
    {
      switch (*(str+1))
      {
        case 'p': // "|p" found, switch IfPet prompt conditional
          if ( gch != NULL )
            suppress_p = TRUE;
          else
            suppress_p = FALSE;
          str = str + 2;
          continue;
          break;
        case 'o': // "|o" found, switch IfOLC prompt conditional
          if ( IS_IMMORTAL(ch) )
          {
            suppress_o = FALSE;
            if ( ( ch->desc )
                 &&   ( ch->desc->editor ) )
              suppress_o = TRUE;
            str = str + 2;
            continue;
          }
          break;
        case 'q': // "|q" found, switch IfQuest prompt conditional
          if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_QUESTING) )
            suppress_q = TRUE;
          else
            suppress_q = FALSE;
          str = str + 2;
          continue;
          break;
        case 'h': // "|h" found, switch IfHungry prompt conditional
          if ( !IS_NPC(ch) && ch->pcdata->condition[COND_HUNGER] == 0 )
            suppress_h = TRUE;
          else
            suppress_h = FALSE;
          str = str + 2;
          continue;
          break;
        case 't': // "|t" found, switch IfThirsty prompt conditional
          if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] == 0 )
            suppress_t = TRUE;
          else
            suppress_t = FALSE;
          str = str + 2;
          continue;
          break;
        case 'f': // "|t" found, switch IfThirsty prompt conditional
          if ( ch->fighting )
            suppress_f = TRUE;
          else
            suppress_f = FALSE;
          str = str + 2;
          continue;
          break;
      }
    } // else

    if (*str == '!')
    {
      switch (*(str+1))
      {
        case 'p': // "!p" found, end IfPet prompt conditional if in one
          suppress_p = FALSE;
          str = str + 2;
          continue;
          break;
        case 'o': // "!o" found, end IfOLC prompt conditional if in one
          if ( IS_IMMORTAL(ch) )
          {
            suppress_o = FALSE;
            str = str + 2;
            continue;
          }
          break;
        case 'q': // "!q" found, end IfQuest prompt conditional if in one
          suppress_q = FALSE;
          str = str + 2;
          continue;
          break;
        case 'h': // "!h" found, end IfHungry prompt conditional if in one
          suppress_h = FALSE;
          str = str + 2;
          continue;
          break;
        case 't': // "!t" found, end IfThirsty prompt conditional if in one
          suppress_t = FALSE;
          str = str + 2;
          continue;
          break;
        case 'f': // "!t" found, end IfThirsty prompt conditional if in one
          suppress_f = FALSE;
          str = str + 2;
          continue;
          break;
      }
    } // end

    if ( (suppress_p)
         ||   (suppress_h)
         ||   (suppress_t)
         ||   (suppress_f)
         ||   (suppress_o)
         ||   (suppress_q) )
    {
      str++;
      continue;
    }

    if ( ( *str != '%' )
         &&   ( *str != '#' ) )
    {
      *point++ = *str++;
      continue;
    }

    if (*str == '%' )
    {
      ++str;
      switch ( *str )
      {
        default :
          i = " ";
          break;
        case 'e':
          found = FALSE;
          doors[0] = '\0';
          if (!IS_AFFECTED(ch,AFF_BLIND) || IS_SET( ch->act, PLR_HOLYLIGHT ))
          {
            for (door = 0; door < 6; door++)
            {
              if ((pexit = ch->in_room->exit[door]) != NULL
                  &&  pexit ->u1.to_room != NULL
                  &&  (can_see_room(ch,pexit->u1.to_room)
                       ||   (IS_AFFECTED(ch,AFF_INFRARED)
                             &&    !IS_AFFECTED(ch,AFF_BLIND)))
                  &&  !IS_SET(pexit->exit_info,EX_HIDDEN))
              {
                found = TRUE;
                strcat(doors,"{G(");
                strcat( doors, dir_name[door] );
                strcat(doors,"{G){x");
              }
            }
          }
          if (!found)
            strcat(buf,"none");
          mprintf(sizeof(buf2), buf2,"%s",doors);
          i = buf2;
          break;
        case 'E' : // OLC: name of thing being edited
// I know this looks funky and IS_IMMORTAL seems like it should be outside,
// but I did it this way for limited local variables, and a faster prompt
          if (!IS_IMMORTAL( ch ) )
            mprintf( sizeof(buf2), buf2, "" );
          else
          {
            AREA_DATA *ppArea;
            OBJ_INDEX_DATA *ppObj;
            MOB_INDEX_DATA *ppMob;
            MPROG_CODE *ppMprog;
            HELP_DATA *ppHelp;
            CLAN_DATA *ppClan;

            switch (ch->desc->editor)
            {
              case ED_ROOM:
                mprintf( sizeof(buf2), buf2, "%s", ch->in_room->name);
                break;

              case ED_AREA:
                ppArea = (AREA_DATA *)ch->desc->pEdit;
                mprintf(sizeof(buf2),buf2, "%s", ppArea ? ppArea->name : "" );
                break;

              case ED_OBJECT:
                ppObj = (OBJ_INDEX_DATA *)ch->desc->pEdit;
                mprintf(sizeof(buf2),buf2, "%s", ppObj ? ppObj->short_descr : "" );
                break;

              case ED_MOBILE:
                ppMob = (MOB_INDEX_DATA *)ch->desc->pEdit;
                mprintf(sizeof(buf2),buf2, "%s", ppMob ? ppMob->short_descr : "" );
                break;

              case ED_MPCODE:
                ppMprog = (MPROG_CODE *)ch->desc->pEdit;
                mprintf(sizeof(buf2),buf2, "%s", ppMprog ? ppMprog->name : "" );
                break;

              case ED_HELP:
                ppHelp = (HELP_DATA *)ch->desc->pEdit;
                mprintf(sizeof(buf2),buf2, "%s", ppHelp ? ppHelp->keyword : "" );
                break;

              case ED_CLAN:
                ppClan = ( CLAN_DATA *)ch->desc->pEdit;
                mprintf( sizeof( buf2 ), buf2, "%s", ppClan ? ppClan->name : "" );
                break;

              default:
                mprintf( sizeof(buf2), buf2, "" );
                break;
            } // end switch
          } // end if isimm
          i = buf2;
          break;
				case 'b' :
					if ( !IS_NPC(ch) && IS_CLASS_OCCULTIST(ch) )
						mprintf(sizeof(buf2), buf2, "%d", ch->pcdata->bloodshards );
					else
						mprintf( sizeof(buf2), buf2, "" );
					i = buf2;
					break;
        case 'c' :
          mprintf(sizeof(buf2), buf2,"%s","\n\r");
          i = buf2;
          break;
        case 'h' :
          mprintf( sizeof(buf2), buf2, "%d", ch->hit );
          i = buf2;
          break;
        case 'H' :
          mprintf( sizeof(buf2), buf2, "%d", GET_HP(ch) );
          i = buf2;
          break;
        case 'm' :
          mprintf( sizeof(buf2), buf2, "%d", ch->mana );
          i = buf2;
          break;
        case 'M' :
          mprintf( sizeof(buf2), buf2, "%d", GET_MANA(ch) );
          i = buf2;
          break;
        case 'v' :
          mprintf( sizeof(buf2), buf2, "%d", ch->move );
          i = buf2;
          break;
        case 'V' :
          mprintf( sizeof(buf2), buf2, "%d", ch->max_move );
          i = buf2;
          break;
        case 'x' :
          mprintf( sizeof(buf2), buf2, "%d", ch->exp );
          i = buf2;
          break;
        case 'X' :
          tnlprint = TNL( exp_per_level(ch,ch->pcdata->points), ch->level ) - ch->exp;
          if (tnlprint < 0)
            tnlprint = tnlprint * -1;
          mprintf(sizeof(buf2), buf2, "%d",
                  IS_NPC(ch) ? 0 : tnlprint);
          i = buf2;
          break;
        case 'g' :
          mprintf(sizeof(buf2), buf2, "%ld", ch->gold);
          i = buf2;
          break;
        case 's' :
          mprintf( sizeof(buf2), buf2, "%ld", ch->silver);
          i = buf2;
          break;
        case 'S' :
          if ( ch->gameclass == cMystic )
            mprintf( sizeof(buf2), buf2, "%s", martial_style_prompt(ch) );
          else
            mprintf( sizeof(buf2), buf2, "%s", "" );
          i = buf2;
          break;
        case 'a' :
          if ( ch->level > 9 )
            mprintf( sizeof(buf2), buf2, "%d", ch->alignment );
          else
            mprintf( sizeof(buf2), buf2, "%s", IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? "evil" : "neutral" );
          i = buf2;
          break;
        case 'r' :
          if ( ch->in_room != NULL )
            mprintf( sizeof(buf2), buf2, "%s",
                     ((!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT)) ||
                      (!IS_AFFECTED(ch,AFF_BLIND) && !room_is_dark( ch->in_room )))
                     ? ch->in_room->name : "darkness");
          else
            mprintf( sizeof(buf2), buf2, " " );
          i = buf2;
          break;
        case 'R' :
          if ( IS_IMMORTAL( ch ) && ch->in_room != NULL )
            mprintf( sizeof(buf2), buf2, "%d", ch->in_room->vnum );
          else
            mprintf( sizeof(buf2), buf2, " " );
          i = buf2;
          break;
        case 'T' :
          if ( IS_IMMORTAL( ch ) )
            mprintf( sizeof(buf2), buf2, "%s", asctime(localtime( &current_time)));
          else
            mprintf( sizeof(buf2), buf2, " " );
          i = buf2;
          break;
        case 'W' :
          if ( IS_IMMORTAL( ch ) && ch->invis_level )
            mprintf( sizeof(buf2), buf2, "%d", ch->invis_level );
          else
            mprintf( sizeof(buf2), buf2, "OFF" );
          i = buf2;
          break;
        case 'I' :
          if ( IS_IMMORTAL( ch ) && ch->incog_level )
            mprintf( sizeof(buf2), buf2, "%d", ch->incog_level );
          else
            mprintf( sizeof(buf2), buf2, "OFF" );
          i = buf2;
          break;
        case 'z' :
          if ( ch->in_room != NULL )
            mprintf( sizeof(buf2), buf2, "%s", ch->in_room->area->name );
          else
            mprintf( sizeof(buf2), buf2, " " );
          i = buf2;
          break;
        case '%' :
          mprintf( sizeof(buf2), buf2, "%%" );
          i = buf2;
          break;
        case 'o' :
          mprintf( sizeof(buf2), buf2, "%s", olc_ed_name(ch) );
          i = buf2;
          break;
        case 'O' :
          mprintf( sizeof(buf2), buf2, "%s", olc_ed_vnum(ch) );
          i = buf2;
          break;
        case 'w' :
          carry_weight      = (get_carry_weight(ch) / 10);
          can_carry_weight  = (can_carry_w(ch) /10);
          weight_percentage = (100*carry_weight / can_carry_weight);
          mprintf( sizeof(buf2), buf2, "%d", weight_percentage );
          i = buf2;
          break;
        case 'q' :
          if (!IS_NPC(ch))
          {
            if (IS_SET(ch->act, PLR_QUESTING))//if (IS_QUESTING(ch))
            {
              if (ch->questdata->countdown > 0)
                mprintf( sizeof(buf2), buf2, "%d", ch->questdata->countdown );
            }
            else if (!IS_SET(ch->act, PLR_QUESTING))
            {
              if (ch->questdata->nextquest > 0)
                mprintf( sizeof(buf2), buf2, "%d", ch->questdata->nextquest );
              else
                mprintf( sizeof(buf2), buf2, "%d", ch->questdata->curr_points );
            }
          }
          i = buf2;
          break;
        case 'G' :
          v1 = 0;
          for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
            if ( is_same_group( gch, ch ) )
              v1++;
          mprintf(sizeof(buf2), buf2, "%d", v1);
          i = buf2;
          break;
        case 'p' :
          mprintf(sizeof(buf2), buf2, "%d", ch->practice);
          i = buf2;
          break;
        case 't' :
          mprintf(sizeof(buf2), buf2, "%d", ch->train);
          i = buf2;
          break;
      }
    } // '%' data
    else if (*str == '#' )
    {
      ++str;
      switch ( *str )
      {
        default :
          i = " ";
          break;
        case '%' :
          mprintf( sizeof(buf2), buf2, "%%" );
          i = buf2;
          break;
        case '#' :
          mprintf( sizeof(buf2), buf2, "#" );
          i = buf2;
          break;
        case 'h' :
          mprintf( sizeof(buf2), buf2, "%d", gch == NULL ? 0 : gch->hit );
          i = buf2;
          break;
        case 'H' :
          mprintf( sizeof(buf2), buf2, "%d", gch == NULL ? 0 : GET_HP(gch) );
          i = buf2;
          break;
        case 'm' :
          mprintf( sizeof(buf2), buf2, "%d", gch == NULL ? 0 : gch->mana );
          i = buf2;
          break;
        case 'M' :
          mprintf( sizeof(buf2), buf2, "%d", gch == NULL ? 0 : GET_MANA(gch) );
          i = buf2;
          break;
        case 'v' :
          mprintf( sizeof(buf2), buf2, "%d", gch == NULL ? 0 : gch->move );
          i = buf2;
          break;
        case 'V' :
          mprintf( sizeof(buf2), buf2, "%d", gch == NULL ? 0 : gch->max_move );
          i = buf2;
          break;
        case 'n' :
          mprintf( sizeof(buf2), buf2, "%s", gch == NULL ? "No pet" : ( IS_NPC(gch) ? gch->short_descr : gch->name ) );
          i = buf2;
          break;
        case 'f' :
          if (ch->fighting)
          {
            if (can_see(ch,ch->fighting))
              mprintf( sizeof(buf2), buf2, "%s", ( IS_NPC(ch->fighting) ? ch->fighting->short_descr : ch->fighting->name ) );
            else
              mprintf( sizeof(buf2), buf2, "%s", "Someone" );
          }
          else
            mprintf( sizeof(buf2), buf2, "%s", "None" );

          i = buf2;
          break;
        case 'F' :
          if (ch->fighting)
          {
            if (can_see(ch,ch->fighting))
            {
              v1 = PERCENT( ch->fighting->hit, ch->fighting->max_hit );
              if ( v1 >= 100 ) strcpy( buf2, "{R[{r=========={R]{x" );
              else if ( v1 <= 0 ) strcpy( buf2, "{R[{D----------{R]{x" );
              else
                sprintf( buf2, "{R[{r%.*s{R+{D%.*s{R]{x",
                         v1/10,   "=========",
                         9-v1/10, "---------" );
            }
            else
              strcpy( buf2, "{R[{D??????????{R]{x" );
          }
          else
            strcpy( buf2, "{R[{D----------{R]{x" );
          i = buf2;
          break;
        case 'R' :
          mprintf( sizeof(buf2), buf2, "%d.%02d", ch->ruby_counter,     (ch->ruby_fragment     * 100 / 250000) );
          i = buf2;
          break;
        case 'S' :
          mprintf( sizeof(buf2), buf2, "%d.%02d", ch->sapphire_counter, (ch->sapphire_fragment * 100 / 200000) );
          i = buf2;
          break;
        case 'E' :
          mprintf( sizeof(buf2), buf2, "%d.%02d", ch->emerald_counter,  (ch->emerald_fragment  * 100 / 150000) );
          i = buf2;
          break;
        case 'D' :
          mprintf( sizeof(buf2), buf2, "%d.%02d", ch->diamond_counter,  (ch->diamond_fragment  * 100 / 100000) );

          i = buf2;
          break;
      }
    } // '#' data
    else
    {
      mprintf( sizeof(buf2), buf2, "");
      i = buf2;
    }

    ++str;
    while ( (*point = *i) != '\0' )
      ++point, ++i;
  } // while loop through prompt

  *point = '\0';
  pbuff = buffer;
  colourconv( pbuff, buf, ch );
  write_to_buffer( ch->desc, buffer, 0 );
  if ( ch->prefix[0] )
    write_to_buffer(ch->desc,ch->prefix,0);
  return;
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
  /*
   * Find length in case caller didn't.
   */
  if ( length <= 0 )
    length = strlen(txt);

  /*
   * Initial \n\r if needed.
   */
  if ( d->outtop == 0 && !d->fcommand )
  {
    d->outbuf[0]  = '\n';
    d->outbuf[1]  = '\r';
    d->outtop  = 2;
  }

  /*
   * Expand the buffer as needed.
   */
  while ( d->outtop + length >= d->outsize )
  {
    char *outbuf;

    if (d->outsize >= 32767)
    {
      bugf("Socket: %d Buffer overflow. d->outsize = %d Closing.\n\r",d->descriptor, d->outsize);
      close_socket(d);
      return;
    }
    outbuf      = alloc_mem( 2 * d->outsize );
    strncpy( outbuf, d->outbuf, d->outtop );
    free_mem( d->outbuf );
    d->outbuf   = outbuf;
    d->outsize *= 2;
  }

  /*
   * Shrink the buffer as space allows.
   */
  while ( d->outsize > DEFAULT_DESC_OUTSIZE
          && d->outtop + length < d->outsize / 2 )
  {
    char *outbuf;

    outbuf      = alloc_mem( d->outsize / 2 );
    strncpy( outbuf, d->outbuf, d->outtop );
    free_mem( d->outbuf  );
    d->outbuf   = outbuf;
    d->outsize /= 2;
  }
  /*
   * Copy.
   */
  strncpy( d->outbuf + d->outtop, txt, length );
  d->outtop += length;
  return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor_2( int desc, char *txt, int length )
{
  int iStart;
  int nWrite;
  int nBlock;
#if defined(macintosh) || defined(MSDOS)
  if ( desc == 0 )
    desc = 1;
#endif

  if ( length <= 0 )
    length = strlen(txt);

  for ( iStart = 0; iStart < length; iStart += nWrite )
  {
    nBlock = UMIN( length - iStart, 4096 );
    /*      strncpy(tempbuf,txt+iStart,nBlock);
      printf(tempbuf);*/
    if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
    {
      perror( "Write_to_descriptor" );
      return FALSE;
    }
  }

  return TRUE;
}


/* mccp: write_to_descriptor wrapper */
bool write_to_descriptor(DESCRIPTOR_DATA *d, char *txt, int length)
{
#if !defined(SPARC)
  if (d->out_compress)
    return writeCompressed(d, txt, length);
  else
#endif
    return write_to_descriptor_2(d->descriptor, txt, length);
}


/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
  int clan;
  DENIED_DATA *dnd;
  /*
   * Reserved words.
   */
  if ( is_exact_name( name,
                      "all auto immortal self someone something the you wanderer light velg larn defender cookie") )
    return FALSE;

  if (str_cmp(capitalize(name),"Alander") && (!str_prefix("Alan",name)
      || !str_suffix("Alander",name)))
    return FALSE;

  /*
   * Length restrictions.
   */

  if ( strlen(name) <  2 )
    return FALSE;

#if defined(MSDOS)
  if ( strlen(name) >  8 )
    return FALSE;
#endif

#if defined(macintosh) || defined(unix)
  if ( strlen(name) > 12 )
    return FALSE;
#endif

  /*
   * Alphanumerics only.
   * Lock out IllIll twits.
   */
  {
    char *pc;
    bool fIll,adjcaps = FALSE,cleancaps = FALSE;
    int total_caps = 0;

    fIll = TRUE;
    for ( pc = name; *pc != '\0'; pc++ )
    {
      if ( !isalpha(*pc) )
        return FALSE;

      if ( isupper(*pc)) /* ugly anti-caps hack */
      {
        if (adjcaps)
          cleancaps = TRUE;
        total_caps++;
        adjcaps = TRUE;
      }
      else
        adjcaps = FALSE;

      if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
        fIll = FALSE;
    }

    if ( fIll )
      return FALSE;

    if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
      return FALSE;
  }

  /* check clans */
  for (clan = 0; clan < MAX_CLAN; clan++)
  {
    if (LOWER(name[0]) == LOWER(clan_table[clan].name[0])
        &&  !str_cmp(name,clan_table[clan].name))
      return FALSE;
  }

  /*
   * Prevent players from naming themselves after mobs.
   */
  {
    extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
    MOB_INDEX_DATA *pMobIndex;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
      for ( pMobIndex  = mob_index_hash[iHash];
            pMobIndex != NULL;
            pMobIndex  = pMobIndex->next )
      {
        if ( is_name( name, pMobIndex->player_name ) )
          return FALSE;
      }
    }
  }
  /* Lock out all denied char names. */
  for (dnd = denied_list; dnd; dnd=dnd->next)
    if (is_exact_name(capitalize(name),dnd->name))
      return FALSE;
  return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
  CHAR_DATA *ch;
  char log_buf[MSL];
  for ( ch = player_list; ch != NULL; ch = ch->next_player )
  {
    if ( !IS_NPC(ch)
         &&   (!fConn || ch->desc == NULL)
         &&   !str_cmp( d->character->name, ch->name ) )
    {
      if ( fConn == FALSE )
      {
        free_string( d->character->pcdata->pwd );
        d->character->pcdata->pwd = str_dup( ch->pcdata->pwd , d->character->pcdata->pwd);
      }
      else
      {
        nuke_pets( d->character, TRUE );
        free_char( d->character );
        d->character = ch;
        ch->desc   = d;
        ch->timer   = 0;
        if (!ch->in_room)
        {
          bugf("Linkdead In_ROOM NULL error.");
          move_to_room(ch, get_room_index(ROOM_VNUM_TEMPLE));
        }
        send_to_char(  "Reconnecting. Type replay to see missed tells.\n\r", ch );

        act( "$n has reconnected.", ch, NULL, NULL, TO_WIZ_ROOM );
        REMOVE_BIT(ch->affected_by,AFF_LINKDEATH);
        if (ch->idle_snapshot != 0)
        {
          ch->idle_time += current_time - ch->idle_snapshot;
          ch->idle_snapshot = 0;
        }
        mprintf( sizeof(log_buf), log_buf, "%s@%s reconnected.", ch->name, d->host );
        log_string( log_buf );


        char buf[MAX_INPUT_LENGTH];
        mprintf( sizeof(buf), buf, "%s groks the fullness of %s link.",
                 capitalize( ch->name ),
                 ch->sex == 2 ? "her" : "his" );
        if (IS_IMMORTAL(ch))
          wiznet(buf,    ch,NULL,WIZ_LINKS,0,IMPLEMENTOR);
        else
          wiznet(buf,    ch,NULL,WIZ_LINKS,0,0);
        d->connected = CON_PLAYING;
      }
      return TRUE;
    }
  }

  return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
  DESCRIPTOR_DATA *dold;

  for ( dold = descriptor_list; dold; dold = dold->next )
  {
    if ( dold != d
         &&   dold->character != NULL
         &&   dold->connected != CON_GET_NAME
         &&   dold->connected != CON_GET_OLD_PASSWORD
         &&   !str_cmp( name, CH(dold)->name ) )
    {
      write_to_buffer( d, "That character is already playing.\n\r",0);
      write_to_buffer( d, "Do you wish to connect anyway (Y/N)?",0);
      d->connected = CON_BREAK_CONNECT;
      return TRUE;
    }
  }

  return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
  CHAR_DATA *vch;

  if ( ch == NULL
       ||   ch->desc == NULL
       ||   ch->desc->connected != CON_PLAYING
       ||   ch->was_in_room == NULL
       ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
    return;

  ch->timer = 0;
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
  {
    if (vch->master == ch && IS_PET(vch))
    {
      move_to_room(vch, vch->was_in_room);
      vch->was_in_room  = NULL;
    }
  }
  move_to_room( ch, ch->was_in_room );
  ch->was_in_room  = NULL;
  if (ch->idle_snapshot != 0)
  {
    ch->idle_time += current_time - ch->idle_snapshot;
    ch->idle_snapshot = 0;
  }
  act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
  return;
}



/*
 * Write to one char.
 */
void send_to_char_bw( const char *txt, CHAR_DATA *ch )
{
  if ( txt != NULL && ch->desc != NULL )
    write_to_buffer( ch->desc, txt, strlen(txt) );
  return;
}

/*
 * Write to one char, new colour version, by Lope.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
  const  char   *point;
  char   *point2;
  char   buf[ MAX_STRING_LENGTH*4 ];
  int  skip = 0;

  buf[0] = '\0';
  point2 = buf;
  if ( txt && ch->desc )
  {
    if ( IS_SET( ch->act, PLR_COLOUR ) )
    {
      for ( point = txt ; *point ; point++ )
      {
        if ( *point == '{' )
        {
          point++;
          //if (!IS_NPC(ch))
          skip = colour( *point, point2 );
          // else skip =0;
          while ( skip-- > 0 )
            ++point2;
          continue;
        }
        *point2 = *point;
        *++point2 = '\0';
      }
      *point2 = '\0';
      write_to_buffer( ch->desc, buf, point2 - buf );
    }
    else
    {
      for ( point = txt ; *point ; point++ )
      {
        if ( *point == '{' )
        {
          point++;
          continue;
        }
        *point2 = *point;
        *++point2 = '\0';
      }
      *point2 = '\0';
      write_to_buffer( ch->desc, buf, point2 - buf );
    }
  }
  return;
}
/*
 * Write to one char, new colour version, by Lope.
 */
void send_to_desc( const char *txt, DESCRIPTOR_DATA *d )
{
  const  char   *point;
  char   *point2;
  char   buf[ MAX_STRING_LENGTH*4 ];
  int  skip = 0;

  buf[0] = '\0';
  point2 = buf;
  if ( txt && d )
  {
    if (d->ansi)
    {
      for ( point = txt ; *point ; point++ )
      {
        if ( *point == '{' )
        {
          point++;
          skip = colour( *point, point2 );
          while ( skip-- > 0 )
            ++point2;
          continue;
        }
        *point2 = *point;
        *++point2 = '\0';
      }
      *point2 = '\0';
      write_to_buffer( d, buf, point2 - buf );
    }
    else
    {
      for ( point = txt ; *point ; point++ )
      {
        if ( *point == '{' )
        {
          point++;
          continue;
        }
        *point2 = *point;
        *++point2 = '\0';
      }
      *point2 = '\0';
      write_to_buffer(d, buf, point2 - buf );
    }
  }
  return;
}

/*
 * Send a page to one char.
 */
void page_to_char_bw( const char *txt, CHAR_DATA *ch )
{
  if ( txt == NULL || ch->desc == NULL)


    if (ch->lines == 0 )
    {
      send_to_char(txt,ch);
      return;
    }

#if defined(macintosh)
  send_to_char(txt,ch);
#else
  ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
  strcpy(ch->desc->showstr_head,txt);
  ch->desc->showstr_point = ch->desc->showstr_head;
  show_string(ch->desc,"\0");
#endif
}

#if AR_PAGE
/*
 * Does the character translate into an ansi string?
 */
bool is_color_char( char c )
{
  switch ( c )
  {
    case 'n':

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case 'A':
    case 'U':
    case 'G':
    case 'R':
    case 'C':
    case 'P':
    case 'O':
    case 'W':
    case 'a':
    case 'u':
    case 'g':
    case 'r':
    case 'c':
    case 'p':
    case 'o':
    case 'w':

    case 'b':
    case 'B':
    case 'f':
    case 'F':
    case ':':
    case '_':

    case '<':
      return TRUE;
  }
  return FALSE;
}


/*
 * Does the character get replaced with non-ansi string?
 */
bool is_replace_char( char c )
{
  switch ( c )
  {
    case 'v':
    case 'x':
    case '-':
      return TRUE;
  }
  return FALSE;
}



/*
 * Return number of pages in string pager based on scroll lines of character.
 */
int get_page_count( const char *txt, int show_lines )
{
  const char  *scan;
  const char  *last = NULL;
  int   lines = 0;
  int   pages = 1;
  bool  toggle = TRUE;
  bool  fNoExtra = TRUE;

  for ( scan = txt; *scan != '\0'; scan++ )
  {
    if ( *scan == '#' && *++scan == '\0' )
      break;
    if ( ( *scan == '\n' || *scan == '\r' ) && ( toggle = !toggle ) )
      if ( ++lines >= show_lines )
      {
        lines = 0;
        ++pages;
        last = scan;
      }

  }

  if ( lines || !last )
    return pages;

  for ( last++; ; last++ )
  {
    if ( *last == '#' )
    {
      if ( is_color_char( *++last ) )
        continue;
      else if ( *last == '\0' )
        break;
      else if ( is_replace_char( *last ) )
      {
        fNoExtra = FALSE;
        break;
      }
      continue;
    }
    if ( isspace( *last ) )
      continue;
    if ( !*last )
      break;
    fNoExtra = FALSE;
    break;
  }

  return fNoExtra ? pages - 1: pages;
}


/*
 * Send a page to one char.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
  int size;

  if ( txt == NULL || ch->desc == NULL)
    return;

  if ( ch->lines == 0 )
  {
    send_to_char( txt, ch );
    return;
  }

#if defined(macintosh)
  send_to_char( txt, ch );
  return;
#else

  /* If there's already data there, free it. */
  if ( ch->desc->showstr_head )
  {
    free_mem( ch->desc->showstr_head);
    ch->desc->showstr_head = NULL;
  }

  size = strlen( txt ) + 1;

  ch->desc->showstr_size   = size;
  ch->desc->showstr_head   = alloc_mem( size );
  strcpy( ch->desc->showstr_head, txt );

  ch->desc->showstr_point   = ch->desc->showstr_head;
  ch->desc->showstr_page   = 1;
  ch->desc->showstr_page_count = get_page_count( txt, ch->lines );

  /*  ch->desc->showstr_ansi   = ch->desc->ansi;
  ch->desc->showstr_ansi_prev  = ch->desc->ansi_prev;
  */
  show_string( ch->desc, "first" );

  /* Testing, testing */
  /*
  if ( ch->desc->showstr_head
  &&   ( strlen( txt ) + strlen( ch->desc->showstr_head ) + 1 ) < 32000 )
  {
  char *temp = alloc_mem( strlen( txt )
  + strlen( ch->desc->showstr_head ) + 1 );
  strcpy( temp, ch->desc->showstr_head );
  strcat( temp, txt );
  ch->desc->showstr_point = temp
  +  ( ch->desc->showstr_point - ch->desc->showstr_head );
  free_mem( ch->desc->showstr_head,
  strlen( ch->desc->showstr_head ) + 1 );
  ch->desc->showstr_head = temp;
  }
  else
  {
  if ( ch->desc->showstr_head )
  free_mem( ch->desc->showstr_head,
  strlen( ch->desc->showstr_head ) + 1 );
  ch->desc->showstr_head = alloc_mem( strlen( txt ) + 1 );
  strcpy( ch->desc->showstr_head, txt );
  ch->desc->showstr_point = ch->desc->showstr_head;
  show_string( ch->desc, "" );
  }
  */
#endif
}


/*
 * Go to a specific page in the string pager.
 */
bool goto_page( DESCRIPTOR_DATA *d, sh_int show_page )
{
  char  *scan;
  char  *last_point  = NULL;
  int   show_lines;
  int   lines    = 0;
  sh_int  last_page  = 0;
  sh_int  pages    = 1;
  bool  toggle    = TRUE;
  bool  ob_act;
  bool  reset    = FALSE;
  bool  color_code  = FALSE;

  if ( show_page <= 1 )
  {
    /*      d->ansi    = d->showstr_ansi;
      d->ansi_prev   = d->showstr_ansi_prev; */
    d->showstr_point = d->showstr_head;
    d->showstr_page  = 1;
    return TRUE;
  }

  if ( show_page > d->showstr_page_count )
    show_page = d->showstr_page_count;

  if ( d->character )
    show_lines = d->character->lines;
  else
    return FALSE;
  /*
  d->ansi     = d->showstr_ansi;
  d->ansi_prev   = d->showstr_ansi_prev;
  */
  for ( scan = d->showstr_head; *scan != '\0'; scan++ )
  {
    if ( *scan != '#' )
    {
      if ( ( *scan == '\n' || *scan == '\r' ) && ( toggle = !toggle ) )
      {
        if ( ++lines >= show_lines )
        {
          lines = 0;
          if ( ++pages == show_page )
          {
            d->showstr_point = scan;
            d->showstr_page  = show_page;
            return TRUE;
          }
          else
          {
            last_point = scan;
            last_page  = show_page;
          }
        }
      }
      color_code = FALSE;
      continue;
    }
    /*
    if ( !color_code && *( scan + 1 ) != '<' )
    d->ansi_prev = d->ansi;
    */
    color_code = TRUE;
    /*
    switch ( *++scan )
    {
    default:   ob_act = FALSE; break;
    case '\0': ob_act = FALSE; scan--; break;

    case '<':  d->ansi = d->ansi_prev; ob_act = TRUE; break;

    case '0':  d->ansi.color = 0; ob_act = TRUE; break;
    case '1':  d->ansi.color = 1; ob_act = TRUE; break;
    case '2':  d->ansi.color = 2; ob_act = TRUE; break;
    case '3':  d->ansi.color = 3; ob_act = TRUE; break;
    case '4':  d->ansi.color = 4; ob_act = TRUE; break;
    case '5':  d->ansi.color = 5; ob_act = TRUE; break;
    case '6':  d->ansi.color = 6; ob_act = TRUE; break;
    case '7':  d->ansi.color = 7; ob_act = TRUE; break;

    case 'A':  d->ansi.color = 0; ob_act = d->ansi.bold = TRUE; break;
    case 'U':  d->ansi.color = 1; ob_act = d->ansi.bold = TRUE; break;
    case 'G':  d->ansi.color = 2; ob_act = d->ansi.bold = TRUE; break;
    case 'R':  d->ansi.color = 3; ob_act = d->ansi.bold = TRUE; break;
    case 'C':  d->ansi.color = 4; ob_act = d->ansi.bold = TRUE; break;
    case 'P':  d->ansi.color = 5; ob_act = d->ansi.bold = TRUE; break;
    case 'O':  d->ansi.color = 6; ob_act = d->ansi.bold = TRUE; break;
    case 'W':  d->ansi.color = 7; ob_act = d->ansi.bold = TRUE; break;

    case 'a':  d->ansi.color = 0; ob_act = TRUE; d->ansi.bold = FALSE; break;
    case 'u':  d->ansi.color = 1; ob_act = TRUE; d->ansi.bold = FALSE; break;
    case 'g':  d->ansi.color = 2; ob_act = TRUE; d->ansi.bold = FALSE; break;
    case 'r':  d->ansi.color = 3; ob_act = TRUE; d->ansi.bold = FALSE; break;
    case 'c':  d->ansi.color = 4; ob_act = TRUE; d->ansi.bold = FALSE; break;
    case 'p':  d->ansi.color = 5; ob_act = TRUE; d->ansi.bold = FALSE; break;
    case 'o':  d->ansi.color = 6; ob_act = TRUE; d->ansi.bold = FALSE; break;
    case 'w':  d->ansi.color = 7; ob_act = TRUE; d->ansi.bold = FALSE; break;

    case 'B':  d->ansi.bold  = TRUE;  ob_act = TRUE; break;
    case 'b':  d->ansi.bold  = FALSE; ob_act = reset = TRUE; break;
    case 'F':  d->ansi.flash = TRUE;  ob_act = TRUE; break;
    case 'f':  d->ansi.flash = FALSE; ob_act = reset = TRUE; break;
    case '_':  d->ansi.under = TRUE;  ob_act = TRUE; break;
    case ':':  d->ansi.under = FALSE; ob_act = reset = TRUE; break;

    case 'n':  d->ansi.bold  = d->ansi.flash = d->ansi.under = FALSE;
    d->ansi.color = 7; ob_act = TRUE; break;
    }
    */
    /*
    if ( ob_act && IS_ANSI( d->character ) )
    color_code = TRUE; */
  }
  if ( last_point )
  {
    d->showstr_point = last_point;
    d->showstr_page  = last_page;
    return TRUE;
  }

  return FALSE;
}


/* String pager. */
void show_string( DESCRIPTOR_DATA *d, char *input )
{
  char buffer[4*MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char *stop;
  char *scan;
  int  show_lines;
  int  lines    = 0;
  bool toggle   = TRUE;
  bool fEndDone = FALSE;
  bool fDone    = TRUE;

  if ( d->character == NULL )
  {
    if ( d->showstr_head )
    {
      free_mem( d->showstr_head );
      d->showstr_head = NULL;
    }
    d->showstr_point = NULL;
    write_to_buffer( d, "#n", 2 );
    return;
  }

  one_argument( input, arg );

  if ( arg[0] == '\0'
       ||   d->connected == CON_READ_IMOTD
       ||   d->connected == CON_READ_MOTD )
  {
    goto_page( d, d->showstr_page + 1 );
    fEndDone = TRUE;
  }
  else if ( !str_prefix( arg, "first" ) )
    fEndDone = TRUE;
  else if ( !str_prefix( arg, "back" ) )
    goto_page( d, d->showstr_page - 1 );
  else if ( !str_prefix( arg, "refresh" ) )
    goto_page( d, d->showstr_page );
  else if ( is_number( arg ) )
  {
    int value = atoi( arg );

    if ( value >= 1 && value <= d->showstr_page_count )
      goto_page( d, value );
    else
    {
      write_to_buffer( d, "Page number too high.\n\r", 0 );
      return;
    }
  }
  else
  {
    if ( d->showstr_head )
    {
      free_mem( d->showstr_head );
      d->showstr_head = NULL;
    }
    d->showstr_point = NULL;
    write_to_buffer( d, "#n", 2 );
    return;
  }

  show_lines = d->character->lines;

  /*  if ( IS_ANSI( d->character ) )
    write_to_buffer( d, color_value_str( &d->ansi, TRUE ), 0 );
  */
  for ( scan = buffer; ; scan++, d->showstr_point++ )
  {
    if ( ( *scan = *d->showstr_point ) == '#' )
      if ( ( *++scan = *++d->showstr_point ) )
        continue;

    if ( ( *scan == '\n' || *scan == '\r' ) && ( toggle = !toggle ) )
      lines++;

    else if ( !*scan || ( show_lines > 0 && lines >= show_lines ) )
    {
      if ( *scan )
        for ( stop = scan; ; scan++, d->showstr_point++ )
        {
          if ( ( *scan = *d->showstr_point ) == '#' )
          {
            if ( is_color_char( ( *++scan = *++d->showstr_point ) ) )
              continue;
            else if ( *scan == '\0' )
              break;
            else if ( is_replace_char( *scan ) )
            {
              fDone = FALSE;
              scan = stop;
              break;
            }
            continue;
          }
          if ( isspace( *scan ) )
            continue;
          if ( !*scan )
            break;
          fDone = FALSE;
          scan = stop;
          break;
        }

      *scan = '\0';
      write_to_buffer( d, buffer, 0 );

      if ( !fEndDone || !fDone )
        return;

      if ( d->showstr_head )
      {
        free_mem( d->showstr_head);
        d->showstr_head = NULL;
      }
      d->showstr_point = NULL;
      return;
    }
  }
}
#endif
/*
 * Page to one char, new colour version, by Lope.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
  const  char  *point;
  char  *point2;
  char  buf[ MAX_STRING_LENGTH * 80 ];
  int  skip = 0;

  if ( txt == NULL || ch->desc == NULL)
    return;

  buf[0] = '\0';
  point2 = buf;
  if ( IS_SET( ch->act, PLR_COLOUR ) )
  {
    for ( point = txt ; *point ; point++ )
    {
      if ( *point == '{' )
      {
        point++;
        skip = colour( *point, point2 );
        while ( skip-- > 0 )
          ++point2;
        continue;
      }
      *point2 = *point;
      *++point2 = '\0';
    }
    *point2 = '\0';
    ch->desc->showstr_head  = alloc_mem( strlen( buf ) + 1 );
    strcpy( ch->desc->showstr_head, buf );
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string( ch->desc, "\0" );
  }
  else
  {
    for ( point = txt ; *point ; point++ )
    {
      if ( *point == '{' )
      {
        point++;
        continue;
      }
      *point2 = *point;
      *++point2 = '\0';
    }
    *point2 = '\0';
#if USE_BUGFIX_1
    if (ch->desc->showstr_head &&
        (strlen(buf)+strlen(ch->desc->showstr_head)+1) < 32000)
    {
      char *temp=alloc_mem(strlen(txt) + strlen(ch->desc->showstr_head) + 1);
      strcpy(temp, ch->desc->showstr_head);
      strcat(temp, buf);
      ch->desc->showstr_point = temp +
                                (ch->desc->showstr_point - ch->desc->showstr_head);
      free_mem(ch->desc->showstr_head);
      ch->desc->showstr_head=temp;
    }
    else
    {
      if (ch->desc->showstr_head)
        free_mem(ch->desc->showstr_head );
      ch->desc->showstr_head = alloc_mem(strlen(buf) + 1);
      strcpy(ch->desc->showstr_head,buf);
      ch->desc->showstr_point = ch->desc->showstr_head;
      show_string(ch->desc,"");
    }
#else
    ch->desc->showstr_head  = alloc_mem( strlen( buf ) + 1 );
    strcpy( ch->desc->showstr_head, buf );
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string( ch->desc, "" );
#endif
  }
  return;
}

/* string pager */
void show_string( struct descriptor_data *d, char *input )
{
  char buffer[4*MAX_STRING_LENGTH], arg[MAX_INPUT_LENGTH],  *scan, *chk;
  int lines = 0, toggle = 1, show_lines = 0;
  int t = 0, t1 = 0, t2 = 0, t3 = 0;

  if ( d->character->lines > 100 )
    d->character->lines = 0;

  one_argument( input, arg );

  if ( arg[0] )
  {
    if ( d->showstr_head )
    {
      free_mem( d->showstr_head );
      d->showstr_head = NULL;
    }
    d->showstr_point  = 0;
    return;
  }

  if ( t || t1 || t2 || t3 )
    bugf( "ALERT: Memory CORRUPTION #1.\n\r" );
  if ( d->character )
    show_lines = d->character->lines;
  else
    show_lines = 0;

  if ( !show_lines && d->showstr_head )
  {
    write_to_buffer( d, d->showstr_head, strlen( d->showstr_head ) );
    free_mem( d->showstr_head );
    d->showstr_head = NULL;
    d->showstr_point = 0;
    return;
  }

  for ( scan = buffer; ; scan++, d->showstr_point++ )
  {
    if ( ( ( *scan = *d->showstr_point ) == '\n' || *scan == '\r' )
         && ( toggle = -toggle ) < 0 )
      lines++;

    else if ( !*scan || ( show_lines > 0 && lines >= show_lines ) )
    {
      *scan = '\0';
      write_to_buffer( d, buffer, strlen( buffer ) );
      for ( chk = d->showstr_point; isspace( *chk ); chk++ );
      {
        if ( !*chk )
        {
          if ( d->showstr_head )
          {
            free_mem( d->showstr_head );
            d->showstr_head = NULL;
          }
          d->showstr_point  = 0;
        }
      }
      return;
    }
  }
  return;
}


/* quick sex fixer */
void fix_sex( CHAR_DATA *ch )
{
  if ( ch->sex < 0 || ch->sex > 2 )
    ch->sex = IS_NPC( ch ) ? 0 : ch->pcdata->true_sex;
}

int format_act( char *msg, const char *format, CHAR_DATA *ch,
                CHAR_DATA *to, const void *arg1, const void *arg2 )
{
  static char * const he_she [] = { "it",  "he",  "she" };
  static char * const him_her[] = { "it",  "him", "her" };
  static char * const his_her[] = { "its", "his", "her" };
  CHAR_DATA *vch = (CHAR_DATA *)arg2;
  OBJ_DATA *obj1 = (OBJ_DATA  *)arg1;
  OBJ_DATA *obj2 = (OBJ_DATA  *)arg2;
  char     *pbuff;
  char     buffer[ MAX_STRING_LENGTH*2 ];
  char fname[MAX_INPUT_LENGTH];
  const char *str;
  const char *i;
  char *point;

  point  = msg;
  str    = format;
  while ( *str != '\0' )
  {
    if ( *str != '$' )
    {
      *point++ = *str++;
      continue;
    }
    ++str;

    if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
    {
      bugf("Format_act: null arg2, code $%c, format %s",  *str, format );
      i = "<@@@>";
    }
    else
    {
      switch ( *str )
      {
        default:
          bugf("Format_act: bad code $%c, format %s, %s",
               *str, format, interp_cmd );
          i = "<@@@>";
          break;
        case 't':
          i = (char *)arg1;
          break;
        case 'T':
          i = (char *)arg2;
          break;
        case 'n':
          i = PERS( ch,  to  );
          break;
        case 'N':
          i = PERS( vch, to  );
          break;
        case 'e':
          i = he_she [URANGE( 0,  ch->sex, 2 )];
          break;
        case 'E':
          i = he_she [URANGE( 0, vch->sex, 2 )];
          break;
        case 'm':
          i = him_her[URANGE( 0,  ch->sex, 2 )];
          break;
        case 'M':
          i = him_her[URANGE( 0, vch->sex, 2 )];
          break;
        case 's':
          i = his_her[URANGE( 0,  ch->sex, 2 )];
          break;
        case 'S':
          i = his_her[URANGE( 0, vch->sex, 2 )];
          break;
        case 'g':
          if ( ch->clan )
            i = ch->clan->clan_immortal;
          else
            i = "{CM{cir{Ml{mya{x";
          break;
        case 'G':
          if ( vch->clan )
            i = vch->clan->clan_immortal;
          else
            i = "{CM{cir{Ml{mya{x";
          break;
        case 'p':
          if ( obj1 == NULL )
          {
            bugf( "Format_act: null obj1, code $%c, format %s, %s",
                  *str, format, interp_cmd );
            i = "<nothing>";
          }
          else
          {
            i = can_see_obj( to, obj1 )
                ? obj1->short_descr
                : "something";
          }
          break;
        case 'P':
          if ( obj2 == NULL )
          {
            bugf("Format_act: null obj2, code $%c, format %s, %s",
                 *str, format, interp_cmd );
            i = "<nothing>";
          }
          else
          {
            i = can_see_obj( to, obj2 )
                ? obj2->short_descr
                : "something";
          }
          break;
        case 'd':
          if ( arg2 == NULL || ((char *)arg2)[0] == '\0' )
          {
            i = "door";
          }
          else
          {
            one_argument( (char *)arg2, fname );
            i = fname;
          }
          break;
      }
    }

    ++str;
    while ( ( *point = *i ) != '\0' )
      ++point, ++i;
  }

  *point++  = '\n';
  *point++  = '\r';
  *point  = '\0';

  msg[0]  = UPPER( msg[0] );
  pbuff  = buffer;
  colourconv( pbuff, msg, to );
  /*    colourconv(msg, msg, to);*/
  /*    str_capitalize_color( msg, FALSE );*/

  return point - msg;
}

/*
 * The colour version of the act_new( ) function, -Lope -OLD
 */

void act_new( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, int min_pos )
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
      bugf( "Act_new: null vch with TO_VICT at [%d]. ch:-%s-,format = -%s-, %s",
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

    if ( ( type == TO_CHAR )
         &&   ( to != ch ) )
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
    if ( ( type == TO_WIZ_ROOM
           || type == TO_WIZ_NOTVICT
           || type == TO_WIZ_VICT )
         && ( get_trust( to ) < ch->invis_level ) )
      continue;

    format_act( msg, format, ch, to, arg1, arg2 );

    if ( to->desc != NULL )
      send_to_char(msg, to );

    /*    write_to_buffer( to->desc, buf, point - buf );*/

    /* probably should make HAS_TRIGGER return false if not IS_NPC */
    if ( MOBtrigger && IS_NPC( to ) )
      mp_act_trigger( msg, to, ch, arg1, arg2, TRIG_ACT );
    /*      if ( MOBtrigger && IS_NPC( to ) && HAS_TRIGGER(to, TRIG_ACT) ) { mp_act_trigger( msg, to, ch, arg1, arg2, TRIG_ACT ); }*/

  }
  return;
}

void act_channels( const char *format, CHAR_DATA *ch, const void *arg1,
                   const void *arg2, int type, int min_pos, const int channel )
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
  *      CONSIDER FIXING THIS.  BY ADDING NEW ACT that CAN PASS
  *           EXCLUDING CERTAIN COMMS.  */
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
           &&   (!IS_SET(vch->chan_flags,CHANNEL_QUIET) && (channel != CHAN_PRAYER) )
           &&   !IS_SET(vch->chan_flags,CHANNEL_NOGOSSIP))
      {
        format_act( msg, format, ch, to, arg1, arg2 );
        vic = d->character;
        send_to_char(msg, vic );

        update_chan_hist(vic, channel, msg);
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

        update_chan_hist(vic, channel, msg);
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
      bugf( "Act_new: null vch with TO_VICT at [%d]. ch:-%s-,format = -%s-, %s",
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

    format_act( msg, format, ch, to, arg1, arg2 );

    if ( to->desc != NULL )
    {
      send_to_char(msg, to );
      update_chan_hist(to, channel, msg);
    }

    /*            write_to_buffer( to->desc, buf, point - buf );*/

    /* probably should make HAS_TRIGGER return false if not IS_NPC */
    if ( MOBtrigger && IS_NPC( to ) )
      mp_act_trigger( msg, to, ch, arg1, arg2, TRIG_ACT );
    /*      if ( MOBtrigger && IS_NPC( to ) && HAS_TRIGGER(to, TRIG_ACT) )
    *               mp_act_trigger( msg, to, ch, arg1, arg2, TRIG_ACT ); */

  }
  return;
}


/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
  tp->tv_sec  = time( NULL );
  tp->tv_usec = 0;
}
#endif


int colour( char type, char *string )
{
  char  code[ 20 ];
  char  *p = '\0';

  switch ( type )
  {
    default:
      mprintf( sizeof(code), code, CLEAR );
      break;
    case 'x':
      mprintf( sizeof(code), code, CLEAR );
      break;
    case 'b':
      mprintf( sizeof(code), code, C_BLUE );
      break;
    case 'c':
      mprintf( sizeof(code), code, C_CYAN );
      break;
    case 'g':
      mprintf( sizeof(code), code, C_GREEN );
      break;
    case 'm':
      mprintf( sizeof(code), code, C_MAGENTA );
      break;
    case 'r':
      mprintf( sizeof(code), code, C_RED );
      break;
    case 'w':
      mprintf( sizeof(code), code, C_WHITE );
      break;
    case 'y':
      mprintf( sizeof(code), code, C_YELLOW );
      break;
    case 'B':
      mprintf( sizeof(code), code, C_B_BLUE );
      break;
    case 'C':
      mprintf( sizeof(code), code, C_B_CYAN );
      break;
    case 'G':
      mprintf( sizeof(code), code, C_B_GREEN );
      break;
    case 'M':
      mprintf( sizeof(code), code, C_B_MAGENTA );
      break;
    case 'R':
      mprintf( sizeof(code), code, C_B_RED );
      break;
    case 'W':
      mprintf( sizeof(code), code, C_B_WHITE );
      break;
    case 'Y':
      mprintf( sizeof(code), code, C_B_YELLOW );
      break;
    case 'D':
      mprintf( sizeof(code), code, C_D_GREY );
      break;
      /*    case '*':
            mprintf( sizeof(code), code, "%c", 007 );
            break;
          case '/':
            mprintf( sizeof(code), code, "%c", 012 );
            break;*/
    case '{':
      mprintf( sizeof(code), code, "%c", '{' );
      break;
    case 'L':
    case 'l' :
      mprintf( sizeof(code), code, C_BLINK);
      break;
    case 'U':
    case 'u':
      mprintf( sizeof(code), code, C_UNDERLINE);
      break;
    case '*':
      switch ( number_range( 1, 15 ) )
      {
        case 1 :
          strcpy( code, C_BLUE );
          break;

        case 2 :
          strcpy( code, C_B_BLUE );
          break;

        case 3 :
          strcpy( code, C_RED );
          break;

        case 4 :
          strcpy( code, C_B_RED );
          break;

        case 5 :
          strcpy( code, C_YELLOW );
          break;

        case 6 :
          strcpy( code, C_B_YELLOW );
          break;

        case 7 :
          strcpy( code, C_GREEN );
          break;

        case 8 :
          strcpy( code, C_B_GREEN );
          break;

        case 9 :
          strcpy( code, C_MAGENTA );
          break;
        case 10:
          strcpy( code, C_B_MAGENTA );
          break;

        case 11:
          strcpy( code, C_CYAN );
          break;

        case 12:
          strcpy( code, C_B_CYAN );
          break;

        case 13:
          strcpy( code, C_WHITE );
          break;

        case 14:
          strcpy( code, C_B_WHITE );
          break;

        case 15:
          strcpy( code, C_D_GREY );
          break;

        default:
          strcpy( code, C_WHITE );
          break;
      }
      break;
    case '^':

      switch ( number_range( 1, 8 ) )
      {
        case 1 :
          strcpy( code, C_BLUE );
          break;

        case 2 :
          strcpy( code, C_RED );
          break;

        case 3 :
          strcpy( code, C_YELLOW );
          break;

        case 4 :
          strcpy( code, C_GREEN );
          break;

        case 5 :
          strcpy( code, C_MAGENTA );
          break;

        case 6 :
          strcpy( code, C_CYAN );
          break;

        case 7 :
          strcpy( code, C_WHITE );
          break;

        case 8 :
          strcpy( code, C_D_GREY );
          break;
      }

      break;

    case '&':

      switch ( number_range( 1, 7 ) )
      {
        case 1 :
          strcpy( code, C_B_BLUE );
          break;

        case 2 :
          strcpy( code, C_B_RED );
          break;

        case 3 :
          strcpy( code, C_B_YELLOW );
          break;

        case 4 :
          strcpy( code, C_B_GREEN );
          break;

        case 5 :
          strcpy( code, C_B_MAGENTA );
          break;

        case 6 :
          strcpy( code, C_B_CYAN );
          break;

        case 7 :
          strcpy( code, C_B_WHITE );
          break;

      }
      break;

  }

  p = code;
  while ( *p != '\0' )
  {
    *string = *p++;
    *++string = '\0';
  }

  return( strlen( code ) );
}

/*
 * Returns an string with the first non-color character set to upper case.  If
 * lower is TRUE, it will set the remaining string to lower case, excepting any
 * color markers.
 */
char *capitalize_color( const char *str, bool lower )
{
  static char strcap[MAX_STRING_LENGTH];
  int i = 0;
  bool nocap = TRUE;

  for ( ; str[i] != '\0'; i++ )
  {
    if ( str[i] == '{' && str[i + 1] != '\0' )
    {
      strcap[i] = str[i];
      i++;
      strcap[i]   = str[i];
    }
    else if ( nocap )
    {
      if ( str[i] == ' ' )
      {
        strcap[i] = str[i];
        continue;
      }
      strcap[i] = UPPER( str[i] );
      nocap = FALSE;
    }
    else
      strcap[i] = lower ? LOWER( str[i] ) : str[i];
  }
  strcap[i] = '\0';
  return strcap;
}


void colourconv( char *buffer, const char *txt, CHAR_DATA *ch )
{
  const  char  *point;
  int  skip = 0;

  if ( ch->desc && txt )
  {
    if ( IS_SET( ch->act, PLR_COLOUR ) )
    {
      for ( point = txt ; *point ; point++ )
      {
        if ( *point == '{' )
        {
          point++;
          if (!IS_NPC(ch))
            skip = colour( *point, buffer );
          else skip =0;
          while ( skip-- > 0 )
            ++buffer;
          continue;
        }
        *buffer = *point;
        *++buffer = '\0';
      }
      *buffer = '\0';
    }
    else
    {
      for ( point = txt ; *point ; point++ )
      {
        if ( *point == '{' )
        {
          point++;
          continue;
        }
        *buffer = *point;
        *++buffer = '\0';
      }
      *buffer = '\0';
    }
  }
  return;
}
/* source: EOD, by John Booth <???> */
void printf_to_char (CHAR_DATA *ch, char *fmt, ...)
{
  char buf [MAX_STRING_LENGTH];
  va_list args;
  va_start (args, fmt);
  vsprintf (buf, fmt, args);
  va_end (args);

  send_to_char ( buf, ch );
}


void bugf (char * fmt, ...)
{
  char buf [MAX_STRING_LENGTH];
  va_list args;
  va_start (args, fmt);
  vsnprintf (buf, MSL, fmt, args);
  va_end (args);
  print_cmd_hist();
  bug (buf, 0);
}

void logit (char * fmt, ...)
{
  char buf [2*MSL];
  va_list args;
  va_start (args, fmt);
  vsnprintf (buf, 2*MSL, fmt, args);
  va_end (args);

  log_string (buf);
}

/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
char *strf( const char *fmt, ...)
{
  char *str_new;
  char buf [MAX_STRING_LENGTH];
  va_list args;
  va_start (args, fmt);
  vsprintf (buf, fmt, args);
  va_end (args);

  if ( buf[0] == '\0' )
    return &str_empty[0];

  //  if ( str >= string_space && str < top_string )
  //    return (char *) str;

  str_new = alloc_mem( strlen(buf) + 1 );
  strcpy( str_new, buf );
  return str_new;
}

void display_race_table(CHAR_DATA *ch)
{
  send_to_char("\n\r{W[{CSelecting a race{W]:{x\n\r",ch);
  do_function(ch,&do_help,"race help");
  send_to_char("\n\r",ch);
}

void display_class_table(CHAR_DATA *ch)
{
  send_to_char("\n\r{W[{CSelecting a class{W]:{x\n\r",ch);
  do_function(ch,&do_help,"class help");
  send_to_char("{WWhat is your class?{x ",ch);
}
void display_ansi_greeting(char *filename, DESCRIPTOR_DATA *d)
{
  FILE *fp;
  char temp_buffer[40000];

  fp =fopen(filename,"rb");
  nFilesOpen++;
//  int fpret = fread(temp_buffer, sizeof(char), INT_MAX, fp);
  fread(temp_buffer, sizeof(char), 40000, fp);
  write_to_buffer(d, temp_buffer, sizeof(temp_buffer));
  fclose(fp);
  nFilesOpen--;
}

int display_weapon_choices(CHAR_DATA *ch)
{
  int i,k=0;
  char buf[MSL];
  int num=0;

  send_to_char("\n\rPlease pick a weapon from the following choices:\n\r",ch);
  strcpy(buf,"{W[{x");
  for ( i = 0; weapon_table[i].name != NULL; i++,k++)
    if ( (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
         &&   (skill_table[skill_lookup(weapon_table[i].skillname)].skill_level[ch->gameclass] == 1) )
    {

      strcat(buf,weapon_table[i].name);
      num++;
      strcat(buf," ");
    }
  strcat(buf,"{W]{x\n\rYour choice? ");
  /* this isn't right... ?
    if (num < 2 )
      return(num);
    else
      send_to_char(buf,ch);
  */
  send_to_char(buf,ch);
  return(num);
}

void do_cleansockets( CHAR_DATA *ch, char *argument )
{


  DESCRIPTOR_DATA *d;
  bool found = FALSE;
  send_to_char("Disabled: use disconnect manually instead.\n\r",ch);
  return;

  if (argument[0] == '\0')
  {
    send_to_char("Syntax: clrsockets [All/Name]\n\r",ch);
    return;
  }

  if (!str_cmp(argument,"All"))
  {
    for ( d = descriptor_list; d; d = d->next )
    {
      if ( d->character && can_see( ch, d->character ) )
      {
        /* NB: You may need to edit the CON_ values */
        switch ( d->connected )
        {
          case CON_PLAYING:
            continue;
            break;
          case CON_GET_NAME:
          case CON_GET_OLD_PASSWORD:
          case CON_CONFIRM_NEW_NAME:
          case CON_GET_NEW_PASSWORD:
          case CON_CONFIRM_NEW_PASSWORD:
          case CON_GET_NEW_RACE:
          case CON_GET_NEW_SEX:
          case CON_GET_NEW_CLASS:
          case CON_GET_ALIGNMENT:
          case CON_DEFAULT_CHOICE:
          case CON_GEN_GROUPS:
          case CON_PICK_WEAPON:
          case CON_BREAK_CONNECT:
          case CON_READ_IMOTD:
          case CON_READ_MOTD:
          case CON_ANSI:
          default:
            send_to_desc("You have been booted due to inactivity.\n\r",d);
            close_socket (d);
            break;
        }
      }
    }
  }
  else
  {
    for ( d = descriptor_list; d; d = d->next )
    {
      if ( d->character && can_see( ch, d->character ) )
      {
        if (is_name(d->character->name,argument))
        {
          /* NB: You may need to edit the CON_ values */
          switch ( d->connected )
          {
            case CON_PLAYING:
              continue;
              break;
            case CON_GET_NAME:
            case CON_GET_OLD_PASSWORD:
            case CON_CONFIRM_NEW_NAME:
            case CON_GET_NEW_PASSWORD:
            case CON_CONFIRM_NEW_PASSWORD:
            case CON_GET_NEW_RACE:
            case CON_GET_NEW_SEX:
            case CON_GET_NEW_CLASS:
            case CON_GET_ALIGNMENT:
            case CON_DEFAULT_CHOICE:
            case CON_GEN_GROUPS:
            case CON_PICK_WEAPON:
            case CON_BREAK_CONNECT:
            case CON_READ_IMOTD:
            case CON_READ_MOTD:
            case CON_ANSI:
            default:
              send_to_desc("You have been booted due to inactivity.\n\r",d);
              found = TRUE;
              close_socket (d);
              break;
          }
        }
        if (!found)
        {
          send_to_char("That char is either not able to be deleted or not online.\n\r",ch);
          send_to_char("Why not use Disconnect # instead?\n\r",ch);
        }
      }
    }
  }
  return;
}



void strip_color(char *buffer, const char *txt)
{
  const  char  *point;
  for ( point = txt ; *point ; point++ )
  {
    if ( *point == '{' )
    {
      point++;
      continue;
    }
    *buffer = *point;
    *++buffer = '\0';
  }
  *buffer = '\0';
}
void do_copyove (CHAR_DATA *ch, char * argument)
{
  send_to_char("If you want to do a COPYOVER spell it out!\n\r",ch);
}


void do_cleanup(CHAR_DATA *ch, char *argument)
{
  cleanup_restrings(ch);
  do_function(ch, &do_echo, "{wA {cm{wi{cst{x sweeps across the land, freezing time as the gods prepare to alter reality.{x");
  do_function(ch, &do_echo, "{wIf you are on a quest or otherwise occupied, only a prayer can save you.{x");
  do_function(ch, &do_owhere, "pcorpse");
  send_to_char("Ready to copyover.\n\r",ch);
}
/*  Copyover - Original idea: Fusion of MUD++
 *  Adapted to Diku by Erwin S. Andreasen, <erwin@pip.dknet.dk>
 *  http://pip.dknet.dk/~pip1773
 *  Changed into a ROM patch after seeing the 100th request for it :)
 */
void do_copyover (CHAR_DATA *ch, char * argument)
{
  FILE *fp, *gfp, *tfp;
  DESCRIPTOR_DATA *d, *d_next;
  char gfilestr[MAX_INPUT], cfilestr[MAX_INPUT], tfilestr[MAX_INPUT];
  char buf [MSL], buf2[100];
  CHAR_DATA *vch, *vch_next;

  mprintf( sizeof(cfilestr), cfilestr, COPYOVER_FILE, port);

  if (argument[0] != '\0')
  {
    if (strcmp(argument,"bypass"))
    {
      send_to_char("Copying anyways...have no clue what command you wanted.\n\r",ch);
      do_function(ch, &do_asave, "changed");
      do_function(ch, &do_asave, "helps");
    }
  }
  else
  {
    do_function(ch, &do_asave, "changed");
    do_function(ch, &do_asave, "helps");
  }
  fp = fopen (cfilestr, "w");
  nFilesOpen++;

  if (!fp)
  {
    send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
    logit ("Could not write to copyover file: %s", COPYOVER_FILE);
    perror ("do_copyover:fopen");
    return;
  }
  ispell_done();
  /* Consider changing all saved areas here, if you use OLC */

  /* do_asave (NULL, ""); - autosave changed areas */

  mprintf (sizeof(buf),buf, "\n\r%s utters the words, 'Morlabres Des Augrocien.'\n\rExistence bows to his power and stands still.\n\rThe world around you seems to change.\n\rYour eyes close and you fade into the dream world.\n\n\rDraknor swirls as it changes within your dreams!\n\r", ch->name );

//  shutdown_web(); //websvr.c

  /* For each playing descriptor, save its state */
  for (d = descriptor_list; d ; d = d_next)
  {
    CHAR_DATA * och = CH (d);
    d_next = d->next;  /* We delete from the list , so need to save this */

    if (!d->character || d->connected > CON_PLAYING) /* drop those logging on */
    {
      write_to_descriptor_2 (d->descriptor, "\n\rSorry, we are upgrading. Come back in a few minutes.\n\r", 0);
      close_socket (d);  /* throw'em out */
    }
    else
    {
      fprintf (fp, "%d %s %s\n", d->descriptor, och->name,
               d->host);

#if 0        /* This is not necessary for ROM */
      if (och->level == 1)
      {
        write_to_descriptor_2 (d->descriptor, "Since you are level one, and level one characters do not save, you gain a free level!\n\r", 0);
        advance_level (och,TRUE);
        och->level++;  /* Advance_level doesn't do that */
      }
#endif
      save_char_obj (och, TRUE);
#if !defined(SPARC)
      if (d->out_compress)
        compressEnd(d);
#endif
      write_to_descriptor_2 (d->descriptor, buf, 0);
    }
  }

  fprintf (fp, "-1\n");
  fclose (fp);
  nFilesOpen--;


  /*
   * Now save the group file.
   */
  mprintf( sizeof(gfilestr), gfilestr, COPYOVER_GROUP_FILE, port );

  if ( ( gfp = fopen( gfilestr, "w" ) ) == NULL )
  {
    printf_to_char( ch, "Can't open group file (%s), continuing.\n\r",
                    gfilestr );
    logit( "Do_copyover: can't open group file (%s), continuing",
           gfilestr );
    perror( "Do_copyover: fopen" );
  }
  else
  {
    nFilesOpen++;
    /*
     * vch->master is who vch is following.
     * vch->leader is who is leader of vch's group.
     */
    for ( vch = player_list; vch; vch = vch_next )
    {
      vch_next = vch->next_player;

      if (vch == NULL)
        continue;

      /* Don't save if leader or master is NPC. */
      if ( ( vch->leader == NULL && vch->master == NULL )
           ||   ( vch->leader && IS_NPC( vch->leader ) )
           ||   ( vch->master && IS_NPC( vch->master ) ) )
        continue;

      if ( vch->leader && vch->master )
        fprintf( gfp, "%d %s ML %s %s %ld\n",
                 vch->level, vch->name,
                 vch->master->name, vch->leader->name, vch->group_num );
      else if ( vch->leader )
        fprintf( gfp, "%d %s L --- %s %ld\n",
                 vch->level, vch->name, vch->leader->name, vch->group_num );
      else
        fprintf( gfp, "%d %s M %s --- %ld\n",
                 vch->level, vch->name, vch->master->name, vch->group_num );
    }
    fprintf( gfp, "-1\n" );
    fclose( gfp );
    nFilesOpen--;
  }

  /*
   * Now save the group file.
   */
  mprintf( sizeof(tfilestr), tfilestr, COPYOVER_TIME_FILE, port );

  if ( ( tfp = fopen( tfilestr, "w" ) ) == NULL )
  {
    printf_to_char( ch, "Can't open TIME file (%s), continuing.\n\r",  tfilestr );
    logit( "Do_copyover: can't open TIME file (%s), continuing", tfilestr );
    perror( "Do_copyover: fopen: Time_File" );
  }
  else
  {
    nFilesOpen++;
    fprintf(tfp,"%ld\n\r", (long) tml_boot_time);
    fclose( tfp );
    nFilesOpen--;
  }



  /* Close reserve and other always-open files and release other resources */
  fclose (fpReserve);
  nFilesOpen--;
  /* exec - descriptors are inherited */
  logit("COPYOVER INITIATED");
  mprintf (sizeof(buf), buf, "%d", port);
  mprintf (sizeof(buf2), buf2, "%d", control);
  execl (EXE_FILE, "sacred", buf, "copyover", buf2, (char *) NULL);
  /* Failed - sucessful exec will not return */

  perror ("do_copyover: execl");
  printf("COPYOVER FAILED\n\r");
  send_to_char ("Copyover FAILED!\n\r",ch);

  /* Here you might want to reopen fpReserve */
  fpReserve = fopen (NULL_FILE, "r");
  nFilesOpen++;
}

/* Recover from a copyover - load players. */
void copyover_recover( void )
{
  DESCRIPTOR_DATA  *d, *descriptor_first, *descriptor_last;
  CHAR_DATA    *char_first, *char_last, *player_first, *player_last;
  FILE    *cfp, *gfp, *tfp;
  char    cfilestr[512], gfilestr[512], tfilestr[512];
  char    name[64];
  char    host[MSL];
  int     desc;
  bool    fOld;

  logit( "Copyover recovery initiated" );

  fCopyover = FALSE;
  fCopyoverRecover = TRUE;

  /* There are some descriptors open which will hang forever then? */
  mprintf( sizeof(cfilestr), cfilestr, COPYOVER_FILE, port);

  if ( ( cfp = fopen( cfilestr, "r" ) ) == NULL )
  {
    bugf( "Copyover_recover: can't open copyover file (%s), exiting",
          cfilestr );
    perror( "Copyover_recover: fopen" );
    exit( EXIT_FAILURE );
  }
  nFilesOpen++;
  /* In case something crashes - doesn't prevent reading. */
  unlink( cfilestr );

  /* Open the group file, but it's not critical, so don't exit on failure. */
  mprintf( sizeof(gfilestr), gfilestr, COPYOVER_GROUP_FILE, port );

  if ( ( gfp = fopen( gfilestr, "r" ) ) == NULL )
  {
    bugf( "Copyover_recover: can't open group file (%s), continuing",
          gfilestr );
    perror( "Copyover_recover: fopen" );
  }
  nFilesOpen++;
  /* In case something crashes - doesn't prevent reading. */
  unlink( gfilestr );

  /* Open the group file, but it's not critical, so don't exit on failure. */
  mprintf( sizeof(tfilestr), tfilestr, COPYOVER_TIME_FILE, port );

  if ( ( tfp = fopen( tfilestr, "r" ) ) == NULL )
  {
    bugf( "Copyover_recover: can't open TIME file (%s), continuing",
          tfilestr );
    perror( "Copyover_recover: TIME" );
  }
  nFilesOpen++;
  /* In case something crashes - doesn't prevent reading. */
  unlink( tfilestr );

  /*
   * We do it this way so the players are inserted into the lists the way
   * they were originally and not reversed.  Basically, we start our own
   * list using the xxx_first and xxx_last, and when done with the loading
   * we attach the xxx_last entry to the beginning of xxx_list and then set
   * xxx_list to xxx_first.  This is done for descriptor, char, and player.
   *
   * What that does is, all the loaded entities are attached, in order, at
   * the beginning of whatever else is already in the various lists.
   */
  descriptor_first  = NULL;
  descriptor_last  = NULL;
  char_first    = NULL;
  char_last    = NULL;
  player_first  = NULL;
  player_last    = NULL;

  for ( ; ; )
  {
    fscanf( cfp, "%d %s %s\n",  &desc, name, host );

    if ( desc == -1 )
      break;

    /* Write something, and check if it goes error-free. */
    if ( !write_to_descriptor_2( desc, "\n\rThe word 'Decroidius' pervades your thoughts as you slowly awaken.\n\r", 0 ) )
    {
      close( desc ); /* nope */
      continue;
    }

    d = new_descriptor();

    d->descriptor  = desc;
    d->connected  = CON_COPYOVER_RECOVER; /* -15, so close_socket frees the char */

    /*  d->addr    = addr;*/
    free_string( d->host );
    d->host    = str_dup( host , d->host);
    /*  free_string( d->ident );
    d->ident  = str_dup( ident );*/

    /*      if ( mccp && !compressStart( d ) )
    {
    bugf( "Copyover_recover: unable to enable compression, socket %d",
    d->descriptor );
    }
    */
    /* Insert in descriptor list, as they were prior to copyover. */
    if ( descriptor_first == NULL )
      descriptor_first = d;
    if ( descriptor_last )
      descriptor_last->next = d;
    descriptor_last = d;
    d->next = NULL;

    /* Now, find the pfile. */
    fOld = load_char_obj( d, name);

    if ( !fOld ) /* Player file not found?! */
    {
      bugf( "Copyover_recover: lost a pfile (%s)", name );
      write_to_descriptor( d,
                           "\n\r"
                           "Somehow your character was lost in the copyover!\n\r",  0 );
      close_socket( d );
    }
    else /* ok! */
    {
      CHAR_DATA *ch = d->character;
      CHAR_DATA *pet;

      /*      ch->pcdata->autosave  = d->descriptor % 30;
          ch->pcdata->logoff_time  = 0; */

      /*      ch->pcdata->addr  = addr;
          free_string( ch->pcdata->ident );
          ch->pcdata->ident  = str_dup( ident );
          free_string( ch->pcdata->host );
          ch->pcdata->host  = str_dup( host );*/
#if !defined(SPARC)
      /* mccp: tell the client we support compression */
    write_to_descriptor( d, (char *)compress_will, 0 );
    write_to_descriptor( d, (char *)compress2_will, 0 );
#endif

      write_to_descriptor( d,
                           "\n\r"
                           "You seem to have come out unharmed.\n\r", 0 );

#if !defined(SPARC)
      if ( d->out_compress )
        write_to_descriptor( d, "Compression enabled.\n\r", 0 );
#endif
      /* Just In Case */
      if ( ch->in_room == NULL )
        ch->in_room = get_room_index( ROOM_VNUM_TEMPLE );

      /* Insert in the char_list, as they were prior to copyover. */
      if ( char_first == NULL )
        char_first = ch;
      if ( char_last )
        char_last->next = ch;
      char_last = ch;
      ch->next = NULL;

      /* Insert in the player_list, as they were prior to copyover. */
      if ( player_first == NULL )
        player_first = ch;
      if ( player_last )
        player_last->next_player = ch;
      player_last  = ch;
      ch->next_player = NULL;

      d->connected = CON_PLAYING;

      char_to_room( ch, ch->in_room );
      do_function( ch, &do_look, "auto" );
      act( "$n materializes!", ch, NULL, NULL, TO_ROOM );

      if ( ( pet = ch->pet ) != NULL )
      {
        if ( pet->in_room == NULL )
          pet->in_room = ch->in_room;
        char_to_room( pet, pet->in_room );
        act( "$n materializes!", pet, NULL, NULL, TO_ROOM );
      }
    }
  }

  /*
   * Attach the lists we loaded to the beginning of the existing lists.
   */
  if ( descriptor_last )
  {
    descriptor_last->next = descriptor_list;
    descriptor_list = descriptor_first;
  }

  if ( char_last )
  {
    char_last->next = char_list;
    char_list = char_first;
  }

  if ( player_last )
  {
    player_last->next_player = player_list;
    player_list = player_first;
  }

  fclose( cfp );
  nFilesOpen--;

  if ( gfp )
  {
    int    level;
    char    char_name[64], ml_type[64];
    char    master_name[64], leader_name[64];
    long    group_num;
    CHAR_DATA  *ch, *mch, *lch;

    for ( ; ; )
    {
      fscanf( gfp,  "%d %s %s %s %s %ld\n", &level, char_name, ml_type, master_name, leader_name, &group_num );

      if ( level == -1 )
        break;

      for ( ch = player_list; ch; ch = ch->next_player )
        if ( ch->level == level && !str_cmp( ch->name, char_name ) )
          break;

      if ( ch == NULL )
      {
        bugf( "Copyover_recover: no group char (%s)", char_name );
        continue;
      }

      if ( str_chr( ml_type, 'L' ) )
      {
        for ( lch = player_list; lch; lch = lch->next_player )
          if ( !str_cmp( lch->name, leader_name ) )
            break;
      }
      else
        lch = NULL;

      if ( str_chr( ml_type, 'M' ) )
      {
        for ( mch = player_list; mch; mch = mch->next_player )
          if ( !str_cmp( mch->name, master_name ) )
            break;
      }
      else
        mch = NULL;

      ch->master = mch;
      ch->leader = lch;
      ch->group_num = group_num;
      if ( ch->pet && ch->leader )
        ch->pet->leader = lch;
    }

    fclose( gfp );
    nFilesOpen--;
  }

  if (tfp)
  {
    tml_boot_time = fread_number(tfp);
    fclose (tfp);
    nFilesOpen--;
  }
  else
    tml_boot_time = boot_time;

  /* Count those players! */
  count_update();

  fCopyoverRecover = FALSE;
}

int get_start_room(bool isevil, bool isgood, bool isnewbie)
{
  if (isevil)
  {
    if (isnewbie)
      return ROOM_VNUM_EVIL_SCHOOL;
    else
      return ROOM_VNUM_EVIL_START;
  }
  else if (isgood)
  {
    if (isnewbie)
      return ROOM_VNUM_SCHOOL;
    else
      return ROOM_VNUM_GOOD_START;
  }
  else
  {
    if (isnewbie)
      return ROOM_VNUM_NEUT_SCHOOL;
    else
      return ROOM_VNUM_NEUT_START;
  }
}

int get_recall_room( CHAR_DATA *ch )
{
  int room = 0;

  if ( ch->clan
       &&  !IS_SET( ch->clan->clan_flags, CLAN_INDEPENDENT ) )
  {
    //If it's not a player area just send them to Kaishaan recall
    if ( continent_table[ch->in_room->area->continent].playerarea == FALSE )
      room = ch->clan->recall[0];
    else
      room = ch->clan->recall[ch->in_room->area->continent];

    if (room > 0)
      return room; // if clan room is set, send it.  Otherwise continue as if no clan
  }

  if ( IS_PET( ch ) )
  {
    if ( ch->master )
    {
      if ( IS_EVIL( ch->master ) )
        switch ( ch->in_room->area->continent )
        {
          case 0 :
            room = ROOM_VNUM_EVIL_ALTAR;
            break;
          case 1 :
            room = ROOM_VNUM_CORUS_RECALL;
            break;
          default :
            room = ROOM_VNUM_EVIL_ALTAR;
            break;
            break;
        }
      else if ( IS_GOOD( ch->master ) )
        switch ( ch->in_room->area->continent )
        {
          case 0 :
            room = ROOM_VNUM_ALTAR;
            break;
          case 1 :
            room = ROOM_VNUM_CORUS_RECALL;
            break;
          default :
            room = ROOM_VNUM_ALTAR;
            break;
            break;
        }
      else
        switch ( ch->in_room->area->continent )
        {
          case 0 :
            room = ROOM_VNUM_NEUT_ALTAR;
            break;
          case 1 :
            room = ROOM_VNUM_CORUS_RECALL;
            break;
          default :
            room = ROOM_VNUM_NEUT_ALTAR;
            break;
            break;
        }
    }
  }
  else
    if ( IS_EVIL( ch ) )
      switch ( ch->in_room->area->continent )
      {
        case 0 :
          room = ROOM_VNUM_EVIL_ALTAR;
          break;
        case 1 :
          room = ROOM_VNUM_CORUS_RECALL;
          break;
        default :
          room = ROOM_VNUM_EVIL_ALTAR;
          break;
          break;
      }
    else if ( IS_GOOD( ch ) )
      switch ( ch->in_room->area->continent )
      {
        case 0 :
          room = ROOM_VNUM_ALTAR;
          break;
        case 1 :
          room = ROOM_VNUM_CORUS_RECALL;
          break;
        default :
          room = ROOM_VNUM_ALTAR;
          break;
          break;
      }
    else
      switch ( ch->in_room->area->continent )
      {
        case 0 :
          room = ROOM_VNUM_NEUT_ALTAR;
          break;
        case 1 :
          room = ROOM_VNUM_CORUS_RECALL;
          break;
        default :
          room = ROOM_VNUM_NEUT_ALTAR;
          break;
          break;
      }

  return room;
}

int get_death_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom )
{
  int room = 0;

  if ( IS_IN_CLAN( ch )
       &&   !IS_SET( ch->clan->clan_flags, CLAN_INDEPENDENT ) )
  {
    if ( continent_table[pRoom->area->continent].playerarea == FALSE )
      room = ch->clan->recall[0];
    else
      room = ch->clan->recall[pRoom->area->continent];

    if (room > 0)
      return room; // if clan room is set, send it.  Otherwise continue as if no clan
  }

  if ( IS_PET( ch ) )
    room = ROOM_VNUM_ALTAR; //Pets die too anyway...
  else
  {
    if ( IS_EVIL( ch ) )
      switch ( pRoom->area->continent )
      {
        case 0 :
          room = ROOM_VNUM_EVIL_ALTAR;
          break;
        case 1 :
          room = ROOM_VNUM_CORUS_DEATH;
          break;
        default :
          room = ROOM_VNUM_EVIL_ALTAR;
          break;
          break;
      }
    else if ( IS_GOOD( ch ) )
      switch ( pRoom->area->continent )
      {
        case 0 :
          room = ROOM_VNUM_ALTAR;
          break;
        case 1 :
          room = ROOM_VNUM_CORUS_DEATH;
          break;
        default :
          room = ROOM_VNUM_ALTAR;
          break;
          break;
      }
    else
      switch ( pRoom->area->continent )
      {
        case 0 :
          room = ROOM_VNUM_NEUT_ALTAR;
          break;
        case 1 :
          room = ROOM_VNUM_CORUS_DEATH;
          break;
        default :
          room = ROOM_VNUM_NEUT_ALTAR;
          break;
          break;
      }
  }

  return room;
}

void init_signals()
{
  signal(SIGTERM,sig_handler);
  signal(SIGBUS,sig_handler);
  signal(SIGABRT,sig_handler);
  signal(SIGSEGV,sig_handler);
}

/* existing function, minor change (one case) */
void sig_handler(int signal)
{
  switch (signal)
  {
    case SIGTERM:
      sig_shutdown();
      break;
    case SIGBUS:
    case SIGABRT:
    case SIGSEGV:
      logit("BUG CRASH:Sig handler SIGSEGV");
      crash_fix();
      break;
  }
}

/* Proper signal handler -- by Taeloch */
void sig_shutdown()
{
  DESCRIPTOR_DATA *d,*d_next;
  CHAR_DATA *vch;

  wiznet("The MUD is being shut down by the system!",NULL,NULL,WIZ_MEMCHECK,0,0);
  log_string( "The MUD is being shut down by the system!" );

  merc_down = TRUE;

  for ( d = descriptor_list; d; d = d_next)
  {
    d_next = d->next;
    send_to_desc("The system is shutting down the game NOW!!!\n\r", d );
    vch = d->original ? d->original : d->character;
    save_char_obj( vch, FALSE );
    close_socket( d );
  }

  return;
}
