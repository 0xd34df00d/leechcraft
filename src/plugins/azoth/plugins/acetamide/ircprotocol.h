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

namespace Azoth
{
namespace Acetamide
{
	class IrcAccount;

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
		QObject *ProxyObject_ = nullptr;
	public:
		IrcProtocol (QObject* = nullptr);
		virtual ~IrcProtocol ();

		void Prepare ();
		QObject* GetProxyObject () const;
		void SetProxyObject (QObject*);

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

		void HandleURI (const QUrl&, QObject*);
		bool SupportsURI (const QUrl&) const;
	private:
		void RestoreAccounts ();
	private slots:
		void saveAccounts () const;
	signals:
		void accountAdded (QObject*);
		void accountRemoved (QObject*);
	};
};
};
};
