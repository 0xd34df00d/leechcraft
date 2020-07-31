/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_todotab.h"

namespace LC
{
struct Entity;

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
		void handleAddChildTodoRequested ();
		void handleRemoveTodoRequested ();
		void handleCloneTodoRequested ();

		void handleEditCommentRequested ();
		void handleSetDueDateRequested ();
		void handleSetCustomDueDateRequested ();
		void handleQuickProgress ();

		void handleImport ();
		void handleExport ();
	signals:
		void removeTab (QWidget*);
		void gotEntity (const LC::Entity&);
	};
}
}
