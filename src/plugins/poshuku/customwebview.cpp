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

#include "customwebview.h"
#include <cmath>
#include <qwebframe.h>
#include <QMouseEvent>
#include <QMenu>
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QFile>
#include <QWebElement>
#include <QTextCodec>
#include <QWindowsStyle>
#include <QtDebug>
#include <plugininterface/util.h>
#include <plugininterface/defaulthookproxy.h>
#include "interfaces/poshukutypes.h"
#include "core.h"
#include "customwebpage.h"
#include "browserwidget.h"
#include "searchtext.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			CustomWebView::CustomWebView (QWidget *parent)
			: QWebView (parent)
			, ScrollTimer_ (new QTimer (this))
			, ScrollDelta_ (0)
			, AccumulatedScrollShift_ (0)
			{
				Zooms_ << 0.3
					<< 0.5
					<< 0.67
					<< 0.8
					<< 0.9
					<< 1
					<< 1.1
					<< 1.2
					<< 1.33
					<< 1.5
					<< 1.7
					<< 2
					<< 2.4
					<< 3;

				Core::Instance ().GetPluginManager ()->RegisterHookable (this);

#if QT_VERSION >= 0x040600
				QPalette p;
				if (p.color (QPalette::Window) != Qt::white)
					setPalette (QWindowsStyle ().standardPalette ());
#endif

				connect (ScrollTimer_,
						SIGNAL (timeout ()),
						this,
						SLOT (handleAutoscroll ()));
				ScrollTimer_->start (30);

				CustomWebPage *page = new CustomWebPage (this);
				setPage (page);

				connect (this,
						SIGNAL (urlChanged (const QUrl&)),
						this,
						SLOT (remakeURL (const QUrl&)));
				connect (this,
						SIGNAL (loadFinished (bool)),
						this,
						SLOT (handleLoadFinished ()));

				connect (page,
						SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)),
						this,
						SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)));
				connect (page,
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));
				connect (page,
						SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
						this,
						SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));
				connect (page,
						SIGNAL (loadingURL (const QUrl&)),
						this,
						SLOT (remakeURL (const QUrl&)));
				connect (page,
						SIGNAL (printRequested (QWebFrame*)),
						this,
						SIGNAL (printRequested (QWebFrame*)));
				connect (page,
						SIGNAL (windowCloseRequested ()),
						this,
						SIGNAL (closeRequested ()));
				connect (page,
						SIGNAL (storeFormData (const PageFormsData_t&)),
						this,
						SIGNAL (storeFormData (const PageFormsData_t&)));

				QList<QByteArray> renderSettings;
				renderSettings << "PrimitivesAntialiasing"
					<< "TextAntialiasing"
					<< "SmoothPixmapTransform"
					<< "HighQualityAntialiasing";
				XmlSettingsManager::Instance ()->RegisterObject (renderSettings,
						this, "renderSettingsChanged");
				renderSettingsChanged ();
			}

			CustomWebView::~CustomWebView ()
			{
			}

			void CustomWebView::SetBrowserWidget (BrowserWidget *widget)
			{
				Browser_ = widget;
			}

			void CustomWebView::Load (const QString& string, QString title)
			{
				Load (Core::Instance ().MakeURL (string), title);
			}

			void CustomWebView::Load (const QUrl& url, QString title)
			{
				if (url.scheme () == "javascript")
				{
					QVariant result = page ()->mainFrame ()->
						evaluateJavaScript (url.toString ().mid (11));
					if (result.canConvert (QVariant::String))
						setHtml (result.toString ());
					return;
				}
				if (url.scheme () == "about")
				{
					if (url.path () == "plugins")
						NavigatePlugins ();
					else if (url.path () == "home")
						NavigateHome ();
					return;
				}
				if (title.isEmpty ())
					title = tr ("Loading...");
				remakeURL (url);
				emit titleChanged (title);
				load (url);
			}

			void CustomWebView::Load (const QNetworkRequest& req,
					QNetworkAccessManager::Operation op, const QByteArray& ba)
			{
				emit titleChanged (tr ("Loading..."));
				QWebView::load (req, op, ba);
			}


			QString CustomWebView::URLToProperString (const QUrl& url)
			{
				QString string = url.toString ();
				QWebElement equivs = page ()->mainFrame ()->
						findFirstElement ("meta[http-equiv=\"Content-Type\"]");
				if (!equivs.isNull ())
				{
					QString content = equivs.attribute ("content", "text/html; charset=UTF-8");
					const QString charset = "charset=";
					int pos = content.indexOf (charset);
					if (pos >= 0)
						PreviousEncoding_ = content.mid (pos + charset.length ()).toLower ();
				}

				if (PreviousEncoding_ != "utf-8" &&
						PreviousEncoding_ != "utf8" &&
						!PreviousEncoding_.isEmpty ())
					string = url.toEncoded ();

				return string;
			}

			void CustomWebView::mousePressEvent (QMouseEvent *e)
			{
				qobject_cast<CustomWebPage*> (page ())->SetButtons (e->buttons ());
				qobject_cast<CustomWebPage*> (page ())->SetModifiers (e->modifiers ());
				QWebView::mousePressEvent (e);
			}

			void CustomWebView::wheelEvent (QWheelEvent *e)
			{
				if (e->modifiers () & Qt::ControlModifier)
				{
					int degrees = e->delta () / 8;
					qreal delta = static_cast<qreal> (degrees) / 150;
					setZoomFactor (zoomFactor () + delta);
					e->accept ();
				}
				else
					QWebView::wheelEvent (e);
			}

			void CustomWebView::contextMenuEvent (QContextMenuEvent *e)
			{
				std::auto_ptr<QMenu> menu (new QMenu (this));
				QWebHitTestResult r = page ()->
					mainFrame ()->hitTestContent (e->pos ());

				IHookProxy_ptr proxy (new Util::DefaultHookProxy ());

				emit hookWebViewContextMenu (proxy, this, e, r,
						menu.get (), WVSStart);

				if (!r.linkUrl ().isEmpty ())
				{
					QUrl url = r.linkUrl ();
					QString text = r.linkText ();

					if (XmlSettingsManager::Instance ()->
							property ("TryToDetectRSSLinks").toBool ())
					{
						bool hasAtom = text.contains ("Atom");
						bool hasRSS = text.contains ("RSS");

						if (hasAtom || hasRSS)
						{
							LeechCraft::Entity e;
							if (hasAtom)
							{
								e.Additional_ ["UserVisibleName"] = "Atom";
								e.Mime_ = "application/atom+xml";
							}
							else
							{
								e.Additional_ ["UserVisibleName"] = "RSS";
								e.Mime_ = "application/rss+xml";
							}

							e.Entity_ = url;
							e.Parameters_ = LeechCraft::FromUserInitiated |
								LeechCraft::OnlyHandle;

							bool ch = false;
							emit couldHandle (e, &ch);
							if (ch)
							{
								QList<QVariant> datalist;
								datalist << url
									<< e.Mime_;
								menu->addAction (tr ("Subscribe"),
										this,
										SLOT (subscribeToLink ()))->setData (datalist);
								menu->addSeparator ();
							}
						}
					}

					menu->addAction (tr ("Open &here"),
							this, SLOT (openLinkHere ()))->setData (url);
					menu->addAction (tr ("Open in new &tab"),
							this, SLOT (openLinkInNewTab ()));
					menu->addSeparator ();
					menu->addAction (tr ("&Save link..."),
							this, SLOT (saveLink ()));

					QList<QVariant> datalist;
					datalist << url
						<< text;
					menu->addAction (tr ("&Bookmark link..."),
							this, SLOT (bookmarkLink ()))->setData (datalist);

					menu->addSeparator ();
					if (!page ()->selectedText ().isEmpty ())
						menu->addAction (pageAction (QWebPage::Copy));
					menu->addAction (tr ("&Copy link"),
							this, SLOT (copyLink ()));
					if (page ()->settings ()->testAttribute (QWebSettings::DeveloperExtrasEnabled))
						menu->addAction (pageAction (QWebPage::InspectElement));
				}

				emit hookWebViewContextMenu (proxy, this, e, r,
						menu.get (), WVSAfterLink);

				if (!r.imageUrl ().isEmpty ())
				{
					if (!menu->isEmpty ())
						menu->addSeparator ();
					menu->addAction (tr ("Open image here"),
							this, SLOT (openImageHere ()))->setData (r.imageUrl ());
					menu->addAction (tr ("Open image in new tab"),
							this, SLOT (openImageInNewTab ()));
					menu->addSeparator ();
					menu->addAction (tr ("Save image..."),
							this, SLOT (saveImage ()));
					menu->addAction (tr ("Copy image"),
							this, SLOT (copyImage ()));
					menu->addAction (tr ("Copy image location"),
							this, SLOT (copyImageLocation ()))->setData (r.imageUrl ());
				}

				emit hookWebViewContextMenu (proxy, this, e, r,
						menu.get (), WVSAfterImage);

				if (!page ()->selectedText ().isEmpty ())
				{
					if (!menu->isEmpty ())
						menu->addSeparator ();

					menu->addAction (pageAction (QWebPage::Copy));
					Browser_->Find_->setData (page ()->selectedText ());
					menu->addAction (Browser_->Find_);
					menu->addAction (tr ("Search..."),
							this, SLOT (searchSelectedText ()));
				}

				emit hookWebViewContextMenu (proxy, this, e, r,
						menu.get (), WVSAfterSelectedText);

				if (menu->isEmpty ())
					menu.reset (page ()->createStandardContextMenu ());

				if (!menu->isEmpty ())
					menu->addSeparator ();

				menu->addAction (Browser_->Add2Favorites_);
				menu->addSeparator ();
				menu->addAction (Browser_->Print_);
				menu->addAction (Browser_->PrintPreview_);
				menu->addSeparator ();
				menu->addAction (Browser_->ViewSources_);
				menu->addSeparator ();
				menu->addAction (pageAction (QWebPage::ReloadAndBypassCache));
				menu->addAction (Browser_->ReloadPeriodically_);
				menu->addAction (Browser_->NotifyWhenFinished_);
				menu->addSeparator ();
				menu->addAction (Browser_->ChangeEncoding_->menuAction ());
				menu->addSeparator ();
				menu->addAction (Browser_->RecentlyClosedAction_);

				emit hookWebViewContextMenu (proxy, this, e, r,
						menu.get (), WVSAfterFinish);

				if (!menu->isEmpty ())
				{
					menu->exec (mapToGlobal (e->pos ()));
					return;
				}

				QWebView::contextMenuEvent (e);
			}

			void CustomWebView::keyReleaseEvent (QKeyEvent *event)
			{
				bool handled = false;
				if (event->matches (QKeySequence::Copy))
				{
					const QString& text = selectedText ();
					if (!text.isEmpty ())
					{
						QApplication::clipboard ()->setText (text,
								QClipboard::Clipboard);
						handled = true;
					}
				}
				else if (event->key () == Qt::Key_F6)
					Browser_->focusLineEdit ();
				else if (event->modifiers () == Qt::SHIFT &&
						(event->key () == Qt::Key_PageUp || event->key () == Qt::Key_PageDown))
					ScrollDelta_ += event->key () == Qt::Key_PageUp ? -0.1 : 0.1;
				else if (event->modifiers () == Qt::SHIFT &&
						event->key () == Qt::Key_Plus)
					ScrollDelta_ = 0;

				if (!handled)
					QWebView::keyReleaseEvent (event);
			}

			int CustomWebView::LevelForZoom (qreal zoom)
			{
				int i = Zooms_.indexOf (zoom);

				if (i >= 0)
					return i;

				for (i = 0; i < Zooms_.size (); ++i)
					if (zoom <= Zooms_ [i])
						break;

				if (i == Zooms_.size ())
					return i - 1;

				if (i == 0)
					return i;

				if (zoom - Zooms_ [i - 1] > Zooms_ [i] - zoom)
					return i;
				else
					return i - 1;
			}

			void CustomWebView::NavigatePlugins ()
			{
				QFile pef (":/resources/html/pluginsenum.html");
				pef.open (QIODevice::ReadOnly);
				QString contents = QString (pef.readAll ())
					.replace ("INSTALLEDPLUGINS", tr ("Installed plugins"))
					.replace ("NOPLUGINS", tr ("No plugins installed"))
					.replace ("FILENAME", tr ("File name"))
					.replace ("MIME", tr ("MIME type"))
					.replace ("DESCR", tr ("Description"))
					.replace ("SUFFIXES", tr ("Suffixes"))
					.replace ("ENABLED", tr ("Enabled"))
					.replace ("NO", tr ("No"))
					.replace ("YES", tr ("Yes"));
				setHtml (contents);
			}

			void CustomWebView::NavigateHome ()
			{
				QFile file (":/resources/html/home.html");
				file.open (QIODevice::ReadOnly);
				QString data = file.readAll ();
				data.replace ("{pagetitle}",
						tr ("Welcome to LeechCraft!"));
				data.replace ("{title}",
						tr ("Welcome to LeechCraft!"));
				data.replace ("{body}",
						tr ("Welcome to LeechCraft, the integrated internet-client.<br />"
							"More info is available on the <a href='http://leechcraft.org'>"
							"project's site</a>."));

				QBuffer iconBuffer;
				iconBuffer.open (QIODevice::ReadWrite);
				QPixmap pixmap (":/resources/images/poshuku.svg");
				pixmap.save (&iconBuffer, "PNG");

				data.replace ("{img}",
						QByteArray ("data:image/png;base64,") + iconBuffer.buffer ().toBase64 ());

				setHtml (data);
			}

			void CustomWebView::zoomIn ()
			{
				int i = LevelForZoom (zoomFactor ());

				if (i < Zooms_.size () - 1)
					setZoomFactor (Zooms_ [i + 1]);

				emit invalidateSettings ();
			}

			void CustomWebView::zoomOut ()
			{
				int i = LevelForZoom (zoomFactor ());

				if (i > 0)
					setZoomFactor (Zooms_ [i - 1]);

				emit invalidateSettings ();
			}

			void CustomWebView::zoomReset ()
			{
				setZoomFactor (1);

				emit invalidateSettings ();
			}

			void CustomWebView::remakeURL (const QUrl& url)
			{
				emit urlChanged (URLToProperString (url));
			}

			void CustomWebView::handleLoadFinished ()
			{
				remakeURL (url ());
			}

			void CustomWebView::openLinkHere ()
			{
				Load (qobject_cast<QAction*> (sender ())->data ().toUrl ());
			}

			void CustomWebView::openLinkInNewTab ()
			{
				pageAction (QWebPage::OpenLinkInNewWindow)->trigger ();
			}

			void CustomWebView::saveLink ()
			{
				pageAction (QWebPage::DownloadLinkToDisk)->trigger ();
			}

			void CustomWebView::subscribeToLink ()
			{
				QList<QVariant> list = qobject_cast<QAction*> (sender ())->data ().toList ();
				Entity e = Util::MakeEntity (list.at (0),
						QString (),
						FromUserInitiated | OnlyHandle,
						list.at (1).toString ());
				emit gotEntity (e);
			}

			void CustomWebView::bookmarkLink ()
			{
				QList<QVariant> list = qobject_cast<QAction*> (sender ())->data ().toList ();
				emit addToFavorites (list.at (1).toString (),
						list.at (0).toUrl ().toString ());
			}

			void CustomWebView::copyLink ()
			{
				pageAction (QWebPage::CopyLinkToClipboard)->trigger ();
			}

			void CustomWebView::openImageHere ()
			{
				Load (qobject_cast<QAction*> (sender ())->data ().toUrl ());
			}

			void CustomWebView::openImageInNewTab ()
			{
				pageAction (QWebPage::OpenImageInNewWindow)->trigger ();
			}

			void CustomWebView::saveImage ()
			{
				pageAction (QWebPage::DownloadImageToDisk)->trigger ();
			}

			void CustomWebView::copyImage ()
			{
				pageAction (QWebPage::CopyImageToClipboard)->trigger ();
			}

			void CustomWebView::copyImageLocation ()
			{
				QString url = qobject_cast<QAction*> (sender ())->data ().toUrl ().toString ();
				QClipboard *cb = QApplication::clipboard ();
				cb->setText (url, QClipboard::Clipboard);
				cb->setText (url, QClipboard::Selection);
			}

			void CustomWebView::searchSelectedText ()
			{
				QString text = page ()->selectedText ();
				if (text.isEmpty ())
					return;

				SearchText *st = new SearchText (text, this);
				connect (st,
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));
				st->setAttribute (Qt::WA_DeleteOnClose);
				st->show ();
			}

			void CustomWebView::renderSettingsChanged ()
			{
				QPainter::RenderHints hints;
				if (XmlSettingsManager::Instance ()->
						property ("PrimitivesAntialiasing").toBool ())
					hints |= QPainter::Antialiasing;
				if (XmlSettingsManager::Instance ()->
						property ("TextAntialiasing").toBool ())
					hints |= QPainter::TextAntialiasing;
				if (XmlSettingsManager::Instance ()->
						property ("SmoothPixmapTransform").toBool ())
					hints |= QPainter::SmoothPixmapTransform;
				if (XmlSettingsManager::Instance ()->
						property ("HighQualityAntialiasing").toBool ())
					hints |= QPainter::HighQualityAntialiasing;

				setRenderHints (hints);
			}

			void CustomWebView::handleAutoscroll ()
			{
				if (!ScrollDelta_)
					return;

				AccumulatedScrollShift_ += ScrollDelta_;

				if (std::abs (AccumulatedScrollShift_) >= 1)
				{
					QWebFrame *mf = page ()->mainFrame ();
					QPoint pos = mf->scrollPosition ();
					pos += QPoint (0, AccumulatedScrollShift_);
					mf->setScrollPosition (pos);

					AccumulatedScrollShift_ -= static_cast<int> (AccumulatedScrollShift_);
				}
			}
		};
	};
};

