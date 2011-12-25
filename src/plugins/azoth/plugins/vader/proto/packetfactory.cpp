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

#include "packetfactory.h"
#include <QCryptographicHash>
#include "headers.h"
#include "halfpacket.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	PacketFactory::PacketFactory ()
	: Seq_ (0)
	{
	}

	Packet PacketFactory::Hello ()
	{
		return HalfPacket { Header (Packets::Hello, Seq_++), QByteArray () };
	}

	Packet PacketFactory::Ping ()
	{
		return HalfPacket { Header (Packets::Ping, Seq_++), QByteArray () };
	}

	Packet PacketFactory::Login (const QString& login,
			const QString& pass,
			quint32 state,
			const QString& status,
			const QString& ua)
	{
		const QByteArray& data = ToMRIM (ToMRIM1251 (login),
				QCryptographicHash::hash (ToMRIM1251 (pass), QCryptographicHash::Md5),
				state,
				QByteArray (),
				QByteArray (),
				ToMRIM16 (status),
				static_cast<quint32> (FeatureFlag::BaseSmiles | FeatureFlag::Wakeup),
				ToMRIM1251 (ua),
				QByteArray ("ru"),
				0,
				0,
				QByteArray ("vader"));
		return HalfPacket { Header (Packets::Login2, Seq_++), data };
	}

	Packet PacketFactory::SetStatus (quint32 state, const QString& status)
	{
		const QByteArray& data = ToMRIM (state,
				QByteArray (),
				QByteArray (),
				ToMRIM16 (status));
		return HalfPacket { Header (Packets::ChangeStatus, Seq_++), data };
	}

	Packet PacketFactory::Message (MsgFlags flags,
			const QString& to, const QString& msg)
	{
		const QByteArray& data = ToMRIM (static_cast<quint32> (flags), to, msg, " ");
		return HalfPacket { Header (Packets::Msg, Seq_++), data };
	}

	Packet PacketFactory::MessageAck (const QString& from, quint32 msgId)
	{
		const QByteArray& data = ToMRIM (from, msgId);
		return HalfPacket { Header (Packets::MsgAck, Seq_++), data };
	}

	Packet PacketFactory::AddContact (ContactOpFlags flags,
			quint32 group, const QString& email, const QString& name)
	{
		const QByteArray& data = ToMRIM (static_cast<quint32> (flags), group, email, name);
		return HalfPacket { Header (Packets::Contact, Seq_++), data };
	}
}
}
}
}
