/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "unauthclentry.h"
#include "glooxaccount.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
	UnauthCLEntry::UnauthCLEntry (const gloox::JID& jid,
			const QString& str, GlooxAccount *accountObj)
	: EntryBase (accountObj)
	, JID_ (jid)
	, Account_ (accountObj)
	{
		SetStatus (EntryStatus (SOffline, str), QString ());
	}

	QObject* UnauthCLEntry::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features UnauthCLEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType UnauthCLEntry::GetEntryType () const
	{
		return ETUnauthEntry;
	}

	QString UnauthCLEntry::GetEntryName () const
	{
		return QString::fromUtf8 (JID_.bare ().c_str ());
	}

	void UnauthCLEntry::SetEntryName (const QString&)
	{
	}

	QByteArray UnauthCLEntry::GetEntryID () const
	{
		return QByteArray (JID_.bare ().c_str ()) + "_unauth";
	}

	QString UnauthCLEntry::GetHumanReadableID () const
	{
		return QString::fromUtf8 (JID_.bare ().c_str ());
	}

	QStringList UnauthCLEntry::Groups () const
	{
		return QStringList ();
	}

	QStringList UnauthCLEntry::Variants () const
	{
		return QStringList (QString ());
	}

	QObject* UnauthCLEntry::CreateMessage (IMessage::MessageType,
			const QString& variant, const QString& body)
	{
		return 0;
	}

	AuthStatus UnauthCLEntry::GetAuthStatus () const
	{
		return ASNone;
	}

	gloox::JID UnauthCLEntry::GetJID () const
	{
		return JID_;
	}
}
}
}
}
}

