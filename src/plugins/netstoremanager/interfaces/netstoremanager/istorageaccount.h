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

#ifndef PLUGINS_NETSTOREMANAGER_INTERFACES_NETSTOREMANAGER_ISTORAGEACCOUNT_H
#define PLUGINS_NETSTOREMANAGER_INTERFACES_NETSTOREMANAGER_ISTORAGEACCOUNT_H
#include <QString>
#include <QtPlugin>

namespace LeechCraft
{
namespace NetStoreManager
{
	enum AccountFeature
	{
		None = 0x00,
		FileListings = 0x01,
		ProlongateFiles = 0x02
	};

	Q_DECLARE_FLAGS (AccountFeatures, AccountFeature);

	class IStorageAccount
	{
	public:
		virtual ~IStorageAccount () {}

		virtual QObject* GetParentPlugin () const = 0;

		virtual QString GetAccountName () const = 0;
		virtual AccountFeatures GetAccountFeatures () const = 0;

		virtual void Upload (const QString& filepath) = 0;
	};
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LeechCraft::NetStoreManager::AccountFeatures);

Q_DECLARE_INTERFACE (LeechCraft::NetStoreManager::IStorageAccount,
		"org.Deviant.LeechCraft.NetStoreManager.IStorageAccount/1.0");

#endif
