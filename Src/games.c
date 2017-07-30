/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
 **************************************************************************
 *      Game Code v2 for ROM based muds. Robert Schultz, Sembiance        *
\**************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "recycle.h"
#include "interp.h"
#include "games.h"

const struct game_type game_table[] =
{
  /* NAME       FUNCTION     */
  { "slots",    do_slots
  },
  { "dice",     do_high_dice },
  { "checkers", do_checkers  },
  {  NULL,      NULL         }
};

void do_game( CHAR_DATA *ch, char *argument )
{
  char                arg[MAX_INPUT_LENGTH];
  int i;

  argument = one_argument(argument, arg);

  if ( arg[0] == '\0' )
  {
    send_to_char("Type 'help games' for more information on games.\n\r", ch);
    return;
  }

  if (IS_NPC(ch))
  {
    send_to_char("Sorry, only player characters may play games.\n\r", ch);
    return;
  }

  for ( i=0;game_table[i].name != NULL;i++ )
  {
    if ( !str_cmp( game_table[i].name, arg ) )
    {
      do_function(ch,game_table[i].function, argument );
      return;
    }
  }

  send_to_char( "Games currently in operation: \n\r", ch );

  for ( i=0;game_table[i].name != NULL;i++ )
  {
    send_to_char( "    ", ch );
    send_to_char( game_table[i].name, ch );
    send_to_char( "\n\r", ch );
  }

  return;
}

