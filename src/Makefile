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


# Path delimiter, chain commands token
# ──────────────────────────────────────────────────────────────────────────────
ifeq (T,$(SYSTEM_WINDOWS))
PATH_DELIM	:= \\
CMD_DELIM 	:= &
else
PATH_DELIM	:= /
CMD_DELIM 	:= ;
endif

# File extensions
# ──────────────────────────────────────────────────────────────────────────────
ifeq (T,$(SYSTEM_WINDOWS))
STATIC_EXT	:= .lib
SHARED_EXT	:= .dll
BIN_EXT		:= .exe
else ifeq (T,$(SYSTEM_OSX))
STATIC_EXT	:= .a
SHARED_EXT	:= .dylib
BIN_EXT		:= $(EMPTY)
else
STATIC_EXT	:= .a
SHARED_EXT	:= .so
BIN_EXT		:= $(EMPTY)
endif


# DIRECTORY CONFIG
# ══════════════════════════════════════════════════════════════════════════════
PATH_JOIN		 = $(call JOIN,$1,$(PATH_DELIM))
SYSTEM_ROOT_PATH	 = $(call TRIM,$(PATH_DELIM)$(call PATH_JOIN,$1))
PROJECT_ROOT		:= ..
SOURCE_DIR		:= $(call PATH_JOIN,$(PROJECT_ROOT) src)
HEADER_DIR		:= $(SOURCE_DIR)
OBJECT_DIR		:= $(call PATH_JOIN,$(PROJECT_ROOT) obj)
PIC_OBJECT_DIR		:= $(OBJECT_DIR)
SHARED_DIR		:= $(call PATH_JOIN,$(PROJECT_ROOT) shared)
STATIC_DIR		:= $(call PATH_JOIN,$(PROJECT_ROOT) static)
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
FILE_PATH		= $(call PATH_JOIN,$1 $(call TRIM,$2))
SCRIPT_FILE_PATH	= $(call FILE_PATH,$1,$2.rb)
SOURCE_FILE_PATH	= $(call FILE_PATH,$1,$2.c)
HEADER_FILE_PATH	= $(call FILE_PATH,$1,$2.h)
OBJECT_FILE_PATH	= $(call FILE_PATH,$1,$2.o)
PIC_OBJECT_FILE_PATH	= $(call FILE_PATH,$1,$2_pic.o)
STATIC_FILE_PATH	= $(call FILE_PATH,$1,lib$2$(STATIC_EXT))
SHARED_FILE_PATH	= $(call FILE_PATH,$1,lib$2$(SHARED_EXT))
BINARY_FILE_PATH	= $(call FILE_PATH,$1,$2$(BIN_EXT))


# Project
# ─────────────── source ───────────────────────────────────────────────────────
SOURCE_PATH	= $(call SOURCE_FILE_PATH,$(SOURCE_DIR),$1)# implementation
HEADER_PATH	= $(call HEADER_FILE_PATH,$(HEADER_DIR),$1)# interface
# ─────────────── build ────────────────────────────────────────────────────────
OBJECT_PATH   	= $(call OBJECT_FILE_PATH,$(OBJECT_DIR),$1)# object files
PIC_OBJECT_PATH = $(call PIC_OBJECT_FILE_PATH,$(PIC_OBJECT_DIR),$1)# pic object files
STATIC_PATH 	= $(call STATIC_FILE_PATH,$(STATIC_DIR),$1)# static library
SHARED_PATH 	= $(call SHARED_FILE_PATH,$(SHARED_DIR),$1)# shared library
BINARY_PATH 	= $(call BINARY_FILE_PATH,$(BINARY_DIR),$1)# binary executable output

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
CC_PIC_FLAG	:= -fpic

ifeq (T,$(DEVELOPMENT_MODE))
CC_FLAGS	:= -g -Wall $(CC_BASE_FLAGS)
else
CC_FLAGS	:= -O2 -funroll-loops $(CC_BASE_FLAGS)
endif # ifeq (T,$(DEVELOPMENT_MODE))

