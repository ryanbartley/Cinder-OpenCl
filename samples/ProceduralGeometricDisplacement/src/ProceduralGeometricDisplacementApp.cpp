#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Log.h"
#include "Cinder-OpenCL.h"
#include "cinder/gl/ConstantConversions.h"
#include "cinder/params/Params.h"
#include "CL\cl_gl_ext.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#pragma OPENCL EXTENSION cl_khr_gl_event : enable

int divide_up(int a, int b)
{
	return ((a % b) != 0) ? (a / b + 1) : (a / b);
}

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light {
	vec3 intensity;
	float ambientCoefficient;
	float attenuation;
};

class ProceduralGeometricDisplacementApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
	void update() override;
	void draw() override;
	
	void setupCl();
	void setupBuffers();
	void setupKernels();
	void createShadowFbo();
	void createCubeMap();
	
	void createParams();
	
	void compute();
	void renderRoom();
	void renderShadow();
	void renderSkyBox();
	
	static void contextErrorCallback( const char *errinfo,
									 const void *private_info,
									 ::size_t cb,
									 void *user_data)
	{
		cout << "ERROR: " << errinfo << endl;
	}
	
	CameraPersp		mCam, mLightCam;
	CameraUi		mCameraControl;
	
	gl::GlslProgRef		mShadowGlsl;
	gl::BatchRef		mSphereEnv, mSphereShadow, mSphereRoom,
						mSkyBatch, mFloorBatch, mFloorShadow,
						mWallBatch, mWallShadow;
	gl::FboRef			mShadowFbo;
	gl::Texture2dRef	mShadowTex;
	gl::TextureCubeMapRef mSkyBox;
	
	params::InterfaceGlRef mParams;
	
	cl::Context			mContext;
	cl::Kernel			mComputeKernel;
	cl::Program			mComputeProgram;
	cl::Device			mDevice;
	cl::CommandQueue	mQueue;
	cl::Buffer			mInputVertexBuffer;
	cl::BufferGL		mOutputVertexBuffer,
						mOutputNormalBuffer;
	
	bool		mRenderRoom			= true,
				mUseLightCam		= false,
				mRenderDepthTex		= false;
	uint32_t	mGroupSize			= 4;
	uint32_t	mMaxWorkGroupSize;
	size_t		mSphereNumVertices;
	vec2		mActualDim;
	
	float	mFrequency	= 1.0f;
	float	mAmplitude  = 0.35f;
	float	mOctaves    = 5.5f;
	float	mRoughness  = 0.025f;
	float	mLacunarity = 2.0f;
	float	mIncrement  = 1.5f;
	float	mPhase      = 0.0f;
	float	mPhaseIncrease = 0.01f;
	
	float	mDepthBias	= -0.0005f;
	float	mPolygonOffsetFactor;
	float	mPolygonOffsetUnits;
	
	Light		mLight;
	Material	mSphereMaterial, mRoomMaterial;
};

