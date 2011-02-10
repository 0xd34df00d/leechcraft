/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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
#include <QInputDialog>
#include <QMainWindow>
#include <QSettings>
#include <QCoreApplication>
#include <QtDebug>
#include <interfaces/iprotocolplugin.h>
#include "ircaccount.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcProtocol::IrcProtocol (QObject *parent)
	: QObject (parent)
	, ParentProtocolPlugin_ (parent)
	,ProxyObject_ (0)
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
		Q_FOREACH (IrcAccount *acc, Accounts_)
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

	QByteArray IrcProtocol::GetProtocolID () const
	{
		return "Acetamide.IRC";
	}

	void IrcProtocol::InitiateAccountRegistration ()
	{
		QString name = QInputDialog::getText (0,
				"LeechCraft",
				tr ("Enter new account name"));
		if (name.isEmpty ())
			return;
		
		IrcAccount *account = new IrcAccount (name, this);
		account->OpenConfigurationDialog ();

		connect (account,
				SIGNAL (accountSettingsChanged ()),
				this,
				SLOT (saveAccounts ()));

		Accounts_ << account;
		saveAccounts ();

		emit accountAdded (account);

		account->ChangeState (EntryStatus (SOnline, QString ()));
	}

	QWidget* IrcProtocol::GetMUCJoinWidget ()
	{
		//TODO join MUC 
		//return new JoinGroupchatWidget ();
	}

	void IrcProtocol::RemoveAccount (QObject *acc)
	{
		IrcAccount *accObj = qobject_cast<IrcAccount*> (acc);
		Accounts_.removeAll (accObj);
		emit accountRemoved (accObj);
		accObj->deleteLater ();
		saveAccounts ();
	}

	void IrcProtocol::saveAccounts () const
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Acetamide_Accounts");
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

	void IrcProtocol::RestoreAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Acetamide_Accounts");
		int size = settings.beginReadArray ("Accounts");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			QByteArray data = settings.value ("SerializedData").toByteArray ();
			
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

			Accounts_ << acc;

			emit accountAdded (acc);
		}
	}

};
};
};
