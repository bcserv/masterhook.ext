# (C)2004-2010 SourceMod Development Team
# Makefile written by David "BAILOPAN" Anderson

###########################################
### EDIT THESE PATHS FOR YOUR OWN SETUP ###
###########################################

REPOSITORIES		= /repositories
SMSDK			= $(REPOSITORIES)/sourcemod-central
HL2SDK_ORIG		= $(REPOSITORIES)/hl2sdk
HL2SDK_OB		= $(REPOSITORIES)/hl2sdk-ob
HL2SDK_OB_VALVE	= $(REPOSITORIES)/hl2sdk-ob-valve
HL2SDK_L4D		= $(REPOSITORIES)/hl2sdk-l4d2
HL2SDK_L4D2		= $(REPOSITORIES)/hl2sdk-l4d2
MMSOURCE18		= $(REPOSITORIES)/mmsource-central

#####################################
### EDIT BELOW FOR OTHER PROJECTS ###
#####################################

PROJECT = masterhook

#Uncomment for Metamod: Source enabled extension
USEMETA = true

OBJECTS = sdk/smsdk_ext.cpp extension.cpp natives.cpp symboltable.cpp rtti.cpp
OBJECTS += DynDetours/arg_class.cpp DynDetours/asmbridge_class.cpp DynDetours/asm.cpp DynDetours/conv_cdecl.cpp DynDetours/conv_main.cpp DynDetours/conv_stdcall.cpp DynDetours/conv_thiscall.cpp
OBJECTS += DynDetours/cpp_manager.cpp DynDetours/sourcemod_manager.cpp DynDetours/dd_utils.cpp DynDetours/detour_class.cpp DynDetours/detourman_class.cpp DynDetours/func_class.cpp DynDetours/func_stack.cpp
OBJECTS += DynDetours/hook_handler.cpp DynDetours/memutils.cpp DynDetours/trampoline_class.cpp
OBJECTS += ASMJit/AsmJit/AssemblerX86X64.cpp ASMJit/AsmJit/CompilerX86X64.cpp ASMJit/AsmJit/CpuInfo.cpp ASMJit/AsmJit/Defs.cpp ASMJit/AsmJit/Lock.cpp ASMJit/AsmJit/Logger.cpp
OBJECTS += ASMJit/AsmJit/LoggerX86X64.cpp ASMJit/AsmJit/LoggerX86X64Dump.cpp ASMJit/AsmJit/MemoryManager.cpp ASMJit/AsmJit/SerializerX86X64.cpp ASMJit/AsmJit/Util.cpp ASMJit/AsmJit/VirtualMemory.cpp

##############################################
### CONFIGURE ANY OTHER FLAGS/OPTIONS HERE ###
##############################################

C_OPT_FLAGS = -DNDEBUG -O3 -funroll-loops -pipe -fno-strict-aliasing -fno-stack-protector
C_DEBUG_FLAGS = -D_DEBUG -DDEBUG -g -ggdb3
C_GCC4_FLAGS = -fvisibility=hidden
CPP_GCC4_FLAGS = -fvisibility-inlines-hidden
CPP = gcc-4.1

##########################
### SDK CONFIGURATIONS ###
##########################

override ENGSET = false

# Check for valid list of engines
ifneq (,$(filter original orangebox orangeboxvalve left4dead left4dead2,$(ENGINE)))
	override ENGSET = true
endif

ifeq "$(ENGINE)" "original"
	HL2SDK = $(HL2SDK_ORIG)
	CFLAGS += -DSOURCE_ENGINE=1
endif
ifeq "$(ENGINE)" "orangebox"
	HL2SDK = $(HL2SDK_OB)
	CFLAGS += -DSOURCE_ENGINE=3
endif
ifeq "$(ENGINE)" "orangeboxvalve"
	HL2SDK = $(HL2SDK_OB_VALVE)
	CFLAGS += -DSOURCE_ENGINE=4
endif
ifeq "$(ENGINE)" "left4dead"
	HL2SDK = $(HL2SDK_L4D)
	CFLAGS += -DSOURCE_ENGINE=5
endif
ifeq "$(ENGINE)" "left4dead2"
	HL2SDK = $(HL2SDK_L4D2)
	CFLAGS += -DSOURCE_ENGINE=6
endif

HL2PUB = $(HL2SDK)/public

ifeq "$(ENGINE)" "original"
	INCLUDE += -I$(HL2SDK)/public/dlls
	METAMOD = $(MMSOURCE18)/core-legacy
else
	INCLUDE += -I$(HL2SDK)/public/game/server
	METAMOD = $(MMSOURCE18)/core
endif

OS := $(shell uname -s)

ifeq "$(OS)" "Darwin"
	LIB_EXT = dylib
	HL2LIB = $(HL2SDK)/lib/mac
else
	LIB_EXT = so
	ifeq "$(ENGINE)" "original"
		HL2LIB = $(HL2SDK)/linux_sdk
	else
		HL2LIB = $(HL2SDK)/lib/linux
	endif
