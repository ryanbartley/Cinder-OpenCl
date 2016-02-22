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
 Erik Smistad
 Portions Copyright (c) 2012
 Distributed under BSD-License
 https://github.com/orangeduck/Corange
 */

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Rand.h"
#include "cinder/Log.h"
#include "cinder/CameraUi.h"
#include "cinder/ObjLoader.h"

#include "Cinder-OpenCL.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const int VOLUME_WIDTH			= 64;
const int VOLUME_HEIGHT			= 64;
const int VOLUME_DEPTH			= 64;
const size_t VOLUME_SIZE		= VOLUME_WIDTH * VOLUME_HEIGHT * VOLUME_DEPTH;

const int MAX_MARCHING_VERTS	= 100000;
const int NUM_PARTICLES			= 200;

class MetaBallsApp : public App {
  public:
	void setup();
	void update();
	void draw();
	
	void setupCl();
	void setupScene();
	
	static void contextCallback( const char *errinfo, const void *private_info,
								::size_t cb, void *user_data)
	{ CI_LOG_E( errinfo ); }
	
	cl::Context			mClContext;
	cl::CommandQueue	mClCommandQueue;
	
	// ---------------------------------------------
	// Particles
	void setupParticleBuffers();
	void setupParticleKernel();
	void updateParticles();
	void drawParticleDebug();
	cl::Kernel					mClParticleUpdate;
	cl::BufferGL				mClParticleBuf;
	ci::gl::VboRef				mGlParticleBuf;
	ci::gl::VaoRef				mGlParticleDebugVao;
	
	//----------------------------------------------
	// Marching Cubes
	void setupMarchingBuffers();
	void setupMarchingKernels();
	void updateMarching();
	void drawMarching();
	void drawMarchingDebug();
	
	ci::gl::BatchRef		mGlMarchingRenderBatch, mGlMarchingShadowBatch, mGlMarchingDebugBatch;
	cl::BufferGL			mClMarchingRenderBuffer, mClMarchingDebugBuffer;
	cl::Buffer				mClMarchingVolume, mClVertIndex;
	cl::Kernel				mKernGenNormals, mKernGenNormalsSmooth,
							mKernWriteMetaballs, mKernWriteClear,
							mKernWritePointColorBack, mKernConstructSurface;
	int32_t					mMarchingVertsWritten;
	//----------------------------------------------
	// Podium
	void setupPodium();
	void drawPodium();
	ci::gl::BatchRef		mPodium;
	ci::gl::Texture2dRef	mSpecularMap, mDiffuseMap, mNormalMap;
	
	ci::CameraPersp			mCam;
	ci::CameraUi			mCamUI;
	ci::gl::Texture2dRef	mEnvironmentMap;
	
	bool					mDebugDraw;
};

void MetaBallsApp::setupCl()
{
	// This is all the same as within the HelloWorld sample. It also features the shared context
	// properties from the OpenGLInterop sample.
	std::vector<ocl::Platform> platforms;
	ocl::Platform::get( &platforms );
	
	std::vector<ocl::Device> devices;
	platforms[0].getDevices( CL_DEVICE_TYPE_GPU, &devices );
	
	mClContext = ocl::Context( devices,
							  ocl::getDefaultSharedGraphicsContextProperties( platforms[0] ),
							  &MetaBallsApp::contextCallback );
	
	mClCommandQueue = ocl::CommandQueue( mClContext, CL_QUEUE_PROFILING_ENABLE );
}

void MetaBallsApp::setupScene()
{
	setupParticleBuffers();
	setupParticleKernel();
	setupMarchingBuffers();
	setupMarchingKernels();
	setupPodium();
}

