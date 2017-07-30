/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/

void do_oswitch(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if (IS_ITEM(ch) )
    {
	send_to_char("You are already switched with an object.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Switch with which object?\n\r", ch);
	return;
    }

    if ( (obj = get_obj_world (ch, argument)) == NULL)
    {
	send_to_char("No objects of that name exist.\n\r", ch);
	return;
    }
    
    if (obj->in_obj != NULL || obj->on != NULL)
    {
	send_to_char("Some mysterious force prevents you.\n\r", ch);
	return;
    }

    if (obj->carried_by != NULL
	&& obj->carried_by->pc_carried != NULL)
    {
	send_to_char("Some mysterious force prevents you.\n\r", ch);
	return;
    }

    if (obj->sw_char != NULL)
    {
	send_to_char("That object is currently occupied.\n\r", ch);
	return;
    }

    if (obj->carried_by == ch)
    {
	obj_from_char(obj);
	obj_to_room(obj, ch->in_room);
    }

    
    if (locate_obj(obj) != ch->in_room)
    {
	act("$n vanishes in a puff of smoke.", ch, NULL, NULL, TO_ROOM);
        char_to_room(ch, locate_obj(obj) );
    }

    act("You switch with $p.", ch, obj, NULL, TO_CHAR);

    obj->destroy_on_char_return = FALSE;
    obj->can_return = TRUE;

    obj_switch(ch, obj);
}


void obj_switch (CHAR_DATA *ch, OBJ_DATA *obj)
{

    if (ch->riding != NULL)
    {
	ch->riding->riden_by = NULL;
	ch->riding = NULL;
    }

    if (ch->riden_by != NULL)
    {
	ch->riden_by->riding = NULL;
	ch->riden_by = NULL;
    }

    if (obj->carried_by != NULL)
    {
	ch->carried_by = obj->carried_by;
	obj->carried_by->pc_carried = ch;
    }

    ch->position = POS_STANDING;
    ch->sw_item = obj;
    obj->sw_char = ch;
    set_pc_name(ch, obj->short_descr);
    SET_BIT(ch->flag3, AFF_IS_ITEM);
    set_form(ch, 0);
}

void obj_return (CHAR_DATA *ch, OBJ_DATA *obj)
{
    ch->sw_item = NULL;
    
    if (obj != NULL) obj->sw_char = NULL;
    REMOVE_BIT(ch->flag3, AFF_IS_ITEM);    
    set_pc_name(ch, GET_PROPER_NAME(ch) );
    if (obj != NULL && obj->destroy_on_char_return)
    	extract_obj(obj);
    set_form(ch, 0);
    if (ch->carried_by != NULL)
    {
	ch->carried_by->pc_carried = NULL;
	ch->carried_by = NULL;
    }
}

void do_saylock(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0]=='\0')
    {
	send_to_char("SayLock who?\n\r",ch);
	return; 
    }

    if ((victim=get_char_world(ch, argument))==NULL)
    {
	send_to_char("They are not playing.\n\r",ch);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("PC's only please.\n\r",ch);
	return;
    }

    if (ch->level <= victim->level)
    {
	send_to_char("You must be of a higher level to SayLock them.\n\r",ch);
	return;
    }

    if (IS_FLAG3(victim, AFF_SAY_LOCK))
    {
	send_to_char("Removing SayLock.\n\r",ch);
	send_to_char("You have been released.\n\r",victim);
	free_string(victim->prefix);
	victim->prefix = str_dup("");
	REMOVE_BIT(victim->flag3, AFF_SAY_LOCK);
	return;
    }
    else
    {
	send_to_char("Setting SayLock.\n\r",ch);
	send_to_char("You have been forced into conversation.\n\r",victim);
	free_string(victim->prefix);
	victim->prefix = str_dup("say ");
	SET_BIT(victim->flag3, AFF_SAY_LOCK);
	return;
    }
}


