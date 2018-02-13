######################################################################################
##
## Makefile for NSP Extensions
## (c) Copyright 2014 Blackrock Microsystems
##
## Major targets:
##
## all        - make the entire project
## plugin 	  - make plugin
## temps      - make intermediate files along with object files and executable
## clean      - clean up
##
######################################################################################

CC      := gcc
CXX     := g++
AR      := ar

#==========================================================================
# Operating System
#==========================================================================
OS := $(shell uname)
OBJSUBDIR := /nx
EXTLIB_POSTFIX :=
DLLEXT := .so
OSVER := $(shell uname -r)
GCCVER := $(shell $(CC) -dumpversion | cut -f1-2 -d.)
COMPATCFLAGS := -flax-vector-conversions


#==========================================================================
# UTILITIES
#==========================================================================
MKPARENT := mkdir -p `dirname $(1)`
ECHO     := @echo
CP       := @cp


#==========================================================================
# CONSTANTS
#==========================================================================
PLUGINLIB := libnsp$(PLAYER)$(EXTLIB_POSTFIX).a
# Name plugin has for operating on the nsp, DO NOT CHANGE.
PLUGIN := nspext_default$(DLLEXT)

# Compilation flags
CFLAGS := -Wall -Wstrict-prototypes                           \
	-m32 -mmmx -msse -msse2 -mfpmath=sse -malign-double       \
	-I.                                                       \
	-I$(PWD)/		                                          \
	-mpreferred-stack-boundary=4 -maccumulate-outgoing-args   \
	-funroll-loops -DNO_AFX

# Directories for binaries
# BinDir  := Install_Extension
BinDir  := ../lib
CFLAGS += -finline-functions -O2 -DNDEBUG -UDEBUG

# Directory for intermediate files and object files
ObjDir  := .obj$(OBJSUBDIR)
DepDir  := .dep

# Directory for included library files and common files
CommonDir := ../ExtensionCommon

# compatibility compilation flags
CFLAGS += $(COMPATCFLAGS)

ifdef REPORT
CFLAGS += -DREPORT
endif

# For creating intermediate files.
CFLAGS_TEMPS := -save-temps

# For linking.
LFLAGS  := -m32 -lgcc -lc -lm -lrt -ldl
CFLAGS += -fPIC
LFLAGS += -lpthread

# this is where all the source files are searched
VPATH := ./:./plugin

# plugin library sources. ADD ADDITIONAL PLUGIN SOURCE FILES HERE.
PLUGIN_SRC := shulz_extension.c

# Plugin API (header files, and examples)
PLUGIN_API_HEADERS := $(CommonDir)/nspChanTrigPlugin.h   \
                      $(CommonDir)/nspPlugin.h           \

# plugin library objects
PLUGIN_OBJS =  $(patsubst %.c, $(ObjDir)/%$(REPORT)$(DEBUG)$(PLAYER).o, $(filter %.c,$(PLUGIN_SRC)))

# all subdirectories
subdirs :=

ifeq ($(GCCVER),4.0)
all: LFLAGS += -lpci
endif

#### This tag must be the first build tag
all: prepare plugin $(BinDir)/$(PROGRAM)

@list='$(subdirs)'; for subdir in $$list; do test "$$subdir" = . || (cd $$subdir && $(MAKE)); done

.PHONY: plugin
### make plugin
plugin: $(BinDir)/$(PLUGIN)
# This will make the plugin
$(BinDir)/$(PLUGIN): $(CommonDir)/$(PLUGINLIB) $(PLUGIN_OBJS)
	@echo creating $@ ...
	$(CC) -o $(BinDir)/$(PLUGIN) $(PLUGIN_OBJS) $(LFLAGS) -L$(CommonDir) -lnsp$(PLAYER)$(EXTLIB_POSTFIX) -shared -Wl,-soname,$(PLUGIN)

# the object files
$(ObjDir)/%$(REPORT)$(DEBUG)$(PLAYER).o : %.c Makefile
	@echo creating $@ ...
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

# compile to create temporary or intermediate files
.PHONY: temps
temps:
	$(CC) -o $(BinDir)/$(PROGRAM) $(OBJS) $(LFLAGS)

# the "common" assembly files
$(ObjDir)/%.s : %.c
	$(CC) $(CFLAGS) $(CFLAGS_TEMPS) -c $<

# Clean out all files leaving installed and library files alone
.PHONY: clean
clean: $(subdirs)
	rm -f *.[ois] tags TAGS cscope.out Makefile~ core*
	rm -rf $(DepDir)
	rm -rf $(ObjDir)
	rm -f $(BinDir)/$(PLUGIN)

# Create all necessary directories.
.PHONY: prepare
prepare:
	@mkdir -p $(ObjDir)
	@mkdir -p $(BinDir)

# For generating dependencies
$(DepDir)/%.d: %.c
	@mkdir -p $(DepDir); \
	set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(DepDir)/%.d: %.c
	@mkdir -p $(DepDir); \
	set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


