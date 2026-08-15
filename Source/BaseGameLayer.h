#pragma once
#include "axmol.h"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <map>
#include <span>
#include "SpriteColor.h"
#include "GameObject.h"
#include "PlayerObject.h"
#include "EffectManager.h"

struct LevelSettings {
    int bgID = 1;
    int _bgID = 1;
    int groundID = 1;
    int _groundID = 1;
    int groundLineID = 1;
    int fontID = 1;
    float songOffset = 0;
    bool twoPlayerMode = false;
    bool isPlatformer = false;
    int gamemode = 0;
    bool mini = false;
    int speed = 0;
    bool dual = false;
    bool twoPlayer = false;
    bool flipGravity = false;
};

struct GroupProperties {
    enum GroupState { NOT_CHANGING, MAIN_ONLY, DETAIL_ONLY, MAIN_DETAIL };
    ax::Color3B _color;
    float _alpha = 1.0f;
    GroupState groupState = NOT_CHANGING;
    std::vector<GameObject*> _objects;
    ax::Vec2 _offset = ax::Vec2(0.f, 0.f);
};

class GJGameLevel;

class BaseGameLayer : public ax::Layer {
protected:
    GJGameLevel* _level;

public:
    static BaseGameLayer* _instance;
    PlayerObject* _player1;
    PlayerObject* _player2;
    EffectManager* _effectManager;
    std::vector<GameObject*> _pObjects;
    std::vector<GameObject*> _allObjects;
    std::vector<std::vector<GameObject*>> _sectionObjects;
    ax::DrawNode* dn;
    ax::SpriteBatchNode* _batchNode;
    ax::SpriteBatchNode* _glowBatchNode;
    std::string _mainBatchNodeTexture = "GJ_GameSheet.png";
    std::string _main2BatchNodeTexture = "GJ_GameSheet02.png";
    ax::SpriteBatchNode* _mainBatchNodeB4 = nullptr;
    ax::SpriteBatchNode* _mainBatchNodeB3 = nullptr;
    ax::SpriteBatchNode* _mainBatchNodeB2 = nullptr;
    ax::SpriteBatchNode* _mainBatchNodeB1 = nullptr;
    ax::SpriteBatchNode* _mainBatchNodeT1 = nullptr;
    ax::SpriteBatchNode* _mainBatchNodeT2 = nullptr;
    ax::SpriteBatchNode* _mainBatchNodeT3 = nullptr;
    ax::SpriteBatchNode* _main2BatchNode = nullptr;
    ax::SpriteBatchNode* _blendingBatchNodeB4 = nullptr;
    ax::SpriteBatchNode* _blendingBatchNodeB3 = nullptr;
    ax::SpriteBatchNode* _blendingBatchNodeB2 = nullptr;
    ax::SpriteBatchNode* _blendingBatchNodeB1 = nullptr;
    ax::SpriteBatchNode* _blendingBatchNodeT1 = nullptr;
    ax::SpriteBatchNode* _blendingBatchNodeT2 = nullptr;
    ax::SpriteBatchNode* _blendingBatchNodeT3 = nullptr;
    LevelSettings _levelSettings;
    std::unordered_map<int, SpriteColor> _colorChannels;
    std::unordered_map<int, SpriteColor> _originalColors;
    std::unordered_map<int, GroupProperties> _groups;
    std::unordered_map<int, int> _itemValues;
    int _prevSection = -1;
    int _nextSection = -1;
    bool _isPracticeMode = false;
    bool _isTestMode = false;
    bool _testMode = false;
    bool _isDualMode = false;
    float m_sectionXFactor = 1.0f;
    float m_sectionYFactor = 1.0f;
    int m_solidCollisionObjectsCount = 0;
    int m_hazardCollisionObjectsCount = 0;
    float m_maxGameplayY = 2700.f;
    bool m_isBetweenSteps = false;

    virtual bool init(GJGameLevel* level);
    virtual void update(float dt) override {}
    virtual void resetLevel() {}
    void setLevel(GJGameLevel* level) { _level = level; }
    GJGameLevel* getLevel() { return _level; }
    void initBatchNodes();
    bool isObjectBlending(GameObject* obj);
    bool checkCollision(int a2, int a3);
    virtual int checkCollisions(PlayerObject* player, float dt, bool a4);
    void fillColorChannel(std::span<std::string_view> colorString, int id);
    SpriteColor& getColorChannel(int channelID);
    ax::Color3B getChannelColorWithCopy(int channelID);
    void loadLevel();
    void setupLevel(std::string_view uncompressedLevelString);
    void createObjectsFromSetup(std::string_view uncompressedLevelString);
    void loadLevelData(std::string_view data);
    virtual void destroyPlayer(PlayerObject* player, GameObject* hazard) {}
    virtual void destroyPlayer(PlayerObject* player) {}
    virtual void addObject(GameObject* obj) {}
    virtual ax::Color3B getLightBG() { return ax::Color3B::WHITE; }
    virtual int sectionForPos(float x);
    void activateGroup(int groupID, bool activate);
    int getItemValue(int itemID);
    void modifyItemValue(int itemID, int amount);
    template<typename... Args>
    ax::Color3B getLightBG(Args... args) { return ax::Color3B::WHITE; }
    template<typename... Args>
    void processMoveActionsStep(Args... args) {}
    static BaseGameLayer* getInstance();
};

inline bool operator==(const std::string& lhs, int rhs) { return rhs == 0 && lhs.empty(); }
inline bool operator!=(const std::string& lhs, int rhs) { return rhs != 0 || !lhs.empty(); }
inline bool operator==(int lhs, const std::string& rhs) { return lhs == 0 && rhs.empty(); }
inline bool operator!=(int lhs, const std::string& rhs) { return lhs != 0 || !rhs.empty(); }
inline bool operator==(const std::string& lhs, std::nullptr_t) { return lhs.empty(); }
inline bool operator!=(const std::string& lhs, std::nullptr_t) { return !lhs.empty(); }