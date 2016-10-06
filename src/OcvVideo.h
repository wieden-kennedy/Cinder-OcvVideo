/*
* 
* Copyright (c) 2016, Wieden+Kennedy
* Stephen Schieberl
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following 
* conditions are met:
* 
* Redistributions of source code must retain the above copyright 
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright 
* notice, this list of conditions and the following disclaimer in 
* the documentation and/or other materials provided with the 
* distribution.
* 
* Neither the name of the Wieden + Kennedy nor the names of its 
* contributors may be used to endorse or promote products 
* derived from this software without specific prior written 
* permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#pragma once

#include "CinderOpenCV.h"
#include "cinder/Filesystem.h"
#include "cinder/Vector.h"
#include <exception>
#include <chrono>
#include <string>

typedef std::shared_ptr<cv::VideoCapture>				VideoCaptureRef;
typedef std::chrono::high_resolution_clock::time_point	time_point;

class OcvVideoPlayer
{
public:
	OcvVideoPlayer();
	OcvVideoPlayer( const OcvVideoPlayer& rhs );
	~OcvVideoPlayer();

	OcvVideoPlayer&			operator=( const OcvVideoPlayer& rhs );

	OcvVideoPlayer&			loop( bool enabled = true );
	OcvVideoPlayer&			pause( bool resume = false );
	OcvVideoPlayer&			speed( float v );

	ci::Surface8uRef		createSurface();
	bool					load( const ci::fs::path& filepath );
	void					play();
	void					seek( double seconds );
	void					seekFrame( uint32_t frameNum );
	void					seekPosition( float ratio );
	void					stop();
	void					unload();
	bool					update();

	const std::string&		getCodec() const;
	double					getDuration() const;
	uint32_t				getElapsedFrames() const;	
	double					getElapsedSeconds() const;	
	const ci::fs::path& 	getFilePath() const;
	double					getFrameRate() const;
	uint32_t				getNumFrames() const;
	double					getPosition() const;
	const ci::ivec2&		getSize() const;
	float					getSpeed() const;
	bool					isLoaded() const;
	bool					isLooped() const;
	bool					isPlaying() const;

	void					setLoop( bool enabled );
	void					setPause( bool resume );
	void					setSpeed( float v );
protected:
	VideoCaptureRef			mCapture		= nullptr;
	std::string				mCodec;
	double					mDuration		= 0.0;
	uint32_t				mElapsedFrames	= 0;
	double					mElapsedSeconds	= 0.0;
	ci::fs::path 			mFilePath;
	double					mFrameDuration	= 0.0;
	double					mFrameRate		= 0.0;
	time_point				mGrabTime;
	bool					mLoaded			= false;
	bool					mLoop			= true;
	bool					mPlaying		= false;
	uint32_t				mNumFrames		= 0;
	double					mPosition		= 0.0;
	ci::ivec2				mSize;
	float					mSpeed			= 1.0f;
};
