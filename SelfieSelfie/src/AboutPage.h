//
//  AboutPage.h
//
//  Created by Soso Limited on 6/10/15.
//
//

#pragma once
#include "Image.h"

namespace soso {

class AboutPage
{
public:
	void setup( const ci::fs::path &iDirectory );
	void draw();
private:
	ci::TimelineRef	timeline = ci::Timeline::create();
	std::unique_ptr<Image>	description;
	std::unique_ptr<Image>	icon;
};

} // namespace soso
