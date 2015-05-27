//
//  Landscape.h
//
//  Created by Soso Limited on 5/26/15.
//
//

#pragma once

namespace soso {

///
/// Patch between the landscape and time grid geometry.
///
class Landscape
{
public:
	void setup();
	void setTextureUnits( uint8_t iClearUnit, uint8_t iBlurredUnit );
	void draw( float iFrameOffset );
private:
	
	ci::gl::BatchRef	batch;
};

} // namespace soso
