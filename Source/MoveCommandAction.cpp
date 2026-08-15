///*************************************************************************
//	OpenGD - Open source Geometry Dash.
//	Copyright (C) 2023  OpenGD Team
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.
//*************************************************************************/
//
//#include "MoveCommandAction.h"
//
//USING_NS_AX;
//
//MoveCommandAction* MoveCommandAction::create(float duration, GroupProperties* group, ax::Vec2 startOffset, ax::Vec2 endOffset)
//{
//	auto ret = new (std::nothrow) MoveCommandAction();
//	if (ret && ret->initWithDuration(duration, group, startOffset, endOffset))
//	{
//		ret->autorelease();
//		return ret;
//	}
//	delete ret;
//	return nullptr;
//}
//
//bool MoveCommandAction::initWithDuration(float duration, GroupProperties* group, ax::Vec2 startOffset, ax::Vec2 endOffset)
//{
//	// Duracion 0 es valida en GD (movimiento instantaneo), pero
//	// ActionInterval::initWithDuration con 0 puede dar t = NaN en algunas
//	// implementaciones al dividir por _duration en update(). Usamos un
//	// epsilon chico para evitar eso sin cambiar el comportamiento percibido.
//	if (!ActionInterval::initWithDuration(duration <= 0.f ? 0.0001f : duration))
//		return false;
//
//	_group = group;
//	_startOffset = startOffset;
//	_endOffset = endOffset;
//	return true;
//}
//
//void MoveCommandAction::update(float time)
//{
//	if (!_group)
//		return;
//
//	_group->_offset = _startOffset + (_endOffset - _startOffset) * time;
//}
//
//MoveCommandAction* MoveCommandAction::clone() const
//{
//	return MoveCommandAction::create(_duration, _group, _startOffset, _endOffset);
//}
//
//MoveCommandAction* MoveCommandAction::reverse() const
//{
//	return MoveCommandAction::create(_duration, _group, _endOffset, _startOffset);
//}
