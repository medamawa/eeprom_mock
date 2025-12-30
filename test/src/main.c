#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "m24c32.h"
#include "eeprom_directory.h"

#define EEPROM_MOCK_FILENAME "eeprom_mock"

int g_mock_fd;

eeprom_status_t mock_eeprom_read(uint16_t addr, uint8_t *data, uint16_t len) {
	if (g_mock_fd < 0)
		return EEPROM_ERROR;
	
	if (lseek(g_mock_fd, addr, SEEK_SET) < 0)
		return EEPROM_ERROR;
	
	ssize_t n = read(g_mock_fd, data, len);
	if (n != len)
		return EEPROM_ERROR;
	
	printf("[READ] addr: %d, len: %d\n", addr, len);
	return EEPROM_OK;
}

eeprom_status_t mock_eeprom_write(uint16_t addr, uint8_t *data, uint16_t len) {
	if (g_mock_fd < 0)
		return EEPROM_ERROR;

	if (lseek(g_mock_fd, addr, SEEK_SET) < 0)
		return EEPROM_ERROR;

	ssize_t n = write(g_mock_fd, data, len);
	if (n != len)
		return EEPROM_ERROR;

	printf("[WRITE] addr: %d, len: %d\n", addr, len);
	return EEPROM_OK;
}

static m24c32_t setup_eeprom_mock() {
	g_mock_fd = open(EEPROM_MOCK_FILENAME, O_RDWR | O_CREAT, 0666);
	if (g_mock_fd < 0) {
		printf("ERROR: Failed to open file.\n");
		exit(1);
	}

	m24c32_t mock;
	mock.read = mock_eeprom_read;
	mock.write = mock_eeprom_write;
	return mock;
}


static void test_get_value(eeprom_directory_t *directory, const char *key, const char *test_name) {
	printf("\n=== Test: %s ===\n", test_name);
	uint8_t *out = NULL;
	uint16_t out_size;
	eeprom_status_t res = get_directory_value(directory, (uint8_t *)key, &out, &out_size);
	if (res == EEPROM_OK && out != NULL) {
		printf("[GET] key: %s, size: %d, data: ", key, out_size);
		for (uint16_t i = 0; i < out_size; i++) {
			printf("%d ", out[i]);
		}
		printf("\n");
		free(out);
		out = NULL;
	} else {
		printf("[GET] Failed - key: %s, status: %d\n", key, res);
	}
}

static void test_set_value(eeprom_directory_t *directory, const char *key, uint8_t *value, uint16_t value_size, const char *test_name) {
	printf("\n=== Test: %s ===\n", test_name);
	eeprom_status_t res = set_directory_value(directory, (uint8_t *)key, value, value_size);
	if (res == EEPROM_OK) {
		printf("[SET] key: %s, size: %d - SUCCESS\n", key, value_size);
	} else {
		printf("[SET] key: %s, size: %d - FAILED with status: %d\n", key, value_size, res);
	}
}

static void test_delete_value(eeprom_directory_t *directory, const char *key, const char *test_name) {
	printf("\n=== Test: %s ===\n", test_name);
	eeprom_status_t res = delete_directory_value(directory, (const uint8_t *)key);
	if (res == EEPROM_OK) {
		printf("[DELETE] key: %s - SUCCESS\n", key);
	} else {
		printf("[DELETE] key: %s - FAILED with status: %d\n", key, res);
	}
}

