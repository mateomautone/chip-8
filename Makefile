CC = gcc
INCLUDEDIR = include
SRCDIR = src
OBJDIR = obj
BINDIR = bin
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
BIN = $(BINDIR)/ch8run

# - Werror
CFLAGS_COMMON = -std=c99 -Wall -Wextra -Wformat=2 -Wformat-security -Wpedantic -Wconversion -Wsign-conversion \
			 -Wdouble-promotion -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wvla -Werror=vla \
			 -Wno-unused-parameter -Wno-error=unused-variable -Wstrict-prototypes \
			 -I $(INCLUDEDIR) $(shell sdl2-config --cflags) \
			 -MMD -MP
LDFLAGS = $(shell sdl2-config --libs)

DEBUG_FLAGS = -g3 -O0 -fanalyzer -fno-omit-frame-pointer -fsanitize=address,undefined,leak
RELEASE_FLAGS = -O2 -DNDEBUG -s -fno-ident -ffunction-sections -fdata-sections -Wl,-gc-sections -Wl,-print-gc-sections

all: debug

debug: CFLAGS = $(CFLAGS_COMMON) $(DEBUG_FLAGS)
debug: $(BIN)

debug-valgrind: CFLAGS = $(CFLAGS_COMMON) -g3 -O0
debug-valgrind: $(BIN)

release: CFLAGS= $(CFLAGS_COMMON) $(RELEASE_FLAGS)
release: clean
release: $(BIN)

$(BIN): $(OBJS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(BIN) $(OBJS) $(OBJS:.o=.d)

-include $(OBJS:.o=.d)
