#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace FMOD {
	class System;
	class Channel;
	class ChannelGroup;
	class Sound;
	class DSP;
}

class FMODAudioEngine {
public:
	static FMODAudioEngine* getInstance();

	void setup();
	void update();

	void playMusic(const std::string& path, float offset = 0.0f, float volume = 1.0f, bool loop = true);
	void playEffect(const std::string& path, float volume = 1.0f, float pitch = 1.0f, float pan = 0.0f);

	void stopAllMusic();
	void stopAllEffects();
	void preloadMusic(const std::string& path);
	void preloadEffect(const std::string& path);

	void setPaused(bool pause);
	void setMusicVolume(float volume);
	void setSFXVolume(float volume);

	float getMusicTimeMS();
	void setMusicTimeMS(float ms);

	FMOD::System* getSystem() { return _system; }

private:
	FMODAudioEngine() = default;
	~FMODAudioEngine() = default;

	FMOD::System* _system = nullptr;
	FMOD::ChannelGroup* _masterGroup = nullptr;
	FMOD::ChannelGroup* _musicGroup = nullptr;
	FMOD::ChannelGroup* _sfxGroup = nullptr;

	FMOD::Channel* _currentMusicChannel = nullptr;

	std::unordered_map<std::string, FMOD::Sound*> _loadedSounds;

	float _musicVolume = 1.0f;
	float _sfxVolume = 1.0f;
	bool _isPaused = false;
};