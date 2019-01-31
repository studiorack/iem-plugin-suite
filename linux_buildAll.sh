#!/bin/sh

error() {
  echo "$@" 1>&2
}
scriptpath="${0%/*}"

# search for projucer (should be configured with global pathes before)
# try to find Projucer in one of:
# - ./JUCE/
# - ${scriptpath}/JUCE/
# - ~/JUCE/ (where Roli suggests to install it
# - your system path (e.g. installed by Debian)
which=$(which which)
_projucer=Projucer
projucer=${PROJUCER:=$(PATH=$(pwd)/JUCE:${HOME}/JUCE:${0%/*}/JUCE:${PATH} ${which} ${_projucer})}
if [ ! -x "${projucer}" ]; then
    if [ "x${projucer}" = "x" ]; then projucer=${_projucer}; fi
    error "no executable '$projucer' found!"
    error "  - either set the PROJUCER environment variable to the full path of the binary"
    error "  - or install ${_projucer} in $(pwd)/JUCE, ${scriptpath}/JUCE or ~/JUCE"
fi
echo "using '$projucer' as ${_projucer}"


cd ${0%/*}
mkdir -p _compiledPlugins/linux/IEM
mkdir -p _compiledPlugins/linux/Standalone

for f in */*.jucer; do
    d=${f%/*}

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
