#！/bin/sh

# $1 == width
# $2 == height
# $3 == bitRate
# $5 == framerate

gst-launch v4l2src always-copy=FALSE ! 'video/x-raw-yuv,format=(fourcc)YUY2', width=$1, height=$2, framerate=$5/1 ! videorate ! ffmpegcolorspace ! dmaiperf  print-arm-load=true engine-name=codecServer !   TIVidenc1 codecName=h264enc engineName=codecServer frameRate=$5 resolution=$1x$2 genTimeStamps=false   encodingPreset=3 iColorSpace=UYVY bitRate=$3 ! filesink location=/dev/null



