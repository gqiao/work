#!/bin/sh

cat > ./set_ctl.sh <<EOF
alsa_amixer cset name='MUX_UL00' 9
alsa_amixer cset name='MUX_UL01' 10
alsa_amixer cset name='MUX_UL00' 13
alsa_amixer cset name='MUX_UL01' 14
alsa_amixer cset name='MUX_UL02' 13
alsa_amixer cset name='MUX_UL03' 14
alsa_amixer cset name='MUX_UL04' 13
alsa_amixer cset name='MUX_UL05' 14
alsa_amixer cset name='MUX_UL06' 13
alsa_amixer cset name='MUX_UL07' 14
alsa_amixer cset name='MUX_UL08' 13
alsa_amixer cset name='MUX_UL09' 14
alsa_amixer cset name='MUX_UL10' 13
alsa_amixer cset name='MUX_UL11' 14
alsa_amixer cset name='DL1 Capture Playback Volume' 120
alsa_amixer cset name='AMIC_UL MM_EXT Switch' 1
alsa_amixer cset name='AMIC UL Volume' 120
alsa_amixer cset name='AUDUL Media Volume' 100
alsa_amixer cset name='Voice Capture Mixer Capture' 1
alsa_amixer cset name='AUDUL Voice UL Volume' 120
alsa_amixer cset name='SDT UL Volume' 110
alsa_amixer cset name='Sidetone Mixer Capture' 1
alsa_amixer cset name='MUX_UL00' 13
alsa_amixer cset name='MUX_UL01' 14
alsa_amixer cset name='DL1 Mixer Multimedia' 0
alsa_amixer cset name='DL1 Mixer Multimedia' 1
alsa_amixer cset name='Sidetone Mixer Playback' 1
alsa_amixer cset name='Sidetone Equalizer' 1
alsa_amixer cset name='DL1 MM_EXT Switch' 1
alsa_amixer cset name='DL1 Capture Playback Volume' 110
alsa_amixer cset name='SDT DL Volume' 120
alsa_amixer cset name='DL1 Media Playback Volume' 110

# Initialize audio output
alsa_aplay -M /misc/iectest/HP.wav

EOF


adb push ./set_ctl.sh /
adb shell "sh /set_ctl.sh"


