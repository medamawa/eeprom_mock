#include "eeprom_directory.h"

static uint16_t *get_ids(const eeprom_directory_t *directory, const uint8_t *key) {
	const directory_key_map_t *key_map = directory->key_map;

	for (int i = 0; i < KEY_MAP_COUNT; i++) {
		if (strcmp(key_map[i].key, key) == 0) {
			return key_map[i].ids;
		}
	}
	return NULL;
}

static eeprom_status_t put_ids_with_key(const eeprom_directory_t *directory, const uint8_t *key, const uint16_t *ids) {
	
	return EEPROM_OK;
}

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

static eeprom_status_t put_one_block(const eeprom_directory_t *directory, const uint16_t id, const uint8_t *data) {
	
}

// return id for block
static uint16_t alloc_one_block_space(eeprom_directory_t *directory) {
	m24c32_t *device = directory->device;
	uint8_t *alloc_table = directory->alloc_table;

	for (uint16_t i = 0; i < BLOCK_COUNT; i++) {
		uint8_t byte_offset = i / 8;
		uint8_t bit_offset = i % 8;
		if ((*(alloc_table + byte_offset) & (1 << bit_offset)) == 0) {
			*(alloc_table + byte_offset) += (1 << bit_offset);
			uint16_t addr = ALLOC_TABLE_BEGIN + byte_offset;
			m24c32_write(device, addr, *(alloc_table + byte_offset), 1);
			return i;
		}
	}
	return BLOCK_COUNT;
}


eeprom_status_t directory_init(
	const m24c32_t *device,
	eeprom_directory_t *directory
) {
	directory = malloc(sizeof(eeprom_directory_t));
	if (directory == NULL) {
		return EEPROM_ERROR_ALLOCATION;
	}

	directory->device = device;
	eeprom_status_t res;
	res = m24c32_read(device, ALLOC_TABLE_BEGIN, directory->alloc_table, ALLOC_TABLE_SIZE);
	if (res != EEPROM_OK) {
		free(directory);
		return res;
	}
	res = m24c32_read(device, KEY_MAP_BEGIN, directory->key_map, KEY_MAP_SIZE);
	if (res != EEPROM_OK) {
		free(directory);
		return res;
	}
	return EEPROM_OK;
}

eeprom_status_t get_directory_value(
	eeprom_directory_t *directory,
	const uint16_t *key,
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
	out_size = BLOCK_SIZE * id_count;
	out = malloc(out_size);
	if (out == NULL) {
		return EEPROM_ERROR_ALLOCATION;
	}

	for (int i = 0; i < id_count; i++) {
		uint16_t id = ids[i];
		uint16_t addr = get_addr_for_data(id);
		uint8_t data_ptr = out + BLOCK_SIZE * id;
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
	const uint16_t *key,
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
