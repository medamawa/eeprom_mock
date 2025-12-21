#include "eeprom_alloc.h"

#define byte_index(n) ((n) / 8)
#define bit_index(n) ((n) % 8)


static int get_alloc_table(uint8_t *table, uint16_t id) {
	return (table[byte_index(id)] >> bit_index(id)) & 1;
}

static void put_alloc_table(uint8_t *table, uint16_t id, uint8_t val) {
	uint8_t bit_mask = nth_bit_mask(bit_index(id));

	if (val) {
		table[byte_index(id)] |= bit_mask;
	} else {
		bit_mask = ~bit_mask;
		table[byte_index(id)] &= bit_mask;
	}
}

static eeprom_status_t update_eeprom_alloc_table(eeprom_directory_t *directory, uint16_t id) {
	uint16_t data_index = byte_index(id);
	uint16_t addr = ALLOC_TABLE_BEGIN + data_index;
	uint8_t *data = directory->alloc_table + data_index;

	return m24c32_write(directory->device, addr, data, 1);
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
	m24c32_t *device = directory->device;
	uint8_t *alloc_table = directory->alloc_table;

	for (uint16_t id = 0; id < BLOCK_COUNT; id++) {
		if (get_alloc_table(alloc_table, id) == 0) {
			put_alloc_table(alloc_table, id, 1);
			eeprom_status_t ret = update_eeprom_alloc_table(directory, id);
			if (ret != EEPROM_OK) {
				put_alloc_table(alloc_table, id, 0);
				return BLOCK_SIZE;
			}
			return id;
		}
	}
	return BLOCK_COUNT;
}

void free_block(eeprom_directory_t *directory, uint16_t id) {

}

int check_availability(eeprom_directory_t *directory, uint16_t id) {

}
