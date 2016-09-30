# ══════════════════════════════════════════════════════════════════════════════
# ──────────────────────────────────────────────────────────────────────────────
#				CONFIGURATION
# ──────────────────────────────────────────────────────────────────────────────
# ══════════════════════════════════════════════════════════════════════════════
#
#
## MISC
# ══════════════════════════════════════════════════════════════════════════════
# Development Mode
# ──────────────────────────────────────────────────────────────────────────────
# toggles utility flags, make targets
DEVELOPMENT_MODE = T

# Phony Targets
# ──────────────────────────────────────────────────────────────────────────────
.PHONY: all run_tests clean


# String Utils
# ──────────────────────────────────────────────────────────────────────────────
EMPTY	       :=
SPACE	       := $(EMPTY) $(EMPTY)
TAB	       := $(EMPTY)	$(EMPTY)
TRIM		= $(subst $(SPACE),$(EMPTY),$1)
JOIN	        = $(subst $(SPACE),$2,$(strip $1))
define NEWLINE :=


endef

# SYSTEM CONFIG
# ══════════════════════════════════════════════════════════════════════════════
# extended from user 'Trevor Robinson''s response to stackoverflow question:
#
# 	http://stackoverflow.com/questions/714100/os-detecting-makefile

ifeq ($(OS),Windows_NT)
        SYSTEM_WINDOWS 	:= T
        SYS_ENV_FLAGS 	:= -DWIN32

        ifeq      (AMD64,$(PROCESSOR_ARCHITECTURE))
                ARCH_X86_64	:= T
                SYS_ARCH_FLAG	:= -arch x86_64
                SYS_ENV_FLAGS	+= -DAMD_64

        else ifeq (x86,$(PROCESSOR_ARCHITECTURE))
                ARCH_I386	:= T
                SYS_ARCH_FLAG	:= -arch i386
                SYS_ENV_FLAGS	+= -DIA_32
        endif
else
        UNAME_S := $(shell uname -s)

        ifeq      ($(UNAME_S),Linux)
                SYSTEM_LINUX 	:= T
                SYS_ENV_FLAGS 	:= -DLINUX

        else ifeq ($(UNAME_S),Darwin)
                SYSTEM_OSX 	:= T
                SYS_ENV_FLAGS 	:= -DOSX
        endif

        UNAME_M := $(shell uname -m)

        ifeq      (x86_64,$(UNAME_M))
                ARCH_X86_64	:= T
                SYS_ARCH_FLAG	:= -arch x86_64
                SYS_ENV_FLAGS	+= -DAMD_64

        else ifneq (,$(filter %86,$(UNAME_M)))
                ARCH_I386	:= T
                SYS_ARCH_FLAG	:= -arch i386
                SYS_ENV_FLAGS 	+= -DIA_32

        else ifneq (,$(filter arm%,$(UNAME_M)))

                ifneq (,$(filter %32%,$(UNAME_M)))
                        ARCH_ARM_32	:= T
                        SYS_ARCH_FLAG	:= -arch AArch32
                        SYS_ENV_FLAGS 	+= -DARM_32
                else
                        ARCH_ARM_64	:= T
                        SYS_ARCH_FLAG	:= -arch AArch64
                        SYS_ENV_FLAGS 	+= -DARM_64
                endif
        endif
endif


# Path delimiter, chain commands token, executable file extension
# ──────────────────────────────────────────────────────────────────────────────
ifeq (T,$(SYSTEM_WINDOWS))
PATH_DELIM	:= \\
CMD_DELIM 	:= &
BIN_EXT		:= .exe
else
PATH_DELIM	:= /
CMD_DELIM 	:= ;
BIN_EXT		:= $(EMPTY)
endif




