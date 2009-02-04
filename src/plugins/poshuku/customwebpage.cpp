#include "customwebpage.h"
#include <QtDebug>
#include <QFile>
#include <QBuffer>
#include <QWebFrame>
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
	qDebug () << Q_FUNC_INFO << reply->error ();
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
			{
				QFile errorPage (":/resources/html/generalerror.html");
				errorPage.open (QIODevice::ReadOnly);
				QString title = tr ("Error loading %1")
					.arg (reply->url ().toString ());
				QString contents = QString (errorPage.readAll ())
					.arg (title)
					.arg (reply->errorString ())
					.arg (reply->url ().toString ());

				QBuffer iconBuffer;
				iconBuffer.open (QIODevice::ReadWrite);
				QPixmap pixmap (":/resources/images/poshuku.png");
				pixmap.save (&iconBuffer, "PNG");
				contents.replace ("POSHUKU_LOGO", iconBuffer.buffer ().toBase64 ());

				QList<QWebFrame*> frames;
				frames.append (mainFrame ());
				while (!frames.isEmpty ())
				{
					QWebFrame *frame = frames.takeFirst ();
					if (frame->url () == reply->url ())
					{
						frame->setHtml (contents, reply->url ());
						break;
					}
					frames << frame->childFrames ();
				}
			}
			break;
	}
}

void CustomWebPage::handleDownloadRequested (const QNetworkRequest& request)
{
	emit gotEntity (request.url ().toString ().toUtf8 ());
}

