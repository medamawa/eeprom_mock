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

// static eeprom_status_t put_ids_with_key(const eeprom_directory_t *directory, const uint8_t *key, const uint16_t *ids) {
	
// 	return EEPROM_OK;
// }

static uint8_t get_id_count(const uint16_t *ids) {
	uint8_t count = 0;

	for (int i = 0; i < 4; i++) {
		if (ids[i] >= BLOCK_COUNT) {
			break;
		}
		count++;
	}
	return count;
}


static uint16_t get_addr_for_data(const uint16_t id) {
	return DATA_SPACE_BEGIN + id * BLOCK_SIZE;
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
	// TODO: check ids' availability
	uint8_t id_count = get_id_count(ids);

	out_size = malloc(sizeof(uint16_t));
	if (out_size == NULL) {
		return EEPROM_ERROR_ALLOCATION;
	}
	*out_size = BLOCK_SIZE * id_count;
	out = malloc(*out_size);
	if (out == NULL) {
		return EEPROM_ERROR_ALLOCATION;
	}

	for (int i = 0; i < id_count; i++) {
		uint16_t id = ids[i];
		uint16_t addr = get_addr_for_data(id);
		uint8_t *data_ptr = out + BLOCK_SIZE * id;
		eeprom_status_t res;

		res = m24c32_read(directory->device, addr, data_ptr, BLOCK_SIZE);
		if (res != EEPROM_OK) {
			free(out);
			free(out_size);
			return res;
		}
	}
	return EEPROM_OK;
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
		ids = malloc(sizeof(uint16_t) * block_count);
		if (ids == NULL) {
			return EEPROM_ERROR_ALLOCATION;
		}
		for (int block_idx = 0; block_idx < block_count; block_idx++) {
			uint16_t id = alloc_one_block_space(directory);
			if (id == BLOCK_COUNT) {
				free(ids);
				return EEPROM_ERROR_ALLOCATION;
			}
			ids[block_idx] = id;

		}
		eeprom_status_t res = put_ids_with_key(directory, key, ids);
		if (res != EEPROM_OK) {
			free(ids);
			return res;
		}

	} else {
		printf("already set\n");
		return EEPROM_ERROR;
	}

}
