#include "BaseGameLayer.h"
#include "PlayerObject.h"
#include "GameObject.h"
#include "EffectGameObject.h"
#include "GJGameLevel.h"
#include "base/Director.h"
#include "GameToolbox/log.h"
#include "GameToolbox/conv.h"
#include "GameToolbox/getTextureString.h"
#include "external/benchmark.h"
#include "external/json.hpp"
#include "platform/FileUtils.h"
#include "math/Rect.h"
#include <2d/SpriteBatchNode.h>
#include <fmt/format.h>
#include <cstdio>
#include <fstream>

USING_NS_AX;

template <typename... Args>
static void DBG_LOG(fmt::format_string<Args...> fmtStr, Args&&... args)
{
    std::string msg = fmt::format(fmtStr, std::forward<Args>(args)...);
    fputs(msg.c_str(), stderr);
    fputc('\n', stderr);
    fflush(stderr);
}

BaseGameLayer* BaseGameLayer::_instance = nullptr;

static std::string shortTextForColorIdx(int channel) {
    switch (channel) {
    case 0: return "D";
    case 1000: return "BG";
    case 1001: return "G1";
    case 1002: return "L";
    case 1003: return "3DL";
    case 1004: return "Obj";
    case 1005: return "P1";
    case 1006: return "P2";
    case 1007: return "LBG";
    case 1008: return "C";
    case 1009: return "G2";
    case 1010: return "Black";
    case 1011: return "White";
    case 1012: return "Ltr";
    case 1013: return "MG";
    case 1014: return "MG2";
    default: return std::to_string(channel);
    }
}

static std::string longTextForColorIdx(int channel) {
    if (0 < channel && channel < 1000) {
        return std::to_string(channel);
    }
    else {
        return shortTextForColorIdx(channel);
    }
}

static int CURRENT_PAGE = 0;
static std::array<int, 15> RECENT_COLOR_IDS{};

static constexpr std::array SPECIAL_CHANNEL_ORDER_SMALL{
    0, 1005, 1006, 1000, 1001, 1013, 1007, 1009, 1014, 1004, 1002, 1003, 1012, 1010, 1011,
};

static constexpr std::array SPECIAL_CHANNEL_ORDER_LARGE{
    0, 1005, 1006, 1012, 1000, 1001, 1013, 1010, 1007, 1009, 1014, 1011, 1004, 1002, 1003,
};

BaseGameLayer* BaseGameLayer::getInstance()
{
    return _instance;
}

bool BaseGameLayer::init(GJGameLevel* level)
{
    DBG_LOG("BaseGameLayer::init - ENTRANDO");
    if (!Layer::init())
        return false;
    _instance = this;
    _level = level;
    _effectManager = EffectManager::create();
    this->addChild(_effectManager);
    initBatchNodes();
    DBG_LOG("BaseGameLayer::init - batch nodes listos, llamando loadLevel()");
    loadLevel();
    DBG_LOG("BaseGameLayer::init - loadLevel() termino");
    return true;
}

void BaseGameLayer::loadLevel()
{
    if (!_level)
    {
        DBG_LOG("BaseGameLayer::loadLevel - _level es nullptr, no se puede cargar nada");
        return;
    }
    std::string levelStr = _level->_levelString;
    DBG_LOG("BaseGameLayer::loadLevel - _levelString.size() = {}, _levelID = {}", levelStr.size(), _level->_levelID);
    if (levelStr.empty())
    {
        DBG_LOG("BaseGameLayer::loadLevel - _levelString vacio, buscando archivo de nivel individual");
        std::string levelFile = fmt::format("levels/{}.txt", _level->_levelID);
        if (!ax::FileUtils::getInstance()->isFileExist(levelFile))
        {
            DBG_LOG("BaseGameLayer::loadLevel - no existe el archivo {}", levelFile);
            return;
        }
        levelStr = ax::FileUtils::getInstance()->getStringFromFile(levelFile);
        if (levelStr.empty())
        {
            DBG_LOG("BaseGameLayer::loadLevel - el archivo {} esta vacio", levelFile);
            return;
        }
    }
    try
    {
        levelStr = GJGameLevel::decompressLvlStr(levelStr);
    }
    catch (const std::exception& e)
    {
        DBG_LOG("BaseGameLayer::loadLevel - excepcion al descomprimir el nivel: {}", e.what());
        return;
    }
    DBG_LOG("BaseGameLayer::loadLevel - nivel descomprimido, {} caracteres", levelStr.size());
    if (levelStr.empty())
    {
        DBG_LOG("BaseGameLayer::loadLevel - el string descomprimido quedo vacio. Abortando carga.");
        return;
    }
    {
        auto s = BenchmarkTimer("load level");
        try
        {
            setupLevel(levelStr);
            createObjectsFromSetup(levelStr);
        }
        catch (const std::exception& e)
        {
            DBG_LOG("BaseGameLayer::loadLevel - EXCEPCION durante setupLevel/createObjectsFromSetup: {}", e.what());
            return;
        }
    }
    DBG_LOG("BaseGameLayer::loadLevel - _allObjects.size() = {}", _allObjects.size());
    if (_allObjects.size() != 0)
    {
        float lastObjXPos = 570.0f;
        for (GameObject* object : _allObjects)
        {
            if (lastObjXPos < object->getPositionX())
                lastObjXPos = object->getPositionX();
        }
        DBG_LOG("last x: {}", lastObjXPos);
        int sectionCount = sectionForPos(lastObjXPos);
        for (int i = 0; i < sectionCount; i++)
        {
            std::vector<GameObject*> vec;
            _sectionObjects.push_back(vec);
        }
        for (GameObject* object : _allObjects)
        {
            int section = sectionForPos(object->getPositionX());
            int idx = section - 1 < 0 ? 0 : section - 1;
            while (idx >= static_cast<int>(_sectionObjects.size()))
                _sectionObjects.push_back(std::vector<GameObject*>());
            object->_section = idx;
            _sectionObjects[idx].push_back(object);
            object->setCascadeOpacityEnabled(false);
            object->update();
        }
    }
}

