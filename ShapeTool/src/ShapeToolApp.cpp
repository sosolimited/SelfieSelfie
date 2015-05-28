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
	ci::vec2 begin;			// where in physical space this bar begins
	ci::vec2 end;
	float		 time;			// frame time offset where this bar is played
	float		 texture_begin;	// where in the texture this bar begins
	float		 texture_end;
};

inline std::string to_string(const ci::vec2 &vector)
{
	return "[" + to_string(vector.x) + "," + to_string(vector.y) + "]";
}

inline ci::vec2 vec2_from_string(const std::string &string)
{
	auto parts = split(string, ",");
	auto x = fromString<float>(parts.at(0).substr(1, parts.at(0).size()));
	auto y = fromString<float>(parts.at(1).substr(0, parts.at(1).size() - 1));

	return vec2(x, y);
}

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
		bars.push_back( Bar{ a, b, time, t1, t2 } );
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

	void drawTemporalFrames();
	void drawSpatialFrames();

	void load(const fs::path &path);
	void save() const;

private:
	Path2d											_path;
	unique_ptr<Path2dCalcCache> _path_cache;

	int													_steps = 12;
	int													_last_frame = 64;

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

	_sections = {Section {0.0f, 0.5f, 0.0f, 8, 4},
								{0.5f, 0.75f, 4.0f, 4, 1},
								{0.75f, 1.0f, 5.0f, 2, 1} };
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
	auto scale = 1.0f / 250.0f;

	auto bars = JsonTree::makeArray("bars");
	for (auto &s : _sections)
	{
		auto section_bars = s.getBars(*_path_cache);
		for (auto &b : section_bars)
		{
			auto bar = JsonTree();
			bar.addChild(JsonTree("begin", to_string(b.begin * scale)));
			bar.addChild(JsonTree("end", to_string(b.end * scale)));
			bar.addChild(JsonTree("texture_begin", b.texture_begin));
			bar.addChild(JsonTree("texture_end", b.texture_end));
			bar.addChild(JsonTree("time", b.time));

			bars.pushBack(bar);
		}
	}

	json.pushBack(bars);

	auto p = getAssetPath("") / "profile.json";
	json.write(p);
}

void ShapeToolApp::mouseDown(MouseEvent event)
{
	save();
}

void ShapeToolApp::fileDrop(cinder::app::FileDropEvent event)
{
	load(event.getFile(0));
}

void ShapeToolApp::update()
{
	_last_frame = 0;
	for (auto &s : _sections)
	{
		_last_frame = max<int>(_last_frame, s.time_begin + s.temporal_steps);
	}
}

void ShapeToolApp::draw()
{
	gl::clear(Color(0, 0, 0));
	gl::ScopedColor color(Color::white());
	gl::ScopedMatrices matrices;

	auto scaling = glm::translate(vec3(getWindowCenter(), 0)) * glm::scale(vec3(0.95f)) * glm::translate(vec3(- getWindowCenter(), 0));

	gl::draw(_path);

	gl::multModelMatrix(scaling);
	drawTemporalFrames();

	gl::multModelMatrix(scaling);
	drawSpatialFrames();
}

void ShapeToolApp::drawTemporalFrames()
{
	for (auto &s : _sections)
	{
		auto bars = s.getBars(*_path_cache);
		for (auto &b : bars) {
			gl::color(Color(CM_HSV, b.time / _last_frame, 1.0f, 1.0f));
			gl::drawLine( b.begin, b.end );
		}
	}
}

void ShapeToolApp::drawSpatialFrames()
{
	for (auto &s : _sections)
	{
		auto bars = s.getBars(*_path_cache);
		gl::begin(GL_LINES);
		for (auto &b : bars) {
			gl::color(Color(b.texture_begin, 0.0f, 0.5f));
			gl::vertex(b.begin);
			gl::color(Color(b.texture_end, 0.0f, 0.5f));
			gl::vertex(b.end);
		}
		gl::end();
	}
}

void prepareSettings(ci::app::App::Settings *settings)
{
	settings->setWindowSize(1000, 1000);
}

CINDER_APP(ShapeToolApp, RendererGl, prepareSettings)
