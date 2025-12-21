#ifndef EEPROM_ALLOC_H
#define EEPROM_ALLOC_H

#include "eeprom_directory.h"

/**
 * @brief 
 * 
 * @param directory 
 */
void print_alloc_table(eeprom_directory_t *directory);

/**
 * @brief 
 * 
 * This allocates one block space according to alloc_table in local memory,
 * and updates allo_table in eeprom memory.
 * 
 * @param directory 
 * @return The allocated block id. return BLOCK_SIZE if failed to allocate.
 */
uint16_t alloc_block(eeprom_directory_t *directory);

/**
 * @brief 
 * 
 * This will do nothing if the block is already freed.
 * 
 * @param directory 
 * @param id The id of block to be freed.
 */
void free_block(eeprom_directory_t *directory, uint16_t id);

/**
 * @brief 
 * 
 * @param directory 
 * @param id 
 * @return 1 if the specified block is allocated.
 *         0 otherwise.
 */
int check_availability(eeprom_directory_t *directory, uint16_t id);

#endif
