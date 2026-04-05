#pragma once

// this class should have zero members, since all that happens is the vtable gets replaced with this one
class CCTransitionPlayLayer : public cocos2d::CCTransitionFade {
public:
    virtual void onEnter() override;
    virtual void onExit() override;
};