endif

# if ENGINE is orig, OB, or L4D
ifneq (,$(filter original orangebox left4dead,$(ENGINE)))
	LIB_SUFFIX = _i486.$(LIB_EXT)
else
	LIB_PREFIX = lib
	LIB_SUFFIX = .$(LIB_EXT)
endif

INCLUDE += -I. -I.. -Isdk -I$(SMSDK)/public -I$(SMSDK)/public/extensions -I$(SMSDK)/public/sourcepawn -I$(SMSDK)/core/logic
INCLUDE += -I./DynDetours/AsmJit -I./ASMJit -I./DynDetours

ifeq "$(USEMETA)" "true"
	LINK_HL2 = $(HL2LIB)/tier1_i486.a $(LIB_PREFIX)vstdlib$(LIB_SUFFIX) $(LIB_PREFIX)tier0$(LIB_SUFFIX)

	LINK += $(LINK_HL2)

	INCLUDE += -I$(HL2SDK)/game/server -I$(HL2SDK)/game/shared \
		-I$(HL2PUB) -I$(HL2PUB)/engine -I$(HL2PUB)/tier0 -I$(HL2PUB)/tier1 -I$(METAMOD) \
		-I$(METAMOD)/sourcehook 
	CFLAGS += -DSE_EPISODEONE=1 -DSE_DARKMESSIAH=2 -DSE_ORANGEBOX=3 -DSE_ORANGEBOXVALVE=4 \
		-DSE_LEFT4DEAD=5 -DSE_LEFT4DEAD2=6 -DSE_ALIENSWARM=7
endif

LINK += -m32 -lm -ldl -lelf -Wl,--hash-style=sysv

CFLAGS += -D_LINUX -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp \
	-D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp -Wall \
	-Wno-invalid-offsetof -Wno-switch -Wno-unused -mfpmath=sse -msse -DSOURCEMOD_BUILD -DHAVE_STDINT_H -m32
CPPFLAGS += -Wno-non-virtual-dtor -fno-exceptions

################################################
### DO NOT EDIT BELOW HERE FOR MOST PROJECTS ###
################################################

BINARY = $(PROJECT).ext.$(LIB_EXT)

ifeq "$(DEBUG)" "true"
	BIN_DIR = Debug
	CFLAGS += $(C_DEBUG_FLAGS)
else
	BIN_DIR = Release
	CFLAGS += $(C_OPT_FLAGS)
endif

ifeq "$(USEMETA)" "true"
	BIN_DIR := $(BIN_DIR).$(ENGINE)
endif

ifeq "$(OS)" "Darwin"
	LIB_EXT = dylib
	CFLAGS += -isysroot /Developer/SDKs/MacOSX10.5.sdk
	LINK += -dynamiclib -lstdc++ -mmacosx-version-min=10.5
else
	LIB_EXT = so
	CFLAGS += -D_LINUX
	LINK += -shared
endif

GCC_VERSION := $(shell $(CPP) -dumpversion >&1 | cut -b1)
ifeq "$(GCC_VERSION)" "4"
	CFLAGS += $(C_GCC4_FLAGS)
	CPPFLAGS += $(CPP_GCC4_FLAGS)
endif

OBJ_BIN := $(OBJECTS:%.cpp=$(BIN_DIR)/%.o)

$(BIN_DIR)/%.o: %.cpp
	$(CPP) $(INCLUDE) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

all: check
	mkdir -p $(BIN_DIR)/sdk
	mkdir -p $(BIN_DIR)/asm
	mkdir -p $(BIN_DIR)/DynDetours
	mkdir -p $(BIN_DIR)/ASMJit/AsmJit
	if [ "$(USEMETA)" = "true" ]; then \
		ln -sf $(HL2LIB)/$(LIB_PREFIX)vstdlib$(LIB_SUFFIX); \
		ln -sf $(HL2LIB)/$(LIB_PREFIX)tier0$(LIB_SUFFIX); \
	fi
	$(MAKE) -f Makefile extension

	cp --force $(BIN_DIR)/$(BINARY) /server/game/hl2mp/test/orangebox/hl2mp/addons/sourcemod/extensions/

check:
	if [ "$(USEMETA)" = "true" ] && [ "$(ENGSET)" = "false" ]; then \
		echo "You must supply one of the following values for ENGINE:"; \
		echo "left4dead2, left4dead, orangeboxvalve, orangebox, or original"; \
		exit 1; \
	fi

extension: check $(OBJ_BIN)
	$(CPP) $(INCLUDE) $(OBJ_BIN) $(LINK) -o $(BIN_DIR)/$(BINARY)

debug:
	$(MAKE) -f Makefile all DEBUG=true

default: all

clean: check
	rm -rf $(BIN_DIR)/*.o
	rm -rf $(BIN_DIR)/sdk/*.o
	rm -rf $(BIN_DIR)/asm/*.o
	rm -rf $(BIN_DIR)/DynDetours/*.o
	rm -rf $(BIN_DIR)/$(BINARY)

