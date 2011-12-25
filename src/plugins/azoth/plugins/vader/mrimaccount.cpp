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

#include "mrimaccount.h"
#include "mrimprotocol.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	MRIMAccount::MRIMAccount (const QString& name, MRIMProtocol *proto)
	: QObject (proto)
	, Proto_ (proto)
	, Name_ (name)
	{
	}

	QObject* MRIMAccount::GetObject ()
	{
		return this;
	}

	QObject* MRIMAccount::GetParentProtocol () const
	{
		return Proto_;
	}

	IAccount::AccountFeatures MRIMAccount::GetAccountFeatures () const
	{
		return FHasConfigurationDialog;
	}

	QList<QObject*> MRIMAccount::GetCLEntries ()
	{
		return QList<QObject*> ();
	}

	QString MRIMAccount::GetAccountName () const
	{
		return Name_;
	}

	QString MRIMAccount::GetOurNick () const
	{
		return Login_.split ('@', QString::SkipEmptyParts).value (0);
	}

	void MRIMAccount::RenameAccount (const QString& name)
	{
		Name_ = name;
	}

	QByteArray MRIMAccount::GetAccountID () const
	{
		return Proto_->GetProtocolID () + "_" + Login_.toUtf8 ();
	}

	QList<QAction*> MRIMAccount::GetActions () const
	{
		return QList<QAction*> ();
	}

	void MRIMAccount::QueryInfo (const QString&)
	{
	}

	void MRIMAccount::OpenConfigurationDialog ()
	{
	}

	EntryStatus MRIMAccount::GetState () const
	{
		return EntryStatus ();
	}

	void MRIMAccount::ChangeState (const EntryStatus& status)
	{

	}

	void MRIMAccount::Synchronize ()
	{
	}

	void MRIMAccount::Authorize (QObject*)
	{
	}

	void MRIMAccount::DenyAuth (QObject*)
	{
	}

	void MRIMAccount::RequestAuth (const QString& , const QString& , const QString& , const QStringList&)
	{
	}

	void MRIMAccount::RemoveEntry (QObject*)
	{
	}

	QObject* MRIMAccount::GetTransferManager () const
	{
		return 0;
	}
}
}
}
