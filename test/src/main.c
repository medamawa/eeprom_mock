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
	
	printf("[READ] addr: %d, data: [%s]\n", addr, data);
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

	printf("[WRITE] addr: %d, data: [%s]\n", addr, data);
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


int main(void) {
	m24c32_t mock = setup_eeprom_mock();
	eeprom_directory_t *directory = malloc(sizeof(eeprom_directory_t));

	// init eeprom mock
	char *data = malloc(EEPROM_SIZE);
	memset(data, 0, EEPROM_SIZE);
	m24c32_write(&mock, 0, (uint8_t *)data, sizeof(data));
	free(data);

	directory_init(&mock, directory);
	print_alloc_table(directory);

	close(g_mock_fd);
	return 0;
}