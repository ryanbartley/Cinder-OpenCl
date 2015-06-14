//
//  Particles.h
//  MetaBalls2
//
//  Created by Ryan Bartley on 7/19/14.
//
//

#include "Cinder-OpenCL.h"
#include "cinder/gl/Vbo.h"

static int particle_count = 64;

typedef std::shared_ptr<class Particles> ParticlesRef;

class Particles {
public:
	
	static ParticlesRef create( const cl::Context &context, const cl::CommandQueue &commandQueue );
	
	~Particles() {}
	
	void update();
	void render();
	
	void reset() { mShouldReset = 1; }
	int count() { return particle_count; }
	
	cl::BufferGL& getClPositions() { return mClPositions; }
	
	ci::gl::VboRef& getGlPositions() { return mGlPositions; }
	ci::gl::VboRef& getGlVelocities() { return mGlVelocities; }
	
	std::vector<cl::Memory>& getAcqRelMemObjs();
	
	int getNumParticles() const { return particle_count; };
	
private:
	Particles( const cl::Context &context, const cl::CommandQueue &commandQueue );
	
	ci::gl::VaoRef				mGlVao;
	ci::gl::VboRef				mGlPositions, mGlVelocities, mGlLifetimes, mGlRandoms;
	ci::gl::GlslProgRef			mGlProgram;
	cl::BufferGL				mClPositions, mClVelocities, mClLifetimes, mClRandoms;
	cl::Program					mClProgram;
	cl::Kernel					mClUpdateKernel;
	cl::CommandQueue			mCommandQueue;
	int							mShouldReset;
	std::vector<cl::Memory>		mGlObjects;
};