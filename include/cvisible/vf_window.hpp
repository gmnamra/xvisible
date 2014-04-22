#include <cinder/Channel.h>
#include <cinder/Area.h>

ci::Channel8u* rcWindow::new_channel () const
{
    if (!isBound()) return 0;
    ci::Channel8u* ch8 = new ci::Channel8u ( width(), height () );

    const cinder::Area clippedArea = ch8->getBounds();
    int32_t rowBytes = ch8->getRowBytes();

    for( int32_t y = clippedArea.getY1(); y < clippedArea.getY2(); ++y )
    {
        uint8 *dstPtr = reinterpret_cast<uint8*>( reinterpret_cast<uint8_t*>( ch8->getData() + clippedArea.getX1() ) + y * rowBytes );
        const uint8 *srcPtr = rowPointer(y);
        for( int32_t x = 0; x < clippedArea.getWidth(); ++x )
        {
            *dstPtr++ = *srcPtr++;
        }
    }

    return ch8;

}
