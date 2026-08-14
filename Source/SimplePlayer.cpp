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

#include "GameToolbox/enums.h"
#include "SimplePlayer.h"

#include "UTF8.h"
#include "base/Director.h"

#include "GameToolbox/math.h"
#include "GameToolbox/conv.h"
#include "2d/SpriteFrameCache.h"

USING_NS_AX;

bool SimplePlayer::init(int cubeID) {
	if (!Sprite::init()) return false;

	this->updateGamemode(cubeID, IconType::kIconTypeCube);
	this->setContentSize({ 60, 60 });
	this->setAnchorPoint({ 0.5f, 0.5f });

	return true;
}

void SimplePlayer::updateGamemode(int iconID, IconType mode) {
	iconID = GameToolbox::inRange(iconID, 1, GameToolbox::getValueForGamemode(mode));

	auto tipo = GameToolbox::getNameGamemode(mode);

	auto mainFrame = StringUtils::format("%s_%02d_001.png", tipo, iconID);
	auto secFrame = StringUtils::format("%s_%02d_2_001.png", tipo, iconID);
	auto extFrame = StringUtils::format("%s_%02d_extra_001.png", tipo, iconID);
	auto glowFrame = StringUtils::format("%s_%02d_glow_001.png", tipo, iconID);
	auto domeFrame = StringUtils::format("%s_%02d_3_001.png", tipo, iconID);

	if (m_pMainSprite) this->removeChild(m_pMainSprite);
	if (m_pSecondarySprite) this->removeChild(m_pSecondarySprite);
	if (m_pGlowSprite) this->removeChild(m_pGlowSprite);
	if (m_pExtraSprite) this->removeChild(m_pExtraSprite);
	if (m_pDomeSprite) this->removeChild(m_pDomeSprite);

	auto frameCache = ax::SpriteFrameCache::getInstance();

	// 1. MAIN COLOR (Capa Base)
	if (frameCache->getSpriteFrameByName(mainFrame)) {
		m_pMainSprite = Sprite::createWithSpriteFrameName(mainFrame);
	}
	else {
		m_pMainSprite = Sprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
	}

	// Centramos el sprite base en nuestro contenedor de 60x60
	m_pMainSprite->setPosition(this->getContentSize() / 2);
	m_pMainSprite->setScale(1.0f);

	this->addChild(m_pMainSprite);

	// 2. SECONDARY COLOR
	if (frameCache->getSpriteFrameByName(secFrame)) {
		m_pSecondarySprite = Sprite::createWithSpriteFrameName(secFrame);
	}
	else {
		m_pSecondarySprite = Sprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
	}
	// Al añadirlo como hijo, su posición es relativa al centro de m_pMainSprite
	m_pSecondarySprite->setPosition(m_pMainSprite->getContentSize() / 2);
	m_pMainSprite->addChild(m_pSecondarySprite, -1);

	// 3. GLOW
	if (frameCache->getSpriteFrameByName(glowFrame)) {
		m_pGlowSprite = Sprite::createWithSpriteFrameName(glowFrame);
	}
	else {
		m_pGlowSprite = Sprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
	}
	m_pGlowSprite->setPosition(m_pMainSprite->getContentSize() / 2);
	m_pGlowSprite->setVisible(m_bHasGlow);
	m_pMainSprite->addChild(m_pGlowSprite, -2);

	// 4. EXTRA (Detalles)
	if (frameCache->getSpriteFrameByName(extFrame)) {
		m_pExtraSprite = Sprite::createWithSpriteFrameName(extFrame);
		m_pExtraSprite->setPosition(m_pMainSprite->getContentSize() / 2);
		m_pMainSprite->addChild(m_pExtraSprite, 1);
	}

	// 5. DOME (Cúpula del UFO)
	if (mode == IconType::kIconTypeUfo) {
		if (frameCache->getSpriteFrameByName(domeFrame)) {
			m_pDomeSprite = Sprite::createWithSpriteFrameName(domeFrame);
			m_pDomeSprite->setPosition(m_pMainSprite->getContentSize() / 2);
			m_pMainSprite->addChild(m_pDomeSprite, -1);
		}
	}

	this->updateIconColors();
}

void SimplePlayer::updateIconColors() {
	m_pMainSprite->setColor(m_MainColor);
	m_pSecondarySprite->setColor(m_SecondaryColor);
	m_pGlowSprite->setColor(m_GlowColor);
}

void SimplePlayer::setMainColor(Color3B col) {
	m_pMainSprite->setColor(col);
	m_MainColor = col;
}

void SimplePlayer::setSecondaryColor(Color3B col) {
	m_pSecondarySprite->setColor(col);
	m_SecondaryColor = col;
}

void SimplePlayer::setGlow(bool glow) {
	m_pGlowSprite->setVisible(glow);
	m_bHasGlow = glow;
}

void SimplePlayer::setGlowColor(Color3B col) {
	m_pGlowSprite->setColor(col);
	m_GlowColor = col;
}

SimplePlayer* SimplePlayer::create(int cubeID) {
	auto pRet = new (std::nothrow) SimplePlayer();

	if (pRet && pRet->init(cubeID)) {
		pRet->autorelease();
		return pRet;
	}
	AX_SAFE_DELETE(pRet);
	return pRet;
}
