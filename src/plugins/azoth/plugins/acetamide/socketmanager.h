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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SOCKETMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SOCKETMANAGER_H

#include <QObject>
#include <QHash>
#include "core.h"

class QTcpSocket;

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	
	class IrcClient;
	class SocketManager : public QObject
	{
		Q_OBJECT

		QTcpSocket *CurrentSocket_;
		QHash<QString, QTcpSocket*> Socket2Server_;
		QString Result_;
		IrcClient *Client_;
	public:
		SocketManager (IrcClient*);
		virtual ~SocketManager ();
		void SendCommand (const ServerOptions&, const ChannelOptions&, const QString&);
		void SendCommand (const QString&, const QString&, int);
		bool IsConnected (const QString&);
		QString GetResult () const;
	private:
		QTcpSocket* CreateSocket (const QString&);
		int Connect (QTcpSocket*, const QString&, const QString&);
		void SendData (const QString&);
		void Init (QTcpSocket*);
	private slots:
		void connectionEstablished ();
		void readAnswer ();
	signals:
		void gotAnswer (const QString&, const QString&);
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SOCKETMANAGER_H
