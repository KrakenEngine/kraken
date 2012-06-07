//
//  KRWBFileSystemItem.m
//  KRWorldBuilder
//
//  Created by Kearwood Gilbert on 12-05-12.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#import "KRWBFileSystemItem.h"


@implementation KRWBFileSystemItem

static KRWBFileSystemItem *rootItem = nil;
static NSMutableArray *leafNode = nil;

+ (void)initialize {
    if (self == [KRWBFileSystemItem class]) {
        leafNode = [[NSMutableArray alloc] init];
    }
}

- (id)initWithPath:(NSURL *)url parent:(KRWBFileSystemItem *)parentItem {
    self = [super init];
    if (self) {
        fullPath = url;
        parent = parentItem;
    }
    return self;
}


+ (KRWBFileSystemItem *)rootItem {
    if (rootItem == nil) {
        rootItem = [[KRWBFileSystemItem alloc] initWithPath:[NSURL URLWithString:@"file://localhost/"] parent:nil];
    }
    return rootItem;
}


// Creates, caches, and returns the array of children
// Loads children incrementally
- (NSArray *)children {
    
    if (children == nil) {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSError *error;
        
        BOOL valid = [fullPath checkResourceIsReachableAndReturnError:NULL];
        
        
        NSNumber *isDirectory = nil;
        if (! [fullPath getResourceValue:&isDirectory forKey:NSURLIsDirectoryKey error:&error]) {
            // handle error
        }
        
        BOOL isDir = [isDirectory boolValue];
        if (valid && isDir) {
            NSArray *keys = [NSArray arrayWithObject:NSURLIsDirectoryKey];
            
            NSArray *child_urls = [fileManager contentsOfDirectoryAtURL:fullPath includingPropertiesForKeys:keys options: NSDirectoryEnumerationSkipsHiddenFiles error:&error];
            /*
            NSDirectoryEnumerator *enumerator = [fileManager
                 enumeratorAtURL:fullPath
                 includingPropertiesForKeys:keys
                 options:0
                 errorHandler:^(NSURL *url, NSError *error) {
                     // Handle the error.
                     // Return YES if the enumeration should continue after the error.
                     return YES;
            }];
             */
            
            children = [[NSMutableArray alloc] init];
            
            for (NSURL *url in child_urls) {
                KRWBFileSystemItem *newChild = [[KRWBFileSystemItem alloc]
                                                initWithPath:url parent:self];     
                [children addObject:newChild];
            }
        } else {
            children = leafNode;
        }
    }
    return children;
}


- (NSString *)relativePath {
    return [[fullPath lastPathComponent] copy];
}


- (KRWBFileSystemItem *)childAtIndex:(NSUInteger)n {
    return [[self children] objectAtIndex:n];
}


- (NSInteger)numberOfChildren {
    NSArray *tmp = [self children];
    return (tmp == leafNode) ? (-1) : [tmp count];
}


- (void)dealloc {

}

@end