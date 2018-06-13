#!/bin/sh

JUCEFLAVOUR=$1
OUTDIR=$2

rm -rf "${OUTDIR}"
mkdir -p "${OUTDIR}"

curl -o juce.zip https://d30pueezughrda.cloudfront.net/juce/juce-${JUCEFLAVOUR}.zip
unzip -q juce.zip -d "${OUTDIR}"
mv "${OUTDIR}"/JUCE/* "${OUTDIR}" || true
