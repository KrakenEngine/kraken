//
//  KRWBDocument.h
//  KRWorldBuilder
//
//  Created by Kearwood Gilbert on 12-02-10.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#import <Cocoa/Cocoa.h>

class KRWorld;

@interface KRWBDocument : NSDocument <NSOutlineViewDelegate, NSOutlineViewDataSource>

@property (nonatomic, readonly) KRWorld *world;
@property (nonatomic, retain) IBOutlet NSOutlineView *outlineView;

@end
