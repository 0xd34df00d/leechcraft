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

#include "docktoolbarmanager.h"
#include <QToolBar>
#include <QDockWidget>
#include <QActionGroup>
#include <QTimer>
#include <util/defaulthookproxy.h>
#include "mainwindow.h"
#include "core.h"
#include "coreinstanceobject.h"
#include "coreplugin2manager.h"

namespace LeechCraft
{
	DockToolbarManager::DockToolbarManager (MainWindow *win)
	: QObject (win)
	, Win_ (win)
	{
		auto instanceObj = Core::Instance ().GetCoreInstanceObject ();
		instanceObj->GetCorePluginManager ()->RegisterHookable (this);

		auto init = [this, win] (Qt::DockWidgetArea area) -> void
		{
			auto bar = win->GetDockListWidget (area);
			bar->setFloatable (false);
			bar->setMovable (false);
			bar->hide ();
			bar->setToolButtonStyle (Qt::ToolButtonIconOnly);
			Area2Info_ [area].Bar_ = bar;
		};
		init (Qt::LeftDockWidgetArea);
		init (Qt::RightDockWidgetArea);
		init (Qt::TopDockWidgetArea);
		init (Qt::BottomDockWidgetArea);
	}

	void DockToolbarManager::AddDock (QDockWidget *dw, Qt::DockWidgetArea area)
	{
		const auto& areaInfo = Area2Info_ [area];

		auto bar = areaInfo.Bar_;
		if (!bar)
			return;

		auto toggleAct = dw->toggleViewAction ();
		if (bar->actions ().contains (toggleAct))
		{
			qWarning () << Q_FUNC_INFO
					<< "double-adding"
					<< dw;
			return;
		}

		emit hookAddingDockAction (IHookProxy_ptr (new Util::DefaultHookProxy),
				Win_, toggleAct, area);
		bar->addAction (toggleAct);
		if (bar->actions ().size () >= 2)
		{
			Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
			emit hookDockBarWillBeShown (proxy, Win_, bar, area);
			if (!proxy->IsCancelled ())
				bar->show ();
		}

		Action2Widget_ [toggleAct] = dw;

		connect (dw,
				SIGNAL (dockLocationChanged (Qt::DockWidgetArea)),
				this,
				SLOT (updateDockLocation (Qt::DockWidgetArea)));
		connect (toggleAct,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleActionToggled (bool)));

		if (toggleAct->isChecked ())
		{
			UpdateActionGroup (toggleAct, true);
			QTimer::singleShot (0, dw, SLOT (show ()));
		}
	}

	void DockToolbarManager::RemoveDock (QDockWidget *dw)
	{
		auto toggleAct = dw->toggleViewAction ();
		disconnect (dw,
				SIGNAL (dockLocationChanged (Qt::DockWidgetArea)),
				this,
				SLOT (updateDockLocation (Qt::DockWidgetArea)));
		disconnect (toggleAct,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleActionToggled (bool)));

		HandleDockDestroyed (dw, toggleAct);
	}

	/* Both dw and act can be already dead and gone here.
	 */
	void DockToolbarManager::HandleDockDestroyed (QDockWidget *dw, QAction *act)
	{
		IHookProxy_ptr proxy (new Util::DefaultHookProxy);
		for (auto& info : Area2Info_)
		{
			if (info.Bar_->actions ().contains (act))
			{
				emit hookRemovingDockAction (proxy, Win_, act, info.Area_);
				info.Bar_->removeAction (act);
			}

			const auto& remaining = info.Bar_->actions ();
			if (remaining.size () < 2)
				info.Bar_->hide ();

			if (!remaining.isEmpty ())
				QTimer::singleShot (0,
						Action2Widget_ [remaining.last ()],
						SLOT (show ()));
		}

		Action2Widget_.remove (act);
	}

	void DockToolbarManager::UpdateActionGroup (QAction *action, bool enabled)
	{
		if (!enabled)
			return;

		const auto& assocs = action->associatedWidgets ();

		for (auto& info : Area2Info_)
		{
			if (!assocs.contains (info.Bar_))
				continue;

			for (auto otherAct : info.Bar_->actions ())
				if (otherAct != action && otherAct->isChecked ())
				{
					auto dw = Action2Widget_ [otherAct];
					dw->hide ();
				}
		}
	}

	void DockToolbarManager::updateDockLocation (Qt::DockWidgetArea area)
	{
		auto dw = qobject_cast<QDockWidget*> (sender ());
		RemoveDock (dw);
		AddDock (dw, area);
	}

	void DockToolbarManager::handleActionToggled (bool enabled)
	{
		UpdateActionGroup (qobject_cast<QAction*> (sender ()), enabled);
	}
}
