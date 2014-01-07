/******************************************************************************
 *   Copyright (c) 2003 Reify Corp. All Rights reserved.
 *
 *	$Id: rc_cellinfowidget.h 7179 2011-02-05 22:25:05Z arman $
 *
 *  Cell info display widget
 *
 ******************************************************************************/

#ifndef _rcCELL_INFO_WIDGET_H_
#define _rcCELL_INFO_WIDGET_H_

#include <qwidget.h>
//Added by qt3to4:
#include <QLabel>
#include <Q3BoxLayout>

#include <rc_timestamp.h>
#include <rc_model.h>

class QLabel;
class Q3BoxLayout;

class rcCellInfoWidget : public QWidget
{
    Q_OBJECT

public:
    rcCellInfoWidget( QWidget* parent );
    ~rcCellInfoWidget();

public slots:
	void updateCursorTime( const rcTimestamp& time ); 
	void updateState( rcExperimentState state );
    void updateDisplay( void );
    void updateSettings( void );
    
signals:

private:
    // called to update the value label, etc.
    void updateTrackDisplay( void );
    Q3BoxLayout*     mTopLayout;
    Q3BoxLayout*     mDataLayout;
    QLabel*         mDataName;
    QLabel*         mDataValue;
    QLabel*         mDataComment;
    rcTimestamp     mLastUpdateTime;
};


#endif // _rcCELL_INFO_WIDGET_H_

