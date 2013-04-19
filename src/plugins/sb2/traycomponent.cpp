/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
