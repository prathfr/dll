<<<<<<< HEAD
#pragma once

#include "../Module.hpp"
#include "../../../../SDK/Client/Network/Packet/TextPacket.hpp"
#include "../../../../Utils/Utils.hpp"
#include "../../../../Config/json/json.hpp"
#include <string>
//#include <windows.h>
#include <regex>
#include <format>
#include <chrono>



using namespace std;

typedef unordered_map<string, string> strStrMap;



string round_places(auto value, int places) {
    string value2 = to_string(value);
    return value2.substr(0, value2.find(".") + 1 + places);
}

string fixFormatting(string str) {
    return regex_replace(str, regex("Â§"), "\u00A7");
}

/*
string removeFormatting(string str) {
    return regex_replace(str, regex("Â§."), "");
}
*/

/*
void copyToClipboard(string str) {
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, str.size() + 1);
        if (hGlob) {
            memcpy(hGlob, str.c_str(), str.size() + 1);
            SetClipboardData(CF_TEXT, hGlob);
            CloseClipboard();
        }
    }
}
*/



class MessageParser {

public:

    strStrMap nethergames(string mesg) {
        const string triggerMesg = " \u00A7ehas joined ";
        const size_t joinIndex = mesg.find(triggerMesg);
        if (joinIndex == string::npos) {
            return {
                { "passed", "false" }
            };
        }
        else {
            const string playerCount = mesg.substr(joinIndex + triggerMesg.length(), mesg.length() - 1);
            const size_t slashIndex = playerCount.find("\/");
            return {
                { "passed", "true" },
                { "name", mesg.substr(3, joinIndex - 3) },
                { "team_color", mesg.substr(2, 1) },
                { "player_count", playerCount },
                { "players_total", playerCount.substr(slashIndex + 4, playerCount.length() - slashIndex - 6) },
                { "end_of_mesg", mesg.substr(joinIndex) }
            };
        }
    }

    strStrMap hive(string mesg) {
        return {
            { "", "" }
        };
    }

};



class NGClass {

private:

    string get_unicode(string type, string arg) {
        if (type == "tier") {
            static strStrMap tier_unicodes = {
                { "Diamond", "\uE1A7" },
                { "Sapphire", "\uE1A6" },
                { "Amethyst", "\uE1A5" },
                { "Opal", "\uE1A4" },
                { "Gold", "\uE1A3" },
                { "Silver", "\uE1A2" },
                { "Bronze", "\uE1A1" },
                { "Steel", "\uE1A0" }
            };
            return tier_unicodes[arg];
        }
        else if (type == "rank") {
            static strStrMap rank_unicodes = {
                { "Designer", "\uE5FA" },
                { "Trainee", "\uE5F9" },
                { "Supervisor", "\uE5F8" },
                { "Advisor", "\uE5F7" },
                { "Admin", "\uE5F6" },
                { "Mod", "\uE5F5" },
                { "Crew", "\uE5F4" },
                { "Builder", "\uE5F3" },
                { "Developer", "\uE5F2" },
                { "Titan", "\uE5F0" },
                { "Partner", "\uE5E6" },
                { "Youtube", "\uE5E4" },
                { "Ultra", "\uE5E2" },
                { "Legend", "\uE5E1" },
                { "Emerald", "\uE5E0" }
            };
            return rank_unicodes[arg];
        }
        else {
            return "";
        }
    }

    string get_team(string teamColor, int playersTotal) {
        const int acceptablePlayerCounts[3] = { 8, 16, 20 };
        static strStrMap teamColorDict = {
            { "c", "R" },
            { "1", "B" },
            { "a", "G" }, // green
            { "e", "Y" },
            { "3", "C" },
            { "f", "W" },
            { "d", "P" },
            { "8", "G" } // gray
        };
        if (
            find(begin(acceptablePlayerCounts), end(acceptablePlayerCounts), playersTotal) == end(acceptablePlayerCounts)
            || !teamColorDict.contains(teamColor)
            ) {
            return format("\u00A7r\u00A7{}", teamColor);
        }
        else {
            return format("\u00A7r\u00A7l\u00A7{}{} \u00A7r\u00A7{}", teamColor, teamColorDict[teamColor], teamColor);
        }
    }

