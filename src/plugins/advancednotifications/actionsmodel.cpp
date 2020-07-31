/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "actionsmodel.h"
#include <QAction>
#include <QtDebug>

namespace LC
{
namespace AdvancedNotifications
{
	ActionsModel::ActionsModel (QObject *parent)
	: RoleNamesMixin<QStandardItemModel> (parent)
	{
		QHash<int, QByteArray> roleNames;
		roleNames [Roles::IconName] = "iconName";
		roleNames [Roles::IsActionChecked] = "isActionChecked";
		setRoleNames (roleNames);
	}

	namespace
	{
		QString ChooseIcon (QAction *action, bool checked)
		{
			const auto& on = action->property ("ActionIcon").toString ();
			if (checked)
				return on;

			const auto& off = action->property ("ActionIconOff").toString ();
			return !off.isEmpty () ? off : on;
		}
	}

	void ActionsModel::AddAction (QAction *action)
	{
		Actions_ << action;

		auto item = new QStandardItem;
		item->setData (ChooseIcon (action, action->isChecked ()), Roles::IconName);
		item->setData (action->isChecked (), Roles::IsActionChecked);
		appendRow (item);

		connect (action,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleActionToggled (bool)));
	}

	void ActionsModel::triggerAction (int index)
	{
		Actions_.at (index)->trigger ();
	}

	void ActionsModel::handleActionToggled (bool checked)
	{
		auto action = static_cast<QAction*> (sender ());
		const auto pos = Actions_.indexOf (action);
		if (pos == -1)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "not found in"
					<< Actions_;
			return;
		}

		item (pos)->setData (ChooseIcon (action, checked), Roles::IconName);
		item (pos)->setData (checked, Roles::IsActionChecked);
	}
}
}
