#include "../include/clist.h"

#include <stdint.h>

typedef struct node {
    node_t *prev, next; // ORDER AND LOCATION IS VITAL, DO NOT REORDER THESE (make it an array of 2?)
} node_t;

struct clist {
    node_t *back, front; // ORDER AND LOCATION IS VITAL, DO NOT REORDER THESE (make it an array of 2?)
    size_t size;
    size_t data_size;
    void (*destructor)(void *const);
};

// it's a list. It benefits from an iterator. The number of functions just doubled. Ugh.
struct clist_itr {
    node_t *root, curr;
};

// Since I can't scan my notes and attach it here...
/*
    Root connects to the two ends, BUT THE TWO ENDS ALSO CONNECT TO ROOT. It makes relinking faster, and NO LINK IS EVER NULL
    Root, when empty, should point to itself at both ends. PREFER SIZE CHECK TO LINK CHECK
        Cores will prevent circular link issues with size checks before starting, complex functions should do the same

    There is a VITAL test in the test folder. It will assert that the relative offset of root's back and front are the same
    as the node relative offsets. This way, root can be cast to a node and not cause a freakout, and that the circular link is fine
    // TODO: Find a way to force cmake to run this test on install

    Not going to store the size and destructor in the nodes because that's a major waste of space, though it is a little more restrictive
    The list-who-shall-not-be-named (hetero_list) will be interesting and allow varying size and destructors
    (does that make this homo_list? It would be a better name than dyn_list...)
*/


// CORE FUNCTIONS. They are what they sound like, core functions.
// They do intense checks, so if you're a thin wrapper, do only what needs to be done to construct your core call

// Does what it sounds like, inserts count objects from data_src after position
// False on parameter or malloc failure
bool clist_core_insert(clist_t *const clist, const clist_itr_t position, const size_t count, const size_t data_size, const void *const data_src);

// Extracts count objects from position, placing it in data_dest and removing the nodes
// False on parameter error
bool clist_core_extract(clist_t *const clist, const clist_itr_t position, const size_t count, void *data_dest);

// Deconstructs count objects at position and removes the nodes.
// False on parameter error
bool clist_core_deconstruct(clist_t *const clist, const clist_itr_t position, const size_t count);

// Hunts down the requested node, NULL on parameter issue
node_t *clist_core_locate(const clist_t *const clist, const size_t position);

/*
    Considering hiding even dyn_core_locate down below, making EVERYTHING a thin wrapper to a dyn_core
    But the complex functions (sort, prune, map/transform) would end up just being a wrapper of the dyn_core version
        Which seems like a dumb level of indirection

    So, for now, just call DATA_POINTER on the locate... once you've checked locate didn't NULL on you
        Maybe a locate_data would be better, even if it's just sticking the two together (inline!)
        So what's better, a function call to a function call, or duplicate functions? Ugh, neither?

    // Hunts down the data pointer at the requested node, NULL on parameter issue
    void *dyn_core_locate_data(const dyn_list_t *const dyn_list, const size_t position);
*/

// This should be the only creation function
clist_t *clist_create(const size_t data_type_size, void (*destruct_func)(void *const)) {
    clist_t clist = NULL;
    if (data_size) {
        clist = malloc(sizeof(clist_t));
        if (clist) {
            clist->back = clist;
            clist->front = clist;
            clist->size = 0;
            clist->data_size = data_size;
            clist->destructor = destruct_func;
        }
    }
    return clist;
}

clist_t *clist_import(const void *const data, const size_t count, const size_t data_type_size, void (*destruct_func)(void *)) {
    clist_t clist = clist_create(data_type_size, destruct_func);
    if (clist) {
        if (dyn_core_insert(clist, 0, count, data)) {
            return clist;
        }
        clist_destroy(clist);
    }
    return NULL;
}


