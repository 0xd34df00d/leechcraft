/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "metaaccount.h"
#include "core.h"
#include "metaprotocol.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Metacontacts
{
	MetaAccount::MetaAccount (MetaProtocol *parent)
	: QObject (parent)
	, Parent_ (parent)
	{
		Core::Instance ().SetMetaAccount (this);
	}
	
	QObject* MetaAccount::GetObject ()
	{
		return this;
	}
	
	QObject* MetaAccount::GetParentProtocol () const
	{
		return Parent_;
	}

	IAccount::AccountFeatures MetaAccount::GetAccountFeatures () const
	{
		return FMUCsSupportFileTransfers;
	}

	QList<QObject*> MetaAccount::GetCLEntries ()
	{
		return Core::Instance ().GetEntries ();
	}

	QString MetaAccount::GetAccountName () const
	{
		return tr ("Metacontacts");
	}

	QString MetaAccount::GetOurNick () const
	{
		return "R";
	}

	void MetaAccount::RenameAccount (const QString&)
	{
	}
	
	QByteArray MetaAccount::GetAccountID () const
	{
		return "org.LeechCraft.Azoth.Accounts.MetaAccount";
	}
	
	QList<QAction*> MetaAccount::GetActions () const
	{
		return QList<QAction*> ();
	}

	void MetaAccount::QueryInfo (const QString&)
	{
	}

	void MetaAccount::OpenConfigurationDialog ()
	{
	}

	EntryStatus MetaAccount::GetState () const
	{
		return EntryStatus (SOnline, QString ());
	}

	void MetaAccount::ChangeState (const EntryStatus&)
	{
	}
	
	void MetaAccount::Synchronize ()
	{
	}
	
	void MetaAccount::Authorize (QObject*)
	{
	}

	void MetaAccount::DenyAuth (QObject*)
	{
	}

	void MetaAccount::RequestAuth (const QString&, const QString&,
			const QString&, const QStringList&)
	{
	}

	void MetaAccount::RemoveEntry (QObject*)
	{
	}

	QObject* MetaAccount::GetTransferManager () const
	{
		return 0;
	}
}
}
}
