//
//  TimeGrid.cpp
//
//  Created by Soso Limited on 5/22/15.
//
//

#include "TimeGrid.h"

using namespace soso;
using namespace cinder;
using namespace std;

namespace {

struct Vertex {
	vec3	position;
	vec2	tex_coord;
	float time_offset;
};

void addRectangle( vector<Vertex> &vertices, const ci::vec3 &iCenter, float iWidth, float iHeight, float iTimeOffset )
{
	auto right = vec3( iWidth / 2.0f, 0, 0 );
	auto left = - right;
	auto down = vec3( 0, 0, iHeight / 2.0f );
	auto up = - down;

	auto ul = iCenter + (left + up);
	auto ur = iCenter + (right + up);
	auto br = iCenter + (right + down);
	auto bl = iCenter + (left + down);

	vertices.insert( vertices.end(), {
		Vertex{ ul, vec2(0, 0), iTimeOffset },
		Vertex{ ur, vec2(1, 0), iTimeOffset },
		Vertex{ br, vec2(1, 1), iTimeOffset },

		Vertex{ ul, vec2(0, 0), iTimeOffset },
		Vertex{ br, vec2(1, 1), iTimeOffset },
		Vertex{ bl, vec2(0, 1), iTimeOffset }
	} );
}


} // namespace

void TimeGrid::setup( const ci::gl::TextureRef &iTexture )
{
	texture = iTexture;

	auto shader = ([] () {
		try {
			auto shader_format = gl::GlslProg::Format()
				.vertex( app::loadAsset("time_grid.vs") )
				.fragment( app::loadAsset("time_grid.fs") );
			return gl::GlslProg::create( shader_format );
		}
		catch( ci::Exception &exc ) {
			app::console() << "Error loading grid shader: " << exc.what() << endl;
		}
		return gl::GlslProgRef();
	}());

	vector<Vertex> vertices;
	auto dims = ivec2( 12 );
	auto offset = - vec2(dims) * vec2(0.5f);
	auto max_dist = length(vec2(dims)) * 0.5f;

	for( auto y = 0; y < dims.y; y += 1 ) {
		for( auto x = 0; x < dims.x; x += 1 ) {
			auto pos = offset + vec2(x ,y);
			auto d_norm = floor( glm::length( pos ) ) / max_dist;

			addRectangle( vertices, vec3( pos.x, -4.0f, pos.y ), 1.0f, 1.0f, d_norm );

			app::console() << "Distance: " << d_norm << ", at: " << pos << endl;
		}
	}

	auto vbo = gl::Vbo::create( GL_ARRAY_BUFFER, vertices );
	auto layout = geom::BufferLayout();
	layout.append( geom::Attrib::POSITION, 3, sizeof(Vertex), offsetof(Vertex, position) );
	layout.append( geom::Attrib::TEX_COORD_0, 2, sizeof(Vertex), offsetof(Vertex, tex_coord) );
	layout.append( geom::Attrib::CUSTOM_0, 1, sizeof(Vertex), offsetof(Vertex, time_offset) );

	auto mesh = gl::VboMesh::create( vertices.size(), GL_TRIANGLES, { {layout, vbo} } );

	batch = gl::Batch::create( mesh, shader, {{ geom::Attrib::CUSTOM_0, "FrameIndex" }} );
}

void TimeGrid::draw( float iCurrentFrame )
{
	gl::ScopedTextureBind tex0( texture );
	batch->getGlslProg()->uniform( "uFrameIndex", iCurrentFrame );
	batch->draw();
}
