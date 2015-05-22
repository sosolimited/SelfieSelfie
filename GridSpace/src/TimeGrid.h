//
//  TimeGrid.h
//
//  Created by Soso Limited on 5/22/15.
//
//

#pragma once

namespace soso {

///
/// A grid rippling outward in time.
/// Each cell of the grid has a temporal offset.
/// Offsets into the GridTexture to get the current time.
///
class TimeGrid
{
public:
	void setup( const ci::gl::TextureRef &iTexture );
	/// Draw to screen assuming iCurrentFrame is the index of the newest frame.
	void draw( float iCurrentFrame );
private:
	ci::gl::TextureRef	texture;
	ci::gl::BatchRef		batch;
};

} // namespace soso
