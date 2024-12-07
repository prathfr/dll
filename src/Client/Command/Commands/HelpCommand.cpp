#include "HelpCommand.hpp"

#include "../CommandManager.hpp"

void HelpCommand::execute(const std::vector<std::string> &args) {
    for (const auto& command : CommandManager::Commands) {
        //SDK::clientInstance->getGuiData()->displayClientMessage("§r§l§9" + command->Name + " §3§l»§r " + command->Description);
        SDK::clientInstance->getGuiData()->displayClientMessage(fmt::format("§l§9{} §3§l»§r {}", command->Name, command->Description));
    }
}
