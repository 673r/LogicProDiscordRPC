#include <libproc.h>
#include <string.h>
#include <iostream>

bool is_logic_pro_running();

#import <AppKit/NSRunningApplication.h>

//function changed from .cpp to .mm to use NSString ->
bool is_logic_pro_running() {
    @autoreleasepool {
        NSString* bundleId = @"com.apple.logic10";
        NSArray<NSRunningApplication*>* apps =
            [NSRunningApplication runningApplicationsWithBundleIdentifier:bundleId];
        return (apps.count > 0);
    }
}

