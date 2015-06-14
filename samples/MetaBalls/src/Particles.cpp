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

using namespace ci;
using namespace ci::app;
using namespace cl;
using namespace ci::gl;
using namespace std;

ParticlesRef Particles::create( const cl::Context &context, const cl::CommandQueue &commandQueue )
{
	return ParticlesRef( new Particles( context, commandQueue ) );
}

Particles::Particles( const cl::Context &context, const cl::CommandQueue &commandQueue )
: mCommandQueue( commandQueue )
{
	std::vector<vec4>	cpuPositions(particle_count),
						cpuVelocities(particle_count),
						cpuRandoms(particle_count);
	std::vector<float>  cpuLifetimes(particle_count);
	
	srand(time(NULL));
	
	mShouldReset = 0;
	
	for(int i = 0; i < particle_count; i++) {
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
	
	mGlPositions = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec4) * particle_count, cpuPositions.data(), GL_DYNAMIC_COPY );
	mGlVelocities = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec4) * particle_count, cpuVelocities.data(), GL_DYNAMIC_COPY );
	mGlLifetimes = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(float) * particle_count, cpuLifetimes.data(), GL_DYNAMIC_COPY );
	mGlRandoms = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vec4) * particle_count, cpuRandoms.data(), GL_DYNAMIC_COPY );
	
	mGlVao = gl::Vao::create();
	{
		gl::ScopedVao scopeVao(mGlVao);
		
		{
			gl::ScopedBuffer scopeBuffer( mGlPositions );
			gl::enableVertexAttribArray( 0 );
			gl::vertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
		}
		{
			gl::ScopedBuffer scopeBuffer( mGlRandoms );
			gl::enableVertexAttribArray( 1 );
			gl::vertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
		}
	}
	
	
	mGlProgram = gl::GlslProg::create( gl::GlslProg::Format()
									  .vertex( loadAsset( "basic.vert" ) )
									  .fragment( loadAsset( "basic.frag" ) )
									  .attribLocation( "ciPosition", 0 )
									  .attribLocation( "ciColor", 1 ) );
	
	mClPositions = cl::BufferGL( context, CL_MEM_READ_WRITE, mGlPositions->getId() );
	mClVelocities = cl::BufferGL( context, CL_MEM_READ_WRITE, mGlVelocities->getId() );
	mClLifetimes = cl::BufferGL( context, CL_MEM_READ_WRITE, mGlLifetimes->getId() );
	mClRandoms = cl::BufferGL( context, CL_MEM_READ_WRITE, mGlRandoms->getId() );
	
	mClProgram = cl::Program( context, loadString( loadAsset( "kernels/particles.cl" ) ), true );
	mClUpdateKernel = cl::Kernel( mClProgram, "particle_update" );
	
	float max_life = 60.0;
	float min_velocity = 0.5;
	
	mClUpdateKernel.setArg( 0, mClPositions );
	mClUpdateKernel.setArg( 1, mClVelocities );
	mClUpdateKernel.setArg( 2, mClLifetimes );
	mClUpdateKernel.setArg( 3, mClRandoms );
	mClUpdateKernel.setArg( 4, sizeof(float), &max_life );
	mClUpdateKernel.setArg( 5, sizeof(float), &min_velocity );
	mClUpdateKernel.setArg( 9, sizeof(cl_int), &particle_count );
	
	mGlObjects.resize( 4 );
	mGlObjects[0] = mClPositions;
	mGlObjects[1] = mClVelocities;
	mGlObjects[2] = mClLifetimes;
	mGlObjects[3] = mClRandoms;
}

std::vector<cl::Memory>& Particles::getAcqRelMemObjs()
{
	static std::vector<cl::Memory> vboMem = {
		mClPositions,
		mClVelocities,
		mClLifetimes,
		mClRandoms
	};
	
	return vboMem;
}

void Particles::update()
{
	int random = rand();
	float time = 1.0f/60.0f;
	
	mClUpdateKernel.setArg( 6, sizeof(float), &time );
	mClUpdateKernel.setArg( 7, sizeof(int), &mShouldReset );
	mClUpdateKernel.setArg( 8, sizeof(int), &random );
	
	mShouldReset = 0;
	
	auto vboMem = getAcqRelMemObjs();

	cl::Event event;
	mCommandQueue.enqueueAcquireGLObjects( &mGlObjects, nullptr, &event );
	
	std::vector<cl::Event> waitList({ event });
	cl::Event kernelEvent;
    // Queue the kernel up for execution across the array
	mCommandQueue.enqueueNDRangeKernel( mClUpdateKernel,
									   cl::NullRange,
									   cl::NDRange( particle_count ),
									   cl::NullRange,
									   &waitList,
									   &kernelEvent );
	
	
	// Release the GL Object
	// Note, we should ensure OpenCL is finished with any commands that might affect the VBO
	
	mCommandQueue.enqueueReleaseGLObjects( &mGlObjects );
}



void Particles::render()
{
	ScopedVao scopeVao( mGlVao );
	ScopedGlslProg scopeGlsl( mGlProgram );
	
	gl::setDefaultShaderVars();
	gl::enable( GL_VERTEX_PROGRAM_POINT_SIZE );
	gl::drawArrays( GL_POINTS, 0, particle_count );
	gl::disable( GL_VERTEX_PROGRAM_POINT_SIZE );
}