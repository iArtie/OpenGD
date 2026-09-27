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

#include "GarageLayer.h"
#include "GameToolbox/enums.h"
#include "MenuItemSpriteExtra.h"
#include "MenuItemToggler.h"
#include "MenuLayer.h"
#include "SimplePlayer.h"
#include "GameManager.h"
#include "TextInputNode.h"
#include "fmt/format.h"
#include "math/Rect.h"
#include "math/Vec2.h"
#include "ui/UITextField.h"
#include "2d/Transition.h"
#include "2d/Menu.h"
#include "EventDispatcher.h"
#include "UTF8.h"
#include "EventListenerKeyboard.h"
#include "ui/UIScale9Sprite.h"
#include "base/Director.h"
#include "GameToolbox/getTextureString.h"
#include "GameToolbox/log.h"
#include "GameToolbox/conv.h"
#include "GameToolbox/nodes.h"
#include <string>

USING_NS_AX;

Scene* GarageLayer::scene(bool popSceneWithTransition)
{
	auto s = Scene::create();
	auto garage = GarageLayer::create();
	//garage->_modePages = ;
	garage->_popSceneWithTransition = popSceneWithTransition;
	s->addChild(garage);

	return s;
}

GarageLayer* GarageLayer::create()
{
	auto r = new GarageLayer();
	if (r && r->init())
		r->autorelease();
	else
	{
		delete r;
		r = nullptr;
	}
	return r;
}

