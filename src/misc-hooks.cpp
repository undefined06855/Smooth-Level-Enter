#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelAreaInnerLayer.hpp>
#include <Geode/modify/CCDelayTime.hpp>
#include <Geode/modify/CCFadeIn.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>

// lol i love this file

geode::Hook* g_noCCDelayHook;
geode::Hook* g_noCCFadeInHook;
geode::Hook* g_noFMODAudioFadeHook;

$on_mod(Loaded) {
    g_noCCDelayHook = nullptr;
    g_noCCFadeInHook = nullptr;
    g_noFMODAudioFadeHook = nullptr;

    for (auto hook : geode::Mod::get()->getHooks()) {
        if (hook->getDisplayName() == "cocos2d::CCDelayTime::create") g_noCCDelayHook = hook;
        if (hook->getDisplayName() == "cocos2d::CCFadeIn::create") g_noCCFadeInHook = hook;
        if (hook->getDisplayName() == "FMODAudioEngine::fadeOutMusic") g_noFMODAudioFadeHook = hook;
    }

    if (g_noCCDelayHook) (void)g_noCCDelayHook->disable();
    if (g_noCCFadeInHook) (void)g_noCCFadeInHook->disable();
    if (g_noFMODAudioFadeHook) (void)g_noFMODAudioFadeHook->disable();
}

class $modify(cocos2d::CCDelayTime) {
    static cocos2d::CCDelayTime* create(float delay) {
        return cocos2d::CCDelayTime::create(0.f);
    }
};

class $modify(cocos2d::CCFadeIn) {
    static cocos2d::CCFadeIn* create(float time) {
        return cocos2d::CCFadeIn::create(999999.f);
    }
};

class $modify(FMODAudioEngine) {
    void fadeOutMusic(float duration, int channel) {
        FMODAudioEngine::fadeOutMusic(0.f, channel);
    }
};

// shorten the delay when entering playlayer
class $modify(PlayLayer) {
    void setupHasCompleted() {
        if (g_noCCDelayHook) (void)g_noCCDelayHook->enable();
        PlayLayer::setupHasCompleted();
        if (g_noCCDelayHook) (void)g_noCCDelayHook->disable();
    }
};

// remove the weird double fade
class $modify(LevelAreaInnerLayer) {
    void onDoor(cocos2d::CCObject* sender) {
        if (g_noCCDelayHook) (void)g_noCCDelayHook->enable();
        if (g_noCCFadeInHook) (void)g_noCCFadeInHook->enable();
        if (g_noFMODAudioFadeHook) (void)g_noFMODAudioFadeHook->enable(); // prevent some crashes
        LevelAreaInnerLayer::onDoor(sender);
        if (g_noCCDelayHook) (void)g_noCCDelayHook->disable();
        if (g_noCCFadeInHook) (void)g_noCCFadeInHook->disable();
        if (g_noFMODAudioFadeHook) (void)g_noFMODAudioFadeHook->disable();
    }
};
