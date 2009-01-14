#include "customwebpage.h"
#include <QtDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include "xmlsettingsmanager.h"
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
			SLOT (gotUnsupportedContent (QNetworkReply*)));
}

CustomWebPage::~CustomWebPage ()
{
}

void CustomWebPage::gotUnsupportedContent (QNetworkReply *reply)
{
	switch (reply->error ())
	{
		case QNetworkReply::NoError:
			emit gotEntity (reply->url ().toString ().toUtf8 ());
			break;
		case QNetworkReply::ProtocolUnknownError:
			if (XmlSettingsManager::Instance ()->
					property ("ExternalSchemes").toString ().split (' ')
					.contains (reply->url ().scheme ()))
				QDesktopServices::openUrl (reply->url ());
			else
				emit gotEntity (reply->url ().toString ().toUtf8 ());
			break;
		default:
			qDebug () << Q_FUNC_INFO << reply->url ().toString () << reply->error () << reply->errorString ();
			break;
	}
}

void CustomWebPage::handleDownloadRequested (const QNetworkRequest& request)
{
	Core::Instance ().GotLink (request.url ().toString ());
}

