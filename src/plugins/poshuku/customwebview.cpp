#include "customwebview.h"
#include <QtDebug>
#include "core.h"
#include "customwebpage.h"

CustomWebView::CustomWebView (QWidget *parent)
: QWebView (parent)
{
	connect (this,
			SIGNAL (urlChanged (const QUrl&)),
			this,
			SLOT (remakeURL (const QUrl&)));
	CustomWebPage *page = new CustomWebPage (this);
	connect (page,
			SIGNAL (gotEntity (const QByteArray&)),
			this,
			SIGNAL (gotEntity (const QByteArray&)));
	setPage (page);
}

CustomWebView::~CustomWebView ()
{
}

QWebView* CustomWebView::createWindow (QWebPage::WebWindowType)
{
	return Core::Instance ().MakeWebView ();
}

void CustomWebView::remakeURL (const QUrl& url)
{
	emit urlChanged (url.toString ());
}