void BaseGameLayer::initBatchNodes()
{
    _blendingBatchNodeB4 = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 50);
    this->addChild(_blendingBatchNodeB4, -23);
    _blendingBatchNodeB4->setBlendFunc(GameToolbox::getBlending());
    _blendingBatchNodeB4->setName("_blendingBatchNodeB4");
    _mainBatchNodeB4 = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 50);
    this->addChild(_mainBatchNodeB4, -22);
    _mainBatchNodeB4->setName("_mainBatchNodeB4");
    _blendingBatchNodeB3 = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 50);
    this->addChild(_blendingBatchNodeB3, -16);
    _blendingBatchNodeB3->setBlendFunc(GameToolbox::getBlending());
    _blendingBatchNodeB3->setName("_blendingBatchNodeB3");
    _mainBatchNodeB3 = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 50);
    this->addChild(_mainBatchNodeB3, -15);
    _mainBatchNodeB3->setName("_mainBatchNodeB3");
    _blendingBatchNodeB2 = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 50);
    this->addChild(_blendingBatchNodeB2, -9);
    _blendingBatchNodeB2->setBlendFunc(GameToolbox::getBlending());
    _blendingBatchNodeB2->setName("_blendingBatchNodeB2");
    _mainBatchNodeB2 = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 50);
    this->addChild(_mainBatchNodeB2, -8);
    _mainBatchNodeB2->setName("_mainBatchNodeB2");
    _blendingBatchNodeB1 = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 50);
    this->addChild(_blendingBatchNodeB1, -2);
    _blendingBatchNodeB1->setBlendFunc(GameToolbox::getBlending());
    _blendingBatchNodeB1->setName("_blendingBatchNodeB1");
    _mainBatchNodeB1 = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 50);
    this->addChild(_mainBatchNodeB1, -1);
    _mainBatchNodeB1->setName("_mainBatchNodeB1");
    _blendingBatchNodeT1 = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 50);
    this->addChild(_blendingBatchNodeT1, 2);
    _blendingBatchNodeT1->setBlendFunc(GameToolbox::getBlending());
    _blendingBatchNodeT1->setName("_blendingBatchNodeT1");
    _mainBatchNodeT1 = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 50);
    this->addChild(_mainBatchNodeT1, 3);
    _mainBatchNodeT1->setName("_mainBatchNodeT1");
    _blendingBatchNodeT2 = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 50);
    this->addChild(_blendingBatchNodeT2, 9);
    _blendingBatchNodeT2->setBlendFunc(GameToolbox::getBlending());
    _blendingBatchNodeT2->setName("_blendingBatchNodeT2");
    _mainBatchNodeT2 = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 50);
    this->addChild(_mainBatchNodeT2, 10);
    _mainBatchNodeT2->setName("_mainBatchNodeT2");
    _blendingBatchNodeT3 = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 50);
    this->addChild(_blendingBatchNodeT3, 24);
    _blendingBatchNodeT3->setBlendFunc(GameToolbox::getBlending());
    _blendingBatchNodeT3->setName("_blendingBatchNodeT3");
    _mainBatchNodeT3 = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 50);
    this->addChild(_mainBatchNodeT3, 25);
    _mainBatchNodeT3->setName("_mainBatchNodeT3");
    _glowBatchNode = ax::SpriteBatchNode::create(GameToolbox::getTextureString("GJ_GameSheetGlow.png"), 150);
    this->addChild(_glowBatchNode);
    _glowBatchNode->setBlendFunc(GameToolbox::getBlending());
    _glowBatchNode->setName("_glowBatchNode");
    _main2BatchNode = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_main2BatchNodeTexture), 150);
    this->addChild(_main2BatchNode);
    _main2BatchNode->setName("_main2BatchNode");
    _mainBatchNodeTexture = _mainBatchNodeT3->getTexture()->getPath();
    _main2BatchNodeTexture = _main2BatchNode->getTexture()->getPath();
}

