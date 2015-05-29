//
//  CameraLandscape.h
//
//  Created by Soso Limited on 5/18/15.
//
//

#pragma once

namespace soso {

class CameraLandscape
{
public:
	void setup( const ci::gl::TextureRef &iTexture );
	ci::ivec2 positionOffset( int iPosition ) const;
	void draw( float iCurrentFrame ) const;
private:

	ci::gl::BatchRef		batch;
	ci::gl::TextureRef	texture;
};

} // namespace soso
