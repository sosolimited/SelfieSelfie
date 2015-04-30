//
//  ParticleSystem.h
//
//  Created by Soso Limited on 4/30/15.
//
//

#pragma once

namespace soso {

///
/// Vertex data passed to the GPU.
///
struct Vertex
{
	ci::vec3	position;
	ci::vec3	velocity;
	float			size;
	ci::vec2	uv;
};

///
/// A weighted position that particles move toward.
///
struct Target
{
	Target() = default;
	Target( const ci::vec2 &iPosition, float iWeight );

	ci::vec3 position;
	float		 weight = 0.5f;
};

///
/// A Particle containing data for GPU rendering and CPU simulation.
///
struct Particle
{
	Particle() = default;
	Particle( const Vertex &v );

	Vertex								vertex;
	float									homeliness = 1.0f;
	float									wanderliness = 1.0f;
	std::array<Target, 1>	targets;
};

///
/// Our particle updating system.
///
class ParticleSystem
{
public:
	/// Connect window touch event signals to our handlers.
	void registerTouchEvents( const ci::app::WindowRef &iWindow );

	/// Perform OpenGl initialization.
	void setup();
	/// Advance simulation and buffer data to GPU.
	void step();
	/// Draw to screen.
	void draw() const;

private:
	std::vector<Target>		targets;
	std::vector<Particle>	particles;
	float									fixedStep = 1.0f / 60.0f;
	float									friction = 0.05f;
	ci::gl::VboRef				vertexBuffer;
	ci::gl::BatchRef			batch;

	std::map<uint32_t, Target> touchTargets;

	void touchesBegan( ci::app::TouchEvent &iEvent );
	void touchesMoved( ci::app::TouchEvent &iEvent );
	void touchesEnded( ci::app::TouchEvent &iEvent );
	using ScopedConnectionRef = std::shared_ptr<ci::signals::ScopedConnection>;
	std::vector<ScopedConnectionRef> signalConnections;
};

} // namespace soso
