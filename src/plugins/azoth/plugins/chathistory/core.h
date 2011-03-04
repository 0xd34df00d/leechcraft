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

#ifndef PLUGINS_AZOTH_PLUGINS_CHATHISTORY_CORE_H
#define PLUGINS_AZOTH_PLUGINS_CHATHISTORY_CORE_H
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <QObject>
#include <QSet>

namespace LeechCraft
{
namespace Azoth
{
class IMessage;
class IProxyObject;

namespace ChatHistory
{
	template<typename T>
	class STGuard
	{
		boost::shared_ptr<T> C_;
	public:
		STGuard ()
		: C_ (T::Instance ())
		{}
	};
	
	class StorageThread;

	class Core : public QObject
	{
		Q_OBJECT
		static boost::weak_ptr<Core> InstPtr_;
		
		StorageThread *StorageThread_;
		IProxyObject *PluginProxy_;
		
		Core ();
	public:
		static boost::shared_ptr<Core> Instance ();
		
		void SetPluginProxy (QObject*);
		IProxyObject* GetPluginProxy () const;
		
		void Process (QObject*);
		void GetOurAccounts ();
		void GetUsersForAccount (const QString&);
		void GetChatLogs (const QString& accountId, const QString& entryId,
				int backpages, int amount);
		void ClearHistory (const QString& accountId, const QString& entryId);
	signals:
		void gotOurAccounts (const QStringList&);
		void gotUsersForAccount (const QStringList&, const QString&);
		/** The variant is a list of QVariantMaps.
		 */
		void gotChatLogs (const QString&, const QString&, int, int, const QVariant&);
	};
}
}
}

#endif
