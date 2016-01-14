#!/bin/bash

set -e

mkdir source/thirdParty/webrtc_ios

pushd source/thirdParty/webrtc_ios

git clone https://chromium.googlesource.com/external/webrtc.git src

pushd src

git reset --hard f01f8c9

mkdir -p out/Debug-iphoneos

pushd out/Debug-iphoneos

curl -O ftp://developer.runamedia.com/webrtc-ios-libs-20151223.tar.gz

tar xzf webrtc-ios-libs-20151223.tar.gz

rm webrtc-ios-libs-20151223.tar.gz

popd

popd

popd
