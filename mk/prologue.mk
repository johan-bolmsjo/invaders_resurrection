# Include this file the very beginning of the top level makefile

###############################################################################
# Terminology:
#
#   Cross compilation terminology 'host' and 'build' is adopted from
#   https://www.gnu.org/software/automake/manual/html_node/Cross_002dCompilation.html.
#   Host is implied, standard variables CC, CXX etc are for the host that will
#   run the software. Some flags variables are prefixed with BUILD_ to genereate
#   software that will be run on the build host.

###############################################################################
# Variable usage:
#
# Makefile (standard) variables read from env.
# See: https://www.gnu.org/software/make/manual/html_node/Implicit-Variables.html
#
#   AR        Archive program for static libs
#   CC        C compiler
#   CFLAGS    C compiler flags
#   CPPFLAGS  C/C++ preprocessor flags (-D, -I)
#   CXX       C++ compiler
#   CXXFLAGS  C++ compiler flags
#   LDFLAGS   Linker flags
#
#   Variants of the above prefixed with BUILD_ are also read in for the
#   compiler to compile programs and libraries for the build host (such
#   as unit tests).
#
# Other variables read from  env.
#
#   GTEST_PREFIX      Installation prefix of goole test (default: /usr)
#   OUTDIR            Bulid products base directory
#
# Custom variables updated by makefiles (documented below).
#
#   PHONY_TARGETS     List with phony targets (shown by 'make show')
#
# Custom variable prefixes (set by makefiles).
# Variables with the following naming are assumed to have special meaning.
#
#   OBJS_name
#     Object files for target host.
#   OBJS_BUILD_name
#     Object files for build host.
#   OBJS_TEST_name
#     Object files for tests for build host.
#   TARGET_EXE_name
#     Program for target host.
#   TARGET_LIB_name
#     Library for target host.
#   TARGET_BUILD_EXE_name
#     Program for build host.
#   TARGET_TEST_EXE_name
#     Test program for build host.
#

###############################################################################
# Convert flag variables to "simply expanded variables" to avoid surprises when
# overriding them in target specific variables.
CFLAGS         := $(CFLAGS)
BUILD_CFLAGS   := $(BUILD_CFLAGS)

CPPFLAGS       := $(CPPFLAGS)
BUILD_CPPFLAGS := $(BUILD_CPPFLAGS)

CXXFLAGS       := $(CXXFLAGS)
BUILD_CXXFLAGS := $(BUILD_CXXFLAGS)

LDFLAGS        := $(LDFLAGS)
BUILD_LDFLAGS  := $(BUILD_LDFLAGS)

ifeq ($(CFLAGS),)
CFLAGS := -O0 -g
endif

ifeq ($(BUILD_CFLAGS),)
BUILD_CFLAGS := -O0 -g
endif

ifeq ($(CXXFLAGS),)
CXXFLAGS := -O0 -g
endif

ifeq ($(BUILD_CXXFLAGS),)
BUILD_CXXFLAGS := -O0 -g
endif

ifeq ($(GTEST_PREFIX),)
GTEST_PREFIX := /usr
endif

GTEST_CFLAGS   :=
GTEST_CXXFLAGS :=
GTEST_CPPFLAGS := -I$(GTEST_PREFIX)/include -DTEST
GTEST_LDFLAGS  := -L$(GTEST_PREFIX)/lib -lgtest

# The address sanitizer is an excellent tool to find memory violation type of bugs.
ifeq ($(GTEST_DISABLE_ASAN),)
GTEST_CFLAGS   += -fsanitize=address -fno-omit-frame-pointer
GTEST_CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
GTEST_LDFLAGS  += -fsanitize=address
endif

# List maintained for 'make show'.
PHONY_TARGETS :=

# Makefile checks
OUTDIR_CHECK     := $(OUTDIR)/check

# Generated sources
OUTDIR_SRCS      := $(OUTDIR)/srcs

# Generated data files
OUTDIR_DATA      := $(OUTDIR)/data

# Build produces for the target host.
OUTDIR_EXE  := $(OUTDIR)/host/bin
OUTDIR_LIB  := $(OUTDIR)/host/lib
OUTDIR_OBJS := $(OUTDIR)/host/obj

# Build products for the build host (e.g. code generators).
OUTDIR_BUILD_EXE  := $(OUTDIR)/build/bin
OUTDIR_BUILD_OBJS := $(OUTDIR)/build/obj

# Test related build products for the build host.
OUTDIR_TEST_EXE  := $(OUTDIR)/test/bin
OUTDIR_TEST_OBJS := $(OUTDIR)/test/obj

###############################################################################
# Set shell to be used to avoid possibly different behavior in different
# environments because of the shell used.
SHELL = /bin/sh

# Disable all built-in rules, specify rules explicitly to avoid surprises.
.SUFFIXES:

# See https://www.gnu.org/software/make/manual/html_node/Secondary-Expansion.html
.SECONDEXPANSION:

# Assume there will be a phony 'all' target defined by the top level makefile
# and make it the default.
.PHONY: all
PHONY_TARGETS += all