void ProceduralGeometricDisplacementApp::setup()
{
	mSphereMaterial.ambient = {0.1f, 0.1f, 0.1f};
	mSphereMaterial.diffuse = Color::hex( 0x014421 );
	mSphereMaterial.specular = {1.0f, 1.0f, 1.0f};
	mSphereMaterial.shininess = 128.0f;
	
	mRoomMaterial.ambient = {0.1f, 0.1f, 0.1f};
	mRoomMaterial.diffuse = {0.5f, 0.0f, 0.0f};
	mRoomMaterial.specular = {1.0f, 1.0f, 1.0f};
	mRoomMaterial.shininess = 0.0f;
	
	mLight.intensity = { 1.0f, 1.0f, 1.0f };
	mLight.ambientCoefficient = { 0.005f };
	mLight.attenuation = 0.1f;
	
	getWindow()->setSize( ivec2( 1024, 1024 ) );
	
	setupCl();
	setupBuffers();
	setupKernels();
	createShadowFbo();
	createCubeMap();
	createParams();
	
	mCam.lookAt( vec3( 0.0f, 5.0f, -7.0f ), vec3( 0 ) );
	mCam.setPerspective( 55.0f, getWindowAspectRatio(), 0.1f, 10000.0f );
	mLightCam.lookAt( vec3( -7.0f, 10.0f, -7.0f ), vec3( 0 ) );
	mLightCam.setPerspective( 55.0f, getWindowAspectRatio(), 0.01f, 1000000.0f );
	
	mCameraControl.setCamera( &mCam );
	
	mPolygonOffsetFactor = mPolygonOffsetUnits = 3.0f;
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void ProceduralGeometricDisplacementApp::setupCl()
{
	// Get all of the platforms on this system
	std::vector<cl::Platform> platforms;
	cl::Platform::get( &platforms );
	// Assign the platform that we need
	auto clPlatform = platforms[0];
	
	// Get the GPU devices from the platform
	std::vector<cl::Device> devices;
	clPlatform.getDevices( CL_DEVICE_TYPE_GPU, &devices );
	mDevice = devices[0];
	
	// Create an OpenCL context on first available platform
	mContext = cl::Context( mDevice,
						   getDefaultSharedGraphicsContextProperties(),
						   &ProceduralGeometricDisplacementApp::contextErrorCallback );
	
	// Create a command-queue on the first device available
	// on the created context
	mQueue = cl::CommandQueue( mContext );
}

void ProceduralGeometricDisplacementApp::setupBuffers()
{
	auto environmentMap = gl::GlslProg::create( loadAsset( "env_map.vert" ), loadAsset( "env_map.frag" ) );
	environmentMap->uniform( "uCubeMapTex", 0 );
	auto skyBoxGlsl		= gl::GlslProg::create( loadAsset( "sky_box.vert" ), loadAsset( "sky_box.frag" ) );
	skyBoxGlsl->uniform( "uCubeMapTex", 0 );
	
	mShadowGlsl	= gl::GlslProg::create( loadAsset( "shadow_mapping.vert"), loadAsset("shadow_mapping.frag") );
	mShadowGlsl->uniform( "uShadowMap", 0 );
	mShadowGlsl->uniform( "uDepthBias", mDepthBias );
	auto basicGlsl = gl::GlslProg::create( loadAsset( "basic.vert" ), loadAsset( "basic.frag" ) );
	
	// Create the room.
	auto floor = geom::Cube().size( vec3( 100, .5, 100 ) );
	mFloorBatch = gl::Batch::create( floor, mShadowGlsl );
	mFloorShadow = gl::Batch::create( floor, basicGlsl );
	auto backWall = geom::Cube().size( vec3( 100, 100, .5f ) );
	mWallBatch = gl::Batch::create( backWall, mShadowGlsl );
	mWallShadow = gl::Batch::create( backWall, basicGlsl );
	auto sky = geom::Cube().size( vec3( 500 ) );
	mSkyBatch = gl::Batch::create( sky, skyBoxGlsl );
	// Create the sphere
	auto sphere = geom::Sphere().subdivisions( 200 );
	auto sphereIndexTrimesh = TriMesh::create( sphere, TriMesh::Format().positions(4) );
	
	mSphereNumVertices = sphereIndexTrimesh->getNumVertices();
	
	size_t bytes = mSphereNumVertices * sizeof(vec4);

	// Create the buffers for the position and normals
	auto positionVbo	= gl::Vbo::create( GL_ARRAY_BUFFER, bytes, nullptr, GL_STATIC_DRAW );
	auto posAttrib		= geom::AttribInfo( geom::POSITION, geom::FLOAT, 4, 0, 0 );
	auto positionLayout = geom::BufferLayout( { posAttrib } );
	auto normalVbo		= gl::Vbo::create( GL_ARRAY_BUFFER, bytes, nullptr, GL_STATIC_DRAW );
	auto normalAttrib	= geom::AttribInfo( geom::NORMAL, geom::FLOAT, 4, 0, 0 );
	auto normalLayout	= geom::BufferLayout( { normalAttrib } );
	auto indicesVbo		= gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER,
										  sphereIndexTrimesh->getNumIndices() * sizeof(uint32_t),
										  sphereIndexTrimesh->getIndices().data(), GL_STATIC_DRAW );
	
	auto vboMesh		= gl::VboMesh::create( mSphereNumVertices, GL_TRIANGLES,
									   { { positionLayout, positionVbo },
										   { normalLayout, normalVbo } }, sphereIndexTrimesh->getNumIndices(), GL_UNSIGNED_INT, indicesVbo );

	mSphereEnv		= gl::Batch::create( vboMesh, environmentMap );
	mSphereRoom		= gl::Batch::create( vboMesh, mShadowGlsl );
	mSphereShadow	= gl::Batch::create( vboMesh, basicGlsl );
	
	mInputVertexBuffer = cl::Buffer( mContext, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
									bytes, sphereIndexTrimesh->getBufferPositions().data() );
	mOutputVertexBuffer = cl::BufferGL( mContext, CL_MEM_READ_WRITE, positionVbo->getId() );
	mOutputNormalBuffer = cl::BufferGL( mContext, CL_MEM_READ_WRITE, normalVbo->getId() );
}

