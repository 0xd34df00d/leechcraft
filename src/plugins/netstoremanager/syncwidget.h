/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_syncwidget.h"

class QStandardItemModel;

namespace LC
{
namespace NetStoreManager
{
	class AccountsManager;

	struct SyncerInfo
	{
		QByteArray AccountId_;
		QString LocalDirectory_;
		QString RemoteDirectory_;

		inline bool operator== (const SyncerInfo& si) const
		{
			return AccountId_ == si.AccountId_ &&
					LocalDirectory_ == si.LocalDirectory_ &&
					RemoteDirectory_ == si.RemoteDirectory_;
		}
	};

	class SyncWidget : public QWidget
	{
		Q_OBJECT

		Ui::SynchronizationWidget Ui_;

		AccountsManager *AM_;
		QStandardItemModel *Model_;

	public:
		SyncWidget (AccountsManager *am, QWidget *parent = 0);
		void RestoreData ();
	private:
		void RemoveInvalidRows ();
		void RemoveDuplicateRows ();
		QList<SyncerInfo> GetInfos () const;

	public slots:
		void accept ();
	private slots:
		void on_Add__released ();
		void on_Remove__released ();

	signals:
		void directoriesToSyncUpdated (const QList<SyncerInfo>& infos);
	};
}
}

Q_DECLARE_METATYPE (LC::NetStoreManager::SyncerInfo)
Q_DECLARE_METATYPE (QList<LC::NetStoreManager::SyncerInfo>)
