#include "CCTransitionPlayLayer.hpp"

// forces actions to run even though they're technically not the active scene
// "running" is cocos speak for currently active and visible
void setFakeIsRunningRecursive(cocos2d::CCNode* node, bool isRunning) {
    node->m_bRunning = isRunning;
    for (auto child : node->getChildrenExt()) {
        setFakeIsRunningRecursive(child, isRunning);
    }
}

namespace status { class Manager : public cocos2d::CCNode {}; } // megahack status
class LevelLayer : public cocos2d::CCNode {}; // gdguesser level page
class ModSettingsPopup : public cocos2d::CCNode {}; // geode settings popup

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

    // m_pInScene animations below (playlayer)
    auto winSize = cocos2d::CCDirector::get()->getWinSize();

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
        megahackUI->setPosition(winSize / 2.f);
        megahackUI->setScale(0.f);
        megahackUI->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(m_fDuration, 1.f)));
    }

    // fix positioning because right now it's at 0, 0 with content size 0, 0
    // though this gets reset after the transition finishes
    playLayer->m_uiTriggerUI->setAnchorPoint({ .5f, .5f });
    playLayer->m_uiTriggerUI->setPosition(winSize / 2.f);
    playLayer->m_uiTriggerUI->setContentSize(winSize);
    playLayer->m_uiTriggerUI->setScale(0.f);
    playLayer->m_uiTriggerUI->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(m_fDuration, 1.f)));

    playLayer->m_progressBar->setPositionY(playLayer->m_progressBar->getPositionY() + 30.f);
    playLayer->m_progressBar->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(m_fDuration, { 0.f, -30.f })));

    playLayer->m_percentageLabel->setPositionY(playLayer->m_percentageLabel->getPositionY() + 30.f);
    playLayer->m_percentageLabel->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(m_fDuration, { 0.f, -30.f })));

    playLayer->m_infoLabel->setPositionX(playLayer->m_infoLabel->getPositionX() - 50.f);
    playLayer->m_infoLabel->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(m_fDuration, { 50.f, 0.f })));

    playLayer->m_shaderLayer->m_sprite->setOpacity(0);
    playLayer->m_shaderLayer->m_sprite->runAction(cocos2d::CCFadeIn::create(m_fDuration));
    playLayer->m_shaderLayer->m_sprite->setBlendFunc({ GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA }); // see CCTransitionPlayLayer::draw

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
    bool useLegacyEase = geode::Mod::get()->getSettingValue<bool>("legacy-easing");

    for (int i = 0; i < playLayer->m_activeObjectsCount; i++) {
        auto obj = playLayer->m_activeObjects[i];
        if (!obj || !obj->getParent()) continue;

        obj->setPosition(obj->getPosition() + cocos2d::CCPoint{ 50.f, -100.f });

        auto origOpacity = obj->getOpacity();
        obj->setOpacity(0);

        auto origRotationX = obj->getRotationX();
        auto origRotationY = obj->getRotationY();
        obj->setRotationX(origRotationX + 50.f);
        obj->setRotationY(origRotationY + 50.f);

        auto origScaleX = obj->getScaleX();
        auto origScaleY = obj->getScaleY();
        obj->setScaleX(0.f);
        obj->setScaleY(0.f);

        auto percentage = obj->getParent()->convertToWorldSpace(obj->getPosition()).x / screenWidth;

        // use legacy ccbounceout if set, else use just exponential out
        auto createBlockXEase = [useLegacyEase](cocos2d::CCActionInterval* action) -> cocos2d::CCActionInterval* {
            if (useLegacyEase) return cocos2d::CCEaseBounceOut::create(action);
            else return cocos2d::CCEaseSineOut::create(action);
        };

        // half the time is spent animating across the screen, and each object takes half the time to animate to its final pos
        obj->runAction(cocos2d::CCSequence::createWithTwoActions(
            cocos2d::CCDelayTime::create(percentage * .5f * m_fDuration),
            cocos2d::CCSpawn::create(
                createBlockXEase(cocos2d::CCMoveBy::create(.5f * m_fDuration, { -50.f, 0.f })),
                cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(.5f * m_fDuration, { 0.f, 100.f })),
                cocos2d::CCEaseSineOut::create(cocos2d::CCScaleTo::create(.5f * m_fDuration, origScaleX, origScaleY)),
                cocos2d::CCEaseSineOut::create(cocos2d::CCRotateBy::create(.5f * m_fDuration, -50.f, -50.f)),
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

    // scary shaderlayer stuff
    // if background is included in shaderlayer then force levelinfolayer into it so lens circle works properly
    // this works surprisingly well as long as nobody else touches it
    bool isBackgroundIncluded = playLayer->m_shaderLayer->m_state.m_minBlendingLayer <= 1;
    if (isBackgroundIncluded) {
        playLayer->m_shaderLayer->m_pChildren->addObject(m_pOutScene);
        m_pOutScene->setScale(playLayer->m_shaderLayer->m_scaleFactor); // for pixelate shader
    }

    if (geode::Mod::get()->getSettingValue<bool>("animate-out-level-page")) {
        this->animateOutScene();
    }

    this->runAction(cocos2d::CCSequence::createWithTwoActions(
        cocos2d::CCDelayTime::create(m_fDuration),

        // cleanup
        geode::cocos::CallFuncExt::create([this, rewindBackground, playLayer, isPlayer2Running, isBackgroundIncluded] {
            setFakeIsRunningRecursive(m_pInScene, false);

            // playlayer kind of depends on ui trigger ui to be at 0, 0
            playLayer->m_uiTriggerUI->setPosition({ 0.f, 0.f });
            playLayer->m_uiTriggerUI->setContentSize({ 0.f, 0.f });

            CCTransitionScene::finish();
            if (rewindBackground) rewindBackground->setVisible(true);
            if (!isPlayer2Running) setFakeIsRunningRecursive(playLayer->m_player2, false);
            if (isBackgroundIncluded) playLayer->m_shaderLayer->m_pChildren->removeObject(m_pOutScene);
        })
    ));

    this->scheduleUpdate();
}

void CCTransitionPlayLayer::update(float dt) {
    auto playLayer = m_pInScene->getChildByType<PlayLayer>(0);
    playLayer->updateShaderLayer(dt);
}

void CCTransitionPlayLayer::draw() {
    auto playLayer = m_pInScene->getChildByType<PlayLayer>(0);

    // this is also for fixing shaderlayer (since the scale of shaderlayer children need to be set for the pixelate shader)
    // keep in mind we have two of the SAME out scenes drawing - one drawn first, then one put in shaderlayer
    // so we need to set the scale to 1 while drawing the BACK out scene, then set it to what it was for the FRONT out scene
    auto origScale = m_pOutScene->getScale();
    m_pOutScene->setScale(1.f);
    m_pOutScene->visit();
    m_pOutScene->setScale(origScale);

    // this is for specifically shaderlayer since the shader doesn't respect the opacity
    // shaderlayer->m_sprite has blend func set to GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA
    glBlendColor(.0f, .0f, .0f, playLayer->m_shaderLayer->m_sprite->getOpacity() / 255.f);
    m_pInScene->visit();
    glBlendColor(0.f, 0.f, 0.f, 1.f);
}

void CCTransitionPlayLayer::onExit() {
    CCScene::onExit();

    cocos2d::CCTouchDispatcher::get()->setDispatchEvents(true);

    m_pInScene->onEnter();

    cocos2d::CCDirector::get()->willSwitchToScene(m_pInScene);

    m_pOutScene->onExit();
    m_pInScene->onEnterTransitionDidFinish();
}

void CCTransitionPlayLayer::animateOutScene() {
    std::vector<cocos2d::CCNode*> up;
    std::vector<cocos2d::CCNode*> down;
    std::vector<cocos2d::CCNode*> left;
    std::vector<cocos2d::CCNode*> right;
    std::vector<cocos2d::CCNode*> scale;

    auto fixMenuAnchorPoint = [](cocos2d::CCNode* menu, bool setPositionToCenter = true) {
        if (!menu) return;

        std::vector<cocos2d::CCPoint> positions;
        for (int i = 0; i < menu->getChildrenCount(); i++) positions.push_back(menu->convertToWorldSpace(menu->getChildByIndex(i)->getPosition()));

        menu->ignoreAnchorPointForPosition(false);
        menu->setPosition(setPositionToCenter ? cocos2d::CCPoint(cocos2d::CCDirector::get()->getWinSize() / 2.f) : cocos2d::CCPoint{ 0.f, 0.f });

        for (int i = 0; i < menu->getChildrenCount(); i++) menu->getChildByIndex(i)->setPosition(menu->convertToNodeSpace(positions[i]));
    };

    auto levelInfoLayer = m_pOutScene->getChildByType<LevelInfoLayer>(0);
    if (levelInfoLayer) {
        if (auto node = levelInfoLayer->getChildByID("copy-indicator")) up.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("length-icon")) right.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("downloads-icon")) right.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("orbs-icon")) right.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("likes-icon")) right.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("downloads-label")) right.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("length-label")) right.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("exact-length-label")) right.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("likes-label")) right.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("orbs-label")) right.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("stars-icon")) left.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("stars-label")) left.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("diamonds-icon")) left.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("diamonds-label")) left.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("coin-icon-1")) left.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("coin-icon-2")) left.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("coin-icon-3")) left.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("custom-songs-widget")) down.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("left-side-menu")) left.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("bottom-left-art")) left.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("bottom-right-art")) right.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("back-menu")) left.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("right-side-menu")) right.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("other-menu")) scale.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("settings-menu")) down.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("creator-info-menu")) up.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("play-menu")) scale.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("difficulty-sprite")) left.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("normal-mode-bar")) down.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("practice-mode-bar")) down.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("normal-mode-percentage")) down.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("practice-mode-percentage")) down.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("normal-mode-label")) down.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("practice-mode-label")) down.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("high-object-indicator")) up.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("title-label")) up.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("garage-menu")) up.push_back(node);

        fixMenuAnchorPoint(levelInfoLayer->getChildByID("play-menu"));
    }

    auto editLevelLayer = m_pOutScene->getChildByType<EditLevelLayer>(0);
    if (editLevelLayer) {
        if (auto node = editLevelLayer->getChildByID("level-name-background")) up.push_back(node);
        if (auto node = editLevelLayer->getChildByID("description-background")) up.push_back(node);
        if (auto node = editLevelLayer->getChildByID("level-edit-menu")) scale.push_back(node);
        if (auto node = editLevelLayer->getChildByID("level-length")) down.push_back(node);
        if (auto node = editLevelLayer->getChildByID("level-song")) down.push_back(node);
        if (auto node = editLevelLayer->getChildByID("level-verified")) down.push_back(node);
        if (auto node = editLevelLayer->getChildByID("version-label")) down.push_back(node);
        if (auto node = editLevelLayer->getChildByID("level-id-label")) down.push_back(node);
        if (auto node = editLevelLayer->getChildByID("description-menu")) left.push_back(node);
        if (auto node = editLevelLayer->getChildByID("folder-menu")) left.push_back(node);
        if (auto node = editLevelLayer->getChildByID("bottom-left-art")) left.push_back(node);
        if (auto node = editLevelLayer->getChildByID("bottom-right-art")) right.push_back(node);
        if (auto node = editLevelLayer->getChildByID("back-menu")) left.push_back(node);
        if (auto node = editLevelLayer->getChildByID("level-actions-menu")) right.push_back(node);
        if (auto node = editLevelLayer->getChildByID("info-button-menu")) scale.push_back(node);
        if (auto node = editLevelLayer->getChildByID("level-name-input")) up.push_back(node);
        if (auto node = editLevelLayer->getChildByID("description-input")) up.push_back(node);

        fixMenuAnchorPoint(editLevelLayer->getChildByID("info-button-menu"));
    }

    auto levelSelectLayer = m_pOutScene->getChildByType<LevelSelectLayer>(0);
    if (levelSelectLayer) {
        if (auto node = levelSelectLayer->getChildByID("ground-layer")) down.push_back(node);
        if (auto node = levelSelectLayer->getChildByID("levels-list")) scale.push_back(node);
        if (auto node = levelSelectLayer->getChildByID("bottom-center-menu")) down.push_back(node);
        if (auto node = levelSelectLayer->getChildByID("info-menu")) right.push_back(node);
        if (auto node = levelSelectLayer->getChildByID("top-bar-sprite")) up.push_back(node);
        if (auto node = levelSelectLayer->getChildByID("bottom-left-corner")) left.push_back(node);
        if (auto node = levelSelectLayer->getChildByID("bottom-right-corner")) right.push_back(node);
        if (auto node = levelSelectLayer->getChildByID("back-menu")) left.push_back(node);
        if (auto node = levelSelectLayer->getChildByID("arrows-menu")) scale.push_back(node);

        fixMenuAnchorPoint(levelSelectLayer->getChildByID("arrows-menu"));
    }

    auto levelAreaInnerLayer = m_pOutScene->getChildByType<LevelAreaInnerLayer>(0);
    if (levelAreaInnerLayer) {
        // this used to list out every node in main-node but i couldnt be bothered
        if (auto node = levelAreaInnerLayer->getChildByID("main-node")) scale.push_back(node);
        if (auto node = levelAreaInnerLayer->getChildByID("back-menu")) scale.push_back(node);

        fixMenuAnchorPoint(levelAreaInnerLayer->getChildByID("back-menu"));
    }

    auto secretLayer2 = m_pOutScene->getChildByType<SecretLayer2>(0);
    if (secretLayer2) {
        if (auto node = secretLayer2->getChildByID("textbox-background")) down.push_back(node);
        if (auto node = secretLayer2->getChildByID("vault-name")) up.push_back(node);
        if (auto node = secretLayer2->getChildByID("message")) up.push_back(node);
        if (auto node = secretLayer2->getChildByID("the-challenge-text1")) right.push_back(node);
        if (auto node = secretLayer2->getChildByID("the-challenge-text2")) right.push_back(node);
        if (auto node = secretLayer2->getChildByID("diamonds-icon")) right.push_back(node);
        if (auto node = secretLayer2->getChildByID("diamonds-label")) right.push_back(node);
        if (auto node = secretLayer2->getChildByID("menu")) scale.push_back(node);
        if (auto node = secretLayer2->getChildByID("text-box")) down.push_back(node);

        fixMenuAnchorPoint(secretLayer2->getChildByID("menu"));
    }

    auto gdgLevelLayer = m_pOutScene->getChildByType<LevelLayer>(0);
    if (gdgLevelLayer) {
        if (auto node = gdgLevelLayer->getChildByID("side-art-bottom-left")) left.push_back(node);
        if (auto node = gdgLevelLayer->getChildByID("side-art-bottom-right")) right.push_back(node);
        if (auto node = gdgLevelLayer->getChildByID("side-art-top-right")) right.push_back(node);
        if (auto node = gdgLevelLayer->getChildByID("CustomSongWidget")) down.push_back(node);
        if (auto node = gdgLevelLayer->getChildByID("difficulty-sprite")) left.push_back(node);
        if (auto node = gdgLevelLayer->getChildByID("stars-label")) left.push_back(node);
        if (auto node = gdgLevelLayer->getChildByID("stars-icon")) left.push_back(node);
        if (auto node = gdgLevelLayer->getChildByID("name-label")) up.push_back(node);
        if (auto node = gdgLevelLayer->getChildByID("author-label")) up.push_back(node);
        if (auto node = gdgLevelLayer->getChildByID("button-menu")) scale.push_back(node);
        if (auto node = gdgLevelLayer->getChildByID("back-menu")) left.push_back(node);

        fixMenuAnchorPoint(gdgLevelLayer->getChildByID("button-menu"));
    }

    if (auto modSettingsPopup = m_pOutScene->getChildByType<ModSettingsPopup>(0)) scale.push_back(modSettingsPopup);

    for (auto node : up) node->runAction(cocos2d::CCEaseExponentialIn::create(cocos2d::CCMoveBy::create(m_fDuration, { 0.f, 150.f })));
    for (auto node : down) node->runAction(cocos2d::CCEaseExponentialIn::create(cocos2d::CCMoveBy::create(m_fDuration, { 0.f, -150.f })));
    for (auto node : left) node->runAction(cocos2d::CCEaseExponentialIn::create(cocos2d::CCMoveBy::create(m_fDuration, { -150.f, 0.f })));
    for (auto node : right) node->runAction(cocos2d::CCEaseExponentialIn::create(cocos2d::CCMoveBy::create(m_fDuration, { 150.f, 0.f })));
    for (auto node : scale) node->runAction(cocos2d::CCEaseExponentialIn::create(cocos2d::CCScaleBy::create(m_fDuration, 1.5f, 1.5f)));
}
