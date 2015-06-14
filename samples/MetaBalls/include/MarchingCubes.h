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

#define MAX_VERTS 100

using MarchingCubesRef = std::shared_ptr<class MarchingCubes>;

class MarchingCubes {
public:
	static int num_verts;
	
	static MarchingCubesRef create( const cl::Context &context, const cl::CommandQueue &commandQueue );
	
	~MarchingCubes() {}
	
	void clear();
	void point( const ci::ivec4 &point );
	void metaball( const ci::vec3 &pos );
	
	void data( const cl::Buffer positions, int numMetaballs );
	void update();
	void render();
	void renderShadows();
	
	void cacheMarchingCubesMetaballData( const cl::BufferGL &positions, int numBalls );
	
	void setupGl();
	void setupCl( const cl::Context &context );
	
private:
	MarchingCubes( const cl::Context &context, const cl::CommandQueue &commandQueue );
	
	
	ci::gl::GlslProgRef		mRenderGlsl, mShadowGlsl;
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
	cl::CommandQueue		mCommandQueue;
	int						mNumBalls;
};
