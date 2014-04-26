#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Timeline.h"
#include "cinder/Timer.h"

#include "cinder/audio2/Source.h"
#include "cinder/audio2/Target.h"
#include "cinder/audio2/dsp/Converter.h"
#include "cinder/audio2/SamplePlayer.h"
#include "cinder/audio2/SampleRecorder.h"
#include "cinder/audio2/NodeEffect.h"
#include "cinder/audio2/Scope.h"
#include "cinder/audio2/Debug.h"

#include "assets/Resources.h"

#include "cvisible/AudioTestGui.h"
#include "cvisible/AudioDrawUtils.h"

//#define INITIAL_AUDIO_RES	RES_TONE440_WAV
//#define INITIAL_AUDIO_RES	RES_TONE440L220R_WAV
//#define INITIAL_AUDIO_RES	RES_TONE440L220R_FLOAT_WAV
//#define INITIAL_AUDIO_RES	RES_TONE440_MP3
//#define INITIAL_AUDIO_RES	RES_TONE440L220R_MP3
#define INITIAL_AUDIO_RES	RES_CASH_MP3
//#define INITIAL_AUDIO_RES	RES_TONE440_OGG
//#define INITIAL_AUDIO_RES	RES_TONE440L220R_OGG
//#define INITIAL_AUDIO_RES	RES_RADIOHEAD_OGG

using namespace ci;
using namespace ci::app;
using namespace std;


class SamplePlayerTestApp : public AppNative {
  public:
	void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );
	void keyDown( KeyEvent event );
	void fileDrop( FileDropEvent event );
	void update();
	void draw();

	void setupBufferPlayer();
	void setupFilePlayer();
	void setupBufferRecorder();
	void setSourceFile( const DataSourceRef &dataSource );
	void writeRecordedToFile();

	void setupUI();
	void processDrag( Vec2i pos );
	void processTap( Vec2i pos );

	void seek( size_t xPos );
	void printBufferSamples( size_t xPos );

	void testConverter();
	void testWrite();

	audio2::SamplePlayerRef		mSamplePlayer;
	audio2::SourceFileRef		mSourceFile;
	audio2::ScopeRef			mScope;
	audio2::GainRef				mGain;
	audio2::Pan2dRef			mPan;
	audio2::BufferRecorderRef	mRecorder;

	WaveformPlot				mWaveformPlot;
	vector<TestWidget *>		mWidgets;
	Button						mEnableSamplePlayerButton, mStartPlaybackButton, mLoopButton, mAsyncButton, mRecordButton, mWriteButton, mAutoResizeButton;
	VSelector					mTestSelector;
	HSlider						mGainSlider, mPanSlider, mLoopBeginSlider, mLoopEndSlider;

	Anim<float>					mUnderrunFade, mOverrunFade, mRecorderOverrunFade;
	Rectf						mUnderrunRect, mOverrunRect, mRecorderOverrunRect;
	bool						mSamplePlayerEnabledState;
	std::future<void>			mAsyncLoadFuture;
};

void SamplePlayerTestApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1000, 500 );
}

void SamplePlayerTestApp::setup()
{
	mUnderrunFade = mOverrunFade = mRecorderOverrunFade = 0;
	mSamplePlayerEnabledState = false;

	setSourceFile( loadResource( INITIAL_AUDIO_RES ) );

	auto ctx = audio2::master();

	mPan = ctx->makeNode( new audio2::Pan2d() );
//	mPan->enableMonoInputMode( false );

	mGain = ctx->makeNode( new audio2::Gain() );
	mGain->setValue( 0.6f );

	mGain >> mPan >> ctx->getOutput();

	setupBufferPlayer();
//	setupFilePlayer();

	setupUI();

	ctx->enable();
	mEnableSamplePlayerButton.setEnabled( true );

	CI_LOG_V( "context samplerate: " << ctx->getSampleRate() );
}

