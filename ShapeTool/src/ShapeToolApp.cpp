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

inline std::string to_string(const ci::vec2 &vector)
{
	return "[" + to_string(vector.x) + "," + to_string(vector.y) + "]";
}

namespace cinder{

template<>
vec2 fromString<vec2>(const std::string &string)
{
	auto parts = split(string, ",");
	auto x = fromString<float>(parts.at(0).substr(1, parts.at(0).size()));
	auto y = fromString<float>(parts.at(1).substr(0, parts.at(1).size() - 1));

	return vec2(x, y);
}

} // namespace cinder

struct Bar
{
	Bar() = default;
	Bar(const ci::vec2 &begin, const ci::vec2 &end, int time, float texture_begin, float texture_end, int repeats);
	explicit Bar(const ci::JsonTree &json);

	/// Physical coordinates of bar in profile
	ci::vec2 begin;
	ci::vec2 end;
	/// What frame time offset this bar is played at, in frames.
	int			 time = 0;
	/// What section of the texture this bar reads from, normalized.
	float		 texture_begin = 0.0f;
	float		 texture_end = 1.0f;
	/// How many horizontal repeats of the texture to do (used by main app; not meaningful in 2d).
	int			 repeats = 1;

	ci::JsonTree toJson(float scale) const;
};

Bar::Bar(const ci::vec2 &begin, const ci::vec2 &end, int time, float texture_begin, float texture_end, int repeats)
: begin(begin),
	end(end),
	time(time),
	texture_begin(texture_begin),
	texture_end(texture_end),
	repeats(repeats)
{}

Bar::Bar(const ci::JsonTree &json)
: begin(fromString<vec2>(json["begin"].getValue())),
	end(fromString<vec2>(json["end"].getValue())),
	time(fromString<int>(json["time"].getValue())),
	texture_begin(fromString<float>(json["texture_begin"].getValue())),
	texture_end(fromString<float>(json["texture_end"].getValue()))
{}

ci::JsonTree Bar::toJson(float scale) const
{
	auto bar = JsonTree();
	bar.addChild(JsonTree("begin", to_string(begin * scale)));
	bar.addChild(JsonTree("end", to_string(end * scale)));
	bar.addChild(JsonTree("time", time));
	bar.addChild(JsonTree("texture_begin", texture_begin));
	bar.addChild(JsonTree("texture_end", texture_end));
	bar.addChild(JsonTree("repeats", repeats));

	return bar;
}

struct Section
{
	Section() = default;
	Section(float curve_begin, float curve_end, int time_begin, int spatial_subdivisions, int temporal_steps, int spatial_repeats);
	explicit Section(const ci::JsonTree &json);

	/// Normalized curve times.
	float							curve_begin = 0.0f;
	float							curve_end = 1.0f;
	/// Frame time offset, in frames.
	int								time_begin;
	/// How many bars within this section.
	int								spatial_subdivisions;
	/// How many frames this section spans (steps > 1 yields slit-scanning effects)
	int								temporal_steps;
	/// How many horizontal repeats of the texture to do (used by main app; not meaningful in 2d).
	int								spatial_repeats;

	std::vector<Bar>	getBars(const Path2dCalcCache &path) const;

	ci::JsonTree	toJson() const;
};

Section::Section(float curve_begin, float curve_end, int time_begin, int spatial_subdivisions, int temporal_steps, int spatial_repeats)
: curve_begin(curve_begin),
	curve_end(curve_end),
	time_begin(time_begin),
	spatial_subdivisions(spatial_subdivisions),
	temporal_steps(temporal_steps),
	spatial_repeats(spatial_repeats)
{}

Section::Section(const ci::JsonTree &json)
: curve_begin(json.getValueForKey<float>("curve_begin")),
	curve_end(json.getValueForKey<float>("curve_end")),
	time_begin(json.getValueForKey<int>("time_begin")),
	spatial_subdivisions(json.getValueForKey<int>("spatial_subdivisions")),
	temporal_steps(json.getValueForKey<int>("temporal_steps")),
	spatial_repeats(json.getValueForKey<int>("spatial_repeats"))
{}