# Display all valid make targets when run without arguments. This is similar to
# how 'git' functions. It's convention to invoke 'all' by default but then the
# user will never figure out that there is a 'help' command available. I think
# this has better usabillity.
.DEFAULT_GOAL := help
###############################################################################
# Basic commands.
RM := rm -f
CP := cp
MV := mv

###############################################################################
# Command verbosity support, make with V=1 or VERBOSE=1
# Commands can be prefixed with $(Q) to make them honor the verbosity level.

ifeq ($(or $(V),$(VERBOSE)),1)
	Q         :=
	Q_compile :=
	Q_link    :=
	Q_ar      :=
else
	Q         := @
	Q_compile = @echo "Compiling $<" ;
	Q_link    = @echo "Linking $(notdir $@)" ;
	Q_ar      = @echo "Creating archive $(notdir $@)" ;
endif

###############################################################################
# Compilation and link commands

# Function that compiles a C source file and generates dependency info.
#
# Arg1: Optional extra compilation arguments
#
cc-compile = $(Q_compile) $(strip $(CC) $(CFLAGS) $(CPPFLAGS) $1 \
	-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -c -o $@ $<)

# Function that compiles a C++ source file and generates dependency info.
#
# Arg1: Optional extra compilation arguments
#
cxx-compile = $(Q_compile) $(strip $(CXX) $(CXXFLAGS) $(CPPFLAGS) $1 \
	-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -c -o $@ $<)

# Function that links a program or shared library using the C++ compiler.
#
# Arg1: Optional extra linker arguments
#
cxx-link = $(Q_link) $(strip $(CXX) $(filter %.o,$^) -o $@ $(LDFLAGS) $1)

# Same as cc-compile but for compiling programs such as unit tests to be run on the build host.
build-cc-compile = $(Q_compile) $(strip $(BUILD_CC) $(BUILD_CFLAGS) $(BUILD_CPPFLAGS) $1 \
	-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -c -o $@ $<)

# Same as cxx-compile but for compiling programs such as unit tests to be run on the build host.
build-cxx-compile = $(Q_compile) $(strip $(BUILD_CXX) $(BUILD_CXXFLAGS) $(BUILD_CPPFLAGS) $1 \
	-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -c -o $@ $<)

# Same as cxx-link but for linking programs such as unit tests to be run on the build host.
#
# Arg1: Optional extra linker arguments
#
build-cxx-link = $(Q_link) $(strip $(BUILD_CXX) $(filter %.o,$^) -o $@ $(BUILD_LDFLAGS) $1)

# Function that creates an archive (static lib):
ar-create = $(Q_ar) $(RM) $@ && $(AR) rsc $@ $(filter %.o,$^)

###############################################################################
# Compilation rules

$(OUTDIR_OBJS)/%.c.o : %.c
	$(call cc-compile)
$(OUTDIR_OBJS)/%.cpp.o : %.cpp
	$(call cxx-compile)
$(OUTDIR_TEST_OBJS)/%.c.o : %.c
	$(call build-cc-compile,$(GTEST_CFLAGS) $(GTEST_CPPFLAGS))
$(OUTDIR_TEST_OBJS)/%.cpp.o : %.cpp
	$(call build-cxx-compile,$(GTEST_CXXFLAGS) $(GTEST_CPPFLAGS))
$(OUTDIR_BUILD_OBJS)/%.c.o : %.c
	$(call build-cc-compile)
$(OUTDIR_BUILD_OBJS)/%.cpp.o : %.cpp
	$(call build-cxx-compile)

###############################################################################
define define-srcs_
OBJS_$(1) = $(foreach file,$2,$(OUTDIR_OBJS)/$(file).o)
endef

define define-build-srcs_
OBJS_BUILD_$(1) = $(foreach file,$2,$(OUTDIR_BUILD_OBJS)/$(file).o)
endef

define define-test-srcs_
OBJS_TEST_$(1) = $(foreach file,$2,$(OUTDIR_TEST_OBJS)/$(file).o)
endef

# Define sources to be compiled for the target host.
#
# Arg1: Sources collection name
# Arg2: List with source files relative project root directory
#
define-srcs = $(eval $(call define-srcs_,$(strip $1),$2))

# Define sources to be compiled for the build host (e.g. code generators).
# Takes the same arguements as 'define-srcs'.
define-build-srcs = $(eval $(call define-build-srcs_,$(strip $1),$2))

# Define sources to be compiled for tests for the build host.
# Takes the same arguements as 'define-srcs'.
define-test-srcs = $(eval $(call define-test-srcs_,$(strip $1),$2))

###############################################################################
define define-lib.a_
TARGET_LIB_$(1) = $(OUTDIR_LIB)/lib$(1).a
$(OUTDIR_LIB)/lib$(1).a: $(foreach var,$2,$$$$(OBJS_$(var)))
	$$(call ar-create)

.PHONY: lib.a/$(1)
PHONY_TARGETS += lib.a/$(1)
lib.a/$(1): $(OUTDIR_LIB)/lib$(1).a
endef

# Create a static library for the target host.
#
# Arg1: Make target name
# Arg2: List of sources collection names
#
define-lib.a = $(eval $(call define-lib.a_,$(strip $1),$2))