void do_makecool (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
	
    one_argument( argument, arg );
		
    if ( arg[0] == '\0' )
    {
	send_to_char( "Make who cool?\n\r", ch );
	return;
    }   
    
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    
    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }
    
    victim->max_hit = 30000;
    victim->max_mana = 30000;
    victim->max_move = 30000;
    victim->max_psp = 30000;
    victim->hit = 30000;
    victim->mana = 30000;
    victim->move = 30000;
    victim->qp = 30000;
    victim->faith = 30000;
    victim->value[0] = 30000;
    victim->psp = 30000;
    victim->exp = 300000;

    send_to_char("You feel like a new person.\n\r",victim);
    send_to_char("Your victim is now cool!\n\r",ch);
    act("$n shimmers with a golden aura.",victim, NULL, NULL, TO_ROOM);
    return;
}
void do_smite( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
	
    if (!IS_GRANTED(ch, CMD_CONTROL) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    one_argument( argument, arg );
		
    if ( arg[0] == '\0' )
    {
	send_to_char( "Smite whom?\n\r", ch );
	return;
    }   
    
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    
    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }
    
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }
    lose_body_part(victim, BODY_LEFT_HAND);

    victim->hit = victim->hit / 3;
    victim->mana = 0;
    victim->move = 0;

    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
  printf_to_char( victim,
    "{wYou scream in agony as a {bbo{Wl{rt{w of {wli{Wght{yn{Yi{yng{x sent from %s smites you where you stand.",
        ch->name );
  send_to_char("You smite them, sending the wrath of the gods.\n\r",ch);
  act("$n drops to $s knees, struck down by a {bbo{Wl{rt{w of {wli{Wght{yn{Yi{yng{x from an angry God.",
    victim, NULL, NULL, TO_NOTVICT);    
    save_char_obj( victim );
    }    
    return;
}

