#pragma once

#include "../Module.hpp"
#include "../../../Events/EventHandler.hpp"
#include "ClickGUIRenderer.hpp"
#include "GUIMouseListener.hpp"

class ClickGUI : public Module {

public:

    ClickGUI() : Module("ClickGUI", "What do you think it is?", "\\Flarial\\assets\\clickgui.png", 'K') {

        onEnable();

    };

    void onEnable() override {

        Module::onEnable();

        EventHandler::registerListener(new ClickGUIRenderer("ClickGUI", this));
        EventHandler::registerListener(new GUIMouseListener("GUIMouse"));

        if(settings.getSettingByName<std::string>("keybind")->value.empty()) settings.getSettingByName<std::string>("keybind")->value = "K";

    }

    void onDisable() override {

        this->settings.getSettingByName<bool>("enabled")->value = false;

        Module::onDisable();

        EventHandler::unregisterListener("ClickGUI");
        EventHandler::unregisterListener("GUIMouse");

    }
};