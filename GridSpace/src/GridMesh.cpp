//
//  GridMesh.cpp
//
//  Created by Soso Limited on 5/15/15.
//
//

#include "GridMesh.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

using namespace cinder;
using namespace soso;

namespace {
  struct Vertex
	{
		ci::vec3	position;
		ci::vec2	tex_coord;
		float			time_offset;
	};

	// Since we are supporting ES2, we can't use shape instancing.
	// On ES3 hardware, we could use a single cross shape and instance it.
	void appendCross( std::vector<Vertex> &iVertices, const ci::vec3 &iCenter, const ci::vec2 &iTexCoord, float iTimeOffset )
	{
		iVertices.push_back( Vertex{ iCenter - vec3( 0.1, 0, 0 ), iTexCoord, iTimeOffset } );
		iVertices.push_back( Vertex{ iCenter + vec3( 0.1, 0, 0 ), iTexCoord, iTimeOffset } );

		iVertices.push_back( Vertex{ iCenter - vec3( 0, 0, 0.1 ), iTexCoord, iTimeOffset } );
		iVertices.push_back( Vertex{ iCenter + vec3( 0, 0, 0.1 ), iTexCoord, iTimeOffset } );

		iVertices.push_back( Vertex{ iCenter - vec3( 0, 0.1, 0 ), iTexCoord, iTimeOffset } );
		iVertices.push_back( Vertex{ iCenter + vec3( 0, 0.1, 0 ), iTexCoord, iTimeOffset } );
	}
} // namespace


void GridMesh::setup()
{
	CI_LOG_I("Building GridMesh");
	std::vector<Vertex> vertices;

	const auto dims = ivec3( 50, 8, 50 );
	auto offset = vec3( -dims ) * vec3( 0.5f, 0.025f, 0.5f );

	for( auto y = 0; y < dims.y; y += 1 ) {
		auto time_offset = (float) y / dims.y;
		time_offset *= dims.y / 32.0f;
		for( auto x = 0; x < dims.x; x += 1 ) {
			for( auto z = 0; z < dims.z; z += 1 ) {
				appendCross( vertices, vec3( x, y, z ) + offset, vec2( x, z ) / vec2( dims.x, dims.z ), time_offset );
			}
		}
	}

	auto vbo = gl::Vbo::create( GL_ARRAY_BUFFER, vertices );
	auto layout = geom::BufferLayout();
	layout.append( geom::Attrib::POSITION, 3, sizeof(Vertex), offsetof(Vertex, position) );
	layout.append( geom::Attrib::TEX_COORD_0, 2, sizeof(Vertex), offsetof(Vertex, tex_coord) );
	layout.append( geom::Attrib::CUSTOM_0, 1, sizeof(Vertex), offsetof(Vertex, time_offset) );
	auto mesh = gl::VboMesh::create( vertices.size(), GL_LINES, {{ layout, vbo }} );
	auto shader = gl::GlslProg::create( app::loadAsset("grid_mesh.vs"), app::loadAsset("grid_mesh.fs") );

	batch = gl::Batch::create( mesh, shader, {{ geom::Attrib::CUSTOM_0, "FrameIndex" }} );
}

void GridMesh::draw( float iCurrentFrame ) const
{
	batch->getGlslProg()->uniform( "uFrameIndex", iCurrentFrame );
	batch->draw();
};
