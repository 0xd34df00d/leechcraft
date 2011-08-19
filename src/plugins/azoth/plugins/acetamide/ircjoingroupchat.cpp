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
#include "ircprotocol.h"

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
		IrcProtocol *proto = qobject_cast<IrcProtocol*> (Core::Instance ().GetProtocols ().at (0));
		QVariantList result;
		Q_FOREACH (QObject *obj, proto->GetRegisteredAccounts ())
		{
			IrcAccount *acc = qobject_cast<IrcAccount*> (obj);
			const QList<IrcBookmark>& bookmarks = acc->GetBookmarks ();
			Q_FOREACH (const IrcBookmark& channel, bookmarks)
			{
				QVariantMap cm;
				cm ["HumanReadableName"] = QString ("%1@%2 (%3)")
						.arg (channel.ChannelName_ )
						.arg (channel.ServerName_)
						.arg (channel.NickName_);
				cm ["AccountID"] = acc->GetAccountID ();
				cm ["Server"] = channel.ServerName_;
				cm ["Port"] = channel.ServerPort_;
				cm ["Encoding"] = channel.ServerEncoding_;
				cm ["Channel"] = channel.ChannelName_;
				cm ["Password"] = channel.ChannelPassword_;
				cm ["Nickname"] = channel.NickName_;
				cm ["SSL"] = channel.SSL_;
				cm ["Autojoin"] = channel.AutoJoin_;
				cm ["StoredName"] = channel.Name_;
				result << cm;
			}
		}
		return result;
	}

	void IrcJoinGroupChat::SetBookmarkedMUCs (QObject *account,
			const QVariantList& datas)
	{
		IrcAccount *acc = qobject_cast<IrcAccount*> (account);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< account
					<< "is not a IrcAccount";
			return;
		}

		
		QList<IrcBookmark> channels;
		Q_FOREACH (const QVariant& var, datas)
		{
			const QVariantMap& map = var.toMap ();
			IrcBookmark bookmark;
			bookmark.AutoJoin_  = map.value ("Autojoin").toBool ();
			bookmark.ServerName_ = map.value ("Server").toString ();
			bookmark.ServerPort_ = map.value ("Port").toInt ();
			bookmark.ServerEncoding_ = map.value ("Encoding").toString ();
			bookmark.ChannelName_ = map.value ("Channel").toString ();
			bookmark.ChannelPassword_ = map.value ("Password").toString ();
			bookmark.SSL_ = map.value ("SSL").toBool ();
			bookmark.NickName_ = map.value ("Nickname").toString ();
			bookmark.Name_ = map.value ("StoredName").toString ();
			channels << bookmark;
		}

		acc->SetBookmarks (channels);
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
