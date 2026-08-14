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

#include "PlayLayer.h"
#include "AudioEngine.h"
#include "FMODAudioEngine.h"
#include "CreatorLayer.h"
#include "EffectGameObject.h"
#include "EndLevelLayer.h"
#include "PauseLayer.h"

#include "LevelInfoLayer.h"
#include "LevelPage.h"
#include "LevelSelectLayer.h"
#include "LevelTools.h"
#include "MenuItemSpriteExtra.h"

#include "ImGui/ImGuiPresenter.h"
#include "ImGui/imgui/imgui.h"

#include "external/benchmark.h"
#include "external/json.hpp"
#include "external/constants.h"

#include "LevelDebugLayer.h"
#include "UILayer.h"
#include "GJGameLevel.h"
#include "GroundLayer.h"
#include "SimpleProgressBar.h"
#include "CircleWave.h"
#include "GameToolbox/log.h"
#include "GameToolbox/getTextureString.h"
#include "GameToolbox/rand.h"
#include "GameToolbox/math.h"
#include "GameToolbox/conv.h"
#include "GameToolbox/nodes.h"

#include "2d/Transition.h"
#include "2d/Camera.h"
#include "2d/ParticleSystemQuad.h"
#include "2d/ActionTween.h"
#include "2d/ActionEase.h"
#include "2d/ActionInstant.h"
#include "2d/Label.h"
#include "2d/SpriteFrameCache.h"
#include "2d/Menu.h"
#include "base/EventListenerKeyboard.h"
#include "base/EventDispatcher.h"
#include <thread>
#include <fmt/format.h>
#include <cstdio>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// Logger de diagnostico temporal: escribe a stderr y fuerza flush inmediato.
template <typename... Args>
static void DBG_LOG(fmt::format_string<Args...> fmtStr, Args&&... args)
{
	std::string msg = fmt::format(fmtStr, std::forward<Args>(args)...);
	// Escribimos en TODOS los canales posibles para descartar de una vez
	// por todas un problema de redireccion de stdout/stderr/consola.
	fputs(msg.c_str(), stdout);
	fputc('\n', stdout);
	fflush(stdout);

	fputs(msg.c_str(), stderr);
	fputc('\n', stderr);
	fflush(stderr);

#ifdef _WIN32
	OutputDebugStringA(msg.c_str());
	OutputDebugStringA("\n");
#endif
}
USING_NS_AX;
USING_NS_AX_EXT;

// Estas variables se usan desde el menu de debug de ImGui (showDn, noclip,
// gameSpeed, fps, fullscreen, monitorN). En el refactor solo quedaron las
// declaraciones "extern" pero se perdio la definicion real (el storage),
// por eso el linker no las encontraba (LNK2001). Las restauro aqui igual
// que en el PlayLayer_opengd.cpp original.
bool showDn = false, noclip = false;

float gameSpeedPlayer = 1, fps = 0;

bool fullscreen = false;
int monitorN = 0;

static PlayLayer* Instance = nullptr;

Scene* PlayLayer::scene(GJGameLevel* level)
{
	auto scene = Scene::create();
	scene->addChild(PlayLayer::create(level));
	return scene;
}

void PlayLayer::showCompleteText()
{
	m_bEndAnimation = true;

	auto size = Director::getInstance()->getWinSize();

	float scale = 1.1f;
	const char* spr = "GJ_levelComplete_001.png";

	auto sprite = Sprite::createWithSpriteFrameName(spr);
	sprite->setScale(0.01f);
	sprite->setPosition({ size.width / 2, size.height / 2 + 35 });
	m_pHudLayer->addChild(sprite);

	sprite->runAction(Sequence::create(EaseElasticOut::create(ScaleTo::create(0.66f, scale), 0.6),
		DelayTime::create(0.88f), EaseIn::create(ScaleTo::create(0.22f, 0), 2.0f),
		RemoveSelf::create(true), nullptr));

	auto col1 = _player1->getMainColor();
	auto col2 = _player1->getSecondaryColor();

	auto par1 = ParticleSystemQuad::create("levelComplete01.plist");
	par1->setPosition(sprite->getPosition());
	par1->setStartColor({ (GLfloat)col1.r, (GLfloat)col1.g, (GLfloat)col1.b, 255 });
	par1->setEndColor({ (GLfloat)col1.r, (GLfloat)col1.g, (GLfloat)col1.b, 0 });
	m_pHudLayer->addChild(par1, -1);

	auto par2 = ParticleSystemQuad::create("levelComplete01.plist");
	par2->setPosition(par1->getPosition());
	par2->setStartColor({ (GLfloat)col2.r, (GLfloat)col2.g, (GLfloat)col2.b, 255 });
	par2->setEndColor({ (GLfloat)col2.r, (GLfloat)col2.g, (GLfloat)col2.b, 0 });
	m_pHudLayer->addChild(par2, -1);

	auto cir = CircleWave::create(0.8f, { col1.r, col1.g, col1.b, 255 }, 5.f, size.width - 10, true, false);
	cir->setPosition({ size.width - 10, size.height / 2 });
	m_pHudLayer->addChild(cir, -1);

	auto cir2 = CircleWave::create(0.8f, { col1.r, col1.g, col1.b, 255 }, 5.f, 250.0f, true, false);
	cir2->setPosition(sprite->getPosition());
	m_pHudLayer->addChild(cir2, -1);

	for (int i = 0; i < 9; i++)
	{
		scheduleOnce([&](float d) { spawnCircle(); }, 0.16f * i, "playlayer_circles");
	}
	scheduleOnce([&](float d) { showEndLayer(); }, 1.5f, "playlayer_levelend");
}

void PlayLayer::spawnCircle()
{
	auto size = Director::getInstance()->getWinSize();

	auto minArea = Vec2({ 40, 70 });
	auto maxArea = Vec2({ size.width - 40, size.height - 70 });

	float x = ((float)rand() / (float)RAND_MAX) * (maxArea.x - minArea.x) + minArea.x;
	float y = ((float)rand() / (float)RAND_MAX) * (maxArea.y - minArea.y) + minArea.y;

	auto col1 = _player1->getMainColor();
	auto cir = CircleWave::create(0.5f, { col1.r, col1.g, col1.b, 255 }, 5.f, 50, true, false);
	cir->setPosition({ x, y });
	m_pHudLayer->addChild(cir, -1);
}

void PlayLayer::showEndLayer()
{
	createLevelEnd();
}

int PlayLayer::sectionForPos(float x)
{
	int section = x / 100;
	if (section < 0) section = 0;
	return section;
}

PlayLayer* PlayLayer::create(GJGameLevel* level)
{
	auto ret = new (std::nothrow) PlayLayer();
	if (ret && ret->init(level))
	{
		ret->autorelease();
		return ret;
	}

	AX_SAFE_DELETE(ret);
	return nullptr;
}

void PlayLayer::setInstance() {
	Instance = this;
	_instance = this;
}

