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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "magic.h"
#include "recycle.h"

void do_butcher(CHAR_DATA *ch, char *argument)
{

  /* Butcher skill, created by Argawal */
  /* Original Idea taken fom Carrion Fields Mud */
  /* If you have an interest in this skill, feel free */
  /* to use it in your mud if you so desire. */
  /* All I ask is that Argawal is credited with creating */
  /* this skill, as I wrote it from scratch. */

  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  int numst = 0;
  OBJ_DATA *steak= NULL;
  OBJ_DATA *obj;
  int x=0;

  one_argument(argument, arg);

  if(get_skill(ch,gsn_butcher)==0)
  {
    send_to_char("Butchering is beyond your skills.\n\r",ch);
    return;
  }

  if(arg[0]=='\0')
  {
    send_to_char("Butcher what?\n\r",ch);
    return;
  }

  obj = get_obj_list( ch, arg, ch->in_room->contents ); 
  if ( obj == NULL )
  {
    obj = get_obj_list( ch, arg, ch->carrying ); 
    if ( obj == NULL )
    {
      send_to_char( "It's not here.\n\r", ch ); 
      return; 
    }
  }

  if (obj->item_type == ITEM_CORPSE_PC)
  {
    send_to_char("You just can't bring yourself to butcher this corpse.\n\r",ch);
    return;
  }

  if( (obj->item_type != ITEM_CORPSE_NPC))
  {
    send_to_char( "You can only butcher corpses.\n\r", ch ); 
    return; 
  }

  /* create and rename the steak */
  mprintf(sizeof(buf), buf,"a steak of %s is here.", obj->short_descr);
  mprintf(sizeof(buf2), buf2,"a steak of %s", obj->short_descr);

  /* Check the skill roll, and put a random ammount of steaks here. */

  if(number_percent( ) < get_skill(ch,gsn_butcher))
  {
    numst = dice(1,4);
    for (x=1; x<= numst; x++)
    {
      steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
      free_string(steak->description);
      steak->description=str_dup(buf, steak->description);

      if (ch->level < 10)
      {
        steak->value[0] = 5;
        steak->value[1] = 5;
      }
      else
      {
        steak->value[0] = ch->level / 2;
        steak->value[1] = ch->level / 5;
      }

      free_string(steak->short_descr);
      steak->short_descr=str_dup(buf2, steak->short_descr);
      obj_to_char( steak, ch );
    }

    mprintf(sizeof(buf), buf,"$n butchers a corpse and creates %d steak%s.",numst, numst > 1 ?"s":"");
    act(buf,ch, steak, NULL, TO_ROOM );
    mprintf(sizeof(buf), buf,"You butcher a corpse and create %d steak%s.",numst, numst > 1 ?"s":"");
    act(buf, ch, steak, NULL, TO_CHAR );
    check_improve(ch,gsn_butcher,TRUE,1);
  }   
  else
  {
    act( "$n fails to butcher a corpse, and destroys it.", ch, NULL, NULL, TO_ROOM );
    act( "You fail to butcher a corpse, and destroy it.", ch, NULL, NULL, TO_CHAR );
    check_improve(ch,gsn_butcher,FALSE,1);
  } 
  /* dump items caried */
  /* Taken from the original ROM code and added into here. */

  if ( obj->item_type == ITEM_CORPSE_PC )
  {   /* save the contents */ 
    OBJ_DATA *t_obj, *next_obj; 
    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
    {
      next_obj = t_obj->next_content; 
      obj_from_obj(t_obj); 

      if (obj->in_obj) /* in another object */
        obj_to_obj(t_obj,obj->in_obj); 
      else if (obj->carried_by) /* carried */
      {
        if (obj->wear_loc == WEAR_FLOAT)
        {
          if (obj->carried_by->in_room == NULL)
            extract_obj(t_obj); 
          else
            obj_to_room(t_obj,obj->carried_by->in_room); 
        }
        else
          obj_to_char(t_obj,obj->carried_by); 
      }
      else if (obj->in_room == NULL) /* destroy it */
        extract_obj(t_obj); 
      else /* to a room */
        obj_to_room(t_obj,obj->in_room); 
    }
  }

  if ( obj->item_type == ITEM_CORPSE_NPC )
  {
    OBJ_DATA *t_obj, *next_obj; 

    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
    {
      next_obj = t_obj->next_content; 
      obj_from_obj(t_obj); 

      if (obj->in_obj) /* in another object */
        obj_to_obj(t_obj,obj->in_obj); 
      else if (obj->carried_by) /* carried */
      {
        if (obj->wear_loc == WEAR_FLOAT)
        {
          if (obj->carried_by->in_room == NULL)
            extract_obj(t_obj); 
          else
            obj_to_room(t_obj,obj->carried_by->in_room); 
        }
        else
          obj_to_char(t_obj,obj->carried_by); 
      }
      else if (obj->in_room == NULL) /* destroy it */
        extract_obj(t_obj); 
      else /* to a room */
        obj_to_room(t_obj,obj->in_room); 
    }
  }

  /* Now remove the corpse */
  extract_obj(obj);
  return;
}
