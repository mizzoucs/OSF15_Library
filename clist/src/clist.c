#include "../include/clist.h"

#include <stdint.h>

typedef struct node {
    node_t *prev, *next;  // ORDER AND LOCATION IS VITAL, DO NOT REORDER THESE (make it an array of 2?)
} node_t;

struct clist {
    node_t *back, *front;  // ORDER AND LOCATION IS VITAL, DO NOT REORDER THESE (make it an array of 2?)
    size_t size;
    const size_t data_size;
    void (*destructor)(void *const);
};

// it's a list. It benefits from an iterator. The number of functions just doubled. Ugh.
struct clist_itr {
    node_t *root, *curr;
};

// Since I can't scan my notes and attach it here...
/*
    Root connects to the two ends, BUT THE TWO ENDS ALSO CONNECT TO ROOT. It makes relinking faster, and NO LINK IS EVER
   NULL
    Root, when empty, should point to itself at both ends. PREFER SIZE CHECK TO LINK CHECK
        Cores will prevent circular link issues with size checks before starting, complex functions should do the same

    There is a VITAL test in the test folder. It will assert that the relative offset of root's back and front are the
   same
    as the node relative offsets. This way, root can be cast to a node and not cause a freakout, and that the circular
   link is fine
    // TODO: Find a way to force cmake to run this test on install

    Not going to store the size and destructor in the nodes because that's a major waste of space, though it is a little
   more restrictive
    The list-who-shall-not-be-named (hetero_list) will be interesting and allow varying size and destructors
    (does that make this homo_list? It would be a better name than dyn_list...)
*/


// So, if node gets padded somehow, we'll avoid the disaster case of us just going crazy
//  but we will waste the padded space ...but isn't padding meant to be wasted?
#define DATA_POINTER(node_ptr) ((void *) ((((node_t *) (node_ptr)) + 1)))

#define NODE_SET(node_ptr, data_ptr, data_size) (memcpy(DATA_POINTER(node_ptr), (data_ptr), (data_size)))

#define NODE_GET(node_ptr, data_ptr, data_size) (memcpy((data_ptr), DATA_POINTER(node_ptr), (data_size)))

// Such a pain
#define INCREMENT_VOID(ptr, value) ((void *) (((uint8_t *) (ptr)) + value))


// CORE FUNCTIONS. They are what they sound like, core functions.
// They do intense checks, so if you're a thin wrapper, do only what needs to be done to construct your core call

// Does what it sounds like, inserts count objects from data_src after position
// False on parameter or malloc failure
bool clist_core_insert(const clist_itr_t position, const size_t count, const void *const data_src);

// Extracts count objects from position, placing it in data_dest and removing the nodes
// False on parameter error
bool clist_core_extract(const clist_itr_t position, const size_t count, void *data_dest);

// Deconstructs count objects at position and removes the nodes.
// False on parameter error
bool clist_core_deconstruct(const clist_itr_t position, const size_t count);

// Hunts down the requested node, NULL on parameter issue
node_t *clist_core_locate(const clist_t *const clist, const size_t position);

// Returns an iterator to position + count, zero'd struct on failure
//  Fails if traversal reaches the list root (or if given position is root)
clist_itr_t clist_core_range_find(clist_itr_t position, const size_t count);

// This should be the only creation function
clist_t *clist_create(const size_t data_type_size, void (*const destruct_func)(void *const)) {
    clist_t clist = NULL;
    if (data_size) {
        clist = malloc(sizeof(clist_t));
        if (clist) {
            memcpy(clist, &((clist_t){clist, clist, 0, data_type_size, destruct_func}), sizeof(clist_t));
            // clist->back = clist;
            // clist->front = clist;
            // clist->size = 0;
            // clist->data_size = data_size;
            // memset(&clist->data_size, &data_type_size, sizeof(clist->data_size));
            // clist->destructor = destruct_func;
            // memcpy(&clist->destructor, &destruct_func, sizeof(clist->destructor));
        }
    }
    return clist;
}

