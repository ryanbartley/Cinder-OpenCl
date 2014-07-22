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

class MarchingCubes {
public:
	
	~MarchingCubes();
	
	void clear();
	void point( const ci::Vec4i &point );
	void metaball( const ci::Vec3f, int metaballId );
	
	void data( const ci::cl::BufferObjRef positions, int numMetaballs );
	void update();
	void render();
	void renderShadows();
	
private:
	MarchingCubes();
	
	ci::cl::BufferObjRef	mClVolume, mClPointColors,
							mClVertPositions, mClVertNormals,
							mClVertIndex;
	ci::gl::VboRef			mGlPointPositions, mGlPointColors,
							mGlVertPositions, mGlVertNormals;
	ci::cl::ProgramRef		mClProgram;
	ci::cl::KernelRef		mKernWritePoint, mKernWriteMetaball,
							mKernWriteMetaballs, mKernWriteClear,
							mKernWritePointColorBack, mKernConstructSurface,
							mKernGenNormals, mKernGenNormalsSmooth;
};
