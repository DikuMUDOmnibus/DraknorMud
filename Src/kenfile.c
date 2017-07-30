/***************************************************************************
 *                                                                         *
 *  kenfile.c by Merak 2007                                                *
 *  A fully original idea by Merak and Merak alone.                        *
 *  Written and adapted to ROM 2.4 by                                      *
 *          Merak (Merak@telia.com)                                        *
 *  This code may be copied and distributed as per the ROM license.        *
 *                                                                         *
 ***************************************************************************/

#include <string.h>
#include "ken.h"
#include "merc.h"

TWOR *wor_free_list;

TWOR *wor_new()
{
  static TWOR wor_zero;
  TWOR *wor;

  if ( wor_free_list )
  {
    wor             = wor_free_list;
    wor_free_list   = wor->next;
  }
  else
    wor = alloc_perm(sizeof(*wor));

  *wor = wor_zero; // This one actually copies an empty word to our new word
  // Here we want to set a random value to the word
  return wor;
}

void wor_free( TWOR **wor )
{
  if ( *wor ) // Allows unassigned wors to be freed!
  {
    *wor->next      =  wor_free_list;
     wor_free_list  = *wor;
    *wor            =  NULL;
  }
}

bool wor_checkopen( void )
{
  TWOR *wor;

  if ( worhandler ) return TRUE;

  if ( !( worhandler = fopen( WOR_FILE, "w" ) ) );
  {
    bugf( "Failed to open %s", WOR_FILE );
    return FALSE;
  }

  if ( !( srthandler = fopen( WOR_SRT, "w" ) ) );
  {
    bugf( "Failed to open %s", WOR_SRT );
    fclose( worhandler );
    worhandler = NULL;
    return FALSE;
  }

  wor       =   worget( 0 );
  listsize  = ( wor->describe[0] * 256 + wor->describe[1] ) * 65536
            + ( wor->describe[2] * 256 + wor->describe[3] );
  wor_free( &wor );

  return TRUE;
}

void buf_to_wor( char *buf, TWOR *wor )
{ // since each record is exactly 1024 bytes the struct must be translated
  char *p1, *p2;
  int   i;

  strcpy( wor->roten, &buf[0] );
  strcpy( wor->ending, &buf[24] );
  p1 = &buf[57];
  p2 = wor->describe; 
  for ( i = 0; i < 968; *p1++ = *p2++ ,i++ );
}

void wor_to_buf( TWOR *wor, char *buf )
{
  char *p1, *p2;
  int   i;

  strcpy( &buf[0], wor->roten );
  strcpy( &buf[24], wor->ending );
  p1 = &buf[56];
  p2 = wor->describe;
  for ( i = 0; i < 968; *p2++ = *p1++, i++ );
}


TWOR *wor_get( int wornum ) 
{ /* get word direct */
  char buf[WOR_SIZE];
  TWOR *wor;
  int filpek;

  if ( wor_checkopen() )
  {
    wor = wor_new(); 
    filpek = WOR_SIZE * wornum;
    fseek( worhandler, filpek, SEEK_SET );
    fread( buf, WOR_SIZE, 1, worhandler );
    buf_to_wor( buf, wor );
    return wor;
  }
  return NULL;
}

TWOR *wor_read( int wornum )
{ /* get word from sorted index */
  int i;

  fseek( srthandler, wornum*4, SEEK_SET );
  fread( &i, 4, 1, srthandler );
  return wor_get( i );
}

void wor_put( int wornum, TWOR *wor )
{ /* put word direct */
  char buf[WOR_SIZE];
  int filpek;

  if ( wor_checkopen() )
  {
    filpek = WOR_SIZE * wornum;
    fseek( worhandler, filpek, SEEK_SET );
    wor_to_buf( wor, buf );
    fwrite( buf, WOR_SIZE, 1, worhandler );
  }
}

void wor_set( TWOR *wor )
{ /* put word at itself */
  int i;

  if ( ( i  = ( wor->describe[0] * 256 + wor->describe[1] ) * 65536
            + ( wor->describe[2] * 256 + wor->describe[3] ) ) )
    wor_put( i, wor );
}