int main(void) {
	printf("========================================\n");
	printf("EEPROM Directory Test Suite\n");
	printf("========================================\n");

	m24c32_t mock = setup_eeprom_mock();
	eeprom_directory_t *directory = malloc(sizeof(eeprom_directory_t));

	// Initialize eeprom mock with zeros
	char *data = malloc(EEPROM_SIZE);
	memset(data, 0, EEPROM_SIZE);
	m24c32_write(&mock, 0, (uint8_t *)data, EEPROM_SIZE);
	free(data);

	// Initialize directory
	eeprom_status_t res = directory_init(&mock, directory);
	if (res != EEPROM_OK) {
		printf("ERROR: Failed to initialize directory: %d\n", res);
		free(directory);
		close(g_mock_fd);
		unlink(EEPROM_MOCK_FILENAME);
		return 1;
	}
	printf("\n=== Initial State ===\n");
	print_alloc_table(directory);

	// Test 1: Set a single byte value
	uint8_t value1 = 116;
	test_set_value(directory, "sogo", &value1, 1, "Set single byte value");
	print_alloc_table(directory);

	// Test 2: Get the value back
	test_get_value(directory, "sogo", "Get single byte value");

	// Test 3: Set multiple keys
	uint8_t value2 = 42;
	test_set_value(directory, "test", &value2, 1, "Set second key");
	
	uint8_t value3 = 99;
	test_set_value(directory, "key1", &value3, 1, "Set third key");
	print_alloc_table(directory);

	// Test 4: Get all values
	test_get_value(directory, "sogo", "Get first key");
	test_get_value(directory, "test", "Get second key");
	test_get_value(directory, "key1", "Get third key");

	// Test 5: Set a multi-block value (larger than BLOCK_SIZE=4)
	uint8_t large_value[] = {1, 2, 3, 4, 5, 6, 7, 8};
	test_set_value(directory, "big", large_value, 8, "Set multi-block value (8 bytes)");
	print_alloc_table(directory);
	test_get_value(directory, "big", "Get multi-block value");

	// Test 6: Overwrite existing value
	uint8_t new_value = 200;
	test_set_value(directory, "sogo", &new_value, 1, "Overwrite existing value");
	test_get_value(directory, "sogo", "Get overwritten value");

	// Test 7: Delete a value
	test_delete_value(directory, "test", "Delete existing key");
	print_alloc_table(directory);
	
	// Test 8: Try to get deleted value (should fail)
	test_get_value(directory, "test", "Get deleted key (should fail)");

	// Test 9: Try to delete non-existent key (should fail)
	test_delete_value(directory, "nonex", "Delete non-existent key");

	// Test 10: Delete another value and verify blocks are freed
	test_delete_value(directory, "key1", "Delete second key");
	print_alloc_table(directory);

	// Test 11: Set value after deletion (reuse freed blocks)
	uint8_t value4 = 77;
	test_set_value(directory, "new", &value4, 1, "Set value after deletion");
	print_alloc_table(directory);
	test_get_value(directory, "new", "Get newly set value");

	// Test 12: Delete all remaining values
	test_delete_value(directory, "sogo", "Delete first key");
	test_delete_value(directory, "big", "Delete multi-block key");
	test_delete_value(directory, "new", "Delete last key");
	print_alloc_table(directory);

	// Test 13: Verify all blocks are freed
	printf("\n=== Final State ===\n");
	print_alloc_table(directory);

	// Test 15: Store a char string ("ABCD") as bytes
	uint8_t str_value[] = {'A', 'B', 'C', 'D'};
	test_set_value(directory, "str1", str_value, sizeof(str_value), "Store char array (\"ABCD\")");
	test_get_value(directory, "str1", "Get char array (\"ABCD\")");

	// Test 16: Store a C-string with null terminator ("XYZ\\0")
	uint8_t cstr_value[] = {'X', 'Y', 'Z', '\0'};
	test_set_value(directory, "cstr", cstr_value, sizeof(cstr_value), "Store C-string with NUL terminator (\"XYZ\\0\")");
	test_get_value(directory, "cstr", "Get C-string with NUL terminator");

	// Test 17: Store an int as 4 bytes (little-endian)
	uint32_t num = 0x12345678;
	uint8_t int_bytes[4];
	int_bytes[0] = (uint8_t)(num & 0xFF);
	int_bytes[1] = (uint8_t)((num >> 8) & 0xFF);
	int_bytes[2] = (uint8_t)((num >> 16) & 0xFF);
	int_bytes[3] = (uint8_t)((num >> 24) & 0xFF);
	test_set_value(directory, "int1", int_bytes, sizeof(int_bytes), "Store 32-bit int as 4 bytes");
	test_get_value(directory, "int1", "Get 32-bit int as 4 bytes");

	// Cleanup of extra test keys
	test_delete_value(directory, "str1", "Delete char array key");
	test_delete_value(directory, "cstr", "Delete C-string key");
	test_delete_value(directory, "int1", "Delete int key");

	// Test 14: Error case - null pointer
	printf("\n=== Test: Error handling - null pointer ===\n");
	res = get_directory_value(NULL, (uint8_t *)"test", NULL, NULL);
	printf("[GET] NULL directory - status: %d (expected: %d)\n", res, EEPROM_ERROR_NULL_POINTER);

	res = set_directory_value(NULL, (uint8_t *)"test", &value1, 1);
	printf("[SET] NULL directory - status: %d (expected: %d)\n", res, EEPROM_ERROR_NULL_POINTER);

	res = delete_directory_value(NULL, (const uint8_t *)"test");
	printf("[DELETE] NULL directory - status: %d (expected: %d)\n", res, EEPROM_ERROR_NULL_POINTER);

	printf("\n========================================\n");
	printf("All tests completed\n");
	printf("========================================\n");

	free(directory);
	close(g_mock_fd);
	unlink(EEPROM_MOCK_FILENAME);
	return 0;
}