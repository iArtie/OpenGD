/*************************************************************************
    OpenGD - Open source Geometry Dash.
    Copyright (C) 2023  OpenGD Team

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License    
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*************************************************************************/

#include "LevelPage.h"
#include "MenuItemSpriteExtra.h"
#include "core/ui/UIScale9Sprite.h"
#include "PlayLayer.h"
#include "FMODAudioEngine.h"
#include "LevelTools.h"

#include "LevelDebugLayer.h"
#include "LevelEditorLayer.h"

#include "2d/Label.h"
#include "2d/Menu.h"
#include "2d/Transition.h"
#include "MenuLayer.h"
#include "GJGameLevel.h"
#include "base/Director.h"
#include "GameToolbox/log.h"
#include "GameToolbox/getTextureString.h"

#include "ButtonSprite.h"
#include "2d/ActionInstant.h"

bool LevelPage::replacingScene = false;

bool LevelPage::init(GJGameLevel* level)
{
	if (!Layer::init()) return false;
	
	LevelPage::replacingScene = false;

	//testing
	//level->_normalPercent = static_cast<float>(GameToolbox::randomInt(0, 100));
	//level->_practicePercent = static_cast<float>(GameToolbox::randomInt(0, 100));
	
	GameToolbox::log("normal: {}, practice: {}", level->_normalPercent, level->_practicePercent);
	
	_level = level;
	std::string barTexture = GameToolbox::getTextureString("GJ_progressBar_001.png");
	std::string bigFontTexture = GameToolbox::getTextureString("bigFont.fnt");

	const auto& winSize = ax::Director::getInstance()->getWinSize();

	//TODO: fix bars lol
	auto normalBar = ax::Sprite::create(barTexture);
	normalBar->setPosition({ winSize.width / 2, winSize.height / 2.f - 30 });
	normalBar->setColor({0, 0, 0});
	normalBar->setOpacity(125);
	addChild(normalBar, 3);

	auto normalProgress = ax::Sprite::create(barTexture);
	normalProgress->setPosition({1.36f, 10});
	normalProgress->setColor({0, 255, 0});
	normalProgress->setOpacity(255);
	normalProgress->setAnchorPoint({0, 0.5});
	normalProgress->setContentSize({340 / (level->_normalPercent / 100.f), 20});
	normalProgress->setScale(0.992f);
	normalProgress->setScaleX(0.992f);
	normalProgress->setScaleY(0.86f);
	normalBar->addChild(normalProgress);


	auto practiceBar = ax::Sprite::create(barTexture);
	practiceBar->setPosition({ winSize.width / 2, winSize.height / 2.f - 80 });
	practiceBar->setColor({0, 0, 0});
	practiceBar->setOpacity(125);
	addChild(practiceBar, 3);

	auto practiceProgress = ax::Sprite::create(barTexture);
	practiceProgress->setPosition({1.36f, 10});
	practiceProgress->setColor({0, 255, 0});
	practiceProgress->setOpacity(255);
	practiceProgress->setAnchorPoint({0, 0.5});
	practiceProgress->setContentSize({340 / (level->_normalPercent / 100.f), 20});
	practiceProgress->setScale(0.992f);
	practiceProgress->setScaleX(0.992f);
	practiceProgress->setScaleY(0.86f);
	practiceBar->addChild(practiceProgress);

	auto normalText = ax::Label::createWithBMFont(bigFontTexture, "Normal Mode");
	normalText->setPosition({ winSize.width / 2, winSize.height / 2.f - 10 });
	// normalText->enableShadow(ax::Color4B::BLACK, {0.2, -0.2});
	normalText->setScale(0.55f);
	addChild(normalText, 4);

	auto practiceText = ax::Label::createWithBMFont(bigFontTexture, "Practice Mode");
	practiceText->setPosition({ winSize.width / 2, winSize.height / 2.f - 60 });
	// practiceText->enableShadow(ax::Color4B::BLACK, {0.2, -0.2});
	practiceText->setScale(0.55f);
	addChild(practiceText, 4);

	auto normalPerc = ax::Label::createWithBMFont(bigFontTexture, "");
	normalPerc->setPosition({ winSize.width / 2, winSize.height / 2.f - 30 });
	normalPerc->setString(fmt::format("{:.3}%", level->_normalPercent));
	normalPerc->setScale(0.55f);
	addChild(normalPerc, 4);

	auto practicePerc = ax::Label::createWithBMFont(bigFontTexture, "");
	practicePerc->setPosition({ winSize.width / 2, winSize.height / 2.f - 80 });
	practicePerc->setString(fmt::format("{:.3}%", level->_practicePercent));
	practicePerc->setScale(0.55f);
	addChild(practicePerc, 4);
	
	auto scale9 = ax::ui::Scale9Sprite::create(GameToolbox::getTextureString("square02_001.png"));
	scale9->setPosition({170, 47.5});
	scale9->setStretchEnabled(true);
	scale9->setContentSize({340, 95});
	scale9->setOpacity(125);
	
	auto levelName = ax::Label::createWithBMFont(bigFontTexture, level->_levelName);
	levelName->setPosition(190, 50.5);

	float baseScale = 0.904f;
	float textWidth = levelName->getContentSize().width;
	float maxWidth = 260.0f; // Límite máximo de ancho en píxeles antes de encogerse

	// Si el texto multiplicado por la escala base es más grande que el límite...
	if ((textWidth * baseScale) > maxWidth) {
		// ...calculamos una nueva escala más pequeña para que encaje perfecto
		float newScale = maxWidth / textWidth;
		levelName->setScale(newScale);
	}
	else {
		// Si es un nombre corto, usamos la escala normal
		levelName->setScale(baseScale);
	}

	scale9->addChild(levelName, 0);

	auto diffIcon = ax::Sprite::createWithSpriteFrameName("diffIcon_01_btn_001.png");
	diffIcon->setScale(1.1f);
	diffIcon->setPosition(35.75, 50.5);
	scale9->addChild(diffIcon, 0);

	//1.0 didnt have stars apparently
	// auto starIcon = ax::Sprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
	// starIcon->setScale(0.7);
	// starIcon->setPosition({325, 82});
	// mainNode->addChild(starIcon, 0);

	// auto starAmt = ax::Label::createWithBMFont("bigFont.fnt", std::to_string(level->_Stars));
	// starAmt->setPosition({313, 82.5});
	// starAmt->setScale(0.5);
	// starAmt->setAnchorPoint({1, 0.5});
	// mainNode->addChild(starAmt, 0);
	auto mainBtn = MenuItemSpriteExtra::create(scale9, AX_CALLBACK_1(LevelPage::onPlay, this));
	mainBtn->setScaleMultiplier(1.1f);
	auto levelMenu = ax::Menu::create();
	levelMenu->addChild(mainBtn);
	levelMenu->setPosition({ winSize.width / 2.f, winSize.height / 2.f + 60 });
	/*
	auto buttonSprite = ButtonSprite::create("editor", 0x32, 0, 0.6, false, GameToolbox::getTextureString("bigFont.fnt"), GameToolbox::getTextureString("GJ_button_01.png"), 30);
	MenuItemSpriteExtra* button = MenuItemSpriteExtra::create(buttonSprite, [this](Node* btn)
	{
		if (LevelPage::replacingScene)
			return;

		ax::Scene* scene = LevelEditorLayer::scene(_level);
		ax::AudioEngine::stopAll();
		ax::AudioEngine::play2d("playSound_01.ogg", false, 0.2f);
		ax::Director::getInstance()->replaceScene(ax::TransitionFade::create(0.5f, scene));
		LevelPage::replacingScene = true;
		MenuLayer::music = false;
	});
	button->setPositionY(70);
	levelMenu->addChild(button);
	*/
	addChild(levelMenu);
	
	return true;
}

