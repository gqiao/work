#!/bin/sh

VIDEO_SRC=/dev/video0

if [ ! -z ${1} ]; then
    VIDEO_SRC=${1}
fi

echo "please check: http://127.0.0.1:8090/?action=stream"

mjpg_streamer  -i "input_uvc.so -d ${VIDEO_SRC}" -o "output_http.so -p 8090"


