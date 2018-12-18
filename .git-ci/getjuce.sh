#!/bin/sh

JUCEFLAVOUR=$1
OUTDIR=$2

rm -rf "${OUTDIR}"
mkdir -p "${OUTDIR}"

curl -o juce.zip https://d30pueezughrda.cloudfront.net/juce/juce-${JUCEFLAVOUR}.zip
unzip -q juce.zip -d "${OUTDIR}"
mv "${OUTDIR}"/JUCE/* "${OUTDIR}" || true

.git-ci/getvst.sh "${OUTDIR}/modules/juce_audio_processors/format_types/VST3_SDK/pluginterfaces/vst2.x"
