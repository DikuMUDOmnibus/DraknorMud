/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/

/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN(spec_breath_any);
DECLARE_SPEC_FUN(spec_breath_acid);
DECLARE_SPEC_FUN(spec_breath_fire);
DECLARE_SPEC_FUN(spec_breath_frost);
DECLARE_SPEC_FUN(spec_breath_gas);
DECLARE_SPEC_FUN(spec_breath_lightning);
DECLARE_SPEC_FUN(spec_cast_adept);
DECLARE_SPEC_FUN(spec_cast_cleric);
DECLARE_SPEC_FUN(spec_cast_judge);
DECLARE_SPEC_FUN(spec_cast_mage);
DECLARE_SPEC_FUN(spec_cast_battlemage    );
DECLARE_SPEC_FUN(spec_cast_undead);
DECLARE_SPEC_FUN(spec_executioner);
DECLARE_SPEC_FUN(spec_fido);
DECLARE_SPEC_FUN(spec_guard);
DECLARE_SPEC_FUN(spec_shadowguard);
DECLARE_SPEC_FUN(spec_janitor);
DECLARE_SPEC_FUN(spec_mayor);
DECLARE_SPEC_FUN(spec_poison);
DECLARE_SPEC_FUN(spec_thief);
DECLARE_SPEC_FUN(spec_nasty);
DECLARE_SPEC_FUN(spec_troll_member);
DECLARE_SPEC_FUN(spec_ogre_member);
DECLARE_SPEC_FUN(spec_questmaster);
DECLARE_SPEC_FUN(spec_patrolman);
DECLARE_SPEC_FUN(spec_tracker);
DECLARE_SPEC_FUN(spec_assassin);
DECLARE_SPEC_FUN(spec_taxidermist);
DECLARE_SPEC_FUN(spec_snake_charmer);
DECLARE_SPEC_FUN(spec_dreddguard);
DECLARE_SPEC_FUN(spec_terrorist);
DECLARE_SPEC_FUN(spec_cast_druid);  // Added Feb, 2004 RWLIII

void spec_changer( CHAR_DATA *ch, CHAR_DATA *victim, int gold, int silver );
void spec_locker ( CHAR_DATA *ch, CHAR_DATA *victim, int gold, int silver );
void spec_path   ( CHAR_DATA *ch );
