#include "eeprom_directory.h"

static uint16_t *get_ids(eeprom_directory_t *directory, const uint8_t *key) {
	directory_key_map_t *key_map = directory->key_map;

	for (int i = 0; i < KEY_MAP_COUNT; i++) {
		if (strcmp((char *)key_map[i].key, (char *)key) == 0) {
			return key_map[i].ids;
		}
	}
	return NULL;
}

static eeprom_status_t set_key(eeprom_directory_t *directory, const uint8_t *key, uint16_t *ids) {
	m24c32_t *device = directory->device;
	directory_key_map_t *key_map = directory->key_map;

	for (int i = 0; i < KEY_MAP_COUNT; i++) {
		if ((char)key_map[i].key[0] == '\0') {
			memcpy(key_map[i].key, key, 4);
			memcpy(key_map[i].ids, ids, 8);
			
			uint16_t addr = KEY_MAP_BEGIN + i * KEY_MAP_STRUCT_SIZE;
			return m24c32_write(device, addr, (uint8_t *)&key_map[i], KEY_MAP_STRUCT_SIZE);
		}
	}
	return EEPROM_ERROR_ALLOCATION;
}


eeprom_status_t directory_init(
	m24c32_t *device,
	eeprom_directory_t *directory
) {
	directory = malloc(sizeof(eeprom_directory_t));
	if (directory == NULL) {
		return EEPROM_ERROR_ALLOCATION;
	}

	directory->device = device;
	eeprom_status_t res;
	res = init_alloc_table(directory);
	if (res != EEPROM_OK) {
		free(directory);
		return res;
	}
	res = init_storage(directory);
	if (res != EEPROM_OK) {
		free(directory);
		return res;
	}
	return EEPROM_OK;
}

eeprom_status_t get_directory_value(
	eeprom_directory_t *directory,
	const uint8_t *key,
	uint8_t *out,
	uint16_t *out_size
) {
	if (directory == NULL || key == NULL) {
		return EEPROM_ERROR_NULL_POINTER;
	}

	uint16_t *ids = get_ids(directory, key);
	if (ids == NULL) {
		return EEPROM_ERROR_NOT_FOUND;
	}

	return get_data(directory, ids, out, out_size);
}

eeprom_status_t set_directory_value(
	eeprom_directory_t *directory,
	const uint8_t *key,
	const uint8_t *value,
	const uint16_t value_size
) {
	if (directory == NULL || value == NULL) {
		return EEPROM_ERROR_NULL_POINTER;
	}
	if (value_size <= 0) {
		return EEPROM_ERROR;
	}

	uint16_t *ids = get_ids(directory, key);
	if (ids == NULL) {
		int block_count = (value_size - 1) / BLOCK_SIZE + 1;
		if (block_count > 4) {
			return EEPROM_ERROR_TOO_BIG;
		}
		ids = malloc(sizeof(uint16_t) * 4);
		if (ids == NULL) {
			return EEPROM_ERROR_ALLOCATION;
		}
		for (int block_idx = 0; block_idx < block_count; block_idx++) {
			uint16_t id = alloc_block(directory);
			if (id == BLOCK_COUNT) {
				free(ids);
				return EEPROM_ERROR_ALLOCATION;
			}
			ids[block_idx] = id;
		}

		eeprom_status_t res;
		res = set_key(directory, key, ids);
		if (res != EEPROM_OK) {
			free(ids);
			return res;
		}
		res = put_data(directory, ids, value, value_size);
		if (res != EEPROM_OK) {
			free(ids);
			return res;
		}
	} else {
		printf("already set\n");
		delete_directory_value(directory, key);
		set_directory_value(directory, key, value, value_size);
		return EEPROM_ERROR;
	}
	return EEPROM_OK;
}
