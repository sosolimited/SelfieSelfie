//
//  GridMesh.cpp
//
//  Created by Soso Limited on 5/15/15.
//
//

#include "GridMesh.h"
#include "cinder/gl/gl.h"

using namespace cinder;
using namespace soso;

namespace {
  struct Vertex
	{
		ci::vec3 position;
		ci::Color color;
	};

	// Since we are supporting ES2, we can't use shape instancing.
	// On ES3 hardware, we could use a single cross shape and instance it.
	void appendCross( std::vector<Vertex> &iVertices, const ci::vec3 &iCenter, const ci::Color &iColor )
	{
		iVertices.push_back( Vertex{ iCenter - vec3( 0.1, 0, 0 ), iColor } );
		iVertices.push_back( Vertex{ iCenter + vec3( 0.1, 0, 0 ), iColor } );

		iVertices.push_back( Vertex{ iCenter - vec3( 0, 0, 0.1 ), iColor } );
		iVertices.push_back( Vertex{ iCenter + vec3( 0, 0, 0.1 ), iColor } );

		iVertices.push_back( Vertex{ iCenter - vec3( 0, 0.1, 0 ), iColor } );
		iVertices.push_back( Vertex{ iCenter + vec3( 0, 0.1, 0 ), iColor } );
	}
} // namespace


void GridMesh::setup()
{
	std::vector<Vertex> vertices;

	const auto dims = ivec3( 50, 6, 50 );
	auto offset = vec3( -dims ) * vec3( 0.5f, 0.0f, 0.5f );

	for( auto y = 0; y < dims.y; y += 1 ) {
		auto hue = (float)y / dims.y;
		auto color = Color( CM_HSV, hue, 1.0f, 0.8f );
		for( auto x = 0; x < dims.x; x += 1 ) {
			for( auto z = 0; z < dims.z; z += 1 ) {
				appendCross( vertices, vec3( x, y, z ) + offset, color );
			}
		}
	}

	auto vbo = gl::Vbo::create( GL_ARRAY_BUFFER, vertices );
	auto layout = geom::BufferLayout();
	layout.append( geom::Attrib::POSITION, 3, sizeof(Vertex), offsetof(Vertex, position) );
	layout.append( geom::Attrib::COLOR, 3, sizeof(Vertex), offsetof(Vertex, color) );
	auto mesh = gl::VboMesh::create( vertices.size(), GL_LINES, {{ layout, vbo }} );
	auto shader = gl::getStockShader( gl::ShaderDef().color() );

	batch = gl::Batch::create( mesh, shader );
}

void GridMesh::draw() const
{
	batch->draw();
};