# DIRECTORY CONFIG
# ══════════════════════════════════════════════════════════════════════════════
PATH_JOIN		 = $(call JOIN,$1,$(PATH_DELIM))
SYSTEM_ROOT_PATH	 = $(call TRIM,$(PATH_DELIM)$(call PATH_JOIN,$1))
PROJECT 		:= tasty_grep
PROJECT_ROOT		:= ..
SOURCE_DIR		:= $(call PATH_JOIN,$(PROJECT_ROOT) src)
HEADER_DIR		:= $(SOURCE_DIR)
OBJECT_DIR		:= $(call PATH_JOIN,$(PROJECT_ROOT) obj)
BINARY_DIR		:= $(call PATH_JOIN,$(PROJECT_ROOT) bin)
TEST_SOURCE_DIR		:= $(call PATH_JOIN,$(PROJECT_ROOT) test)
TEST_HEADER_DIR		:= $(TEST_SOURCE_DIR)
TEST_RUNNER_SOURCE_DIR	:= $(call PATH_JOIN,$(TEST_SOURCE_DIR) test_runners)
TEST_OBJECT_DIR		:= $(OBJECT_DIR)
TEST_RUNNER_OBJECT_DIR	:= $(TEST_OBJECT_DIR)
TEST_BINARY_DIR		:= $(TEST_SOURCE_DIR)
UNITY_ROOT		:= $(call PATH_JOIN,$(TEST_SOURCE_DIR) unity)
UNITY_SOURCE_DIR	:= $(call PATH_JOIN,$(UNITY_ROOT) src)
UNITY_HEADER_DIR	:= $(UNITY_SOURCE_DIR)
UNITY_OBJECT_DIR	:= $(call PATH_JOIN,$(UNITY_ROOT) obj)
UNITY_AUTO_DIR  	:= $(call PATH_JOIN,$(UNITY_ROOT) auto)


# PATH CONFIG
# ══════════════════════════════════════════════════════════════════════════════
FILE_PATH			= $(call PATH_JOIN,$1 $(call TRIM,$2))
SCRIPT_FILE_PATH		= $(call FILE_PATH,$1,$2.rb)
SOURCE_FILE_PATH		= $(call FILE_PATH,$1,$2.c)
HEADER_FILE_PATH		= $(call FILE_PATH,$1,$2.h)
OBJECT_FILE_PATH		= $(call FILE_PATH,$1,$2.o)
BINARY_FILE_PATH		= $(call FILE_PATH,$1,$2$(BIN_EXT))


# Project
# ─────────────── source ───────────────────────────────────────────────────────
SOURCE_PATH		= $(call SOURCE_FILE_PATH,$(SOURCE_DIR),$1)# implementation
HEADER_PATH		= $(call HEADER_FILE_PATH,$(HEADER_DIR),$1)# interface
# ─────────────── build ────────────────────────────────────────────────────────
OBJECT_PATH   		= $(call OBJECT_FILE_PATH,$(OBJECT_DIR),$1)# object files
BINARY_PATH 		= $(call BINARY_FILE_PATH,$(BINARY_DIR),$1)# binary executable output

# Test
# ─────────────── source ───────────────────────────────────────────────────────
TEST_SOURCE_PATH	= $(call SOURCE_FILE_PATH,$(TEST_SOURCE_DIR),$(call JOIN,$1 test,_))# Unity unit test implementation
TEST_HEADER_PATH	= $(call HEADER_FILE_PATH,$(TEST_HEADER_DIR),$(call JOIN,$1 test,_))# Unity unit test interface
# ─────────────── build ────────────────────────────────────────────────────────
TEST_RUNNER_SOURCE_PATH	= $(call SOURCE_FILE_PATH,$(TEST_RUNNER_SOURCE_DIR),$(call JOIN,$1 test_runner,_))# Unity test runners
TEST_RUNNER_OBJECT_PATH	= $(call OBJECT_FILE_PATH,$(TEST_RUNNER_OBJECT_DIR),$(call JOIN,$1 test_runner,_))# Unity test runner object files
TEST_OBJECT_PATH 	= $(call OBJECT_FILE_PATH,$(TEST_OBJECT_DIR),$(call JOIN,$1 test,_))# Unity unit test object files
TEST_BINARY_PATH 	= $(call BINARY_FILE_PATH,$(TEST_BINARY_DIR),$(call JOIN,$1 test,_))# binary executable output


ifeq (T,$(DEVELOPMENT_MODE))
# Unity Test Framework
# ─────────────── source ───────────────────────────────────────────────────────
UNITY_SOURCE_PATH	= $(call SOURCE_FILE_PATH,$(UNITY_SOURCE_DIR),$1)# Unity framework implementation
UNITY_HEADER_PATH	= $(call HEADER_FILE_PATH,$(UNITY_HEADER_DIR),$1)# Uity framework interface
# ─────────────── build ────────────────────────────────────────────────────────
UNITY_SCRIPT_PATH	= $(call SCRIPT_FILE_PATH,$(UNITY_AUTO_DIR),$1)# Unity framework scripts
UNITY_OBJECT_PATH 	= $(call OBJECT_FILE_PATH,$(UNITY_OBJECT_DIR),$1)# Unity framework object files
endif # ifeq (T,$(DEVELOPMENT_MODE))



