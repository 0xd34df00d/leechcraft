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

#include "actionsmodel.h"
#include <QAction>
#include <QtDebug>

namespace LeechCraft
{
namespace AdvancedNotifications
{
	ActionsModel::ActionsModel (QObject *parent)
	: QStandardItemModel (parent)
	{
		QHash<int, QByteArray> roleNames;
		roleNames [Roles::IconName] = "iconName";
		roleNames [Roles::IsActionChecked] = "isActionChecked";
		setRoleNames (roleNames);
	}

	void ActionsModel::AddAction (QAction *action)
	{
		Actions_ << action;

		auto item = new QStandardItem;
		item->setData (action->property ("ActionIcon"), Roles::IconName);
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
		const auto pos = Actions_.indexOf (static_cast<QAction*> (sender ()));
		if (pos == -1)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "not found in"
					<< Actions_;
			return;
		}

		item (pos)->setData (checked, Roles::IsActionChecked);
	}
}
}
