/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "sidebar.h"
#include <QIcon>
#include <QAction>
#include <QMainWindow>
#include <QStatusBar>
#include <interfaces/imwproxy.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihavetabs.h>
#include "sbwidget.h"
#include "newtabactionmanager.h"
#include "qlactionmanager.h"
#include "openedtabmanager.h"

namespace LeechCraft
{
namespace Sidebar
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Bar_ = new SBWidget (proxy);
		NewTabMgr_ = new NewTabActionManager (Bar_, this);
		QLMgr_ = new QLActionManager (Bar_, Proxy_, this);
		OTMgr_ = new OpenedTabManager (Bar_, Proxy_, this);

		Proxy_->GetMWProxy ()->AddSideWidget (Bar_);
		Proxy_->GetMainWindow ()->statusBar ()->hide ();

		const auto& hasTabs = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IHaveTabs*> ();
		Q_FOREACH (QObject *ihtObj, hasTabs)
			OTMgr_->ManagePlugin (ihtObj);

		const auto& hasActions = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IActionsExporter*> ();
		Q_FOREACH (QObject *actObj, hasActions)
			connect (actObj,
					SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)),
					QLMgr_,
					SLOT (handleGotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)));
	}

	void Plugin::SecondInit ()
	{
		auto hasTabs = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IHaveTabs*> ();
		Q_FOREACH (QObject *ihtObj, hasTabs)
		{
			IHaveTabs *iht = qobject_cast<IHaveTabs*> (ihtObj);

			Q_FOREACH (const TabClassInfo& tc, iht->GetTabClasses ())
				NewTabMgr_->AddTabClassOpener (tc, ihtObj);
		}

		const auto& hasActions = Proxy_->GetPluginsManager ()->
				GetAllCastableTo<IActionsExporter*> ();
		Q_FOREACH (IActionsExporter *exp, hasActions)
		{
			const auto& acts = exp->GetActions (AEPLCTray);
			if (!acts.isEmpty ())
				QLMgr_->AddToLCTray (acts);
		}
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Sidebar";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Sidebar";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("A nice sidebar with quick launch area, tabs and tray-like area.");
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

	void Plugin::hookGonnaFillQuickLaunch (IHookProxy_ptr proxy)
	{
		proxy->CancelDefault ();

		auto exporters = Proxy_->GetPluginsManager ()->
				GetAllCastableTo<IActionsExporter*> ();

		Q_FOREACH (IActionsExporter *exp, exporters)
		{
			const auto& actions = exp->GetActions (AEPQuickLaunch);
			if (actions.isEmpty ())
				continue;

			QLMgr_->AddToQuickLaunch (actions);
		}
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_sidebar, LeechCraft::Sidebar::Plugin);
