#include "EndLevelLayer.h"
#include "PlayLayer.h"
#include "DropDownLayer.h"
#include "MenuItemSpriteExtra.h"
#include "UILayer.h"
#include "CircleWave.h"
#include "2d/Label.h"
#include "2d/Menu.h"
#include "base/Director.h"
#include "AudioEngine.h"
#include <fmt/chrono.h>
#include <array>
#include "GameToolbox/getTextureString.h"
#include "GameToolbox/rand.h"
#include "GJGameLevel.h"
#include "2d/ActionInstant.h"
#include "2d/ActionInterval.h"
#include "2d/ActionEase.h"

USING_NS_AX;

constexpr static auto _endingStrings = std::to_array <const char*>({
	"Awesome!", "Good Job!", "Well Done!", "Impressive!",
	"Amazing!", "Incredible!", "Skillful!", "Brilliant!",
	"You are... The One!", "How this is possible?",
	"You beat me...", "Challenge Breaker!", "I am speechless...",
	"Not bad!", "Warp Speed", "Y u do dis?", "lol is lol backwards",
	"DROP THE BEAT", "Took you long enough...", "Teach me oh great one",
	"Haxxor?", "Tripple spikes?", "RubRubRubRubRubRub", "SO FAST"
	});

std::string_view EndLevelLayer::getRandomEndingString() {
	return _endingStrings[GameToolbox::randomInt(0, _endingStrings.size() - 1)];
}

EndLevelLayer* EndLevelLayer::create(int attempts, int jumps, int time, bool everyplayIncluded, int stars) {
	auto pRet = new (std::nothrow) EndLevelLayer();
	if (pRet) {
		pRet->_attempts = attempts;
		pRet->_jumps = jumps;
		pRet->_time = time;
		pRet->_everyplay_included = everyplayIncluded;
		pRet->_stars = stars;
		pRet->_createdWithoutPlaylayer = true;
		if (pRet->init(nullptr)) {
			pRet->autorelease();
			return pRet;
		}
		AX_SAFE_DELETE(pRet);
	}
	return nullptr;
}

EndLevelLayer* EndLevelLayer::create(PlayLayer* pl) {
	auto pRet = new (std::nothrow) EndLevelLayer();
	if (pRet && pl != nullptr) {
		auto level = pl->getLevel();
		pRet->_attempts = pl->_attempts;
		pRet->_jumps = pl->_jumps;
		pRet->_time = pl->_secondsSinceStart;
		pRet->_createdWithoutPlaylayer = false;
		pRet->_everyplay_included = pl->_everyplay_recorded;
		pRet->_stars = level->_stars;
		pRet->_testmode = pl->_isTestMode;
	}
	else {
		pRet->_createdWithoutPlaylayer = true;
	}

	if (pRet && pRet->init(pl)) {
		pRet->autorelease();
		return pRet;
	}
	AX_SAFE_DELETE(pRet);
	return nullptr;
}

