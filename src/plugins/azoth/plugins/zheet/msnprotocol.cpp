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

#include "msnprotocol.h"
#include <QIcon>

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	MSNProtocol::MSNProtocol (QObject*)
	: QObject ()
	{
	}

	QObject* MSNProtocol::GetObject ()
	{
		return this;
	}

	IProtocol::ProtocolFeatures MSNProtocol::GetFeatures () const
	{
		return PFNone;
	}

	QList<QObject*> MSNProtocol::GetRegisteredAccounts ()
	{
		return QList<QObject*> ();
	}

	QObject* MSNProtocol::GetParentProtocolPlugin () const
	{
		return parent ();
	}

	QString MSNProtocol::GetProtocolName() const
	{
		return "MSN";
	}

	QIcon MSNProtocol::GetProtocolIcon () const
	{
		return QIcon ();
	}

	QByteArray MSNProtocol::GetProtocolID () const
	{
		return "msn.libmsn";
	}

	QList<QWidget*> MSNProtocol::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions)
	{
		return QList<QWidget*> ();
	}

	void MSNProtocol::RegisterAccount (const QString&, const QList<QWidget*>&)
	{

	}

	QWidget* MSNProtocol::GetMUCJoinWidget ()
	{
		return 0;
	}

	void MSNProtocol::RemoveAccount (QObject*)
	{
	}
}
}
}
