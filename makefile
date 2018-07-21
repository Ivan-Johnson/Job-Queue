#makefile
#
#Defines how the project should be built
#
#Copyright(C) 2018, Ivan Tobias Johnson
#
#LICENSE: GPL 2.0


APP = jormungandr

SRC_DIR = Src
SRC_TEST_DIR = Test
BIN_DIR_BASE = Bin

MAKEFILE_CONFIG = makefile.config

TEMPLATE_DIR = Templates

MAKEFILE_CONFIG_TEMPLATE=$(TEMPLATE_DIR)/makefile.config
BISECT_TEST_TEMPLATE=$(TEMPLATE_DIR)/bisect.c

#the generate_test_runner.rb script from Unity
#if it's not in $PATH, then this variable can be changed to an absolute path.
SUMMARY_SCRIPT = Scripts/sumarize.bash
GENERATE_RUNNER = generate_test_runner.rb

.DELETE_ON_ERROR:

##############
#CC ARGUMENTS#
##############

#to cross-compile for windows, uncomment. Executables must be renamed to .exe
#CC = x86_64-w64-mingw32-gcc

CFLAGS += -I$(SRC_DIR)
CFLAGS += -Wfatal-errors -std=c99 -Werror -Wconversion -Wall -Wextra -pedantic -pedantic-errors



BUILD_TYPE ?= debug
ifeq ($(BUILD_TYPE), debug)
	CFLAGS += -D DEBUG -O0 -ggdb -fno-inline -fsanitize=address -fsanitize=leak -fsanitize=undefined
	LDLIBS += -lasan -lubsan
endif
ifeq ($(BUILD_TYPE), develop)
	CFLAGS += -D DEVELOP -O0 -ggdb -fno-inline
endif
ifeq ($(BUILD_TYPE), release)
	CFLAGS += -D RELEASE -O3
endif

IS_TEST ?= $(if $(shell [ "$(MAKECMDGOALS)" = "test" ] && echo asdf),yes,no)
ifeq ($(IS_TEST), yes)
	BIN_DIR = $(BIN_DIR_BASE)/$(BUILD_TYPE)/Test
	CFLAGS += -D TEST
	LDLIBS += -lunity
else
	BIN_DIR = $(BIN_DIR_BASE)/$(BUILD_TYPE)/Nontest
endif

-include $(MAKEFILE_CONFIG)

###########
#BUILD APP#
###########

