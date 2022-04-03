/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tabsessmanager.h"
#include <algorithm>
#include <QIcon>
#include <QTimer>
#include <QSettings>
#include <QApplication>
#include <QtDebug>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/ihavetabs.h>
#include "sessionmenumanager.h"
#include "sessionsmanager.h"
#include "unclosemanager.h"
#include "tabspropsmanager.h"

namespace LC::TabSessManager
{
	struct Plugin::Managers
	{
		TabsPropsManager TabsPropsMgr_;
		UncloseManager UncloseMgr_ { &TabsPropsMgr_ };
		SessionsManager SessionsMgr_ { &TabsPropsMgr_ };
		SessionMenuManager SessionMenuMgr_ { &SessionsMgr_ };

		Managers ()
		{
			QObject::connect (&SessionsMgr_,
					&SessionsManager::gotCustomSession,
					&SessionMenuMgr_,
					&SessionMenuManager::AddCustomSession);
		}
	};

	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("tabsessmanager");

		Mgrs_ = std::make_shared<Managers> ();

		for (const auto& name : Mgrs_->SessionsMgr_.GetCustomSessions ())
			Mgrs_->SessionMenuMgr_.AddCustomSession (name);
	}

	void Plugin::SecondInit ()
	{
		using namespace std::chrono_literals;
		QTimer::singleShot (5ms,
				&Mgrs_->SessionsMgr_,
				&SessionsManager::Recover);
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.TabSessManager";
	}

	void Plugin::Release ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_TabSessManager");
		settings.setValue ("CleanShutdown", true);
	}

	QString Plugin::GetName () const
	{
		return "TabSessManager";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Manages sessions of tabs in LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Core.Plugins/1.0" };
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
	{
		if (!Mgrs_)
			return {};

		switch (place)
		{
		case ActionsEmbedPlace::ToolsMenu:
			return
			{
				Mgrs_->SessionMenuMgr_.GetSessionsAction (),
				Mgrs_->UncloseMgr_.GetMenuAction ()
			};
		case ActionsEmbedPlace::CommonContextMenu:
			return { Mgrs_->UncloseMgr_.GetMenuAction () };
		default:
			return {};
		}
	}

	void Plugin::HandleShutdownInitiated ()
	{
		Mgrs_.reset ();
	}

	void Plugin::hookTabIsRemoving (const IHookProxy_ptr&, int index, int windowId)
	{
		if (!Mgrs_)
			return;

		const auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();
		const auto tabWidget = rootWM->GetTabWidget (windowId);
		const auto widget = tabWidget->Widget (index);

		Mgrs_->UncloseMgr_.HandleRemoveTab (widget);
		Mgrs_->SessionsMgr_.handleRemoveTab (widget);
	}

	void Plugin::hookTabAdding (const IHookProxy_ptr&, QWidget *widget)
	{
		if (!Mgrs_)
			return;

		Mgrs_->TabsPropsMgr_.HandleTabAdding (widget);
	}

	void Plugin::hookGetPreferredWindowIndex (const IHookProxy_ptr& proxy, const QWidget *widget) const
	{
		if (!Mgrs_)
			return;

		Mgrs_->TabsPropsMgr_.HandlePreferredWindowIndex (proxy, widget);
	}
}

LC_EXPORT_PLUGIN (leechcraft_tabsessmanager, LC::TabSessManager::Plugin);
