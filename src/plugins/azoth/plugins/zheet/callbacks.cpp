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

#include "callbacks.h"
#include <algorithm>
#include <QSslSocket>
#include <QTcpServer>
#include <util/util.h>
#include "core.h"
#include "msnaccount.h"
#include "zheetutil.h"
#include "msnbuddyentry.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	Callbacks::Callbacks (MSNAccount *parent)
	: QObject (parent)
	, Account_ (parent)
	{
	}

	void Callbacks::SetNotificationServerConnection (MSN::NotificationServerConnection *conn)
	{
		Conn_ = conn;

		if (LogFile_.isOpen ())
			LogFile_.close ();

		try
		{
			const auto& dir = Util::CreateIfNotExists ("azoth/zheet/");
			LogFile_.setFileName (dir.filePath (Account_->GetAccountID ()));
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to create logdir:"
					<< e.what ();
			return;
		}

		LogFile_.open (QIODevice::WriteOnly | QIODevice::Append);
		LogFile_.write (QDateTime::currentDateTime ().toString (Qt::ISODate).toUtf8 ());
		LogFile_.write (": reopened log file\n");
		LogFile_.flush ();
	}

	void Callbacks::registerSocket (void *sock, int read, int write, bool)
	{
		auto sockObj = Sockets_ [sock];
		if (read)
			connect (sockObj,
					SIGNAL (readyRead ()),
					this,
					SLOT (handleSocketRead ()));
		if (write)
			connect (sockObj,
					SIGNAL (bytesWritten (qint64)),
					this,
					SLOT (handleSocketWrite ()));
	}

	void Callbacks::unregisterSocket (void *sock)
	{
		disconnect (Sockets_ [sock],
				0,
				this,
				SLOT (handleSocketRead ()));
		disconnect (Sockets_ [sock],
				0,
				this,
				SLOT (handleSocketWrite ()));
	}

	void Callbacks::closeSocket (void *sock)
	{
		Sockets_ [sock]->close ();
		delete Sockets_.take (sock);
	}

	void Callbacks::showError (MSN::Connection *conn, std::string msg)
	{
		auto name = conn->myNotificationServer ()->myDisplayName;

		const QString& str = tr ("Error for MSN account %1: %2.")
				.arg (QString::fromUtf8 (name.c_str ()))
				.arg (QString::fromUtf8 (msg.c_str ()));
		const Entity& e = Util::MakeNotification ("MSN Error", str, PWarning_);
		Core::Instance ().SendEntity (e);
	}

	void Callbacks::buddyChangedStatus (MSN::NotificationServerConnection*, MSN::Passport buddy,
			std::string friendlyname, MSN::BuddyStatus state, unsigned int, std::string msnobject)
	{
		qDebug () << Q_FUNC_INFO << buddy.c_str () << state << msnobject.c_str ();

		const auto& id = ZheetUtil::FromStd (buddy);
		emit buddyChangedStatus (id, ZheetUtil::FromMSNState (state));
		emit buddyUpdatedName (id, ZheetUtil::FromStd (friendlyname));
	}

	void Callbacks::buddyOffline (MSN::NotificationServerConnection *conn, MSN::Passport buddy)
	{
		qDebug () << Q_FUNC_INFO << buddy.c_str ();

		emit buddyChangedStatus (ZheetUtil::FromStd (buddy), SOffline);
	}

	void Callbacks::log (int writing, const char *buf)
	{
		LogFile_.write (QDateTime::currentDateTime ().toString (Qt::ISODate).toUtf8 () + ": ");
		LogFile_.write ((writing ? "OUT " : "IN ") + QByteArray (buf));
		LogFile_.write ("\n");
		LogFile_.flush ();
	}

	void Callbacks::gotFriendlyName (MSN::NotificationServerConnection *conn, std::string friendlyname)
	{
		emit gotOurFriendlyName (ZheetUtil::FromStd (friendlyname));
	}

	void Callbacks::gotBuddyListInfo (MSN::NotificationServerConnection *conn, MSN::ListSyncInfo *data)
	{
		qDebug () << Q_FUNC_INFO;

		QList<MSN::Group> groups;
		std::for_each (data->groups.begin (), data->groups.end (),
				[&groups] (decltype (*data->groups.cbegin ()) pair)
					{ groups << pair.second; });

		const auto& cl = data->contactList;

		QList<MSN::Buddy*> buddies;

		std::map<std::string, int> allContacts;
		std::for_each (cl.begin (), cl.end (),
				[&allContacts, &buddies] (decltype (*cl.cbegin ()) pair)
					{
						allContacts [pair.first] = pair.second->lists & (MSN::LST_AB | MSN::LST_AL | MSN::LST_BL);
						buddies << pair.second;
					});

		emit gotGroups (groups);
		emit gotBuddies (buddies);

		conn->completeConnection (allContacts, data);

		emit finishedConnecting ();
	}

	void Callbacks::buddyChangedPersonalInfo (MSN::NotificationServerConnection *conn,
			MSN::Passport fromPassport, MSN::personalInfo pInfo)
	{
		emit buddyChangedStatusText (ZheetUtil::FromStd (fromPassport),
				ZheetUtil::FromStd (pInfo.PSM));
	}

	void Callbacks::gotLatestListSerial (MSN::NotificationServerConnection *conn, std::string lastChange)
	{
	}

	void Callbacks::gotGTC (MSN::NotificationServerConnection *conn, char c)
	{
	}

	void Callbacks::gotBLP (MSN::NotificationServerConnection *conn, char c)
	{
	}

	void Callbacks::addedListEntry (MSN::NotificationServerConnection *conn,
			MSN::ContactList list, MSN::Passport pass, std::string friendlyname)
	{
		qDebug () << Q_FUNC_INFO << pass.c_str () << friendlyname.c_str () << list;

		MSN::Buddy buddy (pass, friendlyname);

		QList<MSN::Buddy*> res;
		res << &buddy;
		emit gotBuddies (res);
	}

	void Callbacks::removedListEntry (MSN::NotificationServerConnection *conn,
			MSN::ContactList list, MSN::Passport buddy)
	{
		emit removedBuddy (list, ZheetUtil::FromStd (buddy));
	}

	void Callbacks::addedGroup (MSN::NotificationServerConnection *conn,
			bool added, std::string groupName, std::string groupId)
	{
		emit gotGroups (QList<MSN::Group> () << MSN::Group (groupId, groupName));
	}

	void Callbacks::removedGroup (MSN::NotificationServerConnection *conn,
			bool removed, std::string groupId)
	{
		emit removedGroup (ZheetUtil::FromStd (groupId));
	}

	void Callbacks::renamedGroup (MSN::NotificationServerConnection *conn,
			bool renamed, std::string newGroupName, std::string groupId)
	{
		emit renamedGroup (ZheetUtil::FromStd (groupId), ZheetUtil::FromStd (newGroupName));
	}

	void Callbacks::addedContactToGroup (MSN::NotificationServerConnection *conn,
			bool added, std::string groupId, std::string contactId)
	{
		qDebug () << Q_FUNC_INFO << added;
		if (added)
			emit buddyAddedToGroup (ZheetUtil::FromStd (contactId), ZheetUtil::FromStd (groupId));
	}

	void Callbacks::removedContactFromGroup (MSN::NotificationServerConnection *conn,
			bool removed, std::string groupId, std::string contactId)
	{
		qDebug () << Q_FUNC_INFO << removed;
		if (removed)
			emit buddyRemovedFromGroup (ZheetUtil::FromStd (contactId), ZheetUtil::FromStd (groupId));
	}

	void Callbacks::addedContactToAddressBook (MSN::NotificationServerConnection *conn,
			bool added, std::string passport, std::string displayName, std::string guid)
	{
		qDebug () << Q_FUNC_INFO << added << passport.c_str () << displayName.c_str () << guid.c_str ();
	}

	void Callbacks::removedContactFromAddressBook (MSN::NotificationServerConnection *conn,
			bool removed, std::string contactId, std::string passport)
	{
		qDebug () << Q_FUNC_INFO << removed << ZheetUtil::FromStd (contactId) << ZheetUtil::FromStd (passport);
		if (removed)
			emit removedBuddy (ZheetUtil::FromStd (contactId), ZheetUtil::FromStd (passport));
	}

	void Callbacks::enabledContactOnAddressBook (MSN::NotificationServerConnection *conn,
			bool enabled, std::string contactId, std::string passport)
	{

	}

	void Callbacks::disabledContactOnAddressBook (MSN::NotificationServerConnection *conn,
			bool disabled, std::string contactId)
	{

	}

	void Callbacks::gotSwitchboard (MSN::SwitchboardServerConnection *conn, const void *tag)
	{
		if (!tag)
			return;

		emit gotSB (conn, static_cast<const MSNBuddyEntry*> (tag));
	}

	void Callbacks::buddyJoinedConversation (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy, std::string friendlyname, int is_initial)
	{
		if (conn->auth.tag)
			emit buddyJoinedSB (conn, static_cast<const MSNBuddyEntry*> (conn->auth.tag));
	}

	void Callbacks::buddyLeftConversation (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy)
	{
		if (conn->auth.tag)
			emit buddyLeftSB (conn, static_cast<const MSNBuddyEntry*> (conn->auth.tag));
	}

	void Callbacks::gotInstantMessage (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy, std::string friendlyname, MSN::Message *msg)
	{
		emit gotMessage (ZheetUtil::FromStd (buddy), msg);
	}

	void Callbacks::gotMessageSentACK (MSN::SwitchboardServerConnection *conn, int trID)
	{
		emit messageDelivered (trID);
	}

	void Callbacks::gotEmoticonNotification (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy, std::string alias, std::string)
	{

	}

	void Callbacks::failedSendingMessage (MSN::Connection*)
	{
	}

	void Callbacks::gotNudge (MSN::SwitchboardServerConnection *conn, MSN::Passport username)
	{
		emit gotNudge (ZheetUtil::FromStd (username));
	}

	void Callbacks::gotVoiceClipNotification (MSN::SwitchboardServerConnection *conn, MSN::Passport, std::string)
	{
	}

	void Callbacks::gotWinkNotification (MSN::SwitchboardServerConnection *conn, MSN::Passport, std::string)
	{
	}

	void Callbacks::gotInk (MSN::SwitchboardServerConnection *conn, MSN::Passport username, std::string image)
	{
	}

	void Callbacks::gotActionMessage (MSN::SwitchboardServerConnection *conn, MSN::Passport username, std::string message)
	{
		qDebug () << Q_FUNC_INFO << ZheetUtil::FromStd (username) << ZheetUtil::FromStd (message);
	}

	void Callbacks::buddyTyping (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy, std::string friendlyname)
	{
	}

	void Callbacks::gotInitialEmailNotification (MSN::NotificationServerConnection *conn, int msgs_inbox, int unread_inbox, int msgs_folders, int unread_folders)
	{
		emit initialEmailNotification (msgs_inbox, unread_inbox);
	}

	void Callbacks::gotNewEmailNotification (MSN::NotificationServerConnection *conn, std::string from, std::string subject)
	{
		emit newEmailNotification (ZheetUtil::FromStd (from), ZheetUtil::FromStd (subject));
	}

	void Callbacks::fileTransferProgress (MSN::SwitchboardServerConnection *conn,
			unsigned int sessionID, long long unsigned int transferred, long long unsigned int total)
	{
		emit fileTransferProgress (sessionID, transferred, total);
	}

	void Callbacks::fileTransferFailed (MSN::SwitchboardServerConnection *conn,
			unsigned int sessionID, MSN::fileTransferError error)
	{
		emit fileTransferFailed (sessionID);
	}

	void Callbacks::fileTransferSucceeded (MSN::SwitchboardServerConnection *conn,
			unsigned int sessionID)
	{
		emit fileTransferFinished (sessionID);
	}

	void Callbacks::fileTransferInviteResponse (MSN::SwitchboardServerConnection *conn,
			unsigned int sessionID, bool response)
	{
		emit fileTransferGotResponse (sessionID, response);
	}

	void Callbacks::gotVoiceClipFile (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, std::string file)
	{
	}

	void Callbacks::gotEmoticonFile (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, std::string alias, std::string file)
	{
	}

	void Callbacks::gotWinkFile (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, std::string file)
	{
	}

	void Callbacks::gotNewConnection (MSN::Connection *conn)
	{
		auto nsc = dynamic_cast<MSN::NotificationServerConnection*> (conn);
		if (nsc)
			nsc->synchronizeContactList ();
	}

	void Callbacks::gotOIMList (MSN::NotificationServerConnection *conn, std::vector<MSN::eachOIM> OIMs)
	{
	}

	void Callbacks::gotOIM (MSN::NotificationServerConnection *conn, bool success, std::string id, std::string message)
	{
	}

	void Callbacks::gotOIMSendConfirmation (MSN::NotificationServerConnection *conn, bool success, int id)
	{
	}

	void Callbacks::gotOIMDeleteConfirmation (MSN::NotificationServerConnection *conn, bool success, std::string id)
	{
	}

	void Callbacks::gotContactDisplayPicture (MSN::SwitchboardServerConnection *conn, MSN::Passport passport, std::string filename)
	{

	}

	void Callbacks::connectionReady (MSN::Connection*)
	{
	}

	void Callbacks::closingConnection (MSN::Connection*)
	{
	}

	void Callbacks::changedStatus (MSN::NotificationServerConnection *conn, MSN::BuddyStatus state)
	{
		emit weChangedState (ZheetUtil::FromMSNState (state));
	}

	void* Callbacks::connectToServer (std::string server, int port, bool *connected, bool isSSL)
	{
		const QString& servStr = QString::fromUtf8 (server.c_str ());
		qDebug () << Q_FUNC_INFO << servStr << port << isSSL;
		QTcpSocket *sock = 0;
		if (isSSL)
		{
			auto ssl = new QSslSocket (this);
			connect (ssl,
					SIGNAL (sslErrors (QList<QSslError>)),
					ssl,
					SLOT (ignoreSslErrors ()));
			ssl->connectToHostEncrypted (servStr, port);
			sock = ssl;
		}
		else
		{
			sock = new QTcpSocket (this);
			sock->connectToHost (servStr, port);
		}

		connect (sock,
				SIGNAL (connected ()),
				this,
				SLOT (handleSocketConnected ()));

		*connected = false;

		Sockets_ [sock] = sock;
		return sock;
	}

	void Callbacks::askFileTransfer (MSN::SwitchboardServerConnection *conn, MSN::fileTransferInvite ft)
	{
		emit fileTransferSuggested (ft);
	}

	int Callbacks::listenOnPort (int port)
	{
		QTcpServer *serv = new QTcpServer (this);
		serv->listen (QHostAddress::Any, port);
		return serv->socketDescriptor ();
	}

	std::string Callbacks::getOurIP ()
	{
		// TODO
		return "127.0.0.1";
	}

	std::string Callbacks::getSecureHTTPProxy ()
	{
		return "";
	}

	int Callbacks::getSocketFileDescriptor (void *sock)
	{
		return Sockets_ [sock]->socketDescriptor ();
	}

	size_t Callbacks::getDataFromSocket (void *sock, char *data, size_t size)
	{
		return Sockets_ [sock]->read (data, size);
	}

	size_t Callbacks::writeDataToSocket (void *sock, char *data, size_t size)
	{
		auto res = Sockets_ [sock]->write (data, size);
		Sockets_ [sock]->flush ();
		return res;
	}

	void Callbacks::gotInboxUrl (MSN::NotificationServerConnection*, MSN::hotmailInfo)
	{
		// TODO
	}

	void Callbacks::handleSocketRead ()
	{
		auto c = Conn_->connectionWithSocket (sender ());
		if (!c)
			return;

		c->dataArrivedOnSocket ();
	}

	void Callbacks::handleSocketWrite ()
	{
		auto c = Conn_->connectionWithSocket (sender ());
		if (!c)
			return;

		c->socketIsWritable ();
	}

	void Callbacks::handleSocketConnected ()
	{
		auto c = Conn_->connectionWithSocket (sender ());
		if (!c)
			return;

		c->socketConnectionCompleted ();
	}
}
}
}
