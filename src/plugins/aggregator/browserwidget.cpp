#include "browserwidget.h"

BrowserWidget::BrowserWidget (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);
	connect (Ui_.ItemView_,
			SIGNAL (loadStarted ()),
			this,
			SLOT (loadStarted ()));
	connect (Ui_.ItemView_,
			SIGNAL (loadFinished (bool)),
			this,
			SLOT (loadFinished (bool)));
	connect (Ui_.ItemView_,
			SIGNAL (loadProgress (int)),
			this,
			SLOT (loadProgress (int)));
	Ui_.BrowserBackButton_->setDefaultAction (Ui_.ActionBrowserBack_);
	Ui_.BrowserForwardButton_->setDefaultAction (Ui_.ActionBrowserForward_);
	Ui_.BrowserReloadButton_->setDefaultAction (Ui_.ActionBrowserReload_);
	Ui_.BrowserStopButton_->setDefaultAction (Ui_.ActionBrowserStop_);
	connect (Ui_.ActionBrowserBack_, SIGNAL (triggered ()), Ui_.ItemView_, SLOT (back ()));
	connect (Ui_.ActionBrowserForward_, SIGNAL (triggered ()), Ui_.ItemView_, SLOT (forward ()));
	connect (Ui_.ActionBrowserStop_, SIGNAL (triggered ()), Ui_.ItemView_, SLOT (stop ()));
	connect (Ui_.ActionBrowserReload_, SIGNAL (triggered ()), Ui_.ItemView_, SLOT (reload ()));
}

BrowserWidget::~BrowserWidget ()
{
}

void BrowserWidget::setHtml (const QString& html)
{
	Ui_.ItemView_->setHtml (html);
}

QWebPage* BrowserWidget::page () const
{
	return Ui_.ItemView_->page ();
}

void BrowserWidget::loadStarted ()
{
	Ui_.PageLoadProgressBar_->setFormat (tr ("Loading..."));
	Ui_.PageLoadProgressBar_->setValue (100);
}

void BrowserWidget::loadFinished (bool ok)
{
	QString title = Ui_.ItemView_->title ();
	if (title.isEmpty ())
		title = tr ("Loaded");
	QString text = ok ? title : title + tr (" (with errors)");
	Ui_.PageLoadProgressBar_->setFormat (text);
	Ui_.PageLoadProgressBar_->setValue (100);
}

void BrowserWidget::loadProgress (int progress)
{
	Ui_.PageLoadProgressBar_->setFormat (tr ("Loading (\%p)"));
	Ui_.PageLoadProgressBar_->setValue (progress);
}

