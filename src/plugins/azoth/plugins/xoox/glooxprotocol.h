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
#include <interfaces/azoth/imucprotocol.h>
#include <interfaces/azoth/iurihandler.h>
#include <interfaces/azoth/isupportimport.h>

namespace LC
{
struct Entity;

namespace Azoth
{
class IProxyObject;

namespace Xoox
{
	class GlooxAccount;
	class CapsDatabase;
	class VCardStorage;

	class GlooxProtocol final : public QObject
							  , public IProtocol
							  , public IMUCProtocol
							  , public IURIHandler
							  , public ISupportImport
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IProtocol
				LC::Azoth::IMUCProtocol
				LC::Azoth::IURIHandler
				LC::Azoth::ISupportImport)

		QObject *ParentProtocolPlugin_;
		QList<GlooxAccount*> Accounts_;

		CapsDatabase *CapsDB_;
		VCardStorage *VCardStorage_;
		IProxyObject *ProxyObject_ = nullptr;
	public:
		GlooxProtocol (CapsDatabase*, VCardStorage*, QObject* = nullptr);
		virtual ~GlooxProtocol ();

		void Prepare ();

		IProxyObject* GetProxyObject () const;
		void SetProxyObject (IProxyObject*);

		CapsDatabase* GetCapsDatabase () const;
		VCardStorage* GetVCardStorage () const;

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

		QWidget* GetMUCJoinWidget ();
		QVariantMap TryGuessMUCIdentifyingData (const QString&, QObject*);

		bool SupportsURI (const QUrl&) const;
		void HandleURI (const QUrl&, QObject*);

		QString GetImportProtocolID () const;
		bool ImportAccount (const QVariantMap&);
		QString GetEntryID (const QString&, QObject*);
	private:
		void RestoreAccounts ();
	private slots:
		void saveAccounts () const;
	signals:
		void accountAdded (QObject*);
		void accountRemoved (QObject*);
	};
}
}
}
