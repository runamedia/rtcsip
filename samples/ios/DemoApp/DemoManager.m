/*
 
 Copyright (c) 2015, Runa Media LLC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
 
 */

#import "DemoManager.h"
#import <rtcsip/rtcsip.h>
#import "DemoTableViewController.h"
#import "VideoCallViewController.h"

//#define SET_VIDEO_VIEWS_PROGRAMMATICALLY 1

@interface DemoManager()

@property (nonatomic, strong) NSDictionary* contactsDict;


#ifdef SET_VIDEO_VIEWS_PROGRAMMATICALLY
    @property (nonatomic, strong) UIView* lView;
    @property (nonatomic, strong) UIView* rView;
#endif

@property (nonatomic, copy) NSString* activeCallID;
- (instancetype)initSingleton;

@property (nonatomic, strong) SipController* sipController;

@end



@implementation DemoManager

+ (instancetype) sharedInstance
{
    __strong static id instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] initSingleton];
    });
    return instance;
}

- (instancetype)initSingleton
{
    self = [super init];
    if (self)
    {
        self.contactsDict =  @{ DEMO_USER_1     : [NSString stringWithFormat:@"%@@%@:%d",DEMO_USER_1,DEMO_SIP_SERVER, DEMO_SIP_SERVER_PORT],
                                DEMO_USER_2     : [NSString stringWithFormat:@"%@@%@:%d",DEMO_USER_2,DEMO_SIP_SERVER, DEMO_SIP_SERVER_PORT],
                                DEMO_USER_3     : [NSString stringWithFormat:@"%@@%@:%d",DEMO_USER_3,DEMO_SIP_SERVER, DEMO_SIP_SERVER_PORT],
                                DEMO_USER_4     : [NSString stringWithFormat:@"%@@%@:%d",DEMO_USER_4,DEMO_SIP_SERVER, DEMO_SIP_SERVER_PORT],
                                };
#ifdef SET_VIDEO_VIEWS_PROGRAMMATICALLY
        self.lView = [RTCSIPUtility createVideoViewWithFrame:CGRectMake(0.0, 0.0, 160.0, 200.0)];
        self.rView = [RTCSIPUtility createVideoViewWithFrame:CGRectMake(160.0, 0.0, 320.0, 200.0)];
#endif
        self.sipController = [[SipController alloc] init];
        
        [self.sipController registerRegistrationHandler:^(RegistrationEvent event, NSString *user) {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (event == Registered) {
                    [self.demoTableViewController onUserRegistered:user];
                } else if (event == NotRegistered) {
                }
            });
        }];
        [self.sipController registerCallHandler:^(CallEvent event, NSString *user) {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (event == IncomingCall) {
                    [self.demoTableViewController onIncomingCall:user];
                } else if (event == TerminateCall) {
                    [self.demoTableViewController onEndedCall];
                } else if (event == CallAccepted) {
                    [self.demoTableViewController.videoViewController turnOnSpeakers:YES];
                }
            });
        }];
        [self.sipController registerLogHandler:^(NSString *logMessage) {
            dispatch_async(dispatch_get_main_queue(), ^{
            });
        }];
        [self.sipController registerErrorHandler:^(ErrorType type, NSString *message) {
            if (type == WebRtcError) {
            } else if (type == SipStackError) {
            }
        }];
    }
    return self;
}

- (NSArray *)getContacts
{
    return self.contactsDict.allKeys;
}
- (void) loginWithUsername:(NSString*) username password:(NSString*) password
{
    [self.sipController registerUserWithUsername:username andPassword:password];
}

- (void) makeCallWithParticipant:(NSString*) participant audio:(BOOL) audio video:(BOOL) video localVideoView:(UIView*) localVideoView remoteVideoView:(UIView*) remoteVideoView
{
    NSError* error = nil;
    UIView* lView = nil;
    UIView* rView = nil;
    
#ifdef SET_VIDEO_VIEWS_PROGRAMMATICALLY
    if (!self.lView.superview)
        [self.demoTableViewController.videoViewController.view addSubview:self.lView];
    if (!self.rView.superview)
        [self.demoTableViewController.videoViewController.view addSubview:self.rView];
    
    lView = self.lView;
    rView = self.rView;
#else
    lView = localVideoView;
    rView = remoteVideoView;
#endif
    [self.sipController setLocalView:lView];
    [self.sipController setRemoteView:rView];
    [self.sipController setHasAudio:audio];
    [self.sipController setHasVideo:video];
    [self.sipController makeCallTo:self.contactsDict[participant]];
}

- (void) answerOnCallWithLocalVideoView:(UIView*) localVideoView remoteVideoView:(UIView*) remoteVideoView
{
    UIView* lView = nil;
    UIView* rView = nil;
    
#ifdef SET_VIDEO_VIEWS_PROGRAMMATICALLY
    if (!self.lView.superview)
        [self.demoTableViewController.videoViewController.view addSubview:self.lView];
    if (!self.rView.superview)
        [self.demoTableViewController.videoViewController.view addSubview:self.rView];
    
    lView = self.lView;
    rView = self.rView;
#else
    lView = localVideoView;
    rView = remoteVideoView;
#endif
    
    [self.sipController setLocalView:lView];
    [self.sipController setRemoteView:rView];
    [self.sipController answer];
}

- (void)endCall
{
    [self.sipController endCallAndDestroyLocalStream:YES];
}

#pragma mark - SIPControllerDelegate
- (void) onUserRegistered:(NSString*) username
{
    dispatch_sync(dispatch_get_main_queue(), ^
    {
        [self.demoTableViewController onUserRegistered:username];
    });
}

- (void)onUserUnRegistered:(NSString *)username
{
    
}
@end
