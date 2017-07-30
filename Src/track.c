/***************************************************************************
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 *									   *
 *  Track algorithm by Wolfgang Schmid (e8725232@stid1.tuwien.ac.at)	   *
 *  Credits: uhm... i think i used 12 lines or so from merc code by Turtle *
 ***************************************************************************/

/***************************************************************************
 *  Abysmal Realms is copyright (C) 1997-1998 by Dennis Reichel and	   *
 *  Christopher Eaker as a work in progress.  This code is not to be used  *
 *  or distributed except with the express permission of the authors.	   *
 ***************************************************************************/

/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "special.h"

/* #define NODE_DEBUG  */
#define NODE_SIZE	1000  /* array size */
#define NODE_COLL	600   /* beginning of hash-collision storage */
#define NODE_HASH	599   /* prim */

void init_node		args( ( void ) );
void destroy_open	args( ( void ) );


typedef struct	node_data		NODE_DATA;
typedef struct	open_data		OPEN_DATA;


struct node_data
{
  int			key;
  sh_int		dist;
  sh_int		dir;
};


struct open_data
{
  int			inode;
  OPEN_DATA		*next;
};


NODE_DATA		node		[NODE_SIZE];
OPEN_DATA		*open_first;
OPEN_DATA		*open_last;


#ifdef NODE_DEBUG
int node_coll;
int node_norm;
#endif


/* hash function */
#define HASH(key) ((key) % NODE_HASH)


#ifdef NODE_DEBUG
/* prints out the open list */
void print_open()
{
  struct open_data *open;
  int count= 0;

  for (open=open_first; open; open= open->next)
    {
      NOTE_DATA *enode;
      count++;
      enode= &node[open->inode];
      printf("open %d: inode %d, vnum %d, distance %d, dir %d\n",
	     count, open->inode, enode->key, enode->dist, enode->dir);
    }
  printf("---------------------------------------------------\n");
}
#endif


void init_node()
{
  int i;

  for ( i = 0; i < NODE_SIZE; i++)
    node[i].key= -1;
}


int exit_ok( EXIT_DATA *pExit )
{
  if ( pExit == NULL
       ||	 pExit->u1.to_room == NULL )
    return FALSE;

  return TRUE;
}


bool quarry_in_room( CHAR_DATA *ch, int key, char *name )
{
  ROOM_INDEX_DATA	*pRoom;
  CHAR_DATA		*rch;

  pRoom = get_room_index( key );

  for ( rch = pRoom->people; rch != NULL; rch = rch->next_in_room )
    {
      /*	if ( !IS_DELETED( rch )
		&&  */
      if (can_see( ch, rch )
	  &&   is_name( name, rch->name ) )
	return TRUE;
    }

  return FALSE;
}


void destroy_open()
{
  OPEN_DATA	*open_elem;

  for ( ; open_first; )
    {
      open_elem = open_first;
      open_first = open_first->next;
      free( open_elem );
    }
}


void to_closed( OPEN_DATA *open )
{
  OPEN_DATA *open_elem;

  if ( open_first == open )
    {
      open_first = open->next;
      free( open );
      return;
    }

  for ( open_elem = open_first; open_elem; open_elem = open_elem->next )
    if ( open_elem->next == open )
      break;

  open_elem->next = open_elem->next->next;
  free( open );
}


int in_node( int key )
{
  int inode = HASH( key );

  if ( node[inode].key != key )
    {
      for ( inode = NODE_COLL; inode < NODE_SIZE; inode++ )
	if ( node[inode].key == key )
	  break;
      if ( inode == NODE_SIZE )
	return -1;
    }
  return inode;
}


void to_open( int key, int dist, int dir )
{
  OPEN_DATA *new_open;
  int inode = HASH( key );

  if ( node[inode].key != -1 )
    {
      for ( inode = NODE_COLL; inode < NODE_SIZE; inode++ )
	if ( node[inode].key == -1 )
	  break;
      if ( inode == NODE_SIZE )
	{
#ifdef NODE_DEBUG
	  printf( "NODE ARRAY FULL!!! normal %d collisions %d\n",
		  node_norm, node_coll );
#endif
	  return;
	}
#ifdef NODE_DEBUG
      node_coll++;
#endif
    }
#ifdef NODE_DEBUG
  else
    node_norm++;
#endif

  new_open		= malloc( sizeof( OPEN_DATA ) );

  node[inode].key	= key;
  node[inode].dist	= dist;
  node[inode].dir	= dir;

  new_open->inode	= inode;
  new_open->next	= NULL;
  open_last->next	= new_open;
  open_last		= new_open;
}


