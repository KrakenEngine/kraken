//
//  KRWBFileTreeView.m
//  KRWorldBuilder
//
//  Created by Kearwood Gilbert on 2012-10-17.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#import "KRWBFileTreeView.h"
#import "KRWBFileSystemItem.h"

@interface KRWBFileTreeView()
@property (nonatomic, retain) IBOutlet NSOutlineView *outlineView;
@end

@implementation KRWBFileTreeView

-(void)commonInit
{
    
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self) {
        [self commonInit];
    }
    
    return self;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self commonInit];
    }
    
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
    // Drawing code here.
}


- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item {
    if(item == nil) {
        return [[KRWBFileSystemItem rootItem] numberOfChildren];
    } else {
        return [item numberOfChildren];
    }
    //return (item == nil) ? 1 : [item numberOfChildren];
}


- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item {
    return (item == nil) ? YES : ([item numberOfChildren] != -1);
}


- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item {
    if(item == nil) {
        return [[KRWBFileSystemItem rootItem] childAtIndex:index];
    } else {
        return [(KRWBFileSystemItem *)item childAtIndex:index];
    }
    //return (item == nil) ? [KRWBFileSystemItem rootItem] : [(KRWBFileSystemItem *)item childAtIndex:index];
}


- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item {
    return (item == nil) ? @"/" : [item relativePath];
}


@end
