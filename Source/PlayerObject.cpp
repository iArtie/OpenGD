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

#include "PlayerObject.h"
#include "FMODAudioEngine.h"
#include "PlayLayer.h"
#include "2d/ParticleSystemQuad.h"
#include "2d/ActionInterval.h"
#include "2d/ActionEase.h"
#include "2d/ActionInstant.h"
#include "CircleWave.h"
#include "GameToolbox/math.h"
#include "GameToolbox/log.h"
#include "GameToolbox/conv.h"

USING_NS_AX;

void PlayerObject::reset()
{
	m_dYVel = 0.f;
	m_bIsLocked = false; // FIX: Permite que el jugador pueda avanzar
	m_bIsPlatformer = false; // FIX: Forza el modo auto-scroll normal
	setIsDead(false);
	flipGravity(false);
	m_snappedObject = nullptr;
	m_snapDifference = 0;
	m_isRising = false;
	m_bIsHolding = false;
	_touchedRingObject = nullptr;
	stopActionByTag(0);
	stopActionByTag(1);
	_hasRingJumped = false;

	dragEffect1->pauseEmissions();
	dragEffect2->pauseEmissions();
	dragEffect3->pauseEmissions();
	shipDragEffect->pauseEmissions();
	landEffect1->pauseEmissions();
	landEffect2->pauseEmissions();

	_particles1Activated = false;
	_particles2Activated = false;
	_particles3Activated = false;

	deactivateStreak();
}

void PlayerObject::playDeathEffect()
{
	FMODAudioEngine::getInstance()->stopAllEffects();
	FMODAudioEngine::getInstance()->playEffect("explode_11.ogg");
	dragEffect1->pauseEmissions();
	if (auto layer = getPlayLayer()) layer->unscheduleUpdate();
}

PlayerObject* PlayerObject::create(int playerFrame, Layer* gameLayer) {
	auto pRet = new (std::nothrow) PlayerObject();
	if (pRet && pRet->init(playerFrame, gameLayer)) {
		pRet->autorelease();
		return pRet;
	}
	AX_SAFE_DELETE(pRet);
	return pRet;
}