    strStrMap get_stats(string playerName, bool useUnicode) {
        const json respJson = json::parse(Utils::DownloadString(format("https://api.ngmc.co/v1/players/{}?withFactionData=false&withOnline=false&withPunishments=false&withVoteStatus=false&withGuildData=true", regex_replace(playerName, regex("/ +/"), "\%20"))));

        strStrMap stats = {};

        if (respJson.contains("code") && respJson.contains("message")) {
            if (respJson["code"] == 10012 && respJson["message"] == "Unknown Player") {

                stats = {
                    { "name", playerName },
                    { "nicked", "true"}
                };
            }
        }
        else {

            string level = to_string(respJson["formattedLevel"]);
            level = level.substr(1, level.length() - 2);

            string tier = to_string(respJson["tier"]);
            if (tier.size() > 0) {
                tier = tier.substr(1, tier.length() - 2) + " ";
            }

            string rank = "";
            if (respJson["ranks"].size() > 0) {
                rank = to_string(respJson["ranks"][0]);
                rank = rank.substr(1, rank.length() - 2) + " ";
            }

            if (useUnicode) {
                tier = get_unicode("tier", tier);
                rank = get_unicode("rank", rank);
            }

            string guild = "";
            if (respJson["guildData"].size() > 0) {
                guild = to_string(respJson["guildData"]["rawTag"]);
                guild = " \u00A7l" + guild.substr(1, guild.length() - 2);
            }

            stats = {
                { "name", playerName },
                { "nicked", "false" },
                { "level", level },
                { "tier", tier },
                { "rank", rank },
                { "kdr", to_string(respJson["kdr"]) },
                { "guild", guild }
            };

        }

        return stats;
    }

public:

    bool print_stats(strStrMap parsed_message, bool useUnicodes) {
        auto start = chrono::high_resolution_clock::now();
        strStrMap stats = this->get_stats(parsed_message["name"], useUnicodes);
        auto elapsed = duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start);
        string team_text = this->get_team(parsed_message["team_color"], stoi(parsed_message["players_total"]));
        if (stats["nicked"] == "true") {
            SDK::clientInstance->getGuiData()->displayClientMessage(format("{}{}{} \u00A7r\u00A7cNicked", team_text, stats["name"], parsed_message["end_of_message"]));
            return true;
        }
        else {
            SDK::clientInstance->getGuiData()->displayClientMessage(format("{} {}{}{}{} \u00A7r({}){}\u00A7r{} (\u00A7r\u00A77{}s)", stats["level"], stats["tier"], stats["rank"], team_text, stats["name"], stats["kdr"], stats["guild"], parsed_message["end_of_message"], round_places(elapsed.count() / 1000.0, 2)));
            return true;
        }
        return false;
    }

};



class HiveClass {

};



class ServerTweaks : public Module {

public:
    ServerTweaks() : Module("ServerTweaks", "ServerTweaks",
        IDR_SKULL_PNG, "") {
        Module::setup();
    };

    void onEnable() override {
        Listen(this, PacketEvent, &ServerTweaks::onPacketReceive)
            Module::onEnable();
    }

    void onDisable() override {
        Deafen(this, PacketEvent, &ServerTweaks::onPacketReceive)
            Module::onDisable();
    }

    void defaultConfig() override {
        if (settings.getSettingByName<bool>("debugMode") == nullptr) settings.addSetting("debugMode", (bool) false);
        if (settings.getSettingByName<bool>("useUnicodes") == nullptr) settings.addSetting("useUnicodes", (bool) true);
    }

