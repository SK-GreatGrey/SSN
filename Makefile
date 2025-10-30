CC := gcc
CFLAGS := -g -Wall -Wextra -std=c11 -MMD -MP -Wno-format-truncation -Wno-unused-parameter -Wno-unused-function 
#LFLAGS := -I

# Directories
LIBS_DIR := libs
JANSSON_DIR := libs/jansson
CLIENT_DIR := client
BUILD_DIR := build
TMP_DIR := $(BUILD_DIR)/tmp

# Include paths for GCC
CFLAGS += -I$(LIBS_DIR) -I$(JANSSON_DIR)

# Source files
LIB_SRC := $(shell find $(LIBS_DIR) -name '*.c')
CLIENT_SRC := $(shell find $(CLIENT_DIR) -name '*.c')
SERVER_SRC := $(shell find $(SERVER_DIR) -name '*.c')

# Object files in temp
LIB_OBJ := $(patsubst %.c,$(TMP_DIR)/%.o,$(LIB_SRC))
CLIENT_OBJ := $(patsubst %.c,$(TMP_DIR)/%.o,$(CLIENT_SRC)) $(LIB_OBJ)
SERVER_OBJ := $(patsubst %.c,$(TMP_DIR)/%.o,$(SERVER_SRC)) $(LIB_OBJ)

# Dependency files in temp
DEP := $(LIB_OBJ:.o=.d) $(CLIENT_OBJ:.o=.d) $(SERVER_OBJ:.o=.d)

# Final compiled output path
CLIENT_BIN := $(BUILD_DIR)/client
SERVER_BIN := $(BUILD_DIR)/server

all: client server

client: $(CLIENT_BIN)
server: $(SERVER_BIN)

# Compile all client .o files
$(CLIENT_BIN): $(CLIENT_OBJ)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

# Compile all server .o files
$(SERVER_BIN): $(SERVER_OBJ)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

# Generic object build rule
$(TMP_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

run:
	@echo "Usage: make run-client OR make run-server";

run-client: $(CLIENT_BIN)
	./$(CLIENT_BIN)

run-server: $(SERVER_BIN)
	./$(SERVER_BIN)

clean:
	$(RM) -rf $(TMP_DIR)

-include $(DEP)

.PHONY: all