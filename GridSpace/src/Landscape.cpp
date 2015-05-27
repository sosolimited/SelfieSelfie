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
	const auto inner_radius = 3.0f;
	const auto outer_radius = 50.0f;

	auto total_things = 64;
	auto inner_things = 16;
	auto outer_things = total_things - inner_things;
	auto perp = glm::rotate<float>( ray, Tau * 0.25f, normal );

	// Add full frames
	for( auto i = 0; i < inner_things; i += 1 ) {
		auto d1 = (i + 0.0f) / total_things;
		auto d2 = (i + 1.0f) / total_things;
		auto r1 = mix( inner_radius, outer_radius, d1 );
		auto r2 = mix( inner_radius, outer_radius, d2 );
		auto time = (i + 0.0f) / inner_things;
		auto w1 = mix( inner_width, outer_width, d1 );
		auto w2 = mix( inner_width, outer_width, d2 );

		auto p1 = perp * w1 / 2.0f;
		auto p2 = perp * w2 / 2.0f;

		auto a = base + ray * r2 - p2;
		auto b = base + ray * r2 + p2;
		auto c = base + ray * r1 + p1;
		auto d = base + ray * r1 - p1;

		addQuad(vertices, a, b, c, d, time, Rectf(0, 0, 1, 1));
	}

	// Progressively deteriorating frames
}

} // namespace

void Landscape::setup()
{
	auto shader = loadShader( "landscape.vs", "landscape.fs" );

	std::vector<Vertex> vertices;

	auto dims = ivec2( 15 );
	auto offset = - vec2(dims - 1) * vec2(0.5f);
	auto max_dist = length(vec2(dims)) * 0.5f;

	auto normal = vec3( 0, 1, 0 );
	auto rotation = glm::rotate<float>( Tau * 0.1875f, normal );
	auto ray = vec3( 0, 0, 1 );

	for( auto i = 0; i < 12; i += 1 ) {
		addStrip( vertices, vec3( 0, -2, 0 ), ray, normal, 1.0f, 16.0f );
		ray = vec3(rotation * vec4(ray, 0));
	}
/*
	for( auto y = 0; y < dims.y; y += 1 ) {
		for( auto x = 0; x < dims.x; x += 1 ) {
			auto pos = offset + vec2(x ,y);
			auto d_norm = floor( glm::length( pos ) ) / max_dist;
			auto time_offset = mix( 0.0f, (float)dims.x / 64.0f, d_norm );

			addRectangle( vertices, vec3( pos.x, -8.0f, pos.y ), 1.0f, 1.0f, time_offset );
		}
	}
*/

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
