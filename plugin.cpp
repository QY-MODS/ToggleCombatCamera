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
        //trampoline.create(14);
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
            logger::info("Forcing 1st after gradual");
            RE::PlayerCamera::GetSingleton()->ForceFirstPerson();
            logger::info("listen_gradual_zoom = false");
        }
    }
}

void ToggleCam() {
    //isineffect = !isineffect;
    listen_gradual_zoom = false;
    logger::info("listen_gradual_zoom = false,ToggleDialogueCam");
    auto plyr_c = RE::PlayerCamera::GetSingleton();
    auto thirdPersonState =
        static_cast<RE::ThirdPersonState*>(plyr_c->cameraStates[RE::CameraState::kThirdPerson].get());
    if (plyr_c->IsInFirstPerson()) {
        plyr_c->ForceThirdPerson();
        thirdPersonState->targetZoomOffset = thirdPersonState->savedZoomOffset;
    } else if (plyr_c->IsInThirdPerson()) {
        thirdPersonState->savedZoomOffset = thirdPersonState->currentZoomOffset;
        if (settings->os[0].second) {
            listen_gradual_zoom = true;
            logger::info("listen_gradual_zoom = true,ToggleDialogueCam");
            thirdPersonState->targetZoomOffset = -0.2f;
            logger::info("Player is in 3rd person, gradually zooming in.");
            return;
        }
        plyr_c->ForceFirstPerson();
    } else {
        logger::info("Player is in neither 1st nor 3rd person. Also not killcam.");
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
uint32_t GetWeaponState() { return RE::PlayerCharacter::GetSingleton()->AsActorState()->IsWeaponDrawn(); }

uint32_t CamSwitchHandling(uint32_t newstate) {
    if (newstate) {
        logger::info("newstate 1");
        if (PlayerIsInToggledCam()) {
            logger::info("Player is already in toggled cam");
            return 0;
        }
    } else {
        logger::info("newstate 0");
        if (!PlayerIsInToggledCam()) {
            logger::info("Player is already in untoggled cam");
            return 0;
        }
    } 
    return 1;
}


class OnActorUpdate {
public:
    static void Install() {
        REL::Relocation<std::uintptr_t> hook1{REL::RelocationID(36357, 37348)};  // 84AB90, 876700

        auto& trampoline = SKSE::GetTrampoline();
        //trampoline.create(14);
        _Update = trampoline.write_call<5>(hook1.address() + REL::Relocate(0x6D3, 0x674),
                                           Update);  // 84AD36, 8768A6
        logger::info("Hook installed");
    }

private:
    static void Update(RE::Actor* a_actor, float a_zPos, RE::TESObjectCELL* a_cell);

    static inline REL::Relocation<decltype(Update)> _Update;
};
void OnActorUpdate::Update(RE::Actor* a_actor, float a_zPos, RE::TESObjectCELL* a_cell) {
    //logger::info("Hook works");

    if (!a_actor) return _Update(a_actor, a_zPos, a_cell);
    if (RE::PlayerCharacter::GetSingleton()->GetFormID()!=a_actor->GetFormID()) return _Update(a_actor, a_zPos, a_cell);
    uint32_t shouldToggle = 0;

    // killmove handling
    if (a_actor->IsInKillMove()) {
        logger::info("Player is in killmove");
        oldstate_c = 1;
        return _Update(a_actor, a_zPos, a_cell);
    }
    else if (RE::PlayerCamera::GetSingleton()->IsInBleedoutMode()) {
		logger::info("Player is in bleedout");
		return _Update(a_actor, a_zPos, a_cell);
    } 

    // weapon draw handling
    if (settings->main[2].second && GetWeaponState() != oldstate_w) {
        oldstate_w = !oldstate_w;
        shouldToggle += CamSwitchHandling(oldstate_w);
    }


    // combat handling
    if (settings->main[1].second && GetCombatState() != oldstate_c) {
        oldstate_c = !oldstate_c;
        shouldToggle += CamSwitchHandling(oldstate_c);
    }

    if (shouldToggle) ToggleCam();

    return _Update(a_actor, a_zPos, a_cell);
}



class OurEventSink : public RE::BSTEventSink<RE::TESCombatEvent> {
    OurEventSink() = default;
    OurEventSink(const OurEventSink&) = delete;
    OurEventSink(OurEventSink&&) = delete;
    OurEventSink& operator=(const OurEventSink&) = delete;
    OurEventSink& operator=(OurEventSink&&) = delete;

public:
    static OurEventSink* GetSingleton() {
        static OurEventSink singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* event,
                                          RE::BSTEventSource<RE::TESCombatEvent>*) {
        //if (!settings->main[1].second) return RE::BSEventNotifyControl::kContinue;
        if (!event) return RE::BSEventNotifyControl::kContinue;
        
        if (event->newState == RE::ACTOR_COMBAT_STATE::kCombat) {
            logger::info("Combat state: Combat");
        }
        else if (event->newState == RE::ACTOR_COMBAT_STATE::kSearching) {
            logger::info("Combat state: Searching");
		}
        else {
            logger::info("Combat state: None");
		}

        //if (event->targetActor.get()->GetFormID() != player->GetFormID()) return RE::BSEventNotifyControl::kContinue;
        //if (event->newState != RE::ACTOR_COMBAT_STATE::kCombat) return RE::BSEventNotifyControl::kContinue;
        //logger::info("Combat event received"); 
        //if (player->AsActorState()->IsWeaponDrawn()){
        //    auto weapon_form = player->GetAttackingWeapon()->GetOwner();
        //    auto weapon = RE::TESForm::LookupByID<RE::TESObjectWEAP>(weapon_form);
        //}
        //logger::info("Player is in combat");
		//ToggleCam();
        
        return RE::BSEventNotifyControl::kContinue;
    }
    
};

class OurEventSink2 : public RE::BSTEventSink<RE::TESHitEvent> {
    OurEventSink2() = default;
    OurEventSink2(const OurEventSink2&) = delete;
    OurEventSink2(OurEventSink2&&) = delete;
    OurEventSink2& operator=(const OurEventSink2&) = delete;
    OurEventSink2& operator=(OurEventSink2&&) = delete;

public:
    static OurEventSink2* GetSingleton() {
        static OurEventSink2 singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* event, RE::BSTEventSource<RE::TESHitEvent>*) {
        if (!event) return RE::BSEventNotifyControl::kContinue;
        auto player = RE::PlayerCharacter::GetSingleton();
        logger::info("Hit event received");
        logger::info("player got hit {}", event->target.get()->formID == player->formID);
        return RE::BSEventNotifyControl::kContinue;
    }
};

void OnMessage(SKSE::MessagingInterface::Message* message) {
    switch (message->type) {
        case SKSE::MessagingInterface::kPostPostLoad:
            //RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESCombatEvent>(OurEventSink::GetSingleton());
            //RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESHitEvent>(OurEventSink2::GetSingleton());
            break;
    }
};
//
//void SaveCallback(SKSE::SerializationInterface* serializationInterface) {
//    logger::info("Saving data to skse co-save.");
//    logger::info("oldsate: {}", oldstate_c);
//    logger::info("isineffect: {}", isineffect);
//
//    serializationInterface->WriteRecordData(oldstate_c);
//    serializationInterface->WriteRecordData(isineffect);
//}
//
//void LoadCallback(SKSE::SerializationInterface* serializationInterface) {
//    std::uint32_t type;
//    std::uint32_t version;
//    std::uint32_t length;
//
//    logger::info("Loading data from skse co-save.");
//
//    while (serializationInterface->GetNextRecordInfo(type, version, length)) {
//        logger::info("odifahpsdi");
//        auto temp = Utilities::DecodeTypeCode(type);
//
//        if (version != Settings::kSerializationVersion) {
//            logger::info("Loaded data has incorrect version. Recieved ({}) - Expected ({}) for Data Key ({})",
//                             version, Settings::kSerializationVersion, temp);
//            continue;
//        }
//        switch (type) {
//            case Settings::kDataKey: {
//                logger::info("Hello");
//                serializationInterface->ReadRecordData(oldstate_c);
//                serializationInterface->ReadRecordData(isineffect);
//                logger::info("oldsate: {}", oldstate_c);
//                logger::info("isineffect: {}", isineffect);
//            } break;
//            default:
//                logger::info("Unrecognized Record Type: {}", temp);
//                break;
//        }
//    }
//    logger::info("Data loaded from skse co-save.");
//}
//
//void InitializeSerialization() {
//    auto* serialization = SKSE::GetSerializationInterface();
//    serialization->SetUniqueID(Settings::kDataKey);
//    serialization->SetSaveCallback(SaveCallback);
//    serialization->SetLoadCallback(LoadCallback);
//    SKSE::log::trace("Cosave serialization initialized.");
//};

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
    
    // SKSE
    //InitializeSerialization();
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    
    // Hooks
    auto& trampoline = SKSE::GetTrampoline();
    // Bunu hook ekledikce update et
    if (settings->main[1].second) trampoline.create(14 * (settings->main[1].second));
    // IsInCombat
    if (settings->main[1].second) {
		logger::info("Toggle in combat enabled. Hooking...");
		OnActorUpdate::Install();
	}
    // Gradual zoom
    if (settings->os[0].second) {
        logger::info("Gradual zoom enabled. Hooking...");
        OnCameraUpdate::Install();
    }
    return true;
}