bool EndLevelLayer::init(PlayLayer* pl) {
	auto nd = ax::Layer::create();
	_statsLayer = DropDownLayer::create(nd, "");
	_statsLayer->showLayer(_createdWithoutPlaylayer, true);
	_statsLayer->hideBackButton();
	_playlayer = pl;

	if (!_createdWithoutPlaylayer) {
		pl->m_pHudLayer->addChild(_statsLayer);
	}
	const auto& wsize = ax::Director::getInstance()->getWinSize();

	ax::AudioEngine::play2d("endStart_02.ogg", false, 1.0f);

	auto sprite = ax::Sprite::createWithSpriteFrameName("GJ_levelComplete_001.png");
	sprite->setScale(0.8f);
	sprite->setPositionY(wsize.height / 4.5);
	nd->addChild(sprite);

	std::string goldFontStr = GameToolbox::getTextureString("goldFont.fnt");
	std::string bigFontStr = GameToolbox::getTextureString("bigFont.fnt");

	std::string attemptsText = fmt::format("Attempts: {}", _attempts);
	auto attempts = ax::Label::createWithBMFont(goldFontStr, attemptsText);
	attempts->setPositionY(wsize.height / 5 - 25 - 10);
	attempts->setScale(0.8f);
	nd->addChild(attempts);

	std::string jumpsText = fmt::format("Jumps: {}", _jumps);
	auto jumps = ax::Label::createWithBMFont(goldFontStr, jumpsText);
	jumps->setPositionY(wsize.height / 5 - 50 - 10);
	jumps->setScale(0.8f);
	nd->addChild(jumps);

	std::chrono::seconds duration{ _time };
	std::string timeText = fmt::format("Time: {:%M:%S}", duration);
	auto time = ax::Label::createWithBMFont(goldFontStr, timeText);
	time->setPositionY(wsize.height / 5 - 75 - 10);
	time->setScale(0.8f);
	nd->addChild(time);

	std::string_view randomText = EndLevelLayer::getRandomEndingString();
	if (_testmode) randomText = "Level Verified!";

	auto randomt = ax::Label::createWithBMFont(bigFontStr, randomText);
	randomt->setPositionY(wsize.height / 5 - 120 - 10);
	randomt->setScale(randomText.length() > 13 ? 0.75f : 0.8f);
	nd->addChild(randomt);

	auto btnmenu = ax::Menu::create();
	_statsLayer->_dropLayer->addChild(btnmenu);
	btnmenu->setPosition({ wsize.width / 3, wsize.height / 8 });

	auto replayspr = ax::Sprite::createWithSpriteFrameName("GJ_replayBtn_001.png");
	auto replaybtn = MenuItemSpriteExtra::create(replayspr, [&](Node* nd) {
		this->_statsLayer->hideLayer();
		if (!this->_createdWithoutPlaylayer) {
			ax::AudioEngine::stopAll();
			ax::AudioEngine::play2d("playSound_01.ogg", false, 1.0f);

			auto delay = ax::DelayTime::create(0.5f);
			auto call = ax::CallFunc::create([this]() {
				_playlayer->resumeAndRestart(true);
				this->removeFromParentAndCleanup(true);
				});
			_playlayer->runAction(ax::Sequence::create(delay, call, nullptr));
		}
		});
	btnmenu->addChild(replaybtn);

	auto exitspr = ax::Sprite::createWithSpriteFrameName("GJ_menuBtn_001.png");
	auto exitbtn = MenuItemSpriteExtra::create(exitspr, [&](Node* nd) {
		this->_statsLayer->hideLayer();
		if (!this->_createdWithoutPlaylayer) {
			ax::AudioEngine::stopAll();
			ax::AudioEngine::play2d("quitSound_01.ogg", false, 1.0f);
			this->_playlayer->exit();
		}
		});
	exitbtn->setPositionX(wsize.width / 3);
	btnmenu->addChild(exitbtn);

	this->scheduleOnce([&, wsize](float delta) {
		if (_stars < 1) return;

		ax::AudioEngine::play2d("highscoreGet02.ogg", false, 1.0f);

		auto starNode = ax::Node::create();
		auto bigstar = ax::Sprite::createWithSpriteFrameName("GJ_bigStar_001.png");
		std::string label = fmt::format("+{}", _stars);
		auto star_textLabel = ax::Label::createWithBMFont(GameToolbox::getTextureString("bigFont.fnt"), label);
		auto circleWave = CircleWave::create(2.f, { 0xFF, 0xFF, 0xFF, 0xFF }, 10, 100.f, true, true);

		bigstar->setScale(1.1f);
		starNode->addChild(circleWave, 0);
		starNode->addChild(bigstar, 1);
		starNode->addChild(star_textLabel, 2);

		star_textLabel->setPosition(-2.f, -30.f);
		star_textLabel->setScale(0.7f);

		starNode->setPosition(wsize.width / 1.4f, wsize.height / 2.f);
		starNode->setScale(2.75f);
		starNode->setOpacity(0);

		auto scaleAction = ax::EaseBounceOut::create(ax::ScaleTo::create(0.4f, 1.0f));
		auto fadeAction = ax::FadeTo::create(0.4f, 255);

		starNode->runAction(scaleAction);
		starNode->runAction(fadeAction);

		_statsLayer->_dropLayer->addChild(starNode);
		}, 0.8f, "showStars");

	return true;
}