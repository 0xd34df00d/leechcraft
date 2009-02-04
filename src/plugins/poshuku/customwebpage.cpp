#include "customwebpage.h"
#include <QtDebug>
#include <QFile>
#include <QBuffer>
#include <QWebFrame>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include "xmlsettingsmanager.h"
#include "customwebview.h"
#include "core.h"

CustomWebPage::CustomWebPage (QObject *parent)
: QWebPage (parent)
, MouseButtons_ (Qt::NoButton)
, Modifiers_ (Qt::NoModifier)
{
	setForwardUnsupportedContent (true);
	setNetworkAccessManager (Core::Instance ().GetNetworkAccessManager ());
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

void CustomWebPage::SetButtons (Qt::MouseButtons buttons)
{
	MouseButtons_ = buttons;
}

void CustomWebPage::SetModifiers (Qt::KeyboardModifiers modifiers)
{
	Modifiers_ = modifiers;
}

void CustomWebPage::gotUnsupportedContent (QNetworkReply *reply)
{
	qDebug () << Q_FUNC_INFO << reply->error ();
	switch (reply->error ())
	{
		case QNetworkReply::ProtocolUnknownError:
			if (XmlSettingsManager::Instance ()->
					property ("ExternalSchemes").toString ().split (' ')
					.contains (reply->url ().scheme ()))
				QDesktopServices::openUrl (reply->url ());
			else
				emit gotEntity (reply->url ().toString ().toUtf8 ());
			break;
		case QNetworkReply::NoError:
			{
				QWebFrame *found = FindFrame (reply->url ());
				if (!found &&
						reply->header (QNetworkRequest::ContentTypeHeader).isValid ())
				{
					emit gotEntity (reply->url ().toString ().toUtf8 ());
					break;
				}
			}
		default:
			{
				QWebFrame *found = FindFrame (reply->url ());

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

				if (found)
					found->setHtml (contents, reply->url ());
				else if (LoadingURL_ == reply->url ())
					mainFrame ()->setHtml (contents, reply->url ());
			}
			break;
	}
}

void CustomWebPage::handleDownloadRequested (const QNetworkRequest& request)
{
	emit gotEntity (request.url ().toString ().toUtf8 ());
}

bool CustomWebPage::acceptNavigationRequest (QWebFrame *frame,
		const QNetworkRequest& request, QWebPage::NavigationType type)
{
	if ((type == QWebPage::NavigationTypeLinkClicked ||
				type == QWebPage::NavigationTypeOther) &&
			(MouseButtons_ == Qt::MidButton ||
			 Modifiers_ & Qt::ControlModifier))
	{
		bool invert = Modifiers_ & Qt::ShiftModifier;

		CustomWebView *view = Core::Instance ().MakeWebView (invert);
		view->Load (request);

		MouseButtons_ = Qt::NoButton;
		Modifiers_ = Qt::NoModifier;
		return false;
	}

	if (frame == mainFrame ())
	{
		LoadingURL_ = request.url ();
		emit loadingURL (LoadingURL_);
	}

	QString scheme = request.url ().scheme ();
	if (scheme == "mailto" || scheme == "ftp")
	{
		QDesktopServices::openUrl (request.url ());
		return false;
	}

	return QWebPage::acceptNavigationRequest (frame, request, type);
}

QString CustomWebPage::userAgentForUrl (const QUrl& url) const
{
	return QString ("LeechCraft::Poshuku/") + QWebPage::userAgentForUrl (url);
}

QWebFrame* CustomWebPage::FindFrame (const QUrl& url)
{
	QList<QWebFrame*> frames;
	frames.append (mainFrame ());
	while (!frames.isEmpty ())
	{
		QWebFrame *frame = frames.takeFirst ();
		if (frame->url () == url)
			return frame;
		frames << frame->childFrames ();
	}
	return 0;
}