bool PlayLayer::init(GJGameLevel* level)
{
	DBG_LOG("PlayLayer::init - ENTRANDO, llamando BaseGameLayer::init");

	// 1. Cargamos TODO EL NIVEL usando la descompilacion base oficial.
	if (!BaseGameLayer::init(level))
	{
		DBG_LOG("PlayLayer::init - BaseGameLayer::init devolvio false, ABORTANDO");
		return false;
	}

	DBG_LOG("PlayLayer::init - BaseGameLayer::init OK, continuando");

	setInstance();

	auto winSize = Director::getInstance()->getWinSize();

	dn = ax::DrawNode::create();
	dn->setPosition({ -15, -15 });
	addChild(dn, 99999);

	// 2. Creacion de Jugadores
	this->_player1 = PlayerObject::create(1, this);
	this->_player1->setPosition({ -22, 105 });
	// NO usar _main2BatchNode->addChild aqui: el Sprite base del PlayerObject
	// nunca recibe una textura real (GameObject::init hace return temprano
	// para frames que contienen "player"), y SpriteBatchNode::addChild
	// exige que sprite->getTexture() == atlas->getTexture(), lo cual
	// revienta el assert. Sus graficos reales vienen de sus sprites hijos
	// (m_pMainSprite, etc.), que ya tienen textura propia y se ven bien
	// como hijos de un Node normal.
	this->addChild(this->_player1, 3);
	this->_player1->setAnchorPoint({ 0, 0 });
	_player1->setMainColor({ 125, 255, 0 });
	_player1->setSecondaryColor({ 0, 255, 255 });

	this->_player2 = PlayerObject::create(1, this);
	this->_player2->setPosition({ -22, 105 });
	this->addChild(this->_player2, 3);
	this->_player2->setAnchorPoint({ 0, 0 });
	_player2->setMainColor({ 125, 255, 0 });
	_player2->setSecondaryColor({ 0, 255, 255 });
	_player2->setVisible(false);

	// 3. Creacion de Suelo y Cielo
	int groundID = _levelSettings._groundID > 0 ? _levelSettings._groundID : 1;
	this->_bottomGround = GroundLayer::create(groundID);
	this->_ceiling = GroundLayer::create(groundID);

	// FIX 3: Apagamos el update global aquí. 
	// Así evitamos que PlayLayer mueva el suelo dos veces por frame.
	this->_bottomGround->unscheduleUpdate();
	this->_ceiling->unscheduleUpdate();

	cameraFollow = ax::Node::create();
	cameraFollow->addChild(this->_bottomGround, 1);
	cameraFollow->addChild(this->_ceiling, 1);
	this->addChild(cameraFollow, 100);

	this->_ceiling->setScaleY(-1);
	_ceiling->setVisible(false);
	_bottomGround->setPositionY(-cameraFollow->getPositionY() + 12);
	_ceiling->setPositionY(winSize.height + _ceiling->_sprite->getTextureRect().size.height);

	if (this->_colorChannels.contains(1001) && _bottomGround && _bottomGround->_sprite) {
		_bottomGround->_sprite->setColor(this->_colorChannels.at(1001)._color);
		if (_ceiling && _ceiling->_sprite) _ceiling->_sprite->setColor(this->_colorChannels.at(1001)._color);
	}
	else if (_bottomGround && _bottomGround->_sprite) {
		// Color azul por defecto de GD por si el nivel no especifica color de suelo
		_bottomGround->_sprite->setColor(ax::Color3B(0, 102, 255));
		if (_ceiling && _ceiling->_sprite) _ceiling->_sprite->setColor(ax::Color3B(0, 102, 255));
	}

	// 4. Creacion del Fondo
	int bgID = _levelSettings._bgID > 0 ? _levelSettings._bgID : 1;
	this->m_pBG = Sprite::create(GameToolbox::getTextureString(fmt::format("game_bg_{:02d}_001.png", bgID)));
	if (!this->m_pBG) this->m_pBG = Sprite::create(GameToolbox::getTextureString("game_bg_01_001.png"));

	if (m_pBG) {
		m_pBG->setStretchEnabled(false);
		const Texture2D::TexParams texParams = { backend::SamplerFilter::LINEAR, backend::SamplerFilter::LINEAR,
												backend::SamplerAddressMode::REPEAT, backend::SamplerAddressMode::REPEAT };
		this->m_pBG->getTexture()->setTexParameters(texParams);
		this->m_pBG->setTextureRect(Rect(0, 0, 1024 * 5, 1024));
		this->m_pBG->setPosition(winSize.x / 2, winSize.y / 4);
		this->addChild(this->m_pBG, -100);

		if (this->_colorChannels.contains(1000)) {
			this->m_pBG->setColor(this->_colorChannels.at(1000)._color);
		}
	}

	// 5. Calcula la barra de progreso usando los objetos ya parseados por BaseGameLayer
	this->m_lastObjXPos = 570.0f;
	for (auto obj : _allObjects) {
		if (obj && obj->getPositionX() > m_lastObjXPos) {
			m_lastObjXPos = obj->getPositionX();
		}
	}

	// 6. Creacion Interfaz HUD
	m_pHudLayer = UILayer::create();
	m_pBar = SimpleProgressBar::create();
	m_pBar->setPercentage(0.f);
	m_pBar->setPosition({ winSize.width / 2, winSize.height - 10 });
	m_pHudLayer->addChild(m_pBar);

	m_pPercentage = Label::createWithBMFont(GameToolbox::getTextureString("bigFont.fnt"), "0%");
	m_pPercentage->setScale(.5f);
	m_pPercentage->setAnchorPoint({ 0, .5f });
	m_pPercentage->setPosition({ m_pBar->getPositionX() + 110, m_pBar->getPositionY() + 1 });
	m_pHudLayer->addChild(m_pPercentage);

	// 1. Instanciar el bot�n de pausa
	auto pauseSpr = Sprite::createWithSpriteFrameName("GJ_pauseBtn_clean_001.png");
	auto pauseBtn = MenuItemSpriteExtra::create(pauseSpr, [this](Node* sender) {
		this->pauseGame();
		});

	m_closeMenu = Menu::create(pauseBtn, nullptr);
	m_closeMenu->setOpacity(75);
	m_closeMenu->setPosition({ winSize.width - 15, winSize.height - 15 });
	m_closeMenu->setEnabled(false);
	m_pHudLayer->addChild(m_closeMenu);

	this->addChild(m_pHudLayer, 1000);

	// Mantener la l�gica de congelamiento inicial
	m_freezePlayer = true;
	updateVisibility();

	return true;
}

void PlayLayer::addObject(GameObject* obj)
{
	int section = sectionForPos(obj->getPositionX());
	obj->setDontTransform(false);
	obj->_uniqueID = _allObjects.size();

	if (obj->getPositionX() > m_lastObjXPos) {
		m_lastObjXPos = obj->getPositionX();
	}

	_allObjects.push_back(obj);

	while (_sectionObjects.size() <= section) {
		_sectionObjects.push_back(std::vector<GameObject*>());
	}
	_sectionObjects[section].push_back(obj);
}

void PlayLayer::createLevelEnd()
{
	_jumps = _player1->_jumpedTimes;
	auto levelend = EndLevelLayer::create(this);
	addChild(levelend);
}

void PlayLayer::incrementTime()
{
	// Detiene el reloj anterior y arranca uno nuevo limpio
	this->unschedule("playlayer_stopwatch");
	this->schedule([this](float d) {
		_secondsSinceStart++;
		}, 1.0f, "playlayer_stopwatch");
}

double PlayLayer::getModifiedDelta(float dt)
{
	if (m_resumeTimer > 0) {
		m_resumeTimer--;
		dt = 0.0f;
	}

	float timeWarp = 1.0f;
	if (_levelSettings.speed == 1) timeWarp = 0.7f;
	else if (_levelSettings.speed == 2) timeWarp = 1.1f;
	else if (_levelSettings.speed == 3) timeWarp = 1.3f;
	else if (_levelSettings.speed == 4) timeWarp = 1.6f;

	double baseTick = std::min(1.0f, timeWarp) * 0.004166666666666667;
	double totalDelta = dt + m_extraDelta;
	double result = std::round(totalDelta / baseTick) * baseTick;
	m_extraDelta = totalDelta - result;
	return result;
}

