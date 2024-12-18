#pragma once

#include "../Module.hpp"
#include "../../../../SDK/Client/Network/Packet/TextPacket.hpp"
#include "../../../../Utils/Utils.hpp"
#include "../../../../Config/json/json.hpp"
#include <string>
#include <windows.h>
#include <wininet.h>
#include <regex>
#include <format>
#include <chrono>



using namespace std;

typedef unordered_map<string, string> strStrMap;



string round_places(const auto value, const int& places) {
    string value2 = to_string(value);
    return value2.substr(0, value2.find(".") + 1 + places);
}

string fixFormatting(const string str) {
    string res;
    for (char i : str) {
        if (i != '\u00C2') {
            res += i;
        }
    }
    return res;
}

string URLencodeIGN(const string ign) {
    return regex_replace(ign, regex("/ +/"), "\%20");
}

/*
string removeFormatting(string str) {
    return regex_replace(str, regex("Â§."), "");
}
*/

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

//string GetString(const string& URL) {
//    HINTERNET interwebs = InternetOpenA("Samsung Smart Fridge", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
//    HINTERNET urlFile;
//    string rtn;
//
//    if (interwebs) {
//        urlFile = InternetOpenUrlA(interwebs, URL.c_str(), NULL, NULL, INTERNET_FLAG_RELOAD, 0);
//
//        if (urlFile) {
//            const string header = "User-Agent: MyCustomUserAgent\r\n"
//                "Accept: application/json\r\n";
//
//            HttpAddRequestHeadersA(urlFile, header.c_str(), header.length(), HTTP_ADDREQ_FLAG_ADD);
//
//            // Send the request with headers (HttpSendRequest is called implicitly during InternetOpenUrlA)
//            char buffer[2000];
//            DWORD bytesRead;
//            do {
//                InternetReadFile(urlFile, buffer, 2000, &bytesRead);
//                rtn.append(buffer, bytesRead);
//                memset(buffer, 0, 2000);
//            } while (bytesRead);
//            InternetCloseHandle(urlFile);
//        }
//        InternetCloseHandle(interwebs);
//    }
//
//    return rtn;
//}



class NGClass {

private:

    static string get_unicode(const string& type, const string& arg) {
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

    static string get_team(const string& teamColor, const int& playersTotal) {
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
            return format("§r§{}", teamColor);
        }
        else {
            return format("§r§l§{}{} §r§{}", teamColor, teamColorDict[teamColor], teamColor);
        }
    }

    static strStrMap get_stats(const string& playerName, const bool& useUnicode) {
        const json respJson = json::parse(Utils::DownloadString(format("https://api.ngmc.co/v1/players/{}?withFactionData=false&withOnline=false&withPunishments=false&withVoteStatus=false&withGuildData=true", URLencodeIGN(playerName))));

        strStrMap stats;

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
                tier = tier.substr(1, tier.length() - 2);
            }

            string rank = "";
            if (respJson["ranks"].size() > 0) {
                rank = to_string(respJson["ranks"][0]);
                rank = rank.substr(1, rank.length() - 2);
            }

            if (useUnicode) {
                tier = get_unicode("tier", tier);
                rank = get_unicode("rank", rank);
            }
            tier = tier + " ";
            rank = rank + " ";

            string guild = "";
            if (respJson["guildData"].size() > 0) {
                guild = to_string(respJson["guildData"]["rawTag"]);
                guild = " §l" + guild.substr(1, guild.length() - 2);
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

    static bool print_stats(strStrMap& parsed_message, const bool& useUnicodes) {
        auto start = chrono::high_resolution_clock::now();
        strStrMap stats = NGClass::get_stats(parsed_message["name"], useUnicodes);
        auto elapsed = duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start);
        string team_text = NGClass::get_team(parsed_message["team_color"], stoi(parsed_message["players_total"]));
        if (stats["nicked"] == "true") {
            SDK::clientInstance->getGuiData()->displayClientMessage(format("{}{}{} §r§cNicked", team_text, stats["name"], parsed_message["end_of_message"]));
            return true;
        }
        else {
            SDK::clientInstance->getGuiData()->displayClientMessage(format("{} {}{}{}{} §r({}){}§r{} (§r§7{}s)", stats["level"], stats["tier"], stats["rank"], team_text, stats["name"], stats["kdr"], stats["guild"], parsed_message["end_of_message"], round_places(elapsed.count() / 1000.0, 2)));
            return true;
        }
        return false;
    }

};

strStrMap gamemode_keys = {
            { "BedWars", "bed" },
            { "BED", "bed" },
            { "SkyWars", "sky" },
            { "SKY", "sky" }
};
string current_gamemode = "";
string current_gamemode_key = "";

class HiveClass {

private:

    static strStrMap get_stats(const string& playerName, const string& gamemode_key) {
        const json respJson = json::parse(Utils::DownloadString(format("https://api.playhive.com/v0/game/all/{}/{}", gamemode_key, URLencodeIGN(playerName))));

        int kills = respJson["kills"];
        int deaths = respJson["deaths"];
        string akdr;
        if (deaths == 0) {
            akdr = to_string(kills);
        }
        else {
            akdr = round_places(kills / (deaths * 1.0), 2);
        }
        strStrMap stats = {
            { "xp", respJson["xp"] },
            { "games_total", respJson["played"] },
            { "games_won", respJson["victories"] },
            { "total_kills", to_string(kills) },
            { "total_deaths", to_string(deaths) },
            { "akdr", akdr }
        };

        if (gamemode_key == "bed") {

            stats["final_kills"] = respJson["final_kills"];
            stats["beds"] = respJson["beds_destroyed"];

        }
        else if (gamemode_key == "sky") {

            stats["chests"] = respJson["mystery_chests_destroyed"];
            stats["ores"] = respJson["ores_mined"];
            stats["spells"] = respJson["spells_used"];

        }

    }

public:

