#include "selectablebrowser.h"
#include <QVBoxLayout>

using namespace LeechCraft::Plugins::Aggregator;

SelectableBrowser::SelectableBrowser (QWidget *parent)
: QWidget (parent)
, Internal_ (true)
{
	QVBoxLayout *lay = new QVBoxLayout ();
	lay->setContentsMargins (0, 0, 0, 0);
	setLayout (lay);
}

void SelectableBrowser::Construct (IWebBrowser *browser)
{
	if (browser)
	{
		Internal_ = false;
		InternalBrowser_.reset ();
		ExternalBrowser_.reset (browser->GetWidget ());
		layout ()->addWidget (ExternalBrowser_->Widget ());
	}
	else
	{
		Internal_ = true;
		InternalBrowser_.reset (new QTextEdit (this));
		ExternalBrowser_.reset ();
		layout ()->addWidget (InternalBrowser_.get ());
	}
}

void SelectableBrowser::SetHtml (const QString& html, const QUrl& base)
{
	if (Internal_)
		InternalBrowser_->setHtml (html);
	else
		ExternalBrowser_->SetHtml (html, base);
}


