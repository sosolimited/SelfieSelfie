//
//  Stars.h
//
//  Created by Soso Limited on 5/15/15.
//
//

#pragma once

namespace soso {

class Stars
{
public:
	void setup();
	void draw( float iCurrentFrame ) const;

private:

	ci::gl::BatchRef	batch;
};

} // namespace soso
