/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <QSslSocket>
#include <util/util.h>
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	Callbacks::Callbacks (QObject *parent)
	: QObject (parent)
	{
	}

	void Callbacks::SetNotificationServerConnection (MSN::NotificationServerConnection *conn)
	{
		Conn_ = conn;
	}

	namespace
	{
		int Sock2Num (void *sock)
		{
			return reinterpret_cast<long> (sock);
		}
	}

	void Callbacks::registerSocket (void *sock, int read, int write, bool)
	{
		int num = Sock2Num (sock);

		auto create = [&] (QSocketNotifier::Type t)
			{
				auto s = new QSocketNotifier (num, t);
				Notifiers_ [sock] [t].reset (s);
				connect (s,
						SIGNAL (activated (int)),
						this,
						SLOT (handleSocketActivated (int)));
			};

		if (read)
			create (QSocketNotifier::Read);
		if (write)
			create (QSocketNotifier::Write);
	}

	void Callbacks::unregisterSocket (void *sock)
	{
		Notifiers_.remove (sock);
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

	void Callbacks::buddyChangedStatus (MSN::NotificationServerConnection *conn, MSN::Passport buddy,
			std::string friendlyname, MSN::BuddyStatus state, unsigned int clientID, std::string msnobject)
	{

	}

	void Callbacks::buddyOffline (MSN::NotificationServerConnection *conn, MSN::Passport buddy)
	{

	}

	void Callbacks::log (int writing, const char *buf)
	{
		qDebug () << "[MSN]" << (writing ? "->" : "<-") << buf;
	}

	void Callbacks::gotFriendlyName (MSN::NotificationServerConnection *conn, std::string friendlyname)
	{
	}

	void Callbacks::gotBuddyListInfo (MSN::NotificationServerConnection *conn, MSN::ListSyncInfo *data)
	{

	}

	void Callbacks::buddyChangedPersonalInfo (MSN::NotificationServerConnection *conn, MSN::Passport fromPassport, MSN::personalInfo pInfo)
	{

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

	void Callbacks::addedListEntry (MSN::NotificationServerConnection *conn, MSN::ContactList list, MSN::Passport buddy, std::string friendlyname)
	{

	}

	void Callbacks::removedListEntry (MSN::NotificationServerConnection *conn, MSN::ContactList list, MSN::Passport buddy)
	{

	}

	void Callbacks::addedGroup (MSN::NotificationServerConnection *conn, bool added, std::string groupName, std::string groupId)
	{

	}

	void Callbacks::removedGroup (MSN::NotificationServerConnection *conn, bool removed, std::string groupId)
	{

	}

	void Callbacks::renamedGroup (MSN::NotificationServerConnection *conn, bool renamed, std::string newGroupName, std::string groupId)
	{

	}

	void Callbacks::handleSocketActivated (int socket)
	{
		if (!Conn_)
			return;

		auto notifier = qobject_cast<QSocketNotifier*> (sender ());
		if (!notifier)
			return;

		auto c = Conn_->connectionWithSocket (reinterpret_cast<void*> (socket));
		if (!c)
			return;

		if (notifier->type () == QSocketNotifier::Read)
			c->dataArrivedOnSocket ();
		else if (notifier->type () == QSocketNotifier::Write)
			c->socketIsWritable ();
	}

	void* Callbacks::connectToServer (std::string server, int port, bool *connected, bool isSSL)
	{
		QTcpSocket *sock = 0;
		if (isSSL)
			sock = new QSslSocket (this);
		else
			sock = new QTcpSocket (this);

		sock->connectToHost (QString::fromUtf8 (server.c_str ()), port);

		*connected = true;

		void *res = reinterpret_cast<void*> (sock->socketDescriptor ());
		Sockets_ [res] = sock;
		return res;
	}

	int Callbacks::getSocketFileDescriptor (void *sock)
	{
		return Sock2Num (sock);
	}

	size_t Callbacks::getDataFromSocket (void *sock, char *data, size_t size)
	{
		return Sockets_ [sock]->read (data, size);
	}

	size_t Callbacks::writeDataToSocket (void *sock, char *data, size_t size)
	{
		return Sockets_ [sock]->write (data, size);
	}

	void Callbacks::gotInboxUrl (MSN::NotificationServerConnection*, MSN::hotmailInfo)
	{
		// TODO
	}
}
}
}
