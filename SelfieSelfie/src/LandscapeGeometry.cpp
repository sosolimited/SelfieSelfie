//
//  LandscapeGeometry.cpp
//
//  Created by Soso Limited on 5/28/15.
//
//

#include "LandscapeGeometry.h"
#include "cinder/Path2d.h"
#include "cinder/Json.h"
#include "cinder/Xml.h"
#include "cinder/Log.h"

using namespace std;
using namespace cinder;
using namespace soso;

inline std::string toString(const ci::vec2 &vector)
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

Bar::Bar(const ci::vec2 &begin, const ci::vec2 &end, const ci::vec2 &begin_normal, const ci::vec2 &end_normal, int time, float texture_begin, float texture_end, int repeats)
: begin(begin),
	end(end),
  normal_begin(begin_normal),
  normal_end(end_normal),
	time(time),
	texture_begin(texture_begin),
	texture_end(texture_end),
	repeats(repeats)
{}

Bar::Bar(const ci::XmlTree &xml)
: begin(fromString<vec2>(xml.getChild("begin").getValue())),
	end(fromString<vec2>(xml.getChild("end").getValue())),
  normal_begin(fromString<vec2>(xml.getChild("normal_begin").getValue())),
  normal_end(fromString<vec2>(xml.getChild("normal_end").getValue())),
	time(fromString<int>(xml.getChild("time").getValue())),
	texture_begin(fromString<float>(xml.getChild("texture_begin").getValue())),
	texture_end(fromString<float>(xml.getChild("texture_end").getValue())),
	repeats(fromString<int>(xml.getChild("repeats").getValue()))
{}

ci::XmlTree Bar::toXml(float scale) const
{
	auto bar = XmlTree("b", "");
	bar.push_back(XmlTree("begin", toString(begin * scale)));
	bar.push_back(XmlTree("end", toString(end * scale)));
  bar.push_back(XmlTree("normal_begin", toString(normal_begin)));
  bar.push_back(XmlTree("normal_end", toString(normal_end)));
	bar.push_back(XmlTree("time", toString(time)));
	bar.push_back(XmlTree("texture_begin", toString(texture_begin)));
	bar.push_back(XmlTree("texture_end", toString(texture_end)));
	bar.push_back(XmlTree("repeats", toString(repeats)));

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

std::vector<Bar> Section::getBars(const Path2dCalcCache &path) const
{
	if (temporal_steps > spatial_subdivisions) {
		CI_LOG_W("Temporal subdivisions must be less than spatial subdivisions for all to register");
	}

	vector<Bar> bars;

	for (auto i = 0; i < spatial_subdivisions; i += 1) {
		const auto t1 = (i + 0.0f) / spatial_subdivisions;
		const auto t2 = (i + 1.0f) / spatial_subdivisions;
		const auto time = time_begin + (int)floor(t1 * temporal_steps);

		auto c1 = path.calcNormalizedTime(mix(curve_begin, curve_end, t1), false);
		auto c2 = path.calcNormalizedTime(mix(curve_begin, curve_end, t2), false);
    auto tan1 = path.getPath2d().getTangent( c1 );
    auto tan2 = path.getPath2d().getTangent( c2 );
    auto n1 = normalize(vec2(- tan1.y, tan1.x));
    auto n2 = normalize(vec2(- tan2.y, tan2.x));

		auto a = path.getPosition(c1);
		auto b = path.getPosition(c2);
		bars.push_back( Bar{ a, b, n1, n2, time, t1, t2, spatial_repeats } );
	}

	return bars;
}
