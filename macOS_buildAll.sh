#!/bin/sh
cd ${0%/*}
rm -rf */Builds
rm -rf */JuceLibraryCode
mkdir -p _compiledPlugins/macOS/IEM
mkdir -p _compiledPlugins/macOS/Standalone
for d in */*.jucer; do
  open -W -n ${PWD}/${d} --args --resave ${PWD}/${d}
  d=${d%/*}
  echo "Compiling $d for macOS..."
    if [ -d "${d}/Builds/MacOSX" ]; then
        pushd "${d}/Builds/MacOSX"
        xcodebuild -target "${d} - All" -configuration "Release" build
        echo "done..."
        popd
        cp -R -H "${d}/Builds/MacOSX/build/Release/${d}.vst" "_compiledPlugins/macOS/IEM/"
        cp -R -H "${d}/Builds/MacOSX/build/Release/${d}.app" "_compiledPlugins/macOS/Standalone/"
    else
        echo "no xcode project found, moving on..."
    fi
done
echo "all done!"
