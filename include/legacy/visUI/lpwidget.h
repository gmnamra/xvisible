#ifndef _LPWIDGET_H
#define _LPWIDGET_H

#include <QtGui/QWidget>
#include <QtCore/QVector>
#include <QtCore/QPoint>
#include <lightplot2d.h>
#include <boost/shared_ptr.hpp>
#include <deque>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/qsplitter.h>
#include <QtGui/QWidget>
#include <QtGui/QDial>
#include <QtGui/QLabel>
#include <assert.h>
#include <rc_signal1d.hpp>
#include <rc_contraction_signal.hpp>
#include <rc_uitypes.h>

using namespace std;

typedef boost::shared_ptr<CurveData> SharedCurveDataRef;

class LPWidget : public QWidget
{
    Q_OBJECT
    
public:
    LPWidget(QWidget *parent = 0, const CurveData * cdata = 0);
    
    void new_plot (SharedCurveDataRef& );
    void new_plot ( const CurveData * ); 
    int half_window () const { return m_half_window; }
    
    ~LPWidget();
    
    // The "slots" and "signals" sections in a class definition should only contain functions;
    // neither types nor member variables.
    public slots:    
    void add_contractions (int);

    
private:
       
    std::deque<double> m_signal;
    std::deque<double> m_dsdt;
    std::deque<double> m_zcm;
    
    void compute_2nd_derivative ();
    int get_all_candidates ();

    void demo ();
    void setupUi(QWidget *Form);
    void retranslateUi(QWidget *Form);
    LightPlot2D *_plot;
    QDial  *dial;
    QSplitter *splitter;
    QLabel *contraction_threshold_input;
    std::vector<extrema_pos> m_valleys;
    std::vector<extrema_pos> m_peaks;

    ContractionSignal<double, std::deque> m_csignal;
    valley_detector<double, std::deque> m_vd;    
    second_derivative_producer<double, std::deque> m_sdp;

    int m_cdata_index;
    int m_half_window;
    
    
    
};

#endif // _LPWIDGET_H
