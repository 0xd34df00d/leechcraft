/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "ircprotocol.h"
#include <boost/bind.hpp>
#include <QCoreApplication>
#include <QInputDialog>
#include <QMainWindow>
#include <QSettings>
#include <interfaces/iprotocolplugin.h>
#include "core.h"
#include "ircaccount.h"
#include "ircaccountconfigurationwidget.h"
#include "ircjoingroupchat.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcProtocol::IrcProtocol (QObject *parent)
	: QObject (parent)
	, ParentProtocolPlugin_ (parent)
	, ProxyObject_ (0)
	{
	}

	IrcProtocol::~IrcProtocol ()
	{
	}

	void IrcProtocol::Prepare ()
	{
		RestoreAccounts ();
	}

	QObject* IrcProtocol::GetProxyObject () const
	{
		return ProxyObject_;
	}

	void IrcProtocol::SetProxyObject (QObject *po)
	{
		ProxyObject_ = po;
	}

	QObject* IrcProtocol::GetObject ()
	{
		return this;
	}

	IProtocol::ProtocolFeatures IrcProtocol::GetFeatures () const
	{
		return PFMUCsJoinable | PFSupportsMUCs;
	}

	QList<QObject*> IrcProtocol::GetRegisteredAccounts ()
	{
		QList<QObject*> result;
		Q_FOREACH (IrcAccount *acc, IrcAccounts_)
			result << acc;
		return result;
	}

	QObject* IrcProtocol::GetParentProtocolPlugin () const
	{
		return ParentProtocolPlugin_;
	}

	QString IrcProtocol::GetProtocolName () const
	{
		return "IRC";
	}
	
	QIcon IrcProtocol::GetProtocolIcon () const
	{
		return QIcon (":/plugins/azoth/plugins/acetamide/resources/images/ircicon.svg");
	}

	QByteArray IrcProtocol::GetProtocolID () const
	{
		return "Acetamide.IRC";
	}

	QList<QWidget*> IrcProtocol::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions)
	{
		QList<QWidget*> result;
		result << new IrcAccountConfigurationWidget ();
		return result;
	}

	void IrcProtocol::RegisterAccount (const QString& name,
			const QList<QWidget*>& widgets)
	{
		IrcAccountConfigurationWidget *w =
				qobject_cast<IrcAccountConfigurationWidget*>
					(widgets.value (0));
		if (!w)
		{
			qWarning () << Q_FUNC_INFO
					<< "got invalid widgets"
					<< widgets;
			return;
		}

		IrcAccount *account = new IrcAccount (name, this);
		account->FillSettings (w);
		account->SetAccountID (GetProtocolID () + "." +
				QString::number (QDateTime::currentDateTime ()
					.toTime_t ()));
		IrcAccounts_ << account;
		saveAccounts ();
		emit accountAdded (account);
	}

	QWidget* IrcProtocol::GetMUCJoinWidget ()
	{
		return new IrcJoinGroupChat ();
	}

	QWidget* IrcProtocol::GetMUCBookmarkEditorWidget ()
	{
		return 0;
	}

	void IrcProtocol::RemoveAccount (QObject *acc)
	{
		IrcAccount *accObj = qobject_cast<IrcAccount*> (acc);
		if (IrcAccounts_.removeAll (accObj))
		{
			emit accountRemoved (accObj);
			accObj->deleteLater ();
			saveAccounts ();
		}
	}

	void IrcProtocol::saveAccounts () const
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
					"_Azoth_Acetamide_Accounts");

		settings.beginWriteArray ("Accounts");

		for (int i = 0, size = IrcAccounts_.size (); i < size; ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("SerializedData",
					IrcAccounts_.at (i)->Serialize ());
		}

		settings.endArray ();
		settings.sync ();
	}

	void IrcProtocol::RestoreAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
					"_Azoth_Acetamide_Accounts");

		int size = settings.beginReadArray ("Accounts");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			QByteArray data = settings
					.value ("SerializedData").toByteArray ();

			IrcAccount *acc = IrcAccount::Deserialize (data, this);
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

			IrcAccounts_ << acc;
			emit accountAdded (acc);
		}
	}
};
};
};
