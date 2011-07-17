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

		Q_FOREACH (const QByteArray& codec,
				QTextCodec::availableCodecs ())
			Ui_.Encoding_->addItem (QString::fromUtf8 (codec));
		Ui_.Encoding_->model ()->sort (0);
		Ui_.Encoding_->
				setCurrentIndex (Ui_.Encoding_->findText ("UTF-8"));

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
		acc->JoinServer (GetServerOptions (), GetChannelOptions ());
	}

	void IrcJoinGroupChat::Cancel ()
	{
	}

	QVariantList IrcJoinGroupChat::GetBookmarkedMUCs () const
	{
		return QVariantList ();
	}

	void IrcJoinGroupChat::SetBookmarkedMUCs (QObject*,
			const QVariantList&)
	{
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
		if (!encoding.isEmpty ())
			Ui_.Encoding_->
					setCurrentIndex (Ui_.Encoding_->findText (encoding));
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

	ServerOptions IrcJoinGroupChat::GetServerOptions () const
	{
		ServerOptions so;
		so.ServerName_ = GetServer ();
		so.ServerPort_ = GetPort ();
		so.ServerEncoding_ = GetEncoding ();
		so.ServerPassword_ = QString ();
		so.SSL_ = GetSSL ();
		so.ServerNickName_ = GetNickname ();

		return so;
	}

	ChannelOptions IrcJoinGroupChat::GetChannelOptions () const
	{
		ChannelOptions cho;
		cho.ChannelName_ = GetChannel ();
		cho.ServerName_ = GetServer ();
		cho.ChannelPassword_ = QString ();

		return cho;
	}

};
};
};