bool PlayerObject::init(int playerFrame, Layer* gameLayer_)
{
	if (!GameObject::init(fmt::format("player_{:02d}_001", playerFrame))) return false;

	int frame = GameToolbox::inRange(playerFrame, 1, 13);
	auto sprStr1 = fmt::format("player_{:02d}_001.png", frame);
	auto sprStr2 = fmt::format("player_{:02d}_2_001.png", frame);

	gameLayer = gameLayer_;
	inPlayLayer = dynamic_cast<PlayLayer*>(gameLayer_) != nullptr;

	setTextureRect(Rect(0, 0, 30, 30));

	m_pMainSprite = Sprite::createWithSpriteFrameName(sprStr1);
	m_pMainSprite->setStretchEnabled(false);
	addChild(m_pMainSprite, 1);

	m_pSecondarySprite = Sprite::createWithSpriteFrameName(sprStr2);
	m_pSecondarySprite->setStretchEnabled(false);
	m_pMainSprite->addChild(m_pSecondarySprite, -1);
	m_pSecondarySprite->setPosition(m_pMainSprite->getContentSize() / 2.f);

	m_pShipSprite = Sprite::createWithSpriteFrameName("ship_01_001.png");
	m_pShipSprite->setStretchEnabled(false);
	m_pShipSprite->setVisible(false);
	m_pShipSprite->setPosition({ 0, -5 });
	addChild(m_pShipSprite, 2);

	m_pShipSecondarySprite = Sprite::createWithSpriteFrameName("ship_01_2_001.png");
	m_pShipSecondarySprite->setStretchEnabled(false);
	m_pShipSprite->addChild(m_pShipSecondarySprite, -1);
	m_pShipSecondarySprite->setPosition(m_pShipSprite->getContentSize() / 2.f);

	_ballSprite = Sprite::createWithSpriteFrameName("player_ball_01_001.png");
	_ballSprite->setStretchEnabled(false);
	_ballSprite->setVisible(false);
	addChild(_ballSprite, 1);

	_ballSecondarySprite = Sprite::createWithSpriteFrameName("player_ball_01_2_001.png");
	_ballSecondarySprite->setStretchEnabled(false);
	_ballSprite->addChild(_ballSecondarySprite, -1);
	_ballSecondarySprite->setPosition(_ballSprite->getContentSize() / 2.f);

	_ufoSprite = Sprite::createWithSpriteFrameName("bird_01_001.png");
	_ufoSprite->setStretchEnabled(false);
	_ufoSprite->setVisible(false);
	_ufoSprite->setPositionY(-7);
	addChild(_ufoSprite, 1);

	_ufoSecondarySprite = Sprite::createWithSpriteFrameName("bird_01_2_001.png");
	_ufoSecondarySprite->setStretchEnabled(false);
	_ufoSprite->addChild(_ufoSecondarySprite, -1);
	_ufoSecondarySprite->setPosition(_ufoSprite->getContentSize() / 2.f);

	_ufoTertiarySprite = Sprite::createWithSpriteFrameName("bird_01_3_001.png");
	_ufoTertiarySprite->setStretchEnabled(false);
	_ufoSprite->addChild(_ufoTertiarySprite, -2);
	_ufoTertiarySprite->setPosition(_ufoSprite->getContentSize() / 2.f);

	dragEffect1 = ParticleSystemQuad::create("dragEffect.plist");
	dragEffect1->setPositionType(ParticleSystem::PositionType::FREE);
	dragEffect1->pauseEmissions();
	gameLayer->addChild(dragEffect1, 1);

	dragEffect2 = ParticleSystemQuad::create("dragEffect.plist");
	dragEffect2->setPositionType(ParticleSystem::PositionType::FREE);
	dragEffect2->pauseEmissions();
	dragEffect2->setPositionY(2);
	gameLayer->addChild(dragEffect2, 1);

	dragEffect3 = ParticleSystemQuad::create("dragEffect.plist");
	dragEffect3->setPositionType(ParticleSystem::PositionType::FREE);
	dragEffect3->pauseEmissions();
	dragEffect3->setPositionY(2);
	gameLayer->addChild(dragEffect3, 1);

	dragEffect2->setSpeed(dragEffect2->getSpeed() * 0.2f);
	dragEffect2->setSpeedVar(dragEffect2->getSpeedVar() * 0.2f);
	dragEffect3->setSpeed(dragEffect3->getSpeed() * 0.2f);
	dragEffect3->setSpeedVar(dragEffect3->getSpeedVar() * 0.2f);
	dragEffect3->setAngleVar(dragEffect3->getAngleVar() * 2.f);
	dragEffect3->setStartSize(dragEffect3->getStartSize() * 1.5f);
	dragEffect3->setStartSizeVar(dragEffect3->getStartSizeVar() * 1.5f);

	shipDragEffect = ParticleSystemQuad::create("shipDragEffect.plist");
	shipDragEffect->setPositionType(ParticleSystem::PositionType::GROUPED);
	shipDragEffect->pauseEmissions();
	gameLayer->addChild(shipDragEffect, 1);

	landEffect1 = ParticleSystemQuad::create("landEffect.plist");
	landEffect1->setPositionType(ParticleSystem::PositionType::GROUPED);
	landEffect1->pauseEmissions();
	gameLayer->addChild(landEffect1, 1);

	landEffect2 = ParticleSystemQuad::create("landEffect.plist");
	landEffect2->setPositionType(ParticleSystem::PositionType::GROUPED);
	landEffect2->pauseEmissions();
	gameLayer->addChild(landEffect2, 1);

	deactivateStreak();
	return true;
}

void PlayerObject::setMainColor(Color3B col)
{
	float r = static_cast<float>(col.r);
	float g = static_cast<float>(col.g);
	float b = static_cast<float>(col.b);
	dragEffect1->setStartColor({ r, g, b, 100 });
	dragEffect1->setEndColor({ r, g, b, 0 });
	shipDragEffect->setStartColor({ r, g, b, 190 });
	shipDragEffect->setEndColor({ r, g, b, 0 });

	this->m_pMainSprite->setColor(col);
	this->m_pShipSprite->setColor(col);
	_ballSprite->setColor(col);
	_ufoSprite->setColor(col);
}

void PlayerObject::setSecondaryColor(Color3B col)
{
	this->m_pSecondarySprite->setColor(col);
	this->m_pShipSecondarySprite->setColor(col);
	_ballSecondarySprite->setColor(col);
	_ufoSecondarySprite->setColor(col);
}

Color3B PlayerObject::getMainColor() { return this->m_pMainSprite->getColor(); }
Color3B PlayerObject::getSecondaryColor() { return this->m_pSecondarySprite->getColor(); }
void PlayerObject::setIsDead(bool value) { m_bIsDead = value; }
void PlayerObject::setIsOnGround(bool value) { m_bOnGround = value; }

void PlayerObject::setPosition(const ax::Vec2& pos)
{
	GameObject::setPosition(pos);

	// FIX: Ajustar las hitboxes respetando el Anchor Point {0,0} y el modo Mini
	const float outerSize = 30.0f * _vehicleSize;
	const float innerSize = outerSize * 0.7f;
	float offset = (outerSize - innerSize) / 2.0f;

	setOuterBounds(ax::Rect(pos.x, pos.y, outerSize, outerSize));
	setInnerBounds(ax::Rect(pos.x + offset, pos.y + offset, innerSize, innerSize));
}

