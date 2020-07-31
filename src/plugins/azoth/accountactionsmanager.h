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
	class StatusChangeMenuManager;
	class IAccount;
	class ConsoleWidget;
	class ServiceDiscoveryWidget;
	class MicroblogsTab;
	class ServerHistoryWidget;

	class AccountActionsManager : public QObject
	{
		Q_OBJECT

		QWidget *MW_ = nullptr;

		QHash<IAccount*, ConsoleWidget*> Account2CW_;

		StatusChangeMenuManager * const StatusMenuMgr_;
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

		void SetMainWidget (QWidget*);
		QList<QAction*> GetMenuActions (QMenu*, IAccount*);

		QString GetStatusText (QAction*, State) const;
	private:
		QList<QAction*> AddMenuChangeStatus (QMenu*);
		QList<QAction*> AddBMActions (QMenu*, QObject*);
	private slots:
		void handleChangeStatusRequested ();
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

		void consoleRemoved (QWidget*);
	signals:
		void gotConsoleWidget (ConsoleWidget*);
		void gotSDWidget (ServiceDiscoveryWidget*);
		void gotMicroblogsTab (MicroblogsTab*);
		void gotServerHistoryTab (ServerHistoryWidget*);
	};
}
}
