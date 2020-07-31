/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "toxprotocol.h"
#include <QIcon>
#include <QSettings>
#include <QCoreApplication>
#include <QtDebug>
#include "accregisterdetailspage.h"
#include "toxaccount.h"

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	ToxProtocol::ToxProtocol (const ICoreProxy_ptr& proxy, QObject* parentPlugin)
	: QObject { parentPlugin }
	, CoreProxy_ { proxy }
	, ParentProtocol_ { parentPlugin }
	{
		qRegisterMetaType<int32_t> ("int32_t");
		qRegisterMetaType<uint8_t> ("uint8_t");
		qRegisterMetaType<uint16_t> ("uint16_t");
		qRegisterMetaType<uint32_t> ("uint32_t");
		qRegisterMetaType<uint64_t> ("uint64_t");

		LoadAccounts ();
	}

	ToxProtocol::~ToxProtocol ()
	{
		for (auto acc : Accounts_)
			emit accountRemoved (acc);
	}

	QObject* ToxProtocol::GetQObject ()
	{
		return this;
	}

	IProtocol::ProtocolFeatures ToxProtocol::GetFeatures () const
	{
		return PFNone;
	}

	QList<QObject*> ToxProtocol::GetRegisteredAccounts ()
	{
		QList<QObject*> result;
		for (auto acc : Accounts_)
			result << acc;
		return result;
	}

	QObject* ToxProtocol::GetParentProtocolPlugin () const
	{
		return ParentProtocol_;
	}

	QString ToxProtocol::GetProtocolName () const
	{
		return "Tox";
	}

	QIcon ToxProtocol::GetProtocolIcon () const
	{
		return {};
	}

	QByteArray ToxProtocol::GetProtocolID () const
	{
		return "org.LeechCraft.Azoth.Tox";
	}

	QList<QWidget*> ToxProtocol::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions)
	{
		return { new AccRegisterDetailsPage };
	}

	void ToxProtocol::RegisterAccount (const QString& name, const QList<QWidget*>& widgets)
	{
		const auto detailsPage = qobject_cast<AccRegisterDetailsPage*> (widgets.value (0));

		auto acc = new ToxAccount { name, this };
		acc->SetNickname (detailsPage->GetNickname ());
		SaveAccount (acc);

		Accounts_ << acc;
		emit accountAdded (acc);

		InitConnections (acc);
	}

	void ToxProtocol::RemoveAccount (QObject *accObj)
	{
		const auto account = qobject_cast<ToxAccount*> (accObj);
		if (!Accounts_.contains (account))
			return;

		QSettings settings { QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Sarin_Accounts" };
		settings.remove (account->GetAccountID ());

		Accounts_.removeOne (account);
		emit accountRemoved (accObj);
	}

	const ICoreProxy_ptr& ToxProtocol::GetCoreProxy () const
	{
		return CoreProxy_;
	}

	void ToxProtocol::LoadAccounts ()
	{
		QSettings settings { QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Sarin_Accounts" };

		for (const auto& key : settings.childKeys ())
		{
			const auto& serialized = settings.value (key).toByteArray ();
			const auto acc = ToxAccount::Deserialize (serialized, this);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot deserialize"
						<< key;
				continue;
			}

			Accounts_ << acc;
			InitConnections (acc);
		}
	}

	void ToxProtocol::InitConnections (ToxAccount *acc)
	{
		connect (acc,
				&ToxAccount::accountChanged,
				this,
				&ToxProtocol::SaveAccount);
	}

	void ToxProtocol::SaveAccount (ToxAccount *account)
	{
		QSettings settings { QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Sarin_Accounts" };
		settings.setValue (account->GetAccountID (), account->Serialize ());
	}
}
}
}
