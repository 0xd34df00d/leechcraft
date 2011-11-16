/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_SNAILS_CORE_H
#define PLUGINS_SNAILS_CORE_H
#include <QObject>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include "account.h"

class QAbstractItemModel;
class QStandardItemModel;
class QModelIndex;

namespace LeechCraft
{
namespace Util
{
	class ResourceLoader;
}

namespace Snails
{
	class Storage;
	class ProgressManager;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;

		QStandardItemModel *AccountsModel_;
		QList<Account_ptr> Accounts_;

		Storage *Storage_;
		ProgressManager *ProgressManager_;

		std::shared_ptr<Util::ResourceLoader> MsgView_;

		Core ();
	public:
		static Core& Instance ();
		void Release ();

		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;

		void SendEntity (const Entity&);

		QAbstractItemModel* GetAccountsModel () const;

		QList<Account_ptr> GetAccounts () const;
		Account_ptr GetAccount (const QModelIndex&) const;

		Storage* GetStorage () const;
		ProgressManager* GetProgressManager () const;
		QString GetMsgViewTemplate () const;

		void AddAccount (Account_ptr);
	private:
		void AddAccountImpl (Account_ptr);
		void LoadAccounts ();
	private slots:
		void saveAccounts () const;
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	};
}
}

#endif
