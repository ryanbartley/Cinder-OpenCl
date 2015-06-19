#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"
#include "cinder/params/Params.h"

#include "Cinder-OpenCL.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#pragma OPENCL EXTENSION cl_khr_gl_event : enable

float * createBlurMask(float sigma, int * maskSizePointer) {
	int maskSize = (int)ceil(3.0f*sigma);
	float * mask = new float[(maskSize*2+1)*(maskSize*2+1)];
	float sum = 0.0f;
	for(int a = -maskSize; a < maskSize+1; a++) {
		for(int b = -maskSize; b < maskSize+1; b++) {
			float temp = exp(-((float)(a*a+b*b) / (2*sigma*sigma)));
			sum += temp;
			mask[a+maskSize+(b+maskSize)*(maskSize*2+1)] = temp;
		}
	}
	// Normalize the mask
	for(int i = 0; i < (maskSize*2+1)*(maskSize*2+1); i++)
		mask[i] = mask[i] / sum;
	
	*maskSizePointer = maskSize;
	
	return mask;
}

class GaussianBlurApp : public App {
  public:
	void setup() override;
	void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;
	
	static void contextInfo( const char *info, const void *private_info, ::size_t cb, void *user_data) { CI_LOG_E( info ); }
	static void profileEventCallback( cl_event event, cl_int event_command_exec_status, void *user_data )
	{
		auto gaussianApp = static_cast<GaussianBlurApp*>(user_data);
		auto & profilingEvents = gaussianApp->mProfilingEvents;
		auto currentSigma = gaussianApp->mSigma;
		for( auto profilingEventIt = profilingEvents.begin(); profilingEventIt != profilingEvents.end(); ++profilingEventIt ) {
			if( (*profilingEventIt)() == event ) {
				auto start = profilingEventIt->getProfilingInfo<CL_PROFILING_COMMAND_START>();
				auto end = profilingEventIt->getProfilingInfo<CL_PROFILING_COMMAND_END>();
				auto timeElapsed = double(end - start) / 1000000000.0;
				CI_LOG_I("Current Sigma (" << currentSigma << ") took " << timeElapsed << " seconds to render or " << 1.0 / timeElapsed << " frames/second.");
				gaussianApp->mCurrentOpPerf = timeElapsed;
				profilingEvents.erase( profilingEventIt );
				return;
			}
		}
	}
	
	void setupCl();
	void setupGlTextureClImages();
	void setupNaive();
	void setupParams();
	
	cl::Platform		mPlatform;
	cl::Context			mContext;
	cl::CommandQueue	mCommandQueue;
	cl::Kernel			mGaussianKernel;
	cl::Buffer			mMask;
	cl::Image2D			mClImage;
	cl::ImageGL			mClInteropResult;
	
	gl::Texture2dRef	mGlImageResult;
	
	params::InterfaceGlRef mParams;
	
	ivec2				mImageSize;
	float				mSigma;
	float				mCurrentOpPerf;
	bool				mSigmaUpdated;
	
	std::vector<cl::Event> mProfilingEvents;
};

void GaussianBlurApp::setup()
{
	mCurrentOpPerf = 0.0f;
	mSigma = 2.0f;
	mSigmaUpdated = true;
	
	
	setupCl();

	setupGlTextureClImages();
	
	setupNaive();
}

void GaussianBlurApp::setupParams()
{
	mParams = params::InterfaceGl::create( "Gaussian Blur", ivec2( 200, 200 ) );
	mParams->addParam( "Sigma", &mSigma ).updateFn( [&](){ mSigmaUpdated = true; } );
	mParams->addParam( "Cl Operation Performance", &mCurrentOpPerf );
}

void GaussianBlurApp::setupCl()
{
	// Get all of the platforms on this system
	std::vector<cl::Platform> platforms;
	cl::Platform::get( &platforms );
	// Assign the platform that we need
	mPlatform = platforms[0];
	
	// Get the GPU devices from the platform
	std::vector<cl::Device> devices;
	mPlatform.getDevices( CL_DEVICE_TYPE_GPU, &devices );
	// Create an OpenCL context on first available platform
	mContext = cl::Context( devices[0],
						   getDefaultSharedGraphicsContextProperties(),
						   &GaussianBlurApp::contextInfo );
	
	// Create a command-queue on the on the created context and allow for profiling
	mCommandQueue = cl::CommandQueue( mContext, CL_QUEUE_PROFILING_ENABLE );
}

