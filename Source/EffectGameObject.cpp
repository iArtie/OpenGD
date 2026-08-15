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

#include "EffectGameObject.h"
#include "2d/ActionEase.h"
#include "BaseGameLayer.h"
#include "ColorAction.h"
#include "GameToolbox/conv.h"
#include "GameToolbox/log.h"
#include "GroupColorAction.h"
#include "PlayLayer.h"

USING_NS_AX;

Action* EffectGameObject::actionEasing(ActionInterval* ac, int ease, float rate)
{
	switch (ease)
	{
	case 1:
		return EaseInOut::create(ac, rate);
		break;
	case 2:
		return EaseIn::create(ac, rate);
		break;
	case 3:
		return EaseOut::create(ac, rate);
		break;
	case 4:
		return EaseElasticInOut::create(ac, rate);
		break;
	case 5:
		return EaseElasticIn::create(ac, rate);
		break;
	case 6:
		return EaseElasticOut::create(ac, rate);
		break;
	case 7:
		return EaseBounceInOut::create(ac);
		break;
	case 8:
		return EaseBounceIn::create(ac);
		break;
	case 9:
		return EaseBounceOut::create(ac);
		break;
	case 10:
		return EaseExponentialInOut::create(ac);
		break;
	case 11:
		return EaseExponentialIn::create(ac);
		break;
	case 12:
		return EaseExponentialOut::create(ac);
		break;
	case 13:
		return EaseSineInOut::create(ac);
		break;
	case 14:
		return EaseSineIn::create(ac);
		break;
	case 15:
		return EaseSineOut::create(ac);
		break;
	case 16:
		return EaseBackInOut::create(ac);
		break;
	case 17:
		return EaseBackIn::create(ac);
		break;
	case 18:
		return EaseBackOut::create(ac);
		break;
	}
	return ac;
}

