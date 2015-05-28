#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/svg/Svg.h"
#include "cinder/Path2d.h"
#include "cinder/Shape2d.h"
#include "cinder/Json.h"

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
	void mouseDown(MouseEvent event) override;
	void update() override;
	void draw() override;

	void load();
	void save() const;

private:
	Path2d											_path;
	unique_ptr<Path2dCalcCache> _path_cache;

	int													_steps = 12;
};

void ShapeToolApp::setup()
{
	auto svg = svg::Doc::create(getAssetPath("profile.svg"));
	auto shape = svg->findByIdContains<svg::Path>("profile")->getShape();

	_path = shape.getContour(0);
	_path_cache = unique_ptr<Path2dCalcCache>(new Path2dCalcCache(_path));

	load();
}

void ShapeToolApp::load()
{
	auto p = getAssetPath( "profile.json" );
	if( fs::exists( p ) ) {
		console() << "Loading configuration" << endl;

		auto json = JsonTree(loadString (loadFile (p)));
		_steps = json["steps"].getValue<int>();
	}
	else {
		save();
	}
}

void ShapeToolApp::save() const
{
	auto json = JsonTree::makeObject();
	json.pushBack(JsonTree("steps", _steps));

	auto p = getAssetPath("") / "profile.json";
	json.write(p);
}

void ShapeToolApp::mouseDown(MouseEvent event)
{
}

void ShapeToolApp::update()
{

}

void ShapeToolApp::draw()
{
	gl::clear(Color(1, 1, 1));
	gl::ScopedColor color(Color::black());

	gl::draw(_path);

	for (auto i = 0; i < _steps; i += 1)
	{
		auto t = i / (_steps - 1.0f);
		t = _path_cache->calcNormalizedTime(t, false);
		auto pos = _path_cache->getPosition(t);
		gl::drawStrokedCircle(pos, 8.0f);
	}
}

void prepareSettings(ci::app::App::Settings *settings)
{
	settings->setWindowSize(1000, 1000);
}

CINDER_APP(ShapeToolApp, RendererGl, prepareSettings)
