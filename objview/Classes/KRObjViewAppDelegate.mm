//
//  KRObjViewAppDelegate.m
//  KRObjView
//
//  Created by Mac on 11-04-29.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "KRObjViewAppDelegate.h"
#import "KRObjViewViewController.h"

@implementation KRObjViewAppDelegate

@synthesize window;
@synthesize viewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
    
    // Add the view controller's view to the window and display.
	[[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:UIStatusBarAnimationFade];
    [self.window addSubview:viewController.view];
    [self.window makeKeyAndVisible];

    return YES;
}

- (void)dealloc {
    [viewController release];
    [window release];
    [super dealloc];
}

@end
