/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <QMainWindow>
#include <QMessageBox>
#include <QtDebug>
#include <QTimer>
#include <QNetworkProxy>
#include <QApplication>
#include <QAction>
#include <QToolBar>
#include <QDir>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QAbstractNetworkCache>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QInputDialog>
#include <QMimeData>
#include <QNetworkCookie>
#include <QTimer>
#include <util/xpc/util.h>
#include <util/network/customcookiejar.h>
#include <util/sll/prelude.h>
#include <util/xpc/defaulthookproxy.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ijobholder.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/isummaryrepresentation.h>
#include <interfaces/structures.h>
#include <interfaces/entitytesthandleresult.h>
#include "application.h"
#include "mainwindow.h"
#include "pluginmanager.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "handlerchoicedialog.h"
#include "tagsmanager.h"
#include "application.h"
#include "newtabmenumanager.h"
#include "networkaccessmanager.h"
#include "tabmanager.h"
#include "localsockethandler.h"
#include "coreinstanceobject.h"
#include "coreplugin2manager.h"
#include "dockmanager.h"
#include "entitymanager.h"
#include "rootwindowsmanager.h"

using namespace LC::Util;

namespace LC
{
	Core::Core ()
	: NetworkAccessManager_ (new NetworkAccessManager)
	, LocalSocketHandler_ (new LocalSocketHandler)
	, NewTabMenuManager_ (new NewTabMenuManager)
	, CoreInstanceObject_ (new CoreInstanceObject)
	, RootWindowsManager_ (new RootWindowsManager)
	, DM_ (new DockManager (RootWindowsManager_.get (), this))
	{
		CoreInstanceObject_->GetCorePluginManager ()->RegisterHookable (NetworkAccessManager_.get ());
		CoreInstanceObject_->GetCorePluginManager ()->RegisterHookable (DM_);
		CoreInstanceObject_->GetCorePluginManager ()->RegisterHookable (RootWindowsManager_.get ());

		connect (RootWindowsManager_.get (),
				SIGNAL (tabIsMoving (int, int, int)),
				DM_,
				SLOT (handleTabMove (int, int, int)));

		connect (CoreInstanceObject_->GetSettingsDialog ().get (),
				SIGNAL (pushButtonClicked (const QString&)),
				this,
				SLOT (handleSettingClicked (const QString&)));

		connect (NetworkAccessManager_.get (),
				SIGNAL (error (const QString&)),
				this,
				SIGNAL (error (const QString&)));

		QStringList paths;
		const auto& map = qobject_cast<Application*> (qApp)->GetVarMap ();
		if (map.count ("plugin"))
		{
			const auto& plugins = map ["plugin"].as<std::vector<std::string>> ();
			for (const auto& plugin : plugins)
				paths << QString::fromUtf8 (plugin.c_str ());
		}
		PluginManager_ = new PluginManager (paths, this);
	}

	Core& Core::Instance ()
	{
		static Core core;
		return core;
	}

	void Core::Release ()
	{
		if (IsShuttingDown_)
			return;

		IsShuttingDown_ = true;

		RootWindowsManager_->Release ();

		QTimer::singleShot (0, this,
				[this]
				{
					LocalSocketHandler_.reset ();
					XmlSettingsManager::Instance ()->setProperty ("FirstStart", "false");

					PluginManager_->Release ();
					delete PluginManager_;

					CoreInstanceObject_.reset ();

					NetworkAccessManager_.reset ();

					XmlSettingsManager::Instance ()->Release ();

					qApp->quit ();
				});
	}

	bool Core::IsShuttingDown () const
	{
		return IsShuttingDown_;
	}

	DockManager* Core::GetDockManager () const
	{
		return DM_;
	}

	IShortcutProxy* Core::GetShortcutProxy () const
	{
		return CoreInstanceObject_->GetShortcutProxy ();
	}

	QObjectList Core::GetSettables () const
	{
		return PluginManager_->GetAllCastableRoots<IHaveSettings*> ();
	}

	QList<QList<QAction*>> Core::GetActions2Embed () const
	{
		const auto& plugins = PluginManager_->GetAllCastableTo<IActionsExporter*> ();

		auto result = Util::Map (plugins,
				[] (const IActionsExporter *plugin)
					{ return plugin->GetActions (ActionsEmbedPlace::CommonContextMenu); });
		result.removeAll ({});
		return result;
	}