void PlayerObject::update(float dt)
{
	m_prevPos = getPosition();
	if (this->m_bIsDead) return;

	if (_currentGamemode == PlayerGamemodeCube)
	{
		if (isOnGround())
		{
			if (!_particles1Activated) { dragEffect1->resumeEmissions(); _particles1Activated = true; }
			if (getActionByTag(2)) stopActionByTag(2);
		}
		else
		{
			if (_particles1Activated && !getActionByTag(2))
			{
				Sequence* action = Sequence::create(
					DelayTime::create(1.f / 16.5f), CallFunc::create([&]() {
						if (_particles1Activated) dragEffect1->pauseEmissions();
						_particles1Activated = false;
						}), nullptr);
				action->setTag(2);
				runAction(action);
			}
		}
		if (_particles3Activated) { dragEffect3->pauseEmissions(); _particles3Activated = false; }
		if (_particles2Activated) { dragEffect2->pauseEmissions(); _particles2Activated = false; }
	}
	else
	{
		if (m_bIsHolding) {
			if (!_particles3Activated) dragEffect3->resumeEmissions();
			_particles3Activated = true;
		}
		else {
			if (_particles3Activated) dragEffect3->pauseEmissions();
			_particles3Activated = false;
		}
		if (!_particles2Activated) { dragEffect2->resumeEmissions(); _particles2Activated = true; }
		if (_particles1Activated) { dragEffect1->pauseEmissions(); _particles1Activated = false; }
	}

	if (!this->m_bIsLocked)
	{
		direction = clampf(direction, -1.f, 1.f);
		if (!m_bIsPlatformer) direction = 1.f;
		float dtSlow = dt * 0.9f;
		this->updateJump(dtSlow);
		float velY = (float)((double)dtSlow * m_dYVel);
		float velX = (float)((double)dt * m_dXVel * (!m_bIsPlatformer ? 1.f : direction) * getPlayerSpeed());
		ax::Vec2 velocity{ velX, velY };
		setPosition(getPosition() + velocity);
	}

	if (this->getPositionX() >= 500 && !this->inPlayLayer) this->m_bIsHolding = true;

	dragEffect1->setPosition(this->getPosition() + Vec2{ -10.f, flipMod() * -13.f });
	dragEffect2->setPosition(this->getPosition() + m_pShipSprite->getPosition() + Vec2{ -10.f, flipMod() * -3.f });
	dragEffect3->setPosition(dragEffect2->getPosition());
	shipDragEffect->setPosition(this->getPosition() + Vec2{ 1.f, flipMod() * -15.f });

	dragEffect1->setColor(getMainColor());
	shipDragEffect->setColor(getMainColor());

	_touchedRingObject = nullptr;
	_touchedPadObject = nullptr;
}

void PlayerObject::updateShipRotation(float dt)
{
	float angleRad, curAngleDeg, newAngleDeg;
	Vec2 pos = getPosition();
	Vec2 d = (pos - m_prevPos) / dt;

	if (GameToolbox::SquareDistance(0, 0, d.x, -d.y) >= 1.2f)
	{
		angleRad = atan2f(-d.y, d.x);
		angleRad *= _mini ? 1.2f : 1.f;
		curAngleDeg = getRotation();
		newAngleDeg = GameToolbox::iSlerp(curAngleDeg, angleRad * 57.296f, 0.15f, dt / 60.f);
		setRotation(newAngleDeg);
	}
}

void PlayerObject::spawnPortalCircle(ax::Color4B color, float radius)
{
	CircleWave* circle = CircleWave::create(0.3f, color, 5.f, radius, true, false);
	circle->setPosition(getPortalP());
	gameLayer->addChild(circle, 0);
}

void PlayerObject::deactivateStreak() {}
void PlayerObject::activateStreak() {}

void PlayerObject::setPlayerSpeed(float speed)
{
	m_playerSpeed = speed;
	if (speed == 0.9f) {
		m_dJumpHeight = 11.180032f;
		m_dGravity = 0.958199f;
		m_dXVel = 5.770002f;
	}
	else if (speed == 0.7f) {
		m_dJumpHeight = 10.620032f;
		m_dGravity = 0.940199f;
		m_dXVel = 5.980002f;
	}
	else if (speed == 1.1f) {
		m_dJumpHeight = 11.420032f;
		m_dGravity = 0.957199f;
		m_dXVel = 5.870002f;
	}
	else if (speed == 1.3f || speed == 1.6f) {
		m_dJumpHeight = 11.230032f;
		m_dGravity = 0.961199f;
		m_dXVel = 6.000002f;
	}
}

