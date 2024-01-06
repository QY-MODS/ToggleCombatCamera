#pragma once

#include "Settings.h"
#include "Utils.h"

//
//float seconds2wait = 1.5f;
//
//void UpdateLoop(){
//     if (seconds2wait > 0) {
//        seconds2wait -= 0.5f;
//        return;
//    } else {
//        seconds2wait = 1.5f; 
//        Manager::GetSingleton()->ToggleCam();
//    }
//};
//
//class Manager : public Utilities::Ticker {
//
//    void Init(){};
//
//public:
//    Manager(std::chrono::milliseconds interval) : Utilities::Ticker(std::function<void()>(UpdateLoop), interval) {
//        Init();
//    }
//
//    static Manager* GetSingleton() {
//        static Manager singleton(std::chrono::milliseconds(500));
//        return &singleton;
//    }
//
//    
//    void ToggleCam() {
//         if (!player->IsInCombat()) return;
//        /*listen_gradual_zoom = false;
//        logger::info("listen_gradual_zoom = false,ToggleDialogueCam");
//        auto plyr_c = RE::PlayerCamera::GetSingleton();
//        auto thirdPersonState =
//            static_cast<RE::ThirdPersonState*>(plyr_c->cameraStates[RE::CameraState::kThirdPerson].get());
//        if (plyr_c->IsInFirstPerson()) {
//            plyr_c->ForceThirdPerson();
//            thirdPersonState->targetZoomOffset = thirdPersonState->savedZoomOffset;
//        } else if (plyr_c->IsInThirdPerson()) {
//            thirdPersonState->savedZoomOffset = thirdPersonState->currentZoomOffset;
//            if (settings->os[0].second) {
//                listen_gradual_zoom = true;
//                logger::info("listen_gradual_zoom = true,ToggleDialogueCam");
//                thirdPersonState->targetZoomOffset = -0.2f;
//                logger::info("Player is in 3rd person, gradually zooming in.");
//                return;
//            }
//            plyr_c->ForceFirstPerson();
//        } else {
//            logger::info("Player is in neither 1st nor 3rd person.");
//        }*/
//    };
//
//};












//
//class Manager : public Utilities::Ticker {
//public:
//    Manager(std::chrono::milliseconds interval) : Utilities::Ticker(std::function<void()>(UpdateLoop), interval) {}
//
//    static Manager* GetSingleton(std::chrono::milliseconds interval) {
//        static Manager singleton(interval);
//        return &singleton;
//    }
//};

//class Manager : public Utilities::Ticker {
//public:
//    Manager(std::chrono::milliseconds interval) : Utilities::Ticker(std::function<void()>(UpdateLoop), interval) {}
//
//    static Manager* GetSingleton(int intervalMilliseconds) {
//        static Manager singleton(std::chrono::milliseconds(intervalMilliseconds));
//        return &singleton;
//    }
//};
