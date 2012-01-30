#! /bin/sh

set -e

fontpath="unix/:7101"
#fontpath="inet/127.0.0.1:7101"
fontpattern="-*-*-*-*-*-*-*-*-*-*-*-*-*-*"
#fontpattern="-*-*-medium-r-normal-tt-*-*-*-*-*-*-iso8859-1"
#fontencoding="--encoding windows-1251,iso8859-2,koi8-r"

cd ../..
make && echo Build done.

sync
time src/xfstt --once $fontencoding > lst &
sleep 1

echo Trying to connect via $fontpath
xset +fp $fontpath
xlsfonts > fonts.lst
xfontsel -pattern $fontpattern
#rxvt +sb -fn "TTM160_Courier New" -geometry 32x10
xcoral -fn "TTM12_Courier New" -mfn "TTM16_Times New Roman" libfstt/raster_hints.cc &
xcoral -bg white -fg black -fn "TTP18_Courier New" src/xfstt.cc

echo Disconnecting from $fontpath
xset -fp $fontpath

echo Test complete.
