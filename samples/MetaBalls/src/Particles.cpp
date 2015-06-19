//
//  Particles.cpp
//  MetaBalls2
//
//  Created by Ryan Bartley on 7/19/14.
//
//

#include "Particles.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Utilities.h"

#include "MetaBallsController.h"

const int Particles::sParticleCount = 64;

using namespace ci;
using namespace ci::app;
using namespace cl;
using namespace ci::gl;
using namespace std;

ParticlesRef Particles::create()
{
	return ParticlesRef( new Particles() );
}

Particles::Particles()
: mShouldReset( false ), mDebugDraw( false )
{
	setupGl();
	setupCl();
}

void Particles::setupGl()
{
	std::array<vec4, sParticleCount> cpuPositions, cpuVelocities, cpuRandoms;
	std::array<float, sParticleCount>  cpuLifetimes;
	
	srand(time(NULL));
	
	mShouldReset = 0;
	
	for(int i = 0; i < sParticleCount; i++) {
		cpuLifetimes[i] = 999;
		cpuPositions[i] = vec4( 0.0f );
		cpuVelocities[i] = vec4( 0.0f );
		
		float rx = ((float)rand() / RAND_MAX) * 2 - 1;
		float ry = ((float)rand() / RAND_MAX) * 2 + 0.5;
		float rz = ((float)rand() / RAND_MAX) * 2 - 1;
		float rm = (float)rand() / RAND_MAX;
		
		vec3 rand = normalize( vec3(rx, ry, rz) ) * (rm * 2);
		
		cpuRandoms[i] = vec4( rand, 1.0f );
	}
	
	mGlPositions = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec4) * sParticleCount, cpuPositions.data(), GL_DYNAMIC_COPY );
	mGlVelocities = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec4) * sParticleCount, cpuVelocities.data(), GL_DYNAMIC_COPY );
	mGlLifetimes = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(float) * sParticleCount, cpuLifetimes.data(), GL_DYNAMIC_COPY );
	mGlRandoms = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec4) * sParticleCount, cpuRandoms.data(), GL_DYNAMIC_COPY );
	
	mGlVao = gl::Vao::create();
	{
		gl::ScopedVao scopeVao(mGlVao);
		
		{
			gl::ScopedBuffer scopeBuffer( mGlPositions );
			gl::enableVertexAttribArray( 0 );
			gl::vertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
		}
		{
			gl::ScopedBuffer scopeBuffer( mGlVelocities );
			gl::enableVertexAttribArray( 1 );
			gl::vertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
		}
	}
	
	mGlProgram = gl::GlslProg::create( gl::GlslProg::Format()
									  .vertex( loadAsset( "basic.vert" ) )
									  .fragment( loadAsset( "basic.frag" ) )
									  .attribLocation( "ciPosition", 0 )
									  .attribLocation( "ciColor", 1 ) );
}

void Particles::setupCl()
{
	auto clCtx = MetaBallsController::get()->getContext();
	mClPositions = cl::BufferGL( clCtx, CL_MEM_READ_WRITE, mGlPositions->getId() );
	mClVelocities = cl::BufferGL( clCtx, CL_MEM_READ_WRITE, mGlVelocities->getId() );
	mClLifetimes = cl::BufferGL( clCtx, CL_MEM_READ_WRITE, mGlLifetimes->getId() );
	mClRandoms = cl::BufferGL( clCtx, CL_MEM_READ_WRITE, mGlRandoms->getId() );
	
	mClProgram = cl::Program( clCtx, loadString( loadAsset( "kernels/particles.cl" ) ), true );
	mClUpdateKernel = cl::Kernel( mClProgram, "particle_update" );
	
	float max_life = 60.0;
	float min_velocity = 0.5;
	
	mClUpdateKernel.setArg( 0, mClPositions );
	mClUpdateKernel.setArg( 1, mClVelocities );
	mClUpdateKernel.setArg( 2, mClLifetimes );
	mClUpdateKernel.setArg( 3, mClRandoms );
	mClUpdateKernel.setArg( 4, sizeof(float), &max_life );
	mClUpdateKernel.setArg( 5, sizeof(float), &min_velocity );
	mClUpdateKernel.setArg( 9, sizeof(cl_int), &sParticleCount );
}

std::vector<cl::Memory> Particles::getInterop()
{
	std::vector<cl::Memory> ret = {
		mClPositions,
		mClVelocities,
		mClRandoms,
		mClLifetimes
	};
	return ret;
}

void Particles::update()
{
	auto clCQ = MetaBallsController::get()->getCommandQueue();
	int random = rand();
	float time = 1.0f / 60.0f;
	
	mClUpdateKernel.setArg( 6, sizeof(float), &time );
	mClUpdateKernel.setArg( 7, sizeof(bool), &mShouldReset );
	mClUpdateKernel.setArg( 8, sizeof(int), &random );
	
	mShouldReset = false;
	
    // Queue the kernel up for execution across the array
	clCQ.enqueueNDRangeKernel( mClUpdateKernel,
							cl::NullRange,
							cl::NDRange( sParticleCount ),
							cl::NullRange );
}

void Particles::render()
{
	ScopedVao scopeVao( mGlVao );
	ScopedGlslProg scopeGlsl( mGlProgram );
	ScopedState scopeState( GL_VERTEX_PROGRAM_POINT_SIZE, true );
	
	gl::setDefaultShaderVars();
	gl::drawArrays( GL_POINTS, 0, sParticleCount );
}