/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "metaprotocol.h"
#include <QIcon>
#include "metaaccount.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
namespace Metacontacts
{
	MetaProtocol::MetaProtocol (QObject *parent)
	: QObject (parent)
	, ParentPlugin_ (parent)
	{
		Account_ = new MetaAccount (this);
	}

	MetaProtocol::~MetaProtocol ()
	{
		Release ();
	}

	void MetaProtocol::Release ()
	{
		if (!Account_)
			return;

		Core::Instance ().SetMetaAccount (0);
		delete Account_;
		Account_ = 0;
	}

	QObject* MetaProtocol::GetQObject ()
	{
		return this;
	}

	IProtocol::ProtocolFeatures MetaProtocol::GetFeatures () const
	{
		return PFNoAccountRegistration;
	}

	QList<QObject*> MetaProtocol::GetRegisteredAccounts ()
	{
		QList<QObject*> result;
		if (!Account_->GetCLEntries ().isEmpty ())
			result << Account_;
		return result;
	}

	QObject* MetaProtocol::GetParentProtocolPlugin () const
	{
		return ParentPlugin_;
	}

	QString MetaProtocol::GetProtocolName () const
	{
		return tr ("Metacontacts");
	}

	QIcon MetaProtocol::GetProtocolIcon () const
	{
		return QIcon ();
	}

	QByteArray MetaProtocol::GetProtocolID () const
	{
		return "org.LeechCraft.Azoth.Protocols.MetaProtocol";
	}

	QList<QWidget*> MetaProtocol::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions)
	{
		return QList<QWidget*> ();
	}

	void MetaProtocol::RegisterAccount (const QString&, const QList<QWidget*>&)
	{
	}

	void MetaProtocol::RemoveAccount (QObject*)
	{
	}
}
}
}
