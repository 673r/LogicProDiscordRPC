#import <Cocoa/Cocoa.h>
#include <string>
#include "engine.h"

static NSString * const kLaunchAgentLabel = @"com.benzo.logicrpc";
static NSString * LaunchAgentPlistPath() {
    return [NSHomeDirectory() stringByAppendingPathComponent:
            @"Library/LaunchAgents/com.benzo.logicrpc.plist"];
}

static NSString * ExecutablePath() {
    return [[NSBundle mainBundle] executablePath];
}

static NSString * PlistContents(NSString *execPath) {
    return [NSString stringWithFormat:
        @"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
         "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
         "<plist version=\"1.0\">\n"
         "<dict>\n"
         "  <key>Label</key>\n"
         "  <string>%@</string>\n"
         "  <key>ProgramArguments</key>\n"
         "  <array>\n"
         "    <string>%@</string>\n"
         "  </array>\n"
         "  <key>RunAtLoad</key>\n"
         "  <true/>\n"
         "  <key>KeepAlive</key>\n"
         "  <false/>\n"
         "</dict>\n"
         "</plist>\n",
         kLaunchAgentLabel, execPath];
}

static int RunLaunchctl(NSArray<NSString*> *args) {
    NSTask *task = [[NSTask alloc] init];
    task.launchPath = @"/bin/launchctl";
    task.arguments = args;

    @try {
        [task launch];
        [task waitUntilExit];
        return task.terminationStatus;
    } @catch (NSException *ex) {
        return 1;
    }
}

static BOOL IsStartAtLoginEnabled() {
    return [[NSFileManager defaultManager] fileExistsAtPath:LaunchAgentPlistPath()];
}

static BOOL EnableStartAtLogin() {
    NSString *dir = [LaunchAgentPlistPath() stringByDeletingLastPathComponent];
    [[NSFileManager defaultManager] createDirectoryAtPath:dir
                              withIntermediateDirectories:YES
                                               attributes:nil
                                                    error:nil];

    NSString *plist = PlistContents(ExecutablePath());
    NSError *err = nil;
    BOOL ok = [plist writeToFile:LaunchAgentPlistPath()
                      atomically:YES
                        encoding:NSUTF8StringEncoding
                           error:&err];
    return ok;
}


static void DisableStartAtLogin() {
    NSString *uidStr = [NSString stringWithFormat:@"%d", getuid()];
    NSString *domain = [NSString stringWithFormat:@"gui/%@", uidStr];

    RunLaunchctl(@[@"bootout", domain, LaunchAgentPlistPath()]);
    [[NSFileManager defaultManager] removeItemAtPath:LaunchAgentPlistPath() error:nil];
}

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (strong) NSStatusItem *statusItem;
@property (strong) NSMenuItem *startAtLoginItem;
@property (strong) NSMenuItem *statusMenuItem;
@property (strong) NSMenuItem *projectMenuItem;
@property (strong) NSTimer *updateTimer;
@property (assign) BOOL isConnected;
@property (assign) BOOL isAuthenticated;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];

    self.statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
    if (self.statusItem.button) {
        NSImage *icon = [NSImage imageWithSystemSymbolName:@"music.note" accessibilityDescription:@"Logic RPC"];
        if (icon) {
            [icon setTemplate:YES];
            self.statusItem.button.image = icon;
        } else {
            self.statusItem.button.title = @"♪";
        }
        self.statusItem.button.toolTip = @"Logic Pro Discord RPC";
    }
    
    self.isConnected = NO;
    self.isAuthenticated = NO;

    NSMenu *menu = [[NSMenu alloc] init];
    
    self.statusMenuItem = [[NSMenuItem alloc] initWithTitle:@"Status: Initializing..."
                                                     action:nil
                                              keyEquivalent:@""];
    [self.statusMenuItem setEnabled:NO];
    [menu addItem:self.statusMenuItem];
    
    self.projectMenuItem = [[NSMenuItem alloc] initWithTitle:@"Project: None"
                                                      action:nil
                                               keyEquivalent:@""];
    [self.projectMenuItem setEnabled:NO];
    [menu addItem:self.projectMenuItem];

    [menu addItem:[NSMenuItem separatorItem]];

    self.startAtLoginItem = [[NSMenuItem alloc] initWithTitle:@"Start at Login"
                                                       action:@selector(toggleStartAtLogin)
                                                keyEquivalent:@""];
    self.startAtLoginItem.target = self;
    self.startAtLoginItem.state = IsStartAtLoginEnabled() ? NSControlStateValueOn : NSControlStateValueOff;
    [menu addItem:self.startAtLoginItem];

    [menu addItem:[NSMenuItem separatorItem]];

    NSMenuItem *authItem = [[NSMenuItem alloc] initWithTitle:@"Re-authenticate"
                                                      action:@selector(authenticate)
                                               keyEquivalent:@""];
    authItem.target = self;
    [menu addItem:authItem];

    [menu addItem:[NSMenuItem separatorItem]];

    NSMenuItem *quitItem = [[NSMenuItem alloc] initWithTitle:@"Quit"
                                                      action:@selector(quit)
                                               keyEquivalent:@"q"];
    quitItem.target = self;
    [menu addItem:quitItem];

    self.statusItem.menu = menu;
    
    self.updateTimer = [NSTimer scheduledTimerWithTimeInterval:2.0
                                                        target:self
                                                      selector:@selector(updateMenuStatus)
                                                      userInfo:nil
                                                       repeats:YES];

    engine::start_idle();
    [self updateMenuStatus];
}

- (void)toggleStartAtLogin {
    if (self.startAtLoginItem.state == NSControlStateValueOn) {
        DisableStartAtLogin();
        self.startAtLoginItem.state = NSControlStateValueOff;
    } else {
        BOOL ok = EnableStartAtLogin();
        self.startAtLoginItem.state = ok ? NSControlStateValueOn : NSControlStateValueOff;

        if (!ok) {
            NSBeep();
        }
    }
}

- (void)authenticate {
    engine::reset_auth_and_authenticate();
}


- (void)updateMenuStatus {
    extern bool is_logic_pro_running();
    extern std::string get_logic_project_name();
    
    BOOL logicRunning = is_logic_pro_running();
    
    if (logicRunning) {
        std::string projectName = get_logic_project_name();
        if (!projectName.empty()) {
            NSString *project = [NSString stringWithUTF8String:projectName.c_str()];
            self.projectMenuItem.title = [NSString stringWithFormat:@"Project: %@", project];
            self.statusMenuItem.title = @"Status: ✓ Connected & Active";
        } else {
            self.projectMenuItem.title = @"Project: Browsing...";
            self.statusMenuItem.title = @"Status: ✓ Connected";
        }
    } else {
        self.projectMenuItem.title = @"Project: None";
        self.statusMenuItem.title = @"Status: Logic Pro Not Running";
    }
}

- (void)quit {
    if (self.updateTimer) {
        [self.updateTimer invalidate];
        self.updateTimer = nil;
    }
    engine::stop();
    [NSApp terminate:nil];
}

@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        AppDelegate *delegate = [AppDelegate new];
        app.delegate = delegate;
        NSString *bid = [[NSBundle mainBundle] bundleIdentifier];
        if (bid) {
            NSArray<NSRunningApplication *> *apps =
                [NSRunningApplication runningApplicationsWithBundleIdentifier:bid];

            if (apps.count > 1) {
                return 0;
            }
        }
        [app run];
    }
    return 0;
}
