#!/bin/sh

# script to download and "install" FFTW for windows, so we can use it for juce
# see http://www.fftw.org/install/windows.html

bits="$1"
libexe="Lib.exe"
outdirbase="resources/fftw3"
fftwversion=3.3.5

echo -n "downloading fftw for ${bits}"

if [ "x${bits}" = "x" ]; then bits="32 64"; fi

echo "... ${bits}"

def2lib() {
 for f3 in libfftw3*.*; do
   f=$(echo libfftw${f3#libfftw3} | sed -e 's|-|-3.|')
   mv -v "${f3}" "${f}"
 done
 for def in libfftw*.def; do
  "${libexe}" -machine:${machine} -def:"${def}"
 done
}

for b in ${bits}; do
  fftw="fftw-${fftwversion}-dll${b}.zip"
  outdir="${outdirbase}/x${b}"
  machine="x${b}"
  if [ "x${b}" = "x32" ]; then
    outdir="${outdirbase}/win${b}"
    machine=x86
  fi
  mkdir -p "${outdir}"

  url="ftp://ftp.fftw.org/pub/fftw/${fftw}"

# install fftw3
  echo "extracting ${url} to ${outdir}"

  curl -o "${fftw}" "${url}"
  unzip -q "${fftw}" -d "${outdir}"
  (cd "${outdir}"; def2lib)
  rm "${fftw}"

  echo "extracted files:"
  find "${outdir}"
done