/* this function creates new rooms from the current room		*/
/* when they are within tracking distance and haven't been visited yet	*/
/* they are put into the room storage and into the open list		*/
void expand( CHAR_DATA *ch, int limit, struct open_data *open, bool first )
{
  EXIT_DATA		    *exitp;
  ROOM_INDEX_DATA	*herep;
  int dir;
  int new_dist;

  herep = get_room_index( node[open->inode].key );

  for ( dir = 0; dir < MAX_DIR; dir++ )
  {
    exitp = herep->exit[dir];
    if ( exit_ok( exitp ) )
	{
	  new_dist = node[open->inode].dist + 1;
	  if (  IS_SET( exitp->exit_info, EX_CLOSED )
	  && ( !IS_AFFECTED( ch, AFF_PASS_DOOR )
	  ||	IS_SET( exitp->exit_info, EX_NOPASS ) )
	  &&   !IS_TRUSTED( ch, ANGEL ) )
	    new_dist += 7;		/* door bonus */

	  if ( new_dist <= limit
	  &&   in_node( herep->exit[dir]->u1.to_room->vnum ) == -1
	  &&   can_see_room( ch, exitp->u1.to_room ) )
	    to_open( herep->exit[dir]->u1.to_room->vnum, new_dist,
		     first ? dir : node[open->inode].dir);
	}
  }
}


sh_int find_path( CHAR_DATA *ch, int start, int limit, char *name )
{
  sh_int found= -1;

  init_node();

#ifdef NODE_DEBUG
  node_coll = 0;
  node_norm = 0;
#endif

  open_last			 = NULL;
  open_first			 = malloc( sizeof( OPEN_DATA ) );
  open_first->next		 = NULL;
  open_first->inode		 = HASH( start );
  node[open_first->inode].key  = start;
  node[open_first->inode].dist = 0;
  node[open_first->inode].dir  = -1;
  open_last			 = open_first;

#ifdef NODE_DEBUG
  print_open();
#endif
  if ( quarry_in_room( ch, node[open_first->inode].key, name ) )
    {
#ifdef NODE_DEBUG
      printf("found!!! normal %d coll %d\n", node_norm, node_coll);
#endif
      destroy_open();
      return -2;
    }
  expand( ch, limit, open_first, TRUE );
  to_closed( open_first );

  while ( open_first )
    {
#ifdef NODE_DEBUG
      print_open();
#endif
      if ( quarry_in_room( ch, node[open_first->inode].key, name ) )
	{
#ifdef NODE_DEBUG
	  printf("found!!! normal %d coll %d\n", node_norm, node_coll);
#endif
	  found = node[open_first->inode].dir;
	  free_string( ch->tracking );
	  ch->tracking = str_dup( name, ch->tracking );
	  break;
	}
      expand( ch, limit, open_first, FALSE );
      to_closed( open_first );
    }
#ifdef NODE_DEBUG
  printf( "numbers: normal %d coll %d\n", node_norm, node_coll );
#endif
  destroy_open();
  return found;
}


