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
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "recycle.h"

void show_char_eq_to_char(CHAR_DATA *ch, CHAR_DATA *victim)
{
  BUFFER *buffer = new_buf();
  bool eq_found = FALSE;

  if (IS_NPC(victim))
    bprintf( buffer, "\n{x%s{x is currently wearing the following:{x\n\r", new_capitalize( victim->short_descr ));
  else
    bprintf( buffer, "\n{x%s{x is currently wearing the following:{x\n\r", new_capitalize( victim->name ));

  if (eq_worn(victim, WEAR_LIGHT, ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn as light      %-40s\n\r{x",eq_worn(victim,WEAR_LIGHT,ch));
  }
  if (eq_worn(victim, WEAR_HEAD, ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on head       %-40s\n\r{x",eq_worn(victim,WEAR_HEAD,ch));
  }
  if (eq_worn(victim, WEAR_LEYE, ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on left eye   %-40s\n\r{x",eq_worn(victim,WEAR_LEYE,ch));
  }
  if (eq_worn(victim, WEAR_EAR_L, ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on left ear   %-40s\n\r{x",eq_worn(victim,WEAR_EAR_L,ch));
  }
  if (eq_worn(victim, WEAR_EAR_R, ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on right ear  %-40s\n\r{x",eq_worn(victim,WEAR_EAR_R,ch));
  }
  if (eq_worn(victim, WEAR_NECK_1, ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on neck       %-40s\n\r{x",eq_worn(victim,WEAR_NECK_1,ch));
  }
  if (eq_worn(victim, WEAR_NECK_2, ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on neck       %-40s\n\r{x",eq_worn(victim,WEAR_NECK_2,ch));
  }
  if (eq_worn(victim, WEAR_BACK, ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on back       %-40s\n\r{x",eq_worn(victim,WEAR_BACK,ch));
  }
  if (eq_worn(victim, WEAR_BODY, ch))	    
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on body       %-40s\n\r{x",eq_worn(victim,WEAR_BODY,ch));
  }
  if (eq_worn(victim, WEAR_CREST, ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn as crest      %-40s\n\r{x",eq_worn(victim,WEAR_CREST,ch));
  }
  if (eq_worn(victim, WEAR_ABOUT,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn about torso   %-40s\n\r{x",eq_worn(victim,WEAR_ABOUT,ch));
  }
  if (eq_worn(victim, WEAR_LAPEL,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on lapel      %-40s\n\r{x",eq_worn(victim,WEAR_LAPEL,ch));
  }
  if (eq_worn(victim, WEAR_ARMS,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on arms       %-40s\n\r{x",eq_worn(victim,WEAR_ARMS,ch));
  }
  if (eq_worn(victim, WEAR_WRIST_L, ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn around wrist  %-40s\n\r{x",eq_worn(victim,WEAR_WRIST_L,ch));
  }
  if (eq_worn(victim, WEAR_WRIST_R,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn around wrist  %-40s\n\r{x",eq_worn(victim,WEAR_WRIST_R,ch));
  }
  if (eq_worn(victim, WEAR_HANDS,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on hands      %-40s\n\r{x",eq_worn(victim,WEAR_HANDS,ch));
  }
  if (eq_worn(victim, WEAR_FINGER_L,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on finger     %-40s\n\r{x",eq_worn(victim,WEAR_FINGER_L,ch));
  }
  if (eq_worn(victim, WEAR_FINGER_R,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on finger     %-40s\n\r{x",eq_worn(victim,WEAR_FINGER_R,ch));
  }
  if (eq_worn(victim, WEAR_WAIST,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on waist      %-40s\n\r{x",eq_worn(victim,WEAR_WAIST,ch));
  }
  if (eq_worn(victim, WEAR_BAG,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Used as bag        %-40s\n\r{x",eq_worn(victim,WEAR_BAG,ch));
  }
  if (eq_worn(victim, WEAR_LEGS,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on legs       %-40s\n\r{x",eq_worn(victim,WEAR_LEGS,ch));
  }
  if (eq_worn(victim, WEAR_RFOOT,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on right foot %-40s\n\r{x",eq_worn(victim,WEAR_RFOOT,ch));
  }
  if (eq_worn(victim, WEAR_LFOOT,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on left foot  %-40s\n\r{x",eq_worn(victim,WEAR_LFOOT,ch));
  }
  if (eq_worn(victim, WEAR_FEET,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Worn on Feet       %-40s\n\r{x",eq_worn(victim,WEAR_FEET,ch));
  }
  if (eq_worn(victim,WEAR_FLOAT, ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Floating nearby    %-40s\n\r{x",eq_worn(victim,WEAR_FLOAT,ch));
  }
  if (eq_worn(victim, WEAR_HOLD,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Held in hand       %-40s\n\r{x",eq_worn(victim,WEAR_HOLD,ch));
  }
  if (eq_worn(victim, WEAR_SHIELD,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Used as shield     %-40s\n\r{x",eq_worn(victim,WEAR_SHIELD,ch));
  }
  if (eq_worn(victim, WEAR_WIELD,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Primary weapon     %-40s\n\r{x",eq_worn(victim,WEAR_WIELD,ch));
  }
  if (eq_worn(victim, WEAR_SECONDARY,ch))
  {
    eq_found = TRUE;
    bprintf( buffer, "Secondary weapon   %-40s\n\r{x",eq_worn(victim,WEAR_SECONDARY,ch));
  }

  if (eq_found == TRUE)
    page_to_char( buf_string( buffer ), ch );
  else
  {
    printf_to_char(ch, "\n{x%s{x is currently not wearing any equipment.{x\n",
      (IS_NPC(victim)
        ? new_capitalize( victim->short_descr )
        : new_capitalize( victim->name )));
  }

  free_buf( buffer );
  return;
}
void do_new_equipment( CHAR_DATA *ch, char *argument )
{
  BUFFER *buffer = new_buf();

  bprintf( buffer, "\n{CYou are currently wearing the following EQ:{x\n\r");
  bprintf( buffer, "Floating {g:{x%-40s\n",eq_worn(ch,WEAR_FLOAT,ch));
  bprintf( buffer, "Light    {g:{x%-40s\n",eq_worn(ch,WEAR_LIGHT,ch));
  bprintf( buffer, "Head     {g:{x%-40s\n",eq_worn(ch,WEAR_HEAD,ch));
  if ( IS_IMMORTAL( ch ) && eq_worn( ch, WEAR_LEYE, ch ) )
    bprintf( buffer, "Left Eye {g:{x%-40s\n",eq_worn(ch,WEAR_LEYE,ch));
  bprintf( buffer, "Ear      {g:{x%-40s\nEar      {g:{x",eq_worn(ch,WEAR_EAR_L, ch ) );
  bprintf( buffer, "%-40s\n",eq_worn(ch,WEAR_EAR_R,ch));
  bprintf( buffer, "Neck     {g:{x%-40s\nNeck     {g:{x",eq_worn(ch,WEAR_NECK_1,ch));
  bprintf( buffer, "%-40s\n",eq_worn(ch,WEAR_NECK_2,ch));
  bprintf( buffer, "Back     {g:{x%-40s\n",eq_worn(ch,WEAR_BACK,ch));
  bprintf( buffer, "Body     {g:{x%-40s\n",eq_worn(ch,WEAR_BODY,ch));
  bprintf( buffer, "Crest    {g:{x%-40s\n",eq_worn(ch,WEAR_CREST,ch));
  bprintf( buffer, "Torso    {g:{x%-40s\n",eq_worn(ch,WEAR_ABOUT,ch));
  bprintf( buffer, "Lapel    {g:{x%-40s\n",eq_worn(ch,WEAR_LAPEL,ch));
  bprintf( buffer, "Arms     {g:{x%-40s\n",eq_worn(ch,WEAR_ARMS,ch));
  bprintf( buffer, "Hands    {g:{x%-40s\n",eq_worn(ch,WEAR_HANDS,ch));
  bprintf( buffer, "Wrist    {g:{x%-40s\nWrist    {g:{x",eq_worn(ch,WEAR_WRIST_L,ch));
  bprintf( buffer, "%-40s\n",eq_worn(ch,WEAR_WRIST_R,ch));
  bprintf( buffer, "Finger   {g:{x%-40s\nFinger   {g:{x",eq_worn(ch,WEAR_FINGER_L,ch));
  bprintf( buffer, "%-40s\n",eq_worn(ch,WEAR_FINGER_R,ch));
  bprintf( buffer, "Waist    {g:{x%-40s\n",eq_worn(ch,WEAR_WAIST,ch));
  bprintf( buffer, "Bag      {g:{x%-40s\n",eq_worn(ch,WEAR_BAG,ch));
  bprintf( buffer, "Primary  {g:{x%-40s\nSecondary{g:{x",eq_worn(ch,WEAR_WIELD,ch));
  bprintf( buffer, "%-40s\n",eq_worn(ch,WEAR_SECONDARY,ch));
  bprintf( buffer, "Shield   {g:{x%-40s\nHeld     {g:{x",eq_worn(ch,WEAR_SHIELD,ch));
  bprintf( buffer, "%-40s\n",eq_worn(ch,WEAR_HOLD,ch));
  bprintf( buffer, "Legs     {g:{x%-40s\n",eq_worn(ch,WEAR_LEGS,ch));
  bprintf( buffer, "Feet     {g:{x%-40s\n",eq_worn(ch,WEAR_FEET,ch));

  page_to_char( buf_string( buffer ), ch );
 free_buf( buffer );
  return;

}


char *eq_worn(CHAR_DATA *ch, int iWear, CHAR_DATA *show_ch)
{
  OBJ_DATA *obj;

  if ((obj = get_eq_char(ch,iWear)) != NULL){
  
    if ( can_see_obj( show_ch, obj ) )
      {
	return(format_obj_to_char(obj, show_ch, TRUE, FALSE));
      }
    else
      {
	return( "Something");
      }
  }
  return(NULL);
}


