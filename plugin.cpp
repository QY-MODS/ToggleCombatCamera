#include "Manager.h"

//Manager* manager;


Settings::Settings* settings = nullptr;

// CAM STUFF
bool listen_gradual_zoom = false;
class OnCameraUpdate {
public:
    static void Install() {
        REL::Relocation<std::uintptr_t> hook1{REL::RelocationID(49852, 50784)};  // 84AB90, 876700

        auto& trampoline = SKSE::GetTrampoline();
        _Update = trampoline.write_call<5>(hook1.address() + REL::Relocate(0x1A6, 0x1A6),
                                           Update);  // 84AD36, 8768A6
    }

private:
    static void Update(RE::TESCamera* a_this);

    static inline REL::Relocation<decltype(Update)> _Update;
};
void OnCameraUpdate::Update(RE::TESCamera* a_camera) {
    _Update(a_camera);
    if (listen_gradual_zoom) {
        auto* thirdPersonState = static_cast<RE::ThirdPersonState*>(a_camera->currentState.get());
        if (thirdPersonState->currentZoomOffset < -0.19f) {
            listen_gradual_zoom = false;
            RE::PlayerCamera::GetSingleton()->ForceFirstPerson();
        }
    }
}

void ToggleCam(float extra_offset = 0.f) {
    listen_gradual_zoom = false;
    auto plyr_c = RE::PlayerCamera::GetSingleton();
    auto thirdPersonState =
        static_cast<RE::ThirdPersonState*>(plyr_c->cameraStates[RE::CameraState::kThirdPerson].get());
    if (plyr_c->IsInFirstPerson()) {
        plyr_c->ForceThirdPerson();
        thirdPersonState->targetZoomOffset = thirdPersonState->savedZoomOffset + extra_offset;
    } else if (plyr_c->IsInThirdPerson()) {
        thirdPersonState->savedZoomOffset = thirdPersonState->currentZoomOffset;
        if (settings->os[0].second) {
            listen_gradual_zoom = true;
            thirdPersonState->targetZoomOffset = -0.2f;
            return;
        }
        plyr_c->ForceFirstPerson();
    }
};

bool PlayerIsInToggledCam() {
    auto plyr_c = RE::PlayerCamera::GetSingleton();
    if (settings->main[0].second && plyr_c->IsInFirstPerson()) return true;
    else if (!settings->main[0].second && plyr_c->IsInThirdPerson()) return true;
	else return false;
}


// COMBAT TRIGGER STUFF
uint32_t oldstate_c = 0;
uint32_t GetCombatState() { return RE::PlayerCharacter::GetSingleton()->IsInCombat(); }
uint32_t oldstate_w = 0;

uint32_t CamSwitchHandling(uint32_t newstate) {
    // Toggle i call lamali miyiz ona bakiyoruz
    if (newstate) {
        if (PlayerIsInToggledCam()) {
            return 0;
        }
    } else {
        if (!PlayerIsInToggledCam()) {
            return 0;
        }
        else if (!settings->os[1].second) {
			return 0;
		}
    } 
    return 1;
}

bool bow_cam_switched = false;
class OnActorUpdate {
public:
    static void Install() {
        REL::Relocation<std::uintptr_t> hook1{REL::RelocationID(36357, 37348)};  // 84AB90, 876700

        auto& trampoline = SKSE::GetTrampoline();
        _Update = trampoline.write_call<5>(hook1.address() + REL::Relocate(0x6D3, 0x674),
                                           Update);  // 84AD36, 8768A6
        logger::info("Hook installed");
    }

private:
    static void Update(RE::Actor* a_actor, float a_zPos, RE::TESObjectCELL* a_cell);

    static inline REL::Relocation<decltype(Update)> _Update;
};
void OnActorUpdate::Update(RE::Actor* a_actor, float a_zPos, RE::TESObjectCELL* a_cell) {

    if (!a_actor) return _Update(a_actor, a_zPos, a_cell);
    if (RE::PlayerCharacter::GetSingleton()->GetFormID()!=a_actor->GetFormID()) return _Update(a_actor, a_zPos, a_cell);
    auto plyr_c = RE::PlayerCamera::GetSingleton();
    if (!plyr_c->IsInFirstPerson() && !plyr_c->IsInThirdPerson()) return _Update(a_actor, a_zPos, a_cell);
    uint32_t shouldToggle = 0;

    // killmove handling
    if (a_actor->IsInKillMove()) {
        oldstate_c = 1;
        return _Update(a_actor, a_zPos, a_cell);
    }
    else if (RE::PlayerCamera::GetSingleton()->IsInBleedoutMode()) {
		return _Update(a_actor, a_zPos, a_cell);
    } 

    // weapon draw handling
    if (settings->main[2].second) {
        auto weapon_state = static_cast<uint32_t>(a_actor->AsActorState()->GetWeaponState());
        if ((!weapon_state || weapon_state == 3) && oldstate_w != weapon_state) {
            oldstate_w = weapon_state;
            shouldToggle += CamSwitchHandling(oldstate_w);
        }
    }


    // combat handling
    if (settings->main[1].second && GetCombatState() != oldstate_c) {
        oldstate_c = !oldstate_c;
        shouldToggle += CamSwitchHandling(oldstate_c);
    }

    // bow first person aiming handling
    if (settings->main[3].second) {
        auto attack_state = static_cast<uint32_t>(a_actor->AsActorState()->GetAttackState());
        if (attack_state == 8 && RE::PlayerCamera::GetSingleton()->IsInThirdPerson()) {
            ToggleCam();
            shouldToggle = 0;
            bow_cam_switched = true;
        } else if (bow_cam_switched && (!attack_state || attack_state == 13) &&
                   RE::PlayerCamera::GetSingleton()->IsInFirstPerson() &&
                   settings->os[1].second) {
            ToggleCam(0.2f);
			shouldToggle = 0;
            bow_cam_switched = false;
		}
    }

    if (shouldToggle) ToggleCam();

    return _Update(a_actor, a_zPos, a_cell);
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {

    SetupLog();
    SKSE::Init(skse);
    logger::info("Plugin loaded.");
    
    // Settings
    auto loaded = Settings::LoadSettings();
    
    if (loaded) logger::info("Settings loaded.");
    else logger::info("Could not load settings.");
    assert(loaded && "Could not load settings from ini file!");
    
    settings = Settings::Settings::GetSingleton();

    // Hooks
    auto& trampoline = SKSE::GetTrampoline();
    bool hook1 = settings->main[1].second || settings->main[2].second || settings->main[3].second;
    bool hook2 = settings->os[0].second;
    // Bunu hook ekledikce update et
    if (hook1 + hook2)
        trampoline.create(14 * (hook1 + hook2));
    // IsInCombat
    if (hook1) {
		logger::info("Toggle in combat enabled. Hooking...");
		OnActorUpdate::Install();
	}
    // Gradual zoom
    if (hook2) {
        logger::info("Gradual zoom enabled. Hooking...");
        OnCameraUpdate::Install();
    }
    return true;
}