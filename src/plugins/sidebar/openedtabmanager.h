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

#pragma once

#include <QObject>
#include <QMap>
#include <QIcon>
#include <interfaces/core/icoreproxy.h>

namespace LeechCraft
{
namespace Sidebar
{
	class SBWidget;

	class OpenedTabManager : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		SBWidget *Bar_;

		QMap<QWidget*, QAction*> TabActions_;
		bool ActionUpdateScheduled_;
		QMap<QAction*, QString> ActionTextUpdates_;
		QMap<QAction*, QIcon> ActionIconUpdates_;
	public:
		OpenedTabManager (SBWidget*, ICoreProxy_ptr, QObject* = 0);

		void ManagePlugin (QObject*);
	private:
		void ScheduleUpdate ();
	private slots:
		void handleNewTab (const QString&, QWidget*);
		void handleChangeTabName (QWidget*, const QString&);
		void handleChangeTabIcon (QWidget*, const QIcon&);
		void handleRemoveTab (QWidget*);
		void handleUpdates ();
		void handleSelectTab ();
	};
}
}