void MetaBallsApp::setupParticleBuffers()
{
	struct Particle {
		cl_float4 pos;
		cl_float4 vel;
		cl_float4 rand_life;
	};
	
	std::array<Particle, NUM_PARTICLES> particleData;
	for(int i = 0; i < NUM_PARTICLES; i++) {
		particleData[i].pos = ocl::toCl( vec4( vec3( 0.0f ), 1.0f ) );
		particleData[i].vel = ocl::toCl( vec4( 0.0f ) );
		auto randomVec = vec3( Rand::randFloat( - 1, 1 ), Rand::randFloat( 0, 2 ) + 0.5f, Rand::randFloat( - 1, 1 ) );
		vec3 rand = normalize( randomVec ) * ( Rand::randFloat( - 1, 1 ) * 2 );
		particleData[i].rand_life = ocl::toCl( vec4( rand, 999.0f ) );
	}
	
	mGlParticleBuf = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(Particle) * NUM_PARTICLES,
										   particleData.data(), GL_STATIC_DRAW );
	mClParticleBuf = ocl::BufferGL( mClContext, CL_MEM_READ_WRITE, mGlParticleBuf->getId() );
	
	// Setup a vao for debug.
	mGlParticleDebugVao = gl::Vao::create();
	gl::ScopedVao scopeVao( mGlParticleDebugVao );
	gl::ScopedBuffer scopeBuffer( mGlParticleBuf );
	gl::enableVertexAttribArray( 0 );
	gl::vertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
	gl::enableVertexAttribArray( 1 );
	gl::vertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );
}

void MetaBallsApp::setupParticleKernel()
{
	auto program = ocl::createProgram( mClContext, loadAsset( "kernels/particles.cl" ), true );
	mClParticleUpdate = ocl::Kernel( program, "particle_update" );
	
	float maxLife = 60.0;
	float minVelSqd = 0.5 * 0.5;
	
	mClParticleUpdate.setArg( 0, mClParticleBuf );
	mClParticleUpdate.setArg( 1, sizeof(float), &maxLife );
	mClParticleUpdate.setArg( 2, sizeof(float), &minVelSqd );
	mClParticleUpdate.setArg( 5, sizeof(cl_int), &NUM_PARTICLES );
}

void MetaBallsApp::setupMarchingBuffers()
{
	struct MarchingVert {
		cl_float4 pos;
		cl_float4 norm;
	};
	
	// Create Buffer.
	auto marchingVbo = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(MarchingVert) * MAX_MARCHING_VERTS, nullptr, GL_STATIC_DRAW );
	geom::BufferLayout vertBufferLayout( {
		geom::AttribInfo( geom::POSITION, geom::FLOAT, 4, sizeof(MarchingVert), offsetof(MarchingVert, pos) ),
		geom::AttribInfo( geom::NORMAL, geom::FLOAT, 4, sizeof(MarchingVert), offsetof(MarchingVert, norm) )
	});
	auto glsl = gl::GlslProg::create( loadAsset( "phong.vert" ), loadAsset( "phong.frag" ) );
	auto vboMesh = gl::VboMesh::create( MAX_MARCHING_VERTS, GL_TRIANGLES, { { vertBufferLayout, marchingVbo } } );
	mGlMarchingRenderBatch = gl::Batch::create( vboMesh, glsl );
	
	struct MarchingVolume {
		cl_float4 pos_vol;
	};
	std::array<MarchingVolume, VOLUME_SIZE> volumeData;
	
	int x, y, z;
	for(x = 0; x < VOLUME_WIDTH; x++)
		for(y = 0; y < VOLUME_HEIGHT; y++)
			for(z = 0; z < VOLUME_DEPTH; z++) {
				int id = x + y * VOLUME_WIDTH + z * VOLUME_WIDTH * VOLUME_HEIGHT;
				volumeData[id].pos_vol = ocl::toCl( vec4( x, y, z, 0.0f ) );
			}
	
	// Create Position Indices Buffer
	auto debugVbo = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(MarchingVolume) * VOLUME_SIZE, volumeData.data(), GL_STATIC_DRAW);
	geom::BufferLayout volumeLayout( { geom::AttribInfo( geom::POSITION, geom::FLOAT, 4, 0, 0 ) } );
	glsl = gl::GlslProg::create( loadAsset( "basic.vert" ), loadAsset( "basic.frag" ) );
	vboMesh = gl::VboMesh::create( VOLUME_SIZE, GL_POINTS, { { volumeLayout, debugVbo } } );
	mGlMarchingDebugBatch = gl::Batch::create( vboMesh, glsl );
	
	mClMarchingDebugBuffer = cl::BufferGL( mClContext, CL_MEM_READ_WRITE, debugVbo->getId() );
	mClMarchingRenderBuffer = cl::BufferGL( mClContext, CL_MEM_READ_WRITE, marchingVbo->getId() );
	mClMarchingVolume =	cl::Buffer( mClContext, CL_MEM_READ_WRITE, sizeof(float) * VOLUME_SIZE, nullptr );
	mClVertIndex = cl::Buffer( mClContext, CL_MEM_READ_WRITE, sizeof(int), nullptr );
	
	mEnvironmentMap = gl::Texture2d::createFromDds( loadAsset( "metaballs_env.dds" ) );
}

