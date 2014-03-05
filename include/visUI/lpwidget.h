#ifndef _LPWIDGET_H
#define _LPWIDGET_H

#include <QtGui/QWidget>
#include <QVector>
#include <QPoint>
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
using namespace std;

class CurveData2d 
{
public:
    CurveData2d (deque<deque<double> >& sm, uint32 matrix_size) :
    m_sm (sm), m_matrix (matrix_size)
    {
        
    }
    
    void mean_profile (int pos, int length, deque<float>& profile)
    {
        profile.resize (m_matrix);
        // calculate the end. Limit to the end
        int after  = pos + length - 1;
        after = after >= (int) m_matrix ? (m_matrix - 1) : after;
        assert (after > 0);
        uint32 uafter = (uint32) after;        
        for (uint32 i = pos; i < uafter; i++)
        {
            assert(m_sm[i].size() == m_matrix);
            profile[i-pos] = m_sm[i][i];
        }
        
        for (uint32 i = pos; i < uafter; i++)
            for (uint32 j = (i+1); j < uafter; j++)
            {
                profile[i] += m_sm[i][j];
                profile[j] += m_sm[i][j];
            }
        
        for (uint32 ii = 0; ii < profile.size (); ii++)
        {
            profile [ii] /= m_matrix;
        }
    }
    
    void mean_profile (deque<float>& profile)
    {
        mean_profile (0, m_matrix, profile);
    }
    
private:
    deque<deque<double> > m_sm;
    uint32 m_matrix;

};


typedef boost::shared_ptr<CurveData> SharedCurveDataRef;

class LPWidget : public QWidget
{
    Q_OBJECT
    
public:
    LPWidget(QWidget *parent = 0, const CurveData * cdata = 0);
    void new_plot (SharedCurveDataRef& );
    void new_plot ( const CurveData * ); 
    
public Q_SLOTS:    
    void add_contractions (int);
    
    ~LPWidget();
private:
    peak_detector::Container m_signal;
    peak_detector::Container m_dsdt;
    peak_detector::Container m_zcm;
    
    void demo ();
    void setupUi(QWidget *Form);
    void retranslateUi(QWidget *Form);
    LightPlot2D *_plot;
    QDial  *dial;
    QSplitter *splitter;
    QLabel *contraction_threshold_input;
    std::vector<peak_detector::peak_pos> m_valleys;
    std::vector<peak_detector::peak_pos> m_peaks;
    
};

#endif // _LPWIDGET_H
