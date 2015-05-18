//
//  CameraLandscape.cpp
//
//  Created by Soso Limited on 5/18/15.
//
//

#include "CameraLandscape.h"

using namespace soso;
using namespace cinder;

void CameraLandscape::setup()
{

}

void CameraLandscape::updateTexture( const ci::Surface &iImage )
{
	if( ! texture ) {
		texture = gl::Texture::create( iImage, gl::Texture::Format().loadTopDown() );
	}
	else {
		texture->update( iImage );
	}
}

void CameraLandscape::draw() const
{

	if( texture && batch )
	{
		gl::ScopedTextureBind tex0( texture, 0 );
		batch->draw();

	}

	if( texture ) {
		gl::ScopedModelMatrix mat;
		gl::translate( vec3( 0, 0, 50.0f ) );
		gl::draw( texture );
	}
}
