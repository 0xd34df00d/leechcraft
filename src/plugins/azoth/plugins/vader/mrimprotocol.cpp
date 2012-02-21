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

#include "mrimprotocol.h"
#include <QIcon>
#include <QSettings>
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

	void MRIMProtocol::Init ()
	{
		RestoreAccounts ();
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
		QList<QObject*> result;
		Q_FOREACH (auto acc, Accounts_)
			result << acc;
		return result;
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
		return QIcon (":/plugins/azoth/plugins/vader/resources/images/mrim.svg");
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

		saveAccounts ();
	}

	QWidget* MRIMProtocol::GetMUCJoinWidget ()
	{
		return 0;
	}

	void MRIMProtocol::RemoveAccount (QObject *acc)
	{
		if (Accounts_.removeAll (qobject_cast<MRIMAccount*> (acc)))
		{
			emit accountRemoved (acc);
			saveAccounts ();
			acc->deleteLater ();
		}
	}

	void MRIMProtocol::RestoreAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Vader_Accounts");
		int size = settings.beginReadArray ("Accounts");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			const QByteArray& data = settings.value ("SerializedData").toByteArray ();
			MRIMAccount *acc = MRIMAccount::Deserialize (data, this);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "undeserializable acount"
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

	void MRIMProtocol::saveAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Vader_Accounts");
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
