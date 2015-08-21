//
//  Flash.h
//
//  Created by Soso Limited on 8/21/15.
//
//

#pragma once
#include "cinder/Timeline.h"

namespace soso {

class Flash
{
public:
	Flash() = default;
	explicit Flash(const ci::vec2 &size);
	void draw();
	void flash(float duration);
private:
	ci::Anim<float>	alpha = 0.0f;
	ci::vec2				size;
};

} // namespace soso
