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

#include "core.h"
#include <algorithm>
#include <memory>
#include <typeinfo>
#include <QString>
#include <QUrl>
#include <QWidget>
#include <QTextCodec>
#include <QIcon>
#include <QFile>
#include <QFileInfo>
#include <QNetworkCookieJar>
#include <QDir>
#include <QMenu>
#include <QWebFrame>
#include <QInputDialog>
#include <QNetworkReply>
#include <QSslSocket>
#include <QAuthenticator>
#include <QNetworkProxy>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QWebHistory>
#include <QtDebug>
#include <plugininterface/util.h>
#include <interfaces/ihaveshortcuts.h>
#include "browserwidget.h"
#include "customwebview.h"
#include "addtofavoritesdialog.h"
#include "xmlsettingsmanager.h"
#include "restoresessiondialog.h"
#include "sqlstoragebackend.h"
#include "xbelparser.h"
#include "xbelgenerator.h"
#include "linkhistory.h"
#include "config.h"
#include "favoriteschecker.h"
#include "webpluginfactory.h"
#include "interfaces/pluginbase.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			using LeechCraft::Util::TagsCompletionModel;
			
			Core::Core ()
			: NetworkAccessManager_ (0)
			, WebPluginFactory_ (0)
			, IsShuttingDown_ (false)
			, ShortcutProxy_ (0)
			, FavoritesChecker_ (0)
			, Initialized_ (false)
			{
				qRegisterMetaType<BrowserWidgetSettings> ("LeechCraft::Plugins::Poshuku::BrowserWidgetSettings");
				qRegisterMetaTypeStreamOperators<BrowserWidgetSettings> ("LeechCraft::Plugins::Poshuku::BrowserWidgetSettings");
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_Poshuku");
				int size = settings.beginReadArray ("Saved session");
				if (size)
					for (int i = 0; i < size; ++i)
					{
						settings.setArrayIndex (i);
						QString title = settings.value ("Title").toString ();
						QString url = settings.value ("URL").toString ();
						SavedSessionState_ << QPair<QString, QString> (title, url);
						SavedSessionSettings_ << settings.value ("Settings").value<BrowserWidgetSettings> ();
					}
				settings.endArray ();

				PluginManager_.reset (new PluginManager (this));
				URLCompletionModel_.reset (new URLCompletionModel (this));
			
				PluginsMenu_ = new QMenu (tr ("Plugins"));
			
				QWebHistoryInterface::setDefaultInterface (new LinkHistory);
			}
			
			Core& Core::Instance ()
			{
				static Core core;
				return core;
			}
			
			bool Core::Init ()
			{
				QDir dir = QDir::home ();
				if (!dir.cd (".leechcraft/poshuku") &&
						!dir.mkpath (".leechcraft/poshuku"))
				{
					qCritical () << Q_FUNC_INFO
						<< "could not create neccessary directories for Poshuku";
					return false;
				}
			
				StorageBackend::Type type;
				QString strType = XmlSettingsManager::Instance ()->
					property ("StorageType").toString ();
				if (strType == "SQLite")
					type = StorageBackend::SBSQLite;
				else if (strType == "PostgreSQL")
					type = StorageBackend::SBPostgres;
				else
					throw std::runtime_error (qPrintable (QString ("Unknown storage type %1")
								.arg (strType)));
			
				try
				{
					StorageBackend_ = StorageBackend::Create (type);
				}
				catch (const std::runtime_error& s)
				{
					emit error (QTextCodec::codecForName ("UTF-8")->
							toUnicode (s.what ()));
					return false;
				}
				catch (...)
				{
					emit error (tr ("Poshuku: general storage initialization error."));
					return false;
				}
				StorageBackend_->Prepare ();
			
				HistoryModel_.reset (new HistoryModel (this));
				connect (StorageBackend_.get (),
						SIGNAL (added (const HistoryItem&)),
						HistoryModel_.get (),
						SLOT (handleItemAdded (const HistoryItem&)));
			
				connect (StorageBackend_.get (),
						SIGNAL (added (const HistoryItem&)),
						URLCompletionModel_.get (),
						SLOT (handleItemAdded (const HistoryItem&)));
			
				FavoritesModel_.reset (new FavoritesModel (this));
				connect (StorageBackend_.get (),
						SIGNAL (added (const FavoritesModel::FavoritesItem&)),
						FavoritesModel_.get (),
						SLOT (handleItemAdded (const FavoritesModel::FavoritesItem&)));
				connect (StorageBackend_.get (),
						SIGNAL (updated (const FavoritesModel::FavoritesItem&)),
						FavoritesModel_.get (),
						SLOT (handleItemUpdated (const FavoritesModel::FavoritesItem&)));
				connect (StorageBackend_.get (),
						SIGNAL (removed (const FavoritesModel::FavoritesItem&)),
						FavoritesModel_.get (),
						SLOT (handleItemRemoved (const FavoritesModel::FavoritesItem&)));

				FavoritesChecker_ = new FavoritesChecker (this);
			
				QTimer::singleShot (200, this, SLOT (postConstruct ()));
				Initialized_ = true;
				return true;
			}
			
			void Core::Release ()
			{
				saveSession ();
				IsShuttingDown_ = true;
				while (Widgets_.begin () != Widgets_.end ())
					delete *Widgets_.begin ();
			
				HistoryModel_.reset ();
				FavoritesModel_.reset ();
				StorageBackend_.reset ();
			
				XmlSettingsManager::Instance ()->setProperty ("CleanShutdown", true);
				XmlSettingsManager::Instance ()->Release ();

				delete WebPluginFactory_;
			}
			
			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
				NetworkAccessManager_ = proxy->GetNetworkAccessManager ();
				ShortcutProxy_ = proxy->GetShortcutProxy ();
			}
			
			ICoreProxy_ptr Core::GetProxy () const
			{
				return Proxy_;
			}

			WebPluginFactory* Core::GetWebPluginFactory ()
			{
				if (!WebPluginFactory_)
					WebPluginFactory_ = new WebPluginFactory (this);
				return WebPluginFactory_;
			}
			
			void Core::SetProvider (QObject *object, const QString& feature)
			{
				Providers_ [feature] = object;
			}
			
			QByteArray Core::GetExpectedPluginClass () const
			{
				return QByteArray (typeid (PluginBase).name ());
			}
			
			void Core::AddPlugin (QObject *plugin)
			{
				PluginManager_->AddPlugin (plugin);
			}
			
			QUrl Core::MakeURL (QString url) const
			{
				if (url.isEmpty ())
					return QUrl ();

				url = url.trimmed ();
				if (url == "localhost")
					return QUrl ("http://localhost");

				// If the url without percent signs and two following characters is
				// a valid url (it should not be percent-encoded), then treat source
				// url as percent-encoded, otherwise treat as not percent-encoded.
				QString withoutPercent = url;
				withoutPercent.remove (QRegExp ("%%??",
							Qt::CaseInsensitive, QRegExp::Wildcard));
				QUrl testUrl (withoutPercent);
				QUrl result;
				if (testUrl.toString () == withoutPercent)
					result = QUrl::fromPercentEncoding (url.toUtf8 ());
				else
					result = QUrl (url);
			
				if (result.scheme ().isEmpty ())
				{
					if (!url.count (' ') && url.count ('.'))
						result = QUrl (QString ("http://") + url);
					else
					{
						url.replace (' ', '+');
						result = QUrl (QString ("http://www.google.com/search?q=%1"
									"&client=leechcraft_poshuku"
									"&rls=org.leechcraft:%2")
								.arg (url)
								.arg (QLocale::system ().name ().replace ('_', '-')));
					}
				}
			
				return result;
			}
			
			BrowserWidget* Core::NewURL (const QUrl& url, bool raise)
			{
				if (!Initialized_)
					return 0;

				BrowserWidget *widget = new BrowserWidget ();
				widget->InitShortcuts ();
				widget->SetUnclosers (Unclosers_);
				Widgets_.push_back (widget);
			
				emit addNewTab (tr (""), widget);

				ConnectSignals (widget);
			
				if (!url.isEmpty ())
					widget->SetURL (url);
			
				if (raise)
					emit raiseTab (widget);
			
				return widget;
			}

			BrowserWidget* Core::NewURL (const QString& str, bool raise)
			{
				return NewURL (MakeURL (str), raise);
			}
			
			IWebWidget* Core::GetWidget ()
			{
				if (!Initialized_)
					return 0;

				BrowserWidget *widget = new BrowserWidget ();
				widget->Deown ();
				widget->InitShortcuts ();
				SetupConnections (widget);
				return widget;
			}
			
			CustomWebView* Core::MakeWebView (bool invert)
			{
				if (!Initialized_)
					return 0;

				bool raise = true;
				if (XmlSettingsManager::Instance ()->property ("BackgroundNewTabs").toBool ())
					raise = false;
			
				if (invert)
					raise = !raise;
			
				return NewURL (QUrl (), raise)->GetView ();
			}

			void Core::ConnectSignals (BrowserWidget *widget)
			{
				SetupConnections (widget);
				connect (widget,
						SIGNAL (titleChanged (const QString&)),
						this,
						SLOT (handleTitleChanged (const QString&)));
				connect (widget,
						SIGNAL (iconChanged (const QIcon&)),
						this,
						SLOT (handleIconChanged (const QIcon&)));
				connect (widget,
						SIGNAL (needToClose ()),
						this,
						SLOT (handleNeedToClose ()));
				connect (widget,
						SIGNAL (statusBarChanged (const QString&)),
						this,
						SLOT (handleStatusBarChanged (const QString&)));
				connect (widget,
						SIGNAL (tooltipChanged (QWidget*)),
						this,
						SLOT (handleTooltipChanged (QWidget*)));
				connect (widget,
						SIGNAL (invalidateSettings ()),
						this,
						SLOT (saveSingleSession ()));
			}

			void Core::CheckFavorites ()
			{
				FavoritesChecker_->Check ();
			}
			
			FavoritesModel* Core::GetFavoritesModel () const
			{
				return FavoritesModel_.get ();
			}
			
			HistoryModel* Core::GetHistoryModel () const
			{
				return HistoryModel_.get ();
			}
			
			URLCompletionModel* Core::GetURLCompletionModel () const
			{
				return URLCompletionModel_.get ();
			}
			
			QNetworkAccessManager* Core::GetNetworkAccessManager () const
			{
				return NetworkAccessManager_;
			}
			
			StorageBackend* Core::GetStorageBackend () const
			{
				return StorageBackend_.get ();
			}
			
			PluginManager* Core::GetPluginManager () const
			{
				return PluginManager_.get ();
			}
			
			void Core::SetShortcut (int name, const QKeySequence& shortcut)
			{
				Q_FOREACH (BrowserWidget *widget, Widgets_)
					widget->SetShortcut (name, shortcut);
			}
			
			const IShortcutProxy* Core::GetShortcutProxy () const
			{
				return ShortcutProxy_;
			}

			QIcon Core::GetIcon (const QUrl& url) const
			{
				QIcon result = QWebSettings::iconForUrl (url);
				if (!result.isNull ())
					return result;

				QUrl test;
				test.setScheme (url.scheme ());
				test.setHost (url.host ());

				result = QWebSettings::iconForUrl (test);
				if (!result.isNull ())
					return result;

				return QWebSettings::webGraphic (QWebSettings::DefaultFrameIconGraphic);
			}

			QString Core::GetUserAgent (const QUrl& url, const QWebPage *page) const
			{
				try
				{
					return GetPluginManager ()->OnUserAgentForUrl (page, url);
				}
				catch (...)
				{
					if (url.host ().contains (".google."))
						return QString ();
					else
					{
#if defined (Q_OS_WINCE) || defined (Q_OS_WIN32) || defined (Q_OS_MSDOS)
						QString winver = "unknown Windows";
						switch (QSysInfo::windowsVersion ())
						{
							case QSysInfo::WV_32s:
								winver = "Windows 3.1 with Win32s";
								break;
							case QSysInfo::WV_95:
								winver = "Windows 95";
								break;
							case QSysInfo::WV_98:
								winver = "Windows 98";
								break;
							case QSysInfo::WV_Me:
								winver = "Windows ME";
								break;
							case QSysInfo::WV_NT:
								winver = "Windows NT";
								break;
							case QSysInfo::WV_2000:
								winver = "Windows 2000";
								break;
							case QSysInfo::WV_XP:
								winver = "Windows XP";
								break;
							case QSysInfo::WV_2003:
								winver = "Windows 2003";
								break;
							case QSysInfo::WV_VISTA:
								winver = "Windows Vista";
								break;
							case QSysInfo::WV_WINDOWS7:
								winver = "Windows 7";
								break;
							case QSysInfo::WV_CE:
								winver = "Windows CE";
								break;
							case QSysInfo::WV_CENET:
								winver = "Windows CE .NET";
								break;
							case QSysInfo::WV_CE_5:
								winver = "Windows CE 5.x";
								break;
							case QSysInfo::WV_CE_6:
								winver = "Windows CE 6.x";
								break;
							case QSysInfo::WV_DOS_based:
								winver = "unknown DOS-based";
								break;
							case QSysInfo::WV_NT_based:
								winver = "unkown NT-based";
								break;
							case QSysInfo::WV_CE_based:
								winver = "unknown CE-based";
								break;
						}
#elif defined (Q_OS_DARWIN)
						QString macver = "unknown Mac OS";
						switch (QSysInfo::MacintoshVersion)
						{
							case QSysInfo::MV_CHEETAH:
								macver = "Cheetah";
								break;
							case QSysInfo::MV_PUMA:
								macver = "Puma";
								break;
							case QSysInfo::MV_JAGUAR:
								macver = "Jaguar";
								break;
							case QSysInfo::MV_PANTHER:
								macver = "Panther";
								break;
							case QSysInfo::MV_TIGER:
								macver = "Tiger";
								break;
							case QSysInfo::MV_LEOPARD:
								macver = "Leopard";
								break;
							case QSysInfo::MV_SNOWLEOPARD:
								macver = "Snow Leopard";
								break;
						}
#endif
						return QString ("LeechCraft (%1; %2; %3; %4) (LeechCraft/Poshuku %5; WebKit %6/%7)")
							// %1 platform
#ifdef Q_WS_MAC
							.arg ("Macintosh")
#elif defined (Q_WS_WIN)
							.arg ("Windows")
#elif defined (Q_WS_X11)
							.arg ("X11")
#elif defined (Q_WS_QWS)
							.arg ("QWS")
#else
							.arg ("compatible")
#endif
							// %2 security
							.arg (QSslSocket::supportsSsl () ? "U" : "N")
							// %3 subplatform
#ifdef Q_OS_AIX
							.arg ("AIX")
#elif defined (Q_OS_BSD4)
							.arg ("BSD 4.4")
#elif defined (Q_OS_BSDI)
							.arg ("BSD/OS")
#elif defined (Q_OS_CYGWIN)
							.arg ("Cygwin")
#elif defined (Q_OS_DARWIN)
							.arg (macver)
#elif defined (Q_OS_DGUX)
							.arg ("DG/UX")
#elif defined (Q_OS_DYNIX)
							.arg ("DYNIX/ptx")
#elif defined (Q_OS_FREEBSD)
							.arg ("FreeBSD")
#elif defined (Q_OS_HPUX)
							.arg ("HP-UX")
#elif defined (Q_OS_HURD)
							.arg ("GNU Hurd")
#elif defined (Q_OS_IRIX)
							.arg ("IRIX")
#elif defined (Q_OS_LINUX)
							.arg ("Linux")
#elif defined (Q_OS_LYNX)
							.arg ("LynxOS")
#elif defined (Q_OS_NETBSD)
							.arg ("NetBSD")
#elif defined (Q_OS_OPENBSD)
							.arg ("OpenBSD")
#elif defined (Q_OS_OS2)
							.arg ("OS/2")
#elif defined (Q_OS_OS2EMX)
							.arg ("OS/2 XFree86")
#elif defined (Q_OS_OSF)
							.arg ("HP Tru64 UNIX")
#elif defined (Q_OS_QNX6)
							.arg ("QNX RTP 6.1")
#elif defined (Q_OS_QNX)
							.arg ("QNX")
#elif defined (Q_OS_RELIANT)
							.arg ("Reliant UNIX")
#elif defined (Q_OS_SCO)
							.arg ("SCO OpenServer 5")
#elif defined (Q_OS_SOLARIS)
							.arg ("Sun Solaris")
#elif defined (Q_OS_ULTRIX)
							.arg ("DEC Ultrix")
#elif defined (Q_OS_UNIXWARE)
							.arg ("UnixWare 7 or Open UNIX 8")
#elif defined (Q_OS_WINCE) || defined (Q_OS_WIN32) || defined (Q_OS_MSDOS)
							.arg (winver)
#elif defined (Q_OS_UNIX)
							.arg ("any UNIX BSD/SYSV")
#else
#warning "Unknown OS"
							.arg ("unkown subplatform")
#endif
							// %4 locale
							.arg (QLocale::system ().name ())
							.arg (LEECHCRAFT_VERSION)
							.arg (QT_VERSION_STR)
							.arg (qVersion ());
					}
				}
			}
			
			void Core::Unregister (BrowserWidget *widget)
			{
				widgets_t::iterator pos =
					std::find (Widgets_.begin (), Widgets_.end (), widget);
				if (pos == Widgets_.end ())
				{
					qWarning () << Q_FUNC_INFO << widget << "not found in the collection";
					return;
				}
			
				QString title = widget->GetView ()->title ();
				if (title.isEmpty ())
					title = widget->GetView ()->url ().toString ();
			
				if (!title.isEmpty ())
				{
					if (title.size () > 53)
						title = title.left (50) + "...";
					QAction *action = new QAction (widget->GetView ()->icon (),
							title, this);
					UncloseData ud =
					{
						widget->GetView ()->url (),
						widget->GetView ()->page ()->mainFrame ()->scrollPosition ()
					};
					action->setData (QVariant::fromValue (ud));
			
					connect (action,
							SIGNAL (triggered ()),
							this,
							SLOT (handleUnclose ()));
			
					emit newUnclose (action);
			
					Unclosers_.push_front (action);
				}
			
				Widgets_.erase (pos);
			
				saveSession ();
			}
			
			QMenu* Core::GetPluginsMenu () const
			{
				return PluginsMenu_;
			}
			
			void Core::RestoreSession (bool ask)
			{
				if (!SavedSessionState_.size ()) ;
				else if (ask)
				{
					std::auto_ptr<RestoreSessionDialog> dia (new RestoreSessionDialog ());
					bool added = false;
					for (int i = 0; i < SavedSessionState_.size (); ++i)
					{
						QPair<QString, QString> pair = SavedSessionState_.at (i);
						QString title = pair.first;
						QString url = pair.second;
						if (url.isEmpty ())
							continue;
						dia->AddPair (title, url);
						added = true;
					}
			
					if (added &&
							dia->exec () == QDialog::Accepted)
					{
						RestoredURLs_ = dia->GetSelectedURLs ();
						QTimer::singleShot (2000, this, SLOT (restorePages ()));
					}
					else
						saveSession ();
				}
				else
				{
					for (int i = 0; i < SavedSessionState_.size (); ++i)
					{
						QString url = SavedSessionState_.at (i).second;
						if (url.isEmpty ())
							continue;
						RestoredURLs_ << i;
					}
					QTimer::singleShot (2000, this, SLOT (restorePages ()));
				}
			}
			
			void Core::HandleHistory (QWebView *view)
			{
				QString url = view->url ().toString ();
			
				if (!view->title ().isEmpty () &&
						!url.isEmpty () && url != "about:blank")
					HistoryModel_->AddItem (view->title (),
							url, QDateTime::currentDateTime ());
			}

			void Core::SetupConnections (BrowserWidget *widget)
			{
				connect (widget,
						SIGNAL (addToFavorites (const QString&, const QString&)),
						this,
						SLOT (handleAddToFavorites (const QString&, const QString&)));
				connect (widget,
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
				connect (widget,
						SIGNAL (couldHandle (const LeechCraft::DownloadEntity&, bool*)),
						this,
						SIGNAL (couldHandle (const LeechCraft::DownloadEntity&, bool*)));
				connect (widget,
						SIGNAL (downloadFinished (const QString&)),
						this,
						SIGNAL (downloadFinished (const QString&)));
				connect (widget,
						SIGNAL (urlChanged (const QString&)),
						this,
						SLOT (handleURLChanged (const QString&)));
			}
			
			void Core::importXbel ()
			{
				QString suggestion = XmlSettingsManager::Instance ()->
						Property ("LastXBELOpen", QDir::homePath ()).toString ();
				QString filename = QFileDialog::getOpenFileName (0,
						tr ("Select XBEL file"),
						suggestion,
						tr ("XBEL files (*.xbel);;"
							"All files (*.*)"));
			
				if (filename.isEmpty ())
					return;
			
				XmlSettingsManager::Instance ()->setProperty ("LastXBELOpen",
						QFileInfo (filename).absolutePath ());
			
				QFile file (filename);
				if (!file.open (QIODevice::ReadOnly))
				{
					QMessageBox::critical (0,
							tr ("LeechCraft"),
							tr ("Could not open file %1 for reading.")
								.arg (filename));
					return;
				}
			
				QByteArray data = file.readAll ();
			
				try
				{
					XbelParser p (data);
				}
				catch (const std::exception& e)
				{
					QMessageBox::critical (0,
							tr ("LeechCraft"),
							e.what ());
				}
			}
			
			void Core::exportXbel ()
			{
				QString suggestion = XmlSettingsManager::Instance ()->
						Property ("LastXBELSave", QDir::homePath ()).toString ();
				QString filename = QFileDialog::getSaveFileName (0,
						tr ("Save XBEL file"),
						suggestion,
						tr ("XBEL files (*.xbel);;"
							"All files (*.*)"));
			
				if (filename.isEmpty ())
					return;
			
				if (!filename.endsWith (".xbel"))
					filename.append (".xbel");
			
				XmlSettingsManager::Instance ()->setProperty ("LastXBELSave",
						QFileInfo (filename).absolutePath ());
			
				QFile file (filename);
				if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
				{
					QMessageBox::critical (0,
							tr ("LeechCraft"),
							tr ("Could not open file %1 for writing.")
								.arg (filename));
					return;
				}
			
				QByteArray data;
				XbelGenerator g (data);
				file.write (data);
			}
			
			void Core::handleUnclose ()
			{
				QAction *action = qobject_cast<QAction*> (sender ());
				UncloseData ud = action->data ().value<UncloseData> ();
				BrowserWidget *bw = NewURL (ud.URL_);
				bw->SetOnLoadScrollPoint (ud.SPoint_);
				Unclosers_.removeAll (action);
				action->deleteLater ();
			}
			
			void Core::handleTitleChanged (const QString& newTitle)
			{
				emit changeTabName (dynamic_cast<QWidget*> (sender ()), newTitle);
			
				saveSingleSession ();
			}
			
			void Core::handleURLChanged (const QString&)
			{
				HandleHistory (dynamic_cast<BrowserWidget*> (sender ())->GetView ());
			
				saveSingleSession ();
			}
			
			void Core::handleIconChanged (const QIcon& newIcon)
			{
				emit changeTabIcon (dynamic_cast<QWidget*> (sender ()), newIcon);
			}
			
			void Core::handleNeedToClose ()
			{
				BrowserWidget *w = dynamic_cast<BrowserWidget*> (sender ());
				emit removeTab (w);
			
				w->deleteLater ();
			
				saveSession ();
			}
			
			void Core::handleAddToFavorites (const QString& title, const QString& url)
			{
				std::auto_ptr<AddToFavoritesDialog> dia (new AddToFavoritesDialog (title,
							url,
							qApp->activeWindow ()));
			
				bool result = false;
				do
				{
					if (dia->exec () == QDialog::Rejected)
						return;
			
					result = FavoritesModel_->AddItem (dia->GetTitle (),
							url, dia->GetTags ());
				}
				while (!result);
			}
			
			void Core::handleStatusBarChanged (const QString& msg)
			{
				emit statusBarChanged (static_cast<QWidget*> (sender ()), msg);
			}
			
			void Core::handleTooltipChanged (QWidget *tip)
			{
				emit changeTooltip (static_cast<QWidget*> (sender ()), tip);
			}
			
			void Core::favoriteTagsUpdated (const QStringList& tags)
			{
				XmlSettingsManager::Instance ()->setProperty ("FavoriteTags", tags);
			}
			
			void Core::saveSession ()
			{
				if (IsShuttingDown_ || !Initialized_)
					return;
			
				int pos = 0;
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_Poshuku");
				settings.beginWriteArray ("Saved session");
				settings.remove ("");
				for (widgets_t::const_iterator i = Widgets_.begin (),
						end = Widgets_.end (); i != end; ++i)
				{
					settings.setArrayIndex (pos++);
					settings.setValue ("Title", (*i)->GetView ()->title ());
					settings.setValue ("URL", (*i)->GetView ()->url ().toString ());
					settings.setValue ("Settings",
							QVariant::fromValue<BrowserWidgetSettings> ((*i)->GetWidgetSettings ()));
				}
				settings.endArray ();
			}

			void Core::saveSingleSession ()
			{
				BrowserWidget *source = qobject_cast<BrowserWidget*> (sender ());
				if (!source)
				{
					qWarning () << Q_FUNC_INFO
						<< "sender is not a BrowserWidget*"
						<< sender ();
					return;
				}

				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_Poshuku");
				settings.beginWriteArray ("Saved session");
				for (int i = 0, size = Widgets_.size (); i < size; ++i)
					if (Widgets_.at (i) == source)
					{
						settings.setArrayIndex (i);
						settings.setValue ("Title", source->GetView ()->title ());
						settings.setValue ("URL", source->GetView ()->url ().toString ());
						settings.setValue ("Settings",
								QVariant::fromValue<BrowserWidgetSettings> (source->GetWidgetSettings ()));
						break;
					}
				settings.endArray ();
			}
			
			void Core::restorePages ()
			{
				for (QList<int>::const_iterator i = RestoredURLs_.begin (),
						end = RestoredURLs_.end (); i != end; ++i)
				{
					int idx = *i;
					NewURL (SavedSessionState_.at (idx).second)->
						SetWidgetSettings (SavedSessionSettings_.at (idx));
				}

				SavedSessionState_.clear ();
				SavedSessionSettings_.clear ();
			
				saveSession ();
			}
			
			void Core::postConstruct ()
			{
				bool cleanShutdown = XmlSettingsManager::Instance ()->
					Property ("CleanShutdown", true).toBool ();
				XmlSettingsManager::Instance ()->setProperty ("CleanShutdown", false);
			
				if (!cleanShutdown)
					RestoreSession (true);
				else if (XmlSettingsManager::Instance ()->
						property ("RestorePreviousSession").toBool ())
					RestoreSession (false);
			}
		};
	};
};

