#pragma once
#include "2d/Layer.h"

class PlayLayer;
namespace ax { class Node; }

class PauseLayer : public ax::LayerColor {
public:
	PlayLayer* _playLayer;

	static PauseLayer* create(PlayLayer* playLayer);
	bool init(PlayLayer* playLayer);

	void customSetup();

	void onResume(ax::Node* sender);
	void onQuit(ax::Node* sender);
	void onRestart(ax::Node* sender);
	void onPracticeMode(ax::Node* sender);

	void onEnter() override;
	void onExit() override;
};