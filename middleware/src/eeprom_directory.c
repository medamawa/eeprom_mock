#include "eeprom_directory.h"

eeprom_status_t directory_init(
	const m24c32_t *device,
	eeprom_directory_t *directory
) {
	directory = malloc(sizeof(eeprom_directory_t));
	if (directory == NULL) {
		return EEPROM_ERROR;
	}

	directory->device = device;
	eeprom_status_t res;
	res = m24c32_read(device, ALLOC_TABLE_BEGIN, directory->alloc_table, ALLOC_TABLE_SIZE);
	if (res != EEPROM_OK) {
		return res;
	}
	res = m24c32_read(device, KEY_MAP_BEGIN, directory->key_map, KEY_MAP_SIZE);
	if (res != EEPROM_OK) {
		return res;
	}
	return EEPROM_OK;
}


