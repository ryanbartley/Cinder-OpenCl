#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "BufferObj.h"
#include "Platform.h"
#include "Device.h"
#include "Context.h"
#include "Program.h"
#include "CommandQueue.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void CL_CALLBACK read_complete(cl_event e, cl_int status, void* data) {
	float *float_data = (float*)data;
	printf("New data: %4.2f, %4.2f, %4.2f, %4.2f\n",
		   float_data[0], float_data[1], float_data[2], float_data[3]);
}

class EventApp : public AppNative {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	
	cl::PlatformRef		mPlatform;
	cl::ContextRef		mContext;
	cl::CommandQueueRef mCommandQueue;
	cl::ProgramRef		mProgram;
	cl::Event			userEvent, kernel_event, read_event;
};

void EventApp::setup()
{
	// First, select an OpenCL platform to run on.
	mPlatform = cl::Platform::create( cl::Platform::getAvailablePlatforms()[0], true );
	
    // Next, create an OpenCL context on the selected platform.
	// And authorize creation of the sharing context
	// The true tells Context to create a sharing context and
	// then caches the device associated with gl
    mContext = cl::Context::create( mPlatform, true );
	
    // Create a command-queue on the first device available
    // on the created context
    mCommandQueue = cl::CommandQueue::create( mContext->getAssociatedDevices()[0] );
	
	// This creates a user event by passing in context
	userEvent = cl::Event( mContext );
	
	err = clEnqueueTask(queue, kernel, 1, &user_event, &kernel_event);
}

void EventApp::mouseDown( MouseEvent event )
{
}

void EventApp::update()
{
}

void EventApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( EventApp, RendererGl )
