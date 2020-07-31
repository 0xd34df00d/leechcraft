/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "onlinebookmarks.h"
#include <QIcon>
#include <QMenu>
#include <QStandardItemModel>
#include <QMessageBox>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "accountssettings.h"

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("poshuku_onlinebookmarks");
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

		if (XmlSettingsManager::Instance ()->Property ("DownloadGroup", false).toBool ())
			Core::Instance ().checkDownloadPeriod ();

		if (XmlSettingsManager::Instance ()->Property ("UploadGroup", false).toBool ())
			Core::Instance ().checkUploadPeriod ();

		connect (&Core::Instance (),
				SIGNAL (gotEntity (const LC::Entity&)),
				this,
				SIGNAL (gotEntity (const LC::Entity&)));
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
		return tr ("Sync local bookmarks with your account in online bookmark services like Read It Later.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
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

	void Plugin::hookMoreMenuFillEnd (IHookProxy_ptr, QMenu *menu, QObject*)
	{
		QMenu *menuBookmarksSyn = menu->addMenu (tr ("Sync bookmarks"));

		QAction *sync = menuBookmarksSyn->addAction (tr ("Sync"));
		sync->setProperty ("ActionIcon", "folder-sync");

		QAction *uploadOnly = menuBookmarksSyn->addAction (tr ("Upload only"));
		uploadOnly->setProperty ("ActionIcon", "svn-commit");

		QAction *downloadOnly = menuBookmarksSyn->addAction (tr ("Download only"));
		downloadOnly->setProperty ("ActionIcon", "svn-update");

		QAction *downloadAll = menuBookmarksSyn->addAction (tr ("Download all"));
		downloadAll->setProperty ("ActionIcon", "download");

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

}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_onlinebookmarks,
		LC::Poshuku::OnlineBookmarks::Plugin);
