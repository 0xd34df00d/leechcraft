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
#include "sbwidget.h"

namespace LeechCraft
{
namespace Sidebar
{
	QLActionManager::QLActionManager (SBWidget *w,
			ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Bar_ (w)
	{
	}

	void QLActionManager::AddToQuickLaunch (const QList<QAction*>& actions)
	{
		Q_FOREACH (QAction *action, actions)
		{
			Proxy_->RegisterSkinnable (action);
			Bar_->AddQLAction (action);
		}
	}

	void QLActionManager::AddToLCTray (const QList<QAction*>& actions)
	{
		Q_FOREACH (QAction *action, actions)
		{
			Proxy_->RegisterSkinnable (action);
			Bar_->AddTrayAction (action);
		}
	}

	void QLActionManager::handleGotActions (const QList<QAction*>& actions, ActionsEmbedPlace aep)
	{
		if (aep == AEPQuickLaunch)
			AddToQuickLaunch (actions);
		else if (aep == AEPLCTray)
			AddToLCTray (actions);
	}
}
}
