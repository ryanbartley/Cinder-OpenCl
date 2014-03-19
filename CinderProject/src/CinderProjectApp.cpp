#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "BufferObj.h"
#include "Platform.h"
#include "Device.h"
#include "Context.h"
#include "Program.h"
#include "CommandQueue.h"

const int ARRAY_SIZE = 1000;

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderProjectApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	
	cl::PlatformRef		mClPlatform;
	cl::ContextRef		mContext;
	cl::ProgramRef		mProgram;
	cl::CommandQueueRef mCommandQueue;
	cl::BufferObjRef	mMemObjects[3];
};

void CinderProjectApp::setup()
{
	auto platforms = cl::Platform::getAvailablePlatforms();
	auto devices = cl::Device::getAvailableDevices( platforms[0], CL_DEVICE_TYPE_GPU );
	
	mClPlatform = cl::Platform::create( platforms[0], devices );
	
    // Create an OpenCL context on first available platform
	mContext = cl::Context::create( mClPlatform, false );
	
    // Create a command-queue on the first device available
    // on the created context
	mCommandQueue = cl::CommandQueue::create( mClPlatform->getDevices()[0] );
	
    // Create OpenCL program from HelloWorld.cl kernel source
	mProgram = cl::Program::create( loadAsset( "HelloWorld.cl" ) );
    
	// Create OpenCL kernel
	mProgram->createKernel( "hello_kernel" );
	
    // Create memory objects that will be used as arguments to
    // kernel.  First create host memory arrays that will be
    // used to store the arguments to the kernel
    float result[ARRAY_SIZE];
    float a[ARRAY_SIZE];
    float b[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
    }
	
    mMemObjects[0] = cl::BufferObj::create( CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * ARRAY_SIZE, a);
	mMemObjects[1] = cl::BufferObj::create( CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * ARRAY_SIZE, b );
	mMemObjects[2] = cl::BufferObj::create( CL_MEM_READ_WRITE, sizeof(float) * ARRAY_SIZE, NULL );
	
	mProgram->setKernelArg( "hello_kernel", 0, mMemObjects[0] );
	mProgram->setKernelArg( "hello_kernel", 1, mMemObjects[1] );
	mProgram->setKernelArg( "hello_kernel", 2, mMemObjects[2] );
	
    size_t globalWorkSize[1] = { ARRAY_SIZE };
    size_t localWorkSize[1] = { 1 };
	
    // Queue the kernel up for execution across the array
    clEnqueueNDRangeKernel( mCommandQueue->getId(), mProgram->getKernelIdByName( "hello_kernel" ), 1, NULL,
                                    globalWorkSize, localWorkSize,
                                    0, NULL, NULL);
	
	mMemObjects[2]->enqueueRead( mCommandQueue, true, 0, ARRAY_SIZE * sizeof(float), result );
	
    // Output the result buffer
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        std::cout << result[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "Executed program succesfully." << std::endl;
}

void CinderProjectApp::mouseDown( MouseEvent event )
{
}

void CinderProjectApp::update()
{
}

void CinderProjectApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}



CINDER_APP_NATIVE( CinderProjectApp, RendererGl )
