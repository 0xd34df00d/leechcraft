/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
		TabClass_.Features_ = TFOpenableByRequest | TFSuggestOpening;

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

		std::shared_ptr<StorageBackend> sb;
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

		Initialized_ = true;
	}

	void Core::SecondInit ()
	{
		GetWebPluginFactory ()->refreshPlugins ();
	}

	void Core::Release ()
	{
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

	BrowserWidget* Core::NewURL (const QUrl& url, bool raise,
			const QList<QPair<QByteArray, QVariant>>& props)
	{
		if (!Initialized_)
			return 0;

		BrowserWidget *widget = new BrowserWidget ();
		widget->InitShortcuts ();
		Widgets_.push_back (widget);

		Q_FOREACH (const auto& pair, props)
			widget->setProperty (pair.first, pair.second);

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
		auto pos = std::find (Widgets_.begin (), Widgets_.end (), widget);
		if (pos == Widgets_.end ())
		{
			qWarning () << Q_FUNC_INFO << widget << "not found in the collection";
			return;
		}

		Widgets_.erase (pos);
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

	void Core::handleTitleChanged (const QString& newTitle)
	{
		emit changeTabName (dynamic_cast<QWidget*> (sender ()), newTitle);
	}

	void Core::handleURLChanged (const QString&)
	{
		HandleHistory (dynamic_cast<BrowserWidget*> (sender ())->GetView ());
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
}
}
