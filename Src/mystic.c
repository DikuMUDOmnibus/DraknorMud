/**************************************************************************\
*      The Draknor Codebase(Draknor) is copyright 2003 by Robert Leonard. *
*      Draknor has been created with much time and effort from many       *
*      different people's input and ideas.                                *
*                                                                         *
*      Using this code without the direct permission of its writers is    *
*      not allowed.                                                       *
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
#include "interp.h"

char *martial_style_prompt( CHAR_DATA *ch )
{
  switch ( ch->martial_style )
  {
    case STYLE_BASIC:
    default:
      return "Basic style";
      break;
    case STYLE_DRAGON:
      return "Dragon style";
      break;
    case STYLE_DRUNK:
      return "Drunk style";
      break;
    case STYLE_TIGER:
      return "Tiger style";
      break;
    case STYLE_SNAKE:
      return "Snake style";
      break;
    case STYLE_CRANE:
      return "Crane style";
      break;
    case STYLE_IRONFIST:
      return "Ironfist style";
      break;
    case STYLE_JUDO:
      return "Judo style";
      break;
    case STYLE_KARATE:
      return "Karate style";
      break;
    case STYLE_NONE:
      return "No style";
  }
}

void do_style( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *weapon;

  one_argument(argument,arg);

  if ( ch->gameclass != cMystic )
  {
    send_to_char( "A true martial artist would laugh at your post.\n\r", ch );
    return;
  }

  if (get_skill(ch,gsn_karate) < 1)
  {
    send_to_char("You must at least train in the art of karate to assume a style.\n\r",ch);
    return;
  }

  weapon = get_eq_char(ch,WEAR_WIELD);
  if (weapon != NULL)
  {
    send_to_char("You cannot use styles while wielding a weapons.\n\r",ch);
    return;
  }

  if (IS_GHOST(ch))
  {
    send_to_char("Acting like a martial artist is useless as you are still {rDEAD{x.\n\r",ch);
    return;
  }

  if ( IS_NULLSTR( arg ))
  {
    printf_to_char( ch, "You are currently using %s.\n\r", martial_style_prompt(ch));
    return;
  }

  if (!strcmp( arg,"none"))
  {
    send_to_char( "You are no longer using any specific style.\n\r", ch );
    ch->martial_style = STYLE_NONE;
    return;
  }

  if (!str_prefix(arg,"karate"))
  {
    if (get_skill(ch, gsn_karate) < 1)
    {
      send_to_char("You do not know that style.\n\r",ch);
      return;
    }
    else
      ch->martial_style = STYLE_KARATE;

    send_to_char( "You take on a karate stance.\n\r", ch );
    return;
  }

  if (!str_prefix(arg,"basic"))
  {
    if (get_skill(ch, gsn_basic_style) < 1)
    {
      send_to_char("You do not know that style.\n\r",ch);
      return;
    }
    else
      ch->martial_style = STYLE_BASIC;

    send_to_char( "You begin using a basic martial arts style.\n\r", ch );
    return;
  }

  if (!str_prefix(arg,"dragon"))
  {
    if (get_skill(ch, gsn_dragon_style) < 1)
    {
      send_to_char("You do not know that style.\n\r",ch);
      return;
    }
    else
      ch->martial_style = STYLE_DRAGON;

    send_to_char( "You begin using the dragon martial arts style.\n\r", ch );
    return;
  }

  if (!str_prefix(arg,"drunk"))
  {
    if (get_skill(ch, gsn_drunk_style) < 1)
    {
      send_to_char("You do not know that style.\n\r",ch);
      return;
    }
    else
      ch->martial_style = STYLE_DRUNK;

    send_to_char( "You begin using the drunken martial arts style.\n\r", ch );
    return;
  }

  if (!str_prefix(arg,"tiger"))
  {
    if (get_skill(ch, gsn_tiger_style) < 1)
    {
      send_to_char("You do not know that style.\n\r",ch);
      return;
    }
    else
      ch->martial_style = STYLE_TIGER;

    send_to_char( "You begin using the tiger martial arts style.\n\r", ch );
    return;
  }

  if (!str_prefix(arg,"snake"))
  {
    if (get_skill(ch, gsn_snake_style) < 1)
    {
      send_to_char("You do not know that style.\n\r",ch);
      return;
    }
    else
      ch->martial_style = STYLE_SNAKE;

    send_to_char( "You begin using the snake martial arts style.\n\r", ch );
    return;
  }

  if (!str_prefix(arg,"crane"))
  {
    if (get_skill(ch, gsn_crane_style) < 1)
    {
      send_to_char("You do not know that style.\n\r",ch);
      return;
    }
    else
      ch->martial_style = STYLE_CRANE;

    send_to_char( "You begin using the crane martial arts style.\n\r", ch );
    return;
  }

  if (!str_prefix(arg,"ironfist"))
  {
    if (get_skill(ch, gsn_ironfist_style) < 1)
    {
      send_to_char("You do not know that style.\n\r",ch);
      return;
    }
    else
      ch->martial_style = STYLE_IRONFIST;

    send_to_char( "You begin using the ironfist martial arts style.\n\r", ch );
    return;
  }

  if (!str_prefix(arg,"judo"))
  {
    if (get_skill(ch, gsn_judo_style) < 1)
    {
      send_to_char("You do not know that style.\n\r",ch);
      return;
    }
    else
      ch->martial_style = STYLE_JUDO;

    send_to_char( "You begin using the judo martial arts style.\n\r", ch );
    return;
  }

  send_to_char( "Syntax: style <style>\n\r", ch );
  return;
}

void do_flare( CHAR_DATA *ch, char *argument )
{ // const.c: all -1/-1
  if (get_skill(ch,gsn_flare) < 1)
  {
    send_to_char("Do what now?\n\r",ch);
    return;
  }

/*
  if ( IS_SET( ch->spell_aff, SAFF_FLAME_SHROUD ) )
  else if ( IS_SET( ch->spell_aff, SAFF_ICE_SHROUD ) )
  else if ( IS_SET( ch->spell_aff, SAFF_ELECTRIC_SHROUD ) )
  else if ( IS_SET( ch->spell_aff, SAFF_POISON_SHROUD ) )
  else if ( IS_SET( ch->spell_aff, SAFF_LIFE_DRAIN ) )
  else if ( IS_SET( ch->spell_aff, SAFF_MANA_DRAIN ) )
*/
  return;
}
