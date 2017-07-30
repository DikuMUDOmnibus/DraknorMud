/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/
/*
 * The Mythran Mud Economy Snippet Version 2 (used to be banking.c)
 *
 * Copyrights and rules for using the economy system:
 *
 *      The Mythran Mud Economy system was written by The Maniac, it was
 *      loosly based on the rather simple 'Ack!'s banking system'
 *
 *      If you use this code you must follow these rules.
 *              -Keep all the credits in the code.
 *              -Mail Maniac (v942346@si.hhs.nl) to say you use the code
 *              -Send a bug report, if you find 'it'
 *              -Credit me somewhere in your mud.
 *              -Follow the envy/merc/diku license
 *              -If you want to: send me some of your code
 *
 * All my snippets can be found on http://www.hhs.nl/~v942346/snippets.html
 * Check it often because it's growing rapidly  -- Maniac --
 */
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "bank.h"
#include "merc.h"
#include "interp.h"


int     share_value = SHARE_VALUE;      /* External share_value by Maniac */

void do_bank( CHAR_DATA *ch, char *argument )
{
  /* The Mythran mud economy system (bank and trading)
   *
   * based on:
   * Simple banking system. by -- Stephen --
   *
   * The following changes and additions where
   * made by the Maniac from Mythran Mud
   * (v942346@si.hhs.nl)
   *
   * History:
   * 18/05/96:     Added the transfer option, enables chars to transfer
   *               money from their account to other players' accounts
   * 18/05/96:     Big bug detected, can deposit/withdraw/transfer
   *               negative amounts (nice way to steal is
   *               bank transfer -(lots of dogh)
   *               Fixed it (thought this was better... -= Maniac =-)
   * 21/06/96:     Fixed a bug in transfer (transfer to MOBS)
   *               Moved balance from ch->balance to ch->pcdata->balance
   * 21/06/96:     Started on the invest option, so players can invest
   *               money in shares, using buy, sell and check
   *               Finished version 1.0 releasing it monday 24/06/96
   * 24/06/96:     Mythran Mud Economy System V1.0 released by Maniac
   *
   */

//  CHAR_DATA *mob;
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  if ( IS_NPC( ch ) )
    {
      send_to_char( "Banking Services are only available to players!\n\r", ch );
      return;
    }

  /* Check for mob with act->banker */
/* bank is imm-only now
  for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
  {
      if ( IS_NPC(mob) && IS_SET(mob->act, ACT_BANKER ) )
	    break;
  }

  if ( mob == NULL )
  {
      send_to_char( "You can't do that here.\n\r", ch );
      return;
  }
*/

  /*    if ((time_info.hour < 9) || (time_info.hour > 17))
	    {
	    send_to_char( "The bank is closed, it is open from 9 to 5.\n\r", ch);
	    return;
	    }*/

  if ( argument[0] == '\0' )
    {
      send_to_char( "Bank Options:\n\r\n\r", ch );
      send_to_char( "Bank balance: Displays your balance.\n\r", ch );
      send_to_char("    - balance\n\r",ch);
      send_to_char( "Bank deposit : Deposit {yg{Yo{yld{x into your account.\n\r", ch );
      send_to_char("    - deposit Amount\n\r",ch);
      send_to_char( "Bank withdraw : Withdraw {yg{Yo{yld{x from your account.\n\r", ch );
      send_to_char("    - withdraw Amount\n\r",ch);
#if BANK_TRANSFER
      send_to_char( "Bank transfer  : Transfer {yg{Yo{yld{x to account.\n\r", ch);
      send_to_char("    - transfer Amount Character\n\r",ch);
#endif
#if BANK_INVEST
      send_to_char( "Bank buy #: Buy # shares (in developement)\n\r", ch);
      send_to_char( "Bank sell #: Sell # shares (in developement)\n\r", ch);
      send_to_char( "Bank check: Check the current rates of the shares. (in developement)\n\r", ch);
#endif
      return;
    }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  /* Now work out what to do... */
  if ( !str_prefix( arg1, "balance" ) )
    {
      mprintf(sizeof(buf), buf,"Your current balance is: {Y%d {yg{Yo{yld{x.",ch->pcdata->balance );
//      do_function(mob, &do_say, buf);
      return;
    }

  if ( !str_prefix( arg1, "deposit" ) )
    {
      int amount;

      if ( is_number ( arg2 ) )
	{
	  amount = atoi( arg2 );
	  if (amount > ch->gold )
	    {
	      mprintf( sizeof(buf), buf, "How can you deposit %d gold when you only have %d?", amount, (int) ch->gold );
//	      do_function(mob, &do_say, buf );
	      return;
	    }

	  if (amount < 0 )
	    {
//	      do_function(mob, &do_say, "Only positive amounts allowed...");
	      return;
	    }

	  ch->gold -= amount;
	  ch->pcdata->balance += amount;
	  printf_to_char(ch, "You deposit {Y%d {yg{Yo{yld{x.  Your new balance is {Y%d {yg{Yo{yld{x.\n\r",
			 amount, ch->pcdata->balance );
	  return;
	}
    }

  /* We only allow transfers if this is true... so define it... */

#if BANK_TRANSFER
  if ( !str_prefix( arg1, "transfer" ) )
    {
      int amount;
      CHAR_DATA *victim;

      if ( is_number ( arg2 ) )
	{
	  amount = atoi( arg2 );
	  if ( amount > ch->pcdata->balance )
	    {
	      mprintf( sizeof(buf), buf, "How can you transfer %d gold when your balance is %d?    ",
		       amount, ch->pcdata->balance );
//	      do_function(mob, &do_say, buf);
	      return;
	    }

	  if (amount <= 0 )
	    {
//	      do_function(mob, &do_say, "Only positive amounts allowed...");
	      return;
	    }

	  if ( !( victim = get_char_world( ch, argument ) ) )
	    {
	      mprintf (sizeof(buf), buf, "%s doesn't have a bank account.", argument );
//	      do_function(mob, &do_say, buf );
	      return;
	    }

	  if (IS_NPC(victim))
	    {
//	      do_function(mob, &do_say, "You can only transfer money to players.");
	      return;
	    }

	  ch->pcdata->balance     -= amount;
	  victim->pcdata->balance += amount;
	  printf_to_char(ch, "You transfer {Y%d {yg{Yo{yld{x. Your new balance is {Y%d {yg{Yo{yld{x.\n\r",
			 amount, ch->pcdata->balance );
	  printf_to_char(victim, "{W[{BBANK{W]{x %s has transferred {Y%d {yg{Yo{yld{x coins to your account.\n\r",
			 ch->name, amount);
	  return;
	}
    }
#endif

  if ( !str_prefix( arg1, "withdraw" ) )
    {
      int amount;

      if ( is_number ( arg2 ) )
	{
	  amount = atoi( arg2 );
	  if ( amount > ch->pcdata->balance )
	    {
	      mprintf( sizeof(buf), buf, "How can you withdraw %d gold when your balance is %d?",
		       amount, ch->pcdata->balance );
//	      do_function(mob, &do_say, buf );
	      return;
	    }

	  if (amount < 0 )
	    {
//	      do_function(mob, &do_say, "Only positive amounts allowed...");
	      return;
	    }

	  ch->pcdata->balance -= amount;
	  ch->gold += amount;
	  printf_to_char(ch, "You withdraw {Y%d {yg{Yo{yld{x.  Your new balance is {Y%d {yg{Yo{yld{x.\n\r", amount, ch->pcdata->balance );
	  return;
	}
    }

  /* If you want to have an invest option... define BANK_INVEST */

#if BANK_INVEST
  if ( !str_prefix( arg1, "buy" ) )
    {
      int amount;

      if ( is_number ( arg2 ) )
	{
	  amount = atoi( arg2 );
	  if ( (amount * share_value) > ch->pcdata->balance )
	    {
	      mprintf(sizeof(buf), buf, "%d shares will cost you %d, get more money.", amount, (amount * share_value) );
//	      do_function(mob, &do_say, buf);
	      return;
	    }

	  if (amount < 0 )
	    {
//	      do_function(mob, &do_say, "If you want to sell shares you have to say so...");
	      return;
	    }

	  ch->pcdata->balance -= (amount * share_value);
	  ch->pcdata->shares  += amount;
	  mprintf(sizeof(buf), buf, "You buy %d shares for %d GP, you now have %d shares.", amount, (amount * share_value), ch->pcdata->shares );
//	  do_function(mob, &do_say, buf);
	  return;
	}
    }

  if ( !str_prefix( arg1, "sell" ) )
    {
      int amount;

      if ( is_number ( arg2 ) )
	{
	  amount = atoi( arg2 );
	  if ( amount > ch->pcdata->shares )
	    {
	      mprintf(sizeof(buf), buf, "You only have %d shares.", ch->pcdata->shares );
//	      do_function(mob, &do_say, buf);
	      return;
	    }

	  if (amount < 0 )
	    {
//	      do_function(mob, &do_say, "If you want to buy shares you have to say so...");
	      return;
	    }

	  ch->pcdata->balance += (amount * share_value);
	  ch->pcdata->shares  -= amount;
	  mprintf(sizeof(buf), buf, "You sell %d shares for %d GP, you now have %d shares.", amount, (amount * share_value), ch->pcdata->shares );
//	  do_function(mob, &do_say, buf);
	  return;
	}
    }

  if ( !str_prefix( arg1, "check" ) )
    {
      mprintf (sizeof(buf), buf, "The current shareprice is %d.",share_value);
//      do_say(mob, buf);
      if (ch->pcdata->shares)
	{
	  mprintf (sizeof(buf), buf, "You currently have %d shares, (%d a share) worth totally %d gold.",
		   ch->pcdata->shares, share_value, (ch->pcdata->shares * share_value) );
//	  do_function(mob, &do_say, buf);
	}
      return;
    }
#endif

//  do_function(mob, &do_say, "I don't know what you mean");
//  do_function(mob, &do_bank, "" );              /* Generate Instructions */
  return;
}

