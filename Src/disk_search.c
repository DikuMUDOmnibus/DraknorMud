/**************************************************************************\
 *      The Sacred Codebase(Sacred) is copyright 1997-2003 by             *
 *      Chris Litchfield and Mark Archambault                             *
 *      Sacred has been created with much time and effort from many       *
 *      different people's input and ideas.                               *
 *      By using this code, you have agreed to follow the terms of the    *
 *      Sacred license, in the file doc/sacred.license                    *
\**************************************************************************/
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include <dirent.h>

bool mud_read_player_dir(char temp_table[MAX_PLAYERS][MSL], int *num_files)
{
  struct dirent *dp;
  DIR *dirptr;
  int return_int, return_int2;
  int arrcnum_files=0;
    
  if ((dirptr = opendir(PLAYER_DIR)) == NULL){
    bug("Player dir unable to be opened",0);
    return(FALSE);
  }
  while((dp=readdir(dirptr)) != NULL){
    /* read in filename */
    return_int = memcmp(&dp->d_name[0],".",1);
    return_int2 = memcmp(&dp->d_name[0],"..",2);
    if ((return_int != 0 ) && (return_int2 != 0 )) {
      /*Get Start and Stop Time for DERG File here!!  */
      strcpy(temp_table[arrcnum_files],dp->d_name);
      arrcnum_files++;
    }
  }
  closedir(dirptr);
  *num_files = arrcnum_files;
  return(TRUE);
}

char player_table[MAX_PLAYERS][MSL];
char clan_player[MAX_PLAYERS][MSL];

bool get_specific_field_from_char_file(char *player, char *key_field,
               int *field, int state)
{
  FILE *fp;
  char *word;
  int  numb = 0;
  char strsave[MSL];

  mprintf( sizeof(strsave), strsave, "%s%s", PLAYER_DIR, capitalize( player ) );
  if ((fp = fopen(strsave,"r")) == NULL)
    return(FALSE);
  nFilesOpen++;
    
  for ( ; ; )
  {
    word   = feof( fp ) ? "End" : fread_word( fp );
    if ( !strcmp( word, key_field ) ) 
    {
      // Expanded from KEY2 define
      if ( !str_cmp( word, key_field ) )
        numb  = fread_number( fp );
      *field = numb;
      fclose(fp);
      nFilesOpen--;
      return( TRUE );
    }

    if ( !strcmp( word, "End" ) ) 
    {
      fclose(fp);
      nFilesOpen--;
      return(FALSE);
    }
  }  
}

bool get_specific_field_from_char_file2(char *player, char *key_field, char *field)
{
  FILE *fp;
  char *word;
  char strsave[MSL];

  mprintf( sizeof(strsave), strsave, "%s%s", PLAYER_DIR, capitalize( player ) );
  if ((fp = fopen(strsave,"r")) == NULL)
    return(FALSE);
  nFilesOpen++;
  for ( ; ; )
    {
      word   = feof( fp ) ? "End" : fread_word( fp );
      if (!strcmp(word,key_field)) {
  field = fread_string( fp );
  fclose(fp);
  nFilesOpen--;
  return(TRUE);
      }
      if (!strcmp(word,"End")) {
  fclose(fp);
  nFilesOpen--;
  return(FALSE);
      }
    }  
}


bool mud_read_clan_players(int *num_clannies, int clan)
{
  int i=0,clan_num=0,j=0,x=0;
  int num_players;
  int clan_rank=0;
  *num_clannies =0;
  if (!mud_read_player_dir(player_table,&num_players))
    return(FALSE);
  /* sort by rank */
  for (x=13; x >= 0; x--) {
    for (i=0; i < num_players; i++) {
      if (!get_specific_field_from_char_file(player_table[i], "Clan",
               &clan_num, 1))
  continue;
      if (!get_specific_field_from_char_file(player_table[i], "Rank",
               &clan_rank, 1))
  continue;
      if ((clan_num == clan)&& (x == clan_rank)) {
  strcpy(clan_player[j++],player_table[i]);
      }
    }
  }
  *num_clannies = j;
  return(TRUE);
}

