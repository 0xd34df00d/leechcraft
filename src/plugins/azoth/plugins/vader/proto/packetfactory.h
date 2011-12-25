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

#ifndef PLUGINS_AZOTH_PLUGINS_VADER_PROTO_PACKETFACTORY_H
#define PLUGINS_AZOTH_PLUGINS_VADER_PROTO_PACKETFACTORY_H
#include <QByteArray>
#include "headers.h"

class QString;

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	struct Packet;

	class PacketFactory
	{
		quint32 Seq_;
	public:
		PacketFactory ();

		Packet Hello ();
		Packet Ping ();
		Packet Login (const QString& login, const QString& pass,
				quint32 state, const QString& status, const QString& ua);
		Packet SetStatus (quint32 state, const QString& status);
		Packet Message (MsgFlags flags, const QString& to, const QString& msg);
		Packet MessageAck (const QString& from, quint32 msgId);
		Packet AddContact (ContactOpFlags flags, quint32 group,
				const QString& email, const QString& name);
	};
}
}
}
}

#endif
