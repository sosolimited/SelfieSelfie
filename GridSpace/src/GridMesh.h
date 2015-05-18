//
//  GridMesh.h
//
//  Created by Soso Limited on 5/15/15.
//
//

#pragma once

namespace soso {

class GridMesh
{
public:
	void setup();
	void draw() const;

private:

	ci::gl::BatchRef	batch;
};

} // namespace soso
