/* Generic Linked Lists v1.1
 *
 * Created be David Carroll, for The Lands of Draknor MUD (www.landsofdraknor.net:4000)
 *
 * This code abstracts a linked list so that it is easy to manage, but still very fast.
 * It includes functions to sort the list by a number (id) or a string (name) for an
 * array of strings (display) so that you can easily sort a long list of lines simply
 * and push them to the char all at once, in whatever order you wish.
 *
 * GenList can be extended, but it's not suggested: each addition will slow the whole.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "genlist.h"

#ifdef DRAKNOR
  #include "recycle.h"
#endif

/* ********************************************** */
/* ******* Create a new list root object ******** */
/* ********************************************** */
GENERIC_LIST * gl_new()
{
  GENERIC_LIST *gList;
  /* allocate memory for a new root node */
  gList = malloc( sizeof(*gList) );
  /* if it points to NULL, we ran out of memory! */
  if (gList == NULL) { return NULL; }

  /* give our new list some default values */
  gl_init(gList);

  return gList;
}

/* ********************************************** */
/* ****** Sets up a new linked list to use ****** */
/* ********************************************** */
void gl_init(GENERIC_LIST *gList)
{
  gList->first = NULL;
  gList->last = NULL;
  gList->cursor = NULL;
  gList->sorted = TRUE;
  gList->top_vnum = 0;
}

/* ********************************************** */
/* **** Empties the list and frees its nodes **** */
/* ********************************************** */
void gl_empty(GENERIC_LIST *gList)
{
  GENLIST_ITEM *gPrev;
  /* loop through the entire list backwards and free all memory from each list node */
  for (gList->cursor = gList->last; gList->cursor != NULL; gList->cursor = gPrev)
  {
    gPrev = gList->cursor->prev;
    gl_free_item(gList->cursor);
  }
  return;
}

/* ********************************************** */
/* ****** Resets the linked list for reuse ****** */
/* ********************************************** */
void gl_reset(GENERIC_LIST *gList)
{
  gl_empty(gList);
  gl_init(gList);
  return;
}

/* ********************************************** */
/* ***** Cleans up the finished linked list ***** */
/* ********************************************** */
void gl_close(GENERIC_LIST *gList)
{
  /* remove all nodes */
  gl_empty(gList);

  /* then remove the list from memory */
  if (gList != NULL)
    free(gList);

  return;
}

/* ********************************************** */
/* ***** Remove a gen_list item from memory ***** */
/* ********************************************** */
void gl_free_item(GENLIST_ITEM *gi)
{
  /* Free all a node's strings FIRST */
  free(gi->name);
  free(gi->display);
  /* Then release the node's now-unused memory */
  free(gi);
}

/* ********************************************** */
/* ***** Remove a gen_list item from memory ***** */
/* ********************************************** */
int gl_count_items(GENERIC_LIST *gList)
{
  int c = 0;
  for (gList->cursor=gList->first; gList->cursor != NULL; gList->cursor = gList->cursor->next)
    c++;
  return c;
}

/* ********************************************** */
/* ***** Adds a new item to the linked list ***** */
/* ********************************************** */
bool gl_push(GENERIC_LIST *gList, int id, char * name, char * display)
{
  GENLIST_ITEM *gNew;

/* as soon as we add an item, we can't guarantee the list is sorted */
  gList->sorted = FALSE;

  /* allocate memory for a new node and fill in the passed values */
  gNew = malloc( sizeof(*gNew) );
  /* if it points to NULL, we ran out of memory! */
  if (gNew == NULL) { return FALSE; }

  gNew->id = id;
  gNew->vnum = ++gList->top_vnum;

  /* we can accept NULL names*/
  if (name == NULL) { gNew->name = NULL; }
  else { gNew->name = _strdup(name); }

  /* we can accept NULL display lines */
  if (display == NULL) { gNew->display = NULL; }
  else { gNew->display = _strdup(display); }

  /* if there's no "first" item, this must be it! */
  if (gList->first == NULL) {
    gNew->prev = NULL;
    gList->first = gNew;
    gList->cursor = gNew;
    gList->sorted = TRUE;
  }

  /* if there's a "last" item, tack this onto the end */
  if (gList->last != NULL) {
    gNew->prev = gList->last;
    gList->last->next = gNew;
  }

  /* new items are always "last" in the list */
  gList->last = gNew;
  gNew->next = NULL;

  return TRUE;
}

/* ********************************************** */
/* **** Remove a specific item from the list **** */
/* ********************************************** */
bool gl_pop_name(GENERIC_LIST *gList, char * name) {
  return gl_pop_item(gList,name,FALSE); }

bool gl_pop_id(GENERIC_LIST *gList, int id) {
  return gl_pop_item(gList,&id,TRUE); }

