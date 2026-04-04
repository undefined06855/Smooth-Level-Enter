#include "CCDirector.hpp"
#include "CCTransitionPlayLayer.hpp"

void HookedCCDirector::onModify(auto& self) {
    (void)self.setHookPriorityPost("cocos2d::CCDirector::replaceScene", geode::Priority::Replace);
}

bool HookedCCDirector::replaceScene(cocos2d::CCScene* scene) {
    auto cast = geode::cast::typeinfo_cast<cocos2d::CCTransitionFade*>(scene);
    if (!cast) {
        return CCDirector::replaceScene(scene);
    }

    if (!cast->m_pInScene->getChildByType<PlayLayer>(0)) {
        return CCDirector::replaceScene(scene);
    }

    static void* vtable = []() -> void* {
        CCTransitionPlayLayer temp;
        // dtor releases both of these
        temp.m_pInScene = cocos2d::CCScene::create();
        temp.m_pOutScene = cocos2d::CCScene::create();
        return *(void**)&temp;
    }();

    // swap the vtables
    *(void**)scene = vtable;

    return CCDirector::replaceScene(scene);
}
