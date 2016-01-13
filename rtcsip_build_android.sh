#!/bin/bash

set -e

pushd samples/android/DemoApp/app/src/main

NDK_PATH/ndk-build

cp libs/armeabi-v7a/librtcsip_jni.so jniLibs/armeabi-v7a

popd
