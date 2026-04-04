#include "CCTransitionPlayLayer.hpp"

void setFakeIsRunningRecursive(cocos2d::CCNode* node, bool isRunning) {
    node->m_bRunning = isRunning;
    for (auto child : node->getChildrenExt()) {
        setFakeIsRunningRecursive(child, isRunning);
    }
}

// let me typeinfo_cast to mh status::Manager
namespace status { class Manager : public cocos2d::CCNode {}; }

void CCTransitionPlayLayer::onEnter() {
    CCScene::onEnter();

    cocos2d::CCTouchDispatcher::get()->setDispatchEvents(false);
    m_pOutScene->onExitTransitionDidStart();
    m_pInScene->resumeSchedulerAndActions(); // call instead of onEnter
    setFakeIsRunningRecursive(m_pInScene, true);

    m_fDuration = 1.f;

    auto playLayer = m_pInScene->getChildByType<PlayLayer>(0);
    playLayer->setVisible(true);

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

    playLayer->m_attemptLabel->setOpacity(0);
    playLayer->m_attemptLabel->runAction(cocos2d::CCFadeIn::create(m_fDuration));

    playLayer->m_uiLayer->setScale(0.f);
    playLayer->m_uiLayer->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(m_fDuration, 1.f)));

    auto mh = playLayer->getChildByType<status::Manager>(0);
    if (mh) {
        mh->setAnchorPoint({ .5f, .5f });
        mh->setPosition(cocos2d::CCDirector::get()->getWinSize() / 2.f);
        mh->setScale(0.f);
        mh->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(m_fDuration, 1.f)));
    }

    playLayer->m_progressBar->setPositionY(playLayer->m_progressBar->getPositionY() + 30.f);
    playLayer->m_progressBar->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(m_fDuration, { 0.f, -30.f })));

    playLayer->m_percentageLabel->setPositionY(playLayer->m_percentageLabel->getPositionY() + 30.f);
    playLayer->m_percentageLabel->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(m_fDuration, { 0.f, -30.f })));

    playLayer->m_infoLabel->setPositionX(playLayer->m_infoLabel->getPositionX() - 50.f);
    playLayer->m_infoLabel->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(m_fDuration, { 50.f, 0.f })));

    // objects
    auto screenWidth = cocos2d::CCDirector::get()->getWinSize().width;

    for (int i = 0; i < playLayer->m_activeObjectsCount; i++) {
        auto obj = playLayer->m_activeObjects[i];
        if (!obj->getParent()) continue;

        obj->setPosition(obj->getPosition() + cocos2d::CCPoint{ 100.f, -300.f });
        GLubyte origOpacity = obj->getOpacity();
        obj->setOpacity(0);

        auto percentage = obj->getParent()->convertToWorldSpace(obj->getPosition()).x / screenWidth;

        obj->runAction(cocos2d::CCSequence::createWithTwoActions(
            cocos2d::CCDelayTime::create(percentage * .5f * m_fDuration),
            cocos2d::CCSpawn::create(
                cocos2d::CCEaseBounceOut::create(cocos2d::CCMoveBy::create(.5f * m_fDuration, { -100.f, 0.f })),
                cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(.5f * m_fDuration, { 0.f, 300.f })),
                cocos2d::CCFadeTo::create(.5f * m_fDuration, origOpacity),
                nullptr
            )
        ));
    }

    this->runAction(cocos2d::CCSequence::createWithTwoActions(
        cocos2d::CCDelayTime::create(m_fDuration),

        // cleanup
        geode::cocos::CallFuncExt::create([this, rewindBackground] {
            setFakeIsRunningRecursive(m_pInScene, false);

            CCTransitionScene::finish();
            if (rewindBackground) rewindBackground->setVisible(true);
        })
    ));
}

void CCTransitionPlayLayer::onExit() {
    CCScene::onExit();

    cocos2d::CCTouchDispatcher::get()->setDispatchEvents(true);

    m_pOutScene->onExit();
    m_pInScene->onEnter();
    m_pInScene->onEnterTransitionDidFinish();
}
