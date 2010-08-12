/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef PLUGINS_SYNCER_SERVERCONNECTION_H
#define PLUGINS_SYNCER_SERVERCONNECTION_H
#include <QObject>

class QTcpSocket;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Syncer
		{
			class ServerConnection : public QObject
			{
				Q_OBJECT

				QTcpSocket *Socket_;
				QString Chain_;
			public:
				enum ErrorCode
				{
					ECUnknownCommand = 0,
					ECUserRegistered,
					ECUserNotRegistered,
					ECWrongPassword,
					ECAlreadyConnected,
					ECOddFilterParameters,
					ECWrongDeltaID
				};

				ServerConnection (const QString&, QObject* = 0);
			public slots:
				void performLogin ();
				void reqMaxDelta ();
			private slots:
				void handleConnected ();
				void handleReadyRead ();
			signals:
				void success ();
				void fail ();
				void deltaOutOfOrder ();
			};
		}
	}
}

#endif
