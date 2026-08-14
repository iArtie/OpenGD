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

#include "SimpleProgressBar.h"

#include "GameToolbox/getTextureString.h"

SimpleProgressBar* SimpleProgressBar::create(const char* _slider, const char* _sliderBar) {
	auto pRet = new (std::nothrow) SimpleProgressBar();
	if (pRet && pRet->init(_slider, _sliderBar)) {
		pRet->autorelease();
		return pRet;
	}
	AX_SAFE_DELETE(pRet);
	return nullptr; // Retorno explícito y limpio
}

bool SimpleProgressBar::init(const char* _slider, const char* _sliderBar)
{
	_pGroove = ax::Sprite::create(GameToolbox::getTextureString(_slider));
	_pBar = ax::Sprite::create(GameToolbox::getTextureString(_sliderBar));

	// Configuración de texturas
	_pGroove->setStretchEnabled(false);
	_pBar->setStretchEnabled(false);
	_pBar->getTexture()->setTexParameters({
		ax::backend::SamplerFilter::NEAREST,
		ax::backend::SamplerFilter::NEAREST,
		ax::backend::SamplerAddressMode::REPEAT,
		ax::backend::SamplerAddressMode::REPEAT
		});

	// Variables lógicas para los márgenes
	std::string sliderName = _slider;
	float offsetX = 0.0f;
	float offsetY = 0.0f;
	float widthReduction = 0.0f;

	// Verificamos cuál sprite se está usando
	if (sliderName.find("slidergroove_02") != std::string::npos)
	{
		// Dimensiones para la barra delgada (la que sale al jugar un nivel)
		offsetX = 2.0f;
		offsetY = 1.25f;
		widthReduction = 4.0f;
	}
	else
	{
		// Dimensiones para la barra gruesa (slidergroove normal, ej. opciones/música)
		// Al ser un sprite más grueso, sus bordes lógicos ocupan más espacio.
		offsetX = 2.7f;       // Ajusta este valor si la barra interior se sale por la izquierda
		offsetY = 3.0f;       // Ajusta este valor si la barra interior se sale por abajo
		widthReduction = 7.0f; // Ajusta este valor si la barra interior se sale por la derecha (es el doble de offsetX)
	}

	// Aplicamos los cálculos dinámicos
	_pBarWidth = _pGroove->getContentSize().width - widthReduction;

	_pBar->setAnchorPoint({ 0, 0 });
	_pBar->setPosition({ offsetX, offsetY });

	// Añadimos todo a la escena
	_pGroove->addChild(_pBar, -1);
	addChild(_pGroove);

	scheduleUpdate();
	return true;
}

void SimpleProgressBar::update(float delta)
{
	if(_pPercentage > 100.f)
	{
		_pPercentage = 100.f;
	}
	if(_pPercentage < 0.f)
	{
		_pPercentage = 0.f;
	}
	_pBar->setTextureRect({0, 0, _pBarWidth * (this->_pPercentage / 100), _pBar->getContentSize().height});
	return;
}