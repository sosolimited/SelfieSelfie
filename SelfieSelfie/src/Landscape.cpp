//
//  Landscape.cpp
//
//  Created by Soso Limited on 5/26/15.
//
//

#include "Landscape.h"

#include "cinder/Log.h"
#include "Constants.h"
#include "LandscapeGeometry.h"
#include "cinder/Json.h"
#include "cinder/Xml.h"

#include "cinder/gl/GlslProg.h"
#include "cinder/app/App.h"
#include "cinder/Easing.h"

using namespace soso;
using namespace cinder;

namespace {

struct alignas(16) Vertex {
  /// Position
	vec3	position;
  float frame_offset;
  /// Color
	vec2	color_tex_coord;		 // smooth across sections (bars in a section have shared edge values)
	vec2	flat_tex_coord;      // shared within bands
  float color_weight;
  /// Deformation
  vec3	normal;
  float deform_frame_offset; // shared across seams
  float deform_weight;
};

const auto kVertexLayout = ([] {
		auto layout = geom::BufferLayout();
		layout.append( geom::Attrib::POSITION, 3, sizeof(Vertex), offsetof(Vertex, position) );
		layout.append( geom::Attrib::NORMAL, 3, sizeof(Vertex), offsetof(Vertex, normal) );
		layout.append( geom::Attrib::TEX_COORD_0, 2, sizeof(Vertex), offsetof(Vertex, color_tex_coord) );
		layout.append( geom::Attrib::TEX_COORD_1, 2, sizeof(Vertex), offsetof(Vertex, flat_tex_coord) );
		layout.append( geom::Attrib::CUSTOM_0, 1, sizeof(Vertex), offsetof(Vertex, deform_weight) );
		layout.append( geom::Attrib::CUSTOM_1, 1, sizeof(Vertex), offsetof(Vertex, frame_offset) );
		layout.append( geom::Attrib::CUSTOM_2, 1, sizeof(Vertex), offsetof(Vertex, color_weight) );
    layout.append( geom::Attrib::CUSTOM_3, 1, sizeof(Vertex), offsetof(Vertex, deform_frame_offset) );
		return layout;
	} ());

const auto kVertexMapping = ([] {
		return gl::Batch::AttributeMapping{ { geom::Attrib::CUSTOM_0, "DeformScaling" },
                                        { geom::Attrib::CUSTOM_1, "FrameOffset" },
                                        { geom::Attrib::CUSTOM_2, "ColorWeight" },
                                        { geom::Attrib::CUSTOM_3, "DeformFrameOffset" } };
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

/// Add a ring of geometry containing a given number of time bands (slitscanning effect) and repeats around the donut.
void addRing( std::vector<Vertex> &vertices, const Bar &bar, int next_bar_time, float deform_start_time, const ci::vec2 &center_offset )
{
	const auto segments = 64;
	const auto texture_insets = vec2( 0.05f, 0.0875f );

	// Generate cartesian position.
	const auto calc_pos = [=] (int r, int s) {
		auto pos = mix( bar.begin, bar.end, (float)r ) + center_offset;
		auto t = (float) s / segments;

		auto rotation = glm::rotate<float>( t * Tau, vec3(0, 1, 0) );

		return vec3( rotation * vec4( pos, 0.0f, 1.0f ) );
	};

	const auto calc_normal = [=] (int r, int s) {
		auto normal = normalize( vec3(mix(bar.normal_begin, bar.normal_end, (float)r), 0.0f) );
		auto t = (float) s / segments;
		auto rotation = glm::rotate<float>( t * Tau, vec3(0, 1, 0) );
		return vec3( rotation * vec4(normal, 0.0f) );
	};

	// Generate texture coordinate mirrored at halfway point.
	const auto calc_tc = [=] (int r, int s) {
		auto t = (float) s / segments;
		if( t < 1 ) {
			t = glm::fract( t * bar.repeats );
		}
		// mirror copies
		t = std::abs( t - 0.5f ) * 2.0f;
		auto tc = vec2(0);
		// Repeat t with mirroring
		// insetting the texture coordinates minimizes edge color flashing.
		tc.y = mix( texture_insets.y, 1.0f - texture_insets.y, t );
		auto x = mix( bar.texture_begin, bar.texture_end, (float) r );
		tc.x = mix( 1.0f - texture_insets.x, texture_insets.x, x );
		return tc;
	};

	// Add a vertex to texture (color_tc parameter allows us to do flat shading in ES2)
	const auto add_vert = [=,&vertices] (int r, int s, const ivec2 &provoking) {
		auto pos = calc_pos(r, s);
		auto color_tc = calc_tc(r, s);
    auto flat_tc = calc_tc( provoking.x, provoking.y );
		auto normal = calc_normal(r, s);

		auto deform_t = lmap( glm::clamp<float>( bar.time, deform_start_time, 144.0f ), deform_start_time, 144.0f, 0.0f, 1.0f );
    auto deform_scaling = easeInOutQuad( deform_t );
		auto color_weight = easeInOutCubic( deform_t );
		if( color_weight > 0.8f ) { color_weight = 1.0f; }
		auto deform_frame = (float)mix( bar.time, next_bar_time, (float)r );
		if (deform_scaling > 1.0f) {
			CI_LOG_W("Deform scaling out of bounds: " << deform_scaling );
		}

    vertices.emplace_back( Vertex { pos, (float)bar.time,
                                    color_tc, flat_tc, color_weight,
                                    normal, deform_frame, deform_scaling } );
	};

	// Create triangles for flat shading
	const auto r = 0;
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

} // namespace

void Landscape::setup()
{
  CI_LOG_I("Loading landscape shader");
	auto shader = loadShader( "landscape.vs", "landscape.fs" );

	std::vector<Vertex> vertices;

	auto offset = vec2( 0.05f, 0.0f );

  CI_LOG_I("Loading shape profile.");
	auto xml = XmlTree( app::loadAsset("profile.xml") );
	auto deform_start_time = xml.getChild("shape").getChild("deform_start").getValue<float>();
	auto &bars = xml.getChild("shape").getChild("bars").getChildren();
	auto iter = bars.begin();
	Bar bar(**iter);
	while( iter != bars.end() ) {
		++iter;
		if( iter != bars.end() ) {
			Bar next(**iter);
			addRing( vertices, bar, next.time, deform_start_time, offset );
			bar = next;
		}
		else {
			addRing( vertices, bar, bar.time, deform_start_time, offset );
		}
	}

	auto center = vec3( 0, -4.0f, 0 );

	// Flip up
	auto xf = glm::rotate<float>( Tau * 0.25f, vec3( 0, 0, 1 ) ) * glm::translate( center ) * glm::scale( vec3( 4.0f ) );
	for( auto &v : vertices ) {
		v.position = vec3( xf * vec4(v.position, 1.0f) );
		v.normal = vec3( xf * vec4(v.normal, 0.0f) );
	}

//  /*
	// Mirror (maybe just draw twice)
	auto copy = vertices;
	auto mirror = glm::translate( vec3( -16.0f, 0.0f, 0.0f ) ) * glm::mat4( glm::angleAxis<float>( Tau * 0.5f, vec3( 0, 1, 0 ) ) );

	for( auto &v : copy ) {
		v.position = vec3( mirror * vec4(v.position, 1.0f) );
		v.normal = vec3( mirror * vec4(v.normal, 0.0f) );
	}

	vertices.insert( vertices.end(), copy.begin(), copy.end() );
//  */

  CI_LOG_I("Uploading shape to GPU.");
	auto vbo = gl::Vbo::create( GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW );
	auto mesh = gl::VboMesh::create( (int)vertices.size(), GL_TRIANGLES, {{ kVertexLayout, vbo }} );
	batch = gl::Batch::create( mesh, shader, kVertexMapping );
}

void Landscape::setTextureUnits( uint8_t iClearUnit, uint8_t iBlurredUnit )
{
	auto &shader = batch->getGlslProg();
	shader->uniform( "uClearTexture", iClearUnit );
	shader->uniform( "uBlurredTexture", iBlurredUnit );
}

void Landscape::setGridSize( const ci::vec2 &iSize )
{
  auto &shader = batch->getGlslProg();
  shader->uniform( "uGridSize", iSize );
}

void Landscape::draw( float iFrameOffset )
{
	auto &shader = batch->getGlslProg();
	shader->uniform( "uCurrentFrame", iFrameOffset );

	batch->draw();
}