void PlayLayer::update(float dt)
{
	if (m_freezePlayer || _isPaused)
	{
		FMODAudioEngine::getInstance()->setPaused(true);
		return;
	}
	else
	{
		FMODAudioEngine::getInstance()->setPaused(false);
	}

	double modifiedDelta = getModifiedDelta(dt);
	const double delta60 = modifiedDelta * 60.0;
	const double delta240 = delta60 * 4.0;
	const float newStepCount = delta240 < 0.0 ? std::ceil(delta240 - 0.5) : std::floor(delta240 + 0.5);
	int subStepCount = 4;
	if (newStepCount >= 4.0)
	{
		subStepCount = static_cast<int>(newStepCount);
	}
	const double stepDelta = modifiedDelta / (double)subStepCount;

	_player1->m_bIsPlatformer = m_platformerMode;
	_player1->noclip = noclip;
	_player2->noclip = noclip;

	auto winSize = Director::getInstance()->getWinSize();

	if (this->_colorChannels.contains(1005)) this->_colorChannels.at(1005)._color = this->_player1->getMainColor();
	if (this->_colorChannels.contains(1006)) this->_colorChannels.at(1006)._color = this->_player1->getSecondaryColor();

	// Actualizacion Color Fondo y Suelo Dinamicamente
	if (_colorChannels.contains(1000) && m_pBG) m_pBG->setColor(_colorChannels[1000]._color);
	if (_colorChannels.contains(1001) && _bottomGround && _bottomGround->_sprite) {
		_bottomGround->_sprite->setColor(_colorChannels[1001]._color);
		if (_ceiling && _ceiling->_sprite) _ceiling->_sprite->setColor(_colorChannels[1001]._color);
	}

	_colorChannels[1007]._color = getLightBG();

	if (!m_freezePlayer && (!this->_player1->isDead() || (_isDualMode && !this->_player2->isDead())))
	{
		for (int i = 0; i < subStepCount; i++)
		{
			float step = stepDelta * 60.0f;

			this->_player1->update(step);
			_player1->setOuterBounds(Rect(_player1->getPosition(), Vec2(30, 30)));
			_player1->setInnerBounds(Rect(_player1->getPosition() + Vec2(11.25f, 11.25f), Vec2(7.5, 7.5)));
			this->checkCollisions(_player1, step);

			if (this->_player1->isDead()) break;

			if (_isDualMode)
			{
				this->_player2->update(step);
				_player2->setOuterBounds(Rect(_player2->getPosition(), Vec2(30, 30)));
				_player2->setInnerBounds(Rect(_player2->getPosition() + Vec2(11.25f, 11.25f), Vec2(7.5, 7.5)));
				this->checkCollisions(_player2, step);

				if (this->_player2->isDead()) break;
			}
		}
	}

	m_pBar->setPercentage(_player1->getPositionX() / this->m_lastObjXPos * 100.f);
	float val = m_pBar->getPercentage();
	m_pPercentage->setString(fmt::format("{:.02f}%", val > 100 ? 100 : val < 0 ? 0 : val));

	if (val >= 100 && !m_bEndAnimation)
		this->showCompleteText();

	this->updateVisibility();
	this->updateCamera(modifiedDelta * 60.0f);
	this->updateTriggers(modifiedDelta * 60.0f);

	if (_player1->_currentGamemode == PlayerGamemodeShip)
		_player1->updateShipRotation(modifiedDelta * 60.0f);
	if (_isDualMode && _player2->_currentGamemode == PlayerGamemodeShip)
		_player2->updateShipRotation(modifiedDelta * 60.0f);

	_colorChannels[1005]._color = _player1->getMainColor();
	_colorChannels[1006]._color = _player1->getSecondaryColor();
}

ax::Color3B PlayLayer::getLightBG()
{
	return _colorChannels[1000]._color;
}

void PlayLayer::destroyPlayer(PlayerObject* player, GameObject* hazard)
{
	destroyPlayer(player);
}

void PlayLayer::destroyPlayer(PlayerObject* player)
{
	if (player->isDead() || player->noclip)
		return;

	player->setIsDead(true);
	player->playDeathEffect();
	player->stopRotation();
	player->setVisible(false);

	if (m_closeMenu) m_closeMenu->setEnabled(false);

	FMODAudioEngine::getInstance()->stopAllMusic();

	// Este scheduleOnce es 100% seguro siempre y cuando NO intentemos destruirlo desde adentro de resetLevel
	this->scheduleOnce([this](float d) {
		this->resetLevel();
		}, 1.0f, "playlayer_restart");
}

void PlayLayer::updateCamera(float dt)
{
	auto winSize = ax::Director::getInstance()->getWinSize();
	if (!_player1) return;

	ax::Vec2 cam = m_obCamPos;
	PlayerObject* player = _player1;
	ax::Vec2 pPos = player->getPosition();

	if (player->_currentGamemode != PlayerGamemodeCube || _isDualMode)
	{
		cam.y = (winSize.height * -0.5f) + m_fCameraYCenter;
		if (cam.y <= 0.0f) cam.y = 0.0f;
	}
	else
	{
		float topOffset = 90.0f;
		float bottomOffset = 120.0f;
		if (player->isGravityFlipped()) {
			topOffset = 120.0f;
			bottomOffset = 90.0f;
		}

		if (pPos.y <= winSize.height + cam.y - topOffset) {
			if (pPos.y < bottomOffset + cam.y) cam.y = pPos.y - bottomOffset;
		}
		else {
			cam.y = pPos.y - winSize.height + topOffset;
		}

		if (!player->isGravityFlipped()) {
			ax::Vec2 lastGroundPos = player->getLastGroundPos();
			if (lastGroundPos.y == 105.f && pPos.y <= cam.y + winSize.height - topOffset) {
				cam.y = 0.0f;
			}
		}
	}

	cam.y = ax::clampf(cam.y, 0.0f, m_maxGameplayY - winSize.height);

	if (pPos.x >= winSize.width / 2.5f && !player->isDead() && !player->m_bIsPlatformer)
	{
		float speedMod = player->getPlayerSpeed() * _bottomGround->getSpeed() * 0.1175f;
		this->m_pBG->setPositionX(this->m_pBG->getPositionX() - dt * speedMod);
		_bottomGround->setSpeed(player->m_dXVel);
		_ceiling->setSpeed(player->m_dXVel);
		_bottomGround->update(dt * player->getPlayerSpeed());
		_ceiling->update(dt * player->getPlayerSpeed());
		cam.x = pPos.x - (winSize.width / 2.5f);
	}
	else if (player->m_bIsPlatformer) {
		cam.x = pPos.x - winSize.width / 2.f;
	}

	if (this->m_pBG->getPosition().x <= cam.x - 1024.f)
		this->m_pBG->setPositionX(this->m_pBG->getPositionX() + 1024.f);

	this->m_pBG->setPositionX(this->m_pBG->getPositionX() + (cam.x - m_obCamPos.x));

	if (!this->m_bMoveCameraX) m_obCamPos.x = cam.x;
	if (!this->m_bMoveCameraY && cam.x != 0) {
		m_obCamPos.y = GameToolbox::iLerp(m_obCamPos.y, cam.y, 0.1f, dt / 60.f);
	}
	else {
		m_obCamPos.y = cam.y;
	}

	ax::Camera::getDefaultCamera()->setPosition(this->m_obCamPos + winSize / 2);
	cameraFollow->setPosition(m_obCamPos);
	_ceiling->setVisible(player->_currentGamemode != PlayerGamemodeCube);

	if (player->_currentGamemode == PlayerGamemodeCube && _bottomGround->getNumberOfRunningActions() == 0)
		_bottomGround->setPositionY(-cameraFollow->getPositionY() + 12);

	if (m_pHudLayer) m_pHudLayer->setPosition(this->m_obCamPos);
}

