//
//  KRObjViewAppDelegate.h
//  KRObjView
//
//  Created by Mac on 11-04-29.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class KRObjViewViewController;

@interface KRObjViewAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    KRObjViewViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet KRObjViewViewController *viewController;

@end
