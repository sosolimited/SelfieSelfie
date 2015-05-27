//
//  Landscape.cpp
//
//  Created by Soso Limited on 5/26/15.
//
//

#include "Landscape.h"

#include "cinder/Log.h"
#include "Constants.h"

using namespace soso;
using namespace cinder;

namespace {

struct alignas(16) Vertex {
	vec3	position;
	vec3	normal;
	vec2	tex_coord;
	vec2	color_tex_coord;
	float deform_scaling;
	float frame_offset;
	float color_weight;
};

const auto kVertexLayout = ([] {
		auto layout = geom::BufferLayout();
		layout.append( geom::Attrib::POSITION, 3, sizeof(Vertex), offsetof(Vertex, position) );
		layout.append( geom::Attrib::NORMAL, 3, sizeof(Vertex), offsetof(Vertex, normal) );
		layout.append( geom::Attrib::TEX_COORD_0, 2, sizeof(Vertex), offsetof(Vertex, tex_coord) );
		layout.append( geom::Attrib::TEX_COORD_1, 2, sizeof(Vertex), offsetof(Vertex, color_tex_coord) );
		layout.append( geom::Attrib::CUSTOM_0, 1, sizeof(Vertex), offsetof(Vertex, deform_scaling) );
		layout.append( geom::Attrib::CUSTOM_1, 1, sizeof(Vertex), offsetof(Vertex, frame_offset) );
		layout.append( geom::Attrib::CUSTOM_2, 1, sizeof(Vertex), offsetof(Vertex, color_weight) );
		return layout;
	} ());

const auto kVertexMapping = ([] {
		return gl::Batch::AttributeMapping{ { geom::Attrib::CUSTOM_0, "DeformScaling" }, { geom::Attrib::CUSTOM_1, "FrameOffset" }, { geom::Attrib::CUSTOM_2, "ColorWeight" } };
	} ());

// Load a shader and handle exceptions. Return nullptr on failure.
gl::GlslProgRef loadShader( const fs::path &iVertex, const fs::path &iFragment )
{
	try {
		auto shader_format = gl::GlslProg::Format()
			.vertex( app::loadAsset( iVertex ) )
			.fragment( app::loadAsset( iFragment ) );

		return gl::GlslProg::create( shader_format );
	}
	catch( ci::Exception &exc ) {
		CI_LOG_E( "Error loading shader: " << exc.what() );
	}

	return nullptr;
}

///
/// Create a quadrilateral with clockwise vertices abcd.
/// Maps to frame at time and a slice of the video frame.
///
void addQuad( std::vector<Vertex> &vertices, const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d, float time, const Rectf &slice )
{
	auto normal = vec3( 0, 1, 0 );
	auto deform_scaling = 0.0f;

	vertices.insert( vertices.end(), {
		Vertex{ a, normal, slice.getUpperLeft(), slice.getUpperLeft(), deform_scaling, time, 0.0f },
		Vertex{ b, normal, slice.getUpperRight(), slice.getUpperRight(), deform_scaling, time, 0.0f },
		Vertex{ c, normal, slice.getLowerRight(), slice.getLowerRight(), deform_scaling, time, 0.0f },

		Vertex{ a, normal, slice.getUpperLeft(), slice.getUpperLeft(), deform_scaling, time, 0.0f },
		Vertex{ c, normal, slice.getLowerRight(), slice.getLowerRight(), deform_scaling, time, 0.0f },
		Vertex{ d, normal, slice.getLowerLeft(), slice.getLowerLeft(), deform_scaling, time, 0.0f }
	} );

}

/// Add a ring of geometry containing a given number of time bands (slitscanning effect) and repeats around the donut.
void addRing( std::vector<Vertex> &vertices, const vec3 &center, const vec3 &normal, float inner_radius, float outer_radius, float time_offset, int time_bands, int repeats, float color_weight )
{
	auto rings = time_bands;
	auto segments = 64;

	// Generate cartesian position.
	auto calc_pos = [=] (int r, int s) {
		auto distance = lmap<float>( r, 0, rings, 0.0f, 1.0f );
		auto radius = mix( inner_radius, outer_radius, distance );
		auto t = (float) s / segments;
		auto x = cos( t * Tau ) * radius;
		auto y = sin( t * Tau ) * radius;
		return vec3( x, 0, y ) + center;
	};

	// Generate texture coordinate mirrored at halfway point.
	auto calc_tc = [=] (int r, int s) {
		auto t = (float) s / segments;
		if( t < 1 ) {
			t = glm::fract( t * repeats );
		}
		// mirror copies
		t = std::abs( t - 0.5f ) * 2.0f;
		auto tc = vec2(0);
		// Repeat t with mirroring
		// insetting the texture coordinates minimizes edge color flashing.
		tc.y = mix( 0.05f, 0.95f, t );
		tc.x = lmap<float>( r, 0, rings, 0.9125f, 0.0875f );
		return tc;
	};

	// Add a vertex to texture (color_tc parameter allows us to do flat shading in ES2)
	auto add_vert = [=,&vertices] (int r, int s, const ivec2 &provoking) {
		auto deform_scaling = 0.0f;
		auto pos = calc_pos(r, s);
		auto tc = calc_tc(r, s);
		auto color_tc = calc_tc( provoking.x, provoking.y );
		auto time = time_offset; // + lmap<float>( r, 0, rings, 0.0f, 1.0f / 64.0f );
		if (time_bands > 1) {
			time += lmap<float>( provoking.x, 0.0f, rings, 0.0f, time_bands );
		}
		vertices.push_back( Vertex{ pos, normal, tc, color_tc, deform_scaling, time, color_weight } );
	};

	// Create triangles for flat shading
	for( auto r = 0; r < rings; r += 1 )
	{
		for( auto s = 0; s < segments; s += 1 )
		{
			auto provoking = ivec2(r, s);
			add_vert( r, s, provoking );
			add_vert( r, s + 1, provoking );
			add_vert( r + 1, s + 1, provoking );

			add_vert( r, s, provoking );
			add_vert( r + 1, s, provoking );
			add_vert( r + 1, s + 1, provoking );
		}
	}
}

} // namespace

