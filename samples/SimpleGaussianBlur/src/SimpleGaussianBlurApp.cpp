#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

#include "Cinder-OpenCL.h"



using namespace ci;
using namespace ci::app;
using namespace std;

class SimpleGaussianBlurApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	
	static void contextErrorCallback( const char *info, const void *private_info,
									 ::size_t cb, void *user_data)
	{ CI_LOG_E( info ); }
	
	void setupCl();
	void setupGlTextureClImages();
	
	ocl::Context		mClContext;
	ocl::CommandQueue	mClCommandQueue;
	ocl::Image2D		mClImage;
	ocl::Buffer			mClBuffer;
	gl::Texture2dRef	mGlTexture;
};

void SimpleGaussianBlurApp::setup()
{
	setupCl();
	setupGlTextureClImages();
}

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
	
	mClCommandQueue = ocl::CommandQueue( mClContext );
}

void SimpleGaussianBlurApp::setupGlTextureClImages()
{
	try {
		// Load image
		ocl::printSupportedImageFormats( mClContext, CL_MEM_OBJECT_IMAGE2D );
		auto imageSource = loadImage( loadAsset( "sunset.jpg" ) );
		mClImage = ocl::createImage2D( imageSource, mClContext );
		mClBuffer = ocl::createBuffer( imageSource, mClContext );
		auto source = ocl::createSource( mClImage );
		mGlTexture = gl::Texture2d::create( source );
		writeImage( get, source, ImageTarget::Options(), "png" );
//		auto surface = Surface32f::create( loadImage( loadAsset( "lena.jpg" ) ), SurfaceConstraintsDefault(), true );
//		mImageSize = surface->getSize();
//		
//		getWindow()->setSize( mImageSize );
//		
//		ocl::printSupportedImageFormats( mClContext, CL_MEM_OBJECT_IMAGE2D );
//		
//		// Create an OpenCL Image / texture and transfer data to the device
//		// This cl image will hold the surface data as constant.
//		mClImage = cl::Image2D( mClContext,
//							   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
//							   cl::ImageFormat( CL_RGBA, CL_FLOAT ),
//							   mImageSize.x, mImageSize.y,
//							   0, (void*) surface->getData() );
//		
//		// Allocate an empty texture which will be the placeholder for the result.
//		mGlImageResult = gl::Texture2d::create( mImageSize.x, mImageSize.y,
//											   gl::Texture2d::Format().internalFormat( GL_RGBA8 ) );
//		
//		// Create a buffer for the result
//		mClInteropResult = cl::ImageGL( mContext,
//									   CL_MEM_WRITE_ONLY,
//									   mGlImageResult->getTarget(),
//									   0, mGlImageResult->getId() );
	}
	catch( const cl::Error &e ) {
		CI_LOG_E( e.what() + ocl::errorToString( e.err() ) );
	}
}

void SimpleGaussianBlurApp::mouseDown( MouseEvent event )
{
}

void SimpleGaussianBlurApp::update()
{
}

void SimpleGaussianBlurApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::setMatricesWindow( getWindowSize() );
	gl::draw( mGlTexture );
}

CINDER_APP( SimpleGaussianBlurApp, RendererGl )
