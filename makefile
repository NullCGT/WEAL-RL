# WEAL Makefile
# Kestrel Gregorich-Trevor
# 2021
#
#####################Options#######################

PORT:=${GRAPHICAL_PORT}

#################Definitions######################

# Source files
SRCDIR:=src
SRCS:=$(wildcard $(SRCDIR)/*.c)

# Headers
INCDIR:=include
DEPS:=$(wildcard $(INCIDR)/*.h)

# Objects
OBJDIR:=obj
OBJS:=$(patsubst %.c,%.o,$(patsubst $(SRCDIR)%,$(OBJDIR)%,$(SRCS)))

# Libs and Dependencies
LIBDIR:=lib

# Compiler
CC:=gcc

# Binary Name
BINARY:=weal

#################Compile Flags##################

# Compile Flags
ifeq ($(PORT),pdcurses)
    CFLAGS:= -Wall -lSDL2 -lm -I$(INCDIR)
    LDFLAGS:= -lpdcurses
    XTRALIBS:=lib/pdcurses.a
else
    CFLAGS:= -Wall -lm -lncursesw -D_XOPEN_SOURCE_EXTENDED -I$(INCDIR)
    XTRALIBS:=
endif

#####################Files#######################

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS) | objects
	$(CC) -c -o $@ $< $(CFLAGS)

#####################Recipes#####################

# binary (default)
binary: $(OBJS)
	$(CC) -o $(BINARY) $(OBJS) $(XTRALIBS) $(CFLAGS)

# clean
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINARY)

# all
all: clean binary

# obj
objects:
	mkdir -p $(OBJDIR)

# phony
.PHONY: clean all
