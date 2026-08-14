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

#include "LevelSelectLayer.h"
#include "GJGameLevel.h"
#include "MenuItemSpriteExtra.h"
#include "AudioEngine.h"
// #include "PlayLayer.h"

#include "MenuLayer.h"
#include "LevelPage.h"
#include "SongsLayer.h"
//#include "Checkbox.h"
#include "GroundLayer.h"
#include "BoomScrollLayer.h"
#include "2d/Menu.h"
#include "2d/Label.h"
#include "EventListenerKeyboard.h"
#include "2d/Transition.h"
#include "EventDispatcher.h"
#include "base/Director.h"
#include "GameToolbox/log.h"
#include "GameToolbox/getTextureString.h"
#include "GameToolbox/nodes.h"

USING_NS_AX;

ax::Color3B getColorForPage(int page) {
	int colorIdx = page % 9;
	switch (colorIdx) {
	case 0: return { 40, 125, 255 };  // Azul (Stereo Madness)
	case 1: return { 0, 255, 0 };     // Verde
	case 2: return { 0, 255, 255 };   // Cian
	case 3: return { 255, 0, 0 };     // Rojo
	case 4: return { 255, 125, 0 };   // Naranja
	case 5: return { 255, 255, 0 };   // Amarillo
	case 6: return { 125, 0, 255 };   // Morado
	case 7: return { 255, 0, 255 };   // Magenta
	case 8: return { 255, 125, 125 }; // Rosa
	default: return { 40, 125, 255 };
	}
}

Scene* LevelSelectLayer::scene(int page)
{
	auto scene = Scene::create();
	scene->addChild(LevelSelectLayer::create(page));
	return scene;
}

