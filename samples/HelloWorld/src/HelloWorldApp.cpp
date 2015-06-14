#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"

#include "Cinder-OpenCL.h"

const int ARRAY_SIZE = 1000;

using namespace ci;
using namespace ci::app;
using namespace std;

class HelloWorldApp : public App {
  public:
	void setup() override;
	
	static void contextErrorCallback( const char *errinfo,
								const void *private_info,
								::size_t cb,
								void *user_data)
	{
		cout << "ERROR: " << errinfo << endl;
	}
	
	cl::Platform		mClPlatform;
	cl::Context			mContext;
	cl::Program			mProgram;
	cl::CommandQueue	mCommandQueue;
	cl::Buffer			mMemObjects[3];
};

void HelloWorldApp::setup()
{
	// Get all of the platforms on this system
	std::vector<cl::Platform> platforms;
	cl::Platform::get( &platforms );
	// Assign the platform that we need
	mClPlatform = platforms[0];
	
	// Print the information for each platform
	for( auto & platform : platforms ){
		cout << platform << endl;
	}
	
	// Get the GPU devices from the platform
	std::vector<cl::Device> devices;
	mClPlatform.getDevices( CL_DEVICE_TYPE_GPU, &devices );
	for( auto & device : devices ) {
		cout << "DEVICE NAME: " << device.getInfo<CL_DEVICE_NAME>() << endl;
	}
	// Create an OpenCL context on first available platform
	mContext = cl::Context( devices, nullptr, &HelloWorldApp::contextErrorCallback );

    // Create a command-queue on the first device available
    // on the created context
	mCommandQueue = cl::CommandQueue( mContext );

    // Create OpenCL program from HelloWorld.cl kernel source
	mProgram = cl::Program( mContext, loadString( loadAsset( "HelloWorld.cl" ) ), true );

	// Create OpenCL kernel
	std::vector<cl::Kernel> kernels;
	mProgram.createKernels( &kernels );
	for( auto & kernel : kernels ) {
		cout << "KERNEL NAME: " << kernel.getInfo<CL_KERNEL_FUNCTION_NAME>() << endl;
	}
	cout << endl;
	
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
	
    mMemObjects[0] = cl::Buffer( mContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * ARRAY_SIZE, a);
	mMemObjects[1] = cl::Buffer( mContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * ARRAY_SIZE, b );
	mMemObjects[2] = cl::Buffer( mContext, CL_MEM_READ_WRITE, sizeof(float) * ARRAY_SIZE, NULL );

	kernels[0].setArg( 0, mMemObjects[0] );
	kernels[0].setArg( 1, mMemObjects[1] );
	kernels[0].setArg( 2, mMemObjects[2] );

	cl::Event event;
    // Queue the kernel up for execution across the array
	mCommandQueue.enqueueNDRangeKernel( kernels[0],
									   cl::NullRange,
									   cl::NDRange( ARRAY_SIZE ),
									   cl::NDRange( 1 ),
									   nullptr,
									   &event );
	
	std::vector<cl::Event> waitEvents = { event };
	mCommandQueue.enqueueReadBuffer( mMemObjects[2], true, 0, ARRAY_SIZE * sizeof(float), result, &waitEvents );
	
    // Output the result buffer
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        std::cout << result[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "Executed program succesfully." << std::endl;
	quit();
}

CINDER_APP( HelloWorldApp, RendererGl )
