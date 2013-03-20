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
#include "mainwindow.h"

namespace LeechCraft
{
	DockToolbarManager::DockToolbarManager (MainWindow *win)
	: QObject (win)
	, Win_ (win)
	{
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
	}

	void DockToolbarManager::AddDock (QDockWidget *dw, Qt::DockWidgetArea area)
	{
		const auto& areaInfo = Area2Info_ [area];

		auto bar = areaInfo.Bar_;
		if (!bar)
			return;

		auto toggleAct = dw->toggleViewAction ();

		bar->addAction (toggleAct);
		if (!bar->isVisible ())
			bar->show ();

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
			UpdateActionGroup (toggleAct, true);
	}

	void DockToolbarManager::RemoveDock (QDockWidget *dw)
	{
		HandleDockDestroyed (dw, dw->toggleViewAction ());
	}

	/* Both dw and act can be already dead and gone here.
	 */
	void DockToolbarManager::HandleDockDestroyed (QDockWidget *dw, QAction *act)
	{
		for (auto& info : Area2Info_)
		{
			info.DockOrder_.removeAll (dw);
			if (info.Bar_->actions ().contains (act))
				info.Bar_->removeAction (act);

			if (info.Bar_->actions ().isEmpty ())
				info.Bar_->hide ();
			else if (!info.DockOrder_.isEmpty ())
				QTimer::singleShot (0,
						info.DockOrder_.last (),
						SLOT (show ()));
		}

		Action2Widget_.remove (act);
	}

	void DockToolbarManager::UpdateActionGroup (QAction *action, bool enabled)
	{
		const auto& assocs = action->associatedWidgets ();

		for (auto& info : Area2Info_)
		{
			if (!assocs.contains (info.Bar_))
				continue;

			if (!enabled)
			{
				info.DockOrder_.removeAll (Action2Widget_.value (action));
				continue;
			}

			for (auto otherAct : info.Bar_->actions ())
				if (otherAct != action && otherAct->isChecked ())
				{
					auto dw = Action2Widget_ [otherAct];
					dw->hide ();

					info.DockOrder_.removeAll (dw);
					info.DockOrder_ << dw;
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
