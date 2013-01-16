#!/bin/bash
# ______   _______  _______  _______  ______   ______   _      
#(  __  \ (  ____ )(  ____ \(  ____ )/ ___  \ / ___  \ ( (    /|
#| (  \  )| (    )|| (    \/| (    )|\/   \  \\/   \  \|  \  ( |
#| |   ) || (____)|| |      | (____)|   ___) /   ___) /|   \ | |
#| |   | ||     __)| | ____ |     __)  (___ (   (___ ( | (\ \) |
#| |   ) || (\ (   | | \_  )| (\ (         ) \      ) \| | \   |
#| (__/  )| ) \ \__| (___) || ) \ \__/\___/  //\___/  /| )  \  |
#(______/ |/   \__/(_______)|/   \__/\______/ \______/ |/    )_)
#
#

# A more reliable way to do perfect ScreenCasts in Linux
# ffmpeg -f alsa -i pulse -f x11grab -r 25 -s 1440x900 -i :0.0 -acodec pcm_s16le -vcodec huffyuv -sameq Screencast.avi



echo "Welcome to drgr33n's screen capture script"
echo
sleep 2

echo "Save file as ..:"
read filename

function record {
  ffmpeg -f alsa -ac 2 -i hw:0,0 -f x11grab -level 41 -crf 20 -bufsize 20000k -maxrate 25000k -g 250 \
  -s $(xwininfo -root | grep 'geometry' | awk '{print $2;}') -coder 1 -flags +loop -cmp +chroma -r 20 \
  -partitions +parti4x4+partp8x8+partb8x8 -subq 7 -me_range 16 -keyint_min 25 -sc_threshold 40 \
  -i_qfactor 0.71 -rc_eq 'blurCplx^(1-qComp)' -bf 16 -b_strategy 1 -bidir_refine 1 -refs 6 -deblockalpha 0 \
  -deblockbeta 0 -i :0.0 -acodec pcm_s16le -vcodec libx264 -vpre lossless_ultrafast \
  -threads 0 -y $filename.avi
}

record


