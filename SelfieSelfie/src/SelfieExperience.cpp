//
//  SelfieExperience.cpp
//
//  Created by Soso Limited on 8/5/15.
//
//

#include "SelfieExperience.h"
#include "cinder/MotionManager.h"
#include "cinder/app/App.h"
#include "cinder/Log.h"

using namespace soso;
using namespace cinder;
using namespace std;
using namespace choreograph;

SelfieExperience::SelfieExperience( const fs::path &imagePath )
{
	aboutPage.setup( imagePath );

	CI_LOG_I( "Creating Grid Texture" );
	gridTexture = make_shared<GridTexture>( ivec2( 320, 240 ), 12 );

	CI_LOG_I( "Setting up landscape geometry." );
	landscape.setup();
	landscape.setTextureUnits( 0, 1 );
	landscape.setGridSize( gridTexture->getGridDimensions() );

	setupCamera();

	cameraWeight = 0.0f;
	cameraOffset = vec3( 0 );
  auto target = vec3( 5, 0, 0 );
  camera.lookAt( vec3( 0 ), target, vec3( 0, 1, 0 ) );
  camera.setPerspective( 80, app::getWindowAspectRatio(), 0.1f, 50.0f );
	startOrientation = camera.getOrientation();
	cameraEyePoint = vec3( 3.8f, 0.0f, 0.0f );

	MotionManager::enable();
	orientationUpdateConnection = app::App::get()->getSignalUpdate().connect( [this] {
		auto target = quat( vec3(MotionManager::getRotation() * vec4( 0, 0, -1, 0 )), vec3( 1, 0, 0 ) );
		orientationOffset = normalize( slerp( orientationOffset, target, 0.55f ) );
	} );

	auto window = app::getWindow();
	touchesBeganConnection = window->getSignalTouchesBegan().connect([this] (const app::TouchEvent &event) { touchesBegan( event ); });
	touchesMovedConnection = window->getSignalTouchesMoved().connect([this] (const app::TouchEvent &event) { touchesMoved( event ); });
	touchesEndedConnection = window->getSignalTouchesEnded().connect([this] (const app::TouchEvent &event) { touchesEnded( event ); });
}

SelfieExperience::~SelfieExperience()
{
	if (capture) {
    capture->stop();
	}
}

void SelfieExperience::pause()
{
	if (capture) {
		capture->stop();
		capture.reset();
	}

	MotionManager::disable();
}

void SelfieExperience::resume()
{
	MotionManager::enable();
	setupCamera();
}

void SelfieExperience::setupCamera()
{
	if( ! capture )
	{
		CI_LOG_I("Starting Capture");
		try
		{
			CI_LOG_I( "Initializing hardware camera." );
			auto front_facing_camera = ([] {
				auto &devices = Capture::getDevices();
				auto first_device = devices.front();
				for( auto device : devices ) {
					if( device->isFrontFacing() ) {
						return device;
					}
				}
				return first_device;
			}());

			capture = Capture::create( 1280, 960, front_facing_camera );
			capture->start();
		}
		catch( ci::Exception &exc )
		{
			CI_LOG_E( "Error using device camera: " << exc.what() );
		}
	}
}

void SelfieExperience::update()
{
	aboutPage.update();
	updateCamera();

	if( capture && capture->checkNewFrame() ) {
    gridTexture->addCameraImage( *capture );
  }
}

void SelfieExperience::touchesBegan( const ci::app::TouchEvent &event )
{
  for( auto &t : event.getTouches() ) {
    touches.emplace_back( t.getId(), t.getPos(), t.getPos() );
  }
}

void SelfieExperience::touchesMoved( const ci::app::TouchEvent &event )
{
  for( auto &t : event.getTouches() ) {
    for( auto &s : touches ) {
      if( s.id == t.getId() ) {
				s.previous = s.position;
        s.position = t.getPos();
      }
    }
  }
}

void SelfieExperience::touchesEnded( const ci::app::TouchEvent &event )
{
  touches.erase( std::remove_if( touches.begin(), touches.end(), [&event] (const TouchInfo &s) {
    for( auto &t : event.getTouches() ) {
      if (t.getId() == s.id) {
        return true;
      }
    }
    return false;
  }), touches.end() );
}

