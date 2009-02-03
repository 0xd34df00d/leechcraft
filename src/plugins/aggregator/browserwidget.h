#ifndef BROWSERWIDGET_H
#define BROWSERWIDGET_H
#include <QWidget>
#include "ui_browserwidget.h"

class QWebPage;
class QWebSettings;

class BrowserWidget : public QWidget
{
	Q_OBJECT

	Ui::BrowserWidget Ui_;
public:
	BrowserWidget (QWidget* = 0);
	virtual ~BrowserWidget ();
	void setHtml (const QString&);
	QWebPage *page () const;
	QWebSettings *settings () const;
private slots:
	void loadStarted ();
	void loadFinished (bool);
	void loadProgress (int);
};

#endif

