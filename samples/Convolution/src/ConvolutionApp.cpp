#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "BufferObj.h"
#include "Platform.h"
#include "Device.h"
#include "Context.h"
#include "Program.h"
#include "CommandQueue.h"

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

///
// Function to check and handle OpenCL errors
inline void
checkErr(cl_int err, const char * name)
{
    if (err != CL_SUCCESS) {
        std::cerr << "ERROR: " <<  name << " (" << err << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void CL_CALLBACK contextCallback(
								 const char * errInfo,
								 const void * private_info,
								 size_t cb,
								 void * user_data)
{
	std::cout << "Error occured during context use: " << errInfo << std::endl;
	// should really perform any clearup and so on at this point
	// but for simplicitly just exit.
	exit(1);
}

using namespace ci;
using namespace ci::app;
using namespace std;

class ConvolutionApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	
	void createWithCl();
	void createWithBlock();
	
	cl::PlatformRef mClPlatform;
	cl::ContextRef  mContext;
	cl::ProgramRef  mProgram;
	cl::BufferObjRef	mInputSignalBuffer, mMaskBuffer, mOutputSignalBuffer;
	cl::CommandQueueRef mCommandQueue;
};

void ConvolutionApp::createWithCl()
{
	cl_int errNum;
    cl_uint numPlatforms;
	cl_uint numDevices;
    cl_platform_id * platformIDs;
	cl_device_id * deviceIDs;
    cl_context context = NULL;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	cl_mem inputSignalBuffer;
	cl_mem outputSignalBuffer;
	cl_mem maskBuffer;
	
    // First, select an OpenCL platform to run on.
	errNum = clGetPlatformIDs(0, NULL, &numPlatforms);
	checkErr(
			 (errNum != CL_SUCCESS) ? errNum : (numPlatforms <= 0 ? -1 : CL_SUCCESS),
			 "clGetPlatformIDs");
	
	platformIDs = (cl_platform_id *)alloca(
										   sizeof(cl_platform_id) * numPlatforms);
	
    errNum = clGetPlatformIDs(numPlatforms, platformIDs, NULL);
    checkErr(
			 (errNum != CL_SUCCESS) ? errNum : (numPlatforms <= 0 ? -1 : CL_SUCCESS),
			 "clGetPlatformIDs");
	
	// Iterate through the list of platforms until we find one that supports
	// a CPU device, otherwise fail with an error.
	deviceIDs = NULL;
	cl_uint i;
	for (i = 0; i < numPlatforms; i++)
	{
		errNum = clGetDeviceIDs(
								platformIDs[i],
								CL_DEVICE_TYPE_CPU,
								0,
								NULL,
								&numDevices);
		if (errNum != CL_SUCCESS && errNum != CL_DEVICE_NOT_FOUND)
	    {
			checkErr(errNum, "clGetDeviceIDs");
        }
	    else if (numDevices > 0)
		{
		   	deviceIDs = (cl_device_id *)alloca(sizeof(cl_device_id) * numDevices);
			errNum = clGetDeviceIDs(
									platformIDs[i],
									CL_DEVICE_TYPE_CPU,
									numDevices,
									&deviceIDs[0],
									NULL);
			checkErr(errNum, "clGetDeviceIDs");
			break;
		}
	}
	
	// Check to see if we found at least one CPU device, otherwise return
	if (deviceIDs == NULL) {
		std::cout << "No CPU device found" << std::endl;
		exit(-1);
	}
	
    // Next, create an OpenCL context on the selected platform.
    cl_context_properties contextProperties[] =
    {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platformIDs[i],
        0
    };
    context = clCreateContext(
							  contextProperties,
							  numDevices,
							  deviceIDs,
							  &contextCallback,
							  NULL,
							  &errNum);
	checkErr(errNum, "clCreateContext");
	
	
	std::string srcProg;
	Buffer buffer( loadAsset( "Convolution.cl" ) );
	srcProg.resize( buffer.getDataSize() + 1 );
	memcpy( (void*)srcProg.data(), buffer.getData(), buffer.getDataSize() );
	srcProg[buffer.getDataSize()] = 0;
	size_t size = srcProg.size() - 1;
	
	const char * src[] = {srcProg.c_str()};
	
	// Create program from source
	program = clCreateProgramWithSource(
										context,
										1,
										src,
										&size,
										&errNum);
	checkErr(errNum, "clCreateProgramWithSource");
	
	// Build program
	errNum = clBuildProgram(
							program,
							numDevices,
							deviceIDs,
							NULL,
							NULL,
							NULL);
    if (errNum != CL_SUCCESS)
    {
        // Determine the reason for the error
        char buildLog[16384];
        clGetProgramBuildInfo(
							  program,
							  deviceIDs[0],
							  CL_PROGRAM_BUILD_LOG,
							  sizeof(buildLog),
							  buildLog,
							  NULL);
		
        std::cerr << "Error in kernel: " << std::endl;
        std::cerr << buildLog;
		checkErr(errNum, "clBuildProgram");
    }
	
	// Create kernel object
	kernel = clCreateKernel(
							program,
							"convolve",
							&errNum);
	checkErr(errNum, "clCreateKernel");
	
	// Now allocate buffers
	inputSignalBuffer = clCreateBuffer(
									   context,
									   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
									   sizeof(cl_uint) * inputSignalHeight * inputSignalWidth,
									   static_cast<void *>(inputSignal),
									   &errNum);
	checkErr(errNum, "clCreateBuffer(inputSignal)");
	
	maskBuffer = clCreateBuffer(
								context,
								CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
								sizeof(cl_uint) * maskHeight * maskWidth,
								static_cast<void *>(mask),
								&errNum);
	checkErr(errNum, "clCreateBuffer(mask)");
	
	outputSignalBuffer = clCreateBuffer(
										context,
										CL_MEM_WRITE_ONLY,
										sizeof(cl_uint) * outputSignalHeight * outputSignalWidth,
										NULL,
										&errNum);
	checkErr(errNum, "clCreateBuffer(outputSignal)");
	
	// Pick the first device and create command queue.
	queue = clCreateCommandQueue(
								 context,
								 deviceIDs[0],
								 0,
								 &errNum);
	checkErr(errNum, "clCreateCommandQueue");
	
    errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputSignalBuffer);
	errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &maskBuffer);
    errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &outputSignalBuffer);
	errNum |= clSetKernelArg(kernel, 3, sizeof(cl_uint), &inputSignalWidth);
	errNum |= clSetKernelArg(kernel, 4, sizeof(cl_uint), &maskWidth);
	checkErr(errNum, "clSetKernelArg");
	
	const size_t globalWorkSize[1] = { outputSignalWidth * outputSignalHeight };
    const size_t localWorkSize[1]  = { 1 };
	
    // Queue the kernel up for execution across the array
    errNum = clEnqueueNDRangeKernel(
									queue,
									kernel,
									1,
									NULL,
									globalWorkSize,
									localWorkSize,
									0,
									NULL,
									NULL);
	checkErr(errNum, "clEnqueueNDRangeKernel");
    
	errNum = clEnqueueReadBuffer(
								 queue,
								 outputSignalBuffer,
								 CL_TRUE,
								 0,
								 sizeof(cl_uint) * outputSignalHeight * outputSignalHeight,
								 outputSignal,
								 0, 
								 NULL, 
								 NULL);
	checkErr(errNum, "clEnqueueReadBuffer");
	
    // Output the result buffer
    for (int y = 0; y < outputSignalHeight; y++)
	{
		for (int x = 0; x < outputSignalWidth; x++)
		{
			std::cout << outputSignal[x][y] << " ";
		}
		std::cout << std::endl;
	}
	
    std::cout << std::endl << "Executed program succesfully." << std::endl;
}

