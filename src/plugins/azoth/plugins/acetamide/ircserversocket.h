/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERSOCKET_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERSOCKET_H

#include <memory>
#include <QObject>
#include <QSslSocket>

class QTcpSocket;

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class IrcServerHandler;
	class IrcAccount;

	class IrcServerSocket : public QObject
	{
		Q_OBJECT

		IrcServerHandler *ISH_;
		bool SSL_;
		std::shared_ptr<QTcpSocket> Socket_ptr;
	public:
		IrcServerSocket (IrcServerHandler*);
		void ConnectToHost (const QString&, int);
		void DisconnectFromHost ();
		void Send (const QString&);
		void Close ();
	private:
		void Init ();
	private slots:
		void readReply ();
		void handleSslErrors (const QList<QSslError>& errors);
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERSOCKET_H