void ProceduralGeometricDisplacementApp::setupKernels()
{
	// Create OpenCL program from HelloWorld.cl kernel source
	mComputeProgram = cl::Program( mContext, loadString( loadAsset( "displacement_kernel.cl" ) ), true );
	
	mComputeKernel = cl::Kernel( mComputeProgram, "displace" );
	mMaxWorkGroupSize = mComputeKernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>( mDevice );
}

void ProceduralGeometricDisplacementApp::createShadowFbo()
{
	gl::Texture2d::Format depthFormat;
	depthFormat.setInternalFormat( GL_DEPTH_COMPONENT32F );
	depthFormat.setMagFilter( GL_LINEAR );
	depthFormat.setMinFilter( GL_LINEAR );
	depthFormat.setWrap( GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER );
	depthFormat.setCompareMode( GL_COMPARE_REF_TO_TEXTURE );
	depthFormat.setCompareFunc( GL_LESS );
	mShadowTex = gl::Texture2d::create( 1024, 1024, depthFormat );
	gl::ScopedTextureBind scopeTex( mShadowTex, 0 );
	vec4 color = vec4( 1.0 );
	glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr( color ) );
	
	gl::Fbo::Format fboFormat;
	fboFormat.attachment( GL_DEPTH_ATTACHMENT, mShadowTex ).disableColor();
	mShadowFbo = gl::Fbo::create( 1024, 1024, fboFormat );
}

void ProceduralGeometricDisplacementApp::createCubeMap()
{
	mSkyBox = gl::TextureCubeMap::create( loadImage( loadAsset( "env_map.jpg" ) ), gl::TextureCubeMap::Format().mipmap() );
}

