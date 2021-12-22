#!/bin/bash
while [ 1 ]
do
    printf "Choose a frontend:\n"
    printf "  1) ncurses (default)\n"
    printf "  2) pdcurses (recommended for Windows users)\n"
    read response
    if [ $response == 2 ]
        then
        export GRAPHICAL_PORT=pdcurses
        break
    elif [ $response == 1 ]
        then
        export GRAPHICAL_PORT=ncurses
        break
    fi
done
printf "===============================\n"
printf "Building WEAL for $GRAPHICAL_PORT.\n"
printf "===============================\n\n"
make all
printf "\n===============================\n"
printf "Finished building WEAL for $GRAPHICAL_PORT.\n"
printf "===============================\n"
exit 0
