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
			UpdateActionGroup (toggleAct);
	}

	void DockToolbarManager::RemoveDock (QDockWidget *dw)
	{
		for (const auto& info : Area2Info_)
		{
			info.Bar_->removeAction (dw->toggleViewAction ());
			if (info.Bar_->actions ().isEmpty ())
				info.Bar_->hide ();
		}

		Action2Widget_.remove (Action2Widget_.key (dw));
	}

	void DockToolbarManager::UpdateActionGroup (QAction *action)
	{
		const auto& assocs = action->associatedWidgets ();

		for (const auto& info : Area2Info_)
		{
			if (!assocs.contains (info.Bar_))
				continue;

			for (auto otherAct : info.Bar_->actions ())
				if (otherAct != action && otherAct->isChecked ())
					Action2Widget_ [otherAct]->hide ();
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
		if (!enabled)
			return;

		UpdateActionGroup (qobject_cast<QAction*> (sender ()));
	}
}
