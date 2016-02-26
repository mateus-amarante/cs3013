#include <stdio.h>
#include <stdint.h>

#include "memory.h"

void memoryMaxer(int n) {
	
	int i;
	int valid;
	
	vAddr indexes[n];
	
	for (i = 0; i < n; ++i) {
		indexes[i] = create_page();
		uint32_t value = get_value(indexes[i], &valid);
		printf("valid=%d value=%d\n", valid, value);
		value = (i * 3);
		store_value(indexes[i], &value);
		print_page_table();
		print_mem_maps();
		print_memory_all();
	}
	
	for (i = 0; i < n; ++i) {
		free_page(indexes[i]);
	}
	
	print_page_table();
	print_mem_maps();
	print_memory_all();
}

int main() {
	
	init_memory(RANDOM_EVICT, 2);
	//memoryMaxer(20);
	
	vAddr address[15];
	int i, valid;
	uint32_t value;
	
	for (i = 0; i < 16; i++){
		address[i] = create_page();
		value = (i+1)*2;
		store_value(address[i], &value);
		print_page_table();
		print_mem_maps();
		print_memory_all();
	}
	free_page(2);
	free_page(14);
	free_page(7);
	print_page_table();
	print_mem_maps();
	print_memory_all();/* */
	getchar();
	
	//value = 789;
	printf("got %d\n",get_value(address[10], &valid));
	print_page_table();
	print_mem_maps();
	print_memory_all();
	printf("got %d\n",get_value(address[0], &valid));
	print_page_table();
	print_mem_maps();
	print_memory_all();
	printf("got %d\n",get_value(address[6], &valid));
	print_page_table();
	print_mem_maps();
	print_memory_all();

	destroy_memory();
	
	return 0;
}
