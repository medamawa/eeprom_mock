/**
 * @file eeprom_directory.h
 * @brief EEPROM Directory Management
 * @author Sogo Nishihara
 * 
 * This file provides functions to initialize and manage an EEPROM directory with partitions.
 * Functions return `eeprom_status_t` error codes, which are defined in `eeprom_status.h`.
 * 
 */

#ifndef EEPROM_DIRECTORY_H
#define EEPROM_DIRECTORY_H

#define KEY_MAP_STRUCT_SIZE 12
#define KEY_MAP_COUNT 128
#define BLOCK_SIZE 4
#define BLOCK_COUNT 512
#define ALLOC_TABLE_BEGIN 0
#define ALLOC_TABLE_SIZE (BLOCK_COUNT / 8)
#define KEY_MAP_BEGIN (ALLOC_TABLE_BEGIN + ALLOC_TABLE_SIZE)
#define KEY_MAP_SIZE (KEY_MAP_STRUCT_SIZE * KEY_MAP_COUNT)
#define DATA_SPACE_BEGIN (KEY_MAP_BEGIN + KEY_MAP_SIZE)
#define EEPROM_SIZE 4096

#include <stdlib.h>

#include "m24c32.h"
#include "eeprom_status.h"

typedef struct {
	uint8_t		key[4];
	uint16_t	ids[4];
} directory_key_map_t;

typedef struct {
	m24c32_t			*device;
	directory_key_map_t key_map[KEY_MAP_COUNT];
	uint8_t				alloc_table[16];
} eeprom_directory_t;


eeprom_status_t directory_init(
	const m24c32_t *device,
	eeprom_directory_t *directory
);

eeprom_status_t get_directory_value(
	eeprom_directory_t *directory,
	const uint16_t *key,
	uint8_t *out,
	uint16_t *out_size
);

eeprom_status_t set_directory_value(
	eeprom_directory_t *directory,
	const uint16_t *key,
	const uint8_t *value,
	const uint16_t value_size
);

eeprom_status_t delete_directory_value(
	eeprom_directory_t *directory,
	const uint16_t *key
);


#endif // EEPROM_DIRECTORY_H