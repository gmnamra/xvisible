/*
 *  rc_qvideograbber.cpp
 *
 *  Created by Peter Roberts on Thu Oct 17 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include <rc_types.h>
#include <rc_qvideograbber.h>
#include <rc_qtime.h>
#include <rc_timestamp.h>
#include <rc_resource_ctrl.h>
#include <sys/wait.h>
#include <unistd.h>

/* Support for real time video acquisition, using quicktime API. */

rcQVideoGrabber::rcQVideoGrabber(char* progName, int arg1, char* arg2,
                                 uint32 sz,
                                 uint8 childControlsBufferFirst)
        : _shmemCtrlP(new rcExecWithShmem(progName, arg1, arg2, sz,
                                          childControlsBufferFirst)),
          _lastError(eFrameErrorOK), _isValid(TRUE), _frameCount(-1)
{
    if (!_shmemCtrlP)
    {
        _lastError = eFrameErrorOutOfMemory;
        _isValid = FALSE;
    }
    
    switch (_shmemCtrlP->getCreationError())
    {
        case eNoError:
            break;
            
        case eShmemInit:
        case eSemaphoreInit:
            _lastError = eFrameErrorSystemResources;
            _isValid = FALSE;
            break;
            
        case eProcessFork:
            _lastError = eFrameErrorUnknown;
            _isValid = FALSE;
            break;
            
        default:
            rmAssert(0);
    }
}

rcQVideoGrabber::~rcQVideoGrabber()
{
  /* Give child a second to clean things up.
   */
    fprintf(stderr, "waiting...\n");
    for (int i = 0; (i < 1000) && !_shmemCtrlP->isChildDone(); i++)
        usleep(1000);
    fprintf(stderr, "Done waiting\n");
    
    /* Send terminate signal to child and wait for child process to end
     */
    pid_t pid = _shmemCtrlP->getChildPID();
    if ( pid > 0 ) {
        kill(_shmemCtrlP->getChildPID(), SIGKILL);
        wait(NULL);
    }
    
    delete _shmemCtrlP;
}

rcFrameGrabberStatus rcQVideoGrabber::getAcqInfo(rcAcqInfo& acqInfo,
                                                 bool isBlocking,
                                                 bool releaseMemory)
{
    if (!_isValid)
        return eFrameStatusError;
    
    rcSharedMemoryUser& consumer = _shmemCtrlP->getShmemUser();
    rcSharedMemError err;
    
    rcVideoSharedMemLayout* acqData =
        (rcVideoSharedMemLayout*)consumer.acquireSharedMemory(err, isBlocking);
    
    if (!acqData)
    {
        fprintf(stderr, "Parent: error acquiring shared data. err: %d\n", err);
        perror("Parent: ");
        
        _lastError = eFrameErrorIPC;
        return eFrameStatusError;
    }
    
    /* Read acq state info from shared memory.
     */
    rmAssert(sizeof(acqInfo) == sizeof(acqData->videoState));
    memmove(&acqInfo, &(acqData->videoState), sizeof(acqInfo));
    
    /* All data read from shared buffer. Free it.
     */
    if (releaseMemory)
    {
        switch (err = consumer.releaseSharedMemory())
        {
            case rcSharedMemNoError:
                break;
                
            default:
                fprintf(stderr, "Parent: error freeing shared data, err: %d\n", err);
                perror("Parent: ");
                _lastError = eFrameErrorIPC;
                return eFrameStatusError;
                
        } // End of: switch (consumer.releaseSharedMemory())
    } // End of: if (releaseMemory)
    
    return eFrameStatusOK;
}

rcFrameGrabberStatus rcQVideoGrabber::setAcqControl(rcAcqControl& acqCtrl,
                                                    bool isBlocking,
                                                    bool releaseMemory)
{
    if (!_isValid)
        return eFrameStatusError;
    
    rcSharedMemoryUser& consumer = _shmemCtrlP->getShmemUser();
    rcSharedMemError err;
    
    rcVideoSharedMemLayout* acqData =
        (rcVideoSharedMemLayout*)consumer.acquireSharedMemory(err, isBlocking);
    
    if (!acqData)
    {
        fprintf(stderr, "Parent: error acquiring shared data. err: %d\n", err);
        perror("Parent: ");
        
        _lastError = eFrameErrorIPC;
        return eFrameStatusError;
    }
    
    /* Copy acq control info into shared memory.
     */
    rmAssert(sizeof(acqCtrl) == sizeof(acqData->videoControl));
    memmove(&(acqData->videoControl), &acqCtrl, sizeof(acqCtrl));
    
    /* All data read from shared buffer. Free it.
     */
    if (releaseMemory)
    {
        switch (err = consumer.releaseSharedMemory())
        {
            case rcSharedMemNoError:
                break;
                
            default:
                fprintf(stderr, "Parent: error freeing shared data, err: %d\n", err);
                perror("Parent: ");
                _lastError = eFrameErrorIPC;
                return eFrameStatusError;
                
        } // End of: switch (consumer.releaseSharedMemory())
    } // End of: if (releaseMemory)
    
    return eFrameStatusOK;
}