void PlayerObject::propellPlayer(double force)
{
	m_isRising = true;
	setIsOnGround(false);

	m_dYVel = flipMod() * 16 * force * (_vehicleSize == 1.0 ? 1.0 : 0.8);
	if (_currentGamemode == PlayerGamemodeBall || _currentGamemode == PlayerGamemodeSpider) {
		m_dYVel *= 0.6;
	}

	// Decompilaci�n boostPlayer: Control exacto de rotaci�n
	if (_currentGamemode == PlayerGamemodeBall) {
		runBallRotation();
	}
	else {
		runRotateAction();
	}

	setLastGroundPos(getPosition());
	activateStreak();
}

void PlayerObject::setTouchedRing(GameObject* obj) { _touchedRingObject = obj; }

void PlayerObject::ringJump(GameObject* obj)
{
	if (_touchedRingObject && _queuedHold && m_bIsHolding)
	{
		_touchedRingObject->triggerActivated(this);
		m_isRising = true;
		_queuedHold = false;
		setIsOnGround(false);

		double newYVel = m_dJumpHeight;
		int id = obj->getID();

		if (id == 1751 || obj->getGameObjectType() == kGameObjectTypeDropRing) {
			switch (_currentGamemode) {
			case PlayerGamemodeUFO: newYVel = -11.2 * flipMod(); break;
			case PlayerGamemodeShip:
			case PlayerGamemodeWave: newYVel = -14 * flipMod(); break;
			case PlayerGamemodeSpider: newYVel = -16.5 * flipMod(); break;
			default: newYVel = -15 * flipMod(); break;
			}
			m_dYVel = newYVel;
			activateStreak();
			if (_currentGamemode == PlayerGamemodeBall) m_bIsHolding = false;
			_touchedRingObject = nullptr;
			return;
		}
		else if (id == 1332 || obj->getGameObjectType() == kGameObjectTypeRedJumpRing) {
			switch (_currentGamemode) {
			case PlayerGamemodeShip: if (_vehicleSize != 1.0f) newYVel *= 1.4; break;
			case PlayerGamemodeUFO: if (_vehicleSize == 1.0f) newYVel *= 1.02; else newYVel *= 1.36; break;
			case PlayerGamemodeSpider:
			case PlayerGamemodeBall: newYVel *= 1.34; break;
			case PlayerGamemodeRobot: newYVel *= 1.28; break;
			default: newYVel *= 1.38; break;
			}
		}
		else if (id == 141 || obj->getGameObjectType() == kGameObjectTypePinkJumpRing) {
			switch (_currentGamemode) {
			case PlayerGamemodeShip: newYVel *= 0.37; break;
			case PlayerGamemodeUFO: newYVel *= 0.42; break;
			case PlayerGamemodeBall: newYVel *= 0.77; break;
			default: newYVel *= 0.72; break;
			}
		}
		else if (id == 84 || id == 1022 || obj->getGameObjectType() == kGameObjectTypeGravityRing) {
			newYVel *= 0.8;
			// FIX: NO volteamos la gravedad aquí. Conservamos la velocidad positiva
			// para que la orbe nos empuje suavemente hacia el nuevo techo.
		}
		else if (id == 1704 || obj->getGameObjectType() == kGameObjectTypeGreenRing) {
			if (_currentGamemode == PlayerGamemodeShip) newYVel *= 0.7;
			// FIX: La Orbe Verde SÍ voltea la gravedad primero. Esto hace que la velocidad
			// se vuelva negativa y nos haga saltar alejándonos del techo.
			flipGravity(!isGravityFlipped());
		}
		else {
			// Por defecto: Orbe Amarilla (ID 36) o Dash Orbs
			if (_currentGamemode == PlayerGamemodeRobot) newYVel *= 0.9;
		}

		newYVel *= flipMod();
		newYVel *= _vehicleSize < 1.f ? 0.8f : 1.f;
		m_dYVel = newYVel;

		if (_currentGamemode == PlayerGamemodeBall) {
			runBallRotation();
		}
		else {
			// FIX: La Orbe Azul (ID 84) NO te hace dar una voltereta (spin).
			if (id != 84 && id != 1022 && obj->getGameObjectType() != kGameObjectTypeGravityRing) {
				runRotateAction();
			}
		}

		_touchedRingObject = nullptr;
		_hasRingJumped = true;
		setLastGroundPos(getPosition());
		activateStreak();

		if (_currentGamemode == PlayerGamemodeBall || _currentGamemode == PlayerGamemodeSpider) {
			m_bIsHolding = false;
			m_dYVel *= 0.7;
		}

		// FIX: Volteamos la gravedad de la Orbe Azul al FINAL.
		if (id == 84 || id == 1022 || obj->getGameObjectType() == kGameObjectTypeGravityRing) {
			flipGravity(!isGravityFlipped());
		}
	}
}