void SamplePlayerTestApp::setupBufferPlayer()
{
	auto bufferPlayer = audio2::master()->makeNode( new audio2::BufferPlayer() );

	auto loadFn = [bufferPlayer, this] {
		bufferPlayer->loadBuffer( mSourceFile );
		mWaveformPlot.load( bufferPlayer->getBuffer(), getWindowBounds() );
		CI_LOG_V( "loaded source buffer, frames: " << bufferPlayer->getBuffer()->getNumFrames() );

	};

	auto connectFn = [bufferPlayer, this] {
		mSamplePlayer = bufferPlayer;
		mSamplePlayer >> mGain >> mPan >> audio2::master()->getOutput();
		audio2::master()->printGraph();

		mSamplePlayer->setLoopEnabled( mLoopButton.mEnabled );
		mSamplePlayer->setLoopBeginTime( mLoopBeginSlider.mValueScaled );
		mSamplePlayer->setLoopEndTime( mLoopEndSlider.mValueScaled != 0 ? mLoopEndSlider.mValueScaled : mSamplePlayer->getNumSeconds() );
	};

	bool asyncLoad = mAsyncButton.mEnabled;
	CI_LOG_V( "async load: " << boolalpha << asyncLoad << dec );
	if( asyncLoad ) {
		mWaveformPlot.clear();
		mAsyncLoadFuture = std::async( [=] {
			loadFn();
			dispatchAsync( [=] {
				connectFn();
			} );
		} );
	}
	else {
		loadFn();
		connectFn();
	};
}

void SamplePlayerTestApp::setupFilePlayer()
{
	auto ctx = audio2::master();

//	mSourceFile->setMaxFramesPerRead( 8192 );
	bool asyncRead = mAsyncButton.mEnabled;
	CI_LOG_V( "async read: " << asyncRead );
	mSamplePlayer = ctx->makeNode( new audio2::FilePlayer( mSourceFile, asyncRead ) );

	// TODO: it is pretty surprising when you recreate mScope here without checking if there has already been one added.
	//	- user will no longer see the old mScope, but the context still owns a reference to it, so another gets added each time we call this method.
	//		- this is also because it uses 'addConnection', instead of connect with default bus numbers.
	if( ! mScope )
		mScope = ctx->makeNode( new audio2::Scope( audio2::Scope::Format().windowSize( 1024 ) ) );

	// when these connections are called, some (Gain and Pan) will already be connected, but this is okay, they should silently no-op.

	// or connect in series (it is added to the Context's 'auto pulled list')
	mSamplePlayer >> mGain >> mPan >> ctx->getOutput();
	mPan->addConnection( mScope );

	mSamplePlayer->setLoopEnabled( mLoopButton.mEnabled );
	mSamplePlayer->setLoopBeginTime( mLoopBeginSlider.mValueScaled );
	mSamplePlayer->setLoopEndTime( mLoopEndSlider.mValueScaled != 0 ? mLoopEndSlider.mValueScaled : mSamplePlayer->getNumSeconds() );

	audio2::master()->printGraph();
}

void SamplePlayerTestApp::setupBufferRecorder()
{
	auto ctx = audio2::master();

	mRecorder = ctx->makeNode( new audio2::BufferRecorder( 2 * ctx->getSampleRate() ) );
//	mRecorder->setNumSeconds( 2 ); // can also set seconds afterwards

	CI_ASSERT( mSamplePlayer );

	mSamplePlayer >> mRecorder;

	audio2::master()->printGraph();
}

void SamplePlayerTestApp::setSourceFile( const DataSourceRef &dataSource )
{
	mSourceFile = audio2::load( dataSource );

	getWindow()->setTitle( dataSource->getFilePath().filename().string() );

	CI_LOG_V( "SourceFile info: " );
	console() << "samplerate: " << mSourceFile->getSampleRate() << endl;
	console() << "channels: " << mSourceFile->getNumChannels() << endl;
	console() << "native samplerate: " << mSourceFile->getNativeSampleRate() << endl;
	console() << "native channels: " << mSourceFile->getNativeNumChannels() << endl;
	console() << "frames: " << mSourceFile->getNumFrames() << endl;
	console() << "metadata:\n" << mSourceFile->getMetaData() << endl;
}

