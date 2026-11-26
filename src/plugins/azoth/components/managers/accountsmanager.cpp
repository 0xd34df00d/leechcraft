/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountsmanager.h"
#include <QIODevice>
#include <util/sll/qobjectrefcast.h>
#include <util/sys/serialization.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/icanhavesslerrors.h"
#include "interfaces/azoth/iprotocol.h"
#include "sslerrorshandler.h"
#include "xmlsettingsmanager.h"

namespace LC::Azoth
{
	void AccountsManager::AddProtocol (IProtocol& proto)
	{
		for (const auto accObj : proto.GetRegisteredAccounts ())
			addAccount (accObj);

		connect (proto.GetQObject (),
				SIGNAL (accountAdded (QObject*)),
				this,
				SLOT (addAccount (QObject*)));
		connect (proto.GetQObject (),
				SIGNAL (accountRemoved (QObject*)),
				this,
				SLOT (removeAccount (QObject*)));
	}

	void AccountsManager::OnEachAccount (const QObject& context, std::copyable_function<void (IAccount&)> action) const
	{
		connect (this,
				&AccountsManager::accountAdded,
				&context,
				action);

		for (const auto acc : auto { Accounts_ })
			action (*acc);
	}

	QList<IAccount*> AccountsManager::GetAccounts () const
	{
		return Accounts_;
	}

	namespace
	{
		QDataStream& operator<< (QDataStream& out, const EntryStatus& status)
		{
			constexpr quint8 version = 1;
			out << version
				<< static_cast<quint8> (status.State_)
				<< status.StatusString_;
			return out;
		}

		QDataStream& operator>> (QDataStream& in, EntryStatus& status)
		{
			if (!Util::EnsureVersion<quint8> (in, 1, 1))
				return in;

			quint8 state;
			in >> state
				>> status.StatusString_;
			status.State_ = static_cast<State> (state);
			return in;
		}

		std::optional<EntryStatus> LoadSavedStatus (IAccount& account)
		{
			const auto& var = XmlSettingsManager::Instance ().property (account.GetAccountId ().ToUtf8 ());
			if (var.isNull () || !var.canConvert<QByteArray> ())
				return {};

			EntryStatus s;
			QDataStream stream { var.toByteArray () };
			stream >> s;

			if (s.State_ == SConnecting)
				s.State_ = SOnline;

			return s;
		}

		void SaveStatus (const IAccount& account, const EntryStatus& status)
		{
			QByteArray serializedStatus;
			QDataStream stream { &serializedStatus, QIODevice::WriteOnly };
			stream << status;
			XmlSettingsManager::Instance ().setProperty (account.GetAccountId ().ToUtf8 (), serializedStatus);
		}
	}

	void AccountsManager::addAccount (QObject* accObj)
	{
		auto& acc = qobject_ref_cast<IAccount> (accObj);
		Accounts_ << &acc;

		connect (&acc.GetAccountEmitter (),
				&Emitters::Account::statusChanged,
				this,
				std::bind_front (SaveStatus, std::ref (acc)));

		emit accountAdded (acc);

		if (const auto& s = LoadSavedStatus (acc))
			acc.ChangeState (*s);

		const auto& accountId = acc.GetAccountId ();
		const auto& accountName = acc.GetAccountName ();
		if (const auto ichse = qobject_cast<ICanHaveSslErrors*> (acc.GetQObject ()))
			new SslErrorsHandler { SslErrorsHandler::Account { accountId, accountName }, ichse };

		connect (accObj,
				&QObject::destroyed,
				this,
				[this, &acc]
				{
					if (Accounts_.contains (&acc))
					{
						qWarning () << "removing a dead account without `accountRemoved` signal";
						RemoveAccount (acc);
					}
				});
	}

	void AccountsManager::RemoveAccount (IAccount& acc)
	{
		if (Accounts_.removeOne (&acc))
			emit accountRemoved (acc);
	}

	void AccountsManager::removeAccount (QObject* accObj)
	{
		RemoveAccount (qobject_ref_cast<IAccount> (accObj));
	}
}
