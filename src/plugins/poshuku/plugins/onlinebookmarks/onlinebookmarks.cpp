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
#include <QIcon>
#include <QMenu>
#include <QStandardItemModel>
#include <QMessageBox>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "accountssettings.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Translator_.reset (Util::InstallTranslator ("poshuku_onlinebookmarks"));
		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"poshukuonlinebookmarkssettings.xml");

		Core::Instance ().SetProxy (proxy);
	}

	void Plugin::SecondInit ()
	{
		SettingsDialog_->SetCustomWidget ("Accounts",
				Core::Instance ().GetAccountsSettingsWidget ());
		Core::Instance ().GetAccountsSettingsWidget ()->InitServices ();
		SettingsDialog_->SetDataSource ("QuickUploadAccounts",
				Core::Instance ().GetQuickUploadModel ());

		if (XmlSettingsManager::Instance ()->Property ("DownloadGroup", false).toBool ())
			Core::Instance ().checkDownloadPeriod ();

		if (XmlSettingsManager::Instance ()->Property ("UploadGroup", false).toBool ())
			Core::Instance ().checkUploadPeriod ();

		connect (&Core::Instance (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		connect (&Core::Instance (),
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));

		Core::Instance ().SetQuickUploadButtons ();
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.OnlineBookmarks";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku OnlineBookmarks";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Sync local bookmarks with your account in online bookmark services like Read It Later");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/plugins/poshuku/plugins/onlinebookmarks/resources/images/onlinebookmarks.svg");
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		return Core::Instance ().GetExpectedPluginClasses ();
	}

	void Plugin::AddPlugin (QObject *plugin)
	{
		Core::Instance ().AddPlugin (plugin);
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		Core::Instance ().SetPluginProxy (proxy);
	}

	void Plugin::hookMoreMenuFillEnd (IHookProxy_ptr, QMenu *menu, QGraphicsWebView*, QObject*)
	{
		QMenu *menuBookmarksSyn = menu->addMenu (tr ("Sync bookmarks"));

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
				&Core::Instance (),
				SLOT (syncBookmarks ()));

		connect (uploadOnly,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (uploadBookmarks ()));

		connect (downloadOnly,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (downloadBookmarks()));

		connect (downloadAll,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (downloadAllBookmarks ()));
	}

// 	void Plugin::hookAddedToFavorites (IHookProxy_ptr, QString title, QString url, QStringList tags)
// 	{
// 		bool res = false;
// 		if (!XmlSettingsManager::Instance ()->Property ("ConfirmSend", 0).toBool ())
// 			res = true;
// 		else
// 		{
// 			int result = QMessageBox::question (0,
// 					"OnlineBookmarks",
// 					tr ("Send bookmark to active services"),
// 					QMessageBox::Ok | QMessageBox::Cancel,
// 					QMessageBox::Ok);
// 			if (result == QMessageBox::Ok)
// 				res = true;
// 		}
// 		if (res)
// 			Core::Instance ().UploadBookmark (title, url, tags);
// 	}

}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_poshuku_onlinebookmarks,
		LeechCraft::Poshuku::OnlineBookmarks::Plugin);