SOURCES = $(wildcard $(SRC_DIR)/*.c)
HEADERS = $(wildcard $(SRC_DIR)/*.h)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)
DEPENDS = $(BIN_DIR)/.depends

.PHONY: all
all: $(BIN_DIR)/$(APP)

$(BIN_DIR)/$(APP): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

$(BIN_DIR)/%.o: | $(BIN_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(DEPENDS): $(SOURCES) | $(BIN_DIR)
	$(CC) $(CFLAGS) -MM $^ | sed -e 's!^!$(BIN_DIR)/!' >$@

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),CLEAN)
-include $(DEPENDS)
endif
endif


$(BIN_DIR):
	mkdir -p $@


#############
#BUILD TESTS#
#############
ifeq ($(IS_TEST), yes)
SOURCE_TESTRUNNER_DIR = $(BIN_DIR_BASE)/TestRunners
BIN_TESTRUNNER_DIR    = $(BIN_DIR)/TestRunners
BIN_TEST_DIR          = $(BIN_DIR)/TestCases
OUT_TEST_DIR          = $(BIN_DIR)/TestOut


#1: we're given these test case files
TEST_SOURCES        = $(wildcard $(SRC_TEST_DIR)/*.c)
#2: which we use to generate _runner.c files
TEST_SOURCES_RUNNER = $(TEST_SOURCES:$(SRC_TEST_DIR)/%.c=$(SOURCE_TESTRUNNER_DIR)/%_runner.c)
#3: that are then compiled to _runner.o files
TEST_OBJECTS        = $(TEST_SOURCES:$(SRC_TEST_DIR)/%.c=$(BIN_TESTRUNNER_DIR)/%_runner.o)
#4: which are in turn linked into executables
TEST_RUNNERS        = $(TEST_SOURCES:$(SRC_TEST_DIR)/%.c=$(BIN_TESTRUNNER_DIR)/%_runner)
#5: whose output is collected
TEST_OUT_FILES      = $(TEST_SOURCES:$(SRC_TEST_DIR)/%.c=$(OUT_TEST_DIR)/%.out)
#6: and sumarized
TEST_SUMMARY_FILE   = $(OUT_TEST_DIR)/summary.txt


BISECT_NAME=$(shell basename --suffix=".c" "$(BISECT_TEST_TEMPLATE)")
BISECT_TEST_SRC=$(SRC_TEST_DIR)/$(BISECT_NAME).c
BISECT_BINARY=$(BIN_TESTRUNNER_DIR)/$(BISECT_NAME)_runner


TESTRUNNER_DEPENDS  = $(BIN_TESTRUNNER_DIR)/.depends
TESTCASE_DEPENDS    = $(BIN_TEST_DIR)/.depends

.PHONY:test
test: $(TEST_SUMMARY_FILE) $(TEST_OUT_FILES) $(TEST_RUNNERS)
	@cat $(TEST_SUMMARY_FILE)

$(TEST_SUMMARY_FILE): $(SUMMARY_SCRIPT) $(TEST_OUT_FILES)
#If there are no failures, then exit   successfully and silently.
#If there ARE    failures, then exit UNsuccessfully and show the summary file
#This is done so that "make test" will show the summary exactly once, regardless of whether or not all tests passed.
	$(SUMMARY_SCRIPT) $(TEST_OUT_FILES) > $@ || { cat $@; false; }

$(OUT_TEST_DIR)/%.out: $(BIN_TESTRUNNER_DIR)/%_runner | $(OUT_TEST_DIR)
	$< > $@ || true

#TODO: only link with the objects that it actually needs
.PRECIOUS: $(TEST_RUNNERS)
$(BIN_TESTRUNNER_DIR)/%_runner: $(OBJECTS) $(BIN_TESTRUNNER_DIR)/%_runner.o $(BIN_TEST_DIR)/%.o  | $(BIN_TESTRUNNER_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

$(BIN_TEST_DIR)/%.o: | $(BIN_TEST_DIR)

$(BIN_TESTRUNNER_DIR)/%_runner.o: | $(BIN_TESTRUNNER_DIR)
	@echo "BUILDING $@"
	@echo "USING $^"
	$(CC) $(CFLAGS) -c -o $@ $<

$(TESTRUNNER_DEPENDS): $(BISECT_TEST_SRC) $(TEST_SOURCES_RUNNER) | $(BIN_TESTRUNNER_DIR)
	$(CC) $(CFLAGS) -MM $^ | sed -e 's!^!$(BIN_TESTRUNNER_DIR)/!' >$@
$(TESTCASE_DEPENDS): $(BISECT_TEST_SRC) $(TEST_SOURCES) | $(BIN_TEST_DIR)
	$(CC) $(CFLAGS) -MM $^ | sed -e 's!^!$(BIN_TEST_DIR)/!' >$@

-include $(TESTCASE_DEPENDS)
-include $(TESTRUNNER_DEPENDS)

.PRECIOUS: $(TEST_SOURCES_RUNNER)
$(SOURCE_TESTRUNNER_DIR)/%_runner.c: $(SRC_TEST_DIR)/%.c | $(SOURCE_TESTRUNNER_DIR)
	$(GENERATE_RUNNER) $< $@

$(OUT_TEST_DIR):
	mkdir -p $@

$(BIN_TESTRUNNER_DIR):
	mkdir -p $@

$(SOURCE_TESTRUNNER_DIR):
	mkdir -p $@

$(BIN_TEST_DIR):
	mkdir -p $@

############
#GIT BISECT#
############
$(BISECT_TEST_SRC): $(BISECT_TEST_TEMPLATE)
	@if [ -e "$@" ]; then \
		touch "$@"; \
	else \
		mkdir -p "$$(dirname $@)"; \
		cp "$<" "$@"; \
	fi


.PHONY: bisect
bisect_make: $(BISECT_BINARY)

.PHONY: bisect_run
bisect_run: $(BISECT_BINARY)
	$(BISECT_BINARY)


endif


###############
#MISCELLANEOUS#
###############

.PHONY: clean
clean:
	rm -rf $(BIN_DIR) $(SOURCE_TESTRUNNER_DIR)

.PHONY: CLEAN
CLEAN:
	rm -rf $(BIN_DIR_BASE)

$(MAKEFILE_CONFIG): $(MAKEFILE_CONFIG_TEMPLATE)
	@if [ -e "$@" ]; then \
		touch "$@"; \
	else \
		mkdir -p "$$(dirname $@)"; \
		cp "$<" "$@"; \
	fi
