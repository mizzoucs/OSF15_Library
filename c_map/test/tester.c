#include <stdio.h>
#include <dyn_array.h>
#include "../include/c_map.h"

#define MAX_STR_LEN 50


int main(void) {
    char *key;
    char *value;
    c_map_t *map = c_map_create();
	
    FILE *fp = fopen("dataset.txt", "r");
	
    if (!fp) {
        fputs("FAILURE TO OPEN FILE, create a test file of 10 entries with 2 columns of data called dataset.txt\n", stdout);
        return -1;
    }
    key = calloc(MAX_STR_LEN, sizeof(char));
    value = calloc(MAX_STR_LEN, sizeof(char));
    for (int i = 0; i < 10; ++i)  {
        fscanf(fp, "%s %s", key, value);
        key[MAX_STR_LEN - 1] = 0;
        value[MAX_STR_LEN - 1] = 0;
        if (!c_map_insert(map, key, value)) {
            fputs("BROKED C_MAP INSERT\n", stdout);
        }
    }
    free(key);
    free(value);

    dyn_array_t *keys = c_map_get_keys(map);

    for (int i = 0; i < dyn_array_size(keys); ++i) {
        fprintf(stdout, "key: %s\n", *((char **) dyn_array_at(keys, i)));
        dyn_array_t *values = c_map_get_values(map, *((char **) dyn_array_at(keys, i)));
        for (int i = 0; i < dyn_array_size(values); ++i) {
            fprintf(stdout, "value: %s\n", *((char **) dyn_array_at(values, i)));
        }
        dyn_array_destroy(values);
    }
    dyn_array_destroy(keys);

    c_map_destroy(map);
    fclose(fp);

}
