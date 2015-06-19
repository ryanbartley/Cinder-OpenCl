//
//  MetaBallsController.cpp
//  MetaBallsController
//
//  Created by ryan bartley on 6/14/15.
//
//

#include "MetaBallsController.h"

#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static MetaBallsController* sInstance;

MetaBallsController::MetaBallsController()
{
	CI_ASSERT( sInstance == nullptr );
	sInstance = this;
}

MetaBallsController::~MetaBallsController()
{
	sInstance = nullptr;
}

MetaBallsController* MetaBallsController::get()
{
	CI_ASSERT( sInstance );
	return sInstance;
}

void MetaBallsController::contextCallback( const char *errinfo, const void *private_info, ::size_t cb, void *user_data)
{
	CI_LOG_I( errinfo );
}

void MetaBallsController::setup()
{
	setupCl();
	setupScene();
	
	mCam.setPerspective( 60, getWindowAspectRatio(), 0.01, 1000 );
	mCam.lookAt( vec3( 50, 50, 50 ), vec3( 32, 15, 32 ) );
	mCamUI.setCamera( &mCam );
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void MetaBallsController::update()
{
	std::vector<cl::Memory> acquire;
	auto particleAcquire = mParticles->getInterop();
	std::copy( particleAcquire.begin(), particleAcquire.end(), std::back_inserter(acquire) );
	auto marchingAcquire = mMarchingCubes->getInterop();
	std::copy( marchingAcquire.begin(), marchingAcquire.end(), std::back_inserter(acquire) );
	
	mClCommandQueue.enqueueAcquireGLObjects( &acquire );
	mParticles->update();
	mMarchingCubes->update();
	mClCommandQueue.enqueueReleaseGLObjects( &acquire );
}

void MetaBallsController::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	
	gl::setMatrices( mCam );
	mPodium->draw();
	mMarchingCubes->render();
}

void MetaBallsController::mouseDown( MouseEvent event )
{
	mCamUI.mouseDown( event );
}

void MetaBallsController::mouseDrag( MouseEvent event )
{
	mCamUI.mouseDrag( event );
}

void MetaBallsController::setupCl()
{
	// Get all of the platforms on this system
	std::vector<cl::Platform> platforms;
	cl::Platform::get( &platforms );
	// Assign the platform that we need
	mClPlatform = cl::Platform( platforms[0] );
	
	// Print the information for each platform
	for( auto & platform : platforms ){
		cout << platform << endl;
	}
	
	// Get the GPU devices from the platform
	std::vector<cl::Device> devices;
	mClPlatform.getDevices( CL_DEVICE_TYPE_GPU, &devices );
	for( auto & device : devices ) {
		cout << device << endl;
	}
	
	// Next, create an OpenCL context on the selected platform.
	// And authorize creation of the sharing context
	// The true tells Context to create a sharing context and
	// then caches the device associated with gl
	mClContext = cl::Context( devices,
							 getDefaultSharedGraphicsContextProperties(),
							 &MetaBallsController::contextCallback );
	
	
	// Create a command-queue on the first device available
	// on the created context
	mClCommandQueue = cl::CommandQueue( mClContext ) ;
}

void MetaBallsController::setupScene()
{
	mParticles = Particles::create();
	mMarchingCubes = MarchingCubes::create();
	mMarchingCubes->cacheMarchingCubesMetaballData( mParticles->getClPositions(), mParticles->getNumParticles() );
	
	mPodium = Podium::create();
}