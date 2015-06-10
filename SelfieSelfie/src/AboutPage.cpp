//
//  AboutPage.cpp
//
//  Created by Soso Limited on 6/10/15.
//
//

#include "AboutPage.h"

using namespace soso;
using namespace cinder;

void AboutPage::setup( const fs::path &iDirectory )
{
	auto about = gl::Texture::create( loadImage( app::loadAsset( iDirectory / "about-tray.png" ) ) );
	auto image = Image( about );
	auto position = vec2( app::getWindowSize() - ivec2( image.getSize() ) ) / 2.0f;
	image.setPosition( position );
//	images.emplace_back( image );
}

void AboutPage::draw()
{
	gl::ScopedAlphaBlend blend( true );

	for( auto &item : images ) {
		item.draw();
	}
}