bool GarageLayer::init()
{
	if (!Scene::init())
		return false;

	auto gm = GameManager::getInstance();
	_selectedMode = gm->_mainSelectedMode;
	
	auto director = Director::getInstance();
	auto size  = director->getWinSize();

	GameToolbox::createBG(this, { 150, 150, 150 });
	GameToolbox::createCorners(this, true, false, true, true);

	
	_userNameField = ui::TextField::create("Username", GameToolbox::getTextureString("bigFont.fnt"), 20);
	_userNameField->setPlaceHolderColor({120, 170, 240});
	_userNameField->setMaxLength(10);
	_userNameField->setMaxLengthEnabled(true);
	_userNameField->setCursorEnabled(true);
	_userNameField->setString("Player");
	_userNameField->setPosition({ size.width / 2, size.height - 34 });
	this->addChild(_userNameField);

	/* pendiente, parece estar bugeada la escala
	_usernameInput = TextInputNode::create(180, 50, GameToolbox::getTextureString("bigFont.fnt"), "Username", 1);
	_usernameInput->setPosition({ size.width / 2, size.height - 34 });
	_usernameInput->setString("Player");
	_usernameInput->setMaxDisplayLabelScale(5);
	//to do: max label length
	_usernameInput->setPlaceholderColor({255, 0, 255});
	_usernameInput->setPlaceholderScale(.7f);
	_usernameInput->setAllowedChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
	this->addChild(_usernameInput, 20);*/

	auto line = Sprite::createWithSpriteFrameName("floorLine_001.png");
	line->setBlendFunc(GameToolbox::getBlending());
	line->setPosition(size / 2 + Vec2(0, 50));
	this->addChild(line);

	_iconPrev = SimplePlayer::create(0); // 132
	_iconPrev->updateGamemode(gm->getSelectedIcon(gm->_mainSelectedMode), gm->_mainSelectedMode);
	_iconPrev->setPosition({line->getPositionX(), line->getPositionY() + 25});
	_iconPrev->setScale(1.6f);
	this->addChild(_iconPrev);
	this->updatePlayerColors();

	this->setupIconSelect();

	auto menu = Menu::create();
	menu->setPosition({0, 0});

	auto backBtn = MenuItemSpriteExtra::create("GJ_arrow_03_001.png", [&](Node*) {
		if (_popSceneWithTransition) 
			GameToolbox::popSceneWithTransition(0.5f, popTransition::kTransitionShop);
		else {
			auto director = Director::getInstance();
			director->replaceScene(TransitionFade::create(0.5f, MenuLayer::scene()));
		}
	});
	backBtn->setPosition({24, size.height - 23});
	//backBtn->setSizeMult(1.6f);
	menu->addChild(backBtn);

	auto shop = MenuItemSpriteExtra::create("shopRope_001.png", [&](Node*) {
		_iconPrev->updateGamemode(35, IconType::kIconTypeUfo);
	});

	shop->setPosition({135, size.height - 25});
	shop->setDestination({0, -15});
	menu->addChild(shop);

	auto shard = MenuItemSpriteExtra::create("GJ_shardsBtn_001.png", [&](Node*) {

	});
	shard->setPosition({30, size.height - 80});
	menu->addChild(shard);

	auto paint = MenuItemSpriteExtra::create("GJ_paintBtn_001.png", [&](Node*) {

	});
	paint->setPosition({30, size.height - 120});
	menu->addChild(paint);

	
	// stats
	this->createStat("GJ_starsIcon_001.png", "6");
	this->createStat("GJ_coinsIcon_001.png", "8");
	this->createStat("GJ_coinsIcon2_001.png", "12");
	this->createStat("currencyOrbIcon_001.png", "14");
	this->createStat("GJ_diamondsIcon_001.png", "13");
	//this->createStat("GJ_demonIcon_001.png", "5");

	this->addChild(menu);

	auto listener = ax::EventListenerKeyboard::create();
	listener->onKeyPressed = [&](ax::EventKeyboard::KeyCode key, ax::Event*) {
		if (key == ax::EventKeyboard::KeyCode::KEY_ESCAPE) {
			if (_popSceneWithTransition) GameToolbox::popSceneWithTransition(0.5f, popTransition::kTransitionShop);
			else Director::getInstance()->replaceScene(TransitionFade::create(0.5f, MenuLayer::scene()));
		}
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	return true;
}

void GarageLayer::updatePlayerColors() {
	_iconPrev->setMainColor({125, 0, 255});
	_iconPrev->setSecondaryColor({0, 255, 255});
	_iconPrev->setGlow(true);
	_iconPrev->setGlowColor({0, 255, 0});
}

int GarageLayer::selectedGameModeInt()
{
	return this->modeToPageInt(_selectedMode);
}

int GarageLayer::modeToPageInt(IconType mode) {
	if (mode == IconType::kIconTypeSpecial)
		return _modePages.size() - 1;
	if (mode == IconType::kIconTypeDeathEffect)
		return _modePages.size();

	return static_cast<int>(mode);
}

void GarageLayer::createStat(const char* sprite, const char* statKey)
{
	const auto& size = Director::getInstance()->getWinSize();

	auto stat = Sprite::createWithSpriteFrameName(sprite);
	stat->setScale(.65);
	stat->setPosition({size.width - 20, size.height - 14 - 20 * _stats});
	this->addChild(stat);

	auto statL = Label::createWithBMFont(GameToolbox::getTextureString("bigFont.fnt"), "0"); // GameStatsManager::sharedState()->getStat(statKey)
	statL->setScale(.45);
	statL->setAnchorPoint({1, 0});
	statL->setPosition({stat->getPositionX() - 16, stat->getPositionY() - 7});
	this->addChild(statL);

	_stats++;
}

void GarageLayer::setupIconSelect()
{
	const auto& size = Director::getInstance()->getWinSize();

	auto bg = ui::Scale9Sprite::create(GameToolbox::getTextureString("square02_001.png"), Rect(0, 0, 80, 80));
	bg->setContentSize({385, 100});
	bg->setOpacity(75);
	bg->setPosition(size / 2 + Vec2(0, -65));
	this->addChild(bg);

	auto unlock = Sprite::createWithSpriteFrameName("GJ_unlockTxt_001.png");
	unlock->setPosition({size.width / 2, bg->getPositionY() + bg->getContentSize().height / 2 + 12});
	this->addChild(unlock);

	auto menu = Menu::create();

	for (int i = 0; i <= 8; i++)
	{
		//auto i1 = MenuItemToggler::create(s1, s2, this, menu_selector(SaiGarageLayer::onSelectTab));
		//i1->setSizeMult(1.2f);
		//i1->setTag(i);
		//i1->setClickable(false);
		//menu->addChild(i1);
		createIconButton(static_cast<IconType>(i),menu);
	}

	createIconButton(kIconTypeSpecial, menu);
	createIconButton(kIconTypeDeathEffect, menu);

	menu->alignItemsHorizontallyWithPadding(2.0f);
	menu->setPosition({size.width / 2, unlock->getPositionY() + 30 + 3});
	this->addChild(menu);

	auto menuArr = Menu::create();
	menuArr->setPosition({0, 0});

	auto arrow1 = Sprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	arrow1->setScale(.8f);

	auto onChangePage = [this](bool up)
	{
		int page = this->_modePages[_selectedMode];

		int maxpage = (GameToolbox::getValueForGamemode(_selectedMode) - 1) / 36;

		if (up) {
			_modePages[_selectedMode] = (page >= maxpage) ? 0 : page + 1;
		}
		else {
			_modePages[_selectedMode] = (page <= 0) ? maxpage : page - 1;
		}

		this->setupPage(_selectedMode, _modePages[_selectedMode]);
	};
	
	_leftArrow = MenuItemSpriteExtra::create(arrow1, [onChangePage](Node*) {
		onChangePage(false);
		});
	_leftArrow->setPosition({ bg->getPositionX() - 220, bg->getPositionY() });
	menuArr->addChild(_leftArrow);

	auto arrow2 = Sprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	arrow2->setScale(.8f);
	arrow2->setFlippedX(true);

	_rightArrow = MenuItemSpriteExtra::create(arrow2, [onChangePage](Node*) {
		onChangePage(true);
		});
	_rightArrow->setPosition({ bg->getPositionX() + 220, bg->getPositionY() });
	menuArr->addChild(_rightArrow);

	this->addChild(menuArr);

	_selectSprite = Sprite::createWithSpriteFrameName("GJ_select_001.png");
	_selectSprite->setScale(.9f);
	this->addChild(_selectSprite, 10);

	auto gm = GameManager::getInstance();

	this->setupPage(gm->_mainSelectedMode, -1);
}

void GarageLayer::createIconButton(IconType mode, Menu* parent) 
{
	auto s1 = Sprite::createWithSpriteFrameName(this->getSpriteName(mode, false));
	s1->setScale(.9f);
	auto s2 = Sprite::createWithSpriteFrameName(this->getSpriteName(mode, true));
	s2->setScale(s1->getScale());

	auto item = MenuItemToggler::create(s1, s2, [&](Node* a)
	{
		int tag = a->getTag();
		IconType mode = static_cast<IconType>(tag);
		
		int page = _modePages[mode];

		this->setupPage(mode, page);
	});
	item->setSizeMult(1.2f);
	item->setTag(static_cast<int>(mode));
	parent->addChild(item);

	_tabButtons.pushBack(item);
}


std::string GarageLayer::getSpriteName(IconType mode, bool actived)
{
	const char* name;
	switch (mode)
	{
		case kIconTypeShip: name = "ship"; break;
		case kIconTypeBall: name = "ball"; break;
		case kIconTypeUfo: name = "bird"; break;
		case kIconTypeWave: name = "dart"; break;
		case kIconTypeRobot: name = "robot"; break;
		case kIconTypeSpider: name = "spider"; break;
		case kIconTypeSwing: name = "swing"; break;
		case kIconTypeSpecial: name = "streak"; break;
		case kIconTypeDeathEffect: name = "explosion"; break;
		case kIconTypeCube: 
		default: name = "icon";
	}
	return fmt::format("gj_{}Btn_{}_001.png", name, actived ? "on" : "off");
}

void GarageLayer::setupPage(IconType type, int page)
{
	_iconID = 0;
	auto gm = GameManager::getInstance();
	_selectedMode = type;

	MenuItemToggler* currentButton = nullptr;
	for (auto button : _tabButtons) {

		if (button->getTag() == static_cast<int>(type)) currentButton = button;

		button->toggle(false);
		button->setEnabled(true);
	}

	currentButton->toggle(true);
	currentButton->setEnabled(false);
	
	// Obtenemos el �cono activo y el total de �conos (Traducci�n de activeIconForType y countForType)
	int activeIcon = gm->getSelectedIcon(type);
	int totalIcons = GameToolbox::getValueForGamemode(type);

	// L�gica de p�gina por defecto (-1) extra�da del descompilado de IDA:
	// v3 = floorf((float)((active - 1) / 36));
	if (page == -1)
	{
		float v3 = floorf(static_cast<float>(activeIcon - 1) / 36.0f);
		page = (v3 > 0.0f) ? static_cast<int>(v3) : 0;
	}

	_modePages[_selectedMode] = page;
	//GameToolbox::log("setupPage: Mode {}, Page {} Total: {}", (int)type, page, totalIcons);

	auto size = Director::getInstance()->getWinSize();

	if (_menuIcons) this->removeChild(_menuIcons);

	_menuIcons = Menu::create();
	_menuIcons->setPosition(0, 0);

	if (_selectSprite) _selectSprite->setVisible(false);

	int startIdx = (page * 36) + 1;
	int maxIdx = std::min((page + 1) * 36, totalIcons);

	int col = 0;
	int row = 0;

	int totalPages = (totalIcons == 0) ? 1 : ((totalIcons - 1) / 36) + 1;

	_leftArrow->setVisible(totalPages > 1);
	_rightArrow->setVisible(totalPages > 1);

	for (int i = startIdx; i <= maxIdx; i++)
	{
		auto browserItem = Sprite::createWithSpriteFrameName("playerSquare_001.png");
		browserItem->setOpacity(0);

		if (type == IconType::kIconTypeSpecial)
		{
			auto icono = Sprite::createWithSpriteFrameName(StringUtils::format("player_special_%02d_001.png", i));
			icono->setPosition(browserItem->getContentSize() / 2);
			icono->setScale(27.0f / icono->getContentSize().width);
			browserItem->addChild(icono);
		}
		else if (type == IconType::kIconTypeDeathEffect)
		{
			auto icono = Sprite::createWithSpriteFrameName(StringUtils::format("explosionIcon_%02d_001.png", i));
			icono->setPosition(browserItem->getContentSize() / 2);
			icono->setScale(0.9f);
			browserItem->addChild(icono);
		}
		else
		{
			auto icono = SimplePlayer::create(0);
			icono->updateGamemode(i, type);
			icono->setMainColor({ 175, 175, 175 });

			ax::Vec2 iconPos = browserItem->getContentSize() / 2;
			if (type == IconType::kIconTypeUfo) {
				iconPos.y += 5.0f;
				icono->m_pDomeSprite->setVisible(false);
			}

			// Escala est�ndar. El SimplePlayer se encargar� de achicar los rebeldes por dentro
			float iconScale = 27.0f / icono->m_pMainSprite->getContentSize().width;

			icono->setPosition(iconPos);
			icono->setScale(iconScale);
			browserItem->addChild(icono);
		}

		auto btn = MenuItemSpriteExtra::create(browserItem, [&](Node* a)
			{
				if (a->getTag() != _iconID) {
					_iconID = a->getTag();
				} else {
					GameToolbox::log("Opening info popup {} {}", static_cast<int>(_selectedMode), a->getTag());
				}
				if (_selectedMode != kIconTypeSpecial && _selectedMode != kIconTypeDeathEffect) {
					_iconPrev->updateGamemode(a->getTag(), _selectedMode);
					GameManager::getInstance()->_mainSelectedMode = _selectedMode;
				}
				
				auto gm = GameManager::getInstance();
				gm->setSelectedIcon(_selectedMode, a->getTag());
				_selectSprite->setPosition(a->getPosition());
				_selectSprite->setVisible(true);
			});

		btn->setTag(i);
		btn->setPosition({ size.width / 2 - 165 + (col * 30), size.height / 2 - 65 + 30 - (row * 30) });
		_menuIcons->addChild(btn);
	
		if (i == activeIcon)
		{
			_selectSprite->setPosition(btn->getPosition());
			_selectSprite->setVisible(true);
		}

		col++;
		if (col >= _numPerRow)
		{
			col = 0;
			row++;
		}
	}

	// --- SISTEMA DE PUNTOS DE NAVEGACI�N (NAV DOTS) ---
	if (!_navDotMenu) {
		_navDotMenu = Menu::create();
		this->addChild(_navDotMenu);
	}

	_navDotMenu->removeAllChildren();

	if (totalPages > 1) {
		_navDotMenu->setVisible(true);

		for (int p = 0; p < totalPages; p++) {
			// Si el punto corresponde a la p�gina actual, lo encendemos
			const char* dotName = (p == page) ? "gj_navDotBtn_on_001.png" : "gj_navDotBtn_off_001.png";
			auto dotSprite = Sprite::createWithSpriteFrameName(dotName);

			// dotSprite->setScale(1.1f);

			auto dotBtn = MenuItemSpriteExtra::create(dotSprite, [this, type, p](Node*) {
				_modePages[_selectedMode] = p;
				this->setupPage(type, p);
				});

			_navDotMenu->addChild(dotBtn);
		}

		_navDotMenu->alignItemsHorizontallyWithPadding(6.0f);

		_navDotMenu->setPosition({ size.width / 2, size.height / 2 - 135.0f });
	}
	else {
		_navDotMenu->setVisible(false);
	}

	this->addChild(_menuIcons);
}