clist_t *clist_import(const void *const data, const size_t count, const size_t data_type_size,
                      void (*const destruct_func)(void *)) {
    clist_t clist = clist_create(data_type_size, destruct_func);
    if (clist) {
        if (dyn_core_insert({clist, clist}, count, data)) {
            return clist;
        }
        clist_destroy(clist);
    }
    return NULL;
}

// simple serialization
// dump the work onto for_each?????? (too much jumping around?)
bool clist_export(const clist_t *const clist, void *data_dest) {
    // there's not mcuh that can go wrong without the list being totally broken
    if (clist && clist->size) {
        clist_itr_t itr        = {clist, clist->front};
        const size_t data_size = itr.root->data_size;
        while (itr.root != itr.curr) {
            NODE_GET(itr.curr, data_dest, data_size);
            data_dest = INCREMENT_VOID(data_dest, data_size);
            itr.curr  = itr.curr->next;
        }
        return true;
    }
    return false;
}

void clist_destroy(clist_t *const clist) {
    if (clist) {
        dyn_core_deconstruct(clist, clist->size);
        // deconstruct only fails on NULL, or zero size
        // and neither of those are a threat to free
        free(clist);
    }
}




bool clist_push_front(clist_t *const clist, const void *const data) {
    return clist_core_insert({clist, clist}, 1, data);
}

bool clist_pop_front(clist_t *const clist) {
    return clist ? clist_core_deconstruct
}
bool clist_extract_front(clist_t *const clist, void *const data);




//
///
// HERE BE DRAGONS
///
//


// Because nodes are getting weird and I love them
#define NODE_CALLOC(data_size) ((node_t *) calloc(1, sizeof(node_t) + (data_size)))

// Returns an array of linked nodes of size count
//  Ends of list are NULL!
// Also, free the returned pointer when linked up
// Null on param or malloc failure
// Probably overkill if you're just creating one node
node_t **clist_core_allocate(const size_t count, const size_t data_size) {
    if (count && data_size) {
        node_t **node_array = calloc(count, sizeof(node_t *));
        if (node_array) {
            bool alloc_complete = true;
            for (size_t i = 0; i < count && alloc_complete; ++i) {
                node_array[i] = NODE_CALLOC(data_size);
                // Should be perfectly safe
                alloc_complete = node_array[i];
            }
            if (alloc_complete) {
                // Process them! Because I'm laaazy.
                for (size_t i = 0; i < (count - 1); ++i) {
                    node_array[i]->next = node_array[i + 1];
                }
                // Did I mention I'm lazy?
                for (size_t i = count - 1; i > 0; --i) {
                    node_array[i]->prev = node_array[i - 1];
                }
                // That's all for this version!
                return node_array;
            }
            // Loop of sadness
            for (size_t i = 0; i < count && node_array[i]; ++i) {
                free(node_array[i]);
            }
            free(node_array);
        }
    }
    return NULL;
}

// just use the macro?
inline node_t *clist_core_allocate_single(const size_t data_size) {
    return NODE_CALLOC(data_size);
}

// Does what it sounds like, inserts count objects from data_src after position
// False on parameter or malloc failure
bool clist_core_insert(const clist_itr_t position, const size_t count, const void *data_src) {
    // well, at least the == can try to see if we're working in the right object
    if (position.root && position.curr && count && data_src) {
        const size_t data_size = position.root->data_size;
        node_t **new_nodes = clist_core_allocate(count, data_size);
        if (new_nodes) {
            // We can no longer fail, woo!
            node_t *cur_ptr = position.curr;  // Deconst issue?

            cur_ptr->next->prev = new_nodes[count - 1];
            cur_ptr->next       = new_nodes[0];
            position.root->size += count;
            // Linking complete, but the data's not there yet :/

            for (size_t i = 0; i < count;
                 ++i, data_src = INCREMENT_VOID(data_src, data_size), cur_ptr = cur_ptr->next) {
                NODE_SET(new_nodes[i], data_src, data_size);
            }
            free(new_nodes);
            // Yay!
            return true;
        }
    }
    return false;
}


