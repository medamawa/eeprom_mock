/**
 * @file eeprom_status.h
 * @brief EEPROM Standardized Error Codes
 * 
 * This file defines the `eeprom_status_t` enumeration, which provides standardized 
 * error codes for all EEPROM-related operations.
 * 
 */

#ifndef EEPROM_STATUS
#define EEPROM_STATUS

/**
 * @brief EEPROM operation status codes.
 * 
 * This enumeration provides standardized error codes for all EEPROM-related operations.
 * Negative values indicate errors, while 0 indicates success.
 */
typedef enum {
	EEPROM_OK = 0,						/**< Operation completed successfully. */
	EEPROM_ERROR = -1,					/**< General error occurred. */
	EEPROM_ERROR_NOT_FOUND = -2,		/**< Requested key or resource not found. */
	EEPROM_ERROR_OUT_OF_BOUNDS = -3,	/**< Address or index out of valid range. */
	EEPROM_ERROR_ALIGNMENT = -4,		/**< Address alignment error. */
	EEPROM_ERROR_NULL_POINTER = -5,		/**< Null pointer passed where non-null required. */
	EEPROM_ERROR_ALLOCATION = -6,		/**< Memory or block allocation failed. */
	EEPROM_ERROR_COMMS = -7,			/**< Communication error with EEPROM device. */
	EEPROM_ERROR_TOO_BIG = -8			/**< Data size exceeds maximum allowed size. */

} eeprom_status_t;

#endif