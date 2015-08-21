#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Log.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Shader.h"
#include "cinder/Timer.h"

#include "IntroSequence.h"
#include "SelfieExperience.h"

#include "cinder/MotionManager.h"
#include "cinder/Timeline.h"
#include "asio/asio.hpp"

#if defined(CINDER_ANDROID)
	#include "cinder/android/CinderAndroid.h"
#elsif defined(CINDER_COCOA_TOUCH)
	#include "cinder/cocoa/CinderCocoaTouch.h"
#endif

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace soso;

class SelfieSelfieApp : public App {
public:
	SelfieSelfieApp();

	void setup() override;
	void playIntroAndGetOrientation();
	void determineSizeIndicator();
	void update() override;
	void draw() override;

  void focusGained();
  void focusLost();

	void updateCamera();
	void updateOrientationOffset();
	void showLandscape();

	void touchesBegan(TouchEvent event) override;
	void touchesEnded(TouchEvent event) override;

private:
	IntroSequence											introduction;
	std::unique_ptr<SelfieExperience> selfieExperience;
	std::string												sizeIndicator = "xhdpi";
	ci::Timer													touchTimer;
	uint32_t													touchId = 0;
	bool															doSaveImage = false;
	std::vector<std::future<void>>		saveActions;

	void saveComplete(bool success);
};

SelfieSelfieApp::SelfieSelfieApp()
{
	#ifdef CINDER_ANDROID
		ci::android::setActivityGainedFocusCallback( [this] { focusGained(); } );
		ci::android::setActivityLostFocusCallback( [this] { focusLost(); } );
	#endif
}

void SelfieSelfieApp::touchesBegan(cinder::app::TouchEvent event)
{
	touchId = event.getTouches().back().getId();
	touchTimer.start();
}

void SelfieSelfieApp::touchesEnded(cinder::app::TouchEvent event)
{
	if (touchId == event.getTouches().back().getId())
	{
		touchTimer.stop();
		if (touchTimer.getSeconds() < 0.16f) {
			doSaveImage = true;
		}
	}
}

void SelfieSelfieApp::setup()
{
  CI_LOG_I("Setting up selfie_x_selfie");

	determineSizeIndicator();
	auto image_path = fs::path("img") / sizeIndicator;
	introduction.setup( image_path );
	introduction.setFinishFn( [this] { showLandscape(); } );
}

void SelfieSelfieApp::focusGained()
{
	CI_LOG_I("Focus Gained");
	if( selfieExperience ) {
		selfieExperience->resume();
	}
}

void SelfieSelfieApp::focusLost()
{
	CI_LOG_I("Focus Lost");
	if( selfieExperience ) {
		selfieExperience->pause();
	}
}

void SelfieSelfieApp::determineSizeIndicator()
{
	auto large_side = toPixels( max( getWindowWidth(), getWindowHeight() ) );

	if( large_side <= 1280 ) {
		sizeIndicator = "xhdpi";
	}
	else if( large_side <= 1920 ) {
		sizeIndicator = "xxhdpi";
	}
	else {
		sizeIndicator = "xxxhdpi";
	}

	CI_LOG_I( "Device size: " << large_side << " using images for: " << sizeIndicator );
}

void SelfieSelfieApp::update()
{
	if( selfieExperience )
	{
		selfieExperience->update();
	}
	else
	{
		io_service().post( [this] {
			auto image_path = fs::path("img") / sizeIndicator;
			selfieExperience = unique_ptr<SelfieExperience>( new SelfieExperience( image_path ) );
			introduction.start();
		} );
	}
}

void SelfieSelfieApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );

	if( selfieExperience )
	{
		selfieExperience->draw();
	}

	if( doSaveImage )
	{
		doSaveImage = false;

		auto future = std::async(std::launch::async, [this] (const Surface &s) {
			bool success = false;
			try {
				cocoa::writeToSavedPhotosAlbum(s);
				success = true;
			} catch (std::exception &exc) {
				CI_LOG_W("Exception saving image: " << exc.what());
			}

			io_service().post([this, success] {
				saveComplete(success);
			});
		}, copyWindowSurface());

		saveActions.emplace_back(std::move(future));
	}

	introduction.draw();

	#if DEBUG
		auto err = gl::getError();
		if( err ) {
			CI_LOG_E( "Draw gl error: " << gl::getErrorString(err) );
		}
	#endif
}

void SelfieSelfieApp::saveComplete(bool success)
{
	saveActions.erase(std::remove_if(saveActions.begin(), saveActions.end(), [] (const std::future<void> &fut) {
		return ! fut.valid();
	}), saveActions.end());
	CI_LOG_I( (success ? "Saved Screenshot" : "Failed to save image") );
}

void SelfieSelfieApp::showLandscape()
{
	if( ! selfieExperience ) {
		auto image_path = fs::path("img") / sizeIndicator;
		selfieExperience = unique_ptr<SelfieExperience>( new SelfieExperience( image_path ) );
	}
	selfieExperience->showLandscape();
}

inline void prepareSettings( app::App::Settings *iSettings )
{
  iSettings->setMultiTouchEnabled();
	iSettings->setHighDensityDisplayEnabled();
	#ifdef CINDER_ANDROID
		iSettings->setKeepScreenOn(true);
	#endif
}

CINDER_APP( SelfieSelfieApp, RendererGl, &prepareSettings )
