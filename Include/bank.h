/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/
/*
 * Bank system money transfer option
 *
 * Defining BANK_TRANSFER allows the players to transfer money from
 * their bank account to someone else's account
 * This can be handy if players have to pay admission for their clan etc
 * The clanleader doesn't have to be near the player and he is informed of
 * all the transfers, he can't lose the transfered money either.
 *
 */

#define BANK_TRANSFER 1

/*
 * Mud Economy System
 * Bank Investments
 *
 * Defining BANK_INVEST allows players to invest their money in a
 * mud fund, the value of the fund may change from time to time
 * making it possible for players to speculate on a random generator
 * (or if you want to call it this way: the economy)
 *
 * Define SHARE_VALUE to be the start/average/default value of a share (100)
 */

#define SHARE_VALUE     100