    void settingsRender(float settingsOffset) override {

        float x = Constraints::PercentageConstraint(0.019, "left");
        float y = Constraints::PercentageConstraint(0.10, "top");

        const float scrollviewWidth = Constraints::RelativeConstraint(0.12, "height", true);


        FlarialGUI::ScrollBar(x, y, 140, Constraints::SpacingConstraint(5.5, scrollviewWidth), 2);
        FlarialGUI::SetScrollView(x - settingsOffset, Constraints::PercentageConstraint(0.00, "top"),
            Constraints::RelativeConstraint(1.0, "width"),
            Constraints::RelativeConstraint(0.88f, "height"));

        this->addHeader("Module Settings");
        this->addToggle("Use Unicodes", "Uses unicodes (the fancy custom text) instead of regular text", settings.getSettingByName<bool>("useUnicodes")->value);

        this->extraPadding();

        this->addHeader("Developer Settings");
        this->addToggle("Debug Mode", "debugMode", settings.getSettingByName<bool>("debugMode")->value);

        FlarialGUI::UnsetScrollView();

        this->resetPadding();
    }

    void onPacketReceive(PacketEvent& event) {

    if (event.getPacket()->getId() == MinecraftPacketIds::Text) {

        auto* pkt = reinterpret_cast<TextPacket*>(event.getPacket());
        MessageParser Parser;

        if ( SDK::getServerIP().find("nethergames") != string::npos || settings.getSettingByName<bool>("debugMode")->value ) {

            strStrMap parsed_message = Parser.nethergames(fixFormatting(pkt->message));

            if (parsed_message["passed"] == "true") {
                NGClass NG;
                bool printed = NG.print_stats(parsed_message, settings.getSettingByName<bool>("useUnicodes")->value);
                if (printed) {
                    event.cancel();
                }
            }

        }
        else if ( SDK::getServerIP().find("hivebedrock") != string::npos || settings.getSettingByName<bool>("debugMode")->value ) {

        }

    }

    }
};

// §cPrathpro17 §ehas joined (§b5§e/§b8§e)!

=======
#pragma once

#include "../Module.hpp"
#include "../../../../SDK/Client/Network/Packet/TextPacket.hpp"
#include "../../../../Utils/Utils.hpp"
#include "../../../../Config/json/json.hpp"
#include <string>
//#include <windows.h>
#include <regex>
#include <format>
#include <chrono>



using namespace std;

typedef unordered_map<string, string> strStrMap;



string round_places(auto value, int places) {
    string value2 = to_string(value);
    return value2.substr(0, value2.find(".") + 1 + places);
}

string fixFormatting(string str) {
    return regex_replace(str, regex("Â§"), "\u00A7");
}

/*
string removeFormatting(string str) {
    return regex_replace(str, regex("Â§."), "");
}
*/

/*
void copyToClipboard(string str) {
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, str.size() + 1);
        if (hGlob) {
            memcpy(hGlob, str.c_str(), str.size() + 1);
            SetClipboardData(CF_TEXT, hGlob);
            CloseClipboard();
        }
    }
}
*/



class MessageParser {

public:

    strStrMap nethergames(string mesg) {
        const string triggerMesg = " \u00A7ehas joined ";
        const size_t joinIndex = mesg.find(triggerMesg);
        if (joinIndex == string::npos) {
            return {
                { "passed", "false" }
            };
        }
        else {
            const string playerCount = mesg.substr(joinIndex + triggerMesg.length(), mesg.length() - 1);
            const size_t slashIndex = playerCount.find("\/");
            return {
                { "passed", "true" },
                { "name", mesg.substr(3, joinIndex - 3) },
                { "team_color", mesg.substr(2, 1) },
                { "player_count", playerCount },
                { "players_total", playerCount.substr(slashIndex + 4, playerCount.length() - slashIndex - 6) },
                { "end_of_mesg", mesg.substr(joinIndex) }
            };
        }
    }

    strStrMap hive(string mesg) {
        return {
            { "", "" }
        };
    }

};



class NGClass {

private:

