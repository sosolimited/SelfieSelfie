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
	void setup();
	void updateTexture( const ci::Surface &iImage );
	void draw() const;
private:

	ci::gl::BatchRef		batch;
	ci::gl::TextureRef	texture;
	int									line = 0;
};

} // namespace soso
