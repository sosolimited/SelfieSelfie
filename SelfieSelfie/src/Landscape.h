//
//  Landscape.h
//
//  Created by Soso Limited on 5/26/15.
//
//

#pragma once
#include "cinder/gl/Batch.h"

namespace soso {

///
/// Landscape surface.
/// A revolved curve.
///
class Landscape
{
public:
	void setup();
	void setTextureUnits( uint8_t iClearUnit, uint8_t iBlurredUnit );
  void setGridSize( const ci::vec2 &iSize );
	void draw( float iFrameOffset );
private:
	
	ci::gl::BatchRef	batch;
};

} // namespace soso
