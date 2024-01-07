#pragma once
#include "SimpleIni.h"

// keyboard-gp & mouse-gp
using KeyValuePair = std::pair<const char*, int>;

namespace Settings {

    constexpr auto path = L"Data/SKSE/Plugins/CombatCameraToggle.ini";

    constexpr std::uint32_t kSerializationVersion = 1;
    constexpr std::uint32_t kDataKey = 'TGCC';


    namespace main {
        bool Third2First = false;
        bool ToggleInCombat = false;
        bool ToggleWhenWeaponMagicDrawn = false;
        bool Bow1stPAiming = false;

        const auto comment_Third2First =
            ";Set to true to switch from 3rd to 1st person instead of the other way around.";
        const auto comment_ToggleInCombat = ";Set to true to toggle view upon entering and leaving combat.";
        const auto comment_ToggleWhenWeaponMagicDrawn = ";Set to true to toggle camera view upon drawing your weapon or magic.";
        const auto comment_FirstPersonAimingBow = ";Set to true to switch to 1st person when aiming bow.";
    };

    namespace os {
        bool gradual_zoom = false;
        bool switch_back = true;

        const auto comment_gradual_zoom = ";Set to true to switch between camera states gradually.";
        const auto comment_switch_back = ";Set to false to trigger the toggle only when entering combat and not when leaving. Same goes for drawing weapons. You get the idea.";
    };

    bool LoadSettings() {
        CSimpleIniA ini;
        ini.SetUnicode();

        auto err = ini.LoadFile(path);
        if (err < 0) return false;

        // Main
        main::Third2First = ini.GetBoolValue("Main", "Third2First", main::Third2First);
        ini.SetBoolValue("Main", "Third2First", main::Third2First, main::comment_Third2First);

        main::ToggleInCombat = ini.GetBoolValue("Main", "ToggleInCombat", main::ToggleInCombat);
        ini.SetBoolValue("Main", "ToggleInCombat", main::ToggleInCombat, main::comment_ToggleInCombat);

        main::ToggleWhenWeaponMagicDrawn =
            ini.GetBoolValue("Main", "ToggleWhenWeaponMagicDrawn", main::ToggleWhenWeaponMagicDrawn);
        ini.SetBoolValue("Main", "ToggleWhenWeaponMagicDrawn", main::ToggleWhenWeaponMagicDrawn,
                         main::comment_ToggleWhenWeaponMagicDrawn);

        main::Bow1stPAiming = ini.GetBoolValue("Main", "Bow1stPAiming", main::Bow1stPAiming);
        ini.SetBoolValue("Main", "Bow1stPAiming", main::Bow1stPAiming,
                         main::comment_FirstPersonAimingBow);

        // Other
        os::gradual_zoom= ini.GetBoolValue("Other", "GradualZoom", os::gradual_zoom);
        ini.SetBoolValue("Other", "GradualZoom", os::gradual_zoom, os::comment_gradual_zoom);
        os::switch_back = ini.GetBoolValue("Other", "SwitchBackCamState", os::switch_back);
        ini.SetBoolValue("Other", "SwitchBackCamState", os::switch_back,
                         os::comment_switch_back);
        
        ini.SaveFile(path);

        return true;
    };

    class Settings {
    public:
        static Settings* GetSingleton() {
            static Settings singleton;
            return &singleton;
        }

        // never change the order of these
        std::array<std::pair<const char*, bool>, 4> main = {{{"Third2First", main::Third2First},
                                                             {"ToggleInCombat", main::ToggleInCombat},
                                                             {"ToggleWhenWeaponDrawn", main::ToggleWhenWeaponMagicDrawn},
                                                             {"Bow1stPAiming", main::Bow1stPAiming}}};
        
        std::array<std::pair<const char*, bool>, 2> os = {
            {{"GradualZoom", os::gradual_zoom}, {"SwitchBackCamState", os::switch_back}}};
    };
};