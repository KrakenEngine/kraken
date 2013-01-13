//
//  KRWBDocument.m
//  KRWorldBuilder
//
//  Created by Kearwood Gilbert on 12-02-10.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#import "KRWBDocument.h"

#import <KREngine_osx/KRContext.h>
#import <KREngine_osx/KRVector2.h>
#import <KREngine_osx/KRVector3.h>
#import "KRWBFileSystemItem.h"

@interface KRWBDocument() {
    KRContext *_world;
    NSOutlineView *_outlineView;
}

@end

@implementation KRWBDocument

@synthesize outlineView = _outlineView;

- (KRContext *)world
{
    return _world;
}

- (id)init
{
    _world = NULL;
    self = [super init];
    if (self) {
        // Add your subclass-specific initialization here.
        // If an error occurs here, return nil.
        _world = new KRContext();
    }
    return self;
}

- (void)dealloc
{
    if(_world) {
        delete _world;
        _world = NULL;
    }
}

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"KRWBDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *)aController
{
    [super windowControllerDidLoadNib:aController];

    //[[self.windowControllers objectAtIndex:0] setBackgroundColor: [NSColor blackColor]];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    /*
     Insert code here to write your document to data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning nil.
    You can also choose to override -fileWrapperOfType:error:, -writeToURL:ofType:error:, or -writeToURL:ofType:forSaveOperation:originalContentsURL:error: instead.
    */
    NSException *exception = [NSException exceptionWithName:@"UnimplementedMethod" reason:[NSString stringWithFormat:@"%@ is unimplemented", NSStringFromSelector(_cmd)] userInfo:nil];
    @throw exception;
    return nil;
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError
{
    /*
    Insert code here to read your document from the given data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning NO.
    You can also choose to override -readFromFileWrapper:ofType:error: or -readFromURL:ofType:error: instead.
    If you override either of these, you should also override -isEntireFileLoaded to return NO if the contents are lazily loaded.
    */
    /*
    NSException *exception = [NSException exceptionWithName:@"UnimplementedMethod" reason:[NSString stringWithFormat:@"%@ is unimplemented", NSStringFromSelector(_cmd)] userInfo:nil];
    @throw exception;
     */
    if(_world) delete _world;
    _world = new KRContext();
    
    
    return YES;
}

+ (BOOL)autosavesInPlace
{
    return YES;
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
