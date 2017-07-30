/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,     *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                     *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael     *
 *  Chastain, Michael Quan, and Mitchell Tse.           *
 *                     *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc     *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.               *
 *                     *
 *  Much time and thought has gone into this software and you are     *
 *  benefitting.  We hope that you share your changes too.  What goes     *
 *  around, comes around.               *
 ***************************************************************************/

/***************************************************************************
*  ROM 2.4 is copyright 1993-1996 Russ Taylor         *
*  ROM has been brought to you by the ROM consortium       *
*      Russ Taylor (rtaylor@efn.org)           *
*      Gabrielle Taylor               *
*      Brian Moore (zump@rom.org)             *
*  By using this code, you have agreed to follow the terms of the     *
*  ROM license, in the file Rom24/doc/rom.license         *
***************************************************************************/
/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/


bool check_dispel( int dis_level, CHAR_DATA *victim, int sn);

/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(  spell_null              );
DECLARE_SPELL_FUN(  spell_acid_blast        );
DECLARE_SPELL_FUN(  spell_armor             );
DECLARE_SPELL_FUN(  spell_bless             );
DECLARE_SPELL_FUN(  spell_blindness         );
DECLARE_SPELL_FUN(  spell_bone_armor        );
DECLARE_SPELL_FUN(  spell_bone_rot          );
DECLARE_SPELL_FUN(  spell_burning_hands     );
DECLARE_SPELL_FUN(  spell_call_lightning    );
DECLARE_SPELL_FUN(  spell_calm              );
DECLARE_SPELL_FUN(  spell_cancellation      );
DECLARE_SPELL_FUN(  spell_cause_critical    );
DECLARE_SPELL_FUN(  spell_cause_light       );
DECLARE_SPELL_FUN(  spell_cause_serious     );
DECLARE_SPELL_FUN(  spell_change_sex        );
DECLARE_SPELL_FUN(  spell_chain_lightning   );
DECLARE_SPELL_FUN(  spell_charm_person      );
DECLARE_SPELL_FUN(  spell_chill_touch       );
DECLARE_SPELL_FUN(  spell_colour_spray      );
DECLARE_SPELL_FUN(  spell_continual_light   );
DECLARE_SPELL_FUN(  spell_control_weather   );
DECLARE_SPELL_FUN(  spell_create_food       );
DECLARE_SPELL_FUN(  spell_create_rose       );
DECLARE_SPELL_FUN(  spell_create_spring     );
DECLARE_SPELL_FUN(  spell_create_water      );
DECLARE_SPELL_FUN(  spell_cure_blindness    );
DECLARE_SPELL_FUN(  spell_cure_critical     );
DECLARE_SPELL_FUN(  spell_cure_disease      );
DECLARE_SPELL_FUN(  spell_cure_light        );
DECLARE_SPELL_FUN(  spell_cure_poison       );
DECLARE_SPELL_FUN(  spell_cure_serious      );
DECLARE_SPELL_FUN(  spell_cure_weaken       );
DECLARE_SPELL_FUN(  spell_curse             );
DECLARE_SPELL_FUN(  spell_demonfire         );
DECLARE_SPELL_FUN(  spell_detect_evil       );
DECLARE_SPELL_FUN(  spell_detect_good       );
DECLARE_SPELL_FUN(  spell_detect_hidden     );
DECLARE_SPELL_FUN(  spell_detect_invis      );
DECLARE_SPELL_FUN(  spell_detect_magic      );
DECLARE_SPELL_FUN(  spell_detect_poison     );
DECLARE_SPELL_FUN(  spell_dispel_evil       );
DECLARE_SPELL_FUN(  spell_dispel_good       );
DECLARE_SPELL_FUN(  spell_dispel_magic      );
DECLARE_SPELL_FUN(  spell_earthquake        );
DECLARE_SPELL_FUN(  spell_enchant_armor     );
DECLARE_SPELL_FUN(  spell_enchant_weapon    );
DECLARE_SPELL_FUN(  spell_energy_drain      );
DECLARE_SPELL_FUN(  spell_faerie_fire       );
DECLARE_SPELL_FUN(  spell_faerie_fog        );
DECLARE_SPELL_FUN(  spell_farsight          );
DECLARE_SPELL_FUN(  spell_fireball          );
DECLARE_SPELL_FUN(  spell_fireproof         );
DECLARE_SPELL_FUN(  spell_flamestrike       );
DECLARE_SPELL_FUN(  spell_floating_disc     );
DECLARE_SPELL_FUN(  spell_fly               );
DECLARE_SPELL_FUN(  spell_frenzy            );
DECLARE_SPELL_FUN(  spell_gate              );
DECLARE_SPELL_FUN(  spell_giant_strength    );
DECLARE_SPELL_FUN(  spell_harm              );
DECLARE_SPELL_FUN(  spell_haste             );
DECLARE_SPELL_FUN(  spell_heal              );
DECLARE_SPELL_FUN(  spell_heat_metal        );
DECLARE_SPELL_FUN(  spell_holy_word         );
DECLARE_SPELL_FUN(  spell_identify          );
DECLARE_SPELL_FUN(  spell_infravision       );
DECLARE_SPELL_FUN(  spell_invis             );
DECLARE_SPELL_FUN(  spell_know_alignment    );
DECLARE_SPELL_FUN(  spell_lightning_bolt    );
DECLARE_SPELL_FUN(  spell_locate_object     );
DECLARE_SPELL_FUN(  spell_magic_missile     );
DECLARE_SPELL_FUN(  spell_mass_healing      );
DECLARE_SPELL_FUN(  spell_mass_invis        );
DECLARE_SPELL_FUN(  spell_nexus             );
DECLARE_SPELL_FUN(  spell_nightmare         );
DECLARE_SPELL_FUN(  spell_pass_door         );
DECLARE_SPELL_FUN(  spell_pestilence        );
DECLARE_SPELL_FUN(  spell_plague            );
DECLARE_SPELL_FUN(  spell_poison            );
DECLARE_SPELL_FUN(  spell_poisonous_dart    );
DECLARE_SPELL_FUN(  spell_portal            );
DECLARE_SPELL_FUN(  spell_protection_evil   );
DECLARE_SPELL_FUN(  spell_protection_good   );
DECLARE_SPELL_FUN(  spell_ray_of_truth      );
DECLARE_SPELL_FUN(  spell_recharge          );
DECLARE_SPELL_FUN(  spell_refresh           );
DECLARE_SPELL_FUN(  spell_rejuvination      );
DECLARE_SPELL_FUN(  spell_remove_curse      );
DECLARE_SPELL_FUN(  spell_sanctuary         );
DECLARE_SPELL_FUN(  spell_shadow_walk       );
DECLARE_SPELL_FUN(  spell_shocking_grasp    );
DECLARE_SPELL_FUN(  spell_shield            );
DECLARE_SPELL_FUN(  spell_skeletal_spike    );
DECLARE_SPELL_FUN(  spell_sleep             );
DECLARE_SPELL_FUN(  spell_slow              );
DECLARE_SPELL_FUN(  spell_stone_skin        );
DECLARE_SPELL_FUN(  spell_summon            );
DECLARE_SPELL_FUN(  spell_teleport          );
DECLARE_SPELL_FUN(  spell_ventriloquate     );
DECLARE_SPELL_FUN(  spell_weaken            );
DECLARE_SPELL_FUN(  spell_word_of_recall    );
DECLARE_SPELL_FUN(  spell_acid_breath       );
DECLARE_SPELL_FUN(  spell_fire_breath       );
DECLARE_SPELL_FUN(  spell_frost_breath      );
DECLARE_SPELL_FUN(  spell_gas_breath        );
DECLARE_SPELL_FUN(  spell_lightning_breath  );
DECLARE_SPELL_FUN(  spell_general_purpose   );
DECLARE_SPELL_FUN(  spell_high_explosive    );
DECLARE_SPELL_FUN(  spell_drain_blade       );
DECLARE_SPELL_FUN(  spell_shocking_blade    );
DECLARE_SPELL_FUN(  spell_flame_blade       );
DECLARE_SPELL_FUN(  spell_frost_blade       );
DECLARE_SPELL_FUN(  spell_sharp_blade       );
DECLARE_SPELL_FUN(  spell_vorpal_blade      );
DECLARE_SPELL_FUN(  spell_resilience_blade  );
DECLARE_SPELL_FUN(  spell_erase             );
DECLARE_SPELL_FUN(  spell_create_fountain   );
DECLARE_SPELL_FUN(  spell_incinerate        );
DECLARE_SPELL_FUN(  spell_icicle            );
DECLARE_SPELL_FUN(  spell_bigby_bash        );
DECLARE_SPELL_FUN(  spell_flame_aura        );
DECLARE_SPELL_FUN(  spell_frost_aura        );
DECLARE_SPELL_FUN(  spell_electric_aura     );
DECLARE_SPELL_FUN(  spell_corrosive_aura    );
DECLARE_SPELL_FUN(  spell_arcane_aura       );
DECLARE_SPELL_FUN(  spell_holy_aura         );
DECLARE_SPELL_FUN(  spell_dark_aura         );
DECLARE_SPELL_FUN(  spell_deter             );
DECLARE_SPELL_FUN(  spell_aid               );
DECLARE_SPELL_FUN(  spell_metamorphose      );
DECLARE_SPELL_FUN(  spell_betray            );
DECLARE_SPELL_FUN(  spell_quench            );
DECLARE_SPELL_FUN(  spell_sate              );
DECLARE_SPELL_FUN(  spell_resurrect         );
DECLARE_SPELL_FUN(  spell_make_bag          );
DECLARE_SPELL_FUN(  spell_fear              );
DECLARE_SPELL_FUN(  spell_fumble            );
DECLARE_SPELL_FUN(  spell_martyr            );
DECLARE_SPELL_FUN(  spell_life_transfer     );
DECLARE_SPELL_FUN(  spell_moon_armor        );
DECLARE_SPELL_FUN(  spell_phoenix           );
DECLARE_SPELL_FUN(  spell_water_walk        );
DECLARE_SPELL_FUN(  spell_regeneration      );
DECLARE_SPELL_FUN(  spell_ionwave           );
DECLARE_SPELL_FUN(  spell_holy_bolt         );
DECLARE_SPELL_FUN(  spell_vaccine           );
DECLARE_SPELL_FUN(  spell_fish_breath       );
DECLARE_SPELL_FUN(  spell_mind_shield       );
DECLARE_SPELL_FUN(  spell_power_transfer    );
DECLARE_SPELL_FUN(  spell_confine           );
DECLARE_SPELL_FUN(  spell_cone_of_cold      );
DECLARE_SPELL_FUN(  spell_fire_elemental    );
DECLARE_SPELL_FUN(  spell_banshee_scream    );
DECLARE_SPELL_FUN(  spell_sunbeam           );
DECLARE_SPELL_FUN(  spell_sonic_blast       );
DECLARE_SPELL_FUN(  spell_insect_swarm      );
DECLARE_SPELL_FUN(  spell_tornado           );
DECLARE_SPELL_FUN(  spell_hammer_of_thor    );
DECLARE_SPELL_FUN(  spell_entangle          );
DECLARE_SPELL_FUN(  spell_flash             );
DECLARE_SPELL_FUN(  spell_familiarize_weapon);
DECLARE_SPELL_FUN(  spell_invulnerability   );
DECLARE_SPELL_FUN(  spell_thunder           );
DECLARE_SPELL_FUN(  spell_empower_weapon    );
DECLARE_SPELL_FUN(  spell_dislocation       );
DECLARE_SPELL_FUN(  spell_eagle_spirit      );
DECLARE_SPELL_FUN(  spell_bear_spirit       );
DECLARE_SPELL_FUN(  spell_tiger_spirit      );
DECLARE_SPELL_FUN(  spell_dragon_spirit     );
DECLARE_SPELL_FUN(  spell_scry              );
DECLARE_SPELL_FUN(  spell_cannibalism       );
DECLARE_SPELL_FUN(  spell_clairvoyance      );
DECLARE_SPELL_FUN(  spell_fatigue           );
DECLARE_SPELL_FUN(  spell_fade_out          );
DECLARE_SPELL_FUN(  spell_mass_summon       );
DECLARE_SPELL_FUN(  spell_avalanche         );
DECLARE_SPELL_FUN(  spell_replenish         );
DECLARE_SPELL_FUN(  spell_clearhead         );
DECLARE_SPELL_FUN(  spell_darksight         );
DECLARE_SPELL_FUN(  spell_mageshield        );
DECLARE_SPELL_FUN(  spell_warriorshield     );
DECLARE_SPELL_FUN(  spell_bark_skin         );
DECLARE_SPELL_FUN(  spell_thorn_blast       );
DECLARE_SPELL_FUN(  spell_hiccup            );
DECLARE_SPELL_FUN(  spell_yawn              );
DECLARE_SPELL_FUN(  spell_air_element       );
DECLARE_SPELL_FUN(  spell_fire_element      );
DECLARE_SPELL_FUN(  spell_earth_element     );
DECLARE_SPELL_FUN(  spell_water_element     );
DECLARE_SPELL_FUN(  spell_create_staff      );
DECLARE_SPELL_FUN(  spell_banish            );
DECLARE_SPELL_FUN(  spell_devour_soul       );
DECLARE_SPELL_FUN(  spell_nirvana           );
DECLARE_SPELL_FUN(  spell_aura_read         );
DECLARE_SPELL_FUN(  spell_heal_group        );
DECLARE_SPELL_FUN(  spell_poison_aura       );
DECLARE_SPELL_FUN(  spell_ice_elemental     );
DECLARE_SPELL_FUN(  spell_protection_neutral);
DECLARE_SPELL_FUN(  spell_mass_bless        );
DECLARE_SPELL_FUN(  spell_mass_blindness    );
DECLARE_SPELL_FUN(  spell_mass_weaken       );
DECLARE_SPELL_FUN(  spell_mass_curse        );
DECLARE_SPELL_FUN(  spell_mass_vaccine      );
DECLARE_SPELL_FUN(  spell_full_regen        );
DECLARE_SPELL_FUN(  spell_magma_burst       );
DECLARE_SPELL_FUN(  spell_lava_burst        );
DECLARE_SPELL_FUN(  spell_windtomb          );
DECLARE_SPELL_FUN(  spell_star_storm        );
DECLARE_SPELL_FUN(  spell_jump              );
DECLARE_SPELL_FUN(  spell_typhoon           );
DECLARE_SPELL_FUN(  spell_immolation        );
DECLARE_SPELL_FUN(  spell_petrify           );
DECLARE_SPELL_FUN(  spell_smoke_screen      );
DECLARE_SPELL_FUN(  spell_divine_intervention     );  /*Inquisitor spells*/
DECLARE_SPELL_FUN(  spell_demonic_intervention    );
DECLARE_SPELL_FUN(  spell_force_of_faith_darkflow );
DECLARE_SPELL_FUN(  spell_protection_negative     );
DECLARE_SPELL_FUN(  spell_protection_holy   );
DECLARE_SPELL_FUN(  spell_dark_fire         );
DECLARE_SPELL_FUN(  spell_noble_truth       );
DECLARE_SPELL_FUN(  spell_vile_intent       );
DECLARE_SPELL_FUN(  spell_radiance          );
DECLARE_SPELL_FUN(  spell_malevolent_shroud );
DECLARE_SPELL_FUN(  spell_turmoil           );
DECLARE_SPELL_FUN(  spell_strengthen        );
DECLARE_SPELL_FUN(  spell_nimbleness        );
DECLARE_SPELL_FUN(  spell_holy_ritual       );
DECLARE_SPELL_FUN(  spell_unholy_ritual     );
DECLARE_SPELL_FUN(  spell_soul_shroud       );
DECLARE_SPELL_FUN(  spell_spirit_shroud     );
DECLARE_SPELL_FUN(  spell_flame_shroud      );
DECLARE_SPELL_FUN(  spell_frost_shroud      );
DECLARE_SPELL_FUN(  spell_electric_shroud   );
DECLARE_SPELL_FUN(  spell_poison_shroud     );
DECLARE_SPELL_FUN(  spell_clay_golem        );
DECLARE_SPELL_FUN(  spell_spirit_leech      );
DECLARE_SPELL_FUN(  spell_ghostly_wail      );
DECLARE_SPELL_FUN(  spell_raise_skeleton    );
DECLARE_SPELL_FUN(  spell_holy_runes        ); // Alchemist spells
DECLARE_SPELL_FUN(  spell_aqua_regia        );
DECLARE_SPELL_FUN(  spell_aqua_fortis       );
DECLARE_SPELL_FUN(  spell_aqua_landhi       );
DECLARE_SPELL_FUN(  spell_aqua_albedo       );
DECLARE_SPELL_FUN(  spell_aqua_vitae        );
DECLARE_SPELL_FUN(  spell_aqua_citrinitas   );
DECLARE_SPELL_FUN(  spell_aqua_rubedo       );
DECLARE_SPELL_FUN(  spell_sulfur_blast      );
DECLARE_SPELL_FUN(  spell_adrenaline_rush   );
DECLARE_SPELL_FUN(  spell_dragon_flame      );
DECLARE_SPELL_FUN(  spell_inverted_light    );
DECLARE_SPELL_FUN(  spell_smoke_bomb        );
DECLARE_SPELL_FUN(  spell_naja_naja         );
DECLARE_SPELL_FUN(  spell_crotalus          );
DECLARE_SPELL_FUN(  spell_acidic_gas        );
DECLARE_SPELL_FUN(  spell_clarity           );
DECLARE_SPELL_FUN(  spell_alacrity          );
DECLARE_SPELL_FUN(  spell_hyracal_pressure  );
DECLARE_SPELL_FUN(  spell_prayer            );
DECLARE_SPELL_FUN(  spell_conviction        );
DECLARE_SPELL_FUN(  spell_faith             );
DECLARE_SPELL_FUN(  spell_blessed_shield    );
DECLARE_SPELL_FUN(  spell_demonic_screech   );
DECLARE_SPELL_FUN(  spell_rune_wipe         );
