/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/imucprotocol.h>
#include <interfaces/azoth/iurihandler.h>

namespace LC
{
	struct Entity;
}

namespace LC::Azoth
{
	class IProxyObject;
}

namespace LC::Azoth::Acetamide
{
	class IrcAccount;
	class NickServIdentifyManager;

	class IrcProtocol final : public QObject
							, public IProtocol
							, public IMUCProtocol
							, public IURIHandler
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IProtocol
				LC::Azoth::IMUCProtocol
				LC::Azoth::IURIHandler)

		QObject *ParentProtocolPlugin_;
		QList<IrcAccount*> IrcAccounts_;
		const NickServIdentifyManager& IdentifyManager_;
		IProxyObject *ProxyObject_ = nullptr;
	public:
		explicit IrcProtocol (const NickServIdentifyManager&, QObject*);
		~IrcProtocol () override;

		const NickServIdentifyManager& GetNickServIdentifyManager () const;

		void Prepare ();
		IProxyObject* GetProxyObject () const;
		void SetProxyObject (IProxyObject*);

		QObject* GetQObject () override;
		ProtocolFeatures GetFeatures () const override;
		QList<QObject*> GetRegisteredAccounts () override;

		QObject* GetParentProtocolPlugin () const override;
		QString GetProtocolName () const override;
		QIcon GetProtocolIcon () const override;
		QByteArray GetProtocolID () const override;
		QList<QWidget*> GetAccountRegistrationWidgets (AccountAddOptions) override;
		void RegisterAccount (const QString&, const QList<QWidget*>&) override;
		QWidget* GetMUCJoinWidget () override;
		void RemoveAccount (QObject*) override;

		void HandleURI (const QUrl&, QObject*) override;
		bool SupportsURI (const QUrl&) const override;
	private:
		void RestoreAccounts ();
	private slots:
		void saveAccounts () const;
	signals:
		void accountAdded (QObject*) override;
		void accountRemoved (QObject*) override;
	};
}
