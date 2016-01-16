//
//  MarchingCubes.h
//  MetaBalls2
//
//  Created by Ryan Bartley on 7/19/14.
//
//

#pragma once

#include "Cinder-OpenCL.h"
#include "cinder/gl/gl.h"



using MarchingCubesRef = std::shared_ptr<class MarchingCubes>;

class MarchingCubes {
public:
	static int num_verts;
	
	void clear();
	
	void update();
	
	
private:
	MarchingCubes();
	
	void setupGl();
	void setupCl();
	
	
	ci::gl::VboRef			mGlPointPositions, mGlPointColors,
							mGlVertPositions, mGlVertNormals;
	ci::gl::Texture2dRef	mEnvironmentMap;
	
	int						mNumBalls;
};
