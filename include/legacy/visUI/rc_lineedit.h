#ifndef UI_rcLINE_EDIT_H
#define UI_rcLINE_EDIT_H


#include <QtGui/QtGui>
#include <QtCore/QtCore>



class rcLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	rcLineEdit( QWidget * parent , const char * name = 0 )
		: QLineEdit( parent , name )
	{
		connect( this , SIGNAL( returnPressed( void ) ) ,
				 this , SLOT( relayTextCommited( void ) ) );
	}

	rcLineEdit( const QString& contents , QWidget * parent , const char * name = 0 )
		: QLineEdit( contents , parent , name )
	{
		connect( this , SIGNAL( returnPressed( void ) ) ,
				 this , SLOT( relayTextCommited( void ) ) );
	}

protected:
	virtual void focusOutEvent( QFocusEvent* event )
	{
		QLineEdit::focusOutEvent( event );
		emit textCommited( text() );
	}

public slots:
	void relayTextCommited( void )
	{
		emit textCommited( text() );
	}

signals:
	void textCommited( const QString& text );
};

#endif  // UI_rcLINE_EDIT_H
