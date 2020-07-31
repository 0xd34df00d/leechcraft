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
namespace Sarin
{
	class ToxAccount;

	class ToxProtocol final : public QObject
							, public IProtocol
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IProtocol)

		const ICoreProxy_ptr CoreProxy_;

		QObject * const ParentProtocol_;

		QList<ToxAccount*> Accounts_;
	public:
		ToxProtocol (const ICoreProxy_ptr&, QObject *parentPlugin);
		~ToxProtocol ();

		QObject* GetQObject () override;
		ProtocolFeatures GetFeatures () const override;
		QList<QObject*> GetRegisteredAccounts () override;
		QObject* GetParentProtocolPlugin () const override;

		QString GetProtocolName () const override;
		QIcon GetProtocolIcon () const override;
		QByteArray GetProtocolID () const override;

		QList<QWidget*> GetAccountRegistrationWidgets (AccountAddOptions options) override;
		void RegisterAccount (const QString& name, const QList<QWidget*>& widgets) override;
		void RemoveAccount (QObject* account) override;

		const ICoreProxy_ptr& GetCoreProxy () const;
	private:
		void LoadAccounts ();
		void InitConnections (ToxAccount*);
		void SaveAccount (ToxAccount*);
	signals:
		void accountAdded (QObject*) override;
		void accountRemoved (QObject*) override;
	};
}
}
}
