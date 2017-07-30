/***************************************************************************
 *                          SMC version 0.9.7b3                            *
 *          Additions to Rom2.3 (C) 1995, 1996 by Tom Adriaenssen          *
 *                                                                         *
 * Share and enjoy! But please give the original authors some credit.      *
 *                                                                         *
 * Ideas, tips, or comments can be send to:                                *
 *          tadriaen@zorro.ruca.ua.ac.be                                   *
 *          shadow@www.dma.be                                              *
 ***************************************************************************/
/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int				sh_int;
typedef int				bool;
#define unix
#else
typedef short   int			sh_int;
typedef unsigned char			bool;
#endif

#define IDENT_PORT			113

typedef unsigned long			uint32;

char *resolve_address( uint32 address );

char *resolve_username( 
	uint32 address, sh_int local_port, sh_int remote_port, int sd );

char *replace_char( char *pstr, char *oldch, char *newch )
{ 
    char *str;
   
    for ( str=pstr; *pstr != '\0'; pstr++ )
        if ( *pstr == *oldch )
            *pstr = *newch;

    return str;
}

int init_socket( uint32 my_ip )
{
  static struct sockaddr_in sa_zero;
  struct sockaddr_in sa;
  char returnbuf[1024];
  int sd;
  int option = 1;

  if ( ( sd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
      mprintf( sizeof(returnbuf), returnbuf, strerror( errno ) );
      printf( "%s\n\r", replace_char( returnbuf, " ", "_" ) );
      exit( 0 );
    }

  if ( setsockopt( sd, 
		   SOL_SOCKET, SO_REUSEADDR, (char *) &option, sizeof(option) ) < 0 )
    {
      close( sd );
      mprintf( sizeof(returnbuf),returnbuf, strerror( errno ) );
      printf( "%s\n\r", replace_char( returnbuf, " ", "_" ) );
      exit( 0 );
    }

  sa		    = sa_zero;
  sa.sin_family   = AF_INET;
  sa.sin_port     = htons( 0 );
   
  sa.sin_addr.s_addr = my_ip;

  if ( bind( sd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
      close( sd );
      mprintf( sizeof(returnbuf), returnbuf, strerror( errno ) );
      printf( "%s\n\r", replace_char( returnbuf, " ", "_" ) );
      exit( 0 );
    }    
  return sd;
}

void main( int argc, char *argv[] )
{
    sh_int local_pt, remote_pt;
    int sd;
    uint32 remote_ip, local_ip;
    char userbuf[1024], addrbuf[1024];

    if ( argc != 5 )
    {
    	printf( "unknown.host\n\r" );
    	exit( 0 );
    }

    local_pt  = atoi( argv[1] );
    remote_ip = strcmp(argv[0],"restest") ? atoi(argv[2]) : inet_addr(argv[2]);
    remote_pt = atoi( argv[3] );
    local_ip  = inet_addr(argv[4]);

    addrbuf[0] = '\0';    
    userbuf[0] = '\0';    
        
    mprintf(sizeof(addrbuf), addrbuf, resolve_address( remote_ip ) );

    sd = init_socket( local_ip );
    
    mprintf( sizeof(userbuf), userbuf, resolve_username( remote_ip, local_pt, remote_pt, sd ) );

    printf( "%s %s\n\r", addrbuf, replace_char( userbuf, " ", "_"  ) );
    
    exit( 0 );
}


char *resolve_address( uint32 address )
{
    static char addr_str[256];
    struct hostent *from;
    int addr;
    
    if ( (from=gethostbyaddr(
    	        (char*)&address, sizeof(address), AF_INET ))!=NULL )
    {
    	strcpy( addr_str, 
    		strcmp( from->h_name, "localhost" ) ? 
  		from->h_name : "local-host" );
    }
    else
    {
    	addr = ntohl( address );
    	mprintf( sizeof(addr_str), addr_str, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF );
    }
    
    return addr_str;
}


char *resolve_username( 
        uint32 address, sh_int local_port, sh_int remote_port, int sd )
{
    char request[255];
    char status[32], errtype[32];
    static char user[256]; 
    int id, i;
    sh_int peer_port, my_port;
    static struct sockaddr_in sock;
    fd_set in_set, out_set;
    struct timeval timeout;
    static struct timeval nulltime;
    struct timeval last_time;
    long secDelta;
    long usecDelta;

    gettimeofday( &last_time, NULL );
    
    sock.sin_family = AF_INET;
    sock.sin_port=htons( IDENT_PORT );
    sock.sin_addr.s_addr=address; 

    if ( (id=connect( sd, (struct sockaddr *) &sock, sizeof(sock)))<0 )
    {
    	/*
    	 * Connect failed, so either the remote identd is not running on
    	 * the remote machine or an error has occured.
    	 */
    	close( sd );
    	return "";
    }
     
    mprintf( sizeof(request), request, "%d,%d", remote_port, local_port ); 

    if ( ( i=send( sd, &request, strlen( request )+1, 0 ) )<0 )
    {
    	close( sd );
    	return "request failed";
    }
    
    FD_ZERO( &in_set );
    FD_ZERO( &out_set );
    FD_SET( sd, &in_set );
    FD_SET( sd, &out_set );
    
    gettimeofday( &timeout, NULL );
    usecDelta	= ((int) last_time.tv_usec) - ((int) timeout.tv_usec)
		+ 1000000 * 60;
    secDelta	= ((int) last_time.tv_sec ) - ((int) timeout.tv_sec );
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
    timeout=nulltime;
    timeout.tv_usec = usecDelta;
    timeout.tv_sec  = secDelta;

    if ( (i=select( sd+1, &in_set, &out_set, NULL, &timeout ))<0 )
    {
    	close( sd );
    	return "";
    } 

    gettimeofday( &last_time, NULL );

    if ( i==0 )
    {
        close( sd );
        return "";
    }
    
    if ( (i=recv( sd, &request, 255, 0 ))<0 )
    {
    	close( sd );
    	return "";
    }
        
    close( sd );
    
    if ( (i=sscanf( request, "%hd , %hd : %s : %s : %s", 
    		    &peer_port, &my_port, status, errtype, user ))<5 )
    {
    	return "";
    }
    
    if ( strcmp( status, "USERID" ) )
    {
    	return "";
    }
    
    return user;
}
