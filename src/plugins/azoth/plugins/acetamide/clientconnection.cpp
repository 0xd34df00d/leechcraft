/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "clientconnection.h"
#include <QTextCodec>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/iproxyobject.h>
#include "channelclentry.h"
#include "channelhandler.h"
#include "ircprotocol.h"
#include "ircserverclentry.h"
#include "ircserverhandler.h"
#include "xmlsettingsmanager.h"

namespace LC::Azoth::Acetamide
{
	ClientConnection::ClientConnection (IrcAccount *account)
	: Account_ { account }
	{
	}

	ClientConnection::~ClientConnection ()
	{
		qDeleteAll (ServerHandlers_);
	}

	IrcAccount* ClientConnection::GetAccount () const
	{
		return Account_;
	}

	QList<IrcServerHandler*> ClientConnection::GetServerHandlers () const
	{
		return ServerHandlers_.values ();
	}

	QList<QObject*> ClientConnection::GetCLEntries () const
	{
		QList<QObject*> result;
		for (const auto ish : ServerHandlers_)
		{
			result << ish->GetCLEntry ();
			result << ish->GetCLEntries ();
		}
		return result;
	}
	
	namespace
	{
		QString GetServerKey (const ServerOptions& server)
		{
			return server.ServerName_ + ":" + QString::number (server.ServerPort_);
		}
	}

	bool ClientConnection::IsServerExists (const ServerOptions& server) const
	{
		return ServerHandlers_.contains (GetServerKey (server));
	}

	IrcServerHandler& ClientConnection::JoinServer (const ServerOptions& server)
	{
		const auto& serverId = GetServerKey (server);
		if (const auto ish = ServerHandlers_.value (serverId))
		{
			qWarning () << Q_FUNC_INFO
					<< "server"
					<< serverId
					<< "is already present";
			return *ish;
		}

		const auto ish = new IrcServerHandler (server, Account_);
		emit gotRosterItems ({ ish->GetCLEntry () });

		ish->SetConsoleEnabled (IsConsoleEnabled_);
		if (IsConsoleEnabled_)
			connect (ish,
					SIGNAL (sendMessageToConsole (IMessage::Direction, const QString&)),
					this,
					SLOT (handleLog (IMessage::Direction, const QString&)),
					Qt::UniqueConnection);
		ServerHandlers_ [serverId] = ish;

		ish->ConnectToServer ();

		return *ish;
	}

	void ClientConnection::JoinChannel (const ServerOptions& server,
			const ChannelOptions& channel)
	{
		const auto& serverId = GetServerKey (server);
		const auto& channelId = channel.ChannelName_ + "@" + channel.ServerName_;

		auto ish = ServerHandlers_.value (serverId);
		if (!ish)
			ish = &JoinServer (server);

		if (ish->IsChannelExists (channelId))
		{
			const auto& e = Util::MakeNotification (Lits::AzothAcetamide,
					tr ("This channel is already joined."),
					Priority::Warning);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity(e);
			return;
		}

		if (!channel.ChannelName_.isEmpty ())
			ish->JoinChannel (channel);
	}

	void ClientConnection::SetBookmarks (const QList<IrcBookmark>& bookmarks)
	{
		QList<QVariant> res;
		for (const auto& bookmark : bookmarks)
		{
			QByteArray result;
			{
				QDataStream ostr (&result, QIODevice::WriteOnly);
				ostr << static_cast<quint8> (1)
						<< bookmark.Name_
						<< bookmark.ServerName_
						<< bookmark.ServerPort_
						<< bookmark.ServerPassword_
						<< bookmark.ServerEncoding_
						<< bookmark.ChannelName_
						<< bookmark.ChannelPassword_
						<< bookmark.NickName_
						<< bookmark.SSL_
						<< bookmark.AutoJoin_;
			}

			res << QVariant::fromValue (result);
		}
		XmlSettingsManager::Instance ().setProperty ("Bookmarks",
				QVariant::fromValue (res));
	}

