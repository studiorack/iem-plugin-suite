#!/bin/sh
cd ${0/*}
for d in */; do
    echo "Compiling $d for Linux..."
    if [ -d "$PWD/${d}Builds/LinuxMakefile" ]; then
        cd "$PWD/${d}Builds/LinuxMakefile"
        make CONFIG=Release AR=gcc-ar -j 6
        cd ../../../
        echo "done..."
    fi
    if [ ! -d "$PWD/${d}Builds/LinuxMakefile" ]; then
        echo "no makefile found, moving on..."
    fi
done
echo "all done!"
