/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <QSettings>
#include <QtDebug>
#include "msnaccountconfigwidget.h"
#include "msnaccount.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	MSNProtocol::MSNProtocol (QObject *parent)
	: QObject (parent)
	{
	}

	void MSNProtocol::Init ()
	{
		RestoreAccounts ();
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
		QList<QObject*> result;
		Q_FOREACH (auto acc, Accounts_)
			result << acc;
		return result;
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
		return QIcon (":/plugins/azoth/plugins/zheet/resources/images/wlm.svg");
	}

	QByteArray MSNProtocol::GetProtocolID () const
	{
		return "msn.libmsn";
	}

	QList<QWidget*> MSNProtocol::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions)
	{
		return QList<QWidget*> () << new MSNAccountConfigWidget ();
	}

	void MSNProtocol::RegisterAccount (const QString& name, const QList<QWidget*>& widgets)
	{
		auto w = qobject_cast<MSNAccountConfigWidget*> (widgets.value (0));
		if (!w)
		{
			qWarning () << Q_FUNC_INFO
					<< "wrong first widget"
					<< widgets;
			return;
		}

		MSNAccount *acc = new MSNAccount (name, this);
		acc->FillConfig (w);
		acc->Init ();

		Accounts_ << acc;

		emit accountAdded (acc);

		saveAccounts ();
	}

	QWidget* MSNProtocol::GetMUCJoinWidget ()
	{
		return 0;
	}

	void MSNProtocol::RemoveAccount (QObject *acc)
	{
		if (Accounts_.removeAll (static_cast<MSNAccount*> (acc)))
		{
			emit accountRemoved (acc);
			saveAccounts ();
			acc->deleteLater ();
		}
	}

	void MSNProtocol::RestoreAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Zheet_Accounts");
		int size = settings.beginReadArray ("Accounts");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			const QByteArray& data = settings.value ("SerializedData").toByteArray ();
			MSNAccount *acc = MSNAccount::Deserialize (data, this);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "unserializable acount"
						<< i;
				continue;
			}

			connect (acc,
					SIGNAL (accountSettingsChanged ()),
					this,
					SLOT (saveAccounts ()));

			Accounts_ << acc;

			emit accountAdded (acc);
		}
		settings.endArray ();
	}

	void MSNProtocol::saveAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Zheet_Accounts");
		settings.beginWriteArray ("Accounts");
		for (int i = 0, size = Accounts_.size ();
				i < size; ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("SerializedData", Accounts_.at (i)->Serialize ());
		}
		settings.endArray ();
		settings.sync ();
	}
}
}
}