# UTILITY CONFIG
# ══════════════════════════════════════════════════════════════════════════════
# C Compiler
# ──────────────────────────────────────────────────────────────────────────────
CC		:= gcc
CC_ENV_FLAGS	:= -D__USE_FIXED_PROTOTYPES__ -D_GNU_SOURCE $(SYS_ENV_FLAGS)
CC_BASE_FLAGS	:= -std=gnu99 -march=native $(CC_ENV_FLAGS) -I$(HEADER_DIR)

ifeq (T,$(DEVELOPMENT_MODE))
CC_FLAGS	:= -g -Wall $(CC_BASE_FLAGS)
else
CC_FLAGS	:= -O2 -funroll-loops $(CC_BASE_FLAGS)
endif

# Linker
# ──────────────────────────────────────────────────────────────────────────────
ifeq (T,$(SYSTEM_OSX))
LD		:= ld
LD_FLAGS	:= -macosx_version_min 10.11 -no_pie $(SYS_ARCH_FLAG)
else
LD		:= gcc
LD_FLAGS	:= $(EMPTY)
endif

LD_LIB_FLAGS	:= $(THREADS_LIB_FLAG) $(SYSTEM_LIB_FLAGS) $(LIBMYSQLCLIENT) -lc

# Make Directory
# ──────────────────────────────────────────────────────────────────────────────
MKDIR	    	:= mkdir
MKDIR_FLAGS 	:= $(EMPTY)

ifeq (T,$(SYSTEM_WINDOWS))
MKDIR_IF_DNE	= if not exist $1 $(MKDIR) $(MKDIR_FLAGS) $1
else
MKDIR_IF_DNE	= $(MKDIR) $(MKDIR_FLAGS) -p $1
endif

# Cleaner
# ──────────────────────────────────────────────────────────────────────────────
ifeq (T,$(SYSTEM_WINDOWS))
# 'clean' command exceeds 8192 character limit for windows
# RM		:= cmd \/C del
# RM_FLAGS	:= /F /Q
RM		:= rm
RM_FLAGS	:= -rf
else
RM		:= rm
RM_FLAGS	:= -rf
endif
# Ruby
# ──────────────────────────────────────────────────────────────────────────────
RUBY		:= ruby
RUBY_FLAGS	:= $(EMPTY)




# build directories
# ──────────────────────────────────────────────────────────────────────────────
BUILD_DIRS	:= $(OBJECT_DIR)		\
		   $(UNITY_OBJECT_DIR)		\
		   $(BINARY_DIR)


# tasty_regex (main module)
# ══════════════════════════════════════════════════════════════════════════════
TASTY_REGEX_SRC 	:= $(call SOURCE_PATH,tasty_regex)
TASTY_REGEX_HDR		:= $(call HEADER_PATH,tasty_regex)
TASTY_REGEX_OBJ 	:= $(call OBJECT_PATH,tasty_regex)
# ─────────────── target prequisites ───────────────────────────────────────────
TASTY_REGEX_OBJ_PREQS	:= $(TASTY_REGEX_SRC) 		\
			   $(TASTY_REGEX_HDR)
TASTY_REGEX_OBJ_GROUP	:= $(TASTY_REGEX_OBJ)
# ─────────────── targets ──────────────────────────────────────────────────────
TARGETS			+= $(TASTY_REGEX_OBJ)


# tasty_grep (entry point)
# ──────────────────────────────────────────────────────────────────────────────
PROJECT_SRC 		:= $(call SOURCE_PATH,$(PROJECT))
PROJECT_HDR		:= $(call HEADER_PATH,$(PROJECT))
PROJECT_OBJ 		:= $(call OBJECT_PATH,$(PROJECT))
PROJECT_BIN 		:= $(call BINARY_PATH,$(PROJECT))
# ─────────────── target prequisites ───────────────────────────────────────────
PROJECT_BIN_PREQS	:= $(PROJECT_OBJ)
PROJECT_OBJ_PREQS	:= $(PROJECT_SRC) 	\
			   $(PROJECT_HDR)
