//
//  Landscape.cpp
//
//  Created by Soso Limited on 5/26/15.
//
//

#include "Landscape.h"

#include "cinder/Log.h"

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

	auto normal = vec3( 0, 1, 0 );
	auto deform_scaling = 0.0f;

	vertices.insert( vertices.end(), {
		Vertex{ ul, normal, vec2(0, 0), vec2(0, 0), deform_scaling, iFrameOffset },
		Vertex{ ur, normal, vec2(1, 0), vec2(1, 0), deform_scaling, iFrameOffset },
		Vertex{ br, normal, vec2(1, 1), vec2(1, 1), deform_scaling, iFrameOffset },

		Vertex{ ul, normal, vec2(0, 0), vec2(0, 0), deform_scaling, iFrameOffset },
		Vertex{ br, normal, vec2(1, 1), vec2(1, 1), deform_scaling, iFrameOffset },
		Vertex{ bl, normal, vec2(0, 1), vec2(0, 1), deform_scaling, iFrameOffset }
	} );
}

} // namespace

void Landscape::setup()
{
	auto shader = loadShader( "landscape.vs", "landscape.fs" );

	std::vector<Vertex> vertices;

	auto dims = ivec2( 15 );
	auto offset = - vec2(dims - 1) * vec2(0.5f);
	auto max_dist = length(vec2(dims)) * 0.5f;

	for( auto y = 0; y < dims.y; y += 1 ) {
		for( auto x = 0; x < dims.x; x += 1 ) {
			auto pos = offset + vec2(x ,y);
			auto d_norm = floor( glm::length( pos ) ) / max_dist;
			auto time_offset = mix( 0.0f, (float)dims.x / 64.0f, d_norm );

			addRectangle( vertices, vec3( pos.x, -8.0f, pos.y ), 1.0f, 1.0f, time_offset );
		}
	}

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
