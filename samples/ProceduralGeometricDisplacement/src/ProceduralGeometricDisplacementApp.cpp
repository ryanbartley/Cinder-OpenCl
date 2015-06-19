#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Log.h"
#include "Cinder-OpenCL.h"
#include "cinder/gl/ConstantConversions.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#pragma OPENCL EXTENSION cl_khr_gl_event : enable

int divide_up(int a, int b)
{
	return ((a % b) != 0) ? (a / b + 1) : (a / b);
}

struct Material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

struct Light {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
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
	void createJitterTexture();
	void createShadowFbo();
	void createCubeMap();
	ci::TriMeshRef createSphere( const ci::TriMeshRef &sphereMesh );
	
	void compute();
	
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
						mSkyBatch, mRoomBatch;
	gl::Texture3dRef	mJitterTexture;
	gl::FboRef			mShadowFbo;
	gl::TextureCubeMapRef mSkyBox;
	
	cl::Context			mContext;
	cl::Kernel			mComputeKernel;
	cl::Program			mComputeProgram;
	cl::Device			mDevice;
	cl::CommandQueue	mQueue;
	cl::Buffer			mInputVertexBuffer;
	cl::BufferGL		mOutputVertexBuffer,
						mOutputNormalBuffer;
	
	bool		mRenderRoom			= false;
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
	
	float	mDepthBias	= -0.0005f;
	float	mPolygonOffsetFactor;
	float	mPolygonOffsetUnits;
	
	Light		mLight;
	Material	mSphereMaterial, mQuadMaterial;
};

