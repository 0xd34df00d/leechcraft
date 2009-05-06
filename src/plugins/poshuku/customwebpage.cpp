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
			SIGNAL (contentsChanged ()),
			this,
			SLOT (handleContentsChanged ()));
	connect (this,
			SIGNAL (databaseQuotaExceeded (QWebFrame*, QString)),
			this,
			SLOT (handleDatabaseQuotaExceeded (QWebFrame*, QString)));
	connect (this,
			SIGNAL (downloadRequested (const QNetworkRequest&)),
			this,
			SLOT (handleDownloadRequested (const QNetworkRequest&)));
	connect (this,
			SIGNAL (frameCreated (QWebFrame*)),
			this,
			SLOT (handleFrameCreated (QWebFrame*)));
	connect (this,
			SIGNAL (geometryChangeRequested (const QRect&)),
			this,
			SLOT (handleGeometryChangeRequested (const QRect&)));
	connect (this,
			SIGNAL (linkClicked (const QUrl&)),
			this,
			SLOT (handleLinkClicked (const QUrl&)));
	connect (this,
			SIGNAL (linkHovered (const QString&,
					const QString&, const QString&)),
			this,
			SLOT (handleLinkHovered (const QString&,
					const QString&, const QString&)));
	connect (this,
			SIGNAL (loadFinished (bool)),
			this,
			SLOT (handleLoadFinished (bool)));
	connect (this,
			SIGNAL (loadProgress (int)),
			this,
			SLOT (handleLoadProgress (int)));
	connect (this,
			SIGNAL (loadStarted ()),
			this,
			SLOT (handleLoadStarted ()));
	connect (this,
			SIGNAL (menuBarVisibilityChangeRequested (bool)),
			this,
			SLOT (handleMenuBarVisibilityChangeRequested (bool)));
	connect (this,
			SIGNAL (microFocusChanged ()),
			this,
			SLOT (handleMicroFocusChanged ()));
	connect (this,
			SIGNAL (printRequested (QWebFrame*)),
			this,
			SLOT (handlePrintRequested (QWebFrame*)));
	connect (this,
			SIGNAL (repaintRequested (const QRect&)),
			this,
			SLOT (handleRepaintRequested (const QRect&)));
	connect (this,
			SIGNAL (restoreFrameStateRequested (QWebFrame*)),
			this,
			SLOT (handleRestoreFrameStateRequested (QWebFrame*)));
	connect (this,
			SIGNAL (saveFrameStateRequested (QWebFrame*, QWebHistoryItem*)),
			this,
			SLOT (handleSaveFrameStateRequested (QWebFrame*, QWebHistoryItem*)));
	connect (this,
			SIGNAL (scrollRequested (int, int, const QRect&)),
			this,
			SLOT (handleScrollRequested (int, int, const QRect&)));
	connect (this,
			SIGNAL (selectionChanged ()),
			this,
			SLOT (handleSelectionChanged ()));
	connect (this,
			SIGNAL (statusBarMessage (const QString&)),
			this,
			SLOT (handleStatusBarMessage (const QString&)));
	connect (this,
			SIGNAL (statusBarVisibilityChangeRequested (bool)),
			this,
			SLOT (handleStatusBarVisibilityChangeRequested (bool)));
	connect (this,
			SIGNAL (toolBarVisibilityChangeRequested (bool)),
			this,
			SLOT (handleToolBarVisiblityChangeRequested (bool)));
	connect (this,
			SIGNAL (unsupportedContent (QNetworkReply*)),
			this,
			SLOT (handleUnsupportedContent (QNetworkReply*)));
	connect (this,
			SIGNAL (windowCloseRequested ()),
			this,
			SLOT (handleWindowCloseRequested ()));
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

void CustomWebPage::handleContentsChanged ()
{
	if (Core::Instance ().GetPluginManager ()->
			HandleContentsChanged (this))
		return;

	emit filteredContentsChanged ();
}

void CustomWebPage::handleDatabaseQuotaExceeded (QWebFrame *frame, QString string)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleDatabaseQuotaExceeded (this, frame, string))
		return;

	emit filteredDatabaseQuotaExceeded (frame, string);
}

void CustomWebPage::handleDownloadRequested (const QNetworkRequest& request)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleDownloadRequested (this, request))
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

	emit filteredDownloadRequested (request);
}

void CustomWebPage::handleFrameCreated (QWebFrame *frame)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleFrameCreated (this, frame))
		return;

	emit filteredFrameCreated (frame);
}

