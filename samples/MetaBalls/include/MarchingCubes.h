//
//  MarchingCubes.h
//  MetaBalls2
//
//  Created by Ryan Bartley on 7/19/14.
//
//

#pragma once

#include "BufferObj.h"
#include "cinder/gl/Vbo.h"
#include "Program.h"

const int width = 64;
const int height = 64;
const int depth = 64;

int num_verts = 0;

#define MAX_VERTS 100000

using MarchingCubesRef = std::shared_ptr<class MarchingCubes>;

class MarchingCubes {
public:
	
	static MarchingCubesRef create( const ci::cl::CommandQueueRef &commandQueue )
	{ return MarchingCubesRef( new MarchingCubes( commandQueue ) ); }
	
	~MarchingCubes() {}
	
	void clear();
	void point( const ci::ivec4 &point );
	void metaball( const ci::vec3 &pos );
	
	void data( const ci::cl::BufferObjRef positions, int numMetaballs );
	void update();
	void render();
	void renderShadows();
	
	void cacheMarchingCubesMetaballData( const ci::cl::BufferObjRef &positions, int numBalls );
	
	std::vector<ci::cl::MemoryObjRef>& getAcqRelMemObjs();
	
private:
	MarchingCubes( const ci::cl::CommandQueueRef &commandQueue );
	
	ci::gl::VaoRef			mVao;
	ci::gl::GlslProgRef		mRenderGlsl, mShadowGlsl;
	ci::gl::VboRef			mGlPointPositions, mGlPointColors,
							mGlVertPositions, mGlVertNormals;
	ci::cl::BufferObjRef	mClVolume, mClPointColors,
							mClVertPositions, mClVertNormals,
							mClVertIndex, mMetaballPositions;
	ci::cl::ProgramRef		mClProgram;
	ci::cl::KernelRef		mKernWritePoint, mKernWriteMetaball,
							mKernWriteMetaballs, mKernWriteClear,
							mKernWritePointColorBack, mKernConstructSurface,
							mKernGenNormals, mKernGenNormalsSmooth;
	ci::cl::CommandQueueRef mCommandQueue;
	int						mNumBalls;
};