float PlayLayer::getRelativeMod(Vec2 pos, float v1, float v2, float v3)
{
	auto winSize = ax::Director::getInstance()->getWinSize();
	float camX = m_obCamPos.x;
	float centerX = winSize.width / 2.f;
	float camXCenter = camX + centerX;
	float posX = pos.x;

	float vv1;
	float vv2;
	float vv3;
	float result;

	if (posX <= camXCenter)
	{
		vv2 = v2;
		vv3 = (camXCenter - posX) - v3;
	}
	else
	{
		vv1 = ((posX - v3) - camX) - centerX;
		vv2 = v1;
		vv3 = vv1;
	}
	if (vv2 < 1.f)
		vv2 = 1.f;

	result = (centerX - vv3) / vv2;

	return result;
}

void PlayLayer::applyEnterEffect(GameObject* obj)
{
	if (obj->getEnterEffectID() != _enterEffectID)
		obj->setEnterEffectID(_enterEffectID);
	Vec2 objStartPos = obj->getStartPosition();
	Vec2 objStartScale = obj->getStartScale();
	float rModn = getRelativeMod(objStartPos, 60.f, 60.f, 0.f);
	float rMod = clampf(rModn, 0.f, 1.f);

	switch (obj->getEnterEffectID())
	{
	case 2:
		if (obj->getGameObjectType() != kGameObjectTypeYellowJumpPad)
		{
			obj->setScaleX(rMod * objStartScale.x);
			obj->setScaleY(rMod * objStartScale.y);
		}
		break;
	case 3:
		if (obj->getGameObjectType() != kGameObjectTypeYellowJumpPad)
		{
			obj->setScaleX((2.f - rMod) * objStartScale.x);
			obj->setScaleY((2.f - rMod) * objStartScale.y);
		}
		break;
	case 4:
		if (obj->getGameObjectType() != kGameObjectTypeYellowJumpPad)
			obj->setPositionY((1.0 - rMod) * 100.f + objStartPos.y);
		break;
	case 5:
		if (obj->getGameObjectType() != kGameObjectTypeYellowJumpPad)
			obj->setPositionY((1.0 - rMod) * -100.f + objStartPos.y);
		break;
	case 6:
		if (obj->getGameObjectType() != kGameObjectTypeYellowJumpPad)
			obj->setPositionX((1.0 - rMod) * -100.f + objStartPos.x);
		break;
	case 7:
		if (obj->getGameObjectType() != kGameObjectTypeYellowJumpPad)
			obj->setPositionX((1.0 - rMod) * 100.f + objStartPos.x);
		break;
	default:
		obj->setPosition(objStartPos);
		break;
	}
	obj->setEnterEffectID(0);
}

void PlayLayer::updateVisibility()
{
	auto winSize = ax::Director::getInstance()->getWinSize();
	auto cam = ax::Camera::getDefaultCamera();
	if (!cam) return;

	auto camPos = cam->getPosition();

	float unk = 70.0f;

	int prevSection = floorf((camPos.x - (winSize.width / 2)) / 100) - 1;
	int nextSection = ceilf((camPos.x + winSize.width) / 100) + 1.0f;

	for (int i = prevSection; i < nextSection; i++)
	{
		if (i >= 0)
		{
			if (i < _sectionObjects.size())
			{
				auto section = _sectionObjects[i];
				for (size_t j = 0; j < section.size(); j++)
				{
					GameObject* obj = section[j];
					if (!obj) continue;

					if (obj->getParent() == nullptr)
					{
						if (obj->_particle)
						{
							addChild(obj->_particle);
							AX_SAFE_RELEASE(obj->_particle);
						}
						if (obj->_glowSprite)
						{
							_glowBatchNode->addChild(obj->_glowSprite);
							AX_SAFE_RELEASE(obj->_glowSprite);
						}

						bool added = false;
						if (isObjectBlending(obj))
						{
							switch (obj->_zLayer)
							{
							case -3: _blendingBatchNodeB4->addChild(obj); added = true; break;
							case -1: _blendingBatchNodeB3->addChild(obj); added = true; break;
							case 1:  _blendingBatchNodeB2->addChild(obj); added = true; break;
							case 3:  _blendingBatchNodeB1->addChild(obj); added = true; break;
							default:
							case 5:  _blendingBatchNodeT1->addChild(obj); added = true; break;
							case 7:  _blendingBatchNodeT2->addChild(obj); added = true; break;
							case 9:  _blendingBatchNodeT3->addChild(obj); added = true; break;
							}
						}
						else
						{
							if (obj->_texturePath == _mainBatchNodeTexture)
							{
								switch (obj->_zLayer)
								{
								case -3: _mainBatchNodeB4->addChild(obj); added = true; break;
								case -1: _mainBatchNodeB3->addChild(obj); added = true; break;
								case 1:  _mainBatchNodeB2->addChild(obj); added = true; break;
								case 3:  _mainBatchNodeB1->addChild(obj); added = true; break;
								default:
								case 5:  _mainBatchNodeT1->addChild(obj); added = true; break;
								case 7:  _mainBatchNodeT2->addChild(obj); added = true; break;
								case 9:  _mainBatchNodeT3->addChild(obj); added = true; break;
								}
							}
							else if (obj->_texturePath == _main2BatchNodeTexture) {
								_main2BatchNode->addChild(obj);
								added = true;
							}
						}

						// Fallback de seguridad para asegurar que el bloque aparezca en pantalla.
						if (!added) {
							this->addChild(obj, obj->getGlobalZOrder());
						}
						AX_SAFE_RELEASE(obj);
					}

					obj->setActive(true);
					obj->update();

					float unk2 = 0.0f;
					if (obj->getGameObjectType() == kGameObjectTypeDecoration)
						unk2 = obj->getTextureRect().origin.x * abs(obj->getScaleX()) * 0.4f;

					float opacity = clampf(getRelativeMod(obj->getPosition(), 70.f, 70.f, unk2), 0.f, 1.f);
					if (!obj->getDontTransform())
					{
						obj->_effectOpacityMultipler = opacity;
						this->applyEnterEffect(obj);
					}
				}
			}
		}
	}

	if (_prevSection - 1 >= 0 && _sectionObjects.size() != 0 && _prevSection <= _sectionObjects.size())
	{
		auto section = _sectionObjects[_prevSection - 1];
		for (size_t j = 0; j < section.size(); j++)
		{
			section[j]->setActive(false);
			if (section[j]->getParent() != nullptr)
			{
				AX_SAFE_RETAIN(section[j]);
				if (section[j]->_particle)
				{
					AX_SAFE_RETAIN(section[j]->_particle);
					removeChild(section[j]->_particle, true);
				}
				if (section[j]->_glowSprite)
				{
					AX_SAFE_RETAIN(section[j]->_glowSprite);
					_glowBatchNode->removeChild(section[j]->_glowSprite, true);
				}
				if (isObjectBlending(section[j]))
				{
					switch (section[j]->_zLayer)
					{
					case -3: _blendingBatchNodeB4->removeChild(section[j], true); break;
					case -1: _blendingBatchNodeB3->removeChild(section[j], true); break;
					case 1:  _blendingBatchNodeB2->removeChild(section[j], true); break;
					case 3:  _blendingBatchNodeB1->removeChild(section[j], true); break;
					default:
					case 5:  _blendingBatchNodeT1->removeChild(section[j], true); break;
					case 7:  _blendingBatchNodeT2->removeChild(section[j], true); break;
					case 9:  _blendingBatchNodeT3->removeChild(section[j], true); break;
					}
				}
				else
				{
					if (section[j]->_texturePath == _mainBatchNodeTexture)
					{
						switch (section[j]->_zLayer)
						{
						case -3: _mainBatchNodeB4->removeChild(section[j], true); break;
						case -1: _mainBatchNodeB3->removeChild(section[j], true); break;
						case 1:  _mainBatchNodeB2->removeChild(section[j], true); break;
						case 3:  _mainBatchNodeB1->removeChild(section[j], true); break;
						default:
						case 5:  _mainBatchNodeT1->removeChild(section[j], true); break;
						case 7:  _mainBatchNodeT2->removeChild(section[j], true); break;
						case 9:  _mainBatchNodeT3->removeChild(section[j], true); break;
						}
					}
					else if (section[j]->_texturePath == _main2BatchNodeTexture) {
						_main2BatchNode->removeChild(section[j], true);
					}
					else {
						this->removeChild(section[j], true);
					}
				}
			}
		}
	}

	this->_prevSection = prevSection;
	this->_nextSection = nextSection;
}

