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
private:
	void PrintImpl (bool);
private slots:
	void handleIconChanged ();
	void handleStatusBarMessage (const QString&);
	void on_URLEdit__returnPressed ();
	void handleAdd2Favorites ();
	void handleFind ();
	void findText (const QString&, QWebPage::FindFlags);
	void handlePrinting ();
	void handlePrintingWithPreview ();
	void handleScreenSave ();
signals:
	void titleChanged (const QString&);
	void urlChanged (const QString&);
	void iconChanged (const QIcon&);
	void needToClose ();
	void addToFavorites (const QString&, const QString&);
	void statusBarChanged (const QString&);
};

#endif

