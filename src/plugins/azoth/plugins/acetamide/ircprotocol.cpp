/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "ircprotocol.h"
#include <boost/bind.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include <QCoreApplication>
#include <QInputDialog>
#include <QMainWindow>
#include <QSettings>
#include <interfaces/iprotocolplugin.h>
#include "core.h"
#include "ircaccount.h"
#include "ircaccountconfigurationwidget.h"
#include "ircjoingroupchat.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	using namespace boost::spirit::classic;

	IrcProtocol::IrcProtocol (QObject *parent)
	: QObject (parent)
	, ParentProtocolPlugin_ (parent)
	, ProxyObject_ (0)
	{
	}

	IrcProtocol::~IrcProtocol ()
	{
		Q_FOREACH (QObject *acc, GetRegisteredAccounts ())
			emit accountRemoved (acc);
	}

	void IrcProtocol::Prepare ()
	{
		RestoreAccounts ();
	}

	QObject* IrcProtocol::GetProxyObject () const
	{
		return ProxyObject_;
	}

	void IrcProtocol::SetProxyObject (QObject *po)
	{
		ProxyObject_ = po;
	}

	QObject* IrcProtocol::GetObject ()
	{
		return this;
	}

	IProtocol::ProtocolFeatures IrcProtocol::GetFeatures () const
	{
		return PFMUCsJoinable | PFSupportsMUCs;
	}

	QList<QObject*> IrcProtocol::GetRegisteredAccounts ()
	{
		QList<QObject*> result;
		Q_FOREACH (IrcAccount *acc, IrcAccounts_)
			result << acc;
		return result;
	}

	QObject* IrcProtocol::GetParentProtocolPlugin () const
	{
		return ParentProtocolPlugin_;
	}

	QString IrcProtocol::GetProtocolName () const
	{
		return "IRC";
	}

	QIcon IrcProtocol::GetProtocolIcon () const
	{
		return QIcon (":/plugins/azoth/plugins/acetamide/resources/images/ircicon.svg");
	}

	QByteArray IrcProtocol::GetProtocolID () const
	{
		return "Acetamide.IRC";
	}

	QList<QWidget*> IrcProtocol::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions)
	{
		QList<QWidget*> result;
		result << new IrcAccountConfigurationWidget ();
		return result;
	}

	void IrcProtocol::RegisterAccount (const QString& name,
			const QList<QWidget*>& widgets)
	{
		IrcAccountConfigurationWidget *w =
				qobject_cast<IrcAccountConfigurationWidget*>
					(widgets.value (0));
		if (!w)
		{
			qWarning () << Q_FUNC_INFO
					<< "got invalid widgets"
					<< widgets;
			return;
		}

		IrcAccount *account = new IrcAccount (name, this);
		account->FillSettings (w);
		account->SetAccountID (GetProtocolID () + "." +
				QString::number (QDateTime::currentDateTime ()
					.toTime_t ()));
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
		IrcAccount *accObj = qobject_cast<IrcAccount*> (acc);
		if (IrcAccounts_.removeAll (accObj))
		{
			emit accountRemoved (accObj);
			accObj->deleteLater ();
			saveAccounts ();
		}
	}

	void IrcProtocol::HandleURI (const QUrl& url, QObject *account)
	{
		IrcAccount *acc = qobject_cast<IrcAccount*> (account);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< account
					<< "isn't IrcAccount";
			return;
		}

		std::string host_;
		int port_ = 0;
		std::string channel_;
		std::string nick_;
		bool isNick = false;
		bool serverPass = false;
		bool channelPass = false;

		range<> ascii (char (0x01), char (0x7F));
		rule<> special = lexeme_d [ch_p ('[') | ']' | '\\' | '`' |
				'^' | '{' | '|' | '}' | '-'];
		rule<> hostmask = lexeme_d [+(ascii - ' ' - '\0' - ',' - '\r' - '\n')];
		rule<> let_dig_hyp = alnum_p | ch_p ('-');
		rule<> ldh_str = *(let_dig_hyp >> !ldh_str);
		rule<> label = alpha_p >> !(!ldh_str >> alnum_p);
		rule<> subdomain = label >> +(label >> !ch_p ('.'));
		rule<> host = subdomain  [assign_a (host_)];
		rule<> servername = host;
		rule<> user = (ascii - ' ' - '\0' - '\r' - '\n') >>
				*(ascii - ' ' - '\0' - '\r' - '\n');
		rule<> nick = alpha_p >> *(alnum_p | special);
		rule<> userinfo = user >> ch_p ('@') >> servername;
		rule<> nickinfo = nick >> ch_p ('!')
				>> user >> ch_p ('@')
				>> hostmask;

		rule<> nicktypes = (nick | nickinfo | userinfo)[assign_a (nick_)];

		rule<> nicktrgt = nicktypes >> ch_p (',') >> str_p ("isnick")[assign_a (isNick, true)];

		rule<> channelstr = lexeme_d [!(ch_p ('#') | ch_p ('&') | ch_p ('+')) >>
				+(ascii - ' ' - '\0' - ',' - '\r' - '\n')][assign_a (channel_)];

		rule<> keystr = lexeme_d [channelstr >> ch_p (',') >> str_p ("needkey")[assign_a (channelPass, true)]];

		rule<> channeltrgt = longest_d [channelstr | keystr];

		rule<> target = longest_d [nicktrgt | channeltrgt];

		rule<> port = int_p[assign_a (port_)];
		rule<> uri = str_p ("irc:") >>
				!(str_p ("//") >>
				!(host >> !(ch_p (':') >> port))
				>> !ch_p ('/')
				>> !target >> !(ch_p (',') >> str_p ("needpass")[assign_a (serverPass, true)]));

		bool res = parse (url.toString ().toUtf8 ().constData (), uri).full;
		if (!res)
		{
			qWarning () << "input string is not a valide IRC URI"
					<< url.toString ().toUtf8 ().constData ();
			return;
		}

		if (!isNick)
		{
			ServerOptions so;
			if (host_.empty ())
				so.ServerName_ = "";
			else
				so.ServerName_ = QString::fromUtf8 (host_.c_str ());

			so.ServerEncoding_ = "";
			so.ServerPort_ = port_;
			so.ServerPassword_ = "";
			so.ServerNickName_ = "";

			ChannelOptions cho;
			if (channel_.empty ())
				cho.ChannelName_ = "";
			else
				cho.ChannelName_ = QString::fromUtf8 (channel_.c_str ());
			cho.ServerName_ = so.ServerName_;
			cho.ChannelPassword_ = "";

			if (channelPass)
				cho.ChannelPassword_ = QInputDialog::getText (0,
						tr ("This channel needs password."),
						tr ("Password:"),
						QLineEdit::Password);
			//TODO nickServ for urls
			acc->JoinServer (so, cho);
		}
	}

	bool IrcProtocol::SupportsURI (const QUrl& url) const
	{
		return url.scheme () == "irc";
	}

	void IrcProtocol::saveAccounts () const
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
					"_Azoth_Acetamide_Accounts");

		settings.beginWriteArray ("Accounts");

		for (int i = 0, size = IrcAccounts_.size (); i < size; ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("SerializedData",
					IrcAccounts_.at (i)->Serialize ());
		}

		settings.endArray ();
		settings.sync ();
	}

	void IrcProtocol::RestoreAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
					"_Azoth_Acetamide_Accounts");

		int size = settings.beginReadArray ("Accounts");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			QByteArray data = settings
					.value ("SerializedData").toByteArray ();

			IrcAccount *acc = IrcAccount::Deserialize (data, this);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "unserializable acount"
						<< i;
				continue;
			}

			connect (acc,
					SIGNAL (accountSettingsChanged ()),
					this,
					SLOT (saveAccounts ()));

			IrcAccounts_ << acc;
			emit accountAdded (acc);
		}
	}
};
};
};
