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
#include "chatfindbox.h"
#include "storagestructures.h"
#include "ui_chathistorywidget.h"

class QStandardItemModel;
class QStandardItem;
class QSortFilterProxyModel;

namespace LC
{
struct Entity;

namespace Azoth
{
class ICLEntry;
class IProxyObject;

namespace ChatHistory
{
	class Plugin;
	class ChatFindBox;
	class StorageManager;

	class ChatHistoryWidget : public QWidget
							, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::ChatHistoryWidget Ui_;
	public:
		struct InitParams
		{
			StorageManager * const StorageMgr_;
			IProxyObject * const PluginProxy_;
			const ICoreProxy_ptr CoreProxy_;
			Plugin * const ParentMultiTabs_;
			const TabClassInfo Info_;
		};
	private:
		InitParams Params_;

		const int PerPageAmount_;

		QStandardItemModel *ContactsModel_;
		QSortFilterProxyModel *SortFilter_;
		int Backpages_ = 0;
		int Amount_ = 0;
		int SearchShift_ = 0;
		int SearchResultPosition_ = -1;
		bool ContactSelectedAsGlobSearch_ = false;
		QString CurrentAccount_;
		QString CurrentEntry_;
		QString PreviousSearchText_;
		QToolBar *Toolbar_;

		QHash<QString, QString> EntryID2NameCache_;

		ICLEntry *EntryToFocus_;

		ChatFindBox *FindBox_;

		enum ModelRoles
		{
			MRIDRole = Qt::UserRole + 1
		};
	public:
		ChatHistoryWidget (const InitParams&, ICLEntry* = 0, QWidget* = 0);

		void Remove ();
		QToolBar* GetToolBar () const;
		QObject* ParentMultiTabs ();
		TabClassInfo GetTabClassInfo () const;
		QList<QAction*> GetTabBarContextMenuActions () const;
	private:
		void HandleGotOurAccounts (const QStringList&);
		void HandleGotUsersForAccount (const QString&, const UsersForAccountResult_t&);
		void HandleGotChatLogs (const QString&, const QString&, const ChatLogsResult_t&);
		void HandleGotSearchPosition (const QString&, const QString&, const SearchResult_t&);
		void HandleGotDaysForSheet (const QString&, const QString&, int, int, const DaysResult_t&);
	private slots:
		void on_AccountBox__currentIndexChanged (int);
		void handleContactSelected (const QModelIndex&);

		void on_Calendar__currentPageChanged ();
		void on_Calendar__activated (const QDate&);

		void handleNext (const QString&, ChatFindBox::FindFlags);
		void previousHistory ();
		void nextHistory ();
		void clearHistory ();

		void on_HistView__anchorClicked (const QUrl&);
		void handleBgLinkRequested (const QUrl&);
	private:
		QStandardItem* FindContactItem (const QString&) const;

		void ShowLoading ();
		void UpdateDates ();
		void RequestLogs ();
		void RequestSearch (ChatFindBox::FindFlags);
	signals:
		void removeTab ();
	};
}
}
}
