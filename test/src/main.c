#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "m24c32.h"
#include "eeprom_directory.h"

static uint8_t *g_mock_memory = NULL;

eeprom_status_t mock_eeprom_read(uint16_t addr, uint8_t *data, uint16_t len) {
	if (g_mock_memory == NULL)
		return EEPROM_ERROR;
	
	if (addr + len > EEPROM_SIZE)
		return EEPROM_ERROR;
	
	memcpy(data, &g_mock_memory[addr], len);
	printf("[READ] addr: %d, len: %d\n", addr, len);
	return EEPROM_OK;
}

eeprom_status_t mock_eeprom_write(uint16_t addr, uint8_t *data, uint16_t len) {
	if (g_mock_memory == NULL)
		return EEPROM_ERROR;

	if (addr + len > EEPROM_SIZE)
		return EEPROM_ERROR;

	memcpy(&g_mock_memory[addr], data, len);
	printf("[WRITE] addr: %d, len: %d\n", addr, len);
	return EEPROM_OK;
}

static m24c32_t setup_eeprom_mock() {
	// Allocate memory for EEPROM simulation
	g_mock_memory = (uint8_t *)malloc(EEPROM_SIZE);
	if (g_mock_memory == NULL) {
		printf("ERROR: Failed to allocate memory for EEPROM mock.\n");
		exit(1);
	}
	
	// Initialize with zeros
	memset(g_mock_memory, 0, EEPROM_SIZE);

	m24c32_t mock;
	mock.read = mock_eeprom_read;
	mock.write = mock_eeprom_write;
	return mock;
}


static double measure_time_us(struct timespec *start, struct timespec *end) {
	long elapsed_ns = (end->tv_sec - start->tv_sec) * 1000000000L + (end->tv_nsec - start->tv_nsec);
	return elapsed_ns / 1000.0;
}

static void test_get_value(eeprom_directory_t *directory, const char *key, const char *test_name) {
	printf("\n=== Test: %s ===\n", test_name);
	
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	
	uint8_t *out = NULL;
	uint16_t out_size;
	eeprom_status_t res = get_directory_value(directory, (uint8_t *)key, &out, &out_size);
	
	clock_gettime(CLOCK_MONOTONIC, &end);
	double elapsed_us = measure_time_us(&start, &end);
	
	if (res == EEPROM_OK && out != NULL) {
		printf("[GET] key: %s, size: %d, data: ", key, out_size);
		for (uint16_t i = 0; i < out_size; i++) {
			printf("%d ", out[i]);
		}
		printf("- Time: %.3f us\n", elapsed_us);
		free(out);
		out = NULL;
	} else {
		printf("[GET] Failed - key: %s, status: %d - Time: %.3f us\n", key, res, elapsed_us);
	}
}

static void test_set_value(eeprom_directory_t *directory, const char *key, uint8_t *value, uint16_t value_size, const char *test_name) {
	printf("\n=== Test: %s ===\n", test_name);
	
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	
	eeprom_status_t res = set_directory_value(directory, (uint8_t *)key, value, value_size);
	
	clock_gettime(CLOCK_MONOTONIC, &end);
	double elapsed_us = measure_time_us(&start, &end);
	
	if (res == EEPROM_OK) {
		printf("[SET] key: %s, size: %d - SUCCESS - Time: %.3f us\n", key, value_size, elapsed_us);
	} else {
		printf("[SET] key: %s, size: %d - FAILED with status: %d - Time: %.3f us\n", key, value_size, res, elapsed_us);
	}
}

static void test_delete_value(eeprom_directory_t *directory, const char *key, const char *test_name) {
	printf("\n=== Test: %s ===\n", test_name);
	
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	
	eeprom_status_t res = delete_directory_value(directory, (const uint8_t *)key);
	
	clock_gettime(CLOCK_MONOTONIC, &end);
	double elapsed_us = measure_time_us(&start, &end);
	
	if (res == EEPROM_OK) {
		printf("[DELETE] key: %s - SUCCESS - Time: %.3f us\n", key, elapsed_us);
	} else {
		printf("[DELETE] key: %s - FAILED with status: %d - Time: %.3f us\n", key, res, elapsed_us);
	}
}

static eeprom_directory_t* setup_test_environment(m24c32_t *mock) {
	eeprom_directory_t *directory = malloc(sizeof(eeprom_directory_t));
	if (!directory) {
		return NULL;
	}

	// EEPROM mock memory is already initialized to zeros in setup_eeprom_mock()
	// Initialize directory
	eeprom_status_t res = directory_init(mock, directory);
	if (res != EEPROM_OK) {
		printf("ERROR: Failed to initialize directory: %d\n", res);
		free(directory);
		return NULL;
	}

	return directory;
}

