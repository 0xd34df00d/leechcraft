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
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace Vader
{
	class MRIMAccount;

	class MRIMProtocol final : public QObject
							 , public IProtocol
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IProtocol)

		IProxyObject * const AzothProxy_;
		const ICoreProxy_ptr CoreProxy_;

		QList<MRIMAccount*> Accounts_;
	public:
		MRIMProtocol (IProxyObject*, const ICoreProxy_ptr&, QObject* = nullptr);
		~MRIMProtocol ();

		IProxyObject* GetAzothProxy () const;
		const ICoreProxy_ptr& GetCoreProxy () const;

		QObject* GetQObject () override;
		ProtocolFeatures GetFeatures () const override;
		QList<QObject*> GetRegisteredAccounts () override;
		QObject* GetParentProtocolPlugin () const override;
		QString GetProtocolName () const override;
		QIcon GetProtocolIcon () const override;
		QByteArray GetProtocolID () const override;
		QList<QWidget*> GetAccountRegistrationWidgets (AccountAddOptions) override;
		void RegisterAccount (const QString&, const QList<QWidget*>&) override;
		void RemoveAccount (QObject*) override;
	private:
		void RestoreAccounts ();
	private slots:
		void saveAccounts ();
	signals:
		void accountAdded (QObject*) override;
		void accountRemoved (QObject*) override;
	};
}
}
}