void PlayLayer::changeGameMode(GameObject* obj, PlayerObject* player, PlayerGamemode gameMode)
{
	if (obj) obj->triggerActivated(player);
	switch (gameMode)
	{
	case PlayerGamemodeShip:
	case PlayerGamemodeUFO:
	case PlayerGamemodeWave:
		if (obj && obj->getPositionY() < 270)
			m_fCameraYCenter = 240.0f;
		else if (obj)
			m_fCameraYCenter = (floorf(obj->getPositionY() / 30.0f) * 30.0f);

		tweenBottomGround(-68);
		tweenCeiling(388);
		break;
	case PlayerGamemodeBall:
		if (obj && obj->getPositionY() < 240.0f)
			m_fCameraYCenter = 210.0f;
		else if (obj)
			m_fCameraYCenter = (floorf(obj->getPositionY() / 30.0f) * 30.0f);

		tweenBottomGround(-38);
		tweenCeiling(358);
		break;
	default:
		break;
	}

	player->setRotation(0.f);
	player->setGamemode(gameMode);
}

void PlayLayer::moveCameraToPos(Vec2 pos)
{
	auto moveX = [this](float a, float b, float c) -> void {
		this->stopActionByTag(0);
		auto tweenAction = ActionTween::create(b, "cTX", m_obCamPos.x, a);
		auto easeAction = EaseInOut::create(tweenAction, c);
		easeAction->setTag(0);
		this->runAction(easeAction);
		};
	auto moveY = [this](float a, float b, float c) -> void {
		this->stopActionByTag(1);
		auto tweenAction = ActionTween::create(b, "cTY", m_obCamPos.y, a);
		auto easeAction = EaseInOut::create(tweenAction, c);
		easeAction->setTag(1);
		this->runAction(easeAction);
		};
	moveX(pos.x, 1.2f, 1.8f);
	moveY(pos.y, 1.2f, 1.8f);
}

void PlayLayer::checkCollisions(PlayerObject* player, float dt)
{
	auto playerOuterBounds = player->_mini ? player->getOuterBounds(0.6f, 0.6f) : player->getOuterBounds();
	if (player->getPositionY() < (player->_mini ? 99.f : 105.0f) && player->_currentGamemode == PlayerGamemodeCube)
	{
		if (player->isGravityFlipped())
		{
			this->destroyPlayer(player);
			return;
		}

		player->setPositionY((player->_mini ? 99.f : 105.0f));

		player->hitGround(false);
	}

	else if (player->getPositionY() > 1290.0f)
	{
		this->destroyPlayer(player);
		return;
	}

	if (player->_currentGamemode != PlayerGamemodeCube)
	{
		if (player->getPositionY() <
			_bottomGround->getPositionY() + cameraFollow->getPositionY() + (player->_mini ? 87.f : 93.0f))
		{
			player->setPositionY(_bottomGround->getPositionY() + cameraFollow->getPositionY() +
				(player->_mini ? 87.f : 93.0f));

			if (!player->isGravityFlipped())
				player->hitGround(false);

			player->setYVel(0.f);
		}
		if (player->getPositionY() >
			_ceiling->getPositionY() - (player->_mini ? 234.f : 240.f) + m_fCameraYCenter - 12.f)
		{
			player->setPositionY(_ceiling->getPositionY() - (player->_mini ? 234.f : 240.f) + m_fCameraYCenter - 12.f);

			if (player->isGravityFlipped())
				player->hitGround(true);

			player->setYVel(0.f);
		}
	}

	dn->setVisible(showDn);

	if (showDn)
	{
		dn->clear();
		renderRect(playerOuterBounds, ax::Color4B::RED);
		renderRect(player->getInnerBounds(), ax::Color4B::GREEN);
	}

	int current_section = this->sectionForPos(player->getPositionX());

	std::deque<GameObject*> m_pHazards;

	for (int i = current_section - 2; i < current_section + 1; i++)
	{
		if (i >= 0 && i < _sectionObjects.size())
		{
			std::vector<GameObject*> section = _sectionObjects[i];

			for (int j = 0; j < section.size(); j++)
			{
				GameObject* obj = section[j];

				if (!obj)
					continue;

				auto objBounds = obj->getOuterBounds();

				if ((objBounds.size.width <= 0 || objBounds.size.height <= 0))
					continue;

				if (obj->getGameObjectType() == kGameObjectTypeHazard)
				{
					m_pHazards.push_back(obj);
					if (showDn)
					{
						if (obj->_radius <= 0)
							renderRect(objBounds, ax::Color4B::RED);
						else
							dn->drawCircle(obj->getPosition() + Vec2(15, 15), obj->_radius, 0, 20, 0, ax::Color4B::RED);
					}
				}
				else if (obj->isActive())
				{
					if (showDn)
					{
						renderRect(objBounds, ax::Color4B::BLUE);
					}

					if (playerOuterBounds.intersectsRect(objBounds) && !obj->hasBeenActivatedByPlayer(player))
					{
						// 1. Extraemos el tipo y la ID
						auto objType = obj->getGameObjectType();
						int objID = obj->getID();

						// 2. PARCHE: Enrutamos TODAS las orbes y pads a su tipo correcto exacto usando su ID
						if (objID == 36) objType = kGameObjectTypeYellowJumpRing;
						else if (objID == 141) objType = kGameObjectTypePinkJumpRing;
						else if (objID == 1332) objType = kGameObjectTypeRedJumpRing;
						else if (objID == 84 || objID == 1022) objType = kGameObjectTypeGravityRing; // FIX: El ID 84 es la verdadera Orbe Azul
						else if (objID == 1330) objType = kGameObjectTypeDashRing;
						else if (objID == 1704) objType = kGameObjectTypeGreenRing;
						else if (objID == 1751) objType = kGameObjectTypeDropRing;

						// Pads
						else if (objID == 35) objType = kGameObjectTypeYellowJumpPad;
						else if (objID == 140) objType = kGameObjectTypePinkJumpPad;
						else if (objID == 1333) objType = kGameObjectTypeRedJumpPad;
						else if (objID == 67) objType = kGameObjectTypeGravityPad;

						// 3. Evaluamos el tipo corregido
						switch (objType)
						{
						case kGameObjectTypeInverseGravityPortal:
							obj->triggerActivated(player);
							player->setPortalP(obj->getPosition());
							player->setPortalObject(obj);

							changeGravity(true);
							break;

						case kGameObjectTypeNormalGravityPortal:
							obj->triggerActivated(player);
							player->setPortalP(obj->getPosition());
							player->setPortalObject(obj);

							changeGravity(false);
							break;

						case kGameObjectTypeShipPortal:
							player->setPortalP(obj->getPosition());
							player->setPortalObject(obj);

							this->changeGameMode(obj, player, PlayerGamemodeShip);
							break;

						case kGameObjectTypeBallPortal:
							player->setPortalP(obj->getPosition());
							player->setPortalObject(obj);

							this->changeGameMode(obj, player, PlayerGamemodeBall);
							break;

						case kGameObjectTypeUfoPortal:
							player->setPortalP(obj->getPosition());
							player->setPortalObject(obj);

							this->changeGameMode(obj, player, PlayerGamemodeUFO);
							break;

						case kGameObjectTypeCubePortal:

							player->setPortalP(obj->getPosition());
							player->setPortalObject(obj);

							this->changeGameMode(obj, player, PlayerGamemodeCube);
							break;

						case kGameObjectTypeYellowJumpPad:
							player->setPortalP(obj->getPosition());
							player->setPortalObject(obj);
							obj->triggerActivated(player);
							player->propellPlayer(1);
							player->_touchedPadObject = obj;
							break;

						case kGameObjectTypeGravityPad: {
							if (player->_touchedPadObject)
								break;
							auto pos = obj->getPosition();
							pos.y -= 10;
							player->setPortalP(pos);
							player->setPortalObject(obj);
							obj->triggerActivated(player);
							player->propellPlayer(0.8);
							player->_touchedPadObject = obj;
							changeGravity(!player->isGravityFlipped());
							break;
						}

						case kGameObjectTypePinkJumpPad:
							player->setPortalP(obj->getPosition());
							player->setPortalObject(obj);
							obj->triggerActivated(player);
							player->propellPlayer(0.65);
							player->_touchedPadObject = obj;
							break;

						case kGameObjectTypeRedJumpPad:
							player->setPortalP(obj->getPosition());
							player->setPortalObject(obj);
							obj->triggerActivated(player);
							player->propellPlayer(1.25);
							player->_touchedPadObject = obj;
							break;

						case kGameObjectTypeYellowJumpRing:
						case kGameObjectTypeDashRing:
						case kGameObjectTypeGravityRing:
						case kGameObjectTypeRedJumpRing:
						case kGameObjectTypePinkJumpRing:
						case kGameObjectTypeDropRing:
						case kGameObjectTypeGreenRing:
							player->setPortalP(obj->getPosition());
							player->setPortalObject(obj);

							player->setTouchedRing(obj);

							player->ringJump(obj);
							break;
						case kGameObjectTypeModifier:
							switch (obj->getID())
							{
							case 201:
								changePlayerSpeed(0);
								break;
							case 200:
								changePlayerSpeed(1);
								break;
							case 202:
								changePlayerSpeed(2);
								break;
							case 203:
								changePlayerSpeed(3);
								break;
							case 1334:
								changePlayerSpeed(4);
								break;
							}
							break;
						case kGameObjectTypeSpecial:
						case kGameObjectTypeNormalMirrorPortal:
						case kGameObjectTypeInverseMirrorPortal:
							break;
						case kGameObjectTypeMiniSizePortal:
							obj->triggerActivated(player);
							player->setPortalP(obj->getPosition());
							player->setPortalObject(obj);
							player->toggleMini(true);
							break;
						case kGameObjectTypeRegularSizePortal:
							obj->triggerActivated(player);
							player->setPortalP(obj->getPosition());
							player->setPortalObject(obj);
							player->toggleMini(false);
							break;
						default:
							player->collidedWithObject(dt, obj);
							break;
						}
					}
				}
			}
		}
	}
	for (unsigned int i = 0; i < m_pHazards.size(); ++i)
	{
		GameObject* hazard = m_pHazards[i];
		if (hazard->_radius > 0)
		{
			if (playerOuterBounds.intersectsCircle(hazard->getPosition() + Vec2(15, 15), hazard->_radius))
				destroyPlayer(player);
		}
		else if (playerOuterBounds.intersectsRect(hazard->getOuterBounds()))
		{
			destroyPlayer(player);
		}
	}
	m_pHazards.clear();

	if (player->_currentGamemode == PlayerGamemodeShip)
		player->_queuedHold = false;
}

