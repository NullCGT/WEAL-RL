# WEAL Makefile
# Kestrel Gregorich-Trevor
# 2022
#
#####################Options#######################

SHELL:=/bin/sh
PORT:=ncurses
#PORT:=pdcurses

#################Definitions######################

# Source files
SRCDIR:=src
SRCS:=$(wildcard $(SRCDIR)/*.c)

# Headers
INCDIR:=include

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
    CFLAGS:= -Wall -Wextra -pedantic -g -lSDL2 `xml2-config --cflags` -lm
	LIBS:= -isystem$(INCDIR) `xml2-config --libs`
    LDFLAGS:= -lpdcurses
    XTRALIBS:=lib/pdcurses.a
else
    CFLAGS:= -Wall -Wextra -pedantic -g -lm -lncursesw `xml2-config --cflags` -D_XOPEN_SOURCE_EXTENDED 
	LIBS:= -isystem$(INCDIR) `xml2-config --libs`
    XTRALIBS:=
endif

#####################Files#######################

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/*.h | objects
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

#####################Recipes#####################

# binary (default)
binary: $(OBJS)
	$(CC) -o $(BINARY) $(OBJS) $(XTRALIBS) $(CFLAGS) $(LIBS)

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
