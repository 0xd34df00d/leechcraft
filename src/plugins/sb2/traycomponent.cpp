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

#include "traycomponent.h"
#include <QDockWidget>
#include <interfaces/iactionsexporter.h>
#include <interfaces/core/ipluginsmanager.h>

namespace LeechCraft
{
namespace SB2
{
	TrayComponent::TrayComponent (ICoreProxy_ptr proxy, SBView *view, QObject *parent)
	: BaseActionComponent ({ "SB2_TrayActionImage", "TrayComponent.qml", "SB2_trayModel" }, proxy, view, parent)
	{
		const auto& hasActions = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IActionsExporter*> ();
		for (QObject *actObj : hasActions)
			connect (actObj,
					SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)),
					this,
					SLOT (handleGotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)));
	}

	void TrayComponent::HandleDockAction (QDockWidget *dw, bool visible)
	{
		QAction *act = dw->toggleViewAction ();
		if (!visible)
			RemoveAction (act);
		else
			AddActions ({ act }, ActionPos::Beginning);
	}

	void TrayComponent::handlePluginsAvailable ()
	{
		const auto places = { ActionsEmbedPlace::QuickLaunch, ActionsEmbedPlace::LCTray };
		const auto& hasActions = Proxy_->GetPluginsManager ()->
				GetAllCastableTo<IActionsExporter*> ();
		for (auto place : places)
			for (auto exp : hasActions)
				handleGotActions (exp->GetActions (place), place);
	}

	void TrayComponent::handleGotActions (const QList<QAction*>& acts, ActionsEmbedPlace aep)
	{
		if (aep != ActionsEmbedPlace::LCTray && aep != ActionsEmbedPlace::QuickLaunch)
			return;

		AddActions (acts, ActionPos::End);
	}
}
}