void do_slots(CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *slotMachine;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int counter, winArray[11];
  int cost, jackpot, bars, winnings, numberMatched;
  int bar1, bar2, bar3, bar4, bar5;
  bool partial, won, wonJackpot, frozen, foundSlot;

  char *bar_messages[] =
  {
    "<------------>",
    "<{YGold Coin{x>",               /* 1 */
    "<{RLock Pick{x>",
    "<{MSembiance{x>",               /* 3 */
    "<{cCityguard{x>",
    "<{CElf Sword{x>",               /* 5 */
    "<{yAn Orange{x>",
    "<{rFly Spell{x>",
    "<{GElemental{x>",
    "<{WDualWield{x>",
    "<{BMudSchool{x>",              /* 10 */
  };

  argument = one_argument(argument, arg);

  if (arg[0] == '\0')
  {
    send_to_char("Syntax: game slots <which machine>\n\r", ch);
    return;
  }

  foundSlot = FALSE;

  for (slotMachine = ch->in_room->contents; slotMachine != NULL; slotMachine = slotMachine->next_content)
  {
    if ( (slotMachine->item_type == ITEM_SLOT_MACHINE) && (can_see_obj(ch,slotMachine)))
    {
      if (is_name(arg, slotMachine->name))
      {
        foundSlot = TRUE;
        break;
      }
      else
      {
        foundSlot = FALSE;
      }
    }
  }

  if (foundSlot == FALSE)
  {
    send_to_char("That slot machine is not here.\n\r", ch);
    return;
  }

  cost = slotMachine->value[0];
  if (cost <= 0)
  {
    send_to_char("This slot machine seems to be broken.\n\r", ch);
    return;
  }

  if (cost > ch->gold)
  {
    mprintf(sizeof(buf), buf, "This slot machine costs {Y%d {yg{Yo{yld{x to play.\n\r", cost);
    send_to_char(buf, ch);
    return;
  }

  ch->gold -= cost;

  jackpot = slotMachine->value[1];
  bars = slotMachine->value[2];

  if (slotMachine->value[3] == 1)
    partial = TRUE;
  else
    partial = FALSE;

  if (slotMachine->value[4] == 1)
    frozen = TRUE;
  else
    frozen = FALSE;

  bar1 = number_range( 1, 10 );
  bar2 = number_range( 1, 10 );
  bar3 = number_range( 1, 10 );
  if (bars > 3)
  {
    bar4 = number_range( 1, 10 );
    bar5 = number_range( 1, 10 );
  }
  else
  {
    bar4 = 0;
    bar5 = 0;
  }

  if (bars == 3)
  {
    send_to_char("{g////---------------{MSlot Machine{g---------------\\\\\\\\{x\n\r", ch);
    mprintf(sizeof(buf), buf, "{g|{C{{}{g|{x  %s  %s  %s  {h|{C{{}{g|{x\n\r", bar_messages[bar1],
            bar_messages[bar2], bar_messages[bar3]);
    send_to_char(buf, ch);
    send_to_char("{g\\\\\\\\------------------------------------------////{x\n\r", ch);
  }
  else
  {
    send_to_char("{g////----------------------------{MSlot Machine{g---------------------------\\\\\\\\{x\n\r", ch);
    mprintf(sizeof(buf), buf, "{g|{C{{}{g|{x  %s  %s  %s  %s  %s  {g|{C{{}{g|{x\n\r", bar_messages[bar1],
            bar_messages[bar2], bar_messages[bar3], bar_messages[bar4], bar_messages[bar5]);
    send_to_char(buf, ch);
    send_to_char("{g\\\\\\\\-------------------------------------------------------------------////{x\n\r",
                 ch);
  }

  wonJackpot = FALSE;
  winnings = 0;
  won = FALSE;
  numberMatched = 0;

  if (bars == 3)
  {
    if ( (bar1 == bar2) && (bar2 == bar3) )
    {
      winnings = jackpot;  /* they won the jackpot, make it */
      won = TRUE;          /* worth their while!            */
      slotMachine->value[1] = cost*10;   /* put it back to something */
      wonJackpot = TRUE;
    }
    else
    {
      if (!frozen)
        slotMachine->value[1] += cost;
    }
  }
  else if (bars == 5)
  {
    if ( (bar1 == bar2) && (bar2 == bar3) && (bar3 == bar4) && (bar4 == bar5) )
    {
      winnings = jackpot;  /* if no partial, better have a  */
      won = TRUE;          /* kick butt jackpot for them    */
      slotMachine->value[1] = cost*25;
      wonJackpot = TRUE;
    }
    else
    {
      if (!frozen)
        slotMachine->value[1] += cost;
    }
  }
  else
  {
    send_to_char("This is a bad slot machine. Contact casino administration.\n\r", ch);
    return;
  }
  if (!frozen)
    if (slotMachine->value[1] >= 32000)
      slotMachine->value[1] = 31000;

  // Taeloch 3/30/17: counter iterated from 0 to 12, but max index is 11 "was <="
  for (counter = 0; counter < 12; counter++)
  {
    winArray[counter] = 0;
  }


  if (!won && partial)
  {
    if (bars == 3)
    {
      if (bar1 == bar2)
      {
        winnings += cost/2;
        won = TRUE;
        numberMatched++;
      }
      if (bar1 == bar3)
      {
        numberMatched++;
        if (won)
          winnings += cost;
        else
        {
          winnings += cost/2;
          won = TRUE;
        }
      }
      if (bar2 == bar3)
      {
        numberMatched++;
        if (won)
          winnings += cost;
        else
        {
          winnings += cost/2;
          won = TRUE;
        }
      }
      if (!frozen)
      {
        if (!won)
          slotMachine->value[1] += (int)cost/3;
        else
          slotMachine->value[1] -= winnings;
      }
    }
    if ( bars == 5)
    {
      winArray[bar1]++;
      winArray[bar2]++;
      winArray[bar3]++;
      winArray[bar4]++;
      winArray[bar5]++;

      // Taeloch 3/30/17: counter iterated from 0 to 12, but max index is 11 "was <="
      for (counter = 0; counter < 12; counter++)
      {
        if (winArray[counter] > 1)
          numberMatched += winArray[counter];
      }

      if (numberMatched == 5)
      {
        if (!frozen)
          slotMachine->value[1] -= (cost*7)/2;
        winnings += cost*7;
      }
      if (numberMatched == 4)
      {
        if (!frozen)
          slotMachine->value[1] -= (cost*5)/2;
        winnings += cost*5;
      }
      if (numberMatched == 3)
      {
        winnings += cost/2;
        if (!frozen)
          slotMachine->value[1] += cost/2;
      }
      if (numberMatched == 2)
      {
        if (!frozen)
          slotMachine->value[1] += cost-1;
        winnings = 1;
      }
      if (numberMatched == 0)
      {
        winnings = 0;
        if (!frozen)
          slotMachine->value[1] += cost;
      }
      if (winnings > 0)
        won = TRUE;
    }
  }

  ch->gold += winnings;

  if (won && wonJackpot)
  {
    mprintf(sizeof(buf), buf, "You won the jackpot worth {Y%d {yg{Yo{yld{x!! The jackpot now stands at {Y%d {yg{Yo{yld{z.\n\r",
            winnings, slotMachine->value[1]);
    send_to_char(buf, ch);
    mprintf(sizeof(buf), buf, "emote has won a Jackpot!!{g.");
    do_function(ch,&do_info, buf);
  }
  if (won && !wonJackpot)
  {
    mprintf(sizeof(buf), buf, "You matched %d bars and won {Y%d {yg{Yo{yld{x! The jackpot is now worth {Y%d {yg{Yo{yld{x.\n\r",
            numberMatched, winnings, slotMachine->value[1]);
    send_to_char(buf, ch);
  }
  if (!won)
  {
    mprintf(sizeof(buf), buf, "Sorry you didn't win anything. The jackpot is now worth {Y%d {yg{Yo{yld{x.\n\r",
            slotMachine->value[1]);
    send_to_char(buf, ch);
  }

  if (slotMachine->value[1] >= 32000)
    slotMachine->value[1] = 31000;

  return;
}

