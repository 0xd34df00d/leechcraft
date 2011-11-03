/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <stdexcept>
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
#include <QInputDialog>
#include <QNetworkReply>
#include <QSslSocket>
#include <QAuthenticator>
#include <QNetworkProxy>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <qwebframe.h>
#include <qwebhistory.h>
#include <QtDebug>
#include <QMainWindow>
#include <util/util.h>
#include <util/defaulthookproxy.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/core/icoreproxy.h>
#include "browserwidget.h"
#include "customwebview.h"
#include "addtofavoritesdialog.h"
#include "xmlsettingsmanager.h"
#include "restoresessiondialog.h"
#include "sqlstoragebackend.h"
#include "xbelparser.h"
#include "xbelgenerator.h"
#include "linkhistory.h"
#include "favoriteschecker.h"
#include "webpluginfactory.h"
#include "importentityhandler.h"

namespace LeechCraft
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
		qRegisterMetaType<BrowserWidgetSettings> ("LeechCraft::Poshuku::BrowserWidgetSettings");
		qRegisterMetaTypeStreamOperators<BrowserWidgetSettings> ("LeechCraft::Poshuku::BrowserWidgetSettings");

		qRegisterMetaType<ElementData> ("LeechCraft::Poshuku::ElementData");
		qRegisterMetaTypeStreamOperators<ElementData> ("LeechCraft::Poshuku::ElementData");
		qRegisterMetaType<ElementsData_t> ("LeechCraft::Poshuku::ElementsData_t");
		qRegisterMetaTypeStreamOperators<ElementsData_t> ("LeechCraft::Poshuku::ElementsData_t");

		TabClass_.TabClass_ = "Poshuku";
		TabClass_.VisibleName_ = tr ("Poshuku");
		TabClass_.Description_ = tr ("The Poshuku web browser");
		TabClass_.Icon_ = QIcon (":/resources/images/poshuku.svg");
		TabClass_.Priority_ = 80;
		TabClass_.Features_ = TFOpenableByRequest;

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
		PluginManager_->RegisterHookable (this);

		URLCompletionModel_.reset (new URLCompletionModel (this));
		PluginManager_->RegisterHookable (URLCompletionModel_.get ());

		QWebHistoryInterface::setDefaultInterface (new LinkHistory);
	}

	Core& Core::Instance ()
	{
		static Core core;
		return core;
	}

	void Core::Init ()
	{
		QDir dir = QDir::home ();
		if (!dir.cd (".leechcraft/poshuku") &&
				!dir.mkpath (".leechcraft/poshuku"))
		{
			qCritical () << Q_FUNC_INFO
				<< "could not create necessary directories for Poshuku";
			throw std::runtime_error ("could not create necessary directories for Poshuku");
		}

		StorageBackend::Type type;
		QString strType = XmlSettingsManager::Instance ()->
			property ("StorageType").toString ();
		if (strType == "SQLite")
			type = StorageBackend::SBSQLite;
		else if (strType == "PostgreSQL")
			type = StorageBackend::SBPostgres;
		else if (strType == "MySQL")
			type = StorageBackend::SBMysql;
		else
			throw std::runtime_error (qPrintable (QString ("Unknown storage type %1")
						.arg (strType)));

		boost::shared_ptr<StorageBackend> sb;
		try
		{
			sb = StorageBackend::Create (type);
			sb->Prepare ();
		}
		catch (const std::runtime_error& s)
		{
			emit error (QTextCodec::codecForName ("UTF-8")->
					toUnicode (s.what ()));
			throw;
		}
		catch (...)
		{
			emit error (tr ("Poshuku: general storage initialization error."));
			throw;
		}

		StorageBackend_ = sb;
		StorageBackend_->Prepare ();

		HistoryModel_.reset (new HistoryModel (this));
		connect (StorageBackend_.get (),
				SIGNAL (added (const HistoryItem&)),
				HistoryModel_.get (),
				SLOT (handleItemAdded (const HistoryItem&)));

		PluginManager_->RegisterHookable (HistoryModel_.get ());

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
	}

	void Core::SecondInit ()
	{
		GetWebPluginFactory ()->refreshPlugins ();
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

	TabClassInfo Core::GetTabClass () const
	{
		return TabClass_;
	}

	bool Core::CouldHandle (const Entity& e) const
	{
		if (!(e.Parameters_ & FromUserInitiated) ||
				e.Parameters_ & Internal)
			return false;

		if (e.Mime_ == "x-leechcraft/browser-import-data")
			return true;
		else if (e.Entity_.canConvert<QUrl> ())
		{
			QUrl url = e.Entity_.toUrl ();
			return (url.isValid () &&
				(url.scheme () == "http" || url.scheme () == "https"));
		}

		return false;
	}

	void Core::Handle (Entity e)
	{
		if (e.Mime_ == "x-leechcraft/browser-import-data")
		{
			std::auto_ptr<ImportEntityHandler> eh (new ImportEntityHandler (this));
			eh->Import (e);
		}
		else if (e.Entity_.canConvert<QUrl> ())
		{
			QUrl url = e.Entity_.toUrl ();
			NewURL (url, true);
		}
	}

	WebPluginFactory* Core::GetWebPluginFactory ()
	{
		if (!WebPluginFactory_)
			WebPluginFactory_ = new WebPluginFactory (this);
		return WebPluginFactory_;
	}

	QSet<QByteArray> Core::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	void Core::AddPlugin (QObject *plugin)
	{
		PluginManager_->AddPlugin (plugin);
	}

	QUrl Core::MakeURL (QString url)
	{
		if (url.isEmpty ())
			return QUrl ();

		url = url.trimmed ();
		if (url == "localhost")
			return QUrl ("http://localhost");

		if (url.startsWith ('!'))
		{
			HandleSearchRequest (url);
			return QUrl ();
		}

		QHostAddress testAddress;
		bool success = testAddress.setAddress (url);
		if (success)
		{
			QUrl result;
			result.setHost (url);
			result.setScheme ("http://");
			return result;
		}

		// If the url without percent signs and two following characters is
		// a valid url (it should not be percent-encoded), then treat source
		// url as percent-encoded, otherwise treat as not percent-encoded.
		QString withoutPercent = url;
		withoutPercent.remove (QRegExp ("%%??",
					Qt::CaseInsensitive, QRegExp::Wildcard));
		QUrl testUrl (withoutPercent);
		QUrl result;
		if (testUrl.toString () == withoutPercent)
			result = QUrl::fromEncoded (url.toUtf8 ());
		else
			result = QUrl (url);

		if (result.scheme ().isEmpty ())
		{
			if (!url.count (' ') && url.count ('.'))
				result = QUrl (QString ("http://") + url);
			else
			{
				url.replace ('+', "%2B");
				url.replace (' ', '+');
				QString urlStr = QString ("http://www.google.com/search?q=%2"
						"&client=leechcraft_poshuku"
						"&ie=utf-8"
						"&rls=org.leechcraft:%1")
					.arg (QLocale::system ().name ().replace ('_', '-'))
					.arg (url);
				result = QUrl::fromEncoded (urlStr.toUtf8 ());
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

		QString tabTitle = "Poshuku";
		if (url.host ().size ())
			tabTitle = url.host ();
		emit addNewTab (tabTitle, widget);

		ConnectSignals (widget);

		if (!url.isEmpty ())
			widget->SetURL (url);

		if (raise)
			emit raiseTab (widget);

		emit hookTabAdded (Util::DefaultHookProxy_ptr (new Util::DefaultHookProxy),
				widget,
				widget->getWebView (),
				url);

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
		connect (widget,
				SIGNAL (raiseTab (QWidget*)),
				this,
				SIGNAL (raiseTab (QWidget*)));
	}

	void Core::CheckFavorites ()
	{
		FavoritesChecker_->Check ();
	}

	void Core::ReloadAll ()
	{
		Q_FOREACH (BrowserWidget *widget, Widgets_)
			widget->GetView ()->
					pageAction (QWebPage::Reload)->trigger ();
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

	void Core::SetShortcut (const QString& name, const QKeySequences_t& shortcut)
	{
		Q_FOREACH (BrowserWidget *widget, Widgets_)
			widget->SetShortcut (name, shortcut);
	}

	IShortcutProxy* Core::GetShortcutProxy () const
	{
		return ShortcutProxy_;
	}

	QIcon Core::GetIcon (const QUrl& url) const
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy ());
		emit hookIconRequested (proxy, url);
		if (proxy->IsCancelled ())
			return proxy->GetReturnValue ().value<QIcon> ();

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
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy ());
		emit hookUserAgentForUrlRequested (proxy, url, page);
		if (proxy->IsCancelled ())
		{
			qDebug () << proxy->GetReturnValue ().toString ();
			return proxy->GetReturnValue ().toString ();
		}

		return QString ();

		/*
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
				winver = "unknown NT-based";
				break;
			case QSysInfo::WV_CE_based:
				winver = "unknown CE-based";
				break;
		}
#elif defined (Q_OS_DARWIN)
		QString macver;
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
			default:
				macver = "unknown Mac OS ";
				break;
		}
#endif
		return QString ("LeechCraft (%1; %2; %3; %4) (LeechCraft/Poshuku %5; WebKit %6/%7)")
			// %1 platform
#ifdef Q_WS_MAC
			.arg ("MacOS")
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
			.arg ("unknown subplatform")
#endif
			// %4 locale
			.arg (QLocale::system ().name ())
			.arg (LEECHCRAFT_VERSION)
			.arg (QT_VERSION_STR)
			.arg (qVersion ());
			*/
	}

	bool Core::IsUrlInFavourites (const QString& url)
	{
		return FavoritesModel_->IsUrlExists (url);
	}

	void Core::RemoveFromFavorites (const QString& url)
	{
		emit bookmarkRemoved (url);
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

			QByteArray ba;
			QDataStream out (&ba, QIODevice::WriteOnly);
			out << *widget->GetView ()->page ()->history ();

			UncloseData ud =
			{
				widget->GetView ()->url (),
				widget->GetView ()->page ()->mainFrame ()->scrollPosition (),
				ba
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

	void Core::RestoreSession (bool ask)
	{
		if (!SavedSessionState_.size ()) ;
		else if (ask)
		{
			std::auto_ptr<RestoreSessionDialog> dia (new RestoreSessionDialog (Core::Instance ().GetProxy ()->GetMainWindow ()));
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

		QList<QUrl> toRestore;
		Q_FOREACH (int idx, RestoredURLs_)
			toRestore << SavedSessionState_ [idx].second;

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookSessionRestoreScheduled (proxy,
				toRestore);
		if (proxy->IsCancelled ())
		{
			RestoredURLs_.clear ();
			SavedSessionState_.clear ();
		}
	}

	void Core::HandleHistory (CustomWebView *view)
	{
		QString url = view->URLToProperString (view->url ());

		if (!view->title ().isEmpty () &&
				!url.isEmpty () && url != "about:blank")
			HistoryModel_->addItem (view->title (),
					url,
					QDateTime::currentDateTime (),
					view->GetBrowserWidget ());
	}

	void Core::SetupConnections (BrowserWidget *widget)
	{
		connect (widget,
				SIGNAL (addToFavorites (const QString&, const QString&)),
				this,
				SLOT (handleAddToFavorites (const QString&, const QString&)));
		connect (widget,
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		connect (widget,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));
		connect (widget,
				SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)),
				this,
				SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)));
		connect (widget,
				SIGNAL (urlChanged (const QString&)),
				this,
				SLOT (handleURLChanged (const QString&)));
	}

	void Core::HandleSearchRequest (const QString& url)
	{
		const int pos = url.indexOf (' ');
		const QString& category = url.mid (1, pos - 1);
		const QString& query = url.mid (pos + 1);

		Entity e = Util::MakeEntity (query,
				QString (),
				FromUserInitiated,
				"x-leechcraft/category-search-request");
		e.Additional_ ["Categories"] = QStringList (category);
		emit gotEntity (e);
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
			QMessageBox::critical (Core::Instance ().GetProxy ()->GetMainWindow (),
					"LeechCraft",
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
			QMessageBox::critical (Core::Instance ().GetProxy ()->GetMainWindow (),
					"LeechCraft",
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
			QMessageBox::critical (Core::Instance ().GetProxy ()->GetMainWindow (),
					"LeechCraft",
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

		QDataStream str (ud.History_);
		str >> *bw->GetView ()->page ()->history ();

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

	void Core::handleAddToFavorites (QString title, QString url)
	{
		Util::DefaultHookProxy_ptr proxy = Util::DefaultHookProxy_ptr (new Util::DefaultHookProxy ());
		emit hookAddToFavoritesRequested (proxy, title, url);
		if (proxy->IsCancelled ())
			return;

		proxy->FillValue ("title", title);
		proxy->FillValue ("url", url);

		std::auto_ptr<AddToFavoritesDialog> dia (new AddToFavoritesDialog (title,
					url,
					qApp->activeWindow ()));

		bool result = false;
		bool oneClick = XmlSettingsManager::Instance ()->property ("BookmarkInOneClick").toBool ();

		do
		{
			if (!oneClick)
			{
				if (dia->exec () == QDialog::Rejected)
					return;

				result = FavoritesModel_->addItem (dia->GetTitle (),
						url, dia->GetTags ());
			}
			else
			{
				result = FavoritesModel_->addItem (title,
						url, QStringList ());
				oneClick = false;
			}
		}
		while (!result);

		emit bookmarkAdded (url);
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

		QList<QString> titles;
		QList<QString> urls;
		QList<BrowserWidgetSettings> bwsettings;
		for (widgets_t::const_iterator i = Widgets_.begin (),
				end = Widgets_.end (); i != end; ++i)
		{
			titles << (*i)->GetView ()->title ();
			urls << (*i)->GetView ()->url ().toString ();
			bwsettings << (*i)->GetWidgetSettings ();
		}

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Poshuku");
		settings.beginWriteArray ("Saved session");
		settings.remove ("");
		for (int i = 0; i < titles.size (); ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("Title", titles.at (i));
			settings.setValue ("URL", urls.at (i));
			settings.setValue ("Settings", QVariant::fromValue<BrowserWidgetSettings> (bwsettings.at (i)));
		}
		settings.endArray ();
		settings.sync ();
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

		// It looks like QSettings determines array size by last used index
		// no matter what was passed to QSettings::beginWriteArray (). Forcing correct size
		settings.setArrayIndex (Widgets_.size () - 1);
		settings.endArray ();
		settings.sync ();
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
}
}