void MetaBallsApp::setupMarchingKernels()
{
	static const cl_int3 size{ VOLUME_WIDTH, VOLUME_HEIGHT, VOLUME_DEPTH };
	auto program = ocl::createProgram( mClContext, loadAsset( "kernels/marching_cubes.cl" ), false );
	auto path = "-I" + getAssetPath( "" ).string();
	program.build( path.c_str() );
	
	mKernWriteMetaballs = cl::Kernel( program, "write_metaballs" );
	mKernWriteMetaballs.setArg( 0, mClMarchingVolume );
	mKernWriteMetaballs.setArg( 1, mClParticleBuf );
	mKernWriteMetaballs.setArg( 2, sizeof(cl_int3), &size );
	mKernWriteMetaballs.setArg( 3, sizeof(cl_int), &NUM_PARTICLES );
	
	mKernWriteClear = cl::Kernel( program, "write_clear" );
	mKernWriteClear.setArg( 0, mClMarchingVolume );
	
	mKernWritePointColorBack = cl::Kernel( program, "write_point_color_back" );
	mKernWritePointColorBack.setArg( 0, mClMarchingVolume );
	mKernWritePointColorBack.setArg( 1, mClMarchingDebugBuffer );
	
	mKernConstructSurface = cl::Kernel( program, "construct_surface" );
	mKernConstructSurface.setArg( 0, mClMarchingVolume );
	mKernConstructSurface.setArg( 1, sizeof(cl_int3), &size );
	mKernConstructSurface.setArg( 2, mClMarchingRenderBuffer );
	mKernConstructSurface.setArg( 3, mClVertIndex );
	
	mKernGenNormals = cl::Kernel( program, "generate_flat_normals" );
	mKernGenNormals.setArg( 0, mClMarchingRenderBuffer );
	
	mKernGenNormalsSmooth = cl::Kernel( program, "generate_smooth_normals" );
	mKernGenNormalsSmooth.setArg( 0, mClMarchingRenderBuffer );
	mKernGenNormalsSmooth.setArg( 1, mClParticleBuf );
	mKernGenNormalsSmooth.setArg( 2, sizeof(cl_int), &NUM_PARTICLES );
}

void MetaBallsApp::setupPodium()
{
	auto glsl = gl::GlslProg::create( gl::GlslProg::Format()
									 .vertex( loadAsset( "podium/normal_mapping_vert.glsl" ) )
									 .fragment( loadAsset( "podium/normal_mapping_frag.glsl" ) ) );
	glsl->uniform( "uDiffuseMap", 0 );
	glsl->uniform( "uSpecularMap", 1 );
	glsl->uniform( "uNormalMap", 2 );
	glsl->uniform( "uLights[0].diffuse", vec4( .8, .8, .8, 1 ) );
	glsl->uniform( "uLights[0].specular", vec4( 1, 1, 1, 1 ) );
	glsl->uniform( "uLights[0].position", vec4( 32, 15, 32, 1 ) );
	glsl->uniform( "uNumOfLights", 1 );
	
	auto trimesh = TriMesh::create( ObjLoader( loadAsset( "podium/podium.obj" ) ) );
	trimesh->recalculateTangents();
	mPodium = gl::Batch::create( *trimesh, glsl );
	
	mSpecularMap = gl::Texture2d::createFromDds( loadAsset( "podium/podium_s.dds" ) );
	mDiffuseMap = gl::Texture2d::createFromDds( loadAsset( "podium/podium.dds" ) );
	mNormalMap = gl::Texture2d::createFromDds( loadAsset( "podium/podium_nm.dds" ) );
}

void MetaBallsApp::updateParticles()
{
	int random = rand();
	float time = 1.0f / 60.0f;
	
	mClParticleUpdate.setArg( 3, sizeof(float), &time );
	mClParticleUpdate.setArg( 4, sizeof(int32_t), &random );
	
	// Queue the kernel up for execution across the array
	mClCommandQueue.enqueueNDRangeKernel( mClParticleUpdate,
										 cl::NullRange,
										 cl::NDRange( NUM_PARTICLES ) );
}

