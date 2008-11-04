#include "customwebview.h"
#include "core.h"

CustomWebView::CustomWebView (QWidget *parent)
: QWebView (parent)
{
}

CustomWebView::~CustomWebView ()
{
}

QWebView* CustomWebView::createWindow (QWebPage::WebWindowType type)
{
	QWebView *view = Core::Instance ().CreateWindow ();
	return view ? view : QWebView::createWindow (type);
}

