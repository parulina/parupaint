#!/bin/sh


DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

function die {
	echo $1
	exit 1
}

VER="${DIR}/../src/parupaintVersion.h"
APP="${DIR}/../bin/parupaint.app"
DMG="./parupaint"
DMGTEMP="${DMG}-temp"
TMP="/tmp/_dmg_tmp"

[ -f "$VER" ] || die "This script should be in the scripts directory."
VERSION=$(cat "$VER" | sed -n 's/#define PARUPAINT_.*_VERSION \([0-9]\)/\1/p' | tr -d '\n')
APPNAME="parupaint $VERSION"
MNTNAME="/Volumes/$APPNAME"

rm -rf "$TMP"
rm -f "$DMG"
mkdir -p "$TMP"

echo "Building $(basename ${DMG}.dmg)"

cp -r "$APP" "$TMP"
# -s = 0 depth, -h = human readable, -m = megabytes
SIZE=$(du -sh -m "$TMP" | sed 's/\([0-9]*\).*/\1/')
SIZE=$(echo "$SIZE + 1.0" | bc | awk '{print int($1+0.5)}')

echo " Creating temporary dmg..."
hdiutil create -srcfolder "$TMP" -volname "$APPNAME" -fs HFS+ \
	-fsargs "-c c=64,a=16,e=16" -format UDRW -size ${SIZE}M "${DMGTEMP}.dmg" -quiet

echo " Created ${DMGTEMP}.dmg (${SIZE}M)"
DEVICE=$(hdiutil attach -readwrite -noverify "${DMGTEMP}.dmg" | \
	         egrep '^/dev/' | sed 1q | awk '{print $1}')

[ !$DEVICE ] || die "Something went wrong mounting the DMG."
echo " Mounted ${DMGTEMP}.dmg"


pushd "$MNTNAME" > /dev/null

ln -s /Applications
touch "./parupaint.app"

DMGICON="${MNTNAME}/.VolumeIcon.icns"

cp "$DIR/../resources/parupaint.icns" "${DMGICON}"
SetFile -c icnC "${DMGICON}"
SetFile -a C "${MNTNAME}"

echo '
   tell application "Finder"
     tell disk "'${APPNAME}'"
           open
           set current view of container window to icon view
           set toolbar visible of container window to false
           set statusbar visible of container window to false
           set the bounds of container window to {400, 100, 1200, 700}

	   set iconsize to 100
	   set position of item "parupaint.app" of container window to {200, 300 - (iconsize/100)}
	   set position of item "Applications" of container window to {600, 300 - (iconsize/100)}

           set theViewOptions to the icon view options of container window
           set arrangement of theViewOptions to not arranged
           set icon size of theViewOptions to iconsize
	   set background color of theViewOptions to {61680, 53970, 38550}
           update without registering applications
	   delay 1
	   close
     end tell
   end tell
' | osascript

chmod -Rf go-w "$MNTNAME" &> /dev/null
bless --folder "$MNTNAME" --openfolder "$MNTNAME"

popd > /dev/null
sync

echo " Unmounting..."
hdiutil detach "$DEVICE" -quiet
echo " Compressing to ${DMG}.dmg..."
hdiutil convert "${DMGTEMP}.dmg" -format UDZO -imagekey zlib-level=9 -o "${DMG}.dmg" -quiet
rm -f "${DMGTEMP}.dmg"
rm -rf "$TMP"

NEWSIZE=$(du -hs "${DMG}.dmg" | cut -d $'\t' -f1)
echo "Done! ${DMG}.dmg (${NEWSIZE})"