void Landscape::setup()
{
	auto shader = loadShader( "landscape.vs", "landscape.fs" );

	std::vector<Vertex> vertices;

	auto normal = vec3( 0, 1, 0 );

	auto inner = 0.28f;
	auto outer = 3.0f;
	auto repeats = std::vector<float>{ 2, 4, 6, 8, 10, 12, 10, 8, 10, 8 };
	auto frames =		std::vector<int> { 1, 1, 1, 1,	1,	1,	1, 2,	 3,	4 };

	for( auto i = 0; i < repeats.size(); i += 1 )
	{
		auto copies = repeats.at( i );
		auto t = (i + 0.0f) / repeats.size();
		auto t2 = (i + 1.0f) / repeats.size();
		auto r1 = mix( inner, outer, t );
		auto r2 = mix( inner, outer, t2 );
		addRing( vertices, vec3( 0, -4, 0 ), normal, r1, r2, i * 2 + 1.0f, 1, copies, 0.0f );
	}

	addRing( vertices, vec3( 0, -4, 0 ), normal, outer, outer + 2.0f, repeats.size() * 2, 64 - (repeats.size() * 2), 1, 1.0f );

	// Deform stuff
	/*
	auto min_distance = inner;
	auto max_distance = outer;
	for( auto &v : vertices ) {
		auto distance = lmap<float>( length( v.position ), min_distance, max_distance, 0.0f, 1.0f );
		// pos is on a radial axis, rotate it 90 degrees to bend along length
		auto axis = glm::rotate( glm::angleAxis( (float)Tau / 4.0f, vec3(0, 1, 0) ), normalize(v.position) );
		auto theta = mix( 0.0f, -(float)Tau / 18.0f, distance );
		auto xf = glm::rotate( theta, axis );
		v.normal = vec3(xf * vec4(v.normal, 0.0f));
//		v.deform_scaling = mix( 0.0f, 4.0f, distance );
		v.position = vec3(xf * vec4(v.position, 1.0f));
//		v.color_weight = 0.0f; // mix( 0.0f, 1.0f, distance * distance );
//		if( distance >= 1.0f ) {
//			v.color_weight = 1.0f;
//		}
	}
//	*/

	// Plug center
	auto h = vec3( inner, 0, inner );
	auto c = vec3( 0, -4.001, 0 );
	addQuad( vertices, c + (h * vec3(-1, 0, -1)), c + (h * vec3(1, 0, -1)), c + (h * vec3(1, 0, 1)), c + (h * vec3(-1, 0, 1)), 0.0f, Rectf( 0, 1, 1, 0 ) );

	auto vbo = gl::Vbo::create( GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW );
	auto mesh = gl::VboMesh::create( vertices.size(), GL_TRIANGLES, {{ kVertexLayout, vbo }} );
	batch = gl::Batch::create( mesh, shader, kVertexMapping );
}

void Landscape::setTextureUnits( uint8_t iClearUnit, uint8_t iBlurredUnit )
{
	auto &shader = batch->getGlslProg();
	shader->uniform( "uClearTexture", iClearUnit );
	shader->uniform( "uBlurredTexture", iBlurredUnit );
}

void Landscape::draw( float iFrameOffset )
{
	auto &shader = batch->getGlslProg();
	shader->uniform( "uCurrentFrame", iFrameOffset );

	batch->draw();
}
