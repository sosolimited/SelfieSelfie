//
//  IntroSequence.h
//
//  Created by Soso Limited on 6/9/15.
//
//

#pragma once

#include "cinder/Timeline.h"

namespace soso {

class SequenceItem
{
public:
	virtual ~SequenceItem() = default;
	virtual void show();
	virtual void hide();
};

///
/// IntroSequence is a series of images that play out on the timeline.
///
class IntroSequence
{
public:
	/// Set up the sequence to load images from the given path.
	void setup( const ci::fs::path &iImageBasePath );
	/// Show the next item.
	void next();
	/// Set a function to be called when the intro animations are complete.
	void setFinishFn( const std::function<void ()> &iFunction ) { finishFn = iFunction; }
private:
	std::function<void ()>		finishFn;
	ci::TimelineRef						timeline = ci::Timeline::create();
	std::vector<SequenceItem>	items;

	void handleFinish();

};

} // namespace soso
