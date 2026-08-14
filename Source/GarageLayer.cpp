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
#include "MenuItemSpriteExtra.h"
#include "MenuLayer.h"
#include "SimplePlayer.h"
#include "GameManager.h"
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

USING_NS_AX;

Scene* GarageLayer::scene(bool popSceneWithTransition)
{
	auto s = Scene::create();
	auto garage = GarageLayer::create();
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

	auto line = Sprite::createWithSpriteFrameName("floorLine_001.png");
	line->setBlendFunc(GameToolbox::getBlending());
	line->setPosition({ size.width / 2, size.height / 2 + 50 });
	this->addChild(line);

	_iconPrev = SimplePlayer::create(0); // 132
	_iconPrev->updateGamemode(gm->getSelectedIcon(gm->_mainSelectedMode), gm->_mainSelectedMode);
	_iconPrev->setPosition({line->getPositionX(), line->getPositionY() + 25});
	_iconPrev->setMainColor({125, 0, 255});
	_iconPrev->setSecondaryColor({0, 255, 255});
	_iconPrev->setGlow(true);
	_iconPrev->setGlowColor({0, 255, 0});
	_iconPrev->setScale(1.6f);
	this->addChild(_iconPrev);

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

int GarageLayer::selectedGameModeInt()
{
	if (_selectedMode == IconType::kIconTypeSpecial)
		return 7;
	if (_selectedMode == IconType::kIconTypeDeathEffect)
		return 8;

	return static_cast<int>(_selectedMode);
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

	auto bg = ui::Scale9Sprite::create(GameToolbox::getTextureString("square02_001.png"));
	bg->setContentSize({385, 100});
	bg->setOpacity(75);
	bg->setPosition({size.width / 2, size.height / 2 - 65});
	this->addChild(bg);

	auto unlock = Sprite::createWithSpriteFrameName("GJ_unlockTxt_001.png");
	unlock->setPosition({size.width / 2, bg->getPositionY() + bg->getContentSize().height / 2 + 12});
	this->addChild(unlock);

	auto menu = Menu::create();

	for (int i = 0; i < 9; i++)
	{
		auto s1 = Sprite::createWithSpriteFrameName(this->getSpriteName(i, false));
		s1->setScale(.9f);
		auto s2 = Sprite::createWithSpriteFrameName(this->getSpriteName(i, true));
		s2->setScale(s1->getScale());

		auto i1 = MenuItemSpriteExtra::create(s1, [&](Node* a)
		{
			int tag = a->getTag();
			int page = _modePages[tag];

			IconType mode;
			if (tag == 7)
				mode = IconType::kIconTypeSpecial;
			else if (tag == 8)
				mode = IconType::kIconTypeDeathEffect;
			else
				mode = static_cast<IconType>(tag);

			this->setupPage(mode, page);
		});
		i1->setTag(i);
		menu->addChild(i1);

		//auto i1 = MenuItemToggler::create(s1, s2, this, menu_selector(SaiGarageLayer::onSelectTab));
		//i1->setSizeMult(1.2f);
		//i1->setTag(i);
		//i1->setClickable(false);
		//menu->addChild(i1);
	}

	menu->alignItemsHorizontallyWithPadding(0);
	menu->setPosition({size.width / 2, unlock->getPositionY() + 30 + 3});
	this->addChild(menu);

	auto menuArr = Menu::create();
	menuArr->setPosition({0, 0});

	// Robtop aqui hace otra peruanada de usar "GJ_arrow_%02d_001.png" para las flechas etc...

	auto arrow1 = Sprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	arrow1->setScale(.8f);

	auto onChangePage = [this](bool up)
		{
			int gameMode = selectedGameModeInt();
			int page = this->_modePages[gameMode];

			int maxpage = (GameToolbox::getValueForGamemode(_selectedMode) - 1) / 36;

			if (up) {
				_modePages[gameMode] = (page >= maxpage) ? 0 : page + 1;
			}
			else {
				_modePages[gameMode] = (page <= 0) ? maxpage : page - 1;
			}

			// ELIMINAMOS la línea de GameToolbox::log que causa la excepción de fmt
			this->setupPage(_selectedMode, _modePages[gameMode]);
		};

	// Botón Izquierdo (capturando onChangePage por valor para evitar el crash)
	auto arrowLeftBtn = MenuItemSpriteExtra::create(arrow1, [onChangePage](Node*) {
		onChangePage(false);
		});
	arrowLeftBtn->setPosition({ bg->getPositionX() - 220, bg->getPositionY() });
	menuArr->addChild(arrowLeftBtn);

	auto arrow2 = Sprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	arrow2->setScale(.8f);
	arrow2->setFlippedX(true);

	// Botón Derecho (capturando onChangePage por valor para evitar el crash)
	auto arrowRightBtn = MenuItemSpriteExtra::create(arrow2, [onChangePage](Node*) {
		onChangePage(true);
		});
	arrowRightBtn->setPosition({ bg->getPositionX() + 220, bg->getPositionY() });
	menuArr->addChild(arrowRightBtn);

	this->addChild(menuArr);

	_selectSprite = Sprite::createWithSpriteFrameName("GJ_select_001.png");
	_selectSprite->setScale(.9f);
	this->addChild(_selectSprite, 10);

	auto gm = GameManager::getInstance();

	// Pasamos -1 y dejamos que setupPage aplique la matemática original de RobTop
	this->setupPage(gm->_mainSelectedMode, -1);
}

const char* GarageLayer::getSpriteName(int id, bool actived)
{
	switch (id)
	{
		case 0: return actived ? "gj_iconBtn_on_001.png" : "gj_iconBtn_off_001.png";
		case 1:	return actived ? "gj_shipBtn_on_001.png" : "gj_shipBtn_off_001.png";
		case 2: return actived ? "gj_ballBtn_on_001.png" : "gj_ballBtn_off_001.png";
		case 3: return actived ? "gj_birdBtn_on_001.png" : "gj_birdBtn_off_001.png";
		case 4: return actived ? "gj_dartBtn_on_001.png" : "gj_dartBtn_off_001.png";
		case 5: return actived ? "gj_robotBtn_on_001.png" : "gj_robotBtn_off_001.png";
		case 6: return actived ? "gj_spiderBtn_on_001.png" : "gj_spiderBtn_off_001.png";
		case 7: return actived ? "gj_streakBtn_on_001.png" : "gj_streakBtn_off_001.png";
		case 8: return actived ? "gj_explosionBtn_on_001.png" : "gj_explosionBtn_off_001.png";
	}
	return nullptr;
}

void GarageLayer::setupPage(IconType type, int page)
{
	auto gm = GameManager::getInstance();
	_selectedMode = type;

	// Obtenemos el ícono activo y el total de íconos (Traducción de activeIconForType y countForType)
	int activeIcon = gm->getSelectedIcon(type);
	int totalIcons = GameToolbox::getValueForGamemode(type);

	// Lógica de página por defecto (-1) extraída del descompilado de IDA:
	// v3 = floorf((float)((active - 1) / 36));
	if (page == -1)
	{
		float v3 = floorf(static_cast<float>(activeIcon - 1) / 36.0f);
		page = (v3 > 0.0f) ? static_cast<int>(v3) : 0;
	}

	// Guardamos la página actual en nuestro registro
	_modePages[selectedGameModeInt()] = page;
	GameToolbox::log("setupPage: Mode {}, Page {}", (int)type, page);

	auto size = Director::getInstance()->getWinSize();

	if (_menuIcons)
		this->removeChild(_menuIcons);

	_menuIcons = Menu::create();
	_menuIcons->setPosition(0, 0);

	if (_selectSprite != nullptr)
		_selectSprite->setVisible(false);

	// Límites exactos de la página actual
	int startIdx = (page * 36) + 1;
	int maxIdx = std::min((page + 1) * 36, totalIcons);

	// Variables de cuadrícula (Corregido para que no se desfase en páginas > 0)
	int col = 0;
	int row = 0;

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

			// Escala estándar. El SimplePlayer se encargará de achicar los rebeldes por dentro
			float iconScale = 27.0f / icono->m_pMainSprite->getContentSize().width;

			icono->setPosition(iconPos);
			icono->setScale(iconScale);
			browserItem->addChild(icono);
		}

		auto btn = MenuItemSpriteExtra::create(browserItem, [&](Node* a)
			{
				_iconPrev->updateGamemode(a->getTag(), _selectedMode);
				auto gm = GameManager::getInstance();
				gm->setSelectedIcon(_selectedMode, a->getTag());
				gm->_mainSelectedMode = _selectedMode;
				_selectSprite->setPosition(a->getPosition());
				_selectSprite->setVisible(true);
			});

		btn->setTag(i);
		btn->setPosition({ size.width / 2 - 165 + (col * 30), size.height / 2 - 65 + 30 - (row * 30) });
		_menuIcons->addChild(btn);

		// --- SISTEMA DE PUNTOS DE NAVEGACIÓN (NAV DOTS) ---
		if (!_navDotMenu) {
			_navDotMenu = Menu::create();
			this->addChild(_navDotMenu);
		}

		_navDotMenu->removeAllChildren(); // Limpiamos los puntos de la pestaña anterior

		// Calculamos el total de páginas (redondeando hacia arriba)
		int totalPages = (totalIcons == 0) ? 1 : ((totalIcons - 1) / 36) + 1;

		// Si hay más de una página (ej. Cubos, Naves), mostramos los puntos
		if (totalPages > 1) {
			_navDotMenu->setVisible(true);

			for (int p = 0; p < totalPages; p++) {
				// Si el punto corresponde a la página actual, lo encendemos
				const char* dotName = (p == page) ? "gj_navDotBtn_on_001.png" : "gj_navDotBtn_off_001.png";
				auto dotSprite = Sprite::createWithSpriteFrameName(dotName);

				// Si quieres que resalte un poco más, RobTop a veces los escala ligeramente
				// dotSprite->setScale(1.1f);

				// Capturamos 'p' (la página del punto) y 'type' por valor para memoria segura
				auto dotBtn = MenuItemSpriteExtra::create(dotSprite, [this, type, p](Node*) {
					_modePages[selectedGameModeInt()] = p; // Sincronizamos la memoria de la página
					this->setupPage(type, p);             // Recargamos el garaje en esa página
					});

				_navDotMenu->addChild(dotBtn);
			}

			// RobTop alinea estos puntos horizontalmente con un espacio exacto de 6.0f
			_navDotMenu->alignItemsHorizontallyWithPadding(6.0f);

			// Los posicionamos justo debajo del recuadro negro de los iconos
			_navDotMenu->setPosition({ size.width / 2, size.height / 2 - 135.0f });
		}
		// Si solo hay una página (ej. Robot, Araña), ocultamos los puntos
		else {
			_navDotMenu->setVisible(false);
		}
		// --------------------------------------------------

		// Traducción del puntero m_cursor1 del ensamblador
		if (i == activeIcon)
		{
			_selectSprite->setPosition(btn->getPosition());
			_selectSprite->setVisible(true);
		}

		// Avanzamos en la cuadrícula
		col++;
		if (col >= _numPerRow)
		{
			col = 0;
			row++;
		}
	}

	this->addChild(_menuIcons);
}