/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 Copyright (c) 2021 Bytedance Inc.

 https://axmolengine.github.io/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "AppDelegate.h"
#include "platform/FileUtils.h"
#include "GameManager.h"
#include "ResourcesLoadingLayer.h"
#include "external/constants.h"
#include "GameToolbox/log.h"

#include "platform/GLView.h"
#include "base/Director.h"
#include "base/EventDispatcher.h"

#if defined(AX_PLATFORM_PC) || (AX_TARGET_PLATFORM == AX_PLATFORM_WASM)
#include "platform/GLViewImpl.h"
#elif (AX_TARGET_PLATFORM == AX_PLATFORM_ANDROID)
#include "platform/android/GLViewImpl-android.h"
#elif (AX_TARGET_PLATFORM == AX_PLATFORM_IOS)
#include "platform/ios/GLViewImpl-ios.h"
#elif (AX_TARGET_PLATFORM == AX_PLATFORM_WINRT)
#include "platform/winrt/GLViewImpl-winrt.h"
#endif

#define USE_AUDIO_ENGINE 1

#if USE_AUDIO_ENGINE
#include <audio/AudioEngine.h>
#endif

#include "FMODAudioEngine.h"

USING_NS_AX;

static ax::Size designResolutionSize = ax::Size(480, 320);
static ax::Size smallResolutionSize = ax::Size(480, 320);
static ax::Size mediumResolutionSize = ax::Size(1024, 768);
static ax::Size largeResolutionSize = ax::Size(2048, 1536);

AppDelegate::AppDelegate() {}

AppDelegate::~AppDelegate() {}

void AppDelegate::initGLContextAttrs()
{
	GLContextAttrs glContextAttrs = { 8, 8, 8, 8, 24, 8, 0 };
		GLView::setGLContextAttrs(glContextAttrs);
}

static int register_all_packages()
{
	return 0;
}

int AppDelegate::applicationGetRefreshRate()
{
#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) || (AX_TARGET_PLATFORM == AX_PLATFORM_LINUX)
	auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		return mode->refreshRate;
#else
	return 60;
#endif
}

static void setupDesignResolution(GLView* glView)
{
	// En Geometry Dash, la altura SIEMPRE es fija (320) para garantizar
	// que la c�mara muestre exactamente la misma cantidad de bloques verticalmente.
	// El ancho se expande din�micamente seg�n lo largo que sea el tel�fono o monitor.

	glView->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::FIXED_HEIGHT);
}

#ifdef AX_PLATFORM_PC
static void onGLFWwindowSizeCallback(GLFWwindow*, int w, int h)
{
	auto director = Director::getInstance();
		auto glView = director->getGLView();

		glView->setFrameSize(w, h);
		setupDesignResolution(glView);

		director->getEventDispatcher()->dispatchCustomEvent(GLViewImpl::EVENT_WINDOW_RESIZED, nullptr);
}
#endif

bool AppDelegate::applicationDidFinishLaunching()
{
	auto director = Director::getInstance();
		auto glView = director->getGLView();
		if (!glView)
		{
#ifdef AX_PLATFORM_PC
			glView = GLViewImpl::createWithRect(
				"OpenGD", ax::Rect(0, 0, 1280, 720), 1.f, true);
#else
			glView = GLViewImpl::create("OpenGD");
#endif

#if (AX_TARGET_PLATFORM == AX_PLATFORM_LINUX)
				int dispX;
				int dispY;
				auto disp = glfwGetPrimaryMonitor();
				glfwGetMonitorPhysicalSize(disp, &dispX, &dispY);
#endif

#if FULLSCREEN == true
				auto full = dynamic_cast<GLViewImpl*>(glView);
				full->setFullscreen();
#endif
				director->setGLView(glView);
		}

	director->setStatsDisplay(SHOW_FPS);
		director->setAnimationInterval(1.0f / applicationGetRefreshRate());
		setupDesignResolution(glView);

#ifdef AX_PLATFORM_PC
		glfwSetWindowAspectRatio(static_cast<GLViewImpl*>(glView)->getWindow(), 16, 9);
		glfwSetWindowSizeCallback(static_cast<GLViewImpl*>(glView)->getWindow(), onGLFWwindowSizeCallback);
#endif

		GameToolbox::log("APLICATION INIT");
		GameToolbox::log("HOLA ANDROID, CARGA FMOD POR FAVOR");

		#ifdef AX_PLATFORM_PC
			director->setContentScaleFactor(GameManager::getInstance()->isHigh() ? 4.0f : 2.0f);
		#endif

		#ifdef AX_PLATFORM_ANDROID
    		director->setContentScaleFactor(4.0f);
		#endif

#if FULLSCREEN == true && AX_TARGET_PLATFORM == AX_PLATFORM_LINUX
		std::cout << "X " << dispX << " Y " << dispY << std::endl;
		glView->setFrameSize((float)dispX, (float)dispY);
#endif

		register_all_packages();

		// --- INTEGRACIÓN FMOD ---
		// Inicializamos tu wrapper correctamente
		FMODAudioEngine::getInstance();

		// FMOD necesita actualizarse cada frame
		director->getScheduler()->schedule([](float dt) {
			FMODAudioEngine::getInstance()->update();
			}, director, 0.0f, false, "FMOD_UPDATE_LOOP");
		// ------------------------

		director->runWithScene(ResourcesLoadingLayer::scene());
		director->setClearColor(ax::Color4F(0.0f, 0.0f, 0.0f, 1.0f));
		return true;
}

void AppDelegate::applicationDidEnterBackground()
{
	Director::getInstance()->stopAnimation();
	FMODAudioEngine::getInstance()->setPaused(true);
}

void AppDelegate::applicationWillEnterForeground()
{
	Director::getInstance()->startAnimation();
	FMODAudioEngine::getInstance()->setPaused(false);
}