bool BaseGameLayer::isObjectBlending(GameObject* obj)
{
    return _colorChannels.contains(obj->_mainColorChannel) && _colorChannels[obj->_mainColorChannel]._blending &&
        _colorChannels.contains(obj->_secColorChannel) && _colorChannels[obj->_secColorChannel]._blending ||
        !_colorChannels.contains(obj->_mainColorChannel) && _colorChannels.contains(obj->_secColorChannel) &&
        _colorChannels[obj->_secColorChannel]._blending ||
        !_colorChannels.contains(obj->_secColorChannel) && _colorChannels.contains(obj->_mainColorChannel) &&
        _colorChannels[obj->_mainColorChannel]._blending;
}

bool BaseGameLayer::checkCollision(int a2, int a3)
{
    return false;
}

int BaseGameLayer::checkCollisions(PlayerObject* player, float dt, bool a4)
{
    if (!player) return 0;
    bool m_isOnGround2 = player->isOnGround();
    player->setIsOnGround(false);
    float v11 = 0.0f;
    float v12 = player->_vehicleSize;
    float m_unkAngle1 = 30.0f;
    if (v12 != 1.0f) v11 = (1.0f - v12) * m_unkAngle1 * 0.5f;
    bool isOutOfBounds = false;
    if (_levelSettings.isPlatformer) {
        if (player->getPositionX() < -30.0f) {
            player->m_dXVel = 0.0f;
            player->setPositionX(-30.0f);
        }
    }
    if (player->getPositionY() > v11 + m_maxGameplayY) {
        isOutOfBounds = true;
    }
    if (!isOutOfBounds) {
        for (auto obj : _pObjects) {
            if (obj && obj->isActive()) {
                if (obj->_isTrigger) {
                    ax::Rect pRect = player->getOuterBounds();
                    ax::Rect oRect = obj->getOuterBounds();
                    if (pRect.intersectsRect(oRect)) {
                        if (!obj->hasBeenActivatedByPlayer(player)) {
                            obj->triggerActivated(player);
                        }
                    }
                }
                else {
                    player->collidedWithObject(dt, obj);
                }
            }
        }
    }
    else {
        if (a4) return 1;
        player->setIsDead(true);
    }
    return 1;
}

void BaseGameLayer::createObjectsFromSetup(std::string_view uncompressedLevelString)
{
    std::vector<std::string_view> objData = GameToolbox::splitByDelimStringView(uncompressedLevelString, ';');
    _allObjects.reserve(objData.size());
    objData.erase(objData.begin());
    if (!objData.empty())
    {
        const auto& last = objData.back();
        if (last.empty() || last.front() != '1' || last.size() < 2 || last[1] != ',')
        {
            objData.pop_back();
        }
    }
    DBG_LOG("creating & pushing");
    for (const auto& objectDataSpecific : objData)
    {
        GameObject* obj = GameObject::createFromString(objectDataSpecific);
        if (obj)
        {
            obj->_uniqueID = static_cast<int>(_allObjects.size());
            _allObjects.push_back(obj);
        }
    }
}