# Linker
# ──────────────────────────────────────────────────────────────────────────────
ifeq (T,$(SYSTEM_OSX))
LD		:= ld
LD_SHARED_FLAGS	:= -dylib
LD_BIN_FLAGS	:= -no_pie
LD_FLAGS	:= -macosx_version_min 10.11 $(SYS_ARCH_FLAG)
LD_LIB_FLAGS	:= -lc
else
LD		:= gcc
LD_SHARED_FLAGS	:= -shared
LD_BIN_FLAGS	:= $(EMPTY)
LD_FLAGS	:= $(EMPTY)
LD_LIB_FLAGS	:= $(EMPTY)
endif

# Archiver
# ──────────────────────────────────────────────────────────────────────────────
AR		:= ar
AR_FLAGS	:= rcs

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
RM		:= cmd \/C del
RM_FLAGS	:= /F /Q
else
RM		:= rm
RM_FLAGS	:= -rf
endif
# Ruby
# ──────────────────────────────────────────────────────────────────────────────
RUBY		:= ruby
RUBY_FLAGS	:= $(EMPTY)


# COMPILATION UNITS
# ══════════════════════════════════════════════════════════════════════════════
# Independent headers
# ──────────────────────────────────────────────────────────────────────────────
TASTY_REGEX_GLOBALS_HDR	:= $(call HEADER_PATH,tasty_regex_globals)

# tasty_regex_utils
# ══════════════════════════════════════════════════════════════════════════════
TASTY_REGEX_UTILS_SRC 		:= $(call SOURCE_PATH,tasty_regex_utils)
TASTY_REGEX_UTILS_HDR		:= $(call HEADER_PATH,tasty_regex_utils)
TASTY_REGEX_UTILS_OBJ 		:= $(call OBJECT_PATH,tasty_regex_utils)
TASTY_REGEX_UTILS_PIC_OBJ 	:= $(call PIC_OBJECT_PATH,tasty_regex_utils)
# ─────────────── target prequisites ───────────────────────────────────────────
TASTY_REGEX_UTILS_OBJ_PREQS	:= $(TASTY_REGEX_UTILS_SRC) 	\
			   	   $(TASTY_REGEX_UTILS_HDR)
TASTY_REGEX_UTILS_OBJ_GROUP	:= $(TASTY_REGEX_UTILS_OBJ)
TASTY_REGEX_UTILS_PIC_OBJ_PREQS	:= $(TASTY_REGEX_UTILS_OBJ_PREQS)
TASTY_REGEX_UTILS_PIC_OBJ_GROUP	:= $(TASTY_REGEX_UTILS_PIC_OBJ)
# ─────────────── targets ──────────────────────────────────────────────────────
TARGETS				+= $(TASTY_REGEX_UTILS_OBJ)	\
				   $(TASTY_REGEX_UTILS_PIC_OBJ)


# tasty_regex_compile
# ══════════════════════════════════════════════════════════════════════════════
TASTY_REGEX_COMPILE_SRC 	:= $(call SOURCE_PATH,tasty_regex_compile)
TASTY_REGEX_COMPILE_HDR		:= $(call HEADER_PATH,tasty_regex_compile)
TASTY_REGEX_COMPILE_OBJ		:= $(call OBJECT_PATH,tasty_regex_compile)
TASTY_REGEX_COMPILE_PIC_OBJ	:= $(call PIC_OBJECT_PATH,tasty_regex_compile)
# ─────────────── target prequisites ───────────────────────────────────────────
TASTY_REGEX_COMPILE_OBJ_PREQS		:= $(TASTY_REGEX_COMPILE_SRC) 		\
			   	   	   $(TASTY_REGEX_COMPILE_HDR)		\
			   	   	   $(TASTY_REGEX_GLOBALS_HDR)		\
			   	   	   $(TASTY_REGEX_UTILS_HDR)
TASTY_REGEX_COMPILE_OBJ_GROUP		:= $(TASTY_REGEX_COMPILE_OBJ)		\
			   	   	   $(TASTY_REGEX_UTILS_OBJ_GROUP)
TASTY_REGEX_COMPILE_PIC_OBJ_PREQS	:= $(TASTY_REGEX_COMPILE_OBJ_PREQS)
TASTY_REGEX_COMPILE_PIC_OBJ_GROUP	:= $(TASTY_REGEX_COMPILE_PIC_OBJ)	\
			   	   	   $(TASTY_REGEX_UTILS_PIC_OBJ_GROUP)
