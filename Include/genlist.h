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

// this craziness makes sure I have all I need for both GCC and Visual C++
#ifdef MERC_H // compiling with Draknor
  #define _strdup(s) strdup(s) // VC compliance
  #define DRAKNOR
#else 
  #ifndef VISUALC // my "mock" merc.h in VS2k8
    #include "merc.h"
  #endif
#endif

typedef struct generic_list_item GENLIST_ITEM;
typedef struct generic_list GENERIC_LIST;

struct generic_list {
  GENLIST_ITEM *first;    // first item in the list
  GENLIST_ITEM *last;     // last item in the list
  GENLIST_ITEM *cursor;   // keeps track of location in loops
  bool          sorted;   // is the list in some user-deifined sort order?
  int           top_vnum; // the last used VNum (may not be continuous)
};

struct generic_list_item {
  GENLIST_ITEM *next;    // Next object in list (or NULL, if last)
  GENLIST_ITEM *prev;    // Previous object in list (or NULL, if first)
  int           vnum;    // Automatic unique ID, increases for each added item
  int           id;      // user-defined sortable number value
  char         *name;    // user-defined sortable string value
  char         *display; // user-defined "display" string value
  CHAR_DATA    *lch;     // for making a list of characters
};

GENERIC_LIST * gl_new();
void gl_init(GENERIC_LIST *gList);
void gl_empty(GENERIC_LIST *gList);
void gl_close(GENERIC_LIST *gList);
void gl_reset(GENERIC_LIST *gList);
void gl_free_item(GENLIST_ITEM *gi);
int gl_count_items(GENERIC_LIST *gList);

bool gl_push(GENERIC_LIST *gList, int id, char * name, char * display);
bool gl_pop_item(GENERIC_LIST *gList, void *item, bool isint);
bool gl_pop_name(GENERIC_LIST *gList, char * name);
bool gl_pop_id(GENERIC_LIST *gList, int id);

void gl_sort_by_id(GENERIC_LIST *gList);
void gl_sort_by_vnum(GENERIC_LIST *gList);
void gl_sort_by_name(GENERIC_LIST *gList);
void gl_sort_items(GENERIC_LIST *gList, bool isnum, bool isid, bool ascending);
bool gl_swap_items(GENERIC_LIST *gList, GENLIST_ITEM *g1,GENLIST_ITEM *g2);
bool gl_demote_node(GENERIC_LIST *gList, GENLIST_ITEM *gNode);

#ifdef DRAKNOR
  void gl_send_names_vcolumns(CHAR_DATA *ch, GENERIC_LIST *gList, int col, int width);
  void gl_send_names_hcolumns(CHAR_DATA *ch, GENERIC_LIST *gList, int col, int width);
  void gl_send_names(CHAR_DATA *ch, GENERIC_LIST *gList);
  void g1_send_list(CHAR_DATA *ch, GENERIC_LIST *gList);
#else
  void gl_send_names_vcolumns(GENERIC_LIST *gList, int col, int width);
  void gl_send_names_hcolumns(GENERIC_LIST *gList, int col, int width);
#endif
