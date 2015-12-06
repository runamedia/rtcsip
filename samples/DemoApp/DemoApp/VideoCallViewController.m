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

#import "VideoCallViewController.h"

#import <AVFoundation/AVFoundation.h>
#import "DemoManager.h"

@interface VideoCallViewController ()

- (void) prepareButtons;
@end

@implementation VideoCallViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    self.title = self.contact;
    [self prepareButtons];
    
    if (self.isIncomingCall)
        [[DemoManager sharedInstance] answerOnCallWithLocalVideoView:self.localVideoView remoteVideoView:self.remoteVideoView];
}


- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void) prepareButtons;
{
    self.buttonAudioCall.hidden = self.isCallActive;
    self.buttonVideoCall.hidden = self.isCallActive;
    self.buttonEndCall.hidden = !self.isCallActive;
}
- (IBAction)actionMakeAudioCall:(id)sender
{
    self.isIncomingCall = NO;

    [[DemoManager sharedInstance] makeCallWithParticipant:self.contact audio:YES video:NO localVideoView:nil remoteVideoView:nil];
//    self.remoteVideoView.hidden = YES;
//    self.localVideoView.hidden = YES;
    self.isCallActive = YES;
    [self prepareButtons];
}

- (IBAction)actionMakeVideoCall:(id)sender
{
    self.isIncomingCall = NO;
    
    [[DemoManager sharedInstance] makeCallWithParticipant:self.contact audio:YES video:YES localVideoView:self.localVideoView remoteVideoView:self.remoteVideoView];
//    self.remoteVideoView.hidden = NO;
//    self.localVideoView.hidden = NO;
    self.isCallActive = YES;
    [self prepareButtons];
}

- (IBAction)actionEndCall:(id)sender
{
    [[DemoManager sharedInstance] endCall];
//    self.remoteVideoView.hidden = YES;
//    self.localVideoView.hidden = YES;
    self.isCallActive = NO;
    [self prepareButtons];
}

- (void)turnOnSpeakers:(BOOL)turnOn
{
    NSError* error = nil;
    
    [[AVAudioSession sharedInstance] overrideOutputAudioPort:turnOn ? AVAudioSessionPortOverrideSpeaker : AVAudioSessionPortOverrideNone error:&error];
    if (error)
    {
        NSLog(@"Error:%@",error);
    }
}

- (void) onIncomingCall
{
    self.isIncomingCall = YES;
    self.isCallActive = YES;
    [self prepareButtons];
}

- (void) onCallEnded
{
    self.isIncomingCall = NO;
    self.isCallActive = NO;
    [self prepareButtons];
}
@end
