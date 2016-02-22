/*
 Copyright (c) 2016, The Cinder Project, All rights reserved.
 
 This code is intended for use with the Cinder C++ library: http://libcinder.org
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

/*
 Sample from page 46 of the OpenCL Programming Guide
 Aaftab Munshi 
 Benedict R. Gaster
 Timothy G. Mattson
 James Fung
 Dan Ginsburg
 Portions Copyright (c) 2012
 */

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"

#include "Cinder-OpenCL.h"

// This example will take you through most of the boiler plate code
// that OpenCL requires to make a cohesive project. There'll be some
// explanation and examples. This is a very simple project. Much of
// the stuff discussed in here will be used elsewhere with little to
// no explanation. There are also a number of online resources to
// check out too.

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
		CI_LOG_E( errinfo );
	}
	
	ocl::Platform				mClPlatform;
	ocl::Context				mContext;
	ocl::Program				mProgram;
	ocl::CommandQueue			mCommandQueue;
	std::array<ocl::Buffer, 3>	mMemObjects;
};

void HelloWorldApp::setup()
{
	// Query your system for the OpenCL Platforms installed on it.
	std::vector<ocl::Platform> platforms;
	ocl::Platform::get( &platforms );
	// Assign the platform that we need, this will most likely be the
	// first entry. However, you could have a number of different
	// platforms, i.e. NVidia, AMD, Intel. This is where you'd select
	// the correct platform.
	mClPlatform = platforms[0];
	
	// Here we'll just print all the common information for each platform,
	// that's on your system.
	for( auto & platform : platforms ){
		console() << platform << endl;
	}
	
	// Now we'll select the device we want to use from that platform from
	// the chosen platform. This query could also come back with multiple
	// devices depending on your system.
	std::vector<ocl::Device> devices;
	mClPlatform.getDevices( CL_DEVICE_TYPE_GPU, &devices );
	// Print out the general information for all devices connected.
	for( auto & device : devices ) {
		console() << device << endl;
	}
	
	// Now we'll create an OpenCL Context with our chosen devices. This
	// is the central hub for all of the processing that you'll be doing
	// with OpenCL. It will hold one or more Command Queues, which will
	// in turn hold all of your resources. For our purposes we're going
	// to use the default properties, seen by the nullptr in the second
	// parameter, but this will be where you'll tell OpenCL about any
	// OpenGL Contexts that you'd like to share with OpenCL later.
	mContext = ocl::Context( devices, nullptr, &HelloWorldApp::contextErrorCallback );

    // Now we'll create a Command Queue on this Context that will be the
	// central hub of execution. You can create many Command Queues and
	// share the resources and execute on many different branches. Here,
	// we're just creating one from our Context and it will use the devices
	// that our Context is using.
	mCommandQueue = ocl::CommandQueue( mContext );
	
	// The above setup is pretty much the same for all OpenCL programs, with
	// slight variations based upon exactly what you're looking to acheive.
	// But Basically, the above lines will be copied and pasted for all of
	// your OpenCL programs.
	
	// Now we'll get into some simple resource acquisition and some simple execution.
	// Most of the next few lines don't matter about order like the above lines do.
	// You'll see it's very similar to the way you setup resources in opengl.

    // First, in no particular order we'll set up our programs. Each program could have
	// multiple "kernels" as you'll see. Each kernel acts like a specific line of
	// execution. This is somewhat different from OpenGL, in the sense that you usually
	// need two seperate files, one vertex and one fragment shader, to create a GlslProg.
	// With ocl::Program, you just need a file that has any number of functions defined
	// with '__kernel' reserved word. The last argument specifies if we want to build
	// the program, which we don't.
	mProgram = ocl::createProgram( mContext, loadAsset( "HelloWorld.cl" ), false );
	// This will print out the source code of the program.
	console() << mProgram << endl;
	
	// Now we're going to build the program. We can pass in many options to build the
	// program. We'll see many of them later on. For now we just want it to build with
	// kernel arg info. This is so that later on when we query the kernel, it'll be
	// populated with info about each kernel arg, namely the name and the type. Note:
	// this is only necessary when querying the kernel as you'll see below.
	mProgram.build("-cl-kernel-arg-info");

	// Here we'll extract all of the kernels from this Program. In this case there'll
	// be only one but you can create as many kernels as exist in your program.
	std::vector<ocl::Kernel> kernels;
	mProgram.createKernels( &kernels );
	
	// We can print out some useful information about the kernels created here. This
	// is where -cl-kernel-arg-info comes in handy. If you comment out the above option.
	// you'll see that we can't get name or type info from the arguments.
	for( auto & kernel : kernels ) {
		console() << kernel << std::endl;
	}
	console() << endl;
	
	// Now we'll create some buffers to execute our kernel on.
	std::array<float, ARRAY_SIZE> a, b, result;
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
    }
	
	// As you'll see these buffers are very similar to gl::BufferObj's or Vbo's. First arg
	// is the context that the buffer will be created on. Next are some flags. MEM_READ_ONLY,
	// is like it sounds. We're telling the GPU that we are not going to write to this buffer,
	// only read. The COPY_HOST_PTR flag tells OpenCL that we'd like it to copy the data from
	// the supplied pointer. And MEM_READ_WRITE is again how it sounds. We're telling OpenCL,
	// that we'll read from and write to this buffer. There's also MEM_WRITE_ONLY. The third
	// argument is the size of the buffer we're creating and the fourth is the pointer we'd
	// like OpenCL to copy from.
    mMemObjects[0] = ocl::Buffer( mContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * a.size(), a.data() );
	mMemObjects[1] = ocl::Buffer( mContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * b.size(), b.data() );
	mMemObjects[2] = ocl::Buffer( mContext, CL_MEM_READ_WRITE, sizeof(float) * ARRAY_SIZE );
	

	// Now that we have our buffer's and our kernel. We need to tell OpenCL where everything
	// gets connected to. In OpenGL terms this stage is kind of like the uniform setting and
	// BufferLayout stage. The main difference is that you just connect each piece of data,
	// be it buffer or uniform to each argument. In this case we have 3 Buffers and 3 arguments
	// so we connect them thusly...
	kernels[0].setArg( 0, mMemObjects[0] );
	kernels[0].setArg( 1, mMemObjects[1] );
	kernels[0].setArg( 2, mMemObjects[2] );

    // The way we execute the kernel is by queuing it in the Command Queue. We'll see
	// what each of these arguments means as we go through. If you want to know now,
	// take a look here at the spec...
	// https://www.khronos.org/registry/cl/sdk/1.2/docs/man/xhtml/clEnqueueNDRangeKernel.html
	mCommandQueue.enqueueNDRangeKernel( kernels[0],
									   ocl::NullRange,
									   ocl::NDRange( ARRAY_SIZE ),
									   ocl::NDRange( 1 ),
									   nullptr,
									   nullptr );
	
	// This function will allow us to read the buffer back to the CPU after we're done
	// executing the kernel above. We'll talk more about the arguments of this function
	// as well. If you want to know now, take a look here at the spec...
	// https://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clEnqueueReadBuffer.html
	mCommandQueue.enqueueReadBuffer( mMemObjects[2], true, 0, ARRAY_SIZE * sizeof(float), result.data() );
	
	
    // Output the result buffer
    for (int i = 0; i < ARRAY_SIZE; i++)
        console() << result[i] << " ";
	console() << std::endl << std::endl;
	// No problems everything executed successfully.
    console() << "Executed program succesfully." << std::endl << std::endl;
	// Done.
	quit();
}

CINDER_APP( HelloWorldApp, RendererGl )