    string get_unicode(string type, string arg) {
        if (type == "tier") {
            static strStrMap tier_unicodes = {
                { "Diamond", "\uE1A7" },
                { "Sapphire", "\uE1A6" },
                { "Amethyst", "\uE1A5" },
                { "Opal", "\uE1A4" },
                { "Gold", "\uE1A3" },
                { "Silver", "\uE1A2" },
                { "Bronze", "\uE1A1" },
                { "Steel", "\uE1A0" }
            };
            return tier_unicodes[arg];
        }
        else if (type == "rank") {
            static strStrMap rank_unicodes = {
                { "Designer", "\uE5FA" },
                { "Trainee", "\uE5F9" },
                { "Supervisor", "\uE5F8" },
                { "Advisor", "\uE5F7" },
                { "Admin", "\uE5F6" },
                { "Mod", "\uE5F5" },
                { "Crew", "\uE5F4" },
                { "Builder", "\uE5F3" },
                { "Developer", "\uE5F2" },
                { "Titan", "\uE5F0" },
                { "Partner", "\uE5E6" },
                { "Youtube", "\uE5E4" },
                { "Ultra", "\uE5E2" },
                { "Legend", "\uE5E1" },
                { "Emerald", "\uE5E0" }
            };
            return rank_unicodes[arg];
        }
        else {
            return "";
        }
    }

    string get_team(string teamColor, int playersTotal) {
        const int acceptablePlayerCounts[3] = { 8, 16, 20 };
        static strStrMap teamColorDict = {
            { "c", "R" },
            { "1", "B" },
            { "a", "G" }, // green
            { "e", "Y" },
            { "3", "C" },
            { "f", "W" },
            { "d", "P" },
            { "8", "G" } // gray
        };
        if (
            find(begin(acceptablePlayerCounts), end(acceptablePlayerCounts), playersTotal) == end(acceptablePlayerCounts)
            || !teamColorDict.contains(teamColor)
            ) {
            return format("\u00A7r\u00A7{}", teamColor);
        }
        else {
            return format("\u00A7r\u00A7l\u00A7{}{} \u00A7r\u00A7{}", teamColor, teamColorDict[teamColor], teamColor);
        }
    }

    strStrMap get_stats(string playerName, bool useUnicode) {
        const json respJson = json::parse(Utils::DownloadString(format("https://api.ngmc.co/v1/players/{}?withFactionData=false&withOnline=false&withPunishments=false&withVoteStatus=false&withGuildData=true", regex_replace(playerName, regex("/ +/"), "\%20"))));

        strStrMap stats = {};

        if (respJson.contains("code") && respJson.contains("message")) {
            if (respJson["code"] == 10012 && respJson["message"] == "Unknown Player") {

                stats = {
                    { "name", playerName },
                    { "nicked", "true"}
                };
            }
        }
        else {

            string level = to_string(respJson["formattedLevel"]);
            level = level.substr(1, level.length() - 2);

            string tier = to_string(respJson["tier"]);
            if (tier.size() > 0) {
                tier = tier.substr(1, tier.length() - 2) + " ";
            }

            string rank = "";
            if (respJson["ranks"].size() > 0) {
                rank = to_string(respJson["ranks"][0]);
                rank = rank.substr(1, rank.length() - 2) + " ";
            }

            if (useUnicode) {
                tier = get_unicode("tier", tier);
                rank = get_unicode("rank", rank);
            }

            string guild = "";
            if (respJson["guildData"].size() > 0) {
                guild = to_string(respJson["guildData"]["rawTag"]);
                guild = " \u00A7l" + guild.substr(1, guild.length() - 2);
            }

            stats = {
                { "name", playerName },
                { "nicked", "false" },
                { "level", level },
                { "tier", tier },
                { "rank", rank },
                { "kdr", to_string(respJson["kdr"]) },
                { "guild", guild }
            };

        }

        return stats;
    }

public:

