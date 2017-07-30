

#if defined(macintosh) || defined(MSDOS)
const   char    echo_off_str    [] = { '\0' };
const   char    echo_on_str [] = { '\0' };
const   char    go_ahead_str    [] = { '\0' };
const   char    compress_will   [] = { '\0' };
const   char    compress_do     [] = { '\0' };
const   char    compress_dont   [] = { '\0' };
const   char    compress_start  [] = { '\0' };
#ifdef ANSIAUTODETECTION
const   char    ansi_will   [] = { '\0' };
#endif
#endif


#if defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "telnet.h"
const   char    echo_off_str    [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   char    echo_on_str [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const   char    echo_do     [] = { IAC, DO, TELOPT_ECHO, '\0' };
const   char    echo_dont   [] = { IAC, DONT, TELOPT_ECHO, '\0' };
const   char    go_ahead_str    [] = { IAC, GA, '\0' };
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
const   char    ansi_will   [] = { CTRL('['), '[', '6', 'n', '\0' };
#endif
#endif
#endif


bool legendary;
/*
 * OS-dependent declarations.
 */
#if defined(_AIX)
#include <sys/select.h>
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
void    bzero           args( ( char *b, int length ) );
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int     listen          args( ( int s, int backlog ) );
int     setsockopt      args( ( int s, int level, int optname, void *optval,
                                int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
#endif

#if defined(apollo)
#include <unistd.h>
void    bzero       args( ( char *b, int length ) );
#endif

#if defined(__hpux)
int     accept          args( ( int s, void *addr, int *addrlen ) );
int     bind            args( ( int s, const void *addr, int addrlen ) );
void    bzero           args( ( char *b, int length ) );
int     getpeername     args( ( int s, void *addr, int *addrlen ) );
int     getsockname     args( ( int s, void *name, int *addrlen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int     listen          args( ( int s, int backlog ) );
int     setsockopt      args( ( int s, int level, int optname,
                                const void *optval, int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
#endif

#if defined(apollo)
#include <unistd.h>
void    bzero       args( ( char *b, int length ) );
#endif

#if defined(__hpux)
int     accept          args( ( int s, void *addr, int *addrlen ) );
int     bind            args( ( int s, const void *addr, int addrlen ) );
void    bzero           args( ( char *b, int length ) );
int     getpeername     args( ( int s, void *addr, int *addrlen ) );
int     getsockname     args( ( int s, void *name, int *addrlen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int     listen          args( ( int s, int backlog ) );
int     setsockopt      args( ( int s, int level, int optname,
                                const void *optval, int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
#endif

#if defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if defined(linux)
/*
    Linux shouldn't need these. If you have a problem compiling, try
    uncommenting accept and bind.
int accept      args( ( int s, struct sockaddr *addr, int *addrlen ) );
int bind        args( ( int s, struct sockaddr *name, int namelen ) );
*/

int close       args( ( int fd ) );
/*
int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
*/
int gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
/*
int listen      args( ( int s, int backlog ) );
*/
/*int   read        args( ( int fd, char *buf, int nbyte ) );*/

int select      args( ( int width, fd_set *readfds, fd_set *writefds,
                fd_set *exceptfds, struct timeval *timeout ) );
int socket      args( ( int domain, int type, int protocol ) );
/*int   write       args( ( int fd, char *buf, int nbyte ) );*/
#endif

#if defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct  timeval
{
    time_t  tv_sec;
    time_t  tv_usec;
};
#if !defined(isascii)
#define isascii(c)      ( (c) < 0200 )
#endif
static  long            theKeys [4];

int gettimeofday        args( ( struct timeval *tp, void *tzp ) );
#endif

#if defined(MIPS_OS)
extern  int     errno;
#endif

#if defined(MSDOS)
int gettimeofday    args( ( struct timeval *tp, void *tzp ) );
int kbhit       args( ( void ) );
#endif

#if defined(NeXT)
int close       args( ( int fd ) );
int fcntl       args( ( int fd, int cmd, int arg ) );
#if !defined(htons)
u_short htons       args( ( u_short hostshort ) );
#endif
#if !defined(ntohl)
u_long  ntohl       args( ( u_long hostlong ) );
#endif
int read        args( ( int fd, char *buf, int nbyte ) );
int select      args( ( int width, fd_set *readfds, fd_set *writefds,
                fd_set *exceptfds, struct timeval *timeout ) );
int write       args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(sequent)
int accept      args( ( int s, struct sockaddr *addr, int *addrlen ) );
int bind        args( ( int s, struct sockaddr *name, int namelen ) );
int close       args( ( int fd ) );
int fcntl       args( ( int fd, int cmd, int arg ) );
int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
int gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
#if !defined(htons)
u_short htons       args( ( u_short hostshort ) );
#endif
int listen      args( ( int s, int backlog ) );
#if !defined(ntohl)
u_long  ntohl       args( ( u_long hostlong ) );
#endif
int read        args( ( int fd, char *buf, int nbyte ) );
int select      args( ( int width, fd_set *readfds, fd_set *writefds,
                fd_set *exceptfds, struct timeval *timeout ) );
int setsockopt  args( ( int s, int level, int optname, caddr_t optval,
                int optlen ) );
int socket      args( ( int domain, int type, int protocol ) );
int write       args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int accept      args( ( int s, struct sockaddr *addr, int *addrlen ) );
#if !defined(SPARC)
int bind        args( ( int s, struct sockaddr *name, int
                   namelen ) );
#endif
void    bzero       args( ( char *b, int length ) );
int close       args( ( int fd ) );
int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
#if !defined(WORK_SRC)
int gettimeofday    args( ( struct timeval *tp, struct timezone
                   *tzp ) );
#endif
int listen      args( ( int s, int backlog ) );
#if !defined(WORK_SRC)
int read        args( ( int fd, char *buf, int nbyte ) );
#endif
int select      args( ( int width, fd_set *readfds, fd_set *writefds,
                fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
int setsockopt      args( ( int s, int level, int optname,
                const char *optval, int optlen ) );
#else
#if !defined(WORK_SRC)
int setsockopt  args( ( int s, int level, int optname, void *optval,
                int optlen ) );
#endif
#endif
int socket      args( ( int domain, int type, int protocol ) );
#if !defined(WORK_SRC)
int write       args( ( int fd, char *buf, int nbyte ) );
#endif
#endif

#if defined(ultrix)
int accept      args( ( int s, struct sockaddr *addr, int *addrlen ) );
int bind        args( ( int s, struct sockaddr *name, int namelen ) );
void    bzero       args( ( char *b, int length ) );
int close       args( ( int fd ) );
int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
int gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int listen      args( ( int s, int backlog ) );
int read        args( ( int fd, char *buf, int nbyte ) );
int select      args( ( int width, fd_set *readfds, fd_set *writefds,
                fd_set *exceptfds, struct timeval *timeout ) );
int setsockopt  args( ( int s, int level, int optname, void *optval,
                int optlen ) );
int socket      args( ( int domain, int type, int protocol ) );
int write       args( ( int fd, char *buf, int nbyte ) );
#endif

/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list = NULL; /* All open descriptors     */
DESCRIPTOR_DATA *   d_next;     /* Next descriptor in loop  */
FILE *          fpReserve;      /* Reserved file handle     */
bool            god;        /* All new chars are gods!  */
bool            merc_down;      /* Shutdown         */
bool            wizlock;        /* Game is wizlocked        */
bool            newlock;        /* Game is newlocked        */
char            str_boot_time[MAX_INPUT_LENGTH];
time_t          current_time;   /* time of this pulse */
time_t          update_time;    /* time of this pulse */
time_t          lag_update_time;    /* time of this pulse */
time_t          boot_time;  /* time of this pulse */
time_t          tml_boot_time;  /* tml time since game began */
bool            MOBtrigger = TRUE;  /* act() switch                 */

bool            fCopyover;
bool            fCopyoverRecover;

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void    game_loop_mac_msdos args( ( void ) );
bool    read_from_descriptor    args( ( DESCRIPTOR_DATA *d ) );
bool    write_to_descriptor args( ( int desc, char *txt, int length ) );
#endif

#if defined(unix)
void    game_loop_unix      args( ( int control ) );
int init_socket     args( ( int port ) );
void    init_descriptor     args( ( int control ) );
bool    read_from_descriptor    args( ( DESCRIPTOR_DATA *d ) );
#endif

/*
 * Other local functions (OS-independent).
 */
bool    check_reconnect     args( ( DESCRIPTOR_DATA *d, char *name,
                    bool fConn ) );
bool    check_playing       args( ( DESCRIPTOR_DATA *d, char *name ) );
int main            args( ( int argc, char **argv ) );
void    nanny           args( ( DESCRIPTOR_DATA *d, char *argument ) );
void    new_nanny       args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool    process_output      args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void    read_from_buffer    args( ( DESCRIPTOR_DATA *d ) );
void    stop_idling     args( ( CHAR_DATA *ch ) );
void    bust_a_prompt           args( ( CHAR_DATA *ch ) );
void    check_dup_socket   args( ( CHAR_DATA *ch ) );
void    close_dup_sockets  args( ( DESCRIPTOR_DATA *d, const char *name ) );
int get_start_room(bool isevil, bool isnewbie);

/* Needs to be global because of do_copyover */
//int port, control;
/* Count stuff. */
/*sh_int          countCur = 0;
sh_int          countMax = 0;
sh_int          countMaxDay = 0;
sh_int          countHour = 0;
sh_int          countArr[24];
sh_int          countMaxDoW[7];*/
/* This file holds the copyover data. */
#define COPYOVER_FILE       "../area/copyover%04d.dat"
#define COPYOVER_GROUP_FILE "../area/cgrp%04d.dat"
#define COPYOVER_TIME_FILE  "../Txt/ctime%04d.dat"