# ─────────────── targets ──────────────────────────────────────────────────────
TARGETS			+= $(PROJECT_OBJ)	\
			   $(PROJECT_BIN)




ifeq (T,$(DEVELOPMENT_MODE))
## UNITY TEST FRAMEWORK
# ══════════════════════════════════════════════════════════════════════════════
UNITY_SRC 		:= $(call UNITY_SOURCE_PATH,unity)
UNITY_HDR 		:= $(call UNITY_HEADER_PATH,unity)
UNITY_CONFIG_HDR 	:= $(call UNITY_HEADER_PATH,unity_config)
UNITY_INTERNALS_HDR 	:= $(call UNITY_HEADER_PATH,unity_internals)
UNITY_OBJ 		:= $(call UNITY_OBJECT_PATH,unity)
UNITY_GEN_TRNR_SCRIPT	:= $(call UNITY_SCRIPT_PATH,generate_test_runner)
UNITY_ENV_FLAGS 	:= -DUNITY_INCLUDE_CONFIG_H
UNITY_CC_FLAGS		:= $(CC_FLAGS) $(UNITY_ENV_FLAGS) -I$(UNITY_HEADER_DIR)
UNITY_LD_LIB_FLAGS	:= $(EMPTY)
# ─────────────── run all tests ────────────────────────────────────────────────
TEST_FILES_GLOB		:= $(call BINARY_FILE_PATH,$(TEST_BINARY_DIR),*_test)
ifeq (T,$(SYSTEM_WINDOWS))
RUN_ALL_TESTS		:= FORFILES /M $(TEST_FILES_GLOB) /C "cmd /c @file"
else
RUN_ALL_TESTS		:= for test in $(TEST_FILES_GLOB); do "./$$test"; done
endif
# ─────────────── target prequisites ───────────────────────────────────────────
UNITY_OBJ_PREQS		:= $(UNITY_SRC)			\
			   $(UNITY_HDR)			\
			   $(UNITY_INTERNALS_HDR)	\
			   $(UNITY_CONFIG_HDR)
# ─────────────── targets ──────────────────────────────────────────────────────
BUILD_DIRS 		+= $(TEST_RUNNER_SOURCE_DIR)
TARGETS			+= $(UNITY_OBJ)



# tasty_regex unit test
# ──────────────────────────────────────────────────────────────────────────────
TASTY_REGEX_TEST_SRC		:= $(call TEST_SOURCE_PATH,tasty_regex)
TASTY_REGEX_TRNR_SRC		:= $(call TEST_RUNNER_SOURCE_PATH,tasty_regex)
TASTY_REGEX_TRNR_OBJ		:= $(call TEST_RUNNER_OBJECT_PATH,tasty_regex)
TASTY_REGEX_TEST_OBJ		:= $(call TEST_OBJECT_PATH,tasty_regex)
TASTY_REGEX_TEST_BIN		:= $(call TEST_BINARY_PATH,tasty_regex)
# ─────────────── target prequisites ───────────────────────────────────────────
TASTY_REGEX_TRNR_SRC_PREQS	:= $(TASTY_REGEX_TEST_SRC)		\
				   $(TEST_RUNNER_SOURCE_DIR)
TASTY_REGEX_TEST_OBJ_PREQS	:= $(TASTY_REGEX_TEST_SRC)		\
			   	   $(TASTY_REGEX_OBJ_PREQS) 		\
			   	   $(UNITY_OBJ_PREQS)
TASTY_REGEX_TRNR_OBJ_PREQS	:= $(TASTY_REGEX_TRNR_SRC)		\
			   	   $(TASTY_REGEX_TEST_OBJ_PREQS)
TASTY_REGEX_TEST_BIN_PREQS	:= $(TASTY_REGEX_TRNR_OBJ)		\
				   $(TASTY_REGEX_TEST_OBJ)		\
			   	   $(TASTY_REGEX_OBJ_GROUP)		\
			   	   $(UNITY_OBJ)
# ─────────────── targets ──────────────────────────────────────────────────────
TEST_BINARIES			+= $(TASTY_REGEX_TEST_BIN)
TARGETS				+= $(TASTY_REGEX_TRNR_SRC)		\
				   $(TASTY_REGEX_TRNR_OBJ)		\
				   $(TASTY_REGEX_TEST_OBJ)		\
				   $(TASTY_REGEX_TEST_BIN)