void ProceduralGeometricDisplacementApp::createParams()
{
	static int whichCameraControl = 0;
	static int whichCameraView = 0;
	static float sResetFrequency = mFrequency;
	static float sResetAmplitude = mAmplitude;
	static float sResetOctaves = mOctaves;
	static float sResetRoughness = mRoughness;
	static float sResetLacunarity = mLacunarity;
	static float sResetIncrement = mIncrement;
	static float sResetPhaseIncrease = mPhaseIncrease;
	mParams = params::InterfaceGl::create( "Compute", vec2( 200, 200 ) );
	mParams->addText( "Render" );
	mParams->addParam( "Render Room", &mRenderRoom );
	mParams->addParam( "Control Camera", { "Camera", "Light" }, &whichCameraControl )
	.updateFn( [&](){
		switch ( whichCameraControl ) {
			case 0:
				mCameraControl.setCamera( &mCam );
			break;
			case 1:
				mCameraControl.setCamera( &mLightCam );
			default:
			break;
		}
	} );
	mParams->addParam( "View Camera", { "Camera", "Light" }, &whichCameraView )
	.updateFn( [&](){
		switch ( whichCameraView ) {
			case 0:
				mUseLightCam = false;
				break;
			case 1:
				mUseLightCam = true;
			default:
				break;
		}
	} );
	mParams->addParam( "Render Depth Texture", &mRenderDepthTex );
	mParams->addParam( "Light Attenuation", &mLight.attenuation );
	mParams->addParam( "Ambient Coefficient", &mLight.ambientCoefficient );
	mParams->addParam( "Light Intensity", &mLight.intensity );
	mParams->addSeparator();
	mParams->addText( "Update" );
	mParams->addParam( "Frequency", &mFrequency ).min( 0.0f ).step( 0.01f );
	mParams->addParam( "Amplitude", &mAmplitude ).min( 0.0f ).max( 7.0f ).step( 0.01f );
	mParams->addParam( "Octaves", &mOctaves ).min( 0.0f ).max( 7.0f ).step( 0.01f );
	mParams->addParam( "Roughness", &mRoughness );
	mParams->addParam( "Lacunarity", &mLacunarity );
	mParams->addParam( "Increment", &mIncrement ).min( 0.01f ).step( 0.05f );
	mParams->addParam( "Phase Increase", &mPhaseIncrease ).min( 0.0f ).step( 0.01f );
	mParams->addButton( "Reset", [&]() {
		mFrequency = sResetFrequency;
		mAmplitude = sResetAmplitude;
		mOctaves = sResetOctaves;
		mRoughness = sResetRoughness;
		mLacunarity = sResetLacunarity;
		mIncrement = sResetIncrement;
		mPhaseIncrease = sResetPhaseIncrease;
	});
}

void ProceduralGeometricDisplacementApp::compute()
{
	std::vector<cl::Memory> aqcuire = { mOutputNormalBuffer, mOutputVertexBuffer };
	mQueue.enqueueAcquireGLObjects( &aqcuire );
	
	mComputeKernel.setArg( 0, mInputVertexBuffer );
	mComputeKernel.setArg( 1, mOutputNormalBuffer );
	mComputeKernel.setArg( 2, mOutputVertexBuffer );
	mComputeKernel.setArg( 3, sizeof(float), &mFrequency );
	mComputeKernel.setArg( 4, sizeof(float), &mAmplitude );
	mComputeKernel.setArg( 5, sizeof(float), &mPhase );
	mComputeKernel.setArg( 6, sizeof(float), &mLacunarity );
	mComputeKernel.setArg( 7, sizeof(float), &mIncrement );
	mComputeKernel.setArg( 8, sizeof(float), &mOctaves );
	mComputeKernel.setArg( 9, sizeof(float), &mRoughness );
	
	mQueue.enqueueNDRangeKernel( mComputeKernel,
								cl::NullRange,
								cl::NDRange( mSphereNumVertices ) );
	
	mQueue.enqueueReleaseGLObjects( &aqcuire );
	mPhase += mPhaseIncrease;
}

void ProceduralGeometricDisplacementApp::mouseDown( MouseEvent event )
{
	mCameraControl.mouseDown( event );
}

void ProceduralGeometricDisplacementApp::mouseDrag( MouseEvent event )
{
	mCameraControl.mouseDrag( event );
}

void ProceduralGeometricDisplacementApp::update()
{
	compute();
}