void LevelPage::onPlay(Node* btn)
{
	if (LevelPage::replacingScene)
		return;

	// Bloqueamos futuros toques
	LevelPage::replacingScene = true;
	MenuLayer::music = false;

	// 1. Detenemos la música del menú y hacemos el sonido de confirmación al instante CON FMOD
	FMODAudioEngine::getInstance()->stopAllMusic();
	FMODAudioEngine::getInstance()->playEffect("playSound_01.ogg");

	// Usamos la misma estructura de acciones que GD para delegar el trabajo 
	// al siguiente frame y no congelar el botón en pleno clic
	auto delay = ax::DelayTime::create(0.75f);
	auto callStep2 = ax::CallFunc::create(AX_CALLBACK_0(LevelPage::playStep2, this));
	auto seq = ax::Sequence::create(delay, callStep2, nullptr);

	this->runAction(seq);
}

void LevelPage::playStep2()
{
	// Usamos _levelID para precargar, ya que el levelstring aún no se ha leído
	std::string musicFilename = LevelTools::getAudioFilename(_level->_levelID);

	// Precargamos el audio en FMOD
	FMODAudioEngine::getInstance()->preloadMusic(musicFilename);

	auto delay = ax::DelayTime::create(0.0f);
	auto callStep3 = ax::CallFunc::create(AX_CALLBACK_0(LevelPage::playStep3, this));
	auto seq = ax::Sequence::create(delay, callStep3, nullptr);

	this->runAction(seq);
}

void LevelPage::playStep3()
{
	// Finalmente, cargamos las lógicas pesadas y transicionamos
	ax::Scene* scene = PlayLayer::scene(_level);
	ax::Director::getInstance()->replaceScene(ax::TransitionFade::create(0.5f, scene));
}

LevelPage* LevelPage::create(GJGameLevel* level)
{
	LevelPage* pRet = new LevelPage();
	if (pRet->init(level))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}