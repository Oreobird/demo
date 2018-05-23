TARGET_EXEC:=main
OBJ_DIR:=$(shell pwd)

EXCLUDE_DIR:=

# src dirs
SRC_DIR:=$(filter-out $(EXCLUDE_DIR), $(shell find . -maxdepth 1 -type d))

# src files
SRC_C:=$(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.c))

# obj files
TARGET_OBJ:=$(patsubst %.c, $(OBJ_DIR)/%.o, ${SRC_C})

INC_FLAGS:=$(patsubst %, -I%, $(SRC_DIR))

LDFLAGS+=$(LDFLAG_EXT)
CFLAGS+=$(CFLAG_EXT) -W -Wall -Werror$(INC_FLAGS)

all: $(TARGET_EXEC)

$(TARGET_EXEC): $(TARGET_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TARGET_OBJ) $(LDFLAGS)
	@echo "Build done"

${OBJ_DIR}/%.o: ${SRC_DIR}/%.c
	@echo $<
	$(CC) $(CFLAGS) -c $< -o $@
	
.PHONY:clean
clean:
	@rm $(TARGET_EXEC) -rf
	@rm $(TARGET_OBJ) -rf
