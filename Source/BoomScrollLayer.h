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

#include <string>
#include <vector>

#include "2d/Layer.h"
#include "2d/ActionTween.h"

// Eliminamos ActionTweenDelegate, usaremos Actions estándar de Axmol
class BoomScrollLayer : public ax::Layer {
private:
	ax::Layer* _internalLayer;

	// --- VARIABLES DE TOQUE (ESTILO ROBTOP) ---
	bool _touching = false;
	ax::Vec2 _touchBeganPoint;
	ax::Vec2 _touchLastPoint;
	float _touchStartTime = 0.0f;
	float _velocity = 0.0f;

	// Configuraciones de sensibilidad
	float _minimumTouchLengthToChangePage = 40.0f;
	float _swipeThresholdVelocity = 500.0f; // Píxeles por segundo

public:

	int _currentPage, _leftPage, _rightPage;
	int _totalPages;
	std::vector<ax::Layer*> _layers;

	bool init(std::vector<ax::Layer*> layers, int currentPage);
	static BoomScrollLayer* create(std::vector<ax::Layer*> layers, int currentPage);

	void selectPage(int current);
	void changePageRight();
	void changePageLeft();

	// Animación base de salto de página
	void animateToPage(int direction);

	// Restauramos el sistema táctil usando EventListeners modernos
	bool onTouchBegan(ax::Touch* touch, ax::Event* event);
	void onTouchEnded(ax::Touch* touch, ax::Event* event);
	void onTouchMoved(ax::Touch* touch, ax::Event* event);
	void onTouchCancelled(ax::Touch* touch, ax::Event* event);

	void onExit() override;
};