    bool print_stats(strStrMap parsed_message, bool useUnicodes) {
        auto start = chrono::high_resolution_clock::now();
        strStrMap stats = this->get_stats(parsed_message["name"], useUnicodes);
        auto elapsed = duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start);
        string team_text = this->get_team(parsed_message["team_color"], stoi(parsed_message["players_total"]));
        if (stats["nicked"] == "true") {
            SDK::clientInstance->getGuiData()->displayClientMessage(format("{}{}{} \u00A7r\u00A7cNicked", team_text, stats["name"], parsed_message["end_of_message"]));
            return true;
        }
        else {
            SDK::clientInstance->getGuiData()->displayClientMessage(format("{} {}{}{}{} \u00A7r({}){}\u00A7r{} (\u00A7r\u00A77{}s)", stats["level"], stats["tier"], stats["rank"], team_text, stats["name"], stats["kdr"], stats["guild"], parsed_message["end_of_message"], round_places(elapsed.count() / 1000.0, 2)));
            return true;
        }
        return false;
    }

};



class HiveClass {

};



class ServerTweaks : public Module {

public:
    ServerTweaks() : Module("ServerTweaks", "ServerTweaks",
        IDR_SKULL_PNG, "") {
        Module::setup();
    };

    void onEnable() override {
        Listen(this, PacketEvent, &ServerTweaks::onPacketReceive)
            Module::onEnable();
    }

    void onDisable() override {
        Deafen(this, PacketEvent, &ServerTweaks::onPacketReceive)
            Module::onDisable();
    }

    void defaultConfig() override {
        if (settings.getSettingByName<bool>("debugMode") == nullptr) settings.addSetting("debugMode", (bool) false);
        if (settings.getSettingByName<bool>("useUnicodes") == nullptr) settings.addSetting("useUnicodes", (bool) true);
    }

    void settingsRender(float settingsOffset) override {

        float x = Constraints::PercentageConstraint(0.019, "left");
        float y = Constraints::PercentageConstraint(0.10, "top");

        const float scrollviewWidth = Constraints::RelativeConstraint(0.12, "height", true);


        FlarialGUI::ScrollBar(x, y, 140, Constraints::SpacingConstraint(5.5, scrollviewWidth), 2);
        FlarialGUI::SetScrollView(x - settingsOffset, Constraints::PercentageConstraint(0.00, "top"),
            Constraints::RelativeConstraint(1.0, "width"),
            Constraints::RelativeConstraint(0.88f, "height"));

        this->addHeader("Module Settings");
        this->addToggle("Use Unicodes", "Uses unicodes (the fancy custom text) instead of regular text", settings.getSettingByName<bool>("useUnicodes")->value);

        this->extraPadding();

        this->addHeader("Developer Settings");
        this->addToggle("Debug Mode", "debugMode", settings.getSettingByName<bool>("debugMode")->value);

        FlarialGUI::UnsetScrollView();

        this->resetPadding();
    }

    void onPacketReceive(PacketEvent& event) {

    if (event.getPacket()->getId() == MinecraftPacketIds::Text) {

        auto* pkt = reinterpret_cast<TextPacket*>(event.getPacket());
        MessageParser Parser;

        if ( SDK::getServerIP().find("nethergames") != string::npos || settings.getSettingByName<bool>("debugMode")->value ) {

            strStrMap parsed_message = Parser.nethergames(fixFormatting(pkt->message));

            if (parsed_message["passed"] == "true") {
                NGClass NG;
                bool printed = NG.print_stats(parsed_message, settings.getSettingByName<bool>("useUnicodes")->value);
                if (printed) {
                    event.cancel();
                }
            }

        }
        else if ( SDK::getServerIP().find("hivebedrock") != string::npos || settings.getSettingByName<bool>("debugMode")->value ) {

        }

    }

    }
};

// §cPrathpro17 §ehas joined (§b5§e/§b8§e)!

>>>>>>> be96cccdf76ec0aed48bf7ec38619870c1f9b49b
