#ifndef CUSTOMWEBVIEW_H
#define CUSTOMWEBVIEW_H
#include <QWebView>

class CustomWebView : public QWebView
{
	Q_OBJECT
public:
	CustomWebView (QWidget* = 0);
	virtual ~CustomWebView ();
protected:
	virtual QWebView* createWindow (QWebPage::WebWindowType);
private slots:
	void remakeURL (const QUrl&);
signals:
	void urlChanged (const QString&);
	void gotEntity (const QByteArray&);
};

#endif

