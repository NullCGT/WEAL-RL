#!/bin/sh
clear
PS3="Select your graphical port: "
ports=("ncurses" "pdcurses")
select GRAPHICAL_PORT in "${ports[@]}"
do
    printf "You have selected the $GRAPHICAL_PORT port.\n"
    break
done
printf "Creating makefile...\n"
cp -f compiling/makefile makefile
sed -i "/$GRAPHICAL_PORT/s/^#//" makefile
printf "Building WEAL for $GRAPHICAL_PORT...\n"
make all
printf "Finished building WEAL for $GRAPHICAL_PORT!\n"
printf "The executable should be located in the main directory.\n"
exit 0
