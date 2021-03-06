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
	homeliness( glm::linearRand( 0.3f, 0.9f ) ),
	wanderliness( glm::linearRand( 0.3f, 0.9f ) )
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
	auto shader = ([] () {
		try {
			auto shader_format = gl::GlslProg::Format()
				.vertex( app::loadAsset("particle.vs") )
				.fragment( app::loadAsset("particle.fs") );
			return gl::GlslProg::create( shader_format );
		}
		catch( ci::Exception &exc ) {
			app::console() << "Error loading shader: " << exc.what() << endl;
		}
		return gl::GlslProgRef();
	}());
	shader->uniform( "uVideo", 0 );

	// A healthy number of vertices.
	std::vector<Vertex> vertices;
	const auto w = 1920 / 7;
	const auto h = 1080 / 7;
  const auto center = vec2(app::getWindowCenter());
	vertices.reserve( w * h );
	particles.reserve( w * h );

	for( auto x = 0; x < w; x += 1 )
	{
		for( auto y = 0; y < h; y += 1 )
		{
			auto xy = center + glm::diskRand( 50.0f );
			auto z = 0.0f;

			auto v = Vertex();
			v.position = vec3( xy, z );
			v.velocity = glm::sphericalRand( 120.0f );
			v.size = glm::linearRand( 3.0f, 6.0f );
			v.uv = vec2( x / (w - 1.0f), 1.0f - y / (h - 1.0f) );

			auto p = Particle( v );
			p.targets[0].position = vec3( app::getWindowWidth(), app::getWindowHeight(), 0 ) * vec3( x, y, 0 ) / vec3( w, h, 1 );
			p.targets[0].weight = 0.9f;

			vertices.push_back( v );
			particles.push_back( p );
		}
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
	auto mesh = gl::VboMesh::create( static_cast<uint32_t>(vertices.size()), GL_POINTS, {{layout, vertexBuffer}} );
	batch = gl::Batch::create( mesh, shader, {{geom::Attrib::CUSTOM_0, "Velocity"}, {geom::Attrib::CUSTOM_1, "Size"}} );
}

void ParticleSystem::step()
{
	// Copy touch target map into a vector.
	// This saves us at least 1ms when 5 touches are down.
	vector<Target> touch_targets;
	for( auto &t : touchTargets ) {
		touch_targets.push_back( t.second );
	}

	auto max_distance = 200.0f * 200.0f;
	// Update our positions on the CPU.
	for( auto &p : particles )
	{
		auto &v = p.vertex;

		auto acc = vec3( 0 );
		for( auto &target : touch_targets ) {
			auto delta = (target.position - v.position);
			auto d2 = glm::dot( delta, delta );
			d2 = glm::clamp<float>( d2, 1.0f, max_distance ) / max_distance;
//			delta *= d2; // make farther away things move more
			delta *= mix( 2.0f, 0.01f, d2 ); // closer = faster
			acc += delta * target.weight * p.wanderliness;
		}
		for( auto &target : p.targets ) {
			acc += (target.position - v.position) * target.weight * p.homeliness;
		}

		v.velocity -= v.velocity * friction;
		v.velocity += acc;
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
}

void ParticleSystem::touchesBegan( ci::app::TouchEvent &iEvent )
{
	for( auto &touch : iEvent.getTouches() ) {
		touchTargets[touch.getId()] = Target( touch.getPos(), 1.0f );
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