void do_deposit( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA   *vch;
    int         amount;
    char        arg1[MIL];
    char        buf[MSL];

    argument = one_argument( argument, arg1 );

    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
        if ( IS_NPC( vch )
        &&   IS_SET( vch->act, ACT_BANKER ) )
            break;
    }

    if ( !vch )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    if ( IS_NULLSTR( arg1 ) )
    {
        send_to_char( "Syntax: deposit <amount>\n\r", ch );
        return;
    }

    if ( !is_number(arg1)
    &&   !str_prefix(arg1, "all") )
    {
      amount = ch->gold;
    }
    else
    {
      if ( !is_number( arg1 ) )
      {
          send_to_char( "Syntax: deposit <amount>\n\r", ch );
          return;
      }
      amount = atoi( arg1 );
    }

    if ( amount <= 0 )
    {
        send_to_char( "We're sorry, but you can only deposit positive amounts.\n\r", ch );
        return;
    }

    if ( ( ch->gold - amount ) < 0 )
    {
        send_to_char( "You don't have that much.\n\r", ch );
        return;
    }

    ch->gold -= amount;
    ch->pcdata->balance += amount;
    act( "$n makes a deposit.", ch, NULL, NULL, TO_ROOM );
    printf_to_char( ch,
        "You deposit %d {yg{Yo{yld{x coins.\n\rYour balance is now %d.\n\r",
            amount, ch->pcdata->balance );

    sprintf( buf, "{W%s{x deposited %d {yg{Yo{yld{x ({g%d{x).",
      ch->name, amount, ch->pcdata->balance );
    wiznet( buf, NULL, NULL, WIZ_BANK, 0, get_trust(ch) );

}

