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

#ifndef PLUGINS_AZOTH_PLUGINS_ZHEET_CALLBACKS_H
#define PLUGINS_AZOTH_PLUGINS_ZHEET_CALLBACKS_H
#include <memory>
#include <QObject>
#include <QHash>
#include <QSet>
#include <QMap>
#include <QSocketNotifier>
#include <msn/connection.h>
#include <msn/notificationserver.h>
#include <msn/externals.h>

typedef std::shared_ptr<QSocketNotifier> QSocketNotifier_ptr;

class QTcpSocket;

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	class Callbacks : public QObject
					, public MSN::Callbacks
	{
		QHash<void*, QMap<QSocketNotifier::Type, QSocketNotifier_ptr>> Notifiers_;
		QHash<void*, QTcpSocket*> Sockets_;

		MSN::NotificationServerConnection *Conn_;
	public:
		void SetNotificationServerConnection (MSN::NotificationServerConnection*);

		void registerSocket (void *sock, int read, int write, bool isSSL);
		void unregisterSocket (void *sock);
		void closeSocket (void *sock);
		void* connectToServer (std::string server, int port, bool *connected, bool isSSL = false);
		int getSocketFileDescriptor (void *sock);
		size_t getDataFromSocket (void *sock, char *data, size_t size);
		size_t writeDataToSocket (void *sock, char *data, size_t size);
		void gotInboxUrl (MSN::NotificationServerConnection*, MSN::hotmailInfo);
	private slots:
		void handleSocketActivated (int);
	};
}
}
}

#endif
