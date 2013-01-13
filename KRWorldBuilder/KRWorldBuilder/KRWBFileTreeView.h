//
//  KRWBFileTreeView.h
//  KRWorldBuilder
//
//  Created by Kearwood Gilbert on 2012-10-17.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface KRWBFileTreeView : NSView <NSOutlineViewDelegate, NSOutlineViewDataSource>

@property (nonatomic, retain) NSString *basePath;

@end
