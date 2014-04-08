//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#ifndef rcMODIFIER_KEYS_H
#define rcMODIFIER_KEYS_H

class rcMainWindow;
class QMouseEvent;
class QKeyEvent;

// Utility namespace for getting the current status of
// certain keyboard modifier keys (ctrl, shift, alt).

class rcModifierKeys
{
public:
    // is the shift key pressed?
    static bool isShiftDown( QMouseEvent* mouseEvent );
    static bool isShiftDown( QKeyEvent* keyEvent );
    
    // is the ctrl key pressed?
    static bool isCtrlDown( QMouseEvent* mouseEvent );
    static bool isCtrlDown( QKeyEvent* keyEvent );

    // is the alt key pressed?
    static bool isAltDown( QMouseEvent* mouseEvent );
    static bool isAltDown( QKeyEvent* keyEvent );
};

#endif // rcMODIFIER_KEYS_H