void MetaBallsApp::updateMarching()
{
	static const cl_int3 size{ VOLUME_WIDTH, VOLUME_HEIGHT, VOLUME_DEPTH };
	mClCommandQueue.enqueueNDRangeKernel( mKernWriteClear,
										  cl::NullRange,
										  cl::NDRange( VOLUME_SIZE ) );
	/* Update volumes */
	mClCommandQueue.enqueueNDRangeKernel( mKernWriteMetaballs,
										  cl::NullRange,
										  cl::NDRange(  VOLUME_SIZE ) );
	/* End */
	int zero = 0;
	auto kernelRange = (size.s[0]-1) * (size.s[1]-1) * (size.s[2]-1);
	mClCommandQueue.enqueueWriteBuffer( mClVertIndex, true, 0, sizeof(int), &zero );
	mClCommandQueue.enqueueNDRangeKernel( mKernConstructSurface,
										  cl::NullRange,
										  cl::NDRange( kernelRange ) );
	mClCommandQueue.enqueueReadBuffer( mClVertIndex,
									  true, 0, sizeof(cl_int),
									  &mMarchingVertsWritten );
	
	/* Generate Normals */
	if (mMarchingVertsWritten > 0) {
		bool smooth = true;
		if( ! smooth )
			mClCommandQueue.enqueueNDRangeKernel( mKernGenNormals,
												  cl::NullRange,
												  cl::NDRange( mMarchingVertsWritten ) );
		else
			mClCommandQueue.enqueueNDRangeKernel( mKernGenNormalsSmooth,
												  cl::NullRange,
												  cl::NDRange( mMarchingVertsWritten ) );
	}
	
	//if( mDebugDraw )
		mClCommandQueue.enqueueNDRangeKernel( mKernWritePointColorBack,
											  cl::NullRange,
											  cl::NDRange( VOLUME_SIZE ) );
}


void MetaBallsApp::drawPodium()
{	
	gl::ScopedTextureBind scopeTex0( mDiffuseMap, 0 );
	gl::ScopedTextureBind scopeTex1( mSpecularMap, 1 );
	gl::ScopedTextureBind scopeTex2( mNormalMap, 2 );
	
	mPodium->draw();
}

void MetaBallsApp::drawParticleDebug()
{
	gl::ScopedVao scopeVao( mGlParticleDebugVao );
	gl::ScopedGlslProg scopeGlsl( gl::getStockShader( gl::ShaderDef().color() ) );
	gl::setDefaultShaderVars();
	gl::drawArrays( GL_POINTS, 0, NUM_PARTICLES );
}

void MetaBallsApp::drawMarching()
{
	// add materials and light colors here
	// also add an environment map
	// also add shadow map and render to shadow below
	mGlMarchingRenderBatch->draw( 0, mMarchingVertsWritten );
}

void MetaBallsApp::drawMarchingDebug()
{
	mGlMarchingDebugBatch->draw();
}

void MetaBallsApp::setup()
{
	try {
		setupCl();
		setupScene();
	}
	catch( const ocl::Error &e ) {
		CI_LOG_E( e.what() << " " << ocl::errorToString( e.err() ) );
	}
	
	mCam.setPerspective( 60, ci::app::getWindowAspectRatio(), 0.01, 1000 );
	mCam.lookAt( vec3( 10, 10, 10 ), vec3( 0, 0, 0 ) );
	
	mCamUI.setCamera( &mCam );
	mCamUI.connect( getWindow() );
}

void MetaBallsApp::update()
{
	std::vector<cl::Memory> acquire( { mClParticleBuf, mClMarchingRenderBuffer, mClMarchingDebugBuffer } );
	mClCommandQueue.enqueueAcquireGLObjects( &acquire );
	updateParticles();
	updateMarching();
	mClCommandQueue.enqueueReleaseGLObjects( &acquire );
}

void MetaBallsApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	
	gl::setMatrices( mCam );
	gl::enableDepthRead();
	gl::enableDepthWrite();
	drawPodium();
	drawMarching();
	
	gl::disableDepthWrite();
	gl::disableDepthRead();
	gl::ScopedState	stateScope( GL_PROGRAM_POINT_SIZE, true );
	drawParticleDebug();
	drawMarchingDebug();
}

CINDER_APP( MetaBallsApp, RendererGl, []( App::Settings *settings ) {
#if defined( CINDER_MSW )
	settings->setConsoleWindowEnabled();
#endif
})
