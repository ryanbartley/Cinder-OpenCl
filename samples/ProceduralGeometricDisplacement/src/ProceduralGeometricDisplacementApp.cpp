#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/CameraUi.h"

#include "cinder/gl/GlslProg.h"

#include "Platform.h"
#include "Device.h"
#include "Context.h"
#include "Program.h"
#include "CommandQueue.h"

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
	
	CameraPersp		mCam, mLightCam;
	CameraUi		mCameraControl;
	
	gl::GlslProgRef mPhongGlsl, mFresnelGlsl, mSkyboxGlsl;
	
	cl::ContextRef			mComputeContext;
	cl::Program::KernelRef	mComputeKernel;
	cl::ProgramRef			mComputeProgram;
	cl::DeviceRef			mComputeDevice;
	cl::CommandQueueRef		mComputeQueue;
	cl::BufferObjRef		mInputVertexBuffer,
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
	auto platforms = cl::Platform::getAvailablePlatforms();
	auto platform = cl::Platform::create( platforms[0], CL_DEVICE_TYPE_GPU );
	mComputeDevice = platform->getDeviceByType( CL_DEVICE_TYPE_GPU );
	mComputeContext = cl::Context::create( platform, true );
	mComputeQueue = cl::CommandQueue::create( mComputeDevice );
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
