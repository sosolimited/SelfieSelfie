#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/svg/Svg.h"
#include "cinder/Path2d.h"
#include "cinder/Shape2d.h"
#include "cinder/Json.h"
#include "cinder/Log.h"

#include "LandscapeGeometry.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace soso;

///
/// ShapeTool loads a curve from SVG and allows you to decimate it for
/// use as a series of time-sampled shapes in the main graphics application.
///
class ShapeToolApp : public App {
public:
	void setup() override;
	void keyDown(KeyEvent event) override;
	void fileDrop(FileDropEvent event) override;
	void update() override;
	void draw() override;

	void drawTemporalFrames() const;
	void drawSpatialFrames() const;

	void load(const fs::path &path);
	void save() const;

	void saveXml() const;

	/// Add a section taking up curve length, offset from the end of the previous section in time
	void addSection( float length, int time_offset, int subdivisions, int temporal_steps, int repeats );
	int getLastFrame() const;

	void nextPath();

private:
	vector<Section>							_sections;
	int													_last_frame = 64; // for debug visualization
	int													_deform_start = 0;

	unique_ptr<Path2dCalcCache> _path_cache;
	vector<Path2d>							_paths;
	int													_current_path = -1;
};

void ShapeToolApp::setup()
{
	auto svg = svg::Doc::create(getAssetPath("profile.svg"));
	auto *group = svg->findByIdContains<svg::Group>("profiles");

	for (auto &child : group->getChildren()) {
		auto path = child->getShape().getContour(0);
		_paths.push_back(path);
	}
	nextPath();

	// Build a decent starting curve.
	const auto subdivisions = 3;
	const auto basic_delay = 3;
	const auto total_blocks = 144;

	addSection( 0.12f,  basic_delay, subdivisions, 1, 2 );
	addSection( 0.12f,  basic_delay, subdivisions, 1, 5 );
	addSection( 0.11f,  basic_delay, subdivisions, 1, 8 );
	addSection( 0.095f, basic_delay, subdivisions, 1, 11 );
	addSection( 0.085f, basic_delay, subdivisions, 1, 12 );
	addSection( 0.070f, basic_delay, subdivisions, 1, 9 );
	addSection( 0.050f, basic_delay, subdivisions, 1, 7 );

	_deform_start = getLastFrame() + 1;
	auto divisions = total_blocks - _deform_start;
	addSection( -1.0f, 1, divisions, divisions, 2 );
}

void ShapeToolApp::nextPath()
{
	_current_path = (_current_path + 1) % _paths.size();
	auto &p = _paths.at(_current_path);
	_path_cache = unique_ptr<Path2dCalcCache>(new Path2dCalcCache(p));
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

	if (length == -1.0f) {
		length = 1.0f - curve_begin;
	}

	auto curve_end = curve_begin + length;
	_sections.push_back(Section(curve_begin, curve_end, time, subdivisions, temporal_steps, repeats));
}

int ShapeToolApp::getLastFrame() const
{
	if (! _sections.empty())
	{
		const auto &s = _sections.back();
		return s.time_begin + s.temporal_steps;
	}

	return 0;
}

void ShapeToolApp::load(const fs::path &path)
{
	/*
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
	*/
}

void ShapeToolApp::save() const
{
	saveXml();
}

void ShapeToolApp::saveXml() const
{
	auto xml = XmlTree("shape", "");
	auto scale = 1.0f / 1000.0f;

	auto bars = XmlTree("bars", "");
	for (auto &s : _sections)
	{
		auto section_bars = s.getBars(*_path_cache);
		for (auto &b : section_bars)
		{
			bars.push_back(b.toXml(scale));
		}
	}
	xml.push_back(bars);
	xml.push_back(XmlTree("deform_start", toString(_deform_start)));

	auto p = getAssetPath("") / "../../SelfieSelfie/assets/profile.xml";
	xml.write(DataTargetPath::createRef(p));
}

void ShapeToolApp::keyDown(KeyEvent event)
{
	if (event.getCode() == KeyEvent::KEY_s)
	{
		save();
	}

	if (event.getCode() == KeyEvent::KEY_RIGHT)
	{
		nextPath();
	}
	
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
	gl::scale(vec3(0.25f));

	auto scaling = glm::translate(vec3(getWindowCenter(), 0)) * glm::scale(vec3(0.95f)) * glm::translate(vec3(- getWindowCenter(), 0));

	gl::draw(_paths.at(_current_path));

	gl::multModelMatrix(scaling);
	drawTemporalFrames();

	gl::multModelMatrix(scaling);
	drawSpatialFrames();
}

void ShapeToolApp::drawTemporalFrames() const
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

void ShapeToolApp::drawSpatialFrames() const
{
	for (auto &s : _sections)
	{
		auto bars = s.getBars(*_path_cache);
		gl::begin(GL_LINES);
		for (auto &b : bars) {
			if (b.time >= _deform_start) {
				gl::color(Color(1.0f, 1.0f, 0.0f));
				gl::vertex(b.begin + b.normal_begin * 8.0f);
				gl::vertex(b.begin);
				gl::vertex(b.end);
				gl::vertex(b.end + b.normal_end * 8.0f);
			}

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
	settings->setWindowSize(500, 1000);
}

CINDER_APP(ShapeToolApp, RendererGl, prepareSettings)
