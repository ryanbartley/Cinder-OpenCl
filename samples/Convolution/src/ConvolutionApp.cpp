#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"

#include "Cinder-OpenCl.h"

// Constants
const unsigned int inputSignalWidth  = 8;
const unsigned int inputSignalHeight = 8;

cl_uint inputSignal[inputSignalWidth][inputSignalHeight] =
{
	{3, 1, 1, 4, 8, 2, 1, 3},
	{4, 2, 1, 1, 2, 1, 2, 3},
	{4, 4, 4, 4, 3, 2, 2, 2},
	{9, 8, 3, 8, 9, 0, 0, 0},
	{9, 3, 3, 9, 0, 0, 0, 0},
	{0, 9, 0, 8, 0, 0, 0, 0},
	{3, 0, 8, 8, 9, 4, 4, 4},
	{5, 9, 8, 1, 8, 1, 1, 1}
};

const unsigned int outputSignalWidth  = 6;
const unsigned int outputSignalHeight = 6;

cl_uint outputSignal[outputSignalWidth][outputSignalHeight];

const unsigned int maskWidth  = 3;
const unsigned int maskHeight = 3;

cl_uint mask[maskWidth][maskHeight] =
{
	{1, 1, 1}, {1, 0, 1}, {1, 1, 1},
};

using namespace ci;
using namespace ci::app;
using namespace std;

class ConvolutionApp : public App {
  public:
	void setup() override;

	void createWithBlock();
	
	static void CL_CALLBACK contextCallback(
									 const char * errInfo,
									 const void * private_info,
									 size_t cb,
									 void * user_data)
	{
		std::cout << "Error occured during context use: " << errInfo << std::endl;
		// should really perform any clearup and so on at this point
		// but for simplicitly just exit.
	}
	
	cl::Platform		mClPlatform;
	cl::Context			mContext;
	cl::Program			mProgram;
	cl::Kernel			mConvolveKernel;
	cl::Buffer			mInputSignalBuffer, mMaskBuffer, mOutputSignalBuffer;
	cl::CommandQueue	mCommandQueue;
};

void ConvolutionApp::createWithBlock()
{
	// Get all of the platforms on this system
	std::vector<cl::Platform> platforms;
	cl::Platform::get( &platforms );
	// Assign the platform that we need
	mClPlatform = platforms[0];
	
	// Get the CPU Device
	std::vector<cl::Device> devices;
	mClPlatform.getDevices( CL_DEVICE_TYPE_CPU, &devices );
	
    // Next, create an OpenCL context on the selected platform.
	mContext = cl::Context( devices[0], nullptr, &ConvolutionApp::contextCallback );
	
	// Create program from source
	mProgram = cl::Program( mContext, loadString( loadAsset( "Convolution.cl" ) ), true );
	
	// Create kernel object
	mConvolveKernel = cl::Kernel( mProgram, "convolve" );
	
	// Now allocate buffers
	mInputSignalBuffer = cl::Buffer( mContext,
									CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
									sizeof(cl_uint) * inputSignalHeight * inputSignalWidth,
									static_cast<void *>(inputSignal) );
	
	mMaskBuffer = cl::Buffer( mContext,
							 CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
							 sizeof(cl_uint) * maskHeight * maskWidth,
							 static_cast<void *>(mask) );
	
	mOutputSignalBuffer = cl::Buffer( mContext,
									 CL_MEM_WRITE_ONLY,
									 sizeof(cl_uint) * outputSignalHeight * outputSignalWidth,
									 nullptr );
	
	// Pick the first device and create command queue.
	mCommandQueue = cl::CommandQueue( mContext );
	
	mConvolveKernel.setArg( 0, mInputSignalBuffer );
	mConvolveKernel.setArg( 1, mMaskBuffer );
	mConvolveKernel.setArg( 2, mOutputSignalBuffer );
	mConvolveKernel.setArg( 3, sizeof(cl_uint), (void*)&inputSignalWidth );
	mConvolveKernel.setArg( 4, sizeof(cl_uint), (void*)&maskWidth );
	
    // Queue the kernel up for execution across the array
	mCommandQueue.enqueueNDRangeKernel( mConvolveKernel,
									   cl::NullRange,
									   cl::NDRange( outputSignalWidth * outputSignalHeight ),
									   cl::NDRange( 1 ) );
    
	mCommandQueue.enqueueReadBuffer( mOutputSignalBuffer, CL_TRUE, 0, sizeof(cl_uint) * outputSignalHeight * outputSignalHeight, outputSignal);
	
	console() << std::endl << std::endl;
    // Output the result buffer
    for (int y = 0; y < outputSignalHeight; y++)
	{
		for (int x = 0; x < outputSignalWidth; x++)
		{
			console() << outputSignal[x][y] << " ";
		}
		console() << std::endl;
	}
	
	console() << std::endl << "Executed program succesfully." << std::endl << std::endl;
}

void ConvolutionApp::setup()
{
	createWithBlock();
	quit();
}

CINDER_APP( ConvolutionApp, RendererGl )
