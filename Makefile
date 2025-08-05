CC=gcc
CFLAGS=-g -Wall -Werror -Wno-error=unused-variable -I $(INCLUDEDIR)
# SRCFILES=src/chip8.c src/main.c
INCLUDEDIR=include
SRCDIR=src
OBJDIR=obj
BINDIR=bin
SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
BIN=$(BINDIR)/ch8run

all: $(BIN)

release: CFLAGS=-Wall -O2 -DNDEBUG -I $(INCLUDEDIR)
release: clean
release: $(BIN)

$(BIN): $(OBJS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

bear:
	bear -- make

clean:
	$(RM) $(BIN) $(OBJS)
