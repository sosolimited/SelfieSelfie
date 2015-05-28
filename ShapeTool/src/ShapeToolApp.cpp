#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/svg/Svg.h"
#include "cinder/Path2d.h"
#include "cinder/Shape2d.h"

using namespace ci;
using namespace ci::app;
using namespace std;

///
/// ShapeTool loads a curve from SVG and allows you to decimate it for
/// use as a series of time-sampled shapes in the main graphics application.
///
class ShapeToolApp : public App {
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

private:
	Path2d											_path;
	unique_ptr<Path2dCalcCache> _path_cache;
};

void ShapeToolApp::setup()
{
	auto svg = svg::Doc::create( getAssetPath( "profile.svg" ) );
	auto shape = svg->findByIdContains<svg::Path>( "profile" )->getShape();

	_path = shape.getContour( 0 );
	_path_cache = unique_ptr<Path2dCalcCache>(new Path2dCalcCache(_path));
}

void ShapeToolApp::mouseDown( MouseEvent event )
{
}

void ShapeToolApp::update()
{
}

void ShapeToolApp::draw()
{
	gl::clear( Color( 1, 1, 1 ) );
	gl::ScopedColor color( Color::black() );

	gl::draw( _path );

	for( auto i = 0; i < 100; i += 1 ) {
		auto t = _path_cache->calcNormalizedTime( i / 99.0f );
		auto pos = _path_cache->getPosition( t );
		gl::drawStrokedCircle( pos, 8.0f );
	}
}

void prepareSettings( ci::app::App::Settings *settings )
{
	settings->setWindowSize( 1000, 1000 );
}

CINDER_APP( ShapeToolApp, RendererGl, prepareSettings )
