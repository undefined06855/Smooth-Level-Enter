#pragma once

// this class should have zero members, since all that happens is the vtable gets replaced with this one
// if we need members use the bytes in m_tColor or something
class CCTransitionPlayLayer : public cocos2d::CCTransitionFade {
public:
    virtual void onEnter() override;
    virtual void onExit() override;
    virtual void draw() override;
    virtual void update(float dt) override;

    void animateOutScene();
};
