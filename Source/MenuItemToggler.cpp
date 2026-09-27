#include "MenuItemToggler.h"
#include "2d/MenuItem.h"
#include "2d/Node.h"
#include "2d/Sprite.h"
#include "2d/ActionEase.h"
#include "MenuItemSpriteExtra.h"
#include "math/Vec2.h"
#include <cstddef>

USING_NS_AX;

MenuItemToggler::MenuItemToggler(ax::Node* offNode, ax::Node* onNode, std::function<void(ax::Node*)> callback) {
    _offButton = MenuItemSpriteExtra::create(offNode, [&](Node*) {
        if (!_notClickable) {
            this->toggle(true);
        }
    });
    _onButton = MenuItemSpriteExtra::create(onNode, [&](Node*) {
        if (!_notClickable) {
            this->toggle(false);
        }
    });

    m_fCallback = callback;
}

void MenuItemToggler::toggle(bool on) {
    _offButton->setVisible(!on);
    _onButton->setVisible(on);

    _activeItem = on ? _onButton : _offButton;

    setContentSize(_activeItem->getContentSize());

    _offButton->setPosition(getContentSize() / 2);
    _onButton->setPosition(getContentSize() / 2);
}

void MenuItemToggler::setEnabled(bool value) {
    MenuItem::setEnabled(value);
    _offButton->setEnabled(value);
    _onButton->setEnabled(value);
}

void MenuItemToggler::setSizeMult(float size) {
    _offButton->m_fScaleMult = size;
    _onButton->m_fScaleMult = size;
}

bool MenuItemToggler::init() {
    if (!MenuItem::initWithCallback(nullptr)) return false;

    addChild(_offButton);
    addChild(_onButton);

    _offButton->getNormalImage()->setAnchorPoint(Vec2(.5f, .5f));
    _onButton->getNormalImage()->setAnchorPoint(Vec2(.5f, .5f));

    _notClickable = false;
    toggle(false);
    return true;
}

void MenuItemToggler::selected() {
    MenuItem::selected();
    _activeItem->selected();
}

void MenuItemToggler::unselected() {
    MenuItem::unselected();
    _activeItem->unselected();
}

void MenuItemToggler::activate() {
    MenuItem::activate();

    _activeItem->activate();

    m_fCallback(this);
}

MenuItemToggler* MenuItemToggler::create(ax::Node* offNode, ax::Node* onNode, std::function<void(ax::Node*)> callback) {
    auto ret = new MenuItemToggler(offNode, onNode, callback);
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}