bool gl_pop_item(GENERIC_LIST *gList, void *item, bool isint)
{
  GENLIST_ITEM *gNext;
  bool found = FALSE;
  char *sVal = NULL;
  int iVal = 0;

  /* If we're matching a string and it's NULL, GTFO! */
  if (!isint && ((sVal = (char *) item) == NULL))
    return FALSE;

  /* If it's an int, "(int*)item" contains an address to the value */
  if (isint)
    iVal = *(int *) item;

  for (gList->cursor=gList->first; gList->cursor != NULL; gList->cursor = gNext)
  {
    gNext = gList->cursor->next;

    /* Does this node match?  If not, move to next */
    if (isint && (gList->cursor->id != iVal))
      continue;
    if (!isint && strcmp(gList->cursor->name, sVal))
      continue;

    found = TRUE;

    /* Match: point the previous item to the next (to remove this one from the chain) */
    if (gList->cursor->prev == NULL) { gList->first = gList->cursor->next; }
    else { gList->cursor->prev->next = gList->cursor->next; }

    /* Match: point the next item to the previous (to remove this one from the chain) */
    if (gList->cursor->next == NULL) { gList->last = gList->cursor->prev; }
    else { gList->cursor->next->prev = gList->cursor->prev; }

    /* Nothing points to node, so free its memory or it will be "lost" (memory leak) */
    gl_free_item(gList->cursor);
  }

  return found;
}

/* ****************************************************************************** */
/* *                                  SORTING                                   * */
/* ****************************************************************************** */
void gl_sort_by_id(GENERIC_LIST *gList) { gl_sort_items(gList, TRUE, TRUE, TRUE); return; }
void gl_sort_by_vnum(GENERIC_LIST *gList) { gl_sort_items(gList, TRUE, FALSE, TRUE); return; }
void gl_sort_by_name(GENERIC_LIST *gList) { gl_sort_items(gList, FALSE, FALSE, TRUE); return; }

/* ********************************************** */
/* ***** Workhorse Bubble sorting algorithm ***** */
/* ** Important: this sorting algorithm sucks! ** */
/* ********************************************** */
void gl_sort_items(GENERIC_LIST *gList, bool isnum, bool isid, bool ascending) {
  bool ordered = FALSE;
  int cVal, nVal;

  /* If list has 0-1 items, it's impossible to NOT be in sorted order */
  if ( (gList->first == NULL) || (gList->first->next == NULL) )
  {
    gList->sorted = TRUE;
    return;
  }

  /* We loop until the job is done! */
  while (!ordered)
  {
    /* start off assuming it's in order */
    ordered = TRUE;

    /* go through the entire list to see if any two connected items are out of order */
    for (gList->cursor = gList->first; gList->cursor->next != NULL; gList->cursor = gList->cursor->next)
    {
      /* get the relevant values we need to compare */
      if (isnum) {
        if (isid) {
          /* gl_sort_by_id() was called */
          cVal = gList->cursor->id;
          nVal = gList->cursor->next->id;
        } else {
          /* gl_sort_by_vnum() was called */
          cVal = gList->cursor->vnum;
          nVal = gList->cursor->next->vnum;
        }
      } else { // !isnum:
        /* gl_sort_by_name() was called */
        cVal = strcmp(gList->cursor->name, gList->cursor->next->name);
        nVal = 0; // cVal is -1 if arg1 < arg2 -- +1 if arg1 > arg2 -- 0 if equal
      }

      /* if this item and the next are out of order, swap them and start the sort over */
      if (  (ascending && (cVal > nVal))
      ||   (!ascending && (cVal < nVal)) ) {
        ordered = FALSE;
        gl_swap_items(gList, gList->cursor, gList->cursor->next);
        break;
      } // if (out of order)
    } //for
  } // while

  /* This is why we came here: */
  gList->sorted = TRUE;
  return;
}

/* ********************************************** */
/* ***** Swaps the list position of 2 items ***** */
/* ********************************************** */
bool gl_swap_items(GENERIC_LIST *gList, GENLIST_ITEM *g1,GENLIST_ITEM *g2) {
  GENLIST_ITEM *tmp;

  /* This function needs to take the "next" and "previous" values from
   * the two nodes and switch them, so that their order in the list is
   * exchanged.  But we also need to consider the nodes that point back
   * to these, and swap those as well, to keep list order integrity.
   */

  /* Swap the "next" pointers on the two items */
  tmp = g1->next;
  g1->next = g2->next;
  g2->next = tmp;

  /* For each (if not "last"), make their new "next" nodes point back properly */
  if (g1->next != NULL) { g1->next->prev = g1; }
  if (g2->next != NULL) { g2->next->prev = g2; }

  /* Swap the "previous" pointers on the two items */
  tmp = g1->prev;
  g1->prev = g2->prev;
  g2->prev = tmp;

  /* For each (if not "first"), make their new "previous" nodes point forward properly */
  if (g1->prev != NULL) { g1->prev->next = g1; }
  if (g2->prev != NULL) { g2->prev->next = g2; }

  /* If either was moved to an extreme, modify the list root */
  if (g1->prev == NULL) { gList->first = g1; }
  if (g2->prev == NULL) { gList->first = g2; }
  if (g1->next == NULL) { gList->last = g1; }
  if (g2->next == NULL) { gList->last = g2; }

  /* We cannot guarantee the list is in order at this point, and could have disorganized it */
  gList->sorted = FALSE;

  /* At least the swap was successful: */
  return TRUE;
}