void ConvolutionApp::createWithBlock()
{
	cl_int errNum;

    // First, select an OpenCL platform to run on.
	auto platforms = cl::Platform::getAvailablePlatforms();
	
	// Iterate through the list of platforms until we find one that supports
	// a CPU device, otherwise fail with an error.
	mClPlatform = cl::Platform::create( platforms[0], CL_DEVICE_TYPE_CPU );
	
    // Next, create an OpenCL context on the selected platform.
    mContext = cl::Context::create( mClPlatform, false );
	
	// Create program from source
	mProgram = cl::Program::create( loadAsset( "Convolution.cl" ) );
	
	// Create kernel object
	mProgram->createKernel( "convolve" );
	
	// Now allocate buffers
	mInputSignalBuffer = cl::BufferObj::create( CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * inputSignalHeight * inputSignalWidth, static_cast<void *>(inputSignal) );
	
	mMaskBuffer = cl::BufferObj::create( CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * maskHeight * maskWidth, static_cast<void *>(mask) );
	
	mOutputSignalBuffer = cl::BufferObj::create( CL_MEM_WRITE_ONLY, sizeof(cl_uint) * outputSignalHeight * outputSignalWidth, nullptr );
	
	// Pick the first device and create command queue.
	mCommandQueue = cl::CommandQueue::create( mClPlatform->getDevices()[0] );
	
	mProgram->setKernelArg( "convolve", 0, mInputSignalBuffer );
	mProgram->setKernelArg( "convolve", 1, mMaskBuffer );
	mProgram->setKernelArg( "convolve", 2, mOutputSignalBuffer );
	mProgram->setKernelArg( "convolve", 3, sizeof(cl_uint), (void*)&inputSignalWidth );
	mProgram->setKernelArg( "convolve", 4, sizeof(cl_uint), (void*)&maskWidth );
	
	const size_t globalWorkSize[1] = { outputSignalWidth * outputSignalHeight };
    const size_t localWorkSize[1]  = { 1 };
	
    // Queue the kernel up for execution across the array
    errNum = clEnqueueNDRangeKernel(
									mCommandQueue->getId(),
									mProgram->getKernelByName( "convolve" )->getId(),
									1,
									NULL,
									globalWorkSize,
									localWorkSize,
									0,
									NULL,
									NULL);
    
	mCommandQueue->read( mOutputSignalBuffer, CL_TRUE, 0, sizeof(cl_uint) * outputSignalHeight * outputSignalHeight, outputSignal);
	
    // Output the result buffer
    for (int y = 0; y < outputSignalHeight; y++)
	{
		for (int x = 0; x < outputSignalWidth; x++)
		{
			std::cout << outputSignal[x][y] << " ";
		}
		std::cout << std::endl;
	}
	
    std::cout << std::endl << "Executed program succesfully." << std::endl;
}

void ConvolutionApp::setup()
{
//	createWithCl();
	createWithBlock();
}

void ConvolutionApp::mouseDown( MouseEvent event )
{
}

void ConvolutionApp::update()
{
}

void ConvolutionApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( ConvolutionApp, RendererGl )
