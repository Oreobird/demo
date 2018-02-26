TARGET_EXEC:=mf_demo
EXCLUDE_DIR:=

# src dirs
SRC_SUBDIR:=$(filter-out $(EXCLUDE_DIR), $(shell find . -maxdepth 1 -type d))
# src files
SRC_C:=$(foreach dir, $(SRC_SUBDIR), $(wildcard $(dir)/*.c))
# obj files
SUB_OBJ:=$(patsubst %.c, %.o, ${SRC_C})
TARGET_OBJ:=$(SUB_OBJ) $(TARGET_EXEC).o

LDFLAGS+=$(LDFLAG_EXT)
CFLAGS+=$(CFLAG_EXT) -W -Wall -Werror

all: $(TARGET_EXEC)

$(TARGET_EXEC): $(TARGET_OBJ)
	@echo "Starting build..."
	$(CC) $(CFLAGS) -o $@ $(SRC_C) $(LDFLAGS)
	@echo "Build done"

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
.PHONY:clean
clean:
	@find . -maxdepth 2 -name *.o -exec rm -rf {} \;
