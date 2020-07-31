/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Azer Abdullaev
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lads.h"
#include <QIcon>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QAction>
#include <QTimer>
#include <QMenuBar>
#include <QMainWindow>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/imwproxy.h>

#warning "Don't forget to add support for multiple windows here."

namespace LC
{
namespace Lads
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		UnityDetected_ = false;
		auto sb = QDBusConnection::sessionBus ();
		Action_ = 0;
		Proxy_ = proxy;
		MenuBar_ = new QMenuBar (0);
		MW_ = Proxy_->GetRootWindowsManager ()->GetMainWindow (0);
		const auto& services = sb.interface ()->registeredServiceNames ().value ();
		if (services.contains ("com.canonical.Unity"))
		{
			Action_ = new QAction (tr ("Show/hide LeechCraft window"), this);
			connect (Action_,
				SIGNAL (triggered ()),
				this,
				SLOT (showHideMain ()));
			UnityDetected_ = true;
			MW_->setMenuBar (MenuBar_);
		}
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Lads";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Lads";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Ubuntu Unity integration layer.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	void Plugin::hookGonnaFillMenu (IHookProxy_ptr)
	{
		if (!UnityDetected_)
			return;

		QTimer::singleShot (0,
				this,
				SLOT (fillMenu ()));
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;
		if (aep == ActionsEmbedPlace::TrayMenu && Action_)
			result << Action_;
		return result;
	}

	void Plugin::showHideMain () const
	{
		Proxy_->GetRootWindowsManager ()->GetMWProxy (0)->ToggleVisibility ();
	}

	void Plugin::handleGotActions (const QList<QAction*>&, ActionsEmbedPlace aep)
	{
		if (!UnityDetected_)
			return;

		if (aep != ActionsEmbedPlace::ToolsMenu)
			return;

		MenuBar_->clear ();
		QTimer::singleShot (0,
				this,
				SLOT (fillMenu ()));
	}

	void Plugin::fillMenu ()
	{
		if (!UnityDetected_)
			return;

		auto menu = Proxy_->GetRootWindowsManager ()->GetMWProxy (0)->GetMainMenu ();

		QMenu *lcMenu = 0;
		QList<QAction*> firstLevelActions;
		for (auto action : menu->actions ())
			if (action->menu ())
			{
				MenuBar_->addAction (action);
				if (!lcMenu)
					lcMenu = action->menu ();
			}
			else
			{
				if (action->menuRole () == QAction::TextHeuristicRole)
					action->setMenuRole (QAction::ApplicationSpecificRole);
				firstLevelActions << action;
			}

		if (!lcMenu)
		{
			qWarning () << Q_FUNC_INFO
					<< "LeechCraft menu not found";
			return;
		}

		for (auto act : irstLevelActions)
			lcMenu->addAction (act);

		if (!lcMenu->actions ().isEmpty ())
			MenuBar_->addMenu (lcMenu);

		for (auto actor : Proxy_->GetPluginsManager ()->GetAllCastableRoots<IActionsExporter*> ())
			connect (actor,
					SIGNAL (gotActions (QList<QAction*>, LC::ActionsEmbedPlace)),
					this,
					SLOT (handleGotActions (QList<QAction*>, LC::ActionsEmbedPlace)));
	}

}
}

LC_EXPORT_PLUGIN (leechcraft_lads, LC::Lads::Plugin);

