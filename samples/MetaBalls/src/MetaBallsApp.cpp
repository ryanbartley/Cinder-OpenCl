#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/ObjLoader.h"

#include "BufferObj.h"
#include "Platform.h"
#include "Device.h"
#include "Context.h"
#include "Program.h"
#include "CommandQueue.h"

#include "Particles.h"
#include "MarchingCubes.h"

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
	MarchingCubesRef	mMarchingCubes;
	cl::PlatformRef		mClPlatform;
	cl::ContextRef		mClContext;
	cl::CommandQueueRef mClCommandQueue;
	
	gl::BatchRef		mPodium;
};

void MetaBallsApp::setup()
{
	// First, select an OpenCL platform to run on.
	mClPlatform = cl::Platform::create( cl::Platform::getAvailablePlatforms()[0], true );
	
	for( const auto& device : mClPlatform->getDevices() ) {
		cout << "Gl: " << device->isGlSupported() << endl;
	}
	
    // Next, create an OpenCL context on the selected platform.
	// And authorize creation of the sharing context
	// The true tells Context to create a sharing context and
	// then caches the device associated with gl
    mClContext = cl::Context::create( mClPlatform, true );
	
    // Create a command-queue on the first device available
    // on the created context
    mClCommandQueue = cl::CommandQueue::create( mClContext->getAssociatedDevices()[0] );
	
	mParticles = Particles::create( mClCommandQueue );
//	mMarchingCubes = MarchingCubes::create( mClCommandQueue );
	
	mPodium = gl::Batch::create( ObjLoader( loadAsset( "podium.obj" ) ), gl::getStockShader( gl::ShaderDef().color() ) );
	
	mCam.setPerspective( 60, getWindowAspectRatio(), 0.01, 1000 );
	mCam.lookAt( vec3( 32, 15, 40 ), vec3( 32, 15, 32 ) );
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void MetaBallsApp::mouseDown( MouseEvent event )
{
}

void MetaBallsApp::update()
{
	mParticles->update();
	
//	mMarchingCubes->cacheMarchingCubesMetaballData( mParticles->getClPositions(), mParticles->getNumParticles() );
//	mMarchingCubes->clear();
//	mMarchingCubes->update();
	
	float elapsed = getElapsedSeconds();
	mCam.lookAt( vec3( sin( elapsed ) * 10 + 32, 20,
					  cos( elapsed ) * 10 + 40 ), vec3( 32, 15, 32 ) );
}

void MetaBallsApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	
	gl::setMatrices( mCam );
	{
		gl::ScopedModelMatrix scopeModel;
		gl::setModelMatrix( translate( vec3( 32, 10, 32 ) ) );
		mPodium->draw();
	}

	mParticles->render();
}

CINDER_APP_NATIVE( MetaBallsApp, RendererGl )
