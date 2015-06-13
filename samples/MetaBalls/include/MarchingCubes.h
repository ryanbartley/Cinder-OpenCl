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
	
	static MarchingCubesRef create( const cl::CommandQueueRef &commandQueue )
	{ return MarchingCubesRef( new MarchingCubes( commandQueue ) ); }
	
	~MarchingCubes() {}
	
	void clear();
	void point( const ci::ivec4 &point );
	void metaball( const ci::vec3 &pos );
	
	void data( const cl::BufferObjRef positions, int numMetaballs );
	void update();
	void render();
	void renderShadows();
	
	void cacheMarchingCubesMetaballData( const cl::BufferObjRef &positions, int numBalls );
	
	std::vector<cl::MemoryObjRef>& getAcqRelMemObjs();
	
private:
	MarchingCubes( const cl::CommandQueueRef &commandQueue );
	
	ci::gl::VaoRef			mVao;
	ci::gl::GlslProgRef		mRenderGlsl, mShadowGlsl;
	ci::gl::VboRef			mGlPointPositions, mGlPointColors,
							mGlVertPositions, mGlVertNormals;
	cl::BufferObjRef		mClVolume, mClPointColors,
							mClVertPositions, mClVertNormals,
							mClVertIndex, mMetaballPositions;
	cl::ProgramRef			mClProgram;
	cl::KernelRef			mKernWritePoint, mKernWriteMetaball,
							mKernWriteMetaballs, mKernWriteClear,
							mKernWritePointColorBack, mKernConstructSurface,
							mKernGenNormals, mKernGenNormalsSmooth;
	cl::CommandQueueRef		mCommandQueue;
	int						mNumBalls;
};
