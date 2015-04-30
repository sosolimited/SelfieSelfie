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
using namespace cinder::signals;
using namespace std;

Target::Target( const ci::vec2 &iPosition, float iWeight )
: position( vec3( iPosition, 0.0f ) ),
	weight( iWeight )
{}

Particle::Particle( const Vertex &v )
: vertex( v ),
	homeliness( glm::linearRand( 0.1f, 0.9f ) )
{}

void ParticleSystem::registerTouchEvents( const ci::app::WindowRef &iWindow )
{
	signalConnections.push_back(
		make_shared<ScopedConnection>( iWindow->getSignalTouchesBegan().connect( [this] (app::TouchEvent &e) { touchesBegan(e); } ) )
	);
	signalConnections.push_back(
		make_shared<ScopedConnection>( iWindow->getSignalTouchesMoved().connect( [this] (app::TouchEvent &e) { touchesMoved(e); } ) )
	);
	signalConnections.push_back(
		make_shared<ScopedConnection>( iWindow->getSignalTouchesEnded().connect( [this] (app::TouchEvent &e) { touchesEnded(e); } ) )
	);
}

void ParticleSystem::setup()
{
	// A healthy number of vertices.
	std::vector<Vertex> vertices;
	vertices.resize( 270 * 180 );

	for( auto &v : vertices )
	{
		auto xy = vec2(250, 160) + glm::diskRand( 50.0f );
		auto z = 0.0f;

		v.position = vec3( xy, z );
		v.velocity = glm::sphericalRand( 120.0f );
		v.size = glm::linearRand( 3.0f, 9.0f );
		v.uv = vec2( 0.5f );

		particles.push_back( Particle( v ) );
	}

	// Upload our vertices to the GPU.
	vertexBuffer = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW );
	// Describe our vertex data layout.
	auto layout = geom::BufferLayout();
	layout.append( geom::Attrib::POSITION, 3, sizeof(Vertex), offsetof(Vertex, position) );
	layout.append( geom::Attrib::CUSTOM_0, 3, sizeof(Vertex), offsetof(Vertex, velocity) );
	layout.append( geom::Attrib::CUSTOM_1, 1, sizeof(Vertex), offsetof(Vertex, size) );
	layout.append( geom::Attrib::TEX_COORD_0, 2, sizeof(Vertex), offsetof(Vertex, uv) );

	// Create a VboMesh that correlates our layout description to our data.
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
	// Copy touch target map into a vector.
	// This saves us at least 1ms when 5 touches are down.
	vector<Target> touch_targets;
	for( auto &t : touchTargets ) {
		touch_targets.push_back( t.second );
	}

	// Update our positions on the CPU.
	auto dt2 = fixedStep * fixedStep;
	for( auto &p : particles )
	{
		auto &v = p.vertex;

		auto acc = vec3( 0 );
		for( auto &target : touch_targets ) {
			acc += (target.position - v.position) * target.weight * 32.0f * p.homeliness;
		}
		for( auto &target : p.targets ) {
			acc += (target.position - v.position) * target.weight * 32.0f * p.homeliness;
		}

		v.velocity -= v.velocity * friction;
		v.velocity += acc * dt2;
		v.position += v.velocity * fixedStep;
	}

	// Send new positions to the GPU.
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

	gl::ScopedColor c( Color( CM_HSV, 0.16f, 1.0f, 1.0f ) );
	for( auto &touch : touchTargets ) {
		auto pos = vec2( touch.second.position.x, touch.second.position.y );
		gl::drawStrokedCircle( pos, 40.0f );
	}
}

void ParticleSystem::touchesBegan( ci::app::TouchEvent &iEvent )
{
	for( auto &touch : iEvent.getTouches() ) {
		touchTargets[touch.getId()] = Target( touch.getPos(), 0.5f );
	}
}

void ParticleSystem::touchesMoved( ci::app::TouchEvent &iEvent )
{
	for( auto &touch : iEvent.getTouches() ) {
		touchTargets[touch.getId()].position = vec3( touch.getPos(), 0.0f );
	}
}

void ParticleSystem::touchesEnded( ci::app::TouchEvent &iEvent )
{
	for( auto &touch : iEvent.getTouches() ) {
		touchTargets.erase( touch.getId() );
	}
}
