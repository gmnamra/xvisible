// rc_modifierkeys.cpp


#include <QtGui/QtGui>
#include <QtCore/QtCore>

#include "rc_modifierkeys.h"

// is the shift key pressed?
bool rcModifierKeys::isShiftDown( QMouseEvent* mouseEvent )
{
    if ( mouseEvent ) {
        if ( mouseEvent->state() & Qt::ShiftModifier )
            return true;
    }

    return false;
}

bool rcModifierKeys::isShiftDown( QKeyEvent* keyEvent )
{
    if ( keyEvent ) {
        if ( keyEvent->state() & Qt::ShiftButton )
            return true;
    }

    return false;
}

// is the ctrl key pressed?
bool rcModifierKeys::isCtrlDown( QMouseEvent* mouseEvent )
{
    if ( mouseEvent ) {
        if ( mouseEvent->state() & Qt::ControlModifier )
            return true;
    }

    return false;
}

bool rcModifierKeys::isCtrlDown( QKeyEvent* keyEvent )
{
    if ( keyEvent ) {
        if ( keyEvent->state() & Qt::ControlButton )
            return true;
    }

    return false;
}

// is the alt key pressed?
bool rcModifierKeys::isAltDown( QMouseEvent* mouseEvent )
{
    if ( mouseEvent ) {
        if ( mouseEvent->state() & Qt::AltModifier )
            return true;
    }

    return false;
}

bool rcModifierKeys::isAltDown( QKeyEvent* keyEvent )
{
    if ( keyEvent ) {
        if ( keyEvent->state() & Qt::AltButton )
            return true;
    }

    return false;
}



