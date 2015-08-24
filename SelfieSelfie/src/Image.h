//
//  Image.h
//
//  Created by Soso Limited on 6/10/15.
//
//

#pragma once
#include "cinder/Timeline.h"
#include "cinder/gl/Texture.h"
#include "choreograph/Choreograph.h"

namespace soso {

///
/// A positioned Texture.
///
class Image
{
public:
	explicit Image( ci::Surface &&iSurface );
	explicit Image( const ci::gl::TextureRef &iTexture );

	void draw();

	void setPosition( const ci::vec2 &iPos ) { position = iPos; }
	void setAlpha( float iAlpha ) { alpha = iAlpha; }
	void setFullBleedBackground( bool iBleed ) { fullBleedBackground = iBleed; }

	const ci::vec2&			getSize() const { return size; }
	ci::Rectf						getPlacement() const { return ci::Rectf( position(), position() + size ); }

	ch::Output<float>*			getAlphaAnim() { return &alpha; }
	ch::Output<ci::vec2>*		getPositionAnim() { return &position; }

	void setBackingColor( const ci::ColorA &iColor ) { backing = iColor; }
	void setTint( const ci::Color &iColor ) { tint = iColor; }

private:
	ci::gl::TextureRef	texture;
	ch::Output<ci::vec2>	position;
	ci::vec2						size;
	ch::Output<float>			alpha = 1.0f;
	ci::Color						tint = ci::Color::white();
	ci::ColorA					backing = ci::ColorA( 0, 0, 0, 0 );
	bool								fullBleedBackground = false;
};

} // namespace SelfieSelfie