/* ****************************************************************************** */
/* *                                 DISPLAYING                                 * */
/* ****************************************************************************** */
/* ********************************************** * A D G */
/* ****** Sends vertically-columnized list ****** * B E H */
/* ********************************************** * C F I */
void gl_send_names_vcolumns(CHAR_DATA *ch, GENERIC_LIST *gList, int colct, int width)
{
/* This function can create unexpected output if the number of results
 * fits exactly into cols-1 columns.  It can also create a short last
 * column, but this is the correct result, due to sorting.
 */
  BUFFER *buffer;
  int items = gl_count_items(gList);
  int rows, row, col, i, offset;
  int cols = colct;

  if ((ch == NULL) || IS_NPC(ch)) { return; }
  if ((cols <= 0) || (width <= 0)) { return; }

  buffer = new_buf();

  rows = items / cols;

/* Not sure if there's a good way to prevent a blank last column
  if ((items % (cols-1)) == 0)
    items++;
*/

  for (row = 0; row < (rows + 1); row++)
  {
    for (col = 0; col < cols; col++)
    {
      offset = (col * rows) + row + col;

      gList->cursor = gList->first;

      for (i = 0; (i < offset) && (gList->cursor != NULL); i++)
        gList->cursor = gList->cursor->next;

      if ((gList->cursor != NULL)
      &&  (gList->cursor->name != NULL))
        bprintf(buffer, "%-*s", width, gList->cursor->name);
    } // col
    bprintf(buffer, "\n"); // end of each row
  } // row

  page_to_char( buf_string( buffer ), ch );
  free_buf(buffer);
  return;
}

/* ********************************************** * A B C */
/* ***** Sends horizontally-columnized list ***** * D E F */
/* ********************************************** * G H I */
void gl_send_names_hcolumns(CHAR_DATA *ch, GENERIC_LIST *gList, int col, int width)
{
  BUFFER *buffer;
  int i = 1;
  if ((ch == NULL) || IS_NPC(ch))
    return;

  buffer = new_buf();

  for (gList->cursor=gList->first; gList->cursor != NULL; gList->cursor = gList->cursor->next)
  {
    /* We will ignore any NULL names */
    if (gList->cursor->name != NULL)
    {
      bprintf(buffer, "%-*s", width, gList->cursor->name);
      /* add newline if we hit the column count */
      if ((++i % col) == 1)
        bprintf(buffer, "\n");
    }
  }

  if ((i % col) != 1)
    bprintf(buffer, "\n");

  page_to_char( buf_string( buffer ), ch );
  free_buf(buffer);
  return;
}

/* ********************************************** */
/* **** Sends a simple name list to the char **** */
/* ********************************************** */
void gl_send_names(CHAR_DATA *ch, GENERIC_LIST *gList) {
  BUFFER *buffer;

  if ((ch == NULL) || IS_NPC(ch))
    return;

  buffer = new_buf();

  for (gList->cursor=gList->first; gList->cursor != NULL; gList->cursor = gList->cursor->next)
    if (gList->cursor->name != NULL)
      bprintf(buffer, "%s\n", gList->cursor->name);

  page_to_char( buf_string( buffer ), ch );
  free_buf(buffer);
  return;
}

/* ********************************************** */
/* **** Sends the display lines to the char ***** */
/* ********************************************** */
void g1_send_list(CHAR_DATA *ch, GENERIC_LIST *gList) {
  BUFFER *buffer;

  if ((ch == NULL) || IS_NPC(ch))
    return; // 

  buffer = new_buf();

  for (gList->cursor=gList->first; gList->cursor != NULL; gList->cursor = gList->cursor->next)
    if (gList->cursor->display != NULL)
      bprintf(buffer, "%s\n", gList->cursor->display);

  page_to_char( buf_string( buffer ), ch );
  free_buf(buffer);
  return;
}

/* ****************************************************************************** */
/* *                                    EOF                                     * */
/* ****************************************************************************** */
