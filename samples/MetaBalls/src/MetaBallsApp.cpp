#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/ObjLoader.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/Sync.h"

#include "Cinder-OpenCL.h"

#include "Particles.h"
#include "MarchingCubes.h"
#include "Podium.h"

#pragma OPENCL EXTENSION cl_khr_gl_event : enable

using namespace ci;
using namespace ci::app;
using namespace std;

class MetaBallsApp : public App {
  public:
	void setup();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void update();
	void draw();
	
	static void contextErrorCallback( const char *errinfo,
									 const void *private_info,
									 ::size_t cb,
									 void *user_data)
	{
		cout << "ERROR: " << errinfo << endl;
	}
	
	CameraPersp			mCam;
	CameraUi			mCamUI;
	
	ParticlesRef		mParticles;
	MarchingCubesRef	mMarchingCubes;
	cl::Platform		mClPlatform;
	cl::Context			mClContext;
	cl::CommandQueue	mClCommandQueue;
	
	PodiumRef			mPodium;
	GLsync				mClSync;
	gl::SyncRef			mGlSync;
};

void MetaBallsApp::setup()
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
							 &MetaBallsApp::contextErrorCallback );
	
	
    // Create a command-queue on the first device available
    // on the created context
	mClCommandQueue = cl::CommandQueue( mClContext ) ;
	
	mParticles = Particles::create( mClContext, mClCommandQueue );
	mMarchingCubes = MarchingCubes::create( mClContext, mClCommandQueue );
	mMarchingCubes->cacheMarchingCubesMetaballData( mParticles->getClPositions(), mParticles->getNumParticles() );
	
	mPodium = Podium::create();
	
	mCam.setPerspective( 60, getWindowAspectRatio(), 0.01, 1000 );
	mCam.lookAt( vec3( 50, 50, 50 ), vec3( 32, 15, 32 ) );
	mCamUI.setCamera( &mCam );
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void MetaBallsApp::mouseDown( MouseEvent event )
{
	mCamUI.mouseDown( event );
}

void MetaBallsApp::mouseDrag( MouseEvent event )
{
	mCamUI.mouseDrag( event );
}

void MetaBallsApp::update()
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

void MetaBallsApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	
	gl::setMatrices( mCam );
	mPodium->draw();
	mMarchingCubes->render();
	getWindow()->setTitle( to_string( getAverageFps() ) );
}

CINDER_APP( MetaBallsApp, RendererGl )
