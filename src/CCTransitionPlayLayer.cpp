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

#ifdef NDEBUG
    cocos2d::CCTouchDispatcher::get()->setDispatchEvents(false);
#else
    cocos2d::CCTouchDispatcher::get()->setDispatchEvents(true);
#endif

    auto fakeChildren = cocos2d::CCArray::createWithCapacity(2);
    fakeChildren->addObject(m_pOutScene);
    fakeChildren->addObject(m_pInScene);
    this->setUserObject("geode.devtools/extra-children", fakeChildren);

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
    auto origShaderLayerBlendFunc = playLayer->m_shaderLayer->m_sprite->getBlendFunc();
    playLayer->m_shaderLayer->m_sprite->setBlendFunc({ GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA }); // see CCTransitionPlayLayer::draw

    auto startMode = (IconType)playLayer->m_levelSettings->m_startMode;
    if (playLayer->m_startPosObject) startMode = (IconType)playLayer->m_startPosObject->m_startSettings->m_startMode;

    // walkInPlayer returns false if the player shouldn't walk in (likely offscreen already)
    // if (!this->walkInPlayer(playLayer->m_player1, startMode)) {
        // auto origP1Scale = playLayer->m_player1->getScale(); // may not be 1 if the player starts as mini
        // playLayer->m_player1->setScale(0.f);
        // playLayer->m_player1->setRotation(-20.f);
        // playLayer->m_player1->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(m_fDuration, origP1Scale)));
        // playLayer->m_player1->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCRotateTo::create(m_fDuration, 0.f)));
    // }
    this->walkInPlayer(playLayer->m_player1, startMode);

    // if (!this->walkInPlayer(playLayer->m_player2, startMode)) {
    //     auto origP2Scale = playLayer->m_player2->getScale();
    //     playLayer->m_player2->setScale(0.f);
    //     playLayer->m_player2->setRotation(20.f);
    //     playLayer->m_player2->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(m_fDuration, origP2Scale)));
    //     playLayer->m_player2->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCRotateTo::create(m_fDuration, 0.f)));
    // }
    this->walkInPlayer(playLayer->m_player2, startMode);

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
    // keep in mind this is not correct if the z orders are updated
    bool isBackgroundIncluded = playLayer->m_shaderLayer->m_state.m_minBlendingLayer <= 1;
    if (isBackgroundIncluded) {
        playLayer->m_shaderLayer->m_pChildren->addObject(m_pOutScene);
    }

    // this is so it can be scaled around the bottom left corner for the pixelate shader if required
    m_pOutScene->setAnchorPoint({ 0.f, 0.f });
    m_pOutScene->ignoreAnchorPointForPosition(true);
    m_pOutScene->setContentSize(winSize);

    if (geode::Mod::get()->getSettingValue<bool>("animate-out-level-page")) {
        this->animateOutScene();
    }

    this->runAction(cocos2d::CCSequence::createWithTwoActions(
        cocos2d::CCDelayTime::create(m_fDuration),

        // cleanup
        geode::cocos::CallFuncExt::create([=, this] {
            setFakeIsRunningRecursive(m_pInScene, false);

            // playlayer kind of depends on ui trigger ui to be at 0, 0
            playLayer->m_uiTriggerUI->setPosition({ 0.f, 0.f });
            playLayer->m_uiTriggerUI->setContentSize({ 0.f, 0.f });

            playLayer->m_shaderLayer->m_sprite->setBlendFunc(origShaderLayerBlendFunc);

            CCTransitionScene::finish();
            if (rewindBackground) rewindBackground->setVisible(true);
            if (!isPlayer2Running) setFakeIsRunningRecursive(playLayer->m_player2, false);
            if (isBackgroundIncluded) playLayer->m_shaderLayer->m_pChildren->removeObject(m_pOutScene);
        })
    ));

    playLayer->updateShaderLayer(.01f); // hope nobody cares about losing 0.01s on their fade times
    this->scheduleUpdate();
}

void CCTransitionPlayLayer::update(float dt) {
    auto playLayer = m_pInScene->getChildByType<PlayLayer>(0);
    playLayer->updateShaderLayer(0.f); // we don't want to advance any fade times more than we need to
}