void real_track( CHAR_DATA *ch, char *name )
{
  int direction;
  int chance;

  /*
   * Deduct some movement.
   */
  if ( ch->move )
    ch->move -= 1;
  else
    {
      send_to_char( "You're too exhausted to hunt anyone!\n\r", ch );
      free_string( ch->tracking );
      ch->tracking = &str_empty[0];
      return;
    }

  act( "{C$n{W checks the area around for certain signs.{x",
       ch, NULL, NULL, TO_ROOM );
  WAIT_STATE( ch, skill_table[gsn_track].beats );

  /* Distance is at least 10 or higher, when level is very high. */
  direction = find_path( ch, ch->in_room->vnum, UMAX( ch->level/3, 10 ), name );

  if ( direction == -1 )
  {
    if ( IS_NULLSTR( ch->tracking ) )
	  {
	    send_to_char( "Your hunt failed to get a track of your quarry.\n\r", ch );
	    if ( ch->leader && ch->in_room == ch->leader->in_room )
	      send_to_char( "Your Hunter lost the quarry.\n\r", ch->leader );
	    free_string( ch->tracking );
	    ch->tracking = &str_empty[0];
	  }
    else
	  {
	    send_to_char( "You couldn't find a sign to hunt.\n\r", ch );
	    if ( ch->leader && ch->in_room == ch->leader->in_room )
	      send_to_char( "Your Hunter found no track.\n\r", ch->leader );
	  }
    return;
  }

  if ( direction == -2 )
  {
    send_to_char( "You found your quarry!\n\r", ch );

    if ( ch->leader && ch->in_room == ch->leader->in_room )
	    send_to_char( "Your Hunter found the quarry.\n\r", ch->leader );

    free_string( ch->tracking );
    ch->tracking = &str_empty[0];
    return;
  }

  if ( direction < 0 || direction >= MAX_DIR )
  {
    send_to_char( "Hmm... Something seems to be wrong.\n\r", ch );
    bugf( "Real_track: invalid direction %d", direction );
    free_string( ch->tracking );
    ch->tracking = &str_empty[0];
    return;
  }

  chance = get_skill( ch, gsn_track );

  /*
   * Give a random direction if the player misses the die roll.
   */
  if ( number_percent() >= chance )
    {
      do
	{
	  direction = number_door();
	}
      while ( ch->in_room->exit[direction] == NULL
	      ||	ch->in_room->exit[direction]->u1.to_room == NULL );
    }

  /*
   * Display the results of the search.
   */
  act( "Your quarry is $T from here.",
       ch, NULL, dir_name[direction], TO_CHAR );

  if ( IS_NPC( ch ) && ch->spec_fun == &spec_tracker )
    move_char( ch, direction, FALSE );
}


void do_track( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA * tracker;

  if ( IS_NPC( ch ) )
    return;

  if (IS_GHOST(ch))
  {
    send_to_char("Hunting is useless as you are still DEAD.\n\r",ch);
    return;
  }

  /*
   * Look for a tracker serving ch.  If none present,
   * see if ch can do his own tracking.
   */
  for ( tracker = ch->in_room->people ; tracker ;	tracker = tracker->next_in_room )
  {
    if ( tracker != NULL 
	  ||   !IS_NPC( tracker )
	  ||   tracker->leader != ch
	  ||   tracker->spec_fun != &spec_tracker )
	    continue;
    else
  	  break;
  }

  if (get_skill(ch, gsn_track) < 2)
  {
    send_to_char("huh??\n\r",ch);
    return;
  }

  if ( ( tracker == NULL )
  ||   ( !IS_NPC( ch ) && !is_racial_skill( ch, gsn_track )))
  {
    if ( ( get_skill( ch, gsn_track ) < 1 )
    &&   ( ch->level < skill_table[gsn_track].skill_level[ch->gameclass] ) )
    {
      send_to_char( "Hunting?  What's that?\n\r", ch );
      return;
    }
    else
      tracker = ch;
  }

  if ( IS_NULLSTR( argument ) )
  {
    if ( IS_NULLSTR( tracker->tracking ) )
    {
	    send_to_char( "Who are you trying to find?\n\r", ch );
    }
    else
    {
	    send_to_char( "You abandon the trail.\n\r", ch );
	    free_string( tracker->tracking );
	    tracker->tracking = &str_empty[0];
    }
    return;
  }
  else if ( !str_cmp( argument, "clear" )
  ||        !str_cmp( argument, "none" ) )
  {
    send_to_char( "You abandon the trail.\n\r", ch );
    free_string( tracker->tracking );
    tracker->tracking = &str_empty[0];
    return;
  }
  else if ( !IS_NULLSTR( tracker->tracking ) )
  {
	  free_string( tracker->tracking );
	  tracker->tracking = &str_empty[0];
  }

  real_track( tracker, argument );
  check_improve( tracker, gsn_track, TRUE, 1 );

}
