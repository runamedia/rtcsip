# rtcsip

Communication engine that wraps WebRTC and reSIProcate libraries to establish a Peer to Peer connection between users. Supported platforms are iOS and Android. Demo applications are supplied and they can be used as a starting point for further client application development.

Source code is released under free BSD license.

To clone the repository type the following command: git clone https://github.com/runamedia/rtcsip.git --recursive

At the moment the only supported develpment platform is OSX.

## Install instructions for Android

```
1. Clone the repository
2. cd rtcsip
3. ./rtcsip_sync_android.sh
4. ./rtcsip_build_android.sh
5. Open Android Studio project samples/android/DemoApp (required Android Studio version is 2.0 - use Canary channel to make an update)
```

##Install instructions for iOS

```
1. Clone the repository
2. cd rtcsip
3. ./rtcsip_sync_ios.sh
4. Open Xcode project samples/ios/DemoApp.xcodeproj and run target DemoApp
```