void CustomWebPage::handleGeometryChangeRequested (const QRect& rect)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleGeometryChangeRequested (this, rect))
		return;
	
	emit filteredGeometryChangeRequested (rect);
}

void CustomWebPage::handleLinkClicked (const QUrl& url)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleLinkClicked (this, url))
		return;

	emit filteredLinkClicked (url);
}

void CustomWebPage::handleLinkHovered (const QString& link,
		const QString& title, const QString& context)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleLinkHovered (this, link, title, context))
		return;

	emit filteredLinkHovered (link, title, context);
}

void CustomWebPage::handleLoadFinished (bool ok)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleLoadFinished (this, ok))
		return;

	emit filteredLoadFinished (ok);
}

void CustomWebPage::handleLoadProgress (int progress)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleLoadProgress (this, progress))
		return;

	emit filteredLoadProgress (progress);
}

void CustomWebPage::handleLoadStarted ()
{
	if (Core::Instance ().GetPluginManager ()->
			HandleLoadStarted (this))
		return;

	emit filteredLoadStarted ();
}

void CustomWebPage::handleMenuBarVisibilityChangeRequested (bool vis)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleMenuBarVisibilityChangeRequested (this, vis))
		return;

	emit filteredMenuBarVisibilityChangeRequested (vis);
}

void CustomWebPage::handleMicroFocusChanged ()
{
	if (Core::Instance ().GetPluginManager ()->
			HandleMicroFocusChanged (this))
		return;

	emit filteredMicroFocusChanged ();
}

void CustomWebPage::handlePrintRequested (QWebFrame *frame)
{
	if (Core::Instance ().GetPluginManager ()->
			HandlePrintRequested (this, frame))
		return;

	emit filteredPrintRequested (frame);
}

void CustomWebPage::handleRepaintRequested (const QRect& rect)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleRepaintRequested (this, rect))
		return;

	emit filteredRepaintRequested (rect);
}

void CustomWebPage::handleRestoreFrameStateRequested (QWebFrame *frame)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleRestoreFrameStateRequested (this, frame))
		return;

	emit filteredRestoreFrameStateRequested (frame);
}

void CustomWebPage::handleSaveFrameStateRequested (QWebFrame *frame,
		QWebHistoryItem *item)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleSaveFrameStateRequested (this, frame, item))
		return;

	emit filteredSaveFrameStateRequested (frame, item);
}

void CustomWebPage::handleScrollRequested (int dx, int dy, const QRect& rect)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleScrollRequested (this, dx, dy, rect))
		return;

	emit filteredScrollRequested (dx, dy, rect);
}

void CustomWebPage::handleSelectionChanged ()
{
	if (Core::Instance ().GetPluginManager ()->
			HandleSelectionChanged (this))
		return;

	emit filteredSelectionChanged ();
}

void CustomWebPage::handleStatusBarMessage (const QString& msg)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleStatusBarMessage (this, msg))
		return;

	emit filteredStatusBarMessage (msg);
}

void CustomWebPage::handleStatusBarVisibilityChangeRequested (bool vis)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleStatusBarVisibilityChangeRequested (this, vis))
		return;

	emit filteredStatusBarVisibilityChangeRequested (vis);
}

void CustomWebPage::handleToolBarVisiblityChangeRequested (bool vis)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleToolBarVisibilityChangeRequested (this, vis))
		return;

	emit filteredToolBarVisiblityChangeRequested (vis);
}

void CustomWebPage::handleUnsupportedContent (QNetworkReply *reply)
{
	if (Core::Instance ().GetPluginManager ()->
			HandleUnsupportedContent (this, reply))
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
					else
						qDebug () << Q_FUNC_INFO
							<< reply->header (QNetworkRequest::ContentTypeHeader);
				}
				else
					qDebug () << Q_FUNC_INFO
						<< "but frame is found";
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

	emit filteredUnsupportedContent (reply);
}

void CustomWebPage::handleWindowCloseRequested ()
{
	if (Core::Instance ().GetPluginManager ()->
			HandleWindowCloseRequested (this))
		return;

	emit filteredWindowCloseRequested ();
}

bool CustomWebPage::acceptNavigationRequest (QWebFrame *frame,
		const QNetworkRequest& request, QWebPage::NavigationType type)
{
	if (Core::Instance ().GetPluginManager ()->
			OnAcceptNavigationRequest (this, frame, request, type))
		return false;

	QString scheme = request.url ().scheme ();
	if (scheme == "mailto" || scheme == "ftp")
	{
		QDesktopServices::openUrl (request.url ());
		return false;
	}

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
		return QWebPage::userAgentForUrl (url) + "/Poshuku";
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

