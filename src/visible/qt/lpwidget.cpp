#include "lpwidget.h"
#include <lightplot2d.h>
#include <QVBoxLayout>


LPWidget::LPWidget(QWidget *parent, const CurveData*  cref )
: QWidget (parent, Qt::WDestructiveClose | Qt::WResizeNoErase)
{
    if (cref)
    {
        uint32 size = cref->data().size ();
        const QVector<QPointF>& data = cref->data ();

        m_signal.resize (size);
        for (uint32 ss = 0; ss < size; ss++)
            m_signal[ss] = data[ss].y();
        
        new_plot (cref );
        update ();
    }
}

void LPWidget::new_plot(SharedCurveDataRef & cref)
{
    if ( cref )
    {
        CurveData* cd0 = cref.get();
        new_plot (cd0);
     }    
}

void LPWidget::new_plot(const CurveData*  cref)
{
    if ( cref != 0 )
    {
        const QVector<QPointF>& pvals = cref->data ();
        
        CurveData cdata ( * cref );
        CurveData* cd0 = & cdata;
        cd0->setColor(Qt::red);
        _plot = new LightPlot2D;
        _plot->addCurveData(*cref);
        
        _plot->setMinY(0.0);
        _plot->setMaxY(1.0);
        _plot->setMinX(0.0);
        
        _plot->setNameXAxis("<i><font color=#CD5C5C>Axis</font> </i><font color=#708090>X</font> :)");
        _plot->setLegendOpacity(220);
        
        _plot->setLegendFont(QFont("Serif", 12));
        setupUi (this);
    }
}

void LPWidget::add_contractions (int thr)
{
    int half_window = 10;
    second_derivative_producer<double, std::deque> sdp;
    sdp.operator()(m_signal, m_dsdt, m_zcm);
    m_valleys.clear ();
    m_peaks.clear ();
    peak_detector<double, std::deque> pk;
    valley_detector<double, std::deque> vd;    
    pk.operator() (m_zcm, m_peaks, half_window);
    vd.operator () (m_signal, m_valleys, half_window);
    
    //   QVector<QPointF> data;
    //    for (uint32 ii = 0; ii < mini.size(); ii++)
    //       data.push_back (QPointF(mini[ii], m_signal[mini[ii]]));

    //  QString legend = QString::fromUtf8("Contractions");
    //  CurveData* contractions = new CurveData (data,legend );
    //   _plot->addCurveData (*contractions);
    //   update ();
   
    
}
LPWidget::~LPWidget()
{
    
}

    void LPWidget::setupUi(QWidget *Form)
{
    if (Form->objectName().isEmpty())
        Form->setObjectName(QString::fromUtf8("Form"));
    Form->resize(182, 220);
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(Form->sizePolicy().hasHeightForWidth());
    Form->setSizePolicy(sizePolicy);
    Form->setFocusPolicy(Qt::TabFocus);
    Form->setAutoFillBackground(true);

    QHBoxLayout *mainLayout = new QHBoxLayout(Form);    

    splitter = new QSplitter (Form);

    QVBoxLayout *dialLayout = new QVBoxLayout;
    
    splitter->setObjectName(QString::fromUtf8("splitter")); 
    splitter->setOrientation(Qt::Horizontal);

    dial = new QDial (Form);
    dial->setObjectName(QString::fromUtf8("dial"));
    dial->setGeometry(QRect(20, 20, 141, 111));
    dial->setMouseTracking(true);
    dial->setInvertedAppearance(false);
    dial->setInvertedControls(false);
    dial->setWrapping(true);
    dial->setNotchesVisible(true);

    contraction_threshold_input = new QLabel(Form);
    contraction_threshold_input->setObjectName(QString::fromUtf8("contraction_threshold_input"));
    contraction_threshold_input->setGeometry(QRect(20, 160, 141, 31));
    contraction_threshold_input->setInputMethodHints(Qt::ImhFormattedNumbersOnly);
    contraction_threshold_input->setFrameShape(QFrame::StyledPanel);
    contraction_threshold_input->setFrameShadow(QFrame::Sunken);
    contraction_threshold_input->setLineWidth(3);
    contraction_threshold_input->setMidLineWidth(2);
    contraction_threshold_input->setText(QString::fromUtf8("0"));
    contraction_threshold_input->setTextFormat(Qt::RichText);
    contraction_threshold_input->setAlignment(Qt::AlignCenter);
    contraction_threshold_input->setTextInteractionFlags(Qt::TextEditorInteraction);
    
    splitter->addWidget(_plot);
    dialLayout->addWidget(dial);
    dialLayout->addWidget(contraction_threshold_input);
    mainLayout->addLayout (dialLayout);
    mainLayout->addWidget (splitter);
    
    retranslateUi(Form);
    QObject::connect(dial, SIGNAL(valueChanged(int)), contraction_threshold_input, SLOT(setNum(int)));
    QObject::connect(dial, SIGNAL(valueChanged(int)), this, SLOT(add_contractions(int)));    
    
    QMetaObject::connectSlotsByName(Form);
    
 
    mainLayout->addWidget(_plot);

} // setupUi

void LPWidget::retranslateUi(QWidget *Form)
{
    Form->setWindowTitle(QApplication::translate("Form", "Parameters", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_ACCESSIBILITY
    dial->setAccessibleName(QApplication::translate("Form", "contraction_threshold", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_ACCESSIBILITY
} // retranslateUi


void LPWidget::demo ()
{
    QVector<QPointF> data0;
    QVector<QPointF> data1;
    for (int i = 0; i < 100; i++) {
        double x = double(0.01 * i);
        double y = exp(-(x-0.5)*(x-0.5)*16.0);
        data0.append(QPointF(x, y));
        data1.append(QPointF(x, sin(25.0*x)*y));
    }
    CurveData cd0(data0, "exp(-16(<i>x</i>-1/2)<sup>2</sup>)");
    cd0.setColor(Qt::red);
    cd0.setSymbol(Circle);
    CurveData cd1(data1, "sin(25<i>x</i>)exp(-16(<i>x</i>-1/2)<sup>2</sup>)");
    cd1.setColor(Qt::blue);
    cd1.setSymbol(Rhombus);
    cd1.setPenStyle(Qt::DashLine);
    _plot = new LightPlot2D;
    _plot->addCurveData(cd0);
    _plot->addCurveData(cd1);
    
    _plot->setMinY(-1.0);
    _plot->setMaxY(1.1);
    _plot->setIncrementY(0.25);
    
    _plot->setNameXAxis("<i><font color=#CD5C5C>Axis</font> </i><font color=#708090>X</font> :)");
    _plot->setLegendOpacity(220);
    
    _plot->setLegendFont(QFont("Serif", 12));
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(_plot);
}
