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
#include "localtypes.h"

namespace LC::Azoth::Acetamide
{
	IrcJoinGroupChat::IrcJoinGroupChat (QWidget *parent)
	: QWidget { parent }
	{
		Ui_.setupUi (this);

		for (const auto& codec : QTextCodec::availableCodecs ())
			Ui_.Encoding_->addItem (QString::fromUtf8 (codec));
		Ui_.Encoding_->model ()->sort (0);
		Ui_.Encoding_->setCurrentIndex (Ui_.Encoding_->findText (QStringLiteral ("UTF-8")));

		QRegExp rx { R"(^([\#,\&,\!,\+]?)([^\,,\a,\s]+))" };
		const auto validator = new QRegExpValidator (rx, this);
		Ui_.Channel_->setValidator (validator);
	}

	void IrcJoinGroupChat::AccountSelected (QObject *accObj)
	{
		const auto acc = qobject_cast<IrcAccount*> (accObj);
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
		const auto acc = qobject_cast<IrcAccount*> (accObj);
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
		const auto& nick = data [Lits::Nickname].toString ();
		const auto& channel = data [Lits::Channel].toString ();
		const auto& server = data [Lits::Server].toString ();
		const auto& encoding = data [Lits::Encoding].toString ();
		const auto& serverPass = data [Lits::ServerPassword].toString ();
		const auto& channelPass = data [Lits::ChannelPassword].toString ();
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
		auto channel = Ui_.Channel_->text ().toLower ();
		if (!channel.startsWith ('#') &&
				!channel.startsWith ('&') &&
				!channel.startsWith ('+') &&
				!channel.startsWith ('!'))
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
		return
		{
			.ServerName_ = GetServer (),
			.ServerEncoding_ = GetEncoding (),
			.ServerPassword_ = GetServerPassword (),
			.ServerNickName_ = GetNickname (),
			.ServerPort_ = GetPort (),
			.SSL_ = GetSSL (),
		};
	}

	ChannelOptions IrcJoinGroupChat::GetChannelOptions () const
	{
		return
		{
			.ServerName_ = GetServer (),
			.ChannelName_ = GetChannel (),
			.ChannelPassword_ = GetChannelPassword (),
		};
	}
}
