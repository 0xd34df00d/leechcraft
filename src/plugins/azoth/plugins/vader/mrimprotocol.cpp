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

#include "mrimprotocol.h"
#include <QIcon>
#include <QtDebug>
#include "mrimaccount.h"
#include "mrimaccountconfigwidget.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	MRIMProtocol::MRIMProtocol (QObject *parent)
	: QObject (parent)
	{
	}

	QObject* MRIMProtocol::GetObject ()
	{
		return this;
	}

	IProtocol::ProtocolFeatures MRIMProtocol::GetFeatures () const
	{
		return PFNone;
	}

	QList<QObject*> MRIMProtocol::GetRegisteredAccounts ()
	{
		return QList<QObject*> ();
	}

	QObject* MRIMProtocol::GetParentProtocolPlugin () const
	{
		return parent ();
	}

	QString MRIMProtocol::GetProtocolName () const
	{
		return "Mail.ru Agent";
	}

	QIcon MRIMProtocol::GetProtocolIcon () const
	{
		return QIcon ();
	}

	QByteArray MRIMProtocol::GetProtocolID () const
	{
		return "org.LC.MRIM";
	}

	QList<QWidget*> MRIMProtocol::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions)
	{
		return QList<QWidget*> () << new MRIMAccountConfigWidget;
	}

	void MRIMProtocol::RegisterAccount (const QString& name, const QList<QWidget*>& widgets)
	{
		auto w = qobject_cast<MRIMAccountConfigWidget*> (widgets.value (0));
		if (!w)
		{
			qWarning () << Q_FUNC_INFO
					<< "first widget is invalid"
					<< widgets;
			return;
		}

		MRIMAccount *acc = new MRIMAccount (name, this);
		acc->FillConfig (w);
		Accounts_ << acc;

		emit accountAdded (acc);
	}

	QWidget* MRIMProtocol::GetMUCJoinWidget ()
	{
		return 0;
	}

	void MRIMProtocol::RemoveAccount (QObject*)
	{
	}
}
}
}