void PlayerObject::flipGravity(bool gravity)
{
	if (m_bGravityFlipped != gravity)
	{
		m_bGravityFlipped = gravity;

		// FIX 1: Olvidar el bloque anterior para que el cálculo de snap no colapse.
		m_snappedObject = nullptr;

		m_dYVel /= 2.f;
		setScaleY(gravity ? std::abs(getScaleY()) * -1.f : std::abs(getScaleY()) * 1.f);
		activateStreak();

		dragEffect1->setAngle(dragEffect1->getAngle() + 180);
		dragEffect1->setGravity(Vec2{ dragEffect1->getGravity().x, -dragEffect1->getGravity().y });
		dragEffect2->setAngle(dragEffect2->getAngle() + 180);
		dragEffect2->setGravity(Vec2{ dragEffect2->getGravity().x, -dragEffect2->getGravity().y });
		dragEffect3->setAngle(dragEffect3->getAngle() + 180);
		dragEffect3->setGravity(Vec2{ dragEffect3->getGravity().x, -dragEffect3->getGravity().y });
		shipDragEffect->setAngle(shipDragEffect->getAngle() + 180);
		shipDragEffect->setGravity(Vec2{ shipDragEffect->getGravity().x, -shipDragEffect->getGravity().y });
	}
}

void PlayerObject::updateJump(float dt)
{
	float localGravity = m_dGravity;
	const int flipGravityMult = flipMod();
	float playerSize = _mini ? 0.8f : 1.0f;

	if (_currentGamemode == PlayerGamemodeShip || _currentGamemode == PlayerGamemodeUFO || _currentGamemode == PlayerGamemodeWave)
	{
		if (_mini) playerSize = 0.85f;
		float upperVelocityLimit = 8.0 / playerSize;
		float lowerVelocityLimit = -6.4 / playerSize;

		if (this->_currentGamemode == PlayerGamemodeShip) {
			float shipAccel = 0.8f;
			if (this->m_bIsHolding) shipAccel = -1.0f;
			if (!this->m_bIsHolding && !this->playerIsFalling()) shipAccel = 1.2f;
			float extraBoost = 0.4f;
			if (this->m_bIsHolding && this->playerIsFalling()) extraBoost = 0.5;
			this->m_dYVel -= localGravity * dt * flipGravityMult * shipAccel * extraBoost / playerSize;
		}
		else if (_currentGamemode == PlayerGamemodeUFO) {
			if (m_bIsHolding && _hasJustHeld) {
				_hasJustHeld = false;
				const float sizeMult = _mini ? 8.f : 7.f;
				double newVel = flipMod() * sizeMult * playerSize;
				if (!isGravityFlipped() && m_dYVel < newVel || newVel < m_dYVel) m_dYVel = newVel;
			}
			float gravityMult = 0.8f;
			if (!playerIsFalling()) gravityMult = 1.2f;
			m_dYVel -= localGravity * dt * flipMod() * gravityMult * 0.5 / playerSize;
		}

		if (!this->isGravityFlipped()) {
			if (this->m_dYVel <= lowerVelocityLimit) this->m_dYVel = lowerVelocityLimit;
		}
		else {
			if (this->m_dYVel <= -upperVelocityLimit) this->m_dYVel = -upperVelocityLimit;
			upperVelocityLimit = 6.4f / playerSize;
		}
		if (this->m_dYVel >= upperVelocityLimit) this->m_dYVel = upperVelocityLimit;
	}
	else
	{
		float gravityMultiplier = 1.0f;
		if (_currentGamemode == PlayerGamemodeBall) gravityMultiplier = 0.6f;
		bool shouldJump = m_bIsHolding;

		if (shouldJump && isOnGround()) {
			m_isRising = true;
			setIsOnGround(false);
			float jumpAccel = m_dJumpHeight;
			m_dYVel = flipGravityMult * jumpAccel * playerSize;

			if (_currentGamemode == PlayerGamemodeBall) {
				this->flipGravity(!this->isGravityFlipped());
				this->m_bIsHolding = false;
				this->m_dYVel *= 0.6;
			}
			else if (_currentGamemode == PlayerGamemodeCube) {
				if (!_touchedRingObject) _queuedHold = false;
				runRotateAction();
			}
		}
		else {
			if (m_isRising) {
				m_dYVel -= localGravity * dt * flipGravityMult * gravityMultiplier;
				if (playerIsFalling()) {
					m_isRising = false;
					setIsOnGround(false);
				}
			}
			else {
				if (!isGravityFlipped()) {
					if (m_dYVel < -m_dGravity * 2.f) m_bOnGround = false;
				}
				else {
					if (m_dYVel > m_dGravity * 2.f) m_bOnGround = false;
				}
				m_dYVel -= localGravity * dt * flipGravityMult * gravityMultiplier;
				m_dYVel = isGravityFlipped() ? std::min(m_dYVel, 15.0) : std::max(m_dYVel, -15.0);

				if (!isGravityFlipped()) { if (m_dYVel >= m_dGravity * 2.0f) return; }
				else { if (m_dYVel <= m_dGravity * 2.0f) return; }

				if (_currentGamemode != PlayerGamemodeBall && getActionByTag(0) == nullptr) runRotateAction();

				if (isGravityFlipped()) { if (m_dYVel >= -4.0) return; }
				else { if (m_dYVel <= 4.0) return; }
			}
		}
	}
}

