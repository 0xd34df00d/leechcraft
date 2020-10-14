/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "traycomponent.h"
#include <QDockWidget>
#include <interfaces/iactionsexporter.h>
#include <interfaces/core/ipluginsmanager.h>

namespace LC::SB2
{
	TrayComponent::TrayComponent (ICoreProxy_ptr proxy, SBView *view, QObject *parent)
	: BaseActionComponent ({ "SB2_TrayActionImage", "TrayComponent.qml", "SB2_trayModel" }, proxy, view, parent)
	{
		const auto& hasActions = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IActionsExporter*> ();
		for (QObject *actObj : hasActions)
			connect (actObj,
					SIGNAL (gotActions (QList<QAction*>, LC::ActionsEmbedPlace)),
					this,
					SLOT (handleGotActions (QList<QAction*>, LC::ActionsEmbedPlace)));
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
