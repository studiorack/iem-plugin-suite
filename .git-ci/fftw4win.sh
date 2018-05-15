#!/bin/sh

# script to download and "install" FFTW for windows, so we can use it for juce
# see http://www.fftw.org/install/windows.html

BITS="$1"
LIB="Lib.exe"

echo -n "downloading FFTW for ${BITS}"

if [ "x${BITS}" = "x" ]; then BITS="32 64"; fi

echo "... ${BITS}"

def2lib() {
 for f3 in libfftw3*.*; do
   f=$(echo libfftw${f3#libfftw3} | sed -e 's|-|-3.|')
   mv -v "${f3}" "${f}"
 done
 for def in libfftw*.def; do
  "${LIB}" -machine:${machine} -def:"${def}"
 done
}

for b in $BITS; do
  FFTW="fftw-3.3.5-dll${b}.zip"
  outdir="resources/fftw/x${b}"
  machine="x${b}"
  if [ "x${b}" = "x32" ]; then
    outdir="resources/fftw/win${b}"
    machine=x86
  fi
  mkdir -p "${outdir}"

  URL="ftp://ftp.fftw.org/pub/fftw/${FFTW}"

# install fftw3
  echo "extracting ${URL} to ${outdir}"

  curl -o "${FFTW}" "${URL}"
  unzip -q "${FFTW}" -d "${outdir}"
  (cd "${outdir}"; def2lib)
  rm "${FFTW}"

  echo "extracted files:"
  find "${outdir}"
done
