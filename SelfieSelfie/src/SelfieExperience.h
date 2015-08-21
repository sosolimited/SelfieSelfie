//
//  SelfieExperience.h
//
//  Created by Soso Limited on 8/5/15.
//
//

#pragma once

#include "GridTexture.h"
#include "Landscape.h"
#include "AboutPage.h"
#include "cinder/Capture.h"
#include "TapHandler.h"

namespace soso {

struct TouchInfo {
	TouchInfo() = default;
	TouchInfo( uint32_t iId, const ci::vec2 &iStart, const ci::vec2 &iPosition )
	: id( iId ),
		previous( iStart ),
		position( iPosition )
	{}

  uint32_t		id = 0;
  ci::vec2		previous;
  ci::vec2		position;
};

class SelfieExperience
{
public:
	SelfieExperience( const ci::fs::path &imagePath );
	~SelfieExperience();

	void update();
	void draw();
	void drawScene();

	void drawEndCaps() const;
	void updateCamera();

	void touchesBegan( const ci::app::TouchEvent &event );
  void touchesMoved( const ci::app::TouchEvent &event );
  void touchesEnded( const ci::app::TouchEvent &event );

	void pause();
	void resume();
	void setupCamera();

	void showLandscape();

private:
	ci::CameraPersp						camera;
  ci::CaptureRef						capture;

  GridTextureRef						gridTexture;
  Landscape									landscape;
	AboutPage									aboutPage;
	std::vector<TouchInfo>		touches;

	ci::Anim<float>						cameraWeight = 0.0f;
	ci::Anim<ci::vec3>				cameraEyePoint = ci::vec3( 3.8f, 0.0f, 0.0f );
	ci::vec3									cameraOffset;
	float											cameraVelocity = 0.0f;
	/// Default orientation used during intro.
	ci::quat									startOrientation;
	/// Orientation correction so position held during intro
	ci::quat									orientationOffset;
	/// Interpolated motion orientation to avoid jitter on devices with noisy sensors.
	ci::quat									cameraOrientation;
	ci::signals::ScopedConnection	orientationUpdateConnection;
	ci::signals::ScopedConnection	touchesBeganConnection;
	ci::signals::ScopedConnection touchesMovedConnection;
	ci::signals::ScopedConnection touchesEndedConnection;
};

} // namespace soso
