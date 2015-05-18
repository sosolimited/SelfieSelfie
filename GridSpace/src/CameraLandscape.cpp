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
	auto shader = gl::getStockShader( gl::ShaderDef().texture( GL_TEXTURE_2D ).color() );
	auto mat = glm::translate( vec3( 20, 0, 20 ) ) * glm::scale( vec3( 20 ) ) * glm::rotate<float>( M_PI / 4, vec3( 0, 1, 0 ) );
	batch = gl::Batch::create( geom::Teapot().subdivisions( 8 ) >> geom::Transform( mat ), shader );
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
}
