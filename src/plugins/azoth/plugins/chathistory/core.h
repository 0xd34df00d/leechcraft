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

#ifndef PLUGINS_AZOTH_PLUGINS_CHATHISTORY_CORE_H
#define PLUGINS_AZOTH_PLUGINS_CHATHISTORY_CORE_H
#include <memory>
#include <QObject>
#include <QSet>
#include <QVariantMap>
#include <interfaces/ihavetabs.h>

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
		std::shared_ptr<T> C_;
	public:
		STGuard ()
		: C_ (T::Instance ())
		{}
	};

	class StorageThread;

	class Core : public QObject
	{
		Q_OBJECT
		static std::weak_ptr<Core> InstPtr_;

		StorageThread *StorageThread_;
		IProxyObject *PluginProxy_;
		QSet<QString> DisabledIDs_;

		TabClassInfo TabClass_;

		Core ();
	public:
		static std::shared_ptr<Core> Instance ();

		~Core ();

		TabClassInfo GetTabClass () const;

		void SetPluginProxy (QObject*);
		IProxyObject* GetPluginProxy () const;

		bool IsLoggingEnabled (QObject*) const;
		void SetLoggingEnabled (QObject*, bool);

		void Process (QObject*);
		void Process (QVariantMap);
		void GetOurAccounts ();
		void GetUsersForAccount (const QString&);
		void GetChatLogs (const QString& accountId, const QString& entryId,
				int backpages, int amount);
		void Search (const QString& accountId, const QString& entryId,
				const QString& text, int shift);
		void ClearHistory (const QString& accountId, const QString& entryId);
	private:
		void LoadDisabled ();
		void SaveDisabled ();
	signals:
		void gotOurAccounts (const QStringList&);
		void gotUsersForAccount (const QStringList&, const QString&, const QStringList&);

		/** The variant is a list of QVariantMaps.
		 */
		void gotChatLogs (const QString&, const QString&, int, int, const QVariant&);
		void gotSearchPosition (const QString&, const QString&, int);
	};
}
}
}

#endif
