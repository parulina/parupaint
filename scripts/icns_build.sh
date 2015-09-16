#!/bin/sh
# this script helps converting a png to icns.

[ ! $1 ] && exit

#resulting filename of .icns
NAME=parupaint
ICON=$(echo $1)

mkdir ${NAME}.iconset
sips -z 16 16     ${ICON} --out ${NAME}.iconset/icon_16x16.png
sips -z 32 32     ${ICON} --out ${NAME}.iconset/icon_16x16@2x.png
sips -z 32 32     ${ICON} --out ${NAME}.iconset/icon_32x32.png
sips -z 64 64     ${ICON} --out ${NAME}.iconset/icon_32x32@2x.png
sips -z 128 128   ${ICON} --out ${NAME}.iconset/icon_128x128.png
sips -z 256 256   ${ICON} --out ${NAME}.iconset/icon_128x128@2x.png
sips -z 256 256   ${ICON} --out ${NAME}.iconset/icon_256x256.png
sips -z 512 512   ${ICON} --out ${NAME}.iconset/icon_256x256@2x.png
sips -z 512 512   ${ICON} --out ${NAME}.iconset/icon_512x512.png
cp ${ICON} ${NAME}.iconset/icon_512x512@2x.png
iconutil -c icns ${NAME}.iconset
rm -R ${NAME}.iconset
