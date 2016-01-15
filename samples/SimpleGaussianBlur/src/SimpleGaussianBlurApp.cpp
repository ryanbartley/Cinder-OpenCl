#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

#include "Cinder-OpenCL.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// Event Profiling Callback. Explained below.
void CL_CALLBACK profileEventCallback( cl_event e, cl_int event_command_exec_status, void *user_data );

class SimpleGaussianBlurApp : public App {
  public:
	void setup() override;
	void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;
	
	static void contextErrorCallback( const char *info, const void *private_info,
									 ::size_t cb, void *user_data)
	{ CI_LOG_E( info ); }
	
	void setupCl();
	void setupGlTextureClImages();
	void setupKernel();
	void setupBlurMask();
	
	ocl::Context		mClContext;
	ocl::CommandQueue	mClCommandQueue;
	cl::Buffer			mMask;
	cl::Kernel			mNaiveKernel;
	
	ocl::Image2D		mClImage;
	ocl::ImageGL		mClInteropResult;
	gl::Texture2dRef	mGlTextureResult;
	
	glm::ivec2			mImageSize;
	int					mCurrentMaskSize;
	float				mSigma;
	bool				mSigmaUpdated;
};

void SimpleGaussianBlurApp::setupCl()
{
	// This is all the same as within the HelloWorld sample. It also features the shared context
	// properties from the OpenGLInterop sample.
	std::vector<ocl::Platform> platforms;
	ocl::Platform::get( &platforms );
	
	std::vector<ocl::Device> devices;
	platforms[0].getDevices( CL_DEVICE_TYPE_GPU, &devices );
	
	mClContext = ocl::Context( devices,
							  ocl::getDefaultSharedGraphicsContextProperties( platforms[0] ),
							  &SimpleGaussianBlurApp::contextErrorCallback );

	mClCommandQueue = ocl::CommandQueue( mClContext, CL_QUEUE_PROFILING_ENABLE );
}

void SimpleGaussianBlurApp::setupGlTextureClImages()
{
	try {
		ocl::printSupportedImageFormats( mClContext, CL_MEM_OBJECT_IMAGE2D );
		// Load image
		mClImage = ocl::createImage2D( loadImage( loadAsset( "sunset.jpg" ) ), mClContext );
		// Query image stats
		mImageSize.x = mClImage.getImageInfo<CL_IMAGE_WIDTH>();
		mImageSize.y = mClImage.getImageInfo<CL_IMAGE_HEIGHT>();
		
		// Allocate an empty texture which will be the placeholder for the result.
		mGlTextureResult = gl::Texture2d::create( mImageSize.x, mImageSize.y,
												 gl::Texture2d::Format().internalFormat( GL_RGB8 ) );
		
		// Create a shared image to bridge for the result.
		mClInteropResult = ocl::createImageGL( mGlTextureResult, mClContext, CL_MEM_WRITE_ONLY );
	}
	catch( const cl::Error &e ) {
		CI_LOG_E( e.what() + ocl::errorToString( e.err() ) );
	}
}

void SimpleGaussianBlurApp::setupKernel()
{
	// Compile OpenCL code
	cl::Program program = ocl::createProgram( mClContext, loadAsset( "gaussian_blur.cl" ), false );
	try {
		// Build it seperately
		program.build();
		
		// Gaussian kernel
		mNaiveKernel = cl::Kernel( program, "gaussian_blur" );
		mNaiveKernel.setArg( 0, mClImage );
		mNaiveKernel.setArg( 2, mClInteropResult );
	}
	catch( const cl::Error &e ) {
		auto devices = mClContext.getInfo<CL_CONTEXT_DEVICES>();
		auto buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
		CI_LOG_E("Function: " << e.what() << ", Error: " <<
				 ocl::errorToString( e.err() ) << " " << buildLog );
	}
	
	mSigma = 2.0f;
	mSigmaUpdated = true;
	
	setupBlurMask();
}

void SimpleGaussianBlurApp::setupBlurMask()
{
	if( mSigma <= 0.0f ) return;
	
	mCurrentMaskSize = ceil( 3.0f * mSigma );
	auto maskSize = ( mCurrentMaskSize * 2 + 1 ) * ( mCurrentMaskSize * 2 + 1 );
	
	auto mask = unique_ptr<float[]>( new float[maskSize] );
	
	float sum = 0.0f;
	for(int a = -mCurrentMaskSize; a < mCurrentMaskSize+1; a++) {
		for(int b = -mCurrentMaskSize; b < mCurrentMaskSize+1; b++) {
			float temp = exp( -( (float)( a * a + b * b ) / (2 * mSigma * mSigma ) ) );
			sum += temp;
			mask[a+mCurrentMaskSize+(b+mCurrentMaskSize)*(mCurrentMaskSize*2+1)] = temp;
		}
	}
	// Normalize the mask
	for( int i = 0; i < maskSize; i++ )
		mask[i] = mask[i] / sum;
	
	try {
		mMask = cl::Buffer( mClContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * maskSize, mask.get() );
	}
	catch( const cl::Error &e ) {
		CI_LOG_E( "Function: " << e.what() << ", Error: " << ocl::errorToString( e.err() ) );
	}
	
	mNaiveKernel.setArg( 1, mMask );
	mNaiveKernel.setArg( 3, mCurrentMaskSize );
	mSigmaUpdated = true;
}

void SimpleGaussianBlurApp::setup()
{
	setupCl();
	setupGlTextureClImages();
	setupKernel();
}

void SimpleGaussianBlurApp::keyDown( KeyEvent event )
{
	switch ( event.getCode() ) {
		case KeyEvent::KEY_UP: mSigma++; break;
		case KeyEvent::KEY_DOWN: mSigma--; break;
		default: break;
	}
	setupBlurMask();
}

void SimpleGaussianBlurApp::update()
{
	if( ! mSigmaUpdated ) return;
	
	cl::Event perfEvent;
	std::vector<cl::Memory> memory = { mClInteropResult };
	mClCommandQueue.enqueueAcquireGLObjects( &memory );
	mClCommandQueue.enqueueNDRangeKernel( mNaiveKernel,
										 cl::NullRange,
										 cl::NDRange( mImageSize.x, mImageSize.y ),
										 cl::NullRange,
										 nullptr,
										 &perfEvent );
	mClCommandQueue.enqueueReleaseGLObjects( &memory );
	perfEvent.setCallback( CL_COMPLETE, &profileEventCallback, nullptr );
	mSigmaUpdated = false;
}

void SimpleGaussianBlurApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::setMatricesWindow( getWindowSize(), false );
	gl::draw( mGlTextureResult );
}

void CL_CALLBACK profileEventCallback( cl_event e, cl_int event_command_exec_status, void *user_data )
{
	uint64_t start, end;
	cl_int err;
	err = clGetEventProfilingInfo( e, CL_PROFILING_COMMAND_START, sizeof(uint64_t), &start, nullptr );
	err = clGetEventProfilingInfo( e, CL_PROFILING_COMMAND_END, sizeof(uint64_t), &end, nullptr );
	
//	cl::Event event( e );
//	auto start = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
//	auto end = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();
	auto timeElapsed = (end - start) * 0.000000001;
	CI_LOG_I( "Elapsed Seconds of Execution: " << std::fixed << std::setprecision( 9 ) << timeElapsed );
}

CINDER_APP( SimpleGaussianBlurApp, RendererGl )