bool PlayerObject::playerIsFalling() {
	// F�sicas reales decompiladas de playerIsFallingBugged
	float limit = m_dGravity + m_dGravity;

	if (m_bIsPlatformer || _currentGamemode == PlayerGamemodeWave) {
		return this->isGravityFlipped() ? this->m_dYVel > -limit : limit > this->m_dYVel;
	}
	else {
		float grav = m_dGravity;
		if (this->isGravityFlipped()) return this->m_dYVel > limit;
		else return (grav + grav) > this->m_dYVel;
	}
}

void PlayerObject::collidedWithObject(float dt, GameObject* obj)
{
	auto type = obj->getGameObjectType();
	bool isSolid = (type == kGameObjectTypeSolid || type == kGameObjectTypeSlope || type == kGameObjectTypeCollisionObject);
	if (obj->_isTrigger || !isSolid) {
		if (type == kGameObjectTypeHazard && getInnerBounds().intersectsRect(obj->getOuterBounds())) {
			if (auto layer = dynamic_cast<PlayLayer*>(getPlayLayer())) layer->destroyPlayer(this, obj);
		}
		return;
	}

	Rect rect = obj->getOuterBounds();
	Rect playerRectI = getInnerBounds();
	Rect playerRectO = _mini ? getOuterBounds(0.6f, 0.6f) : getOuterBounds();
	bool isPlatform = (rect.size.height < 25.0f);

	Vec2 pos = getPosition();
	float flipModV = flipMod();
	float mod = (_currentGamemode == PlayerGamemodeShip || _currentGamemode == PlayerGamemodeUFO || _currentGamemode == PlayerGamemodeWave) ? 6.0f : 10.0f;
	mod *= flipModV;

	float topP = (pos.y + (playerRectO.size.height * -0.5f * -flipModV)) - mod;
	float bottomP = (pos.y + (playerRectO.size.height * -0.5f * flipModV)) + mod;

	float MaxY = rect.getMaxY();
	float MinY = rect.getMinY();
	float MaxYP = playerRectO.getMaxY();
	float MinYP = playerRectO.getMinY();

	float t = topP;
	float b = bottomP;

	// FIX: Tu validación. Exigimos que el cubo esté FÍSICAMENTE sobre o debajo del objeto 
	// para permitir aterrizar. Esto mata el bug del teletransporte en las paredes.
	bool isOverlappingX = (playerRectO.getMaxX() > rect.getMinX() + 4.0f) &&
		(playerRectO.getMinX() < rect.getMaxX() - 4.0f);

	if (isGravityFlipped()) {
		if (_currentGamemode != PlayerGamemodeShip && _currentGamemode != PlayerGamemodeUFO) goto topCollision;
		t = bottomP;
		b = topP;
	}

	// --- COLISIÓN INFERIOR (Suelo Normal, Techo Invertido) ---
	if (b >= MaxY || t >= MaxY) {
		if (m_dYVel < 0.0f) {
			if (isPlatform && isGravityFlipped()) return;

			// Restauramos el chequeo lateral original (vital para la hitbox interna)
			playerRectI.origin.x = rect.origin.x;
			if (playerRectI.intersectsRect(rect)) {
				playerRectI.origin.x = pos.x;
				if (isPlatform) goto bypass_death_bottom;
				goto death;
			}

		bypass_death_bottom:
			// FIX DEFINITIVO: m_prevPos.y ya es la base exacta del jugador. 
			// Verificamos que legítimamente veníamos desde arriba del bloque.
			if (MaxYP >= (MinY + MaxY) / 2.f && m_prevPos.y >= MaxY - 8.0f) {
				setPositionY(MaxY - (_mini ? 6.0f : 0.0f));
				if (!isGravityFlipped()) {
					hitGround(_currentGamemode == PlayerGamemodeShip);
					checkSnapJumpToObject(obj);
				}
				else {
					m_dYVel = 0.0f;
					m_isRising = false;
				}
				pos = getPosition();
				playerRectI = getInnerBounds();
			}
		}
	}

	if (!isGravityFlipped()) {
		if (_currentGamemode != PlayerGamemodeShip && _currentGamemode != PlayerGamemodeUFO) goto death;
		t = bottomP;
		b = topP;
	}

topCollision:
	// --- COLISIÓN SUPERIOR (Techo Normal, Suelo Invertido) ---
	if (b <= MinY || t <= MinY) {
		if (m_dYVel > 0.0f) {
			if (isPlatform && !isGravityFlipped()) return;

			// Restauramos el chequeo lateral original
			playerRectI.origin.x = rect.origin.x;
			if (playerRectI.intersectsRect(rect)) {
				playerRectI.origin.x = pos.x;
				if (isPlatform) goto bypass_death_top;
				goto death;
			}

		bypass_death_top:
			// FIX DEFINITIVO: m_prevPos.y + height es el techo exacto del jugador.
			// Verificamos que veníamos desde abajo del bloque.
			if (MinYP <= (MinY + MaxY) / 2.f && (m_prevPos.y + playerRectO.size.height) <= MinY + 8.0f) {
				setPositionY(MinY - (_mini ? 24.0f : 30.0f));
				if (isGravityFlipped()) {
					hitGround(_currentGamemode == PlayerGamemodeShip);
					checkSnapJumpToObject(obj);
				}
				else {
					m_dYVel = 0.0f;
					m_isRising = false;
				}
				pos = getPosition();
				playerRectI = getInnerBounds();
			}
		}
	}

death:
	playerRectI.origin.x = pos.x;
	if (playerRectI.intersectsRect(rect) && !isPlatform) {
		if (auto layer = dynamic_cast<PlayLayer*>(getPlayLayer())) layer->destroyPlayer(this, obj);
	}
}