bool dyn_array_export(const dyn_array_t *const dyn_array, void *data) {
    // Uhh, I guess this counts as a "complex" function since dyn_core can't really save us here
    //   Unless we extract, killing the list, and then do an import and an insert, haha
    if (dyn_array && data && dyn_array->size) {
        // extra statement? Yes, but it allows the compiler to make (valid) assumptions
        //  But we have optimizations off anyway... That should be changed for "working" libraries
        // TODO: Tinker with optimizations in the libraries, only have the testers be O0'd?
        /*
            Whoops, just realized dyn_array is const'd. This shouldn't be needed?
            TODO: Check assembly for const optimization on loop condition
            const size_t size = dyn_array->size;
            const size_t data_size = dyn_array->data_size;
        */
        // Ok, what do? make data a uint8_t or leave it void?
        // Having it be void is nice because who cares about the type
        // But having it be a byte is nice because pointer math on void is a no-no and that's annoying as hell
        // and having a duplicate pointer that is just it but casted is dumb
        // DECISION: Implicit casting to void isn't a warning, but implicit casting of something to something else is
        // Hell, I mght just make a macro that increments a void pointer by n bytes
        node_t *itr = dyn_array->front;
        for (size_t count = 0; count < size; ++count,
                data = (void *)(((uint8_t *)data) + dyn_array->data_size), // THIS IS DUMB AND I HATE IT >:C
                itr = itr->next) {
            memcpy(data, DATA_POINTER(itr), dyn_array->data_size);
        }
    }
}

void clist_destroy(clist_t *const clist) {
    if (clist) {
        dyn_core_deconstruct(clist, clist->size);
        // deconstruct only fails on NULL, or zero size
        // and neither of those are a threat to free
        free(clist);
    }
}


//
///
// HERE BE DRAGONS
///
//


// Because nodes are getting weird and I love them
#define NODE_CALLOC(data_size) ((node_t *) calloc(1, sizeof(node_t) + (data_size)))

// So, if node gets padded somehow, we'll avoid the disaster case of us just going crazy
//  but we will waste the padded space ...but isn't padding meant to be wasted?
#define DATA_POINTER(node_ptr) ((void *) ((((node_t *) (node_ptr)) + 1)))

#define NODE_SET(node_ptr, data_ptr, data_size) (memcpy(DATA_POINTER(node_ptr), (data_ptr), (data_size)))

#define NODE_GET(node_ptr, data_ptr, data_size) (memcpy((data_ptr), DATA_POINTER(node_ptr), (data_size)))

// Such a pain
#define INCREMENT_VOID(ptr, value) ((void *) (((uint8_t *) (ptr)) + value))

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
bool clist_core_insert(clist_t *const clist, const clist_itr_t position, const size_t count, const size_t data_size, const void *data_src) {
    // well, at least the == can try to see if we're working in the right object
    if (clist && position.root && position.next && position.root == clist && count && data_size && data_src) {
        node_t **new_nodes = clist_core_allocate(count, data_size);
        if (new_nodes) {
            // We can no longer fail, woo!
            node_t *cur_ptr = position.curr;

            cur_ptr->next->prev = new_nodes[count - 1];
            cur_ptr->next = new_nodes[0];
            clist->size += count;
            // Linking complete, but the data's not there yet :/

            for (size_t i = 0; i < count; ++i, data_src = INCREMENT_VOID(data_src, data_size),
                    cur_ptr = cur_ptr->next) {
                NODE_SET(new_nodes[i], data_src, data_size);
            }
            free(new_nodes);
            // Yay!
            return true;
        }
    }
    return false;
}

// Extracts count objects from position, placing it in data_dest and removing the nodes
// False on parameter error
bool clist_core_extract(clist_t *const clist, const clist_itr_t position, const size_t count, void *data_dest);

// Deconstructs count objects at position and removes the nodes.
// False on parameter error
bool clist_core_deconstruct(clist_t *const clist, const clist_itr_t position, const size_t count);

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
            if (clist->destructor && deconstruct)
                clist->destructor(DATA_POINTER(begin));
            free(begin);
            begin = backup;
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
        itr = ((node_t **)clist)[offset]; // itr is now either front or back node
        for (size_t distance = offset ? position : size - position - 1;
                distance;
                --distance) {
            itr = ((node_t **)itr)[offset]; // So, 1 for forward, 0 for backwards. Neat, huh?
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




