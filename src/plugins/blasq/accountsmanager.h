/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QModelIndex;
class QAbstractItemModel;
class QStandardItemModel;

namespace LC
{
namespace Blasq
{
	class ServicesManager;
	class IService;
	class IAccount;

	class AccountsManager : public QObject
	{
		Q_OBJECT

		ServicesManager * const SvcMgr_;
		QStandardItemModel * const Model_;
		QList<IAccount*> Accounts_;

	public:
		enum Column
		{
			Name,
			Service
		};
		enum Role
		{
			AccountObj = Qt::UserRole + 1,
			AccountId
		};

		AccountsManager (ServicesManager*, QObject* = 0);

		QAbstractItemModel* GetModel ();
		const QList<IAccount*>& GetAccounts () const;

		int GetAccountIndex (const QByteArray&) const;
		IAccount* GetAccount (const QByteArray&) const;

		void RemoveAccount (const QModelIndex&);
	private:
		void HandleAccount (IAccount*);
	private slots:
		void handleService (IService*);

		void handleAccountAdded (QObject*);
		void handleAccountRemoved (QObject*);
	};
}
}