void PlayLayer::onDrawImGui()
{
	extern bool _showDebugImgui;
	if (!_showDebugImgui)
		return;
	ImGui::SetNextWindowPos({ 1000.0f, 200.0f }, ImGuiCond_FirstUseEver);

	ImGui::Begin("PlayLayer Debug");

	ImGui::Text("%s", std::to_string(_player1->_queuedHold).c_str());

	ImGui::Checkbox("Freeze Player", &m_freezePlayer);
	ImGui::Checkbox("Platformer Mode (Basic)", &m_platformerMode);

#ifdef AX_PLATFORM_PC
	if (ImGui::Checkbox("Fullscreen", &fullscreen))
	{
		int a;
		auto monitor = glfwGetMonitors(&a)[monitorN];
		auto mode = glfwGetVideoMode(monitor);

		if (fullscreen)
			glfwSetWindowMonitor(static_cast<GLViewImpl*>(ax::Director::getInstance()->getGLView())->getWindow(),
				monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		else
		{
			glfwSetWindowMonitor(static_cast<GLViewImpl*>(ax::Director::getInstance()->getGLView())->getWindow(),
				NULL, 0, 0, 1280, 720, 0);
			glfwWindowHint(GLFW_DECORATED, true);
		}
	}
#endif

	ImGui::SameLine();

	if (ImGui::ArrowButton("full", ImGuiDir_Right))
		ImGui::OpenPopup("Fullscreen Settings");

	if (ImGui::BeginPopupModal("Fullscreen Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::InputInt("Monitor", &monitorN);
		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	if (ImGui::Button("Exit"))
	{
		this->exit();
	}

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
		ImGui::GetIO().Framerate);

	ImGui::Text("yVel %.3f", _player1->getYVel());

	ImGui::Checkbox("Show Hitboxes", &showDn);
	ImGui::Checkbox("Gain the power of invincibility", &noclip);

	if (ImGui::InputFloat("Speed", &gameSpeedPlayer))
		Director::getInstance()->getScheduler()->setTimeScale(gameSpeedPlayer);

	if (ImGui::InputFloat("FPS", &fps))
		Director::getInstance()->setAnimationInterval(1.0f / fps);

	ImGui::Text("Sections: %zu", _sectionObjects.size());
	if (_sectionObjects.size() > 0 && sectionForPos(_player1->getPositionX()) - 1 < _sectionObjects.size())
		ImGui::Text("Current Section Size: %zu", _sectionObjects[sectionForPos(_player1->getPositionX()) <= 0
			? 0
			: sectionForPos(_player1->getPositionX()) - 1]
			.size());

	if (ImGui::Button("Reset"))
	{
		this->resetLevel();
	}
	ImGui::End();
}

void PlayLayer::resetLevel()
{
	DBG_LOG("PlayLayer::resetLevel - ejecutando, descongelando jugador");

	// 2. Forzar limpieza de estados (ESTO EVITA QUE SE CONGELE)
	m_freezePlayer = false;
	_isPaused = false;
	m_bEndAnimation = false;

	if (m_closeMenu) m_closeMenu->setEnabled(true);

	_attempts++;
	auto dir = Director::getInstance();

	if (_player1) {
		_player1->setPosition({ 0, 105 });
		_player1->setRotation(0);
		_player1->setVisible(true);
		_player1->setIsDead(false); // �VITAL! Si no revivimos al jugador, las f�sicas se ignoran
		_player1->reset();
	}
	if (_player2) {
		_player2->setPosition({ 0, 105 });
		_player2->setRotation(0);
		_player2->setVisible(false);
		_player2->setActive(false);
		_player2->setIsDead(false); // �VITAL!
		_player2->reset();
	}

	m_obCamPos.x = 0;
	m_obCamPos.y = 0;

	_bottomGround->setPositionX(0);
	_ceiling->setPositionX(0);
	m_pBG->setPositionX(dir->getWinSize().x / 2);
	_enterEffectID = 0;
	m_bEndAnimation = false;
	_isDualMode = false;
	_secondsSinceStart = 0;

	for (auto& [groupID, groupProps] : _groups)
		groupProps._alpha = 1.f;

	for (auto obj : this->_allObjects)
	{
		if (!obj) continue;
		if (obj->_isTrigger)
		{
			auto trigger = dynamic_cast<EffectGameObject*>(obj);
			if (trigger) trigger->_wasTriggerActivated = false;
		}
		obj->_hasBeenActivatedP1 = false;
		obj->_hasBeenActivatedP2 = false;
		obj->_effectOpacityMultipler = 1.f;
		obj->setActive(false);

		if (obj->getParent() != nullptr)
		{
			if (obj->_particle)
			{
				AX_SAFE_RETAIN(obj->_particle);
				removeChild(obj->_particle, true);
			}
			if (obj->_glowSprite)
			{
				AX_SAFE_RETAIN(obj->_glowSprite);
				_glowBatchNode->removeChild(obj->_glowSprite, true);
			}
			AX_SAFE_RETAIN(obj);
			if (isObjectBlending(obj))
			{
				switch (obj->_zLayer)
				{
				case -3: _blendingBatchNodeB4->removeChild(obj, true); break;
				case -1: _blendingBatchNodeB3->removeChild(obj, true); break;
				case 1:  _blendingBatchNodeB2->removeChild(obj, true); break;
				case 3:  _blendingBatchNodeB1->removeChild(obj, true); break;
				default:
				case 5:  _blendingBatchNodeT1->removeChild(obj, true); break;
				case 7:  _blendingBatchNodeT2->removeChild(obj, true); break;
				case 9:  _blendingBatchNodeT3->removeChild(obj, true); break;
				}
			}
			else
			{
				if (obj->_texturePath == _mainBatchNodeTexture)
				{
					switch (obj->_zLayer)
					{
					case -3: _mainBatchNodeB4->removeChild(obj, true); break;
					case -1: _mainBatchNodeB3->removeChild(obj, true); break;
					case 1:  _mainBatchNodeB2->removeChild(obj, true); break;
					case 3:  _mainBatchNodeB1->removeChild(obj, true); break;
					default:
					case 5:  _mainBatchNodeT1->removeChild(obj, true); break;
					case 7:  _mainBatchNodeT2->removeChild(obj, true); break;
					case 9:  _mainBatchNodeT3->removeChild(obj, true); break;
					}
				}
				else if (obj->_texturePath == _main2BatchNodeTexture) {
					_main2BatchNode->removeChild(obj, true);
				}
				else {
					this->removeChild(obj, true);
				}
			}
		}
	}

	// Aseg�rate de que esto est� as� a la mitad de tu resetLevel:
	this->stopAllActions();
	if (_bottomGround) _bottomGround->stopAllActions();
	if (_ceiling) _ceiling->stopAllActions();

	_colorChannels = _originalColors;

	_prevSection = -1;
	_nextSection = -1;

	if (this->_colorChannels.contains(1000))
		this->m_pBG->setColor(this->_colorChannels.at(1000)._color);
	else {
		this->m_pBG->setColor(ax::Color3B::GRAY);
		this->_colorChannels[1000]._color = ax::Color3B::GRAY;
	}

	if (this->_colorChannels.contains(1001) && _bottomGround && _bottomGround->_sprite) {
		_bottomGround->_sprite->setColor(this->_colorChannels.at(1001)._color);
		if (_ceiling && _ceiling->_sprite) _ceiling->_sprite->setColor(this->_colorChannels.at(1001)._color);
	}

	this->_bottomGround->update(0);
	this->_ceiling->update(0);

	// Detenemos la música anterior
	FMODAudioEngine::getInstance()->stopAllMusic();

	// --- LÓGICA DE CANCIÓN OFICIAL VS CUSTOM ---
	// Si _musicID es mayor a 0, usamos la canción que venía en el string. 
	// Si es 0 (nivel oficial), usamos la ID del nivel actual (1=Stereo, 2=Back On Track, etc).
	int finalSongID = (getLevel()->_musicID > 0) ? getLevel()->_musicID : getLevel()->_levelID;

	std::string bgmFile = LevelTools::getAudioFilename(finalSongID);
	FMODAudioEngine::getInstance()->playMusic(bgmFile, _levelSettings.songOffset);
	// -------------------------------------------

	changeGameMode(nullptr, _player1, (PlayerGamemode)_levelSettings.gamemode);
	changeGameMode(nullptr, _player2, (PlayerGamemode)_levelSettings.gamemode);
	_player1->toggleMini(_levelSettings.mini);
	_player2->toggleMini(_levelSettings.mini);
	changePlayerSpeed(_levelSettings.speed);
	m_obCamPos.y = m_fCameraYCenter;
	_isDualMode = _levelSettings.dual;

	this->incrementTime(); // Arrancamos el cron�metro de manera limpia

	this->updateVisibility();

	// �LA CLAVE! Enciende el bucle de f�sicas para que el jugador se mueva y los objetos aparezcan
	this->scheduleUpdate();
}

void PlayLayer::renderRect(ax::Rect rect, ax::Color4B col)
{
	dn->drawRect({ rect.getMinX(), rect.getMinY() }, { rect.getMaxX(), rect.getMaxY() }, col);
	dn->drawSolidRect({ rect.getMinX(), rect.getMinY() }, { rect.getMaxX(), rect.getMaxY() },
		Color4B(col.r, col.g, col.b, 100));
}

void PlayLayer::onEnter()
{
	Layer::onEnter();

	auto listener = EventListenerKeyboard::create();
	auto dir = Director::getInstance();

	listener->onKeyPressed = AX_CALLBACK_2(PlayLayer::onKeyPressed, this);
	listener->onKeyReleased = AX_CALLBACK_2(PlayLayer::onKeyReleased, this);
	dir->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

	auto current = dir->getRunningScene();
#if SHOW_IMGUI == true
	ImGuiPresenter::getInstance()->addRenderLoop("#playlayer", AX_CALLBACK_0(PlayLayer::onDrawImGui, this), current);
#endif

	this->scheduleUpdate();
}

void PlayLayer::onExit()
{

#if SHOW_IMGUI == true
	Director::getInstance()->getEventDispatcher()->removeEventListenersForTarget(this);
	ImGuiPresenter::getInstance()->removeRenderLoop("#playlayer");
#endif
	LevelPage::replacingScene = false;
	Layer::onExit();
}

void PlayLayer::exit()
{
	this->unscheduleUpdate();

	// --- 1. COMPENSACI�N DEL SALTO DE C�MARA NATIVO ---
	// Detenemos cualquier movimiento residual
	if (_player1) _player1->pause();
	if (_player2) _player2->pause();

	// Reiniciamos la c�mara y movemos la capa a la inversa para congelar la vista
	auto winSize = ax::Director::getInstance()->getWinSize();
	ax::Camera::getDefaultCamera()->setPosition(winSize / 2);
	this->setPosition(-m_obCamPos);
	// --------------------------------------------------

	_player1->deactivateStreak();
	_player2->deactivateStreak();
	unscheduleAllCallbacks();
	_player1->unscheduleAllCallbacks();
	_player2->unscheduleAllCallbacks();
	_bottomGround->unscheduleUpdate();
	if (_ceiling)
	{
		_ceiling->unscheduleUpdate();
	}

	int size = _allObjects.size();
	for (int i = 0; i < size; i++)
	{
		GameObject* obj = _allObjects.at(i);
		if (obj && !obj->getParent())
		{
			if (obj->_particle)
			{
				obj->_particle->onExit();
				AX_SAFE_RELEASE_NULL(obj->_particle);
			}
			if (obj->_glowSprite)
			{
				obj->_glowSprite->onExit();
				AX_SAFE_RELEASE_NULL(obj->_glowSprite);
			}
			obj->unscheduleAllCallbacks();
			obj->onExit();
			obj->setActive(false);
			AX_SAFE_RELEASE(obj);
		}
	}

	Instance = nullptr;
	BaseGameLayer::_instance = nullptr;

	// 1. �CR�TICO! Despausar FMOD por si el jugador sali� desde el men� de pausa
	FMODAudioEngine::getInstance()->setPaused(false);

	// 2. Detenemos la m�sica del nivel
	FMODAudioEngine::getInstance()->stopAllMusic();

	// 3. Reproducimos el sonido de salida (ahora s� se escuchar�)
	FMODAudioEngine::getInstance()->playEffect("quitSound_01.ogg");

	// 4. Arrancamos de nuevo la m�sica del men� principal
	FMODAudioEngine::getInstance()->playMusic("menuLoop.mp3", 0.0f, 1.0f, true);

	// (Aseg�rate de borrar el segundo stopAllMusic() que ten�as duplicado aqu� abajo)

	int id = getLevel()->_levelID;
	if (id <= 0 || id > 22) {
		Director::getInstance()->popScene();
		return;
	}

	Director::getInstance()->replaceScene(TransitionFade::create(0.5f, LevelSelectLayer::scene(id - 1)));
}

void PlayLayer::pauseGame() {
	if (_isPaused || _player1->isDead() || m_bEndAnimation) return;

	// Tu sistema seguro detendr� el avance sin romper el motor
	_isPaused = true;
	m_freezePlayer = true;
	FMODAudioEngine::getInstance()->setPaused(true);

	auto pLayer = PauseLayer::create(this);
	pLayer->setName("PauseLayer");
	if (m_pHudLayer) m_pHudLayer->addChild(pLayer, 9999);
	else this->addChild(pLayer, 9999);
}

void PlayLayer::resume() {
	if (!_isPaused) return;

	_isPaused = false;
	m_freezePlayer = false;
	FMODAudioEngine::getInstance()->setPaused(false);

	if (m_pBar) m_pBar->setVisible(true);
	if (m_pPercentage) m_pPercentage->setVisible(true);

	ax::Node* pLayer = m_pHudLayer ? m_pHudLayer->getChildByName("PauseLayer") : this->getChildByName("PauseLayer");
	if (pLayer) pLayer->removeFromParent();
}

void PlayLayer::resumeAndRestart(bool fullReset) {
	resume();
	resetLevel();
}

void PlayLayer::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	if (keyCode == EventKeyboard::KeyCode::KEY_R) {
		// Cancelamos los temporizadores de forma segura desde el evento del teclado
		this->unschedule("playlayer_restart");
		this->unschedule("start_delay");
		resetLevel();
	}
	else if (keyCode == EventKeyboard::KeyCode::KEY_SPACE || keyCode == EventKeyboard::KeyCode::KEY_UP_ARROW) {
		if (!_player1->m_bIsHolding) _player1->pushButton();
		if (_isDualMode && !_player2->m_bIsHolding) _player2->pushButton();
	}
	else if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE) {
		if (_isPaused) {
			// resume() ya busca "PauseLayer" y lo quita (removeFromParent)
			// internamente. Llamar removeFromParent() otra vez aqui sobre
			// el mismo puntero es use-after-free: la capa ya fue liberada
			// dentro de resume(), y esto causaba el cierre/crash al
			// presionar ESC por segunda vez.
			this->resume();
		}
		else {
			_player1->releaseButton();
			if (_isDualMode) _player2->releaseButton();
			this->pauseGame();
		}
		if (event) event->stopPropagation();
	}

	if (keyCode == EventKeyboard::KeyCode::KEY_A && _player1->m_bIsPlatformer)
		_player1->direction = -1.f;
	else if (keyCode == EventKeyboard::KeyCode::KEY_D && _player1->m_bIsPlatformer)
		_player1->direction = 1.f;
}

