#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "BufferObj.h"
#include "Platform.h"
#include "Device.h"
#include "Context.h"
#include "Program.h"
#include "CommandQueue.h"

#include "Particles.h"

#pragma OPENCL EXTENSION cl_khr_gl_event : enable

using namespace ci;
using namespace ci::app;
using namespace std;

class MetaBallsApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	
	CameraPersp			mCam;
	
	ParticlesRef		mParticles;
	cl::PlatformRef		mClPlatform;
	cl::ContextRef		mClContext;
	cl::CommandQueueRef mClCommandQueue;
};

void MetaBallsApp::setup()
{
	// First, select an OpenCL platform to run on.
	mClPlatform = cl::Platform::create( cl::Platform::getAvailablePlatforms()[0], true );
	
    // Next, create an OpenCL context on the selected platform.
	// And authorize creation of the sharing context
    mClContext = cl::Context::create( mClPlatform, true );
	
    // Create a command-queue on the first device available
    // on the created context
    mClCommandQueue = cl::CommandQueue::create( mClContext->getAssociatedDevices()[0] );
	
	mParticles = Particles::create();
	
	mCam.setPerspective( 60, getWindowAspectRatio(), 0.01, 1000 );
	mCam.lookAt( vec3( 32, 15, 35 ), vec3( 32, 15, 32 ) );
}

void MetaBallsApp::mouseDown( MouseEvent event )
{
}

void MetaBallsApp::update()
{
	mParticles->update( mClCommandQueue );
}

void MetaBallsApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	
	gl::setMatrices( mCam );
	
	mParticles->render();
}

CINDER_APP_NATIVE( MetaBallsApp, RendererGl )