void PlayerObject::setGamemode(PlayerGamemode mode)
{
	if (_currentGamemode != mode)
	{
		_ufoSprite->setVisible(false);
		m_pShipSprite->setVisible(false);
		m_pMainSprite->setVisible(false);
		_ballSprite->setVisible(false);
		stopActionByTag(3);
		stopActionByTag(2);
		stopActionByTag(1);
		stopActionByTag(0);
		dragEffect1->pauseEmissions();
		dragEffect2->pauseEmissions();
		dragEffect3->pauseEmissions();
		switch (mode)
		{
		case PlayerGamemodeCube:
			setIsOnGround(false);
			m_pMainSprite->setVisible(true);
			m_pMainSprite->setPositionY(0);
			m_pMainSprite->setScale(1.0f);
			deactivateStreak();
			break;
		case PlayerGamemodeShip:
			m_pShipSprite->setVisible(true);
			m_pMainSprite->setVisible(true);
			m_pMainSprite->setScale(0.55f);
			m_pMainSprite->setPositionY(5);
			setRotation(0.f);
			m_dYVel /= 2.f;
			setIsOnGround(false);
			activateStreak();
			runRotateAction();
			break;
		case PlayerGamemodeBall:
			_ballSprite->setVisible(true);
			deactivateStreak();
			runBallRotation();
			break;
		case PlayerGamemodeUFO:
			m_pMainSprite->setVisible(true);
			m_pMainSprite->setScale(0.55f);
			m_pMainSprite->setPositionY(5);
			_ufoSprite->setVisible(true);
			deactivateStreak();
			break;
		default:
			break;
		}
		_currentGamemode = mode;
	}
}

