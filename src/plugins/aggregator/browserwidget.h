#ifndef BROWSERWIDGET_H
#define BROWSERWIDGET_H
#include <QWidget>
#include "ui_browserwidget.h"

class QWebPage;

class BrowserWidget : public QWidget
{
	Q_OBJECT

	Ui::BrowserWidget Ui_;
public:
	BrowserWidget (QWidget* = 0);
	void setHtml (const QString&);
	QWebPage *page () const;
private slots:
	void loadStarted ();
	void loadFinished (bool);
	void loadProgress (int);
};

#endif