// Modes are parameters, the rest are for grouping
typedef enum {
    MODE_COPY          = 0x01,
    MODE_EXTRACT       = 0x03,
    MODE_ERASE         = 0x02,
    CLIST_EXFILTRATION = 0x01,
    CLIST_REMOVAL      = 0x02
} clist_mode;

// "moves" data, either extract, deconstruct, or range copy, based on mode
// I'd call it remove, but a copy isn't a removal! Names are hard.
// Mode isn't verified as a valid mode because there isn't a pretty way to do it
bool clist_core_move(const clist_itr_t position, const size_t count, void *data_dest, const clist_mode mode) {
    clist_itr_t end_node = clist_core_range_find(position, count);            // deconst isue?
    if (end_node.root && ((mode & CLIST_EXFILTRATION) ? data_dest : true)) {  // got a result
        // Shouldn't fail past here?

        // relink and decrement count first
        // then our workspace is segmented from the list

        if (mode & CLIST_REMOVAL) {  // can just do if(mode) but this is more explicit
            position.curr->prev.next = end_node.curr;
            end_node.curr->prev      = position.curr->prev;
            position.root->size -= count;
            // List rerouted!
        }

        const size_t data_size = position->root.size;

        node_t *backup = position.curr->next;
        void (*const destructor)(void *const) = position.root->destructor;

        // Separate loops?
        // Make an array of all node pointers??????????
        // HMMMMMMMM?????????
        // clist_core_flatten? ???????????

        for (size_t i = 0; i < count; ++i) {
            if (mode & CLIST_EXFILTRATION) {
                NODE_GET(position.curr, data_dest, data_size);

                data_size = INCREMENT_VOID(data_dest, data_size);
            }

            if (mode & CLIST_REMOVAL) {
                if (mode == MODE_ERASE && destructor) {
                    destructor(DATA_POINTER(position.curr));
                }
                free(position.curr);
            }

            position.curr = backup;
            backup        = backup->next;
        }
        return true;
    }
    return false;
}


// Extracts count objects from position, placing it in data_dest and removing the nodes
// False on parameter error
bool clist_core_extract(const clist_itr_t position, const size_t count, void *data_dest) {
    clist_itr_t end_node = clist_core_range_find(position, count);  // deconst isue?
    if (end_node.root) {                                            // got a result
        // Shouldn't fail past here?

        // Now the good question is: Destruct loop THEN free
        // or free AND destruct loop?
        // two loops will be less jumps if the destructor ISN'T used
        // since the whole loop is skipped
        // But then we have to traverse twice
        // But then it should be cached if there isn't a lot?
        // Probably best to just do it in one go?

        // relink and decrement count first
        // then our workspace is segmented from the list

        position.curr->prev.next = end_node.curr;
        end_node.curr->prev      = position.curr->prev;
        position.root->size -= count;
        // List is functional again!

        const size_t data_size = position->root.size;

        node_t *backup = position.curr->next;

        for (size_t i = 0; i < count; ++i) {
            NODE_GET(position.curr, data_dest, data_size);

            data_size = INCREMENT_VOID(data_dest, data_size);

            free(position.curr);
            position.curr = backup;
            backup        = backup->next;
        }
        return true;
    }
    return false;
}

// Deconstructs count objects at position and removes the nodes.
// False on parameter error
bool clist_core_deconstruct(const clist_itr_t position, const size_t count) {
    // I just added a BUNCH of checks to range_find, so our input is good pending range_find
    // if (position.root && position.curr && count) {

    clist_itr_t end_node = clist_core_range_find(position, count);  // deconst isue?
    if (end_node.root) {                                            // got a result
        // Shouldn't fail past here?

        // Now the good question is: Destruct loop THEN free
        // or free AND destruct loop?
        // two loops will be less jumps if the destructor ISN'T used
        // since the whole loop is skipped
        // But then we have to traverse twice
        // But then it should be cached if there isn't a lot?
        // Probably best to just do it in one go?

        // relink and decrement count first
        // then our workspace is segmented from the list

        position.curr->prev.next = end_node.curr;
        end_node.curr->prev      = position.curr->prev;
        position.root->size -= count;
        // List is functional again!

        node_t *backup = position.curr->next;
        void (*const destructor)(void *const) = position.root->destructor;

        for (size_t i = 0; i < count; ++i) {
            if (destructor) {
                destructor(DATA_POINTER(position.curr));
            }
            free(position.curr);
            position.curr = backup;
            backup        = backup->next;
        }
        return true;
    }
    return false;
}


