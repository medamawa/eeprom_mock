# EEPROM Directory API Usage Guide

## Overview

The EEPROM Directory API provides a simple interface for managing a key-value directory stored on an EEPROM device. This API allows you to persistently store, retrieve, and delete values associated with fixed-size (4-byte) keys, with values up to 16 bytes in size.

### Key Features

- **Key-Value Storage**: Store 4-byte keys with values up to 16 bytes
- **Persistence**: Data is stored on EEPROM and persists across power cycles
- **Block-Based Management**: Data is managed in 4-byte blocks
- **Automatic Memory Management**: Block allocation and deallocation are handled automatically

## Inputs and Outputs

### Inputs

- **Key**: A fixed-size 4-byte key (not a null-terminated string)
    - Example: ```"tsms"```, ```"dcnt"```, ```"name"```
- **Value**: Data up to 16 bytes (4 blocks) in size
    - Example: ```21```, ```"sogo"```
- **M24C32 Device**: Pointer to the EEPROM device interface
    - ```m24c32_t```

### Outputs

- **Status Code**: Error code of type `eeprom_status_t`
  - `EEPROM_OK`: Success
  - `EEPROM_ERROR_NOT_FOUND`: Key not found
  - `EEPROM_ERROR_ALLOCATION`: Memory allocation failed
  - `EEPROM_ERROR_TOO_BIG`: Data size exceeds 16 bytes
  - Other error codes (see `eeprom_status.h`)

- **Retrieved Data**: For `get_directory_value()` function, a pointer to a dynamically allocated buffer *(caller must free)*

## API Functions

### 1. `directory_init()` - Initialize Directory

Initializes the directory structure and loads the allocation table and key map from EEPROM.

**Function Signature:**
```c
eeprom_status_t directory_init(m24c32_t *device, eeprom_directory_t *directory);
```

**Parameters:**
- `device`: Pointer to the M24C32 device interface
- `directory`: Pointer to the directory structure to initialize (must be allocated by caller)

**Return Values:**
- `EEPROM_OK`: Initialization successful
- `EEPROM_ERROR_NULL_POINTER`: Parameter is NULL
- `EEPROM_ERROR`: Initialization failed

### 2. `set_directory_value()` - Store Value

Stores data with the specified key. If the key already exists, the old value is deleted first before storing the new value.

**Function Signature:**
```c
eeprom_status_t set_directory_value(eeprom_directory_t *directory,
                                    const uint8_t *key, 
                                    uint8_t *value,
                                    const uint16_t value_size);
```

**Parameters:**
- `directory`: Pointer to the initialized directory structure
- `key`: Pointer to a 4-byte key
- `value`: Pointer to the data to store
- `value_size`: Size of the data in bytes (maximum 16 bytes)

**Return Values:**
- `EEPROM_OK`: Store successful
- `EEPROM_ERROR_NULL_POINTER`: Parameter is NULL
- `EEPROM_ERROR_TOO_BIG`: Data size exceeds 16 bytes
- `EEPROM_ERROR_ALLOCATION`: Block allocation failed

### 3. `get_directory_value()` - Retrieve Value

Retrieves data associated with a key. The output buffer is dynamically allocated by this function and must be freed by the caller.

**Function Signature:**
```c
eeprom_status_t get_directory_value(eeprom_directory_t *directory,
                                    const uint8_t *key, 
                                    uint8_t **out,
                                    uint16_t *out_size);
```

**Parameters:**
- `directory`: Pointer to the initialized directory structure
- `key`: Pointer to a 4-byte key
- `out`: Pointer to a pointer that will receive the allocated output buffer
- `out_size`: Pointer to store the size of the output data in bytes

**Return Values:**
- `EEPROM_OK`: Retrieve successful
- `EEPROM_ERROR_NULL_POINTER`: Parameter is NULL
- `EEPROM_ERROR_NOT_FOUND`: Key does not exist
- `EEPROM_ERROR_ALLOCATION`: Memory allocation failed

**Note:** The retrieved buffer must be freed using `free()`.

### 4. `delete_directory_value()` - Delete Value

Deletes a key and its associated data, freeing the related blocks.

**Function Signature:**
```c
eeprom_status_t delete_directory_value(eeprom_directory_t *directory,
                                       const uint8_t *key);
```

**Parameters:**
- `directory`: Pointer to the initialized directory structure
- `key`: Pointer to a 4-byte key

**Return Values:**
- `EEPROM_OK`: Delete successful
- `EEPROM_ERROR_NULL_POINTER`: Parameter is NULL
- `EEPROM_ERROR_NOT_FOUND`: Key does not exist

## Usage Example: Car Fault Directory

The following examples demonstrate how to store and retrieve car fault information using the EEPROM directory.

### Example 1: Initialize Directory and Write Fault Information

