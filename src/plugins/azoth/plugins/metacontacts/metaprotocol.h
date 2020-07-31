/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/iprotocol.h>

namespace LC
{
namespace Azoth
{
namespace Metacontacts
{
	class MetaAccount;

	class MetaProtocol : public QObject
					   , public IProtocol
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IProtocol)

		QObject *ParentPlugin_;
		MetaAccount *Account_;
	public:
		MetaProtocol (QObject*);
		~MetaProtocol ();

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
	signals:
		void accountAdded (QObject*);
		void accountRemoved (QObject*);
	};
}
}
}
