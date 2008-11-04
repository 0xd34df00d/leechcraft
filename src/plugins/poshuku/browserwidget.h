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
protected:
	void keyReleaseEvent (QKeyEvent*);
private slots:
	void handleIconChanged ();
	void handleStatusBarMessage (const QString&);
	void on_URLEdit__returnPressed ();
signals:
	void titleChanged (const QString&);
	void urlChanged (const QString&);
	void iconChanged (const QIcon&);
	void needToClose ();
};

#endif