void do_tree_walk (CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *room = NULL;

    if (!IS_FLAG4(ch, AFF_TREE_MELD) )
    {
	send_to_char("You may only treewalk while melded with a tree.\n\r",ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_NO_ARBOREA) )
    {
	send_to_char("The forest will not let you do that any more.\n\r",ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_TREE_WALK) )
    {
	send_to_char("You can only do this once every 6 hours.\n\r", ch);
	return;
    }

    room = get_treemeld_room(ch);
    while (room->sector_type != SECT_FOREST)
    {
        room = get_treemeld_room(ch);
    }
    
    SET_TIMER(ch, TIMER_TREE_WALK, 6);

    ch->value[1] = room->vnum;
    send_to_char("You slide down the roots and emerge in another tree.\n\r", ch);

}

    
    

 
void do_smother (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    
    if (ch->in_room->sector_type != SECT_FOREST)
    {
	send_to_char("You may only use this ability in the forest.\n\r",ch);
	return;
    }

    if (argument == '\0')
    {
	send_to_char("Who do you wish the plants to smother?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	send_to_char("The forest tells you 'Who do you wish me to smother?'\n\r", ch);
	return;
    }

    if (IS_SET(victim->flag4, AFF_SMOTHERED)
	|| IS_SET(victim->flag4, AFF_ENTWINED) )
    {
	send_to_char("The forest tells you 'I can't do any more than that!'\n\r", ch);
	return;
    }

    if (victim->fighting != NULL)
    {
	send_to_char("The forest tells you 'Wait till they stop fighting.'\n\r", ch);
	return;
    }

    if (victim->level > ch->level)
    {
	send_to_char("The forest tells you 'They are too powerful to affect.'\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);

    if (victim->power[DISC_VAMP_CELE] > 3)
    {
	send_to_char("The forest tells you 'They are moving too quickly!'\n\r", ch);
	send_to_char("Vines and creepers move towards you, but you dodge out of their way.\n\r",victim);
 	return;
    }

    SET_BIT(victim->flag4, AFF_SMOTHERED);
    send_to_char("The plants and vegetation start to smother you.\n\r", victim);
    act("$n whispers something, and the plants start to smother and choke $N.", ch, NULL, victim,TO_NOTVICT);
    act("Plants and vines start to smother and choke $N.", ch, NULL, victim, TO_CHAR);
}

void do_entwine (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    
    if (ch->in_room->sector_type != SECT_FOREST)
    {
	send_to_char("You may only use this ability in the forest.\n\r",ch);
	return;
    }

    if (argument == '\0')
    {
	send_to_char("Who do you wish the plants to entwine?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	send_to_char("The forest tells you 'Who do you wish me to entwine?'\n\r", ch);
	return;
    }

    if (IS_SET(victim->flag4, AFF_SMOTHERED)
	|| IS_SET(victim->flag4, AFF_ENTWINED) )
    {
	send_to_char("The forest tells you 'I can't do any more than that!'\n\r", ch);
	return;
    }

    if (victim->fighting != NULL)
    {
	send_to_char("The forest tells you 'Wait till they stop fighting.'\n\r", ch);
	return;
    }

    if (victim->level > ch->level)
    {
	send_to_char("The forest tells you 'They are too powerful to affect.'\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);

    if (victim->power[DISC_VAMP_CELE] > 3)
    {
	send_to_char("The forest tells you 'They are moving too quickly!'\n\r", ch);
	send_to_char("Vines and creepers move towards you, but you dodge out of their way.\n\r",victim);
 	return;
    }

    SET_BIT(victim->flag4, AFF_ENTWINED);
    send_to_char("The plants and vegetation start to entwine you.  You cannot move.\n\r", victim);
    act("$n whispers something, and the plants start to entwine $N.", ch, NULL, victim,TO_NOTVICT);
    act("Plants and vines start to hold and entwine $N.", ch, NULL, victim, TO_CHAR);
}

void do_wrench (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA * victim;

    if (ch->power[DISC_HERO_EXOR] < 3)
    {
	send_to_char("You must be at least level 3 in Exorcism to attempt this ability.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_WRENCH))
    {
	send_to_char("You may only use this power once per day.\n\r", ch);
    	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Who do you wish to wrench out of the negative plane?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, argument) )== NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    SET_TIMER(ch, TIMER_WRENCH, 24);

    send_to_char("You start to perform the ritual.\n\r", ch);
    WAIT_STATE(ch, 12);

    act("$n starts to chant in strange ancient languages.", ch, NULL, NULL, TO_ROOM);
    
    if (victim->secondaryrace != RACE_VAMPIRE)
	send_to_char("Nothing seems to have happened.\n\r", ch);
    else
    {
        if (victim->level > ch->level + ch->generation)
	{
	    send_to_char("They are too powerful for you to affect currently.\n\r", ch);
	    return;
	}
	SET_TIMER(victim, TIMER_WRENCHED, ch->water+ch->generation);
	act("$N screams with agony as $E is wrenched out of the negative plane.", ch, NULL, victim, TO_CHAR);
	act("$N screams with agony as $E is wrenched out of the negative plane.", ch, NULL, victim, TO_NOTVICT);
	act("You scream with agony as you are wrenched out of the negative plane.", ch, NULL, victim, TO_VICT);
	return;
	if (victim->cur_form == FRM_MIST)
	{
	    act("$N's incorporeal form solidifies once more.", ch, NULL, victim, TO_CHAR);    
	    act("$N's incorporeal form solidifies once more.", ch, NULL, victim, TO_NOTVICT);    
	    act("You return to humanoid form.", ch, NULL, victim, TO_VICT);    
 	    set_form(victim, 0);
	}
 	if (IS_SET(victim->flag2, AFF_SHADOW_PLANE) )
        {
	    act("$N's body reforms from the shadows.", ch, NULL, victim, TO_CHAR);
	    act("$N's body reforms from the shadows.", ch, NULL, victim, TO_NOTVICT);
	    act("You leave the shadowplane.", ch, NULL, victim, TO_VICT);
	    REMOVE_BIT(victim->flag2, AFF_SHADOW_PLANE);
	}
    }


}


void do_layhands (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    int heal;
    char buf[MAX_STRING_LENGTH];

    if (!TIME_UP(ch, TIMER_LAYONHANDS))
    {
	mprintf(sizeof(buf), buf, "You must wait %d hours to use this power again.\n\r", TIMER(ch, TIMER_LAYONHANDS) );
	send_to_char(buf, ch);
    	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Who do you wish to lay on hands?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, argument) )== NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim==ch)
    {
	send_to_char("You cannot use this power on yourself.\n\r", ch);
	return;
    }

    heal = ch->power[DISC_HERO_FAIT];
    heal *= ch->generation * 10;
    heal = UMAX(10, heal);

    if (victim->secondaryrace == RACE_VAMPIRE)
    {
	send_to_char("You cannot use this power to heal a vampire.\n\r", ch);
	return;
    }

    if (victim->hit > victim->max_hit - 10)
    {
	send_to_char("They are not in desperate need of healing.\n\r", ch);
	return;
    }

    act("$n lays $s hands on you, healing your wounds.", ch, NULL, victim, TO_VICT);
    act("You lay your hands on $N and heal $S wounds.", ch, NULL, victim, TO_CHAR);
    act("$n lays $s hands on $N, healing their wounds.", ch, NULL, victim, TO_NOTVICT);

    victim->hit = UMIN(victim->max_hit, victim->hit + heal);

    WAIT_STATE(ch,10);

    
    /* REMOVED BECAUSE WE DON'T CHANGE ALIGNMENT ANYMORE
    if (dice(1,10) == 1 && victim->alignment > 300)
    {
	send_to_char("That was a good and charitable act.\n\r", ch);
	ch->alignment = UMIN(1000, ch->alignment + 1);
    }

    if (victim->alignment < -500)
    {
	send_to_char("You feel wary of helping the unpure at heart.\n\r", ch);
	ch->alignment = UMAX(300, ch->alignment -1);
    }*/
   
    SET_TIMER(ch, TIMER_LAYONHANDS, 20 - (ch->generation)/2);

}

void do_sigil (CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *wield = NULL;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (ch->power[DISC_HERO_SIGI] < 0)
    {
	send_to_char("You are not yet trained in the Sigil discipline.\n\r", ch);
	return;
    }
    
    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
	send_to_char("Syntax : sigil <weapon> <mah ayn flam ort jig>\n\r", ch);
	return;
    }
    
    for ( obj = ch->carrying ; obj != NULL ; obj = obj->next_content)
    {
	if ( can_see_obj (ch, obj) && is_name (arg1, obj->name))
	wield = obj;
    }

    if (wield == NULL)
    {
	send_to_char("You have not got that item in your inventory.\n\r", ch);
	return;
    }

    if (wield->item_type != ITEM_WEAPON)
    {
	send_to_char("You may only draw a sigil on a weapon.\n\r", ch);
	return;
    }

    if (!str_cmp(arg2, "mah") ) 
    {
	if (IS_SET(wield->value[4], WEAPON_RUNE_FORCE_BOLT) )
	{
	     send_to_char("The Mah rune is already drawn on this weapon.\n\r", ch);
	     return;
	}
	if (ch->faith < 1)
	{
	     send_to_char("It costs one faith point to draw the Mah rune correctly.\n\r", ch);
	     return;
	}
	ch->faith -= 1;
	SET_BIT(wield->value[4], WEAPON_RUNE_FORCE_BOLT);
	act("You draw the Mah rune upon $p.", ch, wield, NULL, TO_CHAR);
	act("$n draws a mystical sigil upon $s weapon.",ch, NULL, NULL, TO_ROOM);
	return;
    }
    
    if (!str_cmp(arg2, "ayn") ) 
    {
	if (ch->power[DISC_HERO_SIGI] < 2)
	{
	    send_to_char("You must be level two in Sigil to draw the Ayn rune.\n\r", ch);
	    return;
	}
	if (IS_SET(wield->value[4], WEAPON_RUNE_SMITE_EVIL) )
	{
	     send_to_char("The Ayn rune is already drawn on this weapon.\n\r", ch);
	     return;
	}
	if (ch->faith < 2)
	{
	     send_to_char("It costs two faith points to draw the Ayn rune correctly.\n\r", ch);
	     return;
	}
	ch->faith -= 2;
	SET_BIT(wield->value[4], WEAPON_RUNE_SMITE_EVIL);
	act("You draw the Ayn rune upon $p.", ch, wield, NULL, TO_CHAR);
	act("$n draws a mystical sigil upon $s weapon.",ch, NULL, NULL, TO_ROOM);
	return;
    }
    if (!str_cmp(arg2, "flam") ) 
    {
	if (ch->power[DISC_HERO_SIGI] < 3)
	{
	    send_to_char("You must be level three in Sigil to draw the Flam rune.\n\r", ch);
	    return;
	}
	if (IS_SET(wield->value[4], WEAPON_RUNE_BLAZE) )
	{
	     send_to_char("The Flam rune is already drawn on this weapon.\n\r", ch);
	     return;
	}
	if (ch->faith < 4)
	{
	     send_to_char("It costs four faith points to draw the Flam rune correctly.\n\r", ch);
	     return;
	}
	ch->faith -= 4;
	SET_BIT(wield->value[4], WEAPON_RUNE_BLAZE);
	act("You draw the Flam rune upon $p.", ch, wield, NULL, TO_CHAR);
	act("$n draws a mystical sigil upon $s weapon.",ch, NULL, NULL, TO_ROOM);
	return;
    }
    if (!str_cmp(arg2, "ort") ) 
    {
	if (ch->power[DISC_HERO_SIGI] < 4)
	{
	    send_to_char("You must be level four in Sigil to draw the Ort rune.\n\r", ch);
	    return;
	}
	if (IS_SET(wield->value[4], WEAPON_RUNE_LIGHTNING) )
	{
	     send_to_char("The Ort rune is already drawn on this weapon.\n\r", ch);
	     return;
	}
	if (ch->faith < 8)
	{
	     send_to_char("It costs eight faith points to draw the Ort rune correctly.\n\r", ch);
	     return;
	}
	ch->faith -= 8;
	SET_BIT(wield->value[4], WEAPON_RUNE_LIGHTNING);
	act("You draw the Ort rune upon $p.", ch, wield, NULL, TO_CHAR);
	act("$n draws a mystical sigil upon $s weapon.",ch, NULL, NULL, TO_ROOM);
	return;
    }
    if (!str_cmp(arg2, "jig") ) 
    {
	if (ch->power[DISC_HERO_SIGI] < 5)
	{
	    send_to_char("You must be level five in Sigil to draw the Jig rune.\n\r", ch);
	    return;
	}
	if (IS_SET(wield->value[4], WEAPON_RUNE_DANCING) )
	{
	     send_to_char("The Jig rune is already drawn on this weapon.\n\r", ch);
	     return;
	}
	if (ch->faith < 12)
	{
	     send_to_char("It costs twelve faith points to draw the Jig rune correctly.\n\r", ch);
	     return;
	}
	ch->faith -= 12;
	SET_BIT(wield->value[4], WEAPON_RUNE_DANCING);
	act("You draw the Jig rune upon $p.", ch, wield, NULL, TO_CHAR);
	act("$n draws a mystical sigil upon $s weapon.",ch, NULL, NULL, TO_ROOM);
	return;
    }
}
