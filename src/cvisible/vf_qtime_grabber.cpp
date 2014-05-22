
#include "vf_cinder_qtime_grabber_impl.hpp"

vf_utils::qtime_support::CinderQtimeGrabber::CinderQtimeGrabber ( const std::string fileName,   // Input file
                   double frameInterval , int32  startAfterFrame, int32  frames)
{
    _impl = boost::shared_ptr<qtfgImpl> ( new qtfgImpl (fileName, frameInterval, startAfterFrame, frames) );
}

bool vf_utils::qtime_support::CinderQtimeGrabber::isValid() const { return _impl->isValid (); }
 bool vf_utils::qtime_support::CinderQtimeGrabber::contentValid() const { return _impl->contentValid (); }
bool vf_utils::qtime_support::CinderQtimeGrabber::start() { return _impl->start (); }
bool vf_utils::qtime_support::CinderQtimeGrabber::stop() { return _impl->stop (); }
int32 vf_utils::qtime_support::CinderQtimeGrabber::frameCount() { return _impl->frameCount (); }
int32 vf_utils::qtime_support::CinderQtimeGrabber::cacheSize() { return _impl->cacheSize (); }
rcFrameGrabberStatus vf_utils::qtime_support::CinderQtimeGrabber::getNextFrame( rcSharedFrameBufPtr& ptr, bool isBlocking )
{
   return _impl->getNextFrame (ptr, isBlocking );
}

const std::string vf_utils::qtime_support::CinderQtimeGrabber::getInputSourceName() {  return _impl->getInputSourceName (); }

rcFrameGrabberError vf_utils::qtime_support::CinderQtimeGrabber::getLastError() const
{
    return _impl->getLastError ();
}

void vf_utils::qtime_support::CinderQtimeGrabber::setLastError( rcFrameGrabberError error )
{
    _impl->setLastError (error);
}

double vf_utils::qtime_support::CinderQtimeGrabber::frame_duration  () const { return _impl->frame_duration(); }

vf_utils::qtime_support::CinderQtimeGrabber::~CinderQtimeGrabber() { }

const std::ostream& vf_utils::qtime_support::CinderQtimeGrabber::print_to_ (std::ostream&  outs) const
{
    return _impl->print_to_ (outs);
}
