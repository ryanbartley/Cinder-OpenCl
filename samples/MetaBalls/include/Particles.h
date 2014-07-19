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
	
	static ParticlesRef create()
	{ return ParticlesRef( new Particles ); }
	
	~Particles() {}
	
	void update( const ci::cl::CommandQueueRef &commandQueue );
	void render();
	
	int count();
	void reset();
	
	ci::cl::BufferObjRef& getClPositions() { return mClPositions; }
	
	ci::gl::VboRef& getGlPositions() { return mGlPositions; }
	ci::gl::VboRef& getGlVelocities() { return mGlVelocities; }
	
private:
	Particles();
	
	ci::gl::VaoRef			mGlVao;
	ci::gl::VboRef			mGlPositions, mGlVelocities, mGlLifetimes, mGlRandoms;
	ci::gl::GlslProgRef		mGlProgram;
	ci::cl::BufferObjRef	mClPositions, mClVelocities, mClLifetimes, mClRandoms;
	ci::cl::ProgramRef		mClProgram;
	int						mShouldReset;
	cl_mem					mGlObjects[4];
};