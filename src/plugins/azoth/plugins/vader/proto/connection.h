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

#ifndef PLUGINS_AZOTH_PLUGINS_VADER_PROTO_CONNECTION_H
#define PLUGINS_AZOTH_PLUGINS_VADER_PROTO_CONNECTION_H
#include <functional>
#include <QObject>
#include <QMap>
#include "packetfactory.h"
#include "packetextractor.h"

class QSslSocket;

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	class Connection : public QObject
	{
		Q_OBJECT

		QString Host_;
		int Port_;

		QString Login_;
		QString Pass_;

		QSslSocket *Socket_;

		PacketFactory PF_;
		PacketExtractor PE_;

		QMap<quint16, std::function<void (HalfPacket)>> PacketActors_;
	public:
		Connection (QObject* = 0);

		void SetTarget (const QString&, int);
		void SetCredentials (const QString&, const QString&);

		void Connect ();
	private:
		void Login ();
		QByteArray Read ();
		void Write (const QByteArray&);
	private slots:
		void tryRead ();
		void greet ();
	};
}
}
}
}

#endif
