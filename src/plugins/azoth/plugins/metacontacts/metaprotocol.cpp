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

namespace LeechCraft
{
namespace Azoth
{
namespace Metacontacts
{
	Metaprotocol::Metaprotocol (QObject *parent)
	: QObject (parent)
	, ParentPlugin_ (parent)
	{
	}
	
	QObject* Metaprotocol::GetObject ()
	{
		return this;
	}
	
	IProtocol::ProtocolFeatures Metaprotocol::GetFeatures () const
	{
		return PFNone;
	}
	
	QList<QObject*> Metaprotocol::GetRegisteredAccounts ()
	{
		return QList<QObject*> ();
	}
	
	QObject* Metaprotocol::GetParentProtocolPlugin () const
	{
		return ParentPlugin_;
	}
	
	QString Metaprotocol::GetProtocolName () const
	{
		return tr ("Metacontacts");
	}

	QIcon Metaprotocol::GetProtocolIcon () const
	{
		return QIcon ();
	}

	QByteArray Metaprotocol::GetProtocolID () const
	{
		return "org.LeechCraft.Azoth.Protocols.Metaprotocol";
	}
	
	QList<QWidget*> Metaprotocol::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions)
	{
		return QList<QWidget*> ();
	}
	
	void Metaprotocol::RegisterAccount (const QString&, const QList<QWidget*>&)
	{
	}
	
	QWidget* Metaprotocol::GetMUCJoinWidget ()
	{
		return 0;
	}
	
	QWidget* Metaprotocol::GetMUCBookmarkEditorWidget ()
	{
		return 0;
	}

	void Metaprotocol::RemoveAccount (QObject*)
	{
	}
}
}
}
