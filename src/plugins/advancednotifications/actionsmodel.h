/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>
#include <util/models/rolenamesmixin.h>

class QAction;

namespace LC::AdvancedNotifications
{
	class ActionsModel : public Util::RoleNamesMixin<QStandardItemModel>
	{
		Q_OBJECT

		QList<QAction*> Actions_;
	public:
		enum Roles
		{
			IconName = Qt::UserRole + 1,
			IsActionChecked
		};

		explicit ActionsModel (QObject*);

		void AddAction (QAction*);
	public slots:
		void triggerAction (int);
	};
}
