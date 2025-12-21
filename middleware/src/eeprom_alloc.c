#include "eeprom_alloc.h"

#define byte_index(n) ((n) / 8)
#define bit_index(n) ((n) % 8)


static int get_alloc_table(uint8_t *table, uint16_t id) {
	return (table[byte_index(id)] >> bit_index(id)) & 1;
}

static int put_alloc_table(uint8_t *table, uint16_t id, uint8_t val) {
	
}

void print_alloc_table(eeprom_directory_t *directory) {
	uint8_t *alloc_table = directory->alloc_table;
	
	for (int i = 0; i < BLOCK_COUNT; i++) {
		putchar(get_alloc_table(alloc_table, i) ? '1' : '0');

		if ((i + 1) % 64 == 0) {
			putchar('\n');
		} else if ((i + 1) % 8 == 0) {
			putchar(' ');
		}
	}
	putchar('\n');
}

uint16_t alloc_block(eeprom_directory_t *directory) {

}

void free_block(eeprom_directory_t *directory, uint16_t id) {

}

int check_availability(eeprom_directory_t *directory, uint16_t id) {

}
