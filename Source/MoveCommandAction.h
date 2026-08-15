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

// Nueva: no existia en el proyecto. Anima GroupProperties::_offset entre dos
// valores a lo largo de una duracion. Se necesita porque GroupProperties no
// es un ax::Node (no tiene setPosition), asi que no podemos usar MoveTo.
// Sigue el mismo patron que ColorAction/GroupColorAction (Action operando
// sobre un puntero a struct en vez de sobre un Node).

#pragma once
#include "axmol.h"
#include "BaseGameLayer.h" // GroupProperties

class MoveCommandAction : public ax::ActionInterval
{
public:
	static MoveCommandAction* create(float duration, GroupProperties* group, ax::Vec2 startOffset, ax::Vec2 endOffset);

	bool initWithDuration(float duration, GroupProperties* group, ax::Vec2 startOffset, ax::Vec2 endOffset);

	void update(float time) override;
	MoveCommandAction* clone() const override;
	MoveCommandAction* reverse() const override;

protected:
	// No ownership. El grupo vive en BaseGameLayer::_groups (unordered_map),
	// que no reubica elementos existentes al insertar nuevas keys, asi que
	// este puntero es estable mientras el nivel este cargado.
	GroupProperties* _group = nullptr;
	ax::Vec2 _startOffset;
	ax::Vec2 _endOffset;
};
