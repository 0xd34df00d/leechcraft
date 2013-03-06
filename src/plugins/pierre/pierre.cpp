/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "pierre.h"
#include <QIcon>
#include <QMenuBar>
#include <QMainWindow>
#include <QTimer>
#include <QSystemTrayIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/imwproxy.h>
#include <interfaces/iactionsexporter.h>
#include "fullscreen.h"
#include "dockutil.h"

extern void qt_mac_set_dock_menu (QMenu*);

namespace LeechCraft
{
namespace Pierre
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		TrayIconMenu_ = 0;

		Proxy_ = proxy;
		MenuBar_ = new QMenuBar (0);
	}

	void Plugin::SecondInit ()
	{
		auto rootWM = Proxy_->GetRootWindowsManager ();
		for (int i = 0; i < rootWM->GetWindowsCount (); ++i)
			handleWindow (i);

		connect (rootWM->GetObject (),
				SIGNAL (windowAdded (int)),
				this,
				SLOT (handleWindow (int)));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Pierre";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Pierre";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Pierre d'Olle is the Mac OS X integration layer.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& entity) const
	{
		const bool isCountInfo = entity.Mime_ == "x-leechcraft/notification-event-count-info";
		return EntityTestHandleResult (isCountInfo ?
					EntityTestHandleResult::PIdeal :
					EntityTestHandleResult::PNone);
	}

	void Plugin::Handle (Entity e)
	{
		const int count = e.Entity_.toInt ();
		DU::SetDockBadge (count ? QString::number (count) : QString ());
	}

	void Plugin::hookGonnaFillMenu (IHookProxy_ptr)
	{
		QTimer::singleShot (0,
				this,
				SLOT (fillMenu ()));
	}

	void Plugin::hookTrayIconCreated (IHookProxy_ptr proxy, QSystemTrayIcon *icon)
	{
		TrayIconMenu_ = icon->contextMenu ();
		qt_mac_set_dock_menu (TrayIconMenu_);
	}

	void Plugin::hookTrayIconVisibilityChanged (IHookProxy_ptr proxy, QSystemTrayIcon*, bool)
	{
		proxy->CancelDefault ();
	}

	void Plugin::handleGotActions (const QList<QAction*>&, ActionsEmbedPlace aep)
	{
		if (aep != ActionsEmbedPlace::ToolsMenu)
			return;

		MenuBar_->clear ();
		QTimer::singleShot (0,
				this,
				SLOT (fillMenu ()));
	}

	void Plugin::handleWindow (int index)
	{
		qDebug () << Q_FUNC_INFO;
		auto rootWM = Proxy_->GetRootWindowsManager ();
		FS::AddAction (rootWM->GetMainWindow (index));
	}

	void Plugin::fillMenu ()
	{
		auto rootWM = Proxy_->GetRootWindowsManager ();
		auto menu = rootWM->GetMWProxy (0)->GetMainMenu ();

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

		for (auto act : firstLevelActions)
			lcMenu->addAction (act);

		if (!lcMenu->actions ().isEmpty ())
			MenuBar_->addMenu (lcMenu);

		const auto& actors = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IActionsExporter*> ();
		for (auto actor : actors)
			connect (actor,
					SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)),
					this,
					SLOT (handleGotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)),
					Qt::UniqueConnection);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_pierre, LeechCraft::Pierre::Plugin);
