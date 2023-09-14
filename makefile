# choose a compiler
CC := gcc

# library name stem
STEM := spng

# set directories
SRC_DIR := src
OBJ_DIR := obj
LIB_DIR := lib
BIN_DIR := bin
TEST_DIR := test
TEMP_DIRS := $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)
# set targets
# executable
TEST := $(BIN_DIR)/test
# srcs
SRC := $(wildcard $(SRC_DIR)/*.c)
TEST_SRC := $(wildcard $(TEST_DIR)/*.c)
# objects
SRC_OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TEST_OBJ := $(OBJ_DIR)/test.o
# libs
STATIC_LIB := $(LIB_DIR)/lib$(STEM).a
SHARED_LIB := $(LIB_DIR)/lib$(STEM).so

# set flags
CPPFLAGS := -fpic -MMD -MP
CFLAGS := -Wall -Wextra -Werror -std=gnu11 -pedantic -0s
# LDFLAGS :=
LDLIBS := -lm -lpthread -lpng
LIBFLAGS := -shared

# set rules

.PHONY: all test lib clean

all: lib test
	./$(TEST)

test: $(TEST)

lib: $(STATIC_LIB)

$(TEST): $(SRC_OBJ) $(TEST_OBJ)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(STATIC_LIB): $(SHARED_LIB)
	ar -rs $@ $<

$(SHARED_LIB): $(SRC_OBJ)
	$(CC) $(LIBFLAGS) $< -o $@

$(SRC_OBJ): $(SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@ $(LDLIBS)

$(TEST_OBJ): $(TEST_SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -I$(SRC_DIR) $< -o $@

# clean all temporary directories
clean:
	@$(RM) -rfv $(TEMP_DIRS)

-include $(OBJ:.o=.d)

# make all directories
$(shell mkdir -p $(TEMP_DIRS))
