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
};

const auto kVertexLayout = ([] {
		auto layout = geom::BufferLayout();
		layout.append( geom::Attrib::POSITION, 3, sizeof(Vertex), offsetof(Vertex, position) );
		layout.append( geom::Attrib::NORMAL, 3, sizeof(Vertex), offsetof(Vertex, normal) );
		layout.append( geom::Attrib::TEX_COORD_0, 2, sizeof(Vertex), offsetof(Vertex, tex_coord) );
		layout.append( geom::Attrib::TEX_COORD_1, 2, sizeof(Vertex), offsetof(Vertex, color_tex_coord) );
		layout.append( geom::Attrib::CUSTOM_0, 1, sizeof(Vertex), offsetof(Vertex, deform_scaling) );
		layout.append( geom::Attrib::CUSTOM_1, 1, sizeof(Vertex), offsetof(Vertex, frame_offset) );
		return layout;
	} ());

const auto kVertexMapping = ([] {
		return gl::Batch::AttributeMapping{ { geom::Attrib::CUSTOM_0, "DeformScaling" }, { geom::Attrib::CUSTOM_1, "FrameOffset" } };
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
		Vertex{ a, normal, slice.getUpperLeft(), slice.getUpperLeft(), deform_scaling, time },
		Vertex{ b, normal, slice.getUpperRight(), slice.getUpperRight(), deform_scaling, time },
		Vertex{ c, normal, slice.getLowerRight(), slice.getLowerRight(), deform_scaling, time },

		Vertex{ a, normal, slice.getUpperLeft(), slice.getUpperLeft(), deform_scaling, time },
		Vertex{ c, normal, slice.getLowerRight(), slice.getLowerRight(), deform_scaling, time },
		Vertex{ d, normal, slice.getLowerLeft(), slice.getLowerLeft(), deform_scaling, time }
	} );

}

void addRectangle( std::vector<Vertex> &vertices, const ci::vec3 &iCenter, float iWidth, float iHeight, float iFrameOffset )
{
	auto right = vec3( iWidth / 2.0f, 0, 0 );
	auto left = - right;
	auto down = vec3( 0, 0, iHeight / 2.0f );
	auto up = - down;

	auto ul = iCenter + (left + up);
	auto ur = iCenter + (right + up);
	auto br = iCenter + (right + down);
	auto bl = iCenter + (left + down);
	addQuad( vertices, ul, ur, br, bl, iFrameOffset, Rectf( 0.25, 0.4, 0.75, 0.6 ) );
}

void addStrip( std::vector<Vertex> &vertices, const vec3 &base, const vec3 &ray, const vec3 &normal, float inner_width, float outer_width )
{
	const auto inner_radius = 1.0f;
	const auto outer_radius = 64.0f;

	auto total_things = 64;
	auto inner_things = 8;
	auto outer_things = total_things - inner_things;
	auto perp = glm::rotate<float>( ray, Tau * 0.25f, normal );

	// Add full frames
	for (auto i = 0; i < inner_things; i += 1)
	{
		auto d1 = (i + 0.0f) / total_things;
		auto d2 = (i + 1.0f) / total_things;
		auto r1 = mix( inner_radius, outer_radius, d1 );
		auto r2 = mix( inner_radius, outer_radius, d2 );
		auto time = (i + 0.0f) / total_things;
		auto w1 = mix( inner_width, outer_width, d1 );
		auto w2 = mix( inner_width, outer_width, d2 );

		auto p1 = perp * w1 / 2.0f;
		auto p2 = perp * w2 / 2.0f;

		auto a = base + ray * r2 - p2;
		auto b = base + ray * r2 + p2;
		auto c = base + ray * r1 + p1;
		auto d = base + ray * r1 - p1;

		addQuad(vertices, a, b, c, d, time, Rectf(0, 1, 1, 0));
	}

	for (auto i = inner_things; i < total_things; i += 1)
	{
		auto outer_d1 = (i + 0.0f) / total_things;
		auto outer_d2 = (i + 1.0f) / total_things;
		auto slices = 48;

		for( auto s = 0; s < slices; s += 1 )
		{
			auto t1 = (s + 0.0f) / slices;
			auto t2 = (s + 1.0f) / slices;
			auto d1 = mix( outer_d1, outer_d2, t1 );
			auto d2 = mix( outer_d1, outer_d2, t2 );

			auto r1 = mix( inner_radius, outer_radius, d1 );
			auto r2 = mix( inner_radius, outer_radius, d2 );

			auto time = (i + 0.0f) / total_things + (s + 0.0f) / slices;
			auto w1 = mix( inner_width, outer_width, d1 );
			auto w2 = mix( inner_width, outer_width, d2 );

			auto p1 = perp * w1 / 2.0f;
			auto p2 = perp * w2 / 2.0f;

			auto a = base + ray * r2 - p2;
			auto b = base + ray * r2 + p2;
			auto c = base + ray * r1 + p1;
			auto d = base + ray * r1 - p1;

			addQuad(vertices, a, b, c, d, time, Rectf(0, t2, 1, t1));
		}
	}

	// Progressively deteriorating frames
}

/// Add a ring of geometry containing a given number of time bands (slitscanning effect) and repeats around the donut.
void addRing( std::vector<Vertex> &vertices, const vec3 &center, const vec3 &normal, float inner_radius, float outer_radius, float time_offset, int time_bands, int repeats )
{
	auto rings = time_bands;
	auto segments = 32;

	// Generate cartesian position.
	auto calc_pos = [=] (int r, int s) {
		auto distance = lmap<float>( r, 0, rings, 0.0f, 1.0f );
		auto radius = mix( inner_radius, outer_radius, distance );
		auto t = (float) s / segments;
		auto x = cos( t * Tau ) * radius;
		auto y = sin( t * Tau ) * radius;
		return vec3( x, -4.0f, y );
	};

	// Generate texture coordinate mirrored at halfway point.
	auto calc_tc = [=] (int r, int s) {
		auto t = (float) s / segments;
		auto tc = vec2(0);
		// insetting the texture coordinates minimizes edge color flashing.
		tc.y = mix( 0.05f, 0.95f, std::abs( t - 0.5f ) * 2.0f );
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
		vertices.push_back( Vertex{ pos, normal, tc, color_tc, deform_scaling, time } );
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

	addRing( vertices, vec3( 0, -3, 0 ), normal, 1.0f, 2.0f, 0.0f, 1, 2 );
	addRing( vertices, vec3( 0, -3, 0 ), normal, 2.5f, 3.5f, 16.0f, 8, 4 );

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
