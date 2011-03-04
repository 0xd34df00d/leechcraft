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
#include <boost/bind.hpp>
#include <QInputDialog>
#include <QSettings>
#include <interfaces/iprotocol.h>
#include <interfaces/iproxyobject.h>
#include "clientconnection.h"
#include "ircprotocol.h"
#include "ircaccountconfigurationdialog.h"
#include "channelclentry.h"
#include "core.h"
#include "ircmessage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcAccount::IrcAccount (const QString& name, QObject *parent)
	: QObject (parent)
	, AccountName_ (name)
	, ParentProtocol_ (qobject_cast<IrcProtocol*> (parent))
	, AccountID_ (QByteArray ())
	, IrcAccountState (SOffline)
	{
		connect (this,
				SIGNAL (scheduleClientDestruction ()),
				this,
				SLOT (handleDestroyClient ()),
				Qt::QueuedConnection);
		Init ();
	}

	void IrcAccount::Init ()
	{
		ClientConnection_.reset (new ClientConnection (this));
		Servers_ = ReadServersSettings (AccountName_ + "_Servers");
		Channels_ = ReadChannelsSettings (AccountName_ +"_Channels");
		SetAccountID ();

		connect (ClientConnection_.get (),
				SIGNAL (gotRosterItems (const QList<QObject*>&)),
				this,
				SLOT (handleGotRosterItems (const QList<QObject*>&)));
		connect (ClientConnection_.get (),
				SIGNAL (rosterItemRemoved (QObject*)),
				this,
				SLOT (handleEntryRemoved (QObject*)));
		connect (ClientConnection_.get (),
				SIGNAL (rosterItemsRemoved (const QList<QObject*>&)),
				this,
				SIGNAL (removedCLItems (const QList<QObject*>&)));
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
		return AccountName_;
	}

	QString IrcAccount::GetOurNick () const
	{
		QList<ServerOptions> srvs = Servers_;
		if (srvs.isEmpty ())
		{
			IrcAccount *def = Core::Instance ().GetDefaultIrcAccount ();
			srvs = ReadServersSettings (def->GetAccountName () + "_Servers");
		}
		
		return srvs.at (0).ServerNicknames_.at (0);
	}

	QList<ServerOptions> IrcAccount::GetServers () const
	{
		return Servers_;
	}

	QList<ChannelOptions> IrcAccount::GetChannels () const
	{
		return Channels_;
	}

	void IrcAccount::RenameAccount (const QString& name)
	{
		AccountName_ = name;
	}

	QByteArray IrcAccount::GetAccountID () const
	{
		return AccountID_;
	}

	void IrcAccount::OpenConfigurationDialog ()
	{
// 		std::auto_ptr<IrcAccountConfigurationDialog> dia (new IrcAccountConfigurationDialog (0));
// 
// 		if (!NickNames_.isEmpty ())
// 			dia->SetNicks (NickNames_);
// 		if (!ServersInfo_.isEmpty ())
// 			dia->SetServersInfo (ServersInfo_);
// 
// 		if (dia->exec () == QDialog::Rejected)
// 			return;
// 
// 		Nick_ = dia->GetDefaultNickname ();
// 		
// 		State lastState = IrcAccountState_;
// 		if (lastState != SOffline)
// 		{
// 			ChangeState (EntryStatus (SOffline, QString ()));
// // 			ClientConnection_->SetOurJID (dia->GetJID () + "/" + dia->GetResource ());
// 		}
// 
// 		NickNames_ = dia->GetNicks ();
// 		ServersInfo_ = dia->GetServersInfo ();
// 		
// 		SaveConnectionSettings (ServersInfo_, QString (Name_ + "_Servers"));
// 		SaveNicknameSettings (NickNames_, QString (Name_ + "_Nicknames"));
// 		
// 		if (lastState != SOffline)
// 			ChangeState (EntryStatus (lastState, QString ()));
// 
// 		emit accountSettingsChanged ();
	}

	EntryStatus IrcAccount::GetState () const
	{
		return EntryStatus (IrcAccountState, QString ());
	}

	void IrcAccount::ChangeState (const EntryStatus& state)
	{
		if (state.State_ == SOffline &&
				!ClientConnection_)
			return;

		IrcAccountState = state.State_;
		emit statusChanged (state);
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

	void IrcAccount::JoinRoom (const ServerOptions& opts, const ChannelOptions& info)
	{
		QString channelStr = QString ("%1@%2")
				.arg (info.ChannelName_, info.ServerName_);
		
		ChannelCLEntry *entry = ClientConnection_->JoinRoom (opts, info);
		if (!entry)
			return;
		
		emit gotCLItems (QList<QObject*> () << entry);
	}
	
	boost::shared_ptr< ClientConnection > IrcAccount::GetClientConnection () const
	{
		return ClientConnection_;
	}

	QObject* IrcAccount::CreateMessage (IMessage::MessageType type
			, const QString& resource
			, const QString& body)
	{
		return ClientConnection_->CreateMessage (type, resource, body);
	}

	void IrcAccount::SetAccountID ()
	{
		QList<QByteArray> accIDs = ParentProtocol_->GetRegisteredAccountsIDs ();
		bool found = true;
		QByteArray accID;
		
		while (found)
		{
			QString idStr = ParentProtocol_->GetProtocolID () + 
					"." + 
					GetAccountName () + 
					"." + 
					QString::number (qrand ());
			found = false;
			accID = idStr.toUtf8 ();
			Q_FOREACH (const QByteArray& b, accIDs)
				if (b == accID)
				{
					found = true;
					break;
				}
		}
		AccountID_ = accID;
	}


	QByteArray IrcAccount::Serialize () const
	{
		quint16 version = 2;

		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << version
				<< AccountName_;
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

		return result;
	}

	void IrcAccount::SaveServersSettings (const QList<ServerOptions>& value, const QString& name)
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Acetamide");
		settings.beginGroup ("Servers");
		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			Q_FOREACH (const ServerOptions& serv, value)
				ostr << serv;
		}
		
		settings.setValue (name, result);
		settings.endGroup ();
	}

	QList<ServerOptions> IrcAccount::ReadServersSettings (const QString& name) const
	{
		QList<ServerOptions> value;
		ServerOptions val;

		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Acetamide");
		settings.beginGroup ("Servers");
		
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

	void IrcAccount::SaveChannelsSettings (const QList<ChannelOptions>& value, const QString& name)
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Acetamide");
		settings.beginGroup ("Channels");
		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			Q_FOREACH (const ChannelOptions& nick, value)
				ostr << nick;
		}
		settings.setValue (name, result);
		settings.endGroup ();
	}
	
	QList<ChannelOptions> IrcAccount::ReadChannelsSettings (const QString& name) const
	{
		QList<ChannelOptions> value;
		ChannelOptions val;

		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Acetamide");
		settings.beginGroup ("Channels");
		
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

	void IrcAccount::handleEntryRemoved (QObject *entry)
	{
		emit removedCLItems (QObjectList () << entry);
	}

	void IrcAccount::handleGotRosterItems (const QList<QObject*>& items)
	{
		emit gotCLItems (items);
	}

	void IrcAccount::handleDestroyClient ()
	{
		ClientConnection_.reset ();
	}

	QDataStream& operator<< (QDataStream& out, const ServerOptions& server)
	{
		out << static_cast<quint8> (1)
				<< server.NetworkName_
				<< server.ServerName_
				<< server.ServerPort_
				<< server.ServerPassword_
				<< server.ServerNicknames_
				<< server.ServerRealName_
				<< server.ServerEncoding_
				<< server.SSL_;

		return out;
	}

	QDataStream& operator>> (QDataStream& in, ServerOptions& server)
	{
		quint8 version = 0;
		
		in >> version;
		if (version != 1)
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
		else
			in >> server.NetworkName_
				>> server.ServerName_
				>> server.ServerPort_
				>> server.ServerPassword_
				>> server.ServerNicknames_
				>> server.ServerRealName_
				>> server.ServerEncoding_
				>> server.SSL_;

		return in;
	}

	QDataStream& operator<< (QDataStream& out, const ChannelOptions& channel)
	{
		out << static_cast<quint8> (1)
				<< channel.ServerName_
				<< channel.ChannelName_
				<< channel.ChannelPassword_
				<< channel.ChannelNickname_;

		return out;
	}

	QDataStream& operator<< (QDataStream& in, ChannelOptions& channel)
	{
		quint8 version = 0;
		in >> version;
		
		if (version != 1)
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
		else
			in >> channel.ServerName_
				>> channel.ChannelName_
				>> channel.ChannelPassword_
				>> channel.ChannelNickname_;

		return in;
	}
};
};
};
