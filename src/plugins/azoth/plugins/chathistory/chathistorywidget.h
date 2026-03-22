/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <util/models/itemsmodel.h>
#include <interfaces/ihavetabs.h>
#include "chatfindbox.h"
#include "storage2.h"
#include "ui_chathistorywidget.h"

class QSortFilterProxyModel;

namespace LC::Azoth
{
	class ICLEntry;
	class IProxyObject;
}

namespace LC::Azoth::ChatHistory
{
	class ChatFindBox;
	class StorageThread;

	class ChatHistoryWidget : public QWidget
							, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::ChatHistoryWidget Ui_;
	public:
		struct InitParams
		{
			StorageThread& StorageThread_;
			IProxyObject * const PluginProxy_;
			QObject * const ParentMultiTabs_;
			const TabClassInfo Info_;
		};
	private:
		InitParams Params_;

		const uint16_t PerPageAmount_;

		struct DisplayedMessagesSpan
		{
			qint64 FirstId_;
			qint64 LastId_;
		};
		std::optional<DisplayedMessagesSpan> DisplayedSpan_;

		QString PreviousSearchText_;
		std::optional<qint64> LastSearchCursor_;

		Util::RoledItemsModel<AccountInfo> AccountsModel_;

		struct DisplayEntry
		{
			qint64 Id_;
			Util::RoleOf<QString, Qt::DisplayRole> Name_;
			Entry Base_;

			bool IsMuc () const
			{
				return History::GetEntryKind (Base_.EntryInfo_) == History::EntryKind::MUC;
			}
		};
		Util::RoledItemsModel<DisplayEntry> EntriesModel_;

		QSortFilterProxyModel *SortFilter_;
		QToolBar *Toolbar_;

		struct FocusEntry
		{
			QByteArray AccId_;
			QString EntryId_;
		};
		std::optional<FocusEntry> FocusEntry_;

		ChatFindBox *FindBox_;
	public:
		ChatHistoryWidget (const InitParams&, ICLEntry* = 0, QWidget* = 0);

		void Remove () override;
		QToolBar* GetToolBar () const override;
		QObject* ParentMultiTabs () override;
		TabClassInfo GetTabClassInfo () const override;
		QList<QAction*> GetTabBarContextMenuActions () const override;
	private:
		std::optional<AccountInfo> GetCurrentAccount () const;
		std::optional<DisplayEntry> GetCurrentEntry () const;

		Util::ContextTask<void> LoadAccounts ();
		Util::ContextTask<void> LoadAccountEntries (int);
		void HandleEntrySelected (const QModelIndex&);
		Util::ContextTask<void> RequestLogs (const Storage2::Pagination&);
		Util::ContextTask<void> RequestLogsForDate (const QDate&);

		Util::ContextTask<void> UpdateDates ();

		Util::ContextTask<void> HandleSearch (const QString& text, ChatFindBox::FindFlags flags);

		struct EntryChanged {};
		Util::Either<EntryChanged, Util::Void> GuardEntryChanged (qint64 entryId) const;

		void ShowLoading ();
		void RenderMessages (const QList<Storage2::HistoryMessage>&);

		void ClearHistory ();
	signals:
		void removeTab () override;
	};
}
