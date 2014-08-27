//
//  Particles.h
//  MetaBalls2
//
//  Created by Ryan Bartley on 7/19/14.
//
//

#include "BufferObj.h"
#include "cinder/gl/Vbo.h"
#include "Program.h"

static int particle_count = 64;

typedef std::shared_ptr<class Particles> ParticlesRef;

class Particles {
public:
	
	static ParticlesRef create( const ci::cl::CommandQueueRef &commandQueue )
	{ return ParticlesRef( new Particles( commandQueue ) ); }
	
	~Particles() {}
	
	void update();
	void render();
	
	void reset() { mShouldReset = 1; }
	int count() { return particle_count; }
	
	ci::cl::BufferObjRef& getClPositions() { return mClPositions; }
	
	ci::gl::VboRef& getGlPositions() { return mGlPositions; }
	ci::gl::VboRef& getGlVelocities() { return mGlVelocities; }
	
	std::vector<ci::cl::MemoryObjRef>& getAcqRelMemObjs();
	
	int getNumParticles() const { return particle_count; };
	
private:
	Particles( const ci::cl::CommandQueueRef &commandQueue );
	
	ci::gl::VaoRef			mGlVao;
	ci::gl::VboRef			mGlPositions, mGlVelocities, mGlLifetimes, mGlRandoms;
	ci::gl::GlslProgRef		mGlProgram;
	ci::cl::BufferObjRef	mClPositions, mClVelocities, mClLifetimes, mClRandoms;
	ci::cl::ProgramRef		mClProgram;
	ci::cl::CommandQueueRef	mCommandQueue;
	int						mShouldReset;
	cl_mem					mGlObjects[4];
};