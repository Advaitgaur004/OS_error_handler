CC = gcc
CFLAGS = -Wall -Wextra -g -I$(SRC_DIR)
SRC_DIR = src
SIM_DIR = src/simulations
BUILD_DIR = build
LOG_DIR = logs

# Create necessary directories
mkdirs:
		mkdir -p $(BUILD_DIR) $(LOG_DIR)

# Source files
SRC_FILES = $(SRC_DIR)/logger.c \
	$(SRC_DIR)/recovery.c \
	$(SRC_DIR)/error_handler.c

# Simulation executables
SIMULATIONS = simulate_memory_error simulate_file_error simulate_device_error

all: clean mkdirs $(SIMULATIONS)

simulate_memory_error: $(SIM_DIR)/simulate_memory_error.c $(SRC_FILES)
	$(CC) $(CFLAGS) $(SIM_DIR)/simulate_memory_error.c $(SRC_FILES) -o $(BUILD_DIR)/simulate_memory_error

simulate_file_error: $(SIM_DIR)/simulate_file_error.c $(SRC_FILES)
	$(CC) $(CFLAGS) $(SIM_DIR)/simulate_file_error.c $(SRC_FILES) -o $(BUILD_DIR)/simulate_file_error

simulate_device_error: $(SIM_DIR)/simulate_device_error.c $(SRC_FILES)
	$(CC) $(CFLAGS) $(SIM_DIR)/simulate_device_error.c $(SRC_FILES) -o $(BUILD_DIR)/simulate_device_error
	$(CC) $(SIM_DIR)/sleep.c -o $(BUILD_DIR)/sleep
	touch $(BUILD_DIR)/access.txt
	chmod 444 $(BUILD_DIR)/access.txt
	touch $(BUILD_DIR)/example.lock

clean:
	rm -rf $(BUILD_DIR)/*

.PHONY: all clean mkdirs
