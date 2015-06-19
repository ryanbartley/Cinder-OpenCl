//
//  MetaBallsController.h
//  MetaBallsController
//
//  Created by ryan bartley on 6/14/15.
//
//

#pragma once

#include "cinder/CameraUi.h"
#include "Particles.h"
#include "MarchingCubes.h"
#include "Podium.h"

#pragma OPENCL EXTENSION cl_khr_gl_event : enable

class MetaBallsController {
public:
	MetaBallsController();
	~MetaBallsController();
	
	static MetaBallsController* get();
	
	static void contextCallback( const char *errinfo, const void *private_info, ::size_t cb, void *user_data);
	
	void setup();
	void update();
	void draw();
	
	void mouseDown( ci::app::MouseEvent event );
	void mouseDrag( ci::app::MouseEvent event );
	
	cl::Context& getContext() { return mClContext; }
	cl::CommandQueue& getCommandQueue() { return mClCommandQueue; }
	
private:
	
	void setupCl();
	void setupScene();
	
	cl::Platform		mClPlatform;
	cl::Context			mClContext;
	cl::CommandQueue	mClCommandQueue;
	
	ParticlesRef		mParticles;
	MarchingCubesRef	mMarchingCubes;
	PodiumRef			mPodium;
	
	ci::CameraPersp		mCam;
	ci::CameraUi		mCamUI;
	ci::gl::Texture2dRef mEnvironmentMap;
};
