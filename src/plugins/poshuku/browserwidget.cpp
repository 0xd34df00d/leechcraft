#include "browserwidget.h"

BrowserWidget::BrowserWidget (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);
	connect (Ui_.WebView_,
			SIGNAL (titleChanged (const QString&)),
			this,
			SIGNAL (titleChanged (const QString&)));
	connect (Ui_.WebView_,
			SIGNAL (urlChanged (const QString&)),
			this,
			SIGNAL (urlChanged (const QString&)));
	connect (Ui_.WebView_,
			SIGNAL (urlChanged (const QString&)),
			Ui_.URLEdit_,
			SLOT (setText (const QString&)));
	connect (Ui_.WebView_,
			SIGNAL (loadProgress (int)),
			Ui_.LoadProgress_,
			SLOT (setValue (int)));
}

BrowserWidget::~BrowserWidget ()
{
}

CustomWebView* BrowserWidget::GetView () const
{
	return Ui_.WebView_;
}

void BrowserWidget::SetURL (const QUrl& url)
{
	Ui_.WebView_->load (url);
}