void SamplePlayerTestApp::writeRecordedToFile()
{
	const string fileName = "recorder_out.wav";
	CI_LOG_V( "writing to: " << fileName );

	mRecorder->writeToFile( fileName );

	CI_LOG_V( "...complete." );
}

void SamplePlayerTestApp::setupUI()
{
	const float padding = 10.0f;

	auto buttonRect = Rectf( padding, padding, 200, 60 );
	mEnableSamplePlayerButton.mIsToggle = true;
	mEnableSamplePlayerButton.mTitleNormal = "SamplePlayer off";
	mEnableSamplePlayerButton.mTitleEnabled = "SamplePlayer on";
	mEnableSamplePlayerButton.mBounds = buttonRect;
	mWidgets.push_back( &mEnableSamplePlayerButton );

	buttonRect += Vec2f( buttonRect.getWidth() + padding, 0 );
	mStartPlaybackButton.mIsToggle = false;
	mStartPlaybackButton.mTitleNormal = "start";
	mStartPlaybackButton.mBounds = buttonRect;
	mWidgets.push_back( &mStartPlaybackButton );

	buttonRect += Vec2f( buttonRect.getWidth() + padding, 0 );
	buttonRect.x2 -= 30;
	mLoopButton.mIsToggle = true;
	mLoopButton.mTitleNormal = "loop off";
	mLoopButton.mTitleEnabled = "loop on";
	mLoopButton.setEnabled( mSamplePlayer->isLoopEnabled() );
	mLoopButton.mBounds = buttonRect;
	mWidgets.push_back( &mLoopButton );

	buttonRect += Vec2f( buttonRect.getWidth() + padding, 0 );
	mAsyncButton.mIsToggle = true;
	mAsyncButton.mTitleNormal = "async off";
	mAsyncButton.mTitleEnabled = "async on";
	mAsyncButton.mBounds = buttonRect;
	mWidgets.push_back( &mAsyncButton );

	buttonRect = Rectf( padding, buttonRect.y2 + padding, padding + buttonRect.getWidth(), buttonRect.y2 + buttonRect.getHeight() + padding );
	mRecordButton.mIsToggle = true;
	mRecordButton.mTitleNormal = "record off";
	mRecordButton.mTitleEnabled = "record on";
	mRecordButton.mBounds = buttonRect;
	mWidgets.push_back( &mRecordButton );

	buttonRect += Vec2f( buttonRect.getWidth() + padding, 0 );
	mWriteButton.mIsToggle = false;
	mWriteButton.mTitleNormal = "write to file";
	mWriteButton.mBounds = buttonRect;
	mWidgets.push_back( &mWriteButton );

	buttonRect += Vec2f( buttonRect.getWidth() + padding, 0 );
	mAutoResizeButton.mIsToggle = true;
	mAutoResizeButton.mTitleNormal = "auto resize off";
	mAutoResizeButton.mTitleEnabled = "auto resize on";
	mAutoResizeButton.mBounds = buttonRect;
	mWidgets.push_back( &mAutoResizeButton );

	Vec2f sliderSize( 200, 30 );
	Rectf selectorRect( getWindowWidth() - sliderSize.x - padding, padding, getWindowWidth() - padding, sliderSize.y * 3 + padding );
	mTestSelector.mSegments.push_back( "BufferPlayer" );
	mTestSelector.mSegments.push_back( "FilePlayer" );
	mTestSelector.mSegments.push_back( "recorder" );
	mTestSelector.mBounds = selectorRect;
	mWidgets.push_back( &mTestSelector );

	Rectf sliderRect( selectorRect.x1, selectorRect.y2 + padding, selectorRect.x2, selectorRect.y2 + padding + sliderSize.y );
//	Rectf sliderRect( getWindowWidth() - 200.0f, kPadding, getWindowWidth(), 50.0f );
	mGainSlider.mBounds = sliderRect;
	mGainSlider.mTitle = "Gain";
	mGainSlider.set( mGain->getValue() );
	mWidgets.push_back( &mGainSlider );

	sliderRect += Vec2f( 0, sliderRect.getHeight() + padding );
	mPanSlider.mBounds = sliderRect;
	mPanSlider.mTitle = "Pan";
	mPanSlider.set( mPan->getPos() );
	mWidgets.push_back( &mPanSlider );

	sliderRect += Vec2f( 0, sliderRect.getHeight() + padding );
	mLoopBeginSlider.mBounds = sliderRect;
	mLoopBeginSlider.mTitle = "Loop Begin";
	mLoopBeginSlider.mMax = (float)mSamplePlayer->getNumSeconds();
	mLoopBeginSlider.set( (float)mSamplePlayer->getLoopBeginTime() );
	mWidgets.push_back( &mLoopBeginSlider );

	sliderRect += Vec2f( 0, sliderRect.getHeight() + padding );
	mLoopEndSlider.mBounds = sliderRect;
	mLoopEndSlider.mTitle = "Loop End";
	mLoopEndSlider.mMax = (float)mSamplePlayer->getNumSeconds();
	mLoopEndSlider.set( (float)mSamplePlayer->getLoopEndTime() );
	mWidgets.push_back( &mLoopEndSlider );

	Vec2f xrunSize( 80, 26 );
	mUnderrunRect = Rectf( padding, getWindowHeight() - xrunSize.y - padding, xrunSize.x + padding, getWindowHeight() - padding );
	mOverrunRect = mUnderrunRect + Vec2f( xrunSize.x + padding, 0 );
	mRecorderOverrunRect = mOverrunRect + Vec2f( xrunSize.x + padding, 0 );

	getWindow()->getSignalMouseDown().connect( [this] ( MouseEvent &event ) { processTap( event.getPos() ); } );
	getWindow()->getSignalMouseDrag().connect( [this] ( MouseEvent &event ) { processDrag( event.getPos() ); } );
	getWindow()->getSignalTouchesBegan().connect( [this] ( TouchEvent &event ) { processTap( event.getTouches().front().getPos() ); } );
	getWindow()->getSignalTouchesMoved().connect( [this] ( TouchEvent &event ) {
		for( const TouchEvent::Touch &touch : getActiveTouches() )
			processDrag( touch.getPos() );
	} );

	gl::enableAlphaBlending();
}

