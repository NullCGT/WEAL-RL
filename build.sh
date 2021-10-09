#!/bin/bash
printf "Choose a frontend:\n"
printf "  1) ncurses (default)\n"
printf "  2) pdcurses (recommended for Windows users)\n"
read response

if [ $response == 2 ]
    then
	export WEAL_PORT=pdcurses
    else
	export WEAL_PORT=ncurses
fi
printf "Building WEAL for $WEAL_PORT.\n"
printf "===============================\n"
make all
exit 0
