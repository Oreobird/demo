EXCLUDE_DIR:=netlink 

DIR:=$(shell find . -maxdepth 1 -type d)
DIR:=$(basename $(patsubst ./%,%,$(DIR)))
DIR:=$(filter-out $(EXCLUDE_DIR), $(DIR))

CURDIR:=$(shell pwd)
SUBDIRS:=$(DIR)
CLEAN_DIRS:=$(addprefix _clean_,$(SUBDIRS))

MAKELIST:=@for subdir in $(SUBDIRS); \
	do \
	cd $(CURDIR)/$$subdir; make; \
	done

CLEANLIST:=@for subdir in $(SUBDIRS); \
	do \
	cd $(CURDIR)/$$subdir; make clean; \
	done

all:
	$(MAKELIST)

.PHONY:clean
clean:
	$(CLEANLIST)
