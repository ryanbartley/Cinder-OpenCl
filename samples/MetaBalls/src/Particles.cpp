//
//  Particles.cpp
//  MetaBalls2
//
//  Created by Ryan Bartley on 7/19/14.
//
//

#include "Particles.h"
#include "CommandQueue.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/GlslProg.h"

using namespace ci;
using namespace ci::app;
using namespace ci::cl;
using namespace ci::gl;
using namespace std;

Particles::Particles()
{
	std::vector<Vec4f>	cpuPositions(particle_count),
						cpuVelocities(particle_count),
						cpuRandoms(particle_count);
	std::vector<float>  cpuLifetimes(particle_count);
	
	srand(time(NULL));
	
	for(int i = 0; i < particle_count; i++) {
		cpuLifetimes[i] = 999;
		cpuPositions[i] = Vec4f::zero();
		cpuVelocities[i] = Vec4f::zero();
	
		float rx = ((float)rand() / RAND_MAX) * 2 - 1;
		float ry = ((float)rand() / RAND_MAX) * 2 + 0.5;
		float rz = ((float)rand() / RAND_MAX) * 2 - 1;
		float rm = (float)rand() / RAND_MAX;
	
		Vec3f rand = Vec3f(rx, ry, rz).normalized() * (rm * 2);
	
		cpuRandoms[i] = Vec4f(rand);
	}
	
	mGlPositions = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(Vec4f) * particle_count, cpuPositions.data(), GL_DYNAMIC_COPY );
	mGlVelocities = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(Vec4f) * particle_count, cpuVelocities.data(), GL_DYNAMIC_COPY );
	mGlLifetimes = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(float) * particle_count, cpuLifetimes.data(), GL_DYNAMIC_COPY );
	mGlRandoms = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(Vec4f) * particle_count, cpuRandoms.data(), GL_DYNAMIC_COPY );
	
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
									  .attribLocation( "position", 0 )
									  .attribLocation( "color", 1 )
									  .uniform( UNIFORM_MODEL_VIEW_PROJECTION, "mvp" ) );
	
	mClPositions = cl::BufferObj::create( mGlPositions, CL_MEM_READ_WRITE );
	mClVelocities = cl::BufferObj::create( mGlVelocities, CL_MEM_READ_WRITE );
	mClLifetimes = cl::BufferObj::create( mGlLifetimes, CL_MEM_READ_WRITE );
	mClRandoms = cl::BufferObj::create( mGlRandoms, CL_MEM_READ_WRITE );
	
	mClProgram = cl::Program::create( loadAsset( "particlesProg.cl" ) );
	auto kernel = mClProgram->createKernel( "particle_update" );
	
	mGlObjects[0] = mClPositions->getId();
	mGlObjects[1] = mClVelocities->getId();
	mGlObjects[2] = mClLifetimes->getId();
	mGlObjects[3] = mClRandoms->getId();
	
	float max_life = 60.0;
	float min_velocity = 0.5;
	
	kernel->setKernelArg( 0, mClPositions );
	kernel->setKernelArg( 1, mClVelocities );
	kernel->setKernelArg( 2, mClLifetimes );
	kernel->setKernelArg( 3, mClRandoms );
	kernel->setKernelArg( 4, sizeof(float), &max_life );
	kernel->setKernelArg( 5, sizeof(float), &min_velocity );
	kernel->setKernelArg( 9, sizeof(cl_int), &particle_count );
}

void Particles::update( const cl::CommandQueueRef &commandQueue )
{
	cl_int errNum = 0;
	int random = rand();
	auto kernel = mClProgram->getKernelByName( "particle_update" );
	float time = 1.0f/60.0f;
	
	kernel->setKernelArg( 6, sizeof(float), &time );
	kernel->setKernelArg( 7, sizeof(int), &mShouldReset );
	kernel->setKernelArg( 8, sizeof(int), &random );
	size_t globalWorkSize[1] = { static_cast<size_t>(particle_count) };
	
	const cl_mem vboMem[4] = {
		mClPositions->getId(),
		mClVelocities->getId(),
		mClLifetimes->getId(),
		mClRandoms->getId()
	};
	
	glFinish();
	glFinish();
	
	cl_event event;
	
	errNum = clEnqueueAcquireGLObjects( commandQueue->getId(), 4, vboMem, 0, NULL, &event );
	
	if ( errNum ) {
		std::cout << "ERROR: Aquiring gl objects" << std::endl;
	}
	
	cl_event events[1] = { event };
    // Queue the kernel up for execution across the array
    errNum = clEnqueueNDRangeKernel( commandQueue->getId(), kernel->getId(), 1, NULL,
                                    globalWorkSize, 0, 1, events, NULL);
    if (errNum != CL_SUCCESS)
    {
        std::cerr << "Error queuing kernel for execution." << std::endl;
    }
	
//	std::cout << "I'm after the enqueue" << endl;
	
	// Release the GL Object
	// Note, we should ensure OpenCL is finished with any commands that might affect the VBO
	
	clFinish(commandQueue->getId());
	
	errNum = clEnqueueReleaseGLObjects( commandQueue->getId(), 4, vboMem, 0, NULL, NULL );
	
	if ( errNum ) {
		std::cout << "ERROR: Releasing gl objects" << std::endl;
	}
	clFinish( commandQueue->getId() );
	cout << "I'm after the release" << endl;
	cout << "________________________________________________________" << endl;
	float* map = (float*)mGlLifetimes->map( GL_READ_ONLY );
	for( int i = 0; i < particle_count; i++ ) {
		std::cout << map[i] << std::endl;
	}
	mGlLifetimes->unmap();
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