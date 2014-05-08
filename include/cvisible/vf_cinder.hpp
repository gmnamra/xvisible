
#ifndef __VF_CINDER_UTILS_
#define __VF_CINDER_UTILS_


#include "vf_utils.hpp"
#include "cinder/Filesystem.h"
#include "cinder/DataSource.h"
#include "cinder/audio2/WaveformType.h"
#include "cinder/audio2/Buffer.h"
#include "cinder/audio2/Exception.h"

#include <vector>
#include <tuple>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

using namespace vf_utils;
using namespace cinder;
using namespace audio2;
using namespace std;


namespace vf_cinder

{
    // From a Visible results file ( or a csv file with one or more columns of float values ) via a Buffer, a DataSourceBufferRef is
    // cretaed. A VisibleAudioSource is created from a DataSourceBufferRef. 
    
    
    class VisibleAudioSource : public SourceFile
    {
                
    public:
        
        static DataSourcePathRef create (const std::string& fqfn)
        {
            return  DataSourcePath::create (fs::path  (fqfn));
        }
        
        VisibleAudioSource ( const DataSourceRef &dataSource) : SourceFile ()
        {
            m_column_select = -1; // column_select; // after init, this is the data column selected 
            mDataSource = dataSource;
            init();
        }
        
        
        virtual ~VisibleAudioSource()	{}
        
        size_t	read( audio2::Buffer *buffer ) override 
        {
            CI_ASSERT( buffer->getNumChannels() == mNumChannels );
            CI_ASSERT( mReadPos < mNumFrames );
            
            size_t numRead;
            size_t numFramesNeeded = std::min( mNumFrames - mReadPos, std::min( mMaxFramesPerRead, buffer->getNumFrames() ) );
            numRead = performRead( buffer, 0, numFramesNeeded );
            
            mReadPos += numRead;
            return numRead;
        }

        void	setOutputFormat( size_t outputSampleRate, size_t outputNumChannels = 0 ) override {} 
        
        //! Returns a clone of this Source.
        virtual SourceFileRef clone() const override
        {
            std::shared_ptr<VisibleAudioSource> result( new VisibleAudioSource (mDataSource ));
            result->init();
            return result;
        }
        
        
        //! Loads and returns the entire contents of this VisibleAudioSource. \return a BufferRef containing the file contents.
        BufferRef loadBuffer()
        {
            BufferRef result = make_shared<audio2::Buffer>( mNumFrames, 1);
            const std::vector<float>& data = mdat[m_column_select];
            std::memmove( result->getChannel( 0 ), & data[0], mNumFrames  * sizeof(float) );
            return result;
        }
        //! Seek the read position to \a readPositionFrames
        void seek( size_t readPositionFrames ) {}
        //! Seek to read position \a readPositionSeconds
        void seekToTime( double readPositionSeconds )	{ return seek( size_t( readPositionSeconds * (double)getSampleRate() ) ); }
        
        //! Returns the length in seconds.
        double getNumSeconds() const					{ return (double)mNumFrames / (double)mSampleRate; }
        //! Returns the length in frames.
        size_t	getNumFrames() const					{ return mNumFrames; }
        
    protected:

        VisibleAudioSource ();
        
        size_t performRead( audio2::Buffer *buffer, size_t bufferFrameOffset, size_t numFramesNeeded ) override 
        {
            CI_ASSERT( buffer->getNumFrames() >= bufferFrameOffset + numFramesNeeded );
            const std::vector<float>& data = mdat[m_column_select];
            size_t offset = bufferFrameOffset;
            memcpy( buffer->getChannel( 0 ) + offset, &data[0], numFramesNeeded * sizeof( float ) );
            mReadPos += numFramesNeeded;
            return static_cast<size_t>( numFramesNeeded );
        }
        //! Implement to perform seek. \a readPositionFrames is in native file units.
        virtual void performSeek( size_t readPositionFrames ) {}
        
        //        size_t mNumFrames, mFileNumFrames, mReadPos;
        std::vector<vector<float> > mdat;
        ci::DataSourceRef	mDataSource;
        int m_column_select;
        
        //! Creates a new VisibleAudioSource from \a dataSource, setting the samplerate and num channels to match the native values.
        void init () // @todo add params: fps, etc 
        {
            CI_ASSERT( mDataSource );
            if( mDataSource->isFilePath() )
            {
                const std::string& fqfn = mDataSource->getFilePath ().string ();
                mdat.resize (0);
                bool c2v_ok = vf_utils::csv::csv2vectors ( fqfn, mdat, false, true, true);
                if ( ! c2v_ok ) 
                    throw AudioFileExc( string( "Failed to validate file with error: " ), -1 );
                
                int columns = mdat.size ();
                int rows = mdat[0].size();
                // only handle case of long columns of data
                if (columns >= rows) 
                    throw AudioFileExc( string( "Failed to validate file with error: " ), -2 );
                
                m_column_select = (m_column_select >= 0 && m_column_select < columns) ? m_column_select : 0;
                mNumFrames = mFileNumFrames = rows;
                mReadPos = 0;
                mNumChannels = 1;
             }
            else
                throw AudioFileExc( string( "Failed to open file with error: " ), -3 );
            
        }
        
    };
    
    //! Convenience method for loading a VisibleAudioSource from \a dataSource. \return VisibleAudioSourceRef. \see VisibleAudioSource::create()
    //    inline VisibleAudioSourceRef	load( const DataSourceRef &dataSource )	{ return VisibleAudioSource::create( dataSource ); }
    
    
}



#endif