void SamplePlayerTestApp::processDrag( Vec2i pos )
{
	if( mGainSlider.hitTest( pos ) )
		mGain->setValue( mGainSlider.mValueScaled );
	else if( mPanSlider.hitTest( pos ) )
		mPan->setPos( mPanSlider.mValueScaled );
	else if( mLoopBeginSlider.hitTest( pos ) )
		mSamplePlayer->setLoopBeginTime( mLoopBeginSlider.mValueScaled );
	else if( mLoopEndSlider.hitTest( pos ) )
		mSamplePlayer->setLoopEndTime( mLoopEndSlider.mValueScaled );
	else if( pos.y > getWindowCenter().y )
		seek( pos.x );
}

void SamplePlayerTestApp::processTap( Vec2i pos )
{
	if( mEnableSamplePlayerButton.hitTest( pos ) )
		mSamplePlayer->setEnabled( ! mSamplePlayer->isEnabled() );
	else if( mStartPlaybackButton.hitTest( pos ) )
		mSamplePlayer->start();
	else if( mLoopButton.hitTest( pos ) )
		mSamplePlayer->setLoopEnabled( ! mSamplePlayer->isLoopEnabled() );
	else if( mRecordButton.hitTest( pos ) ) {
		if( mRecordButton.mEnabled )
			mRecorder->start();
		else
			mRecorder->disable();
	}
	else if( mWriteButton.hitTest( pos ) )
		writeRecordedToFile();
	else if( mAutoResizeButton.hitTest( pos ) )
		;
	else if( mAsyncButton.hitTest( pos ) )
		;
	else if( pos.y > getWindowCenter().y )
		seek( pos.x );

	size_t currentIndex = mTestSelector.mCurrentSectionIndex;
	if( mTestSelector.hitTest( pos ) && currentIndex != mTestSelector.mCurrentSectionIndex ) {
		string currentTest = mTestSelector.currentSection();
		CI_LOG_V( "selected: " << currentTest );

		if( currentTest == "BufferPlayer" )
			setupBufferPlayer();
		if( currentTest == "FilePlayer" )
			setupFilePlayer();
		if( currentTest == "recorder" )
			setupBufferRecorder();
	}
}

