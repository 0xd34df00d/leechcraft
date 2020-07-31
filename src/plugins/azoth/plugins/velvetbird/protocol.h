/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/azoth/iprotocol.h>

typedef struct _PurpleAccount      PurpleAccount;
typedef struct _PurplePlugin           PurplePlugin;

namespace LC
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
		Q_INTERFACES (LC::Azoth::IProtocol)

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
		void RemoveAccount (QObject*);

		QByteArray GetPurpleID () const;
		void PushAccount (PurpleAccount*);

		ICoreProxy_ptr GetCoreProxy () const;
	signals:
		void accountAdded (QObject*);
		void accountRemoved (QObject*);
	};

	using Protocol_ptr = std::shared_ptr<Protocol>;
}
}
}
