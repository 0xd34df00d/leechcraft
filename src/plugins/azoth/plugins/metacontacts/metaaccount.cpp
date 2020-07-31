/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "metaaccount.h"
#include <QtDebug>
#include "core.h"
#include "metaprotocol.h"
#include "metaentry.h"

namespace LC
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

	QObject* MetaAccount::GetQObject ()
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

	void MetaAccount::RemoveEntry (QObject *entryObj)
	{
		MetaEntry *entry = qobject_cast<MetaEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< entryObj
					<< "to MetaEntry";
			return;
		}

		Core::Instance ().RemoveEntry (entry);
	}

	QObject* MetaAccount::GetTransferManager () const
	{
		return 0;
	}
}
}
}
