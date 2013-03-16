/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <QObject>
#include <plugin.h>
#include <account.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/azoth/iprotocol.h>

namespace LeechCraft
{
struct Entity;

namespace Azoth
{
namespace VelvetBird
{
	class Account;

	class Protocol : public QObject
				   , public IProtocol
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IProtocol)

		ICoreProxy_ptr Proxy_;
		PurplePlugin *PPlug_;

		QList<Account*> Accounts_;
	public:
		Protocol (PurplePlugin*, ICoreProxy_ptr, QObject* = 0);

		void Release ();

		QObject* GetQObject ();
		ProtocolFeatures GetFeatures () const;
		QList<QObject*> GetRegisteredAccounts ();
		QObject* GetParentProtocolPlugin () const;
		QString GetProtocolName () const;
		QIcon GetProtocolIcon () const;
		QByteArray GetProtocolID () const;
		QList<QWidget*> GetAccountRegistrationWidgets (AccountAddOptions);
		void RegisterAccount (const QString&, const QList<QWidget*>&);
		QWidget* GetMUCJoinWidget ();
		void RemoveAccount (QObject*);

		QByteArray GetPurpleID () const;
		void PushAccount (PurpleAccount*);

		ICoreProxy_ptr GetCoreProxy () const;
	signals:
		void accountAdded (QObject*);
		void accountRemoved (QObject*);

		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	};
}
}
}
