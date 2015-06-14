#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/GlslProg.h"

#include "Cinder-OpenCL.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ProceduralGeometricDisplacementApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	
	void setupCompute();
	
	static void contextErrorCallback( const char *errinfo,
									 const void *private_info,
									 ::size_t cb,
									 void *user_data)
	{
		cout << "ERROR: " << errinfo << endl;
	}
	
	CameraPersp		mCam, mLightCam;
	CameraUi		mCameraControl;
	
	gl::GlslProgRef mPhongGlsl, mFresnelGlsl, mSkyboxGlsl;
	
	cl::Context			mContext;
	cl::Kernel			mComputeKernel;
	cl::Program			mComputeProgram;
	cl::Device			mDevice;
	cl::CommandQueue	mQueue;
	cl::BufferGL		mInputVertexBuffer,
						mOutputVertexBuffer,
						mOutputNormalBuffer;
	int mMaxWorkGroupSize;
	int mGroupSize = 4;
};

void ProceduralGeometricDisplacementApp::setup()
{
	setupCompute();
}

void ProceduralGeometricDisplacementApp::setupCompute()
{
	// Get all of the platforms on this system
	std::vector<cl::Platform> platforms;
	cl::Platform::get( &platforms );
	// Assign the platform that we need
	auto clPlatform = platforms[0];
	
	// Print the information for each platform
	for( auto & platform : platforms ){
		cout << platform << endl;
	}
	
	// Get the GPU devices from the platform
	std::vector<cl::Device> devices;
	clPlatform.getDevices( CL_DEVICE_TYPE_GPU, &devices );
	for( auto & device : devices ) {
		cout << "DEVICE NAME: " << device.getInfo<CL_DEVICE_NAME>() << endl;
	}
	// Create an OpenCL context on first available platform
	mContext = cl::Context( devices, nullptr, &ProceduralGeometricDisplacementApp::contextErrorCallback );
	
	// Create a command-queue on the first device available
	// on the created context
	mQueue = cl::CommandQueue( mContext );
	
	// Create OpenCL program from HelloWorld.cl kernel source
	mComputeProgram = cl::Program( mContext, loadString( loadAsset( "displacement_kernel.cl" ) ), true );
}

void ProceduralGeometricDisplacementApp::mouseDown( MouseEvent event )
{
}

void ProceduralGeometricDisplacementApp::update()
{
}

void ProceduralGeometricDisplacementApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
}

CINDER_APP( ProceduralGeometricDisplacementApp, RendererGl )
