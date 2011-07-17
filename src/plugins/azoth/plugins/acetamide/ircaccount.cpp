/**********************************************************************
* LeechCraft - modular cross-platform feature rich internet client.
* Copyright (C) 2010-2011 Oleg Linkin
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
#include "channelclentry.h"
#include "clientconnection.h"
#include "core.h"
#include "ircaccountconfigurationdialog.h"
#include "ircaccountconfigurationwidget.h"
#include "ircprotocol.h"
#include "ircserverclentry.h"
#include "xmlsettingsmanager.h"
#include "ircserverhandler.h"

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
	, IrcAccountState_ (SOffline)
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

		connect (ClientConnection_.get (),
				SIGNAL (gotConsoleLog (const QByteArray&, int)),
				this,
				SIGNAL (gotConsolePacket (const QByteArray&, int)));
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
		return "R!";
	}

	QString IrcAccount::GetUserName () const
	{
		return UserName_;
	}

	QString IrcAccount::GetRealName () const
	{
		return RealName_;
	}

	QStringList IrcAccount::GetNickNames () const
	{
		return NickNames_;
	}

	boost::shared_ptr<ClientConnection>
			IrcAccount::GetClientConnection () const
	{
		return ClientConnection_;
	}

	void IrcAccount::RenameAccount (const QString& name)
	{
		AccountName_ = name;
	}

	QByteArray IrcAccount::GetAccountID () const
	{
		return AccountID_;
	}

	void IrcAccount::SetAccountID (const QString& id)
	{
		AccountID_ = id.toUtf8 ();
	}
	
	QList<QAction*> IrcAccount::GetActions () const
	{
		return QList<QAction*> ();
	}

	void IrcAccount::OpenConfigurationDialog ()
	{
		std::auto_ptr<IrcAccountConfigurationDialog> dia (new
				IrcAccountConfigurationDialog (0));

		if (!RealName_.isEmpty ())
			dia->ConfWidget ()->SetRealName (RealName_);
		if (!UserName_.isEmpty ())
			dia->ConfWidget ()->SetUserName (UserName_);
		if (!NickNames_.isEmpty ())
			dia->ConfWidget ()->SetNickNames (NickNames_);

		if (dia->exec () == QDialog::Rejected)
			return;

		FillSettings (dia->ConfWidget ());
	}

	void IrcAccount::FillSettings (IrcAccountConfigurationWidget *widget)
	{
		State lastState = IrcAccountState_;
		if (lastState != SOffline &&
			(RealName_ != widget->GetRealName () ||
			UserName_ != widget->GetUserName () ||
			NickNames_ != widget->GetNickNames ()))
		{
			ChangeState (EntryStatus (SOffline, QString  ()));
		}

		RealName_ = widget->GetRealName ();
		UserName_ = widget->GetUserName ();
		NickNames_ = widget->GetNickNames ();

		if (lastState != SOffline)
			ChangeState (EntryStatus (lastState, QString ()));

		emit accountSettingsChanged ();
	}

	void IrcAccount::JoinServer (const ServerOptions& server,
			const ChannelOptions& channel)
	{
		QString serverId = server.ServerName_ + ":" +
				QString::number (server.ServerPort_);
		if (!ClientConnection_->IsServerExists (serverId))
		{
			ClientConnection_->JoinServer (server);
			ClientConnection_->GetIrcServerHandler (serverId)->
					Add2ChannelsQueue (channel);
		}
		else if (!channel.ChannelName_.isEmpty ())
			ClientConnection_->JoinChannel (server, channel);
	}

	EntryStatus IrcAccount::GetState () const
	{
		return EntryStatus (IrcAccountState_, QString ());
	}

	void IrcAccount::ChangeState (const EntryStatus& state)
	{
		if (IrcAccountState_ == SOffline &&
				!ClientConnection_)
			return;

		IrcAccountState_ = state.State_;
		if (state.State_ == SOffline)
			ClientConnection_->DisconnectFromAll ();

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

	void IrcAccount::RequestAuth (const QString&, const QString&,
			const QString&, const QStringList&)
	{
	}

	void IrcAccount::RemoveEntry (QObject*)
	{
	}

	QObject* IrcAccount::GetTransferManager () const
	{
		return 0;
	}

	IHaveConsole::PacketFormat IrcAccount::GetPacketFormat () const
	{
		return PFPlainText;
	}

	void IrcAccount::SetConsoleEnabled (bool enabled)
	{
		ClientConnection_->SetConsoleEnabled (enabled);
	}

	QByteArray IrcAccount::Serialize () const
	{
		quint16 version = 2;

		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << version
				<< AccountName_
				<< AccountID_
				<< RealName_
				<< UserName_
				<< NickNames_;
		}

		return result;
	}

	IrcAccount* IrcAccount::Deserialize (const QByteArray& data,
			QObject *parent)
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
		in >> result->AccountID_
			>> result->RealName_
			>> result->UserName_
			>> result->NickNames_;

		result->Init ();

		return result;
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
	}
};
};
};
