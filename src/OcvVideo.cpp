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

#include "OcvVideo.h"

using namespace ci;
using namespace std;

OcvVideoPlayer::OcvVideoPlayer()
: mCodec( "" ), mFilePath( "" ), mSize( ivec2( 0 ) )
{
}

OcvVideoPlayer::OcvVideoPlayer( const OcvVideoPlayer& rhs )
{
	*this = rhs;
}

OcvVideoPlayer::~OcvVideoPlayer()
{
	unload();
}

OcvVideoPlayer& OcvVideoPlayer::operator=( const OcvVideoPlayer& rhs )
{
	mCapture		= rhs.mCapture;
	mCodec			= rhs.mCodec;
	mDuration		= rhs.mDuration;
	mElapsedFrames	= rhs.mElapsedFrames;
	mElapsedSeconds	= rhs.mElapsedSeconds;
	mFilePath		= rhs.mFilePath;
	mFrameDuration	= rhs.mFrameDuration;
	mFrameRate		= rhs.mFrameRate;
	mGrabTime		= rhs.mGrabTime;
	mLoaded			= rhs.mLoaded;
	mLoop			= rhs.mLoop;
	mPlaying		= rhs.mPlaying;
	mNumFrames		= rhs.mNumFrames;
	mPosition		= rhs.mPosition;
	mSize			= rhs.mSize;
	mSpeed			= rhs.mSpeed;
	return *this;
}

OcvVideoPlayer& OcvVideoPlayer::loop( bool enabled )
{
	setLoop( enabled );
	return *this;
}

OcvVideoPlayer& OcvVideoPlayer::pause( bool resume )
{
	setPause( resume );
	return *this;
}

OcvVideoPlayer& OcvVideoPlayer::speed( float v )
{
	setSpeed( v );
	return *this;
}

Surface8uRef OcvVideoPlayer::createSurface()
{
	if ( mLoaded ) {
		cv::Mat frame;
		if ( mCapture->retrieve( frame ) ) {
			return Surface8u::create( fromOcv( frame ) );
		}
	}
	return nullptr;
}

bool OcvVideoPlayer::load( const fs::path& filepath )
{
	unload();
	if ( mCapture == nullptr ) {
		mCapture = new cv::VideoCapture();
	}
	if ( mCapture != nullptr && mCapture->open( filepath.string() ) && mCapture->isOpened() ) {
		int32_t cc		= static_cast<int32_t>( mCapture->get( CV_CAP_PROP_FOURCC ) );
		char codec[]	= {
			(char)(		cc & 0X000000FF ), 
			(char)( (	cc & 0X0000FF00 ) >> 8 ),
			(char)( (	cc & 0X00FF0000 ) >> 16 ),
			(char)( (	cc & 0XFF000000 ) >> 24 ),
			0 };
		mCodec			= string( codec );

		mFilePath	= filepath;
		mFrameRate	= mCapture->get( CV_CAP_PROP_FPS );
		mNumFrames	= (uint32_t)mCapture->get( CV_CAP_PROP_FRAME_COUNT );
		mSize.x		= (int32_t)mCapture->get( CV_CAP_PROP_FRAME_WIDTH );
		mSize.y		= (int32_t)mCapture->get( CV_CAP_PROP_FRAME_HEIGHT );

		if ( mFrameRate > 0.0 ) {
			mDuration		= (double)mNumFrames / mFrameRate;
			mFrameDuration	= 1.0 / mFrameRate;
		}
		mLoaded = true;
	}
	return mLoaded;
}

void OcvVideoPlayer::play()
{
	if ( mLoaded ) {
		stop();
		mPlaying	= true;
	}
}

void OcvVideoPlayer::seek( double seconds )
{
	if ( mCapture != nullptr && mLoaded ) {
		double millis	= clamp( seconds, 0.0, mDuration ) * 1000.0;
		mCapture->set( CV_CAP_PROP_POS_MSEC, millis );
		mGrabTime		= chrono::high_resolution_clock::now();
		mPosition		= millis / mDuration;
		mElapsedFrames	= (uint32_t)( mPosition * (double)mNumFrames );
		mElapsedSeconds	= millis * 0.001;
	}
}

void OcvVideoPlayer::seekFrame( uint32_t frameNum )
{
	seek( (double)frameNum * mFrameDuration );
}

void OcvVideoPlayer::seekPosition( float ratio )
{
	seek( (double)ratio * mDuration );
}

void OcvVideoPlayer::stop()
{
	pause();
	seek( 0.0 );
}

void OcvVideoPlayer::unload()
{
	stop();
	if ( mCapture != nullptr && mLoaded ) {
		mCapture->release();
		delete mCapture;
		mCapture = nullptr;
	}
	mCodec			= "";
	mFilePath		= fs::path( "" );
	mFrameDuration	= 0.0;
	mFrameRate		= 0.0;
	mLoaded			= false;
	mNumFrames		= 0;
	mSize			= ivec2( 0 );
}

bool OcvVideoPlayer::update()
{
	if ( mCapture != nullptr && mLoaded && mPlaying && mNumFrames > 0 && mDuration > 0.0 ) {
		auto now				= chrono::high_resolution_clock::now();
		double d				= chrono::duration_cast<chrono::duration<double>>( now - mGrabTime ).count();
		double nextFrame		= mCapture->get( CV_CAP_PROP_POS_FRAMES );
		bool loop = mLoop && (uint32_t)nextFrame == mNumFrames - 1;
		if ( d >= mFrameDuration / mSpeed && mCapture->grab() ) {
			mElapsedFrames		= (uint32_t)nextFrame;
			mElapsedSeconds		= mCapture->get( CV_CAP_PROP_POS_MSEC ) * 0.001;
			mGrabTime			= now;
			mPosition			= mCapture->get( CV_CAP_PROP_POS_AVI_RATIO );
			if ( loop ) {
				seek( 0.0 );
			}
			return true;
		}
	}
	return false;
}

const string& OcvVideoPlayer::getCodec() const
{
	return mCodec;
}

double OcvVideoPlayer::getDuration() const
{
	return mDuration;
}

uint32_t OcvVideoPlayer::getElapsedFrames() const
{
	return mElapsedFrames;
}
 
double OcvVideoPlayer::getElapsedSeconds() const
{
	return mElapsedSeconds;
}
 
const fs::path& OcvVideoPlayer::getFilePath() const
{
	return mFilePath;
}

double OcvVideoPlayer::getFrameRate() const
{
	return mFrameRate;
}

uint32_t OcvVideoPlayer::getNumFrames() const
{
	return mNumFrames;
}

double OcvVideoPlayer::getPosition() const
{
	return mPosition;
}

const ivec2& OcvVideoPlayer::getSize() const
{
	return mSize;
}

float OcvVideoPlayer::getSpeed() const
{
	return mSpeed;
}

bool OcvVideoPlayer::isLoaded() const
{
	return mLoaded;
}

bool OcvVideoPlayer::isLooped() const
{
	return mLoop;
}

bool OcvVideoPlayer::isPlaying() const
{
	return mPlaying;
}

void OcvVideoPlayer::setLoop( bool enabled )
{
	mLoop = enabled;
}

void OcvVideoPlayer::setPause( bool resume )
{
	mPlaying = resume;
}

void OcvVideoPlayer::setSpeed( float v )
{
	if ( v <= 0.0f ) {
		v = 1.0f;
	}
	mSpeed = v;
}
 