void PlayLayer::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	if ((keyCode == EventKeyboard::KeyCode::KEY_A && _player1->direction == -1.f) ||
		(keyCode == EventKeyboard::KeyCode::KEY_D && _player1->direction == 1.f) && _player1->m_bIsPlatformer)
		_player1->direction = 0.f;

	if (keyCode == EventKeyboard::KeyCode::KEY_SPACE || keyCode == EventKeyboard::KeyCode::KEY_UP_ARROW) {
		_player1->releaseButton();
		if (_isDualMode) _player2->releaseButton();
	}
}

void PlayLayer::tweenBottomGround(float y)
{
	_bottomGround->runAction(EaseInOut::create(ActionTween::create(0.1f, "y", _bottomGround->getPositionY(), y), 2.f));
}

void PlayLayer::tweenCeiling(float y)
{
	_ceiling->runAction(EaseInOut::create(ActionTween::create(0.1f, "y", _ceiling->getPositionY(), y), 2.f));
}

void PlayLayer::changePlayerSpeed(int speed)
{
	switch (speed)
	{
	case 0:
		_player1->m_dXVel = 5.77;
		_player1->setPlayerSpeed(0.9);
		_player2->m_dXVel = 5.77;
		_player2->setPlayerSpeed(0.9);
		break;
	case 1:
		_player1->m_dXVel = 5.98;
		_player1->setPlayerSpeed(0.7);
		_player2->m_dXVel = 5.98;
		_player2->setPlayerSpeed(0.7);
		break;
	case 2:
		_player1->m_dXVel = 5.87;
		_player1->setPlayerSpeed(1.1);
		_player2->m_dXVel = 5.87;
		_player2->setPlayerSpeed(1.1);
		break;
	case 3:
		_player1->m_dXVel = 6;
		_player1->setPlayerSpeed(1.3);
		_player2->m_dXVel = 6;
		_player2->setPlayerSpeed(1.3);
		break;
	case 4:
		_player1->m_dXVel = 6;
		_player1->setPlayerSpeed(1.6);
		_player2->m_dXVel = 6;
		_player2->setPlayerSpeed(1.6);
		break;
	}
}

