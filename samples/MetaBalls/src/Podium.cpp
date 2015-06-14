//
//  Podium.cpp
//  MetaBalls
//
//  Created by ryan bartley on 6/13/15.
//
//

#include "Podium.h"
#include "cinder/TriMesh.h"
#include "cinder/ObjLoader.h"
using namespace ci;
using namespace ci::app;
using namespace std;

Podium::Podium()
{
	setupTexture();
	setupBatch();
}

PodiumRef Podium::create()
{
	return PodiumRef( new Podium );
}

void Podium::setupBatch()
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
}

void Podium::setupTexture()
{
	mSpecularMap = gl::Texture2d::createFromDds( loadAsset( "podium/podium_s.dds" ) );
	mDiffuseMap = gl::Texture2d::createFromDds( loadAsset( "podium/podium.dds" ) );
	mNormalMap = gl::Texture2d::createFromDds( loadAsset( "podium/podium_nm.dds" ) );
}

void Podium::draw()
{
	gl::ScopedModelMatrix scopeModel;
	gl::setModelMatrix( ci::translate( vec3( 32, 10, 32 ) ) );
	
	gl::ScopedTextureBind scopeTex0( mDiffuseMap, 0 );
	gl::ScopedTextureBind scopeTex1( mSpecularMap, 1 );
	gl::ScopedTextureBind scopeTex2( mNormalMap, 2 );
	
	mPodium->draw();
}
