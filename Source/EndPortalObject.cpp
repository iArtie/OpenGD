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

#include "EndPortalObject.h"
#include "base/Director.h"
#include "2d/ParticleSystemQuad.h"
#include "PlayLayer.h"
#include <cmath>

USING_NS_AX;

EndPortalObject* EndPortalObject::create()
{
	auto pRet = new (std::nothrow) EndPortalObject();

	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	AX_SAFE_DELETE(pRet);
	return nullptr;
}

bool EndPortalObject::init()
{
	if (!GameObject::init("block001_01_001.png", ""))
		return false;

	setGameObjectType(kGameObjectTypeDecoration);
	_zLayer = 10; // Default Z Order for portals

	// Create the end particle effect
	createAndAddParticle("endEffectPortal.plist", 4);
	if (_particle) {
		// Set specific particle offsets mimicking the decompiled code
		_startPosOffset = ax::Vec2(-15.0f, 0.0f);
	}

	auto winSize = Director::getInstance()->getWinSize();

	_gradientBar = Sprite::createWithSpriteFrameName("gradientBar.png");
	if (_gradientBar) {
		_gradientBar->setBlendFunc({ ax::backend::BlendFactor::SRC_ALPHA, ax::backend::BlendFactor::ONE }); // Additive blending
		addChild(_gradientBar, -1);
	}

	// Calculate how many squares to spawn based on window size
	float targetHeight = winSize.height * 1.5f;
	int numSquares = static_cast<int>(std::round(targetHeight / 30.0f));

	for (int i = 0; i < numSquares; ++i) {
		auto sq = Sprite::createWithSpriteFrameName("square_02_001.png");
		if (sq) {
			addChild(sq);
			// Replicating the positioning loop from the decompilation
			sq->setPosition(ax::Vec2(0.0f, (numSquares / 2.0f) * 30.0f - i * 30.0f));
			_squares.push_back(sq);
		}
	}

	return true;
}

void EndPortalObject::setPosition(const ax::Vec2& pos)
{
	GameObject::setPosition(pos);

	float mod = isFlippedX() ? -1.0f : 1.0f;
	float scale = 1.0f;

	if (auto bgl = PlayLayer::getInstance()) {
		scale = bgl->getScale();
		if (scale == 0.0f) scale = 1.0f; // Prevent division by zero
	}

	if (_gradientBar) {
		_gradientBar->setPosition(pos + ax::Vec2((-14.0f - (15.0f / scale)) * mod, 0.0f));
	}

	if (_particle) {
		_particle->setPosition(pos + ax::Vec2(-15.0f * mod, 0.0f));
	}
}

void EndPortalObject::setVisible(bool visible)
{
	bool wasVisible = isVisible();
	GameObject::setVisible(visible);

	if (visible && !wasVisible) {
		updateEndPos(true);
	}
}

void EndPortalObject::updateEndPos(bool updateParticle)
{
	float y = 225.0f;

	// In the original, it evaluates whether the starting pos height is valid
	if (getStartPositionY() >= 225.0f) {
		y = getStartPositionY();
	}

	ax::Vec2 newPos(getPositionX(), y);
	setStartPosition(newPos);

	if (_particle) {
		if (updateParticle) {
			_particle->update(0.0f);
		}
		_particle->setScale(getScale());
	}

	if (_gradientBar) {
		_gradientBar->setScaleX(getScaleX());
		_gradientBar->setScaleY(getScaleY());
	}

	setScaleX(getScaleX());
	setScaleY(getScaleY());

	GameObject::setPosition(newPos);
}

ax::Vec2 EndPortalObject::getSpawnPos()
{
	float mod = isFlippedX() ? -1.0f : 1.0f;
	float scale = 1.0f;

	if (auto bgl = PlayLayer::getInstance()) {
		scale = bgl->getScale();
		if (scale == 0.0f) scale = 1.0f;
	}

	// This exactly mirrors the math in EndPortalObject::getSpawnPos from the IDA output
	return ax::Vec2(getStartPositionX() - (300.0f / scale) * mod, getStartPositionY());
}