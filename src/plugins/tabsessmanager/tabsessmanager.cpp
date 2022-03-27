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

namespace LC
{
namespace TabSessManager
{
	struct Plugin::Managers
	{
		TabsPropsManager TabsPropsMgr_;
		UncloseManager UncloseMgr_ { &TabsPropsMgr_ };
		SessionsManager SessionsMgr_ { &TabsPropsMgr_ };
		SessionMenuManager SessionMenuMgr_ { &SessionsMgr_ };

		Managers ()
		{
			QObject::connect (&SessionMenuMgr_,
					SIGNAL (loadRequested (QString)),
					&SessionsMgr_,
					SLOT (loadCustomSession (QString)));
			QObject::connect (&SessionMenuMgr_,
					SIGNAL (addRequested (QString)),
					&SessionsMgr_,
					SLOT (addCustomSession (QString)));
			QObject::connect (&SessionMenuMgr_,
					SIGNAL (deleteRequested (QString)),
					&SessionsMgr_,
					SLOT (deleteCustomSession (QString)));
			QObject::connect (&SessionMenuMgr_,
					SIGNAL (saveCustomSessionRequested ()),
					&SessionsMgr_,
					SLOT (saveCustomSession ()));

			QObject::connect (&SessionsMgr_,
					SIGNAL (gotCustomSession (QString)),
					&SessionMenuMgr_,
					SLOT (addCustomSession (QString)));
		}
	};

	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("tabsessmanager");

		Mgrs_ = std::make_shared<Managers> ();

		for (const auto& name : Mgrs_->SessionsMgr_.GetCustomSessions ())
			Mgrs_->SessionMenuMgr_.addCustomSession (name);
	}

	void Plugin::SecondInit ()
	{
		QTimer::singleShot (5000,
				&Mgrs_->SessionsMgr_,
				SLOT (recover ()));
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
		QSet<QByteArray> result;
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
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

	void Plugin::hookTabIsRemoving (IHookProxy_ptr, int index, int windowId)
	{
		if (!Mgrs_)
			return;

		const auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();
		const auto tabWidget = rootWM->GetTabWidget (windowId);
		const auto widget = tabWidget->Widget (index);

		Mgrs_->UncloseMgr_.HandleRemoveTab (widget);
		Mgrs_->SessionsMgr_.handleRemoveTab (widget);
	}

	void Plugin::hookTabAdding (IHookProxy_ptr, QWidget *widget)
	{
		if (!Mgrs_)
			return;

		Mgrs_->TabsPropsMgr_.HandleTabAdding (widget);
	}

	void Plugin::hookGetPreferredWindowIndex (IHookProxy_ptr proxy, const QWidget *widget) const
	{
		if (!Mgrs_)
			return;

		Mgrs_->TabsPropsMgr_.HandlePreferredWindowIndex (proxy, widget);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_tabsessmanager, LC::TabSessManager::Plugin);