void SamplePlayerTestApp::seek( size_t xPos )
{
	mSamplePlayer->seek( mSamplePlayer->getNumFrames() * xPos / getWindowWidth() );
}

void SamplePlayerTestApp::printBufferSamples( size_t xPos )
{
	auto bufferPlayer = dynamic_pointer_cast<audio2::BufferPlayer>( mSamplePlayer );
	if( ! bufferPlayer )
		return;

	auto buffer = bufferPlayer->getBuffer();
	size_t step = buffer->getNumFrames() / getWindowWidth();
	size_t xScaled = xPos * step;
	CI_LOG_V( "samples starting at " << xScaled << ":" );
	for( int i = 0; i < 100; i++ ) {
		if( buffer->getNumChannels() == 1 )
			console() << buffer->getChannel( 0 )[xScaled + i] << ", ";
		else
			console() << "[" << buffer->getChannel( 0 )[xScaled + i] << ", " << buffer->getChannel( 0 )[xScaled + i] << "], ";
	}
	console() << endl;

}

void SamplePlayerTestApp::mouseDown( MouseEvent event )
{
//	printBufferSamples( event.getX() );
}

void SamplePlayerTestApp::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_c )
		testConverter();
	if( event.getCode() == KeyEvent::KEY_w )
		testWrite();
	if( event.getCode() == KeyEvent::KEY_s )
		mSamplePlayer->seekToTime( 1.0 );
}

void SamplePlayerTestApp::fileDrop( FileDropEvent event )
{
	const fs::path &filePath = event.getFile( 0 );
	CI_LOG_V( "File dropped: " << filePath );

	setSourceFile( loadFile( filePath ) );
	mSamplePlayer->seek( 0 );

	CI_LOG_V( "output samplerate: " << mSourceFile->getSampleRate() );

	auto bufferPlayer = dynamic_pointer_cast<audio2::BufferPlayer>( mSamplePlayer );
	if( bufferPlayer ) {
		bufferPlayer->loadBuffer( mSourceFile );
		mWaveformPlot.load( bufferPlayer->getBuffer(), getWindowBounds() );
	}
	else {
		auto filePlayer = dynamic_pointer_cast<audio2::FilePlayer>( mSamplePlayer );
		CI_ASSERT_MSG( filePlayer, "expected sample player to be either BufferPlayer or FilePlayer" );

		filePlayer->setSourceFile( mSourceFile );
	}

	mLoopBeginSlider.mMax = mLoopEndSlider.mMax = (float)mSamplePlayer->getNumSeconds();

	CI_LOG_V( "loaded and set new source buffer, channels: " << mSourceFile->getNumChannels() << ", frames: " << mSourceFile->getNumFrames() );
	audio2::master()->printGraph();
}