void EffectGameObject::triggerActivated(float)
{
	if (!_bgl || (_wasTriggerActivated && !_spawnTriggered))
		return;

	auto pl = PlayLayer::getInstance();

	// FIX (bug #2): esta linea estaba comentada en el codigo que tenias antes,
	// asi que el guard de arriba nunca bloqueaba nada - todo trigger no-spawn
	// se re-disparaba en cada frame mientras estuviera en la ventana de
	// updateTriggers(), reiniciando ColorAction/GroupColorAction/
	// removeFromGameLayer sin parar. _wasTriggerActivated tambien tiene
	// default = false en el header ahora (antes quedaba sin inicializar).
	this->_wasTriggerActivated = true;

	auto id = getID();

	if (id == 29)
		_targetColorId = 1000;
	else if (id == 30)
		_targetColorId = 1001;

	if (!_bgl->_colorChannels.contains(_targetColorId))
	{
		_bgl->_colorChannels.insert({ _targetColorId, SpriteColor(Color3B::WHITE, 255, 0) });
		_bgl->_originalColors.insert({ _targetColorId, SpriteColor(Color3B::WHITE, 255, 0) });
	}

	switch (id)
	{
	case 30:
	case 29:
	case 899: {

		_bgl->_colorChannels.at(_targetColorId)._applyHsv = false;
		if (_copiedColorId > -1)
		{
			int cycles = 0;

			if (!_bgl->_colorChannels.contains(_copiedColorId))
			{
				_bgl->_colorChannels.insert({ _copiedColorId, SpriteColor(Color3B::WHITE, 255, 0) });
				_bgl->_originalColors.insert({ _copiedColorId, SpriteColor(Color3B::WHITE, 255, 0) });
			}

			auto colorChannel = _bgl->_colorChannels.at(_copiedColorId);

			while (colorChannel._copyingColorID != -1 && cycles < 3)
			{
				cycles++;
				// FIX (bug #3 recurrence): operator[] crudo aca reinsertaba un
				// SpriteColor negro si el siguiente eslabon de la cadena de copia
				// todavia no existia en _colorChannels. getColorChannel() defaultea
				// a BLANCO como el resto de los accesos ya parcheados.
				colorChannel = _bgl->getColorChannel(colorChannel._copyingColorID);
			}

			_color = colorChannel._color;

			GameToolbox::applyHSV(_hsv, &_color);

			_bgl->_colorChannels.at(_targetColorId)._applyHsv = true;
		}

		_bgl->runAction(ColorAction::create(
			_duration, &_bgl->_colorChannels.at(_targetColorId), _bgl->_colorChannels.at(_targetColorId)._color, _color,
			_bgl->_colorChannels.at(_targetColorId)._opacity, _opacity * 255.0f, _copiedColorId, &_hsv));
		_bgl->_colorChannels.at(_targetColorId)._blending = _blending;
	}
			break;
	case 22:
		if (pl)
			pl->_enterEffectID = 1;
		break;
	case 23:
		if (pl)
			pl->_enterEffectID = 5;
		break;
	case 24:
		if (pl)
			pl->_enterEffectID = 4;
		break;
	case 25:
		if (pl)
			pl->_enterEffectID = 6;
		break;
	case 26:
		if (pl)
			pl->_enterEffectID = 7;
		break;
	case 27:
		if (pl)
			pl->_enterEffectID = 2;
		break;
	case 28:
		if (pl)
			pl->_enterEffectID = 3;
		break;
	case 901: {
		_bgl->_effectManager->runMoveCommand(_duration, _offset, _easing, _easeRate, _targetGroupId);
	}
			break;
	case 1007:
		runAction(ActionTween::create(_duration, "fade", _bgl->_groups[_targetGroupId]._alpha, _opacity));
		break;
	case 1006: {

		if (!_bgl->_colorChannels.contains(_targetGroupId))
		{
			_bgl->_colorChannels.insert({ _targetGroupId, SpriteColor(Color3B::WHITE, 255, 0) });
			_bgl->_originalColors.insert({ _targetGroupId, SpriteColor(Color3B::WHITE, 255, 0) });
		}
		Color3B original = _bgl->_colorChannels.at(_targetGroupId)._color;

		Color3B target = _color;

		if (_pulseMode) // hsv
		{
			if (_copiedColorId == -1)
				target = original;
			else
			{
				if (!_bgl->_colorChannels.contains(_copiedColorId))
				{
					_bgl->_colorChannels.insert({ _copiedColorId, SpriteColor(Color3B::WHITE, 255, 0) });
					_bgl->_originalColors.insert({ _copiedColorId, SpriteColor(Color3B::WHITE, 255, 0) });
				}
				int cycles = 0;

				auto colorChannel = _bgl->_colorChannels.at(_targetGroupId);

				while (colorChannel._copyingColorID != -1 && cycles < 3)
				{
					cycles++;
					// FIX (bug #3 recurrence, ver case 899/29/30 arriba): mismo
					// operator[] crudo, mismo fix.
					colorChannel = _bgl->getColorChannel(colorChannel._copyingColorID);
				}

				target = colorChannel._color;
			}

			GameToolbox::applyHSV(_hsv, &target);
		}

		ax::Sequence* seq;

		if (_pulseType == 0)
		{
			auto colPointer = &_bgl->_colorChannels.at(_targetGroupId);
			seq = ax::Sequence::create({ ColorAction::create(_fadeIn, colPointer, original, target),
										ColorAction::create(_hold, colPointer, target, target),
										ColorAction::create(_fadeOut, colPointer, target, original) });
		}
		else
		{
			if (!_bgl->_groups.contains(_targetGroupId))
			{
				GroupProperties gp;
				_bgl->_groups.insert({ _targetGroupId, gp });
			}
			auto groupPointer = &_bgl->_groups.at(_targetGroupId);

			if (!_mainOnly && !_detailOnly)
				groupPointer->groupState = GroupProperties::GroupState::MAIN_DETAIL;
			else if (_mainOnly)
				groupPointer->groupState = GroupProperties::GroupState::MAIN_ONLY;
			else
				groupPointer->groupState = GroupProperties::GroupState::DETAIL_ONLY;

			seq = ax::Sequence::create({ GroupColorAction::create(_fadeIn, groupPointer, original, target, false),
										GroupColorAction::create(_hold, groupPointer, target, target, false),
										GroupColorAction::create(_fadeOut, groupPointer, target, original, true) });
		}

		// Nuevo: tag = grupo objetivo, para que Stop Trigger (case 1616) pueda
		// cancelar un Pulse en curso con stopAllActionsByTag.
		seq->setTag(_targetGroupId);
		_bgl->runAction(seq);
		break;
	}
	case 1049: // Toggle Trigger
		// Antes esto duplicaba el loop de activar/desactivar objetos inline.
		// Ahora usa el helper compartido de BaseGameLayer (tambien usado por
		// Count Trigger / Instant Count Trigger de abajo).
		_bgl->activateGroup(_targetGroupId, _activateGroup);
		break;
	case 1268:
		scheduleOnce(
			[&](float dt) {
				for (GameObject* gameObj : _bgl->_groups[_targetGroupId]._objects)
				{
					if (gameObj->_isTrigger)
					{
						EffectGameObject* trigger = static_cast<EffectGameObject*>(gameObj);
						if (trigger->_spawnTriggered)
							trigger->triggerActivated(dt);
					}
				}
			},
			_spawnDelay, "time");

		break;

		// --- Nuevo: Stop Trigger ---
		// Cancela cualquier accion corriendo sobre el BaseGameLayer con tag ==
		// targetGroupID. Cubre Move Trigger y Pulse Trigger (ambas se taggean con
		// el grupo objetivo arriba y en EffectManager::runMoveCommand). No cubre
		// Follow/Animate/Shake porque esos triggers todavia no estan portados.
	case 1616:
		_bgl->stopAllActionsByTag(_targetGroupId);
		break;

		// --- Nuevo: Pickup Trigger ---
	case 1817:
		_bgl->modifyItemValue(_itemID, _subtractCount ? -_count : _count);
		break;

		// --- Nuevo: Count Trigger / Instant Count Trigger ---
		// Simplificado a comparacion ">=" (ver nota en EffectGameObject.h sobre
		// por que no se implemento la comparacion real de GD todavia). El Count
		// Trigger "clasico" de GD tambien puede quedar escuchando en segundo
		// plano sin ser tocado por el jugador; eso requeriria polling por frame
		// en BaseGameLayer::update(), que sigue siendo un no-op ({}) - no lo
		// active para no introducir comportamiento no pedido/no confirmado.
	case 1611:
	case 1811:
		if (_bgl->getItemValue(_itemID) >= _count)
			_bgl->activateGroup(_targetGroupId, _activateGroup);
		break;

		// --- Pendiente, requieren infraestructura que no esta en los archivos
		// que tengo (touch input, colision par-a-par, scheduler por-frame):
		// case 1595: // Touch Trigger
		// case 1815: // Collision Trigger
		// case 1816: // Collision Block (no es un trigger de activacion, es pasivo)
		// case 1347: // Follow Trigger
		// case 1814: // Follow Player Y
		// case 1585: // Animate Trigger
		// case 1520: // Shake Trigger
	}
}

