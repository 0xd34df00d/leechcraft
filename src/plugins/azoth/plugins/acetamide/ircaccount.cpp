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
#include <QSettings>
#include <interfaces/iprotocol.h>
#include <interfaces/iproxyobject.h>
#include "ircprotocol.h"
#include "ircaccountconfigurationdialog.h"
#include "core.h"

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
	, Nick_ (QString ())
	{
		IrcAccountState_ = SOffline;
		
		connect (this,
				SIGNAL (scheduleClientDestruction ()),
				this,
				SLOT (handleDestroyClient ()),
				Qt::QueuedConnection);
		
		ServersInfo_ = ReadConnectionSettings (Name_ + "_Servers");
		NickNames_ = ReadNicknameSettings (Name_ +"_Nicknames");
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
		if (NickNames_.at (0).Nicks_.at (0).isEmpty ())
			return Core::Instance ().GetDefaultIrcAccount ()->GetNickNames ().at (0).Nicks_.at (0);
		
		return NickNames_.at (0).Nicks_.at (0);
	}

	QList<NickNameData> IrcAccount::GetNickNames () const
	{
		return NickNames_;
	}

	QList<ServerInfoData> IrcAccount::GetServersInfo () const
	{
		return ServersInfo_;
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

		if (!NickNames_.isEmpty ())
			dia->SetNicks (NickNames_);
		if (!ServersInfo_.isEmpty ())
			dia->SetServersInfo (ServersInfo_);

		if (dia->exec () == QDialog::Rejected)
			return;

		Nick_ = dia->GetDefaultNickname ();
		
		State lastState = IrcAccountState_;
		if (lastState != SOffline)
		{
			ChangeState (EntryStatus (SOffline, QString ()));
// 			ClientConnection_->SetOurJID (dia->GetJID () + "/" + dia->GetResource ());
		}

		SaveConnectionSettings (dia->GetServersInfo (), QString (Name_ + "_Servers"));
		SaveNicknameSettings (dia->GetNicks (), QString (Name_ + "_Nicknames"));
		
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
				<< Name_;
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
		result->Init ();

		return result;
	}

	void IrcAccount::SaveConnectionSettings (const QList<ServerInfoData>& value, const QString& name)
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Acetamide");
		settings.beginGroup ("ConnectionSettings");
		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			Q_FOREACH (const ServerInfoData& serv, value)
				ostr << serv;
		}
		
		settings.setValue (name, result);
		settings.endGroup ();
	}
	
	QList<ServerInfoData> IrcAccount::ReadConnectionSettings (const QString& name)
	{
		QList<ServerInfoData> value;
		ServerInfoData val;
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Acetamide");
		settings.beginGroup ("ConnectionSettings");
		
		QByteArray ba = settings.value (name).toByteArray ();
		QDataStream istr (ba);
		
		while (!istr.atEnd ())
		{
			istr >> val;
			value << val;
		}
		
		settings.endGroup ();
		
		return value;
	}
	
	void IrcAccount::SaveNicknameSettings (const QList<NickNameData>& value, const QString& name)
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Acetamide");
		settings.beginGroup ("NicknameSettings");
		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			Q_FOREACH (const NickNameData& nick, value)
				ostr << nick;
		}
		settings.setValue (name, result);
		settings.endGroup ();
	}
	
	QList<NickNameData> IrcAccount::ReadNicknameSettings (const QString& name)
	{
		QList<NickNameData> value;
		NickNameData val;
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Acetamide");
		settings.beginGroup ("NicknameSettings");
		
		QByteArray ba = settings.value (name).toByteArray ();
		QDataStream istr (ba);
		
		while (!istr.atEnd ())
		{
			istr >> val;
			value << val;
		}
		
		settings.endGroup ();
		
		return value;
	}
	
	QDataStream& operator<< (QDataStream& out, const NickNameData& nick)
	{
		out << static_cast<quint8> (1)
				<< nick.Server_
				<< nick.ServerName_
				<< nick.Nicks_
				<< nick.AutoGenerate_;
		
		return out;
	}

	QDataStream& operator>> (QDataStream& in, NickNameData& nick)
	{
		quint8 version = 0;
		
		in >> version;
		if (version != 1)
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
		
		in >> nick.Server_
				>> nick.ServerName_
				>> nick.Nicks_
				>> nick.AutoGenerate_;
		
		return in;
	}

	QDataStream& operator<< (QDataStream& out, const ServerInfoData& server)
	{
		out << static_cast<quint8> (1)
				<< server.Network_
				<< server.Server_
				<< server.Port_
				<< server.Password_
				<< server.Channels_
				<< server.SSL_;
		
		return out;
	}

	QDataStream& operator>> (QDataStream& in, ServerInfoData& server)
	{
		quint8 version = 0;
		in >> version;
		
		if (version != 1)
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
		
		in >> server.Network_
				>> server.Server_
				>> server.Port_
				>> server.Password_
				>> server.Channels_
				>> server.SSL_;
		
		return in;
	}
};
};
};
