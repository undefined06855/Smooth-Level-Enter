#include "CCTransitionPlayLayer.hpp"

// forces actions to run even though they're technically not the active scene
// "running" is cocos speak for currently active and visible
void setFakeIsRunningRecursive(cocos2d::CCNode* node, bool isRunning) {
    node->m_bRunning = isRunning;
    for (auto child : node->getChildrenExt()) {
        setFakeIsRunningRecursive(child, isRunning);
    }
}

// let me typeinfo_cast to megahack status::Manager
namespace status { class Manager : public cocos2d::CCNode {}; }

void CCTransitionPlayLayer::onEnter() {
    CCScene::onEnter();

    if (geode::Mod::get()->getSettingValue<bool>("override-length")) {
        m_fDuration = 1.f;
    }

    cocos2d::CCTouchDispatcher::get()->setDispatchEvents(false);
    m_pOutScene->onExitTransitionDidStart();
    m_pInScene->resumeSchedulerAndActions(); // call instead of onEnter
    setFakeIsRunningRecursive(m_pInScene, true);

    auto playLayer = m_pInScene->getChildByType<PlayLayer>(0);
    playLayer->setVisible(true);

    bool isPlayer2Running = playLayer->m_player2->isRunning();
    setFakeIsRunningRecursive(playLayer->m_player2, true);

    auto rewindBackground = playLayer->getChildByID("undefined0.rewind/bg-gradient");
    if (rewindBackground) rewindBackground->setVisible(false);

    // static animations
    playLayer->m_background->setBlendFunc({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });
    playLayer->m_background->setOpacity(0);
    playLayer->m_background->runAction(cocos2d::CCSequence::createWithTwoActions(
        cocos2d::CCDelayTime::create(m_fDuration * .5f),
        cocos2d::CCFadeIn::create(m_fDuration * .5f)
    ));

    playLayer->m_groundLayer->setPositionY(playLayer->m_groundLayer->getPositionY() - 120.f);
    playLayer->m_groundLayer->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(m_fDuration, { 0.f, 120.f })));

    playLayer->m_groundLayer2->setPositionY(playLayer->m_groundLayer2->getPositionY() + 120.f);
    playLayer->m_groundLayer2->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(m_fDuration, { 0.f, -120.f })));

    if (playLayer->m_middleground) {
        auto mgSprites = cocos2d::CCArray::create();
        mgSprites->addObjectsFromArray(playLayer->m_middleground->m_mg1BatchNode->getChildren());
        mgSprites->addObjectsFromArray(playLayer->m_middleground->m_mg2BatchNode->getChildren());
        for (auto sprite : geode::cocos::CCArrayExt<cocos2d::CCSprite>(mgSprites)) {
            sprite->setOpacity(0);
            sprite->setPositionX(sprite->getPositionX() + 500.f);
            sprite->runAction(cocos2d::CCFadeIn::create(m_fDuration));
            sprite->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(m_fDuration, { -500.f, 0.f })));
        }
    }

    playLayer->m_attemptLabel->setOpacity(0);
    playLayer->m_attemptLabel->runAction(cocos2d::CCFadeIn::create(m_fDuration));

    playLayer->m_uiLayer->setScale(0.f);
    playLayer->m_uiLayer->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(m_fDuration, 1.f)));

    // mh ui isnt in m_uiLayer for whatever reason
    // make sure to copy the animations used for m_uiLayer
    auto megahackUI = playLayer->getChildByType<status::Manager>(0);
    if (megahackUI) {
        megahackUI->setAnchorPoint({ .5f, .5f });
        megahackUI->setPosition(cocos2d::CCDirector::get()->getWinSize() / 2.f);
        megahackUI->setScale(0.f);
        megahackUI->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(m_fDuration, 1.f)));
    }

    playLayer->m_progressBar->setPositionY(playLayer->m_progressBar->getPositionY() + 30.f);
    playLayer->m_progressBar->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(m_fDuration, { 0.f, -30.f })));

    playLayer->m_percentageLabel->setPositionY(playLayer->m_percentageLabel->getPositionY() + 30.f);
    playLayer->m_percentageLabel->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(m_fDuration, { 0.f, -30.f })));

    playLayer->m_infoLabel->setPositionX(playLayer->m_infoLabel->getPositionX() - 50.f);
    playLayer->m_infoLabel->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(m_fDuration, { 50.f, 0.f })));

    // likely not onscreen but should animate anyway
    auto origP1Scale = playLayer->m_player1->getScale(); // may not be 1 if the player starts as mini
    playLayer->m_player1->setScale(0.f);
    playLayer->m_player1->setRotation(-20.f);
    playLayer->m_player1->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(m_fDuration, origP1Scale)));
    playLayer->m_player1->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCRotateTo::create(m_fDuration, 0.f)));

    auto origP2Scale = playLayer->m_player2->getScale();
    playLayer->m_player2->setScale(0.f);
    playLayer->m_player2->setRotation(20.f);
    playLayer->m_player2->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(m_fDuration, origP2Scale)));
    playLayer->m_player2->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCRotateTo::create(m_fDuration, 0.f)));

    // objects
    auto screenWidth = cocos2d::CCDirector::get()->getWinSize().width;

    for (int i = 0; i < playLayer->m_activeObjectsCount; i++) {
        auto obj = playLayer->m_activeObjects[i];
        if (!obj->getParent()) continue;

        obj->setPosition(obj->getPosition() + cocos2d::CCPoint{ 100.f, -300.f });

        auto origOpacity = obj->getOpacity();
        obj->setOpacity(0);

        obj->setRotation(obj->getRotation() + 50.f);

        auto origScaleX = obj->getScaleX();
        auto origScaleY = obj->getScaleY();
        obj->setScale(0.f);

        auto percentage = obj->getParent()->convertToWorldSpace(obj->getPosition()).x / screenWidth;

        // half the time is spent animating across the screen, and each object takes half the time to animate to its final pos
        obj->runAction(cocos2d::CCSequence::createWithTwoActions(
            cocos2d::CCDelayTime::create(percentage * .5f * m_fDuration),
            cocos2d::CCSpawn::create(
                cocos2d::CCEaseBounceOut::create(cocos2d::CCMoveBy::create(.5f * m_fDuration, { -100.f, 0.f })),
                cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(.5f * m_fDuration, { 0.f, 300.f })),
                cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(.5f * m_fDuration, origScaleX, origScaleY)),
                cocos2d::CCEaseExponentialOut::create(cocos2d::CCRotateBy::create(.5f * m_fDuration, -50.f)),
                cocos2d::CCFadeTo::create(.5f * m_fDuration, origOpacity),
                nullptr
            )
        ));
    }

    // gradients
    for (auto [i, gradient] : geode::cocos::CCDictionaryExt<int, GJGradientLayer>(playLayer->m_gradientLayers)) {
        auto origOpacity = gradient->getOpacity();
        gradient->setOpacity(0);
        gradient->runAction(cocos2d::CCFadeTo::create(m_fDuration, origOpacity));
    }

    this->runAction(cocos2d::CCSequence::createWithTwoActions(
        cocos2d::CCDelayTime::create(m_fDuration),

        // cleanup
        geode::cocos::CallFuncExt::create([this, rewindBackground, playLayer, isPlayer2Running] {
            setFakeIsRunningRecursive(m_pInScene, false);

            CCTransitionScene::finish();
            if (rewindBackground) rewindBackground->setVisible(true);
            if (!isPlayer2Running) setFakeIsRunningRecursive(playLayer->m_player2, false);
        })
    ));

    playLayer->updateShaderLayer(1.f / 60.f);

    this->scheduleUpdate();
}

void CCTransitionPlayLayer::onExit() {
    CCScene::onExit();

    cocos2d::CCTouchDispatcher::get()->setDispatchEvents(true);

    m_pInScene->onEnter();

    cocos2d::CCDirector::get()->willSwitchToScene(m_pInScene);

    m_pOutScene->onExit();
    m_pInScene->onEnterTransitionDidFinish();
}
