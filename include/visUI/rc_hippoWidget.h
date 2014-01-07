// @file
#ifndef RCHIPPOWIDGET_H
#define RCHIPPOWIDGET_H

#include <qt/QtViewWidget.h>

#include <rc_window.h>
#include <rc_math.h>
#include <rc_writermanager.h>

#include "rc_modeldomain.h"


#include <datasrcs/NTupleController.h>
#include <controllers/DisplayController.h>
#include <datasrcs/NTuple.h>
#include <plotters/PlotterBase.h>
#include <qt/Inspector.h>
#include <qt/PlotterEvent.h>
#include <qt/CanvasWindow.h>


class rcHippoWidget : public QtViewWidget
{
 Q_OBJECT

 public:
  rcHippoWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
  ~rcHippoWidget();

public slots:
  void updateDisplay( void );

 protected:

 private:
  Inspector* m_inspector;
  vector<DataSource *> m_trackSources;
  vector<PlotterBase *> m_trackPlotters;
  DataSource * rcTrackToNTuple (rcTrack * track, int32 groupNumber, 
				int32 trackNumber, int32 semantic);
  void init();


};

#endif // RCHIPPOWIDGET_H
