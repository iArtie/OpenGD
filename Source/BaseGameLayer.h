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
#include "axmol.h"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <map>
#include <span>
#include "SpriteColor.h"
#include "GameObject.h"
#include "PlayerObject.h"
#include "EffectManager.h"

struct LevelSettings {
	int bgID = 1;
	int _bgID = 1;
	int groundID = 1;
	int _groundID = 1;
	int groundLineID = 1;
	int fontID = 1;
	float songOffset = 0;
	bool twoPlayerMode = false;
	bool isPlatformer = false;

	int gamemode = 0;
	bool mini = false;
	int speed = 0;
	bool dual = false;
	bool twoPlayer = false;
	bool flipGravity = false;
};

struct GroupProperties {
	enum GroupState { NOT_CHANGING, MAIN_ONLY, DETAIL_ONLY, MAIN_DETAIL };
	ax::Color3B _color;
	float _alpha = 1.0f;
	GroupState groupState = NOT_CHANGING;
	std::vector<GameObject*> _objects;
};

class GJGameLevel;

class BaseGameLayer : public ax::Layer {
protected:
	GJGameLevel* _level;

public:
	static BaseGameLayer* _instance;

	PlayerObject* _player1;
	PlayerObject* _player2;

	EffectManager* _effectManager;
	std::vector<GameObject*> _pObjects;
	std::vector<GameObject*> _allObjects;
	std::vector<std::vector<GameObject*>> _sectionObjects;

	ax::DrawNode* dn;
	ax::SpriteBatchNode* _batchNode;
	ax::SpriteBatchNode* _glowBatchNode;

	std::string _mainBatchNodeTexture = "GJ_GameSheet.png";
	std::string _main2BatchNodeTexture = "GJ_GameSheet02.png";

	ax::SpriteBatchNode* _mainBatchNodeB4 = nullptr;
	ax::SpriteBatchNode* _mainBatchNodeB3 = nullptr;
	ax::SpriteBatchNode* _mainBatchNodeB2 = nullptr;
	ax::SpriteBatchNode* _mainBatchNodeB1 = nullptr;
	ax::SpriteBatchNode* _mainBatchNodeT1 = nullptr;
	ax::SpriteBatchNode* _mainBatchNodeT2 = nullptr;
	ax::SpriteBatchNode* _mainBatchNodeT3 = nullptr;

	ax::SpriteBatchNode* _main2BatchNode = nullptr;

	ax::SpriteBatchNode* _blendingBatchNodeB4 = nullptr;
	ax::SpriteBatchNode* _blendingBatchNodeB3 = nullptr;
	ax::SpriteBatchNode* _blendingBatchNodeB2 = nullptr;
	ax::SpriteBatchNode* _blendingBatchNodeB1 = nullptr;
	ax::SpriteBatchNode* _blendingBatchNodeT1 = nullptr;
	ax::SpriteBatchNode* _blendingBatchNodeT2 = nullptr;
	ax::SpriteBatchNode* _blendingBatchNodeT3 = nullptr;

	LevelSettings _levelSettings;

	std::unordered_map<int, SpriteColor> _colorChannels;
	std::unordered_map<int, SpriteColor> _originalColors;
	std::unordered_map<int, GroupProperties> _groups;

	int _prevSection = -1;
	int _nextSection = -1;

	bool _isPracticeMode = false;
	bool _isTestMode = false;
	bool _testMode = false;
	bool _isDualMode = false;
	float m_sectionXFactor = 1.0f;
	float m_sectionYFactor = 1.0f;
	int m_solidCollisionObjectsCount = 0;
	int m_hazardCollisionObjectsCount = 0;
	float m_maxGameplayY = 2700.f;
	bool m_isBetweenSteps = false;

	virtual bool init(GJGameLevel* level);
	virtual void update(float dt) override {}
	virtual void resetLevel() {}

	void setLevel(GJGameLevel* level) { _level = level; }
	GJGameLevel* getLevel() { return _level; }

	// Declaraciones de las funciones integradas de la decompilaci�n
	void initBatchNodes();
	bool isObjectBlending(GameObject* obj);
	bool checkCollision(int a2, int a3);
	virtual int checkCollisions(PlayerObject* player, float dt, bool a4);
	void fillColorChannel(std::span<std::string_view> colorString, int id);

	// Carga real del nivel: descomprime el string, parsea LevelSettings/colores
	// y crea todos los GameObject. Sin esto, PlayLayer nunca recibe objetos.
	void loadLevel();
	void setupLevel(std::string_view uncompressedLevelString);
	void createObjectsFromSetup(std::string_view uncompressedLevelString);
	void loadLevelData(std::string_view data);

	virtual void destroyPlayer(PlayerObject* player, GameObject* hazard) {}
	virtual void destroyPlayer(PlayerObject* player) {}
	virtual void addObject(GameObject* obj) {}
	virtual ax::Color3B getLightBG() { return ax::Color3B::WHITE; }
	virtual int sectionForPos(float x);

	// Plantillas Vari�dicas (Comodines Universales)
	template<typename... Args>
	void runMoveCommand(Args... args) {}

	template<typename... Args>
	ax::Color3B getLightBG(Args... args) { return ax::Color3B::WHITE; }

	template<typename... Args>
	void processMoveActionsStep(Args... args) {}

	static BaseGameLayer* getInstance();
};

// Sobrecarga de operadores para proteger comparaciones obsoletas de C++
inline bool operator==(const std::string& lhs, int rhs) { return rhs == 0 && lhs.empty(); }
inline bool operator!=(const std::string& lhs, int rhs) { return rhs != 0 || !lhs.empty(); }
inline bool operator==(int lhs, const std::string& rhs) { return lhs == 0 && rhs.empty(); }
inline bool operator!=(int lhs, const std::string& rhs) { return lhs != 0 || !rhs.empty(); }
inline bool operator==(const std::string& lhs, std::nullptr_t) { return lhs.empty(); }
inline bool operator!=(const std::string& lhs, std::nullptr_t) { return !lhs.empty(); }