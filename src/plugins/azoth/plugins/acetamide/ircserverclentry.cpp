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

#include "ircserverclentry.h"
#include "ircaccount.h"
#include "ircmessage.h"
#include "ircserverhandler.h"
#include "clientconnection.h"
#include "ircparser.h"
#include "servercommandmessage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcServerCLEntry::IrcServerCLEntry (IrcServerHandler *handler,
			IrcAccount *account)
	: EntryBase (account)
	, ISH_ (handler)
	, Account_ (account)
	{
	}

	IrcServerHandler* IrcServerCLEntry::GetIrcServerHandler () const
	{
		return ISH_;
	}

	IrcAccount* IrcServerCLEntry::GetIrcAccount () const
	{
		return Account_;
	}

	QObject* IrcServerCLEntry::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features IrcServerCLEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType IrcServerCLEntry::GetEntryType () const
	{
		return ETMUC;
	}

	QString IrcServerCLEntry::GetEntryID () const
	{
		return Account_->GetAccountID () + "_" + ISH_->GetServerID_ ();
	}

	QString IrcServerCLEntry::GetEntryName () const
	{
		return ISH_->GetServerID_ ();
	}

	void IrcServerCLEntry::SetEntryName (const QString&)
	{
	}

	QStringList IrcServerCLEntry::Groups () const
	{
		return QStringList () << tr ("Servers");
	}

	void IrcServerCLEntry::SetGroups (const QStringList&)
	{
	}

	QStringList IrcServerCLEntry::Variants () const
	{
		QStringList result;
		result << "";
		return result;
	}

	QObject* IrcServerCLEntry::CreateMessage (IMessage::MessageType,
			const QString& variant, const QString& body)
	{
		if (variant.isEmpty ())
			return new ServerCommandMessage (body, this);
		else
			return 0;
	}

	IMUCEntry::MUCFeatures IrcServerCLEntry::GetMUCFeatures () const
	{
		return 0;
	}

	QString IrcServerCLEntry::GetMUCSubject () const
	{
		return QString ();
	}

	void IrcServerCLEntry::SetMUCSubject (const QString&)
	{
	}

	QList<QObject*> IrcServerCLEntry::GetParticipants ()
	{
		return QList<QObject*> ();
	}

	void IrcServerCLEntry::Join ()
	{
		ISH_->GetParser ()->NickCommand (QStringList () << ISH_->GetNickName ());
	}

	void IrcServerCLEntry::Leave (const QString&)
	{
		Account_->GetClientConnection ()->CloseServer (ISH_->GetServerID_ ());
	}

	QString IrcServerCLEntry::GetNick () const
	{
		return ISH_->GetNickName ();
	}

	void IrcServerCLEntry::SetNick (const QString& nick)
	{
		ISH_->SetNickName (nick);
	}

	QString IrcServerCLEntry::GetGroupName () const
	{
		return QString ();
	}

	QString IrcServerCLEntry::GetRealID (QObject*) const
	{
		return QString ();
	}

	QVariantMap IrcServerCLEntry::GetIdentifyingData () const
	{
		QVariantMap result;
		result ["HumanReadableName"] = QString ("%1 on %2:%3")
				.arg (ISH_->GetNickName ())
				.arg (ISH_->GetServerOptions ().ServerName_)
				.arg (ISH_->GetServerOptions ().ServerPort_);
		result ["AccountID"] = ISH_->
				GetAccount ()->GetAccountID ();
		result ["Nickname"] = ISH_->
				GetNickName ();
		result ["Server"] = ISH_->GetServerOptions ().ServerName_;
		result ["Port"] = ISH_->GetServerOptions ().ServerPort_;
		result ["Encoding"] = ISH_->GetServerOptions ().ServerEncoding_;
		result ["SSL"] = ISH_->GetServerOptions ().SSL_;

		return result;
	}

};
};
};