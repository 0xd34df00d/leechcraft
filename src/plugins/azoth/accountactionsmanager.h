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
#include <QHash>

class QAction;
class QMenu;

namespace LeechCraft
{
namespace Azoth
{
	class IAccount;
	class ConsoleWidget;
	class ServiceDiscoveryWidget;

	class AccountActionsManager : public QObject
	{
		Q_OBJECT

		QWidget *MW_;

		QHash<IAccount*, ConsoleWidget*> Account2CW_;

		QAction *AccountJoinConference_;
		QAction *AccountManageBookmarks_;
		QAction *AccountAddContact_;
		QAction *AccountSetActivity_;
		QAction *AccountSetMood_;
		QAction *AccountSetLocation_;
		QAction *AccountSD_;
		QAction *AccountConsole_;
		QAction *AccountRename_;
		QAction *AccountModify_;
	public:
		AccountActionsManager (QWidget*, QObject* = 0);

		QList<QAction*> GetMenuActions (QMenu*, QObject*);
	private slots:
		void joinAccountConference ();
		void joinAccountConfFromBM ();
		void manageAccountBookmarks ();
		void addAccountContact ();
		void handleAccountSetActivity ();
		void handleAccountSetMood ();
		void handleAccountSetLocation ();
		void handleAccountSD ();
		void handleAccountConsole ();
		void handleAccountRename ();
		void handleAccountModify ();

		void consoleRemoved (QWidget*);
	signals:
		void gotConsoleWidget (ConsoleWidget*);
		void gotSDWidget (ServiceDiscoveryWidget*);
	};
}
}
