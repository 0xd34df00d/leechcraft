/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ircjoingroupchat.h"
#include <QComboBox>
#include <QTextCodec>
#include <QValidator>
#include "ircaccount.h"
#include "ircprotocol.h"
#include "localtypes.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	IrcJoinGroupChat::IrcJoinGroupChat (QWidget *parent)
	: QWidget (parent)
	, SelectedAccount_ (0)
	{
		Ui_.setupUi (this);

		Ui_.Channel_->setMaxLength (50);

		for (const auto& codec : QTextCodec::availableCodecs ())
			Ui_.Encoding_->addItem (QString::fromUtf8 (codec));
		Ui_.Encoding_->model ()->sort (0);
		Ui_.Encoding_->setCurrentIndex (Ui_.Encoding_->findText ("UTF-8"));

		QRegExp rx ("^([\\#,\\&,\\!,\\+]?)([^\\,,\\a,\\s]+)");
		const auto validator = new QRegExpValidator (rx, this);
		Ui_.Channel_->setValidator (validator);
	}

	void IrcJoinGroupChat::AccountSelected (QObject *accObj)
	{
		IrcAccount *acc = qobject_cast<IrcAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< accObj
					<< "to IrcAccount";
			return;
		}

		SelectedAccount_ = acc;
		Ui_.Nickname_->setText (acc->GetNickNames ().at (0));
	}

	void IrcJoinGroupChat::Join (QObject *accObj)
	{
		IrcAccount *acc = qobject_cast<IrcAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< accObj
					<< "to IrcAccount";
			return;
		}

		SelectedAccount_ = acc;
		acc->JoinServer (GetServerOptions (), GetChannelOptions (),
				Ui_.OnlyServerConnect_->isChecked ());
	}

	void IrcJoinGroupChat::Cancel ()
	{
	}

	void IrcJoinGroupChat::SetIdentifyingData (const QVariantMap& data)
	{
		const QString& nick = data [Lits::Nickname].toString ();
		const QString& channel = data [Lits::Channel].toString ();
		const QString& server = data [Lits::Server].toString ();
		const QString& encoding = data [Lits::Encoding].toString ();
		const QString& serverPass = data [Lits::ServerPassword].toString ();
		const QString& channelPass = data [Lits::ChannelPassword].toString ();
		const int port = data [Lits::Port].toInt ();
		const bool ssl = data [Lits::SSL].toBool ();

		if (!nick.isEmpty ())
			Ui_.Nickname_->setText (nick);
		if (!channel.isEmpty ())
			Ui_.Channel_->setText (channel);
		if (!server.isEmpty ())
			Ui_.Server_->setText (server);
		if (!encoding.isEmpty ())
			Ui_.Encoding_->setCurrentIndex (Ui_.Encoding_->findText (encoding));
		if (port)
			Ui_.Port_->setValue (port);
		if (!serverPass.isEmpty ())
			Ui_.ServerPassword_->setText (serverPass);
		if (!channelPass.isEmpty ())
			Ui_.Password_->setText (channelPass);
		Ui_.SSL_->setChecked (ssl);
	}

	QVariantMap IrcJoinGroupChat::GetIdentifyingData () const
	{
		const auto& name = QStringLiteral ("%1 on %2@%3:%4")
				.arg (GetNickname (),
						GetChannel (),
						GetServer ())
				.arg (GetPort ());
		return
		{
			{ Lits::HumanReadableName, name },
			{ Lits::AccountID, SelectedAccount_->GetAccountID () },
			{ Lits::Nickname, GetNickname () },
			{ Lits::Channel, GetChannel () },
			{ Lits::ChannelPassword, GetChannelPassword () },
			{ Lits::Server, GetServer () },
			{ Lits::Port, GetPort () },
			{ Lits::ServerPassword, GetServerPassword () },
			{ Lits::Encoding, GetEncoding () },
			{ Lits::SSL, GetSSL () },
		};
	}

	QString IrcJoinGroupChat::GetServer () const
	{
		return Ui_.Server_->text ().toLower ();
	}

	int IrcJoinGroupChat::GetPort () const
	{
		return Ui_.Port_->value ();
	}

	QString IrcJoinGroupChat::GetServerPassword () const
	{
		return Ui_.ServerPassword_->text ();
	}

	QString IrcJoinGroupChat::GetChannel () const
	{
		QString channel = Ui_.Channel_->text ().toLower ();
		if (!Ui_.Channel_->text ().startsWith ('#') &&
				!Ui_.Channel_->text ().startsWith ('&') &&
				!Ui_.Channel_->text ().startsWith ('+') &&
				!Ui_.Channel_->text ().startsWith ('!'))
			channel.prepend ('#');
		return channel;
	}

	QString IrcJoinGroupChat::GetNickname () const
	{
		return Ui_.Nickname_->text ();
	}

	QString IrcJoinGroupChat::GetEncoding () const
	{
		return Ui_.Encoding_->currentText ();
	}

	QString IrcJoinGroupChat::GetChannelPassword () const
	{
		return Ui_.Password_->text ();
	}

	bool IrcJoinGroupChat::GetSSL () const
	{
		return Ui_.SSL_->isChecked ();
	}

	ServerOptions IrcJoinGroupChat::GetServerOptions () const
	{
		ServerOptions so;
		so.ServerName_ = GetServer ();
		so.ServerPort_ = GetPort ();
		so.ServerEncoding_ = GetEncoding ();
		so.ServerPassword_ = GetServerPassword ();
		so.SSL_ = GetSSL ();
		so.ServerNickName_ = GetNickname ();
		return so;
	}

	ChannelOptions IrcJoinGroupChat::GetChannelOptions () const
	{
		ChannelOptions cho;
		cho.ChannelName_ = GetChannel ();
		cho.ServerName_ = GetServer ();
		cho.ChannelPassword_ = GetChannelPassword ();

		return cho;
	}
};
};
};
