/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include "interfaces/azoth/azothcommon.h"

class QAction;
class QMenu;

namespace LC
{
namespace Azoth
{
	class IAccount;
	class ConsoleWidget;
	class ServiceDiscoveryWidget;
	class MicroblogsTab;
	class ServerHistoryWidget;

	class AccountActionsManager : public QObject
	{
		Q_OBJECT

		QHash<IAccount*, ConsoleWidget*> Account2CW_;

		QMenu *MenuChangeStatus_;

		QAction *AccountJoinConference_;
		QAction *AccountManageBookmarks_;
		QAction *AccountAddContact_;
		QAction *AccountOpenNonRosterChat_;
		QAction *AccountOpenServerHistory_;
		QAction *AccountConfigServerHistory_;
		QAction *AccountViewMicroblogs_;
		QAction *AccountSetActivity_;
		QAction *AccountSetMood_;
		QAction *AccountSetLocation_;
		QAction *AccountSD_;
		QAction *AccountConsole_;
		QAction *AccountUpdatePassword_;
		QAction *AccountRename_;
		QAction *AccountModify_;
		QAction *AccountRemove_;
	public:
		AccountActionsManager (QObject* = nullptr);

		QList<QAction*> GetMenuActions (QMenu*, IAccount*);
	private:
		QList<QAction*> AddBMActions (QMenu*, QObject*);

		void ChangeStatus (State, const QString&);
	private slots:
		void joinAccountConference ();
		void joinAccountConfFromBM ();
		void manageAccountBookmarks ();
		void addAccountContact ();
		void handleOpenNonRoster ();
		void handleOpenServerHistory ();
		void handleConfigServerHistory ();
		void handleAccountMicroblogs ();
		void handleAccountSetActivity ();
		void handleAccountSetMood ();
		void handleAccountSetLocation ();
		void handleAccountSD ();
		void handleAccountConsole ();
		void handleUpdatePassword ();
		void handleAccountRename ();
		void handleAccountModify ();
		void handleAccountRemove ();
	};
}
}
