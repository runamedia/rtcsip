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

#import <UIKit/UIKit.h>

@interface VideoCallViewController : UIViewController


@property (weak, nonatomic) IBOutlet UIView *remoteVideoView;
@property (weak, nonatomic) IBOutlet UIView *localVideoView;
@property (weak, nonatomic) IBOutlet UIButton *buttonAudioCall;
@property (weak, nonatomic) IBOutlet UIButton *buttonVideoCall;
@property (weak, nonatomic) IBOutlet UIButton *buttonEndCall;

@property (nonatomic) BOOL isCallActive;
@property (nonatomic) BOOL isIncomingCall;
@property (copy, nonatomic) NSString* contact;

- (IBAction)actionMakeAudioCall:(id)sender;
- (IBAction)actionMakeVideoCall:(id)sender;
- (IBAction)actionEndCall:(id)sender;

- (void)turnOnSpeakers:(BOOL)turnOn;

- (void) onIncomingCall;
- (void) onCallEnded;
@end
