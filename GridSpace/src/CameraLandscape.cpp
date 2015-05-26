//
//  CameraLandscape.cpp
//
//  Created by Soso Limited on 5/18/15.
//
//

#include "CameraLandscape.h"
#include "cinder/Rand.h"
#include "cinder/Log.h"

using namespace soso;
using namespace cinder;
using namespace std;

const double TAU = 6.28318530718;

void CameraLandscape::setup( const ci::gl::TextureRef &iTexture )
{
	CI_LOG_I("Building CameraLandscape");
	texture = iTexture;

	struct Vertex {
		vec3 pos;
		vec3 normal;
		vec2 tc;
		vec2 color_tc;
		float time_offset;
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
	const auto inner_radius = 8.0f;
	const auto outer_radius = 50.0f;
	const auto rings = 64;
	const auto segments = 128;

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
		// insetting the texture coordinates minimizes edge color flashing.
		tc.y = mix( 0.05f, 0.95f, abs( t - 0.5f ) * 2.0f );
		tc.x = lmap<float>( r, 0, rings, 0.9125f, 0.0875f );
		return tc;
	};

	// Add a vertex to texture (color_tc parameter allows us to do flat shading in ES2)
	auto add_vert = [=,&vertices] (int r, int s, const vec2 &color_tc) {
		auto distance = (float)r / rings;
		auto normal = vec3( 0, 1.0f, 0 ); // mix( 0.0f, 4.0f, distance )
		auto time_offset = distance;
		auto pos = calc_pos(r, s);
		auto tc = calc_tc(r, s);
		vertices.push_back( Vertex{ pos, normal, tc, color_tc, time_offset } );
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

			add_vert( r, s, color_tc );
			add_vert( r + 1, s, color_tc );
			add_vert( r + 1, s + 1, color_tc );
		}
	}

	// Deform vertices into a bowl
	auto max_distance = outer_radius;
	for( auto &v : vertices ) {
		auto distance = length( v.pos ) / max_distance;
		// pos is on a radial axis, rotate it 90 degrees to bend along length
		auto axis = glm::rotate( glm::angleAxis( (float)TAU / 4.0f, vec3(0, 1, 0) ), normalize(v.pos) );
		auto theta = mix( 0.0f, -(float)TAU / 18.0f, distance );
		auto xf = glm::rotate( theta, axis );
		v.normal = vec3(xf * vec4(v.normal, 0.0f));
		v.normal *= mix( 0.0f, 4.0f, distance );
		v.pos = vec3(xf * vec4(v.pos, 1.0f));
	}

	auto vertex_vbo = gl::Vbo::create( GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW );

	auto vertex_layout = geom::BufferLayout();
	vertex_layout.append( geom::Attrib::POSITION, 3, sizeof(Vertex), offsetof(Vertex, pos) );
	vertex_layout.append( geom::Attrib::TEX_COORD_0, 2, sizeof(Vertex), offsetof(Vertex, tc) );
	vertex_layout.append( geom::Attrib::NORMAL, 3, sizeof(Vertex), offsetof(Vertex, normal) );
	vertex_layout.append( geom::Attrib::TEX_COORD_1, 2, sizeof(Vertex), offsetof(Vertex, color_tc) );
	vertex_layout.append( geom::Attrib::CUSTOM_0, 1, sizeof(Vertex), offsetof(Vertex, time_offset) );

	auto mesh = gl::VboMesh::create( vertices.size(), GL_TRIANGLES, {{ vertex_layout, vertex_vbo }} );
	batch = gl::Batch::create( mesh, shader, {{ geom::Attrib::CUSTOM_0, "FrameIndex" }} );
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