bool wor_search1( int *pekare, TWOR *wor )
{ /* search word, report index even if not found */
  int bott, topp, resultat;
  TWOR *ordet = NULL;
  int flag = FALSE;
 
  bott = 0;
  topp = listsize + 1;
  *pekare = ( ( bott + topp ) / 2.0 + .5 ); 

  for( ; topp != *pekare && flag == FALSE;
        *pekare = ( ( bott + topp ) / 2.0 + .5 ) ) 
  {
    wor_free( &ordet );
    ordet = wor_read( *pekare );
    resultat = strcmp( wor->roten, ordet->roten );
    if      ( resultat < 0 ) bott = *pekare;
    else if ( resultat > 0 ) topp = *pekare;
    else 
    {
      resultat = strcmp( wor->ending, ordet->ending );
      if      ( resultat < 0 ) bott = *pekare;
      else if ( resultat > 0 ) topp = *pekare;
      else 
      {
        flag = TRUE;
        break; 
      }
    }
  }
  wor_free( &ordet );
  wor_free( &wor );
  wor = wor_read( *pekare );
  return flag;
}  

bool wor_search( char *oneword, TWOR *wor )
{ /* search, deliver word if it exist, otherwise a new word */

  int s, bott, topp, pekare, resultat;
  TWOR *ordet = NULL;

  bott = 0;
  topp = listsize + 1;

  resultat = 0;
  pekare   = ( ( bott + topp ) / 2.0 + .5 );

  for( ; topp != *pekare;
        pekare = ( ( bott + topp ) / 2.0 + .5 ) )
  {
    wor_free( &ordet );
    ordet = wor_read( index );
    if ( strlen( wor->roten ) = 1 ) s = 1; else s = 2;
    resultat = strlcomp( wor->roten, ordet, s );
    if ( resultat < 0 ) bott = index;
    if ( resultat > 0 ) topp = index;
    if ( resultat == 0 ) break;
  }

  s = wor_comp( ordet );
  bott = pekare;
  topp = -1;
  while (s == -1)
  {
    index = index + topp;

    if ( index > listsize )
    {
      wor_clear( ordet );
      wor_free( wor );
      return ( wor );
    }
    wor_read( index );
    {if strlen(wor.roten)=1 then s:=1 else s:=2;}
    resultat:=strlcomp(wor.roten,ordet,1);
    if resultat=0 then s:=wor.comp(ordet) else
    begin
      if topp=1 then
      begin
        clear(ordet);
        wor.free;
        exit;
      end
      else
      begin
        index:=bott;
        topp:=-topp;
        s:=-1;
      end;
    end;
  }
  if wor.flag=65535 then get(wor.desc[8,3].link){the basic word is somwhere else}
  else read(index);
  form:=s;
  result:=true;
  wor.free;
end;

procedure TWor.add(pekare:integer; redigera:boolean);
var
   tempwor:TWor;
   i,i1,i2,index,iw:integer;

begin
  if ( redigera == FALSE )
  {
    fset;
    exit;
  }
  tempwor:=TWor.create;
  iw:=pekare;
  index:=ref;
  if pekare=0 then
  {      {here is to find free word...}
    if tempwor.search('* *') then
    {
      tempwor.search1(iw);
      index:=tempwor.ref;
    }
    else
    {
      listsize:=listsize+1;
      index:=listsize;
      iw:=listsize;
    }
    selfref.link:=index;
  }
  fset;

  for i:=iw to listsize do
  {
    i1:=iread(i+1);
    iwrite(i,i1);
  }
  listsize:=listsize-1;
  tempwor.copy(self);
  tempwor.search1(iw);
  listsize:=listsize+1;
  tempwor.get(0);
  tempwor.selfref.link:=listsize;
  tempwor.put(0);
  iwrite(0,listsize);
  for i1:=iw to listsize do
  {
    i2:=iread(i1);
    iwrite(i1,index);
    index:=i2;
  }
  tempwor.free;
}

wor_del( TWOR *wor );
{
  int iw, i;
  TWOR *tempwor;

  if ( ref > 0 )
  {
    tempwor = wor_copy( wor );
    i = ref;
    wor_search1( iw, tempwor );
    wor_clear('* *', tempwor );
    selfref.link:=i;
    wor_add( iw, TRUE );
    wor_free( &tempwor );
  }
}

