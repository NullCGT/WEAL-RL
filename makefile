# WEAL Makefile
# Kestrel Gregorich-Trevor
# 2021
#
#####################Options#######################

PORT:=${WEAL_PORT}

#################Definitions######################

# Source files
SRCDIR:=src

# Headers
INCDIR:=include

# Objects
OBJDIR:=obj

# Libs and Dependencies
LIBDIR:=lib
DEPS:=$(INCDIR)

# Compiler
CC:=gcc

# Binary Name
BINARY:=weal

#################Compile Flags##################

# Compile Flags
ifeq ($(PORT),pdcurses)
    CFLAGS:= -Wall -lSDL2
    LDFLAGS:= -lpdcurses
    XTRALIBS:=lib/pdcurses.a
else
    CFLAGS:= -Wall -lncursesw -D_XOPEN_SOURCE_EXTENDED
    XTRALIBS:=
endif

#####################Files#######################

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS) -I $(INCDIR)

#####################Recipes#####################

# binary (default)
binary: obj/main.o obj/mapgen.o obj/register.o obj/render.o obj/windows.o
	$(CC) -o $(BINARY) obj/main.o obj/mapgen.o obj/register.o obj/render.o obj/windows.o $(XTRALIBS) $(CFLAGS)


# clean
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINARY)

# all
all: clean binary

# phony
.PHONY: clean all