void SamplePlayerTestApp::update()
{
	// light up rects if an xrun was detected
	const float xrunFadeTime = 1.3f;
	auto filePlayer = dynamic_pointer_cast<audio2::FilePlayer>( mSamplePlayer );
	if( filePlayer ) {
		if( filePlayer->getLastUnderrun() )
			timeline().apply( &mUnderrunFade, 1.0f, 0.0f, xrunFadeTime );
		if( filePlayer->getLastOverrun() )
			timeline().apply( &mOverrunFade, 1.0f, 0.0f, xrunFadeTime );
	}

	// print SamplePlayer start / stop times
	if( mSamplePlayerEnabledState != mSamplePlayer->isEnabled() ) {
		mSamplePlayerEnabledState = mSamplePlayer->isEnabled();
		string stateStr = mSamplePlayerEnabledState ? "started" : "stopped";
		CI_LOG_V( "mSamplePlayer " << stateStr << " at " << to_string( getElapsedSeconds() ) );
	}

	bool testIsRecorder = ( mTestSelector.currentSection() == "recorder" );
	mRecordButton.mHidden = mWriteButton.mHidden = mAutoResizeButton.mHidden = ! testIsRecorder;

	// test auto resizing the Recorder's buffer depending on how full it is
	if( testIsRecorder && mAutoResizeButton.mEnabled ) {
		CI_ASSERT( mRecorder );

		size_t writePos = mRecorder->getWritePosition();
		size_t numFrames = mRecorder->getNumFrames();

		if( writePos + mRecorder->getSampleRate() / 2 > numFrames ) {
			size_t resizeFrames = numFrames + mRecorder->getSampleRate();
			CI_LOG_V( "writePos: " << writePos << ", numFrames: " << numFrames << ", resizing frames to: " << resizeFrames );
			mRecorder->setNumFrames( resizeFrames );
		}

		if( mRecorder->getLastOverrun() )
			timeline().apply( &mRecorderOverrunFade, 1.0f, 0.0f, xrunFadeTime );
	}

}

void SamplePlayerTestApp::draw()
{
	gl::clear();

	if( mTestSelector.currentSection() == "recorder" ) {
		audio2::BufferRef recordedBuffer = mRecorder->getRecordedCopy();
		drawAudioBuffer( *recordedBuffer, getWindowBounds() );
	}
	else {
		auto bufferPlayer = dynamic_pointer_cast<audio2::BufferPlayer>( mSamplePlayer );
		if( bufferPlayer )
			mWaveformPlot.draw();
		else if( mScope && mScope->isInitialized() )
			drawAudioBuffer( mScope->getBuffer(), getWindowBounds() );

		float readPos = (float)getWindowWidth() * mSamplePlayer->getReadPosition() / mSamplePlayer->getNumFrames();
		gl::color( ColorA( 0, 1, 0, 0.7f ) );
		gl::drawSolidRoundedRect( Rectf( readPos - 2, 0, readPos + 2, (float)getWindowHeight() ), 2 );
	}

	if( mUnderrunFade > 0.0001f ) {
		gl::color( ColorA( 1, 0.5f, 0, mUnderrunFade ) );
		gl::drawSolidRect( mUnderrunRect );
		gl::drawStringCentered( "play underrun", mUnderrunRect.getCenter(), Color::black() );
	}
	if( mOverrunFade > 0.0001f ) {
		gl::color( ColorA( 1, 0.5f, 0, mOverrunFade ) );
		gl::drawSolidRect( mOverrunRect );
		gl::drawStringCentered( "play overrun", mOverrunRect.getCenter(), Color::black() );
	}

	if( mRecorderOverrunFade > 0.0001f ) {
		gl::color( ColorA( 1, 0.5f, 0, mRecorderOverrunFade ) );
		gl::drawSolidRect( mRecorderOverrunRect );
		gl::drawStringCentered( "rec overrun", mRecorderOverrunRect.getCenter(), Color::black() );
	}

	drawWidgets( mWidgets );
}