	QList<IrcBookmark> ClientConnection::GetBookmarks () const
	{
		const auto& list = XmlSettingsManager::Instance ().Property ("Bookmarks",
				QList<QVariant> ()).toList ();

		bool hadUnknownVersions = false;

		QList<IrcBookmark> bookmarks;
		for (const auto& variant : list)
		{
			IrcBookmark bookmark;
			QDataStream istr (variant.toByteArray ());

			quint8 version = 0;
			istr >> version;
			if (version != 1)
			{
				qWarning () << Q_FUNC_INFO
						<< "unknown version"
						<< version;
				hadUnknownVersions = true;
				continue;
			}

			istr >> bookmark.Name_
					>> bookmark.ServerName_
					>> bookmark.ServerPort_
					>> bookmark.ServerPassword_
					>> bookmark.ServerEncoding_
					>> bookmark.ChannelName_
					>> bookmark.ChannelPassword_
					>> bookmark.NickName_
					>> bookmark.SSL_
					>> bookmark.AutoJoin_;

			bookmarks << bookmark;
		}

		if (hadUnknownVersions)
		{
			const auto& entity = Util::MakeNotification (Lits::AzothAcetamide,
					tr ("Some bookmarks were lost due to unknown storage version."),
					Priority::Warning);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (entity);
		}

		return bookmarks;
	}

	IrcServerHandler* ClientConnection::GetIrcServerHandler (const ServerOptions& server) const
	{
		return GetIrcServerHandler (GetServerKey (server));
	}

	IrcServerHandler* ClientConnection::GetIrcServerHandler (const QString& serverId) const
	{
		return ServerHandlers_ [serverId];
	}

	void ClientConnection::DisconnectFromAll ()
	{
		// we need to make a copy here since `SendQuit()` might cause the handler
		// to be removed synchronously
		const auto& values = ServerHandlers_.values ();
		for (auto ish : values)
			ish->SendQuit ();
	}

	void ClientConnection::QuitServer (const QStringList& list)
	{
		auto ish = ServerHandlers_ [list.last ()];
		ish->DisconnectFromServer ();
	}

	void ClientConnection::SetConsoleEnabled (bool enabled)
	{
		IsConsoleEnabled_ = enabled;
		for (auto srv : ServerHandlers_)
		{
			srv->SetConsoleEnabled (enabled);
			if (enabled)
				connect (srv,
						SIGNAL (sendMessageToConsole (IMessage::Direction, const QString&)),
						this,
						SLOT (handleLog (IMessage::Direction, const QString&)),
						Qt::UniqueConnection);
			else
				disconnect (srv,
						SIGNAL (sendMessageToConsole (IMessage::Direction, const QString&)),
						this,
						SLOT (handleLog (IMessage::Direction, const QString&)));
		}
	}

	void ClientConnection::ClosePrivateChat (const QString& serverID, QString nick)
	{
		if (const auto ish = ServerHandlers_.value (serverID))
			ish->ClosePrivateChat (nick);
	}

	void ClientConnection::FetchVCard (const QString& serverId, const QString& nick)
	{
		if (const auto ish = ServerHandlers_.value (serverId))
			ish->VCardRequest (nick);
	}

	void ClientConnection::SetAway (bool away, const QString& message)
	{
		QString msg = message;
		if (msg.isEmpty ())
			msg = GetStatusStringForState (SAway);

		if (!away)
			msg.clear ();

		for (const auto& handler : ServerHandlers_)
			handler->SetAway (msg);
	}

	QString ClientConnection::GetStatusStringForState (State state)
	{
		const QString& statusKey = "DefaultStatus" + QString::number (state);
		return Account_->GetParentProtocol ()->GetProxyObject ()->GetSettingsManager ()->
				property (statusKey.toUtf8 ()).toString ();
	}

	void ClientConnection::serverConnected (const QString&)
	{
		if (Account_->GetState ().State_ == SOffline)
		{
			Account_->ChangeState (EntryStatus (SOnline, QString ()));
			Account_->SetState (EntryStatus (SOnline, QString ()));
		}
	}

	void ClientConnection::serverDisconnected (const QString& serverId)
	{
		const auto entry = ServerHandlers_.take (serverId);
		if (!entry)
			return;

		Account_->handleEntryRemoved (entry->GetCLEntry ());
		entry->DisconnectFromServer ();
		entry->deleteLater ();

		if (ServerHandlers_.isEmpty ())
			Account_->SetState (EntryStatus (SOffline,
					QString ()));
	}

	void ClientConnection::handleLog (IMessage::Direction type, const QString& msg)
	{
		switch (type)
		{
		case IMessage::Direction::Out:
			emit gotConsoleLog (msg.toUtf8 (), IHaveConsole::PacketDirection::Out, {});
			break;
		case IMessage::Direction::In:
			emit gotConsoleLog (msg.toUtf8 (), IHaveConsole::PacketDirection::In, {});
			break;
		}
	}
}
