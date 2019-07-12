#import "Cocoa/Cocoa.h"
#include "kraken.h"

using namespace kraken;

int main(int argc, const char * argv[])
{
  @autoreleasepool {
    [NSApplication sharedApplication];

    //
    // Create a window:
    //

    // Style flags:
    NSUInteger windowStyle = NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask;

    NSRect windowRect = NSMakeRect(0, 0, 640, 480);
    NSWindow * window = [[NSWindow alloc] initWithContentRect:windowRect
                                          styleMask:windowStyle
                                          backing:NSBackingStoreBuffered
                                          defer:NO];
    [window autorelease];

    // Window controller:
    NSWindowController * windowController = [[NSWindowController alloc] initWithWindow:window];
    [windowController autorelease];

    [window orderFrontRegardless];
    [NSApp run];
  }
  return 0;
}
