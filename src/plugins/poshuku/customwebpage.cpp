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
#include "pluginmanager.h"

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
	if (Core::Instance ().GetPluginManager ()->
			OnGotUnsupportedContent (this, reply))
		return;

	switch (reply->error ())
	{
		case QNetworkReply::ProtocolUnknownError:
			if (XmlSettingsManager::Instance ()->
					property ("ExternalSchemes").toString ().split (' ')
					.contains (reply->url ().scheme ()))
				QDesktopServices::openUrl (reply->url ());
			else
			{
				reply->abort ();
				LeechCraft::DownloadEntity e =
				{
					reply->url ().toString ().toUtf8 (),
					QString (),
					QString (),
					LeechCraft::FromUserInitiated,
					QVariant ()
				};
				emit gotEntity (e);
			}
			break;
		case QNetworkReply::NoError:
			{
				QWebFrame *found = FindFrame (reply->url ());
				if (!found)
				{
					if (XmlSettingsManager::Instance ()->
							property ("ParanoidDownloadsDetection").toBool () ||
							reply->header (QNetworkRequest::ContentTypeHeader).isValid ())
					{
						LeechCraft::DownloadEntity e =
						{
							reply->url ().toString ().toUtf8 (),
							QString (),
							QString (),
							LeechCraft::FromUserInitiated,
							QVariant::fromValue<QNetworkReply*> (reply)
						};
						emit gotEntity (e);
						break;
					}
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
	if (Core::Instance ().GetPluginManager ()->
			OnHandleDownloadRequested (this, request))
		return;

	LeechCraft::DownloadEntity e =
	{
		request.url ().toString ().toUtf8 (),
		QString (),
		QString (),
		LeechCraft::FromUserInitiated,
		QVariant ()
	};
	emit gotEntity (e);
}

bool CustomWebPage::acceptNavigationRequest (QWebFrame *frame,
		const QNetworkRequest& request, QWebPage::NavigationType type)
{
	if (Core::Instance ().GetPluginManager ()->
			OnAcceptNavigationRequest (this, frame, request, type))
		return false;

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

QString CustomWebPage::chooseFile (QWebFrame *frame, const QString& suggested)
{
	try
	{
		return Core::Instance ().GetPluginManager ()->
			OnChooseFile (this, frame, suggested);
	}
	catch (...)
	{
		return QWebPage::chooseFile (frame, suggested);
	}
}

QObject* CustomWebPage::createPlugin (const QString& clsid, const QUrl& url,
		const QStringList& names, const QStringList& values)
{
	try
	{
		return Core::Instance ().GetPluginManager ()->
			OnCreatePlugin (this, clsid, url, names, values);
	}
	catch (...)
	{
		return QWebPage::createPlugin (clsid, url, names, values);
	}
}

QWebPage* CustomWebPage::createWindow (QWebPage::WebWindowType type)
{
	try
	{
		return Core::Instance ().GetPluginManager ()->
			OnCreateWindow (this, type);
	}
	catch (...)
	{
		return QWebPage::createWindow (type);
	}
}

void CustomWebPage::javaScriptAlert (QWebFrame *frame, const QString& msg)
{
	if (Core::Instance ().GetPluginManager ()->
			OnJavaScriptAlert (this, frame, msg))
		return;
	else
		QWebPage::javaScriptAlert (frame, msg);
}

bool CustomWebPage::javaScriptConfirm (QWebFrame *frame, const QString& msg)
{
	try
	{
		return Core::Instance ().GetPluginManager ()->
			OnJavaScriptConfirm (this, frame, msg);
	}
	catch (...)
	{
		return QWebPage::javaScriptConfirm (frame, msg);
	}
}

void CustomWebPage::javaScriptConsoleMessage (const QString& msg, int line,
		const QString& sid)
{
	if (Core::Instance ().GetPluginManager ()->
			OnJavaScriptConsoleMessage (this, msg, line, sid))
		return;
	else
		QWebPage::javaScriptConsoleMessage (msg, line, sid);
}

bool CustomWebPage::javaScriptPrompt (QWebFrame *frame, const QString& pr,
		const QString& def, QString *result)
{
	try
	{
		return Core::Instance ().GetPluginManager ()->
			OnJavaScriptPrompt (this, frame, pr, def, result);
	}
	catch (...)
	{
		return QWebPage::javaScriptPrompt (frame, pr, def, result);
	}
}

QString CustomWebPage::userAgentForUrl (const QUrl& url) const
{
	try
	{
		return Core::Instance ().GetPluginManager ()->
			OnUserAgentForUrl (this, url);
	}
	catch (...)
	{
		return QString ("LeechCraft::Poshuku/") + QWebPage::userAgentForUrl (url);
	}
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