void GaussianBlurApp::setupGlTextureClImages()
{
	// Load image
	auto surface = Surface32f::create( loadImage( loadAsset( "lena.jpg" ) ) );
	mImageSize = surface->getSize();
	
	getWindow()->setSize( mImageSize );
	
	// Create an OpenCL Image / texture and transfer data to the device
	// This cl image will hold the surface data as constant.
	mClImage = cl::Image2D( mContext,
						   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
						   cl::ImageFormat(CL_RGB, CL_FLOAT),
						   mImageSize.x, mImageSize.y,
						   0, (void*)surface->getData());
	
	// Allocate an empty texture which will be the placeholder for the result.
	mGlImageResult = gl::Texture2d::create( mImageSize.x, mImageSize.y,
										   gl::Texture2d::Format().internalFormat( GL_RGB32F ) );
	
	// Create a buffer for the result
	mClInteropResult = cl::ImageGL( mContext,
								   CL_MEM_WRITE_ONLY,
								   mGlImageResult->getTarget(),
								   0, mGlImageResult->getId() );
}

void GaussianBlurApp::setupNaive()
{
	// Create Gaussian mask
	int maskSize;
	float * mask = createBlurMask( mSigma, &maskSize );
	
	// Create buffer for mask and transfer it to the device
	mMask = cl::Buffer( mContext,
					   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
					   sizeof(float)*(maskSize*2+1)*(maskSize*2+1),
					   mask );
	
	// Compile OpenCL code
	cl::Program program = cl::Program( mContext, loadString( loadAsset( "gaussian_blur.cl" ) ), true );
	
	// Run Gaussian kernel
	mGaussianKernel = cl::Kernel(program, "gaussian_blur");
	mGaussianKernel.setArg(0, mClImage);
	mGaussianKernel.setArg(1, mMask);
	mGaussianKernel.setArg(2, mClInteropResult);
	mGaussianKernel.setArg(3, maskSize);
}

void GaussianBlurApp::keyDown( KeyEvent event )
{
	switch ( event.getCode() ) {
		case KeyEvent::KEY_UP: {
			mSigma++;
		}
		break;
		case KeyEvent::KEY_DOWN: {
			mSigma--;
		}
		break;
		default:
		break;
	}
	// reset with new sigma
	int maskSize;
	float * mask = createBlurMask( mSigma, &maskSize );
	mMask = cl::Buffer( mContext,
					   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
					   sizeof(float)*(maskSize*2+1)*(maskSize*2+1),
					   mask );
	mGaussianKernel.setArg( 1, mMask );
	mGaussianKernel.setArg( 3, maskSize );
	mSigmaUpdated = true;
}

void GaussianBlurApp::update()
{
	if( mSigmaUpdated ) {
		std::vector<cl::Memory> memory = { mClInteropResult };
		mCommandQueue.enqueueAcquireGLObjects( &memory );
		cl::Event perfEvent;
		mCommandQueue.enqueueNDRangeKernel( mGaussianKernel,
										   cl::NullRange,
										   cl::NDRange( mImageSize.x, mImageSize.y ),
										   cl::NullRange,
										   nullptr,
										   &perfEvent );
		perfEvent.setCallback( CL_COMPLETE, &GaussianBlurApp::profileEventCallback, this );
		mProfilingEvents.push_back( std::move( perfEvent ) );
		mCommandQueue.enqueueReleaseGLObjects( &memory );
		mSigmaUpdated = false;
	}
}

void GaussianBlurApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::setMatricesWindow( getWindowSize(), false );
	gl::draw( mGlImageResult );
	getWindow()->setTitle( to_string( getAverageFps() ) );
}

CINDER_APP( GaussianBlurApp, RendererGl )