	QAbstractItemModel* Core::GetPluginsModel () const
	{
		return PluginManager_;
	}

	PluginManager* Core::GetPluginManager () const
	{
		return PluginManager_;
	}

	CoreInstanceObject* Core::GetCoreInstanceObject () const
	{
		return CoreInstanceObject_.get ();
	}

	void Core::PostSecondInit (QObject *plugin)
	{
		if (qobject_cast<IHaveTabs*> (plugin))
			GetNewTabMenuManager ()->AddObject (plugin);
	}

	void Core::DelayedInit ()
	{
		const auto& map = qobject_cast<Application*> (qApp)->GetVarMap ();
		if (map.count ("list-plugins"))
		{
			for (auto loader : PluginManager_->GetAllAvailable ())
				std::cout << "Found plugin: " << loader->GetFileName ().toUtf8 ().constData () << std::endl;
			std::exit (0);
		}

		PluginManager_->Init (map.count ("safe-mode"));

		NewTabMenuManager_->SetToolbarActions (GetActions2Embed ());

		QTimer::singleShot (10000,
				this,
				SLOT (handlePluginLoadErrors ()));

		emit initialized ();
	}

	QNetworkAccessManager* Core::GetNetworkAccessManager () const
	{
		return NetworkAccessManager_.get ();
	}

	RootWindowsManager* Core::GetRootWindowsManager () const
	{
		return RootWindowsManager_.get ();
	}

	QModelIndex Core::MapToSource (const QModelIndex& index) const
	{
		for (const auto summary : PluginManager_->GetAllCastableTo<ISummaryRepresentation*> ())
		{
			const QModelIndex& mapped = summary->MapToSource (index);
			if (mapped.isValid ())
				return mapped;
		}
		return {};
	}

	NewTabMenuManager* Core::GetNewTabMenuManager () const
	{
		return NewTabMenuManager_.get ();
	}

	void Core::handleSettingClicked (const QString& name)
	{
		auto win = RootWindowsManager_->GetPreferredWindow ();
		if (name == "ClearCache")
		{
			if (QMessageBox::question (win,
						"LeechCraft",
						tr ("Do you really want to clear the network cache?"),
						QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				return;

			QAbstractNetworkCache *cache = NetworkAccessManager_->cache ();
			if (cache)
				cache->clear ();
		}
		else if (name == "ClearCookies")
		{
			if (QMessageBox::question (win,
						"LeechCraft",
						tr ("Do you really want to clear cookies?"),
						QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				return;

			const auto jar = static_cast<CustomCookieJar*> (NetworkAccessManager_->cookieJar ());
			jar->setAllCookies ({});
		}
		else if (name == "SetStartupPassword")
		{
			if (QMessageBox::question (win,
						"LeechCraft",
						tr ("This security measure is easily circumvented by modifying "
							"LeechCraft's settings files (or registry on Windows) in a text "
							"editor. For proper and robust protection consider using some "
							"third-party tools like <em>encfs</em> (http://www.arg0.net/encfs/)."
							"<br/><br/>Accept this dialog if you understand the above and "
							"this kind of security through obscurity is OK for you."),
						QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok)
				return;

			bool ok = false;
			const auto& newPass = QInputDialog::getText (win,
					"LeechCraft",
					tr ("Enter new startup password:"),
					QLineEdit::Password,
					QString (),
					&ok);
			if (!ok)
				return;

			QString contents;
			if (!newPass.isEmpty ())
				contents = QCryptographicHash::hash (newPass.toUtf8 (), QCryptographicHash::Sha1).toHex ();
			XmlSettingsManager::Instance ()->setProperty ("StartupPassword", contents);
		}
	}

	bool Core::handleGotEntity (Entity p)
	{
		return EntityManager { nullptr, nullptr }.HandleEntity (p);
	}

	void Core::handlePluginLoadErrors ()
	{
		for (const auto& error : PluginManager_->GetPluginLoadErrors ())
			handleGotEntity (Util::MakeNotification (tr ("Plugin load error"),
					error, Priority::Critical));
	}
}
