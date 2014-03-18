#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "BufferObj.h"
#include "Platform.h"
#include "Device.h"
#include "Context.h"
#include "Program.h"
#include "CommandQueue.h"

const int ARRAY_SIZE = 100;

using namespace ci;
using namespace ci::app;
using namespace std;

class ImplProjectApp : public AppNative {
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

void ImplProjectApp::setup()
{
	auto platforms = cl::Platform::getAvailablePlatforms();
	
	for( auto platformIt = platforms.begin(); platformIt != platforms.end(); ++platformIt ) {
		cl::Platform::displayPlatformInfo( *platformIt );
	}
	auto devices = cl::Device::getAvailableDevices( platforms[0], CL_DEVICE_TYPE_ALL );
	
	for( auto deviceIt = devices.begin(); deviceIt != devices.end(); ++deviceIt ) {
		cl::Device::displayDeviceInfo<cl_uint>(*deviceIt, CL_DEVICE_MAX_COMPUTE_UNITS, "Device" );
		cl::Device::displayDeviceInfo<cl::ArrayType<char>>( *deviceIt, CL_DEVICE_NAME, "Device has max compute units" );
		cl::Device::displayDeviceInfo<cl::ArrayType<char>>( *deviceIt, CL_DEVICE_VENDOR, "Device has max compute units" );
		cl::Device::displayDeviceInfo<cl::ArrayType<char>>( *deviceIt, CL_DEVICE_VERSION, "Device has max compute units" );
		cl::Device::displayDeviceInfo<cl::ArrayType<char>>( *deviceIt, CL_DEVICE_EXTENSIONS, "Device has max compute units" );
	}
	
	mClPlatform = cl::Platform::create( platforms[0], devices );
	
	mContext = cl::Context::create( mClPlatform, false );
	
	mProgram = cl::Program::create( loadAsset( "HelloWorld.cl" ), mClPlatform );
//	mProgram->createKernel( "hello_kernel" );
	
	mCommandQueue = cl::CommandQueue::create( mClPlatform->getDeviceByType( CL_DEVICE_TYPE_GPU ) );
	
	float result[ARRAY_SIZE];
	float a[ARRAY_SIZE];
	float b[ARRAY_SIZE];
	for( int i = 0; i < ARRAY_SIZE; ++i ) {
		a[i] = (float)i;
		b[i] = (float)( i * 2 );
	}
	
	mMemObjects[0] = cl::BufferObj::create( CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * ARRAY_SIZE, a );
	mMemObjects[1] = cl::BufferObj::create( CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * ARRAY_SIZE, b );
	
	mMemObjects[2] = cl::BufferObj::create( CL_MEM_READ_WRITE, sizeof(float) * ARRAY_SIZE, nullptr );
	
//	mProgram->setKernelArg( "hello_kernel", 0, mMemObjects[0] );
//	mProgram->setKernelArg( "hello_kernel", 1, mMemObjects[1] );
//	mProgram->setKernelArg( "hello_kernel", 2, mMemObjects[2] );
	
	cl_int errNum;
	cl_mem mMem = nullptr;
	cl_kernel kernel = nullptr;
	mMem = clCreateBuffer( cl::Context::context()->getId(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * ARRAY_SIZE, a, &errNum );
	
	if( errNum != CL_SUCCESS ) {
		cout << "ERROR: creating buffer " << errNum << endl;
		quit();
	}
	kernel = clCreateKernel( mProgram->getId(), "hello_kernel", &errNum );
	if( errNum != CL_SUCCESS ) {
		cout << "ERROR: creating buffer " << errNum << endl;
		quit();
	}
	
	
	clSetKernelArg( kernel, 0, sizeof(mMem), mMem );
	
	size_t globalWorkSize[1] = { ARRAY_SIZE };
	size_t localWorkSize[1] = { 1 };
	

	
	errNum = clEnqueueNDRangeKernel( mCommandQueue->getId(), mProgram->getKernelIdByName( "hello_kernel" ), 1, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr );
	
	if (errNum != CL_SUCCESS) {
        cerr << "Error queuing kernel for execution." << endl;
        exit(EXIT_FAILURE);
	}
	
	mMemObjects[2]->enqueueRead( mCommandQueue, true, 0, sizeof(float) * ARRAY_SIZE, result );
	
	for( int i = 0; i < ARRAY_SIZE; ++i ) {
		std::cout << result[i] << std::endl;
	}
}

void ImplProjectApp::mouseDown( MouseEvent event )
{
}

void ImplProjectApp::update()
{
}

void ImplProjectApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( ImplProjectApp, RendererGl )