endif # ifeq (T,$(DEVELOPMENT_MODE))



# ══════════════════════════════════════════════════════════════════════════════
# ──────────────────────────────────────────────────────────────────────────────
# 				MAKE RULES
# ──────────────────────────────────────────────────────────────────────────────
# ══════════════════════════════════════════════════════════════════════════════
#
#
## COMPOSITE RULES
# ══════════════════════════════════════════════════════════════════════════════
# make all targets
# ──────────────────────────────────────────────────────────────────────────────
all:  $(BUILD_DIRS) $(TARGETS)

# run all unit tests
# ──────────────────────────────────────────────────────────────────────────────
ifeq (T,$(DEVELOPMENT_MODE))
run_tests: $(TEST_BINARIES)
	$(RUN_ALL_TESTS)
else
run_tests:
	@echo set DEVELOPMENT_MODE := T at the top of Makefile to build and run unit tests
endif # ifeq (T,$(DEVELOPMENT_MODE))

# remove all targets
# ──────────────────────────────────────────────────────────────────────────────
clean:
	$(RM) $(RM_FLAGS) $(TARGETS)

# ensure build directories are present
# ──────────────────────────────────────────────────────────────────────────────
$(TEST_RUNNER_SOURCE_DIR):
	$(MKDIR) $(MKDIR_FLAGS) $@

$(OBJECT_DIR):
	$(MKDIR) $(MKDIR_FLAGS) $@

$(UNITY_OBJECT_DIR):
	$(MKDIR) $(MKDIR_FLAGS) $@

$(BINARY_DIR):
	$(MKDIR) $(MKDIR_FLAGS) $@


## MAKE PROJECT
# ══════════════════════════════════════════════════════════════════════════════
# make tasty_grep binary
# ──────────────────────────────────────────────────────────────────────────────
$(PROJECT_BIN): $(PROJECT_BIN_PREQS)
	$(LD) $(LD_FLAGS) $^ $(LD_LIB_FLAGS) -o $@

# # make tasty_grep object
# # ──────────────────────────────────────────────────────────────────────────────
$(PROJECT_OBJ): $(PROJECT_OBJ_PREQS)
	$(CC) $(CC_FLAGS) -c $< -o $@

# # make tasty_regex object
# # ──────────────────────────────────────────────────────────────────────────────
$(TASTY_REGEX_OBJ): $(TASTY_REGEX_OBJ_PREQS)
	$(CC) $(CC_FLAGS) -c $< -o $@



ifeq (T,$(DEVELOPMENT_MODE))
## MAKE UNITY TEST FRAMEWORK
# ══════════════════════════════════════════════════════════════════════════════
# make unity object
# ──────────────────────────────────────────────────────────────────────────────
$(UNITY_OBJ): $(UNITY_OBJ_PREQS)
	$(CC) $(UNITY_CC_FLAGS) -c $< -o $@


# for tasty_regex
# make:
# 	tasty_regex_test		binary,
# 	tasty_regex_test_runner 	object,
# 	tasty_regex_test		object, and
# 	tasty_regex_test_runner	source
# ──────────────────────────────────────────────────────────────────────────────
$(TASTY_REGEX_TEST_BIN): $(TASTY_REGEX_TEST_BIN_PREQS)
	$(LD) $(LD_FLAGS) $^ $(UNITY_LD_LIB_FLAGS) $(LD_LIB_FLAGS) -o $@

$(TASTY_REGEX_TRNR_OBJ): $(TASTY_REGEX_TRNR_OBJ_PREQS)
	$(CC) $(UNITY_CC_FLAGS) -c $< -o $@

$(TASTY_REGEX_TEST_OBJ): $(TASTY_REGEX_TEST_OBJ_PREQS)
	$(CC) $(UNITY_CC_FLAGS) -c $< -o $@

$(TASTY_REGEX_TRNR_SRC): $(TASTY_REGEX_TRNR_SRC_PREQS)
	$(RUBY) $(RUBY_FLAGS) $(UNITY_GEN_TRNR_SCRIPT) $< $@
endif # ifeq (T,$(DEVELOPMENT_MODE))