```c
#include "eeprom_directory.h"
#include "m24c32.h"

// Fault code definitions (4-byte keys)
#define FAULT_KEY_ENGINE_TEMP    {0x01, 0x00, 0x00, 0x01}  // Engine temperature fault
#define FAULT_KEY_BRAKE_PRESSURE {0x01, 0x00, 0x00, 0x02}  // Brake pressure fault
#define FAULT_KEY_BATTERY_VOLT   {0x01, 0x00, 0x00, 0x03}  // Battery voltage fault

// Fault record structure (maximum 16 bytes)
typedef struct {
    uint32_t timestamp;      // Fault occurrence time (4 bytes)
    uint16_t fault_code;      // Fault code (2 bytes)
    uint8_t  severity;        // Severity (1 byte: 0=minor, 1=warning, 2=critical)
    uint8_t  reserved[9];     // Reserved area (9 bytes)
} fault_record_t;

void example_car_fault_storage(void) {
    m24c32_t eeprom_device;
    eeprom_directory_t directory;
    eeprom_status_t status;
    
    // 1. Initialize EEPROM device (implementation is project-dependent)
    // m24c32_init(&eeprom_device, ...);
    
    // 2. Initialize directory
    status = directory_init(&eeprom_device, &directory);
    if (status != EEPROM_OK) {
        // Error handling
        return;
    }
    
    // 3. Store engine temperature fault information
    uint8_t engine_temp_key[4] = FAULT_KEY_ENGINE_TEMP;
    fault_record_t engine_fault = {
        .timestamp = 1234567890,  // Example: Unix timestamp
        .fault_code = 0x1001,
        .severity = 2,            // Critical
        .reserved = {0}
    };
    
    status = set_directory_value(&directory, 
                                 engine_temp_key,
                                 (uint8_t *)&engine_fault,
                                 sizeof(fault_record_t));
    if (status != EEPROM_OK) {
        // Error handling
        return;
    }
    
    // 4. Store brake pressure fault information
    uint8_t brake_key[4] = FAULT_KEY_BRAKE_PRESSURE;
    fault_record_t brake_fault = {
        .timestamp = 1234567891,
        .fault_code = 0x2001,
        .severity = 1,            // Warning
        .reserved = {0}
    };
    
    status = set_directory_value(&directory,
                                 brake_key,
                                 (uint8_t *)&brake_fault,
                                 sizeof(fault_record_t));
    if (status != EEPROM_OK) {
        // Error handling
        return;
    }
    
    // 5. Store battery voltage fault information
    uint8_t battery_key[4] = FAULT_KEY_BATTERY_VOLT;
    fault_record_t battery_fault = {
        .timestamp = 1234567892,
        .fault_code = 0x3001,
        .severity = 0,            // Minor
        .reserved = {0}
    };
    
    status = set_directory_value(&directory,
                                 battery_key,
                                 (uint8_t *)&battery_fault,
                                 sizeof(fault_record_t));
    if (status != EEPROM_OK) {
        // Error handling
        return;
    }
}
```

### Example 2: Read Fault Information

```c
void example_read_car_faults(void) {
    m24c32_t eeprom_device;
    eeprom_directory_t directory;
    eeprom_status_t status;
    
    // Initialize directory (same as previous example)
    status = directory_init(&eeprom_device, &directory);
    if (status != EEPROM_OK) {
        return;
    }
    
    // Retrieve engine temperature fault information
    uint8_t engine_temp_key[4] = FAULT_KEY_ENGINE_TEMP;
    uint8_t *fault_data = NULL;
    uint16_t fault_data_size = 0;
    
    status = get_directory_value(&directory,
                                 engine_temp_key,
                                 &fault_data,
                                 &fault_data_size);
    
    if (status == EEPROM_OK) {
        // Successfully retrieved data
        fault_record_t *fault = (fault_record_t *)fault_data;
        
        printf("Fault code: 0x%04X\n", fault->fault_code);
        printf("Timestamp: %lu\n", (unsigned long)fault->timestamp);
        printf("Severity: %d\n", fault->severity);
        
        // Free memory (important!)
        free(fault_data);
        fault_data = NULL;
    } else if (status == EEPROM_ERROR_NOT_FOUND) {
        printf("Fault information not found\n");
    } else {
        printf("Error occurred: %d\n", status);
    }
}
```

### Example 3: Update and Delete Fault Information

```c
void example_update_and_delete_fault(void) {
    m24c32_t eeprom_device;
    eeprom_directory_t directory;
    eeprom_status_t status;
    
    // Initialize directory
    status = directory_init(&eeprom_device, &directory);
    if (status != EEPROM_OK) {
        return;
    }
    
    uint8_t engine_temp_key[4] = FAULT_KEY_ENGINE_TEMP;
    
    // Update existing fault information (set_directory_value automatically deletes old value)
    fault_record_t updated_fault = {
        .timestamp = 1234568000,  // New timestamp
        .fault_code = 0x1002,     // Updated fault code
        .severity = 1,             // Updated severity
        .reserved = {0}
    };
    
    status = set_directory_value(&directory,
                                 engine_temp_key,
                                 (uint8_t *)&updated_fault,
                                 sizeof(fault_record_t));
    if (status != EEPROM_OK) {
        // Error handling
        return;
    }
    
    // Delete fault information (e.g., when issue is resolved)
    status = delete_directory_value(&directory, engine_temp_key);
    if (status == EEPROM_OK) {
        printf("Fault information deleted\n");
    } else if (status == EEPROM_ERROR_NOT_FOUND) {
        printf("Fault information to delete not found\n");
    }
}
```

## Important Notes

1. **Memory Management**: Buffers retrieved from `get_directory_value()` must be freed using `free()`.

2. **Key Format**: Keys are fixed-size 4-byte values, not null-terminated strings. If using strings, pad to 4 bytes or use a hash value.

3. **Data Size Limit**: Maximum 16 bytes (4 blocks) of data can be stored per key.

4. **Key Count Limit**: Maximum 128 key-value pairs can be stored (`KEY_MAP_COUNT`).

5. **Block Size**: Data is managed in 4-byte blocks. If data size is not a multiple of 4 bytes, remaining bytes are padded with zeros.

6. **Error Handling**: Always check return values from all function calls and handle errors appropriately.

7. **Initialization Requirement**: Always call `directory_init()` before using the directory.

## Related Files

- `eeprom_directory.h`: Main API header file
- `eeprom_directory_struct.h`: Data structure and constant definitions
- `eeprom_status.h`: Error code definitions
- `eeprom_alloc.h`: Block allocation management
- `eeprom_storage.h`: Data storage management
