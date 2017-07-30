/****************************************************************************\
 *                                                                          *
 *  Here goes all the credits and such. Since this is an addie by Merak and *
 *  Robert Leonard, we take the most credits. Others are honored elsewhere  *
 *  and everybody know I'm lazy, so I stop here.                            *
 *                                                                          *
\****************************************************************************/

#if defined ( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include <stdarg.h>
#include "magic.h"
#include "recycle.h"
#include "tables.h"

/*
 * Gives a pointer to an obj-storage, container, ch, room...
 */
void *string_to_container( CHAR_DATA *ch, char *argument, int *from_flag )
{
  void *container = NULL;

  if ( argument[0] == '\0' )
  {
    *from_flag |= FROM_NOT_FOUND;
    return container;
  }

  if ( !strcmp( argument, "room" ) )
  {
    *from_flag = FROM_ROOM;
     return ch->in_room;
  }

  // Search for container
  if ( ( container = get_obj_here( ch, argument ) ) )
  {
    *from_flag = FROM_OBJ;
    return container;
  }

  // Search for char
  if ( ( container = get_char_room( ch, argument ) ) )
  {
    *from_flag = FROM_CH;
    return container;
  }

  *from_flag |= FROM_NOT_FOUND;
  return container;
}

/*
 * Parse a string into an objlist and possibly source and target
 */
bool parse_objhandling( CHAR_DATA *ch, char *objlist,
                                       void **source, int *from_flag,
                                       void **target, int *to_flag,
                                       char *argument )
{
  char arg[MIL];

  *from_flag |= FROM_DEFAULT;
  *to_flag   |= FROM_DEFAULT;

  /* ACHTUNG!! We must consider the case of a totally empty list! */
  strcpy( objlist, "" );

  one_argument( argument, arg );
  if ( arg[0] == '\0' ) return TRUE; // This means it has failed, actually

  for ( strcpy( arg, "and" ); !strcmp( arg, "and" );
        argument = one_argument( argument, arg ) )
  {
    argument = one_argument( argument, arg );

    if ( is_number( arg ) )
    {
      strcat( strcat( objlist, arg ), "*" );
      argument = one_argument( argument, arg );
    }
    strcat( strcat( objlist, arg ), " " );
  }

 /*
  *  Now we have set up an objlist, which is to be parsed by the caller.
  *  Here we are to find the first location.
  */
  if ( arg[0] == '\0' ) return FALSE;

  if ( !strcmp( arg, "from" ) )
  {
    argument = one_argument( argument, arg );
    // Use arg to find a source -> obj, ch or room
    if ( !( *source = string_to_container( ch, arg, from_flag ) ) )
        return TRUE; // TRUE means failure
    argument = one_argument( argument, arg );

  }
  if ( arg[0] == '\0' ) return FALSE;

  if ( !strcmp( arg, "to" ) )
  {
    argument = one_argument( argument, arg );
    // Use arg to find a target
    if ( !( *target = string_to_container( ch, arg, to_flag ) ) )
        return TRUE;  // This means failure
    argument = one_argument( argument, arg );

  }
  if ( arg[0] == '\0' ) return FALSE;

  if ( !strcmp( arg, "from" ) )
  {
    argument = one_argument( argument, arg );
    // Yes, I know it looks strange, but I know what I am doing :)
    if ( !( *source = string_to_container( ch, arg, from_flag ) ) )
        return TRUE; // Failure!
    argument = one_argument( argument, arg );

  }
  if ( arg[0] == '\0' ) return FALSE;

  if ( IS_SET( *from_flag, FROM_DEFAULT ) )
  {
    if ( !( *source = string_to_container( ch, arg, from_flag ) ) )
        return TRUE;
    argument = one_argument( argument, arg );

  }
  if ( arg[0] == '\0' ) return FALSE;

  if ( IS_SET( *to_flag, FROM_DEFAULT ) )
  {
    if ( !( *target = string_to_container( ch, arg, to_flag ) ) )
        return TRUE;
    argument = one_argument( argument, arg );

  }
  if ( arg[0] == '\0' ) return FALSE;

  if ( !strcmp( arg, "to" ) )
  {
    argument = one_argument( argument, arg );
    // Use arg to find a target
    if ( !( *target = string_to_container( ch, arg, to_flag ) ) )
        return TRUE;  // This means failure
    argument = one_argument( argument, arg );

  }
  if ( arg[0] == '\0' ) return FALSE;

  if ( !strcmp( arg, "from" ) )
  {
    argument = one_argument( argument, arg );
    // Yes, I know it looks strange, but I know what I am doing :)
    if ( !( *source = string_to_container( ch, arg, from_flag ) ) )
        return TRUE; // Failure!
    argument = one_argument( argument, arg );

  }

  return FALSE; // This means it has succeeded, actually!
}

/*
 *  This function moves objects from one list to another according
 *  to a suitable filtering function.
 */
void obj_list_to_list( CHAR_DATA *ch, char *arg, 
                       OBJ_DATA **source, int from_flag,
                       OBJ_DATA **target, int to_flag )
{
  OBJ_DATA *Pobj, obj_next;
  bool all = FALSE;
  int  amount, count;
  char arg1[MIL], arg2[MIL];

  if ( !strcmp( arg, "all" ) )
    all = TRUE;
  else
    amount = mult_argument( arg, arg2 );
  if ( !str_prefix( "all.", arg2 ) )
  {
    strcpy( arg1, arg2[4] )
    count = -1;
  }
  else
    count = number_argument( arg1, arg );

  /* Main loop goes here.... need to think :P */
  for ( obj = *source; obj; obj = obj_next )
  {
    obj_next = obj->next;
    /* Here goes a check to see if we are to move this one... */
    if ( all || is_name( arg1, obj->name ) )
    {
      if ( IS_SET( to_flag, FROM_CHAR ) 
      && ( !CAN_WEAR( obj, ITEM_TAKE ) || !can_loot( ch, obj )
      || get_obj_number( obj ) + ch->carry_number > can_carry_n( ch )
      || get_carry_weight( ch ) + get_obj_weight( obj ) > can_carry_w( ch ) 

      ) )  // This ^ will become a list for cases where a ch is the reciever
      {
        source = &obj->next;
        continue;
      }
       obj->next    = *target;   // This is pretty clear, put the object in the
      *target       =  obj;      // beginning of their target list....

      if ( obj->item_type == ITEM_MONEY )
      {

        extract_obj( obj );
      }
      else
        *source       =  obj_next; // And putting the pointer of next source?
    }
    else
    {
      source        = &obj->next; // If this object wasn't moved, that's where
    }                             // we start the sourcelist from :)
  }
}

