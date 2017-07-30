void do_psearch(CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA d;
  CHAR_DATA *fch;
  CHAR_DATA *qch;
  char pfile[MIL];
  struct dirent *dp;
  DIR *dirptr;

 
  if ((dirptr = opendir(PLAYER_DIR)) == NULL){
    send_to_char("Player dir unable to be opened\n\r",ch);
    return;
  }
 
  while((dp = readdir(dirptr)) != NULL)
  {
    /* read in file (player) names, excluding "." and ".." */
    if ( ( memcmp(&dp->d_name[0],".",1) != 0 )
    &&   ( memcmp(&dp->d_name[0],"..",2) != 0 ) )
    {
      strcpy(pfile,dp->d_name);

      d.original = NULL;

      if (!load_char_obj(&d, pfile)) /* char pfile exists? */
      {
        printf_to_char(ch, "{RNo such player: {W%s{x.\n\r", pfile);
        free_char(d.character);
        continue;
      }

      d.character->desc = NULL; /* safe than sorry */
      fch = d.character;

      if (fch->level == 100) {
        send_to_char("!",ch);


      /* ******** Cleanup ******** */
      for ( qch = char_list ; qch ; qch = qch->next)
      { // Check for rescue-quest mob and remove if loaded
        if ( !IS_NPC( qch ) )
          continue;

        if ( strstr(qch->name,fch->name )
        && ( qch->pIndexData->vnum == MOB_VNUM_QUEST ) )
        {
          do_function(qch,&do_follow,"self");
          do_function(qch,&do_nofollow,"");
          extract_char( qch, TRUE );
        }
      }
      nuke_pets(d.character,FALSE);
      free_char(d.character);

    }
  }
  closedir(dirptr);
  return;
}