void PlayLayer::changeGravity(bool gravityFlipped)
{
	_player1->flipGravity(gravityFlipped);
	if (_isDualMode)
		_player2->flipGravity(!gravityFlipped);
}

PlayLayer* PlayLayer::getInstance()
{
	return Instance;
}

void PlayLayer::updateTriggers(float dt)
{
	int current_section = sectionForPos(ax::Camera::getDefaultCamera()->getPositionX());

	for (int i = current_section - 3; i < current_section + 1; i++)
	{
		if (i >= 0 && i < _sectionObjects.size())
		{
			std::vector<GameObject*> section = _sectionObjects[i];

			for (int j = 0; j < section.size(); j++)
			{
				GameObject* obj = section[j];
				if (obj->isActive() && obj->_isTrigger)
				{
					auto trigger = dynamic_cast<EffectGameObject*>(obj);
					if (trigger && !trigger->_spawnTriggered &&
						trigger->getPositionX() <= ax::Camera::getDefaultCamera()->getPositionX())
					{
						trigger->triggerActivated(dt);
					}
				}
			}
		}
	}
}

void PlayLayer::startGame() {}

void PlayLayer::onEnterTransitionDidFinish()
{
	Layer::onEnterTransitionDidFinish();
	if (m_bFirstAttempt) {
		this->scheduleOnce([this](float d) {
			this->incrementTime(); // Inicia el reloj del nivel de forma segura
			this->resetLevel();
			}, 1.0f, "start_delay");
		m_bFirstAttempt = false;
	}
}