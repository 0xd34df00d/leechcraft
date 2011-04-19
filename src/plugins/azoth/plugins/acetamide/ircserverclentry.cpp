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

#include "ircserverclentry.h"
#include "ircaccount.h"
#include "ircserverhandler.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcServerCLEntry::IrcServerCLEntry (IrcServerHandler *handler,
			IrcAccount *account)
	: EntryBase (account)
	, Account_ (account)
	, ISH_ (handler)
	{
	}

	IrcServerHandler* IrcServerCLEntry::GetIrcServerHandler() const
	{
		return ISH_;
	}

	IrcAccount* IrcServerCLEntry::GetIrcAccount() const
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
		return ETChat;
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

	QObject* IrcServerCLEntry::CreateMessage (IMessage::MessageType type,
			const QString& variant, const QString& body)
	{
	}

};
};
};