void do_withdraw( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA   *vch;
    int         amount;
    char        buf[MSL];
    //char        arg[MIL];

    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
        if ( IS_NPC( vch )
        &&   IS_SET( vch->act, ACT_BANKER ) )
            break;
    }

    if ( !vch )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    if ( IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: withdraw <amount>\n\r", ch );
        return;
    }

    if ( is_number( argument ) )
      amount = atoi( argument );
    else
    {
      if (!str_prefix(argument, "all") )
        amount = ch->pcdata->balance;
      else
      {
        send_to_char( "Syntax: withdraw <amount>\n\r", ch );
        return;
      }
    }

    if ( amount <= 0 )
    {
        send_to_char( "We're sorry, but you can only withdraw positive amounts.\n\r", ch );
        return;
    }

    if ( ( ch->pcdata->balance - amount ) < 0 )
    {
        send_to_char( "You don't have that much in your account.\n\r", ch );
        return;
    }

    ch->gold += amount;
    ch->pcdata->balance -= amount;
    act( "$n makes a withdrawal.", ch, NULL, NULL, TO_ROOM );
    printf_to_char( ch,
        "You withdraw %d {yg{Yo{yld{x coins.\n\rYour balance is now %d.\n\r",
            amount, ch->pcdata->balance );

    sprintf( buf, "{W%s{x withdrew %d {yg{Yo{yld{x ({r%d{x).",
      ch->name, amount, ch->pcdata->balance );
    wiznet( buf, NULL, NULL, WIZ_BANK, 0, get_trust(ch) );

}

void do_bank_transfer( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA   *mob;
    CHAR_DATA   *vch;
    int         amount;
    char        arg[MIL];
    char        buf[MSL];

    argument = one_argument( argument, arg );

    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC( mob )
        &&   IS_SET( mob->act, ACT_BANKER ) )
            break;
    }

    if ( !mob )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    if ( IS_NULLSTR( arg )
    ||   IS_NULLSTR( argument ) )
    {
        send_to_char( "Syntax: transfer <amount> <name>\n\r", ch );
        return;
    }

    if ( !is_number( arg ) )
    {
        send_to_char( "Syntax: transfer <amount> <name>\n\r", ch );
        return;
    }

    amount = atoi( arg );

    if ( amount <= 0 )
    {
        send_to_char( "We're sorry, but you can only transfer positive amounts.\n\r", ch );
        return;
    }

    if ( ( vch = get_char_world( ch, argument ) ) == NULL )
    {
        send_to_char( "Sorry, they are not online.\n\r", ch );
        return;
    }

    if ( IS_NPC( vch ) ) return;

    ch->pcdata->balance -= amount;
    vch->pcdata->balance += amount;
    printf_to_char( ch,
        "You transfer %d {yg{Yo{yld{x coins to %s.\n\rYour balance is now %d.\n\r",
            amount, vch->name, ch->pcdata->balance );
    printf_to_char( vch,
        "%s has transferred %d {yg{Yo{yld{x coins to your account.\n\rYour balance is now %d.\n\r",
            ch->name, amount, vch->pcdata->balance );
    sprintf( buf, "{W%s{x transferred %d {yg{Yo{yld{x to %s ({r%d{x) ({g%d{x)",
      ch->name, amount, vch->name,
      ch->pcdata->balance, vch->pcdata->balance );
    wiznet( buf, NULL, NULL, WIZ_BANK, 0, get_trust(ch) );

}

