/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_DELICIOUS_DELICIOUSACCOUNT_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_DELICIOUS_DELICIOUSACCOUNT_H

#include <QObject>
#include <interfaces/iaccount.h>

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace Delicious
{
	class DeliciousAccount : public QObject
							, public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Poshuku::OnlineBookmarks::IAccount)
	public:
		QString Login_;
		QString Password_;
		IAccount::AuthType AuthType_;
		QObject *ParentService_;
		bool IsSyncing_;
	public:
		DeliciousAccount (const QString&, QObject* = 0);
		QObject* GetObject ();
		QObject* GetParentService () const;
		QByteArray GetAccountID () const;
		QString GetLogin () const;
		QString GetPassword () const;
		void SetPassword (const QString&);
		IAccount::AuthType GetAuthType () const;
		QVariantMap GetIdentifyingData() const;
		bool IsSyncing () const;
		void SetSyncing (bool);
		QByteArray Serialize () const ;
		static DeliciousAccount* Deserialize (const QByteArray&, QObject *);
	};
}
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_DELICIOUS_DELICIOUSACCOUNT_H
