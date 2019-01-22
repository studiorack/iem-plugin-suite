#!/bin/sh
cd ${0%/*}
mkdir -p _compiledPlugins/linux/IEM
mkdir -p _compiledPlugins/linux/Standalone
for d in */*.jucer; do
    d=${d%/*}
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
