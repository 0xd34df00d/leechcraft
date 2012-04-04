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

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_todotab.h"

namespace LeechCraft
{
namespace Otlozhu
{
	class TodoSFProxyModel;

	class TodoTab : public QWidget
				  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::TodoTab Ui_;
		const TabClassInfo TC_;
		QObject *Plugin_;

		TodoSFProxyModel *ProxyModel_;

		QMenu *ProgressMenu_;
		QMenu *DueDateMenu_;
		QToolBar *Bar_;
	public:
		TodoTab (const TabClassInfo&, QObject*);
		~TodoTab ();

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private slots:
		void handleAddTodoRequested ();
		void handleRemoveTodoRequested ();
		void handleEditCommentRequested ();
		void handleSetDueDateRequested ();
		void handleSetCustomDueDateRequested ();
		void handleQuickProgress ();
	signals:
		void removeTab (QWidget*);
	};
}
}