void rcQVideoGrabber::setFrameCount(int32 frameCount)
{
    _frameCount = frameCount;
}

void rcQVideoGrabber::addBuffer(rcSharedFrameBufPtr& buf)
{
    _buffers.push_back(buf);
}

rcFrameGrabberError rcQVideoGrabber::getLastError() const
{
    return _lastError;
}

bool rcQVideoGrabber::start()
{
    return _isValid; // Will have to do until I add IPC code to really do this
}

bool rcQVideoGrabber::stop()
{
    return _isValid; // Will have to do until I add IPC code to really do this
}

int32 rcQVideoGrabber::frameCount()
{
    return _frameCount;
}

rcFrameGrabberStatus rcQVideoGrabber::getNextFrame(rcSharedFrameBufPtr& fptr,
                                                   bool isBlocking)
{
    if (!_isValid)
        return eFrameStatusError;
    
    if (_frameCount > 0)
        _frameCount--;
    else if (_frameCount == 0)
        return eFrameStatusEOF;
    
    /* fgStatus is a place to collect errors that happen before shared
     * resources have been freed.
     */
    rcFrameGrabberStatus fgStatus = eFrameStatusOK;
    
    rcSharedMemoryUser& consumer = _shmemCtrlP->getShmemUser();
    rcSharedMemError err;
    bool needFrame = true;
    
    while (needFrame)
    {
        rcVideoSharedMemLayout* imgData =
            (rcVideoSharedMemLayout*)consumer.acquireSharedMemory(err, isBlocking);
        
        if (!imgData)
        {
            if (err == rcSharedMemNotAvailable)
            {
                _lastError = eFrameErrorFrameNotAvailable;
                return eFrameStatusNoFrame;
            }
            
            fprintf(stderr, "Parent: error acquiring shared data. err: %d\n", err);
            perror("Parent: ");
            
            _lastError = eFrameErrorIPC;
            return eFrameStatusError;
        }
        
        if ((imgData->videoState.acqFlags & ~rcACQFLAGS_FRAMESKIPPED) ==
            rcACQFLAGS_FRAMEAVAIL)
        {
            
            /* Copy pixels from shared data into a new frame buffer created from the
             * heap.
             */
            try
            {
                /* First, try to find a buffer that is currently only
                 * referenced by the rcSharedFrameBufPtr _buffers in
                 * _buffers. Its reference count therefore should be 1.
                 */
                vector<rcSharedFrameBufPtr>::iterator bufptr = _buffers.begin();
                for ( ; bufptr != _buffers.end(); bufptr++)
                    if ((*bufptr).refCount() == 1)
                        break;
                
                if (bufptr == _buffers.end()) {
                    fptr = rcSharedFrameBufPtr(new rcFrame(imgData->rawPixels,
                                                            imgData->rowUpdate,
                                                            imgData->width,
                                                            imgData->height,
                                                            imgData->pixelDepth,
                                                            imgData->isGray));
                    // Init color map
                    if (imgData->pixelDepth == rcPixel8)
                    {
                        fptr->initGrayColorMap();
                    }
                    else if (imgData->pixelDepth == rcPixel16)
                        rfFillColorMap(0, fptr);
                }
                else {
                    fptr = *bufptr;
                    fptr->loadImage(imgData->rawPixels, imgData->rowUpdate, imgData->width,
                                    imgData->height, imgData->pixelDepth, imgData->isGray);
                }
                
                if (!fptr) {
                    _lastError = eFrameErrorSystemResources;
                    fgStatus = eFrameStatusError;
                }
                else
                    fptr->setTimestamp(imgData->timestamp);
            }
            catch (...)
            {
                _lastError = eFrameErrorSystemResources;
                fgStatus = eFrameStatusError;
            }
            needFrame = false;
        }
        else if (imgData->videoState.acqFlags & rcACQFLAGS_FRAMEERROR)
        {
            _lastError = eFrameErrorCameraCapture;
            fgStatus = eFrameStatusError;
        }
#ifdef DEBUGGING
        else
            fprintf(stderr, "xyzzy looping...\n");
#endif
        
        /* All data read from shared buffer. Free it.
         */
        switch (err = consumer.releaseSharedMemory())
        {
            case rcSharedMemNoError:
                break;
                
            default:
                fprintf(stderr, "Parent: error freeing shared data, err: %d\n", err);
                perror("Parent: ");
                _lastError = eFrameErrorIPC;
                fgStatus = eFrameStatusError;
                
        } // End of: switch (consumer.releaseSharedMemory())
        
        if (fgStatus != eFrameStatusOK)
            return fgStatus;
    } // End of: while (needFrame)
       
    return eFrameStatusOK;
}


// Get name of input source, ie. file name, camera name etc.
const std::string rcQVideoGrabber::getInputSourceName()
{
    // TODO: add camera model name here if it can be queried from
    // the device
    std::string cameraName = "Fireware camera";
    
    return cameraName;
}
