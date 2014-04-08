#ifndef UI_rcTEXT_EDIT_H
#define UI_rcTEXT_EDIT_H

#include <q3textedit.h>
//Added by qt3to4:
#include <QFocusEvent>

class rcTextEdit : public Q3TextEdit
{
	Q_OBJECT

public:
	rcTextEdit( QWidget * parent, int lines, const char * name )
		: Q3TextEdit( parent , name ), mLines(lines)
	{
		connect( this , SIGNAL( returnPressed( void ) ) ,
				 this , SLOT( relayTextCommited( void ) ) );
	}

	rcTextEdit( const QString& text, int lines, const QString& context = QString::null ,
				QWidget* parent = 0, const char* name = 0 )
		: Q3TextEdit( text , context , parent , name ), mLines(lines)
	{
		connect( this , SIGNAL( returnPressed( void ) ) ,
				 this , SLOT( relayTextCommited( void ) ) );
	}

    virtual QSize sizeHint() const
    {
        constPolish();
        int f = 2 * frameWidth();
        int h = fontMetrics().height();
        QSize sz( f, f );
        return sz.expandedTo( QSize(12 * h, mLines * h) );
    }
     
protected:
	virtual void focusOutEvent( QFocusEvent* event )
	{
		Q3TextEdit::focusOutEvent( event );
		emit textCommited( text() );
	}

public slots:
	void relayTextCommited( void )
	{
		emit textCommited( text() );
	}

signals:
	void textCommited( const QString& text );

private:
    int mLines; // Minimum widget height in text lines
};

#endif  // UI_rcTEXT_EDIT_H
