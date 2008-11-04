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
};

#endif

