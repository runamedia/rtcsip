# rtcsip

Communication engine that wraps WebRTC and reSIProcate libraries to establish a Peer to Peer connection between users. Supported platforms are iOS and Android. Demo applications are supplied and they can be used as a starting point for further client application development.

Source code is released under free BSD license.

To clone the repository type the following command: git clone https://github.com/runamedia/rtcsip.git --recursive

At the moment, the only supported develpment platform is OSX.

## Install instructions for Android
Prerequisites: Android Studio version 2.1 (use Canary channel to make an update), Android NDK r10
```
1. Clone the repository
2. cd rtcsip
3. ./rtcsip_sync_android.sh
4. export NDK_PATH=<path-to-ndk-bundle>
5. ./rtcsip_build_android.sh
6. Open Android Studio project samples/android/DemoApp
```

##Install instructions for iOS
Prerequisites: Xcode 7
```
1. Clone the repository
2. cd rtcsip
3. ./rtcsip_sync_ios.sh
4. Open Xcode project samples/ios/DemoApp.xcodeproj and run target DemoApp
```

##Configuration settings for FreeSWITCH server
config file - freeswitch/conf/vars.xml
```
<X-PRE-PROCESS cmd="set" data="domain=$${local_ip_v4}"/> - set domain if available (optional)
<X-PRE-PROCESS cmd="set" data="global_codec_prefs=OPUS,G722,PCMU,PCMA,GSM,VP8"/> - add VP8 to the end
<X-PRE-PROCESS cmd="set" data="outbound_codec_prefs=PCMU,PCMA,GSM,VP8"/> - add VP8 to the end
```
config file - freeswitch/conf/sip_profiles/internal.xml
```
<param name="sip-trace" value="yes"/> - set to "yes" to have SIP traces (optional)
<param name="inbound-bypass-media" value="true"/> - uncomment and set to "true"
<param name="ws-binding"  value=":5066"/> - uncomment this line
```

##Configuration settings for coturn TURN server (https://github.com/coturn/coturn)
config file - etc/turnserver.conf
```
listening-ip=<ip-address> - uncomment and set server IP address
fingerprint - uncomment this line
lt-cred-mech - uncomment this line
user=test:test - add this line for user authentication
realm=<domain> - uncomment and set server domain (can be IP address if no domain is assigned)
```

