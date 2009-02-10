#ifndef BROWSERWIDGET_H
#define BROWSERWIDGET_H
#include <QWidget>
#include <interfaces/iwebbrowser.h>
#include "ui_browserwidget.h"

class BrowserWidget : public QWidget
					, public IWebWidget
{
	Q_OBJECT
	Q_INTERFACES (IWebWidget)
	
	Ui::BrowserWidget Ui_;

	QAction *Add2Favorites_;
	QAction *Find_;
	QAction *Print_;
	QAction *PrintPreview_;
	QAction *ScreenSave_;
	QAction *NewTab_;
	QAction *CloseTab_;
	QMenu *RecentlyClosed_;
public:
	BrowserWidget (QWidget* = 0);
	virtual ~BrowserWidget ();

	void SetUnclosers (const QList<QAction*>&);
	CustomWebView* GetView () const;
	void SetURL (const QUrl&);

	void Load (const QString&);
	void SetHtml (const QString&, const QString& = QString ());
	QWidget* Widget ();
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
	void handleNewTab ();
	void focusLineEdit ();
	void handleNewUnclose (QAction*);
	void handleUncloseDestroyed ();
	void enableActions ();
signals:
	void titleChanged (const QString&);
	void urlChanged (const QString&);
	void iconChanged (const QIcon&);
	void needToClose ();
	void addToFavorites (const QString&, const QString&);
	void statusBarChanged (const QString&);
	void gotEntity (const QByteArray&);
};

#endif

