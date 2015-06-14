//
//  Podium.h
//  MetaBalls
//
//  Created by ryan bartley on 6/13/15.
//
//

#pragma once

#include "cinder/gl/gl.h"

using PodiumRef = std::shared_ptr<class Podium>;

class Podium {
public:
	static PodiumRef create();
	
	void draw();
	
private:
	Podium();
	
	void setupBatch();
	void setupTexture();
	
	ci::gl::BatchRef mPodium;
	ci::gl::Texture2dRef mSpecularMap, mDiffuseMap, mNormalMap;
	
};
