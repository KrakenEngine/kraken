//
//  KrakenView.h
//  Kraken
//
//  Created by Kearwood Gilbert on 2017-04-27.
//  Copyright © 2017 Kearwood Software. All rights reserved.
//
class KRContext;
class KRScene;

@protocol KrakenViewDelegate <NSObject>
@optional
- (void)preRender:(KRContext *)context withDeltaTime: (CFTimeInterval)deltaTime;
- (void)postRender:(KRContext *)context withDeltaTime: (CFTimeInterval)deltaTime;
@end

@interface KrakenView : UIView

@property (nonatomic, weak) IBOutlet id <KrakenViewDelegate> delegate;
@property (nonatomic, assign) KRScene *scene;

- (void)startAnimation;
- (void)stopAnimation;

@end