void PlayerObject::checkSnapJumpToObject(GameObject* obj)
{
	if (obj)
	{
		if (m_snappedObject && m_snappedObject->getID() != obj->getID() &&
			m_snappedObject->getGameObjectType() == GameObjectType::kGameObjectTypeSolid)
		{
			auto oldSnapPos = m_snappedObject->getPosition();
			auto newSnapPos = obj->getPosition();

			// Valores matem�ticos exactos extra�dos de la decompilaci�n
			float upTwoGap = 90.0f, downOneGap = 150.0f, upOneGap = 90.0f, xShift = 1.0f;

			if (m_playerSpeed == 0.7f) { upTwoGap = 60.0f; downOneGap = 120.0f; upOneGap = 90.0f; }
			else if (m_playerSpeed == 1.1f) { xShift = 2.0f; upTwoGap = 120.0f; downOneGap = 195.0f; upOneGap = 150.0f; }
			else if (m_playerSpeed == 1.3f || m_playerSpeed == 1.6f) { xShift = 2.0f; upTwoGap = 135.0f; downOneGap = 225.0f; upOneGap = 180.0f; }
			else { upOneGap = 120.0f; }

			float someMultiplier = (isGravityFlipped() ? 30.0f : -30.0f);
			bool snap = false;

			if (xShift >= std::abs(newSnapPos.y - (oldSnapPos.y + upOneGap))) {
				int v11 = !isGravityFlipped() ? 1 : -1;
				if (xShift >= std::abs(newSnapPos.x - ((v11 * 30.0f) + oldSnapPos.x))) snap = true;
			}
			if (!snap && xShift >= std::abs(newSnapPos.y - (oldSnapPos.y + downOneGap))) {
				int v12 = !isGravityFlipped() ? 1 : -1;
				if (xShift >= std::abs(newSnapPos.x - (oldSnapPos.x - (v12 * 30.0f)))) snap = true;
			}
			if (!snap && xShift >= std::abs(newSnapPos.y - (oldSnapPos.y + upTwoGap))) {
				int v10 = !isGravityFlipped() ? 1 : -1;
				if (xShift >= std::abs(newSnapPos.x - ((v10 * 60.0f) + oldSnapPos.x))) snap = true;
			}

			if (snap) {
				float newPos = obj->getPositionX() + this->m_snapDifference;
				float oldPos = this->getPositionX();
				if (xShift < std::abs(newPos - oldPos)) {
					if (newPos > oldPos) newPos = oldPos + xShift;
					else newPos = oldPos - xShift;
				}
				this->setPositionX(newPos);
			}
		}
		m_snappedObject = obj;
		m_snapDifference = this->getPositionX() - obj->getPositionX();
	}
}

void PlayerObject::hitGround(bool reverseGravity)
{
	m_dYVel = 0.0f;
	m_isRising = false; // El motor corta el ascenso forzosamente

	if (!isOnGround() && !reverseGravity) {
		landEffect1->setPosition(getPosition() + Vec2{ 0.f, flipMod() * -15.f });
		landEffect1->resetSystem();
		landEffect1->start();
	}

	// Reseteo duro de rotaciones
	if (_currentGamemode == PlayerGamemodeCube) {
		if (getActionByTag(0)) stopRotation();
	}
	else if (_currentGamemode == PlayerGamemodeBall && !isOnGround()) {
		runBallRotation();
	}

	_queuedHold = false;
	setIsOnGround(true);

	m_obLastGroundPos = getPosition();
	if (_currentGamemode != PlayerGamemode::PlayerGamemodeShip) deactivateStreak();
}

float PlayerObject::flipMod() { return this->m_bGravityFlipped ? -1.0f : 1.0f; }
bool PlayerObject::isGravityFlipped() { return this->m_bGravityFlipped; }
bool PlayerObject::isDead() { return this->m_bIsDead; }
bool PlayerObject::isOnGround() { return this->m_bOnGround; }
ax::Vec2 PlayerObject::getLastGroundPos() { return this->m_obLastGroundPos; }

void PlayerObject::logValues() {
	GameToolbox::log("xVel: {} | yVel: {} | gravity: {} | jumpHeight: {} ", m_dXVel, m_dYVel, m_dGravity, m_dJumpHeight);
}

void PlayerObject::runRotateAction() {
	stopRotation();
	auto action = RotateBy::create(0.41f * (_mini ? 0.8f : 1.f), 180.f * flipMod());
	action->setTag(0);
	runAction(action);
}

void PlayerObject::runBallRotation() {
	if (getActionByTag(3)) stopActionByTag(3);
	auto action = RotateBy::create(0.5f, 360.f * flipMod());
	auto loop = RepeatForever::create(action);
	loop->setTag(3);
	runAction(loop);
}

void PlayerObject::stopRotation() {
	stopActionByTag(0);
	if (getActionByTag(1) == nullptr) {
		if (getRotation() != 0) {
			int degrees = (int)getRotation() % 360;
			auto action = RotateTo::create(0.075f, (90 * roundf(degrees / 90.0f)));
			action->setTag(1);
			runAction(action);
		}
	}
}

void PlayerObject::jump() { this->m_dYVel = this->m_dJumpHeight; }

void PlayerObject::toggleMini(bool active) {
	_mini = active;
	_vehicleSize = active ? 0.6f : 1.f;
	auto ac = ScaleTo::create(0.5f, active ? 0.6f : 1.f);
	auto bounce = EaseBounceOut::create(ac);
	this->runAction(bounce);
}

void PlayerObject::pushButton() {
	if (this->inPlayLayer) {
		m_bIsHolding = true;
		_hasJustHeld = true;
		_queuedHold = true;
	}
	if ((_currentGamemode == PlayerGamemodeCube || _currentGamemode == PlayerGamemodeRobot) && m_bOnGround) _jumpedTimes++;
}

void PlayerObject::releaseButton() {
	if (this->inPlayLayer) {
		_queuedHold = false;
		_hasJustHeld = false;
		m_bIsHolding = false;
	}
}