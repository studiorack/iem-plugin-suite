#!/bin/sh
cd ${0%/*}
mkdir -p _compiledPlugins/linux/IEM
mkdir -p _compiledPlugins/linux/Standalone

# search for projucer (should be configured with global pathes before)
projucer=$(pwd)/JUCE/Projucer
if [ ! -x ${projucer} ]; then
    projucer=$(which Projucer)
    if [ ! -x ${projucer} ]; then
        echo no $projucer found, maybe copy it there
    fi
fi
echo using $projucer as Projucer

for d in */*.jucer; do
    d=${d%/*}

    if [ -x "${projucer}" ]; then ${projucer} --resave ${f}; fi  

    echo "Compiling $d for Linux..."
    if [ -d "${d}/Builds/LinuxMakefile" ]; then
        make CONFIG=Release AR=gcc-ar -j$(nproc) -k -C "${d}/Builds/LinuxMakefile"
        echo "done..."
        cp -R -H "${d}/Builds/LinuxMakefile/build/${d}.so" "_compiledPlugins/linux/IEM/"
        cp -R -H "${d}/Builds/LinuxMakefile/build/${d}" "_compiledPlugins/linux/Standalone/"
    else
        echo "no makefile found, moving on..."
    fi
done
find "_compiledPlugins/linux/IEM/" -type f -exec strip --strip-unneeded {} \;
find "_compiledPlugins/linux/Standalone/" -type f -exec strip --strip-unneeded {} \;
echo "all done!"
