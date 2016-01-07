#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

// Gotta avoid clobbering with list, so names are a little weird
// Maybe I should just append a UUID to every header, haha

typedef struct clist clist_t;

typedef struct clist_itr clist_itr_t;


// push/pop/extract front/back
// extract/erase/at(?)
// prune (remove those who's provided function says to remove/extract (WHICH???))
// destroy/create/import
// sort (awwwww, can't just use qsort now... well, without doing something gross)


// try to follow the order of dyn_array?

// create
// import
// export (uhh... give up space for the data and serialize?)
// destroy

clist_t *clist_create(const size_t data_type_size, void (*destruct_func)(void *const));

// just going to be a loop of push_backs? Surely it can be optimized.
// Something like dyn_shift. dyn_alter? dyn_core? enum on insert/extract/remove?
// Ooooohhh I like that.
clist_t *clist_import(const void *const data, const size_t count, const size_t data_type_size, void (*destruct_func)(void *));

bool clist_export(const clist_t *const clist, void *data_dest);

void clist_destroy(clist_t *const clist);

bool clist_push_front(clist_t *const clist, const void *const data);
bool clist_pop_front(clist_t *const clist);
bool clist_extract_front(clist_t *const clist, void *const data);

bool clist_push_back(clist_t *const clist, const void *const data);
bool pop_back(clist_t *const clist);
bool extract_back(clist_t *const clist, void *const data);

insert
extract
erase
at

itr_insert // insert at position, iterator does not change, prev of node does
itr_extract // removes itr, position gets incremented
itr_erase // removes itr, position gets incremented
itr_at // just returns pointer of itr'd data (rooted = NULL)

itr_create // take list and desired position? itr_begin itr_end itr_at itr_create(rooted)?
itr_destroy
itr_reset
itr_rooted // need better name, detect if itr at root (is reset/taversal complete)
itr_inc
itr_dec // merge the two?
itr_distance // check backwards or return SIZE_MAX if begin after end?
itr_equal // same as distance == 0, which short circuits distance, but if not eq, would trigger full distance hunt

sort
splice
merge
remove_if/prune
for_each
