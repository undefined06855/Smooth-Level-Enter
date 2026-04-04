#pragma once
#include <Geode/modify/CCDirector.hpp>

class $modify(HookedCCDirector, cocos2d::CCDirector) {
    static void onModify(auto& self);

    bool replaceScene(cocos2d::CCScene* scene);
};
