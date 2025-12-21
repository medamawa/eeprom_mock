#ifndef EEPROM_DIRECTORY_STRUCT
#define EEPROM_DIRECTORY_STRUCT

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

#include <stdint.h>

#include "m24c32.h"

typedef struct {
	uint8_t		key[4];
	uint16_t	ids[4];
} directory_key_map_t;

typedef struct {
	m24c32_t			*device;
	uint8_t				alloc_table[ALLOC_TABLE_SIZE];
	directory_key_map_t key_map[KEY_MAP_COUNT];
} eeprom_directory_t;

#endif
