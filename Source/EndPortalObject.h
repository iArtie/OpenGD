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
#include "GameObject.h"
#include <vector>

namespace ax
{
    class Sprite;
}

class EndPortalObject : public GameObject
{
private:
    ax::Sprite* _gradientBar;
    std::vector<ax::Sprite*> _squares;

public:
    static EndPortalObject* create();
    bool init();

    virtual void setPosition(const ax::Vec2& pos) override;
    virtual void setVisible(bool visible) override;

    void updateEndPos(bool updateParticle);
    ax::Vec2 getSpawnPos();
};