void EffectGameObject::updateTweenAction(float value, std::string_view key)
{
	if (!_bgl)
		return;

	if (key == "col1")
	{
		_bgl->_colorChannels.at(_targetColorId)._color.r = static_cast<uint8_t>(value);
	}
	else if (key == "col2")
	{
		_bgl->_colorChannels.at(_targetColorId)._color.g = static_cast<uint8_t>(value);
	}
	else if (key == "col3")
	{
		_bgl->_colorChannels.at(_targetColorId)._color.b = static_cast<uint8_t>(value);
	}
	else if (key == "col4")
	{
		_bgl->_colorChannels.at(_targetColorId)._opacity = value;
	}
	else if (key == "pul1")
	{
		_bgl->_colorChannels.at(_targetGroupId)._color.r = static_cast<uint8_t>(value);
	}
	else if (key == "pul2")
	{
		_bgl->_colorChannels.at(_targetGroupId)._color.g = static_cast<uint8_t>(value);
	}
	else if (key == "pul3")
	{
		_bgl->_colorChannels.at(_targetGroupId)._color.b = static_cast<uint8_t>(value);
	}
	else if (key == "fade")
	{
		_bgl->_groups[_targetGroupId]._alpha = static_cast<uint8_t>(value);
	}
}

EffectGameObject* EffectGameObject::create(std::string_view frame)
{
	auto pRet = new (std::nothrow) EffectGameObject();

	if (pRet && pRet->init(frame))
	{
		pRet->_bgl = BaseGameLayer::getInstance();
		// pRet->setVisible(false);
		pRet->autorelease();
		return pRet;
	}
	AX_SAFE_DELETE(pRet);
	return nullptr;
}