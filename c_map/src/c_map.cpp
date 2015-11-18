
extern "C" {
#include "../include/c_map.h"
}
#include <string>
#include <map>
#include <vector>
#include <exception>
#include <new>
#include <iostream>

// Haha, got ya. It's just a dummy so I can get a named pointer type.
struct c_map {};

// I am not typing this ever again
using map_type = std::map<std::string, std::vector<std::string>>;

// UGH!
#define MAP_POINTER_CAST(c_map_ptr) ((map_type *)(c_map_ptr))

//
// Creates a new c_map
// return pointer to a new c_map, NULL on error
//
c_map_t *c_map_create() {
    map_type *c_map = new(std::nothrow) map_type;
    return (c_map_t *) c_map;
}


//
// Destroys a c_map
//  Note: This will invalidate all pointers to map data
// param c_map the map to destroy
//
void c_map_destroy(c_map_t *const c_map) {
    try {
        delete c_map;
    } catch (std::exception &e) {
        std::cerr << "WAY2GO: " << e.what() << std::endl;
    }
}


//
// Creates a list of the current keys
//  (a dyn_array of c-strings)
//  !!! WARNING !!! INTERNAL POINTERS, DO NOT MODIFY
//  (but do destroy the dy_array, that's your problem now)
// param c_map the map to analyze
// return dyn_array of keys in the map (c-strings)
//
dyn_array_t *c_map_get_keys(const c_map_t *const c_map) {
    try {
        dyn_array_t *key_array = NULL;
        if (c_map && MAP_POINTER_CAST(c_map)->size()) {
            key_array = dyn_array_create(MAP_POINTER_CAST(c_map)->size(), sizeof(char *), NULL);
            if (key_array) {
                for (auto &kv_pair : *MAP_POINTER_CAST(c_map)) {
                    // It doesn't like me taking the address of an rvalue, so...
                    const char *key_ptr = kv_pair.first.c_str();
                    if (!dyn_array_push_back(key_array, &key_ptr)) {
                        // dyn_array broke somehow
                        dyn_array_destroy(key_array);
                        return NULL;
                    }
                }
            }
        }
        return key_array;
    } catch (std::exception &e) {
        std::cerr << "WAY2GO: " << e.what() << std::endl;
        return NULL;
    }
}


//
// Creates a list of values associated with the given key
// param c_map The c_map to check
// param key The key to check
// return A dyn_array of values, NULL on error, key does not exist, or no values for given key
//
dyn_array_t *c_map_get_values(const c_map_t *const c_map, const char *const key) {
    try {
        dyn_array_t *value_array = NULL;
        if (c_map && key) {
            map_type::const_iterator values = MAP_POINTER_CAST(c_map)->find(std::string(key));
            if (values != MAP_POINTER_CAST(c_map)->cend() && values->second.size()) {
                value_array = dyn_array_create(values->second.size(), sizeof(char *), NULL);
                if (value_array) {
                    for (auto &value_strings : values->second) {
                    	// Once again, it knows I'm up to something weird
                    	const char *value_ptr = value_strings.c_str();
                        if (!dyn_array_push_back(value_array, &value_ptr)) {
                            // dyn_array broke somehow
                            dyn_array_destroy(value_array);
                            return NULL;
                        }
                    }
                }
            }
        }
        return value_array;
    } catch (std::exception &e) { // Emergency please-don't-kill-the-program failsafe
        // If you managed to get something thrown, you REALLY messed up
        std::cerr << "WAY2GO: " << e.what() << std::endl;
        return NULL;
    }
}

//
// Inserts a single value into the c_map for the given key
// param c_map The c_map to insert into
// param key The key for the key-value pair
// param value The value for the key-value pair
// return bool indicating success of operation
//
bool c_map_insert(c_map_t *const c_map, const char *const key, const char *const value) {
    try {
        if (c_map && key && value) {
            (*MAP_POINTER_CAST(c_map))[std::string(key)].emplace_back(value);
            return true;
        }
        return false;
    } catch (std::exception &e) { // Emergency please-don't-kill-the-program failsafe
        // If you managed to get something thrown, you REALLY messed up
        std::cerr << "WAY2GO: " << e.what() << std::endl;
        return false;
    }
}
