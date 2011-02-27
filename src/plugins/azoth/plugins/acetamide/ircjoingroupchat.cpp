/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "ircjoingroupchat.h"
#include <QComboBox>
#include <QTextCodec>
#include <QValidator>
#include "ircaccount.h"

namespace LeechCraft
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
		
		Q_FOREACH (const QByteArray& codec, QTextCodec::availableCodecs ())
			Ui_.Encoding_->addItem (QString::fromUtf8 (codec));
		Ui_.Encoding_->model ()->sort (0);
		Ui_.Encoding_->setCurrentIndex (Ui_.Encoding_->findText ("UTF-8"));
		
		QRegExp rx ("^([\\#,\\&,\\!,\\+]?)([^\\,,\\a,\\s]+)");
		QValidator *validator = new QRegExpValidator (rx, this);
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
		Ui_.Nickname_->setText (acc->GetOurNick ());
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
		acc->JoinRoom (GetConnectionOptions (), GetChannelInfo ());
	}

	void IrcJoinGroupChat::Cancel ()
	{

	}

	QVariantList IrcJoinGroupChat::GetBookmarkedMUCs () const
	{
		return QVariantList ();
	}

	void IrcJoinGroupChat::SetIdentifyingData (const QVariantMap& data)
	{
		const QString& nick = data ["Nickname"].toString ();
		const QString& channel = data ["Channel"].toString ();
		const QString& server = data ["Server"].toString ();
		const QString& encoding = data ["Encoding"].toString ();
		int port = data ["Port"].toInt ();
		bool ssl = data ["SSL"].toBool ();

		if (!nick.isEmpty ())
			Ui_.Nickname_->setText (nick);
		if (!channel.isEmpty ())
			Ui_.Channel_->setText (channel);
		if (!server.isEmpty ())
			Ui_.Server_->setText (server);
		if (!encoding.isEmpty())
			Ui_.Encoding_->setCurrentIndex (Ui_.Encoding_->findText (encoding));
		if (port)
			Ui_.Port_->setValue (port);
		Ui_.SSL_->setChecked (ssl);
	}

	QVariantMap IrcJoinGroupChat::GetIdentifyingData () const
	{
		QVariantMap result;
		result ["HumanReadableName"] = QString ("%1 on %2@%3:%4")
				.arg (GetNickname ())
				.arg (GetChannel ())
				.arg (GetServer ())
				.arg (GetPort ());
		result ["AccountID"] = SelectedAccount_->GetAccountID ();
		result ["Nickname"] = GetNickname ();
		result ["Channel"] = GetChannel ();
		result ["Server"] = GetServer ();
		result ["Port"] = GetPort ();
		result ["Encoding"] = GetEncoding ();
		result ["SSL"] = GetSSL ();

		return result;
	}
	
	QString IrcJoinGroupChat::GetServer () const
	{
		return Ui_.Server_->text ().toLower ();
	}

	int IrcJoinGroupChat::GetPort () const
	{
		return Ui_.Port_->value ();
	}

	QString IrcJoinGroupChat::GetChannel () const
	{
		return Ui_.Channel_->text ().toLower ();
	}

	QString IrcJoinGroupChat::GetNickname () const
	{
		//TODO SelectedAccount_->GetNicknames () - return list of nicks for account;
		return Ui_.Nickname_->text ();
	}

	QString IrcJoinGroupChat::GetEncoding () const
	{
		return Ui_.Encoding_->currentText ();
	}

	bool IrcJoinGroupChat::GetSSL () const
	{
		return Ui_.SSL_->isChecked ();
	}

	ServerOptions IrcJoinGroupChat::GetConnectionOptions () const
	{
		ServerOptions opts;
		opts.NetworkName_ = QString ();
		opts.ServerName_ = GetServer ();
		opts.ServerPort_ = GetPort ();
		opts.ServerPassword_ = QString ();
		opts.ServerEncoding_ = GetEncoding ();
		opts.SSL_ = GetSSL ();
		opts.ServerNicknames_ << GetNickname ();
		opts.ServerRealName_ = Core::Instance ().GetDefaultIrcAccount ()->GetServers ().at (0).ServerRealName_;

		return opts;
	}

	ChannelOptions IrcJoinGroupChat::GetChannelInfo () const
	{
		ChannelOptions info;
		info.ServerName_ = GetServer ();
		info.ChannelName_ = GetChannel ();
		info.ChannelPassword_ = QString ();
		info.ChannelNickname_ = GetNickname ();

		return info;
	}
};
};
};