void BaseGameLayer::setupLevel(std::string_view uncompressedLevelString)
{
    std::vector<std::string_view> levelData =
        GameToolbox::splitByDelimStringView(GameToolbox::splitByDelimStringView(uncompressedLevelString, ';')[0], ',');

    for (size_t i = 0; i + 1 < levelData.size(); i += 2)
    {
        if (levelData[i] == "kS1")
        {
            _colorChannels.insert({ 1000, SpriteColor(ax::Color3B(GameToolbox::stof(levelData[i + 1]), 0, 0), 255, 0) });
        }
        else if (levelData[i] == "kS2")
        {
            _colorChannels.at(1000)._color.g = GameToolbox::stof(levelData[i + 1]);
        }
        else if (levelData[i] == "kS3")
        {
            _colorChannels.at(1000)._color.b = GameToolbox::stof(levelData[i + 1]);
        }
        else if (levelData[i] == "kS4")
        {
            _colorChannels.insert({ 1001, SpriteColor(ax::Color3B(GameToolbox::stof(levelData[i + 1]), 0, 0), 255, 0) });
        }
        else if (levelData[i] == "kS5")
        {
            _colorChannels.at(1001)._color.g = GameToolbox::stof(levelData[i + 1]);
        }
        else if (levelData[i] == "kS6")
        {
            _colorChannels.at(1001)._color.b = GameToolbox::stof(levelData[i + 1]);
        }
        else if (levelData[i] == "kS29")
        {
            auto colorString = GameToolbox::splitByDelimStringView(levelData[i + 1], '_');
            fillColorChannel(colorString, 1000);
        }
        else if (levelData[i] == "kS30")
        {
            auto colorString = GameToolbox::splitByDelimStringView(levelData[i + 1], '_');
            fillColorChannel(colorString, 1001);
        }
        else if (levelData[i] == "kS31")
        {
            auto colorString = GameToolbox::splitByDelimStringView(levelData[i + 1], '_');
            fillColorChannel(colorString, 1002);
        }
        else if (levelData[i] == "kS32")
        {
            auto colorString = GameToolbox::splitByDelimStringView(levelData[i + 1], '_');
            fillColorChannel(colorString, 1004);
        }
        else if (levelData[i] == "kS37")
        {
            auto colorString = GameToolbox::splitByDelimStringView(levelData[i + 1], '_');
            fillColorChannel(colorString, 1003);
        }
        else if (levelData[i] == "kS38")
        {
            auto colorString = GameToolbox::splitByDelimStringView(levelData[i + 1], '|');
            for (std::string_view colorData : colorString)
            {
                auto innerData = GameToolbox::splitByDelimStringView(colorData, '_');
                int key = 0;
                SpriteColor col;
                col._blending = false;
                for (size_t j = 0; j + 1 < innerData.size(); j += 2)
                {
                    switch (GameToolbox::stoi(innerData[j]))
                    {
                    case 1:
                        col._color.r = GameToolbox::stof(innerData[j + 1]);
                        break;
                    case 2:
                        col._color.g = GameToolbox::stof(innerData[j + 1]);
                        break;
                    case 3:
                        col._color.b = GameToolbox::stof(innerData[j + 1]);
                        break;
                    case 5:
                        col._blending = GameToolbox::stoi(innerData[j + 1]);
                        break;
                    case 6:
                        key = GameToolbox::stoi(innerData[j + 1]);
                        break;
                    case 7:
                        col._opacity = GameToolbox::stof(innerData[j + 1]) * 255.f;
                        break;
                    case 9:
                        col._copyingColorID = GameToolbox::stoi(innerData[j + 1]);
                        break;
                    case 10:
                    {
                        auto hsv = GameToolbox::splitByDelimStringView(innerData[j + 1], 'a');
                        if (hsv.size() >= 5)
                        {
                            col._hsvModifier.h = GameToolbox::stof(hsv[0]);
                            col._hsvModifier.s = GameToolbox::stof(hsv[1]);
                            col._hsvModifier.v = GameToolbox::stof(hsv[2]);
                            col._hsvModifier.sChecked = GameToolbox::stoi(hsv[3]);
                            col._hsvModifier.vChecked = GameToolbox::stoi(hsv[4]);
                        }
                        break;
                    }
                    }
                }
                _colorChannels.insert({ key, col });
            }
        }
        else if (levelData[i] == "kA6")
        {
            _levelSettings._bgID = GameToolbox::stoi(levelData[i + 1]);
            _levelSettings.bgID = _levelSettings._bgID;
            if (!_levelSettings._bgID)
            {
                _levelSettings._bgID = 1;
                _levelSettings.bgID = 1;
            }
        }
        else if (levelData[i] == "kA7")
        {
            _levelSettings._groundID = GameToolbox::stoi(levelData[i + 1]);
            _levelSettings.groundID = _levelSettings._groundID;
            if (!_levelSettings._groundID)
            {
                _levelSettings._groundID = 1;
                _levelSettings.groundID = 1;
            }
        }
        else if (levelData[i] == "kA2")
        {
            _levelSettings.gamemode = GameToolbox::stoi(levelData[i + 1]);
        }
        else if (levelData[i] == "kA3")
        {
            _levelSettings.mini = GameToolbox::stoi(levelData[i + 1]);
        }
        else if (levelData[i] == "kA4")
        {
            _levelSettings.speed = GameToolbox::stoi(levelData[i + 1]);
        }
        else if (levelData[i] == "kA8")
        {
            _levelSettings.dual = GameToolbox::stoi(levelData[i + 1]);
        }
        else if (levelData[i] == "kA10")
        {
            _levelSettings.twoPlayer = GameToolbox::stoi(levelData[i + 1]);
        }
        else if (levelData[i] == "kA11")
        {
            _levelSettings.flipGravity = GameToolbox::stoi(levelData[i + 1]);
        }
        else if (levelData[i] == "kA13")
        {
            _levelSettings.songOffset = GameToolbox::stof(levelData[i + 1]);
        }
        else if (levelData[i] == "kA12")
        {
            _level->_musicID = GameToolbox::stoi(levelData[i + 1]) + 1;
        }
    }

    _colorChannels[1005]._color = Color3B::WHITE;
    _colorChannels[1005]._blending = true;
    _colorChannels[1006]._color = Color3B::WHITE;
    _colorChannels[1006]._blending = true;
    _colorChannels[1010]._color = Color3B::BLACK;
    _colorChannels[1010]._blending = false;
    _colorChannels[1011]._color = Color3B::WHITE;
    _colorChannels[1011]._blending = false;
    _colorChannels[1012]._color = Color3B(0.5f, 0.5f, 0.5f);
    _colorChannels[1012]._blending = false;
    _colorChannels[1013]._color = Color3B::WHITE;
    _colorChannels[1013]._blending = true;
    _colorChannels[1014]._color = Color3B::WHITE;
    _colorChannels[1014]._blending = true;
    _colorChannels[1007]._color = getColorChannel(1000)._color;
    _colorChannels[1007]._blending = true;
    _colorChannels[1008]._color = Color3B::WHITE;
    _colorChannels[1008]._blending = false;
    _colorChannels[1009]._color = Color3B::WHITE;
    _colorChannels[1009]._blending = true;
    _colorChannels[1003]._color = Color3B::WHITE;
    _colorChannels[1003]._blending = false;

    _originalColors = _colorChannels;
}