void CCTransitionPlayLayer::draw() {
    auto playLayer = m_pInScene->getChildByType<PlayLayer>(0);

    // keep in mind we have two of the SAME out scenes drawing - one drawn first, then one put in shaderlayer
    // the one put in shaderlayer will be drawn with the correct scale for the pixelate shader
    m_pOutScene->visit();

    // this is for specifically shaderlayer since the shader doesn't respect the opacity
    // shaderlayer->m_sprite has blend func set to GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA
    glBlendColor(.0f, .0f, .0f, playLayer->m_shaderLayer->m_sprite->getOpacity() / 255.f);
    m_pOutScene->setScaleX(1.f / (playLayer->m_shaderLayer->m_state.m_pixelateTargetX * playLayer->m_shaderLayer->m_scaleFactor));
    m_pOutScene->setScaleY(1.f / (playLayer->m_shaderLayer->m_state.m_pixelateTargetY * playLayer->m_shaderLayer->m_scaleFactor));
    m_pInScene->visit();
    m_pOutScene->setScale(1.f);
    glBlendColor(0.f, 0.f, 0.f, 0.f);
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
        if (auto node = levelInfoLayer->getChildByID("diamond-icon")) left.push_back(node);
        if (auto node = levelInfoLayer->getChildByID("diamond-label")) left.push_back(node);
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

bool CCTransitionPlayLayer::walkInPlayer(PlayerObject* player, IconType gamemode) {
    if (!player->getParent()) return false;

    auto playerPos = player->getParent()->convertToWorldSpace(player->getPosition());
    auto winSize = cocos2d::CCDirector::get()->getWinSize();
    if (playerPos.x <= -15.f) return false;
    if (playerPos.x >= winSize.width + 15.f) return false;
    if (playerPos.y >= winSize.height + 15.f) return false;
    if (playerPos.y <= -15.f) return false;

    auto endX = player->getPositionX();
    float startX;
    if (playerPos.x < winSize.width / 2.f) {
        // player is more to the left
        startX = std::min(playerPos.x - 134.f, -16.f); // either offset 134 units back or fixed 16 units back, whichever is further left
    } else {
        // player is more to the right
        startX = std::max(playerPos.x + 134.f, winSize.width + 16.f);
    }
    startX = player->getParent()->convertToNodeSpace({ startX, 0.f }).x;

    auto distance = endX - startX;

    player->setPositionX(startX);

    cocos2d::CCActionInterval* moveRightAction = cocos2d::CCMoveBy::create(m_fDuration, { distance, 0.f });

    switch (gamemode) {
        // run in
        case IconType::Robot: {
            player->m_robotSprite->runAnimationForced("run");
        } break;

        // walk in
        case IconType::Spider: {
            player->m_spiderSprite->runAnimationForced("walk");
        } break;

        // rotate in
        case IconType::Ball: {
            // s=r*theta or something so angle in radians=distance/radius
            player->runAction(cocos2d::CCEaseBackOut::create(cocos2d::CCRotateBy::create(m_fDuration, distance / (30.f * player->m_vehicleSize) * kmPIUnder180)));
            moveRightAction = cocos2d::CCEaseBackOut::create(moveRightAction);
        } break;

        // pick a random enter animation
        case IconType::Ship:
        case IconType::Ufo:
        case IconType::Swing:
        case IconType::Wave:
        case IconType::Cube: {
            this->createWalkInAnimation(player, distance, &moveRightAction);
        } break;

        // wtf
        case IconType::Item:
        case IconType::Special:
        case IconType::DeathEffect:
        case IconType::ShipFire:
        case IconType::Jetpack: // (jetpack uses IconType::Ship)
            break;
    }

    player->runAction(moveRightAction);

    return true;
}

// this doesn't and probably won't ever support the player being rotated
// it's not like this runs very often anyway how often does the player start onscreen
void CCTransitionPlayLayer::createWalkInAnimation(PlayerObject* player, float distance, cocos2d::CCActionInterval** moveRightAction) {
    auto forceAnchorPoint = [player](cocos2d::CCPoint pos) {
        auto size = 30.f * player->m_vehicleSize;
        player->setContentSize({ size, size });
        player->setAnchorPoint(pos - cocos2d::CCPoint{ .5f, .5f });
        player->ignoreAnchorPointForPosition(true);
    };

    auto resetAnchorPoint = [player]() {
        player->setContentSize({ 0.f, 0.f });
        player->setAnchorPoint({ .5f, .5f });
        player->ignoreAnchorPointForPosition(false);
    };

    auto moveRightDurationLength = m_fDuration * .5f;

    switch (geode::utils::random::generate(0, 4)) {
        case 0: {
            // jump
            auto jumpDuration = m_fDuration * .5f;
            auto squishDuration = m_fDuration * .25f;

            player->setRotation(-180.f);
            player->runAction(cocos2d::CCSequence::create(
                cocos2d::CCSpawn::createWithTwoActions(
                    cocos2d::CCRotateBy::create(jumpDuration, 180.f),
                    cocos2d::CCSequence::createWithTwoActions(
                        cocos2d::CCEaseSineOut::create(cocos2d::CCMoveBy::create(jumpDuration / 2.f, { 0.f, 64.f * player->m_vehicleSize })),
                        cocos2d::CCEaseSineIn::create(cocos2d::CCMoveBy::create(jumpDuration / 2.f, { 0.f, -64.f * player->m_vehicleSize }))
                    )
                ),
                geode::cocos::CallFuncExt::create([forceAnchorPoint] {
                    // set the anchor point to the bottom middle so we can scale around that point
                    forceAnchorPoint({ .5f, 0.f });
                }),
                cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(squishDuration / 2.f, 1.4f, .7f)),
                cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(squishDuration / 2.f, 1.f, 1.f)),
                geode::cocos::CallFuncExt::create(resetAnchorPoint),
                nullptr
            ));
        } break;

        case 1: {
            // rotate in
            auto rotateDuration = m_fDuration * .5f;
            auto resetDuration = m_fDuration * .5f;

            player->runAction(cocos2d::CCSequence::createWithTwoActions(
                cocos2d::CCRotateBy::create(rotateDuration, distance / (30.f * player->m_vehicleSize) * kmPIUnder180),
                cocos2d::CCSpawn::createWithTwoActions(
                    cocos2d::CCRotateTo::create(resetDuration, 0.f),
                    cocos2d::CCSequence::createWithTwoActions(
                        cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(resetDuration / 2.f, { 0.f, 30.f })),
                        cocos2d::CCEaseExponentialIn::create(cocos2d::CCMoveBy::create(resetDuration / 2.f, { 0.f, -30.f }))
                    )
                )
            ));
        } break;

        case 2: {
            // throw in
            auto throwDuration = m_fDuration * .8f;
            auto squishDuration = m_fDuration * .2f;

            auto origY = player->getPositionY();
            auto newY = player->getParent()->convertToNodeSpace({ 0.f, -80.f }).y;
            player->setPositionY(newY);
            player->setRotation(-180.f);

            player->runAction(cocos2d::CCSequence::create(
                cocos2d::CCSpawn::createWithTwoActions(
                    cocos2d::CCRotateBy::create(throwDuration, -540.f),
                    cocos2d::CCSequence::createWithTwoActions(
                        cocos2d::CCEaseSineOut::create(cocos2d::CCMoveBy::create(throwDuration * .625f, { 0.f, (origY - newY) + 60.f })),
                        cocos2d::CCEaseSineIn::create(cocos2d::CCMoveBy::create(throwDuration * .375f, { 0.f, -60.f }))
                    )
                ),

                // see above
                geode::cocos::CallFuncExt::create([forceAnchorPoint] { forceAnchorPoint({ .5f, 0.f }); }),
                cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(squishDuration / 2.f, 1.2f, .9f)),
                cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(squishDuration / 2.f, 1.f, 1.f)),
                geode::cocos::CallFuncExt::create(resetAnchorPoint),
                nullptr
            ));

            moveRightDurationLength = throwDuration;
            *moveRightAction = cocos2d::CCEaseSineInOut::create(*moveRightAction);
        } break;

        case 3: {
            // boring slide in
            moveRightDurationLength = m_fDuration;
            *moveRightAction = cocos2d::CCEaseExponentialOut::create(*moveRightAction);
        } break;
    }

    (*moveRightAction)->m_fDuration = moveRightDurationLength;
}
