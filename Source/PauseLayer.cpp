#include "PauseLayer.h"
#include "PlayLayer.h"
#include "MenuItemSpriteExtra.h"
#include "core/ui/UIScale9Sprite.h"
#include "base/Director.h"
#include "2d/Menu.h"
#include "2d/Label.h"
#include "EventDispatcher.h"
#include "EventListenerTouch.h"
#include "GameToolbox/getTextureString.h"
#include "GJGameLevel.h"

USING_NS_AX;

PauseLayer* PauseLayer::create(PlayLayer* playLayer) {
	auto ret = new PauseLayer();
	if (ret && ret->init(playLayer)) {
		ret->autorelease();
		return ret;
	}
	AX_SAFE_DELETE(ret);
	return nullptr;
}

bool PauseLayer::init(PlayLayer* playLayer) {
	if (!LayerColor::initWithColor({ 0, 0, 0, 75 })) return false;

	_playLayer = playLayer;
	customSetup();

	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = [](Touch* touch, Event* event) { return true; };
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

	return true;
}

void PauseLayer::customSetup() {
	auto winSize = Director::getInstance()->getWinSize();
	auto bg = ui::Scale9Sprite::create(GameToolbox::getTextureString("square04_001.png"));
	bg->setContentSize({ 549, 300 });
	bg->setPosition(winSize / 2);
	bg->setColor({ 0, 0, 0 });
	bg->setOpacity(175);
	this->addChild(bg);

	std::string levelName = _playLayer->getLevel() ? _playLayer->getLevel()->_levelName : "Level";
	auto title = Label::createWithBMFont(GameToolbox::getTextureString("bigFont.fnt"), levelName);
	title->setPosition({ winSize.width / 2, winSize.height / 2 + 90 });
	title->setScale(0.8f);
	this->addChild(title);

	auto menu = Menu::create();
	this->addChild(menu);

	auto practiceSpr = Sprite::createWithSpriteFrameName("GJ_practiceBtn_001.png");
	auto practiceBtn = MenuItemSpriteExtra::create(practiceSpr, AX_CALLBACK_1(PauseLayer::onPracticeMode, this));

	auto resumeSpr = Sprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
	auto resumeBtn = MenuItemSpriteExtra::create(resumeSpr, AX_CALLBACK_1(PauseLayer::onResume, this));

	auto quitSpr = Sprite::createWithSpriteFrameName("GJ_menuBtn_001.png");
	auto quitBtn = MenuItemSpriteExtra::create(quitSpr, AX_CALLBACK_1(PauseLayer::onQuit, this));

	auto restartSpr = Sprite::createWithSpriteFrameName("GJ_replayBtn_001.png");
	auto restartBtn = MenuItemSpriteExtra::create(restartSpr, AX_CALLBACK_1(PauseLayer::onRestart, this));

	menu->addChild(practiceBtn);
	menu->addChild(resumeBtn);
	menu->addChild(quitBtn);
	menu->addChild(restartBtn);
	menu->alignItemsHorizontallyWithPadding(15.0f);
}

void PauseLayer::onResume(Node* sender) {
	_playLayer->resume();
}

void PauseLayer::onRestart(Node* sender) {
	_playLayer->resumeAndRestart(true);
}

void PauseLayer::onQuit(Node* sender) {
	_playLayer->exit();
}

void PauseLayer::onPracticeMode(Node* sender) {
	_playLayer->_isPracticeMode = !_playLayer->_isPracticeMode;
	onResume(sender);
}

void PauseLayer::onEnter() { LayerColor::onEnter(); }
void PauseLayer::onExit() { LayerColor::onExit(); }