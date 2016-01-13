#!/bin/bash

set -e

mkdir source/thirdParty/webrtc_android

pushd source/thirdParty/webrtc_android

git clone https://chromium.googlesource.com/external/webrtc.git src

pushd src

git reset --hard f01f8c9

git apply ../../webrtc_android_patch.diff

popd

popd

pushd samples/android/DemoApp/app/src/main

mkdir jniLibs/armeabi-v7a

pushd jniLibs/armeabi-v7a

curl -O ftp://developer.runamedia.com/webrtc-android-libs-20151223.tar.gz

tar -xzf webrtc-android-libs-20151223.tar.gz

rm webrtc-android-libs-20151223.tar.gz

popd

popd
