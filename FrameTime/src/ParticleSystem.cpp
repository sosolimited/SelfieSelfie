//
//  ParticleSystem.cpp
//
//  Created by Soso Limited on 4/30/15.
//
//

#include "ParticleSystem.h"
#include "glm/gtc/random.hpp"

using namespace soso;
using namespace cinder;
using namespace std;

Particle::Particle( const Vertex &v )
: vertex( v )
{}

void ParticleSystem::setup()
{
	std::array<Vertex, 1000> vertices;

	for( auto &v : vertices )
	{
		auto xy = vec2(250, 160) + glm::diskRand( 50.0f );
		auto z = 0.0f;

		v.position = vec3( xy, z );
		v.velocity = glm::sphericalRand( 120.0f );
		v.size = glm::linearRand( 5.0f, 30.0f );
		v.uv = vec2( 0.5f );

		particles.push_back( Particle( v ) );
	}

	/// Upload our vertices to the GPU.
	vertexBuffer = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW );
	/// Describe our vertex data layout.
	auto layout = geom::BufferLayout();
	layout.append( geom::Attrib::POSITION, 3, sizeof(Vertex), offsetof(Vertex, position) );
	layout.append( geom::Attrib::CUSTOM_0, 3, sizeof(Vertex), offsetof(Vertex, velocity) );
	layout.append( geom::Attrib::CUSTOM_1, 1, sizeof(Vertex), offsetof(Vertex, size) );
	layout.append( geom::Attrib::TEX_COORD_0, 2, sizeof(Vertex), offsetof(Vertex, uv) );

	/// Create a VboMesh that correlates our layout description to our data.
	auto mesh = gl::VboMesh::create( vertices.size(), GL_POINTS, {{layout, vertexBuffer}} );

	try {
		auto shader_format = gl::GlslProg::Format()
													.vertex( app::loadAsset("particle.vs") )
													.fragment( app::loadAsset("particle.fs") );
		auto shader = gl::GlslProg::create( shader_format );
		batch = gl::Batch::create( mesh, shader, {{geom::Attrib::CUSTOM_0, "Velocity"}, {geom::Attrib::CUSTOM_1, "Size"}} );
	}
	catch( ci::Exception &exc ) {
		app::console() << "Error loading shader: " << exc.what() << endl;
	}

}

void ParticleSystem::step()
{
	for( auto &p : particles )
	{
		auto &v = p.vertex;
		v.position += v.velocity * fixedStep;
		v.velocity *= 0.99f;
	}

	auto *gpu_vertex = static_cast<Vertex*>( vertexBuffer->mapWriteOnly( true ) );
	for( auto &p : particles ) {
		*gpu_vertex = p.vertex;
		++gpu_vertex;
	}
	vertexBuffer->unmap();
}

void ParticleSystem::draw() const
{
	gl::ScopedAlphaBlend blend( true );
	if( batch ) {
		batch->draw();
	}
}
