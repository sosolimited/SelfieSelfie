//
//  LandscapeGeometry.cpp
//
//  Created by Soso Limited on 5/28/15.
//
//

#include "LandscapeGeometry.h"
#include "cinder/Path2d.h"
#include "cinder/Json.h"
#include "cinder/Log.h"

using namespace std;
using namespace cinder;
using namespace soso;

inline std::string to_string(const ci::vec2 &vector)
{
	return "[" + toString(vector.x) + "," + toString(vector.y) + "]";
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

#pragma mark - Bar

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
	texture_end(fromString<float>(json["texture_end"].getValue())),
	repeats(fromString<int>(json["repeats"].getValue()))
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

#pragma mark - Section

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



