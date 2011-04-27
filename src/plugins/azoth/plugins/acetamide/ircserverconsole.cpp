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

#include "ircserverconsole.h"
#include "ircaccount.h"
#include "ircserverhandler.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcServerConsole::IrcServerConsole (IrcServerHandler *ish,
			IrcAccount *acc)
	: EntryBase (acc)
	, Account_ (acc)
	, ISH_ (ish)
	{
	}

	IrcServerHandler* IrcServerConsole::GetIrcServerHandler () const
	{
		return ISH_;
	}

	IrcAccount* IrcServerConsole::GetIrcAccount () const
	{
		return Account_;
	}

	QObject* IrcServerConsole::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features IrcServerConsole::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType IrcServerConsole::GetEntryType () const
	{
		return ETChat;
	}

	QString IrcServerConsole::GetEntryID () const
	{
		return Account_->GetAccountID () + "_" + ISH_->GetServerID_ () +
				"/Console";
	}

	QString IrcServerConsole::GetEntryName () const
	{
		return ISH_->GetServerID_ () + "/Console";
	}

	void IrcServerConsole::SetEntryName (const QString&)
	{
	}

	QStringList IrcServerConsole::Groups () const
	{
		return QStringList () << tr ("Server consoles");
	}

	void IrcServerConsole::SetGroups (const QStringList&)
	{
	}

	QStringList IrcServerConsole::Variants () const
	{
		QStringList result;
		result << "";
		return result;
	}

	QObject* IrcServerConsole::CreateMessage (IMessage::MessageType,
			const QString&, const QString&)
	{
		return 0;
	}
};
};
};