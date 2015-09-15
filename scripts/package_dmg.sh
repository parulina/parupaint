#!/bin/sh

function die {
	echo $1
	exit 1
}

TMP="./_dmg_tmp"
[ -f "../src/parupaintVersion.h" ] || die "Run from scripts directory."

VERSION=$(cat ../src/parupaintVersion.h | sed -n 's/#define PARUPAINT_.*_VERSION \([0-9]\)/\1/p' | tr -d '\n')
APPNAME="parupaint $VERSION"
DMG="parupaint.dmg"

rm -rf $TMP
rm -f $DMG
mkdir $TMP

cp -r ../bin/parupaint.app "$TMP"
SIZE=$(du -sh "$TMP" | sed 's/\([0-9]*\)M\(.*\)/\1/')
SIZE=$(echo "$SIZE + 1.0" | bc | awk '{print int($1+0.5)}')

hdiutil create -srcfolder "$TMP" -volname "$APPNAME" -fs HFS+ \
	-fsargs "-c c=64,a=16,e=16" -format UDRW -size ${SIZE}M "temp-$DMG"

DEVICE=$(hdiutil attach -readwrite -noverify "temp-${DMG}" | \
	         egrep '^/dev/' | sed 1q | awk '{print $1}')
echo "Mounted at $DEVICE"
pushd "/Volumes/$APPNAME"
ln -s /Applications
popd

sync

hdiutil detach "$DEVICE"
hdiutil convert "temp-$DMG" -format UDZO -imagekey zlib-level=9 -o "${DMG}"
rm -f "temp-$DMG"
rm -rf $TMP