void ProceduralGeometricDisplacementApp::setup()
{
	mSphereMaterial.ambient = {0.1f, 0.1f, 0.1f, 1.0f};
	mSphereMaterial.diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
	mSphereMaterial.specular = {1.0f, 1.0f, 1.0f, 1.0f};
	mSphereMaterial.shininess = 128.0f;
	
	mQuadMaterial.ambient = {1.0f, 1.0f, 1.0f, 1.0f};
	mQuadMaterial.diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
	mQuadMaterial.specular = {1.0f, 1.0f, 1.0f, 1.0f};
	mQuadMaterial.shininess = 0.0f;
	
	mLight.ambient = {0.1f, 0.1f, 0.1f, 1.0f};
	mLight.diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
	mLight.specular = {1.0f, 1.0f, 1.0f, 1.0f};
	mLight.attenuation = 0.1f;
	
	setupCl();
	setupBuffers();
	setupKernels();
	createJitterTexture();
	createShadowFbo();
	createCubeMap();
	
	mCam.lookAt( vec3( 0.0f, 5.0f, -7.0f ), vec3( 0 ) );
	mCam.setPerspective( 55.0f, getWindowAspectRatio(), 0.1f, 100.0f );
	mLightCam.lookAt( vec3( -7.0f, 10.0f, 7.0f ), vec3( 0 ) );
	mLightCam.setPerspective( 45.0f, 1.0f, 10.0f, 20.0f );
	
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

ci::TriMeshRef ProceduralGeometricDisplacementApp::createSphere( const ci::TriMeshRef &sphereMesh )
{
	auto ret = ci::TriMesh::create( ci::TriMesh::Format().positions(4) );
	
	for( auto & sphereIndex : sphereMesh->getIndices() ) {
		ret->appendPosition( sphereMesh->getPositions<4>()[sphereIndex] );
	}
	
	return ret;
}

void ProceduralGeometricDisplacementApp::setupBuffers()
{
	auto environmentMap = gl::GlslProg::create( loadAsset( "env_map.vert" ), loadAsset( "env_map.frag" ) );
	environmentMap->uniform( "uCubeMapTex", 0 );
	mShadowGlsl	= gl::GlslProg::create( loadAsset( "shadow_mapping.vert"), loadAsset("shadow_mapping.frag") );
	mShadowGlsl->uniform( "uShadowMap", 0 );
	mShadowGlsl->uniform( "uDepthBias", mDepthBias );
	auto skyBoxGlsl		= gl::GlslProg::create( loadAsset( "sky_box.vert" ), loadAsset( "sky_box.frag" ) );
	skyBoxGlsl->uniform( "uCubeMapTex", 0 );
	
	// Create the room.
	auto cube = geom::Cube().size( vec3( 20 ) );
	mRoomBatch = gl::Batch::create( cube, gl::getStockShader( gl::ShaderDef().color() ) );
	cube.size( vec3( 500 ) );
	mSkyBatch = gl::Batch::create( cube, skyBoxGlsl );
	
	// Create the sphere
	auto sphere = geom::Sphere().subdivisions( 200 );
	auto sphereIndexTrimesh = TriMesh::create( sphere, TriMesh::Format().positions(4) );
	
	auto sphereTrimesh = createSphere( sphereIndexTrimesh );
	
	mSphereNumVertices = sphereTrimesh->getNumVertices();
	cout << mSphereNumVertices << endl;
	size_t bytes = mSphereNumVertices * sizeof(vec4);

	// Create the buffers for the position and normals
	auto positionVbo	= gl::Vbo::create( GL_ARRAY_BUFFER, bytes, nullptr, GL_STATIC_DRAW );
	auto posAttrib		= geom::AttribInfo( geom::POSITION, geom::FLOAT, 4, 0, 0 );
	auto positionLayout = geom::BufferLayout( { posAttrib } );
	auto normalVbo		= gl::Vbo::create( GL_ARRAY_BUFFER, bytes, nullptr, GL_STATIC_DRAW );
	auto normalAttrib	= geom::AttribInfo( geom::NORMAL, geom::FLOAT, 4, 0, 0 );
	auto normalLayout	= geom::BufferLayout( { normalAttrib } );
	
	auto vboMesh		= gl::VboMesh::create( mSphereNumVertices, GL_TRIANGLES,
									   { { positionLayout, positionVbo },
										   { normalLayout, normalVbo } } );

	mSphereEnv		= gl::Batch::create( vboMesh, environmentMap );
	mSphereRoom		= gl::Batch::create( vboMesh, mShadowGlsl );
	mSphereShadow	= gl::Batch::create( vboMesh, gl::getStockShader( gl::ShaderDef() ) );
	
	mInputVertexBuffer = cl::Buffer( mContext, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
									bytes, sphereTrimesh->getBufferPositions().data() );
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

void ProceduralGeometricDisplacementApp::createJitterTexture()
{
	static const float twopi = 2.0 * M_PI;
	
	int size = 16;
	int du = 8;
	int dv = 8;
	
	int tw = size;
	int th = size;
	int td = du * dv * 0.5f;
	
	std::vector<uint8_t> data( 4 * tw * th * td );
	
	for (int i = 0; i < tw; i++ ) {
		for ( int j = 0; j < th; j++ ) {
			for ( int k = 0; k < td; k++ ) {
				int x, y;
				float d[4];
				float v[4];
				
				x = k % (du / 2);
				y = (dv - 1) - k / (du / 2);
				
				v[0] = (float)(x * 2 + 0.5f) / du;
				v[1] = (float)(y + 0.5f) / dv;
				v[2] = (float)(x * 2 + 1 + 0.5f) / du;
				v[3] = v[1];
				
				v[0] += ((float)rand() * 2 / RAND_MAX - 1) * (0.5f / du);
				v[1] += ((float)rand() * 2 / RAND_MAX - 1) * (0.5f / dv);
				v[2] += ((float)rand() * 2 / RAND_MAX - 1) * (0.5f / du);
				v[3] += ((float)rand() * 2 / RAND_MAX - 1) * (0.5f / dv);
				
				d[0] = sqrtf(v[1]) * cosf(twopi * v[0]);
				d[1] = sqrtf(v[1]) * sinf(twopi * v[0]);
				d[2] = sqrtf(v[3]) * cosf(twopi * v[2]);
				d[3] = sqrtf(v[3]) * sinf(twopi * v[2]);
				
				unsigned int index = (k * tw * th + j * tw + i) * 4;
				data[index + 0] = (1.0f + d[0]) * 127;
				data[index + 1] = (1.0f + d[1]) * 127;
				data[index + 2] = (1.0f + d[2]) * 127;
				data[index + 3] = (1.0f + d[3]) * 127;
			}
		}
	}
	
	auto format = gl::Texture3d::Format()
					.minFilter( GL_NEAREST )
					.magFilter( GL_NEAREST )
					.wrap( GL_REPEAT );
	format.setDataType( GL_UNSIGNED_BYTE );
	
	mJitterTexture = gl::Texture3d::create( nullptr, GL_RGBA4, tw, th, td, format );
}

void ProceduralGeometricDisplacementApp::createShadowFbo()
{
	auto texFormat = gl::Texture2d::Format()
						.internalFormat( GL_DEPTH_COMPONENT24 )
						.minFilter( GL_LINEAR )
						.magFilter( GL_LINEAR )
						.wrap( GL_CLAMP_TO_EDGE )
						.compareFunc( GL_COMPARE_R_TO_TEXTURE_ARB )
						.compareMode( GL_LEQUAL );
	
	mShadowFbo = gl::Fbo::create( 1024, 1024,
								gl::Fbo::Format()
								.disableColor()
								.depthTexture( texFormat ) );
}

void ProceduralGeometricDisplacementApp::createCubeMap()
{
	mSkyBox = gl::TextureCubeMap::create( loadImage( loadAsset( "env_map.jpg" ) ), gl::TextureCubeMap::Format().mipmap() );
}

void ProceduralGeometricDisplacementApp::compute()
{
	size_t global[2];
	size_t local[2];
	
	std::vector<cl::Memory> aqcuire = { mOutputNormalBuffer, mOutputVertexBuffer };
	mQueue.enqueueAcquireGLObjects( &aqcuire );
	
	float count = mSphereNumVertices;
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
	mComputeKernel.setArg( 10, sizeof(float), &count );
	
	uint uiSplitCount = ceilf( sqrtf( mSphereNumVertices ) );
	uint uiActive = (mMaxWorkGroupSize / mGroupSize);
	uiActive = uiActive < 1 ? 1 : uiActive;
	
	uint uiQueued = mMaxWorkGroupSize / uiActive;
	
	local[0] = uiActive;
	local[1] = uiQueued;
	
	global[0] = divide_up(uiSplitCount, uiActive) * uiActive;
	global[1] = divide_up(uiSplitCount, uiQueued) * uiQueued;
	cout << global[0] << " " << global[1] << " " << local[0] << " " << local[1] << endl;
	mQueue.enqueueNDRangeKernel( mComputeKernel,
								cl::NullRange,
								cl::NDRange( global[0], global[1] ),
								cl::NDRange( local[0], local[1] ) );
	
	mQueue.enqueueReleaseGLObjects( &aqcuire );
	mPhase += .01;
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

void ProceduralGeometricDisplacementApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::ScopedMatrices scopeMat;
	gl::setMatrices( mCam );
	
	if( mRenderRoom ) {
		gl::ScopedState scopeOffset( GL_POLYGON_OFFSET_FILL, true );
		glPolygonOffset( mPolygonOffsetFactor, mPolygonOffsetUnits );
		{
			gl::ScopedFramebuffer	scopeFbo( mShadowFbo );
			gl::ScopedViewport		scopeView( vec2( 0 ), mShadowFbo->getSize() );
			gl::ScopedMatrices		scopeMat;
			gl::setMatrices( mLightCam );
			mSphereShadow->draw();
		}
		gl::ScopedTextureBind scopeTex( mShadowFbo->getDepthTexture() );
		
		mShadowGlsl->uniform( "uShadowMatrix", mLightCam.getProjectionMatrix() * mLightCam.getViewMatrix() );
		mShadowGlsl->uniform( "uLightPos", vec3( gl::getModelView() * vec4( mLightCam.getEyePoint(), 1.0 ) ) );
		mRoomBatch->draw();
		mSphereShadow->draw();
	}
	else {
		gl::ScopedTextureBind scopeTex( mSkyBox );
//		mSkyBatch->draw();
		mSphereEnv->draw();
	}
	
}

CINDER_APP( ProceduralGeometricDisplacementApp, RendererGl )
