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

#include "BoomScrollLayer.h"
#include "GJGameLevel.h"
#include "MenuItemSpriteExtra.h"
#include "AudioEngine.h"
#include "2d/ActionEase.h"
#include "Director.h"
#include "EventDispatcher.h"
#include "GameToolbox/log.h"
#include "base/Utils.h" 
#include "EventListenerTouch.h" 

USING_NS_AX;

BoomScrollLayer* BoomScrollLayer::create(std::vector<ax::Layer*> layers, int currentPage)
{
	BoomScrollLayer* pRet = new BoomScrollLayer();
	if (pRet->init(layers, currentPage))
	{
		pRet->autorelease();
		return pRet;
	}
	AX_SAFE_DELETE(pRet);
	return nullptr;
}

bool BoomScrollLayer::init(std::vector<ax::Layer*> layers, int currentPage)
{
	if (!Layer::init()) return false;

	_totalPages = layers.size();
	_currentPage = std::clamp(currentPage, 0, _totalPages - 1);
	_layers = layers;

	// Capa gigante que contendrá todas las páginas en fila
	_internalLayer = Layer::create();
	_internalLayer->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	this->addChild(_internalLayer);

	// Etiqueta crucial para que LevelSelectLayer pueda leer la posición y cambiar el color
	_internalLayer->setTag(1234);

	auto winSize = Director::getInstance()->getWinSize();

	// Colocamos TODAS las páginas una al lado de la otra (Método RobTop)
	for (int i = 0; i < _totalPages; i++) {
		auto l = _layers[i];
		l->setPosition({ winSize.width * i, 0 });
		_internalLayer->addChild(l);
	}

	// Movemos la cámara a la página inicial
	_internalLayer->setPositionX(-_currentPage * winSize.width);

	// Sistema táctil
	auto listener = ax::EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = AX_CALLBACK_2(BoomScrollLayer::onTouchBegan, this);
	listener->onTouchMoved = AX_CALLBACK_2(BoomScrollLayer::onTouchMoved, this);
	listener->onTouchEnded = AX_CALLBACK_2(BoomScrollLayer::onTouchEnded, this);
	listener->onTouchCancelled = AX_CALLBACK_2(BoomScrollLayer::onTouchCancelled, this);

	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

	return true;
}

void BoomScrollLayer::animateToPage(int page)
{
	// Limitamos la página para no salirnos de los bordes
	_currentPage = std::clamp(page, 0, _totalPages - 1);

	auto winSize = Director::getInstance()->getWinSize();
	float targetX = -_currentPage * winSize.width;

	// Movemos toda la capa gigante con un solo rebote elástico
	_internalLayer->stopAllActions();
	auto move = ax::MoveTo::create(0.5f, { targetX, _internalLayer->getPositionY() });
	_internalLayer->runAction(ax::EaseElasticOut::create(move, 0.6f));
}

void BoomScrollLayer::changePageRight() { animateToPage(_currentPage + 1); }
void BoomScrollLayer::changePageLeft() { animateToPage(_currentPage - 1); }
void BoomScrollLayer::selectPage(int current) { animateToPage(current - 1); }

bool BoomScrollLayer::onTouchBegan(ax::Touch* touch, ax::Event* event)
{
	_touching = true;
	_touchBeganPoint = touch->getLocation();
	_touchLastPoint = _touchBeganPoint;
	_touchStartTime = utils::getTimeInMilliseconds() / 1000.0f;

	_internalLayer->stopAllActions(); // Frena si lo tocas mientras rebotaba
	return true;
}

void BoomScrollLayer::onTouchMoved(ax::Touch* touch, ax::Event* event)
{
	if (!_touching) return;
	auto currentPoint = touch->getLocation();
	float deltaX = currentPoint.x - _touchLastPoint.x;

	// Desplazamiento 1:1 con el dedo
	_internalLayer->setPositionX(_internalLayer->getPositionX() + deltaX);
	_touchLastPoint = currentPoint;
}

void BoomScrollLayer::onTouchEnded(ax::Touch* touch, ax::Event* event)
{
	_touching = false;
	auto endPoint = touch->getLocation();
	float totalDrag = endPoint.x - _touchBeganPoint.x;
	float timeDiff = std::max((utils::getTimeInMilliseconds() / 1000.0f) - _touchStartTime, 0.01f);
	_velocity = totalDrag / timeDiff;

	auto winSize = Director::getInstance()->getWinSize();

	// Calculamos en qué página virtual estamos soltando el dedo
	float exactPage = -_internalLayer->getPositionX() / winSize.width;
	int targetPage = std::round(exactPage);

	// Si deslizó rápido, forzamos el cambio aunque no haya llegado a la mitad
	if (totalDrag < -_minimumTouchLengthToChangePage || _velocity < -_swipeThresholdVelocity) {
		targetPage = std::ceil(exactPage);
	}
	else if (totalDrag > _minimumTouchLengthToChangePage || _velocity > _swipeThresholdVelocity) {
		targetPage = std::floor(exactPage);
	}

	animateToPage(targetPage);
}

void BoomScrollLayer::onTouchCancelled(ax::Touch* touch, ax::Event* event) {
	onTouchEnded(touch, event);
}

void BoomScrollLayer::onExit()
{
	Director::getInstance()->getEventDispatcher()->removeEventListenersForTarget(this);
	Layer::onExit();
}