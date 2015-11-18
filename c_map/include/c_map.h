#include <dyn_array.h>
#include <stdbool.h>

typedef struct c_map c_map_t;


///
///	Creates a new c_map
/// \return pointer to a new c_map, NULL on error
///
c_map_t *c_map_create();


///
/// Destroys a c_map
///  Note: This will invalidate all pointers to map data
/// \param c_map the map to destroy
///
void c_map_destroy(c_map_t *const c_map);


///
/// Creates a list of the current keys
///  (a dyn_array of c-strings)
///  !!! WARNING !!! INTERNAL POINTERS, DO NOT MODIFY
///  (but do destroy the dy_array, that's your problem now)
/// \param c_map the map to analyze
/// \return dyn_array of keys in the map (c-strings)
///
dyn_array_t *c_map_get_keys(const c_map_t *const c_map);


///
/// Creates a list of values associated with the given key
/// \param c_map The c_map to check
/// \param key The key to check
/// \return A dyn_array of values, NULL on error, key does not exist, or no values for given key
///
dyn_array_t *c_map_get_values(const c_map_t *const c_map, const char *const key);

///
/// Inserts a single value into the c_map for the given key
/// \param c_map The c_map to insert into
/// \param key The key for the key-value pair
/// \param value The value for the key-value pair
/// \return bool indicating success of operation
///
bool c_map_insert(c_map_t *const c_map, const char *const key, const char * const value);
