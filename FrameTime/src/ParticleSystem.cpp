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
		auto xy = vec2(250, 160) + glm::diskRand( 400.0f );
		auto z = 0.0f;

		v.position = vec3( xy, z );
		v.previousPosition = v.position;
		v.size = glm::linearRand( 1.0f, 10.0f );
		v.uv = vec2( 0.5f );

		particles.push_back( Particle( v ) );
	}

	auto vbo = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW );
	auto layout = geom::BufferLayout();
	layout.append( geom::Attrib::POSITION, 3, sizeof(Vertex), offsetof(Vertex, position) );
	layout.append( geom::Attrib::CUSTOM_0, 3, sizeof(Vertex), offsetof(Vertex, previousPosition) );
	layout.append( geom::Attrib::CUSTOM_1, 1, sizeof(Vertex), offsetof(Vertex, size) );
	layout.append( geom::Attrib::TEX_COORD_0, 2, sizeof(Vertex), offsetof(Vertex, uv) );

	auto mesh = gl::VboMesh::create( vertices.size(), GL_POINTS, {{layout, vbo}} );
	try {
		auto format = gl::GlslProg::Format()
										.vertex( app::loadAsset("particle.vs") )
										.fragment( app::loadAsset("particle.fs") );
		auto shader = gl::GlslProg::create( format );
		batch = gl::Batch::create( mesh, shader, {{geom::Attrib::CUSTOM_0, "PreviousPosition"}, {geom::Attrib::CUSTOM_1, "Size"}} );
	}
	catch( ci::Exception &exc ) {
		app::console() << "Error loading shader: " << exc.what() << endl;
	}

}

void ParticleSystem::step()
{
	for( auto &p : particles )
	{

	}
}

void ParticleSystem::draw() const
{
	gl::ScopedAlphaBlend blend( true );
	if( batch ) {
		batch->draw();
	}
}