// Returns an iterator to position + count (the node to relink on a node op)
//  NULL'd struct on failure
// Fails if position + count is past root, count is zero, or position points at root
clist_itr_t clist_core_range_find(clist_itr_t position, const size_t count) {
    // Forbid count of zero because it could lead to very bad/confusing things
    // So if you got to here with a count of zero, we'll catch it.

    // Just throwing in a crapton of checks
    bool valid = count && position.root && position.curr && position.root != position.curr && position.root->size
                 && count <= position.root->size;

    for (size_t i = 0; i < (count - 1) && valid; ++i) {
        position.curr = position.curr->next;
        // No need to check prev state since it MUST be true
        valid = position.root != position.curr;
    }
    // Annoying edge case, end is root because we want the last N
    // So we have to do N-1 hops above and N here
    if (valid) {
        position.curr = position.curr->next;
        return positon;
    }
    // Can't just return {NULL, NULL}, but if you cast it, it's fine
    // C types!
    return (clist_itr_t){NULL, NULL};
}

// Hunts down the requested node, NULL on parameter issue
node_t *clist_core_locate(const clist_t *const clist, const size_t position);


// DO NOT TOUCH
// It will break all of your hopes, dreams, and data integrity if you use it wrong
// It doesn't even check if these objects are all in the same list
// YOU HAVE BEEN WARNED
// ANYWAY, unlinks nodes begin and end from the list, correcting any and all links, decrements count,
//  and then starts killing all nodes from begin to end. Count is just used to correct the size
//  traversal is used to free everything. Destructor flag allows destructor override (for an extract, perhaps)
void dyn_core_purge(clist_t *const clist, node_t *begin, node_t *end, const size_t count, const bool deconstruct) {
    if (clist && begin && end && count) {
        clist->size -= count;
        // COUNT NOT VALID, LIST IN BAD STATE
        begin->prev->next = end->next;
        // BEGIN UNLINKED AND RE-ROUTED, LIST CANNOT BE BACK-TRAVERSED CORRECTLY, LIST IN BAD STATE
        end->next->prev = begin->prev;
        // END UNLINKED AND RE-ROUTED, LIST IN GOOD STATE

        // Start killing unlinked nodes
        node_t *backup = begin->next;
        do {
            if (clist->destructor && deconstruct) clist->destructor(DATA_POINTER(begin));
            free(begin);
            begin  = backup;
            backup = begin->next;
        } while (begin != end);
    }
}

// returns node pointer of requested node, NULL on bad request
node_t *dyn_core_locate(const clist_t *const clist, const size_t position) {
    node_t *itr = NULL;
    if (clist && position < clist->size) {
        // Might be a bad idea

        // This could be done with a relative offset since front/back next/prev are the same offsets
        // instead of two loops.
        // EX: offset = front_half ? 1 : 0
        // itr = ((node_t*)clist)[offset]
        // for(distance = front_half? position : size - position;distance;distance--)
        // itr = ((node_t*)itr)[offset]
        // In a perfect world where I have time to do things, I'd test which is better
        // Becuse that sounds neat, but that indexing sounds expensive vs a direct lookup
        // Screw it, let's do something cool.

        int offset = (position <= clist->size >> 1) ? 1 : 0;
        itr = ((node_t **) clist)[offset];  // itr is now either front or back node
        for (size_t distance = offset ? position : size - position - 1; distance; --distance) {
            itr = ((node_t **) itr)[offset];  // So, 1 for forward, 0 for backwards. Neat, huh?
        }

        /*
            if (position <= (clist->size >> 1)) { // Request for the front half
                itr = clist->back;
                for(size_t distance = position)
            } else { // Request for the back half

            }
        */
    }
    return itr;
}
