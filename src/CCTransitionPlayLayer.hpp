#pragma once

class CCTransitionPlayLayer : public cocos2d::CCTransitionFade {
public:
    virtual void onEnter();
    virtual void onExit();

    void finish();
};
