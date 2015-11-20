#include <stdio.h>
#include <dyn_array.h>
#include <c_map.h>

#define MAX_STR_LEN 50


int main (void) {
	char* key; 
	char* value; 
	c_map_t* map = c_map_create();

	FILE* fp = fopen("dataset.txt","r");
	if(!fp) {
		fputs("FAILURE TO OPEN FILE\n",stdout);
		return -1;
	}
	for (int i = 0; i < 10; ++i)  {
		key = calloc(MAX_STR_LEN,sizeof(char));
		value = calloc(MAX_STR_LEN, sizeof(char));
		fscanf(fp,"%s %s", key, value);
		key[MAX_STR_LEN - 1] = 0;
		value[MAX_STR_LEN - 1] = 0;
		if(!c_map_insert(map,&key[0],&value[0])) {
			fputs("BROKED C_MAP INSERT\n",stdout);
		}

	}

	dyn_array_t* keys = c_map_get_keys(map);
	
	for (int i = 0; i < dyn_array_size(keys); ++i) {	
		fprintf(stdout,"key: %s\n",*((char**) dyn_array_at(keys,i)));	
	}
	for (int i = 0; i < dyn_array_size(keys); ++i) {
		dyn_array_t* values = c_map_get_values(map,*((char**) dyn_array_at(keys,i)));
		for (int i = 0; i < dyn_array_size(values); ++i) {
			fprintf(stdout,"value: %s\n",*((char**) dyn_array_at(values,i)));
			free(*((char**) dyn_array_at(values,i)));
		}
		dyn_array_destroy(values);
		free(*((char**) dyn_array_at(keys,i)));
	}
	dyn_array_destroy(keys);

	c_map_destroy(map);
	fclose(fp);
	
}