void BaseGameLayer::fillColorChannel(std::span<std::string_view> colorString, int id)
{
    if (!_colorChannels.contains(id)) {
        _colorChannels[id] = SpriteColor(ax::Color3B::WHITE, 255, 0);
    }
    for (size_t j = 0; j + 1 < colorString.size(); j += 2)
    {
        switch (GameToolbox::stoi(colorString[j]))
        {
        case 1:
            _colorChannels[id]._color.r = GameToolbox::stof(colorString[j + 1]);
            break;
        case 2:
            _colorChannels[id]._color.g = GameToolbox::stof(colorString[j + 1]);
            break;
        case 3:
            _colorChannels[id]._color.b = GameToolbox::stof(colorString[j + 1]);
            break;
        }
    }
}

int BaseGameLayer::sectionForPos(float x)
{
    int section = static_cast<int>(x / 100);
    if (section < 0)
        section = 0;
    return section;
}

void BaseGameLayer::loadLevelData(std::string_view levelDataString)
{
}

SpriteColor& BaseGameLayer::getColorChannel(int channelID)
{
    auto it = _colorChannels.find(channelID);
    if (it != _colorChannels.end())
        return it->second;

    auto result = _colorChannels.insert({ channelID, SpriteColor(ax::Color3B::WHITE, 255, 0) });
    _originalColors.insert({ channelID, SpriteColor(ax::Color3B::WHITE, 255, 0) });
    return result.first->second;
}

Color3B BaseGameLayer::getChannelColorWithCopy(int channelID)
{
    auto& channel = getColorChannel(channelID);
    if (channel._copyingColorID != -1) {
        auto& source = getColorChannel(channel._copyingColorID);
        Color3B result = source._color;
        GameToolbox::applyHSV(channel._hsvModifier, &result);
        return result;
    }
    Color3B result = channel._color;
    GameToolbox::applyHSV(channel._hsvModifier, &result);
    return result;
}

void BaseGameLayer::activateGroup(int groupID, bool activate)
{
    if (!_groups.contains(groupID))
        return;
    for (auto* obj : _groups.at(groupID)._objects)
    {
        if (!obj)
            continue;
        obj->_toggledOn = activate;
        if (!activate)
            obj->removeFromGameLayer();
    }
}

int BaseGameLayer::getItemValue(int itemID)
{
    auto it = _itemValues.find(itemID);
    return it != _itemValues.end() ? it->second : 0;
}

void BaseGameLayer::modifyItemValue(int itemID, int amount)
{
    _itemValues[itemID] += amount;
}