# ─────────────── targets ──────────────────────────────────────────────────────
TARGETS					+= $(TASTY_REGEX_COMPILE_OBJ)		\
				   	   $(TASTY_REGEX_COMPILE_PIC_OBJ)


# tasty_regex_run
# ══════════════════════════════════════════════════════════════════════════════
TASTY_REGEX_RUN_SRC 		:= $(call SOURCE_PATH,tasty_regex_run)
TASTY_REGEX_RUN_HDR		:= $(call HEADER_PATH,tasty_regex_run)
TASTY_REGEX_RUN_OBJ		:= $(call OBJECT_PATH,tasty_regex_run)
TASTY_REGEX_RUN_PIC_OBJ		:= $(call PIC_OBJECT_PATH,tasty_regex_run)
# ─────────────── target prequisites ───────────────────────────────────────────
TASTY_REGEX_RUN_OBJ_PREQS	:= $(TASTY_REGEX_RUN_SRC) 		\
			   	   $(TASTY_REGEX_RUN_HDR)		\
			   	   $(TASTY_REGEX_GLOBALS_HDR)		\
			   	   $(TASTY_REGEX_UTILS_HDR)
TASTY_REGEX_RUN_OBJ_GROUP	:= $(TASTY_REGEX_RUN_OBJ)		\
			   	   $(TASTY_REGEX_UTILS_OBJ_GROUP)
TASTY_REGEX_RUN_PIC_OBJ_PREQS	:= $(TASTY_REGEX_RUN_OBJ_PREQS)
TASTY_REGEX_RUN_PIC_OBJ_GROUP	:= $(TASTY_REGEX_RUN_PIC_OBJ)		\
			   	   $(TASTY_REGEX_UTILS_PIC_OBJ_GROUP)
# ─────────────── targets ──────────────────────────────────────────────────────
TARGETS				+= $(TASTY_REGEX_RUN_OBJ)		\
				   $(TASTY_REGEX_RUN_PIC_OBJ)


# tasty_regex (main module)
# ══════════════════════════════════════════════════════════════════════════════
TASTY_REGEX_HDR			:= $(call HEADER_PATH,tasty_regex)
TASTY_REGEX_STATIC 		:= $(call STATIC_PATH,tasty_regex)
TASTY_REGEX_SHARED 		:= $(call SHARED_PATH,tasty_regex)
# ─────────────── target prequisites ───────────────────────────────────────────
TASTY_REGEX_STATIC_PREQS	:= $(TASTY_REGEX_COMPILE_OBJ_GROUP) \
				   $(TASTY_REGEX_RUN_OBJ_GROUP)
TASTY_REGEX_SHARED_PREQS	:= $(TASTY_REGEX_COMPILE_PIC_OBJ_GROUP) \
				   $(TASTY_REGEX_RUN_PIC_OBJ_GROUP)
# ─────────────── targets ──────────────────────────────────────────────────────
TARGETS				+= $(TASTY_REGEX_SHARED)	\
			   	   $(TASTY_REGEX_STATIC)


# tasty_grep (entry point)
# ──────────────────────────────────────────────────────────────────────────────
TASTY_GREP_SRC 		:= $(call SOURCE_PATH,tasty_grep)
TASTY_GREP_HDR		:= $(call HEADER_PATH,tasty_grep)
TASTY_GREP_OBJ 		:= $(call OBJECT_PATH,tasty_grep)
TASTY_GREP_BIN 		:= $(call BINARY_PATH,tasty_grep)
# ─────────────── target prequisites ───────────────────────────────────────────
TASTY_GREP_BIN_PREQS	:= $(TASTY_GREP_OBJ)
TASTY_GREP_OBJ_PREQS	:= $(TASTY_GREP_SRC) 	\
			   $(TASTY_GREP_HDR)
# ─────────────── targets ──────────────────────────────────────────────────────
TARGETS			+= $(TASTY_GREP_OBJ)	\
			   $(TASTY_GREP_BIN)




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
				   $(TASTY_REGEX_HDR)			\
			   	   $(UNITY_OBJ_PREQS)
TASTY_REGEX_TRNR_OBJ_PREQS	:= $(TASTY_REGEX_TRNR_SRC)		\
			   	   $(TASTY_REGEX_TEST_OBJ_PREQS)
