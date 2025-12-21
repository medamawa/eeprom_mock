#include "eeprom_storage.h"

eeprom_status_t init_storage(eeprom_directory_t *directory) {
	m24c32_t *device = directory->device;
	directory_key_map_t *key_map = directory->key_map;
	uint8_t *data = malloc(KEY_MAP_SIZE);
	if (data == NULL) {
		return EEPROM_ERROR_ALLOCATION;
	}

	eeprom_status_t res = m24c32_read(device, KEY_MAP_BEGIN, data, KEY_MAP_SIZE);
	if (res != EEPROM_OK) {
		free(data);
		return res;
	}

	for (int i = 0; i < KEY_MAP_COUNT; i++) {
		uint8_t *entry = data + (i * KEY_MAP_STRUCT_SIZE);

		// key[4]
		memcpy(key_map[i].key, entry, 4);
		
		// ids[4]
		for (int j = 0; j < 4; j++) {
			key_map[i].ids[j] = (uint16_t)entry[4 + j * 2] | ((uint16_t)entry[4 + j * 2 + 1] << 8);
		}
	}
	free(data);
	return EEPROM_OK;
}