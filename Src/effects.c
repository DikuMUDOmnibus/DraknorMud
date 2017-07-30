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
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"

void acid_effect(void *vo, int level, int dam, int target)
{
  if (target == TARGET_ROOM) /* nail objects on the floor */
    {
      ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
      OBJ_DATA *obj=NULL, *obj_next=NULL;

      for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	  if (obj == NULL)
	    continue;
	  obj_next = obj->next_content;
	  acid_effect(obj,level,dam,TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_CHAR)  /* do the effect on a victim */
    {
      CHAR_DATA *victim = (CHAR_DATA *) vo;
      OBJ_DATA *obj, *obj_next=NULL;
	
      /* let's toast some gear */
      for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	  if (obj == NULL)
	    continue;
	  obj_next = obj->next_content;
	  acid_effect(obj,level,dam,TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_OBJ) /* toast an object */
    {
      OBJ_DATA *obj = (OBJ_DATA *) vo;
      OBJ_DATA *t_obj,*n_obj;
      int chance;
      char *msg;

      if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	  ||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	  ||  number_range(0,4) == 0)
	return;

      chance = level / 4 + dam / 10;

      if (chance > 25)
	chance = (chance - 25) / 2 + 25;
      if (chance > 50)
	chance = (chance - 50) / 2 + 50;

      if (IS_OBJ_STAT(obj,ITEM_BLESS))
	chance -= 5;

      chance -= obj->level * 2;

      switch (obj->item_type)
	{
	default:
	  return;
	case ITEM_CONTAINER:
	case ITEM_CORPSE_PC:
	case ITEM_CORPSE_NPC:
	  msg = "$p fumes and dissolves.";
	  break;
	case ITEM_ARMOR:
	  msg = "$p is pitted and etched.";
	  break;
	case ITEM_CLOTHING:
	  msg = "$p is corroded into scrap.";
	  break;
	case ITEM_STAFF:
	case ITEM_WAND:
	  chance -= 10;
	  msg = "$p corrodes and breaks.";
	  break;
	case ITEM_SCROLL:
	  chance += 10;
	  msg = "$p is burned into waste.";
	  break; 
	}

    chance = URANGE(5,chance,95);

    if (number_percent() > chance)
	    return;

    if (obj->carried_by != NULL)
	    act(msg,obj->carried_by,obj,NULL,TO_ALL);
    else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    act(msg,obj->in_room->people,obj,NULL,TO_ALL);

    if (obj->item_type == ITEM_ARMOR)  /* etch it */
	{
	  AFFECT_DATA *paf;
	  bool af_found = FALSE;
	  int i;

	  affect_enchant(obj);

	  for ( paf = obj->affected; paf != NULL; paf = paf->next)
      {
	      if ( paf->location == APPLY_AC)
          {
		  af_found = TRUE;
		  paf->type = gsn_acid_breath;
		  paf->modifier += 1;
		  paf->level = UMAX(paf->level,level);
		  obj->condition = UMAX( 5, obj->condition - 10 );
		  break;
          }
      }
 
	  if (!af_found)
            /* needs a new affect */
      {
	      paf = new_affect();
 
	      paf->where	= TO_OBJECT;
	      paf->type       = gsn_acid_breath;
	      paf->level      = level;
	      paf->duration   = -1;
	      paf->location   = APPLY_AC;
	      paf->modifier   = 1;
	      paf->bitvector  = 0;
	      paf->next       = obj->affected;
	      obj->affected   = paf;
	      obj->condition	= UMAX( 5, obj->condition - 10 );
      }
 
	  if (obj->carried_by != NULL && obj->wear_loc != WEAR_NONE)
	  {
		obj->condition	= UMAX( 5, obj->condition - 20 );
		for (i = 0; i < 4; i++)
		    obj->carried_by->armor[i] += 1;
	  }

	  return;
	}

      /* get rid of the object */
      if (obj->contains)  /* dump contents */
	{
	  for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
	    {
	      n_obj = t_obj->next_content;
	      obj_from_obj(t_obj);
	      if (obj->in_room != NULL)
		obj_to_room(t_obj,obj->in_room);
	      else if (obj->carried_by != NULL)
		obj_to_room(t_obj,obj->carried_by->in_room);
	      else
		{
		  extract_obj(t_obj);
		  continue;
		}

	      acid_effect(t_obj,level/2,dam/2,TARGET_OBJ);
	    }
 	}

      extract_obj(obj);
      return;
    }
}


void cold_effect(void *vo, int level, int dam, int target)
{
  if (target == TARGET_ROOM) /* nail objects on the floor */
    {
      ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
      OBJ_DATA *obj, *obj_next=NULL;
 
      for (obj = room->contents; obj != NULL; obj = obj_next)
        {
	  if (obj == NULL)
	    continue;
	  obj_next = obj->next_content;
	  cold_effect(obj,level,dam,TARGET_OBJ);
        }
      return;
    }

  if (target == TARGET_CHAR) /* whack a character */
    {
      CHAR_DATA *victim = (CHAR_DATA *) vo;
      OBJ_DATA *obj, *obj_next=NULL;
	
      /* chill touch effect */
      if (!saves_spell(level/4 + dam / 20, victim, DAM_COLD))
	{
	  AFFECT_DATA af;

	  act("$n turns {Bbl{cue{x and {csh{Bi{cve{Br{cs{x.",victim,NULL,NULL,TO_ROOM);
	  act("A {cch{Bi{cl{Bl{x sinks deep into your bones.",victim,NULL,NULL,TO_CHAR);
	  af.where     = TO_AFFECTS;
	  af.type      = skill_lookup("chill touch");
	  af.level     = level;
	  af.duration  = 6;
	  af.location  = APPLY_STR;
	  af.modifier  = -1;
	  af.bitvector = 0;
	  affect_join( victim, &af );
	}

      /* hunger! (warmth sucked out */
      if (!IS_NPC(victim))
	gain_condition(victim,COND_HUNGER,dam/-20);

      /* let's toast some gear */
      for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	  if (obj == NULL)
	    continue;
	  obj_next = obj->next_content;
	  cold_effect(obj,level,dam,TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_OBJ) /* toast an object */
    {
      OBJ_DATA *obj = (OBJ_DATA *) vo;
      int chance;
      char *msg;

      if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	  ||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	  ||  number_range(0,4) == 0)
	return;

      chance = level / 4 + dam / 10;

      if (chance > 25)
	chance = (chance - 25) / 2 + 25;
      if (chance > 50)
	chance = (chance - 50) / 2 + 50;

      if (IS_OBJ_STAT(obj,ITEM_BLESS))
	chance -= 5;

      chance -= obj->level * 2;

      switch(obj->item_type)
	{
	default:
	  return;
	case ITEM_POTION:
	  msg = "$p freezes and shatters!";
	  chance += 25;
	  break;
	case ITEM_DRINK_CON:
	  msg = "$p freezes and shatters!";
	  chance += 5;
	  break;
	}

      chance = URANGE(5,chance,95);

      if (number_percent() > chance)
	return;

      if (obj->carried_by != NULL)
	act(msg,obj->carried_by,obj,NULL,TO_ALL);
      else if (obj->in_room != NULL && obj->in_room->people != NULL)
	act(msg,obj->in_room->people,obj,NULL,TO_ALL);

      extract_obj(obj);
      return;
    }
}
   


void fire_effect(void *vo, int level, int dam, int target)
{
  if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
      ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
      OBJ_DATA *obj, *obj_next=NULL;

      if (room == NULL) {
	bugf("BUG: Room equal NULL in fire effect.\n\r");
	return;
      }
      for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	  if (obj == NULL)
	    continue;
	  obj_next = obj->next_content;
	  fire_effect(obj,level,dam,TARGET_OBJ);
	}
      return;
    }
 
  if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
      CHAR_DATA *victim = (CHAR_DATA *) vo;
      OBJ_DATA *obj, *obj_next=NULL;

      /* chance of blindness */
      if (!IS_AFFECTED(victim,AFF_BLIND)
	  &&  !saves_spell(level / 4 + dam / 20, victim,DAM_FIRE))
	{
	  AFFECT_DATA af;
	  act("$n is blinded by {Ds{wm{Doke{x!",victim,NULL,NULL,TO_ROOM);
	  act("Your eyes tear up from {Ds{wm{Doke{x...you can't see a thing!",
	      victim,NULL,NULL,TO_CHAR);
	 
	  af.where        = TO_AFFECTS;
	  af.type         = skill_lookup("fire breath");
	  af.level        = level;
	  af.duration     = number_range(0,level/15);
	  af.location     = APPLY_HITROLL;
	  af.modifier     = -4;
	  af.bitvector    = AFF_BLIND;
 
	  affect_to_char(victim,&af);
	}

      /* getting thirsty */
      if (!IS_NPC(victim))
	gain_condition(victim,COND_THIRST,dam/-20);

      /* let's toast some gear! */
      for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	  if (obj == NULL)
	    continue;
	  obj_next = obj->next_content;

	  fire_effect(obj,level,dam,TARGET_OBJ);
        }
      return;
    }

  if (target == TARGET_OBJ)  /* toast an object */
    {
      OBJ_DATA *obj = (OBJ_DATA *) vo;
      OBJ_DATA *t_obj,*n_obj;
      int chance;
      char *msg;

      if ( ( obj->pIndexData->vnum == OBJ_VNUM_CLEAN_SCROLL )
      && ( obj->wear_loc == WEAR_HOLD ) )
        return; // don't destroy blank parchments being held (it screws with do_erase)

      if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	  ||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	  ||  number_range(0,4) == 0)
	return;
 
      chance = level / 4 + dam / 10;
 
      if (chance > 25)
	chance = (chance - 25) / 2 + 25;
      if (chance > 50)
	chance = (chance - 50) / 2 + 50;

      if (IS_OBJ_STAT(obj,ITEM_BLESS))
	chance -= 5;
      chance -= obj->level * 2;

      switch ( obj->item_type )
        {
        default:             
	  return;
        case ITEM_CONTAINER:
	  msg = "$p ignites and burns!";
	  break;
        case ITEM_POTION:
	  chance += 25;
	  msg = "$p bubbles and boils!";
	  break;
        case ITEM_SCROLL:
	  chance += 50;
	  msg = "$p crackles and burns!";
	  break;
        case ITEM_STAFF:
	  chance += 10;
	  msg = "$p smokes and chars!";
	  break;
        case ITEM_WAND:
	  msg = "$p sparks and sputters!";
	  break;
        case ITEM_FOOD:
	  msg = "$p blackens and crisps!";
	  break;
        case ITEM_PILL:
	  msg = "$p melts and drips!";
	  break;
        }

      chance = URANGE(5,chance,95);

      if (number_percent() > chance)
	return;
 
      if (obj->carried_by != NULL)
	act( msg, obj->carried_by, obj, NULL, TO_ALL );
      else if (obj->in_room != NULL && obj->in_room->people != NULL)
	act(msg,obj->in_room->people,obj,NULL,TO_ALL);

      if (obj->contains)
        {
	  /* dump the contents */
 
	  for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
            {
	      n_obj = t_obj->next_content;
	      obj_from_obj(t_obj);
	      if (obj->in_room != NULL)
		obj_to_room(t_obj,obj->in_room);
	      else if (obj->carried_by != NULL)
		obj_to_room(t_obj,obj->carried_by->in_room);
	      else
		{
		  extract_obj(t_obj);
		  continue;
		}
	      fire_effect(t_obj,level/2,dam/2,TARGET_OBJ);
            }
        }
 
      extract_obj( obj );
      return;
    }
}

