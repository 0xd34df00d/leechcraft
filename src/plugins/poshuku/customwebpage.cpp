/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "customwebpage.h"
#include <QtDebug>
#include <QFile>
#include <QBuffer>
#include <QWebFrame>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QSysInfo>
#include <QWebHistory>
#include <plugininterface/util.h>
#include "xmlsettingsmanager.h"
#include "customwebview.h"
#include "core.h"
#include "pluginmanager.h"
#include "jsproxy.h"
#include "externalproxy.h"
#include "webpluginfactory.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			CustomWebPage::CustomWebPage (QObject *parent)
			: QWebPage (parent)
			, MouseButtons_ (Qt::NoButton)
			, Modifiers_ (Qt::NoModifier)
			, JSProxy_ (new JSProxy (this))
			, ExternalProxy_ (new ExternalProxy (this))
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleBeginWebPageConstruction (this))
					return;

				setForwardUnsupportedContent (true);
				setNetworkAccessManager (Core::Instance ().GetNetworkAccessManager ());

				setPluginFactory (Core::Instance ().GetWebPluginFactory ());

				connect (this,
						SIGNAL (delayedFillForms (QWebFrame*)),
						this,
						SLOT (fillForms (QWebFrame*)),
						Qt::QueuedConnection);

				connect (ExternalProxy_.get (),
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));

				connect (mainFrame (),
						SIGNAL (javaScriptWindowObjectCleared ()),
						this,
						SLOT (handleJavaScriptWindowObjectCleared ()));
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

				if (Core::Instance ().GetPluginManager ()->
						HandleEndWebPageConstruction (this))
					return;
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
			}
			
			void CustomWebPage::handleDatabaseQuotaExceeded (QWebFrame *frame, QString string)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleDatabaseQuotaExceeded (this, frame, string))
					return;
			}
			
			void CustomWebPage::handleDownloadRequested (const QNetworkRequest& request)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleDownloadRequested (this, request))
					return;
			
				LeechCraft::DownloadEntity e = LeechCraft::Util::MakeEntity (request.url (),
						QString (),
						LeechCraft::FromUserInitiated);
				emit gotEntity (e);
			}
			
			void CustomWebPage::handleFrameCreated (QWebFrame *frame)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleFrameCreated (this, frame))
					return;
			}
			
			void CustomWebPage::handleJavaScriptWindowObjectCleared ()
			{
				QWebFrame *frame = qobject_cast<QWebFrame*> (sender ());
				if (Core::Instance ().GetPluginManager ()->
						HandleJavaScriptWindowObjectCleared (this, frame))
					return;
			
				frame->addToJavaScriptWindowObject ("JSProxy", JSProxy_.get ());
				frame->addToJavaScriptWindowObject ("external", ExternalProxy_.get ());
			}
			
			void CustomWebPage::handleGeometryChangeRequested (const QRect& rect)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleGeometryChangeRequested (this, rect))
					return;
			}
			
			void CustomWebPage::handleLinkClicked (const QUrl& url)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleLinkClicked (this, url))
					return;
			}
			
			void CustomWebPage::handleLinkHovered (const QString& link,
					const QString& title, const QString& context)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleLinkHovered (this, link, title, context))
					return;
			}
			
			void CustomWebPage::handleLoadFinished (bool ok)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleLoadFinished (this, ok))
					return;
			
				emit delayedFillForms (mainFrame ());
			}
			
			void CustomWebPage::handleLoadProgress (int progress)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleLoadProgress (this, progress))
					return;
			}
			
			void CustomWebPage::handleLoadStarted ()
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleLoadStarted (this))
					return;
			}
			
			void CustomWebPage::handleMenuBarVisibilityChangeRequested (bool vis)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleMenuBarVisibilityChangeRequested (this, vis))
					return;
			}
			
			void CustomWebPage::handleMicroFocusChanged ()
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleMicroFocusChanged (this))
					return;
			}
			
			void CustomWebPage::handlePrintRequested (QWebFrame *frame)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandlePrintRequested (this, frame))
					return;
			}
			
			void CustomWebPage::handleRepaintRequested (const QRect& rect)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleRepaintRequested (this, rect))
					return;
			}
			
			void CustomWebPage::handleRestoreFrameStateRequested (QWebFrame *frame)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleRestoreFrameStateRequested (this, frame))
					return;
			}
			
			void CustomWebPage::handleSaveFrameStateRequested (QWebFrame *frame,
					QWebHistoryItem *item)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleSaveFrameStateRequested (this, frame, item))
					return;
			}
			
			void CustomWebPage::handleScrollRequested (int dx, int dy, const QRect& rect)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleScrollRequested (this, dx, dy, rect))
					return;
			}
			
			void CustomWebPage::handleSelectionChanged ()
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleSelectionChanged (this))
					return;
			}
			
			void CustomWebPage::handleStatusBarMessage (const QString& msg)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleStatusBarMessage (this, msg))
					return;
			}
			
			void CustomWebPage::handleStatusBarVisibilityChangeRequested (bool vis)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleStatusBarVisibilityChangeRequested (this, vis))
					return;
			}
			
			void CustomWebPage::handleToolBarVisiblityChangeRequested (bool vis)
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleToolBarVisibilityChangeRequested (this, vis))
					return;
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
								LeechCraft::Util::MakeEntity (reply->url (),
									QString (),
									LeechCraft::FromUserInitiated);
							emit gotEntity (e);
							if (XmlSettingsManager::Instance ()->
									property ("CloseEmptyDelegatedPages").toBool () &&
									history ()->currentItem ().url ().isEmpty ())
								emit windowCloseRequested ();
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
										LeechCraft::Util::MakeEntity (
												QVariant::fromValue<QNetworkReply*> (reply),
												QString (),
												LeechCraft::FromUserInitiated
												);

									e.Additional_ ["SourceURL"] = reply->url ();
									e.Mime_ = reply->
										header (QNetworkRequest::ContentTypeHeader).toString ();
			
									emit gotEntity (e);
									if (XmlSettingsManager::Instance ()->
											property ("CloseEmptyDelegatedPages").toBool () &&
											history ()->currentItem ().url ().isEmpty ())
										emit windowCloseRequested ();
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
			}
			
			void CustomWebPage::handleWindowCloseRequested ()
			{
				if (Core::Instance ().GetPluginManager ()->
						HandleWindowCloseRequested (this))
					return;
			}
			
			bool CustomWebPage::acceptNavigationRequest (QWebFrame *frame,
					const QNetworkRequest& request, QWebPage::NavigationType type)
			{
				if (Core::Instance ().GetPluginManager ()->
						OnAcceptNavigationRequest (this, frame, request, type))
					return false;
			
				QString scheme = request.url ().scheme ();
				if (scheme == "mailto" ||
						scheme == "ftp")
				{
					LeechCraft::DownloadEntity e =
						LeechCraft::Util::MakeEntity (request.url (),
							QString (),
							LeechCraft::FromUserInitiated);
					bool ch = false;
					emit couldHandle (e, &ch);
					if (ch)
						emit gotEntity (e);
					else
						QDesktopServices::openUrl (request.url ());
					return false;
				}
			
				if (frame)
					HandleForms (frame, request, type);
			
				if ((type == NavigationTypeLinkClicked ||
							type == NavigationTypeOther) &&
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
				QString ua = Core::Instance ().GetUserAgent (url, this);
				if (ua.isEmpty ())
					return QWebPage::userAgentForUrl (url);
				else
					return ua;
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
			
			namespace
			{
				bool CheckData (const PageFormsData_t& data,
						QWebFrame *frame,
						const QNetworkRequest& request = QNetworkRequest ())
				{
					if (data.isEmpty ())
					{
						qWarning () << Q_FUNC_INFO
							<< "no form data for"
							<< frame
							<< request.url ();
						return false;
					}
					if (data.size () > 1)
					{
						qWarning () << Q_FUNC_INFO
							<< "too much form data for"
							<< frame
							<< data.size ()
							<< request.url ();
					}
					return true;
				}
			};
			
			void CustomWebPage::HandleForms (QWebFrame *frame,
					const QNetworkRequest& request, QWebPage::NavigationType type)
			{
				JSProxy_->ClearForms ();
			
				QWebFrame *formFrame = frame ? frame : mainFrame ();
				QFile file (":/resources/scripts/formquery.js");
				if (file.open (QIODevice::ReadOnly))
					formFrame->evaluateJavaScript (file.readAll ());
				else
					qWarning () << Q_FUNC_INFO
						<< "could not open internal file"
						<< file.fileName ()
						<< file.errorString ();
				if (type == NavigationTypeFormSubmitted)
				{
					PageFormsData_t data = JSProxy_->GetForms ();
#ifdef QT_DEBUG
					qDebug () << frame << request.url () << data;
#endif
					if (!CheckData (data, frame, request))
						return;
			
					QString url = frame->url ().toString ();
			
					// Check if this should be emitted at all
					if (Core::Instance ().GetStorageBackend ()->GetFormsIgnored (url))
						return;
			
					emit storeFormData (data);
				}
			}
			
			void CustomWebPage::fillForms (QWebFrame *frame)
			{
				JSProxy_->ClearForms ();
			
				QString url = frame->url ().toString ();
			
				PageFormsData_t data;
				Core::Instance ().GetStorageBackend ()->GetFormsData (url, data [url]);
			
				JSProxy_->SetForms (data);
				QFile sfile (":/resources/scripts/formsetter.js");
				if (sfile.open (QIODevice::ReadOnly))
					frame->evaluateJavaScript (sfile.readAll ());
				else
					qWarning () << Q_FUNC_INFO
						<< "could not open internal file"
						<< sfile.fileName ()
						<< sfile.errorString ();
			
				Q_FOREACH (QWebFrame *childFrame, frame->childFrames ())
					fillForms (childFrame);
			}
		};
	};
};

