//
//  KRWBFileSystemItem.h
//  KRWorldBuilder
//
//  Created by Kearwood Gilbert on 12-05-12.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface KRWBFileSystemItem : NSObject
{
    NSURL *fullPath;
    KRWBFileSystemItem *parent;
    NSMutableArray *children;
}

+ (KRWBFileSystemItem *)rootItem;
- (NSInteger)numberOfChildren;// Returns -1 for leaf nodes
- (KRWBFileSystemItem *)childAtIndex:(NSUInteger)n; // Invalid to call on leaf nodes
- (NSURL *)fullPath;
- (NSString *)relativePath;

@end

