#include "FMODAudioEngine.h"
#include "fmod.hpp"
#include "fmod_errors.h"
#include "GameToolbox/log.h"
#include "platform/FileUtils.h"

static FMODAudioEngine* s_sharedEngine = nullptr;

static std::string getFMODPath(const std::string& path) {
	std::string fullPath = ax::FileUtils::getInstance()->fullPathForFilename(path);
#if (AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID)
	if (fullPath.find("assets/") == 0) {
		fullPath = "file:///android_asset/" + fullPath.substr(7);
	}
#endif
	return fullPath;
}

FMODAudioEngine* FMODAudioEngine::getInstance() {
	if (!s_sharedEngine) {
		s_sharedEngine = new FMODAudioEngine();
		s_sharedEngine->setup();
	}
	return s_sharedEngine;
}

void FMODAudioEngine::setup() {
	FMOD_RESULT result = FMOD::System_Create(&_system);
	if (result != FMOD_OK) {
		GameToolbox::log("FMOD error! ({}) {}", static_cast<int>(result), FMOD_ErrorString(result));
		return;
	}

	result = _system->init(512, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK) {
		GameToolbox::log("FMOD error! ({}) {}", static_cast<int>(result), FMOD_ErrorString(result));
		return;
	}

	_system->getMasterChannelGroup(&_masterGroup);

	// ¡AQUÍ ESTABA EL ERROR! Hay que CREAR los grupos antes de añadirlos.
	_system->createChannelGroup("Music", &_musicGroup);
	_system->createChannelGroup("SFX", &_sfxGroup);

	_masterGroup->addGroup(_musicGroup);
	_masterGroup->addGroup(_sfxGroup);
}

void FMODAudioEngine::update() {
	if (_system) {
		_system->update();
	}
}

void FMODAudioEngine::playMusic(const std::string& path, float offset, float volume, bool loop) {
	if (!_system) return;

	if (_currentMusicChannel) {
		_currentMusicChannel->stop();
		_currentMusicChannel = nullptr;
	}

	std::string realPath = getFMODPath(path); // Traducimos la ruta

	FMOD::Sound* sound = nullptr;
	if (_loadedSounds.find(realPath) == _loadedSounds.end()) {
		_system->createStream(realPath.c_str(), FMOD_DEFAULT | (loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF), nullptr, &sound);
		_loadedSounds[realPath] = sound;
	}
	else {
		sound = _loadedSounds[realPath];
	}

	if (sound) {
		_system->playSound(sound, _musicGroup, false, &_currentMusicChannel);
		_currentMusicChannel->setVolume(volume * _musicVolume);

		if (offset > 0.f) {
			unsigned int ms = static_cast<unsigned int>(offset * 1000.f);
			_currentMusicChannel->setPosition(ms, FMOD_TIMEUNIT_MS);
		}
	}
}

void FMODAudioEngine::playEffect(const std::string& path, float volume, float pitch, float pan) {
	if (!_system) return;

	std::string realPath = getFMODPath(path); // Traducimos la ruta

	FMOD::Sound* sound = nullptr;
	if (_loadedSounds.find(realPath) == _loadedSounds.end()) {
		_system->createSound(realPath.c_str(), FMOD_DEFAULT, nullptr, &sound);
		_loadedSounds[realPath] = sound;
	}
	else {
		sound = _loadedSounds[realPath];
	}

	if (sound) {
		FMOD::Channel* channel = nullptr;
		_system->playSound(sound, _sfxGroup, false, &channel);
		channel->setVolume(volume * _sfxVolume);
		channel->setPitch(pitch);
		channel->setPan(pan);
	}
}

void FMODAudioEngine::preloadMusic(const std::string& path) {
	if (!_system) return;
	std::string realPath = getFMODPath(path);
	if (_loadedSounds.find(realPath) == _loadedSounds.end()) {
		FMOD::Sound* sound = nullptr;
		_system->createStream(realPath.c_str(), FMOD_DEFAULT | FMOD_LOOP_NORMAL, nullptr, &sound);
		_loadedSounds[realPath] = sound;
	}
}

void FMODAudioEngine::preloadEffect(const std::string& path) {
	if (!_system) return;
	std::string realPath = getFMODPath(path);
	if (_loadedSounds.find(realPath) == _loadedSounds.end()) {
		FMOD::Sound* sound = nullptr;
		_system->createSound(realPath.c_str(), FMOD_DEFAULT, nullptr, &sound);
		_loadedSounds[realPath] = sound;
	}
}

void FMODAudioEngine::stopAllMusic() {
	if (_musicGroup) _musicGroup->stop();
	_currentMusicChannel = nullptr;
}

void FMODAudioEngine::stopAllEffects() {
	if (_sfxGroup) _sfxGroup->stop();
}

void FMODAudioEngine::setPaused(bool pause) {
	_isPaused = pause;
	if (_masterGroup) {
		_masterGroup->setPaused(pause);
	}
}

void FMODAudioEngine::setMusicVolume(float volume) {
	_musicVolume = volume;
	if (_musicGroup) _musicGroup->setVolume(volume);
}

void FMODAudioEngine::setSFXVolume(float volume) {
	_sfxVolume = volume;
	if (_sfxGroup) _sfxGroup->setVolume(volume);
}

float FMODAudioEngine::getMusicTimeMS() {
	if (_currentMusicChannel) {
		unsigned int pos = 0;
		_currentMusicChannel->getPosition(&pos, FMOD_TIMEUNIT_MS);
		return static_cast<float>(pos);
	}
	return 0.f;
}

void FMODAudioEngine::setMusicTimeMS(float ms) {
	if (_currentMusicChannel) {
		_currentMusicChannel->setPosition(static_cast<unsigned int>(ms), FMOD_TIMEUNIT_MS);
	}
}