void poison_effect(void *vo,int level, int dam, int target)
{
  if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
      ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
      OBJ_DATA *obj, *obj_next=NULL;
 
      for (obj = room->contents; obj != NULL; obj = obj_next)
        {
	  if (obj == NULL)
	    continue;
	  obj_next = obj->next_content;
	  poison_effect(obj,level,dam,TARGET_OBJ);
        }
      return;
    }
 
  if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
      CHAR_DATA *victim = (CHAR_DATA *) vo;
      OBJ_DATA *obj, *obj_next=NULL;

      /* chance of poisoning */
      if (!saves_spell(level / 4 + dam / 20,victim,DAM_POISON))
        {
	  AFFECT_DATA af;

	  send_to_char("You feel {gpo{ci{Gs{gon{x coursing through your veins.\n\r",
		       victim);
	  act("$n looks very {ci{gll{x.",victim,NULL,NULL,TO_ROOM);

	  af.where     = TO_AFFECTS;
	  af.type      = gsn_poison;
	  af.level     = level;
	  af.duration  = level / 2;
	  af.location  = APPLY_STR;
	  af.modifier  = -1;
	  af.bitvector = AFF_POISON;
	  affect_join( victim, &af );
        }

      /* equipment */
      for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	  if (obj == NULL)
	    continue;
	  obj_next = obj->next_content;
	  poison_effect(obj,level,dam,TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_OBJ)  /* do some poisoning */
    {
      OBJ_DATA *obj = (OBJ_DATA *) vo;
      int chance;
	

      if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	  ||  IS_OBJ_STAT(obj,ITEM_BLESS)
	  ||  number_range(0,4) == 0)
	return;

      chance = level / 4 + dam / 10;
      if (chance > 25)
	chance = (chance - 25) / 2 + 25;
      if (chance > 50)
	chance = (chance - 50) / 2 + 50;

      chance -= obj->level * 2;

      switch (obj->item_type)
	{
	default:
	  return;
	case ITEM_FOOD:
	  break;
	case ITEM_DRINK_CON:
	  if (obj->value[0] == obj->value[1])
	    return;
	  break;
	}

      chance = URANGE(5,chance,95);

      if (number_percent() > chance)
	return;

      obj->value[3] = 1;
      return;
    }
}


