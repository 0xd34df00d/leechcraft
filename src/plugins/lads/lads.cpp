/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Azer Abdullaev
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
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/imwproxy.h>

#warning "Don't forget to add support for multiple windows here."

namespace LeechCraft
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
		static QIcon icon (":/resources/images/lads.svg");
		return icon;
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
		Q_FOREACH (auto action, menu->actions ())
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

		Q_FOREACH (auto act, firstLevelActions)
			lcMenu->addAction (act);

		if (!lcMenu->actions ().isEmpty ())
			MenuBar_->addMenu (lcMenu);

		const auto& actors = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IActionsExporter*> ();
		Q_FOREACH (auto actor, actors)
			connect (actor,
					SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)),
					this,
					SLOT (handleGotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)));
	}

}
}

LC_EXPORT_PLUGIN (leechcraft_lads, LeechCraft::Lads::Plugin);

