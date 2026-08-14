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

#pragma once
#include <string_view>
#include <vector>

#include "EventKeyboard.h"
#include "BaseGameLayer.h"

enum PlayerGamemode;

class GJGameLevel;
class GameObject;
class SimpleProgressBar;
class UILayer;
class PlayerObject;
class GroundLayer;
class MenuItemSpriteExtra;

namespace ax
{
	class Event;
	class Sprite;
	class Label;
}

class PlayLayer : public BaseGameLayer
{
protected:
	bool init(GJGameLevel* level) override;
	void onEnter() override;
	void onExit() override;
	void onDrawImGui();
	virtual void onKeyPressed(ax::EventKeyboard::KeyCode keyCode, ax::Event* event);
	virtual void onKeyReleased(ax::EventKeyboard::KeyCode keyCode, ax::Event* event);
	void createLevelEnd();

	ax::Node* cameraFollow;

	ax::Sprite* m_pBG;
	GroundLayer* _bottomGround, * _ceiling;

	ax::Vec2 m_obCamPos;

	MenuItemSpriteExtra* backbtn;

	float m_fCameraYCenter;
	float m_lastObjXPos = 570.0f;
	bool m_bFirstAttempt = true;
	bool m_bMoveCameraX;
	bool m_bMoveCameraY;
	bool m_bShakingCamera;
	float m_fEndOfLevel = FLT_MAX;
	float m_fShakeIntensity = 1;

	bool m_bIsJumpPressed;

	SimpleProgressBar* m_pBar;
	ax::Label* m_pPercentage;

	bool m_freezePlayer = false;
	bool m_platformerMode = false;
	bool m_bEndAnimation = false;

	double m_extraDelta = 0.0;
	int m_resumeTimer = 0;

	void setInstance();
public:
	int _enterEffectID = 0;

	UILayer* m_pHudLayer;
	ax::Sprite* m_closeButtonSpr;
	ax::Menu* m_closeMenu;

	int _secondsSinceStart;
	int _attempts;
	int _jumps;
	bool _everyplay_recorded;

	std::vector<bool> _coinsCollected;
	bool _isPaused = false;
	bool m_started = false;

	virtual void destroyPlayer(PlayerObject* player, GameObject* hazard) override;
	void destroyPlayer(PlayerObject* player) override;
	void spawnCircle();
	void showEndLayer();
	virtual void showCompleteText();

	void update(float delta) override;
	virtual void updateCamera(float dt);
	void updateVisibility();
	void checkCollisions(PlayerObject* player, float dt);
	void moveCameraToPos(ax::Vec2);
	void changeGameMode(GameObject* obj, PlayerObject* player, PlayerGamemode gameMode);

	void addObject(GameObject* obj) override;
	void updateTriggers(float dt);

	virtual void resetLevel() override;
	void startGame();
	void onEnterTransitionDidFinish() override;

	void pauseGame();
	void resume();
	void resumeAndRestart(bool fullReset);

	void exit();

	void tweenBottomGround(float y);
	void tweenCeiling(float y);
	void renderRect(ax::Rect rect, ax::Color4B col);
	void applyEnterEffect(GameObject* obj);
	float getRelativeMod(ax::Vec2 objPos, float v1, float v2, float v3);
	int sectionForPos(float x) override;

	double getModifiedDelta(float dt);
	void changePlayerSpeed(int speed);
	void changeGravity(bool gravityFlipped);
	void incrementTime();
	ax::Color3B getLightBG() override;

	static ax::Scene* scene(GJGameLevel* level);
	static PlayLayer* create(GJGameLevel* level);
	static PlayLayer* getInstance();
};