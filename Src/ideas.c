//put here for now just to know where it is
if (ch->pcdata->clanowe_dia == 0)
        printf_to_char(ch,"Your debt to %s requires you to get to level %d.\n\r",
                   clan_table[ch->pcdata->clanowe_clan].who_name,
                   ch->pcdata->clanowe_level);
      else if (ch->pcdata->clanowe_level == 0)
        printf_to_char(ch,"You owe %s %d diamond%s.\n\r",
                   clan_table[ch->pcdata->clanowe_clan].who_name,
                   ch->pcdata->clanowe_dia,
                   (ch->pcdata->clanowe_dia == 1 ? "" : "s"));
      else
        printf_to_char(ch,"You owe %s %d diamond%s and need to get to %d level.\n\r",
                   clan_table[ch->pcdata->clanowe_clan].who_name,
                   ch->pcdata->clanowe_dia,
                   (ch->pcdata->clanowe_dia == 1 ? "" : "s"),
                   ch->pcdata->clanowe_level);
    }



//Could use a stamina system I think.
const struct stamina_table_identifiers stamina_table [20];

const struct stamina_table_identifiers
{
	sh_int stamina_number;
}

const struct stamina_table_identifiers stamina_table  [21] =
{

   {  5  },     /*  0 stamina */
   {  10 },     /*  1 stamina */
   {  15 },     /*  2 stamina */
   .....
   .....
  ..... 

}


stamina_table[ get_curr_stat( STAT_STAMINA ).stamina_number