TASTY_REGEX_TEST_BIN_PREQS	:= $(TASTY_REGEX_TRNR_OBJ)		\
				   $(TASTY_REGEX_TEST_OBJ)		\
			   	   $(TASTY_REGEX_STATIC)		\
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
all: $(TARGETS)

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


## MAKE PROJECT
# ══════════════════════════════════════════════════════════════════════════════
# make tasty_grep binary
# ──────────────────────────────────────────────────────────────────────────────
$(TASTY_GREP_BIN): $(TASTY_GREP_BIN_PREQS)
	$(LD) $(LD_FLAGS) $(LD_BIN_FLAGS) $^ $(LD_LIB_FLAGS) -o $@

# make tasty_grep object
# ──────────────────────────────────────────────────────────────────────────────
$(TASTY_GREP_OBJ): $(TASTY_GREP_OBJ_PREQS)
	$(CC) $(CC_FLAGS) -c $< -o $@

# make tasty_regex static library
# ──────────────────────────────────────────────────────────────────────────────
$(TASTY_REGEX_STATIC): $(TASTY_REGEX_STATIC_PREQS)
	$(AR) $(AR_FLAGS) $@ $^

# make tasty_regex shared library
# ──────────────────────────────────────────────────────────────────────────────
$(TASTY_REGEX_SHARED): $(TASTY_REGEX_SHARED_PREQS)
	$(LD) $(LD_FLAGS) $(LD_SHARED_FLAGS) $^ $(LD_LIB_FLAGS) -o $@

# make tasty_regex_run object
# ──────────────────────────────────────────────────────────────────────────────
$(TASTY_REGEX_RUN_OBJ): $(TASTY_REGEX_RUN_OBJ_PREQS)
	$(CC) $(CC_FLAGS) -c $< -o $@

# make tasty_regex_run pic object
# ──────────────────────────────────────────────────────────────────────────────
$(TASTY_REGEX_RUN_PIC_OBJ): $(TASTY_REGEX_RUN_PIC_OBJ_PREQS)
	$(CC) $(CC_FLAGS) $(CC_PIC_FLAG) -c $< -o $@

# make tasty_regex_compile object
# ──────────────────────────────────────────────────────────────────────────────
$(TASTY_REGEX_COMPILE_OBJ): $(TASTY_REGEX_COMPILE_OBJ_PREQS)
	$(CC) $(CC_FLAGS) -c $< -o $@

# make tasty_regex_compile pic object
# ──────────────────────────────────────────────────────────────────────────────
$(TASTY_REGEX_COMPILE_PIC_OBJ): $(TASTY_REGEX_COMPILE_PIC_OBJ_PREQS)
	$(CC) $(CC_FLAGS) $(CC_PIC_FLAG) -c $< -o $@

# make tasty_regex_utils object
# ──────────────────────────────────────────────────────────────────────────────
$(TASTY_REGEX_UTILS_OBJ): $(TASTY_REGEX_UTILS_OBJ_PREQS)
	$(CC) $(CC_FLAGS) -c $< -o $@

# make tasty_regex_utils pic object
# ──────────────────────────────────────────────────────────────────────────────
$(TASTY_REGEX_UTILS_PIC_OBJ): $(TASTY_REGEX_UTILS_PIC_OBJ_PREQS)
	$(CC) $(CC_FLAGS) $(CC_PIC_FLAG) -c $< -o $@



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
	$(LD) $(LD_FLAGS) $(LD_BIN_FLAGS) $^ $(UNITY_LD_LIB_FLAGS) $(LD_LIB_FLAGS) -o $@

$(TASTY_REGEX_TRNR_OBJ): $(TASTY_REGEX_TRNR_OBJ_PREQS)
	$(CC) $(UNITY_CC_FLAGS) -c $< -o $@

$(TASTY_REGEX_TEST_OBJ): $(TASTY_REGEX_TEST_OBJ_PREQS)
	$(CC) $(UNITY_CC_FLAGS) -c $< -o $@

$(TASTY_REGEX_TRNR_SRC): $(TASTY_REGEX_TRNR_SRC_PREQS)
	$(RUBY) $(RUBY_FLAGS) $(UNITY_GEN_TRNR_SCRIPT) $< $@
endif # ifeq (T,$(DEVELOPMENT_MODE))
