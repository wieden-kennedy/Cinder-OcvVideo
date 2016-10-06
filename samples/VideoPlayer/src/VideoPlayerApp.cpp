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

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

#include "OcvVideo.h"

class VideoPlayerApp : public ci::app::App
{
public:
	VideoPlayerApp();

	void					draw() override;
	void					keyDown( ci::app::KeyEvent event ) override;
	void					mouseDown( ci::app::MouseEvent event ) override;
	void					mouseDrag( ci::app::MouseEvent event ) override;
	void					mouseUp( ci::app::MouseEvent event ) override;
	void					update() override;
private:
	bool					mMouseDown	= false;
	float					mPlayhead	= 0.0f;
	ci::gl::Texture2dRef	mTexture	= nullptr;
	OcvVideoPlayer			mVideoPlayer;
};

#include "cinder/app/RendererGl.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

VideoPlayerApp::VideoPlayerApp()
{
	const fs::path filepath = getAssetPath( "sample.mp4" );
	if ( mVideoPlayer.load( filepath ) ) {
		CI_LOG_V( mVideoPlayer.getFilePath() << " loaded successfully: " );

		CI_LOG_V( " > Codec: "		<< mVideoPlayer.getCodec() );
		CI_LOG_V( " > Duration: "	<< mVideoPlayer.getDuration() );
		CI_LOG_V( " > FPS: "		<< mVideoPlayer.getFrameRate() );
		CI_LOG_V( " > Num frames: "	<< mVideoPlayer.getNumFrames() );
		CI_LOG_V( " > Size: "		<< mVideoPlayer.getSize() );

		const ivec2& sz = mVideoPlayer.getSize();
		setWindowSize( sz );
		setWindowPos( ( getDisplay()->getSize() - sz ) / 2 );
		mVideoPlayer.play();
	} else {
		CI_LOG_V( "Unable to load movie" );
		quit();
	}
}

void VideoPlayerApp::draw()
{
	gl::setMatricesWindow( getWindowSize() );
	gl::clear();

	if ( mTexture != nullptr ) {
		gl::draw( mTexture, mTexture->getBounds(), getWindowBounds() );
	}
}

void VideoPlayerApp::keyDown( KeyEvent event )
{
	static const float d	= 0.1f;
	const float s			= mVideoPlayer.getSpeed();
	switch ( event.getCode() ) {
	case KeyEvent::KEY_LEFT:
		mVideoPlayer.setSpeed( math<float>::max( s - d, 0.25f ) );
		break;
	case KeyEvent::KEY_RIGHT:
		mVideoPlayer.setSpeed( math<float>::min( s + d, 4.0f ) );
		break;
	case KeyEvent::KEY_f:
		setFullScreen( !isFullScreen() );
		break;
	case KeyEvent::KEY_q:
		quit();
		break;
	}
}

void VideoPlayerApp::mouseDown( MouseEvent event )
{
	mouseDrag( event );
}

void VideoPlayerApp::mouseDrag( MouseEvent event )
{
	mMouseDown	= true;
	mPlayhead	= (float)event.getPos().x / (float)getWindowWidth();
}

void VideoPlayerApp::mouseUp( MouseEvent event )
{
	mMouseDown = false;
}

void VideoPlayerApp::update()
{
	if ( mMouseDown ) {
		mVideoPlayer.seekPosition( mPlayhead );
	}
	if ( mVideoPlayer.update() ) {
		Surface8uRef s = mVideoPlayer.createSurface();
		if ( s != nullptr ) {
			mTexture = gl::Texture2d::create( *s );
		}
	}
}

CINDER_APP( VideoPlayerApp, RendererGl )
