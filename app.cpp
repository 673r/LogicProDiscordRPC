#define DISCORDPP_IMPLEMENTATION
#include "discordpp.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <functional>
#include <csignal>
#include <chrono>
#include <memory>
#include <mutex>
#include <map>
#include <ctime>

#include "logiccheck.h"
#include "projectname.h"
#include "token_store.h"
#include "config.h"

std::thread logic_thread;
static const uint64_t APPLICATION_ID = 1454592403628359872;
std::atomic<bool> running = true;

struct ProjectSession {
    std::string name;
    std::chrono::steady_clock::time_point start_time;
};

static ProjectSession current_session;
static std::mutex session_mutex;

static std::shared_ptr<discordpp::Client> g_client;
static std::mutex g_client_mtx;

void signalHandler(int) { running.store(false); }

void logic_pro_loop(std::shared_ptr<discordpp::Client> client) {
    bool was_running = false;
    std::string last_project;
    int update_counter = 0;
    const int UPDATE_INTERVAL = 5;

    while (running.load()) {
        bool is_running = is_logic_pro_running();

        if (!is_running) {
            if (was_running) {
                std::cout << "🛑 Logic Pro closed, clearing presence\n";
                client->ClearRichPresence();
                std::lock_guard<std::mutex> lock(session_mutex);
                current_session.name.clear();
            }
            was_running = false;
            last_project.clear();

            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        was_running = true;
        std::string project = get_logic_project_name();

        bool project_changed = (project != last_project);
        if (project_changed) {
            std::lock_guard<std::mutex> lock(session_mutex);
            current_session.name = project;
            current_session.start_time = std::chrono::steady_clock::now();
            last_project = project;
            update_counter = 0;
            
            if (!project.empty()) {
                std::cout << "🎵 Project opened: " << project << "\n";
            } else {
                std::cout << "📂 Browsing projects\n";
            }
        }

        if (project_changed || update_counter % UPDATE_INTERVAL == 0) {
            discordpp::Activity activity;
            activity.SetType(discordpp::ActivityTypes::Playing);

            if (!project.empty()) {
                std::string state_prefix = config::get_custom_state_prefix();
                activity.SetState(state_prefix + " " + project);
                activity.SetDetails("Creating music");
                
                if (config::get_show_elapsed_time()) {
                    std::lock_guard<std::mutex> lock(session_mutex);
                    auto now = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                        now - current_session.start_time
                    ).count();
                    
                    std::time_t current_time = std::time(nullptr);
                    std::time_t start_timestamp = current_time - elapsed;
                    
                    discordpp::ActivityTimestamps timestamps;
                    timestamps.SetStart(start_timestamp);
                    activity.SetTimestamps(timestamps);
                }
            } else {
                activity.SetState("Browsing projects");
            }

            // Add assets for better RPC display
            discordpp::ActivityAssets assets;
            assets.SetLargeImage("logic-pro-icon");
            assets.SetLargeText("Logic Pro");
            assets.SetSmallImage("music-note");
            assets.SetSmallText("Making music");
            activity.SetAssets(assets);

            client->UpdateRichPresence(activity, [](discordpp::ClientResult r) {
                if (!r.Successful()) {
                    std::cerr << "⚠️  Failed to update presence\n";
                } else {
                    std::cout << "✅ Presence updated\n";
                }
            });
        }

        update_counter++;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void app_init() {
    std::signal(SIGINT, signalHandler);

    auto client = std::make_shared<discordpp::Client>();
    {
        std::lock_guard<std::mutex> lk(g_client_mtx);
        g_client = client;
    }

    client->SetApplicationId(APPLICATION_ID);

    client->AddLogCallback([](auto message, auto severity) {
        std::cout << "[" << EnumToString(severity) << "] " << message << std::endl;
    }, discordpp::LoggingSeverity::Info);

    client->SetStatusChangedCallback([client](discordpp::Client::Status status,
                                              discordpp::Client::Error error,
                                              int32_t errorDetail) {
        std::cout << "🔄 Status changed: " << discordpp::Client::StatusToString(status) << std::endl;

        if (status == discordpp::Client::Status::Connected || status == discordpp::Client::Status::Ready) {
            std::cout << "✅ Discord client ready! Starting Logic Pro monitor...\n";
            if (!logic_thread.joinable()) {
                logic_thread = std::thread(logic_pro_loop, client);
            }
        } else if (status == discordpp::Client::Status::Disconnected) {
            std::cerr << "⚠️  Disconnected from Discord\n";
        } else if (error != discordpp::Client::Error::None) {
            std::cerr << "❌ Connection Error: "
                      << discordpp::Client::ErrorToString(error)
                      << " (Code: " << errorDetail << ")\n";
            
            if (error == discordpp::Client::Error::ConnectionFailed) {
                std::cerr << "💡 Tip: Check your internet connection and Discord is running\n";
            } else if (error == discordpp::Client::Error::UnexpectedClose) {
                std::cerr << "💡 Tip: Try re-authenticating from the menu\n";
            }
        }
    });
}

void app_try_silent_auth() {
    std::shared_ptr<discordpp::Client> client;
    {
        std::lock_guard<std::mutex> lk(g_client_mtx);
        client = g_client;
    }
    if (!client) return;

    std::string saved_refresh = token_store::load_refresh_token();
    if (!saved_refresh.empty()) {
        std::cout << "🔑 Found saved credentials\n";
    }

    if (saved_refresh.empty()) return;

    std::cout << "🔁 Silent auth: RefreshToken...\n";

    client->RefreshToken(APPLICATION_ID, saved_refresh,
        [client](discordpp::ClientResult result,
                 std::string accessToken,
                 std::string newRefreshToken,
                 discordpp::AuthorizationTokenType,
                 int32_t,
                 std::string) {

            if (!result.Successful()) {
                std::cerr << "⚠️  Silent authentication failed\n";
                std::cerr << "💡 Please use 'Re-authenticate' from the menu bar\n";
                return;
            }

            if (!newRefreshToken.empty()) token_store::save_refresh_token(newRefreshToken);

            client->UpdateToken(discordpp::AuthorizationTokenType::Bearer, accessToken,
                [client](discordpp::ClientResult r) {
                    if (r.Successful()) {
                        std::cout << "✅ Silent token updated, connecting...\n";
                        client->Connect();
                    } else {
                        std::cerr << "❌ UpdateToken failed after silent refresh\n";
                    }
                }
            );
        }
    );
}

void app_begin_auth() {
    std::shared_ptr<discordpp::Client> client;
    {
        std::lock_guard<std::mutex> lk(g_client_mtx);
        client = g_client;
    }
    if (!client) return;

    std::string saved_refresh = token_store::load_refresh_token();
    if (!saved_refresh.empty()) {
        std::cout << "🔁 Trying RefreshToken (silent)...\n";
        client->RefreshToken(APPLICATION_ID, saved_refresh,
            [client](discordpp::ClientResult result,
                     std::string accessToken,
                     std::string newRefreshToken,
                     discordpp::AuthorizationTokenType,
                     int32_t,
                     std::string) {

                if (!result.Successful()) {
                    std::cerr << "❌ RefreshToken failed, need browser auth\n";
                    return;
                }

                if (!newRefreshToken.empty()) token_store::save_refresh_token(newRefreshToken);

                client->UpdateToken(discordpp::AuthorizationTokenType::Bearer, accessToken,
                    [client](discordpp::ClientResult r) {
                        if (r.Successful()) {
                            std::cout << "✅ Token updated, connecting...\n";
                            client->Connect();
                        } else {
                            std::cerr << "❌ UpdateToken failed after refresh\n";
                        }
                    }
                );
            }
        );
        return;
    }

    std::cout << "🌐 First-time setup: Opening browser for authentication...\n";
    auto codeVerifier = client->CreateAuthorizationCodeVerifier();

    discordpp::AuthorizationArgs args{};
    args.SetClientId(APPLICATION_ID);
    args.SetScopes({ "openid", "sdk.social_layer_presence" });
    args.SetCodeChallenge(codeVerifier.Challenge());

    client->Authorize(args, [client, codeVerifier](auto result, auto code, auto redirectUri) {
        if (!result.Successful()) {
            std::cerr << "❌ Authentication failed: " << result.Error() << std::endl;
            std::cerr << "💡 Please try again or check Discord is running\n";
            return;
        }
        std::cout << "✅ Authentication successful!\n";

        client->GetToken(
            APPLICATION_ID, code, codeVerifier.Verifier(), redirectUri,
            [client](discordpp::ClientResult result,
                     std::string accessToken,
                     std::string refreshToken,
                     discordpp::AuthorizationTokenType,
                     int32_t,
                     std::string) {

                if (!result.Successful()) {
                    std::cerr << "❌ GetToken failed\n";
                    return;
                }

                if (!refreshToken.empty()) token_store::save_refresh_token(refreshToken);

                client->UpdateToken(discordpp::AuthorizationTokenType::Bearer, accessToken,
                    [client](discordpp::ClientResult r) {
                        if (r.Successful()) {
                            std::cout << "✅ Token updated, connecting...\n";
                            client->Connect();
                        } else {
                            std::cerr << "❌ UpdateToken failed\n";
                        }
                    }
                );
            }
        );
    });
}

int app_main() {
    std::cout << "🚀 Logic Pro Discord RPC v2.0\n";
    config::load();
    std::cout << "📡 Initializing Discord SDK...\n";
    app_init();

    app_try_silent_auth();

    while (running.load()) {
        discordpp::RunCallbacks();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    std::cout << "👋 Shutting down...\n";
    config::save();
    // Do not join logic_thread here — engine::stop() already joins it. Double-join causes SIGABRT on quit.
    return 0;
}
