//
//  Particles.h
//  MetaBalls2
//
//  Created by Ryan Bartley on 7/19/14.
//
//

#pragma once

#include "Cinder-OpenCL.h"
#include "cinder/gl/gl.h"

using ParticlesRef = std::shared_ptr<class Particles>;

class Particles {
public:
	
	static ParticlesRef create();
	
	~Particles() {}
	
	void update();
	void render();
	
	void reset() { mShouldReset = true; }
	
	cl::BufferGL& getClPositions() { return mClPositions; }
	cl::BufferGL& getClLifetimes() { return mClLifetimes; }
	
	std::vector<cl::Memory> getInterop();
	
	ci::gl::VboRef& getGlPositions() { return mGlPositions; }
	ci::gl::VboRef& getGlVelocities() { return mGlVelocities; }
	
	int getNumParticles() const { return sParticleCount; };
	
	static const int sParticleCount;
	
	void toggleDebugDraw();
	
	bool shouldDebugDraw() { return mDebugDraw; }
	
private:
	Particles();
	
	void setupGl();
	void setupCl();
	
	ci::gl::VaoRef				mGlVao;
	ci::gl::VboRef				mGlPositions, mGlVelocities, mGlLifetimes, mGlRandoms;
	ci::gl::GlslProgRef			mGlProgram;
	
	cl::BufferGL				mClPositions, mClVelocities, mClLifetimes, mClRandoms;
	cl::Program					mClProgram;
	cl::Kernel					mClUpdateKernel;
	
	bool						mShouldReset, mDebugDraw;
};