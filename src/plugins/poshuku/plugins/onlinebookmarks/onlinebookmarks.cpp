/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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
#include "settings.h"
#include <typeinfo>
#include <QAction>
#include <QIcon>
#include <QStandardItemModel>
#include <QWebView>
#include <QMenu>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <plugininterface/util.h>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "syncbookmarks.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Poshuku
{
namespace Plugins
{
namespace OnlineBookmarks
{
	void OnlineBookmarks::Init (ICoreProxy_ptr proxy)
	{
		Translator_.reset (Util::InstallTranslator ("poshuku_onlinebookmarks"));

		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"poshukuonlinebookmarkssettings.xml");

		SettingsDialog_->SetCustomWidget ("Settings",
				new Settings (Core::Instance ().GetAccountModel (), this));

		Core::Instance ().SetProxy (proxy);
		Core::Instance ().SetBookamrksDir(Util::CreateIfNotExists ("poshuku/onlinebookmarks"));
		
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

	QStringList OnlineBookmarks::Provides () const
	{
		return QStringList ();
	}

	QStringList OnlineBookmarks::Needs () const
	{
		return QStringList ();
	}

	QStringList OnlineBookmarks::Uses () const
	{
		return QStringList ();
	}

	void OnlineBookmarks::SetProvider (QObject *object, const QString& feature)
	{

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

	void OnlineBookmarks::hookMoreMenuFillEnd (IHookProxy_ptr proxy, QMenu *menu, QWebView *view, QObject *parent)
	{
		QMenu *menuBookmarksSyn = menu->addMenu (tr ("Bookmarks Sync"));
		QAction *sync = menuBookmarksSyn->addAction (tr ("Sync"));
		sync->setProperty ("ActionIcon", "poshuku_onlinebookmarks_sync");
		QAction *uploadOnly = menuBookmarksSyn->addAction (tr ("Upload only"));
		uploadOnly->setProperty ("ActionIcon", "poshuku_onlinebookmarks_upload");
		QAction *downloadOnly = menuBookmarksSyn->addAction (tr ("Download only"));
		downloadOnly->setProperty ("ActionIcon", "poshuku_onlinebookmarks_download");

		connect (sync,
				SIGNAL (triggered ()),
				Core::Instance ().GetBookmarksSyncManager (),
				SLOT (syncBookmarks ()));

		connect (uploadOnly,
				SIGNAL (triggered ()),
				Core::Instance ().GetBookmarksSyncManager (),
				SLOT (uploadBookmarks ()));

		connect (downloadOnly,
				SIGNAL (triggered ()),
				Core::Instance ().GetBookmarksSyncManager (),
				SLOT (downloadBookmarks ()));
	}

	void OnlineBookmarks::initPlugin (QObject *proxy)
	{
		Core::Instance ().SetPluginProxy (proxy);
	}
	
	void OnlineBookmarks::hookAddedToFavorites (IHookProxy_ptr proxy, QString title, QString url, QStringList tags)
	{
		
	}
}
}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_poshuku_onlinebookmarks, LeechCraft::Plugins::Poshuku::Plugins::OnlineBookmarks::OnlineBookmarks);

