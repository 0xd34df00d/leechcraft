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

#include "qlactionmanager.h"
#include <QtDebug>
#include "sbwidget.h"
#include "showconfigdialog.h"

namespace LeechCraft
{
namespace Sidebar
{
	QLActionManager::QLActionManager (SBWidget *w,
			ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Bar_ (w)
	, CfgDialog_ (new ShowConfigDialog ("QL"))
	{
		connect (CfgDialog_.get (),
				SIGNAL (showActions (QList<QAction*>)),
				this,
				SLOT (handleShowActions (QList<QAction*>)));
		connect (CfgDialog_.get (),
				SIGNAL (hideActions (QList<QAction*>)),
				this,
				SLOT (handleHideActions (QList<QAction*>)));

		QAction *cfgAction = new QAction (tr ("Configure tray actions..."), this);
		connect (cfgAction,
				SIGNAL (triggered ()),
				CfgDialog_.get (),
				SLOT (show ()));
		w->addAction (cfgAction);
	}

	void QLActionManager::AddToQuickLaunch (const QList<QAction*>& actions)
	{
		AddLabeled (actions, "QL");
	}

	void QLActionManager::AddToLCTray (const QList<QAction*>& actions)
	{
		AddLabeled (actions, "LCTray");
	}

	void QLActionManager::AddLabeled (const QList<QAction*>& actions, const QString& label)
	{
		QList<QAction*> actions2add;
		Q_FOREACH (QAction *action, actions)
		{
			action->setProperty ("Sidebar/Type", label);
			Proxy_->RegisterSkinnable (action);

			const QString& id = action->property ("Action/ID").toString ();
			if (id.isEmpty () || CfgDialog_->CheckAction (id, action))
				actions2add << action;
		}

		handleShowActions (actions2add);
	}

	void QLActionManager::handleGotActions (const QList<QAction*>& actions, ActionsEmbedPlace aep)
	{
		if (aep == AEPQuickLaunch)
			AddToQuickLaunch (actions);
		else if (aep == AEPLCTray)
			AddToLCTray (actions);
	}

	void QLActionManager::handleHideActions (const QList<QAction*>& acts)
	{
		Q_FOREACH (QAction *act, acts)
		{
			if (act->property ("Sidebar/Type") == "QL")
				Bar_->RemoveQLAction (act);
			else
				Bar_->RemoveTrayAction (act);
		}
	}

	void QLActionManager::handleShowActions (const QList<QAction*>& acts)
	{
		Q_FOREACH (QAction *act, acts)
		{
			if (act->property ("Sidebar/Type") == "QL")
				Bar_->AddQLAction (act);
			else
				Bar_->AddTrayAction (act);
		}
	}
}
}