static void cleanup_test_environment(eeprom_directory_t *directory) {
	if (directory) {
		free(directory);
	}
	if (g_mock_memory) {
		free(g_mock_memory);
		g_mock_memory = NULL;
	}
}

static void run_basic_tests(eeprom_directory_t *directory) {
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
}

static void run_delete_tests(eeprom_directory_t *directory) {
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
}

static void run_data_type_tests(eeprom_directory_t *directory) {
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

	// Test 18: Store char[] cast to uint8_t*
	char char_array[] = {'H', 'e', 'l', 'l', 'o'};
	test_set_value(directory, "char_cast", (uint8_t *)char_array, sizeof(char_array), "Store char[] cast to uint8_t*");
	test_get_value(directory, "char_cast", "Get char[] cast to uint8_t*");

	// Cleanup of extra test keys
	test_delete_value(directory, "str1", "Delete char array key");
	test_delete_value(directory, "cstr", "Delete C-string key");
	test_delete_value(directory, "int1", "Delete int key");
	test_delete_value(directory, "char_cast", "Delete char cast key");
}

static void run_error_tests(void) {
	// Test 14: Error case - null pointer
	printf("\n=== Test: Error handling - null pointer ===\n");
	uint8_t value1 = 116;
	
	eeprom_status_t res = get_directory_value(NULL, (uint8_t *)"test", NULL, NULL);
	printf("[GET] NULL directory - status: %d (expected: %d)\n", res, EEPROM_ERROR_NULL_POINTER);

	res = set_directory_value(NULL, (uint8_t *)"test", &value1, 1);
	printf("[SET] NULL directory - status: %d (expected: %d)\n", res, EEPROM_ERROR_NULL_POINTER);

	res = delete_directory_value(NULL, (const uint8_t *)"test");
	printf("[DELETE] NULL directory - status: %d (expected: %d)\n", res, EEPROM_ERROR_NULL_POINTER);
}

