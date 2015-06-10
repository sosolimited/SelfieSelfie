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
	std::vector<Image>	images;
	ci::TimelineRef			timeline = ci::Timeline::create();
};

} // namespace soso
