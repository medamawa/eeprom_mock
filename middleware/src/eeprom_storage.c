#include "eeprom_storage.h"

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

eeprom_status_t get_data(eeprom_directory_t *directory, const uint16_t *ids, uint8_t *out, uint16_t *out_size) {
	uint8_t id_count = get_id_count(ids);
	*out_size = BLOCK_SIZE * id_count;
	if (out) {
		free(out);
	}
	out = malloc(*out_size);
	if (out == NULL) {
		return EEPROM_ERROR_ALLOCATION;
	}

	for (int i = 0; i < id_count; i++) {
		uint16_t id = ids[i];
		if (check_availability(directory, id) == 0) {
			free(out);
			return EEPROM_ERROR_NOT_FOUND;
		}

		uint16_t addr = get_addr_for_data(id);
		uint8_t *data_ptr = out + BLOCK_SIZE * i;
		eeprom_status_t res;

		res = m24c32_read(directory->device, addr, data_ptr, BLOCK_SIZE);
		if (res != EEPROM_OK) {
			free(out);
			return res;
		}
	}
	return EEPROM_OK;
}

eeprom_status_t put_data(eeprom_directory_t *directory, const uint16_t *ids, uint8_t *value) {
	uint8_t id_count = get_id_count(ids);

	for (int i = 0; i < id_count; i++) {
		uint16_t id = ids[i];

		uint16_t addr = get_addr_for_data(id);
		uint8_t *data_ptr = value + BLOCK_SIZE * i;
		eeprom_status_t res;

		res = m24c32_write(directory->device, addr, data_ptr, BLOCK_SIZE);
		if (res != EEPROM_OK) {
			return res;
		}
	}
	return EEPROM_OK;
}

eeprom_status_t delete_data(eeprom_directory_t *directory, uint16_t *ids) {
	uint8_t id_count = get_id_count(ids);

	free_block(directory, ids, id_count);
	return EEPROM_OK;
}
