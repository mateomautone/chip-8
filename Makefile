CC = gcc
INCLUDEDIR = include
SRCDIR = src
OBJDIR = obj
BINDIR = bin
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
BIN = $(BINDIR)/ch8run

# Common includes
# Use C99 standard
CFLAGS_COMMON = -std=c99
# Enable all warnings
CFLAGS_COMMON += -Wall
# More warnings
CFLAGS_COMMON += -Wextra
# Strict printf/scanf checks
CFLAGS_COMMON += -Wformat=2
# Detect dangerous format strings
CFLAGS_COMMON += -Wformat-security
# Enforce strict ISO C compliance
CFLAGS_COMMON += -Wpedantic
# Warn on implicit type conversions
# CFLAGS_COMMON += -Wconversion
# Warn on signed/unsigned conversions
CFLAGS_COMMON += -Wsign-conversion
# Warn if float is promoted to double
CFLAGS_COMMON += -Wdouble-promotion
# Warn on undefined macros
CFLAGS_COMMON += -Wundef
# Warn if variable shadows another
CFLAGS_COMMON += -Wshadow
# Warn on pointer arithmetic on void*
CFLAGS_COMMON += -Wpointer-arith
# Warn if pointer cast changes alignment
CFLAGS_COMMON += -Wcast-align
# Warn on variable-length arrays
CFLAGS_COMMON += -Wvla
# Treat VLA use as an error
CFLAGS_COMMON += -Werror=vla
# Warn about unused parameters
CFLAGS_COMMON += -Wunused-parameter
# Warn about unused variables
CFLAGS_COMMON += -Wunused-variable
# Require prototypes in function declarations
CFLAGS_COMMON += -Wstrict-prototypes
# Warn if null pointer is dereferenced
CFLAGS_COMMON += -Wnull-dereference
# Warn on alloca() usage
CFLAGS_COMMON += -Walloca
# Warn if enum switch is missing values
CFLAGS_COMMON += -Wswitch-enum
# Highest signed overflow warnings
CFLAGS_COMMON += -Wstrict-overflow=5
# Make string literals const char[]
CFLAGS_COMMON += -Wwrite-strings
# Warn if cast discards const/volatile
CFLAGS_COMMON += -Wcast-qual
# Warn if conditions are duplicated
CFLAGS_COMMON += -Wduplicated-cond
# Warn if branches do the same thing
CFLAGS_COMMON += -Wduplicated-branches
# Warn on suspicious logical operations
CFLAGS_COMMON += -Wlogical-op
# Warn if jump bypasses initialization
CFLAGS_COMMON += -Wjump-misses-init
# Get SDL required flags
CFLAGS_COMMON += -I $(INCLUDEDIR) $(shell sdl2-config --cflags)
# Automatic dependency generation
CFLAGS_COMMON += -MMD -MP

# Get SDL required linker flags
LDFLAGS = $(shell sdl2-config --libs)

# Debug Flags
# Generate full debug info (includes macros)
DEBUG_FLAGS = -g3
# No optimization, easier to debug
DEBUG_FLAGS += -O0
# Enable static analysis with GCC analyzer
DEBUG_FLAGS += -fanalyzer
# Keep frame pointers for accurate backtraces
DEBUG_FLAGS += -fno-omit-frame-pointer

# Optional sanitizer flags
SAN ?= 1
# Sanitizers
SAN_FLAGS = -fsanitize=address,undefined,leak,float-cast-overflow,float-divide-by-zero
# Adds stack canaries (allows detecting some stack overflows)
SAN_FLAGS += -fstack-protector-strong
# Checks large stack allocations
SAN_FLAGS += -fstack-clash-protection
ifeq ($(SAN),1)
DEBUG_FLAGS += $(SAN_FLAGS)
endif

# Release build flags
# Optimize for speed
RELEASE_FLAGS = -O2
# Disable debug asserts
RELEASE_FLAGS += -DNDEBUG
# Strip symbols from binary
RELEASE_FLAGS += -s
# Remove compiler identification
RELEASE_FLAGS += -fno-ident
# Put each function in its own section
RELEASE_FLAGS += -ffunction-sections
# Put each data item in its own section
RELEASE_FLAGS += -fdata-sections
# Linker: remove unused sections
RELEASE_FLAGS += -Wl,-gc-sections
# Linker: print removed sections
RELEASE_FLAGS += -Wl,-print-gc-sections
# Stack smashing protection
RELEASE_FLAGS += -fstack-protector-strong
# Runtime checks for unsafe libc functions
RELEASE_FLAGS += -D_FORTIFY_SOURCE=3
# Make read-only relocations
RELEASE_FLAGS += -Wl,-z,relro
# Immediate symbol binding at startup
RELEASE_FLAGS += -Wl,-z,now
# Enable link-time optimization (cross-module)
RELEASE_FLAGS += -flto
# Hide symbols by default
RELEASE_FLAGS += -fvisibility=hidden

# CHIP8 Variants (May be innacurate)
VARIANT ?= TIMENDOUS
VARIANT_VIP = -DCHIP8_VF_RESET -DCHIP8_MEM_INCR -DCHIP8_CLIP -DCHIP8_WAIT_VBLANK -UCHIP8_SHIFT_VX_ONLY -DCHIP8_JUMP_USE_VX
VARIANT_MODERN = -UCHIP8_VF_RESET -UCHIP8_MEM_INCR -UCHIP8_CLIP -UCHIP8_WAIT_VBLANK -DCHIP8_SHIFT_VX_ONLY -UCHIP8_JUMP_USE_VX
VARIANT_TIMENDOUS = -DCHIP8_VF_RESET -DCHIP8_MEM_INCR -DCHIP8_CLIP -DCHIP8_WAIT_VBLANK -DCHIP8_SHIFT_VX_ONLY -UCHIP8_JUMP_USE_VX
ifeq ($(VARIANT),VIP)
VARIANT_FLAGS = $(VARIANT_VIP)
else ifeq ($(VARIANT),MODERN)
VARIANT_FLAGS = $(VARIANT_MODERN)
else ifeq ($(VARIANT),TIMENDOUS)
VARIANT_FLAGS = $(VARIANT_TIMENDOUS)
else
$(error Unknown variant: $(VARIANT))
endif


# Targets
all: debug

# Using DEBUG_FLAGS
debug: CFLAGS = $(CFLAGS_COMMON) $(DEBUG_FLAGS) $(VARIANT_FLAGS)
debug: $(BIN)

# Not using DEBUG_FLAGS because valgrind collides with sanitizers
valgrind: CFLAGS = $(CFLAGS_COMMON) -g3 -O0 $(VARIANT_FLAGS)
valgrind: $(BIN)

# Using RELEASE_FLAGS
release: CFLAGS= $(CFLAGS_COMMON) $(RELEASE_FLAGS) $(VARIANT_FLAGS)
release: clean
release: $(BIN)

# For using with gprof
profile: CFLAGS = $(CFLAGS_COMMON) -g -O0 -pg $(VARIANT_FLAGS)
profile: LDFLAGS += -pg
profile: $(BIN)

# Building the binary
$(BIN): $(OBJS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

# Building object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(BIN) $(OBJS) $(OBJS:.o=.d)

# Dependency stuff (-MMD -MP)
-include $(OBJS:.o=.d)
