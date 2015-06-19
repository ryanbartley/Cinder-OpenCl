//
//  MarchingCubes.h
//  MetaBalls2
//
//  Created by Ryan Bartley on 7/19/14.
//
//

#pragma once

#include "Cinder-OpenCL.h"
#include "cinder/gl/Vbo.h"

const int width = 64;
const int height = 64;
const int depth = 64;

const int MAX_VERTS = 100000;

using MarchingCubesRef = std::shared_ptr<class MarchingCubes>;

class MarchingCubes {
public:
	static int num_verts;
	
	static MarchingCubesRef create();
	
	~MarchingCubes() {}
	
	void clear();
	void point( const ci::ivec4 &point );
	void metaball( const ci::vec3 &pos );
	
	void update();
	void render();
	
	void cacheMarchingCubesMetaballData( const cl::BufferGL &positions, int numBalls );
	
	std::vector<cl::Memory> getInterop();
	
private:
	MarchingCubes();
	
	void setupGl();
	void setupCl();
	
	ci::gl::BatchRef		mRenderBatch, mShadowBatch, mDebugBatch;
	ci::gl::VboRef			mGlPointPositions, mGlPointColors,
							mGlVertPositions, mGlVertNormals;
	ci::gl::Texture2dRef	mEnvironmentMap;
	
	cl::BufferGL			mClPointColors, mClVertPositions,
							mClVertNormals, mMetaballPositions;
	cl::Buffer				mClVolume, mClVertIndex;
	cl::Program				mClProgram;
	cl::Kernel				mKernWritePoint, mKernWriteMetaball,
							mKernWriteMetaballs, mKernWriteClear,
							mKernWritePointColorBack,
							mKernConstructSurface, mKernGenNormals,
							mKernGenNormalsSmooth;
	
	bool					mDebugDraw;
	int						mNumBalls;
};
