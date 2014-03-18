#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "BufferObj.h"
#include "Platform.h"
#include "Device.h"
#include "Context.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class ImplProjectApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	
	cl::PlatformRef mClPlatform;
	cl::ContextRef mContext;
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
