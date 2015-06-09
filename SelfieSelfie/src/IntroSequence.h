//
//  IntroSequence.h
//
//  Created by Soso Limited on 6/9/15.
//
//

#pragma once

#include "cinder/Timeline.h"
#include "cinder/Filesystem.h"
#include "cinder/gl/Texture.h"
#include "cinder/Timer.h"

namespace soso {

struct SequenceItem
{
	SequenceItem( const ci::gl::TextureRef &iTexture )
	: texture( iTexture )
	{}

	ci::gl::TextureRef	texture;
	ci::Rectf						placement;
	ci::Anim<float>			alpha = 0.0f;
};

///
/// IntroSequence is a series of images that play out on the timeline.
///
class IntroSequence
{
public:
	/// Set up the sequence to load images from the given path.
	void setup( const ci::fs::path &iImageBasePath );
	/// Set a function to be called when the intro animations are complete.
	void setFinishFn( const std::function<void ()> &iFunction ) { finishFn = iFunction; }

	void update();
	void draw();

private:
	std::function<void ()>		finishFn;
	ci::TimelineRef						timeline = ci::Timeline::create();
	std::vector<SequenceItem>	items;
	ci::Timer									timer;

	double										endTime = 0.0;
	void showItem( const ci::fs::path &iPath, float duration );
	void showBlank( float duration );
	void showFlash();
	void handleFinish();
	ci::ColorA								overlayColor = ci::ColorA::hex( 0xffF8ED31 );
	ci::Anim<ci::Color>				backgroundColor = ci::Color::gray( 0.12f );
	ci::Anim<float>						backgroundAlpha = 1.0f;
};

} // namespace soso
