/* Draknor custom class Woodsman */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "magic.h"
#include "recycle.h"
#include "interp.h"

void do_chameleon( CHAR_DATA *ch, char *argument )
{
  send_to_char( "You attempt to blend in with your surroundings.\n\r", ch );

  if (IS_GHOST(ch)) {
    send_to_char("You can't do that while {rDEAD{x.\n\r",ch);
    return;
  }

  if ( IS_AFFECTED(ch, AFF_HIDE) )
    REMOVE_BIT(ch->affected_by, AFF_HIDE);

  if ( number_percent( ) < get_skill(ch,gsn_chameleon))
    {
      SET_BIT(ch->affected_by, AFF_HIDE);
      send_to_char("{DYour skin color begins to change.{x\n\r",ch);
      check_improve(ch,gsn_chameleon,TRUE,3);
      WAIT_STATE(ch,8);
    }
  else
    check_improve(ch,gsn_chameleon,FALSE,3);

  return;
}

