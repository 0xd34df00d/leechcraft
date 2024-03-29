/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ircprotocol.h"
#include <QCoreApplication>
#include <QInputDialog>
#include <QMainWindow>
#include <QSettings>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/iprotocolplugin.h>
#include "ircaccount.h"
#include "ircaccountconfigurationwidget.h"
#include "ircjoingroupchat.h"
#include "parsers.h"

namespace LC::Azoth::Acetamide
{
	IrcProtocol::IrcProtocol (const NickServIdentifyManager& nsim, QObject *parent)
	: QObject { parent }
	, ParentProtocolPlugin_ { parent }
	, IdentifyManager_ { nsim }
	{
	}

	IrcProtocol::~IrcProtocol ()
	{
		for (auto acc : GetRegisteredAccounts ())
			emit accountRemoved (acc);
	}

	const NickServIdentifyManager& IrcProtocol::GetNickServIdentifyManager () const
	{
		return IdentifyManager_;
	}

	void IrcProtocol::Prepare ()
	{
		RestoreAccounts ();
	}

	IProxyObject* IrcProtocol::GetProxyObject () const
	{
		return ProxyObject_;
	}

	void IrcProtocol::SetProxyObject (IProxyObject *po)
	{
		ProxyObject_ = po;
	}

	QObject* IrcProtocol::GetQObject ()
	{
		return this;
	}

	IProtocol::ProtocolFeatures IrcProtocol::GetFeatures () const
	{
		return PFMUCsJoinable | PFSupportsMUCs;
	}

	QList<QObject*> IrcProtocol::GetRegisteredAccounts ()
	{
		return Util::Map (IrcAccounts_, [] (auto obj) -> QObject* { return obj; });
	}

	QObject* IrcProtocol::GetParentProtocolPlugin () const
	{
		return ParentProtocolPlugin_;
	}

	QString IrcProtocol::GetProtocolName () const
	{
		return QStringLiteral ("IRC");
	}

	QIcon IrcProtocol::GetProtocolIcon () const
	{
		static const auto icon = GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon (QStringLiteral ("azoth/acetamide/ircicon"));
		return icon;
	}

	QByteArray IrcProtocol::GetProtocolID () const
	{
		return "Acetamide.IRC";
	}

	QList<QWidget*> IrcProtocol::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions)
	{
		return { new IrcAccountConfigurationWidget () };
	}

	void IrcProtocol::RegisterAccount (const QString& name,
			const QList<QWidget*>& widgets)
	{
		const auto w = dynamic_cast<IrcAccountConfigurationWidget*> (widgets.value (0));
		if (!w)
		{
			qWarning () << Q_FUNC_INFO
					<< "got invalid widgets"
					<< widgets;
			return;
		}

		const auto account = new IrcAccount (name, this);
		account->FillSettings (w);
		account->SetAccountID (GetProtocolID () + "." + QString::number (QDateTime::currentSecsSinceEpoch ()));
		IrcAccounts_ << account;
		saveAccounts ();
		emit accountAdded (account);
	}

	QWidget* IrcProtocol::GetMUCJoinWidget ()
	{
		return new IrcJoinGroupChat ();
	}

	void IrcProtocol::RemoveAccount (QObject *acc)
	{
		const auto accObj = qobject_cast<IrcAccount*> (acc);
		if (IrcAccounts_.removeAll (accObj))
		{
			emit accountRemoved (accObj);
			accObj->deleteLater ();
			saveAccounts ();
		}
	}

	void IrcProtocol::HandleURI (const QUrl& url, QObject *account)
	{
		const auto acc = qobject_cast<IrcAccount*> (account);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< account
					<< "isn't IrcAccount";
			return;
		}

		auto maybeDecoded = DecodeUrl (url);
		if (!maybeDecoded)
		{
			qWarning () << "input string is not a valid IRC URI"
					<< url.toString ().toUtf8 ().constData ();
			return;
		}

		auto& res = *maybeDecoded;
		if (const auto channel = std::get_if<ChannelTarget> (&res.Target_))
		{
			if (res.HasServerPassword_)
				res.Server_.ServerPassword_ = QInputDialog::getText (nullptr,
						Lits::AzothAcetamide,
						tr ("This server needs password. Please enter it here:"),
						QLineEdit::Password);

			if (channel->HasPassword_)
				channel->Opts_.ChannelPassword_ = QInputDialog::getText (nullptr,
						Lits::AzothAcetamide,
						tr ("This channel needs password. Please enter it here:"),
						QLineEdit::Password);

			//TODO nickServ for urls
			acc->JoinServer (res.Server_, channel->Opts_);
		}
	}

	bool IrcProtocol::SupportsURI (const QUrl& url) const
	{
		return url.scheme () == "irc"_ql;
	}

	namespace
	{
		const QString Accounts = QStringLiteral ("Accounts");
		const QString SerializedData = QStringLiteral ("SerializedData");
	}

	void IrcProtocol::saveAccounts () const
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
					"_Azoth_Acetamide_Accounts");

		settings.beginWriteArray (Accounts);

		for (int i = 0, size = IrcAccounts_.size (); i < size; ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue (SerializedData, IrcAccounts_.at (i)->Serialize ());
		}

		settings.endArray ();
	}

	void IrcProtocol::RestoreAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
					"_Azoth_Acetamide_Accounts");

		int size = settings.beginReadArray (Accounts);
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			const auto& data = settings.value (SerializedData).toByteArray ();

			const auto acc = IrcAccount::Deserialize (data, this);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "unserializable acount"
						<< i;
				continue;
			}

			connect (acc,
					&IrcAccount::accountSettingsChanged,
					this,
					&IrcProtocol::saveAccounts);

			IrcAccounts_ << acc;
			emit accountAdded (acc);
		}
	}
}