    /*strStrMap gamemode_keys = {
            { "BedWars", "bed" },
            { "BED", "bed" },
            { "SkyWars", "sky" },
            { "SKY", "sky" }
    };
    string current_gamemode = "";
    string current_gamemode_key = "";*/

    static bool print_stats(strStrMap& parsed_message) {
        strStrMap stats = HiveClass::get_stats(parsed_message["name"], current_gamemode_key);

        SDK::clientInstance->getGuiData()->displayClientMessage(stats["kills"] + "| |" + stats["deaths"] + "| |" + stats["akdr"]);

        return false;

    }

};



class MessageParser {

public:

    strStrMap nethergames(string mesg) {
        const string triggerMesg = " §ehas joined ";
        const size_t loc = mesg.find(triggerMesg);
        if (loc == string::npos) {
            return {
                { "passed", "false" },
                { "original_message", mesg }
            };
        }
        else {
            const string playerCount = mesg.substr(loc + triggerMesg.length(), mesg.length() - 1);
            const size_t slashIndex = playerCount.find("\/");
            return {
                { "passed", "true" },
                { "name", mesg.substr(3, loc - 3) },
                { "team_color", mesg.substr(2, 1) },
                { "player_count", playerCount },
                { "players_total", playerCount.substr(slashIndex + 4, playerCount.length() - slashIndex - 6) },
                { "end_of_mesg", mesg.substr(loc) },
                { "original_message", mesg }
            };
        }
    }

    strStrMap hiveGamemode(string mesg) {
        const string triggerMesg = "§b§l» §r§7§7Finding you a game of ";
        const size_t loc = mesg.find(triggerMesg);
        const string gamemode = mesg.substr(loc, mesg.length() - 3);
        strStrMap stats;
        if (loc == string::npos) {
            stats = {
                { "passed", "false"},
                { "original_message", mesg }
            };
        }
        else {
            current_gamemode = gamemode;
            current_gamemode_key = gamemode_keys[gamemode];
            stats = {
                { "passed", "true" },
                { "gamemode", gamemode },
                { "gamemode_key", gamemode_keys[gamemode] },
                { "original_message", mesg }
            };
        }
        return stats;
    }

    /*void hiveGamemodeLite(string mesg) {
        const string triggerMesg = "§b§l» §r§7§7Finding you a game of ";
        const size_t loc = mesg.find(triggerMesg);
        const string gamemode = mesg.substr(loc, mesg.length() - 3);
        if (loc != string::npos) {
            current_gamemode = gamemode;
            current_gamemode_key = gamemode_keys[gamemode];
        }
    }*/

    strStrMap hivePlayer(string mesg) {
        const string triggerMesg1 = "§a§l» §r";
        const string triggerMesg2 = " joined.§8";
        const size_t loc1 = mesg.find(triggerMesg1);
        const size_t loc2 = mesg.find(triggerMesg2);
        if (loc1 == string::npos || loc2 == string::npos) {
            return {
                { "passed", "false"}
            };
        }
        else {
            const string playerCount = mesg.substr(loc2);
            const size_t slashIndex = mesg.find("\/");
            return {
                { "passed", "true" },
                { "name", mesg.substr(loc1, loc2) },
                { "players_total", playerCount.substr(slashIndex + 1, playerCount.length() - slashIndex - 2) }
            };
        }
    }

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

            static strStrMap parsed_message = Parser.nethergames(fixFormatting(pkt->message));

            if (parsed_message["passed"] == "true") {
                bool printed = NGClass::print_stats(parsed_message, settings.getSettingByName<bool>("useUnicodes")->value);
                if (printed) {
                    event.cancel();
                }
            }

        }
        else if ( SDK::getServerIP().find("hivebedrock") != string::npos || settings.getSettingByName<bool>("debugMode")->value ) {

            static strStrMap parsed_gamemode = Parser.hiveGamemode(fixFormatting(pkt->message));
            //Parser.hiveGamemodeLite(fixFormatting(pkt->message));

            if (parsed_gamemode["passed"] == "true") {
                SDK::clientInstance->getGuiData()->displayClientMessage(parsed_gamemode["gamemode"]);
            }

            static strStrMap parsed_message = Parser.hivePlayer(fixFormatting(pkt->message));

            if (parsed_message["passed"] == "true") {
                // SDK::clientInstance->getGuiData()->displayClientMessage(parsed_message["players_total"] + "| |" + parsed_message["name"]);
                bool printed = HiveClass::print_stats(parsed_message);
                if (printed) {
                    event.cancel();
                }
            }

        }

    }

    }
};

// §cPrathpro17 §ehas joined (§b5§e/§b8§e)!
// Â§bÂ§lÂ» Â§rÂ§7Â§7Finding you a game of BedWars...
// Â§aÂ§lÂ» Â§rÂ§7AnshgamerYT8183 joined.Â§8[6 / 8]