###############################################################################
define define-exe_
TARGET_EXE_$(1) = $(OUTDIR_EXE)/$(1)
$(OUTDIR_EXE)/$(1): $(foreach var,$2,$$$$(OBJS_$(var)))
	$$(call cxx-link)

.PHONY: exe/$(1)
PHONY_TARGETS += exe/$(1)
exe/$(1): $(OUTDIR_EXE)/$(1)
endef

# Create a program for the target host.
#
# Arg1: Make target name
# Arg2: List of sources collection names
#
define-exe = $(eval $(call define-exe_,$(strip $1),$2))

###############################################################################
define define-test-exe_
TARGET_TEST_EXE_$(1) = $(OUTDIR_TEST_EXE)/$(1)
$(OUTDIR_TEST_EXE)/$(1): $(foreach var,$2,$$$$(OBJS_TEST_$(var)))
	$$(call build-cxx-link,$(GTEST_LDFLAGS))

.PHONY: test/$(1)
PHONY_TARGETS += test/$(1)
test/$(1): $(OUTDIR_TEST_EXE)/$(1)

.PHONY: runtest/$(1)
PHONY_TARGETS += runtest/$(1)
runtest/$(1): $(OUTDIR_TEST_EXE)/$(1)
	@echo "Executing $(OUTDIR_TEST_EXE)/$(1)" && $(OUTDIR_TEST_EXE)/$(1)
endef

# Create a test program for the build host.
#
# Arg1: Make target name
# Arg2: List of sources collection names
#
define-test-exe = $(eval $(call define-test-exe_,$(strip $1),$2))

###############################################################################
define define-build-exe_
TARGET_BUILD_EXE_$(1) = $(OUTDIR_BUILD_EXE)/$(1)
$(OUTDIR_BUILD_EXE)/$(1): $(foreach var,$2,$$$$(OBJS_BUILD_$(var))) $3
	$$(call build-cxx-link)

.PHONY: build/$(1)
PHONY_TARGETS += build/$(1)
build/$(1): $(OUTDIR_BUILD_EXE)/$(1)
endef

# Create a program for the build host.
#
# Arg1: Target name
# Arg2: List of sources collection names
#
define-build-exe = $(eval $(call define-build-exe_,$(strip $1),$2))

###############################################################################
# Function that expand all OBJS_ prefixed variables.
expand-objs-all = $(sort $(foreach var,$(filter OBJS_%,$(.VARIABLES)),$($(var))))

# Function that expands all variables prefixed with TARGET_TEST_EXE_
expand-targets-test = $(foreach var,$(filter TARGET_TEST_EXE_%,$(.VARIABLES)),$($(var)))

# Function that expands all target prefixed variables.
expand-targets-all = $(foreach var,$(filter TARGET_%,$(.VARIABLES)),$($(var)))

###############################################################################
# Environment checks

# Create check program for if CC produce binaries that can be executed on the build host.
TARGET_CHECK_build-cc := $(OUTDIR_CHECK)/build-cc
$(TARGET_CHECK_build-cc):
	$(Q)$(RM) $@ && echo 'int main() { return 0; }' | \
	  $(CC) -o $@ -xc - > $@.log 2>&1 ;               \
	  if [ ! -x $@ ]; then echo 'exit 1' > $@; chmod +x $@; fi

# Create check program for if CXX produce binaries that can be executed on the build host.
TARGET_CHECK_build-cxx := $(OUTDIR_CHECK)/build-cxx
$(TARGET_CHECK_build-cxx):
	$(Q)$(RM) $@ && echo 'int main() { return 0; }' | \
	  $(CXX) -o $@ -xc - > $@.log 2>&1 ;              \
	  if [ ! -x $@ ]; then echo 'exit 1' > $@; chmod +x $@; fi

TARGET_CHECK_build-cc.mk := $(TARGET_CHECK_build-cc).mk
$(TARGET_CHECK_build-cc.mk): $(TARGET_CHECK_build-cc)
	$(Q)$(TARGET_CHECK_build-cc) > $@.log 2>&1 ; \
	  if [ $$? -eq 0 ]; then                        \
	      (echo 'BUILD_CC := $(CC)' > $@) ;         \
	  else                                          \
	      (echo 'BUILD_CC := /usr/bin/cc' > $@) ;   \
	  fi

TARGET_CHECK_build-cxx.mk := $(TARGET_CHECK_build-cxx).mk
$(TARGET_CHECK_build-cxx.mk): $(TARGET_CHECK_build-cxx)
	$(Q)$(TARGET_CHECK_build-cxx) > $@.log 2>&1 ; \
	  if [ $$? -eq 0 ]; then                         \
	      (echo 'BUILD_CXX := $(CXX)' > $@) ;        \
	  else                                           \
	      (echo 'BUILD_CXX := /usr/bin/c++' > $@) ;  \
	  fi

ifeq ($(BUILD_CC),)
include $(TARGET_CHECK_build-cc.mk)
endif

ifeq ($(BUILD_CXX),)
include $(TARGET_CHECK_build-cxx.mk)
endif
