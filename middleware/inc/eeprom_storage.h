#ifndef EEPROM_STORAGE
#define EEPROM_STORAGE

#include "eeprom_directory_struct.h"

/**
 * @brief Gets key-map table from eeprom memory.
 * 
 * @param directory 
 * @return eeprom_status_t 
 */
eeprom_status_t init_storage(eeprom_directory_t *directory);

eeprom_status_t get_data(eeprom_directory_t *directory, uint16_t *ids, uint8_t size);

eeprom_status_t put_data(eeprom_directory_t *directory);

#endif