static void run_stress_tests(eeprom_directory_t *directory) {
	printf("\n=== Stress Test: Multiple keys operations ===\n");
	
	const int num_keys = 50;
	char key_buf[32];
	uint8_t value = 0;
	
	struct timespec start_all, end_all;
	clock_gettime(CLOCK_MONOTONIC, &start_all);
	
	// Set many keys
	printf("Setting %d keys...\n", num_keys);
	for (int i = 0; i < num_keys; i++) {
		snprintf(key_buf, sizeof(key_buf), "key_%03d", i);
		value = (uint8_t)(i % 256);
		eeprom_status_t res = set_directory_value(directory, (uint8_t *)key_buf, &value, 1);
		if (res != EEPROM_OK) {
			printf("ERROR: Failed to set key %s at index %d\n", key_buf, i);
			break;
		}
	}
	
	// Get all keys
	printf("Getting %d keys...\n", num_keys);
	for (int i = 0; i < num_keys; i++) {
		snprintf(key_buf, sizeof(key_buf), "key_%03d", i);
		uint8_t *out = NULL;
		uint16_t out_size;
		eeprom_status_t res = get_directory_value(directory, (uint8_t *)key_buf, &out, &out_size);
		if (res == EEPROM_OK && out != NULL) {
			if (out[0] != (uint8_t)(i % 256)) {
				printf("ERROR: Value mismatch for key %s: expected %d, got %d\n", key_buf, i % 256, out[0]);
			}
			free(out);
		} else {
			printf("ERROR: Failed to get key %s at index %d\n", key_buf, i);
		}
	}
	
	// Delete all keys
	printf("Deleting %d keys...\n", num_keys);
	for (int i = 0; i < num_keys; i++) {
		snprintf(key_buf, sizeof(key_buf), "key_%03d", i);
		eeprom_status_t res = delete_directory_value(directory, (const uint8_t *)key_buf);
		if (res != EEPROM_OK) {
			printf("ERROR: Failed to delete key %s at index %d\n", key_buf, i);
		}
	}
	
	clock_gettime(CLOCK_MONOTONIC, &end_all);
	double total_time = measure_time_us(&start_all, &end_all);
	printf("Stress test completed: %d keys (set/get/delete) in %.3f us (%.3f ms)\n", 
	       num_keys, total_time, total_time / 1000.0);
	
	printf("\n=== Stress Test: Large data operations ===\n");
	
	const int large_data_size = 100;
	uint8_t *large_data = malloc(large_data_size);
	if (large_data) {
		// Fill with pattern
		for (int i = 0; i < large_data_size; i++) {
			large_data[i] = (uint8_t)(i % 256);
		}
		
		struct timespec start, end;
		clock_gettime(CLOCK_MONOTONIC, &start);
		
		eeprom_status_t res = set_directory_value(directory, (uint8_t *)"large_data", large_data, large_data_size);
		
		clock_gettime(CLOCK_MONOTONIC, &end);
		double set_time = measure_time_us(&start, &end);
		
		if (res == EEPROM_OK) {
			printf("Set large data (%d bytes) - Time: %.3f us (%.3f ms)\n", 
			       large_data_size, set_time, set_time / 1000.0);
			
			clock_gettime(CLOCK_MONOTONIC, &start);
			
			uint8_t *out = NULL;
			uint16_t out_size;
			res = get_directory_value(directory, (uint8_t *)"large_data", &out, &out_size);
			
			clock_gettime(CLOCK_MONOTONIC, &end);
			double get_time = measure_time_us(&start, &end);
			
			if (res == EEPROM_OK && out != NULL) {
				// Verify data
				int mismatch = 0;
				for (int i = 0; i < large_data_size && i < out_size; i++) {
					if (out[i] != large_data[i]) {
						mismatch++;
					}
				}
				
				if (mismatch == 0 && out_size == large_data_size) {
					printf("Get large data (%d bytes) - Time: %.3f us (%.3f ms) - Verified OK\n", 
					       large_data_size, get_time, get_time / 1000.0);
				} else {
					printf("ERROR: Data mismatch or size mismatch (mismatches: %d, size: %d vs %d)\n", 
					       mismatch, out_size, large_data_size);
				}
				free(out);
			} else {
				printf("ERROR: Failed to get large data\n");
			}
			
			delete_directory_value(directory, (const uint8_t *)"large_data");
		} else {
			printf("ERROR: Failed to set large data\n");
		}
		
		free(large_data);
	}
	
	printf("\n=== Stress Test: Repeated overwrite operations ===\n");
	
	uint8_t test_value = 0;
	const int overwrite_count = 100;
	
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	
	for (int i = 0; i < overwrite_count; i++) {
		test_value = (uint8_t)(i % 256);
		eeprom_status_t res = set_directory_value(directory, (uint8_t *)"overwrite_key", &test_value, 1);
		if (res != EEPROM_OK) {
			printf("ERROR: Failed to overwrite at iteration %d\n", i);
			break;
		}
	}
	
	clock_gettime(CLOCK_MONOTONIC, &end);
	double overwrite_time = measure_time_us(&start, &end);
	
	printf("Repeated overwrite (%d times) - Total time: %.3f us (%.3f ms), Avg: %.3f us per operation\n", 
	       overwrite_count, overwrite_time, overwrite_time / 1000.0, overwrite_time / overwrite_count);
	
	// Verify final value
	uint8_t *out = NULL;
	uint16_t out_size;
	eeprom_status_t res = get_directory_value(directory, (uint8_t *)"overwrite_key", &out, &out_size);
	if (res == EEPROM_OK && out != NULL) {
		uint8_t expected = (uint8_t)((overwrite_count - 1) % 256);
		if (out[0] == expected) {
			printf("Final value verified: %d (expected: %d)\n", out[0], expected);
		} else {
			printf("ERROR: Final value mismatch: got %d, expected %d\n", out[0], expected);
		}
		free(out);
	}
	
	delete_directory_value(directory, (const uint8_t *)"overwrite_key");
}

int main(void) {
	printf("========================================\n");
	printf("EEPROM Directory Test Suite\n");
	printf("========================================\n");

	m24c32_t mock = setup_eeprom_mock();
	eeprom_directory_t *directory = setup_test_environment(&mock);
	if (!directory) {
		cleanup_test_environment(NULL);
		return 1;
	}

	printf("\n=== Initial State ===\n");
	print_alloc_table(directory);

	run_basic_tests(directory);
	run_delete_tests(directory);
	run_data_type_tests(directory);
	run_error_tests();
	run_stress_tests(directory);

	printf("\n========================================\n");
	printf("All tests completed\n");
	printf("========================================\n");

	cleanup_test_environment(directory);
	return 0;
}