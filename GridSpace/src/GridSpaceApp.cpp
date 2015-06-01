#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Capture.h"
#include "cinder/Log.h"

#include "GridTexture.h"
#include "Landscape.h"
#include "Constants.h"

#ifdef CINDER_COCOA_TOUCH
	#include "cinder/MotionManager.h"
#endif

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace soso;

struct TouchInfo {
	uint32_t  id;
	vec2			previous;
	vec2			position;
};

class GridSpaceApp : public App {
public:
	void setup() override;
	void touchesBegan( TouchEvent event ) override;
	void touchesMoved( TouchEvent event ) override;
	void touchesEnded( TouchEvent event ) override;

	void pinchStart();
	void pinchUpdate();
	void pinchEnd();

	void update() override;
	void draw() override;

  void drawSimpleAndroidTestStuff() const;

private:
	CaptureRef			capture;
	CameraPersp			camera;

	GridTextureRef	gridTexture;
	Landscape				landscape;

	bool						doDrawDebug = false;

	vector<TouchInfo> touches;
	ci::vec3					cameraOffset;
};

void GridSpaceApp::setup()
{
	CI_LOG_I("Setting up selfie_x_selfie");

	#ifdef CINDER_COCOA_TOUCH
		MotionManager::enable();
	#endif
	GLint size;
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &size );
	CI_LOG_I( "Max texture size: " << size );

	auto target = vec3( 5, 0, 0 );
	camera.lookAt( vec3( 0 ), target, vec3( 0, 1, 0 ) );
	camera.setPerspective( 80, getWindowAspectRatio(), 0.1f, 1000 );

/*
	try {
		CI_LOG_I("Attempting to set up camera input.");
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

		capture = Capture::create( 480, 360, front_facing_camera );
		capture->start();
		const auto divisions = 8;
		const auto size = divisions * capture->getSize();
		gridTexture = make_shared<GridTexture>( size.x, size.y, divisions );
	}
	catch( ci::Exception &exc ) {
		CI_LOG_E( "Error using device camera: " << exc.what() );
	}
//*/
/*
  auto tex_size = vec2( 640, 480 );
  gridTexture = make_shared<GridTexture>( tex_size.x, tex_size.y, 8 );
	landscape.setup();
  */
}

void GridSpaceApp::touchesBegan( TouchEvent event )
{
	CI_LOG_I("Touches began");
	console() << "Touches began." << endl;
	for( auto &t : event.getTouches() ) {
		touches.push_back( { t.getId(), t.getPos(), t.getPos() } );
	}

	if( touches.size() == 2 ) {
		pinchStart();
	}
}

void GridSpaceApp::touchesMoved( TouchEvent event )
{
	CI_LOG_I("Touches moved");
	console() << "Touches moved." << endl;
	for( auto &t : event.getTouches() ) {
		for( auto &s : touches ) {
			if( s.id == t.getId() ) {
				s.previous = s.position;
				s.position = t.getPos();
			}
		}
	}

	if( touches.size() == 2 ) {
		pinchUpdate();
	}
}

void GridSpaceApp::touchesEnded( TouchEvent event )
{
	console() << "Touches ended." << endl;
	touches.erase( std::remove_if( touches.begin(), touches.end(), [&event] (const TouchInfo &s) {
		for( auto &t : event.getTouches() ) {
			if (t.getId() == s.id) {
				return true;
			}
		}
		return false;
	}), touches.end() );
}

void GridSpaceApp::pinchStart()
{
}

void GridSpaceApp::pinchUpdate()
{
	console() << "Pinch update." << endl;
	auto base = distance(touches.at( 0 ).previous, touches.at( 1 ).previous);
	auto current = distance(touches.at( 0 ).position, touches.at( 1 ).position);

	auto delta = current - base;
	if( isfinite( delta ) )
	{
		auto ray = camera.getViewDirection();
		cameraOffset += delta * ray * 0.01f;
	}
}

void GridSpaceApp::update()
{
	auto l = length(cameraOffset);
	auto maximum = 3.0f;
	if( l > maximum ) {
		cameraOffset *= (maximum / l);
	}
	camera.setEyePoint( cameraOffset );
	#ifdef CINDER_COCOA_TOUCH
		if( MotionManager::isDataAvailable() ) {
			auto r = MotionManager::getRotation();
			camera.setOrientation( r );
		}
	#endif
	/*
	if( capture->checkNewFrame() ) {
		gridTexture->update( *capture->getSurface() );
	}
	*/
}

void GridSpaceApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::enableDepthRead();
	gl::enableDepthWrite();

	gl::setMatrices( camera );

/*
  gl::ScopedTextureBind tex0( gridTexture->getTexture(), 0 );
  gl::ScopedTextureBind tex1( gridTexture->getBlurredTexture(), 1 );
  landscape.draw( gridTexture->getCurrentIndex() );
  */

  drawSimpleAndroidTestStuff();

  /*
	if( doDrawDebug )
	{
		if( gridTexture->getTexture() )
		{
			auto size = vec2(getWindowSize()) * vec2(1.0, 0.5);
			gl::ScopedMatrices mat;
			gl::setMatricesWindow( app::getWindowSize() );
			gl::draw( gridTexture->getTexture(), Rectf( vec2(0), size ) );

			gl::translate( size * vec2(0, 1) );
			gl::draw( gridTexture->getBlurredTexture(), Rectf( vec2(0), size ) );
		}
	}
	*/
}

void GridSpaceApp::drawSimpleAndroidTestStuff() const
{
  gl::ScopedColor color( Color( 1.0f, 0.0f, 1.0f ) );
  gl::ScopedMatrices matrices;

  gl::drawSphere( vec3( 5, 0, 0 ), 1.0f, -1 );

  gl::setMatricesWindow( getWindowSize() );
  gl::color( 1.0f, 1.0f, 0.0f );
  gl::drawSolidCircle( vec2( 100.0f, 100.0f ), 20.0f );
}

inline void prepareSettings(app::App::Settings *settings)
{
  settings->setMultiTouchEnabled();
}

CINDER_APP( GridSpaceApp, RendererGl, &prepareSettings )