ci::JsonTree Section::toJson() const
{
	auto section = JsonTree();
	section.addChild(JsonTree("curve_begin", curve_begin));
	section.addChild(JsonTree("curve_end", curve_end));
	section.addChild(JsonTree("time_begin", time_begin));
	section.addChild(JsonTree("spatial_subdivisions", spatial_subdivisions));
	section.addChild(JsonTree("temporal_steps", temporal_steps));
	section.addChild(JsonTree("spatial_repeats", spatial_repeats));

	return section;
}

std::vector<Bar> Section::getBars(const Path2dCalcCache &path) const
{
	if (temporal_steps > spatial_subdivisions) {
		CI_LOG_W("Temporal subdivisions must be less than spatial subdivisions for all to register");
	}

	vector<Bar> bars;

	for (auto i = 0; i < spatial_subdivisions; i += 1) {
		auto t1 = (i + 0.0f) / spatial_subdivisions;
		auto t2 = (i + 1.0f) / spatial_subdivisions;
		auto time = time_begin + (int)floor(t1 * temporal_steps);

		auto c1 = path.calcNormalizedTime(mix(curve_begin, curve_end, t1), false);
		auto c2 = path.calcNormalizedTime(mix(curve_begin, curve_end, t2), false);

		auto a = path.getPosition(c1);
		auto b = path.getPosition(c2);
		bars.push_back( Bar{ a, b, time, t1, t2, spatial_repeats } );
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

	/// Add a section taking up curve length, offset from the end of the previous section in time
	void addSection( float length, int time_offset, int subdivisions, int temporal_steps, int repeats );

private:
	Path2d											_path;
	unique_ptr<Path2dCalcCache> _path_cache;

	vector<Section>							_sections;
	int													_last_frame = 64; // for debug visualization
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
	/*
	_sections = {Section {0.0f, 0.5f, 0, 8, 4, 4},
								{0.5f, 0.75f, 4, 4, 1, 4},
								{0.75f, 1.0f, 5, 2, 1, 2} };
	*/

	addSection( 0.1f, 1, 2, 1, 4 );
	addSection( 0.1f, 3, 2, 1, 4 );
	addSection( 0.1f, 3, 2, 1, 4 );
	addSection( 0.1f, 3, 2, 1, 4 );
	addSection( 0.1f, 3, 2, 1, 4 );
	addSection( 0.1f, 3, 2, 1, 4 );
	addSection( 0.1f, 3, 2, 1, 4 );
}

void ShapeToolApp::addSection(float length, int time_offset, int subdivisions, int temporal_steps, int repeats)
{
	auto time = time_offset;
	auto curve_begin = 0.0f;

	if (! _sections.empty())
	{
		auto last = _sections.back();
		curve_begin = last.curve_end;
		time = last.time_begin + last.temporal_steps + time_offset;
	}

	auto curve_end = curve_begin + length;
	_sections.push_back(Section(curve_begin, curve_end, time, subdivisions, temporal_steps, repeats));
}

void ShapeToolApp::load(const fs::path &path)
{
	if(fs::exists(path) && fs::is_regular_file(path)) {

		try {
			auto json = JsonTree(loadString (loadFile (path)));
			auto sections = json["sections"];
			_sections.clear();
			for (auto &s : sections.getChildren()) {
				_sections.emplace_back( Section(s) );
			}
		}
		catch (ci::Exception &exc) {
			CI_LOG_E("Error reading file: " << exc.what());
		}
	}
}

void ShapeToolApp::save() const
{
	auto json = JsonTree::makeObject();
	auto scale = 1.0f / 250.0f;

	auto bars = JsonTree::makeArray("bars");
	for (auto &s : _sections)
	{
		auto section_bars = s.getBars(*_path_cache);
		for (auto &b : section_bars)
		{
			bars.pushBack(b.toJson(scale));
		}
	}

	auto sections = JsonTree::makeArray("sections");
	for (auto &s : _sections)
	{
		sections.pushBack(s.toJson());
	}

	json.pushBack(sections);
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
			gl::color(Color(CM_HSV, (b.time + 0.0f) / _last_frame, 1.0f, 1.0f));
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