void SelfieExperience::updateCamera()
{
	if( touches.size() == 2 )
	{
		auto base = distance(touches.at(0).previous, touches.at(1).previous);
		auto current = distance(touches.at(0).position, touches.at(1).position);

		auto delta = current - base;
		if( isfinite( delta ) )
		{
			cameraVelocity += delta;
		}
	}

	auto ray = camera.getViewDirection();
	cameraVelocity *= cameraWeight.value();
	cameraOffset += ray * cameraVelocity * 0.0025f;
	cameraVelocity *= 0.86f;

	auto l = length(cameraOffset);
	auto maximum = 2.5f;
	if( l > maximum ) {
		cameraOffset *= (maximum / l);
	}
	camera.setEyePoint( cameraEyePoint() + cameraOffset );

	auto r = glm::slerp( startOrientation, (orientationOffset * MotionManager::getRotation()), cameraWeight.value() );
	cameraOrientation = glm::slerp( cameraOrientation, r, 0.5f );
	camera.setOrientation( cameraOrientation );
}

void SelfieExperience::drawEndCaps() const
{
	gl::ScopedGlslProg prog( gl::getStockShader( gl::ShaderDef().texture(gridTexture->getTexture()->getTarget()) ) );
	gl::ScopedTextureBind tex0( gridTexture->getTexture(), 0 );

	auto mat = translate( vec3( 0, -4.0f, 0 ) );
	auto dims = vec2(gridTexture->getCellDimensions()) / gridTexture->getGridSize();
	auto offset = vec2(gridTexture->getIndexOffset( gridTexture->getCellDimensions(), gridTexture->getCurrentIndex() )) / gridTexture->getGridSize();
	auto rect = Rectf( -1.0f, -1.0f, 1.0f, 1.0f ).scaled( vec2( 1.333f, 1.0f ) ).scaled( 0.2f );
	auto half_pi = (float) M_PI / 2.0f;
	auto xf1 = translate( vec3( - 20.0f, 0.0f, 0.0f ) ) * rotate( - half_pi, vec3( 1, 0, 0 ) ) * rotate( half_pi, vec3( 0, 1, 0 ) );
	auto xf2 = translate( vec3( 4.0f, 0.0f, 0.0f ) ) * rotate( half_pi, vec3( 1, 0, 0 ) ) * rotate( - half_pi, vec3( 0, 1, 0 ) );

	auto draw_rect = [rect, dims, offset] (const mat4 &xf) {
		gl::ScopedModelMatrix m;
		gl::multModelMatrix( xf );
		#if defined(CINDER_ANDROID)
			gl::drawSolidRect( rect, offset + vec2( 0, 1 ) * dims, offset + vec2( 1, 0 ) * dims );
		#else
			gl::drawSolidRect( rect, offset, offset + dims );
		#endif
	};

	draw_rect(xf1);
	draw_rect(xf2);
}

void SelfieExperience::drawScene()
{
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::ScopedTextureBind tex0( gridTexture->getTexture(), 0 );
	gl::ScopedTextureBind tex1( gridTexture->getBlurredTexture(), 1 );
	gl::ScopedMatrices matrices;
	gl::setMatrices( camera );

	landscape.draw( gridTexture->getCurrentIndex() );
	drawEndCaps();
}

void SelfieExperience::draw()
{
	drawScene();

  gl::disableDepthRead();
	gl::setMatricesWindow( app::getWindowSize() );
	aboutPage.draw();
}

void SelfieExperience::showLandscape()
{
	// Stop averaging the orientation.
	orientationUpdateConnection.disconnect();
	// Enable looking around with the gyro
	float zoom = 4.2f;
	sharedTimeline().apply( &cameraEyePoint )
		.then<RampTo>( vec3( 0 ), zoom, ch::EaseOutQuart() );
	sharedTimeline().apply( &cameraWeight )
		.hold( zoom )
		.then<RampTo>( 1.0f, 1.33f, ch::EaseInOutCubic() )
		.finishFn( [this] (Motion<float> &m) { aboutPage.show(); } );
}
