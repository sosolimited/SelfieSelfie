//
//  CameraLandscape.cpp
//
//  Created by Soso Limited on 5/18/15.
//
//

#include "CameraLandscape.h"
#include "cinder/Rand.h"

using namespace soso;
using namespace cinder;
using namespace std;

const double TAU = 6.28318530718;

void CameraLandscape::setup( const ci::gl::TextureRef &iTexture )
{
	texture = iTexture;

	struct Vertex {
		vec3 pos;
		vec3 normal;
		vec2 tc;
		vec2 color_tc;
	};

	auto shader = ([] () {
		try {
			auto shader_format = gl::GlslProg::Format()
				.vertex( app::loadAsset("deform.vs") )
				.fragment( app::loadAsset("deform.fs") );
			return gl::GlslProg::create( shader_format );
		}
		catch( ci::Exception &exc ) {
			app::console() << "Error loading shader: " << exc.what() << endl;
		}
		return gl::GlslProgRef();
	}());

	shader->uniform( "uVideo", 0 );
	auto mat = glm::translate( vec3( 20, 0, 20 ) ) * glm::scale( vec3( 20 ) ) * glm::rotate<float>( M_PI / 4, vec3( 0, 1, 0 ) );
	batch = gl::Batch::create( geom::Teapot().subdivisions( 8 ) >> geom::Transform( mat ), shader );

	vector<Vertex> vertices;
	const auto inner_radius = 3.0f;
	const auto outer_radius = 50.0f;
	const auto rings = 20;
	const auto segments = 64;

	// Generate cartesian position.
	auto calc_pos = [=] (int r, int s) {
		auto distance = (float)r / rings;
		auto radius = mix( inner_radius, outer_radius, distance );
		auto t = (float) s / segments;
		auto x = cos( t * TAU ) * radius;
		auto y = sin( t * TAU ) * radius;
		return vec3( x, -4.0f, y );
	};

	// Generate texture coordinate mirrored at halfway point.
	auto calc_tc = [=] (int r, int s) {
		auto t = (float) s / segments;
		auto tc = vec2(0);
		tc.y = abs( t - 0.5f ) * 2.0f;
		tc.x = lmap<float>( r, 0, rings, 1.0f, 0.0f );
		return tc;
	};

	// Add a vertex to texture (color_tc parameter allows us to do flat shading in ES2)
	auto add_vert = [=,&vertices] (int r, int s, const vec2 &color_tc) {
		auto distance = (float)r / rings;
		auto normal = vec3( 0, mix( 0.0f, 4.0f, distance ), 0 );
		auto pos = calc_pos(r, s);
		auto tc = calc_tc(r, s);
		vertices.push_back( Vertex{ pos, normal, tc, color_tc } );
	};

	// Create triangles for flat shading
	for( auto r = 0; r <= rings; r += 1 )
	{
		for( auto s = 0; s < segments; s += 1 )
		{
			auto color_tc = calc_tc( r, s );
			add_vert( r, s, color_tc );
			add_vert( r, s + 1, color_tc );
			add_vert( r + 1, s + 1, color_tc );

			if( randFloat() < 0.5f ) {
				color_tc = calc_tc( r + 1, s );
			}
			add_vert( r, s, color_tc );
			add_vert( r + 1, s, color_tc );
			add_vert( r + 1, s + 1, color_tc );
		}
	}

	auto vertex_vbo = gl::Vbo::create( GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW );

	auto vertex_layout = geom::BufferLayout();
	vertex_layout.append( geom::Attrib::POSITION, 3, sizeof(Vertex), offsetof(Vertex, pos) );
	vertex_layout.append( geom::Attrib::TEX_COORD_0, 2, sizeof(Vertex), offsetof(Vertex, tc) );
	vertex_layout.append( geom::Attrib::NORMAL, 3, sizeof(Vertex), offsetof(Vertex, normal) );
	vertex_layout.append( geom::Attrib::TEX_COORD_1, 2, sizeof(Vertex), offsetof(Vertex, color_tc) );

	auto mesh = gl::VboMesh::create( vertices.size(), GL_TRIANGLES, {{ vertex_layout, vertex_vbo }} );
	batch = gl::Batch::create( mesh, shader );
}

void CameraLandscape::draw( float iCurrentFrame ) const
{
	if( texture && batch )
	{
		gl::ScopedTextureBind tex0( texture, 0 );
		batch->getGlslProg()->uniform( "uFrameIndex", iCurrentFrame );
		batch->draw();
	}
}
