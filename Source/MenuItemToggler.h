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
#include <functional>

#include "2d/MenuItem.h"
#include "2d/Node.h"
#include "math/Vec2.h"

class MenuItemSpriteExtra;

namespace ax 
{ 
	class Node; 
}

class MenuItemToggler : public ax::MenuItem {
public:
    MenuItemToggler(ax::Node* offNode, ax::Node* onNode, std::function<void(ax::Node*)> callback);
    bool init() override;
    void selected() override;
	void unselected() override;
	void activate() override;

    void toggle(bool on);
    void setEnabled(bool value) override;
    void setSizeMult(float size);

    static MenuItemToggler* create(ax::Node* offNode, ax::Node* onNode, std::function<void(ax::Node*)> callback);
protected:
    MenuItemSpriteExtra* _offButton;
    MenuItemSpriteExtra* _onButton;
    bool _toggled;
    bool _notClickable;
    std::function<void(ax::Node*)> m_fCallback;
    MenuItemSpriteExtra* _activeItem;
};