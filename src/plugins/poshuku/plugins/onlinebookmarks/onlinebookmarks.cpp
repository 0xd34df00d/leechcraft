/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "onlinebookmarks.h"
#include <typeinfo>
#include <QAction>
#include <QIcon>
#include <QWebView>
#include <QMenu>
#include <QStandardItemModel>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include "bookmarksdialog.h"
#include "core.h"
#include "syncbookmarks.h"
#include "settings.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	void OnlineBookmarks::Init (ICoreProxy_ptr proxy)
	{
		Translator_.reset (Util::InstallTranslator ("poshuku_onlinebookmarks"));
		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"poshukuonlinebookmarkssettings.xml");

		Core::Instance ().Init ();
		Core::Instance ().SetProxy (proxy);

		SettingsDialog_->SetCustomWidget ("Accounts", Core::Instance ().GetAccountsWidget ());
		SettingsDialog_->SetDataSource ("ActiveServices", Core::Instance ().GetServiceModel ());

		if (XmlSettingsManager::Instance ()->property ("DownloadGroup").toBool () &&
				XmlSettingsManager::Instance ()->property ("DownloadPeriod").toInt ())
			Core::Instance ().GetBookmarksSyncManager ()->checkDownloadPeriod ();

		if (XmlSettingsManager::Instance ()->property ("UploadGroup").toBool () &&
				XmlSettingsManager::Instance ()->property ("UploadPeriod").toInt ())
			Core::Instance ().GetBookmarksSyncManager ()->checkUploadPeriod ();

		Core::Instance ().SetBookmarksDir (Util::CreateIfNotExists ("poshuku/onlinebookmarks"));

		connect (&Core::Instance (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));

		connect (&Core::Instance (),
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));
	}

	void OnlineBookmarks::SecondInit ()
	{
	}

	void OnlineBookmarks::Release ()
	{
	}

	QByteArray OnlineBookmarks::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.OnlineBookmarks";
	}

	QString OnlineBookmarks::GetName () const
	{
		return "Poshuku OnlineBookmarks";
	}

	QString OnlineBookmarks::GetInfo () const
	{
		return tr ("Sync local bookmarks with your account in online bookmark services like Read It Later");
	}

	QIcon OnlineBookmarks::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr OnlineBookmarks::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	QSet<QByteArray> OnlineBookmarks::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	void OnlineBookmarks::hookMoreMenuFillEnd (IHookProxy_ptr, QMenu *menu, QWebView*, QObject*)
	{
		QMenu *menuBookmarksSyn = menu->addMenu (tr ("Bookmarks Sync"));
		QAction *sync = menuBookmarksSyn->addAction (tr ("Sync"));
		sync->setProperty ("ActionIcon", "poshuku_onlinebookmarks_sync");
		QAction *uploadOnly = menuBookmarksSyn->addAction (tr ("Upload only"));
		uploadOnly->setProperty ("ActionIcon", "poshuku_onlinebookmarks_upload");
		QAction *downloadOnly = menuBookmarksSyn->addAction (tr ("Download only"));
		downloadOnly->setProperty ("ActionIcon", "poshuku_onlinebookmarks_download");
		QAction *downloadAll = menuBookmarksSyn->addAction (tr ("Download all"));
		downloadAll->setProperty ("ActionIcon", "poshuku_onlinebookmarks_downloadall");

		connect (sync,
				SIGNAL (triggered ()),
				Core::Instance ().GetBookmarksSyncManager (),
				SLOT (syncBookmarks ()));

		connect (uploadOnly,
				SIGNAL (triggered ()),
				Core::Instance ().GetBookmarksSyncManager (),
				SLOT (uploadBookmarksAction ()));

		connect (downloadOnly,
				SIGNAL (triggered ()),
				Core::Instance ().GetBookmarksSyncManager (),
				SLOT (downloadBookmarksAction()));

		connect (downloadAll,
				SIGNAL (triggered ()),
				Core::Instance ().GetBookmarksSyncManager (),
				SLOT (downloadAllBookmarksAction ()));
	}

	void OnlineBookmarks::initPlugin (QObject *proxy)
	{
		Core::Instance ().SetPluginProxy (proxy);
	}

	void OnlineBookmarks::hookAddedToFavorites (IHookProxy_ptr,
			QString title, QString url, QStringList tags)
	{
		if (XmlSettingsManager::Instance ()->Property ("ConfirmSend", 0).toBool () &&
				!Core::Instance ().GetBookmarksSyncManager ()->IsUrlInUploadFile (url))
		{
			BookmarksDialog bd;
			bd.SetBookmark (title, url, tags);
			if (bd.exec () != QDialog::Accepted)
				return;
			bd.SendBookmark ();
		}
		else
			Core::Instance ().GetBookmarksSyncManager ()->uploadBookmarksAction (title, url, tags);
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_poshuku_onlinebookmarks, LeechCraft::Poshuku::OnlineBookmarks::OnlineBookmarks);
