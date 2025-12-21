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
 * @param ids The list of block ids to be freed.
 * @param size The size of the list.
 */
void free_block(eeprom_directory_t *directory, uint16_t *ids, uint8_t size);

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
