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
	ci::vec3	previousPosition;
	float			size;
	ci::vec2	uv;
};

///
/// A weighted position that particles move toward.
///
struct Target
{
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
	float									homeliness = 0.2f;
	std::array<Target, 2>	targets;
};

///
/// Our particle updating system.
///
class ParticleSystem
{
public:
	void setup();
	void step();
	void draw() const;
private:
	std::vector<Particle>	particles;
	float									fixedStep = 1.0f / 60.0f;
	ci::gl::BatchRef			batch;
};

} // namespace soso