void do_high_dice( CHAR_DATA *ch, char *argument)
{
  char                buf[MAX_STRING_LENGTH];
  char                arg[MAX_INPUT_LENGTH];
  CHAR_DATA           *dealer;
  int die, dealerDice, playerDice;
  int bet;

  argument = one_argument(argument, arg);

  if (arg[0] == '\0' || !is_number(arg))
  {
    send_to_char("Syntax is: game dice <bet>\n\r", ch);
    return;
  }

  bet = atoi(arg);

  if (bet < 10)
  {
    send_to_char("Minimum bet is {Y10 {yg{Yo{yld{x coins.\n\r", ch);
    return;
  }

  if (bet > 1000)
  {
    send_to_char("Maximum bet is{Y 1,000 {yg{Yo{yld{x coins.\n\r", ch);
    return;
  }

  for ( dealer = ch->in_room->people; dealer; dealer = dealer->next_in_room )
  {
    if ( IS_NPC(dealer) && IS_SET(dealer->act, ACT_DEALER) && can_see(ch,dealer))
      break;
  }

  if ( dealer == NULL )
  {
    send_to_char( "You do not see any dice dealer here.\n\r", ch );
    return;
  }

  if (bet > ch->gold)
  {
    send_to_char("You can not afford to bet that much!\n\r", ch);
    return;
  }

  dealerDice = 0;
  playerDice = 0;


  die = number_range(1, 6);
  dealerDice += die;
  die = number_range(1, 6);
  dealerDice += die;

  die = number_range(1, 6);
  playerDice += die;
  die = number_range(1, 6);
  playerDice += die;

  mprintf(sizeof(buf), buf, "{c%s{g rolled two dice with a total of {W%d!{x\n\r", dealer->short_descr,
          dealerDice);
  send_to_char(buf, ch);
  mprintf(sizeof(buf), buf, "{gYou rolled two dice with a total of {W%d!{x\n\r", playerDice);
  send_to_char(buf, ch);

  if (dealerDice > playerDice)
  {
    mprintf(sizeof(buf), buf, "{RYou lost! {c%s{g takes your bet of {Y%d {yg{Yo{yld{g.{x\n\r",
            dealer->short_descr, bet);
    send_to_char(buf, ch);
    ch->gold -= bet;
  }

  if (dealerDice < playerDice)
  {
    mprintf(sizeof(buf), buf, "{GYou won! {c%s {ggives you your winnings of {Y%d {yg{Yo{yld{g.{x\n\r",
            dealer->short_descr, bet);
    send_to_char(buf, ch);
    ch->gold += bet;
  }

  if (dealerDice == playerDice)
  {
    mprintf(sizeof(buf), buf, "{RYou lost! {gThe dealer always wins in a tie. You lose {Y%d {yg{Yo{yld{g.{x\n\r",
            bet);
    send_to_char(buf, ch);
    ch->gold -= bet;
  }

  return;
}


