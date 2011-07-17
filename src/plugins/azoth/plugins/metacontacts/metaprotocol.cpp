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

#include "metaprotocol.h"
#include <QIcon>
#include "metaaccount.h"

namespace LeechCraft
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
	
	QObject* MetaProtocol::GetObject ()
	{
		return this;
	}
	
	IProtocol::ProtocolFeatures MetaProtocol::GetFeatures () const
	{
		return PFNone;
	}
	
	QList<QObject*> MetaProtocol::GetRegisteredAccounts ()
	{
		QList<QObject*> result;
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
	
	QWidget* MetaProtocol::GetMUCJoinWidget ()
	{
		return 0;
	}
	
	QWidget* MetaProtocol::GetMUCBookmarkEditorWidget ()
	{
		return 0;
	}

	void MetaProtocol::RemoveAccount (QObject*)
	{
	}
}
}
}
