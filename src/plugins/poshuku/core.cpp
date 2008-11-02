#include "core.h"
#include <QString>
#include <QUrl>
#include <QWidget>
#include "browserwidget.h"
#include "customwebview.h"

Core::Core ()
{
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
}

bool Core::IsValidURL (const QString& url) const
{
	return QUrl (url).isValid ();
}

BrowserWidget* Core::NewURL (const QString& url)
{
	BrowserWidget *widget = new BrowserWidget;
	widget->SetURL (QUrl (url));

	connect (widget,
			SIGNAL (titleChanged (const QString&)),
			this,
			SLOT (handleTitleChanged (const QString&)));

	emit addNewTab (tr ("Loading..."), widget);
	return widget;
}

CustomWebView* Core::MakeWebView ()
{
	return NewURL ("")->GetView ();
}

void Core::handleTitleChanged (const QString& newTitle)
{
	emit changeTabName (dynamic_cast<QWidget*> (sender ()), newTitle);
}

