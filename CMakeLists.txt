cmake_minimum_required(VERSION 3.20)

# Enable Objective-C++ so .mm builds
project(DiscordSDKExample LANGUAGES C CXX OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Your target (THIS is what we build/link/copy against) ----
add_executable(LogicRPC MACOSX_BUNDLE
    main.mm
    engine.cpp
    app.cpp
    logiccheck.mm
    projectname.cpp
    token_store.cpp
    config.cpp
)

set_target_properties(LogicRPC PROPERTIES
    INSTALL_RPATH "@executable_path"
    BUILD_WITH_INSTALL_RPATH TRUE
)

# macOS app bundle settings
if(APPLE)
    set_target_properties(LogicRPC PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.benzo.logicrpc"
        MACOSX_BUNDLE_BUNDLE_NAME "Logic RPC"
        MACOSX_BUNDLE_BUNDLE_VERSION "2.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "2.0"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2024"
        MACOSX_BUNDLE_INFO_STRING "Logic Pro Discord RPC"
    )
endif()

# If you don't have app.cpp, remove it above.
# If your old code is still in main.cpp (renamed to app_main), list that file instead.

# ---- Include paths ----
# 1) Social SDK headers
set(DISCORD_SDK_ROOT "${CMAKE_SOURCE_DIR}/lib/discord_social_sdk")
set(DISCORD_SDK_LIB_DIR "${DISCORD_SDK_ROOT}/lib/release")
set(DISCORD_SDK_BIN_DIR "${DISCORD_SDK_ROOT}/bin/release")
set(DISCORD_SDK_INCLUDE_DIR "${DISCORD_SDK_ROOT}/include")

# 2) Your local headers (discordpp.h in /lib or wherever)
target_include_directories(LogicRPC PRIVATE
    ${DISCORD_SDK_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/lib          # so #include "discordpp.h" works if it's in ./lib
    ${CMAKE_SOURCE_DIR}              # so #include "engine.h" etc works
)

# ---- Link Cocoa (needed for menu bar app) ----
find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
target_link_libraries(LogicRPC PRIVATE ${COCOA_FRAMEWORK})

# ---- Platform-specific Social SDK library paths ----
if(WIN32)
    set(DISCORD_LIB_PATH "${DISCORD_SDK_LIB_DIR}/discord_partner_sdk.lib")
    set(DISCORD_SHARED_LIB "${DISCORD_SDK_BIN_DIR}/discord_partner_sdk.dll")
elseif(APPLE)
    set(DISCORD_LIB_PATH "${DISCORD_SDK_LIB_DIR}/libdiscord_partner_sdk.dylib")
    set(DISCORD_SHARED_LIB "${DISCORD_SDK_LIB_DIR}/libdiscord_partner_sdk.dylib")
else() # Linux
    set(DISCORD_LIB_PATH "${DISCORD_SDK_LIB_DIR}/libdiscord_partner_sdk.so")
    set(DISCORD_SHARED_LIB "${DISCORD_SDK_LIB_DIR}/libdiscord_partner_sdk.so")
endif()

# Link the Social SDK library (to your actual target)
target_link_libraries(LogicRPC PRIVATE ${DISCORD_LIB_PATH})

# ---- RPATH ----
if(UNIX)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    if(APPLE)
        set(CMAKE_INSTALL_RPATH "@executable_path")
        set(CMAKE_MACOSX_RPATH TRUE)
    else()
        set(CMAKE_INSTALL_RPATH "$ORIGIN")
    endif()
endif()

# ---- Copy Social SDK shared library next to the built executable ----
add_custom_command(TARGET LogicRPC POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${DISCORD_SHARED_LIB}"
    $<TARGET_FILE_DIR:LogicRPC>
)
