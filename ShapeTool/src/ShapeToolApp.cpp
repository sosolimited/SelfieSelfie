#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/svg/Svg.h"
#include "cinder/Path2d.h"
#include "cinder/Shape2d.h"
#include "cinder/Json.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

struct Bar
{
	ci::vec2 begin;
	ci::vec2 end;
	float		 time;
};

struct Section
{
	float							curve_begin;
	float							curve_end;
	float							time_begin;
	int								spatial_subdivisions;
	int								temporal_steps; // 1 = no divisions

	std::vector<Bar>	getBars(const Path2dCalcCache &path) const;
};

std::vector<Bar> Section::getBars(const Path2dCalcCache &path) const
{
	if (temporal_steps > spatial_subdivisions) {
		CI_LOG_W("Temporal subdivisions must be less than spatial subdivisions for all to register");
	}

	vector<Bar> bars;

	for (auto i = 0; i < spatial_subdivisions; i += 1) {
		auto t1 = (i + 0.0f) / spatial_subdivisions;
		auto t2 = (i + 1.0f) / spatial_subdivisions;
		auto time = time_begin + floor(t1 * temporal_steps);

		auto c1 = path.calcNormalizedTime(mix(curve_begin, curve_end, t1), false);
		auto c2 = path.calcNormalizedTime(mix(curve_begin, curve_end, t2), false);

		auto a = path.getPosition(c1);
		auto b = path.getPosition(c2);
		bars.push_back( Bar{ a, b, time } );
	}

	return bars;
}

///
/// ShapeTool loads a curve from SVG and allows you to decimate it for
/// use as a series of time-sampled shapes in the main graphics application.
///
class ShapeToolApp : public App {
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void fileDrop(FileDropEvent event) override;
	void update() override;
	void draw() override;

	void load(const fs::path &path);
	void save() const;

private:
	Path2d											_path;
	unique_ptr<Path2dCalcCache> _path_cache;

	int													_steps = 12;
	int													_total_frames = 64;

	vector<Section>							_sections;
};

void ShapeToolApp::setup()
{
	auto svg = svg::Doc::create(getAssetPath("profile.svg"));
	auto shape = svg->findByIdContains<svg::Path>("profile")->getShape();

	_path = shape.getContour(0);
	_path_cache = unique_ptr<Path2dCalcCache>(new Path2dCalcCache(_path));

	/*
	auto p = getAssetPath("profile.json");
	load(p);
	*/

	_sections = {Section {0.0f, 0.5f, 0.0f, 4, 4} };
}

void ShapeToolApp::load(const fs::path &path)
{
	if(fs::exists(path) && fs::is_regular_file(path)) {
		auto json = JsonTree(loadString (loadFile (path)));
		_steps = json["steps"].getValue<int>();
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

void ShapeToolApp::fileDrop(cinder::app::FileDropEvent event)
{
	load(event.getFile(0));
}

void ShapeToolApp::update()
{
	_total_frames = 0;
	for (auto &s : _sections)
	{
		_total_frames += s.temporal_steps;
	}
}

void ShapeToolApp::draw()
{
	gl::clear(Color(1, 1, 1));
	gl::ScopedColor color(Color::black());

	gl::draw(_path);

	for (auto &s : _sections)
	{
		auto bars = s.getBars(_path);
		for (auto &b : bars) {
			gl::color(Color(CM_HSV, b.time / _total_frames, 1.0f, 1.0f));
			gl::drawLine( b.begin, b.end );
		}
	}
}

void prepareSettings(ci::app::App::Settings *settings)
{
	settings->setWindowSize(1000, 1000);
}

CINDER_APP(ShapeToolApp, RendererGl, prepareSettings)