void ProceduralGeometricDisplacementApp::renderShadow()
{
	gl::ScopedFramebuffer scopeFrame( mShadowFbo );
	gl::clear( GL_DEPTH_BUFFER_BIT );
	gl::ScopedViewport		scopeView( vec2( 0 ), mShadowFbo->getSize() );
	gl::ScopedMatrices		scopeMat;
	gl::setMatrices( mLightCam );
	{
		gl::ScopedModelMatrix scopeModel;
		gl::translate( vec3( 0, -10, 0 ) );
		mFloorShadow->draw();
	}
	{
		gl::ScopedModelMatrix scopeModel;
		gl::translate( vec3( 0, 0, 10 ) );
		mWallShadow->draw();
	}
	{
		gl::ScopedModelMatrix scopeModel;
		gl::scale( vec3( 2.0f ) );
		mSphereShadow->draw();
	}
}

void ProceduralGeometricDisplacementApp::renderRoom()
{
	gl::ScopedState scopeOffset( GL_POLYGON_OFFSET_FILL, true );
	glPolygonOffset( mPolygonOffsetFactor, mPolygonOffsetUnits );
	
	renderShadow();
	
	gl::ScopedTextureBind scopeTex( mShadowFbo->getDepthTexture(), 0 );
	mShadowGlsl->uniform( "uShadowMatrix", mLightCam.getProjectionMatrix() * mLightCam.getViewMatrix() );
	mShadowGlsl->uniform( "light.position", vec3( gl::getModelView() * vec4( mLightCam.getEyePoint(), 1.0 ) ) );
	mShadowGlsl->uniform( "light.intensities", mLight.intensity );
	mShadowGlsl->uniform( "light.attenuation", mLight.attenuation );
	mShadowGlsl->uniform( "light.ambientCoefficient", mLight.ambientCoefficient );
	
	mShadowGlsl->uniform( "uMatAmbient", mRoomMaterial.ambient );
	mShadowGlsl->uniform( "uMatDiffuse", mRoomMaterial.diffuse );
	mShadowGlsl->uniform( "uMatSpecular", mRoomMaterial.specular );
	mShadowGlsl->uniform( "uMatShininess", mRoomMaterial.shininess );
	{
		gl::ScopedModelMatrix scopeModel;
		gl::translate( vec3( 0, -10, 0 ) );
		mFloorBatch->draw();
	}
	{
		gl::ScopedModelMatrix scopeModel;
		gl::translate( vec3( 0, 0, 10 ) );
		mWallBatch->draw();
	}
	
	mShadowGlsl->uniform( "uMatAmbient", mSphereMaterial.ambient );
	mShadowGlsl->uniform( "uMatDiffuse", mSphereMaterial.diffuse );
	mShadowGlsl->uniform( "uMatSpecular", mSphereMaterial.specular );
	mShadowGlsl->uniform( "uMatShininess", mSphereMaterial.shininess );
	{
		gl::ScopedModelMatrix scopeModel;
		gl::scale( vec3( 2.0f ) );
		mSphereRoom->draw();
	}
}

void ProceduralGeometricDisplacementApp::renderSkyBox()
{
	gl::ScopedTextureBind scopeTex( mSkyBox );
	mSkyBatch->draw();
	{
		gl::ScopedModelMatrix scopeModel;
		gl::scale( vec3( 2.0 ) );
		mSphereEnv->draw();
	}
}

void ProceduralGeometricDisplacementApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::ScopedMatrices scopeMat;
	gl::setMatrices( mUseLightCam ? mLightCam : mCam );
	
	if( mRenderRoom ) {
		renderRoom();
	}
	else {
		renderSkyBox();
	}
	
	{
		gl::ScopedModelMatrix scopeModel;
		gl::translate( mLightCam.getEyePoint() );
		gl::drawSolidCircle( vec2( 0 ), 1.0f );
	}
	
	if( mRenderDepthTex ) {
		gl::setMatricesWindow( getWindowSize() );
		gl::draw( mShadowFbo->getDepthTexture() );
	}
	
	mParams->draw();
}

CINDER_APP( ProceduralGeometricDisplacementApp, RendererGl( RendererGl::Options().msaa( 16 ) ) )
