////
////  MarchingCubes.cpp
////  MetaBalls2
////
////  Created by Ryan Bartley on 7/19/14.
////
////
//
//#include "MarchingCubes.h"
//#include "cinder/Log.h"
//#include "cinder/gl/gl.h"
//#include "cinder/gl/Vao.h"
//#include "cinder/gl/GlslProg.h"
//#include "cinder/Utilities.h"
//#include "cinder/app/App.h"
//
//using namespace std;
//using namespace ci;
//using namespace ci::app;
//using namespace ci::gl;
//
//
//int MarchingCubes::num_verts = 0;
//
//void MarchingCubes::setupGl()
//{
//	std::vector<vec4> vertPosData(MAX_VERTS);
//	std::vector<vec4> vertNormData(MAX_VERTS);
//	
//	// Create Position Buffer
//	mGlVertPositions = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4) * MAX_VERTS, vertPosData.data(), GL_DYNAMIC_COPY);
//	geom::BufferLayout positionBufferLayout( { geom::AttribInfo( geom::POSITION, geom::FLOAT, 4, 0, 0 ) } );
//	// Create Normal Buffer
//	mGlVertNormals = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4) * MAX_VERTS, vertNormData.data(), GL_DYNAMIC_COPY);
//	geom::BufferLayout normalBufferLayout( { geom::AttribInfo( geom::NORMAL, geom::FLOAT, 4, 0, 0 ) } );
//	
//	{
//		auto glsl = gl::GlslProg::create( gl::GlslProg::Format( )
//										 .vertex( ci::app::loadAsset( "phong.vert" ) )
//										 .fragment( ci::app::loadAsset( "phong.frag" ) ) );
//		auto vboMesh = gl::VboMesh::create( MAX_VERTS, GL_TRIANGLES, {
//			{ positionBufferLayout, mGlVertPositions },
//			{ normalBufferLayout, mGlVertNormals }
//		} );
//		mRenderBatch = gl::Batch::create( vboMesh, glsl );
//	}
//	
//	std::vector<vec4> pointColorData(full_size);
//	std::vector<vec4> pointData(full_size);
//	
//	int x, y, z;
//	for(x = 0; x < width; x++)
//		for(y = 0; y < height; y++)
//			for(z = 0; z < depth; z++) {
//				int id = x + y * width + z * width * height;
//				vec4 position = vec4(x, y, z, 1);
//				pointData[id] = position;
//			}
//	
//	// Create Position Indices Buffer
//	mGlPointPositions = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4) * full_size, pointData.data(), GL_STATIC_DRAW);
//	geom::BufferLayout positionIndexBufferLayout( { geom::AttribInfo( geom::POSITION, geom::FLOAT, 4, 0, 0 ) } );
//	// Create Color Buffer
//	mGlPointColors = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4) * full_size, pointColorData.data(), GL_DYNAMIC_COPY);
//	geom::BufferLayout colorBufferLayout( { geom::AttribInfo( geom::COLOR, geom::FLOAT, 4, 0, 0 ) } );
//	
//	{
//		auto glsl = gl::GlslProg::create( gl::GlslProg::Format( )
//										 .vertex( loadAsset( "basic.vert" ) )
//										 .fragment( loadAsset( "basic.frag" ) ) );
//		
//		auto vboMesh = gl::VboMesh::create( full_size, GL_POINTS, {
//			{ positionIndexBufferLayout, mGlPointPositions },
//			{ colorBufferLayout, mGlPointColors },
//		} );
//		mDebugBatch = gl::Batch::create( vboMesh, glsl );
//	}
//	mEnvironmentMap = gl::Texture2d::createFromDds( loadAsset( "metaballs_env.dds" ) );
//}
//
//void MarchingCubes::setupCl()
//{
//	auto clCtx = MetaBallsController::get()->getContext();
//	mClPointColors   =	cl::BufferGL( clCtx, CL_MEM_READ_WRITE, mGlPointColors->getId() );
//	mClVertPositions =	cl::BufferGL( clCtx, CL_MEM_READ_WRITE, mGlVertPositions->getId() );
//	mClVertNormals   =	cl::BufferGL( clCtx, CL_MEM_READ_WRITE, mGlVertNormals->getId() );
//	mClVolume		 =	cl::Buffer( clCtx, CL_MEM_READ_WRITE, sizeof(float) * full_size, nullptr );
//	mClVertIndex     =	cl::Buffer( clCtx, CL_MEM_READ_WRITE, sizeof(int), nullptr );
//	
//	
//	mClProgram = cl::Program( clCtx, loadString( loadAsset( "kernels/marching_cubes.cl" ) ) );
//	auto path = "-I" + getAssetPath( "" ).string();
//	mClProgram.build( path.c_str() );
//	
//	mKernWritePoint = cl::Kernel( mClProgram, "write_point" );
//	mKernWritePoint.setArg( 0, mClVolume );
//	mKernWritePoint.setArg( 4, sizeof(int), (void*)&width );
//	mKernWritePoint.setArg( 5, sizeof(int), (void*)&height );
//	mKernWritePoint.setArg( 6, sizeof(int), (void*)&depth );
//	
//	mKernWriteMetaball = cl::Kernel( mClProgram, "write_metaball" );
//	mKernWriteMetaball.setArg( 0, mClVolume );
//	
//	mKernWriteMetaballs = cl::Kernel( mClProgram, "write_metaballs" );
//	mKernWriteMetaballs.setArg( 0, mClVolume );
//	
//	mKernWriteClear = cl::Kernel( mClProgram, "write_clear" );
//	mKernWriteClear.setArg( 0, mClVolume );
//	
//	mKernWritePointColorBack = cl::Kernel( mClProgram, "write_point_color_back" );
//	mKernWritePointColorBack.setArg( 0, mClVolume );
//	mKernWritePointColorBack.setArg( 1, mClPointColors );
//	
//	mKernConstructSurface = cl::Kernel( mClProgram, "construct_surface" );
//	mKernConstructSurface.setArg( 0, mClVolume );
//	
//	mKernGenNormals = cl::Kernel( mClProgram, "generate_flat_normals" );
//	mKernGenNormalsSmooth = cl::Kernel( mClProgram, "generate_smooth_normals" );
//}
//
//void MarchingCubes::update()
//{
//	
//	ivec3 size{width, height, depth};
//	std::vector<cl::Event> waitEvents;
//	
//	clCQ.enqueueNDRangeKernel( mKernWriteClear,
//							  cl::NullRange,
//							  cl::NDRange( full_size ),
//							  cl::NullRange );
//	
//	/* Update volumes */
//	mKernWriteMetaballs.setArg( 1, sizeof(cl_int3), &size );
//	mKernWriteMetaballs.setArg( 2, mMetaballPositions );
//	mKernWriteMetaballs.setArg( 3, sizeof(cl_int), &mNumBalls );
//	
//	clCQ.enqueueNDRangeKernel( mKernWriteMetaballs,
//							cl::NullRange,
//							cl::NDRange(  width * height * depth ),
//							cl::NullRange );
//	
//	/* End */
//	
//	int zero = 0;
//	cl::Event writeEvent;
//	clCQ.enqueueWriteBuffer( mClVertIndex, false, 0, sizeof(int), &zero, nullptr, &writeEvent );
//	
//	mKernConstructSurface.setArg( 0, mClVolume );
//	mKernConstructSurface.setArg( 1, sizeof(cl_int3), &size );
//	mKernConstructSurface.setArg( 2, mClVertPositions );
//	mKernConstructSurface.setArg( 3, mClVertIndex );
//	
//	waitEvents.push_back( writeEvent );
//	
//	cl::Event constructEvent;
//	clCQ.enqueueNDRangeKernel( mKernConstructSurface,
//										cl::NullRange,
//										cl::NDRange( (width-1) * (height-1) * (depth-1) ),
//										cl::NullRange,
//										&waitEvents,
//										&constructEvent );
//	waitEvents.push_back( constructEvent );
//	cl::Event readEvent;
//	clCQ.enqueueReadBuffer( mClVertIndex, true, 0, sizeof(cl_int), &num_verts, &waitEvents, &readEvent );
//	waitEvents.push_back( readEvent );
//	
//	/* Generate Normals */
//	if (num_verts > 0) {
//		bool smooth = true;
//		cl::Event normals;
//		if( ! smooth ) {
//			mKernGenNormals.setArg( 0, mClVertPositions );
//			mKernGenNormals.setArg( 1, mClVertNormals );
//			clCQ.enqueueNDRangeKernel( mKernGenNormals,
//												cl::NullRange,
//												cl::NDRange( num_verts ),
//												cl::NullRange,
//												&waitEvents,
//												&normals );
//		}
//		else {
//			mKernGenNormalsSmooth.setArg( 0, mClVertPositions );
//			mKernGenNormalsSmooth.setArg( 1, mClVertNormals );
//			mKernGenNormalsSmooth.setArg( 2, mMetaballPositions );
//			mKernGenNormalsSmooth.setArg( 3, sizeof(cl_int), &mNumBalls );
//			clCQ.enqueueNDRangeKernel( mKernGenNormalsSmooth,
//												cl::NullRange,
//												cl::NDRange( num_verts ),
//												cl::NullRange,
//												&waitEvents,
//												&normals );
//		}
//		waitEvents.push_back( normals );
//	}
//
//	if( mDebugDraw ) {
//		cl::Event pointColor;
//		clCQ.enqueueNDRangeKernel( mKernWritePointColorBack,
//											cl::NullRange,
//											cl::NDRange( full_size ),
//											cl::NullRange,
//											&waitEvents,
//											&pointColor );
//		waitEvents.push_back( pointColor );
//	}
//}
//
//