void shock_effect(void *vo,int level, int dam, int target)
{
  if (target == TARGET_ROOM)
    {
      ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
      OBJ_DATA *obj, *obj_next=NULL;

      for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	  if (obj == NULL)
	    continue;
	  obj_next = obj->next_content;
	  shock_effect(obj,level,dam,TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_CHAR)
    {
      CHAR_DATA *victim = (CHAR_DATA *) vo;
      OBJ_DATA *obj, *obj_next=NULL;

      /* daze and confused? */
      if (!saves_spell(level/4 + dam/20,victim,DAM_LIGHTNING))
	{
	  send_to_char("Your muscles stop responding.\n\r",victim);
	  DAZE_STATE(victim,UMAX(12,level/4 + dam/20));
	}

      /* toast some gear */
      for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	  if (obj == NULL)
	    continue;
	  obj_next = obj->next_content;
	  shock_effect(obj,level,dam,TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_OBJ)
    {
      OBJ_DATA *obj = (OBJ_DATA *) vo;
      int chance;
      char *msg;

      if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	  ||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	  ||  number_range(0,4) == 0)
	return;

      chance = level / 4 + dam / 10;

      if (chance > 25)
	chance = (chance - 25) / 2 + 25;
      if (chance > 50)
	chance = (chance - 50) /2 + 50;

      if (IS_OBJ_STAT(obj,ITEM_BLESS))
	chance -= 5;

      chance -= obj->level * 2;

      switch(obj->item_type)
	{
	default:
	  return;
	case ITEM_WAND:
	case ITEM_STAFF:
	  chance += 10;
	  msg = "$p overloads and explodes!";
	  break;
	case ITEM_JEWELRY:
	  chance -= 10;
	  msg = "$p is fused into a worthless lump.";
	}
	
      chance = URANGE(5,chance,95);

      if (number_percent() > chance)
	return;

      if (obj->carried_by != NULL)
	act(msg,obj->carried_by,obj,NULL,TO_ALL);
      else if (obj->in_room != NULL && obj->in_room->people != NULL)
	act(msg,obj->in_room->people,obj,NULL,TO_ALL);

      extract_obj(obj);
      return;
    }
}
