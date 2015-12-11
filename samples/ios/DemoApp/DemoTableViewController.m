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

#import "DemoTableViewController.h"
#import "VideoCallViewController.h"
#import "DemoManager.h"
#import <rtcsip/rtcsip.h>

@interface DemoTableViewController ()

@property (nonatomic, strong) NSMutableArray* contacts;
@property (nonatomic) BOOL loginMode;

- (void) prepare;
- (void) loginUser;
@end

@implementation DemoTableViewController

- (void)prepare
{
    self.loginMode = YES;
    self.contacts = [NSMutableArray arrayWithArray: [[DemoManager sharedInstance].contacts sortedArrayUsingSelector:@selector(localizedCaseInsensitiveCompare:)]];
    
    UIStoryboard* storyboardMain = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
    self.videoViewController = [storyboardMain instantiateViewControllerWithIdentifier:@"VideoViewController"];
}

- (void) loginUser
{
    NSIndexPath* selectedContact = [self.tableView indexPathForSelectedRow];
    
    if (self.contacts.count > selectedContact.row)
    {
        NSString* contact = [self.contacts objectAtIndex:selectedContact.row];
        [[DemoManager sharedInstance] loginWithUsername:contact password:DEMO_DEFAULT_PASSWORD];
        
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [DemoManager sharedInstance].demoTableViewController = self;
    
    [self prepare];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {

    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {

    return self.contacts.count;
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"cellContact" forIndexPath:indexPath];
    
    if (self.contacts.count > indexPath.row)
    {
        
        NSString* contact = [self.contacts objectAtIndex:indexPath.row];
        cell.textLabel.text = contact;
    }
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSIndexPath* selectedContact = [self.tableView indexPathForSelectedRow];
    
    if (self.contacts.count > indexPath.row)
    {
        NSString* contact = [self.contacts objectAtIndex:selectedContact.row];
        
        if (self.loginMode)
        {
            [[DemoManager sharedInstance] loginWithUsername:contact password:DEMO_DEFAULT_PASSWORD];
        }
        else
        {
            [self showConversationForContact:contact];
        }
    }
    
}


- (void) onUserRegistered:(NSString*) contact
{
    self.loginMode = NO;
    self.title = [NSString stringWithFormat:@"%@'s Contacts",contact];
    [self.contacts removeObject:contact];
    [self.tableView reloadData];
}


- (void) showConversationForContact:(NSString*) contact
{
    self.videoViewController.contact = contact;
    
    if (self.navigationController.visibleViewController != self.videoViewController)
    {
        [self.navigationController pushViewController:self.videoViewController animated:NO];
    }
    else if (self.videoViewController.isIncomingCall)
    {
        [[DemoManager sharedInstance] answerOnCallWithLocalVideoView:self.videoViewController.localVideoView remoteVideoView:self.videoViewController.remoteVideoView];
    }
}

- (void) onIncomingCall:(NSString*) contact
{
    [self.videoViewController onIncomingCall];
    
    [self showConversationForContact:contact];
}

- (void) onEndedCall
{
    [self.videoViewController onCallEnded];
}
@end
