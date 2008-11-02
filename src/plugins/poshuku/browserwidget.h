#ifndef BROWSERWIDGET_H
#define BROWSERWIDGET_H
#include <QWidget>
#include "ui_browserwidget.h"

class BrowserWidget : public QWidget
{
	Q_OBJECT
	
	Ui::BrowserWidget Ui_;
public:
	BrowserWidget (QWidget* = 0);
	virtual ~BrowserWidget ();

	CustomWebView* GetView () const;
	void SetURL (const QUrl&);
signals:
	void titleChanged (const QString&);
};

#endif