void SamplePlayerTestApp::testConverter()
{
	audio2::BufferRef audioBuffer = mSourceFile->loadBuffer();

	size_t destSampleRate = 48000;
	size_t destChannels = 1;
	size_t sourceMaxFramesPerBlock = 512;
	auto converter = audio2::dsp::Converter::create( mSourceFile->getSampleRate(), destSampleRate, mSourceFile->getNumChannels(), destChannels, sourceMaxFramesPerBlock );

	CI_LOG_V( "FROM samplerate: " << converter->getSourceSampleRate() << ", channels: " << converter->getSourceNumChannels() << ", frames per block: " << converter->getSourceMaxFramesPerBlock() );
	CI_LOG_V( "TO samplerate: " << converter->getDestSampleRate() << ", channels: " << converter->getDestNumChannels() << ", frames per block: " << converter->getDestMaxFramesPerBlock() );

	audio2::BufferDynamic sourceBuffer( converter->getSourceMaxFramesPerBlock(), converter->getSourceNumChannels() );
	audio2::Buffer destBuffer( converter->getDestMaxFramesPerBlock(), converter->getDestNumChannels() );

	audio2::TargetFileRef target = audio2::TargetFile::create( "resampled.wav", converter->getDestSampleRate(), converter->getDestNumChannels() );

	size_t numFramesConverted = 0;

	Timer timer( true );

	while( numFramesConverted < audioBuffer->getNumFrames() ) {
		if( audioBuffer->getNumFrames() - numFramesConverted > sourceMaxFramesPerBlock ) {
			for( size_t ch = 0; ch < audioBuffer->getNumChannels(); ch++ )
				memcpy( sourceBuffer.getChannel( ch ), audioBuffer->getChannel( ch ) + numFramesConverted, sourceMaxFramesPerBlock * sizeof( float ) );
				//copy( audioBuffer->getChannel( ch ) + numFramesConverted, audioBuffer->getChannel( ch ) + numFramesConverted + sourceMaxFramesPerBlock, sourceBuffer.getChannel( ch ) );
		}
		else {
			// EOF, shrink sourceBuffer to match remaining
			sourceBuffer.setNumFrames( audioBuffer->getNumFrames() - numFramesConverted );
			for( size_t ch = 0; ch < audioBuffer->getNumChannels(); ch++ )
				memcpy( sourceBuffer.getChannel( ch ), audioBuffer->getChannel( ch ) + numFramesConverted, audioBuffer->getNumFrames() * sizeof( float ) );
				//copy( audioBuffer->getChannel( ch ) + numFramesConverted, audioBuffer->getChannel( ch ) + audioBuffer->getNumFrames(), sourceBuffer.getChannel( ch ) );
		}

		pair<size_t, size_t> result = converter->convert( &sourceBuffer, &destBuffer );
		numFramesConverted += result.first;

		target->write( &destBuffer, 0, result.second );
	}

	CI_LOG_V( "seconds: " << timer.getSeconds() );
}

void SamplePlayerTestApp::testWrite()
{
	audio2::BufferRef audioBuffer = mSourceFile->loadBuffer();

	try {
		const string fileName = "out.wav";
		audio2::TargetFileRef target = audio2::TargetFile::create( fileName, mSourceFile->getSampleRate(), mSourceFile->getNumChannels() ); // INT_16
		//	audio2::TargetFileRef target = audio2::TargetFile::create( fileName, mSourceFile->getSampleRate(), mSourceFile->getNumChannels(), audio2::SampleType::FLOAT_32 );

		CI_LOG_V( "writing " << audioBuffer->getNumFrames() << " frames at samplerate: " << mSourceFile->getSampleRate() << ", num channels: " << mSourceFile->getNumChannels() );
		target->write( audioBuffer.get() );
		CI_LOG_V( "...complete." );

//		size_t writeCount = 0;
//		while( numFramesConverted < audioBuffer->getNumFrames() ) {
//			for( size_t ch = 0; ch < audioBuffer->getNumChannels(); ch++ )
//				copy( audioBuffer->getChannel( ch ) + writeCount, audioBuffer->getChannel( ch ) + writeCount + sourceFormat.getFramesPerBlock(), sourceBuffer.getChannel( ch ) );
//		}
	}
	catch( audio2::AudioFileExc &exc ) {
		CI_LOG_E( "AudioFileExc, what: " << exc.what() );
	}
}

CINDER_APP_NATIVE( SamplePlayerTestApp, RendererGl )
