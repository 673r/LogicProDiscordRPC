#include <libproc.h>
#include <string.h>
#include <iostream>

bool is_logic_pro_running();

#import <AppKit/NSRunningApplication.h>
#import <AppKit/NSWorkspace.h>

//function changed from .cpp to .mm to use NSString ->
bool is_logic_pro_running() {
    @autoreleasepool {
        // Known bundle IDs: Logic 10 (logic10), Logic 11 (logic11), Mac App Store (logicpro), Logic 12 / Creator Studio (music.apps.logic)
        NSArray<NSString*>* bundleIds = @[
            @"com.apple.logic10",
            @"com.apple.logic11",
            @"com.apple.logicpro",
            @"com.apple.music.apps.logic"
        ];
        for (NSString* bundleId in bundleIds) {
            NSArray<NSRunningApplication*>* apps =
                [NSRunningApplication runningApplicationsWithBundleIdentifier:bundleId];
            if (apps.count > 0)
                return true;
        }
        // Fallback: match by app name (handles unknown/future bundle IDs)
        NSString* targetName = @"Logic Pro";
        for (NSRunningApplication* app in [[NSWorkspace sharedWorkspace] runningApplications]) {
            NSString* name = app.localizedName;
            if (name && [name isEqualToString:targetName])
                return true;
        }
        return false;
    }
}

