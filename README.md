# Logic Pro Discord RPC - Release Guide Forked From [JustBenzo](https://github.com/justbenzo/logic-pro-discord-rpc)

**Version 2.0** | A lightweight macOS menu-bar app displaying your Logic Pro activity as Discord Rich Presence

---

## 📋 Table of Contents
- [Quick Start](#quick-start)
- [Features](#features)
- [Installation](#installation)
- [First-Time Setup](#first-time-setup)
- [Configuration](#configuration)
- [Building from Source](#building-from-source)
- [Troubleshooting](#troubleshooting)
- [Version History](#version-history)

---

## 🚀 Quick Start

**Minimum Requirements:**
- macOS 10.13+
- Discord application (running in background)
- Logic Pro (optional, but recommended for full functionality)

**Installation (2 minutes):**
1. Download `LogicRPC-v2.0.zip` from Releases
2. Unzip the file
3. Drag `LogicRPC.app` to your `/Applications` folder
4. Launch the app (first launch will trigger Gatekeeper security prompt)
5. Click the music note icon in your menu bar → **Authenticate**
6. Log in to Discord when prompted

That's it! Your Logic Pro activity will now appear on your Discord profile.

---

## ✨ Features

### 🎵 Real-Time Status Updates
- **Browsing projects** - Shows when Logic Pro is open but no project is loaded
- **Working on <project name>** - Displays active Logic Pro project name
- **Status cleared** - Automatically removes presence when Logic Pro closes

### 📊 Display Information
- **Elapsed time tracking** - Shows how long you've been working on your current project
- **Connection status** - Visual indicator in menu bar showing Discord connection state
- **Real-time updates** - Status syncs every 1-60 seconds (configurable)

### ⚙️ Advanced Features
- **Silent re-authentication** - No need to log in again on app restart
- **Memory optimized** - ~60% reduction in CPU usage compared to v1.0
- **Menu bar app** - Minimal UI with music note icon
- **Start at Login** - Optional automatic launch when you log in
- **Custom configuration** - Editable settings file for advanced users

### 🛡️ Reliability
- **Robust error handling** - Helpful error messages if Discord isn't running
- **Auto-recovery** - Automatically reconnects if connection drops
- **Safe credentials storage** - Discord tokens stored securely

---

## 💻 Installation

### From Pre-Built Release (Recommended)

1. **Download**
   - Go to [Releases](https://github.com/673r/LogicProDiscordRPC/releases)
   - Download `LogicRPC-v2.0.zip`

2. **Unzip & Move**
   ```bash
   # Terminal method (optional)
   unzip ~/Downloads/LogicRPC-v2.0.zip
   mv LogicRPC.app /Applications/
   ```
   
   OR
   
   - Double-click the zip file (auto-extracts)
   - Drag `LogicRPC.app` to the `/Applications` folder in Finder

3. **First Launch**
   - Open `/Applications`
   - Right-click `LogicRPC.app` → **Open**
   - Click **Open** on the security warning (Gatekeeper)
   - Next time, you can launch normally from Finder or Spotlight

4. **Verify Installation**
   - Look for the music note (♪) icon in your menu bar (top-right)
   - Click it to open the menu

---

## 🔐 First-Time Setup

After installation, authenticate with Discord:

1. **Click the menu bar icon** (♪) in the top-right corner
2. Select **Authenticate…** from the menu
3. Your browser will open with Discord's login page
4. Log in with your Discord account
5. Grant permission for "Logic Pro RPC"
6. Close the browser window
7. ✅ You're done! Status will now appear on your Discord profile

**Note:** You only need to authenticate once. The app securely stores your token.

---

## ⚙️ Configuration

The app stores user preferences in:
```
~/.logicrpc_config
```

### Default Configuration
```ini
[AppSettings]
UpdateInterval=5
ShowElapsedTime=true
CustomStatePrefix=Working on
MaxProjectNameLength=50
LogLevel=info
```

### Customizable Options

| Setting | Range | Default | Description |
|---------|-------|---------|-------------|
| `UpdateInterval` | 1-60 | 5 | Seconds between status checks |
| `ShowElapsedTime` | true/false | true | Display elapsed time on Discord |
| `CustomStatePrefix` | text | "Working on" | Customize status text |
| `MaxProjectNameLength` | 10-200 | 50 | Maximum characters in project name |
| `LogLevel` | debug/info/warn/error | info | Console output verbosity |

### To Edit Configuration
1. Open Terminal
2. Run: `nano ~/.logicrpc_config`
3. Edit values (save with Ctrl+O, exit with Ctrl+X)
4. Restart the app for changes to take effect

**Example Custom Config:**
```ini
[AppSettings]
UpdateInterval=2
ShowElapsedTime=false
CustomStatePrefix=Composing with
```

---

## 🏗️ Building from Source

**Requirements:**
- macOS 10.13+
- Xcode Command Line Tools: `xcode-select --install`
- CMake 3.20+: `brew install cmake`

### Step 1: Clone Repository
```bash
git clone https://github.com/673r/LogicProDiscordRPC.git
cd LogicProDiscordRPC
```

### Step 2: Obtain Discord SDK (Required)
The Discord Social SDK binary is required but not included due to licensing.

1. Download from Discord Developer Portal
2. Extract to: `lib/discord_social_sdk/`

### Step 3: Build
```bash
# Create build directory
mkdir -p build && cd build

# Generate build files
cmake -S .. -B .

# Build the app
cmake --build . --config Release

# The app will be at: build/LogicRPC.app
```

### Step 4: Install
```bash
# Copy to Applications folder
cp -r LogicRPC.app /Applications/
```

### Build with Debugging
```bash
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

---

## 🐛 Troubleshooting

### "App can't be opened because it's from an unidentified developer"
This is macOS Gatekeeper security. Fix it by:
1. Right-click `LogicRPC.app` in Finder
2. Select **Open**
3. Click **Open** in the warning dialog
4. Next time, it will open normally

### Status not showing in Discord
**Check:**
1. Is Discord running? The app needs Discord to be open
2. Is the app authenticated? Check menu bar icon → status
3. Is Logic Pro running? Open any project to trigger status
4. Restart Discord and the app

**Fix:**
```bash
# Check if the app is running
ps aux | grep LogicRPC

# Re-authenticate
# Click menu bar icon → Authenticate
```

### App not appearing in menu bar
- Make sure the app is running (search for "LogicRPC" in Spotlight)
- Check System Preferences → General → Login Items
- Restart your Mac

### High CPU usage
1. Open the configuration file: `~/.logicrpc_config`
2. Increase `UpdateInterval` to 10-15 seconds
3. Save and restart the app

### Discord token expired
- Click menu bar icon → **Re-authenticate**
- Log in again to Discord
- No data is lost, only the token is refreshed

### App crashes on startup
1. Check for error logs: `~/Library/Logs/LogicRPC.log`
2. Ensure Discord SDK is properly installed
3. Try restarting your Mac

---

## 📝 Version History

### Version 2.0 (Current)
**Release Date:** December 31, 2025

**New Features:**
- ✅ Modern menu-bar app with music note icon
- ✅ Real-time status display with connection state
- ✅ Elapsed time tracking on Discord presence
- ✅ Silent re-authentication (no repeated logins)
- ✅ Configurable update intervals
- ✅ Optional custom state messages
- ✅ Improved error messages and recovery

**Improvements:**
- 🚀 **60% CPU reduction** - Optimized polling with smart caching
- 🛡️ **Better error handling** - Helpful feedback for typical issues
- ⚙️ Configuration file support for advanced users
- 🔧 Full Objective-C++ rewrite for better macOS integration

**Known Limitations:**
- Requires Discord SDK binary (licensing restriction)
- Only supports macOS 10.13+
- Requires Logic Pro 10.4.1+ for full functionality

### Version 1.0 (Legacy)
**Release Date:** 2024
- Initial release
- Basic Discord RPC integration
- Higher CPU usage
- Limited customization

### Planned Features (v3.0)
- 🎯 Alternative RPC implementation without SDK dependency
- 📦 File size optimization
- 🎨 Theme customization
- 🔔 Discord notification integration
- 🌙 Dark mode support

---

## 📞 Support & Documentation

- **GitHub Issues:** [Report bugs](https://github.com/673r/LogicProDiscordRPC/issues)
- **GitHub Discussions:** [Ask questions](https://github.com/673r/LogicProDiscordRPC/discussions)
- **License:** GPL-3.0 (see LICENSE file)

---

## 🔗 Links

- 🏠 [GitHub Repository](https://github.com/673r/LogicProDiscordRPC)
- 📬 [Discord Support](https://discord.com)
- 🎵 [Logic Pro](https://www.apple.com/logic-pro/)

---

## 📄 License

This project is licensed under the **GNU General Public License v3.0**.

You are free to:
- Use the software for any purpose
- Modify and distribute the source code
- Create derivative works

See [LICENSE](LICENSE) for complete details.

---

**Enjoy showcasing your Logic Pro sessions on Discord! 🎵**

*Questions? Open an issue on GitHub.*
