#include "customwebpage.h"
#include <QtDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "core.h"

CustomWebPage::CustomWebPage (QObject *parent)
: QWebPage (parent)
{
	setForwardUnsupportedContent (true);
	connect (this,
			SIGNAL (downloadRequested (const QNetworkRequest&)),
			this,
			SLOT (handleDownloadRequested (const QNetworkRequest&)));
	connect (this,
			SIGNAL (unsupportedContent (QNetworkReply*)),
			this,
			SLOT (handleUnsupportedContent (QNetworkReply*)));
}

CustomWebPage::~CustomWebPage ()
{
}

void CustomWebPage::handleDownloadRequested (const QNetworkRequest& request)
{
	Core::Instance ().GotLink (request.url ().toString ());
}

void CustomWebPage::handleUnsupportedContent (QNetworkReply *reply)
{
	connect (reply,
			SIGNAL (finished ()),
			&Core::Instance (),
			SLOT (gotUnsupportedContent ()));
}

