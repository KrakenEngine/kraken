//
//  KrakenView.h
//  Kraken
//
//  Created by Kearwood Gilbert on 2015-08-07.
//  Copyright © 2015 Kearwood Software. All rights reserved.
//

#import <Cocoa/Cocoa.h>

class KRCamera;

@interface KrakenView : NSView
@property (nonatomic, assign) KRCamera *camera;
- (void)drawFrameWithDeltaTime: (float)deltaTime;
@end
