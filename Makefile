CC := gcc
SRCD := src
TSTD := tests
BLDD := build
BIND := bin
INCD := include

MAIN  := $(BLDD)/main.o

ALL_SRCF := $(shell find $(SRCD) -type f -name *.c)
ALL_OBJF := $(patsubst $(SRCD)/%,$(BLDD)/%,$(ALL_SRCF:.c=.o))
ALL_FUNCF := $(filter-out $(MAIN) $(AUX), $(ALL_OBJF))

TEST_ALL_SRCF := $(shell find $(TSTD) -type f -name *.c)
TEST_SRCF := $(filter-out $(TEST_REF_SRCF), $(TEST_ALL_SRCF))

INC := -I $(INCD)

CFLAGS := -Wall -Werror -Wno-unused-variable -Wno-unused-function -MMD -fcommon
COLORF := -DCOLOR
DFLAGS := -g -DDEBUG -DCOLOR
PGFLAGS := -g -pg
PRINT_STAMENTS := -DERROR -DSUCCESS -DWARN -DINFO

STD := -std=gnu11
TEST_LIB := -lcriterion
LIBS := -lm

CFLAGS += $(STD)

EXEC := birp
TEST_EXEC := $(EXEC)_tests

.PHONY: clean all setup debug

all: setup $(BIND)/$(EXEC) $(BIND)/$(TEST_EXEC)

debug: CFLAGS += $(DFLAGS) $(PRINT_STAMENTS) $(COLORF)
debug: all

prof: CFLAGS += $(PGFLAGS)
prof: all

setup: $(BIND) $(BLDD)
$(BIND):
	mkdir -p $(BIND)
$(BLDD):
	mkdir -p $(BLDD)

$(BIND)/$(EXEC): $(ALL_OBJF)
	$(CC) $(CFLAGS) $^ -o $@  tests/test_help/test_help.o $(LIBS)

$(BIND)/$(TEST_EXEC): $(ALL_FUNCF) $(TEST_SRCF) tests/test_help/test_help.o
	$(CC) $(CFLAGS) $(INC) $(ALL_FUNCF) $(TEST_SRCF) tests/test_help/test_help.o $(TEST_LIB) $(LIBS) -o $@

$(BLDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

$(TSTD)/%.o: $(TSTD)/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	strip --strip-unneeded $@

clean:
	rm -rf $(BLDD) $(BIND)

.PRECIOUS: $(BLDD)/*.d
-include $(BLDD)/*.d
