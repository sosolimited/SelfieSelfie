//
//  LandscapeGeometry.h
//
//  Created by Soso Limited on 5/28/15.
//
//

#pragma once

///
/// @file Descriptive geometry for the revolved landscape.
///

#include "cinder/Vector.h"
#include <vector>

namespace cinder {
  class JsonTree;
	class Path2dCalcCache;
}

namespace soso {

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

	std::vector<Bar>	getBars(const ci::Path2dCalcCache &path) const;

	ci::JsonTree	toJson() const;
};

} // namespace soso