LevelSelectLayer* LevelSelectLayer::create(int page) {
	LevelSelectLayer* pRet = new LevelSelectLayer();
	if (pRet->init(page)) {
		pRet->autorelease();
		return pRet;
	} else {
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}

bool LevelSelectLayer::init(int page)
{

	if(!Layer::init()) return false;

	auto director = Director::getInstance();
	const auto& winSize = director->getWinSize();

	_background = Sprite::create("GJ_gradientBG.png");
	_background->setAnchorPoint({0.0f, 0.0f});
	addChild(_background, -2);

	_background->setScaleX((winSize.width + 10.0f) / _background->getTextureRect().size.width);
	_background->setScaleY((winSize.height + 10.0f) / _background->getTextureRect().size.height);
	_background->setPosition({-5.0f, -5.0f});
	_background->setColor({0x28, 0x7D, 0xFF});
	_ground = GroundLayer::create(1);
	_ground->setPositionY(-25.f);
	addChild(_ground, -1);

	auto topBar = Sprite::createWithSpriteFrameName("GJ_topBar_001.png");
	topBar->setPosition({ winSize.width / 2, winSize.height });
	topBar->setAnchorPoint({ 0.5, 1.0 });
	topBar->setContentSize({ 306.5, 36 });
	addChild(topBar);

	GameToolbox::createCorners(this, false, false, true, true);
	
	constexpr auto levelData = std::to_array<std::tuple<const char*, const char*, int>>({
		{ "Stereo Madness", "RobTop", 1 },
		{ "Back on Track", "RobTop", 2 },
		{ "Polargeist", "RobTop", 3 },
		{ "Dry Out", "RobTop", 4 },
		{ "Base After Base", "RobTop", 5 },
		{ "Cant Let Go", "RobTop", 6 },
		{ "Jumper", "RobTop", 7 },
		{ "Time Machine", "RobTop", 8 },
		{ "Cycles", "RobTop", 9 },
		{ "xStep", "RobTop", 10 },
		{ "Clutterfunk", "RobTop", 11 },
		{ "Theory Of Everything", "RobTop", 12 },
		{ "Electroman Adventures", "RobTop", 13 },
		{ "Clubstep", "RobTop", 14 },
		{ "Electrodynamix", "RobTop", 15 },
		{ "Hexagon Force", "RobTop", 16 },
		{ "Blast Processing", "RobTop", 17 },
		{ "Theory Of Everything 2", "RobTop", 18 },
		{ "Geometrical Dominator", "RobTop", 19 },
		{ "Deadlocked", "RobTop", 20 },
		{ "Fingerdash", "RobTop", 21 },
		{ "Performance Test", "OpenGD", 22 }
	});
	
	//TODO: add getters on level page because they are actually owning the stuff

	std::vector<Layer*> layers;
	layers.reserve(levelData.size());
	
	for (const auto& [name, creator, id] : levelData)
	{
		auto level = GJGameLevel::createWithMinimumData(name, creator, id);
		layers.push_back(LevelPage::create(level));
	}
	_levelPages = layers;
	_bsl = BoomScrollLayer::create(layers, page);
	addChild(_bsl);

	// Aplicamos el color de la página inicial al fondo y al suelo
	auto initialColor = getColorForPage(page);
	_background->setColor(initialColor);

	if (_ground && _ground->_sprite) {
		ax::Color3B groundColor;
		groundColor.r = initialColor.r * 0.8f;
		groundColor.g = initialColor.g * 0.8f;
		groundColor.b = initialColor.b * 0.8f;

		// Aplicamos el color ÚNICAMENTE al sprite del piso
		_ground->_sprite->setColor(groundColor);
	}

	this->schedule([this](float dt) {
		if (!_bsl) return;

		auto internalLayer = _bsl->getChildByTag(1234);
		if (!internalLayer) return;

		auto winSize = ax::Director::getInstance()->getWinSize();
		float scrollX = -internalLayer->getPositionX();
		float exactPage = scrollX / winSize.width;

		// Blindamos las páginas para evitar crasheos en los bordes
		float maxPages = std::max(0, (int)_levelPages.size() - 1);

		// Obtenemos los índices de las páginas sin clampear exactPage aún (para detectar el rebote)
		int page1 = static_cast<int>(std::floor(exactPage));
		int page2 = page1 + 1;

		page1 = std::clamp(page1, 0, (int)maxPages);
		page2 = std::clamp(page2, 0, (int)maxPages);

		// Obtenemos el porcentaje puro de qué tan lejos estamos entre dos páginas (0.0 a 1.0)
		float rawPercentage = exactPage - std::floor(exactPage);

		// --- EL SECRETO DE ROBTOP: LA ZONA MUERTA ---
		// El color solo transiciona en el centro de la pantalla.
		// Esto protege los colores de la animación de rebote elástico.
		float percentage = 0.0f;
		if (rawPercentage > 0.3f && rawPercentage < 0.7f) {
			// Si estamos en el medio, calculamos la mezcla suave
			percentage = (rawPercentage - 0.3f) / 0.4f;
		}
		else if (rawPercentage >= 0.7f) {
			// Si ya pasamos el 70% o estamos rebotando hacia adelante, mostramos 100% el color objetivo
			percentage = 1.0f;
		} // Si es menor a 0.3, percentage se queda en 0.0f (color actual)

		auto color1 = getColorForPage(page1);
		auto color2 = getColorForPage(page2);

		// Interpolación lineal
		ax::Color3B finalColor;
		finalColor.r = color1.r + (color2.r - color1.r) * percentage;
		finalColor.g = color1.g + (color2.g - color1.g) * percentage;
		finalColor.b = color1.b + (color2.b - color1.b) * percentage;

		_background->setColor(finalColor);

		// --- MULTIPLICADOR DE SUELO DE GD ---
		if (_ground) {
			ax::Color3B groundColor;
			groundColor.r = finalColor.r * 0.8f;
			groundColor.g = finalColor.g * 0.8f;
			groundColor.b = finalColor.b * 0.8f;

			// Como sobrescribimos setColor, el GroundLayer sabrá 
			// que solo debe pintar los cuadros y no la línea.
			_ground->setColor(groundColor);
		}

		}, "color_updater");

	auto btnMenu = Menu::create();
	addChild(btnMenu, 5);

	//bool controller = PlatformToolbox::isControllerConnected();
	bool controller = false;

	auto left =
		Sprite::createWithSpriteFrameName(controller ? "controllerBtn_DPad_Left_001.png" : "navArrowBtn_001.png");
	if (!controller) left->setFlippedX(true);

	MenuItemSpriteExtra* leftBtn = MenuItemSpriteExtra::create(left, [this](Node*) {
		_bsl->changePageLeft();
	});
	btnMenu->addChild(leftBtn);

	leftBtn->setPosition(btnMenu->convertToNodeSpace({ 25.0f, winSize.height / 2 }));

	auto right = Sprite::createWithSpriteFrameName(controller ? "controllerBtn_DPad_Right_001.png" : "navArrowBtn_001.png");

	MenuItemSpriteExtra* rightBtn = MenuItemSpriteExtra::create(right, [this](Node*) {
		_bsl->changePageRight();
	});
	btnMenu->addChild(rightBtn);

	rightBtn->setPosition(btnMenu->convertToNodeSpace({ winSize.width - 25.0f, winSize.height / 2 }));

	auto back = Sprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
		MenuItemSpriteExtra* backBtn =
			MenuItemSpriteExtra::create(back, []( Node const* btn) { 
			Director::getInstance()->replaceScene(TransitionFade::create(0.5f, MenuLayer::scene()));
	});
	
	//auto bglCheckbox = Checkbox::create("BaseGameLayer", [this](Node* btn, bool on)
	//{
	//	GameToolbox::log("on: {}", on);
	//	if (auto currentLevelPage = dynamic_cast<LevelPage*>(_bsl->_layers.at(_bsl->_currentPage)))
	//	{
	//		currentLevelPage->_openBGL = on;
	//	}
	//});
	Menu* backMenu = Menu::create();

	addChild(backMenu, 1);
	backMenu->addChild(backBtn);
	//backMenu->addChild(bglCheckbox);
	backMenu->setPosition({25.0f, winSize.height - 22.0f });
	//bglCheckbox->setPosition({backMenu->convertToNodeSpace({winSize.width / 2 + 15.0f, winSize.height - 40.0f})});

	auto infoMenu = Menu::create();
	addChild(infoMenu);

	Sprite* info = Sprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
	MenuItemSpriteExtra* infoBtn = MenuItemSpriteExtra::create(info, [](Node* btn) {

	});
	infoMenu->addChild(infoBtn, 1);

	infoMenu->setPosition({ winSize.width - 20.0f, winSize.height - 20.0f });


	auto dlLabel = Label::createWithBMFont(GameToolbox::getTextureString("bigFont.fnt"), "Download the soundtracks");
	dlLabel->setScale(0.5);

	auto dlMenuItem = MenuItemSpriteExtra::create(dlLabel, [](Node*) {
		SongsLayer::create()->showLayer(true, false);
	});

	auto dlLabelMenu = Menu::create();
	dlLabelMenu->addChild(dlMenuItem);
	addChild(dlLabelMenu);
	dlLabelMenu->setPosition({ winSize.width / 2, 35 });


	auto listener = EventListenerKeyboard::create();

	// int currentlevel = 0;

	listener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event const*)
	{
		if (code == ax::EventKeyboard::KeyCode::KEY_ESCAPE)
		{
			auto scene = MenuLayer::scene();
			Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
			// GameToolbox::popSceneWithTransition(0.5f);
		}
		else if (code == ax::EventKeyboard::KeyCode::KEY_LEFT_ARROW) {
			_bsl->changePageLeft();
		}
		else if (code == ax::EventKeyboard::KeyCode::KEY_RIGHT_ARROW) {
			_bsl->changePageRight();
		}
		else if (code == ax::EventKeyboard::KeyCode::KEY_SPACE || code == ax::EventKeyboard::KeyCode::KEY_ENTER)
		{
			// Evitamos crashear si el usuario presiona espacio mientras se desliza la pantalla
			if (auto currentLevelPage = dynamic_cast<LevelPage*>(_bsl->_layers.at(_bsl->_currentPage)))
			{
				currentLevelPage->onPlay(nullptr);
			}
		}
	};

	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	//if (controller) GameToolbox::addBackButton(this, backBtn);
	return true;
}

void LevelSelectLayer::onExit()
{
	_ground->unscheduleUpdate();

	// Si NO estamos reemplazando la escena, limpiamos los niveles
	if (!LevelPage::replacingScene)
	{
		for (const auto& page : _levelPages)
		{
			if (LevelPage* levelPage = dynamic_cast<LevelPage*>(page))
			{
				if (GJGameLevel* level = levelPage->_level; level) {
					delete level;
				}
			}
		}
	}

	// ¡MUY IMPORTANTE! Esta línea siempre debe ejecutarse al final
	Layer::onExit();
}