void do_checkers( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  int x1, y1, x2, y2, i, j;

  if ( argument[0] == '\0' )
  {
    send_to_char( "Checkers usage:\n\r", ch );
    send_to_char( "    game checkers <xy>-<xy>\n\r", ch );
    send_to_char( "To move a piece from 5, 6 (x, y) to 7, 8 you would type:\n\r", ch );
    send_to_char( "game checkers 56-78\n\r", ch );
    return;
  }

  for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
  {
    if ( obj->item_type == ITEM_CHECKERS )
      break;
  }

  if ( obj == NULL )
  {
    send_to_char( "You'll be needing a checker board!\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );
  if ( !str_cmp( arg, "reset" ) )
  {
    obj->value[0] = A|B|C|D|E|F|G|H|I|J|K|L|U|V|W|X|Y|Z|aa|bb|cc|dd|ee|ff;
    obj->value[1] = A|B|C|D|E|F|G|H|I|J|K|L;
    obj->value[2] = 0;
    act( "$n resets the pieces on the checker board.", ch, NULL, NULL, TO_ROOM );
    act( "You reset the pieces on the checker board.", ch, NULL, NULL, TO_CHAR );
    return;
  }

  if ( !isdigit( arg[0] ) || !isdigit( arg[1] ) || !isdigit( arg[3] ) || !isdigit( arg[4] ) )
  {
    send_to_char( "Bad square coordinates.\n\r", ch );
    return;
  }

  x1 = arg[0] - '0';
  y1 = arg[1] - '0';
  x2 = arg[3] - '0';
  y2 = arg[4] - '0';

  if ( ( DIFF( x1, x2 ) > 1 ) || ( DIFF( y1, y2 ) > 1 ) )
  {
    send_to_char( "Please move one square at a time (move over to take).\n\r", ch );
    return;
  }

  i = x1 & 1;
  j = y1 & 1;
  if ( i != j )
  {
    send_to_char( "Bad square coordinates (original).\n\r", ch );
    return;
  }

  i = x2 & 1;
  j = y2 & 1;
  if ( i != j )
  {
    send_to_char( "Bad square coordinates (proposed).\n\r", ch );
    return;
  }

  i = (int) ( ( x1 - 1 ) / 2 );
  i += ( ( y1 - 1 ) * 4 );
  j = (int) ( ( x2 - 1 ) / 2 );
  j += ( ( y2 - 1 ) * 4 );

  i = 1 << i;
  j = 1 << j;

  if ( ( !IS_SET( obj->value[0], i ) ) && ( !IS_SET( obj->value[1], i ) ) && ( !IS_SET( obj->value[2], i ) ) )
  {
    send_to_char( "There's nothing there.\n\r", ch );
    return;
  }
  else if ( ( y2 == 1 ) && ( !IS_SET( obj->value[1], i ) ) && ( !IS_SET( obj->value[2], i ) ) )
  {
    SET_BIT( obj->value[1], i );
    REMOVE_BIT( obj->value[0], i );
    REMOVE_BIT( obj->value[2], i );
  }
  else if ( ( y2 == 8 ) && ( IS_SET( obj->value[1], i ) ) && ( IS_SET( obj->value[0], i ) ) )
  {
    SET_BIT( obj->value[2], i );
    REMOVE_BIT( obj->value[0], i );
    REMOVE_BIT( obj->value[1], i );
  }

  if ( IS_SET( obj->value[0], i ) )
    SET_BIT( obj->value[0], j );
  else
    REMOVE_BIT( obj->value[0], j );

  if ( IS_SET( obj->value[1], i ) )
    SET_BIT( obj->value[1], j );
  else
    REMOVE_BIT( obj->value[1], j );

  if ( IS_SET( obj->value[2], i ) )
    SET_BIT( obj->value[2], j );
  else
    REMOVE_BIT( obj->value[2], j );

  REMOVE_BIT( obj->value[0], i );
  REMOVE_BIT( obj->value[1], i );
  REMOVE_BIT( obj->value[2], i );

  look_checkers( ch, obj );
  mprintf( sizeof(buf), buf, "$n moves a piece on %s from (%d, %d) to (%d, %d).", obj->short_descr, x1, y1, x2, y2 );
  act( buf, ch, NULL, NULL, TO_ROOM );
}

void do_dice(CHAR_DATA *ch, char *argument)
{
  int num_dice, pip_count, roll;
  char buf[MSL];

  if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
  {
    send_to_char("Due to the properties of this room, your dice do not behave properly.\n\r", ch);
    return;
  }

  if (argument[0] == '\0')
  {
    num_dice = 1;
  }
  else
  {
    if (!is_number(argument))
    {
      send_to_char("The number of dice you can roll must be between 1 and 6.\n\r", ch);
      return;
    }
    num_dice = atoi(argument);
    if (num_dice < 1 || num_dice > 6)
    {
      send_to_char("The number of dice you can roll must be between 1 and 6.\n\r", ch);
      return;
    }
  }

  mprintf(sizeof(buf), buf, "$n rolls %d %s.\n\r", num_dice,
          (num_dice > 1 ? "die" : "dice"));
  act(buf, ch, NULL, NULL, TO_ROOM);
  mprintf(sizeof(buf), buf, "You roll %d %s.\n\r", num_dice,
          (num_dice > 1 ? "die" : "dice"));
  act(buf, ch, NULL, NULL, TO_CHAR);
  for (roll = 1; roll <= num_dice; roll++)
  {
    pip_count = number_range(1, 6);
    switch (pip_count)
    {
      case 1:
        act("\n\r   \n\r * \n\r   \n\r", ch, NULL, NULL, TO_ALL);
        break;
      case 2:
        act("\n\r*  \n\r   \n\r  *\n\r", ch, NULL, NULL, TO_ALL);
        break;
      case 3:
        act("\n\r*  \n\r * \n\r  *\n\r", ch, NULL, NULL, TO_ALL);
        break;
      case 4:
        act("\n\r* *\n\r   \n\r* *\n\r", ch, NULL, NULL, TO_ALL);
        break;
      case 5:
        act("\n\r* *\n\r * \n\r* *\n\r", ch, NULL, NULL, TO_ALL);
        break;
      case 6:
        act("\n\r* *\n\r* *\n\r* *\n\r", ch, NULL, NULL, TO_ALL);
        break;
      default:
        act("Die landed crooked.  Reroll.\n\r", ch, NULL, NULL, TO_ALL);
        break;
    }
    if (roll < num_dice)
      act("---", ch, NULL, NULL, TO_ALL);
  }
}

void do_newdice(CHAR_DATA *ch, char *argument)
{
  int dieroll[10];
  char dice[10][5][30]; // value, line#, line
  char buf[MSL], dicebuf[MSL];

  int num_dice = 1, size = 6, tot = 0;
  int roll, i;

  if ((argument[0] != '\0')
  && (strlen(argument) > 2))
  {
    if (is_number(argument)) // just a number, default to 6-sided
      num_dice = atoi(argument);
    else if (argument[1] == 'd')
    {
      mprintf(sizeof(buf), buf, "%c", argument[0]);
      num_dice = atoi(buf);

      mprintf(sizeof(buf), buf, "%c", argument[2]);
      size = atoi(buf);

      // Check for 2-digit dice size
      if (strlen(argument) > 3)
      {
        mprintf(sizeof(buf), buf, "%c%c", argument[2], argument[3]);
        size = atoi(buf);
      }
    }
    else
    {
      send_to_char("Usage: dice      - roll one six-sided dice\n\r", ch);
      send_to_char("       dice #    - roll 1-6 six-sided dice\n\r", ch);
      send_to_char("       dice #d#  - roll 1-6 dice of a specified size (4,6,8,10,12)\n\r", ch);
      return;
    }
  }

  if (num_dice < 1 || num_dice > 6)
  {
    send_to_char("The number of dice you can roll must be between 1 and 6.\n\r", ch);
    return;
  }

  // allowed die sizes are 4,6,8,10,12
  if (size == 4)
  {
    strcpy(dice[0][0],"{R     .     {x");
    strcpy(dice[0][1],"{R    / \\    {x");
    strcpy(dice[0][2],"{R   / {x*{R \\   {x");
    strcpy(dice[0][3],"{R  /     \\  {x");
    strcpy(dice[0][4],"{R '---{x1{R---' {x");

    strcpy(dice[1][0],"{R     .     {x");
    strcpy(dice[1][1],"{R    / \\    {x");
    strcpy(dice[1][2],"{R   / {x*{R \\   {x");
    strcpy(dice[1][3],"{R  /  {x*{R  \\  {x");
    strcpy(dice[1][4],"{R '---{x2{R---' {x");

    strcpy(dice[2][0],"{R     .     {x");
    strcpy(dice[2][1],"{R    / \\    {x");
    strcpy(dice[2][2],"{R   / {x*{R \\   {x");
    strcpy(dice[2][3],"{R  / {x* *{R \\  {x");
    strcpy(dice[2][4],"{R '---{x3{R---' {x");

    strcpy(dice[3][0],"{R     .     {x");
    strcpy(dice[3][1],"{R    / \\    {x");
    strcpy(dice[3][2],"{R   /{x* *{R\\   {x");
    strcpy(dice[3][3],"{R  / {x* *{R \\  {x");
    strcpy(dice[3][4],"{R '---{x4{R---' {x");
  }
  else if (size == 6)
  {
    strcpy(dice[0][0],"{R  -------  {x");
    strcpy(dice[0][1],"{R |       | {x");
    strcpy(dice[0][2],"{R |{x   *   {R| {x");
    strcpy(dice[0][3],"{R |       | {x");
    strcpy(dice[0][4],"{R  -------  {x");

    strcpy(dice[1][0],"{R  -------  {x");
    strcpy(dice[1][1],"{R |{x *     {R| {x");
    strcpy(dice[1][2],"{R |       | {x");
    strcpy(dice[1][3],"{R |{x     * {R| {x");
    strcpy(dice[1][4],"{R  -------  {x");

    strcpy(dice[2][0],"{R  -------  {x");
    strcpy(dice[2][1],"{R |{x *     {R| {x");
    strcpy(dice[2][2],"{R |{x   *   {R| {x");
    strcpy(dice[2][3],"{R |{x     * {R| {x");
    strcpy(dice[2][4],"{R  -------  {x");

    strcpy(dice[3][0],"{R  -------  {x");
    strcpy(dice[3][1],"{R |{x *   * {R| {x");
    strcpy(dice[3][2],"{R |       | {x");
    strcpy(dice[3][3],"{R |{x *   * {R| {x");
    strcpy(dice[3][4],"{R  -------  {x");

    strcpy(dice[4][0],"{R  -------  {x");
    strcpy(dice[4][1],"{R |{x *   * {R| {x");
    strcpy(dice[4][2],"{R |{x   *   {R| {x");
    strcpy(dice[4][3],"{R |{x *   * {R| {x");
    strcpy(dice[4][4],"{R  -------  {x");

    strcpy(dice[5][0],"{R  -------  {x");
    strcpy(dice[5][1],"{R |{x *   * {R| {x");
    strcpy(dice[5][2],"{R |{x *   * {R| {x");
    strcpy(dice[5][3],"{R |{x *   * {R| {x");
    strcpy(dice[5][4],"{R  -------  {x");
  }
  else if (size == 8)
  {
    strcpy(dice[0][0],"{R     .     {x");
    strcpy(dice[0][1],"{R    / \\    {x");
    strcpy(dice[0][2],"{R   / {x*{R \\   {x");
    strcpy(dice[0][3],"{R  /     \\  {x");
    strcpy(dice[0][4],"{R '---{x1{R---' {x");

    strcpy(dice[1][0],"{R     .     {x");
    strcpy(dice[1][1],"{R    / \\    {x");
    strcpy(dice[1][2],"{R   / {x*{R \\   {x");
    strcpy(dice[1][3],"{R  /  {x*{R  \\  {x");
    strcpy(dice[1][4],"{R '---{x2{R---' {x");

    strcpy(dice[2][0],"{R     .     {x");
    strcpy(dice[2][1],"{R    / \\    {x");
    strcpy(dice[2][2],"{R   / {x*{R \\   {x");
    strcpy(dice[2][3],"{R  / {x* *{R \\  {x");
    strcpy(dice[2][4],"{R '---{x3{R---' {x");

    strcpy(dice[3][0],"{R     .     {x");
    strcpy(dice[3][1],"{R    / \\    {x");
    strcpy(dice[3][2],"{R   /{x* *{R\\   {x");
    strcpy(dice[3][3],"{R  / {x* *{R \\  {x");
    strcpy(dice[3][4],"{R '---{x4{R---' {x");

    strcpy(dice[4][0],"{R     .     {x");
    strcpy(dice[4][1],"{R    / \\    {x");
    strcpy(dice[4][2],"{R   /{x* *{R\\   {x");
    strcpy(dice[4][3],"{R  /{x* * *{R\\  {x");
    strcpy(dice[4][4],"{R '---{x5{R---' {x");

    strcpy(dice[5][0],"{R     .     {x");
    strcpy(dice[5][1],"{R    /{x*{R\\    {x");
    strcpy(dice[5][2],"{R   /{x* *{R\\   {x");
    strcpy(dice[5][3],"{R  /{x* * *{R\\  {x");
    strcpy(dice[5][4],"{R '---{x6{R---' {x");

    strcpy(dice[6][0],"{R     .     {x");
    strcpy(dice[6][1],"{R    /{x*{R\\    {x");
    strcpy(dice[6][2],"{R   /{x***{R\\   {x");
    strcpy(dice[6][3],"{R  /{x* * *{R\\  {x");
    strcpy(dice[6][4],"{R '---{x7{R---' {x");

    strcpy(dice[7][0],"{R     .     {x");
    strcpy(dice[7][1],"{R    /{x*{R\\    {x");
    strcpy(dice[7][2],"{R   /{x***{R\\   {x");
    strcpy(dice[7][3],"{R  /{x** **{R\\  {x");
    strcpy(dice[7][4],"{R '---{x8{R---' {x");

  }
  else if (size == 10)
  {
    strcpy(dice[0][0],"{R     .     {x");
    strcpy(dice[0][1],"{R    / \\    {x");
    strcpy(dice[0][2],"{R   / {x*{R \\   {x");
    strcpy(dice[0][3],"{R  /     \\  {x");
    strcpy(dice[0][4],"{R '-._{x1{R_.-' {x");

    strcpy(dice[1][0],"{R     .     {x");
    strcpy(dice[1][1],"{R    / \\    {x");
    strcpy(dice[1][2],"{R   / {x*{R \\   {x");
    strcpy(dice[1][3],"{R  /  {x*{R  \\  {x");
    strcpy(dice[1][4],"{R '-._{x2{R_.-' {x");

    strcpy(dice[2][0],"{R     .     {x");
    strcpy(dice[2][1],"{R    / \\    {x");
    strcpy(dice[2][2],"{R   /{x * {R\\   {x");
    strcpy(dice[2][3],"{R  /{x * * {R\\  {x");
    strcpy(dice[2][4],"{R '-._{x3{R_.-' {x");

    strcpy(dice[3][0],"{R     .     {x");
    strcpy(dice[3][1],"{R    / \\    {x");
    strcpy(dice[3][2],"{R   /{x* *{R\\   {x");
    strcpy(dice[3][3],"{R  /{x * * {R\\  {x");
    strcpy(dice[3][4],"{R '-._{x4{R_.-' {x");

    strcpy(dice[4][0],"{R     .     {x");
    strcpy(dice[4][1],"{R    / \\    {x");
    strcpy(dice[4][2],"{R   /{x* *{R\\   {x");
    strcpy(dice[4][3],"{R  /{x* * *{R\\  {x");
    strcpy(dice[4][4],"{R '-._{x5{R_.-' {x");

    strcpy(dice[5][0],"{R     .     {x");
    strcpy(dice[5][1],"{R    /{x*{R\\    {x");
    strcpy(dice[5][2],"{R   /{x* *{R\\   {x");
    strcpy(dice[5][3],"{R  /{x* * *{R\\  {x");
    strcpy(dice[5][4],"{R '-._{x6{R_.-' {x");

    strcpy(dice[6][0],"{R     .     {x");
    strcpy(dice[6][1],"{R    /{x*{R\\    {x");
    strcpy(dice[6][2],"{R   /{x***{R\\   {x");
    strcpy(dice[6][3],"{R  /{x* * *{R\\  {x");
    strcpy(dice[6][4],"{R '-._{x7{R_.-' {x");

    strcpy(dice[7][0],"{R     .     {x");
    strcpy(dice[7][1],"{R    /{x*{R\\    {x");
    strcpy(dice[7][2],"{R   /{x***{R\\   {x");
    strcpy(dice[7][3],"{R  /{x** **{R\\  {x");
    strcpy(dice[7][4],"{R '-._{x8{R_.-' {x");

    strcpy(dice[8][0],"{R     .     {x");
    strcpy(dice[8][1],"{R    /{x*{R\\    {x");
    strcpy(dice[8][2],"{R   /{x***{R\\   {x");
    strcpy(dice[8][3],"{R  /{x*****{R\\  {x");
    strcpy(dice[8][4],"{R '-._{x9{R_.-' {x");

    strcpy(dice[9][0],"{R     .     {x");
    strcpy(dice[9][1],"{R    /{x*{R\\    {x");
    strcpy(dice[9][2],"{R   /{x***{R\\   {x");
    strcpy(dice[9][3],"{R  /{x** **{R\\  {x");
    strcpy(dice[9][4],"{R '-{x*{R_{xX{R_{x*{R-' {x");
  }
  else
  {
    send_to_char("The number of dice you can roll must be between 1 and 6.\n\r", ch);
    return;
  }

  for (roll = 1; roll <= num_dice; roll++)
  {
    dieroll[roll] = number_range(1, size);
    tot += dieroll[roll];
  }

  strcpy(dicebuf,""); // initialize the buffer
  for (i=0; i<5; i++)
  {
    mprintf(sizeof(buf), buf, "%s%s%s%s%s%s\n\r",
            num_dice > 0 ? dice[dieroll[1]-1][i] : "", // show one line of ALL rolled dice as it goes down the screen
            num_dice > 1 ? dice[dieroll[2]-1][i] : "", // "-1" because dieroll[] values are 1-6, but dice[] indices are 0-5
            num_dice > 2 ? dice[dieroll[3]-1][i] : "",
            num_dice > 3 ? dice[dieroll[4]-1][i] : "",
            num_dice > 4 ? dice[dieroll[5]-1][i] : "",
            num_dice > 5 ? dice[dieroll[6]-1][i] : "");
    strcat(dicebuf,buf);
  }
  act(dicebuf, ch, NULL, NULL, TO_ALL); // show all dice at once

  mprintf(sizeof(buf), buf, "$n rolled %d %d-sided %s, totalling %d.\n\r",
          num_dice, size, (num_dice > 1 ? "dice" : "die"), tot);
  act(buf, ch, NULL, NULL, TO_ROOM);

  mprintf(sizeof(buf), buf, "You rolled %d %d-sided %s, totalling %d.\n\r",
          num_dice, size, (num_dice > 1 ? "dice" : "die"), tot);
  act(buf, ch, NULL, NULL, TO_CHAR);

  return;
}

void do_tag( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *vch = NULL;
  //AREA_DATA *area = NULL;
  char arg[MIL];
  char buf[MSL];

  argument = one_argument( argument, arg );

  if ( IS_IMMORTAL( ch ) )
  {
    if ( IS_NULLSTR( arg ) )
    {
      send_to_char( "Syntax: tag room|begin|halt|reset|stop|show\n\r", ch );
      send_to_char( "        tag <char> it\n\r", ch );
      return;
    }

    if ( !str_cmp( arg, "room" ) )
    {
      int count = 0;

      if ( !ch->pcdata->tag_area )
      {
        send_to_char( "You have to begin a game first.\n\r", ch );
        return;
      }

      for ( vch = ch->in_room->people ; vch ; vch = vch->next_in_room )
      {
        if ( ch == vch )
          continue;

        if ( IS_NPC( vch ) )
          continue;

        if ( vch->pcdata->tag_area
             && ( vch->pcdata->tag_area == ch->in_room->area ) )
          continue;

        vch->pcdata->num_tags = 0;
        vch->pcdata->num_tagged = 0;
        vch->pcdata->tag_area = ch->in_room->area;
        vch->pcdata->tag_it = FALSE;
        mprintf( sizeof( buf ), buf, "I just joined the tag game." );
        do_function( vch, &do_info, buf );

        count++;
      }

      if ( count > 0 )
        printf_to_char( ch, "You have made %d people join the game.\n\r", count );
      else
        send_to_char( "No new players added.\n\r", ch );


      return;
    }

    if ( !str_cmp( arg, "begin" ) )
    {

      for ( vch = ch->in_room->people ; vch ; vch = vch->next_in_room )
      {
        if ( IS_NPC( vch ) )
          continue;

        if ( vch->pcdata->tag_area )
        {
          send_to_char( "There is already a tag game going.\n\r", ch );
          return;
        }
      }

      mprintf( sizeof( buf ), buf, "I have just started a game of tag in %s!",
               ch->in_room->area->name );
      do_function( ch, &do_info, buf );

      ch->pcdata->num_tagged = 0;
      ch->pcdata->num_tags = 0;
      ch->pcdata->tag_area = ch->in_room->area;
      ch->pcdata->tag_it = FALSE;
      return;
    }

    if ( !str_cmp( arg, "show" ) )
    {
      BUFFER *buffer;
      bool   found = FALSE;

      buffer = new_buf();

      bprintf( buffer,
               "%-12s %5s %6s %s %4s %s\n\r"
               "=====================================================\n\r",
               "Name", "Tags", "Tagged", "TAG", "Area", "Tag Zone" );

      for ( vch = player_list; vch; vch = vch->next_player )
      {
        if ( !vch->in_room || !can_see( ch, vch ) )
          continue;

        if ( !vch->pcdata->tag_area ) //if not set, not playing.
          continue;

        if ( !vch->pcdata->num_tags )  //area IS set, set the number to default.
          vch->pcdata->num_tags = 0; //just to be safe.

        if ( !vch->pcdata->tag_it )
          vch->pcdata->tag_it = FALSE; //see above - Just to be safe.

        bprintf( buffer, "%-14s %3d %6d %3s %4s %s\n\r",
                 vch->name, vch->pcdata->num_tags, vch->pcdata->num_tagged,
                 vch->pcdata->tag_it ? "{CIT!{x" : "{cTAG{x",
                 vch->pcdata->tag_area == vch->in_room->area
                 ? " IN " : " OUT",
                 vch->pcdata->tag_area == vch->in_room->area
                 ? FIX_STR( vch->pcdata->tag_area->name, "(none)", "(null)" )
                 : "-" );

        found = TRUE;
      }

      if ( !found )
        send_to_char( "No players found playing tag.\n\r", ch );
      else
        page_to_char( buf_string( buffer ), ch );
      free_buf( buffer );
      return;

    }

    if ( !str_cmp( arg, "halt" ) )
    {
      DESCRIPTOR_DATA *d;

      for (d = descriptor_list; d != NULL; d = d->next)
      {
        if ( d->connected != CON_PLAYING )
          continue;

        vch = d->character;

        if ( !vch )
          continue;

        if ( ch == vch )
          continue;

        if ( IS_NPC( vch ) )
          continue;

        if ( IS_IMMORTAL( vch ) )
          continue;

        if ( !vch->pcdata->tag_area )
          continue;

        vch->pcdata->tag_it = FALSE;
        WAIT_STATE( vch, 15 * PULSE_PER_SECOND );
      }

      mprintf( sizeof( buf ), buf, "Tag has just been halted, no more tagging is possible!" );
      do_function( ch, &do_info, buf );
      return;
    }


    if ( !str_cmp( arg, "reset" ) )
    {

      for ( vch = ch->in_room->people ; vch ; vch = vch->next_in_room )
      {

        if ( IS_NPC( vch ) )
          continue;

        vch->pcdata->tag_area = ch->in_room->area;
        vch->pcdata->tag_it = FALSE;
        vch->pcdata->num_tags = 0;
        vch->pcdata->num_tagged = 0;

      }

      mprintf( sizeof( buf ), buf, "Tag has just been reset for a new game!" );
      do_function( ch, &do_info, buf );

      return;
    }

    if ( !str_cmp( arg, "stop" ) )
    {
      DESCRIPTOR_DATA *d;

      for (d = descriptor_list; d != NULL; d = d->next)
      {
        if ( d->connected != CON_PLAYING )
          continue;

        vch = d->character;

        if ( !vch )
          continue;

        if ( IS_NPC( vch ) )
          continue;

        vch->pcdata->tag_area = NULL;
        vch->pcdata->tag_it = FALSE;
        vch->pcdata->num_tags = 0;
        vch->pcdata->num_tagged = 0;

      }

      mprintf( sizeof( buf ), buf, "Tag has just ended!" );
      do_function( ch, &do_info, buf );
      return;
    }

    if ( ( vch = get_char_room( ch, arg ) ) == NULL )
    {
      send_to_char( "Nobody here by that name.\n\r", ch );
      return;
    }

    if ( IS_NULLSTR( argument )
         ||   str_cmp( argument, "it" ) )
    {
      send_to_char( "Syntax: tag <char> it\n\r", ch );
      return;
    }

    if ( !str_cmp( argument, "it" ) )
    {
      vch->pcdata->tag_it = TRUE;
      send_to_char( "{CYou are now IT!!!{x\n\r", vch );
      mprintf( sizeof( buf ), buf, "{C%s is now it!!!{x", vch->name );
      do_function( ch, &do_yell, buf );
      WAIT_STATE( vch, 30 * PULSE_PER_SECOND );
      printf_to_char( ch, "You just made %s it!\n\r", vch->name );
      return;
    }
  }

  if ( IS_NULLSTR( arg )
       || ( IS_IMMORTAL( ch ) && !str_cmp( arg, "show" ) ) )
  {
    BUFFER *buffer;
    bool   found = FALSE;

    buffer = new_buf();

    bprintf( buffer,
             "%-12s %5s %6s %s %4s %s\n\r"
             "=====================================================\n\r",
             "Name", "Tags", "Tagged", "TAG", "Area", "Tag Zone" );

    for ( vch = player_list; vch; vch = vch->next_player )
    {
      if ( !vch->in_room || !can_see( ch, vch ) )
        continue;

      if ( IS_IMMORTAL( vch ) )
        continue;

      if ( !vch->pcdata->tag_area ) //if not set, not playing.
        continue;

      if ( !vch->pcdata->num_tags )  //area IS set, set the number to default.
        vch->pcdata->num_tags = 0; //just to be safe.

      if ( !vch->pcdata->tag_it )
        vch->pcdata->tag_it = FALSE; //see above - Just to be safe.

      bprintf( buffer, "%-14s %3d %6d %3s %4s %s\n\r",
               vch->name, vch->pcdata->num_tags, vch->pcdata->num_tagged,
               vch->pcdata->tag_it ? "{CIT!{x" : "{cTAG{x",
               vch->pcdata->tag_area == vch->in_room->area
               ? " IN " : " OUT",
               vch->pcdata->tag_area == vch->in_room->area
               ? FIX_STR( vch->pcdata->tag_area->name, "(none)", "(null)" )
               : "-" );

      found = TRUE;
    }

    if ( !found )
      send_to_char( "No players found playing tag.\n\r", ch );
    else
      page_to_char( buf_string( buffer ), ch );
    free_buf( buffer );
    return;

  }

  //No more Imms from here on out!
  if ( IS_IMMORTAL( ch ) )
  {
    send_to_char( "You can't participate in tag, sorry.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg, "quit" ) )
  {
    mprintf( sizeof( buf ), buf, "I just quit the tag game." );
    do_function( ch, &do_yell, buf );
    ch->pcdata->tag_area = NULL;
    ch->pcdata->tag_it = FALSE;
    return;
  }

  if ( ( vch = get_char_room( ch, arg ) ) == NULL )
  {
    send_to_char( "They are not here.\n\r", ch );
    return;
  }

  if ( IS_NPC( vch ) )
  {
    send_to_char( "You can only tag players!\n\r", ch );
    return;
  }

  if ( vch == ch )
  {
    send_to_char( "You can't tag yourself!!\n\r", ch );
    return;
  }

  if ( !ch->pcdata->tag_area )
  {
    send_to_char( "You cannot tag anyone, you aren't even playing!\n\r", ch );
    return;
  }

  if ( ch->pcdata->tag_it == FALSE )
  {
    send_to_char( "You can't tag them if you aren't even it!\n\r", ch );
    return;
  }

  if ( IS_IMMORTAL( vch ) )
  {
    send_to_char( "You cannot tag them.\n\r", ch );
    return;
  }

  vch->pcdata->tag_it = TRUE;
  ch->pcdata->tag_it = FALSE;
  ch->pcdata->num_tags++;
  vch->pcdata->num_tagged++;
  mprintf( sizeof( buf ), buf, "{CI just tagged %s!!!{x",
           vch->name );
  send_to_char( "You are now {CIT{x!!!\n\r", vch );
  do_function( ch, &do_yell, buf );
  WAIT_STATE( vch, 10 * PULSE_PER_SECOND );


}
