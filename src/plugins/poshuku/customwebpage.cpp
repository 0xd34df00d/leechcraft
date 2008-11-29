#include "customwebpage.h"
#include <QtDebug>
#include <QNetworkRequest>

CustomWebPage::CustomWebPage (QObject *parent)
: QWebPage (parent)
{
	connect (this,
			SIGNAL (downloadRequested (const QNetworkRequest&)),
			this,
			SLOT (handleDownloadRequested (const QNetworkRequest&)));
}

CustomWebPage::~CustomWebPage ()
{
}

void CustomWebPage::handleDownloadRequested (const QNetworkRequest& request)
{
}

