#ifndef rcCONTROL_PANEL_H
#define rcCONTROL_PANEL_H

#include <q3frame.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QLabel>

#include <rc_timestamp.h>
#include <rc_model.h>

class QLabel;

class rcControlPanel : public Q3Frame
{
    Q_OBJECT

public:
    rcControlPanel( QWidget* parent=0, const char* name=0, Qt::WFlags f=0 );
    ~rcControlPanel();

public slots:
	void updateElapsedTime( const rcTimestamp& time );
	void updateCursorTime( const rcTimestamp& time );
	void updateState( rcExperimentState state );
    void updateAnalysisRect( const rcRect& rect );
    void settingChanged();
    void inputSource( int i );
    void updateCursorTime();
  void updateSelectionState (bool trueIsSelect);  

signals:

protected:

private:
	QString timestampToString( const rcTimestamp& time );

	QLabel*	_elapsedTime;
	QLabel*	_cursorTime;
    QPushButton* _startButton;
    QPushButton* _stopButton;
    QPushButton* _processButton;
    QPushButton* _stopAnalysisButton;
    rcRect       _analysisRect;
    rcExperimentState _lastState;
  bool _selectionState;
};


#endif // rcCONTROL_PANEL_H
