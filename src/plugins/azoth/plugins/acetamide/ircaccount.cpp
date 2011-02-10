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

#include "ircaccount.h"
#include <QInputDialog>
#include <QtDebug>
#include <interfaces/iprotocol.h>
#include <interfaces/iproxyobject.h>
#include "ircprotocol.h"
#include "ircaccountconfigurationdialog.h"
#include "core.h"
#include <QSettings>



namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcAccount::IrcAccount (const QString& name, QObject *parent)
	: QObject (parent)
	, Name_ (name)
	, ParentProtocol_ (qobject_cast<IrcProtocol*> (parent))
	{
		IrcAccountState_ = SOffline;
		
		connect (this,
				SIGNAL (scheduleClientDestruction ()),
				this,
				SLOT (handleDestroyClient ()),
				Qt::QueuedConnection);
	}

	void IrcAccount::Init ()
	{
		
	}

	QObject* IrcAccount::GetObject ()
	{
		return this;
	}
	
	QObject* IrcAccount::GetParentProtocol () const
	{
		return ParentProtocol_;
	}

	IAccount::AccountFeatures IrcAccount::GetAccountFeatures () const
	{
		return FRenamable | FMUCsSupportFileTransfers;
	}
	
	QList<QObject*> IrcAccount::GetCLEntries ()
	{
		return QList<QObject*> ();
	}

	void IrcAccount::QueryInfo (const QString&)
	{
	}
	
	QString IrcAccount::GetAccountName () const
	{
		return Name_;
	}

	QString IrcAccount::GetOurNick () const
	{
		return Nicks_;
	}

	void IrcAccount::RenameAccount (const QString& name)
	{
		Name_ = name;
	}

	QByteArray IrcAccount::GetAccountID () const
	{
		//TODO
		return QByteArray ();
	}

	void IrcAccount::OpenConfigurationDialog ()
	{
		std::auto_ptr<IrcAccountConfigurationDialog> dia (new IrcAccountConfigurationDialog (0));
		
		QList<QVariant> serversInfo = ReadConnectionSettings ("Servers");
// 		if (!Nicknames_.isEmpty ())
// 			dia->SetNicks (Nicknames_);
		if (!serversInfo.isEmpty ())
			dia->SetServersInfo (serversInfo);

		if (dia->exec () == QDialog::Rejected)
			return;
		
		State lastState = IrcAccountState_;
		if (lastState != SOffline)
		{
			ChangeState (EntryStatus (SOffline, QString ()));
// 			ClientConnection_->SetOurJID (dia->GetJID () + "/" + dia->GetResource ());
		}
		
		Nicknames_ = dia->GetNicks ();

		SaveConnectionSettings (dia->GetServersInfo (), QString ("Servers"));
		
		if (lastState != SOffline)
			ChangeState (EntryStatus (lastState, QString ()));

		emit accountSettingsChanged ();
	}

	EntryStatus IrcAccount::GetState () const
	{
		return EntryStatus (IrcAccountState_, QString ());
	}

	void IrcAccount::ChangeState (const EntryStatus&)
	{
	}
	
	void IrcAccount::Synchronize ()
	{
	}

	void IrcAccount::Authorize (QObject*)
	{
	}

	void IrcAccount::DenyAuth (QObject*)
	{
	}

	void IrcAccount::RequestAuth (const QString& , const QString& , const QString& , const QStringList&)
	{
	}

	void IrcAccount::RemoveEntry (QObject*)
	{
	}

	QObject* IrcAccount::GetTransferManager () const
	{
		return 0;
	}


	QByteArray IrcAccount::Serialize () const
	{
		quint16 version = 2;

		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << version
				<< Name_
				<< Nicknames_;
		}

		return result;
	}

	IrcAccount* IrcAccount::Deserialize (const QByteArray& data, QObject *parent)
	{
		quint16 version = 0;

		QDataStream in (data);
		in >> version;

		if (version < 1 || version > 2)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return 0;
		}

		QString name;
		in >> name;
		IrcAccount *result = new IrcAccount (name, parent);
		in >> result->Nicknames_;
		result->Init ();

		return result;
	}

	void IrcAccount::SaveConnectionSettings (const QList<QVariant>& value, const QString& name)
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Acetamide");
		settings.beginGroup ("ConnectionSettings");
		settings.setValue (name, value);
		settings.endGroup ();
	}
	
	QList<QVariant> IrcAccount::ReadConnectionSettings(const QString& name)
	{
		QList<QVariant> value;
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Acetamide");
		settings.beginGroup ("ConnectionSettings");
		value = settings.value (name).toList ();
		settings.endGroup ();
		
		return value;
	}

};
};
};