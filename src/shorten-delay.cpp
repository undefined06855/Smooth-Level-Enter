// better but scary byte patch version
// #ifdef GEODE_IS_WINDOWS

// $on_mod(Loaded) {
//     geode::log::info("patching delay to be shorter (windows only)");

//     // write 0.2 to the first empty place in .rdata i found
//     (void)geode::Mod::get()->patch(
//         (void*)(geode::base::get() + 0x515020),
//         { 0xCD, 0xCC, 0x4C, 0x3E } // 0.2
//     );

//     // use that instead of DAT_140622c24 for the delay passed into ccdelaytime
//     (void)geode::Mod::get()->patch(
//         (void*)(geode::base::get() + 0x3a6b00),
//         { 0xF3, 0x44, 0x0F, 0x10, 0x05, 0x17, 0xE5, 0x16, 0x00 }
//     );
// }

// #endif

#include <Geode/modify/PlayLayer.hpp>

geode::Hook* g_hook;

cocos2d::CCDelayTime* CCDelayTime_create(float delay) {
    return cocos2d::CCDelayTime::create(0.f);
}

$on_mod(Loaded) {
    auto res = geode::Mod::get()->hook(
        (void*)geode::addresser::getNonVirtual(&cocos2d::CCDelayTime::create),
        &CCDelayTime_create,
        "cocos2d::CCDelayTime::create"
    );

    if (!res) {
        geode::log::warn("Could not hook CCDelayTime::create!");
        return;
    }

    g_hook = res.unwrap();
    (void)g_hook->disable();
}

class $modify(PlayLayer) {
    void setupHasCompleted() {
        (void)g_hook->enable();
        PlayLayer::setupHasCompleted();
        (void)g_hook->disable();
    }
};
