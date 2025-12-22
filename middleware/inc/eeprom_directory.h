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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "m24c32.h"
#include "eeprom_status.h"
#include "eeprom_directory_struct.h"
#include "eeprom_alloc.h"
#include "eeprom_storage.h"

eeprom_status_t directory_init(
	m24c32_t *device,
	eeprom_directory_t *directory
);

eeprom_status_t get_directory_value(
	eeprom_directory_t *directory,
	const uint8_t *key,
	uint8_t **out,
	uint16_t *out_size
);

eeprom_status_t set_directory_value(
	eeprom_directory_t *directory,
	const uint8_t *key,
	uint8_t *value,
	const uint16_t value_size
);

eeprom_status_t delete_directory_value(
	eeprom_directory_t *directory,
	const uint8_t *key
);


#endif // EEPROM_DIRECTORY_H