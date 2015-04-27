#include "cinder/app/App.h"
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

class HelloWorldApp : public App {
  public:
	void setup() override;
	void draw();
	
	cl::PlatformRef		mClPlatform;
	cl::ContextRef		mContext;
	cl::ProgramRef		mProgram;
	cl::CommandQueueRef mCommandQueue;
	cl::BufferObjRef	mMemObjects[3];
};

void HelloWorldApp::setup()
{
	auto platforms = cl::Platform::getAvailablePlatforms();
	
	mClPlatform = cl::Platform::create( platforms[0], CL_DEVICE_TYPE_GPU );
	
    // Create an OpenCL context on first available platform
	mContext = cl::Context::create( mClPlatform, false );
	
    // Create a command-queue on the first device available
    // on the created context
	mCommandQueue = cl::CommandQueue::create( mClPlatform->getDevices()[0] );
	
    // Create OpenCL program from HelloWorld.cl kernel source
	mProgram = cl::Program::create( loadAsset( "HelloWorld.cl" ) );
    
	// Create OpenCL kernel
	mProgram->createKernel( "hello_kernel" );
	
	std::map<std::string, cl::DeviceRef> mDevFilePair;
	mDevFilePair.insert( std::pair<std::string, cl::DeviceRef>( "mHelloWorld.cl.bin", mClPlatform->getDevices()[0] ) );
	mProgram->saveBinaries( mDevFilePair );
	
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
	
	mCommandQueue->read( mMemObjects[2], true, 0, ARRAY_SIZE * sizeof(float), result );
	
    // Output the result buffer
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        std::cout << result[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "Executed program succesfully." << std::endl;
}

void HelloWorldApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( HelloWorldApp, RendererGl )
